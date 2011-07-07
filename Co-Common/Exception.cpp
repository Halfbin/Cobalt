//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#include <Co/Exception.hpp>

namespace Co
{
  void Exception::init (IxLockedOutStream& new_stream)
  {
    stream = &new_stream;
  }
  
  Exception::~Exception () throw ()
  {
    auto lock = stream -> get_lock ();
    (*stream) << "X Caught\n";
  }
  
  const char* Exception::what () const throw ()
  {
    return "Rk::Exception";
  }
  
  IxLockedOutStream* Exception::stream = 0;
}
