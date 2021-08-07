// gcc -Wall -Wextra -g -O0 -DMAIN -o nl_debug nl_debug.c && ./nl_debug

/*
   Netlink debug functions

   Copyright (C) Volodymyr Prodan 2021

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//#define _GNU_SOURCE // avoid conflict of <net/if.h> with <linux/if.h>
//#define _POSIX_C_SOURCE 200809

#include "nl_debug.h" // WLAN_EID_SSID

#include <stdio.h> // stdout, printf()
#include <stdarg.h> // va_start(), ...
#include <unistd.h> // getpid(), close()
#include <string.h> // memset(), memcmp()
#include <fcntl.h> // F_GETFL
#include <ctype.h> // isprint()
#include <errno.h> // errno
#include <sys/socket.h> // struct sockaddr, PF_NETLINK
#include <netinet/in.h> // struct sockaddr_in{,6}, in{,6}_addr, INET{,6}_ADDRSTRLEN
#include <arpa/inet.h> // inet_ntop()
#include <netdb.h> // getnameinfo()
#include <net/if.h> // avoid conflict with <linux/if.h> included from <linux/wireless.h>
#include <linux/if.h> // IFF_UP, ...
#include <linux/wireless.h> // struct iw_event, SIOCGIWSCAN, ...
#include <linux/netlink.h> // NETLINK_ROUTE
#include <linux/rtnetlink.h> // RTM_GETADDR, IFA_ADDRESS, /usr/include/linux/if_addr.h

#ifdef MAIN
#define debugf(...) do { printf(__VA_ARGS__); putchar('\n'); } while (0)
#else
#include "wsdd.h" // LOG()
#define debugf(...) LOG(LOG_DEBUG, "nl_debug: " __VA_ARGS__)
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

static char outbuf[4096];
static int outlen = 0;

static void outf(const char *fmt, ... ) __attribute__((format(printf, 1, 2)));

void outf(const char *fmt, ...)
{
    char mbuf[1024], *lf;
    int mbuflen;
    va_list list;

    va_start(list, fmt);
    mbuflen = vsnprintf(mbuf, sizeof(mbuf), fmt, list);
    va_end(list);

    strncat(outbuf + outlen, mbuf, mbuflen);
    lf = strchr(outbuf + outlen, '\n');
    outlen += mbuflen;

    while (lf != NULL) {
        *lf = '\0';
        debugf("%s", outbuf);
        strcpy(outbuf, lf + 1);
        outlen -= lf - outbuf + 1;
        lf = strchr(outbuf, '\n');
    }
}

void dump(const void *p, size_t len, unsigned long start, const char *prefix)
{
    const unsigned char *s = p;
    for (size_t i = 0; i < len; i += 16) {
        size_t j, blen = len - i > 16 ? 16 : len - i;
        outf("%s[%08lx]", prefix, start + i);
        for (j = 0; j < blen; j++) outf(" %02x", s[i+j]);
        for (; j < 16; j++) outf("   ");
        outf(" |");
        for (j = 0; j < blen; j++) outf("%c", isprint(s[i+j]) ? s[i+j] : '.');
        for (; j < 16; j++) outf(" ");
        outf("|\n");
    }
}

void dump_str(const void *p, size_t len)
{
    const unsigned char *s = p;
    outf("\"");
    for (size_t i = 0; i < len; i++) {
        if (isprint(s[i]) && s[i] != '"') outf("%c", s[i]);
        else outf("\\x%02x", s[i]);
    }
    outf("\"");
}

void dump_hex(const void *p, size_t len)
{
    const unsigned char *s = p;
    for (size_t i = 0; i < len; i++) outf("%02x", s[i]);
}

static const char nlmsg_type_str[][16] = {
    [NLMSG_NOOP]              = "NOOP",   /* Nothing       */
    [NLMSG_ERROR]             = "ERROR",  /* Error         */
    [NLMSG_DONE]              = "DONE",   /* End of a dump */
    [NLMSG_OVERRUN]           = "OVERRUN",/* Data lost     */
    [RTM_NEWLINK]             = "NEWLINK",
    [RTM_DELLINK]             = "DELLINK",
    [RTM_GETLINK]             = "GETLINK",
    [RTM_SETLINK]             = "SETLINK",
    [RTM_NEWADDR]             = "NEWADDR",
    [RTM_DELADDR]             = "DELADDR",
    [RTM_GETADDR]             = "GETADDR",
    [RTM_NEWROUTE]            = "NEWROUTE",
    [RTM_DELROUTE]            = "DELROUTE",
    [RTM_GETROUTE]            = "GETROUTE",
    [RTM_NEWNEIGH]            = "NEWNEIGH",
    [RTM_DELNEIGH]            = "DELNEIGH",
    [RTM_GETNEIGH]            = "GETNEIGH",
    [RTM_NEWRULE]             = "NEWRULE",
    [RTM_DELRULE]             = "DELRULE",
    [RTM_GETRULE]             = "GETRULE",
    [RTM_NEWQDISC]            = "NEWQDISC",
    [RTM_DELQDISC]            = "DELQDISC",
    [RTM_GETQDISC]            = "GETQDISC",
    [RTM_NEWTCLASS]           = "NEWTCLASS",
    [RTM_DELTCLASS]           = "DELTCLASS",
    [RTM_GETTCLASS]           = "GETTCLASS",
    [RTM_NEWTFILTER]          = "NEWTFILTER",
    [RTM_DELTFILTER]          = "DELTFILTER",
    [RTM_GETTFILTER]          = "GETTFILTER",
    [RTM_NEWACTION]           = "NEWACTION",
    [RTM_DELACTION]           = "DELACTION",
    [RTM_GETACTION]           = "GETACTION",
    [RTM_NEWPREFIX]           = "NEWPREFIX",
    [RTM_GETMULTICAST]        = "GETMULTICAST",
    [RTM_GETANYCAST]          = "GETANYCAST",
    [RTM_NEWNEIGHTBL]         = "NEWNEIGHTBL",
    [RTM_GETNEIGHTBL]         = "GETNEIGHTBL",
    [RTM_SETNEIGHTBL]         = "SETNEIGHTBL",
    [RTM_NEWNDUSEROPT]        = "NEWNDUSEROPT",
    [RTM_NEWADDRLABEL]        = "NEWADDRLABEL",
    [RTM_DELADDRLABEL]        = "DELADDRLABEL",
    [RTM_GETADDRLABEL]        = "GETADDRLABEL",
    [RTM_GETDCB]              = "GETDCB",
    [RTM_SETDCB]              = "SETDCB",
    [RTM_NEWNETCONF]          = "NEWNETCONF",
    [RTM_DELNETCONF]          = "DELNETCONF",
    [RTM_GETNETCONF]          = "GETNETCONF",
    [RTM_NEWMDB]              = "NEWMDB",
    [RTM_DELMDB]              = "DELMDB",
    [RTM_GETMDB]              = "GETMDB",
    [RTM_NEWNSID]             = "NEWNSID",
    [RTM_DELNSID]             = "DELNSID",
    [RTM_GETNSID]             = "GETNSID",
    [RTM_NEWSTATS]            = "NEWSTATS",
    [RTM_GETSTATS]            = "GETSTATS",
    [RTM_NEWCACHEREPORT]      = "NEWCACHEREPORT",
    [RTM_NEWCHAIN]            = "NEWCHAIN",
    [RTM_DELCHAIN]            = "DELCHAIN",
    [RTM_GETCHAIN]            = "GETCHAIN",
    [RTM_NEWNEXTHOP]          = "NEWNEXTHOP",
    [RTM_DELNEXTHOP]          = "DELNEXTHOP",
    [RTM_GETNEXTHOP]          = "GETNEXTHOP",
    [RTM_NEWLINKPROP]         = "NEWLINKPROP",
    [RTM_DELLINKPROP]         = "DELLINKPROP",
    [RTM_GETLINKPROP]         = "GETLINKPROP",
    [RTM_NEWNVLAN]            = "NEWVLAN",
    [RTM_DELVLAN]             = "DELVLAN",
    [RTM_GETVLAN]             = "GETVLAN",
};

static const char ifla_rta_type_str[][32] = {
    [IFLA_UNSPEC]            = "UNSPEC",
    [IFLA_ADDRESS]           = "ADDRESS",
    [IFLA_BROADCAST]         = "BROADCAST",
    [IFLA_IFNAME]            = "IFNAME",
    [IFLA_MTU]               = "MTU",
    [IFLA_LINK]              = "LINK",
    [IFLA_QDISC]             = "QDISC",
    [IFLA_STATS]             = "STATS",
    [IFLA_COST]              = "COST",
    [IFLA_PRIORITY]          = "PRIORITY",
    [IFLA_MASTER]            = "MASTER",
    [IFLA_WIRELESS]          = "WIRELESS",              /* Wireless Extension event - see wireless.h */
    [IFLA_PROTINFO]          = "PROTINFO",              /* Protocol specific information for a link */
    [IFLA_TXQLEN]            = "TXQLEN",
    [IFLA_MAP]               = "MAP",
    [IFLA_WEIGHT]            = "WEIGHT",
    [IFLA_OPERSTATE]         = "OPERSTATE",
    [IFLA_LINKMODE]          = "LINKMODE",
    [IFLA_LINKINFO]          = "LINKINFO",
    [IFLA_NET_NS_PID]        = "NET_NS_PID",
    [IFLA_IFALIAS]           = "IFALIAS",
    [IFLA_NUM_VF]            = "NUM_VF",                /* Number of VFs if device is SR-IOV PF */
    [IFLA_VFINFO_LIST]       = "VFINFO_LIST",
    [IFLA_STATS64]           = "STATS64",
    [IFLA_VF_PORTS]          = "VF_PORTS",
    [IFLA_PORT_SELF]         = "PORT_SELF",
    [IFLA_AF_SPEC]           = "AF_SPEC",
    [IFLA_GROUP]             = "GROUP",         /* Group the device belongs to */
    [IFLA_NET_NS_FD]         = "NET_NS_FD",
    [IFLA_EXT_MASK]          = "EXT_MASK",              /* Extended info mask, VFs, etc */
    [IFLA_PROMISCUITY]       = "PROMISCUITY",   /* Promiscuity count: > 0 means acts PROMISC */
    [IFLA_NUM_TX_QUEUES]     = "NUM_TX_QUEUES",
    [IFLA_NUM_RX_QUEUES]     = "NUM_RX_QUEUES",
    [IFLA_CARRIER]           = "CARRIER",
    [IFLA_PHYS_PORT_ID]      = "PHYS_PORT_ID",
    [IFLA_CARRIER_CHANGES]   = "CARRIER_CHANGES",
    [IFLA_PHYS_SWITCH_ID]    = "PHYS_SWITCH_ID",
    [IFLA_LINK_NETNSID]      = "LINK_NETNSID",
    [IFLA_PHYS_PORT_NAME]    = "PHYS_PORT_NAME",
    [IFLA_PROTO_DOWN]        = "PROTO_DOWN",
    [IFLA_GSO_MAX_SEGS]      = "GSO_MAX_SEGS",
    [IFLA_GSO_MAX_SIZE]      = "GSO_MAX_SIZE",
    [IFLA_PAD]               = "PAD",
    [IFLA_XDP]               = "XDP",
    [IFLA_EVENT]             = "EVENT",
    [IFLA_NEW_NETNSID]       = "NEW_NETNSID",
    [IFLA_TARGET_NETNSID]    = "TARGET_NETNSID",
    [IFLA_CARRIER_UP_COUNT]  = "CARRIER_UP_COUNT",
    [IFLA_CARRIER_DOWN_COUNT]= "CARRIER_DOWN_COUNT",
    [IFLA_NEW_IFINDEX]       = "NEW_IFINDEX",
    [IFLA_MIN_MTU]           = "MIN_MTU",
    [IFLA_MAX_MTU]           = "MAX_MTU",
    [IFLA_PROP_LIST]         = "PROP_LIST",
    [IFLA_ALT_IFNAME]        = "ALT_IFNAME", /* Alternative ifname */
    [IFLA_PERM_ADDRESS]      = "PERM_ADDRESS",
    [IFLA_PROTO_DOWN_REASON] = "PROTO_DOWN_REASON",
};

static const char ifa_rta_type_str[][16] = {
    [IFA_UNSPEC]             = "UNSPEC",
    [IFA_ADDRESS]            = "ADDRESS",
    [IFA_LOCAL]              = "LOCAL",
    [IFA_LABEL]              = "LABEL",
    [IFA_BROADCAST]          = "BROADCAST",
    [IFA_ANYCAST]            = "ANYCAST",
    [IFA_CACHEINFO]          = "CACHEINFO",
    [IFA_MULTICAST]          = "MULTICAST",
    [IFA_FLAGS]              = "FLAGS",
    [IFA_RT_PRIORITY]        = "RT_PRIORITY",  /* u32, priority/metric for prefix route */
    [IFA_TARGET_NETNSID]     = "TARGET_NETNSID",
};

static const char rtmsg_type_str[][16] = {
    [RTN_UNSPEC]             = "UNSPEC",
    [RTN_UNICAST]            = "UNICAST",       /* Gateway or direct route      */
    [RTN_LOCAL]              = "LOCAL",         /* Accept locally               */
    [RTN_BROADCAST]          = "BROADCAST",     /* Accept locally as broadcast, send as broadcast */
    [RTN_ANYCAST]            = "ANYCAST",       /* Accept locally as broadcast, but send as unicast */
    [RTN_MULTICAST]          = "MULTICAST",     /* Multicast route              */
    [RTN_BLACKHOLE]          = "BLACKHOLE",     /* Drop                         */
    [RTN_UNREACHABLE]        = "UNREACHABLE",   /* Destination is unreachable   */
    [RTN_PROHIBIT]           = "PROHIBIT",      /* Administratively prohibited  */
    [RTN_THROW]              = "THROW",         /* Not in this table            */
    [RTN_NAT]                = "NAT",           /* Translate this address       */
    [RTN_XRESOLVE]           = "XRESOLVE",      /* Use external resolver        */
};

static const char rtattr_type_str[][16] = {
    [RTA_UNSPEC]             = "UNSPEC",
    [RTA_DST]                = "DST",
    [RTA_SRC]                = "SRC",
    [RTA_IIF]                = "IIF",
    [RTA_OIF]                = "OIF",
    [RTA_GATEWAY]            = "GATEWAY",
    [RTA_PRIORITY]           = "PRIORITY",
    [RTA_PREFSRC]            = "PREFSRC",
    [RTA_METRICS]            = "METRICS",
    [RTA_MULTIPATH]          = "MULTIPATH",
    [RTA_PROTOINFO]          = "PROTOINFO", /* no longer used */
    [RTA_FLOW]               = "FLOW",
    [RTA_CACHEINFO]          = "CACHEINFO",
    [RTA_SESSION]            = "SESSION", /* no longer used */
    [RTA_MP_ALGO]            = "MP_ALGO", /* no longer used */
    [RTA_TABLE]              = "TABLE",
    [RTA_MARK]               = "MARK",
    [RTA_MFC_STATS]          = "MFC_STATS",
    [RTA_VIA]                = "VIA",
    [RTA_NEWDST]             = "NEWDST",
    [RTA_PREF]               = "PREF",
    [RTA_ENCAP_TYPE]         = "ENCAP_TYPE",
    [RTA_ENCAP]              = "ENCAP",
    [RTA_EXPIRES]            = "EXPIRES",
    [RTA_PAD]                = "PAD",
    [RTA_UID]                = "UID",
    [RTA_TTL_PROPAGATE]      = "TTL_PROPAGATE",
    [RTA_IP_PROTO]           = "IP_PROTO",
    [RTA_SPORT]              = "SPORT",
    [RTA_DPORT]              = "DPORT",
    [RTA_NH_ID]              = "NH_ID",
};

static struct {
    int flag;
    const char *name;
} ifa_flags_descr[] = {
    { IFA_F_SECONDARY,       "SECONDARY" },
    { IFA_F_NODAD,           "NODAD" },
    { IFA_F_OPTIMISTIC,      "OPTIMISTIC" },
    { IFA_F_DADFAILED,       "DADFAILED" },
    { IFA_F_HOMEADDRESS,     "HOMEADDRESS" },
    { IFA_F_DEPRECATED,      "DEPRECATED" },
    { IFA_F_TENTATIVE,       "TENTATIVE" },
    { IFA_F_PERMANENT,       "PERMANENT" },
    { IFA_F_MANAGETEMPADDR,  "MANAGETEMPADDR" },
    { IFA_F_NOPREFIXROUTE,   "NOPREFIXROUTE" },
    { IFA_F_MCAUTOJOIN,      "MCAUTOJOIN" },
    { IFA_F_STABLE_PRIVACY,  "STABLE_PRIVACY" },
    { 0, "" },
};

static const char iw_event_cmd_str[][40] = {
    [SIOCSIWCOMMIT-SIOCIWFIRST]	= "SIOCSIWCOMMIT",		/* Commit pending changes to driver */
    [SIOCGIWNAME-SIOCIWFIRST]	= "SIOCGIWNAME",		/* get name == wireless protocol */
        /* SIOCGIWNAME is used to verify the presence of Wireless Extensions.
         * Common values : "IEEE 802.11-DS", "IEEE 802.11-FH", "IEEE 802.11b"...
         * Don't put the name of your driver there, it's useless. */
    /* Basic operations */
    [SIOCSIWNWID-SIOCIWFIRST]	= "SIOCSIWNWID",		/* set network id (pre-802.11) */
    [SIOCGIWNWID-SIOCIWFIRST]	= "SIOCGIWNWID",		/* get network id (the cell) */
    [SIOCSIWFREQ-SIOCIWFIRST]	= "SIOCSIWFREQ",		/* set channel/frequency (Hz) */
    [SIOCGIWFREQ-SIOCIWFIRST]	= "SIOCGIWFREQ",		/* get channel/frequency (Hz) */
    [SIOCSIWMODE-SIOCIWFIRST]	= "SIOCSIWMODE",		/* set operation mode */
    [SIOCGIWMODE-SIOCIWFIRST]	= "SIOCGIWMODE",		/* get operation mode */
    [SIOCSIWSENS-SIOCIWFIRST]	= "SIOCSIWSENS",		/* set sensitivity (dBm) */
    [SIOCGIWSENS-SIOCIWFIRST]	= "SIOCGIWSENS",		/* get sensitivity (dBm) */
    /* Informative stuff */
    [SIOCSIWRANGE-SIOCIWFIRST]	= "SIOCSIWRANGE",		/* Unused */
    [SIOCGIWRANGE-SIOCIWFIRST]	= "SIOCGIWRANGE",		/* Get range of parameters */
    [SIOCSIWPRIV-SIOCIWFIRST]	= "SIOCSIWPRIV",		/* Unused */
    [SIOCGIWPRIV-SIOCIWFIRST]	= "SIOCGIWPRIV",		/* get private ioctl interface info */
    [SIOCSIWSTATS-SIOCIWFIRST]	= "SIOCSIWSTATS",		/* Unused */
    [SIOCGIWSTATS-SIOCIWFIRST]	= "SIOCGIWSTATS",		/* Get /proc/net/wireless stats */
        /* SIOCGIWSTATS is strictly used between user space and the kernel, and
         * is never passed to the driver (i.e. the driver will never see it). */
    /* Spy support (statistics per MAC address - used for Mobile IP support) */
    [SIOCSIWSPY-SIOCIWFIRST]	= "SIOCSIWSPY",			/* set spy addresses */
    [SIOCGIWSPY-SIOCIWFIRST]	= "SIOCGIWSPY",			/* get spy info (quality of link) */
    [SIOCSIWTHRSPY-SIOCIWFIRST]	= "SIOCSIWTHRSPY",		/* set spy threshold (spy event) */
    [SIOCGIWTHRSPY-SIOCIWFIRST]	= "SIOCGIWTHRSPY",		/* get spy threshold */
    /* Access Point manipulation */
    [SIOCSIWAP-SIOCIWFIRST]	= "SIOCSIWAP",			/* set access point MAC addresses */
    [SIOCGIWAP-SIOCIWFIRST]	= "SIOCGIWAP",			/* get access point MAC addresses */
    [SIOCGIWAPLIST-SIOCIWFIRST]	= "SIOCGIWAPLIST",		/* Deprecated in favor of scanning */
    [SIOCSIWSCAN-SIOCIWFIRST]	= "SIOCSIWSCAN",		/* trigger scanning (list cells) */
    [SIOCGIWSCAN-SIOCIWFIRST]	= "SIOCGIWSCAN",		/* get scanning results */
    /* 802.11 specific support */
    [SIOCSIWESSID-SIOCIWFIRST]	= "SIOCSIWESSID",		/* set ESSID (network name) */
    [SIOCGIWESSID-SIOCIWFIRST]	= "SIOCGIWESSID",		/* get ESSID */
    [SIOCSIWNICKN-SIOCIWFIRST]	= "SIOCSIWNICKN",		/* set node name/nickname */
    [SIOCGIWNICKN-SIOCIWFIRST]	= "SIOCGIWNICKN",		/* get node name/nickname */
        /* As the ESSID and NICKN are strings up to 32 bytes long, it doesn't fit
         * within the 'iwreq' structure, so we need to use the 'data' member to
         * point to a string in user space, like it is done for RANGE... */
    /* Other parameters useful in 802.11 and some other devices */
    [SIOCSIWRATE-SIOCIWFIRST]	= "SIOCSIWRATE",		/* set default bit rate (bps) */
    [SIOCGIWRATE-SIOCIWFIRST]	= "SIOCGIWRATE",		/* get default bit rate (bps) */
    [SIOCSIWRTS-SIOCIWFIRST]	= "SIOCSIWRTS",		/* set RTS/CTS threshold (bytes) */
    [SIOCGIWRTS-SIOCIWFIRST]	= "SIOCGIWRTS",		/* get RTS/CTS threshold (bytes) */
    [SIOCSIWFRAG-SIOCIWFIRST]	= "SIOCSIWFRAG",		/* set fragmentation thr (bytes) */
    [SIOCGIWFRAG-SIOCIWFIRST]	= "SIOCGIWFRAG",		/* get fragmentation thr (bytes) */
    [SIOCSIWTXPOW-SIOCIWFIRST]	= "SIOCSIWTXPOW",		/* set transmit power (dBm) */
    [SIOCGIWTXPOW-SIOCIWFIRST]	= "SIOCGIWTXPOW",		/* get transmit power (dBm) */
    [SIOCSIWRETRY-SIOCIWFIRST]	= "SIOCSIWRETRY",		/* set retry limits and lifetime */
    [SIOCGIWRETRY-SIOCIWFIRST]	= "SIOCGIWRETRY",		/* get retry limits and lifetime */
    /* Encoding stuff (scrambling, hardware security, WEP...) */
    [SIOCSIWENCODE-SIOCIWFIRST]	= "SIOCSIWENCODE",		/* set encoding token & mode */
    [SIOCGIWENCODE-SIOCIWFIRST]	= "SIOCGIWENCODE",		/* get encoding token & mode */
    /* Power saving stuff (power management, unicast and multicast) */
    [SIOCSIWPOWER-SIOCIWFIRST]	= "SIOCSIWPOWER",		/* set Power Management settings */
    [SIOCGIWPOWER-SIOCIWFIRST]	= "SIOCGIWPOWER",		/* get Power Management settings */
    /* WPA : Generic IEEE 802.11 informatiom element (e.g., for WPA/RSN/WMM).
     * This ioctl uses struct iw_point and data buffer that includes IE id and len
     * fields. More than one IE may be included in the request. Setting the generic
     * IE to empty buffer (len=0) removes the generic IE from the driver. Drivers
     * are allowed to generate their own WPA/RSN IEs, but in these cases, drivers
     * are required to report the used IE as a wireless event, e.g., when
     * associating with an AP. */
    [SIOCSIWGENIE-SIOCIWFIRST]	= "SIOCSIWGENIE",		/* set generic IE */
    [SIOCGIWGENIE-SIOCIWFIRST]	= "SIOCGIWGENIE",		/* get generic IE */
    /* WPA : IEEE 802.11 MLME requests */
    [SIOCSIWMLME-SIOCIWFIRST]	= "SIOCSIWMLME",		/* request MLME operation; uses struct iw_mlme */
    /* WPA : Authentication mode parameters */
    [SIOCSIWAUTH-SIOCIWFIRST]	= "SIOCSIWAUTH",		/* set authentication mode params */
    [SIOCGIWAUTH-SIOCIWFIRST]	= "SIOCGIWAUTH",		/* get authentication mode params */
    /* WPA : Extended version of encoding configuration */
    [SIOCSIWENCODEEXT-SIOCIWFIRST] = "SIOCSIWENCODEEXT",		/* set encoding token & mode */
    [SIOCGIWENCODEEXT-SIOCIWFIRST] = "SIOCGIWENCODEEXT",		/* get encoding token & mode */
    /* WPA2 : PMKSA cache management */
    [SIOCSIWPMKSA-SIOCIWFIRST]	= "SIOCSIWPMKSA",		/* PMKSA cache operation */

    /* -------------------- DEV PRIVATE IOCTL LIST -------------------- */

    /* These 32 ioctl are wireless device private, for 16 commands.
     * Each driver is free to use them for whatever purpose it chooses,
     * however the driver *must* export the description of those ioctls
     * with SIOCGIWPRIV and *must* use arguments as defined below.
     * If you don't follow those rules, DaveM is going to hate you (reason :
     * it make mixed 32/64bit operation impossible).
     */
    [SIOCIWFIRSTPRIV-SIOCIWFIRST]	= "SIOCIWFIRSTPRIV",
    [SIOCIWLASTPRIV-SIOCIWFIRST]	= "SIOCIWLASTPRIV",
    /* Previously, we were using SIOCDEVPRIVATE, but we now have our
     * separate range because of collisions with other tools such as
     * 'mii-tool'.
     * We now have 32 commands, so a bit more space ;-).
     * Also, all 'even' commands are only usable by root and don't return the
     * content of ifr/iwr to user (but you are not obliged to use the set/get
     * convention, just use every other two command). More details in iwpriv.c.
     * And I repeat : you are not forced to use them with iwpriv, but you
     * must be compliant with it.
     */

    /* ----------------------- WIRELESS EVENTS ----------------------- */
    /* Those are *NOT* ioctls, do not issue request on them !!! */
    /* Most events use the same identifier as ioctl requests */

    [IWEVTXDROP-SIOCIWFIRST]	= "IWEVTXDROP",		/* Packet dropped to excessive retry */
    [IWEVQUAL-SIOCIWFIRST]	= "IWEVQUAL",		/* Quality part of statistics (scan) */
    [IWEVCUSTOM-SIOCIWFIRST]	= "IWEVCUSTOM",		/* Driver specific ascii string */
    [IWEVREGISTERED-SIOCIWFIRST]= "IWEVREGISTERED",	/* Discovered a new node (AP mode) */
    [IWEVEXPIRED-SIOCIWFIRST]	= "IWEVEXPIRED",	/* Expired a node (AP mode) */
    [IWEVGENIE-SIOCIWFIRST]	= "IWEVGENIE",		/* Generic IE (WPA, RSN, WMM, ..) (scan results); This includes id and
                                                 * length fields. One IWEVGENIE may contain more than one IE. Scan
                                                 * results may contain one or more IWEVGENIE events. */
    [IWEVMICHAELMICFAILURE-SIOCIWFIRST] = "IWEVMICHAELMICFAILURE",	/* Michael MIC failure (struct iw_michaelmicfailure) */
    [IWEVASSOCREQIE-SIOCIWFIRST]	= "IWEVASSOCREQIE",		/* IEs used in (Re)Association Request.
                                             * The data includes id and length
                                             * fields and may contain more than one
                                             * IE. This event is required in
                                             * Managed mode if the driver
                                             * generates its own WPA/RSN IE. This
                                             * should be sent just before
                                             * IWEVREGISTERED event for the
                                             * association. */
    [IWEVASSOCRESPIE-SIOCIWFIRST]	= "IWEVASSOCRESPIE",		/* IEs used in (Re)Association
                                             * Response. The data includes id and
                                             * length fields and may contain more
                                             * than one IE. This may be sent
                                             * between IWEVASSOCREQIE and
                                             * IWEVREGISTERED events for the
                                             * association. */
    [IWEVPMKIDCAND-SIOCIWFIRST]		= "IWEVPMKIDCAND",		/* PMKID candidate for RSN
                                             * pre-authentication
                                             * (struct iw_pmkid_cand) */
};

/* WLAN IE: Information Elements */

struct wlan_ie {
    u8 id;
    u8 len;
    u8 data[];
} STRUCT_PACKED;

struct wlan_ies { // struct iw_point without void *pointer;
    u16 len; /* number of fields or size in bytes */
    u16 flags; /* Optional params */
    struct wlan_ie ies[];
};

/* Information Element IDs (IEEE Std 802.11-2016, 9.4.2.1, Table 9-77) */
/* see aircrack-ng/third-party/ieee80211.h and hostap-git/src/common/ieee802_11_defs.h */

static const char wlan_eid_str[][40] = { // max len(EID_VHT_OPERATING_MODE_NOTIFICATION) = 36
    [WLAN_EID_SSID] = "EID_SSID",
    [WLAN_EID_SUPP_RATES] = "EID_SUPP_RATES",
    [WLAN_EID_DS_PARAMS] = "EID_DS_PARAMS",
    [WLAN_EID_CF_PARAMS] = "EID_CF_PARAMS",
    [WLAN_EID_TIM] = "EID_TIM",
    [WLAN_EID_IBSS_PARAMS] = "EID_IBSS_PARAMS",
    [WLAN_EID_COUNTRY] = "EID_COUNTRY",
    [WLAN_EID_REQUEST] = "EID_REQUEST",
    [WLAN_EID_BSS_LOAD] = "EID_BSS_LOAD",
    [WLAN_EID_EDCA_PARAM_SET] = "EID_EDCA_PARAM_SET",
    [WLAN_EID_TSPEC] = "EID_TSPEC",
    [WLAN_EID_TCLAS] = "EID_TCLAS",
    [WLAN_EID_SCHEDULE] = "EID_SCHEDULE",
    [WLAN_EID_CHALLENGE] = "EID_CHALLENGE",
    [WLAN_EID_CHALLENGE+1] = "EID_CHALLENGE17", /* 17-31 reserved for challenge text extension */
    [WLAN_EID_CHALLENGE+2] = "EID_CHALLENGE18",
    [WLAN_EID_CHALLENGE+3] = "EID_CHALLENGE19",
    [WLAN_EID_CHALLENGE+4] = "EID_CHALLENGE20",
    [WLAN_EID_CHALLENGE+5] = "EID_CHALLENGE21",
    [WLAN_EID_CHALLENGE+6] = "EID_CHALLENGE22",
    [WLAN_EID_CHALLENGE+7] = "EID_CHALLENGE23",
    [WLAN_EID_CHALLENGE+8] = "EID_CHALLENGE24",
    [WLAN_EID_CHALLENGE+9] = "EID_CHALLENGE25",
    [WLAN_EID_CHALLENGE+10] = "EID_CHALLENGE26",
    [WLAN_EID_CHALLENGE+11] = "EID_CHALLENGE27",
    [WLAN_EID_CHALLENGE+12] = "EID_CHALLENGE28",
    [WLAN_EID_CHALLENGE+13] = "EID_CHALLENGE29",
    [WLAN_EID_CHALLENGE+14] = "EID_CHALLENGE30",
    [WLAN_EID_CHALLENGE+15] = "EID_CHALLENGE31",
    [WLAN_EID_PWR_CONSTRAINT] = "EID_PWR_CONSTRAINT",
    [WLAN_EID_PWR_CAPABILITY] = "EID_PWR_CAPABILITY",
    [WLAN_EID_TPC_REQUEST] = "EID_TPC_REQUEST",
    [WLAN_EID_TPC_REPORT] = "EID_TPC_REPORT",
    [WLAN_EID_SUPPORTED_CHANNELS] = "EID_SUPPORTED_CHANNELS",
    [WLAN_EID_CHANNEL_SWITCH] = "EID_CHANNEL_SWITCH",
    [WLAN_EID_MEASURE_REQUEST] = "EID_MEASURE_REQUEST",
    [WLAN_EID_MEASURE_REPORT] = "EID_MEASURE_REPORT",
    [WLAN_EID_QUIET] = "EID_QUIET",
    [WLAN_EID_IBSS_DFS] = "EID_IBSS_DFS",
    [WLAN_EID_ERP_INFO] = "EID_ERP_INFO",
    [WLAN_EID_TS_DELAY] = "EID_TS_DELAY",
    [WLAN_EID_TCLAS_PROCESSING] = "EID_TCLAS_PROCESSING",
    [WLAN_EID_HT_CAP] = "EID_HT_CAP",
    [WLAN_EID_QOS] = "EID_QOS",
    [WLAN_EID_RSN] = "EID_RSN",
    [WLAN_EID_EXT_SUPP_RATES] = "EID_EXT_SUPP_RATES",
    [WLAN_EID_AP_CHANNEL_REPORT] = "EID_AP_CHANNEL_REPORT",
    [WLAN_EID_NEIGHBOR_REPORT] = "EID_NEIGHBOR_REPORT",
    [WLAN_EID_RCPI] = "EID_RCPI",
    [WLAN_EID_MOBILITY_DOMAIN] = "EID_MOBILITY_DOMAIN",
    [WLAN_EID_FAST_BSS_TRANSITION] = "EID_FAST_BSS_TRANSITION",
    [WLAN_EID_TIMEOUT_INTERVAL] = "EID_TIMEOUT_INTERVAL",
    [WLAN_EID_RIC_DATA] = "EID_RIC_DATA",
    [WLAN_EID_DSE_REGISTERED_LOCATION] = "EID_DSE_REGISTERED_LOCATION",
    [WLAN_EID_SUPPORTED_OPERATING_CLASSES] = "EID_SUPPORTED_OPERATING_CLASSES",
    [WLAN_EID_EXT_CHANSWITCH_ANN] = "EID_EXT_CHANSWITCH_ANN",
    [WLAN_EID_HT_OPERATION] = "EID_HT_OPERATION",
    [WLAN_EID_SECONDARY_CHANNEL_OFFSET] = "EID_SECONDARY_CHANNEL_OFFSET",
    [WLAN_EID_BSS_AVERAGE_ACCESS_DELAY] = "EID_BSS_AVERAGE_ACCESS_DELAY",
    [WLAN_EID_ANTENNA] = "EID_ANTENNA",
    [WLAN_EID_RSNI] = "EID_RSNI",
    [WLAN_EID_MEASUREMENT_PILOT_TRANSMISSION] = "EID_MEASUREMENT_PILOT_TRANSMISSION",
    [WLAN_EID_BSS_AVAILABLE_ADM_CAPA] = "EID_BSS_AVAILABLE_ADM_CAPA",
    [WLAN_EID_BSS_AC_ACCESS_DELAY] = "EID_BSS_AC_ACCESS_DELAY", /* note: also used by WAPI */
    [WLAN_EID_TIME_ADVERTISEMENT] = "EID_TIME_ADVERTISEMENT",
    [WLAN_EID_RRM_ENABLED_CAPABILITIES] = "EID_RRM_ENABLED_CAPABILITIES",
    [WLAN_EID_MULTIPLE_BSSID] = "EID_MULTIPLE_BSSID",
    [WLAN_EID_20_40_BSS_COEXISTENCE] = "EID_20_40_BSS_COEXISTENCE",
    [WLAN_EID_20_40_BSS_INTOLERANT] = "EID_20_40_BSS_INTOLERANT",
    [WLAN_EID_OVERLAPPING_BSS_SCAN_PARAMS] = "EID_OVERLAPPING_BSS_SCAN_PARAMS",
    [WLAN_EID_RIC_DESCRIPTOR] = "EID_RIC_DESCRIPTOR",
    [WLAN_EID_MMIE] = "EID_MMIE",
    [WLAN_EID_EVENT_REQUEST] = "EID_EVENT_REQUEST",
    [WLAN_EID_EVENT_REPORT] = "EID_EVENT_REPORT",
    [WLAN_EID_DIAGNOSTIC_REQUEST] = "EID_DIAGNOSTIC_REQUEST",
    [WLAN_EID_DIAGNOSTIC_REPORT] = "EID_DIAGNOSTIC_REPORT",
    [WLAN_EID_LOCATION_PARAMETERS] = "EID_LOCATION_PARAMETERS",
    [WLAN_EID_NONTRANSMITTED_BSSID_CAPA] = "EID_NONTRANSMITTED_BSSID_CAPA",
    [WLAN_EID_SSID_LIST] = "EID_SSID_LIST",
    [WLAN_EID_MULTIPLE_BSSID_INDEX] = "EID_MULTIPLE_BSSID_INDEX",
    [WLAN_EID_FMS_DESCRIPTOR] = "EID_FMS_DESCRIPTOR",
    [WLAN_EID_FMS_REQUEST] = "EID_FMS_REQUEST",
    [WLAN_EID_FMS_RESPONSE] = "EID_FMS_RESPONSE",
    [WLAN_EID_QOS_TRAFFIC_CAPABILITY] = "EID_QOS_TRAFFIC_CAPABILITY",
    [WLAN_EID_BSS_MAX_IDLE_PERIOD] = "EID_BSS_MAX_IDLE_PERIOD",
    [WLAN_EID_TFS_REQ] = "EID_TFS_REQ",
    [WLAN_EID_TFS_RESP] = "EID_TFS_RESP",
    [WLAN_EID_WNMSLEEP] = "EID_WNMSLEEP",
    [WLAN_EID_TIM_BROADCAST_REQUEST] = "EID_TIM_BROADCAST_REQUEST",
    [WLAN_EID_TIM_BROADCAST_RESPONSE] = "EID_TIM_BROADCAST_RESPONSE",
    [WLAN_EID_COLLOCATED_INTERFERENCE_REPORT] = "EID_COLLOCATED_INTERFERENCE_REPORT",
    [WLAN_EID_CHANNEL_USAGE] = "EID_CHANNEL_USAGE",
    [WLAN_EID_TIME_ZONE] = "EID_TIME_ZONE",
    [WLAN_EID_DMS_REQUEST] = "EID_DMS_REQUEST",
    [WLAN_EID_DMS_RESPONSE] = "EID_DMS_RESPONSE",
    [WLAN_EID_LINK_ID] = "EID_LINK_ID",
    [WLAN_EID_WAKEUP_SCHEDULE] = "EID_WAKEUP_SCHEDULE",
    [WLAN_EID_CHANNEL_SWITCH_TIMING] = "EID_CHANNEL_SWITCH_TIMING",
    [WLAN_EID_PTI_CONTROL] = "EID_PTI_CONTROL",
    [WLAN_EID_TPU_BUFFER_STATUS] = "EID_TPU_BUFFER_STATUS",
    [WLAN_EID_INTERWORKING] = "EID_INTERWORKING",
    [WLAN_EID_ADV_PROTO] = "EID_ADV_PROTO",
    [WLAN_EID_EXPEDITED_BANDWIDTH_REQ] = "EID_EXPEDITED_BANDWIDTH_REQ",
    [WLAN_EID_QOS_MAP_SET] = "EID_QOS_MAP_SET",
    [WLAN_EID_ROAMING_CONSORTIUM] = "EID_ROAMING_CONSORTIUM",
    [WLAN_EID_EMERGENCY_ALERT_ID] = "EID_EMERGENCY_ALERT_ID",
    [WLAN_EID_MESH_CONFIG] = "EID_MESH_CONFIG",
    [WLAN_EID_MESH_ID] = "EID_MESH_ID",
    [WLAN_EID_MESH_LINK_METRIC_REPORT] = "EID_MESH_LINK_METRIC_REPORT",
    [WLAN_EID_CONGESTION_NOTIFICATION] = "EID_CONGESTION_NOTIFICATION",
    [WLAN_EID_PEER_MGMT] = "EID_PEER_MGMT",
    [WLAN_EID_MESH_CHANNEL_SWITCH_PARAMETERS] = "EID_MESH_CHANNEL_SWITCH_PARAMETERS",
    [WLAN_EID_MESH_AWAKE_WINDOW] = "EID_MESH_AWAKE_WINDOW",
    [WLAN_EID_BEACON_TIMING] = "EID_BEACON_TIMING",
    [WLAN_EID_MCCAOP_SETUP_REQUEST] = "EID_MCCAOP_SETUP_REQUEST",
    [WLAN_EID_MCCAOP_SETUP_REPLY] = "EID_MCCAOP_SETUP_REPLY",
    [WLAN_EID_MCCAOP_ADVERTISEMENT] = "EID_MCCAOP_ADVERTISEMENT",
    [WLAN_EID_MCCAOP_TEARDOWN] = "EID_MCCAOP_TEARDOWN",
    [WLAN_EID_GANN] = "EID_GANN",
    [WLAN_EID_RANN] = "EID_RANN",
    [WLAN_EID_EXT_CAPAB] = "EID_EXT_CAPAB",
    [WLAN_EID_PREQ] = "EID_PREQ",
    [WLAN_EID_PREP] = "EID_PREP",
    [WLAN_EID_PERR] = "EID_PERR",
    [WLAN_EID_PXU] = "EID_PXU",
    [WLAN_EID_PXUC] = "EID_PXUC",
    [WLAN_EID_AMPE] = "EID_AMPE",
    [WLAN_EID_MIC] = "EID_MIC",
    [WLAN_EID_DESTINATION_URI] = "EID_DESTINATION_URI",
    [WLAN_EID_U_APSD_COEX] = "EID_U_APSD_COEX",
    [WLAN_EID_DMG_WAKEUP_SCHEDULE] = "EID_DMG_WAKEUP_SCHEDULE",
    [WLAN_EID_EXTENDED_SCHEDULE] = "EID_EXTENDED_SCHEDULE",
    [WLAN_EID_STA_AVAILABILITY] = "EID_STA_AVAILABILITY",
    [WLAN_EID_DMG_TSPEC] = "EID_DMG_TSPEC",
    [WLAN_EID_NEXT_DMG_ATI] = "EID_NEXT_DMG_ATI",
    [WLAN_EID_DMG_CAPABILITIES] = "EID_DMG_CAPABILITIES",
    [WLAN_EID_DMG_OPERATION] = "EID_DMG_OPERATION",
    [WLAN_EID_DMG_BSS_PARAMETER_CHANGE] = "EID_DMG_BSS_PARAMETER_CHANGE",
    [WLAN_EID_DMG_BEAM_REFINEMENT] = "EID_DMG_BEAM_REFINEMENT",
    [WLAN_EID_CHANNEL_MEASUREMENT_FEEDBACK] = "EID_CHANNEL_MEASUREMENT_FEEDBACK",
    [WLAN_EID_CCKM] = "EID_CCKM",
    [WLAN_EID_AWAKE_WINDOW] = "EID_AWAKE_WINDOW",
    [WLAN_EID_MULTI_BAND] = "EID_MULTI_BAND",
    [WLAN_EID_ADDBA_EXTENSION] = "EID_ADDBA_EXTENSION",
    [WLAN_EID_NEXTPCP_LIST] = "EID_NEXTPCP_LIST",
    [WLAN_EID_PCP_HANDOVER] = "EID_PCP_HANDOVER",
    [WLAN_EID_DMG_LINK_MARGIN] = "EID_DMG_LINK_MARGIN",
    [WLAN_EID_SWITCHING_STREAM] = "EID_SWITCHING_STREAM",
    [WLAN_EID_SESSION_TRANSITION] = "EID_SESSION_TRANSITION",
    [WLAN_EID_DYNAMIC_TONE_PAIRING_REPORT] = "EID_DYNAMIC_TONE_PAIRING_REPORT",
    [WLAN_EID_CLUSTER_REPORT] = "EID_CLUSTER_REPORT",
    [WLAN_EID_REPLAY_CAPABILITIES] = "EID_REPLAY_CAPABILITIES",
    [WLAN_EID_RELAY_TRANSFER_PARAM_SET] = "EID_RELAY_TRANSFER_PARAM_SET",
    [WLAN_EID_BEAMLINK_MAINTENANCE] = "EID_BEAMLINK_MAINTENANCE",
    [WLAN_EID_MULTIPLE_MAC_SUBLAYERS] = "EID_MULTIPLE_MAC_SUBLAYERS",
    [WLAN_EID_U_PID] = "EID_U_PID",
    [WLAN_EID_DMG_LINK_ADAPTATION_ACK] = "EID_DMG_LINK_ADAPTATION_ACK",
    [WLAN_EID_MCCAOP_ADVERTISEMENT_OVERVIEW] = "EID_MCCAOP_ADVERTISEMENT_OVERVIEW",
    [WLAN_EID_QUIET_PERIOD_REQUEST] = "EID_QUIET_PERIOD_REQUEST",
    [WLAN_EID_QUIET_PERIOD_RESPONSE] = "EID_QUIET_PERIOD_RESPONSE",
    [WLAN_EID_QMF_POLICY] = "EID_QMF_POLICY",
    [WLAN_EID_ECAPC_POLICY] = "EID_ECAPC_POLICY",
    [WLAN_EID_CLUSTER_TIME_OFFSET] = "EID_CLUSTER_TIME_OFFSET",
    [WLAN_EID_INTRA_ACCESS_CATEGORY_PRIORITY] = "EID_INTRA_ACCESS_CATEGORY_PRIORITY",
    [WLAN_EID_SCS_DESCRIPTOR] = "EID_SCS_DESCRIPTOR",
    [WLAN_EID_QLOAD_REPORT] = "EID_QLOAD_REPORT",
    [WLAN_EID_HCCA_TXOP_UPDATE_COUNT] = "EID_HCCA_TXOP_UPDATE_COUNT",
    [WLAN_EID_HIGHER_LAYER_STREAM_ID] = "EID_HIGHER_LAYER_STREAM_ID",
    [WLAN_EID_GCR_GROUP_ADDRESS] = "EID_GCR_GROUP_ADDRESS",
    [WLAN_EID_ANTENNA_SECTOR_ID_PATTERN] = "EID_ANTENNA_SECTOR_ID_PATTERN",
    [WLAN_EID_VHT_CAP] = "EID_VHT_CAP",
    [WLAN_EID_VHT_OPERATION] = "EID_VHT_OPERATION",
    [WLAN_EID_VHT_EXTENDED_BSS_LOAD] = "EID_VHT_EXTENDED_BSS_LOAD",
    [WLAN_EID_VHT_WIDE_BW_CHSWITCH] = "EID_VHT_WIDE_BW_CHSWITCH",
    [WLAN_EID_VHT_TRANSMIT_POWER_ENVELOPE] = "EID_VHT_TRANSMIT_POWER_ENVELOPE",
    [WLAN_EID_VHT_CHANNEL_SWITCH_WRAPPER] = "EID_VHT_CHANNEL_SWITCH_WRAPPER",
    [WLAN_EID_VHT_AID] = "EID_VHT_AID",
    [WLAN_EID_VHT_QUIET_CHANNEL] = "EID_VHT_QUIET_CHANNEL",
    [WLAN_EID_VHT_OPERATING_MODE_NOTIFICATION] = "EID_VHT_OPERATING_MODE_NOTIFICATION",
    [WLAN_EID_UPSIM] = "EID_UPSIM",
    [WLAN_EID_REDUCED_NEIGHBOR_REPORT] = "EID_REDUCED_NEIGHBOR_REPORT",
    [WLAN_EID_TVHT_OPERATION] = "EID_TVHT_OPERATION",
    [WLAN_EID_DEVICE_LOCATION] = "EID_DEVICE_LOCATION",
    [WLAN_EID_WHITE_SPACE_MAP] = "EID_WHITE_SPACE_MAP",
    [WLAN_EID_FTM_PARAMETERS] = "EID_FTM_PARAMETERS",
    [WLAN_EID_VENDOR_SPECIFIC] = "EID_VENDOR_SPECIFIC",
    [WLAN_EID_CAG_NUMBER] = "EID_CAG_NUMBER",
    [WLAN_EID_AP_CSN] = "EID_AP_CSN",
    [WLAN_EID_FILS_INDICATION] = "EID_FILS_INDICATION",
    [WLAN_EID_DILS] = "EID_DILS",
    [WLAN_EID_FRAGMENT] = "EID_FRAGMENT",
    [WLAN_EID_RSNX] = "EID_RSNX",
    [WLAN_EID_EXTENSION] = "EID_EXTENSION",
};

static const char wlan_eid_ext_str[][40] = { // max len(EID_VHT_OPERATING_MODE_NOTIFICATION) = 36
    /* Element ID Extension (EID 255) values */
    [WLAN_EID_EXT_ASSOC_DELAY_INFO] = "EID_EXT_ASSOC_DELAY_INFO",
    [WLAN_EID_EXT_FILS_REQ_PARAMS] = "EID_EXT_FILS_REQ_PARAMS",
    [WLAN_EID_EXT_FILS_KEY_CONFIRM] = "EID_EXT_FILS_KEY_CONFIRM",
    [WLAN_EID_EXT_FILS_SESSION] = "EID_EXT_FILS_SESSION",
    [WLAN_EID_EXT_FILS_HLP_CONTAINER] = "EID_EXT_FILS_HLP_CONTAINER",
    [WLAN_EID_EXT_FILS_IP_ADDR_ASSIGN] = "EID_EXT_FILS_IP_ADDR_ASSIGN",
    [WLAN_EID_EXT_KEY_DELIVERY] = "EID_EXT_KEY_DELIVERY",
    [WLAN_EID_EXT_WRAPPED_DATA] = "EID_EXT_WRAPPED_DATA",
    [WLAN_EID_EXT_FTM_SYNC_INFO] = "EID_EXT_FTM_SYNC_INFO",
    [WLAN_EID_EXT_EXTENDED_REQUEST] = "EID_EXT_EXTENDED_REQUEST",
    [WLAN_EID_EXT_ESTIMATED_SERVICE_PARAMS] = "EID_EXT_ESTIMATED_SERVICE_PARAMS",
    [WLAN_EID_EXT_FILS_PUBLIC_KEY] = "EID_EXT_FILS_PUBLIC_KEY",
    [WLAN_EID_EXT_FILS_NONCE] = "EID_EXT_FILS_NONCE",
    [WLAN_EID_EXT_FUTURE_CHANNEL_GUIDANCE] = "EID_EXT_FUTURE_CHANNEL_GUIDANCE",
    [WLAN_EID_EXT_OWE_DH_PARAM] = "EID_EXT_OWE_DH_PARAM",
    [WLAN_EID_EXT_PASSWORD_IDENTIFIER] = "EID_EXT_PASSWORD_IDENTIFIER",
    [WLAN_EID_EXT_HE_CAPABILITIES] = "EID_EXT_HE_CAPABILITIES",
    [WLAN_EID_EXT_HE_OPERATION] = "EID_EXT_HE_OPERATION",
    [WLAN_EID_EXT_HE_MU_EDCA_PARAMS] = "EID_EXT_HE_MU_EDCA_PARAMS",
    [WLAN_EID_EXT_SPATIAL_REUSE] = "EID_EXT_SPATIAL_REUSE",
    [WLAN_EID_EXT_OCV_OCI] = "EID_EXT_OCV_OCI",
    [WLAN_EID_EXT_SHORT_SSID_LIST] = "EID_EXT_SHORT_SSID_LIST",
    [WLAN_EID_EXT_HE_6GHZ_BAND_CAP] = "EID_EXT_HE_6GHZ_BAND_CAP",
    [WLAN_EID_EXT_EDMG_CAPABILITIES] = "EID_EXT_EDMG_CAPABILITIES",
    [WLAN_EID_EXT_EDMG_OPERATION] = "EID_EXT_EDMG_OPERATION",
    [WLAN_EID_EXT_MSCS_DESCRIPTOR] = "EID_EXT_MSCS_DESCRIPTOR",
    [WLAN_EID_EXT_TCLAS_MASK] = "EID_EXT_TCLAS_MASK",
    [WLAN_EID_EXT_REJECTED_GROUPS] = "EID_EXT_REJECTED_GROUPS",
    [WLAN_EID_EXT_ANTI_CLOGGING_TOKEN] = "EID_EXT_ANTI_CLOGGING_TOKEN",
};

static void print_rta_addr(int family, const struct rtattr *rta)
{
    if (!family && RTA_PAYLOAD(rta) == 6)
        family = AF_PACKET;

    switch (family) {
    case AF_PACKET: {
        unsigned char *x = RTA_DATA(rta);
        outf(" %02x:%02x:%02x:%02x:%02x:%02x", x[0], x[1], x[2], x[3], x[4], x[5]);
        break;
    }
    case AF_INET:
    case AF_INET6: {
        char host[INET6_ADDRSTRLEN];
        outf(" %s", inet_ntop(family, RTA_DATA(rta), host, sizeof(host)));
        break;
    }
    default:
        outf(" AF_unexpected_family_%d\n", family);
        dump(RTA_DATA(rta), RTA_PAYLOAD(rta), 0, "\t");
        break;
    }
}

static void print_iff(int flags)
{
    outf(" ");
    if (!flags) {
        outf("-");
        return;
    }
    int first = 1;
    for (int i = 0; ifa_flags_descr[i].flag; i++) {
        if (flags & ifa_flags_descr[i].flag) {
            outf("%s%s", first ? "" : "|", ifa_flags_descr[i].name);
            first = 0;
        }
        flags &= ~ifa_flags_descr[i].flag;
    }
    if (flags) outf("|%#x", flags);
}

/*
 * Compare two ethernet addresses
 */

static int iw_ether_cmp(const struct ether_addr* eth1, const struct ether_addr* eth2)
{
    return memcmp(eth1, eth2, sizeof(*eth1));
}

/*
 * Display an Wireless Access Point Socket Address in readable format.
 * Note : 0x44 is an accident of history, that's what the Orinoco/PrismII
 * chipset report, and the driver doesn't filter it.
 */

static void print_ap_addr(const struct sockaddr *sap)
{
    const struct ether_addr ether_zero = {{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
    const struct ether_addr ether_bcast = {{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }};
    const struct ether_addr ether_hack = {{ 0x44, 0x44, 0x44, 0x44, 0x44, 0x44 }};
    const struct ether_addr *ap = (const struct ether_addr *) sap->sa_data;

    if (!iw_ether_cmp(ap, &ether_zero))
        outf("Not-Associated");
    else if (!iw_ether_cmp(ap, &ether_bcast))
        outf("Invalid");
    else if (!iw_ether_cmp(ap, &ether_hack))
        outf("None");
    else
        outf("%02x:%02x:%02x:%02x:%02x:%02x",
            ap->ether_addr_octet[0], ap->ether_addr_octet[1],
            ap->ether_addr_octet[2], ap->ether_addr_octet[3],
            ap->ether_addr_octet[4], ap->ether_addr_octet[5]);
}

int nl_debug(void *buf, int len)
{
    if (len < 0) {
        outf("netlink_read: len=%d - %s\n", len, strerror(errno));
        return len;
    }
    if (len == 0) {
        outf("netlink_read: EOF\n");
        return -1;
    }
    if (len < NLMSG_HDRLEN) {
        outf("netlink_read: short read (%d)\n", len);
        return 0;
    }

    //dump(&buf, len, 0, "\t");

    for (struct nlmsghdr *nh = buf; NLMSG_OK(nh, len); nh = NLMSG_NEXT(nh, len)) {
        if (0) {
            outf("--- Netlink message %d bytes, flags %#x %s\n", nh->nlmsg_len, nh->nlmsg_flags & ~NLM_F_MULTI,
                (nh->nlmsg_flags & NLM_F_MULTI) ? "[multi]" : "");
        }

        if (nh->nlmsg_type == NLMSG_DONE) {
            //done = 1;
            break;

        } else if (nh->nlmsg_type == NLMSG_ERROR) {
            // struct nlmsgerr {
            //     int error;        /* Negative errno or 0 for acknowledgements */
            //     struct nlmsghdr msg;  /* Message header that caused the error */
            // };
            struct nlmsgerr *err = (struct nlmsgerr *) NLMSG_DATA(nh);
            if (err->error == 0) {
                outf("(ACK)\n");
            } else {
                errno = -err->error;
                outf("netlink_read: %s, caused by nlmsghdr { type %s, len %u, flags %#x, seq %u, pid %u }\n",
                    strerror(errno),
                    nlmsg_type_str[err->msg.nlmsg_type], /* Type of message content */
                    err->msg.nlmsg_len, /* Length of message including header */
                    err->msg.nlmsg_flags, /* Additional flags */
                    err->msg.nlmsg_seq, /* Sequence number */
                    err->msg.nlmsg_pid /* Sender port ID */
                );
            }

        } else if (nh->nlmsg_type == RTM_NEWLINK || nh->nlmsg_type == RTM_DELLINK) {
            // struct ifinfomsg {
            //     unsigned char  ifi_family; /* AF_UNSPEC */
            //     unsigned short ifi_type;   /* Device type */
            //     int            ifi_index;  /* Interface index */
            //     unsigned int   ifi_flags;  /* Device flags  */
            //     unsigned int   ifi_change; /* change mask */
            // };
            struct ifinfomsg *ifm = (struct ifinfomsg *) NLMSG_DATA(nh);
            size_t rta_len = IFLA_PAYLOAD(nh); //NLMSG_PAYLOAD(nh, sizeof(*ifm));

            outf("%s interface ifindex %u, type %hu, family %d, flags %#x, change %#x,",
                nh->nlmsg_type == RTM_NEWLINK ? "+" : "-", ifm->ifi_index, ifm->ifi_type,
                ifm->ifi_family, ifm->ifi_flags, ifm->ifi_change);

            for (struct rtattr *rta = IFLA_RTA(ifm); RTA_OK(rta, rta_len); rta = RTA_NEXT(rta, rta_len)) {
                outf(" %s", ifla_rta_type_str[rta->rta_type]);
                switch (rta->rta_type) {
                // not implemented
                case IFLA_UNSPEC:
                case IFLA_AF_SPEC:
                case IFLA_STATS64:
                case IFLA_STATS:
                case IFLA_MAP:
                case IFLA_XDP:
                case IFLA_LINKINFO:
                    outf(" -");
                    break;
                // mac
                case IFLA_ADDRESS:
                case IFLA_BROADCAST:
                case IFLA_PERM_ADDRESS:
                    print_rta_addr(ifm->ifi_family, rta);
                    break;
                // asciiz
                case IFLA_IFNAME:
                case IFLA_QDISC: {
                    const char *val = RTA_DATA(rta);
                    outf(" %s", val);
                    break;
                }
                // unsigned char
                case IFLA_OPERSTATE:
                case IFLA_LINKMODE:
                case IFLA_CARRIER:
                case IFLA_PROTO_DOWN: {
                    const unsigned char *val = RTA_DATA(rta);
                    outf(" %u", *val);
                    break;
                }
                // unsigned int
                case IFLA_TXQLEN:
                case IFLA_MTU:
                case IFLA_MIN_MTU:
                case IFLA_MAX_MTU:
                case IFLA_GROUP:
                case IFLA_PROMISCUITY:
                case IFLA_NUM_TX_QUEUES:
                case IFLA_GSO_MAX_SEGS:
                case IFLA_GSO_MAX_SIZE:
                case IFLA_NUM_RX_QUEUES:
                case IFLA_CARRIER_CHANGES:
                case IFLA_CARRIER_UP_COUNT:
                case IFLA_CARRIER_DOWN_COUNT: {
                    const unsigned *val = RTA_DATA(rta);
                    outf(" %u", *val);
                    break;
                }
                // int
                case IFLA_LINK: {
                    const int *val = RTA_DATA(rta);
                    outf(" %d", *val);
                    break;
                }
                case IFLA_WIRELESS: {
                    // struct iw_event {
                    //     __u16 len;                      /* Real length of this stuff (with header) */
                    //     __u16 cmd;                      /* Wireless IOCTL */
                    //     union iwreq_data u;             /* IOCTL fixed payload */
                    // };
                    // union iwreq_data {
                    //     char            name[IFNAMSIZ]; /* Name : used to verify the presence of  wireless extensions. Name of the protocol/provider... */
                    //     struct iw_point essid;          /* Extended network name */
                    //     struct iw_param nwid;           /* network id (or domain - the cell) */
                    //     struct iw_freq  freq;           /* frequency or channel : * 0-1000 = channel * > 1000 = frequency in Hz */
                    //     struct iw_param sens;           /* signal level threshold */
                    //     struct iw_param bitrate;        /* default bit rate */
                    //     struct iw_param txpower;        /* default transmit power */
                    //     struct iw_param rts;            /* RTS threshold */
                    //     struct iw_param frag;           /* Fragmentation threshold */
                    //     __u32           mode;           /* Operation mode */
                    //     struct iw_param retry;          /* Retry limits & lifetime */
                    //     struct iw_point encoding;       /* Encoding stuff : tokens */
                    //     struct iw_param power;          /* PM duration/timeout */
                    //     struct iw_quality qual;         /* Quality part of statistics */
                    //     struct sockaddr ap_addr;        /* Access point address */
                    //     struct sockaddr addr;           /* Destination address (hw/mac) */
                    //     struct iw_param param;          /* Other small parameters */
                    //     struct iw_point data;           /* Other large parameters */
                    // };
                    // struct iw_point {
                    //     void *pointer; /* Pointer to the data  (in user space) */
                    //     __u16 length;  /* number of fields or size in bytes (without header) */
                    //     __u16 flags;   /* Optional params */
                    // };

                    struct iw_event *iw = RTA_DATA(rta);
                    for (int iwlen = RTA_PAYLOAD(rta); iwlen > 0; iwlen -= iw->len,
                            iw = (struct iw_event *) ((u8*)iw + iw->len)) {
                        const char *iwcmd = iw_event_cmd_str[iw->cmd - SIOCIWFIRST];

                        switch (iw->cmd) {
                        // unsigned int
                        case SIOCGIWSCAN: {
                            outf(" %s:%u", iwcmd, iw->u.mode);
                            break;
                        }
                        // hex
                        case SIOCGIWAP: { // [16]
                            outf(" AP:");
                            print_ap_addr(&iw->u.ap_addr);
                            //dump_hex(&iw->u, iwlen - sizeof(*iw) + sizeof(iw->u));
                            break;
                        }
                        // struct wlan_ies (struct iw_point without pointer)
                        case IWEVMICHAELMICFAILURE:
                        case IWEVCUSTOM:
                        case IWEVASSOCREQIE:
                        case IWEVASSOCRESPIE:
                        case IWEVPMKIDCAND: {
                            outf(" %s:", iwcmd);
                            struct wlan_ies *ies = (struct wlan_ies *) &iw->u.data;
                            struct wlan_ie *ie = ies->ies;
                            for (int ieslen = ies->len; ieslen > 0; ieslen -= sizeof(*ie) + ie->len,
                                    ie = (struct wlan_ie *) (ie->data + ie->len)) {
                                const char *ieid = wlan_eid_str[ie->id];
                                switch (ie->id) {
                                // ascii
                                case WLAN_EID_SSID:
                                    if (ie->len) outf(" %s=\"%.*s\"", ieid, ie->len, ie->data);
                                    break;
                                // hex
                                case WLAN_EID_SUPP_RATES: // [6..8]
                                case WLAN_EID_EXT_SUPP_RATES: // [4]
                                case WLAN_EID_PWR_CAPABILITY: // [2]
                                case WLAN_EID_SUPPORTED_CHANNELS: // [2]
                                case WLAN_EID_RSN: // [20]
                                case WLAN_EID_EXT_CAPAB: // [8]
                                case WLAN_EID_VENDOR_SPECIFIC: // [7..9]
                                case WLAN_EID_HT_CAP: // [26]
                                    outf(" %s=", ieid);
                                    dump_hex(ie->data, ie->len);
                                    break;
                                // 3-byte int
                                case WLAN_EID_BSS_MAX_IDLE_PERIOD: { // [3]
                                    unsigned v = ie->data[0] | ie->data[1] << 8 | ie->data[2] << 16;
                                    outf(" %s:%u", ieid, v);
                                    break;
                                }
                                case WLAN_EID_EXTENSION: {
                                    const char *extid = wlan_eid_ext_str[ie->data[0]];
                                    outf(" %s[%d]=", extid, ie->len - 1);
                                    dump_str(&ie->data[1], ie->len - 1);
                                    break;
                                }
                                default:
                                    outf(" %s[%d]=", ieid, ie->len);
                                    dump_str(ie->data, ie->len);
                                    break;
                                }

                            }
                            //outf("\n"); dump(iw, iwlen, 0, "\tIEs:\t");
                            break;
                        }
                        default:
                            outf("\n\t%s(cmd=%#x,iwlen=%d,len=%u):", iwcmd, iw->cmd, iwlen, iw->len);
                            outf("\n"); dump(iw, iwlen, 0, "\t\t");
                            break;
                        }

                    }

                    //outf("\n"); dump(RTA_DATA(rta), RTA_PAYLOAD(rta), 0, "\t");
                    break;
                }
                default:
                    outf("\n"); dump(RTA_DATA(rta), RTA_PAYLOAD(rta), 0, "\t");
                    break;
                }
            }
            outf("\n");

        } else if (nh->nlmsg_type == RTM_NEWADDR || nh->nlmsg_type == RTM_DELADDR) {
            // struct ifaddrmsg {
            //     unsigned char ifa_family;    /* Address type */
            //     unsigned char ifa_prefixlen; /* Prefixlength of address */
            //     unsigned char ifa_flags;     /* Address flags */
            //     unsigned char ifa_scope;     /* Address scope */
            //     unsigned int  ifa_index;     /* Interface index */
            // };
            struct ifaddrmsg *ifm = (struct ifaddrmsg *) NLMSG_DATA(nh);
            size_t rta_len = IFA_PAYLOAD(nh); //NLMSG_PAYLOAD(nh, sizeof(*ifm));

            outf("%s address ifindex %d, family %d, prefix /%d, flags %x, scope %#x,",
                nh->nlmsg_type == RTM_NEWADDR ? "+" : "-", ifm->ifa_index, ifm->ifa_family,
                ifm->ifa_prefixlen, ifm->ifa_flags, ifm->ifa_scope);

            for (struct rtattr *rta = IFA_RTA(ifm); RTA_OK(rta, rta_len); rta = RTA_NEXT(rta, rta_len)) {
                outf(" %s", ifa_rta_type_str[rta->rta_type]);
                switch (rta->rta_type) {
                // address
                case IFA_ADDRESS:
                case IFA_LOCAL:
                case IFA_BROADCAST:
                case IFA_ANYCAST:
                case IFA_MULTICAST:
                    print_rta_addr(ifm->ifa_family, rta);
                    break;
                // asciiz
                case IFA_LABEL: {
                    const char *ifname = RTA_DATA(rta);
                    outf(" %s", ifname);
                    break;
                }
                // ifa_cacheinfo
                case IFA_CACHEINFO: {
                    // struct ifa_cacheinfo {
                    //     __u32 ifa_prefered;
                    //     __u32 ifa_valid;
                    //     __u32 cstamp; // created timestamp, hundredths of seconds
                    //     __u32 tstamp; // updated timestamp, hundredths of seconds
                    // };
                    struct ifa_cacheinfo *ifci = RTA_DATA(rta);
                    outf(" prefered=%d,valid=%d,ct=%.2f,ut=%.2f",
                        ifci->ifa_prefered, ifci->ifa_valid,
                        ifci->cstamp/100.0, ifci->tstamp/100.0);
                    break;
                }
                // flags
                case IFA_FLAGS: {
                    int *pflags = RTA_DATA(rta), flags_len = RTA_PAYLOAD(rta);
                    if (flags_len == sizeof(int)) print_iff(*pflags);
                    break;
                }
                default:
                    //outf("IFA_0x%04x\n", rta->rta_type);
                    dump(RTA_DATA(rta), RTA_PAYLOAD(rta), 0, "\t");
                    break;
                }
            }
            outf("\n");

        } else if (nh->nlmsg_type == RTM_NEWROUTE || nh->nlmsg_type == RTM_DELROUTE) {
            // struct rtmsg {
            //     unsigned char rtm_family;
            //     unsigned char rtm_dst_len;
            //     unsigned char rtm_src_len;
            //     unsigned char rtm_tos;
            //     unsigned char rtm_table;     /* Routing table id */
            //     unsigned char rtm_protocol;  /* Routing protocol; see below  */
            //     unsigned char rtm_scope;     /* See below */
            //     unsigned char rtm_type;      /* See below    */
            //     unsigned      rtm_flags;
            // };
            struct rtmsg *ifm = (struct rtmsg *) NLMSG_DATA(nh);
            size_t rta_len = RTM_PAYLOAD(nh); //NLMSG_PAYLOAD(nh, sizeof(*ifm));

            outf("%s route %s, family %d, flags %#x, dst_len %u, src_len %u, tos %u, table %u, protocol %u, scope %u,",
                nh->nlmsg_type == RTM_NEWROUTE ? "+" : "-", rtmsg_type_str[ifm->rtm_type], ifm->rtm_family,
                ifm->rtm_flags, ifm->rtm_dst_len, ifm->rtm_src_len, ifm->rtm_tos,
                ifm->rtm_table, ifm->rtm_protocol, ifm->rtm_scope);

            for (struct rtattr *rta = RTM_RTA(ifm); RTA_OK(rta, rta_len); rta = RTA_NEXT(rta, rta_len)) {
                outf(" %s", rtattr_type_str[rta->rta_type]);
                switch (rta->rta_type) {
                // not implemented
                case RTA_UNSPEC:
                case RTA_CACHEINFO:
                    outf(" -");
                    break;
                // address
                case RTA_DST:
                case RTA_SRC:
                case RTA_GATEWAY:
                case RTA_PREFSRC:
                case RTA_NEWDST:
                    print_rta_addr(ifm->rtm_family, rta);
                    break;
                // unsigned char
                case RTA_PREF: {
                    const unsigned char *val = RTA_DATA(rta);
                    outf(" %u", *val);
                    break;
                }
                // unsigned short
                case RTA_ENCAP_TYPE: {
                    const unsigned short *val = RTA_DATA(rta);
                    outf(" %hu", *val);
                    break;
                }
                // unsigned int
                case RTA_TABLE:
                case RTA_PRIORITY:
                case RTA_IIF:
                case RTA_OIF:
                case RTA_METRICS:
                case RTA_FLOW:
                case RTA_MARK:
                case RTA_EXPIRES: {
                    const unsigned *val = RTA_DATA(rta);
                    outf(" %u", *val);
                    break;
                }
                default:
                    outf("\n");
                    dump(RTA_DATA(rta), RTA_PAYLOAD(rta), 0, "\t");
                    break;
                }
            }
            outf("\n");

        } else {
                outf("unimplemented nlmsg_type %d (%s)\n", nh->nlmsg_type,
                    nlmsg_type_str[nh->nlmsg_type]);
                dump(NLMSG_DATA(nh), NLMSG_PAYLOAD(nh, 0), NLMSG_DATA(nh) - (void*) &buf, "\t");
        }
    }

    return len;
}

#ifdef MAIN

static int send_req(int nls, unsigned seqno)
{
    int rc;
    struct sockaddr_nl nladdr = { AF_NETLINK, 0, 0, 0 };
    struct {
        struct nlmsghdr hdr;
        struct rtgenmsg gen;
    } req = {
        { sizeof(req), NLMSG_NOOP, NLM_F_REQUEST | NLM_F_ROOT | NLM_F_ACK, seqno, getpid() },
        { AF_UNSPEC },
    };

    switch (seqno) {
    case 0:
        req.hdr.nlmsg_type = RTM_GETLINK;
        rc = sendto(nls, &req, sizeof(req), 0, (struct sockaddr *) &nladdr, sizeof(nladdr));
        break;
    case 1:
        req.hdr.nlmsg_type = RTM_GETADDR;
        rc = sendto(nls, &req, sizeof(req), 0, (struct sockaddr *) &nladdr, sizeof(nladdr));
        break;
    case 2:
        req.hdr.nlmsg_type = RTM_GETROUTE;
        rc = sendto(nls, &req, sizeof(req), 0, (struct sockaddr *) &nladdr, sizeof(nladdr));
        break;
    }

    return rc == (int) sizeof(req);
}

int main(int argc, char *argv[])
{
    int rc;
    (void)argc;
    (void)argv;

    int nls = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (nls < 0) goto fail;

    int bufsize = 65536;
    rc = setsockopt(nls, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));
    if (rc < 0) goto fail;
    rc = setsockopt(nls, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
    if (rc < 0) goto fail;
    rc = setsockopt(nls, SOL_SOCKET, SO_RCVBUFFORCE, &bufsize, sizeof(bufsize));
    if (rc < 0) perror("setsockopt(SO_RCVBUFFORCE)");

    const int enable = 1;
    rc = setsockopt(nls, SOL_NETLINK, NETLINK_NO_ENOBUFS, &enable, sizeof(enable));
    if (rc < 0) goto fail;

    struct sockaddr_nl nlsaddr = { AF_NETLINK, 0, getpid(),
        RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV4_ROUTE |
        RTMGRP_IPV6_IFADDR | RTMGRP_IPV6_ROUTE };

    rc = bind(nls, (struct sockaddr *) &nlsaddr, sizeof(nlsaddr));
    if (rc < 0) goto fail;

    // Connect to kernel (pid 0) as remote address.
    struct sockaddr_nl nlraddr = { AF_NETLINK, 0, 0, 0 };
    rc = connect(nls, (struct sockaddr *) &nlraddr, sizeof(nlraddr));
    if (rc != 0) goto fail;

    size_t nlslen = sizeof(nlsaddr);
    rc = getsockname(nls, (struct sockaddr *) &nlsaddr, &nlslen);
    if (rc < 0) goto fail;

    outf("getsockname: len=%d family=%d pid=%d groups=%#x\n", nlslen,
        nlsaddr.nl_family, nlsaddr.nl_pid, nlsaddr.nl_groups);

    size_t nlrlen = sizeof(nlraddr);
    rc = getpeername(nls, (struct sockaddr *) &nlraddr, &nlrlen);
    if (rc < 0) goto fail;

    outf("getpeername: len=%d family=%d pid=%d groups=%#x\n", nlrlen,
        nlraddr.nl_family, nlraddr.nl_pid, nlraddr.nl_groups);

    //rc = fcntl(nls, F_GETFL, 0);
    //if (rc < 0) goto fail;
    //rc = fcntl(nls, F_SETFL, rc | O_NONBLOCK);
    //if (rc < 0) goto fail;

    // struct iovec {                    /* Scatter/gather array items */
    //     void  *iov_base;              /* Starting address */
    //     size_t iov_len;               /* Number of bytes to transfer */
    // };

    // struct msghdr {
    //     void         *msg_name;       /* Optional address */
    //     socklen_t     msg_namelen;    /* Size of address */
    //     struct iovec *msg_iov;        /* Scatter/gather array */
    //     size_t        msg_iovlen;     /* # elements in msg_iov */
    //     void         *msg_control;    /* Ancillary data, see below */
    //     size_t        msg_controllen; /* Ancillary data buffer len */
    //     int           msg_flags;      /* Flags on received message */
    // };

    // struct nlmsghdr {
    //     __u32 nlmsg_len;    /* Length of message including header */
    //     __u16 nlmsg_type;   /* Type of message content */
    //     __u16 nlmsg_flags;  /* Additional flags */
    //     __u32 nlmsg_seq;    /* Sequence number */
    //     __u32 nlmsg_pid;    /* Sender port ID */
    // };

    // struct rtgenmsg {
    //     unsigned char rtgen_family;
    // };

    // struct sockaddr_nl {
    //     unsigned short  nl_family;      /* AF_NETLINK   */
    //     unsigned short  nl_pad;         /* zero         */
    //     __u32           nl_pid;         /* port ID      */
    //     __u32           nl_groups;      /* multicast groups mask */
    // };

    union {
        struct nlmsghdr nh;
        char buf[8192];
    } buf;

    int done = 0;
    unsigned seqno = 0;
    struct iovec iov = { &buf, sizeof(buf) };
    struct sockaddr_nl nladdr = { AF_NETLINK, 0, 0, 0 }; // send to kernel (pid 0) as remote address
    struct msghdr msg = { &nladdr, sizeof(nladdr), &iov, 1, NULL, 0, MSG_WAITALL };

    send_req(nls, seqno++);

    do {
        int len = recvmsg(nls, &msg, 0);

        if (len < 0) {
            goto fail;
        } else if (len == 0) {
            outf("netlink_read: EOF\n");
            goto fail;
        } else if (msg.msg_flags & MSG_TRUNC) {
            outf("netlink_read: message truncated (%d)\n", len);
            continue;
        } else if (len < NLMSG_HDRLEN) {
            outf("netlink_read: short read (%d)\n", len);
            continue;
        } else if (msg.msg_namelen != nlslen) {
            outf("netlink_read: unexpected sender address length (%d)\n", msg.msg_namelen);
            goto fail;
        } else if (nladdr.nl_pid != 0) {
            outf("netlink_read: message from pid=%d\n", nladdr.nl_pid);
            continue;
        }

        if (1) {
            struct nlmsghdr *nh = (void *) &buf;
            outf("\n-------------------- Netlink message %s seq %d pid %d, %d bytes %s ----------------------------\n",
                len >= NLMSG_HDRLEN ? nlmsg_type_str[nh->nlmsg_type] : "?",
                len >= NLMSG_HDRLEN ? nh->nlmsg_seq : 0xffffffff,
                len >= NLMSG_HDRLEN ? nh->nlmsg_pid : 0xffffffff,
                len, nh->nlmsg_flags & NLM_F_MULTI ? "[multi]" : ""
            );
            if (nh->nlmsg_type == NLMSG_DONE || (nh->nlmsg_flags & NLM_F_MULTI) == 0)
                send_req(nls, seqno++);
        }

        if (nl_debug(&buf, len) < 0)
            goto fail;
    } while (!done);

    return 0;

  fail:
    fflush(stdout);
    perror("netlink");
    if (nls > 0) close(nls);
    return 1;
}
#endif
