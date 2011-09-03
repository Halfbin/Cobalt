//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_CONFIG
#define CO_H_CONFIG

#ifndef CO_COMMON_API 
  #define CO_COMMON_API __declspec(dllimport)
#endif

#include <Rk/StringRef.hpp>

#pragma warning (push)
#pragma warning (disable:4251)

namespace Co
{
  class CO_COMMON_API ConfigKey;
  
  class CO_COMMON_API ConfigFile
  {
    friend class ConfigKey;
    struct Impl;
    Impl* impl;

  public:
    ConfigFile (Rk::StringRef path);
    ~ConfigFile ();

    ConfigKey operator () (Rk::StringRef section, Rk::StringRef key) const;

  };

  class CO_COMMON_API ConfigKey
  {
    Rk::StringRef data;
    
    friend class ConfigFile;
    ConfigKey (Rk::StringRef);

  public:
    Rk::StringRef as_string (Rk::StringRef def = ""   ) const;
    bool          as_bool   (bool          def = false) const;
    int           as_int    (int           def = 0    ) const;
    float         as_float  (float         def = 0.0f ) const;

  };

} // namespace Co

#pragma warning (pop)

#endif
