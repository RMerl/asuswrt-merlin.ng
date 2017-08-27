/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
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

/*
 * $Id$
 *
 * @brief Functions and datatypes for the REST (HTTP) transport.
 * @file rest.c
 *
 * @copyright 2012-2013  Arran Cudbard-Bell <a.cudbard-bell@freeradius.org>
 */

RCSID("$Id$")

#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include <freeradius-devel/rad_assert.h>
#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/libradius.h>
#include <freeradius-devel/connection.h>

#include "rest.h"

/** Table of encoder/decoder support.
 *
 * Indexes in this table match the http_body_type_t enum, and should be
 * updated if additional enum values are added.
 *
 * @see http_body_type_t
 */
const http_body_type_t http_body_type_supported[HTTP_BODY_NUM_ENTRIES] = {
	HTTP_BODY_UNSUPPORTED,	// HTTP_BODY_UNKOWN
	HTTP_BODY_UNSUPPORTED,	// HTTP_BODY_UNSUPPORTED
	HTTP_BODY_UNSUPPORTED,  // HTTP_BODY_UNAVAILABLE
	HTTP_BODY_UNSUPPORTED,	// HTTP_BODY_INVALID
	HTTP_BODY_POST,		// HTTP_BODY_POST
#ifdef HAVE_JSON
	HTTP_BODY_JSON,		// HTTP_BODY_JSON
#else
	HTTP_BODY_UNAVAILABLE,
#endif
	HTTP_BODY_UNSUPPORTED,	// HTTP_BODY_XML
	HTTP_BODY_UNSUPPORTED,	// HTTP_BODY_YAML
	HTTP_BODY_INVALID,	// HTTP_BODY_HTML
	HTTP_BODY_INVALID	// HTTP_BODY_PLAIN
};

/*
 *	Lib CURL doesn't define symbols for unsupported auth methods
 */
#ifndef CURLOPT_TLSAUTH_SRP
#define CURLOPT_TLSAUTH_SRP 	0
#endif
#ifndef CURLAUTH_BASIC
#define CURLAUTH_BASIC 		0
#endif
#ifndef CURLAUTH_DIGEST
#define CURLAUTH_DIGEST 	0
#endif
#ifndef CURLAUTH_DIGEST_IE
#define CURLAUTH_DIGEST_IE 	0
#endif
#ifndef CURLAUTH_GSSNEGOTIATE
#define CURLAUTH_GSSNEGOTIATE	0
#endif
#ifndef CURLAUTH_NTLM
#define CURLAUTH_NTLM		0
#endif
#ifndef CURLAUTH_NTLM_WB
#define CURLAUTH_NTLM_WB	0
#endif

const unsigned long http_curl_auth[HTTP_AUTH_NUM_ENTRIES] = {
	0,			// HTTP_AUTH_UNKNOWN
	0,			// HTTP_AUTH_NONE
	CURLOPT_TLSAUTH_SRP,	// HTTP_AUTH_TLS_SRP
	CURLAUTH_BASIC,		// HTTP_AUTH_BASIC
	CURLAUTH_DIGEST,	// HTTP_AUTH_DIGEST
	CURLAUTH_DIGEST_IE,	// HTTP_AUTH_DIGEST_IE
	CURLAUTH_GSSNEGOTIATE,	// HTTP_AUTH_GSSNEGOTIATE
	CURLAUTH_NTLM,		// HTTP_AUTH_NTLM
	CURLAUTH_NTLM_WB,	// HTTP_AUTH_NTLM_WB
	CURLAUTH_ANY,		// HTTP_AUTH_ANY
	CURLAUTH_ANYSAFE	// HTTP_AUTH_ANY_SAFE
};


/** Conversion table for method config values.
 *
 * HTTP verb strings for http_method_t enum values. Used by libcurl in the
 * status line of the outgoing HTTP header, by rest_write_header for decoding
 * incoming HTTP responses, and by the configuration parser.
 *
 * @see http_method_t
 * @see fr_str2int
 * @see fr_int2str
 */
const FR_NAME_NUMBER http_method_table[] = {
	{ "GET",				HTTP_METHOD_GET		},
	{ "POST",				HTTP_METHOD_POST	},
	{ "PUT",				HTTP_METHOD_PUT		},
	{ "DELETE",				HTTP_METHOD_DELETE	},

	{  NULL , -1 }
};

/** Conversion table for type config values.
 *
 * Textual names for http_body_type_t enum values, used by the
 * configuration parser.
 *
 * @see http_body_Type_t
 * @see fr_str2int
 * @see fr_int2str
 */
const FR_NAME_NUMBER http_body_type_table[] = {
	{ "unknown",				HTTP_BODY_UNKNOWN	},
	{ "unsupported",			HTTP_BODY_UNSUPPORTED	},
	{ "unavailable",			HTTP_BODY_UNAVAILABLE	},
	{ "invalid",				HTTP_BODY_INVALID	},
	{ "post",				HTTP_BODY_POST		},
	{ "json",				HTTP_BODY_JSON		},
	{ "xml",				HTTP_BODY_XML		},
	{ "yaml",				HTTP_BODY_YAML		},
	{ "html",				HTTP_BODY_HTML		},
	{ "plain",				HTTP_BODY_PLAIN		},

	{  NULL , -1 }
};

const FR_NAME_NUMBER http_auth_table[] = {
	{ "none",				HTTP_AUTH_NONE		},
	{ "srp",				HTTP_AUTH_TLS_SRP	},
	{ "basic",				HTTP_AUTH_BASIC		},
	{ "digest",				HTTP_AUTH_DIGEST	},
	{ "digest-ie",				HTTP_AUTH_DIGEST_IE	},
	{ "gss-negotiate",			HTTP_AUTH_GSSNEGOTIATE	},
	{ "ntlm",				HTTP_AUTH_NTLM		},
	{ "ntlm-winbind",			HTTP_AUTH_NTLM_WB	},
	{ "any",				HTTP_AUTH_ANY		},
	{ "safe",				HTTP_AUTH_ANY_SAFE	},

	{  NULL , -1 }
};

/** Conversion table for "Content-Type" header values.
 *
 * Used by rest_write_header for parsing incoming headers.
 *
 * Values we expect to see in the 'Content-Type:' header of the incoming
 * response.
 *
 * Some data types (like YAML) do no have standard MIME types defined,
 * so multiple types, are listed here.
 *
 * @see http_body_Type_t
 * @see fr_str2int
 * @see fr_int2str
 */
const FR_NAME_NUMBER http_content_type_table[] = {
	{ "application/x-www-form-urlencoded",	HTTP_BODY_POST		},
	{ "application/json",			HTTP_BODY_JSON		},
	{ "text/html",				HTTP_BODY_HTML		},
	{ "text/plain",				HTTP_BODY_PLAIN		},
	{ "text/xml",				HTTP_BODY_XML		},
	{ "text/yaml",				HTTP_BODY_YAML		},
	{ "text/x-yaml",			HTTP_BODY_YAML		},
	{ "application/yaml",			HTTP_BODY_YAML		},
	{ "application/x-yaml",			HTTP_BODY_YAML		},
	{  NULL , -1 }
};

#ifdef HAVE_JSON
/** Flags to control the conversion of JSON values to VALUE_PAIRs.
 *
 * These fields are set when parsing the expanded format for value pairs in
 * JSON, and control how json_pairmake_leaf and json_pairmake convert the JSON
 * value, and move the new VALUE_PAIR into an attribute list.
 *
 * @see json_pairmake
 * @see json_pairmake_leaf
 */
typedef struct json_flags {
	int do_xlat;		//!< If true value will be expanded with xlat.
	int is_json;		//!< If true value will be inserted as raw JSON
				// (multiple values not supported).
	FR_TOKEN op;		//!< The operator that determines how the new VP
				// is processed. @see fr_tokens
} json_flags_t;
#endif

/** Initialises libcurl.
 *
 * Allocates global variables and memory required for libcurl to fundtion.
 * MUST only be called once per module instance.
 *
 * rest_cleanup must not be called if rest_init fails.
 *
 * @see rest_cleanup
 *
 * @param[in] instance configuration data.
 * @return 0 if init succeeded -1 if it failed.
 */
int rest_init(rlm_rest_t *instance)
{
	CURLcode ret;

	ret = curl_global_init(CURL_GLOBAL_ALL);
	if (ret != CURLE_OK) {
		ERROR("rlm_rest (%s): CURL init returned error: %i - %s",
		       instance->xlat_name,
		       ret, curl_easy_strerror(ret));

		curl_global_cleanup();
		return -1;
	}

	DEBUG("rlm_rest (%s): CURL library version: %s", instance->xlat_name, curl_version());

	return 0;
}

/** Cleans up after libcurl.
 *
 * Wrapper around curl_global_cleanup, frees any memory allocated by rest_init.
 * Must only be called once per call of rest_init.
 *
 * @see rest_init
 */
void rest_cleanup(void)
{
	curl_global_cleanup();
}

/** Creates a new connection handle for use by the FR connection API.
 *
 * Matches the fr_connection_create_t function prototype, is passed to
 * fr_connection_pool_init, and called when a new connection is required by the
 * connection pool API.
 *
 * Creates an instances of rlm_rest_handle_t, and rlm_rest_curl_context_t
 * which hold the context data required for generating requests and parsing
 * responses. Calling mod_conn_delete will free this memory.
 *
 * If instance->connect_uri is not NULL libcurl will attempt to open a
 * TCP socket to the server specified in the URI. This is done so that when the
 * socket is first used, there will already be a cached TCP connection to the
 * REST server associated with the curl handle.
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
void *mod_conn_create(void *instance)
{
	rlm_rest_t *inst = instance;

	rlm_rest_handle_t	*randle = NULL;
	rlm_rest_curl_context_t	*ctx = NULL;

	CURL *candle = curl_easy_init();
	CURLcode ret;

	if (!candle) {
		ERROR("rlm_rest (%s): Failed to create CURL handle", inst->xlat_name);
		return NULL;
	}

	if (!*inst->connect_uri) {
		ERROR("rlm_rest (%s): Skipping pre-connect, connect_uri not specified", inst->xlat_name);
		return candle;
	}

	/*
	 *	Pre-establish TCP connection to webserver. This would usually be
	 *	done on the first request, but we do it here to minimise
	 *	latency.
	 */
	ret = curl_easy_setopt(candle, CURLOPT_CONNECT_ONLY, 1);
	if (ret != CURLE_OK) goto error;

	ret = curl_easy_setopt(candle, CURLOPT_URL,
			       inst->connect_uri);
	if (ret != CURLE_OK) goto error;

	DEBUG("rlm_rest (%s): Connecting to \"%s\"", inst->xlat_name, inst->connect_uri);

	ret = curl_easy_perform(candle);
	if (ret != CURLE_OK) {
		ERROR("rlm_rest (%s): Connection failed: %i - %s", inst->xlat_name, ret, curl_easy_strerror(ret));

		goto connection_error;
	}

	/*
	 *	Allocate memory for the connection handle abstraction.
	 */
	randle = talloc_zero(inst, rlm_rest_handle_t);
	ctx = talloc_zero(randle, rlm_rest_curl_context_t);

	ctx->headers = NULL; /* CURL needs this to be NULL */
	ctx->read.instance = inst;

	randle->ctx = ctx;
	randle->handle = candle;

	/*
	 *	Clear any previously configured options for the first request.
	 */
	curl_easy_reset(candle);

	return randle;

	/*
	 *	Cleanup for error conditions.
	 */
	error:

	ERROR("rlm_rest (%s): Failed setting curl option: %i - %s", inst->xlat_name, ret, curl_easy_strerror(ret));

	/*
	 *	So we don't leak CURL handles.
	 */
	connection_error:

	curl_easy_cleanup(candle);
	if (randle) talloc_free(randle);

	return NULL;
}

/** Verifies that the last TCP socket associated with a handle is still active.
 *
 * Quieries libcurl to try and determine if the TCP socket associated with a
 * connection handle is still viable.
 *
 * @param[in] instance configuration data.
 * @param[in] handle to check.
 * @returns false if the last socket is dead, or if the socket state couldn't be
 *	determined, else true.
 */
int mod_conn_alive(void *instance, void *handle)
{
	rlm_rest_t *inst 		= instance;
	rlm_rest_handle_t *randle	= handle;
	CURL *candle			= randle->handle;

	long last_socket;
	CURLcode ret;

	ret = curl_easy_getinfo(candle, CURLINFO_LASTSOCKET, &last_socket);
	if (ret != CURLE_OK) {
		ERROR("rlm_rest (%s): Couldn't determine socket state: %i - %s", inst->xlat_name, ret,
		      curl_easy_strerror(ret));

		return false;
	}

	if (last_socket == -1) {
		return false;
	}

	return true;
}

/** Frees a libcurl handle, and any additional memory used by context data.
 *
 * @param[in] instance configuration data.
 * @param[in] handle rlm_rest_handle_t to close and free.
 * @return returns true.
 */
int mod_conn_delete(UNUSED void *instance, void *handle)
{
	rlm_rest_handle_t *randle	= handle;
	CURL *candle			= randle->handle;

	curl_easy_cleanup(candle);

	talloc_free(randle);

	return true;
}

/** Encodes VALUE_PAIR linked list in POST format
 *
 * This is a stream function matching the rest_read_t prototype. Multiple
 * successive calls will return additional encoded VALUE_PAIRs.
 * Only complete attribute headers @verbatim '<name>=' @endverbatim and values
 * will be written to the ptr buffer.
 *
 * POST request format is:
 * @verbatim <attribute0>=<value0>&<attribute1>=<value1>&<attributeN>=<valueN>@endverbatim
 *
 * All attributes and values are url encoded. There is currently no support for
 * nested attributes, or attribute qualifiers.
 *
 * Nested attributes may be added in the future using
 * @verbatim <attribute-outer>:<attribute-inner>@endverbatim
 * to denotate nesting.
 *
 * Requires libcurl for url encoding.
 *
 * @see rest_decode_post
 *
 * @param[out] ptr Char buffer to write encoded data to.
 * @param[in] size Multiply by nmemb to get the length of ptr.
 * @param[in] nmemb Multiply by size to get the length of ptr.
 * @param[in] userdata rlm_rest_read_t to keep encoding state between calls.
 * @return length of data (including NULL) written to ptr, or 0 if no more
 *	data to write.
 */
static size_t rest_encode_post(void *ptr, size_t size, size_t nmemb,
			       void *userdata)
{
	rlm_rest_read_t *ctx	= userdata;
	REQUEST *request	= ctx->request; /* Used by RDEBUG */
	VALUE_PAIR *vp;

	char *p = ptr;	/* Position in buffer */
	char *f = ptr;	/* Position in buffer of last fully encoded attribute or value */
	char *escaped;	/* Pointer to current URL escaped data */

	ssize_t len = 0;
	ssize_t s = (size * nmemb) - 1;

	/* Allow manual chunking */
	if ((ctx->chunk) && (ctx->chunk <= s)) {
		s = (ctx->chunk - 1);
	}

	if (ctx->state == READ_STATE_END) return 0;

	/* Post data requires no headers */
	if (ctx->state == READ_STATE_INIT) {
		ctx->state = READ_STATE_ATTR_BEGIN;
	}

	while (s > 0) {
		vp = paircurrent(&ctx->cursor);
		if (!vp) {
			ctx->state = READ_STATE_END;

			goto end_chunk;
		}

		RDEBUG2("Encoding attribute \"%s\"", vp->da->name);

		if (ctx->state == READ_STATE_ATTR_BEGIN) {
			escaped = curl_escape(vp->da->name, strlen(vp->da->name));
			len = strlen(escaped);

			if (s < (1 + len)) {
				curl_free(escaped);
				goto no_space;
			}

			len = sprintf(p, "%s=", escaped);

			curl_free(escaped);

			p += len;
			s -= len;

			/*
			 *	We wrote the attribute header, record progress.
			 */
			f = p;
			ctx->state = READ_STATE_ATTR_CONT;
		}

		/*
		 *	Write out single attribute string.
		 */
		len = vp_prints_value(p , s, vp, 0);
		escaped = curl_escape(p, len);
		len = strlen(escaped);

		if (s < len) {
			curl_free(escaped);
			goto no_space;
		}

		len = strlcpy(p, escaped, len + 1);

		curl_free(escaped);

		RDEBUG("\tLength : %i", len);
		RDEBUG("\tValue  : %s", p);

		p += len;
		s -= len;

		if (!vp) {
			if (!--s) goto no_space;
			*p++ = '&';
		}

		/*
		 *	We wrote one full attribute value pair, record progress.
		 */
		f = p;
		pairnext(&ctx->cursor);
		ctx->state = READ_STATE_ATTR_BEGIN;
	}

	end_chunk:

	*p = '\0';

	len = p - (char*)ptr;

	RDEBUG2("POST Data: %s", (char*) ptr);
	RDEBUG2("Returning %i bytes of POST data", len);

	return len;

	/*
	 *	Cleanup for error conditions
	 */
	no_space:

	*f = '\0';

	len = f - (char*)ptr;

	RDEBUG2("POST Data: %s", (char*) ptr);

	/*
	 *	The buffer wasn't big enough to encode a single attribute chunk.
	 */
	if (!len) {
		REDEBUG("AVP exceeds buffer length or chunk");
	} else {
		RDEBUG2("Returning %i bytes of POST data (buffer full or chunk exceeded)", len);
	}

	return len;
}

/** Encodes VALUE_PAIR linked list in JSON format
 *
 * This is a stream function matching the rest_read_t prototype. Multiple
 * successive calls will return additional encoded VALUE_PAIRs.
 *
 * Only complete attribute headers
 * @verbatim "<name>":{"type":"<type>","value":[' @endverbatim
 * and complete attribute values will be written to ptr.
 *
 * If an attribute occurs multiple times in the request the attribute values
 * will be concatenated into a single value array.
 *
 * JSON request format is:
@verbatim
{
	"<attribute0>":{
		"type":"<type0>",
		"value":[<value0>,<value1>,<valueN>]
	},
	"<attribute1>":{
		"type":"<type1>",
		"value":[...]
	},
	"<attributeN>":{
		"type":"<typeN>",
		"value":[...]
	},
}
@endverbatim
 *
 * @param[out] ptr Char buffer to write encoded data to.
 * @param[in] size Multiply by nmemb to get the length of ptr.
 * @param[in] nmemb Multiply by size to get the length of ptr.
 * @param[in] userdata rlm_rest_read_t to keep encoding state between calls.
 * @return length of data (including NULL) written to ptr, or 0 if no more
 *	data to write.
 */
static size_t rest_encode_json(void *ptr, size_t size, size_t nmemb,
			       void *userdata)
{
	rlm_rest_read_t *ctx	= userdata;
	REQUEST *request	= ctx->request; /* Used by RDEBUG */
	VALUE_PAIR *vp, *next;

	char *p = ptr;	/* Position in buffer */
	char *f = ptr;	/* Position in buffer of last fully encoded attribute or value */

	char const *type;

	ssize_t len = 0;
	ssize_t s = (size * nmemb) - 1;

	assert(s > 0);

	/* Allow manual chunking */
	if ((ctx->chunk) && (ctx->chunk <= s)) {
		s = (ctx->chunk - 1);
	}

	if (ctx->state == READ_STATE_END) return 0;

	if (ctx->state == READ_STATE_INIT) {
		ctx->state = READ_STATE_ATTR_BEGIN;

		if (!--s) goto no_space;
		*p++ = '{';
	}

	while (s > 0) {
		vp = paircurrent(&ctx->cursor);

		/*
		 *	We've encoded all the VPs
		 */
		if (!vp) {
			ctx->state = READ_STATE_END;

			if (!--s) goto no_space;
			*p++ = '}';

			goto end_chunk;
		}

		/*
		 *	New attribute, write name, type, and beginning of
		 *	value array.
		 */
		RDEBUG2("Encoding attribute \"%s\"", vp->da->name);
		if (ctx->state == READ_STATE_ATTR_BEGIN) {
			type = fr_int2str(dict_attr_types, vp->da->type, "¿Unknown?");

			len  = strlen(type);
			len += strlen(vp->da->name);

			if (s < (23 + len)) goto no_space;

			len = sprintf(p, "\"%s\":{\"type\":\"%s\",\"value\":[" , vp->da->name, type);
			p += len;
			s -= len;

			RDEBUG2("\tType   : %s", type);

			/*
		 	 *	We wrote the attribute header, record progress
		 	 */
			f = p;
			ctx->state = READ_STATE_ATTR_CONT;
		}

		for (;;) {
			len = vp_prints_value_json(p, s, vp);
			if (len >= s) goto no_space;

			/*
			 *	Show actual value length minus quotes
			 */
			RDEBUG2("\tLength : %i", (*p == '"') ? (len - 2) : len);
			RDEBUG2("\tValue  : %s", p);

			p += len;
			s -= len;

			/*
			 *	Multivalued attribute, we sorted all the attributes earlier, so multiple
			 *	instances should occur in a contiguous block.
			 */
			if ((next = pairnext(&ctx->cursor)) && (vp->da == next->da)) {
				*p++ = ',';

				/*
				 *	We wrote one attribute value, record
				 *	progress.
				 */
				f = p;
				vp = next;
				continue;
			}
			break;
		}

		if (!(s -= 2)) goto no_space;
		*p++ = ']';
		*p++ = '}';

		if (next) {
			if (!--s) goto no_space;
			*p++ = ',';
		}

		/*
		 *	We wrote one full attribute value pair, record progress.
		 */
		f = p;
		ctx->state = READ_STATE_ATTR_BEGIN;
	}

	end_chunk:

	*p = '\0';

	len = p - (char*)ptr;

	RDEBUG2("JSON Data: %s", (char*) ptr);
	RDEBUG2("Returning %i bytes of JSON data", len);

	return len;

	/*
	 * Were out of buffer space
	 */
	no_space:

	*f = '\0';

	len = f - (char*)ptr;

	RDEBUG2("JSON Data: %s", (char*) ptr);

	/*
	 *	The buffer wasn't big enough to encode a single attribute chunk.
	 */
	if (!len) {
		REDEBUG("AVP exceeds buffer length or chunk");
	} else {
		RDEBUG2("Returning %i bytes of JSON data (buffer full or chunk exceeded)", len);
	}

	return len;
}

/** Emulates successive libcurl calls to an encoding function
 *
 * This function is used when the request will be sent to the HTTP server as one
 * contiguous entity. A buffer of REST_BODY_INCR bytes is allocated and passed
 * to the stream encoding function.
 *
 * If the stream function does not return 0, a new buffer is allocated which is
 * the size of the previous buffer + REST_BODY_INCR bytes, the data from the
 * previous buffer is copied, and freed, and another call is made to the stream
 * function, passing a pointer into the new buffer at the end of the previously
 * written data.
 *
 * This process continues until the stream function signals (by returning 0)
 * that it has no more data to write.
 *
 * @param[out] buffer where the pointer to the alloced buffer should
 *	be written.
 * @param[in] func Stream function.
 * @param[in] limit Maximum buffer size to alloc.
 * @param[in] userdata rlm_rest_read_t to keep encoding state between calls to
 *	stream function.
 * @return the length of the data written to the buffer (excluding NULL) or -1
 *	if alloc >= limit.
 */
static ssize_t rest_read_wrapper(char **buffer, rest_read_t func,
				 size_t limit, void *userdata)
{
	char *previous = NULL;
	char *current;

	size_t alloc = REST_BODY_INCR;	/* Size of buffer to alloc */
	size_t used  = 0;		/* Size of data written */
	size_t len   = 0;

	while (alloc < limit) {
		current = rad_malloc(alloc);

		if (previous) {
			strlcpy(current, previous, used + 1);
			free(previous);
		}

		len = func(current + used, REST_BODY_INCR, 1, userdata);
		used += len;
		if (!len) {
			*buffer = current;
			return used;
		}

		alloc += REST_BODY_INCR;
		previous = current;
	};

	free(current);

	return -1;
}

/** (Re-)Initialises the data in a rlm_rest_read_t.
 *
 * Resets the values of a rlm_rest_read_t to their defaults.
 *
 * @param[in] request Current request.
 * @param[in] ctx to initialise.
 * @param[in] sort If true VALUE_PAIRs will be sorted within the VALUE_PAIR
 *	pointer array.
 */
static void rest_read_ctx_init(REQUEST *request, rlm_rest_read_t *ctx, bool sort)
{
	/*
	 * 	Setup stream read data
	 */
	ctx->request = request;
	ctx->state = READ_STATE_INIT;

	/*
	 *	Sorts pairs in place, oh well...
	 */
	if (sort) {
		pairsort(&request->packet->vps, true);
	}
	paircursor(&ctx->cursor, &request->packet->vps);
}

/** Converts POST response into VALUE_PAIRs and adds them to the request
 *
 * Accepts VALUE_PAIRS in the same format as rest_encode_post, but with the
 * addition of optional attribute list qualifiers as part of the attribute name
 * string.
 *
 * If no qualifiers are specified, will default to the request list.
 *
 * POST response format is:
 * @verbatim [outer.][<list>:]<attribute0>=<value0>&[outer.][<list>:]<attribute1>=<value1>&[outer.][<list>:]<attributeN>=<valueN> @endverbatim
 *
 * @see rest_encode_post
 *
 * @param[in] instance configuration data.
 * @param[in] section configuration data.
 * @param[in] handle rlm_rest_handle_t to use.
 * @param[in] request Current request.
 * @param[in] raw buffer containing POST data.
 * @param[in] rawlen Length of data in raw buffer.
 * @return the number of VALUE_PAIRs processed or -1 on unrecoverable error.
 */
static int rest_decode_post(UNUSED rlm_rest_t *instance,
			    UNUSED rlm_rest_section_t *section,
			    REQUEST *request, void *handle, char *raw,
			    UNUSED size_t rawlen)
{
	rlm_rest_handle_t *randle = handle;
	CURL *candle		  = randle->handle;

	char const *p = raw, *q;

	char const *attribute;
	char *name  = NULL;
	char *value = NULL;

	char *expanded = NULL;

	DICT_ATTR const *da;
	VALUE_PAIR *vp;

	DICT_ATTR const **current, *processed[REST_BODY_MAX_ATTRS + 1];

	pair_lists_t list_name;
	request_refs_t request_name;
	REQUEST *reference = request;
	VALUE_PAIR **vps;

	size_t len;
	int curl_len; /* Length from last curl_easy_unescape call */

	int count = 0;
	int ret;

	processed[0] = NULL;

	/*
	 * Empty response?
	 */
	while (isspace(*p)) p++;
	if (*p == '\0') return 0;

	while (((q = strchr(p, '=')) != NULL) &&
	       (count < REST_BODY_MAX_ATTRS)) {
		attribute = name;
		reference = request;

		name = curl_easy_unescape(candle, p, (q - p), &curl_len);
		p = (q + 1);

		RDEBUG("Decoding attribute \"%s\"", name);

		request_name = radius_request_name(&attribute, REQUEST_CURRENT);
		if (request_name == REQUEST_UNKNOWN) {
			RWDEBUG("Invalid request qualifier, skipping");

			curl_free(name);

			continue;
		}

		if (!radius_request(&reference, request_name)) {
			RWDEBUG("Attribute name refers to outer request"
		       	       " but not in a tunnel, skipping");

			curl_free(name);

			continue;
		}

		list_name = radius_list_name(&attribute, PAIR_LIST_REPLY);
		if (list_name == PAIR_LIST_UNKNOWN) {
			RWDEBUG("Invalid list qualifier, skipping");

			curl_free(name);

			continue;
		}

		da = dict_attrbyname(attribute);
		if (!da) {
			RWDEBUG("Attribute \"%s\" unknown, skipping",
			       attribute);

			curl_free(name);

			continue;
		}

		vps = radius_list(reference, list_name);

		assert(vps);

		RDEBUG2("\tType  : %s", fr_int2str(dict_attr_types, da->type, "¿Unknown?"));

		q = strchr(p, '&');
		len = (!q) ? (rawlen - (p - raw)) : (unsigned)(q - p);

		value = curl_easy_unescape(candle, p, len, &curl_len);

		/*
		 *	If we found a delimiter we want to skip over it,
		 *	if we didn't we do *NOT* want to skip over the end
		 *	of the buffer...
		 */
		p += (!q) ? len : (len + 1);

		RDEBUG2("\tLength : %i", curl_len);
		RDEBUG2("\tValue  : \"%s\"", value);

		RDEBUG("Performing xlat expansion of response value");

		if (radius_axlat(&expanded, request, value, NULL, NULL) < 0) {
			goto skip;
		}

		vp = pairalloc(request, da);
		if (!vp) {
			REDEBUG("Failed creating valuepair");
			talloc_free(expanded);

			goto error;
		}

		vp->op = T_OP_SET;

		/*
		 * 	Check to see if we've already processed an
		 *	attribute of the same type if we have, change the op
		 *	from T_OP_ADD to T_OP_SET.
		 */
		current = processed;
		while (*current++) {
			if (current[0] == da) {
				vp->op = T_OP_ADD;
				break;
			}
		}

		if (vp->op != T_OP_ADD) {
			current[0] = da;
			current[1] = NULL;
		}

		ret = pairparsevalue(vp, expanded);
		talloc_free(expanded);
		if (!ret) {
			RWDEBUG("Incompatible value assignment, skipping");
			talloc_free(vp);
			goto skip;
		}

		if (++count == REST_BODY_MAX_ATTRS) {
			REDEBUG("At maximum attribute limit");
			return count;
		}

		skip:

		curl_free(name);
		curl_free(value);

		continue;

		error:

		curl_free(name);
		curl_free(value);

		return count;
	}

	if (!count) {
		REDEBUG("Malformed POST data \"%s\"", raw);
	}

	return count;

}

#ifdef HAVE_JSON
/** Converts JSON "value" key into VALUE_PAIR.
 *
 * If leaf is not in fact a leaf node, but contains JSON data, the data will
 * written to the attribute in JSON string format.
 *
 * @param[in] instance configuration data.
 * @param[in] section configuration data.
 * @param[in] request Current request.
 * @param[in] da Attribute to create.
 * @param[in] flags containing the operator other flags controlling value
 *	expansion.
 * @param[in] leaf object containing the VALUE_PAIR value.
 * @return The VALUE_PAIR just created, or NULL on error.
 */
static VALUE_PAIR *json_pairmake_leaf(UNUSED rlm_rest_t *instance,
				      UNUSED rlm_rest_section_t *section,
				      REQUEST *request, DICT_ATTR const *da,
				      json_flags_t *flags, json_object *leaf)
{
	char const *value, *to_parse;
	char *expanded = NULL;
	int ret;

	VALUE_PAIR *vp;

	/*
	 *	Should encode any nested JSON structures into JSON strings.
	 *
	 *	"I knew you liked JSON so I put JSON in your JSON!"
	 */
	value = json_object_get_string(leaf);

	RDEBUG2("\tType   : %s", fr_int2str(dict_attr_types, da->type,
					    "¿Unknown?"));
	RDEBUG2("\tLength : %i", strlen(value));
	RDEBUG2("\tValue  : \"%s\"", value);

	if (flags->do_xlat) {
		if (radius_axlat(&expanded, request, value, NULL, NULL) < 0) {
			return NULL;
		}

		to_parse = expanded;
	} else {
		to_parse = value;
	}

	vp = paircreate(NULL, da->attr, da->vendor);
	if (!vp) {
		REDEBUG("Failed creating valuepair");
		talloc_free(expanded);

		return NULL;
	}

	vp->op = flags->op;

	ret = pairparsevalue(vp, to_parse);
	talloc_free(expanded);
	if (!ret) {
		RDEBUG("Incompatible value assignment, skipping");
		talloc_free(vp);

		return NULL;
	}

	return vp;
}

/** Processes JSON response and converts it into multiple VALUE_PAIRs
 *
 * Processes JSON attribute declarations in the format below. Will recurse when
 * processing nested attributes. When processing nested attributes flags and
 * operators from previous attributes are not inherited.
 *
 * JSON response format is:
@verbatim
{
	"<attribute0>":{
		do_xlat:<bool>,
		is_json:<bool>,
		"op":"<operator>",
		"value":[<value0>,<value1>,<valueN>]
	},
	"<attribute1>":{
		"value":{
			"<nested-attribute0>":{
				"op":"<operator>",
				"value":<value0>
			}
		}
	},
	"<attribute2>":"<value0>",
	"<attributeN>":"[<value0>,<value1>,<valueN>]"
}
@endverbatim
 *
 * JSON valuepair flags (bools):
 *  - do_xlat	(optional) Controls xlat expansion of values. Defaults to true.
 *  - is_json	(optional) If true, any nested JSON data will be copied to the
 *			   VALUE_PAIR in string form. Defaults to true.
 *  - op	(optional) Controls how the attribute is inserted into
 *			   the target list. Defaults to ':=' (T_OP_SET).
 *
 * If "op" is ':=' or '=', it will be automagically changed to '+=' for the
 * second and subsequent values in multivalued attributes. This does not work
 * between multiple attribute declarations.
 *
 * @see fr_tokens
 *
 * @param[in] instance configuration data.
 * @param[in] section configuration data.
 * @param[in] request Current request.
 * @param[in] object containing root node, or parent node.
 * @param[in] level Current nesting level.
 * @param[in] max_attrs counter, decremented after each VALUE_PAIR is created,
 * 	      when 0 no more attributes will be processed.
 * @return VALUE_PAIR or NULL on error.
 */
static VALUE_PAIR *json_pairmake(rlm_rest_t *instance,
				 UNUSED rlm_rest_section_t *section,
				 REQUEST *request, json_object *object,
				 UNUSED int level, int *max_attrs)
{
	char const *p;
	char *q;

	char const *name, *attribute;

	struct json_object *value, *idx, *tmp;
	struct lh_entry *entry;
	json_flags_t flags;

	DICT_ATTR const *da;
	VALUE_PAIR *vp = NULL;

	request_refs_t request_name;
	pair_lists_t list_name;
	REQUEST *reference = request;
	VALUE_PAIR **vps;

	int i, len;

	if (!json_object_is_type(object, json_type_object)) {
		RDEBUG("Can't process VP container, expected JSON object, got \"%s\", skipping",
      	       	       json_type_to_name(json_object_get_type(object)));
		return NULL;
   	}

	/*
	 *	Process VP container
	 */
	entry = json_object_get_object(object)->head;
	while (entry) {
		flags.op = T_OP_SET;
		flags.do_xlat  = 1;
		flags.is_json  = 0;

		name = (char*)entry->k;

		/* Fix the compiler warnings regarding const... */
		memcpy(&value, &entry->v, sizeof(value));

		entry = entry->next;

		/*
		 *	For people handcrafting JSON responses
		 */
		p = name;
		while ((p = q = strchr(p, '|'))) {
			*q = ':';
			p++;
		}

		attribute = name;
		reference = request;

		/*
		 *	Resolve attribute name to a dictionary entry and
		 *	pairlist.
		 */
		RDEBUG2("Decoding attribute \"%s\"", name);

		request_name = radius_request_name(&attribute, REQUEST_CURRENT);
		if (request_name == REQUEST_UNKNOWN) {
			RWDEBUG("Request qualifier unknown, skipping");

			continue;
		}

		if (!radius_request(&reference, request_name)) {
			RWDEBUG("Attribute name refers to outer request"
		       	       " but not in a tunnel, skipping");

			continue;
		}

		list_name = radius_list_name(&attribute, PAIR_LIST_REPLY);
		if (list_name == PAIR_LIST_UNKNOWN) {
			RWDEBUG("Invalid list qualifier, skipping");

			continue;
		}

		da = dict_attrbyname(attribute);
		if (!da) {
			RWDEBUG("Attribute \"%s\" unknown, skipping",
			       attribute);

			continue;
		}

		vps = radius_list(reference, list_name);

		assert(vps);

		/*
		 *	Alternate JSON structure that allows operator,
		 *	and other flags to be specified.
		 *
		 *	"<name>":{
		 *		"do_xlat":<bool>,
		 *		"is_json":<bool>,
		 *		"op":"<op>",
		 *		"value":<value>
		 *	}
		 *
		 *	Where value is a:
		 *	  - []	Multivalued array
		 *	  - {}	Nested Valuepair
		 *	  - *	Integer or string value
		 */
		if (json_object_is_type(value, json_type_object)) {
			/*
			 *	Process operator if present.
			 */
			tmp = json_object_object_get(value, "op");
			if (tmp) {
				flags.op = fr_str2int(fr_tokens, json_object_get_string(tmp), 0);
				if (!flags.op) {
					RDEBUG("Invalid operator value \"%s\","
					       " skipping", tmp);
					continue;
				}
			}

			/*
			 *	Process optional do_xlat bool.
			 */
			tmp = json_object_object_get(value, "do_xlat");
			if (tmp) {
				flags.do_xlat = json_object_get_boolean(tmp);
			}

			/*
			 *	Process optional is_json bool.
			 */
			tmp = json_object_object_get(value, "is_json");
			if (tmp) {
				flags.is_json = json_object_get_boolean(tmp);
			}

			/*
			 *	Value key must be present if were using
			 *	the expanded syntax.
			 */
			value = json_object_object_get(value, "value");
			if (!value) {
				RDEBUG("Value key missing, skipping", value);
				continue;
			}
   		}

		/*
		 *	Setup pairmake / recursion loop.
		 */
		if (!flags.is_json &&
		    json_object_is_type(value, json_type_array)) {
			len = json_object_array_length(value);
			if (!len) {
				RDEBUG("Zero length value array, skipping",
				       value);
				continue;
			}
			idx = json_object_array_get_idx(value, 0);
		} else {
			len = 1;
			idx = value;
		}

		i = 0;
		do {
			if (!(*max_attrs)--) {
					REDEBUG("At maximum attribute limit");
					return NULL;
			}

			/*
			 *	Automagically switch the op for multivalued
			 *	attributes.
			 */
			if (((flags.op == T_OP_SET) ||
			     (flags.op == T_OP_EQ)) && (len > 1)) {
				flags.op = T_OP_ADD;
			}

			if (!flags.is_json && json_object_is_type(value, json_type_object)) {
				/* TODO: Insert nested VP into VP structure...*/
				REDEBUG("Found nested VP, these are not yet supported");

				return NULL;

				/*
				vp = json_pairmake(instance, section,
						   request, value,
						   level + 1, max_attrs);*/
			} else {
				vp = json_pairmake_leaf(instance, section,
							request, da, &flags,
							idx);
			}
		} while ((++i < len) &&
			 (idx = json_object_array_get_idx(value, i)));
   }

   return vp;
}

/** Converts JSON response into VALUE_PAIRs and adds them to the request.
 *
 * Converts the raw JSON string into a json-c object tree and passes it to
 * json_pairmake. After the tree has been parsed json_object_put is called
 * which decrements the reference count of the root node by one, and frees
 * the entire tree.
 *
 * @see rest_encode_json
 * @see json_pairmake
 *
 * @param[in] instance configuration data.
 * @param[in] section configuration data.
 * @param[in,out] request Current request.
 * @param[in] handle REST handle.
 * @param[in] raw buffer containing JSON data.
 * @param[in] rawlen Length of data in raw buffer.
 * @return the number of VALUE_PAIRs processed or -1 on unrecoverable error.
 */
static int rest_decode_json(rlm_rest_t *instance,
			    UNUSED rlm_rest_section_t *section,
			    REQUEST *request, UNUSED void *handle,
			    char *raw, UNUSED size_t rawlen)
{
	char const *p = raw;

	struct json_object *json;

	int max = REST_BODY_MAX_ATTRS;

	/*
	 *	Empty response?
	 */
	while (isspace(*p)) p++;
	if (*p == '\0') return 0;

	json = json_tokener_parse(p);
	if (!json) {
		RDEBUG("Malformed JSON data \"%s\"", raw);
		return -1;
	}

	json_pairmake(instance, section, request, json, 0, &max);

	/*
	 *	Decrement reference count for root object, should free entire
	 *	JSON tree.
	 */
	json_object_put(json);

	return (REST_BODY_MAX_ATTRS - max);
}
#endif

/** Processes incoming HTTP header data from libcurl.
 *
 * Processes the status line, and Content-Type headers from the incoming HTTP
 * response.
 *
 * Matches prototype for CURLOPT_HEADERFUNCTION, and will be called directly
 * by libcurl.
 *
 * @param[in] ptr Char buffer where inbound header data is written.
 * @param[in] size Multiply by nmemb to get the length of ptr.
 * @param[in] nmemb Multiply by size to get the length of ptr.
 * @param[in] userdata rlm_rest_write_t to keep parsing state between calls.
 * @return Length of data processed, or 0 on error.
 */
static size_t rest_write_header(void *ptr, size_t size, size_t nmemb,
				void *userdata)
{
	rlm_rest_write_t *ctx  = userdata;
	REQUEST *request       = ctx->request; /* Used by RDEBUG */

	char const *p = ptr, *q;
	char *tmp;

	size_t const t = (size * nmemb);
	size_t s = t;
	size_t len;

	http_body_type_t type;
	http_body_type_t supp;

	switch (ctx->state) {
	case WRITE_STATE_INIT:
		RDEBUG("Processing header");

		/*
		 * HTTP/<version> <reason_code>[ <reason_phrase>]\r\n
		 *
		 * "HTTP/1.1 " (8) + "100 " (4) + "\r\n" (2) = 14
		 */
		if (s < 14) goto malformed;

		/*
		 * Check start of header matches...
		 */
		if (strncasecmp("HTTP/", p, 5) != 0) goto malformed;

		p += 5;
		s -= 5;

		/*
		 * Skip the version field, next space should mark start
		 * of reason_code.
		 */
		q = memchr(p, ' ', s);
		if (!q) goto malformed;

		s -= (q - p);
		p  = q;

		/*
		 * Process reason_code.
		 *
		 * " 100" (4) + "\r\n" (2) = 6
		 */
		if (s < 6) goto malformed;
		p++;
		s--;

		/* Char after reason code must be a space, or \r */
		if (!((p[3] == ' ') || (p[3] == '\r'))) goto malformed;

		ctx->code = atoi(p);

		/*
		 *	Process reason_phrase (if present).
		 */
		if (p[3] == ' ') {
			p += 4;
			s -= 4;

			q = memchr(p, '\r', s);
			if (!q) goto malformed;

			len = (q - p);

			tmp = rad_malloc(len + 1);
			strlcpy(tmp, p, len + 1);

			RDEBUG("\tStatus : %i (%s)", ctx->code, tmp);

			free(tmp);
		} else {
			RDEBUG("\tStatus : %i", ctx->code);
		}

		ctx->state = WRITE_STATE_PARSE_HEADERS;

		break;

	case WRITE_STATE_PARSE_HEADERS:
		if ((s >= 14) &&
		    (strncasecmp("Content-Type: ", p, 14) == 0)) {
			p += 14;
			s -= 14;

			/*
			 *	Check to see if there's a parameter
			 *	separator.
			 */
			q = memchr(p, ';', s);

			/*
			 *	If there's not, find the end of this
			 *	header.
			 */
			if (!q) q = memchr(p, '\r', s);

			len = (!q) ? s : (unsigned)(q - p);

			type = fr_substr2int(http_content_type_table,
				p, HTTP_BODY_UNKNOWN,
				len);

			supp = http_body_type_supported[type];

			tmp = rad_malloc(len + 1);
			strlcpy(tmp, p, len + 1);

			RDEBUG("\tType   : %s (%s)",
				fr_int2str(http_body_type_table, type,
					"¿Unknown?"), tmp);

			free(tmp);

			if (type == HTTP_BODY_UNKNOWN) {
				RDEBUG("Couldn't determine type, using"
				       " request type \"%s\".",
				       fr_int2str(http_body_type_table,
						  ctx->type,
						  "¿Unknown?"));

			} else if (supp == HTTP_BODY_UNSUPPORTED) {
				RDEBUG("Type \"%s\" is currently"
				       " unsupported",
				       fr_int2str(http_body_type_table,
						  type, "¿Unknown?"));
				ctx->type = HTTP_BODY_UNSUPPORTED;
			} else if (supp == HTTP_BODY_UNAVAILABLE) {
				RDEBUG("Type \"%s\" is currently"
				       " unavailable, please rebuild"
				       " this module with the required"
				       " headers",
				       fr_int2str(http_body_type_table,
						  type, "¿Unknown?"));
				ctx->type = HTTP_BODY_UNSUPPORTED;

			} else if (supp == HTTP_BODY_INVALID) {
				RDEBUG("Type \"%s\" is not a valid web"
				       " API data markup format",
				       fr_int2str(http_body_type_table,
						  type, "¿Unknown?"));

				ctx->type = HTTP_BODY_INVALID;

			} else if (type != ctx->type) {
				ctx->type = type;
			}
		}
		break;

	default:
		break;
	}
	return t;

	malformed:

	RDEBUG("Incoming header was malformed");
	ctx->code = -1;

	return (t - s);
}

/** Processes incoming HTTP body data from libcurl.
 *
 * Writes incoming body data to an intermediary buffer for later parsing by
 * one of the decode functions.
 *
 * @param[in] ptr Char buffer where inbound header data is written
 * @param[in] size Multiply by nmemb to get the length of ptr.
 * @param[in] nmemb Multiply by size to get the length of ptr.
 * @param[in] userdata rlm_rest_write_t to keep parsing state between calls.
 * @return length of data processed, or 0 on error.
 */
static size_t rest_write_body(void *ptr, size_t size, size_t nmemb,
			      void *userdata)
{
	rlm_rest_write_t *ctx  = userdata;
	REQUEST *request       = ctx->request; /* Used by RDEBUG */

	char const *p = ptr;
	char *tmp;

	size_t const t = (size * nmemb);

	/*
	 *	Any post processing of headers should go here...
	 */
	if (ctx->state == WRITE_STATE_PARSE_HEADERS) {
		ctx->state = WRITE_STATE_PARSE_CONTENT;
	}

	switch (ctx->type) {
	case HTTP_BODY_UNSUPPORTED:
		return t;

	case HTTP_BODY_INVALID:
		tmp = rad_malloc(t + 1);
		strlcpy(tmp, p, t + 1);

		RDEBUG2("%s", tmp);

		free(tmp);

		return t;

	default:
		if (t > (ctx->alloc - ctx->used)) {
			ctx->alloc += ((t + 1) > REST_BODY_INCR) ?
				t + 1 : REST_BODY_INCR;

			tmp = ctx->buffer;

			ctx->buffer = rad_malloc(ctx->alloc);

			/* If data has been written previously */
			if (tmp) {
				strlcpy(ctx->buffer, tmp,
				       (ctx->used + 1));
				free(tmp);
			}
		}
		strlcpy(ctx->buffer + ctx->used, p, t + 1);
		ctx->used += t;

		break;
	}

	return t;
}

/** (Re-)Initialises the data in a rlm_rest_write_t.
 *
 * This resets the values of the a rlm_rest_write_t to their defaults.
 * Must be called between encoding sessions.
 *
 * @see rest_write_body
 * @see rest_write_header
 *
 * @param[in] request Current request.
 * @param[in] ctx data to initialise.
 * @param[in] type Default http_body_type to use when decoding raw data, may be
 * overwritten by rest_write_header.
 */
static void rest_write_ctx_init(REQUEST *request, rlm_rest_write_t *ctx,
				http_body_type_t type)
{
	ctx->request	= request;
	ctx->type	= type;
	ctx->state	= WRITE_STATE_INIT;
	ctx->alloc	= 0;
	ctx->used	= 0;
	ctx->buffer	= NULL;
}

/** Frees the intermediary buffer created by rest_write.
 *
 * @param[in] ctx data to be freed.
 */
static void rest_write_free(rlm_rest_write_t *ctx)
{
	if (ctx->buffer != NULL) {
		free(ctx->buffer);
	}
}

/** Configures body specific curlopts.
 *
 * Configures libcurl handle to use either chunked mode, where the request
 * data will be sent using multiple HTTP requests, or contiguous mode where
 * the request data will be sent in a single HTTP request.
 *
 * @param[in] instance configuration data.
 * @param[in] section configuration data.
 * @param[in] request Current request.
 * @param[in] handle rlm_rest_handle_t to configure.
 * @param[in] func to pass to libcurl for chunked.
 *	      transfers (NULL if not using chunked mode).
 * @return 0 on success -1 on error.
 */
static int rest_request_config_body(UNUSED rlm_rest_t *instance,
				    rlm_rest_section_t *section,
				    REQUEST *request,
				    rlm_rest_handle_t *handle,
				    rest_read_t func)
{
	rlm_rest_curl_context_t *ctx = handle->ctx;
	CURL *candle	     	     = handle->handle;

	ssize_t len;
	CURLcode ret;

	if (section->chunk > 0) {
		ret = curl_easy_setopt(candle, CURLOPT_READDATA, &ctx->read);
		if (ret != CURLE_OK) goto error;

		ret = curl_easy_setopt(candle, CURLOPT_READFUNCTION, rest_encode_json);
		if (ret != CURLE_OK) goto error;
	} else {
		len = rest_read_wrapper(&ctx->body, func, REST_BODY_MAX_LEN,
					&ctx->read);
		if (len <= 0) {
			REDEBUG("Failed creating HTTP body content");
		}

		ret = curl_easy_setopt(candle, CURLOPT_POSTFIELDS, ctx->body);
		if (ret != CURLE_OK) goto error;

		ret = curl_easy_setopt(candle, CURLOPT_POSTFIELDSIZE, len);
		if (ret != CURLE_OK) goto error;
	}

	return 0;

	error:
	REDEBUG("Failed setting curl option: %i - %s", ret, curl_easy_strerror(ret));

	return -1;
}

/** Configures request curlopts.
 *
 * Configures libcurl handle setting various curlopts for things like local
 * client time, Content-Type, and other FreeRADIUS custom headers.
 *
 * Current FreeRADIUS custom headers are:
 *  - X-FreeRADIUS-Section	The module section being processed.
 *  - X-FreeRADIUS-Server	The current virtual server the REQUEST is
 *				passing through.
 *
 * Sets up callbacks for all response processing (buffers and body data).
 *
 * @param[in] instance configuration data.
 * @param[in] section configuration data.
 * @param[in] handle to configure.
 * @param[in] request Current request.
 * @param[in] method to use (HTTP verbs PUT, POST, DELETE etc...).
 * @param[in] type Content-Type for request encoding, also sets the default for decoding.
 * @param[in] username to use for HTTP authentication, may be NULL in which case configured defaults will be used.
 * @param[in] password to use for HTTP authentication, may be NULL in which case configured defaults will be used.
 * @param[in] uri buffer containing the expanded URI to send the request to.
 * @return 0 on success (all opts configured) -1 on error.
 */
int rest_request_config(rlm_rest_t *instance, rlm_rest_section_t *section,
			REQUEST *request, void *handle, http_method_t method,
			http_body_type_t type,
			char const *uri, char const *username, char const *password)
{
	rlm_rest_handle_t *randle	= handle;
	rlm_rest_curl_context_t *ctx	= randle->ctx;
	CURL *candle			= randle->handle;

	http_auth_type_t auth = section->auth;

	CURLcode ret;
	long val = 1;

	char buffer[512];

	rad_assert((!username && !password) || (username && password));

	buffer[(sizeof(buffer) - 1)] = '\0';

	/*
	 *	Setup any header options and generic headers.
	 */
	ret = curl_easy_setopt(candle, CURLOPT_URL, uri);
	if (ret != CURLE_OK) goto error;

	ret = curl_easy_setopt(candle, CURLOPT_USERAGENT, "FreeRADIUS");
	if (ret != CURLE_OK) goto error;

	snprintf(buffer, (sizeof(buffer) - 1), "Content-Type: %s",
		 fr_int2str(http_content_type_table, type, "¿Unknown?"));
	ctx->headers = curl_slist_append(ctx->headers, buffer);
	if (!ctx->headers) goto error_header;

	if (section->timeout) {
		ret = curl_easy_setopt(candle, CURLOPT_TIMEOUT,
				       section->timeout);
		if (ret != CURLE_OK) goto error;
	}

	ret = curl_easy_setopt(candle, CURLOPT_PROTOCOLS,
			       (CURLPROTO_HTTP | CURLPROTO_HTTPS));
	if (ret != CURLE_OK) goto error;

	/*
	 *	FreeRADIUS custom headers
	 */
  	snprintf(buffer, (sizeof(buffer) - 1), "X-FreeRADIUS-Section: %s",
		 section->name);
	ctx->headers = curl_slist_append(ctx->headers, buffer);
	if (!ctx->headers) goto error_header;

	snprintf(buffer, (sizeof(buffer) - 1), "X-FreeRADIUS-Server: %s",
		 request->server);
	ctx->headers = curl_slist_append(ctx->headers, buffer);
	if (!ctx->headers) goto error_header;

	/*
	 *	Configure HTTP verb (GET, POST, PUT, DELETE, other...)
	 */
	switch (method) {
	case HTTP_METHOD_GET :
		ret = curl_easy_setopt(candle, CURLOPT_HTTPGET,
				       val);
		if (ret != CURLE_OK) goto error;

		break;

	case HTTP_METHOD_POST :
		ret = curl_easy_setopt(candle, CURLOPT_POST,
				       val);
		if (ret != CURLE_OK) goto error;

		break;

	case HTTP_METHOD_PUT :
		ret = curl_easy_setopt(candle, CURLOPT_PUT,
				       val);
		if (ret != CURLE_OK) goto error;

		break;

	case HTTP_METHOD_DELETE :
		ret = curl_easy_setopt(candle, CURLOPT_HTTPGET,
				       val);
		if (ret != CURLE_OK) goto error;

		ret = curl_easy_setopt(candle,
				       CURLOPT_CUSTOMREQUEST, "DELETE");
		if (ret != CURLE_OK) goto error;

		break;

	case HTTP_METHOD_CUSTOM :
		ret = curl_easy_setopt(candle, CURLOPT_HTTPGET,
				       val);
		if (ret != CURLE_OK) goto error;

		ret = curl_easy_setopt(candle,
				       CURLOPT_CUSTOMREQUEST,
				       section->method);
		if (ret != CURLE_OK) goto error;
		break;

	default:
		assert(0);
		break;
	};

	/*
	 *	Set user based authentication parameters
	 */
	if (auth) {
		if ((auth >= HTTP_AUTH_BASIC) &&
	    	    (auth <= HTTP_AUTH_ANY_SAFE)) {
			ret = curl_easy_setopt(candle, CURLOPT_HTTPAUTH, http_curl_auth[auth]);
			if (ret != CURLE_OK) goto error;

			if (username) {
				ret = curl_easy_setopt(candle, CURLOPT_USERNAME, username);
				if (ret != CURLE_OK) {
					goto error;
				}
			} else if (section->username) {
				if (radius_xlat(buffer, sizeof(buffer), request, section->username, NULL, NULL) < 0) {
					goto error;
				}
				ret = curl_easy_setopt(candle, CURLOPT_USERNAME, buffer);
				if (ret != CURLE_OK) {
					goto error;
				}
			}

			if (password) {
				ret = curl_easy_setopt(candle, CURLOPT_PASSWORD, password);
				if (ret != CURLE_OK) {
					goto error;
				}
			} else if (section->password) {
				if (radius_xlat(buffer, sizeof(buffer), request, section->password, NULL, NULL) < 0) {
					goto error;
				}
				ret = curl_easy_setopt(candle, CURLOPT_PASSWORD, buffer);
				if (ret != CURLE_OK) {
					goto error;
				}
			}
#ifdef CURLOPT_TLSAUTH_USERNAME
		} else if (type == HTTP_AUTH_TLS_SRP) {
			ret = curl_easy_setopt(candle, CURLOPT_TLSAUTH_TYPE, http_curl_auth[auth]);

			if (username) {
				ret = curl_easy_setopt(candle, CURLOPT_TLSAUTH_USERNAME, username);
				if (ret != CURLE_OK) {
					goto error;
				}
			} else if (section->username) {
				if (radius_xlat(buffer, sizeof(buffer), request, section->username, NULL, NULL) < 0) {
					goto error;
				}
				ret = curl_easy_setopt(candle, CURLOPT_TLSAUTH_USERNAME, buffer);
				if (ret != CURLE_OK) {
					goto error;
				}
			}

			if (password) {
				ret = curl_easy_setopt(candle, CURLOPT_TLSAUTH_PASSWORD, password);
				if (ret != CURLE_OK) {
					goto error;
				}
			} else if (section->password) {
				if (radius_xlat(buffer, sizeof(buffer), request, section->password, NULL, NULL) < 0) {
					goto error;
				}
				ret = curl_easy_setopt(candle, CURLOPT_TLSAUTH_PASSWORD, buffer);
				if (ret != CURLE_OK) {
					goto error;
				}
			}
#endif
		}
	}

	/*
	 *	Set SSL/TLS authentication parameters
	 */
	if (section->tls_certificate_file) {
		ret = curl_easy_setopt(candle,
			       	       CURLOPT_SSLCERT,
				       section->tls_certificate_file);
		if (ret != CURLE_OK) goto error;
	}

	if (section->tls_private_key_file) {
		ret = curl_easy_setopt(candle,
			       	       CURLOPT_SSLKEY,
				       section->tls_private_key_file);
		if (ret != CURLE_OK) goto error;
	}

	if (section->tls_private_key_password) {
		ret = curl_easy_setopt(candle,
			       	       CURLOPT_KEYPASSWD,
				       section->tls_private_key_password);
		if (ret != CURLE_OK) goto error;
	}

	if (section->tls_ca_file) {
		ret = curl_easy_setopt(candle,
			       	       CURLOPT_ISSUERCERT,
				       section->tls_ca_file);
		if (ret != CURLE_OK) goto error;
	}

	if (section->tls_ca_path) {
		ret = curl_easy_setopt(candle,
			       	       CURLOPT_CAPATH,
				       section->tls_ca_path);
		if (ret != CURLE_OK) goto error;
	}

	if (section->tls_random_file) {
		ret = curl_easy_setopt(candle,
			       	       CURLOPT_RANDOM_FILE,
				       section->tls_random_file);
		if (ret != CURLE_OK) goto error;
	}

	if (section->tls_check_cert) {
		ret = curl_easy_setopt(candle,
				       CURLOPT_SSL_VERIFYHOST,
				       (section->tls_check_cert_cn == true) ?
					2 : 0);
		if (ret != CURLE_OK) goto error;
	} else {
		ret = curl_easy_setopt(candle,
		       CURLOPT_SSL_VERIFYPEER,
		       0);
		if (ret != CURLE_OK) goto error;
	}

	/*
	 *	Tell CURL how to get HTTP body content, and how to process
	 *	incoming data.
	 */
	rest_write_ctx_init(request, &ctx->write, type);

	ret = curl_easy_setopt(candle, CURLOPT_HEADERFUNCTION,
			       rest_write_header);
	if (ret != CURLE_OK) goto error;

	ret = curl_easy_setopt(candle, CURLOPT_HEADERDATA,
			       &ctx->write);
	if (ret != CURLE_OK) goto error;

	ret = curl_easy_setopt(candle, CURLOPT_WRITEFUNCTION,
			       rest_write_body);
	if (ret != CURLE_OK) goto error;

	ret = curl_easy_setopt(candle, CURLOPT_WRITEDATA,
			       &ctx->write);
	if (ret != CURLE_OK) goto error;

	switch (method) {
	case HTTP_METHOD_GET :
	case HTTP_METHOD_DELETE :
		return 0;

	case HTTP_METHOD_POST :
	case HTTP_METHOD_PUT :
	case HTTP_METHOD_CUSTOM :
		if (section->chunk > 0) {
			ctx->read.chunk = section->chunk;

			ctx->headers = curl_slist_append(ctx->headers,
							 "Expect:");
			if (!ctx->headers) goto error_header;

			ctx->headers = curl_slist_append(ctx->headers,
							 "Transfer-Encoding: chunked");
			if (!ctx->headers) goto error_header;
		}

		switch (type) {
#ifdef HAVE_JSON
		case HTTP_BODY_JSON:
			rest_read_ctx_init(request, &ctx->read, 1);

			if (rest_request_config_body(instance, section, request, handle,
						     rest_encode_json) < 0) {
				return -1;
			}

			break;
#endif

		case HTTP_BODY_POST:
			rest_read_ctx_init(request, &ctx->read, 0);

			if (rest_request_config_body(instance, section, request, handle,
						     rest_encode_post) < 0) {
				return -1;
			}

			break;

		default:
			assert(0);
		}

		ret = curl_easy_setopt(candle, CURLOPT_HTTPHEADER,
				       ctx->headers);
		if (ret != CURLE_OK) goto error;

		break;

	default:
		assert(0);
	};

	return 0;

	error:
	RDEBUG("Failed setting curl option: %i - %s", ret, curl_easy_strerror(ret));
	return -1;

	error_header:
	RDEBUG("Failed creating header", instance->xlat_name);
	return -1;
}

/** Sends a REST (HTTP) request.
 *
 * Send the actual REST request to the server. The response will be handled by
 * the numerous callbacks configured in rest_request_config.
 *
 * @param[in] instance configuration data.
 * @param[in] section configuration data.
 * @param[in] request Current request.
 * @param[in] handle to use.
 * @return 0 on success or -1 on error.
 */
int rest_request_perform(UNUSED rlm_rest_t *instance,
			 UNUSED rlm_rest_section_t *section,
			 REQUEST *request, void *handle)
{
	rlm_rest_handle_t *randle = handle;
	CURL *candle		  = randle->handle;
	CURLcode ret;

	ret = curl_easy_perform(candle);
	if (ret != CURLE_OK) {
		REDEBUG("Request failed: %i - %s", ret, curl_easy_strerror(ret));

		return -1;
	}

	return 0;
}

/** Sends the response to the correct decode function.
 *
 * Uses the Content-Type information written in rest_write_header to
 * determine the correct decode function to use. The decode function will
 * then convert the raw received data into VALUE_PAIRs.
 *
 * @param[in] instance configuration data.
 * @param[in] section configuration data.
 * @param[in] request Current request.
 * @param[in] handle to use.
 * @return 0 on success or -1 on error.
 */
int rest_request_decode(rlm_rest_t *instance,
			UNUSED rlm_rest_section_t *section,
			REQUEST *request, void *handle)
{
	rlm_rest_handle_t *randle	= handle;
	rlm_rest_curl_context_t *ctx	= randle->ctx;

	int ret = -1;	/* -Wsometimes-uninitialized */

	if (!ctx->write.buffer) {
		RDEBUG("Skipping attribute processing, no body data received");
		return ret;
	}

	RDEBUG("Processing body");

	switch (ctx->write.type) {
	case HTTP_BODY_POST:
		ret = rest_decode_post(instance, section, request,
				       handle, ctx->write.buffer,
				       ctx->write.used);
		break;
#ifdef HAVE_JSON
	case HTTP_BODY_JSON:
		ret = rest_decode_json(instance, section, request,
				       handle, ctx->write.buffer,
				       ctx->write.used);
		break;
#endif
	case HTTP_BODY_UNSUPPORTED:
	case HTTP_BODY_UNAVAILABLE:
	case HTTP_BODY_INVALID:
		return -1;

	default:
		assert(0);
	}

	return ret;
}

/** Cleans up after a REST request.
 *
 * Resets all options associated with a CURL handle, and frees any headers
 * associated with it.
 *
 * Calls rest_read_ctx_free and rest_write_free to free any memory used by
 * context data.
 *
 * @param[in] instance configuration data.
 * @param[in] section configuration data.
 * @param[in] handle to cleanup.
 */
void rest_request_cleanup(UNUSED rlm_rest_t *instance,
			  UNUSED rlm_rest_section_t *section, void *handle)
{
	rlm_rest_handle_t *randle	= handle;
	rlm_rest_curl_context_t *ctx	= randle->ctx;
	CURL *candle			= randle->handle;

	/*
   	 * Clear any previously configured options
   	 */
  	curl_easy_reset(candle);

	/*
   	 * Free header list
   	 */
  	if (ctx->headers != NULL) {
  		curl_slist_free_all(ctx->headers);
  		ctx->headers = NULL;
  	}

	/*
   	 * Free body data (only used if chunking is disabled)
   	 */
  	if (ctx->body != NULL) free(ctx->body);

  	/*
   	 * Free other context info
   	 */
  	rest_write_free(&ctx->write);
}

/** URL encodes a string.
 *
 * Encode special chars as per RFC 3986 section 4.
 *
 * @param[in] request Current request.
 * @param[out] out Where to write escaped string.
 * @param[in] outlen Size of out buffer.
 * @param[in] raw string to be urlencoded.
 * @param[in] arg pointer, gives context for escaping.
 * @return length of data written to out (excluding NULL).
 */
static size_t rest_uri_escape(UNUSED REQUEST *request, char *out, size_t outlen,
			      char const *raw, UNUSED void *arg)
{
	char *escaped;

	escaped = curl_escape(raw, strlen(raw));
	strlcpy(out, escaped, outlen);
	curl_free(escaped);

	return strlen(out);
}

/** Builds URI; performs XLAT expansions and encoding.
 *
 * Splits the URI into "http://example.org" and "/%{xlat}/query/?bar=foo"
 * Both components are expanded, but values expanded for the second component
 * are also url encoded.
 *
 * @param[out] out Where to write the pointer to the new buffer containing the escaped URI.
 * @param[in] instance configuration data.
 * @param[in] section configuration data.
 * @param[in] request Current request
 * @return length of data written to buffer (excluding NULL) or < 0 if an error
 *	occurred.
 */
ssize_t rest_uri_build(char **out, UNUSED rlm_rest_t *instance, rlm_rest_section_t *section, REQUEST *request)
{
	char const *p;
	char *path_exp = NULL;

	char *scheme;
	char const *path;

	ssize_t len, outlen;

	p = section->uri;

	/*
	 *	All URLs must contain at least <scheme>://<server>/
	 */
	p = strchr(p, ':');
	if (!p || (*++p != '/') || (*++p != '/')) {
		malformed:
		REDEBUG("Error URI is malformed, can't find start of path");
		return -1;
	}
	p = strchr(p + 1, '/');
	if (!p) {
		goto malformed;
	}

	len = (p - section->uri);

	/*
	 *	Allocate a temporary buffer to hold the first part of the URI
	 */
	scheme = talloc_array(request, char, len + 1);
	strlcpy(scheme, section->uri, len + 1);

	path = (section->uri + len);

	len = radius_axlat(out, request, scheme, NULL, NULL);
	talloc_free(scheme);
	if (len < 0) {
		TALLOC_FREE(*out);

		return 0;
	}

	outlen = len;

	len = radius_axlat(&path_exp, request, path, rest_uri_escape, NULL);
	if (len < 0) {
		TALLOC_FREE(*out);

		return 0;
	}

	*out = talloc_strdup_append(*out, path_exp);
	talloc_free(path_exp);

	return outlen += len;
}
