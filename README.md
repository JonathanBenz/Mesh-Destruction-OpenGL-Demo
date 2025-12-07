# Mesh-Destruction-OpenGL-Demo
#### by Jonathan Benz

**Description:**  
This demo shows how using the geometry shader can visually displace a mesh, such as if a sledgehammer hit a wall and bent it. Once the mesh is "hit" over 4 times, a compute particle system plays to represent the debris flying off as the mesh is fully penetrated. This demo was built with walls in mind, but any mesh can be loaded in and will work.  

## Features

- **Geometry Shader Mesh Deformation**
- **Compute Particle System for Debris**
- **ASSIMP Asset Loading**

## GIFs
<p align="center">
  <img src="gifs/MeshDestruction.gif" alt="Mesh Destruction Demo Scene"/>
</p>

## Potential Future Work
- Right now, when the particle system activates, it does a one-to-one mapping with the mesh's vertices. This looks quite bad with sparse meshes. I would either try to tesselate right before doing the mapping so that the vertex density for the mesh is higher, or I would want to try and procedurally generate a variant of the mesh with the hole in it that quickly be switched to when the particle system is activated. That way the material information for the mesh doesn't disappear when the switch happens. 
