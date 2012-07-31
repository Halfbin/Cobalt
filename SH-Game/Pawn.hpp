//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef SH_H_PAWN
#define SH_H_PAWN

// Uses
#include <Co/Spatial.hpp>
#include <Co/UIInput.hpp>
#include <Co/Frame.hpp>


#include <memory>

namespace SH
{
  class Pawn
  {
  protected:
    Co::Spatial view_cur,
                view_next;

  public:
    typedef std::shared_ptr <Pawn> Ptr;

    void get_view (Co::Spatial& cur, Co::Spatial& next)
    {
      cur  = view_cur;
      next = view_next;
    }

    virtual void tick   (float time, float step, const Co::KeyState* kb, v2f mouse_delta) = 0;
    virtual void render (Co::Frame& frame, float alpha) = 0;

  };

  Pawn::Ptr create_spectator (Co::Spatial);
  Pawn::Ptr create_player    (Co::Spatial);

}

#endif
