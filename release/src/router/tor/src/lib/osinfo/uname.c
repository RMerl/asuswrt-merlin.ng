/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file uname.c
 * \brief Look up a description of the operating system.
 **/

#include "orconfig.h"
#include "lib/osinfo/uname.h"

#include "lib/string/compat_string.h"
#include "lib/string/printf.h"

#ifdef HAVE_UNAME
#include <sys/utsname.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif
#include <string.h>

/** Hold the result of our call to <b>uname</b>. */
static char uname_result[256];
/** True iff uname_result is set. */
static int uname_result_is_set = 0;

#ifdef _WIN32
/** Table to map claimed windows versions into human-readable windows
 * versions. */
static struct {
  unsigned major;
  unsigned minor;
  const char *client_version;
  const char *server_version;
} win_version_table[] = {
  /* This table must be sorted in descending order.
   * Sources:
   *   https://en.wikipedia.org/wiki/List_of_Microsoft_Windows_versions
   *   https://docs.microsoft.com/en-us/windows/desktop/api/winnt/
   *     ns-winnt-_osversioninfoexa#remarks
   */
   /* Windows Server 2019 is indistinguishable from Windows Server 2016
    * using GetVersionEx().
   { 10,  0, NULL,                        "Windows Server 2019" }, */
   // clang-format off
   { 10,  0, "Windows 10",                "Windows Server 2016" },
   {  6,  3, "Windows 8.1",               "Windows Server 2012 R2" },
   {  6,  2, "Windows 8",                 "Windows Server 2012" },
   {  6,  1, "Windows 7",                 "Windows Server 2008 R2" },
   {  6,  0, "Windows Vista",             "Windows Server 2008" },
   {  5,  2, "Windows XP Professional",   "Windows Server 2003" },
   /* Windows XP did not have a server version, but we need something here */
   {  5,  1, "Windows XP",                "Windows XP Server" },
   {  5,  0, "Windows 2000 Professional", "Windows 2000 Server" },
   /* Earlier versions are not supported by GetVersionEx(). */
   {  0,  0, NULL,                        NULL }
   // clang-format on
};
#endif /* defined(_WIN32) */

/** Return a pointer to a description of our platform.
 */
MOCK_IMPL(const char *,
get_uname,(void))
{
#ifdef HAVE_UNAME
  struct utsname u;
#endif
  if (!uname_result_is_set) {
#ifdef HAVE_UNAME
    if (uname(&u) != -1) {
      /* (Linux says 0 is success, Solaris says 1 is success) */
      strlcpy(uname_result, u.sysname, sizeof(uname_result));
    } else
#endif /* defined(HAVE_UNAME) */
      {
#ifdef _WIN32
        OSVERSIONINFOEX info;
        int i;
        int is_client = 0;
        int is_server = 0;
        const char *plat = NULL;
        memset(&info, 0, sizeof(info));
        info.dwOSVersionInfoSize = sizeof(info);
        if (! GetVersionEx((LPOSVERSIONINFO)&info)) {
          strlcpy(uname_result, "Bizarre version of Windows where GetVersionEx"
                  " doesn't work.", sizeof(uname_result));
          uname_result_is_set = 1;
          return uname_result;
        }
#ifdef VER_NT_SERVER
        if (info.wProductType == VER_NT_SERVER ||
            info.wProductType == VER_NT_DOMAIN_CONTROLLER) {
          is_server = 1;
        } else {
          is_client = 1;
        }
#endif /* defined(VER_NT_SERVER) */
        /* Search the version table for a matching version */
        for (i=0; win_version_table[i].major>0; ++i) {
          if (win_version_table[i].major == info.dwMajorVersion &&
              win_version_table[i].minor == info.dwMinorVersion) {
            if (is_server) {
              plat = win_version_table[i].server_version;
            } else {
              /* Use client versions for clients, and when we don't know if it
              * is a client or a server. */
              plat = win_version_table[i].client_version;
            }
            break;
          }
        }
        if (plat) {
          strlcpy(uname_result, plat, sizeof(uname_result));
        } else {
          if (info.dwMajorVersion > win_version_table[0].major ||
              (info.dwMajorVersion == win_version_table[0].major &&
               info.dwMinorVersion > win_version_table[0].minor))
            tor_snprintf(uname_result, sizeof(uname_result),
                         "Very recent version of Windows [major=%d,minor=%d]",
                         (int)info.dwMajorVersion,(int)info.dwMinorVersion);
          else
            tor_snprintf(uname_result, sizeof(uname_result),
                         "Unrecognized version of Windows [major=%d,minor=%d]",
                         (int)info.dwMajorVersion,(int)info.dwMinorVersion);
        }
        /* Now append extra information to the name.
         *
         * Microsoft's API documentation says that on Windows 8.1 and later,
         * GetVersionEx returns Windows 8 (6.2) for applications without an
         * app compatibility manifest (including tor's default build).
         *
         * But in our testing, we have seen the actual Windows version on
         * Windows Server 2012 R2, even without a manifest. */
        if (info.dwMajorVersion > 6 ||
            (info.dwMajorVersion == 6 && info.dwMinorVersion >= 2)) {
          /* When GetVersionEx() returns Windows 8, the actual OS may be any
           * later version. */
          strlcat(uname_result, " [or later]", sizeof(uname_result));
        }
        /* When we don't know if the OS is a client or server version, we use
         * the client version, and this qualifier. */
        if (!is_server && !is_client) {
          strlcat(uname_result, " [client or server]", sizeof(uname_result));
        }
#else /* !defined(_WIN32) */
        /* LCOV_EXCL_START -- can't provoke uname failure */
        strlcpy(uname_result, "Unknown platform", sizeof(uname_result));
        /* LCOV_EXCL_STOP */
#endif /* defined(_WIN32) */
      }
    uname_result_is_set = 1;
  }
  return uname_result;
}
