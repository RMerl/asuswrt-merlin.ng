/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Broadcom Ltd.
 */
#include <common.h>
#include <wdt.h>
#include <asm/types.h>
#include <asm/byteorder.h>
#include <linux/errno.h>
#include <asm/unaligned.h>
#include <malloc.h>
#include "u-boot/rsa.h"
#include "u-boot/rsa-mod-exp.h"
#include <u-boot/sha256.h>
#include <uboot_aes.h>
#include "bcm_secure.h"
#include "bcm_otp.h"
#include <watchdog.h>


/* callback can be called early in before relocation and 
 * before bss zero loop */
static bcm_sec_t s_bcm_sec __attribute__((section(".data"))) = {0};
int secBoot = 0;


static inline void copy_append(bcm_sec_ctrl_arg_t* d, bcm_sec_ctrl_arg_t* s, u32 len)
{
		int i,j;
		for(i = 0; i < len; i++) {
			if (!s[i].ctrl) {
				continue;	
			}
			for(j = 0; j < len; j++) {
				if ( (u32)d[j].ctrl == (u32)s[i].ctrl ||
						!((u32)d[j].ctrl) ) {
					d[j].ctrl = s[i].ctrl;
					d[j].ctrl_arg = s[i].ctrl_arg;
					break;
				}
			} 
		}
}
 

static void bcm_sec_set_args(bcm_sec_t *sec, bcm_sec_cb_arg_t args[SEC_CTRL_ARG_MAX])
{
	int i;
	for (i = 0; i < SEC_CTRL_ARG_MAX; i++) {
 		copy_append(sec->cb[i].arg, 
			args[i].arg, SEC_CTRL_RUN_ORDER_MAX);
	}
}



static bcm_sec_ctrl_arg_t* bcm_sec_get_ctrl_arg(bcm_sec_t* sec, 
			bcm_sec_ctrl_arg_t* ctrl,
			bcm_sec_ctrl_arg_num_t ctrl_id)
{
	int i;
	bcm_sec_ctrl_arg_t *c; 
	bcm_sec_ctrl_arg_t *res = NULL;
	c = sec->cb[ctrl_id].arg;
	for(i = 0; i < SEC_CTRL_RUN_ORDER_MAX; i++) {
		if (!c[i].ctrl) {
			continue;	
		}
		if ( (u32)c[i].ctrl == (u32)ctrl->ctrl) {
			res = &c[i];
			break;
		}
	}
	return res;
}

int	bcm_sec_update_ctrl_arg(bcm_sec_ctrl_arg_t* arg,
			bcm_sec_ctrl_arg_num_t ctrl)
{
	bcm_sec_ctrl_arg_t *_arg = bcm_sec_get_ctrl_arg(bcm_sec(), arg, ctrl);
	if (!_arg) {
		return -1; 
	}
	if (ctrl == SEC_CTRL_ARG_KEY && ( 
		(_arg->ctrl == SEC_CTRL_KEY_CHAIN_AES) || (_arg->ctrl == SEC_CTRL_KEY_CHAIN_ENCKEY) || (_arg->ctrl == SEC_CTRL_KEY_EXPORT_ITEM)) ) {
		((bcm_sec_key_arg_t*)_arg->ctrl_arg)->arg = arg->ctrl_arg;
	} else { 
		_arg->ctrl_arg = arg->ctrl_arg;
	}  
	return 0;
}

/* based on the content of theotp field returns state - unsec, mfg or fld*/
static int bcm_sec_boot_state(bcm_sec_state_t* sec_state)
{
	otp_hw_ctl_data_t ctl_data = { 
		.addr = SOTP_MAP_FLD_ROE,
		.status = (OTP_HW_CMN_STATUS_ROW_DATA_VALID|OTP_HW_CMN_STATUS_ROW_RD_LOCKED)
	};
	otp_hw_cmn_ctl_cmd_t cmd = {
		.ctl = OTP_HW_CMN_CTL_STATUS ,
		.data = (uintptr_t)&ctl_data, 
		.size = sizeof(ctl_data)
	};
	int rc = -1;
	u32 *btrm_en = NULL,
		*btrm_cust_en = NULL,
		*cust_mid = NULL, size = 0, res = 0;
	*sec_state = SEC_STATE_UNSEC;
	if (bcm_otp_read(OTP_MAP_CUST_BTRM_BOOT_ENABLE, &btrm_cust_en, &size) ||
		bcm_otp_read(OTP_MAP_BRCM_BTRM_BOOT_ENABLE, &btrm_en, &size) ||
		bcm_otp_read(OTP_MAP_CUST_MFG_MRKTID, &cust_mid, &size)) {
		goto err;	
	}
	if ( !(*btrm_en) ||  !(*btrm_cust_en)) {
		goto done;	
	}
	*sec_state = SEC_STATE_GEN3_MFG;
	if ( !(*cust_mid) ) {
		goto done;
	}

	if (bcm_otp_ctl(BCM_SOTP_MAP, &cmd, &res)) {
		goto err;	
	}
	if (!(res&OTP_HW_CMN_STATUS_ROW_DATA_VALID)) {
		/*printf("%s ek: 0x%x \n",__FUNCTION__,*(u32*)BCM_SECBT_CRED_AES);*/
                if (!(res&OTP_HW_CMN_STATUS_ROW_RD_LOCKED) ) { 
		        printf("Warning: Requested FLD ROE (%d) is invalid or empty. MID %x \n",SOTP_MAP_FLD_ROE, *cust_mid);
			goto done;
		}
	}
	res = 0;
	ctl_data.addr = SOTP_MAP_FLD_HMID;
	if (bcm_otp_ctl(BCM_SOTP_MAP, &cmd, &res)) {
		goto err;	
	}
	if (!(res&OTP_HW_CMN_STATUS_ROW_DATA_VALID)) {
		/*printf("%s pub: 0x%x \n",__FUNCTION__,*(u32*)BCM_SECBT_CRED_MOD);*/
                if (!(res&OTP_HW_CMN_STATUS_ROW_RD_LOCKED)) { 
		        printf("Warning: Requested FLD HMID (%d) is invalid or empty. MID %x \n",SOTP_MAP_FLD_HMID, *cust_mid);
			goto done;
		}
	}
	*sec_state = SEC_STATE_GEN3_FLD;
done:
	rc = 0;
err:
	return rc;
}

int bcm_sec_set_sec_ser_num( char * ser_num, u32 ser_num_size)
{
	int rc = -1;

	/* Commit the new secure serial number to S/OTP */
	rc = bcm_otp_write(SOTP_MAP_SER_NUM, ser_num, ser_num_size);
	if(rc) {
		printf("%s: ERROR! Failed to commit secure serial number! rc:%d\n", __FUNCTION__, rc);
	}
	return rc;
}

int bcm_sec_get_sec_ser_num( char * ser_num, u32 ser_num_size)
{
	int rc = -1;
	u32 * pdata = NULL;
	u32 size = 0;
	u32 num_words = 0;

	/* Get secure serial number fields from S/OTP */
	rc = bcm_otp_read(SOTP_MAP_SER_NUM, &pdata, &size);

	if( rc == OTP_HW_CMN_ERR_KEY_EMPTY )
	{
		memset(ser_num, 0x0, ser_num_size);
		return OTP_HW_CMN_OK;
	}

	/* If retrieval fails, check dtb for exported value */
	if( rc ) {
		int node;
		node = fdt_path_offset(gd->fdt_blob, "/trust/serial_num");
		if (node >= 0) {
			char * cp = NULL;
			cp = (char *)fdt_getprop(gd->fdt_blob, node, "value", &size);
			if (cp) {
				memcpy(ser_num, cp, (size>ser_num_size?ser_num_size:size));
				rc = 0;
			}
		}
		if(rc) {
			printf("%s: ERROR! Failed to retrieve secure serial number! rc:%d\n", __FUNCTION__, rc);
		} 
	} else {
		memcpy(ser_num, pdata, (size>ser_num_size?ser_num_size:size));
	}
	return rc;
}

int bcm_sec_set_dev_spec_key( char * dev_spec_key, u32 dev_spec_key_size)
{
	int rc = -1;

	/* Commit the new device specific key to S/OTP */
	rc = bcm_otp_write(SOTP_MAP_KEY_DEV_SPECIFIC, dev_spec_key, dev_spec_key_size);
	if(rc) {
		printf("%s: ERROR! Failed to commit secure serial number! rc:%d\n", __FUNCTION__, rc);
	}
	return rc;
}

int bcm_sec_get_dev_spec_key( char * dev_spec_key, u32 dev_spec_key_size)
{
	int rc = -1;
	u32 * pdata = NULL;
	u32 size = 0;
	u32 num_words = 0;

	/* Get device specific from S/OTP */
	rc = bcm_otp_read(SOTP_MAP_KEY_DEV_SPECIFIC, &pdata, &size);

	if( rc == OTP_HW_CMN_ERR_KEY_EMPTY )
	{
		memset(dev_spec_key, 0x0, dev_spec_key_size);
		return OTP_HW_CMN_OK;
	}

	if(rc) {
		printf("%s: ERROR! Failed to retrieve secure serial number! rc:%d\n", __FUNCTION__, rc);
	} else {
		memcpy(dev_spec_key, pdata, (size>dev_spec_key_size?dev_spec_key_size:size));
	}
	return rc;
}

#define ANTI_ROLLBACK_LVL_NUMBITS	2
#define ANTI_ROLLBACK_LVL_MASK		((1 << ANTI_ROLLBACK_LVL_NUMBITS)-1)
#define ANTI_ROLLBACK_LVL_SHIFT		ANTI_ROLLBACK_LVL_NUMBITS
static int bcm_sec_antirollback_lvl_ctl( u32 * lvl, int write )
{
	int rc = -1;
	u32 * pdata = NULL;
	u32 size = 0;
	u32 current_lvl = 0;
	u32 num_lvls_word = sizeof(u32)*8/ANTI_ROLLBACK_LVL_NUMBITS;
	u32 num_words = 0;
	u32 i,j,tmp_lvl;

	/* Get anti_rollback fields from S/OTP */
	rc = bcm_otp_read(SOTP_MAP_ANTI_ROLLBACK, &pdata, &size);
	if (rc) {
		if ( rc == OTP_MAP_CMN_ERR_UNSP ) {
			/* ANTI-ROLLBACK not supported, return with level 0 */
			printf("INFO: anti-rollback level not implemented. Returning default level:0\n");
			current_lvl = 0;
			rc = 0;
		} else {
			printf("%s: ERROR! Failed to retrieve anti-rollback level data! rc:%d\n", 
			__FUNCTION__, rc);
		}
		goto finish_read;	
	}

	/* Calculate level */
	num_words = size/sizeof(u32);
	for( i=0; i<num_words; i++ ){
		for( j=0; j<num_lvls_word; j++) {
			if( pdata[i] & (ANTI_ROLLBACK_LVL_MASK << (j*ANTI_ROLLBACK_LVL_SHIFT))) {
				current_lvl++;
			} else {
				goto finish_read;
			}
		}
	}

finish_read:
	if( write ) {
		tmp_lvl = *lvl;
		/* Do boundary checks on requested level */
		if( tmp_lvl <= current_lvl ) {
			printf("%s: ERROR! Requested anti-rollback level %d is lower than current %d\n", __FUNCTION__, tmp_lvl, current_lvl);
			rc = -1;
			goto finish;
		}
		if( tmp_lvl > num_words * num_lvls_word ) { //FIXME BOUNDARY
			printf("%s: ERROR! Requested anti-rollback level %d is higher than maximum %d\n", __FUNCTION__, tmp_lvl, num_words * num_lvls_word );
			rc = -1;
			goto finish;
		}

		/* Encode new rollback level */
		for( i=0; i<num_words; i++ ){
			/* Batch fill the row if level exceeds it */
			if( tmp_lvl >= num_lvls_word )
			{
				/* adjust level */
				tmp_lvl -= num_lvls_word;

				/* If current word is filled continue to next word */
				if( pdata[i] == 0xFFFFFFFF )
					continue;
				else
					pdata[i] = 0xFFFFFFFF;
			}
			else
			{
				/* Selectively fill the row according to level */
				while( tmp_lvl )
				{
					pdata[i] |= ANTI_ROLLBACK_LVL_MASK << ((tmp_lvl-1)*ANTI_ROLLBACK_LVL_SHIFT);
					tmp_lvl--;
				}
			}
			if( tmp_lvl == 0 )
				break;
		}

		/* Commit the new anti-rollback level to S/OTP */
		rc = bcm_otp_write(SOTP_MAP_ANTI_ROLLBACK, pdata, size);
		if(rc) {
			printf("%s: ERROR! Failed to commit anti-rollback level data! rc:%d\n", __FUNCTION__, rc);
			goto finish;	
		}
	} else {
		*lvl = current_lvl;
	}
finish:
	return rc;
}

int bcm_sec_get_antirollback_lvl( u32 * lvl)
{
	return(bcm_sec_antirollback_lvl_ctl( lvl, 0 ));
}
int bcm_sec_set_antirollback_lvl( u32 lvl)
{
	return(bcm_sec_antirollback_lvl_ctl( &lvl, 1 ));
}
/* assigns a number to a callback to be run ascending order in bcm_sec_do*/
static void bcm_set_cb_order(bcm_sec_ctrl_arg_num_t ord_arg[SEC_CTRL_ARG_MAX])
{
	bcm_sec_ctrl_arg_num_t *ord = bcm_sec()->ord;
	if (ord_arg) {
		memcpy(ord, ord_arg, 
				SEC_CTRL_ARG_MAX*sizeof(bcm_sec_ctrl_arg_num_t));
	} else {
		int i;
		for (i = 0; i < SEC_CTRL_ARG_MAX; i++) {
			ord[i] = i;
		}
	}
}

bcm_sec_t* bcm_sec(void)
{
	return &s_bcm_sec;
}


bcm_sec_state_t bcm_sec_state(void)
{
	return bcm_sec()->state;
}

void bcm_sec_get_active_aes_key(u8** key)
{
	*key = bcm_sec()->key.ek;
}

void bcm_sec_set_active_aes_key(u8* key)
{
	bcm_sec()->key.ek = key;
}

u8* bcm_sec_get_active_pub_key(void)
{
	return bcm_sec()->key.pub;
}

u8* bcm_sec_set_active_pub_key(u8 * key)
{
	bcm_sec()->key.pub = key;
}

__weak void bcm_sec_clean_secmem(bcm_sec_t* sec)
{

}

__weak int bcm_sec_btrm_key_info(bcm_sec_t* sec)
{
	/* Set root keys */
	memcpy(sec->key.rsa_pub, (void*)BCM_SECBT_CRED_MOD, 
			RSA2048_BYTES);
	memcpy(sec->key.aes_ek, (void*)BCM_SECBT_CRED_AES, 
			BCM_SECBT_AES_CBC128_EK_LEN); 
	memcpy(sec->key.aes_ek + BCM_SECBT_AES_CBC128_EK_LEN,  
		(void*)BCM_SECBT_CRED_AES_IV, BCM_SECBT_AES_CBC128_EK_LEN);

	/* Set active keys */
	sec->key.pub = sec->key.rsa_pub;
	sec->key.ek = sec->key.aes_ek;

	/* Clear delegation config */
	sec->delg_cfg_obj = NULL;

	return 0;	
}

void bcm_sec_clean_keys(bcm_sec_t* sec)
{
	if (sec->key.ek) {
		memset(sec->key.ek, 0, BCM_SECBT_AES_CBC128_EK_LEN*2);
	}
	memset(&sec->key, 0, sizeof(bcm_sec_key_t));
}

/* should be overridden by the implementation */
__weak void bcm_sec_cb_init(bcm_sec_t* obj)
{
	
}
 
__weak void bcm_sec_init(void)
{
	bcm_sec_t* sec = bcm_sec();
	bcm_sec_cb_init(sec);
	bcm_set_cb_order(NULL);	
	bcm_sec_boot_state(&sec->state);
	switch (sec->state) {
		case SEC_STATE_GEN3_MFG:
			secBoot=2;
			printf("Board is MFG secure\n");
			break;
		case SEC_STATE_GEN3_FLD:
			secBoot=3;
			printf("Board is FLD secure\n");
			break;
		default:
			secBoot=1;
			printf("Board is non secure\n");
	}
}

__weak int bcm_sec_do(bcm_sec_ctx_t ctx, bcm_sec_cb_arg_t arg[SEC_CTRL_ARG_MAX])
{
	int rc = -1, i, j;
	bcm_sec_t *sec = bcm_sec();
	bcm_sec_ctrl_cb_t *__cb; 
	bcm_sec_ctrl_arg_t *cb_arg;
	if (arg) {
		bcm_sec_set_args(sec, arg);
	}
	/* This is a 2 step function.  eg SEC_LEVEL_NONE->SEC_LEVEL_INIT->FINAL LEVEL(see below) .
 	 */
	if (ctx & SEC_SCHED_CLR) {
		sec->sched_ctx =  SEC_NONE;
		return rc;
	}

	if (sec->sched_ctx & SEC_SET_SCHED) {
		/* continuing with schedule level*/
		ctx = (sec->sched_ctx & (~SEC_SET_SCHED));
	} else if (ctx & SEC_SET_SCHED) {
		sec->sched_ctx =  ctx;
		//debug("SEC *** SCHEDULED \n");
		return rc;
	}
	for (j = 0; j < SEC_CTRL_ARG_MAX; j++) {
		__cb = &sec->cb[sec->ord[j]];
		if (!__cb->cb) {
			continue;
		}
		cb_arg = __cb->arg;
		for (i = 0;  i < SEC_CTRL_RUN_ORDER_MAX; i++) {
#if DEBUG 
			if (cb_arg[i].ctrl) {
				printf("SEC *** DO %d CTRL %d  parm 0x%x\n", j, 
							cb_arg[i].ctrl,
							cb_arg[i].ctrl_arg);
			}
#endif 
			if (__cb->cb(sec, cb_arg[i].ctrl, cb_arg[i].ctrl_arg)) {
				cb_arg[i].ctrl = 0;
				cb_arg[i].ctrl_arg = NULL;
				goto err;
			}
			cb_arg[i].ctrl = 0;
			cb_arg[i].ctrl_arg = NULL;
		} 
	}
	sec->curr_ctx = ctx;
	rc = 0;
err:
	return rc;
}

void bcm_sec_deinit(void)
{
	bcm_sec_clean_secmem(&s_bcm_sec);
	bcm_sec_clean_keys(&s_bcm_sec);
	memset(&s_bcm_sec, 0, sizeof(bcm_sec_t));
}

void bcm_sec_abort(void)
{
	bcm_sec_deinit();
	WATCHDOG_RESET();
	/* we never return if watchdog wasn't enabled */
	hang();
}
