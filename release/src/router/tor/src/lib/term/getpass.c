/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file getpass.c
 * \brief Cross-platform wrapper to read passphrases from the terminal.
 **/

#include "lib/term/getpass.h"

#include "lib/log/util_bug.h"
#include "lib/malloc/malloc.h"

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#include <wchar.h>
/* Some mingw headers lack these. :p */
#if defined(HAVE_DECL__GETWCH) && !HAVE_DECL__GETWCH
wint_t _getwch(void);
#endif
#ifndef WEOF
#define WEOF (wchar_t)(0xFFFF)
#endif
#if defined(HAVE_DECL_SECUREZEROMEMORY) && !HAVE_DECL_SECUREZEROMEMORY
static inline void
SecureZeroMemory(PVOID ptr, SIZE_T cnt)
{
  volatile char *vcptr = (volatile char*)ptr;
  while (cnt--)
    *vcptr++ = 0;
}
#endif /* defined(HAVE_DECL_SECUREZEROMEMORY) && !HAVE_DECL_SECUREZEROMEMORY */
#elif defined(HAVE_READPASSPHRASE_H)
#include <readpassphrase.h>
#else
#include "ext/tor_readpassphrase.h"
#endif /* defined(_WIN32) || ... */

#include <stdlib.h>
#include <string.h>

/** Emit the password prompt <b>prompt</b>, then read up to <b>buflen</b>
 * bytes of passphrase into <b>output</b>. Return the number of bytes in
 * the passphrase, excluding terminating NUL.
 */
ssize_t
tor_getpass(const char *prompt, char *output, size_t buflen)
{
  tor_assert(buflen <= SSIZE_MAX);
  tor_assert(buflen >= 1);
#if defined(HAVE_READPASSPHRASE)
  char *pwd = readpassphrase(prompt, output, buflen, RPP_ECHO_OFF);
  if (pwd == NULL)
    return -1;
  return strlen(pwd);
#elif defined(_WIN32)
  int r = -1;
  while (*prompt) {
    _putch(*prompt++);
  }

  tor_assert(buflen <= INT_MAX);
  wchar_t *buf = tor_calloc(buflen, sizeof(wchar_t));

  wchar_t *ptr = buf, *lastch = buf + buflen - 1;
  while (ptr < lastch) {
    wint_t ch = _getwch();
    switch (ch) {
      case '\r':
      case '\n':
      case WEOF:
        goto done_reading;
      case 3:
        goto done; /* Can't actually read ctrl-c this way. */
      case '\b':
        if (ptr > buf)
          --ptr;
        continue;
      case 0:
      case 0xe0:
        ch = _getwch(); /* Ignore; this is a function or arrow key */
        break;
      default:
        *ptr++ = ch;
        break;
    }
  }
 done_reading:
  ;

#ifndef WC_ERR_INVALID_CHARS
#define WC_ERR_INVALID_CHARS 0x80
#endif

  /* Now convert it to UTF-8 */
  r = WideCharToMultiByte(CP_UTF8,
                          WC_NO_BEST_FIT_CHARS|WC_ERR_INVALID_CHARS,
                          buf, (int)(ptr-buf),
                          output, (int)(buflen-1),
                          NULL, NULL);
  if (r <= 0) {
    r = -1;
    goto done;
  }

  tor_assert(r < (int)buflen);

  output[r] = 0;

 done:
  SecureZeroMemory(buf, sizeof(wchar_t)*buflen);
  tor_free(buf);
  return r;
#else
#error "No implementation for tor_getpass found!"
#endif /* defined(HAVE_READPASSPHRASE) || ... */
}
