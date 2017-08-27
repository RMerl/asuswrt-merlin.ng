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
 * @brief Function prototypes and datatypes for the REST (HTTP) transport.
 * @file rest.h
 *
 * @copyright 2012-2013  Arran Cudbard-Bell <a.cudbard-bell@freeradius.org>
 */

RCSIDH(other_h, "$Id$")

#include <freeradius-devel/connection.h>
#include "config.h"

#ifdef HAVE_JSON_JSONH
#define HAVE_JSON
#endif

#define CURL_NO_OLDIES 1
#include <curl/curl.h>

#ifdef HAVE_JSON
#include <json/json.h>
#endif

#define REST_URI_MAX_LEN		2048
#define REST_BODY_MAX_LEN		8192
#define REST_BODY_INCR			512
#define REST_BODY_MAX_ATTRS		256

typedef enum {
	HTTP_METHOD_CUSTOM,
	HTTP_METHOD_GET,
	HTTP_METHOD_POST,
	HTTP_METHOD_PUT,
	HTTP_METHOD_DELETE
} http_method_t;

typedef enum {
	HTTP_BODY_UNKNOWN = 0,
	HTTP_BODY_UNSUPPORTED,
	HTTP_BODY_UNAVAILABLE,
	HTTP_BODY_INVALID,
	HTTP_BODY_POST,
	HTTP_BODY_JSON,
	HTTP_BODY_XML,
	HTTP_BODY_YAML,
	HTTP_BODY_HTML,
	HTTP_BODY_PLAIN,
	HTTP_BODY_NUM_ENTRIES
} http_body_type_t;

typedef enum {
	HTTP_AUTH_UNKNOWN = 0,
	HTTP_AUTH_NONE,
	HTTP_AUTH_TLS_SRP,
	HTTP_AUTH_BASIC,
	HTTP_AUTH_DIGEST,
	HTTP_AUTH_DIGEST_IE,
	HTTP_AUTH_GSSNEGOTIATE,
	HTTP_AUTH_NTLM,
	HTTP_AUTH_NTLM_WB,
	HTTP_AUTH_ANY,
	HTTP_AUTH_ANY_SAFE,
	HTTP_AUTH_NUM_ENTRIES
} http_auth_type_t;

/*
 *	Must be updated (in rest.c) if additional values are added to
 *	http_body_type_t
 */
extern const http_body_type_t http_body_type_supported[HTTP_BODY_NUM_ENTRIES];

extern const unsigned long http_curl_auth[HTTP_AUTH_NUM_ENTRIES];

extern const FR_NAME_NUMBER http_auth_table[];

extern const FR_NAME_NUMBER http_method_table[];

extern const FR_NAME_NUMBER http_body_type_table[];

extern const FR_NAME_NUMBER http_content_header_table[];

/*
 *	Structure for section configuration
 */
typedef struct rlm_rest_section_t {
	char const *name;
	char *uri;

	char *method_str;
	http_method_t method;

	char *body_str;
	http_body_type_t body;

	char *username;
	char *password;
	char *auth_str;
	http_auth_type_t auth;
	int require_auth;

	char *tls_certificate_file;
	char *tls_private_key_file;
	char *tls_private_key_password;
	char *tls_ca_file;
	char *tls_ca_path;
	char *tls_random_file;
	int tls_check_cert;
	int tls_check_cert_cn;

	int timeout;
	unsigned int chunk;
} rlm_rest_section_t;

/*
 *	Structure for module configuration
 */
typedef struct rlm_rest_t {
	char const *xlat_name;

	char *connect_uri;

	fr_connection_pool_t *conn_pool;

	rlm_rest_section_t authorize;
	rlm_rest_section_t authenticate;
	rlm_rest_section_t accounting;
	rlm_rest_section_t checksimul;
	rlm_rest_section_t postauth;
} rlm_rest_t;

/*
 *	States for stream based attribute encoders
 */
typedef enum {
	READ_STATE_INIT	= 0,
	READ_STATE_ATTR_BEGIN,
	READ_STATE_ATTR_CONT,
	READ_STATE_END,
} read_state_t;

/*
 *	States for the response parser
 */
typedef enum {
	WRITE_STATE_INIT = 0,
	WRITE_STATE_PARSE_HEADERS,
	WRITE_STATE_PARSE_CONTENT,
	WRITE_STATE_DISCARD,
} write_state_t;

/*
 *	Outbound data context (passed to CURLOPT_READFUNCTION as CURLOPT_READDATA)
 */
typedef struct rlm_rest_read_t {
	rlm_rest_t 	*instance;
	REQUEST 	*request;
	read_state_t 	state;

	vp_cursor_t 	cursor;

	unsigned int	chunk;
} rlm_rest_read_t;

/*
 *	Curl inbound data context (passed to CURLOPT_WRITEFUNCTION and
 *	CURLOPT_HEADERFUNCTION as CURLOPT_WRITEDATA and CURLOPT_HEADERDATA)
 */
typedef struct rlm_rest_write_t {
	rlm_rest_t	 *instance;
	REQUEST		 *request;
	write_state_t	 state;

	char 		 *buffer;	/* HTTP incoming raw data */
	size_t		 alloc;		/* Space allocated for buffer */
	size_t		 used;		/* Space used in buffer */

	int		 code;		/* HTTP Status Code */
	http_body_type_t type;		/* HTTP Content Type */
} rlm_rest_write_t;

/*
 *	Curl context data
 */
typedef struct rlm_rest_curl_context_t {
	struct curl_slist	*headers;
	char			*body;
	rlm_rest_read_t		read;
	rlm_rest_write_t	write;
} rlm_rest_curl_context_t;

/*
 *	Connection API handle
 */
typedef struct rlm_rest_handle_t {
	void	*handle;	/* Real Handle */
	void	*ctx;		/* Context */
} rlm_rest_handle_t;

/*
 *	Function prototype for rest_read_wrapper. Matches CURL's
 *	CURLOPT_READFUNCTION prototype.
 */
typedef size_t (*rest_read_t)(void *ptr, size_t size, size_t nmemb,
			      void *userdata);

/*
 *	Connection API callbacks
 */
int rest_init(rlm_rest_t *instance);

void rest_cleanup(void);

void *mod_conn_create(void *instance);

int mod_conn_alive(void *instance, void *handle);

int mod_conn_delete(void *instance, void *handle);

/*
 *	Request processing API
 */
int rest_request_config(rlm_rest_t *instance,
			rlm_rest_section_t *section, REQUEST *request,
			void *handle, http_method_t method,
			http_body_type_t type, char const *uri,
			char const *username, char const *password);

int rest_request_perform(rlm_rest_t *instance,
			 rlm_rest_section_t *section, REQUEST *request,
			 void *handle);

int rest_request_decode(rlm_rest_t *instance,
			UNUSED rlm_rest_section_t *section, REQUEST *request,
			void *handle);

void rest_request_cleanup(rlm_rest_t *instance, rlm_rest_section_t *section,
			  void *handle);

#define rest_get_handle_code(handle)(((rlm_rest_curl_context_t*)((rlm_rest_handle_t*)handle)->ctx)->write.code)

#define rest_get_handle_type(handle)(((rlm_rest_curl_context_t*)((rlm_rest_handle_t*)handle)->ctx)->write.type)

/*
 *	Helper functions
 */
ssize_t rest_uri_build(char **out, rlm_rest_t *instance, rlm_rest_section_t *section, REQUEST *request);
