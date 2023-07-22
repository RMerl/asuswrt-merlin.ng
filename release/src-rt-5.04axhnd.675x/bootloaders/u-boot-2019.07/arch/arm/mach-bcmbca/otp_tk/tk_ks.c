/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2020 Broadcom Ltd.
 */
#include <common.h>
#include <linux/types.h>
#include <linux/io.h>
#include "linux/printk.h"
#include <asm/arch/misc.h>
#include <asm/byteorder.h>
#include <asm/sections.h>
#include <linux/errno.h>
#include <asm/unaligned.h>
#include <malloc.h>
#include <u-boot/rsa.h>
#include "u-boot/rsa-mod-exp.h"
#include <u-boot/sha256.h>
#include <uboot_aes.h>
#include <fdtdec.h>
#include "bcm_secure.h"
#include "tk_ks.h"
#include "bcm_rng.h"
#include "asm/arch/rng.h"


enum ks_obj_state {
        KS_OBJ_INACTIVE = 0,
        KS_OBJ_INVALID = -1,
        KS_OBJ_OK = 1
};

static struct ksobj {
        ks_t *ks;
        u32 ks_sz;
        u32 sec_state;
        enum ks_obj_state state;
        /* data buffer allocated/set in gen_info call*/
        void* mem;
	u8 *ek;
	u8 *pub;
} ks_obj;

static u8 keystore[KS_MAX_SZ] __attribute__((section(".data"))) ;


static inline int authenticate(const u8* sig,
			u32 size, 
			const u8* pub_key)
{
	struct image_sign_info im;
	im.checksum = image_get_checksum_algo("sha256,");
	if (bcm_sec_rsa_verify(sig + KS_SIG_SIZE, size , sig, KS_SIG_SIZE, pub_key, &im )) {
		return -1;
	}
	return 0;
}

/*in-place decrypt */
static inline void  aes_cbc128_decrypt(u32 *txt,
                        u32 size, u8 *key, u8 *iv) 
{
	bcm_sec_aes_cbc128((u8*)key, (u8*)iv, (u8*)txt, size, 0);
}

/*
 *
 *	KS - short for key_store 
 *
 * */
static inline u32 get_crc32(u8 *v, u32 size, u32 crc)
{
     	if (crc == 0) {
		crc = 0xffffffff;
     	}
	/* for this implementation chosen no complement crc32 as exepcted by crc calc for 
 	* sko object
 	* */
	return  crc32_no_comp(crc, v, size);
}





static inline ks_err_t ks_type2size(ks_data_type_t data_type, u32 *size)
{
        u32 sz;
        switch(data_type) {
                case KS_DATA_TYPE_KEY_AES_CBC_128_IV:
                case KS_DATA_TYPE_KEY_AES_CBC_128_EK:
                case KS_DATA_TYPE_DEV_KEY_128:
                        sz = KS_AES_128_CBC_SZ;
                        break;  
                case KS_DATA_TYPE_KEY_AES_CBC_256_EK:
                case KS_DATA_TYPE_KEY_AES_CBC_256_IV:
                case KS_DATA_TYPE_DEV_KEY_256:
                        sz = KS_AES_256_CBC_SZ;
                        break;  
                case KS_DATA_TYPE_RSA_PUB:
                case KS_DATA_TYPE_HASH:
                        sz = KS_HASH_SZ;
                        break;  
                case KS_DATA_TYPE_MID:
                case KS_DATA_TYPE_OID:
                        sz = KS_MID_SZ;
                        break;  
                default:
                        return  KS_ERR_INVALID;
        }
        *size = sz;
        return  KS_ERR_SUCC;
}

static ks_err_t ks_verify(ks_t* ks,
			u32 curr_sec_state,
			const u8* pub_key
                        /*Sizeof of the object without signature*/
                       )
{
        ks_req_state_t sec_state_req = ks->hdr.req_info.state;
        switch(sec_state_req) {
                case KS_REQ_TRANSIT_FLD: 
                        if (curr_sec_state == SEC_STATE_GEN3_MFG) {
                                if (authenticate(ks->key.sig, 
					ks->hdr.info_size - (sizeof(ks->key.crc) + KS_SIG_SIZE), 
					pub_key)) {
					goto err_fatal;
				}
                                printf("KATH\n");
                        } else if (curr_sec_state == SEC_STATE_UNSEC) {
                                if (ks->hdr.info_size < sizeof(u32) || 
                                ks->key.crc != get_crc32(((u8*)&ks->key.crc)+sizeof(ks->key.crc), 
                                        ks->hdr.info_size - sizeof(ks->key.crc), 0)) {
                                                                        goto err;
                                }
                        }
                        break;  
                case KS_REQ_TRANSIT_MFG:
                        if (ks->hdr.info_size != 0 || curr_sec_state != SEC_STATE_UNSEC) {
                                goto err;
                        }
                        break;
                default:
                        goto err;
        }
        return  KS_ERR_SUCC;
err:
        printf("EVER\n");
        return  KS_ERR_INVALID;
err_fatal:
	ks_reset();
	/* destroy hang, watchdog reset*/	
	hang();	
}
/* copy keystore to an internal data array; 
 * on v7 if not copied it can be erased by
 * BSS clean loop before int_r is called */
void sec_tk_find_keystore()
{
        u8 *bdata = KS_OFFSET;
	if (KS_IS_FDT(bdata)) {
		bdata +=  KS_FDT_SIZE(bdata);
	}
	memcpy(keystore, bdata, KS_MAX_SZ);
}

/*
        Reads and verifies key store
*/
ks_err_t ks_init(bcm_sec_state_t sec_state, 
                u32 sec_arch,
		const u8* pub_key,
		const u8* aes_key)
{
        /*
          Read flash block at offset 724*1024  
          Verify if valid header is valid 
        */
        ks_hdr_t *hdr = NULL;
        u8 *mem = NULL, *bdata = NULL;
	printf("KINI\n");
        /*BRKPT;*/ 
        if (ks_obj.state  == KS_OBJ_OK) {
                return KS_ERR_SUCC; 
        }
        if (ks_obj.state == KS_OBJ_INVALID) {      
                return KS_ERR_INVALID; 
        }
        
	hdr = (ks_hdr_t*)keystore;
        if (memcmp(hdr->magic, KS_MAGIC, 
			KS_MAGIC_SIZE)) {
                printf("EMGC\n");
                goto err;
        }
        if (hdr->crc != get_crc32(keystore, 
                        sizeof(ks_hdr_t) - sizeof((*hdr).crc),
                        0)) {
                printf("EHCR\n");
                goto err;
        }

        if (hdr->info_size + sizeof(ks_hdr_t) > KS_MAX_SZ) {
                printf("ESIZ\n");
                goto err;
        }



        /* we've compiled in with predefined sec arch support
        - GEN3
        */
        if (sec_arch != hdr->sec_arch) {
                printf("EARC\n");
                goto err;
        }
        /* if booted in non-sec mode - verify data crc */
        if (ks_verify((ks_t*)keystore, sec_state, 
			bcm_sec_get_active_pub_key())) {
                goto err;
        }
        ks_obj.mem = keystore;
        ks_obj.ks = (ks_t*)keystore;
        ks_obj.sec_state = sec_state;
	ks_obj.pub = pub_key;
	ks_obj.ek = aes_key;
        ks_obj.state = KS_OBJ_OK;
        return KS_ERR_SUCC; 
err:
        ks_obj.state = KS_OBJ_INVALID;
        return KS_ERR_INVALID;
}

static ks_err_t ks_get_data(ks_key_info_t* key_info,
			u8* cbc128_ek,
			void* data)
{
        /*Copy data to the destination*/
        ks_err_t rc =  KS_ERR_INVALID;
        u32 data_sz; 
        if (ks_type2size(KS_DATA_GET_TYPE(key_info->type_state), &data_sz)) {
                printf("EKTP\n");
                goto err;
        }
        switch(KS_DATA_GET_STATE(key_info->type_state)) {
                case KS_DATA_STATE_FLD_ENCR:
                        /*Decrypting 
 			* key_info->size must be padded to u32 
 			* */
			if (!IS_ALIGNED(key_info->size, sizeof(u32))) {
                		printf("EKEY\n");
				goto err;
			}
			/*in place decryption encrypted - */
                        aes_cbc128_decrypt( (u32*)&key_info->data[0],/*src*/
                                        key_info->size, /*data len*/
                                        cbc128_ek, /* key */
                                        cbc128_ek + BCM_SECBT_AES_CBC128_EK_LEN); /*iv*/ 
                        /*key_sz is expected size of the key per type*/
                        memcpy(data, (u32*)&key_info->data[0], data_sz);
                        break;
                case KS_DATA_STATE_RAW:
                        memcpy(data, &key_info->data[0], data_sz);
                        break;
                case KS_DATA_STATE_GEN_RAND:
			//printk("Requested RAND\n");
			rng_get_rand(data, data_sz);		
                        break;
                default:
                        goto err;
        }
        rc = KS_ERR_SUCC; 
err:
        return rc;
}
/* returned pointer must be released in non CFE_ROM mode and 
        keystore_data_release needs to be called when no more keys are not needed
*/
ks_err_t ks_get_data_info(ks_data_type_t type, 
			ks_data_state_t state,
			void* data)
{
        /*Copy data to the destination*/
        ks_err_t  rc = KS_ERR_INVALID;
        ks_hdr_t *hdr;
        ks_key_info_t *key_info;
        u8 *key_info_max;
        if (ks_obj.state != KS_OBJ_OK) {
                return KS_ERR_INVALID; 
        }
        hdr = &ks_obj.ks->hdr;
        key_info = &((ks_t*)ks_obj.mem)->key.info;
        key_info_max = (u8*)key_info + hdr->info_size - (sizeof(u32) + KS_SIG_SIZE);
        while ((u8*)key_info < key_info_max) {
                if ( KS_DATA_GET_TYPE(key_info->type_state) == type &&
                        KS_DATA_GET_STATE(key_info->type_state) == state) {
                        break;
                }
                key_info = (ks_key_info_t*)((uintptr_t)key_info+sizeof(ks_key_info_t)+key_info->size);
        }
        if ((u8*)key_info == key_info_max) {
                goto err;
        }
        if (ks_get_data(key_info, 
               ks_obj.ek,data) != KS_ERR_SUCC) {
               goto err;
        }
        rc =  KS_ERR_SUCC; 
err:
        if (rc) {
            printf("ERNF\n");
        }
        return rc;
}

ks_err_t ks_get_req_info(ks_req_info_t* req_info)
{
        if (ks_obj.state != KS_OBJ_OK) {
                goto err;
        }
        memcpy(req_info, 
		&ks_obj.ks->hdr.req_info, 
		sizeof(ks_req_info_t));
        return KS_ERR_SUCC; 
err:
        return KS_ERR_INVALID; 
}

static  void ks_data_release(void)
{

}
ks_err_t ks_reset()
{
        if (ks_obj.state == KS_OBJ_OK) {
                ks_data_release();
        }
        memset(&ks_obj, 0, sizeof(ks_obj));
        return KS_ERR_SUCC; 
}
