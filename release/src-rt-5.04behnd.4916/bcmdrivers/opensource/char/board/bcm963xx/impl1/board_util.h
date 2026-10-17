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

#include <gpon_defs.h>

#define NVRAM_MAC_COUNT_MAX         64

#define NVRAM_ULVERSION                  "ulVersion"
#define NVRAM_SZBOOTLINE                 "szBootline"
#define NVRAM_SZBOARDID                  "boardid"
#define NVRAM_ULMAINTPNUM                "ulMainTpNum"
#define NVRAM_ULPSISIZE                  "ulPsiSize"
#define NVRAM_ULNUMMACADDRS              "nummacaddrs"
#define NVRAM_UCABASEMACADDR             "ethaddr"
#define NVRAM_PAD                        "pad"
#define NVRAM_BACKUPPSI                  "backupPsi"
#define NVRAM_ULCHECKSUMV4               "ulCheckSumV4"
#define NVRAM_GPONSERIALNUMBER           "gponsn"
#define NVRAM_GPONPASSWORD               "gponpswd"
#define NVRAM_WPSDEVICEPIN               "wpsDevicePin"
#define NVRAM_WLANPARAMS                 "wlanParams"
#define NVRAM_WLFEATURE                  "wlFeature"
#define NVRAM_ULSYSLOGSIZE               "ulSyslogSize"
#define NVRAM_ULNANDPARTOFSKB            "ulNandPartOfsKb"
#define NVRAM_ULNANDPARTSIZEKB           "ulNandPartSizeKb"
#define NVRAM_SZVOICEBOARDID             "voiceboardid"
#define NVRAM_AFEID                      "afeId"
#define NVRAM_OPTICRXPWRREADING          "opticRxPwrReading"
#define NVRAM_OPTICRXPWROFFSET           "opticRxPwrOffset"
#define NVRAM_OPTICTXPWRREADING          "opticTxPwrReading"
#define NVRAM_UCUNUSED2                  "ucUnused2"
#define NVRAM_UCFLASHBLKSIZE             "ucFlashBlkSize"
#define NVRAM_UCAUXFSPERCENT             "ucAuxFSPercent"
#define NVRAM_UCUNUSED3                  "ucUnused3"
#define NVRAM_ULBOARDSTUFFOPTION         "ulBoardStuffOption"
#define NVRAM_ALLOCS                     "allocs"
#define NVRAM_ULMEMORYCONFIG             "ulMemoryConfig"
#define NVRAM_PART_INFO                  "part_info"
#define NVRAM_ALLOC_DHD                  "alloc_dhd"
#define NVRAM_ULFEATURES                 "ulFeatures"
#define NVRAM_CHUNUSED                   "chUnused"
#define NVRAM_ULCHECKSUM                 "ulCheckSum"

#define NAND_RFS_OFS_NAME           "NAND_RFS_OFS"

#define  WLAN_FEATURE_DHD_NIC_ENABLE     0x01
#define  WLAN_FEATURE_DHD_MFG_ENABLE     0x02
#define  WLAN_FEATURE_LAST_FEATURE       0x80

#define  WLAN_MFG_PARTITION_ISNAND       0x01
#define  WLAN_MFG_PARTITION_MFGSET       0x02
#define  WLAN_MFG_PARTITION_HASSIZE      0x04
#define  WLAN_BOOTMODE_IS_UBOOT          0x08
#define  WLAN_MFG_PARTITION_NAME         "misc3"
#define  WLAN_MFG_PARTITION_INDEX         2


#define DEFAULT_PSI_SIZE    128
#define DEFAULT_GPON_SN     "BRCM12345678"
#define DEFAULT_GPON_PW     "          "
#define DEFAULT_LOG_SIZE    0
#define DEFAULT_FLASHBLK_SIZE  64
#define MAX_FLASHBLK_SIZE      128
#define DEFAULT_AUXFS_PERCENT 0
#define MAX_AUXFS_PERCENT   80
#define MAX_NOR_AUXFS_SIZE  255
#define NOR_AUFS_SIZE_UNIT (4*ONEK)
#define DEFAUT_BACKUP_PSI  0

#define DEFAULT_WPS_DEVICE_PIN     "12345670"
#define DEFAULT_WLAN_DEVICE_FEATURE  0

#define DEFAULT_VOICE_BOARD_ID     "NONE"


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
    unsigned char gponPassword[NVRAM_XGPON_PASSWORD_LEN];
} GPON_INFO, *PGPON_INFO;

typedef struct
{
    unsigned int eventmask;
} BOARD_IOC, *PBOARD_IOC;

void set_mac_info( void );
void set_gpon_info( void );
PMAC_INFO get_mac_info(void);

void __init boardLedInit(void);
void boardLedCtrl(BOARD_LED_NAME, BOARD_LED_STATE);

BOARD_IOC* board_ioc_alloc(void);
void board_ioc_free(BOARD_IOC* board_ioc);
int ConfigCs (BOARD_IOCTL_PARMS *parms);

void stopOtherCpu(void);
void resetPwrmgmtDdrMips(void);

void kerSysInitMonitorSocket( void );
void __exit kerSysCleanupMonitorSocket( void );
void kerSysSetMonitorTaskPid(int pid);
int board_ioctl_mem_access(BOARD_MEMACCESS_IOCTL_PARMS* parms, char* kbuf, int len);

void board_util_init(void);
void board_util_deinit(void);

#endif
