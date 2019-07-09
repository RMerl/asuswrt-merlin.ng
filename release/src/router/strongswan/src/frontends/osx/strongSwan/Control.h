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

#import <Foundation/Foundation.h>

/**
 * State of a connection during connect.
 */
typedef enum {
	/** The connection is currently down */
	STATE_DOWN,
	/** The connection is getting established, the daemon is working */
	STATE_WORKING,
	/** The connection has been established successfully */
	STATE_UP,
} connectionState;

/**
 * This protocol defines callback functions to invoke during connect.
 */
@protocol ControlDelegate

/**
 * The connection has changed its state
 *
 * @param state		new connection state
 * @param conn		connection that changed its state
 */
- (void)change:(connectionState)state withConnection:(NSDictionary*)conn;

/**
 * A password is required for authentication
 *
 * @param conn		connection a password is required for
 * @return			a retained password string
 */
- (NSString*)createPasswordWithConnection:(NSDictionary*)conn;

/**
 * Show an alert message to the user
 *
 * @param alert		alert message string
 * @param conn		connection an error has occurred
 */
- (void)raise:(NSString*)alert withConnection:(NSDictionary*)conn;

/**
 * Log a line for a specific connection
 *
 * @param line		log line string
 * @param conn		connection the line is logged for
 */
- (void)log:(NSString*)line withConnection:(NSDictionary*)conn;
@end

/**
 * Implements connection connect/disconnect operations using a GUI delegate.
 */
@interface Control : NSObject

/**
 * Try to establish a connection
 *
 * @param conn		connection configuration to initiate
 */
- (void)connect:(NSDictionary*)conn;

/**
 * Disconnect a previously established connection
 *
 * @param conn		connection configuration to terminate
 */
- (void)disconnect:(NSDictionary*)conn;

/**
 * Initiate the Control class using a delegate for GUI operations
 *
 * @param delegate	delegate to invoke callbacks on during connect operation
 * @return			class instance
 */
- (id)initWithDelegate:(id<ControlDelegate>)delegate;

@end
