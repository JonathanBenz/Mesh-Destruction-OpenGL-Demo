#include "model.h"

Model::Model(const char* path)
{
	// Init bounds for the model
	minBounds = glm::vec3(FLT_MAX);
	maxBounds = glm::vec3(-FLT_MAX);
	loadModel(path);
}

void Model::Draw(Shader& shader) 
{
	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		meshes[i].Draw(shader);
	}
	shader.setVec3("modelCenter", modelCenter);
}

void Model::loadModel(std::string path)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl; return;
	}

	directory = path.substr(0, path.find_last_of('\\'));
	processNode(scene->mRootNode, scene);
	modelCenter = (minBounds + maxBounds) * 0.5f; // Calculate the center of the model

	std::cout << "DEBUG LOG: ASSIMP MODEL LOAD SUCCESSFUL" << std::endl;
	glfwSetTime(0.0);
}

/// <summary>
/// Process each node of the ASSIMP scene. 
/// This includes processing the models and meshes of each node. 
/// </summary>
/// <param name="node"></param>
/// <param name="scene"></param>
void Model::processNode(aiNode* node, const aiScene* scene)
{
	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}

	// recursively go into the children and do the same
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

/// <summary>
/// Get the vertices (position, normal, texture coordinate), indices, and texture data from the mesh.
/// </summary>
/// <param name="mesh"></param>
/// <param name="scene"></param>
/// <returns></returns>
Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	totalVertices += mesh->mNumVertices;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	Material mat;

	// process vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;

		// process vertex positions, normals, and texture coordinates
		glm::vec3 vector; 
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;
		minBounds = glm::min(minBounds, vertex.Position);
		maxBounds = glm::max(maxBounds, vertex.Position);

		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = vector;

		if (mesh->mTextureCoords[0]) // if mesh contains texture coordinates
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else vertex.TexCoords = glm::vec2(0.0f, 0.0f);

		vertices.push_back(vertex);
	}
	//std::cout << "DEBUG LOG: VERTEX PROCESSING SUCCESSFUL" << std::endl;

	// process indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}
	//std::cout << "DEBUG LOG: INDEX PROCESSING SUCCESSFUL" << std::endl;

	// process textures and materials
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		// --- load textures
		// Get diffuse maps and insert them into the textures vector
		std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		// Get specular maps and insert them into the textures vector
		std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		// --- load material properties (ambience, diffuse, specular, shininess values) 
		aiColor3D color(0.0f, 0.0f, 0.0f);
		float shininess = 0.0f;

		material->Get(AI_MATKEY_COLOR_AMBIENT, color);
		mat.ambient = glm::vec3(color.r, color.g, color.b);

		material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		mat.diffuse = glm::vec3(color.r, color.g, color.b);

		material->Get(AI_MATKEY_COLOR_SPECULAR, color);
		mat.specular = glm::vec3(color.r, color.g, color.b);

		material->Get(AI_MATKEY_SHININESS, shininess);
		mat.shininess = shininess;
	}
	//std::cout << "DEBUG LOG: MATERIAL PROCESSING SUCCESSFUL" << std::endl;

	return Mesh(vertices, indices, textures, mat);
}

/// <summary>
/// Load the materials' textures. 
/// </summary>
/// <param name="mat"> assimp material. </param>
/// <param name="type"> material type (e.g., diffuse, specular, etc.) </param>
/// <param name="typeName"></param>
/// <returns></returns>
std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
	std::vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);

		bool skip = false;
		for (unsigned int j = 0; j < textures_loaded.size(); j++)
		{
			// See if we have already loaded in the material's texture. If so, we can skip loading it back in again since it is memoized 
			if (std::strcmp(textures_loaded[j].path.c_str(), str.C_Str()) == 0)
			{
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}

		// If this is the first time loading it, then load it normally 
		if (!skip)
		{
			Texture texture;
			texture.id = TextureFromFile(str.C_Str(), directory);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			textures_loaded.push_back(texture);
		}
	}

	//std::cout << "DEBUG LOG: MATERIAL TEXTURES OF TYPE " << typeName << " LOADED SUCCESSFUL" << std::endl;
	return textures;
}

/// <summary>
/// Generate and bind textures. 
/// </summary>
/// <param name="textureName"></param>
/// <param name="directory"></param>
/// <returns></returns>
unsigned int Model::TextureFromFile(const char *textureName, const std::string &directory)
{
	std::string path = directory + "\\" + textureName;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cerr << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}