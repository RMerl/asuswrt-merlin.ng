/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2016:DUAL/GPL:standard

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
 * lport_proc.c
 *
 *  Created on: Jan 2016
 *  Author: Kosta Sopov kosta.sopov@broadcom.com
 */

#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/io.h>

#include "proc_cmd.h"
#include "lport_drv.h"
#include "lport_stats.h"

#define PROC_DIR           "driver/lport"
#define CMD_PROC_FILE      "cmd"

#define LPORT_MAX_PHY_ID 31
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *cmd_proc_file;

static int lport_port_validate(int argc, char *argv[], const char *usage, int min, int max)
{
    int port;

    if (argc < 2)
        goto error;

    if (kstrtos32(argv[1], 10, &port))
        goto error;

    if (port < min || port > max)
        goto error;

    return port;

error:
    pr_info("%s\n", usage);
    return -1;
}

static int lport_proc_cmd_init(int argc, char *argv[])
{
    char *usage = "Usage: init <mux_map>\nMux map: String, 1 char for every port (S|I|U|L|M|H|X|K|F|G|R)\n"
        "S-SGMII\nI-SGMII_AN_IEEE_CL37\nU-SGMII_AN_USER_CL37\nL-SGMII_AN_SLAVE\nM-SGMII_AN_MASTER\n"
        "H-HSGMII\nF-SFI\nK-KR\nX-XFI\nG-GPHY\nR-RGMII\nZ-Unavailabe\nExample: init GGGGXRRZ";
    lport_init_s init_params;
    int i, res;

    if (argc < 2)
    {
        pr_info("%s\n", usage);
        return 0;
    }
    if (strlen(argv[1]) != LPORT_NUM_OF_PORTS)
    {
        pr_err("Wrong params\n%s\n", usage);
        return 0;
    }

    for (i = 0; i <  LPORT_NUM_OF_PORTS; i++)
        switch (argv[1][i])
        {
        case 'S': init_params.prt_mux_sel[i] = PORT_SGMII; break;
        case 'H': init_params.prt_mux_sel[i] = PORT_HSGMII; break;
        case 'X': init_params.prt_mux_sel[i] = PORT_XFI; break;
        case 'G': init_params.prt_mux_sel[i] = PORT_GPHY; break;
        case 'R': init_params.prt_mux_sel[i] = PORT_RGMII; break;
        case 'Z': init_params.prt_mux_sel[i] = PORT_UNAVAIL; break;
        default: 
                  pr_err("Wrong map char \'%c\'\n%s\n", argv[1][i], usage);
                  return 0;
        }

    if (!(res = lport_init_driver(&init_params)))
        pr_info("lport init done\n");
    else
        pr_err("lport init failed; err %d\n", res);

    return 0;
}

static int lport_proc_cmd_rxtx_enable(int argc, char *argv[])
{
    char *usage = "Usage: rxtx_enable <port id (0-6)> [<rx_enable 0|1> <tx_enable 0|1>]";
    int port;
    uint8_t rx_enable, tx_enable;

    if ((port = lport_port_validate(argc, argv, usage, 0, 6)) == -1)
        return 0;

    if (argc == 2)
    {
        lport_get_port_rxtx_enable(port, &rx_enable, &tx_enable);
        pr_info("Port %d: rx_enable %d, tx_enable %d\n", port, rx_enable, tx_enable);
        return 0;
    }
    if (argc < 4)
        goto error;

    if (kstrtou8(argv[2], 10, &rx_enable) || kstrtou8(argv[3], 10, &tx_enable))
        goto error;

    lport_set_port_rxtx_enable(port, rx_enable, tx_enable);

    return 0;

error:
    pr_info("Wrong params\n%s\n", usage);
    return 0;
}

static int lport_proc_cmd_reset(int argc, char *argv[])
{
    char *usage = "Usage: reset <port id (0-6)>]";
    int port;

    if ((port = lport_port_validate(argc, argv, usage, 0, 6)) == -1)
        return 0;

    if (argc < 2)
        goto error;

    lport_port_credits_restart(port);

    return 0;

error:
    pr_info("Wrong params\n%s\n", usage);
    return 0;
}

static int lport_proc_cmd_rate(int argc, char *argv[])
{
    int port;
    lport_port_cfg_s port_conf;
    LPORT_PORT_RATE rate; 
    char *lport_rate_usage = "Usage: rate <port id (0-6)> [10M|100M|1G|2.5G|10G]";

    if ((port = lport_port_validate(argc, argv, lport_rate_usage, 0, 6)) == -1)
        return 0;

    lport_get_port_configuration(port, &port_conf);

    if (argc == 2)
    {
        pr_info("Port %d rate: %s\n", port, lport_rate_to_str(port_conf.speed));
        return 0;
    }

    if ((rate = lport_str_to_rate(argv[2])) == LPORT_RATE_UNKNOWN)
    {
        pr_info("Wrong param\n%s\n", lport_rate_usage);
        return 0;
    }

    port_conf.speed = rate;
    lport_set_port_configuration(port, &port_conf);

    return 0;
}

static int lport_proc_cmd_stats(int argc, char *argv[])
{
    int port, clean = 0;
    lport_rx_stats_s rx_stats;
    lport_tx_stats_s tx_stats;
    char *lport_stats_usage = "Usage: lport_stats <port id (0-6)> [\"clean\"]";

    if ((port = lport_port_validate(argc, argv, lport_stats_usage, 0, 6)) == -1)
        return -1;

    if (argc >= 3)
    {
        if (strcmp(argv[2], "clean"))
        {
            pr_info("%s\n", lport_stats_usage);
            return 0;
        }
        clean = 1;
    }

    lport_stats_get_rx(port, &rx_stats);
    lport_stats_get_tx(port, &tx_stats);

    pr_info("\x1b[33m\t\tLPORT %d Statistics %s\x1B[0m\n\x1b[0m", port,clean ? "Clear":"");
    pr_info("+----------------+---------++----------------+---------+\n");
    pr_info("| Counter        | Value   || Counter        | Value   |\n");
    pr_info("+----------------+---------++----------------+---------+\n");
    pr_info("| Rx Packet OK   |%9llu|| Tx Packet OK   |%9llu|\n", rx_stats.GRXPOK, tx_stats.GTXPOK);
    pr_info("| Rx 64 Byte     |0x%07llx|| Tx 64 Byte     |0x%07llx|\n", rx_stats.GRX64, tx_stats.GTX64);
    pr_info("| Rx 65 to 127   |%9llu|| Tx 65 to 127   |%9llu|\n", rx_stats.GRX127, tx_stats.GTX127);
    pr_info("| Rx 128 to 255  |%9llu|| Tx 128 to 255  |%9llu|\n", rx_stats.GRX255, tx_stats.GTX255);
    pr_info("| Rx 256 to 511  |%9llu|| Tx 256 to 511  |%9llu|\n", rx_stats.GRX511, tx_stats.GTX511);
    pr_info("| Rx 512 to 1023 |%9llu|| Tx 512 to 1023 |%9llu|\n", rx_stats.GRX1023, tx_stats.GTX1023);
    pr_info("| Rx 1024 to 1518|%9llu|| Tx 1024 to 1518|%9llu|\n", rx_stats.GRX1518, tx_stats.GTX1518);
    pr_info("| Rx 1519 to 2047|%9llu|| Tx 1519 to 2047|%9llu|\n", rx_stats.GRX2047, tx_stats.GTX2047);
    pr_info("| Rx frame       |%9llu|| Tx frame       |%9llu|\n", rx_stats.GRXPKT, tx_stats.GTXPKT);
    pr_info("| Rx Bytes       |0x%07llx|| Tx Bytes       |0x%07llx|\n", rx_stats.GRXBYT, tx_stats.GTXBYT);
    pr_info("| Rx Unicast     |%9llu|| Tx Unicast     |%9llu|\n", rx_stats.GRXUCA, tx_stats.GTXUCA);
    pr_info("| Rx Multicast   |%9llu|| Tx Multicast   |%9llu|\n", rx_stats.GRXMCA, tx_stats.GTXUCA);
    pr_info("| Rx Broadcast   |%9llu|| Tx Broadcast   |%9llu|\n", rx_stats.GRXBCA, tx_stats.GTXBCA);
    pr_info("| Rx FCS Error   |%9llu|| Tx FCS Error   |%9llu|\n", rx_stats.GRXFCS, tx_stats.GTXFCS);
    pr_info("| Rx Control     |%9llu|| Tx Control     |%9llu|\n", rx_stats.GRXCF, tx_stats.GTXCF);
    pr_info("| Rx PAUSE       |%9llu|| Tx PAUSE       |%9llu|\n", rx_stats.GRXPF, tx_stats.GTXCF);
    pr_info("| Rx PFC         |%9llu|| Tx PFC         |%9llu|\n", rx_stats.GRXPP, tx_stats.GTXPFC);
    pr_info("| Rx Unsup Op    |%9llu|| Tx Oversized   |%9llu|\n", rx_stats.GRXUO, tx_stats.GTXOVR);
    pr_info("| Rx Unsup DA    |%9llu||                |         |\n", rx_stats.GRXUDA);
    pr_info("| Rx Wrong SA    |%9llu||                |         |\n", rx_stats.GRXWSA);
    pr_info("| Rx Align Error |%9llu||                |         |\n", rx_stats.GRXALN);
    pr_info("| Rx Length OOR  |%9llu||                |         |\n", rx_stats.GRXFLR);
    pr_info("| Rx Code Error  |%9llu||                |         |\n", rx_stats.GRXFRERR);
    pr_info("| Rx False Carri |%9llu||                |         |\n", rx_stats.GRXFCR);
    pr_info("| Rx Oversized   |%9llu||                |         |\n", rx_stats.GRXOVR);
    pr_info("| Rx Jabber      |%9llu||                |         |\n", rx_stats.GRXJBR);
    pr_info("| Rx MTU ERR     |%9llu||                |         |\n", rx_stats.GRXMTUE);
    pr_info("| Rx Truncated   |%9llu||                |         |\n", rx_stats.GRXTRFU);
    pr_info("| Rx SCH CRC     |%9llu||                |         |\n", rx_stats.GRXSCHCRC);
    pr_info("| Rx RUNT        |%9llu||                |         |\n", rx_stats.GRXRPKT);
    pr_info("| Rx RUNT Bytes  |0x%07llx||                |         |\n", rx_stats.GRXRBYT);
    pr_info("| Rx Undersize   |%9llu||                |         |\n", rx_stats.GRXUND);
    pr_info("| Rx Fragment    |%9llu||                |         |\n", rx_stats.GRXFRG);
    pr_info("+----------------+---------++----------------+---------+\n");

    if (clean)
        lport_stats_reset(port);

    return 0;
}

static struct proc_cmd_ops command_entries[] = {
    { .name = "init", .do_command	= lport_proc_cmd_init},
    {. name = "rxtx_enable", .do_command = lport_proc_cmd_rxtx_enable},
    {. name = "reset", .do_command = lport_proc_cmd_reset},
    { .name = "rate", .do_command	= lport_proc_cmd_rate},
    { .name = "stats", .do_command	= lport_proc_cmd_stats},
};

static struct proc_cmd_table lport_command_table = {
    .module_name = "LPORT",
    .size = ARRAY_SIZE(command_entries),
    .ops = command_entries
};

static void lport_proc_exit(void)
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

int __init lport_proc_init(void)
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
        &lport_command_table);
    if (!cmd_proc_file) 
    {
        pr_err("Failed to create %s\n", CMD_PROC_FILE);
        goto error;
    }

    return status;

error:
    if (proc_dir)
        lport_proc_exit();

    status = -EIO;
    return status;
}
late_initcall(lport_proc_init);
