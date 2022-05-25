/*
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom 
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

/*****************************************************************************
 *  Description:
 *      This contains the PMC driver for Linux.
 *****************************************************************************/

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/reboot.h>
#include <bcm_intr.h>
#include "pmc_drv.h"
#include "pmc_dsl.h"
#include "board.h"

#include "command.h"
#include "BPCM.h"
#include "shared_utils.h"
#include "pmc_spu.h"

extern int pmc_mode;

static DEFINE_SPINLOCK(pmc_lock);

void pmc_spin_lock(void)
{
	/* PMC function can take very long time in a few ms. Driver should not
	 * call it in interrupt context(hardware interrupt, bottom half and
	 * softirq).  Add check function to print out warning and call stack
	 * when this happens.
	 */

	if (in_interrupt()) {
		printk
		    ("PMC driver function called in interrupt context !!!!\n");
		/*dump_stack(); */
	}
	spin_lock(&pmc_lock);	// TODO: to be replaced with mutex_lock()
}

void pmc_spin_unlock(void)
{
	spin_unlock(&pmc_lock);
}

int GetDevPresence(int devAddr, int *value)
{
	if (pmc_mode == PMC_MODE_DQM) {
		TCommand rsp;
		int status =
		    SendCommand(cmdGetDevPresence, devAddr, 0, 0, 0, 0, &rsp);

		if (status == kPMC_NO_ERROR)
			*value = (rsp.u.cmdResponse.word2 & 1) ? TRUE : FALSE;
		else
			*value = FALSE;

		return status;
	} else
		return kPMC_INVALID_COMMAND;
}

EXPORT_SYMBOL(GetDevPresence);

int GetSWStrap(int devAddr, int *value)
{
	if (pmc_mode == PMC_MODE_DQM) {
		TCommand rsp;
		int status =
		    SendCommand(cmdGetSWStrap, devAddr, 0, 0, 0, 0, &rsp);

		if (status == kPMC_NO_ERROR)
			*value = rsp.u.cmdResponse.word2 & 0xffff;

		return status;
	} else
		return kPMC_INVALID_COMMAND;
}

EXPORT_SYMBOL(GetSWStrap);

int GetHWRev(int devAddr, int *value)
{
	if (pmc_mode == PMC_MODE_DQM) {
		TCommand rsp;
		int status =
		    SendCommand(cmdGetHWRev, devAddr, 0, 0, 0, 0, &rsp);

		if (status == kPMC_NO_ERROR)
			*value = rsp.u.cmdResponse.word2 & 0xff;

		return status;
	} else
		return kPMC_INVALID_COMMAND;
}

EXPORT_SYMBOL(GetHWRev);

int GetNumZones(int devAddr, int *value)
{
	if (pmc_mode == PMC_MODE_DQM) {
		TCommand rsp;
		int status =
		    SendCommand(cmdGetNumZones, devAddr, 0, 0, 0, 0, &rsp);

		if (status == kPMC_NO_ERROR)
			*value = rsp.u.cmdResponse.word2 & 0xff;

		return status;
	} else
		return kPMC_INVALID_COMMAND;
}

EXPORT_SYMBOL(GetNumZones);

int GetAvsDisableState(int island, int *state)
{
	if (pmc_mode == PMC_MODE_DQM) {
		TCommand rsp;
		int status =
		    SendCommand(cmdGetAvsDisableState, 0, 0, island, 0, 0,
				&rsp);

		if (status == kPMC_NO_ERROR)
			*state = rsp.u.cmdResponse.word2;
		return status;
	} else
		return kPMC_INVALID_COMMAND;
}

EXPORT_SYMBOL(GetAvsDisableState);

static void _devices_power_down_init(void)
{
	// Power down certain blocks by default and let driver power them up
#if !defined(CONFIG_BRCM_IKOS)
#if defined(PMB_ADDR_CRYPTO)
	pmc_spu_power_down();
#endif
#if defined(NO_DSL) && (defined(CONFIG_BCM963158) || defined(CONFIG_BCM963178))
	pmc_dsl_power_down();
#elif defined(CONFIG_BCM963178)
	if (!kerSysGetDslPhyEnable())
		pmc_dsl_power_down();
#endif
#endif
}

int GetAllROs(uint32_t pa)
{
#if defined(PMC_GETALL_RO_SUPPORT)
	if (pmc_mode == PMC_MODE_DQM) {
		TCommand rsp;

		if (pa == 0x0)
			return kPMC_INVALID_PARAM;

		return SendCommand(cmdReadROs, 0, 0, 0, pa, 0, &rsp);
	} else
		return kPMC_INVALID_COMMAND;
#else
		return kPMC_INVALID_COMMAND;
#endif
}

EXPORT_SYMBOL(GetAllROs);

#if defined(CONFIG_BCM963158)
static irqreturn_t pmc_isr(int irq, void *dev_id)
{
	volatile Pmc *pmc = (volatile Pmc *)g_pmc->pmc_base;
	/* Clear the PMC mailbox interrupt */
	pmc->ctrl.l2IrqGpStatus |= 1 << 3;

	if (pmc->ctrl.hostMboxOut == -1)
	{
		orderly_reboot();
		pr_emerg("%s: !!! CPU Too HOT, performing orderly reboot ...\n",
			 __func__);
	} 
	else
	{
		pr_info("%s: pmc->ctrl.hostMboxOut=0x%08x\n",
			__func__, pmc->ctrl.hostMboxOut);
	}

	return IRQ_HANDLED;
}

void install_pmc_isr(void)
{
	if (request_irq(INTERRUPT_ID_PMC, pmc_isr, 0, "pmc_mbox", NULL))
		pr_err("ERROR: failed to configure INTERRUPT_ID_PMC\n");
}
#endif /* CONFIG_BCM963158 */

#if defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622) || \
	defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813) || \
        defined(CONFIG_BCM96756)
static irqreturn_t pmc_temp_warn_isr(int irq, void *dev_id)
{
	uint32_t val;
	int status = read_bpcm_reg_direct(PMB_ADDR_PVTMON, 0x54 >> 2, &val);

	if (status || !(val & (1 << 15)))	// PVTMONRO_ACQ_TEMP_WARN_RESET.WARN
		return IRQ_NONE;

	// temperature exceeded warn_threshold
	orderly_reboot();	// schedule reboot work
	val |= 1 << 14;		// clear_warn
	val &= ~(1 << 10);	// disable warning
	status = write_bpcm_reg_direct(PMB_ADDR_PVTMON, 0x54 >> 2, val);
	val = 0;
	while (!status && !(val & (1 << 18)))
		status = read_bpcm_reg_direct(PMB_ADDR_PVTMON, 0x60 >> 2, &val);
	if (val & (1 << 18))	// PVTMONRO_ACQ_TMON.VALID
		val &= 0x3ff;	// PVTMONRO_ACQ_TMON.VALUE

	pr_emerg("%s cpu %u: DieTemp %s %d, performing orderly reboot !!!\n",
		 __func__, smp_processor_id(), status ? "status" : "mC",
		 status ? : pmc_convert_pvtmon(kTEMPERATURE, val));
	return IRQ_HANDLED;
}

void install_pmc_temp_warn_isr(void)
{
	if (request_irq(INTERRUPT_ID_PMC_TEMP_WARN, pmc_temp_warn_isr, 0,
			"pmc_temp_warn", NULL))
		pr_err("ERROR: failed to configure irq pmc_temp_warn\n");
}
#endif /* #if defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622) || \
	  defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813) ||
          defined(CONFIG_BCM96756) */

int RecloseAVS(int iscold)
{
#if defined(PMC_RECLOSE_SUPPORT) 
	static int last_slow, last_fast;
	int slow, fast;		// margins

	void __iomem *res = ioremap(JTAG_OTP_PHYS_BASE, 0x4); 
	uint32_t cap = readl(res); 

	iounmap(res);

	// check sec_chipvar and send command
	if ((cap & 15) != 0) {
		slow = 80, fast = 55;
		if (iscold)
			slow += 10, fast += 10;
	} else
		slow = 100, fast = 100;
	if (slow == last_slow && fast == last_fast)
		return 0;
	return CloseAVS(0, last_slow = slow, last_fast = fast, 0, 0);
#else
    return kPMC_INVALID_COMMAND;
#endif
}

EXPORT_SYMBOL(RecloseAVS);

#if defined(CONFIG_BCM963138)
static void pmc_patch_63138(void)
{
#define logsize (8 << 10)
	static const __attribute__ ((aligned(logsize)))
#include "restart.h"
	unsigned linkaddr = 0x9fc10000;	// translated code start
	const void *physaddr = restart_bin;
	TCommand rsp;
	volatile Pmc *pmc = (volatile Pmc *)g_pmc->pmc_base;

	if (((pmc->ctrl.hostMboxIn >> 24) & 7) !=
	    kPMCRunStateAVSCompleteWaitingForImage)
		return;

	/* window 2 baseout specifies destination of pmcapp image */
	/* e4k will translate the block at BaseIn to BaseOut */
	/* pmc ram command at linkaddr */
	pmc->ctrl.addr2WndwMask = -(logsize);
	pmc->ctrl.addr2WndwBaseIn = linkaddr & 0x1fffffff;
	pmc->ctrl.addr2WndwBaseOut = virt_to_phys(physaddr);

	// register command
	SendCommand(cmdRegisterCmdHandler, 0, 0, 0, 96, linkaddr, &rsp);
	if (rsp.word0.Bits.error)
		printk("%s %d\n", "cmdRegisterCmdHandler",
		       rsp.word0.Bits.error);
	else {
		// send command with option to enable AVS
		SendCommand(96, 1, 0, 0, 75, 75, &rsp);
		printk("%s:%x %x %x\n", __func__,
		       rsp.word1.Reg32, rsp.u.cmdResponse.word2,
		       rsp.u.cmdResponse.word3);
	}
}
#endif

#if defined(PMC_ON_HOSTCPU)
void pmc_tracking_init(void);
#endif

int pmc_init(void)
{
	int rc = 0;

	pmc_initmode();

#if defined(CONFIG_BCM963158)
	install_pmc_isr();
#endif

#if defined(PMC_ON_HOSTCPU) && !defined(CONFIG_BRCM_IKOS)
	pmc_tracking_init();
#endif

	printk("%s:PMC using %s mode\n", __func__,
	       pmc_mode == PMC_MODE_PMB_DIRECT ? "PMB_DIRECT" : "DQM");
	if (pmc_mode == PMC_MODE_PMB_DIRECT)
		return 0;

#if defined CONFIG_BCM963138
	pmc_patch_63138();
#endif

#if defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622) || \
	defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813) || \
        defined(CONFIG_BCM96756)
	install_pmc_temp_warn_isr();
#endif

	_devices_power_down_init();
	return rc;
}
