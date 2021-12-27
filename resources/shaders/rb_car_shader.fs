#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;

    float shininess;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
#define NR_POINT_LIGHTS 1
#define NR_SPOT_LIGHTS 4

in VS_OUT{
    vec2 TexCoords;
    vec3 Normal;
    vec3 FragPos;
    mat3 TBN;
} fs_in;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight[NR_SPOT_LIGHTS];
uniform Material material;

uniform int hasPointLight = 0;
uniform int hasSpotLight = 0;
uniform int hasDirLight = 0;

uniform int checkSpotlight[4];
uniform float transparency = 1.0;
uniform bool blinn;

uniform bool hasNormalMap = false;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{

    vec3 norm;

    if(!hasNormalMap){
        norm = normalize(fs_in.Normal);
    }
    else{
        norm = texture(material.texture_normal1, fs_in.TexCoords).rgb;
        norm = normalize(norm * 2.0 - 1.0);
        norm = normalize(fs_in.TBN * norm);
    }

    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    vec3 result = vec3(0.0f);
    if(hasDirLight == 1){
            result = CalcDirLight(dirLight, norm,viewDir);
    }
    if (hasPointLight == 1){
        for (int i = 0; i < NR_POINT_LIGHTS; i++){
            result += CalcPointLight(pointLights[i], norm, fs_in.FragPos, viewDir);
        }
    }
    if(hasSpotLight == 1){
        for (int i = 0; i < NR_SPOT_LIGHTS; i++){
            if (checkSpotlight[i] == 1){
                result += CalcSpotLight(spotLight[i], norm, fs_in.FragPos, viewDir);
            }
        }
    }
    FragColor = vec4(result, transparency);


    //FragColor = vec4(1.0);

    //FragColor = texture(material.texture_diffuse1, fs_in.TexCoords);

}
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    float spec = 0.0f;
    if(blinn){
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    }
    else{
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    }
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    vec3 specular = light.specular * spec * texture(material.texture_specular1, fs_in.TexCoords).xxx;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    float spec = 0.0f;
    if(blinn){
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    }
    else{
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    }

    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    vec3 specular = light.specular * spec * texture(material.texture_specular1, fs_in.TexCoords).ggg;
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    float spec = 0.0f;
    if(blinn){
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    }
    else{
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    }
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction));
    //   theta = 1 treba jos malo da se istestira jer kada se stavi po defaultu, javljaju se bagovi ali deluje da lepo radi ovako
//    float theta = 1;
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, fs_in.TexCoords));
    vec3 specular = light.specular * spec * texture(material.texture_specular1, fs_in.TexCoords).ggg;

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}