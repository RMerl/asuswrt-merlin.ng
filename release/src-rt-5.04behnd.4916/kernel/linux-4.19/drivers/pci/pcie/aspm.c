// SPDX-License-Identifier: GPL-2.0
/*
 * Enable PCIe link L0s/L1 state and Clock Power Management
 *
 * Copyright (C) 2007 Intel
 * Copyright (C) Zhang Yanmin (yanmin.zhang@intel.com)
 * Copyright (C) Shaohua Li (shaohua.li@intel.com)
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/pci.h>
#include <linux/pci_regs.h>
#include <linux/errno.h>
#include <linux/pm.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/pci-aspm.h>
#include "../pci.h"

#ifdef MODULE_PARAM_PREFIX
#undef MODULE_PARAM_PREFIX
#endif
#define MODULE_PARAM_PREFIX "pcie_aspm."

/* Note: those are not register definitions */
#define ASPM_STATE_L0S_UP	(1)	/* Upstream direction L0s state */
#define ASPM_STATE_L0S_DW	(2)	/* Downstream direction L0s state */
#define ASPM_STATE_L1		(4)	/* L1 state */
#define ASPM_STATE_L1_1		(8)	/* ASPM L1.1 state */
#define ASPM_STATE_L1_2		(0x10)	/* ASPM L1.2 state */
#define ASPM_STATE_L1_1_PCIPM	(0x20)	/* PCI PM L1.1 state */
#define ASPM_STATE_L1_2_PCIPM	(0x40)	/* PCI PM L1.2 state */
#define ASPM_STATE_L1_SS_PCIPM	(ASPM_STATE_L1_1_PCIPM | ASPM_STATE_L1_2_PCIPM)
#define ASPM_STATE_L1_2_MASK	(ASPM_STATE_L1_2 | ASPM_STATE_L1_2_PCIPM)
#define ASPM_STATE_L1SS		(ASPM_STATE_L1_1 | ASPM_STATE_L1_1_PCIPM |\
				 ASPM_STATE_L1_2_MASK)
#define ASPM_STATE_L0S		(ASPM_STATE_L0S_UP | ASPM_STATE_L0S_DW)
#define ASPM_STATE_ALL		(ASPM_STATE_L0S | ASPM_STATE_L1 |	\
				 ASPM_STATE_L1SS)

struct aspm_latency {
	u32 l0s;			/* L0s latency (nsec) */
	u32 l1;				/* L1 latency (nsec) */
};

struct pcie_link_state {
	struct pci_dev *pdev;		/* Upstream component of the Link */
	struct pci_dev *downstream;	/* Downstream component, function 0 */
	struct pcie_link_state *root;	/* pointer to the root port link */
	struct pcie_link_state *parent;	/* pointer to the parent Link state */
	struct list_head sibling;	/* node in link_list */
	struct list_head children;	/* list of child link states */
	struct list_head link;		/* node in parent's children list */

	/* ASPM state */
	u32 aspm_support:7;		/* Supported ASPM state */
	u32 aspm_enabled:7;		/* Enabled ASPM state */
	u32 aspm_capable:7;		/* Capable ASPM state with latency */
	u32 aspm_default:7;		/* Default ASPM state by BIOS */
	u32 aspm_disable:7;		/* Disabled ASPM state */

	/* Clock PM state */
	u32 clkpm_capable:1;		/* Clock PM capable? */
	u32 clkpm_enabled:1;		/* Current Clock PM state */
	u32 clkpm_default:1;		/* Default Clock PM state by BIOS */
	u32 clkpm_disable:1;		/* Clock PM disabled */

	/* Exit latencies */
	struct aspm_latency latency_up;	/* Upstream direction exit latency */
	struct aspm_latency latency_dw;	/* Downstream direction exit latency */
	/*
	 * Endpoint acceptable latencies. A pcie downstream port only
	 * has one slot under it, so at most there are 8 functions.
	 */
	struct aspm_latency acceptable[8];

	/* L1 PM Substate info */
	struct {
		u32 up_cap_ptr;		/* L1SS cap ptr in upstream dev */
		u32 dw_cap_ptr;		/* L1SS cap ptr in downstream dev */
		u32 ctl1;		/* value to be programmed in ctl1 */
		u32 ctl2;		/* value to be programmed in ctl2 */
	} l1ss;
};

static int aspm_disabled, aspm_force;
static bool aspm_support_enabled = true;
static DEFINE_MUTEX(aspm_lock);
static LIST_HEAD(link_list);

#define POLICY_DEFAULT 0	/* BIOS default setting */
#define POLICY_PERFORMANCE 1	/* high performance */
#define POLICY_POWERSAVE 2	/* high power saving */
#define POLICY_POWER_SUPERSAVE 3 /* possibly even more power saving */
#if defined(CONFIG_BCM_KF_POWER_SAVE)
#define POLICY_L0SPOWERSAVE 4	/* Only do L0S */
#define POLICY_L1POWERSAVE 5	/* Typically same savings as L1+L0s */
#endif

#ifdef CONFIG_PCIEASPM_PERFORMANCE
static int aspm_policy = POLICY_PERFORMANCE;
#elif defined CONFIG_PCIEASPM_POWERSAVE
static int aspm_policy = POLICY_POWERSAVE;
#elif defined CONFIG_PCIEASPM_POWER_SUPERSAVE
static int aspm_policy = POLICY_POWER_SUPERSAVE;
#else
static int aspm_policy;
#endif

static const char *policy_str[] = {
	[POLICY_DEFAULT] = "default",
	[POLICY_PERFORMANCE] = "performance",
	[POLICY_POWERSAVE] = "powersave",
	[POLICY_POWER_SUPERSAVE] = "powersupersave"
#if defined(CONFIG_BCM_KF_POWER_SAVE)
        ,
	[POLICY_L0SPOWERSAVE] = "l0s_powersave",
	[POLICY_L1POWERSAVE] = "l1_powersave",
#endif
};

#define LINK_RETRAIN_TIMEOUT HZ

static int policy_to_aspm_state(struct pcie_link_state *link)
{
	switch (aspm_policy) {
	case POLICY_PERFORMANCE:
		/* Disable ASPM and Clock PM */
		return 0;
	case POLICY_POWERSAVE:
		/* Enable ASPM L0s/L1 */
		return (ASPM_STATE_L0S | ASPM_STATE_L1);
	case POLICY_POWER_SUPERSAVE:
		/* Enable Everything */
		return ASPM_STATE_ALL;
#if defined(CONFIG_BCM_KF_POWER_SAVE)
	case POLICY_L0SPOWERSAVE:
		/* Enable ASPM L0s */
		return ASPM_STATE_L0S;
	case POLICY_L1POWERSAVE:
		/* Enable ASPM L1 */
		return ASPM_STATE_L1;
#endif
	case POLICY_DEFAULT:
		return link->aspm_default;
	}
	return 0;
}

static int policy_to_clkpm_state(struct pcie_link_state *link)
{
	switch (aspm_policy) {
	case POLICY_PERFORMANCE:
#if defined(CONFIG_BCM_KF_POWER_SAVE)
	case POLICY_L0SPOWERSAVE:
#endif
		/* Disable ASPM and Clock PM */
		return 0;
	case POLICY_POWERSAVE:
	case POLICY_POWER_SUPERSAVE:
#if defined(CONFIG_BCM_KF_POWER_SAVE)
	case POLICY_L1POWERSAVE:
#endif
		/* Enable Clock PM */
		return 1;
	case POLICY_DEFAULT:
		return link->clkpm_default;
	}
	return 0;
}

static void pcie_set_clkpm_nocheck(struct pcie_link_state *link, int enable)
{
	struct pci_dev *child;
	struct pci_bus *linkbus = link->pdev->subordinate;
	u32 val = enable ? PCI_EXP_LNKCTL_CLKREQ_EN : 0;

	list_for_each_entry(child, &linkbus->devices, bus_list)
		pcie_capability_clear_and_set_word(child, PCI_EXP_LNKCTL,
						   PCI_EXP_LNKCTL_CLKREQ_EN,
						   val);
	link->clkpm_enabled = !!enable;
}

static void pcie_set_clkpm(struct pcie_link_state *link, int enable)
{
	/*
	 * Don't enable Clock PM if the link is not Clock PM capable
	 * or Clock PM is disabled
	 */
	if (!link->clkpm_capable || link->clkpm_disable)
		enable = 0;
	/* Need nothing if the specified equals to current state */
	if (link->clkpm_enabled == enable)
		return;
	pcie_set_clkpm_nocheck(link, enable);
}

static void pcie_clkpm_cap_init(struct pcie_link_state *link, int blacklist)
{
	int capable = 1, enabled = 1;
	u32 reg32;
	u16 reg16;
	struct pci_dev *child;
	struct pci_bus *linkbus = link->pdev->subordinate;

	/* All functions should have the same cap and state, take the worst */
	list_for_each_entry(child, &linkbus->devices, bus_list) {
		pcie_capability_read_dword(child, PCI_EXP_LNKCAP, &reg32);
		if (!(reg32 & PCI_EXP_LNKCAP_CLKPM)) {
			capable = 0;
			enabled = 0;
			break;
		}
		pcie_capability_read_word(child, PCI_EXP_LNKCTL, &reg16);
		if (!(reg16 & PCI_EXP_LNKCTL_CLKREQ_EN))
			enabled = 0;
	}
	link->clkpm_enabled = enabled;
	link->clkpm_default = enabled;
#if defined(CONFIG_BCM_KF_POWER_SAVE)
	// Do not enable clkpm for the moment
	link->clkpm_capable = 0;
#else
	link->clkpm_capable = capable;
	link->clkpm_disable = blacklist ? 1 : 0;
#endif
}

static int pcie_wait_for_retrain(struct pci_dev *pdev)
{
	unsigned long end_jiffies;
	u16 reg16;

	/* Wait for Link Training to be cleared by hardware */
	end_jiffies = jiffies + LINK_RETRAIN_TIMEOUT;
	do {
		pcie_capability_read_word(pdev, PCI_EXP_LNKSTA, &reg16);
		if (!(reg16 & PCI_EXP_LNKSTA_LT))
			return 0;
		msleep(1);
	} while (time_before(jiffies, end_jiffies));

	return -ETIMEDOUT;
}

static int pcie_retrain_link(struct pcie_link_state *link)
{
	struct pci_dev *parent = link->pdev;
	int rc;
	u16 reg16;

	/*
	 * Ensure the updated LNKCTL parameters are used during link
	 * training by checking that there is no ongoing link training to
	 * avoid LTSSM race as recommended in Implementation Note at the
	 * end of PCIe r6.0.1 sec 7.5.3.7.
	 */
	rc = pcie_wait_for_retrain(parent);
	if (rc)
		return rc;

	pcie_capability_read_word(parent, PCI_EXP_LNKCTL, &reg16);
	reg16 |= PCI_EXP_LNKCTL_RL;
	pcie_capability_write_word(parent, PCI_EXP_LNKCTL, reg16);
	if (parent->clear_retrain_link) {
		/*
		 * Due to an erratum in some devices the Retrain Link bit
		 * needs to be cleared again manually to allow the link
		 * training to succeed.
		 */
		reg16 &= ~PCI_EXP_LNKCTL_RL;
		pcie_capability_write_word(parent, PCI_EXP_LNKCTL, reg16);
	}

	return pcie_wait_for_retrain(parent);
}

/*
 * pcie_aspm_configure_common_clock: check if the 2 ends of a link
 *   could use common clock. If they are, configure them to use the
 *   common clock. That will reduce the ASPM state exit latency.
 */
static void pcie_aspm_configure_common_clock(struct pcie_link_state *link)
{
	int same_clock = 1;
	u16 reg16, parent_reg, child_reg[8];
	struct pci_dev *child, *parent = link->pdev;
	struct pci_bus *linkbus = parent->subordinate;
	/*
	 * All functions of a slot should have the same Slot Clock
	 * Configuration, so just check one function
	 */
	child = list_entry(linkbus->devices.next, struct pci_dev, bus_list);
	BUG_ON(!pci_is_pcie(child));

	/* Check downstream component if bit Slot Clock Configuration is 1 */
	pcie_capability_read_word(child, PCI_EXP_LNKSTA, &reg16);
	if (!(reg16 & PCI_EXP_LNKSTA_SLC))
		same_clock = 0;

	/* Check upstream component if bit Slot Clock Configuration is 1 */
	pcie_capability_read_word(parent, PCI_EXP_LNKSTA, &reg16);
	if (!(reg16 & PCI_EXP_LNKSTA_SLC))
		same_clock = 0;

	/* Port might be already in common clock mode */
	pcie_capability_read_word(parent, PCI_EXP_LNKCTL, &reg16);
	if (same_clock && (reg16 & PCI_EXP_LNKCTL_CCC)) {
		bool consistent = true;

		list_for_each_entry(child, &linkbus->devices, bus_list) {
			pcie_capability_read_word(child, PCI_EXP_LNKCTL,
						  &reg16);
			if (!(reg16 & PCI_EXP_LNKCTL_CCC)) {
				consistent = false;
				break;
			}
		}
		if (consistent)
			return;
		pci_warn(parent, "ASPM: current common clock configuration is broken, reconfiguring\n");
	}

	/* Configure downstream component, all functions */
	list_for_each_entry(child, &linkbus->devices, bus_list) {
		pcie_capability_read_word(child, PCI_EXP_LNKCTL, &reg16);
		child_reg[PCI_FUNC(child->devfn)] = reg16;
		if (same_clock)
			reg16 |= PCI_EXP_LNKCTL_CCC;
		else
			reg16 &= ~PCI_EXP_LNKCTL_CCC;
		pcie_capability_write_word(child, PCI_EXP_LNKCTL, reg16);
	}

	/* Configure upstream component */
	pcie_capability_read_word(parent, PCI_EXP_LNKCTL, &reg16);
	parent_reg = reg16;
	if (same_clock)
		reg16 |= PCI_EXP_LNKCTL_CCC;
	else
		reg16 &= ~PCI_EXP_LNKCTL_CCC;
	pcie_capability_write_word(parent, PCI_EXP_LNKCTL, reg16);

	if (pcie_retrain_link(link)) {

		/* Training failed. Restore common clock configurations */
		pci_err(parent, "ASPM: Could not configure common clock\n");
		list_for_each_entry(child, &linkbus->devices, bus_list)
			pcie_capability_write_word(child, PCI_EXP_LNKCTL,
					   child_reg[PCI_FUNC(child->devfn)]);
		pcie_capability_write_word(parent, PCI_EXP_LNKCTL, parent_reg);
	}
}

/* Convert L0s latency encoding to ns */
static u32 calc_l0s_latency(u32 encoding)
{
	if (encoding == 0x7)
		return (5 * 1000);	/* > 4us */
	return (64 << encoding);
}

/* Convert L0s acceptable latency encoding to ns */
static u32 calc_l0s_acceptable(u32 encoding)
{
	if (encoding == 0x7)
		return -1U;
	return (64 << encoding);
}

/* Convert L1 latency encoding to ns */
static u32 calc_l1_latency(u32 encoding)
{
	if (encoding == 0x7)
		return (65 * 1000);	/* > 64us */
	return (1000 << encoding);
}

/* Convert L1 acceptable latency encoding to ns */
static u32 calc_l1_acceptable(u32 encoding)
{
	if (encoding == 0x7)
		return -1U;
	return (1000 << encoding);
}

/* Convert L1SS T_pwr encoding to usec */
static u32 calc_l1ss_pwron(struct pci_dev *pdev, u32 scale, u32 val)
{
	switch (scale) {
	case 0:
		return val * 2;
	case 1:
		return val * 10;
	case 2:
		return val * 100;
	}
	pci_err(pdev, "%s: Invalid T_PwrOn scale: %u\n", __func__, scale);
	return 0;
}

static void encode_l12_threshold(u32 threshold_us, u32 *scale, u32 *value)
{
	u32 threshold_ns = threshold_us * 1000;

	/* See PCIe r3.1, sec 7.33.3 and sec 6.18 */
	if (threshold_ns < 32) {
		*scale = 0;
		*value = threshold_ns;
	} else if (threshold_ns < 1024) {
		*scale = 1;
		*value = threshold_ns >> 5;
	} else if (threshold_ns < 32768) {
		*scale = 2;
		*value = threshold_ns >> 10;
	} else if (threshold_ns < 1048576) {
		*scale = 3;
		*value = threshold_ns >> 15;
	} else if (threshold_ns < 33554432) {
		*scale = 4;
		*value = threshold_ns >> 20;
	} else {
		*scale = 5;
		*value = threshold_ns >> 25;
	}
}

struct aspm_register_info {
	u32 support:2;
	u32 enabled:2;
	u32 latency_encoding_l0s;
	u32 latency_encoding_l1;

	/* L1 substates */
	u32 l1ss_cap_ptr;
	u32 l1ss_cap;
	u32 l1ss_ctl1;
	u32 l1ss_ctl2;
};

static void pcie_get_aspm_reg(struct pci_dev *pdev,
			      struct aspm_register_info *info)
{
	u16 reg16;
	u32 reg32;

	pcie_capability_read_dword(pdev, PCI_EXP_LNKCAP, &reg32);
	info->support = (reg32 & PCI_EXP_LNKCAP_ASPMS) >> 10;
	info->latency_encoding_l0s = (reg32 & PCI_EXP_LNKCAP_L0SEL) >> 12;
	info->latency_encoding_l1  = (reg32 & PCI_EXP_LNKCAP_L1EL) >> 15;
	pcie_capability_read_word(pdev, PCI_EXP_LNKCTL, &reg16);
	info->enabled = reg16 & PCI_EXP_LNKCTL_ASPMC;

	/* Read L1 PM substate capabilities */
	info->l1ss_cap = info->l1ss_ctl1 = info->l1ss_ctl2 = 0;
	info->l1ss_cap_ptr = pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_L1SS);
	if (!info->l1ss_cap_ptr)
		return;
	pci_read_config_dword(pdev, info->l1ss_cap_ptr + PCI_L1SS_CAP,
			      &info->l1ss_cap);
	if (!(info->l1ss_cap & PCI_L1SS_CAP_L1_PM_SS)) {
		info->l1ss_cap = 0;
		return;
	}

	/*
	 * If we don't have LTR for the entire path from the Root Complex
	 * to this device, we can't use ASPM L1.2 because it relies on the
	 * LTR_L1.2_THRESHOLD.  See PCIe r4.0, secs 5.5.4, 6.18.
	 */
	if (!pdev->ltr_path)
		info->l1ss_cap &= ~PCI_L1SS_CAP_ASPM_L1_2;

	pci_read_config_dword(pdev, info->l1ss_cap_ptr + PCI_L1SS_CTL1,
			      &info->l1ss_ctl1);
	pci_read_config_dword(pdev, info->l1ss_cap_ptr + PCI_L1SS_CTL2,
			      &info->l1ss_ctl2);
}

static void pcie_aspm_check_latency(struct pci_dev *endpoint)
{
	u32 latency, l1_switch_latency = 0;
	struct aspm_latency *acceptable;
	struct pcie_link_state *link;

	/* Device not in D0 doesn't need latency check */
	if ((endpoint->current_state != PCI_D0) &&
	    (endpoint->current_state != PCI_UNKNOWN))
		return;

	link = endpoint->bus->self->link_state;
	acceptable = &link->acceptable[PCI_FUNC(endpoint->devfn)];

	while (link) {
		/* Check upstream direction L0s latency */
		if ((link->aspm_capable & ASPM_STATE_L0S_UP) &&
		    (link->latency_up.l0s > acceptable->l0s))
			link->aspm_capable &= ~ASPM_STATE_L0S_UP;

		/* Check downstream direction L0s latency */
		if ((link->aspm_capable & ASPM_STATE_L0S_DW) &&
		    (link->latency_dw.l0s > acceptable->l0s))
			link->aspm_capable &= ~ASPM_STATE_L0S_DW;
		/*
		 * Check L1 latency.
		 * Every switch on the path to root complex need 1
		 * more microsecond for L1. Spec doesn't mention L0s.
		 *
		 * The exit latencies for L1 substates are not advertised
		 * by a device.  Since the spec also doesn't mention a way
		 * to determine max latencies introduced by enabling L1
		 * substates on the components, it is not clear how to do
		 * a L1 substate exit latency check.  We assume that the
		 * L1 exit latencies advertised by a device include L1
		 * substate latencies (and hence do not do any check).
		 */
		latency = max_t(u32, link->latency_up.l1, link->latency_dw.l1);
		if ((link->aspm_capable & ASPM_STATE_L1) &&
		    (latency + l1_switch_latency > acceptable->l1))
			link->aspm_capable &= ~ASPM_STATE_L1;
		l1_switch_latency += 1000;

		link = link->parent;
	}
}

/*
 * The L1 PM substate capability is only implemented in function 0 in a
 * multi function device.
 */
static struct pci_dev *pci_function_0(struct pci_bus *linkbus)
{
	struct pci_dev *child;

	list_for_each_entry(child, &linkbus->devices, bus_list)
		if (PCI_FUNC(child->devfn) == 0)
			return child;
	return NULL;
}

/* Calculate L1.2 PM substate timing parameters */
static void aspm_calc_l1ss_info(struct pcie_link_state *link,
				struct aspm_register_info *upreg,
				struct aspm_register_info *dwreg)
{
	u32 val1, val2, scale1, scale2;
	u32 t_common_mode, t_power_on, l1_2_threshold, scale, value;

	link->l1ss.up_cap_ptr = upreg->l1ss_cap_ptr;
	link->l1ss.dw_cap_ptr = dwreg->l1ss_cap_ptr;
	link->l1ss.ctl1 = link->l1ss.ctl2 = 0;

	if (!(link->aspm_support & ASPM_STATE_L1_2_MASK))
		return;

	/* Choose the greater of the two Port Common_Mode_Restore_Times */
	val1 = (upreg->l1ss_cap & PCI_L1SS_CAP_CM_RESTORE_TIME) >> 8;
	val2 = (dwreg->l1ss_cap & PCI_L1SS_CAP_CM_RESTORE_TIME) >> 8;
	t_common_mode = max(val1, val2);

	/* Choose the greater of the two Port T_POWER_ON times */
	val1   = (upreg->l1ss_cap & PCI_L1SS_CAP_P_PWR_ON_VALUE) >> 19;
	scale1 = (upreg->l1ss_cap & PCI_L1SS_CAP_P_PWR_ON_SCALE) >> 16;
	val2   = (dwreg->l1ss_cap & PCI_L1SS_CAP_P_PWR_ON_VALUE) >> 19;
	scale2 = (dwreg->l1ss_cap & PCI_L1SS_CAP_P_PWR_ON_SCALE) >> 16;

	if (calc_l1ss_pwron(link->pdev, scale1, val1) >
	    calc_l1ss_pwron(link->downstream, scale2, val2)) {
		link->l1ss.ctl2 |= scale1 | (val1 << 3);
		t_power_on = calc_l1ss_pwron(link->pdev, scale1, val1);
	} else {
		link->l1ss.ctl2 |= scale2 | (val2 << 3);
		t_power_on = calc_l1ss_pwron(link->downstream, scale2, val2);
	}

	/*
	 * Set LTR_L1.2_THRESHOLD to the time required to transition the
	 * Link from L0 to L1.2 and back to L0 so we enter L1.2 only if
	 * downstream devices report (via LTR) that they can tolerate at
	 * least that much latency.
	 *
	 * Based on PCIe r3.1, sec 5.5.3.3.1, Figures 5-16 and 5-17, and
	 * Table 5-11.  T(POWER_OFF) is at most 2us and T(L1.2) is at
	 * least 4us.
	 */
	l1_2_threshold = 2 + 4 + t_common_mode + t_power_on;
	encode_l12_threshold(l1_2_threshold, &scale, &value);
	link->l1ss.ctl1 |= t_common_mode << 8 | scale << 29 | value << 16;
}

static void pcie_aspm_cap_init(struct pcie_link_state *link, int blacklist)
{
	struct pci_dev *child = link->downstream, *parent = link->pdev;
	struct pci_bus *linkbus = parent->subordinate;
	struct aspm_register_info upreg, dwreg;

	if (blacklist) {
		/* Set enabled/disable so that we will disable ASPM later */
		link->aspm_enabled = ASPM_STATE_ALL;
		link->aspm_disable = ASPM_STATE_ALL;
		return;
	}

	/* Get upstream/downstream components' register state */
	pcie_get_aspm_reg(parent, &upreg);
	pcie_get_aspm_reg(child, &dwreg);

	/*
	 * If ASPM not supported, don't mess with the clocks and link,
	 * bail out now.
	 */
	if (!(upreg.support & dwreg.support))
		return;

	/* Configure common clock before checking latencies */
	pcie_aspm_configure_common_clock(link);

	/*
	 * Re-read upstream/downstream components' register state
	 * after clock configuration
	 */
	pcie_get_aspm_reg(parent, &upreg);
	pcie_get_aspm_reg(child, &dwreg);

	/*
	 * Setup L0s state
	 *
	 * Note that we must not enable L0s in either direction on a
	 * given link unless components on both sides of the link each
	 * support L0s.
	 */
	if (dwreg.support & upreg.support & PCIE_LINK_STATE_L0S)
		link->aspm_support |= ASPM_STATE_L0S;
	if (dwreg.enabled & PCIE_LINK_STATE_L0S)
		link->aspm_enabled |= ASPM_STATE_L0S_UP;
	if (upreg.enabled & PCIE_LINK_STATE_L0S)
		link->aspm_enabled |= ASPM_STATE_L0S_DW;
	link->latency_up.l0s = calc_l0s_latency(upreg.latency_encoding_l0s);
	link->latency_dw.l0s = calc_l0s_latency(dwreg.latency_encoding_l0s);

	/* Setup L1 state */
	if (upreg.support & dwreg.support & PCIE_LINK_STATE_L1)
		link->aspm_support |= ASPM_STATE_L1;
	if (upreg.enabled & dwreg.enabled & PCIE_LINK_STATE_L1)
		link->aspm_enabled |= ASPM_STATE_L1;
	link->latency_up.l1 = calc_l1_latency(upreg.latency_encoding_l1);
	link->latency_dw.l1 = calc_l1_latency(dwreg.latency_encoding_l1);

	/* Setup L1 substate */
	if (upreg.l1ss_cap & dwreg.l1ss_cap & PCI_L1SS_CAP_ASPM_L1_1)
		link->aspm_support |= ASPM_STATE_L1_1;
	if (upreg.l1ss_cap & dwreg.l1ss_cap & PCI_L1SS_CAP_ASPM_L1_2)
		link->aspm_support |= ASPM_STATE_L1_2;
	if (upreg.l1ss_cap & dwreg.l1ss_cap & PCI_L1SS_CAP_PCIPM_L1_1)
		link->aspm_support |= ASPM_STATE_L1_1_PCIPM;
	if (upreg.l1ss_cap & dwreg.l1ss_cap & PCI_L1SS_CAP_PCIPM_L1_2)
		link->aspm_support |= ASPM_STATE_L1_2_PCIPM;

	if (upreg.l1ss_ctl1 & dwreg.l1ss_ctl1 & PCI_L1SS_CTL1_ASPM_L1_1)
		link->aspm_enabled |= ASPM_STATE_L1_1;
	if (upreg.l1ss_ctl1 & dwreg.l1ss_ctl1 & PCI_L1SS_CTL1_ASPM_L1_2)
		link->aspm_enabled |= ASPM_STATE_L1_2;
	if (upreg.l1ss_ctl1 & dwreg.l1ss_ctl1 & PCI_L1SS_CTL1_PCIPM_L1_1)
		link->aspm_enabled |= ASPM_STATE_L1_1_PCIPM;
	if (upreg.l1ss_ctl1 & dwreg.l1ss_ctl1 & PCI_L1SS_CTL1_PCIPM_L1_2)
		link->aspm_enabled |= ASPM_STATE_L1_2_PCIPM;

	if (link->aspm_support & ASPM_STATE_L1SS)
		aspm_calc_l1ss_info(link, &upreg, &dwreg);

	/* Save default state */
	link->aspm_default = link->aspm_enabled;

	/* Setup initial capable state. Will be updated later */
	link->aspm_capable = link->aspm_support;

	/* Get and check endpoint acceptable latencies */
	list_for_each_entry(child, &linkbus->devices, bus_list) {
		u32 reg32, encoding;
		struct aspm_latency *acceptable =
			&link->acceptable[PCI_FUNC(child->devfn)];

		if (pci_pcie_type(child) != PCI_EXP_TYPE_ENDPOINT &&
		    pci_pcie_type(child) != PCI_EXP_TYPE_LEG_END)
			continue;

		pcie_capability_read_dword(child, PCI_EXP_DEVCAP, &reg32);
		/* Calculate endpoint L0s acceptable latency */
		encoding = (reg32 & PCI_EXP_DEVCAP_L0S) >> 6;
		acceptable->l0s = calc_l0s_acceptable(encoding);
		/* Calculate endpoint L1 acceptable latency */
		encoding = (reg32 & PCI_EXP_DEVCAP_L1) >> 9;
		acceptable->l1 = calc_l1_acceptable(encoding);

		pcie_aspm_check_latency(child);
	}
}

static void pci_clear_and_set_dword(struct pci_dev *pdev, int pos,
				    u32 clear, u32 set)
{
	u32 val;

	pci_read_config_dword(pdev, pos, &val);
	val &= ~clear;
	val |= set;
	pci_write_config_dword(pdev, pos, val);
}

/* Configure the ASPM L1 substates */
static void pcie_config_aspm_l1ss(struct pcie_link_state *link, u32 state)
{
	u32 val, enable_req;
	struct pci_dev *child = link->downstream, *parent = link->pdev;
	u32 up_cap_ptr = link->l1ss.up_cap_ptr;
	u32 dw_cap_ptr = link->l1ss.dw_cap_ptr;

	enable_req = (link->aspm_enabled ^ state) & state;

	/*
	 * Here are the rules specified in the PCIe spec for enabling L1SS:
	 * - When enabling L1.x, enable bit at parent first, then at child
	 * - When disabling L1.x, disable bit at child first, then at parent
	 * - When enabling ASPM L1.x, need to disable L1
	 *   (at child followed by parent).
	 * - The ASPM/PCIPM L1.2 must be disabled while programming timing
	 *   parameters
	 *
	 * To keep it simple, disable all L1SS bits first, and later enable
	 * what is needed.
	 */

	/* Disable all L1 substates */
	pci_clear_and_set_dword(child, dw_cap_ptr + PCI_L1SS_CTL1,
				PCI_L1SS_CTL1_L1SS_MASK, 0);
	pci_clear_and_set_dword(parent, up_cap_ptr + PCI_L1SS_CTL1,
				PCI_L1SS_CTL1_L1SS_MASK, 0);
	/*
	 * If needed, disable L1, and it gets enabled later
	 * in pcie_config_aspm_link().
	 */
	if (enable_req & (ASPM_STATE_L1_1 | ASPM_STATE_L1_2)) {
		pcie_capability_clear_and_set_word(child, PCI_EXP_LNKCTL,
						   PCI_EXP_LNKCTL_ASPM_L1, 0);
		pcie_capability_clear_and_set_word(parent, PCI_EXP_LNKCTL,
						   PCI_EXP_LNKCTL_ASPM_L1, 0);
	}

	if (enable_req & ASPM_STATE_L1_2_MASK) {

		/* Program T_POWER_ON times in both ports */
		pci_write_config_dword(parent, up_cap_ptr + PCI_L1SS_CTL2,
				       link->l1ss.ctl2);
		pci_write_config_dword(child, dw_cap_ptr + PCI_L1SS_CTL2,
				       link->l1ss.ctl2);

		/* Program Common_Mode_Restore_Time in upstream device */
		pci_clear_and_set_dword(parent, up_cap_ptr + PCI_L1SS_CTL1,
					PCI_L1SS_CTL1_CM_RESTORE_TIME,
					link->l1ss.ctl1);

		/* Program LTR_L1.2_THRESHOLD time in both ports */
		pci_clear_and_set_dword(parent,	up_cap_ptr + PCI_L1SS_CTL1,
					PCI_L1SS_CTL1_LTR_L12_TH_VALUE |
					PCI_L1SS_CTL1_LTR_L12_TH_SCALE,
					link->l1ss.ctl1);
		pci_clear_and_set_dword(child, dw_cap_ptr + PCI_L1SS_CTL1,
					PCI_L1SS_CTL1_LTR_L12_TH_VALUE |
					PCI_L1SS_CTL1_LTR_L12_TH_SCALE,
					link->l1ss.ctl1);
	}

	val = 0;
	if (state & ASPM_STATE_L1_1)
		val |= PCI_L1SS_CTL1_ASPM_L1_1;
	if (state & ASPM_STATE_L1_2)
		val |= PCI_L1SS_CTL1_ASPM_L1_2;
	if (state & ASPM_STATE_L1_1_PCIPM)
		val |= PCI_L1SS_CTL1_PCIPM_L1_1;
	if (state & ASPM_STATE_L1_2_PCIPM)
		val |= PCI_L1SS_CTL1_PCIPM_L1_2;

	/* Enable what we need to enable */
	pci_clear_and_set_dword(parent, up_cap_ptr + PCI_L1SS_CTL1,
				PCI_L1SS_CTL1_L1SS_MASK, val);
	pci_clear_and_set_dword(child, dw_cap_ptr + PCI_L1SS_CTL1,
				PCI_L1SS_CTL1_L1SS_MASK, val);
}

static void pcie_config_aspm_dev(struct pci_dev *pdev, u32 val)
{
	pcie_capability_clear_and_set_word(pdev, PCI_EXP_LNKCTL,
					   PCI_EXP_LNKCTL_ASPMC, val);
}

static void pcie_config_aspm_link(struct pcie_link_state *link, u32 state)
{
	u32 upstream = 0, dwstream = 0;
	struct pci_dev *child = link->downstream, *parent = link->pdev;
	struct pci_bus *linkbus = parent->subordinate;

	/* Enable only the states that were not explicitly disabled */
	state &= (link->aspm_capable & ~link->aspm_disable);

	/* Can't enable any substates if L1 is not enabled */
	if (!(state & ASPM_STATE_L1))
		state &= ~ASPM_STATE_L1SS;

	/* Spec says both ports must be in D0 before enabling PCI PM substates*/
	if (parent->current_state != PCI_D0 || child->current_state != PCI_D0) {
		state &= ~ASPM_STATE_L1_SS_PCIPM;
		state |= (link->aspm_enabled & ASPM_STATE_L1_SS_PCIPM);
	}

	/* Nothing to do if the link is already in the requested state */
	if (link->aspm_enabled == state)
		return;
	/* Convert ASPM state to upstream/downstream ASPM register state */
	if (state & ASPM_STATE_L0S_UP)
		dwstream |= PCI_EXP_LNKCTL_ASPM_L0S;
	if (state & ASPM_STATE_L0S_DW)
		upstream |= PCI_EXP_LNKCTL_ASPM_L0S;
	if (state & ASPM_STATE_L1) {
		upstream |= PCI_EXP_LNKCTL_ASPM_L1;
		dwstream |= PCI_EXP_LNKCTL_ASPM_L1;
	}

	if (link->aspm_capable & ASPM_STATE_L1SS)
		pcie_config_aspm_l1ss(link, state);

	/*
	 * Spec 2.0 suggests all functions should be configured the
	 * same setting for ASPM. Enabling ASPM L1 should be done in
	 * upstream component first and then downstream, and vice
	 * versa for disabling ASPM L1. Spec doesn't mention L0S.
	 */
	if (state & ASPM_STATE_L1)
		pcie_config_aspm_dev(parent, upstream);
	list_for_each_entry(child, &linkbus->devices, bus_list)
		pcie_config_aspm_dev(child, dwstream);
	if (!(state & ASPM_STATE_L1))
		pcie_config_aspm_dev(parent, upstream);

	link->aspm_enabled = state;
}

static void pcie_config_aspm_path(struct pcie_link_state *link)
{
	while (link) {
		pcie_config_aspm_link(link, policy_to_aspm_state(link));
		link = link->parent;
	}
}

static void free_link_state(struct pcie_link_state *link)
{
	link->pdev->link_state = NULL;
	kfree(link);
}

static int pcie_aspm_sanity_check(struct pci_dev *pdev)
{
	struct pci_dev *child;
	u32 reg32;

	/*
	 * Some functions in a slot might not all be PCIe functions,
	 * very strange. Disable ASPM for the whole slot
	 */
	list_for_each_entry(child, &pdev->subordinate->devices, bus_list) {
		if (!pci_is_pcie(child))
			return -EINVAL;

		/*
		 * If ASPM is disabled then we're not going to change
		 * the BIOS state. It's safe to continue even if it's a
		 * pre-1.1 device
		 */

		if (aspm_disabled)
			continue;

		/*
		 * Disable ASPM for pre-1.1 PCIe device, we follow MS to use
		 * RBER bit to determine if a function is 1.1 version device
		 */
		pcie_capability_read_dword(child, PCI_EXP_DEVCAP, &reg32);
		if (!(reg32 & PCI_EXP_DEVCAP_RBER) && !aspm_force) {
			pci_info(child, "disabling ASPM on pre-1.1 PCIe device.  You can enable it with 'pcie_aspm=force'\n");
			return -EINVAL;
		}
#if defined(CONFIG_BCM_KF_POWER_SAVE)
		// Only enable ASPM where thoroughly tested
		dev_printk(KERN_DEBUG, &pdev->dev, "Checking PCIe ASPM for vendor %x device %x\n", child->vendor, child->device);
		if ((child->vendor == 0x14e4) &&
			(child->device != 0x4365) &&  // 4365
			(child->device != 0x4366) &&  // 4366
			(child->device != 0xaa90) &&  // 4366E
			(child->device != 0xaa92) &&  // 4365E
			(child->device != 0x4360) &&  // 4360
			(child->device != 0x43a0) &&  // 4360
			(child->device != 0x43a1) &&  // 4360
			(child->device != 0x43a2) &&  // 4360
			(child->device != 0x43a9) &&  // 43217
			(child->device != 0xa8db))    // 43217-43227
		{
			dev_printk(KERN_DEBUG, &pdev->dev, "Disabling PCIe ASPM for vendor %x device %x\n", child->vendor, child->device);
			return -EINVAL;
		}
#endif
	}
	return 0;
}

static struct pcie_link_state *alloc_pcie_link_state(struct pci_dev *pdev)
{
	struct pcie_link_state *link;

	link = kzalloc(sizeof(*link), GFP_KERNEL);
	if (!link)
		return NULL;

	INIT_LIST_HEAD(&link->sibling);
	INIT_LIST_HEAD(&link->children);
	INIT_LIST_HEAD(&link->link);
	link->pdev = pdev;
	link->downstream = pci_function_0(pdev->subordinate);

	/*
	 * Root Ports and PCI/PCI-X to PCIe Bridges are roots of PCIe
	 * hierarchies.  Note that some PCIe host implementations omit
	 * the root ports entirely, in which case a downstream port on
	 * a switch may become the root of the link state chain for all
	 * its subordinate endpoints.
	 */
	if (pci_pcie_type(pdev) == PCI_EXP_TYPE_ROOT_PORT ||
	    pci_pcie_type(pdev) == PCI_EXP_TYPE_PCIE_BRIDGE ||
	    !pdev->bus->parent->self) {
		link->root = link;
	} else {
		struct pcie_link_state *parent;

		parent = pdev->bus->parent->self->link_state;
		if (!parent) {
			kfree(link);
			return NULL;
		}

		link->parent = parent;
		link->root = link->parent->root;
		list_add(&link->link, &parent->children);
	}

	list_add(&link->sibling, &link_list);
	pdev->link_state = link;
	return link;
}

/*
 * pcie_aspm_init_link_state: Initiate PCI express link state.
 * It is called after the pcie and its children devices are scanned.
 * @pdev: the root port or switch downstream port
 */
void pcie_aspm_init_link_state(struct pci_dev *pdev)
{
	struct pcie_link_state *link;
	int blacklist = !!pcie_aspm_sanity_check(pdev);

	if (!aspm_support_enabled)
		return;

	if (pdev->link_state)
		return;

	/*
	 * We allocate pcie_link_state for the component on the upstream
	 * end of a Link, so there's nothing to do unless this device has a
	 * Link on its secondary side.
	 */
	if (!pdev->has_secondary_link)
		return;

	/* VIA has a strange chipset, root port is under a bridge */
	if (pci_pcie_type(pdev) == PCI_EXP_TYPE_ROOT_PORT &&
	    pdev->bus->self)
		return;

	down_read(&pci_bus_sem);
	if (list_empty(&pdev->subordinate->devices))
		goto out;

	mutex_lock(&aspm_lock);
	link = alloc_pcie_link_state(pdev);
	if (!link)
		goto unlock;
	/*
	 * Setup initial ASPM state. Note that we need to configure
	 * upstream links also because capable state of them can be
	 * update through pcie_aspm_cap_init().
	 */
	pcie_aspm_cap_init(link, blacklist);

	/* Setup initial Clock PM state */
	pcie_clkpm_cap_init(link, blacklist);

	/*
	 * At this stage drivers haven't had an opportunity to change the
	 * link policy setting. Enabling ASPM on broken hardware can cripple
	 * it even before the driver has had a chance to disable ASPM, so
	 * default to a safe level right now. If we're enabling ASPM beyond
	 * the BIOS's expectation, we'll do so once pci_enable_device() is
	 * called.
	 */
	if (aspm_policy != POLICY_POWERSAVE &&
	    aspm_policy != POLICY_POWER_SUPERSAVE) {
		pcie_config_aspm_path(link);
		pcie_set_clkpm(link, policy_to_clkpm_state(link));
	}

unlock:
	mutex_unlock(&aspm_lock);
out:
	up_read(&pci_bus_sem);
}

/* Recheck latencies and update aspm_capable for links under the root */
static void pcie_update_aspm_capable(struct pcie_link_state *root)
{
	struct pcie_link_state *link;
	BUG_ON(root->parent);
	list_for_each_entry(link, &link_list, sibling) {
		if (link->root != root)
			continue;
		link->aspm_capable = link->aspm_support;
	}
	list_for_each_entry(link, &link_list, sibling) {
		struct pci_dev *child;
		struct pci_bus *linkbus = link->pdev->subordinate;
		if (link->root != root)
			continue;
		list_for_each_entry(child, &linkbus->devices, bus_list) {
			if ((pci_pcie_type(child) != PCI_EXP_TYPE_ENDPOINT) &&
			    (pci_pcie_type(child) != PCI_EXP_TYPE_LEG_END))
				continue;
			pcie_aspm_check_latency(child);
		}
	}
}

/* @pdev: the endpoint device */
void pcie_aspm_exit_link_state(struct pci_dev *pdev)
{
	struct pci_dev *parent = pdev->bus->self;
	struct pcie_link_state *link, *root, *parent_link;

	if (!parent || !parent->link_state)
		return;

	down_read(&pci_bus_sem);
	mutex_lock(&aspm_lock);
	/*
	 * All PCIe functions are in one slot, remove one function will remove
	 * the whole slot, so just wait until we are the last function left.
	 */
	if (!list_empty(&parent->subordinate->devices))
		goto out;

	link = parent->link_state;
	root = link->root;
	parent_link = link->parent;

	/* All functions are removed, so just disable ASPM for the link */
	pcie_config_aspm_link(link, 0);
	list_del(&link->sibling);
	list_del(&link->link);
	/* Clock PM is for endpoint device */
	free_link_state(link);

	/* Recheck latencies and configure upstream links */
	if (parent_link) {
		pcie_update_aspm_capable(root);
		pcie_config_aspm_path(parent_link);
	}
out:
	mutex_unlock(&aspm_lock);
	up_read(&pci_bus_sem);
}

/* @pdev: the root port or switch downstream port */
void pcie_aspm_pm_state_change(struct pci_dev *pdev)
{
	struct pcie_link_state *link = pdev->link_state;

	if (aspm_disabled || !link)
		return;
	/*
	 * Devices changed PM state, we should recheck if latency
	 * meets all functions' requirement
	 */
	down_read(&pci_bus_sem);
	mutex_lock(&aspm_lock);
	pcie_update_aspm_capable(link->root);
	pcie_config_aspm_path(link);
	mutex_unlock(&aspm_lock);
	up_read(&pci_bus_sem);
}

void pcie_aspm_powersave_config_link(struct pci_dev *pdev)
{
	struct pcie_link_state *link = pdev->link_state;

	if (aspm_disabled || !link)
		return;

	if (aspm_policy != POLICY_POWERSAVE &&
	    aspm_policy != POLICY_POWER_SUPERSAVE)
		return;

	down_read(&pci_bus_sem);
	mutex_lock(&aspm_lock);
	pcie_config_aspm_path(link);
	pcie_set_clkpm(link, policy_to_clkpm_state(link));
	mutex_unlock(&aspm_lock);
	up_read(&pci_bus_sem);
}

static void __pci_disable_link_state(struct pci_dev *pdev, int state, bool sem)
{
	struct pci_dev *parent = pdev->bus->self;
	struct pcie_link_state *link;

	if (!pci_is_pcie(pdev))
		return;

	if (pdev->has_secondary_link)
		parent = pdev;
	if (!parent || !parent->link_state)
		return;

	/*
	 * A driver requested that ASPM be disabled on this device, but
	 * if we don't have permission to manage ASPM (e.g., on ACPI
	 * systems we have to observe the FADT ACPI_FADT_NO_ASPM bit and
	 * the _OSC method), we can't honor that request.  Windows has
	 * a similar mechanism using "PciASPMOptOut", which is also
	 * ignored in this situation.
	 */
	if (aspm_disabled) {
		pci_warn(pdev, "can't disable ASPM; OS doesn't have ASPM control\n");
		return;
	}

	if (sem)
		down_read(&pci_bus_sem);
	mutex_lock(&aspm_lock);
	link = parent->link_state;
	if (state & PCIE_LINK_STATE_L0S)
		link->aspm_disable |= ASPM_STATE_L0S;
	if (state & PCIE_LINK_STATE_L1)
		link->aspm_disable |= ASPM_STATE_L1;
	pcie_config_aspm_link(link, policy_to_aspm_state(link));

	if (state & PCIE_LINK_STATE_CLKPM)
		link->clkpm_disable = 1;
	pcie_set_clkpm(link, policy_to_clkpm_state(link));
	mutex_unlock(&aspm_lock);
	if (sem)
		up_read(&pci_bus_sem);
}

void pci_disable_link_state_locked(struct pci_dev *pdev, int state)
{
	__pci_disable_link_state(pdev, state, false);
}
EXPORT_SYMBOL(pci_disable_link_state_locked);

/**
 * pci_disable_link_state - Disable device's link state, so the link will
 * never enter specific states.  Note that if the BIOS didn't grant ASPM
 * control to the OS, this does nothing because we can't touch the LNKCTL
 * register.
 *
 * @pdev: PCI device
 * @state: ASPM link state to disable
 */
void pci_disable_link_state(struct pci_dev *pdev, int state)
{
	__pci_disable_link_state(pdev, state, true);
}
EXPORT_SYMBOL(pci_disable_link_state);

static int pcie_aspm_set_policy(const char *val,
				const struct kernel_param *kp)
{
	int i;
	struct pcie_link_state *link;

	if (aspm_disabled)
		return -EPERM;
	i = sysfs_match_string(policy_str, val);
	if (i < 0)
		return i;
	if (i == aspm_policy)
		return 0;

	down_read(&pci_bus_sem);
	mutex_lock(&aspm_lock);
	aspm_policy = i;
	list_for_each_entry(link, &link_list, sibling) {
		pcie_config_aspm_link(link, policy_to_aspm_state(link));
		pcie_set_clkpm(link, policy_to_clkpm_state(link));
	}
	mutex_unlock(&aspm_lock);
	up_read(&pci_bus_sem);
	return 0;
}

static int pcie_aspm_get_policy(char *buffer, const struct kernel_param *kp)
{
	int i, cnt = 0;
	for (i = 0; i < ARRAY_SIZE(policy_str); i++)
		if (i == aspm_policy)
			cnt += sprintf(buffer + cnt, "[%s] ", policy_str[i]);
		else
			cnt += sprintf(buffer + cnt, "%s ", policy_str[i]);
	cnt += sprintf(buffer + cnt, "\n");
	return cnt;
}

module_param_call(policy, pcie_aspm_set_policy, pcie_aspm_get_policy,
	NULL, 0644);

#ifdef CONFIG_PCIEASPM_DEBUG
static ssize_t link_state_show(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	struct pci_dev *pci_device = to_pci_dev(dev);
	struct pcie_link_state *link_state = pci_device->link_state;

	return sprintf(buf, "%d\n", link_state->aspm_enabled);
}

static ssize_t link_state_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf,
		size_t n)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct pcie_link_state *link, *root = pdev->link_state->root;
	u32 state;

	if (aspm_disabled)
		return -EPERM;

	if (kstrtouint(buf, 10, &state))
		return -EINVAL;
	if ((state & ~ASPM_STATE_ALL) != 0)
		return -EINVAL;

	down_read(&pci_bus_sem);
	mutex_lock(&aspm_lock);
	list_for_each_entry(link, &link_list, sibling) {
		if (link->root != root)
			continue;
		pcie_config_aspm_link(link, state);
	}
	mutex_unlock(&aspm_lock);
	up_read(&pci_bus_sem);
	return n;
}

static ssize_t clk_ctl_show(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	struct pci_dev *pci_device = to_pci_dev(dev);
	struct pcie_link_state *link_state = pci_device->link_state;

	return sprintf(buf, "%d\n", link_state->clkpm_enabled);
}

static ssize_t clk_ctl_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf,
		size_t n)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	bool state;

	if (strtobool(buf, &state))
		return -EINVAL;

	down_read(&pci_bus_sem);
	mutex_lock(&aspm_lock);
	pcie_set_clkpm_nocheck(pdev->link_state, state);
	mutex_unlock(&aspm_lock);
	up_read(&pci_bus_sem);

	return n;
}

static DEVICE_ATTR_RW(link_state);
static DEVICE_ATTR_RW(clk_ctl);

static char power_group[] = "power";
void pcie_aspm_create_sysfs_dev_files(struct pci_dev *pdev)
{
	struct pcie_link_state *link_state = pdev->link_state;

	if (!link_state)
		return;

	if (link_state->aspm_support)
		sysfs_add_file_to_group(&pdev->dev.kobj,
			&dev_attr_link_state.attr, power_group);
	if (link_state->clkpm_capable)
		sysfs_add_file_to_group(&pdev->dev.kobj,
			&dev_attr_clk_ctl.attr, power_group);
}

void pcie_aspm_remove_sysfs_dev_files(struct pci_dev *pdev)
{
	struct pcie_link_state *link_state = pdev->link_state;

	if (!link_state)
		return;

	if (link_state->aspm_support)
		sysfs_remove_file_from_group(&pdev->dev.kobj,
			&dev_attr_link_state.attr, power_group);
	if (link_state->clkpm_capable)
		sysfs_remove_file_from_group(&pdev->dev.kobj,
			&dev_attr_clk_ctl.attr, power_group);
}
#endif

static int __init pcie_aspm_disable(char *str)
{
	if (!strcmp(str, "off")) {
		aspm_policy = POLICY_DEFAULT;
		aspm_disabled = 1;
		aspm_support_enabled = false;
		printk(KERN_INFO "PCIe ASPM is disabled\n");
	} else if (!strcmp(str, "force")) {
		aspm_force = 1;
		printk(KERN_INFO "PCIe ASPM is forcibly enabled\n");
	}
	return 1;
}

__setup("pcie_aspm=", pcie_aspm_disable);

void pcie_no_aspm(void)
{
	/*
	 * Disabling ASPM is intended to prevent the kernel from modifying
	 * existing hardware state, not to clear existing state. To that end:
	 * (a) set policy to POLICY_DEFAULT in order to avoid changing state
	 * (b) prevent userspace from changing policy
	 */
	if (!aspm_force) {
		aspm_policy = POLICY_DEFAULT;
		aspm_disabled = 1;
	}
}

bool pcie_aspm_support_enabled(void)
{
	return aspm_support_enabled;
}
EXPORT_SYMBOL(pcie_aspm_support_enabled);
