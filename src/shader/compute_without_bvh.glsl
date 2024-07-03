#version 460 core
layout(local_size_x = 16, local_size_y = 16) in;

struct Triangle {
    vec4 v0;
    vec4 v1;
    vec4 v2;
};

layout(std430, binding = 0) buffer Triangles {
    Triangle triangles[];
};

layout(rgba32f, binding = 0) uniform image2D imgOutput;

uniform vec3 cameraPosition;
uniform vec3 cameraFront;
uniform vec3 cameraUp;
uniform vec3 cameraRight;
uniform float aspectRatio;
uniform float fov;
uniform int numTriangles;

vec3 rayDirection(float fov, float aspectRatio, vec2 uv) {
    float tanFov = tan(radians(fov) / 2.0);
    return normalize(uv.x * cameraRight * aspectRatio * tanFov + uv.y * cameraUp * tanFov + cameraFront);
}

bool intersectTriangle(vec3 origin, vec3 dir, Triangle triangle) {
    const float EPSILON = 0.0000001;
    vec3 edge1, edge2, h, s, q;
    float a, f, u, v;

    // Extract vec3 components from vec4
    vec3 v0 = triangle.v0.xyz;
    vec3 v1 = triangle.v1.xyz;
    vec3 v2 = triangle.v2.xyz;
    
    edge1 = v1 - v0;
    edge2 = v2 - v0;
    h = cross(dir, edge2);
    a = dot(edge1, h);
    
    if (a > -EPSILON && a < EPSILON)
        return false;    // Ray is parallel to the triangle

    f = 1.0 / a;
    s = origin - v0;
    u = f * dot(s, h);
    
    if (u < 0.0 || u > 1.0)
        return false;
    
    q = cross(s, edge1);
    v = f * dot(dir, q);
    
    if (v < 0.0 || u + v > 1.0)
        return false;
    
    return true;
}

void main() {
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 imgSize = imageSize(imgOutput);

    if (pixelCoords.x >= imgSize.x || pixelCoords.y >= imgSize.y) return;

    vec2 uv = (vec2(pixelCoords) / vec2(imgSize)) * 2.0 - 1.0;

    vec3 dir = rayDirection(fov, aspectRatio, uv);
    vec3 origin = cameraPosition;

    vec3 color = vec3(0.0);

    for (int i = 0; i < numTriangles; ++i) {
        Triangle triangle = triangles[i];
        if (intersectTriangle(origin, dir, triangle)) {
            color = vec3(1.0);
        }
    }

    imageStore(imgOutput, pixelCoords, vec4(color, 1.0));
}