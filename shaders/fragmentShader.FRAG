#version 330 core

// -------------------- Structs ----------------------------
struct Material 
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	float shininess;
};

struct DirLight
{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

// -------------------- Variables --------------------------
uniform Material material;
uniform DirLight dirLight;
uniform vec3 cameraPos;
uniform bool hasDiffuseTex;
uniform bool hasSpecularTex;

in GS_OUT 
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

out vec4 FragColor;

// -------------------- Prototype Functions ----------------
vec3 CalculateDirLight(DirLight light, vec3 normal, vec3 viewDir);

void main()
{    
	vec3 norm = normalize(fs_in.Normal);
	vec3 viewDir = normalize(cameraPos - fs_in.FragPos);
	vec3 result = CalculateDirLight(dirLight, norm, viewDir);

	FragColor = vec4(result, 1.0f);
}

vec3 CalculateDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);

	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);

	// specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);

	// Check whether to use material texture maps or not 
	// (if no texture, use regular material color values)
	vec3 diffuseColor = hasDiffuseTex ? vec3(texture(material.texture_diffuse1, fs_in.TexCoords)) : material.diffuse;
	vec3 specularColor = hasSpecularTex ? vec3(texture(material.texture_specular1, fs_in.TexCoords)) : material.specular;

	// combine results
	vec3 ambient = light.ambient * diffuseColor;
	vec3 diffuse = light.diffuse * diff * diffuseColor;
	vec3 specular = light.specular * spec * specularColor;

	return (ambient + diffuse + specular);
}