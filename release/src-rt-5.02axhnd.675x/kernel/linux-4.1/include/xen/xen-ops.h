#ifndef INCLUDE_XEN_OPS_H
#define INCLUDE_XEN_OPS_H

#include <linux/percpu.h>
#include <linux/notifier.h>
#include <linux/efi.h>
#include <asm/xen/interface.h>

DECLARE_PER_CPU(struct vcpu_info *, xen_vcpu);

void xen_arch_pre_suspend(void);
void xen_arch_post_suspend(int suspend_cancelled);

void xen_timer_resume(void);
void xen_arch_resume(void);
void xen_arch_suspend(void);

void xen_resume_notifier_register(struct notifier_block *nb);
void xen_resume_notifier_unregister(struct notifier_block *nb);

int xen_setup_shutdown_event(void);

extern unsigned long *xen_contiguous_bitmap;
int xen_create_contiguous_region(phys_addr_t pstart, unsigned int order,
				unsigned int address_bits,
				dma_addr_t *dma_handle);

void xen_destroy_contiguous_region(phys_addr_t pstart, unsigned int order);

struct vm_area_struct;

/*
 * xen_remap_domain_mfn_array() - map an array of foreign frames
 * @vma:     VMA to map the pages into
 * @addr:    Address at which to map the pages
 * @gfn:     Array of GFNs to map
 * @nr:      Number entries in the GFN array
 * @err_ptr: Returns per-GFN error status.
 * @prot:    page protection mask
 * @domid:   Domain owning the pages
 * @pages:   Array of pages if this domain has an auto-translated physmap
 *
 * @gfn and @err_ptr may point to the same buffer, the GFNs will be
 * overwritten by the error codes after they are mapped.
 *
 * Returns the number of successfully mapped frames, or a -ve error
 * code.
 */
int xen_remap_domain_mfn_array(struct vm_area_struct *vma,
			       unsigned long addr,
			       xen_pfn_t *gfn, int nr,
			       int *err_ptr, pgprot_t prot,
			       unsigned domid,
			       struct page **pages);

/* xen_remap_domain_mfn_range() - map a range of foreign frames
 * @vma:     VMA to map the pages into
 * @addr:    Address at which to map the pages
 * @gfn:     First GFN to map.
 * @nr:      Number frames to map
 * @prot:    page protection mask
 * @domid:   Domain owning the pages
 * @pages:   Array of pages if this domain has an auto-translated physmap
 *
 * Returns the number of successfully mapped frames, or a -ve error
 * code.
 */
int xen_remap_domain_mfn_range(struct vm_area_struct *vma,
			       unsigned long addr,
			       xen_pfn_t gfn, int nr,
			       pgprot_t prot, unsigned domid,
			       struct page **pages);
int xen_unmap_domain_mfn_range(struct vm_area_struct *vma,
			       int numpgs, struct page **pages);
int xen_xlate_remap_gfn_array(struct vm_area_struct *vma,
			      unsigned long addr,
			      xen_pfn_t *gfn, int nr,
			      int *err_ptr, pgprot_t prot,
			      unsigned domid,
			      struct page **pages);
int xen_xlate_unmap_gfn_range(struct vm_area_struct *vma,
			      int nr, struct page **pages);

bool xen_running_on_version_or_later(unsigned int major, unsigned int minor);

#ifdef CONFIG_XEN_EFI
extern efi_system_table_t *xen_efi_probe(void);
#else
static inline efi_system_table_t __init *xen_efi_probe(void)
{
	return NULL;
}
#endif

#ifdef CONFIG_PREEMPT

static inline void xen_preemptible_hcall_begin(void)
{
}

static inline void xen_preemptible_hcall_end(void)
{
}

#else

DECLARE_PER_CPU(bool, xen_in_preemptible_hcall);

static inline void xen_preemptible_hcall_begin(void)
{
	__this_cpu_write(xen_in_preemptible_hcall, true);
}

static inline void xen_preemptible_hcall_end(void)
{
	__this_cpu_write(xen_in_preemptible_hcall, false);
}

#endif /* CONFIG_PREEMPT */

#endif /* INCLUDE_XEN_OPS_H */
