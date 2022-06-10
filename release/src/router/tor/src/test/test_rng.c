/* Copyright (c) 2016-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/*
 * Example usage:
 *
 * ./src/test/test-rng --emit | dieharder -g 200 -a
 *
 * Remember, dieharder can tell you that your RNG is completely broken, but if
 * your RNG is not _completely_ broken, dieharder cannot tell you whether your
 * RNG is actually secure.
 */

#include "orconfig.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "lib/crypt_ops/crypto_rand.h"

int
main(int argc, char **argv)
{
  uint8_t buf[0x123];

  if (argc != 2 || strcmp(argv[1], "--emit")) {
    fprintf(stderr, "If you want me to fill stdout with a bunch of random "
            "bytes, you need to say --emit.\n");
    return 1;
  }

  if (crypto_seed_rng() < 0) {
    fprintf(stderr, "Can't seed RNG.\n");
    return 1;
  }

#if 0
  while (1) {
    crypto_rand(buf, sizeof(buf));
    if (write(1 /*stdout*/, buf, sizeof(buf)) != sizeof(buf)) {
      fprintf(stderr, "write() failed: %s\n", strerror(errno));
      return 1;
    }
  }
#endif /* 0 */

  crypto_fast_rng_t *rng = crypto_fast_rng_new();
  while (1) {
    crypto_fast_rng_getbytes(rng, buf, sizeof(buf));
    if (write(1 /*stdout*/, buf, sizeof(buf)) != sizeof(buf)) {
      fprintf(stderr, "write() failed: %s\n", strerror(errno));
      return 1;
    }
  }
}
