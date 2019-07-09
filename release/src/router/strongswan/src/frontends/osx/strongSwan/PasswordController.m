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

#import "PasswordController.h"

@interface PasswordController ()

@end

@implementation PasswordController

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

- (IBAction)confirm:(id)sender
{
    [NSApp stopModal];
}

- (NSString *)query
{
	[password setStringValue:@""];
	[self moveWindowToActiveSpace];
	[NSApp runModalForWindow: [self window]];
	[[self window] orderOut: self];

	return [[password stringValue] retain];
}

@end
