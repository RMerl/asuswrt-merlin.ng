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

#include "board.h"
#include "clk_rst.h"
#include "wan_drv.h"
#ifdef CONFIG_BCM_TIME_SYNC_MODULE
#include "time_sync.h"
#endif
#include "rdpa_drv.h"

/* init system params */
#define BP_NO_EXT_SW 30
#define RDPA_SWITCHING_MODE_PSP_KEY "SwitchingMode"
#define RDPA_IPCLASS_METHOD_PSP_KEY "IpClassMethod" /* Backward compatibility. */
#define RDPA_US_DDR_QUEUE_ENABLE "UsDDRQueueEn"
#define RDPA_VLAN_STATS_ENABLE "VlanStatsEn"
#define RDPA_IPTV_TABLE_SIZE "IptvTableSize"
#define RDPA_QM_US_QUEUE_SIZE "QmUsQueueSize"
#define RDPA_QM_DS_QUEUE_SIZE "QmDsQueueSize"
#define RDPA_QM_SERVICE_QUEUE_SIZE "QmServiceSize"

#define FC_TCP_ACK_PRIO_ENABLE "FcTcpAckPrioEn"


#define NVRAM_RNR_MAX_KEYS                    1
#define NVRAM_RNR_KEY_STR_LEN                 32
#define NVRAM_RNR_RES_STR_LEN                 32

#define NVRAM_RNR_KEY_DHD_OFFL_OVERRIDE       0

extern char *nvram_k_get(char *name);

/* storage key id vs format str */
char runner_key_fmt_str[NVRAM_RNR_MAX_KEYS][NVRAM_RNR_KEY_STR_LEN] = {
    "dhd_rnr_offload_override",
};

#ifndef CONFIG_BCM_PON
static int emac_map = 0;
#endif

static int ext_sw_pid = BP_NO_EXT_SW;

extern int bdmf_history_init(uint32_t size, bdmf_boolean record_on);
#ifndef CONFIG_BCM_PON
#include <linux/of.h>

static int rdpa_get_init_system_dt_params(rdpa_system_init_cfg_t  *init_cfg)
{
    int rc = 0;
#ifdef CONFIG_BRCM_QEMU
    emac_map = 0xF;
#else // !QEMU
    struct device_node *np, *child, *child_p;
    const unsigned int *port_reg;
    uint32_t port_index;

    if ((np = of_find_compatible_node(NULL, NULL, "brcm,enet")))
    {
        for_each_available_child_of_node(np, child)
        {
            for_each_available_child_of_node(child, child_p)
            {
                port_reg = of_get_property(child_p, "reg", NULL);
                port_index = be32_to_cpup(port_reg);

                if (of_property_read_bool(child_p, "management")) continue;

                emac_map |= 1 << port_index;
                if (of_parse_phandle(child_p, "link", 0))
                    ext_sw_pid = port_index;
            }
        }
    }

    if (np)
        of_node_put(np);

#if defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813)
    for_each_compatible_node(np, NULL, "brcm,bcaphy")
    {
        const char *m_type_str = of_get_property(np, "usxgmii-m-type", NULL);

        if (!m_type_str) continue;
        if (!strcasecmp(m_type_str, "10G-SXGMII"))      init_cfg->usxgmiim_port_cfg = rdpa_usxgmiim_1_port;
        else if (!strcasecmp(m_type_str, "10G-DXGMII")) init_cfg->usxgmiim_port_cfg = rdpa_usxgmiim_2_port;
        else if (!strcasecmp(m_type_str, "10G-QXGMII")) init_cfg->usxgmiim_port_cfg = rdpa_usxgmiim_4_port;
        else if (!strcasecmp(m_type_str, "5G-SXGMII"))  init_cfg->usxgmiim_port_cfg = rdpa_usxgmiim_1_port;
        else if (!strcasecmp(m_type_str, "5G-DXGMII"))  init_cfg->usxgmiim_port_cfg = rdpa_usxgmiim_2_port;
        else if (!strcasecmp(m_type_str, "2.5G-SXGMII"))init_cfg->usxgmiim_port_cfg = rdpa_usxgmiim_1_port;
        break;
    }
#endif //4912
#endif // !QEMU

    printk("rdpa_get_init_system_dt_params: emac_map=%x ext_sw_pid=%d usxgmiim_cnt=%d\n", emac_map, ext_sw_pid, init_cfg->usxgmiim_port_cfg);
    return rc;
}
#endif

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

int operation_mode_get(rdpa_operation_mode *operation_mode)
{
    *operation_mode = rdpa_method_fc;
#if defined(CONFIG_BCM_PLATFORM_RDP_PRV)
    *operation_mode = rdpa_method_prv;
#endif

   return 0;
}

#ifdef XRDP

static int nvram_runner_key_get(int key_id, char *buff, int len)
{
    char *key_str = NULL;

    if (key_id >= NVRAM_RNR_MAX_KEYS)
        return 0;

    key_str = nvram_k_get(runner_key_fmt_str[key_id]);

    if (key_str)
    {
        /* Copy the key value */
        snprintf(buff, len, "%s", key_str);
        len = strlen(key_str);
        if (len < strlen(key_str))
        {
            printk("runner NVRAM param [%s] exceeds local buffer len [%d]\r\n", key_str, len);
        }
    } else
    {
        /* No key present in the NVRAM */
        len = 0;
    }
    return len;
}

/************************************************************************/
/* bdmf_history_buff_size_get - get history buffer size from scratchpad */
/* return 0 - ok-1 error                                                */
/************************************************************************/
static inline int bdmf_history_buff_size_get(int *history_buf_size)
{
    char buf[PSP_BUFLEN_16];
    int n;
    int rc;

    rc = scratchpad_get_or_init(BDMF_HISTORY_BUFF_SIZE_PSP_KEY, buf, sizeof(buf), NULL, scratchpad_func_get);
    if (rc <= 0)
    {
        return -1;
    }

    if (!sscanf(buf, "%i", &n))
    {
        printk("ERR: Failed convert <%s> to number.\n",
            buf);

        return -1;
    }

    *history_buf_size = n;

    return 0;
}

/******************************************************************************/
/* bdmf_history_buff_size_config - config history buffer size from scratchpad */
/* if history buffer size exist - bdmf history recording will be start        */
/******************************************************************************/
static inline void bdmf_history_buff_size_config(void)
{
    int history_buf_size = 0, rc;
    /* start history recording if needed */
    rc = bdmf_history_buff_size_get(&history_buf_size);

    if (!rc)
    {
        rc = bdmf_history_init(history_buf_size, 1);
        if (rc)
            printk("%s:%d ERR: Failed to configure bdmf_history, err %d\n", __FUNCTION__, __LINE__, rc);
        else
            printk("Read bdmf_history_buf_size %d \n", history_buf_size);
    }
}
#ifdef CONFIG_BLOG
int fc_tcp_ack_prio_ena_get(int *tcp_ack_prio_ena)
{
    int rc;
    char buf[PSP_BUFLEN_16];

    *tcp_ack_prio_ena = 0; /* Default */

    rc = scratchpad_get_or_init(FC_TCP_ACK_PRIO_ENABLE, buf, sizeof(buf), NULL, scratchpad_func_get);
    if (rc <= 0)
        return rc;

    if (!strcmp(buf ,"1"))
    {
        *tcp_ack_prio_ena = 1;
    }

    return 0;
}
#endif /* CONFIG_BLOG */

int dhd_offload_config_get(uint32_t *dhd_offload_bitmask)
{
    char buff[NVRAM_RNR_RES_STR_LEN];
    int  length = 0;

    /* Fetch the offlaod override Configuration information of the radio, if present */
    memset(buff, 0, sizeof(buff));
    length = nvram_runner_key_get(NVRAM_RNR_KEY_DHD_OFFL_OVERRIDE, buff, sizeof(buff));

    if (length == 0)
    {
        *dhd_offload_bitmask = 0xff;
    }
    else
    {
        sscanf(buff, "%d", dhd_offload_bitmask);
    }

    return 0;
}
#endif

int qm_queues_size_get(rdpa_qm_cfg_t *qm_cfg_table)
{
    int rc;
    uint32_t val;
    char buf[PSP_BUFLEN_16];

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

static int rdpa_create_cpu_port_and_filter(void)
{
    int rc;
    bdmf_object_handle rdpa_cpu_port_obj = NULL;
    bdmf_object_handle cpu =  NULL;
    bdmf_object_handle rdpa_filter_obj = NULL;
    BDMF_MATTR_ALLOC(rdpa_port_attrs, rdpa_port_drv());
    BDMF_MATTR_ALLOC(rdpa_filter_attrs, rdpa_filter_drv());

    rdpa_port_name_set(rdpa_port_attrs, DEF_RDPA_CPU_PORT_OBJ_NAME);
    rdpa_port_type_set(rdpa_port_attrs, rdpa_port_cpu);
    rdpa_port_index_set(rdpa_port_attrs, 0);
    rdpa_port_is_wan_set(rdpa_port_attrs, 0);
    rdpa_port_handle_set(rdpa_port_attrs, RDPA_PORT_CPU_HANDLE);

    rc = bdmf_new_and_set(rdpa_port_drv(), NULL, rdpa_port_attrs, &rdpa_cpu_port_obj);
    if (rc < 0)
    {
        BDMF_TRACE_ERR("Failed to create cpu port object\n");
        goto exit;
    }

    rc = rdpa_cpu_get(rdpa_cpu_host, &cpu);

    /* CPU Interrupts are no longer automatically connected as part of CPU object
     * creation. They need to be explicitly enabled using below rdpa call. */
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
    BDMF_MATTR_FREE(rdpa_port_attrs);
    BDMF_MATTR_FREE(rdpa_filter_attrs);
    if (cpu)
        bdmf_put(cpu);

    return rc;

}

#ifdef CONFIG_BCM_PON

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
    BDMF_MATTR_ALLOC(rdpa_system_attrs, rdpa_system_drv());
    bdmf_object_handle rdpa_system_obj = NULL;
#if defined(CONFIG_BCM_RUNNER_QOS_MAPPER)
    bdmf_object_handle rdpa_pbit_to_queue_obj = NULL;
    BDMF_MATTR_ALLOC(rdpa_pbit_to_queue_attrs, rdpa_pbit_to_queue_drv());
#if defined(CONFIG_BCM_PLATFORM_RDP_PRV)
    bdmf_object_handle rdpa_tc_to_queue_obj = NULL;
    BDMF_MATTR_ALLOC(rdpa_tc_to_queue_attrs, rdpa_tc_to_queue_drv());
#endif 

    int i;
#endif
    int rc;
    rdpa_system_init_cfg_t sys_init_cfg = {};
    int rc_dqm;
    int rc_cntr_cfg;
#ifdef CONFIG_BLOG
    int tcp_ack_prio_ena = 0;
#endif /* CONFIG_BLOG */
    int dhd_offload_bitmask = 0;
    rdpa_qm_cfg_t qm_cfg = {};
    rdpa_counter_cfg_t counter_cfg = {};

#if defined(XRDP) && !defined(BUFMNG_HW_RUNNER_SUPPORT)
    rdpa_packet_buffer_cfg_t packet_buffer_cfg = {};
#endif
    rdpa_system_cfg_t sys_cfg = {};



    /* Default values */
    sys_init_cfg.enabled_emac = RDPA_PORT_ALL_EMACS;
    sys_init_cfg.operation_mode = rdpa_method_fc;
    sys_init_cfg.runner_ext_sw_cfg.enabled = 0;
    sys_init_cfg.switching_mode = rdpa_switching_none;
    sys_init_cfg.us_ddr_queue_enable = 0;

    /* configure bdmf history if scratchpad key exist */
    bdmf_history_buff_size_config();

    sys_cfg.def_max_pkt_size = BCM_MAX_PKT_LEN_MTU_BASED;

    sys_cfg.car_mode = 1;

    sys_init_cfg.runner_ext_sw_cfg.emac_id = (ext_sw_pid == BP_NO_EXT_SW) ? rdpa_emac_none:(rdpa_emac)ext_sw_pid;
    sys_init_cfg.runner_ext_sw_cfg.enabled = (ext_sw_pid == BP_NO_EXT_SW) ? 0 : 1;
#if defined(G9991_COMMON)
    sys_init_cfg.runner_ext_sw_cfg.enabled = 1;
    sys_init_cfg.runner_ext_sw_cfg.type = rdpa_brcm_fttdp;
#else
    sys_init_cfg.runner_ext_sw_cfg.type = rdpa_brcm_hdr_opcode_0;
#endif

    rc = operation_mode_get(&sys_init_cfg.operation_mode);
    if (rc)
        goto exit;

#ifdef CONFIG_BLOG
    rc = fc_tcp_ack_prio_ena_get(&tcp_ack_prio_ena);
    if (rc<0)
        goto exit;
    if (rc>0)
        blog_support_set_tcp_ack_mflows(tcp_ack_prio_ena);
#endif /* CONFIG_BLOG */

    dhd_offload_config_get(&dhd_offload_bitmask);
    sys_init_cfg.dhd_offload_bitmask = dhd_offload_bitmask;

#if !defined(RDP_UFC)
    rc = rc ? : iptv_table_size_get(&sys_init_cfg.iptv_table_size);
    if (rc)
        goto exit;
    rc_cntr_cfg = vlan_stats_enable_get(&counter_cfg.vlan_stats_enable);
#endif
    rc_dqm = qm_queues_size_get(&qm_cfg);

#ifndef BRCM_CMS_BUILD
    rc = us_ddr_queue_enable_get(&sys_init_cfg.us_ddr_queue_enable);
    if (rc)
        goto exit;
#endif

    rdpa_system_cfg_set(rdpa_system_attrs, &sys_cfg);
    rdpa_system_init_cfg_set(rdpa_system_attrs, &sys_init_cfg);

#if defined(XRDP) && !defined(BUFMNG_HW_RUNNER_SUPPORT)
    packet_buffer_cfg.us_prio_rsv_thrs.min_buf_rsv_threshold = 1;
    packet_buffer_cfg.ds_prio_rsv_thrs.min_buf_rsv_threshold = 1;
    packet_buffer_cfg.wlan_prio_rsv_thrs.high_prio_buf_threshold = 2;
    packet_buffer_cfg.wlan_prio_rsv_thrs.excl_prio_buf_threshold = 5;
    rdpa_system_packet_buffer_cfg_set(rdpa_system_attrs, &packet_buffer_cfg);
#endif


    if (rc_dqm == 0)
        rdpa_system_qm_cfg_set(rdpa_system_attrs, &qm_cfg);
    if (rc_cntr_cfg == 0)
        rdpa_system_counter_cfg_set(rdpa_system_attrs, &counter_cfg);


    rc = bdmf_new_and_set(rdpa_system_drv(), NULL, rdpa_system_attrs, &rdpa_system_obj);
    if (rc)
    {
        printk("%s %s Failed to create rdpa system object rc(%d)\n", __FILE__, __FUNCTION__, rc);
        goto exit;
    }

    rc = rdpa_create_cpu_port_and_filter();
    if (rc)
    {
        BDMF_TRACE_ERR("Failed to create cpu port and filter\n");
        goto exit;
    }

#if defined(CONFIG_BCM_RUNNER_QOS_MAPPER)
#if defined(CONFIG_BCM_PLATFORM_RDP_PRV)
    /* Configure tc to q table */
    rdpa_tc_to_queue_table_set(rdpa_tc_to_queue_attrs, 0);
    rc = bdmf_new_and_set(rdpa_tc_to_queue_drv(), NULL, rdpa_tc_to_queue_attrs, &rdpa_tc_to_queue_obj);
    if (rc)
    {
        printk("%s %s Failed to create rdpa tc_to_queue object rc(%d)\n", __FILE__, __FUNCTION__, rc);
        goto exit;
    }
#endif
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
#if defined(CONFIG_BCM_PLATFORM_RDP_PRV)    
        rdpa_tc_to_queue_tc_map_set(rdpa_tc_to_queue_obj, i, 0);
#endif
        rdpa_pbit_to_queue_pbit_map_set(rdpa_pbit_to_queue_obj, i, 0);
    }
#endif

exit:
    if (rc && rdpa_system_obj)
        bdmf_destroy(rdpa_system_obj);

    BDMF_MATTR_FREE(rdpa_system_attrs);
#if defined(CONFIG_BCM_RUNNER_QOS_MAPPER)
#if defined(CONFIG_BCM_PLATFORM_RDP_PRV)    
    BDMF_MATTR_FREE(rdpa_tc_to_queue_attrs);
#endif
    BDMF_MATTR_FREE(rdpa_pbit_to_queue_attrs);
#endif

    return rc;
}

#else /* !defined(CONFIG_BCM_PON) */

int rdpa_init_system(void)
{
    BDMF_MATTR_ALLOC(rdpa_system_attrs, rdpa_system_drv());
    bdmf_object_handle rdpa_system_obj = NULL;
    int rc;
    int dhd_offload_bitmask = 0;
    rdpa_system_init_cfg_t sys_init_cfg = {};

    rdpa_system_cfg_t sys_cfg = {};
#if defined(XRDP) && !defined(BUFMNG_HW_RUNNER_SUPPORT)
    rdpa_packet_buffer_cfg_t packet_buffer_cfg = {};
#endif
    bdmf_object_handle cpu =  NULL;

    /* Default values */
    sys_init_cfg.enabled_emac = 0;
    sys_init_cfg.operation_mode = rdpa_method_fc;
    sys_init_cfg.runner_ext_sw_cfg.enabled = 0;
    sys_init_cfg.switching_mode = rdpa_switching_none;
    sys_init_cfg.us_ddr_queue_enable = 0;

    /* configure bdmf history if scratchpad key exist */
    bdmf_history_buff_size_config();

    rc = rdpa_get_init_system_dt_params(&sys_init_cfg);
    if (rc)
        goto exit;

    sys_cfg.def_max_pkt_size = BCM_MAX_PKT_LEN_MTU_BASED;

    sys_cfg.car_mode = 1;

    sys_init_cfg.enabled_emac = emac_map;

    sys_init_cfg.runner_ext_sw_cfg.emac_id = (ext_sw_pid == BP_NO_EXT_SW) ? rdpa_emac_none:(rdpa_emac)ext_sw_pid;
    sys_init_cfg.runner_ext_sw_cfg.enabled = (ext_sw_pid == BP_NO_EXT_SW) ? 0 : 1;
    sys_init_cfg.runner_ext_sw_cfg.type = rdpa_brcm_hdr_opcode_0;

    dhd_offload_config_get(&dhd_offload_bitmask);
    sys_init_cfg.dhd_offload_bitmask = dhd_offload_bitmask;

    rdpa_system_cfg_set(rdpa_system_attrs, &sys_cfg);
    rdpa_system_init_cfg_set(rdpa_system_attrs, &sys_init_cfg);

#if defined(XRDP) && !defined(BUFMNG_HW_RUNNER_SUPPORT)
    packet_buffer_cfg.us_prio_rsv_thrs.high_prio_buf_threshold = 5;
    packet_buffer_cfg.us_prio_rsv_thrs.min_buf_rsv_threshold = 10;
    packet_buffer_cfg.wlan_prio_rsv_thrs.high_prio_buf_threshold = 2;
    packet_buffer_cfg.wlan_prio_rsv_thrs.excl_prio_buf_threshold = 5;
    rdpa_system_packet_buffer_cfg_set(rdpa_system_attrs, &packet_buffer_cfg);
#endif


    rc = bdmf_new_and_set(rdpa_system_drv(), NULL, rdpa_system_attrs, &rdpa_system_obj);
    if (rc)
    {
        printk("%s %s Failed to create rdpa system object rc(%d)\n", __FILE__, __FUNCTION__, rc);
        goto exit;
    }

    rc = rdpa_create_cpu_port_and_filter();
    if (rc)
    {
        BDMF_TRACE_ERR("Failed to create cpu port and filter\n");
        goto exit;
    }

exit:
    BDMF_MATTR_FREE(rdpa_system_attrs);

    if (cpu)
        bdmf_put(cpu);
    if (rc && rdpa_system_obj)
        bdmf_destroy(rdpa_system_obj);

    return rc;
}
#endif /* defined(CONFIG_BCM_PON)*/

#elif defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908)

int rdpa_init_system(void)
{
    BDMF_MATTR_ALLOC(rdpa_system_attrs, rdpa_system_drv());
    bdmf_object_handle rdpa_system_obj = NULL;
    BDMF_MATTR_ALLOC(rdpa_filter_attrs, rdpa_filter_drv());
    bdmf_object_handle rdpa_filter_obj = NULL;
    int rc;
    rdpa_system_init_cfg_t sys_init_cfg = {};
    bdmf_object_handle cpu =  NULL;

    /* Default values */
    sys_init_cfg.enabled_emac = 0;
    sys_init_cfg.operation_mode = rdpa_method_fc;
    sys_init_cfg.runner_ext_sw_cfg.enabled = 0;
    sys_init_cfg.switching_mode = rdpa_switching_none;
    sys_init_cfg.us_ddr_queue_enable = 0;

    rc = rdpa_get_init_system_dt_params(&sys_init_cfg);
    if (rc)
        goto exit;

    sys_init_cfg.enabled_emac = emac_map;
    sys_init_cfg.runner_ext_sw_cfg.emac_id = (ext_sw_pid == BP_NO_EXT_SW) ? rdpa_emac_none:(rdpa_emac)ext_sw_pid;
    sys_init_cfg.runner_ext_sw_cfg.enabled = (ext_sw_pid == BP_NO_EXT_SW) ? 0 : 1;
    sys_init_cfg.runner_ext_sw_cfg.type = rdpa_brcm_hdr_opcode_0;
    sys_init_cfg.switching_mode = rdpa_switching_none;
    sys_init_cfg.operation_mode = rdpa_method_fc;

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

    BDMF_MATTR_FREE(rdpa_system_attrs);
    BDMF_MATTR_FREE(rdpa_filter_attrs);
    return rc;
}

#endif

