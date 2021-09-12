/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include <sys/types.h>

enum highlight_type {
	HighlightSection,
	HighlightField,
	HighlightPrivateKey,
	HighlightPublicKey,
	HighlightPresharedKey,
	HighlightIP,
	HighlightCidr,
	HighlightHost,
	HighlightPort,
	HighlightMTU,
	HighlightKeepalive,
	HighlightComment,
	HighlightDelimiter,
#ifndef MOBILE_WGQUICK_SUBSET
	HighlightTable,
	HighlightFwMark,
	HighlightSaveConfig,
	HighlightCmd,
#endif
	HighlightError,
	HighlightEnd
};

struct highlight_span {
	enum highlight_type type;
	size_t start, len;
};

struct highlight_span *highlight_config(const char *config);
