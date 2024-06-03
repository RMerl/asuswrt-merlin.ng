// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/post.h>
#include <asm/arch/device.h>
#include <asm/arch/tnc.h>
#include <asm/fsp/fsp_support.h>
#include <asm/processor.h>

static int __maybe_unused disable_igd(void)
{
	struct udevice *igd, *sdvo;
	int ret;

	ret = dm_pci_bus_find_bdf(TNC_IGD, &igd);
	if (ret)
		return ret;
	if (!igd)
		return 0;

	ret = dm_pci_bus_find_bdf(TNC_SDVO, &sdvo);
	if (ret)
		return ret;
	if (!sdvo)
		return 0;

	/*
	 * According to Atom E6xx datasheet, setting VGA Disable (bit17)
	 * of Graphics Controller register (offset 0x50) prevents IGD
	 * (D2:F0) from reporting itself as a VGA display controller
	 * class in the PCI configuration space, and should also prevent
	 * it from responding to VGA legacy memory range and I/O addresses.
	 *
	 * However test result shows that with just VGA Disable bit set and
	 * a PCIe graphics card connected to one of the PCIe controllers on
	 * the E6xx, accessing the VGA legacy space still causes system hang.
	 * After a number of attempts, it turns out besides VGA Disable bit,
	 * the SDVO (D3:F0) device should be disabled to make it work.
	 *
	 * To simplify, use the Function Disable register (offset 0xc4)
	 * to disable both IGD (D2:F0) and SDVO (D3:F0) devices. Now these
	 * two devices will be completely disabled (invisible in the PCI
	 * configuration space) unless a system reset is performed.
	 */
	dm_pci_write_config32(igd, IGD_FD, FUNC_DISABLE);
	dm_pci_write_config32(sdvo, IGD_FD, FUNC_DISABLE);

	/*
	 * After setting the function disable bit, IGD and SDVO devices will
	 * disappear in the PCI configuration space. This however creates an
	 * inconsistent state from a driver model PCI controller point of view,
	 * as these two PCI devices are still attached to its parent's child
	 * device list as maintained by the driver model. Some driver model PCI
	 * APIs like dm_pci_find_class(), are referring to the list to speed up
	 * the finding process instead of re-enumerating the whole PCI bus, so
	 * it gets the stale cached data which is wrong.
	 *
	 * Note x86 PCI enueration normally happens twice, in pre-relocation
	 * phase and post-relocation. One option might be to call disable_igd()
	 * in one of the pre-relocation initialization hooks so that it gets
	 * disabled in the first round, and when it comes to the second round
	 * driver model PCI will construct a correct list. Unfortunately this
	 * does not work as Intel FSP is used on this platform to perform low
	 * level initialization, and fsp_init_phase_pci() is called only once
	 * in the post-relocation phase. If we disable IGD and SDVO devices,
	 * fsp_init_phase_pci() simply hangs and never returns.
	 *
	 * So the only option we have is to manually remove these two devices.
	 */
	ret = device_remove(igd, DM_REMOVE_NORMAL);
	if (ret)
		return ret;
	ret = device_unbind(igd);
	if (ret)
		return ret;
	ret = device_remove(sdvo, DM_REMOVE_NORMAL);
	if (ret)
		return ret;
	ret = device_unbind(sdvo);
	if (ret)
		return ret;

	return 0;
}

int arch_cpu_init(void)
{
	post_code(POST_CPU_INIT);

	return x86_cpu_init_f();
}

static void tnc_irq_init(void)
{
	struct tnc_rcba *rcba;
	u32 base;

	pci_read_config32(TNC_LPC, LPC_RCBA, &base);
	base &= ~MEM_BAR_EN;
	rcba = (struct tnc_rcba *)base;

	/* Make sure all internal PCI devices are using INTA */
	writel(INTA, &rcba->d02ip);
	writel(INTA, &rcba->d03ip);
	writel(INTA, &rcba->d27ip);
	writel(INTA, &rcba->d31ip);
	writel(INTA, &rcba->d23ip);
	writel(INTA, &rcba->d24ip);
	writel(INTA, &rcba->d25ip);
	writel(INTA, &rcba->d26ip);

	/*
	 * Route TunnelCreek PCI device interrupt pin to PIRQ
	 *
	 * Since PCIe downstream ports received INTx are routed to PIRQ
	 * A/B/C/D directly and not configurable, we have to route PCIe
	 * root ports' INTx to PIRQ A/B/C/D as well. For other devices
	 * on TunneCreek, route them to PIRQ E/F/G/H.
	 */
	writew(PIRQE, &rcba->d02ir);
	writew(PIRQF, &rcba->d03ir);
	writew(PIRQG, &rcba->d27ir);
	writew(PIRQH, &rcba->d31ir);
	writew(PIRQA, &rcba->d23ir);
	writew(PIRQB, &rcba->d24ir);
	writew(PIRQC, &rcba->d25ir);
	writew(PIRQD, &rcba->d26ir);
}

int arch_early_init_r(void)
{
	int ret = 0;

#ifdef CONFIG_DISABLE_IGD
	ret = disable_igd();
#endif

	tnc_irq_init();

	return ret;
}
