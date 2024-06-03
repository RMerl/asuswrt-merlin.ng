/*---------------------------------------------------------------------------

<:copyright-BRCM:2013:DUAL/GPL:standard 

   Copyright (c) 2013 Broadcom 
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
 ------------------------------------------------------------------------- */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/namei.h>
#include <linux/ioctl.h>
#include <laser.h>
#include <linux/spinlock.h>
#include <net_port.h>
#include <linux/bcm_log.h>
#include <linux/kthread.h>
#include <rdpa_api.h>
#include <rdpa_epon.h>
#include "blocks.h"
#include "clkrst_testif.h"
#include "ld_lia.h"
#include "lrl.h"
#include "pmd_msg.h"
#include "HostCommon.h"
#include "pmd_op.h"
#include "pmd_cal.h"
#include "pmd_temp_cal.h"
#include "pmd_data_col.h"
#include "ru.h"
#include "shared_utils.h"
#include "wan_drv.h"
#include "pmd.h"
#include "dev_id.h"
#include <linux/of_platform.h>
#include <linux/pinctrl/consumer.h>
#include <linux/gpio/consumer.h>
#include <bcm_bca_extintr.h>
#include <linux/i2c.h>
#include "pmc_core_api.h"
#include "clk_rst.h"
#include "trxbus.h"

#include <linux/bcm_log.h>


#define COPY_TO_USER(a, b, c) if(copy_to_user(a, b, c)){ printk("%s:%d: copy_to_user failed", __FUNCTION__, __LINE__ );}

/* PMD functionalities (first 4 bits are reserved for pon type)*/
#define PMD_APD_REG_DISABLED         (0x0 << 4)
#define PMD_APD_REG_ENABLED          (0x1 << 4)
#define PMD_APD_TYPE_FLYBACK         (0x0 << 5)
#define PMD_APD_TYPE_BOOST           (0x1 << 5)

/* character major device number */
#define PMD_DEV_MAJOR   314

struct pmd_data
{
    struct device *dev;
    struct gpio_desc *ewake;
    struct gpio_desc *rstn;
    int alarm_irq;
    unsigned short cap;
    struct pinctrl *pinctrl;
    struct pinctrl_state *pins_ewake;
    struct i2c_adapter *i2c;
};
static struct pmd_data *g_pdata;

static struct proc_dir_entry *pmd_msg_procfs_entry;
static struct proc_dir_entry *pmd_rst_procfs_entry = NULL;
static wait_queue_head_t pmd_log_wait;

uint16_t last_hmid_cal_set_argument;
#define NO_LAST_HOST_CAL_SET_MESSAGE (0xFFFF)
#define MAX_PMD_MESSAGE_LENGTH       (   128)
static char pmd_message_buffer[MAX_PMD_MESSAGE_LENGTH];
static pid_t pmd_messaging_session_pid;
static int pmd_messaging_sessions_counter;
static int is_pmd_message_ready;
static loff_t previous_session_read_whole_messages_byte_counter;
static uint32_t previous_session_overwritten_messages_counter, overwritten_pmd_messages_counter;
static loff_t previous_session_overwritten_bytes_counter, overwritten_pmd_bytes_counter;
static uint32_t previous_session_read_whole_messages_counter, read_pmd_whole_messages_counter;
static uint32_t previous_session_written_messages_counter, written_pmd_messages_counter;
static loff_t previous_session_written_bytes_counter, written_pmd_bytes_counter;
static uint32_t previous_session_read_fragmented_messages_counter, pmd_read_fragmented_messages_counter;
static loff_t previous_session_read_fragmented_bytes_counter, pmd_read_fragmented_bytes_counter;
static uint32_t previous_session_read_fragments_counter, pmd_read_fragments_counter;

/* mutex & semaphore */
static struct mutex pmd_dev_lock;
static spinlock_t exported_symbols_spinlock;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
static spinlock_t pon_message_bh_spinlock;
#endif
static volatile int exported_symbols_readers;
static volatile int exported_symbols_writers;

#define LOCK() mutex_lock(&pmd_dev_lock)
#define UNLOCK() mutex_unlock(&pmd_dev_lock)

#define RATE_STR_LEN 2

#define PMD_TIMER_INDICATION 0x1

/* periodic timer */
struct timer_list periodic_timer;
#define PMD_TIMER_INTERVAL 1000 /*once in a sec*/

static struct task_struct *pmd_timer_thread;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
static struct task_struct *pon_msg_event_thread;
static wait_queue_head_t pon_msg_event_wq;
static int __pon_msg_type_bit_mask;
#define PON_CHANGE_TRCK_STAT_INDICATION 0x1
#endif
static wait_queue_head_t pmd_thread_event;
static int pmd_thread_sched;

static int pmd_reset_mode = 0;
static uint16_t pmd_state;
static uint16_t pmd_prev_state;
static bool pmd_fb_done;
static uint8_t num_of_ranging_attempts;
static uint8_t temp_poll;
static uint8_t in_calibration;

static char* tracking_state_string[] = {"pmd_tracking_state_disabled", "pmd_tracking_state_tune",
    "pmd_tracking_state_first_burst", "pmd_tracking_state_tracking"};
static char* pmd_wan_type_string[] = {"PMD_EPON_1_1_WAN", "PMD_EPON_2_1_WAN", "PMD_GPON_2_1_WAN", "PMD_GBE_1_1_WAN",
    "PMD_XGPON1_10_2_WAN", "PMD_EPON_10_1_WAN", "PMD_AUTO_DETECT_WAN"};

#define CAL_ERROR_SIZE 25
pmd_cal_msg_str pmd_cal_ret[CAL_ERROR_SIZE] =  {{0,"done"},
                                                {1,"error"},
                                                {2,"invalid_calibration_procedure"},
                                                {32,"apd_disabled"},
                                                {33,"apd_overcurrent_condition"},
                                                {34,"apd_regulator_fault"},
                                                {35,"apd_cannot_attain_vbr"},
                                                {36,"apd_too_many_overcurrent_conditions"},
                                                {37,"apd_vbr_too_low"},
                                                {38,"apd_protection_circuit_fault"},
                                                {39,"apd_regulator_fault_after_protection_circuit_test"},
                                                {64,"transmitter_fault1"},
                                                {65,"transmitter_fault2"},
                                                {66,"transmitter_optical_power_too_low"},
                                                {67,"transmitter_modulation_fault"},
                                                {68,"zero_optical_power_measure"},
                                                {96,"mpd_current_too_large"},
                                                {97,"mpd_current_too_small"},
                                                {128,"eyesafety_thr_too_high"},
                                                {129,"eyesafety_thr_too_low"},
                                                {160,"rogue_thr_too_high"},
                                                {161,"rogue_thr_too_low"},
                                                {162,"rogue_thr_gap_too_low"},
                                                {192,"tracking_convergence_fault"},
                                                {224,"los_clear_alarm_not_guaranteed"}};

PMD_WAN_TYPES pmd_wan_type = PMD_AUTO_DETECT_WAN;
extern uint16_t cal_index;

#define PMD_TRACKING_DELTA_TEMP 2
static pon_set_pmd_fb_done_callback pon_set_pmd_fb_done;

static void pon_set_pmd_fb_done_wrapper(uint8_t state);

static void pmd_dev_clean_registered_callbacks(void);
static void pmd_dev_first_burst_temp_dep(void);

/* indicate pmd fw is ready */
static wait_queue_head_t pmd_reset_event;
static int pmd_reset_normal;

/* holds latest read alarm */
static uint16_t alarm_msg;
static bool alarm_read_flag;

/* for debugging and calibration */
static uint8_t disable_tracking_flag;
/* debug print data collection */
static uint8_t data_col_flag;

static uint32_t original_crc;

#define TEMP_NOT_SET 0x555

static int16_t pmd_ext_tracking_temp;

#define PMD_DATA_COL_INTERVAL 5;

#define PMD_CALIBRATION_INIT 1
#define PMD_CALIBRATION_STOP 15

#define MAX_NUM_RANGING_ATTEMPTS 4

#define MIN_CHIP_TEMPERATURE -39
#define MAX_CHIP_TEMPERATURE 109

static pmd_calibration_parameters_index pmd_cal_rx_param_valid[] = {PMD_RSSI_B};
static pmd_calibration_parameters_index pmd_cal_tx_param_valid[] = {PMD_MOD, PMD_BIAS};

static pmd_calibration_parameters static_calibration_parameters_from_json;
static uint16_t temp2apd_table[APD_TEMP_TABLE_SIZE];
static uint32_t res2temp[TEMP_TABLE_SIZE];

extern void clk_divide_50mhz_to_25mhz(void);

static void pmd_dev_exit_steps(void);
static int pmd_dev_init_steps(pmd_calibration_parameters *calibration_parameters_from_json);
static void enforce_rssi_adf_los_backwards_compatibility(void);
static void calculate_and_update_adf_los_cal_params(void);

static int pmdmsg_try_single_user_session(void);
static void pmdmsg_check_session_statistics_accounting_stability(const loff_t *ppos);
static void pmdmsg_print_session_report(const loff_t *ppos);
static void pmdmsg_accumulate_session_statistics(const loff_t *ppos);
static void pmdmsg_init_session(void);
static void pmdmsg_write_calibration_state(const char *calibration_state_message);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
static int pon_message_handler_thread(void *arg);
#endif

static void exported_symbols_write_lock(void)
{
    spin_lock_bh(&exported_symbols_spinlock);
    mb();
    exported_symbols_writers++; /* Block new invokations of exported symbols */
    mb();
    while(1 != exported_symbols_writers)
    {   /* Should never be here! A single PMD reset is allowed concurrently */
        spin_unlock_bh(&exported_symbols_spinlock);
        spin_lock_bh(&exported_symbols_spinlock);
        mb();
    }
    while(exported_symbols_readers)
    {   /* Let all the already running exported symbols to finish */
        spin_unlock_bh(&exported_symbols_spinlock);
        spin_lock_bh(&exported_symbols_spinlock);
        mb();
    }
    spin_unlock_bh(&exported_symbols_spinlock);
}


static void exported_symbols_write_unlock(void)
{
    spin_lock_bh(&exported_symbols_spinlock);
    mb();
    exported_symbols_writers--; /* Allow new invokations of exported symbols */
    mb();
    spin_unlock_bh(&exported_symbols_spinlock);
}


#define READ_LOCK_WAS_OBTAINED     (1)
#define READ_LOCK_WAS_NOT_OBTAINED (0)
static int exported_symbol_read_trylock(void)
{
    spin_lock_bh(&exported_symbols_spinlock);
    mb();
    if (exported_symbols_writers)
    {
        spin_unlock_bh(&exported_symbols_spinlock);
        return READ_LOCK_WAS_NOT_OBTAINED;
    }
    exported_symbols_readers++;
    mb();
    spin_unlock_bh(&exported_symbols_spinlock);

    return READ_LOCK_WAS_OBTAINED;
}


static void exported_symbol_read_unlock(void)
{
    spin_lock_bh(&exported_symbols_spinlock);
    mb();
    exported_symbols_readers--;
    mb();
    spin_unlock_bh(&exported_symbols_spinlock);
}

static int pmd_dev_reconfigure_wan_type_unlocked(PMD_WAN_TYPES new_pmd_wan_type,
    pmd_calibration_parameters *calibration_parameters_from_json)
{
    int rc;

    BCM_LOG_NOTICE(BCM_LOG_ID_PMD, "Reconfiguring PMD WAN type to %s\n", pmd_wan_type_string[new_pmd_wan_type]);

    exported_symbols_write_lock();

    pmd_dev_exit_steps();
    pmd_dev_clean_registered_callbacks();
    pmd_wan_type = new_pmd_wan_type;
    rc = pmd_dev_init_steps(calibration_parameters_from_json);

    exported_symbols_write_unlock();
    
    return rc;
}

int pmd_dev_reconfigure_wan_type(PMD_WAN_TYPES new_pmd_wan_type,
    pmd_calibration_parameters *calibration_parameters_from_json)
{
    int rc;

    LOCK();
    rc = pmd_dev_reconfigure_wan_type_unlocked(new_pmd_wan_type, calibration_parameters_from_json);
    UNLOCK();
    return rc;
}
EXPORT_SYMBOL(pmd_dev_reconfigure_wan_type);

static void pon_set_pmd_fb_done_wrapper(uint8_t state)
{
    if(pon_set_pmd_fb_done)
    {
        pon_set_pmd_fb_done(state);
    }
    else
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "The PMD callback pon_set_pmd_fb_done is not registered");
    }    
}

static void pmd_dev_clean_registered_callbacks(void)
{
    pon_set_pmd_fb_done = NULL;
}

void pmd_dev_assign_pon_stack_callback(pon_set_pmd_fb_done_callback _pon_set_pmd_fb_done)
{
    if (READ_LOCK_WAS_NOT_OBTAINED == exported_symbol_read_trylock())
    {
        BCM_LOG_NOTICE(BCM_LOG_ID_PMD, "It is not allowed to invoke the exported symbol %s during PMD reset\n",
            __func__);
        return;
    }
    pon_set_pmd_fb_done = _pon_set_pmd_fb_done;
    exported_symbol_read_unlock();
}
EXPORT_SYMBOL(pmd_dev_assign_pon_stack_callback);

static void set_gpio_rstn(uint8_t gpio_state)
{
    pmd_reset_mode = gpio_state;

    gpiod_direction_output(g_pdata->rstn, gpio_state);
    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "gpio_rstn set to %s \n", gpio_state ? "active" : "inactive");
}

static int ewake_get(void)
{
    if (g_pdata->ewake)
        return 0;

    g_pdata->ewake = gpiod_get_index(g_pdata->dev, "ewake", 0, GPIOD_ASIS);
    if (IS_ERR(g_pdata->ewake))
    {
        dev_err(g_pdata->dev, "Missing ewake configuration %ld", PTR_ERR(g_pdata->ewake));
        g_pdata->ewake = NULL;
        return -EINVAL;
    }

    return 0;
}

static void ewake_put(void)
{
    if (g_pdata->ewake)
    {
        gpiod_put(g_pdata->ewake);
        g_pdata->ewake = NULL;
    }
}

static void pmd_dev_ewake_mac_control(uint16_t enable)
{
    struct pinctrl_state *pinctrl = g_pdata->pins_ewake;

    if (!enable)
    {
        ewake_get();
        gpiod_direction_output(g_pdata->ewake, 1);
    }
    else
    {
        ewake_put();
        pinctrl_select_state(g_pdata->pinctrl, pinctrl);
    }
}

static void pmd_dev_diable_ewake_in_calibration(hm_msg_id msg_id, uint16_t type)
{
    if(msg_id != hmid_cal_type_set)
        return;

    if (type == PMD_CALIBRATION_INIT)
    {
        pmd_dev_ewake_mac_control(0);
        in_calibration = 1;
    }

    else if (type == PMD_CALIBRATION_STOP)
    {
        pmd_dev_ewake_mac_control(1);
        in_calibration = 0;
    }

}

static void pmd_dev_update_pmd_fw_tracking(void)
{
    if (pmd_state != pmd_prev_state)
    {
        if (!disable_tracking_flag)
        {
            int rc;

            printk("PMD tracking state change from: %s to: %s\n", tracking_state_string[pmd_prev_state],
                tracking_state_string[pmd_state]);

            rc = pmd_msg_handler(hmid_laser_tracking_set, &pmd_state, sizeof(uint16_t));
            if (rc)
                BCM_LOG_ERROR(BCM_LOG_ID_PMD, " Fail to change tracking state \n");

            pmd_prev_state = pmd_state;
        }
        else
            printk("ERR: PMD tracking state change from: %s to: %s\n is not allowed",
                tracking_state_string[pmd_prev_state], tracking_state_string[pmd_state]);
    }
}

static int pmd_dev_update_pmd_fw_force(uint16_t force)
{
    int rc;

    rc = pmd_msg_handler(hmid_force_enable, &force, sizeof(force));
    if (rc)
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, " Fail in hmid_force_enable \n");

    return rc;
}

static long pmd_dev_file_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    uint32_t *pPwr = (uint32_t *)arg;
    unsigned long rc=0;

    LOCK();
    switch (cmd) 
    {
        case LASER_IOCTL_GET_RX_PWR:
        {           
            uint16_t op;
			uint32_t pmdVaule;
  
            rc = pmd_msg_handler(hmid_estimated_op_get, &op, sizeof(uint16_t));
            if (rc)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_PMD, " Error reading PMD RX optical power \n");
            }
            pmdVaule = ((uint32_t)op * 10 / 16); /* PMD units is 1/16uW, converting to 0.1uW units */
			COPY_TO_USER(pPwr, &pmdVaule, sizeof(uint32_t));

			break;
        }
        case LASER_IOCTL_GET_TX_PWR:
        {
            int32_t target_level_1, target_tx_power, are_there_tx_power_samples;
            uint16_t measured_level_1;
			uint32_t pmdVaule;			

            rc = pmd_op_i2c(2, 0x8130, (unsigned char *)&are_there_tx_power_samples, 4, PMD_READ_OP);
            if (rc)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_PMD, " Error reading Tx power samples \n");
                pmdVaule = 0;
				COPY_TO_USER(pPwr, &pmdVaule, sizeof(uint32_t));
                break;
            }
            if (!are_there_tx_power_samples)
            {
                BCM_LOG_DEBUG(BCM_LOG_ID_PMD, " There are no Tx power samples \n");
                pmdVaule = 0;
				COPY_TO_USER(pPwr, &pmdVaule, sizeof(uint32_t));
                break;
            }

            rc = pmd_msg_handler(hmid_avrg_tx_power_tracking_get, &measured_level_1, sizeof(uint16_t));
            if (rc)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_PMD, " Error reading measured PMD average TX optical power \n");
                pmdVaule = 0;
				COPY_TO_USER(pPwr, &pmdVaule, sizeof(uint32_t));
                break;
            }
            if (pmd_cal_param_get(PMD_TX_POWER, &target_tx_power, cal_index))
            {
                BCM_LOG_ERROR(BCM_LOG_ID_PMD, " Error reading target PMD average TX optical power \n");
                pmdVaule = 0;
				COPY_TO_USER(pPwr, &pmdVaule, sizeof(uint32_t));
                break;
            }
            if (pmd_cal_param_get(PMD_AVG_LEVEL_1_DAC, &target_level_1, cal_index))
            {
                BCM_LOG_ERROR(BCM_LOG_ID_PMD, " Error reading target PMD level_1_dac \n");
                pmdVaule = 0;
				COPY_TO_USER(pPwr, &pmdVaule, sizeof(uint32_t));
                break;
            }
            if (0 == target_level_1)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_PMD, " Error: target PMD level_1_dac == 0 \n");
                pmdVaule = 0;
				COPY_TO_USER(pPwr, &pmdVaule, sizeof(uint32_t));
                break;
            }

            pmdVaule = 10 * target_tx_power * measured_level_1 / target_level_1; /* in 0.1uW units */
			COPY_TO_USER(pPwr, &pmdVaule, sizeof(uint32_t));

            BCM_LOG_DEBUG(BCM_LOG_ID_PMD,
                " Measured PMD level_1_dac = %d 0x%x target PMD TX optical power = %d 0x%x (measured power %d) \n",
                measured_level_1, measured_level_1, target_tx_power, target_tx_power, *pPwr / 10);

            break;
        }
        case LASER_IOCTL_GET_TEMPTURE:
        {
            uint16_t cur_temp = 0xFFFF;
			uint32_t pmdVaule;		

            /* read and clear pmd interrupt */
            rc = pmd_msg_handler(hmid_temperature_get, &cur_temp, sizeof(uint16_t));
            if (rc)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_PMD, " Error reading PMD temperature \n");
            }

            pmdVaule = ((uint32_t)cur_temp << 8);
			COPY_TO_USER(pPwr, &pmdVaule, sizeof(uint32_t));
			
            break;
        }
        case LASER_IOCTL_GET_VOLTAGE:
        {
            uint32_t pmd_voltage_1_0 = 0;
            uint32_t pmd_voltage_3_3 = 0;
            int value;
			uint32_t pmdVaule;	
			
            rc = GetPVT(4, 0, &value);
            if (kPMC_NO_ERROR != rc)
                BCM_LOG_ERROR(BCM_LOG_ID_PMD, " Error reading select4 \n");
            else
                pmd_voltage_1_0 = (880 * value * 10) / (7 * 1024);

            rc = GetPVT(6, 0, &value);
            if (kPMC_NO_ERROR != rc)
                BCM_LOG_ERROR(BCM_LOG_ID_PMD, " Error reading select6 \n");
            else
                pmd_voltage_3_3 = (880 * value * 10) / (2 * 1024);
            pmdVaule = ((10 * pmd_voltage_1_0) << 16)
                | (10 * pmd_voltage_3_3); /* in 10mV units (compatible to laser_dev) */

			COPY_TO_USER(pPwr, &pmdVaule, sizeof(uint32_t));

            break;
        }
        case LASER_IOCTL_GET_BIAS_CURRENT:
        {
            uint16_t cur_bias = 0;
			uint32_t pmdVaule;

            rc = pmd_msg_handler(hmid_bias_cur_get, &cur_bias, sizeof(uint16_t));
            if (rc)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_PMD, " Error reading PMD current bias \n");
            }

            pmdVaule = (uint32_t)cur_bias * 10; /* In 2 uA */
			COPY_TO_USER(pPwr, &pmdVaule, sizeof(uint32_t));
			
            break;
        }
        case PMD_IOCTL_SET_PARAMS:
        {
            pmd_params pmd_op;
            char buf[PMD_BUF_MAX_SIZE];

            rc = copy_from_user(&pmd_op, (void *)arg, sizeof(pmd_params));
            if (rc)
            {
                rc = -EFAULT;
                break;
            }
            BCM_IOC_PTR_ZERO_EXT(pmd_op.buf);

            if (pmd_op.len > PMD_BUF_MAX_SIZE)
                pmd_op.len = PMD_BUF_MAX_SIZE;

            rc = copy_from_user(buf, pmd_op.buf, pmd_op.len);
            if (rc)
            {
                rc = -EFAULT;
                break;
            }

            rc = pmd_op_i2c(pmd_op.client, pmd_op.offset, buf, pmd_op.len, PMD_WRITE_OP);
            break;
        }
        case PMD_IOCTL_GET_PARAMS:
        {
            pmd_params pmd_op;
            char buf[PMD_BUF_MAX_SIZE];

            rc = copy_from_user(&pmd_op, (void *)arg, sizeof(pmd_params));
            if (rc)
            {
                rc = -EFAULT;
                break;
            }
            BCM_IOC_PTR_ZERO_EXT(pmd_op.buf);

            if( pmd_op.len > PMD_BUF_MAX_SIZE )
                pmd_op.len = PMD_BUF_MAX_SIZE;

            rc = pmd_op_i2c(pmd_op.client, pmd_op.offset, buf, pmd_op.len, PMD_READ_OP);
            if (rc)
                break;

            rc = copy_to_user(pmd_op.buf, buf, pmd_op.len);
            if (rc)
            {
                rc = -EFAULT;
            }
            break;
        }
        case PMD_IOCTL_CAL_FILE_WRITE:
        {
            pmd_params pmd_op;
            int32_t val;

            rc = copy_from_user(&pmd_op, (void *)arg, sizeof(pmd_params));
            if (rc)
            {
                rc = -EFAULT;
                break;
            }
            BCM_IOC_PTR_ZERO_EXT(pmd_op.buf);

            rc = copy_from_user(&val, pmd_op.buf, sizeof(int32_t));
            if (rc)
            {
                rc = -EFAULT;
                break;
            }

            if (PMD_HOST_SW_CONTROL == pmd_op.offset)
                disable_tracking_flag = val;
            else if (PMD_STATISTICS_COLLECTION == pmd_op.offset)
            {
                data_col_flag = val;
                if (data_col_flag)
                    pmd_data_col_print();
            }
            else
                rc = pmd_cal_param_set((pmd_calibration_parameters_index)pmd_op.offset, val, pmd_op.len);

            break;
        }
        case PMD_IOCTL_CAL_FILE_READ:
        {
            pmd_params pmd_op;
            int32_t val;

            rc = copy_from_user(&pmd_op, (void *)arg, sizeof(pmd_params));
            if (rc)
            {
                rc = -EFAULT;
                break;
            }
            BCM_IOC_PTR_ZERO_EXT(pmd_op.buf);

            rc = pmd_cal_param_get((pmd_calibration_parameters_index)pmd_op.offset, &val, pmd_op.len);
            if (rc)
            {
                /* negative ioctl file_operations are returned as (-1). So we use positive values and convert */
                rc = -rc;
                break;
            }

            rc = copy_to_user(pmd_op.buf, &val, sizeof(int32_t));
            if (rc)
            {
                rc = -EFAULT;
            }
            break;
        }
        case PMD_IOCTL_MSG_WRITE:
        {
            pmd_params pmd_op;
            char buf[PMD_BUF_MAX_SIZE];
#ifdef __LITTLE_ENDIAN
            int i;
#endif

            rc = copy_from_user(&pmd_op, (void *)arg, sizeof(pmd_params));
            if (rc)
            {
                rc = -EFAULT;
                break;
            }
            BCM_IOC_PTR_ZERO_EXT(pmd_op.buf);

            if (get_msg_type((hm_msg_id)pmd_op.offset) != pmd_set_msg)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_PMD, " msg_id = %x  is not a set msg \n", pmd_op.offset);
                rc = -1;
                break;
            }

            if (pmd_op.len > PMD_BUF_MAX_SIZE)
                pmd_op.len = PMD_BUF_MAX_SIZE;

            rc = copy_from_user(buf, pmd_op.buf, pmd_op.len);
            if (rc)
            {
                rc = -EFAULT;
                break;
            }

#ifdef __LITTLE_ENDIAN
            for (i = 0; i < (pmd_op.len / 2); i++)
            {
                *(uint16_t *) &buf[i*2] = swab16(*(uint16_t *)&buf[i*2]);
            }
#endif

            pmd_dev_diable_ewake_in_calibration((hm_msg_id)pmd_op.offset, *(uint16_t *)buf);
            rc = pmd_msg_handler((hm_msg_id)pmd_op.offset, (uint16_t *)buf, pmd_op.len);

            break;
        }
        case PMD_IOCTL_MSG_READ:
        {
            pmd_params pmd_op;
            char buf[PMD_BUF_MAX_SIZE];
#ifdef __LITTLE_ENDIAN
            int i;
#endif

            rc = copy_from_user(&pmd_op, (void *)arg, sizeof(pmd_params));
            if (rc)
            {
                rc = -EFAULT;
                break;
            }
            BCM_IOC_PTR_ZERO_EXT(pmd_op.buf);

            if (get_msg_type((hm_msg_id)pmd_op.offset) != pmd_get_msg)
            {
                BCM_LOG_ERROR(BCM_LOG_ID_PMD, " msg_id = %x is not a get msg \n", pmd_op.offset);
                rc = -1;
                break;
            }

            if (pmd_op.len > PMD_BUF_MAX_SIZE)
                pmd_op.len = PMD_BUF_MAX_SIZE;

            rc = pmd_msg_handler((hm_msg_id)pmd_op.offset, (uint16_t *)buf, pmd_op.len);
            if (rc)
                break;

#ifdef __LITTLE_ENDIAN
            for (i = 0; i < (pmd_op.len / 2); i++)
            {
                *(uint16_t *) &buf[i*2] = swab16(*(uint16_t *)&buf[i*2]);
            }
#endif

            rc = copy_to_user(pmd_op.buf, buf, pmd_op.len);
            if (rc)
            {
                rc = -EFAULT;
            }
            break;
        }
        case PMD_IOCTL_TEMP2APD_WRITE:
        {
            pmd_params pmd_op;

            rc = copy_from_user(&pmd_op, (void *)arg, sizeof(pmd_params));
            if (rc)
            {
                rc = -EFAULT;
                break;
            }
            BCM_IOC_PTR_ZERO_EXT(pmd_op.buf);
            rc = copy_from_user(temp2apd_table, (void *)pmd_op.buf, sizeof(temp2apd_table));
            if (rc)
            {
                rc = -EFAULT;
                break;
            }
            rc = pmd_op_temp_apd_conv_table_download(temp2apd_table);
            if (rc)
                BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Error in pmd_op_temp_apd_conv_table_download \n");

            break;
        }
        case PMD_IOCTL_DUMP_DATA:
        {
            int rc;
            rc =  pmd_dump_data();
            if(rc)
                BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "pmd_dumpdata command failed  \n");
            break;
        }
        case PMD_IOCTL_RESET_INTO_CALIBRATION_MODE:
        {
            BCM_LOG_NOTICE(BCM_LOG_ID_PMD, "Start PMD reset\n");
            exported_symbols_write_lock();
            pmd_dev_exit_steps();
            rc = pmd_dev_init_steps(NULL);
            exported_symbols_write_unlock();
            BCM_LOG_NOTICE(BCM_LOG_ID_PMD, "PMD reset done\n");
            break;
        }
        case PMD_IOCTL_RES2TEMP_WRITE:
        {
            pmd_params pmd_op;

            rc = copy_from_user(&pmd_op, (void *)arg, sizeof(pmd_params));
            if (rc)
            {
                rc = -EFAULT;
                break;
            }
            BCM_IOC_PTR_ZERO_EXT(pmd_op.buf);
            rc = copy_from_user(res2temp, (void *)pmd_op.buf, sizeof(res2temp));
            if (rc)
            {
                rc = -EFAULT;
                break;
            }
            rc = pmd_op_temp_conv_table_download(res2temp);
            if (rc)
                break;
            if (rc)
                BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Error in pmd_op_temp_conv_table_download \n");

            break;
        }
        case PMD_IOCTL_SET_WAN_TYPE:
        {
            PMD_WAN_TYPES pmd_wan_type = PMD_AUTO_DETECT_WAN;
            pmd_params pmd_op;
            net_port_t net_port;

            rc = copy_from_user(&pmd_op, (void *)arg, sizeof(pmd_params));
            if (rc)
            {
                rc = -EFAULT;
                break;
            }
            BCM_IOC_PTR_ZERO_EXT(pmd_op.buf);

            net_port = pmd_op.net_port;

#ifdef CONFIG_BCM963158
            /* 63158 supports GPON only */
            pmd_wan_type = PMD_GPON_2_1_WAN;
#else
            if (net_port.port == NET_PORT_EPON)
            {
                if (net_port.speed == NET_PORT_SPEED_1001)
                    pmd_wan_type = PMD_EPON_10_1_WAN;
                else if (net_port.speed == NET_PORT_SPEED_0201)
                    pmd_wan_type = PMD_GPON_2_1_WAN;
                else if ((net_port.speed == NET_PORT_SPEED_0101) || (net_port.speed == NET_PORT_SPEED_NONE))
                    pmd_wan_type = PMD_EPON_1_1_WAN;
                else
                    rc = -EFAULT;
            }
            else if (net_port.port == NET_PORT_AE && net_port.speed == NET_PORT_SPEED_0101)
                pmd_wan_type = PMD_GBE_1_1_WAN;
            else if (net_port.port == NET_PORT_GPON && net_port.sub_type == NET_PORT_SUBTYPE_GPON)
                pmd_wan_type = PMD_GPON_2_1_WAN;
            else if (net_port.port == NET_PORT_GPON && net_port.sub_type == NET_PORT_SUBTYPE_XGPON)
                pmd_wan_type = PMD_XGPON1_10_2_WAN;
            else
            {
                BCM_LOG_NOTICE(BCM_LOG_ID_PMD, "Error can't determine the PMD WAN type\n");
                rc = -EINVAL;
                break;
            }
#endif

            if (pmd_op.buf)
            {
                rc = copy_from_user(&static_calibration_parameters_from_json, pmd_op.buf,
                    sizeof(pmd_calibration_parameters));
                if (rc)
                {
                    BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Error can't get the PMD calibration parameters (rc=%d)\n", rc);
                    rc = -EFAULT;
                    break;
                }
            }
            else
            {
                memset(&static_calibration_parameters_from_json, 0x00, sizeof(pmd_calibration_parameters));
                BCM_LOG_NOTICE(BCM_LOG_ID_PMD, "The PMD is not calibrated\n");
            }

            rc = pmd_dev_reconfigure_wan_type_unlocked(pmd_wan_type, &static_calibration_parameters_from_json);
            if (rc)
            {
#ifdef CONFIG_BCM963158
                BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Error setting PMD GPON 2/1 WAN type\n");
#else
                BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Error setting wan type port=%d subtype=%d speed=%d\n", net_port.port,
                    net_port.sub_type, net_port.speed);
#endif
                rc = -EFAULT;
            }
            else
                BCM_LOG_NOTICE(BCM_LOG_ID_PMD, "Reconfiguring PMD WAN type to %s\n", pmd_wan_type_string[pmd_wan_type]);

            break;
        }
        case LASER_IOCTL_GET_DRV_INFO:
        {
            uint32_t x = BCM_I2C_PON_OPTICS_TYPE_PMD;

            COPY_TO_USER(pPwr, &x, sizeof(uint32_t));
            break;
        }

        default:
            BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "pmd_dev IOCtl command unknown \n");
            break;
    }
    UNLOCK();
    return rc;
}

static int pmd_dev_file_open(struct inode *inode, struct file *file)
{
    return PMD_OPERATION_SUCCESS;
}
static int pmd_dev_file_release(struct inode *inode, struct file *file)
{
    return PMD_OPERATION_SUCCESS;
}

static const struct file_operations pmd_file_ops = {
    .owner =        THIS_MODULE,
    .open =         pmd_dev_file_open,
    .release =      pmd_dev_file_release,
    .unlocked_ioctl =   pmd_dev_file_ioctl,
#if defined(CONFIG_COMPAT)
    .compat_ioctl = pmd_dev_file_ioctl,
#endif
    
};

static int crt_reset_config(void)
{
    uint32_t rv;
    int rc;

    rc = RU_REG_READ(CRT, DIGITAL_BLOCK_RESET_CONFIG, &rv);
    RU_FIELD_SET(CRT, DIGITAL_BLOCK_RESET_CONFIG, CFG_CPU_RST, rv, 0);
    rc = rc ? rc : RU_REG_WRITE(CRT, DIGITAL_BLOCK_RESET_CONFIG, rv);
    return rc;
}

static int pmd_func_set(void)
{
    int rc;
    uint32_t rv;

    rc = RU_REG_READ(CRT, CRT_PARAM, &rv);
    if (rc)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Error reading CRT register. rc = %d\n", rc);
    }

    rv = g_pdata->cap | pmd_wan_type;

    rc = RU_REG_WRITE(CRT, CRT_PARAM, rv);
    if (rc)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Error writing CRT register. rc = %d\n", rc);
    }

    return rc;
}

static void ld_lia_set_cdr(void)
{
    RU_FIELD_WRITE(LD_LIA, LD_CDR_CONFIG_2, CFG_LD_CDR_RESET, 0); /* Take CDR out of reset */
    RU_FIELD_WRITE(LD_LIA, LD_CONTROL_1, CFG_LD_SIG_BIASADJ, 0x4222); /* Enable CDR signal path buffers */

    RU_FIELD_WRITE(LD_LIA, LD_CDR_RX_CONFIG, CFG_LD_RX0_PWRDN, 0);
    RU_FIELD_WRITE(LD_LIA, LD_CDR_RX_CONFIG, CFG_LD_RX0_SIGDET_PWRDN, 0);
    if (pmd_wan_type != PMD_XGPON1_10_2_WAN)
    {
        RU_FIELD_WRITE(LD_LIA, LD_CDR_CONFIG_1, CFG_LD_CDR_FREQ_SEL, 0);
        RU_FIELD_WRITE(LD_LIA, LD_CDR_CONFIG_1, CFG_LD_CDR_PHASE_SAT_SM, 0);
    }
}

static int ld_pll_init(void)
{
    uint32_t rv = 0;
    uint32_t retry = 1024;
    int rc;

#if defined(CONFIG_BCM963158)
    clk_divide_50mhz_to_25mhz();
#endif

    /* RefClkCheck */
    RU_FIELD_WRITE(CRT, REF_CLOCK_MONITOR_CONFIG, CFG_REFCLK_MON_EN, 0);
    RU_FIELD_WRITE(CRT, REF_CLOCK_MONITOR_CONFIG, CFG_REFCLK_MON_EN, 1);
    rc = RU_REG_READ(CRT, REF_CLOCK_MONITOR_STATUS, &rv);
    if (RU_FIELD_GET(CRT, REF_CLOCK_MONITOR_STATUS, STAT_REFCLK_MON_RESULT, rv) < 10)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "pll error: External reference clock frequency failure");
        return -1;
    }

    rc = rc ? rc : RU_REG_READ(LD_LIA, LD_PLL_CONTROL_1, &rv);
    RU_FIELD_SET(LD_LIA, LD_PLL_CONTROL_1, CFG_LD_PLL_KA, rv, 3);
    RU_FIELD_SET(LD_LIA, LD_PLL_CONTROL_1, CFG_LD_PLL_KI, rv, 1);
    RU_FIELD_SET(LD_LIA, LD_PLL_CONTROL_1, CFG_LD_PLL_KP, rv, 9);
    RU_FIELD_SET(LD_LIA, LD_PLL_CONTROL_1, CFG_LD_PLL_MODE1P25, rv, 0);

    rc = rc ? rc : RU_REG_WRITE(LD_LIA, LD_PLL_CONTROL_1, rv);
    RU_FIELD_SET(LD_LIA, LD_PLL_CONTROL_1, CFG_LD_PLL_PWRDN, rv, 0);
    rc = rc ? rc : RU_REG_WRITE(LD_LIA, LD_PLL_CONTROL_1, rv);
    RU_FIELD_SET(LD_LIA, LD_PLL_CONTROL_1, CFG_LD_PLL_PDIV, rv, 1);

    if (pmd_wan_type == PMD_GPON_2_1_WAN)
    {
        RU_FIELD_SET(LD_LIA, LD_PLL_CONTROL_1, CFG_LD_PLL_NDIV_INT, rv, 99);
        rc = rc ? rc : RU_REG_WRITE(LD_LIA, LD_PLL_CONTROL_1, rv);
        RU_FIELD_WRITE(LD_LIA, LD_PLL_CONTROL_2, CFG_LD_PLL_NDIV_FRAC, 558682);
        RU_FIELD_WRITE(LD_LIA, LD_PLL_DIVIDER_0, CFG_LD_PLL_MDIV_0, 1);
    }
    else if (pmd_wan_type == PMD_XGPON1_10_2_WAN)
    {
        RU_FIELD_SET(LD_LIA, LD_PLL_CONTROL_1, CFG_LD_PLL_NDIV_INT, rv, 99);
        rc = rc ? rc : RU_REG_WRITE(LD_LIA, LD_PLL_CONTROL_1, rv);
        RU_FIELD_WRITE(LD_LIA, LD_PLL_CONTROL_2, CFG_LD_PLL_NDIV_FRAC, 545260);
        RU_FIELD_WRITE(LD_LIA, LD_PLL_DIVIDER_0, CFG_LD_PLL_MDIV_0, 1);
    }
    else if ((pmd_wan_type == PMD_EPON_1_1_WAN) || (pmd_wan_type == PMD_EPON_10_1_WAN))
    {
        RU_FIELD_SET(LD_LIA, LD_PLL_CONTROL_1, CFG_LD_PLL_NDIV_INT, rv, 100);
        rc = rc ? rc : RU_REG_WRITE(LD_LIA, LD_PLL_CONTROL_1, rv);
        RU_FIELD_WRITE(LD_LIA, LD_PLL_CONTROL_2, CFG_LD_PLL_NDIV_FRAC, 0);
        RU_FIELD_WRITE(LD_LIA, LD_PLL_DIVIDER_0, CFG_LD_PLL_MDIV_0, 1);
    }

    RU_FIELD_WRITE(LD_LIA, LD_PLL_DIVIDER_1, CFG_LD_PLL_MDIV_1, 5);
    RU_FIELD_WRITE(LD_LIA, LD_PLL_DIVIDER_2, CFG_LD_PLL_MDIV_2, 12);
    RU_FIELD_WRITE(LD_LIA, LD_PLL_DIVIDER_3, CFG_LD_PLL_MDIV_3, 39);
    RU_FIELD_WRITE(LD_LIA, LD_PLL_DIVIDER_4, CFG_LD_PLL_MDIV_4, 5);
    RU_FIELD_WRITE(LD_LIA, LD_PLL_DIVIDER_5, CFG_LD_PLL_MDIV_5, 5);
   
    RU_FIELD_SET(LD_LIA, LD_PLL_CONTROL_1, CFG_LD_PLL_RESETB, rv, 1);
    rc = rc ? rc : RU_REG_WRITE(LD_LIA, LD_PLL_CONTROL_1, rv);

    while (--retry && !RU_FIELD_READ(LD_LIA, LD_PLL_STATUS, LD_PLL_LOCK))
        ;

    if (!retry)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "PLL lock has failed");
        return -1;
    }
    else
    {
        RU_FIELD_SET(LD_LIA, LD_PLL_CONTROL_1, CFG_LD_PLL_POST_RESETB, rv, 1);
        rc = rc ? rc : RU_REG_WRITE(LD_LIA, LD_PLL_CONTROL_1, rv);


        /* CrtBoot */
        RU_FIELD_WRITE(CRT, CLOCK_SOURCE_CONFIG, CFG_CORE_CLOCK_SOURCE, 1);
        RU_FIELD_WRITE(CRT, DIGITAL_BLOCK_RESET_CONFIG, CFG_GEN_CLOCKS_RST, 0);
        RU_FIELD_WRITE(CRT, CLOCK_SOURCE_CONFIG, CFG_BSL_DB_CLOCK_SOURCE, 1);
        RU_FIELD_WRITE(CRT, DIGITAL_BLOCK_RESET_CONFIG, CFG_LDC_RST, 0);
        RU_FIELD_WRITE(CRT, DIGITAL_BLOCK_RESET_CONFIG, CFG_DYN_MDIV_RST, 0);

        ld_lia_set_cdr();
    }

    return rc;
}

static int pmd_dev_periodic_thread(void *arg)
{
    int16_t chip_temperature;
    uint32_t pmd_indications;

    while (1)
    {
        static uint8_t col_data_interval = PMD_DATA_COL_INTERVAL;

        wait_event_interruptible(pmd_thread_event, pmd_thread_sched || kthread_should_stop());

        pmd_indications =  pmd_thread_sched;
        pmd_thread_sched = 0;

        if (kthread_should_stop())
        {
            BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "kthread_should_stop detected on pmd_dev\n");
            return PMD_OPERATION_SUCCESS;
        }
        
        if (pmd_indications & PMD_TIMER_INDICATION)
        {
            if (data_col_flag)
            {
                if (!col_data_interval)
                {
                    if (pmd_state == pmd_tracking_state_tracking)
                        pmd_data_collect(1);
                    else
                        pmd_data_collect(0);

                    col_data_interval = PMD_DATA_COL_INTERVAL;
                }

                if (temp_poll)
                    pmd_msg_handler(hmid_temperature_get, &pmd_ext_tracking_temp, sizeof(uint16_t));
                else if (pmd_state == pmd_tracking_state_tracking)
                    pmd_dev_first_burst_temp_dep();


                pmd_msg_handler(hmid_chip_temp_get, &chip_temperature, sizeof(int16_t));
                if ((chip_temperature < MIN_CHIP_TEMPERATURE) || (chip_temperature > MAX_CHIP_TEMPERATURE))
                {
                    set_gpio_rstn(1);
                    BCM_LOG_ERROR(BCM_LOG_ID_PMD, "PMD is DISABLED - temperature (%d) exceeded MIN/MAX thresholds\n",
                        chip_temperature);
                }

                col_data_interval--;
            }
        }
 
    }
    return PMD_OPERATION_SUCCESS;
}

static int pmd_dev_thread_init(void)
{
    /* Initialize threads */
    pmd_timer_thread = kthread_create(pmd_dev_periodic_thread, NULL, "pmd_thread");
    if(!pmd_timer_thread)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Timer thread creation failed\n");
        return -1;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
    pon_msg_event_thread = kthread_create(pon_message_handler_thread, NULL, "pon_msg_event_thread");
    if(!pon_msg_event_thread)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "PON message thread creation failed\n");
        kthread_stop(pmd_timer_thread);
        pmd_timer_thread = 0;
        return -1;
    }
    __pon_msg_type_bit_mask = 0;
    init_waitqueue_head(&pon_msg_event_wq);
    wake_up_process(pon_msg_event_thread);
#endif

    pmd_thread_sched = 0;
    init_waitqueue_head(&pmd_thread_event);
    wake_up_process(pmd_timer_thread);

    return PMD_OPERATION_SUCCESS;
}

/* check if init val is already in the flash, if so use it */
static int set_param_from_flash(pmd_calibration_parameters_index param_id, hm_msg_id msg_id)
{
    int32_t val;
    uint16_t msg_seq;
    int rc = PMD_OPERATION_SUCCESS;

    if (msg_id == hmid_apd_config_set)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "PMD APD configuration set message is deprecated\n");
        return PMD_OPERATION_SUCCESS;
    }

    /* don't configure if it's not a valid parameter */
    if (pmd_cal_param_get(param_id, &val, cal_index))
        return PMD_OPERATION_SUCCESS;

    msg_seq = get_msg_seq_num(msg_id);
    switch (msg_seq)
    {
    case 1:
#ifdef __LITTLE_ENDIAN
        rc = pmd_msg_handler(msg_id, &((int16_t *)&val)[0], 2);
#else
        rc = pmd_msg_handler(msg_id, &((int16_t *)&val)[1], 2);
#endif
        BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "msg_id %x  val %x \n", (uint32_t)msg_id, val);
        break;
    case 2:
#ifdef __LITTLE_ENDIAN
        val = (val & 0x0000ffffUL) << 16 | (val & 0xffff0000UL) >> 16;
#endif
        rc = pmd_msg_handler(msg_id, (uint16_t *)&val, 4);
        BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "msg_id %x  val %x \n", (uint32_t)msg_id, val);
        break;
    default :
        rc = -1;
        break;
    }

    return rc;
}

static int pmd_set_init_val(void)
{
    int rc;
#if 0
    int32_t val = 0;

    rc = pmd_cal_param_get(PMD_FILE_WATERMARK, &val, cal_index);
    if (rc || (val != CAL_FILE_WATERMARK))
    {
        BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "Flash file is corrupted\n");
        return PMD_OPERATION_SUCCESS;
    }
#endif

    rc = set_param_from_flash(PMD_FAQ_LEVEL0_DAC, hmid_level_0_dac_ref);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_FAQ_LEVEL1_DAC, hmid_level_1_dac_ref);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_BIAS, hmid_bias_set);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_MOD, hmid_mod_set);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_MPD_CONFIG, hmid_mpd_config_set);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_MPD_GAINS, hmid_mpd_gains_set);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_APDOI_CTRL, hmid_apdoi_ctrl_set);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_RSSI_A, hmid_rssi_a_factor_cal_set);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_RSSI_B, hmid_rssi_b_factor_cal_set);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_RSSI_C, hmid_rssi_c_factor_cal_set);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_TEMP_0, hmid_temp_0_set);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_TEMP_COFF_H, hmid_temp_coff_h_set);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_TEMP_COFF_L, hmid_temp_coff_l_set);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_ESC_THR, hmid_esc_thr_set);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_ROGUE_THR, hmid_rogue_thr_set);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_LEVEL_0_DAC, hmid_avg_level_0_dac_ref);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_AVG_LEVEL_1_DAC, hmid_avg_level_1_dac_ref);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_DACRANGE, hmid_dacrange_set);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_LOS_THR, hmid_lrl_los_thr_set);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_SAT_POS, hmid_lia_rssi_peakpos_sat_set);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_SAT_NEG, hmid_lia_rssi_peakneg_sat_set);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_EDGE_RATE, hmid_edge_rate_set);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_PREEMPHASIS_WEIGHT, hmid_preemphasis_weight_set);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_PREEMPHASIS_DELAY, hmid_preemphasis_delay_set);
    if (rc)
        return rc;
    if (pmd_wan_type == PMD_XGPON1_10_2_WAN)
    {
        /*
        Ignore duty cycle distortion calibration for all systems,
        except for XGPON1 10/2, since the transmitter data is now
        retimed in order to resolve any duty cycle distortion.
        */
        rc = set_param_from_flash(PMD_DUTY_CYCLE, hmid_duty_cycle_set);
        if (rc)
            return rc;
    }
    rc = set_param_from_flash(PMD_MPD_CALIBCTRL, hmid_calibctrl_set);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_TX_POWER, hmid_cal_meas_tx_power);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_BIAS0, hmid_cal_meas_bias0);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_TEMP_OFFSET, hmid_temp_offset_set);
    if (rc)
        return rc;
    /*rc = set_param_from_flash(PMD_BIAS_DELTA_I, hmid_cal_bias_delta_i);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_SLOPE_EFFICIENCY, hmid_cal_slope_efficiency);
    if (rc)
        return rc;*/
    /* set board type */
    /* rc = pmd_msg_handler(hmid_board_type_set, &pmd_wan_type, sizeof(uint16_t)); */
    rc = set_param_from_flash(PMD_ADF_LOS_THRESHOLDS, hmid_adf_los_thresholds_set);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_ADF_LOS_LEAKY_BUCKET, hmid_adf_los_leaky_bucket_set);
    if (rc)
        return rc;
    rc = set_param_from_flash(PMD_COMPENSATION, hmid_compensation_params_set);
    if (rc)
        return rc;

    return rc;
}

static int pmd_dev_init_seq(const uint32_t *pmd_res_temp_conv, const uint16_t *pmd_temp_apd_conv)
{
    int rc;
    uint16_t cal_valid = PMD_CALIBRATION_UNCALIBRATED;

    init_waitqueue_head(&pmd_reset_event);

    set_gpio_rstn(1);
    set_gpio_rstn(0);

    /* Set PLL */
    rc = ld_pll_init();
    if (rc)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Error in ld_pll_init \n");
        return -1;
    }

    rc = pmd_op_sw_download(&original_crc);
    if (rc)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Error in pmd_op_sw_download \n");
        return -1;
    }
    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "SW DOWNLOAD DONE \n");

    printk("PMD F/W DOWNLOAD DONE\n");

    rc = pmd_func_set();
    if (rc)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Error in pmd_func_set \n");
        return -1;
    }

    rc = crt_reset_config();
    if (rc)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Error in crt_reset_config \n");
        return -1;
    }

    /* wait till pmd finish init then continue with configuration*/
    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "Going to sleep, waiting for PMD reset normal indication \n");

    wait_event_interruptible(pmd_reset_event, pmd_reset_normal != 0);
    /* wait_event_interruptible_timeout(pmd_reset_event, pmd_reset_normal != 0, 5000); */

    pmd_reset_normal = 0;

    rc = pmd_set_init_val();
    if (rc)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Error in pmd_set_init_val \n");
        return -1;
    }

    if (pmd_res_temp_conv)
    {
        rc = pmd_op_temp_conv_table_download(pmd_res_temp_conv);
    }
    else
    {
        BCM_LOG_NOTICE(BCM_LOG_ID_PMD,
            "The temperature table is not valid in the PMD file, using the hardcoded default table\n");
        rc = pmd_op_temp_conv_table_download(default_pmd_res_temp_conv);
    }
    if (rc)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Error in pmd_op_temp_conv_table_download \n");
        return -1;
    }

    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "temperature convert table DOWNLOAD DONE \n");

    if (pmd_temp_apd_conv)
    {
        rc = pmd_op_temp_apd_conv_table_download(pmd_temp_apd_conv);
    }
    else
    {
        /* This code will execute also for boards with no APD to keep it clean and simple */
        BCM_LOG_NOTICE(BCM_LOG_ID_PMD,
            "The thermistor table is not valid in the PMD file, using the hardcoded default table\n");
        rc = pmd_op_temp_apd_conv_table_download(default_pmd_temp_apd_conv);
    }
    if (rc)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Error in pmd_op_temp_apd_conv_table_download \n");
        return -1;
    }
    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "temp to APD table DOWNLOAD DONE \n");

    /* Send configuration done message */
    cal_valid |= pmd_cal_is_config_valid(pmd_cal_rx_param_valid, sizeof(pmd_cal_rx_param_valid) / \
        sizeof(pmd_calibration_parameters_index)) == PMD_OPERATION_SUCCESS ? PMD_CALIBRATION_RX_DONE : 0;
    cal_valid |= pmd_cal_is_config_valid(pmd_cal_tx_param_valid, sizeof(pmd_cal_tx_param_valid) / \
        sizeof(pmd_calibration_parameters_index)) == PMD_OPERATION_SUCCESS ? PMD_CALIBRATION_TX_DONE : 0;

    switch (cal_valid)
    {
        case PMD_CALIBRATION_UNCALIBRATED:
        {
            printk("PMD is in pre calibration mode: No RX or TX calibrations were done\n");
            break;
        }
        case PMD_CALIBRATION_RX_DONE:
        {
            printk("PMD is in pre calibration mode: No TX calibration was done\n");
            break;
        }
        case PMD_CALIBRATION_TX_DONE:
        {
            printk("PMD is in pre calibration mode: No RX calibration was done\n");
            break;
        }
        case PMD_CALIBRATION_RXTX_DONE:
        {
            enforce_rssi_adf_los_backwards_compatibility();
            printk("PMD is calibrated\n");
            break;
        }
        default:
            printk("Invalid value %u\n", cal_valid);        
    }
    
    pmd_msg_handler(hmid_config_done, &cal_valid, sizeof(cal_valid));

    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "Configuration DOWNLOAD DONE \n");

    return rc;
}

static void enforce_rssi_adf_los_backwards_compatibility(void)
{
    pmd_calibration_parameters_index adf_los_param_valid[] = {PMD_ADF_LOS_THRESHOLDS, PMD_ADF_LOS_LEAKY_BUCKET};

    if (!pmd_cal_is_config_valid(adf_los_param_valid, sizeof(adf_los_param_valid) / \
        sizeof(pmd_calibration_parameters_index)))
    {
        printk("PMD ADF LOS calibration is valid\n");
        return;
    }

    printk("PMD ADF LOS calibration is not valid\n");
    calculate_and_update_adf_los_cal_params();
}

static void calculate_and_update_adf_los_cal_params(void)
{
    /*
    * It is assumed that:
    * los_assert_th_op_dbm = -35
    * los_deassert_th_op_dbm = -32
    *
    * constants: (based on assumption above)
    */
    const int32_t op_assert_th_1_16_uw_q8 = 1295;
    const int32_t op_deassert_th_1_16_uw_q8 = 2584;
    const int32_t scale_factor = 964010 >> 2; /* 1/lambda_zero/scaling_exp_pow */

    int32_t rssi_a_factor;
    int32_t rssi_b_factor;
    int32_t adf_los_assert_th;
    int32_t adf_los_deassert_th;

    pmd_cal_param_get(PMD_RSSI_A, &rssi_a_factor, 0);
    pmd_cal_param_get(PMD_RSSI_B, &rssi_b_factor, 0);
    rssi_b_factor >>= 2;

    /*
    * x = (op_assert_1_16_uw-rssi_a_factor)/rssi_b_factor
    * diff_sample = x/lambda_zero/scaling_exp_pow
    * diff_sample = x*scale_factor
    * diff_sample = (op_assert_1_16_uw-rssi_a_factor)/rssi_b_factor*scale_factor
    *             = (op_assert_1_16_uw-rssi_a_factor)*scale_factor/rssi_b_factor
    */
    adf_los_assert_th   = ((op_assert_th_1_16_uw_q8   - rssi_a_factor) * scale_factor) / rssi_b_factor;
    adf_los_deassert_th = ((op_deassert_th_1_16_uw_q8 - rssi_a_factor) * scale_factor) / rssi_b_factor;
   
    printk("PMD ADF LOS rssi_a_factor = 0x%x (%d)\n", rssi_a_factor, rssi_a_factor);
    printk("PMD ADF LOS rssi_b_factor = 0x%x (%d)\n", rssi_b_factor, rssi_b_factor);
    printk("PMD ADF LOS adf_los_assert_th   = 0x%x (%d)\n", adf_los_assert_th,   adf_los_assert_th  );
    printk("PMD ADF LOS adf_los_deassert_th = 0x%x (%d)\n", adf_los_deassert_th, adf_los_deassert_th);

    if (pmd_cal_param_set(PMD_ADF_LOS_THRESHOLDS, (adf_los_deassert_th << 16) | adf_los_assert_th, 0))
    {
        printk("PMD ADF LOS can't update pmd_adf_los_thresholds\n");
    }
    if (pmd_cal_param_set(PMD_ADF_LOS_LEAKY_BUCKET, (0x10 << 8) | 0xf, 0))
    {
        printk("PMD ADF LOS can't update pmd_adf_los_leaky_bucket\n");
    }

    if (set_param_from_flash(PMD_ADF_LOS_THRESHOLDS, hmid_adf_los_thresholds_set))
    {
        printk("PMD ADF LOS can't set pmd_adf_los_thresholds\n");
    }
    if (set_param_from_flash(PMD_ADF_LOS_LEAKY_BUCKET, hmid_adf_los_leaky_bucket_set))
    {
        printk("PMD ADF LOS can't set pmd_adf_los_leaky_bucket\n");
    }
}

static int pmd_dev_get_esc_alarm(void)
{
    uint16_t msg = 0;
    int rc;
    
#ifdef PMD_FULL_DEBUG
    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "\n");
#endif
    /* read and clear pmd interrupt */
    rc = pmd_msg_handler(hmid_esc_alarm_int_get, &msg, sizeof(uint16_t));
    if (rc)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, " Error clearing PMD ESC interrupt \n");
        return rc;
    }

    printk("ESC: esc_alarm_int = %x \n", msg);

    if (msg & PMD_ESC_BE_ALARM)
    {
        disable_tracking_flag = 1;
        printk("ESC: burst enable alarm \r\n");
    }
    if (msg & PMD_ESC_ROGUE_ALARM)
    {
        disable_tracking_flag = 1;
        printk("ESC: rogue alarm \r\n");
    }
    if (msg & PMD_ESC_MOD_OVR_CRNT)
    {
        disable_tracking_flag = 1;
        printk("ESC: modulation over current alarm \r\n");
    }
    if (msg & PMD_ESC_BIAS_OVR_CRNT)
    {
        disable_tracking_flag = 1;
        printk("ESC: bias over current alarm \r\n");
    }
    if (msg & PMD_ESC_ALARM_MPD_FAULT)
    {
        disable_tracking_flag = 1;
        printk("ESC: mpd fault alarm \r\n");
    }
    if (msg & PMD_ESC_ALARM_EYESAFE)
    {
        disable_tracking_flag = 1;
        printk("ESC: eye safety alarm \r\n");
    }

    return PMD_OPERATION_SUCCESS;
}

static int pmd_dev_get_task_stuck_alarm(void)
{
    int rc;
    uint16_t msg = 0;

    rc = pmd_msg_handler(hmid_task_stuck_get, &msg, sizeof(uint16_t));
    if (rc)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, " Error reading which task is stuck \n");
        return rc;
    }
    if (msg & PMD_IDLE_TASK_STUCK_ALARM)
        printk("PMD: Idle task stuck\n");
    if (msg & PMD_LASER_TASK_STUCK_ALARM)
        printk("PMD: laser task stuck\n");
    if (msg & PMD_RSSI_TASK_STUCK_ALARM)
        printk("PMD: rssi task stuck\n");
    if (msg & PMD_TIMER_TASK_STUCK_ALARM)
        printk("PMD: timer task stuck\n");
    return rc;
}

static int pmd_dev_get_cal_error_alarm(void)
{
    int i, rc;
    uint16_t msg = 0;

    rc = pmd_msg_handler(hmid_cal_error_get, &msg, sizeof(uint16_t));
    if (rc)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, " Error reading which cal error \n");
        return rc;
    }

    if (msg == 0)
    {
        pmdmsg_write_calibration_state("PMD: Calibration state over");
        return rc;
    }

    for (i=1; i<CAL_ERROR_SIZE; i++)
    {
        if (msg == pmd_cal_ret[i].error_num)
        {
            char failure_message[MAX_PMD_MESSAGE_LENGTH];
            snprintf(failure_message, MAX_PMD_MESSAGE_LENGTH, "PMD: Calibration failure [%d]: %s",
                pmd_cal_ret[i].error_num, pmd_cal_ret[i].error_message);
            pmdmsg_write_calibration_state(failure_message);
            break;
        }
    }

    return rc;
}

static void pmd_dev_get_first_burst_done_alarm(void)
{
    pmd_msg_handler(hmid_temperature_get, &pmd_ext_tracking_temp, sizeof(uint16_t));
    pmd_state = pmd_tracking_state_tracking;

    /* notify stack */
    pmd_fb_done = true;

    pon_set_pmd_fb_done_wrapper(1);
}

#define REORDER_UINT32(in) (((in) & 0x000000ff) << 24) | (((in) & 0x0000ff00) << 8) | (((in) & 0x00ff0000) >> 8) | \
    (((in) & 0xff000000) >> 24)
static int pmd_dev_alarm(uint16_t alarm)
{
    int rc = PMD_OPERATION_SUCCESS;
    uint32_t pmd_crc;

    if (alarm & PMD_RESET_NORMAL)
    {
        RU_REG_READ(LD_LIA, LD_LIA_PARAM, &pmd_crc);
#ifndef __LITTLE_ENDIAN
        pmd_crc = REORDER_UINT32(pmd_crc);
#endif
        if (original_crc != pmd_crc)
        {
            printk("\nPMD: Software download failed. host crc = %u, pmd crc = %u\n", original_crc, pmd_crc);
            return -1;
        }
        pmd_reset_normal = 1;
        wake_up_interruptible(&pmd_reset_event);
        printk("PMD: Reset normal\n");
    }
    if (alarm & PMD_RESET_WATCHDOD)
        printk("PMD: Reset watchdog \n");
    if (alarm & PMD_FATAL_ERROR)
        printk("PMD: Fatal error \n");
    if (alarm & PMD_EYE_SAFETY)
        rc = pmd_dev_get_esc_alarm();
    if (alarm & PMD_OTP_FAILURE)
        printk("PMD: OTP failure \n");
    if (alarm & PMD_LOSS_SIGNAL)
        printk("PMD: Signal loss \n");
    if (alarm & PMD_MEM_DATA_TEST_FAIL)
        printk("PMD: Memory data test error \n");
    if (alarm & PMD_MEM_ADDR_TEST_FAIL)
        printk("PMD: Memory address test error \n");
    if (alarm & PMD_MEM_CELL_TEST_FAIL)
        printk("PMD: Memory cell test error \n");
    if (alarm & ALM_MPD_POWER_UP_FAIL)
        printk("PMD: MPD power up failure \n");
    if (alarm & ALM_TASK_STUCK_ERROR)
        rc |= pmd_dev_get_task_stuck_alarm();
    if (alarm & ALM_CAL_STATE_OVER)
        rc |= pmd_dev_get_cal_error_alarm();
    if (alarm & ALM_THERMISTOR_MISBEHAVIOUR)
    {
        char buf[32];
        /* Read from PMD client 2 (DRAM) some debug data for the thermistor resistance */
        pmd_op_i2c(2, 0x810c, buf, 28, PMD_READ_OP);
        
        /* Parsing the buffer ignoring endianness issues */
        printk("PMD: Thermistor misbehaviour:\n");
        printk("PMD: ext_temp_res = 0x%02X%02X%02X%02X\n", buf[0] , buf[1] , buf[2] , buf[3] );
        printk("PMD: therm_refbot = 0x%02X%02X%02X%02X\n", buf[8] , buf[9] , buf[10], buf[11]);
        printk("PMD: therm_adc    = 0x%02X%02X%02X%02X\n", buf[12], buf[13], buf[14], buf[15]);
        printk("PMD: therm_reftop = 0x%02X%02X%02X%02X\n", buf[16], buf[17], buf[18], buf[19]);
        printk("PMD: therm_down   = 0x%02X%02X%02X%02X\n", buf[20], buf[21], buf[22], buf[23]);
        printk("PMD: therm_up     = 0x%02X%02X%02X%02X\n", buf[24], buf[25], buf[26], buf[27]);
    }
    if (alarm & PMD_FIRST_BURST_COMPLETE)
    {
        printk("PMD: First Burst Complete \n");
        pmd_dev_get_first_burst_done_alarm();
    }

    return rc;
}

/* this function changes the PMD bursten and eye safety mode according to the prbs, misc mode */
void pmd_dev_enable_prbs_or_misc_mode(uint16_t enable, uint8_t prbs_mode)
{
    uint16_t enable_esc;

    if (READ_LOCK_WAS_NOT_OBTAINED == exported_symbol_read_trylock())
    {
        BCM_LOG_NOTICE(BCM_LOG_ID_PMD, "It is not allowed to invoke the exported symbol %s during PMD reset\n",
            __func__);
        return;
    }

    printk("PMD: pmd_dev_enable_prbs_or_misc_mode: enable = %d, prbs_mode = %d\n", enable, prbs_mode);

    if (enable)
        enable_esc = 0;
    else
        enable_esc = 1;
        
    pmd_msg_handler(hmid_esc_block_enable_set, &enable_esc, sizeof(uint16_t));

    if (prbs_mode)
        pmd_dev_ewake_mac_control(!enable);

    if (!enable && in_calibration)
            pmd_dev_ewake_mac_control(0);

    pmd_msg_handler(hmid_burst_en_dep_set, &enable, sizeof(uint16_t));

    exported_symbol_read_unlock();
}
EXPORT_SYMBOL(pmd_dev_enable_prbs_or_misc_mode);

static int __pmd_dev_enable_prbs_or_misc_mode(void *ptr)
{
	pmd_pbrs_param *params;

	params=(pmd_pbrs_param*)ptr;
	if(ptr != NULL)
		pmd_dev_enable_prbs_or_misc_mode(params->enable, params->prbs_mode);
	return 0;
}

static void pmd_dev_first_burst_temp_dep(void)
{
    int16_t cur_temp;
    uint8_t delta_temp = 0;

    /* compare retrieved temp with cur temp */
    pmd_msg_handler(hmid_temperature_get, &cur_temp, 2);

    delta_temp = abs(cur_temp - pmd_ext_tracking_temp);

    if (delta_temp <= PMD_TRACKING_DELTA_TEMP )
    {
        /* temp didn't change much - continue with tracking */
        pmd_state = pmd_tracking_state_tracking;
        goto update_pmd_state;
    }

    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "cur temperature %d last tracking temperature %d delta %d \n", cur_temp,
        pmd_ext_tracking_temp, delta_temp);

    printk("PMD: change to first_burst\n");

    pmd_ext_tracking_temp = TEMP_NOT_SET;
    pmd_fb_done = false;

    pon_set_pmd_fb_done_wrapper(0);

    pmd_state = pmd_tracking_state_first_burst;

update_pmd_state:
    pmd_dev_update_pmd_fw_tracking();
}

static void pmd_dev_o2_awaiting_register_action(void)
{
    int rc = 0;

    rc = pmd_dev_update_pmd_fw_force(0);
    temp_poll = 0;

    if (pmd_fb_done)
    {
        printk("PMD: pmd_fb_done TRUE, pmd_state = %d\n", pmd_state);

        pmd_dev_first_burst_temp_dep();

        if (pmd_state == pmd_tracking_state_tracking)
        {
            num_of_ranging_attempts++;
            if (num_of_ranging_attempts == MAX_NUM_RANGING_ATTEMPTS)
            {
                printk("PMD: change to FB since MAX_NUM_RANGING_ATTEMPTS\n");
                pmd_ext_tracking_temp = TEMP_NOT_SET;
                pmd_fb_done = false;

                pon_set_pmd_fb_done_wrapper(0);

                pmd_state = pmd_tracking_state_first_burst;
                num_of_ranging_attempts = 0;
            }
        }
    }
    else
    {
        printk("PMD: pmd_fb_done FALSE, pmd_state = %d\n", pmd_state);
        if (pmd_state != pmd_tracking_state_first_burst)
        {
            pmd_ext_tracking_temp = TEMP_NOT_SET;
            pmd_fb_done = false;

            pon_set_pmd_fb_done_wrapper(0);

            pmd_state = pmd_tracking_state_first_burst;
        }
    }
}

static void pmd_dev_o5_registered_action(void)
{
    /* Enable force after ranging */
    pmd_dev_update_pmd_fw_force(1);

    num_of_ranging_attempts = 0;

    temp_poll = 1;
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
void __pmd_dev_change_tracking_state(uint32_t old_state, uint32_t new_state);
static uint32_t __pmd_dev_old_tracking_state;
static uint32_t __pmd_dev_new_tracking_state;

static int pon_message_handler_thread(void *arg)
{
    int ret, msg_bm;
    

    while (1)
    {
        ret = wait_event_interruptible(pon_msg_event_wq, __pon_msg_type_bit_mask || kthread_should_stop());
        if (ret) continue;

        if (kthread_should_stop())
        {
            BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "kthread_should_stop detected on pon_message_handler_thread\n");
            return PMD_OPERATION_SUCCESS;
        }

        spin_lock_bh(&pon_message_bh_spinlock);

        msg_bm = __pon_msg_type_bit_mask;
        __pon_msg_type_bit_mask = 0;

        if (PON_CHANGE_TRCK_STAT_INDICATION & msg_bm)
        {
            __pmd_dev_change_tracking_state(__pmd_dev_old_tracking_state, __pmd_dev_new_tracking_state);
        }

        spin_unlock_bh(&pon_message_bh_spinlock);
    }

    return PMD_OPERATION_SUCCESS;
}

void pmd_dev_change_tracking_state(uint32_t old_state, uint32_t new_state)
{
    spin_lock_bh(&pon_message_bh_spinlock);
    __pmd_dev_old_tracking_state = old_state;
    __pmd_dev_new_tracking_state = new_state;
    __pon_msg_type_bit_mask |= PON_CHANGE_TRCK_STAT_INDICATION;
    spin_unlock_bh(&pon_message_bh_spinlock);

    if (pmd_wan_type != PMD_AUTO_DETECT_WAN)
        wake_up_interruptible(&pon_msg_event_wq);    
}
#else
#define __pmd_dev_change_tracking_state pmd_dev_change_tracking_state
#endif


void __pmd_dev_change_tracking_state(uint32_t old_state, uint32_t new_state)
{
    if (READ_LOCK_WAS_NOT_OBTAINED == exported_symbol_read_trylock())
    {
        BCM_LOG_NOTICE(BCM_LOG_ID_PMD, "It is not allowed to invoke the exported symbol %s during PMD reset\n",
            __func__);
        return;
    }

    printk("PMD: PON state change: old = %d, new = %d\n", old_state, new_state);

    if ((pmd_wan_type == PMD_GPON_2_1_WAN) || (pmd_wan_type == PMD_XGPON1_10_2_WAN))
    {
        switch(new_state)
        {
            case PMD_GPON_STATE_STANDBY_O2:
            {
                pmd_dev_o2_awaiting_register_action();
                break;
            }
            case PMD_GPON_STATE_OPERATION_O5:
            {
                pmd_dev_o5_registered_action();
                break;
            }
            default:
            {
                switch(old_state)
                {
                    case PMD_GPON_STATE_OPERATION_O5:
                    {
                        temp_poll = 0;
                        break;
                    }
                    case PMD_GPON_STATE_STANDBY_O2:
                    case PMD_GPON_STATE_SERIAL_NUMBER_O3:
                    case PMD_GPON_STATE_RANGING_O4:
                    case PMD_GPON_STATE_POPUP_O6:
                    {
                        if (new_state == PMD_GPON_STATE_INIT_O1 || new_state == PMD_GPON_STATE_EMERGENCY_STOP_O7 )
                            pmd_state = pmd_tracking_state_disabled;

                        break;
                    }
                    default:
                        break;
                }
                break;
            }
        }
    }
    else if ((pmd_wan_type == PMD_EPON_1_1_WAN) || ((pmd_wan_type == PMD_EPON_10_1_WAN)))
    {
        switch((rdpa_epon_link_mpcp_state)new_state)
        {
            case rdpa_epon_link_unregistered:
            {
                temp_poll = 0;
                pmd_state = pmd_tracking_state_disabled;
                break;
            }
            case rdpa_epon_link_awaiting_register:
            {
                pmd_dev_o2_awaiting_register_action();
                break;
            }
            case rdpa_epon_link_in_service:
            {
                pmd_dev_o5_registered_action();
                break;
            }
            default:
            {
                temp_poll = 0;
                break;
            }
        }
    }

    pmd_dev_update_pmd_fw_tracking();

    exported_symbol_read_unlock();
}
EXPORT_SYMBOL(pmd_dev_change_tracking_state);


static void pmd_dev_isr_callback(int irq)
{
    uint16_t msg = 0;
    int rc;

#ifdef PMD_FULL_DEBUG
    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "\n");
#endif

    /* read and clear pmd interrupt */
    rc = pmd_msg_handler(hmid_alarms_get, &msg, sizeof(uint16_t));
    if (rc)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, " Error clearing PMD interrupt \n");
        goto exit;
    }

    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "msg %x \n", msg);

    rc = pmd_dev_alarm(msg);
    if (rc)
        goto exit;

    if (alarm_read_flag)
    {
        alarm_msg = msg;
        alarm_read_flag = 0;
    }

exit:
    bcm_bca_extintr_unmask(irq);
}

int pmd_dev_poll_epon_alarm(void)
{
    if (READ_LOCK_WAS_NOT_OBTAINED == exported_symbol_read_trylock())
    {
        BCM_LOG_NOTICE(BCM_LOG_ID_PMD, "It is not allowed to invoke the exported symbol %s during PMD reset\n",
            __func__);
        return PMD_OPERATION_SUCCESS;
    }

    alarm_read_flag = 1;
    exported_symbol_read_unlock();

    return alarm_msg;
}
EXPORT_SYMBOL(pmd_dev_poll_epon_alarm);

static irqreturn_t pmd_dev_threaded_isr(int irq, void *priv)
{
#if defined(CONFIG_BCM_PON_XRDP) || defined(CONFIG_BCM963158)
    bcm_bca_extintr_mask(irq);
    bcm_bca_extintr_clear(irq);
#endif

    return IRQ_WAKE_THREAD;
}

irqreturn_t pmd_dev_alarm_handler(int irq, void *dev_id)
{
    pmd_dev_isr_callback(irq);
    return IRQ_HANDLED;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
static void pmd_dev_timer_handler(struct timer_list *data)
#else
static void pmd_dev_timer_handler(unsigned long data)
#endif
{
    /* call thread */
    pmd_thread_sched |= PMD_TIMER_INDICATION;

    wake_up_interruptible(&pmd_thread_event);

    /*reset timer */
    mod_timer(&periodic_timer, jiffies + msecs_to_jiffies(PMD_TIMER_INTERVAL));
}

static void pmd_dev_timer_init(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
    timer_setup(&periodic_timer, pmd_dev_timer_handler, 0);
#else
    setup_timer(&periodic_timer, pmd_dev_timer_handler, 0);
#endif
    mod_timer(&periodic_timer, jiffies + msecs_to_jiffies(PMD_TIMER_INTERVAL));
}

static ssize_t pmdmsg_read(struct file *file, char __user *ubuf, size_t _user_buffer_size, loff_t *ppos)
{
    /* This is an implementation for PMD calibration scripts support.
     * No reentrancy and no race conditions are treated in this implementation as they are not expected in our use case.
    **/
    int pmd_message_length, remaining_message_length, user_buffer_size, copy_to_user_length, error;
    static int next_pointer_to_pmd_message = 0;
    int copy_pointer_to_pmd_message;

    user_buffer_size = _user_buffer_size; /* size_t has different types in different profiles */
    error = pmdmsg_try_single_user_session();
    if (error)
    {
        return (ssize_t)error;
    }

    if (!next_pointer_to_pmd_message)
    {
        /* whole message or first fragment */
        printk( KERN_DEBUG "PMD read handler: enter wait_event_interruptible, last read_pmd_whole_messages_counter = %d\n",
                read_pmd_whole_messages_counter);
        error = wait_event_interruptible(pmd_log_wait, is_pmd_message_ready);
        if (1 < is_pmd_message_ready)
        {
            printk( KERN_ERR "PMD read handler: %d messages were lost since the last reading.\n", is_pmd_message_ready
                - 1);
        }
        is_pmd_message_ready = 0;
        if (error)
        {
            printk( KERN_DEBUG "PMD read handler: wait_event_interruptible error, last read_pmd_whole_messages_counter = %d\n",
                read_pmd_whole_messages_counter);
            pmdmsg_check_session_statistics_accounting_stability(ppos);
            pmdmsg_print_session_report(ppos);
            pmdmsg_accumulate_session_statistics(ppos);
            pmdmsg_init_session();
            return PMD_OPERATION_SUCCESS;
        }
    }

    pmd_message_length = strlen(pmd_message_buffer);
    remaining_message_length = pmd_message_length - next_pointer_to_pmd_message;
    copy_pointer_to_pmd_message = next_pointer_to_pmd_message;
    if (remaining_message_length > user_buffer_size)
    {
        /* all fragments except for the last fragment */
        copy_to_user_length = user_buffer_size;
        pmd_read_fragmented_bytes_counter += copy_to_user_length;
        pmd_read_fragments_counter++;

        printk( KERN_DEBUG "\n\nPMD read handler: remaining_message_length=%d > user_buffer_size=%d, "
            "next_pointer_to_pmd_message=%d\n\n", remaining_message_length, user_buffer_size,
            next_pointer_to_pmd_message);

        if (0 == next_pointer_to_pmd_message)
        {
            /* first fragment */
            pmd_read_fragmented_messages_counter++;
        }

        next_pointer_to_pmd_message += user_buffer_size;
    }
    else
    {
        /* whole message or last fragment */
        copy_to_user_length = remaining_message_length;

        if (next_pointer_to_pmd_message)
        {
            /* last fragment */
            pmd_read_fragments_counter++;
            pmd_read_fragmented_bytes_counter += copy_to_user_length;
            printk( KERN_DEBUG "\n\nPMD read handler: last fragment at next_pointer_to_pmd_message=%d, "
                "last read_pmd_whole_messages_counter = %d\nremaining data\n'%s'\n\n", next_pointer_to_pmd_message,
                read_pmd_whole_messages_counter, pmd_message_buffer + next_pointer_to_pmd_message);

            next_pointer_to_pmd_message = 0;
        }
        else
        {
            /* whole message */
            read_pmd_whole_messages_counter++;
        }
    }

    printk( KERN_DEBUG "PMD read handler: user_buffer_size = %d, *ppos = %lld, read_pmd_whole_messages_counter = %d\n",
        user_buffer_size, *ppos, read_pmd_whole_messages_counter);
    printk( KERN_DEBUG "PMD read handler: read_pmd_whole_messages_counter = %d, complete pmd_message_buffer[pmd_message_length=%d]"
        " |%s\n", read_pmd_whole_messages_counter, pmd_message_length, pmd_message_buffer);

    if(copy_to_user(ubuf, pmd_message_buffer + copy_pointer_to_pmd_message, copy_to_user_length))
    {
        printk( KERN_ERR "PMD read handler copy_to_user failed, read_pmd_whole_messages_counter = %d\n",
            read_pmd_whole_messages_counter);
        return -EFAULT;
    }

    return copy_to_user_length;
}

static ssize_t pmdrst_read(struct file *file, char __user *ubuf, size_t _user_buffer_size, loff_t *ppos)
{
    int copy_to_user_length = 0;
    char pmdrst_buf[MAX_PMD_MESSAGE_LENGTH];

    if (*ppos > 0 || _user_buffer_size < MAX_PMD_MESSAGE_LENGTH)
        return 0;

    copy_to_user_length = snprintf(pmdrst_buf, MAX_PMD_MESSAGE_LENGTH, "%d\n", pmd_reset_mode);

    if (copy_to_user(ubuf, pmdrst_buf, copy_to_user_length))
        return -EFAULT;

    *ppos = copy_to_user_length;

	return copy_to_user_length;
}

static ssize_t pmdrst_write(struct file *file, const char __user *ubuf, size_t _user_buffer_size, loff_t *ppos)
{
    int new_pmd_reset_mode, items;
    char reset_msg[12];

    if (0 < *ppos || 8 < _user_buffer_size)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "PMD reset write handler error.\n");
        return -EFAULT;
    }
    if(copy_from_user(reset_msg, ubuf, _user_buffer_size))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "PMD reset write handler copy_from_user failed\n");
        return -EFAULT;
    }
    reset_msg[8] = 0;
    items = sscanf(reset_msg, "%d", &new_pmd_reset_mode);
    if (1 != items)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "PMD reset write handler invalid input error.\n");
        return -EFAULT;
    }

    if (new_pmd_reset_mode)
    {
        if (pmd_reset_mode)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_PMD, "PMD reset write handler error: already in reset mode.\n");
            return -EFAULT;
        }
        set_gpio_rstn(1);
        BCM_LOG_NOTICE(BCM_LOG_ID_PMD, "PMD is in reset mode\n");
    }
    else
    {
        if (!pmd_reset_mode)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_PMD, "PMD reset write handler error: already in out of reset mode.\n");
            return -EFAULT;
        }
        pmd_dev_reconfigure_wan_type(pmd_wan_type, &static_calibration_parameters_from_json);
        BCM_LOG_NOTICE(BCM_LOG_ID_PMD, "PMD is in out of reset mode\n");
    }

    *ppos = _user_buffer_size;
    return _user_buffer_size;
}

static int pmdmsg_try_single_user_session(void)
{
    if (!pmd_messaging_session_pid)
    {
        pmd_messaging_session_pid = current->tgid;
        printk( KERN_INFO "Wellcome to PMD calibration messaging! This is session #%d\n",
            pmd_messaging_sessions_counter);
        return PMD_OPERATION_SUCCESS;
    }

    if (current->tgid != pmd_messaging_session_pid)
    {
        printk( KERN_ERR "Only a single session is supported in the PMD calibration messaging!\nThe current active "
            "session is #%d on PID %d your PID is %d\n", pmd_messaging_sessions_counter, pmd_messaging_session_pid,
            current->tgid);
        return -EBUSY;
    }

    return PMD_OPERATION_SUCCESS;
}

static void pmdmsg_print_session_report(const loff_t *ppos)
{
    loff_t read_whole_pmd_messages_byte_counter;

    read_whole_pmd_messages_byte_counter = *ppos - pmd_read_fragmented_bytes_counter;

    printk( KERN_INFO "Thank you for using the PMD calibration messaging! In this session #%d you had:\n"
        "%d whole messages read (%d in total)\n%lld whole messages bytes read (%lld in total)\n%d overwritten messages "
        "(%d in total)\n%lld overwritten bytes (%lld in total)\n%d fragmented messages read (%d in total)\n%lld "
        "fragmented bytes read (%lld in total)\n%d fragments read (%d in total)\n%d messages written (%d in total)\n"
        "%lld bytes written (%lld in total)\n", pmd_messaging_sessions_counter,
        read_pmd_whole_messages_counter, previous_session_read_whole_messages_counter + read_pmd_whole_messages_counter,
        read_whole_pmd_messages_byte_counter, previous_session_read_whole_messages_byte_counter +
            read_whole_pmd_messages_byte_counter,
        overwritten_pmd_messages_counter, previous_session_overwritten_messages_counter +
            overwritten_pmd_messages_counter,
        overwritten_pmd_bytes_counter, previous_session_overwritten_bytes_counter + overwritten_pmd_bytes_counter,
        pmd_read_fragmented_messages_counter, previous_session_read_fragmented_messages_counter +
            pmd_read_fragmented_messages_counter,
        pmd_read_fragmented_bytes_counter, previous_session_read_fragmented_bytes_counter +
            pmd_read_fragmented_bytes_counter,
        pmd_read_fragments_counter, previous_session_read_fragments_counter + pmd_read_fragments_counter,
        written_pmd_messages_counter, previous_session_written_messages_counter + written_pmd_messages_counter,
        written_pmd_bytes_counter, previous_session_written_bytes_counter + written_pmd_bytes_counter);
}

static void pmdmsg_check_session_statistics_accounting_stability(const loff_t *ppos)
{
    loff_t read_whole_pmd_messages_byte_counter;
    int pmd_statistics_error;

    read_whole_pmd_messages_byte_counter = *ppos - pmd_read_fragmented_bytes_counter;
    pmd_statistics_error = 0;

    if (written_pmd_bytes_counter != overwritten_pmd_bytes_counter + read_whole_pmd_messages_byte_counter +
        pmd_read_fragmented_bytes_counter)
    {
        printk( KERN_ERR "Error: PMD messages bytes statistics accounting stability check failed!\n");
        printk( KERN_ERR "written_pmd_bytes_counter (%lld) != overwritten_pmd_bytes_counter (%lld) + "
            "read_whole_pmd_messages_byte_counter (%lld) + pmd_read_fragmented_bytes_counter (%lld)\n",
            written_pmd_bytes_counter, overwritten_pmd_bytes_counter, read_whole_pmd_messages_byte_counter,
            pmd_read_fragmented_bytes_counter);
        pmd_statistics_error++;
    }
    if (written_pmd_messages_counter != read_pmd_whole_messages_counter + overwritten_pmd_messages_counter +
        pmd_read_fragmented_messages_counter)
    {
        printk( KERN_ERR "Error: PMD messages counters statistics accounting stability check failed!\n");
        printk( KERN_ERR "written_pmd_messages_counter (%d) != read_pmd_whole_messages_counter (%d) + "
            "overwritten_pmd_messages_counter (%d) + pmd_read_fragmented_messages_counter (%d)\n",
            written_pmd_messages_counter, read_pmd_whole_messages_counter, overwritten_pmd_messages_counter,
            pmd_read_fragmented_messages_counter);
        pmd_statistics_error++;
    }
    if (previous_session_written_bytes_counter != previous_session_read_whole_messages_byte_counter +
        previous_session_overwritten_bytes_counter + previous_session_read_fragmented_bytes_counter)
    {
        printk( KERN_ERR "Error: PMD previous messages bytes statistics accounting stability check failed!\n");
        printk( KERN_ERR "previous_session_written_bytes_counter (%lld) != "
            "previous_session_read_whole_messages_byte_counter (%lld)+ previous_session_overwritten_bytes_counter "
            "(%lld) + previous_session_read_fragmented_bytes_counter (%lld)\n",
            previous_session_written_bytes_counter, previous_session_read_whole_messages_byte_counter,
            previous_session_overwritten_bytes_counter, previous_session_read_fragmented_bytes_counter);
        pmd_statistics_error++;
    }
    if (previous_session_written_messages_counter != previous_session_read_whole_messages_counter +
        previous_session_overwritten_messages_counter + previous_session_read_fragmented_messages_counter)
    {
        printk( KERN_ERR "Error: PMD previous messages counters statistics accounting stability check failed!\n");
        printk( KERN_ERR "previous_session_written_messages_counter (%d) != "
            "previous_session_read_whole_messages_counter (%d) + previous_session_overwritten_messages_counter (%d) + "
            "previous_session_read_fragmented_messages_counter (%d)\n",
            previous_session_written_messages_counter, previous_session_read_whole_messages_counter,
            previous_session_overwritten_messages_counter, previous_session_read_fragmented_messages_counter);
        pmd_statistics_error++;
    }

    if (!pmd_statistics_error)
    {
        printk( KERN_DEBUG "PMD messages statistics accounting stability check passed successfully for session #%d\n",
            pmd_messaging_sessions_counter);        
    }
}

static void pmdmsg_accumulate_session_statistics(const loff_t *ppos)
{
    loff_t read_whole_pmd_messages_byte_counter;

    read_whole_pmd_messages_byte_counter = *ppos - pmd_read_fragmented_bytes_counter;

    pmd_messaging_sessions_counter++;
    previous_session_read_whole_messages_counter += read_pmd_whole_messages_counter;
    previous_session_read_whole_messages_byte_counter += read_whole_pmd_messages_byte_counter;
    previous_session_overwritten_messages_counter += overwritten_pmd_messages_counter;
    previous_session_overwritten_bytes_counter += overwritten_pmd_bytes_counter;    
    previous_session_read_fragmented_messages_counter += pmd_read_fragmented_messages_counter;
    previous_session_read_fragmented_bytes_counter += pmd_read_fragmented_bytes_counter;
    previous_session_read_fragments_counter += pmd_read_fragments_counter;
    previous_session_written_messages_counter += written_pmd_messages_counter;
    previous_session_written_bytes_counter += written_pmd_bytes_counter;
}

static void pmdmsg_init_session(void)
{
    pmd_messaging_session_pid = 0;
    read_pmd_whole_messages_counter = 0;
    overwritten_pmd_messages_counter = 0;
    overwritten_pmd_bytes_counter = 0;
    pmd_read_fragmented_bytes_counter = 0;
    pmd_read_fragmented_messages_counter = 0;
    pmd_read_fragments_counter = 0;
    written_pmd_messages_counter = 0;
    written_pmd_bytes_counter = 0;
}

static void pmdmsg_write_calibration_state(const char *calibration_state_message)
{
    /* This is an implementation for PMD calibration scripts support.
     * No reentrancy and no race conditions are treated in this implementation as they are not expected in our use case.
    **/
    int pmd_message_length;

    printk(calibration_state_message);

    printk( KERN_DEBUG "PMD write: is_pmd_message_ready = %d", is_pmd_message_ready);
    if (is_pmd_message_ready)
    {
        overwritten_pmd_messages_counter += 1;
        overwritten_pmd_bytes_counter += strlen(pmd_message_buffer);
        printk( KERN_ERR "PMD write: overwritting previous unread message\n%s\n"
            "overwritten_pmd_messages_counter=%d\n", pmd_message_buffer, overwritten_pmd_messages_counter);
    }

    if (NO_LAST_HOST_CAL_SET_MESSAGE == last_hmid_cal_set_argument)
    {
        pmd_message_length = snprintf(pmd_message_buffer, MAX_PMD_MESSAGE_LENGTH, "%s, ERROR: No last host cal set "
            "message! message #%d\n", calibration_state_message, written_pmd_messages_counter);
    }
    else
    {
        pmd_message_length = snprintf(pmd_message_buffer, MAX_PMD_MESSAGE_LENGTH, "%s, last host cal set message "
            "argument=0x%x, message #%d\n", calibration_state_message, last_hmid_cal_set_argument,
            written_pmd_messages_counter);
        last_hmid_cal_set_argument = NO_LAST_HOST_CAL_SET_MESSAGE;
    }
    written_pmd_messages_counter++;
    written_pmd_bytes_counter += pmd_message_length;
    printk( KERN_DEBUG "PMD write: written_pmd_messages_counter = %d, complete pmd_message_buffer"
        "[pmd_message_length=%d] |%s\n", written_pmd_messages_counter, pmd_message_length, pmd_message_buffer);

    is_pmd_message_ready += 1;
    wake_up_interruptible(&pmd_log_wait);
}

#define PROC_DIR           "pmd"

static struct proc_dir_entry *proc_dir;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
static const struct proc_ops proc_pmdmsg_operations = {
    .proc_read       = pmdmsg_read,
};

static const struct proc_ops proc_pmdrst_operations = {
    .proc_read       = pmdrst_read,
    .proc_write      = pmdrst_write,
};
#else
static const struct file_operations proc_pmdmsg_operations = {
    .read       = pmdmsg_read,
};

static const struct file_operations proc_pmdrst_operations = {
    .read       = pmdrst_read,
    .write      = pmdrst_write,
};
#endif

static int pmd_dev_create_proc(void)
{
    proc_dir = proc_mkdir(PROC_DIR, NULL);
    if (!proc_dir) 
    {
        printk(KERN_ERR "Failed to create pmd proc directory %s.\n", PROC_DIR);
        return -1;
    }

    pmd_msg_procfs_entry = proc_create("calibration", S_IRUSR, proc_dir, &proc_pmdmsg_operations);
    if (!pmd_msg_procfs_entry)
    {
        printk(KERN_ERR "\nPMD module failed to create a procfs calibration entry.\n");
        return -1;
    }

    pmd_rst_procfs_entry = proc_create("reset", S_IRUSR | S_IWUSR, proc_dir, &proc_pmdrst_operations);
    if (!pmd_rst_procfs_entry)
    {
        printk(KERN_ERR "\nPMD module failed to create a procfs reset entry.\n");
        return -1;
    }

    return 0;
}

static void pmd_dev_remove_proc(void)
{
    if (pmd_msg_procfs_entry)
    {
        proc_remove(pmd_msg_procfs_entry);
        pmd_msg_procfs_entry = NULL;
    }

    if (pmd_rst_procfs_entry)
    {
        proc_remove(pmd_rst_procfs_entry);
        pmd_rst_procfs_entry = NULL;
    }

    if (proc_dir)
    {
        remove_proc_entry(PROC_DIR, NULL);
        proc_dir = NULL;
    }	
}

static void pmd_dev_exit_steps(void)
{
    BCM_LOG_DEBUG(BCM_LOG_ID_PMD, "\n");

    del_timer(&periodic_timer);

    if (pmd_timer_thread)
    {
        kthread_stop(pmd_timer_thread);
        pmd_timer_thread = 0;
    }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
    if (pon_msg_event_thread)
    {
        kthread_stop(pon_msg_event_thread);
        pon_msg_event_thread = 0;        
    }
#endif
    if (g_pdata)
    {
        ewake_put();
    }
}

static void pmd_dev_exit(void)
{
    pmd_dev_remove_proc();

    pmd_dev_exit_steps();

    bcm_bca_extintr_free(g_pdata->dev, g_pdata->alarm_irq, 0);

    mutex_destroy(&pmd_dev_lock);

    unregister_chrdev(PMD_DEV_MAJOR, PMD_DEV_CLASS);

    printk(KERN_INFO "\nPMD module unloaded.\n");
}

static void pmd_dev_init_globals(void)
{
    pmd_state = pmd_tracking_state_disabled;
    pmd_prev_state = pmd_tracking_state_disabled;
    pmd_fb_done = false;
    num_of_ranging_attempts = 0;
    temp_poll = 0;
    in_calibration = 0;
    disable_tracking_flag = 0;
    data_col_flag = 0;
    pmd_ext_tracking_temp = TEMP_NOT_SET;
    alarm_read_flag = 1;
    is_pmd_message_ready = 0;
    last_hmid_cal_set_argument = NO_LAST_HOST_CAL_SET_MESSAGE;
}

static int pmd_dev_init_steps(pmd_calibration_parameters *calibration_parameters_from_json)
{
    int rc;
    uint32_t *pmd_res_temp_conv;
    uint16_t *pmd_temp_apd_conv;

    if (PMD_AUTO_DETECT_WAN == pmd_wan_type)
    {
        return PMD_OPERATION_SUCCESS;
    }

    printk("PMD: wan_type = %s\n", pmd_wan_type_string[pmd_wan_type]);

    init_waitqueue_head(&pmd_log_wait);

    pmd_dev_init_globals();
    msg_system_init();

    pmd_cal_param_init(calibration_parameters_from_json);

    bcmFun_reg(BCM_FUN_ID_PMD_PRBS,__pmd_dev_enable_prbs_or_misc_mode);

    rc = pmd_dev_thread_init();
    if (rc)
    {
        printk("\npmd_dev_thread_init failed\n");
        return rc;
    }

    if (calibration_parameters_from_json && calibration_parameters_from_json->temp_table.valid)
    {
        pmd_res_temp_conv = calibration_parameters_from_json->temp_table.val;
    }
    else
    {
        pmd_res_temp_conv = NULL;
    }
    if (calibration_parameters_from_json && calibration_parameters_from_json->temp2apd_table.valid)
    {
        pmd_temp_apd_conv = calibration_parameters_from_json->temp2apd_table.val;
    }
    else
    {
        pmd_temp_apd_conv = NULL;
    }

    rc = pmd_dev_init_seq(pmd_res_temp_conv, pmd_temp_apd_conv);
    if (rc)
    {
        printk("\npmd_dev_init_seq failed\n");
        return rc;
    }


    pmd_dev_timer_init();

    return PMD_OPERATION_SUCCESS;
}

static void pmdmsg_reset_session_statistics(void)
{
    pmd_messaging_session_pid = 0;
    pmd_messaging_sessions_counter = 1;

    previous_session_read_whole_messages_byte_counter = 0;

    read_pmd_whole_messages_counter = 0;
    previous_session_read_whole_messages_counter = 0;

    overwritten_pmd_messages_counter = 0;
    previous_session_overwritten_messages_counter = 0;

    overwritten_pmd_bytes_counter = 0;
    previous_session_overwritten_bytes_counter = 0;

    pmd_read_fragments_counter = 0;
    previous_session_read_fragments_counter = 0;

    pmd_read_fragmented_bytes_counter = 0;
    previous_session_read_fragmented_bytes_counter = 0;

    pmd_read_fragmented_messages_counter = 0;    
    previous_session_read_fragmented_messages_counter = 0;

    written_pmd_messages_counter = 0;
    previous_session_written_messages_counter = 0;

    written_pmd_bytes_counter = 0;
    previous_session_written_bytes_counter = 0;
}

static int _pmd_dev_init(void)
{
    int rc;

    mutex_init(&pmd_dev_lock);
    exported_symbols_readers = 0;
    exported_symbols_writers = 0;
    mb();
    spin_lock_init(&exported_symbols_spinlock);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
    spin_lock_init(&pon_message_bh_spinlock);
#endif

    pmdmsg_reset_session_statistics();

    /* allow chrdev only after init seq finished so that no locks are needed in init*/
    rc = register_chrdev(PMD_DEV_MAJOR, PMD_DEV_CLASS, &pmd_file_ops);
    if (rc)
    {
        printk(KERN_ERR "\nPMD module failed to load.\n");
        goto exit;
    }

    pmd_dev_create_proc();

    printk(KERN_INFO "\nPMD module loaded.\n");

    return PMD_OPERATION_SUCCESS;

exit:
    pmd_dev_exit();
    return rc;
}

static int client_num_to_i2c_addr(pmd_dev_client client_num)
{
    switch (client_num)
    {
        case pmd_reg_map:
            return 0x50;
        case pmd_iram_map:
            return 0x51;
        case pmd_dram_map:
            return 0x52;
        default:
            BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Cannot map client_num %d to i2c addr\n", client_num);
            return -1;
    }
}

static ssize_t i2c_read(struct i2c_adapter *i2c, int addr, char *buf, size_t count, int pmd_mode)
{
    struct i2c_msg msg[2];

    if (addr < 0)
        return -ENODEV;

    /* First write the offset  */
    msg[0].addr = msg[1].addr = addr;
    msg[0].flags = 0;

    msg[0].len = pmd_mode ? PMD_I2C_HEADER : 1;
    msg[0].buf = buf;

    /* Now read the data */
    msg[1].flags |= I2C_M_RD;
    msg[1].len = count;
    msg[1].buf = buf;

    if (i2c_transfer(i2c, msg, ARRAY_SIZE(msg)) == ARRAY_SIZE(msg))
        return count;

    return -1;
}

ssize_t pmd_i2c_read(u8 client_num, char *buf, size_t count)
{
    int addr = client_num_to_i2c_addr(client_num);

    if (addr < 0)
        return -1;

    return i2c_read(g_pdata->i2c, addr, buf, count, 1); 
}

static int i2c_write_reg(struct i2c_adapter *i2c, int addr, char *buf, size_t count)
{
    int ret;
    struct i2c_msg msg =
    {
        .addr = addr,
        .flags = 0,
        .len = count,
        .buf = buf,
    };

    ret = i2c_transfer(i2c, &msg, 1);

    /*
     * If everything went ok (i.e. 1 msg transferred), return #bytes
     * transferred, else error code.
     */
    return (ret == 1) ? count : ret;
}

int pmd_i2c_write(u8 client_num, char *buf, size_t count)
{
    int addr = client_num_to_i2c_addr(client_num);

    if (addr < 0)
        return -1;

    return i2c_write_reg(g_pdata->i2c, addr, buf, count); 
}

static int detect_pmd(struct i2c_adapter *i2c)
{
#define PMD_ID_LEN 4
    char buf[PMD_ID_LEN], pmd_device[][PMD_ID_LEN] = {
        { 0x8, 0x68, 0x90, 0x10 },
        { 0x8, 0x68, 0x91, 0x00 },
        {},
    };
    int i = 0;

    buf[0] = 0x0; /* Offset to read from */
    i2c_read(i2c, 0x50, buf, sizeof(buf), 0);

    do
    {
        if (!memcmp(pmd_device[i], buf, sizeof(buf)))
            return 1;
    } while (*pmd_device[++i]);

    return 0;
}

static int i2c_configure(struct device *dev, struct i2c_adapter **i2c, struct device_node *np)
{
    struct device_node *i2c_np;

    i2c_np = of_parse_phandle(np, "i2c-bus", 0);
    if (!i2c_np)
    {
        dev_err(dev, "missing 'i2c-bus' property\n");
        return -ENODEV;
    }

    *i2c = of_find_i2c_adapter_by_node(i2c_np);
    of_node_put(i2c_np);
    if (!*i2c)
        return -EPROBE_DEFER;

    if (!i2c_check_functionality(*i2c, I2C_FUNC_I2C))
        return -EINVAL;

    return 0;
}

static int pmd_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct device_node *np = dev->of_node;
    struct pmd_data *pdata;
    struct gpio_desc *rstn;
    struct i2c_adapter *i2c;
    int ret;

    if (!np)
        return -ENODEV;

    if (i2c_configure(dev, &i2c, np))
    {
        dev_err(dev, "failed to configure i2c\n");
        return -EINVAL;
    }

    rstn = devm_gpiod_get(dev, "rstn", GPIOD_ASIS);
    if (IS_ERR(rstn))
    {
        dev_err(dev, "Missing rstn configuration %ld", PTR_ERR(rstn));
        return -EINVAL;
    }
    gpiod_direction_output(rstn, 1);
    gpiod_direction_output(rstn, 0);
    mdelay(50);

    if (!detect_pmd(i2c))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Fail to detect pmd.\n");
        return -ENODEV;
    }

    if (bcm_enable_xtal_clk())
    {
        BCM_LOG_ERROR(BCM_LOG_ID_PMD, "Fail to enable xtal clock.\n");
        return -EINVAL;
    }

    trxbus_module_present(i2c->nr, 1);

    pdata = devm_kzalloc(&pdev->dev, sizeof(*pdata), GFP_KERNEL);
    if (!pdata)
        return -ENOMEM;

    pdata->rstn = rstn;
    pdata->i2c = i2c;
    pdata->pinctrl = devm_pinctrl_get(&pdev->dev);
    if (IS_ERR(pdata->pinctrl))
    {
        ret = PTR_ERR(pdata->pinctrl);
        return -EINVAL;
    }

    pdata->pins_ewake = pinctrl_lookup_state(pdata->pinctrl, "state_ewake");

    ret = bcm_bca_extintr_request(dev, np, "alarm", pmd_dev_threaded_isr, 0, "PMD alarm", pmd_dev_alarm_handler);
    if (ret < 0)
    {
        dev_err(dev, "Error with alarm irq configuration: %d\n", ret);
        return ret;
    }

    pdata->alarm_irq = ret;

    pdata->cap = 0;
    if (of_property_read_bool(np, "apd-flyback"))
        pdata->cap |= PMD_APD_TYPE_FLYBACK;

    if (of_property_read_bool(np, "apd-boost"))
        pdata->cap |= PMD_APD_TYPE_BOOST;

    if (of_property_read_bool(np, "apd-reg-enabled"))
        pdata->cap |= PMD_APD_REG_ENABLED;

    pdata->dev = dev;
    g_pdata = pdata;
    if (ewake_get())
        return -EINVAL;
    platform_set_drvdata(pdev, pdata);
    ret = _pmd_dev_init();

    if (ret)
        dev_err(dev, "failed to register %d\n", ret);
    else
        dev_info(dev, "registered\n");
    return ret ? -EINVAL : 0;
}

const struct of_device_id of_platform_pmd_table[] = {
    { .compatible = "brcm,pmd", .data = (void *)0, },
    { /* end of list */ },
};

struct platform_driver of_platform_pmd_driver = {
    .driver = {
        .name = "pmd",
        .of_match_table = of_platform_pmd_table,
    },
    .probe = pmd_probe,
};

module_platform_driver(of_platform_pmd_driver);

MODULE_DESCRIPTION("Pmd Device driver");
MODULE_LICENSE("GPL");
