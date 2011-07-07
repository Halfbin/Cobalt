//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GLCOMPILATION
#define CO_GLRENDERER_H_GLCOMPILATION

#include <Co/IxGeomCompilation.hpp>

namespace Co
{
  class GLBuffer;
  class IxGeomBuffer;

  class GLCompilation :
    public IxGeomCompilation
  {
    u32               vao;
    const GeomAttrib* attribs;
    uint              attrib_count;
    GLBuffer*         elements;
    GLBuffer*         indices;

    ~GLCompilation ();
    virtual void destroy ();

  public:
    GLCompilation (const GeomAttrib* new_attribs, uint new_attrib_count, IxGeomBuffer* new_elements, IxGeomBuffer* new_indices);

    void use ();
  //void done ();

  }; // class GLCompilation

} // namespace Co

#endif
