// DPTexturedMesh.cpp
// cs200 2/19

#include "DPTexturedMesh.h"


// vertex data for DP mesh
const glm::vec4 cs200::DPTexturedMesh::dp_verts[18]
  = { cs200::point(5,3),     cs200::point(5,6),
      cs200::point(2,6),     cs200::point(0,4),
      cs200::point(0,2),     cs200::point(2,0),
      cs200::point(5,0),     cs200::point(5.5f,0),
      cs200::point(6.5f,0),  cs200::point(6.5f,12),
      cs200::point(5.5f,12), cs200::point(7,9),
      cs200::point(7,6),     cs200::point(10,6),
      cs200::point(12,8),    cs200::point(12,10),
      cs200::point(10,12),   cs200::point(7,12) };


// face data for DP mesh
const cs200::Mesh::Face cs200::DPTexturedMesh::dp_faces[12]
   = { Face(0,1,2),    Face(0,2,3),
       Face(0,3,4),    Face(0,4,5),
       Face(0,5,6),    Face(7,8,9),
       Face(7,9,10),   Face(11,12,13),
       Face(11,13,14), Face(11,14,15),
       Face(11,15,16), Face(11,16,17) };


// edge data for DP mesh
const cs200::Mesh::Edge cs200::DPTexturedMesh::dp_edges[18]
  = { Edge(0,1),   Edge(1,2),
      Edge(2,3),   Edge(3,4),
      Edge(4,5),   Edge(5,6),
      Edge(6,0),   Edge(7,8),
      Edge(8,9),   Edge(9,10),
      Edge(10,7),  Edge(11,12),
      Edge(12,13), Edge(13,14),
      Edge(14,15), Edge(15,16),
      Edge(16,17), Edge(17,11) };


cs200::DPTexturedMesh::DPTexturedMesh(void) {
  glm::mat4 M = cs200::translate(cs200::vector(0.5f,0.5f))
                * cs200::scale(1.0f/12.0f)
                * cs200::translate(cs200::vector(-6,-6));
  for (int i=0; i < vertexCount(); ++i)
    dp_texcoords[i] = glm::vec2(M * vertexArray()[i]);
}


const char *cs200::DPTexturedMesh::textureFileName(void) const {
  const char *fname = "texture.bmp";
  return fname;
}


