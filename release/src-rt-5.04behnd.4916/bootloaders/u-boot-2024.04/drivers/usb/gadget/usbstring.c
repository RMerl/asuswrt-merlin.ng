// SPDX-License-Identifier: LGPL-2.1+
/*
 * Copyright (C) 2003 David Brownell
 *
 * Ported to U-Boot by: Thomas Smits <ts.smits@gmail.com> and
 *                      Remy Bohmer <linux@bohmer.net>
 */

#include <common.h>
#include <linux/errno.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/utf.h>

/**
 * usb_gadget_get_string - fill out a string descriptor
 * @table: of c strings encoded using UTF-8
 * @id: string id, from low byte of wValue in get string descriptor
 * @buf: at least 256 bytes
 *
 * Finds the UTF-8 string matching the ID, and converts it into a
 * string descriptor in utf16-le.
 * Returns length of descriptor (always even) or negative errno
 *
 * If your driver needs stings in multiple languages, you'll probably
 * "switch (wIndex) { ... }"  in your ep0 string descriptor logic,
 * using this routine after choosing which set of UTF-8 strings to use.
 * Note that US-ASCII is a strict subset of UTF-8; any string bytes with
 * the eighth bit set will be multibyte UTF-8 characters, not ISO-8859/1
 * characters (which are also widely used in C strings).
 */
int
usb_gadget_get_string(struct usb_gadget_strings *table, int id, u8 *buf)
{
	struct usb_string	*s;
	int			len;

	if (!table)
		return -EINVAL;

	/* descriptor 0 has the language id */
	if (id == 0) {
		buf[0] = 4;
		buf[1] = USB_DT_STRING;
		buf[2] = (u8) table->language;
		buf[3] = (u8) (table->language >> 8);
		return 4;
	}
	for (s = table->strings; s && s->s; s++)
		if (s->id == id)
			break;

	/* unrecognized: stall. */
	if (!s || !s->s)
		return -EINVAL;

	/* string descriptors have length, tag, then UTF16-LE text */
	len = min((size_t) 126, strlen(s->s));
	memset(buf + 2, 0, 2 * len);	/* zero all the bytes */
	len = utf8_to_utf16le(s->s, (__le16 *)&buf[2], len);
	if (len < 0)
		return -EINVAL;
	buf[0] = (len + 1) * 2;
	buf[1] = USB_DT_STRING;
	return buf[0];
}
