// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include <stdlib.h>
#include <string.h>
#include "highlighter.h"

int LLVMFuzzerTestOneInput(const char *data, size_t size)
{
	char *str = strndup(data, size);
	if (!str)
		return 0;
	struct highlight_span *spans = highlight_config(str);
	if (!spans)
		return 0;
	for (struct highlight_span *span = spans; span->type != HighlightEnd; ++span);
	free(spans);
	free(str);
	return 0;
}
