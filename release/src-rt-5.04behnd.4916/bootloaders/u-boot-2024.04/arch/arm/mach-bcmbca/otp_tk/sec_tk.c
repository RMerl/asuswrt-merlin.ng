/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2020 Broadcom Ltd.
 */

#include <linux/types.h>
#include <linux/io.h>
#include <stdio.h>
#include <linux/delay.h>
#include "linux/printk.h"
#include <asm/arch/misc.h>
#include "bcm_otp.h"
#include <u-boot/rsa.h>
#include <u-boot/sha256.h>
#include <uboot_aes.h>
#include "bcm_secure.h"
#include "bcm_rng.h"
#include "tk_ks.h"



#ifdef DRY_RUN
void dbg_prt_array(u8* d, u32 sz) {
	u8* p = d;
	while(p < d+sz) {
		if (!((p-d)%32))  printf("\n");
		
		printf("%02x",*p++);
	}
	printf("\n");
}

#define __tr printf("%s:%d \n",__FUNCTION__,__LINE__); 
#define prta(___a,___sz)  dbg_prt_array(___a,___sz) 

#else

#define prta(___a,___sz) 
#define __tr

#endif

#define	TM_SAMPLE_RATE 10
static char _get_char(void)
{
	return tstc() ? getchar():'\0';
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

static ks_err_t sec_tk_fuse_verify(otp_map_feat_t id, 
			const u8* data, 
			u32 size,
			int verify)
{
	u32	*data_verify = NULL,
		size_verify = 0;
	otp_map_cmn_err_t rc = KS_ERR_SUCC; 
	rc = bcm_otp_read(id, &data_verify, &size_verify);
	if (rc) {
		if (rc == OTP_HW_CMN_ERR_KEY_EMPTY) {
			rc = KS_ERR_SUCC;
		} else {
			goto err;
		}
	}
#ifndef  DRY_RUN
	if (!verify){
		rc = bcm_otp_write(id, data, size);
	}
#endif
	prta(data,size);
err:
	if (rc) {
		rc = KS_ERR_OTP_FUS;
		printf("EFSD\n");
	}
	return rc;
}


static inline int sec_tk_do_req(u32 sec_arch,
				bcm_sec_state_t sec_state)
{
	switch(sec_arch) {
		case SEC_ARCH_GEN3:
			if (!ks_run_request(sec_state, sec_tk_fuse_verify)) {
				break;
			}
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
			rng_init();
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
	if (sec_tk_do_req(SEC_ARCH_GEN3, state /*,&req_info*/) != 0) {
		goto err;	
	}
	printf("POR!\n");
done:
	return 0;
err:
	printf("SERR\n");
	return -1;
}

