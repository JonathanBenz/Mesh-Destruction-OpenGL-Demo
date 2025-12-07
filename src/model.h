#ifndef MODEL_H
#define MODEL_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include "mesh.h"
#include "stb_image.h"

/// <summary>
/// This class handles importing models using ASSIMP and processing its Mesh information. 
/// </summary>
class Model
{
public:
	unsigned int totalVertices;
	glm::vec3 modelCenter;
	std::vector<Mesh> meshes;
	Model(const char* path);
	void Draw(Shader& shader);

private:
	// model data
	std::string directory;
	std::vector<Texture> textures_loaded;
	glm::vec3 minBounds;
	glm::vec3 maxBounds;

	void loadModel(std::string const path);
	void processNode(aiNode *node, const aiScene *scene);
	Mesh processMesh(aiMesh *mesh, const aiScene *scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
	unsigned int TextureFromFile(const char *textureName, const std::string &directory);
};

#endif