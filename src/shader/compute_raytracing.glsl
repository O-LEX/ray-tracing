#version 460 core
layout(local_size_x = 16, local_size_y = 16) in;

struct Data {
    vec4 v0;
    vec4 v1;
    vec4 v2;
};

struct BVHNode {
    vec4 min;
    vec4 max;
    ivec4 data; // x: left, y: right, z: dataOffset, w: dataCount
};

layout(std430, binding = 0) buffer Triangles {
    Data triangles[];
};

layout(std430, binding = 1) buffer BVHNodes {
    BVHNode nodes[];
};

layout(rgba32f, binding = 0) uniform image2D imgOutput;

uniform vec3 cameraPosition;
uniform vec3 cameraFront;
uniform vec3 cameraUp;
uniform vec3 cameraRight;
uniform float aspectRatio;
uniform float fov;

vec3 rayDirection(float fov, float aspectRatio, vec2 uv) {
    float tanFov = tan(radians(fov) / 2.0);
    return normalize(uv.x * cameraRight * aspectRatio * tanFov + uv.y * cameraUp * tanFov + cameraFront);
}

bool intersectAABB(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax) {
    vec3 tMin = (boxMin - rayOrigin) / rayDir;
    vec3 tMax = (boxMax - rayOrigin) / rayDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return tNear <= tFar && tFar > 0;
}

bool intersectTriangle(vec3 origin, vec3 dir, Data triangle, out float t, out vec3 normal) {
    const float EPSILON = 0.0000001;
    vec3 edge1, edge2, h, s, q;
    float a, f, u, v;

    vec3 v0 = triangle.v0.xyz;
    vec3 v1 = triangle.v1.xyz;
    vec3 v2 = triangle.v2.xyz;

    edge1 = v1 - v0;
    edge2 = v2 - v0;
    h = cross(dir, edge2);
    a = dot(edge1, h);

    if (a > -EPSILON && a < EPSILON)
        return false;

    f = 1.0 / a;
    s = origin - v0;
    u = f * dot(s, h);

    if (u < 0.0 || u > 1.0)
        return false;

    q = cross(s, edge1);
    v = f * dot(dir, q);

    if (v < 0.0 || u + v > 1.0)
        return false;

    t = f * dot(edge2, q);
    if (t > EPSILON) {
        vec3 intersectionPoint = origin + t * dir;
        normal = normalize(cross(edge1, edge2));
        return true;
    } else
        return false;
}

bool traverseBVH(vec3 origin, vec3 dir, out float t, out vec3 normal) {
    int stack[64];
    int stackPtr = 0;
    stack[stackPtr++] = 0;

    bool hit = false;
    t = 1e30;
    normal = vec3(0.0);

    while (stackPtr > 0) {
        int nodeIdx = stack[--stackPtr];
        BVHNode node = nodes[nodeIdx];

        if (intersectAABB(origin, dir, node.min.xyz, node.max.xyz)) {
            if (node.data.z >= 0) { // Leaf node
                for (int i = 0; i < node.data.w; ++i) {
                    float tmpT;
                    vec3 tmpNormal;
                    Data triangle = triangles[node.data.z + i];
                    if (intersectTriangle(origin, dir, triangle, tmpT, tmpNormal) && tmpT < t) {
                        hit = true;
                        t = tmpT;
                        normal = tmpNormal;
                    }
                }
            } else { // Internal node
                stack[stackPtr++] = node.data.y; // Right child
                stack[stackPtr++] = node.data.x; // Left child
            }
        }
    }
    return hit;
}

void main() {
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 imgSize = imageSize(imgOutput);

    if (pixelCoords.x >= imgSize.x || pixelCoords.y >= imgSize.y) return;

    vec2 uv = (vec2(pixelCoords) / vec2(imgSize)) * 2.0 - 1.0;

    vec3 dir = rayDirection(fov, aspectRatio, uv);
    vec3 origin = cameraPosition;

    vec3 color = vec3(0.0);
    vec3 throughput = vec3(1.0);
    int maxBounces = 10;
    int bounces = 0;

    while (bounces < maxBounces) {
        float t;
        vec3 normal;
        if (traverseBVH(origin, dir, t, normal)) {
            color += throughput * vec3(0.8); // A simple diffuse color
            throughput *= 0.8; // Reduce the throughput for each bounce
            origin += t * dir + normal * 0.001; // Move the origin slightly to avoid self-intersection
            dir = reflect(dir, normal); // Reflect the ray
        } else {
            color += throughput * vec3(0.1, 0.1, 0.3); // Background color
            break;
        }
        bounces++;
    }

    imageStore(imgOutput, pixelCoords, vec4(color, 1.0));
}
