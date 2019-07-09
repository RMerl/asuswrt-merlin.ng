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

#import "AppDelegate.h"
#import "Control.h"
#import "LogController.h"
#import "ConnController.h"
#import "PasswordController.h"

@implementation AppDelegate {
	NSStatusItem *statusItem;
	NSImage *active;
	NSImage *inactive;
	NSInteger nonConnectionItemCount;
	NSMutableArray *connections;
	Control *control;
	ConnController *editor;
	PasswordController *password;
}

- (void)change:(connectionState)state withConnection:(NSDictionary*)conn
{
	NSInteger idx;
	NSMenuItem *item;

	idx = [statusMenu indexOfItemWithRepresentedObject:conn];
	if (idx != -1)
	{
		item = [statusMenu itemAtIndex:idx];

		switch (state) {
			case STATE_DOWN:
				[item setState:NSOffState];
				[item setAction:@selector(doConnection:)];
				[[[item submenu] itemAtIndex:0] setAction:@selector(doConnection:)];
				[[[item submenu] itemAtIndex:0] setTitle:@"Connect"];
				[[[item submenu] itemAtIndex:1] setAction:@selector(editConnection:)];
				[[[item submenu] itemAtIndex:2] setAction:@selector(removeConnection:)];
				[self updateIcon];
				break;
			case STATE_WORKING:
				[item setState:NSMixedState];
				[item setAction:@selector(undoConnection:)];
				[[[item submenu] itemAtIndex:0] setAction:@selector(undoConnection:)];
				[[[item submenu] itemAtIndex:0] setTitle:@"Disconnect"];
				[[[item submenu] itemAtIndex:1] setAction:nil];
				[[[item submenu] itemAtIndex:2] setAction:nil];
				[self updateIcon];
				break;
			case STATE_UP:
				[item setState:NSOnState];
				[self updateIcon];
				break;
		}
	}
}

- (NSString*)createPasswordWithConnection:(NSDictionary*)conn
{
	return [password query];
}

- (void)raise:(NSString*)alert withConnection:(NSDictionary*)conn
{
	NSAlert *popup;
	NSString *text;

	text = [[NSString alloc]
			initWithFormat:@"Establishing connection %@ failed:",
			[conn objectForKey:@"name"]];
	popup = [[NSAlert alloc] init];
	[popup setMessageText:text];
	[popup setInformativeText:alert];
	[popup runModal];
	[text release];
	[popup release];
}

- (void)log:(NSString*)line withConnection:(NSDictionary*)conn
{
	[[self findLog:conn] appendLine:line];
}

- (IBAction)undoConnection:(id)sender
{
	NSDictionary *conn;

	conn = [sender representedObject];
	[control disconnect:conn];
}

- (IBAction)doConnection:(id)sender
{
    NSDictionary *conn;

	conn = [sender representedObject];
	[[self findLog:conn] clear];
	[control connect:conn];
}

- (IBAction)addConnection:(id)sender
{
	NSMutableDictionary *conn;

	conn = [editor createConnection];
	if (conn)
	{
		[connections insertObject:conn atIndex:[connections count]];
		[self saveConnections];
	}
}

- (IBAction)editConnection:(id)sender
{
    NSMutableDictionary *conn;

	conn = [sender representedObject];
	if ([editor editConnection:conn])
	{
		[self saveConnections];
	}
}

- (IBAction)removeConnection:(id)sender
{
    NSDictionary *conn;

	conn = [sender representedObject];
	[connections removeObject:conn];
	[self saveConnections];
}

- (void)loadConnections
{
	NSUserDefaults *defaults;
	NSArray *array;

	defaults = [NSUserDefaults standardUserDefaults];
	array = [defaults arrayForKey:@"connections"];
	if (array)
	{
		[array enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
			if ([obj isKindOfClass:[NSDictionary class]])
			{
				NSMutableDictionary *dict;

				dict = [obj mutableCopy];
				[connections insertObject:dict atIndex:[connections count]];
				[dict release];
			}
		}];
	}
	[defaults addObserver:self
			   forKeyPath:@"connections"
				  options:NSKeyValueObservingOptionNew
				  context:NULL];
}

- (void)saveConnections
{
	NSUserDefaults *defaults;

	defaults = [NSUserDefaults standardUserDefaults];
	[defaults setObject:connections forKey:@"connections"];
	[defaults synchronize];
}

- (void)viewLog:(id)sender
{
	[[sender representedObject] show];
}

- (LogController*)findLog:(NSDictionary *)connection
{
	NSInteger idx;
	NSMenuItem *item;

	idx = [statusMenu indexOfItemWithRepresentedObject:connection];
	if (idx != -1)
	{
		item = [statusMenu itemAtIndex:idx];
		item = [[item submenu] itemAtIndex:3];
		return [item representedObject];
	}
	return nil;
}

- (void)addMenuItem:(NSDictionary *)connection
{
	NSString *name, *title;
	NSMenuItem *item;
	NSMenu *submenu;
	LogController *ctl;

	name = [connection objectForKey:@"name"];
	if (name) {
		/* create submenu first */
		submenu = [[NSMenu alloc] initWithTitle:@"submenu"];

		item = [[NSMenuItem alloc] initWithTitle:@"Connect"
										  action:@selector(doConnection:)
								   keyEquivalent:@""];
		[item setTarget:self];
		[item setRepresentedObject:connection];
		[submenu insertItem:item atIndex:0];
		[item release];

		item = [[NSMenuItem alloc] initWithTitle:@"Edit..."
										  action:@selector(editConnection:)
								   keyEquivalent:@""];
		[item setTarget:self];
		[item setRepresentedObject:connection];
		[submenu insertItem:item atIndex:1];
		[item release];

		item = [[NSMenuItem alloc] initWithTitle:@"Remove"
										  action:@selector(removeConnection:)
								   keyEquivalent:@""];
		[item setTarget:self];
		[item setRepresentedObject:connection];
		[submenu insertItem:item atIndex:2];
		[item release];

		item = [[NSMenuItem alloc] initWithTitle:@"View Log..."
										  action:@selector(viewLog:)
								   keyEquivalent:@""];
		[item setTarget:self];
		ctl = [[LogController alloc] initWithWindowNibName:@"LogWindow"];
		title = [NSString stringWithFormat: @"strongSwan Log: %@", name];
		[[ctl window] setTitle:title];
		[item setRepresentedObject:ctl];
		[ctl release];
		[submenu insertItem:item atIndex:3];
		[item release];

		item = [[NSMenuItem alloc] initWithTitle:name
									action:@selector(doConnection:)
									keyEquivalent:@""];
		[item setTarget:self];
		[item setRepresentedObject:connection];
		[item setSubmenu:submenu];
		[submenu release];
		[statusMenu insertItem:item atIndex:
		 [statusMenu numberOfItems] - nonConnectionItemCount];
		[item release];
	}
}

- (void)repopulateMenu
{
	while ([statusMenu numberOfItems] > nonConnectionItemCount)
	{
		[statusMenu removeItemAtIndex:0];
	}
	[connections enumerateObjectsUsingBlock:^(id obj, NSUInteger index, BOOL *stop) {
		if ([obj isKindOfClass:[NSDictionary class]]) {
			[self addMenuItem:obj];
		}
	}];
}

- (void)observeValueForKeyPath:(NSString *)keyPath
					  ofObject:(id)object change:(NSDictionary *)change
					   context:(void *)context
{
	[self repopulateMenu];
}

- (void)updateIcon
{
	NSMenuItem *item;
	bool anyactive = FALSE;
	int i;

	for (i = 0; i < [statusMenu numberOfItems] - nonConnectionItemCount; i++)
	{
		item = [statusMenu itemAtIndex:i];
		if ([item state] == NSOnState)
		{
			anyactive = TRUE;
			break;
		}
	}
	[statusItem setImage: anyactive ? active : inactive];
}

- (void)initializeMenu
{
	NSImage *icon;
	NSString *path;

	statusItem = [[[NSStatusBar systemStatusBar]
				   statusItemWithLength:NSVariableStatusItemLength] retain];
	[statusItem setMenu:statusMenu];
	[statusItem setHighlightMode:YES];

	path = [[NSBundle mainBundle] pathForResource:@"icon" ofType:@"png"];
	icon = [[NSImage alloc] initWithContentsOfFile:path];
	[statusItem setImage:icon];
	inactive = icon;

	path = [[NSBundle mainBundle] pathForResource:@"icon-active" ofType:@"png"];
	icon = [[NSImage alloc] initWithContentsOfFile:path];
	active = icon;

	path = [[NSBundle mainBundle] pathForResource:@"icon-alt" ofType:@"png"];
	icon = [[NSImage alloc] initWithContentsOfFile:path];
	[statusItem setAlternateImage:icon];
	[icon release];

	nonConnectionItemCount = [statusMenu numberOfItems];

	[self repopulateMenu];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	[self loadConnections];
	[self initializeMenu];
}

- (id)init
{
	self = [super init];
	control = [[Control alloc] initWithDelegate:self];
	connections = [[NSMutableArray alloc] init];
	editor = [[ConnController alloc] initWithWindowNibName:@"ConnWindow"];
	password = [[PasswordController alloc] initWithWindowNibName:@"PasswordWindow"];
	return self;
}

- (void)dealloc
{
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	[defaults removeObserver:self forKeyPath:@"connections"];
	[statusItem release];
	[active release];
	[inactive release];
	[connections release];
	[control release];
	[editor release];
	[password release];
	[super dealloc];
}

@end
