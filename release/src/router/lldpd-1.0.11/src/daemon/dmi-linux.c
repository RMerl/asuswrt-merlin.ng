/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2009 Vincent Bernat <bernat@luffy.cx>
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
#include <unistd.h>

#ifdef ENABLE_LLDPMED
	/* Fill in inventory stuff:
	    - hardware version: /sys/class/dmi/id/product_version
	    - firmware version: /sys/class/dmi/id/bios_version
	    - software version: `uname -r`
	    - serial number: /sys/class/dmi/id/product_serial
	    - manufacturer: /sys/class/dmi/id/sys_vendor
	    - model: /sys/class/dmi/id/product_name
	    - asset: /sys/class/dmi/id/chassis_asset_tag
	*/

static char*
dmi_get(char *file)
{
	int dmi, s;
	char buffer[100] = {};

	log_debug("localchassis", "DMI request for file %s", file);
	if ((dmi = priv_open(file)) < 0) {
		log_debug("localchassis", "cannot get DMI file %s", file);
		return NULL;
	}
	if ((s = read(dmi, buffer, sizeof(buffer))) == -1) {
		log_debug("localchassis", "cannot read DMI file %s", file);
		close(dmi);
		return NULL;
	}
	close(dmi);
	buffer[sizeof(buffer) - 1] = '\0';
	if ((s > 0) && (buffer[s-1] == '\n'))
		buffer[s-1] = '\0';
	if (strlen(buffer))
		return strdup(buffer);
	return NULL;
}

char*
dmi_hw()
{
	return dmi_get(SYSFS_CLASS_DMI "product_version");
}

char*
dmi_fw()
{
	return dmi_get(SYSFS_CLASS_DMI "bios_version");
}

char*
dmi_sn()
{
	return dmi_get(SYSFS_CLASS_DMI "product_serial");
}

char*
dmi_manuf()
{
	return dmi_get(SYSFS_CLASS_DMI "sys_vendor");
}

char*
dmi_model()
{
	return dmi_get(SYSFS_CLASS_DMI "product_name");
}

char*
dmi_asset()
{
	return dmi_get(SYSFS_CLASS_DMI "chassis_asset_tag");
}
#endif
