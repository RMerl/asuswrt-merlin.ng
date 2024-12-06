/*
<:copyright-BRCM:2021:DUAL/GPL:standard 

   Copyright (c) 2021 Broadcom 
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

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/stop_machine.h>
#include "clk_rst.h"
#include "clk-bcm63xx.h"

/*
 * B15 CFG
 */
typedef struct B15ArchRegion {
	u32 addr_ulimit;
	u32 addr_llimit;
	u32 permission;
	u32 access_right_ctrl;
} B15ArchRegion;

typedef struct B15Arch {
	B15ArchRegion region[8];
	u32 unused[95];
	u32 scratch;
} B15Arch;

typedef struct B15CpuBusRange {
#define ULIMIT_SHIFT 4
#define BUSNUM_MASK 0x0000000FU

#define BUSNUM_UBUS 1
#define BUSNUM_RBUS 2
#define BUSNUM_RSVD 3
#define BUSNUM_MCP0 4
#define BUSNUM_MCP1 5
#define BUSNUM_MCP2 6

	u32 ulimit;
	u32 llimit;
} B15CpuBusRange;

typedef struct B15CpuAccessRightViol {
	u32 addr;
	u32 upper_addr;
	u32 detail_addr;
} B15CpuAccessRightViol;

typedef struct B15CpuBPCMAVS {
	u32 bpcm_id;
	u32 bpcm_capability;
	u32 bpcm_ctrl;
	u32 bpcm_status;
	u32 avs_rosc_ctrl;
	u32 avs_rosc_threshold;
	u32 avs_rosc_cnt;
	u32 avs_pwd_ctrl;
} B15CpuBPCMAVS;

typedef struct B15CpuCtrl {
	B15CpuBusRange bus_range[11];	/* 0x0 */
	u32 secure_reset_hndshake;
	u32 secure_soft_reset;
	B15CpuAccessRightViol access_right_viol[2];	/* 0x60 */
	u32 rac_cfg0;
	u32 rac_cfg1;
	u32 rac_flush;		/* 0x80 */
	u32 cpu_power_cfg;
	u32 cpu0_pwr_zone_ctrl;
	u32 cpu1_pwr_zone_ctrl;
	u32 cpu2_pwr_zone_ctrl;	/* 0x90 */
	u32 cpu3_pwr_zone_ctrl;
	u32 l2biu_pwr_zone_ctrl;
	u32 cpu0_pwr_zone_cfg1;
	u32 cpu0_pwr_zone_cfg2;	/* 0xa0 */
	u32 cpu1_pwr_zone_cfg1;
	u32 cpu1_pwr_zone_cfg2;
	u32 cpu2_pwr_zone_cfg1;
	u32 cpu2_pwr_zone_cfg2;	/* 0xb0 */
	u32 cpu3_pwr_zone_cfg1;
	u32 cpu3_pwr_zone_cfg2;
	u32 l2biu_pwr_zone_cfg1;
	u32 l2biu_pwr_zone_cfg2;	/* 0xc0 */
	u32 cpu0_pwr_freq_scalar_ctrl;
	u32 cpu1_pwr_freq_scalar_ctrl;
	u32 cpu2_pwr_freq_scalar_ctrl;
	u32 cpu3_pwr_freq_scalar_ctrl;	/* 0xd0 */
	u32 l2biu_pwr_freq_scalar_ctrl;
	B15CpuBPCMAVS cpu_bpcm_avs[4];	/* 0xd8 */
	B15CpuBPCMAVS l2biu_bpcm_avs;	/* 0x158 */
	u32 reset_cfg;		/* 0x178 */
	u32 clock_cfg;
	u32 misc_cfg;		/* 0x180 */
	u32 credit;
	u32 therm_throttle_temp;
	u32 term_throttle_irq_cfg;
	u32 therm_irq_high;		/* 0x190 */
	u32 therm_irq_low;
	u32 therm_misc_threshold;
	u32 therm_irq_misc;
	u32 defeature;		/* 0x1a0 */
	u32 defeature_key;
	u32 debug_rom_addr;
	u32 debug_self_addr;
	u32 debug_tracectrl;		/* 0x1b0 */
	u32 axi_cfg;
	u32 revision;
	u32 ubus_cfg_window[8];	/* 0x1bc */
	u32 ubus_cfg;		/* 0x1dc */
	u32 unused[135];
	u32 scratch;			/* 0x3fc */
} B15CpuCtrl;

typedef struct B15Ctrl {
	u32 unused0[1024];
	B15Arch arch;			/* 0x1000 */
	u32 unused1[896];
	B15CpuCtrl cpu_ctrl;		/* 0x2000 */
} B15Ctrl;

struct core_set_param {
	int shift;
	unsigned int core_base_rate;
	B15Ctrl __iomem *b15_ctrl;
};

#define BUS_RANGE_3_DEFAULT_ULIMIT  0x3ffffU
#define BUS_RANGE_4_DEFAULT_ULIMIT 0x1bffffU

extern void arm_wfi_enable(unsigned int freqHz);
static volatile u32 core_set_freq_done, core_set_freq_core_1_rdy;

/*
 * CPU frequency can be changed via the B15 pll or clock-ratio
 *
 * Access to the pll is through bpcm so reads/writes are slow.
 * Access to the clock-ratio is through a fast soc register.
 *
 * To change the frequency from:
 *
 * 1:1 to 1:n
 * - stop all write traffic (i.e. stop all CPUs)
 * - set safe-clock-mode (clock configuration register)
 * - DSB
 * - set clock-divisor (clock configuration register)
 * - DSB
 * - start stopped CPUs
 *
 * 1:n to 1:1
 * - stop all write traffic (i.e. stop all CPUs)
 * - clear clock-divisor (clock configuration register)
 * - DSB
 * - clear safe-clock-mode (clock configuration register)
 * - DSB
 * - start stopped CPUs
 *
 * The configuration changes should be done close together and
 * as quickly as possible to limit the down time for other CPUS.
 * [this makes changing the clock-ratio preferrable to the pll]
 */

static int core_set_freq(void* p)
{
	struct core_set_param* param = (struct core_set_param*)p;
	volatile B15Ctrl __iomem *B15CTRL = param->b15_ctrl;
	unsigned ratio = B15CTRL->cpu_ctrl.clock_cfg;
	const unsigned safe_mode = 16;

	int shift = param->shift;

	// only one core running, no idlers;
	// enable/disable wfi for idlers
	arm_wfi_enable(param->core_base_rate >> shift);

	if (shift != 0) {
		//A barrier here to ensure there are no pending memory accesses
		//when entering safe mode.
		smp_wmb();
		//Switching ARM DDR access over to UBUS temporarily. We need to make sure there's no
		//MCP activity when we enter Safe mode.
		B15CTRL->cpu_ctrl.bus_range[3].ulimit = (BUS_RANGE_3_DEFAULT_ULIMIT<<ULIMIT_SHIFT)|BUSNUM_UBUS;
		B15CTRL->cpu_ctrl.bus_range[4].ulimit = (BUS_RANGE_4_DEFAULT_ULIMIT<<ULIMIT_SHIFT)|BUSNUM_UBUS;
		//Read back to make sure the setting has taken effect before moving on.
		(void)B15CTRL->cpu_ctrl.bus_range[3].ulimit;
		(void)B15CTRL->cpu_ctrl.bus_range[4].ulimit;
		dsb();
		// set safe_clk_mode if < 1000MHz (2x 500MHz MCP)
		ratio |= safe_mode;
		B15CTRL->cpu_ctrl.clock_cfg = ratio; // set safe-mode
		//UBUS fast-ack makes above write operation a posted write.
		//Counter fast-ack by reading back the register. We want to
		//be sure the clock_cfg change has taken effect before
		//moving on.
		B15CTRL->cpu_ctrl.clock_cfg;
		dsb();

		ratio = (ratio & ~7) | shift;
		B15CTRL->cpu_ctrl.clock_cfg = ratio; // new divisor
		//Counter fast-ack
		B15CTRL->cpu_ctrl.clock_cfg;
		dsb();
		//Switching ARM DDR access back to MCP
		B15CTRL->cpu_ctrl.bus_range[3].ulimit = (BUS_RANGE_3_DEFAULT_ULIMIT<<ULIMIT_SHIFT)|BUSNUM_MCP0;
		B15CTRL->cpu_ctrl.bus_range[4].ulimit = (BUS_RANGE_4_DEFAULT_ULIMIT<<ULIMIT_SHIFT)|BUSNUM_MCP0;
		//Read back to make sure the setting has taken effect before moving on.
		(void)B15CTRL->cpu_ctrl.bus_range[3].ulimit;
		(void)B15CTRL->cpu_ctrl.bus_range[4].ulimit;
		dsb();
	} else {
		shift = ratio & 7;
		while (shift--) {
			// frequency doubling one step at a time
			ratio = (ratio & ~7) | shift;
			B15CTRL->cpu_ctrl.clock_cfg = ratio;
			//Counter fast-ack
			B15CTRL->cpu_ctrl.clock_cfg;
			if (shift <= 1) {
				// 50us spike mitigation at 750 & 1500MHz
				set_spike_mitigation(50);
			}
		}

		//A barrier here to ensure there are no pending memory accesses
		//when exiting safe mode.
		smp_wmb();
		// clear safe_clk_mode if >= 1000MHz (2x 500MHz MCP)
		B15CTRL->cpu_ctrl.clock_cfg = ratio & ~safe_mode; // clear safe-mode
		//Counter fast-ack
		B15CTRL->cpu_ctrl.clock_cfg;
		dsb();
	}

	return 0;
}

static int core_set_freq_sync(void *p) {
	//Load variables used into cache. We don't want DDR accesses
	//in the code sequence below.
	(void)core_set_freq_core_1_rdy;
	(void)core_set_freq_done;

	if (smp_processor_id()==0) {
		//Core0 is doing the frequency change. Wait until core1
		//is ready for it. We have to make sure core1 is not
		//doing any memory accesses while core0 is changing
		//CPU frequency.
		//Deliberately using cached variables for inter-core
		//synchronization instead of atomic variables.
		//Atomic variable primitives would generate a memory
		//access because MegaBarriers are used.
		// check if the remote cpu is online
		if(cpumask_test_cpu(1, cpu_online_mask))
			while(!core_set_freq_core_1_rdy);

		core_set_freq(p);
		core_set_freq_done=1;
	}
	else {

		core_set_freq_core_1_rdy=1;
		//Wait until core0 is done changing frequency before moving on.
		while(!core_set_freq_done);

	}

	return 0;
}

int get_arm_core_ratio(struct bcm63xx_cpuclk *cpuclk, int *ratio, int *ratio_base, int *mdiv)
{
	volatile B15Ctrl __iomem *B15CTRL = cpuclk->reg;
	u32 shift = B15CTRL->cpu_ctrl.clock_cfg&0xf;

	/* no need to update mdiv. Fixed post divider */
	*ratio_base = 16;
	*ratio = 16>>shift;

	return 0;
}

long round_arm_core_rate(struct bcm63xx_cpuclk *cpuclk, unsigned long rate)
{
	long new_rate = 0;
	/* b15 always use fix pll mdiv_in(2 by default) output(base_rate) and then apply cpu 
	   core ratio between 1, 1/2, ..., 1/16 to further scale down cpu clock */
	unsigned long cpu_base_rate = cpuclk->pllclk/cpuclk->mdiv;
	int shift;

	// find power-of-2 divisor
	for (shift = 0; shift <= 4; shift++)
		if ((cpu_base_rate >> shift) <= rate)
			break;
	if (shift > 4) {
		pr_warn("Invalid cpu frequency %luMHz, limit to smallest possible clock rate\n", 
			rate / FREQ_MHZ(1));
		shift = 4;
	}
	new_rate = cpu_base_rate>>shift;

	return new_rate;
}

int set_arm_core_clock(struct bcm63xx_cpuclk *cpuclk, unsigned long parent_rate, unsigned long rate)
{
	int cpu_base_rate = cpuclk->pllclk/cpuclk->mdiv;
	int shift;
	struct core_set_param param;

	// find power-of-2 divisor
	for (shift = 0; shift <= 4; shift++)
		if ((cpu_base_rate >> shift) == rate)
			break;
	if (shift > 4) {
		pr_warn("Invalid cpu frequency %luMHz\n", rate / FREQ_MHZ(1));
		return -EINVAL;
	}

	cpuclk->ratio = 16>>shift;
	smp_mb();

	core_set_freq_done=0;
	core_set_freq_core_1_rdy=0;
	param.shift = shift;
	param.core_base_rate = cpu_base_rate;
	param.b15_ctrl = (B15Ctrl __iomem *)cpuclk->reg;
	stop_machine(core_set_freq_sync, &param, cpu_online_mask);

	return 0;
}

int init_arm_core_pll(struct bcm63xx_cpuclk *cpuclk)
{
	/* arm pll at 3.0GHz and cpu base freq at 1.5GHz */
	cpuclk->pllclk = FREQ_MHZ(3000);
	cpuclk->mdiv = 2;
    set_b15_mdiv(2);

	return 0;
}
