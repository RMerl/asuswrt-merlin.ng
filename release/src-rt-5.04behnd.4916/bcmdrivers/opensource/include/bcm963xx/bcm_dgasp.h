/*
    Copyright 2007-2015 Broadcom Corporation

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
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

