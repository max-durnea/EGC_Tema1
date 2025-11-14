#version 330

// Explicit attribute locations to match CreateMesh setup in lab6.cpp
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_color;    
layout(location = 2) in vec2 a_texCoord; 
layout(location = 3) in vec3 a_normal;   

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

// Outputs for the fragment shader
//out vec3 f_color;
//out vec2 f_texCoord;
out vec3 f_normal;

void main()
{
    // forward vertex color (and a scalar for the old fragment shader)
    //f_color = a_color;
    //f_texCoord = a_texCoord;
    f_normal = a_normal;

    // compute clip-space position
    gl_Position = Projection * View * Model * vec4(a_position, 1.0);
}