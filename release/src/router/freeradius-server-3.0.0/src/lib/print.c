/*
 * print.c	Routines to print stuff.
 *
 * Version:	$Id$
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2000,2006  The FreeRADIUS server project
 */

RCSID("$Id$")

#include	<freeradius-devel/libradius.h>

#include	<ctype.h>

/*
 *	Checks for utf-8, taken from:
 *
 *  http://www.w3.org/International/questions/qa-forms-utf-8
 *
 *	Note that we don't care about the length of the input string,
 *	because '\0' is an invalid UTF-8 character.
 */
int fr_utf8_char(uint8_t const *str)
{
	if (*str < 0x20) return 0;

	if (*str <= 0x7e) return 1; /* 1 */

	if (*str <= 0xc1) return 0;

	if ((str[0] >= 0xc2) &&	/* 2 */
	    (str[0] <= 0xdf) &&
	    (str[1] >= 0x80) &&
	    (str[1] <= 0xbf)) {
		return 2;
	}

	if ((str[0] == 0xe0) &&	/* 3 */
	    (str[1] >= 0xa0) &&
	    (str[1] <= 0xbf) &&
	    (str[2] >= 0x80) &&
	    (str[2] <= 0xbf)) {
		return 3;
	}

	if ((str[0] >= 0xe1) &&	/* 4a */
	    (str[0] <= 0xec) &&
	    (str[1] >= 0x80) &&
	    (str[1] <= 0xbf) &&
	    (str[2] >= 0x80) &&
	    (str[2] <= 0xbf)) {
		return 3;
	}

	if ((str[0] >= 0xee) &&	/* 4b */
	    (str[0] <= 0xef) &&
	    (str[1] >= 0x80) &&
	    (str[1] <= 0xbf) &&
	    (str[2] >= 0x80) &&
	    (str[2] <= 0xbf)) {
		return 3;
	}

	if ((str[0] == 0xed) &&	/* 5 */
	    (str[1] >= 0x80) &&
	    (str[1] <= 0x9f) &&
	    (str[2] >= 0x80) &&
	    (str[2] <= 0xbf)) {
		return 3;
	}

	if ((str[0] == 0xf0) &&	/* 6 */
	    (str[1] >= 0x90) &&
	    (str[1] <= 0xbf) &&
	    (str[2] >= 0x80) &&
	    (str[2] <= 0xbf) &&
	    (str[3] >= 0x80) &&
	    (str[3] <= 0xbf)) {
		return 4;
	}

	if ((str[0] >= 0xf1) &&	/* 6 */
	    (str[1] <= 0xf3) &&
	    (str[1] >= 0x80) &&
	    (str[1] <= 0xbf) &&
	    (str[2] >= 0x80) &&
	    (str[2] <= 0xbf) &&
	    (str[3] >= 0x80) &&
	    (str[3] <= 0xbf)) {
		return 4;
	}


	if ((str[0] == 0xf4) &&	/* 7 */
	    (str[1] >= 0x80) &&
	    (str[1] <= 0x8f) &&
	    (str[2] >= 0x80) &&
	    (str[2] <= 0xbf) &&
	    (str[3] >= 0x80) &&
	    (str[3] <= 0xbf)) {
		return 4;
	}

	/*
	 *	Invalid UTF-8 Character
	 */
	return 0;
}

/*
 *	Convert a string to something printable.  The output string
 *	has to be larger than the input string by at least 5 bytes.
 *	If not, the output is silently truncated...
 */
size_t fr_print_string(char const *in, size_t inlen, char *out, size_t outlen)
{
	char const	*start = out;
	uint8_t const	*str = (uint8_t const *) in;
	int		sp = 0;
	int		utf8 = 0;

	if (!in) {
		if (outlen) {
			*out = '\0';
		}

		return 0;
	}

	if (inlen == 0) {
		inlen = strlen(in);
	}

	/*
	 *
	 */
	while ((inlen > 0) && (outlen > 4)) {
		/*
		 *	Hack: never print trailing zero.
		 *	Some clients send strings with an off-by-one
		 *	length (confused with strings in C).
		 */
		if ((inlen == 1) && (*str == 0)) break;

		switch (*str) {
			case '\\':
				sp = '\\';
				break;
			case '\r':
				sp = 'r';
				break;
			case '\n':
				sp = 'n';
				break;
			case '\t':
				sp = 't';
				break;
			case '"':
				sp = '"';
				break;
			default:
				sp = 0;
				break;
		}

		if (sp) {
			*out++ = '\\';
			*out++ = sp;
			outlen -= 2;
			str++;
			inlen--;
			continue;
		}

		utf8 = fr_utf8_char(str);
		if (!utf8) {
			snprintf(out, outlen, "\\%03o", *str);
			out  += 4;
			outlen -= 4;
			str++;
			inlen--;
			continue;
		}

		do {
			*out++ = *str++;
			outlen--;
			inlen--;
		} while (--utf8 > 0);
	}
	*out = '\0';

	return out - start;
}


/** Print the value of an attribute to a string
 *
 * @param[out] out Where to write the string.
 * @param[in] outlen Size of outlen.
 * @param[in] vp to print.
 * @param[in] quote Char to add before and after printed value, if 0 no char will be added, if < 0 raw string will be
 *	added.
 * @return length of data written to out or 0 on error.
 */
size_t vp_prints_value(char *out, size_t outlen, VALUE_PAIR const *vp, int8_t quote)
{
	DICT_VALUE  *v;
	char	buf[1024];
	char const  *a = NULL;
	size_t      len;
	time_t      t;
	struct tm   s_tm;

	out[0] = '\0';
	if (!vp) return 0;

	switch (vp->da->type) {
		case PW_TYPE_STRING:
			if ((quote > 0) && vp->da->flags.has_tag) {
				/* Tagged attribute: print delimiter and ignore tag */
				buf[0] = (char) quote;
				len = fr_print_string(vp->vp_strvalue, vp->length, buf + 1, sizeof(buf) - 2);
				buf[len + 1] = (char) quote;
				buf[len + 2] = '\0';
			} else if (quote > 0) {
				/* Non-tagged attribute: print delimiter */
				buf[0] = (char) quote;
				len = fr_print_string(vp->vp_strvalue, vp->length, buf + 1, sizeof(buf) - 2);
				buf[len + 1] = (char) quote;
				buf[len + 2] = '\0';

			} else if (quote < 0) { /* xlat.c */
				strlcpy(out, vp->vp_strvalue, outlen);
				return strlen(out);

			} else {
				/* Non-tagged attribute: no delimiter */
				fr_print_string(vp->vp_strvalue, vp->length, buf, sizeof(buf));
			}
			a = buf;
			break;
		case PW_TYPE_INTEGER:
			if (vp->da->flags.has_tag) {
				/* Attribute value has a tag, need to ignore it */
				if ((v = dict_valbyattr(vp->da->attr, vp->da->vendor, (vp->vp_integer & 0xffffff)))
				    != NULL)
					a = v->name;
				else {
					snprintf(buf, sizeof(buf), "%u", (vp->vp_integer & 0xffffff));
					a = buf;
				}
			} else {
		case PW_TYPE_BYTE:
		case PW_TYPE_SHORT:
				/* Normal, non-tagged attribute */
				if ((v = dict_valbyattr(vp->da->attr, vp->da->vendor, vp->vp_integer))
				    != NULL)
					a = v->name;
				else {
					snprintf(buf, sizeof(buf), "%u", vp->vp_integer);
					a = buf;
				}
			}
			break;
		case PW_TYPE_INTEGER64:
			snprintf(buf, sizeof(buf), "%" PRIu64, vp->vp_integer64);
			a = buf;
			break;
		case PW_TYPE_DATE:
			t = vp->vp_date;
			if (quote > 0) {
				len = strftime(buf, sizeof(buf) - 1, "%%%b %e %Y %H:%M:%S %Z%%",
					       localtime_r(&t, &s_tm));
				buf[0] = (char) quote;
				buf[len - 1] = (char) quote;
				buf[len] = '\0';
			} else {
				len = strftime(buf, sizeof(buf), "%b %e %Y %H:%M:%S %Z",
					       localtime_r(&t, &s_tm));
			}
			if (len > 0) a = buf;
			break;
		case PW_TYPE_SIGNED: /* Damned code for 1 WiMAX attribute */
			snprintf(buf, sizeof(buf), "%d", vp->vp_signed);
			a = buf;
			break;
		case PW_TYPE_IPADDR:
			a = inet_ntop(AF_INET, &(vp->vp_ipaddr),
				      buf, sizeof(buf));
			break;
		case PW_TYPE_ABINARY:
#ifdef WITH_ASCEND_BINARY
			a = buf;
			print_abinary(vp, buf, sizeof(buf), quote);
			break;
#else
		  /* FALL THROUGH */
#endif
		case PW_TYPE_OCTETS:
			if (outlen <= (2 * (vp->length + 1))) return 0;

			strcpy(buf, "0x");

			fr_bin2hex(buf + 2, vp->vp_octets, vp->length);
			a = buf;
		  break;

		case PW_TYPE_IFID:
			a = ifid_ntoa(buf, sizeof(buf), vp->vp_ifid);
			break;

		case PW_TYPE_IPV6ADDR:
			a = inet_ntop(AF_INET6,
				      &vp->vp_ipv6addr,
				      buf, sizeof(buf));
			break;

		case PW_TYPE_IPV6PREFIX:
		{
			struct in6_addr addr;

			/*
			 *	Alignment issues.
			 */
			memcpy(&addr, &(vp->vp_ipv6prefix[2]), sizeof(addr));

			a = inet_ntop(AF_INET6, &addr, buf, sizeof(buf));
			if (a) {
				char *p = buf + strlen(buf);
				snprintf(p, buf + sizeof(buf) - p - 1, "/%u",
					 (unsigned int) vp->vp_ipv6prefix[1]);
			}
		}
			break;

		case PW_TYPE_IPV4PREFIX:
		{
			struct in_addr addr;

			/*
			 *	Alignment issues.
			 */
			memcpy(&addr, &(vp->vp_ipv4prefix[2]), sizeof(addr));

			a = inet_ntop(AF_INET, &addr, buf, sizeof(buf));
			if (a) {
				char *p = buf + strlen(buf);
				snprintf(p, buf + sizeof(buf) - p - 1, "/%u",
					 (unsigned int) (vp->vp_ipv4prefix[1] & 0x3f));
			}
		}
			break;

		case PW_TYPE_ETHERNET:
			snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
				 vp->vp_ether[0], vp->vp_ether[1],
				 vp->vp_ether[2], vp->vp_ether[3],
				 vp->vp_ether[4], vp->vp_ether[5]);
			a = buf;
			break;

		case PW_TYPE_TLV:
			if (outlen <= (2 * (vp->length + 1))) return 0;

			strcpy(buf, "0x");

			fr_bin2hex(buf + 2, vp->vp_tlv, vp->length);
			a = buf;
		  break;

		default:
			a = "UNKNOWN-TYPE";
			break;
	}

	if (a != NULL) strlcpy(out, a, outlen);

	return strlen(out);
}


char *vp_aprinttype(TALLOC_CTX *ctx, PW_TYPE type)
{
	switch (type) {
	case PW_TYPE_STRING :
		return talloc_strdup(ctx, "_");

	case PW_TYPE_INTEGER64:
	case PW_TYPE_SIGNED:
	case PW_TYPE_BYTE:
	case PW_TYPE_SHORT:
	case PW_TYPE_INTEGER:
	case PW_TYPE_DATE :
		return talloc_strdup(ctx, "0");

	case PW_TYPE_IPADDR :
		return talloc_strdup(ctx, "?.?.?.?");

	case PW_TYPE_IPV4PREFIX:
		return talloc_strdup(ctx, "?.?.?.?/?");

	case PW_TYPE_IPV6ADDR:
		return talloc_strdup(ctx, "[:?:]");

	case PW_TYPE_IPV6PREFIX:
		return talloc_strdup(ctx, "[:?:]/?");

	case PW_TYPE_OCTETS:
		return talloc_strdup(ctx, "0x??");

	case PW_TYPE_ETHERNET:
		return talloc_strdup(ctx, "??:??:??:??:??:??:??:??");

#ifdef WITH_ASCEND_BINARY
	case PW_TYPE_ABINARY:
		return talloc_strdup(ctx, "??");
#endif

	default :
		break;
	}

	return talloc_strdup(ctx, "<UNKNOWN-TYPE>");
}

/*
 *	vp_prints_value for talloc
 */
char *vp_aprint(TALLOC_CTX *ctx, VALUE_PAIR const *vp)
{
	char *p;

	switch (vp->da->type) {
	case PW_TYPE_STRING:
		/*
		 *	FIXME: deal with \r\n" ??
		 */
		p = talloc_strdup(ctx, vp->vp_strvalue);
		break;

	case PW_TYPE_BYTE:
	case PW_TYPE_SHORT:
	case PW_TYPE_INTEGER:
		{
			DICT_VALUE *dv;

			dv = dict_valbyattr(vp->da->attr, vp->da->vendor,
					    vp->vp_integer);
			if (dv) {
				p = talloc_strdup(ctx, dv->name);
			} else {
				p = talloc_asprintf(ctx, "%u", vp->vp_integer);
			}
		}
		break;

	case PW_TYPE_SIGNED:
		p = talloc_asprintf(ctx, "%d", vp->vp_signed);
		break;

	case PW_TYPE_INTEGER64:
		p = talloc_asprintf(ctx, "%" PRIu64 , vp->vp_integer64);
		break;

	case PW_TYPE_ETHERNET:
		p = talloc_asprintf(ctx, "%02x:%02x:%02x:%02x:%02x:%02x",
				    vp->vp_ether[0], vp->vp_ether[1],
				    vp->vp_ether[2], vp->vp_ether[3],
				    vp->vp_ether[4], vp->vp_ether[5]);
		break;

	case PW_TYPE_ABINARY:
#ifdef WITH_ASCEND_BINARY
		p = talloc_array(ctx, char, 128);
		if (!p) return NULL;
		print_abinary(vp, p, 128, 0);
		break;
#else
		  /* FALL THROUGH */
#endif

	case PW_TYPE_OCTETS:
		p = talloc_array(ctx, char, 3 + vp->length * 2);
		if (!p) return NULL;
		memcpy(p, "0x", 2);
		fr_bin2hex(p + 2, vp->vp_octets, vp->length);
		break;

	case PW_TYPE_DATE:
	{
		time_t      t;
		struct tm   s_tm;

		t = vp->vp_date;

		p = talloc_array(ctx, char, 64);
		strftime(p, 64, "%b %e %Y %H:%M:%S %Z",
			 localtime_r(&t, &s_tm));
		break;
	}

	case PW_TYPE_IPADDR:
		p = talloc_asprintf(ctx, "%u.%u.%u.%u",
				    vp->vp_ipv4prefix[0], /* network byte order */
				    vp->vp_ipv4prefix[1],
				    vp->vp_ipv4prefix[2],
				    vp->vp_ipv4prefix[3]);
		break;

	case PW_TYPE_IPV4PREFIX:
		p = talloc_asprintf(ctx, "%u.%u.%u.%u/%u",
				    vp->vp_ipv4prefix[2],
				    vp->vp_ipv4prefix[3],
				    vp->vp_ipv4prefix[4],
				    vp->vp_ipv4prefix[5],
				    vp->vp_ipv4prefix[1] & 0x3f);
		break;

	case PW_TYPE_IPV6ADDR:
		p = talloc_asprintf(ctx, "%x:%x:%x:%x:%x:%x:%x:%x",
				    (vp->vp_ipv6addr.s6_addr[0] << 8) | vp->vp_ipv6addr.s6_addr[1],
				    (vp->vp_ipv6addr.s6_addr[2] << 8) | vp->vp_ipv6addr.s6_addr[3],
				    (vp->vp_ipv6addr.s6_addr[4] << 8) | vp->vp_ipv6addr.s6_addr[5],
				    (vp->vp_ipv6addr.s6_addr[6] << 8) | vp->vp_ipv6addr.s6_addr[7],
				    (vp->vp_ipv6addr.s6_addr[8] << 8) | vp->vp_ipv6addr.s6_addr[9],
				    (vp->vp_ipv6addr.s6_addr[10] << 8) | vp->vp_ipv6addr.s6_addr[11],
				    (vp->vp_ipv6addr.s6_addr[12] << 8) | vp->vp_ipv6addr.s6_addr[13],
				    (vp->vp_ipv6addr.s6_addr[14] << 8) | vp->vp_ipv6addr.s6_addr[15]);
		break;

	case PW_TYPE_IPV6PREFIX:
		p = talloc_asprintf(ctx, "%x:%x:%x:%x:%x:%x:%x:%x/%u",
				    (vp->vp_ipv6prefix[2] << 8) | vp->vp_ipv6prefix[3],
				    (vp->vp_ipv6prefix[4] << 8) | vp->vp_ipv6prefix[5],
				    (vp->vp_ipv6prefix[6] << 8) | vp->vp_ipv6prefix[7],
				    (vp->vp_ipv6prefix[8] << 8) | vp->vp_ipv6prefix[9],
				    (vp->vp_ipv6prefix[10] << 8) | vp->vp_ipv6prefix[11],
				    (vp->vp_ipv6prefix[12] << 8) | vp->vp_ipv6prefix[13],
				    (vp->vp_ipv6prefix[14] << 8) | vp->vp_ipv6prefix[15],
				    (vp->vp_ipv6prefix[16] << 8) | vp->vp_ipv6prefix[17],
				    vp->vp_ipv6prefix[2]);
		break;

	case PW_TYPE_IFID:
		p = talloc_asprintf(ctx, "%x:%x:%x:%x",
				    (vp->vp_ifid[0] << 8) | vp->vp_ifid[1],
				    (vp->vp_ifid[2] << 8) | vp->vp_ifid[3],
				    (vp->vp_ifid[4] << 8) | vp->vp_ifid[5],
				    (vp->vp_ifid[6] << 8) | vp->vp_ifid[7]);
		break;

	default:
		p = NULL;
		break;
	}

	return p;
}


/**  Prints attribute values escaped suitably for use as JSON values
 *
 *  Returns < 0 if the buffer may be (or have been) too small to write the encoded
 *  JSON value to.
 *
 * @param out Where to write the string.
 * @param outlen Lenth of output buffer.
 * @param vp to print.
 * @return the length of data written to out, or a value >= outlen on truncation.
 */
size_t vp_prints_value_json(char *out, size_t outlen, VALUE_PAIR const *vp)
{
	char		*start = out;
	char const	*q;
	size_t		len, freespace = outlen;

	if (!vp->da->flags.has_tag) {
		switch (vp->da->type) {
			case PW_TYPE_INTEGER:
			case PW_TYPE_BYTE:
			case PW_TYPE_SHORT:
				if (vp->da->flags.has_value) break;

				len = snprintf(out, freespace, "%u", vp->vp_integer);
				return len;

			case PW_TYPE_SIGNED:
				len = snprintf(out, freespace, "%d", vp->vp_signed);
				return len;

			default:
				break;
		}
	}

	if (freespace < 2) return -1;
	*out++ = '"';
	freespace--;

	switch (vp->da->type) {
		case PW_TYPE_STRING:
			for (q = vp->vp_strvalue; q < vp->vp_strvalue + vp->length; q++) {
				if (freespace < 3) return -1;

				if (*q == '"') {
					*out++ = '\\';
					*out++ = '"';
					freespace -= 2;
				} else if (*q == '\\') {
					*out++ = '\\';
					*out++ = '\\';
					freespace -= 2;
				} else if (*q == '/') {
					*out++ = '\\';
					*out++ = '/';
					freespace -= 2;
				} else if (*q >= ' ') {
					*out++ = *q;
					freespace--;
				} else {
					*out++ = '\\';
					freespace--;

					switch (*q) {
					case '\b':
						*out++ = 'b';
						freespace--;
						break;

					case '\f':
						*out++ = 'f';
						freespace--;
						break;

					case '\n':
						*out++ = 'b';
						freespace--;
						break;

					case '\r':
						*out++ = 'r';
						freespace--;
						break;

					case '\t':
						*out++ = 't';
						freespace--;
						break;
					default:
						len = snprintf(out, freespace, "u%04X", *q);
						if (len >= freespace) return outlen;
						out += len;
						freespace -= len;
					}
				}
			}
			break;

		default:
			len = vp_prints_value(out, freespace, vp, 0);
			if (len >= freespace) return outlen;
			out += len;
			freespace -= len;
			break;
	}

	if (freespace < 2) return outlen;
	*out++ = '"';
	*out = '\0'; // We don't increment out, because the nul byte should not be included in the length

	return out - start;
}

/*
 *  This is a hack, and has to be kept in sync with tokens.h
 */
static char const *vp_tokens[] = {
  "?",				/* T_OP_INVALID */
  "EOL",			/* T_EOL */
  "{",
  "}",
  "(",
  ")",
  ",",
  ";",
  "++",
  "+=",
  "-=",
  ":=",
  "=",
  "!=",
  ">=",
  ">",
  "<=",
  "<",
  "=~",
  "!~",
  "=*",
  "!*",
  "==",
  "#",
  "<BARE-WORD>",
  "<\"STRING\">",
  "<'STRING'>",
  "<`STRING`>"
};

extern int fr_attr_max_tlv;
extern int fr_attr_shift[];
extern int fr_attr_mask[];

/** Print an attribute OID (does not include vendor)
 *
 * @param out Where to write the string.
 * @param outlen Lenth of output buffer.
 * @param attr id.
 * @param dv_type Type of dictionary value.
 * @return the length of data written to out, or a value >= outlen on truncation.
 */
static size_t vp_print_attr_oid(char *out, size_t outlen, unsigned int attr, int dv_type)
{
	int		nest;
	char		*start = out;
	size_t		len, freespace = outlen;

	switch (dv_type) {
	case 4:
		return snprintf(out, freespace, "%u", attr);

	case 2:
		return snprintf(out, freespace, "%u", attr & 0xffff);

	default:
	case 1:
		len = snprintf(out, freespace, "%u", attr & 0xff);
		if (len >= freespace) return outlen;
		out += len;
		freespace -= len;
		break;
	}

	if ((attr >> 8) == 0) return out - start;

	for (nest = 1; nest <= fr_attr_max_tlv; nest++) {
		if (((attr >> fr_attr_shift[nest]) & fr_attr_mask[nest]) == 0) break;

		len = snprintf(out, freespace, ".%u", (attr >> fr_attr_shift[nest]) & fr_attr_mask[nest]);
		if (len >= freespace) return outlen;
		out += freespace;
		freespace -= len;
	}

	return out - start;
}

/** Print the names of attributes which are not in the dictionaries
 *
 * Print name for an unknown attribute in the format:
@verbatim
	Attr-<vendor id>-<attribute oid>
@endverbatim
 * to a string.
 *
 * @param out Where to write the string.
 * @param outlen Lenth of output buffer.
 * @param attr id.
 * @param vendor id.
 * @return the length of data written to out, or a value >= outlen on truncation.
 */
size_t vp_print_name(char *out, size_t outlen, unsigned int attr, unsigned int vendor)
{
	int 		dv_type = 1;
	char		*start = out;
	size_t		len, freespace = outlen;

	if (!out) return 0;

	len = snprintf(out, freespace, "Attr-");
	if (len >= freespace) return outlen;
	out += len;
	freespace -= len;

	if (vendor > FR_MAX_VENDOR) {
		len = snprintf(out, freespace, "%u.", vendor / FR_MAX_VENDOR);
		if (len >= freespace) return outlen;
		out += len;
		freespace -= len;

		vendor &= (FR_MAX_VENDOR) - 1;
	}

	if (vendor) {
		DICT_VENDOR *dv;

		dv = dict_vendorbyvalue(vendor);
		if (dv) {
			dv_type = dv->type;
		}

		len = snprintf(out, freespace, "26.%u.", vendor);
		if (len >= freespace) return outlen;
		out += len;
		freespace -= len;
	}

	len = vp_print_attr_oid(out, freespace, attr, dv_type);
	if (len >= freespace) return outlen;
	out += len;

	return out - start;
}


/** Print one attribute and value to a string
 *
 * Print a VALUE_PAIR in the format:
@verbatim
	<attribute_name>[:tag] <op> <value>
@endverbatim
 * to a string.
 *
 * @param out Where to write the string.
 * @param outlen Lenth of output buffer.
 * @param vp to print.
 * @return the length of data written to out, or a value >= outlen on truncation.
 */
size_t vp_prints(char *out, size_t outlen, VALUE_PAIR const *vp)
{
	char const	*token = NULL;
	char		*start = out;
	size_t		len, freespace = outlen;

	if (!out) return 0;

	*out = '\0';
	if (!vp || !vp->da) return 0;

	VERIFY_VP(vp);

	if ((vp->op > T_OP_INVALID) && (vp->op < T_TOKEN_LAST)) {
		token = vp_tokens[vp->op];
	} else {
		token = "<INVALID-TOKEN>";
	}

	if(vp->da->flags.has_tag) {
		len = snprintf(out, freespace, "%s:%d %s ", vp->da->name, vp->tag, token);
	} else {
		len = snprintf(out, freespace, "%s %s ", vp->da->name, token);
	}
	if (len >= freespace) return outlen;
	out += len;
	freespace -= len;

	len = vp_prints_value(out, freespace, vp, '\'');
	if (len >= freespace) return outlen;
	out += len;

	return out - start;
}


/** Print one attribute and value to FP
 *
 * Complete string with '\\t' and '\\n' is written to buffer before printing to
 * avoid issues when running with multiple threads.
 *
 * @param fp to output to.
 * @param vp to print.
 */
void vp_print(FILE *fp, VALUE_PAIR const *vp)
{
	char	buf[1024];
	char	*p = buf;
	size_t	len;

	*p++ = '\t';
	len = vp_prints(p, sizeof(buf) - 1, vp);
	if (!len) {
		return;
	}
	p += len;

	/*
	 *	Deal with truncation gracefully
	 */
	if (((size_t) (p - buf)) >= sizeof(buf)) {
		p = buf + (sizeof(buf) - 2);
	}

	*p++ = '\n';
	*p = '\0';

	fputs(buf, fp);
}


/** Print a list of attributes and values
 *
 * @param fp to output to.
 * @param vp to print.
 */
void vp_printlist(FILE *fp, VALUE_PAIR const *vp)
{
	vp_cursor_t cursor;
	for (vp = paircursorc(&cursor, &vp); vp; vp = pairnext(&cursor)) {
		vp_print(fp, vp);
	}
}

