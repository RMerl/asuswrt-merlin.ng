/* eccdata.c

   Generate compile time constant (but machine dependent) tables.

   Copyright (C) 2013, 2014, 2017 Niels Möller
   Copyright (C) 2017 Daiki Ueno
   Copyright (C) 2017 Red Hat, Inc.

   This file is part of GNU Nettle.

   GNU Nettle is free software: you can redistribute it and/or
   modify it under the terms of either:

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at your
       option) any later version.

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at your
       option) any later version.

   or both in parallel, as here.

   GNU Nettle is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see http://www.gnu.org/licenses/.
*/

/* Development of Nettle's ECC support was funded by the .SE Internet Fund. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mini-gmp.c"

/* Affine coordinates, for simplicity. Infinity point, i.e., te
   neutral group element, is represented using the is_zero flag. */
struct ecc_point
{
  int is_zero;
  mpz_t x;
  mpz_t y;
};

enum ecc_type
  {
    /* y^2 = x^3 - 3x + b (mod p) */
    ECC_TYPE_WEIERSTRASS,
    /* x^2 + y^2 = 1 - d x^2 y^2 */
    ECC_TYPE_EDWARDS,
    /* -x^2 + y^2 = 1 - d x^2 y^2 */
    ECC_TYPE_TWISTED_EDWARDS,
  };

struct ecc_curve
{
  unsigned bit_size;
  unsigned pippenger_k;
  unsigned pippenger_c;

  enum ecc_type type;

  /* Prime */
  mpz_t p;
  mpz_t b;

  /* Curve order */
  mpz_t q;
  struct ecc_point g;

  /* Table for pippenger's algorithm.
     Element

       i 2^c + j_0 + j_1 2 + j_2 2^2 + ... + j_{c-1} 2^{c-1}

     holds

       2^{ikc} ( j_0 + j_1 2^k + j_2 2^{2k} + ... + j_{c-1} 2^{(c-1)k}) g
   */
  mp_size_t table_size;
  struct ecc_point *table;

  /* If non-NULL, holds 2g, 3g, 4g */
  struct ecc_point *ref;
};

static void
ecc_init (struct ecc_point *p)
{
  mpz_init (p->x);
  mpz_init (p->y);
}

static void
ecc_clear (struct ecc_point *p)
{
  mpz_clear (p->x);
  mpz_clear (p->y);
}

static int
ecc_zero_p (const struct ecc_point *p)
{
  return p->is_zero;
}

static int
ecc_equal_p (const struct ecc_point *p, const struct ecc_point *q)
{
  return p->is_zero ? q->is_zero
    : !q->is_zero && mpz_cmp (p->x, q->x) == 0 && mpz_cmp (p->y, q->y) == 0;
}

static void
ecc_set_zero (const struct ecc_curve *ecc, struct ecc_point *r)
{
  r->is_zero = 1;
  mpz_set_ui (r->x, 0);
  mpz_set_ui (r->y, ecc->type != ECC_TYPE_WEIERSTRASS);
}

static void
ecc_set (struct ecc_point *r, const struct ecc_point *p)
{
  r->is_zero = p->is_zero;
  mpz_set (r->x, p->x);
  mpz_set (r->y, p->y);
}

static void
ecc_add (const struct ecc_curve *ecc, struct ecc_point *r,
	 const struct ecc_point *p, const struct ecc_point *q);

/* Needs to support in-place operation. */
static void
ecc_dup (const struct ecc_curve *ecc,
	 struct ecc_point *r, const struct ecc_point *p)
{
  if (ecc->type != ECC_TYPE_WEIERSTRASS)
    {
      ecc_add (ecc, r, p, p);
      return;
    }
  if (ecc_zero_p (p))
    ecc_set_zero (ecc, r);

  else
    {
      mpz_t m, t, x, y;

      mpz_init (m);
      mpz_init (t);
      mpz_init (x);
      mpz_init (y);

      /* m = (2 y)^-1 */
      mpz_mul_ui (m, p->y, 2);
      mpz_invert (m, m, ecc->p);

      /* t = 3 (x^2 - 1) * m */
      mpz_mul (t, p->x, p->x);
      mpz_mod (t, t, ecc->p);
      mpz_sub_ui (t, t, 1);
      mpz_mul_ui (t, t, 3);

      mpz_mul (t, t, m);
      mpz_mod (t, t, ecc->p);

      /* x' = t^2 - 2 x */
      mpz_mul (x, t, t);
      mpz_submul_ui (x, p->x, 2);

      mpz_mod (x, x, ecc->p);

      /* y' = (x - x') * t - y */
      mpz_sub (y, p->x, x);
      mpz_mul (y, y, t);
      mpz_sub (y, y, p->y);
      mpz_mod (y, y, ecc->p);

      r->is_zero = 0;
      mpz_swap (x, r->x);
      mpz_swap (y, r->y);

      mpz_clear (m);
      mpz_clear (t);
      mpz_clear (x);
      mpz_clear (y);
    }
}

static void
ecc_add (const struct ecc_curve *ecc, struct ecc_point *r,
	 const struct ecc_point *p, const struct ecc_point *q)
{
  if (ecc->type == ECC_TYPE_WEIERSTRASS)
    {
      if (ecc_zero_p (p))
	ecc_set (r, q);

      else if (ecc_zero_p (q))
	ecc_set (r, p);

      else if (mpz_cmp (p->x, q->x) == 0)
	{
	  if (mpz_cmp (p->y, q->y) == 0)
	    ecc_dup (ecc, r, p);
	  else
	    ecc_set_zero (ecc, r);
	}
      else
	{
	  mpz_t s, t, x, y;
	  mpz_init (s);
	  mpz_init (t);
	  mpz_init (x);
	  mpz_init (y);

	  /* t = (q_y - p_y) / (q_x - p_x) */
	  mpz_sub (t, q->x, p->x);
	  mpz_invert (t, t, ecc->p);
	  mpz_sub (s, q->y, p->y);
	  mpz_mul (t, t, s);
	  mpz_mod (t, t, ecc->p);

	  /* x' = t^2 - p_x - q_x */
	  mpz_mul (x, t, t);
	  mpz_sub (x, x, p->x);
	  mpz_sub (x, x, q->x);
	  mpz_mod (x, x, ecc->p);

	  /* y' = (x - x') * t - y */
	  mpz_sub (y, p->x, x);
	  mpz_mul (y, y, t);
	  mpz_sub (y, y, p->y);
	  mpz_mod (y, y, ecc->p);

	  r->is_zero = 0;
	  mpz_swap (x, r->x);
	  mpz_swap (y, r->y);

	  mpz_clear (s);
	  mpz_clear (t);
	  mpz_clear (x);
	  mpz_clear (y);
	}
    }
  else if (ecc->type == ECC_TYPE_EDWARDS)
    {
      mpz_t s, t, x, y;
      mpz_init (s);
      mpz_init (t);
      mpz_init (x);
      mpz_init (y);

      /* t = d p_x p_y q_x q_y */
      mpz_mul (t, ecc->b, p->x);
      mpz_mod (t, t, ecc->p);
      mpz_mul (t, t, p->y);
      mpz_mod (t, t, ecc->p);
      mpz_mul (t, t, q->x);
      mpz_mod (t, t, ecc->p);
      mpz_mul (t, t, q->y);
      mpz_mod (t, t, ecc->p);

      /* x' = (p_x q_y + q_x p_y) / (1 + t) */
      mpz_mul (x, p->x, q->y);
      mpz_mod (x, x, ecc->p);
      mpz_addmul (x, q->x, p->y);
      mpz_mod (x, x, ecc->p);
      mpz_add_ui (s, t, 1);
      mpz_invert (s, s, ecc->p);
      mpz_mul (x, x, s);
      mpz_mod (x, x, ecc->p);

      /* y' = (p_y q_y - p_x q_x) / (1 - t) */
      mpz_mul (y, p->y, q->y);
      mpz_mod (y, y, ecc->p);
      mpz_submul (y, p->x, q->x);
      mpz_mod (y, y, ecc->p);
      mpz_set_ui (s, 1);
      mpz_sub (s, s, t);
      mpz_invert (s, s, ecc->p);
      mpz_mul (y, y, s);
      mpz_mod (y, y, ecc->p);

      mpz_swap (x, r->x);
      mpz_swap (y, r->y);
      r->is_zero = mpz_cmp_ui (r->x, 0) == 0 && mpz_cmp_ui (r->y, 1) == 0;

      mpz_clear (s);
      mpz_clear (t);
      mpz_clear (x);
      mpz_clear (y);
    }
  else
    {
      /* Untwisted:
	 x = (p_x q_y + p_y q_x) / (1 - d p_x p_y q_x q_y)
	 y = (p_y q_y - p_x q_x) / (1 + d p_x p_y q_x q_y)

	 Twisted:
	 x = (p_x q_y + p_y q_x) / (1 - d p_x p_y q_x q_y)
	 y = (p_y q_y + p_x q_x) / (1 + d p_x p_y q_x q_y)

	 So they differ only by a sign in the expression for the new y
	 coordinate.
      */

      mpz_t s, t, x, y;
      mpz_init (s);
      mpz_init (t);
      mpz_init (x);
      mpz_init (y);

      /* t = d p_x p_y q_x q_y */
      mpz_mul (t, ecc->b, p->x);
      mpz_mod (t, t, ecc->p);
      mpz_mul (t, t, p->y);
      mpz_mod (t, t, ecc->p);
      mpz_mul (t, t, q->x);
      mpz_mod (t, t, ecc->p);
      mpz_mul (t, t, q->y);
      mpz_mod (t, t, ecc->p);

      /* x' = (p_x q_y + q_x p_y) / (1 - t) */
      mpz_mul (x, p->x, q->y);
      mpz_mod (x, x, ecc->p);
      mpz_addmul (x, q->x, p->y);
      mpz_mod (x, x, ecc->p);
      mpz_ui_sub (s, 1, t);
      mpz_invert (s, s, ecc->p);
      mpz_mul (x, x, s);
      mpz_mod (x, x, ecc->p);

      /* y' = (p_y q_y - p_x q_x) / (1 + t) */
      mpz_mul (y, p->y, q->y);
      mpz_mod (y, y, ecc->p);
      mpz_addmul (y, p->x, q->x);
      mpz_mod (y, y, ecc->p);
      mpz_add_ui (s, t, 1);
      mpz_invert (s, s, ecc->p);
      mpz_mul (y, y, s);
      mpz_mod (y, y, ecc->p);

      mpz_swap (x, r->x);
      mpz_swap (y, r->y);
      r->is_zero = (mpz_cmp_ui (r->x, 0) == 0 && mpz_cmp_ui (r->y, 1) == 0);

      mpz_clear (s);
      mpz_clear (t);
      mpz_clear (x);
      mpz_clear (y);
    }
}

static void 
ecc_mul_binary (const struct ecc_curve *ecc,
		struct ecc_point *r, const mpz_t n, const struct ecc_point *p)
{
  /* Avoid the mp_bitcnt_t type for compatibility with older GMP
     versions. */
  unsigned k;

  assert (r != p);
  assert (mpz_sgn (n) > 0);

  ecc_set (r, p);

  /* Index of highest one bit */
  for (k = mpz_sizeinbase (n, 2) - 1; k-- > 0; )
    {
      ecc_dup (ecc, r, r);
      if (mpz_tstbit (n, k))
	ecc_add (ecc, r, r, p);
    }  
}

static struct ecc_point *
ecc_alloc (size_t n)
{
  struct ecc_point *p = malloc (n * sizeof(*p));
  size_t i;

  if (!p)
    {
      fprintf (stderr, "Virtual memory exhausted.\n");
      exit (EXIT_FAILURE);
    }
  for (i = 0; i < n; i++)
    ecc_init (&p[i]);

  return p;
}

static void
ecc_set_str (struct ecc_point *p,
	     const char *x, const char *y)
{
  p->is_zero = 0;
  mpz_set_str (p->x, x, 16);
  mpz_set_str (p->y, y, 16);  
}

static void
ecc_curve_init_str (struct ecc_curve *ecc, enum ecc_type type,
		    const char *p, const char *b, const char *q,
		    const char *gx, const char *gy)
{
  ecc->type = type;

  mpz_init_set_str (ecc->p, p, 16);
  mpz_init_set_str (ecc->b, b, 16);
  mpz_init_set_str (ecc->q, q, 16);
  ecc_init (&ecc->g);
  ecc_set_str (&ecc->g, gx, gy);

  ecc->pippenger_k = 0;
  ecc->pippenger_c = 0;
  ecc->table = NULL;

  ecc->ref = NULL;
}

static void
ecc_curve_init (struct ecc_curve *ecc, const char *curve)
{
  if (!strcmp (curve, "secp192r1"))
    {
      ecc_curve_init_str (ecc, ECC_TYPE_WEIERSTRASS,
			  /* p = 2^{192} - 2^{64} - 1 */
			  "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE"
			  "FFFFFFFFFFFFFFFF",

			  "64210519e59c80e70fa7e9ab72243049"
			  "feb8deecc146b9b1", 

			  "ffffffffffffffffffffffff99def836"
			  "146bc9b1b4d22831",

			  "188da80eb03090f67cbf20eb43a18800"
			  "f4ff0afd82ff1012",

			  "07192b95ffc8da78631011ed6b24cdd5"
			  "73f977a11e794811");
      ecc->ref = ecc_alloc (3);
      ecc_set_str (&ecc->ref[0], /* 2 g */
		   "dafebf5828783f2ad35534631588a3f629a70fb16982a888",
		   "dd6bda0d993da0fa46b27bbc141b868f59331afa5c7e93ab");
      
      ecc_set_str (&ecc->ref[1], /* 3 g */
		   "76e32a2557599e6edcd283201fb2b9aadfd0d359cbb263da",
		   "782c37e372ba4520aa62e0fed121d49ef3b543660cfd05fd");

      ecc_set_str (&ecc->ref[2], /* 4 g */
		   "35433907297cc378b0015703374729d7a4fe46647084e4ba",
		   "a2649984f2135c301ea3acb0776cd4f125389b311db3be32");

    }
  else if (!strcmp (curve, "secp224r1"))
    {
      ecc_curve_init_str (ecc, ECC_TYPE_WEIERSTRASS,
			  /* p = 2^{224} - 2^{96} + 1 */
			  "ffffffffffffffffffffffffffffffff"
			  "000000000000000000000001",

			  "b4050a850c04b3abf54132565044b0b7"
			  "d7bfd8ba270b39432355ffb4",

			  "ffffffffffffffffffffffffffff16a2"
			  "e0b8f03e13dd29455c5c2a3d",

			  "b70e0cbd6bb4bf7f321390b94a03c1d3"
			  "56c21122343280d6115c1d21",

			  "bd376388b5f723fb4c22dfe6cd4375a0"
			  "5a07476444d5819985007e34");

      ecc->ref = ecc_alloc (3);
      ecc_set_str (&ecc->ref[0], /* 2 g */
		   "706a46dc76dcb76798e60e6d89474788d16dc18032d268fd1a704fa6",
		   "1c2b76a7bc25e7702a704fa986892849fca629487acf3709d2e4e8bb");
      
      ecc_set_str (&ecc->ref[1], /* 3 g */
		   "df1b1d66a551d0d31eff822558b9d2cc75c2180279fe0d08fd896d04",
		   "a3f7f03cadd0be444c0aa56830130ddf77d317344e1af3591981a925");

      ecc_set_str (&ecc->ref[2], /* 4 g */
		   "ae99feebb5d26945b54892092a8aee02912930fa41cd114e40447301",
		   "482580a0ec5bc47e88bc8c378632cd196cb3fa058a7114eb03054c9");

    }
  else if (!strcmp (curve, "secp256r1"))
    {
      ecc_curve_init_str (ecc, ECC_TYPE_WEIERSTRASS,
			  /* p = 2^{256} - 2^{224} + 2^{192} + 2^{96} - 1 */
			  "FFFFFFFF000000010000000000000000"
			  "00000000FFFFFFFFFFFFFFFFFFFFFFFF",

			  "5AC635D8AA3A93E7B3EBBD55769886BC"
			  "651D06B0CC53B0F63BCE3C3E27D2604B",

			  "FFFFFFFF00000000FFFFFFFFFFFFFFFF"
			  "BCE6FAADA7179E84F3B9CAC2FC632551",

			  "6B17D1F2E12C4247F8BCE6E563A440F2"
			  "77037D812DEB33A0F4A13945D898C296",

			  "4FE342E2FE1A7F9B8EE7EB4A7C0F9E16"
			  "2BCE33576B315ECECBB6406837BF51F5");

      ecc->ref = ecc_alloc (3);
      ecc_set_str (&ecc->ref[0], /* 2 g */
		   "7cf27b188d034f7e8a52380304b51ac3c08969e277f21b35a60b48fc47669978",
		   "7775510db8ed040293d9ac69f7430dbba7dade63ce982299e04b79d227873d1");
      
      ecc_set_str (&ecc->ref[1], /* 3 g */
		   "5ecbe4d1a6330a44c8f7ef951d4bf165e6c6b721efada985fb41661bc6e7fd6c",
		   "8734640c4998ff7e374b06ce1a64a2ecd82ab036384fb83d9a79b127a27d5032");

      ecc_set_str (&ecc->ref[2], /* 4 g */
		   "e2534a3532d08fbba02dde659ee62bd0031fe2db785596ef509302446b030852",
		   "e0f1575a4c633cc719dfee5fda862d764efc96c3f30ee0055c42c23f184ed8c6");

    }
  else if (!strcmp (curve, "secp384r1"))
    {
      ecc_curve_init_str (ecc, ECC_TYPE_WEIERSTRASS,
			  /* p = 2^{384} - 2^{128} - 2^{96} + 2^{32} - 1 */
			  "ffffffffffffffffffffffffffffffff"
			  "fffffffffffffffffffffffffffffffe"
			  "ffffffff0000000000000000ffffffff",
			  
			  "b3312fa7e23ee7e4988e056be3f82d19"
			  "181d9c6efe8141120314088f5013875a"
			  "c656398d8a2ed19d2a85c8edd3ec2aef",
			  
			  "ffffffffffffffffffffffffffffffff"
			  "ffffffffffffffffc7634d81f4372ddf"
			  "581a0db248b0a77aecec196accc52973",
			  
			  "aa87ca22be8b05378eb1c71ef320ad74"
			  "6e1d3b628ba79b9859f741e082542a38"
			  "5502f25dbf55296c3a545e3872760ab7",
			  
			  "3617de4a96262c6f5d9e98bf9292dc29"
			  "f8f41dbd289a147ce9da3113b5f0b8c0"
			  "0a60b1ce1d7e819d7a431d7c90ea0e5f");

      ecc->ref = ecc_alloc (3);
      ecc_set_str (&ecc->ref[0], /* 2 g */
		   "8d999057ba3d2d969260045c55b97f089025959a6f434d651d207d19fb96e9e4fe0e86ebe0e64f85b96a9c75295df61",
		   "8e80f1fa5b1b3cedb7bfe8dffd6dba74b275d875bc6cc43e904e505f256ab4255ffd43e94d39e22d61501e700a940e80");

      ecc_set_str (&ecc->ref[1], /* 3 g */
		   "77a41d4606ffa1464793c7e5fdc7d98cb9d3910202dcd06bea4f240d3566da6b408bbae5026580d02d7e5c70500c831",
		   "c995f7ca0b0c42837d0bbe9602a9fc998520b41c85115aa5f7684c0edc111eacc24abd6be4b5d298b65f28600a2f1df1");

      ecc_set_str (&ecc->ref[2], /* 4 g */
		   "138251cd52ac9298c1c8aad977321deb97e709bd0b4ca0aca55dc8ad51dcfc9d1589a1597e3a5120e1efd631c63e1835",
		   "cacae29869a62e1631e8a28181ab56616dc45d918abc09f3ab0e63cf792aa4dced7387be37bba569549f1c02b270ed67");

    }
  else if (!strcmp (curve, "secp521r1"))
    {
      ecc_curve_init_str (ecc, ECC_TYPE_WEIERSTRASS,
			  "1ff" /* p = 2^{521} - 1 */
			  "ffffffffffffffffffffffffffffffff"
			  "ffffffffffffffffffffffffffffffff"
			  "ffffffffffffffffffffffffffffffff"
			  "ffffffffffffffffffffffffffffffff",

			  "051"
			  "953eb9618e1c9a1f929a21a0b68540ee"
			  "a2da725b99b315f3b8b489918ef109e1"
			  "56193951ec7e937b1652c0bd3bb1bf07"
			  "3573df883d2c34f1ef451fd46b503f00",

			  "1ff"
			  "ffffffffffffffffffffffffffffffff"
			  "fffffffffffffffffffffffffffffffa"
			  "51868783bf2f966b7fcc0148f709a5d0"
			  "3bb5c9b8899c47aebb6fb71e91386409",

			  "c6"
			  "858e06b70404e9cd9e3ecb662395b442"
			  "9c648139053fb521f828af606b4d3dba"
			  "a14b5e77efe75928fe1dc127a2ffa8de"
			  "3348b3c1856a429bf97e7e31c2e5bd66",

			  "118"
			  "39296a789a3bc0045c8a5fb42c7d1bd9"
			  "98f54449579b446817afbd17273e662c"
			  "97ee72995ef42640c550b9013fad0761"
			  "353c7086a272c24088be94769fd16650");

      ecc->ref = ecc_alloc (3);
      ecc_set_str (&ecc->ref[0], /* 2 g */
		   "433c219024277e7e682fcb288148c282747403279b1ccc06352c6e5505d769be97b3b204da6ef55507aa104a3a35c5af41cf2fa364d60fd967f43e3933ba6d783d",
		   "f4bb8cc7f86db26700a7f3eceeeed3f0b5c6b5107c4da97740ab21a29906c42dbbb3e377de9f251f6b93937fa99a3248f4eafcbe95edc0f4f71be356d661f41b02");
      
      ecc_set_str (&ecc->ref[1], /* 3 g */
		   "1a73d352443de29195dd91d6a64b5959479b52a6e5b123d9ab9e5ad7a112d7a8dd1ad3f164a3a4832051da6bd16b59fe21baeb490862c32ea05a5919d2ede37ad7d",
		   "13e9b03b97dfa62ddd9979f86c6cab814f2f1557fa82a9d0317d2f8ab1fa355ceec2e2dd4cf8dc575b02d5aced1dec3c70cf105c9bc93a590425f588ca1ee86c0e5");

      ecc_set_str (&ecc->ref[2], /* 4 g */
		   "35b5df64ae2ac204c354b483487c9070cdc61c891c5ff39afc06c5d55541d3ceac8659e24afe3d0750e8b88e9f078af066a1d5025b08e5a5e2fbc87412871902f3",
		   "82096f84261279d2b673e0178eb0b4abb65521aef6e6e32e1b5ae63fe2f19907f279f283e54ba385405224f750a95b85eebb7faef04699d1d9e21f47fc346e4d0d");

    }
  else if (!strcmp (curve, "curve25519"))
    {
      /* curve25519, y^2 = x^3 + 486662 x^2 + x (mod p), with p = 2^{255} - 19.

	 According to http://cr.yp.to/papers.html#newelliptic, this
	 is birationally equivalent to the Edwards curve

	   x^2 + y^2 = 1 + (121665/121666) x^2 y^2 (mod p).

	   -x^2 + y^2 = 1 - (121665/121666) x^2 y^2, with p = 2^{255} - 19.

	 The generator is
	   x = 0x216936d3cd6e53fec0a4e231fdd6dc5c692cc7609525a7b2c9562d608f25d51a
           y = 0x6666666666666666666666666666666666666666666666666666666666666658

	 Also birationally equivalent to the curve25519 Montgomery curve,

	   y^2 = x^3 + 486662 x^2 + x (mod p)
      */
      ecc_curve_init_str (ecc, ECC_TYPE_TWISTED_EDWARDS,
			  "7fffffffffffffffffffffffffffffff"
			  "ffffffffffffffffffffffffffffffed",
			  /* (121665/121666) mod p, from PARI/GP
			     c = Mod(121665, p); c / (c+1)
			  */
			  "2dfc9311d490018c7338bf8688861767"
			  "ff8ff5b2bebe27548a14b235eca6874a",
			  /* Order of the subgroup is 2^252 + q_0, where
			     q_0 = 27742317777372353535851937790883648493,
			     125 bits.
			  */
			  "10000000000000000000000000000000"
			  "14def9dea2f79cd65812631a5cf5d3ed",
			  /* Generator */
			  "216936d3cd6e53fec0a4e231fdd6dc5c"
			  "692cc7609525a7b2c9562d608f25d51a",
			  "66666666666666666666666666666666"
			  "66666666666666666666666666666658");

      ecc->ref = ecc_alloc (3);
      ecc_set_str (&ecc->ref[0], /* 2 g */
		   "36ab384c9f5a046c3d043b7d1833e7ac"
		   "080d8e4515d7a45f83c5a14e2843ce0e",
		   "2260cdf3092329c21da25ee8c9a21f56"
		   "97390f51643851560e5f46ae6af8a3c9");
      ecc_set_str (&ecc->ref[1], /* 3 g */
		   "67ae9c4a22928f491ff4ae743edac83a"
		   "6343981981624886ac62485fd3f8e25c",
		   "1267b1d177ee69aba126a18e60269ef7"
		   "9f16ec176724030402c3684878f5b4d4");

      ecc_set_str (&ecc->ref[2], /* 4 g */
		   "203da8db56cff1468325d4b87a3520f9"
		   "1a739ec193ce1547493aa657c4c9f870",
		   "47d0e827cb1595e1470eb88580d5716c"
		   "4cf22832ea2f0ff0df38ab61ca32112f");
    }
  else if (!strcmp (curve, "gost_gc256b"))
    {
      ecc_curve_init_str (ecc, ECC_TYPE_WEIERSTRASS,
			  "ffffffffffffffffffffffffffffffff"
			  "fffffffffffffffffffffffffffffd97",

			  "00000000000000000000000000000000"
			  "000000000000000000000000000000a6",

			  "ffffffffffffffffffffffffffffffff"
			  "6c611070995ad10045841b09b761b893",

			  "00000000000000000000000000000000"
			  "00000000000000000000000000000001",

			  "8d91e471e0989cda27df505a453f2b76"
			  "35294f2ddf23e3b122acc99c9e9f1e14");

      ecc->ref = ecc_alloc (3);
      ecc_set_str (&ecc->ref[0], /* 2 g */
		   "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd95",
		   "726e1b8e1f676325d820afa5bac0d489cad6b0d220dc1c4edd5336636160df83");

      ecc_set_str (&ecc->ref[1], /* 3 g */
		   "8e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38d2c",
		   "76bcd1ca9a23b041d4d9baf507a6cd821267a94c838768e8486117796b788a51");

      ecc_set_str (&ecc->ref[2], /* 4 g */
		   "f7063e7063e7063e7063e7063e7063e7063e7063e7063e7063e7063e7063e4b7",
		   "83ccf17ba6706d73625cc3534c7a2b9d6ec1ee6a9a7e07c10d84b388de59f741");

    }
  else if (!strcmp (curve, "gost_gc512a"))
    {
      ecc_curve_init_str (ecc, ECC_TYPE_WEIERSTRASS,
			  "ffffffffffffffffffffffffffffffff"
			  "ffffffffffffffffffffffffffffffff"
			  "ffffffffffffffffffffffffffffffff"
			  "fffffffffffffffffffffffffffffdc7",
			  "e8c2505dedfc86ddc1bd0b2b6667f1da"
			  "34b82574761cb0e879bd081cfd0b6265"
			  "ee3cb090f30d27614cb4574010da90dd"
			  "862ef9d4ebee4761503190785a71c760",
			  "ffffffffffffffffffffffffffffffff"
			  "ffffffffffffffffffffffffffffffff"
			  "27e69532f48d89116ff22b8d4e056060"
			  "9b4b38abfad2b85dcacdb1411f10b275",
			  "00000000000000000000000000000000"
			  "00000000000000000000000000000000"
			  "00000000000000000000000000000000"
			  "00000000000000000000000000000003",
			  "7503cfe87a836ae3a61b8816e25450e6"
			  "ce5e1c93acf1abc1778064fdcbefa921"
			  "df1626be4fd036e93d75e6a50e3a41e9"
			  "8028fe5fc235f5b889a589cb5215f2a4");

      ecc->ref = ecc_alloc (3);
      ecc_set_str (&ecc->ref[0], /* 2 g */
		   "3b89dcfc622996ab97a5869dbff15cf51db00954f43a58a5e5f6b0470a132b2f4434bbcd405d2a9516151d2a6a04f2e4375bf48de1fdb21fb982afd9d2ea137c",
		   "c813c4e2e2e0a8a391774c7903da7a6f14686e98e183e670ee6fb784809a3e92ca209dc631d85b1c7534ed3b37fddf64d854d7e01f91f18bb3fd307591afc051");

      ecc_set_str (&ecc->ref[1], /* 3 g */
		   "a1ff1ab2712a267eb53935ddb5a567f84db156cc096168a1174291d5f488fba543d2840b4d2dd35d764b2f57b308907aec55cfba10544e8416e134687ccb87c3",
		   "3cb5c4417ec4637f30374f189bb5b984c41e3a48d7f84fbfa3819e3f333f7eb311d3af7e67c4c16eeacfac2fe94c6dd4c6366f711a4fb6c7125cd7ec518d90d6");

      ecc_set_str (&ecc->ref[2], /* 4 g */
		   "b7bfb80956c8670031ba191929f64e301d681634236d47a60e571a4bedc0ef257452ef78b5b98dbb3d9f3129d9349433ce2a3a35cb519c91e2d633d7b373ae16",
		   "3bee95e29eecc5d5ad2beba941abcbf9f1cad478df0fecf614f63aeebef77850da7efdb93de8f3df80bc25eac09239c14175f5c29704ce9a3e383f1b3ec0e929");

    }
  else if (!strcmp (curve, "curve448"))
    {
      /* curve448, y^2 = x^3 + 156326 x^2 + x (mod p), with p = 2^{448} - 2^{224} - 1.

	 According to RFC 7748, this is 4-isogenious to the Edwards
	 curve called "edwards448"

	   x^2 + y^2 = 1 - 39081 x^2 y^2 (mod p).

	 And since the constant is not a square, the Edwards formulas
	 should be "complete", with no special cases needed for
	 doubling, neutral element, negatives, etc.

	 Generator is x = 5, with y coordinate
	 355293926785568175264127502063783334808976399387714271831880898435169088786967410002932673765864550910142774147268105838985595290606362,
	 according to

	   x = Mod(5, 2^448-2^224-1); sqrt(x^3 + 156326*x^2 + x)

	 in PARI/GP. Also, in PARI notation,

	   curve448 = Mod([0, 156326, 0, 1, 0], 2^448-2^224-1)
       */
      ecc_curve_init_str (ecc, ECC_TYPE_EDWARDS,
			  "fffffffffffffffffffffffffffffff"
			  "ffffffffffffffffffffffffeffffff"
			  "fffffffffffffffffffffffffffffff"
			  "fffffffffffffffffff",
			  /* -39081 mod p, from PARI/GP
			     c = Mod(-39081, p)
			  */
			  "fffffffffffffffffffffffffffffff"
			  "ffffffffffffffffffffffffeffffff"
			  "fffffffffffffffffffffffffffffff"
			  "fffffffffffffff6756",
			  /* Order of the subgroup is 2^446 - q_0, where
			     q_0 = 13818066809895115352007386748515426880336692474882178609894547503885,
			     224 bits.
			  */
			  "3ffffffffffffffffffffffffffffff"
			  "fffffffffffffffffffffffff7cca23"
			  "e9c44edb49aed63690216cc2728dc58"
			  "f552378c292ab5844f3",
			  "4f1970c66bed0ded221d15a622bf36d"
			  "a9e146570470f1767ea6de324a3d3a4"
			  "6412ae1af72ab66511433b80e18b009"
			  "38e2626a82bc70cc05e",
			  "693f46716eb6bc248876203756c9c76"
			  "24bea73736ca3984087789c1e05a0c2"
			  "d73ad3ff1ce67c39c4fdbd132c4ed7c"
			  "8ad9808795bf230fa14");
      ecc->ref = ecc_alloc (3);
      ecc_set_str (&ecc->ref[0], /* 2 g */
		   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
		   "aaaaaaaaaaaaaaaaaaaaaaa955555555"
		   "55555555555555555555555555555555"
		   "5555555555555555",
		   "ae05e9634ad7048db359d6205086c2b0"
		   "036ed7a035884dd7b7e36d728ad8c4b8"
		   "0d6565833a2a3098bbbcb2bed1cda06b"
		   "daeafbcdea9386ed");
      ecc_set_str (&ecc->ref[1], /* 3 g */
		   "865886b9108af6455bd64316cb694333"
		   "2241b8b8cda82c7e2ba077a4a3fcfe8d"
		   "aa9cbf7f6271fd6e862b769465da8575"
		   "728173286ff2f8f",
		   "e005a8dbd5125cf706cbda7ad43aa644"
		   "9a4a8d952356c3b9fce43c82ec4e1d58"
		   "bb3a331bdb6767f0bffa9a68fed02daf"
		   "b822ac13588ed6fc");

      ecc_set_str (&ecc->ref[2], /* 4 g */
		   "49dcbc5c6c0cce2c1419a17226f929ea"
		   "255a09cf4e0891c693fda4be70c74cc3"
		   "01b7bdf1515dd8ba21aee1798949e120"
		   "e2ce42ac48ba7f30",
		   "d49077e4accde527164b33a5de021b97"
		   "9cb7c02f0457d845c90dc3227b8a5bc1"
		   "c0d8f97ea1ca9472b5d444285d0d4f5b"
		   "32e236f86de51839");
    }
  else
    {
      fprintf (stderr, "No known curve with name %s\n", curve);
      exit(EXIT_FAILURE);
    }
  ecc->bit_size = mpz_sizeinbase (ecc->p, 2);
}

static void
ecc_curve_clear (struct ecc_curve *ecc)
{
  mpz_clear (ecc->p);
  mpz_clear (ecc->b);
  mpz_clear (ecc->q);
  ecc_clear (&ecc->g);
  if (ecc->table)
    {
      size_t i;
      for (i = 0; i < ecc->table_size; i++)
	ecc_clear (&ecc->table[i]);
      free (ecc->table);
    }
  if (ecc->ref)
    {
      size_t i;
      for (i = 0; i < 3; i++)
	ecc_clear (&ecc->ref[i]);
      free (ecc->ref);
    }
}

static unsigned
ecc_table_size(unsigned bits, unsigned k, unsigned c)
{
  unsigned p = (bits + k-1) / k;
  unsigned M = (p + c-1)/c;
  return M;
}

static void
ecc_pippenger_precompute (struct ecc_curve *ecc, unsigned k, unsigned c)
{
  unsigned M = ecc_table_size (ecc->bit_size, k, c);
  unsigned i, j;

  if (M < 2)
    {
      fprintf (stderr, "Invalid parameters, implies M = %u\n", M);
      exit (EXIT_FAILURE);
    }

  if (M == ecc_table_size (ecc->bit_size, k-1, c))
    fprintf(stderr,
	    "warn: Parameters k = %u, c = %d are suboptimal, could use smaller k\n",
	    k, c);

  ecc->pippenger_k = k;
  ecc->pippenger_c = c;
  ecc->table_size = M << c;
  assert (ecc->table_size >= 2);
  ecc->table = ecc_alloc (ecc->table_size);

  /* Compute the first 2^c entries */
  ecc_set_zero (ecc, &ecc->table[0]);
  ecc_set (&ecc->table[1], &ecc->g);

  for (j = 2; j < (1U<<c); j <<= 1)
    {
      /* T[j] = 2^k T[j/2] */
      assert (j < ecc->table_size);
      ecc_dup (ecc, &ecc->table[j], &ecc->table[j/2]);
      for (i = 1; i < k; i++)
	ecc_dup (ecc, &ecc->table[j], &ecc->table[j]);

      for (i = 1; i < j; i++)
	{
	  assert (j + i < ecc->table_size);
	  ecc_add (ecc, &ecc->table[j + i], &ecc->table[j], &ecc->table[i]);
	}
    }
  for (j = 1<<c; j < ecc->table_size; j++)
    {
      /* T[j] = 2^{kc} T[j-2^c] */
      ecc_dup (ecc, &ecc->table[j], &ecc->table[j - (1<<c)]);
      for (i = 1; i < k*c; i++)
	ecc_dup (ecc, &ecc->table[j], &ecc->table[j]);
    }
}

static void
ecc_mul_pippenger (const struct ecc_curve *ecc,
		   struct ecc_point *r, const mpz_t n_input)
{
  mpz_t n;
  unsigned k, c;
  unsigned i, j;
  unsigned bit_rows;

  mpz_init (n);
  
  mpz_mod (n, n_input, ecc->q);
  ecc_set_zero (ecc, r);

  k = ecc->pippenger_k;
  c = ecc->pippenger_c;

  bit_rows = (ecc->bit_size + k - 1) / k;

  for (i = k; i-- > 0; )
    {
      ecc_dup (ecc, r, r);
      for (j = 0; j * c < bit_rows; j++)
	{
	  unsigned bits;
	  mp_size_t bit_index;
	  
	  /* Extract c bits of the exponent, stride k, starting at i + kcj, ending at
	    i + k (cj + c - 1)*/
	  for (bits = 0, bit_index = i + k*(c*j+c); bit_index > i + k*c*j; )
	    {
	      bit_index -= k;
	      bits = (bits << 1) | mpz_tstbit (n, bit_index);
	    }

	  ecc_add (ecc, r, r, &ecc->table[(j << c) | bits]);
	}
    }
  mpz_clear (n);
}

static void
ecc_point_out (FILE *f, const struct ecc_point *p)
{
  if (p->is_zero)
    fprintf (f, "zero");
  else
    {
	fprintf (f, "(");
	mpz_out_str (f, 16, p->x);
	fprintf (f, ",\n     ");
	mpz_out_str (f, 16, (p)->y);
	fprintf (f, ")");
    }
}
#define ASSERT_EQUAL(p, q) do {						\
    if (!ecc_equal_p (p, q))						\
      {									\
	fprintf (stderr, "%s:%d: ASSERT_EQUAL (%s, %s) failed.\n",	\
		 __FILE__, __LINE__, #p, #q);				\
	fprintf (stderr, "p = ");					\
	ecc_point_out (stderr, (p));					\
	fprintf (stderr, "\nq = ");					\
	ecc_point_out (stderr, (q));					\
	fprintf (stderr, "\n");						\
	abort();							\
      }									\
  } while (0)

#define ASSERT_ZERO(p) do {						\
    if (!ecc_zero_p (p))						\
      {									\
	fprintf (stderr, "%s:%d: ASSERT_ZERO (%s) failed.\n",		\
		 __FILE__, __LINE__, #p);				\
	fprintf (stderr, "p = ");					\
	ecc_point_out (stderr, (p));					\
	fprintf (stderr, "\n");						\
	abort();							\
      }									\
  } while (0)

static void
ecc_curve_check (const struct ecc_curve *ecc)
{
  struct ecc_point p, q;
  mpz_t n;

  ecc_init (&p);
  ecc_init (&q);
  mpz_init (n);

  ecc_dup (ecc, &p, &ecc->g);
  if (ecc->ref)
    ASSERT_EQUAL (&p, &ecc->ref[0]);
  else
    {
      fprintf (stderr, "g2 = ");
      mpz_out_str (stderr, 16, p.x);
      fprintf (stderr, "\n     ");
      mpz_out_str (stderr, 16, p.y);
      fprintf (stderr, "\n");
    }
  ecc_add (ecc, &q, &p, &ecc->g);
  if (ecc->ref)
    ASSERT_EQUAL (&q, &ecc->ref[1]);
  else
    {
      fprintf (stderr, "g3 = ");
      mpz_out_str (stderr, 16, q.x);
      fprintf (stderr, "\n     ");
      mpz_out_str (stderr, 16, q.y);
      fprintf (stderr, "\n");
    }

  ecc_add (ecc, &q, &q, &ecc->g);
  if (ecc->ref)
    ASSERT_EQUAL (&q, &ecc->ref[2]);
  else
    {
      fprintf (stderr, "g4 = ");
      mpz_out_str (stderr, 16, q.x);
      fprintf (stderr, "\n     ");
      mpz_out_str (stderr, 16, q.y);
      fprintf (stderr, "\n");
    }

  ecc_dup (ecc, &q, &p);
  if (ecc->ref)
    ASSERT_EQUAL (&q, &ecc->ref[2]);
  else
    {
      fprintf (stderr, "g4 = ");
      mpz_out_str (stderr, 16, q.x);
      fprintf (stderr, "\n     ");
      mpz_out_str (stderr, 16, q.y);
      fprintf (stderr, "\n");
    }

  ecc_mul_binary (ecc, &p, ecc->q, &ecc->g);
  ASSERT_ZERO (&p);

  ecc_mul_pippenger (ecc, &q, ecc->q);
  ASSERT_ZERO (&q);

  ecc_clear (&p);
  ecc_clear (&q);
  mpz_clear (n);
}

static void
output_digits (const mpz_t x,
	       unsigned size, unsigned bits_per_limb)
{  
  mpz_t t;
  mpz_t mask;
  mpz_t limb;
  unsigned i;
  const char *suffix;

  mpz_init (t);
  mpz_init (mask);
  mpz_init (limb);

  mpz_setbit (mask, bits_per_limb);
  mpz_sub_ui (mask, mask, 1);

  suffix = bits_per_limb > 32 ? "ULL" : "UL";

  mpz_init_set (t, x);

  for (i = 0; i < size; i++)
    {
      if ( (i % 8) == 0)
	printf("\n ");
      
      mpz_and (limb, mask, t);
      printf (" 0x");
      mpz_out_str (stdout, 16, limb);
      printf ("%s,", suffix);
      mpz_tdiv_q_2exp (t, t, bits_per_limb);
    }

  mpz_clear (t);
  mpz_clear (mask);
  mpz_clear (limb);
}

static void
output_bignum (const char *name, const mpz_t x,
	       unsigned size, unsigned bits_per_limb)
{  
  printf ("static const mp_limb_t %s[%d] = {", name, size);
  output_digits (x, size, bits_per_limb);
  printf("\n};\n");
}

static void
output_bignum_redc (const char *name, const mpz_t x, const mpz_t p,
		    unsigned size, unsigned bits_per_limb)
{
  mpz_t t;
  mpz_init (t);
  mpz_mul_2exp (t, x, size * bits_per_limb);
  mpz_mod (t, t, p);
  output_bignum (name, t, size, bits_per_limb);
}

static void
output_point (const struct ecc_curve *ecc,
	      const struct ecc_point *p, int use_redc,
	      unsigned size, unsigned bits_per_limb)
{
  mpz_t x, y, t;

  mpz_init (x);
  mpz_init (y);
  mpz_init (t);
 
  mpz_set (x, p->x);
  mpz_set (y, p->y);

  if (use_redc)
    {
      mpz_mul_2exp (x, x, size * bits_per_limb);
      mpz_mod (x, x, ecc->p);
      mpz_mul_2exp (y, y, size * bits_per_limb);
      mpz_mod (y, y, ecc->p);
    }
      
  output_digits (x, size, bits_per_limb);
  output_digits (y, size, bits_per_limb);

  mpz_clear (x);
  mpz_clear (y);
  mpz_clear (t);
}

static unsigned
output_modulo (const char *name, const mpz_t x,
	       unsigned size, unsigned bits_per_limb)
{
  mpz_t mod;
  unsigned bits;

  mpz_init (mod);

  mpz_setbit (mod, bits_per_limb * size);
  mpz_mod (mod, mod, x);

  bits = mpz_sizeinbase (mod, 2);
  output_bignum (name, mod, size, bits_per_limb);
  
  mpz_clear (mod);
  return bits;
}

static void
output_curve (const struct ecc_curve *ecc, unsigned bits_per_limb)
{
  unsigned limb_size = (ecc->bit_size + bits_per_limb - 1)/bits_per_limb;
  unsigned i;
  unsigned bits;
  int redc_limbs;
  mpz_t t;
  mpz_t z;

  mpz_init (t);
  mpz_init (z);

  printf ("/* For NULL. */\n#include <stddef.h>\n");

  printf ("#define ECC_LIMB_SIZE %u\n", limb_size);
  printf ("#define ECC_PIPPENGER_K %u\n", ecc->pippenger_k);
  printf ("#define ECC_PIPPENGER_C %u\n", ecc->pippenger_c);

  output_bignum ("ecc_p", ecc->p, limb_size, bits_per_limb);
  output_bignum ("ecc_b", ecc->b, limb_size, bits_per_limb);
  output_bignum ("ecc_q", ecc->q, limb_size, bits_per_limb);
  
  bits = output_modulo ("ecc_Bmodp", ecc->p, limb_size, bits_per_limb);
  printf ("#define ECC_BMODP_SIZE %u\n",
	  (bits + bits_per_limb - 1) / bits_per_limb);
  bits = output_modulo ("ecc_Bmodq", ecc->q, limb_size, bits_per_limb);
  printf ("#define ECC_BMODQ_SIZE %u\n",
	  (bits + bits_per_limb - 1) / bits_per_limb);
  bits = mpz_sizeinbase (ecc->q, 2);
  if (bits < ecc->bit_size)
    {
      /* for curve25519, with q = 2^k + q', with a much smaller q' */
      unsigned mbits;
      unsigned shift;

      /* Shift to align the one bit at B */
      shift = bits_per_limb * limb_size + 1 - bits;
      
      mpz_set (t, ecc->q);
      mpz_clrbit (t, bits-1);
      mbits = mpz_sizeinbase (t, 2);

      /* The shifted value must be a limb smaller than q. */
      if (mbits + shift + bits_per_limb <= bits)
	{
	  /* q of the form 2^k + q', with q' a limb smaller */
	  mpz_mul_2exp (t, t, shift);
	  output_bignum ("ecc_mBmodq_shifted", t, limb_size, bits_per_limb);
	}
    }

  if (ecc->bit_size < limb_size * bits_per_limb)
    {
      int shift;

      mpz_set_ui (t, 0);
      mpz_setbit (t, ecc->bit_size);
      mpz_sub (t, t, ecc->p);      
      output_bignum ("ecc_Bmodp_shifted", t, limb_size, bits_per_limb);

      shift = limb_size * bits_per_limb - ecc->bit_size;
      if (shift > 0)
	{
	  /* Check condition for reducing hi limbs. If s is the
	     normalization shift and n is the bit size (so that s + n
	     = limb_size * bite_per_limb), then we need

	       (2^n - 1) + (2^s - 1) (2^n - p) < 2p

	     or equivalently,

	       2^s (2^n - p) <= p

	     To a allow a carry limb to be added in at the same time,
	     substitute s+1 for s.
	  */
	  /* FIXME: For ecdsa verify, we actually need the stricter
	     inequality < 2 q. */
	  mpz_mul_2exp (t, t, shift + 1);
	  if (mpz_cmp (t, ecc->p) > 0)
	    {
	      fprintf (stderr, "Reduction condition failed for %u-bit curve.\n",
		       ecc->bit_size);
	      exit (EXIT_FAILURE);
	    }
	}
    }
  else
    printf ("#define ecc_Bmodp_shifted ecc_Bmodp\n");

  if (bits < limb_size * bits_per_limb)
    {
      mpz_set_ui (t, 0);
      mpz_setbit (t, bits);
      mpz_sub (t, t, ecc->q);      
      output_bignum ("ecc_Bmodq_shifted", t, limb_size, bits_per_limb);      
    }
  else
    printf ("#define ecc_Bmodq_shifted ecc_Bmodq\n");

  mpz_add_ui (t, ecc->p, 1);
  mpz_fdiv_q_2exp (t, t, 1);
  output_bignum ("ecc_pp1h", t, limb_size, bits_per_limb);      

  mpz_add_ui (t, ecc->q, 1);
  mpz_fdiv_q_2exp (t, t, 1);
  output_bignum ("ecc_qp1h", t, limb_size, bits_per_limb);  

  /* Trailing zeros in p+1 correspond to trailing ones in p. */
  redc_limbs = mpz_scan0 (ecc->p, 0) / bits_per_limb;
  if (redc_limbs > 0)
    {
      mpz_add_ui (t, ecc->p, 1);
      mpz_fdiv_q_2exp (t, t, redc_limbs * bits_per_limb);
      output_bignum ("ecc_redc_ppm1", t, limb_size - redc_limbs, bits_per_limb);
    }
  else
    {    
      /* Trailing zeros in p-1 correspond to zeros just above the low
	 bit of p */
      redc_limbs = mpz_scan1 (ecc->p, 1) / bits_per_limb;
      if (redc_limbs > 0)
	{
	  printf ("#define ecc_redc_ppm1 (ecc_p + %d)\n",
		  redc_limbs);
	  redc_limbs = -redc_limbs;
	}
      else
	printf ("#define ecc_redc_ppm1 NULL\n");
    }
  printf ("#define ECC_REDC_SIZE %d\n", redc_limbs);

  /* For mod p square root computation. */
  if (mpz_fdiv_ui (ecc->p, 4) == 3)
    {
      /* x = a^{(p+1)/4} gives square root of a (if it exists,
	 otherwise the square root of -a). We use no precomputed
	 values for this. */
    }
  else
    {
      /* p-1 = 2^e s, s odd, t = (s-1)/2*/
      unsigned g, i, e;
      mpz_t s;

      mpz_init (s);

      mpz_sub_ui (s, ecc->p, 1);
      e = mpz_scan1 (s, 0);
      assert (e > 1);

      mpz_fdiv_q_2exp (s, s, e);

      /* Find a non-square g, g^{(p-1)/2} = -1,
	 and z = g^{(p-1)/4 */
      for (g = 2; ; g++)
	{
	  mpz_set_ui (z, g);
	  mpz_powm (z, z, s, ecc->p);
	  mpz_mul (t, z, z);
	  mpz_mod (t, t, ecc->p);

	  for (i = 2; i < e; i++)
	    {
	      mpz_mul (t, t, t);
	      mpz_mod (t, t, ecc->p);
	    }
	  if (mpz_cmp_ui (t, 1) != 0)
	    break;
	}
      mpz_add_ui (t, t, 1);
      assert (mpz_cmp (t, ecc->p) == 0);

      mpz_fdiv_q_2exp (t, s, 1);

      mpz_clear (s);
      printf ("#define ECC_SQRT_E %u\n", e);
    }
  printf ("#if USE_REDC\n");
  printf ("#define ecc_unit ecc_Bmodp\n");
  if (mpz_sgn(z) > 0)
    output_bignum_redc ("ecc_sqrt_z", z, ecc->p, limb_size, bits_per_limb);

  printf ("static const mp_limb_t ecc_table[%lu] = {",
	 (unsigned long) (2*ecc->table_size * limb_size));
  for (i = 0; i < ecc->table_size; i++)
    output_point (ecc, &ecc->table[i], 1, limb_size, bits_per_limb);

  printf("\n};\n");

  printf ("#else\n");

  mpz_set_ui (t, 1);
  output_bignum ("ecc_unit", t, limb_size, bits_per_limb);
  if (mpz_sgn(z) > 0)
    output_bignum ("ecc_sqrt_z", z, limb_size, bits_per_limb);
  
  printf ("static const mp_limb_t ecc_table[%lu] = {",
	 (unsigned long) (2*ecc->table_size * limb_size));
  for (i = 0; i < ecc->table_size; i++)
    output_point (ecc, &ecc->table[i], 0, limb_size, bits_per_limb);

  printf("\n};\n");
  printf ("#endif\n");
  
  mpz_clear (t);
  mpz_clear (z);
}

int
main (int argc, char **argv)
{
  struct ecc_curve ecc;

  if (argc < 4)
    {
      fprintf (stderr, "Usage: %s CURVE K C [BITS-PER-LIMB]\n", argv[0]);
      return EXIT_FAILURE;
    }

  ecc_curve_init (&ecc, argv[1]);

  ecc_pippenger_precompute (&ecc, atoi(argv[2]), atoi(argv[3]));

  fprintf (stderr, "Table size: %lu entries\n",
	   (unsigned long) ecc.table_size);

  ecc_curve_check (&ecc);

  if (argc > 4)
    output_curve (&ecc, atoi(argv[4]));

  ecc_curve_clear (&ecc);
  return EXIT_SUCCESS;
}
