/* Copyright (c) 2016-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/*
 * \file protover_rust.c
 * \brief Provide a C wrapper for functions exposed in /src/rust/protover,
 * and safe translation/handling between the Rust/C boundary.
 */

#include "core/or/or.h"
#include "core/or/protover.h"

#ifdef HAVE_RUST

/* Define for compatibility, used in main.c */
void
protover_free_all(void)
{
}

int protover_contains_long_protocol_names_(const char *s);

/**
 * Return true if the unparsed protover in <b>s</b> would contain a protocol
 * name longer than MAX_PROTOCOL_NAME_LENGTH, and false otherwise.
 */
bool
protover_list_is_invalid(const char *s)
{
  return protover_contains_long_protocol_names_(s) != 0;
}

#endif /* defined(HAVE_RUST) */

