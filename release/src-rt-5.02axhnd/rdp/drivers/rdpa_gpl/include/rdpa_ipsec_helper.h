/*
* <:copyright-BRCM:2015:DUAL/GPL:standard
* 
*    Copyright (c) 2015 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :>
*/

/*
 * rdpa_ipsec_helper.h
 *
 *  Created on: Apr 16, 2015
 */

#include "rdd_data_structures_auto.h"
#include "access_macros.h"


/* IPsec RDB definitions */
#define IPSEC_RDB_AES_KEY_SIZE_128_DEFAULT       0x1
#define IPSEC_RDB_AES_KEY_SIZE_192               0x2
#define IPSEC_RDB_AES_KEY_SIZE_256               0x4

#define IPSEC_RDB_DES_DEC_VEC_DEFAULT            0x4

#define IPSEC_RDB_ITERS_DES                      0x0
#define IPSEC_RDB_ITERS_3DES_DEFAULT             0x2

#define IPSEC_RDB_RNG_SAMPLE_NUM_DEFAULT         0x2

#define IPSEC_RDB_MECH_CBC_DEFAULT               0x0
#define IPSEC_RDB_MECH_CTR                       0x1
#define IPSEC_RDB_MECH_PLAIN                     0x2
#define IPSEC_RDB_CRYPT_CFG_MECH_SHIFT           3

#define IPSEC_RDB_ENCRYPT_DEFAULT                0x0
#define IPSEC_RDB_DECRYPT                        0x1

#define IPSEC_RDB_ALG_AES_DEFAULT                0x0
#define IPSEC_RDB_ALG_DES                        0x1
#define IPSEC_RDB_ALG_BYPASS                     0x2

#define IPSEC_RDB_ALG_SHA1_DEFAULT               0x0
#define IPSEC_RDB_ALG_MD5                        0x1
#define IPSEC_RDB_ALG_SHA2                       0x2
#define IPSEC_RDB_ALG_CHKSM                      0x3

#define IPSEC_RDB_ESP_IV_SIZE                    16

#define IPSEC_RDB_AES128_KEY_SIZE                128
#define IPSEC_RDB_AES192_KEY_SIZE                192
#define IPSEC_RDB_AES256_KEY_SIZE                256
#define IPSEC_RDB_DES_KEY_SIZE                   8


/** IPsec authentication algorithm */
typedef enum
{
   rdpa_auth_alg_sha1,
   rdpa_auth_alg_md5,
   rdpa_auth_alg_sha2,
   rdpa_auth_alg_checksum
} rdpa_ipsec_auth_alg_e;

/** IPsec crypto algorithm */
typedef enum
{
   rdpa_crypt_alg_aes_128,
   rdpa_crypt_alg_aes_192,
   rdpa_crypt_alg_aes_256,
   rdpa_crypt_alg_des,
   rdpa_crypt_alg_3des,
   rdpa_crypt_alg_bypass
} rdpa_ipsec_crypt_alg_e;

/** IPsec crypto mechanism  */
typedef enum
{
   rdpa_crypt_mech_cbc,
   rdpa_crypt_mech_ctr,
   rdpa_crypt_mech_plain
} rdpa_ipsec_crypt_mech_e;


/* SA descriptor */
typedef union
{
   struct {
      uint32_t    spi;
      uint32_t    auth_config;
      uint32_t    crypt_config;
      uint32_t    crypt_config2;
   };

   RDD_IPSEC_SA_DESC_DTS sa_desc;

} SA_DESC_DTS;


/** Format IPsec auth_config, crypt_config and crypt_config2 registers setting. 
 *
 * \param[in] dir: Indicates whether the packet is upstream or downstream.
 * \param[in] auth_alg: Hash algorithm.
 * \param[in] crypt_mech: Crypto mechanism.
 * \param[in] crypt_alg: Crypto algorithm.
 * \param[in] next_hdr: Next header value to be placed at end of padding.
 * \param[out] auth_config: Runner IPsec Auth Config register setting.
 * \param[out] crypt_config: Runner IPsec Crypt Config register setting.
 * \param[out] crypt_config2: Runner IPsec Crypt Config2 register setting.
 * \return 0=OK or int error code\n
 */
inline int rdpa_ipsec_sa_desc_setting(rdpa_traffic_dir        dir, 
                                      rdpa_ipsec_auth_alg_e   auth_algorithm,
                                      rdpa_ipsec_crypt_mech_e crypt_mechanism,
                                      rdpa_ipsec_crypt_alg_e  crypt_algorithm,
                                      uint8_t  nxt_hdr,
                                      uint32_t *auth_config,
                                      uint32_t *crypt_config,
                                      uint32_t *crypt_config2)
{
   int  ret = BDMF_ERR_OK;
   SA_DESC_DTS   sa_desc;

   uint8_t auth_alg       = IPSEC_RDB_ALG_SHA1_DEFAULT;
   uint8_t crypt_alg      = IPSEC_RDB_ALG_AES_DEFAULT;
   uint8_t mech           = IPSEC_RDB_MECH_CBC_DEFAULT;
   uint8_t aes_key_size   = IPSEC_RDB_AES_KEY_SIZE_128_DEFAULT;
   uint8_t des_dec_vec    = IPSEC_RDB_DES_DEC_VEC_DEFAULT;
   uint8_t des_iters      = IPSEC_RDB_ITERS_3DES_DEFAULT;
   uint8_t rng_sample_num = 0;
   uint8_t rng_clk_en     = 1;   /* enable */
   
   switch (auth_algorithm)
   {
      case rdpa_auth_alg_sha1:
         auth_alg = IPSEC_RDB_ALG_SHA1_DEFAULT;
         break;
      case rdpa_auth_alg_md5:
         auth_alg = IPSEC_RDB_ALG_MD5;
         break;
      case rdpa_auth_alg_sha2:
         auth_alg = IPSEC_RDB_ALG_SHA2;
         break;
      default:
         ret = BDMF_ERR_PARM;
         break;
   }

   switch (crypt_algorithm)
   {
      case rdpa_crypt_alg_aes_128:
         crypt_alg    = IPSEC_RDB_ALG_AES_DEFAULT;
         aes_key_size = IPSEC_RDB_AES_KEY_SIZE_128_DEFAULT;
         break;
      case rdpa_crypt_alg_aes_192:
         crypt_alg    = IPSEC_RDB_ALG_AES_DEFAULT;
         aes_key_size = IPSEC_RDB_AES_KEY_SIZE_192;
         break;
      case rdpa_crypt_alg_aes_256:
         crypt_alg    = IPSEC_RDB_ALG_AES_DEFAULT;
         aes_key_size = IPSEC_RDB_AES_KEY_SIZE_256;
         break;
      case rdpa_crypt_alg_des:
         crypt_alg    = IPSEC_RDB_ALG_DES;
         des_iters    = IPSEC_RDB_ITERS_DES;
         des_dec_vec  = 2;
         break;
      case rdpa_crypt_alg_3des:
         crypt_alg    = IPSEC_RDB_ALG_DES;
         des_iters    = IPSEC_RDB_ITERS_3DES_DEFAULT;
         des_dec_vec  = 2;
         break;
      case rdpa_crypt_alg_bypass:
         crypt_alg    = IPSEC_RDB_ALG_BYPASS;
         break;
      default:
         ret = BDMF_ERR_PARM;
         break;
   }

   switch (crypt_mechanism)
   {
      case rdpa_crypt_mech_cbc:
         mech = IPSEC_RDB_MECH_CBC_DEFAULT;
         break;
      case rdpa_crypt_mech_ctr:
         mech = IPSEC_RDB_MECH_CTR;
         break;
      case rdpa_crypt_mech_plain:
         mech = IPSEC_RDB_MECH_PLAIN;
         break;
      default:
         ret = BDMF_ERR_PARM;
         break;
   }

   if (ret != BDMF_ERR_OK)
      return ret;   

   memset((void *)&sa_desc, 0, sizeof(RDD_IPSEC_SA_DESC_DTS));

   /* format auth config register */
   RDD_IPSEC_SA_DESC_ENG_CFG_WRITE(0, &sa_desc);
   RDD_IPSEC_SA_DESC_HMAC_DIS_WRITE(0, &sa_desc);
   RDD_IPSEC_SA_DESC_AUTH_KEY_FETCH_DIS_WRITE(0, &sa_desc);
   RDD_IPSEC_SA_DESC_AUTH_ALG_WRITE(auth_alg, &sa_desc);

   /* format crypt config register */   
   RDD_IPSEC_SA_DESC_NXT_HDR_WRITE(nxt_hdr, &sa_desc);
   RDD_IPSEC_SA_DESC_AES_KEY_SIZE_WRITE(aes_key_size, &sa_desc);
   RDD_IPSEC_SA_DESC_DES_DEC_VEC_WRITE(des_dec_vec, &sa_desc);
   RDD_IPSEC_SA_DESC_DES_ITERS_WRITE(des_iters, &sa_desc);
   RDD_IPSEC_SA_DESC_RNG_CLK_EN_WRITE(rng_clk_en, &sa_desc);
   RDD_IPSEC_SA_DESC_RNG_SEED_WRITE(0, &sa_desc);
   RDD_IPSEC_SA_DESC_RNG_SAMPLE_NUM_WRITE(rng_sample_num, &sa_desc);
   RDD_IPSEC_SA_DESC_CRYPT_KEY_FETCH_DIS_WRITE(0, &sa_desc);
   RDD_IPSEC_SA_DESC_WRITE_DIS_WRITE(0, &sa_desc);
   RDD_IPSEC_SA_DESC_MECH_WRITE(mech, &sa_desc);
   if (dir == rdpa_dir_ds)
      RDD_IPSEC_SA_DESC_DECRYPT_WRITE(IPSEC_RDB_DECRYPT, &sa_desc);
   else
      RDD_IPSEC_SA_DESC_DECRYPT_WRITE(IPSEC_RDB_ENCRYPT_DEFAULT, &sa_desc);
   RDD_IPSEC_SA_DESC_CRYPT_ALG_WRITE(crypt_alg, &sa_desc);

   /* format crypt config 2 register */   
   RDD_IPSEC_SA_DESC_CLUSTR_OVRRD_WRITE(0, &sa_desc);
   RDD_IPSEC_SA_DESC_READ_CLUSTR_SEL_WRITE(0, &sa_desc);
   RDD_IPSEC_SA_DESC_WRITE_CLUSTR_SEL_WRITE(0, &sa_desc);
   RDD_IPSEC_SA_DESC_ADD_WRITE(0, &sa_desc);

   *auth_config   = sa_desc.auth_config;
   *crypt_config  = sa_desc.crypt_config;
   *crypt_config2 = sa_desc.crypt_config2;
   
   return ret;                   
}
