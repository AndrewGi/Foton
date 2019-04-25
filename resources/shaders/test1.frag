#version 400

out vec4 frag_color;
in vec3 vc;
uniform float time;
void main()
{
    frag_color = vec4(vc, 1.0);
}