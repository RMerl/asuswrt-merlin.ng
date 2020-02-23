/*
   Copyright (c) 2014 Broadcom
   All Rights Reserved

    <:label-BRCM:2014:DUAL/GPL:standard
    
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

/******************************************************************************/
/*                                                                            */
/* File Description:                                                          */
/*                                                                            */
/* This file contains the implementation of misc shell commands for Lilac     */
/*                                                                            */
/******************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/

#define BDMF_SESSION_DISABLE_FORMAT_CHECK

#include "bdmf_shell.h"
#include "access_macros.h"
#if (defined(CONFIG_BCM963158) || defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96858) || \
    defined(CONFIG_BCM96856) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96878)) && !defined(BDMF_SYSTEM_SIM)
#include "bcm_OS_Deps.h" /* We need the following 3 #includes for PSP_BUFLEN_16 / RDPA_WAN_TYPE_PSP_KEY definitions */
#include "boardparms.h"
#include "board.h"
#include "wan_drv.h"
#if defined(CONFIG_BCM96838)
#include "mdio_drv_impl1.h"
#endif
#if defined(CONFIG_BCM96848)
#include "mdio_drv_impl2.h"
#endif
#if defined(CONFIG_BCM96858)
#include "lport_mdio.h"
#endif
#endif
#if defined(XRDP) && !defined(BDMF_SYSTEM_SIM)
#include "rdp_drv_rnr.h"
#endif
#include <rdpa_api.h>
#include "bdmf_dev.h"
#if defined(XRDP) && defined(CONFIG_RNR_BRIDGE)
#include "rdpa_vlan_ex.h"
#include "rdd_ag_ds_tm.h"
#include "rdd_ag_us_tm.h"
#endif

#ifdef USE_BDMF_SHELL

#if defined(BDMF_SYSTEM_SIM)
#ifndef XRDP
#include "rdd.h"
#endif
#ifndef LEGACY_RDP
#include "rdd_simulator.h"
#ifdef XRDP
#include "XRDP_AG.h"
uint8_t *soc_base_address;
#include "rdd_stubs.h"
#include "rdp_drv_tcam.h"
#ifdef RDP_SIM
#include "rdp_cpu_sim.h"
#endif
#endif
#else
#include "rdd_common.h"
#include "rdd_legacy_conv.h"
#endif
#else
#ifdef XRDP
#include "rdd_runner_proj_defs.h"
#endif
#endif

typedef union
{
    uint32_t word;
    uint8_t word_byte[4];
    uint16_t word_half[2];
} word_t;

#ifdef XRDP
#define TEXT_OFFSET 48
#define LINE_LENGTH 66
#else
#define TEXT_OFFSET 32
#define LINE_LENGTH 50
#endif
#define TEXT_END 17

enum
{
    SRC_DFP,
    SRC_SHELL
};

static void p_chr(char *line, uint32_t index, char cc)
{
    if ((isgraph((int)cc) != 0) || (cc == ' '))
        line[index] = cc;
    else
        line[index] = '.';
}

static void prn(uint32_t source, bdmf_session_handle session, char *line)
{
#if defined(DFP)
    switch (source)
    {
    case SRC_DFP:
        /*pi_dfp_shell_printf(line);*/
        break;
    case SRC_SHELL:
        bdmf_session_print(session, "%s", line);
        break;
    }
#else
    bdmf_session_print(session, "%s", line);
#endif
}

static void read_mem(bdmf_session_handle session, void *adrs, uint32_t nwords, uint32_t width, uint32_t source)
{
    uint32_t value = 0;
    word_t pvalue[4];
    void *read_address = adrs;
    int32_t length = nwords;
    uint32_t linecounter = 0;
    uint32_t l = 0;
    uint16_t k16;
    uint32_t k32;
    char *line;
    char tline[10];

    line = bdmf_alloc(LINE_LENGTH);
    if (!line)
        return;
    memset((void *)line, ' ', LINE_LENGTH);
    line[LINE_LENGTH - 1] = '\0';
    for (; length > 0; read_address += width, length--)
    {
        switch (width)
        {
        case 1:
#ifdef RDP_SIM
            READ_8((uint8_t *)read_address, value);
#else
            value = *(uint8_t *)read_address;
#endif
            if (linecounter % 8 == 0)
            {
                if (linecounter != 0)
                {
                    line[strlen(line)] = ' ';
                    line[TEXT_OFFSET] = '*';
                    memset(&line[TEXT_OFFSET + 1], '.', TEXT_END);
                    for (l = 0; l < linecounter; l++)
                        p_chr(line, TEXT_OFFSET + 1 + l, (char)pvalue[l / 4].word_byte[l % 4]);
                    line[TEXT_OFFSET + TEXT_END] = '*';
                    line[TEXT_OFFSET + TEXT_END + 1] = '\0';
                    prn(source, session, line);
                }
                memset((void *)line, ' ', LINE_LENGTH);
                switch (source)
                {
                case SRC_DFP:
                    sprintf(line, "%p: %02x", read_address, (unsigned int)value);
                    break;
                case SRC_SHELL:
                    sprintf(line, "\n\r%p: %02x", read_address, (unsigned int)value);
                    break;
                }
                linecounter = 1;
            }
            else
            {
                sprintf(tline, " %02x", (unsigned int)value);
                strcat(line, tline);
                linecounter++;
            }
            pvalue[(linecounter - 1) / 4].word_byte[(linecounter - 1) % 4] = (uint8_t)value;
            break;

        case 2:
#ifdef RDP_SIM
            READ_16((uint16_t *)read_address, value);
#else
            value = swap2bytes(*(uint16_t *)read_address);
#endif
            if (linecounter % 4 == 0)
            {
                if (linecounter != 0)
                {
                    line[strlen(line)] = ' ';
                    line[TEXT_OFFSET] = '*';
                    memset(&line[TEXT_OFFSET + 1], '.', TEXT_END);
                    for (l = 0; l < linecounter; l++)
                    {
                        k16 = pvalue[l / 2].word_half[l % 2];
                        p_chr(line, TEXT_OFFSET + 1 + l*2, (char)((k16 & 0xFF00) >> 8));
                        p_chr(line, TEXT_OFFSET + 2 + l*2, (char)(k16 & 0xFF));
                    }
                    line[TEXT_OFFSET + TEXT_END] = '*';
                    line[TEXT_OFFSET + TEXT_END + 1] = '\0';
                    prn(source, session, line);
                }
                memset((void *)line, ' ', LINE_LENGTH);
                switch (source)
                {
                case SRC_DFP:
                    sprintf(line, "%p: %04x", read_address, (unsigned int)value);
                    break;
                case SRC_SHELL:
                    sprintf(line, "\n\r%p: %04x", read_address, (unsigned int)value);
                    break;
                }
                linecounter = 1;
            }
            else
            {
                sprintf(tline, " %04x", (unsigned int)value);
                strcat(line, tline);
                linecounter++;
            }
            pvalue[(linecounter - 1) / 2].word_half[(linecounter - 1) % 2] = (uint16_t)value;
            break;

        case 4:
#ifdef RDP_SIM
            READ_32((uint32_t *)read_address, value);
#else
            value = swap4bytes(*(uint32_t *)read_address);
#endif
            if (linecounter % 2 == 0)
            {
                if (linecounter != 0)
                {
                    line[strlen(line)] = ' ';
                    line[TEXT_OFFSET] = '*';
                    memset(&line[TEXT_OFFSET + 1], '.', TEXT_END);
                    for (l = 0; l < linecounter; l++)
                    {
                        k32 = pvalue[l].word;
                        p_chr(line, TEXT_OFFSET + 1 + l*4, (char)((k32 & 0xFF000000) >> 24));
                        p_chr(line, TEXT_OFFSET + 2 + l*4, (char)((k32 & 0xFF0000) >> 16));
                        p_chr(line, TEXT_OFFSET + 3 + l*4, (char)((k32 & 0xFF00) >> 8));
                        p_chr(line, TEXT_OFFSET + 4 + l*4, (char)(k32 & 0xFF));
                    }
                    line[TEXT_OFFSET + TEXT_END] = '*';
                    line[TEXT_OFFSET + TEXT_END + 1] = '\0';
                    prn(source, session, line);
                }
                memset((void *)line, ' ', LINE_LENGTH);
                switch (source)
                {
                case SRC_DFP:
                    sprintf(line, "%p: %08x", read_address, (unsigned int)value);
                    break;
                case SRC_SHELL:
                    sprintf(line, "\n\r%p: %08x", read_address, (unsigned int)value);
                    break;
                }
                linecounter = 1;
            }
            else
            {
                sprintf(tline, " %08x", (unsigned int)value);
                strcat(line, tline);
                linecounter++;
            }
            pvalue[linecounter - 1].word = (uint32_t)value;
            break;

        default:
            break;
        }
    }
    switch (width)
    {
    case 1:
        if (linecounter > 0)
        {
            line[strlen(line)] = ' ';
            line[TEXT_OFFSET] = '*';
            memset(&line[TEXT_OFFSET + 1], '.', TEXT_END);
            for (l = 0; l < linecounter; l++)
            {
                p_chr(line, TEXT_OFFSET + 1 + l, (char) pvalue[l / 4].word_byte[l % 4]);
            }
            line[TEXT_OFFSET + TEXT_END] = '*';
            line[TEXT_OFFSET + TEXT_END + 1] = '\0';
            prn(source, session, line);
        }
        break;

    case 2:
        if (linecounter > 0)
        {
            line[strlen(line)] = ' ';
            line[TEXT_OFFSET] = '*';
            memset(&line[TEXT_OFFSET + 1], '.', TEXT_END);
            for (l = 0; l < linecounter; l++)
            {
                k16 = pvalue[l / 2].word_half[l % 2];
                p_chr(line, TEXT_OFFSET + 1 + l*2, (char)((k16 & 0xFF00) >> 8));
                p_chr(line, TEXT_OFFSET + 2 + l*2, (char)(k16 & 0xFF));
            }
            line[TEXT_OFFSET + TEXT_END] = '*';
            line[TEXT_OFFSET + TEXT_END + 1] = '\0';
            prn(source, session, line);
        }
        break;

    case 4:
        if (linecounter > 0)
        {
            line[strlen(line)] = ' ';
            line[TEXT_OFFSET] = '*';
            memset(&line[TEXT_OFFSET + 1], '.', TEXT_END);
            for (l = 0; l < linecounter; l++)
            {
                k32 = pvalue[l].word;
                p_chr(line, TEXT_OFFSET + 1 + l*4, (char)((k32 & 0xFF000000) >> 24));
                p_chr(line, TEXT_OFFSET + 2 + l*4, (char)((k32 & 0xFF0000) >> 16));
                p_chr(line, TEXT_OFFSET + 3 + l*4, (char)((k32 & 0xFF00) >> 8));
                p_chr(line, TEXT_OFFSET + 4 + l*4, (char)(k32 & 0xFF));
            }
            line[TEXT_OFFSET + TEXT_END] = '*';
            line[TEXT_OFFSET + TEXT_END + 1] = '\0';
            prn(source, session, line);
        }
        break;

    default:
        break;
    }
    bdmf_free(line);
}

static void write_mem(void *write_address, uint32_t value, uint32_t width)
{
   switch (width)
   {
#ifdef RDP_SIM
   case 0:
   case 1:
       WRITE_8((uint8_t *)write_address, value);
       break;
   default:
   case 4:
   case 5:
       WRITE_32((uint16_t *)write_address, value);
       break;
   case 2:
   case 3:
       WRITE_16((uint32_t *)write_address, value);
       break;
#else
   case 0:
   case 1:
       *(uint8_t *)write_address = (uint8_t)value;
       break;
   default:
   case 4:
   case 5:
       *(uint32_t *)write_address = value;
       break;
   case 2:
   case 3:
       *(uint16_t *)write_address = (uint16_t)value;
       break;
#endif
   }
}

typedef enum {
    MISC_MEMORY_BYTE = 0,
    MISC_MEMORY_WORD,
    MISC_MEMORY_HEX,
} misc_mem_width_t;

#define WIDTH_FROM_ENUM(x) (x == MISC_MEMORY_BYTE ? 1 : (x == MISC_MEMORY_WORD ? 4 : (x == MISC_MEMORY_HEX ? 2 : 0)))

static int misc_read_memory_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t parameter_index = 0;
#if defined(__LP64__) || defined(_LP64) 
    uint8_t *address = (uint8_t *)(uintptr_t)parm[parameter_index++].value.unumber64; /* input */
#else
    uint8_t *address = (uint8_t *)parm[parameter_index++].value.unumber; /* input */
#endif
    misc_mem_width_t width = parm[parameter_index++].value.unumber; /* input */
    uint32_t number_of_words = parm[parameter_index++].value.unumber; /* input */

    read_mem(session, address, number_of_words, WIDTH_FROM_ENUM(width), SRC_SHELL);
    bdmf_session_print(session, "\n\r");
    return 0;
}

static int misc_write_memory_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t parameter_index = 0;
#if defined(__LP64__) || defined(_LP64) 
    uint8_t *address = (uint8_t *)(uintptr_t)parm[parameter_index++].value.unumber64; /* input */
#else
    uint8_t *address = (uint8_t *)parm[parameter_index++].value.unumber; /* input */
#endif
    misc_mem_width_t width = parm[parameter_index++].value.unumber; /* input */
    uint32_t value = parm[parameter_index++].value.unumber; /* input */

    write_mem(address, value, WIDTH_FROM_ENUM(width));
    bdmf_session_print(session, "Writing to memory done\n\r");
    return 0;
}

void misc_mr_cmd(void *address, int num_of_words)
{
    read_mem(NULL, address, num_of_words, WIDTH_FROM_ENUM(MISC_MEMORY_WORD), SRC_SHELL);
}

void misc_mw_cmd(void *address, uint32_t value)
{
    write_mem(address, value, WIDTH_FROM_ENUM(MISC_MEMORY_WORD));
}

#if defined(BDMF_SYSTEM_SIM)
static int misc_runner_memory_dump_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    if (_segment_file_init("sim_mem", "w+b", (uint8_t *)DEVICE_ADDRESS(0), SIM_MEM_SIZE))
        return -1;

    bdmf_session_print(session, "Runner mem dump done\n\r");
    return 0;
}

#ifdef CONFIG_DHD_RUNNER
void rdd_sim_dhd_tx_flow_ring_mgmt_init(void);
#endif

#define MAX_NUM_OF_WAN_CHANNELS 40 /* Temporary, to cover all platforms */
static int misc_runner_segs_dump_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    int rc = -1;
#ifndef XRDP  
    uint16_t *tx_pointers_tbl;
    int tx_pointers_tbl_size;
#else
    char file_name[100] =  {};
    uint32_t rnr_idx, mem_high_byte_num, mem_cntxt_byte_num;
#endif
     
#ifndef XRDP            
    tx_pointers_tbl_size = sizeof(uint16_t) * MAX_NUM_OF_WAN_CHANNELS *
        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLER_ADDR_NUMBER *
        RDD_US_RATE_CONTROLLER_DESCRIPTOR_TX_QUEUE_ADDR_NUMBER;
    tx_pointers_tbl = (uint16_t *)bdmf_alloc(tx_pointers_tbl_size);
    if (!tx_pointers_tbl)
        goto exit;

#ifndef LEGACY_RDP
#ifdef CONFIG_DHD_RUNNER
    rdd_sim_dhd_tx_flow_ring_mgmt_init();
    rdd_save_wifi_dongle_config();
#endif
#endif

    if (rdd_sim_save_ddr_tables())
        goto exit;
    if (rdd_sim_save_hw_cfg())
        goto exit;
    rdd_sim_save_tx_pointers(tx_pointers_tbl);

    if (_segment_file_init("tx_pointers_table", "w+b", (uint8_t *)tx_pointers_tbl, tx_pointers_tbl_size))
        goto exit;
    if (segment_file_init("data_common", "w+b", RUNNER_COMMON_0_OFFSET, sizeof(RUNNER_COMMON)))
        goto exit;
    if (segment_file_init("data_private_A", "w+b", RUNNER_PRIVATE_0_OFFSET, sizeof(RUNNER_PRIVATE)))
        goto exit;
    if (segment_file_init("context_segment_A", "w+b", RUNNER_CNTXT_MAIN_0_OFFSET, sizeof(RUNNER_CNTXT_MAIN)))
        goto exit;
    if (segment_file_init("context_segment_C", "w+b", RUNNER_CNTXT_PICO_0_OFFSET, sizeof(RUNNER_CNTXT_PICO)))
        goto exit;
    if (segment_file_init("data_common", "a+b", RUNNER_COMMON_1_OFFSET, sizeof(RUNNER_COMMON)))
        goto exit;
    if (segment_file_init("data_private_B", "w+b", RUNNER_PRIVATE_1_OFFSET, sizeof(RUNNER_PRIVATE)))
        goto exit;
    if (segment_file_init("context_segment_B", "w+b", RUNNER_CNTXT_MAIN_1_OFFSET, sizeof(RUNNER_CNTXT_MAIN)))
        goto exit;
    if (segment_file_init("context_segment_D", "w+b", RUNNER_CNTXT_PICO_1_OFFSET, sizeof(RUNNER_CNTXT_PICO)))
        goto exit;
    if (segment_file_init("sim_mem", "w+b", 0, SIM_MEM_SIZE))
        goto exit;
#else
    mem_high_byte_num = (RU_REG_RAM_CNT(RNR_MEM, HIGH) + 1) * sizeof(uint32_t);
    mem_cntxt_byte_num = (RU_REG_RAM_CNT(RNR_CNTXT, MEM_ENTRY) + 1) * sizeof(uint32_t);

    for (rnr_idx = 0; rnr_idx < RNR_MEM_BLOCK.addr_count; rnr_idx++)
    {
        sprintf(file_name, "data_mem_%d", rnr_idx);
        if (segment_file_init(file_name, "w+b", (RU_BLK(RNR_MEM).addr[rnr_idx] + RU_REG_OFFSET(RNR_MEM, HIGH)),
            mem_high_byte_num * 2))
            goto exit;
        sprintf(file_name, "context_segment_%d", rnr_idx);
        if (segment_file_init(file_name, "w+b", (RU_BLK(RNR_CNTXT).addr[rnr_idx] + RU_REG_OFFSET(RNR_CNTXT, MEM_ENTRY)),
            mem_cntxt_byte_num))
            goto exit;
    }
    if (rdd_sim_save_ddr_tables())
        goto exit;
    if (_segment_file_init("sim_mem", "w+b", (uint8_t *)soc_base_address, SIM_MEM_SIZE))
        goto exit;
    if (drv_tcam_mem_dump(session, "tcam"))
        goto exit;
#endif
    rc = 0;

exit:
#ifndef XRDP
    if (tx_pointers_tbl)
        bdmf_free(tx_pointers_tbl);
#endif
    bdmf_session_print(session, "Dump segments %s\n\r", rc ? "failed" : "done");
    return rc;
}
#endif
static bdmfmon_handle_t misc_dir;

#if (defined(CONFIG_BCM963158) || defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96858) || \
    defined(CONFIG_BCM96856) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96878)) && !defined(BDMF_SYSTEM_SIM)
static struct bdmfmon_enum_val prbs_wan_enum_table[] = {
    { "none", SERDES_WAN_TYPE_NONE },
    { "gpon", SERDES_WAN_TYPE_GPON },
    { "epon1g", SERDES_WAN_TYPE_EPON_1G },
    { "epon2g", SERDES_WAN_TYPE_EPON_2G },
    { NULL, 0 },
};

static struct bdmfmon_enum_val prbs_mode_enum_table[] = {
    { "7", 0 },
    { "15", 1 },
    { "23", 2 },
    { "31", 3 },
    { NULL, 0 },
};

static int misc_prbs_status(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_boolean valid;
    uint32_t errors;

    wan_prbs_status(&valid, &errors);
    bdmf_session_print(session, "prbs valid %d\n\r", valid);

    if (!valid)
        bdmf_session_print(session, "prbs errors %d\n\r", errors);

    return 0;
}

static int misc_prbs_enable(bdmf_session_handle session, serdes_wan_type_t wan_type, int host_tracking_enable, int mode)
{
    static serdes_wan_type_t wan_type_enabled = SERDES_WAN_TYPE_NONE;
    bdmf_boolean dummy;
    
    if (wan_type == wan_type_enabled)
    {
        bdmf_session_print(session, "prbs already %sabled\n\r", wan_type_enabled == SERDES_WAN_TYPE_NONE ?
            "dis" : "en");

        return -1;
    }

    if (wan_type != SERDES_WAN_TYPE_NONE)
        wan_type_enabled = wan_type;

    wan_prbs_gen(wan_type != SERDES_WAN_TYPE_NONE, host_tracking_enable, mode, wan_type_enabled, &dummy);
    wan_type_enabled = wan_type;
    return 0;
}

static int misc_prbs(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t parameter_index = 0;
    serdes_wan_type_t wan_type = parm[parameter_index++].value.unumber; /* input */
    uint32_t mode = parm[parameter_index++].value.unumber; /* input */
    int host_tracking_enable = parm[parameter_index++].value.unumber; /* input */

    return misc_prbs_enable(session, wan_type, host_tracking_enable, mode);
}

static int misc_init_serdes_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    int                    count;
    int                    rc = 0;
    char wan_type_buf[PSP_BUFLEN_16] = {};
    char emac_id_buf[PSP_BUFLEN_16]  = {};
    char epon_speed_buf[PSP_BUFLEN_16] = {};

    count = kerSysScratchPadGet((char *)RDPA_WAN_TYPE_PSP_KEY, (char *)wan_type_buf, (int)sizeof(wan_type_buf));
    if (count == 0)
    {
        bdmf_session_print(session, "Cannot read WAN type from the scratchpad\n\r");
        return -1;
    }
    if (strcasecmp(wan_type_buf, "GPON") == 0)
    {
        wan_serdes_config(SERDES_WAN_TYPE_GPON);
    }
    else if (strcasecmp(wan_type_buf, "EPON") == 0)
    {
        count = kerSysScratchPadGet((char *)RDPA_EPON_SPEED_PSP_KEY, (char *)epon_speed_buf,
            (int)sizeof(epon_speed_buf));
        if (count == 0)
        {
            bdmf_session_print(session, "Cannot read EPON speed from the scratchpad\n\r");
            return -1;
        }

        if (!strcasecmp(epon_speed_buf, "Turbo"))
        {
            wan_serdes_config(SERDES_WAN_TYPE_EPON_2G);
        }
        else if (!strcasecmp(epon_speed_buf, "Normal"))
        {
            wan_serdes_config(SERDES_WAN_TYPE_EPON_1G);
        }
    }
    else if (strcasecmp(wan_type_buf, "GBE") == 0)
    {
        count = kerSysScratchPadGet((char *)RDPA_WAN_OEMAC_PSP_KEY, (char *)emac_id_buf, (int)sizeof(emac_id_buf));
        if (count == 0)
        {
            bdmf_session_print(session, "Cannot read EMAC ID from the scratchpad\n\r");
            return -1;
        }
        
        if (strcasecmp(emac_id_buf, "EMAC5") == 0)
        {
           wan_serdes_config(SERDES_WAN_TYPE_AE);
        }
        else
        {
          bdmf_session_print(session, "SerDes initialization is not required for %s\n\r", emac_id_buf);
          goto exit;
        }
   }
   else
   {
       bdmf_session_print(session, "SerDes initialization is not required for WAN type %s\n\r", wan_type_buf);
   }

exit:

   return rc;
}
#endif

#if (defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96858)) && !defined(BDMF_SYSTEM_SIM)

#if defined(CONFIG_BCM96838)
#define misc_mdio_type_t mdio_type_t
static struct bdmfmon_enum_val mdio_enum_table[] = {
    {"ext", MDIO_EXT},
    {"egphy", MDIO_EGPHY},
    {"sata", MDIO_SATA},
    {"ae", MDIO_AE},
    {NULL, 0},
};
#endif
#if defined(CONFIG_BCM96848)
#define misc_mdio_type_t mdio_type_t
static struct bdmfmon_enum_val mdio_enum_table[] = {
    {"ext", MDIO_EXT},
    {"int", MDIO_INT},
    {NULL, 0},
};
#endif
#if defined(CONFIG_BCM96858)
#define misc_mdio_type_t int
static struct bdmfmon_enum_val mdio_enum_table[] = {
    {"int", 0},
    {NULL, 0},
};
#define mdio_read_c22_register(a, b, c, d) lport_mdio22_rd(b, c, d)
#define mdio_write_c22_register(a, b, c, d) lport_mdio22_wr(b, c, d)
#define mdio_read_c45_register(a, b, c, d, e) lport_mdio45_rd(b, c, d, e)
#define mdio_write_c45_register(a, b, c, d, e) lport_mdio45_wr(b, c, d, e)
#endif

static int misc_mdio_22_read_register_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    uint8_t parameter_index = 0;
    misc_mdio_type_t type = parm[parameter_index++].value.unumber; /* input */
    uint32_t phy = parm[parameter_index++].value.unumber; /* input */
    uint32_t addr = parm[parameter_index++].value.unumber; /* input */
    uint16_t value;

    if (mdio_read_c22_register(type, phy, addr, &value))
    {
        bdmf_session_print(session, "MDIO cl22 read command failed. type=0x%x\n\r", type);
        return -1;
    }

    bdmf_session_print(session, "\n\r%x\n\r", value);

    return 0;
}

static int misc_mdio_22_write_register_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    uint8_t parameter_index = 0;
    misc_mdio_type_t type = parm[parameter_index++].value.unumber; /* input */
    uint32_t phy = parm[parameter_index++].value.unumber; /* input */
    uint32_t addr = parm[parameter_index++].value.unumber; /* input */
    uint32_t value = parm[parameter_index++].value.unumber; /* input */

    if (mdio_write_c22_register(type, phy, addr, value))
    {
        bdmf_session_print(session, "MDIO cl22 write command failed. type=0x%x\n\r", type);
        return -1;
    }

    bdmf_session_print(session, "\n\r");

    return 0;
}

static int misc_mdio_45_read_register_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    uint8_t parameter_index = 0;
    misc_mdio_type_t type = parm[parameter_index++].value.unumber; /* input */
    uint32_t phy = parm[parameter_index++].value.unumber; /* input */
    uint32_t dev = parm[parameter_index++].value.unumber; /* input */
    uint32_t addr = parm[parameter_index++].value.unumber; /* input */
    uint16_t value;

    if (mdio_read_c45_register(type, phy, dev, addr, &value))
    {
        bdmf_session_print(session, "MDIO cl45 read command failed. type=0x%x\n\r", type);
        return -1;
    }

    bdmf_session_print(session, "\n\r%x\n\r", value);

    return 0;
}

static int misc_mdio_45_write_register_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    uint8_t parameter_index = 0;
    misc_mdio_type_t type = parm[parameter_index++].value.unumber; /* input */
    uint32_t phy = parm[parameter_index++].value.unumber; /* input */
    uint32_t dev = parm[parameter_index++].value.unumber; /* input */
    uint32_t addr = parm[parameter_index++].value.unumber; /* input */
    uint32_t value = parm[parameter_index++].value.unumber; /* input */

    if (mdio_write_c45_register(type, phy, dev, addr, value))
    {
        bdmf_session_print(session, "MDIO cl45 write command failed. type=0x%x\n\r", type);
        return -1;
    }

    bdmf_session_print(session, "\n\r");

    return 0;
}

#if defined(CONFIG_BCM96848)
static int misc_onu2g_read_register_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    uint8_t parameter_index = 0;
    uint32_t addr = parm[parameter_index++].value.unumber; /* input */
    uint16_t value;

    onu2g_read(addr, &value);
    bdmf_session_print(session, "\n\r%x\n\r", value);

    return 0;
}

static int misc_onu2g_write_register_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    uint8_t parameter_index = 0;
    uint32_t addr = parm[parameter_index++].value.unumber; /* input */
    uint32_t value = parm[parameter_index++].value.unumber; /* input */

    onu2g_write(addr, value);
    bdmf_session_print(session, "\n\r");

    return 0;
}
#endif

#endif

#if defined(XRDP)
/* write SRAM
 *   BDMFMON_MAKE_PARM_RANGE("core", "Core index", BDMFMON_PARM_NUMBER, 0, 0, 15),
 *   BDMFMON_MAKE_PARM("address", "SRAM address", BDMFMON_PARM_HEX, 0),
 *   BDMFMON_MAKE_PARM("data", "Data to write", BDMFMON_PARM_STRING, 0));
 */
static int misc_sram_write_handler(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    int core = (int)parm[0].value.number;
    uint32_t addr = (uint32_t)parm[1].value.number;
    const char *data_str = (const char *)parm[2].value.string;

    uint8_t *start_addr = (uint8_t *)(DEVICE_ADDRESS(rdp_runner_core_addr[core]) + addr);
    uint32_t len = strlen(data_str);
    int n;

    if ((len % 2) == 1)
    {
        bdmf_session_print(session, "Hex string must have even number of bytes. Got %u\n", len);
        return BDMF_ERR_PARM;
    }
    n =  bdmf_strhex(data_str, start_addr, len/2);
    if (n != len/2)
    {
        bdmf_session_print(session, "Hex string must have even number of bytes. Got %u\n", len);
        return BDMF_ERR_PARM;
    }

    return BDMF_ERR_OK;
}

/* write SRAM
 *   BDMFMON_MAKE_PARM_RANGE("core", "Core index", BDMFMON_PARM_NUMBER, 0, 0, 15),
 *   BDMFMON_MAKE_PARM("address", "SRAM address", BDMFMON_PARM_HEX, 0),
 *   BDMFMON_MAKE_PARM("data", "Data to write", BDMFMON_PARM_STRING, 0));
 */
static int misc_image_sram_write_handler(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    int core = (int)parm[0].value.number;
    uint32_t addr = (uint32_t)parm[1].value.number;
    const char *data_str = (const char *)parm[2].value.string;
    rdp_runner_image_e image_id = rdp_core_to_image_map[core];
    uint8_t *start_addr;
    uint32_t len = strlen(data_str);
    int i, n;

    if ((len % 2) == 1)
    {
        bdmf_session_print(session, "Hex string must have even number of bytes. Got %u\n", len);
        return BDMF_ERR_PARM;
    }
    bdmf_session_print(session, "image_id = %d cores: ", image_id);
    for (i = 0; i < RNR_LAST; i++)
    {
        if (image_id != rdp_core_to_image_map[i])
            continue;
        start_addr = (uint8_t *)(DEVICE_ADDRESS(rdp_runner_core_addr[i]) + addr);
        bdmf_session_print(session, "%d, ", i);
        n =  bdmf_strhex(data_str, start_addr, len/2);
        if (n != len/2)
        {
            bdmf_session_print(session, "Hex string must have even number of bytes. Got %u\n", len);
            return BDMF_ERR_PARM;
        }
    }
    bdmf_session_print(session, "\n");
    return BDMF_ERR_OK;
}

/* Read SRAM
 *   BDMFMON_MAKE_PARM_RANGE("core", "Core index", BDMFMON_PARM_NUMBER, 0, 0, 15),
 *   BDMFMON_MAKE_PARM("address", "SRAM address", BDMFMON_PARM_HEX, 0),
 *   BDMFMON_MAKE_PARM_RANGE("size", "Area size", BDMFMON_PARM_NUMBER, 0, 1, 512));
 */
static int misc_sram_read_handler(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    int core = (int)parm[0].value.number;
    uint32_t addr = (uint32_t)parm[1].value.number;
    uint32_t size = (uint32_t)parm[2].value.number;

    uint8_t *start_addr = (uint8_t *)(DEVICE_ADDRESS(rdp_runner_core_addr[core]) + addr);

    bdmf_session_hexdump(session, start_addr, 0, size);

    return BDMF_ERR_OK;
}

#if !defined(BDMF_SYSTEM_SIM)
/* WAkeup runner task
 *   BDMFMON_MAKE_PARM_RANGE("core", "Core index", BDMFMON_PARM_NUMBER, 0, 0, 15),
 *   BDMFMON_MAKE_PARM_RANGE("task", "Task index", BDMFMON_PARM_NUMBER, 0, 0, 31));
 */
static int misc_task_wakeup_handler(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    int core = (int)parm[0].value.number;
    int task = (int)parm[1].value.number;
    ag_drv_rnr_regs_cfg_cpu_wakeup_set(core, task);
    return BDMF_ERR_OK;
}
#endif

#endif

/* Setting system global EPON Mode from shell commands*/
#if defined(BDMF_SYSTEM_SIM) && !defined(WL4908) && !defined(BCM63158)
extern rdpa_epon_mode g_epon_mode;
static struct bdmfmon_enum_val epon_mode_enum_table[] = {
    {"none", rdpa_epon_none},
    {"ctc", rdpa_epon_ctc},
    {"cuc", rdpa_epon_cuc},
    {"dpoe", rdpa_epon_dpoe},
    {"bcm", rdpa_epon_bcm},
    {"ctc_dyn", rdpa_epon_ctc_dyn},
    {"cuc_dyn", rdpa_epon_cuc_dyn},
    {NULL, 0},
};
static int misc_epon_mode_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    g_epon_mode = parm[0].value.unumber;
    return BDMF_ERR_OK;
}
#endif

#if defined(XRDP) && defined(CONFIG_RNR_BRIDGE)

/* Unfortunately there is no convenient way to translate from bdmf_attr_enum_table_t to bdmfmon_enum_val */
static struct bdmfmon_enum_val __rdpa_if_enum_table[] =
{
    /** WAN ports */
    {"wan0", rdpa_if_wan0},
    {"wan1", rdpa_if_wan1},
    {"wan2", rdpa_if_wan2},

    /** LAN ports */
    {"lan0", rdpa_if_lan0},
    {"lan1", rdpa_if_lan1},
    {"lan2", rdpa_if_lan2},
    {"lan3", rdpa_if_lan3},
    {"lan4", rdpa_if_lan4},
    {"lan5", rdpa_if_lan5},
    {"lan6", rdpa_if_lan6},
    {"lan7", rdpa_if_lan7},
#if !defined(BCM_DSL_RDP) && !defined(BCM63158)
    {"lan8", rdpa_if_lan8},
    {"lan9", rdpa_if_lan9},
    {"lan10", rdpa_if_lan10},
    {"lan11", rdpa_if_lan11},
    {"lan12", rdpa_if_lan12},
    {"lan13", rdpa_if_lan13},
    {"lan14", rdpa_if_lan14},
    {"lan15", rdpa_if_lan15},
    {"lan16", rdpa_if_lan16},
    {"lan17", rdpa_if_lan17},
    {"lan18", rdpa_if_lan18},
    {"lan19", rdpa_if_lan19},
    {"lan20", rdpa_if_lan20},
    {"lan21", rdpa_if_lan21},
#endif
#ifdef G9991
    {"lan22", rdpa_if_lan22},
    {"lan23", rdpa_if_lan23},
    {"lan24", rdpa_if_lan24},
    {"lan25", rdpa_if_lan25},
    {"lan26", rdpa_if_lan26},
    {"lan27", rdpa_if_lan27},
    {"lan28", rdpa_if_lan28},
    {"lan29", rdpa_if_lan29},
#endif

    /** Special ports */
    {"lag0", rdpa_if_lag0},
    {"lag1", rdpa_if_lag1},
    {"lag2", rdpa_if_lag2},
    {"lag3", rdpa_if_lag3},
    {"lag4", rdpa_if_lag4},

    /** Switch aggregate port */
    {"switch", rdpa_if_switch},

#ifndef XRDP
    /** WiFi physical ports */
    {"wlan0", rdpa_if_wlan0},
    {"wlan1", rdpa_if_wlan1},

    /** CPU (local termination) */
    {"cpu",  rdpa_if_cpu},

    /** WiFi logical ports (SSIDs) */
    {"ssid0", rdpa_if_ssid0},
    {"ssid1", rdpa_if_ssid1},
    {"ssid2", rdpa_if_ssid2},
    {"ssid3", rdpa_if_ssid3},
    {"ssid4", rdpa_if_ssid4},
    {"ssid5", rdpa_if_ssid5},
    {"ssid6", rdpa_if_ssid6},
    {"ssid7", rdpa_if_ssid7},
    {"ssid8", rdpa_if_ssid8},
    {"ssid9", rdpa_if_ssid9},
    {"ssid10", rdpa_if_ssid10},
    {"ssid11", rdpa_if_ssid11},
    {"ssid12", rdpa_if_ssid12},
    {"ssid13", rdpa_if_ssid13},
    {"ssid14", rdpa_if_ssid14},
    {"ssid15", rdpa_if_ssid15},
#else
    /** CPU (local termination) */
    {"cpu0", rdpa_if_cpu0},
    {"cpu1", rdpa_if_cpu1},
    {"cpu2", rdpa_if_cpu2},
    {"cpu3", rdpa_if_cpu3},
    {"wlan0", rdpa_if_wlan0},
    {"wlan1", rdpa_if_wlan1},
    {"wlan2", rdpa_if_wlan2},
#endif
    {"bond0", rdpa_if_bond0},
    {"bond1", rdpa_if_bond1},
    {"bond2", rdpa_if_bond2},

    {"any", rdpa_if_any},
    {"none", rdpa_if_none},
    {NULL, 0}
};

/* List of actions after invoke of rdpa_forward_action2rdd_action */
static struct bdmfmon_enum_val __rdd_action_enum_table[] =
{
    {"forward", ACTION_FORWARD},
    {"host", ACTION_TRAP},
    {"drop", ACTION_DROP},
    {"flood", ACTION_MULTICAST}, /* use only for bridge DAL */
    {NULL, 0}
};

#endif

#if defined(XRDP) && defined(CONFIG_RNR_BRIDGE)
/* Read VLAN Hash entry by port and VID */
static int misc_vlan_hash_entry_handler_read(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS bridge_and_vlan_lkp_result;
    rdpa_if port = parm[0].value.unumber;
    int16_t vid = (int16_t)parm[1].value.number;
    int rc;

    rc = rdpa_vlan_hash_entry_read(port, vid, &bridge_and_vlan_lkp_result);
    if (rc)
    {
        if (rc == BDMF_ERR_NOENT)
            bdmf_session_print(session, "Entry not found\n");
        else
            bdmf_session_print(session, "Entry lookup in hash failed, rc = %d\n", rc);
        return BDMF_ERR_OK;
    }

    bdmf_session_print(session, "== Result:\n\t"
        "bridge_id = %d, port_isolation_map = 0x%x,\n\t"
        "ingress_filter_profile = 0x%x, protocol_filters_dis = 0x%x,\n\t"
        "sa_lookup_en = %d, sa_lookup_miss_action = %d,\n\t"
        "da_lookup_en = %d, da_lookup_miss_action = %d,\n\t"
        "aggregation_en = %d, counter_id = %d, counter_id_valid = %d\n",
        bridge_and_vlan_lkp_result.bridge_id, bridge_and_vlan_lkp_result.port_isolation_map,
        bridge_and_vlan_lkp_result.ingress_filter_profile, bridge_and_vlan_lkp_result.protocol_filters_dis,
        bridge_and_vlan_lkp_result.sa_lookup_en, bridge_and_vlan_lkp_result.sa_lookup_miss_action,
        bridge_and_vlan_lkp_result.da_lookup_en, bridge_and_vlan_lkp_result.da_lookup_miss_action,
        bridge_and_vlan_lkp_result.aggregation_en, 
        bridge_and_vlan_lkp_result.counter_id, bridge_and_vlan_lkp_result.counter_id_valid);

    return BDMF_ERR_OK;
}

static int misc_vlan_hash_entry_handler_write(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    RDD_BRIDGE_AND_VLAN_LKP_RESULT_DTS bridge_and_vlan_lkp_result;
    int parm_idx = 0;
    rdpa_if port = parm[parm_idx++].value.unumber;
    int16_t vid = (int16_t)parm[parm_idx++].value.number;
    uint32_t vport_mask = (uint32_t)parm[parm_idx++].value.number;
    int rc;

    rdpa_vlan_hash_entry_isolation_vector_set(vport_mask, &bridge_and_vlan_lkp_result);
    bridge_and_vlan_lkp_result.bridge_id = (int16_t)parm[parm_idx++].value.number;
    bridge_and_vlan_lkp_result.ingress_filter_profile = (uint8_t)parm[parm_idx++].value.unumber;
    bridge_and_vlan_lkp_result.protocol_filters_dis = (uint8_t)parm[parm_idx++].value.unumber;
    bridge_and_vlan_lkp_result.sa_lookup_en = (uint8_t)parm[parm_idx++].value.unumber;
    bridge_and_vlan_lkp_result.da_lookup_en = (uint8_t)parm[parm_idx++].value.unumber;
    bridge_and_vlan_lkp_result.sa_lookup_miss_action = parm[parm_idx++].value.unumber;
    bridge_and_vlan_lkp_result.da_lookup_miss_action = parm[parm_idx++].value.unumber;
    bridge_and_vlan_lkp_result.aggregation_en = (uint8_t)parm[parm_idx++].value.unumber;
    bridge_and_vlan_lkp_result.counter_id = (uint8_t)parm[parm_idx++].value.unumber;
    bridge_and_vlan_lkp_result.counter_id_valid = (uint8_t)parm[parm_idx++].value.unumber;


    bdmf_session_print(session, "== Writing new entry, Key (port = %d, vid = %d), Result:\n\t"
        "bridge_id = %d, port_isolation_map = 0x%x,\n\t"
        "ingress_filter_profile = 0x%x, protocol_filters_dis = 0x%x,\n\t"
        "sa_lookup_en = %d, sa_lookup_miss_action = %d,\n\t"
        "da_lookup_en = %d, da_lookup_miss_action = %d,\n\t"
        "aggregation_en = %d, counter_id = %d, counter_id_valid = %d\n",
        port, vid,
        bridge_and_vlan_lkp_result.bridge_id, bridge_and_vlan_lkp_result.port_isolation_map,
        bridge_and_vlan_lkp_result.ingress_filter_profile, bridge_and_vlan_lkp_result.protocol_filters_dis,
        bridge_and_vlan_lkp_result.sa_lookup_en, bridge_and_vlan_lkp_result.sa_lookup_miss_action,
        bridge_and_vlan_lkp_result.da_lookup_en, bridge_and_vlan_lkp_result.da_lookup_miss_action,
        bridge_and_vlan_lkp_result.aggregation_en, 
        bridge_and_vlan_lkp_result.counter_id, bridge_and_vlan_lkp_result.counter_id_valid);

    rc = rdpa_vlan_hash_entry_write(port, vid, &bridge_and_vlan_lkp_result);
    if (rc)
    {
        bdmf_session_print(session, "Failed to write new entry to hash, rc = %d\n", rc);
        return BDMF_ERR_OK;
    }

    bdmf_session_print(session, "Done\n");
    return BDMF_ERR_OK;
}

/* Delete VLAN Hash entry by port and VID */
static int misc_vlan_hash_entry_handler_del(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    rdpa_if port = parm[0].value.unumber;
    int16_t vid = (int16_t)parm[1].value.number;
    int rc;

    rc = rdpa_vlan_hash_entry_delete(port, vid);
    if (rc)
    {
        if (rc == BDMF_ERR_NOENT)
            bdmf_session_print(session, "Entry not found\n");
        else
            bdmf_session_print(session, "Entry delete in hash failed, rc = %d\n", rc);
    }
    else
        bdmf_session_print(session, "Done\n");

    return BDMF_ERR_OK;
}

/* disable flush task */
static int disable_flush_task_write(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    int rc;
    uint16_t disable = parm[0].value.unumber;

    rc = rdd_ag_us_tm_flush_aggregation_task_disable_set(disable);
    if (rc)
        return rc;

    rc = rdd_ag_ds_tm_flush_aggregation_task_disable_set(disable);

    return BDMF_ERR_OK;
}

#endif /* defined(XRDP) && defined(CONFIG_RNR_BRIDGE) */


void misc_shell_init(void)
{
    static struct bdmfmon_enum_val size_enum_table[] = {
        {"w",   MISC_MEMORY_WORD},
        {"h",   MISC_MEMORY_HEX},
        {"b",   MISC_MEMORY_BYTE},
        {NULL, 0},
    };

    misc_dir = NULL;

    misc_dir = bdmfmon_dir_find(NULL, "misc");
    if (!misc_dir)
    {
        misc_dir = bdmfmon_dir_add(NULL, "misc", "miscellaneous", BDMF_ACCESS_ADMIN, NULL);
        if (!misc_dir)
        {
            bdmf_session_print(NULL, "Can't create %s directory\n", "misc");
            return;
        }
    }

    BDMFMON_MAKE_CMD(misc_dir, "mr", "read memory", misc_read_memory_command,
#if defined(__LP64__) || defined(_LP64) 
        BDMFMON_MAKE_PARM("address", "address: Hex64", BDMFMON_PARM_HEX64, 0),
#else
        BDMFMON_MAKE_PARM("address", "address: Hex", BDMFMON_PARM_HEX, 0),
#endif
        BDMFMON_MAKE_PARM_ENUM("width", "width", size_enum_table, 0),
        BDMFMON_MAKE_PARM("length", "number of words", BDMFMON_PARM_NUMBER, 0));

    BDMFMON_MAKE_CMD(misc_dir, "mw", "write memory", misc_write_memory_command,
#if defined(__LP64__) || defined(_LP64) 
        BDMFMON_MAKE_PARM("address", "address: Hex64", BDMFMON_PARM_HEX64, 0),
#else
        BDMFMON_MAKE_PARM("address", "address: Hex", BDMFMON_PARM_HEX, 0),
#endif
        BDMFMON_MAKE_PARM_ENUM("width", "width", size_enum_table, 0),
        BDMFMON_MAKE_PARM("value", "value: Hex", BDMFMON_PARM_HEX, 0));
#if defined(BDMF_SYSTEM_SIM)
    BDMFMON_MAKE_CMD_NOPARM(misc_dir, "rmd", "runner memory dump", misc_runner_memory_dump_command);
    BDMFMON_MAKE_CMD_NOPARM(misc_dir, "rsmd", "runner segments memory dump", misc_runner_segs_dump_command);
#ifdef XRDP
    BDMFMON_MAKE_CMD(misc_dir, "connect", "set connect mode", cpu_runner_sim_connect,
        BDMFMON_MAKE_PARM("port", "session port", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
#endif
#if !defined(WL4908) && !defined(BCM63158)
    BDMFMON_MAKE_CMD(misc_dir, "sem", "set epon mode", misc_epon_mode_set,
        BDMFMON_MAKE_PARM_ENUM("mode", "epon mode", epon_mode_enum_table, 0));
#endif
#endif

#if (defined(CONFIG_BCM963158) || defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96858) || \
    defined(CONFIG_BCM96856) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96878)) && !defined(BDMF_SYSTEM_SIM)
    BDMFMON_MAKE_CMD_NOPARM(misc_dir, "is", "init serdes according to WAN interface type", misc_init_serdes_command);
    
    BDMFMON_MAKE_CMD(misc_dir, "prbs", "configure prbs", misc_prbs,
        BDMFMON_MAKE_PARM_ENUM("wan", "wan", prbs_wan_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("mode", "prbs mode", prbs_mode_enum_table, 0),
        BDMFMON_MAKE_PARM("tracking", "host tracking", BDMFMON_PARM_HEX, 0));
    BDMFMON_MAKE_CMD_NOPARM(misc_dir, "prbs_status", "get prbs status", misc_prbs_status);
#endif

#if (defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96858)) && !defined(BDMF_SYSTEM_SIM)    

    BDMFMON_MAKE_CMD(misc_dir, "md22r", "read mdio register", misc_mdio_22_read_register_command,
        BDMFMON_MAKE_PARM_ENUM("mdio", "mdio bus", mdio_enum_table, 0),
        BDMFMON_MAKE_PARM("phy", "phy address", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM("address", "register address", BDMFMON_PARM_HEX, 0));

    BDMFMON_MAKE_CMD(misc_dir, "md22w", "write mdio register", misc_mdio_22_write_register_command,
        BDMFMON_MAKE_PARM_ENUM("mdio", "mdio bus", mdio_enum_table, 0),
        BDMFMON_MAKE_PARM("phy", "phy address", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM("address", "register address", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_HEX, 0));

    BDMFMON_MAKE_CMD(misc_dir, "md45r", "read mdio register", misc_mdio_45_read_register_command,
        BDMFMON_MAKE_PARM_ENUM("mdio", "mdio bus", mdio_enum_table, 0),
        BDMFMON_MAKE_PARM("phy", "phy address", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM("device", "device address", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM("address", "register address", BDMFMON_PARM_HEX, 0));

    BDMFMON_MAKE_CMD(misc_dir, "md45w", "write mdio register", misc_mdio_45_write_register_command,
        BDMFMON_MAKE_PARM_ENUM("mdio", "mdio bus", mdio_enum_table, 0),
        BDMFMON_MAKE_PARM("phy", "phy address", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM("device", "device address", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM("address", "register address", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_HEX, 0));
#if defined(CONFIG_BCM96848)
    BDMFMON_MAKE_CMD(misc_dir, "sr", "read onu2g register", misc_onu2g_read_register_command,
        BDMFMON_MAKE_PARM("address", "register address", BDMFMON_PARM_HEX, 0));

    BDMFMON_MAKE_CMD(misc_dir, "sw", "write onu2g register", misc_onu2g_write_register_command,
        BDMFMON_MAKE_PARM("address", "register address", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_HEX, 0));
#endif
#endif
#if defined(XRDP)
    BDMFMON_MAKE_CMD(misc_dir, "sramw", "Write SRAM", misc_sram_write_handler,
        BDMFMON_MAKE_PARM_RANGE("core", "Core index", BDMFMON_PARM_NUMBER, 0, 0, (NUM_OF_RUNNER_CORES-1)),
        BDMFMON_MAKE_PARM("address", "SRAM address", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM("data", "Data to write", BDMFMON_PARM_STRING, 0));

    BDMFMON_MAKE_CMD(misc_dir, "sramr", "Read SRAM", misc_sram_read_handler,
        BDMFMON_MAKE_PARM_RANGE("core", "Core index", BDMFMON_PARM_NUMBER, 0, 0, (NUM_OF_RUNNER_CORES-1)),
        BDMFMON_MAKE_PARM("address", "SRAM address", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM_RANGE("size", "Area size", BDMFMON_PARM_NUMBER, 0, 1, 512));

    BDMFMON_MAKE_CMD(misc_dir, "isramw", "Image write SRAM", misc_image_sram_write_handler,
        BDMFMON_MAKE_PARM_RANGE("core", "Core index (image inferred from core)", BDMFMON_PARM_NUMBER, 0, 0,
        (NUM_OF_RUNNER_CORES-1)),
        BDMFMON_MAKE_PARM("address", "SRAM address", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM("data", "Data to write", BDMFMON_PARM_STRING, 0));
#if !defined(BDMF_SYSTEM_SIM)
    BDMFMON_MAKE_CMD(misc_dir, "wakeup", "Wakeup task", misc_task_wakeup_handler,
        BDMFMON_MAKE_PARM_RANGE("core", "Core index", BDMFMON_PARM_NUMBER, 0, 0, (NUM_OF_RUNNER_CORES-1)),
        BDMFMON_MAKE_PARM_RANGE("task", "Task index", BDMFMON_PARM_NUMBER, 0, 0, 31));
#endif
#endif
#if defined(XRDP) && defined(CONFIG_RNR_BRIDGE)
    BDMFMON_MAKE_CMD(misc_dir, "rvh", "Read VLAN Hash entry", misc_vlan_hash_entry_handler_read,
        BDMFMON_MAKE_PARM_ENUM("port", "port (rdpa_if)", __rdpa_if_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("vid", "VID", BDMFMON_PARM_NUMBER, 0, 0, 4095));

    BDMFMON_MAKE_CMD(misc_dir, "wvh", "Write VLAN Hash entry", misc_vlan_hash_entry_handler_write,
        BDMFMON_MAKE_PARM_ENUM("port", "port (rdpa_if)", __rdpa_if_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("vid", "VID", BDMFMON_PARM_NUMBER, 0, 0, 4095),
        BDMFMON_MAKE_PARM("vports_vector", "Vport vector", BDMFMON_PARM_HEX, 0), /* TODO: Add enum mask */
        BDMFMON_MAKE_PARM_RANGE("bridge_id", "BRIDGE ID", BDMFMON_PARM_NUMBER, 0, 0, RDPA_BRIDGE_MAX_BRIDGES),
        BDMFMON_MAKE_PARM("filter_profile", "Ingress filter profile (0-f, 0x3f for invalid)", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM("dis_proto", "Disabled protocols mask (bits: 0-ipv4, 1-ipv6, 2-pppoe, 3-non-ip)",
            BDMFMON_PARM_HEX, 0), /* TODO: Add enum mask */
        BDMFMON_MAKE_PARM_RANGE("sal", "SA Lookup (0-disabled, 1-enabled)", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("dal", "DA Lookup (0-disabled, 1-enabled)", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_ENUM("sal_action", "SA Lookup action", __rdd_action_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("dal_action", "DA Lookup action", __rdd_action_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("aggr", "Aggregation (0-disabled, 1-enabled)", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("vlan_counter_index", "Vlan counter Index 0-127", BDMFMON_PARM_NUMBER, 0, 0, 127),
        BDMFMON_MAKE_PARM_RANGE("counter_id_valid", "counter id valid (0-no, 1-yes)", BDMFMON_PARM_NUMBER, 0, 0, 1));

    BDMFMON_MAKE_CMD(misc_dir, "dvh", "Delete VLAN Hash entry", misc_vlan_hash_entry_handler_del,
        BDMFMON_MAKE_PARM_ENUM("port", "port (rdpa_if)", __rdpa_if_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("vid", "VID", BDMFMON_PARM_NUMBER, 0, 0, 4095));

    BDMFMON_MAKE_CMD(misc_dir, "disable_flush_task", "disable flush task", disable_flush_task_write,
        BDMFMON_MAKE_PARM_RANGE("disable", "disable flush task (0-no, 1-yes)", BDMFMON_PARM_NUMBER, 0, 0, 1));

#endif
}

void misc_shell_uninit(void)
{
    if (misc_dir)
    {
        bdmfmon_token_destroy(misc_dir);
        misc_dir = NULL;
    }
}

#endif
