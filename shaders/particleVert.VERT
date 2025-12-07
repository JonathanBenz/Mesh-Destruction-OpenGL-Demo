#version 430

uniform mat4 view; 
uniform mat4 proj; 

layout(location = 0) in vec4 vertex_position;
layout(location = 1) in vec4 vertex_colour;

out vec4 colour;

void main() {
  colour = vertex_colour;
  gl_Position = proj * view * vertex_position;
}