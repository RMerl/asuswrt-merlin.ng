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

#import "Control.h"
#import "Helper.h"

@implementation Control {
	/* Provides the XPC main connection to the privileged helper */
	Helper *helper;
	/* active connection specific XPC channels, indexed by connection name */
	NSDictionary *active;
	/* delegate to invoke callbacks on */
	id<ControlDelegate> del;
}

- (NSString*)translateAlert:(const char*)alert
{
	if (strcmp(alert, "local-auth") == 0)
	{
		return @"Client authentication failed";
	}
	if (strcmp(alert, "remote-auth") == 0)
	{
		return @"Server authentication failed";
	}
	if (strcmp(alert, "dns") == 0)
	{
		return @"Resolving server hostname failed";
	}
	if (strcmp(alert, "unreachable") == 0)
	{
		return @"Server is unreachable";
	}
	if (strcmp(alert, "timeout") == 0)
	{
		return @"Server did not respond";
	}
	if (strcmp(alert, "proposal-mismatch") == 0)
	{
		return @"No common cryptographic algorithms found";
	}
	if (strcmp(alert, "ts-mismatch") == 0)
	{
		return @"No common traffic selectors found";
	}
	return NULL;
}

- (void)changeState:(connectionState)state withChannel:(xpc_connection_t)channel
			andConn:(NSDictionary*)conn
{
	NSString *name;

	name = [conn objectForKey:@"name"];
	switch (state)
	{
		case STATE_WORKING:
			xpc_dictionary_set_value(active, [name UTF8String], channel);
			break;
		case STATE_DOWN:
			xpc_dictionary_set_value(active, [name UTF8String], NULL);
			break;
		case STATE_UP:
			break;
	}
	dispatch_sync(dispatch_get_main_queue(), ^{
		[del change:state withConnection:conn];
	});
}

- (void)handleEvent:(xpc_object_t)request withChannel:(xpc_connection_t)channel
			andConn:(NSDictionary*)conn
{
	xpc_connection_t client;
	xpc_object_t reply;
	const char *type, *rpc, *event;
	client = xpc_dictionary_get_remote_connection(request);
	type = xpc_dictionary_get_string(request, "type");
	if (type)
	{
		if (strcmp(type, "rpc") == 0)
		{
			reply = xpc_dictionary_create_reply(request);
			rpc = xpc_dictionary_get_string(request, "rpc");
			if (rpc)
			{
				if (strcmp(rpc, "get_password") == 0)
				{
					__block NSString *password;

					dispatch_sync(dispatch_get_main_queue(), ^{
						password = [del createPasswordWithConnection:conn];
					});
					xpc_dictionary_set_string(reply, "password",
											  [password UTF8String]);
					[password release];
				}
			}
			xpc_connection_send_message(client, reply);
			xpc_release(reply);
		}
		if (strcmp(type, "event") == 0)
		{
			event = xpc_dictionary_get_string(request, "event");
			if (event)
			{
				if (strcmp(event, "log") == 0)
				{
					NSString *line;

					line = [[NSString alloc] initWithUTF8String:
							xpc_dictionary_get_string(request, "message")];
					dispatch_async(dispatch_get_main_queue(), ^{
						[del log:line withConnection:conn];
					});
					[line release];
				}
				if (strcmp(event, "alert") == 0)
				{
					NSString *msg;
					const char *str;

					str = xpc_dictionary_get_string(request, "alert");
					if (str)
					{
						msg = [self translateAlert:str];
						if (msg)
						{
							dispatch_async(dispatch_get_main_queue(), ^{
								[del raise:msg withConnection:conn];
							});
						}
					}
				}
				if (strcmp(event, "connecting") == 0)
				{
					[self changeState:STATE_WORKING withChannel:channel andConn:conn];
				}
				if (strcmp(event, "up") == 0)
				{
					/* IKE_SA up */
				}
				if (strcmp(event, "down") == 0)
				{
					[self changeState:STATE_DOWN withChannel:channel andConn:conn];
				}
				if (strcmp(event, "child_up") == 0)
				{
					[self changeState:STATE_UP withChannel:channel andConn:conn];
				}
				if (strcmp(event, "child_down") == 0)
				{
					[self changeState:STATE_DOWN withChannel:channel andConn:conn];
				}
			}
		}
	}
}

- (void)connect:(NSDictionary*)conn
{
	xpc_object_t request;
	xpc_connection_t service, daemon;
	NSString *name, *server, *username;

	name = [conn objectForKey:@"name"];
	server = [conn objectForKey:@"server"];
	username = [conn objectForKey:@"username"];

	daemon = [helper getConnection];
	if (!daemon)
	{
		[del raise:[helper getError] withConnection:conn];
	}

	request = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(request, "type", "rpc");
	xpc_dictionary_set_string(request, "rpc", "start_connection");
	xpc_dictionary_set_string(request, "name", [name UTF8String]);
	xpc_dictionary_set_string(request, "host", [server UTF8String]);
	xpc_dictionary_set_string(request, "id", [username UTF8String]);

	service = xpc_connection_create(NULL, NULL);
	xpc_connection_set_event_handler(service, ^(xpc_object_t channel) {

		xpc_connection_set_event_handler(channel, ^(xpc_object_t event) {

			if (event == XPC_ERROR_CONNECTION_INTERRUPTED ||
				event == XPC_ERROR_CONNECTION_INVALID)
			{
				xpc_dictionary_set_value(active, [name UTF8String], NULL);
				dispatch_sync(dispatch_get_main_queue(), ^{
					[del change:STATE_DOWN withConnection:conn];
				});
			}
			else
			{
				[self handleEvent:event withChannel:channel andConn:conn];
			}
		});
		xpc_connection_resume(channel);
	});

	xpc_connection_resume(service);
	xpc_dictionary_set_connection(request, "channel", xpc_retain(service));
	xpc_connection_send_message_with_reply(daemon, request,
										   dispatch_get_main_queue(),
										   ^(xpc_object_t reply) {});
	xpc_release(request);
}

- (void)disconnect:(NSDictionary*)conn
{
	xpc_connection_t tunnel;
	xpc_object_t request;

	tunnel = xpc_dictionary_get_value(active,
									  [[conn objectForKey:@"name"] UTF8String]);
	if (tunnel)
	{
		request = xpc_dictionary_create(NULL, NULL, 0);
		xpc_dictionary_set_string(request, "type", "rpc");
		xpc_dictionary_set_string(request, "rpc", "stop_connection");
		xpc_connection_send_message_with_reply(tunnel, request,
											   dispatch_get_main_queue(),
											   ^(xpc_object_t reply) {});
		xpc_release(request);
	}
}

- (void)dealloc
{
	xpc_release(active);
	[super dealloc];
}

- (id)initWithDelegate:(id<ControlDelegate>)delegate;
{
	self = [super init];
	helper = [[Helper alloc] init];
	active = xpc_dictionary_create(NULL, NULL, 0);
	del = delegate;
	return self;
}

@end
