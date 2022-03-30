// SPDX-License-Identifier: BSD-3-Clause
/*
 * inih -- simple .INI file parser
 *
 * Copyright (c) 2009, Brush Technology
 * Copyright (c) 2012:
 *              Joe Hershberger, National Instruments, joe.hershberger@ni.com
 * All rights reserved.
 *
 * Go to the project home page for more info:
 * http://code.google.com/p/inih/
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <linux/ctype.h>
#include <linux/string.h>

#ifdef CONFIG_INI_MAX_LINE
#define MAX_LINE CONFIG_INI_MAX_LINE
#else
#define MAX_LINE 200
#endif

#ifdef CONFIG_INI_MAX_SECTION
#define MAX_SECTION CONFIG_INI_MAX_SECTION
#else
#define MAX_SECTION 50
#endif

#ifdef CONFIG_INI_MAX_NAME
#define MAX_NAME CONFIG_INI_MAX_NAME
#else
#define MAX_NAME 50
#endif

/* Strip whitespace chars off end of given string, in place. Return s. */
static char *rstrip(char *s)
{
	char *p = s + strlen(s);

	while (p > s && isspace(*--p))
		*p = '\0';
	return s;
}

/* Return pointer to first non-whitespace char in given string. */
static char *lskip(const char *s)
{
	while (*s && isspace(*s))
		s++;
	return (char *)s;
}

/* Return pointer to first char c or ';' comment in given string, or pointer to
   null at end of string if neither found. ';' must be prefixed by a whitespace
   character to register as a comment. */
static char *find_char_or_comment(const char *s, char c)
{
	int was_whitespace = 0;

	while (*s && *s != c && !(was_whitespace && *s == ';')) {
		was_whitespace = isspace(*s);
		s++;
	}
	return (char *)s;
}

/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
static char *strncpy0(char *dest, const char *src, size_t size)
{
	strncpy(dest, src, size);
	dest[size - 1] = '\0';
	return dest;
}

/* Emulate the behavior of fgets but on memory */
static char *memgets(char *str, int num, char **mem, size_t *memsize)
{
	char *end;
	int len;
	int newline = 1;

	end = memchr(*mem, '\n', *memsize);
	if (end == NULL) {
		if (*memsize == 0)
			return NULL;
		end = *mem + *memsize;
		newline = 0;
	}
	len = min((end - *mem) + newline, num);
	memcpy(str, *mem, len);
	if (len < num)
		str[len] = '\0';

	/* prepare the mem vars for the next call */
	*memsize -= (end - *mem) + newline;
	*mem += (end - *mem) + newline;

	return str;
}

/* Parse given INI-style file. May have [section]s, name=value pairs
   (whitespace stripped), and comments starting with ';' (semicolon). Section
   is "" if name=value pair parsed before any section heading. name:value
   pairs are also supported as a concession to Python's ConfigParser.

   For each name=value pair parsed, call handler function with given user
   pointer as well as section, name, and value (data only valid for duration
   of handler call). Handler should return nonzero on success, zero on error.

   Returns 0 on success, line number of first error on parse error (doesn't
   stop on first error).
*/
static int ini_parse(char *filestart, size_t filelen,
	int (*handler)(void *, char *, char *, char *),	void *user)
{
	/* Uses a fair bit of stack (use heap instead if you need to) */
	char line[MAX_LINE];
	char section[MAX_SECTION] = "";
	char prev_name[MAX_NAME] = "";

	char *curmem = filestart;
	char *start;
	char *end;
	char *name;
	char *value;
	size_t memleft = filelen;
	int lineno = 0;
	int error = 0;

	/* Scan through file line by line */
	while (memgets(line, sizeof(line), &curmem, &memleft) != NULL) {
		lineno++;
		start = lskip(rstrip(line));

		if (*start == ';' || *start == '#') {
			/*
			 * Per Python ConfigParser, allow '#' comments at start
			 * of line
			 */
		}
#if CONFIG_INI_ALLOW_MULTILINE
		else if (*prev_name && *start && start > line) {
			/*
			 * Non-blank line with leading whitespace, treat as
			 * continuation of previous name's value (as per Python
			 * ConfigParser).
			 */
			if (!handler(user, section, prev_name, start) && !error)
				error = lineno;
		}
#endif
		else if (*start == '[') {
			/* A "[section]" line */
			end = find_char_or_comment(start + 1, ']');
			if (*end == ']') {
				*end = '\0';
				strncpy0(section, start + 1, sizeof(section));
				*prev_name = '\0';
			} else if (!error) {
				/* No ']' found on section line */
				error = lineno;
			}
		} else if (*start && *start != ';') {
			/* Not a comment, must be a name[=:]value pair */
			end = find_char_or_comment(start, '=');
			if (*end != '=')
				end = find_char_or_comment(start, ':');
			if (*end == '=' || *end == ':') {
				*end = '\0';
				name = rstrip(start);
				value = lskip(end + 1);
				end = find_char_or_comment(value, '\0');
				if (*end == ';')
					*end = '\0';
				rstrip(value);
				/* Strip double-quotes */
				if (value[0] == '"' &&
				    value[strlen(value)-1] == '"') {
					value[strlen(value)-1] = '\0';
					value += 1;
				}

				/*
				 * Valid name[=:]value pair found, call handler
				 */
				strncpy0(prev_name, name, sizeof(prev_name));
				if (!handler(user, section, name, value) &&
				     !error)
					error = lineno;
			} else if (!error)
				/* No '=' or ':' found on name[=:]value line */
				error = lineno;
		}
	}

	return error;
}

static int ini_handler(void *user, char *section, char *name, char *value)
{
	char *requested_section = (char *)user;
#ifdef CONFIG_INI_CASE_INSENSITIVE
	int i;

	for (i = 0; i < strlen(requested_section); i++)
		requested_section[i] = tolower(requested_section[i]);
	for (i = 0; i < strlen(section); i++)
		section[i] = tolower(section[i]);
#endif

	if (!strcmp(section, requested_section)) {
#ifdef CONFIG_INI_CASE_INSENSITIVE
		for (i = 0; i < strlen(name); i++)
			name[i] = tolower(name[i]);
		for (i = 0; i < strlen(value); i++)
			value[i] = tolower(value[i]);
#endif
		env_set(name, value);
		printf("ini: Imported %s as %s\n", name, value);
	}

	/* success */
	return 1;
}

static int do_ini(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *section;
	char *file_address;
	size_t file_size;

	if (argc == 1)
		return CMD_RET_USAGE;

	section = argv[1];
	file_address = (char *)simple_strtoul(
		argc < 3 ? env_get("loadaddr") : argv[2], NULL, 16);
	file_size = (size_t)simple_strtoul(
		argc < 4 ? env_get("filesize") : argv[3], NULL, 16);

	return ini_parse(file_address, file_size, ini_handler, (void *)section);
}

U_BOOT_CMD(
	ini, 4, 0, do_ini,
	"parse an ini file in memory and merge the specified section into the env",
	"section [[file-address] file-size]"
);
