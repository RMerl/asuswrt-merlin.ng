/*
 * dice_transaction.c - a part of driver for Dice based devices
 *
 * Copyright (c) Clemens Ladisch
 * Copyright (c) 2014 Takashi Sakamoto
 *
 * Licensed under the terms of the GNU General Public License, version 2.
 */

#include "dice.h"

#define NOTIFICATION_TIMEOUT_MS	100

static u64 get_subaddr(struct snd_dice *dice, enum snd_dice_addr_type type,
		       u64 offset)
{
	switch (type) {
	case SND_DICE_ADDR_TYPE_TX:
		offset += dice->tx_offset;
		break;
	case SND_DICE_ADDR_TYPE_RX:
		offset += dice->rx_offset;
		break;
	case SND_DICE_ADDR_TYPE_SYNC:
		offset += dice->sync_offset;
		break;
	case SND_DICE_ADDR_TYPE_RSRV:
		offset += dice->rsrv_offset;
		break;
	case SND_DICE_ADDR_TYPE_GLOBAL:
	default:
		offset += dice->global_offset;
		break;
	}
	offset += DICE_PRIVATE_SPACE;
	return offset;
}

int snd_dice_transaction_write(struct snd_dice *dice,
			       enum snd_dice_addr_type type,
			       unsigned int offset, void *buf, unsigned int len)
{
	return snd_fw_transaction(dice->unit,
				  (len == 4) ? TCODE_WRITE_QUADLET_REQUEST :
					       TCODE_WRITE_BLOCK_REQUEST,
				  get_subaddr(dice, type, offset), buf, len, 0);
}

int snd_dice_transaction_read(struct snd_dice *dice,
			      enum snd_dice_addr_type type, unsigned int offset,
			      void *buf, unsigned int len)
{
	return snd_fw_transaction(dice->unit,
				  (len == 4) ? TCODE_READ_QUADLET_REQUEST :
					       TCODE_READ_BLOCK_REQUEST,
				  get_subaddr(dice, type, offset), buf, len, 0);
}

static unsigned int get_clock_info(struct snd_dice *dice, __be32 *info)
{
	return snd_dice_transaction_read_global(dice, GLOBAL_CLOCK_SELECT,
						info, 4);
}

static int set_clock_info(struct snd_dice *dice,
			  unsigned int rate, unsigned int source)
{
	unsigned int retries = 3;
	unsigned int i;
	__be32 info;
	u32 mask;
	u32 clock;
	int err;
retry:
	err = get_clock_info(dice, &info);
	if (err < 0)
		goto end;

	clock = be32_to_cpu(info);
	if (source != UINT_MAX) {
		mask = CLOCK_SOURCE_MASK;
		clock &= ~mask;
		clock |= source;
	}
	if (rate != UINT_MAX) {
		for (i = 0; i < ARRAY_SIZE(snd_dice_rates); i++) {
			if (snd_dice_rates[i] == rate)
				break;
		}
		if (i == ARRAY_SIZE(snd_dice_rates)) {
			err = -EINVAL;
			goto end;
		}

		mask = CLOCK_RATE_MASK;
		clock &= ~mask;
		clock |= i << CLOCK_RATE_SHIFT;
	}
	info = cpu_to_be32(clock);

	if (completion_done(&dice->clock_accepted))
		reinit_completion(&dice->clock_accepted);

	err = snd_dice_transaction_write_global(dice, GLOBAL_CLOCK_SELECT,
						&info, 4);
	if (err < 0)
		goto end;

	/* Timeout means it's invalid request, probably bus reset occurred. */
	if (wait_for_completion_timeout(&dice->clock_accepted,
			msecs_to_jiffies(NOTIFICATION_TIMEOUT_MS)) == 0) {
		if (retries-- == 0) {
			err = -ETIMEDOUT;
			goto end;
		}

		err = snd_dice_transaction_reinit(dice);
		if (err < 0)
			goto end;

		msleep(500);	/* arbitrary */
		goto retry;
	}
end:
	return err;
}

int snd_dice_transaction_get_clock_source(struct snd_dice *dice,
					  unsigned int *source)
{
	__be32 info;
	int err;

	err = get_clock_info(dice, &info);
	if (err >= 0)
		*source = be32_to_cpu(info) & CLOCK_SOURCE_MASK;

	return err;
}

int snd_dice_transaction_get_rate(struct snd_dice *dice, unsigned int *rate)
{
	__be32 info;
	unsigned int index;
	int err;

	err = get_clock_info(dice, &info);
	if (err < 0)
		goto end;

	index = (be32_to_cpu(info) & CLOCK_RATE_MASK) >> CLOCK_RATE_SHIFT;
	if (index >= SND_DICE_RATES_COUNT) {
		err = -ENOSYS;
		goto end;
	}

	*rate = snd_dice_rates[index];
end:
	return err;
}
int snd_dice_transaction_set_rate(struct snd_dice *dice, unsigned int rate)
{
	return set_clock_info(dice, rate, UINT_MAX);
}

int snd_dice_transaction_set_enable(struct snd_dice *dice)
{
	__be32 value;
	int err = 0;

	if (dice->global_enabled)
		goto end;

	value = cpu_to_be32(1);
	err = snd_fw_transaction(dice->unit, TCODE_WRITE_QUADLET_REQUEST,
				 get_subaddr(dice, SND_DICE_ADDR_TYPE_GLOBAL,
					     GLOBAL_ENABLE),
				 &value, 4,
				 FW_FIXED_GENERATION | dice->owner_generation);
	if (err < 0)
		goto end;

	dice->global_enabled = true;
end:
	return err;
}

void snd_dice_transaction_clear_enable(struct snd_dice *dice)
{
	__be32 value;

	value = 0;
	snd_fw_transaction(dice->unit, TCODE_WRITE_QUADLET_REQUEST,
			   get_subaddr(dice, SND_DICE_ADDR_TYPE_GLOBAL,
				       GLOBAL_ENABLE),
			   &value, 4, FW_QUIET |
			   FW_FIXED_GENERATION | dice->owner_generation);

	dice->global_enabled = false;
}

static void dice_notification(struct fw_card *card, struct fw_request *request,
			      int tcode, int destination, int source,
			      int generation, unsigned long long offset,
			      void *data, size_t length, void *callback_data)
{
	struct snd_dice *dice = callback_data;
	u32 bits;
	unsigned long flags;

	if (tcode != TCODE_WRITE_QUADLET_REQUEST) {
		fw_send_response(card, request, RCODE_TYPE_ERROR);
		return;
	}
	if ((offset & 3) != 0) {
		fw_send_response(card, request, RCODE_ADDRESS_ERROR);
		return;
	}

	bits = be32_to_cpup(data);

	spin_lock_irqsave(&dice->lock, flags);
	dice->notification_bits |= bits;
	spin_unlock_irqrestore(&dice->lock, flags);

	fw_send_response(card, request, RCODE_COMPLETE);

	if (bits & NOTIFY_CLOCK_ACCEPTED)
		complete(&dice->clock_accepted);
	wake_up(&dice->hwdep_wait);
}

static int register_notification_address(struct snd_dice *dice, bool retry)
{
	struct fw_device *device = fw_parent_device(dice->unit);
	__be64 *buffer;
	unsigned int retries;
	int err;

	retries = (retry) ? 3 : 0;

	buffer = kmalloc(2 * 8, GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;

	for (;;) {
		buffer[0] = cpu_to_be64(OWNER_NO_OWNER);
		buffer[1] = cpu_to_be64(
			((u64)device->card->node_id << OWNER_NODE_SHIFT) |
			dice->notification_handler.offset);

		dice->owner_generation = device->generation;
		smp_rmb(); /* node_id vs. generation */
		err = snd_fw_transaction(dice->unit, TCODE_LOCK_COMPARE_SWAP,
					 get_subaddr(dice,
						     SND_DICE_ADDR_TYPE_GLOBAL,
						     GLOBAL_OWNER),
					 buffer, 2 * 8,
					 FW_FIXED_GENERATION |
							dice->owner_generation);
		if (err == 0) {
			/* success */
			if (buffer[0] == cpu_to_be64(OWNER_NO_OWNER))
				break;
			/* The address seems to be already registered. */
			if (buffer[0] == buffer[1])
				break;

			dev_err(&dice->unit->device,
				"device is already in use\n");
			err = -EBUSY;
		}
		if (err != -EAGAIN || retries-- > 0)
			break;

		msleep(20);
	}

	kfree(buffer);

	if (err < 0)
		dice->owner_generation = -1;

	return err;
}

static void unregister_notification_address(struct snd_dice *dice)
{
	struct fw_device *device = fw_parent_device(dice->unit);
	__be64 *buffer;

	buffer = kmalloc(2 * 8, GFP_KERNEL);
	if (buffer == NULL)
		return;

	buffer[0] = cpu_to_be64(
		((u64)device->card->node_id << OWNER_NODE_SHIFT) |
		dice->notification_handler.offset);
	buffer[1] = cpu_to_be64(OWNER_NO_OWNER);
	snd_fw_transaction(dice->unit, TCODE_LOCK_COMPARE_SWAP,
			   get_subaddr(dice, SND_DICE_ADDR_TYPE_GLOBAL,
				       GLOBAL_OWNER),
			   buffer, 2 * 8, FW_QUIET |
			   FW_FIXED_GENERATION | dice->owner_generation);

	kfree(buffer);

	dice->owner_generation = -1;
}

void snd_dice_transaction_destroy(struct snd_dice *dice)
{
	struct fw_address_handler *handler = &dice->notification_handler;

	if (handler->callback_data == NULL)
		return;

	unregister_notification_address(dice);

	fw_core_remove_address_handler(handler);
	handler->callback_data = NULL;
}

int snd_dice_transaction_reinit(struct snd_dice *dice)
{
	struct fw_address_handler *handler = &dice->notification_handler;

	if (handler->callback_data == NULL)
		return -EINVAL;

	return register_notification_address(dice, false);
}

int snd_dice_transaction_init(struct snd_dice *dice)
{
	struct fw_address_handler *handler = &dice->notification_handler;
	__be32 *pointers;
	int err;

	/* Use the same way which dice_interface_check() does. */
	pointers = kmalloc(sizeof(__be32) * 10, GFP_KERNEL);
	if (pointers == NULL)
		return -ENOMEM;

	/* Get offsets for sub-addresses */
	err = snd_fw_transaction(dice->unit, TCODE_READ_BLOCK_REQUEST,
				 DICE_PRIVATE_SPACE,
				 pointers, sizeof(__be32) * 10, 0);
	if (err < 0)
		goto end;

	/* Allocation callback in address space over host controller */
	handler->length = 4;
	handler->address_callback = dice_notification;
	handler->callback_data = dice;
	err = fw_core_add_address_handler(handler, &fw_high_memory_region);
	if (err < 0) {
		handler->callback_data = NULL;
		goto end;
	}

	/* Register the address space */
	err = register_notification_address(dice, true);
	if (err < 0) {
		fw_core_remove_address_handler(handler);
		handler->callback_data = NULL;
		goto end;
	}

	dice->global_offset = be32_to_cpu(pointers[0]) * 4;
	dice->tx_offset = be32_to_cpu(pointers[2]) * 4;
	dice->rx_offset = be32_to_cpu(pointers[4]) * 4;
	dice->sync_offset = be32_to_cpu(pointers[6]) * 4;
	dice->rsrv_offset = be32_to_cpu(pointers[8]) * 4;

	/* Set up later. */
	if (be32_to_cpu(pointers[1]) * 4 >= GLOBAL_CLOCK_CAPABILITIES + 4)
		dice->clock_caps = 1;
end:
	kfree(pointers);
	return err;
}
