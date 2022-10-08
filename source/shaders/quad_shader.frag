#version 430 core

in vec2 st;
uniform sampler2D img;
out vec4 fc;

void main()
{
	fc = texture(img, st);
}
