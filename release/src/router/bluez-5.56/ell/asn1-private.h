/*
 *  Embedded Linux library
 *
 *  Copyright (C) 2017  Intel Corporation. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#define ASN1_ID(class, pc, tag)	(((class) << 6) | ((pc) << 5) | (tag))

#define ASN1_CLASS_UNIVERSAL	0
#define ASN1_CLASS_CONTEXT	2

#define ASN1_ID_SEQUENCE	ASN1_ID(ASN1_CLASS_UNIVERSAL, 1, 0x10)
#define ASN1_ID_SET		ASN1_ID(ASN1_CLASS_UNIVERSAL, 1, 0x11)
#define ASN1_ID_BOOLEAN		ASN1_ID(ASN1_CLASS_UNIVERSAL, 0, 0x01)
#define ASN1_ID_INTEGER		ASN1_ID(ASN1_CLASS_UNIVERSAL, 0, 0x02)
#define ASN1_ID_BIT_STRING	ASN1_ID(ASN1_CLASS_UNIVERSAL, 0, 0x03)
#define ASN1_ID_OCTET_STRING	ASN1_ID(ASN1_CLASS_UNIVERSAL, 0, 0x04)
#define ASN1_ID_NULL		ASN1_ID(ASN1_CLASS_UNIVERSAL, 0, 0x05)
#define ASN1_ID_OID		ASN1_ID(ASN1_CLASS_UNIVERSAL, 0, 0x06)
#define ASN1_ID_UTF8STRING	ASN1_ID(ASN1_CLASS_UNIVERSAL, 0, 0x0c)
#define ASN1_ID_PRINTABLESTRING	ASN1_ID(ASN1_CLASS_UNIVERSAL, 0, 0x13)
#define ASN1_ID_IA5STRING	ASN1_ID(ASN1_CLASS_UNIVERSAL, 0, 0x16)

struct asn1_oid {
	uint8_t asn1_len;
	uint8_t asn1[11];
};

#define asn1_oid_eq(oid1, oid2_len, oid2_string) \
	((oid1)->asn1_len == (oid2_len) && \
	 !memcmp((oid1)->asn1, (oid2_string), (oid2_len)))

#if __STDC_VERSION__ <= 199409L
#define inline __inline__
#endif

static inline int asn1_parse_definite_length(const uint8_t **buf,
						size_t *len)
{
	int n;
	size_t result = 0;

	/* Decrease the buffer length left */
	if ((*len)-- < 1)
		return -1;

	/*
	 * If short form length, move the pointer to start of data and
	 * return the data length.
	 */
	if (!(**buf & 0x80))
		return *(*buf)++;

	n = *(*buf)++ & 0x7f;
	if ((size_t) n > *len)
		return -1;

	*len -= n;
	while (n--)
		result = (result << 8) | *(*buf)++;

	return result;
}

static inline void asn1_write_definite_length(uint8_t **buf, size_t len)
{
	int n;

	if (len < 0x80) {
		*(*buf)++ = len;
		return;
	}

	for (n = 1; len >> (n * 8); n++);
	*(*buf)++ = 0x80 | n;

	while (n--)
		*(*buf)++ = len >> (n * 8);
}

#define ASN1_CONTEXT_IMPLICIT(tag) (0x1000 | (tag))
#define ASN1_CONTEXT_EXPLICIT(tag) (0x2000 | (tag))

/*
 * Return the tag, length and value of the @index'th
 * non-context-specific-tagged element in a DER SEQUENCE or one who's
 * ASN1_CONTEXT_IMPLICIT(tag) matches @index or the inner element of
 * the one who's ASN1_CONTEXT_EXPLICIT(tag) matches @index.
 */
static inline const uint8_t *asn1_der_find_elem(const uint8_t *buf,
						size_t len_in, int index,
						uint8_t *tag, size_t *len_out)
{
	int n = 0;

	while (1) {
		int tlv_len;

		if (len_in < 2)
			return NULL;

		*tag = *buf++;
		len_in--;

		tlv_len = asn1_parse_definite_length((void *) &buf, &len_in);
		if (tlv_len < 0 || (size_t) tlv_len > len_in)
			return NULL;

		if (*tag >> 6 != ASN1_CLASS_CONTEXT) {
			if (n++ == index) {
				*len_out = tlv_len;
				return buf;
			}
		} else if ((*tag & 0x1f) == (index & 0xfff)) {
			/* Context-specific tag */
			if (index & 0x1000) {		/* Implicit */
				*len_out = tlv_len;
				return buf;
			} else if (index & 0x2000) {	/* Explicit */
				const uint8_t *outer = buf;
				int inner_len;

				if (!(*tag & 0x20))	/* Primitive */
					return NULL;

				if (unlikely(tlv_len < 2))
					return NULL;

				*tag = *buf++;

				inner_len = asn1_parse_definite_length(
							(void *) &buf, &len_in);
				if (outer + tlv_len != buf + inner_len)
					return NULL;

				*len_out = inner_len;
				return buf;
			}
		}

		buf += tlv_len;
		len_in -= tlv_len;
	}
}

/* Return an element in a DER SEQUENCE structure by path */
static inline const uint8_t *asn1_der_find_elem_by_path(const uint8_t *buf,
						size_t len_in, uint8_t tag,
						size_t *len_out, ...)
{
	int index;
	va_list vl;

	va_start(vl, len_out);

	index = va_arg(vl, int);

	while (index != -1) {
		uint8_t elem_tag;
		uint8_t expect_tag;
		int prev_index = index;

		buf = asn1_der_find_elem(buf, len_in, index,
						&elem_tag, &len_in);
		if (!buf) {
			va_end(vl);
			return NULL;
		}

		index = va_arg(vl, int);

		if (prev_index & 0x1000)
			expect_tag = ASN1_ID(ASN1_CLASS_CONTEXT,
						index != -1 ? 1 :
						((elem_tag >> 5) & 1),
						prev_index & 0xfff);
		else
			expect_tag = (index == -1) ? tag : ASN1_ID_SEQUENCE;

		if (elem_tag != expect_tag) {
			va_end(vl);
			return NULL;
		}
	}

	va_end(vl);

	*len_out = len_in;
	return buf;
}
