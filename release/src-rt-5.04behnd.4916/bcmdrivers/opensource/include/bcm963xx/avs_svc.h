/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom
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
/****************************************************************************
 * Power RPC Driver
 *
 * Author: Samyon Furman <samyon.furman@broadcom.com> 
*****************************************************************************/

#ifndef _AVS_RPC_SVC_H_
#define _AVS_RPC_SVC_H_

#include <linux/types.h>
#include <linux/platform_device.h>
#include <itc_rpc.h>

#define RPC_SERVICE_VER_AVS_GET_DATA            0
#define RPC_SERVICE_VER_AVS_HOST_CMD            0

enum avs_svc_func_idx
{
    AVS_SVC_HOST_CMD,
    AVS_SVC_SET_INIT_PARAMS,
    AVS_SVC_GET_INIT_PARAMS,
    AVS_SVC_GET_DATA,
    AVS_SVC_FUNC_MAX
};

enum pmc_svc_data_idx
{
    kDIE_TEMP,
    kCORE_VIN,
    kCPU_VIN,
    kCPU_FREQ,
    kCPU_FREQ_MHz,
    k_VIN_1p8,
    k_VIN_3p3,
    k_VIN_1p2,
};

typedef struct {
	uint32_t command;       /* 0:RW for passing commands to firmware */
	uint32_t status;        /* 1:RW for receiving status back after issuing command */

	/* This is the data firmware supplies: */
	uint32_t voltage0;      /* 2:RO current voltage for CORE (in milli-volts) */
	int32_t temperature0;   /* 3:RO PVTMON temperature (in 1000th deg-C) */
	uint32_t PV0;           /* 4:RO initial predicted voltage value for CORE (in milli-volts) */
	uint32_t MV0;           /* 5:RO initial measured voltage value for CORE (in milli-volts) */

	uint32_t command_p0;    /* 6:RW used to pass parameters for commands */
	uint32_t command_p1;    /* 7:RW used to pass parameters for commands */
	uint32_t command_p2;    /* 8:RW used to pass parameters for commands */
	uint32_t command_p3;    /* 9:RW used to pass parameters for commands */
	uint32_t command_p4;    /*10:RW used to pass parameters for commands */
	uint32_t command_p5;    /*11:RW used to pass parameters for commands */
	uint32_t command_p6;    /*12:RW used to pass parameters for commands */
	uint32_t command_p7;    /*13:RW used to pass parameters for commands */
	uint32_t command_p8;    /*14:RW used to pass parameters for commands */
	uint32_t command_p9;    /*15:RW used to pass parameters for commands */
	uint32_t voltage1;      /*16:RO current voltage for CPU (in 1000th deg-C) */
	uint32_t PC2;           /*17:RO PV0, but includes C2 */
	uint32_t PV1;           /*18:RO initial predicted voltage value for CPU (in milli-volts) */
	uint32_t MV1;           /*19:RO initial measured voltage value for CPU (in milli-volts) */
	uint32_t freq;          /*20:RO current CPU frequency */
	uint32_t current0;      /*21:RO CORE current, if available (in milli-amperes) */
	uint32_t current1;      /*22:RO CPU current, if available (in milli-amperes) */

	uint32_t pmic_die_temp; /*23:RO PMIC die temperature (in 1000th deg-C) */
	uint32_t pmic_ext_temp; /*24:RO PMIC external thermistor temperature (in 1000th deg-C) */
	uint32_t pmic_power;    /*25:RO PMIC power consumption (in mW) */
	uint32_t pmic_psm;      /*26:RO PMIC PSM value */
	uint32_t reg_0;         /*27:RO Regulator #0 current/voltage current(mA)=[31:16], voltage(mV)=[15:0] */
	uint32_t reg_1;         /*28:RO Regulator #1  (as above) */
	uint32_t reg_2;         /*29:RO Regulator #2  (as above) */
	uint32_t reg_3;         /*30:RO Regulator #3  (as above) */
	uint32_t reg_4;         /*31:RO Regulator #4  (as above) */
	uint32_t reg_5;         /*32:RO Regulator #5  (as above) */
	uint32_t reg_6;         /*33:RO Regulator #6  (as above) */
	uint32_t reg_7;         /*34:RO Regulator #7  (as above) */
	uint32_t reg_8;         /*35:RO Regulator #8  (as above) */
	uint32_t reg_9;         /*36:RO Regulator #9  (as above) */
	uint32_t reg_10;        /*37:RO Regulator #10 (as above) */
	uint32_t vsys_main;     /*38:RO VSys_Main (5V input) */

	uint32_t command_p10;   /*39:RW */
	uint32_t command_p11;   /*40:RW */
	uint32_t command_p12;   /*41:RW */
	uint32_t command_p13;   /*42:RW */
	uint32_t command_p14;   /*43:RW */
	uint32_t command_p15;   /*44:RW */
	uint32_t command_p16;   /*45:RW */
	uint32_t command_p17;   /*46:RW */
	uint32_t command_p18;   /*47:RW */
	uint32_t command_p19;   /*48:RW */
	uint32_t command_p20;   /*49:RW */
	uint32_t command_p21;   /*50:RW */
	uint32_t command_p22;   /*51:RW */
	uint32_t command_p23;   /*52:RW */
	uint32_t command_p24;   /*53:RW */
} avs_host_cmd_t;

#define CMD_GET_P_MAP           0x30
#define CMD_SET_P_MAP           0x31
#define CMD_GET_P_STATE         0x40 
#define CMD_SET_P_STATE         0x41
#define CMD_READ_SENSOR         0x50
#define CMD_CALC_FREQ           0x52

#define CMD_GET_P_PMAP_AVSMODE_SHIFT 0
#define CMD_GET_P_PMAP_AVSMODE_MASK 0xff
#define CMD_GET_P_PMAP_PMAPID_SHIFT 8
#define CMD_GET_P_PMAP_PMAPID_MASK 0xff /* 5-bits actually as 0~31 are valid */

#define AVS_MODE  0x0
#define DFS_MODE  0x1
#define DVS_MODE  0x2
#define DVFS_MODE 0x3

/* Command parameter values for 'CMD_GET_P_STATE' and 'CMD_SET_P_STATE' commands */
/* These are the possible 'command_p3' values for CMD_SET_P_STATE command */
#define P_STATE_0  0
#define P_STATE_1  1
#define P_STATE_2  2
#define P_STATE_3  3
#define P_STATE_4  4

/* 'command_p0' values: unused:31-5, mode:4-0 */
#define MODE_SHIFT 0
#define MODE_MASK  0xf
/* 'command_p3' values: unused:31-5, state:4-0 */
#define STATE_SHIFT 0
#define STATE_MASK  0xf


   /* Responses: */
#define RSP_SUCCESS	0xF0	/* Command/notification accepted */
#define RSP_FAILURE	0xFF	/* Command/notification rejected */
#define RSP_INVALID	0xF1	/* Invalid command/notification (unknown) */
#define RSP_NO_SUPPORT	0xF2	/* Non-AVS modes are not supported */
#define RSP_NO_MAP	0xF3	/* Cannot set P-State until P-Map supplied */
/* Cannot change P-Map after initial P-Map set */
#define RSP_MAP_SET	0xF4
                                    
int bcm68xx_get_pmap(uint32_t *mode, uint32_t *pmap, uint32_t *cur_pstate, uint32_t *total_pstates, uint32_t *status);
int bcm68xx_set_pmap(uint32_t mode, uint32_t init_pstate, uint32_t *status);
int bcm68xx_get_pstate(uint32_t *cur_pstate, uint32_t *total_pstates, uint32_t *status);
int bcm68xx_set_pstate(uint32_t new_pstate, uint32_t *status);
int bcm68xx_get_cpu_freq(uint32_t for_pstate, uint32_t *cpu_freq, uint32_t *status);
int bcm68xx_get_sensor(uint32_t sensor, uint32_t *val, uint32_t *status);

#endif
