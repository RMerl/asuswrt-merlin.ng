/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

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

/*
 *  Created on: Dec 2015
 *      Author: yuval.raviv@broadcom.com
 */

#include "phy_drv.h"
#include "phy_drv_mii.h"
#ifdef PHY_LINK_CHANGE_NOTIFY
#include <linux/workqueue.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/bcm_log.h>
#endif
#ifdef PHY_PROC_FS
#include "proc_cmd.h"
#endif
#ifdef MACSEC_SUPPORT
#include "phy_macsec_api.h"
#include "phy_macsec_common.h"
#endif
#include "crossbar_dev.h"

#define MAX_PHY_DEVS 31     /* value should be < 32 */

extern phy_drv_t phy_drv_egphy;
extern phy_drv_t phy_drv_sgmii;
extern phy_drv_t phy_drv_ext1;
extern phy_drv_t phy_drv_ext2;
extern phy_drv_t phy_drv_ext3;
extern phy_drv_t phy_drv_mptwo;
extern phy_drv_t phy_drv_shortfin;
extern phy_drv_t phy_drv_shasta;
extern phy_drv_t phy_drv_wan_ae;
extern phy_drv_t phy_drv_dsl_gphy;      //TODO_DSL? create 4 different phy types for 138,148,4908, don't know if above types can be reused ...
extern phy_drv_t phy_drv_serdes_138class;
extern phy_drv_t phy_drv_serdes_158class;
extern phy_drv_t phy_drv_serdes_146class;
extern phy_drv_t phy_drv_serdes_6756class;
extern phy_drv_t phy_drv_crossbar;
extern phy_drv_t phy_drv_mac2mac;
extern phy_drv_t phy_drv_pon;

phy_drv_t *phy_drivers[PHY_TYPE_MAX] = {};

#if defined(CONFIG_BCM_ETH_PWRSAVE)
int apd_enabled = 1;
#else
int apd_enabled = 0;
#endif
EXPORT_SYMBOL(apd_enabled);

#if defined(CONFIG_BCM_ENERGY_EFFICIENT_ETHERNET)
int eee_enabled = 1;
#else
int eee_enabled = 0;
#endif
EXPORT_SYMBOL(eee_enabled);

int phy_speed_max = PHY_SPEED_AUTO;
EXPORT_SYMBOL(phy_speed_max);

int phy_driver_set(phy_drv_t *phy_drv)
{
    if (phy_drivers[phy_drv->phy_type])
    {
        printk("Failed adding phy driver %s: already set\n", phy_drv->name);
        return -1;
    }
    else
    {
        phy_drivers[phy_drv->phy_type] = phy_drv;
        return 0;
    }
}
EXPORT_SYMBOL(phy_driver_set);

static int phy_drv_init(phy_drv_t *phy_drv)
{
    if (phy_drv->initialized)
        return 0;

    if (!phy_drv->drv_init)
        return 0;

    return phy_drv->drv_init(phy_drv);
}

int phy_driver_init(phy_type_t phy_type)
{
    phy_drv_t *phy_drv;

    if (!(phy_drv = phy_drivers[phy_type]))
        return 0;

    return phy_drv_init(phy_drv);
}
EXPORT_SYMBOL(phy_driver_init);

int phy_drivers_set(void)
{
    int ret = 0;

#ifdef PHY_EGPHY
    ret |= phy_driver_set(&phy_drv_egphy);
#endif
#ifdef PHY_SGMII
    ret |= phy_driver_set(&phy_drv_sgmii);
#endif
#ifdef PHY_EXT1
    ret |= phy_driver_set(&phy_drv_ext1);
#endif
#ifdef PHY_EXT2
    ret |= phy_driver_set(&phy_drv_ext2);
#endif
#ifdef PHY_EXT3
    ret |= phy_driver_set(&phy_drv_ext3);
#endif
#ifdef PHY_MPTWO
    ret |= phy_driver_set(&phy_drv_mptwo);
#endif
#ifdef PHY_SHORTFIN
    ret |= phy_driver_set(&phy_drv_shortfin);
#endif
#ifdef PHY_SHASTA
    ret |= phy_driver_set(&phy_drv_shasta);
#endif
#ifdef PHY_WAN_AE
    ret |= phy_driver_set(&phy_drv_wan_ae);
#endif
#ifdef PHY_DSL_GPHY
    ret |= phy_driver_set(&phy_drv_dsl_gphy);
#endif
#ifdef PHY_138CLASS_SERDES
    ret |= phy_driver_set(&phy_drv_serdes_138class);
#endif
#ifdef PHY_158CLASS_SERDES
    ret |= phy_driver_set(&phy_drv_serdes_158class);
#endif
#ifdef PHY_146CLASS_SERDES
    ret |= phy_driver_set(&phy_drv_serdes_146class);
#endif
#ifdef PHY_6756CLASS_SERDES
    ret |= phy_driver_set(&phy_drv_serdes_6756class);
#endif
#ifdef PHY_CROSSBAR
    ret |= phy_driver_set(&phy_drv_crossbar);
#endif
#ifdef PHY_M2M
    ret |= phy_driver_set(&phy_drv_mac2mac);
#endif
#ifdef PHY_PON
    ret |= phy_driver_set(&phy_drv_pon);
#endif

    return ret;
}
EXPORT_SYMBOL(phy_drivers_set);

int phy_drivers_init(void)
{
    int ret = 0;

#ifndef CONFIG_BRCM_QEMU
    ret |= phy_driver_init(PHY_TYPE_EGPHY);
    ret |= phy_driver_init(PHY_TYPE_DSL_GPHY);
    ret |= phy_driver_init(PHY_TYPE_SGMII);
    ret |= phy_driver_init(PHY_TYPE_EXT1);
    ret |= phy_driver_init(PHY_TYPE_EXT2);
    ret |= phy_driver_init(PHY_TYPE_EXT3);
    ret |= phy_driver_init(PHY_TYPE_MPTWO);
    ret |= phy_driver_init(PHY_TYPE_SHORTFIN);
    ret |= phy_driver_init(PHY_TYPE_SHASTA);
    ret |= phy_driver_init(PHY_TYPE_WAN_AE);
    ret |= phy_driver_init(PHY_TYPE_CROSSBAR);
    ret |= phy_driver_init(PHY_TYPE_MAC2MAC);
    ret |= phy_driver_init(PHY_TYPE_PON);
#endif
    return ret;
}
EXPORT_SYMBOL(phy_drivers_init);

int phy_dev_prog(phy_dev_t *phy_dev, prog_entry_t *prog_entry)
{
    int ret = 0;

#ifdef DEBUG
    printk("Configuring PHY addr 0x%x:\n", phy_dev->addr);
#endif

    while (prog_entry->desc)
    {
#ifdef DEBUG
        printk("0x%04x=0x%04x %s\n", prog_entry->reg, prog_entry->val, prog_entry->desc);
#endif

        if ((ret = phy_dev_write(phy_dev, prog_entry->reg, prog_entry->val)))
            break;

        prog_entry++;
    }

    return ret;
}

int phy_dev_prog_ext(phy_dev_t *phy_dev, prog_entry_ext_t *prog_entry)
{
    int ret = 0;

#ifdef DEBUG
    printk("Configuring PHY addr 0x%x:\n", phy_dev->addr);
#endif

    while (prog_entry->desc && strlen(prog_entry->desc))
    {
#ifdef DEBUG
        printk("%2d.0x%04x=0x%04x %s\n", prog_entry->dev, prog_entry->reg, prog_entry->val, prog_entry->desc);
#endif

        switch (prog_entry->type)
        {
        case (PROG_TYPE_ENTRY):
            ret = phy_dev_c45_write_mask(phy_dev, prog_entry->dev, prog_entry->reg, prog_entry->mask, 0, prog_entry->val);
            break;
        case (PROG_TYPE_SEQUENCE):
            ret  = phy_dev_prog_ext(phy_dev, (prog_entry_ext_t *)prog_entry->param);
            break;
        case (PROG_TYPE_UDELAY):
            udelay((unsigned long)prog_entry->param);
            break;
        default:
            break;
        }

        if (!ret)
            prog_entry++;
    }

    return ret;
}

char * usxgmii_m_type_strs[] = {
    [USXGMII_S] = "none",
    [USXGMII_M_10G_S] = "10G-SXGMII", [USXGMII_M_10G_D] = "10G-DXGMII", [USXGMII_M_10G_Q] = "10G-QXGMII",
    [USXGMII_M_5G_S]  = "5G-SXGMII",  [USXGMII_M_5G_D]  = "5G-DXGMII",
    [USXGMII_M_2G5_S] = "2.5G-SXGMII",
};

char *phy_dev_mii_type_to_str(phy_mii_type_t mii_type)
{
	switch (mii_type)
    {
	case PHY_MII_TYPE_MII:
        return "MII";
	case PHY_MII_TYPE_TMII:
        return "TMII";
	case PHY_MII_TYPE_GMII:
        return "GMII";
	case PHY_MII_TYPE_RGMII:
        return "RGMII";
	case PHY_MII_TYPE_SGMII:
        return "SGMII";
	case PHY_MII_TYPE_HSGMII:
        return "HSGMII";
	case PHY_MII_TYPE_XFI:
        return "XFI";
	case PHY_MII_TYPE_SERDES:
        return "SERDES";
	default:
		return "Unknown";
	}
}
EXPORT_SYMBOL(phy_dev_mii_type_to_str);

char *phy_dev_speed_to_short_str(phy_speed_t speed)
{
	switch (speed)
    {
	case PHY_SPEED_10:
		return "10M";
	case PHY_SPEED_100:
		return "100M";
	case PHY_SPEED_1000:
		return "1G";
	case PHY_SPEED_2500:
		return "2.5G";
	case PHY_SPEED_5000:
		return "5G";
	case PHY_SPEED_10000:
		return "10G";
	default:
		return "Unkn";
	}
}
EXPORT_SYMBOL(phy_dev_speed_to_short_str);

char *phy_dev_speed_to_str(phy_speed_t speed)
{
	switch (speed)
    {
	case PHY_SPEED_10:
		return "10 Mbps";
	case PHY_SPEED_100:
		return "100 Mbps";
	case PHY_SPEED_1000:
		return "1 Gbps";
	case PHY_SPEED_2500:
		return "2.5 Gbps";
	case PHY_SPEED_5000:
		return "5 Gbps";
	case PHY_SPEED_10000:
		return "10 Gbps";
	default:
		return "Unknown";
	}
}
EXPORT_SYMBOL(phy_dev_speed_to_str);

char *phy_dev_duplex_to_str(phy_duplex_t duplex)
{
	switch (duplex)
    {
	case PHY_DUPLEX_HALF:
		return "Half";
	case PHY_DUPLEX_FULL:
		return "Full";
	default:
		return "Unknown";
    }
}

char *phy_dev_flowctrl_to_str(int pause_rx, int pause_tx)
{
    if (pause_rx && pause_tx)
        return "TXRX";
    else if (pause_rx)
        return "RX";
    else if (pause_tx)
        return "TX";
    else
        return "";
}

void phy_dev_print_status(phy_dev_t *phy_dev)
{
    if (phy_dev->link)
    {
        pr_cont("%s:%s:0x%x - Link Up %s %s duplex\n",
            phy_dev->phy_drv->name, phy_dev_mii_type_to_str(phy_dev->mii_type), phy_dev->addr,
            phy_dev_speed_to_str(phy_dev->speed),
            phy_dev_duplex_to_str(phy_dev->duplex));
    }
    else
    {
        pr_cont("%s:%s:0x%x - Link Down\n", phy_dev->phy_drv->name, phy_dev_mii_type_to_str(phy_dev->mii_type), phy_dev->addr);
	}
}
EXPORT_SYMBOL(phy_dev_print_status);

static phy_dev_t phy_devices[MAX_PHY_DEVS] = {};

phy_dev_t *phy_dev_get_force(phy_type_t phy_type, uint32_t addr)
{
    uint32_t i;
    phy_dev_t *phy_dev = NULL;

    for (i = 0; i < MAX_PHY_DEVS; i++)
    {
        phy_dev_t *phy = &phy_devices[i];

        if (phy->phy_drv == NULL)
            continue;

        if ((phy_type != PHY_TYPE_UNKNOWN) &&       // for ioctl use
            (phy->phy_drv->phy_type != phy_type))
            continue;

        if (phy->addr != addr)
            continue;

        phy_dev = phy;
        break;
    }

    return phy_dev;
}
EXPORT_SYMBOL(phy_dev_get_force);

phy_dev_t *phy_dev_get(phy_type_t phy_type, uint32_t addr)
{
    uint32_t i;
    phy_dev_t *phy_dev = NULL;

    for (i = 0; i < MAX_PHY_DEVS; i++)
    {
        phy_dev_t *phy = &phy_devices[i];

        if (phy->phy_drv == NULL)
            continue;

        if ((phy_type != PHY_TYPE_UNKNOWN) &&       // for ioctl use
            (phy->phy_drv->phy_type != phy_type))
            continue;

        if (phy->addr != addr || (phy->flag & PHY_FLAG_NOT_PRESENTED))
            continue;

        phy_dev = phy;
        break;
    }

    return phy_dev;
}
EXPORT_SYMBOL(phy_dev_get);

phy_dev_t *phy_dev_add(phy_type_t phy_type, uint32_t addr, void *priv)
{
    uint32_t i;
    phy_drv_t *phy_drv;
    phy_dev_t *phy_dev;

    if (!(phy_drv = phy_drivers[phy_type]))
    {
        printk("Failed to find phy driver: phy_type=%d\n", phy_type);
        return NULL;
    }

    if ((phy_dev = phy_dev_get(phy_type, addr)))
    {
        printk("Phy device already exists: %s:%d\n", phy_drv->name, addr);
        return NULL;
    }

    for (i = 0; i < MAX_PHY_DEVS && phy_devices[i].phy_drv != NULL; i++);

    if (i ==  MAX_PHY_DEVS)
    {
        printk("Failed adding phy device: %s:%d\n", phy_drv->name, addr);
        return NULL;
    }

    phy_dev = &phy_devices[i];

    phy_dev->phy_drv = phy_drv;
    phy_dev->addr = addr;
    phy_dev->priv = priv;

    return phy_dev;
}
EXPORT_SYMBOL(phy_dev_add);

int phy_dev_del(phy_dev_t *phy_dev)
{
    phy_drv_dev_del(phy_dev);
#ifdef PHY_LINK_CHANGE_NOTIFY
    phy_dev_link_change_unregister(phy_dev);
#endif
    memset(phy_dev, 0, sizeof(phy_dev_t));

    return 0;
}
EXPORT_SYMBOL(phy_dev_del);

extern phy_dev_t *phy_drv_crossbar_find_device(dt_handle_t dt_handle);

phy_dev_t *phy_drv_find_device(dt_handle_t dt_handle)
{
    int i;

    for (i = 0; i < MAX_PHY_DEVS; i++)
    {
        if (phy_devices[i].phy_drv == NULL)
            continue;

        if (dt_is_equal(dt_handle, phy_devices[i].dt_handle))
            return &phy_devices[i];
    }

    return phy_drv_crossbar_find_device(dt_handle);
}
EXPORT_SYMBOL(phy_drv_find_device);

/* For internal use only by proc interface */
int phy_devices_internal_index(phy_dev_t *phy_dev)
{
    uint32_t i;

    for (i = 0; i < MAX_PHY_DEVS; i++)
    {
        if (&phy_devices[i] == phy_dev && phy_dev->phy_drv)
            return i;
    }

    return -1;
}
EXPORT_SYMBOL(phy_devices_internal_index);

int phy_priv_fun(phy_dev_t *phy_dev, int op, ...)
{
    int ret;
    va_list ap;

    if (!phy_dev->phy_drv->priv_fun)
        return -1;

    va_start(ap, op);
    ret=phy_dev->phy_drv->priv_fun(phy_dev, op, ap);
    va_end(ap);
    
    return ret;
}
EXPORT_SYMBOL(phy_priv_fun);

#ifdef PHY_LINK_CHANGE_NOTIFY
void phy_dev_link_change_notify(phy_dev_t *phy_dev)
{
    int old_link = phy_dev->link;
    phy_speed_t old_speed = phy_dev->speed;
    phy_duplex_t old_duplex = phy_dev->duplex;

    phy_dev_read_status(phy_dev);

    /* If speed or duplex changed while link is up, force link to to notify upper layer */
    if (old_link && phy_dev->link && (old_speed != phy_dev->speed || old_duplex != phy_dev->duplex))
        phy_dev->link = 0;

    if (phy_dev->link != old_link && phy_dev->link_change_cb)
    {
        phy_dev->link_change_cb(phy_dev->link_change_ctx);
    }
}
EXPORT_SYMBOL(phy_dev_link_change_notify);

void phy_dev_force_link_reset(phy_dev_t *phy_dev)
{
    phy_dev_t *end_phy = get_active_phy(phy_dev);

    end_phy = cascade_phy_get_last(end_phy);

    /* Some old chip use the same board ID for daughter card design but without card plugged in */
    if (!end_phy)   
        return;

    /* Force status to link down to trigger link up event */
    end_phy->link = 0;
    phy_dev_link_change_notify(end_phy);
}
EXPORT_SYMBOL(phy_dev_force_link_reset);

static void phy_devices_link_change_notify(void)
{
    uint32_t i;

    for (i = 0; i < MAX_PHY_DEVS; i++)
    {
        if (phy_devices[i].phy_drv == NULL)
            continue;

        if (phy_devices[i].phy_drv->link_change_register)
            continue;

        phy_dev_link_change_notify(&phy_devices[i]);
    }
}

static struct timer_list phy_link_timer;
static int phy_link_timer_refs = 0;

static void phy_link_work_cb(struct work_struct *work)
{
    phy_devices_link_change_notify();
    mod_timer(&phy_link_timer, jiffies + msecs_to_jiffies(1000));
}

DECLARE_WORK(_work, phy_link_work_cb);

static void phy_link_timer_cb(struct timer_list *tl)
{
    schedule_work(&_work);
}

static void phy_link_timer_start(void)
{
    timer_setup(&phy_link_timer, phy_link_timer_cb, 0);
    phy_link_timer.expires = jiffies + msecs_to_jiffies(1000);
    add_timer(&phy_link_timer);
}

static void phy_link_timer_stop(void)
{
    del_timer(&phy_link_timer);
}

void phy_dev_link_change_register(phy_dev_t *phy_dev, link_change_cb_t cb, void *ctx)
{
    if (phy_dev->link_change_cb != NULL)
        return;

    phy_dev->link_change_cb = cb;
    phy_dev->link_change_ctx = ctx;

    if (phy_dev->phy_drv->link_change_register)
    {
        phy_dev->phy_drv->link_change_register(phy_dev);
        return;
    }

    if (phy_link_timer_refs == 0)
        phy_link_timer_start();

    phy_link_timer_refs++;
}
EXPORT_SYMBOL(phy_dev_link_change_register);

void phy_dev_link_change_unregister(phy_dev_t *phy_dev)
{
    if (phy_dev->link_change_cb == NULL)
        return;

    phy_dev->link_change_cb = NULL;
    phy_dev->link_change_ctx = NULL;

    if (phy_dev->phy_drv->link_change_unregister)
    {
        phy_dev->phy_drv->link_change_unregister(phy_dev);
        return;
    }

    phy_link_timer_refs--;

    if (phy_link_timer_refs == 0)
        phy_link_timer_stop();
}
EXPORT_SYMBOL(phy_dev_link_change_unregister);

typedef struct
{
    struct work_struct base_work;
    phy_dev_t *phy_dev;
    phy_dev_work_func_t func;
} phy_dev_work_t;

static void phy_dev_work_cb(struct work_struct *work)
{
    phy_dev_work_t *phy_dev_work = container_of(work, phy_dev_work_t, base_work);
    phy_dev_t *phy_dev = phy_dev_work->phy_dev;
    phy_dev_work_func_t func = phy_dev_work->func;

    func(phy_dev);
    kfree(phy_dev_work);
}

int phy_dev_queue_work(phy_dev_t *phy_dev, phy_dev_work_func_t func)
{
    phy_dev_work_t *phy_dev_work = kmalloc(sizeof(phy_dev_work_t), GFP_ATOMIC);
    if (!phy_dev_work)
    {
        printk("phy_dev_queue_work: kmalloc failed to allocate work struct\n");
        return -1;
    }

    INIT_WORK(&phy_dev_work->base_work, phy_dev_work_cb);
    phy_dev_work->phy_dev = phy_dev;
    phy_dev_work->func = func;

    queue_work(system_unbound_wq, &phy_dev_work->base_work);

    return 0;
}
#endif

#ifdef PHY_PROC_FS

#define PROC_DIR           "driver/phy"
#define CMD_PROC_FILE      "cmd"

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *cmd_proc_file;

extern int mac_proc_cmd_stats(int id);

static int phy_proc_cmd_stats(int argc, char *argv[])
{
    int id;

    if (argc < 2)
        goto Error;

    if (kstrtos32(argv[1], 10, &id) || id < 0)
        goto Error;

    return mac_proc_cmd_stats(id);

Error:
    printk("Usage: stats <mac id>\n");
    return 0;
}

static int phy_proc_cmd_status(int argc, char *argv[])
{
    int id;
    phy_dev_t *phy_dev;

    if (argc < 2)
        goto Error;

    if (kstrtos32(argv[1], 10, &id) || id < 0 || id >= MAX_PHY_DEVS)
        goto Error;

    phy_dev = &phy_devices[id];
    if (phy_dev->phy_drv == NULL)
        goto Error;

    printk("Phy %d status:\n", id);
    printk("\tDriver: %s\n", phy_dev->phy_drv->name);
    printk("\tMII: %s\n", phy_dev_mii_type_to_str(phy_dev->mii_type));
    printk("\tAddress: 0x%02x\n", phy_dev->addr);
    printk("\tLink: %s\n", phy_dev->link ? "Up" : "Down");
    printk("\tSpeed: %s\n", phy_dev_speed_to_str(phy_dev->speed));
    printk("\tDuplex: %s\n", phy_dev_duplex_to_str(phy_dev->duplex));
    printk("\tFlowctrl: %s\n", phy_dev_flowctrl_to_str(phy_dev->pause_rx, phy_dev->pause_tx));

    return 0;

Error:
    printk("Usage: status <id>\n");
    return 0;
}

static int phy_proc_cmd_init(int argc, char *argv[])
{
    int id;
    phy_dev_t *phy_dev;

    if (argc < 2)
        goto Error;

    if (kstrtos32(argv[1], 10, &id) || id < 0 || id >= MAX_PHY_DEVS)
        goto Error;

    phy_dev = &phy_devices[id];
    if (phy_dev->phy_drv == NULL)
        goto Error;

    if (phy_dev_init(phy_dev))
        goto Error;

    return 0;

Error:
    printk("Usage: init <id>\n");
    return 0;
}

extern int mac_proc_cmd_list(void);

static int phy_proc_cmd_list(int argc, char *argv[])
{
    int id;
    phy_dev_t *phy_dev;

    printk("|======================================================================================|\n");
    printk("|  Id |  State  |   Phy    | Addr |   Bus   |   Speed   | Duplex | Flowctl |   PHYID   |\n");
    printk("|======================================================================================|\n");

    for (id = 0; id < MAX_PHY_DEVS; id++)
    {
        uint32_t phyid = 0;
        phy_dev = &phy_devices[id];
        if (phy_dev->phy_drv == NULL)
            continue;

        pr_cont("| %2d  ", id);

        if (phy_dev->phy_drv->initialized)
            phy_dev_phyid_get(phy_dev, &phyid);

        pr_cont("| %s  ", phy_dev->link ? " Up   " : " Down ");
        pr_cont("| %-8s ", phy_dev->phy_drv->name);
        pr_cont("| 0x%02x ", phy_dev->addr);
        pr_cont("| %-7s ", phy_dev->mii_type == PHY_MII_TYPE_UNKNOWN ? "" : phy_dev_mii_type_to_str(phy_dev->mii_type));
        pr_cont("| %9s ", phy_dev->speed == PHY_SPEED_UNKNOWN ? "" : phy_dev_speed_to_str(phy_dev->speed));
        pr_cont("| %5s  ", phy_dev->duplex == PHY_DUPLEX_UNKNOWN ? "" : phy_dev_duplex_to_str(phy_dev->duplex));
        pr_cont("|  %4s   ", phy_dev_flowctrl_to_str(phy_dev->pause_rx, phy_dev->pause_tx));
        pr_cont("| %04x:%04x ", phyid >> 16, phyid & 0xffff);
        pr_cont("|\n");
    }
    printk("|======================================================================================|\n\n");

    mac_proc_cmd_list();

    return 0;
}

static int phy_proc_cmd_read(int argc, char *argv[])
{
    int id;
    uint16_t reg, val;
    phy_dev_t *phy_dev;

    if (argc < 3)
        goto Error;

    if (kstrtos32(argv[1], 10, &id) || id < 0 || id >= MAX_PHY_DEVS)
        goto Error;

    if (kstrtou16(argv[2], 16, &reg))
        goto Error;

    phy_dev = &phy_devices[id];
    if (phy_dev->phy_drv == NULL)
        goto Error;

    if (phy_dev_read(phy_dev, reg, &val))
        goto Error;

    printk("Read register 0x%04x=0x%04x\n", reg, val);

    return 0;

Error:
    printk("Usage: read <id> <reg>\n");
    return 0;
}

static int phy_proc_cmd_write(int argc, char *argv[])
{
    int id;
    uint16_t reg, val;
    phy_dev_t *phy_dev;

    if (argc < 4)
        goto Error;

    if (kstrtos32(argv[1], 10, &id) || id < 0 || id >= MAX_PHY_DEVS)
        goto Error;

    if (kstrtou16(argv[2], 16, &reg))
        goto Error;

    if (kstrtou16(argv[3], 16, &val))
        goto Error;

    phy_dev = &phy_devices[id];
    if (phy_dev->phy_drv == NULL)
        goto Error;

    if (phy_dev_write(phy_dev, reg, val))
        goto Error;

    printk("Write register 0x%04x=0x%04x\n", reg, val);

    return 0;

Error:
    printk("Usage: write <id> <reg> <val>\n");
    return 0;
}

static int phy_proc_cmd_read45(int argc, char *argv[])
{
    int id;
    uint16_t dev, reg, val;
    phy_dev_t *phy_dev;

    if (argc < 4)
        goto Error;

    if (kstrtos32(argv[1], 10, &id) || id < 0 || id >= MAX_PHY_DEVS)
        goto Error;

    if (kstrtou16(argv[2], 16, &dev))
        goto Error;

    if (kstrtou16(argv[3], 16, &reg))
        goto Error;

    phy_dev = &phy_devices[id];
    if (phy_dev->phy_drv == NULL)
        goto Error;

    if (phy_dev_c45_read(phy_dev, dev, reg, &val))
        goto Error;

    printk("Read45 dev=%d register 0x%04x=0x%04x\n", dev, reg, val);

    return 0;

Error:
    printk("Usage: read45 <id> <dev> <reg>\n");
    return 0;
}

static int phy_proc_cmd_write45(int argc, char *argv[])
{
    int id;
    uint16_t dev, reg, val;
    phy_dev_t *phy_dev;

    if (argc < 5)
        goto Error;

    if (kstrtos32(argv[1], 10, &id) || id < 0 || id >= MAX_PHY_DEVS)
        goto Error;

    if (kstrtou16(argv[2], 16, &dev))
        goto Error;

    if (kstrtou16(argv[3], 16, &reg))
        goto Error;

    if (kstrtou16(argv[4], 16, &val))
        goto Error;

    phy_dev = &phy_devices[id];
    if (phy_dev->phy_drv == NULL)
        goto Error;

    if (phy_dev_c45_write(phy_dev, dev, reg, val))
        goto Error;

    printk("Write45 dev=%d register 0x%04x=0x%04x\n", dev, reg, val);

    return 0;

Error:
    printk("Usage: write45 <id> <dev> <reg> <val>\n");
    return 0;
}

#ifdef MACSEC_SUPPORT
static int phy_proc_cmd_read_indirect(int argc, char *argv[])
{
    int id;
    uint16_t temp=0;
    uint32_t reg, val;
    phy_dev_t *phy_dev;

    if (argc < 3)
        goto Error;

    if (kstrtos32(argv[1], 10, &id) || id < 0 || id >= MAX_PHY_DEVS)
        goto Error;

    if (kstrtou32(argv[2], 16, &reg))
        goto Error;

    phy_dev = &phy_devices[id];
    if (phy_dev->phy_drv == NULL)
        goto Error;


    temp = (uint16_t)(reg & 0xffff);
    if (phy_bus_c45_write(phy_dev, 0x01, 0xa819, temp))
        goto Error;

    temp = (uint16_t)(reg >> 16);
    if (phy_bus_c45_write(phy_dev, 0x01, 0xa81a, temp))
        goto Error;

    if (phy_bus_c45_write(phy_dev, 0x01, 0xa817, 0x000a))
        goto Error;

    if (phy_bus_c45_read(phy_dev, 0x01, 0xa81b, &temp))
        goto Error;

    val = temp;
    
    if (phy_bus_c45_read(phy_dev, 0x01, 0xa81c, &temp))
        goto Error;

    val |= (uint32_t)(temp<<16);

    printk("Read32 register 0x%08x=0x%08x\n", reg, val);

    return 0;

Error:
    printk("Usage: read32 <id> <reg>\n");
    return 0;
}

static int phy_proc_cmd_write_indirect(int argc, char *argv[])
{
    int id;
    uint32_t reg, val;
    uint16_t temp=0;
    phy_dev_t *phy_dev;

    if (argc < 4)
        goto Error;

    if (kstrtos32(argv[1], 10, &id) || id < 0 || id >= MAX_PHY_DEVS)
        goto Error;

    if (kstrtou32(argv[2], 16, &reg))
        goto Error;

    if (kstrtou32(argv[3], 16, &val))
        goto Error;

    phy_dev = &phy_devices[id];
    if (phy_dev->phy_drv == NULL)
        goto Error;


    temp = (uint16_t)(reg & 0xffff);
    if (phy_bus_c45_write(phy_dev, 0x01, 0xa819, temp))
        goto Error;

    temp = (uint16_t)(reg >> 16);
    if (phy_bus_c45_write(phy_dev, 0x01, 0xa81a, temp))
        goto Error;

    temp = (uint16_t)(val & 0xffff);
    if (phy_bus_c45_write(phy_dev, 0x01, 0xa81b, temp))
        goto Error;

    temp = (uint16_t)(val >> 16);
    if (phy_bus_c45_write(phy_dev, 0x01, 0xa81c, temp))
        goto Error;

    if (phy_bus_c45_write(phy_dev, 0x01, 0xa817, 0x0009))
        goto Error;

    return 0;

Error:
    printk("Usage: write32 <id> <reg> <val>\n");
    return 0;
}

static int phy_proc_macsec_enable64(int argc, char *argv[])
{
    int id, ret, enable;
    phy_dev_t *phy_dev;
    macsec_api_data macsec_data = {};
    macsec_api_settings_t settings = {0,0,0,0,0,0,0,0,1,0,0,0,1};
    macsec_api_sa_t sa_egress = {{0xde, 0xad, 0xbe, 0xef, 0xa5, 0x5a, 0x00, 0x01}, {0x7A, 0x30, 0xC1, 0x18}, {0xE6, 0x30, 0xE8, 0x1A, 0x48, 0xDE, 0x86, 0xA2, 0x1C, 0x66, 0xFA, 0x6D},
                                 {0xad, 0x7a, 0x2b, 0xd0, 0x3e, 0xac, 0x83, 0x5a, 0x6f, 0x62, 0x0f, 0xdc, 0xb5, 0x06, 0xb3, 0x45, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                 {0x73, 0xa2, 0x3d, 0x80, 0x12, 0x1d, 0xe2, 0xd5, 0xa8, 0x50, 0x25, 0x3f, 0xcf, 0x43, 0x12, 0x0e},
                                 16, 1, {{0,1,1,0,0,0,1,1,1}}, MACSEC_SA_ACTION_EGRESS};
    macsec_api_sa_t sa_ingress = {{0xde, 0xad, 0xbe, 0xef, 0xa5, 0x5a, 0x00, 0x01}, {0x7A, 0x30, 0xC1, 0x18}, {0xE6, 0x30, 0xE8, 0x1A, 0x48, 0xDE, 0x86, 0xA2, 0x1C, 0x66, 0xFA, 0x6D},
                                 {0xad, 0x7a, 0x2b, 0xd0, 0x3e, 0xac, 0x83, 0x5a, 0x6f, 0x62, 0x0f, 0xdc, 0xb5, 0x06, 0xb3, 0x45, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                 {0x73, 0xa2, 0x3d, 0x80, 0x12, 0x1d, 0xe2, 0xd5, 0xa8, 0x50, 0x25, 0x3f, 0xcf, 0x43, 0x12, 0x0e},
                                 16, 1, {{0,0,MACSEC_FRAME_VALIDATE_STRICT,0,1,0,0,0}}, MACSEC_SA_ACTION_INGRESS};

    if (argc < 3)
        goto Error;

    if (kstrtos32(argv[1], 10, &id) || id < 0 || id >= MAX_PHY_DEVS)
        goto Error;

    if (kstrtou32(argv[2], 10, &enable))
        goto Error;

    phy_dev = &phy_devices[id];

    if (!phy_dev->macsec_dev)
    {
        printk("PHY does not support MACSEC!!\n");
        return 0;
    }

    if (phy_dev->phy_drv == NULL)
        goto Error;

    if (phy_dev->phy_drv->macsec_oper == NULL)
        goto Error;

    macsec_data.op = MACSEC_OPER_INIT;
    memcpy(&macsec_data.ext_data.secy_conf, &settings, sizeof(macsec_api_settings_t));
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    memset(&macsec_data.ext_data, 0, sizeof(macsec_api_ext_data));

    /* Egress */
    macsec_data.direction = 0;

    macsec_data.op = MACSEC_OPER_VPORT_ADD;
    macsec_data.index1 = 0;
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    macsec_data.op = MACSEC_OPER_SA_ADD;
    macsec_data.index1 = 0;
    macsec_data.index2 = 0;
    memcpy(&macsec_data.ext_data.sa_conf, &sa_egress, sizeof(macsec_api_sa_t));
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    memset(&macsec_data.ext_data, 0, sizeof(macsec_api_ext_data));

    macsec_data.op = MACSEC_OPER_RULE_ADD;
    macsec_data.index1 = 0;
    macsec_data.index2 = 0;
    macsec_data.ext_data.rule_conf.num_tags = 1;
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    memset(&macsec_data.ext_data, 0, sizeof(macsec_api_ext_data));

    macsec_data.op = MACSEC_OPER_RULE_ENABLE;
    macsec_data.index1 = 0;
    macsec_data.data1 = 1;
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    /* Ingress */
    macsec_data.direction = 1;

    macsec_data.op = MACSEC_OPER_VPORT_ADD;
    macsec_data.index1 = 0;
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    macsec_data.op = MACSEC_OPER_SA_ADD;
    macsec_data.index1 = 0;
    macsec_data.index2 = 0;
    memcpy(&macsec_data.ext_data.sa_conf, &sa_ingress, sizeof(macsec_api_sa_t));
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    memset(&macsec_data.ext_data, 0, sizeof(macsec_api_ext_data));

    macsec_data.op = MACSEC_OPER_RULE_ADD;
    macsec_data.index1 = 0;
    macsec_data.index2 = 0;
    macsec_data.ext_data.rule_conf.num_tags = 1;
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    memset(&macsec_data.ext_data, 0, sizeof(macsec_api_ext_data));

    macsec_data.op = MACSEC_OPER_RULE_ENABLE;
    macsec_data.index1 = 0;
    macsec_data.data1 = 1;
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

Error:
    printk("Usage: macsecenable64 <id> <1/0>\n");
    return 0;
}

static int phy_proc_macsec_enable(int argc, char *argv[])
{
    int id, ret, enable;
    phy_dev_t *phy_dev;
    macsec_api_data macsec_data = {};
    macsec_api_settings_t settings = {0,0,0,0,0,0,0,0,1,0,0,0,1};
    macsec_api_sa_t sa_egress = {{0xde, 0xad, 0xbe, 0xef, 0xa5, 0x5a, 0x00, 0x01}, {}, {},
                                 {0xad, 0x7a, 0x2b, 0xd0, 0x3e, 0xac, 0x83, 0x5a, 0x6f, 0x62, 0x0f, 0xdc, 0xb5, 0x06, 0xb3, 0x45, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                 {0x73, 0xa2, 0x3d, 0x80, 0x12, 0x1d, 0xe2, 0xd5, 0xa8, 0x50, 0x25, 0x3f, 0xcf, 0x43, 0x12, 0x0e},
                                 16, 0, {{0,1,1,0,0,0,1,1,0}}, MACSEC_SA_ACTION_EGRESS};
    macsec_api_sa_t sa_ingress = {{0xde, 0xad, 0xbe, 0xef, 0xa5, 0x5a, 0x00, 0x01}, {}, {},
                                 {0xad, 0x7a, 0x2b, 0xd0, 0x3e, 0xac, 0x83, 0x5a, 0x6f, 0x62, 0x0f, 0xdc, 0xb5, 0x06, 0xb3, 0x45, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                 {0x73, 0xa2, 0x3d, 0x80, 0x12, 0x1d, 0xe2, 0xd5, 0xa8, 0x50, 0x25, 0x3f, 0xcf, 0x43, 0x12, 0x0e},
                                 16, 0, {{0,0,MACSEC_FRAME_VALIDATE_STRICT,0,1,0,0,1}}, MACSEC_SA_ACTION_INGRESS};

    if (argc < 3)
        goto Error;

    if (kstrtos32(argv[1], 10, &id) || id < 0 || id >= MAX_PHY_DEVS)
        goto Error;

    if (kstrtou32(argv[2], 10, &enable))
        goto Error;

    phy_dev = &phy_devices[id];

    if (!phy_dev->macsec_dev)
    {
        printk("PHY does not support MACSEC!!\n");
        return 0;
    }

    if (phy_dev->phy_drv == NULL)
        goto Error;

    if (phy_dev->phy_drv->macsec_oper == NULL)
        goto Error;

    macsec_data.op = MACSEC_OPER_INIT;
    memcpy(&macsec_data.ext_data.secy_conf, &settings, sizeof(macsec_api_settings_t));
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    memset(&macsec_data.ext_data, 0, sizeof(macsec_api_ext_data));

    /* Egress */
    macsec_data.direction = 0;

    macsec_data.op = MACSEC_OPER_VPORT_ADD;
    macsec_data.index1 = 0;
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    macsec_data.op = MACSEC_OPER_SA_ADD;
    macsec_data.index1 = 0;
    macsec_data.index2 = 0;
    memcpy(&macsec_data.ext_data.sa_conf, &sa_egress, sizeof(macsec_api_sa_t));
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    memset(&macsec_data.ext_data, 0, sizeof(macsec_api_ext_data));

    macsec_data.op = MACSEC_OPER_RULE_ADD;
    macsec_data.index1 = 0;
    macsec_data.index2 = 0;
    macsec_data.ext_data.rule_conf.num_tags = 1;
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    memset(&macsec_data.ext_data, 0, sizeof(macsec_api_ext_data));

    macsec_data.op = MACSEC_OPER_RULE_ENABLE;
    macsec_data.index1 = 0;
    macsec_data.data1 = 1;
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    /* Ingress */
    macsec_data.direction = 1;

    macsec_data.op = MACSEC_OPER_VPORT_ADD;
    macsec_data.index1 = 0;
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    macsec_data.op = MACSEC_OPER_SA_ADD;
    macsec_data.index1 = 0;
    macsec_data.index2 = 0;
    memcpy(&macsec_data.ext_data.sa_conf, &sa_ingress, sizeof(macsec_api_sa_t));
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    memset(&macsec_data.ext_data, 0, sizeof(macsec_api_ext_data));

    macsec_data.op = MACSEC_OPER_RULE_ADD;
    macsec_data.index1 = 0;
    macsec_data.index2 = 0;
    macsec_data.ext_data.rule_conf.num_tags = 1;
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    memset(&macsec_data.ext_data, 0, sizeof(macsec_api_ext_data));

    macsec_data.op = MACSEC_OPER_RULE_ENABLE;
    macsec_data.index1 = 0;
    macsec_data.data1 = 1;
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

Error:
    printk("Usage: macsecenable <id> <1/0>\n");
    return 0;
}

static int phy_proc_macsec_enablebp(int argc, char *argv[])
{
    int id, ret, enable;
    phy_dev_t *phy_dev;
    macsec_api_data macsec_data = {};
    macsec_api_settings_t settings = {0,0,0,0,0,0,0,0,1,0,0,0,1};

    if (argc < 3)
        goto Error;

    if (kstrtos32(argv[1], 10, &id) || id < 0 || id >= MAX_PHY_DEVS)
        goto Error;

    if (kstrtou32(argv[2], 10, &enable))
        goto Error;

    phy_dev = &phy_devices[id];

    if (!phy_dev->macsec_dev)
    {
        printk("PHY does not support MACSEC!!\n");
        return 0;
    }

    if (phy_dev->phy_drv == NULL)
        goto Error;

    if (phy_dev->phy_drv->macsec_oper == NULL)
        goto Error;

    macsec_data.op = MACSEC_OPER_INIT;
    memcpy(&macsec_data.ext_data.secy_conf, &settings, sizeof(macsec_api_settings_t));
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    memset(&macsec_data.ext_data, 0, sizeof(macsec_api_ext_data));

    /* Egress */
    macsec_data.direction = 0;

    macsec_data.op = MACSEC_OPER_VPORT_ADD;
    macsec_data.index1 = 0;
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    macsec_data.op = MACSEC_OPER_SA_ADD;
    macsec_data.index1 = 0;
    macsec_data.index2 = 0;
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    memset(&macsec_data.ext_data, 0, sizeof(macsec_api_ext_data));

    macsec_data.op = MACSEC_OPER_RULE_ADD;
    macsec_data.index1 = 0;
    macsec_data.index2 = 0;
    macsec_data.ext_data.rule_conf.num_tags = 1;
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    memset(&macsec_data.ext_data, 0, sizeof(macsec_api_ext_data));

    macsec_data.op = MACSEC_OPER_RULE_ENABLE;
    macsec_data.index1 = 0;
    macsec_data.data1 = 1;
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    /* Ingress */
    macsec_data.direction = 1;

    macsec_data.op = MACSEC_OPER_VPORT_ADD;
    macsec_data.index1 = 0;
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    macsec_data.op = MACSEC_OPER_SA_ADD;
    macsec_data.index1 = 0;
    macsec_data.index2 = 0;
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    memset(&macsec_data.ext_data, 0, sizeof(macsec_api_ext_data));

    macsec_data.op = MACSEC_OPER_RULE_ADD;
    macsec_data.index1 = 0;
    macsec_data.index2 = 0;
    macsec_data.ext_data.rule_conf.num_tags = 1;
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

    memset(&macsec_data.ext_data, 0, sizeof(macsec_api_ext_data));

    macsec_data.op = MACSEC_OPER_RULE_ENABLE;
    macsec_data.index1 = 0;
    macsec_data.data1 = 1;
    ret = phy_dev->phy_drv->macsec_oper(phy_dev, &macsec_data);

Error:
    printk("Usage: macsecenable <id> <1/0>\n");
    return 0;
}
#endif

#if defined(DSL_DEVICES) && !defined(ARCHER_DEVICE)
extern void phy_drv_sfp_group_list(void);
static int phy_proc_cmd_sfp_list(int argc, char *argv[])
{
    phy_drv_sfp_group_list();
    return 0;
}
#endif

#ifdef PHY_CROSSBAR
extern void phy_drv_crossbar_group_list(void);

static int phy_proc_cmd_crossbar_list(int argc, char *argv[])
{
    phy_drv_crossbar_group_list();

    return 0;
}
#endif

#if !defined(DSL_DEVICES)
static int serdes_proc_cmd_diag(int argc, char *argv[])
{
    int id;
    uint32_t level;
    phy_dev_t *phy_dev;

    if (argc < 2)
        goto Error;

    if (kstrtos32(argv[1], 10, &id) || id < 0 || id >= MAX_PHY_DEVS)
        goto Error;

    phy_dev = &phy_devices[id];
    if (phy_dev->phy_drv == NULL)
        goto Error;

    if (argc < 3)
        level = 0x1ff;
    else if (kstrtou32(argv[2], 0, &level))
        goto Error;

    return phy_dev_diag(phy_dev, level);

Error:
    printk("Usage: diag <id> [<level>]\n");
    return 0;
}
#else
static int serdes_proc_cmd_diag(int argc, char *argv[])
{
    char *serdes_proc_cmd_diag_usage = 
        "Usage: diag <phy_address> [<level>]\n"
        "   <level>:\n"
        "           SRDS_DIAG_LANE       = 1<<0,\n"
        "           SRDS_DIAG_CORE       = 1<<1,\n"
        "           SRDS_DIAG_EVENT      = 1<<2,\n"
        "           SRDS_DIAG_EYE        = 1<<3,\n"
        "           SRDS_DIAG_REG_CORE   = 1<<4,\n"
        "           SRDS_DIAG_REG_LANE   = 1<<5,\n"
        "           SRDS_DIAG_UC_CORE    = 1<<6,\n"
        "           SRDS_DIAG_UC_LANE    = 1<<7,\n"
        "           SRDS_DIAG_LANE_DEBUG = 1<<8,\n"
        "           SRDS_DIAG_BER_VERT   = 1<<9,\n"
        "           SRDS_DIAG_BER_HORZ   = 1<<10\n"
        "   When no <level>, all flags above will be set\n"
        ;
        
    uint32_t phy_addr, level;
    phy_dev_t *phy_dev;

    if (argc < 2)
        goto wrong_params;

    if (kstrtou32(argv[1], 0, &phy_addr))
        goto wrong_params;

    if(!(phy_dev = phy_dev_get(PHY_TYPE_UNKNOWN, phy_addr)))
        goto wrong_params;

    if (argc < 3)
        level = 511;

    else if (kstrtou32(argv[2], 0, &level))
        goto wrong_params;

    return phy_dev_diag(phy_dev, level);

wrong_params:
    pr_info("%s\n", serdes_proc_cmd_diag_usage);
    return -1;
}
#endif

static int serdes_proc_cmd_prbs(int argc, char *argv[])
{
    int id;
    int param;
    phy_dev_t *phy_dev;

    if (argc < 2)
        goto Error;

    if (kstrtos32(argv[1], 10, &id) || id < 0 || id >= MAX_PHY_DEVS)
        goto Error;

    phy_dev = &phy_devices[id];
    if (phy_dev->phy_drv == NULL)
        goto Error;

    if (argc < 3)
        param = -1;
    else if (kstrtou32(argv[2], 0, &param))
        goto Error;

    return phy_dev_prbs(phy_dev, param);

Error:
    printk("Usage: prbs <id> [<0|7|9|11|15|23|31|58>]\n");
    return 0;
}

static int serdes_proc_cmd_txcfg(int argc, char *argv[])
{
    int id;
    phy_dev_t *phy_dev;
    uint8_t pre, main, post1, post2, hpf;

    if (argc < 2)
        goto Error;

    if (kstrtos32(argv[1], 10, &id) || id < 0 || id >= MAX_PHY_DEVS)
        goto Error;

    phy_dev = &phy_devices[id];
    if (phy_dev->phy_drv == NULL)
        goto Error;

    if (argc == 2)
    {
        if (phy_dev_tx_cfg_get(phy_dev, &pre, &main, &post1, &post2, &hpf))
            goto Error;

        printk("%d %d %d %d %d\n", pre, main, post1, post2, hpf);
        return 0;
    }

    if (argc < 7)
        goto Error;

    if (kstrtou8(argv[2], 10, &pre))
        goto Error;

    if (kstrtou8(argv[3], 10, &main))
        goto Error;

    if (kstrtou8(argv[4], 10, &post1))
        goto Error;

    if (kstrtou8(argv[5], 10, &post2))
        goto Error;

    if (kstrtou8(argv[6], 10, &hpf))
        goto Error;

    return phy_dev_tx_cfg_set(phy_dev, pre, main, post1, post2, hpf);

Error:
    printk("Usage: txcfg <id> [<pre> <main> <post1> <post2> <hpf>]\n");
    return 0;
}

static int serdes_proc_cmd_temp(int argc, char *argv[])
{
    int16_t temp;
    int id;
    phy_dev_t *phy_dev;

    if (argc < 2)
        goto Error;

    if (kstrtos32(argv[1], 10, &id) || id < 0 || id >= MAX_PHY_DEVS)
        goto Error;

    phy_dev = &phy_devices[id];

    if (phy_dev->phy_drv->phy_temp_get == NULL) {
        printk("Temperature monitor not available\n");
	return 0;
    }

    phy_dev->phy_drv->phy_temp_get(phy_dev, &temp);
    printk("%d Celsius\n", temp);

    return 0;

Error:
    printk("Usage: temp <id>\n");
    return 0;
}

static struct proc_cmd_ops command_entries[] = {
    { .name = "stats", .do_command	= phy_proc_cmd_stats},
    { .name = "status", .do_command	= phy_proc_cmd_status},
    { .name = "init", .do_command	= phy_proc_cmd_init},
    { .name = "list", .do_command	= phy_proc_cmd_list},
    { .name = "read", .do_command	= phy_proc_cmd_read},
    { .name = "write", .do_command	= phy_proc_cmd_write},
    { .name = "read45", .do_command	= phy_proc_cmd_read45},
    { .name = "write45", .do_command = phy_proc_cmd_write45},
    { .name = "diag", .do_command = serdes_proc_cmd_diag},
    { .name = "prbs", .do_command = serdes_proc_cmd_prbs},
    { .name = "txcfg", .do_command = serdes_proc_cmd_txcfg},
    { .name = "temp", .do_command = serdes_proc_cmd_temp},
#ifdef MACSEC_SUPPORT
    { .name = "read32", .do_command	= phy_proc_cmd_read_indirect},
    { .name = "write32", .do_command = phy_proc_cmd_write_indirect},
    { .name = "macsecenable", .do_command = phy_proc_macsec_enable},
    { .name = "macsecenable64", .do_command = phy_proc_macsec_enable64},
    { .name = "macsecenablebp", .do_command = phy_proc_macsec_enablebp},
#endif
#ifdef PHY_CROSSBAR
    { .name = "crossbars", .do_command = phy_proc_cmd_crossbar_list},
#endif
#if defined(DSL_DEVICES) && !defined(ARCHER_DEVICE)
    { .name = "sfp", .do_command	= phy_proc_cmd_sfp_list},
#endif
};

static struct proc_cmd_table phy_command_table = {
    .module_name = "PHY",
    .size = ARRAY_SIZE(command_entries),
    .ops = command_entries
};

static void phy_proc_exit(void)
{
    if (cmd_proc_file)
    {
        remove_proc_entry(CMD_PROC_FILE, proc_dir);
        cmd_proc_file = NULL;
    }
    if (proc_dir)
    {
        remove_proc_entry(PROC_DIR, NULL);
        proc_dir = NULL;
    }
}

int __init phy_proc_init(void)
{
    int status = 0;

    proc_dir = proc_mkdir(PROC_DIR, NULL);
    if (!proc_dir)
    {
        pr_err("Failed to create PROC directory %s.\n",
            PROC_DIR);
        goto error;
    }
    cmd_proc_file = proc_create_cmd(CMD_PROC_FILE, proc_dir,
        &phy_command_table);
    if (!cmd_proc_file)
    {
        pr_err("Failed to create %s\n", CMD_PROC_FILE);
        goto error;
    }

    return status;

error:
    if (proc_dir)
        phy_proc_exit();

    status = -EIO;
    return status;
}
late_initcall(phy_proc_init);
#endif

#define LEN 256
int logger_write(int message_verbose_level, const char *format, ...)
{
    va_list args;
    int r;

    static char leftover[LEN] = "";
    char *eol; 
    char line[LEN];

    r = snprintf(line, LEN, "%s", leftover);
    va_start(args, format);
    r = vsnprintf(line + r, LEN - r, format, args);
    va_end(args);
    if ((eol = strrchr(line, '\n')))
    {
        strncpy(leftover, ++eol, LEN-1); // print later
        *eol = '\0';
        printk("%s", line); // print now until the last \n
    }
    else
    {
        strncpy(leftover, line, LEN-1); // print later
        leftover[LEN-1]='\0';
    }

    return r;
}

/* power down all PHYs through GPIO if exist, except for the given _phy_dev) */
void phy_devices_shutdown(phy_dev_t *_phy_dev)
{
    uint32_t i;

    for (i = 0; i < MAX_PHY_DEVS; i++)
    {
        phy_dev_t *phy_dev = &phy_devices[i];

        if (!_phy_dev)
            goto power_down;

        if (phy_dev == _phy_dev)
            continue;

        if (_phy_dev->inter_phy_types & INTER_PHY_TYPE_USXGMII_MP_M &&
            phy_dev->inter_phy_types & INTER_PHY_TYPE_USXGMII_MP_M)
        {
            printk("phy_devices_shutdown: skipping USXGMII-M port 0x%x\n", phy_dev->addr);
            continue;
        }

power_down:
        if (dt_gpio_exists(phy_dev->gpiod_phy_reset))
        {
            printk("phy_devices_shutdown: PHY reset 0x%x\n", phy_dev->addr);
            dt_gpio_set_value(phy_dev->gpiod_phy_reset, 1);
        }

        if (dt_gpio_exists(phy_dev->gpiod_phy_power))
        {
            printk("phy_devices_shutdown: PHY power down 0x%x\n", phy_dev->addr);
            dt_gpio_set_value(phy_dev->gpiod_phy_power, 0);
        }
    }
}
EXPORT_SYMBOL(phy_devices_shutdown);
