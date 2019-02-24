/*
 * Copyright (C) 2008-2009 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
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
 * @defgroup eap_aka_3gpp2 eap_aka_3gpp2
 * @ingroup cplugins
 *
 * @defgroup eap_aka_3gpp2_plugin eap_aka_3gpp2_plugin
 * @{ @ingroup eap_aka_3gpp2
 */

#ifndef EAP_AKA_3GPP2_PLUGIN_H_
#define EAP_AKA_3GPP2_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct eap_aka_3gpp2_plugin_t eap_aka_3gpp2_plugin_t;

/**
 * Plugin to provide a SIM card/provider using the 3GPP2 (S.S0055) standard.
 *
 * This plugin implements the standard of the 3GPP2 (S.S0055) and not the one
 * of 3GGP, completely in software using the libgmp library..
 * The shared key used for authentication is from ipsec.secrets. The
 * peers ID is used to query it.
 * The AKA mechanism uses sequence numbers to detect replay attacks. The
 * peer stores the sequence number normally in a USIM and accepts
 * incremental sequence numbers (incremental for lifetime of the USIM). To
 * prevent a complex sequence number management, this implementation uses
 * a sequence number derived from time. It is initialized to the startup
 * time of the daemon.
 * To enable time based SEQs, define SEQ_CHECK as 1. Default is to accept
 * any SEQ numbers. This allows an attacker to do replay attacks. But since
 * the server has proven his identity via IKE, such an attack is only
 * possible between server and AAA (if any).
 */
struct eap_aka_3gpp2_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

#endif /** EAP_AKA_3GPP2_PLUGIN_H_ @}*/
