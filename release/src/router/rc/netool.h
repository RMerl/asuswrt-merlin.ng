#include <sys/time.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <syslog.h>
#include <shared.h>
#include <disk_io_tools.h>
#include <linklist.h>

#define NETOOL_DEBUG                       "/tmp/NETOOL_DEBUG"
#define NETOOL_QUEUE                       "/tmp/NETOOL_QUEUE"
#define NETOOL_SOCKET_PATH                 "/etc/netool_socket"
#define MAX_SOCKET_CLIENT                  5

#define NETOOL_RESULT_DIR                  "/tmp/netool"
#define NETOOL_RESULT_PING_NORMAL_LOG      "PING.log"
#define NETOOL_RESULT_TRACERT_NORMAL_LOG   "TRACEROUTE.log"
#define NETOOL_RESULT_NSLOOKUP_LOG         "NSLOOKUP.log"
#define NETOOL_RESULT_NETSTAT_LOG          "NETSTAT.log"
#define NETOOL_RESULT_NETSTAT_NAT_LOG      "NETSTAT_NAT.log"

/* netstat */
#define NETSTAT_BIT_SHOW_LISTEN_SCOKET         0x1 << 0
#define NETSTAT_BIT_SHOW_ALL_SOCKET            0x1 << 1
#define NETSTAT_BIT_RESOLVE_NAME               0x1 << 2
#define NETSTAT_BIT_SHOW_TCP_SOCKET            0x1 << 3
#define NETSTAT_BIT_SHOW_UDP_SOCKET            0x1 << 4
#define NETSTAT_BIT_SHOW_RAW_SOCKET            0x1 << 5
#define NETSTAT_BIT_SHOW_UNIX_SOCKET           0x1 << 6
#define NETSTAT_BIT_SHOW_ROUTING               0x1 << 7
#define MAX_NETSTAT_OPTIONS_NUM                8

/* netstat-nat */
#define NETSTAT_NAT_BIT_RESOLVE_NAME           0x1 << 0
#define NETSTAT_NAT_BIT_STRIP_HEADER           0x1 << 1
#define NETSTAT_NAT_BIT_EXTEND_HOSTNAME        0x1 << 2
#define NETSTAT_NAT_BIT_SHOW_SNAT              0x1 << 3
#define NETSTAT_NAT_BIT_SHOW_DNAT              0x1 << 4
#define NETSTAT_NAT_BIT_SHOW_ONLY_NAT          0x1 << 5
#define NETSTAT_NAT_BIT_SHOW_ONLY_ROUTED_NAT   0x1 << 6
#define NETSTAT_NAT_BIT_SHOW_ONLY_VAILD_NAT    0x1 << 7
#define MAX_NETSTAT_NAT_OPTIONS_NUM            8

#define BIT(var) NETSTAT_BIT_##var
#define NATBIT(var) NETSTAT_NAT_BIT_##var

struct netstat_t {
	int      bit;
	char    *option;
};

struct netstat_t netstInfo[] =
{
	{BIT(SHOW_LISTEN_SCOKET) , "l"},
	{BIT(SHOW_ALL_SOCKET)    , "a"},
	{BIT(RESOLVE_NAME)       , "n"},
	{BIT(SHOW_TCP_SOCKET)    , "t"},
	{BIT(SHOW_UDP_SOCKET)    , "u"},
	{BIT(SHOW_RAW_SOCKET)    , "w"},
	{BIT(SHOW_UNIX_SOCKET)   , "x"},
	{BIT(SHOW_ROUTING)       , "r"},
	{0,0}
};

struct netstat_t netst_natInfo[] =
{
	{NATBIT(RESOLVE_NAME)           , "n"},
	{NATBIT(STRIP_HEADER)           , "o"},
	{NATBIT(EXTEND_HOSTNAME)        , "x"},
	{NATBIT(SHOW_SNAT)              , "S"},
	{NATBIT(SHOW_DNAT)              , "D"},
	{NATBIT(SHOW_ONLY_NAT)          , "L"},
	{NATBIT(SHOW_ONLY_ROUTED_NAT)   , "R"},
	{NATBIT(SHOW_ONLY_VAILD_NAT)    , "N"},
	{0,0}
};

typedef enum {
	REQ_GET_RESULT = 0,
	REQ_PING_MODE,
	REQ_TRACEROUTE_MODE,
	REQ_PING_NORMAL_MODE,
	REQ_TRACEROUTE_NORMAL_MODE,
	REQ_NSLOOKUP_MODE,
	REQ_NETSTAT_MODE,
	REQ_NETSTAT_NAT_MODE,
	REQ_MODE_TOTAL
}REQ_TYPE;

typedef struct __request_info__t_
{
	int    type;      
	int    ping_cnt;      /* Send only CNT pings */
	int    hops;          /* traceroute max hops */
	int    response;      /* Seconds to wait for the first response */
	int    exec_cnt;      /* Number of executions */
	int    netst;         /* Netstat/Netstat-nat options */
	char   ver[4];        /* IPv4/IPv6 Protocol */
	char   sort[10];      /* Netstat-nat options */
	char   proto[8];      /* Netstat-nat options */
	char   srchost[64];   /* Netstat-nat options */
	char   dsthost[64];   /* Netstat-nat options */
	char   interface[12]; /* Using Interface */
	char   target[128];   /* IP/Domain */
	time_t tstamp;        /* Receive time */
	
}REQUEST_INFO_T;
