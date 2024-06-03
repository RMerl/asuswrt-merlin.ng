// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Tuomas Tynkkynen <tuomas.tynkkynen@iki.fi>
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 *
 * virtio ring implementation
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <virtio_types.h>
#include <virtio.h>
#include <virtio_ring.h>

int virtqueue_add(struct virtqueue *vq, struct virtio_sg *sgs[],
		  unsigned int out_sgs, unsigned int in_sgs)
{
	struct vring_desc *desc;
	unsigned int total_sg = out_sgs + in_sgs;
	unsigned int i, n, avail, descs_used, uninitialized_var(prev);
	int head;

	WARN_ON(total_sg == 0);

	head = vq->free_head;

	desc = vq->vring.desc;
	i = head;
	descs_used = total_sg;

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

	for (n = 0; n < out_sgs; n++) {
		struct virtio_sg *sg = sgs[n];

		desc[i].flags = cpu_to_virtio16(vq->vdev, VRING_DESC_F_NEXT);
		desc[i].addr = cpu_to_virtio64(vq->vdev, (u64)(size_t)sg->addr);
		desc[i].len = cpu_to_virtio32(vq->vdev, sg->length);

		prev = i;
		i = virtio16_to_cpu(vq->vdev, desc[i].next);
	}
	for (; n < (out_sgs + in_sgs); n++) {
		struct virtio_sg *sg = sgs[n];

		desc[i].flags = cpu_to_virtio16(vq->vdev, VRING_DESC_F_NEXT |
						VRING_DESC_F_WRITE);
		desc[i].addr = cpu_to_virtio64(vq->vdev,
					       (u64)(uintptr_t)sg->addr);
		desc[i].len = cpu_to_virtio32(vq->vdev, sg->length);

		prev = i;
		i = virtio16_to_cpu(vq->vdev, desc[i].next);
	}
	/* Last one doesn't continue */
	desc[prev].flags &= cpu_to_virtio16(vq->vdev, ~VRING_DESC_F_NEXT);

	/* We're using some buffers from the free list. */
	vq->num_free -= descs_used;

	/* Update free pointer */
	vq->free_head = i;

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
	__virtio16 nextflag = cpu_to_virtio16(vq->vdev, VRING_DESC_F_NEXT);

	/* Put back on free list: unmap first-level descriptors and find end */
	i = head;

	while (vq->vring.desc[i].flags & nextflag) {
		i = virtio16_to_cpu(vq->vdev, vq->vring.desc[i].next);
		vq->num_free++;
	}

	vq->vring.desc[i].next = cpu_to_virtio16(vq->vdev, vq->free_head);
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

	return (void *)(uintptr_t)virtio64_to_cpu(vq->vdev,
						  vq->vring.desc[i].addr);
}

static struct virtqueue *__vring_new_virtqueue(unsigned int index,
					       struct vring vring,
					       struct udevice *udev)
{
	unsigned int i;
	struct virtqueue *vq;
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(udev);
	struct udevice *vdev = uc_priv->vdev;

	vq = malloc(sizeof(*vq));
	if (!vq)
		return NULL;

	vq->vdev = vdev;
	vq->index = index;
	vq->num_free = vring.num;
	vq->vring = vring;
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
		vq->vring.desc[i].next = cpu_to_virtio16(vdev, i + 1);

	return vq;
}

struct virtqueue *vring_create_virtqueue(unsigned int index, unsigned int num,
					 unsigned int vring_align,
					 struct udevice *udev)
{
	struct virtqueue *vq;
	void *queue = NULL;
	struct vring vring;

	/* We assume num is a power of 2 */
	if (num & (num - 1)) {
		printf("Bad virtqueue length %u\n", num);
		return NULL;
	}

	/* TODO: allocate each queue chunk individually */
	for (; num && vring_size(num, vring_align) > PAGE_SIZE; num /= 2) {
		queue = memalign(PAGE_SIZE, vring_size(num, vring_align));
		if (queue)
			break;
	}

	if (!num)
		return NULL;

	if (!queue) {
		/* Try to get a single page. You are my only hope! */
		queue = memalign(PAGE_SIZE, vring_size(num, vring_align));
	}
	if (!queue)
		return NULL;

	memset(queue, 0, vring_size(num, vring_align));
	vring_init(&vring, num, queue, vring_align);

	vq = __vring_new_virtqueue(index, vring, udev);
	if (!vq) {
		free(queue);
		return NULL;
	}
	debug("(%s): created vring @ %p for vq @ %p with num %u\n", udev->name,
	      queue, vq, num);

	return vq;
}

void vring_del_virtqueue(struct virtqueue *vq)
{
	free(vq->vring.desc);
	list_del(&vq->list);
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

	printf("Descriptor dump:\n");
	for (i = 0; i < vq->vring.num; i++) {
		printf("\tdesc[%u] = { 0x%llx, len %u, flags %u, next %u }\n",
		       i, vq->vring.desc[i].addr, vq->vring.desc[i].len,
		       vq->vring.desc[i].flags, vq->vring.desc[i].next);
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
