/*
 * eap_sim.h    Header file containing the EAP-SIM types
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
 * Copyright 2006  The FreeRADIUS server project
 *
 */
#ifndef _EAP_SIM_H
#define _EAP_SIM_H

RCSIDH(eap_sim_h, "$Id$")

#include "eap_types.h"

#define EAP_SIM_VERSION 0x0001

/* base for dictionary values */
#define ATTRIBUTE_EAP_SIM_BASE      (6*256)

#define ATTRIBUTE_EAP_SIM_SUBTYPE   	1200
#define ATTRIBUTE_EAP_SIM_RAND1	 1201
#define ATTRIBUTE_EAP_SIM_RAND2	 1202
#define ATTRIBUTE_EAP_SIM_RAND3	 1203

#define ATTRIBUTE_EAP_SIM_SRES1	 1204
#define ATTRIBUTE_EAP_SIM_SRES2	 1205
#define ATTRIBUTE_EAP_SIM_SRES3	 1206

#define ATTRIBUTE_EAP_SIM_STATE	 1207
#define ATTRIBUTE_EAP_SIM_IMSI	  1208
#define ATTRIBUTE_EAP_SIM_HMAC	  1209
#define ATTRIBUTE_EAP_SIM_KEY	   1210
#define ATTRIBUTE_EAP_SIM_EXTRA	 1211

#define ATTRIBUTE_EAP_SIM_KC1	   1212
#define ATTRIBUTE_EAP_SIM_KC2	   1213
#define ATTRIBUTE_EAP_SIM_KC3	   1214

enum eapsim_subtype {
  eapsim_start       = 10,
  eapsim_challenge   = 11,
  eapsim_notification= 12,
  eapsim_reauth      = 13,
  eapsim_client_error = 14,
  eapsim_max_subtype = 15
};

enum eapsim_clientstates {
  eapsim_client_init = 0,
  eapsim_client_start = 1,
  eapsim_client_maxstates
};

/* server states
 *
 * in server_start, we send a EAP-SIM Start message.
 *
 */
enum eapsim_serverstates {
  eapsim_server_start = 0,
  eapsim_server_challenge=1,
  eapsim_server_success=10,
  eapsim_server_maxstates
};

#define PW_EAP_SIM_RAND		 1
#define PW_EAP_SIM_PADDING	      6
#define PW_EAP_SIM_NONCE_MT	     7
#define PW_EAP_SIM_PERMANENT_ID_REQ    10
#define PW_EAP_SIM_MAC		 11
#define PW_EAP_SIM_NOTIFICATION	12
#define PW_EAP_SIM_ANY_ID_REQ	  13
#define PW_EAP_SIM_IDENTITY	    14
#define PW_EAP_SIM_VERSION_LIST	15
#define PW_EAP_SIM_SELECTED_VERSION    16
#define PW_EAP_SIM_FULLAUTH_ID_REQ     17
#define PW_EAP_SIM_COUNTER	     19
#define PW_EAP_SIM_COUNTER_TOO_SMALL   20
#define PW_EAP_SIM_NONCE_S	     21
#define PW_EAP_SIM_IV		 129
#define PW_EAP_SIM_ENCR_DATA	  130
#define PW_EAP_SIM_NEXT_PSEUDONUM     132
#define PW_EAP_SIM_NEXT_REAUTH_ID     133
#define PW_EAP_SIM_CHECKCODE	  134

/*
 * interfaces in eapsimlib.c
 */
extern int map_eapsim_basictypes(RADIUS_PACKET *r, eap_packet_t *ep);
extern char const *sim_state2name(enum eapsim_clientstates state, char *buf, int buflen);
extern char const *sim_subtype2name(enum eapsim_subtype subtype, char *buf, int buflen);
extern int unmap_eapsim_basictypes(RADIUS_PACKET *r,
				   uint8_t *attr, unsigned int attrlen);


/************************/
/*   CRYPTO FUNCTIONS   */
/************************/

/*
 * key derivation functions/structures
 *
 */

#define EAPSIM_SRES_SIZE 4
#define EAPSIM_RAND_SIZE 16
#define EAPSIM_Kc_SIZE   8
#define EAPSIM_CALCMAC_SIZE 20
#define EAPSIM_NONCEMT_SIZE 16
#define EAPSIM_AUTH_SIZE    16

struct eapsim_keys {
  /* inputs */
  unsigned char identity[MAX_STRING_LEN];
  unsigned int  identitylen;
  unsigned char nonce_mt[EAPSIM_NONCEMT_SIZE];
  unsigned char rand[3][EAPSIM_RAND_SIZE];
  unsigned char sres[3][EAPSIM_SRES_SIZE];
  unsigned char Kc[3][EAPSIM_Kc_SIZE];
  unsigned char versionlist[MAX_STRING_LEN];
  unsigned char versionlistlen;
  unsigned char versionselect[2];

  /* outputs */
  unsigned char master_key[20];
  unsigned char K_aut[EAPSIM_AUTH_SIZE];
  unsigned char K_encr[16];
  unsigned char msk[64];
  unsigned char emsk[64];
};


/*
 * interfaces in eapsimlib.c
 */
extern int  eapsim_checkmac(TALLOC_CTX *ctx, VALUE_PAIR *rvps,
			    uint8_t key[8],
			    uint8_t *extra, int extralen,
			    uint8_t calcmac[20]);

/*
 * in eapcrypto.c
 */
extern void eapsim_calculate_keys(struct eapsim_keys *ek);
extern void eapsim_dump_mk(struct eapsim_keys *ek);


#endif /* _EAP_SIM_H */
