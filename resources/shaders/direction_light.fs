#version 330 core
out vec4 FragColor;

struct DirLight {

    vec3 m_direction;

    vec3 m_ambient;
    vec3 m_diffuse;
    vec3 m_specular;

};

struct Material {

    sampler2D texture_diffuse1;
    sampler2D texture_specular1;

    float m_shininess;
};

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform DirLight directional_light;
uniform Material material;

uniform vec3 viewPosition;

// calculates the color when using a point light.
vec3 CalcDirLight(DirLight directional_light, vec3 normal, vec3 view_direction)
{
    vec3 light_direction = normalize(-directional_light.m_direction);

    float diff = max(dot(normal, light_direction), 0.0);

    vec3 reflected_light_direction = reflect(-light_direction, normal);

    vec3 halfway_direction = normalize(light_direction + view_direction);

    float spec = pow(max(dot(normal, halfway_direction), 0.0), material.m_shininess);

    vec3 ambient = directional_light.m_ambient * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = diff * directional_light.m_diffuse * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = spec * directional_light.m_specular * vec3(texture(material.texture_specular1, TexCoords).xxx);

    return (ambient + diffuse + specular);
}

void main()
{
    vec3 normal = normalize(Normal);
    vec3 view_direction = normalize(viewPosition - FragPos);
    vec3 result = CalcDirLight(directional_light, normal, view_direction);
    if (texture(material.texture_diffuse1, TexCoords).a < 0.8)
        discard;
    FragColor = vec4(result, 1.0);
}