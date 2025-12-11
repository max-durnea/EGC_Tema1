#version 330

// Input
in vec3 world_position;
in vec3 world_normal;
in vec3 vertex_color;

// Uniforms for light properties
uniform vec3 light_position;
uniform vec3 eye_position;

uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;

// Output
layout(location = 0) out vec4 out_color;

void main()
{
    vec3 N = normalize(world_normal);
    vec3 L = normalize(light_position - world_position);
    vec3 V = normalize(eye_position - world_position);
    vec3 R = reflect(-L, N);
    
    // Ambient light
    float ambient_light = 0.25;
    vec3 ambient = ambient_light * vertex_color;
    
    // Diffuse light
    float diffuse_light = material_kd * max(dot(N, L), 0.0);
    
    // Specular light
    float specular_light = 0.0;
    if (diffuse_light > 0)
    {
        specular_light = material_ks * pow(max(dot(V, R), 0.0), material_shininess);
    }
    
    // Attenuation
    float distance = length(light_position - world_position);
    float attenuation_factor = 1.0 / (1.0 + 0.1 * distance + 0.01 * distance * distance);
    
    // Compute final color
    vec3 diffuse = attenuation_factor * diffuse_light * vertex_color;
    vec3 specular = attenuation_factor * specular_light * vec3(1.0);
    
    vec3 final_color = ambient + diffuse + specular;
    
    out_color = vec4(final_color, 1.0);
}
