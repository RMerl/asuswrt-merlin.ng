/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#include <stdbool.h>
#include <inttypes.h>

bool use_color(void);

#define COLOR_OFF	"\x1B[0m"
#define COLOR_BLACK	"\x1B[0;30m"
#define COLOR_RED	"\x1B[0;31m"
#define COLOR_GREEN	"\x1B[0;32m"
#define COLOR_YELLOW	"\x1B[0;33m"
#define COLOR_BLUE	"\x1B[0;34m"
#define COLOR_MAGENTA	"\x1B[0;35m"
#define COLOR_CYAN	"\x1B[0;36m"
#define COLOR_WHITE	"\x1B[0;37m"
#define COLOR_WHITE_BG	"\x1B[0;47;30m"
#define COLOR_HIGHLIGHT	"\x1B[1;39m"

#define COLOR_RED_BOLD		"\x1B[1;31m"
#define COLOR_GREEN_BOLD	"\x1B[1;32m"
#define COLOR_BLUE_BOLD		"\x1B[1;34m"
#define COLOR_MAGENTA_BOLD	"\x1B[1;35m"

#define COLOR_ERROR	"\x1B[1;31m"
#define COLOR_WARN	"\x1B[1m"
#define COLOR_INFO	COLOR_OFF
#define COLOR_DEBUG	COLOR_WHITE

#define FALLBACK_TERMINAL_WIDTH 80

#define print_indent(indent, color1, prefix, title, color2, fmt, args...) \
do { \
	printf("%*c%s%s%s%s" fmt "%s\n", (indent), ' ', \
		use_color() ? (color1) : "", prefix, title, \
		use_color() ? (color2) : "", ## args, \
		use_color() ? COLOR_OFF : ""); \
} while (0)

#define print_text(color, fmt, args...) \
		print_indent(8, COLOR_OFF, "", "", color, fmt, ## args)

#define print_field(fmt, args...) \
		print_indent(8, COLOR_OFF, "", "", COLOR_OFF, fmt, ## args)

struct bitfield_data {
	uint64_t bit;
	const char *str;
};

static inline uint64_t print_bitfield(int indent, uint64_t val,
					const struct bitfield_data *table)
{
	uint64_t mask = val;
	int i;

	for (i = 0; table[i].str; i++) {
		if (val & (((uint64_t) 1) << table[i].bit)) {
			print_field("%*c%s", indent, ' ', table[i].str);
			mask &= ~(((uint64_t) 1) << table[i].bit);
		}
	}

	return mask;
}

int num_columns(void);

void open_pager(void);
void close_pager(void);
