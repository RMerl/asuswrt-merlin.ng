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

/**
 * Controller for the Connection Editor window
 */
@interface ConnController : NSWindowController {

	/**
	 * Text field for the connection name
	 */
	IBOutlet NSTextField *name;

	/**
	 * Authentication method select control
	 */
	IBOutlet NSPopUpButton *auth;

	/**
	 * Server address of connection
	 */
	IBOutlet NSTextField *server;

	/**
	 * Username used for client authentication
	 */
	IBOutlet NSTextField *user;

	/**
	 * Confirmation button to save/create connection
	 */
	IBOutlet NSButton *ok;
}

/**
 * Save the currently edited connection
 */
- (IBAction)saveConnEditor:(id)sender;

/**
 * Cancel editing the current connection
 */
- (IBAction)cancelConnEditor:(id)sender;

/**
 * Open a dialog to create a new connection.
 *
 * @return		dictionary with connection settings, retained
 */
- (NSMutableDictionary*)createConnection;

/**
 * Open a dialog to edit a connection
 *
 * @param conn	dictionary with connection settings, gets updated
 * @return		TRUE if connection has been updated, FALSE if aborted
 */
- (bool)editConnection:(NSMutableDictionary*)conn;

@end
