//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_GLRENDERER_H_GLFRAME
#define CO_GLRENDERER_H_GLFRAME

#include <Co/Frame.hpp>

namespace Co
{
  //
  // = GLFrame =========================================================================================================
  //
  class GLFrame :
    public Frame
  {
  public:
    uint id;
    float time, prev_time;

    enum Maxima { max_garbage_vaos = 256 };
    u32 garbage_vaos [max_garbage_vaos];
    uint garbage_vao_back_index;

    void render (float alpha, u32 model_to_world_loc, u32 world_to_clip_loc, u32 world_to_eye_loc);
    
    u32 reset (float new_prev_time, float new_current_time, u32 id_advance, u32& new_id);

  }; // class GLFrame

} // namespace Co

#endif
