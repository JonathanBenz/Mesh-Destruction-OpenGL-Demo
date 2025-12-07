#include <iostream>
#include <iomanip>
#include <numeric>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"
#include "Shader.h"
#include "model.h"
#include "ParticleSystem.h"

// ------------------------------------ Prototype Functions ------------------------------------
int Init();
void TrackFPS();
void PrintFPSDiagnostic();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
// ---------------------------------------------------------------------------------------------

// ------------------------------------ Global Variables ---------------------------------------
// --- Screen Settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
GLFWwindow* window = nullptr;

// --- Camera Settings
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
bool firstMouse = true;                                    // To prevent huge jump on screen when first starting program
float pitch = 0.0f;                                        // Up-Down camera rotation (around the X-axis)
float yaw = -90.0f;                                        // Left-Right camera rotation (around the Y-axis). Init to face forward. 
float lastX = SCR_WIDTH / 2.0f, lastY = SCR_HEIGHT / 2.0f; // Init to middle of the screen
float FOV = 45.0f;                                         // Default FOV value
float cameraSpeed = 2.5f;
float cameraSensitivity = 0.1f;

// --- Delta Time 
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// --- FPS Tracker
float FPS = 0.0f;
float secondsPassed = 0.0f;
float maxFPS = 0.0f;
float minFPS = FLT_MAX;
float meanFPS = 0.0f;
int tickCount = 0;
std::vector<float> totalFPSTracked; // This will be needed to get the mean FPS at the end of the program

// --- Input Tracker
int buttonPressCounter = 0;
bool wasPressed = false;
bool inputThresholdReached = false;

// ---------------------------------------------------------------------------------------------

int main()
{
	// Initialize GLFW window and GLAD function pointers. Exit out of program early and terminate if -1 is returned
	if (Init() == -1) return -1;

	// tell stb_image.h to flip any loaded textures on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);

	// build and compile shaders
	Shader vgfShader("shaders\\vertexShader.VERT", "shaders\\fragmentShader.FRAG", "shaders\\geometryShader.GEO");
	Shader particleShader("shaders\\particleVert.VERT", "shaders\\particleFrag.FRAG");
	Shader cShader("shaders\\computeShader.COMP");

	// load models
	//Model brickWallModel("assets\\models\\goblin\\EvilCartoonVillain.obj");
	//Model brickWallModel("assets\\models\\brick_wall\\brick_wall.obj");
	Model brickWallModel("assets\\models\\brick_wall\\brick_wall_highres.obj");

	// initialize Particle System
	ParticleSystem particleSystem(particleShader, cShader, brickWallModel, brickWallModel.totalVertices);

	// Enable depth testing and MSAA
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	// Set up random seed
	std::srand(std::time(nullptr));

	// Run the render loop
	while (!glfwWindowShouldClose(window))
	{
		// Calculate delta time
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Track FPS data
		TrackFPS();

		// Check for when wall has been hit enough times to switch shaders
		if (buttonPressCounter > 4) inputThresholdReached = true;

		// Get user input
		processInput(window);

		// ------------------------------ Render stuff here... ------------------------------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Model/ View/ Projection transforms
		glm::mat4 projection = glm::perspective(glm::radians(FOV), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		vgfShader.setMat4("projection", projection);
		vgfShader.setMat4("view", view);

		// If wall has been hit less than 3 times, run the normal vertex/ geometry/ fragment shaders
		if (!inputThresholdReached)
		{
			// Enable shader
			vgfShader.use();
			vgfShader.setInt("implosionCounter", buttonPressCounter);
			vgfShader.setVec3("cameraPos", cameraPos);

			// Set up direction light 
			vgfShader.setVec3("dirLight.direction", -0.1f, -0.2f, -0.9f);
			vgfShader.setVec3("dirLight.ambient", 0.33f, 0.33f, 0.33f);
			vgfShader.setVec3("dirLight.diffuse", 1.0f, 1.0f, 1.0f);
			vgfShader.setVec3("dirLight.specular", 1.0f, 0.6f, 0.3f);

			// Render the loaded model. Bring it to origin and initialize scale to 1:1:1
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
			vgfShader.setMat4("model", model);
			brickWallModel.Draw(vgfShader);
		}

		// If the hit threshold is reached, switch to the particle system compute shader
		else
		{
			particleSystem.update(deltaTime);
			particleSystem.draw(2.0f, projection, view);
		}

		// Check and call events/ callback functions, then swap the buffer
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	PrintFPSDiagnostic(); // Print out FPS information before terminating
	glfwTerminate();
	return 0;
}

int Init()
{
	// Initialize GLFW, tell it we are using OpenGL 3.3 and to use the core profile
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4); // Enable MSAA

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Mesh Destruction OpenGL Demo", NULL, NULL);

	// Handle Errors if window fails to be created
	if (window == NULL)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Make sure GLAD is initialized so that it can manage OpenGL Function Pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Initialize the viewport
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	// Set a callback function for resizing the window
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Disable v-sync
	glfwSwapInterval(0);
}

void TrackFPS()
{
	tickCount++;
	secondsPassed += deltaTime;
	if (secondsPassed >= 1.0f) // When 1 second has passed
	{
		FPS = tickCount;
		totalFPSTracked.emplace_back(FPS);
		maxFPS = std::max(maxFPS, FPS);
		minFPS = std::min(minFPS, FPS);
		
		tickCount = 0;
		secondsPassed = 0.0f;
	}
}

void PrintFPSDiagnostic()
{
	meanFPS = std::accumulate(totalFPSTracked.begin(), totalFPSTracked.end(), 0.0) / totalFPSTracked.size();
	std::cout << "\n---------------- FPS RUNTIME DATA ----------------" << std::endl;
	std::cout << " > Mean FPS: " << std::fixed << std::setprecision(2) << meanFPS << std::endl;
	std::cout << " > Minimum FPS: " << minFPS << std::endl;
	std::cout << " > Maximum FPS: " << maxFPS << std::endl;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // ESC
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // UP
		cameraPos += cameraFront * cameraSpeed * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) // LEFT
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed * deltaTime;
	
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) // DOWN
		cameraPos -= cameraFront * cameraSpeed * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) // RIGHT
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) // LEFT SHIFT
		cameraSpeed = 10.0f;
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
		cameraSpeed = 2.5f;

	// Left Mouse Button
	bool isPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS; // Check if mouse is currently being pressed
	if (!isPressed && wasPressed && !inputThresholdReached) // Check if the mouse was let go but was previously being pressed (only caring for a singular click and not the mouse being held down)
		buttonPressCounter++;
	wasPressed = isPressed;

	// Keep the user on the ground plane
	cameraPos.y = 0.0f;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	// To prevent huge jump on screen when first starting program
	if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }

	// Calculate mouse offset since last frame
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed: y ranges from bottom to top
	lastX = xpos;
	lastY = ypos;

	// Adjust according to camera sensitivity
	xoffset *= cameraSensitivity;
	yoffset *= cameraSensitivity;

	// Add the offsets to the yaw and pitch
	yaw += xoffset;
	pitch += yoffset;

	// Prevent LookAt Flip if pitch matches the World Up vector
	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	// Do trig calculations to determine the direction vector
	glm::vec3 direction;
	direction.x = glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
	direction.y = glm::sin(glm::radians(pitch));
	direction.z = glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}