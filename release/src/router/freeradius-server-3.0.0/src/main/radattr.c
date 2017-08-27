/*
 * radattr.c	RADIUS Attribute debugging tool.
 *
 * Version:	$Id$
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2010  Alan DeKok <aland@freeradius.org>
 */

RCSID("$Id$")

#include <freeradius-devel/libradius.h>
#include <freeradius-devel/parser.h>
#include <freeradius-devel/conf.h>
#include <freeradius-devel/radpaths.h>

#include <ctype.h>

#ifdef HAVE_GETOPT_H
#	include <getopt.h>
#endif

#include <assert.h>

typedef struct REQUEST REQUEST;

#include <freeradius-devel/log.h>
log_debug_t debug_flag = 0;

/**********************************************************************
 *	Hacks for xlat
 */
typedef size_t (*RADIUS_ESCAPE_STRING)(REQUEST *, char *out, size_t outlen, char const *in, void *arg);
typedef ssize_t (*RAD_XLAT_FUNC)(void *instance, REQUEST *, char const *, char *, size_t);
int            xlat_register(char const *module, RAD_XLAT_FUNC func, RADIUS_ESCAPE_STRING escape,
			     void *instance);
#include <sys/wait.h>
pid_t rad_fork(void);
pid_t rad_waitpid(pid_t pid, int *status);

pid_t rad_fork(void)
{
	return fork();
}

pid_t rad_waitpid(pid_t pid, int *status)
{
	return waitpid(pid, status, 0);
}

static ssize_t xlat_test(UNUSED void *instance, UNUSED REQUEST *request,
			 UNUSED char const *fmt, UNUSED char *out, UNUSED size_t outlen)
{
	return 0;
}

/*
 *	End of hacks for xlat
 *
 **********************************************************************/

static int encode_tlv(char *buffer, uint8_t *output, size_t outlen);

static char const *hextab = "0123456789abcdef";

static int encode_data_string(char *buffer,
			      uint8_t *output, size_t outlen)
{
	int length = 0;
	char *p;

	p = buffer + 1;

	while (*p && (outlen > 0)) {
		if (*p == '"') {
			return length;
		}

		if (*p != '\\') {
			*(output++) = *(p++);
			outlen--;
			length++;
			continue;
		}

		switch (p[1]) {
		default:
			*(output++) = p[1];
			break;

		case 'n':
			*(output++) = '\n';
			break;

		case 'r':
			*(output++) = '\r';
			break;

		case 't':
			*(output++) = '\t';
			break;
		}

		outlen--;
		length++;
	}

	fprintf(stderr, "String is not terminated\n");
	return 0;
}

static int encode_data_tlv(char *buffer, char **endptr,
			   uint8_t *output, size_t outlen)
{
	int depth = 0;
	int length;
	char *p;

	for (p = buffer; *p != '\0'; p++) {
		if (*p == '{') depth++;
		if (*p == '}') {
			depth--;
			if (depth == 0) break;
		}
	}

	if (*p != '}') {
		fprintf(stderr, "No trailing '}' in string starting "
			"with \"%s\"\n",
			buffer);
		return 0;
	}

	*endptr = p + 1;
	*p = '\0';

	p = buffer + 1;
	while (isspace((int) *p)) p++;

	length = encode_tlv(p, output, outlen);
	if (length == 0) return 0;

	return length;
}

static int encode_hex(char *p, uint8_t *output, size_t outlen)
{
	int length = 0;
	while (*p) {
		char *c1, *c2;

		while (isspace((int) *p)) p++;

		if (!*p) break;

		if(!(c1 = memchr(hextab, tolower((int) p[0]), 16)) ||
		   !(c2 = memchr(hextab, tolower((int)  p[1]), 16))) {
			fprintf(stderr, "Invalid data starting at "
				"\"%s\"\n", p);
			return 0;
		}

		*output = ((c1 - hextab) << 4) + (c2 - hextab);
		output++;
		length++;
		p += 2;

		outlen--;
		if (outlen == 0) {
			fprintf(stderr, "Too much data\n");
			return 0;
		}
	}

	return length;
}


static int encode_data(char *p, uint8_t *output, size_t outlen)
{
	int length;

	if (!isspace((int) *p)) {
		fprintf(stderr, "Invalid character following attribute "
			"definition\n");
		return 0;
	}

	while (isspace((int) *p)) p++;

	if (*p == '{') {
		int sublen;
		char *q;

		length = 0;

		do {
			while (isspace((int) *p)) p++;
			if (!*p) {
				if (length == 0) {
					fprintf(stderr, "No data\n");
					return 0;
				}

				break;
			}

			sublen = encode_data_tlv(p, &q, output, outlen);
			if (sublen == 0) return 0;

			length += sublen;
			output += sublen;
			outlen -= sublen;
			p = q;
		} while (*q);

		return length;
	}

	if (*p == '"') {
		length = encode_data_string(p, output, outlen);
		return length;
	}

	length = encode_hex(p, output, outlen);

	if (length == 0) {
		fprintf(stderr, "Empty string\n");
		return 0;
	}

	return length;
}

static int decode_attr(char *buffer, char **endptr)
{
	long attr;

	attr = strtol(buffer, endptr, 10);
	if (*endptr == buffer) {
		fprintf(stderr, "No valid number found in string "
			"starting with \"%s\"\n", buffer);
		return 0;
	}

	if (!**endptr) {
		fprintf(stderr, "Nothing follows attribute number\n");
		return 0;
	}

	if ((attr <= 0) || (attr > 256)) {
		fprintf(stderr, "Attribute number is out of valid "
			"range\n");
		return 0;
	}

	return (int) attr;
}

static int decode_vendor(char *buffer, char **endptr)
{
	long vendor;

	if (*buffer != '.') {
		fprintf(stderr, "Invalid separator before vendor id\n");
		return 0;
	}

	vendor = strtol(buffer + 1, endptr, 10);
	if (*endptr == (buffer + 1)) {
		fprintf(stderr, "No valid vendor number found\n");
		return 0;
	}

	if (!**endptr) {
		fprintf(stderr, "Nothing follows vendor number\n");
		return 0;
	}

	if ((vendor <= 0) || (vendor > (1 << 24))) {
		fprintf(stderr, "Vendor number is out of valid range\n");
		return 0;
	}

	if (**endptr != '.') {
		fprintf(stderr, "Invalid data following vendor number\n");
		return 0;
	}
	(*endptr)++;

	return (int) vendor;
}

static int encode_tlv(char *buffer, uint8_t *output, size_t outlen)
{
	int attr;
	int length;
	char *p;

	attr = decode_attr(buffer, &p);
	if (attr == 0) return 0;

	output[0] = attr;
	output[1] = 2;

	if (*p == '.') {
		p++;
		length = encode_tlv(p, output + 2, outlen - 2);

	} else {
		length = encode_data(p, output + 2, outlen - 2);
	}

	if (length == 0) return 0;
	if (length > (255 - 2)) {
		fprintf(stderr, "TLV data is too long\n");
		return 0;
	}

	output[1] += length;

	return length + 2;
}

static int encode_vsa(char *buffer, uint8_t *output, size_t outlen)
{
	int vendor;
	int length;
	char *p;

	vendor = decode_vendor(buffer, &p);
	if (vendor == 0) return 0;

	output[0] = 0;
	output[1] = (vendor >> 16) & 0xff;
	output[2] = (vendor >> 8) & 0xff;
	output[3] = vendor & 0xff;

	length = encode_tlv(p, output + 4, outlen - 4);
	if (length == 0) return 0;
	if (length > (255 - 6)) {
		fprintf(stderr, "VSA data is too long\n");
		return 0;
	}


	return length + 4;
}

static int encode_evs(char *buffer, uint8_t *output, size_t outlen)
{
	int vendor;
	int attr;
	int length;
	char *p;

	vendor = decode_vendor(buffer, &p);
	if (vendor == 0) return 0;

	attr = decode_attr(p, &p);
	if (attr == 0) return 0;

	output[0] = 0;
	output[1] = (vendor >> 16) & 0xff;
	output[2] = (vendor >> 8) & 0xff;
	output[3] = vendor & 0xff;
	output[4] = attr;

	length = encode_data(p, output + 5, outlen - 5);
	if (length == 0) return 0;

	return length + 5;
}

static int encode_extended(char *buffer,
			   uint8_t *output, size_t outlen)
{
	int attr;
	int length;
	char *p;

	attr = decode_attr(buffer, &p);
	if (attr == 0) return 0;

	output[0] = attr;

	if (attr == 26) {
		length = encode_evs(p, output + 1, outlen - 1);
	} else {
		length = encode_data(p, output + 1, outlen - 1);
	}
	if (length == 0) return 0;
	if (length > (255 - 3)) {
		fprintf(stderr, "Extended Attr data is too long\n");
		return 0;
	}

	return length + 1;
}

static int encode_long_extended(char *buffer,
				 uint8_t *output, size_t outlen)
{
	int attr;
	int length, total;
	char *p;

	attr = decode_attr(buffer, &p);
	if (attr == 0) return 0;

	/* output[0] is the extended attribute */
	output[1] = 4;
	output[2] = attr;
	output[3] = 0;

	if (attr == 26) {
		length = encode_evs(p, output + 4, outlen - 4);
		if (length == 0) return 0;

		output[1] += 5;
		length -= 5;
	} else {
		length = encode_data(p, output + 4, outlen - 4);
	}
	if (length == 0) return 0;

	total = 0;
	while (1) {
		int sublen = 255 - output[1];

		if (length <= sublen) {
			output[1] += length;
			total += output[1];
			break;
		}

		length -= sublen;

		memmove(output + 255 + 4, output + 255, length);
		memcpy(output + 255, output, 4);

		output[1] = 255;
		output[3] |= 0x80;

		output += 255;
		output[1] = 4;
		total += 255;
	}

	return total;
}

static int encode_rfc(char *buffer, uint8_t *output, size_t outlen)
{
	int attr;
	int length, sublen;
	char *p;

	attr = decode_attr(buffer, &p);
	if (attr == 0) return 0;

	length = 2;
	output[0] = attr;
	output[1] = 2;

	if (attr == 26) {
		sublen = encode_vsa(p, output + 2, outlen - 2);

	} else if ((attr < 241) || (attr > 246)) {
		sublen = encode_data(p, output + 2, outlen - 2);

	} else {
		if (*p != '.') {
			fprintf(stderr, "Invalid data following "
				"attribute number\n");
			return 0;
		}

		if (attr < 245) {
			sublen = encode_extended(p + 1,
						 output + 2, outlen - 2);
		} else {

			/*
			 *	Not like the others!
			 */
			return encode_long_extended(p + 1, output, outlen);
		}
	}
	if (sublen == 0) return 0;
	if (sublen > (255 -2)) {
		fprintf(stderr, "RFC Data is too long\n");
		return 0;
	}

	output[1] += sublen;
	return length + sublen;
}

static void parse_condition(char const *input, char *output, size_t outlen)
{
	ssize_t slen;
	char const *error = NULL;
	fr_cond_t *cond;

	slen = fr_condition_tokenize(NULL, NULL, input, &cond, &error, FR_COND_ONE_PASS);
	if (slen <= 0) {
		snprintf(output, outlen, "ERROR offset %d %s", (int) -slen, error);
		return;
	}

	input += slen;
	if (*input != '\0') {
		talloc_free(cond);
		snprintf(output, outlen, "ERROR offset %d 'Too much text'", (int) slen);
		return;
	}

	fr_cond_sprint(output, outlen, cond);

	talloc_free(cond);
}

static void parse_xlat(char const *input, char *output, size_t outlen)
{
	ssize_t slen;
	char const *error = NULL;
	char *fmt = talloc_strdup(NULL, input);
	xlat_exp_t *head;

	slen = xlat_tokenize(fmt, fmt, &head, &error);
	if (slen <= 0) {
		snprintf(output, outlen, "ERROR offset %d '%s'", (int) -slen, error);
		return;
	}

	if (input[slen] != '\0') {
		snprintf(output, outlen, "ERROR offset %d 'Too much text'", (int) slen);
		return;
	}

	xlat_sprint(output, outlen, head);
	talloc_free(fmt);
}

static void process_file(char const *filename)
{
	int lineno;
	size_t i, outlen;
	ssize_t len, data_len;
	FILE *fp;
	char input[8192], buffer[8192];
	char output[8192];
	uint8_t *attr, data[2048];

	if (strcmp(filename, "-") == 0) {
		fp = stdin;
		filename = "<stdin>";

	} else {
		fp = fopen(filename, "r");
		if (!fp) {
			fprintf(stderr, "Error opening %s: %s\n",
				filename, strerror(errno));
			exit(1);
		}
	}

	lineno = 0;
	*output = '\0';
	data_len = 0;

	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		char *p = strchr(buffer, '\n');
		VALUE_PAIR *vp, *head = NULL;
		VALUE_PAIR **tail = &head;

		lineno++;

		if (!p) {
			if (!feof(fp)) {
				fprintf(stderr, "Line %d too long in %s\n",
					lineno, filename);
				exit(1);
			}
		} else {
			*p = '\0';
		}

		/*
		 *	Comments, with hacks for User-Name[#]
		 */
		p = strchr(buffer, '#');
		if (p && ((p == buffer) ||
			  ((p > buffer) && (p[-1] != '[')))) *p = '\0';

		p = buffer;
		while (isspace((int) *p)) p++;
		if (!*p) continue;

		strlcpy(input, p, sizeof(input));

		if (strncmp(p, "raw ", 4) == 0) {
			outlen = encode_rfc(p + 4, data, sizeof(data));
			if (outlen == 0) {
				fprintf(stderr, "Parse error in line %d of %s\n",
					lineno, filename);
				exit(1);
			}

		print_hex:
			if (outlen == 0) {
				output[0] = 0;
				continue;
			}

			data_len = outlen;
			for (i = 0; i < outlen; i++) {
				snprintf(output + 3*i, sizeof(output),
					 "%02x ", data[i]);
			}
			outlen = strlen(output);
			output[outlen - 1] = '\0';
			continue;
		}

		if (strncmp(p, "data ", 5) == 0) {
			if (strcmp(p + 5, output) != 0) {
				fprintf(stderr, "Mismatch in line %d of %s, expected: %s\n",
					lineno, filename, output);
				exit(1);
			}
			continue;
		}

		if (strncmp(p, "encode ", 7) == 0) {
			if (strcmp(p + 7, "-") == 0) {
				p = output;
			} else {
				p += 7;
			}

			if (userparse(NULL, p, &head) != T_EOL) {
				strlcpy(output, fr_strerror(), sizeof(output));
				continue;
			}

			attr = data;
			vp = head;
			len = 0;
			while (vp) {
				len = rad_vp2attr(NULL, NULL, NULL, (VALUE_PAIR const **)(void **)&vp,
						  attr, sizeof(data) - (attr - data));
				if (len < 0) {
					fprintf(stderr, "Failed encoding %s: %s\n",
						vp->da->name, fr_strerror());
					exit(1);
				}

				attr += len;
				if (len == 0) break;
			}

			pairfree(&head);
			outlen = len;
			goto print_hex;
		}

		if (strncmp(p, "decode ", 7) == 0) {
			ssize_t my_len;

			if (strcmp(p + 7, "-") == 0) {
				attr = data;
				len = data_len;
			} else {
				attr = data;
				len = encode_hex(p + 7, data, sizeof(data));
				if (len == 0) {
					fprintf(stderr, "Failed decoding hex string at line %d of %s\n", lineno, filename);
					exit(1);
				}
			}

			my_len = 0;
			while (len > 0) {
				vp = NULL;
				my_len = rad_attr2vp(NULL, NULL, NULL,
						     attr, len, &vp);
				if (my_len < 0) {
					pairfree(&head);
					break;
				}

				if (my_len > len) {
					fprintf(stderr, "Internal sanity check failed at %d\n", __LINE__);
					exit(1);
				}

				*tail = vp;
				while (vp) {
					tail = &(vp->next);
					vp = vp->next;
				}

				attr += my_len;
				len -= my_len;
			}

			/*
			 *	Output may be an error, and we ignore
			 *	it if so.
			 */
			if (head) {
				vp_cursor_t cursor;
				p = output;
				for (vp = paircursor(&cursor, &head);
				     vp;
				     vp = pairnext(&cursor)) {
					vp_prints(p, sizeof(output) - (p - output), vp);
					p += strlen(p);

					if (vp->next) {strcpy(p, ", ");
						p += 2;
					}
				}

				pairfree(&head);
			} else if (my_len < 0) {
				strlcpy(output, fr_strerror(), sizeof(output));

			} else { /* zero-length attribute */
				*output = '\0';
			}
			continue;
		}

		if (strncmp(p, "$INCLUDE ", 9) == 0) {
			p += 9;
			while (isspace((int) *p)) p++;

			process_file(p);
			continue;
		}

		if (strncmp(p, "condition ", 10) == 0) {
			p += 10;
			parse_condition(p, output, sizeof(output));
			continue;
		}

		if (strncmp(p, "xlat ", 5) == 0) {
			p += 5;
			parse_xlat(p, output, sizeof(output));
			continue;
		}

		fprintf(stderr, "Unknown input at line %d of %s\n",
			lineno, filename);
		exit(1);
	}

	if (fp != stdin) fclose(fp);
}

int main(int argc, char *argv[])
{
	int c;
	int report = false;
	char const *radius_dir = RADDBDIR;

	while ((c = getopt(argc, argv, "d:xM")) != EOF) switch(c) {
		case 'd':
			radius_dir = optarg;
			break;
	  	case 'x':
			fr_debug_flag++;
			debug_flag = fr_debug_flag;
			break;
		case 'M':
			report = true;
			break;
		default:
			fprintf(stderr, "usage: radattr [OPTS] filename\n");
			exit(1);
	}
	argc -= (optind - 1);
	argv += (optind - 1);

	if (report) {
		talloc_enable_null_tracking();
	}
	talloc_set_log_fn(log_talloc);

	if (dict_init(radius_dir, RADIUS_DICTIONARY) < 0) {
		fr_perror("radattr");
		return 1;
	}

	if (xlat_register("test", xlat_test, NULL, NULL) < 0) {
		fprintf(stderr, "Failed registering xlat");
		return 1;
	}

	if (argc < 2) {
		process_file("-");

	} else {
		process_file(argv[1]);
	}

	if (report) {
		dict_free();
		log_talloc_report(NULL);
	}

	return 0;
}
