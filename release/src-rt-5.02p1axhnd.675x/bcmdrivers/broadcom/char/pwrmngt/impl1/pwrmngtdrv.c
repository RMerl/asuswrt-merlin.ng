/*
* <:copyright-BRCM:2008:proprietary:standard
* 
*    Copyright (c) 2008 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
:>
*/
#include <linux/version.h>
#include <linux/module.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
#include <generated/autoconf.h>
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21))
#include <linux/autoconf.h>
#else
#include <linux/config.h>
#endif

#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/pci.h>
#include "bcmpci.h"
#include "bcmpwrmngtdrv.h"
#include "bcmpwrmngtcfg.h"
#include "pwrmngt.h"
#include "bcm_map_part.h"
#include "board.h"
#include "pmc_pcie.h"
#include "bmu.h"
#include "bcm_misc_hw_init.h"

#define PWRMNGT_IF_MAX   10
#define IFINVALID         -1

/* Typedefs. */
typedef void (*FN_IOCTL) (unsigned long arg);
static DEFINE_MUTEX(pwrmgnt_mutex);

/* ---- Device driver init and cleanup -------------------------------- */

static int __init bcmPwrMngtDrvInit( void );
static void __exit bcmPwrMngtDrvCleanup( void );
extern void __init bcmInitBmuHandler( void );
extern void __exit bcmDeinitBmuHandler( void );

/* ---- Device driver entry points ------------------------------------ */

static int bcmPwrMngtDrv_open( struct inode *inode, struct file *filp );
int bcmPwrMngtDrv_ioctl( struct inode *inode, struct file *flip, 
                          unsigned int command, unsigned long arg );
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
static long bcmPwrMngtDrv_unlocked_ioctl(struct file *file, uint cmd, ulong arg);
#endif
/* ---- Device driver ioctl entry points ------------------------------------ */
static void DoPwrMngtInitialize( unsigned long arg );
static void DoPwrMngtUninitialize( unsigned long arg );
static void DoPwrMngtSetConfig( unsigned long arg );
static void DoPwrMngtGetConfig( unsigned long arg );


PWRMNGT_STATUS BcmPwrMngtInitialize(PPWRMNGT_CONFIG_PARAMS  pInitParms );
PWRMNGT_STATUS BcmPwrMngtUninitialize( void );
PWRMNGT_STATUS BcmPwrMngtGetConfig( PPWRMNGT_CONFIG_PARAMS pConfigParams, ui32 configMask);
PWRMNGT_STATUS BcmPwrMngtSetConfig( PPWRMNGT_CONFIG_PARAMS pConfigParams, ui32 configMask);


static struct file_operations pwrmngt_drv_fops =
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
   .unlocked_ioctl     = bcmPwrMngtDrv_unlocked_ioctl,
#if defined(CONFIG_COMPAT)
   .compat_ioctl       = bcmPwrMngtDrv_unlocked_ioctl,
#endif
#else
   .ioctl     = bcmPwrMngtDrv_ioctl,
#endif
   .open      = bcmPwrMngtDrv_open,
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 33)
static long bcmPwrMngtDrv_unlocked_ioctl(struct file *file, uint cmd, ulong arg)
{

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 00)
   struct inode *inode = file->f_dentry->d_inode;
#else
   struct inode *inode = file_inode(file);
#endif
   long ret;
  
   mutex_lock(&pwrmgnt_mutex);
   ret = bcmPwrMngtDrv_ioctl(inode, file, cmd, arg);
   mutex_unlock(&pwrmgnt_mutex);

   return ret;
}
#endif

#if defined(PCIE_BASE)
/***********************************************************************
* Function Name: bcmPwrMngtCheckPowerDownPcie
* Description  : Power Down PCIe if no device enumerated
*                Otherwise enable Power Saving modes
***********************************************************************/
void bcmPwrMngtCheckPowerDownPcie(void)
{
#if !defined (CONFIG_BCM963138) && !defined(CONFIG_BCM963148) && !defined(CONFIG_BCM94908) /* This feature is implemented in the brcm pcie driver starting with this chip */
   struct pci_dev *dev = NULL;
   int pcie0DevPresent = 0;
#if defined(CONFIG_BCM96838)
   int pcie1DevPresent = 0;
#endif

   while ((dev=pci_get_device(PCI_ANY_ID, PCI_ANY_ID, dev))!=NULL) {
      if((BCM_BUS_PCIE_DEVICE == dev->bus->number) && !pcie0DevPresent) {
         /* Enable PCIe L1 PLL power savings */
#if !defined(CONFIG_BCM963381)
         PCIEH_BLK_1800_REGS->phyCtrl[1] |= REG_POWERDOWN_P1PLL_ENA;
#endif
#if defined(PCIE_BRIDGE_CLKREQ_ENABLE)
         {
            unsigned int GPIOOverlays;

            /* Enable PCIe CLKREQ# power savings */
            if( (BpGetGPIOverlays(&GPIOOverlays) == BP_SUCCESS) && (GPIOOverlays & BP_OVERLAY_PCIE_CLKREQ)) {
               PCIEH_BRIDGE_REGS->pcieControl |= PCIE_BRIDGE_CLKREQ_ENABLE;
            }
         }
#endif
         pcie0DevPresent = 1;
      }
	  
#if defined(PCIEH_1_BLK_1800_REGS)
      if((BCM_BUS_PCIE1_DEVICE == dev->bus->number) && !pcie1DevPresent) {
         /* Enable PCIe L1 PLL power savings */
         PCIEH_1_BLK_1800_REGS->phyCtrl[1] |= REG_POWERDOWN_P1PLL_ENA;
#if defined(PCIE_BRIDGE_CLKREQ_ENABLE)
         {
            unsigned int GPIOOverlays;

            /* Enable PCIe CLKREQ# power savings */
            if( (BpGetGPIOverlays(&GPIOOverlays) == BP_SUCCESS) && (GPIOOverlays & BP_OVERLAY_PCIE_CLKREQ)) {
               PCIEH1_BRIDGE_REGS->pcieControl |= PCIE_BRIDGE_CLKREQ_ENABLE;
            }
         }
#endif
         pcie1DevPresent = 1;
      }
#endif
   }
   
#if defined(PCIE_MISC_HARD_PCIE_HARD_DEBUG_SERDES_IDDQ)
   /* PCIe is disabled in 6318B */
   {
      if(!pcie0DevPresent)
         PCIEH_MISC_HARD_REGS->hard_eco_ctrl_hard |= PCIE_MISC_HARD_PCIE_HARD_DEBUG_SERDES_IDDQ;
#if defined(PCIEH_1_MISC_HARD_REGS)
      if(!pcie1DevPresent)
         PCIEH_1_MISC_HARD_REGS->hard_eco_ctrl_hard |= PCIE_MISC_HARD_PCIE_HARD_DEBUG_SERDES_IDDQ;
#endif
   }
#endif

#if defined(CONFIG_BCM96838)
   if(!pcie0DevPresent)
   {
      printk("PCIe0: No device found - Powering down\n");
      pmc_pcie_power_down(0);
   }

   if(!pcie1DevPresent)
   {
      printk("PCIe1: No device found - Powering down\n");
      pmc_pcie_power_down(1);
   }
		
   return;
#elif defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848)
   if(!pcie0DevPresent)
   {
      printk("PCIe0: No device found - Powering down\n");
      pmc_pcie_power_down(0);
   }
   return;
#else

   if(pcie0DevPresent)
      return;
	
   printk("PCIe: No device found - Powering down\n");
   /* pcie clock disable*/
   
#if defined(PCIE_CLK_EN)
   PERF->blkEnables &= ~PCIE_CLK_EN;
#endif

#if defined(MISC_CLK100_DISABLE)
   MISC->miscLcpll_ctrl |= MISC_CLK100_DISABLE;
#endif

   /* pcie serdes disable */
#if defined(SERDES_PCIE_ENABLE) && defined(SERDES_PCIE_EXD_ENABLE)
   MISC->miscSerdesCtrl &= ~(SERDES_PCIE_ENABLE|SERDES_PCIE_EXD_ENABLE);
#endif

   /* pcie disable additional clocks */
#if defined(PCIE_UBUS_CLK_EN)
   PLL_PWR->PllPwrControlActiveUbusPorts &= ~PORT_ID_PCIE;
   PERF->blkEnablesUbus &= ~PCIE_UBUS_CLK_EN;
#endif

#if defined(PCIE_25_CLK_EN)
   PERF->blkEnables &= ~PCIE_25_CLK_EN;
#endif 

#if defined(PCIE_ASB_CLK_EN)
   PERF->blkEnables &= ~PCIE_ASB_CLK_EN;
#endif

#if defined(SOFT_RST_PCIE) && defined(SOFT_RST_PCIE_EXT) && defined(SOFT_RST_PCIE_CORE)
   /* pcie and ext device */
   PERF->softResetB &= ~(SOFT_RST_PCIE | SOFT_RST_PCIE_EXT | SOFT_RST_PCIE_CORE);
#endif    

#if defined(SOFT_RST_PCIE_HARD)
   PERF->softResetB &= ~SOFT_RST_PCIE_HARD;
#endif

#if defined(IDDQ_PCIE)
   PLL_PWR->PllPwrControlIddqCtrl |= IDDQ_PCIE;
#endif

#if defined(CONFIG_BCM960333) && defined(CONFIG_PCI)
   /* Stop the 100 MHz clock from AFE to avoid an increase in AFE_PI
    * current when a PCI card is not present (JIRA FWBCAPLC-2219) */
   stop_100mhz_afe_clock();
#endif
#endif
#endif
}
#endif

/***************************************************************************
 * Function Name: bcmPwrMngtDrvInit
 * Description  : Initial function that is called at system startup that
 *                registers this device.
 * Returns      : None.
 ***************************************************************************/
static int __init bcmPwrMngtDrvInit( void )
{
   int ret = 0;

   if(register_chrdev( PWRMNGT_DRV_MAJOR, "pwrmngt", &pwrmngt_drv_fops ))
   {
      PWR_TRC("bcmPwrMngtDrvInit failed\n");
      ret = -1;
	  return ret;
   }

#if defined(PCIE_BASE)
   bcmPwrMngtCheckPowerDownPcie();
#endif

#if defined(CONFIG_BCM_BMU)
   bcmInitBmuHandler();
#endif
   
   return ret;
} /* bcmPwrMngtDrvInit */ 

/***************************************************************************
 * Function Name: bcmPwrMngtDrvCleanup 
 * Description  : Final function that is called when the module is unloaded.
 * Returns      : None.
 ***************************************************************************/
static void __exit bcmPwrMngtDrvCleanup( void )
{
   unregister_chrdev( PWRMNGT_DRV_MAJOR, "pwrmngt" );

#if defined(CONFIG_BCM_BMU)
   bcmDeinitBmuHandler();
#endif

   return;
} /* bcmPwrMngtDrvCleanup */ 

/***************************************************************************
 * Function Name: bcmPwrMngtDrv_open 
 * Description  : Called when an application opens this device.
 * Returns      : 0 - success
 ***************************************************************************/
static int bcmPwrMngtDrv_open( struct inode *inode, struct file *filp )
{
   /* Do nothing, but get rid of warnings */
   (void)inode;
   (void)filp;
   return 0;
} /* bcmPwrMngtDrv_open */

/***************************************************************************
 * Function Name: bcmPwrMngtDrv_ioctl
 * Description  : Main entry point for an application ioctl commands.
 * Returns      : 0 - success or error
 ***************************************************************************/
int bcmPwrMngtDrv_ioctl( struct inode *inode, struct file *filp, 
    unsigned int command, unsigned long arg )
{
   int err = 0;
   unsigned int cmdnr = _IOC_NR(command);

   FN_IOCTL IoctlFuncs[] = {DoPwrMngtInitialize, DoPwrMngtUninitialize,
        DoPwrMngtSetConfig, DoPwrMngtGetConfig, NULL};

   if( cmdnr < MAX_PWRMNGT_DRV_IOCTL_COMMANDS && IoctlFuncs[cmdnr] != NULL )
   {
        (*IoctlFuncs[cmdnr]) (arg);
   }
   else
       err = -EINVAL;

   return err ;
} /* bcmPwrMngtDrv_ioctl */

/***************************************************************************
 * Function Name: DoPwrMngtInitialize
 * Description  : Calls BcmPwrMngtInitialize on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoPwrMngtInitialize( unsigned long arg )
{
   PWRMNGT_DRV_INITIALIZE KArg;

   PWR_TRC ("%s entry \n", __FUNCTION__ );

   if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
   {
      KArg.status = BcmPwrMngtInitialize ( &KArg.init ) ;
      put_user( KArg.status, &((PPWRMNGT_DRV_INITIALIZE) arg)->status );
      if (KArg.status != PWRMNGTSTS_SUCCESS)
         PWR_TRC ("bcmPwrMngtInitialize failed. status = %d \n", KArg.status) ;
   }

   PWR_TRC (" %s exit \n", __FUNCTION__ );
} /* DoPwrMngtInitialize */

/***************************************************************************
 * Function Name: DoPwrMngtUninitialize
 * Description  : Calls BcmPwrMngtUninitialize on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoPwrMngtUninitialize( unsigned long arg )
{
   PWRMNGT_STATUS status = BcmPwrMngtUninitialize () ;
   put_user( status, &((PPWRMNGT_DRV_STATUS_ONLY) arg)->status ) ;
} /* DoPwrMngtUninitialize */

/***************************************************************************
 * Function Name: DoPwrMngtSetConfig
 * Description  : Calls BcmPwrMngtSetConfig on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoPwrMngtSetConfig( unsigned long arg )
{
   PWRMNGT_DRV_CONFIG_PARAMS KArg ;

   if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
   {
      KArg.status = BcmPwrMngtSetConfig( &KArg.configParams, KArg.configMask ) ;
      put_user( KArg.status, &((PPWRMNGT_DRV_CONFIG_PARAMS) arg)->status ) ;
   }
} /* DoPwrMngtSetConfig */

/***************************************************************************
 * Function Name: bcmDoPwrMngtGetConfig 
 * Description  : Calls BcmPwrMngtGetConfig on behalf of a user program.
 * Returns      : None.
 ***************************************************************************/
static void DoPwrMngtGetConfig( unsigned long arg )
{
   PWRMNGT_DRV_CONFIG_PARAMS KArg ;

   if( copy_from_user( &KArg, (void *) arg, sizeof(KArg) ) == 0 )
   {
      KArg.status = BcmPwrMngtGetConfig( &KArg.configParams, KArg.configMask ) ;

      if( KArg.status == PWRMNGTSTS_SUCCESS )
      {
         copy_to_user( &((PPWRMNGT_DRV_CONFIG_PARAMS) arg)->configParams,
                &KArg.configParams, sizeof(KArg.configParams) );
      }

      put_user( KArg.status, &((PPWRMNGT_DRV_CONFIG_PARAMS) arg)->status );
   }
} /* DoPwrMngtGetConfig */

/***************************************************************************
 * Power Management to call driver initialization and cleanup functions.
 ***************************************************************************/
module_init( bcmPwrMngtDrvInit );
module_exit( bcmPwrMngtDrvCleanup );
MODULE_LICENSE("Proprietary");

EXPORT_SYMBOL(BcmPwrMngtInitialize);
EXPORT_SYMBOL(BcmPwrMngtUninitialize); 
EXPORT_SYMBOL(BcmPwrMngtSetConfig);
EXPORT_SYMBOL(BcmPwrMngtGetConfig);
