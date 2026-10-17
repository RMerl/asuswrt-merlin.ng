// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 1990, 1993
 * The Regents of the University of California. All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Copyright (c) 2011 The FreeBSD Foundation
 * All rights reserved.
 * Portions of this software were developed by David Chisnall
 * under sponsorship from the FreeBSD Foundation.
 *
 * Author: Juergen Gross <jgross@suse.com>
 * Date: Jun 2016
 */

#if !defined HAVE_LIBC

#include <os.h>
#include <linux/kernel.h>
#include <linux/ctype.h>
#include <vsprintf.h>
#include <linux/string.h>
#include <malloc.h>
#define __DECONST(type, var)    ((type)(uintptr_t)(const void *)(var))

/**
 * struct str_info - Input string parameters
 * @neg: negative number or not
 *	 0 - not negative
 *	 1 - negative
 * @any: set any if any `digits' consumed; make it negative to indicate
 *	 overflow
 * @acc: accumulated value
 */
struct str_info {
	int neg, any;
	u64 acc;
};

/**
 * str_to_int_convert() - Write string data to structure
 * @nptr: pointer to string
 * @base: number's base
 * @unsign: describes what integer is expected
 *	    0 - not unsigned
 *	    1 - unsigned
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 *
 * Return: struct str_info *, which contains string data to future process
 */
static struct str_info *
str_to_int_convert(const char **nptr, int base, unsigned int unsign)
{
	const char *s = *nptr;
	u64 acc;
	unsigned char c;
	u64 cutoff;
	int neg, any, cutlim;
	u64 qbase;
	struct str_info *info;

	/*
	 * Skip white space and pick up leading +/- sign if any.
	 * If base is 0, allow 0x for hex and 0 for octal, else
	 * assume decimal; if base is already 16, allow 0x.
	 */
	info = (struct str_info *)malloc(sizeof(struct str_info));
	if (!info)
		return NULL;

	do {
		c = *s++;
	} while (isspace(c));
	if (c == '-') {
		neg = 1;
		c = *s++;
	} else {
		neg = 0;
		if (c == '+')
			c = *s++;
	}
	if ((base == 0 || base == 16) &&
	    c == '0' && (*s == 'x' || *s == 'X')) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if (base == 0)
		base = c == '0' ? 8 : 10;

	/*
	 * Compute the cutoff value between legal numbers and illegal
	 * numbers.  That is the largest legal value, divided by the
	 * base.  An input number that is greater than this value, if
	 * followed by a legal input character, is too big.  One that
	 * is equal to this value may be valid or not; the limit
	 * between valid and invalid numbers is then based on the last
	 * digit.  For instance, if the range for quads is
	 * [-9223372036854775808..9223372036854775807] and the input base
	 * is 10, cutoff will be set to 922337203685477580 and cutlim to
	 * either 7 (neg==0) or 8 (neg==1), meaning that if we have
	 * accumulated a value > 922337203685477580, or equal but the
	 * next digit is > 7 (or 8), the number is too big, and we will
	 * return a range error.
	 *
	 * Set any if any `digits' consumed; make it negative to indicate
	 * overflow.
	 */
	qbase = (unsigned int)base;

	if (!unsign) {
		cutoff = neg ? (u64)-(LLONG_MIN + LLONG_MAX) + LLONG_MAX : LLONG_MAX;
		cutlim = cutoff % qbase;
		cutoff /= qbase;
	} else {
		cutoff = (u64)ULLONG_MAX / qbase;
		cutlim = (u64)ULLONG_MAX % qbase;
	}

	for (acc = 0, any = 0;; c = *s++) {
		if (!isascii(c))
			break;
		if (isdigit(c))
			c -= '0';
		else if (isalpha(c))
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) {
			any = -1;
		} else {
			any = 1;
			acc *= qbase;
			acc += c;
		}
	}

	info->any = any;
	info->neg = neg;
	info->acc = acc;

	*nptr = s;

	return info;
}

/**
 * strtoq() - Convert a string to a quad integer
 * @nptr: pointer to string
 * @endptr: pointer to number's end in the string
 * @base: number's base
 *
 * Return: s64 quad integer number converted from input string
 */
static s64
strtoq(const char *nptr, char **endptr, int base)
{
	const char *s = nptr;
	u64 acc;
	int unsign = 0;
	struct str_info *info;

	info = str_to_int_convert(&s, base, unsign);
	if (!info)
		return -1;

	acc = info->acc;

	if (info->any < 0)
		acc = info->neg ? LLONG_MIN : LLONG_MAX;
	else if (info->neg)
		acc = -acc;
	if (endptr != 0)
		*endptr = __DECONST(char *, info->any ? s - 1 : nptr);

	free(info);

	return acc;
}

/**
 * strtouq() - Convert a string to an unsigned quad integer
 * @nptr: pointer to string
 * @endptr: pointer to number's end in the string
 * @base: number's base
 *
 * Return: s64 unsigned quad integer number converted from
 *         input string
 */
u64
strtouq(const char *nptr, char **endptr, int base)
{
		const char *s = nptr;
	u64 acc;
	int unsign = 1;
	struct str_info *info;

	info = str_to_int_convert(&s, base, unsign);
	if (!info)
		return -1;

	acc = info->acc;

	if (info->any < 0)
		acc = ULLONG_MAX;
	else if (info->neg)
		acc = -acc;
	if (endptr != 0)
		*endptr = __DECONST(char *, info->any ? s - 1 : nptr);

	free(info);

	return acc;
}

/**
 * __sccl() - Fill in the given table from the scanset at the given format
 * (just after `[')
 * @tab: table to fill in
 * @fmt: format of buffer
 *
 * The table has a 1 wherever characters should be considered part of the
 * scanset.
 *
 * Return: pointer to the character past the closing `]'
 */
static const u_char *
__sccl(char *tab, const u_char *fmt)
{
	int c, n, v;

	/* first `clear' the whole table */
	c = *fmt++;             /* first char hat => negated scanset */
	if (c == '^') {
		v = 1;          /* default => accept */
		c = *fmt++;     /* get new first char */
	} else {
		v = 0;          /* default => reject */
	}

	/* XXX: Will not work if sizeof(tab*) > sizeof(char) */
	for (n = 0; n < 256; n++)
		tab[n] = v;        /* memset(tab, v, 256) */

	if (c == 0)
		return (fmt - 1);/* format ended before closing ] */

	/*
	 * Now set the entries corresponding to the actual scanset
	 * to the opposite of the above.
	 *
	 * The first character may be ']' (or '-') without being special;
	 * the last character may be '-'.
	 */
	v = 1 - v;
	for (;;) {
		tab[c] = v;             /* take character c */
doswitch:
		n = *fmt++;             /* and examine the next */
		switch (n) {
		case 0:                 /* format ended too soon */
			return (fmt - 1);

		case '-':
			/*
			 * A scanset of the form
			 *      [01+-]
			 * is defined as `the digit 0, the digit 1,
			 * the character +, the character -', but
			 * the effect of a scanset such as
			 *      [a-zA-Z0-9]
			 * is implementation defined.  The V7 Unix
			 * scanf treats `a-z' as `the letters a through
			 * z', but treats `a-a' as `the letter a, the
			 * character -, and the letter a'.
			 *
			 * For compatibility, the `-' is not considerd
			 * to define a range if the character following
			 * it is either a close bracket (required by ANSI)
			 * or is not numerically greater than the character
			 * we just stored in the table (c).
			 */
			n = *fmt;
			if (n == ']' || n < c) {
				c = '-';
				break;  /* resume the for(;;) */
			}
			fmt++;
			/* fill in the range */
			do {
				tab[++c] = v;
			} while (c < n);
			c = n;
			/*
			 * Alas, the V7 Unix scanf also treats formats
			 * such as [a-c-e] as `the letters a through e'.
			 * This too is permitted by the standard....
			 */
			goto doswitch;
			break;

		case ']':               /* end of scanset */
			return (fmt);

		default:                /* just another character */
			c = n;
			break;
		}
	}
	/* NOTREACHED */
}

/**
 * vsscanf - Unformat a buffer into a list of arguments
 * @buf:	input buffer
 * @fmt:	format of buffer
 * @args:	arguments
 */
#define BUF             32      /* Maximum length of numeric string. */

/*
 * Flags used during conversion.
 */
#define LONG            0x01    /* l: long or double */
#define SHORT           0x04    /* h: short */
#define SUPPRESS        0x08    /* suppress assignment */
#define POINTER         0x10    /* weird %p pointer (`fake hex') */
#define NOSKIP          0x20    /* do not skip blanks */
#define QUAD            0x400
#define SHORTSHORT      0x4000  /** hh: char */

/*
 * The following are used in numeric conversions only:
 * SIGNOK, NDIGITS, DPTOK, and EXPOK are for floating point;
 * SIGNOK, NDIGITS, PFXOK, and NZDIGITS are for integral.
 */
#define SIGNOK          0x40    /* +/- is (still) legal */
#define NDIGITS         0x80    /* no digits detected */

#define DPTOK           0x100   /* (float) decimal point is still legal */
#define EXPOK           0x200   /* (float) exponent (e+3, etc) still legal */

#define PFXOK           0x100   /* 0x prefix is (still) legal */
#define NZDIGITS        0x200   /* no zero digits detected */

/*
 * Conversion types.
 */
#define CT_CHAR         0       /* %c conversion */
#define CT_CCL          1       /* %[...] conversion */
#define CT_STRING       2       /* %s conversion */
#define CT_INT          3       /* integer, i.e., strtoq or strtouq */
typedef u64 (*ccfntype)(const char *, char **, int);

int
vsscanf(const char *inp, char const *fmt0, va_list ap)
{
	int inr;
	const u_char *fmt = (const u_char *)fmt0;
	int c;                  /* character from format, or conversion */
	size_t width;           /* field width, or 0 */
	char *p;                /* points into all kinds of strings */
	int n;                  /* handy integer */
	int flags;              /* flags as defined above */
	char *p0;               /* saves original value of p when necessary */
	int nassigned;          /* number of fields assigned */
	int nconversions;       /* number of conversions */
	int nread;              /* number of characters consumed from fp */
	int base;               /* base argument to strtoq/strtouq */
	ccfntype ccfn;          /* conversion function (strtoq/strtouq) */
	char ccltab[256];       /* character class table for %[...] */
	char buf[BUF];          /* buffer for numeric conversions */

	/* `basefix' is used to avoid `if' tests in the integer scanner */
	static short basefix[17] = { 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
				     12, 13, 14, 15, 16 };

	inr = strlen(inp);

	nassigned = 0;
	nconversions = 0;
	nread = 0;
	base = 0;               /* XXX just to keep gcc happy */
	ccfn = NULL;            /* XXX just to keep gcc happy */
	for (;;) {
		c = *fmt++;
		if (c == 0)
			return (nassigned);
		if (isspace(c)) {
			while (inr > 0 && isspace(*inp))
				nread++, inr--, inp++;
			continue;
		}
		if (c != '%')
			goto literal;
		width = 0;
		flags = 0;
		/*
		 * switch on the format.  continue if done;
		 * break once format type is derived.
		 */
again:          c = *fmt++;
		switch (c) {
		case '%':
literal:
			if (inr <= 0)
				goto input_failure;
			if (*inp != c)
				goto match_failure;
			inr--, inp++;
			nread++;
			continue;

		case '*':
			flags |= SUPPRESS;
			goto again;
		case 'l':
			if (flags & LONG) {
				flags &= ~LONG;
				flags |= QUAD;
			} else {
				flags |= LONG;
			}
			goto again;
		case 'q':
			flags |= QUAD;
			goto again;
		case 'h':
			if (flags & SHORT) {
				flags &= ~SHORT;
				flags |= SHORTSHORT;
			} else {
				flags |= SHORT;
			}
			goto again;

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			width = width * 10 + c - '0';
			goto again;

		/*
		 * Conversions.
		 *
		 */
		case 'd':
			c = CT_INT;
			ccfn = (ccfntype)strtoq;
			base = 10;
			break;

		case 'i':
			c = CT_INT;
			ccfn = (ccfntype)strtoq;
			base = 0;
			break;

		case 'o':
			c = CT_INT;
			ccfn = strtouq;
			base = 8;
			break;

		case 'u':
			c = CT_INT;
			ccfn = strtouq;
			base = 10;
			break;

		case 'x':
			flags |= PFXOK; /* enable 0x prefixing */
			c = CT_INT;
			ccfn = strtouq;
			base = 16;
			break;

		case 's':
			c = CT_STRING;
			break;

		case '[':
			fmt = __sccl(ccltab, fmt);
			flags |= NOSKIP;
			c = CT_CCL;
			break;

		case 'c':
			flags |= NOSKIP;
			c = CT_CHAR;
			break;

		case 'p':       /* pointer format is like hex */
			flags |= POINTER | PFXOK;
			c = CT_INT;
			ccfn = strtouq;
			base = 16;
			break;

		case 'n':
			nconversions++;
			if (flags & SUPPRESS)   /* ??? */
				continue;
			if (flags & SHORTSHORT)
				*va_arg(ap, char *) = nread;
			else if (flags & SHORT)
				*va_arg(ap, short *) = nread;
			else if (flags & LONG)
				*va_arg(ap, long *) = nread;
			else if (flags & QUAD)
				*va_arg(ap, s64 *) = nread;
			else
				*va_arg(ap, int *) = nread;
			continue;
		}

		/*
		 * We have a conversion that requires input.
		 */
		if (inr <= 0)
			goto input_failure;

		/*
		 * Consume leading white space, except for formats
		 * that suppress this.
		 */
		if ((flags & NOSKIP) == 0) {
			while (isspace(*inp)) {
				nread++;
				if (--inr > 0)
					inp++;
				else
					goto input_failure;
			}
			/*
			 * Note that there is at least one character in
			 * the buffer, so conversions that do not set NOSKIP
			 * can no longer result in an input failure.
			 */
		}

		/*
		 * Do the conversion.
		 */
		switch (c) {
		case CT_CHAR:
			/* scan arbitrary characters (sets NOSKIP) */
			if (width == 0)
				width = 1;
			if (flags & SUPPRESS) {
				size_t sum = 0;

				n = inr;
				if (n < width) {
					sum += n;
					width -= n;
					inp += n;
					if (sum == 0)
						goto input_failure;
				} else {
					sum += width;
					inr -= width;
					inp += width;
				}
				nread += sum;
			} else {
				memcpy(va_arg(ap, char *), inp, width);
				inr -= width;
				inp += width;
				nread += width;
				nassigned++;
			}
			nconversions++;
			break;

		case CT_CCL:
			/* scan a (nonempty) character class (sets NOSKIP) */
			if (width == 0)
				width = (size_t)~0;     /* `infinity' */
			/* take only those things in the class */
			if (flags & SUPPRESS) {
				n = 0;
				while (ccltab[(unsigned char)*inp]) {
					n++, inr--, inp++;
					if (--width == 0)
						break;
					if (inr <= 0) {
						if (n == 0)
							goto input_failure;
						break;
					}
				}
				if (n == 0)
					goto match_failure;
			} else {
				p = va_arg(ap, char *);
				p0 = p;
				while (ccltab[(unsigned char)*inp]) {
					inr--;
					*p++ = *inp++;
					if (--width == 0)
						break;
					if (inr <= 0) {
						if (p == p0)
							goto input_failure;
						break;
					}
				}
				n = p - p0;
				if (n == 0)
					goto match_failure;
				*p = 0;
				nassigned++;
			}
			nread += n;
			nconversions++;
			break;

		case CT_STRING:
			/* like CCL, but zero-length string OK, & no NOSKIP */
			if (width == 0)
				width = (size_t)~0;
			if (flags & SUPPRESS) {
				n = 0;
				while (!isspace(*inp)) {
					n++, inr--, inp++;
					if (--width == 0)
						break;
					if (inr <= 0)
						break;
				}
				nread += n;
			} else {
				p = va_arg(ap, char *);
				p0 = p;
				while (!isspace(*inp)) {
					inr--;
					*p++ = *inp++;
					if (--width == 0)
						break;
					if (inr <= 0)
						break;
				}
				*p = 0;
				nread += p - p0;
				nassigned++;
			}
			nconversions++;
			continue;

		case CT_INT:
			/* scan an integer as if by strtoq/strtouq */
#ifdef hardway
			if (width == 0 || width > sizeof(buf) - 1)
				width = sizeof(buf) - 1;
#else
			/* size_t is unsigned, hence this optimisation */
			if (--width > sizeof(buf) - 2)
				width = sizeof(buf) - 2;
			width++;
#endif
			flags |= SIGNOK | NDIGITS | NZDIGITS;
			for (p = buf; width; width--) {
				c = *inp;
				/*
				 * Switch on the character; `goto ok'
				 * if we accept it as a part of number.
				 */
				switch (c) {
				/*
				 * The digit 0 is always legal, but is
				 * special.  For %i conversions, if no
				 * digits (zero or nonzero) have been
				 * scanned (only signs), we will have
				 * base==0.  In that case, we should set
				 * it to 8 and enable 0x prefixing.
				 * Also, if we have not scanned zero digits
				 * before this, do not turn off prefixing
				 * (someone else will turn it off if we
				 * have scanned any nonzero digits).
				 */
				case '0':
					if (base == 0) {
						base = 8;
						flags |= PFXOK;
					}
					if (flags & NZDIGITS)
						flags &= ~(SIGNOK | NZDIGITS | NDIGITS);
					else
						flags &= ~(SIGNOK | PFXOK | NDIGITS);
					goto ok;

				/* 1 through 7 always legal */
				case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
					base = basefix[base];
					flags &= ~(SIGNOK | PFXOK | NDIGITS);
					goto ok;

				/* digits 8 and 9 ok iff decimal or hex */
				case '8': case '9':
					base = basefix[base];
					if (base <= 8)
						break;  /* not legal here */
					flags &= ~(SIGNOK | PFXOK | NDIGITS);
					goto ok;

				/* letters ok iff hex */
				case 'A': case 'B': case 'C':
				case 'D': case 'E': case 'F':
				case 'a': case 'b': case 'c':
				case 'd': case 'e': case 'f':
					/* no need to fix base here */
					if (base <= 10)
						break;  /* not legal here */
					flags &= ~(SIGNOK | PFXOK | NDIGITS);
					goto ok;

				/* sign ok only as first character */
				case '+': case '-':
					if (flags & SIGNOK) {
						flags &= ~SIGNOK;
						goto ok;
						}
					break;

				/* x ok iff flag still set & 2nd char */
				case 'x': case 'X':
					if (flags & PFXOK && p == buf + 1) {
						base = 16;      /* if %i */
						flags &= ~PFXOK;
						goto ok;
					}
					break;
				}

				/*
				 * If we got here, c is not a legal character
				 * for a number.  Stop accumulating digits.
				 */
				break;
ok:
				/*
				 * c is legal: store it and look at the next.
				 */
				*p++ = c;
				if (--inr > 0)
					inp++;
				else
					break;          /* end of input */
			}
			/*
			 * If we had only a sign, it is no good; push
			 * back the sign.  If the number ends in `x',
			 * it was [sign] '' 'x', so push back the x
			 * and treat it as [sign] ''.
			 */
			if (flags & NDIGITS) {
				if (p > buf) {
					inp--;
					inr++;
				}
				goto match_failure;
			}
			c = ((u_char *)p)[-1];
			if (c == 'x' || c == 'X') {
				--p;
				inp--;
				inr++;
			}
			if ((flags & SUPPRESS) == 0) {
				u64 res;

				*p = 0;
				res = (*ccfn)(buf, (char **)NULL, base);
				if (flags & POINTER)
					*va_arg(ap, void **) =
					(void *)(uintptr_t)res;
				else if (flags & SHORTSHORT)
					*va_arg(ap, char *) = res;
				else if (flags & SHORT)
					*va_arg(ap, short *) = res;
				else if (flags & LONG)
					*va_arg(ap, long *) = res;
				else if (flags & QUAD)
					*va_arg(ap, s64 *) = res;
				else
					*va_arg(ap, int *) = res;
				nassigned++;
			}
			nread += p - buf;
			nconversions++;
			break;
		}
	}
input_failure:
		return (nconversions != 0 ? nassigned : -1);
match_failure:
		return (nassigned);
}

/**
 * sscanf - Unformat a buffer into a list of arguments
 * @buf:	input buffer
 * @fmt:	formatting of buffer
 * @...:	resulting arguments
 */
int sscanf(const char *buf, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsscanf(buf, fmt, args);
	va_end(args);
	return i;
}

#endif
