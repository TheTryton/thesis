#version 330

in vec4 fragmentPosition;

out vec4 fragmentColor;

void main()
{
    fragmentColor = vec4((fragmentPosition.x + 1)/2, (fragmentPosition.y + 1)/2, 0.0f, 1.0f);
}