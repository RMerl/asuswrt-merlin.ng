/*
 *   This program is is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License, version 2 if the
 *   License as published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/**
 * $Id$
 * @file rlm_dynamic_clients.c
 * @brief Reads client definitions from flat files as required.
 *
 * @copyright 2008  The FreeRADIUS server project
 * @copyright 2008  Alan DeKok <aland@deployingradius.com>
 */
RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>

#ifdef WITH_DYNAMIC_CLIENTS
/*
 *	Find the client definition.
 */
static rlm_rcode_t mod_authorize(UNUSED void *instance,
				 REQUEST *request)
{
	size_t length;
	char const *value;
	CONF_PAIR *cp;
	RADCLIENT *c;
	char buffer[2048];

	/*
	 *	Ensure we're only being called from the main thread,
	 *	with fake packets.
	 */
	if ((request->packet->src_port != 0) || (request->packet->vps != NULL) ||
	    (request->parent != NULL)) {
		RDEBUG("Improper configuration");
		return RLM_MODULE_NOOP;
	}

	if (!request->client || !request->client->cs) {
		RDEBUG("Unknown client definition");
		return RLM_MODULE_NOOP;
	}

	cp = cf_pair_find(request->client->cs, "directory");
	if (!cp) {
		RDEBUG("No directory configuration in the client");
		return RLM_MODULE_NOOP;
	}

	value = cf_pair_value(cp);
	if (!value) {
		RDEBUG("No value given for the directory entry in the client.");
		return RLM_MODULE_NOOP;
	}

	length = strlen(value);
	if (length > (sizeof(buffer) - 256)) {
		RDEBUG("Directory name too long");
		return RLM_MODULE_NOOP;
	}

	memcpy(buffer, value, length + 1);
	ip_ntoh(&request->packet->src_ipaddr,
		buffer + length, sizeof(buffer) - length - 1);

	/*
	 *	Read the buffer and generate the client.
	 */
	c = client_read(buffer, (request->client->server != NULL), true);
	if (!c) return RLM_MODULE_FAIL;

	/*
	 *	Replace the client.  This is more than a bit of a
	 *	hack.
	 */
	request->client = c;

	return RLM_MODULE_OK;
}
#else
static rlm_rcode_t mod_authorize(UNUSED void *instance, REQUEST *request)
{
	RDEBUG("Dynamic clients are unsupported in this build.");
	return RLM_MODULE_FAIL;
}
#endif

/*
 *	The module name should be the only globally exported symbol.
 *	That is, everything else should be 'static'.
 *
 *	If the module needs to temporarily modify it's instantiation
 *	data, the type should be changed to RLM_TYPE_THREAD_UNSAFE.
 *	The server will then take care of ensuring that the module
 *	is single-threaded.
 */
module_t rlm_dynamic_clients = {
	RLM_MODULE_INIT,
	"dynamic_clients",
	RLM_TYPE_THREAD_SAFE,		/* type */
	0,
	NULL,				/* CONF_PARSER */
	NULL,				/* instantiation */
	NULL,				/* detach */
	{
		NULL,			/* authentication */
		mod_authorize,	/* authorization */
		NULL,			/* preaccounting */
		NULL,			/* accounting */
		NULL,			/* checksimul */
		NULL,			/* pre-proxy */
		NULL,			/* post-proxy */
		NULL			/* post-auth */
	},
};
