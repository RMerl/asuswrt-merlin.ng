/*
 * Copyright (C) 2013 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
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

#include "cmd_options.h"

#include <getopt.h>

/**
 * See header.
 */
cmd_option_t cmd_options[CMD_OPT_COUNT] = {
	{ CMD_OPT_HELP, "help", no_argument, "",
	  "print this usage information and exit", {}},
	{ CMD_OPT_VERSION, "version", no_argument, "",
	  "show version information and exit", {}},
	{ CMD_OPT_DEBUG, "debug", required_argument, "level",
	  "set the default log level (-1..4, default: 1)", {}},
	{ CMD_OPT_HOST, "host", required_argument, "hostname",
	  "DNS name or address to connect to", {}},
	{ CMD_OPT_IDENTITY, "identity", required_argument, "identity",
	  "identity the client uses for the IKE exchange", {}},
	{ CMD_OPT_EAP_IDENTITY, "eap-identity", required_argument, "eap-identity",
	  "identity the client uses for EAP authentication", {}},
	{ CMD_OPT_XAUTH_USER, "xauth-username", required_argument, "xauth-username",
	  "username the client uses for XAuth authentication", {}},
	{ CMD_OPT_REMOTE_IDENTITY, "remote-identity", required_argument, "identity",
	  "server identity to expect, defaults to host", {}},
	{ CMD_OPT_CERT, "cert", required_argument, "path",
	  "certificate for authentication or trust chain validation", {}},
	{ CMD_OPT_RSA, "rsa", required_argument, "path",
	  "RSA private key to use for authentication", {}},
	{ CMD_OPT_PKCS12, "p12", required_argument, "path",
	  "PKCS#12 file with private key and certificates to use for ", {
		"authentication and trust chain validation"
	}},
	{ CMD_OPT_AGENT, "agent", optional_argument, "socket",
	  "use SSH agent for authentication. If socket is not specified", {
		"it is read from the SSH_AUTH_SOCK environment variable",
	}},
	{ CMD_OPT_LOCAL_TS, "local-ts", required_argument, "subnet",
	  "additional traffic selector to propose for our side", {}},
	{ CMD_OPT_REMOTE_TS, "remote-ts", required_argument, "subnet",
	  "traffic selector to propose for remote side", {}},
	{ CMD_OPT_IKE_PROPOSAL, "ike-proposal", required_argument, "proposal",
	  "a single IKE proposal to offer instead of the default", {}},
	{ CMD_OPT_ESP_PROPOSAL, "esp-proposal", required_argument, "proposal",
	  "a single ESP proposal to offer instead of the default", {}},
	{ CMD_OPT_AH_PROPOSAL, "ah-proposal", required_argument, "proposal",
	  "a single AH proposal to offer instead of the default", {}},
	{ CMD_OPT_PROFILE, "profile", required_argument, "name",
	  "authentication profile to use, where name is one of:", {
		"  ikev2-pub, ikev2-eap, ikev2-pub-eap",
		"  ikev1-pub[-am], ikev1-xauth[-am],",
		"  ikev1-xauth-psk[-am], ikev1-hybrid[-am]",
	}},
};
