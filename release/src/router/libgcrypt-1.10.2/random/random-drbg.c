/* random-drbg.c - Deterministic Random Bits Generator
 * Copyright 2014 Stephan Mueller <smueller@chronox.de>
 *
 * DRBG: Deterministic Random Bits Generator
 *       Based on NIST Recommended DRBG from NIST SP800-90A with the following
 *       properties:
 *		* CTR DRBG with DF with AES-128, AES-192, AES-256 cores
 * 		* Hash DRBG with DF with SHA-1, SHA-256, SHA-384, SHA-512 cores
 * 		* HMAC DRBG with DF with SHA-1, SHA-256, SHA-384, SHA-512 cores
 * 		* with and without prediction resistance
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, and the entire permission notice in its entirety,
 *    including the disclaimer of warranties.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * ALTERNATIVELY, this product may be distributed under the terms of
 * LGPLv2+, in which case the provisions of the LGPL are
 * required INSTEAD OF the above restrictions.  (This clause is
 * necessary due to a potential bad interaction between the LGPL and
 * the restrictions contained in a BSD-style copyright.)
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ALL OF
 * WHICH ARE HEREBY DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF NOT ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 *
 * gcry_control GCRYCTL_DRBG_REINIT
 * ================================
 * This control request re-initializes the DRBG completely, i.e. the entire
 * state of the DRBG is zeroized (with two exceptions listed in
 * GCRYCTL_DRBG_SET_ENTROPY).
 *
 * The control request takes the following values which influences how
 * the DRBG is re-initialized:
 *
 *  - const char *flagstr
 *
 *      This variable specifies the DRBG type to be used for the next
 *	initialization.  If set to NULL, the previous DRBG type is
 *	used for the initialization.  If not NULL a space separated
 *	list of tokens with associated flag values is expected which
 *	are ORed to form the mandatory flags of the requested DRBG
 *	strength and cipher type.  Optionally, the prediction
 *	resistance flag can be ORed into the flags variable.
 *
 *      | String token | Flag value             |
 *      |--------------+------------------------|
 *      | aes          | DRBG_CTRAES            |
 *      | serpent      | DRBG_CTRSERPENT        |
 *      | twofish      | DRBG_CTRTWOFISH        |
 *      | sha1         | DRBG_HASHSHA1          |
 *      | sha256       | DRBG_HASHSHA256        |
 *      | sha512       | DRBG_HASHSHA512        |
 *      | hmac         | DRBG_HMAC              |
 *      | sym128       | DRBG_SYM128            |
 *      | sym192       | DRBG_SYM192            |
 *      | sym256       | DRBG_SYM256            |
 *      | pr           | DRBG_PREDICTION_RESIST |
 *
 *    For example:
 *
 * 	- CTR-DRBG with AES-128 without prediction resistance:
 * 	    "aes sym128"
 * 	- HMAC-DRBG with SHA-512 with prediction resistance:
 * 	    "hmac sha512 pr"
 *
 *  - gcry_buffer_t *pers
 *
 *      NULL terminated array with personalization strings to be used
 *	for initialization.
 *
 *  - int npers
 *
 *     Size of PERS.
 *
 *  - void *guard
 *
 *      A value of NULL must be passed for this.
 *
 * The variable of flags is independent from the pers/perslen variables. If
 * flags is set to 0 and perslen is set to 0, the current DRBG type is
 * completely reset without using a personalization string.
 *
 * DRBG Usage
 * ==========
 * The SP 800-90A DRBG allows the user to specify a personalization string
 * for initialization as well as an additional information string for each
 * random number request.  The following code fragments show how a caller
 * uses the API to use the full functionality of the DRBG.
 *
 * Usage without any additional data
 * ---------------------------------
 * gcry_randomize(outbuf, OUTLEN, GCRY_STRONG_RANDOM);
 *
 *
 * Usage with personalization string during initialization
 * -------------------------------------------------------
 * drbg_string_t pers;
 * char personalization[11] = "some-string";
 *
 * drbg_string_fill(&pers, personalization, strlen(personalization));
 * // The reset completely re-initializes the DRBG with the provided
 * // personalization string without changing the DRBG type
 * ret = gcry_control(GCRYCTL_DRBG_REINIT, 0, &pers);
 * gcry_randomize(outbuf, OUTLEN, GCRY_STRONG_RANDOM);
 *
 *
 * Usage with additional information string during random number request
 * ---------------------------------------------------------------------
 * drbg_string_t addtl;
 * char addtl_string[11] = "some-string";
 *
 * drbg_string_fill(&addtl, addtl_string, strlen(addtl_string));
 * // The following call is a wrapper to gcry_randomize() and returns
 * // the same error codes.
 * gcry_randomize_drbg(outbuf, OUTLEN, GCRY_STRONG_RANDOM, &addtl);
 *
 *
 * Usage with personalization and additional information strings
 * -------------------------------------------------------------
 * Just mix both scenarios above.
 *
 *
 * Switch the DRBG type to some other type
 * ---------------------------------------
 * // Switch to CTR DRBG AES-128 without prediction resistance
 * ret = gcry_control(GCRYCTL_DRBG_REINIT, DRBG_NOPR_CTRAES128, NULL);
 * gcry_randomize(outbuf, OUTLEN, GCRY_STRONG_RANDOM);
 */

#include <config.h>

#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "g10lib.h"
#include "random.h"
#include "rand-internal.h"
#include "../cipher/bufhelp.h"



/******************************************************************
 * Constants
 ******************************************************************/

/*
 * DRBG flags bitmasks
 *
 * 31 (B) 28      19         (A)         0
 *  +-+-+-+--------+---+-----------+-----+
 *  |~|~|u|~~~~~~~~| 3 |     2     |  1  |
 *  +-+-+-+--------+- -+-----------+-----+
 * ctl flg|        |drbg use selection flags
 *
 */

/* Internal state control flags (B) */
#define DRBG_PREDICTION_RESIST	((u32)1<<28)

/* CTR type modifiers (A.1)*/
#define DRBG_CTRAES		((u32)1<<0)
#define DRBG_CTRSERPENT		((u32)1<<1)
#define DRBG_CTRTWOFISH		((u32)1<<2)
#define DRBG_CTR_MASK	        (DRBG_CTRAES | DRBG_CTRSERPENT \
                                 | DRBG_CTRTWOFISH)

/* HASH type modifiers (A.2)*/
#define DRBG_HASHSHA1		((u32)1<<4)
#define DRBG_HASHSHA224		((u32)1<<5)
#define DRBG_HASHSHA256		((u32)1<<6)
#define DRBG_HASHSHA512		((u32)1<<8)
#define DRBG_HASH_MASK		(DRBG_HASHSHA1 | DRBG_HASHSHA224 \
				 | DRBG_HASHSHA256 | DRBG_HASHSHA512)
/* type modifiers (A.3)*/
#define DRBG_HMAC		((u32)1<<12)
#define DRBG_SYM128		((u32)1<<13)
#define DRBG_SYM192		((u32)1<<14)
#define DRBG_SYM256		((u32)1<<15)
#define DRBG_TYPE_MASK		(DRBG_HMAC | DRBG_SYM128 | DRBG_SYM192 \
				 | DRBG_SYM256)
#define DRBG_CIPHER_MASK        (DRBG_CTR_MASK | DRBG_HASH_MASK \
                                 | DRBG_TYPE_MASK)

#define DRBG_PR_CTRAES128   (DRBG_PREDICTION_RESIST | DRBG_CTRAES | DRBG_SYM128)
#define DRBG_PR_CTRAES192   (DRBG_PREDICTION_RESIST | DRBG_CTRAES | DRBG_SYM192)
#define DRBG_PR_CTRAES256   (DRBG_PREDICTION_RESIST | DRBG_CTRAES | DRBG_SYM256)
#define DRBG_NOPR_CTRAES128 (DRBG_CTRAES | DRBG_SYM128)
#define DRBG_NOPR_CTRAES192 (DRBG_CTRAES | DRBG_SYM192)
#define DRBG_NOPR_CTRAES256 (DRBG_CTRAES | DRBG_SYM256)
#define DRBG_PR_HASHSHA1     (DRBG_PREDICTION_RESIST | DRBG_HASHSHA1)
#define DRBG_PR_HASHSHA256   (DRBG_PREDICTION_RESIST | DRBG_HASHSHA256)
#define DRBG_PR_HASHSHA512   (DRBG_PREDICTION_RESIST | DRBG_HASHSHA512)
#define DRBG_NOPR_HASHSHA1   (DRBG_HASHSHA1)
#define DRBG_NOPR_HASHSHA256 (DRBG_HASHSHA256)
#define DRBG_NOPR_HASHSHA512 (DRBG_HASHSHA512)
#define DRBG_PR_HMACSHA1     (DRBG_PREDICTION_RESIST | DRBG_HASHSHA1 \
                              | DRBG_HMAC)
#define DRBG_PR_HMACSHA256   (DRBG_PREDICTION_RESIST | DRBG_HASHSHA256 \
                              | DRBG_HMAC)
#define DRBG_PR_HMACSHA512   (DRBG_PREDICTION_RESIST | DRBG_HASHSHA512 \
                              | DRBG_HMAC)
#define DRBG_NOPR_HMACSHA1   (DRBG_HASHSHA1 | DRBG_HMAC)
#define DRBG_NOPR_HMACSHA256 (DRBG_HASHSHA256 | DRBG_HMAC)
#define DRBG_NOPR_HMACSHA512 (DRBG_HASHSHA512 | DRBG_HMAC)


/* The default DRGB type.  */
#define DRBG_DEFAULT_TYPE    DRBG_NOPR_HMACSHA256


#define DRBG_CTR_NULL_LEN 128


/******************************************************************
 * Common data structures
 ******************************************************************/

/*
 * SP800-90A requires the concatenation of different data. To avoid copying
 * buffers around or allocate additional memory, the following data structure
 * is used to point to the original memory with its size. In addition, it
 * is used to build a linked list. The linked list defines the concatenation
 * of individual buffers. The order of memory block referenced in that
 * linked list determines the order of concatenation.
 */
struct drbg_string_s
{
  const unsigned char *buf;
  size_t len;
  struct drbg_string_s *next;
};
typedef struct drbg_string_s drbg_string_t;


/* DRBG input data structure for DRBG generate with additional
 * information string.  */
struct drbg_gen_s
{
  unsigned char *outbuf;	/* output buffer for random numbers */
  unsigned int outlen;	        /* size of output buffer */
  drbg_string_t *addtl;	        /* input buffer for
				 * additional information string */
};
typedef struct drbg_gen_s drbg_gen_t;


/* Forward declaration of the state object pointer.  */
struct drbg_state_s;
typedef struct drbg_state_s *drbg_state_t;


struct drbg_core_s
{
  u32 flags;			/* flags for the cipher */
  ushort statelen;		/* maximum state length */
  ushort blocklen_bytes;	/* block size of output in bytes */
  int backend_cipher;		/* libgcrypt backend cipher */
};

struct drbg_state_ops_s
{
  gpg_err_code_t (*update) (drbg_state_t drbg,
			    drbg_string_t *seed, int reseed);
  gpg_err_code_t (*generate) (drbg_state_t drbg,
			      unsigned char *buf, unsigned int buflen,
			      drbg_string_t *addtl);
  gpg_err_code_t (*crypto_init) (drbg_state_t drbg);
  void		 (*crypto_fini) (drbg_state_t drbg);
};

struct drbg_test_data_s
{
  drbg_string_t *testentropy;	/* TEST PARAMETER: test entropy */
  int fail_seed_source:1;	/* If set, the seed function will
                                 * return an error. */
};


/* This state object keeps the state of an DRBG instance.  */
struct drbg_state_s
{
  unsigned char *V;		/* internal state 10.1.1.1 1a) */
  unsigned char *C;		/* hash: static value 10.1.1.1 1b)
				 * hmac / ctr: key */
  size_t reseed_ctr;		/* Number of RNG requests since last reseed --
				 * 10.1.1.1 1c) */
  unsigned char *scratchpad;	/* some memory the DRBG can use for its
				 * operation -- allocated during init */
  void *priv_data;		/* Cipher handle */
  gcry_cipher_hd_t ctr_handle;	/* CTR mode cipher handle */
  int seeded:1;			/* DRBG fully seeded? */
  int pr:1;			/* Prediction resistance enabled? */
  /* Taken from libgcrypt ANSI X9.31 DRNG: We need to keep track of the
   * process which did the initialization so that we can detect a fork.
   * The volatile modifier is required so that the compiler does not
   * optimize it away in case the getpid function is badly attributed. */
  pid_t seed_init_pid;
  const struct drbg_state_ops_s *d_ops;
  const struct drbg_core_s *core;
  struct drbg_test_data_s *test_data;
};

enum drbg_prefixes
{
  DRBG_PREFIX0 = 0x00,
  DRBG_PREFIX1,
  DRBG_PREFIX2,
  DRBG_PREFIX3
};

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/***************************************************************
 * Global variables
 ***************************************************************/

/* The instance of the DRBG, to be refereed by drbg_state.  */
static struct drbg_state_s drbg_instance;

/* Global state variable holding the current instance of the DRBG.  */
static drbg_state_t drbg_state;

/* This is the lock variable we use to serialize access to this RNG. */
GPGRT_LOCK_DEFINE(drbg_lock_var);


/***************************************************************
 * Backend cipher definitions available to DRBG
 ***************************************************************/

static const struct drbg_core_s drbg_cores[] = {
  /* Hash DRBGs */
  {DRBG_HASHSHA1, 55, 20, GCRY_MD_SHA1},
  {DRBG_HASHSHA256, 55, 32, GCRY_MD_SHA256},
  {DRBG_HASHSHA512, 111, 64, GCRY_MD_SHA512},
  /* HMAC DRBGs */
  {DRBG_HASHSHA1   | DRBG_HMAC, 20, 20, GCRY_MD_SHA1},
  {DRBG_HASHSHA256 | DRBG_HMAC, 32, 32, GCRY_MD_SHA256},
  {DRBG_HASHSHA512 | DRBG_HMAC, 64, 64, GCRY_MD_SHA512},
  /* block ciphers */
  {DRBG_CTRAES | DRBG_SYM128, 32, 16, GCRY_CIPHER_AES128},
  {DRBG_CTRAES | DRBG_SYM192, 40, 16, GCRY_CIPHER_AES192},
  {DRBG_CTRAES | DRBG_SYM256, 48, 16, GCRY_CIPHER_AES256}
};

static gpg_err_code_t drbg_hash_init (drbg_state_t drbg);
static gpg_err_code_t drbg_hmac_init (drbg_state_t drbg);
static gpg_err_code_t drbg_hmac_setkey (drbg_state_t drbg,
					const unsigned char *key);
static void drbg_hash_fini (drbg_state_t drbg);
static byte *drbg_hash (drbg_state_t drbg, const drbg_string_t *buf);
static gpg_err_code_t drbg_sym_init (drbg_state_t drbg);
static void drbg_sym_fini (drbg_state_t drbg);
static gpg_err_code_t drbg_sym_setkey (drbg_state_t drbg,
				       const unsigned char *key);
static gpg_err_code_t drbg_sym (drbg_state_t drbg, unsigned char *outval,
				const drbg_string_t *buf);
static gpg_err_code_t drbg_sym_ctr (drbg_state_t drbg,
			const unsigned char *inbuf, unsigned int inbuflen,
			unsigned char *outbuf, unsigned int outbuflen);

/******************************************************************
 ******************************************************************
 ******************************************************************
 * Generic DRBG code
 ******************************************************************
 ******************************************************************
 ******************************************************************/

/******************************************************************
 * Generic helper functions
 ******************************************************************/

#if 0
#define dbg(x) do { log_debug x; } while(0)
#else
#define dbg(x)
#endif

/*
 * Parse a string of flags and store the flag values at R_FLAGS.
 * Return 0 on success.
 */
static gpg_err_code_t
parse_flag_string (const char *string, u32 *r_flags)
{
  struct {
    const char *name;
    u32 flag;
  } table[] = {
    { "aes",     DRBG_CTRAES            },
    { "serpent", DRBG_CTRSERPENT        },
    { "twofish", DRBG_CTRTWOFISH        },
    { "sha1",    DRBG_HASHSHA1          },
    { "sha256",  DRBG_HASHSHA256        },
    { "sha512",  DRBG_HASHSHA512        },
    { "hmac",    DRBG_HMAC              },
    { "sym128",  DRBG_SYM128            },
    { "sym192",  DRBG_SYM192            },
    { "sym256",  DRBG_SYM256            },
    { "pr",      DRBG_PREDICTION_RESIST }
  };

  *r_flags = 0;
  if (string)
    {
      char **tl;
      const char *s;
      int i, j;

      tl = _gcry_strtokenize (string, NULL);
      if (!tl)
        return gpg_err_code_from_syserror ();
      for (i=0; (s=tl[i]); i++)
        {
          for (j=0; j < DIM (table); j++)
            if (!strcmp (s, table[j].name))
              {
                *r_flags |= table[j].flag;
                break;
              }
          if (!(j < DIM (table)))
            {
              xfree (tl);
              return GPG_ERR_INV_FLAG;
            }
        }
      xfree (tl);
    }

  return 0;
}

static inline void
drbg_string_fill (drbg_string_t *string,
                       const unsigned char *buf, size_t len)
{
  string->buf = buf;
  string->len = len;
  string->next = NULL;
}

static inline ushort
drbg_statelen (drbg_state_t drbg)
{
  if (drbg && drbg->core)
    return drbg->core->statelen;
  return 0;
}

static inline ushort
drbg_blocklen (drbg_state_t drbg)
{
  if (drbg && drbg->core)
    return drbg->core->blocklen_bytes;
  return 0;
}

static inline ushort
drbg_keylen (drbg_state_t drbg)
{
  if (drbg && drbg->core)
    return (drbg->core->statelen - drbg->core->blocklen_bytes);
  return 0;
}

static inline size_t
drbg_max_request_bytes (void)
{
  /* SP800-90A requires the limit 2**19 bits, but we return bytes */
  return (1 << 16);
}

static inline size_t
drbg_max_addtl (void)
{
  /* SP800-90A requires 2**35 bytes additional info str / pers str */
#ifdef __LP64__
  return (1UL << 35);
#else
  /*
   * SP800-90A allows smaller maximum numbers to be returned -- we
   * return SIZE_MAX - 1 to allow the verification of the enforcement
   * of this value in drbg_healthcheck_sanity.
   */
  return (SIZE_MAX - 1);
#endif
}

static inline size_t
drbg_max_requests (void)
{
  /* SP800-90A requires 2**48 maximum requests before reseeding */
#ifdef __LP64__
  return (1UL << 48);
#else
  return SIZE_MAX;
#endif
}

/*
 * Return strength of DRBG according to SP800-90A section 8.4
 *
 * flags: DRBG flags reference
 *
 * Return: normalized strength value or 32 as a default to counter
 * 	   programming errors
 */
static inline unsigned short
drbg_sec_strength (u32 flags)
{
  if ((flags & DRBG_HASHSHA1) || (flags & DRBG_SYM128))
    return 16;
  else if (flags & DRBG_SYM192)
    return 24;
  else if ((flags & DRBG_SYM256) || (flags & DRBG_HASHSHA256) ||
	   (flags & DRBG_HASHSHA512))
    return 32;
  else
    return 32;
}

static void
drbg_add_buf (unsigned char *dst, size_t dstlen,
              unsigned char *add, size_t addlen)
{
  /* implied: dstlen > addlen */
  unsigned char *dstptr, *addptr;
  unsigned int remainder = 0;
  size_t len = addlen;

  dstptr = dst + (dstlen - 1);
  addptr = add + (addlen - 1);
  while (len)
    {
      remainder += *dstptr + *addptr;
      *dstptr = remainder & 0xff;
      remainder >>= 8;
      len--;
      dstptr--;
      addptr--;
    }
  len = dstlen - addlen;
  while (len && remainder > 0)
    {
      remainder = *dstptr + 1;
      *dstptr = remainder & 0xff;
      remainder >>= 8;
      len--;
      dstptr--;
    }
}

/* Helper variables for read_cb().
 *
 *   The _gcry_rnd*_gather_random interface does not allow to provide a
 *   data pointer.  Thus we need to use a global variable for
 *   communication.  However, the then required locking is anyway a good
 *   idea because it does not make sense to have several readers of (say
 *   /dev/random).  It is easier to serve them one after the other.
 */
static unsigned char *read_cb_buffer;	/* The buffer.  */
static size_t read_cb_size;	        /* Size of the buffer.  */
static size_t read_cb_len;	        /* Used length.  */

/* Callback for generating seed from kernel device. */
static void
drbg_read_cb (const void *buffer, size_t length,
              enum random_origins origin)
{
  const unsigned char *p = buffer;

  (void) origin;
  gcry_assert (read_cb_buffer);

  /* Note that we need to protect against gatherers returning more
   * than the requested bytes (e.g. rndw32).  */
  while (length-- && read_cb_len < read_cb_size)
    read_cb_buffer[read_cb_len++] = *p++;
}

static inline int
drbg_get_entropy (drbg_state_t drbg, unsigned char *buffer,
		       size_t len)
{
  int rc = 0;

  /* Perform testing as defined in 11.3.2 */
  if (drbg->test_data && drbg->test_data->fail_seed_source)
    return -1;

  read_cb_buffer = buffer;
  read_cb_size = len;
  read_cb_len = 0;
#if USE_RNDGETENTROPY
  rc = _gcry_rndgetentropy_gather_random (drbg_read_cb, 0, len,
                                          GCRY_VERY_STRONG_RANDOM);
#elif USE_RNDOLDLINUX
  rc = _gcry_rndoldlinux_gather_random (drbg_read_cb, 0, len,
                                        GCRY_VERY_STRONG_RANDOM);
#elif USE_RNDUNIX
  rc = _gcry_rndunix_gather_random (drbg_read_cb, 0, len,
				    GCRY_VERY_STRONG_RANDOM);
#elif USE_RNDW32
  do
    {
      rc = _gcry_rndw32_gather_random (drbg_read_cb, 0, len,
				       GCRY_VERY_STRONG_RANDOM);
    }
  while (rc >= 0 && read_cb_len < read_cb_size);
#else
  rc = -1;
#endif
  return rc;
}

/******************************************************************
 * CTR DRBG callback functions
 ******************************************************************/

/* BCC function for CTR DRBG as defined in 10.4.3 */
static gpg_err_code_t
drbg_ctr_bcc (drbg_state_t drbg,
              unsigned char *out, const unsigned char *key,
              drbg_string_t *in)
{
  gpg_err_code_t ret = GPG_ERR_GENERAL;
  drbg_string_t *curr = in;
  size_t inpos = curr->len;
  const unsigned char *pos = curr->buf;
  drbg_string_t data;

  drbg_string_fill (&data, out, drbg_blocklen (drbg));

  /* 10.4.3 step 1 */
  memset (out, 0, drbg_blocklen (drbg));

  ret = drbg_sym_setkey(drbg, key);
  if (ret)
    return ret;

  /* 10.4.3 step 2 / 4 */
  while (inpos)
    {
      short cnt = 0;
      /* 10.4.3 step 4.1 */
      for (cnt = 0; cnt < drbg_blocklen (drbg); cnt++)
	{
	  out[cnt] ^= *pos;
	  pos++;
	  inpos--;
	  /* the following branch implements the linked list
	   * iteration. If we are at the end of the current data
	   * set, we have to start using the next data set if
	   * available -- the inpos value always points to the
	   * current byte and will be zero if we have processed
	   * the last byte of the last linked list member */
	  if (0 == inpos)
	    {
	      curr = curr->next;
	      if (NULL != curr)
		{
		  pos = curr->buf;
		  inpos = curr->len;
		}
	      else
		{
		  inpos = 0;
		  break;
		}
	    }
	}
      /* 10.4.3 step 4.2 */
      ret = drbg_sym (drbg, out, &data);
      if (ret)
	return ret;
      /* 10.4.3 step 2 */
    }
  return 0;
}


/*
 * scratchpad usage: drbg_ctr_update is interlinked with drbg_ctr_df
 * (and drbg_ctr_bcc, but this function does not need any temporary buffers),
 * the scratchpad is used as follows:
 * drbg_ctr_update:
 *	temp
 *		start: drbg->scratchpad
 *		length: drbg_statelen(drbg) + drbg_blocklen(drbg)
 *			note: the cipher writing into this variable works
 *			blocklen-wise. Now, when the statelen is not a multiple
 *			of blocklen, the generateion loop below "spills over"
 *			by at most blocklen. Thus, we need to give sufficient
 *			memory.
 *	df_data
 *		start: drbg->scratchpad +
 *				drbg_statelen(drbg) +
 *				drbg_blocklen(drbg)
 *		length: drbg_statelen(drbg)
 *
 * drbg_ctr_df:
 *	pad
 *		start: df_data + drbg_statelen(drbg)
 *		length: drbg_blocklen(drbg)
 *	iv
 *		start: pad + drbg_blocklen(drbg)
 *		length: drbg_blocklen(drbg)
 *	temp
 *		start: iv + drbg_blocklen(drbg)
 *		length: drbg_satelen(drbg) + drbg_blocklen(drbg)
 *			note: temp is the buffer that the BCC function operates
 *			on. BCC operates blockwise. drbg_statelen(drbg)
 *			is sufficient when the DRBG state length is a multiple
 *			of the block size. For AES192 (and maybe other ciphers)
 *			this is not correct and the length for temp is
 *			insufficient (yes, that also means for such ciphers,
 *			the final output of all BCC rounds are truncated).
 *			Therefore, add drbg_blocklen(drbg) to cover all
 *			possibilities.
 */

/* Derivation Function for CTR DRBG as defined in 10.4.2 */
static gpg_err_code_t
drbg_ctr_df (drbg_state_t drbg, unsigned char *df_data,
             size_t bytes_to_return, drbg_string_t *addtl)
{
  gpg_err_code_t ret = GPG_ERR_GENERAL;
  unsigned char L_N[8];
  /* S3 is input */
  drbg_string_t S1, S2, S4, cipherin;
  drbg_string_t *tempstr = addtl;
  unsigned char *pad = df_data + drbg_statelen (drbg);
  unsigned char *iv = pad + drbg_blocklen (drbg);
  unsigned char *temp = iv + drbg_blocklen (drbg);
  size_t padlen = 0;
  unsigned int templen = 0;
  /* 10.4.2 step 7 */
  unsigned int i = 0;
  /* 10.4.2 step 8 */
  const unsigned char *K = (unsigned char *)
    "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
    "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f";
  unsigned char *X;
  size_t generated_len = 0;
  size_t inputlen = 0;

  memset (pad, 0, drbg_blocklen (drbg));
  memset (iv, 0, drbg_blocklen (drbg));
  memset (temp, 0, drbg_statelen (drbg));

  /* 10.4.2 step 1 is implicit as we work byte-wise */

  /* 10.4.2 step 2 */
  if ((512 / 8) < bytes_to_return)
    return GPG_ERR_INV_ARG;

  /* 10.4.2 step 2 -- calculate the entire length of all input data */
  for (; NULL != tempstr; tempstr = tempstr->next)
    inputlen += tempstr->len;
  buf_put_be32 (&L_N[0], inputlen);

  /* 10.4.2 step 3 */
  buf_put_be32 (&L_N[4], bytes_to_return);

  /* 10.4.2 step 5: length is size of L_N, input_string, one byte, padding */
  padlen = (inputlen + sizeof (L_N) + 1) % (drbg_blocklen (drbg));
  /* wrap the padlen appropriately */
  if (padlen)
    padlen = drbg_blocklen (drbg) - padlen;
  /* pad / padlen contains the 0x80 byte and the following zero bytes, so
   * add one for byte for 0x80 */
  padlen++;
  pad[0] = 0x80;

  /* 10.4.2 step 4 -- first fill the linked list and then order it */
  drbg_string_fill (&S1, iv, drbg_blocklen (drbg));
  drbg_string_fill (&S2, L_N, sizeof (L_N));
  drbg_string_fill (&S4, pad, padlen);
  S1.next = &S2;
  S2.next = addtl;

  /* Splice in addtl between S2 and S4 -- we place S4 at the end of the
   * input data chain. As this code is only triggered when addtl is not
   * NULL, no NULL checks are necessary.*/
  tempstr = addtl;
  while (tempstr->next)
    tempstr = tempstr->next;
  tempstr->next = &S4;

  /* 10.4.2 step 9 */
  while (templen < (drbg_keylen (drbg) + (drbg_blocklen (drbg))))
    {
      /* 10.4.2 step 9.1 - the padding is implicit as the buffer
       * holds zeros after allocation -- even the increment of i
       * is irrelevant as the increment remains within length of i */
      buf_put_be32 (iv, i);
      /* 10.4.2 step 9.2 -- BCC and concatenation with temp */
      ret = drbg_ctr_bcc (drbg, temp + templen, K, &S1);
      if (ret)
	goto out;
      /* 10.4.2 step 9.3 */
      i++;
      templen += drbg_blocklen (drbg);
    }

  /* 10.4.2 step 11 */
  /* implicit key len with seedlen - blocklen according to table 3 */
  X = temp + (drbg_keylen (drbg));
  drbg_string_fill (&cipherin, X, drbg_blocklen (drbg));

  /* 10.4.2 step 12: overwriting of outval */

  /* 10.4.2 step 13 */
  ret = drbg_sym_setkey(drbg, temp);
  if (ret)
    goto out;
  while (generated_len < bytes_to_return)
    {
      short blocklen = 0;
      /* 10.4.2 step 13.1 */
      /* the truncation of the key length is implicit as the key
       * is only drbg_blocklen in size -- check for the implementation
       * of the cipher function callback */
      ret = drbg_sym (drbg, X, &cipherin);
      if (ret)
	goto out;
      blocklen = (drbg_blocklen (drbg) < (bytes_to_return - generated_len)) ?
	drbg_blocklen (drbg) : (bytes_to_return - generated_len);
      /* 10.4.2 step 13.2 and 14 */
      memcpy (df_data + generated_len, X, blocklen);
      generated_len += blocklen;
    }

  ret = 0;

 out:
  memset (iv, 0, drbg_blocklen (drbg));
  memset (temp, 0, drbg_statelen (drbg));
  memset (pad, 0, drbg_blocklen (drbg));
  return ret;
}

/*
 * Update function of CTR DRBG as defined in 10.2.1.2
 *
 * The reseed variable has an enhanced meaning compared to the update
 * functions of the other DRBGs as follows:
 * 0 => initial seed from initialization
 * 1 => reseed via drbg_seed
 * 2 => first invocation from drbg_ctr_update when addtl is present. In
 *      this case, the df_data scratchpad is not deleted so that it is
 *      available for another calls to prevent calling the DF function
 *      again.
 * 3 => second invocation from drbg_ctr_update. When the update function
 *      was called with addtl, the df_data memory already contains the
 *      DFed addtl information and we do not need to call DF again.
 */
static gpg_err_code_t
drbg_ctr_update (drbg_state_t drbg, drbg_string_t *addtl, int reseed)
{
  gpg_err_code_t ret = GPG_ERR_GENERAL;
  /* 10.2.1.2 step 1 */
  unsigned char *temp = drbg->scratchpad;
  unsigned char *df_data = drbg->scratchpad +
    drbg_statelen (drbg) + drbg_blocklen (drbg);
  unsigned char prefix = DRBG_PREFIX1;

  memset (temp, 0, drbg_statelen (drbg) + drbg_blocklen (drbg));
  if (3 > reseed)
    memset (df_data, 0, drbg_statelen (drbg));

  if (!reseed)
    {
      /*
       * The DRBG uses the CTR mode of the underlying AES cipher. The
       * CTR mode increments the counter value after the AES operation
       * but SP800-90A requires that the counter is incremented before
       * the AES operation. Hence, we increment it at the time we set
       * it by one.
       */
      drbg_add_buf (drbg->V, drbg_blocklen (drbg), &prefix, 1);

      ret = _gcry_cipher_setkey (drbg->ctr_handle, drbg->C, drbg_keylen (drbg));
      if (ret)
        goto out;
    }

  /* 10.2.1.3.2 step 2 and 10.2.1.4.2 step 2 */
  if (addtl && 0 < addtl->len)
    {
      ret =
	drbg_ctr_df (drbg, df_data, drbg_statelen (drbg), addtl);
      if (ret)
	goto out;
    }

  ret = drbg_sym_ctr (drbg, df_data, drbg_statelen(drbg),
		      temp, drbg_statelen(drbg));
  if (ret)
    goto out;

  /* 10.2.1.2 step 5 */
  ret = _gcry_cipher_setkey (drbg->ctr_handle, temp, drbg_keylen (drbg));
  if (ret)
    goto out;

  /* 10.2.1.2 step 6 */
  memcpy (drbg->V, temp + drbg_keylen (drbg), drbg_blocklen (drbg));
  /* See above: increment counter by one to compensate timing of CTR op */
  drbg_add_buf (drbg->V, drbg_blocklen (drbg), &prefix, 1);
  ret = 0;

 out:
  memset (temp, 0, drbg_statelen (drbg) + drbg_blocklen (drbg));
  if (2 != reseed)
    memset (df_data, 0, drbg_statelen (drbg));
  return ret;
}

/*
 * scratchpad use: drbg_ctr_update is called independently from
 * drbg_ctr_extract_bytes. Therefore, the scratchpad is reused
 */
/* Generate function of CTR DRBG as defined in 10.2.1.5.2 */
static gpg_err_code_t
drbg_ctr_generate (drbg_state_t drbg,
                   unsigned char *buf, unsigned int buflen,
                   drbg_string_t *addtl)
{
  static const unsigned char drbg_ctr_null[DRBG_CTR_NULL_LEN] = { 0, };
  gpg_err_code_t ret = 0;

  memset (drbg->scratchpad, 0, drbg_blocklen (drbg));

  /* 10.2.1.5.2 step 2 */
  if (addtl && 0 < addtl->len)
    {
      addtl->next = NULL;
      ret = drbg_ctr_update (drbg, addtl, 2);
      if (ret)
	return ret;
    }

  /* 10.2.1.5.2 step 4.1 */
  ret = drbg_sym_ctr (drbg, drbg_ctr_null, sizeof(drbg_ctr_null), buf, buflen);
  if (ret)
    goto out;

  /* 10.2.1.5.2 step 6 */
  if (addtl)
    addtl->next = NULL;
  ret = drbg_ctr_update (drbg, addtl, 3);

 out:
  return ret;
}

static struct drbg_state_ops_s drbg_ctr_ops = {
  drbg_ctr_update,
  drbg_ctr_generate,
  drbg_sym_init,
  drbg_sym_fini,
};

/******************************************************************
 * HMAC DRBG callback functions
 ******************************************************************/

static gpg_err_code_t
drbg_hmac_update (drbg_state_t drbg, drbg_string_t *seed, int reseed)
{
  gpg_err_code_t ret = GPG_ERR_GENERAL;
  int i = 0;
  drbg_string_t seed1, seed2, cipherin;

  if (!reseed)
    {
      /* 10.1.2.3 step 2 already implicitly covered with
       * the initial memset(0) of drbg->C */
      memset (drbg->V, 1, drbg_statelen (drbg));
      ret = drbg_hmac_setkey (drbg, drbg->C);
      if (ret)
	return ret;
    }

  /* build linked list which implements the concatenation and fill
   * first part*/
  drbg_string_fill (&seed1, drbg->V, drbg_statelen (drbg));
  /* buffer will be filled in for loop below with one byte */
  drbg_string_fill (&seed2, NULL, 1);
  seed1.next = &seed2;
  /* seed may be NULL */
  seed2.next = seed;

  drbg_string_fill (&cipherin, drbg->V, drbg_statelen (drbg));
  /* we execute two rounds of V/K massaging */
  for (i = 2; 0 < i; i--)
    {
      byte *retval;
      /* first round uses 0x0, second 0x1 */
      unsigned char prefix = DRBG_PREFIX0;
      if (1 == i)
	prefix = DRBG_PREFIX1;
      /* 10.1.2.2 step 1 and 4 -- concatenation and HMAC for key */
      seed2.buf = &prefix;
      retval = drbg_hash (drbg, &seed1);
      ret = drbg_hmac_setkey (drbg, retval);
      if (ret)
	return ret;

      /* 10.1.2.2 step 2 and 5 -- HMAC for V */
      retval = drbg_hash (drbg, &cipherin);
      memcpy(drbg->V, retval, drbg_blocklen (drbg));

      /* 10.1.2.2 step 3 */
      if (!seed || 0 == seed->len)
	return ret;
    }
  return 0;
}

/* generate function of HMAC DRBG as defined in 10.1.2.5 */
static gpg_err_code_t
drbg_hmac_generate (drbg_state_t drbg, unsigned char *buf, unsigned int buflen,
                    drbg_string_t *addtl)
{
  gpg_err_code_t ret = 0;
  unsigned int len = 0;
  drbg_string_t data;

  /* 10.1.2.5 step 2 */
  if (addtl && 0 < addtl->len)
    {
      addtl->next = NULL;
      ret = drbg_hmac_update (drbg, addtl, 1);
      if (ret)
	return ret;
    }

  drbg_string_fill (&data, drbg->V, drbg_statelen (drbg));
  while (len < buflen)
    {
      unsigned int outlen = 0;
      /* 10.1.2.5 step 4.1 */
      byte *retval = drbg_hash (drbg, &data);
      memcpy(drbg->V, retval, drbg_blocklen (drbg));
      outlen = (drbg_blocklen (drbg) < (buflen - len)) ?
	drbg_blocklen (drbg) : (buflen - len);

      /* 10.1.2.5 step 4.2 */
      memcpy (buf + len, drbg->V, outlen);
      len += outlen;
    }

  /* 10.1.2.5 step 6 */
  if (addtl)
    addtl->next = NULL;
  ret = drbg_hmac_update (drbg, addtl, 1);

  return ret;
}

static struct drbg_state_ops_s drbg_hmac_ops = {
  drbg_hmac_update,
  drbg_hmac_generate,
  drbg_hmac_init,
  drbg_hash_fini,
};

/******************************************************************
 * Hash DRBG callback functions
 ******************************************************************/

/*
 * scratchpad usage: as drbg_hash_update and drbg_hash_df are used
 * interlinked, the scratchpad is used as follows:
 * drbg_hash_update
 *	start: drbg->scratchpad
 *	length: drbg_statelen(drbg)
 * drbg_hash_df:
 *	start: drbg->scratchpad + drbg_statelen(drbg)
 *	length: drbg_blocklen(drbg)
 */
/* Derivation Function for Hash DRBG as defined in 10.4.1 */
static gpg_err_code_t
drbg_hash_df (drbg_state_t drbg,
              unsigned char *outval, size_t outlen,
              drbg_string_t *entropy)
{
  size_t len = 0;
  unsigned char input[5];
  drbg_string_t data1;

  /* 10.4.1 step 3 */
  input[0] = 1;
  buf_put_be32 (&input[1], (outlen * 8));

  /* 10.4.1 step 4.1 -- concatenation of data for input into hash */
  drbg_string_fill (&data1, input, 5);
  data1.next = entropy;

  /* 10.4.1 step 4 */
  while (len < outlen)
    {
      short blocklen = 0;
      /* 10.4.1 step 4.1 */
      byte *retval = drbg_hash (drbg, &data1);
      /* 10.4.1 step 4.2 */
      input[0]++;
      blocklen = (drbg_blocklen (drbg) < (outlen - len)) ?
	drbg_blocklen (drbg) : (outlen - len);
      memcpy (outval + len, retval, blocklen);
      len += blocklen;
    }

  return 0;
}

/* update function for Hash DRBG as defined in 10.1.1.2 / 10.1.1.3 */
static gpg_err_code_t
drbg_hash_update (drbg_state_t drbg, drbg_string_t *seed, int reseed)
{
  gpg_err_code_t ret = 0;
  drbg_string_t data1, data2;
  unsigned char *V = drbg->scratchpad;
  unsigned char prefix = DRBG_PREFIX1;

  memset (drbg->scratchpad, 0, drbg_statelen (drbg));
  if (!seed)
    return GPG_ERR_INV_ARG;

  if (reseed)
    {
      /* 10.1.1.3 step 1: string length is concatenation of
       * 1 byte, V and seed (which is concatenated entropy/addtl
       * input)
       */
      memcpy (V, drbg->V, drbg_statelen (drbg));
      drbg_string_fill (&data1, &prefix, 1);
      drbg_string_fill (&data2, V, drbg_statelen (drbg));
      data1.next = &data2;
      data2.next = seed;
    }
  else
    {
      drbg_string_fill (&data1, seed->buf, seed->len);
      data1.next = seed->next;
    }

  /* 10.1.1.2 / 10.1.1.3 step 2 and 3 */
  ret = drbg_hash_df (drbg, drbg->V, drbg_statelen (drbg), &data1);
  if (ret)
    goto out;

  /* 10.1.1.2 / 10.1.1.3 step 4 -- concatenation  */
  prefix = DRBG_PREFIX0;
  drbg_string_fill (&data1, &prefix, 1);
  drbg_string_fill (&data2, drbg->V, drbg_statelen (drbg));
  data1.next = &data2;
  /* 10.1.1.2 / 10.1.1.3 step 4 -- df operation */
  ret = drbg_hash_df (drbg, drbg->C, drbg_statelen (drbg), &data1);

 out:
  memset (drbg->scratchpad, 0, drbg_statelen (drbg));
  return ret;
}

/* Processing of additional information string for Hash DRBG.  */
static gpg_err_code_t
drbg_hash_process_addtl (drbg_state_t drbg, drbg_string_t *addtl)
{
  drbg_string_t data1, data2;
  drbg_string_t *data3;
  unsigned char prefix = DRBG_PREFIX2;
  byte *retval;

  /* 10.1.1.4 step 2 */
  if (!addtl || 0 == addtl->len)
    return 0;

  /* 10.1.1.4 step 2a -- concatenation */
  drbg_string_fill (&data1, &prefix, 1);
  drbg_string_fill (&data2, drbg->V, drbg_statelen (drbg));
  data3 = addtl;
  data1.next = &data2;
  data2.next = data3;
  data3->next = NULL;
  /* 10.1.1.4 step 2a -- cipher invocation */
  retval = drbg_hash (drbg, &data1);

  /* 10.1.1.4 step 2b */
  drbg_add_buf (drbg->V, drbg_statelen (drbg), retval, drbg_blocklen (drbg));

  return 0;
}

/*
 * Hashgen defined in 10.1.1.4
 */
static gpg_err_code_t
drbg_hash_hashgen (drbg_state_t drbg, unsigned char *buf, unsigned int buflen)
{
  unsigned int len = 0;
  unsigned char *src = drbg->scratchpad;
  drbg_string_t data;
  unsigned char prefix = DRBG_PREFIX1;

  /* 10.1.1.4 step hashgen 2 */
  memcpy (src, drbg->V, drbg_statelen (drbg));

  drbg_string_fill (&data, src, drbg_statelen (drbg));
  while (len < buflen)
    {
      unsigned int outlen = 0;
      /* 10.1.1.4 step hashgen 4.1 */
      byte *retval = drbg_hash (drbg, &data);
      outlen = (drbg_blocklen (drbg) < (buflen - len)) ?
	drbg_blocklen (drbg) : (buflen - len);
      /* 10.1.1.4 step hashgen 4.2 */
      memcpy (buf + len, retval, outlen);
      len += outlen;
      /* 10.1.1.4 hashgen step 4.3 */
      if (len < buflen)
	drbg_add_buf (src, drbg_statelen (drbg), &prefix, 1);
    }

  memset (drbg->scratchpad, 0, drbg_statelen (drbg));
  return 0;
}

/* Generate function for Hash DRBG as defined in 10.1.1.4  */
static gpg_err_code_t
drbg_hash_generate (drbg_state_t drbg, unsigned char *buf, unsigned int buflen,
		    drbg_string_t *addtl)
{
  gpg_err_code_t ret;
  unsigned char prefix = DRBG_PREFIX3;
  drbg_string_t data1, data2;
  byte *retval;
  union
  {
    unsigned char req[8];
    u64 req_int;
  } u;

  /* 10.1.1.4 step 2 */
  ret = drbg_hash_process_addtl (drbg, addtl);
  if (ret)
    return ret;
  /* 10.1.1.4 step 3 -- invocation of the Hashgen function defined in
   * 10.1.1.4 */
  ret = drbg_hash_hashgen (drbg, buf, buflen);
  if (ret)
    return ret;

  /* 10.1.1.4 step 4 */
  drbg_string_fill (&data1, &prefix, 1);
  drbg_string_fill (&data2, drbg->V, drbg_statelen (drbg));
  data1.next = &data2;

  /* this is the value H as documented in 10.1.1.4 */
  retval = drbg_hash (drbg, &data1);

  /* 10.1.1.4 step 5 */
  drbg_add_buf (drbg->V, drbg_statelen (drbg), retval, drbg_blocklen (drbg));
  drbg_add_buf (drbg->V, drbg_statelen (drbg), drbg->C, drbg_statelen (drbg));
  u.req_int = be_bswap64 (drbg->reseed_ctr);
  drbg_add_buf (drbg->V, drbg_statelen (drbg), u.req, sizeof (u.req));

  return ret;
}

/*
 * scratchpad usage: as update and generate are used isolated, both
 * can use the scratchpad
 */
static struct drbg_state_ops_s drbg_hash_ops = {
  drbg_hash_update,
  drbg_hash_generate,
  drbg_hash_init,
  drbg_hash_fini,
};

/******************************************************************
 * Functions common for DRBG implementations
 ******************************************************************/

/*
 * Seeding or reseeding of the DRBG
 *
 * @drbg: DRBG state struct
 * @pers: personalization / additional information buffer
 * @reseed: 0 for initial seed process, 1 for reseeding
 *
 * return:
 *	0 on success
 *	error value otherwise
 */
static gpg_err_code_t
drbg_seed (drbg_state_t drbg, drbg_string_t *pers, int reseed)
{
  gpg_err_code_t ret = 0;
  unsigned char *entropy = NULL;
  size_t entropylen = 0;
  drbg_string_t data1;

  /* 9.1 / 9.2 / 9.3.1 step 3 */
  if (pers && pers->len > (drbg_max_addtl ()))
    {
      dbg (("DRBG: personalization string too long %lu\n", pers->len));
      return GPG_ERR_INV_ARG;
    }
  if (drbg->test_data && drbg->test_data->testentropy)
    {
      drbg_string_fill (&data1, drbg->test_data->testentropy->buf,
			     drbg->test_data->testentropy->len);
      dbg (("DRBG: using test entropy\n"));
    }
  else
    {
      /* Gather entropy equal to the security strength of the DRBG.
       * With a derivation function, a nonce is required in addition
       * to the entropy. A nonce must be at least 1/2 of the security
       * strength of the DRBG in size. Thus, entropy * nonce is 3/2
       * of the strength. The consideration of a nonce is only
       * applicable during initial seeding. */
      entropylen = drbg_sec_strength (drbg->core->flags);
      if (!entropylen)
	return GPG_ERR_GENERAL;
      if (0 == reseed)
	/* make sure we round up strength/2 in
	 * case it is not divisible by 2 */
	entropylen = ((entropylen + 1) / 2) * 3;
      dbg (("DRBG: (re)seeding with %lu bytes of entropy\n", entropylen));
      entropy = xcalloc_secure (1, entropylen);
      if (!entropy)
	return GPG_ERR_ENOMEM;
      ret = drbg_get_entropy (drbg, entropy, entropylen);
      if (ret)
	goto out;
      drbg_string_fill (&data1, entropy, entropylen);
    }

  /* concatenation of entropy with personalization str / addtl input)
   * the variable pers is directly handed by the caller, check its
   * contents whether it is appropriate */
  if (pers && pers->buf && 0 < pers->len && NULL == pers->next)
    {
      data1.next = pers;
      dbg (("DRBG: using personalization string\n"));
    }

  ret = drbg->d_ops->update (drbg, &data1, reseed);
  dbg (("DRBG: state updated with seed\n"));
  if (ret)
    goto out;
  drbg->seeded = 1;
  /* 10.1.1.2 / 10.1.1.3 step 5 */
  drbg->reseed_ctr = 1;

 out:
  xfree (entropy);
  return ret;
}


/*************************************************************************
 * Exported interfaces.
 *************************************************************************/

/*
 * DRBG generate function as required by SP800-90A - this function
 * generates random numbers
 *
 * @drbg   DRBG state handle
 * @buf    Buffer where to store the random numbers -- the buffer must already
 *         be pre-allocated by caller
 * @buflen Length of output buffer - this value defines the number of random
 *	   bytes pulled from DRBG
 * @addtl  Additional input that is mixed into state, may be NULL -- note
 *	   the entropy is pulled by the DRBG internally unconditionally
 *	   as defined in SP800-90A. The additional input is mixed into
 *	   the state in addition to the pulled entropy.
 *
 * return: Generated number of bytes.
 */
static gpg_err_code_t
drbg_generate (drbg_state_t drbg,
               unsigned char *buf, unsigned int buflen,
               drbg_string_t *addtl)
{
  gpg_err_code_t ret = GPG_ERR_INV_ARG;

  if (0 == buflen || !buf)
    {
      dbg (("DRBG: no buffer provided\n"));
      return ret;
    }
  if (addtl && NULL == addtl->buf && 0 < addtl->len)
    {
      dbg (("DRBG: wrong format of additional information\n"));
      return ret;
    }

  /* 9.3.1 step 2 */
  if (buflen > (drbg_max_request_bytes ()))
    {
      dbg (("DRBG: requested random numbers too large %u\n", buflen));
      return ret;
    }
  /* 9.3.1 step 3 is implicit with the chosen DRBG */
  /* 9.3.1 step 4 */
  if (addtl && addtl->len > (drbg_max_addtl ()))
    {
      dbg (("DRBG: additional information string too long %lu\n",
	    addtl->len));
      return ret;
    }
  /* 9.3.1 step 5 is implicit with the chosen DRBG */
  /* 9.3.1 step 6 and 9 supplemented by 9.3.2 step c -- the spec is a
   * bit convoluted here, we make it simpler */
  if ((drbg_max_requests ()) < drbg->reseed_ctr)
    drbg->seeded = 0;

  if (drbg->pr || !drbg->seeded)
    {
      dbg (("DRBG: reseeding before generation (prediction resistance: %s, state %s)\n", drbg->pr ? "true" : "false", drbg->seeded ? "seeded" : "unseeded"));
      /* 9.3.1 steps 7.1 through 7.3 */
      ret = drbg_seed (drbg, addtl, 1);
      if (ret)
	return ret;
      /* 9.3.1 step 7.4 */
      addtl = NULL;
    }

  if (addtl && addtl->buf)
    {
      dbg (("DRBG: using additional information string\n"));
    }

  /* 9.3.1 step 8 and 10 */
  ret = drbg->d_ops->generate (drbg, buf, buflen, addtl);

  /* 10.1.1.4 step 6, 10.1.2.5 step 7, 10.2.1.5.2 step 7 */
  drbg->reseed_ctr++;
  if (ret)
    return ret;

  /* 11.3.3 -- re-perform self tests after some generated random
   * numbers, the chosen value after which self test is performed
   * is arbitrary, but it should be reasonable */
  /* Here we do not perform the self tests because of the following
   * reasons: it is mathematically impossible that the initial self tests
   * were successfully and the following are not. If the initial would
   * pass and the following would not, the system integrity is violated.
   * In this case, the entire system operation is questionable and it
   * is unlikely that the integrity violation only affects to the
   * correct operation of the DRBG.
   */
#if 0
  if (drbg->reseed_ctr && !(drbg->reseed_ctr % 4096))
    {
      dbg (("DRBG: start to perform self test\n"));
      ret = drbg_healthcheck ();
      if (ret)
	{
	  log_fatal (("DRBG: self test failed\n"));
	  return ret;
	}
      else
	{
	  dbg (("DRBG: self test successful\n"));
	}
    }
#endif

  return ret;
}

/*
 * Wrapper around drbg_generate which can pull arbitrary long strings
 * from the DRBG without hitting the maximum request limitation.
 *
 * Parameters: see drbg_generate
 * Return codes: see drbg_generate -- if one drbg_generate request fails,
 *		 the entire drbg_generate_long request fails
 */
static gpg_err_code_t
drbg_generate_long (drbg_state_t drbg,
                    unsigned char *buf, unsigned int buflen,
                    drbg_string_t *addtl)
{
  gpg_err_code_t ret = 0;
  unsigned int slice = 0;
  unsigned char *buf_p = buf;
  unsigned len = 0;
  do
    {
      unsigned int chunk = 0;
      slice = ((buflen - len) / drbg_max_request_bytes ());
      chunk = slice ? drbg_max_request_bytes () : (buflen - len);
      ret = drbg_generate (drbg, buf_p, chunk, addtl);
      if (ret)
	return ret;
      buf_p += chunk;
      len += chunk;
    }
  while (slice > 0 && (len < buflen));
  return ret;
}

/*
 * DRBG uninstantiate function as required by SP800-90A - this function
 * frees all buffers and the DRBG handle
 *
 * @drbg DRBG state handle
 *
 * return
 * 	0 on success
 */
static gpg_err_code_t
drbg_uninstantiate (drbg_state_t drbg)
{
  if (!drbg)
    return GPG_ERR_INV_ARG;
  drbg->d_ops->crypto_fini(drbg);
  xfree (drbg->V);
  drbg->V = NULL;
  xfree (drbg->C);
  drbg->C = NULL;
  drbg->reseed_ctr = 0;
  xfree (drbg->scratchpad);
  drbg->scratchpad = NULL;
  drbg->seeded = 0;
  drbg->pr = 0;
  drbg->seed_init_pid = 0;
  return 0;
}

/*
 * DRBG instantiation function as required by SP800-90A - this function
 * sets up the DRBG handle, performs the initial seeding and all sanity
 * checks required by SP800-90A
 *
 * @drbg memory of state -- if NULL, new memory is allocated
 * @pers Personalization string that is mixed into state, may be NULL -- note
 *	 the entropy is pulled by the DRBG internally unconditionally
 *	 as defined in SP800-90A. The additional input is mixed into
 *	 the state in addition to the pulled entropy.
 * @coreref reference to core
 * @flags Flags defining the requested DRBG type and cipher type. The flags
 * 	  are defined in drbg.h and may be XORed. Beware, if you XOR multiple
 * 	  cipher types together, the code picks the core on a first come first
 * 	  serve basis as it iterates through the available cipher cores and
 * 	  uses the one with the first match. The minimum required flags are:
 * 		cipher type flag
 *
 * return
 *	0 on success
 *	error value otherwise
 */
static gpg_err_code_t
drbg_instantiate (drbg_state_t drbg,
                  drbg_string_t *pers, int coreref, int pr)
{
  gpg_err_code_t ret = GPG_ERR_ENOMEM;
  unsigned int sb_size = 0;

  if (!drbg)
    return GPG_ERR_INV_ARG;

  dbg (("DRBG: Initializing DRBG core %d with prediction resistance %s\n",
	coreref, pr ? "enabled" : "disabled"));
  drbg->core = &drbg_cores[coreref];
  drbg->pr = pr;
  drbg->seeded = 0;
  if (drbg->core->flags & DRBG_HMAC)
    drbg->d_ops = &drbg_hmac_ops;
  else if (drbg->core->flags & DRBG_HASH_MASK)
    drbg->d_ops = &drbg_hash_ops;
  else if (drbg->core->flags & DRBG_CTR_MASK)
    drbg->d_ops = &drbg_ctr_ops;
  else
    return GPG_ERR_GENERAL;
  /* 9.1 step 1 is implicit with the selected DRBG type -- see
   * drbg_sec_strength() */

  /* 9.1 step 2 is implicit as caller can select prediction resistance
   * and the flag is copied into drbg->flags --
   * all DRBG types support prediction resistance */

  /* 9.1 step 4 is implicit in  drbg_sec_strength */

  ret = drbg->d_ops->crypto_init(drbg);
  if (ret)
    goto err;

  drbg->V = xcalloc_secure (1, drbg_statelen (drbg));
  if (!drbg->V)
    goto fini;
  drbg->C = xcalloc_secure (1, drbg_statelen (drbg));
  if (!drbg->C)
    goto fini;
  /* scratchpad is only generated for CTR and Hash */
  if (drbg->core->flags & DRBG_HMAC)
    sb_size = 0;
  else if (drbg->core->flags & DRBG_CTR_MASK)
    sb_size = drbg_statelen (drbg) + drbg_blocklen (drbg) +	/* temp */
      drbg_statelen (drbg) +	/* df_data */
      drbg_blocklen (drbg) +	/* pad */
      drbg_blocklen (drbg) +	/* iv */
      drbg_statelen (drbg) + drbg_blocklen (drbg);	/* temp */
  else
    sb_size = drbg_statelen (drbg);

  if (0 < sb_size)
    {
      drbg->scratchpad = xcalloc_secure (1, sb_size);
      if (!drbg->scratchpad)
	goto fini;
    }
  dbg (("DRBG: state allocated with scratchpad size %u bytes\n", sb_size));

  /* 9.1 step 6 through 11 */
  ret = drbg_seed (drbg, pers, 0);
  if (ret)
    goto fini;

  dbg (("DRBG: core %d %s prediction resistance successfully initialized\n",
	coreref, pr ? "with" : "without"));
  return 0;

 fini:
  drbg->d_ops->crypto_fini(drbg);
 err:
  drbg_uninstantiate (drbg);
  return ret;
}

/*
 * DRBG reseed function as required by SP800-90A
 *
 * @drbg DRBG state handle
 * @addtl Additional input that is mixed into state, may be NULL -- note
 * 		the entropy is pulled by the DRBG internally unconditionally
 * 		as defined in SP800-90A. The additional input is mixed into
 * 		the state in addition to the pulled entropy.
 *
 * return
 * 	0 on success
 * 	error value otherwise
 */
static gpg_err_code_t
drbg_reseed (drbg_state_t drbg,drbg_string_t *addtl)
{
  gpg_err_code_t ret = 0;
  ret = drbg_seed (drbg, addtl, 1);
  return ret;
}



/******************************************************************
 * Libgcrypt integration code.
 ******************************************************************/

/***************************************************
 * Libgcrypt backend functions to the RNG API code.
 ***************************************************/

static inline void
drbg_lock (void)
{
  gpg_err_code_t ec;

  ec = gpgrt_lock_lock (&drbg_lock_var);
  if (ec)
    log_fatal ("failed to acquire the RNG lock: %s\n", gpg_strerror (ec));
}

static inline void
drbg_unlock (void)
{
  gpg_err_code_t ec;

  ec = gpgrt_lock_unlock (&drbg_lock_var);
  if (ec)
    log_fatal ("failed to release the RNG lock: %s\n", gpg_strerror (ec));
}

/* Basic initialization is required to initialize mutexes and
   do a few checks on the implementation.  */
static void
basic_initialization (void)
{
  static int initialized;

  if (initialized)
    return;
  initialized = 1;

  /* Make sure that we are still using the values we have
     traditionally used for the random levels.  */
  gcry_assert (GCRY_WEAK_RANDOM == 0
               && GCRY_STRONG_RANDOM == 1
               && GCRY_VERY_STRONG_RANDOM == 2);
}

/****** helper functions where lock must be held by caller *****/

/* Check whether given flags are known to point to an applicable DRBG */
static gpg_err_code_t
drbg_algo_available (u32 flags, int *coreref)
{
  int i = 0;
  for (i = 0; ARRAY_SIZE (drbg_cores) > i; i++)
    {
      if ((drbg_cores[i].flags & DRBG_CIPHER_MASK) ==
	  (flags & DRBG_CIPHER_MASK))
	{
	  *coreref = i;
	  return 0;
	}
    }
  return GPG_ERR_GENERAL;
}

static gpg_err_code_t
_drbg_init_internal (u32 flags, drbg_string_t *pers)
{
  static u32 oldflags;
  gpg_err_code_t ret = 0;
  int coreref = 0;
  int pr = 0;

  /* If a caller provides 0 as flags, use the flags of the previous
   * initialization, otherwise use the current flags and remember them
   * for the next invocation.  If no flag is given and no global state
   * is set this is the first initialization and we set the default
   * type.
   */
  if (!flags && !drbg_state)
    flags = oldflags = DRBG_DEFAULT_TYPE;
  else if (!flags)
    flags = oldflags;
  else
    oldflags = flags;

  ret = drbg_algo_available (flags, &coreref);
  if (ret)
    return ret;

  if (drbg_state)
    {
      drbg_uninstantiate (drbg_state);
    }
  else
    {
      drbg_state = &drbg_instance;
    }
  if (flags & DRBG_PREDICTION_RESIST)
    pr = 1;
  ret = drbg_instantiate (drbg_state, pers, coreref, pr);
  if (ret)
    fips_signal_error ("DRBG cannot be initialized");
  else
    drbg_state->seed_init_pid = getpid ();
  return ret;
}

/************* calls available to common RNG code **************/

/*
 * Initialize one DRBG invoked by the libgcrypt API
 */
void
_gcry_rngdrbg_inititialize (int full)
{
  basic_initialization ();
  if (!full)
      return;
  drbg_lock ();
  if (!drbg_state)
    _drbg_init_internal (0, NULL);
  drbg_unlock ();
}

/*
 * Backend handler function for GCRYCTL_DRBG_REINIT
 *
 * Select a different DRBG type and initialize it.
 * Function checks whether requested DRBG type exists and returns an error in
 * case it does not. In case of an error, the previous instantiated DRBG is
 * left untouched and alive. Thus, in case of an error, a DRBG is always
 * available, even if it is not the chosen one.
 *
 * Re-initialization will be performed in any case regardless whether flags
 * or personalization string are set.
 *
 * If flags is NULL, do not change current DRBG.  If PERS is NULL and
 * NPERS is 0, re-initialize without personalization string.  If PERS
 * is not NULL NPERS must be one and PERS and the first ietm from the
 * bufer is take as personalization string.
 */
gpg_err_code_t
_gcry_rngdrbg_reinit (const char *flagstr, gcry_buffer_t *pers, int npers)
{
  gpg_err_code_t ret;
  unsigned int flags;

  /* If PERS is not given we expect NPERS to be zero; if given we
     expect a one-item array.  */
  if ((!pers && npers) || (pers && npers != 1))
    return GPG_ERR_INV_ARG;

  ret = parse_flag_string (flagstr, &flags);
  if (!ret)
    {
      dbg (("DRBG: reinitialize internal DRBG state with flags %u\n", flags));
      drbg_lock ();
      if (pers)
        {
          drbg_string_t persbuf;

          drbg_string_fill
            (&persbuf, (const unsigned char *)pers[0].data + pers[0].off,
             pers[0].len);
          ret = _drbg_init_internal (flags, &persbuf);
        }
      else
        ret = _drbg_init_internal (flags, NULL);
      drbg_unlock ();
    }
  return ret;
}

/* Release resources used by this DRBG module.  That is, close the FDs
 * of the random gather module (if any), and release memory used.
 */
void
_gcry_rngdrbg_close_fds (void)
{
  drbg_lock ();
#if USE_RNDGETENTROPY
  _gcry_rndgetentropy_gather_random (NULL, 0, 0, 0);
#endif
#if USE_RNDOLDLINUX
  _gcry_rndoldlinux_gather_random (NULL, 0, 0, 0);
#endif
  if (drbg_state)
    {
      drbg_uninstantiate (drbg_state);
      drbg_state = NULL;
    }
  drbg_unlock ();
}

/* Print some statistics about the RNG.  */
void
_gcry_rngdrbg_dump_stats (void)
{
  /* Not yet implemented.  */
  /* Maybe dumping of reseed counter? */
}

/* This function returns true if no real RNG is available or the
 * quality of the RNG has been degraded for test purposes.  */
int
_gcry_rngdrbg_is_faked (void)
{
  return 0;			/* Faked random is not allowed.  */
}

/* Add BUFLEN bytes from BUF to the internal random pool.  QUALITY
 * should be in the range of 0..100 to indicate the goodness of the
 * entropy added, or -1 for goodness not known. */
gcry_error_t
_gcry_rngdrbg_add_bytes (const void *buf, size_t buflen, int quality)
{
  gpg_err_code_t ret = 0;
  drbg_string_t seed;
  (void) quality;
  _gcry_rngdrbg_inititialize (1); /* Auto-initialize if needed */
  if (!drbg_state)
    return GPG_ERR_GENERAL;
  drbg_string_fill (&seed, (unsigned char *) buf, buflen);
  drbg_lock ();
  ret = drbg_reseed (drbg_state, &seed);
  drbg_unlock ();
  return ret;
}

/* This function is to be used for all types of random numbers, including
 * nonces
 */
void
_gcry_rngdrbg_randomize (void *buffer, size_t length,
		      enum gcry_random_level level)
{
  (void) level;
  _gcry_rngdrbg_inititialize (1); /* Auto-initialize if needed */
  drbg_lock ();
  if (!drbg_state)
    {
      fips_signal_error ("DRBG is not initialized");
      goto bailout;
    }

  /* As reseeding changes the entire state of the DRBG, including any
   * key, either a re-init or a reseed is sufficient for a fork */
  if (drbg_state->seed_init_pid != getpid ())
    {
      /* Update the PID recorded.  */
      drbg_state->seed_init_pid = getpid ();

      /* We are in a child of us. Perform a reseeding. */
      if (drbg_reseed (drbg_state, NULL))
	{
	  fips_signal_error ("reseeding upon fork failed");
	  log_fatal ("severe error getting random\n");
	  goto bailout;
	}
    }
  /* potential integer overflow is covered by drbg_generate which
   * ensures that length cannot overflow an unsigned int */
  if (0 < length)
    {
      if (!buffer)
	goto bailout;
      if (drbg_generate_long (drbg_state, buffer, (unsigned int) length, NULL))
	log_fatal ("No random numbers generated\n");
    }
  else
    {
      drbg_gen_t *data = (drbg_gen_t *)buffer;
      /* catch NULL pointer */
      if (!data || !data->outbuf)
	{
	  fips_signal_error ("No output buffer provided");
	  goto bailout;
	}
      if (drbg_generate_long (drbg_state, data->outbuf, data->outlen,
                              data->addtl))
	log_fatal ("No random numbers generated\n");
    }

 bailout:
  drbg_unlock ();
  return;

}

/***************************************************************
 * Self-test code
 ***************************************************************/

/*
 * Test vectors from
 * http://csrc.nist.gov/groups/STM/cavp/documents/drbg/drbgtestvectors.zip
 */
struct gcry_drbg_test_vector drbg_test_pr[] = {
  {
    /* .flags = */ "sha256 pr" /* DRBG_PR_HASHSHA256 */,
    /* .entropy = */ (unsigned char *)
    "\x5d\xf2\x14\xbc\xf6\xb5\x4e\x0b\xf0\x0d\x6f\x2d"
    "\xe2\x01\x66\x7b\xd0\xa4\x73\xa4\x21\xdd\xb0\xc0"
    "\x51\x79\x09\xf4\xea\xa9\x08\xfa\xa6\x67\xe0\xe1"
    "\xd1\x88\xa8\xad\xee\x69\x74\xb3\x55\x06\x9b\xf6",
    /* .entropylen = */ 48,
    /* .entpra = */ (unsigned char *)
    "\xef\x48\x06\xa2\xc2\x45\xf1\x44\xfa\x34\x2c\xeb"
    "\x8d\x78\x3c\x09\x8f\x34\x72\x20\xf2\xe7\xfd\x13"
    "\x76\x0a\xf6\xdc\x3c\xf5\xc0\x15",
    /* .entprb = */ (unsigned char *)
    "\x4b\xbe\xe5\x24\xed\x6a\x2d\x0c\xdb\x73\x5e\x09"
    "\xf9\xad\x67\x7c\x51\x47\x8b\x6b\x30\x2a\xc6\xde"
    "\x76\xaa\x55\x04\x8b\x0a\x72\x95",
    /* .entprlen = */ 32,
    /* .addtla = */ (unsigned char *)
    "\xbe\x13\xdb\x2a\xe9\xa8\xfe\x09\x97\xe1\xce\x5d"
    "\xe8\xbb\xc0\x7c\x4f\xcb\x62\x19\x3f\x0f\xd2\xad"
    "\xa9\xd0\x1d\x59\x02\xc4\xff\x70",
    /* .addtlb = */ (unsigned char *)
    "\x6f\x96\x13\xe2\xa7\xf5\x6c\xfe\xdf\x66\xe3\x31"
    "\x63\x76\xbf\x20\x27\x06\x49\xf1\xf3\x01\x77\x41"
    "\x9f\xeb\xe4\x38\xfe\x67\x00\xcd",
    /* .addtllen = */ 32,
    /* .pers = */ NULL,
    /* .perslen = */ 0,
    /* .expected = */ (unsigned char *)
    "\x3b\x14\x71\x99\xa1\xda\xa0\x42\xe6\xc8\x85\x32"
    "\x70\x20\x32\x53\x9a\xbe\xd1\x1e\x15\xef\xfb\x4c"
    "\x25\x6e\x19\x3a\xf0\xb9\xcb\xde\xf0\x3b\xc6\x18"
    "\x4d\x85\x5a\x9b\xf1\xe3\xc2\x23\x03\x93\x08\xdb"
    "\xa7\x07\x4b\x33\x78\x40\x4d\xeb\x24\xf5\x6e\x81"
    "\x4a\x1b\x6e\xa3\x94\x52\x43\xb0\xaf\x2e\x21\xf4"
    "\x42\x46\x8e\x90\xed\x34\x21\x75\xea\xda\x67\xb6"
    "\xe4\xf6\xff\xc6\x31\x6c\x9a\x5a\xdb\xb3\x97\x13"
    "\x09\xd3\x20\x98\x33\x2d\x6d\xd7\xb5\x6a\xa8\xa9"
    "\x9a\x5b\xd6\x87\x52\xa1\x89\x2b\x4b\x9c\x64\x60"
    "\x50\x47\xa3\x63\x81\x16\xaf\x19",
    /* .expectedlen = */ 128,
    /* .entropyreseed = */ NULL,
    /* .entropyreseed_len = */ 0,
    /* .addtl_reseed = */ NULL,
    /* .addtl_reseed_len = */ 0
  },
  {
    /* flags = */ "hmac sha256 pr" /* DRBG_PR_HMACSHA256 */,
    /* .entropy = */ (unsigned char *)
    "\x13\x54\x96\xfc\x1b\x7d\x28\xf3\x18\xc9\xa7\x89"
    "\xb6\xb3\xc8\x72\xac\x00\xd4\x59\x36\x25\x05\xaf"
    "\xa5\xdb\x96\xcb\x3c\x58\x46\x87\xa5\xaa\xbf\x20"
    "\x3b\xfe\x23\x0e\xd1\xc7\x41\x0f\x3f\xc9\xb3\x67",
    /* .entropylen = */ 48,
    /* .entpra = */ (unsigned char *)
    "\xe2\xbd\xb7\x48\x08\x06\xf3\xe1\x93\x3c\xac\x79"
    "\xa7\x2b\x11\xda\xe3\x2e\xe1\x91\xa5\x02\x19\x57"
    "\x20\x28\xad\xf2\x60\xd7\xcd\x45",
    /* .entprb = */ (unsigned char *)
    "\x8b\xd4\x69\xfc\xff\x59\x95\x95\xc6\x51\xde\x71"
    "\x68\x5f\xfc\xf9\x4a\xab\xec\x5a\xcb\xbe\xd3\x66"
    "\x1f\xfa\x74\xd3\xac\xa6\x74\x60",
    /* .entprlen = */ 32,
    /* .addtla = */ NULL,
    /* .addtlb = */ NULL,
    /* .addtllen = */ 0,
    /* .pers = */ (unsigned char *)
    "\x64\xb6\xfc\x60\xbc\x61\x76\x23\x6d\x3f\x4a\x0f"
    "\xe1\xb4\xd5\x20\x9e\x70\xdd\x03\x53\x6d\xbf\xce"
    "\xcd\x56\x80\xbc\xb8\x15\xc8\xaa",
    /* .perslen = */ 32,
    /* .expected = */ (unsigned char *)
    "\x1f\x9e\xaf\xe4\xd2\x46\xb7\x47\x41\x4c\x65\x99"
    "\x01\xe9\x3b\xbb\x83\x0c\x0a\xb0\xc1\x3a\xe2\xb3"
    "\x31\x4e\xeb\x93\x73\xee\x0b\x26\xc2\x63\xa5\x75"
    "\x45\x99\xd4\x5c\x9f\xa1\xd4\x45\x87\x6b\x20\x61"
    "\x40\xea\x78\xa5\x32\xdf\x9e\x66\x17\xaf\xb1\x88"
    "\x9e\x2e\x23\xdd\xc1\xda\x13\x97\x88\xa5\xb6\x5e"
    "\x90\x14\x4e\xef\x13\xab\x5c\xd9\x2c\x97\x9e\x7c"
    "\xd7\xf8\xce\xea\x81\xf5\xcd\x71\x15\x49\x44\xce"
    "\x83\xb6\x05\xfb\x7d\x30\xb5\x57\x2c\x31\x4f\xfc"
    "\xfe\x80\xb6\xc0\x13\x0c\x5b\x9b\x2e\x8f\x3d\xfc"
    "\xc2\xa3\x0c\x11\x1b\x80\x5f\xf3",
    /* .expectedlen = */ 128,
    /* .entropyreseed = */ NULL,
    /* .entropyreseed_len = */ 0,
    /* .addtl_reseed = */ NULL,
    /* .addtl_reseed_len = */ 0
  },
  {
    /* .flags = */ "aes sym128 pr", /* DRBG_PR_CTRAES128 */
    /* .entropy = */ (unsigned char *)
    "\x92\x89\x8f\x31\xfa\x1c\xff\x6d\x18\x2f\x26\x06"
    "\x43\xdf\xf8\x18\xc2\xa4\xd9\x72\xc3\xb9\xb6\x97",
    /* .entropylen = */ 24,
    /* .entpra = */ (unsigned char *)
    "\x20\x72\x8a\x06\xf8\x6f\x8d\xd4\x41\xe2\x72\xb7"
    "\xc4\x2c\xe8\x10",
    /* .entprb = */ (unsigned char *)
    "\x3d\xb0\xf0\x94\xf3\x05\x50\x33\x17\x86\x3e\x22"
    "\x08\xf7\xa5\x01",
    /* .entprlen = */ 16,
    /* .addtla = */ (unsigned char *)
    "\x1a\x40\xfa\xe3\xcc\x6c\x7c\xa0\xf8\xda\xba\x59"
    "\x23\x6d\xad\x1d",
    /* .addtlb = */ (unsigned char *)
    "\x9f\x72\x76\x6c\xc7\x46\xe5\xed\x2e\x53\x20\x12"
    "\xbc\x59\x31\x8c",
    /* .addtllen = */ 16,
    /* .pers = */ (unsigned char *)
    "\xea\x65\xee\x60\x26\x4e\x7e\xb6\x0e\x82\x68\xc4"
    "\x37\x3c\x5c\x0b",
    /* .perslen = */ 16,
    /* .expected = */ (unsigned char *)
    "\x5a\x35\x39\x87\x0f\x4d\x22\xa4\x09\x24\xee\x71"
    "\xc9\x6f\xac\x72\x0a\xd6\xf0\x88\x82\xd0\x83\x28"
    "\x73\xec\x3f\x93\xd8\xab\x45\x23\xf0\x7e\xac\x45"
    "\x14\x5e\x93\x9f\xb1\xd6\x76\x43\x3d\xb6\xe8\x08"
    "\x88\xf6\xda\x89\x08\x77\x42\xfe\x1a\xf4\x3f\xc4"
    "\x23\xc5\x1f\x68",
    /* .expectedlen = */ 64,
    /* .entropyreseed = */ NULL,
    /* .entropyreseed_len = */ 0,
    /* .addtl_reseed = */ NULL,
    /* .addtl_reseed_len = */ 0
   }
};

struct gcry_drbg_test_vector drbg_test_nopr[] = {
  {
    /* .flags = */ "sha256" /* DRBG_NOPR_HASHSHA256 */,
    /* .entropy = */ (unsigned char *)
    "\x73\xd3\xfb\xa3\x94\x5f\x2b\x5f\xb9\x8f\xf6\x9c"
    "\x8a\x93\x17\xae\x19\xc3\x4c\xc3\xd6\xca\xa3\x2d"
    "\x16\xfc\x42\xd2\x2d\xd5\x6f\x56\xcc\x1d\x30\xff"
    "\x9e\x06\x3e\x09\xce\x58\xe6\x9a\x35\xb3\xa6\x56",
    /* .entropylen = */ 48,
    /* .entpra = */ NULL,
    /* .entprb = */ NULL,
    /* .entprlen = */ 0,
    /* .addtla = */ (unsigned char *)
    "\xf4\xd5\x98\x3d\xa8\xfc\xfa\x37\xb7\x54\x67\x73"
    "\xc7\xc3\xdd\x47\x34\x71\x02\x5d\xc1\xa0\xd3\x10"
    "\xc1\x8b\xbd\xf5\x66\x34\x6f\xdd",
    /* .addtlb = */ (unsigned char *)
    "\xf7\x9e\x6a\x56\x0e\x73\xe9\xd9\x7a\xd1\x69\xe0"
    "\x6f\x8c\x55\x1c\x44\xd1\xce\x6f\x28\xcc\xa4\x4d"
    "\xa8\xc0\x85\xd1\x5a\x0c\x59\x40",
    /* .addtllen = */ 32,
    /* .pers = */ NULL,
    /* .perslen = */ 0,
    /* .expected = */ (unsigned char *)
    "\x71\x7b\x93\x46\x1a\x40\xaa\x35\xa4\xaa\xc5\xe7"
    "\x6d\x5b\x5b\x8a\xa0\xdf\x39\x7d\xae\x71\x58\x5b"
    "\x3c\x7c\xb4\xf0\x89\xfa\x4a\x8c\xa9\x5c\x54\xc0"
    "\x40\xdf\xbc\xce\x26\x81\x34\xf8\xba\x7d\x1c\xe8"
    "\xad\x21\xe0\x74\xcf\x48\x84\x30\x1f\xa1\xd5\x4f"
    "\x81\x42\x2f\xf4\xdb\x0b\x23\xf8\x73\x27\xb8\x1d"
    "\x42\xf8\x44\x58\xd8\x5b\x29\x27\x0a\xf8\x69\x59"
    "\xb5\x78\x44\xeb\x9e\xe0\x68\x6f\x42\x9a\xb0\x5b"
    "\xe0\x4e\xcb\x6a\xaa\xe2\xd2\xd5\x33\x25\x3e\xe0"
    "\x6c\xc7\x6a\x07\xa5\x03\x83\x9f\xe2\x8b\xd1\x1c"
    "\x70\xa8\x07\x59\x97\xeb\xf6\xbe",
    /* .expectedlen = */ 128,
    /* .entropyreseed = */ NULL,
    /* .entropyreseed_len = */ 0,
    /* .addtl_reseed = */ NULL,
    /* .addtl_reseed_len = */ 0
  },
  {
    /* .flags = */ "hmac sha256" /* DRBG_NOPR_HMACSHA256 */,
    /* .entropy = */ (unsigned char *)
    "\x8d\xf0\x13\xb4\xd1\x03\x52\x30\x73\x91\x7d\xdf"
    "\x6a\x86\x97\x93\x05\x9e\x99\x43\xfc\x86\x54\x54"
    "\x9e\x7a\xb2\x2f\x7c\x29\xf1\x22\xda\x26\x25\xaf"
    "\x2d\xdd\x4a\xbc\xce\x3c\xf4\xfa\x46\x59\xd8\x4e",
    /* .entropylen = */ 48,
    /* .entpra = */ NULL,
    /* .entprb = */ NULL,
    /* .entprlen = */ 0,
    /* .addtla = */ NULL,
    /* .addtlb = */ NULL,
    /* .addtllen = */ 0,
    /* .pers = */ (unsigned char *)
    "\xb5\x71\xe6\x6d\x7c\x33\x8b\xc0\x7b\x76\xad\x37"
    "\x57\xbb\x2f\x94\x52\xbf\x7e\x07\x43\x7a\xe8\x58"
    "\x1c\xe7\xbc\x7c\x3a\xc6\x51\xa9",
    /* .perslen = */ 32,
    /* .expected = */ (unsigned char *)
    "\xb9\x1c\xba\x4c\xc8\x4f\xa2\x5d\xf8\x61\x0b\x81"
    "\xb6\x41\x40\x27\x68\xa2\x09\x72\x34\x93\x2e\x37"
    "\xd5\x90\xb1\x15\x4c\xbd\x23\xf9\x74\x52\xe3\x10"
    "\xe2\x91\xc4\x51\x46\x14\x7f\x0d\xa2\xd8\x17\x61"
    "\xfe\x90\xfb\xa6\x4f\x94\x41\x9c\x0f\x66\x2b\x28"
    "\xc1\xed\x94\xda\x48\x7b\xb7\xe7\x3e\xec\x79\x8f"
    "\xbc\xf9\x81\xb7\x91\xd1\xbe\x4f\x17\x7a\x89\x07"
    "\xaa\x3c\x40\x16\x43\xa5\xb6\x2b\x87\xb8\x9d\x66"
    "\xb3\xa6\x0e\x40\xd4\xa8\xe4\xe9\xd8\x2a\xf6\xd2"
    "\x70\x0e\x6f\x53\x5c\xdb\x51\xf7\x5c\x32\x17\x29"
    "\x10\x37\x41\x03\x0c\xcc\x3a\x56",
    /* .expectedlen = */ 128,
    /* .entropyreseed = */ NULL,
    /* .entropyreseed_len = */ 0,
    /* .addtl_reseed = */ NULL,
    /* .addtl_reseed_len = */ 0
  },
  {
    /* .flags = */ "aes sym128" /* DRBG_NOPR_CTRAES128 */,
    /* .entropy = */ (unsigned char *)
    "\xc0\x70\x1f\x92\x50\x75\x8f\xcd\xf2\xbe\x73\x98"
    "\x80\xdb\x66\xeb\x14\x68\xb4\xa5\x87\x9c\x2d\xa6",
    /* .entropylen = */ 24,
    /* .entpra = */ NULL,
    /* .entprb = */ NULL,
    /* .entprlen = */ 0,
    /* .addtla = */ (unsigned char *)
    "\xf9\x01\xf8\x16\x7a\x1d\xff\xde\x8e\x3c\x83\xe2"
    "\x44\x85\xe7\xfe",
    /* .addtlb = */ (unsigned char *)
    "\x17\x1c\x09\x38\xc2\x38\x9f\x97\x87\x60\x55\xb4"
    "\x82\x16\x62\x7f",
    /* .addtllen = */ 16,
    /* .pers = */ (unsigned char *)
    "\x80\x08\xae\xe8\xe9\x69\x40\xc5\x08\x73\xc7\x9f"
    "\x8e\xcf\xe0\x02",
    /* .perslen = */ 16,
    /* .expected = */ (unsigned char *)
    "\x97\xc0\xc0\xe5\xa0\xcc\xf2\x4f\x33\x63\x48\x8a"
    "\xdb\x13\x0a\x35\x89\xbf\x80\x65\x62\xee\x13\x95"
    "\x7c\x33\xd3\x7d\xf4\x07\x77\x7a\x2b\x65\x0b\x5f"
    "\x45\x5c\x13\xf1\x90\x77\x7f\xc5\x04\x3f\xcc\x1a"
    "\x38\xf8\xcd\x1b\xbb\xd5\x57\xd1\x4a\x4c\x2e\x8a"
    "\x2b\x49\x1e\x5c",
    /* .expectedlen = */ 64,
    /* .entropyreseed = */ NULL,
    /* .entropyreseed_len = */ 0,
    /* .addtl_reseed = */ NULL,
    /* .addtl_reseed_len = */ 0
  },
  {
    /* .flags = */ "sha1" /* DRBG_NOPR_HASHSHA1 */,
    /* .entropy = */ (unsigned char *)
    "\x16\x10\xb8\x28\xcc\xd2\x7d\xe0\x8c\xee\xa0\x32"
    "\xa2\x0e\x92\x08\x49\x2c\xf1\x70\x92\x42\xf6\xb5",
    /* .entropylen = */ 24,
    /* .entpra = */ NULL,
    /* .entprb = */ NULL,
    /* .entprlen = */ 0,
    /* .addtla = */ NULL,
    /* .addtlb = */ NULL,
    /* .addtllen = */ 0,
    /* .pers = */ NULL,
    /* .perslen = */ 0,
    /* .expected = */ (unsigned char *)
    "\x56\xf3\x3d\x4f\xdb\xb9\xa5\xb6\x4d\x26\x23\x44"
    "\x97\xe9\xdc\xb8\x77\x98\xc6\x8d\x08\xf7\xc4\x11"
    "\x99\xd4\xbd\xdf\x97\xeb\xbf\x6c\xb5\x55\x0e\x5d"
    "\x14\x9f\xf4\xd5\xbd\x0f\x05\xf2\x5a\x69\x88\xc1"
    "\x74\x36\x39\x62\x27\x18\x4a\xf8\x4a\x56\x43\x35"
    "\x65\x8e\x2f\x85\x72\xbe\xa3\x33\xee\xe2\xab\xff"
    "\x22\xff\xa6\xde\x3e\x22\xac\xa2",
    /* .expectedlen = */ 80,
    /* .entropyreseed = */ (unsigned char *)
    "\x72\xd2\x8c\x90\x8e\xda\xf9\xa4\xd1\xe5\x26\xd8"
    "\xf2\xde\xd5\x44",
    /* .entropyreseed_len = */ 16,
    /* .addtl_reseed = */ NULL,
    /* .addtl_reseed_len = */ 0
  },
  {
    /* .flags = */ "sha1" /* DRBG_NOPR_HASHSHA1 */,
    /* .entropy = */ (unsigned char *)
    "\xd9\xba\xb5\xce\xdc\xa9\x6f\x61\x78\xd6\x45\x09"
    "\xa0\xdf\xdc\x5e\xda\xd8\x98\x94\x14\x45\x0e\x01",
    /* .entropylen = */ 24,
    /* .entpra = */ NULL,
    /* .entprb = */ NULL,
    /* .entprlen = */ 0,
    /* .addtla = */ (unsigned char *)
    "\x04\xfa\x28\x95\xaa\x5a\x6f\x8c\x57\x43\x34\x3b"
    "\x80\x5e\x5e\xa4",
    /* .addtlb = */ (unsigned char *)
    "\xdf\x5d\xc4\x59\xdf\xf0\x2a\xa2\xf0\x52\xd7\x21"
    "\xec\x60\x72\x30",
    /* .addtllen = */ 16,
    /* .pers = */ NULL,
    /* .perslen = */ 0,
    /* .expected = */ (unsigned char *)
    "\xc4\x8b\x89\xf9\xda\x3f\x74\x82\x45\x55\x5d\x5d"
    "\x03\x3b\x69\x3d\xd7\x1a\x4d\xf5\x69\x02\x05\xce"
    "\xfc\xd7\x20\x11\x3c\xc2\x4e\x09\x89\x36\xff\x5e"
    "\x77\xb5\x41\x53\x58\x70\xb3\x39\x46\x8c\xdd\x8d"
    "\x6f\xaf\x8c\x56\x16\x3a\x70\x0a\x75\xb2\x3e\x59"
    "\x9b\x5a\xec\xf1\x6f\x3b\xaf\x6d\x5f\x24\x19\x97"
    "\x1f\x24\xf4\x46\x72\x0f\xea\xbe",
    /* .expectedlen = */ 80,
    /* .entropyreseed = */ (unsigned char *)
    "\xc6\xba\xd0\x74\xc5\x90\x67\x86\xf5\xe1\xf3\x20"
    "\x99\xf5\xb4\x91",
    /* .entropyreseed_len = */ 16,
    /* .addtl_reseed = */ (unsigned char *)
    "\x3e\x6b\xf4\x6f\x4d\xaa\x38\x25\xd7\x19\x4e\x69"
    "\x4e\x77\x52\xf7",
    /* .addtl_reseed_len = */ 16
  }
};


/*
 * Tests implement the CAVS test approach as documented in
 * http://csrc.nist.gov/groups/STM/cavp/documents/drbg/DRBGVS.pdf
 */

/*
 * CAVS test
 *
 * This function is not static as it is needed for as a private API
 * call for the CAVS test tool.
 */
gpg_err_code_t
_gcry_rngdrbg_cavs_test (struct gcry_drbg_test_vector *test, unsigned char *buf)
{
  gpg_err_code_t ret = 0;
  drbg_state_t drbg = NULL;
  struct drbg_test_data_s test_data;
  drbg_string_t addtl, pers, testentropy;
  int coreref = 0;
  int pr = 0;
  u32 flags;

  ret = parse_flag_string (test->flagstr, &flags);
  if (ret)
    goto outbuf;

  ret = drbg_algo_available (flags, &coreref);
  if (ret)
    goto outbuf;

  drbg = xtrycalloc_secure (1, sizeof *drbg);
  if (!drbg)
    {
      ret = gpg_err_code_from_syserror ();
      goto outbuf;
    }

  if ((flags & DRBG_PREDICTION_RESIST))
    pr = 1;

  test_data.testentropy = &testentropy;
  drbg_string_fill (&testentropy, test->entropy, test->entropylen);
  drbg->test_data = &test_data;
  drbg_string_fill (&pers, test->pers, test->perslen);
  ret = drbg_instantiate (drbg, &pers, coreref, pr);
  if (ret)
    goto outbuf;

  if (test->entropyreseed)
    {
      drbg_string_fill (&testentropy, test->entropyreseed,
			     test->entropyreseed_len);
      drbg_string_fill (&addtl, test->addtl_reseed,
			     test->addtl_reseed_len);
      if (drbg_reseed (drbg, &addtl))
	goto outbuf;
    }

  drbg_string_fill (&addtl, test->addtla, test->addtllen);
  if (test->entpra)
    {
      drbg_string_fill (&testentropy, test->entpra, test->entprlen);
      drbg->test_data = &test_data;
    }
  drbg_generate_long (drbg, buf, test->expectedlen, &addtl);

  drbg_string_fill (&addtl, test->addtlb, test->addtllen);
  if (test->entprb)
    {
      drbg_string_fill (&testentropy, test->entprb, test->entprlen);
      drbg->test_data = &test_data;
    }
  drbg_generate_long (drbg, buf, test->expectedlen, &addtl);
  drbg_uninstantiate (drbg);

 outbuf:
  xfree (drbg);
  return ret;
}

/*
 * Invoke the CAVS test and perform the final check whether the
 * calculated random value matches the expected one.
 *
 * This function is not static as it is needed for as a private API
 * call for the CAVS test tool.
 */
gpg_err_code_t
_gcry_rngdrbg_healthcheck_one (struct gcry_drbg_test_vector * test)
{
  gpg_err_code_t ret = GPG_ERR_ENOMEM;
  unsigned char *buf = xcalloc_secure (1, test->expectedlen);
  if (!buf)
    return GPG_ERR_ENOMEM;

  ret = _gcry_rngdrbg_cavs_test (test, buf);
  /* FIXME: The next line is wrong.   */
  ret = memcmp (test->expected, buf, test->expectedlen);

  xfree (buf);
  return ret;
}

/*
 * Tests as defined in 11.3.2 in addition to the cipher tests: testing
 * of the error handling.
 *
 * Note, testing the reseed counter is not done as an automatic reseeding
 * is performed in drbg_generate when the reseed counter is too large.
 */
static gpg_err_code_t
drbg_healthcheck_sanity (struct gcry_drbg_test_vector *test)
{
  unsigned int len = 0;
  drbg_state_t drbg = NULL;
  gpg_err_code_t ret = GPG_ERR_GENERAL;
  gpg_err_code_t tmpret = GPG_ERR_GENERAL;
  struct drbg_test_data_s test_data;
  drbg_string_t addtl, testentropy;
  int coreref = 0;
  unsigned char *buf = NULL;
  size_t max_addtllen, max_request_bytes;
  u32 flags;

  /* only perform test in FIPS mode */
  if (0 == fips_mode ())
    return 0;

  ret = parse_flag_string (test->flagstr, &flags);
  if (ret)
    return ret;
  ret = GPG_ERR_GENERAL; /* Fixme: Improve handling of RET.  */

  buf = xtrycalloc_secure (1, test->expectedlen);
  if (!buf)
    return gpg_err_code_from_syserror ();
  tmpret = drbg_algo_available (flags, &coreref);
  if (tmpret)
    goto outbuf;
  drbg = xtrycalloc_secure (1, sizeof *drbg);
  if (!drbg)
    {
      ret = gpg_err_code_from_syserror ();
      goto outbuf;
    }

  /* if the following tests fail, it is likely that there is a buffer
   * overflow and we get a SIGSEV */
  ret = drbg_instantiate (drbg, NULL, coreref, 1);
  if (ret)
    goto outbuf;
  max_addtllen = drbg_max_addtl ();
  max_request_bytes = drbg_max_request_bytes ();
  /* overflow addtllen with additional info string */
  drbg_string_fill (&addtl, test->addtla, (max_addtllen + 1));
  len = drbg_generate (drbg, buf, test->expectedlen, &addtl);
  if (len)
    goto outdrbg;

  /* overflow max_bits */
  len = drbg_generate (drbg, buf, (max_request_bytes + 1), NULL);
  if (len)
    goto outdrbg;
  drbg_uninstantiate (drbg);

  /* test failing entropy source as defined in 11.3.2 */
  test_data.testentropy = NULL;
  test_data.fail_seed_source = 1;
  drbg->test_data = &test_data;
  tmpret = drbg_instantiate (drbg, NULL, coreref, 0);
  if (!tmpret)
    goto outdrbg;
  test_data.fail_seed_source = 0;

  test_data.testentropy = &testentropy;
  drbg_string_fill (&testentropy, test->entropy, test->entropylen);
  /* overflow max addtllen with personalization string */
  tmpret = drbg_instantiate (drbg, &addtl, coreref, 0);
  if (!tmpret)
    goto outdrbg;

  dbg (("DRBG: Sanity tests for failure code paths successfully completed\n"));
  ret = 0;

 outdrbg:
  drbg_uninstantiate (drbg);
 outbuf:
  xfree (buf);
  xfree (drbg);
  return ret;
}

/*
 * DRBG Healthcheck function as required in SP800-90A
 *
 * return:
 * 	0 on success (all tests pass)
 * 	>0 on error (return code indicate the number of failures)
 */
static int
drbg_healthcheck (void)
{
  int ret = 0;
  ret += _gcry_rngdrbg_healthcheck_one (&drbg_test_nopr[0]);
  ret += _gcry_rngdrbg_healthcheck_one (&drbg_test_nopr[1]);
  ret += _gcry_rngdrbg_healthcheck_one (&drbg_test_nopr[2]);
  ret += _gcry_rngdrbg_healthcheck_one (&drbg_test_nopr[3]);
  ret += _gcry_rngdrbg_healthcheck_one (&drbg_test_nopr[4]);
  ret += _gcry_rngdrbg_healthcheck_one (&drbg_test_pr[0]);
  ret += _gcry_rngdrbg_healthcheck_one (&drbg_test_pr[1]);
  ret += _gcry_rngdrbg_healthcheck_one (&drbg_test_pr[2]);
  ret += drbg_healthcheck_sanity (&drbg_test_nopr[0]);
  return ret;
}

/* Run the self-tests.  */
gcry_error_t
_gcry_rngdrbg_selftest (selftest_report_func_t report)
{
  gcry_err_code_t ec;
  const char *errtxt = NULL;
  drbg_lock ();
  if (0 != drbg_healthcheck ())
    errtxt = "RNG output does not match known value";
  drbg_unlock ();
  if (report && errtxt)
    report ("random", 0, "KAT", errtxt);
  ec = errtxt ? GPG_ERR_SELFTEST_FAILED : 0;
  return gpg_error (ec);
}

/***************************************************************
 * Cipher invocations requested by DRBG
 ***************************************************************/

static gpg_err_code_t
drbg_hash_init (drbg_state_t drbg)
{
  gcry_md_hd_t hd;
  gpg_error_t err;

  err = _gcry_md_open (&hd, drbg->core->backend_cipher, 0);
  if (err)
    return err;

  drbg->priv_data = hd;

  return 0;
}

static gpg_err_code_t
drbg_hmac_init (drbg_state_t drbg)
{
  gcry_md_hd_t hd;
  gpg_error_t err;

  err = _gcry_md_open (&hd, drbg->core->backend_cipher, GCRY_MD_FLAG_HMAC);
  if (err)
    return err;

  drbg->priv_data = hd;

  return 0;
}

static gpg_err_code_t
drbg_hmac_setkey (drbg_state_t drbg, const unsigned char *key)
{
  gcry_md_hd_t hd = (gcry_md_hd_t)drbg->priv_data;

  return _gcry_md_setkey (hd, key, drbg_statelen (drbg));
}

static void
drbg_hash_fini (drbg_state_t drbg)
{
  gcry_md_hd_t hd = (gcry_md_hd_t)drbg->priv_data;

  _gcry_md_close (hd);
}

static byte *
drbg_hash (drbg_state_t drbg, const drbg_string_t *buf)
{
  gcry_md_hd_t hd = (gcry_md_hd_t)drbg->priv_data;

  _gcry_md_reset(hd);
  for (; NULL != buf; buf = buf->next)
    _gcry_md_write (hd, buf->buf, buf->len);
  _gcry_md_final (hd);
  return _gcry_md_read (hd, drbg->core->backend_cipher);
}

static void
drbg_sym_fini (drbg_state_t drbg)
{
  gcry_cipher_hd_t hd = (gcry_cipher_hd_t)drbg->priv_data;

  if (hd)
    _gcry_cipher_close (hd);
  if (drbg->ctr_handle)
    _gcry_cipher_close (drbg->ctr_handle);
}

static gpg_err_code_t
drbg_sym_init (drbg_state_t drbg)
{
  gcry_cipher_hd_t hd;
  gpg_error_t err;

  err = _gcry_cipher_open (&hd, drbg->core->backend_cipher,
			   GCRY_CIPHER_MODE_ECB, 0);
  if (err)
    {
      drbg_sym_fini (drbg);
      return err;
    }
  drbg->priv_data = hd;

  err = _gcry_cipher_open (&drbg->ctr_handle, drbg->core->backend_cipher,
			   GCRY_CIPHER_MODE_CTR, 0);
  if (err)
    {
      drbg_sym_fini (drbg);
      return err;
    }


  if (drbg_blocklen (drbg) !=
      _gcry_cipher_get_algo_blklen (drbg->core->backend_cipher))
    {
      drbg_sym_fini (drbg);
      return -GPG_ERR_NO_ERROR;
    }

  return 0;
}

static gpg_err_code_t
drbg_sym_setkey (drbg_state_t drbg, const unsigned char *key)
{
  gcry_cipher_hd_t hd = (gcry_cipher_hd_t)drbg->priv_data;

  return _gcry_cipher_setkey (hd, key, drbg_keylen (drbg));
}

static gpg_err_code_t
drbg_sym (drbg_state_t drbg, unsigned char *outval, const drbg_string_t *buf)
{
  gcry_cipher_hd_t hd = (gcry_cipher_hd_t)drbg->priv_data;

  _gcry_cipher_reset(hd);
  if (drbg_blocklen (drbg) < buf->len)
    return -GPG_ERR_NO_ERROR;
  /* in is only component */
  return _gcry_cipher_encrypt (hd, outval, drbg_blocklen (drbg), buf->buf,
			       buf->len);
}

static gpg_err_code_t
drbg_sym_ctr (drbg_state_t drbg,
	      const unsigned char *inbuf, unsigned int inbuflen,
	      unsigned char *outbuf, unsigned int outbuflen)
{
  gpg_error_t err;

  _gcry_cipher_reset(drbg->ctr_handle);
  err = _gcry_cipher_setctr(drbg->ctr_handle, drbg->V, drbg_blocklen (drbg));
  if (err)
    return err;

  while (outbuflen)
    {
       unsigned int cryptlen = (inbuflen > outbuflen) ? outbuflen : inbuflen;

       err = _gcry_cipher_encrypt (drbg->ctr_handle, outbuf, cryptlen, inbuf,
				   cryptlen);
       if (err)
         return err;

       outbuflen -= cryptlen;
       outbuf += cryptlen;
    }
  return _gcry_cipher_getctr(drbg->ctr_handle, drbg->V, drbg_blocklen (drbg));
}
