#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

// -------------------- Variables --------------------------
uniform int implosionCounter;
uniform vec3 modelCenter;
uniform mat4 view;
uniform mat4 projection;

in VS_OUT 
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} gs_in[];

out GS_OUT 
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} gs_out;

// -------------------- Prototype Functions ----------------
vec3 Implode(vec3 position);

void main() 
{
    // For each vertex of the triangle
    for (int i = 0; i < 3; i++)
    {
        vec3 displacedPos = Implode(gs_in[i].FragPos);
        vec4 clipSpace = projection * view * vec4(displacedPos, 1.0);
        gl_Position = clipSpace;

        gs_out.FragPos = displacedPos;
        gs_out.Normal = gs_in[i].Normal;
        gs_out.TexCoords = gs_in[i].TexCoords;

        EmitVertex();
    }

    EndPrimitive();
}

vec3 Implode(vec3 position) 
{
    float maxDisplacement = 0.15 * implosionCounter; // Maximum amount that the face can move from its original position.
    float falloffRadius = 1.0; // Faces within the falloffRadius will be effected. Anything outside won't.
    float dist = length(position - modelCenter); // How far the current face is from the center. Used to determine if within falloff range. 

    float falloff = clamp(1.0 - dist / falloffRadius, 0.0, 1.0); // Values which are outside of the falloffRadius (the values clamped to 1) will have a falloff of zero (i.e., they won't be affected).
    falloff = smoothstep(0, 1, falloff); // smoothstep eases in and out providing a more natural feel to the impact crater.

    vec3 implosionVector = vec3(0.0, 0.0, -1.0) * maxDisplacement * falloff; // Magnitude and direction of the implosion direction
    return position + implosionVector;
}