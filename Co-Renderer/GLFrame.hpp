//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GLFRAME
#define CO_GLRENDERER_H_GLFRAME

// Implements
#include <Co/Frame.hpp>

// Uses
#include "GeomProgram.hpp"
#include "RectProgram.hpp"

namespace Co
{
  //
  // = GLFrame =========================================================================================================
  //
  class GLFrame :
    public Frame
  {
    void render_point_geoms (GeomProgram& geom_program, float alpha);
    void render_labels      (float alpha);
    void render_ui_batches  (RectProgram& rect_program);

  public:
    uint id;
    float time, prev_time;

    enum Maxima { max_garbage_vaos = 256 };
    u32 garbage_vaos [max_garbage_vaos];
    uint garbage_vao_back_index;

    void render (float alpha, GeomProgram& geom_program, RectProgram& rect_program);
    
    u32 reset (float new_prev_time, float new_current_time, u32 id_advance, u32& new_id);

  }; // class GLFrame

} // namespace Co

#endif
