#version 400
in vec3 vp;
out vec3 vc;
uniform float time;
uniform mat4 trans;
const float PI = 3.1415926535897932384626433832795;
void main()
{
    gl_Position = trans * vec4(vp,1.0);
}