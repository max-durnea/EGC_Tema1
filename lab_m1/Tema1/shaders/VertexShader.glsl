#version 330

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec3 v_color;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

// Output values to fragment shader
out vec3 world_position;
out vec3 world_normal;
out vec3 vertex_color;

void main()
{
    // Compute world space vertex position and normal
    world_position = (Model * vec4(v_position, 1.0)).xyz;
    world_normal = normalize(mat3(Model) * v_normal);
    vertex_color = v_color;

    gl_Position = Projection * View * Model * vec4(v_position, 1.0);
}
