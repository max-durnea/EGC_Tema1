#version 330

// Input
// TODO(student): Get values from vertex shader
//in vec3 f_color;
in vec3 f_normal;
//in vec2 f_texcoord;
// Output
layout(location = 0) out vec4 out_color;


void main()
{
    // TODO(student): Write pixel out color
    out_color = vec4(abs(f_normal*0.5),1.0);

}
