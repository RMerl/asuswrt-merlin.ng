#ifndef REALMS_H
#define REALMS_H

/*
 * realms.h	Structures, prototypes and global variables
 *		for realms
 *
 * Version:	$Id$
 *
 */

RCSIDH(realms_h, "$Id$")

#ifdef __cplusplus
extern "C" {
#endif

#define HOME_TYPE_INVALID (0)
#define HOME_TYPE_AUTH    (1)
#define HOME_TYPE_ACCT    (2)
#ifdef WITH_COA
#define HOME_TYPE_COA     (3)
#endif

#define HOME_PING_CHECK_NONE		(0)
#define HOME_PING_CHECK_STATUS_SERVER	(1)
#define HOME_PING_CHECK_REQUEST		(2)

#define HOME_STATE_ALIVE		(0)
#define HOME_STATE_ZOMBIE		(1)
#define HOME_STATE_IS_DEAD		(2)
#define HOME_STATE_UNKNOWN		(3)

typedef struct fr_socket_limit_t {
	int		max_connections;
	int		num_connections;
	int		max_requests;
	int		num_requests;
	int		lifetime;
	int		idle_timeout;
} fr_socket_limit_t;

typedef struct home_server {
	char const	*name;

	char const	*hostname;
	char const	*server; /* for internal proxying */
	char const	*parent_server;

	fr_ipaddr_t	ipaddr;

	int		port;
	int		type;		/* auth/acct */

	int		proto;
	fr_socket_limit_t limit;

	fr_ipaddr_t	src_ipaddr; /* preferred source IP address */

	char const	*secret;

	fr_event_t	*ev;
	struct timeval	when;

	int		response_window;
	int		max_outstanding; /* don't overload it */
	int		currently_outstanding;

	time_t		last_packet_sent;
	time_t		last_packet_recv;
	struct timeval	revive_time;
	struct timeval	zombie_period_start;
	int		zombie_period; /* unresponsive for T, mark it dead */

	int		state;

	int		ping_check;
	char const	*ping_user_name;
	char const	*ping_user_password;

	int		ping_interval;
	int		num_pings_to_alive;
	int		num_sent_pings;
	int		num_received_pings;
	int		ping_timeout;

	int		revive_interval; /* if it doesn't support pings */
	CONF_SECTION	*cs;
#ifdef WITH_COA
	int			coa_irt;
	int			coa_mrc;
	int			coa_mrt;
	int			coa_mrd;
#endif
#ifdef WITH_TLS
	fr_tls_server_conf_t	*tls;
#endif

#ifdef WITH_STATS
	int		number;

	fr_stats_t	stats;

	fr_stats_ema_t  ema;
#endif
} home_server;


typedef enum home_pool_type_t {
	HOME_POOL_INVALID = 0,
	HOME_POOL_LOAD_BALANCE,
	HOME_POOL_FAIL_OVER,
	HOME_POOL_CLIENT_BALANCE,
	HOME_POOL_CLIENT_PORT_BALANCE,
	HOME_POOL_KEYED_BALANCE
} home_pool_type_t;


typedef struct home_pool_t {
	char const		*name;
	home_pool_type_t	type;

	int			server_type;
	CONF_SECTION		*cs;

	char const		*virtual_server; /* for pre/post-proxy */

	home_server		*fallback;
	int			in_fallback;
	time_t			time_all_dead;

	int			num_home_servers;
	home_server		*servers[1];
} home_pool_t;


typedef struct _realm {
	char const		*name;

	int			striprealm;

	home_pool_t		*auth_pool;
	home_pool_t		*acct_pool;
#ifdef WITH_COA
	home_pool_t		*coa_pool;
#endif
} REALM;

int realms_init(CONF_SECTION *config);
void realms_free(void);
REALM *realm_find(char const *name); /* name is from a packet */
REALM *realm_find2(char const *name); /* ... with name taken from realm_find */

void home_server_update_request(home_server *home, REQUEST *request);
home_server *home_server_ldb(char const *realmname, home_pool_t *pool, REQUEST *request);
home_server *home_server_find(fr_ipaddr_t *ipaddr, int port, int proto);
#ifdef WITH_COA
home_server *home_server_byname(char const *name, int type);
#endif
#ifdef WITH_STATS
home_server *home_server_bynumber(int number);
#endif
home_pool_t *home_pool_byname(char const *name, int type);

#ifdef __cplusplus
}
#endif

#endif /* REALMS_H */
