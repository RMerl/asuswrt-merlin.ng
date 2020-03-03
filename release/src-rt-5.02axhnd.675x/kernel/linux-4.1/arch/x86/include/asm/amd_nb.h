#ifndef _ASM_X86_AMD_NB_H
#define _ASM_X86_AMD_NB_H

#include <linux/ioport.h>
#include <linux/pci.h>

struct amd_nb_bus_dev_range {
	u8 bus;
	u8 dev_base;
	u8 dev_limit;
};

extern const struct pci_device_id amd_nb_misc_ids[];
extern const struct amd_nb_bus_dev_range amd_nb_bus_dev_ranges[];

extern bool early_is_amd_nb(u32 value);
extern struct resource *amd_get_mmconfig_range(struct resource *res);
extern int amd_cache_northbridges(void);
extern void amd_flush_garts(void);
extern int amd_numa_init(void);
extern int amd_get_subcaches(int);
extern int amd_set_subcaches(int, unsigned long);

struct amd_l3_cache {
	unsigned indices;
	u8	 subcaches[4];
};

struct threshold_block {
	unsigned int		block;
	unsigned int		bank;
	unsigned int		cpu;
	u32			address;
	u16			interrupt_enable;
	bool			interrupt_capable;
	u16			threshold_limit;
	struct kobject		kobj;
	struct list_head	miscj;
};

struct threshold_bank {
	struct kobject		*kobj;
	struct threshold_block	*blocks;

	/* initialized to the number of CPUs on the node sharing this bank */
	atomic_t		cpus;
};

struct amd_northbridge {
	struct pci_dev *misc;
	struct pci_dev *link;
	struct amd_l3_cache l3_cache;
	struct threshold_bank *bank4;
};

struct amd_northbridge_info {
	u16 num;
	u64 flags;
	struct amd_northbridge *nb;
};
extern struct amd_northbridge_info amd_northbridges;

#define AMD_NB_GART			BIT(0)
#define AMD_NB_L3_INDEX_DISABLE		BIT(1)
#define AMD_NB_L3_PARTITIONING		BIT(2)

#ifdef CONFIG_AMD_NB

static inline u16 amd_nb_num(void)
{
	return amd_northbridges.num;
}

static inline bool amd_nb_has_feature(unsigned feature)
{
	return ((amd_northbridges.flags & feature) == feature);
}

static inline struct amd_northbridge *node_to_amd_nb(int node)
{
	return (node < amd_northbridges.num) ? &amd_northbridges.nb[node] : NULL;
}

static inline u16 amd_get_node_id(struct pci_dev *pdev)
{
	struct pci_dev *misc;
	int i;

	for (i = 0; i != amd_nb_num(); i++) {
		misc = node_to_amd_nb(i)->misc;

		if (pci_domain_nr(misc->bus) == pci_domain_nr(pdev->bus) &&
		    PCI_SLOT(misc->devfn) == PCI_SLOT(pdev->devfn))
			return i;
	}

	WARN(1, "Unable to find AMD Northbridge id for %s\n", pci_name(pdev));
	return 0;
}

#else

#define amd_nb_num(x)		0
#define amd_nb_has_feature(x)	false
#define node_to_amd_nb(x)	NULL

#endif


#endif /* _ASM_X86_AMD_NB_H */
