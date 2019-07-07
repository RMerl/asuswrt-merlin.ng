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

#import "ConnController.h"

@interface ConnController ()

@end

@implementation ConnController

- (void)moveWindowToActiveSpace
{
	NSInteger behavior, old;

	/* seems that NSWindowCollectionBehaviorMoveToActiveSpace does not work
	 * when a window is opened a second time. Fix that by changing the
	 * behavior forth and back. */
	old = behavior = [[self window] collectionBehavior];
	behavior &= ~NSWindowCollectionBehaviorMoveToActiveSpace;
	behavior |= NSWindowCollectionBehaviorCanJoinAllSpaces;

	[[self window] setCollectionBehavior: behavior];
	[[self window] setCollectionBehavior: old];

	[NSApp activateIgnoringOtherApps:YES];
}

- (IBAction)saveConnEditor:(id)sender
{
	[NSApp stopModal];
}

- (IBAction)cancelConnEditor:(id)sender
{
    [NSApp abortModal];
}

- (NSMutableDictionary*)createConnection
{
	NSMutableDictionary *conn = nil;

	[[self window] setTitle:@"Add new connection"];
	[name setStringValue:@""];
	[server setStringValue:@""];
	[user setStringValue:@""];
	[ok setEnabled:FALSE];
	[self moveWindowToActiveSpace];
	if ([NSApp runModalForWindow: [self window]] == NSRunStoppedResponse)
	{
		conn = [NSMutableDictionary dictionaryWithObjectsAndKeys:
				[name stringValue], @"name",
				[server stringValue], @"server",
				[user stringValue], @"username",
				nil];
	}
	[[self window] orderOut: self];
	return conn;
}

- (bool)editConnection:(NSMutableDictionary*)conn
{
	bool edited = NO;

	[[self window] setTitle:@"Edit connection"];
	[name setStringValue:[conn objectForKey:@"name"]];
	[server setStringValue:[conn objectForKey:@"server"]];
	[user setStringValue:[conn objectForKey:@"username"]];
	[ok setEnabled:TRUE];
	[self moveWindowToActiveSpace];
	if ([NSApp runModalForWindow: [self window]] == NSRunStoppedResponse)
	{
		[conn setObject:[name stringValue] forKey:@"name"];
		[conn setObject:[server stringValue] forKey:@"server"];
		[conn setObject:[user stringValue] forKey:@"username"];
		edited = YES;
	}
	[[self window] orderOut: self];
	return edited;
}

- (void)controlTextDidChange:(NSNotification *)notification
{
	[ok setEnabled:
	 [[name stringValue] length] &&
	 [[server stringValue] length] &&
	 [[user stringValue] length]];
}

@end
