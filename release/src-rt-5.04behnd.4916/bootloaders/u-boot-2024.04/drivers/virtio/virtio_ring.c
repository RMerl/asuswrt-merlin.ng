// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Tuomas Tynkkynen <tuomas.tynkkynen@iki.fi>
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 *
 * virtio ring implementation
 */

#include <bouncebuf.h>
#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <virtio_types.h>
#include <virtio.h>
#include <virtio_ring.h>
#include <linux/bug.h>
#include <linux/compat.h>
#include <linux/kernel.h>

static void *virtio_alloc_pages(struct udevice *vdev, u32 npages)
{
	return memalign(PAGE_SIZE, npages * PAGE_SIZE);
}

static void virtio_free_pages(struct udevice *vdev, void *ptr, u32 npages)
{
	free(ptr);
}

static int __bb_force_page_align(struct bounce_buffer *state)
{
	const ulong align_mask = PAGE_SIZE - 1;

	if ((ulong)state->user_buffer & align_mask)
		return 0;

	if (state->len != state->len_aligned)
		return 0;

	return 1;
}

static unsigned int virtqueue_attach_desc(struct virtqueue *vq, unsigned int i,
					  struct virtio_sg *sg, u16 flags)
{
	struct vring_desc_shadow *desc_shadow = &vq->vring_desc_shadow[i];
	struct vring_desc *desc = &vq->vring.desc[i];
	void *addr;

	if (IS_ENABLED(CONFIG_BOUNCE_BUFFER) && vq->vring.bouncebufs) {
		struct bounce_buffer *bb = &vq->vring.bouncebufs[i];
		unsigned int bbflags;
		int ret;

		if (flags & VRING_DESC_F_WRITE)
			bbflags = GEN_BB_WRITE;
		else
			bbflags = GEN_BB_READ;

		ret = bounce_buffer_start_extalign(bb, sg->addr, sg->length,
						   bbflags, PAGE_SIZE,
						   __bb_force_page_align);
		if (ret) {
			debug("%s: failed to allocate bounce buffer (length 0x%zx)\n",
			      vq->vdev->name, sg->length);
		}

		addr = bb->bounce_buffer;
	} else {
		addr = sg->addr;
	}

	/* Update the shadow descriptor. */
	desc_shadow->addr = (u64)(uintptr_t)addr;
	desc_shadow->len = sg->length;
	desc_shadow->flags = flags;

	/* Update the shared descriptor to match the shadow. */
	desc->addr = cpu_to_virtio64(vq->vdev, desc_shadow->addr);
	desc->len = cpu_to_virtio32(vq->vdev, desc_shadow->len);
	desc->flags = cpu_to_virtio16(vq->vdev, desc_shadow->flags);
	desc->next = cpu_to_virtio16(vq->vdev, desc_shadow->next);

	return desc_shadow->next;
}

static void virtqueue_detach_desc(struct virtqueue *vq, unsigned int idx)
{
	struct vring_desc *desc = &vq->vring.desc[idx];
	struct bounce_buffer *bb;

	if (!IS_ENABLED(CONFIG_BOUNCE_BUFFER) || !vq->vring.bouncebufs)
		return;

	bb = &vq->vring.bouncebufs[idx];
	bounce_buffer_stop(bb);
	desc->addr = cpu_to_virtio64(vq->vdev, (u64)(uintptr_t)bb->user_buffer);
}

int virtqueue_add(struct virtqueue *vq, struct virtio_sg *sgs[],
		  unsigned int out_sgs, unsigned int in_sgs)
{
	struct vring_desc *desc;
	unsigned int descs_used = out_sgs + in_sgs;
	unsigned int i, n, avail, uninitialized_var(prev);
	int head;

	WARN_ON(descs_used == 0);

	head = vq->free_head;

	desc = vq->vring.desc;
	i = head;

	if (vq->num_free < descs_used) {
		debug("Can't add buf len %i - avail = %i\n",
		      descs_used, vq->num_free);
		/*
		 * FIXME: for historical reasons, we force a notify here if
		 * there are outgoing parts to the buffer.  Presumably the
		 * host should service the ring ASAP.
		 */
		if (out_sgs)
			virtio_notify(vq->vdev, vq);
		return -ENOSPC;
	}

	for (n = 0; n < descs_used; n++) {
		u16 flags = VRING_DESC_F_NEXT;

		if (n >= out_sgs)
			flags |= VRING_DESC_F_WRITE;
		prev = i;
		i = virtqueue_attach_desc(vq, i, sgs[n], flags);
	}
	/* Last one doesn't continue */
	vq->vring_desc_shadow[prev].flags &= ~VRING_DESC_F_NEXT;
	desc[prev].flags = cpu_to_virtio16(vq->vdev, vq->vring_desc_shadow[prev].flags);

	/* We're using some buffers from the free list. */
	vq->num_free -= descs_used;

	/* Update free pointer */
	vq->free_head = i;

	/* Mark the descriptor as the head of a chain. */
	vq->vring_desc_shadow[head].chain_head = true;

	/*
	 * Put entry in available array (but don't update avail->idx
	 * until they do sync).
	 */
	avail = vq->avail_idx_shadow & (vq->vring.num - 1);
	vq->vring.avail->ring[avail] = cpu_to_virtio16(vq->vdev, head);

	/*
	 * Descriptors and available array need to be set before we expose the
	 * new available array entries.
	 */
	virtio_wmb();
	vq->avail_idx_shadow++;
	vq->vring.avail->idx = cpu_to_virtio16(vq->vdev, vq->avail_idx_shadow);
	vq->num_added++;

	/*
	 * This is very unlikely, but theoretically possible.
	 * Kick just in case.
	 */
	if (unlikely(vq->num_added == (1 << 16) - 1))
		virtqueue_kick(vq);

	return 0;
}

static bool virtqueue_kick_prepare(struct virtqueue *vq)
{
	u16 new, old;
	bool needs_kick;

	/*
	 * We need to expose available array entries before checking
	 * avail event.
	 */
	virtio_mb();

	old = vq->avail_idx_shadow - vq->num_added;
	new = vq->avail_idx_shadow;
	vq->num_added = 0;

	if (vq->event) {
		needs_kick = vring_need_event(virtio16_to_cpu(vq->vdev,
				vring_avail_event(&vq->vring)), new, old);
	} else {
		needs_kick = !(vq->vring.used->flags & cpu_to_virtio16(vq->vdev,
				VRING_USED_F_NO_NOTIFY));
	}

	return needs_kick;
}

void virtqueue_kick(struct virtqueue *vq)
{
	if (virtqueue_kick_prepare(vq))
		virtio_notify(vq->vdev, vq);
}

static void detach_buf(struct virtqueue *vq, unsigned int head)
{
	unsigned int i;

	/* Unmark the descriptor as the head of a chain. */
	vq->vring_desc_shadow[head].chain_head = false;

	/* Put back on free list: unmap first-level descriptors and find end */
	i = head;

	while (vq->vring_desc_shadow[i].flags & VRING_DESC_F_NEXT) {
		virtqueue_detach_desc(vq, i);
		i = vq->vring_desc_shadow[i].next;
		vq->num_free++;
	}

	virtqueue_detach_desc(vq, i);
	vq->vring_desc_shadow[i].next = vq->free_head;
	vq->free_head = head;

	/* Plus final descriptor */
	vq->num_free++;
}

static inline bool more_used(const struct virtqueue *vq)
{
	return vq->last_used_idx != virtio16_to_cpu(vq->vdev,
			vq->vring.used->idx);
}

void *virtqueue_get_buf(struct virtqueue *vq, unsigned int *len)
{
	unsigned int i;
	u16 last_used;

	if (!more_used(vq)) {
		debug("(%s.%d): No more buffers in queue\n",
		      vq->vdev->name, vq->index);
		return NULL;
	}

	/* Only get used array entries after they have been exposed by host */
	virtio_rmb();

	last_used = (vq->last_used_idx & (vq->vring.num - 1));
	i = virtio32_to_cpu(vq->vdev, vq->vring.used->ring[last_used].id);
	if (len) {
		*len = virtio32_to_cpu(vq->vdev,
				       vq->vring.used->ring[last_used].len);
		debug("(%s.%d): last used idx %u with len %u\n",
		      vq->vdev->name, vq->index, i, *len);
	}

	if (unlikely(i >= vq->vring.num)) {
		printf("(%s.%d): id %u out of range\n",
		       vq->vdev->name, vq->index, i);
		return NULL;
	}

	if (unlikely(!vq->vring_desc_shadow[i].chain_head)) {
		printf("(%s.%d): id %u is not a head\n",
		       vq->vdev->name, vq->index, i);
		return NULL;
	}

	detach_buf(vq, i);
	vq->last_used_idx++;
	/*
	 * If we expect an interrupt for the next entry, tell host
	 * by writing event index and flush out the write before
	 * the read in the next get_buf call.
	 */
	if (!(vq->avail_flags_shadow & VRING_AVAIL_F_NO_INTERRUPT))
		virtio_store_mb(&vring_used_event(&vq->vring),
				cpu_to_virtio16(vq->vdev, vq->last_used_idx));

	return (void *)(uintptr_t)vq->vring_desc_shadow[i].addr;
}

static struct virtqueue *__vring_new_virtqueue(unsigned int index,
					       struct vring vring,
					       struct udevice *udev)
{
	unsigned int i;
	struct virtqueue *vq;
	struct vring_desc_shadow *vring_desc_shadow;
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(udev);
	struct udevice *vdev = uc_priv->vdev;

	vq = malloc(sizeof(*vq));
	if (!vq)
		return NULL;

	vring_desc_shadow = calloc(vring.num, sizeof(struct vring_desc_shadow));
	if (!vring_desc_shadow) {
		free(vq);
		return NULL;
	}

	vq->vdev = vdev;
	vq->index = index;
	vq->num_free = vring.num;
	vq->vring = vring;
	vq->vring_desc_shadow = vring_desc_shadow;
	vq->last_used_idx = 0;
	vq->avail_flags_shadow = 0;
	vq->avail_idx_shadow = 0;
	vq->num_added = 0;
	list_add_tail(&vq->list, &uc_priv->vqs);

	vq->event = virtio_has_feature(vdev, VIRTIO_RING_F_EVENT_IDX);

	/* Tell other side not to bother us */
	vq->avail_flags_shadow |= VRING_AVAIL_F_NO_INTERRUPT;
	if (!vq->event)
		vq->vring.avail->flags = cpu_to_virtio16(vdev,
				vq->avail_flags_shadow);

	/* Put everything in free lists */
	vq->free_head = 0;
	for (i = 0; i < vring.num - 1; i++)
		vq->vring_desc_shadow[i].next = i + 1;

	return vq;
}

struct virtqueue *vring_create_virtqueue(unsigned int index, unsigned int num,
					 unsigned int vring_align,
					 struct udevice *udev)
{
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(udev);
	struct udevice *vdev = uc_priv->vdev;
	struct virtqueue *vq;
	void *queue = NULL;
	struct bounce_buffer *bbs = NULL;
	struct vring vring;

	/* We assume num is a power of 2 */
	if (num & (num - 1)) {
		printf("Bad virtqueue length %u\n", num);
		return NULL;
	}

	/* TODO: allocate each queue chunk individually */
	for (; num && vring_size(num, vring_align) > PAGE_SIZE; num /= 2) {
		size_t sz = vring_size(num, vring_align);

		queue = virtio_alloc_pages(vdev, DIV_ROUND_UP(sz, PAGE_SIZE));
		if (queue)
			break;
	}

	if (!num)
		return NULL;

	if (!queue) {
		/* Try to get a single page. You are my only hope! */
		queue = virtio_alloc_pages(vdev, 1);
	}
	if (!queue)
		return NULL;

	memset(queue, 0, vring_size(num, vring_align));

	if (virtio_has_feature(vdev, VIRTIO_F_IOMMU_PLATFORM)) {
		bbs = calloc(num, sizeof(*bbs));
		if (!bbs)
			goto err_free_queue;
	}

	vring_init(&vring, num, queue, vring_align, bbs);

	vq = __vring_new_virtqueue(index, vring, udev);
	if (!vq)
		goto err_free_bbs;

	debug("(%s): created vring @ %p for vq @ %p with num %u\n", udev->name,
	      queue, vq, num);

	return vq;

err_free_bbs:
	free(bbs);
err_free_queue:
	virtio_free_pages(vdev, queue, DIV_ROUND_UP(vring.size, PAGE_SIZE));
	return NULL;
}

void vring_del_virtqueue(struct virtqueue *vq)
{
	virtio_free_pages(vq->vdev, vq->vring.desc,
			  DIV_ROUND_UP(vq->vring.size, PAGE_SIZE));
	free(vq->vring_desc_shadow);
	list_del(&vq->list);
	free(vq->vring.bouncebufs);
	free(vq);
}

unsigned int virtqueue_get_vring_size(struct virtqueue *vq)
{
	return vq->vring.num;
}

ulong virtqueue_get_desc_addr(struct virtqueue *vq)
{
	return (ulong)vq->vring.desc;
}

ulong virtqueue_get_avail_addr(struct virtqueue *vq)
{
	return (ulong)vq->vring.desc +
	       ((char *)vq->vring.avail - (char *)vq->vring.desc);
}

ulong virtqueue_get_used_addr(struct virtqueue *vq)
{
	return (ulong)vq->vring.desc +
	       ((char *)vq->vring.used - (char *)vq->vring.desc);
}

bool virtqueue_poll(struct virtqueue *vq, u16 last_used_idx)
{
	virtio_mb();

	return last_used_idx != virtio16_to_cpu(vq->vdev, vq->vring.used->idx);
}

void virtqueue_dump(struct virtqueue *vq)
{
	unsigned int i;

	printf("virtqueue %p for dev %s:\n", vq, vq->vdev->name);
	printf("\tindex %u, phys addr %p num %u\n",
	       vq->index, vq->vring.desc, vq->vring.num);
	printf("\tfree_head %u, num_added %u, num_free %u\n",
	       vq->free_head, vq->num_added, vq->num_free);
	printf("\tlast_used_idx %u, avail_flags_shadow %u, avail_idx_shadow %u\n",
	       vq->last_used_idx, vq->avail_flags_shadow, vq->avail_idx_shadow);

	printf("Shadow descriptor dump:\n");
	for (i = 0; i < vq->vring.num; i++) {
		struct vring_desc_shadow *desc = &vq->vring_desc_shadow[i];

		printf("\tdesc_shadow[%u] = { 0x%llx, len %u, flags %u, next %u }\n",
		       i, desc->addr, desc->len, desc->flags, desc->next);
	}

	printf("Avail ring dump:\n");
	printf("\tflags %u, idx %u\n",
	       vq->vring.avail->flags, vq->vring.avail->idx);
	for (i = 0; i < vq->vring.num; i++) {
		printf("\tavail[%u] = %u\n",
		       i, vq->vring.avail->ring[i]);
	}

	printf("Used ring dump:\n");
	printf("\tflags %u, idx %u\n",
	       vq->vring.used->flags, vq->vring.used->idx);
	for (i = 0; i < vq->vring.num; i++) {
		printf("\tused[%u] = { %u, %u }\n", i,
		       vq->vring.used->ring[i].id, vq->vring.used->ring[i].len);
	}
}
