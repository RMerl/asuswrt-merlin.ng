/*
 * Copyright (c) 2006, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307 USA.
 *
 * Copyright (C) Ashok Raj <ashok.raj@intel.com>
 * Copyright (C) Shaohua Li <shaohua.li@intel.com>
 */

#ifndef __DMAR_H__
#define __DMAR_H__

#include <linux/acpi.h>
#include <linux/types.h>
#include <linux/msi.h>
#include <linux/irqreturn.h>
#include <linux/rwsem.h>
#include <linux/rcupdate.h>

struct acpi_dmar_header;

#ifdef	CONFIG_X86
# define	DMAR_UNITS_SUPPORTED	MAX_IO_APICS
#else
# define	DMAR_UNITS_SUPPORTED	64
#endif

/* DMAR Flags */
#define DMAR_INTR_REMAP		0x1
#define DMAR_X2APIC_OPT_OUT	0x2

struct intel_iommu;

struct dmar_dev_scope {
	struct device __rcu *dev;
	u8 bus;
	u8 devfn;
};

#ifdef CONFIG_DMAR_TABLE
extern struct acpi_table_header *dmar_tbl;
struct dmar_drhd_unit {
	struct list_head list;		/* list of drhd units	*/
	struct  acpi_dmar_header *hdr;	/* ACPI header		*/
	u64	reg_base_addr;		/* register base address*/
	struct	dmar_dev_scope *devices;/* target device array	*/
	int	devices_cnt;		/* target device count	*/
	u16	segment;		/* PCI domain		*/
	u8	ignored:1; 		/* ignore drhd		*/
	u8	include_all:1;
	struct intel_iommu *iommu;
};

struct dmar_pci_path {
	u8 bus;
	u8 device;
	u8 function;
};

struct dmar_pci_notify_info {
	struct pci_dev			*dev;
	unsigned long			event;
	int				bus;
	u16				seg;
	u16				level;
	struct dmar_pci_path		path[];
}  __attribute__((packed));

extern struct rw_semaphore dmar_global_lock;
extern struct list_head dmar_drhd_units;

#define for_each_drhd_unit(drhd) \
	list_for_each_entry_rcu(drhd, &dmar_drhd_units, list)

#define for_each_active_drhd_unit(drhd)					\
	list_for_each_entry_rcu(drhd, &dmar_drhd_units, list)		\
		if (drhd->ignored) {} else

#define for_each_active_iommu(i, drhd)					\
	list_for_each_entry_rcu(drhd, &dmar_drhd_units, list)		\
		if (i=drhd->iommu, drhd->ignored) {} else

#define for_each_iommu(i, drhd)						\
	list_for_each_entry_rcu(drhd, &dmar_drhd_units, list)		\
		if (i=drhd->iommu, 0) {} else 

static inline bool dmar_rcu_check(void)
{
	return rwsem_is_locked(&dmar_global_lock) ||
	       system_state == SYSTEM_BOOTING;
}

#define	dmar_rcu_dereference(p)	rcu_dereference_check((p), dmar_rcu_check())

#define	for_each_dev_scope(a, c, p, d)	\
	for ((p) = 0; ((d) = (p) < (c) ? dmar_rcu_dereference((a)[(p)].dev) : \
			NULL, (p) < (c)); (p)++)

#define	for_each_active_dev_scope(a, c, p, d)	\
	for_each_dev_scope((a), (c), (p), (d))	if (!(d)) { continue; } else

extern int dmar_table_init(void);
extern int dmar_dev_scope_init(void);
extern int dmar_parse_dev_scope(void *start, void *end, int *cnt,
				struct dmar_dev_scope **devices, u16 segment);
extern void *dmar_alloc_dev_scope(void *start, void *end, int *cnt);
extern void dmar_free_dev_scope(struct dmar_dev_scope **devices, int *cnt);
extern int dmar_insert_dev_scope(struct dmar_pci_notify_info *info,
				 void *start, void*end, u16 segment,
				 struct dmar_dev_scope *devices,
				 int devices_cnt);
extern int dmar_remove_dev_scope(struct dmar_pci_notify_info *info,
				 u16 segment, struct dmar_dev_scope *devices,
				 int count);
/* Intel IOMMU detection */
extern int detect_intel_iommu(void);
extern int enable_drhd_fault_handling(void);
extern int dmar_device_add(acpi_handle handle);
extern int dmar_device_remove(acpi_handle handle);

static inline int dmar_res_noop(struct acpi_dmar_header *hdr, void *arg)
{
	return 0;
}

#ifdef CONFIG_INTEL_IOMMU
extern int iommu_detected, no_iommu;
extern int intel_iommu_init(void);
extern int dmar_parse_one_rmrr(struct acpi_dmar_header *header, void *arg);
extern int dmar_parse_one_atsr(struct acpi_dmar_header *header, void *arg);
extern int dmar_check_one_atsr(struct acpi_dmar_header *hdr, void *arg);
extern int dmar_release_one_atsr(struct acpi_dmar_header *hdr, void *arg);
extern int dmar_iommu_hotplug(struct dmar_drhd_unit *dmaru, bool insert);
extern int dmar_iommu_notify_scope_dev(struct dmar_pci_notify_info *info);
#else /* !CONFIG_INTEL_IOMMU: */
static inline int intel_iommu_init(void) { return -ENODEV; }

#define	dmar_parse_one_rmrr		dmar_res_noop
#define	dmar_parse_one_atsr		dmar_res_noop
#define	dmar_check_one_atsr		dmar_res_noop
#define	dmar_release_one_atsr		dmar_res_noop

static inline int dmar_iommu_notify_scope_dev(struct dmar_pci_notify_info *info)
{
	return 0;
}

static inline int dmar_iommu_hotplug(struct dmar_drhd_unit *dmaru, bool insert)
{
	return 0;
}
#endif /* CONFIG_INTEL_IOMMU */

#ifdef CONFIG_IRQ_REMAP
extern int dmar_ir_hotplug(struct dmar_drhd_unit *dmaru, bool insert);
#else  /* CONFIG_IRQ_REMAP */
static inline int dmar_ir_hotplug(struct dmar_drhd_unit *dmaru, bool insert)
{ return 0; }
#endif /* CONFIG_IRQ_REMAP */

#else /* CONFIG_DMAR_TABLE */

static inline int dmar_device_add(void *handle)
{
	return 0;
}

static inline int dmar_device_remove(void *handle)
{
	return 0;
}

#endif /* CONFIG_DMAR_TABLE */

struct irte {
	union {
		struct {
			__u64	present 	: 1,
				fpd		: 1,
				dst_mode	: 1,
				redir_hint	: 1,
				trigger_mode	: 1,
				dlvry_mode	: 3,
				avail		: 4,
				__reserved_1	: 4,
				vector		: 8,
				__reserved_2	: 8,
				dest_id		: 32;
		};
		__u64 low;
	};

	union {
		struct {
			__u64	sid		: 16,
				sq		: 2,
				svt		: 2,
				__reserved_3	: 44;
		};
		__u64 high;
	};
};

enum {
	IRQ_REMAP_XAPIC_MODE,
	IRQ_REMAP_X2APIC_MODE,
};

/* Can't use the common MSI interrupt functions
 * since DMAR is not a pci device
 */
struct irq_data;
extern void dmar_msi_unmask(struct irq_data *data);
extern void dmar_msi_mask(struct irq_data *data);
extern void dmar_msi_read(int irq, struct msi_msg *msg);
extern void dmar_msi_write(int irq, struct msi_msg *msg);
extern int dmar_set_interrupt(struct intel_iommu *iommu);
extern irqreturn_t dmar_fault(int irq, void *dev_id);
extern int arch_setup_dmar_msi(unsigned int irq);

#endif /* __DMAR_H__ */
