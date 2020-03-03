#include <linux/bootmem.h>
#include <linux/gfp.h>
#include <linux/export.h>
#include <linux/rwlock.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/vmalloc.h>
#include <linux/swiotlb.h>

#include <xen/xen.h>
#include <xen/interface/memory.h>
#include <xen/swiotlb-xen.h>

#include <asm/cacheflush.h>
#include <asm/xen/page.h>
#include <asm/xen/hypercall.h>
#include <asm/xen/interface.h>

struct xen_p2m_entry {
	unsigned long pfn;
	unsigned long mfn;
	unsigned long nr_pages;
	struct rb_node rbnode_phys;
};

static rwlock_t p2m_lock;
struct rb_root phys_to_mach = RB_ROOT;
EXPORT_SYMBOL_GPL(phys_to_mach);

static int xen_add_phys_to_mach_entry(struct xen_p2m_entry *new)
{
	struct rb_node **link = &phys_to_mach.rb_node;
	struct rb_node *parent = NULL;
	struct xen_p2m_entry *entry;
	int rc = 0;

	while (*link) {
		parent = *link;
		entry = rb_entry(parent, struct xen_p2m_entry, rbnode_phys);

		if (new->pfn == entry->pfn)
			goto err_out;

		if (new->pfn < entry->pfn)
			link = &(*link)->rb_left;
		else
			link = &(*link)->rb_right;
	}
	rb_link_node(&new->rbnode_phys, parent, link);
	rb_insert_color(&new->rbnode_phys, &phys_to_mach);
	goto out;

err_out:
	rc = -EINVAL;
	pr_warn("%s: cannot add pfn=%pa -> mfn=%pa: pfn=%pa -> mfn=%pa already exists\n",
			__func__, &new->pfn, &new->mfn, &entry->pfn, &entry->mfn);
out:
	return rc;
}

unsigned long __pfn_to_mfn(unsigned long pfn)
{
	struct rb_node *n = phys_to_mach.rb_node;
	struct xen_p2m_entry *entry;
	unsigned long irqflags;

	read_lock_irqsave(&p2m_lock, irqflags);
	while (n) {
		entry = rb_entry(n, struct xen_p2m_entry, rbnode_phys);
		if (entry->pfn <= pfn &&
				entry->pfn + entry->nr_pages > pfn) {
			read_unlock_irqrestore(&p2m_lock, irqflags);
			return entry->mfn + (pfn - entry->pfn);
		}
		if (pfn < entry->pfn)
			n = n->rb_left;
		else
			n = n->rb_right;
	}
	read_unlock_irqrestore(&p2m_lock, irqflags);

	return INVALID_P2M_ENTRY;
}
EXPORT_SYMBOL_GPL(__pfn_to_mfn);

int set_foreign_p2m_mapping(struct gnttab_map_grant_ref *map_ops,
			    struct gnttab_map_grant_ref *kmap_ops,
			    struct page **pages, unsigned int count)
{
	int i;

	for (i = 0; i < count; i++) {
		if (map_ops[i].status)
			continue;
		set_phys_to_machine(map_ops[i].host_addr >> PAGE_SHIFT,
				    map_ops[i].dev_bus_addr >> PAGE_SHIFT);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(set_foreign_p2m_mapping);

int clear_foreign_p2m_mapping(struct gnttab_unmap_grant_ref *unmap_ops,
			      struct gnttab_unmap_grant_ref *kunmap_ops,
			      struct page **pages, unsigned int count)
{
	int i;

	for (i = 0; i < count; i++) {
		set_phys_to_machine(unmap_ops[i].host_addr >> PAGE_SHIFT,
				    INVALID_P2M_ENTRY);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(clear_foreign_p2m_mapping);

bool __set_phys_to_machine_multi(unsigned long pfn,
		unsigned long mfn, unsigned long nr_pages)
{
	int rc;
	unsigned long irqflags;
	struct xen_p2m_entry *p2m_entry;
	struct rb_node *n = phys_to_mach.rb_node;

	if (mfn == INVALID_P2M_ENTRY) {
		write_lock_irqsave(&p2m_lock, irqflags);
		while (n) {
			p2m_entry = rb_entry(n, struct xen_p2m_entry, rbnode_phys);
			if (p2m_entry->pfn <= pfn &&
					p2m_entry->pfn + p2m_entry->nr_pages > pfn) {
				rb_erase(&p2m_entry->rbnode_phys, &phys_to_mach);
				write_unlock_irqrestore(&p2m_lock, irqflags);
				kfree(p2m_entry);
				return true;
			}
			if (pfn < p2m_entry->pfn)
				n = n->rb_left;
			else
				n = n->rb_right;
		}
		write_unlock_irqrestore(&p2m_lock, irqflags);
		return true;
	}

	p2m_entry = kzalloc(sizeof(struct xen_p2m_entry), GFP_NOWAIT);
	if (!p2m_entry) {
		pr_warn("cannot allocate xen_p2m_entry\n");
		return false;
	}
	p2m_entry->pfn = pfn;
	p2m_entry->nr_pages = nr_pages;
	p2m_entry->mfn = mfn;

	write_lock_irqsave(&p2m_lock, irqflags);
	if ((rc = xen_add_phys_to_mach_entry(p2m_entry)) < 0) {
		write_unlock_irqrestore(&p2m_lock, irqflags);
		return false;
	}
	write_unlock_irqrestore(&p2m_lock, irqflags);
	return true;
}
EXPORT_SYMBOL_GPL(__set_phys_to_machine_multi);

bool __set_phys_to_machine(unsigned long pfn, unsigned long mfn)
{
	return __set_phys_to_machine_multi(pfn, mfn, 1);
}
EXPORT_SYMBOL_GPL(__set_phys_to_machine);

static int p2m_init(void)
{
	rwlock_init(&p2m_lock);
	return 0;
}
arch_initcall(p2m_init);
