#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "model.h"

class ParticleSystem
{
public:
    unsigned int maxParticles;
    GLuint VAO;
    // the ssbo ids of those particle's pos, dir, speed on GPU
    GLuint pos_ssbo;
    GLuint dir_ssbo;
    GLuint speed_ssbo;
    GLuint color_ssbo;
    GLuint active_ssbo;

    // Constructor
    ParticleSystem(Shader& vfShader, Shader& cShader, Model& model, unsigned int maxParticles);

    void init();
    void update(float deltaTime);
    void draw(float particle_size, glm::mat4 projection, glm::mat4 view);

private:
    glm::vec4* pos_array;	// positions (x, y, z, w) of particles, w = 1, which is mapped to pos_ssbo
    glm::vec4* dir_array;	// moving directions (x, y, z, w) of particles, w = 0, which his mapped to dir_ssbo 
    glm::vec4* color_array;	// colors (r, g, b, a) of particles, a = 1, which is mapped to color_ssbo
    float* speed_array;	    // the moving speed of particles, which is mapped to speed_ssbo 
    int* active_array;

    Shader& vfShader;
    Shader& cShader;
    Model& model;
    std::vector<glm::vec3> verticesTotal;
    float randomf(float min = -1.0f, float max = 1.0f);
};
#endif
