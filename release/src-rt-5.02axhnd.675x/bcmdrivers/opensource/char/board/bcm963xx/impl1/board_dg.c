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

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/slab.h>

#include <bcmtypes.h>
#include <bcm_map_part.h>
#include <board.h>
#include <boardparms.h>
#include <bcm_intr.h>
#include <shared_utils.h>
#include <bcm_pinmux.h>

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963381)
#include "pmc_dsl.h"
#endif

#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
#include "pmc_dgasp.h"
#endif

#include "board_dg.h"
#include "board_wl.h"
#include "board_util.h"
#include "board_wd.h"

/* DG controls */
#define DISABLE_DGINT_IF_ACTIVE_ON_BOOT 0 /* This flag will keep DG interrupt disabled if DG is active on bootup. */
                                          /* Use this flag for debug purposes only */

#if  !defined(CONFIG_BCM960333) && !defined(CONFIG_BCM947189)
static irqreturn_t kerSysDyingGaspIsr(int irq, void * dev_id);
#endif

/* dgaspMutex Protects dyingGasp enable/disable functions */
/* also protects list add and delete, but is ignored during isr. */
static DEFINE_MUTEX(dgaspMutex);
static volatile int isDyingGaspTriggered = 0;
static CB_DGASP_LIST *g_cb_dgasp_list_head = NULL;

#if !defined(CONFIG_BCM960333) && !defined(CONFIG_BCM947189)
static int dg_enabled = 0;
static int dg_prevent_enable = 0;
static int dg_active_on_boot = 0;
static int isDGActiveOnBoot(void);
#endif

/***************************************************************************
* Dying gasp ISR and functions.
***************************************************************************/

/* For any driver running on another cpu that needs to know if system is losing
   power */
int kerSysIsDyingGaspTriggered(void)
{
    return isDyingGaspTriggered;
}

#if !defined(CONFIG_BCM960333) && !defined(CONFIG_BCM947189)
static irqreturn_t kerSysDyingGaspIsr(int irq, void * dev_id)
{
    struct list_head *pos;
    CB_DGASP_LIST *tmp = NULL, *dslCallBack = NULL;
    unsigned short usPassDyingGaspGpio;        // The GPIO pin to propogate a dying gasp signal

    isDyingGaspTriggered = 1;
#if defined(CONFIG_BCM947189)
#else
#if defined(BRCM_SERIAL_CONSOLE)
    UART->Data = 'D';
    UART->Data = '%';
    UART->Data = 'G';
#elif defined(ARM_UART)
    ARM_UART->dr = 'D';
    ARM_UART->dr = '%';
    ARM_UART->dr = 'G';
#endif
#endif
#if defined (WIRELESS)
    kerSetWirelessPD(WLAN_OFF);
#endif
    /* first to turn off everything other than dsl */
    list_for_each(pos, &g_cb_dgasp_list_head->list) {
        tmp = list_entry(pos, CB_DGASP_LIST, list);
        if(strncmp(tmp->name, "dsl", 3)) {
            (tmp->cb_dgasp_fn)(tmp->context);
        }else {
            dslCallBack = tmp;
        }
    }
    
    // Invoke dying gasp handlers
    if(dslCallBack)
        (dslCallBack->cb_dgasp_fn)(dslCallBack->context);

    /* reset and shutdown system */



    /* Set WD to fire in 1 sec in case power is restored before reset occurs */
#if defined(CONFIG_BCM_WATCHDOG_TIMER)
    bcm_suspend_watchdog();
#endif
    start_watchdog(1000000, 1);

    // If configured, propogate dying gasp to other processors on the board
    if(BpGetPassDyingGaspGpio(&usPassDyingGaspGpio) == BP_SUCCESS)
    {
        // Dying gasp configured - set GPIO
        kerSysSetGpioState(usPassDyingGaspGpio, kGpioInactive);
    }

    // If power is going down, nothing should continue!
    while (1);
    return( IRQ_HANDLED );
}
#endif /* !defined(CONFIG_BCM963138) && !defined(CONFIG_BCM963148)*/


#if !defined(CONFIG_BCM960333) && !defined(CONFIG_BCM947189)
void kerSysDisableDyingGaspInterrupt( void )
{
    mutex_lock(&dgaspMutex);

    if (!dg_enabled) {
        mutex_unlock(&dgaspMutex);
        return;
    }

    BcmHalInterruptDisable(INTERRUPT_ID_DG);
    printk("DYING GASP IRQ Disabled\n");
    dg_enabled = 0;
    mutex_unlock(&dgaspMutex);
}
EXPORT_SYMBOL(kerSysDisableDyingGaspInterrupt);

static int isDGActiveOnBoot(void)
{
    int dg_active = 0;

    /* Check if DG is already active */
#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96858) || defined(CONFIG_BCM94908) || \
    defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856) || defined (CONFIG_BCM947622) || defined (CONFIG_BCM963178) || defined(CONFIG_BCM96878)
    dg_active = 0;
#elif defined(CONFIG_BCM963158)
    dg_active =  INTSET->IrqStatus0[0] & (1 << (INTERRUPT_DYING_GASP_IRQ - SPI_TABLE_OFFSET));
#elif defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
    dg_active =  PERF->IrqStatus[0] & (1 << (INTERRUPT_ID_DG - ISR_TABLE_OFFSET));
#elif defined(CONFIG_BCM963381)
    dg_active =  (PERF->IrqStatus & (((uint64)1) << (INTERRUPT_ID_DG - INTERNAL_ISR_TABLE_OFFSET)))? 1 : 0;
#else
    dg_active =  PERF->IrqControl[0].IrqStatus & (1 << (INTERRUPT_ID_DG - INTERNAL_ISR_TABLE_OFFSET));
#endif

    if( dg_active )
        printk("DYING GASP IRQ ACTIVE on Boot!\n");

    return dg_active;
}

#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
void kerSysDisableDyingGaspOverride(void)
{
	pmc_dgasp_override_disable();
	
}
EXPORT_SYMBOL(kerSysDisableDyingGaspOverride);

void kerSysGetDyingGaspConfig( unsigned int * afe_reg0, unsigned int * bg_bias0)
{
	pmc_dgasp_get_config( afe_reg0, bg_bias0 );
}
EXPORT_SYMBOL(kerSysGetDyingGaspConfig);
#endif

void kerSysEnableDyingGaspInterrupt( void )
{
    static int dg_mapped = 0;

    mutex_lock(&dgaspMutex);
    
    /* Ignore requests to enable DG if it is already enabled */
    if (dg_enabled) {
        printk("DYING GASP IRQ Already Enabled\n");
        mutex_unlock(&dgaspMutex);
        return;
    }

    /* Set DG Parameters */
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963158)
    msleep(5);
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963381)
    /* Setup dying gasp threshold @ 1.25V with 0mV Heysteresis */
    DSLPHY_AFE->BgBiasReg[0] = (DSLPHY_AFE->BgBiasReg[0] & ~0xffff) | 0x04cd;
    /* Note that these settings are based on the ATE characterization of the threshold and hysterises 
     * register settings and as such dont match what is stated in the register descriptions */
    DSLPHY_AFE->AfeReg[0] = (DSLPHY_AFE->AfeReg[0] & ~0xffff) | 0x00EF;
#elif defined(CONFIG_BCM963158) && (CONFIG_BRCM_CHIP_REV!=0x63158A0)
    /* For 63158B0 DGASP settings are initialized in kerSysInitDyingGaspHandler. The DSL
     * driver ensures that the DGASP config us updated everytime the phy is reset  */
#endif /* defined(CONFIG_BCM963158) */   
    msleep(5);
#endif /* defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963158) */    


    if (dg_prevent_enable) 
    {
        printk("DYING GASP enabling postponed\n");
    } 
    else 
    {        
        if (dg_mapped) 
        { 
            BcmHalInterruptEnable(INTERRUPT_ID_DG);
            printk("DYING GASP IRQ Enabled\n");
        }
        else 
        {
            BcmHalMapInterrupt((FN_HANDLER)kerSysDyingGaspIsr, (void*)0, INTERRUPT_ID_DG);
#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
            BcmHalInterruptEnable( INTERRUPT_ID_DG );
#endif
            dg_mapped = 1;
            printk("DYING GASP IRQ Initialized and Enabled\n");
        }
        dg_enabled = 1;
    }
    mutex_unlock(&dgaspMutex);
}
EXPORT_SYMBOL(kerSysEnableDyingGaspInterrupt);
#endif /* !defined(CONFIG_BCM960333) && !defined(CONFIG_BCM947189) */

void __init kerSysInitDyingGaspHandler( void )
{
    CB_DGASP_LIST *new_node;

    if( g_cb_dgasp_list_head != NULL) {
        printk("Error: kerSysInitDyingGaspHandler: list head is not null\n");
        return;
    }
    new_node= (CB_DGASP_LIST *)kmalloc(sizeof(CB_DGASP_LIST), GFP_KERNEL);
    memset(new_node, 0x00, sizeof(CB_DGASP_LIST));
    INIT_LIST_HEAD(&new_node->list);
    g_cb_dgasp_list_head = new_node;

#if !defined(CONFIG_BCM960333) && !defined(CONFIG_BCM947189)
    /* Disable DG Interrupt */
    kerSysDisableDyingGaspInterrupt();

#if defined(CONFIG_BCM96838)
    GPIO->dg_control |= (1 << DG_EN_SHIFT);    
#elif defined(CONFIG_BCM96848)
    MISC_REG->DgSensePadCtrl |= (1 << DG_EN_SHIFT); 
#elif defined(CONFIG_BCM96858) || (defined(CONFIG_BCM963158) && (CONFIG_BRCM_CHIP_REV==0x63158A0))
    MISC->miscDgSensePadCtrl |= (1 << DG_EN_SHIFT);
#elif defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178)
    /* Set BgBias and Afe regs via BPCM */
    pmc_dgasp_init();
#elif defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM947622) || defined(CONFIG_BCM963178) || defined(CONFIG_BCM96878)
    TOP->DgSensePadCtl |= (1 << DG_EN_SHIFT);
#elif defined(CONFIG_BCM94908) 
    TOPCTRL->DgSensePadCtrl |= (1 << DG_EN_SHIFT);
#else
    {
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963381)
        pmc_dsl_power_up();
        pmc_dsl_core_reset();
#endif
    }
#endif /* defined(CONFIG_BCM96838) */

    /* Set DG related global variables */
    mutex_lock(&dgaspMutex);

    /* Prevent DG enable if this is a battery enabled system */
    dg_prevent_enable = kerSysIsBatteryEnabled(); 

    /* Check if DG is active on boot */
    dg_active_on_boot = isDGActiveOnBoot();

#if DISABLE_DGINT_IF_ACTIVE_ON_BOOT
    dg_prevent_enable |= dg_active_on_boot; 
#else
    dg_active_on_boot = 0;    
#endif    

    mutex_unlock(&dgaspMutex);

#if defined(CONFIG_BCM963158) && (CONFIG_BRCM_CHIP_REV==0x63158A0)
       /* Issue in 63158_A0 DSL builds prevents programming of afe regs before phy boots
        * therefore, we postpone the enabling of the dying gasp ISR over here. It will
        * be enabled instead when the phy calls kersysEnableDyingGaspInterrupt after it
        * has finished booting */
       printk("DYING GASP enabling postponed until DGASP H/W is ready!\n");
#else
       /* Enable DG Interrupt */
       kerSysEnableDyingGaspInterrupt();
#endif /* defined(CONFIG_BCM963158) && (CONFIG_BRCM_CHIP_REV==0x63158A0) */            

#endif /* !defined(CONFIG_BCM960333) */
} 

void __exit kerSysDeinitDyingGaspHandler( void )
{
    struct list_head *pos;
    CB_DGASP_LIST *tmp;

    if(g_cb_dgasp_list_head == NULL)
        return;

    list_for_each(pos, &g_cb_dgasp_list_head->list) {
        tmp = list_entry(pos, CB_DGASP_LIST, list);
        list_del(pos);
        kfree(tmp);
    }

    kfree(g_cb_dgasp_list_head);
    g_cb_dgasp_list_head = NULL;

} /* kerSysDeinitDyingGaspHandler */

void kerSysRegisterDyingGaspHandler(char *devname, void *cbfn, void *context)
{
    CB_DGASP_LIST *new_node;

    // do all the stuff that can be done without the lock first
    if( devname == NULL || cbfn == NULL ) {
        printk("Error: kerSysRegisterDyingGaspHandler: register info not enough (%s,%p,%p)\n", devname, cbfn, context);
        return;
    }

    if (strlen(devname) > (IFNAMSIZ - 1)) {
        printk("Warning: kerSysRegisterDyingGaspHandler: devname too long, will be truncated\n");
    }

    new_node= (CB_DGASP_LIST *)kmalloc(sizeof(CB_DGASP_LIST), GFP_KERNEL);
    memset(new_node, 0x00, sizeof(CB_DGASP_LIST));
    INIT_LIST_HEAD(&new_node->list);
    strncpy(new_node->name, devname, IFNAMSIZ-1);
    new_node->cb_dgasp_fn = (cb_dgasp_t)cbfn;
    new_node->context = context;

    // OK, now acquire the lock and insert into list
    mutex_lock(&dgaspMutex);
    if( g_cb_dgasp_list_head == NULL) {
        printk("Error: kerSysRegisterDyingGaspHandler: list head is null\n");
        kfree(new_node);
    } else {
        list_add(&new_node->list, &g_cb_dgasp_list_head->list);
        printk("dgasp: kerSysRegisterDyingGaspHandler: %s registered \n", devname);
    }
    mutex_unlock(&dgaspMutex);

    return;
} /* kerSysRegisterDyingGaspHandler */

void kerSysDeregisterDyingGaspHandler(char *devname)
{
    struct list_head *pos;
    CB_DGASP_LIST *tmp;
    int found=0;

    if(devname == NULL) {
        printk("Error: kerSysDeregisterDyingGaspHandler: devname is null\n");
        return;
    }

    printk("kerSysDeregisterDyingGaspHandler: %s is deregistering\n", devname);

    mutex_lock(&dgaspMutex);
    if(g_cb_dgasp_list_head == NULL) {
        printk("Error: kerSysDeregisterDyingGaspHandler: list head is null\n");
    } else {
        list_for_each(pos, &g_cb_dgasp_list_head->list) {
            tmp = list_entry(pos, CB_DGASP_LIST, list);
            if(!strcmp(tmp->name, devname)) {
                list_del(pos);
                kfree(tmp);
                found = 1;
                printk("kerSysDeregisterDyingGaspHandler: %s is deregistered\n", devname);
                break;
            }
        }
        if (!found)
            printk("kerSysDeregisterDyingGaspHandler: %s not (de)registered\n", devname);
    }
    mutex_unlock(&dgaspMutex);

    return;
} /* kerSysDeregisterDyingGaspHandler */

#if !defined(CONFIG_BCM960333) && !defined(CONFIG_BCM947189)
void kerSysDyingGaspIoctl(DGASP_ENABLE_OPTS opt)
{
    switch (opt)
    {
        case DG_ENABLE_FORCE:
             mutex_lock(&dgaspMutex);
             if( !dg_active_on_boot )
                 dg_prevent_enable = 0;
             mutex_unlock(&dgaspMutex);
             /* FALLTHROUGH */
        case DG_ENABLE:
             kerSysEnableDyingGaspInterrupt();
             break;
        case DG_DISABLE_PREVENT_ENABLE:
             mutex_lock(&dgaspMutex);
             dg_prevent_enable = 1;
             mutex_unlock(&dgaspMutex);
             /* FALLTHROUGH */
        case DG_DISABLE:
             kerSysDisableDyingGaspInterrupt();
             break;
        default:
             break;
    }

    return;
}
#endif
