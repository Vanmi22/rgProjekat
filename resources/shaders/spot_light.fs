#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float m_shininess;
};

struct spotLight {
    vec3 m_position;
    vec3 m_direction;
    float m_cutOff;
    float m_outerCutOff;

    vec3 m_ambient;
    vec3 m_diffuse;
    vec3 m_specular;

    float m_constant;
    float m_linear;
    float m_quadratic;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform Material material;
uniform spotLight light;

void main()
{
    // ambient
    vec3 ambient = light.m_ambient * vec3(texture(material.texture_diffuse1, TexCoords));

    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.m_position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.m_diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));

    // specular
    vec3 viewDir = normalize(light.m_position - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.m_shininess);
    vec3 specular = light.m_specular * spec * vec3(texture(material.texture_specular1, TexCoords).xxx);

    // spotlight (soft edges)
    float theta = dot(lightDir, normalize(-light.m_direction));
    float epsilon = (light.m_cutOff - light.m_outerCutOff);
    float intensity = clamp((theta - light.m_outerCutOff) / epsilon, 0.0, 1.0);
    diffuse  *= intensity;
    specular *= intensity;

    // attenuation
    float distance    = length(light.m_position - FragPos);
    float attenuation = 1.0 / (light.m_constant + light.m_linear * distance + light.m_quadratic * (distance * distance));
    ambient  *= attenuation;
    diffuse   *= attenuation;
    specular *= attenuation;

    vec3 result = ambient + diffuse + specular;
    if (texture(material.texture_diffuse1, TexCoords).a < 0.8)
            discard;
    FragColor = vec4(result, 1.0);
}