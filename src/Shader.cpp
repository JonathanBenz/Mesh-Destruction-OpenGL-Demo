#include "Shader.h"

// Constructors
Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	// 1. Retrive vertex/ fragment shader source code from file path
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vertexShaderFile;
	std::ifstream fragmentShaderFile;

	// error exception handling when reading files
	vertexShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fragmentShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		vertexShaderFile.open(vertexPath);
		fragmentShaderFile.open(fragmentPath);

		std::stringstream vertexShaderStream, fragmentShaderStream, geometryShaderStream;
		vertexShaderStream << vertexShaderFile.rdbuf();
		fragmentShaderStream << fragmentShaderFile.rdbuf();

		vertexShaderFile.close();
		fragmentShaderFile.close();

		vertexCode = vertexShaderStream.str();
		fragmentCode = fragmentShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cerr << "ERROR: Shader Files Not Successfully Read!" << std::endl;
	}

	// Convert shader code from cpp string to c-string
	const char* vertexShaderCode = vertexCode.c_str();
	const char* fragmentShaderCode = fragmentCode.c_str();

	// 2. Compile Shaders
	unsigned int vertex, fragment;
	int success;
	char infoLog[512];

	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexShaderCode, NULL);
	glCompileShader(vertex);
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cerr << "ERROR: Vertex Shader Compilation Failed!\n" << infoLog << std::endl;
	}

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentShaderCode, NULL);
	glCompileShader(fragment);
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cerr << "ERROR: Fragment Shader Compilation Failed!\n" << infoLog << std::endl;
	}

	// Link shaders to a program
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cerr << "ERROR: Shader Program Linkage Failed!\n" << infoLog << std::endl;
	}

	// Delete shaders after successfully linking them, they're no longer needed
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath)
{
	// 1. Retrive vertex/ fragment shader source code from file path
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	std::ifstream vertexShaderFile;
	std::ifstream fragmentShaderFile;
	std::ifstream geometryShaderFile;

	// error exception handling when reading files
	vertexShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fragmentShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	geometryShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		vertexShaderFile.open(vertexPath);
		fragmentShaderFile.open(fragmentPath);
		geometryShaderFile.open(geometryPath);

		std::stringstream vertexShaderStream, fragmentShaderStream, geometryShaderStream;
		vertexShaderStream << vertexShaderFile.rdbuf();
		fragmentShaderStream << fragmentShaderFile.rdbuf();
		geometryShaderStream << geometryShaderFile.rdbuf();

		vertexShaderFile.close();
		fragmentShaderFile.close();
		geometryShaderFile.close();

		vertexCode = vertexShaderStream.str();
		fragmentCode = fragmentShaderStream.str();
		geometryCode = geometryShaderStream.str();
	} 
	catch (std::ifstream::failure e)
	{
		std::cerr << "ERROR: Shader Files Not Successfully Read!" << std::endl;
	}

	// Convert shader code from cpp string to c-string
	const char* vertexShaderCode = vertexCode.c_str();
	const char* fragmentShaderCode = fragmentCode.c_str();
	const char* geometryShaderCode = geometryCode.c_str();

	// 2. Compile Shaders
	unsigned int vertex, fragment, geometry;
	int success;
	char infoLog[512];
	
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexShaderCode, NULL);
	glCompileShader(vertex);
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cerr << "ERROR: Vertex Shader Compilation Failed!\n" << infoLog << std::endl;
	}

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentShaderCode, NULL);
	glCompileShader(fragment);
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cerr << "ERROR: Fragment Shader Compilation Failed!\n" << infoLog << std::endl;
	}

	geometry = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(geometry, 1, &geometryShaderCode, NULL);
	glCompileShader(geometry);
	glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(geometry, 512, NULL, infoLog);
		std::cerr << "ERROR: Geometry Shader Compilation Failed!\n" << infoLog << std::endl;
	}

	// Link shaders to a program
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glAttachShader(ID, geometry);

	// Before linking, mention which output attribs we want to capture in our Transform Feedback buffer
	/*const char* feedbackVaryings[] = { "gs_out.FragPos" };
	glTransformFeedbackVaryings(ID, 1, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);*/

	glLinkProgram(ID);
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cerr << "ERROR: Shader Program Linkage Failed!\n" << infoLog << std::endl;
	}

	// Delete shaders after successfully linking them, they're no longer needed
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	glDeleteShader(geometry);
}

Shader::Shader(const char* computePath)
{
	std::string computeCode;
	std::ifstream computeShaderFile;

	computeShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		computeShaderFile.open(computePath);
		std::stringstream computeShaderStream;
		computeShaderStream << computeShaderFile.rdbuf();
		computeShaderFile.close();
		computeCode = computeShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cerr << "ERROR: Compute Shader File Not Successfully Read!" << std::endl;
	}

	const char* computeShaderCode = computeCode.c_str();

	unsigned int compute;
	int success;
	char infoLog[512];

	compute = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(compute, 1, &computeShaderCode, NULL);
	glCompileShader(compute);
	glGetShaderiv(compute, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(compute, 512, NULL, infoLog);
		std::cerr << "ERROR: Compute Shader Compilation Failed!\n" << infoLog << std::endl;
	}

	ID = glCreateProgram();
	glAttachShader(ID, compute);
	glLinkProgram(ID);
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cerr << "ERROR: Compute Shader Program Linkage Failed!\n" << infoLog << std::endl;
	}

	glDeleteShader(compute);
}

void Shader::use()
{
	glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void Shader::setInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setMat4(const std::string& name, glm::mat4& value) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, glm::vec3& value) const 
{
	glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}