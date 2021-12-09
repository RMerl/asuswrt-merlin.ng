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


/***************************************************************************
* File Name  : board.c
*
* Description: This file contains Linux character device driver entry
*              for the board related ioctl calls: flash, get free kernel
*              page and dump kernel memory, etc.
*
*
***************************************************************************/

/* Includes. */
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/ctype.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>

#if defined(CONFIG_COMPAT)
#include "compat_board.h"
#endif
#include <bcmtypes.h>
#include <board.h>
#if defined(CONFIG_BCM_6802_MoCA)
#include "./moca/board_moca.h"
#include "./bbsi/bbsi.h"
#else
#include <spidevices.h>
#endif

#include <boardparms.h>
#include <bcm_extirq.h>
#include "board_button.h"
#include "board_wl.h"
#include "board_dg.h"
#include "board_proc.h"
#include "board_util.h"
#include "board_image.h"
#include "board_ioctl.h"
#include "board_wd.h"

#if defined(CONFIG_BCM_EXT_TIMER)
#include "bcm_ext_timer.h"
#endif

/* Externs. */

static int board_open( struct inode *inode, struct file *filp );
static ssize_t board_read(struct file *filp,  char __user *buffer, size_t count, loff_t *ppos);
static unsigned int board_poll(struct file *filp, struct poll_table_struct *wait);
static int board_release(struct inode *inode, struct file *filp);

static int board_driver_init(void);

/* brcmboard device driver related variables */
static struct cdev brcmboard_cdev;
static struct device *brcmboard_device = NULL;
static struct class *brcmboard_cl     = NULL;
static dev_t brcmboard_devId;

wait_queue_head_t g_board_wait_queue;

static struct file_operations board_fops =
{
    open:       board_open,
#if defined(HAVE_UNLOCKED_IOCTL)
    unlocked_ioctl: board_unlocked_ioctl,
#else
    ioctl:      board_ioctl,
#endif
#if defined(CONFIG_COMPAT)
    compat_ioctl: compat_board_ioctl,
#endif    
    poll:       board_poll,
    read:       board_read,
    release:    board_release,
};

uint32 board_init_success = -1;

#if defined(MODULE)
int init_module(void)
{
    return( brcm_board_init() );
}

void cleanup_module(void)
{
    if (MOD_IN_USE)
        printk("brcm flash: cleanup_module failed because module is in use\n");
    else
        brcm_board_cleanup();
}
#endif //MODULE

static int board_driver_init(void)
{
    int ret = 0;
  
    alloc_chrdev_region(&brcmboard_devId, 0, 2, "brcmboard");
    
    /* Create class and device ( /sys entries ) */
    brcmboard_cl = class_create(THIS_MODULE, "brcmboard");
    if(brcmboard_cl == NULL)
    {
       printk(KERN_ERR "Error creating device class\n");
       goto err_cdev_cleanup;
    }
    
    brcmboard_device = device_create(brcmboard_cl, NULL, brcmboard_devId, NULL, "brcmboard");
    if(brcmboard_device == NULL)
    {
       printk(KERN_ERR "Error creating device\n");
       goto err_class_cleanup;
    }
    
    /* Set the DMA masks for this device */
    dma_coerce_mask_and_coherent(brcmboard_device, DMA_BIT_MASK(32));
        
    /* Init the character device */
    cdev_init(&brcmboard_cdev, &board_fops);
    brcmboard_cdev.owner = THIS_MODULE;
    ret = cdev_add(&brcmboard_cdev, brcmboard_devId, 1);
    
    if( ret!=0 )
    {
       printk(KERN_ERR "Error %d adding brcmboard driver", ret);
       goto err_device_cleanup;
    }
    else
        return ret;

err_device_cleanup:
    device_destroy(brcmboard_cl, brcmboard_devId);
err_class_cleanup:
    class_destroy(brcmboard_cl);
err_cdev_cleanup:
    cdev_del(&brcmboard_cdev);

    return -1;
}

static int __init brcm_board_init( void )
{
    if( board_driver_init() == 0 )
        printk(KERN_ALERT "brcmboard registered\n");
    else
    { 
        printk( "brcm_board_init: fail to register device.\n");
        return -1;
    }

    printk("brcmboard: brcm_board_init entry\n");
    init_waitqueue_head(&g_board_wait_queue);

    board_util_init();
#if defined(CONFIG_BCM_6802_MoCA)
    /* Init moca */
    board_mocaInit(); 

    /* Init moca extirq */
    init_moca_ext_irq_info();
#endif

    init_reset_irq();

    board_wl_init();
    
    boardLedInit();
#if defined(CONFIG_BCM_EXT_TIMER)
    init_hw_timers();
#endif
    board_wd_init();

    registerBtns();
    add_proc_files();

    board_init_success = 0;

    return board_init_success;
}

void __exit brcm_board_cleanup( void )
{
    printk("brcm_board_cleanup()\n");

    del_proc_files();

    /* Delete cdev */
    cdev_del(&brcmboard_cdev);

    /* destroy the device and device class */
    device_destroy(brcmboard_cl, brcmboard_devId);
    class_destroy(brcmboard_cl);

    /* Unregister chrdev region */
    unregister_chrdev_region(brcmboard_devId, 1);

    /* Deinit specific modules if initialization was successful */
    if (board_init_success == 0)
    {
        board_wl_deinit();
        board_util_deinit();
	board_init_success = -1;
    }
}

static int board_open( struct inode *inode, struct file *filp )
{
    filp->private_data = board_ioc_alloc();

    if (filp->private_data == NULL)
        return -ENOMEM;

    return( 0 );
}

static int board_release(struct inode *inode, struct file *filp)
{
    BOARD_IOC *board_ioc = filp->private_data;

    wait_event_interruptible(g_board_wait_queue, 1);
    board_ioc_free(board_ioc);

    return( 0 );
}


static unsigned int board_poll(struct file *filp, struct poll_table_struct *wait)
{
    unsigned int mask = 0;
#if defined (WIRELESS)
    BOARD_IOC *board_ioc = filp->private_data;
#endif

    poll_wait(filp, &g_board_wait_queue, wait);
#if defined (WIRELESS)
    if(board_ioc->eventmask & SES_EVENTS){
        mask |= sesBtn_poll(filp, wait);
    }
#endif

    return mask;
}

static ssize_t board_read(struct file *filp,  char __user *buffer, size_t count, loff_t *ppos)
{
#if defined (WIRELESS)
    BOARD_IOC *board_ioc = filp->private_data;
    if(board_ioc->eventmask & SES_EVENTS){
        return sesBtn_read(filp, buffer, count, ppos);
    }
#endif
    return 0;
}

/***************************************************************************
* MACRO to call driver initialization and cleanup functions.
***************************************************************************/
module_init( brcm_board_init );
module_exit( brcm_board_cleanup );

EXPORT_SYMBOL(dumpaddr);
EXPORT_SYMBOL(kerSysGetChipId);
EXPORT_SYMBOL(kerSysGetChipName);
EXPORT_SYMBOL(kerSysMacAddressNotifyBind);
EXPORT_SYMBOL(kerSysGetMacAddressType);
EXPORT_SYMBOL(kerSysGetMacAddress);
EXPORT_SYMBOL(kerSysReleaseMacAddress);
EXPORT_SYMBOL(kerSysGetGponSerialNumber);
EXPORT_SYMBOL(kerSysGetGponPassword);
EXPORT_SYMBOL(kerSysFsFileGet);
EXPORT_SYMBOL(kerSysFsFileSet);
EXPORT_SYMBOL(kerSysGetSdramSize);
EXPORT_SYMBOL(kerSysGetDslPhyEnable);
EXPORT_SYMBOL(kerSysSetOpticalPowerValues);
EXPORT_SYMBOL(kerSysGetOpticalPowerValues);
EXPORT_SYMBOL(kerSysLedCtrl);
EXPORT_SYMBOL(kerSysRegisterDyingGaspHandler);
EXPORT_SYMBOL(kerSysDeregisterDyingGaspHandler);
EXPORT_SYMBOL(kerSysIsDyingGaspTriggered);
EXPORT_SYMBOL(kerSysSendtoMonitorTask);
EXPORT_SYMBOL(kerSysGetAfeId);
EXPORT_SYMBOL(kerSysGetUbusFreq);
EXPORT_SYMBOL(kerSysInitPinmuxInterface);
#if !defined(CONFIG_BCM960333) && !defined(CONFIG_BCM963381) && !defined(CONFIG_BCM947189)
EXPORT_SYMBOL(kerSysBcmSpiSlaveRead);
EXPORT_SYMBOL(kerSysBcmSpiSlaveReadReg32);
EXPORT_SYMBOL(kerSysBcmSpiSlaveWrite);
EXPORT_SYMBOL(kerSysBcmSpiSlaveWriteReg32);
EXPORT_SYMBOL(kerSysBcmSpiSlaveWriteBuf);

#if defined(CONFIG_BCM_6802_MoCA)
EXPORT_SYMBOL(kerSysBcmSpiSlaveReadBuf);
EXPORT_SYMBOL(kerSysBcmSpiSlaveModify);
EXPORT_SYMBOL(kerSysMocaHostIntrReset);
EXPORT_SYMBOL(kerSysRegisterMocaHostIntrCallback);
EXPORT_SYMBOL(kerSysMocaHostIntrEnable);
EXPORT_SYMBOL(kerSysMocaHostIntrDisable);
#endif
#endif

#if defined(CONFIG_BCM_WATCHDOG_TIMER)
EXPORT_SYMBOL(kerSysRegisterWatchdogCB);
EXPORT_SYMBOL(kerSysDeregisterWatchdogCB);
EXPORT_SYMBOL(bcm_suspend_watchdog);
EXPORT_SYMBOL(bcm_resume_watchdog);
#endif

EXPORT_SYMBOL(BpGetSimInterfaces);
EXPORT_SYMBOL(BpGetBoardId);
EXPORT_SYMBOL(BpGetBoardIds);
EXPORT_SYMBOL(BpGetGPIOverlays);
EXPORT_SYMBOL(BpGetFpgaResetGpio);
EXPORT_SYMBOL(BpGetEthernetMacInfo);
EXPORT_SYMBOL(BpGetEthernetMacInfoArrayPtr);
EXPORT_SYMBOL(BpGetGphyBaseAddress);
EXPORT_SYMBOL(BpGetDeviceOptions);
EXPORT_SYMBOL(BpGetPortConnectedToExtSwitch);
EXPORT_SYMBOL(BpGetRj11InnerOuterPairGpios);
EXPORT_SYMBOL(BpGetRtsCtsUartGpios);
EXPORT_SYMBOL(BpGetAdslLedGpio);
EXPORT_SYMBOL(BpGetAdslFailLedGpio);
EXPORT_SYMBOL(BpGetWanDataLedGpio);
EXPORT_SYMBOL(BpGetWanErrorLedGpio);
EXPORT_SYMBOL(BpGetVoipLedGpio);
EXPORT_SYMBOL(BpGetPotsLedGpio);
EXPORT_SYMBOL(BpGetVoip2FailLedGpio);
EXPORT_SYMBOL(BpGetVoip2LedGpio);
EXPORT_SYMBOL(BpGetVoip1FailLedGpio);
EXPORT_SYMBOL(BpGetVoip1LedGpio);
EXPORT_SYMBOL(BpGetDectLedGpio);
EXPORT_SYMBOL(BpGetMoCALedGpio);
EXPORT_SYMBOL(BpGetMoCAFailLedGpio);
EXPORT_SYMBOL(BpGetWirelessSesExtIntr);
EXPORT_SYMBOL(BpGetWirelessSesLedGpio);
EXPORT_SYMBOL(BpGetWirelessFlags);
EXPORT_SYMBOL(BpGetWirelessPowerDownGpio);
EXPORT_SYMBOL(BpUpdateWirelessSromMap);
EXPORT_SYMBOL(BpGetSecAdslLedGpio);
EXPORT_SYMBOL(BpGetSecAdslFailLedGpio);
EXPORT_SYMBOL(BpGetDslPhyAfeIds);
EXPORT_SYMBOL(BpGetAFELDPwrBoostGpio);
EXPORT_SYMBOL(BpGetExtAFEResetGpio);
EXPORT_SYMBOL(BpGetExtAFELDPwrGpio);
EXPORT_SYMBOL(BpGetExtAFELDModeGpio);
EXPORT_SYMBOL(BpGetIntAFELDPwrGpio);
EXPORT_SYMBOL(BpGetIntAFELDModeGpio);
EXPORT_SYMBOL(BpGetAFELDRelayGpio);
EXPORT_SYMBOL(BpGetExtAFELDDataGpio);
EXPORT_SYMBOL(BpGetExtAFELDClkGpio);
EXPORT_SYMBOL(BpGetAFEVR5P3PwrEnGpio);
EXPORT_SYMBOL(BpGetUart2SdoutGpio);
EXPORT_SYMBOL(BpGetUart2SdinGpio);
EXPORT_SYMBOL(BpGet6829PortInfo);
EXPORT_SYMBOL(BpGetEthSpdLedGpio);
EXPORT_SYMBOL(BpGetLaserDisGpio);
EXPORT_SYMBOL(BpGetLaserTxPwrEnGpio);
EXPORT_SYMBOL(BpGetVregSel1P2);
EXPORT_SYMBOL(BpGetVregAvsMin);
EXPORT_SYMBOL(BpGetI2cGpios);
EXPORT_SYMBOL(BpGetMiiOverGpioFlag);
EXPORT_SYMBOL(BpGetSwitchPortMap);
EXPORT_SYMBOL(BpGetMocaInfo);
EXPORT_SYMBOL(BpGetPhyResetGpio);
EXPORT_SYMBOL(BpGetPhyAddr);
EXPORT_SYMBOL(BpGetBatteryEnable);
EXPORT_SYMBOL(BpGetI2cDefXponBus);

#if defined(CONFIG_EPON_SDK)
EXPORT_SYMBOL(BpGetNumFePorts);
EXPORT_SYMBOL(BpGetNumGePorts);
EXPORT_SYMBOL(BpGetNumVoipPorts);
#endif
#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM963158)
EXPORT_SYMBOL(BpGetPonTxEnGpio);
EXPORT_SYMBOL(BpGetPonRxEnGpio);
EXPORT_SYMBOL(BpGetPonResetGpio);
#endif
EXPORT_SYMBOL(BpGetOpticalModulePresenceExtIntr);
EXPORT_SYMBOL(BpGetOpticalModulePresenceExtIntrGpio);
EXPORT_SYMBOL(BpGetOpticalModuleTxPwrDownGpio);
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
EXPORT_SYMBOL(BpGetExtAFELDPwrDslCtl);
EXPORT_SYMBOL(BpGetExtAFELDModeDslCtl);
EXPORT_SYMBOL(BpGetIntAFELDPwrDslCtl);
EXPORT_SYMBOL(BpGetIntAFELDModeDslCtl);
EXPORT_SYMBOL(BpGetIntAFELDDataDslCtl);
EXPORT_SYMBOL(BpGetIntAFELDClkDslCtl);
EXPORT_SYMBOL(BpGetExtAFELDDataDslCtl);
EXPORT_SYMBOL(BpGetExtAFELDClkDslCtl);
#endif

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908)
EXPORT_SYMBOL(BpGetSgmiiGpios);
#endif
EXPORT_SYMBOL(BpGetSfpDetectGpio);

EXPORT_SYMBOL(BpGetOpticalWan);
EXPORT_SYMBOL(BpGetRogueOnuEn);
EXPORT_SYMBOL(BpGetGpioLedSim);
EXPORT_SYMBOL(BpGetGpioLedSimITMS);

#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM963158)
EXPORT_SYMBOL(BpGetTsyncPonUnstableGpio);
EXPORT_SYMBOL(BpGetTsync1ppsPin);
EXPORT_SYMBOL(BpGetPmdAlarmExtIntr);
EXPORT_SYMBOL(BpGetPmdAlarmExtIntrGpio);
EXPORT_SYMBOL(BpGetWanSignalDetectedExtIntr);
EXPORT_SYMBOL(BpGetWanSignalDetectedExtIntrGpio);
EXPORT_SYMBOL(BpGetPmdMACEwakeEn);
EXPORT_SYMBOL(BpGetPmdInvSerdesRxPol);
EXPORT_SYMBOL(BpGetPmdInvSerdesTxPol);
EXPORT_SYMBOL(BpGetGpioPmdReset);
EXPORT_SYMBOL(BpGetPmdFunc);

EXPORT_SYMBOL(BpGetTrplxrTxFailExtIntr);
EXPORT_SYMBOL(BpGetTrplxrTxFailExtIntrGpio);
EXPORT_SYMBOL(BpGetTrplxrSdExtIntr);
EXPORT_SYMBOL(BpGetTrplxrSdExtIntrGpio);
EXPORT_SYMBOL(BpGetTxLaserOnOutN);
EXPORT_SYMBOL(BpGet1ppsStableGpio);
EXPORT_SYMBOL(BpGetLteResetGpio);
EXPORT_SYMBOL(BpGetStrapTxEnGpio);
EXPORT_SYMBOL(BpGetWifiOnOffExtIntr);
EXPORT_SYMBOL(BpGetWifiOnOffExtIntrGpio);
EXPORT_SYMBOL(BpGetLteExtIntr);
EXPORT_SYMBOL(BpGetLteExtIntrGpio);
EXPORT_SYMBOL(BpGetAePolarity);
EXPORT_SYMBOL(BpGetWanSignalDetectedGpio);
#endif

EXPORT_SYMBOL(BpGetWL0ActLedGpio);
EXPORT_SYMBOL(BpGetWL1ActLedGpio);

EXPORT_SYMBOL(BpGetUsbPwrOn0);
EXPORT_SYMBOL(BpGetUsbPwrOn1);
EXPORT_SYMBOL(BpGetUsbPwrFlt0);
EXPORT_SYMBOL(BpGetUsbPwrFlt1);

EXPORT_SYMBOL(BpGetAttachedInfo);

EXPORT_SYMBOL(BpGetAggregateLnkLedGpio);
EXPORT_SYMBOL(BpGetAggregateActLedGpio);

#if defined CONFIG_BCM963138 || defined CONFIG_BCM963148
EXPORT_SYMBOL(BpGetNfcExtIntr);
EXPORT_SYMBOL(BpGetNfcPowerGpio);
EXPORT_SYMBOL(BpGetNfcWakeGpio);
EXPORT_SYMBOL(BpGetBitbangSclGpio);
EXPORT_SYMBOL(BpGetBitbangSdaGpio);
#endif

#ifdef CONFIG_BP_PHYS_INTF
EXPORT_SYMBOL(BpInitPhyIntfInfo);
EXPORT_SYMBOL(BpGetAllPhyIntfInfo);
EXPORT_SYMBOL(BpGetPhyIntfInfo);
EXPORT_SYMBOL(BpGetPhyIntfInfoByType);
EXPORT_SYMBOL(BpGetPhyIntfNumByType);
EXPORT_SYMBOL(BpGetDslPhyAfeIdByIntfIdx);
EXPORT_SYMBOL(BpGetAFELDClkGpio);
EXPORT_SYMBOL(BpGetAFELDModeGpio);
EXPORT_SYMBOL(BpGetAFELDDataGpio);
EXPORT_SYMBOL(BpGetAFELDPwrGpio);
EXPORT_SYMBOL(BpGetAFELDClkDslCtl);
EXPORT_SYMBOL(BpGetAFELDModeDslCtl);
EXPORT_SYMBOL(BpGetAFELDPwrDslCtl);
EXPORT_SYMBOL(BpGetAFELDDataDslCtl);
EXPORT_SYMBOL(BpGetAFEResetGpio);
EXPORT_SYMBOL(BpGetWanActLedGpio);
EXPORT_SYMBOL(BpGetWanErrLedGpio);
EXPORT_SYMBOL(BpGetWanLinkLedGpio);
EXPORT_SYMBOL(BpGetWanLinkFailLedGpio);
EXPORT_SYMBOL(BpGetSfpModDetectGpio);
EXPORT_SYMBOL(BpGetSfpSigDetect);
EXPORT_SYMBOL(BpGetIntfMgmtType);
EXPORT_SYMBOL(BpGetIntfMgmtBusNum);
EXPORT_SYMBOL(BpGetAllAdvLedInfo);
EXPORT_SYMBOL(BpGetAdvLedInfo);
#endif

#if defined(_BCM947189_) || defined(CONFIG_BCM947189)
EXPORT_SYMBOL(BpGetMoCAResetGpio);
EXPORT_SYMBOL(BpGetSpiClkGpio);
EXPORT_SYMBOL(BpGetSpiCsGpio);
EXPORT_SYMBOL(BpGetSpiMisoGpio);
EXPORT_SYMBOL(BpGetSpiMosiGpio);
#endif 

EXPORT_SYMBOL(BpGetUsbDis);
EXPORT_SYMBOL(BpGetPciDis);
EXPORT_SYMBOL(BpGetSataDis);

EXPORT_SYMBOL(BpGetMemoryConfig);

MODULE_LICENSE("GPL");
