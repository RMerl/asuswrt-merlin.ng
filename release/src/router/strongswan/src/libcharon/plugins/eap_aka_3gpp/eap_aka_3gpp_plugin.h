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
/*
 * Copyright (C) 2015 Thomas Strangert
 * Polystar System AB, Sweden
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @defgroup eap_aka_3gpp eap_aka_3gpp
 * @ingroup cplugins
 *
 * @defgroup eap_aka_3gpp_plugin eap_aka_3gpp_plugin
 * @{ @ingroup eap_aka_3gpp
 */

#ifndef EAP_AKA_3GPP_PLUGIN_H_
#define EAP_AKA_3GPP_PLUGIN_H_

#include <plugins/plugin.h>

typedef struct eap_aka_3gpp_plugin_t eap_aka_3gpp_plugin_t;

/**
 * Plugin to provide a USIM card/provider according to the 3GPP standard.
 *
 * This plugin implements the 3GPP standards TS 35.205, .206, .207, .208
 * completely in software using the MILENAGE algorithm.
 * The shared keys used for authentication (K, OPc) are from ipsec.secrets.
 * The peers ID is used to query it.
 *
 * To enable SEQ sequence check by default define SEQ_CHECK. Left undefined/off,
 * it makes the USIM 'card' to accept any SEQ number, not comparing received
 * SQN with its own locally stored value. This potentially allows an attacker
 * to do replay attacks. But since the server has proven his identity via IKE,
 * such an attack is only possible between server and AAA (if any).
 * Note that SEQ_CHECK only controls the compile-time default behaviour,
 * but the run-time behaviour can always be controlled by setting the
 * charon.plugins.eap-aka-3gpp.seq_check config variable.
 */
struct eap_aka_3gpp_plugin_t {

	/**
	 * implements plugin interface
	 */
	plugin_t plugin;
};

/**
 * The AKA mechanism uses sequence numbers to detect replay attacks. The
 * peer stores the sequence number normally in a USIM and accepts
 * incremental sequence numbers (incremental for lifetime of the USIM). To
 * prevent a complex sequence number management, this implementation uses
 * a sequence number derived from time. It is initialized to the startup
 * time of the daemon. On the provider side, an offset can optionally be
 * added to allow for a time sqew towards the card side.
 */
#define SQN_TIME_OFFSET 180

#endif /** EAP_AKA_3GPP_PLUGIN_H_ @}*/
