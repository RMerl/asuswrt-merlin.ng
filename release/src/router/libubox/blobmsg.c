/*
 * Copyright (C) 2010-2012 Felix Fietkau <nbd@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include "blobmsg.h"

static const int blob_type[__BLOBMSG_TYPE_LAST] = {
	[BLOBMSG_TYPE_INT8] = BLOB_ATTR_INT8,
	[BLOBMSG_TYPE_INT16] = BLOB_ATTR_INT16,
	[BLOBMSG_TYPE_INT32] = BLOB_ATTR_INT32,
	[BLOBMSG_TYPE_INT64] = BLOB_ATTR_INT64,
	[BLOBMSG_TYPE_STRING] = BLOB_ATTR_STRING,
	[BLOBMSG_TYPE_UNSPEC] = BLOB_ATTR_BINARY,
};

static uint16_t
blobmsg_namelen(const struct blobmsg_hdr *hdr)
{
	return be16_to_cpu(hdr->namelen);
}

bool blobmsg_check_attr(const struct blob_attr *attr, bool name)
{
	const struct blobmsg_hdr *hdr;
	const char *data;
	int id, len;

	if (blob_len(attr) < sizeof(struct blobmsg_hdr))
		return false;

	hdr = (void *) attr->data;
	if (!hdr->namelen && name)
		return false;

	if (blobmsg_namelen(hdr) > blob_len(attr) - sizeof(struct blobmsg_hdr))
		return false;

	if (hdr->name[blobmsg_namelen(hdr)] != 0)
		return false;

	id = blob_id(attr);
	len = blobmsg_data_len(attr);
	data = blobmsg_data(attr);

	if (id > BLOBMSG_TYPE_LAST)
		return false;

	if (!blob_type[id])
		return true;

	return blob_check_type(data, len, blob_type[id]);
}

int blobmsg_check_array(const struct blob_attr *attr, int type)
{
	struct blob_attr *cur;
	bool name;
	int rem;
	int size = 0;

	switch (blobmsg_type(attr)) {
	case BLOBMSG_TYPE_TABLE:
		name = true;
		break;
	case BLOBMSG_TYPE_ARRAY:
		name = false;
		break;
	default:
		return -1;
	}

	blobmsg_for_each_attr(cur, attr, rem) {
		if (type != BLOBMSG_TYPE_UNSPEC && blobmsg_type(cur) != type)
			return -1;

		if (!blobmsg_check_attr(cur, name))
			return -1;

		size++;
	}

	return size;
}

bool blobmsg_check_attr_list(const struct blob_attr *attr, int type)
{
	return blobmsg_check_array(attr, type) >= 0;
}

int blobmsg_parse_array(const struct blobmsg_policy *policy, int policy_len,
			struct blob_attr **tb, void *data, unsigned int len)
{
	struct blob_attr *attr;
	int i = 0;

	memset(tb, 0, policy_len * sizeof(*tb));
	__blob_for_each_attr(attr, data, len) {
		if (policy[i].type != BLOBMSG_TYPE_UNSPEC &&
		    blob_id(attr) != policy[i].type)
			continue;

		if (!blobmsg_check_attr(attr, false))
			return -1;

		if (tb[i])
			continue;

		tb[i++] = attr;
		if (i == policy_len)
			break;
	}

	return 0;
}


int blobmsg_parse(const struct blobmsg_policy *policy, int policy_len,
                  struct blob_attr **tb, void *data, unsigned int len)
{
	struct blobmsg_hdr *hdr;
	struct blob_attr *attr;
	uint8_t *pslen;
	int i;

	memset(tb, 0, policy_len * sizeof(*tb));
	pslen = alloca(policy_len);
	for (i = 0; i < policy_len; i++) {
		if (!policy[i].name)
			continue;

		pslen[i] = strlen(policy[i].name);
	}

	__blob_for_each_attr(attr, data, len) {
		hdr = blob_data(attr);
		for (i = 0; i < policy_len; i++) {
			if (!policy[i].name)
				continue;

			if (policy[i].type != BLOBMSG_TYPE_UNSPEC &&
			    blob_id(attr) != policy[i].type)
				continue;

			if (blobmsg_namelen(hdr) != pslen[i])
				continue;

			if (!blobmsg_check_attr(attr, true))
				return -1;

			if (tb[i])
				continue;

			if (strcmp(policy[i].name, (char *) hdr->name) != 0)
				continue;

			tb[i] = attr;
		}
	}

	return 0;
}


static struct blob_attr *
blobmsg_new(struct blob_buf *buf, int type, const char *name, int payload_len, void **data)
{
	struct blob_attr *attr;
	struct blobmsg_hdr *hdr;
	int attrlen, namelen;
	char *pad_start, *pad_end;

	if (!name)
		name = "";

	namelen = strlen(name);
	attrlen = blobmsg_hdrlen(namelen) + payload_len;
	attr = blob_new(buf, type, attrlen);
	if (!attr)
		return NULL;

	attr->id_len |= be32_to_cpu(BLOB_ATTR_EXTENDED);
	hdr = blob_data(attr);
	hdr->namelen = cpu_to_be16(namelen);
	strcpy((char *) hdr->name, (const char *)name);
	pad_end = *data = blobmsg_data(attr);
	pad_start = (char *) &hdr->name[namelen];
	if (pad_start < pad_end)
		memset(pad_start, 0, pad_end - pad_start);

	return attr;
}

static inline int
attr_to_offset(struct blob_buf *buf, struct blob_attr *attr)
{
	return (char *)attr - (char *) buf->buf + BLOB_COOKIE;
}


void *
blobmsg_open_nested(struct blob_buf *buf, const char *name, bool array)
{
	struct blob_attr *head;
	int type = array ? BLOBMSG_TYPE_ARRAY : BLOBMSG_TYPE_TABLE;
	unsigned long offset = attr_to_offset(buf, buf->head);
	void *data;

	if (!name)
		name = "";

	head = blobmsg_new(buf, type, name, 0, &data);
	if (!head)
		return NULL;
	blob_set_raw_len(buf->head, blob_pad_len(buf->head) - blobmsg_hdrlen(strlen(name)));
	buf->head = head;
	return (void *)offset;
}

int
blobmsg_vprintf(struct blob_buf *buf, const char *name, const char *format, va_list arg)
{
	va_list arg2;
	char cbuf;
	char *sbuf;
	int len, ret;

	va_copy(arg2, arg);
	len = vsnprintf(&cbuf, sizeof(cbuf), format, arg2);
	va_end(arg2);

	sbuf = blobmsg_alloc_string_buffer(buf, name, len + 1);
	if (!sbuf)
		return -1;
	ret = vsprintf(sbuf, format, arg);
	blobmsg_add_string_buffer(buf);

	return ret;
}

int
blobmsg_printf(struct blob_buf *buf, const char *name, const char *format, ...)
{
	va_list ap;
	int ret;

	va_start(ap, format);
	ret = blobmsg_vprintf(buf, name, format, ap);
	va_end(ap);

	return ret;
}

void *
blobmsg_alloc_string_buffer(struct blob_buf *buf, const char *name, unsigned int maxlen)
{
	struct blob_attr *attr;
	void *data_dest;

	attr = blobmsg_new(buf, BLOBMSG_TYPE_STRING, name, maxlen, &data_dest);
	if (!attr)
		return NULL;

	blob_set_raw_len(buf->head, blob_pad_len(buf->head) - blob_pad_len(attr));
	blob_set_raw_len(attr, blob_raw_len(attr) - maxlen);

	return data_dest;
}

void *
blobmsg_realloc_string_buffer(struct blob_buf *buf, unsigned int maxlen)
{
	struct blob_attr *attr = blob_next(buf->head);
	int offset = attr_to_offset(buf, blob_next(buf->head)) + blob_pad_len(attr) - BLOB_COOKIE;
	int required = maxlen - (buf->buflen - offset);

	if (required <= 0)
		goto out;

	if (!blob_buf_grow(buf, required))
		return NULL;
	attr = blob_next(buf->head);

out:
	return blobmsg_data(attr);
}

void
blobmsg_add_string_buffer(struct blob_buf *buf)
{
	struct blob_attr *attr;
	int len, attrlen;

	attr = blob_next(buf->head);
	len = strlen(blobmsg_data(attr)) + 1;

	attrlen = blob_raw_len(attr) + len;
	blob_set_raw_len(attr, attrlen);
	blob_fill_pad(attr);

	blob_set_raw_len(buf->head, blob_raw_len(buf->head) + blob_pad_len(attr));
}

int
blobmsg_add_field(struct blob_buf *buf, int type, const char *name,
                  const void *data, unsigned int len)
{
	struct blob_attr *attr;
	void *data_dest;

	attr = blobmsg_new(buf, type, name, len, &data_dest);
	if (!attr)
		return -1;

	if (len > 0)
		memcpy(data_dest, data, len);

	return 0;
}
