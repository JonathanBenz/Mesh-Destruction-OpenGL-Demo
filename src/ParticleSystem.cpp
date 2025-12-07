#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(Shader& vfShader, Shader& cShader, Model& model, unsigned int maxParticles)
    : vfShader(vfShader), cShader(cShader), model(model), maxParticles(maxParticles)
{
    pos_array = NULL;
    dir_array = NULL;
    speed_array = NULL;
    color_array = NULL;

    VAO = 0;
    pos_ssbo = dir_ssbo = speed_ssbo = color_ssbo = 0;
    // Initialize the particle system
    init();
}

void ParticleSystem::init()
{
	//********* create and fulfill the particle data into shader storage buffer objects (for gen-purpose computing) **********
	GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

	// creaste a ssbo of particle positions
	glGenBuffers(1, &pos_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, pos_ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, maxParticles * sizeof(glm::vec4), NULL, GL_STATIC_DRAW); // there isn't data yet, just init memory, data will sent at run-time. 

	// map and create the postion array
	pos_array = (glm::vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, maxParticles * sizeof(glm::vec4), bufMask);

	if (pos_array != NULL)
	{
		for (const auto& mesh : model.meshes)
		{
			for (const auto& vertex : mesh.vertices)
				verticesTotal.push_back(vertex.Position);
		}

		for (unsigned int i = 0; i < maxParticles; i++)
		{
			pos_array[i].x = verticesTotal[i].x;
			pos_array[i].y = verticesTotal[i].y;
			pos_array[i].z = verticesTotal[i].z;
			pos_array[i].w = 1.0f;
		}
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	// map and create the direction array
	glGenBuffers(1, &dir_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, dir_ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, maxParticles * sizeof(glm::vec4), NULL, GL_STATIC_DRAW); // there isn't data yet, just init memory, data will sent at run-time. 
	dir_array = (glm::vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, maxParticles * sizeof(glm::vec4), bufMask);

	if (dir_array != NULL)
	{
		for (unsigned int i = 0; i < maxParticles; i++)
		{
			glm::vec4 v;
			v.x = randomf();
			v.y = randomf();
			v.z = randomf(-1, 0);
			v.w = 0.0f;
			dir_array[i] = normalize(v); // make sure having a normalized directional vector, so that its magnitude = 1
		}
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	// map and create the speed array
	glGenBuffers(1, &speed_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, speed_ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, maxParticles * sizeof(float), NULL, GL_STATIC_DRAW);
	speed_array = (float*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, maxParticles * sizeof(float), bufMask);

	if (speed_array != NULL)
	{
		for (unsigned int i = 0; i < maxParticles; i++)
			speed_array[i] = randomf(-5.0f, -1.0f);
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	// map and create the color array
	glGenBuffers(1, &color_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, color_ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, maxParticles * sizeof(glm::vec4), NULL, GL_STATIC_DRAW);
	color_array = (glm::vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, maxParticles * sizeof(glm::vec4), bufMask);

	if (color_array != NULL)
	{
		for (unsigned int i = 0; i < maxParticles; i++)
		{
			color_array[i].r = model.meshes[0].material.diffuse.r;
			color_array[i].g = model.meshes[0].material.diffuse.g;
			color_array[i].b = model.meshes[0].material.diffuse.b;
			color_array[i].a = 1.0f;
		}
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	// map and create the IsActive array
	glGenBuffers(1, &active_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, active_ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, maxParticles * sizeof(int), NULL, GL_STATIC_DRAW);

	active_array = (int*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, maxParticles * sizeof(int), bufMask);

	if (active_array != NULL)
	{
		for (unsigned int i = 0; i < maxParticles; i++)
			active_array[i] = 0; // Start all particles as inactive
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	// bind the SSBOs to the labeled "binding" in the compute shader using assigned layout labels
	// this is similar to mapping data to attribute variables in the vertex shader
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, pos_ssbo);    // 4 - lay out id for positions in compute shader 
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, dir_ssbo);	// 5 - lay out id for directions in compute shader 
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, color_ssbo);	// 6 - lay out id for colors in compute shader 
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, speed_ssbo);	// 7 - lay out id for speeds in compute shader 
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, active_ssbo);

	// ************** Define VAO (for rendering) **************
	// for particle rendering, the vertex and fragment shaders just need the verts and colors (computed by the compute shader).  
	// which need to be input "attribute" variables to the vertex shader. We set up the layout of both of these with a single vertex array object - the VAO represents our complete object, 
	// use vao's attributes mapping to each of the buffer objects with separte layout labels.
	// then, we don't need to track individual buffer objects. 
	// Note that: the purpose of vao is to have verts and colors as separate attributes in the vertex shader, 
	// the actual vert and color data have already been kept on the GPU memory by the SSBOs. 
	// So VAO's attrobites point to these data on the GPU, rather than referring back to any CPU data. 
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, pos_ssbo);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL); // 0 - the layout id in vertex shader
	glBindBuffer(GL_ARRAY_BUFFER, color_ssbo);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, NULL); // 1 - the layout id in fragment shader

	// Attributes are disabled by default in OpenGL 4. 
	// We need to explicitly enable each one.
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

void ParticleSystem::update(float deltaTime)
{
	// invoke the compute shader to update the status of particles 
    cShader.use();
    cShader.setFloat("deltaTime", deltaTime);
	cShader.setVec3("modelCenter", model.modelCenter);
	glDispatchCompute((maxParticles + 128 - 1) / 128, 1, 1); // one-dimentional GPU threading config, 128 threads per group 
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void ParticleSystem::draw(float particle_size, glm::mat4 projection, glm::mat4 view)
{
    vfShader.use();
    glBindVertexArray(VAO);
    vfShader.setMat4("proj", projection);
	vfShader.setMat4("view", view);
	glPointSize(particle_size);
    glDrawArrays(GL_POINTS, 0, maxParticles);
}

float ParticleSystem::randomf(float min, float max)
{
    if (max < min) return 0.0f;
    float n = ((float)rand()) / ((float)RAND_MAX);
    float range = max - min;
    return n * range + min;
}