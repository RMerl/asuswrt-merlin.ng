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

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

#ifdef ENABLE_LLDPMED
static char*
dmi_get(const char *classname, CFStringRef property)
{
	char *result = NULL;
	CFMutableDictionaryRef matching = NULL;
	CFTypeRef cfres = NULL;
	io_service_t service = 0;
	matching = IOServiceMatching(classname);
	if (!matching) {
		log_debug("localchassis", "cannot get %s class from registry",
		    classname);
		goto end;
	}
	service = IOServiceGetMatchingService(kIOMasterPortDefault, matching);
	if (!service) {
		log_warnx("localchassis", "cannot get matching %s class from registry",
			classname);
		goto end;
	}
	cfres = IORegistryEntryCreateCFProperty(service, property,
	    kCFAllocatorDefault, kNilOptions);
	if (!cfres) {
		log_debug("localchassis", "cannot find property %s in class %s in registry",
		    CFStringGetCStringPtr(property, kCFStringEncodingMacRoman),
		    classname);
		goto end;
	}

	if (CFGetTypeID(cfres) == CFStringGetTypeID())
		result = strdup(CFStringGetCStringPtr((CFStringRef)cfres, kCFStringEncodingMacRoman));
	else if (CFGetTypeID(cfres) == CFDataGetTypeID()) {
		/* OK, we know this is a string. */
		result = calloc(1, CFDataGetLength((CFDataRef)cfres) + 1);
		if (!result) goto end;
		memcpy(result, CFDataGetBytePtr((CFDataRef)cfres),
		    CFDataGetLength((CFDataRef)cfres));
	} else log_debug("localchassis", "unknown type for property %s in class %s",
	    CFStringGetCStringPtr(property, kCFStringEncodingMacRoman),
	    classname);

end:
	if (cfres) CFRelease(cfres);
	if (service) IOObjectRelease(service);
	return result;
}

char*
dmi_hw()
{
	return dmi_get("IOPlatformExpertDevice", CFSTR("version"));
}

char*
dmi_fw()
{
	/* Dunno where it is. Maybe in SMC? */
	return NULL;
}

char*
dmi_sn()
{
	return dmi_get("IOPlatformExpertDevice", CFSTR("IOPlatformSerialNumber"));
}

char*
dmi_manuf()
{
	return dmi_get("IOPlatformExpertDevice", CFSTR("manufacturer"));
}

char*
dmi_model()
{
	return dmi_get("IOPlatformExpertDevice", CFSTR("model"));
}

char*
dmi_asset()
{
	return dmi_get("IOPlatformExpertDevice", CFSTR("board-id"));
}
#endif
