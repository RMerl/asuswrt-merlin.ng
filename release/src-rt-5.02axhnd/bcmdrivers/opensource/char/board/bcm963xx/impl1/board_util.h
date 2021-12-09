/*
* <:copyright-BRCM:2016:DUAL/GPL:standard
* 
*    Copyright (c) 2016 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :> 
*/


#ifndef _BOARD_UTIL_H_
#define _BOARD_UTIL_H_

#include <bcm_hwdefs.h>

/* Typedefs. */
typedef struct
{
    unsigned long ulId;
    char chInUse;
    char chReserved[3];
} MAC_ADDR_INFO, *PMAC_ADDR_INFO;

typedef struct
{
    unsigned long ulNumMacAddrs;
    unsigned char ucaBaseMacAddr[NVRAM_MAC_ADDRESS_LEN];
    MAC_ADDR_INFO MacAddrs[1];
} MAC_INFO, *PMAC_INFO;

typedef struct
{
    unsigned char gponSerialNumber[NVRAM_GPON_SERIAL_NUMBER_LEN];
    unsigned char gponPassword[NVRAM_GPON_PASSWORD_LEN];
} GPON_INFO, *PGPON_INFO;

typedef struct
{
    unsigned int eventmask;
} BOARD_IOC, *PBOARD_IOC;

extern int (*bcm_sata_test_ioctl_fn)(void *);

void set_mac_info( void );
void set_gpon_info( void );
PMAC_INFO get_mac_info(void);

unsigned long getMemorySize(void);
void __init boardLedInit(void);
void boardLedCtrl(BOARD_LED_NAME, BOARD_LED_STATE);

BOARD_IOC* board_ioc_alloc(void);
void board_ioc_free(BOARD_IOC* board_ioc);
int ConfigCs (BOARD_IOCTL_PARMS *parms);

void stopOtherCpu(void);
void resetPwrmgmtDdrMips(void);

void kerSysInitMonitorSocket( void );
void __exit kerSysCleanupMonitorSocket( void );
void __init kerSysInitBatteryManagementUnit(void);
void kerSysSetMonitorTaskPid(int pid);
int board_ioctl_mem_access(BOARD_MEMACCESS_IOCTL_PARMS* parms, char* kbuf, int len);

void board_util_init(void);
void board_util_deinit(void);

#endif
