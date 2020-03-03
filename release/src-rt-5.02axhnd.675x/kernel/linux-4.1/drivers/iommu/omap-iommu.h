/*
 * omap iommu: main structures
 *
 * Copyright (C) 2008-2009 Nokia Corporation
 *
 * Written by Hiroshi DOYU <Hiroshi.DOYU@nokia.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _OMAP_IOMMU_H
#define _OMAP_IOMMU_H

struct iotlb_entry {
	u32 da;
	u32 pa;
	u32 pgsz, prsvd, valid;
	union {
		u16 ap;
		struct {
			u32 endian, elsz, mixed;
		};
	};
};

struct omap_iommu {
	const char	*name;
	void __iomem	*regbase;
	struct device	*dev;
	struct iommu_domain *domain;
	struct dentry	*debug_dir;

	spinlock_t	iommu_lock;	/* global for this whole object */

	/*
	 * We don't change iopgd for a situation like pgd for a task,
	 * but share it globally for each iommu.
	 */
	u32		*iopgd;
	spinlock_t	page_table_lock; /* protect iopgd */

	int		nr_tlb_entries;

	void *ctx; /* iommu context: registres saved area */

	int has_bus_err_back;
};

struct cr_regs {
	union {
		struct {
			u16 cam_l;
			u16 cam_h;
		};
		u32 cam;
	};
	union {
		struct {
			u16 ram_l;
			u16 ram_h;
		};
		u32 ram;
	};
};

/**
 * dev_to_omap_iommu() - retrieves an omap iommu object from a user device
 * @dev: iommu client device
 */
static inline struct omap_iommu *dev_to_omap_iommu(struct device *dev)
{
	struct omap_iommu_arch_data *arch_data = dev->archdata.iommu;

	return arch_data->iommu_dev;
}

/*
 * MMU Register offsets
 */
#define MMU_REVISION		0x00
#define MMU_IRQSTATUS		0x18
#define MMU_IRQENABLE		0x1c
#define MMU_WALKING_ST		0x40
#define MMU_CNTL		0x44
#define MMU_FAULT_AD		0x48
#define MMU_TTB			0x4c
#define MMU_LOCK		0x50
#define MMU_LD_TLB		0x54
#define MMU_CAM			0x58
#define MMU_RAM			0x5c
#define MMU_GFLUSH		0x60
#define MMU_FLUSH_ENTRY		0x64
#define MMU_READ_CAM		0x68
#define MMU_READ_RAM		0x6c
#define MMU_EMU_FAULT_AD	0x70
#define MMU_GP_REG		0x88

#define MMU_REG_SIZE		256

/*
 * MMU Register bit definitions
 */
/* IRQSTATUS & IRQENABLE */
#define MMU_IRQ_MULTIHITFAULT	(1 << 4)
#define MMU_IRQ_TABLEWALKFAULT	(1 << 3)
#define MMU_IRQ_EMUMISS		(1 << 2)
#define MMU_IRQ_TRANSLATIONFAULT	(1 << 1)
#define MMU_IRQ_TLBMISS		(1 << 0)

#define __MMU_IRQ_FAULT		\
	(MMU_IRQ_MULTIHITFAULT | MMU_IRQ_EMUMISS | MMU_IRQ_TRANSLATIONFAULT)
#define MMU_IRQ_MASK		\
	(__MMU_IRQ_FAULT | MMU_IRQ_TABLEWALKFAULT | MMU_IRQ_TLBMISS)
#define MMU_IRQ_TWL_MASK	(__MMU_IRQ_FAULT | MMU_IRQ_TABLEWALKFAULT)
#define MMU_IRQ_TLB_MISS_MASK	(__MMU_IRQ_FAULT | MMU_IRQ_TLBMISS)

/* MMU_CNTL */
#define MMU_CNTL_SHIFT		1
#define MMU_CNTL_MASK		(7 << MMU_CNTL_SHIFT)
#define MMU_CNTL_EML_TLB	(1 << 3)
#define MMU_CNTL_TWL_EN		(1 << 2)
#define MMU_CNTL_MMU_EN		(1 << 1)

/* CAM */
#define MMU_CAM_VATAG_SHIFT	12
#define MMU_CAM_VATAG_MASK \
	((~0UL >> MMU_CAM_VATAG_SHIFT) << MMU_CAM_VATAG_SHIFT)
#define MMU_CAM_P		(1 << 3)
#define MMU_CAM_V		(1 << 2)
#define MMU_CAM_PGSZ_MASK	3
#define MMU_CAM_PGSZ_1M		(0 << 0)
#define MMU_CAM_PGSZ_64K	(1 << 0)
#define MMU_CAM_PGSZ_4K		(2 << 0)
#define MMU_CAM_PGSZ_16M	(3 << 0)

/* RAM */
#define MMU_RAM_PADDR_SHIFT	12
#define MMU_RAM_PADDR_MASK \
	((~0UL >> MMU_RAM_PADDR_SHIFT) << MMU_RAM_PADDR_SHIFT)

#define MMU_RAM_ENDIAN_SHIFT	9
#define MMU_RAM_ENDIAN_MASK	(1 << MMU_RAM_ENDIAN_SHIFT)
#define MMU_RAM_ENDIAN_LITTLE	(0 << MMU_RAM_ENDIAN_SHIFT)
#define MMU_RAM_ENDIAN_BIG	(1 << MMU_RAM_ENDIAN_SHIFT)

#define MMU_RAM_ELSZ_SHIFT	7
#define MMU_RAM_ELSZ_MASK	(3 << MMU_RAM_ELSZ_SHIFT)
#define MMU_RAM_ELSZ_8		(0 << MMU_RAM_ELSZ_SHIFT)
#define MMU_RAM_ELSZ_16		(1 << MMU_RAM_ELSZ_SHIFT)
#define MMU_RAM_ELSZ_32		(2 << MMU_RAM_ELSZ_SHIFT)
#define MMU_RAM_ELSZ_NONE	(3 << MMU_RAM_ELSZ_SHIFT)
#define MMU_RAM_MIXED_SHIFT	6
#define MMU_RAM_MIXED_MASK	(1 << MMU_RAM_MIXED_SHIFT)
#define MMU_RAM_MIXED		MMU_RAM_MIXED_MASK

#define MMU_GP_REG_BUS_ERR_BACK_EN	0x1

#define get_cam_va_mask(pgsz)				\
	(((pgsz) == MMU_CAM_PGSZ_16M) ? 0xff000000 :	\
	 ((pgsz) == MMU_CAM_PGSZ_1M)  ? 0xfff00000 :	\
	 ((pgsz) == MMU_CAM_PGSZ_64K) ? 0xffff0000 :	\
	 ((pgsz) == MMU_CAM_PGSZ_4K)  ? 0xfffff000 : 0)

/*
 * utilities for super page(16MB, 1MB, 64KB and 4KB)
 */

#define iopgsz_max(bytes)			\
	(((bytes) >= SZ_16M) ? SZ_16M :		\
	 ((bytes) >= SZ_1M)  ? SZ_1M  :		\
	 ((bytes) >= SZ_64K) ? SZ_64K :		\
	 ((bytes) >= SZ_4K)  ? SZ_4K  :	0)

#define bytes_to_iopgsz(bytes)				\
	(((bytes) == SZ_16M) ? MMU_CAM_PGSZ_16M :	\
	 ((bytes) == SZ_1M)  ? MMU_CAM_PGSZ_1M  :	\
	 ((bytes) == SZ_64K) ? MMU_CAM_PGSZ_64K :	\
	 ((bytes) == SZ_4K)  ? MMU_CAM_PGSZ_4K  : -1)

#define iopgsz_to_bytes(iopgsz)				\
	(((iopgsz) == MMU_CAM_PGSZ_16M)	? SZ_16M :	\
	 ((iopgsz) == MMU_CAM_PGSZ_1M)	? SZ_1M  :	\
	 ((iopgsz) == MMU_CAM_PGSZ_64K)	? SZ_64K :	\
	 ((iopgsz) == MMU_CAM_PGSZ_4K)	? SZ_4K  : 0)

#define iopgsz_ok(bytes) (bytes_to_iopgsz(bytes) >= 0)

/*
 * global functions
 */
#ifdef CONFIG_OMAP_IOMMU_DEBUG
extern ssize_t
omap_iommu_dump_ctx(struct omap_iommu *obj, char *buf, ssize_t len);
extern size_t
omap_dump_tlb_entries(struct omap_iommu *obj, char *buf, ssize_t len);

void omap_iommu_debugfs_init(void);
void omap_iommu_debugfs_exit(void);

void omap_iommu_debugfs_add(struct omap_iommu *obj);
void omap_iommu_debugfs_remove(struct omap_iommu *obj);
#else
static inline void omap_iommu_debugfs_init(void) { }
static inline void omap_iommu_debugfs_exit(void) { }

static inline void omap_iommu_debugfs_add(struct omap_iommu *obj) { }
static inline void omap_iommu_debugfs_remove(struct omap_iommu *obj) { }
#endif

/*
 * register accessors
 */
static inline u32 iommu_read_reg(struct omap_iommu *obj, size_t offs)
{
	return __raw_readl(obj->regbase + offs);
}

static inline void iommu_write_reg(struct omap_iommu *obj, u32 val, size_t offs)
{
	__raw_writel(val, obj->regbase + offs);
}

#endif /* _OMAP_IOMMU_H */
