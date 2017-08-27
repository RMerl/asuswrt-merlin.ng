/**
 * $Id$
 * @file validate.c
 * @brief Authentication for yubikey OTP tokens using the ykclient library.
 *
 * @author Arran Cudbard-Bell <a.cudbardb@networkradius.com>
 * @copyright 2013 The FreeRADIUS server project
 * @copyright 2013 Network RADIUS <info@networkradius.com>
 */
#include "rlm_yubikey.h"

#ifdef HAVE_YKCLIENT
#include <freeradius-devel/connection.h>

/** Creates a new connection handle for use by the FR connection API.
 *
 * Matches the fr_connection_create_t function prototype, is passed to
 * fr_connection_pool_init, and called when a new connection is required by the
 * connection pool API.
 *
 * @see mod_conn_delete
 * @see fr_connection_pool_init
 * @see fr_connection_create_t
 * @see connection.c
 *
 * @param[in] instance configuration data.
 * @return connection handle or NULL if the connection failed or couldn't
 *	be initialised.
 */
static void *mod_socket_create(void *instance)
{
	rlm_yubikey_t *inst = instance;
	ykclient_rc status;
	ykclient_handle_t *yandle;

	status = ykclient_handle_init(inst->ykc, &yandle);
	if (status != YKCLIENT_OK) {
		EDEBUG("rlm_yubikey (%s): %s", inst->name, ykclient_strerror(status));

		return NULL;
	}

	return yandle;
}

/** Frees a ykclient handle
 *
 * @param[in] instance configuration data.
 * @param[in] handle rlm_yubikey_handle_t to close and free.
 * @return returns true.
 */
static int mod_socket_delete(UNUSED void *instance, void *handle)
{
	ykclient_handle_t *yandle = handle;

	ykclient_handle_done(&yandle);

	return true;
}

int rlm_yubikey_ykclient_init(CONF_SECTION *conf, rlm_yubikey_t *inst)
{
	ykclient_rc status;
	CONF_SECTION *servers;

	char prefix[100];

	int count = 0;

	if (!inst->client_id) {
		EDEBUG("rlm_yubikey (%s): client_id must be set when validation is enabled", inst->name);

		return -1;
	}

	if (!inst->api_key) {
		EDEBUG("rlm_yubikey (%s): api_key must be set when validation is enabled", inst->name);

		return -1;
	}

	DEBUG("rlm_yubikey (%s): Initialising ykclient", inst->name);

	status = ykclient_global_init();
	if (status != YKCLIENT_OK) {
yk_error:
		EDEBUG("rlm_yubikey (%s): %s", ykclient_strerror(status), inst->name);

		return -1;
	}

	status = ykclient_init(&inst->ykc);
	if (status != YKCLIENT_OK) {
		goto yk_error;
	}

	servers = cf_section_sub_find(conf, "servers");
	if (servers) {
		CONF_PAIR *uri, *first;
		/*
		 *	If there were no uris configured we just use the default
		 *	ykclient uris which point to the yubico servers.
		 */
		first = uri = cf_pair_find(servers, "uri");
		if (!uri) {
			goto init;
		}

		while (uri) {
			count++;
			uri = cf_pair_find_next(servers, uri, "uri");
		}
		inst->uris = talloc_zero_array(inst, char const *, count);

		uri = first;
		count = 0;
		while (uri) {
			inst->uris[count++] = cf_pair_value(uri);
			uri = cf_pair_find_next(servers, uri, "uri");
		}
		if (count) {
			status = ykclient_set_url_templates(inst->ykc, count, inst->uris);
			if (status != YKCLIENT_OK) {
				goto yk_error;
			}
		}
	}

init:
	status = ykclient_set_client_b64(inst->ykc, inst->client_id, inst->api_key);
	if (status != YKCLIENT_OK) {
		EDEBUG("rlm_yubikey (%s): Failed setting API credentials: %s", ykclient_strerror(status), inst->name);

		return -1;
	}

	snprintf(prefix, sizeof(prefix), "rlm_yubikey (%s)", inst->name);
	inst->conn_pool = fr_connection_pool_init(conf, inst, mod_socket_create, NULL, mod_socket_delete, prefix);
	if (!inst->conn_pool) {
		ykclient_done(&inst->ykc);

		return -1;
	}

	return 0;
}

int rlm_yubikey_ykclient_detach(rlm_yubikey_t *inst)
{
	fr_connection_pool_delete(inst->conn_pool);
	ykclient_done(&inst->ykc);
	ykclient_global_done();

	return 0;
}

rlm_rcode_t rlm_yubikey_validate(rlm_yubikey_t *inst, REQUEST *request,  VALUE_PAIR *otp)
{
	rlm_rcode_t rcode = RLM_MODULE_OK;
	ykclient_rc status;
	ykclient_handle_t *yandle;

	yandle = fr_connection_get(inst->conn_pool);
	if (!yandle) {
		return RLM_MODULE_FAIL;
	}

	/*
	 *	The libcurl multi-handle interface will tear down the TCP sockets for any partially completed
	 *	requests when their easy handle is removed from the multistack.
	 *
	 *	For performance reasons ykclient will stop processing the request immediately after receiving
	 *	a response from one of the servers. If we then immediately call ykclient_handle_cleanup
	 *	the connections are destroyed and will need to be re-established the next time the handle
	 *	is used.
	 *
	 *	To try and prevent this from happening, we leave cleanup until the *next* time
	 *	the handle is used, by which time the requests will of hopefully completed and the connections
	 *	can be re-used.
	 *
	 */
	ykclient_handle_cleanup(yandle);

	status = ykclient_request_process(inst->ykc, yandle, otp->vp_strvalue);
	if (status != YKCLIENT_OK) {
		REDEBUG("%s", ykclient_strerror(status));

		switch (status) {
			case YKCLIENT_BAD_OTP:
			case YKCLIENT_REPLAYED_OTP:
				rcode = RLM_MODULE_REJECT;
				break;

			case YKCLIENT_NO_SUCH_CLIENT:
				rcode = RLM_MODULE_NOTFOUND;
				break;

			default:
				rcode = RLM_MODULE_FAIL;
		}
	}

	fr_connection_release(inst->conn_pool, yandle);

	return rcode;
}
#endif
