#version 330 core
out vec4 FragColor;

in vec2 TexCoords;


uniform sampler2DMS screenTexture;
uniform int width;
uniform int height;

uniform float nightVision;

void main()
{
    ivec2 viewportDim = ivec2(width, height);
    ivec2 coords = ivec2(viewportDim * TexCoords);
    vec3 sample0 = texelFetch(screenTexture, coords, 0).rgb;
    vec3 sample1 = texelFetch(screenTexture, coords, 1).rgb;
    vec3 sample2 = texelFetch(screenTexture, coords, 2).rgb;
    vec3 sample3 = texelFetch(screenTexture, coords, 3).rgb;

    vec3 col = 0.25 * (sample0 + sample1 + sample2 + sample3);

    const vec3 lumvec = vec3(0.30, 0.59, 0.11);
    float intentisy = dot(lumvec,col);
    float green = clamp(intentisy / (0.59*(nightVision)), 0.0, 1.0);
    vec3 visionColor = vec3(1.0-(nightVision),green,1.0-(nightVision));

    FragColor = vec4(col * (visionColor), 1.0);
} 