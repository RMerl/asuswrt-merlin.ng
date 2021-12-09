/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
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

#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>
#include <rdpa_api.h>
#ifdef CONFIG_BLOG
#include <linux/blog_rule.h>
#endif
#include "boardparms.h"
#include "board.h"
#include "clk_rst.h"
#include "wan_drv.h"
#ifdef CONFIG_BCM_TIME_SYNC_MODULE
#include "time_sync.h"
#endif
//TODO:Remove the FTTD runner code when ready
#if defined(G9991)
#include "access_macros.h"
#endif

/* init system params */
#define BP_NO_EXT_SW 30
#define RDPA_SWITCHING_MODE_PSP_KEY "SwitchingMode"
#define RDPA_IPCLASS_METHOD_PSP_KEY "IpClassMethod"
#define RDPA_US_DDR_QUEUE_ENABLE "UsDDRQueueEn"
#define RDPA_VLAN_STATS_ENABLE "VlanStatsEn"
#define RDPA_DPU_SPLIT_SCHED_MODE "DpuSplitSMode"
#define RDPA_IPTV_TABLE_SIZE "IptvTableSize"
#define RDPA_QM_US_QUEUE_SIZE "QmUsQueueSize"
#define RDPA_QM_DS_QUEUE_SIZE "QmDsQueueSize"
#define RDPA_QM_SERVICE_QUEUE_SIZE "QmServiceSize"

#define FC_TCP_ACK_PRIO_ENABLE "FcTcpAckPrioEn"
#define FW_CLANG_DISABLE "FwClangDis"


typedef enum
{
    mw_wan_type_none,
    mw_wan_type_gbe,
    mw_wan_type_epon_ae,
    mw_wan_type_epon,
    mw_wan_type_gpon,
    mw_wan_type_xgpon1,
    mw_wan_type_ngpon2,
    mw_wan_type_xgs,
    mw_wan_type_auto
} mw_wan_type;

static int emac_map = 0;
static int ext_sw_pid = BP_NO_EXT_SW;
#if !defined(CONFIG_BCM963138) && !defined(CONFIG_BCM963148) && !defined(CONFIG_BCM94908) && !defined(CONFIG_BCM963158)
/* scratchpad defaults */
static char *wan_default_type = "GPON";
static char *wan_oe_default_emac = "EMAC0";
static char *gbe_wan_defautl_emac = "NONE";

#define base(x) ((x >= '0' && x <= '9') ? '0' : \
    (x >= 'a' && x <= 'f') ? 'a' - 10 : \
    (x >= 'A' && x <= 'F') ? 'A' - 10 : \
    '\255')

#define TOHEX(x) (x - base(x))
#endif

extern int drv_qm_get_max_dynamic_queue_number(void);

static int rdpa_get_init_system_bp_params(rdpa_emac *gbe_wan_emac)
{
    int rc = 0;
    int i;
    const ETHERNET_MAC_INFO* EnetInfos;
    EnetInfos = BpGetEthernetMacInfoArrayPtr();
    if (EnetInfos == NULL)
        return rc;

    emac_map = EnetInfos[0].sw.port_map & 0xFF;
    for (i = 0; i < BP_MAX_SWITCH_PORTS; ++i)
    {
        if ((1<<i) & emac_map)
        {
            if ( IsPortConnectedToExternalSwitch(EnetInfos[0].sw.phy_id[i]) )
            {
                ext_sw_pid = i;
            }
#if defined(CONFIG_BCM_ETHWAN)
            /* BP_ENET_NO_PHY is used on 63138/63148/62118/63158 Platforms for Runner switch
             * Runner switch will have Ethernet Ports either as WAN or connected to switch */
            else if ((EnetInfos[0].ucPhyType == BP_ENET_NO_PHY)) 
            {
                *gbe_wan_emac = i;
            }
#endif /* CONFIG_BCM_ETHWAN */
        }
    }

    return rc;
}

#if !defined(CONFIG_BCM963138) && !defined(CONFIG_BCM963148) && !defined(CONFIG_BCM94908)
typedef enum
{
    scratchpad_func_get_or_init = 0,
    scratchpad_func_init = 1,
    scratchpad_func_get = 2,
} scratchpad_func_t;

static char *func_str[] =
{
    "get_or_init",
    "init",
    "get",
};

static int scratchpad_get_or_init(char *sp_key, char *buf, int max_len,
    char *default_val, scratchpad_func_t func)
{
    int count = 0;

    if (func == scratchpad_func_get_or_init || func == scratchpad_func_get)
    {
        count = kerSysScratchPadGet(sp_key, buf, max_len - 1);
        if (count >= 0)
            buf[count] = '\0';

        if (func == scratchpad_func_get)
            goto Exit;
    }

    /* init_only or read from scratch pad failed */
    if (count <= 0)
    {
        count = kerSysScratchPadSet(sp_key, default_val, strlen(default_val));
        if (count)
        {
            printk("Could not set PSP %s to %s, rc=%d", sp_key, default_val,
                    count);
            return count;
        }

        if (buf)
        {
            int cpy_len = MIN(max_len, strlen(default_val));
            strncpy(buf, default_val, cpy_len );
        }
    }

Exit:
    if (count > 0)
    {
        printk("RDPA_MW: scratchpad %s: %s - %s \n", func_str[func], sp_key, buf ? : default_val);
    }

    return count;
}

#if !defined(CONFIG_BCM963158) /* All PON Platforms */
static int wan_scratchpad_get(mw_wan_type *wan_type, rdpa_emac *wan_emac)
{
    int rc;
    char buf[PSP_BUFLEN_16];

    *wan_type = mw_wan_type_none;
    if (wan_emac)
        *wan_emac = rdpa_emac_none;

    rc = scratchpad_get_or_init(RDPA_WAN_TYPE_PSP_KEY, buf, sizeof(buf), wan_default_type, scratchpad_func_get_or_init);
    if (rc < 0)
        return rc;

    if (!strcasecmp(buf ,RDPA_WAN_TYPE_VALUE_GBE))
    {
        *wan_type = mw_wan_type_gbe;
        rc = scratchpad_get_or_init(RDPA_WAN_OEMAC_PSP_KEY, buf, sizeof(buf), wan_oe_default_emac, scratchpad_func_get_or_init);
        if (rc < 0)
            return rc;

        if (!strncasecmp(buf, "EMAC",4) && (strlen(buf) == strlen("EMACX")))
        {
            if (wan_emac)
                *wan_emac = (rdpa_emac)(TOHEX(buf[4]));
        }
        else if(!strcasecmp(buf, RDPA_WAN_OEMAC_VALUE_EPONMAC))
        {
          *wan_type = mw_wan_type_epon_ae;
        }
        else if(!strcasecmp(buf, RDPA_WAN_TYPE_VALUE_NONE))
        {
            return 0;
        }
        else
        {
            printk("%s %s Wrong EMAC string in ScrachPad - ###(%s)###\n", __FILE__, __FUNCTION__, buf);
            return -1;
        }
    }
    /* saved for backward compatibility */
    else if (!strcasecmp(buf ,RDPA_WAN_TYPE_VALUE_AE))
    {
        *wan_type = mw_wan_type_gbe;
        rc = scratchpad_get_or_init(RDPA_WAN_TYPE_PSP_KEY, NULL, 0, "GBE", scratchpad_func_init);
        if (rc < 0)
            return rc;
        rc = scratchpad_get_or_init(RDPA_WAN_OEMAC_PSP_KEY, NULL, 0, "EMAC5", scratchpad_func_init);
        if (rc < 0)
            return rc;

        if (wan_emac)
            *wan_emac = rdpa_emac5;
    }
    else if (!strcasecmp(buf ,RDPA_WAN_TYPE_VALUE_XGPON1))
    {
        *wan_type = mw_wan_type_xgpon1;
    }
    else if (!strcasecmp(buf ,RDPA_WAN_TYPE_VALUE_NGPON2))
    {
        *wan_type = mw_wan_type_ngpon2;
    }
    else if (!strcasecmp(buf ,RDPA_WAN_TYPE_VALUE_XGS))
    {
        *wan_type = mw_wan_type_xgs;
    }
    else if (!strcasecmp(buf ,RDPA_WAN_TYPE_VALUE_EPON))
    {
        *wan_type = mw_wan_type_epon;
    }
    else if (!strcasecmp(buf ,RDPA_WAN_TYPE_VALUE_AUTO))
        return 0; /* returns wan_type = rdpa_wan_none */
    else
    {
        *wan_type = mw_wan_type_gpon;
        if ( strcasecmp(buf ,RDPA_WAN_TYPE_VALUE_GPON))
        {
            printk("%s:%s:%d: Unknown wan type set in scratchpad %s: %s.  Configuring GPON mode\n", __FILE__, __FUNCTION__, __LINE__, RDPA_WAN_TYPE_PSP_KEY, buf);
            rc = scratchpad_get_or_init(RDPA_WAN_TYPE_PSP_KEY, NULL, 0, wan_default_type, scratchpad_func_init);
            if (rc < 0)
                return rc;
        }
    }

    rc = scratchpad_get_or_init(RDPA_GBE_WAN_EMAC_PSP_KEY, buf, sizeof(buf), gbe_wan_defautl_emac, scratchpad_func_get_or_init);
    if (rc < 0)
        return rc;

    if (!strncasecmp(buf ,"EMAC",4) && (strlen(buf) == strlen("EMACX")))
    {
        if (wan_emac)
            *wan_emac = (rdpa_emac)(TOHEX(buf[4]));
    }

    return 0;
}
#endif

#if defined(CONFIG_BCM_PON_RDP)
int switching_mode_get(rdpa_vlan_switching *switching_mode, mw_wan_type wan_type)
{
#ifndef BRCM_CMS_BUILD
    char buf[PSP_BUFLEN_16];
    int rc;
#endif

    *switching_mode = rdpa_switching_none;

#if defined(CONFIG_EPON_SFU)
    if (wan_type == mw_wan_type_epon)
        *switching_mode = rdpa_mac_based_switching;
#endif

#ifndef BRCM_CMS_BUILD
    rc = scratchpad_get_or_init(RDPA_SWITCHING_MODE_PSP_KEY, buf, sizeof(buf), NULL, scratchpad_func_get);
    if (rc < 0)
        return rc;

    if (!strcmp(buf ,"MAC"))
    {
        *switching_mode = rdpa_mac_based_switching;
    }
    else if (!strcmp(buf ,"VLAN"))
    {
        *switching_mode = rdpa_vlan_aware_switching;
    }
#endif

    return 0;
}
#endif

int ipclass_method_get(rdpa_ip_class_method *ipclass_method)
{
    int rc;
    char buf[PSP_BUFLEN_16];

#if defined(CONFIG_GPON_SFU) || defined(CONFIG_EPON_SFU)
    *ipclass_method = rdpa_method_none;
#else
    *ipclass_method = rdpa_method_fc;
#endif

    rc = scratchpad_get_or_init(RDPA_IPCLASS_METHOD_PSP_KEY, buf, sizeof(buf), NULL, scratchpad_func_get);
    if (rc < 0)
        return rc;

    if (!strcasecmp(buf ,"FC"))
    {
        *ipclass_method = rdpa_method_fc;
    }
    else if (!strcasecmp(buf ,"MIXED"))
    {
        *ipclass_method = rdpa_method_mixed;
    }
    else if (!strcasecmp(buf ,"NONE"))
    {
        *ipclass_method = rdpa_method_none;
    }

    return 0;
}

#ifdef XRDP
int fc_tcp_ack_prio_ena_get(int *tcp_ack_prio_ena)
{
    int rc;
    char buf[PSP_BUFLEN_16];

    *tcp_ack_prio_ena = 0; /* Default */

    rc = scratchpad_get_or_init(FC_TCP_ACK_PRIO_ENABLE, buf, sizeof(buf), NULL, scratchpad_func_get);
    if (rc < 0)
        return rc;

    if (!strcmp(buf ,"1"))
    {
        *tcp_ack_prio_ena = 1;
    }

    return 0;
}

int fw_clang_dis_get(int *fw_clang_dis)
{
#if defined(CONFIG_BCM96878) || defined(CONFIG_BCM96858) || defined(CONFIG_BCM963146)
    int rc;
    char buf[PSP_BUFLEN_16];
#endif

#ifdef CONFIG_BCM96878
    *fw_clang_dis = 0; /* By Default 96878 chip runs C based FW */
#else
    *fw_clang_dis = 1; /* By Default Gen3 and Gen4 chips, and 963146, run Assembler based FW 
						  Among all 4G platforms only 96858 can be switched to C version.
						  Other platforms don't have C code compiled in.
                       */
#endif 

    /* For 96858, 96878 and 963146 we allow to change the default */
#if defined(CONFIG_BCM96878) || defined(CONFIG_BCM96858) || defined(CONFIG_BCM963146)
    rc = scratchpad_get_or_init(FW_CLANG_DISABLE, buf, sizeof(buf), NULL, scratchpad_func_get);
    if (rc < 0)
        return rc;

    if (!strcmp(buf ,"1"))
    {
        *fw_clang_dis = 1;
    }
    else if (!strcmp(buf ,"0")) 
    {
        *fw_clang_dis = 0;
    }

#endif 

    return 0;
}
#endif

int qm_queues_size_get(rdpa_qm_cfg_t *qm_cfg_table)
{
    int rc;
    uint32_t val;
    char buf[PSP_BUFLEN_16];
#ifdef XRDP
    uint16_t max_dynamic_queue_number = (uint16_t)drv_qm_get_max_dynamic_queue_number();
    uint16_t requested_queue_number;
#endif

    rc = scratchpad_get_or_init(RDPA_QM_US_QUEUE_SIZE, buf, sizeof(buf), NULL, scratchpad_func_get);
    if (rc <= 0)
        return -1;
    sscanf(buf, "%u", &val);
    qm_cfg_table->number_of_us_queues = (uint16_t)val;

    rc = scratchpad_get_or_init(RDPA_QM_DS_QUEUE_SIZE, buf, sizeof(buf), NULL, scratchpad_func_get);
    if (rc <= 0)
        return -1;
    sscanf(buf, "%u", &val);
    qm_cfg_table->number_of_ds_queues = (uint16_t)val;

    rc = scratchpad_get_or_init(RDPA_QM_SERVICE_QUEUE_SIZE, buf, sizeof(buf), NULL, scratchpad_func_get);
    if (rc <= 0)
        return -1;
    sscanf(buf, "%u", &val);
    qm_cfg_table->number_of_service_queues = (uint16_t)val;

#ifdef XRDP
    requested_queue_number = qm_cfg_table->number_of_ds_queues + qm_cfg_table->number_of_us_queues + qm_cfg_table->number_of_service_queues;
    if (requested_queue_number > max_dynamic_queue_number)
    {
        printk("%s:%d Illigal queue numbers us %d, ds %d, service %d, max queues allowed %d\n", __FUNCTION__, __LINE__,
            qm_cfg_table->number_of_ds_queues, qm_cfg_table->number_of_us_queues, qm_cfg_table->number_of_service_queues, max_dynamic_queue_number);
        return -1;
    }
#endif

    return 0;    
}

int iptv_table_size_get(rdpa_iptv_table_size *iptv_table_size)
{
    int rc;
    char buf[PSP_BUFLEN_16];

    *iptv_table_size = rdpa_table_256_entries;

    rc = scratchpad_get_or_init(RDPA_IPTV_TABLE_SIZE, buf, sizeof(buf), NULL, scratchpad_func_get);
    if (rc < 0)
        return rc;

    if (!strcasecmp(buf ,"256"))
    {
        *iptv_table_size = rdpa_table_256_entries;
    }
    else if (!strcasecmp(buf ,"1024"))
    {
        *iptv_table_size = rdpa_table_1024_entries;
    }

    return 0;
}

int vlan_stats_enable_get(bdmf_boolean *vlan_stats_enable)
{
    int rc;
    char buf[6];

    *vlan_stats_enable = BDMF_FALSE;
    rc = scratchpad_get_or_init(RDPA_VLAN_STATS_ENABLE, buf, sizeof(buf), NULL, scratchpad_func_get);
    if (rc < 0)
        return rc;

    if (!strcmp(buf ,"YES"))
        *vlan_stats_enable = BDMF_TRUE;

    return 0;
}

int dpu_split_scheduling_mode_get(bdmf_boolean *dpu_split_scheduling_mode)
{
    int rc;
    char buf[6];

    *dpu_split_scheduling_mode = BDMF_FALSE;
    rc = scratchpad_get_or_init(RDPA_DPU_SPLIT_SCHED_MODE, buf, sizeof(buf), NULL, scratchpad_func_get);
    if (rc < 0)
        return rc;

    if (!strcmp(buf ,"YES"))
        *dpu_split_scheduling_mode = BDMF_TRUE;

    return 0;
}

#if !defined(CONFIG_BCM963158) /* All PON Platforms */

#ifndef BRCM_CMS_BUILD
static int us_ddr_queue_enable_get(bdmf_boolean *us_ddr_queue_enable)
{
    int rc;
    char buf[6];

    *us_ddr_queue_enable = BDMF_FALSE;
    rc = scratchpad_get_or_init(RDPA_US_DDR_QUEUE_ENABLE, buf, sizeof(buf), NULL, scratchpad_func_get);
    if (rc < 0)
        return rc;

    if (!strcmp(buf ,"YES"))
        *us_ddr_queue_enable = BDMF_TRUE;

    return rc;
}
#endif

int rdpa_init_system(void)
{
    BDMF_MATTR(rdpa_system_attrs, rdpa_system_drv());
    bdmf_object_handle rdpa_system_obj = NULL;
    bdmf_object_handle rdpa_filter_obj = NULL;
    bdmf_object_handle rdpa_tc_to_queue_obj = NULL;
    bdmf_object_handle rdpa_pbit_to_queue_obj = NULL;
    BDMF_MATTR(rdpa_filter_attrs, rdpa_filter_drv());
    BDMF_MATTR(rdpa_tc_to_queue_attrs, rdpa_tc_to_queue_drv());
    BDMF_MATTR(rdpa_pbit_to_queue_attrs, rdpa_pbit_to_queue_drv());
#if defined(G9991) && !defined(XRDP)
    uint32_t fttdp_addr, fttdp_val;
#endif
    int rc,i;
    rdpa_system_init_cfg_t sys_init_cfg = {};
#ifdef XRDP
    int rc_dqm;
	int rc_cntr_cfg;
    int tcp_ack_prio_ena = 0;
    int fw_clang_dis = 0;
    rdpa_qm_cfg_t qm_cfg = {};
	rdpa_counter_cfg_t counter_cfg = {};
    bdmf_object_handle rdpa_cpu_port_obj = NULL;
    BDMF_MATTR(rdpa_port_attrs, rdpa_port_drv());
#if !(defined(CHIP_63158) || defined(CHIP_6856) || defined(CHIP_6878))
    rdpa_packet_buffer_cfg_t packet_buffer_cfg = {};
#endif
#endif

    mw_wan_type wan_type;
    rdpa_system_cfg_t sys_cfg = {};
    bdmf_object_handle cpu =  NULL;

    /* Default values */
    sys_init_cfg.enabled_emac = 0;
    sys_init_cfg.gbe_wan_emac = rdpa_emac_none;
    sys_init_cfg.ip_class_method = rdpa_method_fc;
    sys_init_cfg.runner_ext_sw_cfg.enabled = 0;
    sys_init_cfg.switching_mode = rdpa_switching_none;
    sys_init_cfg.us_ddr_queue_enable = 0;

    rc = rdpa_get_init_system_bp_params(&sys_init_cfg.gbe_wan_emac);
    if (rc)
        goto exit;

    wan_scratchpad_get(&wan_type, &sys_init_cfg.gbe_wan_emac);

    sys_cfg.mtu_size = RDPA_MTU;

#ifndef XRDP
    sys_cfg.inner_tpid = 0x8100;
    sys_cfg.outer_tpid = 0x88a8;
#endif
    sys_cfg.car_mode = 1;

#if defined(CONFIG_BCM_RDP)
    /* GBE EMAC5 cannot be configured in system init, must be configed later via port object! */
    if (sys_init_cfg.gbe_wan_emac == rdpa_emac5)
        sys_init_cfg.gbe_wan_emac = rdpa_emac_none;
#endif

    sys_init_cfg.enabled_emac = emac_map;

    sys_init_cfg.runner_ext_sw_cfg.emac_id = (ext_sw_pid == BP_NO_EXT_SW) ? rdpa_emac_none:(rdpa_emac)ext_sw_pid;
    sys_init_cfg.runner_ext_sw_cfg.enabled = (ext_sw_pid == BP_NO_EXT_SW) ? 0 : 1;
#if defined(G9991)
    sys_init_cfg.runner_ext_sw_cfg.enabled = 1;
    sys_init_cfg.runner_ext_sw_cfg.type = rdpa_brcm_fttdp;
#else
    sys_init_cfg.runner_ext_sw_cfg.type = rdpa_brcm_hdr_opcode_0;
#endif

#if defined(CONFIG_BCM_PON_RDP)
    rc = switching_mode_get(&sys_init_cfg.switching_mode, wan_type);
    if (rc)
        goto exit;
#endif

    rc = ipclass_method_get(&sys_init_cfg.ip_class_method);
    if (rc)
        goto exit;

#ifdef XRDP
    rc = fc_tcp_ack_prio_ena_get(&tcp_ack_prio_ena);
    if (rc)
        goto exit;
    blog_support_set_tcp_ack_mflows(tcp_ack_prio_ena);
    
    rc = fw_clang_dis_get(&fw_clang_dis);
    if (rc)
        goto exit;
    sys_init_cfg.fw_clang_dis = fw_clang_dis;
#endif

    rc = dpu_split_scheduling_mode_get(&sys_init_cfg.dpu_split_scheduling_mode);
    if (rc)
        goto exit;
#ifdef XRDP
    rc = rc ? : iptv_table_size_get(&sys_init_cfg.iptv_table_size);
    if (rc)
        goto exit;
    rc_cntr_cfg = vlan_stats_enable_get(&counter_cfg.vlan_stats_enable);
    rc_dqm = qm_queues_size_get(&qm_cfg);
#endif

#ifndef BRCM_CMS_BUILD
    rc = us_ddr_queue_enable_get(&sys_init_cfg.us_ddr_queue_enable);
    if (rc)
        goto exit;
#endif

    rdpa_system_cfg_set(rdpa_system_attrs, &sys_cfg);
    rdpa_system_init_cfg_set(rdpa_system_attrs, &sys_init_cfg);

#ifdef XRDP

#if !(defined(CHIP_63158) || defined(CHIP_6856) || defined(CHIP_6878))
    packet_buffer_cfg.us_prio_rsv_thrs.min_buf_rsv_threshold = 1;
    packet_buffer_cfg.ds_prio_rsv_thrs.min_buf_rsv_threshold = 1;
    rdpa_system_packet_buffer_cfg_set(rdpa_system_attrs, &packet_buffer_cfg);
#endif

    if (rc_dqm == 0)
        rdpa_system_qm_cfg_set(rdpa_system_attrs, &qm_cfg);
    if (rc_cntr_cfg == 0)
        rdpa_system_counter_cfg_set(rdpa_system_attrs, &counter_cfg);
	
#endif

    rc = bdmf_new_and_set(rdpa_system_drv(), NULL, rdpa_system_attrs, &rdpa_system_obj);
    if (rc)
    {
        printk("%s %s Failed to create rdpa system object rc(%d)\n", __FILE__, __FUNCTION__, rc);
        goto exit;
    }

#ifdef XRDP
    rdpa_port_index_set(rdpa_port_attrs, rdpa_if_cpu);
    rc = bdmf_new_and_set(rdpa_port_drv(), NULL, rdpa_port_attrs, &rdpa_cpu_port_obj);
    if (rc < 0)
    {
        BDMF_TRACE_ERR("Failed to create cpu port object\n");
        goto exit;
    }
#endif

    rc = rdpa_cpu_get(rdpa_cpu_host, &cpu);

    /*CPU Interrupts are no longer automatically connected as part of CPU object
     *creation. They need to be explicitly using below rdpa call.*/
    rc = rc ? rc : rdpa_cpu_int_connect_set(cpu, 1 /*true*/);
    if (rc < 0)
    {
        BDMF_TRACE_ERR("Failed to connect cpu interrupts\n");
        goto exit;
    }

#ifdef XRDP
    rc = rdpa_port_cpu_obj_set(rdpa_cpu_port_obj, cpu);
    if (rc)
    {
        BDMF_TRACE_ERR("Failed to connect cpu port object and cpu host object\n");
        goto exit;
    }
#endif

    rc = bdmf_new_and_set(rdpa_filter_drv(), NULL, rdpa_filter_attrs, &rdpa_filter_obj);
    if (rc)
        printk("%s %s Failed to create rdpa filter object rc(%d)\n", __FILE__, __FUNCTION__, rc);

    /* Configure tc to q table - id 0 direction DS */
    rdpa_tc_to_queue_table_set(rdpa_tc_to_queue_attrs, 0);
    rdpa_tc_to_queue_dir_set(rdpa_tc_to_queue_attrs, rdpa_dir_ds); 
    rc = bdmf_new_and_set(rdpa_tc_to_queue_drv(), NULL, rdpa_tc_to_queue_attrs, &rdpa_tc_to_queue_obj);
    if (rc)
    {        
        printk("%s %s Failed to create rdpa tc_to_queue object rc(%d)\n", __FILE__, __FUNCTION__, rc);
        goto exit;
    }

    /* Configure pbit to q table - id 0 direction DS */
    rdpa_pbit_to_queue_table_set(rdpa_pbit_to_queue_attrs, 0);
    rc = bdmf_new_and_set(rdpa_pbit_to_queue_drv(), NULL, rdpa_pbit_to_queue_attrs, &rdpa_pbit_to_queue_obj);
    if (rc)
    {        
        printk("%s %s Failed to create rdpa pbit_to_queue object rc(%d)\n", __FILE__, __FUNCTION__, rc);
        goto exit;
    }

    for (i = 0; i < 8; ++i)
    {
        rdpa_tc_to_queue_tc_map_set(rdpa_tc_to_queue_obj, i, 0);
        rdpa_pbit_to_queue_pbit_map_set(rdpa_pbit_to_queue_obj, i, 0);
    }

exit:
    if (rc && rdpa_system_obj)
        bdmf_destroy(rdpa_system_obj);

//TODO:Remove the FTTD runner code when ready
#if defined(G9991) && !defined(XRDP)
    fttdp_addr = 0xb30d1818;
    fttdp_val = 0x19070019;
    WRITE_32(fttdp_addr, fttdp_val);
    fttdp_addr = 0xb30e1018;
    fttdp_val = 0x00002c2c;
    WRITE_32(fttdp_addr, fttdp_val);
    fttdp_addr = 0xb30e101c;
    fttdp_val = 0x00001013;
    WRITE_32(fttdp_addr, fttdp_val);
    fttdp_addr = 0xb30e1020;
    fttdp_val = 0x00001919;
    WRITE_32(fttdp_addr, fttdp_val);
#endif

    return rc;
}

#else /* defined(CONFIG_BCM963158) */

int rdpa_init_system(void)
{
    BDMF_MATTR(rdpa_system_attrs, rdpa_system_drv());
    bdmf_object_handle rdpa_system_obj = NULL;
    bdmf_object_handle rdpa_filter_obj = NULL;
    int rc;
    rdpa_system_init_cfg_t sys_init_cfg = {};
    bdmf_object_handle rdpa_cpu_port_obj = NULL;
    BDMF_MATTR(rdpa_filter_attrs, rdpa_filter_drv());
    BDMF_MATTR(rdpa_port_attrs, rdpa_port_drv());

    rdpa_system_cfg_t sys_cfg = {};
    rdpa_packet_buffer_cfg_t packet_buffer_cfg = {};
    bdmf_object_handle cpu =  NULL;

    /* Default values */
    sys_init_cfg.enabled_emac = 0;
    sys_init_cfg.gbe_wan_emac = rdpa_emac_none;
    sys_init_cfg.ip_class_method = rdpa_method_fc;
    sys_init_cfg.runner_ext_sw_cfg.enabled = 0;
    sys_init_cfg.switching_mode = rdpa_switching_none;
    sys_init_cfg.us_ddr_queue_enable = 0;

    rc = rdpa_get_init_system_bp_params(&sys_init_cfg.gbe_wan_emac);
    if (rc)
        goto exit;

    sys_cfg.mtu_size = RDPA_MTU;

    sys_cfg.car_mode = 1;

    sys_init_cfg.enabled_emac = emac_map;

    sys_init_cfg.runner_ext_sw_cfg.emac_id = (ext_sw_pid == BP_NO_EXT_SW) ? rdpa_emac_none:(rdpa_emac)ext_sw_pid;
    sys_init_cfg.runner_ext_sw_cfg.enabled = (ext_sw_pid == BP_NO_EXT_SW) ? 0 : 1;
    sys_init_cfg.runner_ext_sw_cfg.type = rdpa_brcm_hdr_opcode_0;

    packet_buffer_cfg.us_prio_rsv_thrs.high_prio_buf_threshold = 5;
    packet_buffer_cfg.us_prio_rsv_thrs.min_buf_rsv_threshold = 10;
    packet_buffer_cfg.wlan_prio_rsv_thrs.high_prio_buf_threshold = 2;
    packet_buffer_cfg.wlan_prio_rsv_thrs.excl_prio_buf_threshold = 3;

    rdpa_system_cfg_set(rdpa_system_attrs, &sys_cfg);
    rdpa_system_init_cfg_set(rdpa_system_attrs, &sys_init_cfg);
    rdpa_system_packet_buffer_cfg_set(rdpa_system_attrs, &packet_buffer_cfg);

    rc = bdmf_new_and_set(rdpa_system_drv(), NULL, rdpa_system_attrs, &rdpa_system_obj);
    if (rc)
    {
        printk("%s %s Failed to create rdpa system object rc(%d)\n", __FILE__, __FUNCTION__, rc);
        goto exit;
    }

    rdpa_port_index_set(rdpa_port_attrs, rdpa_if_cpu);
    rc = bdmf_new_and_set(rdpa_port_drv(), NULL, rdpa_port_attrs, &rdpa_cpu_port_obj);
    if (rc < 0)
    {
        BDMF_TRACE_ERR("Failed to create cpu port object\n");
        goto exit;
    }

    rc = rdpa_cpu_get(rdpa_cpu_host, &cpu);

    /*CPU Interrupts are no longer automatically connected as part of CPU object
     *creation. They need to be explicitly using below rdpa call.*/
    rc = rc ? rc : rdpa_cpu_int_connect_set(cpu, 1 /*true*/);
    if (rc < 0)
    {
        BDMF_TRACE_ERR("Failed to connect cpu interrupts\n");
        goto exit;
    }

    rc = rdpa_port_cpu_obj_set(rdpa_cpu_port_obj, cpu);
    if (rc)
    {
        BDMF_TRACE_ERR("Failed to connect cpu port object and cpu host object\n");
        goto exit;
    }

    rc = bdmf_new_and_set(rdpa_filter_drv(), NULL, rdpa_filter_attrs, &rdpa_filter_obj);
    if (rc)
        printk("%s %s Failed to create rdpa filter object rc(%d)\n", __FILE__, __FUNCTION__, rc);

exit:
    if (cpu)
        bdmf_put(cpu);
    if (rc && rdpa_system_obj)
        bdmf_destroy(rdpa_system_obj);

    return rc;
}
#endif /* defined(CONFIG_BCM963158)*/

#elif defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908)

int rdpa_init_system(void)
{
    BDMF_MATTR(rdpa_system_attrs, rdpa_system_drv());
    bdmf_object_handle rdpa_system_obj = NULL;
    BDMF_MATTR(rdpa_filter_attrs, rdpa_filter_drv());
    bdmf_object_handle rdpa_filter_obj = NULL;
    int rc;
    rdpa_system_init_cfg_t sys_init_cfg = {};
    bdmf_object_handle cpu =  NULL;

    /* Default values */
    sys_init_cfg.enabled_emac = 0;
    sys_init_cfg.gbe_wan_emac = rdpa_emac_none;
    sys_init_cfg.ip_class_method = rdpa_method_fc;
    sys_init_cfg.runner_ext_sw_cfg.enabled = 0;
    sys_init_cfg.switching_mode = rdpa_switching_none;
    sys_init_cfg.us_ddr_queue_enable = 0;

    rc = rdpa_get_init_system_bp_params(&sys_init_cfg.gbe_wan_emac);
    if (rc)
        goto exit;

    sys_init_cfg.enabled_emac = emac_map;
    sys_init_cfg.runner_ext_sw_cfg.emac_id = (ext_sw_pid == BP_NO_EXT_SW) ? rdpa_emac_none:(rdpa_emac)ext_sw_pid;
    sys_init_cfg.runner_ext_sw_cfg.enabled = (ext_sw_pid == BP_NO_EXT_SW) ? 0 : 1;
    sys_init_cfg.runner_ext_sw_cfg.type = rdpa_brcm_hdr_opcode_0;
    sys_init_cfg.switching_mode = rdpa_switching_none;
    sys_init_cfg.ip_class_method = rdpa_method_fc;

    rdpa_system_init_cfg_set(rdpa_system_attrs, &sys_init_cfg);

    rc = bdmf_new_and_set(rdpa_system_drv(), NULL, rdpa_system_attrs, &rdpa_system_obj);
    if (rc)
    {
        printk("%s %s Failed to create rdpa system object rc(%d)\n", __FILE__, __FUNCTION__, rc);
        goto exit;
    }

    rc = rdpa_cpu_get(rdpa_cpu_host, &cpu);

    /*CPU Interrupts are no longer automatically connected as part of CPU object
     *creation. They need to be explicitly using below rdpa call.*/
    rc = rc ? rc : rdpa_cpu_int_connect_set(cpu, 1 /*true*/);
    if (rc < 0)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to connect cpu interrupts\n");
        goto exit;
    }

    rc = bdmf_new_and_set(rdpa_filter_drv(), NULL, rdpa_filter_attrs, &rdpa_filter_obj);
    if (rc)
        printk("%s %s Failed to create rdpa filter object rc(%d)\n", __FILE__, __FUNCTION__, rc);

exit:
    if (rc && rdpa_system_obj)
        bdmf_destroy(rdpa_system_obj);
    return rc;
}

#endif

