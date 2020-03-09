/* Copyright (c) 2016-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file protover.h
 * \brief Headers and type declarations for protover.c
 **/

#ifndef TOR_PROTOVER_H
#define TOR_PROTOVER_H

#include <stdbool.h>
#include "lib/cc/torint.h"
#include "lib/testsupport/testsupport.h"
struct smartlist_t;

/** The first version of Tor that included "proto" entries in its
 * descriptors.  Authorities should use this to decide whether to
 * guess proto lines. */
/* This is a guess. */
/// C_RUST_COUPLED: src/rust/protover/protover.rs
///                 `FIRST_TOR_VERSION_TO_ADVERTISE_PROTOCOLS`
#define FIRST_TOR_VERSION_TO_ADVERTISE_PROTOCOLS "0.2.9.3-alpha"

/** The protover version number that signifies HSDir support for HSv3 */
#define PROTOVER_HSDIR_V3 2
/** The protover version number that signifies HSv3 intro point support */
#define PROTOVER_HS_INTRO_V3 4
/** The protover version number that signifies HSv3 rendezvous point support */
#define PROTOVER_HS_RENDEZVOUS_POINT_V3 2
/** The protover that signals support for HS circuit setup padding machines */
#define PROTOVER_HS_SETUP_PADDING 2

/** List of recognized subprotocols. */
/// C_RUST_COUPLED: src/rust/protover/ffi.rs `translate_to_rust`
/// C_RUST_COUPLED: src/rust/protover/protover.rs `Proto`
typedef enum protocol_type_t {
  PRT_LINK      = 0,
  PRT_LINKAUTH  = 1,
  PRT_RELAY     = 2,
  PRT_DIRCACHE  = 3,
  PRT_HSDIR     = 4,
  PRT_HSINTRO   = 5,
  PRT_HSREND    = 6,
  PRT_DESC      = 7,
  PRT_MICRODESC = 8,
  PRT_CONS      = 9,
  PRT_PADDING   = 10,
  PRT_FLOWCTRL  = 11,
} protocol_type_t;

bool protover_contains_long_protocol_names(const char *s);
int protover_all_supported(const char *s, char **missing);
int protover_is_supported_here(protocol_type_t pr, uint32_t ver);
const char *protover_get_supported_protocols(void);

char *protover_compute_vote(const struct smartlist_t *list_of_proto_strings,
                            int threshold);
const char *protover_compute_for_old_tor(const char *version);
int protocol_list_supports_protocol(const char *list, protocol_type_t tp,
                                    uint32_t version);
int protocol_list_supports_protocol_or_later(const char *list,
                                             protocol_type_t tp,
                                             uint32_t version);

void protover_free_all(void);

#ifdef PROTOVER_PRIVATE
/** Represents a range of subprotocols of a given type. All subprotocols
 * between <b>low</b> and <b>high</b> inclusive are included. */
typedef struct proto_range_t {
  uint32_t low;
  uint32_t high;
} proto_range_t;

/** Represents a set of ranges of subprotocols of a given type. */
typedef struct proto_entry_t {
  /** The name of the protocol.
   *
   * (This needs to handle voting on protocols which
   * we don't recognize yet, so it's a char* rather than a protocol_type_t.)
   */
  char *name;
  /** Smartlist of proto_range_t */
  struct smartlist_t *ranges;
} proto_entry_t;

#if !defined(HAVE_RUST) && defined(TOR_UNIT_TESTS)
STATIC struct smartlist_t *parse_protocol_list(const char *s);
STATIC char *encode_protocol_list(const struct smartlist_t *sl);
STATIC const char *protocol_type_to_str(protocol_type_t pr);
STATIC int str_to_protocol_type(const char *s, protocol_type_t *pr_out);
STATIC void proto_entry_free_(proto_entry_t *entry);
#endif /* !defined(HAVE_RUST) && defined(TOR_UNIT_TESTS) */

#define proto_entry_free(entry) \
  FREE_AND_NULL(proto_entry_t, proto_entry_free_, (entry))

#endif /* defined(PROTOVER_PRIVATE) */

#endif /* !defined(TOR_PROTOVER_H) */
