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
#ifndef _CFE_
#include <linux/workqueue.h>
#include <linux/slab.h>
#include "proc_cmd.h"
#endif

#define MAX_PHY_DEVS 8

extern phy_drv_t phy_drv_6848_ephy;
extern phy_drv_t phy_drv_6848_egphy;
extern phy_drv_t phy_drv_6848_sgmii;
extern phy_drv_t phy_drv_pcs;
extern phy_drv_t phy_drv_6858_egphy;
extern phy_drv_t phy_drv_ext1;
extern phy_drv_t phy_drv_ext2;
extern phy_drv_t phy_drv_ext3;
extern phy_drv_t phy_drv_lport_serdes;
extern phy_drv_t phy_drv_53125_sw;

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

phy_drv_t *phy_drv_get(phy_type_t phy_type)
{
    phy_drv_t *phy_drv = NULL;

    switch (phy_type)
    {
#ifdef PHY_6848_EPHY
    case PHY_TYPE_6848_EPHY:
        phy_drv = &phy_drv_6848_ephy;
        break;
#endif
#ifdef PHY_6848_EGPHY
    case PHY_TYPE_6848_EGPHY:
        phy_drv = &phy_drv_6848_egphy;
        break;
#endif
#ifdef PHY_6848_SGMII
    case PHY_TYPE_6848_SGMII:
        phy_drv = &phy_drv_6848_sgmii;
        break;
#endif
#ifdef PHY_PCS
    case PHY_TYPE_PCS:
        phy_drv = &phy_drv_pcs;
        break;
#endif
#ifdef PHY_6858_EGPHY
    case PHY_TYPE_6858_EGPHY:
        phy_drv = &phy_drv_6858_egphy;
        break;
#endif
#ifdef PHY_EXT1
    case PHY_TYPE_EXT1:
        phy_drv = &phy_drv_ext1;
        break;
#endif
#ifdef PHY_EXT2
    case PHY_TYPE_EXT2:
        phy_drv = &phy_drv_ext2;
        break;
#endif
#ifdef PHY_EXT3
    case PHY_TYPE_EXT3:
        phy_drv = &phy_drv_ext3;
        break;
#endif
#ifdef PHY_LPORT_SERDES
    case PHY_TYPE_LPORT_SERDES:
        phy_drv = &phy_drv_lport_serdes;
        break;
#endif
#ifdef PHY_EXT_SWITCH
    case PHY_TYPE_53125:
        phy_drv = &phy_drv_53125_sw;
        break;
#endif
    default:
        break;
    }

    return phy_drv;
}
EXPORT_SYMBOL(phy_drv_get);

static int _phy_drv_init(phy_type_t phy_type)
{
    phy_drv_t *phy_drv;

    if ((phy_drv = phy_drv_get(phy_type)) == NULL)
        return 0;

    return phy_drv_init(phy_drv);
}

int phy_drivers_init(void)
{
    int ret = 0;

    ret |= _phy_drv_init(PHY_TYPE_6848_EPHY);
    ret |= _phy_drv_init(PHY_TYPE_6848_EGPHY);
    ret |= _phy_drv_init(PHY_TYPE_6848_SGMII);
    ret |= _phy_drv_init(PHY_TYPE_PCS);
    ret |= _phy_drv_init(PHY_TYPE_6858_EGPHY);
    ret |= _phy_drv_init(PHY_TYPE_EXT1);
    ret |= _phy_drv_init(PHY_TYPE_EXT2);
    ret |= _phy_drv_init(PHY_TYPE_EXT3);
    ret |= _phy_drv_init(PHY_TYPE_LPORT_SERDES);
    ret |= _phy_drv_init(PHY_TYPE_53125);

    return ret;
}
EXPORT_SYMBOL(phy_drivers_init);

void phy_dev_prog(phy_dev_t *phy_dev, prog_entry_t *prog_entry)
{
    while (prog_entry->desc)
    {
#ifdef DEBUG
        printk("Configuring addr %d: ", phy_dev->addr);
        printk("%s - Writing 0x%04x to register 0x%02x\n", prog_entry->desc, prog_entry->val, prog_entry->reg);
#endif

        if (phy_dev_write(phy_dev, prog_entry->reg, prog_entry->val) != 0)
            break;

        prog_entry++;
    }
}

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
		return "Unknown mii type";
	}
}

char *phy_dev_speed_to_str(phy_speed_t speed)
{
	switch (speed)
    {
	case PHY_SPEED_10:
		return "10 Mbps";
	case PHY_SPEED_100:
		return "100 Mbps";
	case PHY_SPEED_1000:
		return "1000 Mbps";
	case PHY_SPEED_2500:
		return "2.5 Gbps";
	case PHY_SPEED_5000:
		return "5 Gbps";
	case PHY_SPEED_10000:
		return "10 Gbps";
	default:
		return "Unknown speed";
	}
}

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

void phy_dev_print_status(phy_dev_t *phy_dev)
{
    printk("%s:%s:0x%x - ", phy_dev->phy_drv->name, phy_dev_mii_type_to_str(phy_dev->mii_type), phy_dev->addr);

    if (phy_dev->link)
    {
        printk("Link Up %s %s duplex\n",
            phy_dev_speed_to_str(phy_dev->speed),
            phy_dev_duplex_to_str(phy_dev->duplex));
    }
    else
    {
        printk("Link Down\n");
	}
}
EXPORT_SYMBOL(phy_dev_print_status);

static phy_dev_t phy_devices[MAX_PHY_DEVS] = {};

phy_dev_t *phy_dev_get(phy_type_t phy_type, uint32_t addr)
{
    uint32_t i;
    phy_dev_t *phy_dev = NULL;

    for (i = 0; i < MAX_PHY_DEVS; i++)
    {
        if (phy_devices[i].phy_drv == NULL)
            continue;

        if (phy_devices[i].phy_drv->phy_type != phy_type)
            continue;

        if (phy_devices[i].addr != addr)
            continue;

        phy_dev = &phy_devices[i];
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

    if (!(phy_drv = phy_drv_get(phy_type)))
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

    if (phy_drv_dev_add(phy_dev))
    {
        printk("Failed to add phy device to the driver: %s:%d\n", phy_drv->name, addr);
        phy_dev_del(phy_dev);
        return NULL;
    }

    return phy_dev;
}
EXPORT_SYMBOL(phy_dev_add);

int phy_dev_del(phy_dev_t *phy_dev)
{
    phy_drv_dev_del(phy_dev);
#ifndef _CFE_
    phy_dev_link_change_unregister(phy_dev);
#endif
    memset(phy_dev, 0, sizeof(phy_dev_t));

    return 0;
}
EXPORT_SYMBOL(phy_dev_del);

#ifndef _CFE_
void phy_dev_link_change_notify(phy_dev_t *phy_dev)
{
    int old_link = phy_dev->link;

    phy_dev_read_status(phy_dev);

    if (phy_dev->link != old_link)
    {
        if (phy_dev->link_change_cb)
            phy_dev->link_change_cb(phy_dev->link_change_ctx);

        if (phy_dev->phy_drv->link_change_handler)
            phy_dev->phy_drv->link_change_handler(phy_dev);
    }
}

static void phy_devices_link_change_notify(void)
{
    uint32_t i;

    for (i = 0; i < MAX_PHY_DEVS; i++)
    {
        if (phy_devices[i].phy_drv == NULL)
            continue;

        if (phy_devices[i].addr == 0)
            continue;

        if (phy_devices[i].link_change_cb == NULL)
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

static void phy_link_timer_cb(unsigned long data)
{
    schedule_work(&_work);
}

static void phy_link_timer_start(void)
{
    init_timer(&phy_link_timer);
    phy_link_timer.function = phy_link_timer_cb;
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

#define PROC_DIR           "driver/phy"
#define CMD_PROC_FILE      "cmd"

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *cmd_proc_file;

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

static int phy_proc_cmd_list(int argc, char *argv[])
{
    int id;
    uint16_t phyid1, phyid2;
    phy_dev_t *phy_dev;

    printk("|=========================================================================|\n");
    printk("|  Id |  State  |  Phy   |  Bus   | Addr |   Speed   | Duplex |   PHYID   |\n");
    printk("|=========================================================================|\n"); 

    for (id = 0; id < MAX_PHY_DEVS; id++)
    {
        printk("| %2d  ", id);

        phy_dev = &phy_devices[id];
        if (phy_dev->phy_drv == NULL)
        {
            printk("|         |        |        |      |           |        |           |\n");
            continue;
        }

        if ((phy_bus_read(phy_dev, MII_PHYSID1, &phyid1)) || phy_bus_read(phy_dev, MII_PHYSID2, &phyid2))
            phyid1 = phyid2 = 0;

        printk("| %s  ", phy_dev->link ? " Up   " : " Down ");
        printk("| %6s ", phy_dev->phy_drv->name);
        printk("| %6s ", phy_dev_mii_type_to_str(phy_dev->mii_type));
        printk("| 0x%02x ", phy_dev->addr);
        printk("| %9s ", phy_dev->speed == PHY_SPEED_UNKNOWN ? "" : phy_dev_speed_to_str(phy_dev->speed));
        printk("| %6s ", phy_dev->duplex == PHY_DUPLEX_UNKNOWN ? "" : phy_dev_duplex_to_str(phy_dev->duplex));
        printk("| %04x:%04x ", phyid1, phyid2); 
        printk("|\n");
    }
    printk("|=========================================================================|\n"); 

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

    if (phy_bus_c45_read(phy_dev, dev, reg, &val))
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

    if (phy_bus_c45_write(phy_dev, dev, reg, val))
        goto Error;

    printk("Write45 dev=%d register 0x%04x=0x%04x\n", dev, reg, val);

    return 0;

Error:
    printk("Usage: write45 <id> <dev> <reg> <val>\n");
    return 0;
}

static struct proc_cmd_ops command_entries[] = {
    { .name = "status", .do_command	= phy_proc_cmd_status},
    { .name = "init", .do_command	= phy_proc_cmd_init},
    { .name = "list", .do_command	= phy_proc_cmd_list},
    { .name = "read", .do_command	= phy_proc_cmd_read},
    { .name = "write", .do_command	= phy_proc_cmd_write},
    { .name = "read45", .do_command	= phy_proc_cmd_read45},
    { .name = "write45", .do_command	= phy_proc_cmd_write45},
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
postcore_initcall(phy_proc_init);
#endif
