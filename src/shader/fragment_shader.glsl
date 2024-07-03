#version 410 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture_diffuse;

void main()
{
    FragColor = texture(texture_diffuse, TexCoord);
}