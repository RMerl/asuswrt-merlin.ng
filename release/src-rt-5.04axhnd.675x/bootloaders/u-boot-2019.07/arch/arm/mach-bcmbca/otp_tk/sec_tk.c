/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2020 Broadcom Ltd.
 */
#include <common.h>
#include <linux/types.h>
#include <linux/io.h>
#include "linux/printk.h"
#include <asm/arch/misc.h>
#include "bcm_otp.h"
#include <u-boot/rsa.h>
#include <u-boot/sha256.h>
#include <uboot_aes.h>
#include "bcm_secure.h"
#include "tk_ks.h"

//#define DRY_RUN

#define	TM_SAMPLE_RATE 10

static char _get_char(void)
{
	return tstc() ? getc():'\0';
}

static inline int sec_tk_check_abort(int sec)
{
    char str[8],ch;
    sec *= TM_SAMPLE_RATE;
    while (sec > 0) {
	ch = _get_char();
        if (ch == 'c' || ch == 'C') {
            break;
        }
	if (!(sec%TM_SAMPLE_RATE)) {
        	sprintf(str,"%3d",sec/TM_SAMPLE_RATE);
	}
       	puts(str);
        putc('\r');
        memset(str, 0, 8);
        mdelay(1000/TM_SAMPLE_RATE);
        sec--;
    }
    return sec;
}


static inline void ntohl_array(u32* a, u32 length) 
{
    int i;
    for (i = 0;i < length; i++)  {
         a[i] = ntohl(a[i]);
    }
}


static int sec_tk_fuse_verify(otp_map_feat_t id, 
			const u8* data, 
			u32 size)
{
	u32	*data_verify = NULL,
		size_verify = 0;
	otp_map_cmn_err_t rc = OTP_MAP_CMN_OK; 
	rc = bcm_otp_read(id, &data_verify, &size_verify);
	if (rc) {
		if (rc != OTP_HW_CMN_ERR_KEY_EMPTY) {
			goto err;
		}
		rc = OTP_MAP_CMN_OK;
	}
#ifndef  DRY_RUN 
	if (!data || !(*data)) {
		printf("EINV\n");
		goto err;
	}
	rc = bcm_otp_write(id, data, size);
	if (rc) {
		printf("EFSD\n");
		goto err;
	}
#else 
	do{
		int i;
		printf("Fusing with size %d data: \n\t",size);
		for (i = 0; i < size; i++) {
			printf(" 0x%x ",data[i]);
		}
		printf("\n");
	}while(0);
#endif
err:
	return rc;
}

static inline int sec_tk_commit_req(ks_req_state_t req_state, 
                                bcm_sec_state_t cur_state)
{
        int res, rc = 0;
	
        switch(cur_state) {
                case SEC_STATE_UNSEC:
                        if (req_state == KS_REQ_TRANSIT_FLD || 
                                req_state == KS_REQ_TRANSIT_MFG){
                                printf("MFG\n");
				u32 val = 0x1, size = 4;
				if (sec_tk_fuse_verify(OTP_MAP_BRCM_BTRM_BOOT_ENABLE, &val, size )) {
					goto err;
				}
				val = 0x7;
				if (sec_tk_fuse_verify(OTP_MAP_CUST_BTRM_BOOT_ENABLE, &val, size )) {
					goto err;
				}
                        }
                        break;
                case SEC_STATE_GEN3_MFG:
                        if (req_state == KS_REQ_TRANSIT_FLD) {
                                int res;
                        	/*program otp bits accordingly */
                                u8 ek_iv[KS_AES_128_CBC_SZ+KS_AES_128_CBC_SZ],
                                                hash[KS_HASH_SZ]; 
                                u32 mid;

                                if (ks_get_data_info(KS_DATA_TYPE_KEY_AES_CBC_128_EK, 
                                        KS_DATA_STATE_FLD_ENCR, ek_iv) != KS_ERR_SUCC) {
                                        goto err;
                                }
                                if (ks_get_data_info(KS_DATA_TYPE_KEY_AES_CBC_128_IV, 
                                        KS_DATA_STATE_FLD_ENCR, ek_iv+KS_AES_128_CBC_SZ) != KS_ERR_SUCC) {
                                        goto err;
                                }
                                if (ks_get_data_info(KS_DATA_TYPE_HASH, 
                                                KS_DATA_STATE_RAW, hash) != KS_ERR_SUCC) {
                                        goto err;
                                }
                                if (ks_get_data_info(KS_DATA_TYPE_MID, 
                                        KS_DATA_STATE_RAW, &mid) != KS_ERR_SUCC) {
                                        goto err;
                                }
                                printf("FLD\n");
				/*
                                 since sotp interface dealing with 32 bit integers - assumption is
                                 that raw byte data from keystore are always honoring big endian 
                                */
				mid = ntohl(mid);
                                ntohl_array((u32*)ek_iv,(KS_AES_128_CBC_SZ*2)/sizeof(u32)); 
                                ntohl_array((u32*)hash,(KS_HASH_SZ)/sizeof(u32)); 
				rc = sec_tk_fuse_verify(OTP_MAP_CUST_MFG_MRKTID, &mid, sizeof(u32));
				if (rc) {
                                        goto err;
				}
				rc = sec_tk_fuse_verify(SOTP_MAP_FLD_HMID, hash, KS_HASH_SZ);
				if (rc) {
                                        goto err;
				}
				rc = sec_tk_fuse_verify(SOTP_MAP_FLD_ROE, ek_iv, KS_AES_128_CBC_SZ*2);
				if (rc) {
                                        goto err;
				}
                        }
                        break;
                default:
                        goto err;
        }
        return 0;
err:    
        return -1;
}


static inline int sec_tk_do_req(u32 sec_arch,
				bcm_sec_state_t sec_state,
                                ks_req_info_t *req_info)
{
	/*
        if (ks_get_req_info(&req_info) != KS_ERR_SUCC) {
                goto err;
        }*/
        
        switch(sec_arch) {
                case SEC_ARCH_GEN3:
                        if (!sec_tk_commit_req(req_info->state, sec_state)) {
                        	break;
			}
                case SEC_ARCH_GEN2:
                case SEC_ARCH_NONE:
                case SEC_ARCH_GEN1:
                default:
                        goto err;
        }
        return 0;
err:
        printf("EREQ\n");
        return -1;
}

static int sec_tk_allow_req(bcm_sec_state_t sec_state, 
                        ks_req_state_t req_state)
{
        switch (sec_state) {
                case SEC_STATE_UNSEC:
                        if (req_state == KS_REQ_TRANSIT_MFG ||
                                req_state == KS_REQ_TRANSIT_FLD) {
                                return 1;       
                        }
                        break;
                case SEC_STATE_GEN3_MFG:
                        if (req_state == KS_REQ_TRANSIT_FLD) {
                                return 1;       
                        }
                        break;
                default:
                        break;
        }
        return 0; 
}

int sec_tk()
{	
	char* msg = NULL;
	u8* aes_key;
        ks_req_info_t req_info;
	bcm_sec_state_t state = bcm_sec()->state;
        printf("SECE\n");
        printf("GEN3\n");
	bcm_sec_get_active_aes_key(&aes_key);
        if (ks_init(state,
		SEC_ARCH_GEN3, 
		bcm_sec_get_active_pub_key(),
		aes_key) != KS_ERR_SUCC) {
                goto err;
        }
        if (ks_get_req_info(&req_info) != KS_ERR_SUCC) {
                goto err;
        }

        if (!sec_tk_allow_req(state, req_info.state)) {
                printf("FUSD\n");
                goto done;
        }
        switch (state) {
                case SEC_STATE_UNSEC:
                        msg = "NSEC\n";
                        break;
                case SEC_STATE_GEN3_MFG:
                        msg = "MFG\n";
                        break;
                default:
                        goto err;
        }
       	printf(msg);
        printf("ABT?\n");
        if (sec_tk_check_abort(req_info.abort_delay)) {
                
                printf("CNCL\n");
                goto done;
        }
        if (sec_tk_do_req(SEC_ARCH_GEN3, state ,&req_info) != 0) {
		goto err;	
	}
        printf("POR!\n");
done:
        return 0;
err:
        printf("SERR\n"); 
        return -1;
}

