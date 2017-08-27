/*
 * Copyright (c) Dan Harkins, 2012
 *
 *  Copyright holder grants permission for redistribution and use in source
 *  and binary forms, with or without modification, provided that the
 *  following conditions are met:
 *     1. Redistribution of source code must retain the above copyright
 *	notice, this list of conditions, and the following disclaimer
 *	in all source files.
 *     2. Redistribution in binary form must retain the above copyright
 *	notice, this list of conditions, and the following disclaimer
 *	in the documentation and/or other materials provided with the
 *	distribution.
 *
 *  "DISCLAIMER OF LIABILITY
 *
 *  THIS SOFTWARE IS PROVIDED BY DAN HARKINS ``AS IS'' AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INDUSTRIAL LOUNGE BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE."
 *
 * This license and distribution terms cannot be changed. In other words,
 * this code cannot simply be copied and put under a different distribution
 * license (including the GNU public license).
 */

#ifndef _EAP_PWD_H
#define _EAP_PWD_H

RCSIDH(eap_pwd_h, "$Id$")
#include "eap.h"

#include <openssl/bn.h>
#include <openssl/sha.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

typedef struct _pwd_hdr {
    uint8_t lm_exchange;
#define EAP_PWD_EXCH_ID		 1
#define EAP_PWD_EXCH_COMMIT	     2
#define EAP_PWD_EXCH_CONFIRM	    3
//    uint16_t total_length;      /* there if the L-bit is set */
    uint8_t data[0];
} __attribute__ ((packed)) pwd_hdr;

#define EAP_PWD_GET_LENGTH_BIT(x)       ((x)->lm_exchange & 0x80)
#define EAP_PWD_SET_LENGTH_BIT(x)       ((x)->lm_exchange |= 0x80)
#define EAP_PWD_GET_MORE_BIT(x)	 ((x)->lm_exchange & 0x40)
#define EAP_PWD_SET_MORE_BIT(x)	 ((x)->lm_exchange |= 0x40)
#define EAP_PWD_GET_EXCHANGE(x)	 ((x)->lm_exchange & 0x3f)
#define EAP_PWD_SET_EXCHANGE(x,y)       ((x)->lm_exchange |= (y))

typedef struct _pwd_id_packet {
    uint16_t group_num;
    uint8_t random_function;
#define EAP_PWD_DEF_RAND_FUN	    1
    uint8_t prf;
#define EAP_PWD_DEF_PRF		 1
    uint8_t token[4];
    uint8_t prep;
#define EAP_PWD_PREP_NONE	       0
#define EAP_PWD_PREP_MS		 1
#define EAP_PWD_PREP_SASL	       2
    char identity[0];
} __attribute__ ((packed)) pwd_id_packet;

typedef struct _pwd_session_t {
    uint16_t state;
#define PWD_STATE_ID_REQ		1
#define PWD_STATE_COMMIT		2
#define PWD_STATE_CONFIRM	       3
    uint16_t group_num;
    uint32_t ciphersuite;
    uint32_t token;
    char peer_id[MAX_STRING_LEN];
    size_t peer_id_len;
    int mtu;
    uint8_t *in_buf;      /* reassembled fragments */
    int in_buf_pos;
    int in_buf_len;
    uint8_t *out_buf;     /* message to fragment */
    int out_buf_pos;
    int out_buf_len;
    EC_GROUP *group;
    EC_POINT *pwe;
    BIGNUM *order;
    BIGNUM *prime;
    BIGNUM *k;
    BIGNUM *private_value;
    BIGNUM *peer_scalar;
    BIGNUM *my_scalar;
    EC_POINT *my_element;
    EC_POINT *peer_element;
    uint8_t my_confirm[SHA256_DIGEST_LENGTH];
} pwd_session_t;

int compute_password_element(pwd_session_t *sess, uint16_t grp_num,
			     char const *password, int password_len,
			     char *id_server, int id_server_len,
			     char *id_peer, int id_peer_len,
			     uint32_t *token);
int compute_scalar_element(pwd_session_t *sess, BN_CTX *bnctx);
int process_peer_commit (pwd_session_t *sess, uint8_t *commit, BN_CTX *bnctx);
int compute_server_confirm(pwd_session_t *sess, uint8_t *buf, BN_CTX *bnctx);
int compute_peer_confirm(pwd_session_t *sess, uint8_t *buf, BN_CTX *bnctx);
int compute_keys(pwd_session_t *sess, uint8_t *peer_confirm,
		 uint8_t *msk, uint8_t *emsk);
#ifdef PRINTBUF
void print_buf(char *str, uint8_t *buf, int len);
#endif  /* PRINTBUF */

#endif  /* _EAP_PWD_H */
