/*
 * Copyright (c) 2005 Topspin Communications.  All rights reserved.
 * Copyright (c) 2005 Mellanox Technologies. All rights reserved.
 * Copyright (c) 2013 Cisco Systems.  All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <linux/sched.h>
#include <linux/hugetlb.h>
#include <linux/dma-attrs.h>
#include <linux/iommu.h>
#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/pci.h>

#include "usnic_log.h"
#include "usnic_uiom.h"
#include "usnic_uiom_interval_tree.h"

static struct workqueue_struct *usnic_uiom_wq;

#define USNIC_UIOM_PAGE_CHUNK						\
	((PAGE_SIZE - offsetof(struct usnic_uiom_chunk, page_list))	/\
	((void *) &((struct usnic_uiom_chunk *) 0)->page_list[1] -	\
	(void *) &((struct usnic_uiom_chunk *) 0)->page_list[0]))

static void usnic_uiom_reg_account(struct work_struct *work)
{
	struct usnic_uiom_reg *umem = container_of(work,
						struct usnic_uiom_reg, work);

	down_write(&umem->mm->mmap_sem);
	umem->mm->locked_vm -= umem->diff;
	up_write(&umem->mm->mmap_sem);
	mmput(umem->mm);
	kfree(umem);
}

static int usnic_uiom_dma_fault(struct iommu_domain *domain,
				struct device *dev,
				unsigned long iova, int flags,
				void *token)
{
	usnic_err("Device %s iommu fault domain 0x%pK va 0x%lx flags 0x%x\n",
		dev_name(dev),
		domain, iova, flags);
	return -ENOSYS;
}

static void usnic_uiom_put_pages(struct list_head *chunk_list, int dirty)
{
	struct usnic_uiom_chunk *chunk, *tmp;
	struct page *page;
	struct scatterlist *sg;
	int i;
	dma_addr_t pa;

	list_for_each_entry_safe(chunk, tmp, chunk_list, list) {
		for_each_sg(chunk->page_list, sg, chunk->nents, i) {
			page = sg_page(sg);
			pa = sg_phys(sg);
			if (dirty)
				set_page_dirty_lock(page);
			put_page(page);
			usnic_dbg("pa: %pa\n", &pa);
		}
		kfree(chunk);
	}
}

static int usnic_uiom_get_pages(unsigned long addr, size_t size, int writable,
				int dmasync, struct list_head *chunk_list)
{
	struct page **page_list;
	struct scatterlist *sg;
	struct usnic_uiom_chunk *chunk;
	unsigned long locked;
	unsigned long lock_limit;
	unsigned long cur_base;
	unsigned long npages;
	int ret;
	int off;
	int i;
	int flags;
	dma_addr_t pa;
	DEFINE_DMA_ATTRS(attrs);

	if (dmasync)
		dma_set_attr(DMA_ATTR_WRITE_BARRIER, &attrs);

	if (!can_do_mlock())
		return -EPERM;

	INIT_LIST_HEAD(chunk_list);

	page_list = (struct page **) __get_free_page(GFP_KERNEL);
	if (!page_list)
		return -ENOMEM;

	npages = PAGE_ALIGN(size + (addr & ~PAGE_MASK)) >> PAGE_SHIFT;

	down_write(&current->mm->mmap_sem);

	locked = npages + current->mm->locked_vm;
	lock_limit = rlimit(RLIMIT_MEMLOCK) >> PAGE_SHIFT;

	if ((locked > lock_limit) && !capable(CAP_IPC_LOCK)) {
		ret = -ENOMEM;
		goto out;
	}

	flags = IOMMU_READ | IOMMU_CACHE;
	flags |= (writable) ? IOMMU_WRITE : 0;
	cur_base = addr & PAGE_MASK;
	ret = 0;

	while (npages) {
		ret = get_user_pages(current, current->mm, cur_base,
					min_t(unsigned long, npages,
					PAGE_SIZE / sizeof(struct page *)),
					1, !writable, page_list, NULL);

		if (ret < 0)
			goto out;

		npages -= ret;
		off = 0;

		while (ret) {
			chunk = kmalloc(sizeof(*chunk) +
					sizeof(struct scatterlist) *
					min_t(int, ret, USNIC_UIOM_PAGE_CHUNK),
					GFP_KERNEL);
			if (!chunk) {
				ret = -ENOMEM;
				goto out;
			}

			chunk->nents = min_t(int, ret, USNIC_UIOM_PAGE_CHUNK);
			sg_init_table(chunk->page_list, chunk->nents);
			for_each_sg(chunk->page_list, sg, chunk->nents, i) {
				sg_set_page(sg, page_list[i + off],
						PAGE_SIZE, 0);
				pa = sg_phys(sg);
				usnic_dbg("va: 0x%lx pa: %pa\n",
						cur_base + i*PAGE_SIZE, &pa);
			}
			cur_base += chunk->nents * PAGE_SIZE;
			ret -= chunk->nents;
			off += chunk->nents;
			list_add_tail(&chunk->list, chunk_list);
		}

		ret = 0;
	}

out:
	if (ret < 0)
		usnic_uiom_put_pages(chunk_list, 0);
	else
		current->mm->locked_vm = locked;

	up_write(&current->mm->mmap_sem);
	free_page((unsigned long) page_list);
	return ret;
}

static void usnic_uiom_unmap_sorted_intervals(struct list_head *intervals,
						struct usnic_uiom_pd *pd)
{
	struct usnic_uiom_interval_node *interval, *tmp;
	long unsigned va, size;

	list_for_each_entry_safe(interval, tmp, intervals, link) {
		va = interval->start << PAGE_SHIFT;
		size = ((interval->last - interval->start) + 1) << PAGE_SHIFT;
		while (size > 0) {
			/* Workaround for RH 970401 */
			usnic_dbg("va 0x%lx size 0x%lx", va, PAGE_SIZE);
			iommu_unmap(pd->domain, va, PAGE_SIZE);
			va += PAGE_SIZE;
			size -= PAGE_SIZE;
		}
	}
}

static void __usnic_uiom_reg_release(struct usnic_uiom_pd *pd,
					struct usnic_uiom_reg *uiomr,
					int dirty)
{
	int npages;
	unsigned long vpn_start, vpn_last;
	struct usnic_uiom_interval_node *interval, *tmp;
	int writable = 0;
	LIST_HEAD(rm_intervals);

	npages = PAGE_ALIGN(uiomr->length + uiomr->offset) >> PAGE_SHIFT;
	vpn_start = (uiomr->va & PAGE_MASK) >> PAGE_SHIFT;
	vpn_last = vpn_start + npages - 1;

	spin_lock(&pd->lock);
	usnic_uiom_remove_interval(&pd->rb_root, vpn_start,
					vpn_last, &rm_intervals);
	usnic_uiom_unmap_sorted_intervals(&rm_intervals, pd);

	list_for_each_entry_safe(interval, tmp, &rm_intervals, link) {
		if (interval->flags & IOMMU_WRITE)
			writable = 1;
		list_del(&interval->link);
		kfree(interval);
	}

	usnic_uiom_put_pages(&uiomr->chunk_list, dirty & writable);
	spin_unlock(&pd->lock);
}

static int usnic_uiom_map_sorted_intervals(struct list_head *intervals,
						struct usnic_uiom_reg *uiomr)
{
	int i, err;
	size_t size;
	struct usnic_uiom_chunk *chunk;
	struct usnic_uiom_interval_node *interval_node;
	dma_addr_t pa;
	dma_addr_t pa_start = 0;
	dma_addr_t pa_end = 0;
	long int va_start = -EINVAL;
	struct usnic_uiom_pd *pd = uiomr->pd;
	long int va = uiomr->va & PAGE_MASK;
	int flags = IOMMU_READ | IOMMU_CACHE;

	flags |= (uiomr->writable) ? IOMMU_WRITE : 0;
	chunk = list_first_entry(&uiomr->chunk_list, struct usnic_uiom_chunk,
									list);
	list_for_each_entry(interval_node, intervals, link) {
iter_chunk:
		for (i = 0; i < chunk->nents; i++, va += PAGE_SIZE) {
			pa = sg_phys(&chunk->page_list[i]);
			if ((va >> PAGE_SHIFT) < interval_node->start)
				continue;

			if ((va >> PAGE_SHIFT) == interval_node->start) {
				/* First page of the interval */
				va_start = va;
				pa_start = pa;
				pa_end = pa;
			}

			WARN_ON(va_start == -EINVAL);

			if ((pa_end + PAGE_SIZE != pa) &&
					(pa != pa_start)) {
				/* PAs are not contiguous */
				size = pa_end - pa_start + PAGE_SIZE;
				usnic_dbg("va 0x%lx pa %pa size 0x%zx flags 0x%x",
					va_start, &pa_start, size, flags);
				err = iommu_map(pd->domain, va_start, pa_start,
							size, flags);
				if (err) {
					usnic_err("Failed to map va 0x%lx pa %pa size 0x%zx with err %d\n",
						va_start, &pa_start, size, err);
					goto err_out;
				}
				va_start = va;
				pa_start = pa;
				pa_end = pa;
			}

			if ((va >> PAGE_SHIFT) == interval_node->last) {
				/* Last page of the interval */
				size = pa - pa_start + PAGE_SIZE;
				usnic_dbg("va 0x%lx pa %pa size 0x%zx flags 0x%x\n",
					va_start, &pa_start, size, flags);
				err = iommu_map(pd->domain, va_start, pa_start,
						size, flags);
				if (err) {
					usnic_err("Failed to map va 0x%lx pa %pa size 0x%zx with err %d\n",
						va_start, &pa_start, size, err);
					goto err_out;
				}
				break;
			}

			if (pa != pa_start)
				pa_end += PAGE_SIZE;
		}

		if (i == chunk->nents) {
			/*
			 * Hit last entry of the chunk,
			 * hence advance to next chunk
			 */
			chunk = list_first_entry(&chunk->list,
							struct usnic_uiom_chunk,
							list);
			goto iter_chunk;
		}
	}

	return 0;

err_out:
	usnic_uiom_unmap_sorted_intervals(intervals, pd);
	return err;
}

struct usnic_uiom_reg *usnic_uiom_reg_get(struct usnic_uiom_pd *pd,
						unsigned long addr, size_t size,
						int writable, int dmasync)
{
	struct usnic_uiom_reg *uiomr;
	unsigned long va_base, vpn_start, vpn_last;
	unsigned long npages;
	int offset, err;
	LIST_HEAD(sorted_diff_intervals);

	/*
	 * Intel IOMMU map throws an error if a translation entry is
	 * changed from read to write.  This module may not unmap
	 * and then remap the entry after fixing the permission
	 * b/c this open up a small windows where hw DMA may page fault
	 * Hence, make all entries to be writable.
	 */
	writable = 1;

	va_base = addr & PAGE_MASK;
	offset = addr & ~PAGE_MASK;
	npages = PAGE_ALIGN(size + offset) >> PAGE_SHIFT;
	vpn_start = (addr & PAGE_MASK) >> PAGE_SHIFT;
	vpn_last = vpn_start + npages - 1;

	uiomr = kmalloc(sizeof(*uiomr), GFP_KERNEL);
	if (!uiomr)
		return ERR_PTR(-ENOMEM);

	uiomr->va = va_base;
	uiomr->offset = offset;
	uiomr->length = size;
	uiomr->writable = writable;
	uiomr->pd = pd;

	err = usnic_uiom_get_pages(addr, size, writable, dmasync,
					&uiomr->chunk_list);
	if (err) {
		usnic_err("Failed get_pages vpn [0x%lx,0x%lx] err %d\n",
				vpn_start, vpn_last, err);
		goto out_free_uiomr;
	}

	spin_lock(&pd->lock);
	err = usnic_uiom_get_intervals_diff(vpn_start, vpn_last,
						(writable) ? IOMMU_WRITE : 0,
						IOMMU_WRITE,
						&pd->rb_root,
						&sorted_diff_intervals);
	if (err) {
		usnic_err("Failed disjoint interval vpn [0x%lx,0x%lx] err %d\n",
						vpn_start, vpn_last, err);
		goto out_put_pages;
	}

	err = usnic_uiom_map_sorted_intervals(&sorted_diff_intervals, uiomr);
	if (err) {
		usnic_err("Failed map interval vpn [0x%lx,0x%lx] err %d\n",
						vpn_start, vpn_last, err);
		goto out_put_intervals;

	}

	err = usnic_uiom_insert_interval(&pd->rb_root, vpn_start, vpn_last,
					(writable) ? IOMMU_WRITE : 0);
	if (err) {
		usnic_err("Failed insert interval vpn [0x%lx,0x%lx] err %d\n",
						vpn_start, vpn_last, err);
		goto out_unmap_intervals;
	}

	usnic_uiom_put_interval_set(&sorted_diff_intervals);
	spin_unlock(&pd->lock);

	return uiomr;

out_unmap_intervals:
	usnic_uiom_unmap_sorted_intervals(&sorted_diff_intervals, pd);
out_put_intervals:
	usnic_uiom_put_interval_set(&sorted_diff_intervals);
out_put_pages:
	usnic_uiom_put_pages(&uiomr->chunk_list, 0);
	spin_unlock(&pd->lock);
out_free_uiomr:
	kfree(uiomr);
	return ERR_PTR(err);
}

void usnic_uiom_reg_release(struct usnic_uiom_reg *uiomr, int closing)
{
	struct mm_struct *mm;
	unsigned long diff;

	__usnic_uiom_reg_release(uiomr->pd, uiomr, 1);

	mm = get_task_mm(current);
	if (!mm) {
		kfree(uiomr);
		return;
	}

	diff = PAGE_ALIGN(uiomr->length + uiomr->offset) >> PAGE_SHIFT;

	/*
	 * We may be called with the mm's mmap_sem already held.  This
	 * can happen when a userspace munmap() is the call that drops
	 * the last reference to our file and calls our release
	 * method.  If there are memory regions to destroy, we'll end
	 * up here and not be able to take the mmap_sem.  In that case
	 * we defer the vm_locked accounting to the system workqueue.
	 */
	if (closing) {
		if (!down_write_trylock(&mm->mmap_sem)) {
			INIT_WORK(&uiomr->work, usnic_uiom_reg_account);
			uiomr->mm = mm;
			uiomr->diff = diff;

			queue_work(usnic_uiom_wq, &uiomr->work);
			return;
		}
	} else
		down_write(&mm->mmap_sem);

	current->mm->locked_vm -= diff;
	up_write(&mm->mmap_sem);
	mmput(mm);
	kfree(uiomr);
}

struct usnic_uiom_pd *usnic_uiom_alloc_pd(void)
{
	struct usnic_uiom_pd *pd;
	void *domain;

	pd = kzalloc(sizeof(*pd), GFP_KERNEL);
	if (!pd)
		return ERR_PTR(-ENOMEM);

	pd->domain = domain = iommu_domain_alloc(&pci_bus_type);
	if (IS_ERR_OR_NULL(domain)) {
		usnic_err("Failed to allocate IOMMU domain with err %ld\n",
				PTR_ERR(pd->domain));
		kfree(pd);
		return ERR_PTR(domain ? PTR_ERR(domain) : -ENOMEM);
	}

	iommu_set_fault_handler(pd->domain, usnic_uiom_dma_fault, NULL);

	spin_lock_init(&pd->lock);
	INIT_LIST_HEAD(&pd->devs);

	return pd;
}

void usnic_uiom_dealloc_pd(struct usnic_uiom_pd *pd)
{
	iommu_domain_free(pd->domain);
	kfree(pd);
}

int usnic_uiom_attach_dev_to_pd(struct usnic_uiom_pd *pd, struct device *dev)
{
	struct usnic_uiom_dev *uiom_dev;
	int err;

	uiom_dev = kzalloc(sizeof(*uiom_dev), GFP_ATOMIC);
	if (!uiom_dev)
		return -ENOMEM;
	uiom_dev->dev = dev;

	err = iommu_attach_device(pd->domain, dev);
	if (err)
		goto out_free_dev;

	if (!iommu_capable(dev->bus, IOMMU_CAP_CACHE_COHERENCY)) {
		usnic_err("IOMMU of %s does not support cache coherency\n",
				dev_name(dev));
		err = -EINVAL;
		goto out_detach_device;
	}

	spin_lock(&pd->lock);
	list_add_tail(&uiom_dev->link, &pd->devs);
	pd->dev_cnt++;
	spin_unlock(&pd->lock);

	return 0;

out_detach_device:
	iommu_detach_device(pd->domain, dev);
out_free_dev:
	kfree(uiom_dev);
	return err;
}

void usnic_uiom_detach_dev_from_pd(struct usnic_uiom_pd *pd, struct device *dev)
{
	struct usnic_uiom_dev *uiom_dev;
	int found = 0;

	spin_lock(&pd->lock);
	list_for_each_entry(uiom_dev, &pd->devs, link) {
		if (uiom_dev->dev == dev) {
			found = 1;
			break;
		}
	}

	if (!found) {
		usnic_err("Unable to free dev %s - not found\n",
				dev_name(dev));
		spin_unlock(&pd->lock);
		return;
	}

	list_del(&uiom_dev->link);
	pd->dev_cnt--;
	spin_unlock(&pd->lock);

	return iommu_detach_device(pd->domain, dev);
}

struct device **usnic_uiom_get_dev_list(struct usnic_uiom_pd *pd)
{
	struct usnic_uiom_dev *uiom_dev;
	struct device **devs;
	int i = 0;

	spin_lock(&pd->lock);
	devs = kcalloc(pd->dev_cnt + 1, sizeof(*devs), GFP_ATOMIC);
	if (!devs) {
		devs = ERR_PTR(-ENOMEM);
		goto out;
	}

	list_for_each_entry(uiom_dev, &pd->devs, link) {
		devs[i++] = uiom_dev->dev;
	}
out:
	spin_unlock(&pd->lock);
	return devs;
}

void usnic_uiom_free_dev_list(struct device **devs)
{
	kfree(devs);
}

int usnic_uiom_init(char *drv_name)
{
	if (!iommu_present(&pci_bus_type)) {
		usnic_err("IOMMU required but not present or enabled.  USNIC QPs will not function w/o enabling IOMMU\n");
		return -EPERM;
	}

	usnic_uiom_wq = create_workqueue(drv_name);
	if (!usnic_uiom_wq) {
		usnic_err("Unable to alloc wq for drv %s\n", drv_name);
		return -ENOMEM;
	}

	return 0;
}

void usnic_uiom_fini(void)
{
	flush_workqueue(usnic_uiom_wq);
	destroy_workqueue(usnic_uiom_wq);
}
