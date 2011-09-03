//
// Copyright (C) 2011 Roadkill Software
// All Rights Reserved.
//

// Implements
#include <Co/Config.hpp>

// Uses
#include <Rk/File.hpp>

#include <unordered_map>

namespace Co
{
  ConfigKey::ConfigKey (Rk::StringRef data) :
    data (data)
  {

  }
  
  Rk::StringRef ConfigKey::as_string (Rk::StringRef def) const
  {
    return "";
  }

  bool ConfigKey::as_bool (bool def) const
  {
    return false;
  }

  int ConfigKey::as_int (int def) const
  {
    return 0;
  }

  float ConfigKey::as_float (float def) const
  {
    return 0.0f;
  }

  //
  // = ConfigFile ======================================================================================================
  //
  struct ConfigFile::Impl
  {
    /*struct Index
    {
      Rk::StringRef section,
                    name;
    };

    char* data;
    uptr  size;

    std::unordered_map <Index, ConfigKey> map;

    Impl (Rk::StringRef path);
    ~Impl ();

    struct Token
    {
      Rk::StringRef raw;
      enum { section, key, value, eof } type;
    };

    struct LexerContext
    {
      uint cur;
      enum { new_line, need_value } state;

      LexerContext ();

    };

    Token lex_token (LexerContext& ctx);
    void  parse_ini ();

    ConfigKey get (Rk::StringRef section, Rk::StringRef key) const;
    */
  };
  
  /*ConfigFile::Impl::LexerContext::LexerContext () :
    cur   (0),
    state (new_line)
  { }
  
  auto ConfigFile::Impl::lex_token (LexerContext& ctx)
    -> Token
  {
    for (;;)
    {
      char c = data [ctx.cur++];

      switch (c)
      {
        case ' ':
        case '\t':
          continue;
        
        case '\n':
          ctx.state = ctx.new_line;
        continue;

        case '=':
          if (ctx.state != ctx.need_value)
            throw Rk::Exception ("Unexpected \'=\'");
          else { };

      }
    }
  }

  void ConfigFile::Impl::parse_ini ()
  {
    LexerContext ctx;

    for (;;)
    {
      Token tok = lex_token (ctx);

    }
  }

  ConfigFile::Impl::Impl (Rk::StringRef path) try :
    data (0)
  {
    Rk::File file (path, Rk::File::open_read_existing);
    size = (uptr) file.size ();
    data = new char [size];
    file.read (data, size);
    file.close ();

    // Parse
    parse_ini ();
  }
  catch (...)
  {
    Rk::log_frame ("Co::ConfigFile::Impl::Impl");
    delete data;
  }

  ConfigKey ConfigFile::Impl::get (Rk::StringRef section, Rk::StringRef key) const
  {
    Index index = { section, key };
    auto iter = map.find (index);
    if (iter == map.end ())
      return nil;
    return iter -> second;
  }

  ConfigFile::Impl::~Impl ()
  {
    delete data;
  }*/

  ConfigFile::ConfigFile (Rk::StringRef path)
  {
    //impl = new Impl (path);
  }

  ConfigKey ConfigFile::operator () (Rk::StringRef section, Rk::StringRef key) const
  {
    return "";
    //return impl -> get (section, key);
  }

} // namespace Co

