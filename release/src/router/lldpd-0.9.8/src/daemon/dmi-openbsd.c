/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2012 Vincent Bernat <bernat@luffy.cx>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "lldpd.h"
#include <sys/param.h>
#include <sys/sysctl.h>

#ifdef ENABLE_LLDPMED

char*
dmi_get(int what, const char *descr)
{
	char result[100] = {};
	size_t len = sizeof(result) - 1;
	int mib[2] = {
		CTL_HW,
		what
	};
	if (sysctl(mib, 2, result, &len, NULL, 0) == -1) {
		log_debug("localchassis", "cannot get %s",
		    descr);
		return NULL;
	}
	log_debug("localchassis", "got `%s` for %s",
	    result, descr);
	return strdup(result);
}

char*
dmi_hw()
{
	return dmi_get(HW_VERSION, "hardware revision");
}

char*
dmi_fw()
{
	return NULL;
}

char*
dmi_sn()
{
	return dmi_get(HW_SERIALNO, "serial number");
}

char*
dmi_manuf()
{
	return dmi_get(HW_VENDOR, "hardware vendor");
}

char*
dmi_model()
{
	return dmi_get(HW_PRODUCT, "hardware product");
}

char*
dmi_asset()
{
	return dmi_get(HW_UUID, "hardware UUID");
}
#endif
