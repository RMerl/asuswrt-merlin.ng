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
#include "u-boot/rsa.h"
#include "u-boot/rsa-mod-exp.h"
#include <u-boot/sha256.h>
#include <uboot_aes.h>
#include <watchdog.h>
#include "bcm_secure.h"

#define FIT_PAD_MAX_SIZE 0x1000
#define BCM_TAG_FIT_HDR_SEC_DELG_REC    0x44454c47 // DELG
#define BCM_TAG_FIT_HDR_SEC_SIGNATURE	0x46495453 // FITS
#define BCM_TAG_FIT_HDR_SEC_PUBKEY	0x46495450 // FITP
#define BCM_FIT_HDR_MAX_RECORDS		4	
#define BCM_FIT_HDR_MIN_HDR_SIZE	(RSA2048_BYTES+sizeof(u32))

 
int bcm_tag_fit_hdr_validate(const u8 *fit, u32 size, 
		u32 hdr_size, u8* master_pub)
{
	int rc = -1;
	u32 tag = 0, cnt = 0;
	u8 sig[RSA2048_BYTES];
	u8 *p = (u8*)fit + size;
	u8 *pmax = p; 
	u8 key_buf[RSA2048_BYTES];
	const u8 *key = master_pub;
	struct image_sign_info im;

#if defined (CONFIG_TPL_BUILD)
	u32 sdr_plus_sig_size = 0;
	int sdr_verified = 0;
#endif 	

	if (hdr_size < BCM_FIT_HDR_MIN_HDR_SIZE) {
		goto err;
	}	
	
	im.checksum = image_get_checksum_algo("sha256,");
	if (!im.checksum) {
		printf("ERROR: couldn't get checksum algo\n"); 
		goto err;
	}
	while (cnt < BCM_FIT_HDR_MAX_RECORDS && (p - pmax) < hdr_size) {
		memcpy(&tag, p, sizeof(u32));
		switch ( tag ) {
#if defined (CONFIG_TPL_BUILD)
			case BCM_TAG_FIT_HDR_SEC_DELG_REC:
				rc = bcm_sec_delg_process_sdr(p, pmax+hdr_size-1, &sdr_plus_sig_size);
				if( rc ) {
					printf("ERROR: SDR verification failed!\n");
					return rc;
				} else {
					/* SDR valid, swap in new pub key */
					key = bcm_sec_get_active_pub_key();
					p += sdr_plus_sig_size; 
					sdr_verified = 1;
				}
			break;
#endif /* defined (CONFIG_TPL_BUILD) */
			case BCM_TAG_FIT_HDR_SEC_SIGNATURE:
				memcpy(sig, p + sizeof(u32), RSA2048_BYTES);
				debug("\nVerifying Signature; 4 leading bytes 0x%x verifying with key 0x%x\n",*(u32*)sig, *(u32*)key);
				if (!bcm_sec_rsa_verify(fit, size, sig,  RSA2048_BYTES, key, &im )) {
					rc = 0;
#if defined (CONFIG_TPL_BUILD)
					/* Upon FIT hdr & sdr authentication success, check sec node */
					if( sdr_verified ) {
						rc = bcm_sec_delg_process_sec_node(fit);
						if( rc ) {
							printf("ERROR: Security Node verification failed!\n");
						}
					}
#endif /* defined (CONFIG_TPL_BUILD) */
					return rc;
				}
				p += sizeof(u32) + RSA2048_BYTES; 
			break;
			case BCM_TAG_FIT_HDR_SEC_PUBKEY:
#if defined (CONFIG_TPL_BUILD)
				if( !sdr_verified ) 
#endif					
				{
					memcpy(key_buf, p + sizeof(u32), RSA2048_BYTES);
					memcpy(sig, p + sizeof(u32) + RSA2048_BYTES , RSA2048_BYTES);
					debug("\nGot Key and Signature, 4 leading bytes: key 0x%x sig 0x%x\n", 
							*(u32*)key_buf, *(u32*)sig);
					if (!bcm_sec_rsa_verify(key_buf, RSA2048_BYTES, sig,  RSA2048_BYTES, master_pub, &im )) {
						key = key_buf; 
					} else {
						printf("The key 0x%x can't be verified\n", *(u32*)key_buf);
					} 

					p += sizeof(u32) + RSA2048_BYTES*2;
					cnt++; 
				}
			break;
			default:
				p++;
		}
	} 
err:
	return rc;
}

ulong bcm_sec_get_reqd_load_size( void * fit )
{
	return( fdt_totalsize(fit) + FIT_PAD_MAX_SIZE );
}

int bcm_sec_validate_fit(void* fit, u32 max_image_size)
{
	int rc = -1;
	ulong size = fdt_totalsize(fit);
	bcm_sec_state_t st = bcm_sec_state();

	if (FIT_PAD_MAX_SIZE > max_image_size) {
		printf("ERROR: FIT image is too short to process\n");
		goto _die;
	}

	u8* key = bcm_sec_get_active_pub_key();
	if (!key) {
		if((st == SEC_STATE_UNSEC)) {
			rc = 0;
		}
		goto _die;
	}

	rc = bcm_tag_fit_hdr_validate(fit, size, FIT_PAD_MAX_SIZE, (u8*)key);
	if (rc) {
		goto _die;
	}
	rc = 0;
	printf("FIT Header Authentication Successfull!\n");
_die:
	if(rc ) {
		printf("FAILED to authenticate FIT header!!! \n");
		if(st & SEC_STATE_SECURE) {
			bcm_sec_abort();
		} 
	}
	return rc; 
}

u8* bcm_util_get_fdt_prop_data(void* fdt, char* path, char *prop, int* len)
{
	u8* d = NULL; 
	int node = fdt_path_offset (fdt, path);
	if(node < 0) {
		debug("ERROR: unable to find path fdt %s \n", path);
		goto err;
	} 
	d = (u8*)fdt_getprop(fdt, node, prop, len);
	if (!d) {
		debug("ERROR: unable to get %s\n", prop);
		goto err;
	}
err :
	return d;
}
