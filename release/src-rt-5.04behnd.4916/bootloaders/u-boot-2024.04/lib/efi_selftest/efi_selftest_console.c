// SPDX-License-Identifier: GPL-2.0+
/*
 * EFI efi_selftest
 *
 * Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#include <efi_selftest.h>
#include <net.h>
#include <vsprintf.h>

struct efi_simple_text_output_protocol *con_out;
struct efi_simple_text_input_protocol *con_in;

/*
 * Print a MAC address to an u16 string
 *
 * @pointer: mac address
 * @buf: pointer to buffer address
 * on return position of terminating zero word
 */
static void mac(void *pointer, u16 **buf)
{
	int i, j;
	u16 c;
	u8 *p = (u8 *)pointer;
	u8 byte;
	u16 *pos = *buf;

	for (i = 0; i < ARP_HLEN; ++i) {
		if (i)
			*pos++ = ':';
		byte = p[i];
		for (j = 4; j >= 0; j -= 4) {
			c = (byte >> j) & 0x0f;
			c += '0';
			if (c > '9')
				c += 'a' - '9' - 1;
			*pos++ = c;
		}
	}
	*pos = 0;
	*buf = pos;
}

/*
 * printx() - print hexadecimal number to an u16 string
 *
 * @p:		value to print
 * @prec:	minimum number of digits to print
 * @buf:	pointer to buffer address,
 *		on return position of terminating zero word
 */
static void printx(u64 p, int prec, u16 **buf)
{
	int i;
	u16 c;
	u16 *pos = *buf;

	for (i = 2 * sizeof(p) - 1; i >= 0; --i) {
		c = (p >> (4 * i)) & 0x0f;
		if (c || pos != *buf || !i || i < prec) {
			c += '0';
			if (c > '9')
				c += 'a' - '9' - 1;
			*pos++ = c;
		}
	}
	*pos = 0;
	*buf = pos;
}

/**
 * print_guid() - print GUID to an u16 string
 *
 * @p:		GUID to print
 * @buf:	pointer to buffer address,
 *		on return position of terminating zero word
 */
static void print_uuid(u8 *p, u16 **buf)
{
	int i;
	const u8 seq[] = {
		3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-',
		8, 9, 10, 11, 12, 13, 14, 15 };

	for (i = 0; i < sizeof(seq); ++i) {
		if (seq[i] == '-')
			*(*buf)++ = u'-';
		else
			printx(p[seq[i]], 2, buf);
	}
}

/*
 * Print an unsigned 32bit value as decimal number to an u16 string
 *
 * @value:	value to be printed
 * @prec:	minimum number of digits to display
 * @buf:	pointer to buffer address
 *		on return position of terminating zero word
 */
static void uint2dec(u32 value, int prec, u16 **buf)
{
	u16 *pos = *buf;
	int i;
	u16 c;
	u64 f;

	/*
	 * Increment by .5 and multiply with
	 * (2 << 60) / 1,000,000,000 = 0x44B82FA0.9B5A52CC
	 * to move the first digit to bit 60-63.
	 */
	f = 0x225C17D0;
	f += (0x9B5A52DULL * value) >> 28;
	f += 0x44B82FA0ULL * value;

	for (i = 0; i < 10; ++i) {
		/* Write current digit */
		c = f >> 60;
		if (c || pos != *buf || 10 - i <= prec)
			*pos++ = c + '0';
		/* Eliminate current digit */
		f &= 0xfffffffffffffff;
		/* Get next digit */
		f *= 0xaULL;
	}
	if (pos == *buf)
		*pos++ = '0';
	*pos = 0;
	*buf = pos;
}

/*
 * Print a signed 32bit value as decimal number to an u16 string
 *
 * @value:	value to be printed
 * @prec:	minimum number of digits to display
 * @buf:	pointer to buffer address
 * on return position of terminating zero word
 */
static void int2dec(s32 value, int prec, u16 **buf)
{
	u32 u;
	u16 *pos = *buf;

	if (value < 0) {
		*pos++ = '-';
		u = -value;
	} else {
		u = value;
	}
	uint2dec(u, prec, &pos);
	*buf = pos;
}

/*
 * Print a colored formatted string to the EFI console
 *
 * @color	color, see constants in efi_api.h, use -1 for no color
 * @fmt		format string
 * @...		optional arguments
 */
void efi_st_printc(int color, const char *fmt, ...)
{
	va_list args;
	u16 buf[160];
	const char *c;
	u16 *pos = buf;
	const char *s;
	u16 *u;
	int prec;

	va_start(args, fmt);

	if (color >= 0)
		con_out->set_attribute(con_out, (unsigned long)color);
	c = fmt;
	for (; *c; ++c) {
		switch (*c) {
		case '\\':
			++c;
			switch (*c) {
			case '\0':
				--c;
				break;
			case 'n':
				*pos++ = '\n';
				break;
			case 'r':
				*pos++ = '\r';
				break;
			case 't':
				*pos++ = '\t';
				break;
			default:
				*pos++ = *c;
			}
			break;
		case '%':
			++c;
			/* Parse precision */
			if (*c == '.') {
				++c;
				prec = *c - '0';
				++c;
			} else {
				prec = 0;
			}
			switch (*c) {
			case '\0':
				--c;
				break;
			case 'd':
				int2dec(va_arg(args, s32), prec, &pos);
				break;
			case 'p':
				++c;
				switch (*c) {
				/* MAC address */
				case 'm':
					mac(va_arg(args, void*), &pos);
					break;

				/* u16 string */
				case 's':
					u = va_arg(args, u16*);
					if (pos > buf) {
						*pos = 0;
						con_out->output_string(con_out,
								       buf);
					}
					con_out->output_string(con_out, u);
					pos = buf;
					break;
				case 'U':
					print_uuid(va_arg(args, void*), &pos);
					break;
				default:
					--c;
					printx((uintptr_t)va_arg(args, void *),
					       2 * sizeof(void *), &pos);
					break;
				}
				break;
			case 's':
				s = va_arg(args, const char *);
				for (; *s; ++s)
					*pos++ = *s;
				break;
			case 'u':
				uint2dec(va_arg(args, u32), prec, &pos);
				break;
			case 'x':
				printx((u64)va_arg(args, unsigned int),
				       prec, &pos);
				break;
			default:
				break;
			}
			break;
		default:
			*pos++ = *c;
		}
	}
	va_end(args);
	*pos = 0;
	con_out->output_string(con_out, buf);
	if (color >= 0)
		con_out->set_attribute(con_out, EFI_LIGHTGRAY);
}

/*
 * Reads an Unicode character from the input device.
 *
 * Return: Unicode character
 */
u16 efi_st_get_key(void)
{
	struct efi_input_key input_key;
	efi_status_t ret;

	/* Wait for next key */
	do {
		ret = con_in->read_key_stroke(con_in, &input_key);
	} while (ret == EFI_NOT_READY);
	return input_key.unicode_char;
}
