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

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include "ta_sec_key.h"
#include <tee_api.h>
#include <string.h>

int map_algo[] = { TEE_ALG_AES_CTR,
                   TEE_ALG_AES_CTS,
                   TEE_ALG_AES_CBC_NOPAD,
                   TEE_ALG_AES_GCM,
                   TEE_ALG_AES_CCM,
                   TEE_ALG_RSA_NOPAD,
                   TEE_ALG_RSASSA_PKCS1_V1_5_SHA1,
                   TEE_ALG_RSASSA_PKCS1_V1_5_SHA256,
                   TEE_ALG_RSASSA_PKCS1_V1_5_SHA256};

int map_mode[] = {TEE_MODE_ENCRYPT, TEE_MODE_DECRYPT, TEE_MODE_SIGN, TEE_MODE_VERIFY};

SessionHandle session[MAX_SESSION] = {0};

/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */

TEE_Result TA_CreateEntryPoint(void)
{
   TEE_MemFill(session, 0, sizeof(session));
   return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
   DMSG ("TA: Destroying entrypoint \n");
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
                                    TEE_Param  params[4], void **sess_ctx)
{
   uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
                                              TEE_PARAM_TYPE_NONE,
                                              TEE_PARAM_TYPE_NONE,
                                              TEE_PARAM_TYPE_NONE);

   if (param_types != exp_param_types)
      return TEE_ERROR_BAD_PARAMETERS;

   /* Unused parameters */
   (void)&params;
   (void)&sess_ctx;

   return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void *sess_ctx)
{
   (void)&sess_ctx; /* Unused parameter */
   DMSG ("TA: Session closing\n");
}


TEE_Result TA_InvokeCommandEntryPoint(void *sess_ctx, uint32_t cmd_id,
                                      uint32_t param_types, TEE_Param params[4])
{
  ObjectHeader        header;
  SessionHandle       *ses_handle = NULL;
  uint32_t            read_bytes = 0;
  uint8_t             *obj_content = NULL;
  uint8_t             *key_data = NULL;
  TEE_Result          result = TEE_ERROR_GENERIC;
  TEE_Attribute       secret_key[8];
  TEE_ObjectHandle    obj_handle;
  void                *ta_param;
  (void)&sess_ctx; /* Unused parameter */
  (void)param_types;

  /* Extract the object handle */
  switch (cmd_id){
  case TA_SK_CREATE_OBJECT:
  case TA_SK_OPEN_OBJECT:
    /* Object handle yet to be created */
    break;
  default:
    if (sk_find_session(params[0].value.a, &ses_handle) != TEE_SUCCESS){
      DMSG("TA: Session not found\n");
      return TEE_ERROR_ITEM_NOT_FOUND;
    }
  }

  /* Start processing the commands */
  switch (cmd_id) {

  /****************************************************************
   * Handle request to create a new object
   ****************************************************************/
  case TA_SK_CREATE_OBJECT:
    if ((result = sk_create_object(params, &obj_handle)) == TEE_SUCCESS){
       uint32_t ses_id;
       if ((result = sk_register_session(obj_handle, &ses_id)) == TEE_SUCCESS){
          /* Let the client know the valid object handle */
          params[1].value.a = ses_id;
       }
    }
    break;

  /****************************************************************
   * Handle request to open an existing object
   ****************************************************************/
  case TA_SK_OPEN_OBJECT:
    DMSG("TA: Request to open the object \n");
    if ((result = sk_open_object(params, &obj_handle)) == TEE_SUCCESS){
       uint32_t ses_id;
       if ((result = sk_register_session(obj_handle, &ses_id)) == TEE_SUCCESS){
          /* Let the client know the valid object handle */
          params[1].value.a = ses_id;
          DMSG("TA: Able to open the object \n");
       }
    }
    break;


  /****************************************************************
   * Handle request to close an existing object
   ****************************************************************/
  case TA_SK_CLOSE_OBJECT:
    TEE_CloseObject(ses_handle->obj_handle);
    ses_handle->obj_handle = TEE_HANDLE_NULL;

    if (ses_handle->key_handle) {
       TEE_FreeTransientObject(ses_handle->key_handle);
       ses_handle->key_handle = TEE_HANDLE_NULL;
    }

    if(ses_handle->op_handle){
       TEE_FreeOperation(ses_handle->op_handle);
       ses_handle->op_handle = TEE_HANDLE_NULL;
    }
    result = TEE_SUCCESS;
    break;

  /****************************************************************
   * Handle request to assign a private key to an object
   ****************************************************************/
  case TA_SK_GENERATE_KEY:
  case TA_SK_SET_PRIV_KEY:
    /* Read the object header */
    if((result = sk_get_object_header(ses_handle->obj_handle, &header)) == TEE_SUCCESS){
       uint32_t key_len = (cmd_id == TA_SK_GENERATE_KEY) ? (params[1].value.a + 7) / 8 : params[1].memref.size;

       if (header.flag & OBJECT_FLAG_LOCK) {
          EMSG("ERROR: Object cannot be updated\n");
          result = TEE_ERROR_ACCESS_DENIED;
          break;
       }

       /* Update object header */
       header.flag &= ~(OBJECT_FLAG_PRIV_MASK | OBJECT_FLAG_PUB_MASK);
       header.flag |= OBJECT_FLAG_PRIV_KEY;
       header.priv_len = key_len;
       TEE_SeekObjectData(ses_handle->obj_handle, 0, TEE_DATA_SEEK_SET);
       if((result |= TEE_WriteObjectData(ses_handle->obj_handle, &header, sizeof(header))) != TEE_SUCCESS)
          break;

       /* Generate private key and IV */
       if (cmd_id == TA_SK_GENERATE_KEY){
          uint8_t *random_bits = NULL;

          /* Allocate memory to hold random contents */
          random_bits = TEE_Malloc(key_len, TEE_MALLOC_FILL_ZERO);
          /* Now generate and save random key */
          TEE_GenerateRandom(random_bits, key_len);
          result |= TEE_WriteObjectData(ses_handle->obj_handle, random_bits, key_len);
          /* Now generate and save 16 bytes IV */
          TEE_GenerateRandom(random_bits, 16);
          result |= TEE_WriteObjectData(ses_handle->obj_handle, random_bits, 16);
          TEE_Free(random_bits);
       }
       /* Save imported key and IV */
       else{
          /* Save the key */
	  ta_param = TEE_Malloc(params[1].memref.size, TEE_MALLOC_FILL_ZERO);
	  memcpy(ta_param, params[1].memref.buffer, params[1].memref.size);
          result |= TEE_WriteObjectData(ses_handle->obj_handle, ta_param, params[1].memref.size);
	  TEE_Free(ta_param);
          /* Save the IV */
          if (params[2].memref.buffer && params[2].memref.size) {
	     ta_param = TEE_Malloc(params[2].memref.size, TEE_MALLOC_FILL_ZERO);
	     memcpy(ta_param, params[2].memref.buffer, params[2].memref.size);
             result |= TEE_WriteObjectData(ses_handle->obj_handle, ta_param, params[2].memref.size);
	     TEE_Free(ta_param);
	  }
          else{
             uint8_t null_iv[16];

             TEE_MemFill(null_iv, 0, 16);
             result |= TEE_WriteObjectData(ses_handle->obj_handle, null_iv, 16);
          }
       }
    }
    break;

  /****************************************************************
   * Handle request to generate private/public key
   ****************************************************************/
  case TA_SK_GENERATE_KEY_PAIR:
    if((result = sk_get_object_header(ses_handle->obj_handle, &header)) == TEE_SUCCESS){
       TEE_ObjectHandle key;
       rsa_key_len_t rsa_len;
       uint8_t *rsa_keys = TEE_Malloc(8 * RSA_MAX_MOD_BYTES, TEE_MALLOC_FILL_ZERO);
       uint32_t key_len;

       if (header.flag & OBJECT_FLAG_LOCK) {
          EMSG("ERROR: Object cannot be updated\n");
          result = TEE_ERROR_ACCESS_DENIED;
          break;
       }

       /* Update object header */
       header.flag &= ~(OBJECT_FLAG_PUB_MASK | OBJECT_FLAG_PRIV_MASK);
       header.flag |= OBJECT_FLAG_ASYM_KEY;
       /* Only create RSA key pair */
       result = TEE_AllocateTransientObject(TEE_TYPE_RSA_KEYPAIR, params[1].value.a, &key);
       if (result) {
          EMSG("ERROR: Object allocation failed\n");
          break;
       }
       /* Generate the key */
       result = TEE_GenerateKey(key, params[1].value.a, NULL, 0);
       if (result) {
          EMSG("ERROR: Key pair generation failed\n");
          break;
       }
       /* Now extract the key attributes and save */
       key_len = 0;
       rsa_len.n_len = RSA_MAX_MOD_BYTES;
       TEE_GetObjectBufferAttribute(key, TEE_ATTR_RSA_MODULUS, rsa_keys + key_len, &rsa_len.n_len);
       key_len += rsa_len.n_len;
       rsa_len.e_len = RSA_MAX_MOD_BYTES;
       TEE_GetObjectBufferAttribute(key, TEE_ATTR_RSA_PUBLIC_EXPONENT, rsa_keys + key_len, &rsa_len.e_len);
       key_len += rsa_len.e_len;
       rsa_len.d_len = RSA_MAX_MOD_BYTES;
       TEE_GetObjectBufferAttribute(key, TEE_ATTR_RSA_PRIVATE_EXPONENT, rsa_keys + key_len, &rsa_len.d_len);
       key_len += rsa_len.d_len;
       rsa_len.p_len = RSA_MAX_MOD_BYTES >> 1;
       TEE_GetObjectBufferAttribute(key, TEE_ATTR_RSA_PRIME1, rsa_keys + key_len, &rsa_len.p_len);
       key_len += rsa_len.p_len;
       rsa_len.q_len = RSA_MAX_MOD_BYTES >> 1;
       TEE_GetObjectBufferAttribute(key, TEE_ATTR_RSA_PRIME2, rsa_keys + key_len, &rsa_len.q_len);
       key_len += rsa_len.q_len;
       rsa_len.p1_len = RSA_MAX_MOD_BYTES >> 1;
       TEE_GetObjectBufferAttribute(key, TEE_ATTR_RSA_EXPONENT1, rsa_keys + key_len, &rsa_len.p1_len);
       key_len += rsa_len.p1_len;
       rsa_len.q1_len = RSA_MAX_MOD_BYTES >> 1;
       TEE_GetObjectBufferAttribute(key, TEE_ATTR_RSA_EXPONENT2, rsa_keys + key_len, &rsa_len.q1_len);
       key_len += rsa_len.q1_len;
       rsa_len.p1q1_len = RSA_MAX_MOD_BYTES >> 1;
       TEE_GetObjectBufferAttribute(key, TEE_ATTR_RSA_COEFFICIENT, rsa_keys + key_len, &rsa_len.p1q1_len);
       key_len += rsa_len.p1q1_len;
       header.priv_len = sizeof(rsa_len) + key_len;

       TEE_SeekObjectData(ses_handle->obj_handle, 0, TEE_DATA_SEEK_SET);
       /* Store the key header */
       if((result = TEE_WriteObjectData(ses_handle->obj_handle, &header, sizeof(header))) != TEE_SUCCESS)
          break;
       /* Store key lengths */
       if((result = TEE_WriteObjectData(ses_handle->obj_handle, &rsa_len, sizeof(rsa_len))) != TEE_SUCCESS)
          break;
       /* Store key elements */
       if((result = TEE_WriteObjectData(ses_handle->obj_handle, rsa_keys, key_len)) != TEE_SUCCESS)
          break;
       TEE_Free(rsa_keys);
       TEE_FreeTransientObject(key);
    }
    break;

  /****************************************************************
   * Handle request to assign a public key/certificate to an object
   ****************************************************************/
  case TA_SK_SET_CERT:
  case TA_SK_SET_PUBL_KEY:
    /* Read the object header */
    if((result = sk_get_object_header(ses_handle->obj_handle, &header)) == TEE_SUCCESS){
       DMSG("TA: Total object size %d bytes\n", header.priv_len + header.publ_len);
       if (header.flag & OBJECT_FLAG_LOCK) {
          EMSG("ERROR: Object cannot be updated\n");
          result = TEE_ERROR_ACCESS_DENIED;
          break;
       }

       /* Update object header */
       header.flag &= ~OBJECT_FLAG_PUB_MASK;
       header.flag |= (cmd_id == TA_SK_SET_PUBL_KEY) ? OBJECT_FLAG_PUB_KEY : OBJECT_FLAG_PUB_CERT;
       header.publ_len = params[1].memref.size;
       TEE_SeekObjectData(ses_handle->obj_handle, 0, TEE_DATA_SEEK_SET);
       TEE_WriteObjectData(ses_handle->obj_handle, &header, sizeof(header));
       /* Locate the public content */
       if (header.priv_len)
	  if((result = TEE_SeekObjectData(ses_handle->obj_handle, header.priv_len, TEE_DATA_SEEK_CUR)) != TEE_SUCCESS)
	     break;

       ta_param = TEE_Malloc(params[1].memref.size, TEE_MALLOC_FILL_ZERO);
       memcpy(ta_param, params[1].memref.buffer, params[1].memref.size);
       /* Update public key/certificate */
       result = TEE_WriteObjectData(ses_handle->obj_handle, ta_param, params[1].memref.size);
       TEE_Free(ta_param);
       if (result != TEE_SUCCESS)
	  break;
    }
    break;

  /****************************************************************
   * Handle request to retrieve public key/certificate from an object
   ****************************************************************/
  case TA_SK_GET_CERT:
  case TA_SK_GET_PUBL_KEY:
    params[2].value.a = 0;
    /* Read the object header */
    if((result = sk_get_object_header(ses_handle->obj_handle, &header)) == TEE_SUCCESS){
       DMSG("TA: Total object size %d bytes\n", header.priv_len + header.publ_len);
       if (header.flag & OBJECT_FLAG_LOCK) {
          EMSG("ERROR: Object cannot be updated\n");
          result = TEE_ERROR_ACCESS_DENIED;
          break;
       }

       /* Check if there is public content */
       if (header.publ_len) {
          uint32_t req_size = 0;

          /* Locate public object */
          if (header.priv_len)
             if((result |= TEE_SeekObjectData(ses_handle->obj_handle, header.priv_len, TEE_DATA_SEEK_CUR)) != TEE_SUCCESS)
                break;

          /* Extract the public content */
          read_bytes = req_size = (header.publ_len < params[1].memref.size) ? header.publ_len : params[1].memref.size;
	  ta_param = TEE_Malloc(read_bytes, TEE_MALLOC_FILL_ZERO);
          result |= TEE_ReadObjectData(ses_handle->obj_handle, ta_param, read_bytes, &read_bytes);
	  memcpy(params[1].memref.buffer, ta_param, read_bytes);
	  TEE_Free(ta_param);

          if(req_size != read_bytes){
             EMSG("ERROR: Failed to read public %d bytes\n", req_size);
             break;
          }
          /* Let the client know the number of bytes to read */
          params[2].value.a = read_bytes;
       }
       /* Extract public content from RSA key pair */
       else if (sk_contains_asymmetric_key(&header)){
         rsa_key_t rsa_key;

         sk_get_rsa_key(ses_handle, &rsa_key);
         /* Send the length of the public parameters */
         TEE_MemMove(params[1].memref.buffer, &rsa_key.size.n_len, 2 * sizeof(uint32_t));
         /* Copy the modulus */
         TEE_MemMove(((char*)params[1].memref.buffer + 2 * sizeof(uint32_t)), rsa_key.n, rsa_key.size.n_len);
         /* Copy the public exponent */
         TEE_MemMove(((char*)params[1].memref.buffer + 2 * sizeof(uint32_t)) + rsa_key.size.n_len, rsa_key.e, rsa_key.size.e_len);
          /* Let the client know the number of bytes to read */
         params[2].value.a = rsa_key.size.n_len + rsa_key.size.e_len + 2 * sizeof(uint32_t);
       }
       else {
          result = TEE_ERROR_ITEM_NOT_FOUND;
       }
    }
    break;

  /****************************************************************
   * Handle request to obtain the lenth of the public content
   ****************************************************************/
  case TA_SK_GET_LENGTH:
    params[1].value.a = 0;
    /* Read the object header */
    if((result = sk_get_object_header(ses_handle->obj_handle, &header)) == TEE_SUCCESS)
       params[1].value.a = header.publ_len;
    break;

  /****************************************************************
   * Handle request to begin crypto operations
   ****************************************************************/
  case TA_SK_CRYPTO_INIT:
    if((result = sk_get_object_header(ses_handle->obj_handle, &header)) == TEE_SUCCESS){
       /* Check the kind of key we are using */
       if (sk_contains_symmetric_key(&header)){
	  uint8_t init_v[16];
	  uint32_t algo = map_algo[params[1].value.a];
	  uint32_t mode = map_mode[params[1].value.b];
	  uint32_t key_size = header.priv_len * 8;
	  int ae = 0;

          DMSG("TA: Using symmetric key\n");
          /* Remember the crypto session type */
	  if (algo == TEE_ALG_AES_GCM || algo == TEE_ALG_AES_CCM) {
	     ses_handle->session = (mode == TEE_MODE_ENCRYPT) ? SK_SESSION_SYMMETRIC_AUTHENC : SK_SESSION_SYMMETRIC_AUTHDEC;
	     ae = 1;
	  }
	  else {
	     ses_handle->session = SK_SESSION_SYMMETRIC_CIPHER;
	     ae = 0;
	  }

          /* Set the symmetric key */
          secret_key[0].content.ref.buffer = sk_get_private_key(ses_handle->obj_handle);
          secret_key[0].content.ref.length = header.priv_len;
          secret_key[0].attributeID = TEE_ATTR_SECRET_VALUE;

          result  = TEE_AllocateOperation(&ses_handle->op_handle, algo, mode, key_size);
          /* Create a symmetric key for AES */
          result += TEE_AllocateTransientObject(TEE_TYPE_AES, key_size, &ses_handle->key_handle);
          /* Hook up the secret key for this crypto operation */
          result += TEE_PopulateTransientObject(ses_handle->key_handle, &secret_key[0], 1);
          result += TEE_SetOperationKey(ses_handle->op_handle, ses_handle->key_handle);

          /* Point to the  Initialization Vector at the end of private key */
          TEE_SeekObjectData(ses_handle->obj_handle, sizeof(ObjectHeader) + header.priv_len, TEE_DATA_SEEK_SET);
          /* Read saved IV */
          read_bytes = 16;
          result |= TEE_ReadObjectData(ses_handle->obj_handle, init_v, read_bytes, &read_bytes);

          /* Initialize symmetric crypto */
          if (ae)
	     TEE_AEInit(ses_handle->op_handle, init_v, 8, 128, 0, params[2].value.a);
	  else
	     TEE_CipherInit(ses_handle->op_handle, init_v, 16);

          if (secret_key[0].content.ref.buffer)
             TEE_Free(secret_key[0].content.ref.buffer);
       }
      else if (sk_contains_asymmetric_key(&header)){
         int indx = 0;

         /* Remember the crypto session type */
         if (map_mode[params[1].value.b] == TEE_MODE_ENCRYPT)
            ses_handle->session = SK_SESSION_ASYMMETRIC_ENCRYPT;
         else if (map_mode[params[1].value.b] == TEE_MODE_DECRYPT)
            ses_handle->session = SK_SESSION_ASYMMETRIC_DECRYPT;
         else if (map_mode[params[1].value.b] == TEE_MODE_SIGN)
            ses_handle->session = SK_SESSION_ASYMMETRIC_SIGN;
         else if (map_mode[params[1].value.b] == TEE_MODE_VERIFY)
            ses_handle->session = SK_SESSION_ASYMMETRIC_VERIFY;
         else{
            EMSG("ERROR: Invalid Asymmetric Request\n");
            result = TEE_ERROR_NOT_IMPLEMENTED;
            break;
         }

         if (header.priv_len){
            rsa_key_t rsa_key;

            DMSG("TA: Using asymmetric key pair \n");
            sk_get_rsa_key(ses_handle, &rsa_key);;

            result |= TEE_AllocateOperation(&ses_handle->op_handle, map_algo[params[1].value.a], map_mode[params[1].value.b], rsa_key.size.n_len * 8);
            /* Set private key attributes */
            secret_key[indx].content.ref.buffer = rsa_key.n;
            secret_key[indx].content.ref.length = rsa_key.size.n_len;
            secret_key[indx].attributeID = TEE_ATTR_RSA_MODULUS;
            indx++;

            secret_key[indx].content.ref.buffer = rsa_key.e;
            secret_key[indx].content.ref.length = rsa_key.size.e_len;
            secret_key[indx].attributeID = TEE_ATTR_RSA_PUBLIC_EXPONENT;
            indx++;

            secret_key[indx].content.ref.buffer = rsa_key.d;
            secret_key[indx].content.ref.length = rsa_key.size.d_len;
            secret_key[indx].attributeID = TEE_ATTR_RSA_PRIVATE_EXPONENT;
            indx++;

            secret_key[indx].content.ref.buffer = rsa_key.p;
            secret_key[indx].content.ref.length = rsa_key.size.p_len;
            secret_key[indx].attributeID = TEE_ATTR_RSA_PRIME1;
            indx++;

            secret_key[indx].content.ref.buffer = rsa_key.q;
            secret_key[indx].content.ref.length = rsa_key.size.q_len;
            secret_key[indx].attributeID = TEE_ATTR_RSA_PRIME2;
            indx++;

            secret_key[indx].content.ref.buffer = rsa_key.p1;
            secret_key[indx].content.ref.length = rsa_key.size.p1_len;
            secret_key[indx].attributeID = TEE_ATTR_RSA_EXPONENT1;
            indx++;

            secret_key[indx].content.ref.buffer = rsa_key.q1;
            secret_key[indx].content.ref.length = rsa_key.size.q1_len;
            secret_key[indx].attributeID = TEE_ATTR_RSA_EXPONENT2;
            indx++;

            secret_key[indx].content.ref.buffer = rsa_key.p1q1;
            secret_key[indx].content.ref.length = rsa_key.size.p1q1_len;
            secret_key[indx].attributeID = TEE_ATTR_RSA_COEFFICIENT;
            indx++;
            /* Create the private key */
            result |= TEE_AllocateTransientObject(TEE_TYPE_RSA_KEYPAIR, rsa_key.size.n_len * 8, &ses_handle->key_handle);
         }
         else{
            uint8_t *mod, *exp;
            uint32_t m_size, e_size;
            uint32_t key_len = header.publ_len;


            /* Asymmetric public key only allows to do ENCRYPTION and VERIFICATION */
            if ((ses_handle->session == SK_SESSION_ASYMMETRIC_DECRYPT) ||
                (ses_handle->session == SK_SESSION_ASYMMETRIC_SIGN)){

               EMSG("ERROR: Operation not allowed with this key\n");
               result = TEE_ERROR_BAD_STATE;
               break;
            }
            DMSG("TA: Using asymmetric public key\n");
            /* Extract RSA pub key */
            obj_content = TEE_Malloc(key_len, TEE_MALLOC_FILL_ZERO);
            key_data = TEE_Malloc(key_len, TEE_MALLOC_FILL_ZERO);
            /* Extract RSA key from pem content */
            read_bytes = key_len;
            result |= TEE_ReadObjectData(ses_handle->obj_handle, obj_content, read_bytes, &read_bytes);
            result |= sk_extract_key_from_pem(obj_content, read_bytes, key_data, &key_len);
            result |= sk_parse_publ_key(key_data, key_len, &mod, &m_size, &exp, &e_size);
            result |= TEE_AllocateOperation(&ses_handle->op_handle, map_algo[params[1].value.a], map_mode[params[1].value.b], m_size * 8);
            /* Set public key attributes */
            secret_key[indx].content.ref.buffer = mod;
            secret_key[indx].content.ref.length = m_size;
            secret_key[indx].attributeID = TEE_ATTR_RSA_MODULUS;
            indx++;

            secret_key[indx].content.ref.buffer = exp;
            secret_key[indx].content.ref.length = e_size;
            secret_key[indx].attributeID = TEE_ATTR_RSA_PUBLIC_EXPONENT;
            indx++;
            /* Create the public key */
            result |= TEE_AllocateTransientObject(TEE_TYPE_RSA_PUBLIC_KEY, m_size * 8, &ses_handle->key_handle);
         }
         /* Attach the attributed to the created key */
         result |= TEE_PopulateTransientObject(ses_handle->key_handle, secret_key, indx);
         /* Hook up the key to the intended asymmetric crypto opeartion */
         result |= TEE_SetOperationKey(ses_handle->op_handle, ses_handle->key_handle);
         DMSG("TA: Set key status 0x%08X\n", (int)result);
      }
      else
          result = TEE_ERROR_BAD_PARAMETERS;
    }
    else
       result = TEE_ERROR_ITEM_NOT_FOUND;
    break;

  /****************************************************************
   * Handle request to continue updating crypto operations
   ****************************************************************/
  case TA_SK_CRYPTO_UPDATE:
    if (ses_handle->key_handle && ses_handle->op_handle){
       switch (ses_handle->session) {
       case SK_SESSION_SYMMETRIC_CIPHER:
	  result = TEE_CipherUpdate(ses_handle->op_handle,
				    params[1].memref.buffer, params[1].memref.size,
				    params[2].memref.buffer, &params[2].memref.size);
	  break;
       case SK_SESSION_SYMMETRIC_AUTHENC:
       case SK_SESSION_SYMMETRIC_AUTHDEC:
	  result = TEE_AEUpdate(ses_handle->op_handle,
				params[1].memref.buffer, params[1].memref.size,
				params[2].memref.buffer, &params[2].memref.size);
	  break;
       case SK_SESSION_ASYMMETRIC_ENCRYPT:
	  result = TEE_AsymmetricEncrypt(ses_handle->op_handle, NULL, 0,
					 params[1].memref.buffer, params[1].memref.size,
					 params[2].memref.buffer, &params[2].memref.size);
	  break;
       case SK_SESSION_ASYMMETRIC_DECRYPT:
	  result = TEE_AsymmetricDecrypt(ses_handle->op_handle, NULL, 0,
					 params[1].memref.buffer, params[1].memref.size,
					 params[2].memref.buffer, &params[2].memref.size);
	  break;
       default:
	  EMSG("ERROR: Operation not supported\n");
       }
    }
    break;

  /****************************************************************
   * Handle request to finalize crypto operations
   ****************************************************************/
  case TA_SK_CRYPTO_FINAL:
    if (ses_handle->key_handle && ses_handle->op_handle){
       char tag[16];
       uint32_t tag_len = sizeof(tag);

       result = TEE_SUCCESS;
       if(params[2].memref.buffer && ses_handle->session < SK_SESSION_ASYMMETRIC_ENCRYPT) {
	  if (ses_handle->session == SK_SESSION_SYMMETRIC_AUTHENC) {
	     result |= TEE_AEEncryptFinal(ses_handle->op_handle, params[1].memref.buffer, params[1].memref.size, params[2].memref.buffer, &params[2].memref.size, tag, &tag_len);
	     /* Append authenticated tag */
	     memcpy((char*)(params[2].memref.buffer) + params[2].memref.size, tag, tag_len);
	     params[2].memref.size += tag_len;
	  }
	  else if (ses_handle->session == SK_SESSION_SYMMETRIC_AUTHDEC) {
	     uint32_t data_len = params[1].memref.size - tag_len;
	     /* Extract authenticated tag */
	     result |= TEE_AEDecryptFinal(ses_handle->op_handle, params[1].memref.buffer, data_len,
					  params[2].memref.buffer, &params[2].memref.size,
					  params[1].memref.buffer + data_len, tag_len);
	  }
	  else
	     result |= TEE_CipherDoFinal(ses_handle->op_handle, params[1].memref.buffer, params[1].memref.size, params[2].memref.buffer, &params[2].memref.size);
       }

       if (ses_handle->key_handle) {
          TEE_FreeTransientObject(ses_handle->key_handle);
          ses_handle->key_handle = TEE_HANDLE_NULL;
       }

       if(ses_handle->op_handle){
          TEE_FreeOperation(ses_handle->op_handle);
          ses_handle->op_handle = TEE_HANDLE_NULL;
       }

       if(params[1].memref.buffer && params[2].memref.buffer && ses_handle->session >= SK_SESSION_ASYMMETRIC_ENCRYPT){
          EMSG("ERROR: Asymmetric FINAL must not contain any data\n");
          result = TEE_ERROR_EXCESS_DATA;
       }
    }
    break;

  /****************************************************************
   * Handle request to sign operations
   ****************************************************************/
  case TA_SK_CRYPTO_SIGN:
    if (ses_handle->key_handle && ses_handle->op_handle && ses_handle->session == SK_SESSION_ASYMMETRIC_SIGN ){
       result = TEE_AsymmetricSignDigest(ses_handle->op_handle, NULL, 0,
                                         params[1].memref.buffer, params[1].memref.size,
                                         params[2].memref.buffer, &params[2].memref.size);
       TEE_FreeTransientObject(ses_handle->key_handle);
       ses_handle->key_handle = TEE_HANDLE_NULL;

       TEE_FreeOperation(ses_handle->op_handle);
       ses_handle->op_handle = TEE_HANDLE_NULL;
    }
    else
       EMSG("ERROR: Sign operation not supported\n");

    break;

  /****************************************************************
   * Handle request to VERIFY operations
   ****************************************************************/
  case TA_SK_CRYPTO_VERIFY:
    if (ses_handle->key_handle && ses_handle->op_handle && ses_handle->session == SK_SESSION_ASYMMETRIC_VERIFY ){

       result = TEE_AsymmetricVerifyDigest(ses_handle->op_handle, NULL, 0,
                                           params[1].memref.buffer, params[1].memref.size,
                                           params[2].memref.buffer, params[2].memref.size);
       params[3].value.a = (result == TEE_SUCCESS) ? 1 : 0;

       TEE_FreeTransientObject(ses_handle->key_handle);
       ses_handle->key_handle = TEE_HANDLE_NULL;

       TEE_FreeOperation(ses_handle->op_handle);
       ses_handle->op_handle = TEE_HANDLE_NULL;
    }
    else
       EMSG("ERROR: Verify operation not supported\n");

    break;

  case TA_SK_OBJ_DEBUG:
    /* Read the object header */
    if((result = sk_get_object_header(ses_handle->obj_handle, &header)) == TEE_SUCCESS){
       MSG("Priv size %d, Publ size %d bytes\n", header.priv_len, header.publ_len);
       if (header.flag & OBJECT_FLAG_LOCK) {
          EMSG("ERROR: Object cannot be updated\n");
          result = TEE_ERROR_ACCESS_DENIED;
          break;
       }

       /* Display private contents */
       if (header.priv_len){
          uint8_t print_buffer[32];
          int line_size = 0;

          /* Allocate buffer */
          if(!(obj_content = TEE_Malloc(header.priv_len, TEE_MALLOC_FILL_ZERO)))
             return TEE_ERROR_OUT_OF_MEMORY;

          /* Read private content */
          read_bytes = header.priv_len;
          result = TEE_ReadObjectData(ses_handle->obj_handle, obj_content, read_bytes, &read_bytes);
          if(header.priv_len != read_bytes){
             EMSG("ERROR: Failed to read priv %d bytes\n", header.priv_len);
             break;
          }
          MSG("private contents %d bytes\n", header.priv_len);
          if ((int)read_bytes - 32 >= 0){
             TEE_MemMove(print_buffer, obj_content, 32);
             MSG("%s", print_buffer);
             MSG("... ...");
             line_size = read_bytes % 32;
             if (line_size){
               TEE_MemMove(print_buffer, &obj_content[read_bytes-line_size], line_size);
               MSG("%s", print_buffer);
             }
          }
          else{
             TEE_MemMove(print_buffer, obj_content, read_bytes);
             MSG("%s", print_buffer);
          }

          TEE_Free(obj_content);
          obj_content = NULL;
       }

       /* Display public contents */
       if (header.publ_len){
          uint8_t print_buffer[32];
          int line_size = 0;

          /* Allocate buffer */
          if(!(obj_content = TEE_Malloc(header.publ_len, TEE_MALLOC_FILL_ZERO))){
             EMSG("ERROR: Failed to allocate %d bytes memory\n", header.publ_len);
             return TEE_ERROR_OUT_OF_MEMORY;
          }

          /* Read public content */
          read_bytes = header.publ_len;
          result = TEE_ReadObjectData(ses_handle->obj_handle, obj_content, read_bytes, &read_bytes);
          if(header.publ_len != read_bytes){
             EMSG("ERROR: Failed to read pub %d bytes\n", header.publ_len);
             break;
          }

          MSG("public contents %d bytes:\n", read_bytes);
          if ((int)read_bytes - 32 >= 0){
             TEE_MemMove(print_buffer, obj_content, 32);
             MSG("%s", print_buffer);
             MSG("... ...");
             line_size = read_bytes % 32;
             if (line_size){
                TEE_MemMove(print_buffer, &obj_content[read_bytes-line_size], line_size);
                MSG("%s", print_buffer);
             }
          }
          else{
             TEE_MemMove(print_buffer, obj_content, read_bytes);
             MSG("%s", print_buffer);
          }

          TEE_Free(obj_content);
          obj_content = NULL;
       }
    }
    break;

  default:
    EMSG("Unexpected: Case not handled\n");
  }

  if (key_data)
    TEE_Free(key_data);

  if (obj_content)
    TEE_Free(obj_content);

  if (result != TEE_SUCCESS)
    EMSG("ERROR: Command failed with error code 0x%08X\n", result);

  return result;
}
