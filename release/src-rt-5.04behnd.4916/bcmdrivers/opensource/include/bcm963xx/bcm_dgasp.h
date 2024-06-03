/*
    Copyright 2007-2015 Broadcom Corporation

    <:label-BRCM:2015:DUAL/GPL:standard
    
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

/***********************************************************************/
/*                                                                     */
/*   MODULE:  bcm_dgasp.h                                              */
/*   PURPOSE: Dying gasp hw  information.  This module should include  */
/*            all base device addresses and board specific macros.     */
/*                                                                     */
/***********************************************************************/
#ifndef _BCM_DGASP_H
#define _BCM_DGASP_H

#ifdef __cplusplus
extern "C" {
#endif

//DGASP defines
typedef enum
{   
    DG_ENABLE = 0,
    DG_ENABLE_FORCE,            //Enable DG, overriding any checks which are preventing the enable
    DG_DISABLE,                             
    DG_DISABLE_PREVENT_ENABLE,  //Disable DG, DG can then only be enabled via DG_ENABLE_FORCE
} DGASP_ENABLE_OPTS;

typedef enum {
    DGASP_EVT_PWRDOWN,	// power down unnecessary blocks
    DGASP_EVT_SENDMSG,	// send the dying gasp message
} DGASP_EVENT;

extern void kerSysDisableDyingGaspInterrupt(void);
extern void kerSysEnableDyingGaspInterrupt(void);
extern void kerSysRegisterDyingGaspHandler(char *devname, void *cbfn, void *context);
extern void kerSysRegisterDyingGaspHandlerV2(char *devname, void *cbfn, void *context);
extern void kerSysDeregisterDyingGaspHandler(char *devname);
extern int kerSysIsDyingGaspTriggered(void);
void kerSysDyingGaspIoctl(DGASP_ENABLE_OPTS opt);
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM963146)
extern void kerSysDisableDyingGaspOverride(void);
extern void kerSysEnableDyingGaspOverride(void);
extern void kerSysGetDyingGaspConfig( unsigned int * afe_reg0, unsigned int * bg_bias0);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _BCM_DGASP_H */

