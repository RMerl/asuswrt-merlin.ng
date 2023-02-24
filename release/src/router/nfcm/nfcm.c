#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <bcmnvram.h>
#include <shared.h>

#include <ev.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <sys/types.h>
#include <string.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack_tcp.h>
#include <internal/internal.h>

#include "log.h"
#include "nfsw.h"
#include "nfcm.h"
#include "nfct.h"
#include "nfjs.h"
#include "nfcfg.h"

#if defined(HND_ROUTER_AX_6756)
#include "nfbr.h"
#endif

#if defined(CODB)
#include "cosql_utils.h"
#endif

#if defined(SQL)
    #include "nfsql.h"
#endif

#include "nfarp.h"
#if defined(QCA)
    #include "nffdb.h"
#elif defined(HND)
    #include "nfmc.h"
#else
    #include "nfrob.h"
#endif

struct u32_mask {
    uint32_t value;
    uint32_t mask;
};

#define DEBUG_DUMP_CONNTRACK 0 
#define ONE_K (1024)
#define ONE_M (1048576)  // ONE_K*ONE_K

//#define MAX_DB_SIZE    (8) //(8*ONE_M)
#define DAY_SEC        (86400)

#define NF_APP_DAY 1
#define NF_SUM_DAY 7

#if 1
#define MAX_DB_SIZE    (2) //(4*ONE_M)
#define NF_APP_DB_TIMER (10.0)   // 1mins to vacuum sqlite3 nfcm_app_db
#define NF_SUM_DB_TIMER (60.0)   // 10mins to vacuum sqlite3 nfcm_sum_db
#define NF_CONNTRACK_TIMER (10)
#define NF_CONNTRACK_PERIOD (6)
#define NF_PHY_LAYER_TIMER (5)
#define NF_CLIENT_LIST_TIMER (10.0)
#define NF_BRIDGE_TIMER (10.0)   // for ET12
#define NF_MAIN_LOOP_TIMER (5)
#else
#define MAX_DB_SIZE    (8) //(8*ONE_M)
#define NF_APP_DB_TIMER (10.0) //(60.0)   // 1mins to vacuum sqlite3 nfcm_app_db
#define NF_SUM_DB_TIMER (60.0) //(600.0)   // 10mins to vacuum sqlite3 nfcm_sum_db
#define NF_CONNTRACK_TIMER (10)
#define NF_CONNTRACK_PERIOD (6)
#define NF_PHY_LAYER_TIMER (5)
#define NF_CLIENT_LIST_TIMER (10.0)
#define NF_BRIDGE_TIMER (10.0)   // for ET12
#define NF_MAIN_LOOP_TIMER (5)
#endif

int nf_main_loop_timer = NF_MAIN_LOOP_TIMER;

//========================================================================//
#if defined(SQL)
#define NFCM_DB_FOLDER "/jffs"

//------------------------------------------------------------------------//
char nf_app_db_file[128] = "/jffs/nfcm_app.db";
int nf_app_vacuum_flag = 0;
int nf_app_db_timer = NF_APP_DB_TIMER;
time_t nf_app_db_timestamp = 0;     // first timestamp in nfcm_app table

#define NF_APP_DB_VACUUM_DEL 1800  // 30 mins to delete for nfcm_app_db
time_t nf_app_db_vacuum_del = NF_APP_DB_VACUUM_DEL;

#define NF_APP_DB_VACUUM_BUF 600   // 10 mins buffer
time_t nf_app_db_vacuum_buf = NF_APP_DB_VACUUM_BUF;

//------------------------------------------------------------------------//
char nf_sum_db_file[128] = "/jffs/nfcm_sum.db";
int nf_sum_vacuum_flag = 0;
int nf_sum_db_timer = NF_SUM_DB_TIMER;
time_t nf_sum_db_timestamp = 0;     // first timestamp in nfcm_sum table
time_t nf_sum_db_write_ts;

#define NF_SUM_DB_SIZE 4
uint32_t nf_sum_db_size = NF_SUM_DB_SIZE;

//------------------------------------------------------------------------//
char nf_tcp_db_file[128] = "/jffs/nfcm_tcp.db";

time_t nf_tcp_db_timestamp = 0;     // first timestamp in nfcm_tcp table

#endif // defined(SQL)
//========================================================================//

int nf_phy_layer_flag = 0;
int nf_phy_layer_timer = NF_PHY_LAYER_TIMER;

#if defined(AIMESH)
int nf_client_list_flag = 0;
int nf_client_list_timer = NF_CLIENT_LIST_TIMER;
#endif

#if defined(HND_ROUTER_AX_6756)
int nf_bridge_flag = 0;
int nf_bridge_timer = NF_BRIDGE_TIMER;
#endif

int nf_conntrack_flag = 0;
int nf_conntrack_timer = NF_CONNTRACK_TIMER;
int nf_conntrack_period = NF_CONNTRACK_PERIOD;

#define NFCT_PID_FILE "/var/run/nfcm.pid"
#define NFCT_ACCT_FILE "/proc/sys/net/netfilter/nf_conntrack_acct"

/* These are the template objects that are used to send commands. */
static struct {
    struct nf_conntrack *ct;
    struct nf_expect *exp;
    /* Expectations require the expectation tuple and the mask. */
    struct nf_conntrack *exptuple, *mask;

    /* Allows filtering/setting specific bits in the ctmark */
    struct u32_mask mark;

    /* Allow to filter by mark from kernel-space. */
    struct nfct_filter_dump_mark filter_mark_kernel;
    bool filter_mark_kernel_set;

    /* Allows filtering by ctlabels */
    struct nfct_bitmask *label;

    /* Allows setting/removing specific ctlabels */
    struct nfct_bitmask *label_modify;
} tmpl;

static int alloc_tmpl_objects(void)
{
    tmpl.ct = nfct_new();
    tmpl.exptuple = nfct_new();
    tmpl.mask = nfct_new();
    tmpl.exp = nfexp_new();

    memset(&tmpl.mark, 0, sizeof(tmpl.mark));

    return tmpl.ct != NULL && tmpl.exptuple != NULL &&
           tmpl.mask != NULL && tmpl.exp != NULL;
}

static void free_tmpl_objects(void)
{
    if (tmpl.ct) nfct_destroy(tmpl.ct);
    if (tmpl.exptuple) nfct_destroy(tmpl.exptuple);
    if (tmpl.mask) nfct_destroy(tmpl.mask);
    if (tmpl.exp) nfexp_destroy(tmpl.exp);
    if (tmpl.label) nfct_bitmask_destroy(tmpl.label);
    if (tmpl.label_modify) nfct_bitmask_destroy(tmpl.label_modify);
}

union ct_address {
    uint32_t v4;
    uint32_t v6[4];
};

static struct option original_opts[] = {
    { "dump", 2, 0, 'L' },
    { 0, 0, 0, 0 }
};

static const int famdir2attr[2][2] = {
    { ATTR_ORIG_IPV4_SRC, ATTR_ORIG_IPV4_DST },
    { ATTR_ORIG_IPV6_SRC, ATTR_ORIG_IPV6_DST }
};

enum {
    _O_XML	= (1 << 0),
    _O_EXT	= (1 << 1),
    _O_TMS	= (1 << 2),
    _O_ID	= (1 << 3),
    _O_KTMS	= (1 << 4),
    _O_CL	= (1 << 5),
};

static struct nfct_handle *cth;
struct nfct_filter_dump *filter_dump;
static struct option *opts = original_opts;
static unsigned int global_option_offset = 0;
static unsigned int options;

/* The below macros handle endian mis-matches between wl utility and wl driver. */
bool g_swap = false;

LIST_HEAD(iplist);
LIST_HEAD(bklist); // backup previous iplist

LIST_HEAD(smlist); // summary list from iplist

LIST_HEAD(arlist);
LIST_HEAD(swlist);

LIST_HEAD(clilist); // for AiMesh parsed from /tmp/clientlist.json


#if defined(NFCM_COLLECT_INVALID_TCP)
LIST_HEAD(tcplist); // TCP list 
LIST_HEAD(tbklist); // broken TCP list
#endif

#if defined(SQL)
sqlite3 *appdb = NULL;
sqlite3 *sumdb = NULL;
sqlite3 *tcpdb = NULL;
#endif

#if defined(CODB)
int nf_app_db_dbg = 0;
int nf_sum_db_dbg = 0;
int nf_tcp_db_dbg = 0;
#endif

void get_date_time(int utc, char *buff, int len)
{
    time_t rawtime;
    struct tm *timeinfo;

    if(utc) {
        time(&rawtime);
        timeinfo = gmtime(&rawtime);
        strftime(buff, len, "%Y-%m-%d %H:%M:%S", timeinfo);
    } else {
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(buff, len, "%Y-%m-%d %H:%M:%S", timeinfo);
    }
}

static bool nf_write_pid_file(char *fname)
{
    char pidstr[32];

    FILE *fp = fopen(fname, "w+");
    if (!fp)  return false;

    sprintf(pidstr, "%d", getpid());
    fputs(pidstr, fp);
    fclose(fp);

    return true;
}

static bool nf_set_ct_acct_flags(char *fname)
{
    char cmd[64];

    sprintf(cmd, "echo 1 > %s", fname);
    //info("%s", cmd);
    system(cmd);

    return true;
}

LIST_HEAD(lanlist);
lan_info_t* lan_info_new()
{
    lan_info_t *li = (lan_info_t *)calloc(1, sizeof(lan_info_t));
    if (!li) return NULL;

    INIT_LIST_HEAD(&li->list);

    return li;

}

void lan_info_free(lan_info_t *li)
{
    if (li) free(li);

    return;
}

void lan_info_list_dump(struct list_head *list)
{
    lan_info_t *li;
    char ipstr[INET6_ADDRSTRLEN];
    //inet_ntop(AF_INET, src, ipstr, INET_ADDRSTRLEN);

    printf("%s:\n", __FUNCTION__);
    list_for_each_entry(li, list, list) {
        printf("ifname=\t%s\n", li->ifname);
        printf("enable=\t%s\n", li->enabled ? "true" : "false");
        inet_ntop(AF_INET, &li->addr, ipstr, INET_ADDRSTRLEN);
        printf("addr=\t%s\n", ipstr);
        inet_ntop(AF_INET, &li->subnet, ipstr, INET_ADDRSTRLEN);
        printf("subnet=\t%s\n", ipstr);
    }
}

char* mac2str(const unsigned char *e, char *a)
{
    sprintf(a, "%02x:%02x:%02x:%02x:%02x:%02x", e[0], e[1], e[2], e[3], e[4], e[5]);
    return a;
}

char *str2mac(const unsigned char *a, char *e)
{
	sprintf(e, "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c",
			a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9], a[10], a[11]);
	return e;
}

int get_eth_type(char *ethtype, PHY_TYPE etype)
{
    switch (etype) {
    case PHY_ETHER:
        strcpy(ethtype, "EPHY");
        break;
    case PHY_WIRELESS:
        strcpy(ethtype, "WPHY");
        break;
    default:
        strcpy(ethtype, "UNKNOWN");
        break;
    }

    return 0;
}

bool is_in_lanv4(struct in_addr *src)
{
    lan_info_t *li;

    //char ipstr[INET6_ADDRSTRLEN];
    //inet_ntop(AF_INET, src, ipstr, INET_ADDRSTRLEN);

    list_for_each_entry(li, &lanlist, list) {
        if ((src->s_addr & li->subnet.s_addr) == (li->addr.s_addr & li->subnet.s_addr)) {
            return true;
        }
    }

    return false;
}

bool is_in_lanv6(struct in6_addr *src)
{

    struct in6_addr prefix;
    int prefix_len = nvram_get_int("ipv6_prefix_length"); 
    int tail_bits;
    //passthrough don't have ipv6_prefix_length, ipv6_prefix, ipv6_rtr_addr, don't care lan
    if(!strcmp("ipv6pt", nvram_get("ipv6_service"))) {
       return true;
    }

    if(prefix_len < 1 || prefix_len > 127) {
       nf_printf("wrong prefix lenght %d \n ", prefix_len); 
       return false;
    } 
    if(!inet_pton(AF_INET6, nvram_get("ipv6_prefix"), &prefix)) {
   
       nf_printf("can't convert ipv6 prefix into v6 addr - %s \n ", nvram_get("ipv6_prefix")); 
      return false;
    }
   // nf_printf("is_in_lavn6 prefix_length %d\n", prefix_len);
    for(int i = 0; i< prefix_len/8; i++)
    {    
        nf_printf("is_in_lanv6 %d %2x<->%2x\n", i, src->s6_addr[i], prefix.s6_addr[i]);
        if (src->s6_addr[i] != prefix.s6_addr[i]) {
            return false;
        }
    }
    tail_bits = prefix_len % 8;
    if ( tail_bits > 0 &&  src->s6_addr[prefix_len] >> (8 - tail_bits) != prefix.s6_addr[prefix_len] >> (8 - tail_bits) ) {
            return false;
    }

    nf_printf("is_in_lanv6 return true\n");
    return true;
}

bool is_local_link_addr(struct in6_addr *addr)
{

    if (addr->s6_addr[0] == 0xfe && (addr->s6_addr[1] & 0x80) == 0x80) 
            return true;
    return false;
}

bool is_router_v6addr(struct in6_addr *addr)
{
    struct in6_addr router;
    nf_printf("router addr: %s\n", nvram_get("ipv6_rtr_addr"));
    if(!strcmp("ipv6pt", nvram_get("ipv6_service")))
            return false;
    if(!inet_pton(AF_INET6, nvram_get("ipv6_rtr_addr"), &router)) 
            return false;
    if (!memcmp(addr, &router, sizeof(struct in6_addr)))
            return true;

    return false;
}

bool is_router_addr(struct in_addr *addr)
{
    lan_info_t *li;

    //char ipstr[INET6_ADDRSTRLEN];
    //inet_ntop(AF_INET, src, ipstr, INET_ADDRSTRLEN);

    list_for_each_entry(li, &lanlist, list) {
        if (addr->s_addr == li->addr.s_addr) {
            return true;
        }
    }

    return false;
}

bool is_broadcast_addr(struct in_addr *addr)
{
    lan_info_t *li;

    //char ipstr[INET6_ADDRSTRLEN];
    //inet_ntop(AF_INET, src, ipstr, INET_ADDRSTRLEN);

    list_for_each_entry(li, &lanlist, list) {
        if (addr->s_addr == li->subnet.s_addr) {
            return true;
        }
    }

    return false;
}

bool is_multi_addr(struct in_addr *addr)
{
    //in_addr_t stored in network order
    uint32_t address = ntohl(addr->s_addr);

    return (address & 0xF0000000) == 0xE0000000;
}


bool is_acceptable_addr(nf_node_t *nn)
{

 if(nn->isv4) {
    if (!is_in_lanv4(&nn->srcv4))
        return false;

    if (is_broadcast_addr(&nn->dstv4))
        return false;

    if (is_router_addr(&nn->srcv4))
        return false;

    if (is_multi_addr(&nn->dstv4))
        return false;

    if (nn->proto == IPPROTO_UDP) {
        if (nn->dst_port == 53)
            return false;
    }

    return true;
  } else {
    
    if (!is_in_lanv6(&nn->srcv6))
    {   
     return false;
    }
    if (is_local_link_addr(&nn->srcv6))
        return false;

    if (is_router_v6addr(&nn->srcv6))
        return false;
    
    if (nn->proto == IPPROTO_UDP) {
        if (nn->dst_port == 53)
            return false;
    }
    return true;
  }
}

bool get_wan_addr(char *ifname, struct in_addr *wan_addr, struct in_addr *wan_mask)
{
    int fd;
    struct ifreq ifr;
    //struct sockaddr_in *sin_ptr;
    struct in_addr addr, mask;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;

    /* I want IP address attached to ifname */
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);


    /* get ip address of my interface */
    if(ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
        perror("ioctl SIOCGIFADDR error");
        addr.s_addr = 0;
    } else {
        //sin_ptr = (struct sockaddr_in *)&ifr.ifr_addr;
        //addr = sin_ptr->sin_addr;
        memcpy(&addr, &sin_addr(&ifr.ifr_addr), sizeof(struct in_addr));
    }
#if 1
    //ioctl(fd, SIOCGIFADDR, &ifr);
    printf("addr: %s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
    printf("ADDR: %s\n", inet_ntoa(sin_addr(&ifr.ifr_addr)));
#endif

    close(fd);
/*
    typedef uint32_t in_addr_t;

    struct in_addr {
        in_addr_t s_addr;
    };
*/
    fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;

    /* I want IP address attached to ifname */
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);

    /* get network mask of my interface */
    if (ioctl(fd, SIOCGIFNETMASK, &ifr) < 0) {
        perror("ioctl SIOCGIFNETMASK error");
        mask.s_addr = 0;
    } else {
        //sin_ptr = (struct sockaddr_in *)&ifr.ifr_addr;
        memcpy(&mask, &sin_addr(&ifr.ifr_addr), sizeof(struct in_addr));
    }
#if 1
    //ioctl(fd, SIOCGIFNETMASK, &ifr);
    printf("mask: %s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
    printf("MASK: %s\n", inet_ntoa(sin_addr(&ifr.ifr_addr)));
#endif

    close(fd);

    /* display result */
    printf("addr: [%s], mask: [%s]\n", inet_ntoa(addr), inet_ntoa(mask));

    memcpy(wan_addr, &addr, sizeof(struct in_addr));
    memcpy(wan_mask, &mask, sizeof(struct in_addr));
    printf("ADDR: [%s], MASK: [%s]\n", inet_ntoa(*wan_addr), inet_ntoa(*wan_mask));


    return 0;
}

int lan_info_init(struct list_head *list)
{
    lan_info_t *li;
    char word[64], *next;
    char *b, *nv, *nvp;
    char *ifname, *laninfo;
    char *addr, *mask;
    char *rulelist;

    li = lan_info_new();
    list_add_tail(&li->list, list);

    strcpy(li->ifname, "br0");
    li->enabled = true;
    inet_pton(AF_INET, nvram_get("lan_ipaddr"), &li->addr);
    inet_pton(AF_INET, nvram_get("lan_netmask"), &li->subnet);

    // get wireless guest network status
    if (!nvram_get_int("wgn_enabled"))
        goto out_f;

    rulelist = nvram_get("wgn_brif_rulelist");
    if (!rulelist)
        goto out_f;

    nv = nvp = strdup(rulelist);
    if (!nv)
        goto out_free;

    while ((b = strsep(&nvp, "<")) != NULL) {
        if (!b || !strlen(b)) continue;
        //fields = vstrsep(b, ">", &ifname, &laninfo);
        vstrsep(b, ">", &ifname, &laninfo);

        addr = strtok(laninfo, "/");
        mask = strtok(NULL, "/");

        li = lan_info_new();
        list_add_tail(&li->list, list);

        strcpy(li->ifname, ifname);
        inet_pton(AF_INET, addr, &li->addr); // lan_ipaddr
        li->subnet.s_addr = htonl(0xFFFFFFFF << (32 - atoi(mask)));
    }

    foreach(word, nvram_get("wgn_ifnames"), next) {
        list_for_each_entry(li, list, list) {
            if (!strcmp(word, li->ifname)) {
                li->enabled = true;
                break;
            }
        }
    }

out_free:
    free(nv);

out_f:

#if defined(NFCMDBG)
    lan_info_list_dump(list);
#endif

    return 0;
}

void sig_str(int signum, char *sigstr, size_t sigstr_sz)
{
    char sig_default[16];
    char *sig;

    switch (signum) {
    case SIGHUP:
        sig = "SIGHUP";      break;
    case SIGINT:
        sig = "SIGINT";      break;
    case SIGQUIT:
        sig = "SIGQUIT";     break;
    case SIGILL:
        sig = "SIGILL";      break;
    case SIGTRAP:
        sig = "SIGTRAP";     break;
    case SIGABRT:
        sig = "SIGABRT";     break;
    case SIGBUS:
        sig = "SIGBUS";      break;
    case SIGFPE:
        sig = "SIGFPE";      break;
    case SIGKILL:
        sig = "SIGKILL";     break;
    case SIGUSR1:
        sig = "SIGUSR1";     break;
    case SIGSEGV:
        sig = "SIGSEGV";     break;
    case SIGUSR2:
        sig = "SIGUSR2";     break;
    case SIGPIPE:
        sig = "SIGPIPE";     break;
    case SIGALRM:
        sig = "SIGALRM";     break;
    case SIGTERM:
        sig = "SIGTERM";     break;
    case SIGCHLD:
        sig = "SIGCHLD";     break;
    case SIGCONT:
        sig = "SIGCONT";     break;
    case SIGSTOP:
        sig = "SIGSTOP";     break;
    case SIGTSTP:
        sig = "SIGTSTP";     break;
    case SIGTTIN:
        sig = "SIGTTIN";     break;
    case SIGTTOU:
        sig = "SIGTTOU";     break;
    case SIGURG:
        sig = "SIGURG";      break;
    case SIGXCPU:
        sig = "SIGXCPU";     break;
    case SIGXFSZ:
        sig = "SIGXFSZ";     break;
    case SIGVTALRM:
        sig = "SIGVTALRM";   break;
    case SIGPROF:
        sig = "SIGPROF";     break;
    case SIGWINCH:
        sig = "SIGWINCH";    break;
    case SIGIO:
        sig = "SIGIO";       break;
    case SIGPWR:
        sig = "SIGPWR";      break;
    case SIGSYS:
        sig = "SIGSYS";      break;

    default:
        snprintf(sig_default, sizeof(sig_default), "%d", signum);
        sig = sig_default;
        break;
    }

    snprintf(sigstr, sigstr_sz, "%s (%s)", sig, strsignal(signum));
}

static void free_options(void)
{
    if (opts != original_opts) {
        free(opts);
        opts = original_opts;
        global_option_offset = 0;
    }
}

void __attribute__((noreturn))
exit_error(enum exittype status, const char *msg, ...)
{
    va_list args;

    free_options();
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");
    va_end(args);

    /* release template objects that were allocated in the setup stage. */
    free_tmpl_objects();
    exit(status);
}

void nf_list_free(struct list_head *list)
{
    nf_node_t * nn,*nnt;

    list_for_each_entry_safe(nn, nnt, list, list) {
        list_del(&nn->list);
        nf_node_free(nn);
    }
    return;
}

void nf_list_tcp_free(struct list_head *list)
{
    nf_node_t * nn,*nnt;

    list_for_each_entry_safe(nn, nnt, list, list) {
        if (nn->state >= TCP_CONNTRACK_ESTABLISHED && nn->time_in > 100) {
            list_del(&nn->list);
            nf_node_free(nn);
        } else {

        }
    }
    return;
}

void nf_node_dump(nf_node_t *nn)
{
    char ipstr[INET6_ADDRSTRLEN];
    char ethtype[10];

    if (nn->isv4) {
         if (!is_acceptable_addr(nn)) {
            return;
         }
        /*
        if (!is_in_lanv4(&nn->srcv4)) return;
        if (is_broadcast_addr(&nn->dstv4)) return;
        if (is_router_addr(&nn->srcv4)) return; 
        */ 
        nf_printf("proto:  %d\n", nn->proto);
        inet_ntop(AF_INET, &nn->srcv4, ipstr, INET_ADDRSTRLEN);
        nf_printf("src:	%s[%u]\n", ipstr, nn->srcv4.s_addr);
        nf_printf("port:   %d\n", nn->src_port);
        nf_printf("src_mac:%s\n", nn->src_mac);
        inet_ntop(AF_INET, &nn->dstv4, ipstr, INET_ADDRSTRLEN);
        nf_printf("dst:	%s[%u]\n", ipstr, nn->dstv4.s_addr);
        nf_printf("port:   %d\n", nn->dst_port);
    } else {
        nf_printf("proto:  %d\n", nn->proto);
        inet_ntop(AF_INET6, &nn->srcv6, ipstr, INET6_ADDRSTRLEN);
        nf_printf("src:	%s\n", ipstr);
        nf_printf("port:   %d\n", nn->src_port);
        nf_printf("src_mac:%s\n", nn->src_mac);
        inet_ntop(AF_INET6, &nn->dstv6, ipstr, INET6_ADDRSTRLEN);
        nf_printf("dst:	%s\n", ipstr);
        nf_printf("port:   %d\n", nn->dst_port);
    }

    if (nn->state) {
        nf_printf("tcp_state :     %u\n", nn->state);
        nf_printf("timestamp :     %ld\n", nn->timestamp);
        nf_printf("time_in   :     %ld\n", nn->time_in);
    } else {
        nf_printf("up_bytes:       %llu\n", nn->up_bytes);
        nf_printf("up_ttl_bytes:   %llu\n", nn->up_ttl_bytes);
        nf_printf("up_dif_bytes:   %llu\n", nn->up_dif_bytes);
        nf_printf("up_speed:       %llu\n", nn->up_speed);

        nf_printf("dn_bytes:       %llu\n", nn->dn_bytes);
        nf_printf("dn_ttl_bytes:   %llu\n", nn->dn_ttl_bytes);
        nf_printf("dn_dif_bytes:   %llu\n", nn->dn_dif_bytes);
        nf_printf("dn_speed:       %llu\n", nn->dn_speed);

        nf_printf("ifname  :       %s\n", nn->layer1_info.ifname);
        nf_printf("phy_type:       %d\n", get_eth_type(ethtype, nn->layer1_info.eth_type));
        nf_printf("is_guest:       %d\n", nn->layer1_info.is_guest);
        nf_printf("phy_port:       %d\n", nn->layer1_info.eth_port);
    }
    nf_printf("-----------------------\n");

    return;
}

int nf_list_size(struct list_head *list)
{
    nf_node_t *nn;
    int cnt = 0;

    list_for_each_entry(nn, list, list) {
        cnt++;
    }

    return cnt;
}

void nf_list_dump(char *title, struct list_head *list)
{
    nf_node_t *nn;

    printf("[%s]%s: %s, count=[%d]\n", __FILE__, __FUNCTION__, title, nf_list_size(list));
    list_for_each_entry(nn, list, list) {
        nf_node_dump(nn);
    }
    printf("=======================\n");

    return;
}

void nf_list_move(struct list_head *dst, struct list_head *src)
{
    nf_node_t * nn,*nnt;

    list_for_each_entry_safe(nn, nnt, src, list) {
        list_move_tail(&nn->list, dst);
    }

}

// compare the nn and nnt node according to five tuple
bool nf_node_compare(bool v4, nf_node_t *nn, nf_node_t *nnt)
{
    if (v4) {
        if (nnt->srcv4.s_addr == nn->srcv4.s_addr &&
            nnt->dstv4.s_addr == nn->dstv4.s_addr &&
            nnt->src_port == nn->src_port &&
            nnt->dst_port == nn->dst_port &&
            nnt->proto == nn->proto)
        {
            return true;
        }
    } else {
        if (!memcmp(&nn->srcv6, &nnt->srcv6, sizeof(struct in6_addr)) &&
            !memcmp(&nn->dstv6, &nnt->dstv6, sizeof(struct in6_addr)) &&
            nnt->src_port == nn->src_port &&
            nnt->dst_port == nn->dst_port &&
            nnt->proto == nn->proto)
        {
            return true;
        }
    }

    return false;
}

void nf_list_speed_calc(struct list_head *iplist, struct list_head *bklist)
{
    nf_node_t *nn,*nnt;
    int64_t diff;
    bool bFound = false;
    //char src_ipstr[INET6_ADDRSTRLEN];
    //char dst_ipstr[INET6_ADDRSTRLEN];

    list_for_each_entry(nn, iplist, list) {
        list_for_each_entry(nnt, bklist, list) {
            if (nnt->isv4 != nn->isv4) continue;
            if (nn->isv4) {
                if (!is_in_lanv4(&nn->srcv4)) continue;
            } else {
            // TODO: add filter for not target ipv6 target (src is in lan)
                if(!is_in_lanv6(&nn->srcv6)) continue; 
            }
            
            if (nf_node_compare(nn->isv4, nn, nnt)) {
                // five tuples are the same
                diff = nn->up_bytes - nnt->up_bytes;
                nn->up_dif_bytes = (diff < 0) ? 0 : diff;
                nn->up_speed = (diff < 0) ? 0 : (int64_t)(nn->up_dif_bytes / (int)nf_conntrack_timer);

                diff = nn->dn_bytes - nnt->dn_bytes;
                nn->dn_dif_bytes = (diff < 0) ? 0 : diff;
                nn->dn_speed = (diff < 0) ? 0 : (int64_t)(nn->dn_dif_bytes / (int)nf_conntrack_timer);

                //inet_ntop(AF_INET, &nn->srcv4, src_ipstr, INET_ADDRSTRLEN);
                //inet_ntop(AF_INET, &nn->dstv4, dst_ipstr, INET_ADDRSTRLEN);
                //printf("src:%s[%d], dst:%s[%d] up_dif=[%lld], up_speed=[%lld], dn_dif=[%lld], dn_speed=[%lld]\n",
                //       src_ipstr, nn->src_port, dst_ipstr, nn->dst_port,
                //       nn->up_dif_bytes, nn->up_speed, nn->dn_dif_bytes, nn->dn_speed);
                bFound = true;
                break;
            }
        } // bklist

        if (!bFound) { // there is not the same node in bklist
            nn->up_dif_bytes = (nn->up_bytes < 0) ? 0 : nn->up_bytes;
            nn->up_speed = (int64_t)(nn->up_dif_bytes / (int)nf_conntrack_timer);

            nn->dn_dif_bytes = (nn->dn_bytes < 0) ? 0 : nn->dn_bytes;
            nn->dn_speed = (int64_t)(nn->dn_dif_bytes / (int)nf_conntrack_timer);
        }
        bFound = false;
    } // iplist

    return;
}

// find the nn in smlist according to src addr
nf_node_t* nf_node_statistics_search(arp_node_t *ar, struct list_head *smlist)
{
    nf_node_t *nn;

    list_for_each_entry(nn, smlist, list) {
        if (nn->isv4) {
            if (nn->srcv4.s_addr == ar->srcv4.s_addr) {
                if (!is_in_lanv4(&nn->srcv4))
                    continue;
                return nn;
            }
        } else {
            
            if (!memcmp(&nn->srcv6, &ar->srcv6, sizeof(struct in6_addr))) {
                if (!is_in_lanv6(&nn->srcv6))
                     continue;
                return nn; 
            }
        }
    }

    return NULL;
}

int nf_list_statistics_calc(struct list_head *arlist, struct list_head *iplist,
                            struct list_head *clilist, struct list_head *smlist)
{
    nf_node_t *nn = NULL;
    nf_node_t *nnt;
    arp_node_t *ar;
    bool b_new = false;

    //nf_list_free(smlist);

    // get the summary from iplist based on arlist
    list_for_each_entry(ar, arlist, list) {
        if (ar->is_v4 && !is_in_lanv4(&ar->srcv4)) continue;
        if (!ar->is_v4 && !is_in_lanv6(&ar->srcv6)) continue;
        nn = nf_node_statistics_search(ar, smlist);
        if (nn == NULL) {
            nn = nf_node_new();
            nn->isv4 = ar->is_v4;
            nn->srcv4.s_addr = ar->srcv4.s_addr;
            memcpy(&nn->srcv6, &ar->srcv6, sizeof(struct in6_addr));
            // ip v6
            if(nn->isv4)
               strcpy(nn->src6_ip, DEFAULT_IPV6_ADDR);
            else
               inet_ntop(AF_INET6, &nn->srcv6, nn->src6_ip, INET6_ADDRSTRLEN);
           
            memcpy(nn->src_mac, ar->mac, ETHER_ADDR_LENGTH);
            list_add_tail(&nn->list, smlist);

            nf_set_layer1_info(nn, clilist, arlist);
            b_new = true;
        }

        //nn->up_speed = nn->dn_speed = 0;
        list_for_each_entry(nnt, iplist, list) {
            if (ar->is_v4) {
                if (nn->srcv4.s_addr == nnt->srcv4.s_addr) {
                    if (b_new) {
                        //nn->up_bytes += nnt->up_bytes;
                        nn->up_ttl_bytes += nnt->up_bytes;
                        //nn->dn_bytes += nnt->dn_bytes;
                        nn->dn_ttl_bytes += nnt->dn_bytes;
                    } else {
                        nn->up_ttl_bytes += nnt->up_dif_bytes;
                        nn->dn_ttl_bytes += nnt->dn_dif_bytes;
                    }
                    nn->up_dif_bytes += nnt->up_dif_bytes;
                    nn->up_speed += nnt->up_speed;

                    nn->dn_dif_bytes += nnt->dn_dif_bytes;
                    nn->dn_speed += nnt->dn_speed;

                    nn->layer1_info.eth_type = nnt->layer1_info.eth_type;
                    nn->layer1_info.eth_port = nnt->layer1_info.eth_port;
                }
            } else {
                if (!memcmp(&ar->srcv6, &nnt->srcv6, sizeof(struct in6_addr))) {
                    if (b_new) {
                        nn->up_ttl_bytes += nnt->up_bytes;
                        nn->dn_ttl_bytes += nnt->dn_bytes;
                    } else {
                        nn->up_ttl_bytes += nnt->up_dif_bytes;
                        nn->dn_ttl_bytes += nnt->dn_dif_bytes;
                    }
                    nn->up_dif_bytes += nnt->up_dif_bytes;
                    nn->up_speed += nnt->up_speed;

                    nn->dn_dif_bytes += nnt->dn_dif_bytes;
                    nn->dn_speed += nnt->dn_speed;

                    nn->layer1_info.eth_type = nnt->layer1_info.eth_type;
                    nn->layer1_info.eth_port = nnt->layer1_info.eth_port;
                }
            }
        }
        b_new = false;
    }

    return 0;
}

uint16_t get_port_from_ct(struct nf_conntrack *ct, int src_or_dst)
{
  struct __nfct_tuple *tuple = &ct->head.orig;
  uint16_t port;
  switch (tuple->protonum) {
    case IPPROTO_TCP:
        port  = src_or_dst == 0 ? tuple->l4src.tcp.port:tuple->l4dst.tcp.port;
        break;
    case IPPROTO_UDP:
        port = src_or_dst == 0 ? tuple->l4src.udp.port:tuple->l4dst.udp.port;
        break;
    case IPPROTO_SCTP:
        port = src_or_dst == 0 ? tuple->l4src.sctp.port:tuple->l4dst.sctp.port;
        break;
    case IPPROTO_DCCP:
        port = src_or_dst == 0 ? tuple->l4src.dccp.port:tuple->l4dst.dccp.port;
        break;
    //case IPPROTO_ICMP:
    //    nn->dst_port = (tuple->l4dst.icmp.type<<8) | tuple->l4dst.icmp.code;
    //    break;
    default:
        port = src_or_dst == 0 ? tuple->l4src.all:tuple->l4dst.all;
        break;
    }

    port = ntohs(port);

    return port;
}

static int nf_conntrack_handle_cb(enum nf_conntrack_msg_type type,
                        struct nf_conntrack *ct,
                        void *data)
{
#if DEBUG_DUMP_CONNTRACK
    char ipstr[INET_ADDRSTRLEN];
    char ipdst[INET_ADDRSTRLEN];
    if (ct->head.orig.l3protonum == AF_INET)
    {
      inet_ntop(AF_INET,&ct->head.orig.src.v4, ipstr, INET_ADDRSTRLEN);
      inet_ntop(AF_INET,&ct->head.orig.dst.v4, ipdst, INET_ADDRSTRLEN);
    
      // rtsp.stream and wowzaec2demo.streamlock.net
      if(!strcmp(ipdst, "34.227.104.115")||!strcmp(ipdst, "23.88.67.97")) 
      {
        printf("id[%lu] to[%lu] status[%lu] src[%s][%u] dst[%s][%u] --[%llu][%llu] --[%llu][%llu]\n", 
	    ct->id, ct->timeout, ct->status,
	    ipstr, get_port_from_ct(ct, 0), 
            ipdst, get_port_from_ct(ct, 1), 
	    ct->counters[__DIR_ORIG].packets,
            ct->counters[__DIR_REPL].packets,
	    ct->counters[__DIR_ORIG].bytes,
            ct->counters[__DIR_REPL].bytes);
      }
    }
#endif 
   
    nf_conntrack_process(ct, &iplist, &clilist, &arlist);

#if defined(NFCM_COLLECT_INVALID_TCP)
    // collect invalid tcp conntrack
    nf_conntrack_tcp_process(ct, &tcplist);
#endif
    return NFCT_CB_CONTINUE;
}

#if defined(NFCM_COLLECT_INVALID_TCP)
// all tcp node have the same protonum, and we also discard the src_port comparation
bool nf_node_tcp_compare(nf_node_t *nn, nf_node_t *nnt)
{
    if (nn->isv4) {
        if (nnt->srcv4.s_addr == nn->srcv4.s_addr &&
            nnt->dstv4.s_addr == nn->dstv4.s_addr &&
            nnt->dst_port == nn->dst_port)
        {
            return true;
        }
    } else {
        if (!memcmp(&nn->srcv6, &nnt->srcv6, sizeof(struct in6_addr)) &&
            !memcmp(&nn->dstv6, &nnt->dstv6, sizeof(struct in6_addr)) &&
            nnt->dst_port == nn->dst_port)
        {
            return true;
        }
    }

    return false;
}

nf_node_t* nf_list_tcp_search(nf_node_t *nn, struct list_head *llist)
{
    nf_node_t *bk;

    list_for_each_entry(bk, llist, list) {
        if (nf_node_tcp_compare(nn, bk))
            return bk;
    }

    return NULL;
}

int nf_list_tcp_collect(struct list_head *tbklist,
                        struct list_head *tcplist)

{
    nf_node_t *nn, *nnt;
    nf_node_t *bk;
    time_t diff;

    // clean the tbklist nodes which are not in tcplist
    list_for_each_entry_safe(bk, nnt, tbklist, list) {
        nn = nf_list_tcp_search(bk, tcplist);
        if (!nn) { // 
            list_del(&bk->list);
            nf_node_free(bk);
        } // tbklist
    } // tcplist

    // get the tcplist node which is not in tbklist
    // update the tcplist nodes which is already in tbklist
    list_for_each_entry(nn, tcplist, list) {
        bk = nf_list_tcp_search(nn, tbklist);
        if (!bk) {
            bk = nf_node_new();
            list_add_tail(&bk->list, tbklist);

            bk->isv4 = nn->isv4;
            bk->proto = IPPROTO_TCP;
            bk->srcv4.s_addr = nn->srcv4.s_addr;
            memcpy(&bk->srcv6, &nn->srcv6, sizeof(struct in6_addr));
            // ip v6
            if(bk->isv4)
               strcpy(bk->src6_ip, DEFAULT_IPV6_ADDR);
            else
	       inet_ntop(AF_INET6, &bk->srcv6, bk->src6_ip, INET6_ADDRSTRLEN);

            bk->dstv4.s_addr = nn->dstv4.s_addr;
            memcpy(&bk->dstv6, &nn->dstv6, sizeof(struct in6_addr));
            // ip v6
            if(bk->isv4)
               strcpy(bk->dst6_ip, DEFAULT_IPV6_ADDR);
            else
	       inet_ntop(AF_INET6, &bk->dstv6, bk->dst6_ip, INET6_ADDRSTRLEN);
	    
	    bk->dst_port = nn->dst_port;
	    bk->src_port = nn->src_port;
            

            memcpy(bk->src_mac, nn->src_mac, ETHER_ADDR_LENGTH);

            bk->state = nn->state;
            bk->timestamp = nn->timestamp;
            bk->time_in = nn->time_in;
        } else {
            diff = time(NULL) - bk->timestamp;
            bk->time_in = (diff > bk->time_in) ? diff : bk->time_in;
        }
    }

    return 0;
}
#endif

// this is callback to parse conntrack
void func_conntrack()
{
    int lock; // file lock
    time_t ts = time(NULL);

    nf_list_free(&bklist);
    nf_list_move(&bklist, &iplist);

#if defined(NFCM_COLLECT_INVALID_TCP)
    nf_list_free(&tcplist);
    //nf_list_move(&tbklist, &tcplist);
#endif
    // every nfct_query's entry will call nf_conntrack_handle_cb
    nfct_query(cth, NFCT_Q_DUMP_FILTER, filter_dump);

    nf_list_speed_calc(&iplist, &bklist);

    nf_list_statistics_calc(&arlist, &iplist, &clilist, &smlist);

#if defined(NFCM_COLLECT_INVALID_TCP)
    nf_list_tcp_collect(&tbklist, &tcplist);
#endif

#if defined(NFCMDBG)
    nf_list_dump("smlist", &smlist);
    nf_list_dump("iplist", &iplist);

#if defined(NFCM_COLLECT_INVALID_TCP)
    nf_list_dump("tbklist", &tbklist);
#endif

#endif // #if defined(NFCMDBG)

#if defined(SQL)
    // nfcm_app.db is wrote per nf_conntrack_timer
    lock = file_lock("nfcm_app");
    sqlite_app_insert(appdb, &iplist);
    file_unlock(lock);

    // nfcm_sum.db is wrote per nf_conntrack_timer * nf_conntrack_period
    if (ts - nf_sum_db_write_ts >= nf_conntrack_timer * nf_conntrack_period) {
        lock = file_lock("nfcm_sum");
        sqlite_sum_insert(sumdb, &smlist);
        file_unlock(lock);
        nf_sum_db_write_ts = ts;
        nf_list_free(&smlist);
    }

#if defined(NFCM_COLLECT_INVALID_TCP)
    // nfcm_tcp.db is wrote per nf_conntrack_timer
    lock = file_lock("nfcm_tcp");
    sqlite_tcp_insert(tcpdb, &tbklist);
    file_unlock(lock);
#endif

#endif

    nf_list_to_json(&iplist, &arlist);
    nf_list_statistics_to_json(&smlist, &arlist);

#if defined(NFCM_COLLECT_INVALID_TCP)
    nf_list_tcp_to_json(&tbklist);
#endif
    return;
}

void timer_conntrack(struct ev_loop *loop, ev_timer *w, int e)
{
#if 1
    nf_conntrack_flag = 1;
#else
    int lock; // file lock

    nf_list_free(&bklist);
    nf_list_move(&bklist, &iplist);

    // every nfct_query's entry will call ct_handle_cb
    nfct_query(cth, NFCT_Q_DUMP_FILTER, filter_dump);

    nf_list_speed_calc(&iplist, &bklist);
    nf_list_statistics_calc(&arlist, &iplist, &smlist);

#if defined(NFCMDBG)
    nf_list_dump("smlist", &smlist);
    nf_list_dump("iplist", &iplist);
#endif // #if defined(NFCMDBG)

#if defined(SQL)
    lock = file_lock("nfcm_app");
    sqlite_app_insert(appdb, &iplist);
    file_unlock(lock);

    lock = file_lock("nfcm_sum");
    sqlite_sum_insert(sumdb, &smlist);
    file_unlock(lock);
#endif

    nf_list_to_json(&iplist, &arlist);
    nf_list_statistics_to_json(&smlist, &arlist);

    //w->repeat = TIMER_DURATION;
    ev_timer_again(loop, w);
#endif
}

int ev_timer_conntrack(struct ev_loop *loop, ev_timer *timer)
{
    // initialize a timer watcher, then start it
    // simple non-repeating TIMER_DURATION second timeout
    ev_init(timer, timer_conntrack);
    timer->repeat = nf_conntrack_timer;
    ev_timer_again(loop, timer);

    return 0;
}

// this is callback to parse fdb(QCA), rob(non-HND), switch-mib(HND), 'arp -n' result
void func_phy_layer()
{
#if defined(SUPPORT_SWITCH_DUMP)
    sw_list_free(&swlist);
    // use ioctl(brcm)/netlink(QCA) to get MAC layer information
    sw_list_parse(&swlist);
#endif

    arp_list_free(&arlist);
    // parse the "cat /proc/net/arp" result
    arp_list_parse(&arlist, &swlist);

    return;
}

void timer_phy_layer(struct ev_loop *loop, ev_timer *w, int e)
{
#if 1
    nf_phy_layer_flag = 1;
#else
    sw_list_free(&swlist);

    // use ioctl(brcm)/netlink(QCA) to get MAC layer information
    sw_list_parse(&swlist);

    arp_list_free(&arlist);

    // parse the "cat /proc/net/arp" result
    arp_list_parse(&arlist, &swlist);

    //w->repeat = TIMER_DURATION;
    ev_timer_again(loop, w);
#endif
}

int ev_timer_phy_layer(struct ev_loop *loop, ev_timer *timer)
{
    // initialize a timer watcher, then start it
    // simple non-repeating TIMER_DURATION second timeout
    ev_init(timer, timer_phy_layer);
    timer->repeat = nf_phy_layer_timer;
    ev_timer_again(loop, timer);

    return 0;
}

#if defined(HND_ROUTER_AX_6756)
// use 'brctl showmacs br0' and 'brctl showstp br0' to get MAC table
void func_bridge()
{
    br_cmd_showmacs(1, "br0");
    br_cmd_showstp(1, "br0");

    return;
}

void timer_bridge(struct ev_loop *loop, ev_timer *w, int e)
{
#if 1
    nf_bridge_flag = 1;
#else
    sw_list_free(&swlist);

    // use ioctl(brcm)/netlink(QCA) to get MAC layer information
    sw_list_parse(&swlist);

    arp_list_free(&arlist);

    // parse the "cat /proc/net/arp" result
    arp_list_parse(&arlist, &swlist);

    //w->repeat = TIMER_DURATION;
    ev_timer_again(loop, w);
#endif
}

int ev_timer_bridge(struct ev_loop *loop, ev_timer *timer)
{
    // initialize a timer watcher, then start it
    // simple non-repeating TIMER_DURATION second timeout
    ev_init(timer, timer_bridge);
    timer->repeat = nf_bridge_timer;
    ev_timer_again(loop, timer);

    return 0;
}
#endif //HND_ROUTER_AX_6756

#if defined(AIMESH)
// get AiMesh info from /tmp/clientlist.json
void func_client_list()
{
    cli_list_free(&clilist);

    cli_list_file_parse(&clilist);

    return;
}

void timer_client_list(struct ev_loop *loop, ev_timer *w, int e)
{
    nf_client_list_flag = 1;
}

int ev_timer_client_list(struct ev_loop *loop, ev_timer *timer)
{
    // initialize a timer watcher, then start it
    // simple non-repeating TIMER_DURATION second timeout
    ev_init(timer, timer_client_list);
    timer->repeat = nf_client_list_timer;
    ev_timer_again(loop, timer);

    return 0;
}

PHY_TYPE client_list_get_phy_type(char *ptype)
{
    if (!strcasecmp(ptype, "wired_mac")) {
        return PHY_ETHER;
    } else if(!strcasecmp(ptype, "2G") ||
              !strcasecmp(ptype, "5G")) 
    {
        return PHY_WIRELESS;
    } 
    return PHY_UNKNOWN;
}

int client_list_get_phy_port(char *ptype)
{
    if (!strcasecmp(ptype, "wired_mac")) {
        return 0;
    } else if(!strcasecmp(ptype, "2G")) {
        return 2;
    } else if(!strcasecmp(ptype, "5G")) {
        return 5;
    } 
    return -1;
}
#endif // AIMESH

void signal_action(struct ev_loop *loop, ev_signal *w, int e)
{
    char sigstr[64];
    int var;

    sig_str(w->signum, sigstr, sizeof(sigstr));
    warn("Signal %s received.", sigstr);

    switch (w->signum) {
    case SIGUSR1:
#if defined(SUPPORT_SWITCH_DUMP)
        sw_list_free(&swlist);
        // use ioctl to get switch layer information
        sw_list_parse(&swlist);
#endif

        arp_list_free(&arlist);
        // parse the "cat /proc/net/arp" result
        arp_list_parse(&arlist, &swlist);
        break;

    case SIGUSR2:
#if defined(CODB)
        var = nvram_get_int("nf_app_db_dbg");
        if (var != nf_app_db_dbg) {
            nf_app_db_dbg = var;
            cosql_enable_debug(appdb, nf_app_db_dbg);
        }

        var = nvram_get_int("nf_sum_db_dbg");
        if (var != nf_sum_db_dbg) {
            nf_sum_db_dbg = var;
            cosql_enable_debug(sumdb, nf_sum_db_dbg);
        }

        var = nvram_get_int("nf_tcp_db_dbg");
        if (var != nf_tcp_db_dbg) {
            nf_tcp_db_dbg = var;
            cosql_enable_debug(tcpdb, nf_tcp_db_dbg);
        }
        break;
#endif
    case SIGINT:
        ev_signal_stop(loop, w);
        ev_break(loop, EVBREAK_ALL);
        break;

    default:
        /* At this point the handler for this signal was reset
           (except for SIGUSR2) due to the SA_RESETHAND flag;
           so re-send the signal to ourselves in order to properly crash */
        raise(w->signum);
    }

}

int ev_signal_sigint(struct ev_loop *loop, ev_signal *signaler)
{
    ev_signal_init(signaler, signal_action, SIGINT);
    ev_signal_start(loop, signaler);

    return 0;
}

int ev_signal_sigusr1(struct ev_loop *loop, ev_signal *signaler)
{
    ev_signal_init(signaler, signal_action, SIGUSR1);
    ev_signal_start(loop, signaler);

    return 0;
}

int ev_signal_sigusr2(struct ev_loop *loop, ev_signal *signaler)
{
    ev_signal_init(signaler, signal_action, SIGUSR2);
    ev_signal_start(loop, signaler);

    return 0;
}

#if defined(SQL)
/*
 *  check filesize is over or not, size is Mbytes
 *  if over size, return 1, else return 0
 */
int check_file_size_over(char *fname, long int size)
{
    struct stat st;
    long max_size = size * ONE_M;
    int is_oversize = false;

    stat(fname, &st);

    is_oversize = (st.st_size >= max_size) ? 1 : 0;
    //printf("================================================\n"
    //   "[%s]%s: leave.. file[%s] file_size=[%ld], size=[%d], is_oversize=[%d]\n",
    //      __FILE__, __FUNCTION__, fname, st.st_size, max_size, is_oversize);

    return is_oversize;
}

/*
 *  get last month's timestamp
 *  ex.
 *  now = 1445817600
 *  tm  = 2015/10/26 00:00:00
 *  t   = 2015/10/01 00:00:00
 *  t_t = 1443628800
 */
time_t get_last_month_timestamp()
{
    struct tm local, t;
    time_t now, t_t = 0;

    // get timestamp and tm
    time(&now);
    localtime_r(&now, &local);

    // copy t from local
    t.tm_year = local.tm_year;
    t.tm_mon = local.tm_mon;
    t.tm_mday = 1;
    t.tm_hour = 0;
    t.tm_min = 0;
    t.tm_sec = 0;

    // transfer tm to timestamp
    t_t = mktime(&t);

    return t_t;
}

void func_nfcm_app_db_vacuum()
{
#if !defined(CODB)
    int ret;
    char *err = NULL;
    char sql[1024];
#endif
    time_t timestamp_now, timestamp_buf, timestamp_del;
    int lock; // file lock

    // step1. get the timestamp
    timestamp_now = time(NULL);
    timestamp_del = timestamp_now - nf_app_db_vacuum_del;
    timestamp_buf = timestamp_del - nf_app_db_vacuum_buf;

    if (nf_app_db_timestamp == 0) {
        sqlite_get_timestamp(appdb, &nf_app_db_timestamp);
    }

    // timeline is following:
    // timestamp_now --> nf_app_db_vacuum_del <-- timestamp_del --> nf_app_db_vacuum_buf <-- timestamp_buf | nf_app_db_timestamp

    if ( timestamp_buf < nf_app_db_timestamp) // don't need to delete
        return;

    lock = file_lock("nfcm_app");

    /* if *-journal exists, remove it !! */
    sqlite_remove_journal(nf_app_db_file);

    /* integrity check */
#if defined(CODB)
    if (cosql_integrity_check(appdb) == COSQL_ERROR) {
        eval("rm", "-f", nf_app_db_file);
        //info("%s: remove broken database : %s\n", __FUNCTION__, nf_app_db_file);
        file_unlock(lock);
        return;
    }

    // delete rows which timestamp < timestamp_del
    if (cosql_remove_data_between_column_value(appdb, "timestamp", 0, timestamp_del) == COSQL_ERROR) {
        file_unlock(lock);
        return;
    }
#else
    if (sqlite_integrity_check(appdb, nf_app_db_file) == 0) {
        file_unlock(lock);
        return;
    }

    // step2. execute sql to delete nfcm_app
    snprintf(sql, sizeof(sql), "DELETE from nfcm_app WHERE timestamp < %ld;", timestamp_del);
    //info("sql=[%s]\n", sql);
    ret = sqlite3_exec(appdb, sql,  NULL, NULL, &err);
    if (ret != SQLITE_OK && err != NULL) {
        printf("SQL error: %s\n", err);
        sqlite3_free(err);
        file_unlock(lock);
        return;
    }

    // step3. compact file
    //info("sql=[%s]\n", "VACUUM;");
    ret = sqlite3_exec(appdb, "VACUUM;",  NULL, NULL, &err);
    if (ret != SQLITE_OK && err != NULL) {
        printf("SQL error: %s\n", err);
        sqlite3_free(err);
        file_unlock(lock);
        return;
    }
#endif

    nf_app_db_timestamp = timestamp_del;

    file_unlock(lock);

    return;
}

void timer_nfcm_app_db_vacuum()
{
    nf_app_vacuum_flag = 1;
}

int ev_timer_nfcm_app_db(struct ev_loop *loop, ev_timer *timer)
{
    // initialize a timer watcher, then start it
    // simple non-repeating TIMER_DURATION second timeout
    ev_init(timer, timer_nfcm_app_db_vacuum);
    timer->repeat = nf_app_db_timer; //TIMER_NFCM_APP_DB;
    ev_timer_again(loop, timer);

    return 0;
}

//static int max_db_size = MAX_DB_SIZE;
void func_nfcm_sum_db_vacuum()
{
    int ret;
    char *err = NULL;
    char sql[1024];
    time_t timestamp;
    int compact;
    int lock; // file lock

    compact = check_file_size_over(nf_sum_db_file, nf_sum_db_size);
    if (compact) {
        // step1. get timestamp
        timestamp = time(NULL) - NF_SUM_DAY * DAY_SEC;
    } else
        return;

    lock = file_lock("nfcm_sum");

    /* if *-journal exists, remove it !! */
    sqlite_remove_journal(nf_sum_db_file);

    /* integrity check */
    if (sqlite_integrity_check(sumdb, nf_sum_db_file) == 0) {
        file_unlock(lock);
        return;
    }

    //while (compact) 
    {
        // step2. execute sql to delete nfcm_sum
        snprintf(sql, sizeof(sql), "DELETE from nfcm_sum WHERE timestamp < %ld;", timestamp);
        //info("sql=[%s]\n", sql);
        ret = sqlite3_exec(sumdb, sql,  NULL, NULL, &err);
        if (ret != SQLITE_OK && err != NULL) {
            printf("SQL error: %s\n", err);
            sqlite3_free(err);
            file_unlock(lock);
            return;
        }

        // step3. compact file
        //info("sql=[%s]\n", "VACUUM;");
        ret = sqlite3_exec(sumdb, "VACUUM;",  NULL, NULL, &err);
        if (ret != SQLITE_OK && err != NULL) {
            printf("SQL error: %s\n", err);
            sqlite3_free(err);
            file_unlock(lock);
            return;
        }
        //compact = check_file_size_over(NFCM_SUM_DB_FILE, MAX_DB_SIZE);
    }
    file_unlock(lock);

    return;
}

void timer_nfcm_sum_db_vacuum()
{
    nf_sum_vacuum_flag = 1;
}

int ev_timer_nfcm_sum_db(struct ev_loop *loop, ev_timer *timer)
{
    // initialize a timer watcher, then start it
    // simple non-repeating TIMER_DURATION second timeout
    ev_init(timer, timer_nfcm_sum_db_vacuum);
    timer->repeat = nf_sum_db_timer; //TIMER_NFCM_SUM_DB;
    ev_timer_again(loop, timer);

    return 0;
}
#endif // defined(SQL)

void timer_main_loop(struct ev_loop *loop, ev_timer *w, int e)
{
#if defined(AIMESH)
    // get the AiMesh info from /tmp/clientlist.json
    if (nf_client_list_flag) {
        func_client_list();
        nf_client_list_flag = 0;
    }
#endif //AIMESH

    // get the switch layer mac table
    if (nf_phy_layer_flag) {
        func_phy_layer();
        nf_phy_layer_flag = 0;
    }

#if defined(HND_ROUTER_AX_6756)
    // get the switch layer mac table
    if (nf_bridge_flag) {
        func_bridge();
        nf_bridge_flag = 0;
    }

#endif

    // get the conntrack from kernel
    if (nf_conntrack_flag) {
        func_conntrack();
        nf_conntrack_flag = 0;
    }

#if defined(SQL)
    // check the size of nfcm_app.db and try compact it
    if (nf_app_vacuum_flag) {
        func_nfcm_app_db_vacuum();
        nf_app_vacuum_flag = 0;
    }

    // check the sizeof nfcm_sum.db and try compact it
    if (nf_sum_vacuum_flag) {
        func_nfcm_sum_db_vacuum();
        nf_sum_vacuum_flag = 0;
    }
#endif // defined(SQL)

    return;
}

int ev_timer_main_loop(struct ev_loop *loop, ev_timer *timer)
{
    ev_init(timer, timer_main_loop);
    timer->repeat = nf_main_loop_timer;
    ev_timer_again(loop, timer);

    return 0;
}

bool nfcm_config_init()
{
    int32_t var;

    var = nvram_get_int("nf_main_loop_timer");
    nf_main_loop_timer = (var > 0) ? var : NF_MAIN_LOOP_TIMER;

    var = nvram_get_int("nf_conntrack_timer");
    nf_conntrack_timer = (var > 0) ? var : NF_CONNTRACK_TIMER;

    var = nvram_get_int("nf_conntrack_period");
    nf_conntrack_period = (var > 0) ? var : NF_CONNTRACK_PERIOD;

    nf_sum_db_write_ts = time(NULL);

    var = nvram_get_int("nf_phy_layer_timer");
    nf_phy_layer_timer = (var > 0) ? var : NF_PHY_LAYER_TIMER;

    var = nvram_get_int("nf_app_db_vacuum_del");
    nf_app_db_vacuum_del = (var > 0) ? var : NF_APP_DB_VACUUM_DEL;

    var = nvram_get_int("nf_app_db_vacuum_buf");
    nf_app_db_vacuum_buf = (var > 0) ? var : NF_APP_DB_VACUUM_BUF;

    sprintf(nf_app_db_file, "%s/nfcm_app.db", nvram_get("nf_db_folder") ? : NFCM_DB_FOLDER);
    sprintf(nf_sum_db_file, "%s/nfcm_sum.db", nvram_get("nf_db_folder") ? : NFCM_DB_FOLDER);
    sprintf(nf_tcp_db_file, "%s/nfcm_tcp.db", nvram_get("nf_db_folder") ? : NFCM_DB_FOLDER);

#if defined(CODB)
    nf_app_db_dbg = nvram_get_int("nf_app_db_dbg");
    nf_sum_db_dbg = nvram_get_int("nf_sum_db_dbg");
    nf_tcp_db_dbg = nvram_get_int("nf_tcp_db_dbg");
#endif

    var = nvram_get_int("nf_sum_db_size");
    nf_sum_db_size = (var > 0) ? var : NF_SUM_DB_SIZE;

    return true;
}

int nfcm_delete_db_file()
{
    // if fname is existed, delete it
    if(!access(nf_app_db_file, F_OK))
    	unlink(nf_app_db_file);

    if(!access(JSON_OUTPUT_APP_FILE, F_OK))
        unlink(JSON_OUTPUT_APP_FILE);

    if(!access(nf_sum_db_file, F_OK))
        unlink(nf_sum_db_file);

    if(!access(JSON_OUTPUT_SUM_FILE, F_OK))
        unlink(JSON_OUTPUT_SUM_FILE);

    if(!access(nf_tcp_db_file, F_OK))
        unlink(nf_tcp_db_file);

    if(!access(JSON_OUTPUT_TCP_FILE, F_OK))
        unlink(JSON_OUTPUT_TCP_FILE);

    return 0;
}

int nfcm_check_lan()
{
    char *lan_ipaddr, *lan_netmask;
    char *nfcm_ipaddr, *nfcm_netmask;

    lan_ipaddr = nvram_safe_get("lan_ipaddr") ? : NULL;
    lan_netmask = nvram_safe_get("lan_netmask") ? : NULL;
    //printf("lan_ipaddr=[%s], lan_netmask=[%s]\n", lan_ipaddr, lan_netmask);

    nfcm_ipaddr = nvram_safe_get("nfcm_ipaddr") ? : NULL;
    nfcm_netmask = nvram_safe_get("nfcm_netmask") ? : NULL;
    //printf("nfcm_ipaddr=[%s], nfcm_netmask=[%s]\n", nfcm_ipaddr, nfcm_netmask);

    if (nfcm_ipaddr == NULL || strcmp(nfcm_ipaddr, lan_ipaddr) ||
        nfcm_netmask == NULL || strcmp(nfcm_netmask, lan_netmask))
    {
        nfcm_delete_db_file();
        nvram_set("nfcm_ipaddr", lan_ipaddr);
        nvram_set("nfcm_netmask", lan_netmask);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int family = AF_UNSPEC;
    struct ev_loop *loop = EV_DEFAULT;
    ev_timer timer_conntrack;
    ev_timer timer_phy_layer;
    ev_timer timer_main_loop;
#if defined(SQL)
    ev_timer timer_nfcm_app_db;
    ev_timer timer_nfcm_sum_db;
#endif
    ev_signal signal_sigint;
    ev_signal signal_sigusr1;
    ev_signal signal_sigusr2;
#if defined(AIMESH)
    ev_timer timer_client_list;
#endif
#if defined(HND_ROUTER_AX_6756)
    ev_timer timer_bridge;
#endif

#if 0
    struct in_addr wan_addr, wan_mask;
    get_wan_addr("eth3", &wan_addr, &wan_mask);
    return 0;
#endif

    // init nfcm configuration variables
    nfcm_config_init();

#if defined(HND_ROUTER_AX_6756)
    // init bridge configuration variables
    br_init();
#endif
    
    // check nfcm start
    nfcm_check_lan();

    // init lan info according to br0/wifi intf
    lan_info_init(&lanlist);

#if defined(CONFIG_ET)
    bcm5301x_age_timeout(10);
#endif

    nf_set_ct_acct_flags(NFCT_ACCT_FILE);
    nf_write_pid_file(NFCT_PID_FILE);

    /* we release these objects in the exit_error() path. */
    if (!alloc_tmpl_objects())
        exit_error(OTHER_PROBLEM, "out of memory");

#if defined(SQL)
    appdb = sqlite_open(NFCM_DB_APP, appdb, nf_app_db_file);
    sumdb = sqlite_open(NFCM_DB_SUM, sumdb, nf_sum_db_file);
    tcpdb = sqlite_open(NFCM_DB_TCP, tcpdb, nf_tcp_db_file);
#if defined(CODB)
    cosql_enable_debug(sumdb, nf_sum_db_dbg);
    cosql_enable_debug(appdb, nf_app_db_dbg);
    cosql_enable_debug(tcpdb, nf_tcp_db_dbg);
#endif // defined(CODB)
#endif

#if defined(SUPPORT_SWITCH_DUMP)
    sw_list_parse(&swlist);
#endif
    arp_list_parse(&arlist, &swlist);

    cth = nfct_open(CONNTRACK, 0);
    if (!cth)
        exit_error(OTHER_PROBLEM, "Can't open handler");

    if (options & CT_COMPARISON && options & CT_OPT_ZERO)
        exit_error(PARAMETER_PROBLEM, "Can't use -z with filtering parameters");

    // tmpl.ct will pass to nf_conntrack_handle_cb as 'void *data'
    nfct_callback_register(cth, NFCT_T_ALL, nf_conntrack_handle_cb, tmpl.ct);

    filter_dump = nfct_filter_dump_create();
    if (filter_dump == NULL)
        exit_error(OTHER_PROBLEM, "OOM");

    if (tmpl.filter_mark_kernel_set) {
        nfct_filter_dump_set_attr(filter_dump, NFCT_FILTER_DUMP_MARK, &tmpl.filter_mark_kernel);
        nfct_filter_dump_set_attr_u8(filter_dump, NFCT_FILTER_DUMP_L3NUM, family);
    }

    nfct_query(cth, NFCT_Q_DUMP_FILTER, filter_dump);

#if defined(NFCMDBG)
    nf_list_dump("iplist", &iplist);
#endif

    nf_list_to_json(&iplist, &arlist);

    //nf_list_free(&iplist); // should remove the line??
    //nf_list_free(&tcplist);

    ev_timer_phy_layer(loop, &timer_phy_layer);
#if defined(AIMESH)
    ev_timer_client_list(loop, &timer_client_list);
#endif
    ev_timer_conntrack(loop, &timer_conntrack);

#if defined(SQL)
    // delete records and vacuum the table
    ev_timer_nfcm_app_db(loop, &timer_nfcm_app_db);
    ev_timer_nfcm_sum_db(loop, &timer_nfcm_sum_db);
#endif

#if defined(HND_ROUTER_AX_6756)
    ev_timer_bridge(loop, &timer_bridge);
#endif

    ev_signal_sigint(loop, &signal_sigint);
    ev_signal_sigusr1(loop, &signal_sigusr1); // re-parse
    ev_signal_sigusr2(loop, &signal_sigusr2);

    sqlite_get_timestamp(appdb, &nf_app_db_timestamp);
    sqlite_get_timestamp(sumdb, &nf_sum_db_timestamp);
    sqlite_get_timestamp(tcpdb, &nf_tcp_db_timestamp);
#if 0
    // get the following tables' first timestamp
    sqlite_db_timestamp_select(appdb, &nf_app_db_timestamp, "select timestamp from nfcm_app order by timestamp asc limit 1;");
    sqlite_db_timestamp_select(sumdb, &nf_sum_db_timestamp, "select timestamp from nfcm_sum order by timestamp asc limit 1;");
    sqlite_db_timestamp_select(tcpdb, &nf_tcp_db_timestamp, "select timestamp from nfcm_tcp order by timestamp asc limit 1;");
#endif
    // work alarmer
    ev_timer_main_loop(loop, &timer_main_loop);

    // main loop
    ev_run(loop, 0);

#if defined(NFCM_COLLECT_INVALID_TCP)
    nf_list_free(&tcplist);
    nf_list_free(&tbklist);
#endif

    nf_list_free(&iplist);
    nf_list_free(&bklist);

    sw_list_free(&swlist);
    arp_list_free(&arlist);

    //nfct_filter_dump_destroy(filter_dump);
    nfct_close(cth);

#if defined(HND_ROUTER_AX_6756)
    br_shutdown();
#endif

#if defined(SQL)
    sqlite_close(appdb);
    sqlite_close(sumdb);
    sqlite_close(tcpdb);
#endif

    free_tmpl_objects();

    return 0;
}
