#ifndef _KEY_VALUE_H_
#define _KEY_VALUE_H_
#include "first.h"

#ifdef HAVE_PCRE_H
# include <pcre.h>
#endif

struct server;

/* sources:
 * - [RFC2616], Section 9
 *   (or http://tools.ietf.org/html/draft-ietf-httpbis-p2-semantics-22)
 * - http://tools.ietf.org/html/draft-ietf-httpbis-method-registrations-11, Appendix A
 *
 * http://tools.ietf.org/html/draft-ietf-httpbis-p2-semantics-22, Section 8.1 defines
 * a new registry (not available yet):
 *   http://www.iana.org/assignments/http-methods
 */

typedef enum {
	HTTP_METHOD_UNSET = -1,
	HTTP_METHOD_GET,               /* [RFC2616], Section 9.3 */
	HTTP_METHOD_HEAD,              /* [RFC2616], Section 9.4 */
	HTTP_METHOD_POST,              /* [RFC2616], Section 9.5 */
	HTTP_METHOD_PUT,               /* [RFC2616], Section 9.6 */
	HTTP_METHOD_DELETE,            /* [RFC2616], Section 9.7 */
	HTTP_METHOD_CONNECT,           /* [RFC2616], Section 9.9 */
	HTTP_METHOD_OPTIONS,           /* [RFC2616], Section 9.2 */
	HTTP_METHOD_TRACE,             /* [RFC2616], Section 9.8 */
	HTTP_METHOD_ACL,               /* [RFC3744], Section 8.1 */
	HTTP_METHOD_BASELINE_CONTROL,  /* [RFC3253], Section 12.6 */
	HTTP_METHOD_BIND,              /* [RFC5842], Section 4 */
	HTTP_METHOD_CHECKIN,           /* [RFC3253], Section 4.4 and [RFC3253], Section 9.4 */
	HTTP_METHOD_CHECKOUT,          /* [RFC3253], Section 4.3 and [RFC3253], Section 8.8 */
	HTTP_METHOD_COPY,              /* [RFC4918], Section 9.8 */
	HTTP_METHOD_LABEL,             /* [RFC3253], Section 8.2 */
	HTTP_METHOD_LINK,              /* [RFC2068], Section 19.6.1.2 */
	HTTP_METHOD_LOCK,              /* [RFC4918], Section 9.10 */
	HTTP_METHOD_MERGE,             /* [RFC3253], Section 11.2 */
	HTTP_METHOD_MKACTIVITY,        /* [RFC3253], Section 13.5 */
	HTTP_METHOD_MKCALENDAR,        /* [RFC4791], Section 5.3.1 */
	HTTP_METHOD_MKCOL,             /* [RFC4918], Section 9.3 */
	HTTP_METHOD_MKREDIRECTREF,     /* [RFC4437], Section 6 */
	HTTP_METHOD_MKWORKSPACE,       /* [RFC3253], Section 6.3 */
	HTTP_METHOD_MOVE,              /* [RFC4918], Section 9.9 */
	HTTP_METHOD_ORDERPATCH,        /* [RFC3648], Section 7 */
	HTTP_METHOD_PATCH,             /* [RFC5789], Section 2 */
	HTTP_METHOD_PROPFIND,          /* [RFC4918], Section 9.1 */
	HTTP_METHOD_PROPPATCH,         /* [RFC4918], Section 9.2 */
	HTTP_METHOD_REBIND,            /* [RFC5842], Section 6 */
	HTTP_METHOD_REPORT,            /* [RFC3253], Section 3.6 */
	HTTP_METHOD_SEARCH,            /* [RFC5323], Section 2 */
	HTTP_METHOD_UNBIND,            /* [RFC5842], Section 5 */
	HTTP_METHOD_UNCHECKOUT,        /* [RFC3253], Section 4.5 */
	HTTP_METHOD_UNLINK,            /* [RFC2068], Section 19.6.1.3 */
	HTTP_METHOD_UNLOCK,            /* [RFC4918], Section 9.11 */
	HTTP_METHOD_UPDATE,            /* [RFC3253], Section 7.1 */
	HTTP_METHOD_UPDATEREDIRECTREF, /* [RFC4437], Section 7 */
	HTTP_METHOD_VERSION_CONTROL    /* [RFC3253], Section 3.5 */
} http_method_t;

typedef enum { HTTP_VERSION_UNSET = -1, HTTP_VERSION_1_0, HTTP_VERSION_1_1 } http_version_t;

typedef struct {
	int key;

	char *value;
} keyvalue;

typedef struct {
#ifdef HAVE_PCRE_H
	pcre *key;
	pcre_extra *key_extra;
#endif

	buffer *value;
} pcre_keyvalue;

typedef struct {
	pcre_keyvalue **kv;
	size_t used;
	size_t size;
} pcre_keyvalue_buffer;

const char *get_http_status_name(int i);
const char *get_http_version_name(int i);
const char *get_http_method_name(http_method_t i);
const char *get_http_status_body_name(int i);
int get_http_version_key(const char *s);
http_method_t get_http_method_key(const char *s);

const char *keyvalue_get_value(keyvalue *kv, int k);
int keyvalue_get_key(keyvalue *kv, const char *s);

pcre_keyvalue_buffer *pcre_keyvalue_buffer_init(void);
int pcre_keyvalue_buffer_append(struct server *srv, pcre_keyvalue_buffer *kvb, const char *key, const char *value);
void pcre_keyvalue_buffer_free(pcre_keyvalue_buffer *kvb);

#endif
