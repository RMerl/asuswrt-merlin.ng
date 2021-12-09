/*
   <:copyright-BRCM:2015:DUAL/GPL:standard
   
      Copyright (c) 2015 Broadcom 
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

#include "port.h"
#include "phy_drv.h"
#include "enet.h"
#include "crossbar_dev.h"
#include <net_port.h>
#include <proc_cmd.h>

extern int phy_devices_internal_index(phy_dev_t *phy_dev);
extern int mac_devices_internal_index(mac_dev_t *mac_dev);

static const char *port_type_string[] =
{
    FOREACH_PORT_TYPE(GENERATE_STRING)
};

static const char *net_port_type_string[] =
{
    FOREACH_NET_PORT_STRING
};

#define PROC_DIR           "driver/enet"
#define CMD_PROC_FILE      "cmd"

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *cmd_proc_file;


#ifdef ENET_DEBUG_RX_CPU_TRAFFIC
#define DEBUG_MODE_PROC_FILE        "debug_mode"

static struct proc_dir_entry *debug_cmd_proc_file;

#ifdef CONFIG_ARM64
uint64_t g_debug_mode_pckt_rx;
#else
uint32_t g_debug_mode_pckt_rx;
#endif

uint32_t g_debug_mode;
struct timeval g_start_time;
struct timeval g_end_time;

int debug_mode_set(int argc, char *argv[])
{
    char *usage = "Usage: set_debug_mode <[<mode 0|1>]";
    unsigned int debug_mode = 0xffffffff;
    int rc = 0;

    if (argc < 2)
        goto wrong_params;

    if (kstrtos32(argv[1], 10, &debug_mode))
        goto wrong_params;


    if (debug_mode > 1)
        goto wrong_params; 
    
    
    g_debug_mode = debug_mode;

    printk("\x1b[32m DEBUG_MODE[%d]: \x1b[0m\n",g_debug_mode);

    return rc;


wrong_params:
    pr_info("%s\n", usage);
    return rc;

}

int debug_mode_get_pps(int argc, char *argv[])
{
    char *usage = "Usage: get_pps";
    int rc = 0;
#ifdef CONFIG_ARM64
    uint64_t time_diff;
#else
    uint32_t time_diff;
#endif
    
    if (argc < 1)
        goto wrong_params;
    
    do_gettimeofday(&g_end_time);
    
    time_diff = (g_end_time.tv_sec - g_start_time.tv_sec) + (g_end_time.tv_usec - g_start_time.tv_usec) / 1000000;

#ifdef CONFIG_ARM64
    printk("\x1b[32m DEBUG_MODE: pckt_rx[%lld] end_time[%ldsec] start_time[%ldsec] PPS[%lld]\x1b[0m\n", 
#else
    printk("\x1b[32m DEBUG_MODE: pckt_rx[%d] end_time[%ldsec] start_time[%ldsec] PPS[%d]\x1b[0m\n", 
#endif
           g_debug_mode_pckt_rx,
           g_end_time.tv_sec,
           g_start_time.tv_sec,
           g_debug_mode_pckt_rx / time_diff);

    memcpy(&g_start_time, &g_end_time, sizeof(struct timespec)); 
    g_debug_mode_pckt_rx = 0;
    
    return rc;                      


wrong_params:
    pr_info("%s\n", usage);
    return rc;
}

static struct proc_cmd_ops debug_mode_command_entries[] = 
{
    { .name = "set_debug_mode",  .do_command = debug_mode_set },
    { .name = "get_pps_measure", .do_command = debug_mode_get_pps},
};

static struct proc_cmd_table debug_mode_command_table = 
{
    .module_name = "debug_mode_masters",
    .size = sizeof(debug_mode_command_entries) / sizeof(debug_mode_command_entries[0]),
    .ops = debug_mode_command_entries
};
#endif

static int port_proc_cmd_status(int argc, char *argv[])
{
    int id;

    if (argc < 2)
        goto Error;

    if (kstrtos32(argv[1], 10, &id) || id < 0 || id >= 18)
        goto Error;

    /* XXX: Do something with port here */

    return 0;

Error:
    printk("Usage: status <id>\n");
    return 0;
}

int crossbar_info_by_phy(phy_dev_t *phy_dev, int *crossbar_id, int *internal_endpoint, int *external_endpoint);

static int tr_port_list(enetx_port_t *p, void *_ctx)
{
    char *class_str;
    int mac_internal_index, ext_ep, int_ep, phy_internal_index;
    
    switch (p->port_class)
    {
        case PORT_CLASS_SW: class_str = "SW"; break;
        case PORT_CLASS_PORT: class_str = "PORT"; break;
        case PORT_CLASS_PORT_DETECT: class_str = "DTCT"; break;
        default: class_str = "ERR";
    }

    pr_cont("| %4s ", class_str);
    pr_cont("| %9s  ", p->obj_name);
    pr_cont("| %10s  ", p->dev ? p->dev->name : "-");
    pr_cont("| %s     ", !p->is_init ? "y" : " ");
    pr_cont("| %13s  ", port_type_string[p->port_type] + sizeof("PORT_TYPE_") - 1);

    if (p->port_class == PORT_CLASS_PORT ||
        p->port_class == PORT_CLASS_PORT_DETECT)
    {
        pr_cont("| %9s  ", p->p.parent_sw ? p->p.parent_sw->obj_name : "");
        pr_cont("| %02d  ", p->p.port_id);
        
        if (p->p.phy)
        {
            if (phy_is_crossbar(p->p.phy))
            {
                if (!crossbar_info_by_phy(get_active_phy(p->p.phy), NULL, &int_ep, &ext_ep))
                    pr_cont("| E%02d ", ext_ep);
                else
                    pr_cont("| E-- ");
            }
            else
            {
                phy_internal_index = phy_devices_internal_index(p->p.phy);
                if (phy_internal_index >= 0)
                    pr_cont("| %02d  ", phy_internal_index);
            }
        }
        else
            pr_cont("| %2s  ", "");
        
        mac_internal_index = p->p.mac ? mac_devices_internal_index(p->p.mac) : -1;
        if (mac_internal_index >= 0)
            pr_cont("| %02d  ", mac_internal_index);
        else
            pr_cont("| %2s  ", "");

        if (p->p.ops && p->p.ops->print_priv)
            pr_cont("| %17s  ", p->p.ops->print_priv(p));
        else
            pr_cont("| %17s  ", "");
    }
    if (p->port_class == PORT_CLASS_SW)
    {
        pr_cont("| %9s  ", p->s.parent_port ? p->s.parent_port->obj_name : "");
        pr_cont("| %2s  ", "");
        pr_cont("| %2s  ", "");
        pr_cont("| %2s  ", "");
        if (p->s.ops->print_priv)
            pr_cont("| %17s  ", p->s.ops->print_priv(p));
        else
            pr_cont("| %17s  ", "");
    }

    pr_cont("|\n");

    return 0;
}

static int port_proc_cmd_list(int argc, char *argv[])
{
    printk("|==============================================================================================================|\n");
    printk("| Clss |  Name      | Dev Name    | !Init |  Type          |  Parent    |PrtID| PHY | MAC | Extra              |\n");
    printk("|==============================================================================================================|\n");
        
    port_traverse_ports(root_sw, tr_port_list, PORT_CLASS_SW | PORT_CLASS_PORT | PORT_CLASS_PORT_DETECT, NULL);

    printk("|==============================================================================================================|\n");

    return 0;
}

static int port_proc_cmd_if_phy_pwr_sync(int argc, char *argv[])
{
    if (argc == 1)
        printk("interface state PHY power sync = %s\n", g_enet_flags & ENET_FLAG_IF_PHY_PWR_SYNC_DISABLE ? "disable" : "enable");
    else if (!strcmp(argv[1], "disable"))
        g_enet_flags |= ENET_FLAG_IF_PHY_PWR_SYNC_DISABLE;
    else if (!strcmp(argv[1], "enable"))
        g_enet_flags &= ~ENET_FLAG_IF_PHY_PWR_SYNC_DISABLE;
    else
        goto Error;

    return 0;

Error:
    printk("Usage: if_phy_pwr_sync <enable|disable> set PHY power behavior in sync with i/f state\n"
           "       if_phy_pwr_sync                  get current PHY power behavior\n");
    return 0;
}

int enet_get_next_port(enetx_port_t *port, enetx_port_t **next_port);
static ssize_t proc_get_crossbar_status(struct file *file, char *buff, size_t len, loff_t *offset)
{
#if defined(CONFIG_BCM963158)
    static const char *intPortName[] = {"Switch Port 4", "Switch Port 6", "WAN Port"};
    static const char *extPortName[]= {"AE", "GPHY4", "RGMII1", "RGMII0"};
#endif
#if defined(CONFIG_BCM963138)
    static const char *intPortName[] = {"Switch Port 3", "Switch Port 4", "WAN Port"};
    static const char *extPortName[]= {"AE", "GPHY4", "RGMII3", "xMII", "GPHY3"};
#endif
#if defined(CONFIG_BCM963148)
    static const char *intPortName[] = {"Switch Port 4", "WAN Port"};
    static const char *extPortName[]= {"AE", "GPHY4", "RGMII3", "xMII"};
#endif
#if defined(CONFIG_BCM94908)
    static const char *intPortName[] = {"Switch Port 7", "WAN Port"};
    static const char *extPortName[]= {"AE", "GPHY4", "xMII"};
#endif

#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908)
    phy_dev_t *phy_crossbar;
    int extPort, intPort;
    enetx_port_t *port;

    if (*offset)
        return 0;

    for (enet_get_next_port(NULL, &port); port; enet_get_next_port(port, &port))
    {
        phy_crossbar = port->p.phy;
        if (!phy_is_crossbar(phy_crossbar))
            continue;
        crossbar_current_status(phy_crossbar, &intPort, &extPort);
        *offset += sprintf(buff + *offset, "%s is connected to: %s at Port %d\n", 
            intPortName[intPort], extPortName[extPort], 
            extPort+BP_CROSSBAR_PORT_BASE);

    }
    return *offset;
#else
    if (*offset)
        return 0;

    *offset += sprintf(buff, "Not Supported\n");
    return *offset;
#endif
}

#define PROC_ETHERNET_DIR           "ethernet"
#define PROC_CROSSBAR_STATUS        "crossbar_status"
static const struct file_operations crossbar_status_fops = {
    .read  = proc_get_crossbar_status,
};

static int port_proc_cmd_net_port(int argc, char *argv[])
{
    int i;

    printk("|=================|\n");
    printk("|    |  Name      |\n");
    printk("|=================|\n");
        
    for (i = NET_PORT_LAN_0; i < NET_PORT_LAST; i++)
        printk("| %2d | %s\n", i, net_port_type_string[i] + sizeof("NET_") - 1);
    
    for (i = NET_PORT_SUBTYPE_GPON; i < NET_PORT_SUBTYPE_LAST; i++)
        printk("| %2d | %s\n", i, net_port_type_string[NET_PORT_NONE + 1 + i] + sizeof("NET_PORT_") - 1); 

    for (i = NET_PORT_SPEED_0101; i < NET_PORT_SPEED_LAST; i++)
        printk("| %2d | %s\n", i, net_port_type_string[NET_PORT_NONE + 1 + NET_PORT_SUBTYPE_NONE + 1 + i] + sizeof("NET_PORT_") - 1); 

    return 0;
}

static int port_proc_cmd_dump(int argc, char *argv[])
{
    enet_sys_dump();
    return 0;
}

/* Command usage: echo "<name> <arg>" > /proc/driver/enet/cmd */
static struct proc_cmd_ops command_entries[] = {
    { .name = "status",   .do_command = port_proc_cmd_status},
    { .name = "list",     .do_command = port_proc_cmd_list},
    { .name = "net_port", .do_command = port_proc_cmd_net_port},
    { .name = "dump",     .do_command = port_proc_cmd_dump},
    { .name = "if_phy_pwr_sync",   .do_command = port_proc_cmd_if_phy_pwr_sync},
};

static struct proc_cmd_table port_command_table = {
    .module_name = "ENET",
    .size = ARRAY_SIZE(command_entries),
    .ops = command_entries,
};

void enet_proc_exit(void)
{
    if (cmd_proc_file) 
    {
        remove_proc_entry(CMD_PROC_FILE, proc_dir);
        cmd_proc_file = NULL;
    }	
#ifdef ENET_DEBUG_RX_CPU_TRAFFIC
    if (debug_cmd_proc_file) 
    {
        remove_proc_entry(DEBUG_MODE_PROC_FILE, proc_dir);
        debug_cmd_proc_file = NULL;
    }
#endif
    if (proc_dir)
    {
        remove_proc_entry(PROC_DIR, NULL);
        proc_dir = NULL;
    }	
}

int __init enet_proc_init(void)
{
    proc_dir = proc_mkdir(PROC_DIR, NULL);
    if (!proc_dir) 
    {
        pr_err("Failed to create PROC directory %s.\n",
            PROC_DIR);
        goto error;
    }

    cmd_proc_file = proc_create_cmd(CMD_PROC_FILE, proc_dir,
        &port_command_table);
    if (!cmd_proc_file) 
    {
        pr_err("Failed to create %s\n", CMD_PROC_FILE);
        goto error;
    }

#ifdef ENET_DEBUG_RX_CPU_TRAFFIC
    debug_cmd_proc_file = proc_create_cmd(DEBUG_MODE_PROC_FILE, proc_dir, &debug_mode_command_table);
    if (!debug_cmd_proc_file) 
    {
        pr_err("Failed to create %s\n", DEBUG_MODE_PROC_FILE);
        goto error;
    }

    g_debug_mode = 0;
    g_debug_mode_pckt_rx = 0;
    do_gettimeofday(&g_start_time);
#endif
    proc_dir = proc_mkdir(PROC_ETHERNET_DIR, NULL);
    if (!proc_create(PROC_CROSSBAR_STATUS, 0644, proc_dir, &crossbar_status_fops))
		goto error;
  
    return 0;

error:
    if (proc_dir)
        enet_proc_exit();

    return -EIO;
}

