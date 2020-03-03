#ifndef __OF_IOMMU_H
#define __OF_IOMMU_H

#include <linux/device.h>
#include <linux/iommu.h>
#include <linux/of.h>

#ifdef CONFIG_OF_IOMMU

extern int of_get_dma_window(struct device_node *dn, const char *prefix,
			     int index, unsigned long *busno, dma_addr_t *addr,
			     size_t *size);

extern void of_iommu_init(void);
extern struct iommu_ops *of_iommu_configure(struct device *dev,
					struct device_node *master_np);

#else

static inline int of_get_dma_window(struct device_node *dn, const char *prefix,
			    int index, unsigned long *busno, dma_addr_t *addr,
			    size_t *size)
{
	return -EINVAL;
}

static inline void of_iommu_init(void) { }
static inline struct iommu_ops *of_iommu_configure(struct device *dev,
					 struct device_node *master_np)
{
	return NULL;
}

#endif	/* CONFIG_OF_IOMMU */

void of_iommu_set_ops(struct device_node *np, struct iommu_ops *ops);
struct iommu_ops *of_iommu_get_ops(struct device_node *np);

extern struct of_device_id __iommu_of_table;

typedef int (*of_iommu_init_fn)(struct device_node *);

#define IOMMU_OF_DECLARE(name, compat, fn) \
	_OF_DECLARE(iommu, name, compat, fn, of_iommu_init_fn)

#endif /* __OF_IOMMU_H */
