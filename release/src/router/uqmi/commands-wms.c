/*
 * uqmi -- tiny QMI support implementation
 *
 * Copyright (C) 2014-2015 Felix Fietkau <nbd@openwrt.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include "qmi-message.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define CEILDIV(x,y) (((x) + (y) - 1) / (y))

static void cmd_wms_list_messages_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_wms_list_messages_response res;
	void *c;
	int i;

	qmi_parse_wms_list_messages_response(msg, &res);
	c = blobmsg_open_array(&status, NULL);
	for (i = 0; i < res.data.message_list_n; i++)
		blobmsg_add_u32(&status, NULL, res.data.message_list[i].memory_index);

	blobmsg_close_array(&status, c);
}

static enum qmi_cmd_result
cmd_wms_list_messages_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	static struct qmi_wms_list_messages_request mreq = {
		QMI_INIT(storage_type, QMI_WMS_STORAGE_TYPE_UIM),
		QMI_INIT(message_tag, QMI_WMS_MESSAGE_TAG_TYPE_MT_NOT_READ),
	};

	qmi_set_wms_list_messages_request(msg, &mreq);

	return QMI_CMD_REQUEST;
}

static int
put_unicode_char(char *dest, uint16_t c)
{
	if (c < 0x80) {
		*dest = c;
		return 1;
	} else if (c < 0x800) {
		*(dest++) = 0xc0 | ((c >> 6) & 0x1f);
		*dest = 0x80 | (c & 0x3f);
		return 2;
	} else {
		*(dest++) = 0xe0 | ((c >> 12) & 0xf);
		*(dest++) = 0x80 | ((c >> 6) & 0x3f);
		*dest = 0x80 | (c & 0x3f);
		return 3;
	}
}


static int
pdu_decode_7bit_char(char *dest, int len, unsigned char c, bool *escape)
{
	uint16_t conv_0x20[] = {
		0x0040, 0x00A3, 0x0024, 0x00A5, 0x00E8, 0x00E9, 0x00F9, 0x00EC,
		0x00F2, 0x00E7, 0x000A, 0x00D8, 0x00F8, 0x000D, 0x00C5, 0x00E5,
		0x0394, 0x005F, 0x03A6, 0x0393, 0x039B, 0x03A9, 0x03A0, 0x03A8,
		0x03A3, 0x0398, 0x039E, 0x00A0, 0x00C6, 0x00E6, 0x00DF, 0x00C9,
	};
	uint16_t conv_0x5b[] = {
		0x00C4, 0x00D6, 0x00D1, 0x00DC, 0x00A7, 0x00BF,
	};
	uint16_t conv_0x7b[] = {
		0x00E4, 0x00F6, 0x00F1, 0x00FC, 0x00E0
	};
	int cur_len = 0;
	uint16_t outc;

	fprintf(stderr, " %02x", c);
	dest += len;
	if (*escape) {
		*escape = false;
		switch(c) {
		case 0x0A:
			*dest = 0x0C;
			return 1;
		case 0x14:
			*dest = 0x5E;
			return 1;
		case 0x28:
			*dest = 0x7B;
			return 1;
		case 0x29:
			*dest = 0x7D;
			return 1;
		case 0x2F:
			*dest = 0x5C;
			return 1;
		case 0x3C:
			*dest = 0x5B;
			return 1;
		case 0x3D:
			*dest = 0x7E;
			return 1;
		case 0x3E:
			*dest = 0x5D;
			return 1;
		case 0x40:
			*dest = 0x7C;
			return 1;
		case 0x65:
			outc = 0x20AC;
			goto out;
		case 0x1B:
			goto normal;
		default:
			/* invalid */
			*(dest++) = conv_0x20[0x1B];
			cur_len++;
			goto normal;
		}
	}

	if (c == 0x1b) {
		*escape = true;
		return 0;
	}

normal:
	if (c < 0x20)
		outc = conv_0x20[(int) c];
	else if (c == 0x40)
		outc = 0x00A1;
	else if (c >= 0x5b && c <= 0x60)
		outc = conv_0x5b[c - 0x5b];
	else if (c >= 0x7b && c <= 0x7f)
		outc = conv_0x7b[c - 0x7b];
	else
		outc = c;

out:
	return cur_len + put_unicode_char(dest, outc);
}

static int
pdu_decode_7bit_str(char *dest, const unsigned char *data, int data_len, int bit_offset)
{
	bool escape = false;
	int len = 0;
	int i;

	fprintf(stderr, "Raw text:");
	for (i = 0; i < data_len; i++) {
		int pos = (i + bit_offset) % 7;

		if (pos == 0) {
			len += pdu_decode_7bit_char(dest, len, data[i] & 0x7f, &escape);
		} else {
			if (i)
				len += pdu_decode_7bit_char(dest, len,
				                            (data[i - 1] >> (7 + 1 - pos)) |
				                            ((data[i] << pos) & 0x7f), &escape);

			if (pos == 6)
				len += pdu_decode_7bit_char(dest, len, (data[i] >> 1) & 0x7f,
				                            &escape);
		}
	}
	dest[len] = 0;
	fprintf(stderr, "\n");
	return len;
}

static int decode_udh(const unsigned char *data)
{
	const unsigned char *end;
	unsigned int type, len, udh_len;

	udh_len = *(data++);
	end = data + udh_len;
	while (data < end) {
		const unsigned char *val;

		type = data[0];
		len = data[1];
		val = &data[2];
		data += 2 + len;
		if (data > end)
			break;

		switch (type) {
		case 0x00:
			blobmsg_add_u32(&status, "concat_ref", (uint32_t) val[0]);
			blobmsg_add_u32(&status, "concat_part", (uint32_t) val[2]);
			blobmsg_add_u32(&status, "concat_parts", (uint32_t) val[1]);
			break;
		case 0x08:
			blobmsg_add_u32(&status, "concat_ref", (uint32_t) (val[0] << 8 | val[1]));
			blobmsg_add_u32(&status, "concat_part", (uint32_t) val[3]);
			blobmsg_add_u32(&status, "concat_parts", (uint32_t) val[2]);
			break;
		default:
			break;
		}
	}

	return udh_len + 1;
}

static void decode_7bit_field(char *name, const unsigned char *data, int data_len, int bit_offset)
{
	char *dest = blobmsg_alloc_string_buffer(&status, name, 3 * data_len + 2);
	pdu_decode_7bit_str(dest, data, CEILDIV(data_len * 7, 8), bit_offset);
	dest[data_len] = 0;
	blobmsg_add_string_buffer(&status);
}

static char *pdu_add_semioctet(char *str, char val)
{
	*str = '0' + (val & 0xf);
	if (*str <= '9')
		str++;

	*str = '0' + ((val >> 4) & 0xf);
	if (*str <= '9')
		str++;

	return str;
}

static void
pdu_decode_address(char *str, unsigned char *data, int len)
{
	unsigned char toa;

	toa = *(data++);
	switch (toa & 0x70) {
	case 0x50:
		pdu_decode_7bit_str(str, data, len, 0);
		return;
	case 0x10:
		*(str++) = '+';
		/* fall through */
	default:
		while (len--) {
			str = pdu_add_semioctet(str, *data);
			data++;
		}
	}

	*str = 0;
}

static void wms_decode_address(char *name, unsigned char *data, int len)
{
	char *str = blobmsg_alloc_string_buffer(&status, name, len * 2 + 2);
	pdu_decode_address(str, data, len);
	blobmsg_add_string_buffer(&status);
}

static void blobmsg_add_hex(struct blob_buf *buf, const char *name, unsigned const char *data, int len)
{
	char* str = blobmsg_alloc_string_buffer(buf, name, len * 2 + 1);
	for (int i = 0; i < len; i++) {
		str += sprintf(str, "%02x", data[i]);
	}
	blobmsg_add_string_buffer(buf);
}

#define cmd_wms_delete_message_cb no_cb
static enum qmi_cmd_result
cmd_wms_delete_message_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	char *err;
	int id;

	id = strtoul(arg, &err, 10);
	if (err && *err) {
		uqmi_add_error("Invalid message ID");
		return QMI_CMD_EXIT;
	}

	static struct qmi_wms_delete_request mreq = {
		QMI_INIT(memory_storage, QMI_WMS_STORAGE_TYPE_UIM),
		QMI_INIT(message_mode, QMI_WMS_MESSAGE_MODE_GSM_WCDMA),
	};

	mreq.set.memory_index = 1;
	mreq.data.memory_index = id;

	qmi_set_wms_delete_request(msg, &mreq);

	return QMI_CMD_REQUEST;
}


static void cmd_wms_get_message_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_wms_raw_read_response res;
	unsigned char *data, *end;
	char *str;
	int cur_len;
	bool sent;
	unsigned char first, dcs;
	void *c;

	qmi_parse_wms_raw_read_response(msg, &res);
	c = blobmsg_open_table(&status, NULL);
	data = (unsigned char *) res.data.raw_message_data.raw_data;
	end = data + res.data.raw_message_data.raw_data_n;

	cur_len = *(data++);
	if (data + cur_len >= end)
		goto error;

	if (cur_len) {
		wms_decode_address("smsc", data, cur_len - 1);
		data += cur_len;
	}

	if (data + 3 >= end)
		goto error;

	first = *(data++);
	sent = (first & 0x3) == 1;
	if (sent)
		data++;

	cur_len = *(data++);
	if (data + cur_len >= end)
		goto error;

	if (cur_len) {
		cur_len = (cur_len + 1) / 2;
		wms_decode_address(sent ? "receiver" : "sender", data, cur_len);
		data += cur_len + 1;
	}

	if (data + 3 >= end)
		goto error;

	/* Protocol ID */
	if (*(data++) != 0)
		goto error;

	/* Data Encoding */
	dcs = *(data++);

	if (dcs & 0x10)
		blobmsg_add_u32(&status, "class", (dcs & 3));

	if (sent) {
		/* Message validity */
		data++;
	} else {
		if (data + 6 >= end)
			goto error;

		str = blobmsg_alloc_string_buffer(&status, "timestamp", 32);

		/* year */
		*(str++) = '2';
		*(str++) = '0';
		str = pdu_add_semioctet(str, data[0]);
		/* month */
		*(str++) = '-';
		str = pdu_add_semioctet(str, data[1]);
		/* day */
		*(str++) = '-';
		str = pdu_add_semioctet(str, data[2]);

		/* hour */
		*(str++) = ' ';
		str = pdu_add_semioctet(str, data[3]);
		/* minute */
		*(str++) = ':';
		str = pdu_add_semioctet(str, data[4]);
		/* second */
		*(str++) = ':';
		str = pdu_add_semioctet(str, data[5]);
		*str = 0;

		blobmsg_add_string_buffer(&status);

		data += 7;
	}

	int message_len = *(data++);
	int udh_len = 0;
	int bit_offset = 0;

	/* User Data Header */
	if (first & 0x40) {
		udh_len = decode_udh(data);
		data += udh_len;
		bit_offset = udh_len % 7;
		}

	if (data >= end)
		goto error;

	switch(dcs & 0x0c) {
		case 0x00:
			/* 7 bit GSM alphabet */
			message_len = message_len - CEILDIV(udh_len * 8, 7);
			message_len = MIN(message_len, CEILDIV((end - data) * 8, 7));
			decode_7bit_field("text", data, message_len, bit_offset);
			break;
		case 0x04:
			/* 8 bit data */
			message_len = MIN(message_len - udh_len, end - data);
			blobmsg_add_hex(&status, "data", data, message_len);
			break;
		case 0x08:
			/* 16 bit UCS-2 string */
			message_len = MIN(message_len - udh_len, end - data);
			blobmsg_add_hex(&status, "ucs-2", data, message_len);
			break;
		default:
			goto error;
		}

	blobmsg_close_table(&status, c);
	return;

error:
	blobmsg_close_table(&status, c);
	fprintf(stderr, "There was an error reading message.\n");
}

static enum qmi_cmd_result
cmd_wms_get_message_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	static struct qmi_wms_raw_read_request mreq = {
		QMI_INIT_SEQUENCE(message_memory_storage_id,
			.storage_type = QMI_WMS_STORAGE_TYPE_UIM,
		),
		QMI_INIT(message_mode, QMI_WMS_MESSAGE_MODE_GSM_WCDMA),
	};
	char *err;
	int id;

	id = strtoul(arg, &err, 10);
	if (err && *err) {
		uqmi_add_error("Invalid message ID");
		return QMI_CMD_EXIT;
	}

	mreq.data.message_memory_storage_id.memory_index = id;
	qmi_set_wms_raw_read_request(msg, &mreq);

	return QMI_CMD_REQUEST;
}


static void cmd_wms_get_raw_message_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_wms_raw_read_response res;
	unsigned char *data;
	char *str;
	int i;

	qmi_parse_wms_raw_read_response(msg, &res);
	data = (unsigned char *) res.data.raw_message_data.raw_data;
	str = blobmsg_alloc_string_buffer(&status, NULL, res.data.raw_message_data.raw_data_n * 3);
	for (i = 0; i < res.data.raw_message_data.raw_data_n; i++) {
		str += sprintf(str, &" %02x"[i ? 0 : 1], data[i]);
	}
	blobmsg_add_string_buffer(&status);
}

#define cmd_wms_get_raw_message_prepare cmd_wms_get_message_prepare


static struct {
	const char *smsc;
	const char *target;
	bool flash;
} _send;


#define cmd_wms_send_message_smsc_cb no_cb
static enum qmi_cmd_result
cmd_wms_send_message_smsc_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	_send.smsc = arg;
	return QMI_CMD_DONE;
}

#define cmd_wms_send_message_target_cb no_cb
static enum qmi_cmd_result
cmd_wms_send_message_target_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	_send.target = arg;
	return QMI_CMD_DONE;
}

#define cmd_wms_send_message_flash_cb no_cb
static enum qmi_cmd_result
cmd_wms_send_message_flash_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	_send.flash = true;
	return QMI_CMD_DONE;
}

static int
pdu_encode_semioctet(unsigned char *dest, const char *str)
{
	int len = 0;
	bool lower = true;

	while (*str) {
		char digit = *str - '0';

		if (lower)
			dest[len] = 0xf0 | digit;
		else
			dest[len++] &= (digit << 4) | 0xf;

		lower = !lower;
		str++;
	}

	return lower ? len : (len + 1);
}

static int
pdu_encode_7bit_str(unsigned char *data, const char *str)
{
	unsigned char c;
	int len = 0;
	int ofs = 0;

	while(1) {
		c = *(str++) & 0x7f;
		if (!c)
			break;

		switch(ofs) {
		case 0:
			data[len] = c;
			break;
		default:
			data[len++] |= c << (8 - ofs);
			data[len] = c >> ofs;
			break;
		}

		ofs = (ofs + 1) % 8;
	}

	return len + 1;
}

static int
pdu_encode_number(unsigned char *dest, const char *str, bool smsc)
{
	unsigned char format;
	bool ascii = false;
	int len = 0;
	int i;

	dest[len++] = 0;
	if (*str == '+') {
		str++;
		format = 0x91;
	} else {
		format = 0x81;
	}

	for (i = 0; str[i]; i++) {
		if (str[i] >= '0' && str[i] <= '9')
			continue;

		ascii = true;
		break;
	}

	if (ascii)
		format |= 0x40;

	dest[len++] = format;
	if (!ascii)
		len += pdu_encode_semioctet(&dest[len], str);
	else
		len += pdu_encode_7bit_str(&dest[len], str);

	if (smsc)
		dest[0] = len - 1;
	else
		dest[0] = strlen(str);

	return len;
}

static int
pdu_encode_data(unsigned char *dest, const char *str)
{
	int len = 0;

	dest[len++] = 0;
	len += pdu_encode_7bit_str(&dest[len], str);
	dest[0] = strlen(str);

	return len;
}

#define cmd_wms_send_message_cb no_cb
static enum qmi_cmd_result
cmd_wms_send_message_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	static unsigned char buf[512];
	static struct qmi_wms_raw_send_request mreq = {
		QMI_INIT_SEQUENCE(raw_message_data,
			.format = QMI_WMS_MESSAGE_FORMAT_GSM_WCDMA_POINT_TO_POINT,
			.raw_data = buf,
		),
	};
	unsigned char *cur = buf;
	unsigned char first_octet = 0x11;
	unsigned char protocol_id = 0x00;
	unsigned char dcs = 0x00;

	if (!_send.target || !*_send.target) {
		uqmi_add_error("Missing argument");
		return QMI_CMD_EXIT;
	}

	if ((_send.smsc && strlen(_send.smsc) > 16) || strlen(_send.target) > 16 || strlen(arg) > 160) {
		uqmi_add_error("Argument too long");
		return QMI_CMD_EXIT;
	}

	if (_send.flash)
		dcs |= 0x10;

	if (!_send.smsc || !*_send.smsc)
		*(cur++) = 0;
	else
		cur += pdu_encode_number(cur, _send.smsc, true);

	*(cur++) = first_octet;
	*(cur++) = 0; /* reference */

	cur += pdu_encode_number(cur, _send.target, false);
	*(cur++) = protocol_id;
	*(cur++) = dcs;

	*(cur++) = 0xff; /* validity */
	cur += pdu_encode_data(cur, arg);

	mreq.data.raw_message_data.raw_data_n = cur - buf;
	qmi_set_wms_raw_send_request(msg, &mreq);

	return QMI_CMD_REQUEST;
}
