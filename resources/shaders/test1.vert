#version 400
in vec3 vp;
out vec3 vc;
uniform float time;
const float PI = 3.1415926535897932384626433832795;
void main()
{
	vc = vec3(sin(time*2+2*vp.x)/2+.5, sin(time*10-4*vp.y+PI/2)/2+.5, sin(time/3+4*vp.y)/2+.5);
	float new_y = sin(time*20+2*vp.x+PI/2)*.2+vp.y + sin(time);
    gl_Position = vec4(vp.x+sin(time)*.45 + new_y / 10 + cos(time), new_y, vp.z, 1.0);
}