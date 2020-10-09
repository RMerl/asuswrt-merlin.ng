/*
 * cstring.c -- parse and print strings using the C escape sequences
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#include <string.h>

#include "cstring.h"

int parse_c_string(char *str)
{
	char *to, *from, ch;
	int v;

	to = from = str;

	for (to = from = (char *) str;
	     *from && *from != '"'; to++, from++) {
		if (*from == '\\') {
			ch = *(++from);
			switch (ch) {
			case 'a':
				*to = '\a';
				break;
			case 'b':
				*to = '\b';
				break;
			case 'f':
				*to = '\f';
				break;
			case 'n':
				*to = '\n';
				break;
			case 't':
				*to = '\t';
				break;
			case 'v':
				*to = '\v';
				break;
			case 'x':
				ch = *(from + 1);
				if (ch >= 'a' && ch <= 'f')
					ch = ch - 'a' + 'A';
				if (ch >= '0' && ch <= '9')
					v = ch - '0';
				else if (ch >= 'A' && ch <= 'F')
					v = ch + 10 - 'A';
				else {
					*to = 'x';
					break;
				}
				from++;
				ch = *(from + 1);
				if (ch >= 'a' && ch <= 'f')
					ch = ch - 'a' + 'A';
				if (ch >= '0' && ch <= '9')
					v = (v * 16) + (ch - '0');
				else if (ch >= 'A' && ch <= 'F')
					v = (v * 16) + (ch + 10 - 'A');
				else {
					*to = 'x';
					from--;
					break;
				}
				from++;
				*to = v;
				break;
			default:
				if (ch >= '0' && ch <= '9') {
					v = ch - '0';
					ch = *(from + 1);
					if (ch >= '0' && ch <= '9') {
						from++;
						v = (8 * v) + (ch - '0');
						ch = *(from + 1);
						if (ch >= '0' && ch <= '9') {
							from++;
							v = (8 * v) + (ch - '0');
						}
					}
					ch = v;
				}
				*to = ch;
			}
			continue;
		}
		*to = *from;
	}
	*to = '\0';
	return to - (char *) str;
}

void print_c_string(FILE *f, const char *cp, int len)
{
	unsigned char	ch;

	if (len < 0)
		len = strlen(cp);

	while (len--) {
		ch = *cp++;
		if (ch == '\a')
			fputs("\\a", f);
		else if (ch == '\b')
			fputs("\\b", f);
		else if (ch == '\f')
			fputs("\\f", f);
		else if (ch == '\n')
			fputs("\\n", f);
		else if (ch == '\t')
			fputs("\\t", f);
		else if (ch == '\v')
			fputs("\\v", f);
		else if (ch == '\\')
			fputs("\\\\", f);
		else if (ch == '\'')
			fputs("\\\'", f);
		else if (ch == '\"')
			fputs("\\\"", f);
		else if ((ch < 32) || (ch > 126))
			fprintf(f, "\\%03o", ch);
		else
			fputc(ch, f);
	}
}

#ifdef DEBUG_PROGRAM
int main(int argc, char **argv)
{
	char buf[4096];
	int c, raw = 0;

	while ((c = getopt(argc, argv, "r")) != EOF) {
		switch (c) {
		case 'r':
			raw++;
			break;
		default:
			fprintf(stderr, "Usage: %s [-r]\n", argv[0]);
			exit(1);
		}
	}

	while (!feof(stdin)) {
		if (fgets(buf, sizeof(buf), stdin) == NULL)
			break;
		c = parse_c_string(buf);
		if (raw)
			fputs(buf, stdout);
		else {
			print_c_string(stdout, buf, c);
			printf(" <%d>\n", c);
		}
	}
}	
#endif
