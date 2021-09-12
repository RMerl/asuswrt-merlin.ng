/*
 * Converts a binary file into a C character or integer array
 * (for inclusion in a source file)
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: bin2c.c 774251 2019-04-17 07:48:33Z $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef __GNUC__
extern int getopt(int ac, char *av[], const char *optstring);
extern int optind;
extern char *optarg;
#else
#define _GNU_SOURCE
#include <getopt.h>
#endif // endif
#include <sys/stat.h>

int opt_bigendian = 0;
int opt_word = 0;
int opt_indent = 8;
int opt_maxcol = 79;

char *opt_utype = NULL;

#define SWAP32(x)	((((x) >> 24) & 0x000000ff) | \
			 (((x) >>  8) & 0x0000ff00) | \
			 (((x) <<  8) & 0x00ff0000) | \
			 (((x) << 24) & 0xff000000))

int
convert(char *ifn, char *ofn, char *array_name)
{
	FILE *ifp, *ofp;
	int c, lc;
	struct stat istat;
	char *array_type;
	unsigned int array_len;

	if (stat(ifn, &istat) < 0) {
		fprintf(stderr, "error: could not stat input file %s: %s\n",
		        ifn, strerror(errno));
		return -1;
	}

	array_type = (opt_word ? "unsigned int" : "unsigned char");
	array_len = (unsigned int)(opt_word ? (istat.st_size + 3) / 4 : istat.st_size);

	if ((ifp = fopen(ifn, "rb")) == NULL) {
		fprintf(stderr, "error: could not open input file %s: %s\n",
		        ifn, strerror(errno));
		return -1;
	}

	if ((ofp = fopen(ofn, "w")) == NULL) {
		fclose(ifp);
		fprintf(stderr, "error: could not open output file %s: %s\n",
		        ofn, strerror(errno));
		return -1;
	}

	fprintf(ofp, "/* FILE-CSTYLED */\n");
	if (opt_utype != NULL) {
		fprintf(ofp,
		        "#define %s %s_align._%s\n"
		        "union %s_u {\n"
		        "%*s%s _%s[%u];\n"
		        "%*s%s align;\n"
		        "} %s_align = {{\n",
		        array_name, array_name, array_name,
		        array_name,
		        opt_indent, "", array_type, array_name, array_len,
		        opt_indent, "", opt_utype,
		        array_name);
	} else {
		fprintf(ofp,
		        "%s %s[%u] = {\n",
		        array_type, array_name, array_len);
	}

	lc = 0;

	if (opt_word) {
		while ((c = fgetc(ifp)) != EOF) {
			int i;
			unsigned int val = (unsigned int)c;
			for (i = 0; i < 3; i++) {
				if ((c = getc(ifp)) == EOF)
					c = 0;
				val = (val << 8) | (unsigned int)c;
			}
			if (!opt_bigendian)
				val = SWAP32(val);
			if (lc > 0 && lc >= opt_maxcol - 12) {
				fprintf(ofp, ",\n");
				lc = 0;
			}
			if (lc == 0)
				lc += fprintf(ofp, "%*s0x%08x", opt_indent, "", val);
			else
				lc += fprintf(ofp, ", 0x%08x", val);
		}
	} else {
		while ((c = getc(ifp)) != EOF) {
			if (lc > 0 && lc >= opt_maxcol - 5) {
				fprintf(ofp, ",\n");
				lc = 0;
			}
			if (lc == 0)
				lc += fprintf(ofp, "%*s%d", opt_indent, "", c);
			else
				lc += fprintf(ofp, ", %d", c);
		}
	}

	if (lc > 0)
		fprintf(ofp, "\n");

	if (opt_utype != NULL)
		fprintf(ofp, "}");
	fprintf(ofp, "};\n");

	(void)fclose(ifp);

	if (fclose(ofp) < 0) {
		fprintf(stderr, "error: could not close output file %s: %s\n",
		        ofn, strerror(errno));
		return -1;
	}

	return 0;
}

void
usage(void)
{
	fprintf(stderr,
	        "Usage: bin2c [-w] [-eb] [-u type] [-i indent] [-m maxcol]\n"
	        "                  <input> <output> <arrayname>\n"
	        "    -w         Output 32-bit ints instead of bytes\n"
	        "    -eb        Output for big-endian CPU (when used with -w)\n"
	        "    -u type    Output a union with specified alignment type\n"
	        "    -i indent  Indentation amount\n"
	        "    -m maxcol  Maximum output columns to use\n");
	exit(1);
}

int
main(int argc, char **argv)
{
	int c;

	while ((c = getopt(argc, argv, "we:u:i:m:")) > 0)
		switch (c) {
		case 'w':
			opt_word = 1;
			break;
		case 'e':
			opt_bigendian = (optarg[0] == 'b');
			break;
		case 'u':
			opt_utype = optarg;
			break;
		case 'i':
			opt_indent = atoi(optarg);
			break;
		case 'm':
			opt_maxcol = atoi(optarg);
			break;
		default:
			usage();
		}

	if (optind + 3 != argc)
		usage();

	if (convert(argv[optind], argv[optind + 1], argv[optind + 2]) < 0)
		exit(1);

	exit(0);
}
