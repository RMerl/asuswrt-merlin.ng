/* Copyright (c) 2018-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file winprocess_sys.c
 * \brief Subsystem object for windows process setup.
 **/

#include "orconfig.h"
#include "lib/subsys/subsys.h"
#include "lib/llharden/winprocess_sys.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef _WIN32
#include <windows.h>

#define WINPROCESS_SYS_ENABLED true

static int
subsys_winprocess_initialize(void)
{
#ifndef HeapEnableTerminationOnCorruption
#define HeapEnableTerminationOnCorruption 1
#endif

  /* On heap corruption, just give up; don't try to play along. */
  HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

  /* SetProcessDEPPolicy is only supported on 32-bit Windows.
   * (On 64-bit Windows it always fails, and some compilers don't like the
   * PSETDEP cast.)
   * 32-bit Windows defines _WIN32.
   * 64-bit Windows defines _WIN32 and _WIN64. */
#ifndef _WIN64
  /* Call SetProcessDEPPolicy to permanently enable DEP.
     The function will not resolve on earlier versions of Windows,
     and failure is not dangerous. */
  HMODULE hMod = GetModuleHandleA("Kernel32.dll");
  if (hMod) {
    typedef BOOL (WINAPI *PSETDEP)(DWORD);
    PSETDEP setdeppolicy = (PSETDEP)GetProcAddress(hMod,
                           "SetProcessDEPPolicy");
    if (setdeppolicy) {
      /* PROCESS_DEP_ENABLE | PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION */
      setdeppolicy(3);
    }
  }
#endif /* !defined(_WIN64) */

  return 0;
}
#else /* !defined(_WIN32) */
#define WINPROCESS_SYS_ENABLED false
#define subsys_winprocess_initialize NULL
#endif /* defined(_WIN32) */

const subsys_fns_t sys_winprocess = {
  .name = "winprocess",
  SUBSYS_DECLARE_LOCATION(),
  /* HeapEnableTerminationOnCorruption and setdeppolicy() are security
   * features, we want them to run first. */
  .level = -100,
  .supported = WINPROCESS_SYS_ENABLED,
  .initialize = subsys_winprocess_initialize,
};
