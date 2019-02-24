/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "osx_attr_handler.h"

#include <networking/host.h>
#include <utils/debug.h>

#include <SystemConfiguration/SCDynamicStore.h>

typedef struct private_osx_attr_handler_t private_osx_attr_handler_t;

/**
 * Private data of an osx_attr_handler_t object.
 */
struct private_osx_attr_handler_t {

	/**
	 * Public interface
	 */
	osx_attr_handler_t public;

	/**
	 * Backup of original DNS servers, before we mess with it
	 */
	CFMutableArrayRef original;

	/**
	 * Append DNS servers to existing entries, instead of replacing
	 */
	bool append;
};

/**
 * Create a path to the DNS configuration of the Primary IPv4 Service
 */
static CFStringRef create_dns_path(SCDynamicStoreRef store)
{
	CFStringRef service, path = NULL;
	CFDictionaryRef dict;

	/* get primary service */
	dict = SCDynamicStoreCopyValue(store, CFSTR("State:/Network/Global/IPv4"));
	if (dict)
	{
		service = CFDictionaryGetValue(dict, CFSTR("PrimaryService"));
		if (service)
		{
			path = CFStringCreateWithFormat(NULL, NULL,
								CFSTR("State:/Network/Service/%@/DNS"), service);
		}
		else
		{
			DBG1(DBG_CFG, "SystemConfiguration PrimaryService not known");
		}
		CFRelease(dict);
	}
	else
	{
		DBG1(DBG_CFG, "getting global IPv4 SystemConfiguration failed");
	}
	return path;
}

/**
 * Create a mutable dictionary from path, a new one if not found
 */
static CFMutableDictionaryRef get_dictionary(SCDynamicStoreRef store,
											 CFStringRef path)
{
	CFDictionaryRef dict;
	CFMutableDictionaryRef mut = NULL;

	dict = SCDynamicStoreCopyValue(store, path);
	if (dict)
	{
		if (CFGetTypeID(dict) == CFDictionaryGetTypeID())
		{
			mut = CFDictionaryCreateMutableCopy(NULL, 0, dict);
		}
		CFRelease(dict);
	}
	if (!mut)
	{
		mut = CFDictionaryCreateMutable(NULL, 0,
										&kCFTypeDictionaryKeyCallBacks,
										&kCFTypeDictionaryValueCallBacks);
	}
	return mut;
}

/**
 * Create a mutable array from dictionary path, a new one if not found
 */
static CFMutableArrayRef get_array_from_dict(CFDictionaryRef dict,
											 CFStringRef name)
{
	CFArrayRef arr;

	arr = CFDictionaryGetValue(dict, name);
	if (arr && CFGetTypeID(arr) == CFArrayGetTypeID())
	{
		return CFArrayCreateMutableCopy(NULL, 0, arr);
	}
	return CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
}

/**
 * Add/Remove a DNS server to the configuration
 */
static bool manage_dns(private_osx_attr_handler_t *this,
					   int family, chunk_t data, bool add)
{
	SCDynamicStoreRef store;
	CFStringRef path, dns;
	CFMutableArrayRef arr;
	CFMutableDictionaryRef dict;
	CFIndex i;
	host_t *server;
	char buf[64];
	bool success = FALSE;

	server = host_create_from_chunk(family, data, 0);
	if (!server)
	{
		return FALSE;
	}
	snprintf(buf, sizeof(buf), "%H", server);
	server->destroy(server);

	store = SCDynamicStoreCreate(NULL, CFSTR("osx-attr"), NULL, NULL);
	path = create_dns_path(store);
	if (path)
	{
		dict = get_dictionary(store, path);
		arr = get_array_from_dict(dict, CFSTR("ServerAddresses"));
		dns = CFStringCreateWithCString(NULL, buf, kCFStringEncodingUTF8);
		if (add)
		{
			if (!this->append && !this->original)
			{	/* backup original config, start with empty set */
				this->original = arr;
				arr = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
			}
			DBG1(DBG_CFG, "installing %s as DNS server", buf);
			CFArrayInsertValueAtIndex(arr, 0, dns);
		}
		else
		{
			i = CFArrayGetFirstIndexOfValue(arr,
									CFRangeMake(0, CFArrayGetCount(arr)), dns);
			if (i >= 0)
			{
				DBG1(DBG_CFG, "removing %s from DNS servers (%d)", buf, i);
				CFArrayRemoveValueAtIndex(arr, i);
			}
			if (!this->append && this->original && CFArrayGetCount(arr) == 0)
			{	/* restore original config */
				CFRelease(arr);
				arr = this->original;
				this->original = NULL;
			}
		}
		CFRelease(dns);
		CFDictionarySetValue(dict, CFSTR("ServerAddresses"), arr);
		CFRelease(arr);

		success = SCDynamicStoreSetValue(store, path, dict);
		CFRelease(dict);
		CFRelease(path);
	}
	CFRelease(store);

	if (!success)
	{
		DBG1(DBG_CFG, "adding DNS server to SystemConfiguration failed");
	}
	return success;
}

METHOD(attribute_handler_t, handle, bool,
	private_osx_attr_handler_t *this, ike_sa_t *ike_sa,
	configuration_attribute_type_t type, chunk_t data)
{
	switch (type)
	{
		case INTERNAL_IP4_DNS:
			return manage_dns(this, AF_INET, data, TRUE);
		default:
			return FALSE;
	}
}

METHOD(attribute_handler_t, release, void,
	private_osx_attr_handler_t *this, ike_sa_t *ike_sa,
	configuration_attribute_type_t type, chunk_t data)
{
	switch (type)
	{
		case INTERNAL_IP4_DNS:
			manage_dns(this, AF_INET, data, FALSE);
			break;
		default:
			break;
	}
}

METHOD(enumerator_t, enumerate_dns, bool,
	enumerator_t *this, va_list args)
{
	configuration_attribute_type_t *type;
	chunk_t *data;

	VA_ARGS_VGET(args, type, data);
	*type = INTERNAL_IP4_DNS;
	*data = chunk_empty;
	this->venumerate = (void*)return_false;
	return TRUE;
}

METHOD(attribute_handler_t, create_attribute_enumerator, enumerator_t *,
	private_osx_attr_handler_t *this, ike_sa_t *ike_sa,
	linked_list_t *vips)
{
	enumerator_t *enumerator;

	INIT(enumerator,
		.enumerate = enumerator_enumerate_default,
		.venumerate = _enumerate_dns,
		.destroy = (void*)free,
	);
	return enumerator;
}

METHOD(osx_attr_handler_t, destroy, void,
	private_osx_attr_handler_t *this)
{
	free(this);
}

/**
 * See header
 */
osx_attr_handler_t *osx_attr_handler_create()
{
	private_osx_attr_handler_t *this;

	INIT(this,
		.public = {
			.handler = {
				.handle = _handle,
				.release = _release,
				.create_attribute_enumerator = _create_attribute_enumerator,
			},
			.destroy = _destroy,
		},
		.append = lib->settings->get_bool(lib->settings,
								"%s.plugins.osx-attr.append", TRUE, lib->ns),
	);

	return &this->public;
}
