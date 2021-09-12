// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "highlighter.h"

#define TERMINAL_FG_BLACK	"\x1b[30m"
#define TERMINAL_FG_RED		"\x1b[31m"
#define TERMINAL_FG_GREEN	"\x1b[32m"
#define TERMINAL_FG_YELLOW	"\x1b[33m"
#define TERMINAL_FG_BLUE	"\x1b[34m"
#define TERMINAL_FG_MAGENTA	"\x1b[35m"
#define TERMINAL_FG_CYAN	"\x1b[36m"
#define TERMINAL_FG_WHITE	"\x1b[37m"
#define TERMINAL_FG_DEFAULT	"\x1b[39m"

#define TERMINAL_BG_BLACK	"\x1b[40m"
#define TERMINAL_BG_RED		"\x1b[41m"
#define TERMINAL_BG_GREEN	"\x1b[42m"
#define TERMINAL_BG_YELLOW	"\x1b[43m"
#define TERMINAL_BG_BLUE	"\x1b[44m"
#define TERMINAL_BG_MAGENTA	"\x1b[45m"
#define TERMINAL_BG_CYAN	"\x1b[46m"
#define TERMINAL_BG_WHITE	"\x1b[47m"
#define TERMINAL_BG_DEFAULT	"\x1b[49m"

#define TERMINAL_BOLD		"\x1b[1m"
#define TERMINAL_NO_BOLD	"\x1b[22m"
#define TERMINAL_UNDERLINE	"\x1b[4m"
#define TERMINAL_NO_UNDERLINE	"\x1b[24m"

#define TERMINAL_RESET		"\x1b[0m"

static const char *colormap[] = {
	[HighlightSection] = TERMINAL_FG_BLACK TERMINAL_BOLD,
	[HighlightField] = TERMINAL_FG_BLUE TERMINAL_BOLD,
	[HighlightPrivateKey] = TERMINAL_FG_YELLOW TERMINAL_BOLD,
	[HighlightPublicKey] = TERMINAL_FG_YELLOW TERMINAL_BOLD,
	[HighlightPresharedKey] = TERMINAL_FG_YELLOW TERMINAL_BOLD,
	[HighlightIP] = TERMINAL_FG_GREEN,
	[HighlightCidr] = TERMINAL_FG_YELLOW,
	[HighlightHost] = TERMINAL_FG_GREEN TERMINAL_BOLD,
	[HighlightPort] = TERMINAL_FG_MAGENTA,
	[HighlightMTU] = TERMINAL_FG_BLUE,
	[HighlightKeepalive] = TERMINAL_FG_BLUE,
	[HighlightComment] = TERMINAL_FG_CYAN,
	[HighlightDelimiter] = TERMINAL_FG_CYAN,
#ifndef MOBILE_WGQUICK_SUBSET
	[HighlightTable] = TERMINAL_FG_BLUE,
	[HighlightFwMark] = TERMINAL_FG_BLUE,
	[HighlightSaveConfig] = TERMINAL_FG_BLUE,
	[HighlightCmd] = TERMINAL_FG_WHITE,
#endif
	[HighlightError] = TERMINAL_FG_RED TERMINAL_UNDERLINE
};

int main(int argc, char *argv[])
{
	char input[1024 * 1024];
	struct highlight_span *spans;
	size_t last = 0, total_len;

	total_len = fread(input, 1, sizeof(input) - 1, stdin);
	input[total_len] = '\0';
	spans = highlight_config(input);

	fputs(TERMINAL_RESET, stdout);
	for (struct highlight_span *span = spans; span->type != HighlightEnd; ++span) {
		fwrite(input + last, 1, span->start - last, stdout);
		fputs(colormap[span->type], stdout);
		fwrite(input + span->start, 1, span->len, stdout);
		fputs(TERMINAL_RESET, stdout);
		last = span->start + span->len;
	}
	fwrite(input + last, 1, total_len - last, stdout);

	free(spans);
	return 0;
}
