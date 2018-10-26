/* 
    Copyright 2000-2017 Broadcom Corporation

    <:label-BRCM:2017:DUAL/GPL:standard
    
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
#ifndef __BCM_SEC_H
#define __BCM_SEC_H

#define  SEC_ARCH_NONE 0
#define  SEC_ARCH_GEN1 1
#define  SEC_ARCH_GEN2 2
#define  SEC_ARCH_GEN3 3
typedef enum _boot_mode {
        CFE_BOOT_XIP=0,
        CFE_BOOT_BTRM,
} boot_mode_t;

typedef enum _cfe_sec_err {
        CFE_SEC_ERR_OK=0,
        CFE_SEC_ERR_FAIL,
} cfe_sec_err_t;

typedef enum _sec_state {
        SEC_STATE_UNSEC=0,
        SEC_STATE_GEN2_BTRM,
        SEC_STATE_GEN2_OP,
        SEC_STATE_GEN2_MFG,
        SEC_STATE_GEN3_MFG,
        SEC_STATE_GEN3_FLD
} sec_state_t;

typedef struct boot_status_ {
        sec_state_t sec_state;
        unsigned int sec_arch;
        boot_mode_t boot_mode;
} boot_status_t;
#endif
