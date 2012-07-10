//
// Copyright (C) 2012 Roadkill Software
// All Rights Reserved.
//

#ifndef CO_H_CLOCK
#define CO_H_CLOCK

#include <Rk/Types.hpp>

#pragma comment (lib, "winmm")

namespace Co
{
  extern "C"
  {
  /*__declspec(dllimport) __stdcall i32 QueryPerformanceFrequency (u64*);
    __declspec(dllimport) __stdcall i32 QueryPerformanceCounter (u64*);*/

    void* const hkey_local_machine ((void*) uptr (0x80000002));
    enum WinRegConstants : u32 { rrf_rt_reg_dword = 0x10 };
    __declspec(dllimport) i32 __stdcall RegGetValueA (void* key, const char* sub, const char* value, u32 flags, u32* type, void* out, u32* size);

    __declspec(dllimport) u32 __stdcall timeBeginPeriod (u32);
    __declspec(dllimport) u32 __stdcall timeEndPeriod (u32);

    __declspec(dllimport) i32 __stdcall QueryPerformanceFrequency (u64*);
    __declspec(dllimport) i32 __stdcall QueryPerformanceCounter (u64*);
  }

  class Clock
  {
    u64   start;
    float ticks_per_second;

    #if defined (CO_CLOCK_USE_RDTSC)

    #ifndef _WIN64
      static __forceinline u64 rdtsc ()
      {
        register u32 hi, lo;
        __asm
        {
          lfence
          rdtsc
          mov hi, edx
          mov lo, eax
        }
        return u64 (lo) | (u64 (hi) << 32);
      }
    #else
      static __forceinline u64 rdtsc ()
      {
        register u64 result;
        __asm
        {
          lfence
          rdtsc
          mov result, rax
        }
        return result;
      }
    #endif
    
    #elif defined (CO_CLOCK_USE_RDTSCP)

    #ifndef _WIN64
      static __forceinline u64 rdtsc ()
      {
        register u32 hi, lo;
        __asm
        {
          rdtscp
          mov hi, edx
          mov lo, eax
        }
        return u64 (lo) | (u64 (hi) << 32);
      }
    #else
      static __forceinline u64 rdtsc ()
      {
        register u64 result;
        __asm
        {
          rdtscp
          mov result, rax
        }
        return result;
      }
    #endif

    #endif

  public:
    Clock ()
    {
    #if defined (CO_CLOCK_USE_RDTSC) || defined (CO_CLOCK_USE_RDTSCP)
      u32 mhz;
      u32 size = 4;
      RegGetValueA (hkey_local_machine, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "~MHz", rrf_rt_reg_dword, 0, &mhz, &size);
      
      ticks_per_second = float (u64 (mhz) * 1000000ull);
      start = rdtsc ();
    #else
      u64 perf_freq;
      QueryPerformanceFrequency (&perf_freq);
      ticks_per_second = float (perf_freq);
      QueryPerformanceCounter (&start);
    #endif
    }

    __forceinline float time () const
    {
    #if defined (CO_CLOCK_USE_RDTSC) || defined (CO_CLOCK_USE_RDTSCP)
      return float (rdtsc () - start) / ticks_per_second;
    #else
      u64 count;
      QueryPerformanceCounter (&count);
      return float (count - start) / ticks_per_second;
    #endif
    }

  };

  class MasterClock :
    public Clock
  {
  public:
    MasterClock ()
    {
      timeBeginPeriod (1);
    }
    
    ~MasterClock ()
    {
      timeEndPeriod (1);
    }

  };

} // namespace Co

#endif
