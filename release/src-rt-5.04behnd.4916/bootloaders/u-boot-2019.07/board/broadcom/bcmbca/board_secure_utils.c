/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Broadcom Ltd.
 */
#include <common.h>
#include <fdtdec.h>
#include <asm/types.h>
#include <asm/byteorder.h>
#include <linux/errno.h>
#include <asm/unaligned.h>
#include <malloc.h>
#include "tpl_params.h"
#include "spl_env.h"
#include "bcm_secure.h"
#include "u-boot/rsa.h"
#include "u-boot/rsa-mod-exp.h"
#include <u-boot/sha256.h>
#include <uboot_aes.h>
#include <watchdog.h>
#include "mini-gmp/mini-gmp.h"
#include "mini-gmp/mini-mpq.h"


#if defined (CONFIG_TPL_BUILD)
extern tpl_params * tplparams;

static inline char* __get_ev(const char* nm)
{
        char* k = NULL;
        if (!tplparams) {
                return NULL;
        }
        k = find_spl_env_val(tplparams->environment, nm);
        if (!k) {
                return NULL;
        }
        return k;
}

#else

static inline char* __get_ev(const char* nm)
{
        return env_get(nm);
}

#endif
int bcm_util_hex2u32(const char* s, u8*  d)
{
	int len;
	u32* u = (u32*)d ;
        char buf[sizeof(u32)*2 + sizeof(u8)] = {0};
	char* pmax, *p = (char*)s;
	if (!p || !d) {
		return -1;
	}

	len = strlen(p);
	len -= (len%sizeof(u32));
	pmax = (char*)s + len; 
        while(p < pmax )        {
                memcpy(buf, p, sizeof(u32)*2);
                *u++ = ntohl(simple_strtoul((const char*)buf, NULL, 16));
                //printf("\n %x %s \n",new_key[j-1],buf);
                p += sizeof(u32)*2;
        }
	return len/2; 
}

u8* bcm_util_env_var2bin(const char* id, u32 len )
{
        char* p = NULL, *pmax;
        u8 *data = NULL;
        char buf[sizeof(u32)*2+sizeof(u8)] = {0};
        int j = 0 ;
        p = __get_ev(id);
        if (!p) {
                debug("No key\n");
                return NULL;
        }
        pmax = p + strlen((const char*)p);
        if (pmax-p != len*2) {
                printf("ERROR: invalid length for %s;must be %d got only %d\n",
                id, len*2, (u32)(pmax - p));
                return NULL;
        }
        data = malloc(len);
        if (!data) {
                return NULL;
        }
        while(p < pmax )        {
                memcpy(buf, p, sizeof(u32)*2);
                ((u32*)data)[j++] = ntohl(simple_strtoul((const char*)buf, NULL, 16));
                //printf("\n %x %s \n",new_key[j-1],buf);
                p += sizeof(u32)*2;
        }
        return data;
}

void bcm_sec_aes_cbc(u8 *key, u32 key_len, u8 *iv, u8* txt, 
			u32 length, u32 flag)
{
	/* key_len in bytes iv length is always AES_BLOCK_LENGTH */ 
	u32 num_aes_blocks;
	u8 key_schedule[AES_EXPAND_KEY_MAX_LEN];
	aes_expand_key(key, key_len, key_schedule);
	num_aes_blocks = (length + AES_BLOCK_LENGTH - 1) / AES_BLOCK_LENGTH;
	flag? aes_cbc_encrypt_blocks(key_len, key_schedule, iv, txt, txt, num_aes_blocks) :
			aes_cbc_decrypt_blocks(key_len, key_schedule, iv, txt, txt, num_aes_blocks);
}

void bcm_sec_aes_cbc128(u8 *key, u8 *iv, u8* txt, u32 length, u32 flag)
{
	bcm_sec_aes_cbc(key, AES128_KEY_LENGTH, iv, txt, length, flag);
}

static inline void checksum_sha256(const u8 *obj, 
				u32 len, u8* hash)
{
	sha256_context ctx;
	sha256_starts(&ctx);
	sha256_update(&ctx, obj, len);
	sha256_finish(&ctx, hash);
}

static int rsa_get_params(const u8* modulus,
			u32 modulus_size,	
			unsigned long *n0inv, 
			u8 *rr)
{
	size_t wcnt; 
	int rc = 0; 
	mpz_t	 _inv, _n0inv, 
		_r_sqrd, _key, _2_32, _2, _2_x;

	mpz_init (_n0inv);
	mpz_init (_inv);
	mpz_init (_r_sqrd);
	mpz_init (_key);
	mpz_init (_2);
	mpz_init (_2_32);
	mpz_init (_2_x);
	mpz_set_ui (_2, 2);
	mpz_import(_key , modulus_size, 1, sizeof(u8), 0, 0, modulus);
	mpz_pow_ui(_2_32, _2, 32);
	/* Calculate an inverse for _key and 2^32 */
	if ( mpz_invert(_inv, _key, _2_32) == 0) {
		rc = -1;
		goto err;
	}
	/*subtract to get negative of _inv*/
	mpz_sub(_n0inv, _2_32, _inv);
	
	mpz_export(n0inv, &wcnt, 1, sizeof(unsigned long), -1, 0, _n0inv);
	
	/*printf(" wcnt;--- n0inv:  %lu \n \t 0x%u\n", wcnt, *n0inv);*/
	/* RSA2048_BYTES converted to bits then multipled by 2 
  		to square [2^(RSA2048_BYTES*8)]^2
	*/
	mpz_pow_ui(_2_x, _2, modulus_size*8*2);
	/* modulo division to get an r-squared */
	mpz_mod(_r_sqrd, _2_x, _key);

	mpz_export(rr, &wcnt, 1, modulus_size, 1, 0, _r_sqrd);
	//dbg_hex(rr, RSA2048_BYTES, "R squared:");
err:
	mpz_clear (_n0inv);
	mpz_clear (_inv);
	mpz_clear (_r_sqrd);
	mpz_clear (_key);
	mpz_clear (_2);
	mpz_clear (_2_32);
	mpz_clear (_2_x);
	return rc;

}


void bcm_sec_digest(const u8 *data, u32 len, u8* digest, char* algo)
{
	//TODO: Support other algo, currently Ignoring algo and defaulting to sha256
	checksum_sha256(data, len, digest);
}

int bcm_sec_rsa_verify(const u8 *obj, 
		u32 obj_len, const u8* sig, 
		u32 pub_len, const u8 *pub, 
		struct image_sign_info *im )
{
	int rc = -1;
	struct key_prop rsa_prop ;
	u8 hash[SHA256_SUM_LEN] = {0};
	u8 rr[BCM_SECBT_RSA_MOD_MAX_LEN*2] = {0};
        u8	sig_dec[BCM_SECBT_RSA_MOD_MAX_LEN] = {0};
	unsigned long n0inv = 0;
	checksum_sha256(obj, obj_len, hash);
	rc = rsa_get_params(pub, pub_len, &n0inv, rr); 
	if (rc) {
		printf("ERROR: rsa arguments\n");	
		goto err;
	}

	rsa_prop.modulus = pub;
	rsa_prop.n0inv = n0inv;
	rsa_prop.rr = rr;
	rsa_prop.num_bits = pub_len*8;
	rsa_prop.exp_len = 4;
	rsa_prop.public_exponent = NULL;
	rc = rsa_mod_exp_sw(sig, pub_len, &rsa_prop, sig_dec);
	if (rc) {
		printf("ERROR: rsa decryption\n");	
		goto err; 
	}
	//dbg_hex(sig_dec, RSA2048_BYTES, " Sig Decrypted :");
	//dbg_hex(hash, SHA256_SUM_LEN, " hash :");
#if defined(CONFIG_FIT_ENABLE_RSASSA_PSS_SUPPORT)
	rc = emsa_pss_verify(im, sig_dec, pub_len, hash, SHA256_SUM_LEN, -1);
#endif
err:
	memset(sig_dec, 0, pub_len);
	memset(hash, 0, SHA256_SUM_LEN);
	memset(rr, 0, pub_len*2);
	memset(&rsa_prop, 0, sizeof(struct key_prop));
	return rc;
}

ulong bcm_sec_get_reqd_load_size( void * fit )
{
	return( fdt_totalsize(fit) + FIT_PAD_MAX_SIZE );
}
