/* 
    Copyright 2000-2019 Broadcom Corporation

    <:label-BRCM:2019:DUAL/GPL:standard
    
    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:
    
       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.
    
    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.
    
    :>

*/

/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *
    *  File: bcm63xx_boot_state.c
    *
    *  Maintain boot state and error information across soft reset 
    *
    *
*/
#include "bcm63xx_common.h"
#include "bcm_hwdefs.h"
#include "bcm_auth_if.h"
#include "bcm_encr_if.h"
#include "bcm63xx_boot.h"
#include "bcm63xx_ipc.h"
#include "bcm63xx_boot_state.h"
#include "bcm_sec_common.h"
#include "rom_parms.h"
#include "rom_main.h"

extern void die(void);
static void __boot_st_set(cfe_boot_state_info_type_t _type, 
                      unsigned int state);
static unsigned int __boot_st_get(cfe_boot_state_info_type_t _type);
/*#define _DBG_BOOT_STATE*/
#ifdef _DBG_BOOT_STATE
static inline void __trace_state(void)
{
    enum {
          _STATE_=0,
          _ERROR_,
          _INFO_,
          _MAX_INF_
    };
    static char _nm[_MAX_INF_][16][64] ={
                      {
                        {"BOOT_NONE"},
                        {"BOOT_UNSECURE"},
                        {"BOOT_SECURE"},
                        {"BOOT_SAFEMODE"},
                        {"BOOT_HALT"} 
                      }, 
                      {
                        {"BOOT_ERR_OK"},
                        {"BOOT_ERR_ABORTED"},
                        {"BOOT_ERR_CRIT"}
                      },
                      {
                        {"BOOT_INFO_NONE"},
                        { "BOOT_INFO_ROM"},
                        { "BOOT_INFO_PRIMARY"},
                        { ""}, 
                        {"BOOT_INFO_SECONDARY"},
                        {""},{""},{""},
                        {"BOOT_INFO_LINUX"}
                      } 
    };

    int err_info, st_info, st;
    st = __boot_st_get(CFE_BOOT_STATE_TYPE_ST);
    err_info = __boot_st_get(CFE_BOOT_STATE_TYPE_ERR);
    st_info = __boot_st_get(CFE_BOOT_STATE_TYPE_INFO);
    xprintf("%s MBOX 0x%x MBOX1 0x%x \n",__FUNCTION__,BCM_MBOX,BCM_MBOX1);
    if (st == CFE_BOOT_STATE_ERR_INVALID) {
        st = CFE_BOOT_NONE;
        xprintf("%s INVALID STATE\n",__FUNCTION__);
    }
    if (err_info == CFE_BOOT_STATE_ERR_INVALID) {
        err_info = CFE_BOOT_NONE; 
        xprintf("%s INVALID ERR\n",__FUNCTION__);
    }
    if (st_info == CFE_BOOT_STATE_ERR_INVALID) {
        st_info = CFE_BOOT_NONE; 
        xprintf("%s INVALID STATE INFO\n",__FUNCTION__);
    }
    xprintf("%s STATE: %s ERROR: %s ",__FUNCTION__,_nm[_STATE_][st], _nm[_ERROR_][err_info]);
    if (st_info&CFE_BOOT_INFO_PRIMARY) {
         xprintf(" : %s ",_nm[_INFO_][st_info&CFE_BOOT_INFO_PRIMARY]);
    }
    if (st_info&CFE_BOOT_INFO_SECONDARY) {
         xprintf(" : %s ", _nm[_INFO_][st_info&CFE_BOOT_INFO_SECONDARY]);
    }
    if (st_info&CFE_BOOT_INFO_LINUX) {
         xprintf(" : %s ", _nm[_INFO_][st_info&CFE_BOOT_INFO_LINUX]);
    }
    xprintf("\n");
}

#define _tr  \
     __trace_state()
#else
#define _tr
#endif
void cfe_boot_st_wd(unsigned int wd_cmd, unsigned int delay)
{
        switch (wd_cmd) {
            case BCM_CMN_WD_SET:
                bcm_cmn_wd_set(delay, 1);
                break; 
            case BCM_CMN_WD_DIS:
                bcm_cmn_wd_set(delay, 0);
                break;
            case BCM_CMN_WD_RST:
                bcm_cmn_wd_reset();
                break;
            default:
                break;
       }
}

static void __boot_st_set(cfe_boot_state_info_type_t _type, 
                      unsigned int val)
{
    unsigned int st = BCM_MBOX_MSG1_GET();
    /**/
    if ((_type & CFE_BOOT_STATE_TYPE_ERR) == CFE_BOOT_STATE_TYPE_ERR) {
        BCM_BLR_BOOT_STATE_SET_ERR(st, val);
    }
    if ((_type & CFE_BOOT_STATE_TYPE_ST) == CFE_BOOT_STATE_TYPE_ST) {
        BCM_BLR_BOOT_STATE_SET_ST(st, val);
    }
    if ((_type & CFE_BOOT_STATE_TYPE_INFO) == CFE_BOOT_STATE_TYPE_INFO) {
        BCM_BLR_BOOT_STATE_SET_INFO(st, val);
    } 
    if (_type ) {
        BCM_MBOX_MSG1_SET(st);
    }
    _tr; 
}


static unsigned int __boot_st_get(cfe_boot_state_info_type_t _type)
{
    unsigned int  st  = BCM_MBOX_MSG1_GET(), rval = CFE_BOOT_STATE_ERR_INVALID;
    if ((_type & CFE_BOOT_STATE_TYPE_ERR) == CFE_BOOT_STATE_TYPE_ERR) {
       rval = BCM_BLR_BOOT_STATE_GET_ERR(st);
    } else if ((_type & CFE_BOOT_STATE_TYPE_ST) == CFE_BOOT_STATE_TYPE_ST) {
        rval = BCM_BLR_BOOT_STATE_GET_ST(st);
    } else if ((_type & CFE_BOOT_STATE_TYPE_INFO) == CFE_BOOT_STATE_TYPE_INFO) {
        rval = BCM_BLR_BOOT_STATE_GET_INFO(st);
    } 
    return rval;
}

static cfe_boot_state_err_t  __mbox_valid(void)
{
    int err_info, st_info, st;
    if (!BCM_MBOX1_RESET() && !BCM_MBOX1_STATUS()) {
        return CFE_BOOT_STATE_ERR_DIS;
    }
    st = __boot_st_get(CFE_BOOT_STATE_TYPE_ST);
    err_info = __boot_st_get(CFE_BOOT_STATE_TYPE_ERR);
    st_info = __boot_st_get(CFE_BOOT_STATE_TYPE_INFO);
    switch(st_info) {
        case CFE_BOOT_INFO_NONE:
        case CFE_BOOT_INFO_ROM:
        case CFE_BOOT_INFO_PRIMARY:
        case CFE_BOOT_INFO_SECONDARY:
        case (CFE_BOOT_INFO_PRIMARY|CFE_BOOT_INFO_LINUX):
        case (CFE_BOOT_INFO_SECONDARY|CFE_BOOT_INFO_LINUX):
            break; 
        default:
            goto err;
    }
    if (err_info < 0 || err_info > CFE_BOOT_ERR_CRIT || 
           st < 0 || st > CFE_BOOT_HALT) {   
       goto err;
    } 
    _tr; 
    return CFE_BOOT_STATE_ERR_OK;
err:
    _tr; 
    return CFE_BOOT_STATE_ERR_INVALID; 
}

#ifndef CFG_RAMAPP
extern  unsigned long rom_option;
static inline void __on_crit(cfe_boot_st_info_t st_info)
{
    __boot_st_set(CFE_BOOT_STATE_TYPE_ST, CFE_BOOT_HALT);
}
 
static inline void __on_abort(cfe_boot_st_info_t st_info) 
{
    __boot_st_set(CFE_BOOT_STATE_TYPE_ERR, CFE_BOOT_ERR_CRIT);

    if ((st_info & CFE_BOOT_INFO_ROM) == CFE_BOOT_INFO_ROM) {
        /* must try safemode */ 
        __boot_st_set(CFE_BOOT_STATE_TYPE_ST, CFE_BOOT_SAFEMODE);
        rom_option |= MCB_SEL_SAFEMODE;
    } else  if ((st_info & CFE_BOOT_INFO_PRIMARY)) {
                 __boot_st_set(CFE_BOOT_STATE_TYPE_INFO, 
                     (st_info&CFE_BOOT_INFO_LINUX)?
                          (CFE_BOOT_INFO_LINUX|CFE_BOOT_INFO_SECONDARY): CFE_BOOT_INFO_SECONDARY);
                 rom_option |= NAND_IMAGESEL_OVERRIDE;
    }
    _tr; 
}

static inline cfe_boot_st_err_info_t __on_error(cfe_boot_st_info_t st_info, 
                           cfe_boot_st_err_info_t err_info)
{
    switch (err_info) {
        case CFE_BOOT_ERR_CRIT:
            __on_crit(st_info);
            _tr;
            board_setleds(0x48414c54);  /* HALT */
#if defined(BOARD_SEC_ARCH)
            die();
#else
            cfe_launch(0);
#endif
            break;
        case CFE_BOOT_ERR_ABORTED:
            /* stop tracking rom at this point by clearing the error*/
            __on_abort(st_info);
	    break;
        case CFE_BOOT_ERR_OK:
        default:
            /* start tracking primary/secondary cferam even though it is still in cferom path while entering 
               to nand/emmc/nor boot */
            break;
    }
    return err_info;
}
#endif

cfe_boot_state_err_t cfe_boot_st_init(void)
{
    cfe_boot_state_err_t  rc = CFE_BOOT_STATE_ERR_OK;
    rc = __mbox_valid();
    if (rc) {
       /* do nothing message box is not on */
        goto err;
    }  
#ifndef CFG_RAMAPP
    {
        cfe_boot_st_t  st = __boot_st_get(CFE_BOOT_STATE_TYPE_ST);
        unsigned int err_info = __boot_st_get(CFE_BOOT_STATE_TYPE_ERR);
        if (st == CFE_BOOT_NONE || 
            err_info == CFE_BOOT_ERR_OK) {
#if defined(BOARD_SEC_ARCH)
            __boot_st_set(CFE_BOOT_STATE_TYPE_ST, 
                      cfe_sec_get_state() == SEC_STATE_UNSEC? CFE_BOOT_UNSECURE : CFE_BOOT_SECURE);
#else
            __boot_st_set(CFE_BOOT_STATE_TYPE_ST, SEC_STATE_UNSEC);
#endif
            /* Start tracking cferom*/
            __boot_st_set(CFE_BOOT_STATE_TYPE_INFO, CFE_BOOT_INFO_ROM);
            __boot_st_set(CFE_BOOT_STATE_TYPE_ERR, CFE_BOOT_ERR_ABORTED);
        } else {
                cfe_boot_st_info_t st_info = __boot_st_get(CFE_BOOT_STATE_TYPE_INFO);
                rc = __on_error(st_info, err_info);
        }
    }
#endif
    _tr;
    cfe_boot_st_wd(BCM_CMN_WD_SET, CONFIG_CFE_FAILSAFE_BOOT_WD_TMO);
err:
    return rc;
}

unsigned int cfe_boot_st_error(void)
{
    if (!BCM_MBOX1_STATUS()) {
       /* do nothing message box is not on */
       return CFE_BOOT_INFO_NONE;
    }  
    return __boot_st_get(CFE_BOOT_STATE_TYPE_ERR);
}

cfe_boot_st_info_t cfe_boot_st_track(void)
{
    cfe_boot_st_info_t inf;
    if (!BCM_MBOX1_STATUS()) {
       /* do nothing message box is not on */
       return CFE_BOOT_INFO_NONE;
    }  
    inf = __boot_st_get(CFE_BOOT_STATE_TYPE_INFO);
#ifndef CFG_RAMAPP
    if ((inf & CFE_BOOT_INFO_ROM)) {
        inf = CFE_BOOT_INFO_PRIMARY;
    }
#else
    if ((inf & (CFE_BOOT_INFO_PRIMARY | CFE_BOOT_INFO_SECONDARY)) && 
        !(inf &CFE_BOOT_INFO_LINUX)) {
        inf |= CFE_BOOT_INFO_LINUX;
    }
    cfe_boot_st_wd(BCM_CMN_WD_SET, CONFIG_CFE_FAILSAFE_BOOT_WD_TMO);
#endif
    __boot_st_set(CFE_BOOT_STATE_TYPE_INFO, inf);
    _tr;
    return inf;
}

void cfe_boot_st_complete(void)
{
    cfe_boot_st_wd(BCM_CMN_WD_DIS, 0);
    __boot_st_set(CFE_BOOT_STATE_TYPE_ERR, CFE_BOOT_ERR_OK);
}
