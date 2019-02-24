/* Copyright (c) 2017-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#ifndef TOR_HS_TEST_HELPERS_H
#define TOR_HS_TEST_HELPERS_H

#include "trunnel/ed25519_cert.h"
#include "feature/hs/hs_descriptor.h"

/* Set of functions to help build and test descriptors. */
hs_desc_intro_point_t *hs_helper_build_intro_point(
                          const ed25519_keypair_t *signing_kp, time_t now,
                          const char *addr, int legacy);
hs_descriptor_t *hs_helper_build_hs_desc_no_ip(
                                 const ed25519_keypair_t *signing_kp);
hs_descriptor_t *hs_helper_build_hs_desc_with_ip(
                                 const ed25519_keypair_t *signing_kp);
void hs_helper_desc_equal(const hs_descriptor_t *desc1,
                          const hs_descriptor_t *desc2);
void
hs_helper_get_subcred_from_identity_keypair(ed25519_keypair_t *signing_kp,
                                            uint8_t *subcred_out);

#endif /* !defined(TOR_HS_TEST_HELPERS_H) */

