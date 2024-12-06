/*
* <:copyright-BRCM:2021:DUAL/GPL:standard
* 
*    Copyright (c) 2021 Broadcom 
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
#include <linux/if.h>
#include <linux/interrupt.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <linux/module.h>

#include <bcmtypes.h>
#include <board.h>

#include <shared_utils.h>
#include <bcm_pinmux.h>
#include <bcm_otp.h>
#include <bcm_dgasp.h>
#include "bcm_dgasphw.h"
#include <bcm_ioremap_shared.h>
#include <linux/bcm_log.h>

/* DG controls */
#define DISABLE_DGINT_IF_ACTIVE_ON_BOOT 0 /* This flag will keep DG interrupt disabled if DG is active on bootup. */
                                          /* Use this flag for debug purposes only */
#define DG_IRQ_ACTIVE_WAIT_MS           2000

/*Dyinggasp callback*/
typedef void (*cb_dgasp_t)(void *context);
typedef void (*cb_dgasp_v2_t)(void *context, int event);
typedef struct _CB_DGASP__LIST
{
    struct list_head list;
    char name[IFNAMSIZ];
    cb_dgasp_t cb_dgasp_fn;
    cb_dgasp_v2_t cb_dgasp_v2_fn;
    void *context;
}CB_DGASP_LIST , *PCB_DGASP_LIST;

BCM_DGASPHW_CMN dgasphw_common;
static int dg_gpio = -1;

static irqreturn_t kerSysDyingGaspIsr(int irq, void * dev_id);

/* dgaspMutex Protects dyingGasp enable/disable functions */
/* also protects list add and delete, but is ignored during isr. */
static DEFINE_MUTEX(dgaspMutex);
static volatile int isDyingGaspTriggered = 0;
static CB_DGASP_LIST *g_cb_dgasp_list_head = NULL;

static int dg_enabled = 0;
static int dg_prevent_enable = 0;
static int dg_active_on_boot = 0;

/***************************************************************************
* Dying gasp ISR and functions.
***************************************************************************/

/* For any driver running on another cpu that needs to know if system is losing
   power */
int kerSysIsDyingGaspTriggered(void)
{
    return isDyingGaspTriggered;
}

void dg_set_gpio(int gpio)
{
    dg_gpio = gpio;
}
EXPORT_SYMBOL(dg_set_gpio);

static int _gpio_direction_output(unsigned int gpio, int val)
{
    int (*cb)(unsigned int, int) = (int (*)(unsigned int, int))bcmFun_get(BCM_FUN_ID_GPIO_DIR_OUT);
    BCM_ASSERT(cb);
    return cb(gpio, val);
}
#define gpio_direction_output  _gpio_direction_output

static irqreturn_t kerSysDyingGaspIsr(int irq, void * dev_id)
{
    struct list_head *pos;
    CB_DGASP_LIST *tmp = NULL, *dslCallBack = NULL, *lastCallBack = NULL;
    isDyingGaspTriggered = 1;

    if (unlikely(dg_gpio != -1))
    {
        gpio_direction_output(dg_gpio, 0);
    }

    /* Print D%G message */
    *(dgasphw_common.uart_data_reg) = 'D';
    *(dgasphw_common.uart_data_reg) = '%';
    *(dgasphw_common.uart_data_reg) = 'G';

    /* power down any block that is not needed for sending the message */

    // If configured, propogate dying gasp to other processors on the board
    if(dgasphw_common.dg_out_gpio)
    {
        // Dying gasp configured - set GPIO
        gpiod_direction_output(dgasphw_common.dg_out_gpio, 0);
    }

    list_for_each(pos, &g_cb_dgasp_list_head->list) {
        tmp = list_entry(pos, CB_DGASP_LIST, list);
	if (tmp->cb_dgasp_v2_fn)
        	tmp->cb_dgasp_v2_fn(tmp->context, DGASP_EVT_PWRDOWN);
    }

    list_for_each(pos, &g_cb_dgasp_list_head->list) {
        tmp = list_entry(pos, CB_DGASP_LIST, list);
        if (!strncmp(tmp->name, "dgpoll", 2)) {
            lastCallBack = tmp;
        }
        else if (strncmp(tmp->name, "dsl", 3)) {
	    if (tmp->cb_dgasp_v2_fn)
                tmp->cb_dgasp_v2_fn(tmp->context, DGASP_EVT_SENDMSG);
	    else
                tmp->cb_dgasp_fn(tmp->context);
        } else {
            dslCallBack = tmp;
        }
    }
    if (dslCallBack) {
	if (dslCallBack->cb_dgasp_v2_fn)
            dslCallBack->cb_dgasp_v2_fn(dslCallBack->context, DGASP_EVT_SENDMSG);
        else
            dslCallBack->cb_dgasp_fn(tmp->context);
    }
    if (lastCallBack) {
	if (lastCallBack->cb_dgasp_v2_fn)
            lastCallBack->cb_dgasp_v2_fn(lastCallBack->context, DGASP_EVT_SENDMSG);
        else
            lastCallBack->cb_dgasp_fn(tmp->context);
    }

    /* reset and shutdown system */

#if defined(CONFIG_BCM96XXX_WDT)
    /* Set WD to fire in 1 sec in case power is restored before reset occurs */
	bcmbca_wd_start(1000000);
#endif
    // If power is going down, nothing should continue!
    while (1) cpu_relax(); 
    return( IRQ_HANDLED );
}

void kerSysDisableDyingGaspInterrupt( void )
{
    mutex_lock(&dgaspMutex);

    if (!dg_enabled) {
        mutex_unlock(&dgaspMutex);
        return;
    }

    /* Call local hw disable routine */
    dgasp_hw_disable_irq();

    disable_irq(dgasphw_common.dg_irq);
    printk("DYING GASP IRQ Disabled\n");
    dg_enabled = 0;
    mutex_unlock(&dgaspMutex);
}
EXPORT_SYMBOL(kerSysDisableDyingGaspInterrupt);

static int dyingGaspIrqCheckEnable(void) 
{
    bool state = false;
    int ret = -1;
    int dg_postpone = 0;

    /* Get current state of Dying Gasp IRQ line (output of DG comparator) */
    ret = irq_get_irqchip_state(dgasphw_common.dg_irq, IRQCHIP_STATE_PENDING, &state);
    if (ret < 0) 
    {
        printk("DG IRQ status check failed, postponing IRQ enable!\n");
        dg_postpone = 1;
    }
    else
    {
        /* If DG IRQ line is currently asserted */
        if( state )
        {
            int msec = 0;

            /* Wait DG_IRQ_ACTIVE_WAIT_MS to see if line clears */
            while( state && (msec < DG_IRQ_ACTIVE_WAIT_MS) )
            {
                msleep(1);
                msec++;
                ret = irq_get_irqchip_state(dgasphw_common.dg_irq, IRQCHIP_STATE_PENDING, &state);
                if (ret < 0) 
                {
                    printk("DG IRQ status check failed, postponing IRQ enable!\n");
                    dg_postpone = 1;
                    break;
                }
            }
            if( !ret )
            {
                if( state )
                {
                    printk("DG still active after %d ms!\n", DG_IRQ_ACTIVE_WAIT_MS);
                    dg_active_on_boot = 1;
#if DISABLE_DGINT_IF_ACTIVE_ON_BOOT
                    dg_postpone = 1;
#else
                    dg_postpone = 0;
#endif
                }
                else 
                {
                    if( msec )
                        printk("DG cleared after %d ms\n", msec);                    

                    /* sleep for 10ms to avoid any transients */
                    msleep(10);
                }
            }
        }
    }

    /* Only enable DG IRQ if we are not explicilty prevented OR if we are not postponing */
    if( !dg_prevent_enable && !dg_postpone)
    {
        enable_irq(dgasphw_common.dg_irq);
    }
    else
    {
        printk("DYING GASP IRQ NOT Enabled! active:%d|prevent:%d|postpone:%d!\n", dg_active_on_boot, dg_prevent_enable, dg_postpone);
        ret = -1;
    }

    return ret;
}

void kerSysEnableDyingGaspInterrupt( void )
{
    static int dg_mapped = 0, ret=0;

    mutex_lock(&dgaspMutex);
    
    /* Ignore requests to enable DG if it is already enabled */
    if (dg_enabled) {
        printk("DYING GASP IRQ Already Enabled\n");
        mutex_unlock(&dgaspMutex);
        return;
    }

    if (dg_prevent_enable) 
    {
        printk("DYING GASP enabling postponed\n");
    } 
    else 
    {        
        /* Call local hw int enable routine */
        dgasp_hw_enable_irq(dg_mapped);

        if (dg_mapped) 
        { 
            /* Enable IRQ */
            if( !dyingGaspIrqCheckEnable() )
                printk("DYING GASP IRQ Enabled\n");
        }
        else 
        {
            /* Ensure that IRQ doesnt enable when we request it, we want to check
             * if DGASP is pending before we enable IRQ. Also enable UNLAZY irq
             * disabling, so that irq gets disabled at hw level when disable_irq 
             * is called */
            irq_set_status_flags(dgasphw_common.dg_irq, IRQ_NOAUTOEN);
            ret=request_irq(dgasphw_common.dg_irq, kerSysDyingGaspIsr, 0, "dying_gasp", (void*) 0);
            if(ret)
                printk(KERN_ERR "Failed to request dying_gasp irq %d\n", dgasphw_common.dg_irq);
            irq_set_status_flags(dgasphw_common.dg_irq, IRQ_DISABLE_UNLAZY);
            dg_mapped = 1;

            /* Enable IRQ */
            if( !dyingGaspIrqCheckEnable() )
                printk("DYING GASP IRQ Initialized and Enabled\n");
        }
        dg_enabled = 1;
    }
    mutex_unlock(&dgaspMutex);
}
EXPORT_SYMBOL(kerSysEnableDyingGaspInterrupt);

static void __exit kerSysDeinitDyingGaspHandler( void )
{
    struct list_head *pos;
    CB_DGASP_LIST *tmp;

    if(g_cb_dgasp_list_head == NULL)
        return ;

    list_for_each(pos, &g_cb_dgasp_list_head->list) {
        tmp = list_entry(pos, CB_DGASP_LIST, list);
        list_del(pos);
        kfree(tmp);
    }

    kfree(g_cb_dgasp_list_head);
    g_cb_dgasp_list_head = NULL;


} /* kerSysDeinitDyingGaspHandler */

static void __kerSysRegisterDyingGaspHandler(char *devname, void *cbfn, void *context, int version)
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
    if (version == 2)
        new_node->cb_dgasp_v2_fn = (cb_dgasp_v2_t)cbfn;
    else
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

void kerSysRegisterDyingGaspHandler(char *devname, void *cbfn, void *context)
{
    __kerSysRegisterDyingGaspHandler(devname, cbfn, context, 0);
}
EXPORT_SYMBOL(kerSysRegisterDyingGaspHandler);

void kerSysRegisterDyingGaspHandlerV2(char *devname, void *cbfn, void *context)
{
    __kerSysRegisterDyingGaspHandler(devname, cbfn, context, 2);
}
EXPORT_SYMBOL(kerSysRegisterDyingGaspHandlerV2);

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
EXPORT_SYMBOL(kerSysDeregisterDyingGaspHandler);

void kerSysDyingGaspIoctl(DGASP_ENABLE_OPTS opt)
{
    switch (opt)
    {
        case DG_ENABLE_FORCE:
             mutex_lock(&dgaspMutex);
             if( !dg_active_on_boot )
                 dg_prevent_enable = 0;
             mutex_unlock(&dgaspMutex);
             __attribute__((__fallthrough__));
        case DG_ENABLE:
             kerSysEnableDyingGaspInterrupt();
             break;
        case DG_DISABLE_PREVENT_ENABLE:
             mutex_lock(&dgaspMutex);
             dg_prevent_enable = 1;
             mutex_unlock(&dgaspMutex);
             __attribute__((__fallthrough__));
        case DG_DISABLE:
             kerSysDisableDyingGaspInterrupt();
             break;
        default:
             break;
    }

    return;
}
EXPORT_SYMBOL(kerSysIsDyingGaspTriggered);

static const struct of_device_id bcm_dying_gasp_of_match[] = {
    { .compatible = "brcm,dgasp-periph", .data = NULL, },
    { .compatible = "brcm,dgasp-afe", .data = NULL, },
    { .compatible = "brcm,dgasp-pmc", .data = NULL, },
    {},
};

static int bcm_dying_gasp_probe(struct platform_device *pdev)
{
    struct resource *res1;
    void __iomem *uart_dr_base;
    int dg_irq;
    const struct of_device_id *of_id =
            of_match_device(bcm_dying_gasp_of_match, &pdev->dev);

    dev_info(&pdev->dev, "Dying Gasp platform device %s matched\n", of_id->compatible);

    /* Set DG related global variables */
    mutex_lock(&dgaspMutex);
    
    /* Get common resources - Uart data register */
    res1 = platform_get_resource_byname(pdev, IORESOURCE_MEM, "uart-dr" );
    if ( !res1 ) 
    {
        dev_err(&pdev->dev, "Platform resource uart-dr is missing\n");
        mutex_unlock(&dgaspMutex);
        return -EINVAL;
    }
        
    /* Get gpio to signal off-chip peripherals */
    dgasphw_common.dg_out_gpio = devm_gpiod_get_optional(&pdev->dev, "dgasp", 0);

    uart_dr_base = devm_ioremap_shared_resource(&pdev->dev, res1);
    if (IS_ERR(uart_dr_base)) 
    {
        dev_err(&pdev->dev, "Ioremap failed for uart-dr\n");
        mutex_unlock(&dgaspMutex);
        return -EINVAL;
    }
    else
        dgasphw_common.uart_data_reg = (uint16_t *)uart_dr_base;

    /* Get common resources - IRQ */
    dg_irq = platform_get_irq(pdev, 0);
    if (dg_irq < 0) 
    {
        dev_err(&pdev->dev, "Failed to get Dying Gasp irq\n");
        mutex_unlock(&dgaspMutex);
        return -EINVAL;
    }
    else
        dgasphw_common.dg_irq = dg_irq;

    /* Init dg hw */
    if(dgasp_hw_init(pdev))
    {
        dev_err(&pdev->dev, "Dying Gasp HW init failed!!\n");
        mutex_unlock(&dgaspMutex);
        return -EINVAL;
    }
    
    /* Prevent DG enable if this is a battery enabled system */
    dg_prevent_enable = bcm_bmu_is_battery_enabled(); 
    
    mutex_unlock(&dgaspMutex);
    
    /* Enable DG Interrupt */
    kerSysEnableDyingGaspInterrupt();
    return 0;
}

static struct platform_driver bcm_dying_gasp_driver = {
	.probe = bcm_dying_gasp_probe,
	.driver = {
		.name = "bcm-dgasp",
		.of_match_table = bcm_dying_gasp_of_match,
	},
};

static int __init kerSysInitDyingGaspHandler( void )
{
    CB_DGASP_LIST *new_node;

    if( g_cb_dgasp_list_head != NULL) {
        printk("Error: kerSysInitDyingGaspHandler: list head is not null\n");
        return -EINVAL;
    }
    new_node= (CB_DGASP_LIST *)kmalloc(sizeof(CB_DGASP_LIST), GFP_KERNEL);
    memset(new_node, 0x00, sizeof(CB_DGASP_LIST));
    INIT_LIST_HEAD(&new_node->list);
    g_cb_dgasp_list_head = new_node;

    return platform_driver_register(&bcm_dying_gasp_driver);
} 

/* Early init/deinit */
subsys_initcall(kerSysInitDyingGaspHandler);
__exitcall(kerSysDeinitDyingGaspHandler);
MODULE_AUTHOR("Farhan Ali (farhan.ali@broadcom.com)");
MODULE_DESCRIPTION("Broadcom BCA Dying Gasp Driver");
MODULE_LICENSE("GPL v2");
