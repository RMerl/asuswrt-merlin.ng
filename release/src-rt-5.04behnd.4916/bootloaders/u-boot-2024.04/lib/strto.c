/*
 *  linux/lib/vsprintf.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/* vsprintf.c -- Lars Wirzenius & Linus Torvalds. */
/*
 * Wirzenius wrote this portably, Torvalds fucked it up :-)
 */

#include <errno.h>
#include <malloc.h>
#include <vsprintf.h>
#include <linux/ctype.h>

/* from lib/kstrtox.c */
static const char *_parse_integer_fixup_radix(const char *s, uint *basep)
{
	/* Look for a 0x prefix */
	if (s[0] == '0') {
		int ch = tolower(s[1]);

		if (ch == 'x') {
			*basep = 16;
			s += 2;
		} else if (!*basep) {
			/* Only select octal if we don't have a base */
			*basep = 8;
		}
	}

	/* Use decimal by default */
	if (!*basep)
		*basep = 10;

	return s;
}

/**
 * decode_digit() - Decode a single character into its numeric digit value
 *
 * This ignore case
 *
 * @ch: Character to convert (expects '0'..'9', 'a'..'f' or 'A'..'F')
 * Return: value of digit (0..0xf) or 255 if the character is invalid
 */
static uint decode_digit(int ch)
{
	if (!isxdigit(ch))
		return 256;

	ch = tolower(ch);

	return ch <= '9' ? ch - '0' : ch - 'a' + 0xa;
}

ulong simple_strtoul(const char *cp, char **endp, uint base)
{
	ulong result = 0;
	uint value;

	cp = _parse_integer_fixup_radix(cp, &base);

	while (value = decode_digit(*cp), value < base) {
		result = result * base + value;
		cp++;
	}

	if (endp)
		*endp = (char *)cp;

	return result;
}

ulong hextoul(const char *cp, char **endp)
{
	return simple_strtoul(cp, endp, 16);
}

ulong dectoul(const char *cp, char **endp)
{
	return simple_strtoul(cp, endp, 10);
}

int strict_strtoul(const char *cp, unsigned int base, unsigned long *res)
{
	char *tail;
	unsigned long val;
	size_t len;

	*res = 0;
	len = strlen(cp);
	if (len == 0)
		return -EINVAL;

	val = simple_strtoul(cp, &tail, base);
	if (tail == cp)
		return -EINVAL;

	if ((*tail == '\0') ||
		((len == (size_t)(tail - cp) + 1) && (*tail == '\n'))) {
		*res = val;
		return 0;
	}

	return -EINVAL;
}

long simple_strtol(const char *cp, char **endp, unsigned int base)
{
	if (*cp == '-')
		return -simple_strtoul(cp + 1, endp, base);

	return simple_strtoul(cp, endp, base);
}

unsigned long ustrtoul(const char *cp, char **endp, unsigned int base)
{
	unsigned long result = simple_strtoul(cp, endp, base);
	switch (tolower(**endp)) {
	case 'g':
		result *= 1024;
		/* fall through */
	case 'm':
		result *= 1024;
		/* fall through */
	case 'k':
		result *= 1024;
		(*endp)++;
		if (**endp == 'i')
			(*endp)++;
		if (**endp == 'B')
			(*endp)++;
	}
	return result;
}

unsigned long long ustrtoull(const char *cp, char **endp, unsigned int base)
{
	unsigned long long result = simple_strtoull(cp, endp, base);
	switch (tolower(**endp)) {
	case 'g':
		result *= 1024;
		/* fall through */
	case 'm':
		result *= 1024;
		/* fall through */
	case 'k':
		result *= 1024;
		(*endp)++;
		if (**endp == 'i')
			(*endp)++;
		if (**endp == 'B')
			(*endp)++;
	}
	return result;
}

unsigned long long simple_strtoull(const char *cp, char **endp,
					unsigned int base)
{
	unsigned long long result = 0;
	uint value;

	cp = _parse_integer_fixup_radix(cp, &base);

	while (value = decode_digit(*cp), value < base) {
		result = result * base + value;
		cp++;
	}

	if (endp)
		*endp = (char *) cp;

	return result;
}

long long simple_strtoll(const char *cp, char **endp, unsigned int base)
{
	if (*cp == '-')
		return -simple_strtoull(cp + 1, endp, base);

	return simple_strtoull(cp, endp, base);
}

long trailing_strtoln_end(const char *str, const char *end, char const **endp)
{
	const char *p;

	if (!end)
		end = str + strlen(str);
	p = end - 1;
	if (p > str && isdigit(*p)) {
		do {
			if (!isdigit(p[-1])) {
				if (endp)
					*endp = p;
				return dectoul(p, NULL);
			}
		} while (--p > str);
	}
	if (endp)
		*endp = end;

	return -1;
}

long trailing_strtoln(const char *str, const char *end)
{
	return trailing_strtoln_end(str, end, NULL);
}

long trailing_strtol(const char *str)
{
	return trailing_strtoln(str, NULL);
}

void str_to_upper(const char *in, char *out, size_t len)
{
	for (; len > 0 && *in; len--)
		*out++ = toupper(*in++);
	if (len)
		*out = '\0';
}

const char **str_to_list(const char *instr)
{
	const char **ptr;
	char *str, *p;
	int count, i;

	/* don't allocate if the string is empty */
	str = *instr ? strdup(instr) : (char *)instr;
	if (!str)
		return NULL;

	/* count the number of space-separated strings */
	for (count = *str != '\0', p = str; *p; p++) {
		if (*p == ' ') {
			count++;
			*p = '\0';
		}
	}

	/* allocate the pointer array, allowing for a NULL terminator */
	ptr = calloc(count + 1, sizeof(char *));
	if (!ptr) {
		if (*str)
			free(str);
		return NULL;
	}

	for (i = 0, p = str; i < count; p += strlen(p) + 1, i++)
		ptr[i] = p;

	return ptr;
}

void str_free_list(const char **ptr)
{
	if (ptr)
		free((char *)ptr[0]);
	free(ptr);
}
