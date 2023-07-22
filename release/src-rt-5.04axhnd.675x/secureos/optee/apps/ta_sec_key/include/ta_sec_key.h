/*
<:copyright-BRCM:2018:DUAL/GPL:standard 

   Copyright (c) 2018 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:> 
*/
#ifndef TA_SEC_KEY_H
#define TA_SEC_KEY_H

/* This UUID is generated with uuidgen
   the ITU-T UUID generator at http://www.itu.int/ITU-T/asn1/uuid.html */
#define TA_SEC_KEY_UUID { 0x8aaaf201, 0x2450, 0x11e4, \
      { 0xab, 0xe2, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b} }

typedef struct{
  uint32_t flag;
  uint32_t priv_len;
  uint32_t publ_len;
}ObjectHeader;

typedef enum{
  SK_SESSION_SYMMETRIC_CIPHER,
  SK_SESSION_SYMMETRIC_AUTHENC,
  SK_SESSION_SYMMETRIC_AUTHDEC,
  SK_SESSION_ASYMMETRIC_ENCRYPT,
  SK_SESSION_ASYMMETRIC_DECRYPT,
  SK_SESSION_ASYMMETRIC_SIGN,
  SK_SESSION_ASYMMETRIC_VERIFY
}SK_Session;

typedef struct{
  TEE_ObjectHandle    obj_handle;
  TEE_ObjectHandle    key_handle;
  TEE_OperationHandle op_handle;
  SK_Session          session;
}SessionHandle;

#define RSA_MAX_MOD_BYTES (512)
typedef struct rsa_key_len{
  uint32_t n_len;
  uint32_t e_len;
  uint32_t d_len;
  uint32_t p_len;
  uint32_t q_len;
  uint32_t p1_len;
  uint32_t q1_len;
  uint32_t p1q1_len;
  /* NOTE: Order of the fields is important */
}rsa_key_len_t;

typedef struct rsa_key_s {
  uint8_t n[RSA_MAX_MOD_BYTES];            // modulus
  uint8_t e[RSA_MAX_MOD_BYTES];            // public exponent
  uint8_t d[RSA_MAX_MOD_BYTES];            // private exponent
  uint8_t p[RSA_MAX_MOD_BYTES >> 1];       // prime1
  uint8_t q[RSA_MAX_MOD_BYTES >> 1];       // prime2
  uint8_t p1[RSA_MAX_MOD_BYTES >> 1];      // exponent1
  uint8_t q1[RSA_MAX_MOD_BYTES >> 1];      // exponent2
  uint8_t p1q1[RSA_MAX_MOD_BYTES >> 1];    // coefficient
  rsa_key_len_t size;
} rsa_key_t;

#define MAX_SESSION                  2

#define OBJECT_FLAG_LOCK           0x80000000
#define OBJECT_FLAG_PRIV_KEY       0x00000001
#define OBJECT_FLAG_ASYM_KEY       0x00000002
#define OBJECT_FLAG_PRIV_MASK      0x0000000F
#define OBJECT_FLAG_PUB_MASK       0x000000F0
#define OBJECT_FLAG_PUB_KEY        0x00000010
#define OBJECT_FLAG_PUB_CERT       0x00000020

#define CMN_TEE_FLAG              ( TEE_DATA_FLAG_ACCESS_WRITE |        \
                                    TEE_DATA_FLAG_ACCESS_READ |         \
                                    TEE_DATA_FLAG_ACCESS_WRITE_META |   \
                                    TEE_DATA_FLAG_OVERWRITE )


#define SK_MAX_OBJ_LEN           32

#define TA_SK_CREATE_OBJECT      0
#define TA_SK_OPEN_OBJECT        1
#define TA_SK_CLOSE_OBJECT       2
#define TA_SK_SET_PRIV_KEY       4
#define TA_SK_SET_PUBL_KEY       5
#define TA_SK_SET_CERT           6
#define TA_SK_GET_PUBL_KEY       7
#define TA_SK_GET_CERT           8
#define TA_SK_GET_LENGTH         9
#define TA_SK_DELETE_OBJECT      10
#define TA_SK_CRYPTO_INIT        11
#define TA_SK_CRYPTO_UPDATE      12
#define TA_SK_CRYPTO_FINAL       13
#define TA_SK_GENERATE_KEY       14
#define TA_SK_GENERATE_KEY_PAIR  15
#define TA_SK_CRYPTO_SIGN        16
#define TA_SK_CRYPTO_VERIFY      17

#define TA_SK_OBJ_DEBUG          18

TEE_Result sk_open_object (TEE_Param params[4], TEE_ObjectHandle *handle);
TEE_Result sk_create_object (TEE_Param params[4], TEE_ObjectHandle *handle);
TEE_Result sk_get_object_header (TEE_ObjectHandle handle, ObjectHeader *header);
TEE_Result sk_register_session (TEE_ObjectHandle object, uint32_t *ses_id);
TEE_Result sk_find_session (int ses_id, SessionHandle **ses);
TEE_Result sk_extract_key_from_pem(uint8_t *pem_data, int pem_len, uint8_t *key_data, uint32_t *key_len);
TEE_Result sk_parse_publ_key(uint8_t *data_ptr, size_t data_len, uint8_t **mod, uint32_t *m_size, uint8_t **exp, uint32_t *e_size);
TEE_Result sk_parse_priv_key(uint8_t *p, int size, rsa_key_t *key);
TEE_Result sk_get_rsa_key(SessionHandle *ses_handle, rsa_key_t *rsa_key);
TEE_Result sk_alert(void);
int sk_contains_symmetric_key(ObjectHeader *header);
int sk_contains_asymmetric_key(ObjectHeader *header);
uint8_t*   sk_get_private_key (TEE_ObjectHandle handle);
#endif /*TA_SEC_KEY_H*/
