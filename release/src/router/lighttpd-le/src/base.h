#ifndef _BASE_H_
#define _BASE_H_
#include "first.h"

#include "settings.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <limits.h>

#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif

#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#endif

#include "buffer.h"
#include "array.h"
#include "chunk.h"
#include "keyvalue.h"
#include "fdevent.h"
#include "sys-socket.h"
#include "splaytree.h"
#include "etag.h"


#if defined HAVE_LIBSSL && defined HAVE_OPENSSL_SSL_H
# define USE_OPENSSL
# include <openssl/opensslconf.h>
#  ifndef USE_OPENSSL_KERBEROS
#   ifndef OPENSSL_NO_KRB5
#   define OPENSSL_NO_KRB5
#   endif
#  endif
# include <openssl/ssl.h>
# if ! defined OPENSSL_NO_TLSEXT && ! defined SSL_CTRL_SET_TLSEXT_HOSTNAME
#  define OPENSSL_NO_TLSEXT
# endif
#endif

#ifdef HAVE_FAM_H
# include <fam.h>
#endif

#ifndef O_BINARY
# define O_BINARY 0
#endif

#ifndef O_LARGEFILE
# define O_LARGEFILE 0
#endif

#ifndef SIZE_MAX
# ifdef SIZE_T_MAX
#  define SIZE_MAX SIZE_T_MAX
# else
#  define SIZE_MAX ((size_t)~0)
# endif
#endif

#ifndef SSIZE_MAX
# define SSIZE_MAX ((size_t)~0 >> 1)
#endif

#ifdef __APPLE__
#include <crt_externs.h>
#define environ (* _NSGetEnviron())
#else
extern char **environ;
#endif

/* for solaris 2.5 and NetBSD 1.3.x */
#ifndef HAVE_SOCKLEN_T
typedef int socklen_t;
#endif

/* solaris and NetBSD 1.3.x again */
#if (!defined(HAVE_STDINT_H)) && (!defined(HAVE_INTTYPES_H)) && (!defined(uint32_t))
# define uint32_t u_int32_t
#endif


#ifndef SHUT_WR
# define SHUT_WR 1
#endif

typedef enum { T_CONFIG_UNSET,
		T_CONFIG_STRING,
		T_CONFIG_SHORT,
		T_CONFIG_INT,
		T_CONFIG_BOOLEAN,
		T_CONFIG_ARRAY,
		T_CONFIG_LOCAL,
		T_CONFIG_DEPRECATED,
		T_CONFIG_UNSUPPORTED
} config_values_type_t;

typedef enum { T_CONFIG_SCOPE_UNSET,
		T_CONFIG_SCOPE_SERVER,
		T_CONFIG_SCOPE_CONNECTION
} config_scope_type_t;

typedef struct {
	const char *key;
	void *destination;

	config_values_type_t type;
	config_scope_type_t scope;
} config_values_t;

typedef enum { DIRECT, EXTERNAL } connection_type;

typedef struct {
	char *key;
	connection_type type;
	char *value;
} request_handler;

typedef struct {
	char *key;
	char *host;
	unsigned short port;
	int used;
	short factor;
} fcgi_connections;


typedef union {
#ifdef HAVE_IPV6
	struct sockaddr_in6 ipv6;
#endif
	struct sockaddr_in ipv4;
#ifdef HAVE_SYS_UN_H
	struct sockaddr_un un;
#endif
	struct sockaddr plain;
} sock_addr;

/* fcgi_response_header contains ... */
#define HTTP_STATUS         BV(0)
#define HTTP_CONNECTION     BV(1)
#define HTTP_CONTENT_LENGTH BV(2)
#define HTTP_DATE           BV(3)
#define HTTP_LOCATION       BV(4)

typedef struct {
	/** HEADER */
	/* the request-line */
	buffer *request;
	buffer *uri;

	buffer *orig_uri;

	http_method_t  http_method;
	http_version_t http_version;

	buffer *request_line;

	/* strings to the header */
	buffer *http_host; /* not alloced */
	const char   *http_range;
	const char   *http_content_type;
	const char   *http_if_modified_since;
	const char   *http_if_none_match;

	array  *headers;

	/* CONTENT */
	off_t content_length; /* returned by strtoll() */
	off_t te_chunked;

	/* internal representation */
	int     accept_encoding;

	/* internal */
	buffer *pathinfo;
} request;

typedef struct {
	off_t   content_length;
	int     keep_alive;               /* used by  the subrequests in proxy, cgi and fcgi to say the subrequest was keep-alive or not */

	array  *headers;

	enum {
		HTTP_TRANSFER_ENCODING_IDENTITY, HTTP_TRANSFER_ENCODING_CHUNKED
	} transfer_encoding;
} response;

typedef struct {
	buffer *scheme; /* scheme without colon or slashes ( "http" or "https" ) */

	/* authority with optional portnumber ("site.name" or "site.name:8080" ) NOTE: without "username:password@" */
	buffer *authority;

	/* path including leading slash ("/" or "/index.html") - urldecoded, and sanitized  ( buffer_path_simplify() && buffer_urldecode_path() ) */
	buffer *path;
	buffer *path_raw; /* raw path, as sent from client. no urldecoding or path simplifying */
	buffer *query; /* querystring ( everything after "?", ie: in "/index.php?foo=1", query is "foo=1" ) */
} request_uri;

typedef struct {
	buffer *path;
	buffer *basedir; /* path = "(basedir)(.*)" */

	buffer *doc_root; /* path = doc_root + rel_path */
	buffer *rel_path;

	buffer *etag;
} physical;

typedef struct {
	buffer *name;
	buffer *etag;

	struct stat st;

	time_t stat_ts;

#ifdef HAVE_LSTAT
	char is_symlink;
#endif

#ifdef HAVE_FAM_H
	int    dir_version;
#endif

	buffer *content_type;
} stat_cache_entry;

typedef struct {
	splay_tree *files; /* the nodes of the tree are stat_cache_entry's */

	buffer *dir_name; /* for building the dirname from the filename */
#ifdef HAVE_FAM_H
	splay_tree *dirs; /* the nodes of the tree are fam_dir_entry */

	FAMConnection fam;
	int    fam_fcce_ndx;
#endif
	buffer *hash_key;  /* temp-store for the hash-key */
} stat_cache;

typedef struct {
	array *mimetypes;

	/* virtual-servers */
	buffer *document_root;
	buffer *server_name;
	buffer *error_handler;
	buffer *error_handler_404;
	buffer *server_tag;
	buffer *dirlist_encoding;
	buffer *errorfile_prefix;

	unsigned short high_precision_timestamps;
	unsigned short max_keep_alive_requests;
	unsigned short max_keep_alive_idle;
	unsigned short max_read_idle;
	unsigned short max_write_idle;
	unsigned short use_xattr;
	unsigned short follow_symlink;
	unsigned short range_requests;
	unsigned short stream_request_body;
	unsigned short stream_response_body;

	/* debug */

	unsigned short log_file_not_found;
	unsigned short log_request_header;
	unsigned short log_request_handling;
	unsigned short log_response_header;
	unsigned short log_condition_handling;
	unsigned short log_ssl_noise;
	unsigned short log_timeouts;


	/* server wide */
	buffer *ssl_pemfile;
	buffer *ssl_ca_file;
	buffer *ssl_cipher_list;
	buffer *ssl_dh_file;
	buffer *ssl_ec_curve;
	unsigned short ssl_honor_cipher_order; /* determine SSL cipher in server-preferred order, not client-order */
	unsigned short ssl_empty_fragments; /* whether to not set SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS */
	unsigned short ssl_use_sslv2;
	unsigned short ssl_use_sslv3;
	unsigned short ssl_verifyclient;
	unsigned short ssl_verifyclient_enforce;
	unsigned short ssl_verifyclient_depth;
	buffer *ssl_verifyclient_username;
	unsigned short ssl_verifyclient_export_cert;
	unsigned short ssl_disable_client_renegotiation;
	unsigned short ssl_read_ahead;

	unsigned short use_ipv6, set_v6only; /* set_v6only is only a temporary option */
	unsigned short defer_accept;
	unsigned short ssl_enabled; /* only interesting for setting up listening sockets. don't use at runtime */
	unsigned short allow_http11;
	unsigned short etag_use_inode;
	unsigned short etag_use_mtime;
	unsigned short etag_use_size;
	unsigned short force_lowercase_filenames; /* if the FS is case-insensitive, force all files to lower-case */
	unsigned int http_parseopts;
	unsigned int max_request_size;
	int listen_backlog;

	unsigned short kbytes_per_second; /* connection kb/s limit */

	/* configside */
	unsigned short global_kbytes_per_second; /*  */

	off_t  global_bytes_per_second_cnt;
	/* server-wide traffic-shaper
	 *
	 * each context has the counter which is inited once
	 * a second by the global_kbytes_per_second config-var
	 *
	 * as soon as global_kbytes_per_second gets below 0
	 * the connected conns are "offline" a little bit
	 *
	 * the problem:
	 * we somehow have to loose our "we are writable" signal
	 * on the way.
	 *
	 */
	off_t *global_bytes_per_second_cnt_ptr; /*  */

#if defined(__FreeBSD__) || defined(__NetBSD__) \
 || defined(__OpenBSD__) || defined(__DragonFly__)
	buffer *bsd_accept_filter;
#endif

#ifdef USE_OPENSSL
	SSL_CTX *ssl_ctx; /* not patched */
	/* SNI per host: with COMP_SERVER_SOCKET, COMP_HTTP_SCHEME, COMP_HTTP_HOST */
	EVP_PKEY *ssl_pemfile_pkey;
	X509 *ssl_pemfile_x509;
	STACK_OF(X509_NAME) *ssl_ca_file_cert_names;
#endif
} specific_config;

/* the order of the items should be the same as they are processed
 * read before write as we use this later */
typedef enum {
	CON_STATE_CONNECT,
	CON_STATE_REQUEST_START,
	CON_STATE_READ,
	CON_STATE_REQUEST_END,
	CON_STATE_READ_POST,
	CON_STATE_HANDLE_REQUEST,
	CON_STATE_RESPONSE_START,
	CON_STATE_WRITE,
	CON_STATE_RESPONSE_END,
	CON_STATE_ERROR,
	CON_STATE_CLOSE
} connection_state_t;

typedef enum {
	/* condition not active at the moment because itself or some
	 * pre-condition depends on data not available yet
	 */
	COND_RESULT_UNSET,

	/* special "unset" for branches not selected due to pre-conditions
	 * not met (but pre-conditions are not "unset" anymore)
	 */
	COND_RESULT_SKIP,

	/* actually evaluated the condition itself */
	COND_RESULT_FALSE, /* not active */
	COND_RESULT_TRUE, /* active */
} cond_result_t;

typedef struct {
	/* current result (with preconditions) */
	cond_result_t result;
	/* result without preconditions (must never be "skip") */
	cond_result_t local_result;
	int patterncount;
	int matches[3 * 10];
	buffer *comp_value; /* just a pointer */
} cond_cache_t;

typedef struct {
	connection_state_t state;

	/* timestamps */
	time_t read_idle_ts;
	time_t close_timeout_ts;
	time_t write_request_ts;

	time_t connection_start;
	time_t request_start;
	struct timespec request_start_hp;

	size_t request_count;        /* number of requests handled in this connection */
	size_t loops_per_request;    /* to catch endless loops in a single request
				      *
				      * used by mod_rewrite, mod_fastcgi, ... and others
				      * this is self-protection
				      */

	int fd;                      /* the FD for this connection */
	int fde_ndx;                 /* index for the fdevent-handler */
	int ndx;                     /* reverse mapping to server->connection[ndx] */

	/* fd states */
	int is_readable;
	int is_writable;

	int keep_alive;              /* only request.c can enable it, all other just disable */
	int keep_alive_idle;         /* remember max_keep_alive_idle from config */

	int file_started;
	int file_finished;

	chunkqueue *write_queue;      /* a large queue for low-level write ( HTTP response ) [ file, mem ] */
	chunkqueue *read_queue;       /* a small queue for low-level read ( HTTP request ) [ mem ] */
	chunkqueue *request_content_queue; /* takes request-content into tempfile if necessary [ tempfile, mem ]*/

	int traffic_limit_reached;

	off_t bytes_written;          /* used by mod_accesslog, mod_rrd */
	off_t bytes_written_cur_second; /* used by mod_accesslog, mod_rrd */
	off_t bytes_read;             /* used by mod_accesslog, mod_rrd */
	off_t bytes_header;

	int http_status;

	sock_addr dst_addr;
	buffer *dst_addr_buf;

	/* request */
	buffer *parse_request;
	unsigned int parsed_response; /* bitfield which contains the important header-fields of the parsed response header */

	request  request;
	request_uri uri;
	physical physical;
	response response;

	size_t header_len;

	array  *environment; /* used to pass lighttpd internal stuff to the FastCGI/CGI apps, setenv does that */

	/* response */
	int    got_response;

	int    in_joblist;

	connection_type mode;

	void **plugin_ctx;           /* plugin connection specific config */

	specific_config conf;        /* global connection specific config */
	cond_cache_t *cond_cache;

	buffer *server_name;

	/* error-handler */
	int error_handler_saved_status;
	http_method_t error_handler_saved_method;

	struct server_socket *srv_socket;   /* reference to the server-socket */

#ifdef USE_OPENSSL
	SSL *ssl;
# ifndef OPENSSL_NO_TLSEXT
	buffer *tlsext_server_name;
# endif
	unsigned int renegotiations; /* count of SSL_CB_HANDSHAKE_START */
#endif
	/* etag handling */
	etag_flags_t etag_flags;

	int conditional_is_valid[COMP_LAST_ELEMENT]; 
} connection;

typedef struct {
	connection **ptr;
	size_t size;
	size_t used;
} connections;


#ifdef HAVE_IPV6
typedef struct {
	int family;
	union {
		struct in6_addr ipv6;
		struct in_addr  ipv4;
	} addr;
	char b2[INET6_ADDRSTRLEN + 1];
	time_t ts;
} inet_ntop_cache_type;
#endif


typedef struct {
	buffer *uri;
	time_t mtime;
	int http_status;
} realpath_cache_type;

typedef struct {
	time_t  mtime;  /* the key */
	buffer *str;    /* a buffer for the string represenation */
} mtime_cache_type;

typedef struct {
	void  *ptr;
	size_t used;
	size_t size;
} buffer_plugin;

typedef struct {
	unsigned short port;
	buffer *bindhost;

	buffer *errorlog_file;
	unsigned short errorlog_use_syslog;
	buffer *breakagelog_file;

	unsigned short dont_daemonize;
	unsigned short preflight_check;
	buffer *changeroot;
	buffer *username;
	buffer *groupname;

	buffer *pid_file;

	buffer *event_handler;

	buffer *modules_dir;
	buffer *network_backend;
	array *modules;
	array *upload_tempdirs;
	unsigned int upload_temp_file_size;
	unsigned int max_request_field_size;

	unsigned short max_worker;
	unsigned short max_fds;
	unsigned short max_conns;

	unsigned short log_request_header_on_error;
	unsigned short log_state_handling;

	enum { STAT_CACHE_ENGINE_UNSET,
			STAT_CACHE_ENGINE_NONE,
			STAT_CACHE_ENGINE_SIMPLE
#ifdef HAVE_FAM_H
			, STAT_CACHE_ENGINE_FAM
#endif
	} stat_cache_engine;
	unsigned short enable_cores;
	unsigned short reject_expect_100_with_417;
	buffer *xattr_name;

	unsigned short http_header_strict;
	unsigned short http_host_strict;
	unsigned short http_host_normalize;
	unsigned short high_precision_timestamps;
	time_t loadts;
	double loadavg[3];
} server_config;

typedef struct server_socket {
	sock_addr addr;
	int       fd;
	int       fde_ndx;

	unsigned short is_ssl;

	buffer *srv_token;

#ifdef USE_OPENSSL
	SSL_CTX *ssl_ctx;
#endif
} server_socket;

typedef struct {
	server_socket **ptr;

	size_t size;
	size_t used;
} server_socket_array;

typedef struct server {
	server_socket_array srv_sockets;

	/* the errorlog */
	int errorlog_fd;
	enum { ERRORLOG_FILE, ERRORLOG_FD, ERRORLOG_SYSLOG, ERRORLOG_PIPE } errorlog_mode;
	buffer *errorlog_buf;

	fdevents *ev, *ev_ins;

	buffer_plugin plugins;
	void *plugin_slots;

	/* counters */
	int con_opened;
	int con_read;
	int con_written;
	int con_closed;

	int ssl_is_init;

	int max_fds;    /* max possible fds */
	int cur_fds;    /* currently used fds */
	int want_fds;   /* waiting fds */
	int sockets_disabled;

	size_t max_conns;

	/* buffers */
	buffer *parse_full_path;
	buffer *response_header;
	buffer *response_range;
	buffer *tmp_buf;

	buffer *tmp_chunk_len;

	buffer *empty_string; /* is necessary for cond_match */

	buffer *cond_check_buf;

	/* caches */
#ifdef HAVE_IPV6
	inet_ntop_cache_type inet_ntop_cache[INET_NTOP_CACHE_MAX];
#endif
	mtime_cache_type mtime_cache[FILE_CACHE_MAX];

	array *split_vals;

	/* Timestamps */
	time_t cur_ts;
	time_t last_generated_date_ts;
	time_t last_generated_debug_ts;
	time_t startup_ts;

	buffer *ts_debug_str;
	buffer *ts_date_str;

	/* config-file */
	array *config_touched;

	array *config_context;
	specific_config **config_storage;

	server_config  srvconf;

	short int config_deprecated;
	short int config_unsupported;

	connections *conns;
	connections *joblist;
	connections *fdwaitqueue;

	stat_cache  *stat_cache;

	/**
	 * The status array can carry all the status information you want
	 * the key to the array is <module-prefix>.<name>
	 * and the values are counters
	 *
	 * example:
	 *   fastcgi.backends        = 10
	 *   fastcgi.active-backends = 6
	 *   fastcgi.backend.<key>.load = 24
	 *   fastcgi.backend.<key>....
	 *
	 *   fastcgi.backend.<key>.disconnects = ...
	 */
	array *status;

	fdevent_handler_t event_handler;

	int (* network_backend_write)(struct server *srv, connection *con, int fd, chunkqueue *cq, off_t max_bytes);
#ifdef USE_OPENSSL
	int (* network_ssl_backend_write)(struct server *srv, connection *con, SSL *ssl, chunkqueue *cq, off_t max_bytes);
#endif

	uid_t uid;
	gid_t gid;
} server;


#endif
