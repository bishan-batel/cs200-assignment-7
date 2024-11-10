// DPTexturedMesh.h
// cs200 2/19

#ifndef CS200_MYTEXUREDMESH_H
  #define CS200_MYTEXTUREDMESH_H

  #include "TexturedMesh.h"
  #include "Affine.h"

namespace cs200 {

  class DPTexturedMesh : public TexturedMesh {
  public:

    DPTexturedMesh(void);
    int vertexCount(void) const;
    const glm::vec4* vertexArray(void) const;
    glm::vec4 dimensions(void) const;
    glm::vec4 center(void) const;
    int faceCount(void) const;
    const Face* faceArray(void) const;
    int edgeCount(void) const;
    const Edge* edgeArray(void) const;
    const glm::vec2* texcoordArray(void) const;
    const char* textureFileName(void) const;

  private:

    static const glm::vec4 dp_verts[18];
    glm::vec2 dp_texcoords[18];
    static const Face dp_faces[12];
    static const Edge dp_edges[18];
  };

}

inline int cs200::DPTexturedMesh::vertexCount(void) const {
  return 18;
}

inline const glm::vec4* cs200::DPTexturedMesh::vertexArray(void) const {
  return dp_verts;
}

inline glm::vec4 cs200::DPTexturedMesh::dimensions(void) const {
  return cs200::vector(12, 12);
}

inline glm::vec4 cs200::DPTexturedMesh::center(void) const {
  return cs200::point(6, 6);
}

inline int cs200::DPTexturedMesh::faceCount(void) const {
  return 12;
}

inline const cs200::Mesh::Face* cs200::DPTexturedMesh::faceArray(void) const {
  return dp_faces;
}

inline int cs200::DPTexturedMesh::edgeCount(void) const {
  return 18;
}

inline const cs200::Mesh::Edge* cs200::DPTexturedMesh::edgeArray(void) const {
  return dp_edges;
}

inline const glm::vec2* cs200::DPTexturedMesh::texcoordArray(void) const {
  return dp_texcoords;
}

#endif
