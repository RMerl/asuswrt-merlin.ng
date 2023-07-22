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

#define ASN1_INTEGER                 0x02
#define ASN1_BIT_STRING              0x03
#define ASN1_SEQUENCE                0x10
#define ASN1_CONSTRUCTED             0x20
#define ASN1_ONEBYTE_SIZE            0x81
#define ASN1_MULTIBYTE_SIZE          0x82
#define ASN1_START_SEQ_SIZE          0x02

#define ASN1_ERR_OUT_OF_DATA         -0x0060
#define ASN1_ERR_UNEXPECTED_TAG      -0x0062
#define ASN1_ERR_INVALID_LENGTH      -0x0064

#define MAX_NUM_CHARS                64
#define MAX_NUM_CHARS_INDEX          (MAX_NUM_CHARS - 1)
#define MAX_DEC_VAL                  127

extern SessionHandle session[MAX_SESSION];

static int b64_dec(uint8_t *dst, uint32_t *dlen, const uint8_t *src, uint32_t slen);
static int asn1_get_tag( uint8_t **p, const uint8_t *end, size_t *len, int tag );
static int asn1_get_int( uint8_t **p, const uint8_t *end, int *val );
static int asn1_get_len( uint8_t **p, const uint8_t *end, size_t *len );
static int get_key_data(uint8_t *dst, uint8_t *src, uint8_t **end, unsigned int *elem_len);
static uint8_t* sk_strstr(uint8_t *str, const char *substr);
static uint32_t asn1_parse_tag(uint8_t **ptr);

static const uint8_t base64_dec_map[128] =
{
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127,  62, 127, 127, 127,  63,  52,  53,
     54,  55,  56,  57,  58,  59,  60,  61, 127, 127,
    127,  64, 127, 127, 127,   0,   1,   2,   3,   4,
      5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
     15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
     25, 127, 127, 127, 127, 127, 127,  26,  27,  28,
     29,  30,  31,  32,  33,  34,  35,  36,  37,  38,
     39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
     49,  50,  51, 127, 127, 127, 127, 127
};

uint8_t* sk_strstr(uint8_t *str, const char *substr)
{
   while (*str){
      uint8_t *begin = str;
      const char *pattern = substr;

      while (*str && *pattern && *str == *pattern){
         str++;
         pattern++;
      }

      if (!*pattern)
        return begin;

      str = begin + 1;
   }
   return NULL;
}

TEE_Result sk_open_object(TEE_Param params[4], TEE_ObjectHandle *handle)
{
   void * ta_param;
   TEE_Result rc = TEE_SUCCESS;

   ta_param = TEE_Malloc(params[0].memref.size, TEE_MALLOC_FILL_ZERO);
   memcpy(ta_param, (void*)params[0].memref.buffer, params[0].memref.size);

   if ( TEE_OpenPersistentObject( TEE_STORAGE_PRIVATE,
                                  ta_param,
                                  params[0].memref.size,
                                  CMN_TEE_FLAG, handle) != TEE_SUCCESS){
     rc = TEE_ERROR_ITEM_NOT_FOUND;
   }

   TEE_Free(ta_param);
   return rc;
}


TEE_Result sk_create_object(TEE_Param params[4], TEE_ObjectHandle *handle)
{
   ObjectHeader header;
   void * ta_param;
   TEE_Result rc = TEE_SUCCESS;

   /* Make sure object does not exist */
   if (sk_open_object(params, handle) == TEE_SUCCESS)
      return TEE_ERROR_BAD_STATE;

   ta_param = TEE_Malloc(params[0].memref.size, TEE_MALLOC_FILL_ZERO);
   memcpy(ta_param, (void*)params[0].memref.buffer, params[0].memref.size);

   /* Now create the new object header */
   header.flag = 0;
   header.priv_len = 0;
   header.publ_len = 0;

   if (TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE,
                                  ta_param,
                                  params[0].memref.size,
                                  CMN_TEE_FLAG,
                                  (TEE_ObjectHandle)(uintptr_t)0,
                                  &header, sizeof(header), handle) != TEE_SUCCESS){
     rc = TEE_ERROR_STORAGE_NOT_AVAILABLE;
   }

   TEE_Free(ta_param);
   return rc;
}


TEE_Result sk_get_object_header(TEE_ObjectHandle handle, ObjectHeader *header)
{
   uint32_t read_bytes = sizeof(ObjectHeader);

   /* Set the read pointer */
   TEE_SeekObjectData(handle, 0, TEE_DATA_SEEK_SET);
   /* Read the object header */
   return TEE_ReadObjectData(handle, header, sizeof(ObjectHeader), &read_bytes);
}


TEE_Result sk_register_session(TEE_ObjectHandle object, uint32_t *ses_id)
{
   int i = 0;
   int ses_slot = -1;

   *ses_id = 0xFFFFFFFF;

   for (i = 0; i < MAX_SESSION; i++){
      if ((session[i].obj_handle == 0) && (ses_slot == -1))
         ses_slot = i;

      if (session[i].obj_handle == object)
         return TEE_ERROR_BAD_STATE;
   }

   /* Is there any empty slot */
   if (ses_slot == -1)
      return TEE_ERROR_OUT_OF_MEMORY;
   else {
      *ses_id = ses_slot;
      session[ses_slot].obj_handle = object;
      return TEE_SUCCESS;
   }
}


TEE_Result sk_find_session(int ses_id, SessionHandle **ses)
{
   *ses = NULL;

   if (ses_id >= MAX_SESSION && ses_id < 0)
      return TEE_ERROR_BAD_PARAMETERS;

   *ses = &session[ses_id];

   return TEE_SUCCESS;
}


uint8_t* sk_get_private_key(TEE_ObjectHandle handle)
{
   ObjectHeader header;
   uint8_t *priv_key;
   uint32_t read_bytes = 0;

   if (sk_get_object_header(handle, &header) != TEE_SUCCESS)
      return NULL;

   if (header.priv_len){
      priv_key = TEE_Malloc(header.priv_len, TEE_MALLOC_FILL_ZERO);
      read_bytes = header.priv_len;
      TEE_ReadObjectData(handle, priv_key, read_bytes, &read_bytes);
      return priv_key;
   }
   return NULL;
}

int sk_contains_symmetric_key(ObjectHeader *header)
{
  return (header->priv_len && header->priv_len <= 64 && !(header->flag & (OBJECT_FLAG_PUB_KEY | OBJECT_FLAG_PUB_CERT | OBJECT_FLAG_ASYM_KEY))) ? 1 : 0;
}

int sk_contains_asymmetric_key(ObjectHeader *header)
{
  return (header->publ_len || header->priv_len > 64 || (header->flag & OBJECT_FLAG_PRIV_MASK) == OBJECT_FLAG_ASYM_KEY) ? 1 : 0;
}


TEE_Result sk_parse_priv_key(uint8_t *p, int size, rsa_key_t *key)
{
   int offset = 0;
   uint8_t *end = p+size;
   uint8_t *buf_end = p+size;
   uint8_t *start = p;
   size_t len;

   if ((asn1_get_tag(&start, end, &len, ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
      return -1;

   if (asn1_get_int(&start, end, &offset) != 0)
      return -1;

   end = (uint8_t*)(end + len + ASN1_START_SEQ_SIZE);

   start += offset;
   p = start;

   /* get modulus - n*/
   if (get_key_data(key->n, p, &end, &key->size.n_len) != 0)
      return -1;
   p = end;
   end = buf_end;

   /* get exponent -e  (public exponent) */
   get_key_data(key->e, p, &end, &key->size.e_len);
   p = end;
   end = buf_end;

   /* get private -d (private exponent)*/
   get_key_data(key->d, p, &end, &key->size.d_len);
   p = end;
   end = buf_end;

   /* get prime1 - p */
   get_key_data(key->p, p, &end, &key->size.p_len);
   p = end;
   end = buf_end;

   /* get prime2 = q */
   get_key_data(key->q, p, &end, &key->size.q_len);
   p = end;
   end = buf_end;

   /* get exponent1 = p1 */
   get_key_data(key->p1, p, &end, &key->size.p1_len);
   p = end;
   end = buf_end;

   /* get exponent2 = q1 */
   get_key_data(key->q1, p, &end, &key->size.q1_len);
   p = end;
   end = buf_end;

   /* get coefficient - p1q1 */
   get_key_data(key->p1q1, p, &end, &key->size.p1q1_len);

   return TEE_SUCCESS;
}


TEE_Result sk_parse_publ_key(uint8_t *data_ptr, size_t data_len, uint8_t **mod, uint32_t *m_size, uint8_t **exp, uint32_t *e_size)
{
   uint32_t tag;
   size_t len;
   uint8_t* ptr = (uint8_t*) data_ptr;
   uint8_t* end = ptr + data_len;
   uint32_t bitString_UnusedBits = 0;

   /* SubjectPublicKeyInfo */
   tag = asn1_parse_tag(&ptr);
   asn1_get_len(&ptr, end, &len);

   /* AlgorithmIdentifier */
   tag = asn1_parse_tag(&ptr);
   asn1_get_len(&ptr, end, &len);
   ptr += len;

   /* PublicKey */
   tag = asn1_parse_tag(&ptr);
   if(tag != ASN1_BIT_STRING)
      return TEE_ERROR_BAD_PARAMETERS;
   asn1_get_len(&ptr, end, &len);

   bitString_UnusedBits = *ptr;
   if(bitString_UnusedBits > 7)
      return TEE_ERROR_BAD_PARAMETERS;

   ptr++;
   ptr += bitString_UnusedBits;
   tag = asn1_parse_tag(&ptr);
   asn1_get_len(&ptr, end, &len);
   tag = asn1_parse_tag(&ptr);
   if(tag != ASN1_INTEGER)
      return TEE_ERROR_BAD_FORMAT;

   asn1_get_len(&ptr, end, &len);
   if (*ptr == 0x00){
      ptr++;
      len--;
   }
   *mod = ptr;
   *m_size = len;

   /* Move on the exponent */
   ptr += len;
   tag = asn1_parse_tag(&ptr);
   if(tag != ASN1_INTEGER)
      return TEE_ERROR_BAD_FORMAT;
   asn1_get_len(&ptr, end, &len);
   *exp = ptr;
   *e_size = len;

   return TEE_SUCCESS;
}

static int b64_dec(uint8_t *dst, uint32_t *dlen, const uint8_t *src, uint32_t slen)
{
   uint32_t i, n;
   uint32_t j, x;
   uint8_t *p;

   /* First pass: check for validity and get output length */
   for (i = 0, n = 0, j = 0; i < slen; i++) {
      /* Skip spaces before checking for EOL */
      x = 0;
      while (i < slen && src[i] == ' ') {
         ++i;
         ++x;
      }
      /* Spaces at end of buffer are OK */
      if (i == slen)
         break;

      if (( slen - i) >= 2 && src[i] == '\r' && src[i + 1] == '\n')
         continue;

      if (src[i] == '\n')
         continue;
      /* Space inside a line is an error */
      if (x != 0)
         return (-1);

      if (src[i] == '=' && ++j > 2)
         return (-1);

      if (src[i] > MAX_DEC_VAL || base64_dec_map[src[i]] == MAX_DEC_VAL)
         return (-1);

      if (base64_dec_map[src[i]] < MAX_NUM_CHARS && j != 0)
         return (-1);

      n++;
   }

   if (n == 0)
      return (0);

   n = ((n * 6) + 7) >> 3;
   n -= j;

   if (dst == NULL || *dlen < n) {
      *dlen = n;
      return (-2);
   }

   for (j = 3, n = 0, x = 0, p = dst; i > 0; i--, src++) {
      if (*src == '\r' || *src == '\n' || *src == ' ')
         continue;

      j -= (base64_dec_map[*src] == MAX_NUM_CHARS);
      x  = (x << 6) | (base64_dec_map[*src] & MAX_NUM_CHARS_INDEX);

      if (++n == 4) {
         n = 0;
         if (j > 0)
            *p++ = (uint8_t)(x >> 16);
         if (j > 1)
            *p++ = (uint8_t)(x >> 8);
         if (j > 2)
            *p++ = (uint8_t)(x);
      }
   }

   *dlen = p - dst;
   return (0);
}

static int get_key_data(uint8_t *dst, uint8_t *src, uint8_t **end, unsigned int *elem_len)
{
   size_t len;
   int ret = -1;
   uint8_t *p = src;

   if (*src == ASN1_INTEGER) {

      p++;
      if (*p == ASN1_MULTIBYTE_SIZE) {
         len = (p[1] << 8)|(p[2]);
         p = p + 3;
      }else if (*p == ASN1_ONEBYTE_SIZE) {
         len = p[1];
         p = p + 2;
      }else {
         len = *p;
         p++;
      }

      if ((src + len) > *end) {
         return ret;
      }
      if (*p == 0){
         p++;
         len--;
      }
      TEE_MemMove(dst, p, len);

      *elem_len = len;

      *end = p + len;
      ret = 0;
   } else {
      EMSG("invalid asn1 index:%x\n",*src);
   }
   return ret;
}

TEE_Result sk_extract_key_from_pem(uint8_t *pem_data, int pem_len, uint8_t *key_data, uint32_t *key_len)
{
   int ret = TEE_SUCCESS;
   const uint8_t *hdr, *ftr, *end;
   const uint8_t *start;
   uint32_t len = 0;
   int pem_length;

   /* parse the data for certificates */
   hdr = (uint8_t*)sk_strstr(pem_data, "-----BEGIN");

   end = pem_data + pem_len;

   if (hdr == NULL) {
     ret = TEE_ERROR_BAD_FORMAT;
      goto end_extract;
   }

   ftr = (uint8_t*)sk_strstr(pem_data, "-----END");

   if (ftr == NULL) {
      ret = TEE_ERROR_BAD_FORMAT;
      goto end_extract;
   }

   /* increment the size of '-----BEGIN' */
   hdr += strlen((const char *)"-----BEGIN");

   /* go to the end of hdr */
   while ((hdr < end) && (*hdr != '-'))
      hdr++;

   while ((hdr < end) && (*hdr == '-'))
      hdr++;

   if (*hdr == '\r') hdr++;
   if (*hdr == '\n') hdr++;

   start = hdr;
   if ((ftr <= hdr) || (ftr > end))
      return TEE_ERROR_BAD_FORMAT;

   pem_length = (int)(ftr-start);

   /* first call will return the len */
   ret = b64_dec(key_data, &len, start, pem_length);
   if ((ret != -2)||(len == 0)) {
      ret = TEE_ERROR_BAD_FORMAT;
      goto end_extract;
   }

   /* actual decode happens here */
   ret = b64_dec(key_data, &len, start, pem_length);
   if (ret != 0) {
      ret = TEE_ERROR_BAD_FORMAT;
   }
   *key_len = len;

end_extract:
   return ret;
}

TEE_Result sk_get_rsa_key(SessionHandle *ses_handle, rsa_key_t *rsa_key)
{
   ObjectHeader header;
   TEE_Result result = TEE_ERROR_ITEM_NOT_FOUND;

   TEE_MemFill(rsa_key, 0, sizeof(rsa_key_t));
   sk_get_object_header(ses_handle->obj_handle, &header);

   if(sk_contains_asymmetric_key(&header)){
      uint32_t read_bytes;
      uint32_t key_len = header.priv_len ? header.priv_len : header.publ_len;
      uint8_t *obj_content = TEE_Malloc(key_len, TEE_MALLOC_FILL_ZERO);

      if ((header.flag & OBJECT_FLAG_PRIV_MASK) == OBJECT_FLAG_ASYM_KEY){
         uint32_t key_pos = 0;
         /* Extract RSA key from locally generated key pair */
         read_bytes = sizeof(rsa_key_len_t);
         result = TEE_ReadObjectData(ses_handle->obj_handle, &rsa_key->size, read_bytes, &read_bytes);
         read_bytes = key_len - sizeof(rsa_key_len_t);
         result |= TEE_ReadObjectData(ses_handle->obj_handle, obj_content, read_bytes, &read_bytes);
	 //print_attr("ALL READ:", obj_content, read_bytes);
         TEE_MemMove(rsa_key->n, obj_content + key_pos, rsa_key->size.n_len);
         key_pos += rsa_key->size.n_len;
         TEE_MemMove(rsa_key->e, obj_content + key_pos, rsa_key->size.e_len);
         key_pos += rsa_key->size.e_len;
         TEE_MemMove(rsa_key->d, obj_content + key_pos, rsa_key->size.d_len);
         key_pos += rsa_key->size.d_len;
         TEE_MemMove(rsa_key->p, obj_content + key_pos, rsa_key->size.p_len);
         key_pos += rsa_key->size.p_len;
         TEE_MemMove(rsa_key->q, obj_content + key_pos, rsa_key->size.q_len);
         key_pos += rsa_key->size.q_len;
         TEE_MemMove(rsa_key->p1, obj_content + key_pos, rsa_key->size.p1_len);
         key_pos += rsa_key->size.p1_len;
         TEE_MemMove(rsa_key->q1, obj_content + key_pos, rsa_key->size.q1_len);
         key_pos += rsa_key->size.q1_len;
         TEE_MemMove(rsa_key->p1q1, obj_content + key_pos, rsa_key->size.p1q1_len);
      }
      else{
         uint8_t *key_data = TEE_Malloc(key_len, TEE_MALLOC_FILL_ZERO);

         read_bytes = key_len;
         /* Extract RSA key from pem content */
         result = TEE_ReadObjectData(ses_handle->obj_handle, obj_content, key_len, &read_bytes);
	 result |= sk_extract_key_from_pem(obj_content, read_bytes, key_data, &key_len);
         /* Extract RSA private key */
         result |= sk_parse_priv_key(key_data, key_len, rsa_key);
         TEE_Free(key_data);
      }
      TEE_Free(obj_content);
   }
   return result;
}

TEE_Result sk_alert(void)
{
   EMSG("ALERT: Security Violation Detected\n");
   EMSG("ALERT: Reboot system to get back the service\n");
   return TEE_ERROR_SECURITY;
}

static int asn1_get_tag( uint8_t **p, const uint8_t *end, size_t *len, int tag )
{
   if( ( end - *p ) < 1 )
     return( ASN1_ERR_OUT_OF_DATA );

   if( **p != tag )
      return( ASN1_ERR_UNEXPECTED_TAG );

   (*p)++;

   return( asn1_get_len( p, end, len ) );
}

static uint32_t asn1_parse_tag(uint8_t **ptr)
{
   uint8_t byte;
   uint32_t tag = 0;

   byte = **ptr;
   *ptr += 1;
   if ((byte & 0x1f) != 0x1f) {
      /* one octet tag */
     return byte & 0x1f;
   }
   /* multi-octet tag */
   do {
      byte = **ptr;
      *ptr += 1;
      tag = (tag << 7) | (byte & 0x7f);
   } while (byte & 0x80);

   return tag;
}


static int asn1_get_len( uint8_t **p, const uint8_t *end, size_t *len )
{
   if( ( end - *p ) < 1 )
      return( ASN1_ERR_OUT_OF_DATA );

   if( ( **p & 0x80 ) == 0 )
      *len = *(*p)++;
   else
   {
      switch( **p & 0x7F )
        {
        case 1:
          if( ( end - *p ) < 2 )
             return( ASN1_ERR_OUT_OF_DATA );

          *len = (*p)[1];
          (*p) += 2;
          break;

        case 2:
          if( ( end - *p ) < 3 )
             return( ASN1_ERR_OUT_OF_DATA );

          *len = ( (*p)[1] << 8 ) | (*p)[2];
          (*p) += 3;
          break;

        case 3:
          if( ( end - *p ) < 4 )
             return( ASN1_ERR_OUT_OF_DATA );

          *len = ( (*p)[1] << 16 ) | ( (*p)[2] << 8 ) | (*p)[3];
          (*p) += 4;
          break;

        case 4:
          if( ( end - *p ) < 5 )
             return( ASN1_ERR_OUT_OF_DATA );

          *len = ( (*p)[1] << 24 ) | ( (*p)[2] << 16 ) | ( (*p)[3] << 8 ) | (*p)[4];
          (*p) += 5;
          break;

        default:
          return( ASN1_ERR_INVALID_LENGTH );
        }
   }

   if( *len > (size_t) ( end - *p ) )
     return( ASN1_ERR_OUT_OF_DATA );

   return( 0 );
}


static int asn1_get_int( uint8_t **p, const uint8_t *end, int *val )
{
   int ret;
   size_t len;

   if( ( ret = asn1_get_tag( p, end, &len, ASN1_INTEGER ) ) != 0 )
      return( ret );

   if( len > sizeof( int ) || ( **p & 0x80 ) != 0 )
      return( ASN1_ERR_INVALID_LENGTH );

   *val = 0;

   while( len-- > 0 ) {
      *val = ( *val << 8 ) | **p;
      (*p)++;
   }

   return( 0 );
}

