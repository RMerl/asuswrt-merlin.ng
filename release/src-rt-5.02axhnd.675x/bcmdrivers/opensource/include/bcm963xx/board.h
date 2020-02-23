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
/*   MODULE:  board.h                                                  */
/*   PURPOSE: Board specific information.  This module should include  */
/*            all base device addresses and board specific macros.     */
/*                                                                     */
/***********************************************************************/
#ifndef _BOARD_H
#define _BOARD_H

#include "bcm_hwdefs.h"
#include "bcmTag.h"
#include "boardparms.h"

#ifdef __cplusplus
extern "C" {
#endif


/* BOARD_H_API_VER increases when other modules (such as PHY) depend on */
/* a new function in the board driver or in boardparms.h                */

#define BOARD_H_API_VER 15

/*****************************************************************************/
/*          board ioctl calls for flash, led and some other utilities        */
/*****************************************************************************/
/* Defines. for board driver */
#define BOARD_IOCTL_MAGIC       'B'

/** Device filename of ioctl device. */
#define BOARD_DEVICE_NAME  "/dev/brcmboard"

#define BOARD_IOCTL_FLASH_WRITE                 _IOWR(BOARD_IOCTL_MAGIC, 0, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_FLASH_READ                  _IOWR(BOARD_IOCTL_MAGIC, 1, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_DUMP_ADDR                   _IOWR(BOARD_IOCTL_MAGIC, 2, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SET_MEMORY                  _IOWR(BOARD_IOCTL_MAGIC, 3, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_MIPS_SOFT_RESET             _IOWR(BOARD_IOCTL_MAGIC, 4, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_LED_CTRL                    _IOWR(BOARD_IOCTL_MAGIC, 5, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_PSI_SIZE                _IOWR(BOARD_IOCTL_MAGIC, 6, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_SDRAM_SIZE              _IOWR(BOARD_IOCTL_MAGIC, 7, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_ID                      _IOWR(BOARD_IOCTL_MAGIC, 8, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_CHIP_ID                 _IOWR(BOARD_IOCTL_MAGIC, 9, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_CHIP_REV                _IOWR(BOARD_IOCTL_MAGIC, 10, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_CFE_VER                 _IOWR(BOARD_IOCTL_MAGIC, 11, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_BASE_MAC_ADDRESS        _IOWR(BOARD_IOCTL_MAGIC, 12, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_MAC_ADDRESS             _IOWR(BOARD_IOCTL_MAGIC, 13, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_RELEASE_MAC_ADDRESS         _IOWR(BOARD_IOCTL_MAGIC, 14, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_NUM_ENET_MACS           _IOWR(BOARD_IOCTL_MAGIC, 15, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_NUM_ENET_PORTS          _IOWR(BOARD_IOCTL_MAGIC, 16, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SET_MONITOR_FD              _IOWR(BOARD_IOCTL_MAGIC, 17, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_WAKEUP_MONITOR_TASK         _IOWR(BOARD_IOCTL_MAGIC, 18, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SET_TRIGGER_EVENT           _IOWR(BOARD_IOCTL_MAGIC, 19, BOARD_IOCTL_PARMS)        
#define BOARD_IOCTL_GET_TRIGGER_EVENT           _IOWR(BOARD_IOCTL_MAGIC, 20, BOARD_IOCTL_PARMS)        
#define BOARD_IOCTL_UNSET_TRIGGER_EVENT         _IOWR(BOARD_IOCTL_MAGIC, 21, BOARD_IOCTL_PARMS) 
#define BOARD_IOCTL_GET_WLAN_ANT_INUSE          _IOWR(BOARD_IOCTL_MAGIC, 22, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SET_SES_LED                 _IOWR(BOARD_IOCTL_MAGIC, 23, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SET_CS_PAR                  _IOWR(BOARD_IOCTL_MAGIC, 25, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SET_GPIO                    _IOWR(BOARD_IOCTL_MAGIC, 26, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_FLASH_LIST                  _IOWR(BOARD_IOCTL_MAGIC, 27, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_BACKUP_PSI_SIZE         _IOWR(BOARD_IOCTL_MAGIC, 28, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_SYSLOG_SIZE             _IOWR(BOARD_IOCTL_MAGIC, 29, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SET_SHUTDOWN_MODE           _IOWR(BOARD_IOCTL_MAGIC, 30, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SET_STANDBY_TIMER           _IOWR(BOARD_IOCTL_MAGIC, 31, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_BOOT_IMAGE_OPERATION        _IOWR(BOARD_IOCTL_MAGIC, 32, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_TIMEMS                  _IOWR(BOARD_IOCTL_MAGIC, 33, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_ALLOC_MAC_ADDRESSES         _IOWR(BOARD_IOCTL_MAGIC, 36, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_RELEASE_MAC_ADDRESSES       _IOWR(BOARD_IOCTL_MAGIC, 37, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SPI_SLAVE_INIT              _IOWR(BOARD_IOCTL_MAGIC, 38, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SPI_SLAVE_READ              _IOWR(BOARD_IOCTL_MAGIC, 39, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SPI_SLAVE_WRITE             _IOWR(BOARD_IOCTL_MAGIC, 40, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_NUM_FE_PORTS            _IOWR(BOARD_IOCTL_MAGIC, 41, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_NUM_GE_PORTS            _IOWR(BOARD_IOCTL_MAGIC, 42, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_NUM_VOIP_PORTS          _IOWR(BOARD_IOCTL_MAGIC, 43, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_SWITCH_PORT_MAP         _IOWR(BOARD_IOCTL_MAGIC, 44, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_PORT_MAC_TYPE           _IOWR(BOARD_IOCTL_MAGIC, 45, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_EPON_GPIOS              _IOWR(BOARD_IOCTL_MAGIC, 46, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SPI_SLAVE_SET_BITS          _IOWR(BOARD_IOCTL_MAGIC, 47, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SPI_SLAVE_CLEAR_BITS        _IOWR(BOARD_IOCTL_MAGIC, 48, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SPI_SLAVE_WRITE_BUF         _IOWR(BOARD_IOCTL_MAGIC, 49, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_GPIOVERLAYS             _IOWR(BOARD_IOCTL_MAGIC, 50, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_BATTERY_ENABLE          _IOWR(BOARD_IOCTL_MAGIC, 51, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_FTTDP_DSP_BOOTER            _IOWR(BOARD_IOCTL_MAGIC, 52, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_MEM_ACCESS                  _IOWR(BOARD_IOCTL_MAGIC, 53, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SET_DYING_GASP_INTERRUPT    _IOWR(BOARD_IOCTL_MAGIC, 54, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_BTRM_BOOT               _IOWR(BOARD_IOCTL_MAGIC, 55, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_BOOT_SECURE             _IOWR(BOARD_IOCTL_MAGIC, 56, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_SATA_TEST                   _IOWR(BOARD_IOCTL_MAGIC, 57, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_BT_GPIO                     _IOWR(BOARD_IOCTL_MAGIC, 58, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_BOOT_MFG_SECURE         _IOWR(BOARD_IOCTL_MAGIC, 59, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_FLASH_TYPE              _IOWR(BOARD_IOCTL_MAGIC, 60, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_SEQUENCE_NUMBER         _IOWR(BOARD_IOCTL_MAGIC, 61, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GETSET_BOOT_INACTIVE_IMAGE  _IOWR(BOARD_IOCTL_MAGIC, 62, BOARD_IOCTL_PARMS)
#define BOARD_IOCTL_GET_GPIO                    _IOWR(BOARD_IOCTL_MAGIC, 63, BOARD_IOCTL_PARMS)
# define BOARD_IOCTL_BT_GPIO_RESET 1
# define BOARD_IOCTL_BT_GPIO_WAKE 2

// for the action in BOARD_IOCTL_PARMS for flash operation
typedef enum 
{
    PERSISTENT,
    NVRAM,
    BCM_IMAGE_CFE,
    BCM_IMAGE_FS,
    BCM_IMAGE_KERNEL,
    BCM_IMAGE_WHOLE,
    SCRATCH_PAD,
    FLASH_SIZE,
    SET_CS_PARAM,
    BACKUP_PSI,
    SYSLOG,
    ENVRAM
} BOARD_IOCTL_ACTION;
    
typedef struct boardIoctParms
{
    union {
	    char *string;
	    char *value;
	};
    union {
      char *buf;
      char *param;
    };
    union {
        int strLen;
        int value_length;
    };
    int offset;
    BOARD_IOCTL_ACTION  action;        /* flash read/write: nvram, persistent, bcm image */
    int result;
} BOARD_IOCTL_PARMS;

#define BOARD_MEMACCESS_IOCTL_OP_READ  0
#define BOARD_MEMACCESS_IOCTL_OP_WRITE  1
#define BOARD_MEMACCESS_IOCTL_OP_FILL  2
#define BOARD_MEMACCESS_IOCTL_SPACE_REG  0
#define BOARD_MEMACCESS_IOCTL_SPACE_KERN  1
#define BOARD_MEMACCESS_IOCTL_SPACE_MEM  2

typedef struct boardMemaccessIoctlParms
{
    int op;
    int space;
    unsigned long long address;
    int size; // 1, 2, or 4
    int count; // number of items
    char *buf;
} BOARD_MEMACCESS_IOCTL_PARMS;

//DGASP defines
typedef enum
{   
    DG_ENABLE = 0,
    DG_ENABLE_FORCE,            //Enable DG, overriding any checks which are preventing the enable
    DG_DISABLE,                             
    DG_DISABLE_PREVENT_ENABLE,  //Disable DG, DG can then only be enabled via DG_ENABLE_FORCE
} DGASP_ENABLE_OPTS;

// LED defines 
typedef enum
{   
    kLedAdsl,
    kLedSecAdsl,
    kLedWanData,
    kLedSes,
    kLedVoip,
    kLedVoip1,
    kLedVoip2,
    kLedPots,
    kLedDect,
    kLedGpon,
    kLedMoCA,
    kLedEth0Duplex,
    kLedEth0Spd100,
    kLedEth0Spd1000,
    kLedEth1Duplex,
    kLedEth1Spd100,
    kLedEth1Spd1000,
#if defined(BCM_PON) || defined(CONFIG_BCM_PON) || defined(CONFIG_BCM963158)
    kLedOpticalLink,
    kLedUSB,
    kLedSim,
    kLedSimITMS,
    kLedEpon,
#endif
    kLedWL0,
    kLedWL1,
    kLedEnd                             // NOTE: Insert the new led name before this one.
} BOARD_LED_NAME;

typedef enum
{
    kLedStateOff,                        /* turn led off */
    kLedStateOn,                         /* turn led on */
    kLedStateFail,                       /* turn led on red */
    kLedStateSlowBlinkContinues,         /* slow blink continues at 2HZ interval */
    kLedStateFastBlinkContinues,         /* fast blink continues at 4HZ interval */
    kLedStateUserWpsInProgress,          /* 200ms on, 100ms off */
    kLedStateUserWpsError,               /* 100ms on, 100ms off */
    kLedStateUserWpsSessionOverLap       /* 100ms on, 100ms off, 5 times, off for 500ms */                     
} BOARD_LED_STATE;

typedef void (*HANDLE_LED_FUNC)(BOARD_LED_STATE ledState);

typedef enum
{
    kGpioInactive,
    kGpioActive
} GPIO_STATE_t;

// FLASH_ADDR_INFO is now defined in flash_common.h
#include "flash_common.h"

typedef struct cs_config_pars_tag
{
  int   mode;
  int   base;
  int   size;
  int   wait_state;
  int   setup_time;
  int   hold_time;
} cs_config_pars_t;

typedef enum
{
    MAC_ADDRESS_OP_GET=0,
    MAC_ADDRESS_OP_RELEASE,
    MAC_ADDRESS_OP_MAX
} MAC_ADDRESS_OPERATION;

typedef enum
{
	USB_HOST_FUNC,
	USB_DEVICE_FUNC,
	USB_ALL_FUNC
} USB_FUNCTION;


typedef void (* kerSysMacAddressNotifyHook_t)(unsigned char *pucaMacAddr, MAC_ADDRESS_OPERATION op);

#define UBUS_BASE_FREQUENCY_IN_MHZ  160

#define IF_NAME_ETH    "eth"
#define IF_NAME_USB    "usb"
#define IF_NAME_WLAN   "wl"
#define IF_NAME_MOCA   "moca"
#define IF_NAME_ATM    "atm"
#define IF_NAME_PTM    "ptm"
#define IF_NAME_GPON   "gpon"
#define IF_NAME_EPON   "epon"
#define IF_NAME_VEIP   "veip"

#define MAC_ADDRESS_ANY         (unsigned long) -1

/* A unique mac id is required for a WAN interface to request for a MAC address.
 * The 32bit mac id is formated as follows:
 *     bit 28-31: MAC Address IF type (MAC_ADDRESS_*)
 *     bit 20-27: the unit number. 0 for atm0, 1 for atm1, ...
 *     bit 12-19: the connection id which starts from 1.
 *     bit 0-11:  not used. should be zero.
 */
#define MAC_ADDRESS_PLC         0x10000000
#define MAC_ADDRESS_EPONONU     0x20000000
#define MAC_ADDRESS_EPON        0x40000000
#define MAC_ADDRESS_GPON        0x80000000
#define MAC_ADDRESS_1905        0x90000000
#define MAC_ADDRESS_ETH         0xA0000000
#define MAC_ADDRESS_USB         0xB0000000
#define MAC_ADDRESS_WLAN        0xC0000000
#define MAC_ADDRESS_MOCA        0xD0000000
#define MAC_ADDRESS_ATM         0xE0000000
#define MAC_ADDRESS_PTM         0xF0000000


/* board event definition */
/* wps button events */
#define SES_EVENT_BTN_PRESSED   0x00000001
#define SES_EVENT_BTN_SHORT     0x00000002
#define SES_EVENT_BTN_LONG      0x00000004
/* alias */
#define SES_EVENTS              SES_EVENT_BTN_PRESSED
#define SES_EVENT_BTN_AP        SES_EVENT_BTN_SHORT
#define SES_EVENT_BTN_STA       SES_EVENT_BTN_LONG


#define UNUSED_PARAM(a)        (a = a)

/*****************************************************************************/
/*          Function Prototypes                                              */
/*****************************************************************************/
#ifndef _LANGUAGE_ASSEMBLY
void dumpaddr( unsigned char *pAddr, int nLen );

extern void kerSysEarlyFlashInit( void );
extern int kerSysGetChipId( void );
extern char * kerSysGetChipName( char *buf, int len);
extern int kerSysGetDslPhyEnable( void );
extern void kerSysFlashInit( void );
extern void kerSysFlashAddrInfoGet(PFLASH_ADDR_INFO pflash_addr_info);
extern int kerSysCfeVersionGet(char *string, int stringLength);

extern int kerSysNvRamSet(const char *string, int strLen, int offset);
extern void kerSysNvRamGet(char *string, int strLen, int offset);
extern void kerSysNvRamLoad(void * mtd_ptr);
extern void kerSysNvRamGetBootline(char *bootline);
extern void kerSysNvRamGetBootlineLocked(char *bootline);
extern void kerSysNvRamGetBoardId(char *boardId);
extern void kerSysNvRamGetBoardIdLocked(char *boardId);
extern void kerSysNvRamGetBaseMacAddr(unsigned char *baseMacAddr);
extern unsigned long kerSysNvRamGetVersion(void);

extern int kerSysIsRootfsSet(void);
extern int kerSysIsIkosBootSet(void);
extern PFILE_TAG kerSysImageTagGet(void);
extern PFILE_TAG kerSysUpdateTagSequenceNumber(int imageNumber);

extern void kerSysSendtoMonitorTask(int msgType, char *msgData, int msgDataLen);

extern void kerSysDisableDyingGaspInterrupt(void);
extern void kerSysEnableDyingGaspInterrupt(void);

#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
extern void kerSysDisableDyingGaspOverride(void);
extern void kerSysGetDyingGaspConfig( unsigned int * afe_reg0, unsigned int * bg_bias0);
#endif

extern int kerSysIsBatteryEnabled(void);
extern int kerSysGetResetStatus(void);

extern int kerSysFsFileSet(const char *filename, char *buf, int len);
extern int kerSysFsFileGet(const char *filename, char *buf, int len);
extern int kerSysPersistentGet(char *string, int strLen, int offset);
extern int kerSysPersistentSet(char *string, int strLen, int offset);
extern int kerSysBackupPsiGet(char *string, int strLen, int offset);
extern int kerSysBackupPsiSet(char *string, int strLen, int offset);
extern int kerSysSyslogGet(char *string, int strLen, int offset);
extern int kerSysSyslogSet(char *string, int strLen, int offset);
extern int kerSysScratchPadList(char *tokBuf, int tokLen);
extern int kerSysScratchPadGet(char *tokName, char *tokBuf, int tokLen);
extern int kerSysScratchPadSet(char *tokName, char *tokBuf, int tokLen);
extern int kerSysScratchPadClearAll(void);
extern int kerSysBcmImageSet( int flash_start_addr, char *string, int size,
    int should_yield);
extern int kerSysGetSequenceNumber(int imageNumber);
extern int kerSysSetBootImageState( int state );
extern int kerSysGetBootImageState( void );
extern int kerSysSetOpticalPowerValues(unsigned short rxReading, unsigned short rxOffset, 
    unsigned short txReading);
extern int kerSysGetOpticalPowerValues(unsigned short *prxReading, unsigned short *prxOffset, 
    unsigned short *ptxReading);
extern int kerSysBlParmsGetInt( char *name, int *pvalue );
extern int kerSysBlParmsGetStr( char *name, char *pvalue, int size );
extern unsigned long kerSysGetMacAddressType( unsigned char *ifName );
extern int kerSysMacAddressNotifyBind(kerSysMacAddressNotifyHook_t hook);
extern int kerSysGetMacAddress( unsigned char *pucaAddr, unsigned long ulId );
extern int kerSysReleaseMacAddress( unsigned char *pucaAddr );
extern int kerSysGetMacAddresses( unsigned char *pucaMacAddr, unsigned int num_addresses, unsigned long ulId );
extern int kerSysReleaseMacAddresses( unsigned char *pucaMacAddr, unsigned int num_addresses );
extern void kerSysGetGponSerialNumber( unsigned char *pGponSerialNumber);
extern void kerSysGetGponPassword( unsigned char *pGponPassword);
extern int kerSysGetSdramSize( void );
extern void kerSysGetBootline(char *string, int strLen);
extern void kerSysSetBootline(char *string, int strLen);
extern void kerSysSoftReset(void);
extern void kerSysLedCtrl(BOARD_LED_NAME, BOARD_LED_STATE);
extern int kerSysFlashSizeGet(void);
extern int kerSysMemoryMappedFlashSizeGet(void);
extern unsigned long kerSysReadFromFlash( void *toaddr, unsigned long fromaddr, unsigned long len );
extern int kerSysEraseFlash(unsigned long eraseaddr, unsigned long len);
extern int kerSysWriteToFlash( unsigned long toaddr, void * fromaddr, unsigned long len);
extern void kerSysRegisterDyingGaspHandler(char *devname, void *cbfn, void *context);
extern void kerSysDeregisterDyingGaspHandler(char *devname);
extern int kerSysIsDyingGaspTriggered(void);
extern int kerConfigCs(BOARD_IOCTL_PARMS *parms);

extern int kerSysGetPciePortEnable(int port);
extern int kerSysGetPciePortMode(int port);
extern int kerSysGetSataPortEnable(void);
extern int kerSysGetUsbHostPortEnable(int port);
extern int kerSysGetUsbDeviceEnable(void);
extern int kerSysGetUsb30HostEnable(void);
extern int kerSysSetUsbPower(int on, USB_FUNCTION func);
extern void kerSysSetBootParm(char *name, char *value);
extern void kerSysInitPinmuxInterface(unsigned int interface);

#if defined(WIRELESS)
extern void kerSysSesEventTrigger( int forced );
#endif

#define  WL_SROM_DEFAULT_FILE           "/mnt/mfg_data/.wlsromcustomerfile.nvm"
#define  WL_SROM_CUSTOMER_FILE          "/data/.wlsromcustomerfile.nvm"

extern unsigned char kerSysGetWlanFeature(void);
unsigned int kerSysGetWifiLed(unsigned char core);
void kerSysWifiLed(unsigned int led, unsigned int on);

/*
 * Access to shared GPIO registers should be done by calling these
 * functions below, which will grab a spinlock while accessing the
 * GPIO register.  However, if your code needs to access a shared
 * GPIO register but cannot call these functions, you should still
 * acquire the spinlock.
 * (Technically speaking, I should include spinlock.h and declare extern here
 * but that breaks the C++ compile of xtm.)
 */
//#include <linux/spinlock.h>
//extern spinlock_t bcm_gpio_spinlock;
extern void kerSysSetGpioState(unsigned short bpGpio, GPIO_STATE_t state);
extern void kerSysSetGpioStateLocked(unsigned short bpGpio, GPIO_STATE_t state);
extern void kerSysSetGpioDir(unsigned short bpGpio);
extern void kerSysSetGpioDirLocked(unsigned short bpGpio);
extern int kerSysSetGpioDirInput(unsigned short bpGpio);
extern int kerSysGetGpioValue(unsigned short bpGpio);


// for the voice code, which has too many kernSysSetGpio to change
#define kerSysSetGpio kerSysSetGpioState


extern unsigned int kerSysGetUbusFreq(unsigned int miscStrapBus);
extern int kerSysGetAfeId( unsigned int *afeId );
#define __kerSysGetAfeId  kerSysGetAfeId

typedef void (*MocaHostIntrCallback)(int irq, void * arg);
extern void kerSysMocaHostIntrReset(int dev);
extern void kerSysRegisterMocaHostIntrCallback(MocaHostIntrCallback callback, void * userArg, int dev);
extern void kerSysMocaHostIntrEnable(int dev);
extern void kerSysMocaHostIntrDisable(int dev);

extern unsigned int kerSysGetExtIntInfo(unsigned int irq);

void kerSysRegisterPlcUkeCallback( void (* callback)(unsigned long timeInMs, void* param), void* param );
void kerSysDeregisterPlcUkeCallback( void (* callback)(unsigned long timeInMs, void* param) );
void kerSysRegisterPlcRandomizeCallback( void (* callback)(unsigned long timeInMs, void* param), void* param );
void kerSysDeregisterPlcRandomizeCallback( void (* callback)(unsigned long timeInMs, void* param) );

/* implement in arch dependent setup.c */
extern void* kerSysGetDslPhyMemory(void);

#if defined(CONFIG_BCM_AVS_PWRSAVE) || defined (CONFIG_BCM_MoCA_AVS)
extern void kerSysBcmEnableAvs(int enable);
extern int kerSysBcmAvsEnabled(void);
#endif

#if defined(CONFIG_BCM_WATCHDOG_TIMER)
void kerSysRegisterWatchdogCB(char *devname, void *cbfn, void *context);
void kerSysDeregisterWatchdogCB(char *devname);
#endif

#if defined(CONFIG_BCM_DDR_SELF_REFRESH_PWRSAVE)
#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963138)
#define CONFIG_BCM_PWRMNGT_DDR_SR_API
// The structure below is to be declared in ADSL PHY MIPS LMEM, if ADSL is compiled in
typedef struct _PWRMNGT_DDR_SR_CTRL_ {
  union {
   struct {
      unsigned int   phyBusy:1;
      unsigned int   phy1Busy:1;
      unsigned int   reserved0:14;
      unsigned int   tp0Busy:1;
      unsigned int   tp1Busy:1;
      unsigned int   tp2Busy:1;
      unsigned int   tp3Busy:1;
      unsigned int   reserved1:12;
    };
   struct {
      unsigned int phy:16;
      unsigned int host:16;
    };
    unsigned int     word;
  };
} PWRMNGT_DDR_SR_CTRL;

void BcmPwrMngtRegisterLmemAddr(PWRMNGT_DDR_SR_CTRL *pDdrSr);
#endif
#endif

extern const unsigned char* bcm_get_blparms(void);
unsigned int  bcm_get_blparms_size(void);

#define RDPA_WAN_TYPE_PSP_KEY           "RdpaWanType"
#define RDPA_WAN_OEMAC_PSP_KEY          "WanOEMac"
#define RDPA_EPON_SPEED_PSP_KEY         "EponSpeed"
#define RDPA_WAN_RATE_PSP_KEY           "WanRate"
#define RDPA_GBE_WAN_EMAC_PSP_KEY       "GbeWanEMac"
#define PSP_BUFLEN_16                   16

/* value sets for epon_speed */
#define EPON_SPEED_NORMAL 	"Normal"
#define EPON_SPEED_TURBO  	"Turbo"

/* value sets for oemac */
#define RDPA_WAN_OEMAC_VALUE_EPONMAC "EPONMAC"

/* value sets for wan_type */
#define RDPA_WAN_TYPE_VALUE_NONE    "NONE"
#define RDPA_WAN_TYPE_VALUE_GBE     "GBE"
#define RDPA_WAN_TYPE_VALUE_AE      "AE"
#define RDPA_WAN_TYPE_VALUE_EPON    "EPON"
#define RDPA_WAN_TYPE_VALUE_GPON    "GPON"
#define RDPA_WAN_TYPE_VALUE_XGPON1  "XGPON1"
#define RDPA_WAN_TYPE_VALUE_NGPON2  "NGPON2"
#define RDPA_WAN_TYPE_VALUE_XGS     "XGS"
#define RDPA_WAN_TYPE_VALUE_AUTO    "AUTO"

/* value sets for wan_rate */
#define RDPA_WAN_RATE_10G			"10"
#define RDPA_WAN_RATE_2_5G			"25"
#define RDPA_WAN_RATE_2G			"02"
#define RDPA_WAN_RATE_1G			"01"

/* 2ndWAN related defines : Format 0xAB; bit7=valid; bit6=unit; bit5-0=port */
#define WAN_2nd_PORT_PSP_KEY            "2ndWAN"
#define WAN_2nd_PORT_PSP_VALUE_LEN      4  
#define WAN_2nd_PORT_IS_VALID(x)        ( ( (x) & (1<<7) ) >> 7 )
#define WAN_2nd_PORT_GET_UNIT(x)        ( ( (x) & (1<<6) ) >> 6 )
#define WAN_2nd_PORT_GET_PORT(x)        ( ( (x) & (0x3F<<0) ) >> 0 )

extern int rdp_post_init(void);

#endif

#ifdef __cplusplus
}
#endif

#endif /* _BOARD_H */

