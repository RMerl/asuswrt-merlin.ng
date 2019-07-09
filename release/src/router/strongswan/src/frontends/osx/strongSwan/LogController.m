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

#import "LogController.h"

@interface LogController ()

@end

@implementation LogController

- (void)appendLine:(NSString*)line
{
	NSAttributedString* attr;

	attr = [[NSAttributedString alloc] initWithString:line];
	[[textView textStorage] appendAttributedString:attr];
	[attr release];

	attr = [[NSAttributedString alloc] initWithString:@"\n"];
	[[textView textStorage] appendAttributedString:attr];
	[attr release];

	[textView scrollRangeToVisible:NSMakeRange([[textView string] length], 0)];
}

- (void)clear
{
	[textView setString:@""];
}

- (void)show
{
	[self showWindow:self];
	[[self window] makeKeyAndOrderFront:self];
	[[self window] orderFrontRegardless];
}

@end
