// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "highlighter.h"

typedef struct {
	const char *s;
	size_t len;
} string_span_t;

static bool is_decimal(char c)
{
	return c >= '0' && c <= '9';
}

static bool is_hexadecimal(char c)
{
	return is_decimal(c) || ((c | 32) >= 'a' && (c | 32) <= 'f');
}

static bool is_alphabet(char c)
{
	return (c | 32) >= 'a' && (c | 32) <= 'z';
}

static bool is_same(string_span_t s, const char *c)
{
	size_t len = strlen(c);

	if (len != s.len)
		return false;
	return !memcmp(s.s, c, len);
}

static bool is_caseless_same(string_span_t s, const char *c)
{
	size_t len = strlen(c);

	if (len != s.len)
		return false;
	for (size_t i = 0; i < len; ++i) {
		char a = c[i], b = s.s[i];
		if ((unsigned)a - 'a' < 26)
			a &= 95;
		if ((unsigned)b - 'a' < 26)
			b &= 95;
		if (a != b)
			return false;
	}
	return true;
}

static bool is_valid_key(string_span_t s)
{
	if (s.len != 44 || s.s[43] != '=')
		return false;

	for (size_t i = 0; i < 42; ++i) {
		if (!is_decimal(s.s[i]) && !is_alphabet(s.s[i]) &&
		    s.s[i] != '/' && s.s[i] != '+')
			return false;
	}
	switch (s.s[42]) {
	case 'A':
	case 'E':
	case 'I':
	case 'M':
	case 'Q':
	case 'U':
	case 'Y':
	case 'c':
	case 'g':
	case 'k':
	case 'o':
	case 's':
	case 'w':
	case '4':
	case '8':
	case '0':
		break;
	default:
		return false;
	}
	return true;
}

static bool is_valid_hostname(string_span_t s)
{
	size_t num_digit = 0, num_entity = s.len;

	if (s.len > 63 || !s.len)
		return false;
	if (s.s[0] == '-' || s.s[s.len - 1] == '-')
		return false;
	if (s.s[0] == '.' || s.s[s.len - 1] == '.')
		return false;

	for (size_t i = 0; i < s.len; ++i) {
		if (is_decimal(s.s[i])) {
			++num_digit;
			continue;
		}
		if (s.s[i] == '.') {
			--num_entity;
			continue;
		}

		if (!is_alphabet(s.s[i]) && s.s[i] != '-')
			return false;

		if (i && s.s[i] == '.' && s.s[i - 1] == '.')
			return false;
	}
	return num_digit != num_entity;
}

static bool is_valid_ipv4(string_span_t s)
{
	for (size_t j, i = 0, pos = 0; i < 4 && pos < s.len; ++i) {
		uint32_t val = 0;

		for (j = 0; j < 3 && pos + j < s.len && is_decimal(s.s[pos + j]); ++j)
			val = 10 * val + s.s[pos + j] - '0';
		if (j == 0 || (j > 1 && s.s[pos] == '0') || val > 255)
			return false;
		if (pos + j == s.len && i == 3)
			return true;
		if (s.s[pos + j] != '.')
			return false;
		pos += j + 1;
	}
	return false;
}

static bool is_valid_ipv6(string_span_t s)
{
	size_t pos = 0;
	bool seen_colon = false;

	if (s.len < 2)
		return false;
	if (s.s[pos] == ':' && s.s[++pos] != ':')
		return false;
	if (s.s[s.len - 1] == ':' && s.s[s.len - 2] != ':')
		return false;

	for (size_t j, i = 0; pos < s.len; ++i) {
		if (s.s[pos] == ':' && !seen_colon) {
			seen_colon = true;
			if (++pos == s.len)
				break;
			if (i == 7)
				return false;
			continue;
		}
		for (j = 0; j < 4 && pos + j < s.len && is_hexadecimal(s.s[pos + j]); ++j);
		if (j == 0)
			return false;
		if (pos + j == s.len && (seen_colon || i == 7))
			break;
		if (i == 7)
			return false;
		if (s.s[pos + j] != ':') {
			if (s.s[pos + j] != '.' || (i < 6 && !seen_colon))
				return false;
			return is_valid_ipv4((string_span_t){ s.s + pos, s.len - pos });
		}
		pos += j + 1;
	}
	return true;
}

static bool is_valid_uint(string_span_t s, bool support_hex, uint64_t min, uint64_t max)
{
	uint64_t val = 0;

	/* Bound this around 32 bits, so that we don't have to write overflow logic. */
	if (s.len > 10 || !s.len)
		return false;

	if (support_hex && s.len > 2 && s.s[0] == '0' && s.s[1] == 'x') {
		for (size_t i = 2; i < s.len; ++i) {
			if ((unsigned)s.s[i] - '0' < 10)
				val = 16 * val + (s.s[i] - '0');
			else if (((unsigned)s.s[i] | 32) - 'a' < 6)
				val = 16 * val + (s.s[i] | 32) - 'a' + 10;
			else
				return false;
		}
	} else {
		for (size_t i = 0; i < s.len; ++i) {
			if (!is_decimal(s.s[i]))
				return false;
			val = 10 * val + s.s[i] - '0';
		}
	}
	return val <= max && val >= min;
}

static bool is_valid_port(string_span_t s)
{
	return is_valid_uint(s, false, 0, 65535);
}

static bool is_valid_mtu(string_span_t s)
{
	return is_valid_uint(s, false, 576, 65535);
}

static bool is_valid_persistentkeepalive(string_span_t s)
{
	if (is_same(s, "off"))
		return true;
	return is_valid_uint(s, false, 0, 65535);
}

#ifndef MOBILE_WGQUICK_SUBSET

static bool is_valid_fwmark(string_span_t s)
{
	if (is_same(s, "off"))
		return true;
	return is_valid_uint(s, true, 0, 4294967295);
}

static bool is_valid_table(string_span_t s)
{
	if (is_same(s, "auto"))
		return true;
	if (is_same(s, "off"))
		return true;
	/* This pretty much invalidates the other checks, but rt_names.c's
	 * fread_id_name does no validation aside from this. */
	if (s.len < 512)
		return true;
	return is_valid_uint(s, false, 0, 4294967295);
}

static bool is_valid_saveconfig(string_span_t s)
{
	return is_same(s, "true") || is_same(s, "false");
}

static bool is_valid_prepostupdown(string_span_t s)
{
	/* It's probably not worthwhile to try to validate a bash expression.
	 * So instead we just demand non-zero length. */
	return s.len;
}
#endif

static bool is_valid_scope(string_span_t s)
{
	if (s.len > 64 || !s.len)
		return false;
	for (size_t i = 0; i < s.len; ++i) {
		if (!is_alphabet(s.s[i]) && !is_decimal(s.s[i]) &&
		    s.s[i] != '_' && s.s[i] != '=' && s.s[i] != '+' &&
		    s.s[i] != '.' && s.s[i] != '-')
			return false;
	}
	return true;
}

static bool is_valid_endpoint(string_span_t s)
{

	if (!s.len)
		return false;

	if (s.s[0] == '[') {
		bool seen_scope = false;
		string_span_t hostspan = { s.s + 1, 0 };

		for (size_t i = 1; i < s.len; ++i) {
			if (s.s[i] == '%') {
				if (seen_scope)
					return false;
				seen_scope = true;
				if (!is_valid_ipv6(hostspan))
					return false;
				hostspan = (string_span_t){ s.s + i + 1, 0 };
			} else if (s.s[i] == ']') {
				if (seen_scope) {
					if (!is_valid_scope(hostspan))
						return false;
				} else if (!is_valid_ipv6(hostspan)) {
					return false;
				}
				if (i == s.len - 1 || s.s[i + 1] != ':')
					return false;
				return is_valid_port((string_span_t){ s.s + i + 2, s.len - i - 2 });
			} else {
				++hostspan.len;
			}
		}
		return false;
	}
	for (size_t i = 0; i < s.len; ++i) {
		if (s.s[i] == ':') {
			string_span_t host = { s.s, i }, port = { s.s + i + 1, s.len - i - 1};
			return is_valid_port(port) && (is_valid_ipv4(host) || is_valid_hostname(host));
		}
	}
	return false;
}

static bool is_valid_network(string_span_t s)
{
	for (size_t i = 0; i < s.len; ++i) {
		if (s.s[i] == '/') {
			string_span_t ip = { s.s, i }, cidr = { s.s + i + 1, s.len - i - 1};
			uint16_t cidrval = 0;

			if (cidr.len > 3 || !cidr.len)
				return false;

			for (size_t j = 0; j < cidr.len; ++j) {
				if (!is_decimal(cidr.s[j]))
					return false;
				cidrval = 10 * cidrval + cidr.s[j] - '0';
			}
			if (is_valid_ipv4(ip))
				return cidrval <= 32;
			else if (is_valid_ipv6(ip))
				return cidrval <= 128;
			return false;
		}
	}
	return is_valid_ipv4(s) || is_valid_ipv6(s);
}

enum field {
	InterfaceSection,
	PrivateKey,
	ListenPort,
	Address,
	DNS,
	MTU,
#ifndef MOBILE_WGQUICK_SUBSET
	FwMark,
	Table,
	PreUp, PostUp, PreDown, PostDown,
	SaveConfig,
#endif

	PeerSection,
	PublicKey,
	PresharedKey,
	AllowedIPs,
	Endpoint,
	PersistentKeepalive,

	Invalid
};

static enum field section_for_field(enum field t)
{
	if (t > InterfaceSection && t < PeerSection)
		return InterfaceSection;
	if (t > PeerSection && t < Invalid)
		return PeerSection;
	return Invalid;
}

static enum field get_field(string_span_t s)
{
#define check_enum(t) do { if (is_caseless_same(s, #t)) return t; } while (0)
	check_enum(PrivateKey);
	check_enum(ListenPort);
	check_enum(Address);
	check_enum(DNS);
	check_enum(MTU);
	check_enum(PublicKey);
	check_enum(PresharedKey);
	check_enum(AllowedIPs);
	check_enum(Endpoint);
	check_enum(PersistentKeepalive);
#ifndef MOBILE_WGQUICK_SUBSET
	check_enum(FwMark);
	check_enum(Table);
	check_enum(PreUp);
	check_enum(PostUp);
	check_enum(PreDown);
	check_enum(PostDown);
	check_enum(SaveConfig);
#endif
	return Invalid;
#undef check_enum
}

static enum field get_sectiontype(string_span_t s)
{
	if (is_caseless_same(s, "[Peer]"))
		return PeerSection;
	if (is_caseless_same(s, "[Interface]"))
		return InterfaceSection;
	return Invalid;
}

struct highlight_span_array {
	size_t len, capacity;
	struct highlight_span *spans;
};

/* A useful OpenBSD-ism. */
static void *realloc_array(void *optr, size_t nmemb, size_t size)
{
	if ((nmemb >= (size_t)1 << (sizeof(size_t) * 4) ||
	     size >= (size_t)1 << (sizeof(size_t) * 4)) &&
	    nmemb > 0 && SIZE_MAX / nmemb < size) {
		errno = ENOMEM;
		return NULL;
	}
	return realloc(optr, size * nmemb);
}

static bool append_highlight_span(struct highlight_span_array *a, const char *o, string_span_t s, enum highlight_type t)
{
	if (!s.len)
		return true;
	if (a->len >= a->capacity) {
		struct highlight_span *resized;

		a->capacity = a->capacity ? a->capacity * 2 : 64;
		resized = realloc_array(a->spans, a->capacity, sizeof(*resized));
		if (!resized) {
			free(a->spans);
			memset(a, 0, sizeof(*a));
			return false;
		}
		a->spans = resized;
	}
	a->spans[a->len++] = (struct highlight_span){ t, s.s - o, s.len };
	return true;
}

static void highlight_multivalue_value(struct highlight_span_array *ret, const string_span_t parent, const string_span_t s, enum field section)
{
	switch (section) {
	case DNS:
		if (is_valid_ipv4(s) || is_valid_ipv6(s))
			append_highlight_span(ret, parent.s, s, HighlightIP);
		else if (is_valid_hostname(s))
			append_highlight_span(ret, parent.s, s, HighlightHost);
		else
			append_highlight_span(ret, parent.s, s, HighlightError);
		break;
	case Address:
	case AllowedIPs: {
		size_t slash;

		if (!is_valid_network(s)) {
			append_highlight_span(ret, parent.s, s, HighlightError);
			break;
		}
		for (slash = 0; slash < s.len; ++slash) {
			if (s.s[slash] == '/')
				break;
		}
		if (slash == s.len) {
			append_highlight_span(ret, parent.s, s, HighlightIP);
		} else {
			append_highlight_span(ret, parent.s, (string_span_t){ s.s, slash }, HighlightIP);
			append_highlight_span(ret, parent.s, (string_span_t){ s.s + slash, 1 }, HighlightDelimiter);
			append_highlight_span(ret, parent.s, (string_span_t){ s.s + slash + 1, s.len - slash - 1 }, HighlightCidr);
		}
		break;
	}
	default:
		append_highlight_span(ret, parent.s, s, HighlightError);
	}
}

static void highlight_multivalue(struct highlight_span_array *ret, const string_span_t parent, const string_span_t s, enum field section)
{
	string_span_t current_span = { s.s, 0 };
	size_t len_at_last_space = 0;

	for (size_t i = 0; i < s.len; ++i) {
		if (s.s[i] == ',') {
			current_span.len = len_at_last_space;
			highlight_multivalue_value(ret, parent, current_span, section);
			append_highlight_span(ret, parent.s, (string_span_t){ s.s + i, 1 }, HighlightDelimiter);
			len_at_last_space = 0;
			current_span = (string_span_t){ s.s + i + 1, 0 };
		} else if (s.s[i] == ' ' || s.s[i] == '\t') {
			if (&s.s[i] == current_span.s && !current_span.len)
				++current_span.s;
			else
				++current_span.len;
		} else {
			len_at_last_space = ++current_span.len;
		}
	}
	current_span.len = len_at_last_space;
	if (current_span.len)
		highlight_multivalue_value(ret, parent, current_span, section);
	else if (ret->spans[ret->len - 1].type == HighlightDelimiter)
		ret->spans[ret->len - 1].type = HighlightError;
}

static void highlight_value(struct highlight_span_array *ret, const string_span_t parent, const string_span_t s, enum field section)
{
	switch (section) {
	case PrivateKey:
		append_highlight_span(ret, parent.s, s, is_valid_key(s) ? HighlightPrivateKey : HighlightError);
		break;
	case PublicKey:
		append_highlight_span(ret, parent.s, s, is_valid_key(s) ? HighlightPublicKey : HighlightError);
		break;
	case PresharedKey:
		append_highlight_span(ret, parent.s, s, is_valid_key(s) ? HighlightPresharedKey : HighlightError);
		break;
	case MTU:
		append_highlight_span(ret, parent.s, s, is_valid_mtu(s) ? HighlightMTU : HighlightError);
		break;
#ifndef MOBILE_WGQUICK_SUBSET
	case SaveConfig:
		append_highlight_span(ret, parent.s, s, is_valid_saveconfig(s) ? HighlightSaveConfig : HighlightError);
		break;
	case FwMark:
		append_highlight_span(ret, parent.s, s, is_valid_fwmark(s) ? HighlightFwMark : HighlightError);
		break;
	case Table:
		append_highlight_span(ret, parent.s, s, is_valid_table(s) ? HighlightTable : HighlightError);
		break;
	case PreUp:
	case PostUp:
	case PreDown:
	case PostDown:
		append_highlight_span(ret, parent.s, s, is_valid_prepostupdown(s) ? HighlightCmd : HighlightError);
		break;
#endif
	case ListenPort:
		append_highlight_span(ret, parent.s, s, is_valid_port(s) ? HighlightPort : HighlightError);
		break;
	case PersistentKeepalive:
		append_highlight_span(ret, parent.s, s, is_valid_persistentkeepalive(s) ? HighlightKeepalive : HighlightError);
		break;
	case Endpoint: {
		size_t colon;

		if (!is_valid_endpoint(s)) {
			append_highlight_span(ret, parent.s, s, HighlightError);
			break;
		}
		for (colon = s.len; colon --> 0;) {
			if (s.s[colon] == ':')
				break;
		}
		append_highlight_span(ret, parent.s, (string_span_t){ s.s, colon }, HighlightHost);
		append_highlight_span(ret, parent.s, (string_span_t){ s.s + colon, 1 }, HighlightDelimiter);
		append_highlight_span(ret, parent.s, (string_span_t){ s.s + colon + 1, s.len - colon - 1 }, HighlightPort);
		break;
	}
	case Address:
	case DNS:
	case AllowedIPs:
		highlight_multivalue(ret, parent, s, section);
		break;
	default:
		append_highlight_span(ret, parent.s, s, HighlightError);
	}
}

struct highlight_span *highlight_config(const char *config)
{
	struct highlight_span_array ret = { 0 };
	const string_span_t s = { config, strlen(config) };
	string_span_t current_span = { s.s, 0 };
	enum field current_section = Invalid, current_field = Invalid;
	enum { OnNone, OnKey, OnValue, OnComment, OnSection } state = OnNone;
	size_t len_at_last_space = 0, equals_location = 0;

	for (size_t i = 0; i <= s.len; ++i) {
		if (i == s.len || s.s[i] == '\n' || (state != OnComment && s.s[i] == '#')) {
			if (state == OnKey) {
				current_span.len = len_at_last_space;
				append_highlight_span(&ret, s.s, current_span, HighlightError);
			} else if (state == OnValue) {
				if (current_span.len) {
					append_highlight_span(&ret, s.s, (string_span_t){ s.s + equals_location, 1 }, HighlightDelimiter);
					current_span.len = len_at_last_space;
					highlight_value(&ret, s, current_span, current_field);
				} else {
					append_highlight_span(&ret, s.s, (string_span_t){ s.s + equals_location, 1 }, HighlightError);
				}
			} else if (state == OnSection) {
				current_span.len = len_at_last_space;
				current_section = get_sectiontype(current_span);
				append_highlight_span(&ret, s.s, current_span, current_section == Invalid ? HighlightError : HighlightSection);
			} else if (state == OnComment) {
				append_highlight_span(&ret, s.s, current_span, HighlightComment);
			}
			if (i == s.len)
				break;
			len_at_last_space = 0;
			current_field = Invalid;
			if (s.s[i] == '#') {
				current_span = (string_span_t){ s.s + i, 1 };
				state = OnComment;
			} else {
				current_span = (string_span_t){ s.s + i + 1, 0 };
				state = OnNone;
			}
		} else if (state == OnComment) {
			++current_span.len;
		} else if (s.s[i] == ' ' || s.s[i] == '\t') {
			if (&s.s[i] == current_span.s && !current_span.len)
				++current_span.s;
			else
				++current_span.len;
		} else if (s.s[i] == '=' && state == OnKey) {
			current_span.len = len_at_last_space;
			current_field = get_field(current_span);
			enum field section = section_for_field(current_field);
			if (section == Invalid || current_field == Invalid || section != current_section)
				append_highlight_span(&ret, s.s, current_span, HighlightError);
			else
				append_highlight_span(&ret, s.s, current_span, HighlightField);
			equals_location = i;
			current_span = (string_span_t){ s.s + i + 1, 0 };
			state = OnValue;
		} else {
			if (state == OnNone)
				state = s.s[i] == '[' ? OnSection : OnKey;
			len_at_last_space = ++current_span.len;
		}
	}

	append_highlight_span(&ret, s.s, (string_span_t){ s.s, -1 }, HighlightEnd);
	return ret.spans;
}
