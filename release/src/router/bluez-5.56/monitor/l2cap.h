/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#include <stdint.h>
#include <stdbool.h>

struct l2cap_frame {
	uint16_t index;
	bool in;
	uint16_t handle;
	uint8_t ident;
	uint16_t cid;
	uint16_t psm;
	uint16_t chan;
	uint8_t mode;
	uint8_t seq_num;
	const void *data;
	uint16_t size;
};

static inline void l2cap_frame_pull(struct l2cap_frame *frame,
				const struct l2cap_frame *source, uint16_t len)
{
	if (frame != source) {
		frame->index   = source->index;
		frame->in      = source->in;
		frame->handle  = source->handle;
		frame->ident   = source->ident;
		frame->cid     = source->cid;
		frame->psm     = source->psm;
		frame->chan    = source->chan;
		frame->mode    = source->mode;
	}

	frame->data = source->data + len;
	frame->size = source->size - len;
}

static inline bool l2cap_frame_get_u8(struct l2cap_frame *frame, uint8_t *value)
{
	if (frame->size < sizeof(*value))
		return false;

	if (value)
		*value = *((uint8_t *) frame->data);

	l2cap_frame_pull(frame, frame, sizeof(*value));

	return true;
}

static inline bool l2cap_frame_get_be16(struct l2cap_frame *frame,
								uint16_t *value)
{
	if (frame->size < sizeof(*value))
		return false;

	if (value)
		*value = get_be16(frame->data);

	l2cap_frame_pull(frame, frame, sizeof(*value));

	return true;
}

static inline bool l2cap_frame_get_le16(struct l2cap_frame *frame,
								uint16_t *value)
{
	if (frame->size < sizeof(*value))
		return false;

	if (value)
		*value = get_le16(frame->data);

	l2cap_frame_pull(frame, frame, sizeof(*value));

	return true;
}

static inline bool l2cap_frame_get_be32(struct l2cap_frame *frame,
								uint32_t *value)
{
	if (frame->size < sizeof(*value))
		return false;

	if (value)
		*value = get_be32(frame->data);

	l2cap_frame_pull(frame, frame, sizeof(*value));

	return true;
}

static inline bool l2cap_frame_get_le32(struct l2cap_frame *frame,
								uint32_t *value)
{
	if (frame->size < sizeof(*value))
		return false;

	if (value)
		*value = get_le32(frame->data);

	l2cap_frame_pull(frame, frame, sizeof(*value));

	return true;
}

static inline bool l2cap_frame_get_be64(struct l2cap_frame *frame,
								uint64_t *value)
{
	if (frame->size < sizeof(*value))
		return false;

	if (value)
		*value = get_be64(frame->data);

	l2cap_frame_pull(frame, frame, sizeof(*value));

	return true;
}

static inline bool l2cap_frame_get_le64(struct l2cap_frame *frame,
								uint64_t *value)
{
	if (frame->size < sizeof(*value))
		return false;

	if (value)
		*value = get_le64(frame->data);

	l2cap_frame_pull(frame, frame, sizeof(*value));

	return true;
}

static inline bool l2cap_frame_get_be128(struct l2cap_frame *frame,
					uint64_t *lvalue, uint64_t *rvalue)
{
	if (frame->size < (sizeof(*lvalue) + sizeof(*rvalue)))
		return false;

	if (lvalue && rvalue) {
		*lvalue = get_be64(frame->data);
		*rvalue = get_be64(frame->data);
	}

	l2cap_frame_pull(frame, frame, (sizeof(*lvalue) + sizeof(*rvalue)));

	return true;
}

void l2cap_frame(uint16_t index, bool in, uint16_t handle, uint16_t cid,
		uint16_t psm, const void *data, uint16_t size);

void l2cap_packet(uint16_t index, bool in, uint16_t handle, uint8_t flags,
					const void *data, uint16_t size);

void rfcomm_packet(const struct l2cap_frame *frame);
