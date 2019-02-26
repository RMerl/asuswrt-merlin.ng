/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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

/**
 * @defgroup xauth_pam xauth_pam
 * @ingroup cplugins
 *
 * @defgroup xauth_pam_plugin xauth_pam_plugin
 * @{ @ingroup xauth_pam
 */

#ifndef XAUTH_PAM_PLUGIN_H_
#define XAUTH_PAM_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct xauth_pam_plugin_t xauth_pam_plugin_t;

/**
 * XAuth plugin using Pluggable Authentication Modules to verify credentials.
 */
struct xauth_pam_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** XAUTH_PAM_PLUGIN_H_ @}*/
