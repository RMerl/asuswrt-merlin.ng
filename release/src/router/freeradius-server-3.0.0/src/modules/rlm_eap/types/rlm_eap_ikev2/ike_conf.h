/*
 *  ike_conf.h - module config loading functions
 *
 *  This file is part of rlm_eap_ikev2 freeRADIUS module which implements
 *  EAP-IKEv2 protocol functionality.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Copyright (C) 2005-2006 Krzysztof Rzecki <krzysztof.rzecki@ccns.pl>
 *  Copyright (C) 2005-2006 Rafal Mijal <rafal.mijal@ccns.pl>
 *  Copyright (C) 2005-2006 Piotr Marnik <piotr.marnik@ccns.pl>
 *  Copyright (C) 2005-2006 Pawel Matejski <pawel.matejski@ccns.pl>
 *
 */

#ifndef IKE_CONF_H
#define IKE_CONF_H

#include <EAPIKEv2/connector.h>
#include "eap.h"

#define RAD_EAP_IKEV2_IDTYPE		1103
#define RAD_EAP_IKEV2_ID		1104
#define RAD_EAP_IKEV2_SECRET		1105
#define RAD_EAP_IKEV2_AUTHTYPE 		1106


int rad_load_proposals(ikev2_ctx *i2,CONF_SECTION *cf);
int rad_load_credentials(TALLOC_CTX *ctx, ikev2_ctx *i2,char *filename,char *authtype_name);
int getusersfile(TALLOC_CTX *ctx, char const *filename, PAIR_LIST **pair_list,char const *compat_mode_str);
void rad_update_shared_seclist(struct sharedSecList **list, char const *id, VALUE_PAIR *items,
			       int default_client_authtype);
int rad_get_authtype(char *authtype_name);
int rad_get_client_authtype(char const *authtype);
int file_exists(char *filename);
#endif //IKE_CONF_H
