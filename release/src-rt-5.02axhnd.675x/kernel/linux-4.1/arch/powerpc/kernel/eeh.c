/*
 * Copyright IBM Corporation 2001, 2005, 2006
 * Copyright Dave Engebretsen & Todd Inglett 2001
 * Copyright Linas Vepstas 2005, 2006
 * Copyright 2001-2012 IBM Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * Please address comments and feedback to Linas Vepstas <linas@austin.ibm.com>
 */

#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/pci.h>
#include <linux/iommu.h>
#include <linux/proc_fs.h>
#include <linux/rbtree.h>
#include <linux/reboot.h>
#include <linux/seq_file.h>
#include <linux/spinlock.h>
#include <linux/export.h>
#include <linux/of.h>

#include <linux/atomic.h>
#include <asm/debug.h>
#include <asm/eeh.h>
#include <asm/eeh_event.h>
#include <asm/io.h>
#include <asm/iommu.h>
#include <asm/machdep.h>
#include <asm/ppc-pci.h>
#include <asm/rtas.h>


/** Overview:
 *  EEH, or "Extended Error Handling" is a PCI bridge technology for
 *  dealing with PCI bus errors that can't be dealt with within the
 *  usual PCI framework, except by check-stopping the CPU.  Systems
 *  that are designed for high-availability/reliability cannot afford
 *  to crash due to a "mere" PCI error, thus the need for EEH.
 *  An EEH-capable bridge operates by converting a detected error
 *  into a "slot freeze", taking the PCI adapter off-line, making
 *  the slot behave, from the OS'es point of view, as if the slot
 *  were "empty": all reads return 0xff's and all writes are silently
 *  ignored.  EEH slot isolation events can be triggered by parity
 *  errors on the address or data busses (e.g. during posted writes),
 *  which in turn might be caused by low voltage on the bus, dust,
 *  vibration, humidity, radioactivity or plain-old failed hardware.
 *
 *  Note, however, that one of the leading causes of EEH slot
 *  freeze events are buggy device drivers, buggy device microcode,
 *  or buggy device hardware.  This is because any attempt by the
 *  device to bus-master data to a memory address that is not
 *  assigned to the device will trigger a slot freeze.   (The idea
 *  is to prevent devices-gone-wild from corrupting system memory).
 *  Buggy hardware/drivers will have a miserable time co-existing
 *  with EEH.
 *
 *  Ideally, a PCI device driver, when suspecting that an isolation
 *  event has occurred (e.g. by reading 0xff's), will then ask EEH
 *  whether this is the case, and then take appropriate steps to
 *  reset the PCI slot, the PCI device, and then resume operations.
 *  However, until that day,  the checking is done here, with the
 *  eeh_check_failure() routine embedded in the MMIO macros.  If
 *  the slot is found to be isolated, an "EEH Event" is synthesized
 *  and sent out for processing.
 */

/* If a device driver keeps reading an MMIO register in an interrupt
 * handler after a slot isolation event, it might be broken.
 * This sets the threshold for how many read attempts we allow
 * before printing an error message.
 */
#define EEH_MAX_FAILS	2100000

/* Time to wait for a PCI slot to report status, in milliseconds */
#define PCI_BUS_RESET_WAIT_MSEC (5*60*1000)

/*
 * EEH probe mode support, which is part of the flags,
 * is to support multiple platforms for EEH. Some platforms
 * like pSeries do PCI emunation based on device tree.
 * However, other platforms like powernv probe PCI devices
 * from hardware. The flag is used to distinguish that.
 * In addition, struct eeh_ops::probe would be invoked for
 * particular OF node or PCI device so that the corresponding
 * PE would be created there.
 */
int eeh_subsystem_flags;
EXPORT_SYMBOL(eeh_subsystem_flags);

/*
 * EEH allowed maximal frozen times. If one particular PE's
 * frozen count in last hour exceeds this limit, the PE will
 * be forced to be offline permanently.
 */
int eeh_max_freezes = 5;

/* Platform dependent EEH operations */
struct eeh_ops *eeh_ops = NULL;

/* Lock to avoid races due to multiple reports of an error */
DEFINE_RAW_SPINLOCK(confirm_error_lock);

/* Lock to protect passed flags */
static DEFINE_MUTEX(eeh_dev_mutex);

/* Buffer for reporting pci register dumps. Its here in BSS, and
 * not dynamically alloced, so that it ends up in RMO where RTAS
 * can access it.
 */
#define EEH_PCI_REGS_LOG_LEN 8192
static unsigned char pci_regs_buf[EEH_PCI_REGS_LOG_LEN];

/*
 * The struct is used to maintain the EEH global statistic
 * information. Besides, the EEH global statistics will be
 * exported to user space through procfs
 */
struct eeh_stats {
	u64 no_device;		/* PCI device not found		*/
	u64 no_dn;		/* OF node not found		*/
	u64 no_cfg_addr;	/* Config address not found	*/
	u64 ignored_check;	/* EEH check skipped		*/
	u64 total_mmio_ffs;	/* Total EEH checks		*/
	u64 false_positives;	/* Unnecessary EEH checks	*/
	u64 slot_resets;	/* PE reset			*/
};

static struct eeh_stats eeh_stats;

#define IS_BRIDGE(class_code) (((class_code)<<16) == PCI_BASE_CLASS_BRIDGE)

static int __init eeh_setup(char *str)
{
	if (!strcmp(str, "off"))
		eeh_add_flag(EEH_FORCE_DISABLED);
	else if (!strcmp(str, "early_log"))
		eeh_add_flag(EEH_EARLY_DUMP_LOG);

	return 1;
}
__setup("eeh=", eeh_setup);

/*
 * This routine captures assorted PCI configuration space data
 * for the indicated PCI device, and puts them into a buffer
 * for RTAS error logging.
 */
static size_t eeh_dump_dev_log(struct eeh_dev *edev, char *buf, size_t len)
{
	struct pci_dn *pdn = eeh_dev_to_pdn(edev);
	u32 cfg;
	int cap, i;
	int n = 0, l = 0;
	char buffer[128];

	n += scnprintf(buf+n, len-n, "%04x:%02x:%02x:%01x\n",
		       edev->phb->global_number, pdn->busno,
		       PCI_SLOT(pdn->devfn), PCI_FUNC(pdn->devfn));
	pr_warn("EEH: of node=%04x:%02x:%02x:%01x\n",
		edev->phb->global_number, pdn->busno,
		PCI_SLOT(pdn->devfn), PCI_FUNC(pdn->devfn));

	eeh_ops->read_config(pdn, PCI_VENDOR_ID, 4, &cfg);
	n += scnprintf(buf+n, len-n, "dev/vend:%08x\n", cfg);
	pr_warn("EEH: PCI device/vendor: %08x\n", cfg);

	eeh_ops->read_config(pdn, PCI_COMMAND, 4, &cfg);
	n += scnprintf(buf+n, len-n, "cmd/stat:%x\n", cfg);
	pr_warn("EEH: PCI cmd/status register: %08x\n", cfg);

	/* Gather bridge-specific registers */
	if (edev->mode & EEH_DEV_BRIDGE) {
		eeh_ops->read_config(pdn, PCI_SEC_STATUS, 2, &cfg);
		n += scnprintf(buf+n, len-n, "sec stat:%x\n", cfg);
		pr_warn("EEH: Bridge secondary status: %04x\n", cfg);

		eeh_ops->read_config(pdn, PCI_BRIDGE_CONTROL, 2, &cfg);
		n += scnprintf(buf+n, len-n, "brdg ctl:%x\n", cfg);
		pr_warn("EEH: Bridge control: %04x\n", cfg);
	}

	/* Dump out the PCI-X command and status regs */
	cap = edev->pcix_cap;
	if (cap) {
		eeh_ops->read_config(pdn, cap, 4, &cfg);
		n += scnprintf(buf+n, len-n, "pcix-cmd:%x\n", cfg);
		pr_warn("EEH: PCI-X cmd: %08x\n", cfg);

		eeh_ops->read_config(pdn, cap+4, 4, &cfg);
		n += scnprintf(buf+n, len-n, "pcix-stat:%x\n", cfg);
		pr_warn("EEH: PCI-X status: %08x\n", cfg);
	}

	/* If PCI-E capable, dump PCI-E cap 10 */
	cap = edev->pcie_cap;
	if (cap) {
		n += scnprintf(buf+n, len-n, "pci-e cap10:\n");
		pr_warn("EEH: PCI-E capabilities and status follow:\n");

		for (i=0; i<=8; i++) {
			eeh_ops->read_config(pdn, cap+4*i, 4, &cfg);
			n += scnprintf(buf+n, len-n, "%02x:%x\n", 4*i, cfg);

			if ((i % 4) == 0) {
				if (i != 0)
					pr_warn("%s\n", buffer);

				l = scnprintf(buffer, sizeof(buffer),
					      "EEH: PCI-E %02x: %08x ",
					      4*i, cfg);
			} else {
				l += scnprintf(buffer+l, sizeof(buffer)-l,
					       "%08x ", cfg);
			}

		}

		pr_warn("%s\n", buffer);
	}

	/* If AER capable, dump it */
	cap = edev->aer_cap;
	if (cap) {
		n += scnprintf(buf+n, len-n, "pci-e AER:\n");
		pr_warn("EEH: PCI-E AER capability register set follows:\n");

		for (i=0; i<=13; i++) {
			eeh_ops->read_config(pdn, cap+4*i, 4, &cfg);
			n += scnprintf(buf+n, len-n, "%02x:%x\n", 4*i, cfg);

			if ((i % 4) == 0) {
				if (i != 0)
					pr_warn("%s\n", buffer);

				l = scnprintf(buffer, sizeof(buffer),
					      "EEH: PCI-E AER %02x: %08x ",
					      4*i, cfg);
			} else {
				l += scnprintf(buffer+l, sizeof(buffer)-l,
					       "%08x ", cfg);
			}
		}

		pr_warn("%s\n", buffer);
	}

	return n;
}

static void *eeh_dump_pe_log(void *data, void *flag)
{
	struct eeh_pe *pe = data;
	struct eeh_dev *edev, *tmp;
	size_t *plen = flag;

	/* If the PE's config space is blocked, 0xFF's will be
	 * returned. It's pointless to collect the log in this
	 * case.
	 */
	if (pe->state & EEH_PE_CFG_BLOCKED)
		return NULL;

	eeh_pe_for_each_dev(pe, edev, tmp)
		*plen += eeh_dump_dev_log(edev, pci_regs_buf + *plen,
					  EEH_PCI_REGS_LOG_LEN - *plen);

	return NULL;
}

/**
 * eeh_slot_error_detail - Generate combined log including driver log and error log
 * @pe: EEH PE
 * @severity: temporary or permanent error log
 *
 * This routine should be called to generate the combined log, which
 * is comprised of driver log and error log. The driver log is figured
 * out from the config space of the corresponding PCI device, while
 * the error log is fetched through platform dependent function call.
 */
void eeh_slot_error_detail(struct eeh_pe *pe, int severity)
{
	size_t loglen = 0;

	/*
	 * When the PHB is fenced or dead, it's pointless to collect
	 * the data from PCI config space because it should return
	 * 0xFF's. For ER, we still retrieve the data from the PCI
	 * config space.
	 *
	 * For pHyp, we have to enable IO for log retrieval. Otherwise,
	 * 0xFF's is always returned from PCI config space.
	 *
	 * When the @severity is EEH_LOG_PERM, the PE is going to be
	 * removed. Prior to that, the drivers for devices included in
	 * the PE will be closed. The drivers rely on working IO path
	 * to bring the devices to quiet state. Otherwise, PCI traffic
	 * from those devices after they are removed is like to cause
	 * another unexpected EEH error.
	 */
	if (!(pe->type & EEH_PE_PHB)) {
		if (eeh_has_flag(EEH_ENABLE_IO_FOR_LOG) ||
		    severity == EEH_LOG_PERM)
			eeh_pci_enable(pe, EEH_OPT_THAW_MMIO);

		/*
		 * The config space of some PCI devices can't be accessed
		 * when their PEs are in frozen state. Otherwise, fenced
		 * PHB might be seen. Those PEs are identified with flag
		 * EEH_PE_CFG_RESTRICTED, indicating EEH_PE_CFG_BLOCKED
		 * is set automatically when the PE is put to EEH_PE_ISOLATED.
		 *
		 * Restoring BARs possibly triggers PCI config access in
		 * (OPAL) firmware and then causes fenced PHB. If the
		 * PCI config is blocked with flag EEH_PE_CFG_BLOCKED, it's
		 * pointless to restore BARs and dump config space.
		 */
		eeh_ops->configure_bridge(pe);
		if (!(pe->state & EEH_PE_CFG_BLOCKED)) {
			eeh_pe_restore_bars(pe);

			pci_regs_buf[0] = 0;
			eeh_pe_traverse(pe, eeh_dump_pe_log, &loglen);
		}
	}

	eeh_ops->get_log(pe, severity, pci_regs_buf, loglen);
}

/**
 * eeh_token_to_phys - Convert EEH address token to phys address
 * @token: I/O token, should be address in the form 0xA....
 *
 * This routine should be called to convert virtual I/O address
 * to physical one.
 */
static inline unsigned long eeh_token_to_phys(unsigned long token)
{
	pte_t *ptep;
	unsigned long pa;
	int hugepage_shift;

	/*
	 * We won't find hugepages here(this is iomem). Hence we are not
	 * worried about _PAGE_SPLITTING/collapse. Also we will not hit
	 * page table free, because of init_mm.
	 */
	ptep = __find_linux_pte_or_hugepte(init_mm.pgd, token, &hugepage_shift);
	if (!ptep)
		return token;
	WARN_ON(hugepage_shift);
	pa = pte_pfn(*ptep) << PAGE_SHIFT;

	return pa | (token & (PAGE_SIZE-1));
}

/*
 * On PowerNV platform, we might already have fenced PHB there.
 * For that case, it's meaningless to recover frozen PE. Intead,
 * We have to handle fenced PHB firstly.
 */
static int eeh_phb_check_failure(struct eeh_pe *pe)
{
	struct eeh_pe *phb_pe;
	unsigned long flags;
	int ret;

	if (!eeh_has_flag(EEH_PROBE_MODE_DEV))
		return -EPERM;

	/* Find the PHB PE */
	phb_pe = eeh_phb_pe_get(pe->phb);
	if (!phb_pe) {
		pr_warn("%s Can't find PE for PHB#%d\n",
			__func__, pe->phb->global_number);
		return -EEXIST;
	}

	/* If the PHB has been in problematic state */
	eeh_serialize_lock(&flags);
	if (phb_pe->state & EEH_PE_ISOLATED) {
		ret = 0;
		goto out;
	}

	/* Check PHB state */
	ret = eeh_ops->get_state(phb_pe, NULL);
	if ((ret < 0) ||
	    (ret == EEH_STATE_NOT_SUPPORT) ||
	    (ret & (EEH_STATE_MMIO_ACTIVE | EEH_STATE_DMA_ACTIVE)) ==
	    (EEH_STATE_MMIO_ACTIVE | EEH_STATE_DMA_ACTIVE)) {
		ret = 0;
		goto out;
	}

	/* Isolate the PHB and send event */
	eeh_pe_state_mark(phb_pe, EEH_PE_ISOLATED);
	eeh_serialize_unlock(flags);

	pr_err("EEH: PHB#%x failure detected, location: %s\n",
		phb_pe->phb->global_number, eeh_pe_loc_get(phb_pe));
	dump_stack();
	eeh_send_failure_event(phb_pe);

	return 1;
out:
	eeh_serialize_unlock(flags);
	return ret;
}

/**
 * eeh_dev_check_failure - Check if all 1's data is due to EEH slot freeze
 * @edev: eeh device
 *
 * Check for an EEH failure for the given device node.  Call this
 * routine if the result of a read was all 0xff's and you want to
 * find out if this is due to an EEH slot freeze.  This routine
 * will query firmware for the EEH status.
 *
 * Returns 0 if there has not been an EEH error; otherwise returns
 * a non-zero value and queues up a slot isolation event notification.
 *
 * It is safe to call this routine in an interrupt context.
 */
int eeh_dev_check_failure(struct eeh_dev *edev)
{
	int ret;
	int active_flags = (EEH_STATE_MMIO_ACTIVE | EEH_STATE_DMA_ACTIVE);
	unsigned long flags;
	struct pci_dn *pdn;
	struct pci_dev *dev;
	struct eeh_pe *pe, *parent_pe, *phb_pe;
	int rc = 0;
	const char *location = NULL;

	eeh_stats.total_mmio_ffs++;

	if (!eeh_enabled())
		return 0;

	if (!edev) {
		eeh_stats.no_dn++;
		return 0;
	}
	dev = eeh_dev_to_pci_dev(edev);
	pe = eeh_dev_to_pe(edev);

	/* Access to IO BARs might get this far and still not want checking. */
	if (!pe) {
		eeh_stats.ignored_check++;
		pr_debug("EEH: Ignored check for %s\n",
			eeh_pci_name(dev));
		return 0;
	}

	if (!pe->addr && !pe->config_addr) {
		eeh_stats.no_cfg_addr++;
		return 0;
	}

	/*
	 * On PowerNV platform, we might already have fenced PHB
	 * there and we need take care of that firstly.
	 */
	ret = eeh_phb_check_failure(pe);
	if (ret > 0)
		return ret;

	/*
	 * If the PE isn't owned by us, we shouldn't check the
	 * state. Instead, let the owner handle it if the PE has
	 * been frozen.
	 */
	if (eeh_pe_passed(pe))
		return 0;

	/* If we already have a pending isolation event for this
	 * slot, we know it's bad already, we don't need to check.
	 * Do this checking under a lock; as multiple PCI devices
	 * in one slot might report errors simultaneously, and we
	 * only want one error recovery routine running.
	 */
	eeh_serialize_lock(&flags);
	rc = 1;
	if (pe->state & EEH_PE_ISOLATED) {
		pe->check_count++;
		if (pe->check_count % EEH_MAX_FAILS == 0) {
			pdn = eeh_dev_to_pdn(edev);
			if (pdn->node)
				location = of_get_property(pdn->node, "ibm,loc-code", NULL);
			printk(KERN_ERR "EEH: %d reads ignored for recovering device at "
				"location=%s driver=%s pci addr=%s\n",
				pe->check_count,
				location ? location : "unknown",
				eeh_driver_name(dev), eeh_pci_name(dev));
			printk(KERN_ERR "EEH: Might be infinite loop in %s driver\n",
				eeh_driver_name(dev));
			dump_stack();
		}
		goto dn_unlock;
	}

	/*
	 * Now test for an EEH failure.  This is VERY expensive.
	 * Note that the eeh_config_addr may be a parent device
	 * in the case of a device behind a bridge, or it may be
	 * function zero of a multi-function device.
	 * In any case they must share a common PHB.
	 */
	ret = eeh_ops->get_state(pe, NULL);

	/* Note that config-io to empty slots may fail;
	 * they are empty when they don't have children.
	 * We will punt with the following conditions: Failure to get
	 * PE's state, EEH not support and Permanently unavailable
	 * state, PE is in good state.
	 */
	if ((ret < 0) ||
	    (ret == EEH_STATE_NOT_SUPPORT) ||
	    ((ret & active_flags) == active_flags)) {
		eeh_stats.false_positives++;
		pe->false_positives++;
		rc = 0;
		goto dn_unlock;
	}

	/*
	 * It should be corner case that the parent PE has been
	 * put into frozen state as well. We should take care
	 * that at first.
	 */
	parent_pe = pe->parent;
	while (parent_pe) {
		/* Hit the ceiling ? */
		if (parent_pe->type & EEH_PE_PHB)
			break;

		/* Frozen parent PE ? */
		ret = eeh_ops->get_state(parent_pe, NULL);
		if (ret > 0 &&
		    (ret & active_flags) != active_flags)
			pe = parent_pe;

		/* Next parent level */
		parent_pe = parent_pe->parent;
	}

	eeh_stats.slot_resets++;

	/* Avoid repeated reports of this failure, including problems
	 * with other functions on this device, and functions under
	 * bridges.
	 */
	eeh_pe_state_mark(pe, EEH_PE_ISOLATED);
	eeh_serialize_unlock(flags);

	/* Most EEH events are due to device driver bugs.  Having
	 * a stack trace will help the device-driver authors figure
	 * out what happened.  So print that out.
	 */
	phb_pe = eeh_phb_pe_get(pe->phb);
	pr_err("EEH: Frozen PHB#%x-PE#%x detected\n",
	       pe->phb->global_number, pe->addr);
	pr_err("EEH: PE location: %s, PHB location: %s\n",
	       eeh_pe_loc_get(pe), eeh_pe_loc_get(phb_pe));
	dump_stack();

	eeh_send_failure_event(pe);

	return 1;

dn_unlock:
	eeh_serialize_unlock(flags);
	return rc;
}

EXPORT_SYMBOL_GPL(eeh_dev_check_failure);

/**
 * eeh_check_failure - Check if all 1's data is due to EEH slot freeze
 * @token: I/O address
 *
 * Check for an EEH failure at the given I/O address. Call this
 * routine if the result of a read was all 0xff's and you want to
 * find out if this is due to an EEH slot freeze event. This routine
 * will query firmware for the EEH status.
 *
 * Note this routine is safe to call in an interrupt context.
 */
int eeh_check_failure(const volatile void __iomem *token)
{
	unsigned long addr;
	struct eeh_dev *edev;

	/* Finding the phys addr + pci device; this is pretty quick. */
	addr = eeh_token_to_phys((unsigned long __force) token);
	edev = eeh_addr_cache_get_dev(addr);
	if (!edev) {
		eeh_stats.no_device++;
		return 0;
	}

	return eeh_dev_check_failure(edev);
}
EXPORT_SYMBOL(eeh_check_failure);


/**
 * eeh_pci_enable - Enable MMIO or DMA transfers for this slot
 * @pe: EEH PE
 *
 * This routine should be called to reenable frozen MMIO or DMA
 * so that it would work correctly again. It's useful while doing
 * recovery or log collection on the indicated device.
 */
int eeh_pci_enable(struct eeh_pe *pe, int function)
{
	int active_flag, rc;

	/*
	 * pHyp doesn't allow to enable IO or DMA on unfrozen PE.
	 * Also, it's pointless to enable them on unfrozen PE. So
	 * we have to check before enabling IO or DMA.
	 */
	switch (function) {
	case EEH_OPT_THAW_MMIO:
		active_flag = EEH_STATE_MMIO_ACTIVE;
		break;
	case EEH_OPT_THAW_DMA:
		active_flag = EEH_STATE_DMA_ACTIVE;
		break;
	case EEH_OPT_DISABLE:
	case EEH_OPT_ENABLE:
	case EEH_OPT_FREEZE_PE:
		active_flag = 0;
		break;
	default:
		pr_warn("%s: Invalid function %d\n",
			__func__, function);
		return -EINVAL;
	}

	/*
	 * Check if IO or DMA has been enabled before
	 * enabling them.
	 */
	if (active_flag) {
		rc = eeh_ops->get_state(pe, NULL);
		if (rc < 0)
			return rc;

		/* Needn't enable it at all */
		if (rc == EEH_STATE_NOT_SUPPORT)
			return 0;

		/* It's already enabled */
		if (rc & active_flag)
			return 0;
	}


	/* Issue the request */
	rc = eeh_ops->set_option(pe, function);
	if (rc)
		pr_warn("%s: Unexpected state change %d on "
			"PHB#%d-PE#%x, err=%d\n",
			__func__, function, pe->phb->global_number,
			pe->addr, rc);

	/* Check if the request is finished successfully */
	if (active_flag) {
		rc = eeh_ops->wait_state(pe, PCI_BUS_RESET_WAIT_MSEC);
		if (rc < 0)
			return rc;

		if (rc & active_flag)
			return 0;

		return -EIO;
	}

	return rc;
}

static void *eeh_disable_and_save_dev_state(void *data, void *userdata)
{
	struct eeh_dev *edev = data;
	struct pci_dev *pdev = eeh_dev_to_pci_dev(edev);
	struct pci_dev *dev = userdata;

	/*
	 * The caller should have disabled and saved the
	 * state for the specified device
	 */
	if (!pdev || pdev == dev)
		return NULL;

	/* Ensure we have D0 power state */
	pci_set_power_state(pdev, PCI_D0);

	/* Save device state */
	pci_save_state(pdev);

	/*
	 * Disable device to avoid any DMA traffic and
	 * interrupt from the device
	 */
	pci_write_config_word(pdev, PCI_COMMAND, PCI_COMMAND_INTX_DISABLE);

	return NULL;
}

static void *eeh_restore_dev_state(void *data, void *userdata)
{
	struct eeh_dev *edev = data;
	struct pci_dn *pdn = eeh_dev_to_pdn(edev);
	struct pci_dev *pdev = eeh_dev_to_pci_dev(edev);
	struct pci_dev *dev = userdata;

	if (!pdev)
		return NULL;

	/* Apply customization from firmware */
	if (pdn && eeh_ops->restore_config)
		eeh_ops->restore_config(pdn);

	/* The caller should restore state for the specified device */
	if (pdev != dev)
		pci_save_state(pdev);

	return NULL;
}

/**
 * pcibios_set_pcie_slot_reset - Set PCI-E reset state
 * @dev: pci device struct
 * @state: reset state to enter
 *
 * Return value:
 * 	0 if success
 */
int pcibios_set_pcie_reset_state(struct pci_dev *dev, enum pcie_reset_state state)
{
	struct eeh_dev *edev = pci_dev_to_eeh_dev(dev);
	struct eeh_pe *pe = eeh_dev_to_pe(edev);

	if (!pe) {
		pr_err("%s: No PE found on PCI device %s\n",
			__func__, pci_name(dev));
		return -EINVAL;
	}

	switch (state) {
	case pcie_deassert_reset:
		eeh_ops->reset(pe, EEH_RESET_DEACTIVATE);
		eeh_unfreeze_pe(pe, false);
		eeh_pe_state_clear(pe, EEH_PE_CFG_BLOCKED);
		eeh_pe_dev_traverse(pe, eeh_restore_dev_state, dev);
		eeh_pe_state_clear(pe, EEH_PE_ISOLATED);
		break;
	case pcie_hot_reset:
		eeh_pe_state_mark(pe, EEH_PE_ISOLATED);
		eeh_ops->set_option(pe, EEH_OPT_FREEZE_PE);
		eeh_pe_dev_traverse(pe, eeh_disable_and_save_dev_state, dev);
		eeh_pe_state_mark(pe, EEH_PE_CFG_BLOCKED);
		eeh_ops->reset(pe, EEH_RESET_HOT);
		break;
	case pcie_warm_reset:
		eeh_pe_state_mark(pe, EEH_PE_ISOLATED);
		eeh_ops->set_option(pe, EEH_OPT_FREEZE_PE);
		eeh_pe_dev_traverse(pe, eeh_disable_and_save_dev_state, dev);
		eeh_pe_state_mark(pe, EEH_PE_CFG_BLOCKED);
		eeh_ops->reset(pe, EEH_RESET_FUNDAMENTAL);
		break;
	default:
		eeh_pe_state_clear(pe, EEH_PE_ISOLATED | EEH_PE_CFG_BLOCKED);
		return -EINVAL;
	};

	return 0;
}

/**
 * eeh_set_pe_freset - Check the required reset for the indicated device
 * @data: EEH device
 * @flag: return value
 *
 * Each device might have its preferred reset type: fundamental or
 * hot reset. The routine is used to collected the information for
 * the indicated device and its children so that the bunch of the
 * devices could be reset properly.
 */
static void *eeh_set_dev_freset(void *data, void *flag)
{
	struct pci_dev *dev;
	unsigned int *freset = (unsigned int *)flag;
	struct eeh_dev *edev = (struct eeh_dev *)data;

	dev = eeh_dev_to_pci_dev(edev);
	if (dev)
		*freset |= dev->needs_freset;

	return NULL;
}

/**
 * eeh_reset_pe_once - Assert the pci #RST line for 1/4 second
 * @pe: EEH PE
 *
 * Assert the PCI #RST line for 1/4 second.
 */
static void eeh_reset_pe_once(struct eeh_pe *pe)
{
	unsigned int freset = 0;

	/* Determine type of EEH reset required for
	 * Partitionable Endpoint, a hot-reset (1)
	 * or a fundamental reset (3).
	 * A fundamental reset required by any device under
	 * Partitionable Endpoint trumps hot-reset.
	 */
	eeh_pe_dev_traverse(pe, eeh_set_dev_freset, &freset);

	if (freset)
		eeh_ops->reset(pe, EEH_RESET_FUNDAMENTAL);
	else
		eeh_ops->reset(pe, EEH_RESET_HOT);

	eeh_ops->reset(pe, EEH_RESET_DEACTIVATE);
}

/**
 * eeh_reset_pe - Reset the indicated PE
 * @pe: EEH PE
 *
 * This routine should be called to reset indicated device, including
 * PE. A PE might include multiple PCI devices and sometimes PCI bridges
 * might be involved as well.
 */
int eeh_reset_pe(struct eeh_pe *pe)
{
	int flags = (EEH_STATE_MMIO_ACTIVE | EEH_STATE_DMA_ACTIVE);
	int i, state, ret;

	/* Mark as reset and block config space */
	eeh_pe_state_mark(pe, EEH_PE_RESET | EEH_PE_CFG_BLOCKED);

	/* Take three shots at resetting the bus */
	for (i = 0; i < 3; i++) {
		eeh_reset_pe_once(pe);

		/*
		 * EEH_PE_ISOLATED is expected to be removed after
		 * BAR restore.
		 */
		state = eeh_ops->wait_state(pe, PCI_BUS_RESET_WAIT_MSEC);
		if ((state & flags) == flags) {
			ret = 0;
			goto out;
		}

		if (state < 0) {
			pr_warn("%s: Unrecoverable slot failure on PHB#%d-PE#%x",
				__func__, pe->phb->global_number, pe->addr);
			ret = -ENOTRECOVERABLE;
			goto out;
		}

		/* We might run out of credits */
		ret = -EIO;
		pr_warn("%s: Failure %d resetting PHB#%x-PE#%x\n (%d)\n",
			__func__, state, pe->phb->global_number, pe->addr, (i + 1));
	}

out:
	eeh_pe_state_clear(pe, EEH_PE_RESET | EEH_PE_CFG_BLOCKED);
	return ret;
}

/**
 * eeh_save_bars - Save device bars
 * @edev: PCI device associated EEH device
 *
 * Save the values of the device bars. Unlike the restore
 * routine, this routine is *not* recursive. This is because
 * PCI devices are added individually; but, for the restore,
 * an entire slot is reset at a time.
 */
void eeh_save_bars(struct eeh_dev *edev)
{
	struct pci_dn *pdn;
	int i;

	pdn = eeh_dev_to_pdn(edev);
	if (!pdn)
		return;

	for (i = 0; i < 16; i++)
		eeh_ops->read_config(pdn, i * 4, 4, &edev->config_space[i]);

	/*
	 * For PCI bridges including root port, we need enable bus
	 * master explicitly. Otherwise, it can't fetch IODA table
	 * entries correctly. So we cache the bit in advance so that
	 * we can restore it after reset, either PHB range or PE range.
	 */
	if (edev->mode & EEH_DEV_BRIDGE)
		edev->config_space[1] |= PCI_COMMAND_MASTER;
}

/**
 * eeh_ops_register - Register platform dependent EEH operations
 * @ops: platform dependent EEH operations
 *
 * Register the platform dependent EEH operation callback
 * functions. The platform should call this function before
 * any other EEH operations.
 */
int __init eeh_ops_register(struct eeh_ops *ops)
{
	if (!ops->name) {
		pr_warn("%s: Invalid EEH ops name for %p\n",
			__func__, ops);
		return -EINVAL;
	}

	if (eeh_ops && eeh_ops != ops) {
		pr_warn("%s: EEH ops of platform %s already existing (%s)\n",
			__func__, eeh_ops->name, ops->name);
		return -EEXIST;
	}

	eeh_ops = ops;

	return 0;
}

/**
 * eeh_ops_unregister - Unreigster platform dependent EEH operations
 * @name: name of EEH platform operations
 *
 * Unregister the platform dependent EEH operation callback
 * functions.
 */
int __exit eeh_ops_unregister(const char *name)
{
	if (!name || !strlen(name)) {
		pr_warn("%s: Invalid EEH ops name\n",
			__func__);
		return -EINVAL;
	}

	if (eeh_ops && !strcmp(eeh_ops->name, name)) {
		eeh_ops = NULL;
		return 0;
	}

	return -EEXIST;
}

static int eeh_reboot_notifier(struct notifier_block *nb,
			       unsigned long action, void *unused)
{
	eeh_clear_flag(EEH_ENABLED);
	return NOTIFY_DONE;
}

static struct notifier_block eeh_reboot_nb = {
	.notifier_call = eeh_reboot_notifier,
};

/**
 * eeh_init - EEH initialization
 *
 * Initialize EEH by trying to enable it for all of the adapters in the system.
 * As a side effect we can determine here if eeh is supported at all.
 * Note that we leave EEH on so failed config cycles won't cause a machine
 * check.  If a user turns off EEH for a particular adapter they are really
 * telling Linux to ignore errors.  Some hardware (e.g. POWER5) won't
 * grant access to a slot if EEH isn't enabled, and so we always enable
 * EEH for all slots/all devices.
 *
 * The eeh-force-off option disables EEH checking globally, for all slots.
 * Even if force-off is set, the EEH hardware is still enabled, so that
 * newer systems can boot.
 */
int eeh_init(void)
{
	struct pci_controller *hose, *tmp;
	struct pci_dn *pdn;
	static int cnt = 0;
	int ret = 0;

	/*
	 * We have to delay the initialization on PowerNV after
	 * the PCI hierarchy tree has been built because the PEs
	 * are figured out based on PCI devices instead of device
	 * tree nodes
	 */
	if (machine_is(powernv) && cnt++ <= 0)
		return ret;

	/* Register reboot notifier */
	ret = register_reboot_notifier(&eeh_reboot_nb);
	if (ret) {
		pr_warn("%s: Failed to register notifier (%d)\n",
			__func__, ret);
		return ret;
	}

	/* call platform initialization function */
	if (!eeh_ops) {
		pr_warn("%s: Platform EEH operation not found\n",
			__func__);
		return -EEXIST;
	} else if ((ret = eeh_ops->init()))
		return ret;

	/* Initialize EEH event */
	ret = eeh_event_init();
	if (ret)
		return ret;

	/* Enable EEH for all adapters */
	list_for_each_entry_safe(hose, tmp, &hose_list, list_node) {
		pdn = hose->pci_data;
		traverse_pci_dn(pdn, eeh_ops->probe, NULL);
	}

	/*
	 * Call platform post-initialization. Actually, It's good chance
	 * to inform platform that EEH is ready to supply service if the
	 * I/O cache stuff has been built up.
	 */
	if (eeh_ops->post_init) {
		ret = eeh_ops->post_init();
		if (ret)
			return ret;
	}

	if (eeh_enabled())
		pr_info("EEH: PCI Enhanced I/O Error Handling Enabled\n");
	else
		pr_warn("EEH: No capable adapters found\n");

	return ret;
}

core_initcall_sync(eeh_init);

/**
 * eeh_add_device_early - Enable EEH for the indicated device node
 * @pdn: PCI device node for which to set up EEH
 *
 * This routine must be used to perform EEH initialization for PCI
 * devices that were added after system boot (e.g. hotplug, dlpar).
 * This routine must be called before any i/o is performed to the
 * adapter (inluding any config-space i/o).
 * Whether this actually enables EEH or not for this device depends
 * on the CEC architecture, type of the device, on earlier boot
 * command-line arguments & etc.
 */
void eeh_add_device_early(struct pci_dn *pdn)
{
	struct pci_controller *phb;
	struct eeh_dev *edev = pdn_to_eeh_dev(pdn);

	if (!edev)
		return;

	if (!eeh_has_flag(EEH_PROBE_MODE_DEVTREE))
		return;

	/* USB Bus children of PCI devices will not have BUID's */
	phb = edev->phb;
	if (NULL == phb ||
	    (eeh_has_flag(EEH_PROBE_MODE_DEVTREE) && 0 == phb->buid))
		return;

	eeh_ops->probe(pdn, NULL);
}

/**
 * eeh_add_device_tree_early - Enable EEH for the indicated device
 * @pdn: PCI device node
 *
 * This routine must be used to perform EEH initialization for the
 * indicated PCI device that was added after system boot (e.g.
 * hotplug, dlpar).
 */
void eeh_add_device_tree_early(struct pci_dn *pdn)
{
	struct pci_dn *n;

	if (!pdn)
		return;

	list_for_each_entry(n, &pdn->child_list, list)
		eeh_add_device_tree_early(n);
	eeh_add_device_early(pdn);
}
EXPORT_SYMBOL_GPL(eeh_add_device_tree_early);

/**
 * eeh_add_device_late - Perform EEH initialization for the indicated pci device
 * @dev: pci device for which to set up EEH
 *
 * This routine must be used to complete EEH initialization for PCI
 * devices that were added after system boot (e.g. hotplug, dlpar).
 */
void eeh_add_device_late(struct pci_dev *dev)
{
	struct pci_dn *pdn;
	struct eeh_dev *edev;

	if (!dev || !eeh_enabled())
		return;

	pr_debug("EEH: Adding device %s\n", pci_name(dev));

	pdn = pci_get_pdn_by_devfn(dev->bus, dev->devfn);
	edev = pdn_to_eeh_dev(pdn);
	if (edev->pdev == dev) {
		pr_debug("EEH: Already referenced !\n");
		return;
	}

	/*
	 * The EEH cache might not be removed correctly because of
	 * unbalanced kref to the device during unplug time, which
	 * relies on pcibios_release_device(). So we have to remove
	 * that here explicitly.
	 */
	if (edev->pdev) {
		eeh_rmv_from_parent_pe(edev);
		eeh_addr_cache_rmv_dev(edev->pdev);
		eeh_sysfs_remove_device(edev->pdev);
		edev->mode &= ~EEH_DEV_SYSFS;

		/*
		 * We definitely should have the PCI device removed
		 * though it wasn't correctly. So we needn't call
		 * into error handler afterwards.
		 */
		edev->mode |= EEH_DEV_NO_HANDLER;

		edev->pdev = NULL;
		dev->dev.archdata.edev = NULL;
	}

	if (eeh_has_flag(EEH_PROBE_MODE_DEV))
		eeh_ops->probe(pdn, NULL);

	edev->pdev = dev;
	dev->dev.archdata.edev = edev;

	eeh_addr_cache_insert_dev(dev);
}

/**
 * eeh_add_device_tree_late - Perform EEH initialization for the indicated PCI bus
 * @bus: PCI bus
 *
 * This routine must be used to perform EEH initialization for PCI
 * devices which are attached to the indicated PCI bus. The PCI bus
 * is added after system boot through hotplug or dlpar.
 */
void eeh_add_device_tree_late(struct pci_bus *bus)
{
	struct pci_dev *dev;

	list_for_each_entry(dev, &bus->devices, bus_list) {
		eeh_add_device_late(dev);
		if (dev->hdr_type == PCI_HEADER_TYPE_BRIDGE) {
			struct pci_bus *subbus = dev->subordinate;
			if (subbus)
				eeh_add_device_tree_late(subbus);
		}
	}
}
EXPORT_SYMBOL_GPL(eeh_add_device_tree_late);

/**
 * eeh_add_sysfs_files - Add EEH sysfs files for the indicated PCI bus
 * @bus: PCI bus
 *
 * This routine must be used to add EEH sysfs files for PCI
 * devices which are attached to the indicated PCI bus. The PCI bus
 * is added after system boot through hotplug or dlpar.
 */
void eeh_add_sysfs_files(struct pci_bus *bus)
{
	struct pci_dev *dev;

	list_for_each_entry(dev, &bus->devices, bus_list) {
		eeh_sysfs_add_device(dev);
		if (dev->hdr_type == PCI_HEADER_TYPE_BRIDGE) {
			struct pci_bus *subbus = dev->subordinate;
			if (subbus)
				eeh_add_sysfs_files(subbus);
		}
	}
}
EXPORT_SYMBOL_GPL(eeh_add_sysfs_files);

/**
 * eeh_remove_device - Undo EEH setup for the indicated pci device
 * @dev: pci device to be removed
 *
 * This routine should be called when a device is removed from
 * a running system (e.g. by hotplug or dlpar).  It unregisters
 * the PCI device from the EEH subsystem.  I/O errors affecting
 * this device will no longer be detected after this call; thus,
 * i/o errors affecting this slot may leave this device unusable.
 */
void eeh_remove_device(struct pci_dev *dev)
{
	struct eeh_dev *edev;

	if (!dev || !eeh_enabled())
		return;
	edev = pci_dev_to_eeh_dev(dev);

	/* Unregister the device with the EEH/PCI address search system */
	pr_debug("EEH: Removing device %s\n", pci_name(dev));

	if (!edev || !edev->pdev || !edev->pe) {
		pr_debug("EEH: Not referenced !\n");
		return;
	}

	/*
	 * During the hotplug for EEH error recovery, we need the EEH
	 * device attached to the parent PE in order for BAR restore
	 * a bit later. So we keep it for BAR restore and remove it
	 * from the parent PE during the BAR resotre.
	 */
	edev->pdev = NULL;
	dev->dev.archdata.edev = NULL;
	if (!(edev->pe->state & EEH_PE_KEEP))
		eeh_rmv_from_parent_pe(edev);
	else
		edev->mode |= EEH_DEV_DISCONNECTED;

	/*
	 * We're removing from the PCI subsystem, that means
	 * the PCI device driver can't support EEH or not
	 * well. So we rely on hotplug completely to do recovery
	 * for the specific PCI device.
	 */
	edev->mode |= EEH_DEV_NO_HANDLER;

	eeh_addr_cache_rmv_dev(dev);
	eeh_sysfs_remove_device(dev);
	edev->mode &= ~EEH_DEV_SYSFS;
}

int eeh_unfreeze_pe(struct eeh_pe *pe, bool sw_state)
{
	int ret;

	ret = eeh_pci_enable(pe, EEH_OPT_THAW_MMIO);
	if (ret) {
		pr_warn("%s: Failure %d enabling IO on PHB#%x-PE#%x\n",
			__func__, ret, pe->phb->global_number, pe->addr);
		return ret;
	}

	ret = eeh_pci_enable(pe, EEH_OPT_THAW_DMA);
	if (ret) {
		pr_warn("%s: Failure %d enabling DMA on PHB#%x-PE#%x\n",
			__func__, ret, pe->phb->global_number, pe->addr);
		return ret;
	}

	/* Clear software isolated state */
	if (sw_state && (pe->state & EEH_PE_ISOLATED))
		eeh_pe_state_clear(pe, EEH_PE_ISOLATED);

	return ret;
}


static struct pci_device_id eeh_reset_ids[] = {
	{ PCI_DEVICE(0x19a2, 0x0710) },	/* Emulex, BE     */
	{ PCI_DEVICE(0x10df, 0xe220) },	/* Emulex, Lancer */
	{ PCI_DEVICE(0x14e4, 0x1657) }, /* Broadcom BCM5719 */
	{ 0 }
};

static int eeh_pe_change_owner(struct eeh_pe *pe)
{
	struct eeh_dev *edev, *tmp;
	struct pci_dev *pdev;
	struct pci_device_id *id;
	int flags, ret;

	/* Check PE state */
	flags = (EEH_STATE_MMIO_ACTIVE | EEH_STATE_DMA_ACTIVE);
	ret = eeh_ops->get_state(pe, NULL);
	if (ret < 0 || ret == EEH_STATE_NOT_SUPPORT)
		return 0;

	/* Unfrozen PE, nothing to do */
	if ((ret & flags) == flags)
		return 0;

	/* Frozen PE, check if it needs PE level reset */
	eeh_pe_for_each_dev(pe, edev, tmp) {
		pdev = eeh_dev_to_pci_dev(edev);
		if (!pdev)
			continue;

		for (id = &eeh_reset_ids[0]; id->vendor != 0; id++) {
			if (id->vendor != PCI_ANY_ID &&
			    id->vendor != pdev->vendor)
				continue;
			if (id->device != PCI_ANY_ID &&
			    id->device != pdev->device)
				continue;
			if (id->subvendor != PCI_ANY_ID &&
			    id->subvendor != pdev->subsystem_vendor)
				continue;
			if (id->subdevice != PCI_ANY_ID &&
			    id->subdevice != pdev->subsystem_device)
				continue;

			goto reset;
		}
	}

	return eeh_unfreeze_pe(pe, true);

reset:
	return eeh_pe_reset_and_recover(pe);
}

/**
 * eeh_dev_open - Increase count of pass through devices for PE
 * @pdev: PCI device
 *
 * Increase count of passed through devices for the indicated
 * PE. In the result, the EEH errors detected on the PE won't be
 * reported. The PE owner will be responsible for detection
 * and recovery.
 */
int eeh_dev_open(struct pci_dev *pdev)
{
	struct eeh_dev *edev;
	int ret = -ENODEV;

	mutex_lock(&eeh_dev_mutex);

	/* No PCI device ? */
	if (!pdev)
		goto out;

	/* No EEH device or PE ? */
	edev = pci_dev_to_eeh_dev(pdev);
	if (!edev || !edev->pe)
		goto out;

	/*
	 * The PE might have been put into frozen state, but we
	 * didn't detect that yet. The passed through PCI devices
	 * in frozen PE won't work properly. Clear the frozen state
	 * in advance.
	 */
	ret = eeh_pe_change_owner(edev->pe);
	if (ret)
		goto out;

	/* Increase PE's pass through count */
	atomic_inc(&edev->pe->pass_dev_cnt);
	mutex_unlock(&eeh_dev_mutex);

	return 0;
out:
	mutex_unlock(&eeh_dev_mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(eeh_dev_open);

/**
 * eeh_dev_release - Decrease count of pass through devices for PE
 * @pdev: PCI device
 *
 * Decrease count of pass through devices for the indicated PE. If
 * there is no passed through device in PE, the EEH errors detected
 * on the PE will be reported and handled as usual.
 */
void eeh_dev_release(struct pci_dev *pdev)
{
	struct eeh_dev *edev;

	mutex_lock(&eeh_dev_mutex);

	/* No PCI device ? */
	if (!pdev)
		goto out;

	/* No EEH device ? */
	edev = pci_dev_to_eeh_dev(pdev);
	if (!edev || !edev->pe || !eeh_pe_passed(edev->pe))
		goto out;

	/* Decrease PE's pass through count */
	atomic_dec(&edev->pe->pass_dev_cnt);
	WARN_ON(atomic_read(&edev->pe->pass_dev_cnt) < 0);
	eeh_pe_change_owner(edev->pe);
out:
	mutex_unlock(&eeh_dev_mutex);
}
EXPORT_SYMBOL(eeh_dev_release);

#ifdef CONFIG_IOMMU_API

static int dev_has_iommu_table(struct device *dev, void *data)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct pci_dev **ppdev = data;
	struct iommu_table *tbl;

	if (!dev)
		return 0;

	tbl = get_iommu_table_base(dev);
	if (tbl && tbl->it_group) {
		*ppdev = pdev;
		return 1;
	}

	return 0;
}

/**
 * eeh_iommu_group_to_pe - Convert IOMMU group to EEH PE
 * @group: IOMMU group
 *
 * The routine is called to convert IOMMU group to EEH PE.
 */
struct eeh_pe *eeh_iommu_group_to_pe(struct iommu_group *group)
{
	struct pci_dev *pdev = NULL;
	struct eeh_dev *edev;
	int ret;

	/* No IOMMU group ? */
	if (!group)
		return NULL;

	ret = iommu_group_for_each_dev(group, &pdev, dev_has_iommu_table);
	if (!ret || !pdev)
		return NULL;

	/* No EEH device or PE ? */
	edev = pci_dev_to_eeh_dev(pdev);
	if (!edev || !edev->pe)
		return NULL;

	return edev->pe;
}
EXPORT_SYMBOL_GPL(eeh_iommu_group_to_pe);

#endif /* CONFIG_IOMMU_API */

/**
 * eeh_pe_set_option - Set options for the indicated PE
 * @pe: EEH PE
 * @option: requested option
 *
 * The routine is called to enable or disable EEH functionality
 * on the indicated PE, to enable IO or DMA for the frozen PE.
 */
int eeh_pe_set_option(struct eeh_pe *pe, int option)
{
	int ret = 0;

	/* Invalid PE ? */
	if (!pe)
		return -ENODEV;

	/*
	 * EEH functionality could possibly be disabled, just
	 * return error for the case. And the EEH functinality
	 * isn't expected to be disabled on one specific PE.
	 */
	switch (option) {
	case EEH_OPT_ENABLE:
		if (eeh_enabled()) {
			ret = eeh_pe_change_owner(pe);
			break;
		}
		ret = -EIO;
		break;
	case EEH_OPT_DISABLE:
		break;
	case EEH_OPT_THAW_MMIO:
	case EEH_OPT_THAW_DMA:
		if (!eeh_ops || !eeh_ops->set_option) {
			ret = -ENOENT;
			break;
		}

		ret = eeh_pci_enable(pe, option);
		break;
	default:
		pr_debug("%s: Option %d out of range (%d, %d)\n",
			__func__, option, EEH_OPT_DISABLE, EEH_OPT_THAW_DMA);
		ret = -EINVAL;
	}

	return ret;
}
EXPORT_SYMBOL_GPL(eeh_pe_set_option);

/**
 * eeh_pe_get_state - Retrieve PE's state
 * @pe: EEH PE
 *
 * Retrieve the PE's state, which includes 3 aspects: enabled
 * DMA, enabled IO and asserted reset.
 */
int eeh_pe_get_state(struct eeh_pe *pe)
{
	int result, ret = 0;
	bool rst_active, dma_en, mmio_en;

	/* Existing PE ? */
	if (!pe)
		return -ENODEV;

	if (!eeh_ops || !eeh_ops->get_state)
		return -ENOENT;

	result = eeh_ops->get_state(pe, NULL);
	rst_active = !!(result & EEH_STATE_RESET_ACTIVE);
	dma_en = !!(result & EEH_STATE_DMA_ENABLED);
	mmio_en = !!(result & EEH_STATE_MMIO_ENABLED);

	if (rst_active)
		ret = EEH_PE_STATE_RESET;
	else if (dma_en && mmio_en)
		ret = EEH_PE_STATE_NORMAL;
	else if (!dma_en && !mmio_en)
		ret = EEH_PE_STATE_STOPPED_IO_DMA;
	else if (!dma_en && mmio_en)
		ret = EEH_PE_STATE_STOPPED_DMA;
	else
		ret = EEH_PE_STATE_UNAVAIL;

	return ret;
}
EXPORT_SYMBOL_GPL(eeh_pe_get_state);

static int eeh_pe_reenable_devices(struct eeh_pe *pe)
{
	struct eeh_dev *edev, *tmp;
	struct pci_dev *pdev;
	int ret = 0;

	/* Restore config space */
	eeh_pe_restore_bars(pe);

	/*
	 * Reenable PCI devices as the devices passed
	 * through are always enabled before the reset.
	 */
	eeh_pe_for_each_dev(pe, edev, tmp) {
		pdev = eeh_dev_to_pci_dev(edev);
		if (!pdev)
			continue;

		ret = pci_reenable_device(pdev);
		if (ret) {
			pr_warn("%s: Failure %d reenabling %s\n",
				__func__, ret, pci_name(pdev));
			return ret;
		}
	}

	/* The PE is still in frozen state */
	return eeh_unfreeze_pe(pe, true);
}

/**
 * eeh_pe_reset - Issue PE reset according to specified type
 * @pe: EEH PE
 * @option: reset type
 *
 * The routine is called to reset the specified PE with the
 * indicated type, either fundamental reset or hot reset.
 * PE reset is the most important part for error recovery.
 */
int eeh_pe_reset(struct eeh_pe *pe, int option)
{
	int ret = 0;

	/* Invalid PE ? */
	if (!pe)
		return -ENODEV;

	if (!eeh_ops || !eeh_ops->set_option || !eeh_ops->reset)
		return -ENOENT;

	switch (option) {
	case EEH_RESET_DEACTIVATE:
		ret = eeh_ops->reset(pe, option);
		eeh_pe_state_clear(pe, EEH_PE_CFG_BLOCKED);
		if (ret)
			break;

		ret = eeh_pe_reenable_devices(pe);
		break;
	case EEH_RESET_HOT:
	case EEH_RESET_FUNDAMENTAL:
		/*
		 * Proactively freeze the PE to drop all MMIO access
		 * during reset, which should be banned as it's always
		 * cause recursive EEH error.
		 */
		eeh_ops->set_option(pe, EEH_OPT_FREEZE_PE);

		eeh_pe_state_mark(pe, EEH_PE_CFG_BLOCKED);
		ret = eeh_ops->reset(pe, option);
		break;
	default:
		pr_debug("%s: Unsupported option %d\n",
			__func__, option);
		ret = -EINVAL;
	}

	return ret;
}
EXPORT_SYMBOL_GPL(eeh_pe_reset);

/**
 * eeh_pe_configure - Configure PCI bridges after PE reset
 * @pe: EEH PE
 *
 * The routine is called to restore the PCI config space for
 * those PCI devices, especially PCI bridges affected by PE
 * reset issued previously.
 */
int eeh_pe_configure(struct eeh_pe *pe)
{
	int ret = 0;

	/* Invalid PE ? */
	if (!pe)
		return -ENODEV;

	return ret;
}
EXPORT_SYMBOL_GPL(eeh_pe_configure);

static int proc_eeh_show(struct seq_file *m, void *v)
{
	if (!eeh_enabled()) {
		seq_printf(m, "EEH Subsystem is globally disabled\n");
		seq_printf(m, "eeh_total_mmio_ffs=%llu\n", eeh_stats.total_mmio_ffs);
	} else {
		seq_printf(m, "EEH Subsystem is enabled\n");
		seq_printf(m,
				"no device=%llu\n"
				"no device node=%llu\n"
				"no config address=%llu\n"
				"check not wanted=%llu\n"
				"eeh_total_mmio_ffs=%llu\n"
				"eeh_false_positives=%llu\n"
				"eeh_slot_resets=%llu\n",
				eeh_stats.no_device,
				eeh_stats.no_dn,
				eeh_stats.no_cfg_addr,
				eeh_stats.ignored_check,
				eeh_stats.total_mmio_ffs,
				eeh_stats.false_positives,
				eeh_stats.slot_resets);
	}

	return 0;
}

static int proc_eeh_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_eeh_show, NULL);
}

static const struct file_operations proc_eeh_operations = {
	.open      = proc_eeh_open,
	.read      = seq_read,
	.llseek    = seq_lseek,
	.release   = single_release,
};

#ifdef CONFIG_DEBUG_FS
static int eeh_enable_dbgfs_set(void *data, u64 val)
{
	if (val)
		eeh_clear_flag(EEH_FORCE_DISABLED);
	else
		eeh_add_flag(EEH_FORCE_DISABLED);

	/* Notify the backend */
	if (eeh_ops->post_init)
		eeh_ops->post_init();

	return 0;
}

static int eeh_enable_dbgfs_get(void *data, u64 *val)
{
	if (eeh_enabled())
		*val = 0x1ul;
	else
		*val = 0x0ul;
	return 0;
}

static int eeh_freeze_dbgfs_set(void *data, u64 val)
{
	eeh_max_freezes = val;
	return 0;
}

static int eeh_freeze_dbgfs_get(void *data, u64 *val)
{
	*val = eeh_max_freezes;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(eeh_enable_dbgfs_ops, eeh_enable_dbgfs_get,
			eeh_enable_dbgfs_set, "0x%llx\n");
DEFINE_SIMPLE_ATTRIBUTE(eeh_freeze_dbgfs_ops, eeh_freeze_dbgfs_get,
			eeh_freeze_dbgfs_set, "0x%llx\n");
#endif

static int __init eeh_init_proc(void)
{
	if (machine_is(pseries) || machine_is(powernv)) {
		proc_create("powerpc/eeh", 0, NULL, &proc_eeh_operations);
#ifdef CONFIG_DEBUG_FS
		debugfs_create_file("eeh_enable", 0600,
                                    powerpc_debugfs_root, NULL,
                                    &eeh_enable_dbgfs_ops);
		debugfs_create_file("eeh_max_freezes", 0600,
				    powerpc_debugfs_root, NULL,
				    &eeh_freeze_dbgfs_ops);
#endif
	}

	return 0;
}
__initcall(eeh_init_proc);
