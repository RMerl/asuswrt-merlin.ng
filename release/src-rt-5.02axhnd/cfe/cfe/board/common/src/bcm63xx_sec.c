/*  *********************************************************************
    *
    <:copyright-BRCM:2017:proprietary:standard
    
       Copyright (c) 2017 Broadcom 
       All Rights Reserved
    
     This program is the proprietary software of Broadcom and/or its
     licensors, and may only be used, duplicated, modified or distributed pursuant
     to the terms and conditions of a separate, written license agreement executed
     between you and Broadcom (an "Authorized License").  Except as set forth in
     an Authorized License, Broadcom grants no license (express or implied), right
     to use, or waiver of any kind with respect to the Software, and Broadcom
     expressly reserves all rights in and to the Software and all intellectual
     property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
     NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
     BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
    
     Except as expressly set forth in the Authorized License,
    
     1. This program, including its structure, sequence and organization,
        constitutes the valuable trade secrets of Broadcom, and you shall use
        all reasonable efforts to protect the confidentiality thereof, and to
        use this information only in connection with your use of Broadcom
        integrated circuit products.
    
     2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
        AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
        WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
        RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
        ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
        FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
        COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
        TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
        PERFORMANCE OF THE SOFTWARE.
    
     3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
        ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
        INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
        WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
        IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
        OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
        SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
        SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
        LIMITED REMEDY.
    :> 
    ********************************************************************* */

#include "rom_main.h"
#include "rom_parms.h"
#include "bcm_otp.h"
#include "lib_byteorder.h"
#include "initdata.h"
#include "bcm63xx_storage_dev.h"
#include "bcm63xx_sec.h"

#ifdef CFG_RAMAPP
#include "cfe_iocb.h"
#include "cfe_devfuncs.h"
#endif

#if defined (SEC_TK)
#include "bcm63xx_keystore.h"
#endif

/* Types */
typedef enum _sec_obj_status {
        SEC_OBJ_STATUS_INACTIVE,
        SEC_OBJ_STATUS_ACTIVE_KEYSTORE_OFF,
        SEC_OBJ_STATUS_ACTIVE,
} sec_obj_status_t;

#ifdef CFG_RAMAPP
typedef enum _sec_console_status {
        SEC_CONSOLE_STATUS_SECURE,
        SEC_CONSOLE_STATUS_NONSECURE,
} sec_console_status_t;
#endif

typedef struct _boot_status_obj {
        boot_status_t boot_status;
#ifdef CFG_RAMAPP
        sec_console_status_t console_status;
#endif        
        sec_obj_status_t status;
/*#if (!defined(_BCM960333_) && !defined(_BCM96848_) && !defined(_BCM947189_) && \
     !defined(_BCM963381_)) || (defined(_BCM963381_) && \
     (INC_BTRM_BOOT==1))*/
/* Struct for capturing args passed down by CFE BTRM*/
        Booter1Args sec_args;
/*#endif*/
} boot_status_obj_t;

static boot_status_obj_t _obj;

/* Static function prototypes */
#if defined (SEC_TK)
extern cfe_sec_err_t cfe_sec_tk_implement(cfe_storage_dev_t* fs);
#endif

extern int sotp_sec89_both_readlocked(void);

#ifdef CFG_RAMAPP
extern void _cfe_flushcache(int, uint8_t *, uint8_t *);
#if defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined (_BCM96856_) || defined(_BCM96846_) 
static void cfe_sec_shred_credentials(void);
#endif
static inline void _query_sec_console_status(sec_console_status_t *con_status);
#endif

#if defined(_BCM94908_) || defined(CONFIG_BCM94908) || defined(_BCM96858_) || defined(CONFIG_BCM96858) || \
    defined(_BCM963158_) || defined(CONFIG_BCM963158) || \
    defined(_BCM96846_) || defined(CONFIG_BCM96846) || defined(_BCM96856_) || defined(CONFIG_BCM96856)
static int bcm_otp_get_boot_sec_state(unsigned int *sec_state)
{
    int rval = 0;
    unsigned int bcmBtrmEn, cusBtrmEn, cusMtkid;
    *sec_state = SEC_STATE_UNSEC;

    rval = bcm_otp_get_row(OTP_BRCM_BTRM_BOOT_ENABLE_ROW, &bcmBtrmEn);
    rval |= bcm_otp_get_row( OTP_CUST_BTRM_BOOT_ENABLE_ROW, &cusBtrmEn);
    rval |= bcm_otp_get_row( OTP_CUST_MFG_MRKTID_ROW, &cusMtkid);
    if (rval)  {
       return -1;
    }
    if ((bcmBtrmEn & OTP_BRCM_BTRM_BOOT_ENABLE_MASK) &&
          (cusBtrmEn & OTP_CUST_BTRM_BOOT_ENABLE_MASK)) {
       *sec_state = SEC_STATE_GEN3_MFG;
       if ( (cusMtkid & OTP_CUST_MFG_MRKTID_MASK) && sotp_sec89_both_readlocked()) {
           *sec_state = SEC_STATE_GEN3_FLD;
       }
    }
    return rval;
}

#else
static int bcm_otp_get_boot_sec_state(unsigned int *sec_state)
{
    *sec_state = SEC_STATE_UNSEC;
#if defined(_BCM963268_)

#if (INC_BTRM_BOOT==1)
   *sec_state = SEC_STATE_GEN2_MFG;
#endif

#elif defined(_BCM96838_)

   if (bcm_otp_is_btrm_boot()) {
      *sec_state = SEC_STATE_GEN2_BTRM;
      if (((*(uint32_t *)(OTP_BASE + OTP_SHADOW_ADDR_MARKET_ID_CUST_ROW)) & OTP_MFG_MRKTID_OTP_BITS_MASK)) {
        *sec_state = SEC_STATE_GEN2_MFG;
      }
   }

#else

    int rval = 0;
    unsigned int bcmBtrmEn, cusBtrmEn, cusMktid,custOid;

#if defined(_BTRM_DEVEL_)
    return ROM_ARG_ISSET(ROM_ARG_SEC_FLD_DEVEL);
#endif
    rval = bcm_otp_get_row(OTP_BRCM_BTRM_BOOT_ENABLE_ROW, &bcmBtrmEn);
    rval |= bcm_otp_get_row( OTP_CUST_BTRM_BOOT_ENABLE_ROW, &cusBtrmEn);
    rval |= bcm_otp_get_row( OTP_CUST_MFG_MRKTID_ROW, &cusMktid);
    rval |= bcm_otp_get_row( OTP_CUST_OP_MRKTID_ROW, &custOid);
    if (rval) {
        return -1;
    }
    if ((bcmBtrmEn & OTP_BRCM_BTRM_BOOT_ENABLE_MASK) &&
          (cusBtrmEn & OTP_CUST_BTRM_BOOT_ENABLE_MASK)) {
		if (cusMktid&OTP_CUST_MFG_MRKTID_MASK) {
                   *sec_state = (custOid & OTP_CUST_OP_MRKTID_MASK)? SEC_STATE_GEN2_OP:SEC_STATE_GEN2_MFG;
                } else {
                   *sec_state = SEC_STATE_GEN2_BTRM;
                }
    }
#endif
    return 0;
}
#endif


cfe_sec_err_t cfe_sec_get_boot_status(boot_status_t *info)
{
        if (_obj.status == SEC_OBJ_STATUS_INACTIVE) {
                return CFE_SEC_ERR_FAIL;
        }
        memcpy(info,&_obj.boot_status,sizeof(_obj.boot_status));
        return CFE_SEC_ERR_OK;
}

static inline cfe_sec_err_t _query_boot_status(boot_status_t *info)
{
        if (bcm_otp_get_boot_sec_state(&info->sec_state)) {
                return CFE_SEC_ERR_FAIL;
        }
        info->boot_mode = CFE_BOOT_XIP;
#if (BOARD_SEC_ARCH == SEC_ARCH_GEN2)
        if (info->sec_state != SEC_STATE_UNSEC) {
                info->boot_mode = CFE_BOOT_BTRM;
        }
#else
        info->boot_mode = CFE_BOOT_BTRM;
#endif
        info->sec_arch = BOARD_SEC_ARCH;
        return CFE_SEC_ERR_OK;
}

Booter1Args* cfe_sec_get_bootrom_args(void)
{
        if (_obj.status != SEC_OBJ_STATUS_ACTIVE) {
                return NULL;
        }
        return &_obj.sec_args;
}

cfe_sec_err_t cfe_sec_reset_keys(void)
{
        if (_obj.status != SEC_OBJ_STATUS_ACTIVE) {
                return CFE_SEC_ERR_FAIL;
        }
        ek_iv_cleanup(&_obj.sec_args.encrArgs);
        return CFE_SEC_ERR_OK;
}

cfe_sec_err_t cfe_sec_init(void)
{
        boot_status_t* info = &_obj.boot_status;

        if (_obj.status != SEC_OBJ_STATUS_INACTIVE) {
                goto err;
        }
        /*__asm __volatile ("1: b 1b");*/
        if (_query_boot_status(info) == CFE_SEC_ERR_FAIL) {
                goto err;
        }
        switch (info->sec_state) {
                case SEC_STATE_UNSEC:
#if (BOARD_SEC_ARCH == SEC_ARCH_GEN2)
                case SEC_STATE_GEN2_BTRM:
#endif
                        break;
#if (BOARD_SEC_ARCH == SEC_ARCH_GEN2)
                case SEC_STATE_GEN2_MFG:
                case SEC_STATE_GEN2_OP:
#else
                case SEC_STATE_GEN3_MFG:
                case SEC_STATE_GEN3_FLD:
#endif
#ifdef CFG_RAMAPP
                        CFE_RAM_ROM_PARMS_AUTH_PARM_GETM(&_obj.sec_args.authArgs);
#else
                        memcpy((void *)&_obj.sec_args, (void *)BTRM_INT_MEM_CREDENTIALS_ADDR, sizeof(Booter1Args));
#endif
                        break;
                default:
                        goto err;
        }
        _obj.status = SEC_OBJ_STATUS_ACTIVE;

#ifdef CFG_RAMAPP
        /* Initialize console status */
        _query_sec_console_status(&_obj.console_status);
#else
#if defined (SEC_TK)

        if (cfe_storage_dev_init()) {
                goto err;
        }
        if (cfe_sec_tk_implement(cfe_storage_dev_get())) {
                goto err;
        }
        PRINT4CH(SEND);
#endif
#endif /* !defined(CFG_RAMAPP) */

        return CFE_SEC_ERR_OK;
err:
        PRINT4CH(SERR);
        return CFE_SEC_ERR_FAIL;
}

#ifdef CFG_RAMAPP
#if defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined (_BCM96856_) || defined(_BCM96846_) 
static void cfe_sec_shred_credentials(void)
{
        char * pCredentials = NULL;
        int credential_size = 0;
        pCredentials = (char*)BTRM_INT_MEM_CREDENTIALS_ADDR; 
            
        credential_size = BTRM_INT_MEM_CREDENTIALS_ADDR_END - BTRM_INT_MEM_CREDENTIALS_ADDR;
        memset( pCredentials, 0, credential_size );
        cfe_flushcache(CFE_CACHE_FLUSH_D);
}
#endif /* Only for devices supporting Gen3+ secureboot */   

static inline void _query_sec_console_status(sec_console_status_t *con_status)
{
        int console_nonsec = 0;

#ifdef CONFIG_CFE_NONSEC_FLD_CONSOLE
        console_nonsec |= bcm_otp_is_boot_secure();
#endif            
#ifdef CONFIG_CFE_NONSEC_MFG_CONSOLE
        console_nonsec |= bcm_otp_is_boot_mfg_secure();
#endif       
        if( console_nonsec )
                *con_status = SEC_CONSOLE_STATUS_NONSECURE;
        else
                *con_status = SEC_CONSOLE_STATUS_SECURE;
}

/* Returns 1 if interactive console should be running in non-secure environment */
int cfe_sec_is_console_nonsec(void)
{
        return( _obj.console_status == SEC_CONSOLE_STATUS_NONSECURE );
}

/* Set privilege levels and cleanup credentials inorder to access console in non-secure state */
void cfe_sec_set_console_privLvl(void)
{
        /* drop to nonsecure and wipe credential SRAM if console is non-secure  */
        if (cfe_sec_is_console_nonsec()) 
        {
#if defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined (_BCM96856_) || defined(_BCM96846_) 
                /* Shred all credentials in SRAM */
                cfe_sec_shred_credentials();
                PRINT4CH(SHRD);
#endif   

#if defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || defined (_BCM96856_) || defined(_BCM96846_) || defined(_BCM63138_) || defined(_BCM63148_)
                /* Drop to non secure */
                cfe_nonsecure();
                PRINT4CH(NSCN);
#endif                
        }
}

/* Sets access permissions for the SOTP register block */
void cfe_sec_set_sotp_access_permissions(void)
{
#if defined(_BCM963138_) || defined(_BCM963148_) || \
    defined(_BCM94908_) || defined(_BCM96858_) || defined(_BCM963158_) || \
    defined(_BCM96846_) || defined(_BCM96856_) 
    int sotp_permissions = SOTP_PERM_ALLOW_SECURE_ACCESS;

#if !defined(BCM_OPTEE) 
#if defined(CONFIG_CFE_ALLOW_NONSEC_SOTP_ACCESS)
    sotp_permissions |=  SOTP_PERM_ALLOW_NONSEC_ACCESS;
#endif
#else 
    /* If OPTEE is being built SOTP can only be accessed via Trusted Client/APP */
#endif

    SOTP->sotp_perm = sotp_permissions;
#endif
}

#endif /* CFG_RAMAPP */
