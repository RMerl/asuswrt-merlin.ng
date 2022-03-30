/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2020 Broadcom Ltd.
 */

#ifndef _BCM_SECURE_H
#define _BCM_SECURE_H
#include <asm/arch/brom.h>
#define	BCM_RSA				"rsa2048"
#define	BCM_PADDING			"emsa_pss"
#define	BCM_CHECKSUM			"sha256"
/*Sizes in bytes*/
#define BCM_SECBT_RSA_PUBEXP		65537
#define BCM_SECBT_RSA2048_MOD_LEN	256	
#define BCM_SECBT_AES_CBC128_EK_LEN	16	
#define BCM_SECBT_AES_CBC128_IV_LEN	16	
#define BCM_SEC_BOOTROM_CRED_ADDR	
#define	BCM_SECBT_CRED		(bcm_secbt_args())
#define	BCM_SECBT_CRED_MOD	BCM_SECBT_CRED->auth.manu 
#define	BCM_SECBT_CRED_AES	BCM_SECBT_CRED->encr.bek 
#define	BCM_SECBT_CRED_AES_IV	BCM_SECBT_CRED->encr.biv

#ifdef CONFIG_BCMBCA_EARLY_ABORT_JTAG_UNLOCK 
#define BCM_SEC_UNLOCK_JTAG BROM_GEN_JTAG_UNLOCK 
#else
#define BCM_SEC_UNLOCK_JTAG
#endif

#include "image.h"

#define FIT_AES1 "fit-aes1"
#define FIT_AES2 "fit-aes2"

struct bcm_secbt_auth_args {
	uint8_t	manu[BCM_SECBT_RSA2048_MOD_LEN] __attribute__ ((aligned (4)));
	uint8_t	oper[BCM_SECBT_RSA2048_MOD_LEN] __attribute__ ((aligned (4)));
};

struct bcm_secbt_encr_args {
	uint8_t	bek[BCM_SECBT_AES_CBC128_EK_LEN];
	uint8_t	iek[BCM_SECBT_AES_CBC128_EK_LEN];
	uint8_t	biv[BCM_SECBT_AES_CBC128_IV_LEN];
	uint8_t	iiv[BCM_SECBT_AES_CBC128_IV_LEN];
};

typedef struct __bcm_secbt_args{
	struct bcm_secbt_auth_args	auth __attribute__ ((aligned (4)));
	struct bcm_secbt_encr_args	encr;
} bcm_secbt_args_t;

typedef struct _bcm_sec_key_aes_arg {
	u8	key[BCM_SECBT_AES_CBC128_EK_LEN*2];
	char	id[64];
} bcm_sec_key_aes_arg_t;

typedef struct _bcm_sec_enc_key_arg {
	char * name;
	char * perm;
	char * algo;
	u8 * data;
	u32 size;
	u32 size_enc;
	u64 load_addr;
} bcm_sec_enc_key_arg_t;

typedef struct _bcm_sec_export_item_arg {
	char * id;
	char * name;
	u32 salt;
	char * algo;
	u8 * value;
	u8 exp_flag;
	u32 len;
} bcm_sec_export_item_t;

typedef	struct _bcm_sek_key_arg {
	int len; 
	void*	arg;
	union {
		bcm_sec_enc_key_arg_t * enc_key; 
		bcm_sec_export_item_t * item;
		bcm_sec_key_aes_arg_t * aes;
	};
} bcm_sec_key_arg_t;

typedef struct _bcm_sec_key_ {
	uint8_t rsa_pub[BCM_SECBT_RSA2048_MOD_LEN];
	uint8_t aes_ek[BCM_SECBT_AES_CBC128_EK_LEN*2];
	u8*	pub;
	union {
		u8*	ek;
		void*	ch_ek;
	};
} bcm_sec_key_t;


static inline volatile bcm_secbt_args_t * bcm_secbt_args(void) {
	return (volatile bcm_secbt_args_t*)(CONFIG_SYS_SEC_CRED_ADDR);
}

typedef enum _bcm_sec_states { 
	SEC_STATE_UNSEC = 0,
	SEC_STATE_GEN3_MFG = 0x1,
	SEC_STATE_GEN3_FLD = 0x2 
} bcm_sec_state_t;

#define SEC_STATE_SECURE (SEC_STATE_GEN3_MFG|SEC_STATE_GEN3_FLD)

/* Security runtime context */
typedef enum _bcm_sec_ctx { 
	SEC_NONE = 0x0,
	SEC_INIT = 0x1, /* various initializations if any */
	SEC_SET = 0x2, /* commits various parameters for keys and sotp permissions*/
	SEC_SET_SCHED = 0x100, /* Schedules multiple parameters to be set(run)  
						later by invoking SEC_SET*/ 
	SEC_SCHED_CLR = 0x200, /*  clears the schedule mask; */
} bcm_sec_ctx_t;

typedef enum _bcm_sec_ctrl {
	SEC_CTRL_NONE = 0,
	SEC_CTRL_FIT_AUTH, 		/* verify signature */
	SEC_CTRL_FIT_SEC, 		/* FIT secure header process  */
	SEC_CTRL_DT_CHAIN_SHA, 	/* verify signature */
	SEC_CTRL_SOTP_LOCK_ALL, /*default - everything is open  */
	SEC_CTRL_SOTP_UNLOCK_SOTP , /*    */
	SEC_CTRL_SOTP_UNLOCK_SOTP_UNSEC, /*    */
	SEC_CTRL_SOTP_UNLOCK_SOTP_UNSEC_PROV, /*    */
	SEC_CTRL_SOTP_UNLOCK_SOTP_SEC, /*   */
	SEC_CTRL_SOTP_JTAG_UNLOCK, /*   */
	SEC_CTRL_KEY_GET,
	SEC_CTRL_KEY_CHAIN_RSA,
	SEC_CTRL_KEY_CHAIN_AES,
	SEC_CTRL_KEY_CHAIN_ENCKEY,
	SEC_CTRL_KEY_EXPORT_ITEM,
	SEC_CTRL_KEY_CLEAN_SEC_MEM,
	SEC_CTRL_KEY_CLEAN_ALL,
	SEC_CTRL_RNG_LOCK_ALL, /*default - everything is open  */
	SEC_CTRL_RNG_UNLOCK_RNG , /*    */
	SEC_CTRL_RNG_UNLOCK_RNG_UNSEC, /*    */
	SEC_CTRL_RNG_UNLOCK_RNG_SEC, /*   */
	SEC_CTRL_MAX
} bcm_sec_ctrl_t;

typedef struct _sec_ctrl_arg {
	/* an array with SEC_CTRL_RUN_ORDER_MAX 
 	*	represents order in which each corresponding
 	*	value is processed  
 	* 	*/
	bcm_sec_ctrl_t ctrl;
	void* ctrl_arg;
} bcm_sec_ctrl_arg_t;

typedef struct _bcm_sec_ bcm_sec_t;

#define SEC_CTRL_RUN_ORDER_MAX 10 
typedef struct _bcm_sec_cb_arg {
	bcm_sec_ctrl_arg_t arg[SEC_CTRL_RUN_ORDER_MAX];
} bcm_sec_cb_arg_t;

/*Handlers xxxx_cb*/
typedef struct _bcm_sec_ctrl_cb_ {
	int (*cb)(bcm_sec_t*, bcm_sec_ctrl_t, void *);
	bcm_sec_ctrl_arg_t arg[SEC_CTRL_RUN_ORDER_MAX];
} bcm_sec_ctrl_cb_t;

typedef enum _ctrl_args {
	SEC_CTRL_ARG_MAIN = 0,
	SEC_CTRL_ARG_KEY,
	SEC_CTRL_ARG_SOTP,
	SEC_CTRL_ARG_RNG,
	SEC_CTRL_ARG_MAX,
} bcm_sec_ctrl_arg_num_t;


typedef struct {
	u32 delg_id;
	u32 max_antirollback;
	u8* sec_policy_fit;
	u8 rsa_pub[BCM_SECBT_RSA2048_MOD_LEN];
	u8 aes_ek[BCM_SECBT_AES_CBC128_EK_LEN*2];
	u32 post_loader_entry_point;
} bcm_sec_delg_cfg;

struct _bcm_sec_ {
        bcm_sec_key_t key;
	bcm_sec_delg_cfg  * delg_cfg_obj;
        bcm_sec_state_t state;
        bcm_sec_ctx_t curr_ctx;
        bcm_sec_ctx_t sched_ctx;
	bcm_sec_ctrl_cb_t cb[SEC_CTRL_ARG_MAX]; 
	bcm_sec_ctrl_arg_num_t ord[SEC_CTRL_ARG_MAX]; 
}; 

int bcm_sec_set_sec_ser_num( char * ser_num, u32 ser_num_size);
int bcm_sec_get_sec_ser_num( char * ser_num, u32 ser_num_size);
int bcm_sec_set_dev_spec_key( char * dev_spec_key, u32 dev_spec_key_size);
int bcm_sec_get_dev_spec_key( char * dev_spec_key, u32 dev_spec_key_size);
int bcm_sec_get_antirollback_lvl( u32 * lvl);
int bcm_sec_set_antirollback_lvl( u32 lvl);
void bcm_sec_abort(void);
bcm_sec_t* bcm_sec(void);
bcm_sec_state_t bcm_sec_state(void);

/* returns ek + iv in the 'aes' pointer  */
void bcm_sec_get_active_aes_key(u8** key);
/* pass pointer to ek+iv*/
void bcm_sec_set_active_aes_key(u8* key);

u8* bcm_sec_get_active_pub_key(void);
u8* bcm_sec_set_active_pub_key(u8 * key);

u8* bcm_sec_get_root_pub_key(void);
void bcm_sec_get_root_aes_key(u8** key);
int bcm_sec_delg_process_sdr( u8 * psdr, u8 * hdr_end, u32 * sdr_plus_sig_size);
int bcm_sec_delg_process_sec_node(u8 * fit);
int bcm_sec_delg_post_process_sw_restrictions( u8 * fit_hdr );
bcm_sec_delg_cfg * bcm_sec_get_delg_cfg(void);

int bcm_sec_update_ctrl_arg(bcm_sec_ctrl_arg_t* k,
			bcm_sec_ctrl_arg_num_t ctrl);
void bcm_sec_clean_keys(bcm_sec_t* sec);
void bcm_sec_init(void);
int bcm_sec_do(bcm_sec_ctx_t, bcm_sec_cb_arg_t args[SEC_CTRL_ARG_MAX]); 
ulong bcm_sec_get_reqd_load_size( void * fit );
int bcm_sec_validate_fit(void*, uint32_t);
void bcm_sec_digest(const u8 *data, u32 len, u8* digest, char* algo);
int bcm_sec_rsa_verify(const u8 *obj,
                u32 obj_len, const u8* sig,
                u32 sig_len, const u8 *pub,
                struct image_sign_info *im );
void bcm_sec_deinit(void);
int bcm_sec_sec_fit(void* fit);
int bcm_sec_btrm_key_info(bcm_sec_t* sec);
void bcm_sec_aes_cbc128(u8 *key, u8 *iv, u8* txt, u32 length, u32 flag);

/* TODO:must be moved or replaced by common primitives*/
u8* bcm_util_env_var2bin(const char* id, u32 data_len );
u8* bcm_util_get_fdt_prop_data(void* fdt, char* path, char *prop, int* len);
int bcm_util_hex2u32(const char* s, u8*  d);
#endif
