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

/***************************************************************************
 * File Name  : bcm_nvram.h
 *
 * Description: This file contains the definitions and structures for the
 *              Broadcom kernel NVRAM interface
 *
 ***************************************************************************/

#ifndef __BCM_NVRAM_H__
#define __BCM_NVRAM_H__

/* Header file for kernel nvram module */

/* Search the given kernel nvram parameter (name) and returns the value */
extern char *nvram_k_get(char *name);

/* Sets the given value to the kernel nvram parameter (name) */
/* Returns 0 on success, else -ve value on failure */
extern int nvram_k_set(char *name, char *value);

#endif /* __BCM_NVRAM_H__ */
