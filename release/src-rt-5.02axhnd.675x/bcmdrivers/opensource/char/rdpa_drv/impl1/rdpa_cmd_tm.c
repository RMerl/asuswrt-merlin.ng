
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

/*
 *******************************************************************************
 * File Name  : rdpa_cmd_tm.c
 *
 * Description: This file contains the RDPA Traffic Manager configuration API.
 *
 *******************************************************************************
 */

#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/bcm_log.h>
#include "bcmenet.h"
#include "bcmtypes.h"
#include "bcmnet.h"
#include "rdpa_types.h"
#include "rdpa_api.h"
#include "rdpa_ag_port.h"
#include "rdpa_drv.h"
#include "rdpa_cmd_tm.h"
#include "bcm_OS_Deps.h"
#include "rdpa_cmd_misc.h"
#include "bcm_hwdefs.h"

#if defined (CONFIG_BCM_PON) || defined(CONFIG_BCM_XRDP)
#include <rdpa_epon.h>
#include "rdpa_ag_epon.h"
#endif

#define __BDMF_LOG__
/* #define __DUMP_PORTS__ */

#define CMD_TM_LOG_ID_RDPA_CMD_DRV BCM_LOG_ID_RDPA_CMD_DRV

#if defined(__BDMF_LOG__)
#define CMD_TM_LOG_ERROR(fmt, args...)                                         \
    do {                                                                            \
        if (bdmf_global_trace_level >= bdmf_trace_level_error)                      \
            bdmf_trace("ERR: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);    \
    } while(0)
#define CMD_TM_LOG_INFO(fmt, args...)                                         \
    do {                                                                            \
        if (bdmf_global_trace_level >= bdmf_trace_level_info)                          \
            bdmf_trace("INF: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);    \
    } while(0)
#define CMD_TM_LOG_DEBUG(fmt, args...)                                         \
    do {                                                                            \
        if (bdmf_global_trace_level >= bdmf_trace_level_debug)                          \
            bdmf_trace("DBG: %s#%d: " fmt "\n", __FUNCTION__, __LINE__, ## args);    \
    } while(0)
#else
#define CMD_TM_LOG_ERROR(fmt, arg...) BCM_LOG_ERROR(fmt, arg...)
#define CMD_TM_LOG_INFO(fmt, arg...)   BCM_LOG_INFO(fmt, arg...)
#define CMD_TM_LOG_DEBUG(fmt, arg...) BCM_LOG_DEBUG(fmt, arg...)
#endif

#define MAX_TM_PER_PORT    32
#define MAX_RUT_QOS_PORTS  16 /* rdpa_if_lan_max is the maximum */
#define MAX_Q_PER_TM       8
#define MAX_Q_PER_SID      4
#define MAX_WRR_WEIGHT     64
#define TCONT_LLID_VP_BASE (MAX_RUT_QOS_PORTS)
#define DS_SVCQ_VP_BASE    (TCONT_LLID_VP_BASE + RDPA_MAX_TCONT)
#define MAX_VIRT_TM_PORTS  (DS_SVCQ_VP_BASE + 3)  /* multiwan introduce port_id that goes byhond the 1 extra.. */

#define QUEUE_SIZE_FACTOR 1536

#if defined(CONFIG_BCM_XRDP) 
#define getDeviceQueueSize(usrSize) (usrSize * QUEUE_SIZE_FACTOR)
#define getUsrQueueSize(devSize) (devSize / QUEUE_SIZE_FACTOR)
#else
#define getDeviceQueueSize(usrSize) (usrSize)
#define getUsrQueueSize(devSize) (devSize)
#endif /* CONFIG_BCM_XRDP */

#define QUEUE_SIZE 256
#define VACANT     -1

#define INVALID_ID -1

/* proc debug tool. */
#define RDPADRV_PROC_ENTRY_NAME "driver/rdpadrv"
#define RDPADRV_PROC_TM_NAME    "tm"
#define RDPADRV_PROC_MAX_ARGS 5
#define RDPADRV_PROC_WR_KBUF_SIZE 128

/* Unlike others, 6858 GPON upstream use single tm structure.
     By default, q_num is set to 32 and sp_q_num is set to 16.
     The upper 16 are for sp queues, while the lower 16 are for wrr.
     So wrr queue must begin from index 16 to 31. */
#define DFT_SINGLE_TM_Q_NUM      32
#define DFT_SINGLE_TM_SP_Q_NUM   16
#define FIRST_SINGLE_TM_WRR_Q    16


typedef struct {
    uint32 tm_index;
} rdpa_cmd_tm_ctrl_t;

typedef struct {
    BOOL        alloc;
    int         tm_id;
    bdmf_number qid;
    int         q_index;
    uint32_t    weight;
} queue_st;

typedef struct {
    BOOL               alloc;
    int                root_tm_id;
    int                root_sp_tm_id;
    int                root_wrr_tm_id;
    int                orl_id;
    BOOL               orl_linked;
    rdpa_if            port_id;
    rdpa_tm_sched_mode arbiter_mode;    /* root TM arbiter mode */
    queue_st           queue_list[MAX_Q_PER_TM];
    int                queue_alloc_cnt;
    uint32_t           cfg_flags;
} port_st;

/* Private Function Prototypes. */

static void rdpa_drv_tm_procCreate(void);
static void rdpa_drv_tm_procDelete(void);
static ssize_t rdpa_drv_tm_proc_read(struct file *file, char *buf,
  size_t count, loff_t *pos);
static ssize_t rdpa_drv_tm_proc_write(struct file *file, const char *buf,
  size_t cnt, loff_t *pos);
static void print_all_ports(void);
static void print_port_queue(uint32_t virt_port_id);


/* Private Variables. */

static port_st port_list[MAX_VIRT_TM_PORTS];
static struct proc_dir_entry *proc_dir;

static struct file_operations rdpa_drv_tm_proc_fops =
{
    .owner = THIS_MODULE,
    .read  = rdpa_drv_tm_proc_read,
    .write = rdpa_drv_tm_proc_write,
};

/* Only for epon special case use.
    In DPoE SFU/HGU, we share TM for control and data queues */
static BOOL share_root_tm = FALSE;

static BOOL svcq_enable = FALSE;

/*******************************************************************************/
/* static routines Functions                                                   */
/*******************************************************************************/

static inline rdpa_if convert_dev_type_id_to_if(uint32_t dev_type, uint32_t dev_id)
{
    rdpa_if _if = rdpa_if_none;
    switch ( dev_type )
    {
    case RDPA_IOCTL_DEV_PORT:
        _if = (rdpa_if)dev_id;
        break;

    case RDPA_IOCTL_DEV_XTM:
        _if = rdpa_wan_type_to_if(rdpa_wan_dsl);
        break;

    case RDPA_IOCTL_DEV_LLID:
        _if = rdpa_wan_type_to_if(rdpa_wan_epon);
        break;

    case RDPA_IOCTL_DEV_TCONT:
        _if = rdpa_wan_type_to_if(rdpa_wan_gpon);
        break;

    case RDPA_IOCTL_DEV_NONE:
        _if = (rdpa_if)dev_id; /* ?? */
        break;

    default:
        CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)", dev_type, dev_id);
    }

    return _if;
}

static inline port_st* get_virt_port(uint32_t dev_type, uint32_t dev_id)
{
    port_st  *pport;
    uint32_t vp_offset;

    if ((dev_type == RDPA_IOCTL_DEV_TCONT) ||
       (dev_type == RDPA_IOCTL_DEV_LLID)) {
        if (dev_id >= RDPA_MAX_TCONT) {
            CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)", dev_type, dev_id);
            return NULL;
        }
        vp_offset = dev_id + TCONT_LLID_VP_BASE;
    }
    else if (dev_type == RDPA_IOCTL_DEV_NONE) {
        vp_offset = dev_id + DS_SVCQ_VP_BASE;
    }
    else {
        if (dev_id >= MAX_RUT_QOS_PORTS) {
            CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)", dev_type, dev_id);
            return NULL;
        }
        vp_offset = dev_id;
    }

    CMD_TM_LOG_DEBUG("devtype(%u) portid(%u) vpoffset(%d)", dev_type, dev_id, vp_offset);
    if (vp_offset >= MAX_VIRT_TM_PORTS) {
        CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u) vpoffset(%d)",
                         dev_type, dev_id, vp_offset);
        return NULL;
    }

    pport = &port_list[vp_offset];
    return pport;
}

static void init_portlist_entry(port_st* pport)
{
    queue_st *pq;
    int j;

    pq = &(pport->queue_list[0]);

    memset(pport, 0, sizeof(port_st));
    pport->root_tm_id = pport->root_sp_tm_id = INVALID_ID;
    pport->root_wrr_tm_id = pport->orl_id = INVALID_ID;

    for (j = 0; j < MAX_Q_PER_TM; j++, pq++) {
        pq->tm_id = INVALID_ID;
        pq->q_index = INVALID_ID;
    }
}

static uint32_t get_port_tm_caps(rdpa_traffic_dir dir)
{
    uint32_t sched_caps = 0;
    
    if (dir == rdpa_dir_us) 
    {      
#if defined(CONFIG_BCM_XRDP) 
#if defined(CONFIG_BCM_EPON_STACK) || defined(CONFIG_BCM_EPON_STACK_MODULE)
        rdpa_wan_type wan_type = rdpa_wan_none;
        rdpa_if wan_if = rdpa_wan_type_to_if(rdpa_wan_epon); /* rdpa_if_wanX will be same for epon/xepon */

        get_rdpa_wan_type(wan_if, &wan_type);
        if ((wan_type == rdpa_wan_epon || wan_type == rdpa_wan_xepon) && !is_ae_enable(wan_if))
        {
            int rc = 0;
            bdmf_object_handle epon_obj = NULL;

            rc = rdpa_epon_get(&epon_obj);
            if (rc == 0)
            {
                rdpa_epon_mode epon_mode;
                rc = rdpa_epon_mode_get(epon_obj, &epon_mode);
                if (rc == 0 && epon_mode == rdpa_epon_dpoe)
                    sched_caps |= RDPA_TM_1LEVEL_CAPABLE;
            }
            
            if (epon_obj)
                bdmf_put(epon_obj);
        }
        else
#endif /* CONFIG_BCM_EPON_STACK ||  CONFIG_BCM_EPON_STACK_MODULE */
        {
            sched_caps |= RDPA_TM_1LEVEL_CAPABLE;
        }   
#endif /* CONFIG_BCM_XRDP */

        sched_caps |= RDPA_TM_SP_CAPABLE | RDPA_TM_WRR_CAPABLE |
                                RDPA_TM_SP_WRR_CAPABLE;

    }
    else {
        sched_caps = RDPA_TM_SP_CAPABLE;
    }

    return sched_caps;
}

#define ONE_MB (1024 * 1024)
extern int BcmMemReserveGetByName(char *name, void **virt_addr, phys_addr_t* phys_addr, unsigned int *size);
static int get_tm_memory_info(rdpa_drv_ioctl_tm_t *tm_p)
{
    void *fpm_pool_addr;
    uint32_t fpm_pool_size;
    phys_addr_t fpm_pool_phys_addr;
    /* fetch the reserved-memory from device tree */
    if (BcmMemReserveGetByName(FPMPOOL_BASE_ADDR_STR, &fpm_pool_addr, &fpm_pool_phys_addr, &fpm_pool_size))
    {
        return RDPA_DRV_ERROR;
    }
    tm_p->fpm_pool_memory_size = fpm_pool_size / ONE_MB;
    return 0;
}

static int get_port_tm_parms(rdpa_drv_ioctl_tm_t *tm_p)
{
    port_st *port_p = NULL;

    CMD_TM_LOG_DEBUG("IN: GET PORT TM port_id(%u)", tm_p->dev_id);

    port_p = get_virt_port(tm_p->dev_type, tm_p->dev_id);
    if (port_p == NULL) {
        CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)", tm_p->dev_type, tm_p->dev_id);
        return RDPA_DRV_PORT_ID_NOT_VALID;
    }

    tm_p->cfg_flags     = port_p->cfg_flags;
    tm_p->max_queues    = MAX_Q_PER_TM;
    tm_p->max_sp_queues = MAX_Q_PER_TM;
    tm_p->port_shaper   = TRUE;

    if (tm_p->dir == rdpa_dir_us) {
        tm_p->queue_shaper    = TRUE; 
    }
    else {
        tm_p->queue_shaper    = FALSE;
#ifdef CONFIG_BCM_FTTDP_G9991
        tm_p->max_queues    = MAX_Q_PER_SID;
        tm_p->max_sp_queues = MAX_Q_PER_SID;
#endif
    }

    tm_p->port_sched_caps = get_port_tm_caps(tm_p->dir);
        
    return 0;
}

static uint32_t get_tcont_tm_caps(void)
{
    uint32_t sched_caps = 0;

    sched_caps = RDPA_TM_SP_CAPABLE | RDPA_TM_WRR_CAPABLE |
                            RDPA_TM_SP_WRR_CAPABLE;

#if defined(CONFIG_BCM_XRDP) 
    sched_caps |= RDPA_TM_1LEVEL_CAPABLE;
#endif

    return sched_caps;
}

static int get_tcont_tm_parms(rdpa_drv_ioctl_tm_t *tm_p)
{
    port_st *port_p = NULL;
    
    CMD_TM_LOG_DEBUG("IN: GET TCONT TM dev_id(%u)", tm_p->dev_id);

    port_p = get_virt_port(tm_p->dev_type, tm_p->dev_id);
    if (port_p == NULL) {
        CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)", tm_p->dev_type, tm_p->dev_id);
        return RDPA_DRV_PORT_ID_NOT_VALID;
    }

    tm_p->cfg_flags     = port_p->cfg_flags;
    tm_p->max_queues    = MAX_Q_PER_TM;
    tm_p->max_sp_queues = MAX_Q_PER_TM;
    tm_p->port_shaper   = TRUE;
    tm_p->queue_shaper    = TRUE;
    tm_p->port_sched_caps = get_tcont_tm_caps();

    return 0;
}

static uint32_t get_llid_tm_caps(void)
{
    uint32_t sched_caps = RDPA_TM_SP_CAPABLE;
    
#if defined(CONFIG_BCM_XRDP)
    int rc = 0;
    bdmf_object_handle epon_obj = NULL;

    rc = rdpa_epon_get(&epon_obj);
    if (rc == 0)
    {
        rdpa_epon_mode epon_mode;
        rc = rdpa_epon_mode_get(epon_obj, &epon_mode);
        if (rc == 0 && epon_mode == rdpa_epon_dpoe)
        {
            sched_caps = RDPA_TM_SP_CAPABLE | RDPA_TM_WRR_CAPABLE |
                         RDPA_TM_SP_WRR_CAPABLE | RDPA_TM_1LEVEL_CAPABLE;
        }
    }
    
    if (epon_obj)
        bdmf_put(epon_obj);
#endif

    return sched_caps;
}

static int get_llid_tm_parms(rdpa_drv_ioctl_tm_t *tm_p)
{
    port_st *port_p = NULL;
    
    CMD_TM_LOG_DEBUG("IN: GET LLID TM dev_id(%u)", tm_p->dev_id);

    port_p = get_virt_port(tm_p->dev_type, tm_p->dev_id);
    if (port_p == NULL) {
        CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)", tm_p->dev_type, tm_p->dev_id);
        return RDPA_DRV_PORT_ID_NOT_VALID;
    }

    tm_p->cfg_flags     = port_p->cfg_flags;
    tm_p->max_queues    = MAX_Q_PER_TM;
    tm_p->max_sp_queues = MAX_Q_PER_TM;
    tm_p->port_shaper   = TRUE;
    tm_p->queue_shaper  = TRUE;
    tm_p->port_sched_caps = get_llid_tm_caps();

    return 0;
}

static int get_svcq_tm_parms(rdpa_drv_ioctl_tm_t *tm_p)
{
    port_st *port_p = NULL;
    
    CMD_TM_LOG_DEBUG("IN: GET SVCQ TM dev_id(%u)", tm_p->dev_id);

    port_p = get_virt_port(tm_p->dev_type, tm_p->dev_id);
    if (port_p == NULL) {
        CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)", tm_p->dev_type, tm_p->dev_id);
        return RDPA_DRV_PORT_ID_NOT_VALID;
    }

    tm_p->cfg_flags     = port_p->cfg_flags;
    tm_p->max_queues    = MAX_Q_PER_TM;
    tm_p->max_sp_queues = MAX_Q_PER_TM;
    tm_p->port_shaper   = TRUE;
    tm_p->queue_shaper    = TRUE;
    tm_p->port_sched_caps = RDPA_TM_SP_CAPABLE;

    return 0;
}

static int add_root_tm_to_virt_port(
    uint32_t dev_type,
    uint32_t port_id,
    uint32_t tm_id,
    uint32_t arbiter_mode,
    uint32_t cfg_flags)
{
    port_st    *pport = NULL;

    CMD_TM_LOG_DEBUG("IN: ADDED ROOT TM(%u) devtype(%u) port_id(%u)",
                     tm_id, dev_type, port_id);

    pport = get_virt_port(dev_type, port_id);
    if (pport == NULL) {
        CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)", dev_type, port_id);
        return RDPA_DRV_PORT_ID_NOT_VALID;
    }

    if (pport->alloc) {
        CMD_TM_LOG_ERROR("FAIL: TM(%u) port_id(%u) is ALLOCATED", tm_id,  port_id);
        return RDPA_DRV_PORT_NOT_ALLOC;
    }

    pport->alloc        = TRUE;
    pport->root_tm_id   = tm_id;
    pport->port_id      = port_id;
    pport->arbiter_mode = arbiter_mode;
    pport->cfg_flags    = cfg_flags;

    return 0;
}

static int queue_allocate(
    uint32_t dev_type,
    uint32_t dev_id,
    uint32_t q_id,
    BOOL     alloc)
{
    port_st  *pport;

    CMD_TM_LOG_DEBUG("IN: dev_id=%u q_id=%u setalloc=%d", dev_id, q_id, alloc);

    if (q_id >= MAX_Q_PER_TM) {
        CMD_TM_LOG_ERROR("FAIL: Invalid q_id(%u)", q_id);
        return RDPA_DRV_Q_ID_NOT_VALID;
    }

    pport = get_virt_port(dev_type, dev_id);
    if (pport == NULL) {
        CMD_TM_LOG_ERROR("Invalid dev_type(%u) dev_id(%u)", dev_type, dev_id);
        return RDPA_DRV_PORT_ID_NOT_VALID;
    }

    pport->queue_list[q_id].alloc = alloc;

    return 0;
}

static int get_orl(
    uint32_t dev_type,
    uint32_t port_id,
    uint32_t dir,
    uint32_t *porl_id,
    uint32_t *pshaping_rate,
    BOOL     *porl_linked)
{
    int rc = 0;
    bdmf_object_handle   sched  = NULL;
    rdpa_egress_tm_key_t tm_key = {};
    rdpa_tm_rl_cfg_t     rl_cfg = {};
    port_st              *pport = NULL;

    CMD_TM_LOG_DEBUG("IN: port_id(%u)", port_id);

    *porl_id       = INVALID_ID;
    *pshaping_rate = 0;
    *porl_linked   = FALSE;

    pport = get_virt_port(dev_type, port_id);
    if (pport == NULL) {
        CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)", dev_type, port_id);
        return RDPA_DRV_PORT_ID_NOT_VALID;
    }

    if (pport->orl_id == INVALID_ID)
        return 0;   /* orl does not exist */

    /* get the orl shaping rate */
    tm_key.dir       = dir;
    tm_key.index = pport->orl_id;

    if ((rc = rdpa_egress_tm_get(&tm_key, &sched)) || (sched == NULL)) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_get() failed: orl_tm(%u) rc(%d)", pport->orl_id, rc);
        return RDPA_DRV_TM_GET;
    }

    if ((rc = rdpa_egress_tm_rl_get(sched, &rl_cfg))) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_rl_get() failed: orl_tm(%u) af(%llu) rc(%d)",
                         pport->orl_id, (uint64_t)rl_cfg.af_rate, rc);
        bdmf_put(sched);
        return RDPA_DRV_TM_CFG_GET;
    }

    bdmf_put(sched);

    *porl_id       = pport->orl_id;
    *porl_linked   = pport->orl_linked;
    *pshaping_rate = rl_cfg.af_rate / 1000;

    return 0;
}

static int add_q_to_virt_port(
    uint32_t dev_type,
    uint32_t port_id,
    uint32_t tm_id,
    uint32_t q_id,
    int      q_index,
    uint32_t weight)
{
    port_st *pport;
    queue_st *pq;

    CMD_TM_LOG_DEBUG("IN: dev_type(%u) port_id(%u) tm(%u) q_id(%u) index(%d) wt(%u)",
                     dev_type, port_id, tm_id, q_id, q_index, weight);

    if (q_id >= MAX_Q_PER_TM) {
        CMD_TM_LOG_ERROR("FAIL: Invalid q_id(%u)", q_id);
        return RDPA_DRV_Q_ID_NOT_VALID;
    }

    pport = get_virt_port(dev_type, port_id);
    if (pport == NULL) {
        CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)", dev_type, port_id);
        return RDPA_DRV_PORT_ID_NOT_VALID;
    }

    pq = &(pport->queue_list[q_id]);

    if (!pport->alloc) {
        CMD_TM_LOG_ERROR("FAIL: port_id(%u) NOT ALLOCATED", port_id);
        return RDPA_DRV_PORT_NOT_ALLOC;
    }

    pq->tm_id  = tm_id;
    pq->qid    = q_id;
    pq->weight = weight;

    pq->q_index = q_index;

    if (!pq->alloc)
    {
        pq->alloc  = TRUE;
        pport->queue_alloc_cnt++;
    }

    CMD_TM_LOG_DEBUG("OUT: dev_type(%u) port_id(%u) tm(%u) q_id(%u) index(%d) wt(%u)",
                     dev_type, port_id, tm_id, q_id, q_index, weight);

    return 0;
}

static int remove_q_from_virt_port(
    uint32_t dev_type,
    uint32_t port_id,
    uint32_t q_id)
{
    port_st *pport;
    queue_st *pq;

    CMD_TM_LOG_DEBUG("IN: dev_type(%u) port_id(%u) q_id(%u)",
                     dev_type, port_id, q_id);

    if (q_id >= MAX_Q_PER_TM) {
        CMD_TM_LOG_ERROR("FAIL: Invalid q_id(%u)", q_id);
        return RDPA_DRV_Q_ID_NOT_VALID;
    }

    pport = get_virt_port(dev_type, port_id);
    if (pport == NULL) {
        CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)", dev_type, port_id);
        return RDPA_DRV_PORT_ID_NOT_VALID;
    }

    pq = &(pport->queue_list[q_id]);

    if (!pport->alloc) {
        CMD_TM_LOG_ERROR("FAIL: port_id(%u) NOT ALLOCATED", port_id);
        return RDPA_DRV_PORT_NOT_ALLOC;
    }

    pq->alloc  = FALSE;
    pq->tm_id  = INVALID_ID;
    pq->qid    = q_id;
    pq->weight = 0;
    pq->q_index = INVALID_ID;
    pport->queue_alloc_cnt--;
    CMD_TM_LOG_DEBUG("OUT: dev_type(%u) port_id(%u) q_id(%u)",
                     dev_type, port_id, q_id);

    return 0;
}

static int get_q_from_virt_port(
    uint32_t dev_type,
    uint32_t port_id,
    uint32_t q_id,
    queue_st **ppq)
{
    port_st *pport;
    queue_st *pq = NULL;

    CMD_TM_LOG_DEBUG("IN: dev_type(%u) port_id(%u) q_id(%u)",
                     dev_type, port_id, q_id);

    if (q_id >= MAX_Q_PER_TM) {
        CMD_TM_LOG_ERROR("FAIL: Invalid q_id(%u)", q_id);
        return RDPA_DRV_Q_ID_NOT_VALID;
    }

    pport = get_virt_port(dev_type, port_id);
    if (pport == NULL) {
        CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)", dev_type, port_id);
        return RDPA_DRV_PORT_ID_NOT_VALID;
    }

    pq = &(pport->queue_list[q_id]);
    *ppq = pq;

    if (!pport->alloc) {
        CMD_TM_LOG_DEBUG("port_id(%u) NOT ALLOCATED", port_id);
        return RDPA_DRV_PORT_NOT_ALLOC;
    }

    if (pq->tm_id == INVALID_ID) {
        CMD_TM_LOG_DEBUG("q(%u) tm NOT ALLOCATED", q_id);
        return RDPA_DRV_TM_NOT_ALLOC;
    }

    return RDPA_DRV_SUCCESS;
}

/*
 * In queue deletion case, application provides the qid, but no priority.
 * Need to look up priority either based on qid, or saved data.
 */
static uint32 get_queue_index(rdpa_drv_ioctl_tm_t *ptm)
{
    queue_st *pq;
    int ret;

    // HACK GREG
    if (ptm->service_queue && ptm->dir == rdpa_dir_us)
       return ptm->q_id - 100;

    ret = get_q_from_virt_port(ptm->dev_type, ptm->dev_id, ptm->q_id, &pq);
    if (ret == RDPA_DRV_SUCCESS) {
        CMD_TM_LOG_DEBUG("OUT: dev_type(%u) dev_id(%u) q_id(%u) index(%u)",
                         ptm->dev_type, ptm->dev_id, ptm->q_id, pq->q_index);
        return pq->q_index;
    }
    else {
        CMD_TM_LOG_DEBUG("Q index not found, dev_type(%u) dev_id(%u) q_id(%u)",
                         ptm->dev_type, ptm->dev_id, ptm->q_id);
        return ptm->q_id;
    }
}

static int get_tm_by_qid(
    uint32_t dev_type,
    uint32_t dev_id,
    uint32_t q_id,
    uint32_t *ptm_id,
    BOOL     *pfound)
{
    int ret = RDPA_DRV_SUCCESS;

    CMD_TM_LOG_DEBUG("IN: dev_type(%u) dev_id(%u) q_id(%u)", dev_type, dev_id, q_id);

    /* initialize return values */
    *ptm_id = INVALID_ID;
    *pfound = FALSE;

    switch (dev_type)
    {
    case RDPA_IOCTL_DEV_PORT:
    {
        port_st  *pport;
        queue_st *pq;

        if (q_id >= MAX_Q_PER_TM) {
            CMD_TM_LOG_ERROR("FAIL: Invalid q_id(%u)", q_id);
            ret = RDPA_DRV_Q_ID_NOT_VALID;
            break;
        }
        pport = get_virt_port(dev_type, dev_id);
        if (pport == NULL) {
            CMD_TM_LOG_ERROR("Invalid devtype(%u) dev_id(%u)", dev_type, dev_id);
            return RDPA_DRV_PORT_ID_NOT_VALID;
        }

        if (!pport->alloc) {
            /* Port tm has not been configured.
             * Return as SUCCESS, but not found.
             */
            break;
        }

        pq = &(pport->queue_list[q_id]);
        *ptm_id = pq->tm_id;
        *pfound = pq->alloc;

        break;
    }
    default:
        ret = RDPA_DRV_ERROR;
        CMD_TM_LOG_ERROR("not implement for such dev type(%u) still", dev_type);
        break;
    }

    return ret;
}

static int get_virt_port_q_cfg(
    uint32_t dev_type,
    uint32_t dev_id,
    uint32_t q_id,
    uint32_t dir,
    rdpa_drv_ioctl_tm_t *q_info)
{
    bdmf_object_handle   sched = NULL;
    rdpa_tm_sched_mode   tm_mode;
    rdpa_egress_tm_key_t tm_key = {};
    rdpa_tm_queue_cfg_t  q_cfg  = {};
    rdpa_tm_rl_cfg_t     rl_cfg = {};
    bdmf_error_t         rc     = BDMF_ERR_OK;
    queue_st *pq;
    uint32_t q_index;
    int      ret = RDPA_DRV_SUCCESS;

    CMD_TM_LOG_DEBUG("IN: dev_type(%u dev_id(%u) q_id(%u) dir(%u)",
                     dev_type, dev_id, q_id, dir);

    /* initialize return values */
    q_info->tm_id        = INVALID_ID;
    q_info->min_rate     = 0;
    q_info->shaping_rate = 0;
    q_info->found        = FALSE;

    ret = get_q_from_virt_port(dev_type, dev_id, q_id, &pq);
    if (ret != RDPA_DRV_SUCCESS) {
        if ((ret == RDPA_DRV_PORT_NOT_ALLOC) || (ret == RDPA_DRV_TM_NOT_ALLOC)) {
            ret = RDPA_DRV_SUCCESS;
        }
        return ret;
    }

    /* Runner queue has been configured for this qid. Get queue config. */

    tm_key.dir   = dir ;
    tm_key.index = pq->tm_id;

    if ((rc = rdpa_egress_tm_get(&tm_key, &sched))) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_get() failed: tm(%d) rc(%d)", pq->tm_id, rc);
        return RDPA_DRV_TM_GET;
    }

    /* get tm mode */
    if ((rc = rdpa_egress_tm_mode_get(sched, &tm_mode))) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_mode_get() failed: rc(%d)", rc);
        ret = RDPA_DRV_MODE_GET;
        goto get_tm_exit;
    }

    if (tm_mode == rdpa_tm_sched_disabled) {
        /* get the shaping rate */
        if ((rc = rdpa_egress_tm_rl_get(sched, &rl_cfg))) {
            CMD_TM_LOG_ERROR("rdpa_egress_tm_rl_get() failed: tm(%u) rc(%d)", pq->tm_id, rc);
            ret = RDPA_DRV_TM_CFG_GET;
            goto get_tm_exit;
        }

        if (dir == rdpa_dir_us) {
            port_st                *pport       = NULL;
            bdmf_object_handle     root_sched   = NULL;
            rdpa_tm_rl_rate_mode   root_rl_mode = rdpa_tm_rl_single_rate;

            pport = get_virt_port(dev_type, dev_id);
            if (pport == NULL) {
                CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)", dev_type, dev_id);
                ret = RDPA_DRV_PORT_ID_NOT_VALID;
                goto get_tm_exit;
            }

            tm_key.dir   = dir;
            tm_key.index = pport->root_tm_id;

            if ((rc = rdpa_egress_tm_get(&tm_key, &root_sched))) {
                CMD_TM_LOG_ERROR("rdpa_egress_tm_get() failed: tm(%d) rc(%d)", pport->root_tm_id, rc);
                ret = RDPA_DRV_TM_GET;
                goto get_tm_exit;
            }
            
            if ((rc = rdpa_egress_tm_rl_rate_mode_get(root_sched, &root_rl_mode))) {
                CMD_TM_LOG_ERROR("rdpa_egress_tm_rl_rate_mode_get() failed: tm(%u) rc(%d)",
                                 pport->root_tm_id, rc);
                bdmf_put(root_sched);
                ret = RDPA_DRV_MODE_GET;
                goto get_tm_exit;
            }

            if (root_rl_mode == rdpa_tm_rl_dual_rate) {
                q_info->min_rate = rl_cfg.af_rate / 1000;
                if (rl_cfg.be_rate < 1000000000L)
                    q_info->shaping_rate = (rl_cfg.af_rate + rl_cfg.be_rate) / 1000;
            }
            else {
                q_info->shaping_rate = rl_cfg.af_rate / 1000 ;
            }

            bdmf_put(root_sched);
        }
        else {
            /* downstream must be single rate shaping */
            q_info->shaping_rate = rl_cfg.af_rate / 1000 ;
        }

        /* each shaper has only one queue. i.e. q_index = 0 */
        q_index = 0;
    }
    else {
        q_index = pq->q_index;
    }

    /* get qsize from q_cfg.drop_threshold. */
    if ((rc = rdpa_egress_tm_queue_cfg_get(sched, q_index, &q_cfg))) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_queue_cfg_get() failed: q_index %d, rc(%d)", q_index, rc);
        ret = RDPA_DRV_Q_CFG_GET;
        goto get_tm_exit;
    }

#if defined(CONFIG_BCM_XRDP) /* TBD - What is different between RDP & XRDP? */
    if (tm_mode != rdpa_tm_sched_disabled)
        q_info->shaping_rate = q_cfg.rl_cfg.af_rate / 1000;
#endif

    if (q_cfg.queue_id != q_id)
    {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_queue_cfg_get() unmatched data: "
                         "tm(%u) index(%u) user q_id(%u) q_id(%u) rc(%d)",
                         pq->tm_id, q_index, q_id, q_cfg.queue_id, rc);
        ret = RDPA_DRV_Q_CFG_GET;
        goto get_tm_exit;
    }

    q_info->tm_id  = pq->tm_id;
    q_info->index  = q_index;
    q_info->qsize  = getUsrQueueSize(q_cfg.drop_threshold);
    q_info->weight = pq->weight;
    q_info->minBufs = q_cfg.reserved_packet_buffers;
    q_info->found  = pq->alloc;
    q_info->drop_alg       = q_cfg.drop_alg;
    q_info->red_min_thr_lo = getUsrQueueSize(q_cfg.low_class.min_threshold);
    q_info->red_max_thr_lo = getUsrQueueSize(q_cfg.low_class.max_threshold);
    q_info->red_min_thr_hi = getUsrQueueSize(q_cfg.high_class.min_threshold);
    q_info->red_max_thr_hi = getUsrQueueSize(q_cfg.high_class.max_threshold);
    q_info->priority_mask_0 = q_cfg.priority_mask_0;
    q_info->priority_mask_1 = q_cfg.priority_mask_1;
    q_info->red_drop_percent_lo = RDPA_WRED_MAX_DROP_PROBABILITY;
    q_info->red_drop_percent_hi = RDPA_WRED_MAX_DROP_PROBABILITY;
    q_info->best_effort = q_cfg.best_effort;

    CMD_TM_LOG_DEBUG("OUT: dev_type(%u dev_id(%u) q_id(%u) dir(%u) "
                     "tm_id(%u) index(%u) size(%u) wt(%u) found(%u)",
                     dev_type, dev_id, q_id, dir,
                     q_info->tm_id, q_info->index, q_info->qsize, q_info->weight, q_info->found);

get_tm_exit:
    bdmf_put(sched);
    return ret;
}


#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158)
static int get_xtm_channel_q_cfg(
    uint32_t dev_type,
    uint32_t dev_id,
    uint32_t q_id,
    rdpa_drv_ioctl_tm_t *q_info)
{
    bdmf_object_handle xtm_obj = NULL, sched = NULL;
    rdpa_tm_queue_cfg_t q_cfg = {};
    bdmf_error_t rc = BDMF_ERR_OK;
    bdmf_number tm_idx;
    rdpa_tm_sched_mode tm_mode;
    rdpa_tm_rl_cfg_t rl_cfg = {};
    int ret = RDPA_DRV_SUCCESS;

    if ((rc = rdpa_xtmchannel_get(q_id, &xtm_obj)) || (xtm_obj == NULL)) {
        CMD_TM_LOG_ERROR("rdpa_xtmchannel_get() failed: channel(%d) rc(%d)", q_id, rc);
        return RDPA_DRV_PORT_GET;
    }

    if ((rc = rdpa_xtmchannel_egress_tm_get(xtm_obj, &sched))) {
        CMD_TM_LOG_ERROR("rdpa_xtmchannel_egress_tm_get() failed: channel(%d) rc(%d)", q_id, rc);
        ret = RDPA_DRV_PORT_GET;
        goto get_xtm_q_cfg_exit;
    }

    if ((rc = rdpa_egress_tm_queue_cfg_get(sched, 0, &q_cfg))) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_queue_cfg_get() failed: q_index(0), rc(%d)", rc);
        ret = RDPA_DRV_Q_CFG_GET;
        goto get_xtm_q_cfg_exit;
    }

    rdpa_egress_tm_index_get(sched, &tm_idx);

    /* get tm mode */
    if ((rc = rdpa_egress_tm_mode_get(sched, &tm_mode))) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_mode_get() failed: rc(%d)", rc);
        ret = RDPA_DRV_MODE_GET;
        goto get_xtm_q_cfg_exit;
    }

    if (tm_mode == rdpa_tm_sched_disabled) {
        /* If weight is 0, the queue is SP. Get the queue shaping rate */
        if (q_cfg.weight == 0) {
            /* get the shaping rate */
            if ((rc = rdpa_egress_tm_rl_get(sched, &rl_cfg))) {
                CMD_TM_LOG_ERROR("rdpa_egress_tm_rl_get() failed: tm(%lld) rc(%d)", tm_idx, rc);
                ret = RDPA_DRV_TM_CFG_GET;
                goto get_xtm_q_cfg_exit;
            }
            q_info->shaping_rate = rl_cfg.af_rate / 1000 ;
        }
    }
    q_info->tm_id = tm_idx;
    q_info->index = 0;
    q_info->qsize = q_cfg.drop_threshold;
    q_info->weight = q_cfg.weight;
    q_info->minBufs = q_cfg.reserved_packet_buffers;
    q_info->found = TRUE;
    q_info->drop_alg = q_cfg.drop_alg;
    q_info->red_min_thr_lo = getUsrQueueSize(q_cfg.low_class.min_threshold);
    q_info->red_max_thr_lo = getUsrQueueSize(q_cfg.low_class.max_threshold);
    q_info->red_min_thr_hi = getUsrQueueSize(q_cfg.high_class.min_threshold);
    q_info->red_max_thr_hi = getUsrQueueSize(q_cfg.high_class.max_threshold);
    q_info->priority_mask_0 = q_cfg.priority_mask_0;
    q_info->priority_mask_1 = q_cfg.priority_mask_1;
    q_info->red_drop_percent_lo = RDPA_WRED_MAX_DROP_PROBABILITY;
    q_info->red_drop_percent_hi = RDPA_WRED_MAX_DROP_PROBABILITY;

get_xtm_q_cfg_exit:

    bdmf_put(xtm_obj);
    return ret;
}
#endif /* defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158) */


static int get_q_cfg(
    uint32_t dev_type,
    uint32_t dev_id,
    uint32_t q_id,
    uint32_t dir,
    rdpa_drv_ioctl_tm_t *q_info)
{
    int ret = RDPA_DRV_SUCCESS;

    CMD_TM_LOG_DEBUG("IN: dev_id(%u) q_id(%u)", dev_id, q_id);

    switch (dev_type)
    {
    case RDPA_IOCTL_DEV_PORT:
    case RDPA_IOCTL_DEV_TCONT:
    case RDPA_IOCTL_DEV_LLID:
    case RDPA_IOCTL_DEV_NONE:
        ret = get_virt_port_q_cfg(dev_type, dev_id, q_id, dir, q_info);
        break;
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158)
    case RDPA_IOCTL_DEV_XTM:
        ret = get_xtm_channel_q_cfg(dev_type, dev_id, q_id, q_info);
        break;
#endif /* defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158) */
    default:
        CMD_TM_LOG_ERROR("Illegal dev_type (%d)", dev_type);
        ret = RDPA_DRV_ERROR;
        break;
    }

    return ret;
}


static int get_q_stats(
    uint32_t dev_type,
    uint32_t dev_id,
    uint32_t q_id,
    uint32_t dir,
    rdpa_stat_1way_t *pstats)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    bdmf_object_handle        sched = NULL;
    bdmf_object_handle    sub_sched = NULL;
    bdmf_object_handle handle_sched = NULL;
    rdpa_egress_tm_key_t  tm_key = {};
    rdpa_tm_queue_index_t q_index = {};
    port_st      *pport;
    queue_st *pq;
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158)
    bdmf_object_handle xtm_obj = NULL;
#endif
    int ret = RDPA_DRV_SUCCESS;
    BOOL need_sub_tm = FALSE;

    CMD_TM_LOG_DEBUG("IN: dev_type(%u) dev_id(%u) q_id(%u)", dev_type, dev_id, q_id);

    /* initialize return values */
    memset(pstats, 0, sizeof(rdpa_stat_1way_t));

    if (q_id >= MAX_Q_PER_TM) {
        CMD_TM_LOG_ERROR("FAIL: Invalid q_id(%u)", q_id);
        return RDPA_DRV_Q_ID_NOT_VALID;
    }

    if (dev_type == RDPA_IOCTL_DEV_PORT) {
        pport = get_virt_port(dev_type, dev_id);
        if (pport == NULL) {
            CMD_TM_LOG_ERROR("Invalid devtype(%u) dev_id(%u)", dev_type, dev_id);
            return RDPA_DRV_PORT_ID_NOT_VALID;
        }

        if (!pport->alloc) {
            /* Port tm has not been configured.
             * Return as SUCCESS.
             */
            return RDPA_DRV_SUCCESS;
        }

        pq = &(pport->queue_list[q_id]);
        if (pq->tm_id == INVALID_ID) {
            /* Runner queue has not been configured.
             * Return as SUCCESS.
             */
            return RDPA_DRV_SUCCESS;
        }

        tm_key.dir    = dir;
        /* use the root tm index.
         * queue stats are returned only by querring the queue's root tm.
         */
        tm_key.index = pport->root_tm_id;

        if (dir == rdpa_dir_us) {
#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM963158)
            /* tm is assigned in cascade way
             * root --> sub (sp) << stats only work on this sub tm
             *     | --> sub (wrr)
             * need to get sub tm to continue the process
             */
            need_sub_tm = TRUE;
#endif
        }
        else {
            /* ethLan channel id is its emac id.
             * lan0 channel id = BL_LILAC_RDD_EMAC_ID_0 (1) = rdpa_if_lan0 - rdpa_if_wan_max
             * lan1 channel id = BL_LILAC_RDD_EMAC_ID_1 (2) = rdpa_if_lan1 - rdpa_if_wan_max
             * etc....
             */
            q_index.channel = dev_id - rdpa_if_wan_max;
        }

        /* Get the root tm object of the queue */
        if ((rc = rdpa_egress_tm_get(&tm_key, &sched))) {
            CMD_TM_LOG_ERROR("rdpa_egress_tm_get() failed: tm(%d) rc(%d)", (int)tm_key.index, rc);
            return RDPA_DRV_TM_GET;
        }
    }
#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM963158)
    else if (dev_type == RDPA_IOCTL_DEV_LLID
        || dev_type == RDPA_IOCTL_DEV_TCONT) {
        ret = get_q_from_virt_port(dev_type, dev_id, q_id, &pq);
        if (ret != RDPA_DRV_SUCCESS) {
            if ((ret == RDPA_DRV_PORT_NOT_ALLOC) || (ret == RDPA_DRV_TM_NOT_ALLOC)) {
                ret = RDPA_DRV_SUCCESS;
            }
            return ret;
        }
        /* Runner queue has been configured for this qid. Get queue config. */
        tm_key.dir   = dir ;
        tm_key.index = pq->tm_id;

        /* Get the root tm object of the queue */
        if ((rc = rdpa_egress_tm_get(&tm_key, &sched))) {
            CMD_TM_LOG_ERROR("rdpa_egress_tm_get() failed: tm(%d) rc(%d)", (int)tm_key.index, rc);
            return RDPA_DRV_TM_GET;
        }
    }
#endif
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158)
    else if (dev_type == RDPA_IOCTL_DEV_XTM) {
        if ((rc = rdpa_xtmchannel_get(q_id, &xtm_obj)) || (xtm_obj == NULL)) {
            CMD_TM_LOG_ERROR("rdpa_xtmchannel_get() failed: channel(%d) rc(%d)", q_id, rc);
            return RDPA_DRV_PORT_GET;
        }

        if ((rc = rdpa_xtmchannel_egress_tm_get(xtm_obj, &sched))) {
            CMD_TM_LOG_ERROR("rdpa_xtmchannel_egress_tm_get() failed: channel(%d) rc(%d)", q_id, rc);
            bdmf_put(xtm_obj);
            return RDPA_DRV_PORT_GET;
        }
        bdmf_get(sched);
    }
#endif
    else {
        CMD_TM_LOG_ERROR("not implement for such dev type(%u)", dev_type);
        return RDPA_DRV_ERROR;
    }

    handle_sched = sched;
    if (need_sub_tm) {
        if(rdpa_egress_tm_subsidiary_get(sched, 0, &sub_sched) == 0) {
            handle_sched = sub_sched;
        }
    }

    q_index.queue_id = q_id;
    
    /* get queue statistics */
    if ((rc = rdpa_egress_tm_queue_stat_get(handle_sched, &q_index, pstats))) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_queue_stat_get() failed: rc(%d)", rc);
        ret = RDPA_DRV_QUEUE_STATS_GET;
    }

    bdmf_put(sched);
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158)
    if (xtm_obj)
        bdmf_put(xtm_obj);
#endif

    return ret;
}

#ifdef __DUMP_PORTS__
static int dump_ports(void)
{
    port_st    *pport = NULL;
    int i;

    for (i = 0; i < MAX_VIRT_TM_PORTS; i++) {
        pport = &port_list[i];
        CMD_TM_LOG_DEBUG("PORT(%u) STAT: alloc(%u) queue_cnt(%u) root_id(%u)",
                         i, pport->alloc, pport->queue_alloc_cnt, pport->root_tm_id);
    }

    return 0;
}
#endif /* __DUMP_PORTS__ */


static int tm_set(
    bdmf_mattr_handle sched_attrs,
    rdpa_drv_ioctl_tm_t *pTm,
    uint32_t *ptm_id,
    bdmf_object_handle root_sched,
    bdmf_object_handle *psched)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    bdmf_number new_tm_id;
    bdmf_object_handle tmp_psched = NULL;
    rdpa_tm_service_queue_t  service_queue = {.enable = 0};
    BOOL useSingleTm = FALSE;

    CMD_TM_LOG_DEBUG("IN: ROOT_tm(%u) dir(%u) dev_type(%u) dev_id(%u) arbiter mode(%u) level(%u)",
                     pTm->root_tm_id, pTm->dir, pTm->dev_type, pTm->dev_id, pTm->arbiter_mode, pTm->level);

    rdpa_egress_tm_dir_set(sched_attrs, pTm->dir);
    rdpa_egress_tm_level_set(sched_attrs, pTm->level); /* rdpa_tm_level_queue / rdpa_tm_level_egress_tm */
    rdpa_egress_tm_mode_set(sched_attrs, pTm->arbiter_mode);
    if(pTm->service_queue)
        service_queue.enable = 1;
    rdpa_egress_tm_service_queue_set(sched_attrs, &service_queue);

    if (pTm->dev_type == RDPA_IOCTL_DEV_PORT &&
        pTm->dir == rdpa_dir_us &&
        pTm->level == rdpa_tm_level_egress_tm &&
        pTm->arbiter_mode == rdpa_tm_sched_sp &&
        !pTm->service_queue) {
        rdpa_egress_tm_rl_rate_mode_set(sched_attrs, pTm->rl_mode);
    }

    if (pTm->dev_type == RDPA_IOCTL_DEV_PORT)
        useSingleTm = ((get_port_tm_caps(pTm->dir) & RDPA_TM_1LEVEL_CAPABLE) == RDPA_TM_1LEVEL_CAPABLE);
    else if (pTm->dev_type == RDPA_IOCTL_DEV_TCONT)
        useSingleTm = ((get_tcont_tm_caps() & RDPA_TM_1LEVEL_CAPABLE) == RDPA_TM_1LEVEL_CAPABLE);
    else if (pTm->dev_type == RDPA_IOCTL_DEV_LLID)
        useSingleTm = ((get_llid_tm_caps() & RDPA_TM_1LEVEL_CAPABLE) == RDPA_TM_1LEVEL_CAPABLE);
    else
        useSingleTm = FALSE;

    
    if (useSingleTm &&
        pTm->arbiter_mode == rdpa_tm_sched_sp_wrr &&
        pTm->level == rdpa_tm_level_queue &&
        pTm->dir == rdpa_dir_us) {
        rdpa_egress_tm_num_queues_set(sched_attrs, DFT_SINGLE_TM_Q_NUM);
        rdpa_egress_tm_num_sp_elements_set(sched_attrs, DFT_SINGLE_TM_SP_Q_NUM);
        }

    if ((rc = bdmf_new_and_set(rdpa_egress_tm_drv(), root_sched, sched_attrs, &tmp_psched)) ||
        (tmp_psched == NULL)) {
        CMD_TM_LOG_ERROR("bdmf_new_and_set() failed: ROOT_tm(%u) rc(%d)", pTm->root_tm_id, rc);
        return RDPA_DRV_NEW_TM_ALLOC;
    }

    *psched = tmp_psched;

    if ((rc = rdpa_egress_tm_index_get(*psched, &new_tm_id))) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_index_get() failed: ROOT_tm(%u) rc(%d)", pTm->root_tm_id, rc);
        return RDPA_DRV_TM_INDEX_GET;
    }

    *ptm_id = (uint32_t) new_tm_id;

    return 0;
}

static int orl_config(
    rdpa_drv_ioctl_tm_t *ptm)
{
    BDMF_MATTR(sched_attrs, rdpa_egress_tm_drv());
    bdmf_error_t       rc         = BDMF_ERR_OK;
    bdmf_object_handle system_obj = NULL;
    bdmf_object_handle sched      = NULL;
    bdmf_number        new_tm_id;
    uint32_t orl_id       = INVALID_ID;
    uint32_t shaping_rate = 0;
    BOOL     orl_linked   = FALSE;
    port_st  *pport;
    int ret = 0;

    CMD_TM_LOG_DEBUG("orl_config: dev_id(%u) dir(%u)", ptm->dev_id, ptm->dir);

    pport = get_virt_port(ptm->dev_type, ptm->dev_id);
    if (pport == NULL) {
        CMD_TM_LOG_ERROR("Invalid devtype(%u) dev_id(%u)",
                         ptm->dev_type, ptm->dev_id);
        return RDPA_DRV_PORT_GET;
    }

    if ((ret = get_orl(ptm->dev_type, ptm->dev_id, ptm->dir,
                       &orl_id, &shaping_rate, &orl_linked))) {
        CMD_TM_LOG_ERROR("get_orl() failed: port_id(%u) rc(%d)", ptm->dev_id, ret);
        return RDPA_DRV_TM_GET;
    }
    if (orl_id != INVALID_ID)
    {
        CMD_TM_LOG_ERROR("port_id(%u) already has an orl(%u)", ptm->dev_id, orl_id);
        return RDPA_DRV_TM_GET;
    }

    if ((rc = rdpa_system_get(&system_obj)) || (system_obj == NULL)) {
        CMD_TM_LOG_ERROR("rdpa_system_get() failed: rc(%d)", rc);
        return RDPA_DRV_PORT_GET;
    }

    rdpa_egress_tm_dir_set(sched_attrs, ptm->dir);
    if (ptm->dir == rdpa_dir_us) {
        rdpa_egress_tm_level_set(sched_attrs, rdpa_tm_level_egress_tm);
    }
    else {
        rdpa_egress_tm_level_set(sched_attrs, rdpa_tm_level_queue);
    }

    rdpa_egress_tm_mode_set(sched_attrs, rdpa_tm_sched_disabled);
    rdpa_egress_tm_overall_rl_set(sched_attrs, TRUE);

    if ((rc = bdmf_new_and_set(rdpa_egress_tm_drv(), system_obj, sched_attrs, &sched)) ||
        (sched == NULL)) {
        CMD_TM_LOG_ERROR("bdmf_new_and_set() failed: port_id(%u) rc(%d)", ptm->dev_id, rc);
        bdmf_put(system_obj);
        return RDPA_DRV_NEW_TM_ALLOC;
    }

    bdmf_put(system_obj);

    if ((rc = rdpa_egress_tm_index_get(sched, &new_tm_id))) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_index_get() failed: port_id(%u) rc(%d)", ptm->dev_id, rc);
        ret = RDPA_DRV_TM_INDEX_GET;

        /* Error! destroy the orl just created */
        if ((rc = bdmf_destroy(sched))) {
            CMD_TM_LOG_ERROR("bdmf_destroy() failed: orl_tm(%u) rc(%d)", ptm->tm_id, rc);
        }
    }
    else {
        ptm->tm_id = (uint32_t)new_tm_id;
        pport->orl_id = ptm->tm_id;
    }

    return ret;
}

static int orl_remove(
    uint32_t dev_type,
    uint32_t port_id,
    uint32_t dir)
{
    bdmf_error_t         rc     = BDMF_ERR_OK;
    bdmf_object_handle   sched  = NULL;
    rdpa_egress_tm_key_t tm_key = {};
    uint32_t orl_id     = INVALID_ID;
    BOOL     orl_linked = FALSE;
    port_st  *pport     = NULL;
    int ret = 0;

    CMD_TM_LOG_DEBUG("orl_remove: dev_type(%u) port_id(%u) dir(%u)",
                     dev_type, port_id, dir);

    pport = get_virt_port(dev_type, port_id);
    if (pport == NULL) {
        CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)", dev_type, port_id);
        return RDPA_DRV_PORT_ID_NOT_VALID;
    }

    orl_id     = pport->orl_id;
    orl_linked = pport->orl_linked;

    if (orl_id == INVALID_ID)
    {
        CMD_TM_LOG_DEBUG("port_id(%u) does not have an orl.", port_id);
        return 0;   /* do nothing */
    }

    tm_key.dir   = dir;
    tm_key.index = orl_id;

    if ((rc = rdpa_egress_tm_get(&tm_key, &sched)) || (sched == NULL)) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_get() failed: orl_tm(%u) rc(%d)",
                         orl_id, rc);
        return RDPA_DRV_TM_GET;
    }

    /* If the orl is linked to the port, unlink it first. */
    if (orl_linked) {
        bdmf_object_handle port_obj = NULL;

        if ((rc = rdpa_port_get(port_id, &port_obj)) || (port_obj == NULL)) {
            CMD_TM_LOG_ERROR("rdpa_port_get() failed: port(%d) rc(%d)", port_id, rc);
            ret = RDPA_DRV_PORT_GET;
        }
        else {
            if ((rc = bdmf_unlink(port_obj, sched))) {
                CMD_TM_LOG_ERROR("bdmf_unlink() failed: port(%u) orl_tm(%u) rc(%d)",
                                 port_id, orl_id, rc);
                ret = RDPA_DRV_ORL_UNLINK;
            }
            else {
                pport->orl_linked = FALSE;
            }
            bdmf_put(port_obj);
        }
    }

    bdmf_put(sched);

    if (!ret && !pport->orl_linked) {
        if ((rc = bdmf_destroy(sched))) {
            CMD_TM_LOG_ERROR("bdmf_destroy() failed: orl_tm(%u) rc(%d)", orl_id, rc);
            ret = RDPA_DRV_SH_DESTROY;
        }
        else {
            pport->orl_id = INVALID_ID;
        }
    }

    return ret;
}

static int orl_link(
    rdpa_drv_ioctl_tm_t *ptm)
{
    bdmf_error_t         rc     = BDMF_ERR_OK;
    bdmf_object_handle   sched  = NULL;
    rdpa_egress_tm_key_t tm_key = {};
    rdpa_tm_rl_cfg_t     rl_cfg = {};
    uint32_t orl_id       = INVALID_ID;
    uint32_t shaping_rate = 0;
    BOOL     orl_linked   = FALSE;
    port_st  *pport;
    int ret = 0;

    rl_cfg.af_rate = ptm->shaping_rate * 1000;      /* Best Effort: shaping_rate is in kbit/s: 1 kilobit = 1000 bits */

    CMD_TM_LOG_DEBUG("orl_link: port(%u) orl_tm(%u) af(%llu)",
                     ptm->dev_id, ptm->tm_id, (uint64_t)rl_cfg.af_rate);

    pport = get_virt_port(ptm->dev_type, ptm->dev_id);
    if (pport == NULL) {
        CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)",
                         ptm->dev_type, ptm->dev_id);
        return RDPA_DRV_ORL_LINK;
    }

    if ((ret = get_orl(ptm->dev_type, ptm->dev_id, ptm->dir,
                       &orl_id, &shaping_rate, &orl_linked))) {
        CMD_TM_LOG_ERROR("get_orl() failed: port_id(%u) rc(%d)", ptm->dev_id, ret);
        return RDPA_DRV_ORL_LINK;
    }
    if (orl_id != ptm->tm_id)
    {
        CMD_TM_LOG_ERROR("orl(%u) was not allocated for port_id(%u)", ptm->tm_id, ptm->dev_id);
        return RDPA_DRV_ORL_LINK;
    }

    tm_key.dir       = ptm->dir;
    tm_key.index = ptm->tm_id;

    if ((rc = rdpa_egress_tm_get(&tm_key, &sched)) || (sched == NULL)) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_get() failed: orl_tm(%u) rc(%d)", ptm->tm_id, rc);
        return RDPA_DRV_TM_GET;
    }

    if ((rc = rdpa_egress_tm_rl_set(sched, &rl_cfg))) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_rl_set() failed: orl_tm(%u) af(%llu) rc(%d)",
                         ptm->tm_id, (uint64_t)rl_cfg.af_rate, rc);
        ret = RDPA_DRV_TM_CFG_SET;
    }
    else if (!orl_linked) {
        bdmf_object_handle   port_obj = NULL;

        if ((rc = rdpa_port_get(ptm->dev_id, &port_obj)) || (port_obj == NULL)) {
            CMD_TM_LOG_ERROR("rdpa_port_get() failed: port(%d) rc(%d)", ptm->dev_id, rc);
            ret = RDPA_DRV_PORT_GET;
        }
        else {
            if ((rc = bdmf_link(port_obj, sched, NULL))) {
                CMD_TM_LOG_ERROR("bdmf_link() failed: port(%u) orl_tm(%u) rc(%d)",
                                 ptm->dev_id, ptm->tm_id, rc);
                ret = RDPA_DRV_ORL_LINK;
            }
            else {
                pport->orl_linked = TRUE;
            }
            bdmf_put(port_obj);
        }
    }

    bdmf_put(sched);

    return ret;
}

static int orl_unlink(
    rdpa_drv_ioctl_tm_t *ptm)
{
    bdmf_error_t         rc     = BDMF_ERR_OK;
    bdmf_object_handle   sched  = NULL;
    rdpa_egress_tm_key_t tm_key = {};
    uint32_t orl_id       = INVALID_ID;
    uint32_t shaping_rate = 0;
    BOOL     orl_linked   = FALSE;
    port_st  *pport;
    int ret = 0;

    CMD_TM_LOG_DEBUG("orl_unlink: port(%u) orl_tm(%u)", ptm->dev_id, ptm->tm_id);

    pport = get_virt_port(ptm->dev_type, ptm->dev_id);
    if (pport == NULL) {
        CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)",
                        ptm->dev_type, ptm->dev_id);
        return RDPA_DRV_ORL_UNLINK;
    }

    if ((ret = get_orl(ptm->dev_type, ptm->dev_id, ptm->dir,
                       &orl_id, &shaping_rate, &orl_linked))) {
        CMD_TM_LOG_ERROR("get_orl() failed: port_id(%u) rc(%d)", ptm->dev_id, ret);
        return RDPA_DRV_ORL_UNLINK;
    }
    if (orl_id != ptm->tm_id)
    {
        CMD_TM_LOG_ERROR("orl(%u) was not allocated for port_id(%u)", ptm->tm_id, ptm->dev_id);
        return RDPA_DRV_ORL_UNLINK;
    }

    tm_key.dir       = ptm->dir;
    tm_key.index = ptm->tm_id;

    if ((rc = rdpa_egress_tm_get(&tm_key, &sched)) || (sched == NULL)) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_get() failed: orl_tm(%u) rc(%d)", ptm->tm_id, rc);
        return RDPA_DRV_TM_GET;
    }

    if (orl_linked) {
        bdmf_object_handle   port_obj = NULL;

        if ((rc = rdpa_port_get(ptm->dev_id, &port_obj))) {
            CMD_TM_LOG_ERROR("rdpa_port_get() failed: port(%d) rc(%d)", ptm->dev_id, rc);
            ret = RDPA_DRV_PORT_GET;
        }
        else {
            if ((rc = bdmf_unlink(port_obj, sched))) {
                CMD_TM_LOG_ERROR("bdmf_unlink() failed: port(%u) orl_tm(%u) rc(%d)",
                                 ptm->dev_id, ptm->tm_id, rc);
                ret = RDPA_DRV_ORL_UNLINK;
            }
            else {
                pport->orl_linked = FALSE;
            }
            bdmf_put(port_obj);
        }
    }

    if (!ret && !pport->orl_linked)
    {
        rdpa_tm_rl_cfg_t  rl_cfg = {};

        /* clear af rate when orl tm is unlinked from a port. */
        rl_cfg.af_rate = 0;

        if ((rc = rdpa_egress_tm_rl_set(sched, &rl_cfg))) {
            CMD_TM_LOG_ERROR("rdpa_egress_tm_rl_set() failed: orl_tm(%u) af(%llu) rc(%d)",
                             ptm->tm_id, (uint64_t)rl_cfg.af_rate, rc);
//                 ret = RDPA_DRV_TM_CFG_SET;
            /* don't need to return as error since the orl is already unlinke. */
        }
    }

    bdmf_put(sched);

    return ret;
}


static int remove_all_queues(uint32_t dev_type, uint32_t dev_id, uint32_t root_tm_id, bdmf_object_handle root_sched)
{
    port_st *pport = NULL;
    queue_st *pq = NULL;
    int qid = 0;
    rdpa_tm_queue_cfg_t q_cfg;
    int rc = 0;
    int ret = 0;
    
    pport = get_virt_port(dev_type, dev_id);
    for (qid = 0; qid < MAX_Q_PER_TM; qid++)
    {
        pq = &(pport->queue_list[qid]);
        if (pq->tm_id == root_tm_id) 
        {
            if (pq->alloc)
            {
                if ((rc = rdpa_egress_tm_queue_cfg_get(root_sched, pq->q_index, &q_cfg)))
                {
                    CMD_TM_LOG_ERROR("rdpa_egress_tm_queue_cfg_get() failed: q_index(%d), rc(%d)",
                                     pq->q_index, rc);
                    ret = RDPA_DRV_Q_CFG_GET;
                    goto rm_all_q_exit;
                }

                q_cfg.drop_threshold = 0;

                if ((rc = rdpa_egress_tm_queue_cfg_set(root_sched, pq->q_index, &q_cfg)))
                {
                    CMD_TM_LOG_ERROR("rdpa_egress_tm_queue_cfg_set() failed: q_index(%d) rc(%d)",
                                      pq->q_index, rc);
                    ret = RDPA_DRV_Q_CFG_SET;
                    goto rm_all_q_exit;
                }

                if ((ret = remove_q_from_virt_port(dev_type, dev_id, pq->qid)))
                    CMD_TM_LOG_ERROR("remove_q_from_virt_port() failed: "
                                     "port_id(%u) q_id(%lld) rc(%d)",
                                     dev_id, pq->qid, ret);
            }       
        }
            
    }

rm_all_q_exit:    
    return ret;
}


static int root_tm_remove(
    uint32_t dev_type,
    uint32_t dev_id,
    uint32_t root_tm_id,
    uint32_t dir)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    bdmf_object_handle sched = NULL;
    int ret = 0;

    CMD_TM_LOG_DEBUG("IN: dev_type(%u) dev_id(%u) root_tm_id(%u) dir(%u)", dev_type, dev_id, root_tm_id, dir);

    switch (dev_type)
    {
    case RDPA_IOCTL_DEV_PORT:
    case RDPA_IOCTL_DEV_NONE:
    {
        rdpa_egress_tm_key_t tm_key = {};
        port_st *pport;

#ifdef __DUMP_PORTS__
        dump_ports();
#endif
        pport = get_virt_port(dev_type, dev_id);
        if (pport == NULL) {
            CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)", dev_type, dev_id);
            ret = RDPA_DRV_PORT_ID_NOT_VALID;
            goto root_tm_rm_exit;
        }

        tm_key.dir   = dir;
        tm_key.index = root_tm_id;

        if ((rc = rdpa_egress_tm_get(&tm_key, &sched))) {
            CMD_TM_LOG_ERROR("rdpa_egress_tm_get() failed: tm(%u) rc(%d)", root_tm_id, rc);
            ret = RDPA_DRV_TM_GET;
            goto root_tm_rm_exit;
        }
        if (sched) {
            bdmf_put(sched);            
            if (share_root_tm && (dir == rdpa_dir_us))
            {
                /* some epon case share control and data tm, so cannot destroy data tm as others here */ 
                ret = remove_all_queues(dev_type, dev_id, root_tm_id, sched);
                goto root_tm_rm_exit;
            }
            else if ((rc = bdmf_destroy(sched))) {
                CMD_TM_LOG_ERROR("bdmf_destroy() failed: tm(%u) rc(%d)", root_tm_id, rc);
                ret = RDPA_DRV_SH_DESTROY;
                goto root_tm_rm_exit;
            }
        }

        if (pport->root_tm_id == root_tm_id) {
            /* If an orl is linked to the port, remove it. */
            orl_remove(dev_type, dev_id, dir);
            init_portlist_entry(pport);
        }
        /* else, something wrong. */

        break;
    }
    case RDPA_IOCTL_DEV_LLID:
    {
        bdmf_object_handle llid = NULL;
        port_st *pport;

        pport = get_virt_port(dev_type, dev_id);
        if (pport == NULL) {
            CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)",
                             dev_type, dev_id);
            ret = RDPA_DRV_PORT_ID_NOT_VALID;
            goto root_tm_rm_exit;
        }
        else
        {
            if (pport->alloc == FALSE)
                goto root_tm_rm_exit;
        }
        
        if ((rc = rdpa_llid_get(dev_id, &llid)))
        {
            CMD_TM_LOG_ERROR("rdpa_llid_get() failed: llid(%u) rc(%d)", dev_id, rc);
            ret = RDPA_DRV_LLID_GET;
            goto root_tm_rm_exit;
        }

       if ((rc = rdpa_llid_egress_tm_get(llid, &sched)))
        {
            CMD_TM_LOG_ERROR("rdpa_llid_egress_tm_get() failed: llid(%u) rc(%d)", dev_id, rc);
            bdmf_put(llid);
            ret = RDPA_DRV_LLID_TM_GET;
            goto root_tm_rm_exit;
        }
        if (sched)
        {
            if (share_root_tm)
            {
                /* some epon case share control and data tm, so cannot destroy data tm as others here */ 
                ret = remove_all_queues(dev_type, dev_id, root_tm_id, sched);
            }
            else
            {
                if ((rc = rdpa_llid_egress_tm_set(llid, NULL)))
                {
                    CMD_TM_LOG_ERROR("rdpa_llid_egress_tm_set() failed: llid(%u) rc(%d)", dev_id, rc);
                    bdmf_put(llid);
                    ret = RDPA_DRV_LLID_TM_SET;
                    goto root_tm_rm_exit;
                }
                
                if ((rc = bdmf_destroy(sched)))
                {
                    bdmf_put(llid);
                    CMD_TM_LOG_ERROR("bdmf_destroy() failed: rc(%d)", rc);
                    ret = RDPA_DRV_SH_DESTROY;
                    goto root_tm_rm_exit;
                }
            }
        }
        bdmf_put(llid);

        if (pport->root_tm_id == root_tm_id) {
            orl_remove(dev_type, dev_id, dir);
            init_portlist_entry(pport);
        }

        break;
    }
    case RDPA_IOCTL_DEV_TCONT:
    {
        port_st *pport = NULL;
        bdmf_object_handle tcont = NULL;

        pport = get_virt_port(dev_type, dev_id);
        if (pport == NULL) {
            CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)",
                             dev_type, dev_id);
            ret = RDPA_DRV_PORT_ID_NOT_VALID;
            goto root_tm_rm_exit;
        }

        if ((rc = rdpa_tcont_get(dev_id, &tcont))) {
            CMD_TM_LOG_ERROR("rdpa_tcont_get() failed: tcont(%u) rc(%d)",
                             dev_id, rc);
            ret = RDPA_DRV_TCONT_GET;
            goto root_tm_rm_exit;
        }
        if ((rc = rdpa_tcont_egress_tm_get(tcont, &sched))) {
            CMD_TM_LOG_ERROR("rdpa_tcont_egress_tm_get() failed: tcont(%u) rc(%d)",
                             dev_id, rc);
            bdmf_put(tcont);
            ret = RDPA_DRV_TCONT_TM_GET;
            goto root_tm_rm_exit;
        }
        if (sched) {
            if ((rc = rdpa_tcont_egress_tm_set(tcont, NULL))) {
                CMD_TM_LOG_ERROR("rdpa_tcont_egress_tm_set() failed: tcont(%u) rc(%d)",
                                 dev_id, rc);
                bdmf_put(tcont);
                ret = RDPA_DRV_TCONT_TM_SET;
                goto root_tm_rm_exit;
            }

            if ((rc = bdmf_destroy(sched))) {
                bdmf_put(tcont);
                CMD_TM_LOG_ERROR("bdmf_destroy() failed: rc(%d)", rc);
                ret = RDPA_DRV_SH_DESTROY;
                goto root_tm_rm_exit;
            }
        }

        bdmf_put(tcont);

        if (pport->root_tm_id == root_tm_id) {
            orl_remove(dev_type, dev_id, dir);
            init_portlist_entry(pport);
        }
        break;
    }
    default:
        ret = RDPA_DRV_ERROR;
        break;

    }

root_tm_rm_exit:
    return ret;
}

static int root_tm_config(
    rdpa_drv_ioctl_tm_t *ptm,
    bdmf_object_handle *psched)
{
    BDMF_MATTR(sched_attrs, rdpa_egress_tm_drv());
    bdmf_object_handle dev_obj = NULL;
    bdmf_object_handle tmp_psched = NULL;
    bdmf_error_t rc = BDMF_ERR_OK;
    rdpa_if _rdpa_if = rdpa_if_none;
    int ret = 0;

    _rdpa_if = convert_dev_type_id_to_if(ptm->dev_type, ptm->dev_id);

    CMD_TM_LOG_DEBUG("IN: dev_type(%u) dev_id(%u) = rdpa_if(%d)", ptm->dev_type, ptm->dev_id, _rdpa_if);

    switch (ptm->dev_type)
    {
    case RDPA_IOCTL_DEV_PORT:
    {
        rdpa_wan_type wan_type = rdpa_wan_none;

        /* ------------------- PORT Configuration: Acquire the port object ------------------- */
        if ((rc = rdpa_port_get(ptm->dev_id, &dev_obj))) {
            CMD_TM_LOG_ERROR("rdpa_port_get() failed: port(%d) rc(%d)", ptm->dev_id, rc);
            ret = RDPA_DRV_PORT_GET;
            goto root_tm_cfg_exit;
        }

        if (share_root_tm && (ptm->dir == rdpa_dir_us))
            goto root_tm_cfg_exit;
        
        if ((ret = tm_set(sched_attrs, ptm, &ptm->root_tm_id, dev_obj, &tmp_psched))) {
            CMD_TM_LOG_ERROR("tm_set() failed: ret(%d)", ret);
            goto root_tm_cfg_exit;
        }


        CMD_TM_LOG_DEBUG("ADDING ROOT TM: ROOT_tm(%u) port_id(%d)", ptm->root_tm_id, ptm->dev_id);

        *psched = tmp_psched;
        if ((ret = add_root_tm_to_virt_port(ptm->dev_type, ptm->dev_id, ptm->root_tm_id,
                                            ptm->arbiter_mode, ptm->cfg_flags))) {
            CMD_TM_LOG_ERROR("add_root_tm_to_virt_port() failed: ROOT_tm(%u) ret(%d)",
                             ptm->root_tm_id, ret);
            goto root_tm_cfg_exit;
        }
        

        if (!get_rdpa_wan_type(_rdpa_if, &wan_type) && 
            (wan_type == rdpa_wan_epon || wan_type == rdpa_wan_xepon) && !is_ae_enable(_rdpa_if))

            if ((get_port_tm_caps(rdpa_dir_us) & RDPA_TM_1LEVEL_CAPABLE) != 0)
                share_root_tm = TRUE;

        break;
    }
    case RDPA_IOCTL_DEV_LLID:
    {
        if ((rc = rdpa_llid_get(ptm->dev_id, &dev_obj))) {
            CMD_TM_LOG_ERROR("rdpa_llid_get() failed: llid(%u) rc(%d)", ptm->dev_id, rc);
            ret = RDPA_DRV_LLID_GET;
            goto root_tm_cfg_exit;
        }

        rc = rdpa_llid_egress_tm_get(dev_obj, &tmp_psched);

        if (rc)
        {
            CMD_TM_LOG_ERROR("rdpa_llid_egress_tm_get() failed: llid(%u) rc(%d)", ptm->dev_id, rc);
            ret = RDPA_DRV_LLID_TM_GET;
            goto root_tm_cfg_exit;
        }
            
        if (tmp_psched)
        {
            bdmf_number tm_index;
            
            share_root_tm = TRUE;
            
            if ((rc = rdpa_egress_tm_index_get(tmp_psched, &tm_index)))
            {
                CMD_TM_LOG_ERROR("rdpa_egress_tm_index_get() failed: rc(%d)", rc);
                ret = RDPA_DRV_TM_INDEX_GET;
                goto root_tm_cfg_exit;
            }
            
            ptm->root_tm_id = tm_index;
        }
        else
        {
            if ((ret = tm_set(sched_attrs, ptm, &ptm->root_tm_id, NULL, &tmp_psched))) {
                CMD_TM_LOG_ERROR("tm_set() failed: ret(%d)", ret);
                goto root_tm_cfg_exit;
            }

            if ((rc = rdpa_llid_egress_tm_set(dev_obj, tmp_psched))) {
                CMD_TM_LOG_ERROR("rdpa_llid_egress_tm_set() failed: rc(%d)", rc);
                ret = RDPA_DRV_LLID_TM_SET;
                if ((rc = bdmf_destroy(tmp_psched)))
                    CMD_TM_LOG_ERROR("bdmf_destroy() failed: rc(%d)", rc);

                goto root_tm_cfg_exit;
            }
        }
        
        *psched = tmp_psched;

        if ((ret = add_root_tm_to_virt_port(ptm->dev_type, ptm->dev_id, ptm->root_tm_id,
                                            ptm->arbiter_mode, ptm->cfg_flags))) {
            CMD_TM_LOG_ERROR("add_root_tm_to_virt_port() failed: ROOT_tm(%u) ret(%d)",
                             ptm->root_tm_id, ret);
            goto root_tm_cfg_exit;
        }

        break;
    }
    case RDPA_IOCTL_DEV_TCONT:
    {
        if ((rc = rdpa_tcont_get(ptm->dev_id, &dev_obj))) {
            CMD_TM_LOG_ERROR("rdpa_tcont_get() failed: tcont(%u) rc(%d)", ptm->dev_id, rc);
            ret = RDPA_DRV_TCONT_GET;
            goto root_tm_cfg_exit;
        }
        if ((ret = tm_set(sched_attrs, ptm, &ptm->root_tm_id, dev_obj, &tmp_psched))) {
            CMD_TM_LOG_ERROR("tm_set() failed: ret(%d)", ret);
            goto root_tm_cfg_exit;
        }
        if ((rc = rdpa_tcont_egress_tm_set(dev_obj, tmp_psched))) {
            CMD_TM_LOG_ERROR("rdpa_tcont_egress_tm_set() failed: rc(%d)", rc);
            ret = RDPA_DRV_TCONT_TM_SET;
            if ((rc = bdmf_destroy(tmp_psched)))
                CMD_TM_LOG_ERROR("bdmf_destroy() failed: rc(%d)", rc);

            goto root_tm_cfg_exit;
        }
        *psched = tmp_psched;
        if ((ret = add_root_tm_to_virt_port(ptm->dev_type, ptm->dev_id, ptm->root_tm_id,
                                            ptm->arbiter_mode, ptm->cfg_flags))) {
            CMD_TM_LOG_ERROR("add_root_tm_to_virt_port() failed: ROOT_tm(%u) ret(%d)",
                             ptm->root_tm_id, ret);
            goto root_tm_cfg_exit;
        }
        break;
    }
    case RDPA_IOCTL_DEV_NONE:
    {
        ptm->service_queue = 1;
        if ((ret = tm_set(sched_attrs, ptm, &ptm->root_tm_id, NULL, &tmp_psched))) {
            CMD_TM_LOG_ERROR("tm_set() failed: ret(%d)", ret);
            goto root_tm_cfg_exit;
        }

        *psched = tmp_psched;
        if (ptm->level == rdpa_tm_level_egress_tm) {
            if ((ret = add_root_tm_to_virt_port(ptm->dev_type, ptm->dev_id, ptm->root_tm_id,
                                                ptm->arbiter_mode, ptm->cfg_flags))) {
                CMD_TM_LOG_ERROR("add_root_tm_to_virt_port() failed: ROOT_tm(%u) ret(%d)",
                                 ptm->root_tm_id, ret);
                goto root_tm_cfg_exit;
            }
        }
        break;
    }
    default:
        ret = RDPA_DRV_ERROR;
        break;
    }

root_tm_cfg_exit:
    if (dev_obj)
        bdmf_put(dev_obj);

    return ret;
}

static int tm_remove(rdpa_drv_ioctl_tm_t *ptm)
{
    int ret = 0;
    bdmf_error_t rc = BDMF_ERR_OK;
    rdpa_egress_tm_key_t tm_key = {};
    bdmf_object_handle sched = NULL;
    port_st *pport = NULL;

#ifdef __DUMP_PORTS__
    dump_ports();
#endif

    pport = get_virt_port(ptm->dev_type, ptm->dev_id);
    if (pport == NULL) {
        CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)",
                          ptm->dev_type, ptm->dev_id);
        ret = RDPA_DRV_PORT_ID_NOT_VALID;
        goto tm_remove_exit;
    }

    if (ptm->tm_id == pport->root_tm_id ||
        ptm->tm_id == pport->root_sp_tm_id ||
        ptm->tm_id == pport->root_wrr_tm_id)
    {
        /* this function cannot remove any of the root TMs. Use root_tm_remove()
         * instead. root_tm_remove() will remove all the three root TMs.
         */
        CMD_TM_LOG_ERROR("FAIL: tm(%u) cannot be one of the root TMs of dev_id(%u).",
                         ptm->tm_id, ptm->dev_id);
        ret = RDPA_DRV_ERROR;
        goto tm_remove_exit;
    }

    tm_key.dir   = ptm->dir;
    tm_key.index = ptm->tm_id;

    if ((rc = rdpa_egress_tm_get(&tm_key, &sched))) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_get() failed: dir(%u) tm(%u) rc(%d)",
                         ptm->dir, ptm->tm_id, rc);
        ret = RDPA_DRV_TM_GET;
        goto tm_remove_exit;
    }

    if (sched) {
        bdmf_put(sched);
        if ((rc = bdmf_destroy(sched))) {
            CMD_TM_LOG_ERROR("bdmf_destroy() failed: dir(%u) tm(%u) rc(%d)",
                             ptm->dir, ptm->tm_id, rc);
            ret = RDPA_DRV_SH_DESTROY;
            goto tm_remove_exit;
        }
    }

    switch (ptm->dev_type)
    {
    case RDPA_IOCTL_DEV_PORT:
    case RDPA_IOCTL_DEV_TCONT:
    case RDPA_IOCTL_DEV_LLID:
    {
        queue_st *pq = &pport->queue_list[0];
        int i;

        for (i = 0; i < MAX_Q_PER_TM; i++, pq++) {
            if (pq->tm_id == ptm->tm_id) {
                if (pq->alloc)
                    pport->queue_alloc_cnt--;

                pq->tm_id = INVALID_ID;
                pq->qid   = 0;
                pq->alloc = FALSE;
            }
        }
        break;
    }
    default:
        break;
    }

tm_remove_exit:
    return ret;
}

/*
 * Check if any hidden entry will be overwritten, it may be created based on
 * a different QID/priority scheme.
 */
static queue_st* check_index_conflict(uint32_t dev_type, uint32_t port_id,
    uint32_t q_id, int new_index, int *old_index)
{
    port_st *pport;
    queue_st *pq;
    int old_q_index = INVALID_ID;
    int j;

    pport = get_virt_port(dev_type, port_id);
    if (pport == NULL)
    {
        CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)", dev_type, port_id);
        return NULL;
    }

    if ((q_id >= MAX_Q_PER_TM) || (new_index < 0))
    {
        CMD_TM_LOG_ERROR("Invalid q_id(%u) or q_index(%d)", q_id, new_index);
        return NULL;
    }

    pq = &(pport->queue_list[q_id]);
    old_q_index = pq->q_index;

    pq = &(pport->queue_list[0]);
    for (j = 0; j < MAX_Q_PER_TM; j++, pq++)
    {
       if ((pq->alloc == FALSE) && (pq->q_index == new_index) &&
           (pq->qid != q_id))
       {
           /* Confliction with hidden queue exists. */
           *old_index = old_q_index;
           return pq;
       }
    }

    return NULL;
}

static BOOL check_qid_conflict(uint32_t dev_type, uint32_t port_id,
    uint32_t q_id, int *old_index)
{
    port_st *pport;
    queue_st *pq;
    int old_q_index = INVALID_ID;
    BOOL found = FALSE;

    pport = get_virt_port(dev_type, port_id);
    if (pport == NULL)
    {
        CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)", dev_type, port_id);
        return FALSE;
    }

    if (q_id >= MAX_Q_PER_TM)
    {
        CMD_TM_LOG_ERROR("Invalid q_id(%u)", q_id);
        return FALSE;
    }

    pq = &(pport->queue_list[q_id]);
    old_q_index = pq->q_index;

    if ((pq->alloc == FALSE) && (pq->qid == q_id))
    {
        *old_index = old_q_index;
        found = TRUE;
    }

    return found;
}

static int port_q_cfg(
    rdpa_drv_ioctl_tm_t *ptm)
{
    bdmf_object_handle sched = NULL;
    rdpa_egress_tm_key_t tm_key = {};
    rdpa_tm_queue_cfg_t queue_cfg = {};
    rdpa_tm_queue_cfg_t cur_queue_cfg = {};
    rdpa_tm_sched_mode arbiter_mode = rdpa_tm_sched_disabled;
    bdmf_error_t rc = BDMF_ERR_OK;
    int cur_q_index = INVALID_ID;
    queue_st *swap_pq = NULL;
    int q_index;
    int ret = 0;

    CMD_TM_LOG_DEBUG("Q CONFIG: tm(%u) dir(%u) index(%u) q_id(%u) qsize(%u) shapingrate(%u)",
                     ptm->tm_id, ptm->dir, ptm->index, ptm->q_id, ptm->qsize, ptm->shaping_rate);

    tm_key.dir   = ptm->dir;
    tm_key.index = ptm->tm_id;

    if ((rc = rdpa_egress_tm_get(&tm_key, &sched))) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_get() failed: tm(%u) rc(%d)", ptm->tm_id, rc);
        return RDPA_DRV_TM_GET;
    }

    if ((rc = rdpa_egress_tm_mode_get(sched, &arbiter_mode))) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_mode_get() failed: tm(%u) rc(%d)", ptm->tm_id, rc);
        ret = RDPA_DRV_MODE_GET;
        goto port_q_cfg_exit;
    }

    queue_cfg.queue_id = ptm->q_id;
    queue_cfg.drop_alg = rdpa_tm_drop_alg_dt;
    queue_cfg.drop_threshold = getDeviceQueueSize(ptm->qsize);
#if defined(CONFIG_BCM_XRDP) 
    queue_cfg.rl_cfg.af_rate = ptm->shaping_rate * 1000;
#endif
    queue_cfg.weight = ptm->weight;
    queue_cfg.high_class.min_threshold = 0;
    queue_cfg.high_class.max_threshold = 0;
    queue_cfg.low_class.min_threshold = 0;
    queue_cfg.low_class.max_threshold = 0;
    queue_cfg.stat_enable = 1;
    queue_cfg.best_effort = ptm->best_effort;

    q_index = ptm->index;
    if (q_index == -1)
    {
        /* queue index is not given. derive it from qid. */
        if (arbiter_mode == rdpa_tm_sched_disabled)
            q_index = 0;   /* rate limiter has only one queue */
        else
        {
            q_index = get_queue_index(ptm);
        }
    }
    else
    {
        BOOL useSingleTm = ((get_port_tm_caps(ptm->dir) & RDPA_TM_1LEVEL_CAPABLE) == RDPA_TM_1LEVEL_CAPABLE);
        
        if (arbiter_mode == rdpa_tm_sched_sp_wrr &&
            useSingleTm)
        {
            if (queue_cfg.weight != 0)
            {
                q_index += FIRST_SINGLE_TM_WRR_Q;
            }
            
            /* In some epon special case, ctrl q will always occupy q_cfg[0], so need +1 here */
            if (queue_cfg.weight == 0 && share_root_tm && ptm->dir == rdpa_dir_us)
            {
                q_index += 1;
            }
        }
    }

    CMD_TM_LOG_DEBUG("rdpa_egress_tm_queue_cfg_set(): arbiter(%u) tm(%u) q_index(%u) q_index2(%u) q_id(%u) drop_alg(%u) qsize(%u) weight(%u) minBufs(%u) shaper_rate(%u) clean(%u)",
                     arbiter_mode, ptm->tm_id, ptm->index, q_index, ptm->q_id, queue_cfg.drop_alg, ptm->qsize, ptm->weight, queue_cfg.reserved_packet_buffers, ptm->shaping_rate, ptm->q_clean);

    if ( ptm->qsize )
    {
        if ( ptm->q_clean == TRUE )
        {
            swap_pq = check_index_conflict(ptm->dev_type, ptm->dev_id, ptm->q_id, q_index,
                                           &cur_q_index);
            if ( swap_pq != NULL )
            {
                if ( (rc = rdpa_egress_tm_queue_cfg_get(sched, q_index, &cur_queue_cfg)) )
                {
                    CMD_TM_LOG_ERROR("rdpa_egress_tm_queue_cfg_get() failed: q_index %d, rc(%d)",
                                     q_index, rc);
                    ret = RDPA_DRV_Q_CFG_GET;
                    goto port_q_cfg_exit;
                }
            }
        }
        else
        {
            if ( (check_qid_conflict(ptm->dev_type, ptm->dev_id, ptm->q_id, &cur_q_index) == TRUE) &&
                 ((rdpa_egress_tm_queue_cfg_get(sched, cur_q_index, &cur_queue_cfg) == 0) &&
                  cur_queue_cfg.drop_threshold == 0) )
            {
                CMD_TM_LOG_DEBUG("queue_cfg_delete(): q_index(%u)", cur_q_index);
                if ( (rc = rdpa_egress_tm_queue_cfg_delete(sched, cur_q_index)) )
                {
                    CMD_TM_LOG_ERROR("rdpa_egress_tm_queue_cfg_delete() failed: q_index %d, rc(%d)",
                                     cur_q_index, rc);
                    ret = RDPA_DRV_Q_CFG_GET;
                    goto port_q_cfg_exit;
                }
            }
        }
    }

    if ((rc = rdpa_egress_tm_queue_cfg_set(sched, q_index, &queue_cfg)))
    {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_queue_cfg_set() failed: mode(%u) q_index(%u) rc(%d)",
                         arbiter_mode, q_index, rc);
        ret = RDPA_DRV_Q_CFG_SET;
        goto port_q_cfg_exit;
    }

    if (ptm->qsize)
    {
        if ((ret = add_q_to_virt_port(ptm->dev_type, ptm->dev_id, ptm->tm_id,
                                      ptm->q_id, q_index, ptm->weight)))
        {
            CMD_TM_LOG_ERROR("add_q_to_virt_port() failed: "
                             "port_id(%u) tm_id(%u) q_id(%u) q_index(%u) ret(%d)",
                             ptm->dev_id, ptm->tm_id, ptm->q_id, q_index, ret);
        }

        if (swap_pq != NULL)
        {
            swap_pq->q_index = cur_q_index;
            if ((rc = rdpa_egress_tm_queue_cfg_set(sched, cur_q_index, &cur_queue_cfg)))
            {
                CMD_TM_LOG_ERROR("rdpa_egress_tm_queue_cfg_set() failed: "
                                 "mode(%u) q_index(%u) rc(%d)",
                                 arbiter_mode, cur_q_index, rc);
                ret = RDPA_DRV_Q_CFG_SET;
                goto port_q_cfg_exit;
            }
        }
    }
    else
    {
        if ((ret = remove_q_from_virt_port(ptm->dev_type, ptm->dev_id, ptm->q_id)))
            CMD_TM_LOG_ERROR("remove_q_from_virt_port() failed: "
                             "port_id(%u) q_id(%u) rc(%d)",
                             ptm->dev_id, ptm->q_id, ret);
    }

port_q_cfg_exit:
    bdmf_put(sched);
    return ret;
}

/* q cfg implementation for general dev(not port) use */
static int gen_device_q_cfg(rdpa_drv_ioctl_tm_t *ptm)
{
    bdmf_object_handle sched = NULL;
    rdpa_egress_tm_key_t tm_key = {};
    rdpa_tm_queue_cfg_t queue_cfg = {};
    rdpa_tm_sched_mode arbiter_mode = rdpa_tm_sched_disabled;
    bdmf_error_t rc = BDMF_ERR_OK;
    rdpa_tm_sched_mode tm_mode;
    int cur_q_index = INVALID_ID;
    rdpa_tm_queue_cfg_t cur_queue_cfg = {};
    rdpa_tm_service_queue_t service_queue = {};
    int ret = 0;

    int q_index;

    tm_key.dir = ptm->dir;
    tm_key.index = ptm->tm_id;
    if ((rc = rdpa_egress_tm_get(&tm_key, &sched))) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_get() failed: tm(%u) rc(%d)",
                         ptm->tm_id, rc);
        ret = RDPA_DRV_TM_GET;
        goto q_cfg_exit;
    }

    if ((rc = rdpa_egress_tm_mode_get(sched, &arbiter_mode))) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_mode_get() failed: tm(%u) rc(%d)", ptm->tm_id, rc);
        ret = RDPA_DRV_MODE_GET;
        goto q_cfg_exit;
    } 

    if ((rc = rdpa_egress_tm_service_queue_get(sched, &service_queue))) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_service_queue_get() failed: tm(%u) rc(%d)", ptm->tm_id, rc);
        ret = RDPA_DRV_MODE_GET;
        goto q_cfg_exit;
    }
    ptm->service_queue = service_queue.enable;

    queue_cfg.queue_id           = ptm->q_id;
    queue_cfg.drop_alg           = rdpa_tm_drop_alg_dt;
    queue_cfg.drop_threshold     = getDeviceQueueSize(ptm->qsize);
#if defined(CONFIG_BCM_XRDP) 
    queue_cfg.rl_cfg.af_rate = ptm->shaping_rate * 1000; /* shaping_rate is in kbit/s: 1 kilobit = 1000 bits */
#endif
    queue_cfg.weight             = ptm->weight;
    queue_cfg.reserved_packet_buffers = ptm->minBufs;
    queue_cfg.high_class.min_threshold = 0;
    queue_cfg.high_class.max_threshold = 0;
    queue_cfg.low_class.min_threshold = 0;
    queue_cfg.low_class.max_threshold = 0;
    queue_cfg.stat_enable        = 1;
    queue_cfg.best_effort        = ptm->best_effort;

    if ((rc = rdpa_egress_tm_mode_get(sched, &tm_mode))) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_mode_get() failed: rc(%d)", rc);
        ret = RDPA_DRV_Q_CFG_SET;
        goto q_cfg_exit;
    }
    if (tm_mode == rdpa_tm_sched_disabled) {
        q_index = 0;
    }
    else {
        BOOL useSingleTm = FALSE;

        if (ptm->dev_type == RDPA_IOCTL_DEV_TCONT)
            useSingleTm = ((get_tcont_tm_caps() & RDPA_TM_1LEVEL_CAPABLE) == RDPA_TM_1LEVEL_CAPABLE);
        
        if (ptm->dev_type == RDPA_IOCTL_DEV_LLID)
            useSingleTm = ((get_llid_tm_caps() & RDPA_TM_1LEVEL_CAPABLE) == RDPA_TM_1LEVEL_CAPABLE);
        
        q_index = get_queue_index(ptm);

        if (arbiter_mode == rdpa_tm_sched_sp_wrr &&
            useSingleTm)
        {
            if (queue_cfg.weight != 0 && q_index < FIRST_SINGLE_TM_WRR_Q)
                q_index += FIRST_SINGLE_TM_WRR_Q;

            /* In some epon special case, ctrl q will always occupy q_cfg[0], so need +1 here */
            if (queue_cfg.weight == 0 && share_root_tm && ptm->dir == rdpa_dir_us)
                q_index += 1;
        }
    }

    CMD_TM_LOG_DEBUG("rdpa_egress_tm_queue_cfg_set(): "
                     "tm(%u) q_index(%d) q_id(%u) drop_alg(%u) drop_tresh(%u) weight(%u) af(%llu)",
                     ptm->tm_id, q_index, queue_cfg.queue_id, queue_cfg.drop_alg,
                     queue_cfg.drop_threshold, queue_cfg.weight, (uint64_t)queue_cfg.rl_cfg.af_rate);

    if (ptm->dev_type == RDPA_IOCTL_DEV_LLID ||
        ptm->dev_type == RDPA_IOCTL_DEV_TCONT)
    {
        if ( ptm->qsize )
        {
            if ( (check_qid_conflict(ptm->dev_type, ptm->dev_id, ptm->q_id, &cur_q_index) == TRUE) &&
                    ((rdpa_egress_tm_queue_cfg_get(sched, cur_q_index, &cur_queue_cfg) == 0) &&
                     cur_queue_cfg.drop_threshold == 0) )
            {
                CMD_TM_LOG_DEBUG("queue_cfg_delete(): q_index(%u)", cur_q_index);
                if ( (rc = rdpa_egress_tm_queue_cfg_delete(sched, cur_q_index)) )
                {
                    CMD_TM_LOG_ERROR("rdpa_egress_tm_queue_cfg_delete() failed: q_index %d, rc(%d)",
                            cur_q_index, rc);
                    ret = RDPA_DRV_Q_CFG_GET;
                    goto q_cfg_exit;
                }
            }
        }
    }

    if ((rc = rdpa_egress_tm_queue_cfg_set(sched, q_index, &queue_cfg))) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_queue_cfg_set() failed: rc(%d)", rc);
        ret = RDPA_DRV_Q_CFG_SET;
        goto q_cfg_exit;
    }

    if (ptm->dev_type == RDPA_IOCTL_DEV_LLID ||
        ptm->dev_type == RDPA_IOCTL_DEV_TCONT)
    {
        if (ptm->qsize) {
            if ((ret = add_q_to_virt_port(ptm->dev_type, ptm->dev_id, ptm->tm_id,
                                          ptm->q_id, q_index, ptm->weight)))
                CMD_TM_LOG_ERROR("add_q_to_virt_port() failed: "
                                 "port_id(%u) tm_id(%u) q_id(%u) q_index(%d) ret(%d)",
                                 ptm->dev_id, ptm->tm_id, ptm->q_id, q_index, ret);
        }
        else {
            if ((ret = remove_q_from_virt_port(ptm->dev_type, ptm->dev_id, ptm->q_id)))
                CMD_TM_LOG_ERROR("remove_q_from_virt_port() failed: "
                                 "port_id(%u) q_id(%u) rc(%d)",
                                 ptm->dev_id, ptm->q_id, ret);
        }
    }

q_cfg_exit:
    if (sched)
        bdmf_put(sched);
    return ret;
}


static int q_config(
    rdpa_drv_ioctl_tm_t *ptm)
{
    int ret = 0;
    switch (ptm->dev_type)
    {
    case RDPA_IOCTL_DEV_PORT:
        ret = port_q_cfg(ptm);
        break;
    case RDPA_IOCTL_DEV_LLID:
    case RDPA_IOCTL_DEV_TCONT:
    case RDPA_IOCTL_DEV_NONE:
        ret = gen_device_q_cfg(ptm);
        break;
    default:
        ret = RDPA_DRV_ERROR;
        break;
    }
    return ret;
}

static
int port_root_tm_get(uint32_t dev_type, uint32_t port_id, uint32_t *proot_tm_id, BOOL *found)
{
    port_st *pport = NULL;
    int     ret = 0;

#ifdef __DUMP_PORTS__
    dump_ports();
#endif

    *found      = FALSE;
    *proot_tm_id = 0;

    pport = get_virt_port(dev_type, port_id);
    if (pport == NULL)
    {
        CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)", dev_type, port_id);
        return RDPA_DRV_PORT_ID_NOT_VALID;
    }

    if (pport->alloc)
    {
        *found      = TRUE;
        *proot_tm_id = pport->root_tm_id;
    }
    else
    {
        bdmf_object_handle port_obj = NULL;
        bdmf_number        root_tm_id;
        rdpa_port_tm_cfg_t tm_cfg;
        bdmf_error_t       rc = BDMF_ERR_OK;

        if ((rc = rdpa_port_get(port_id, &port_obj)))
        {
            CMD_TM_LOG_ERROR("rdpa_port_get FAILED: rc(%d)", rc);
            ret = RDPA_DRV_PORT_GET;
            goto port_root_tm_exit;
        }

        if ((rc = rdpa_port_tm_cfg_get(port_obj, &tm_cfg)))
        {
            CMD_TM_LOG_ERROR("rdpa_port_tm_cfg_get FAILED rc=%d", rc);
            ret = RDPA_DRV_TM_CFG_GET;
            goto port_root_tm_exit;
        }

        if ((rc = rdpa_egress_tm_index_get(tm_cfg.sched, &root_tm_id)))
        {
            CMD_TM_LOG_INFO("**: RDPA_IOCTL_TM_CMD_GET_ROOT_TM: ROOT NOT FOUND");
            ret = 0; //this is not an error, just Root TM is not set yet!
        }
        else
        {
            rdpa_tm_sched_mode mode;

            *found = TRUE;
            *proot_tm_id = root_tm_id;
            CMD_TM_LOG_INFO("**: RDPA_IOCTL_TM_CMD_GET_ROOT_TM: ROOT TM id=[%u]", *proot_tm_id);
            
            rdpa_egress_tm_mode_get(tm_cfg.sched, &mode);

            /* set cfg_flags to 0 */
            ret = add_root_tm_to_virt_port(dev_type, port_id, *proot_tm_id, mode, 0);
        }

port_root_tm_exit:
        if (port_obj)
            bdmf_put(port_obj);
    }

    return ret;
}

static
int llid_root_tm_get(uint32_t llid_id, uint32_t *root_tm_id, BOOL *found)
{
    int ret = 0;
    bdmf_object_handle   llid_obj = NULL;
    bdmf_object_handle   tm_obj = NULL;
    bdmf_number tmp_id;
    int wan_root_tm_id;
    BOOL wan_found;
    bdmf_error_t rc = BDMF_ERR_OK;

    *found = FALSE;
    if ((rc = rdpa_llid_get(llid_id, &llid_obj)))
    {
        CMD_TM_LOG_ERROR("rdpa_llid_get FAILED: llid(%u) rc(%d)", llid_id, rc);
        ret = RDPA_DRV_LLID_GET;
        goto llid_root_tm_exit;
    }

    if ((rc = rdpa_llid_egress_tm_get(llid_obj, &tm_obj)))
    {
        CMD_TM_LOG_ERROR("rdpa_llid_egress_tm_get FAILED: llid(%u) rc(%d)", llid_id, rc);
        ret = RDPA_DRV_LLID_TM_GET;
        goto llid_root_tm_exit;
    }

    if (!tm_obj)
    {
        goto llid_root_tm_exit;
    }

    if ((rc = rdpa_egress_tm_index_get(tm_obj, &tmp_id)))
    {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_index_get FAILED: rc(%d)", rc);
        ret = RDPA_DRV_LLID_TM_ID_GET;
        goto llid_root_tm_exit;
    }

    /* Check whether RDPA CAR mode is enabled and in use. */
    ret = port_root_tm_get(RDPA_IOCTL_DEV_NONE, rdpa_wan_type_to_if(rdpa_wan_epon), &wan_root_tm_id, &wan_found); 
    if ((ret == 0) && (wan_found == TRUE) && (wan_root_tm_id == tmp_id))
    {
        *found = FALSE;
        CMD_TM_LOG_DEBUG("llid(%u) root tm is shared with wan0", llid_id);
    }
    else
    {
        port_st *pport = NULL;

        pport = get_virt_port(RDPA_IOCTL_DEV_LLID, llid_id);
        if (pport == NULL)
        {
            goto llid_root_tm_exit;
        }
        else
        {
            if (pport->alloc == FALSE)
                goto llid_root_tm_exit;
        }
        
        *root_tm_id = tmp_id;
        *found = TRUE;
        /* clear the error ret */
        ret = 0;
        CMD_TM_LOG_DEBUG("get llid(%u) root tm successfully", llid_id);
    }

llid_root_tm_exit:
    if (llid_obj)
        bdmf_put(llid_obj);
    return ret;
}

static
int tcont_root_tm_get(uint32_t tcont_id, uint32_t *root_tm_id, BOOL *found)
{
    int ret = 0;
    bdmf_object_handle   tcont_obj = NULL;
    bdmf_object_handle   tm_obj = NULL;
    bdmf_number tmp_id;
    int wan_root_tm_id;
    BOOL wan_found;
    bdmf_error_t rc = BDMF_ERR_OK;

    *found = FALSE;
    if ((rc = rdpa_tcont_get(tcont_id, &tcont_obj)))
    {
        CMD_TM_LOG_ERROR("rdpa_tcont_get FAILED: tcont(%u) rc(%d)", tcont_id, rc);
        ret = RDPA_DRV_TCONT_GET;
        goto tcont_root_tm_exit;
    }

    if ((rc = rdpa_tcont_egress_tm_get(tcont_obj, &tm_obj)))
    {
        CMD_TM_LOG_ERROR("rdpa_tcont_egress_tm_get FAILED: tcont(%u) rc(%d)", tcont_id, rc);
        ret = RDPA_DRV_TCONT_TM_GET;
        goto tcont_root_tm_exit;
    }

    if (!tm_obj)
    {
        goto tcont_root_tm_exit;
    }

    if ((rc = rdpa_egress_tm_index_get(tm_obj, &tmp_id)))
    {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_index_get FAILED: rc(%d)", rc);
        ret = RDPA_DRV_TCONT_TM_ID_GET;
        goto tcont_root_tm_exit;
    }

    /* Check whether RDPA CAR mode is enabled and in use. */
    ret = port_root_tm_get(RDPA_IOCTL_DEV_NONE, rdpa_wan_type_to_if(rdpa_wan_gpon), &wan_root_tm_id, &wan_found); 
    if ((ret == 0) && (wan_found == TRUE) && (wan_root_tm_id == tmp_id))
    {
        *found = FALSE;
        CMD_TM_LOG_DEBUG("tcont(%u) root tm is shared with wan0", tcont_id);
    }
    else
    {
        /* clear the error ret */
        ret = 0;
        *root_tm_id = tmp_id;
        *found = TRUE;
        CMD_TM_LOG_DEBUG("get tcont(%u) root tm successfully", tcont_id);
    }

tcont_root_tm_exit:
    if (tcont_obj)
        bdmf_put(tcont_obj);
    return ret;
}

static
int svcq_root_tm_get(uint32_t dev_id, uint32_t *proot_tm_id, BOOL *found)
{
    port_st *pport = NULL;

    *found = FALSE;
    *proot_tm_id = 0;

    pport = get_virt_port(RDPA_IOCTL_DEV_NONE, dev_id);
    if (pport == NULL)
    {
        CMD_TM_LOG_ERROR("Invalid dev_id(%u)", dev_id);
        return RDPA_DRV_PORT_ID_NOT_VALID;
    }

    if (pport->alloc)
    {
        *found = TRUE;
        *proot_tm_id = pport->root_tm_id;
    }

    return RDPA_DRV_SUCCESS;
}

static
int get_best_effort_queue_from_tm(bdmf_object_handle egress_tm, uint32_t *tm, uint32_t *qid, BOOL *found)
{
    rdpa_tm_level_type level;
    int ret;
    int i;

    ret = rdpa_egress_tm_level_get(egress_tm, &level);
    if (ret) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_level_get failed, ret[%d]", ret);
        return RDPA_DRV_ERROR;
    }

    if (level == rdpa_tm_level_queue) {
        rdpa_tm_queue_cfg_t queue_cfg = {0};
        uint8_t num_queues;

        ret = rdpa_egress_tm_num_queues_get(egress_tm, &num_queues);
        if (ret) {
            CMD_TM_LOG_ERROR("rdpa_egress_tm_num_queues_get failed, ret[%d]", ret);
            return RDPA_DRV_ERROR;
        }

        /* get each queue config and see if any are best effort */
        for (i = 0; i < num_queues; i++) {
            ret = rdpa_egress_tm_queue_cfg_get(egress_tm, i, &queue_cfg);
            if (!ret && queue_cfg.best_effort) {
                bdmf_number tm_id;
                rdpa_egress_tm_index_get(egress_tm, &tm_id);
                *tm  = tm_id;
                *qid = queue_cfg.queue_id;
                *found = TRUE;
                break;
            }
        }
    } else {
        for (i = 0; i < RDPA_TM_MAX_SCHED_ELEMENTS; i++) {
            bdmf_object_handle subsidiary_tm = NULL;

            rdpa_egress_tm_subsidiary_get(egress_tm, i, &subsidiary_tm);
            if (!subsidiary_tm)
                continue;

            ret = get_best_effort_queue_from_tm(subsidiary_tm, tm, qid, found);
            if (ret || *found)
                break;
        }
    }

    return RDPA_DRV_SUCCESS;
}

static
int dev_root_tm_get(uint32_t dev_type, uint32_t dev_id, uint32_t *root_tm_id, BOOL *found)
{
    int ret = 0;
    switch (dev_type)
    {
    case RDPA_IOCTL_DEV_PORT:
        ret = port_root_tm_get(dev_type, dev_id, root_tm_id, found);
        break;
    case RDPA_IOCTL_DEV_TCONT:
        ret = tcont_root_tm_get(dev_id, root_tm_id, found);
        break;
    case RDPA_IOCTL_DEV_LLID:
        ret = llid_root_tm_get(dev_id, root_tm_id, found);
        break;
    case RDPA_IOCTL_DEV_NONE:
        ret = svcq_root_tm_get(dev_id, root_tm_id, found);
        break;
    default:
        ret = RDPA_DRV_ERROR;
        break;
    }

    return ret;
}

static int get_tm_config(rdpa_drv_ioctl_tm_t *tm_p, BOOL  b_root_tm)
{
    int                  rc = 0;
    int                  ret = 0;
    rdpa_tm_rl_cfg_t     rl_cfg = {};
    BOOL                 overall_rl = FALSE;
    rdpa_tm_service_queue_t          service_queue = {FALSE};
    rdpa_tm_sched_mode   arbiter_mode = rdpa_tm_sched_disabled;
    bdmf_object_handle   sched = NULL;
    BOOL                 b_tm_exist = TRUE;
    bdmf_number          weight = 0;
    rdpa_tm_level_type   tm_level = rdpa_tm_level_queue;
    rdpa_egress_tm_key_t tm_key = {};


    CMD_TM_LOG_DEBUG("IN: GET PORT TM CONFIG port_id(%u)", tm_p->dev_id);

    tm_key.dir   = tm_p->dir;
    tm_key.index = (b_root_tm)? tm_p->root_tm_id : tm_p->tm_id;

    if ((rc = rdpa_egress_tm_get(&tm_key, &sched))) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_get() failed: tm(%u) rc(%d)", tm_p->root_tm_id, rc);
        ret = RDPA_DRV_TM_GET;
        b_tm_exist = FALSE;
    }

    rl_cfg.af_rate = 0;
    rl_cfg.burst_size = 0;

    if (b_tm_exist) {
        if ((rc = rdpa_egress_tm_rl_get(sched, &rl_cfg))) {
             CMD_TM_LOG_ERROR("rdpa_egress_tm_rl_get() failed: port_id(%u) rc(%d)", tm_p->dev_id, rc);
             ret = RDPA_DRV_TM_CFG_GET;
             goto get_tm_config_exit;
       }
        if ((rc = rdpa_egress_tm_overall_rl_get(sched, &overall_rl))) {
             CMD_TM_LOG_ERROR("rdpa_egress_tm_overall_rl_get() failed: port_id(%u) rc(%d)", tm_p->dev_id, rc);
             ret = RDPA_DRV_TM_CFG_GET;
             goto get_tm_config_exit;

        }
        if ((rc = rdpa_egress_tm_mode_get(sched, &arbiter_mode))) {
             CMD_TM_LOG_ERROR("rdpa_egress_tm_mode_get() failed: port_id(%u) rc(%d)", tm_p->dev_id, rc);
             ret = RDPA_DRV_TM_CFG_GET;
             goto get_tm_config_exit;
        }
        if ((rc = rdpa_egress_tm_service_queue_get(sched, &service_queue))) {
             CMD_TM_LOG_ERROR("rdpa_egress_tm_service_queue_get() failed: port_id(%u) rc(%d)", tm_p->dev_id, rc);
             ret = RDPA_DRV_TM_CFG_GET;
             goto get_tm_config_exit;
        }
        if ((rc = rdpa_egress_tm_weight_get(sched, &weight))) {
             CMD_TM_LOG_ERROR("rdpa_egress_tm_weight_get() failed: port_id(%u) rc(%d)", tm_p->dev_id, rc);
             ret = RDPA_DRV_TM_CFG_GET;
             goto get_tm_config_exit;
        }

        if ((rc = rdpa_egress_tm_level_get(sched, &tm_level))) {
             CMD_TM_LOG_ERROR("rdpa_egress_tm_level_get() failed: port_id(%u) rc(%d)", tm_p->dev_id, rc);
             ret = RDPA_DRV_TM_CFG_GET;
             goto get_tm_config_exit;
        }
    }

    tm_p->shaping_rate = rl_cfg.af_rate / 1000;
    tm_p->burst        = rl_cfg.burst_size;
    tm_p->arbiter_mode = arbiter_mode;
    tm_p->service_queue= service_queue.enable;
    tm_p->weight       = weight;
    tm_p->level        = tm_level;
    tm_p->found        = b_tm_exist;

get_tm_config_exit:
    if (sched)
        bdmf_put(sched);

    return ret;
}

static int set_q_drop_alg(rdpa_drv_ioctl_tm_t *ptm)
{
    bdmf_object_handle sched = NULL;
    rdpa_egress_tm_key_t tm_key = {};
    rdpa_tm_queue_cfg_t queue_cfg = {};
    bdmf_error_t rc = BDMF_ERR_OK;
    rdpa_drv_ioctl_tm_t qinfo;
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158)
    bdmf_object_handle xtm_obj = NULL;
#endif
    int ret = 0;

    CMD_TM_LOG_DEBUG("Q DROP ALG: type(%u) dev_id(%u) dir(%u) q_id(%u) "
                     "drop_alg(%u) min_thr_hi(%u) max_thr_hi(%u) min_thr_lo(%u) max_thr_lo(%u)",
                     ptm->dev_type, ptm->dev_id, ptm->dir, ptm->q_id,
                     ptm->drop_alg, ptm->red_min_thr_hi, ptm->red_max_thr_hi,
                     ptm->red_min_thr_lo, ptm->red_max_thr_lo);

    if (ptm->dev_type == RDPA_IOCTL_DEV_XTM) {
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158)
        if ((rc = rdpa_xtmchannel_get(ptm->q_id, &xtm_obj)) || (xtm_obj == NULL)) {
            CMD_TM_LOG_ERROR("rdpa_xtmchannel_get() failed: channel(%d) rc(%d)", ptm->q_id, rc);
            return RDPA_DRV_PORT_GET;
        }

        if ((rc = rdpa_xtmchannel_egress_tm_get(xtm_obj, &sched))) {
            CMD_TM_LOG_ERROR("rdpa_xtmchannel_egress_tm_get() failed: channel(%d) rc(%d)", ptm->q_id, rc);
            ret = RDPA_DRV_PORT_GET;
            goto q_cfg_exit;
        }
        bdmf_get(sched);

        if ((rc = rdpa_egress_tm_queue_cfg_get(sched, 0, &queue_cfg))) {
            CMD_TM_LOG_ERROR("rdpa_egress_tm_queue_cfg_get() failed: q_index(0), rc(%d)", rc);
            ret = RDPA_DRV_Q_CFG_GET;
            goto q_cfg_exit;
        }
        qinfo.weight = queue_cfg.weight;
        qinfo.qsize = queue_cfg.drop_threshold;
        qinfo.index = 0;
#else
        return RDPA_DRV_PORT_GET;
#endif /* defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158) */
    } else {
        if ((ret = get_q_cfg(ptm->dev_type, ptm->dev_id, ptm->q_id, ptm->dir, &qinfo))) {
            CMD_TM_LOG_ERROR("get_q_cfg() failed: dev_type(%d) dev_id(%u) q_id(%u) ret(%d)",
                             ptm->dev_type, ptm->dev_id, ptm->q_id, ret);
            ret = RDPA_DRV_Q_CFG_GET;
            goto q_cfg_exit;
        }

        tm_key.dir   = ptm->dir;
        tm_key.index = qinfo.tm_id;
        if ((rc = rdpa_egress_tm_get(&tm_key, &sched))) {
            CMD_TM_LOG_ERROR("rdpa_egress_tm_get() failed: tm(%u) rc(%d)", ptm->tm_id, rc);
            return RDPA_DRV_TM_GET;
        }
    }

    /* Reuse saved weight and queue size. */
    queue_cfg.weight         = qinfo.weight;
    queue_cfg.drop_threshold = getDeviceQueueSize(qinfo.qsize);
    queue_cfg.stat_enable    = 1;

    queue_cfg.queue_id = ptm->q_id;
    queue_cfg.drop_alg = ptm->drop_alg;

    if (queue_cfg.drop_alg == rdpa_tm_drop_alg_red)
    {
        queue_cfg.low_class.min_threshold  = getDeviceQueueSize(ptm->red_min_thr_lo);
        queue_cfg.low_class.max_threshold  = getDeviceQueueSize(ptm->red_max_thr_lo);
        queue_cfg.high_class.min_threshold = queue_cfg.drop_threshold;
        queue_cfg.high_class.max_threshold = queue_cfg.drop_threshold;
    }
    else if (queue_cfg.drop_alg == rdpa_tm_drop_alg_wred)
    {
        queue_cfg.low_class.min_threshold  = getDeviceQueueSize(ptm->red_min_thr_lo);
        queue_cfg.low_class.max_threshold  = getDeviceQueueSize(ptm->red_max_thr_lo);
        queue_cfg.high_class.min_threshold = getDeviceQueueSize(ptm->red_min_thr_hi);
        queue_cfg.high_class.max_threshold = getDeviceQueueSize(ptm->red_max_thr_hi);
        queue_cfg.priority_mask_0 = ptm->priority_mask_0;
        queue_cfg.priority_mask_1 = ptm->priority_mask_1;
    }
    else
    {
        /* DT */
        queue_cfg.low_class.min_threshold  = 0;   
        queue_cfg.low_class.max_threshold  = 0;   
        queue_cfg.high_class.min_threshold = 0;
        queue_cfg.high_class.max_threshold = 0;
    }

    CMD_TM_LOG_DEBUG("rdpa_egress_tm_queue_cfg_set(): "
                     "tm(%u) q_id(%u) drop_alg(%u) hi_min_thr(%u) hi_max_thr(%u) lo_min_thr(%u) lo_max_thr(%u)",
                     ptm->tm_id, queue_cfg.queue_id, queue_cfg.drop_alg,
                     queue_cfg.high_class.min_threshold, queue_cfg.high_class.max_threshold, 
                     queue_cfg.low_class.min_threshold, queue_cfg.low_class.max_threshold);

    if ((rc = rdpa_egress_tm_queue_cfg_set(sched, qinfo.index, &queue_cfg)))
    {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_queue_cfg_set() failed: "
                         "tm(%u) q_id(%u) rc(%d)",
                         ptm->tm_id, queue_cfg.queue_id, rc);
        ret = RDPA_DRV_Q_CFG_SET;
        goto q_cfg_exit;
    }

q_cfg_exit:

    if (sched)
        bdmf_put(sched);

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158)
    if (xtm_obj)
        bdmf_put(xtm_obj);
#endif

    return ret;
}


static int set_q_size(rdpa_drv_ioctl_tm_t *ptm)
{
    bdmf_object_handle sched = NULL;
    rdpa_egress_tm_key_t tm_key = {};
    rdpa_tm_queue_cfg_t queue_cfg = {};
    bdmf_error_t rc = BDMF_ERR_OK;
    rdpa_drv_ioctl_tm_t qinfo = {};
    int ret = 0;

    CMD_TM_LOG_DEBUG("Q size: type(%u) dev_id(%u) dir(%u) q_id(%u) size(%u)",
                     ptm->dev_type, ptm->dev_id, ptm->dir, ptm->q_id, ptm->qsize);

    if ((ret = get_q_cfg(ptm->dev_type, ptm->dev_id, ptm->q_id, ptm->dir, &qinfo)))
    {
        CMD_TM_LOG_ERROR("get_q_cfg() failed: dev_type(%d) dev_id(%u) q_id(%u) ret(%d)",
                         ptm->dev_type, ptm->dev_id, ptm->q_id, ret);
        ret = RDPA_DRV_Q_CFG_GET;
        goto q_cfg_exit;
    }

    tm_key.dir   = ptm->dir;
    tm_key.index = qinfo.tm_id;
    if ((rc = rdpa_egress_tm_get(&tm_key, &sched)))
    {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_get() failed: tm(%u) rc(%d)", ptm->tm_id, rc);
        return RDPA_DRV_TM_GET;
    }

    /* Reuse saved parameters except queue size. */
    queue_cfg.queue_id = ptm->q_id;
    queue_cfg.weight   = qinfo.weight;
    queue_cfg.drop_alg = qinfo.drop_alg;
    queue_cfg.low_class.min_threshold  = getDeviceQueueSize(qinfo.red_min_thr_lo);
    queue_cfg.low_class.max_threshold  = getDeviceQueueSize(qinfo.red_max_thr_lo);
    queue_cfg.high_class.min_threshold = getDeviceQueueSize(qinfo.red_min_thr_hi);
    queue_cfg.high_class.max_threshold = getDeviceQueueSize(qinfo.red_max_thr_hi);

    //TODO: restore rl_cfg when rl_cfg is feature is done
    //queue_cfg.rl_cfg = qinfo.XXX;    
    
    queue_cfg.drop_threshold = getDeviceQueueSize(ptm->qsize);
    queue_cfg.stat_enable    = 1;

    CMD_TM_LOG_DEBUG("rdpa_egress_tm_queue_cfg_set(): "
                     "tm(%u) q_id(%u) q_size(%u) drop_alg(%u) hi_min_thr(%u) hi_max_thr(%u) lo_min_thr(%u) lo_max_thr(%u)",
                     ptm->tm_id, queue_cfg.queue_id, queue_cfg.drop_threshold, queue_cfg.drop_alg,
                     queue_cfg.high_class.min_threshold, queue_cfg.high_class.max_threshold, 
                     queue_cfg.low_class.min_threshold, queue_cfg.low_class.max_threshold);

    if ((rc = rdpa_egress_tm_queue_cfg_set(sched, qinfo.index, &queue_cfg)))
    {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_queue_cfg_set() failed: "
                         "tm(%u) q_id(%u) rc(%d)",
                         ptm->tm_id, queue_cfg.queue_id, rc);
        ret = RDPA_DRV_Q_CFG_SET;
        goto q_cfg_exit;
    }

q_cfg_exit:
    if (sched)
        bdmf_put(sched);
    return ret;
}


int rdpa_cmd_drv_get_tm_key_q_idx(
    uint32_t dev_id,
    uint32_t q_id,
    uint32_t dir,
    rdpa_egress_tm_key_t  *ptm_key,
    uint32_t *pq_idx
    )
{
    rdpa_egress_tm_key_t  tm_key = {};
    port_st      *pport;
    queue_st *pq;
    int ret = RDPA_DRV_SUCCESS;
    uint32_t dev_type = RDPA_IOCTL_DEV_PORT;
    uint32_t q_idx=0;

    if (q_id >= MAX_Q_PER_TM) {
        CMD_TM_LOG_ERROR("FAIL: Invalid q_id(%u)", q_id);
        return RDPA_DRV_Q_ID_NOT_VALID;
    }

    if (dev_type == RDPA_IOCTL_DEV_PORT) {
        pport = get_virt_port(dev_type, dev_id);
        if (pport == NULL) {
            CMD_TM_LOG_ERROR("Invalid devtype(%u) dev_id(%u)", dev_type, dev_id);
            return RDPA_DRV_PORT_ID_NOT_VALID;
        }

        if (dev_id == rdpa_wan_type_to_if(rdpa_wan_dsl)) {
            tm_key.dir   = dir;
            /* For xtm wan, the queue's root tm index is q_id. See xtmrt_runner.c. */
            tm_key.index = q_id;
        }
        else {
            if (!pport->alloc) {
                /* Port tm has not been configured.
                 * Return as SUCCESS.
                 */
                return RDPA_DRV_SUCCESS;
            }

            pq = &(pport->queue_list[q_id]);
            if (pq->tm_id == INVALID_ID) {
                /* Runner queue has not been configured.
                 * Return as SUCCESS.
                 */
                return RDPA_DRV_SUCCESS;
            }

            tm_key.dir    = dir;
            /* use the queue tm index.  */
            tm_key.index = pq->tm_id;
        }
        if (dir == rdpa_dir_us) {
            q_idx = 0; 
        }
        else {
            q_idx = q_id;
        }
    }
    else {
        CMD_TM_LOG_ERROR("not implement for such dev type(%u)", dev_type);
        return RDPA_DRV_ERROR;
    }

    *pq_idx = q_idx;
    *ptm_key = tm_key;
    return ret;
}


int rdpa_cmd_drv_get_q_size(
    uint32_t dev_id,
    uint32_t q_id,
    uint32_t dir,
    uint32_t *pqsize)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    bdmf_object_handle        sched = NULL;
    rdpa_egress_tm_key_t  tm_key = {};
    rdpa_tm_queue_cfg_t  q_cfg  = {};
    int ret = RDPA_DRV_SUCCESS;
    uint32_t q_idx=0;

    CMD_TM_LOG_DEBUG("IN: dev_id(%u) q_id(%u) dir(%u)", dev_id, q_id, dir);

    if ((ret = rdpa_cmd_drv_get_tm_key_q_idx( dev_id, q_id, dir, &tm_key, &q_idx)) != RDPA_DRV_SUCCESS)
        return ret;

    /* Get the root tm object of the queue */
    if ((rc = rdpa_egress_tm_get(&tm_key, &sched))) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_get() failed: tm(%d) rc(%d)", (int)tm_key.index, rc);
        return RDPA_DRV_TM_GET;
    }
    
    q_cfg.queue_id = q_id;

    /* get qsize from q_cfg.drop_threshold. */
    if ((rc = rdpa_egress_tm_queue_cfg_get(sched, q_idx, &q_cfg))) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_queue_cfg_get() failed: queue_id(%d) rc(%d)", q_idx, rc);
        return RDPA_DRV_Q_CFG_GET;
    }
   
    *pqsize  = q_cfg.drop_threshold;

    bdmf_put(sched);
    return ret;
}

EXPORT_SYMBOL(rdpa_cmd_drv_get_q_size);


int rdpa_cmd_drv_get_q_occupancy(
    uint32_t dev_id,
    uint32_t q_id,
    uint32_t dir,
    uint32_t *pq_occupancy)
{
    rdpa_stat_t pq_occupancy_stat;
    bdmf_error_t rc = BDMF_ERR_OK;
    bdmf_object_handle        sched = NULL;
    rdpa_egress_tm_key_t  tm_key = {};
    rdpa_tm_queue_index_t q_index = {};
    int ret = RDPA_DRV_SUCCESS;
    uint32_t q_idx=0;

    CMD_TM_LOG_DEBUG("IN: dev_id(%u) q_id(%u) dir(%u)", dev_id, q_id, dir);

    if ((ret = rdpa_cmd_drv_get_tm_key_q_idx( dev_id, q_id, dir, &tm_key, &q_idx)) != RDPA_DRV_SUCCESS)
        return ret;

    /* Get the root tm object of the queue */
    if ((rc = rdpa_egress_tm_get(&tm_key, &sched))) {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_get() failed: tm(%d) rc(%d)", (int)tm_key.index, rc);
        return RDPA_DRV_TM_GET;
    }
    
    q_index.queue_id = q_id;

    /* get queue occupancy */
    if ((rc = rdpa_egress_tm_queue_occupancy_get(sched, &q_index, &pq_occupancy_stat)) != BDMF_ERR_OK)
    {
        CMD_TM_LOG_DEBUG("rdpa_egress_tm_queue_occupancy_get() failed: queue_id(%d) rc(%d)", q_index.queue_id, rc);
        ret = RDPA_DRV_Q_OCCUPANCY_GET;
    }
    *pq_occupancy = pq_occupancy_stat.packets;
    bdmf_put(sched);
    return ret;
}
EXPORT_SYMBOL(rdpa_cmd_drv_get_q_occupancy);


static int set_q_shaper(
    rdpa_drv_ioctl_tm_t *ptm)
{
#if defined(CONFIG_BCM_XRDP)
    bdmf_object_handle sched = NULL;
    rdpa_egress_tm_key_t tm_key = {};
    rdpa_tm_queue_cfg_t queue_cfg = {};
    rdpa_tm_rl_rate_mode rl_mode;
    rdpa_drv_ioctl_tm_t qinfo = {};
    bdmf_error_t rc = BDMF_ERR_OK;
    int ret = 0;

    CMD_TM_LOG_DEBUG("Q SHAPER: tm(%u) dir(%u) index(%u) q_id(%u) min_rate(%u) shapingrate(%u) burst(%u)",
                     ptm->tm_id, ptm->dir, ptm->index, ptm->q_id, ptm->min_rate, ptm->shaping_rate, ptm->burst);

    if ((ret = get_q_cfg(ptm->dev_type, ptm->dev_id, ptm->q_id, ptm->dir, &qinfo)))
    {
        CMD_TM_LOG_ERROR("get_q_cfg() failed: dev_type(%d) dev_id(%u) q_id(%u) ret(%d)",
                         ptm->dev_type, ptm->dev_id, ptm->q_id, ret);
        return RDPA_DRV_Q_RATE_SET;
    }

    tm_key.dir   = ptm->dir;
    tm_key.index = qinfo.tm_id;
    if ((rc = rdpa_egress_tm_get(&tm_key, &sched))) 
    {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_get() failed: tm(%u) rc(%d)", ptm->tm_id, rc);
        return RDPA_DRV_Q_RATE_SET;
    }

    if ((rc = rdpa_egress_tm_queue_cfg_get(sched, qinfo.index, &queue_cfg)))
    {   
        CMD_TM_LOG_ERROR("rdpa_egress_tm_queue_cfg_get() failed: q_index(%u) rc(%d)",
                         qinfo.index, rc);
        ret = RDPA_DRV_Q_RATE_SET;
        goto port_q_cfg_exit;
        
    }

    rdpa_egress_tm_rl_rate_mode_get(sched, &rl_mode);
    if(rl_mode == rdpa_tm_rl_dual_rate)
    {
        queue_cfg.rl_cfg.be_rate = ptm->shaping_rate * 1000;
        queue_cfg.rl_cfg.af_rate = ptm->min_rate * 1000;
        queue_cfg.rl_cfg.burst_size = ptm->burst * 8;
    }
    else
    {
        /* Single mode, variable af_rate used as  best effort rate*/
        queue_cfg.rl_cfg.af_rate = ptm->shaping_rate * 1000;
        queue_cfg.rl_cfg.burst_size = ptm->burst;
    }

    if ((rc = rdpa_egress_tm_queue_cfg_set(sched, qinfo.index, &queue_cfg)))
    {
        CMD_TM_LOG_ERROR("rdpa_egress_tm_queue_cfg_set() failed: q_index(%u) rc(%d)",
                         qinfo.index, rc);
        ret = RDPA_DRV_Q_RATE_SET;
    }

port_q_cfg_exit:
    bdmf_put(sched);
    return ret;
#else
    return RDPA_DRV_Q_RATE_SET;
#endif
}

/* Debug functions. */

static void print_all_ports(void)
{
    port_st *pport = NULL;
    int i;

    bcm_printk("  # a    root root sp rootwrr     orl ol port mode qcnt\n");
    bcm_printk("=== = ======= ======= ======= ======= == ==== ==== ====\n");

    for (i = 0; i < MAX_VIRT_TM_PORTS; i++)
    {
        pport = &port_list[i];
        if ((i == TCONT_LLID_VP_BASE) || (i == DS_SVCQ_VP_BASE))
        {
            bcm_printk("--- - ------- ------- ------- ------- -- ---- ---- ----\n");
        }
        bcm_printk("%3d %1d %7d %7d %7d %7d %2d %4d %4d %4d\n",
           i,
           pport->alloc,
           pport->root_tm_id,
           pport->root_sp_tm_id,
           pport->root_wrr_tm_id,
           pport->orl_id,
           pport->orl_linked,
           (int)pport->port_id,
           (int)pport->arbiter_mode,
           pport->queue_alloc_cnt);
    }
    bcm_printk("=== = ======= ======= ======= ======= == ==== ==== ====\n");
}

static void print_port_queue(uint32_t virt_port_id)
{
    port_st *pport = NULL;
    queue_st *pq = NULL;
    int qid;

    if (virt_port_id >= MAX_VIRT_TM_PORTS)
    {
        bcm_printk("Invalid port %d, valid range [0, %d)\n",
          virt_port_id, MAX_VIRT_TM_PORTS);
        return;
    }

    bcm_printk("virtual port id = %d\n", virt_port_id);
    bcm_printk("  # a     qid   tm_id  qindex  weight\n");
    bcm_printk("=== = ======= ======= ======= =======\n");

    pport = &port_list[virt_port_id];
    for (qid = 0; qid < MAX_Q_PER_TM; qid++)
    {
        pq = &(pport->queue_list[qid]);
        bcm_printk("%3d %1d %7d %7d %7d %7u\n",
           qid,
           pq->alloc,
           (int)pq->qid,
           pq->tm_id,
           pq->q_index,
           pq->weight);
    }
}

static ssize_t rdpa_drv_tm_proc_write(struct file *file, const char *buf,
    size_t cnt, loff_t *pos)
{
    int i;
    char kbuf[RDPADRV_PROC_WR_KBUF_SIZE];
    uint32_t arg[RDPADRV_PROC_MAX_ARGS];
    int argc;
    char cmd;
    char opt;

    if ((cnt >= RDPADRV_PROC_WR_KBUF_SIZE) ||
      (copy_from_user(kbuf, buf, cnt) != 0))
    {
        return -EFAULT;
    }

    kbuf[cnt] = 0;

    argc = sscanf(kbuf, "%c %c %d", &cmd, &opt, &arg[0]);
    if (argc < 2)
    {
        bcm_printk("Usage:\n");
        bcm_printk("  Read all ports: 'r p'");
        bcm_printk("  Read all queues in one port: 'r q <virt port id>'\n");
        return 0;
    }

    bcm_printk("Debug write: cmd:'%c', opt:'%c', argc:%d\n", cmd, opt, argc);

    if (argc > 2)
    {
        for (i = 0; i < (argc - 2); i++)
        {
            bcm_printk("arg[%d]: %d\n", i, arg[i]);
        }
        bcm_printk("\n");
    }

    if (cmd != 'r')
    {
        bcm_printk("Unrecognized command '%c'\n", cmd);
        return 0;
    }

    switch (opt)
    {
        case 'p':
            print_all_ports();
            break;

        case 'q':
            print_port_queue(arg[0]);
            break;

        default:
            bcm_printk("Unrecognized option '%c'\n", opt);
            break;
    }

    return cnt;
}

static ssize_t rdpa_drv_tm_proc_read(struct file *file, char *buf,
    size_t count, loff_t *pos)
{
    CMD_TM_LOG_DEBUG("");
    return 0;
}

static void rdpa_drv_tm_procCreate(void)
{
    struct proc_dir_entry *ent;

    CMD_TM_LOG_DEBUG("");

    proc_dir = proc_mkdir(RDPADRV_PROC_ENTRY_NAME, NULL);
    if (proc_dir == NULL)
    {
        CMD_TM_LOG_ERROR("Unable to create /proc/%s.\n",
           RDPADRV_PROC_ENTRY_NAME);
        return;
    }

    ent = proc_create(RDPADRV_PROC_TM_NAME, 0, proc_dir, &rdpa_drv_tm_proc_fops);
    if (ent == NULL)
    {
        remove_proc_entry(RDPADRV_PROC_ENTRY_NAME, NULL);
        CMD_TM_LOG_ERROR("Unable to create /proc/%s/%s.\n",
          RDPADRV_PROC_ENTRY_NAME, RDPADRV_PROC_TM_NAME);
        return;
    }

    CMD_TM_LOG_DEBUG("rdpadrv_tm proc created.");
}

static void rdpa_drv_tm_procDelete(void)
{
    CMD_TM_LOG_DEBUG("");

    remove_proc_entry(RDPADRV_PROC_TM_NAME, proc_dir);
    remove_proc_entry(RDPADRV_PROC_ENTRY_NAME, NULL);
}


/*******************************************************************************/
/* global routines                                                             */
/*******************************************************************************/

/*******************************************************************************
 *
 * Function: rdpa_cmd_tm_ioctl
 *
 * IOCTL interface to the RDPA TM API.
 *
 *******************************************************************************/
int rdpa_cmd_tm_ioctl(unsigned long arg)
{
    rdpa_drv_ioctl_tm_t *userTm_p = (rdpa_drv_ioctl_tm_t *)arg;
    rdpa_drv_ioctl_tm_t tm;
    bdmf_object_handle sched = NULL;
    bdmf_object_handle root_sched = NULL;
    rdpa_egress_tm_key_t  egress_tm_key;
    bdmf_error_t rc = BDMF_ERR_OK;
    rdpa_if _rdpa_if = rdpa_if_none;
    int ret = 0;

    copy_from_user(&tm, userTm_p, sizeof(rdpa_drv_ioctl_tm_t));

    _rdpa_if = convert_dev_type_id_to_if(tm.dev_type, tm.dev_id);

    CMD_TM_LOG_DEBUG("RDPA TM CMD(%d) dev_type(%u) dev_id(%u) = rdpa_if(%d)", tm.cmd, tm.dev_type, tm.dev_id,_rdpa_if);

    /*  For T-CONT device type, index 0 is reserved for Default ALLOC Id so we
        need to increment by 1 the index of the TCONT that comes from
        OMCI userspace application. */
    if (tm.dev_type == RDPA_IOCTL_DEV_TCONT) {
        tm.dev_id++;
    }

    bdmf_lock();

    switch(tm.cmd)
    {
    case RDPA_IOCTL_TM_CMD_GET_ROOT_TM:
    {
        CMD_TM_LOG_DEBUG("*** RDPA_IOCTL_TM_CMD_GET_ROOT_TM: ROOT_tm(%u) dev_type(%u) dev_id(%u)", tm.root_tm_id, tm.dev_type, tm.dev_id);
        if ((ret = dev_root_tm_get(tm.dev_type, tm.dev_id, &tm.root_tm_id, &tm.found)))
            CMD_TM_LOG_ERROR("dev_root_tm_get() failed: dev_type(%u) dev_id(%u) rc(%d)",tm.dev_type, tm.dev_id, ret);

        break;
    }
    case RDPA_IOCTL_TM_CMD_GET_ROOT_SP_TM: {

        port_st *pport = NULL;

        CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_GET_ROOT_SP_TM: dev_type(%u) dev_id(%d)",
                        tm.dev_type, tm.dev_id);

#ifdef __DUMP_PORTS__
        dump_ports();
#endif

        tm.tm_id = INVALID_ID;
        tm.found = FALSE;

        pport = get_virt_port(tm.dev_type, tm.dev_id);
        if (pport == NULL) {
            CMD_TM_LOG_ERROR("Invalid devtype(%u) devid(%u)",
                             tm.dev_type, tm.dev_id);
            ret = RDPA_DRV_PORT_ID_NOT_VALID;
            break;
        }

        if (!pport->alloc) {
            CMD_TM_LOG_DEBUG("dev_id(%u) NOT allocated.", tm.dev_id);
            break;
        }

        tm.tm_id = pport->root_sp_tm_id;
        if (tm.tm_id != INVALID_ID)
            tm.found = TRUE;

        break;
    }
    case RDPA_IOCTL_TM_CMD_GET_ROOT_WRR_TM: {

        port_st *pport = NULL;

        CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_GET_ROOT_WRR_TM: dev_id(%d)", tm.dev_id);

#ifdef __DUMP_PORTS__
        dump_ports();
#endif

        tm.tm_id = INVALID_ID;
        tm.found = FALSE;

        pport = get_virt_port(tm.dev_type, tm.dev_id);
        if (pport == NULL) {
            CMD_TM_LOG_ERROR("Invalid devtype(%u) devid(%u)",
                             tm.dev_type, tm.dev_id);
            ret = RDPA_DRV_PORT_ID_NOT_VALID;
            break;
        }

        if (!pport->alloc) {
            CMD_TM_LOG_DEBUG("dev_id(%u) NOT allocated.", tm.dev_id);
            break;
        }

        tm.tm_id = pport->root_wrr_tm_id;
        if (tm.tm_id != INVALID_ID)
            tm.found = TRUE;

        break;
    }
    case RDPA_IOCTL_TM_CMD_GET_PORT_ORL: {
        CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_GET_PORT_ORL: port_id(%d)", tm.dev_id);

#ifdef __DUMP_PORTS__
        dump_ports();
#endif

        tm.tm_id        = INVALID_ID;
        tm.shaping_rate = 0;
        tm.orl_linked   = FALSE;
        tm.found        = FALSE;

        /* TMCTL1018 */
        if ((ret = get_orl(tm.dev_type, tm.dev_id, tm.dir,
                           &tm.tm_id, &tm.shaping_rate, &tm.orl_linked))) {
            CMD_TM_LOG_ERROR("get_orl() failed: port_id(%u) rc(%d)", tm.dev_id, ret);
            break;
        }

        if (tm.tm_id != INVALID_ID)
            tm.found = TRUE;

        break;
    }

    case RDPA_IOCTL_TM_GET_BY_QID: {
        CMD_TM_LOG_INFO("RDPA_IOCTL_TM_GET_BY_QID: ROOT_tm(%u) dev_type(%d) dev_id(%d) q_id(%u)",
                        tm.root_tm_id, tm.dev_type, tm.dev_id, tm.q_id);

#ifdef __DUMP_PORTS__
        dump_ports();
#endif
        tm.found = FALSE;

        if ((ret = get_tm_by_qid(tm.dev_type, tm.dev_id, tm.q_id,
                                 &tm.tm_id, &tm.found))) {
            CMD_TM_LOG_ERROR("get_tm_by_qid() failed: dev_type(%d) dev_id(%u) q_id(%u) ret(%d)",
                             tm.dev_type, tm.dev_id, tm.q_id, ret);
            goto ioctl_exit;
        }
        break;
    }
    case RDPA_IOCTL_TM_CMD_GET_QUEUE_CONFIG: {
        CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_GET_QUEUE_CONFIG: dev_type(%d) dev_id(%d) q_id(%u)",
                        tm.dev_type, tm.dev_id, tm.q_id);

#ifdef __DUMP_PORTS__
        dump_ports();
#endif
        tm.found = FALSE;

        if ((ret = get_q_cfg(tm.dev_type, tm.dev_id, tm.q_id, tm.dir, &tm))) {
            CMD_TM_LOG_ERROR("get_q_cfg() failed: dev_type(%d) dev_id(%u) q_id(%u) ret(%d)",
                             tm.dev_type, tm.dev_id, tm.q_id, ret);
            goto ioctl_exit;
        }
        break;
    }
    case RDPA_IOCTL_TM_CMD_ROOT_TM_CONFIG: {

        CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_ROOT_TM_CONFIG: dev_type(%u) dev_id(%u) dir(%s)",
                        tm.dev_type, tm.dev_id, (tm.dir == rdpa_dir_us)?"US":"DS");

        if ((ret = root_tm_config(&tm, &sched))) {
            CMD_TM_LOG_ERROR("root_tm_config() failed: dev_type(%u) dev_id(%u) ret(%d)", tm.dev_type, tm.dev_id, ret);
            goto ioctl_exit;
        }

        break;
    }
    case RDPA_IOCTL_TM_CMD_TM_CONFIG: {
        if (tm.dev_type == RDPA_IOCTL_DEV_NONE) {
            BDMF_MATTR(sched_attrs, rdpa_egress_tm_drv());
            rdpa_egress_tm_key_t egress_tm_key;
            rdpa_tm_service_queue_t service_queue = {.enable = 1};
            bdmf_object_handle sub_tm = NULL;
            bdmf_number new_tm_id;

            CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_TM_CONFIG: root_tm(%u) dev_type(%u) sub_tm(%u) dir(%s)",
                            tm.root_tm_id, tm.dev_type, tm.tm_id, (tm.dir == rdpa_dir_us)?"US":"DS");

            egress_tm_key.dir = tm.dir;
            egress_tm_key.index = tm.root_tm_id;

            if ((rc = rdpa_egress_tm_get(&egress_tm_key, &root_sched))) {
                CMD_TM_LOG_ERROR("rdpa_egress_tm_get() failed: ROOT_tm(%u) rc(%d)", tm.root_tm_id, rc);
                ret = RDPA_DRV_TM_GET;
                goto ioctl_exit;
            }

            rc = rdpa_egress_tm_subsidiary_get(root_sched, tm.index, &sub_tm);
            if (!rc) {
                bdmf_put(root_sched);
                ret = RDPA_IOCTL_TM_CMD_GET_TM_SUBSIDIARY;
                goto ioctl_exit;
            }

            rdpa_egress_tm_dir_set(sched_attrs, tm.dir);
            rdpa_egress_tm_level_set(sched_attrs, tm.level); /* rdpa_tm_level_queue / rdpa_tm_level_egress_tm */
            rdpa_egress_tm_mode_set(sched_attrs, tm.arbiter_mode);
            rdpa_egress_tm_service_queue_set(sched_attrs, &service_queue);
#if defined(CONFIG_BCM_XRDP)
            rdpa_egress_tm_num_queues_set(sched_attrs, DFT_SINGLE_TM_Q_NUM);
#endif

            if ((rc = bdmf_new_and_set(rdpa_egress_tm_drv(), root_sched, sched_attrs, &sched)) ||
                (sched == NULL)) {
                CMD_TM_LOG_ERROR("bdmf_new_and_set() failed: ROOT_tm(%u) rc(%d)", tm.root_tm_id, rc);
                bdmf_put(root_sched);
                return RDPA_DRV_NEW_TM_ALLOC;
            }

            if ((rc = rdpa_egress_tm_index_get(sched, &new_tm_id))) {
                CMD_TM_LOG_ERROR("rdpa_egress_tm_index_get() failed: ROOT_tm(%u) rc(%d)", tm.root_tm_id, rc);
                return RDPA_DRV_TM_INDEX_GET;
            }
            tm.tm_id = (uint32_t) new_tm_id;

            if ((rc = rdpa_egress_tm_subsidiary_set(root_sched, tm.index, sched))) {
                CMD_TM_LOG_ERROR("rdpa_egress_tm_subsidiary_set() failed: rootId(%u) rc(%d)", tm.root_tm_id, rc);
                bdmf_put(root_sched);
                ret = RDPA_DRV_SUBS_SET;
                goto ioctl_exit;
            }

            bdmf_put(root_sched);

            break;
        }
        else {
            BDMF_MATTR(sched_attrs, rdpa_egress_tm_drv());
            rdpa_egress_tm_key_t egress_tm_key;

            CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_TM_CONFIG: root_tm(%u) dev_type(%u) dev_id(%u) dir(%s)",
                            tm.root_tm_id, tm.dev_type, tm.dev_id, (tm.dir == rdpa_dir_us)?"US":"DS");

#ifdef __DUMP_PORTS__
            dump_ports();
#endif

            egress_tm_key.dir   = tm.dir;
            egress_tm_key.index = tm.root_tm_id;

            if ((rc = rdpa_egress_tm_get(&egress_tm_key, &root_sched))) {
                CMD_TM_LOG_ERROR("rdpa_egress_tm_get() failed: ROOT_tm(%u) rc(%d)", tm.root_tm_id, rc);
                ret = RDPA_DRV_TM_GET;
                goto ioctl_exit;
            }

            if ((ret = tm_set(sched_attrs, &tm, &tm.tm_id, root_sched, &sched))) {
                CMD_TM_LOG_ERROR("tm_set() failed: ROOT_tm(%u) rc(%d)", tm.root_tm_id, rc);
                bdmf_put(root_sched);
                goto ioctl_exit;
            }

            if (tm.arbiter_mode == rdpa_tm_sched_disabled) {
                rdpa_tm_sched_mode  root_tm_mode = rdpa_tm_sched_sp;
                
                if ((rc = rdpa_egress_tm_mode_get(root_sched, &root_tm_mode))) {
                    CMD_TM_LOG_ERROR("rdpa_egress_tm_mode_get() failed: tm(%u) rc(%d)",
                                     tm.root_tm_id, rc);
                    bdmf_put(root_sched);
                    bdmf_destroy(sched);
                    ret = RDPA_DRV_MODE_GET;
                    goto ioctl_exit;
                }

                if (root_tm_mode == rdpa_tm_sched_wrr) {
                    rdpa_egress_tm_weight_set(sched, tm.weight);
                }

                {
                    rdpa_tm_rl_cfg_t      rl_cfg = {};
                    rdpa_tm_rl_rate_mode  root_rl_mode = rdpa_tm_rl_single_rate;
                    rdpa_wan_type         wan_type     = rdpa_wan_none;

                    if ((ret = get_rdpa_wan_type(_rdpa_if, &wan_type))) {
                        CMD_TM_LOG_ERROR("get_rdpa_wan_type() failed: dev_id(%u) ret(%d)",
                                         userTm_p->dev_id, ret);
                        bdmf_put(root_sched);
                        bdmf_destroy(sched);
                        goto ioctl_exit;
                    }

                    if ((rc = rdpa_egress_tm_rl_rate_mode_get(root_sched, &root_rl_mode))) {
                        CMD_TM_LOG_ERROR("rdpa_egress_tm_rl_rate_mode_get() failed: tm(%u) rc(%d)",
                                         tm.root_tm_id, rc);
                        bdmf_put(root_sched);
                        bdmf_destroy(sched);
                        ret = RDPA_DRV_MODE_GET;
                        goto ioctl_exit;
                    }

                    if (wan_type     != rdpa_wan_epon &&
                        wan_type     != rdpa_wan_xepon &&
                        tm.dev_type  == RDPA_IOCTL_DEV_PORT &&
                        root_rl_mode == rdpa_tm_rl_dual_rate) {
                        if (tm.min_rate)
                            rl_cfg.af_rate = tm.min_rate * 1000;
                        else
                            /* for dual rate mode, AF rate must be greater than 0.
                             * i.e. set AF to 1 for no min rate shaping.
                             */
                            rl_cfg.af_rate = 1;

                        if (tm.shaping_rate) {
                            if (tm.shaping_rate < tm.min_rate) {
                                CMD_TM_LOG_ERROR("RDPA_IOCTL_TM_CMD_TM_CONFIG failed: maxRate(%u) < minRate(%u)",
                                                 tm.shaping_rate, tm.min_rate);
                                bdmf_put(root_sched);
                                bdmf_destroy(sched);
                                ret = RDPA_DRV_ERROR;
                                goto ioctl_exit;
                            }
                            rl_cfg.be_rate = (tm.shaping_rate - tm.min_rate) * 1000;
                        }
                        else
                            /* for dual rate mode, BE must be set to max for no max rate shaping. */
                            rl_cfg.be_rate = 1000000000L;  /* 1 Gig */
                    }
                    else
                        rl_cfg.af_rate = tm.shaping_rate * 1000;

                    /* BE bucket size is enforced. burst size shall be configured to sensible value. */ 
                    if (tm.burst < rl_cfg.be_rate)
                        rl_cfg.burst_size = 1000000000L / 8; /* The unit of Burst Size is Bytes. */
                    else
                        rl_cfg.burst_size = tm.burst ;

                    CMD_TM_LOG_DEBUG("egress_tm_rl_set: af_rate=%llu be_rate=%llu burst=%llu\n",
                                     (uint64_t)rl_cfg.af_rate, (uint64_t)rl_cfg.be_rate, (uint64_t)rl_cfg.burst_size);
                    rc = rdpa_egress_tm_rl_set(sched, &rl_cfg);
                }
            }

            CMD_TM_LOG_DEBUG("rdpa_egress_tm_subsidiary ind=%d is set, roottm(%u) subtm(%u)\n",
                             (int)tm.index, tm.root_tm_id, tm.tm_id);
            if ((rc = rdpa_egress_tm_subsidiary_set(root_sched, tm.index, sched))) {
                CMD_TM_LOG_ERROR("rdpa_egress_tm_subsidiary_set() failed: rootId(%u) rc(%d)", tm.root_tm_id, rc);
                bdmf_put(root_sched);
                bdmf_destroy(sched);
                ret = RDPA_DRV_SUBS_SET;
                goto ioctl_exit;
            }

            bdmf_put(root_sched);

            if ((tm.dev_type == RDPA_IOCTL_DEV_PORT) ||
               (tm.dev_type == RDPA_IOCTL_DEV_TCONT) ||
               (tm.dev_type == RDPA_IOCTL_DEV_LLID) ||
               (tm.dev_type == RDPA_IOCTL_DEV_NONE)) {
                port_st *pport = NULL;

                pport = get_virt_port(tm.dev_type, tm.dev_id);
                if (pport == NULL) {
                    CMD_TM_LOG_ERROR("Invalid devtype(%u) portid(%u)",
                                     tm.dev_type, tm.dev_id);
                    ret = RDPA_DRV_PORT_ID_NOT_VALID;
                    goto ioctl_exit;
                }

                if (pport->alloc) {
                    /* If the parent of this tm is the root tm, then this tm must be
                     * either the root sp tm or the root wrr tm. Save the tm id to
                     * the port list.
                     */
                    if (tm.root_tm_id == pport->root_tm_id) {
                        if (tm.arbiter_mode == rdpa_tm_sched_sp)
                            pport->root_sp_tm_id = tm.tm_id;
                        else if (tm.arbiter_mode == rdpa_tm_sched_wrr)
                            pport->root_wrr_tm_id = tm.tm_id;
#if defined(CONFIG_BCM_XRDP) 
                        else if (tm.arbiter_mode == rdpa_tm_sched_sp_wrr)
                        {
                            pport->root_sp_tm_id = tm.tm_id;
                            pport->root_wrr_tm_id = tm.tm_id;
                        }
#endif
                    }
                }
            }

            break;
        }
    }
    case RDPA_IOCTL_TM_CMD_ROOT_TM_REMOVE: {
        CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_ROOT_TM_REMOVE: tm(%u)", tm.root_tm_id);

        if ((ret = root_tm_remove(tm.dev_type, tm.dev_id, tm.root_tm_id, tm.dir))) {
            CMD_TM_LOG_ERROR("root_tm_remove() failed: ROOT_tm(%u) ret(%d)", tm.root_tm_id, ret);
            goto ioctl_exit;
        }

        break;
    }
    case RDPA_IOCTL_TM_CMD_TM_REMOVE: {
        CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_TM_REMOVE: tm(%u)", tm.tm_id);
        if ((ret = tm_remove(&tm)))
            CMD_TM_LOG_ERROR("tm_remove() failed: tm(%u) rc(%d)", tm.tm_id, ret);

        break;
    }
    case RDPA_IOCTL_TM_CMD_QUEUE_CONFIG: {
        CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_QUEUE_CONFIG: dev_type(%u) dev_id(%u) tm(%u) dir(%s) q_id(%u) index(%u)",
                        tm.dev_type, tm.dev_id, tm.tm_id, (tm.dir == rdpa_dir_us)?"US":"DS", tm.q_id, tm.index);

#ifdef __DUMP_PORTS__
        dump_ports();
#endif

        if ((ret = q_config(&tm))) {
            CMD_TM_LOG_ERROR("q_config() failed: dev_type(%u) dev_id(%u) tm_id(%u) q_id(%u) index(%u) ret(%d)",
                             tm.dev_type, tm.dev_id, tm.tm_id, tm.q_id, tm.index, ret);
            goto ioctl_exit;
        }

        break;
    }
    case RDPA_IOCTL_TM_CMD_QUEUE_REMOVE: {
        CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_QUEUE_REMOVE: dev_type(%u) dev_id(%u) tm(%u) dir(%s) q_id(%u) index(%u)",
                        tm.dev_type, tm.dev_id, tm.tm_id, (tm.dir == rdpa_dir_us)?"US":"DS", tm.q_id, tm.index);

#ifdef __DUMP_PORTS__
        dump_ports();
#endif

        if ((ret = q_config(&tm))) {
            CMD_TM_LOG_ERROR("q_config() failed: dev_type(%u) dev_id(%u) tm_id(%u) q_id(%u) index(%u) rc(%d)",
                             tm.dev_type, tm.dev_id, tm.tm_id, tm.q_id, tm.index, ret);
        }

        break;
    }
    case RDPA_IOCTL_TM_CMD_QUEUE_ALLOCATE: {
        CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_QUEUE_ALLOCATE: dev_type(%u) dev_id(%u) q_id(%u)",
                        tm.dev_type, tm.dev_id, tm.q_id);

#ifdef __DUMP_PORTS__
        dump_ports();
#endif

        ret = queue_allocate(tm.dev_type, tm.dev_id, tm.q_id, TRUE);

        break;
    }
    case RDPA_IOCTL_TM_CMD_QUEUE_DISLOCATE: {
        CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_QUEUE_DISLOCATE: dev_type(%u) dev_id(%u) q_id(%u)",
                        tm.dev_type, tm.dev_id, tm.q_id);

#ifdef __DUMP_PORTS__
        dump_ports();
#endif

        ret = queue_allocate(tm.dev_type, tm.dev_id, tm.q_id, FALSE);

        break;
    }
    case RDPA_IOCTL_TM_CMD_ORL_CONFIG: {

        CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_ORL_CONFIG: port_id(%u) dir(%s)",
                        tm.dev_id, (tm.dir == rdpa_dir_us)?"US":"DS");

        if ((ret = orl_config(&tm))) {
            CMD_TM_LOG_ERROR("orl_config() failed: port_id(%u) rc(%d)", tm.dev_id, rc);
        }

        break;
    }
    case RDPA_IOCTL_TM_CMD_ORL_REMOVE: {
        bdmf_error_t rc = BDMF_ERR_OK;

        CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_ORL_REMOVE: port_id(%u) orl_tm(%u) dir(%s)",
                        tm.dev_id, tm.tm_id, (tm.dir == rdpa_dir_us)?"US":"DS");

#ifdef __DUMP_PORTS__
        dump_ports();
#endif

        if ((ret = orl_remove(tm.dev_type, tm.dev_id, tm.dir))) {
            CMD_TM_LOG_ERROR("orl_remove() failed: orl_tm(%u) rc(%d)", tm.tm_id, rc);
        }

        break;
    }
    case RDPA_IOCTL_TM_CMD_ORL_LINK: {

        CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_ORL_LINK: port_id(%u) orl_tm_id(%u)",
                        tm.dev_id, tm.tm_id);

        if ((ret = orl_link(&tm))) {
            CMD_TM_LOG_ERROR("orl_config() failed: port_id(%u) orl_tm_id(%u) rc(%d)",
                             tm.dev_id, tm.tm_id, rc);
        }

        break;
    }
    case RDPA_IOCTL_TM_CMD_ORL_UNLINK: {

        CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_ORL_UNLINK: port_id(%u) orl_tm_id(%u)",
                        tm.dev_id, tm.tm_id);

        if ((ret = orl_unlink(&tm))) {
            CMD_TM_LOG_ERROR("orl_config() failed: port_id(%u) orl_tm_id(%u) rc(%d)",
                             tm.dev_id, tm.tm_id, rc);
        }

        break;
    }
    case RDPA_IOCTL_TM_CMD_TM_RL_CONFIG: {

        rdpa_tm_rl_cfg_t rl_cfg = {};

        CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_TM_RL_CONFIG: tm(%u) dir(%s)",
                        tm.tm_id, (tm.dir == rdpa_dir_us)?"US":"DS");

        egress_tm_key.dir = tm.dir;
        egress_tm_key.index = tm.tm_id;

        if ((rc = rdpa_egress_tm_get(&egress_tm_key, &sched)))
        {
            CMD_TM_LOG_ERROR("rdpa_egress_tm_get() failed: tm(%u) rc(%d)", tm.tm_id, rc);
            ret = RDPA_DRV_TM_GET;
            goto ioctl_exit;
        }

        rl_cfg.af_rate = tm.shaping_rate * 1000;    /* Best Effort: shaping_rate is in kbit/s: 1 kilobit = 1000 bits */
        rl_cfg.be_rate = 0;
        rl_cfg.burst_size = tm.burst;

        if ((rc = rdpa_egress_tm_rl_set(sched, &rl_cfg)))
        {
            CMD_TM_LOG_ERROR("rdpa_egress_tm_rl_set() failed: tm(%u) af(%llu) rc(%d)", tm.tm_id, (uint64_t)rl_cfg.af_rate, rc);
            bdmf_put(sched);
            ret = RDPA_DRV_Q_RATE_SET;
            goto ioctl_exit;
        }
        bdmf_put(sched);
        break;
    }

    case RDPA_IOCTL_TM_CMD_GET_TM_MEMORY_INFO: {
        if ((ret = get_tm_memory_info(&tm)))
        {
            CMD_TM_LOG_ERROR("get_tm_memory_info() failed: rc(%d)", ret);
        }
        break;
    }

    case RDPA_IOCTL_TM_CMD_GET_TM_CAPS: {
        CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_GET_TM_CAPS: dev_id(%d)", tm.dev_id);

        switch (tm.dev_type)
        {
        case RDPA_IOCTL_DEV_PORT:
            if ((ret = get_port_tm_parms(&tm)))
            {
                tm.found = FALSE;
                if (ret == RDPA_DRV_PORT_NOT_ALLOC)
                {
                    ret = 0; /* Hide this error from upper layer */
                }
                else
                {
                    CMD_TM_LOG_ERROR("get_port_tm_parms() failed: port_id(%u) rc(%d)", tm.dev_id, ret);
                }
            }
            else
            {
                tm.found = TRUE;
            }
            break;

        case RDPA_IOCTL_DEV_TCONT:
            if ((ret = get_tcont_tm_parms(&tm)))
            {
                CMD_TM_LOG_ERROR("get_tcont_tm_parms() failed: tcont_id(%u) rc(%d)", tm.dev_id, ret);
                tm.found = FALSE;
            }
            else
            {
                tm.found = TRUE;
            }
            break;

        case RDPA_IOCTL_DEV_LLID:
            if ((ret = get_llid_tm_parms(&tm)))
            {
                CMD_TM_LOG_ERROR("get_llid_tm_parms() failed: llid_id(%u) rc(%d)", tm.dev_id, ret);
                tm.found = FALSE;
            }
            else
            {
                tm.found = TRUE;
            }
            break;

        case RDPA_IOCTL_DEV_NONE:
            if ((ret = get_svcq_tm_parms(&tm)))
            {
                CMD_TM_LOG_ERROR("get_svcq_tm_parms() failed: svcq_id(%u) rc(%d)", tm.dev_id, ret);
                tm.found = FALSE;
            }
            else
            {
                tm.found = TRUE;
            }
            break;

        default:
            ret = RDPA_DRV_ERROR;
            CMD_TM_LOG_ERROR("illegal type(%u)", tm.dev_type);
        }

        break;
    }
    case RDPA_IOCTL_TM_CMD_GET_QUEUE_STATS: {

        CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_GET_QUEUE_STATS: dev_type(%d) dev_id(%d) q_id(%u)",
                        tm.dev_type, tm.dev_id, tm.q_id);

        if ((ret = get_q_stats(tm.dev_type, tm.dev_id, tm.q_id, tm.dir, &tm.qstats))) {
            CMD_TM_LOG_ERROR("get_q_stats() failed: dev_type(%d) dev_id(%u) q_id(%u) ret(%d)",
                             tm.dev_type, tm.dev_id, tm.q_id, ret);
            goto ioctl_exit;
        }
        break;
    }

    case RDPA_IOCTL_TM_CMD_GET_TM_CONFIG: {
        CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_GET_TM_CONFIG: dev_id(%d)", tm.dev_id);
        ret = get_tm_config(&tm, TRUE);  // meanwile config get will be supported for Root TM only
    }
    break;

    case RDPA_IOCTL_TM_CMD_SET_Q_DROP_ALG: {
        ret = set_q_drop_alg(&tm);
    }
    break;
    
    case RDPA_IOCTL_TM_CMD_SET_Q_SIZE: {
        ret = set_q_size(&tm);
    }
    break;

    case RDPA_IOCTL_TM_CMD_SET_Q_SHAPER: {
        ret = set_q_shaper(&tm);
    }
    break;

    case RDPA_IOCTL_TM_CMD_SVC_Q_ENABLE_GET: {
        CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_SVC_Q_ENABLE_GET: %sabled", svcq_enable ? "en" : "dis");
        tm.service_queue = svcq_enable;
    }
    break;

    case RDPA_IOCTL_TM_CMD_SVC_Q_ENABLE_SET: {
        CMD_TM_LOG_INFO("RDPA_IOCTL_TM_CMD_SVC_Q_ENABLE_SET: %sable", tm.service_queue ? "en" : "dis");
        svcq_enable = tm.service_queue;
    }
    break;

    case RDPA_IOCTL_TM_CMD_GET_BEST_EFFORT_TM_ID:
    {
        rdpa_egress_tm_key_t tm_key = {};
        bdmf_object_handle egress_tm = NULL;
        uint32_t root_tm_id;
        BOOL found = FALSE;

        tm.tm_id = INVALID_ID;
        tm.q_id = INVALID_ID;

        port_root_tm_get(RDPA_IOCTL_DEV_PORT, tm.dev_id, &root_tm_id, &found);
        if (!found)
            break;

        tm_key.dir = rdpa_dir_us;
        tm_key.index = root_tm_id;
        ret = rdpa_egress_tm_get(&tm_key, &egress_tm);
        if (ret || !egress_tm) {
            CMD_TM_LOG_ERROR("Unable to find egress_tm (id %d)\n", root_tm_id);
            return RDPA_DRV_ERROR;
        }

        found = FALSE;
        ret = get_best_effort_queue_from_tm(egress_tm, &tm.tm_id, &tm.q_id, &found);
        bdmf_put(egress_tm);
    }
    break;

    case RDPA_IOCTL_TM_CMD_GET_TM_SUBSIDIARY:
    {
        rdpa_egress_tm_key_t tm_key = {};
        bdmf_object_handle   sched = NULL;
        bdmf_object_handle   sub_sched = NULL;
        bdmf_number          tm_idx;

        tm_key.dir   = tm.dir;
        tm_key.index = tm.root_tm_id;

        tm.found = FALSE;
        if ((rc = rdpa_egress_tm_get(&tm_key, &sched))) {
            CMD_TM_LOG_ERROR("rdpa_egress_tm_get() failed: tm(%u) rc(%d)", tm.root_tm_id, rc);
            break;
        }

        if ((rc = rdpa_egress_tm_subsidiary_get(sched, tm.index, &sub_sched))) {
            CMD_TM_LOG_ERROR("rdpa_egress_tm_subsidiary_get() failed: tm(%u) index(%u) rc(%d)", tm.root_tm_id, tm.index, rc);
            break;
        }

        rdpa_egress_tm_index_get(sub_sched, &tm_idx);

        tm.found = TRUE;
        tm.tm_id = tm_idx;

        bdmf_put(sched);

        ret = get_tm_config(&tm, FALSE);
    }
    break;

    default:
        CMD_TM_LOG_ERROR("Invalid IOCTL cmd %d", tm.cmd);
        ret = RDPA_DRV_ERROR;
    }
ioctl_exit:
    if (ret) {
        CMD_TM_LOG_ERROR("rdpa_cmd_tm_ioctl() OUT: FAILED: cmd=%d tm(%u) rc(%d)", tm.cmd, tm.tm_id, rc);
    }

    bdmf_unlock();

    copy_to_user(userTm_p, &tm, sizeof(rdpa_drv_ioctl_tm_t));

    return ret;
}

/*******************************************************************************
 *
 * Function: rdpa_cmd_tm_init
 *
 * Initializes the RDPA TM API.
 *
 *******************************************************************************/
void rdpa_cmd_tm_init(void)
{
    port_st  *pport;
    int i;

    CMD_TM_LOG_DEBUG("RDPA TM INIT");

    /* Initialize the port list. */
    pport = &port_list[0];

    for (i = 0; i < MAX_VIRT_TM_PORTS; i++, pport++) {
        init_portlist_entry(pport);
    }

    rdpa_drv_tm_procCreate();

}

void rdpa_cmd_tm_exit(void)
{
    rdpa_drv_tm_procDelete();
}
