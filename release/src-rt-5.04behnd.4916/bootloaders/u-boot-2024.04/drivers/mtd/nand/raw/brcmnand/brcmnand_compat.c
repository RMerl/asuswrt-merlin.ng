// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <dm/devres.h>
#include "brcmnand_compat.h"

static char *devm_kvasprintf(struct udevice *dev, gfp_t gfp, const char *fmt,
			     va_list ap)
{
	unsigned int len;
	char *p;
	va_list aq;

	va_copy(aq, ap);
	len = vsnprintf(NULL, 0, fmt, aq);
	va_end(aq);

	p = devm_kmalloc(dev, len + 1, gfp);
	if (!p)
		return NULL;

	vsnprintf(p, len + 1, fmt, ap);

	return p;
}

char *devm_kasprintf(struct udevice *dev, gfp_t gfp, const char *fmt, ...)
{
	va_list ap;
	char *p;

	va_start(ap, fmt);
	p = devm_kvasprintf(dev, gfp, fmt, ap);
	va_end(ap);

	return p;
}
