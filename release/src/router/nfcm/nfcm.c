#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <bcmnvram.h>
#include <shared.h>

#include <ev.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

#include "log.h"
#include "nfcm.h"
#include "nfct.h"
#include "nfjs.h"

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

#define ONE_K (1024)
#define ONE_M (1048576)  // ONE_K*ONE_K

#define MAX_DB_SIZE    (8*ONE_M)
#define DAY_SEC        (86400)
#define TIMER_NFCM_APP_DB (60.0)   // 1mins to vacuum sqlite3 nfcm_app_db
#define TIMER_NFCM_SUM_DB (600.0)   // 10mins to vacuum sqlite3 nfcm_sum_db

#define TIMER_DURATION (10.0)

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
	if (tmpl.ct)
		nfct_destroy(tmpl.ct);
	if (tmpl.exptuple)
		nfct_destroy(tmpl.exptuple);
	if (tmpl.mask)
		nfct_destroy(tmpl.mask);
	if (tmpl.exp)
		nfexp_destroy(tmpl.exp);
	if (tmpl.label)
		nfct_bitmask_destroy(tmpl.label);
	if (tmpl.label_modify)
		nfct_bitmask_destroy(tmpl.label_modify);
}

union ct_address {
	uint32_t v4;
	uint32_t v6[4];
};

static struct option original_opts[] = {
	{"dump", 2, 0, 'L'},
	{0, 0, 0, 0}
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

#define NFCM_APP_DB_FILE "/jffs/nfcm_app.db"
#define NFCM_SUM_DB_FILE "/jffs/nfcm_sum.db"

// for RTAC68U, RTAC88U
#define ARP_TABLE_NAME "/tmp/arp_table"
#define ROB_TABLE_NAME "/tmp/rob_table"

#define FDB_TABLE_NAME "/tmp/fdb_table"

#define NFCT_PID_FILE "/var/run/nfcm.pid"
#define NFCT_ACCT_FILE "/proc/sys/net/netfilter/nf_conntrack_acct"

/* The below macros handle endian mis-matches between wl utility and wl driver. */
bool g_swap = false;

LIST_HEAD(iplist);
LIST_HEAD(smlist); // summary list from iplist
LIST_HEAD(bklist); // backup previous iplist

LIST_HEAD(arlist);
#if defined(QCA)
  LIST_HEAD(fblist);
#elif defined(HND)
  LIST_HEAD(mclist);
#else
  LIST_HEAD(rblist);
#endif 

#if defined(SQL) 
sqlite3 *appdb = NULL;
sqlite3 *sumdb = NULL;
#endif

LIST_HEAD(lanlist);

lan_info_t *lan_info_new()
{
    lan_info_t *li = (lan_info_t *)calloc(1, sizeof(lan_info_t));
    if (!li) return NULL;

    INIT_LIST_HEAD(&li->list);

    return li;

}

void lan_info_free(lan_info_t *li)
{
	if (li)
		free(li);

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

char *mac2str(const unsigned char *e, char *a)
{
    sprintf(a, "%02x:%02x:%02x:%02x:%02x:%02x", e[0], e[1], e[2], e[3], e[4], e[5]);
    return a;
}

bool is_in_lanv4(struct in_addr *src)
{
    lan_info_t *li;

	//char ipstr[INET6_ADDRSTRLEN];
    //inet_ntop(AF_INET, src, ipstr, INET_ADDRSTRLEN);    

    list_for_each_entry(li, &lanlist, list) {
        if ((src->s_addr & li->subnet.s_addr) == (li->addr.s_addr & li->subnet.s_addr)) {
        //if ((src->s_addr << (32-24)) == (li->addr.s_addr << (32-24))) {
            return true;
        }
    }

    return false;
}

int lan_info_init(struct list_head *list)
{
    lan_info_t *li;
    char word[64], *next;
    char *b, *nv, *nvp;
    char *ifname, *laninfo;
    char *addr, *mask;
    char *rulelist;
    //int fields;

    li = lan_info_new();
    list_add_tail(&li->list , list);

    strcpy(li->ifname, "br0");
    li->enabled = true;
    inet_pton(AF_INET, nvram_get("lan_ipaddr"), &li->addr);
    inet_pton(AF_INET, nvram_get("lan_netmask"), &li->subnet);

    // get wireless guest network status
    if (!nvram_get_int("wgn_enabled")) 
        return 0;

    rulelist = nvram_get("wgn_brif_rulelist");
    if (!rulelist) 
        return 0;

    nv = nvp = strdup(rulelist);
    if (!nv)
        return 0;

    while ((b = strsep(&nvp, "<")) != NULL) {
        if (!b || !strlen(b)) 
            continue;
        //fields = vstrsep(b, ">", &ifname, &laninfo);
        vstrsep(b, ">", &ifname, &laninfo);

        addr = strtok(laninfo, "/");
        mask = strtok(NULL, "/");

        li = lan_info_new();
        list_add_tail(&li->list , list);

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

    free(nv);

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
        case SIGHUP:       sig = "SIGHUP";      break;
        case SIGINT:       sig = "SIGINT";      break;
        case SIGQUIT:      sig = "SIGQUIT";     break;
        case SIGILL:       sig = "SIGILL";      break;
        case SIGTRAP:      sig = "SIGTRAP";     break;
        case SIGABRT:      sig = "SIGABRT";     break;
        case SIGBUS:       sig = "SIGBUS";      break;
        case SIGFPE:       sig = "SIGFPE";      break;
        case SIGKILL:      sig = "SIGKILL";     break;
        case SIGUSR1:      sig = "SIGUSR1";     break;
        case SIGSEGV:      sig = "SIGSEGV";     break;
        case SIGUSR2:      sig = "SIGUSR2";     break;
        case SIGPIPE:      sig = "SIGPIPE";     break;
        case SIGALRM:      sig = "SIGALRM";     break;
        case SIGTERM:      sig = "SIGTERM";     break;
        case SIGCHLD:      sig = "SIGCHLD";     break;
        case SIGCONT:      sig = "SIGCONT";     break;
        case SIGSTOP:      sig = "SIGSTOP";     break;
        case SIGTSTP:      sig = "SIGTSTP";     break;
        case SIGTTIN:      sig = "SIGTTIN";     break;
        case SIGTTOU:      sig = "SIGTTOU";     break;
        case SIGURG:       sig = "SIGURG";      break;
        case SIGXCPU:      sig = "SIGXCPU";     break;
        case SIGXFSZ:      sig = "SIGXFSZ";     break;
        case SIGVTALRM:    sig = "SIGVTALRM";   break;
        case SIGPROF:      sig = "SIGPROF";     break;
        case SIGWINCH:     sig = "SIGWINCH";    break;
        case SIGIO:        sig = "SIGIO";       break;
        case SIGPWR:       sig = "SIGPWR";      break;
        case SIGSYS:       sig = "SIGSYS";      break;

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
	nf_node_t *nn, *nnt;

	list_for_each_entry_safe(nn, nnt, list, list) {
		list_del(&nn->list);
		nf_node_free(nn);
	}
	return;
}

void nf_node_dump(nf_node_t *nn)
{
	char ipstr[INET6_ADDRSTRLEN];

	if (nn->isv4) {
		if (!is_in_lanv4(&nn->srcv4))
			return;
        printf("proto:  %d\n", nn->proto);
		inet_ntop(AF_INET, &nn->srcv4, ipstr, INET_ADDRSTRLEN);
		printf("src:	%s\n", ipstr);
        printf("port:   %d\n", nn->src_port);
        inet_ntop(AF_INET, &nn->dstv4, ipstr, INET_ADDRSTRLEN);
        printf("dst:	%s\n", ipstr);
        printf("port:   %d\n", nn->dst_port);
	} else {
        printf("proto:  %d\n", nn->proto);
		inet_ntop(AF_INET6, &nn->srcv6, ipstr, INET6_ADDRSTRLEN);
		printf("src:	%s\n", ipstr);
        printf("port:   %d\n", nn->src_port);
        inet_ntop(AF_INET6, &nn->dstv6, ipstr, INET6_ADDRSTRLEN);
        printf("dst:	%s\n", ipstr);
        printf("port:   %d\n", nn->dst_port);
	}

	printf("up_pkts:        %llu\n", nn->up_pkts);
    printf("up_diff_pkts:   %llu\n", nn->up_diff_pkts);
    printf("up_ttl_pkts:    %llu\n", nn->up_ttl_pkts);
	printf("up_bytes:       %llu\n", nn->up_bytes);
    printf("up_diff_bytes:  %llu\n", nn->up_diff_bytes);
    printf("up_ttl_bytes:   %llu\n", nn->up_ttl_bytes);
	printf("up_speed:       %llu\n", nn->up_speed);

    printf("dn_pkts:        %llu\n", nn->dn_pkts);
    printf("dn_diff_pkts:   %llu\n", nn->dn_diff_pkts);
    printf("dn_ttl_pkts:    %llu\n", nn->dn_ttl_pkts);
	printf("dn_bytes:       %llu\n", nn->dn_bytes);
    printf("dn_diff_bytes:  %llu\n", nn->dn_diff_bytes);
    printf("dn_ttl_bytes:   %llu\n", nn->dn_ttl_bytes);
	printf("dn_speed:       %llu\n", nn->dn_speed);

    printf("phy_type:       %d\n", nn->layer1_info.eth_type);
	printf("phy_port:       %d\n", nn->layer1_info.eth_port);
	printf("-----------------------\n");

	return;
}

void nf_list_dump(struct list_head *list)
{
	nf_node_t *nn;

    printf("%s:\n", __FUNCTION__);
	list_for_each_entry(nn, list, list) {
		nf_node_dump(nn);
	}
	printf("=======================\n");

	return;
}

void nf_list_move(struct list_head *dst, struct list_head *src)
{
	nf_node_t *nn, *nnt;

	list_for_each_entry_safe(nn, nnt, src, list) {
		list_move_tail(&nn->list, dst);
	}

}

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
        if(!memcmp(&nn->srcv6, &nnt->srcv6, sizeof(struct in6_addr)) &&
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

void nf_list_calc_speed(struct list_head *iplist, struct list_head *bklist)
{
	nf_node_t *nn, *nnt;
	int64_t diff;
    //char src_ipstr[INET6_ADDRSTRLEN];
    //char dst_ipstr[INET6_ADDRSTRLEN];

	list_for_each_entry(nn, iplist, list) {
		list_for_each_entry(nnt, bklist, list) {
			if (nnt->isv4 != nn->isv4)
				continue;
			if (nn->isv4) {
				if (!is_in_lanv4(&nn->srcv4))
					continue;
            }

			if (nf_node_compare(nn->isv4, nn, nnt)) {
                diff = nn->up_pkts - nnt->up_pkts;
                nn->up_diff_pkts = (diff < 0) ? 0 : diff;
                diff = nn->up_bytes - nnt->up_bytes;
                nn->up_diff_bytes = (diff < 0) ? 0 : diff;
                nn->up_speed = (diff < 0) ? 0 : (int64_t)(diff / (int)TIMER_DURATION);

                diff = nn->dn_pkts - nnt->dn_pkts;
                nn->dn_diff_pkts = (diff < 0) ? 0 : diff;
                diff = nn->dn_bytes - nnt->dn_bytes;
                nn->dn_diff_bytes = (diff < 0) ? 0 : diff;
                nn->dn_speed = (diff < 0) ? 0 : (int64_t)(diff / (int)TIMER_DURATION);

                //inet_ntop(AF_INET, &nn->srcv4, src_ipstr, INET_ADDRSTRLEN);
                //inet_ntop(AF_INET, &nn->dstv4, dst_ipstr, INET_ADDRSTRLEN);
                //printf("src:%s[%d], dst:%s[%d] up_diff=[%lld], up_speed=[%lld], dn_diff=[%lld], dn_speed=[%lld]\n", 
                //       src_ipstr, nn->src_port, dst_ipstr, nn->dst_port, 
                //       nn->up_diff_bytes, nn->up_speed, nn->dn_diff_bytes, nn->dn_speed);
				break;
            }
		} // bklist
	} // iplist

}

nf_node_t *nf_node_statistics_search(arp_node_t *ar, struct list_head *smlist)
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
            if (!memcmp(&nn->srcv6, &ar->srcv6, sizeof(struct in6_addr))) 
                return nn;
        }
    }

    return NULL;
}

int nf_list_calc_statistics(struct list_head *arlist, 
                            struct list_head *iplist, 
                            struct list_head *smlist)
{
    nf_node_t *nn = NULL;
    nf_node_t *nnt;
    arp_node_t *ar;
    bool new = false;

    //nf_list_free(smlist);

    // get the summary from iplist based on arlist
    list_for_each_entry(ar, arlist, list) {
        if (!is_in_lanv4(&ar->srcv4))
            continue;
        nn = nf_node_statistics_search(ar, smlist);
        if (nn == NULL) {
            nn = nf_node_new(); 
            nn->isv4 = ar->isv4;
            nn->srcv4.s_addr = ar->srcv4.s_addr;
            memcpy(&nn->srcv6, &ar->srcv6, sizeof(struct in6_addr));
            list_add_tail(&nn->list, smlist);
            new = true;
        }
        nn->up_speed = nn->dn_speed = 0;
        list_for_each_entry(nnt, iplist, list) {
            if (ar->isv4) {
                if (nn->srcv4.s_addr == nnt->srcv4.s_addr) {
                    if (new) {
                        nn->up_pkts += nnt->up_pkts;
                        nn->up_bytes += nnt->up_bytes;
                        nn->dn_pkts += nnt->dn_pkts;
                        nn->dn_bytes += nnt->dn_bytes;
                    } else {
                        nn->up_pkts += nnt->up_diff_pkts;
                        nn->up_bytes += nnt->up_diff_bytes;
                        nn->dn_pkts += nnt->dn_diff_pkts;
                        nn->dn_bytes += nnt->dn_diff_bytes;
                    }
                    nn->up_ttl_pkts += nnt->up_diff_pkts;
                    nn->up_ttl_bytes += nnt->up_diff_bytes;
                    nn->up_speed += nnt->up_speed;

                    nn->dn_ttl_pkts += nnt->dn_diff_pkts;
                    nn->dn_ttl_bytes += nnt->dn_diff_bytes;
                    nn->dn_speed += nnt->dn_speed;

                    nn->layer1_info.eth_type = nnt->layer1_info.eth_type;
                    nn->layer1_info.eth_port = nnt->layer1_info.eth_port;
                }
            } else {
                if (!memcmp(&ar->srcv6, &nnt->srcv6, sizeof(struct in6_addr))) {
                    if (new) {
                        nn->up_pkts += nnt->up_pkts;
                        nn->up_bytes += nnt->up_bytes;
                        nn->dn_pkts += nnt->dn_pkts;
                        nn->dn_bytes += nnt->dn_bytes;
                    } else {
                        nn->up_pkts += nnt->up_diff_pkts;
                        nn->up_bytes += nnt->up_diff_bytes;
                        nn->dn_pkts += nnt->dn_diff_pkts;
                        nn->dn_bytes += nnt->dn_diff_bytes;
                    }
                    nn->up_ttl_pkts += nnt->up_diff_pkts;
                    nn->up_ttl_bytes += nnt->up_diff_bytes;
                    nn->up_speed += nnt->up_speed;

                    nn->dn_ttl_pkts += nnt->dn_diff_pkts;
                    nn->dn_ttl_bytes += nnt->dn_diff_bytes;
                    nn->dn_speed += nnt->dn_speed;

                    nn->layer1_info.eth_type = nnt->layer1_info.eth_type;
                    nn->layer1_info.eth_port = nnt->layer1_info.eth_port;
                }
            }
        }
    }

    return 0;
}

static int ct_handle_cb(enum nf_conntrack_msg_type type, struct nf_conntrack *ct, void *data)
{
    nf_process_conntrack(ct, &iplist, &arlist);

	return NFCT_CB_CONTINUE;
}

// this is callback to parse conntrack
void timer_ct(struct ev_loop *loop, ev_timer *w, int e)
{
	nf_list_free(&bklist);
	nf_list_move(&bklist, &iplist);

	// every nfct_query's entry will call ct_handle_cb
	nfct_query(cth, NFCT_Q_DUMP_FILTER, filter_dump);

	nf_list_calc_speed(&iplist, &bklist);
    nf_list_calc_statistics(&arlist, &iplist, &smlist);

#if defined(NFCMDBG)
    nf_list_dump(&smlist);
	nf_list_dump(&iplist);
#endif // #if defined(NFCMDBG)

#if defined(SQL)
	sqlite_app_insert(appdb, &iplist);
    sqlite_sum_insert(sumdb, &smlist);
#endif

    nf_list_to_json(&iplist, &arlist);
    nf_list_statistics_to_json(&smlist, &arlist);

	//w->repeat = TIMER_DURATION;
	ev_timer_again(loop, w);
}

int ev_timer_ct(struct ev_loop *loop, ev_timer *timer)
{
    // initialize a timer watcher, then start it
    // simple non-repeating TIMER_DURATION second timeout
    ev_init(timer, timer_ct);
    timer->repeat = TIMER_DURATION;
    ev_timer_again(loop, timer);

	return 0;
}

// this is callback to parse fdb(QCA), rob(non-HND), switch-mib(HND), 'arp -n' result
void timer_phy_port(struct ev_loop *loop, ev_timer *w, int e)
{
#if defined(QCA)
    fdb_list_free(&fblist);
    // parse the "ssdk_sh fdb entry show" result
    fdb_list_parse(FDB_TABLE_NAME, &fblist);

    arp_list_free(&arlist);
    // parse the "arp -n" result
    arp_list_parse(ARP_TABLE_NAME, &arlist, &fblist);
#elif defined(HND)
    mc_list_free(&mclist);
    // use ioctl to get switch layer information
    mc_list_parse("", &mclist);

    arp_list_free(&arlist);
    // parse the "arp -n" result
    arp_list_parse(ARP_TABLE_NAME, &arlist, &mclist);
#else
    rob_list_free(&rblist);
    // parse the "robocfg show" result
    rob_list_parse(ROB_TABLE_NAME, &rblist);

    arp_list_free(&arlist);
    // parse the "arp -n" result
    arp_list_parse(ARP_TABLE_NAME, &arlist, &rblist);
#endif

	//w->repeat = TIMER_DURATION;
	ev_timer_again(loop, w);

}

int ev_timer_phy_port(struct ev_loop *loop, ev_timer *timer)
{
    // initialize a timer watcher, then start it
    // simple non-repeating TIMER_DURATION second timeout
    ev_init(timer, timer_phy_port);
    timer->repeat = TIMER_DURATION;
    ev_timer_again(loop, timer);

	return 0;
}

void signal_action(struct ev_loop *loop, ev_signal *w, int e)
{
	char sigstr[64];

	sig_str(w->signum, sigstr, sizeof(sigstr));
	warn("Signal %s received, generating stack dump...", sigstr);

	switch (w->signum) {
    case SIGUSR1:
#if defined(QCA)
        fdb_list_free(&fblist);
        // parse the "ssdk_sh fdb entry show" result
        fdb_list_parse(FDB_TABLE_NAME, &fblist);

        arp_list_free(&arlist);
        // parse the "arp -n" result
        arp_list_parse(ARP_TABLE_NAME, &arlist, &fblist);
#elif defined(HND)
        mc_list_free(&mclist);
        // use ioctl to get switch layer information
        mc_list_parse("", &mclist);

        arp_list_free(&arlist);
        // parse the "arp -n" result
        arp_list_parse(ARP_TABLE_NAME, &arlist, &mclist);
#else 
        rob_list_free(&rblist);
        // parse the "robocfg show" result
        rob_list_parse(ROB_TABLE_NAME, &rblist);

        arp_list_free(&arlist);
        // parse the "arp -n" result
        arp_list_parse(ARP_TABLE_NAME, &arlist, &rblist);
#endif
		break;
	case SIGINT:
	case SIGUSR2:
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
void timer_nfcm_app_db(struct ev_loop *loop, ev_timer *w, int e)
{
    int ret;
    char *err = NULL;
    char sql[1024];
    time_t timestamp;
    struct stat st;

    ret = stat(NFCM_APP_DB_FILE, &st);
    if (st.st_nlink) {
        if ((long)st.st_size > MAX_DB_SIZE) {
            // step1. get timestamp
            timestamp = time(NULL) - 1*DAY_SEC;

            // step2. execute sql to delete nfcm_app
            snprintf(sql, sizeof(sql), "DELETE from nfcm_app WHERE timestamp < %ld;", timestamp);
            printf("sql=[%s]\n", sql);
            ret = sqlite3_exec(appdb, sql,  NULL, NULL, &err);
            if (ret != SQLITE_OK && err != NULL) {
                printf("SQL error: %s\n", err);
                sqlite3_free(err);
                return;
            }

            // step3. compact file
            printf("sql=[%s]\n", "VACUUM;");
            ret = sqlite3_exec(appdb, "VACUUM;",  NULL, NULL, &err);
            if (ret != SQLITE_OK && err != NULL) {
                printf("SQL error: %s\n", err);
                sqlite3_free(err);
                return;
            }
        } 
    } else {
        printf("'%s' file is not existed!\n", NFCM_APP_DB_FILE);
    }

    //w->repeat = TIMER_NFCM_APP_DB;
	ev_timer_again(loop, w);

}

int ev_timer_nfcm_app_db(struct ev_loop *loop, ev_timer *timer)
{
    // initialize a timer watcher, then start it
    // simple non-repeating TIMER_DURATION second timeout
    ev_init(timer, timer_nfcm_app_db);
    timer->repeat = TIMER_NFCM_APP_DB;
    ev_timer_again(loop, timer);

	return 0;
}

void timer_nfcm_sum_db(struct ev_loop *loop, ev_timer *w, int e)
{
    int ret;
    char *err = NULL;
    char sql[1024];
    time_t timestamp;
    struct stat st;

    ret = stat(NFCM_SUM_DB_FILE, &st);
    if (st.st_nlink) {
        if ((long)st.st_size > MAX_DB_SIZE) {
            // step1. get timestamp
            timestamp = time(NULL) - 5*DAY_SEC;

            // step2. execute sql to delete nfcm_sum
            snprintf(sql, sizeof(sql), "DELETE from nfcm_sum WHERE timestamp < %ld;", timestamp);
            printf("sql=[%s]\n", sql);
            ret = sqlite3_exec(sumdb, sql,  NULL, NULL, &err);
            if (ret != SQLITE_OK && err != NULL) {
                printf("SQL error: %s\n", err);
                sqlite3_free(err);
                return;
            }

            // step3. compact file
            printf("sql=[%s]\n", "VACUUM;");
            ret = sqlite3_exec(sumdb, "VACUUM;",  NULL, NULL, &err);
            if (ret != SQLITE_OK && err != NULL) {
                printf("SQL error: %s\n", err);
                sqlite3_free(err);
                return;
            }
        } 
    } else {
        printf("'%s' file is not existed!\n", NFCM_SUM_DB_FILE);
    }

    //w->repeat = TIMER_NFCM_SUM_DB;
	ev_timer_again(loop, w);

}

int ev_timer_nfcm_sum_db(struct ev_loop *loop, ev_timer *timer)
{
    // initialize a timer watcher, then start it
    // simple non-repeating TIMER_DURATION second timeout
    ev_init(timer, timer_nfcm_app_db);
    timer->repeat = TIMER_NFCM_SUM_DB;
    ev_timer_again(loop, timer);

	return 0;
}

#endif 

static bool nf_write_pid_file(char *fname)
{
    char pidstr[32];

	FILE *fp = fopen(fname, "w+");
	if (!fp)  
        return false;

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

int main(int argc, char *argv[])
{
	int family = AF_UNSPEC;
	struct ev_loop *loop = EV_DEFAULT;
	ev_timer timer_ct;
	ev_timer timer_phy_port;
#if defined(SQL)
    //ev_stat stat_nfcm_db;
    ev_timer timer_nfcm_app_db;
    ev_timer timer_nfcm_sum_db;
#endif
	ev_signal signal_sigint;
	ev_signal signal_sigusr1;
	ev_signal signal_sigusr2;

    lan_info_init(&lanlist);

    nf_set_ct_acct_flags(NFCT_ACCT_FILE);
	nf_write_pid_file(NFCT_PID_FILE);

	/* we release these objects in the exit_error() path. */
	if (!alloc_tmpl_objects())
		exit_error(OTHER_PROBLEM, "out of memory");

#if defined(SQL)
    appdb = sqlite_open(true, appdb, NFCM_APP_DB_FILE);
    sumdb = sqlite_open(false, sumdb, NFCM_SUM_DB_FILE);
#endif

#if defined(QCA)
    fdb_list_parse(FDB_TABLE_NAME, &fblist);
    arp_list_parse(ARP_TABLE_NAME, &arlist, &fblist);
#elif defined(HND)
    mc_list_parse("", &mclist);
    arp_list_parse(ARP_TABLE_NAME, &arlist, &mclist);
#else 
    rob_list_parse(ROB_TABLE_NAME, &rblist);
    arp_list_parse(ARP_TABLE_NAME, &arlist, &rblist);
#endif

	cth = nfct_open(CONNTRACK, 0);
	if (!cth)
		exit_error(OTHER_PROBLEM, "Can't open handler");

	if (options & CT_COMPARISON && options & CT_OPT_ZERO)
		exit_error(PARAMETER_PROBLEM, "Can't use -z with filtering parameters");

	// tmpl.ct will pass to ct_handle_cb as 'void *data'
	nfct_callback_register(cth, NFCT_T_ALL, ct_handle_cb, tmpl.ct);

	filter_dump = nfct_filter_dump_create();
	if (filter_dump == NULL)
		exit_error(OTHER_PROBLEM, "OOM");

	if (tmpl.filter_mark_kernel_set) {
		nfct_filter_dump_set_attr(filter_dump, NFCT_FILTER_DUMP_MARK, &tmpl.filter_mark_kernel);
		nfct_filter_dump_set_attr_u8(filter_dump, NFCT_FILTER_DUMP_L3NUM, family);
	}

	nfct_query(cth, NFCT_Q_DUMP_FILTER, filter_dump);

#if defined(NFCMDBG)
	nf_list_dump(&iplist);
#endif // #if defined(NFCMDBG)

	nf_list_to_json(&iplist, &arlist);
	nf_list_free(&iplist);

	ev_timer_phy_port(loop, &timer_phy_port);
	ev_timer_ct(loop, &timer_ct);
#if defined(SQL)
    ev_timer_nfcm_app_db(loop, &timer_nfcm_app_db);
    ev_timer_nfcm_sum_db(loop, &timer_nfcm_sum_db);
#endif
	ev_signal_sigint(loop, &signal_sigint);
	ev_signal_sigusr1(loop, &signal_sigusr1); // re-parse
	ev_signal_sigusr2(loop, &signal_sigusr2);

	// main loop
	ev_run(loop, 0);

	nf_list_free(&iplist);
	nf_list_free(&bklist);

#if defined(QCA)
    fdb_list_free(&fblist);
#elif defined(HND)
    mc_list_free(&mclist);
#else
    rob_list_free(&rblist);
#endif
    arp_list_free(&arlist);

	//nfct_filter_dump_destroy(filter_dump);
	nfct_close(cth);
#if defined(SQL)
    sqlite_close(appdb);
    sqlite_close(sumdb);
#endif
	free_tmpl_objects();

    return 0;
}
