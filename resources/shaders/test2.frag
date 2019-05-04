#version 400

out vec4 frag_color;
in vec3 vc;
uniform float time;
uniform mat4 model_mat;
uniform mat4 view_mat;
uniform mat4 projection_mat;
void main()
{
    frag_color = vec4(vc+gl_FragCoord.xyz/5000, 1.0);
}