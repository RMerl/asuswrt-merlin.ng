/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2020 Broadcom Ltd.
 */
#ifndef __KS_H_
#define __KS_H_
 
/*
 *
 *	KS - Key Store 
 *
 * */

#define KS_AES_256_CBC_SZ	AES256_KEY_LENGTH
#define KS_AES_192_CBC_SZ	AES192_KEY_LENGTH
#define KS_AES_128_CBC_SZ	AES128_KEY_LENGTH
#define KS_HASH_SZ		SHA256_SUM_LEN 
#define KS_MID_SZ		4 

#define KS_MAGIC		"BRCMKEYSTORE"
/*Adjusting for \0*/
#define KS_MAGIC_SIZE 		(sizeof(KS_MAGIC)-sizeof(char)) 
#define KS_MAGIC_LEN		ALIGN(KS_MAGIC_SIZE, 4)

#define KS_SIG_SIZE		RSA2048_BYTES
#define KS_SIG1_SIZE		RSA3072_BYTES
#define KS_SIG2_SIZE		RSA4096_BYTES
#define KS_DEF_ABORT_DELAY	1

#define	SEC_ARCH_NONE 0
#define	SEC_ARCH_GEN1 1
#define	SEC_ARCH_GEN2 2
#define	SEC_ARCH_GEN3 3

#define KS_MAX_KEYS 1 
#define KS_MAX_SZ ALIGN((sizeof(ks_t)+KS_MAX_KEYS*(sizeof(ks_key_info_t)+KS_AES_256_CBC_SZ*4+KS_SIG2_SIZE+KS_HASH_SZ*2+KS_MID_SZ*2)),4)


#if defined (CONFIG_ARM64)
#define KS_OFFSET  (u8*) _image_binary_end
#else
#define KS_OFFSET   __bss_end
#endif
#define KS_IS_FDT(_FDT_)  (fdt_magic(_FDT_) == FDT_MAGIC)
#define KS_FDT_SIZE(_FDT_)  (fdt_totalsize(_FDT_)) 

typedef enum _ks_err {
        KS_ERR_SUCC,
        KS_ERR_INVALID,
        KS_ERR_KEY_NOT_FOUND
} ks_err_t;

typedef enum _ks_req_state {
        KS_REQ_NONE=0x0,
        KS_REQ_TRANSIT_GEN2_BTRM,
        KS_REQ_TRANSIT_GEN2_MFG,
        KS_REQ_TRANSIT_GEN2_OP,
        KS_REQ_TRANSIT_MFG,
        KS_REQ_TRANSIT_FLD,
} ks_req_state_t;

typedef enum _ks_data_type {
        KS_DATA_TYPE_KEY_AES_CBC_128_EK=0x0,
        KS_DATA_TYPE_KEY_AES_CBC_IV=0x1,
        KS_DATA_TYPE_KEY_AES_CBC_256_EK=0x2,
        KS_DATA_TYPE_KEY_AES_CBC_192_EK=0x3,
        KS_DATA_TYPE_RSA_PUB=0x4,
        KS_DATA_TYPE_RSA_PRIV=0x5,
        KS_DATA_TYPE_HASH=0x6,
        KS_DATA_TYPE_MID=0x7,
        KS_DATA_TYPE_OID=0x8,
        KS_DATA_TYPE_BROM_MODE=0x9,
        KS_DATA_TYPE_CUST_BROM_MODE=0xa,
        KS_DATA_TYPE_DEV_KEY_128=0xb,
        KS_DATA_TYPE_DEV_KEY_256=0xc,
        KS_DATA_TYPE_MAX=0xd
} ks_data_type_t;

typedef enum _ks_data_state {
        KS_DATA_STATE_RAW=0x0,
        KS_DATA_STATE_MFG_ENCR=0x1,
        KS_DATA_STATE_MFG_OEM_ENCR=0x2,
        KS_DATA_STATE_FLD_ENCR=0x3,
        KS_DATA_STATE_FLD_OEM_ENCR=0x4,
        KS_DATA_STATE_GEN_RAND=0x5,
        KS_DATA_STATE_MAX=0x6
} ks_data_state_t;

typedef enum _ks_sec_ver {
        KS_ARCH_SEC_VER=0x0,
        KS_ARCH_SEC_VERv1=0x2,
        KS_ARCH_SEC_VERv2=0x3,
} ks_arch_sec_ver_t;

#define KS_STAT_MSK 0xff
#define KS_DATA_GET_TYPE(d) (d&KS_STAT_MSK)
#define KS_DATA_GET_STATE(d) ((d>>8)&KS_STAT_MSK)

struct __attribute__((packed)) ks_req_info {
        /*This what was requested by build*/
        /*ks_req_state_t*/
        u32 state;
        u32 abort_delay;
        /*sec_arch_info_t */
};

typedef struct  ks_req_info ks_req_info_t;

struct __attribute__((packed)) ks_key_info {
        u32 size;
        u32 type_state;
	/* must be multiple of u32*/
        u8 data[0];
};

typedef struct ks_key_info ks_key_info_t;

typedef struct __attribute__((packed)) ks_key {
        /*crc of the data, sig and the key_info*/
        u32 crc;
        u8 sig[KS_SIG_SIZE];
        ks_key_info_t info;
} ks_key_t;

/* Dual purpose:
1. In non-secure mode (verifying via otp) if not-requested to enter to SEC mode
   proceed to default cfe_rom path
   -if requiested an MFG_FLD proceed to MFG enter
   -if requiested an FLD proceed to FLD via MFG
2. If in secure MFG but requested MFG_FLD proceed to FLD
3. If in FLD mode proceed to secure boot  
*/
struct __attribute__((packed)) ks_hdr {
        u8 magic[KS_MAGIC_SIZE];
        u32 sec_arch;
        ks_req_info_t req_info;
        u32 info_size; 
        /*header crc */
        u32 crc; 
};

typedef struct ks_hdr ks_hdr_t; 

typedef struct __attribute__((packed)) ks {
        ks_hdr_t hdr;
        ks_key_t key;
} ks_t;

/*
        initializes and  verifies a key store
*/
ks_err_t ks_init(bcm_sec_state_t sec_state, 
                u32 sec_arch,
		const u8* pub,
		const u8* aes_cbc128);
ks_err_t ks_get_data_info(ks_data_type_t type, 
			ks_data_state_t state,
			void* data);
ks_err_t ks_get_req_info(ks_req_info_t* req_info);
ks_err_t ks_type2size(ks_data_type_t data_type, u32 *size);
ks_err_t ks_reset(void);




//#define DRY_RUN_FLD 1

#ifdef DRY_RUN_MFG
static int  __dbg_dr_mfg; 
static inline int dbg_is_dr_mfg(void) {return __dbg_dr_mfg; }
static inline void dbg_set_dr_mfg(void) {__dbg_dr_mfg = 1;}
#define dbg_dr_mfg	dbg_is_dr_mfg() 
#define dbg_dr_mfg_set	dbg_set_dr_mfg() 
#else
#define dbg_dr_mfg 0
#define dbg_dr_mfg_set
#endif

#ifdef DRY_RUN_FLD
static int  __dbg_dr_fld; 
static inline int dbg_is_dr_fld(void) { return __dbg_dr_fld; }
static inline void dbg_set_dr_fld(void) {__dbg_dr_fld = 1;}
#define dbg_dr_fld	dbg_is_dr_fld() 
#define dbg_dr_fld_set	dbg_set_dr_fld() 
#else
#define dbg_dr_fld  0 
#define dbg_dr_fld_set
#endif

#if defined(DRY_RUN_MFG) || defined(DRY_RUN_FLD) 
#define DRY_RUN
#endif

//#define DRY_RUN
#ifdef DRY_RUN
#define BRKPT	do {						\
			*((volatile u32*)0xff800600)=0x6;	\
			asm("1: b 1b"); }while(0)
#else
#define BRKPT
#endif

#define dbg_dr (dbg_dr_fld || dbg_dr_mfg)

#endif
