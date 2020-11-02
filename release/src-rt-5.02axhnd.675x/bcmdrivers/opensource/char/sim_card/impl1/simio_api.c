/*
 <:copyright-BRCM:2014:DUAL/GPL:standard
 
    Copyright (c) 2014 Broadcom 
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

#include <bcmtypes.h>
#include <linux/types.h>

#include "bcm_OS_Deps.h"

#include <linux/bcm_log.h>
#include "chal_types.h"
#include "chal_simio.h"

#include <simio_def_common.h>
#include "simio.h"
#include "simio_board.h"

#include "sim_export_defs.h"

#include "shared_utils.h"

static int vcc_polarity = 0;
module_param(vcc_polarity, int,  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(vcc_polarity, "0: active high, 1: active low");

int sim_active(SIMIO_ID_t sim_id, UInt8 *data, int *len) 
{
    NO_SIM_RET(sim_id);

    if ((data == NULL) || (*len == 0))
        return SIM_CARD_DATA_IS_INVALID;

    SIMIO_SetParam(sim_id, SIMIO_PARAM_VOLTAGE, SIM_3V);
    SIMIO_set_frequency(sim_id, SIMIO_CLK_3P12HZ); 
    SIMIO_SetParam(sim_id, SIMIO_PARAM_T0_WWT, 0xffff);

    return (simio_activesim(sim_id, data, len));
}

int sim_online(SIMIO_ID_t sim_id)
{
    int detection_status;
    SIMIO_read_detection_status(sim_id, &detection_status); 
    return detection_status;
}

int sim_set_baud(SIMIO_ID_t sim_id, int F, int D)
{
    CHAL_SIMIO_SPEED_t val;
    cUInt16 divisor;

    NO_SIM_RET(sim_id);

    if (F == 372 && D == 1)
    {
        val = CHAL_SIMIO_SPD_372_1;
        divisor = 80;
    }
    else if (F == 512 && D == 8) 
    {
        val = CHAL_SIMIO_SPD_512_8;
        divisor = 16;
    }
    else if (F == 512 && D == 16) 
    {
        val = CHAL_SIMIO_SPD_512_16;
        divisor = 8;
    }
    else if (F == 512 && D == 32)
    {
        val = CHAL_SIMIO_SPD_512_32;
        divisor = 4;
    }
    else if (F == 512 && D == 64)
    {
        val = CHAL_SIMIO_SPD_512_64;
        divisor = 2;
    }
    else
    {
        printk (KERN_EMERG "smart card err: wrong value %s:%d\n",__FILE__, __LINE__ );
        return SIM_CARD_INPUT_IS_INVALID;
    }

    return (SIMIO_set_pps(sim_id, val, divisor));
}

int sim_card_command(SIMIO_ID_t sim_id, UInt16 command, UInt8 *data, int len)
{
    if ((data == NULL) || (len == 0))
        return SIM_CARD_DATA_IS_INVALID;

    return SIMIO_command(sim_id, command, data, len);
} 

int sim_write(SIMIO_ID_t sim_id, UInt8 *data, int len)
{
    NO_SIM_RET(sim_id);

    return (sim_card_command(sim_id, 1, data, len));
}

int sim_read(SIMIO_ID_t sim_id, UInt8 *data, int len)
{
    NO_SIM_RET(sim_id);

    return (sim_card_command(sim_id, 0, data, len));
}

int sim_card_protocol(SIMIO_ID_t sim_id, PROTOCOL_t protocol)
{
    int ret;

    NO_SIM_RET(sim_id);

    if ((protocol == SIM_PROTOCOL_T0) || (protocol == SIM_PROTOCOL_T1))
    {
        SIMIO_SetPrefProtocol(sim_id, protocol); 
        ret = 0;
    }
    else
        ret = SIM_CARD_INPUT_IS_INVALID;

    return ret;
}

int sim_card_control(SIMIO_ID_t sim_id, int control)
{
    NO_SIM_RET(sim_id);

    if (control == 0)
        SIMIO_DeactiveCard(sim_id);
    else if (control == 1)
        SIMIO_activeCard(sim_id);
    else
        return SIM_CARD_INPUT_IS_INVALID;

    return 0;
}

int sim_card_reset(SIMIO_ID_t sim_id, int reset_mode, SimVoltageLevel_t activation_voltage, SIMIO_DIVISOR_t frequency)
{
    int ret;

    NO_SIM_RET(sim_id);

    if (((reset_mode != 0) && (reset_mode != 1)) ||
        ((activation_voltage != SIM_5V) && (activation_voltage != SIM_3V)) ||
        (frequency < 0) || (frequency >= SIMIO_CLK_LAST))
        return SIM_CARD_INPUT_IS_INVALID;

    ret = SIMIO_set_frequency(sim_id, frequency); 

    SIMIO_SetParam(sim_id, SIMIO_PARAM_VOLTAGE, activation_voltage);

    if (reset_mode == 0)
        SIMIO_ResetCard(sim_id);
    else
        SIMIO_WarmResetCard(sim_id);
    
    return ret;    
}

static int __init sim_card_init(void)
{
    int ret = 0;
    sim_card_info sim_card_prm;
    sim_card_prm.id = SIMIO_ID_0; 

    printk(KERN_NOTICE "loading sim card module with vcc_polarity = %d\n",vcc_polarity);
#ifdef SIM_CARD_PROC_SUPPORT
    simio_user_register();
#else
    ret = simio_user_kernel_register();
#endif

    if(!ret)
        SIMIO_Init(SIMIO_ID_0, vcc_polarity);

    return ret;
}

static void __exit sim_card_exit(void)
{
    printk(KERN_NOTICE "leaving sim card module\n");
#ifndef SIM_CARD_PROC_SUPPORT
    simio_user_kernel_unregister();
#endif
}

module_init(sim_card_init);
module_exit(sim_card_exit);
MODULE_LICENSE("GPL");

