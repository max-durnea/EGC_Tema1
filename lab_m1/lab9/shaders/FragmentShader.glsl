#version 330

// Input
in vec2 texcoord;

// Uniform properties
uniform sampler2D texture_1;
uniform sampler2D texture_2;
uniform int has_texture_2;  // Flag to check if second texture exists
uniform float time;  // Engine time for animation
uniform int animate_texture;  // Flag to enable texture scrolling
// TODO(student): Declare various other uniforms

// Output
layout(location = 0) out vec4 out_color;


void main()
{
    // TODO(student): Calculate the out_color using the texture2D() function.

    vec2 modified_texcoord = texcoord;
    
    // Animate texture coordinates if flag is set
    if (animate_texture == 1)
    {
        // Scroll texture on X axis (S coordinate) based on time
        modified_texcoord.x = texcoord.x + time * 0.1;  // 0.1 is scroll speed
    }
    
    vec4 color1 = texture2D(texture_1, modified_texcoord);
    
    if (color1.a < 0.5f)
    {
        discard;
    }
    
    // Check if we should use the second texture
    if (has_texture_2 == 1)
    {
        vec4 color2 = texture2D(texture_2, modified_texcoord);
        vec3 mixedColor = mix(color1.rgb, color2.rgb, 0.5f);
        out_color = vec4(mixedColor, color1.a);
    }
    else
    {
        // Use only the first texture
        out_color = color1;
    }
}
