/*
* <:copyright-BRCM:2021:DUAL/GPL:standard
*
*    Copyright (c) 2021 Broadcom
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

#include <linux/module.h>
#include <linux/workqueue.h>
#include "linux/delay.h"
#include <linux/device.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/preempt.h>
#include <linux/bcm_log.h>
#include <pon_drv_nl.h>
#include "wan_drv.h"
#include "bcmtypes.h"
#include "bcm_synce_holdover.h"
#include <phy_drv.h>
#include <shared_utils.h>
#include <bdmf_system.h>
#include <pon_drv_task.h>

#if defined(CONFIG_BRCM_SMC_BOOT)
#include <bp3_license.h>
#endif

/* constants from Assaf's test python 12/2021 */
#define NCO_FRACN_NUM 10
#define NCO_FRACN_DEN 160
#define NCO_COMP_VAL 0
#define NCO_SW_HOLD_VAL 0
#define NCO_UPDATE_MAX (1 << 30)

#define PRINTK(format, ...) printk("%s: " format "\n", __FUNCTION__, ##__VA_ARGS__)
#define DEBUG 0
#define DBG_PRINTK(...) do { if (DEBUG) PRINTK(__VA_ARGS__); } while (0)

#define READ_32(a, r, base)             ( *(volatile uint32_t *) & (r)               = *(volatile uint32_t *)   (base + a) )
#define WRITE_32(a, r, base)            ( *(volatile uint32_t *) (base + a)  = *(volatile uint32_t *) & (r) )

static void *synce_virt_address = NULL/*, *merlin_virt_address = NULL*/;
#define SYNCE_READ_32(a, r)             READ_32(a, r, synce_virt_address)
#define SYNCE_WRITE_32(a, r)            WRITE_32(a, r, synce_virt_address)

#define NBITSSET(n)                     (uint32_t)( (uint64_t)~0U >> (8 * sizeof(0U) - n) )
#define CLR_BITS(r, pos, nbits)         ({ BCM_ASSERT( ((int)pos >= 0) && (pos <= 32) && ((int)nbits >= 0) && (pos + nbits <= 32) );        \
                                        r &= ~(NBITSSET(nbits) << pos); })

#define OR_BITS(r, pos, nbits, v)       ({ BCM_ASSERT( ((int)pos >= 0) && (pos <= 32) && ((int)nbits >= 0) && (pos + nbits <= 32) );        \
                                        r |=   v << pos;  })

#define CLR_BIT(r, pos)                 CLR_BITS(r, pos, 1)
#define OR_BIT(r, pos, v)               OR_BITS(r, pos, 1, v)


/* state machine */
typedef enum {
    sm_first,
    sm_no_sync,
    sm_a_w5,
    sm_b_w1,
    sm_c_w5,
    sm_holdover,
    sm_park,
    sm_last,
} sm_state;

#define INITIALIZER(tag) [tag] = #tag  // can be used in any order to init an array of field names

const char *sm_state_names[] = {
    INITIALIZER(sm_first),
    INITIALIZER(sm_no_sync),
    INITIALIZER(sm_a_w5),
    INITIALIZER(sm_b_w1),
    INITIALIZER(sm_c_w5),
    INITIALIZER(sm_holdover),
    INITIALIZER(sm_park),
    INITIALIZER(sm_last),
};

#define ILLEGAL_STATE(s)        (((s) <= sm_first) || ((s) >= sm_last))
#define STATE_NAME(s)           (ILLEGAL_STATE(s) ? "out of range" : sm_state_names[s])


/* data structure */

struct synce_holdover_data
{
    struct device *dev;
    struct mutex st_mutex;
    spinlock_t lock_sm;
    int eth_clk_good;
    volatile sm_state current_state;
    volatile unsigned long jiffies_entering_state;
    struct timer_list synce_holdover_timer_A5;
    struct timer_list synce_holdover_timer_B1;
    struct timer_list synce_holdover_timer_C5;
    bdmf_queue_t synce_holdover_msg_q;
    int eth_mode; /* 0 = 10G, 1 = 5G, 0 = 2.5G, -1 < 2.5G */
    phy_dev_t *lan_phy_dev;
    int merlin_up;
    int pon_up;
    int serdes_wan_type;
    uint32_t nums_pon_sync_events; /* timer cb can use to check if there have been events since mod_timer */
    uint32_t ref_nums_pon_sync_events;
    int initialized;
} data;


/* helpers */

static void synce_clear_reg(uint32_t offset, uint32_t position, uint32_t nbits)
{
    volatile uint32_t reg;

    SYNCE_READ_32(offset, reg);
    usleep_range(50, 100);
    CLR_BITS(reg, position, nbits);
    SYNCE_WRITE_32(offset, reg);
}

static void synce_or_reg(uint32_t offset, uint32_t position, uint32_t nbits, uint32_t val)
{
    volatile uint32_t reg;

    SYNCE_READ_32(offset, reg);
    OR_BITS(reg, position, nbits, val);
    SYNCE_WRITE_32(offset, reg);
}

static int merlin_enable_synce(phy_dev_t *lan_phy_dev)
{
    int rc = 0;

    rc = rc ?: phy_dev_c45_write_mask(lan_phy_dev, 1, 0xD075, (~0xEFFF) & 0xffff, 0, 0x0000);
    rc = rc ?: phy_dev_c45_write_mask(lan_phy_dev, 1, 0xD075, (~0xEFFF) & 0xffff, 0, 0x1000);
    rc = rc ?: phy_dev_c45_write_mask(lan_phy_dev, 1, 0xD072, (~0xEFFF) & 0xffff, 0, 0x1000);
    return rc;
}

 
/* there are four mappings, different per vi script */
/* epon_*_ea are not in use with pon_drv/impl2.  only mpcs_*_ae */

/* onu_synce_mtie_ctrl.vi */
static int serdes_wan_type_to_pon_mode1(serdes_wan_type_t wan_type)
{
    static int map[] = {
        [SERDES_WAN_TYPE_GPON] = 2,
        [SERDES_WAN_TYPE_EPON_1G] = 3,
        [SERDES_WAN_TYPE_EPON_2G] = 2,
        [SERDES_WAN_TYPE_AE] = 11,
        [SERDES_WAN_TYPE_AE_2_5G] = 10,
        [SERDES_WAN_TYPE_AE_10G] = 8,
        [SERDES_WAN_TYPE_EPON_10G_SYM] = 0,
        [SERDES_WAN_TYPE_EPON_10G_ASYM] = 1,
        [SERDES_WAN_TYPE_XGPON_10G_2_5G] = 1,
        [SERDES_WAN_TYPE_NGPON_10G_10G] = 0,
        [SERDES_WAN_TYPE_AE_5G] = 9,
    };

    return map[wan_type];
}

/* onu_synce_fcomp_ctrl.vi onu_synce_lf_ctrl.vi */
static int serdes_wan_type_to_pon_mode2(serdes_wan_type_t wan_type)
{
    static int map[] = {
        [SERDES_WAN_TYPE_GPON] = 6,
        [SERDES_WAN_TYPE_EPON_1G] = 3,
        [SERDES_WAN_TYPE_EPON_2G] = 2,
        [SERDES_WAN_TYPE_AE] = 14,
        [SERDES_WAN_TYPE_AE_2_5G] = 13,
        [SERDES_WAN_TYPE_AE_10G] = 11,
        [SERDES_WAN_TYPE_EPON_10G_SYM] = 0,
        [SERDES_WAN_TYPE_EPON_10G_ASYM] = 1,
        [SERDES_WAN_TYPE_XGPON_10G_2_5G] = 5,
        [SERDES_WAN_TYPE_NGPON_10G_10G] = 4,
        [SERDES_WAN_TYPE_AE_5G] = 12,
    };

    return map[wan_type];
}

/* onu_synce_mtie_ctrl.vi */
static int serdes_wan_type_to_pon_mode3(serdes_wan_type_t wan_type)
{
    static int map[] = {
        [SERDES_WAN_TYPE_GPON] = 0,
        [SERDES_WAN_TYPE_EPON_1G] = 1,
        [SERDES_WAN_TYPE_EPON_2G] = 1,
        [SERDES_WAN_TYPE_AE] = 1,
        [SERDES_WAN_TYPE_AE_2_5G] = 1,
        [SERDES_WAN_TYPE_AE_10G] = 1,
        [SERDES_WAN_TYPE_EPON_10G_SYM] = 1,
        [SERDES_WAN_TYPE_EPON_10G_ASYM] = 1,
        [SERDES_WAN_TYPE_XGPON_10G_2_5G] = 0,
        [SERDES_WAN_TYPE_NGPON_10G_10G] = 0,
        [SERDES_WAN_TYPE_AE_5G] = 1,
    };

    return map[wan_type];
}

/* onu_synce_nco_ctrl.job */
static int serdes_wan_type_to_auto_hold_sel(serdes_wan_type_t wan_type)
{
    static int map[] = {
        [SERDES_WAN_TYPE_GPON] = 1,
        [SERDES_WAN_TYPE_EPON_1G] = 3,
        [SERDES_WAN_TYPE_EPON_2G] = 3,
        [SERDES_WAN_TYPE_AE] = 6,
        [SERDES_WAN_TYPE_AE_2_5G] = 6,
        [SERDES_WAN_TYPE_AE_10G] = 6,
        [SERDES_WAN_TYPE_EPON_10G_SYM] = 3,
        [SERDES_WAN_TYPE_EPON_10G_ASYM] = 3,
        [SERDES_WAN_TYPE_XGPON_10G_2_5G] = 2,
        [SERDES_WAN_TYPE_NGPON_10G_10G] = 2,
        [SERDES_WAN_TYPE_AE_5G] = 6,
    };

    return map[wan_type];
}

static int serdes_wan_type_to_nco_auto_hold_inv(serdes_wan_type_t wan_type)
{
    static int map[] = {
        [SERDES_WAN_TYPE_GPON] = 1,
        [SERDES_WAN_TYPE_EPON_1G] = 0,
        [SERDES_WAN_TYPE_EPON_2G] = 0,
        [SERDES_WAN_TYPE_AE] = 1,
        [SERDES_WAN_TYPE_AE_2_5G] = 1,
        [SERDES_WAN_TYPE_AE_10G] = 1,
        [SERDES_WAN_TYPE_EPON_10G_SYM] = 0,
        [SERDES_WAN_TYPE_EPON_10G_ASYM] = 0,
        [SERDES_WAN_TYPE_XGPON_10G_2_5G] = 1,
        [SERDES_WAN_TYPE_NGPON_10G_10G] = 1,
        [SERDES_WAN_TYPE_AE_5G] = 1,
    };

    return map[wan_type];
}

static int is_epon(serdes_wan_type_t wan_type)
{
    return (wan_type == SERDES_WAN_TYPE_EPON_1G) || (wan_type == SERDES_WAN_TYPE_EPON_2G)
        || (wan_type == SERDES_WAN_TYPE_EPON_10G_SYM) || (wan_type == SERDES_WAN_TYPE_EPON_10G_ASYM);
}

int is_serdes_vi_pon_type_mpcs(serdes_wan_type_t wan_type)
{
    return wan_type == SERDES_WAN_TYPE_AE || wan_type == SERDES_WAN_TYPE_AE_2_5G || wan_type == SERDES_WAN_TYPE_AE_5G || wan_type == SERDES_WAN_TYPE_AE_10G;
}

/* calls to pon_drv via bcmFun */

/* cbptr is a pointer to typeof send_message_to_pon_drv_task */
typedef int (*cbptr)(pon_drv_msg_type t, ...);

static cbptr __send_message_to_pon_drv_task(void)
{
    bcmFun_t *cb = bcmFun_get(BCM_FUN_ID_SEND_MESSAGE_TO_PON_DRV_TASK);
    BCM_ASSERT(cb);
    return (cbptr)cb;
}
#define send_message_to_pon_drv_task(...) (__send_message_to_pon_drv_task())(__VA_ARGS__)

static int _wan_serdes_type_get(void)
{
    bcmFun_t *cb = bcmFun_get(BCM_FUN_ID_WAN_SERDES_TYPE_GET);
    BCM_ASSERT(cb);
    return cb(NULL);
}
#define wan_serdes_type_get _wan_serdes_type_get

static int _nco_sw_hold_val_get(void)
{
    bcmFun_t *cb = bcmFun_get(BCM_FUN_ID_NCO_SW_HOLD_VAL_GET);
    BCM_ASSERT(cb);
    return cb(NULL);
}
#define nco_sw_hold_val_get _nco_sw_hold_val_get

static int _netlink_invoke_serdes_job_with_output(const char *msg, uint32_t *results, uint64_t job_timeout_ns)
{
    int (*cb)(const char *, uint32_t *, uint64_t) = (int (*)(const char *, uint32_t *, uint64_t))bcmFun_get(BCM_FUN_ID_NETLINK_INVOKE_SERDES_JOB_WITH_OUTPUT);
    BCM_ASSERT(cb);
    return cb(msg, results, job_timeout_ns);
}
#define netlink_invoke_serdes_job_with_output _netlink_invoke_serdes_job_with_output

static int _netlink_invoke_serdes_job(const char *job)
{
    int (*cb)(const char *) = (int (*)(const char *))bcmFun_get(BCM_FUN_ID_NETLINK_INVOKE_SERDES_JOB);
    BCM_ASSERT(cb);
    return cb(job);
}
#define netlink_invoke_serdes_job _netlink_invoke_serdes_job


/* state machine */

static inline sm_state get_sm_state(void)
{
    if (unlikely(ILLEGAL_STATE(data.current_state)))
    {
        PRINTK("unexpected: sm in illegal state==%d", data.current_state);
        dump_stack();
    }
    return data.current_state;
}

static inline void set_sm_state(const sm_state s)
{
    unsigned long flags;
    sm_state current_state = data.current_state;

    if (s == current_state)
        return;
    if (unlikely(ILLEGAL_STATE(s)))
    {
        PRINTK("unexpected: sm moving to an illegal state==%d", s);
        dump_stack();
    }
    DBG_PRINTK("sm state change %d(%s)-->%d(%s) jiffies=%lu", current_state, STATE_NAME(current_state), s, STATE_NAME(s), jiffies);
    spin_lock_irqsave(&data.lock_sm, flags);
    data.current_state = s;
    data.jiffies_entering_state = jiffies;
    spin_unlock_irqrestore(&data.lock_sm, flags);
}


/* init SyncE registers and ucode */

#define WAN_TOP_WAN_MISC_SYNCE_OFFSET                                    0x000F4
#define SGB_TOP_16NM_SGB_TOP_SYNCE_EN_OFFSET                             0x0C440
#define SGB_TOP_16NM_SGB_TOP_SYNCE_PHASE_COMP_LPF_ALU_RAM_ACC_OFFSET     0x0C4B4
#define SGB_TOP_16NM_SGB_TOP_SYNCE_PHASE_COMP_LPF_ALU_SM_CTL_OFFSET      0x0C4B8
#define SGB_TOP_16NM_SGB_TOP_SYNCE_PHASE_COMP_CTL_OFFSET                 0x0C460
#define SGB_TOP_SYNCE_PHASE_COMP_DET_DELTA_AVE                           0x0C5BC
#define SGB_TOP_16NM_SGB_TOP_SYNCE_PHASE_COMP_LPF_ALU_RAM_RD_DATA_OFFSET 0x0C5d4

static void init_synce_sequencer(void)
{
    /* will reset ucode memory */
    synce_clear_reg(WAN_TOP_WAN_MISC_SYNCE_OFFSET, 0, 3);
    synce_clear_reg(SGB_TOP_16NM_SGB_TOP_SYNCE_EN_OFFSET, 0, 1);
    synce_clear_reg(SGB_TOP_16NM_SGB_TOP_SYNCE_PHASE_COMP_LPF_ALU_RAM_ACC_OFFSET, 31, 1);
    synce_clear_reg(SGB_TOP_16NM_SGB_TOP_SYNCE_PHASE_COMP_LPF_ALU_RAM_ACC_OFFSET, 30, 1);
    synce_clear_reg(SGB_TOP_16NM_SGB_TOP_SYNCE_PHASE_COMP_LPF_ALU_SM_CTL_OFFSET, 15, 1);
    synce_or_reg(SGB_TOP_16NM_SGB_TOP_SYNCE_PHASE_COMP_CTL_OFFSET, 0, 1, 1);
    synce_or_reg(SGB_TOP_16NM_SGB_TOP_SYNCE_PHASE_COMP_CTL_OFFSET, 20, 1, 1);
    synce_clear_reg(SGB_TOP_16NM_SGB_TOP_SYNCE_PHASE_COMP_LPF_ALU_SM_CTL_OFFSET, 16, 11);
    synce_or_reg(SGB_TOP_16NM_SGB_TOP_SYNCE_PHASE_COMP_LPF_ALU_SM_CTL_OFFSET, 15, 1, 1);
    synce_or_reg(SGB_TOP_16NM_SGB_TOP_SYNCE_EN_OFFSET, 0, 1, 1);
    synce_clear_reg(SGB_TOP_16NM_SGB_TOP_SYNCE_PHASE_COMP_LPF_ALU_SM_CTL_OFFSET, 15, 1);
}

#define DELAY_US 10
static const uint8_t synce_ucode[];
static const int sizeof_synce_ucode;

void init_synce_ucode(void)
{
    volatile uint32_t reg;
    uint32_t i, j, err_cnt = 0, read_loop = 1;

    for (i = 0; i < sizeof_synce_ucode; i++)
    {
        reg = (1 << 30) | (synce_ucode[i] << 16) | i;
        SYNCE_WRITE_32(SGB_TOP_16NM_SGB_TOP_SYNCE_PHASE_COMP_LPF_ALU_RAM_ACC_OFFSET, reg);
        udelay(DELAY_US);
    }

    read_loop = 2; /* repeated readloop required for 968880B0 and 6837 */
    for (i = 0; i < sizeof_synce_ucode; i++)
    {
        /* read multiple times because a hw bug in sgb_synce_pcomp_lpf_alu_up_ram_rd_get signal */
        for (j = 0; j < read_loop; j++)
        {
            reg = (1 << 31) | i;
            SYNCE_WRITE_32(SGB_TOP_16NM_SGB_TOP_SYNCE_PHASE_COMP_LPF_ALU_RAM_ACC_OFFSET, reg);
            udelay(DELAY_US);
        }
        SYNCE_READ_32(SGB_TOP_16NM_SGB_TOP_SYNCE_PHASE_COMP_LPF_ALU_RAM_RD_DATA_OFFSET, reg);
        udelay(DELAY_US);
        err_cnt += ((reg & 0xFF) != synce_ucode[i]);
    }
    if (err_cnt)
        PRINTK("unexpected err_cnt==%d\n", err_cnt);
}


/* SyncE flowchart from Edmonton SyncE VI scripts sheet, Da Xia.
   grouped in blocks A..D, steps (sub blocks or boxes) named after their vi_script when possible.
   runs in non-atomic context.  */

#define invoke_job_with_output(fname_format, output, ...)                                                       \
({                                                                                                              \
    char msg[NETLINK_MSG_MAX_LEN] = "";                                                                         \
    BCM_ASSERT(snprintf(msg, NETLINK_MSG_MAX_LEN, fname_format, ## __VA_ARGS__) < sizeof(msg));                 \
    netlink_invoke_serdes_job_with_output(msg, (uint32_t *)output, 3000000000ULL);                              \
})

#define invoke_job(fname_format, ...)                                                                           \
({                                                                                                              \
    char msg[NETLINK_MSG_MAX_LEN] = "";                                                                         \
    BCM_ASSERT(snprintf(msg, NETLINK_MSG_MAX_LEN, fname_format, ## __VA_ARGS__) < sizeof(msg));                 \
    netlink_invoke_serdes_job(msg);                                                                             \
})

void onu_synce_reset(void)
{
    serdes_wan_type_t wan_type = wan_serdes_type_get();

    if (!((wan_type > 0) && (wan_type < SERDES_WAN_TYPE_MAX) && (wan_type != SERDES_WAN_TYPE_AE_2_5G_R)
        && (wan_type != SERDES_WAN_TYPE_NGPON_10G_10G_8B10B) && (wan_type != SERDES_WAN_TYPE_NGPON_10G_2_5G_8B10B)
        && (wan_type != SERDES_WAN_TYPE_NGPON_10G_2_5G)))
            PRINTK("invalid wan_type==%d\n", wan_type);
    data.serdes_wan_type = wan_type;
    init_synce_sequencer();
    init_synce_ucode();
    merlin_enable_synce(data.lan_phy_dev);
}


static void block_A(void)
{
    struct __attribute__((packed, aligned(4))) {
        uint32_t tx_pi_bucket_get_int;
        uint32_t tx_pi_bucket_under_int;
        uint32_t tx_pi_bucket_over_int;
        uint32_t tx_pi_bucket;
        uint32_t tx_pi_bucket_max;
        uint32_t tx_pi_bucket_min;
    } job_res;

    if (!(data.merlin_up && (data.eth_mode >= 0)) || !data.pon_up)
        return;

    set_sm_state(sm_a_w5);
    onu_synce_reset();
    invoke_job("onu_synce_nco_ctrl.job %d %d %d %d %d %d %d %d", 1, NCO_FRACN_NUM, NCO_FRACN_DEN, NCO_COMP_VAL, nco_sw_hold_val_get(),
        NCO_UPDATE_MAX, serdes_wan_type_to_auto_hold_sel(data.serdes_wan_type), serdes_wan_type_to_nco_auto_hold_inv(data.serdes_wan_type));invoke_job("onu_synce_nco_go_stop.job %d %d", 1, 0);
    invoke_job("onu_merlin_pi_stepper_ctrl.job %d %d", 1, 0b000);
    invoke_job("onu_synce_lf_ctrl.job %d %d %d", serdes_wan_type_to_pon_mode2(data.serdes_wan_type), 1, data.eth_mode);
    invoke_job("onu_merlin_pi_stepper_ctrl.job %d %d", 1, 0b001);
    invoke_job("onu_merlin_pi_stepper_ctrl.job %d %d", 1, 0b111);
    invoke_job("onu_synce_mtie_ctrl.job %d %d %d %d %d %d %d %d %d", serdes_wan_type_to_pon_mode3(data.serdes_wan_type),
        serdes_wan_type_to_pon_mode1(data.serdes_wan_type), data.eth_mode, 1, 1, 0, 0, 0, 0);
    invoke_job_with_output("onu_merlin_pi_stepper_bucket_read.job", &job_res);
    if (job_res.tx_pi_bucket_get_int || job_res.tx_pi_bucket_under_int || job_res.tx_pi_bucket_over_int)
    {
        PRINTK("onu_merlin_pi_stepper_bucket_read returned tx_pi_bucket_get_int=%d, tx_pi_bucket_under_int=%d, "
            "tx_pi_bucket_over_int=%d, tx_pi_bucket=%d, tx_pi_bucket_max=%d, tx_pi_bucket_min=%d",
            job_res.tx_pi_bucket_get_int, job_res.tx_pi_bucket_under_int, job_res.tx_pi_bucket_over_int,
            job_res.tx_pi_bucket, job_res.tx_pi_bucket_max, job_res.tx_pi_bucket_min);
    }
    if (mod_timer(&data.synce_holdover_timer_A5, jiffies + msecs_to_jiffies(5*1000)))
    {
        PRINTK("Unable to start synce_holdover timer A5");
    }
}

static void block_B(void)
{
    invoke_job("onu_merlin_pi_stepper_accum_int_clr.job");
    invoke_job("onu_merlin_pi_stepper_accum_go.job %d", data.eth_mode);
    if (mod_timer(&data.synce_holdover_timer_B1, jiffies + msecs_to_jiffies(1*1000)))
    {
        PRINTK("Unable to start synce_holdover timer B1");
    }
}


static void calc_nco_holdover_value(int32_t lf_pi_step_accum, int32_t mtie_pi_step_accum, int32_t *new_nco_sw_hold_value)
{
    int64_t total_pi_accum_step;
    const uint32_t nco_ratio = NCO_FRACN_DEN / NCO_FRACN_NUM, tx_pi_accum_secs = 1,
        tx_pi_accum_clks_per_sec = (data.eth_mode == 2) ? 625000000 : 515625000;
    uint32_t eth_interval;

    total_pi_accum_step = lf_pi_step_accum + mtie_pi_step_accum;
    eth_interval = (tx_pi_accum_secs & 0x3FF) * tx_pi_accum_clks_per_sec;
    *new_nco_sw_hold_value = (total_pi_accum_step * NCO_UPDATE_MAX * nco_ratio + (eth_interval >> 1)) / eth_interval;
    DBG_PRINTK("nco_ratio=0x%x", nco_ratio);
    DBG_PRINTK("total_pi_accum_step=0x%llx", total_pi_accum_step);
    DBG_PRINTK("eth_interval=0x%x", eth_interval);
    DBG_PRINTK("new_nco_sw_hold_value=0x%x", *new_nco_sw_hold_value);
}

static struct workqueue_struct *synce_workqueue; /* for block_C, work in this Q takes 1000ms, every 5000ms */

static struct {
    struct work_struct work; /* no parameters for the moment */
} block_C_work;

static void block_C_work_handler(struct work_struct *work)
{
    struct __attribute__((packed, aligned(4))) {
        int32_t pi_accum_get;
        int32_t lf_pi_step_accum;
        int32_t mtie_pi_step_accum;
        int32_t nco_pi_step_accum;
    } job_res;
    int32_t new_nco_sw_hold_value;

    do {
        msleep(200);
        invoke_job_with_output("onu_merlin_pi_stepper_accum_rd.job", &job_res);
    } while (!job_res.pi_accum_get);
    calc_nco_holdover_value(job_res.lf_pi_step_accum, job_res.mtie_pi_step_accum, &new_nco_sw_hold_value);
    DBG_PRINTK("new_nco_sw_hold_value=0x%x", new_nco_sw_hold_value);
    /* B+C takes 6000ms, if wan linkdown occurs in the last 100ms, then the new nco value is bad, based on measurements
       after wan linkdown, and setting it will actually ruin holdover */
    if (data.merlin_up && data.pon_up) /* we wouldn't be here unless eth_mode must be >= 0 */
    {
        invoke_job("onu_synce_nco_set_sw_hold_val.job %d", new_nco_sw_hold_value);
        if (mod_timer(&data.synce_holdover_timer_C5, jiffies + msecs_to_jiffies(5*1000)))
        {
            PRINTK("Unable to start synce_holdover timer C5");
        }
    }
}


static void block_C(void)
{
    queue_work(synce_workqueue, (struct work_struct *)&block_C_work);
}

static void block_D(void)
{
    invoke_job("onu_synce_lf_ctrl.job %d %d %d", serdes_wan_type_to_pon_mode2(data.serdes_wan_type), 1, data.eth_mode);
    invoke_job("onu_synce_mtie_ctrl.job %d %d %d %d %d %d %d %d %d", serdes_wan_type_to_pon_mode3(data.serdes_wan_type),
        serdes_wan_type_to_pon_mode1(data.serdes_wan_type), data.eth_mode, 0, 1, 0, 0, 0, 0);
}


/* sm event handlers, run in non-atomic context, called by pon_drv_task, invoke blocks */

static void synce_holdover_wan_linkup_handler(void)
{
    sm_state s;

    DBG_PRINTK("%s: jiffies=%lu", __FUNCTION__, jiffies);
    s = get_sm_state();
    switch (s) {
        case sm_holdover:
            block_D();
            __attribute__((__fallthrough__));
        case sm_no_sync:
            block_A();
            break;

        case sm_a_w5:
            DBG_PRINTK("ignoring transition to state==%d(%s) in state==%d (%s), likely spurious LODS/DS_SYNC during DS_SYNC processing", s, STATE_NAME(s), s, STATE_NAME(s));
            break;

        default:
            PRINTK("unexpected: default in switch, state==%d (%s)", s, STATE_NAME(s));
            dump_stack();
            break;
    }
}

static struct {
    struct delayed_work work; /* no parameters for the moment */
} ae_linkup_work;

static void ae_linkup_work_handler(struct work_struct *work)
{
    synce_holdover_wan_linkup_handler();
}

static void synce_holdover_ae_linkup_handler(void)
{
    schedule_delayed_work((struct delayed_work *)&ae_linkup_work,
        msecs_to_jiffies(is_epon(wan_serdes_type_get()) ? 15000 : 1000));
}

static void synce_holdover_wan_linkdown_handler(void)
{
    sm_state s;

    DBG_PRINTK("%s: jiffies=%lu", __FUNCTION__, jiffies);
    s = get_sm_state();
    switch (s) {
        case sm_a_w5:
            set_sm_state(sm_no_sync);
            del_timer(&data.synce_holdover_timer_A5);
            break;

        case sm_b_w1:
            set_sm_state(sm_holdover);
            del_timer(&data.synce_holdover_timer_B1);
            PRINTK("SYNCE: WAN clock lost at jiffies=%lu.", jiffies);
            break;

        case sm_c_w5:
            set_sm_state(sm_holdover);
            del_timer(&data.synce_holdover_timer_C5);
            PRINTK("SYNCE: WAN clock lost at jiffies=%lu.", jiffies);
            data.eth_clk_good = FALSE;
            break;

        case sm_no_sync:
            if (!is_epon(wan_serdes_type_get()) /* sometimes we see linkdown b4 linkup in EPON during boot */
                && (data.merlin_up && (data.eth_mode >= 0)))
                    PRINTK("unexpected: LODS in state==%d (%s) and merlin_up, eth_mode==%d", s, STATE_NAME(s), data.eth_mode);
            break;

        default:
            PRINTK("unexpected: default in switch, state==%d (%s)", s, STATE_NAME(s));
            dump_stack();
            break;
    }
}

static void synce_holdover_timer_A5_handler(void)
{
    sm_state s;
    unsigned long time_in_a_w5;
    volatile uint32_t reg;

    time_in_a_w5 = data.jiffies_entering_state + msecs_to_jiffies(5*1000);
    if (unlikely(((s = get_sm_state()) != sm_a_w5)))
    {
        PRINTK("unexpected: timer A5 expired in state==%d (%s)", s, STATE_NAME(s));
    }
    else
    {
        if (!time_after(jiffies, time_in_a_w5))
        {
            PRINTK("unexpected: timer A5 expired after less than 5s in state sm_a_w5");
        }
        SYNCE_READ_32(SGB_TOP_SYNCE_PHASE_COMP_DET_DELTA_AVE, reg);
        if (unlikely(reg))
        {
            PRINTK("unexpected: SGB_TOP_SYNCE_PHASE_COMP_DET_DELTA_AVE==%d", reg);
        }
        else
        {
            DBG_PRINTK("SYNCE: Eth clock good to go at jiffies=%lu.", jiffies);
        }
        data.eth_clk_good = TRUE;
        set_sm_state(sm_b_w1);
        block_B();
    }
}

static void synce_holdover_timer_B1_handler(void)
{
    sm_state s;

    if (unlikely(((s = get_sm_state()) != sm_b_w1)))
    {
        PRINTK("unexpected: timer B1 expired in state==%d (%s)", s, STATE_NAME(s));
    }
    else
    {
        set_sm_state(sm_c_w5);
        block_C();
    }
}

static void synce_holdover_timer_C5_handler(void)
{
    sm_state s;

    if (likely(((s = get_sm_state()) == sm_c_w5)))
    {   /* true unless there was a disconnect and the timer expired */
        set_sm_state(sm_b_w1);
        block_B();
    }
}


/* sm timers callbacks */

void synce_holdover_timer_A5_callback(struct timer_list *timer)
{
    sm_state s;

    if (unlikely(((s = get_sm_state()) != sm_a_w5)))
    {
        PRINTK("unexpected: timer A5 expired in state==%d (%s)", s, STATE_NAME(s));
    }
    else
    {
        send_message_to_pon_drv_task(msg_call_synce_timer_A5_handler);
    }
}

void synce_holdover_timer_B1_callback(struct timer_list *timer)
{
    sm_state s;

    if (unlikely(((s = get_sm_state()) != sm_b_w1)))
    {
        PRINTK("unexpected: timer B1 expired in state==%d (%s)", s, STATE_NAME(s));
    }
    else
    {
        send_message_to_pon_drv_task(msg_call_synce_timer_B1_handler);
    }
}

void synce_holdover_timer_C5_callback(struct timer_list *timer)
{
    sm_state s;

    if (unlikely(((s = get_sm_state()) != sm_c_w5)))
    {
        PRINTK("unexpected: timer C5 expired in state==%d (%s)", s, STATE_NAME(s));
    }
    else
    {
        send_message_to_pon_drv_task(msg_call_synce_timer_C5_handler);
    }
}


void synce_holdover_eth_up_handler(void)
{
    sm_state s;

    s = get_sm_state();
    if (s != sm_no_sync)
        PRINTK("unexpected: merlin_up in state==%d (%s)", s, STATE_NAME(s));
    block_A();
}

void synce_holdover_eth_down_handler(void)
{
    sm_state s;

    s = get_sm_state();
    del_timer(&data.synce_holdover_timer_A5);
    del_timer(&data.synce_holdover_timer_B1);
    del_timer(&data.synce_holdover_timer_C5);
    set_sm_state(sm_no_sync);
    DBG_PRINTK("merlin_down in state==%d (%s)", s, STATE_NAME(s));
}


/* atomic handlers, called from [n]gpon_isr */

void synce_holdover_wan_linkup_atomic_handler(void)
{
    if (!data.initialized)
        return;
    data.nums_pon_sync_events++;
    data.pon_up = TRUE;
    send_message_to_pon_drv_task(msg_call_synce_wan_linkup_handler);
}
EXPORT_SYMBOL(synce_holdover_wan_linkup_atomic_handler);

void synce_holdover_ae_linkup_atomic_handler(void)
{
    if (!data.initialized)
        return;
    data.nums_pon_sync_events++;
    data.pon_up = TRUE;
    send_message_to_pon_drv_task(msg_call_synce_ae_linkup_handler);
}
EXPORT_SYMBOL(synce_holdover_ae_linkup_atomic_handler);

void synce_holdover_wan_linkdown_atomic_handler(void)
{
    if (!data.initialized)
        return;
    data.nums_pon_sync_events++;
    data.pon_up = FALSE;
    send_message_to_pon_drv_task(msg_call_synce_wan_linkdown_handler);
}
EXPORT_SYMBOL(synce_holdover_wan_linkdown_atomic_handler);

/* atomic, called by phy_link_change_cb() */

static void synce_holdover_set_eth_mode(phy_dev_t *lan_phy_dev, const int merlin_linkup, int speed)
{
    switch (speed)
    {
        case PHY_SPEED_10000:
        case PHY_SPEED_2500:  /* since 2.5Gbps is also using 10.3GHz VCO */
            data.eth_mode = 0;
            break;
        case PHY_SPEED_5000:
            data.eth_mode = 1;
            break;
        default:
            data.eth_mode = -1; /* link_down or (link_up < 2.5GbE) */
            if (merlin_linkup)
            {
                DBG_PRINTK("SyncE doesn't support PHY_SPEED=%d", speed);
            }
    }
    data.lan_phy_dev = merlin_linkup ? lan_phy_dev : NULL;
    DBG_PRINTK("merlin link %s, eth_mode==%d", merlin_linkup ? "up" : "down", data.eth_mode);
    send_message_to_pon_drv_task(merlin_linkup ? msg_call_synce_eth_up_handler : msg_call_synce_eth_down_handler);
}

static void synce_holdover_eth_link_change_cb(phy_dev_t *phy_dev)
{
    uint32_t caps = 0;
    phy_dev_t *first = cascade_phy_get_first(phy_dev);

    if (!data.initialized)
        return;
    phy_dev_caps_get(first, CAPS_TYPE_SUPPORTED, &caps);
    if (caps & PHY_CAP_SYNCE)
    {
        data.merlin_up = phy_dev->link;
        DBG_PRINTK("Merlin LAN link %s\n", phy_dev->link ? "up" : "down");
        synce_holdover_set_eth_mode(phy_dev->cascade_prev ? phy_dev->cascade_prev : phy_dev, phy_dev->link, phy_dev->speed);
    }

    if (first->phy_drv->phy_type == PHY_TYPE_WAN_AE)
    {
        DBG_PRINTK("WAN AE link %s\n", phy_dev->link ? "up" : "down");
        if (phy_dev->link)
            synce_holdover_ae_linkup_atomic_handler();
        else
            synce_holdover_wan_linkdown_atomic_handler();
    }
}


/* mostly generic stuff below */

/* init */

static struct synce_holdover_data *synce_holdover_data_init(struct device *dev)
{
    struct synce_holdover_data *psynce_holdover = &data;

    *psynce_holdover = (struct synce_holdover_data) { 0 };
    bcmFun_reg(BCM_FUN_ID_SYNCE_ETH_LINK_CHANGE, (bcmFun_t *)synce_holdover_eth_link_change_cb);
    DBG_PRINTK("%s.%d registered cb synce_holdover_eth_link_change_cb()\n", __FUNCTION__, __LINE__);

#if defined(CONFIG_BRCM_SMC_BOOT)
    if (bcm_license_check_msg(BP3_FEATURE_SYNCE) <= 0)
        return psynce_holdover;
#endif

    psynce_holdover->dev = dev;
    psynce_holdover->eth_clk_good = FALSE;
    psynce_holdover->current_state = sm_no_sync;
    psynce_holdover->jiffies_entering_state = jiffies;
    spin_lock_init(&psynce_holdover->lock_sm);
    mutex_init(&psynce_holdover->st_mutex);
    synce_workqueue = create_workqueue("SYNCE");
    INIT_WORK((struct work_struct *)&block_C_work, block_C_work_handler);
    INIT_DELAYED_WORK((struct delayed_work *)&ae_linkup_work, ae_linkup_work_handler);
    timer_setup(&psynce_holdover->synce_holdover_timer_A5, synce_holdover_timer_A5_callback, 0);
    timer_setup(&psynce_holdover->synce_holdover_timer_B1, synce_holdover_timer_B1_callback, 0);
    timer_setup(&psynce_holdover->synce_holdover_timer_C5, synce_holdover_timer_C5_callback, 0);
    psynce_holdover->eth_mode = -1;
    psynce_holdover->merlin_up = FALSE; /* this driver loads before net but after pon_drv */
    send_message_to_pon_drv_task(msg_synce_register_handlers, synce_holdover_wan_linkup_handler, synce_holdover_ae_linkup_handler, synce_holdover_wan_linkdown_handler, synce_holdover_timer_A5_handler,
        synce_holdover_timer_B1_handler, synce_holdover_timer_C5_handler, synce_holdover_eth_up_handler, synce_holdover_eth_down_handler);
    psynce_holdover->pon_up = FALSE;
    psynce_holdover->serdes_wan_type = 0;
    psynce_holdover->nums_pon_sync_events = 0;
    psynce_holdover->initialized = TRUE;
    return psynce_holdover;
}

static const struct of_device_id of_platform_synce_holdover_table[] = {
    { .compatible = "brcm,synce_holdover", },
    { /* end of list */ },
};

static int _probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct device_node *np = dev->of_node;
    struct synce_holdover_data *psynce_holdover;
    const struct of_device_id *of_id;
    int ret = 0;

    if (!np)
        return -ENODEV;

    if (IS_ERR(synce_virt_address = of_iomap(np, 0)))
        return -ENODEV;

    psynce_holdover = synce_holdover_data_init(dev);

    platform_set_drvdata(pdev, psynce_holdover);

    of_id = of_match_node(of_platform_synce_holdover_table, np);
    if (WARN_ON(!of_id))
        return -EINVAL;

    return ret;
}

struct platform_driver of_platform_synce_holdover_driver = {
    .driver = {
        .name = "brcm_synce_holdover",
        .of_match_table = of_platform_synce_holdover_table,
    },
    .probe = _probe,
};

static int probe_init(void)
{
    struct synce_holdover_data *psynce_holdover;
    struct device_node *np = of_find_compatible_node(NULL, NULL, "brcm,synce_holdover");

    if (!np)
    {
        printk(KERN_ERR "%s: failed to find DT CFG\n", __FUNCTION__);
        return -ENODEV;
    }

    if (IS_ERR(synce_virt_address = of_iomap(np, 0)))
    {
        printk("%s: of_iomap failed ret=%ld\n", __FUNCTION__, PTR_ERR(synce_virt_address));
        return -ENODEV;
    }

    psynce_holdover = synce_holdover_data_init(NULL);
    return 0;
}

static int __init detect_init(void)
{
    return probe_init();
}
module_init(detect_init);

static void __exit detect_exit(void)
{
    data.initialized = 0;
    PRINTK("synce_holdover driver unloading.");
}
module_exit(detect_exit);

/* as received from Assaf on 24/12/21 */
static const uint8_t synce_ucode[] = {
3, 25, 97, 27, 98, 18, 129, 28, 131, 20, 161, 0, 24, 113, 26, 116,
19, 64, 37, 65, 16, 145, 21, 166, 39, 97, 31, 98, 18, 112, 120, 19,
64, 38, 67, 16, 160, 24, 96, 27, 98, 18, 112, 28, 116, 19, 64, 23,
65, 16, 162, 39, 97, 32, 98, 18, 112, 120, 19, 160, 24, 96, 29, 98,
18, 112, 30, 116, 19, 64, 23, 65, 16, 163, 39, 97, 33, 98, 18, 129,
34, 131, 20, 167, 1, 24, 113, 26, 116, 19, 64, 37, 65, 16, 145, 21,
166, 39, 97, 31, 98, 18, 112, 120, 19, 64, 38, 67, 16, 160, 24, 96,
27, 98, 18, 112, 28, 116, 19, 64, 23, 65, 16, 162, 39, 97, 32, 98,
18, 112, 120, 19, 160, 24, 96, 29, 98, 18, 112, 30, 116, 19, 64, 23,
65, 16, 163, 39, 97, 33, 98, 18, 129, 34, 131, 20, 167, 2, 24, 113,
26, 116, 19, 64, 37, 65, 16, 145, 21, 166, 39, 97, 31, 98, 18, 112,
120, 19, 64, 38, 67, 16, 160, 24, 96, 27, 98, 18, 112, 28, 116, 19,
64, 23, 65, 16, 162, 39, 97, 32, 98, 18, 112, 120, 19, 160, 24, 96,
29, 98, 18, 112, 30, 116, 19, 64, 23, 65, 16, 163, 39, 97, 33, 98,
18, 129, 34, 131, 20, 167, 3, 24, 96, 29, 98, 18, 112, 30, 116, 19,
64, 40, 67, 16, 64, 41, 67, 16, 64, 42, 65, 16, 129, 34, 131, 20,
80, 59, 81, 17, 161, 4, 24, 80, 57, 81, 17, 161, 24, 97, 45, 98,
18, 178, 24, 64, 46, 66, 16, 176, 129, 48, 130, 20, 80, 59, 81, 17,
97, 47, 98, 18, 112, 48, 117, 19, 64, 56, 67, 16, 178, 24, 80, 50,
82, 17, 97, 51, 98, 24, 177, 18, 64, 56, 66, 16, 145, 21, 178, 97,
52, 98, 18, 160, 5, 43, 112, 44, 115, 19, 161, 24, 97, 45, 98, 18,
178, 24, 64, 46, 66, 16, 176, 129, 48, 130, 20, 80, 59, 81, 17, 97,
47, 98, 18, 112, 48, 117, 19, 64, 58, 67, 16, 178, 24, 80, 50, 82,
17, 97, 51, 98, 24, 177, 18, 64, 58, 66, 16, 145, 21, 178, 97, 52,
98, 18, 112, 53, 117, 19, 161, 23, 64, 24, 67, 16, 180, 56, 179, 255,
};

static const int sizeof_synce_ucode = sizeof(synce_ucode) / sizeof(synce_ucode[0]);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("synce_holdover device driver");
MODULE_LICENSE("GPL");
