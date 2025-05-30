/*
 * Copyright (c) 2002-2007, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "includes.h"
#include "common.h"

static int hex2num(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}


int hex2byte(const char *hex)
{
	int a, b;
	a = hex2num(*hex++);
	if (a < 0)
		return -1;
	b = hex2num(*hex++);
	if (b < 0)
		return -1;
	return (a << 4) | b;
}


static const char * hwaddr_parse(const char *txt, u8 *addr)
{
	size_t i;

	for (i = 0; i < ETH_ALEN; i++) {
		int a;

		a = hex2byte(txt);
		if (a < 0)
			return NULL;
		txt += 2;
		addr[i] = a;
		if (i < ETH_ALEN - 1 && *txt++ != ':')
			return NULL;
	}
	return txt;
}


/**
 * hwaddr_aton - Convert ASCII string to MAC address (colon-delimited format)
 * @txt: MAC address as a string (e.g., "00:11:22:33:44:55")
 * @addr: Buffer for the MAC address (ETH_ALEN = 6 bytes)
 * Returns: 0 on success, -1 on failure (e.g., string not a MAC address)
 */
int hwaddr_aton(const char *txt, u8 *addr)
{
	return hwaddr_parse(txt, addr) ? 0 : -1;
}


/**
 * hwaddr_masked_aton - Convert ASCII string with optional mask to MAC address (colon-delimited format)
 * @txt: MAC address with optional mask as a string (e.g., "00:11:22:33:44:55/ff:ff:ff:ff:00:00")
 * @addr: Buffer for the MAC address (ETH_ALEN = 6 bytes)
 * @mask: Buffer for the MAC address mask (ETH_ALEN = 6 bytes)
 * @maskable: Flag to indicate whether a mask is allowed
 * Returns: 0 on success, -1 on failure (e.g., string not a MAC address)
 */
int hwaddr_masked_aton(const char *txt, u8 *addr, u8 *mask, u8 maskable)
{
	const char *r;

	/* parse address part */
	r = hwaddr_parse(txt, addr);
	if (!r)
		return -1;

	/* check for optional mask */
	if (*r == '\0' || isspace((unsigned char) *r)) {
		/* no mask specified, assume default */
		os_memset(mask, 0xff, ETH_ALEN);
	} else if (maskable && *r == '/') {
		/* mask specified and allowed */
		r = hwaddr_parse(r + 1, mask);
		/* parser error? */
		if (!r)
			return -1;
	} else {
		/* mask specified but not allowed or trailing garbage */
		return -1;
	}

	return 0;
}


/**
 * hwaddr_compact_aton - Convert ASCII string to MAC address (no colon delimitors format)
 * @txt: MAC address as a string (e.g., "001122334455")
 * @addr: Buffer for the MAC address (ETH_ALEN = 6 bytes)
 * Returns: 0 on success, -1 on failure (e.g., string not a MAC address)
 */
int hwaddr_compact_aton(const char *txt, u8 *addr)
{
	int i;

	for (i = 0; i < 6; i++) {
		int a, b;

		a = hex2num(*txt++);
		if (a < 0)
			return -1;
		b = hex2num(*txt++);
		if (b < 0)
			return -1;
		*addr++ = (a << 4) | b;
	}

	return 0;
}

/**
 * hwaddr_aton2 - Convert ASCII string to MAC address (in any known format)
 * @txt: MAC address as a string (e.g., 00:11:22:33:44:55 or 0011.2233.4455)
 * @addr: Buffer for the MAC address (ETH_ALEN = 6 bytes)
 * Returns: Characters used (> 0) on success, -1 on failure
 */
int hwaddr_aton2(const char *txt, u8 *addr)
{
	int i;
	const char *pos = txt;

	for (i = 0; i < 6; i++) {
		int a, b;

		while (*pos == ':' || *pos == '.' || *pos == '-')
			pos++;

		a = hex2num(*pos++);
		if (a < 0)
			return -1;
		b = hex2num(*pos++);
		if (b < 0)
			return -1;
		*addr++ = (a << 4) | b;
	}

	return pos - txt;
}


/**
 * hexstr2bin - Convert ASCII hex string into binary data
 * @hex: ASCII hex string (e.g., "01ab")
 * @buf: Buffer for the binary data
 * @len: Length of the text to convert in bytes (of buf); hex will be double
 * this size
 * Returns: 0 on success, -1 on failure (invalid hex string)
 */
int hexstr2bin(const char *hex, u8 *buf, size_t len)
{
	size_t i;
	int a;
	const char *ipos = hex;
	u8 *opos = buf;

	for (i = 0; i < len; i++) {
		a = hex2byte(ipos);
		if (a < 0)
			return -1;
		*opos++ = a;
		ipos += 2;
	}
	return 0;
}


int hwaddr_mask_txt(char *buf, size_t len, const u8 *addr, const u8 *mask)
{
	size_t i;
	int print_mask = 0;
	int res;

	for (i = 0; i < ETH_ALEN; i++) {
		if (mask[i] != 0xff) {
			print_mask = 1;
			break;
		}
	}

	if (print_mask)
		res = os_snprintf(buf, len, MACSTR "/" MACSTR,
				  MAC2STR(addr), MAC2STR(mask));
	else
		res = os_snprintf(buf, len, MACSTR, MAC2STR(addr));
	if (os_snprintf_error(len, res))
		return -1;
	return res;
}


/**
 * inc_byte_array - Increment arbitrary length byte array by one
 * @counter: Pointer to byte array
 * @len: Length of the counter in bytes
 *
 * This function increments the last byte of the counter by one and continues
 * rolling over to more significant bytes if the byte was incremented from
 * 0xff to 0x00.
 */
void inc_byte_array(u8 *counter, size_t len)
{
	int pos = len - 1;
	while (pos >= 0) {
		counter[pos]++;
		if (counter[pos] != 0)
			break;
		pos--;
	}
}


#ifdef CONFIG_ANSI_C_EXTRA

#ifdef _WIN32_WCE
void perror(const char *s)
{
	mapd_printf(MSG_ERROR, "%s: GetLastError: %d",
		   s, (int) GetLastError());
}
#endif /* _WIN32_WCE */


int optind = 1;
int optopt;
char *optarg;

int getopt(int argc, char *const argv[], const char *optstring)
{
	static int optchr = 1;
	char *cp;

	if (optchr == 1) {
		if (optind >= argc) {
			/* all arguments processed */
			return EOF;
		}

		if (argv[optind][0] != '-' || argv[optind][1] == '\0') {
			/* no option characters */
			return EOF;
		}
	}

	if (os_strcmp(argv[optind], "--") == 0) {
		/* no more options */
		optind++;
		return EOF;
	}

	optopt = argv[optind][optchr];
	cp = os_strchr(optstring, optopt);
	if (cp == NULL || optopt == ':') {
		if (argv[optind][++optchr] == '\0') {
			optchr = 1;
			optind++;
		}
		return '?';
	}

	if (cp[1] == ':') {
		/* Argument required */
		optchr = 1;
		if (argv[optind][optchr + 1]) {
			/* No space between option and argument */
			optarg = &argv[optind++][optchr + 1];
		} else if (++optind >= argc) {
			/* option requires an argument */
			return '?';
		} else {
			/* Argument in the next argv */
			optarg = argv[optind++];
		}
	} else {
		/* No argument */
		if (argv[optind][++optchr] == '\0') {
			optchr = 1;
			optind++;
		}
		optarg = NULL;
	}
	return *cp;
}
#endif /* CONFIG_ANSI_C_EXTRA */

void printf_encode(char *txt, size_t maxlen, const u8 *data, size_t len)
{
	char *end = txt + maxlen;
	size_t i;

	for (i = 0; i < len; i++) {
		if (txt + 4 >= end)
			break;

		switch (data[i]) {
		case '\"':
			*txt++ = '\\';
			*txt++ = '\"';
			break;
		case '\\':
			*txt++ = '\\';
			*txt++ = '\\';
			break;
		case '\033':
			*txt++ = '\\';
			*txt++ = 'e';
			break;
		case '\n':
			*txt++ = '\\';
			*txt++ = 'n';
			break;
		case '\r':
			*txt++ = '\\';
			*txt++ = 'r';
			break;
		case '\t':
			*txt++ = '\\';
			*txt++ = 't';
			break;
		default:
			if (data[i] >= 32 && data[i] <= 126) {
				*txt++ = data[i];
			} else {
				txt += os_snprintf(txt, end - txt, "\\x%02x",
						   data[i]);
			}
			break;
		}
	}

	*txt = '\0';
}


size_t printf_decode(u8 *buf, size_t maxlen, const char *str)
{
	const char *pos = str;
	size_t len = 0;
	int val;

	while (*pos) {
		if (len + 1 >= maxlen)
			break;
		switch (*pos) {
		case '\\':
			pos++;
			switch (*pos) {
			case '\\':
				buf[len++] = '\\';
				pos++;
				break;
			case '"':
				buf[len++] = '"';
				pos++;
				break;
			case 'n':
				buf[len++] = '\n';
				pos++;
				break;
			case 'r':
				buf[len++] = '\r';
				pos++;
				break;
			case 't':
				buf[len++] = '\t';
				pos++;
				break;
			case 'e':
				buf[len++] = '\033';
				pos++;
				break;
			case 'x':
				pos++;
				val = hex2byte(pos);
				if (val < 0) {
					val = hex2num(*pos);
					if (val < 0)
						break;
					buf[len++] = val;
					pos++;
				} else {
					buf[len++] = val;
					pos += 2;
				}
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				val = *pos++ - '0';
				if (*pos >= '0' && *pos <= '7')
					val = val * 8 + (*pos++ - '0');
				if (*pos >= '0' && *pos <= '7')
					val = val * 8 + (*pos++ - '0');
				buf[len++] = val;
				break;
			default:
				break;
			}
			break;
		default:
			buf[len++] = *pos++;
			break;
		}
	}
	if (maxlen > len)
		buf[len] = '\0';

	return len;
}

void * __hide_aliasing_typecast(void *foo)
{
	return foo;
}


char * mapd_config_parse_string(const char *value, size_t *len)
{
	if (*value == '"') {
		const char *pos;
		char *str;
		value++;
		pos = os_strrchr(value, '"');
		if (pos == NULL || pos[1] != '\0')
			return NULL;
		*len = pos - value;
		str = dup_binstr(value, *len);
		if (str == NULL)
			return NULL;
		return str;
	} else if (*value == 'P' && value[1] == '"') {
		const char *pos;
		char *tstr, *str;
		size_t tlen;
		value += 2;
		pos = os_strrchr(value, '"');
		if (pos == NULL || pos[1] != '\0')
			return NULL;
		tlen = pos - value;
		tstr = dup_binstr(value, tlen);
		if (tstr == NULL)
			return NULL;

		str = os_malloc(tlen + 1);
		if (str == NULL) {
			os_free(tstr);
			return NULL;
		}

		*len = printf_decode((u8 *) str, tlen + 1, tstr);
		os_free(tstr);

		return str;
	} else {
		u8 *str;
		size_t tlen, hlen = os_strlen(value);
		if (hlen & 1)
			return NULL;
		tlen = hlen / 2;
		str = os_malloc(tlen + 1);
		if (str == NULL)
			return NULL;
		if (hexstr2bin(value, str, tlen)) {
			os_free(str);
			return NULL;
		}
		str[tlen] = '\0';
		*len = tlen;
		return (char *) str;
	}
}


int is_hex(const u8 *data, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++) {
		if (data[i] < 32 || data[i] >= 127)
			return 1;
	}
	return 0;
}


int has_ctrl_char(const u8 *data, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++) {
		if (data[i] < 32 || data[i] == 127)
			return 1;
	}
	return 0;
}


int has_newline(const char *str)
{
	while (*str) {
		if (*str == '\n' || *str == '\r')
			return 1;
		str++;
	}
	return 0;
}


size_t merge_byte_arrays(u8 *res, size_t res_len,
			 const u8 *src1, size_t src1_len,
			 const u8 *src2, size_t src2_len)
{
	size_t len = 0;

	os_memset(res, 0, res_len);

	if (src1) {
		if (src1_len >= res_len) {
			os_memcpy(res, src1, res_len);
			return res_len;
		}

		os_memcpy(res, src1, src1_len);
		len += src1_len;
	}

	if (src2) {
		if (len + src2_len >= res_len) {
			os_memcpy(res + len, src2, res_len - len);
			return res_len;
		}

		os_memcpy(res + len, src2, src2_len);
		len += src2_len;
	}

	return len;
}


char * dup_binstr(const void *src, size_t len)
{
	char *res;

	if (src == NULL)
		return NULL;
	res = os_malloc(len + 1);
	if (res == NULL)
		return NULL;
	os_memcpy(res, src, len);
	res[len] = '\0';

	return res;
}

int int_array_len(const int *a)
{
	int i;
	for (i = 0; a && a[i]; i++)
		;
	return i;
}


void int_array_concat(int **res, const int *a)
{
	int reslen, alen, i;
	int *n;

	reslen = int_array_len(*res);
	alen = int_array_len(a);

	n = os_realloc_array(*res, reslen + alen + 1, sizeof(int));
	if (n == NULL) {
		os_free(*res);
		*res = NULL;
		return;
	}
	for (i = 0; i <= alen; i++)
		n[reslen + i] = a[i];
	*res = n;
}


static int freq_cmp(const void *a, const void *b)
{
	int _a = *(int *) a;
	int _b = *(int *) b;

	if (_a == 0)
		return 1;
	if (_b == 0)
		return -1;
	return _a - _b;
}


void int_array_sort_unique(int *a)
{
	int alen;
	int i, j;

	if (a == NULL)
		return;

	alen = int_array_len(a);
	qsort(a, alen, sizeof(int), freq_cmp);

	i = 0;
	j = 1;
	while (a[i] && a[j]) {
		if (a[i] == a[j]) {
			j++;
			continue;
		}
		a[++i] = a[j++];
	}
	if (a[i])
		i++;
	a[i] = 0;
}


void int_array_add_unique(int **res, int a)
{
	int reslen;
	int *n;

	for (reslen = 0; *res && (*res)[reslen]; reslen++) {
		if ((*res)[reslen] == a)
			return; /* already in the list */
	}

	n = os_realloc_array(*res, reslen + 2, sizeof(int));
	if (n == NULL) {
		os_free(*res);
		*res = NULL;
		return;
	}

	n[reslen] = a;
	n[reslen + 1] = 0;

	*res = n;
}


void str_clear_free(char *str)
{
	if (str) {
		size_t len = os_strlen(str);
		os_memset(str, 0, len);
		os_free(str);
	}
}


void bin_clear_free(void *bin, size_t len)
{
	if (bin) {
		os_memset(bin, 0, len);
		os_free(bin);
	}
}


int random_mac_addr(u8 *addr)
{
	if (os_get_random(addr, ETH_ALEN) < 0)
		return -1;
	addr[0] &= 0xfe; /* unicast */
	addr[0] |= 0x02; /* locally administered */
	return 0;
}


int random_mac_addr_keep_oui(u8 *addr)
{
	if (os_get_random(addr + 3, 3) < 0)
		return -1;
	addr[0] &= 0xfe; /* unicast */
	addr[0] |= 0x02; /* locally administered */
	return 0;
}


/**
 * cstr_token - Get next token from const char string
 * @str: a constant string to tokenize
 * @delim: a string of delimiters
 * @last: a pointer to a character following the returned token
 *      It has to be set to NULL for the first call and passed for any
 *      further call.
 * Returns: a pointer to token position in str or NULL
 *
 * This function is similar to str_token, but it can be used with both
 * char and const char strings. Differences:
 * - The str buffer remains unmodified
 * - The returned token is not a NULL terminated string, but a token
 *   position in str buffer. If a return value is not NULL a size
 *   of the returned token could be calculated as (last - token).
 */
const char * cstr_token(const char *str, const char *delim, const char **last)
{
	const char *end, *token = str;

	if (!str || !delim || !last)
		return NULL;

	if (*last)
		token = *last;

	while (*token && os_strchr(delim, *token))
		token++;

	if (!*token)
		return NULL;

	end = token + 1;

	while (*end && !os_strchr(delim, *end))
		end++;

	*last = end;
	return token;
}


/**
 * str_token - Get next token from a string
 * @buf: String to tokenize. Note that the string might be modified.
 * @delim: String of delimiters
 * @context: Pointer to save our context. Should be initialized with
 *	NULL on the first call, and passed for any further call.
 * Returns: The next token, NULL if there are no more valid tokens.
 */
char * str_token(char *str, const char *delim, char **context)
{
	char *token = (char *) cstr_token(str, delim, (const char **) context);

	if (token && **context)
		*(*context)++ = '\0';

	return token;
}


size_t utf8_unescape(const char *inp, size_t in_size,
		     char *outp, size_t out_size)
{
	size_t res_size = 0;

	if (!inp || !outp)
		return 0;

	if (!in_size)
		in_size = os_strlen(inp);

	/* Advance past leading single quote */
	if (*inp == '\'' && in_size) {
		inp++;
		in_size--;
	}

	while (in_size--) {
		if (res_size >= out_size)
			return 0;

		switch (*inp) {
		case '\'':
			/* Terminate on bare single quote */
			*outp = '\0';
			return res_size;

		case '\\':
			if (!in_size--)
				return 0;
			inp++;
			/* fall through */

		default:
			*outp++ = *inp++;
			res_size++;
		}
	}

	/* NUL terminate if space allows */
	if (res_size < out_size)
		*outp = '\0';

	return res_size;
}


size_t utf8_escape(const char *inp, size_t in_size,
		   char *outp, size_t out_size)
{
	size_t res_size = 0;

	if (!inp || !outp)
		return 0;

	/* inp may or may not be NUL terminated, but must be if 0 size
	 * is specified */
	if (!in_size)
		in_size = os_strlen(inp);

	while (in_size--) {
		if (res_size++ >= out_size)
			return 0;

		switch (*inp) {
		case '\\':
		case '\'':
			if (res_size++ >= out_size)
				return 0;
			*outp++ = '\\';
			/* fall through */

		default:
			*outp++ = *inp++;
			break;
		}
	}

	/* NUL terminate if space allows */
	if (res_size < out_size)
		*outp = '\0';

	return res_size;
}


int is_ctrl_char(char c)
{
	return c > 0 && c < 32;
}

int str_starts(const char *str, const char *start)
{
	return os_strncmp(str, start, os_strlen(start)) == 0;
}


/**
 * rssi_to_rcpi - Convert RSSI to RCPI
 * @rssi: RSSI to convert
 * Returns: RCPI corresponding to the given RSSI value, or 255 if not available.
 *
 * It's possible to estimate RCPI based on RSSI in dBm. This calculation will
 * not reflect the correct value for high rates, but it's good enough for Action
 * frames which are transmitted with up to 24 Mbps rates.
 */
u8 rssi_to_rcpi(int rssi)
{
	if (!rssi)
		return 255; /* not available */
	if (rssi < -110)
		return 0;
	if (rssi > 0)
		return 220;
	return (rssi + 110) * 2;
}

s8 rcpi_to_rssi(u8 rcpi)
{
	if (rcpi == 255)
		return 0; /* not available */
	return ((rcpi - 220)/2);
}


uint8_t chan_to_idx(uint8_t chan)
{
    if (chan >= 1 && chan <= 14)
        return chan - 1;
    if (chan >= 36 && chan <= 64)
        return 14 +  ((chan -36)/4);
    if (chan >= 100 && chan <= 144)
        return 22 + ((chan - 100)/4);
    else if (chan >= 149 && chan <= 165)
        return 34 + ((chan - 149)/4);
    return -1;
}

uint8_t idx_to_chan(uint8_t idx)
{
    if (idx >= 0 && idx <= 13)
        return idx + 1;
    if (idx >= 14 && idx <= 21)
        return 36 + ((idx - 14) * 4);
    if (idx >= 22 && idx <= 33)
        return 100 + ((idx - 22) * 4);
    else if (idx >= 34 && idx <= 37)
        return 149 + ((idx -34) * 4);
    return 0;
}

uint8_t isChan5GL(uint8_t chan)
{
	if (chan >= 36 && chan <= 64)
		return 1;
	return 0;
}

uint8_t isChan5GH(uint8_t chan)
{
	if (chan >= 100 && chan <= 165)
		return 1;
	return 0;
}
