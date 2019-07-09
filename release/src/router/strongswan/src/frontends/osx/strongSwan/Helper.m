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

#import "Helper.h"

#include <ServiceManagement/ServiceManagement.h>
#include <Security/Authorization.h>

/* to check strongSwan version */
#include "../../../../config.h"

@implementation Helper {
	/* XPC connection to the helper */
	xpc_connection_t helper;
	/* authorization instance, if authorized */
	AuthorizationRef auth;
	/* string of last error occurred */
	NSString* errmsg;
}

- (void)setError:(NSString*)error
{
	if (errmsg)
	{
		[errmsg release];
	}
	errmsg = error;
}

- (bool)authorize
{
	AuthorizationItem item = { kSMRightBlessPrivilegedHelper, 0, NULL, 0 };
	AuthorizationRights rights = { 1, &item };
	OSStatus status;

	if (auth)
	{
		return TRUE;
	}
	status = AuthorizationCreate(&rights, kAuthorizationEmptyEnvironment,
								 kAuthorizationFlagDefaults |
								 kAuthorizationFlagInteractionAllowed |
								 kAuthorizationFlagExtendRights, &auth);
	if (status == errAuthorizationSuccess)
	{
		return TRUE;
	}
	if (errmsg)
	{
		[errmsg release];
	}
	[self setError:[NSString stringWithFormat:@"Authorization failed: %@",
					SecCopyErrorMessageString(status, NULL)]];
	return FALSE;
}

- (bool)manage:(CFStringRef)label bless:(bool)bless
{
	bool done;
	CFErrorRef error;

	if (![self authorize])
	{
		return FALSE;
	}
	if (bless)
	{
		done = SMJobBless(kSMDomainSystemLaunchd, label, auth, &error);
	}
	else
	{
		done = SMJobRemove(kSMDomainSystemLaunchd, label, auth, TRUE, &error);
	}
	if (!done)
	{
		[self setError:
		 [NSString stringWithFormat:@"Installing privileged helper failed: %@",
		  CFErrorCopyDescription(error)]];
		CFRelease(error);
	}
	return done;
}

- (bool)make:(CFStringRef)label
{
	char str[128];

	if (!CFStringGetCString(label, str, sizeof(str), kCFStringEncodingUTF8))
	{
		[self setError:@"converting XPC service name failed"];
		return FALSE;
	}
	helper = xpc_connection_create_mach_service(str, NULL,
							XPC_CONNECTION_MACH_SERVICE_PRIVILEGED);
	if (!helper)
	{
		[self setError:@"creating XPC mach service failed"];
		return FALSE;
	}

	xpc_connection_set_event_handler(helper, ^(xpc_object_t event) {
		if (xpc_get_type(event) == XPC_TYPE_ERROR)
		{
			if (event == XPC_ERROR_CONNECTION_INTERRUPTED ||
				event == XPC_ERROR_CONNECTION_INVALID)
			{
				if (helper)
				{
					xpc_connection_cancel(helper);
					helper = NULL;
				}
			}
		}
	});
	xpc_connection_resume(helper);

	return TRUE;
}

- (bool)checkVersion
{
	xpc_object_t request, response;
	bool match = FALSE;

	request = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(request, "type", "rpc");
	xpc_dictionary_set_string(request, "rpc", "get_version");

	response = xpc_connection_send_message_with_reply_sync(helper, request);
	xpc_release(request);
	if (xpc_get_type(response) == XPC_TYPE_DICTIONARY)
	{
		match = strcmp(xpc_dictionary_get_string(response, "version"),
					   PACKAGE_VERSION) == 0;
	}
	xpc_release(response);
	return match;
}

- (bool)makeAndCheck:(CFStringRef)label
{
	if (![self make:label])
	{
		return FALSE;
	}
	if ([self checkVersion])
	{
		return TRUE;
	}
	xpc_connection_cancel(helper);
	helper = NULL;
	/* version outdated, uninstall old helper */
	[self manage:label bless:FALSE];
	return FALSE;
}

- (bool)blessAndMake:(CFStringRef)label
{
#ifdef DEBUG
	/* always update helper when debugging */
	[self manage:label bless:FALSE];
	[self manage:label bless:TRUE];
#endif

	if (![self makeAndCheck:label])
	{
		if ([self manage:label bless:TRUE])
		{
			[self makeAndCheck:label];
		}
	}
	return helper;
}

- (xpc_connection_t)getConnection
{
	if (!helper)
	{
		[self blessAndMake:CFSTR("org.strongswan.charon-xpc")];
	}
	return helper;
}

- (NSString*)getError
{
	return errmsg;
}

- (id)init
{
	self = [super init];
	return self;
}

- (void)dealloc
{
	if (helper)
	{
		xpc_connection_cancel(helper);
	}
	if (auth)
	{
		AuthorizationFree(auth, kAuthorizationFlagDefaults);
	}
	if (errmsg)
	{
		[errmsg release];
	}
	[super dealloc];
}
@end
