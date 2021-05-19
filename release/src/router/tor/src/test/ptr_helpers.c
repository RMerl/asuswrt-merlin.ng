/* Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#include "test/ptr_helpers.h"

/**
 * Cast <b> (inptr_t value) to a void pointer.
 */
void *
cast_intptr_to_voidstar(intptr_t x)
{
  void *r = (void *)x;

  return r;
}

/**
 * Cast x (void pointer) to inptr_t value.
 */
intptr_t
cast_voidstar_to_intptr(void *x)
{
  intptr_t r = (intptr_t)x;

  return r;
}

/**
 * Cast x (uinptr_t value) to void pointer.
 */
void *
cast_uintptr_to_voidstar(uintptr_t x)
{
  void *r = (void *)x;

  return r;
}

/**
 * Cast x (void pointer) to uinptr_t value.
 */
uintptr_t
cast_voidstar_to_uintptr(void *x)
{
  uintptr_t r = (uintptr_t)x;

  return r;
}
