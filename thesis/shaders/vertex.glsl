#version 330

layout (location = 0) in vec3 position;

out vec4 fragmentPosition;

void main()
{
    fragmentPosition = gl_Position = vec4(position.x, position.y, position.z, 1.0);
}
