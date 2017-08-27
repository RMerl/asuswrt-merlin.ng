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
 * @file clients.c
 * @brief LDAP module dynamic clients.
 *
 * @author Arran Cudbard-Bell <a.cudbardb@freeradius.org>
 * @copyright 2013 Arran Cudbard-Bell <a.cudbardb@freeradius.org>
 * @copyright 2013 The FreeRADIUS Server Project.
 */
#include	<freeradius-devel/rad_assert.h>
#include	<ctype.h>

#include	"ldap.h"

 /** Load clients from LDAP on server start
  *
  * @param[in] inst rlm_ldap configuration.
  * @return -1 on error else 0.
  */
int rlm_ldap_load_clients(ldap_instance_t const *inst)
{
	int 			ret = 0;
	ldap_rcode_t		status;
	ldap_handle_t		*conn = NULL;

	/* This needs to be updated if additional attributes need to be retrieved */
	char const		*attrs[7];
	char const		**attrs_p;

	LDAPMessage		*result = NULL;
	LDAPMessage		*entry;

	RADCLIENT		*c;

	LDAP_DBG("Loading dynamic clients");

	/*
	 *	Basic sanity checks.
	 */
	if (!inst->clientobj_identifier) {
		LDAP_ERR("Told to load clients but 'client.identifier_attribute' not specified");

		return -1;
	}

	if (!inst->clientobj_secret) {
		LDAP_ERR("Told to load clients but 'client.secret_attribute' not specified");

		return -1;
	}

	if (!inst->clientobj_base_dn) {
		LDAP_ERR("Told to load clients but 'client.base_dn' not specified");

		return -1;
	}

	if (!inst->clientobj_filter) {
		LDAP_ERR("Told to load clients but 'client.filter' not specified");

		return -1;
	}

	/*
	 *	Construct the attribute array
	 */
	attrs[0] = inst->clientobj_identifier;
	attrs[1] = inst->clientobj_secret;
	attrs_p  = attrs + 2;

	if (inst->clientobj_shortname) { /* 2 */
		*attrs_p++ = inst->clientobj_shortname;
	}

	if (inst->clientobj_type) { /* 3 */
		*attrs_p++ = inst->clientobj_type;
	}

	if (inst->clientobj_server) { /* 4 */
		*attrs_p++ = inst->clientobj_server;
	}

	if (inst->clientobj_require_ma) { /* 5 */
		*attrs_p++ = inst->clientobj_require_ma;
	}

	*attrs_p = NULL;	/* 6 - array needs to be NULL terminated */

	conn = rlm_ldap_get_socket(inst, NULL);
	if (!conn) return -1;

	/*
	 *	Perform all searches as the admin user.
	 */
	if (conn->rebound) {
		status = rlm_ldap_bind(inst, NULL, &conn, inst->admin_dn, inst->password, true);
		if (status != LDAP_PROC_SUCCESS) {
			return -1;
		}

		rad_assert(conn);

		conn->rebound = false;
	}

	status = rlm_ldap_search(inst, NULL, &conn, inst->clientobj_base_dn, inst->clientobj_scope,
				 inst->clientobj_filter, attrs, &result);
	switch (status) {
		case LDAP_PROC_SUCCESS:
			break;
		case LDAP_PROC_NO_RESULT:
			LDAP_INFO("No clients were found in the directory");
			return 0;
		default:
			return -1;
	}

	rad_assert(conn);

	entry = ldap_first_entry(conn->handle, result);
	if (!entry) {
		int ldap_errno;

		ldap_get_option(conn->handle, LDAP_OPT_RESULT_CODE, &ldap_errno);
		LDAP_ERR("Failed retrieving entry: %s", ldap_err2string(ldap_errno));

		ret = -1;
		goto finish;
	}

	do {
		char *dn;
		char **identifier	= NULL;
		char **shortname 	= NULL;
		char **secret		= NULL;
		char **type		= NULL;
		char **server		= NULL;
		char **require_ma	= NULL;

		dn = ldap_get_dn(conn->handle, entry);

		/*
		 *	Check for the required attributes first
		 */
		identifier = ldap_get_values(conn->handle, entry, inst->clientobj_identifier);
		if (!identifier) {
			LDAP_WARN("Client \"%s\" missing required attribute 'identifier', skipping...", dn);
			goto next;
		}

		secret = ldap_get_values(conn->handle, entry, inst->clientobj_secret);
		if (!secret) {
			LDAP_WARN("Client \"%s\" missing required attribute 'secret', skipping...", dn);
			goto next;
		}

		if (inst->clientobj_shortname) {
			shortname = ldap_get_values(conn->handle, entry, inst->clientobj_shortname);
			if (!shortname) {
				LDAP_DBG("Client \"%s\" missing optional attribute 'shortname'", dn);
			}
		}

		if (inst->clientobj_type) {
			type = ldap_get_values(conn->handle, entry, inst->clientobj_type);
			if (!type) {
				LDAP_DBG("Client \"%s\" missing optional attribute 'type'", dn);
			}
		}

		if (inst->clientobj_server) {
			server = ldap_get_values(conn->handle, entry, inst->clientobj_server);
			if (!server) {
				LDAP_DBG("Client \"%s\" missing optional attribute 'server'", dn);
			}
		}

		if (inst->clientobj_require_ma) {
			require_ma = ldap_get_values(conn->handle, entry, inst->clientobj_require_ma);
			if (!require_ma) {
				LDAP_DBG("Client \"%s\" missing optional attribute 'require_ma'", dn);
			}
		}

		/* FIXME: We should really pass a proper ctx */
		c = client_from_query(NULL,
				      identifier[0],
				      secret[0],
				      shortname ? shortname[0] : NULL,
				      type ? type[0] : NULL,
				      server ? server[0] : NULL,
				      require_ma ? strncmp(require_ma[0], "true", 4) == 0 : false);
		if (!c) {
			goto next;
		}

		if (!client_add(NULL, c)) {
			WARN("Failed to add client, possible duplicate?");

			client_free(c);
			goto next;
		}

		LDAP_DBG("Client \"%s\" added", dn);

		next:
		ldap_memfree(dn);
		if (identifier)	ldap_value_free(identifier);
		if (shortname)	ldap_value_free(shortname);
		if (secret)	ldap_value_free(secret);
		if (type)	ldap_value_free(type);
		if (server)	ldap_value_free(server);
	} while((entry = ldap_next_entry(conn->handle, entry)));

	finish:
	if (result) {
		ldap_msgfree(result);
	}

	return ret;
}

