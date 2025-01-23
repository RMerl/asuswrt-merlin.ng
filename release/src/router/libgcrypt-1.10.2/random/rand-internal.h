/* rand-internal.h - header to glue the random functions
 *	Copyright (C) 1998, 2002 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef G10_RAND_INTERNAL_H
#define G10_RAND_INTERNAL_H

#include "../src/cipher-proto.h"

/* Constants used to define the origin of random added to the pool.
   The code is sensitive to the order of the values.  */
enum random_origins
  {
    RANDOM_ORIGIN_INIT = 0,      /* Used only for initialization. */
    RANDOM_ORIGIN_EXTERNAL = 1,  /* Added from an external source.  */
    RANDOM_ORIGIN_FASTPOLL = 2,  /* Fast random poll function.  */
    RANDOM_ORIGIN_SLOWPOLL = 3,  /* Slow poll function.  */
    RANDOM_ORIGIN_EXTRAPOLL = 4  /* Used to mark an extra pool seed
                                    due to a GCRY_VERY_STRONG_RANDOM
                                    random request.  */
  };

#define RANDOM_CONF_DISABLE_JENT 1
#define RANDOM_CONF_ONLY_URANDOM 2


/*-- random.c --*/
unsigned int _gcry_random_read_conf (void);
void _gcry_random_progress (const char *what, int printchar,
                            int current, int total);


/*-- random-csprng.c --*/
void _gcry_rngcsprng_initialize (int full);
void _gcry_rngcsprng_close_fds (void);
void _gcry_rngcsprng_dump_stats (void);
void _gcry_rngcsprng_secure_alloc (void);
void _gcry_rngcsprng_enable_quick_gen (void);
int  _gcry_rngcsprng_is_faked (void);
gcry_error_t _gcry_rngcsprng_add_bytes (const void *buf, size_t buflen,
                                        int quality);
void *_gcry_rngcsprng_get_bytes (size_t nbytes,
                                 enum gcry_random_level level);
void *_gcry_rngcsprng_get_bytes_secure (size_t nbytes,
                                        enum gcry_random_level level);
void _gcry_rngcsprng_randomize (void *buffer, size_t length,
                                enum gcry_random_level level);
void _gcry_rngcsprng_set_seed_file (const char *name);
void _gcry_rngcsprng_update_seed_file (void);
void _gcry_rngcsprng_fast_poll (void);

/*-- random-drbg.c --*/
void _gcry_rngdrbg_inititialize (int full);
void _gcry_rngdrbg_close_fds (void);
void _gcry_rngdrbg_dump_stats (void);
int  _gcry_rngdrbg_is_faked (void);
gcry_error_t _gcry_rngdrbg_add_bytes (const void *buf, size_t buflen,
                                      int quality);
void _gcry_rngdrbg_randomize (void *buffer, size_t length,
                              enum gcry_random_level level);
gcry_error_t _gcry_rngdrbg_selftest (selftest_report_func_t report);

/*-- random-system.c --*/
void _gcry_rngsystem_initialize (int full);
void _gcry_rngsystem_close_fds (void);
void _gcry_rngsystem_dump_stats (void);
int  _gcry_rngsystem_is_faked (void);
gcry_error_t _gcry_rngsystem_add_bytes (const void *buf, size_t buflen,
                                        int quality);
void _gcry_rngsystem_randomize (void *buffer, size_t length,
                                enum gcry_random_level level);



/*-- rndgetentropy.c --*/
int _gcry_rndgetentropy_gather_random (void (*add) (const void *, size_t,
                                                    enum random_origins),
                                       enum random_origins origin,
                                       size_t length, int level);

/*-- rndoldlinux.c --*/
int _gcry_rndoldlinux_gather_random (void (*add) (const void *, size_t,
                                                  enum random_origins),
                                     enum random_origins origin,
                                     size_t length, int level);

/*-- rndunix.c --*/
int _gcry_rndunix_gather_random (void (*add) (const void *, size_t,
                                              enum random_origins),
                                 enum random_origins origin,
                                 size_t length, int level);

/*-- rndegd.c --*/
int _gcry_rndegd_gather_random (void (*add) (const void *, size_t,
                                             enum random_origins),
                                enum random_origins origin,
                                size_t length, int level);
int _gcry_rndegd_connect_socket (int nofail);

/*-- rndw32.c --*/
int _gcry_rndw32_gather_random (void (*add) (const void *, size_t,
                                             enum random_origins),
                                enum random_origins origin,
                                size_t length, int level);
void _gcry_rndw32_gather_random_fast (void (*add)(const void*, size_t,
                                                  enum random_origins),
                                      enum random_origins origin );

/*-- rndw32ce.c --*/
int _gcry_rndw32ce_gather_random (void (*add) (const void *, size_t,
                                               enum random_origins),
                                  enum random_origins origin,
                                  size_t length, int level);
void _gcry_rndw32ce_gather_random_fast (void (*add)(const void*, size_t,
                                                    enum random_origins),
                                        enum random_origins origin );

/*-- rndjent.c --*/
size_t _gcry_rndjent_poll (void (*add)(const void*,
                                       size_t, enum random_origins),
                           enum random_origins origin,
                           size_t length);
void _gcry_rndjent_dump_stats (void);
void _gcry_rndjent_fini (void);

/*-- rndhw.c --*/
int _gcry_rndhw_failed_p (void);
void _gcry_rndhw_poll_fast (void (*add)(const void*, size_t,
                                        enum random_origins),
                            enum random_origins origin);
size_t _gcry_rndhw_poll_slow (void (*add)(const void*, size_t,
                                          enum random_origins),
                              enum random_origins origin, size_t req_length);



#endif /*G10_RAND_INTERNAL_H*/
