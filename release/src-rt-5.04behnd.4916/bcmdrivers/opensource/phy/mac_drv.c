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
 *  Created on: Jan 2016
 *      Author: yuval.raviv@broadcom.com
 */

#include "mac_drv.h"

#define MAX_MAC_DEVS 16

extern mac_drv_t mac_drv_unimac;
extern mac_drv_t mac_drv_lport;
extern mac_drv_t mac_drv_sf2;
#ifdef MAC_XPORT
extern mac_drv_t mac_drv_xport;
#endif

mac_drv_t *mac_drivers[MAC_TYPE_MAX] = {};

int mac_driver_set(mac_drv_t *mac_drv)
{
    if (mac_drivers[mac_drv->mac_type])
    {
        printk("Failed adding mac driver %s: already set\n", mac_drv->name);
        return -1;
    }
    else
    {
        mac_drivers[mac_drv->mac_type] = mac_drv;
        return 0;
    }
}
EXPORT_SYMBOL(mac_driver_set);

static int mac_drv_init(mac_drv_t *mac_drv)
{
    if (mac_drv->initialized)
        return 0;

    if (!mac_drv->drv_init)
        return 0;

    return mac_drv->drv_init(mac_drv);
}

int mac_driver_init(mac_type_t mac_type)
{
    mac_drv_t *mac_drv;

    if (!(mac_drv = mac_drivers[mac_type]))
        return 0;

    return mac_drv_init(mac_drv);
}
EXPORT_SYMBOL(mac_driver_init);

int mac_drivers_set(void)
{
    int ret = 0;

#ifdef MAC_UNIMAC
    ret |= mac_driver_set(&mac_drv_unimac);
#endif
#ifdef MAC_LPORT
    ret |= mac_driver_set(&mac_drv_lport);
#endif
#if defined(MAC_SF2) || defined(MAC_SF2_EXTERNAL) || defined(MAC_SF2_DUAL)
    ret |= mac_driver_set(&mac_drv_sf2);
#endif
#ifdef MAC_XPORT
    ret |= mac_driver_set(&mac_drv_xport);
#endif

    return ret;
}
EXPORT_SYMBOL(mac_drivers_set);

void mac_driver_unset(mac_type_t mac_type)
{
    mac_drivers[mac_type] = NULL;
}
EXPORT_SYMBOL(mac_driver_unset);

int mac_drivers_init(void)
{
    int ret = 0;

    ret |= mac_driver_init(MAC_TYPE_UNIMAC);
    ret |= mac_driver_init(MAC_TYPE_LPORT);
    ret |= mac_driver_init(MAC_TYPE_GMAC);
    ret |= mac_driver_init(MAC_TYPE_SF2);
    ret |= mac_driver_init(MAC_TYPE_XPORT);

    return ret;
}
EXPORT_SYMBOL(mac_drivers_init);

static mac_dev_t mac_devices[MAX_MAC_DEVS] = {};

/* For internal use only by proc interface */
int mac_devices_internal_index(mac_dev_t *mac_dev)
{
    int64_t i = mac_dev - &mac_devices[0];
    if (i < 0 || i >= MAX_MAC_DEVS) return -1;

    return i;
}
EXPORT_SYMBOL(mac_devices_internal_index);

#if !defined(MAC_SF2_DUAL)   // 6756+53134 has 2 SF2s so mac_id will collide
static mac_dev_t *mac_dev_get(mac_type_t mac_type, int mac_id)
{
    int i;
    mac_dev_t *mac_dev = NULL;

    for (i = 0; i < MAX_MAC_DEVS; i++)
    {
        if (!mac_devices[i].mac_drv)
            continue;

        if (mac_devices[i].mac_drv->mac_type != mac_type)
            continue;

        if (mac_devices[i].mac_id != mac_id)
            continue;

        mac_dev = &mac_devices[i];
        break;
    }

    return mac_dev;
}
#endif

mac_dev_t *mac_dev_add(mac_type_t mac_type, int mac_id, void *priv)
{
    uint32_t i;
    mac_drv_t *mac_drv = NULL;
    mac_dev_t *mac_dev = NULL;

    if (!(mac_drv = mac_drivers[mac_type]))
    {
        printk("Failed to find MAC driver: mac_type=%d\n", mac_type);
        return NULL;
    }

#if !defined(MAC_SF2_DUAL)   // 6756+53134 has 2 SF2s so mac_id will collide
    if ((mac_dev = mac_dev_get(mac_type, mac_id)))
    {
        printk("Mac device already exists: %s:%d\n", mac_drv->name, mac_id);
        return NULL;
    }
#endif

    for (i = 0; i < MAX_MAC_DEVS && mac_devices[i].mac_drv != NULL; i++);

    if (i ==  MAX_MAC_DEVS)
    {
        printk("Failed adding mac device: %s:%d\n", mac_drv->name, mac_id);
        return NULL;
    }

    mac_dev = &mac_devices[i];
    
    mac_dev->mac_drv = mac_drv;
    mac_dev->mac_id = mac_id;
    mac_dev->priv = priv;

    if (mac_drv_dev_add(mac_dev))
    {
        printk("Failed to add MAC device to the driver: %s:%d\n", mac_drv->name, mac_id);
        mac_dev_del(mac_dev);
        return NULL;
    }

    return mac_dev;
}
EXPORT_SYMBOL(mac_dev_add);

int mac_dev_del(mac_dev_t *mac_dev)
{
    mac_drv_dev_del(mac_dev);
    memset(mac_dev, 0, sizeof(mac_dev_t));

    return 0;
}
EXPORT_SYMBOL(mac_dev_del);

char *mac_dev_speed_to_str(mac_speed_t speed)
{
	switch (speed)
    {
	case MAC_SPEED_10:
		return "10 Mbps";
	case MAC_SPEED_100:
		return "100 Mbps";
	case MAC_SPEED_1000:
		return "1 Gbps";
	case MAC_SPEED_2500:
		return "2.5 Gbps";
	case MAC_SPEED_5000:
		return "5 Gbps";
	case MAC_SPEED_10000:
		return "10 Gbps";
	default:
		return "Unknown";
	}
}
EXPORT_SYMBOL(mac_dev_speed_to_str);

char *mac_dev_speed_to_short_str(mac_speed_t speed)
{
    switch (speed)
    {
        case MAC_SPEED_10:
            return "10Mb";
        case MAC_SPEED_100:
            return "100M";
        case MAC_SPEED_1000:
            return "1G";
        case MAC_SPEED_2500:
            return "2.5G";
        case MAC_SPEED_5000:
            return "5G";
        case MAC_SPEED_10000:
            return "10G";
        default:
            return "Unkn";
    }
}
EXPORT_SYMBOL(mac_dev_speed_to_short_str);

char *mac_dev_duplex_to_str(mac_duplex_t duplex)
{
	switch (duplex)
    {
	case MAC_DUPLEX_HALF:
		return "Half";
	case MAC_DUPLEX_FULL:
		return "Full";
	default:
		return "Unknown";
    }
}
EXPORT_SYMBOL(mac_dev_duplex_to_str);

char *mac_dev_flowctrl_to_str(int pause_rx, int pause_tx)
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
EXPORT_SYMBOL(mac_dev_flowctrl_to_str);

int mac_proc_cmd_list(void)
{
    int id;

	printk("|======================================================================================|\n");
	printk("|  Id |  Link 	|   Mac   | Addr |   Speed   | Duplex | Flowctl  | RX Pkt  |   TX Pkt  |\n");
	printk("|======================================================================================|\n");

	for (id = 0; id < MAX_MAC_DEVS; id++)
    {
		mac_dev_t *mac_dev =  &mac_devices[id];
		mac_stats_t *mac_stats = &(mac_dev->stats);
        mac_status_t mac_status = {};

        if (!mac_dev->mac_drv)
            continue;

        if (mac_dev->mac_drv->initialized)
        {
            if (mac_dev_read_status(mac_dev, &mac_status))
            {
                printk("Failed to get MAC status\n");
                continue;
            }

            if (mac_dev_stats_get(mac_dev, mac_stats))
            {
                printk("Failed to get MAC statistics\n");
                continue;
            }
        }

		pr_cont("| %2d  ", id);
		pr_cont("|  %-5s  ", mac_status.link ? "Up" : "Down");
		pr_cont("| %-7s ", mac_dev->mac_drv->name);
		pr_cont("| 0x%02x ", mac_dev->mac_id);
		pr_cont("| %9s ", !mac_status.link || mac_status.speed == MAC_SPEED_UNKNOWN ? "" : mac_dev_speed_to_str(mac_status.speed));
		pr_cont("| %-5s  ", !mac_status.link || mac_status.duplex == MAC_DUPLEX_UNKNOWN ? "" : mac_dev_duplex_to_str(mac_status.duplex));
		pr_cont("|  %-4s	 ", mac_status.link ? mac_dev_flowctrl_to_str(mac_status.pause_rx, mac_status.pause_tx) : "");
		pr_cont("| %08lld ", mac_stats->rx_packet);
		pr_cont("| %08lld ", mac_stats->tx_packet);
		pr_cont("|\n");
	}

	printk("|======================================================================================|\n");

    return 0;
}

int mac_proc_cmd_stats(int id)
{
    mac_dev_t *mac_dev = NULL;
    mac_stats_t *mac_stats = NULL;

    if (id < 0 || id >= MAX_MAC_DEVS)
        return -1;

    mac_dev =  &mac_devices[id];
    mac_stats = &(mac_dev->stats);

    if (!mac_dev->mac_drv)
        return -1;

    if (mac_dev_stats_get(mac_dev, mac_stats))
        return -1;

    printk("rx_byte                         %20llu\n", mac_stats->rx_byte);
    printk("rx_packet                       %20llu\n", mac_stats->rx_packet);
    printk("rx_frame_64                     %20llu\n", mac_stats->rx_frame_64);
    printk("rx_frame_65_127                 %20llu\n", mac_stats->rx_frame_65_127);
    printk("rx_frame_128_255                %20llu\n", mac_stats->rx_frame_128_255);
    printk("rx_frame_256_511                %20llu\n", mac_stats->rx_frame_256_511);
    printk("rx_frame_512_1023               %20llu\n", mac_stats->rx_frame_512_1023);
    printk("rx_frame_1024_1518              %20llu\n", mac_stats->rx_frame_1024_1518);
    printk("rx_frame_1519_mtu               %20llu\n", mac_stats->rx_frame_1519_mtu);
    printk("rx_multicast_packet             %20llu\n", mac_stats->rx_multicast_packet);
    printk("rx_broadcast_packet             %20llu\n", mac_stats->rx_broadcast_packet);
    printk("rx_unicast_packet               %20llu\n", mac_stats->rx_unicast_packet);
    printk("rx_alignment_error              %20llu\n", mac_stats->rx_alignment_error);
    printk("rx_frame_length_error           %20llu\n", mac_stats->rx_frame_length_error);
    printk("rx_code_error                   %20llu\n", mac_stats->rx_code_error);
    printk("rx_carrier_sense_error          %20llu\n", mac_stats->rx_carrier_sense_error);
    printk("rx_fcs_error                    %20llu\n", mac_stats->rx_fcs_error);
    printk("rx_undersize_packet             %20llu\n", mac_stats->rx_undersize_packet);
    printk("rx_oversize_packet              %20llu\n", mac_stats->rx_oversize_packet);
    printk("rx_fragments                    %20llu\n", mac_stats->rx_fragments);
    printk("rx_jabber                       %20llu\n", mac_stats->rx_jabber);
    printk("rx_overflow                     %20llu\n", mac_stats->rx_overflow);
    printk("rx_control_frame                %20llu\n", mac_stats->rx_control_frame);
    printk("rx_pause_control_frame          %20llu\n", mac_stats->rx_pause_control_frame);
    printk("rx_unknown_opcode               %20llu\n", mac_stats->rx_unknown_opcode);
    printk("rx_fifo_errors                  %20llu\n", mac_stats->rx_fifo_errors);
    printk("rx_dropped                      %20llu\n", mac_stats->rx_dropped);
    printk("\n");
    printk("tx_byte                         %20llu\n", mac_stats->tx_byte);
    printk("tx_packet                       %20llu\n", mac_stats->tx_packet);
    printk("tx_frame_64                     %20llu\n", mac_stats->tx_frame_64);
    printk("tx_frame_65_127                 %20llu\n", mac_stats->tx_frame_65_127);
    printk("tx_frame_128_255                %20llu\n", mac_stats->tx_frame_128_255);
    printk("tx_frame_256_511                %20llu\n", mac_stats->tx_frame_256_511);
    printk("tx_frame_512_1023               %20llu\n", mac_stats->tx_frame_512_1023);
    printk("tx_frame_1024_1518              %20llu\n", mac_stats->tx_frame_1024_1518);
    printk("tx_frame_1519_mtu               %20llu\n", mac_stats->tx_frame_1519_mtu);
    printk("tx_fcs_error                    %20llu\n", mac_stats->tx_fcs_error);
    printk("tx_multicast_packet             %20llu\n", mac_stats->tx_multicast_packet);
    printk("tx_broadcast_packet             %20llu\n", mac_stats->tx_broadcast_packet);
    printk("tx_unicast_packet               %20llu\n", mac_stats->tx_unicast_packet);
    printk("tx_total_collision              %20llu\n", mac_stats->tx_total_collision);
    printk("tx_jabber_frame                 %20llu\n", mac_stats->tx_jabber_frame);
    printk("tx_oversize_frame               %20llu\n", mac_stats->tx_oversize_frame);
    printk("tx_undersize_frame              %20llu\n", mac_stats->tx_undersize_frame);
    printk("tx_fragments_frame              %20llu\n", mac_stats->tx_fragments_frame);
    printk("tx_error                        %20llu\n", mac_stats->tx_error);
    printk("tx_underrun                     %20llu\n", mac_stats->tx_underrun);
    printk("tx_excessive_collision          %20llu\n", mac_stats->tx_excessive_collision);
    printk("tx_late_collision               %20llu\n", mac_stats->tx_late_collision);
    printk("tx_single_collision             %20llu\n", mac_stats->tx_single_collision);
    printk("tx_multiple_collision           %20llu\n", mac_stats->tx_multiple_collision);
    printk("tx_pause_control_frame          %20llu\n", mac_stats->tx_pause_control_frame);
    printk("tx_deferral_packet              %20llu\n", mac_stats->tx_deferral_packet);
    printk("tx_excessive_deferral_packet    %20llu\n", mac_stats->tx_excessive_deferral_packet);
    printk("tx_control_frame                %20llu\n", mac_stats->tx_control_frame);
    printk("tx_fifo_errors                  %20llu\n", mac_stats->tx_fifo_errors);
    printk("tx_dropped                      %20llu\n", mac_stats->tx_dropped);

    return 0;
}
EXPORT_SYMBOL(mac_proc_cmd_stats);


#define MIB_REFRESH_IN_SEC      6                           /* based on 32bit byte counter speed 5G byte rollover 6.871 second */
#define MIB_REFRESH_PERIOD      MIB_REFRESH_IN_SEC* 1000    /* in msec */

int mac_dev_get_stats_refresh_interval(mac_dev_t *mac_dev, int byte_cntr_width, int pkt_cntr_width, int speed)
{
    int interval = 0;
    if (!speed) return interval;                            /* link down */
        
    if (byte_cntr_width == 32)
    {
        interval = 5000 / speed;
        if (!interval) interval = 1;
    } else if (pkt_cntr_width == 32)
    {
        interval = 10000 / speed;
        interval *= (288 / MIB_REFRESH_IN_SEC);               /* 32bit pkt counter speed 10G rollover 288.621 second */
    } else {
                                                            /* next check pkt_cntr_width 40bit */
                                                            /* 40bit pkt counter speed 10G rollover 73877.180 second (20.5 hours) */ 
        interval = 7200 / MIB_REFRESH_IN_SEC;                 /* just refresh every 2 hours no matter what speed */
    }
    return interval;
}

static void mac_devices_stat_refresh(void)
{
    uint32_t i;

    for (i = 0; i < MAX_MAC_DEVS; i++)
    {
        mac_dev_t *mac_dev = &mac_devices[i];

        if (mac_dev->enabled && mac_dev->stats_refresh_interval && --mac_dev->stats_refresh_cnt_dn <= 0)
        {
            /* coverity[check_return] */
            mac_dev_stats_get(mac_dev, &mac_dev->stats);
        }
    }
}

static struct timer_list mac_stats_timer;

static void mac_stats_work_cb(struct work_struct *work)
{
    mac_devices_stat_refresh();
    mod_timer(&mac_stats_timer, jiffies + msecs_to_jiffies(MIB_REFRESH_PERIOD));
}

DECLARE_WORK(_mac_stats_work, mac_stats_work_cb);

static void mac_stats_timer_cb(struct timer_list *tl)
{
    schedule_work(&_mac_stats_work);
}

static void mac_stats_timer_start(void)
{
    timer_setup(&mac_stats_timer, mac_stats_timer_cb, 0);
    mac_stats_timer.expires = jiffies + msecs_to_jiffies(MIB_REFRESH_PERIOD);
    add_timer(&mac_stats_timer);
}

static int __init mac_init(void)
{
    mac_stats_timer_start();

    return 0;
}

late_initcall(mac_init);
