/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file crypto_util.c
 *
 * \brief Common cryptographic utilities.
 **/

#include "lib/crypt_ops/crypto_util.h"
#include "lib/cc/compat_compiler.h"

#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <wincrypt.h>
#endif /* defined(_WIN32) */

#include <stdlib.h>

#ifdef ENABLE_OPENSSL
DISABLE_GCC_WARNING("-Wredundant-decls")
#include <openssl/err.h>
#include <openssl/crypto.h>
ENABLE_GCC_WARNING("-Wredundant-decls")
#endif /* defined(ENABLE_OPENSSL) */

#include "lib/log/log.h"
#include "lib/log/util_bug.h"

/**
 * Destroy the <b>sz</b> bytes of data stored at <b>mem</b>, setting them to
 * the value <b>byte</b>.
 * If <b>mem</b> is NULL or <b>sz</b> is zero, nothing happens.
 *
 * This function is preferable to memset, since many compilers will happily
 * optimize out memset() when they can convince themselves that the data being
 * cleared will never be read.
 *
 * Right now, our convention is to use this function when we are wiping data
 * that's about to become inaccessible, such as stack buffers that are about
 * to go out of scope or structures that are about to get freed.  (In
 * practice, it appears that the compilers we're currently using will optimize
 * out the memset()s for stack-allocated buffers, but not those for
 * about-to-be-freed structures. That could change, though, so we're being
 * wary.)  If there are live reads for the data, then you can just use
 * memset().
 */
void
memwipe(void *mem, uint8_t byte, size_t sz)
{
  if (sz == 0) {
    return;
  }
  /* If sz is nonzero, then mem must not be NULL. */
  tor_assert(mem != NULL);

  /* Data this large is likely to be an underflow. */
  tor_assert(sz < SIZE_T_CEILING);

  /* Because whole-program-optimization exists, we may not be able to just
   * have this function call "memset".  A smart compiler could inline it, then
   * eliminate dead memsets, and declare itself to be clever. */

#if defined(SecureZeroMemory) || defined(HAVE_SECUREZEROMEMORY)
  /* Here's what you do on windows. */
  SecureZeroMemory(mem,sz);
#elif defined(HAVE_RTLSECUREZEROMEMORY)
  RtlSecureZeroMemory(mem,sz);
#elif defined(HAVE_EXPLICIT_BZERO)
  /* The BSDs provide this. */
  explicit_bzero(mem, sz);
#elif defined(HAVE_MEMSET_S)
  /* This is in the C99 standard. */
  memset_s(mem, sz, 0, sz);
#elif defined(ENABLE_OPENSSL)
  /* This is a slow and ugly function from OpenSSL that fills 'mem' with junk
   * based on the pointer value, then uses that junk to update a global
   * variable.  It's an elaborate ruse to trick the compiler into not
   * optimizing out the "wipe this memory" code.  Read it if you like zany
   * programming tricks! In later versions of Tor, we should look for better
   * not-optimized-out memory wiping stuff...
   *
   * ...or maybe not.  In practice, there are pure-asm implementations of
   * OPENSSL_cleanse() on most platforms, which ought to do the job.
   **/

  OPENSSL_cleanse(mem, sz);
#else
  memset(mem, 0, sz);
  asm volatile("" ::: "memory");
#endif /* defined(SecureZeroMemory) || defined(HAVE_SECUREZEROMEMORY) || ... */

  /* Just in case some caller of memwipe() is relying on getting a buffer
   * filled with a particular value, fill the buffer.
   *
   * If this function gets inlined, this memset might get eliminated, but
   * that's okay: We only care about this particular memset in the case where
   * the caller should have been using memset(), and the memset() wouldn't get
   * eliminated.  In other words, this is here so that we won't break anything
   * if somebody accidentally calls memwipe() instead of memset().
   **/
  memset(mem, byte, sz);
}

/**
 * Securely all memory in <b>str</b>, then free it.
 *
 * As tor_free(), tolerates null pointers.
 **/
void
tor_str_wipe_and_free_(char *str)
{
  if (!str)
    return;
  memwipe(str, 0, strlen(str));
  tor_free_(str);
}
