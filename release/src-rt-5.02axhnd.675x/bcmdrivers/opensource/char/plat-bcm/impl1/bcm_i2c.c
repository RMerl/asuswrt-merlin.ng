/*
<:copyright-BRCM:2017:DUAL/GPL:standard 

   Copyright (c) 2017 Broadcom 
   All Rights Reserved

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

/*
 ****************************************************************************
 * File Name  : bcm_i2c.c
 *
 * Description: This file contains the platform dependent code for detecting 
 *    and adding i2c device to the system
 ***************************************************************************/

#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>

#include <bcm_intr.h>
#include <boardparms.h>
#include <board.h>
#include <bcmsfp_i2c.h>

#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define MAX_I2C_ADDR 8
#if MAX_SFP_I2C_ADDR > MAX_I2C_ADDR
#error
#endif

#ifdef BCM_I2C_DEBUG
#define BCM_I2C_LOG_DEBUG(fmt, args...) printk("bcm_i2c: %s " fmt,  __FUNCTION__, ##args)
#else
#define BCM_I2C_LOG_DEBUG(fmt, args...)
#endif

#define BCM_I2C_LOG(fmt, args...) printk("bcm_i2c: " fmt, ##args)

enum {
    SFP_E_INSERT,
    SFP_E_REMOVE,
    SFP_E_TIMEOUT,
    SFP_E_RESCAN,
};

#define T_PROBE_INIT msecs_to_jiffies(500)
#define T_PROBE_RETRY msecs_to_jiffies(1000)
#define T_PROBE_RETRY_NUM 100

struct bcm_i2c_platform_data {
    int bus;
    struct i2c_client* i2c_clients[MAX_I2C_ADDR];
    int state;
    int present;
    unsigned short mod_abs_gpio;
    int polling;
    int retries_left;
    struct i2c_adapter *i2c;
    struct delayed_work dwork;
    struct delayed_work dwork_irq;
    struct mutex sm_mutex;
} bcm_i2c_platform_data;

static struct bcm_i2c_platform_data bcm_i2c_platform_data_enum[MAX_I2C_BUS_INCLUDE_MUX];
static int num_mux_i2c_bus;

#define SFP_I2C_ADDR 0x50

static struct i2c_board_info sfp_board_info[] = {
    {   /* SFP EEPROM client */
        I2C_BOARD_INFO("sfp_eeprom", SFP_I2C_EEPROM_ADDR),
    },
    {  	/* SFP diagnostic & monitor cient */
        I2C_BOARD_INFO("sfp_diag", SFP_I2C_DIAG_ADDR),
    },
    {	/* SFP Eth PHY client*/
        I2C_BOARD_INFO("sfp_phy", SFP_I2C_PHY_ADDR),
    },
};

#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM963158)
static struct i2c_board_info pmd_board_info[]  = {
    {   /* pmd reg client */
        I2C_BOARD_INFO("pmd_reg", PMD_I2C_REG_ADDR),
    },
    {  	/* SFP diagnostic & monitor cient */
        I2C_BOARD_INFO("pmd_iram", PMD_I2C_IRAM_ADDR),
    },
    {	/* SFP client on address 0x52, not sure what it is*/
        I2C_BOARD_INFO("pmd_dram", PMD_I2C_DRAM_ADDR),
    },
};
#endif

#define I2C_MUX_BUS 0
static struct i2c_board_info pca954x_board_info[] = {
    {   /* PCA9548 i2c mux */
        I2C_BOARD_INFO("pca9548", 0x70),
    },
};

static struct blocking_notifier_head sfp_notifier;

static int gponOpticsType = -1;

#ifdef BCM_I2C_DEBUG
static const char * const event_strings[] = {
    [SFP_E_INSERT] = "insert",
    [SFP_E_REMOVE] = "remove",
    [SFP_E_TIMEOUT] = "timeout",
    [SFP_E_RESCAN] = "rescan",
};

static const char *event_to_str(unsigned short event)
{
    if (event >= ARRAY_SIZE(event_strings))
        return "Unknown event";
    return event_strings[event];
}

static const char *const mod_state_strings[] = {
    [SFP_STATUS_REMOVED] = "empty",
    [SFP_STATUS_PROBING] = "probe",
    [SFP_STATUS_INSERTED] = "present",
    [SFP_STATUS_ERROR] = "error",
};

static const char *state_to_str(unsigned short mod_state)
{
    if (mod_state >= ARRAY_SIZE(mod_state_strings))
        return "Unknown module state";
    return mod_state_strings[mod_state];
}
#endif

static void remove_sfp_i2c_client(struct bcm_i2c_platform_data* psfp)
{
    int i;

    for (i = 0; i < MAX_I2C_ADDR; i++)
    {
        if (psfp->i2c_clients[i])
            i2c_unregister_device(psfp->i2c_clients[i]);

        psfp->i2c_clients[i] = NULL;
    }

    BCM_I2C_LOG("sfp removed: i2c devices unregistered from bus %d\n", psfp->bus);
}

static int i2c_read(struct i2c_adapter *adap, unsigned short addr, char buf[], int len)
{
    struct i2c_msg msg[2];
    int ret;

    msg[0].addr = msg[1].addr = addr;
    msg[0].flags = 0;

    msg[0].len = 1;
    buf[0] = 0x0; /* Offset to read from */
    msg[0].buf = buf;

    msg[1].flags = I2C_M_RD;
    msg[1].len = len;
    msg[1].buf = buf;

    ret = i2c_transfer(adap, msg, ARRAY_SIZE(msg));
    if (ret < 0)
        return ret;

    BCM_I2C_LOG_DEBUG("addr 0x%x ret %d reg 0 0x%x\n", addr, ret, buf[0]);

    return ret == ARRAY_SIZE(msg) ? len : 0;
}

static int sfp_probe(struct i2c_adapter *adap, unsigned short addr)
{
    char buf[2];
    int len;

    if (addr == SFP_I2C_PHY_ADDR)
        len = 2;
    else
        len = 1;

    return i2c_read(adap, addr, buf, len) > 0;
}

#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM963158)
static int is_pmd(char *buf, int len)
{
#define PMD_ID_LEN 4
    char pmd_device[][PMD_ID_LEN] = {
        { 0x08, 0x68, 0x90, 0x10 },
        { 0x08, 0x68, 0x91, 0x00 },
        {},
    };
    int i = 0;
    unsigned short rstn;
    
    if (BpGetGpioPmdReset(&rstn) != BP_SUCCESS)
        return 0;

    if (len < PMD_ID_LEN)
        return 0;

    do
    {
        if (!memcmp(pmd_device[i], buf, PMD_ID_LEN))
            return 1;
    } while (*pmd_device[++i]);

    return 0;
}
#endif

static int probe_i2c_and_register_clients(struct bcm_i2c_platform_data* psfp)
{
#define EEPROM_LEN_TO_READ 4
    char buf[EEPROM_LEN_TO_READ];
    int ret, i;
    struct i2c_adapter *i2c_adap = psfp->i2c;

    if (psfp->i2c_clients[0])
        BUG();

    if ((ret = i2c_read(i2c_adap, sfp_board_info[0].addr, buf, sizeof(buf))) < 0)
    {
        BCM_I2C_LOG_DEBUG("bus %d: failed to read EEPROM %d\n", psfp->bus, ret);
        return -EAGAIN;
    }

    /* TRX type */
    if (buf[0] == 0)
        return -EAGAIN;

#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM963158)
    if (is_pmd(buf, sizeof(buf)))
    {
        for (i = 0; i < ARRAY_SIZE(pmd_board_info) && i < MAX_I2C_ADDR; i++)
        {
            psfp->i2c_clients[i] = i2c_new_device(i2c_adap, &pmd_board_info[i]);
            if (psfp->i2c_clients[i])
                BCM_I2C_LOG("sfp i2c device found registered for bus %d address 0x%x\n", psfp->bus, (pmd_board_info[i].addr<<1));
        }

        gponOpticsType = BCM_I2C_PON_OPTICS_TYPE_PMD;
    }
    else
#endif
    {
        for (i = 0; i < ARRAY_SIZE(sfp_board_info) && i < MAX_I2C_ADDR; i++)
        {
            psfp->i2c_clients[i] = i2c_new_probed_device(i2c_adap, &sfp_board_info[i], I2C_ADDRS(sfp_board_info[i].addr), sfp_probe);
            if (psfp->i2c_clients[i])
                BCM_I2C_LOG("sfp i2c device found registered for bus %d address 0x%x\n", psfp->bus, (sfp_board_info[i].addr<<1));
        }
    }

    if (!psfp->i2c_clients[0])
        return -EAGAIN;

    return 0;
}

#ifdef CONFIG_BP_PHYS_INTF
static int find_sfp_i2c_bus_in_intf(int bus, unsigned short intf_type)
{
    int i, intf_num;
    unsigned short mgmt_type, mgmt_bus_num;

    if((intf_num = BpGetPhyIntfNumByType(intf_type))) {
        for( i = 0; i < intf_num; i++ ) {
            if( BpGetIntfMgmtType(intf_type, i, &mgmt_type ) == BP_SUCCESS && mgmt_type == BP_INTF_MGMT_TYPE_I2C ) {
                if( BpGetIntfMgmtBusNum(intf_type, i, &mgmt_bus_num) == BP_SUCCESS && mgmt_bus_num == bus ) {
                    return i; 
                }
            }
        }
    }

    return -1;
}
#else
#define BP_INTF_TYPE_xPON 1 /* Dummy value so code below will have less #ifdef */
#endif

static int get_i2c_bus_num(int idx, int *bus)
{
#ifdef CONFIG_BP_PHYS_INTF
    *bus = 0;
    if( BpGetIntfPortNum(BP_INTF_TYPE_I2C, idx, (unsigned short*)bus) != BP_SUCCESS ) {
        BCM_I2C_LOG("No port/bus number define i2c interface %d!!!\n", idx);
        return -1;
    }
#else
    *bus = idx;
#endif
    return 0;
}

/* return the number of i2c buses is used in the design */
static int get_num_of_i2c_bus(void) 
{
    int num_i2c_bus;

#ifdef CONFIG_BP_PHYS_INTF
    /* always return the max supported i2c bus number by the chip instead of the actual bus used:
     * num_i2c_bus = BpGetPhyIntfNumByType(BP_INTF_TYPE_I2C);
     * due to PON device does not support multiple i2cs buses */
    num_i2c_bus = MAX_I2C_BUS;
#else
    /* assume only one i2c bus for the old style board parameter */
    num_i2c_bus = 1;
#endif

    return num_i2c_bus + num_mux_i2c_bus;
}

static int is_sfp_plugin(struct bcm_i2c_platform_data *psfp)
{
    int gpioActHigh, plugin = 0;
    unsigned int value;
    unsigned short gpio = psfp->mod_abs_gpio;
    
    if (gpio == BP_GPIO_NONE)
    {
        if (psfp->polling)
            return sfp_probe(psfp->i2c, SFP_I2C_ADDR); /* Poll I2C */

        return 1; /* For SFF */
    }

    gpioActHigh = gpio & BP_ACTIVE_LOW ? 0 : 1;
    value = kerSysGetGpioValue(gpio);

    if ((value&&gpioActHigh) || (!value&&!gpioActHigh))
        plugin = 1;

    return plugin;
}

static void sfp_sm_next(struct bcm_i2c_platform_data *psfp, int state, int timeout)
{
    BCM_I2C_LOG_DEBUG("bus %d SM next: new state %s timeout %d\n", psfp->bus, state_to_str(state), timeout);

    psfp->state = state;
    if (timeout)
        mod_delayed_work(system_power_efficient_wq, &psfp->dwork, timeout); 
    else
        cancel_delayed_work(&psfp->dwork);
}

static void sfp_sm_event(struct bcm_i2c_platform_data *psfp, int event)
{
    mutex_lock(&psfp->sm_mutex);
    BCM_I2C_LOG_DEBUG("bus %d SM event: current state %s event %s\n", psfp->bus, state_to_str(psfp->state), event_to_str(event));

    switch (psfp->state)
    {
        case SFP_STATUS_PROBING:
            if (event == SFP_E_REMOVE)
            {
                sfp_sm_next(psfp, SFP_STATUS_REMOVED, 0);
            }
            if (event == SFP_E_TIMEOUT)
            {
                int val = probe_i2c_and_register_clients(psfp);

                psfp->retries_left--;
                if (val == 0)
                {
                    sfp_sm_next(psfp, SFP_STATUS_INSERTED, 0);
                    blocking_notifier_call_chain(&sfp_notifier, SFP_STATUS_INSERTED, (void*)((uintptr_t)&psfp->bus));
                }
                else if (val != -EAGAIN)
                {
                    /* Error, do nothing */
                    sfp_sm_next(psfp, SFP_STATUS_ERROR, 0);
                }
                else if (psfp->retries_left <= 0)
                {
                    sfp_sm_next(psfp, SFP_STATUS_REMOVED, 0);
                    BCM_I2C_LOG("bus %d: Failed to detect SFP: %d retries exhausted\n", psfp->bus, T_PROBE_RETRY_NUM);
                }
                else
                    sfp_sm_next(psfp, SFP_STATUS_PROBING, T_PROBE_RETRY);
            }
            break;
        case SFP_STATUS_INSERTED:
        case SFP_STATUS_ERROR:
            if (event == SFP_E_REMOVE || event == SFP_E_RESCAN)
            {
                sfp_sm_next(psfp, SFP_STATUS_REMOVED, 0);
                remove_sfp_i2c_client(psfp);
                blocking_notifier_call_chain(&sfp_notifier, SFP_STATUS_REMOVED, (void*)((uintptr_t)&psfp->bus));
            }
            if (event == SFP_E_RESCAN)
            {
                psfp->retries_left = T_PROBE_RETRY_NUM;
                sfp_sm_next(psfp, SFP_STATUS_PROBING, T_PROBE_INIT);
                BCM_I2C_LOG("bus %d: module rescanning\n", psfp->bus);
            }
            break;
        default:
            if (event == SFP_E_INSERT)
            {
                psfp->retries_left = T_PROBE_RETRY_NUM;
                sfp_sm_next(psfp, SFP_STATUS_PROBING, T_PROBE_INIT);
                BCM_I2C_LOG("bus %d: module inserted\n", psfp->bus);
            }
            break;
    }

    mutex_unlock(&psfp->sm_mutex);
}

static void check_sfp_present(struct bcm_i2c_platform_data *psfp)
{
    int new_present, present_changed;

    new_present = is_sfp_plugin(psfp);
    present_changed = new_present != psfp->present;
    psfp->present = new_present;
                
    if (present_changed)
        sfp_sm_event(psfp, new_present ? SFP_E_INSERT : SFP_E_REMOVE);
}

static void sfp_cb(struct work_struct *work_arg)
{
    struct bcm_i2c_platform_data *psfp = container_of(work_arg, struct bcm_i2c_platform_data, dwork.work);

    sfp_sm_event(psfp, SFP_E_TIMEOUT);
}

static void sfp_poll_all(void)
{
    struct bcm_i2c_platform_data *psfp;
    int i;

    for (i = 0; i < ARRAY_SIZE(bcm_i2c_platform_data_enum); i++)
    {
        psfp = &bcm_i2c_platform_data_enum[i];
        if (!psfp->i2c)
            continue;

        /* It is not possible to poll a mux device */
        if (num_mux_i2c_bus > 0 && psfp->bus == I2C_MUX_BUS)
            continue;

        if (psfp->polling)
            check_sfp_present(psfp);
    }
}

static void sfp_cb_irq(struct work_struct *work_arg)
{
    struct bcm_i2c_platform_data *psfp = container_of(work_arg, struct bcm_i2c_platform_data, dwork_irq.work);

    /* ISR triggered connected to all SFP cages, so i2c poll to check which one is connected. */
    if (num_mux_i2c_bus > 0 && psfp->bus == I2C_MUX_BUS)
        sfp_poll_all();
    else
        check_sfp_present(psfp);
}

static irqreturn_t sfp_isr(int irq, void *arg)
{
    struct bcm_i2c_platform_data *psfp = (struct bcm_i2c_platform_data *)((uintptr_t)arg);
    
    BCM_I2C_LOG_DEBUG("sfp_isr interrupt called irq %d for i2c bus %d!\n", irq, psfp->bus);

    mod_delayed_work(system_power_efficient_wq, &psfp->dwork_irq, msecs_to_jiffies(1)); 
    
#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
    BcmHalInterruptEnable(irq);
#else
    BcmHalExternalIrqClear(irq);
#endif

    return IRQ_HANDLED;
}

static int sfp_polling_func(void * arg) 
{
    while (1)
    {
        sfp_poll_all();
        msleep(800);
    }

    return 0;
}

static int create_polling_thread(void)
{
    static struct task_struct *thread;

    if (thread)
        return 0;

    thread = kthread_run(sfp_polling_func, NULL, "sfp_polling");
    if (!thread)
    {
        BCM_I2C_LOG("Failed to polling create thread.\n");
        return -1;
    }
   
    return 0;
}

static int prepare_pmd_detection(int bus)
{
#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM963158)
    unsigned short intf_type, rstn;
#ifdef CONFIG_BP_PHYS_INTF
    int intf_idx;
    
    if (bcm_i2c_sfp_get_intf(bus, &intf_type, &intf_idx) != 0)
        return -1;
#else
    intf_type = BP_INTF_TYPE_xPON;
#endif

    if (intf_type != BP_INTF_TYPE_xPON)
        return 0;

    /* PMD device need taken out of reset */
    if (BpGetGpioPmdReset(&rstn) == BP_SUCCESS)
    {
        kerSysSetGpioDir(rstn);
        kerSysSetGpioState(rstn, kGpioActive);
        kerSysSetGpioState(rstn, kGpioInactive);
    }
#endif

    return 0;
}

static int boarparms_get(int bus, unsigned short *irq, unsigned short *mod_abs_gpio)
{
    unsigned short intf_type;
    int rc;
#ifdef CONFIG_BP_PHYS_INTF
    int intf_idx;
#endif

#ifdef CONFIG_BP_PHYS_INTF
    if (bcm_i2c_sfp_get_intf(bus, &intf_type, &intf_idx) != 0)
        return -1;
        
    BCM_I2C_LOG("i2c bus %d used by intf type %d idx %d\n", bus, intf_type, intf_idx);
    /* check if SFP use interrupt */
    if (BpGetOpticalModulePresenceExtIntr(intf_type, intf_idx, irq) == BP_SUCCESS && *irq != BP_EXT_INTR_NONE) {
#else
    intf_type = BP_INTF_TYPE_xPON;
    if (BpGetOpticalModulePresenceExtIntr(irq) == BP_SUCCESS && *irq != BP_EXT_INTR_NONE) {
#endif
#ifdef CONFIG_BP_PHYS_INTF
       rc = BpGetOpticalModulePresenceExtIntrGpio(intf_type, intf_idx, mod_abs_gpio);
#else
       rc = BpGetOpticalModulePresenceExtIntrGpio(mod_abs_gpio);
#endif
    }
    else
    {
        /* check if it has sfp mod_abs gpio pin for polling */
#ifdef CONFIG_BP_PHYS_INTF
        rc = BpGetSfpModDetectGpio(intf_type, intf_idx, mod_abs_gpio);
#else
        rc = BpGetSfpDetectGpio(mod_abs_gpio);
#endif
    }

    if (rc != BP_SUCCESS)
        *mod_abs_gpio = BP_GPIO_NONE;

    return 0;
}

static int sfp_detect(int bus)
{
    struct bcm_i2c_platform_data *psfp;
    unsigned short irq = BP_NOT_DEFINED;
    struct i2c_adapter *i2c = i2c_get_adapter(bus);

    if (!i2c)
        return -1;

    psfp = &bcm_i2c_platform_data_enum[bus];

    mutex_init(&psfp->sm_mutex);
    INIT_DELAYED_WORK(&psfp->dwork, sfp_cb);
    INIT_DELAYED_WORK(&psfp->dwork_irq, sfp_cb_irq);
    psfp->state = SFP_STATUS_REMOVED;
    psfp->bus = bus;
    psfp->mod_abs_gpio = BP_GPIO_NONE;

    /* In case mux exists, must poll devices via i2c */
    if (num_mux_i2c_bus > 0)
    {
        psfp->polling = 1;
        create_polling_thread();
        goto exit;
    }

    boarparms_get(bus, &irq, &psfp->mod_abs_gpio);
    if (psfp->mod_abs_gpio != BP_GPIO_NONE)
        kerSysSetGpioDirInput(psfp->mod_abs_gpio);

    if (psfp->mod_abs_gpio != BP_GPIO_NONE && !is_sfp_plugin(psfp) && sfp_probe(i2c, SFP_I2C_ADDR))
    {
        /* Workaround for BCM96858XREF boardid, which has either SFF or SFP:
         * meaning it has OpticalModulePresence declared in SFF boardid */
        BCM_I2C_LOG("i2c bus %d mod_abs_gpio %u value low, but found SFP on i2c bus!\n", bus, psfp->mod_abs_gpio & BP_GPIO_NUM_MASK);
        psfp->mod_abs_gpio = BP_GPIO_NONE;
        check_sfp_present(psfp);
    }
    else if (irq != BP_NOT_DEFINED && psfp->mod_abs_gpio != BP_GPIO_NONE)
    {
        BCM_I2C_LOG("i2c bus %d sfp detection using interrupt on gpio %d\n", bus, psfp->mod_abs_gpio & BP_GPIO_NUM_MASK);
        ext_irq_connect(irq, (void *)((uintptr_t)psfp), (FN_HANDLER)sfp_isr);
        check_sfp_present(psfp);
    }
    else if (psfp->mod_abs_gpio == BP_GPIO_NONE && gponOpticsType != -1)
    {
        /* PON board without mod abs pin must be using SFF/PMD only */
        BCM_I2C_LOG("i2c bus %d PON optical interface soldered on board\n", bus);
        check_sfp_present(psfp);
    }
    else if (psfp->mod_abs_gpio != BP_GPIO_NONE)
    {
        BCM_I2C_LOG("i2c bus %d polling mode (mod_abs_gpio %u)\n", bus, psfp->mod_abs_gpio & BP_GPIO_NUM_MASK);
        psfp->polling = 1;
        create_polling_thread();
    }

exit:
    psfp->i2c = i2c;
    return 0;
}

#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM963158)
static int proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "%d\n", gponOpticsType);
    return 0;
}

static int proc_open(struct inode *inode, struct  file *file)
{
    return single_open(file, proc_show, NULL);
}

static const struct file_operations proc_fops = {
    .owner = THIS_MODULE,
    .open = proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

static __init int create_proc_entry(void)
{
    if (!proc_create(BCM_I2C_PROC_DIR, 0, NULL, &proc_fops))
    {
        BCM_I2C_LOG("Failed to create proc entry\n");
        return -ENOMEM;
    }

    return 0;
}
#endif

static int probe_i2c_mux(void)
{
    struct bcm_i2c_platform_data *pdata = &bcm_i2c_platform_data_enum[I2C_MUX_BUS];
    struct i2c_adapter *i2c_adap = i2c_get_adapter(I2C_MUX_BUS);

    if (!i2c_adap)
        return -1;

    pdata->i2c_clients[0] = i2c_new_probed_device(i2c_adap, pca954x_board_info, I2C_ADDRS(0x70, 0x72), sfp_probe);
    i2c_put_adapter(i2c_adap);
    if (!pdata->i2c_clients[0])
        return 0;

    /* Must be assigned before calling sfp_detect */
    num_mux_i2c_bus = PCA954x_NUM_DEVICES;
    if (sfp_detect(I2C_MUX_BUS))
        return -1;
    
    BCM_I2C_LOG("registered i2c mux on bus 0: client = %p using addr 0x%x\n", pdata->i2c_clients[0], pdata->i2c_clients[0]->addr);

    return 1;
}

static __init int bcm_add_i2c(void)
{
    int ret = -1, i, num_i2c_bus, bus;

    BLOCKING_INIT_NOTIFIER_HEAD(&sfp_notifier);
    
#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM963158)
    gponOpticsType = BCM_I2C_PON_OPTICS_TYPE_LEGACY;

    if (create_proc_entry())
        goto exit;
#endif

    if (probe_i2c_mux() < 0)
        goto exit;

    num_i2c_bus = get_num_of_i2c_bus();
    i = num_mux_i2c_bus > 0 ? 1 : 0; /* Skip probing bus 0 since mux is connected to it */
    for (; i < num_i2c_bus; i++)
    {
        if (get_i2c_bus_num(i, &bus))
            continue;

        if (bus < 0 || bus >= ARRAY_SIZE(bcm_i2c_platform_data_enum))
            continue;

        if (prepare_pmd_detection(bus))
            goto exit;

        if (sfp_detect(bus))
            goto exit;
    }

    ret = 0;
exit:
    if (ret)
        BCM_I2C_LOG("Error in loading module\n");
    
    return ret;
}

int bcm_i2c_sfp_register_notifier(struct notifier_block* nb)
{
    return blocking_notifier_chain_register(&sfp_notifier, nb);
}
EXPORT_SYMBOL(bcm_i2c_sfp_register_notifier);

int bcm_i2c_sfp_unregister_notifier(struct notifier_block* nb)
{
    return blocking_notifier_chain_unregister(&sfp_notifier, nb);
}
EXPORT_SYMBOL(bcm_i2c_sfp_unregister_notifier);

int bcm_i2c_sfp_get_status(int bus)
{
    int status;
    struct bcm_i2c_platform_data *psfp;

    if (bus >= ARRAY_SIZE(bcm_i2c_platform_data_enum))
        return SFP_STATUS_INVALID;

    psfp = &bcm_i2c_platform_data_enum[bus];
    if (!psfp->i2c)
        return SFP_STATUS_INVALID;

    mutex_lock(&psfp->sm_mutex);
    status = bcm_i2c_platform_data_enum[bus].state;
    mutex_unlock(&psfp->sm_mutex);

    return status;
}
EXPORT_SYMBOL(bcm_i2c_sfp_get_status);

#ifdef CONFIG_BP_PHYS_INTF
/*Given the interface type and index, find the i2c bus number */
int bcm_i2c_sfp_get_bus_num(unsigned short intf_type, int intf_idx, int *bus)
{
    int ret = -1;
    unsigned short mgmt_type, bus_num;

    if( BpGetIntfMgmtType(intf_type, intf_idx, &mgmt_type ) == BP_SUCCESS && 
         mgmt_type == BP_INTF_MGMT_TYPE_I2C ) 
    {
        if( BpGetIntfMgmtBusNum(intf_type, intf_idx, &bus_num) == BP_SUCCESS ) {
            *bus = bus_num;
            ret = 0;
        }
    }

    return ret;
}
EXPORT_SYMBOL(bcm_i2c_sfp_get_bus_num);

/* Given the i2c bus number, find interface type and index */
int bcm_i2c_sfp_get_intf(int bus, unsigned short* intf_type, int* intf_idx)
{
    /* Only xPON, SGMII interface has sfp */
    *intf_idx = find_sfp_i2c_bus_in_intf(bus, BP_INTF_TYPE_xPON);
    if (*intf_idx != - 1)
    {
        *intf_type = BP_INTF_TYPE_xPON;
        return 0;
    }

    *intf_idx = find_sfp_i2c_bus_in_intf(bus, BP_INTF_TYPE_SGMII);
    if (*intf_idx != - 1)
    {
        *intf_type = BP_INTF_TYPE_SGMII;
        return 0;
    }

    return -1;
}
EXPORT_SYMBOL(bcm_i2c_sfp_get_intf);
#endif

int bcm_i2c_pon_optics_type_get(unsigned short *pusValue)
{
    *pusValue = gponOpticsType;
    if (gponOpticsType == -1)
        return BP_VALUE_NOT_DEFINED;

    return BP_SUCCESS;
}
EXPORT_SYMBOL(bcm_i2c_pon_optics_type_get);

int bcm_i2c_pon_optics_sd_get(unsigned char * sig_det)
{
    int ret = BP_SUCCESS;
    static unsigned short los_gpio = BP_NOT_DEFINED;
    
    if (los_gpio == BP_NOT_DEFINED)
    {
        ret = BpGetWanSignalDetectedGpio(&los_gpio);
        if (ret == BP_SUCCESS)
            kerSysSetGpioDirInput(los_gpio);
        else
            return ret;
    }
    
    *sig_det = !kerSysGetGpioValue(los_gpio);
    return BP_SUCCESS;
}
EXPORT_SYMBOL(bcm_i2c_pon_optics_sd_get);

static void bcm_remove_i2c(void)
{
    int i;
    struct bcm_i2c_platform_data *psfp;
    struct i2c_adapter *i2c;

#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM963158)
    remove_proc_entry(BCM_I2C_PROC_DIR, NULL);
#endif

    /* TODO: remove poll thread */

    for (i = 0; i < ARRAY_SIZE(bcm_i2c_platform_data_enum); i++)
    {
        psfp = &bcm_i2c_platform_data_enum[i];
        i2c = psfp->i2c;
        if (!i2c)
            continue;
        
        psfp->i2c = NULL;
        sfp_sm_event(psfp, SFP_E_REMOVE);
        i2c_put_adapter(i2c);
    }
}

int bcm_i2c_sfp_rescan(int bus)
{
    struct bcm_i2c_platform_data *psfp;

    BCM_I2C_LOG_DEBUG("rescan on bus %d", bus);

    if (bus >= ARRAY_SIZE(bcm_i2c_platform_data_enum))
        return -1;

    psfp = &bcm_i2c_platform_data_enum[bus];
    if (!psfp->i2c)
        return -1;

    sfp_sm_event(psfp, SFP_E_RESCAN);

    return 0;
}
EXPORT_SYMBOL(bcm_i2c_sfp_rescan);

static int _optics_en(int enable, int tx)
{
    unsigned short gpio;
    int rc;

    if (tx)
        rc = BpGetPonTxEnGpio(&gpio);
    else
        rc = BpGetPonRxEnGpio(&gpio);

    if (rc != BP_SUCCESS)
    {
        BCM_I2C_LOG("Transceiver %cX %s failed: GpioPon%cxEn is not defined\n", tx ? 'T' : 'R',  enable ? "Enable" : "Disable", tx ? 'T' : 'R');
        return -1;
    }

    if (gpio == BP_GPIO_NONE)
        return 0;

    kerSysSetGpioDir(gpio);
    kerSysSetGpioState(gpio, enable ? kGpioActive : kGpioInactive);
    BCM_I2C_LOG_DEBUG("Transceiver %cX %s\n", tx ? 'T' : 'R', enable ? "Enable" : "Disable");

    return 0;
}

int bcm_i2c_optics_rx_control(int enable)
{
    return _optics_en(enable, 0);
}
EXPORT_SYMBOL(bcm_i2c_optics_rx_control);

int bcm_i2c_optics_tx_control_get(int *enable)
{
    unsigned short gpio;
    int rc;

    rc = BpGetPonTxEnGpio(&gpio);
    if (rc != BP_SUCCESS)
    {
        BCM_I2C_LOG("Transceiver TX get failed: GpioPontxEn is not defined\n");
        return -1;
    }

    if (gpio == BP_GPIO_NONE)
        return -1;

    kerSysSetGpioDir(gpio);
    *enable = (kerSysGetGpioValue(gpio) == kGpioActive)? BCM_I2C_OPTICS_ENABLE : BCM_I2C_OPTICS_DISABLE;

    return 0;
}
EXPORT_SYMBOL(bcm_i2c_optics_tx_control_get);

int bcm_i2c_optics_tx_control(int enable)
{
    return _optics_en(enable, 1);
}
EXPORT_SYMBOL(bcm_i2c_optics_tx_control);

module_init(bcm_add_i2c);
module_exit(bcm_remove_i2c);
MODULE_LICENSE("GPL");

