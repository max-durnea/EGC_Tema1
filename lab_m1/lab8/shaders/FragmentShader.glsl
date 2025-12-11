#version 330

// Input
in vec3 world_position;
in vec3 world_normal;

// Uniforms for light properties
uniform vec3 light_direction;
uniform vec3 light_position;
uniform vec3 eye_position;

uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;

// TODO(student): Declare any other uniforms
uniform int is_spotlight;
uniform float cutoff_angle;

uniform vec3 light_color1;
uniform vec3 light_position2;
uniform vec3 light_color2;

uniform vec3 object_color;

// Output
layout(location = 0) out vec4 out_color;


void main()
{
    vec3 N = normalize(world_normal);
    vec3 V = normalize(eye_position - world_position);
    
    float ambient_light = 0.35;
    vec3 ambient = ambient_light * object_color;
    
    vec3 L1 = normalize(light_position - world_position);
    vec3 R1 = reflect(-L1, N);
    
    float diffuse_light1 = material_kd * max(dot(N, L1), 0.0);
    
    float specular_light1 = 0.0;
    
    // It's important to distinguish between "reflection model" and
    // "shading method". In this shader, we are experimenting with the Phong
    // (1975) and Blinn-Phong (1977) reflection models, and we are using the
    // Phong (1975) shading method. Don't mix them up!
    if (diffuse_light1 > 0)
    {
        specular_light1 = material_ks * pow(max(dot(V, R1), 0.0), material_shininess);
    }

    // TODO(student): If (and only if) the light is a spotlight, we need to do
    // some additional things.
    float attenuation_factor1 = 1.0;
    
    if (is_spotlight == 1)
    {
        vec3 spot_dir = normalize(light_direction);
        float theta = dot(L1, -spot_dir);
        float cut_off_rad = radians(cutoff_angle);
        float cut_off = cos(cut_off_rad);
        
        if (theta > cut_off)
        {
            float epsilon = 0.05;
            float intensity = clamp((theta - cut_off) / epsilon, 0.0, 1.0);
            attenuation_factor1 = intensity * intensity;
        }
        else
        {
            attenuation_factor1 = 0.0;
        }
    }
    
    float distance1 = length(light_position - world_position);
    float distance_attenuation1 = 1.0 / (1.0 + 0.01 * distance1 + 0.001 * distance1 * distance1);
    
    attenuation_factor1 *= distance_attenuation1;
    
    vec3 diffuse1 = attenuation_factor1 * diffuse_light1 * object_color * light_color1;
    vec3 specular1 = attenuation_factor1 * specular_light1 * light_color1;
    
    vec3 L2 = normalize(light_position2 - world_position);
    vec3 R2 = reflect(-L2, N);
    
    float diffuse_light2 = material_kd * max(dot(N, L2), 0.0);
    
    float specular_light2 = 0.0;
    if (diffuse_light2 > 0)
    {
        specular_light2 = material_ks * pow(max(dot(V, R2), 0.0), material_shininess);
    }
    
    float distance2 = length(light_position2 - world_position);
    float distance_attenuation2 = 1.0 / (1.0 + 0.01 * distance2 + 0.001 * distance2 * distance2);
    
    vec3 diffuse2 = distance_attenuation2 * diffuse_light2 * object_color * light_color2;
    vec3 specular2 = distance_attenuation2 * specular_light2 * light_color2;
   
    // TODO(student): Compute the total light. You can just add the components
    // together, but if you're feeling extra fancy, you can add individual
    // colors to the light components. To do that, pick some vec3 colors that
    // you like, and multiply them with the respective light components.
    
    vec3 final_color = ambient + diffuse1 + specular1 + diffuse2 + specular2;

    // TODO(student): Write pixel out color
    out_color = vec4(final_color, 1.0);
}
    