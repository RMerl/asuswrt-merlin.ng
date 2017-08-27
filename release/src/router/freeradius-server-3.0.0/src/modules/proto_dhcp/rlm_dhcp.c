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
 * @file rlm_dhcp.c
 * @brief Will contain dhcp listener code.
 *
 * @copyright 2012  The FreeRADIUS server project
 */
RCSID("$Id$")

#include <freeradius-devel/libradius.h>

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>
#include <freeradius-devel/dhcp.h>

#include <ctype.h>

/*
 *	Define a structure for our module configuration.
 *
 *	These variables do not need to be in a structure, but it's
 *	a lot cleaner to do so, and a pointer to the structure can
 *	be used as the instance handle.
 */
typedef struct rlm_dhcp_t {
	int nothing;
} rlm_dhcp_t;


/*
 *	Allow single attribute values to be retrieved from the dhcp.
 */
static ssize_t dhcp_options_xlat(UNUSED void *instance, REQUEST *request,
			 	 char const *fmt, char *out, size_t freespace)
{
	vp_cursor_t cursor;
	VALUE_PAIR *vp, *head = NULL;
	int decoded = 0;

	while (isspace((int) *fmt)) fmt++;


	if ((radius_get_vp(request, fmt, &vp) < 0) || !vp) {
		 *out = '\0';
		 return 0;
	}

	if ((fr_dhcp_decode_options(request->packet,
				    vp->vp_octets, vp->length, &head) < 0) || (!head)) {
		RWDEBUG("DHCP option decoding failed");
		*out = '\0';
		return -1;
	}


	for (vp = paircursor(&cursor, &head);
	     vp;
	     vp = pairnext(&cursor)) {
		decoded++;
	}

	pairmove(request->packet, &(request->packet->vps), &head);

	/* Free any unmoved pairs */
	pairfree(&head);

	snprintf(out, freespace, "%i", decoded);

	return strlen(out);
}


/*
 *	Only free memory we allocated.  The strings allocated via
 *	cf_section_parse() do not need to be freed.
 */
static int mod_detach(void *instance)
{
	xlat_unregister("dhcp_options", dhcp_options_xlat, instance);
	return 0;
}


/*
 *	Instantiate the module.
 */
static int mod_instantiate(UNUSED CONF_SECTION *conf, void *instance)
{
	rlm_dhcp_t *inst = instance;

	xlat_register("dhcp_options", dhcp_options_xlat, NULL, inst);

	return 0;
}


/*
 *	The module name should be the only globally exported symbol.
 *	That is, everything else should be 'static'.
 *
 *	If the module needs to temporarily modify it's instantiation
 *	data, the type should be changed to RLM_TYPE_THREAD_UNSAFE.
 *	The server will then take care of ensuring that the module
 *	is single-threaded.
 */
module_t rlm_dhcp = {
	RLM_MODULE_INIT,
	"dhcp",
	0,				/* type */
	sizeof(rlm_dhcp_t),
	NULL,				/* CONF_PARSER */
	mod_instantiate,		/* instantiation */
	mod_detach,			/* detach */
	{
		NULL,			/* authentication */
		NULL,			/* authorization */
		NULL,			/* preaccounting */
		NULL,			/* accounting */
		NULL,			/* checksimul */
		NULL,			/* pre-proxy */
		NULL,		 	/* post-proxy */
		NULL,			/* post-auth */
	},
};
