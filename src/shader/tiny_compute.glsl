#version 460 core
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct Triangle {
    vec3 v0;
    vec3 v1;
    vec3 v2;
};

layout(std430, binding = 0) buffer Triangles {
    Triangle triangles[];
};

layout(rgba32f, binding = 0) uniform image2D imgOutput;

void main() {
    ivec2 imgSize = imageSize(imgOutput);
    uint index = gl_GlobalInvocationID.x;

    // 三角形の各頂点のx, y座標を取得
    vec2 v0 = triangles[index].v0.xy;
    vec2 v1 = triangles[index].v1.xy;
    vec2 v2 = triangles[index].v2.xy;

    // x, y 座標をテクスチャに書き込む
    imageStore(imgOutput, ivec2(v0 * imgSize), vec4(1.0, 0.0, 0.0, 1.0));
    imageStore(imgOutput, ivec2(v1 * imgSize), vec4(0.0, 1.0, 0.0, 1.0));
    imageStore(imgOutput, ivec2(v2 * imgSize), vec4(0.0, 0.0, 1.0, 1.0));
}
