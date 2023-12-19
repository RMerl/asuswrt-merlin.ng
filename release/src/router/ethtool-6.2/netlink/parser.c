/*
 * parser.c - netlink command line parser
 *
 * Implementation of command line parser used by netlink code.
 */

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#include "../internal.h"
#include "../common.h"
#include "netlink.h"
#include "parser.h"

static void parser_err_unknown_param(struct nl_context *nlctx)
{
	fprintf(stderr, "ethtool (%s): unknown parameter '%s'\n", nlctx->cmd,
		nlctx->param);
}

static void parser_err_dup_param(struct nl_context *nlctx)
{
	fprintf(stderr, "ethtool (%s): duplicate parameter '%s'\n", nlctx->cmd,
		nlctx->param);
}

static void parser_err_min_argc(struct nl_context *nlctx, unsigned int min_argc)
{
	if (min_argc == 1)
		fprintf(stderr, "ethtool (%s): no value for parameter '%s'\n",
			nlctx->cmd, nlctx->param);
	else
		fprintf(stderr,
			"ethtool (%s): parameter '%s' requires %u words\n",
			nlctx->cmd, nlctx->param, min_argc);
}

static void parser_err_invalid_value(struct nl_context *nlctx, const char *val)
{
	fprintf(stderr, "ethtool (%s): invalid value '%s' for parameter '%s'\n",
		nlctx->cmd, val, nlctx->param);
}

static void parser_err_invalid_flag(struct nl_context *nlctx, const char *flag)
{
	fprintf(stderr, "ethtool (%s): flag '%s' for parameter '%s' is not followed by 'on' or 'off'\n",
		nlctx->cmd, flag, nlctx->param);
}

static bool __prefix_0x(const char *p)
{
	return p[0] == '0' && (p[1] == 'x' || p[1] == 'X');
}

static float parse_float(const char *arg, float *result, float min,
			 float max)
{
	char *endptr;
	float val;

	if (!arg || !arg[0])
		return -EINVAL;
	val = strtof(arg, &endptr);
	if (*endptr || val < min || val > max)
		return -EINVAL;

	*result = val;
	return 0;
}

static int __parse_u32(const char *arg, uint32_t *result, uint32_t min,
		       uint32_t max, int base)
{
	unsigned long long val;
	char *endptr;

	if (!arg || !arg[0])
		return -EINVAL;
	val = strtoul(arg, &endptr, base);
	if (*endptr || val < min || val > max)
		return -EINVAL;

	*result = (uint32_t)val;
	return 0;
}

static int parse_u32d(const char *arg, uint32_t *result)
{
	return __parse_u32(arg, result, 0, 0xffffffff, 10);
}

static int parse_x32(const char *arg, uint32_t *result)
{
	return __parse_u32(arg, result, 0, 0xffffffff, 16);
}

int parse_u32(const char *arg, uint32_t *result)
{
	if (!arg)
		return -EINVAL;
	if (__prefix_0x(arg))
		return parse_x32(arg + 2, result);
	else
		return parse_u32d(arg, result);
}

static int parse_u8(const char *arg, uint8_t *result)
{
	uint32_t val;
	int ret = parse_u32(arg, &val);

	if (ret < 0)
		return ret;
	if (val > UINT8_MAX)
		return -EINVAL;

	*result = (uint8_t)val;
	return 0;
}

static int lookup_u32(const char *arg, uint32_t *result,
		      const struct lookup_entry_u32 *tbl)
{
	if (!arg)
		return -EINVAL;
	while (tbl->arg) {
		if (!strcmp(tbl->arg, arg)) {
			*result = tbl->val;
			return 0;
		}
		tbl++;
	}

	return -EINVAL;
}

static int lookup_u8(const char *arg, uint8_t *result,
		     const struct lookup_entry_u8 *tbl)
{
	if (!arg)
		return -EINVAL;
	while (tbl->arg) {
		if (!strcmp(tbl->arg, arg)) {
			*result = tbl->val;
			return 0;
		}
		tbl++;
	}

	return -EINVAL;
}

/* Parser handler for a flag. Expects a name (with no additional argument),
 * generates NLA_FLAG or sets a bool (if the name was present).
 */
int nl_parse_flag(struct nl_context *nlctx __maybe_unused, uint16_t type,
		  const void *data __maybe_unused, struct nl_msg_buff *msgbuff,
		  void *dest)
{
	if (dest)
		*(bool *)dest = true;
	return (type && ethnla_put_flag(msgbuff, type, true)) ? -EMSGSIZE : 0;
}

/* Parser handler for null terminated string. Expects a string argument,
 * generates NLA_NUL_STRING or fills const char *
 */
int nl_parse_string(struct nl_context *nlctx, uint16_t type,
		    const void *data __maybe_unused,
		    struct nl_msg_buff *msgbuff, void *dest)
{
	const char *arg = *nlctx->argp;

	nlctx->argp++;
	nlctx->argc--;

	if (dest)
		*(const char **)dest = arg;
	return (type && ethnla_put_strz(msgbuff, type, arg)) ? -EMSGSIZE : 0;
}

/* Parser handler for unsigned 32-bit integer. Expects a numeric argument
 * (may use 0x prefix), generates NLA_U32 or fills an uint32_t.
 */
int nl_parse_direct_u32(struct nl_context *nlctx, uint16_t type,
			const void *data __maybe_unused,
			struct nl_msg_buff *msgbuff, void *dest)
{
	const char *arg = *nlctx->argp;
	uint32_t val;
	int ret;

	nlctx->argp++;
	nlctx->argc--;
	ret = parse_u32(arg, &val);
	if (ret < 0) {
		parser_err_invalid_value(nlctx, arg);
		return ret;
	}

	if (dest)
		*(uint32_t *)dest = val;
	return (type && ethnla_put_u32(msgbuff, type, val)) ? -EMSGSIZE : 0;
}

/* Parser handler for unsigned 32-bit integer. Expects a numeric argument
 * (may use 0x prefix), generates NLA_U32 or fills an uint32_t.
 */
int nl_parse_direct_u8(struct nl_context *nlctx, uint16_t type,
		       const void *data __maybe_unused,
		       struct nl_msg_buff *msgbuff, void *dest)
{
	const char *arg = *nlctx->argp;
	uint8_t val;
	int ret;

	nlctx->argp++;
	nlctx->argc--;
	ret = parse_u8(arg, &val);
	if (ret < 0) {
		parser_err_invalid_value(nlctx, arg);
		return ret;
	}

	if (dest)
		*(uint8_t *)dest = val;
	return (type && ethnla_put_u8(msgbuff, type, val)) ? -EMSGSIZE : 0;
}

/* Parser handler for float meters and convert it to cm. Generates
 * NLA_U32 or fills an uint32_t.
 */
int nl_parse_direct_m2cm(struct nl_context *nlctx, uint16_t type,
			 const void *data __maybe_unused,
			 struct nl_msg_buff *msgbuff, void *dest)
{
	const char *arg = *nlctx->argp;
	float meters = 0.0;
	uint32_t cm;
	int ret;

	nlctx->argp++;
	nlctx->argc--;
	ret = parse_float(arg, &meters, 0, 150);
	if (ret < 0) {
		parser_err_invalid_value(nlctx, arg);
		return ret;
	}

	cm = (uint32_t)(meters * 100 + 0.5);
	if (dest)
		*(uint32_t *)dest = cm;
	return (type && ethnla_put_u32(msgbuff, type, cm)) ? -EMSGSIZE : 0;
}

/* Parser handler for (tri-state) bool. Expects "name on|off", generates
 * NLA_U8 which is 1 for "on" and 0 for "off".
 */
int nl_parse_u8bool(struct nl_context *nlctx, uint16_t type,
		    const void *data __maybe_unused,
		    struct nl_msg_buff *msgbuff, void *dest)
{
	const char *arg = *nlctx->argp;
	int ret;

	nlctx->argp++;
	nlctx->argc--;
	if (!strcmp(arg, "on")) {
		if (dest)
			*(uint8_t *)dest = 1;
		ret = type ? ethnla_put_u8(msgbuff, type, 1) : 0;
	} else if (!strcmp(arg, "off")) {
		if (dest)
			*(uint8_t *)dest = 0;
		ret = type ? ethnla_put_u8(msgbuff, type, 0) : 0;
	} else {
		parser_err_invalid_value(nlctx, arg);
		return -EINVAL;
	}

	return ret ? -EMSGSIZE : 0;
}

/* Parser handler for 32-bit lookup value. Expects a string argument, looks it
 * up in a table, generates NLA_U32 or fills uint32_t variable. The @data
 * parameter is a null terminated array of struct lookup_entry_u32.
 */
int nl_parse_lookup_u32(struct nl_context *nlctx, uint16_t type,
			const void *data, struct nl_msg_buff *msgbuff,
			void *dest)
{
	const char *arg = *nlctx->argp;
	uint32_t val;
	int ret;

	nlctx->argp++;
	nlctx->argc--;
	ret = lookup_u32(arg, &val, data);
	if (ret < 0) {
		parser_err_invalid_value(nlctx, arg);
		return ret;
	}

	if (dest)
		*(uint32_t *)dest = val;
	return (type && ethnla_put_u32(msgbuff, type, val)) ? -EMSGSIZE : 0;
}

/* Parser handler for 8-bit lookup value. Expects a string argument, looks it
 * up in a table, generates NLA_U8 or fills uint8_t variable. The @data
 * parameter is a null terminated array of struct lookup_entry_u8.
 */
int nl_parse_lookup_u8(struct nl_context *nlctx, uint16_t type,
		       const void *data, struct nl_msg_buff *msgbuff,
		       void *dest)
{
	const char *arg = *nlctx->argp;
	uint8_t val;
	int ret;

	nlctx->argp++;
	nlctx->argc--;
	ret = lookup_u8(arg, &val, data);
	if (ret < 0) {
		parser_err_invalid_value(nlctx, arg);
		return ret;
	}

	if (dest)
		*(uint8_t *)dest = val;
	return (type && ethnla_put_u8(msgbuff, type, val)) ? -EMSGSIZE : 0;
}

/* number of significant bits */
static unsigned int __nsb(uint32_t x)
{
	unsigned int ret = 0;

	if (x & 0xffff0000U) {
		x >>= 16;
		ret += 16;
	}
	if (x & 0xff00U) {
		x >>= 8;
		ret += 8;
	}
	if (x & 0xf0U) {
		x >>= 4;
		ret += 4;
	}
	if (x & 0xcU) {
		x >>= 2;
		ret += 2;
	}
	if (x & 0x2U) {
		x >>= 1;
		ret += 1;
	}

	return ret + x;
}

static bool __is_hex(char c)
{
	if (isdigit(c))
		return true;
	else
		return (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static unsigned int __hex_val(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 0xa;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 0xa;
	return 0;
}

static bool __bytestr_delim(const char *p, char delim)
{
	return !*p || (delim ? (*p == delim) : !__is_hex(*p));
}

/* Parser handler for generic byte string in MAC-like format. Expects string
 * argument in the "[[:xdigit:]]{2}(:[[:xdigit:]]{2})*" format, generates
 * NLA_BINARY or fills a struct byte_str_value (if @dest is not null and the
 * handler succeeds, caller is responsible for freeing the value). The @data
 * parameter points to struct byte_str_parser_data.
 */
int nl_parse_byte_str(struct nl_context *nlctx, uint16_t type, const void *data,
		      struct nl_msg_buff *msgbuff, void *dest)
{
	const struct byte_str_parser_data *pdata = data;
	struct byte_str_value *dest_value = dest;
	const char *arg = *nlctx->argp;
	uint8_t *val = NULL;
	unsigned int len, i;
	const char *p;
	int ret;

	nlctx->argp++;
	nlctx->argc--;

	len = 0;
	p = arg;
	if (!*p)
		goto err;
	while (true) {
		len++;
		if (!__bytestr_delim(p, pdata->delim))
			p++;
		if (!__bytestr_delim(p, pdata->delim))
			p++;
		if (!__bytestr_delim(p, pdata->delim))
			goto err;
		if (!*p)
			break;
		p++;
		if (*p && __bytestr_delim(p, pdata->delim))
			goto err;
	}
	if (len < pdata->min_len || (pdata->max_len && len > pdata->max_len))
		goto err;
	val = malloc(len);
	if (!val)
		return -ENOMEM;

	p = arg;
	for (i = 0; i < len; i++) {
		uint8_t byte = 0;

		if (!__is_hex(*p))
			goto err;
		while (__is_hex(*p))
			byte = 16 * byte + __hex_val(*p++);
		if (!__bytestr_delim(p, pdata->delim))
			goto err;
		val[i] = byte;
		if (*p)
			p++;
	}
	ret = type ? ethnla_put(msgbuff, type, len, val) : 0;
	if (dest) {
		dest_value->len = len;
		dest_value->data = val;
	} else {
		free(val);
	}
	return ret;

err:
	free(val);
	fprintf(stderr, "ethtool (%s): invalid value '%s' of parameter '%s'\n",
		nlctx->cmd, arg, nlctx->param);
	return -EINVAL;
}

/* Parser handler for parameters recognized for backward compatibility but
 * supposed to fail without passing to kernel. Does not generate any netlink
 * attributes of fill any variable. The @data parameter points to struct
 * error_parser_params (error message, return value and number of extra
 * arguments to skip).
 */
int nl_parse_error(struct nl_context *nlctx, uint16_t type __maybe_unused,
		   const void *data, struct nl_msg_buff *msgbuff __maybe_unused,
		   void *dest __maybe_unused)
{
	const struct error_parser_data *parser_data = data;
	unsigned int skip = parser_data->extra_args;

	fprintf(stderr, "ethtool (%s): ", nlctx->cmd);
	fprintf(stderr, parser_data->err_msg, nlctx->param);
	if (nlctx->argc < skip) {
		fprintf(stderr, "ethtool (%s): too few arguments for parameter '%s' (expected %u)\n",
			nlctx->cmd, nlctx->param, skip);
	} else {
		nlctx->argp += skip;
		nlctx->argc -= skip;
	}

	return parser_data->ret_val;
}

/* bitset parser handlers */

/* Return true if a bitset argument should be parsed as numeric, i.e.
 * (a) it starts with '0x'
 * (b) it consists only of hex digits and at most one slash which can be
 *     optionally followed by "0x"; if no_mask is true, slash is not allowed
 */
static bool is_numeric_bitset(const char *arg, bool no_mask)
{
	const char *p = arg;
	bool has_slash = false;

	if (!arg)
		return false;
	if (__prefix_0x(arg))
		return true;
	while (*p) {
		if (*p == '/') {
			if (has_slash || no_mask)
				return false;
			has_slash = true;
			p++;
			if (__prefix_0x(p))
				p += 2;
			continue;
		}
		if (!__is_hex(*p))
			return false;
		p++;
	}
	return true;
}

#define __MAX_U32_DIGITS 10

/* Parse hex string (without leading "0x") into a bitmap consisting of 32-bit
 * words. Caller must make sure arg is at least len characters long and dst has
 * place for at least (len + 7) / 8 32-bit words. If force_hex is false, allow
 * also base 10 unsigned 32-bit value.
 *
 * Returns number of significant bits in the bitmap on success and negative
 * value on error.
 */
static int __parse_num_string(const char *arg, unsigned int len, uint32_t *dst,
			      bool force_hex)
{
	char buff[__MAX_U32_DIGITS + 1] = {};
	unsigned int nbits = 0;
	const char *p = arg;

	if (!len)
		return -EINVAL;
	if (!force_hex && len <= __MAX_U32_DIGITS) {
		strncpy(buff, arg, len);
		if (!buff[__MAX_U32_DIGITS]) {
			u32 val;
			int ret;

			ret = parse_u32d(buff, &val);
			if (!ret) {
				*dst = val;
				return __nsb(val);
			}
		}
	}

	dst += (len - 1) / 8;
	while (len > 0) {
		unsigned int chunk = (len % 8) ?: 8;
		unsigned long val;
		char *endp;

		memcpy(buff, p, chunk);
		buff[chunk] = '\0';
		val = strtoul(buff, &endp, 16);
		if (*endp)
			return -EINVAL;
		*dst-- = (uint32_t)val;
		if (nbits)
			nbits += 4 * chunk;
		else
			nbits = __nsb(val);

		p += chunk;
		len -= chunk;
	}
	return nbits;
}

/* Parse bitset provided as a base 16 numeric value (@no_mask is true) or pair
 * of base 16 numeric values  separated by '/' (@no_mask is false). The "0x"
 * prefix is optional. Generates bitset nested attribute in compact form.
 */
static int parse_numeric_bitset(struct nl_context *nlctx, uint16_t type,
				bool no_mask, bool force_hex,
				struct nl_msg_buff *msgbuff)
{
	unsigned int nwords, len1, len2;
	const char *arg = *nlctx->argp;
	bool force_hex1 = force_hex;
	bool force_hex2 = force_hex;
	uint32_t *value = NULL;
	uint32_t *mask = NULL;
	struct nlattr *nest;
	const char *maskptr;
	int ret = 0;
	int nbits;

	if (__prefix_0x(arg)) {
		force_hex1 = true;
		arg += 2;
	}

	maskptr = strchr(arg, '/');
	if (maskptr && no_mask) {
		parser_err_invalid_value(nlctx, arg);
		return -EINVAL;
	}
	len1 = maskptr ? (unsigned int)(maskptr - arg) : strlen(arg);
	nwords = DIV_ROUND_UP(len1, 8);
	nbits = 0;

	if (maskptr) {
		maskptr++;
		if (__prefix_0x(maskptr)) {
			maskptr += 2;
			force_hex2 = true;
		}
		len2 = strlen(maskptr);
		if (len2 > len1)
			nwords = DIV_ROUND_UP(len2, 8);
		mask = calloc(nwords, sizeof(uint32_t));
		if (!mask)
			return -ENOMEM;
		ret = __parse_num_string(maskptr, strlen(maskptr), mask,
					 force_hex2);
		if (ret < 0) {
			parser_err_invalid_value(nlctx, arg);
			goto out_free;
		}
		nbits = ret;
	}

	value = calloc(nwords, sizeof(uint32_t));
	if (!value) {
		free(mask);
		return -ENOMEM;
	}
	ret = __parse_num_string(arg, len1, value, force_hex1);
	if (ret < 0) {
		parser_err_invalid_value(nlctx, arg);
		goto out_free;
	}
	nbits = (nbits < ret) ? ret : nbits;
	nwords = (nbits + 31) / 32;

	ret = 0;
	if (!type)
		goto out_free;
	ret = -EMSGSIZE;
	nest = ethnla_nest_start(msgbuff, type);
	if (!nest)
	       goto out_free;
	if (ethnla_put_flag(msgbuff, ETHTOOL_A_BITSET_NOMASK, !mask) ||
	    ethnla_put_u32(msgbuff, ETHTOOL_A_BITSET_SIZE, nbits) ||
	    ethnla_put(msgbuff, ETHTOOL_A_BITSET_VALUE,
		       nwords * sizeof(uint32_t), value) ||
	    (mask &&
	     ethnla_put(msgbuff, ETHTOOL_A_BITSET_MASK,
			nwords * sizeof(uint32_t), mask)))
		goto out_free;
	ethnla_nest_end(msgbuff, nest);
	ret = 0;

out_free:
	free(value);
	free(mask);
	nlctx->argp++;
	nlctx->argc--;
	return ret;
}

/* Parse bitset provided as series of "name on|off" pairs (@no_mask is false)
 * or names (@no_mask is true). Generates bitset nested attribute in verbose
 * form with names from command line.
 */
static int parse_name_bitset(struct nl_context *nlctx, uint16_t type,
			     bool no_mask, struct nl_msg_buff *msgbuff)
{
	struct nlattr *bitset_attr;
	struct nlattr *bits_attr;
	struct nlattr *bit_attr;
	int ret;

	bitset_attr = ethnla_nest_start(msgbuff, type);
	if (!bitset_attr)
		return -EMSGSIZE;
	ret = -EMSGSIZE;
	if (no_mask && ethnla_put_flag(msgbuff, ETHTOOL_A_BITSET_NOMASK, true))
		goto err;
	bits_attr = ethnla_nest_start(msgbuff, ETHTOOL_A_BITSET_BITS);
	if (!bits_attr)
		goto err;

	while (nlctx->argc > 0) {
		bool bit_val = true;

		if (!strcmp(*nlctx->argp, "--")) {
			nlctx->argp++;
			nlctx->argc--;
			break;
		}
		ret = -EINVAL;
		if (!no_mask) {
			if (nlctx->argc < 2 ||
			    (strcmp(nlctx->argp[1], "on") &&
			     strcmp(nlctx->argp[1], "off"))) {
				parser_err_invalid_flag(nlctx, *nlctx->argp);
				goto err;
			}
			bit_val = !strcmp(nlctx->argp[1], "on");
		}

		ret = -EMSGSIZE;
		bit_attr = ethnla_nest_start(msgbuff,
					     ETHTOOL_A_BITSET_BITS_BIT);
		if (!bit_attr)
			goto err;
		if (ethnla_put_strz(msgbuff, ETHTOOL_A_BITSET_BIT_NAME,
				    nlctx->argp[0]))
			goto err;
		if (!no_mask &&
		    ethnla_put_flag(msgbuff, ETHTOOL_A_BITSET_BIT_VALUE,
				    bit_val))
			goto err;
		ethnla_nest_end(msgbuff, bit_attr);

		nlctx->argp += (no_mask ? 1 : 2);
		nlctx->argc -= (no_mask ? 1 : 2);
	}

	ethnla_nest_end(msgbuff, bits_attr);
	ethnla_nest_end(msgbuff, bitset_attr);
	return 0;
err:
	ethnla_nest_cancel(msgbuff, bitset_attr);
	return ret;
}

static bool is_char_bitset(const char *arg,
			   const struct char_bitset_parser_data *data)
{
	bool mask = (arg[0] == '+' || arg[0] == '-');
	unsigned int i;
	const char *p;

	for (p = arg; *p; p++) {
		if (*p == data->reset_char)
			continue;
		if (mask && (*p == '+' || *p == '-'))
			continue;
		for (i = 0; i < data->nbits; i++)
			if (*p == data->bit_chars[i])
				goto found;
		return false;
found:
		;
	}

	return true;
}

/* Parse bitset provided as a string consisting of characters corresponding to
 * bit indices. The "reset character" resets the no-mask bitset to empty. If
 * the first character is '+' or '-', generated bitset has mask and '+' and
 * '-' switch between enabling and disabling the following bits (i.e. their
 * value being true/false). In such case, "reset character" is not allowed.
 */
static int parse_char_bitset(struct nl_context *nlctx, uint16_t type,
			     const struct char_bitset_parser_data *data,
			     struct nl_msg_buff *msgbuff)
{
	const char *arg = *nlctx->argp;
	struct nlattr *bitset_attr;
	struct nlattr *saved_pos;
	struct nlattr *bits_attr;
	struct nlattr *bit_attr;
	unsigned int idx;
	bool val = true;
	const char *p;
	bool no_mask;
	int ret;

	no_mask = data->no_mask || !(arg[0] == '+' || arg[0] == '-');
	bitset_attr = ethnla_nest_start(msgbuff, type);
	if (!bitset_attr)
		return -EMSGSIZE;
	ret = -EMSGSIZE;
	if (no_mask && ethnla_put_flag(msgbuff, ETHTOOL_A_BITSET_NOMASK, true))
		goto err;
	bits_attr = ethnla_nest_start(msgbuff, ETHTOOL_A_BITSET_BITS);
	if (!bits_attr)
		goto err;
	saved_pos = mnl_nlmsg_get_payload_tail(msgbuff->nlhdr);

	for (p = arg; *p; p++) {
		if (*p == '+' || *p == '-') {
			if (no_mask) {
				parser_err_invalid_value(nlctx, arg);
				ret = -EINVAL;
				goto err;
			}
			val = (*p == '+');
			continue;
		}
		if (*p == data->reset_char) {
			if (no_mask) {
				mnl_attr_nest_cancel(msgbuff->nlhdr, saved_pos);
				continue;
			}
			fprintf(stderr, "ethtool (%s): invalid char '%c' in '%s' for parameter '%s'\n",
				nlctx->cmd, *p, arg, nlctx->param);
			ret = -EINVAL;
			goto err;
		}

		for (idx = 0; idx < data->nbits; idx++) {
			if (data->bit_chars[idx] == *p)
				break;
		}
		if (idx >= data->nbits) {
			fprintf(stderr, "ethtool (%s): invalid char '%c' in '%s' for parameter '%s'\n",
				nlctx->cmd, *p, arg, nlctx->param);
			ret = -EINVAL;
			goto err;
		}
		bit_attr = ethnla_nest_start(msgbuff,
					     ETHTOOL_A_BITSET_BITS_BIT);
		if (!bit_attr)
			goto err;
		if (ethnla_put_u32(msgbuff, ETHTOOL_A_BITSET_BIT_INDEX, idx))
			goto err;
		if (!no_mask &&
		    ethnla_put_flag(msgbuff, ETHTOOL_A_BITSET_BIT_VALUE, val))
			goto err;
		ethnla_nest_end(msgbuff, bit_attr);
	}

	ethnla_nest_end(msgbuff, bits_attr);
	ethnla_nest_end(msgbuff, bitset_attr);
	nlctx->argp++;
	nlctx->argc--;
	return 0;
err:
	ethnla_nest_cancel(msgbuff, bitset_attr);
	return ret;
}

/* Parser handler for bitset. Expects either a numeric value (base 16 or 10
 * (unless force_hex is set)), optionally followed by '/' and another numeric
 * value (mask, unless no_mask is set), or a series of "name on|off" pairs
 * (no_mask not set) or names (no_mask set). In the latter case, names are
 * passed on as they are and kernel performs their interpretation and
 * validation. The @data parameter points to struct bitset_parser_data.
 * Generates only a bitset nested attribute. Fails if @type is zero or @dest
 * is not null.
 */
int nl_parse_bitset(struct nl_context *nlctx, uint16_t type, const void *data,
		    struct nl_msg_buff *msgbuff, void *dest)
{
	const struct bitset_parser_data *parser_data = data;

	if (!type || dest) {
		fprintf(stderr, "ethtool (%s): internal error parsing '%s'\n",
			nlctx->cmd, nlctx->param);
		return -EFAULT;
	}
	if (is_numeric_bitset(*nlctx->argp, false))
		return parse_numeric_bitset(nlctx, type, parser_data->no_mask,
					    parser_data->force_hex, msgbuff);
	else
		return parse_name_bitset(nlctx, type, parser_data->no_mask,
					 msgbuff);
}

/* Parser handler for bitset. Expects either a numeric value (base 10 or 16),
 * optionally followed by '/' and another numeric value (mask, unless no_mask
 * is set), or a string consisting of characters corresponding to bit indices.
 * The @data parameter points to struct char_bitset_parser_data. Generates
 * biset nested attribute. Fails if type is zero or if @dest is not null.
 */
int nl_parse_char_bitset(struct nl_context *nlctx, uint16_t type,
			 const void *data, struct nl_msg_buff *msgbuff,
			 void *dest)
{
	const struct char_bitset_parser_data *parser_data = data;

	if (!type || dest) {
		fprintf(stderr, "ethtool (%s): internal error parsing '%s'\n",
			nlctx->cmd, nlctx->param);
		return -EFAULT;
	}
	if (is_char_bitset(*nlctx->argp, data) ||
	    !is_numeric_bitset(*nlctx->argp, false))
		return parse_char_bitset(nlctx, type, parser_data, msgbuff);
	else
		return parse_numeric_bitset(nlctx, type, parser_data->no_mask,
					    false, msgbuff);
}

/* parser implementation */

static const struct param_parser *find_parser(const struct param_parser *params,
					      const char *arg)
{
	const struct param_parser *parser;

	for (parser = params; parser->arg; parser++)
		if (!strcmp(arg, parser->arg))
			return parser;
	return NULL;
}

static bool __parser_bit(const uint64_t *map, unsigned int idx)
{
	return map[idx / 64] & (1 << (idx % 64));
}

static void __parser_set(uint64_t *map, unsigned int idx)
{
	map[idx / 64] |= (1 << (idx % 64));
}

static void __parser_set_group(const struct param_parser *params,
			       uint64_t *map, unsigned int alt_group)
{
	const struct param_parser *parser;
	unsigned int idx = 0;

	for (parser = params; parser->arg; parser++, idx++)
		if (parser->alt_group == alt_group)
			__parser_set(map, idx);
}

struct tmp_buff {
	struct nl_msg_buff	*msgbuff;
	unsigned int		id;
	unsigned int		orig_len;
	struct tmp_buff		*next;
};

static struct tmp_buff *tmp_buff_find(struct tmp_buff *head, unsigned int id)
{
	struct tmp_buff *buff;

	for (buff = head; buff; buff = buff->next)
		if (buff->id == id)
			break;

	return buff;
}

static struct tmp_buff *tmp_buff_find_or_create(struct tmp_buff **phead,
						unsigned int id)
{
	struct tmp_buff **pbuff;
	struct tmp_buff *new_buff;

	for (pbuff = phead; *pbuff; pbuff = &(*pbuff)->next)
		if ((*pbuff)->id == id)
			return *pbuff;

	new_buff = malloc(sizeof(*new_buff));
	if (!new_buff)
		return NULL;
	new_buff->id = id;
	new_buff->msgbuff = malloc(sizeof(*new_buff->msgbuff));
	if (!new_buff->msgbuff) {
		free(new_buff);
		return NULL;
	}
	msgbuff_init(new_buff->msgbuff);
	new_buff->next = NULL;
	*pbuff = new_buff;

	return new_buff;
}

static void tmp_buff_destroy(struct tmp_buff *head)
{
	struct tmp_buff *buff = head;
	struct tmp_buff *next;

	while (buff) {
		next = buff->next;
		if (buff->msgbuff) {
			msgbuff_done(buff->msgbuff);
			free(buff->msgbuff);
		}
		free(buff);
		buff = next;
	}
}

/* Main entry point of parser implementation.
 * @nlctx: netlink context
 * @params:      array of struct param_parser describing expected arguments
 *               and their handlers; the array must be terminated by null
 *               element {}
 * @dest:        optional destination to copy parsed data to (at
 *               param_parser::offset)
 * @group_style: defines if identifiers in .group represent separate messages,
 *               nested attributes or are not allowed
 * @msgbuffs:    (only used for @group_style = PARSER_GROUP_MSG) array to store
 *               pointers to composed messages; caller must make sure this
 *               array is sufficient, i.e. that it has at least as many entries
 *               as the number of different .group values in params array;
 *               entries are filled from the start, remaining entries are not
 *               modified; caller should zero initialize the array before
 *               calling nl_parser()
 */
int nl_parser(struct nl_context *nlctx, const struct param_parser *params,
	      void *dest, enum parser_group_style group_style,
	      struct nl_msg_buff **msgbuffs)
{
	struct nl_socket *nlsk = nlctx->ethnl_socket;
	const struct param_parser *parser;
	struct tmp_buff *buffs = NULL;
	unsigned int n_msgbuffs = 0;
	struct tmp_buff *buff;
	unsigned int n_params;
	uint64_t *params_seen;
	int ret;

	n_params = 0;
	for (parser = params; parser->arg; parser++) {
		struct nl_msg_buff *msgbuff;
		struct nlattr *nest;

		n_params++;
		if (group_style == PARSER_GROUP_NONE || !parser->group)
			continue;
		ret = -ENOMEM;
		buff = tmp_buff_find_or_create(&buffs, parser->group);
		if (!buff)
			goto out_free_buffs;
		msgbuff = buff->msgbuff;
		ret = msg_init(nlctx, msgbuff, parser->group,
			       NLM_F_REQUEST | NLM_F_ACK);
		if (ret < 0)
			goto out_free_buffs;

		switch (group_style) {
		case PARSER_GROUP_NEST:
			ret = -EMSGSIZE;
			nest = ethnla_nest_start(buff->msgbuff, parser->group);
			if (!nest)
				goto out_free_buffs;
			break;
		case PARSER_GROUP_MSG:
			if (ethnla_fill_header(msgbuff,
					       ETHTOOL_A_LINKINFO_HEADER,
					       nlctx->devname, 0))
				goto out_free_buffs;
			break;
		default:
			break;
		}

		buff->orig_len = msgbuff_len(msgbuff);
	}
	ret = -ENOMEM;
	params_seen = calloc(DIV_ROUND_UP(n_params, 64), sizeof(uint64_t));
	if (!params_seen)
		goto out_free_buffs;

	while (nlctx->argc > 0) {
		struct nl_msg_buff *msgbuff;
		void *param_dest;

		nlctx->param = *nlctx->argp;
		ret = -EINVAL;
		parser = find_parser(params, nlctx->param);
		if (!parser) {
			parser_err_unknown_param(nlctx);
			goto out_free;
		}

		/* check duplicates and minimum number of arguments */
		if (__parser_bit(params_seen, parser - params)) {
			parser_err_dup_param(nlctx);
			goto out_free;
		}
		nlctx->argc--;
		nlctx->argp++;
		if (nlctx->argc < parser->min_argc) {
			parser_err_min_argc(nlctx, parser->min_argc);
			goto out_free;
		}
		if (parser->alt_group)
			__parser_set_group(params, params_seen,
					   parser->alt_group);
		else
			__parser_set(params_seen, parser - params);

		buff = NULL;
		if (parser->group)
			buff = tmp_buff_find(buffs, parser->group);
		msgbuff = buff ? buff->msgbuff : &nlsk->msgbuff;

		param_dest = dest ? ((char *)dest + parser->dest_offset) : NULL;
		ret = parser->handler(nlctx, parser->type, parser->handler_data,
				      msgbuff, param_dest);
		if (ret < 0)
			goto out_free;
	}

	if (group_style == PARSER_GROUP_MSG) {
		ret = -EOPNOTSUPP;
		for (buff = buffs; buff; buff = buff->next)
			if (msgbuff_len(buff->msgbuff) > buff->orig_len &&
			    netlink_cmd_check(nlctx->ctx, buff->id, false))
				goto out_free;
	}
	for (buff = buffs; buff; buff = buff->next) {
		struct nl_msg_buff *msgbuff = buff->msgbuff;

		if (group_style == PARSER_GROUP_NONE ||
		    msgbuff_len(msgbuff) == buff->orig_len)
			continue;
		switch (group_style) {
		case PARSER_GROUP_NEST:
			ethnla_nest_end(msgbuff, msgbuff->payload);
			ret = msgbuff_append(&nlsk->msgbuff, msgbuff);
			if (ret < 0)
				goto out_free;
			break;
		case PARSER_GROUP_MSG:
			msgbuffs[n_msgbuffs++] = msgbuff;
			buff->msgbuff = NULL;
			break;
		default:
			break;
		}
	}

	ret = 0;
out_free:
	free(params_seen);
out_free_buffs:
	tmp_buff_destroy(buffs);
	return ret;
}
