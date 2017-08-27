/*
 * eapcrypto.c      Common key derivation routines for EAP/SIM.
 *
 * The development of the EAP/SIM support was funded by Internet Foundation
 * Austria (http://www.nic.at/ipa).
 *
 * Version:     $Id$
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2003  Michael Richardson <mcr@sandelman.ottawa.on.ca>
 * Copyright 2003,2006  The FreeRADIUS server project
 *
 */

RCSID("$Id$")

#include <stdio.h>
#include <stdlib.h>

#include "eap_types.h"
#include "eap_sim.h"
#include <freeradius-devel/sha1.h>

void eapsim_calculate_keys(struct eapsim_keys *ek)
{
	fr_SHA1_CTX context;
	uint8_t fk[160];
	unsigned char buf[256];
	unsigned char *p;
	unsigned int  blen;

	p = buf;
	memcpy(p, ek->identity, ek->identitylen);   p = p+ek->identitylen;
	memcpy(p, ek->Kc[0], EAPSIM_Kc_SIZE);       p = p+EAPSIM_Kc_SIZE;
	memcpy(p, ek->Kc[1], EAPSIM_Kc_SIZE);       p = p+EAPSIM_Kc_SIZE;
	memcpy(p, ek->Kc[2], EAPSIM_Kc_SIZE);       p = p+EAPSIM_Kc_SIZE;
	memcpy(p, ek->nonce_mt, sizeof(ek->nonce_mt)); p=p+sizeof(ek->nonce_mt);
	memcpy(p, ek->versionlist, ek->versionlistlen);p=p+ek->versionlistlen;
	memcpy(p, ek->versionselect, sizeof(ek->versionselect)); p=p+sizeof(ek->versionselect);
	/* *p++ = ek->versionselect[1]; */

	blen = p - buf;

#if defined(TEST_CASE) || defined(DUMP_EAPSIM_KEYS)
	{
	  unsigned int i, j, k;

	  j=0; k=0;

	  printf("SHA1buffer was: ");
	  for (i = 0; i < blen; i++) {
	    if(j==4) {
	      printf("_");
	      j=0;
	    }
	    if(k==20) {
	      printf("\n		");
	      k=0;
	      j=0;
	    }
	    j++;
	    k++;

	    printf("%02x", buf[i]);
	  }
	  printf("\n");
	}
#endif


	/* do the master key first */
	fr_SHA1Init(&context);
	fr_SHA1Update(&context, buf, blen);
	fr_SHA1Final(ek->master_key, &context);

	/*
	 * now use the PRF to expand it, generated K_aut, K_encr,
	 * MSK and EMSK.
	 */
	fips186_2prf(ek->master_key, fk);

	/* split up the result */
	memcpy(ek->K_encr, fk +  0, 16);    /* 128 bits for encryption    */
	memcpy(ek->K_aut,  fk + 16, EAPSIM_AUTH_SIZE); /*128 bits for auth */
	memcpy(ek->msk,    fk + 32, 64);  /* 64 bytes for Master Session Key */
	memcpy(ek->emsk,   fk + 96, 64);  /* 64- extended Master Session Key */
}


void eapsim_dump_mk(struct eapsim_keys *ek)
{
	unsigned int i, j, k;

	j=0; k=0;

	printf("Input was: \n");
	printf("   identity: (len=%d)", ek->identitylen);
	for (i = 0; i < ek->identitylen; i++) {
		printf("%02x", ek->identity[i]);
	}

	printf("\n   nonce_mt: ");
	for (i = 0; i < EAPSIM_NONCEMT_SIZE; i++) {
		printf("%02x", ek->nonce_mt[i]);
	}

	for (k = 0; k<3; k++) {
		printf("\n   rand%d: ", k);
		for (i = 0; i < EAPSIM_RAND_SIZE; i++) {
			printf("%02x", ek->rand[k][i]);
		}
	}

	for (k = 0; k<3; k++) {
		printf("\n   sres%d: ", k);
		for (i = 0; i < EAPSIM_SRES_SIZE; i++) {
			printf("%02x", ek->sres[k][i]);
		}
	}

	for (k = 0; k<3; k++) {
		printf("\n   Kc%d: ", k);
		for (i = 0; i < EAPSIM_Kc_SIZE; i++) {
			printf("%02x", ek->Kc[k][i]);
		}
	}

	printf("\n   versionlist[%d]: ",ek->versionlistlen);
	for (i = 0; i < ek->versionlistlen; i++) {
		printf("%02x", ek->versionlist[i]);
	}

	printf("\n   select %02x %02x\n",
	       ek->versionselect[0],
	       ek->versionselect[1]);

	printf("\n\nOutput\n");

	printf("mk:	 ");
	j=0; k=0;
	for (i = 0; i < sizeof(ek->master_key); i++) {
		if(j==4) {
			printf("_");
			j=0;
		}
		j++;

		printf("%02x", ek->master_key[i]);
	}

	printf("\nK_aut:      ");
	j=0; k=0;
	for (i = 0; i < sizeof(ek->K_aut); i++) {
		if(j==4) {
			printf("_");
			j=0;
		}
		j++;

		printf("%02x", ek->K_aut[i]);
	}

	printf("\nK_encr:     ");
	j=0; k=0;
	for (i = 0; i < sizeof(ek->K_encr); i++) {
		if(j==4) {
			printf("_");
			j=0;
		}
		j++;

		printf("%02x", ek->K_encr[i]);
	}

	printf("\nmsk:	");
	j=0; k=0;
	for (i = 0; i < sizeof(ek->msk); i++) {
		if(k==20) {
			printf("\n	    ");
			k=0;
			j=0;
		}
		if(j==4) {
			printf("_");
			j=0;
		}
		k++;
		j++;

		printf("%02x", ek->msk[i]);
	}
	printf("\nemsk:       ");
	j=0; k=0;
	for (i = 0; i < sizeof(ek->emsk); i++) {
		if(k==20) {
			printf("\n	    ");
			k=0;
			j=0;
		}
		if(j==4) {
			printf("_");
			j=0;
		}
		k++;
		j++;

		printf("%02x", ek->emsk[i]);
	}
	printf("\n");
}

#ifdef TEST_CASE

#include <assert.h>

struct eapsim_keys inputkey1 = {
	{'e', 'a', 'p', 's','i','m' },
	6,
	  0x4d, 0x6c, 0x40, 0xde, 0x48, 0x3a, 0xdd, 0x99,   /* nonce_mt */
	  0x50, 0x90, 0x2c, 0x40, 0x24, 0xce, 0x76, 0x5e,
	  0x89, 0xab, 0xcd, 0xef, 0x89, 0xab, 0xcd, 0xef,   /* chalX */
	  0x89, 0xab, 0xcd, 0xef, 0x89, 0xab, 0xcd, 0xef,
	  0x9a, 0xbc, 0xde, 0xf8, 0x9a, 0xbc, 0xde, 0xf8,
	  0x9a, 0xbc, 0xde, 0xf8, 0x9a, 0xbc, 0xde, 0xf8,
	  0xab, 0xcd, 0xef, 0x89, 0xab, 0xcd, 0xef, 0x89,
	  0xab, 0xcd, 0xef, 0x89, 0xab, 0xcd, 0xef, 0x89,
	  0x12, 0x34, 0xab, 0xcd,			     /* sresX */
	  0x12, 0x34, 0xab, 0xcd,
	  0x23, 0x4a, 0xbc, 0xd1,
	  0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,  /* Kc */
	  0x10, 0x21, 0x32, 0x43, 0x54, 0x65, 0x76, 0x87,
	  0x30, 0x41, 0x52, 0x63, 0x74, 0x85, 0x96, 0xa7,
	  {0x00, 0x02, 0x00, 0x01},
	  4,
	  0x00, 0x01 ,
};

struct eapsim_keys inputkey2 = {
  {'1','2','4','4','0','7','0','1','0','0','0','0','0','0','0','1','@','e','a','p','s','i','m','.','f','o','o'},
  27,
  0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,   /* nonce_mt */
  0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,   /* chalX */
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
  0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
  0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,

  0xd1, 0xd2, 0xd3, 0xd4,  /* SRES 1 */
  0xe1, 0xe2, 0xe3, 0xe4,
  0xf1, 0xf2, 0xf3, 0xf4,

  0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,   /* Kc */
  0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
  0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
  /*   {0x00, 0x02, 0x00, 0x01}, */
  {0x00, 0x01},
  2,
  0x00, 0x01 ,
};



main(int argc, char *argv[])
{
	struct eapsim_keys *ek;

	ek = &inputkey1;

	eapsim_calculate_keys(ek);
	eapsim_dump_mk(ek);

	ek = &inputkey2;

	eapsim_calculate_keys(ek);
	eapsim_dump_mk(ek);
}
#endif






/*
 * Local Variables:
 * c-style: bsd
 * End:
 */
