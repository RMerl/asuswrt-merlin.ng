/*
 *   This program is is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License, version 2 if the
 *   License as published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/**
 * $Id$
 * @file rlm_expr.c
 * @brief Register many xlat expansions including the expr expansion.
 *
 * @copyright 2001,2006  The FreeRADIUS server project
 * @copyright 2002  Alan DeKok <aland@ox.org>
 */
RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/md5.h>
#include <freeradius-devel/sha1.h>
#include <freeradius-devel/base64.h>
#include <freeradius-devel/modules.h>

#include <ctype.h>

#include "rlm_expr.h"

/*
 *	Define a structure for our module configuration.
 */
typedef struct rlm_expr_t {
	char const *xlat_name;
	char *allowed_chars;
} rlm_expr_t;

static const CONF_PARSER module_config[] = {
	{"safe_characters", PW_TYPE_STRING_PTR,
	 offsetof(rlm_expr_t, allowed_chars), NULL,
	"@abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_: /"},
	{NULL, -1, 0, NULL, NULL}
};

typedef enum expr_token_t {
  TOKEN_NONE = 0,
  TOKEN_INTEGER,
  TOKEN_ADD,
  TOKEN_SUBTRACT,
  TOKEN_DIVIDE,
  TOKEN_REMAINDER,
  TOKEN_MULTIPLY,
  TOKEN_AND,
  TOKEN_OR,
  TOKEN_LAST
} expr_token_t;

typedef struct expr_map_t {
	char op;
	expr_token_t token;
} expr_map_t;

static expr_map_t map[] =
{
	{'+',	TOKEN_ADD },
	{'-',	TOKEN_SUBTRACT },
	{'/',	TOKEN_DIVIDE },
	{'*',	TOKEN_MULTIPLY },
	{'%',	TOKEN_REMAINDER },
	{'&',	TOKEN_AND },
	{'|',	TOKEN_OR },
	{0,	TOKEN_LAST}
};

/*
 *	Lookup tables for randstr char classes
 */
static char randstr_punc[] = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
static char randstr_salt[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmopqrstuvwxyz/.";

static int get_number(REQUEST *request, char const **string, int64_t *answer)
{
	int		i, found;
	int64_t		result;
	int64_t		x;
	char const 	*p;
	expr_token_t	this;

	/*
	 *  Loop over the input.
	 */
	result = 0;
	this = TOKEN_NONE;

	for (p = *string; *p != '\0'; /* nothing */) {
		if ((*p == ' ') ||
		    (*p == '\t')) {
			p++;
			continue;
		}

		/*
		 *  Discover which token it is.
		 */
		found = false;
		for (i = 0; map[i].token != TOKEN_LAST; i++) {
			if (*p == map[i].op) {
				if (this != TOKEN_NONE) {
					RDEBUG2("Invalid operator at \"%s\"", p);
					return -1;
				}
				this = map[i].token;
				p++;
				found = true;
				break;
			}
		}

		/*
		 *  Found the algebraic operator.  Get the next number.
		 */
		if (found) {
			continue;
		}

		/*
		 *  End of a group.  Stop.
		 */
		if (*p == ')') {
			if (this != TOKEN_NONE) {
				RDEBUG2("Trailing operator before end sub-expression at \"%s\"", p);
				return -1;
			}
			p++;
			break;
		}

		/*
		 *  Start of a group.  Call ourselves recursively.
		 */
		if (*p == '(') {
			p++;

			found = get_number(request, &p, &x);
			if (found < 0) {
				return -1;
			}
		} else {
			/*
			 *  No algrebraic operator found, the next thing
			 *  MUST be a number.
			 *
			 *  If it isn't, then we die.
			 */
			if ((*p == '0') && (p[1] == 'x')) {
				char *end;

				x = strtoul(p, &end, 16);
				p = end;
				goto calc;
			}


			if ((*p < '0') || (*p > '9')) {
				RDEBUG2("Not a number at \"%s\"", p);
				return -1;
			}

			/*
			 *  This is doing it the hard way, but it also allows
			 *  us to increment 'p'.
			 */
			x = 0;
			while ((*p >= '0') && (*p <= '9')) {
				x *= 10;
				x += (*p - '0');
				p++;
			}
		}

	calc:
		switch (this) {
		default:
		case TOKEN_NONE:
			result = x;
			break;

		case TOKEN_ADD:
			result += x;
			break;

		case TOKEN_SUBTRACT:
			result -= x;
			break;

		case TOKEN_DIVIDE:
			if (x == 0) {
				result = 0; /* we don't have NaN for integers */
				break;
			}
			result /= x;
			break;

		case TOKEN_REMAINDER:
			if (x == 0) {
				result = 0; /* we don't have NaN for integers */
				break;
			}
			result %= x;
			break;

		case TOKEN_MULTIPLY:
			result *= x;
			break;

		case TOKEN_AND:
			result &= x;
			break;

		case TOKEN_OR:
			result |= x;
			break;
		}

		/*
		 *  We've used this token.
		 */
		this = TOKEN_NONE;
	}

	/*
	 *  And return the answer to the caller.
	 */
	*string = p;
	*answer = result;
	return 0;
}

/*
 *  Do xlat of strings!
 */
static ssize_t expr_xlat(UNUSED void *instance, REQUEST *request, char const *fmt,
			 char *out, size_t outlen)
{
	int		rcode;
	int64_t		result;
	char const 	*p;

	p = fmt;
	rcode = get_number(request, &p, &result);
	if (rcode < 0) {
		return -1;
	}

	/*
	 *  We MUST have eaten the entire input string.
	 */
	if (*p != '\0') {
		RDEBUG2("Failed at %s", p);
		return -1;
	}

	snprintf(out, outlen, "%ld", (long int) result);
	return strlen(out);
}

/**
 *  @brief Generate a random integer value
 *
 */
static ssize_t rand_xlat(UNUSED void *instance, UNUSED REQUEST *request, char const *fmt,
			 char *out, size_t outlen)
{
	int64_t		result;

	result = atoi(fmt);

	/*
	 *	Too small or too big.
	 */
	if (result <= 0) {
		*out = '\0';
		return -1;
	}
	if (result >= (1 << 30)) result = (1 << 30);

	result *= fr_rand();	/* 0..2^32-1 */
	result >>= 32;

	snprintf(out, outlen, "%ld", (long int) result);
	return strlen(out);
}

/**
 *  @brief Generate a string of random chars
 *
 *  Build strings of random chars, useful for generating tokens and passcodes
 *  Format similar to String::Random.
 */
static ssize_t randstr_xlat(UNUSED void *instance, UNUSED REQUEST *request,
			    char const *fmt, char *out, size_t outlen)
{
	char const 	*p;
	unsigned int	result;
	size_t		freespace = outlen;

	if (outlen <= 1) return 0;

	*out = '\0';

	p = fmt;
	while (*p && (--freespace > 0)) {
		result = fr_rand();
		switch (*p) {
			/*
			 *  Lowercase letters
			 */
			case 'c':
				*out++ = 'a' + (result % 26);
				break;

			/*
			 *  Uppercase letters
			 */
			case 'C':
				*out++ = 'A' + (result % 26);
				break;

			/*
			 *  Numbers
			 */
			case 'n':
				*out++ = '0' + (result % 10);
				break;

			/*
			 *  Alpha numeric
			 */
			case 'a':
				*out++ = randstr_salt[result % (sizeof(randstr_salt) - 3)];
				break;

			/*
			 *  Punctuation
			 */
			case '!':
				*out++ = randstr_punc[result % (sizeof(randstr_punc) - 1)];
				break;

			/*
			 *  Alpa numeric + punctuation
			 */
			case '.':
				*out++ = '!' + (result % 95);
				break;

			/*
			 *  Alpha numeric + salt chars './'
			 */
			case 's':
				*out++ = randstr_salt[result % (sizeof(randstr_salt) - 1)];
				break;

			/*
			 *  Binary data as hexits (we don't really support
			 *  non printable chars).
			 */
			case 'h':
				if (freespace < 2) {
					break;
				}

				snprintf(out, 3, "%02x", result % 256);

				/* Already decremented */
				freespace -= 1;
				out += 2;
				break;

			default:
				ERROR("rlm_expr: invalid character class '%c'", *p);

				return -1;
		}

		p++;
	}

	*out++ = '\0';

	return outlen - freespace;
}

/**
 * @brief URLencode special characters
 *
 * Example: "%{urlquote:http://example.org/}" == "http%3A%47%47example.org%47"
 */
static ssize_t urlquote_xlat(UNUSED void *instance, UNUSED REQUEST *request,
			     char const *fmt, char *out, size_t outlen)
{
	char const 	*p;
	size_t	freespace = outlen;

	if (outlen <= 1) return 0;

	p = fmt;
	while (*p && (--freespace > 0)) {
		if (isalnum(*p)) {
			*out++ = *p++;
			continue;
		}

		switch (*p) {
			case '-':
			case '_':
			case '.':
			case '~':
				*out++ = *p++;
				break;
			default:
				if (freespace < 3)
					break;

				snprintf(out, 4, "%%%02x", *p++); /* %xx */

				/* Already decremented */
				freespace -= 2;
				out += 3;
		}
	}

	*out = '\0';

	return outlen - freespace;
}

/**
 * @brief Equivalent to the old safe_characters functionality in rlm_sql
 *
 * @verbatim Example: "%{escape:<img>foo.jpg</img>}" == "=60img=62foo.jpg=60=/img=62" @endverbatim
 */
static ssize_t escape_xlat(UNUSED void *instance, UNUSED REQUEST *request,
			   char const *fmt, char *out, size_t outlen)
{
	rlm_expr_t *inst = instance;
	char const 	*p;
	size_t	freespace = outlen;

	if (outlen <= 1) return 0;

	p = fmt;
	while (*p && (--freespace > 0)) {
		/*
		 *	Non-printable characters get replaced with their
		 *	mime-encoded equivalents.
		 */
		if ((*p > 31) && strchr(inst->allowed_chars, *p)) {
			*out++ = *p++;
			continue;
		}

		if (freespace < 3)
			break;

		snprintf(out, 4, "=%02X", *p++);

		/* Already decremented */
		freespace -= 2;
		out += 3;
	}

	*out = '\0';

	return outlen - freespace;
}

/**
 * @brief Convert a string to lowercase
 *
 * Example "%{lc:Bar}" == "bar"
 *
 * Probably only works for ASCII
 */
static ssize_t lc_xlat(UNUSED void *instance, UNUSED REQUEST *request,
		       char const *fmt, char *out, size_t outlen)
{
	char *q;
	char const *p;

	if (outlen <= 1) return 0;

	for (p = fmt, q = out; *p != '\0'; p++, outlen--) {
		if (outlen <= 1) break;

		*(q++) = tolower((int) *p);
	}

	*q = '\0';

	return strlen(out);
}

/**
 * @brief Convert a string to uppercase
 *
 * Example: "%{uc:Foo}" == "FOO"
 *
 * Probably only works for ASCII
 */
static ssize_t uc_xlat(UNUSED void *instance, UNUSED REQUEST *request,
		       char const *fmt, char *out, size_t outlen)
{
	char *q;
	char const *p;

	if (outlen <= 1) return 0;

	for (p = fmt, q = out; *p != '\0'; p++, outlen--) {
		if (outlen <= 1) break;

		*(q++) = toupper((int) *p);
	}

	*q = '\0';

	return strlen(out);
}

/**
 * @brief Calculate the MD5 hash of a string.
 *
 * Example: "%{md5:foo}" == "acbd18db4cc2f85cedef654fccc4a4d8"
 */
static ssize_t md5_xlat(UNUSED void *instance, UNUSED REQUEST *request,
		        char const *fmt, char *out, size_t outlen)
{
	uint8_t digest[16];
	int i, len;
	FR_MD5_CTX ctx;

	/*
	 *	We need room for at least one octet of output.
	 */
	if (outlen < 3) {
		*out = '\0';
		return 0;
	}

	fr_MD5Init(&ctx);
	fr_MD5Update(&ctx, (const void *) fmt, strlen(fmt));
	fr_MD5Final(digest, &ctx);

	/*
	 *	Each digest octet takes two hex digits, plus one for
	 *	the terminating NUL.
	 */
	len = (outlen / 2) - 1;
	if (len > 16) len = 16;

	for (i = 0; i < len; i++) {
		snprintf(out + i * 2, 3, "%02x", digest[i]);
	}

	return strlen(out);
}

/**
 * @brief Calculate the SHA1 hash of a string.
 *
 * Example: "%{sha1:foo}" == "0beec7b5ea3f0fdbc95d0dd47f3c5bc275da8a33"
 */
static ssize_t sha1_xlat(UNUSED void *instance, UNUSED REQUEST *request,
                         char const *fmt, char *out, size_t outlen)
{
        uint8_t digest[20];
        int i, len;
        fr_SHA1_CTX ctx;

        /*
         *      We need room for at least one octet of output.
         */
        if (outlen < 3) {
                *out = '\0';
                return 0;
        }

        fr_SHA1Init(&ctx);
        fr_SHA1Update(&ctx, (const void *) fmt, strlen(fmt));
        fr_SHA1Final(digest, &ctx);

        /*
         *      Each digest octet takes two hex digits, plus one for
         *      the terminating NUL. SHA1 is 160 bits (20 bytes)
         */
        len = (outlen / 2) - 1;
        if (len > 20) len = 20;

        for (i = 0; i < len; i++) {
                snprintf(out + i * 2, 3, "%02x", digest[i]);
        }

        return strlen(out);
}

/**
 * @brief Encode string as base64
 *
 * Example: "%{tobase64:foo}" == "Zm9v"
 */
static ssize_t base64_xlat(UNUSED void *instance, UNUSED REQUEST *request,
			   char const *fmt, char *out, size_t outlen)
{
	ssize_t len;

	len = strlen(fmt);

	/*
	 *  We can accurately calculate the length of the output string
	 *  if it's larger than outlen, the output would be useless so abort.
	 */
	if ((len < 0) || ((FR_BASE64_ENC_LENGTH(len) + 1) > (ssize_t) outlen)) {
		REDEBUG("xlat failed.");
		*out = '\0';
		return -1;
	}

	return fr_base64_encode((const uint8_t *) fmt, len, out, outlen);
}

/**
 * @brief Convert base64 to hex
 *
 * Example: "%{base64tohex:Zm9v}" == "666f6f"
 */
static ssize_t base64_to_hex_xlat(UNUSED void *instance, UNUSED REQUEST *request,
				  char const *fmt, char *out, size_t outlen)
{
	uint8_t decbuf[1024];

	ssize_t declen;
	ssize_t len = strlen(fmt);

	*out = '\0';

	declen = fr_base64_decode(fmt, len, decbuf, sizeof(decbuf));
	if (declen < 0) {
		REDEBUG("Base64 string invalid");
		return -1;
	}

	if ((size_t)((declen * 2) + 1) > outlen) {
		REDEBUG("Base64 conversion failed, output buffer exhausted, needed %zd bytes, have %zd bytes",
			(declen * 2) + 1, outlen);
	}

	return fr_bin2hex(out, decbuf, declen);
}


/*
 *	Do any per-module initialization that is separate to each
 *	configured instance of the module.  e.g. set up connections
 *	to external databases, read configuration files, set up
 *	dictionary entries, etc.
 *
 *	If configuration information is given in the config section
 *	that must be referenced in later calls, store a handle to it
 *	in *instance otherwise put a null pointer there.
 */
static int mod_instantiate(CONF_SECTION *conf, void *instance)
{
	rlm_expr_t *inst = instance;

	inst->xlat_name = cf_section_name2(conf);
	if (!inst->xlat_name) {
		inst->xlat_name = cf_section_name1(conf);
	}

	xlat_register(inst->xlat_name, expr_xlat, NULL, inst);

	/*
	 *	FIXME: unregister these, too
	 */
	xlat_register("rand", rand_xlat, NULL, inst);
	xlat_register("randstr", randstr_xlat, NULL, inst);
	xlat_register("urlquote", urlquote_xlat, NULL, inst);
	xlat_register("escape", escape_xlat, NULL, inst);
	xlat_register("tolower", lc_xlat, NULL, inst);
	xlat_register("toupper", uc_xlat, NULL, inst);
	xlat_register("md5", md5_xlat, NULL, inst);
	xlat_register("sha1", sha1_xlat, NULL, inst);
	xlat_register("tobase64", base64_xlat, NULL, inst);
	xlat_register("base64tohex", base64_to_hex_xlat, NULL, inst);

	/*
	 *	Initialize various paircompare functions
	 */
	pair_builtincompare_add(instance);
	return 0;
}

/*
 *	The module name should be the only globally exported symbol.
 *	That is, everything else should be 'static'.
 *
 *	If the module needs to temporarily modify it's instantiation
 *	data, the type should be changed to RLM_TYPE_THREAD_UNSAFE.
 *	The server will then take care of ensuring that the module
 *	is single-threaded.
 */
module_t rlm_expr = {
	RLM_MODULE_INIT,
	"expr",				/* Name */
	RLM_TYPE_CHECK_CONFIG_SAFE,   	/* type */
	sizeof(rlm_expr_t),
	module_config,
	mod_instantiate,		/* instantiation */
	NULL,				/* detach */
	{
		NULL,			/* authentication */
		NULL,			/* authorization */
		NULL,			/* pre-accounting */
		NULL			/* accounting */
	},
};
