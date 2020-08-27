/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Copyright 2020, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */
#include "rc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <networkmap.h>
#include <json.h>

struct json_object *nmp_vc_json;

extern void toUpperCase(char *str);

int update = 0;

void
add_entry(char *tmac, struct json_object *clients)
{
	char mac_buf[32];
	struct json_object *client = NULL, *t_client = NULL;

	memset(mac_buf, 0, sizeof(mac_buf));
	strlcpy(mac_buf, tmac, sizeof(mac_buf));
	toUpperCase(mac_buf);

	NMP_DEBUG_VC("write_to_memory: %s\n", mac_buf);

	json_object_object_get_ex(clients, mac_buf, &client);

	if(client) {
		json_object_object_get_ex(client, "vendorclass", &t_client);
		if(!json_object_get_string(t_client)) {
			json_object_object_add(client, "vendorclass", json_object_new_string(safe_getenv("DNSMASQ_VENDOR_CLASS")));
			update = 1;
		}
	}
	else {
		/* json vendor class list database */
		client = json_object_new_object();
		json_object_object_add(client, "mac", json_object_new_string(mac_buf));
		json_object_object_add(client, "vendorclass", json_object_new_string(safe_getenv("DNSMASQ_VENDOR_CLASS")));
		json_object_object_add(clients, mac_buf, client);
		update = 1;
	}
}

int
nmp_get_vendorclass(int argc, char **argv)
{
	int lock;

	if (argc < 4)
		return -1;

	if (!(nmp_vc_json = json_object_from_file(NMP_VC_JSON_FILE))) {
		NMP_DEBUG_VC("open vendor class list json database ERR:\n");
		nmp_vc_json = json_object_new_object();
	}

	//_dprintf("%s():: %s, %s, %s, %s\n", __FUNCTION__, argv[1], argv[2], argv[3], argv[4] ? : "No hostname");

	if (!strcmp(argv[1], "add") || !strcmp(argv[1], "old")) {
		_dprintf("vendor class=%s\n", safe_getenv("DNSMASQ_VENDOR_CLASS"));
		add_entry(argv[2], nmp_vc_json);
		if(update) {
			lock = file_lock(NMP_VC_FILE_LOCK);
			json_object_to_file(NMP_VC_JSON_FILE, nmp_vc_json); 
			file_unlock(lock);
			NMP_DEBUG_VC("%s():: done\n", __FUNCTION__);
		}		
	}

	json_object_put(nmp_vc_json);

	return 0;
}
