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

#import <Cocoa/Cocoa.h>

#import "Control.h"

/**
 * Main application delegate, controls the status menu
 */
@interface AppDelegate : NSObject <NSApplicationDelegate,ControlDelegate> {

	/**
	 * Status menu controlled
	 */
	IBOutlet NSMenu *statusMenu;
}

/**
 * Action to add a new connection
 */
- (IBAction)addConnection:(id)sender;

@end
