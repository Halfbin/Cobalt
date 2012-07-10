//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//
/*
// Implements
#include "Net.hpp"

// Uses
#include <Rk/Net/AsyncDatagramSocket.hpp>
#include <Rk/Net/IP.hpp>
#include <Rk/IO.hpp>

namespace Co
{
  class NetImpl :
    public Net
  {
    typedef Rk::Net::TCPIP                       Proto;
    typedef Rk::Net::AsyncDatagramSocket <Proto> Sock;
    
    Rk::IO::Service service;
    Sock            socket;

  public:
    NetImpl ();

    virtual void init     (uint concurrency);
    virtual void shutdown ();

    virtual void open  (u16 port);
    virtual void close ();

    virtual void work ();

  } net_impl;

  NetImpl::NetImpl () :
    service (nil)
  { }
  
  void NetImpl::init (uint concurrency)
  {
    service.start (concurrency);
  }

  void NetImpl::shutdown ()
  {
    close ();
    service.stop ();
  }

  void NetImpl::open (u16 port)
  {
    socket.bind (Proto::Endpoint::local (port));
    socket.open (service);
  }

  void NetImpl::close ()
  {
    socket.cancel ();
    socket.close ();
  }

  void NetImpl::work ()
  {
    for (;;)
      service.work_once ();
  }

  Net& net = net_impl;

};
*/