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
 * Privileged Helper abstraction.
 *
 * Manages the installation of the privileged helper charon-xpc binary
 * using authorization, SMJobBless() installation using XPC and also
 * checks the installed helper version.
 */
@interface Helper : NSObject

/**
 * Get the XPC connection singleton, installing helper if required
 *
 * @return		XPC service connection, as a singleton
 */
- (xpc_connection_t)getConnection;

/**
 * Return an error string if if getConnection fails
 *
 * @return		error string, unretained
 */
- (NSString*)getError;

@end