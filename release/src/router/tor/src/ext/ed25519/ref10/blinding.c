/* Added to ref10 for Tor. We place this in the public domain.  Alternatively,
 * you may have it under the Creative Commons 0 "CC0" license. */
//#include "fe.h"
#include "ge.h"
#include "sc.h"
#include "crypto_hash_sha512.h"
#include "ed25519_ref10.h"

#include <string.h>
#include "lib/crypt_ops/crypto_util.h"

static void
ed25519_ref10_gettweak(unsigned char *out, const unsigned char *param)
{
  memcpy(out, param, 32);

  out[0] &= 248;  /* Is this necessary necessary ? */
  out[31] &= 63;
  out[31] |= 64;
}

int ed25519_ref10_blind_secret_key(unsigned char *out,
                              const unsigned char *inp,
                              const unsigned char *param)
{
  const char str[] = "Derive temporary signing key hash input";
  unsigned char tweak[64];
  unsigned char zero[32];
  ed25519_ref10_gettweak(tweak, param);

  memset(zero, 0, 32);
  sc_muladd(out, inp, tweak, zero);

  crypto_hash_sha512_2(tweak, (const unsigned char *)str, strlen(str),
                       inp+32, 32);
  memcpy(out+32, tweak, 32);

  memwipe(tweak, 0, sizeof(tweak));

  return 0;
}

int ed25519_ref10_blind_public_key(unsigned char *out,
                              const unsigned char *inp,
                              const unsigned char *param)
{
  unsigned char tweak[64];
  unsigned char zero[32];
  unsigned char pkcopy[32];
  ge_p3 A;
  ge_p2 Aprime;
  int retval = -1;

  ed25519_ref10_gettweak(tweak, param);

  memset(zero, 0, sizeof(zero));
  /* Not the greatest implementation of all of this.  I wish I had
   * better-suited primitives to work with here... (but I don't wish that so
   * strongly that I'm about to code my own ge_scalarmult_vartime). */

  /* We negate the public key first, so that we can pass it to
   * frombytes_negate_vartime, which negates it again. If there were a
   * "ge_frombytes", we'd use that, but there isn't. */
  memcpy(pkcopy, inp, 32);
  pkcopy[31] ^= (1<<7);
  if (ge_frombytes_negate_vartime(&A, pkcopy) != 0) {
    goto done;
  }
  /* There isn't a regular ge_scalarmult -- we have to do tweak*A + zero*B. */
  ge_double_scalarmult_vartime(&Aprime, tweak, &A, zero);
  ge_tobytes(out, &Aprime);

  retval = 0;

 done:
  memwipe(tweak, 0, sizeof(tweak));
  memwipe(&A, 0, sizeof(A));
  memwipe(&Aprime, 0, sizeof(Aprime));
  memwipe(pkcopy, 0, sizeof(pkcopy));

  return retval;
}

/* This is the group order encoded in a format that
 * ge_double_scalarmult_vartime() understands. The group order m is:
 * m = 	 2^252 +  27742317777372353535851937790883648493 =
 *       0x1000000000000000000000000000000014def9dea2f79cd65812631a5cf5d3ed
 */
static const uint8_t modm_m[32] = {0xed,0xd3,0xf5,0x5c,0x1a,0x63,0x12,0x58,
                                   0xd6,0x9c,0xf7,0xa2,0xde,0xf9,0xde,0x14,
                                   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10};

/* Do the scalar multiplication of <b>pubkey</b> with the group order
 * <b>modm_m</b>.  Place the result in <b>out</b> which must be at least 32
 * bytes long. */
int
ed25519_ref10_scalarmult_with_group_order(unsigned char *out,
                                          const unsigned char *pubkey)
{
  unsigned char pkcopy[32];
  unsigned char zero[32] = {0};
  ge_p3 Point;
  ge_p2 Result;

  /* All this is done to fit 'pubkey' in 'Point' so that it can be used by
   * ed25519 ref code. Same thing as in blinding function */
  memcpy(pkcopy, pubkey, 32);
  pkcopy[31] ^= (1<<7);
  if (ge_frombytes_negate_vartime(&Point, pkcopy) != 0) {
    return -1; /* error: bail out */
  }

  /* There isn't a regular scalarmult -- we have to do r = l*P + 0*B */
  ge_double_scalarmult_vartime(&Result, modm_m, &Point, zero);
  ge_tobytes(out, &Result);

  return 0;
}
