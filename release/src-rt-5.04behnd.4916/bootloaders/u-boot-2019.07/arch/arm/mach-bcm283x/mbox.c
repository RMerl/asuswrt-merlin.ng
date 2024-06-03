// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012 Stephen Warren
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/mbox.h>
#include <phys2bus.h>

#define TIMEOUT 1000 /* ms */

int bcm2835_mbox_call_raw(u32 chan, u32 send, u32 *recv)
{
	struct bcm2835_mbox_regs *regs =
		(struct bcm2835_mbox_regs *)BCM2835_MBOX_PHYSADDR;
	ulong endtime = get_timer(0) + TIMEOUT;
	u32 val;

	debug("time: %lu timeout: %lu\n", get_timer(0), endtime);

	if (send & BCM2835_CHAN_MASK) {
		printf("mbox: Illegal mbox data 0x%08x\n", send);
		return -1;
	}

	/* Drain any stale responses */

	for (;;) {
		val = readl(&regs->status);
		if (val & BCM2835_MBOX_STATUS_RD_EMPTY)
			break;
		if (get_timer(0) >= endtime) {
			printf("mbox: Timeout draining stale responses\n");
			return -1;
		}
		val = readl(&regs->read);
	}

	/* Wait for space to send */

	for (;;) {
		val = readl(&regs->status);
		if (!(val & BCM2835_MBOX_STATUS_WR_FULL))
			break;
		if (get_timer(0) >= endtime) {
			printf("mbox: Timeout waiting for send space\n");
			return -1;
		}
	}

	/* Send the request */

	val = BCM2835_MBOX_PACK(chan, send);
	debug("mbox: TX raw: 0x%08x\n", val);
	writel(val, &regs->write);

	/* Wait for the response */

	for (;;) {
		val = readl(&regs->status);
		if (!(val & BCM2835_MBOX_STATUS_RD_EMPTY))
			break;
		if (get_timer(0) >= endtime) {
			printf("mbox: Timeout waiting for response\n");
			return -1;
		}
	}

	/* Read the response */

	val = readl(&regs->read);
	debug("mbox: RX raw: 0x%08x\n", val);

	/* Validate the response */

	if (BCM2835_MBOX_UNPACK_CHAN(val) != chan) {
		printf("mbox: Response channel mismatch\n");
		return -1;
	}

	*recv = BCM2835_MBOX_UNPACK_DATA(val);

	return 0;
}

#ifdef DEBUG
void dump_buf(struct bcm2835_mbox_hdr *buffer)
{
	u32 *p;
	u32 words;
	int i;

	p = (u32 *)buffer;
	words = buffer->buf_size / 4;
	for (i = 0; i < words; i++)
		printf("    0x%04x: 0x%08x\n", i * 4, p[i]);
}
#endif

int bcm2835_mbox_call_prop(u32 chan, struct bcm2835_mbox_hdr *buffer)
{
	int ret;
	u32 rbuffer;
	struct bcm2835_mbox_tag_hdr *tag;
	int tag_index;

#ifdef DEBUG
	printf("mbox: TX buffer\n");
	dump_buf(buffer);
#endif

	flush_dcache_range((unsigned long)buffer,
			   (unsigned long)((void *)buffer +
			   roundup(buffer->buf_size, ARCH_DMA_MINALIGN)));

	ret = bcm2835_mbox_call_raw(chan,
				    phys_to_bus((unsigned long)buffer),
				    &rbuffer);
	if (ret)
		return ret;

	invalidate_dcache_range((unsigned long)buffer,
				(unsigned long)((void *)buffer +
				roundup(buffer->buf_size, ARCH_DMA_MINALIGN)));

	if (rbuffer != phys_to_bus((unsigned long)buffer)) {
		printf("mbox: Response buffer mismatch\n");
		return -1;
	}

#ifdef DEBUG
	printf("mbox: RX buffer\n");
	dump_buf(buffer);
#endif

	/* Validate overall response status */

	if (buffer->code != BCM2835_MBOX_RESP_CODE_SUCCESS) {
		printf("mbox: Header response code invalid\n");
		return -1;
	}

	/* Validate each tag's response status */

	tag = (void *)(buffer + 1);
	tag_index = 0;
	while (tag->tag) {
		if (!(tag->val_len & BCM2835_MBOX_TAG_VAL_LEN_RESPONSE)) {
			printf("mbox: Tag %d missing val_len response bit\n",
				tag_index);
			return -1;
		}
		/*
		 * Clear the reponse bit so clients can just look right at the
		 * length field without extra processing
		 */
		tag->val_len &= ~BCM2835_MBOX_TAG_VAL_LEN_RESPONSE;
		tag = (void *)(((u8 *)tag) + sizeof(*tag) + tag->val_buf_size);
		tag_index++;
	}

	return 0;
}
