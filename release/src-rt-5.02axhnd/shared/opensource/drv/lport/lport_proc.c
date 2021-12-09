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
#include "lport_mdio.h"
#include "serdes_access.h"

#define PROC_DIR           "driver/lport"
#define CMD_PROC_FILE      "cmd"
#define SERDES_PROC_FILE   "serdes_cmd"

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
static struct proc_dir_entry *serdes_cmd_proc_file;

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
    char *usage = "Usage: init <mux_map>\nMux map: String, 1 char for every port (U|S|B|H|X|G|R)\nU-Unavailabe\n"
        "S-SGMII\nB-SGMII_AN_IEEE_CL37\nH-HSGMII\nX-XFI\nG-GPHY\nR-RGMII\nExample: init GGGGXRRU";
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
        case 'B': init_params.prt_mux_sel[i] = PORT_SGMII_AN_IEEE_CL37; break;
        case 'H': init_params.prt_mux_sel[i] = PORT_HSGMII; break;
        case 'X': init_params.prt_mux_sel[i] = PORT_XFI; break;
        case 'G': init_params.prt_mux_sel[i] = PORT_GPHY; break;
        case 'R': init_params.prt_mux_sel[i] = PORT_RGMII; break;
        case 'U': init_params.prt_mux_sel[i] = PORT_UNAVAIL; break;
        default: 
                  pr_err("Wrong map char \'%c\'\n%s\n", argv[1][i], usage);
                  return 0;
        }

    if (!(res = lport_reinit_driver(&init_params, 1)))
        pr_info("lport reinit done\n");
    else
        pr_err("lport reinit failed; err %d\n", res);

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

static char *lport_duplex_to_str(LPORT_PORT_DUPLEX duplex)
{
    return duplex == LPORT_HALF_DUPLEX ? "Half duplex" : "Full duplex";
}

static int lport_proc_cmd_status(int argc, char *argv[])
{
    int port;
    lport_port_status_s port_status;
    char *lport_status_usage = "Usage: lport_status <port id (0-6)>";

    if ((port = lport_port_validate(argc, argv, lport_status_usage, 0, 6)) == -1)
        return -1;

    lport_get_port_status(port, &port_status);
    pr_info("Port %d status:\n", port);
    pr_info("\tautoneg_en: %d\n", port_status.autoneg_en);
    pr_info("\tport_up: %d\n", port_status.port_up);
    pr_info("\trate: %s\n", lport_rate_to_str(port_status.rate));
    pr_info("\tduplex: %s\n", lport_duplex_to_str(port_status.duplex));
    pr_info("\trx_pause_en: %d\n", port_status.rx_pause_en);
    pr_info("\ttx_pause_en: %d\n", port_status.tx_pause_en);

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

static int lport_proc_cmd_phy_reg(int argc, char *argv[])
{
    char *lport_phy_usage = "Usage: phy_reg <phy_id (0-31)> <register (hex)> [value (hex)]";
    int phyid, is_write_op = 0;
    uint16_t reg, val;

    if (argc < 3)
        goto error;

    if (kstrtos32(argv[1], 10, &phyid) || phyid < 0 || phyid > LPORT_MAX_PHY_ID)
        goto error;

    if (kstrtou16(argv[2], 16, &reg))
        goto error;

    if (argc > 3)
    {
        if (kstrtou16(argv[3], 16, &val))
            goto error;

        if (lport_mdio22_wr(phyid, reg, val))
        {
            pr_err("Failed to write register 0x%x\n", reg);
            return 0;
        }
        is_write_op = 1;
    }
    if (lport_mdio22_rd(phyid, reg, &val))
    {
        pr_err("Failed to read register 0x%x\n", reg);
        return -1;
    }
    pr_info("%s Phy Reg 0x%02x = 0x%04x\n", is_write_op ? "Write" : "Read", reg, val);
    return 0;

error:
    pr_info("%s\n", lport_phy_usage);
    return 0;
}

static int lport_proc_cmd_serdes_reg(int argc, char *argv[])
{
    char *lport_serdes_reg_usage = "Usage: serdes_reg <serdes id 0|1> <addr (hex)> [mask (hex)] [value (hex)]";
    int serdes_id;
    E_MERLIN_ID id;
    uint16_t reg_addr, mask = 0, value;

    if (argc < 3)
        goto error;

    if (kstrtos32(argv[1], 10, &serdes_id))
        goto error;

    switch (serdes_id)
    {
    case 0: id = MERLIN_ID_0; break;
    case 1: id = MERLIN_ID_1; break;
    default: goto error;
    }

    if (kstrtou16(argv[2], 16, &reg_addr))
        goto error;

    if (argc >= 4 && kstrtou16(argv[3], 16, &mask))
        goto error;

    if (argc < 5)
    {
        /* Register read was requested */
        read_serdes_reg(id, reg_addr, mask, &value);
        pr_info("0x%x\n", value);
        return 0;
    }

    /* Register write was requested */
    if (kstrtou16(argv[4], 16, &value))
        goto error;

    write_serdes_reg(id, reg_addr, mask, value);

    return 0;

error:
    pr_info("%s\n", lport_serdes_reg_usage);
    return -1;
}

static int lport_proc_cmd_serdes_tx_dis(int argc, char *argv[])
{
     char *serdes_tx_dis_usage = "Usage: serdes_tx_dis <port id (0-6)> <state 0|1>";
     int port;
     uint32_t  gpio_state = 0;

     if ( argc != 3 )
         goto error;

     if ((port = lport_port_validate(argc, argv, serdes_tx_dis_usage, 0, 6)) == -1)
         return 0;

     if (kstrtos32(argv[2], 10, &gpio_state))
         goto error;

     if (gpio_state > 1)
         goto error;

     port_write_tx_dis_state(port, gpio_state); 

     return 0;

error:

     pr_info("%s\n", serdes_tx_dis_usage);
     return -1;
}



extern int lport_serdes_set_loopback(uint32_t port, uint32_t loopback_mode, uint32_t enable);
extern int lport_serdes_diag(uint32_t port, uint32_t cmd);
extern int lport_serdes_read_reg(uint32_t port, uint16_t device_type, uint16_t reg_address);
extern int lport_serdes_write_reg(uint32_t port, uint16_t device_type, uint16_t reg_address, uint32_t value);

static int serdes_proc_cmd_reg(int argc, char *argv[])
{
    char *serdes_proc_cmd_reg_usage = "Usage: reg <port_id (0-6)> <device type> <addr (hex)> [value (hex)]";
    uint32_t port_id = 0xFFFFFFFF;
    uint16_t dev_type = 0;
    uint16_t addr = 0;
    uint16_t value = 0;

    if (argc < 4)
        goto wrong_params;

    if (kstrtou32(argv[1], 10, &port_id))
        goto wrong_params;

    if( port_id > 6)
        goto wrong_params;

    if (kstrtou16(argv[2], 10, &dev_type))
        goto wrong_params;


    if (kstrtou16(argv[3], 16, &addr))
        goto wrong_params;

    if (argc < 5)
    {
        /* read requested */
        return lport_serdes_read_reg(port_id, dev_type, addr);
    }

    if (kstrtou16(argv[4], 16, &value))
        goto wrong_params;

    return lport_serdes_write_reg(port_id, dev_type, addr, value);

wrong_params:
    pr_info("%s\n", serdes_proc_cmd_reg_usage);
    return -1;
}

static int serdes_proc_cmd_prbs_generation(int argc, char *argv[])
{
    char *wrong_params_msg = 
        "Usage: prbs <port_id (0-6)> <pattern_type(0-7)[|PRBS7|PRBS9|PRBS11|PRBS15|PRBS23|PRBS31|PRBS58|8180|]>";
      
    uint32_t port_id = 0xffffffff;
    uint32_t pattern_type = 0xffffffff;

    if (argc < 3)
        goto wrong_params;

    if (kstrtou32(argv[1], 10, &port_id))
        goto wrong_params;

    if( port_id > 6)
        goto wrong_params;


    if (kstrtou32(argv[2], 10, &pattern_type))
        goto wrong_params;

    if( pattern_type >= SERDES_PRBS_PATTERN_TYPE_LAST)
        goto wrong_params;

    return lport_serdes_prbs_generation(port_id, pattern_type);

wrong_params:
    pr_info("portId[%d] pattern_type[%d]\n", port_id, pattern_type);
    pr_info("%s\n", wrong_params_msg);
    return -1;
}


static int serdes_proc_cmd_prbs_monitor(int argc, char *argv[])
{
    char *wrong_params_msg = 
                "Usage: prbs <port_id (0-6)> <pattern_type(0-7)[|PRBS7|PRBS9|PRBS11|PRBS15|PRBS23|PRBS31|PRBS58|]>";
      
    uint32_t port_id = 0xffffffff;
    uint32_t pattern_type = 0xffffffff;

    if (argc < 3)
        goto wrong_params;

    if (kstrtou32(argv[1], 10, &port_id))
        goto wrong_params;

    if( port_id > 6)
        goto wrong_params;

    if (kstrtou32(argv[2], 10, &pattern_type))
        goto wrong_params;

    /* programable pattern not support in monitor mode */
    if( pattern_type > SERDE_SPRBS_PATTERN_TYPE_PRBS58)
        goto wrong_params;

    return lport_serdes_prbs_monitor(port_id, pattern_type);


wrong_params:
    pr_info("portId[%d] pattern_type[%d]\n", port_id, pattern_type);
    pr_info("%s\n", wrong_params_msg);
    return -1;
}

static int serdes_proc_cmd_prbs_stat(int argc, char *argv[])
{
    char *serdes_proc_cmd_prbs_usage = "Usage: prbs <port_id (0-6)>";
    uint32_t port_id;

    if (argc < 2)
        goto wrong_params;

    if (kstrtou32(argv[1], 10, &port_id))
        goto wrong_params;

    if( port_id > 6)
        goto wrong_params;

    return lport_serdes_prbs_stats_get(port_id);

wrong_params:
    pr_info("%s\n", serdes_proc_cmd_prbs_usage);
    return -1;
}


static int serdes_proc_cmd_stats(int argc, char *argv[])
{
    char *serdes_proc_cmd_stats_usage = "Usage: stats <port_id (0-6)>";
    uint32_t port_id;

    if (argc < 2)
        goto wrong_params;

    if (kstrtou32(argv[1], 10, &port_id))
        goto wrong_params;

    if( port_id > 6)
        goto wrong_params;

    return lport_serdes_diag(port_id, 2);

wrong_params:
    pr_info("%s\n", serdes_proc_cmd_stats_usage);
    return -1;
}

static int serdes_proc_cmd_status(int argc, char *argv[])
{
    char *serdes_proc_cmd_status_usage = "Usage: status <port_id (0-6)>";
    uint32_t port_id;

    if (argc < 2)
        goto wrong_params;

    if (kstrtou32(argv[1], 10, &port_id))
        goto wrong_params;

    if( port_id > 6)
        goto wrong_params;

    return lport_serdes_diag(port_id, 1);

wrong_params:
    pr_info("%s\n", serdes_proc_cmd_status_usage);
    return -1;
}

static int serdes_proc_cmd_loopback(int argc, char *argv[])
{
    char *serdes_proc_cmd_loopback_usage = "Usage: loopback <port_id (0-6)> <loopback mode (1-4)> <enable (0-1)>";
    uint32_t port_id, mode, enable;

    if (argc < 4)
        goto wrong_params;

    if (kstrtou32(argv[1], 10, &port_id))
        goto wrong_params;

    if( port_id > 6)
        goto wrong_params;

    if (kstrtou32(argv[2], 10, &mode))
        goto wrong_params;

    if( mode > 4 || mode == 0)
        goto wrong_params;

    if (kstrtou32(argv[3], 10, &enable))
        goto wrong_params;

    if( enable > 1)
        goto wrong_params;

    return lport_serdes_set_loopback(port_id, mode, enable);

wrong_params:
    pr_info("%s\n", serdes_proc_cmd_loopback_usage);
    pr_info("Loopback modes: \n 1 - PCS Local loopback\n 2 - PCS Remote loopback\n"
        " 3 - PMD Local Loopback\n 4 - PMD Remote Loopback\n");  
    return -1;
}

static int serdes_proc_cmd_tx_cfg(int argc, char *argv[])
{
    uint32_t port_id;
    uint16_t pre, main, post1, post2, hpf;
    char *serdes_proc_cmd_tx_cfg_usage = "Usage: txcfg <port (0-6)> [<pre (0-10)> <main (1-60)> <post1> <post2 (0-5)> <hpf (0-15)]\n\
    \n\
    1. if post2 != 0: post1 (0-18), (pre + post1) <= 22\n\
    2. if post2 == 0: post1 (0-23), (pre + post1) <= 27\n\
    3. (pre + post1 + post2 + 1)    <= main\n\
    4. (pre + main + post1 + post2) <= 60\n\
    4. 0 <= hpf <= 15\n";

    if (argc < 2)
        goto wrong_params;
    
    if (kstrtou32(argv[1], 10, &port_id))
        goto wrong_params;

    if (port_id > 6)
        goto wrong_params;
    
    if (argc == 2)
    {
        lport_serdes_get_tx_cfg(port_id, &pre, &main, &post1, &post2, &hpf);
        pr_info("pre   %d\n", pre);
        pr_info("main  %d\n", main);
        pr_info("post1 %d\n", post1);
        pr_info("post2 %d\n", post2);
        pr_info("hpf   %d\n", hpf);
        return 0;
    }
    else if (argc < 7)
        goto wrong_params;

    if (kstrtou16(argv[2], 10, &pre))
        goto wrong_params;

    if (kstrtou16(argv[3], 10, &main))
        goto wrong_params;

    if (kstrtou16(argv[4], 10, &post1))
        goto wrong_params;

    if (kstrtou16(argv[5], 10, &post2))
        goto wrong_params;

    if (kstrtou16(argv[6], 10, &hpf))
        goto wrong_params;

    return lport_serdes_set_tx_cfg(port_id, pre, main, post1, post2, hpf);

wrong_params:
    pr_info("%s\n", serdes_proc_cmd_tx_cfg_usage);
    return -1;
}

static struct proc_cmd_ops command_entries[] = {
    { .name = "init", .do_command	= lport_proc_cmd_init},
    {. name = "rxtx_enable", .do_command = lport_proc_cmd_rxtx_enable},
    {. name = "reset", .do_command = lport_proc_cmd_reset},
    { .name = "rate", .do_command	= lport_proc_cmd_rate},
    { .name = "status", .do_command	= lport_proc_cmd_status},
    { .name = "stats", .do_command	= lport_proc_cmd_stats},
    { .name = "phy_reg", .do_command	= lport_proc_cmd_phy_reg},
    { .name = "serdes_reg", .do_command	= lport_proc_cmd_serdes_reg},
    { .name = "serdes_tx_dis", .do_command	= lport_proc_cmd_serdes_tx_dis},
};

static struct proc_cmd_table lport_command_table = {
    .module_name = "LPORT",
    .size = ARRAY_SIZE(command_entries),
    .ops = command_entries
};

static struct proc_cmd_ops serdes_command_entries[] = {
    { .name = "reg",        .do_command = serdes_proc_cmd_reg},
    {. name = "prbs_gen",   .do_command = serdes_proc_cmd_prbs_generation},
    {. name = "prbs_mon",   .do_command = serdes_proc_cmd_prbs_monitor},
    {. name = "prbs_stat",  .do_command = serdes_proc_cmd_prbs_stat},
    { .name = "stats",      .do_command = serdes_proc_cmd_stats},
    { .name = "status",     .do_command = serdes_proc_cmd_status},
    { .name = "loopback",   .do_command = serdes_proc_cmd_loopback},
    { .name = "txcfg",      .do_command = serdes_proc_cmd_tx_cfg},
};

static struct proc_cmd_table serdes_command_table = {
    .module_name = "SERDES",
    .size = ARRAY_SIZE(serdes_command_entries),
    .ops = serdes_command_entries
};

static void lport_proc_exit(void)
{
    if (serdes_cmd_proc_file) 
    {
        remove_proc_entry(SERDES_PROC_FILE, proc_dir);
        serdes_cmd_proc_file = NULL;
    }	

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

    serdes_cmd_proc_file = proc_create_cmd(SERDES_PROC_FILE, proc_dir,
                                                &serdes_command_table);
    if (!serdes_cmd_proc_file) 
    {
        pr_err("Failed to create serdes %s\n", SERDES_PROC_FILE);
        goto error;
    }

    return status;

error:
    if (proc_dir)
        lport_proc_exit();

    status = -EIO;
    return status;
}
postcore_initcall(lport_proc_init);
