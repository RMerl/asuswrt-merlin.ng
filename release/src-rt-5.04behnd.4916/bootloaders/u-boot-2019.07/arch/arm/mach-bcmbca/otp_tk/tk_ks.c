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
#include "bcm_otp.h"
#include <u-boot/rsa.h>
#include "u-boot/rsa-mod-exp.h"
#include <u-boot/sha256.h>
#include <uboot_aes.h>
#include <fdtdec.h>
#include "bcm_secure.h"
#include "tk_ks.h"
#include "asm/arch/rng.h"
#include "bcm_rng.h"

#define KS_MAX_TYPE_SZ (KS_AES_256_CBC_SZ*2)

#define SEC_MAX_PROFILE 7	
#define SEC_MIN_MFG_PROFILE	2
#define SEC_MIN_FLD_PROFILE	4
#define MAX_CONSEQ_RID 		4

#define KS_SET_TYPE_ATTR(_ord, _rid, _data_type_state, _data_type, _sec_state)			\
		(((_ord&0x3f)<<26)|((_rid&0x3ff)<<16)|((_sec_state&0xf)<<12)|((_data_type_state&0xf)<<8)|(_data_type&0xff))

enum ks_obj_state {
        KS_OBJ_INACTIVE = 0,
        KS_OBJ_INVALID = -1,
        KS_OBJ_OK = 1
};

typedef struct rid {
	ks_key_info_t* ki;
} rid_t;

static struct ksobj {
        ks_t *ks;
        u32 ks_sz;
        u32 sec_state;
        enum ks_obj_state state;
        /* data buffer allocated/set in gen_info call*/
        void* mem;
	u8 *ek;
	u32 ek_len;
	u8 *pub;
} ks_obj;

static u8 keystore[KS_MAX_SZ] __attribute__((section(".data"))) ;




static u32 sec_mode_profile[2][SEC_MAX_PROFILE] = {
	{ 	
		KS_SET_TYPE_ATTR(0x0, OTP_MAP_BRCM_BTRM_BOOT_ENABLE, KS_DATA_STATE_RAW, KS_DATA_TYPE_BROM_MODE, SEC_STATE_UNSEC),
		KS_SET_TYPE_ATTR(0x0, OTP_MAP_CUST_BTRM_BOOT_ENABLE, KS_DATA_STATE_RAW, KS_DATA_TYPE_CUST_BROM_MODE, SEC_STATE_UNSEC),
		KS_SET_TYPE_ATTR(0x0, SOTP_MAP_FLD_HMID, KS_DATA_STATE_RAW, KS_DATA_TYPE_HASH, SEC_STATE_GEN3_MFG),
		KS_SET_TYPE_ATTR(0x0, OTP_MAP_CUST_MFG_MRKTID, KS_DATA_STATE_RAW, KS_DATA_TYPE_MID, SEC_STATE_GEN3_MFG),
		KS_SET_TYPE_ATTR(0x0, SOTP_MAP_FLD_ROE, KS_DATA_STATE_FLD_ENCR, KS_DATA_TYPE_KEY_AES_CBC_128_EK, SEC_STATE_GEN3_MFG),
		KS_SET_TYPE_ATTR(0x0, SOTP_MAP_FLD_ROE, KS_DATA_STATE_FLD_ENCR, KS_DATA_TYPE_KEY_AES_CBC_IV, SEC_STATE_GEN3_MFG),
	},
	{
		KS_SET_TYPE_ATTR(0x0, OTP_MAP_BRCM_BTRM_BOOT_ENABLE, KS_DATA_STATE_RAW, KS_DATA_TYPE_BROM_MODE, SEC_STATE_UNSEC),
		KS_SET_TYPE_ATTR(0x0, OTP_MAP_CUST_BTRM_BOOT_ENABLE, KS_DATA_STATE_RAW, KS_DATA_TYPE_CUST_BROM_MODE, SEC_STATE_UNSEC),
		KS_SET_TYPE_ATTR(0x0, SOTP_MAP_FLD_HMID, KS_DATA_STATE_RAW, KS_DATA_TYPE_HASH, SEC_STATE_GEN3_MFG),
		KS_SET_TYPE_ATTR(0x0, OTP_MAP_CUST_MFG_MRKTID, KS_DATA_STATE_RAW, KS_DATA_TYPE_MID, SEC_STATE_GEN3_MFG),
		KS_SET_TYPE_ATTR(0x0, SOTP_MAP_FLD_ROE, KS_DATA_STATE_FLD_ENCR, KS_DATA_TYPE_KEY_AES_CBC_192_EK, SEC_STATE_GEN3_MFG),
		KS_SET_TYPE_ATTR(0x0, SOTP_MAP_FLD_ROE, KS_DATA_STATE_FLD_ENCR, KS_DATA_TYPE_KEY_AES_CBC_256_EK, SEC_STATE_GEN3_MFG),
		KS_SET_TYPE_ATTR(0x0, SOTP_MAP_FLD_ROE1, KS_DATA_STATE_FLD_ENCR, KS_DATA_TYPE_KEY_AES_CBC_IV, SEC_STATE_GEN3_MFG),
	}
};


static ks_key_info_compat_t key_info_compat[ ] = {
		{4, KS_SET_TYPE_ATTR(0x0, OTP_MAP_BRCM_BTRM_BOOT_ENABLE, KS_DATA_STATE_RAW, KS_DATA_TYPE_BROM_MODE, SEC_STATE_UNSEC), 0x1},
		{4, KS_SET_TYPE_ATTR(0x1, OTP_MAP_CUST_BTRM_BOOT_ENABLE, KS_DATA_STATE_RAW, KS_DATA_TYPE_CUST_BROM_MODE, SEC_STATE_UNSEC), 0x7},
		/*{4, KS_SET_TYPE_ATTR(0x2, OTP_MAP_BRCM_PRODUCTION_MODE, KS_DATA_STATE_RAW, KS_DATA_TYPE_4B, SEC_STATE_UNSEC), 0x1},*/
		{4, KS_SET_TYPE_ATTR(0x0, SOTP_MAP_FLD_HMID, KS_DATA_STATE_RAW, KS_DATA_TYPE_HASH, SEC_STATE_GEN3_MFG), 0},
		{4, KS_SET_TYPE_ATTR(0x2, OTP_MAP_CUST_MFG_MRKTID, KS_DATA_STATE_RAW, KS_DATA_TYPE_MID, SEC_STATE_GEN3_MFG), 0},
		{4, KS_SET_TYPE_ATTR(0x3, SOTP_MAP_FLD_ROE, KS_DATA_STATE_FLD_ENCR, KS_DATA_TYPE_KEY_AES_CBC_128_EK, SEC_STATE_GEN3_MFG), 0},
		{4, KS_SET_TYPE_ATTR(0x4, SOTP_MAP_FLD_ROE, KS_DATA_STATE_FLD_ENCR, KS_DATA_TYPE_KEY_AES_CBC_IV, SEC_STATE_GEN3_MFG), 0},
		{4, KS_SET_TYPE_ATTR(0x1, SOTP_MAP_KEY_DEV_SPECIFIC, KS_DATA_STATE_GEN_RAND, KS_DATA_TYPE_DEV_KEY_256, SEC_STATE_GEN3_MFG), 0},
};




static inline int authenticate(const u8* sig,
			u32 size, 
			const u8* pub_key,
			u32 pub_len)
/* pub length is equal to sig_size */
{
	struct image_sign_info im;
	im.checksum = image_get_checksum_algo("sha256,");
	if (bcm_sec_rsa_verify(sig + pub_len, size , sig, pub_len, pub_key,  &im )) {
		return -1;
	}
	return 0;
}

/*in-place decrypt */
static inline void  aes_cbc_decrypt(u32 *txt, 
                        u32 size, u8 *key, u8 *iv, u32 key_len) 
{
	bcm_sec_aes_cbc((u8*)key, key_len, (u8*)iv, (u8*)txt, size, 0);
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

static ks_err_t ks_type2size(ks_data_type_t data_type, u32 *size, u32* max_sz)
{
        u32 sz = 0;
	ks_err_t rc = KS_ERR_SUCC;
	*max_sz = KS_AES_256_CBC_SZ;
        switch(data_type) {
                case KS_DATA_TYPE_KEY_AES_CBC_IV:
                        sz = KS_AES_128_CBC_SZ;
                        break;  
                case KS_DATA_TYPE_KEY_AES_CBC_128_EK:
                case KS_DATA_TYPE_DEV_KEY_128:
                        sz = KS_AES_128_CBC_SZ;
                        break;  
                case KS_DATA_TYPE_KEY_AES_CBC_256_EK:
                case KS_DATA_TYPE_DEV_KEY_256:
                        sz = KS_AES_256_CBC_SZ;
                        break;  
                case KS_DATA_TYPE_KEY_AES_CBC_192_EK:
                        sz = KS_AES_192_CBC_SZ;
                        break;  
                case KS_DATA_TYPE_RSA_PUB:
                case KS_DATA_TYPE_HASH:
                        sz = KS_HASH_SZ;
                        break;  
        	case KS_DATA_TYPE_BROM_MODE:
        	case KS_DATA_TYPE_CUST_BROM_MODE:
                case KS_DATA_TYPE_MID:
		case KS_DATA_TYPE_OID:
                        sz = KS_MID_SZ;
                        break;  
        	case KS_DATA_TYPE_4B:
                        sz = 4; 
                        break;  
                default:
                        rc = KS_ERR_INVALID;
			break;
        }
        *size = sz;
        return  rc;
}

static ks_err_t ks_verify(ks_t* ks,
			u32 curr_sec_state,
			const u8* pub_key,
			u32 pub_len
                        /*sizeof of the object without signature*/
                       )
{
	ks_req_state_t sec_state_req = ks->hdr.req_info.state;
	u32	sig_size = pub_len;
        switch((sec_state_req)) {
                case KS_REQ_TRANSIT_FLD:
                        if (curr_sec_state == SEC_STATE_GEN3_MFG) {
                                if (authenticate(ks->key.sig, 
					ks->hdr.info_size - (sizeof(ks->key.crc) + sig_size),
					pub_key, pub_len)) {
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


static void ntohl_array(u32* a, u32 length) 
{
    int i;
    for (i = 0;i < length; i++)  {
         a[i] = ntohl(a[i]);
    }
}

static void ks_get_data(ks_key_info_t* ki, void* data, u32* data_sz)
{
        /*Copy data to the destination*/
        u8* cbc_ek = ks_obj.ek;
	u32 key_len = ks_obj.ek_len, max_sz = 0, act_sz = 0;
	ks_type2size(KS_DATA_GET_TYPE(ki->type_state), &act_sz, &max_sz);
        switch(KS_DATA_GET_STATE(ki->type_state)) {
                case KS_DATA_STATE_FLD_ENCR:
                        /*Decrypting 
 			* key_info->size must be padded to u32 
 			* */
			/*in place decryption encrypted - */
                        aes_cbc_decrypt( (u32*)&ki->data[0],/*src*/
                                        ki->size, /*data len*/
                                        cbc_ek, /* key */
                                        cbc_ek + key_len,
					key_len); /*iv*/ 
                        /*key_sz is expected size of the key per type*/
                        memcpy(data, (u32*)&ki->data[0], act_sz);
                        break;
                case KS_DATA_STATE_GEN_RAND:
			//printk("Requested RAND\n");
			rng_get_rand(data, act_sz);		
                        break;
                case KS_DATA_STATE_RAW:
                default:
                        memcpy(data, &ki->data[0], act_sz);
                        break;
        }

	switch (KS_DATA_GET_TYPE(ki->type_state)) {
        	case	KS_DATA_TYPE_KEY_AES_CBC_128_EK:
        	case	KS_DATA_TYPE_KEY_AES_CBC_IV:
        	case	KS_DATA_TYPE_KEY_AES_CBC_256_EK:
        	case	KS_DATA_TYPE_KEY_AES_CBC_192_EK:
        	case	KS_DATA_TYPE_HASH:
        	case	KS_DATA_TYPE_MID:
			ntohl_array((u32*)data, act_sz/sizeof(u32));
			break;
		default:
			break;
	}
	*data_sz = act_sz;
}


static int ks_data_ord_valid(u32 ord)
{
	return (ord >= 0 && ord < MAX_ORD);
}

static int ks_data_state_valid(ks_data_state_t st)
{
	return (st >= KS_DATA_STATE_RAW && st <  KS_DATA_STATE_MAX);
}

static int ks_data_type_valid(ks_data_type_t st)
{
	return (st >= KS_DATA_TYPE_KEY_AES_CBC_128_EK && st <  KS_DATA_TYPE_MAX);
}

bcm_sec_state_t ks_get_arch_info(void) 
{
        return ks_obj.ks->hdr.sec_arch;
}

static void key_info_update_compat(u32* type_state,
				bcm_sec_state_t curr_state)
{
	ks_key_info_compat_t *k, *kmax;
	u32 ver = (ks_get_arch_info() >> 24)&0xf;
	if (ver == 1 || curr_state != SEC_STATE_GEN3_MFG) {
		return;
	}
	/* only update keystore for the FLD request from MFG secure mode e.g 
 	* for backward compatiblity update with rid(otpmap row id), order and current secmode 
 	* the older version of the keystore for each key_info item  
  	*/
	/* ver == 0 && curr_state == SEC_STATE_GEN3_MFG*/
	k = &key_info_compat[0];
	kmax = k + sizeof(key_info_compat)/sizeof(ks_key_info_compat_t);
	while(k < kmax) {
		if (KS_DATA_GET_TRANSIT_STATE((*k).type_state) == curr_state && 
			((*k).type_state&0xfff) == ((*type_state)&0xfff))  {
			*type_state |= ((*k).type_state & ~0xfff);
			break;
		}
		k++;
	}
}

static void __get_key_info(ks_key_info_t** ki,
			ks_key_info_t** ki_max,
			bcm_sec_state_t curr_state)
{
        ks_hdr_t *hdr = &ks_obj.ks->hdr;
	u32 arch_ver, ver;
	arch_ver = (ks_get_arch_info() >> 28)&0xf;
	ver = (ks_get_arch_info() >> 24)&0xf;
	u32 sig_size = (arch_ver == KS_ARCH_SEC_VERv1? KS_SIG1_SIZE:
						(arch_ver == KS_ARCH_SEC_VERv2? 
							KS_SIG2_SIZE : KS_SIG_SIZE));
	/* force kstore parsing to version 1 if arch_ver is VERv1 or VERv2*/
	if (ver == 0 && curr_state == SEC_STATE_UNSEC) {
		/* if in unsecure mode and this back compatiblity to the older tkmfg lib then
 		*  use statically linked array of the key_info values	
 		*/
		*ki = (ks_key_info_t*)(&key_info_compat[0]);
		*ki_max = (ks_key_info_t*)(&key_info_compat[2]);
	} else {
	        /*ver == 1 , curr_state == SEC_STATE_UNSEC || ver == 0 , curr_state == SEC_STATE_MFG*/
        	*ki = (ks_key_info_t*)((uintptr_t)&((ks_t*)ks_obj.mem)->key.info + sig_size - KS_SIG_SIZE);
        	*ki_max = (ks_key_info_t*)((uintptr_t)(*ki) + hdr->info_size - sizeof(u32) - sig_size);
	}
}

static  ks_err_t rid_max_size(u32 cur_state, u32 id, ks_key_info_t* ki, ks_key_info_t* max, u32 max_type_sz)
{
	int sz = 0;
        while ((u8*)ki < (u8*)max) {
		if (cur_state == KS_DATA_GET_TRANSIT_STATE(ki->type_state) && 
			id == KS_DATA_GET_RID(ki->type_state)) {
			sz +=  ki->size;
			if (sz > max_type_sz) {
				return KS_ERR_INVALID;
			}
		}
		ki = (ks_key_info_t*)((uintptr_t)ki + sizeof(ks_key_info_t) + ki->size);
	}
	return KS_ERR_SUCC;
}

static ks_err_t  key_info_verify_request(bcm_sec_state_t curr_state) 
{
		
	u32 *pfl, *pfl_max, *ppfl; 
	ks_key_info_t* ki, *ki_max;
	u32 profl = 0, profl_min = 0, data_type_sz = 0, data_type_max_sz = 0;
	u64 ord_msk = 0;
	u32 arch_ver = (ks_get_arch_info() >> 28)&0xf;
	__get_key_info(&ki, &ki_max, curr_state);
	pfl = ((arch_ver == KS_ARCH_SEC_VERv2 || arch_ver == KS_ARCH_SEC_VERv1)? sec_mode_profile[1] : sec_mode_profile[0]);
	ppfl = pfl;
	pfl_max = pfl + SEC_MAX_PROFILE;
	/* mode_min depicts establishes the minimum number of the specific rows per Bootrom Secure mode 
 	* 		which are required to be fused to turn to the that mode e.g to MFG or to FLD 
  	*/ 
	if (curr_state == SEC_STATE_UNSEC) {
		 profl_min = SEC_MIN_MFG_PROFILE;
	} else if (curr_state ==  SEC_STATE_GEN3_MFG) {
		 profl_min = SEC_MIN_FLD_PROFILE;
	} else { 
                return KS_ERR_INVALID;
	}

      	while ((u8*)ki < (u8*)ki_max) {
		/* verifying validity of the key_info data from the keystore*/
		key_info_update_compat(&ki->type_state, curr_state);
		if (curr_state ==  KS_DATA_GET_TRANSIT_STATE(ki->type_state) ) {
		if (!ks_data_type_valid(KS_DATA_GET_TYPE(ki->type_state)) || 
			!ks_data_state_valid(KS_DATA_GET_STATE(ki->type_state)) || 
			!ks_data_ord_valid(KS_DATA_GET_ORD(ki->type_state)) ||
			ks_type2size(KS_DATA_GET_TYPE(ki->type_state), &data_type_sz, &data_type_max_sz) ||
			(KS_DATA_GET_STATE(ki->type_state) == KS_DATA_STATE_FLD_ENCR && !IS_ALIGNED(ki->size, sizeof(u32))) || 
			ki->size < data_type_sz || ki->size > data_type_max_sz  ||
			/* order values must be unique */
			((ord_msk>>KS_DATA_GET_ORD(ki->type_state))&0x1) ||
			rid_max_size(curr_state, KS_DATA_GET_RID(ki->type_state), 
					(ks_key_info_t*)((uintptr_t)ki + sizeof(ks_key_info_t) + ki->size), ki_max, data_type_max_sz) ) {
				return KS_ERR_INVALID;
		}
		ord_msk |= (0x1 << KS_DATA_GET_ORD(ki->type_state));
		/* count the number or rows per brom secure mode */
		
		while (ppfl < pfl_max) {
			if (curr_state == KS_DATA_GET_TRANSIT_STATE(*ppfl) &&
				KS_DATA_GET_TYPE(ki->type_state) == KS_DATA_GET_TYPE(*ppfl) &&
				KS_DATA_GET_STATE(ki->type_state) == KS_DATA_GET_STATE(*ppfl) && 
				!((*ppfl >> 26)&0x1)) {
				/* using ORD field to count number of item per single profile*/
				*ppfl |= (0x1<<26);
				profl++;
			}
			ppfl++;
		}
		ppfl = pfl;
		}
               	ki = (ks_key_info_t*)((uintptr_t)ki + sizeof(ks_key_info_t) + ki->size);
	}
	return (profl_min == profl)? KS_ERR_SUCC : KS_ERR_INVALID;
}

static inline void iterate_by_rid(u32 id, rid_t rid[MAX_CONSEQ_RID], ks_key_info_t* ki, ks_key_info_t* max)
{
	int i = 0, k;
        while ((u8*)ki < (u8*)max && i <  MAX_CONSEQ_RID) {
		if (id == KS_DATA_GET_RID(ki->type_state)) {
			rid[i++].ki = ki;
		}
		ki = (ks_key_info_t*)((uintptr_t)ki + sizeof(ks_key_info_t) + ki->size);
	}
	/* sort by the order */
	for (i--; i > 0; i--) {
		for(k = i - 1; k >= 0; k--) {
			if (KS_DATA_GET_ORD(rid[i].ki->type_state) < KS_DATA_GET_ORD(rid[k].ki->type_state)) {
				rid[k].ki = (ks_key_info_t*)((uintptr_t)rid[k].ki ^ (uintptr_t)(rid[i].ki));
				rid[i].ki = (ks_key_info_t*)((uintptr_t)(rid[i].ki) ^ (uintptr_t)rid[k].ki);
				rid[k].ki = (ks_key_info_t*)((uintptr_t)rid[k].ki ^ (uintptr_t)(rid[i].ki));
			}
		}
	}
}



/* returned pointer must be released in non CFE_ROM mode and 
        keystore_data_release needs to be called when no more keys are not needed
*/
/*static*/ void ks_get_data_info(bcm_sec_state_t curr_state, 
			u32 ord,
			void* data,
			u32* data_sz,
			rid_t rid[MAX_CONSEQ_RID])
{
        /*Copy data to the destination*/
        ks_key_info_t *ki, *pki, *max; 
	u8 *p = NULL;
	u32 sz = 0, act_sz = 0 ;
	int i;
	__get_key_info(&pki, &max, curr_state);
	ki = pki;
        while ((u8*)pki < (u8*)max) {
		/* for backward compatibility TRANSIT_STATE and ORD*/
		if (ord == KS_DATA_GET_ORD(pki->type_state) && 
			curr_state == KS_DATA_GET_TRANSIT_STATE(pki->type_state)) {
			iterate_by_rid(KS_DATA_GET_RID(pki->type_state), rid,  ki, max);
			break;
               	}
                pki = (ks_key_info_t*)((uintptr_t)pki + sizeof(ks_key_info_t) + pki->size);
	}
	for (i = 0, p = (u8*)data; i < MAX_CONSEQ_RID && rid[i].ki; i++) {
        	ks_get_data(rid[i].ki, p + sz, &act_sz);
		/* disable record from being searchable*/
		KS_DATA_SET_ORD(rid[i].ki->type_state, MAX_ORD);  
		sz += act_sz;
	}
	*data_sz = sz;
}


ks_err_t ks_run_request(bcm_sec_state_t sec_state, int (*fnc)(otp_map_feat_t, const u8*, u32))
{
	u32 ord;
	rid_t rid[MAX_CONSEQ_RID]; 
	u8 data[KS_MAX_TYPE_SZ];
	u32 data_sz; 
	for (ord = 0;  ord  < MAX_ORD; ord++)  {
		data_sz = 0;
		memset(rid, 0, sizeof(rid_t)*MAX_CONSEQ_RID);
		memset(data, 0, sizeof(u8)*KS_MAX_TYPE_SZ);
		ks_get_data_info(sec_state, ord, data, &data_sz, rid);
		if (data_sz > 0) {
#ifdef DRY_RUN
			printf("fuse info: ord :requested  %d  selected  %d; trsit st %u rid %x data: type %u st %u sz %u val %x\n",
				ord, 
				KS_DATA_GET_ORD(rid[0].ki->type_state), 
				KS_DATA_GET_TRANSIT_STATE(rid[0].ki->type_state),
				KS_DATA_GET_RID(rid[0].ki->type_state),
				KS_DATA_GET_TYPE(rid[0].ki->type_state),
				KS_DATA_GET_STATE(rid[0].ki->type_state), 
				data_sz, *(u32*)data);
#endif
			if (fnc(KS_DATA_GET_RID(rid[0].ki->type_state), data, data_sz)) {
				return  -1;
			}
		}
	} 
	return 0;
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

/* copy keystore to an internal data array; 
 * on v7 if not copied it can be erased by
 * BSS clean loop before int_r is called */
void sec_tk_find_keystore(void)
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
	printf("KINI\n");
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


	BRKPT;

        /* we've compiled in with predefined sec arch support
        - GEN3
        */
        /*printf("sec_arch %x\n", (hdr->sec_arch&0xfffffff));*/
        if (sec_arch != (hdr->sec_arch&0xffff)) {
                printf("EARC\n");
                goto err;
        }
        /* if booted in non-sec mode - verify data crc */
        if (ks_verify((ks_t*)keystore, sec_state, 
			bcm_sec_get_active_pub_key(),BCM_SECBT_CRED_MOD_LEN )) {
                goto err;
        }
        ks_obj.mem = (u8*)keystore;
        ks_obj.ks = (ks_t*)keystore;
        ks_obj.sec_state = sec_state;
	ks_obj.pub = (u8*)pub_key;
	ks_obj.ek = (u8*)aes_key;
	ks_obj.ek_len = BCM_SECBT_CRED_AES_LEN ;
        ks_obj.state = KS_OBJ_OK;
	if (key_info_verify_request(sec_state)){
		printf("EKST\n");
		goto err;
	} 
        return KS_ERR_SUCC; 
err:
        ks_obj.state = KS_OBJ_INVALID;
        return KS_ERR_INVALID;
}

