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
#ifndef RADIUSD_H
#define RADIUSD_H
/*
 * $Id$
 *
 * @file radiusd.h
 * @brief Structures, prototypes and global variables for the FreeRADIUS server.
 *
 * @copyright 1999-2000,2002-2008  The FreeRADIUS server project
 */

RCSIDH(radiusd_h, "$Id$")

#include <freeradius-devel/libradius.h>
#include <freeradius-devel/radpaths.h>
#include <freeradius-devel/conf.h>
#include <freeradius-devel/conffile.h>
#include <freeradius-devel/event.h>
#include <freeradius-devel/connection.h>
#include <freeradius-devel/map.h>

typedef struct request REQUEST;

#include <freeradius-devel/log.h>

#ifdef HAVE_PTHREAD_H
#include	<pthread.h>
#endif

#ifdef HAVE_PCREPOSIX_H
#include <pcreposix.h>
#else
#ifdef HAVE_REGEX_H
#	include <regex.h>

/*
 *  For POSIX Regular expressions.
 *  (0) Means no extended regular expressions.
 *  REG_EXTENDED means use extended regular expressions.
 */
#ifndef REG_EXTENDED
#define REG_EXTENDED (0)
#endif

#ifndef REG_NOSUB
#define REG_NOSUB (0)
#endif
#endif
#endif

#ifndef NDEBUG
#define REQUEST_MAGIC (0xdeadbeef)
#endif

/*
 *	WITH_VMPS is handled by src/include/features.h
 */
#ifdef WITHOUT_VMPS
#undef WITH_VMPS
#endif

#ifdef WITH_TLS
#include <freeradius-devel/tls.h>
#endif

#include <freeradius-devel/stats.h>
#include <freeradius-devel/realms.h>

#ifdef WITH_COMMAND_SOCKET
#define PW_RADMIN_PORT 18120
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	See util.c
 */
typedef struct request_data_t request_data_t;

typedef struct radclient {
	fr_ipaddr_t		ipaddr;
	fr_ipaddr_t		src_ipaddr;
	int			prefix;
	char const		*longname;
	char const		*secret;
	char const		*shortname;
	int			message_authenticator;
	char			*nas_type;
	char			*login;
	char			*password;
	char			*server;
	int			number;	/* internal use only */
	CONF_SECTION const 	*cs;
#ifdef WITH_STATS
	fr_stats_t		auth;
#ifdef WITH_ACCOUNTING
	fr_stats_t		acct;
#endif
#ifdef WITH_COA
	fr_stats_t		coa;
	fr_stats_t		dsc;
#endif
#endif

	int			proto;
#ifdef WITH_TCP
	fr_socket_limit_t	limit;
#endif

#ifdef WITH_DYNAMIC_CLIENTS
	int			lifetime;
	int			dynamic; /* was dynamically defined */
	time_t			created;
	time_t			last_new_client;
	char			*client_server;
	int			rate_limit;
#endif

#ifdef WITH_COA
	char			*coa_name;
	home_server		*coa_server;
	home_pool_t		*coa_pool;
#endif
} RADCLIENT;

/*
 *	Types of listeners.
 *
 *	Ordered by priority!
 */
typedef enum RAD_LISTEN_TYPE {
	RAD_LISTEN_NONE = 0,
#ifdef WITH_PROXY
	RAD_LISTEN_PROXY,
#endif
	RAD_LISTEN_AUTH,
#ifdef WITH_ACCOUNTING
	RAD_LISTEN_ACCT,
#endif
#ifdef WITH_DETAIL
	RAD_LISTEN_DETAIL,
#endif
#ifdef WITH_VMPS
	RAD_LISTEN_VQP,
#endif
#ifdef WITH_DHCP
	RAD_LISTEN_DHCP,
#endif
#ifdef WITH_COMMAND_SOCKET
	RAD_LISTEN_COMMAND,
#endif
#ifdef WITH_COA
	RAD_LISTEN_COA,
#endif
	RAD_LISTEN_MAX
} RAD_LISTEN_TYPE;


/*
 *	For listening on multiple IP's and ports.
 */
typedef struct rad_listen_t rad_listen_t;

typedef		void (*fr_request_process_t)(REQUEST *, int);
/*
 *  Function handler for requests.
 */
typedef		int (*RAD_REQUEST_FUNP)(REQUEST *);

#define REQUEST_DATA_REGEX (0xadbeef00)
#define REQUEST_MAX_REGEX (8)

#if defined(WITH_VERIFY_PTR)
#define VERIFY_REQUEST(_x) (void) talloc_get_type_abort(_x, REQUEST)
#else
#define VERIFY_REQUEST(_x)
#endif

struct request {
#ifndef NDEBUG
	uint32_t		magic; 		//!< Magic number used to
						//!< detect memory corruption,
						//!< or request structs that
						//!< have not been properly
						//!< initialised.
#endif
	RADIUS_PACKET		*packet;	//!< Incoming request.
#ifdef WITH_PROXY
	RADIUS_PACKET		*proxy;		//!< Outgoing request.
#endif
	RADIUS_PACKET		*reply;		//!< Outgoing response.
#ifdef WITH_PROXY
	RADIUS_PACKET		*proxy_reply;	//!< Incoming response.
#endif
	VALUE_PAIR		*config_items;	//!< VALUE_PAIR s used to set
						//!< per request parameters for
						//!< modules and the server
						//!< core at runtime.
	VALUE_PAIR		*username;	//!< Cached username VALUE_PAIR.
	VALUE_PAIR		*password;	//!< Cached password VALUE_PAIR.

	fr_request_process_t	process;	//!< The function to call to
						//!< move the request through
						//!< the state machine.

	RAD_REQUEST_FUNP	handle;		//!< The function to call to
						//!< move the request through
						//!< the various server
						//!< configuration sections.

	struct main_config_t	*root;		//!< Pointer to the main config
						//!< hack to try and deal with
						//!< hup.

	request_data_t		*data;		//!< Request metadata.

	RADCLIENT		*client;	//!< The client that originally
						//!< sent us the request.

#ifdef HAVE_PTHREAD_H
	pthread_t    		child_pid;	//!< Current thread handling
						//!< the request.
#endif
	time_t			timestamp;	//!< When the request was
						//!< received.
	unsigned int	       	number; 	//!< Monotonically increasing
						//!< request number. Reset on
						//!< server restart.

	rad_listen_t		*listener;	//!< The listener that received
						//!< the request.
#ifdef WITH_PROXY
	rad_listen_t		*proxy_listener;//!< Listener for outgoing
						//!< requests.
#endif


	int		     simul_max;	//!< Maximum number of
						//!< concurrent sessions for
						//!< this user.
#ifdef WITH_SESSION_MGMT
	int		     simul_count;	//!< The current number of
						//!< sessions for this user.
	int		     simul_mpp; 	//!< WEIRD: 1 is false,
						//!< 2 is true.
#endif

	log_debug_t		options;	//!< Request options, currently
						//!< just holds the debug level
						//!< for the request.

	char const		*module;	//!< Module the request is
						//!< currently being processed
						//!< by.
	char const		*component; 	//!< Section the request is
						//!< in.

	int			delay;

	int			master_state;
	int			child_state;
	RAD_LISTEN_TYPE		priority;

	int			timer_action;
	fr_event_t		*ev;

	int			in_request_hash;
#ifdef WITH_PROXY
	int			in_proxy_hash;

	home_server	       	*home_server;
	home_pool_t		*home_pool; /* for dynamic failover */

	struct timeval		proxy_retransmit;

	int			num_proxied_requests;
	int			num_proxied_responses;
#endif

	char const		*server;
	REQUEST			*parent;
	radlog_func_t		radlog;		//!< Function to call to output
						//!< log messages about this
						//!< request.
#ifdef WITH_COA
	REQUEST			*coa;		//!< CoA request originated
						//!< by this request.
	int			num_coa_requests;//!< Counter for number of
						//!< requests sent including
						//!< retransmits.
#endif
};				/* REQUEST typedef */

#define RAD_REQUEST_OPTION_NONE	    (0)
#define RAD_REQUEST_OPTION_DEBUG	   (1)
#define RAD_REQUEST_OPTION_DEBUG2	  (2)
#define RAD_REQUEST_OPTION_DEBUG3	  (3)
#define RAD_REQUEST_OPTION_DEBUG4	  (4)

#define REQUEST_ACTIVE 		(1)
#define REQUEST_STOP_PROCESSING (2)
#define REQUEST_COUNTED		(3)

#define REQUEST_QUEUED		(1)
#define REQUEST_RUNNING		(2)
#define REQUEST_PROXIED		(3)
#define REQUEST_REJECT_DELAY	(4)
#define REQUEST_CLEANUP_DELAY	(5)
#define REQUEST_DONE		(6)

typedef struct radclient_list RADCLIENT_LIST;

typedef int (*rad_listen_recv_t)(rad_listen_t *);
typedef int (*rad_listen_send_t)(rad_listen_t *, REQUEST *);
typedef int (*rad_listen_print_t)(rad_listen_t const *, char *, size_t);
typedef int (*rad_listen_encode_t)(rad_listen_t *, REQUEST *);
typedef int (*rad_listen_decode_t)(rad_listen_t *, REQUEST *);

struct rad_listen_t {
	struct rad_listen_t *next; /* should be rbtree stuff */

	/*
	 *	For normal sockets.
	 */
	RAD_LISTEN_TYPE	type;
	int		fd;
	char const	*server;
	int		status;
#ifdef WITH_TCP
	int		count;
#endif
	int		nodup;
	int		synchronous;
	int		workers;

#ifdef WITH_TLS
	fr_tls_server_conf_t *tls;
#endif

	rad_listen_recv_t recv;
	rad_listen_send_t send;
	rad_listen_encode_t encode;
	rad_listen_decode_t decode;
	rad_listen_print_t print;

	CONF_SECTION const *cs;
	void		*data;

#ifdef WITH_STATS
	fr_stats_t	stats;
#endif
};

/*
 *	This shouldn't really be exposed...
 */
typedef struct listen_socket_t {
	/*
	 *	For normal sockets.
	 */
	fr_ipaddr_t	my_ipaddr;
	int		my_port;

	char const	*interface;
#ifdef SO_BROADCAST
	int		broadcast;
#endif
	time_t		rate_time;
	int		rate_pps_old;
	int		rate_pps_now;
	int		max_rate;

	/* for outgoing sockets */
	home_server	*home;
	fr_ipaddr_t	other_ipaddr;
	int		other_port;

	int		proto;

#ifdef WITH_TCP
  	/* for a proxy connecting to home servers */
	time_t		last_packet;
	time_t		opened;
	fr_event_t	*ev;

	fr_socket_limit_t limit;

	struct listen_socket_t *parent;
	RADCLIENT	*client;

	RADIUS_PACKET   *packet; /* for reading partial packets */
#endif

#ifdef WITH_TLS
	tls_session_t	*ssn;
	REQUEST		*request; /* horrible hacks */
	VALUE_PAIR	*certs;
	pthread_mutex_t mutex;
	uint8_t		*data;
#endif

	RADCLIENT_LIST	*clients;
} listen_socket_t;

#define RAD_LISTEN_STATUS_INIT       (0)
#define RAD_LISTEN_STATUS_KNOWN      (1)
#define RAD_LISTEN_STATUS_EOL 	     (2)
#define RAD_LISTEN_STATUS_REMOVE_NOW (3)

typedef struct main_config_t {
	struct main_config *next;
	fr_ipaddr_t	myip;	/* from the command-line only */
	int		port;	/* from the command-line only */
	int		log_auth;
	int		log_auth_badpass;
	int		log_auth_goodpass;
	int		allow_core_dumps;
	int		debug_level;
#ifdef WITH_PROXY
	int		proxy_requests;
#endif
	int		reject_delay;
	int		status_server;
	int		max_request_time;
	int		cleanup_delay;
	int		max_requests;
#ifdef DELETE_BLOCKED_REQUESTS
	int		kill_unresponsive_children;
#endif
	char		*log_file;
	char const	*dictionary_dir;
	char		*checkrad;
	char const      *pid_file;
	rad_listen_t	*listen;
	int		syslog_facility;
	CONF_SECTION	*config;
	char const	*name;
	char const	*auth_badpass_msg;
	char const	*auth_goodpass_msg;
	int		colourise;	//!< Messages output to stderr and
					//!< stdout may be formatted using
					//!< VT100 escape sequences.
	int		debug_memory;
} MAIN_CONFIG_T;

#define SECONDS_PER_DAY		86400
#define MAX_REQUEST_TIME	30
#define CLEANUP_DELAY		5
#define MAX_REQUESTS		256
#define RETRY_DELAY	     5
#define RETRY_COUNT	     3
#define DEAD_TIME	       120

/* for paircompare_register */
typedef int (*RAD_COMPARE_FUNC)(void *instance, REQUEST *,VALUE_PAIR *, VALUE_PAIR *, VALUE_PAIR *, VALUE_PAIR **);

typedef enum request_fail {
	REQUEST_FAIL_UNKNOWN = 0,
	REQUEST_FAIL_NO_THREADS,	//!< No threads to handle it.
	REQUEST_FAIL_DECODE,		//!< Rad_decode didn't like it.
	REQUEST_FAIL_PROXY,		//!< Call to proxy modules failed.
	REQUEST_FAIL_PROXY_SEND,	//!< Proxy_send didn't like it.
	REQUEST_FAIL_NO_RESPONSE,	//!< We weren't told to respond,
					//!< so we reject.
	REQUEST_FAIL_HOME_SERVER,	//!< The home server didn't respond.
	REQUEST_FAIL_HOME_SERVER2,	//!< Another case of the above.
	REQUEST_FAIL_HOME_SERVER3,	//!< Another case of the above.
	REQUEST_FAIL_NORMAL_REJECT,	//!< Authentication failure.
	REQUEST_FAIL_SERVER_TIMEOUT	//!< The server took too long to
					//!< process the request.
} request_fail_t;

/*
 *	Global variables.
 *
 *	We really shouldn't have this many.
 */
extern char const	*progname;
extern log_debug_t	debug_flag;
extern char const	*radacct_dir;
extern char const	*radlog_dir;
extern char const	*radlib_dir;
extern char const	*radius_dir;
extern char const	*radius_libdir;
extern uint32_t		expiration_seconds;
extern int		log_stripped_names;
extern int		log_auth_detail;
extern char const	*radiusd_version;
void			radius_signal_self(int flag);

#define RADIUS_SIGNAL_SELF_NONE		(0)
#define RADIUS_SIGNAL_SELF_HUP		(1 << 0)
#define RADIUS_SIGNAL_SELF_TERM		(1 << 1)
#define RADIUS_SIGNAL_SELF_EXIT		(1 << 2)
#define RADIUS_SIGNAL_SELF_DETAIL	(1 << 3)
#define RADIUS_SIGNAL_SELF_NEW_FD	(1 << 4)
#define RADIUS_SIGNAL_SELF_MAX		(1 << 5)

/*
 *	Function prototypes.
 */

/* acct.c */
int		rad_accounting(REQUEST *);

/* session.c */
int		rad_check_ts(uint32_t nasaddr, unsigned int port, char const *user,
			     char const *sessionid);
int		session_zap(REQUEST *request, uint32_t nasaddr,
			    unsigned int port, char const *user,
			    char const *sessionid, uint32_t cliaddr,
			    char proto,int session_time);

/* radiusd.c */
#undef debug_pair
void		debug_pair(VALUE_PAIR *);
void		debug_pair_list(VALUE_PAIR *);
void 		rdebug_pair_list(int, REQUEST *, VALUE_PAIR *);
int		log_err (char *);

/* util.c */
#define MEM(x) if (!(x)) { ERROR("Out of memory"); exit(1); }
void (*reset_signal(int signo, void (*func)(int)))(int);
void		request_free(REQUEST **request);
int			request_opaque_free(REQUEST *request);
int		rad_mkdir(char *directory, mode_t mode);
int		rad_checkfilename(char const *filename);
int		rad_file_exists(char const *filename);
void		*rad_malloc(size_t size); /* calls exit(1) on error! */
void		*rad_calloc(size_t size); /* calls exit(1) on error! */
void		rad_const_free(void const *ptr);
REQUEST		*request_alloc(TALLOC_CTX *ctx);
REQUEST		*request_alloc_fake(REQUEST *oldreq);
REQUEST		*request_alloc_coa(REQUEST *request);
int		request_data_add(REQUEST *request,
				 void *unique_ptr, int unique_int,
				 void *opaque, bool free_opaque);
void		*request_data_get(REQUEST *request,
				  void *unique_ptr, int unique_int);
void		*request_data_reference(REQUEST *request,
				  void *unique_ptr, int unique_int);
int		rad_copy_string(char *dst, char const *src);
int		rad_copy_string_bare(char *dst, char const *src);
int		rad_copy_variable(char *dst, char const *from);
int		rad_pps(int *past, int *present, time_t *then,
			struct timeval *now);
int		rad_expand_xlat(REQUEST *request, char const *cmd,
				int max_argc, char *argv[], bool can_fail,
				size_t argv_buflen, char *argv_buf);
void		rad_regcapture(REQUEST *request, int compare, char const *value,
			       regmatch_t rxmatch[]);

/* client.c */
RADCLIENT_LIST	*clients_init(CONF_SECTION *cs);
void		clients_free(RADCLIENT_LIST *clients);
RADCLIENT_LIST	*clients_parse_section(CONF_SECTION *section);
void		client_free(RADCLIENT *client);
int		client_add(RADCLIENT_LIST *clients, RADCLIENT *client);
#ifdef WITH_DYNAMIC_CLIENTS
void		client_delete(RADCLIENT_LIST *clients, RADCLIENT *client);
RADCLIENT	*client_from_query(TALLOC_CTX *ctx, char const *identifier, char const *secret, char const *shortname,
				   char const *type, char const *server, bool require_ma);
RADCLIENT	*client_from_request(RADCLIENT_LIST *clients, REQUEST *request);
#endif
RADCLIENT	*client_find(RADCLIENT_LIST const *clients,
			     fr_ipaddr_t const *ipaddr, int proto);

RADCLIENT	*client_findbynumber(RADCLIENT_LIST const *clients,
				     int number);
RADCLIENT	*client_find_old(fr_ipaddr_t const *ipaddr);
bool		client_validate(RADCLIENT_LIST *clients, RADCLIENT *master, RADCLIENT *c);
RADCLIENT	*client_read(char const *filename, int in_server, int flag);


/* files.c */
int		pairlist_read(TALLOC_CTX *ctx, char const *file, PAIR_LIST **list, int complain);
void		pairlist_free(PAIR_LIST **);

/* version.c */
int 		ssl_check_version(void);
char const	*ssl_version(void);
void		version(void);

/* auth.c */
char	*auth_name(char *buf, size_t buflen, REQUEST *request, int do_cli);
int		rad_authenticate (REQUEST *);
int		rad_postauth(REQUEST *);
int		rad_virtual_server(REQUEST *);

/* exec.c */
pid_t radius_start_program(char const *cmd, REQUEST *request,
			int exec_wait,
			int *input_fd,
			int *output_fd,
			VALUE_PAIR *input_pairs,
			int shell_escape);
int radius_readfrom_program(REQUEST *request, int fd, pid_t pid, int timeout,
			    char *answer, int left);
int radius_exec_program(REQUEST *request, char const *cmd, bool exec_wait, bool shell_escape,
			char *user_msg, size_t msg_len,
			VALUE_PAIR *input_pairs, VALUE_PAIR **output_pairs);
void exec_trigger(REQUEST *request, CONF_SECTION *cs, char const *name, int quench);

/* valuepair.c */
int paircompare_register(DICT_ATTR const *attribute, DICT_ATTR const *from,
          bool first_only, RAD_COMPARE_FUNC func, void *instance);
void		paircompare_unregister(DICT_ATTR const *attr, RAD_COMPARE_FUNC func);
void		paircompare_unregister_instance(void *instance);
int		paircompare(REQUEST *request, VALUE_PAIR *req_list,
			    VALUE_PAIR *check, VALUE_PAIR **rep_list);
int		radius_xlat_do(REQUEST *request, VALUE_PAIR *vp);
void		radius_xlat_move(REQUEST *, VALUE_PAIR **to, VALUE_PAIR **from);
int radius_compare_vps(REQUEST *request, VALUE_PAIR *check, VALUE_PAIR *vp);
int radius_callback_compare(REQUEST *request, VALUE_PAIR *req,
			    VALUE_PAIR *check, VALUE_PAIR *check_pairs,
			    VALUE_PAIR **reply_pairs);
int radius_find_compare(DICT_ATTR const *attribute);
VALUE_PAIR	*radius_paircreate(REQUEST *request, VALUE_PAIR **vps,
				   unsigned int attribute, unsigned int vendor);
void module_failure_msg(REQUEST *request, char const *fmt, ...)
#ifdef __GNUC__
		__attribute__ ((format (printf, 2, 3)))
#endif
;

/*
 *	Less code == less bugs
 */
#define pairmake_packet(_a, _b, _c) pairmake(request->packet, &request->packet->vps, _a, _b, _c)
#define pairmake_reply(_a, _b, _c) pairmake(request->reply, &request->reply->vps, _a, _b, _c)
#define pairmake_config(_a, _b, _c) pairmake(request, &request->config_items, _a, _b, _c)


/* xlat.c */
typedef size_t (*RADIUS_ESCAPE_STRING)(REQUEST *, char *out, size_t outlen, char const *in, void *arg);

ssize_t radius_xlat(char *out, size_t outlen, REQUEST *request, char const *fmt, RADIUS_ESCAPE_STRING escape,
		    void *escape_ctx);

ssize_t radius_axlat(char **out, REQUEST *request, char const *fmt, RADIUS_ESCAPE_STRING escape,
		    	  void *escape_ctx);

typedef ssize_t (*RAD_XLAT_FUNC)(void *instance, REQUEST *, char const *, char *, size_t);
int		xlat_register(char const *module, RAD_XLAT_FUNC func, RADIUS_ESCAPE_STRING escape,
			      void *instance);
void		xlat_unregister(char const *module, RAD_XLAT_FUNC func,
				void *instance);
void		xlat_free(void);

/* threads.c */
extern		int thread_pool_init(CONF_SECTION *cs, int *spawn_flag);
extern		void thread_pool_stop(void);
extern		int thread_pool_addrequest(REQUEST *, RAD_REQUEST_FUNP);
extern		pid_t rad_fork(void);
extern		pid_t rad_waitpid(pid_t pid, int *status);
extern	  int total_active_threads(void);
extern	  void thread_pool_lock(void);
extern	  void thread_pool_unlock(void);
extern		void thread_pool_queue_stats(int array[RAD_LISTEN_MAX], int pps[2]);

#ifndef HAVE_PTHREAD_H
#define rad_fork(n) fork()
#define rad_waitpid(a,b) waitpid(a,b, 0)
#endif

/* mainconfig.c */
/* Define a global config structure */
extern struct main_config_t mainconfig;

int read_mainconfig(int reload);
int free_mainconfig(void);
void hup_mainconfig(void);
void hup_logfile(void);
void fr_suid_down(void);
void fr_suid_up(void);
void fr_suid_down_permanent(void);

/* listen.c */
void listen_free(rad_listen_t **head);
int listen_init(CONF_SECTION *cs, rad_listen_t **head, int spawn_flag);
rad_listen_t *proxy_new_listener(home_server *home, int src_port);
RADCLIENT *client_listener_find(rad_listen_t *listener,
				fr_ipaddr_t const *ipaddr, int src_port);

#ifdef WITH_STATS
RADCLIENT_LIST *listener_find_client_list(fr_ipaddr_t const *ipaddr,
					  int port);
#endif
rad_listen_t *listener_find_byipaddr(fr_ipaddr_t const *ipaddr, int port,
				     int proto);
int rad_status_server(REQUEST *request);

/* event.c */
int radius_event_init(CONF_SECTION *cs, int spawn_flag);
void radius_event_free(void);
int radius_event_process(void);
int event_new_fd(rad_listen_t *listener);
void revive_home_server(void *ctx);
void mark_home_server_dead(home_server *home, struct timeval *when);

/* evaluate.c */
typedef struct fr_cond_t fr_cond_t;
typedef int (*radius_tmpl_getvalue_t)(VALUE_PAIR **out, REQUEST *request, value_pair_map_t const *map, void *ctx);

int radius_evaluate_tmpl(REQUEST *request, int modreturn, int depth,
			 value_pair_tmpl_t const *vpt);
int radius_evaluate_map(REQUEST *request, int modreturn, int depth,
			fr_cond_t const *c);
int radius_evaluate_cond(REQUEST *request, int modreturn, int depth,
			 fr_cond_t const *c);
void radius_pairmove(REQUEST *request, VALUE_PAIR **to, VALUE_PAIR *from);

VALUE_PAIR **radius_list(REQUEST *request, pair_lists_t list);
pair_lists_t radius_list_name(char const **name, pair_lists_t unknown);
int radius_request(REQUEST **request, request_refs_t name);
request_refs_t radius_request_name(char const **name, request_refs_t unknown);

int radius_mapexec(VALUE_PAIR **out, REQUEST *request, value_pair_map_t const *map);
int radius_map2vp(VALUE_PAIR **out, REQUEST *request, value_pair_map_t const *map, void *ctx);
int radius_map2request(REQUEST *request, value_pair_map_t const *map,
		       char const *src, radius_tmpl_getvalue_t func, void *ctx);

int radius_str2vp(REQUEST *request, char const *str, request_refs_t request_def, pair_lists_t list_def);
VALUE_PAIR *radius_vpt_get_vp(REQUEST *request, value_pair_tmpl_t const *vpt);
int radius_get_vp(REQUEST *request, char const *name, VALUE_PAIR **vp_p);

#ifdef WITH_TLS
/*
 *	For run-time patching of which function handles which socket.
 */
int dual_tls_recv(rad_listen_t *listener);
int dual_tls_send(rad_listen_t *listener, REQUEST *request);
int proxy_tls_recv(rad_listen_t *listener);
int proxy_tls_send(rad_listen_t *listener, REQUEST *request);
#endif

#ifdef __cplusplus
}
#endif

#endif /*RADIUSD_H*/
