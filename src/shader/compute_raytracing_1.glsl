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

struct Light {
    vec4 position; // Position or direction of the light
    vec4 intensity; // Intensity and color (xyz: intensity, w: not used)
};

layout(std430, binding = 0) buffer Triangles {
    Data triangles[];
};

layout(std430, binding = 1) buffer BVHNodes {
    BVHNode nodes[];
};

layout(std430, binding = 2) buffer Lights {
    Light lights[];
};

layout(rgba32f, binding = 0) uniform image2D imgOutput;

uniform vec3 cameraPosition;
uniform vec3 cameraFront;
uniform vec3 cameraUp;
uniform vec3 cameraRight;
uniform float aspectRatio;
uniform float fov;
uniform int numLights;

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

bool intersectTriangle(vec3 origin, vec3 dir, Data triangle, out float t) {
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
    
    return t > EPSILON;
}

bool traverseBVH(vec3 origin, vec3 dir, out vec3 hitPoint, out vec3 hitNormal, out float tMin) {
    int stack[64];
    int stackPtr = 0;
    stack[stackPtr++] = 0;

    bool hit = false;
    tMin = 1e30;  // 非常に大きな値で初期化

    while (stackPtr > 0) {
        int nodeIdx = stack[--stackPtr];
        BVHNode node = nodes[nodeIdx];

        if (intersectAABB(origin, dir, node.min.xyz, node.max.xyz)) {
            if (node.data.z >= 0) { // Leaf node
                for (int i = 0; i < node.data.w; ++i) {
                    Data triangle = triangles[node.data.z + i];
                    float t;
                    if (intersectTriangle(origin, dir, triangle, t) && t < tMin) {
                        hit = true;
                        tMin = t;
                        hitPoint = origin + dir * t;
                        hitNormal = normalize(cross(triangle.v1.xyz - triangle.v0.xyz, triangle.v2.xyz - triangle.v0.xyz));
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

vec3 computeLighting(vec3 hitPoint, vec3 normal, vec3 viewDir) {
    vec3 totalLight = vec3(0.0);

    for (int i = 0; i < numLights; ++i) {
        Light light = lights[i];
        vec3 lightDir = normalize(light.position.xyz - hitPoint);

        // シャドウレイの生成
        vec3 shadowOrigin = hitPoint + normal * 0.001; // シャドウアクネを防ぐための微小オフセット
        vec3 shadowHitPoint, shadowHitNormal;
        float shadowTMin;
        if (!traverseBVH(shadowOrigin, lightDir, shadowHitPoint, shadowHitNormal, shadowTMin)) {
            // Diffuse reflection (Lambertian)
            float diffuseFactor = max(dot(normal, lightDir), 0.0);
            vec3 diffuseColor = light.intensity.xyz * diffuseFactor;
            totalLight += diffuseColor;
        }
    }

    return totalLight;
}

void main() {
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 imgSize = imageSize(imgOutput);

    if (pixelCoords.x >= imgSize.x || pixelCoords.y >= imgSize.y) return;

    vec2 uv = (vec2(pixelCoords) / vec2(imgSize)) * 2.0 - 1.0;

    vec3 dir = rayDirection(fov, aspectRatio, uv);
    vec3 origin = cameraPosition;

    vec3 color = vec3(0.0);

    int maxBounces = 1;  // とりあえず1回のバウンスで終了
    int currentBounce = 0;

    while (currentBounce < maxBounces) {
        vec3 hitPoint;
        vec3 normal;
        float tMin;

        if (traverseBVH(origin, dir, hitPoint, normal, tMin)) {
            // ヒットポイントからのライティング計算
            vec3 viewDir = normalize(-dir);
            color += computeLighting(hitPoint, normal, viewDir);

            // 次のバウンスのためにoriginとdirを更新（現在は仮の更新）
            origin = hitPoint + normal * 0.001; // ヒットポイントを微小オフセット
            dir = reflect(dir, normal); // レイの反射方向を更新
        } else {
            // 交差しない場合、ループを抜ける
            break;
        }

        ++currentBounce;
    }

    imageStore(imgOutput, pixelCoords, vec4(color, 1.0));
}
