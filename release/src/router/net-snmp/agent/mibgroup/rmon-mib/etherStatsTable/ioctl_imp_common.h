#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <ifaddrs.h>

/* use kernel's ethtool.h  */

#ifdef HAVE_LINUX_ETHTOOL_NEEDS_U64
#include <linux/types.h>
typedef __u64 u64;
typedef __u32 u32;
typedef __u16 u16;
typedef __u8 u8;
#endif
#include <linux/ethtool.h>

/* structure for storing the interface names in the system */

struct ifname {
    struct ifname *ifn_next;
    char name [IF_NAMESIZE];
};

struct ifname *etherstats_interface_name_list_get (struct ifname *, int *);
int etherstats_interface_name_list_free (struct ifname *list_head);
int etherstats_interface_ioctl_ifindex_get (int fd, const char *name);
int _etherStats_ioctl_get(int fd, int which, struct ifreq *ifrq, const char* name);
int interface_ioctl_etherstats_get(etherStatsTable_rowreq_ctx *rowreq_ctx, int fd, const char* name);

/* for maintainability */

#define GENERIC_INCOMING_OCTETS                 "rx_octets"
#define GENERIC_INCOMING_PACKETS                "rx_packets"
#define DSA_INCOMING_GOOD_OCTETS                "in_good_octets"
#define DSA_INCOMING_BAD_OCTETS                 "in_bad_octets"
#define GENERIC_INCOMING_UNICAST                "rx_unicast"
#define DSA_INCOMING_UNICAST                    "in_unicast"
#define GENERIC_INCOMING_BROADCAST              "rx_broadcast"
#define DSA_INCOMING_BROADCAST                  "in_broadcast"
#define GENERIC_INCOMING_MULTICAST              "rx_multicast"
#define DSA_INCOMING_MULTICAST                  "in_multicast"
#define DSA_INCOMING_FCS_ERROR                  "in_fcs_error"
#define FEC_INCOMING_CRC_ERROR                  "rx_crc_errors"
#define DSA_INCOMING_UNDERSIZE                  "in_undersize"
#define FEC_INCOMING_UNDERSIZE                  "rx_undersize"
#define DSA_INCOMING_OVERSIZE                   "in_oversize"
#define FEC_INCOMING_OVERSIZE                   "rx_oversize"
#define DSA_INCOMING_FRAGMENTS                  "in_fragments"
#define FEC_INCOMING_FRAGMENT                   "rx_fragment"
#define DSA_INCOMING_JABBER                     "in_jabber"
#define FEC_INCOMING_JABBER                     "rx_jabber"
#define BROADCOM_RECEIVE_JABBERS                "rx_jabbers"
#define DSA_COLLISIONS                          "collisions"
#define FEC_OUTGOING_COLLISION                  "tx_collision"
#define DSA_64BYTES                             "hist_64bytes"
#define FEC_64BYTES                             "rx_64byte"
#define DSA_65_127BYTES                         "hist_65_127bytes"
#define FEC_65_127BYTES                         "rx_65to127byte"
#define DSA_128_255BYTES                        "hist_128_255bytes"
#define FEC_128_255BYTES                        "rx_128to255byte"
#define DSA_256_511BYTES                        "hist_256_511bytes"
#define FEC_256_511BYTES                        "rx_256to511byte"
#define DSA_512_1023BYTES                       "hist_512_1023bytes"
#define FEC_512_1023BYTES                       "rx_512to1023byte"
#define DSA_1024_MAXBYTES                       "hist_1024_max_bytes"
#define FEC_1024_2047BYTES                      "rx_1024to2047byte"
#define FEC_GTE2048BYTES                        "rx_GTE2048byte"

#define ETHERSTATSOCTETS(x)                     (!strcmp(x, DSA_INCOMING_GOOD_OCTETS) || \
                                                 !strcmp(x, DSA_INCOMING_BAD_OCTETS) || \
                                                 !strcmp(x, GENERIC_INCOMING_OCTETS))
#define ETHERSTATSPKTS(x)                       (!strcmp(x, DSA_INCOMING_UNICAST) || \
                                                 !strcmp(x, DSA_INCOMING_BROADCAST) || \
                                                 !strcmp(x, DSA_INCOMING_MULTICAST) || \
                                                 !strcmp(x, GENERIC_INCOMING_PACKETS))
#define ETHERSTATSBROADCASTPKTS(x)              (!strcmp(x, DSA_INCOMING_BROADCAST) || \
                                                 !strcmp(x, GENERIC_INCOMING_BROADCAST))
#define ETHERSTATSMULTICASTPKTS(x)              (!strcmp(x, DSA_INCOMING_MULTICAST) || \
                                                 !strcmp(x, GENERIC_INCOMING_MULTICAST))
#define ETHERSTATSCRCALIGNERRORS(x)             (!strcmp(x, DSA_INCOMING_FCS_ERROR) || \
                                                 !strcmp(x, FEC_INCOMING_CRC_ERROR))
#define ETHERSTATSUNDERSIZEPKTS(x)              (!strcmp(x, DSA_INCOMING_UNDERSIZE) || \
                                                 !strcmp(x, FEC_INCOMING_UNDERSIZE))
#define ETHERSTATSOVERSIZEPKTS(x)               (!strcmp(x, DSA_INCOMING_OVERSIZE) || \
                                                 !strcmp(x, FEC_INCOMING_OVERSIZE))
#define ETHERSTATSFRAGMENTS(x)                  (!strcmp(x, DSA_INCOMING_FRAGMENTS) || \
                                                 !strcmp(x, FEC_INCOMING_FRAGMENT))
#define ETHERSTATSJABBERS(x)                    (!strcmp(x, BROADCOM_RECEIVE_JABBERS) || \
                                                 !strcmp(x, FEC_INCOMING_JABBER) || \
                                                 !strcmp(x, DSA_INCOMING_JABBER))
#define ETHERSTATSCOLLISIONS(x)                 (!strcmp(x, DSA_COLLISIONS) || \
                                                 !strcmp(x, FEC_OUTGOING_COLLISION))
#define ETHERSTATSPKTS64OCTETS(x)               (!strcmp(x, DSA_64BYTES) || \
                                                 !strcmp(x, FEC_64BYTES))
#define ETHERSTATSPKTS65TO127OCTETS(x)          (!strcmp(x, DSA_65_127BYTES) || \
                                                 !strcmp(x, FEC_65_127BYTES))
#define ETHERSTATSPKTS128TO255OCTETS(x)         (!strcmp(x, DSA_128_255BYTES) || \
                                                 !strcmp(x, FEC_128_255BYTES))
#define ETHERSTATSPKTS256TO511OCTETS(x)         (!strcmp(x, DSA_256_511BYTES) || \
                                                 !strcmp(x, FEC_256_511BYTES))
#define ETHERSTATSPKTS512TO1023OCTETS(x)        (!strcmp(x, DSA_512_1023BYTES) || \
                                                 !strcmp(x, FEC_512_1023BYTES))
#define ETHERSTATSPKTS1024TO1518OCTETS(x)       (!strcmp(x, DSA_1024_MAXBYTES) || \
                                                 !strcmp(x, FEC_1024_2047BYTES) || \
                                                 !strcmp(x, FEC_GTE2048BYTES))
