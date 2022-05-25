
/*
    Copyright 2000-2018 Broadcom Corporation

    <:label-BRCM:2011:DUAL/GPL:standard
    
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

#include "bp_defs.h"
#include "boardparms.h"

WLAN_PCI_PATCH_INFO wlanPciInfo[]={
    {"",                 0, 0, {{"",       0,      0}}}, /* last entry*/
};

#if defined(_BCM963138_) || defined(CONFIG_BCM963138)
static WLAN_SROM_ENTRY wlan_patch_unique_963138REF[] = {
    {0, 0}
};

static WLAN_SROM_ENTRY wlan_patch_common_963138REF[] = {
    { 66,  0x200},
    { 68,  0x9800},
    { 87,  0x41f},
    { 96,  0x204A},
    { 97,  0xFE9B}, 
    { 98,  0x17D5},
    { 99,  0xFAAD},
    { 112, 0x204A},
    { 113, 0xFE74},
    { 114, 0x1646},
    { 115, 0xFAC2},
    { 162, 0x4444},
    { 170, 0x4444},
    { 172, 0x4444},
    {0, 0}
};
#endif

static WLAN_SROM_ENTRY wlan_patch_LAST[] = {
    {0, 0}
};
/* this data structure could be moved to boardparams structure in the future */
/* does not require to rebuild cfe here if more srom entries are needed */
WLAN_SROM_PATCH_INFO wlanPaInfo[]={
#if defined(_BCM963138_) || defined(CONFIG_BCM963138)
    {"963138REF_LTE",     0xa8d1, 220, wlan_patch_unique_963138REF,     wlan_patch_common_963138REF},
    {"963138LTE_P302",     0xa8d1, 220, wlan_patch_unique_963138REF,     wlan_patch_common_963138REF},
#endif
   
    {"", 0, 0, wlan_patch_LAST, wlan_patch_LAST}, /* last entry*/
};
