#version 400
in vec3 vp;
out vec3 vc;
uniform float time;
const float PI = 3.1415926535897932384626433832795;
void main()
{
	vc = vec3(sin(time+4*vp.x), sin(time-4*vp.y+PI/2), sin(time+4*vp.y));
    gl_Position = vec4(vp.x+sin(time)*.15, sin(time*3+2*vp.x)*.2+vp.y, vp.z, 1.0);
}