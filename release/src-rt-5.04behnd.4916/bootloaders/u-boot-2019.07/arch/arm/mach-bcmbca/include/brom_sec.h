/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2023 Broadcom Ltd.
 */

#ifndef _BROM_SEC_H
#define _BROM_SEC_H

typedef enum _bcm_sec_arch_ver_ { 
	SEC_ARCH_VER = 0,
	SEC_ARCH_VER1 = 0x2,
	SEC_ARCH_VER2 = 0x3,
}bcm_sec_arch_ver_t;

#if defined  (CONFIG_BRCM_SEC_GEN3V)
#define BCM_SECBT_RSA_MOD_MAX_LEN	BCM_SECBT_RSA4096_MOD_LEN
#define BCM_SECBT_AES_EK_MAX_LEN	BCM_SECBT_AES_CBC256_EK_LEN
#define	BCM_SECBT_CRED_AES_LEN		(bcm_sec_arch_ver() == SEC_ARCH_VER2? BCM_SECBT_AES_CBC256_EK_LEN : (bcm_sec_arch_ver() == SEC_ARCH_VER1? BCM_SECBT_AES_CBC192_EK_LEN : BCM_SECBT_AES_CBC128_EK_LEN))
#define	BCM_SECBT_CRED_MOD_LEN		(bcm_sec_arch_ver() == SEC_ARCH_VER2? BCM_SECBT_RSA4096_MOD_LEN : (bcm_sec_arch_ver() == SEC_ARCH_VER1? BCM_SECBT_RSA3072_MOD_LEN : BCM_SECBT_RSA2048_MOD_LEN))
#define AES_EXPAND_KEY_MAX_LEN		AES256_EXPAND_KEY_LENGTH
 
static inline bcm_sec_arch_ver_t bcm_sec_arch_ver(void)
{
	return ((*(volatile u32*)(CONFIG_SYS_SEC_CRED_ADDR+0x7fc))>>24);
}

struct bcm_secbt_auth_args {
	uint8_t	manu[BCM_SECBT_RSA_MOD_MAX_LEN] __attribute__ ((aligned (4)));
};

struct bcm_secbt_encr_args {
	uint8_t	bek[BCM_SECBT_AES_EK_MAX_LEN];
	uint8_t	biv[BCM_SECBT_AES_CBC_IV_LEN*2];
};

typedef struct __bcm_secbt_args{
	struct bcm_secbt_encr_args	encr;
	struct bcm_secbt_auth_args	auth __attribute__ ((aligned (4)));
} bcm_secbt_args_t;

#else

#define AES_EXPAND_KEY_MAX_LEN		AES128_EXPAND_KEY_LENGTH 
#define BCM_SECBT_AES_EK_MAX_LEN	BCM_SECBT_AES_CBC128_EK_LEN
#define BCM_SECBT_RSA_MOD_MAX_LEN	BCM_SECBT_RSA2048_MOD_LEN

#define	BCM_SECBT_CRED_AES_LEN		BCM_SECBT_AES_CBC128_EK_LEN
#define	BCM_SECBT_CRED_MOD_LEN		BCM_SECBT_RSA2048_MOD_LEN

struct bcm_secbt_auth_args {
        uint8_t manu[BCM_SECBT_RSA_MOD_MAX_LEN] __attribute__ ((aligned (4)));
        uint8_t oper[BCM_SECBT_RSA_MOD_MAX_LEN] __attribute__ ((aligned (4)));
};

struct bcm_secbt_encr_args {
        uint8_t bek[BCM_SECBT_AES_EK_MAX_LEN];
        uint8_t iek[BCM_SECBT_AES_EK_MAX_LEN];
        uint8_t biv[BCM_SECBT_AES_CBC_IV_LEN];
        uint8_t iiv[BCM_SECBT_AES_CBC_IV_LEN];
};

typedef struct __bcm_secbt_args{
        struct bcm_secbt_auth_args      auth __attribute__ ((aligned (4)));
        struct bcm_secbt_encr_args      encr;
} bcm_secbt_args_t;

#endif


#endif
