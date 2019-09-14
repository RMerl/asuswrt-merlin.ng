/*
 * lib/utils.c		Utility Functions
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2012 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup core
 * @defgroup utils Utilities
 *
 * Collection of helper functions
 *
 * @{
 *
 * Header
 * ------
 * ~~~~{.c}
 * #include <netlink/utils.h>
 * ~~~~
 */

#include <netlink-private/netlink.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <linux/socket.h>
#include <stdlib.h> /* exit() */

/**
 * Global variable indicating the desired level of debugging output.
 *
 * Level | Messages Printed
 * ----- | ---------------------------------------------------------
 *     0 | Debugging output disabled
 *     1 | Warnings, important events and notifications
 *     2 | More or less important debugging messages
 *     3 | Repetitive events causing a flood of debugging messages
 *     4 | Even less important messages
 *
 * If available, the variable will be initialized to the value of the
 * environment variable `NLDBG`. The default value is 0 (disabled).
 *
 * For more information, see section @core_doc{_debugging, Debugging}.
 */
int nl_debug = 0;

/** @cond SKIP */
#ifdef NL_DEBUG
struct nl_dump_params nl_debug_dp = {
	.dp_type = NL_DUMP_DETAILS,
};

static void __init nl_debug_init(void)
{
	char *nldbg, *end;
	
	if ((nldbg = getenv("NLDBG"))) {
		long level = strtol(nldbg, &end, 0);
		if (nldbg != end)
			nl_debug = level;
	}

	nl_debug_dp.dp_fd = stderr;
}
#endif

int __nl_read_num_str_file(const char *path, int (*cb)(long, const char *))
{
	FILE *fd;
	char buf[128];

	fd = fopen(path, "r");
	if (fd == NULL)
		return -nl_syserr2nlerr(errno);

	while (fgets(buf, sizeof(buf), fd)) {
		int goodlen, err;
		long num;
		char *end;

		if (*buf == '#' || *buf == '\n' || *buf == '\r')
			continue;

		num = strtol(buf, &end, 0);
		if (end == buf) {
			fclose(fd);
			return -NLE_INVAL;
		}

		if (num == LONG_MIN || num == LONG_MAX) {
			fclose(fd);
			return -NLE_RANGE;
		}

		while (*end == ' ' || *end == '\t')
			end++;

		goodlen = strcspn(end, "#\r\n\t ");
		if (goodlen == 0) {
			fclose(fd);
			return -NLE_INVAL;
		}

		end[goodlen] = '\0';

		err = cb(num, end);
		if (err < 0) {
			fclose(fd);
			return err;
		}
	}

	fclose(fd);

	return 0;
}
/** @endcond */

/**
 * @name Pretty Printing of Numbers
 * @{
 */

/**
 * Cancel down a byte counter
 * @arg	l		byte counter
 * @arg	unit		destination unit pointer
 *
 * Cancels down a byte counter until it reaches a reasonable
 * unit. The chosen unit is assigned to \a unit.
 * This function assume 1024 bytes in one kilobyte
 * 
 * @return The cancelled down byte counter in the new unit.
 */
double nl_cancel_down_bytes(unsigned long long l, char **unit)
{
	if (l >= 1099511627776LL) {
		*unit = "TiB";
		return ((double) l) / 1099511627776LL;
	} else if (l >= 1073741824) {
		*unit = "GiB";
		return ((double) l) / 1073741824;
	} else if (l >= 1048576) {
		*unit = "MiB";
		return ((double) l) / 1048576;
	} else if (l >= 1024) {
		*unit = "KiB";
		return ((double) l) / 1024;
	} else {
		*unit = "B";
		return (double) l;
	}
}

/**
 * Cancel down a bit counter
 * @arg	l		bit counter
 * @arg unit		destination unit pointer
 *
 * Cancels down bit counter until it reaches a reasonable
 * unit. The chosen unit is assigned to \a unit.
 * This function assume 1000 bits in one kilobit
 *
 * @return The cancelled down bit counter in the new unit.
 */
double nl_cancel_down_bits(unsigned long long l, char **unit)
{
	if (l >= 1000000000000ULL) {
		*unit = "Tbit";
		return ((double) l) / 1000000000000ULL;
	}

	if (l >= 1000000000) {
		*unit = "Gbit";
		return ((double) l) / 1000000000;
	}

	if (l >= 1000000) {
		*unit = "Mbit";
		return ((double) l) / 1000000;
	}

	if (l >= 1000) {
		*unit = "Kbit";
		return ((double) l) / 1000;
	}

	*unit = "bit";
	return (double) l;
}

int nl_rate2str(unsigned long long rate, int type, char *buf, size_t len)
{
	char *unit;
	double frac;

	switch (type) {
	case NL_BYTE_RATE:
		frac = nl_cancel_down_bytes(rate, &unit);
		break;
	
	case NL_BIT_RATE:
		frac = nl_cancel_down_bits(rate, &unit);
		break;
	
	default:
		BUG();
	}

	return snprintf(buf, len, "%.2f%s/s", frac, unit);
}

/**
 * Cancel down a micro second value
 * @arg	l		micro seconds
 * @arg unit		destination unit pointer
 *
 * Cancels down a microsecond counter until it reaches a
 * reasonable unit. The chosen unit is assigned to \a unit.
 *
 * @return The cancelled down microsecond in the new unit
 */
double nl_cancel_down_us(uint32_t l, char **unit)
{
	if (l >= 1000000) {
		*unit = "s";
		return ((double) l) / 1000000;
	} else if (l >= 1000) {
		*unit = "ms";
		return ((double) l) / 1000;
	} else {
		*unit = "us";
		return (double) l;
	}
}

/** @} */

/**
 * @name Generic Unit Translations
 * @{
 */

/**
 * Convert a character string to a size
 * @arg str		size encoded as character string
 *
 * Converts the specified size as character to the corresponding
 * number of bytes.
 *
 * Supported formats are:
 *  - b,kb/k,m/mb,gb/g for bytes
 *  - bit,kbit/mbit/gbit
 *
 * This function assume 1000 bits in one kilobit and
 * 1024 bytes in one kilobyte
 *
 * @return The number of bytes or -1 if the string is unparseable
 */
long nl_size2int(const char *str)
{
	char *p;
	long l = strtol(str, &p, 0);
	if (p == str)
		return -NLE_INVAL;

	if (*p) {
		if (!strcasecmp(p, "kb") || !strcasecmp(p, "k"))
			l *= 1024;
		else if (!strcasecmp(p, "gb") || !strcasecmp(p, "g"))
			l *= 1024*1024*1024;
		else if (!strcasecmp(p, "gbit"))
			l *= 1000000000L/8;
		else if (!strcasecmp(p, "mb") || !strcasecmp(p, "m"))
			l *= 1024*1024;
		else if (!strcasecmp(p, "mbit"))
			l *= 1000000/8;
		else if (!strcasecmp(p, "kbit"))
			l *= 1000/8;
		else if (!strcasecmp(p, "bit"))
			l /= 8;
		else if (strcasecmp(p, "b") != 0)
			return -NLE_INVAL;
	}

	return l;
}

static const struct {
	double limit;
	const char *unit;
} size_units[] = {
	{ 1024. * 1024. * 1024. * 1024. * 1024., "EiB" },
	{ 1024. * 1024. * 1024. * 1024., "TiB" },
	{ 1024. * 1024. * 1024., "GiB" },
	{ 1024. * 1024., "MiB" },
	{ 1024., "KiB" },
	{ 0., "B" },
};

/**
 * Convert a size toa character string
 * @arg size		Size in number of bytes
 * @arg buf		Buffer to write character string to
 * @arg len		Size of buf
 *
 * This function converts a value in bytes to a human readable representation
 * of it. The function uses IEC prefixes:
 *
 * @code
 * 1024 bytes => 1 KiB
 * 1048576 bytes => 1 MiB
 * @endcode
 *
 * The highest prefix is used which ensures a result of >= 1.0, the result
 * is provided as floating point number with a maximum precision of 2 digits:
 * @code
 * 965176 bytes => 942.55 KiB
 * @endcode
 *
 * @return pointer to buf
 */
char *nl_size2str(const size_t size, char *buf, const size_t len)
{
	size_t i;

	if (size == 0) {
		snprintf(buf, len, "0B");
		return buf;
	}

	for (i = 0; i < ARRAY_SIZE(size_units); i++) {
		if (size >= size_units[i].limit) {
			snprintf(buf, len, "%.2g%s",
				(double) size / size_units[i].limit,
				size_units[i].unit);
			return buf;
		}
	}

	BUG();
}

/**
 * Convert a character string to a probability
 * @arg str		probability encoded as character string
 *
 * Converts the specified probability as character to the
 * corresponding probability number.
 *
 * Supported formats are:
 *  - 0.0-1.0
 *  - 0%-100%
 *
 * @return The probability relative to NL_PROB_MIN and NL_PROB_MAX
 */
long nl_prob2int(const char *str)
{
	char *p;
	double d = strtod(str, &p);

	if (p == str)
		return -NLE_INVAL;

	if (d > 1.0)
		d /= 100.0f;

	if (d > 1.0f || d < 0.0f)
		return -NLE_RANGE;

	if (*p && strcmp(p, "%") != 0)
		return -NLE_INVAL;

	return rint(d * NL_PROB_MAX);
}

/** @} */

/**
 * @name Time Translations
 * @{
 */

#ifndef USER_HZ
#define USER_HZ 100
#endif

static uint32_t user_hz = USER_HZ;
static uint32_t psched_hz = USER_HZ;

static double ticks_per_usec = 1.0f;

/* Retrieves the configured HZ and ticks/us value in the kernel.
 * The value is cached. Supported ways of getting it:
 *
 * 1) environment variable
 * 2) /proc/net/psched and sysconf
 *
 * Supports the environment variables:
 *   PROC_NET_PSCHED  - may point to psched file in /proc
 *   PROC_ROOT        - may point to /proc fs */ 
static void __init get_psched_settings(void)
{
	char name[FILENAME_MAX];
	FILE *fd;
	int got_hz = 0;

	if (getenv("HZ")) {
		long hz = strtol(getenv("HZ"), NULL, 0);

		if (LONG_MIN != hz && LONG_MAX != hz) {
			user_hz = hz;
			got_hz = 1;
		}
	}

	if (!got_hz)
		user_hz = sysconf(_SC_CLK_TCK);

	psched_hz = user_hz;

	if (getenv("TICKS_PER_USEC")) {
		double t = strtod(getenv("TICKS_PER_USEC"), NULL);
		ticks_per_usec = t;
	}
	else {
		if (getenv("PROC_NET_PSCHED"))
			snprintf(name, sizeof(name), "%s", getenv("PROC_NET_PSCHED"));
		else if (getenv("PROC_ROOT"))
			snprintf(name, sizeof(name), "%s/net/psched",
				 getenv("PROC_ROOT"));
		else
			strncpy(name, "/proc/net/psched", sizeof(name) - 1);
		
		if ((fd = fopen(name, "r"))) {
			unsigned int ns_per_usec, ns_per_tick, nom, denom;

			if (fscanf(fd, "%08x %08x %08x %08x",
			       &ns_per_usec, &ns_per_tick, &nom, &denom) != 4) {
                            NL_DBG(1, "Fatal error: can not read psched settings from \"%s\". " \
                                    "Try to set TICKS_PER_USEC, PROC_NET_PSCHED or PROC_ROOT " \
                                    "environment variables\n", name);
                            exit(1);
                        }

			ticks_per_usec = (double) ns_per_usec / 
					 (double) ns_per_tick;

			if (nom == 1000000)
				psched_hz = denom;

			fclose(fd);
		}
	}
}


/**
 * Return the value of HZ
 */
int nl_get_user_hz(void)
{
	return user_hz;
}

/**
 * Return the value of packet scheduler HZ
 */
int nl_get_psched_hz(void)
{
	return psched_hz;
}

/**
 * Convert micro seconds to ticks
 * @arg us		micro seconds
 * @return number of ticks
 */
uint32_t nl_us2ticks(uint32_t us)
{
	return us * ticks_per_usec;
}


/**
 * Convert ticks to micro seconds
 * @arg ticks		number of ticks
 * @return microseconds
 */
uint32_t nl_ticks2us(uint32_t ticks)
{
	return ticks / ticks_per_usec;
}

int nl_str2msec(const char *str, uint64_t *result)
{
	uint64_t total = 0, l;
	int plen;
	char *p;

	do {
		l = strtoul(str, &p, 0);
		if (p == str)
			return -NLE_INVAL;
		else if (*p) {
			plen = strcspn(p, " \t");

			if (!plen)
				total += l;
			else if (!strncasecmp(p, "sec", plen))
				total += (l * 1000);
			else if (!strncasecmp(p, "min", plen))
				total += (l * 1000*60);
			else if (!strncasecmp(p, "hour", plen))
				total += (l * 1000*60*60);
			else if (!strncasecmp(p, "day", plen))
				total += (l * 1000*60*60*24);
			else
				return -NLE_INVAL;

			str = p + plen;
		} else
			total += l;
	} while (*str && *p);

	*result = total;

	return 0;
}

/**
 * Convert milliseconds to a character string
 * @arg msec		number of milliseconds
 * @arg buf		destination buffer
 * @arg len		buffer length
 *
 * Converts milliseconds to a character string split up in days, hours,
 * minutes, seconds, and milliseconds and stores it in the specified
 * destination buffer.
 *
 * @return The destination buffer.
 */
char * nl_msec2str(uint64_t msec, char *buf, size_t len)
{
	uint64_t split[5];
	size_t i;
	static const char *units[5] = {"d", "h", "m", "s", "msec"};
	char * const buf_orig = buf;

	if (msec == 0) {
		snprintf(buf, len, "0msec");
		return buf_orig;
	}

#define _SPLIT(idx, unit) if ((split[idx] = msec / unit)) msec %= unit
	_SPLIT(0, 86400000);	/* days */
	_SPLIT(1, 3600000);	/* hours */
	_SPLIT(2, 60000);	/* minutes */
	_SPLIT(3, 1000);	/* seconds */
#undef  _SPLIT
	split[4] = msec;

	for (i = 0; i < ARRAY_SIZE(split) && len; i++) {
		int l;
		if (split[i] == 0)
			continue;
		l = snprintf(buf, len, "%s%" PRIu64 "%s",
			(buf==buf_orig) ? "" : " ", split[i], units[i]);
		buf += l;
		len -= l;
	}

	return buf_orig;
}

/** @} */

/**
 * @name Netlink Family Translations
 * @{
 */

static const struct trans_tbl nlfamilies[] = {
	__ADD(NETLINK_ROUTE,route)
	__ADD(NETLINK_USERSOCK,usersock)
	__ADD(NETLINK_FIREWALL,firewall)
	__ADD(NETLINK_INET_DIAG,inetdiag)
	__ADD(NETLINK_NFLOG,nflog)
	__ADD(NETLINK_XFRM,xfrm)
	__ADD(NETLINK_SELINUX,selinux)
	__ADD(NETLINK_ISCSI,iscsi)
	__ADD(NETLINK_AUDIT,audit)
	__ADD(NETLINK_FIB_LOOKUP,fib_lookup)
	__ADD(NETLINK_CONNECTOR,connector)
	__ADD(NETLINK_NETFILTER,netfilter)
	__ADD(NETLINK_IP6_FW,ip6_fw)
	__ADD(NETLINK_DNRTMSG,dnrtmsg)
	__ADD(NETLINK_KOBJECT_UEVENT,kobject_uevent)
	__ADD(NETLINK_GENERIC,generic)
	__ADD(NETLINK_SCSITRANSPORT,scsitransport)
	__ADD(NETLINK_ECRYPTFS,ecryptfs)
};

char * nl_nlfamily2str(int family, char *buf, size_t size)
{
	return __type2str(family, buf, size, nlfamilies,
			  ARRAY_SIZE(nlfamilies));
}

int nl_str2nlfamily(const char *name)
{
	return __str2type(name, nlfamilies, ARRAY_SIZE(nlfamilies));
}

/**
 * @}
 */

/**
 * @name Link Layer Protocol Translations
 * @{
 */

static const struct trans_tbl llprotos[] = {
	{0, "generic"},
	__ADD(ARPHRD_ETHER,ether)
	__ADD(ARPHRD_EETHER,eether)
	__ADD(ARPHRD_AX25,ax25)
	__ADD(ARPHRD_PRONET,pronet)
	__ADD(ARPHRD_CHAOS,chaos)
	__ADD(ARPHRD_IEEE802,ieee802)
	__ADD(ARPHRD_ARCNET,arcnet)
	__ADD(ARPHRD_APPLETLK,atalk)
	__ADD(ARPHRD_DLCI,dlci)
	__ADD(ARPHRD_ATM,atm)
	__ADD(ARPHRD_METRICOM,metricom)
	__ADD(ARPHRD_IEEE1394,ieee1394)
#ifdef ARPHRD_EUI64
	__ADD(ARPHRD_EUI64,eui64)
#endif
	__ADD(ARPHRD_INFINIBAND,infiniband)
	__ADD(ARPHRD_SLIP,slip)
	__ADD(ARPHRD_CSLIP,cslip)
	__ADD(ARPHRD_SLIP6,slip6)
	__ADD(ARPHRD_CSLIP6,cslip6)
	__ADD(ARPHRD_RSRVD,rsrvd)
	__ADD(ARPHRD_ADAPT,adapt)
	__ADD(ARPHRD_ROSE,rose)
	__ADD(ARPHRD_X25,x25)
#ifdef ARPHRD_HWX25
	__ADD(ARPHRD_HWX25,hwx25)
#endif
	__ADD(ARPHRD_CAN,can)
	__ADD(ARPHRD_PPP,ppp)
	__ADD(ARPHRD_HDLC,hdlc)
	__ADD(ARPHRD_LAPB,lapb)
	__ADD(ARPHRD_DDCMP,ddcmp)
	__ADD(ARPHRD_RAWHDLC,rawhdlc)
	__ADD(ARPHRD_TUNNEL,ipip)
	__ADD(ARPHRD_TUNNEL6,tunnel6)
	__ADD(ARPHRD_FRAD,frad)
	__ADD(ARPHRD_SKIP,skip)
	__ADD(ARPHRD_LOOPBACK,loopback)
	__ADD(ARPHRD_LOCALTLK,localtlk)
	__ADD(ARPHRD_FDDI,fddi)
	__ADD(ARPHRD_BIF,bif)
	__ADD(ARPHRD_SIT,sit)
	__ADD(ARPHRD_IPDDP,ip/ddp)
	__ADD(ARPHRD_IPGRE,gre)
	__ADD(ARPHRD_PIMREG,pimreg)
	__ADD(ARPHRD_HIPPI,hippi)
	__ADD(ARPHRD_ASH,ash)
	__ADD(ARPHRD_ECONET,econet)
	__ADD(ARPHRD_IRDA,irda)
	__ADD(ARPHRD_FCPP,fcpp)
	__ADD(ARPHRD_FCAL,fcal)
	__ADD(ARPHRD_FCPL,fcpl)
	__ADD(ARPHRD_FCFABRIC,fcfb_0)
	__ADD(ARPHRD_FCFABRIC+1,fcfb_1)
	__ADD(ARPHRD_FCFABRIC+2,fcfb_2)
	__ADD(ARPHRD_FCFABRIC+3,fcfb_3)
	__ADD(ARPHRD_FCFABRIC+4,fcfb_4)
	__ADD(ARPHRD_FCFABRIC+5,fcfb_5)
	__ADD(ARPHRD_FCFABRIC+6,fcfb_6)
	__ADD(ARPHRD_FCFABRIC+7,fcfb_7)
	__ADD(ARPHRD_FCFABRIC+8,fcfb_8)
	__ADD(ARPHRD_FCFABRIC+9,fcfb_9)
	__ADD(ARPHRD_FCFABRIC+10,fcfb_10)
	__ADD(ARPHRD_FCFABRIC+11,fcfb_11)
	__ADD(ARPHRD_FCFABRIC+12,fcfb_12)
	__ADD(ARPHRD_IEEE802_TR,tr)
	__ADD(ARPHRD_IEEE80211,ieee802.11)
	__ADD(ARPHRD_PHONET,phonet)
#ifdef ARPHRD_CAIF
	__ADD(ARPHRD_CAIF, caif)
#endif
#ifdef ARPHRD_IEEE80211_PRISM
	__ADD(ARPHRD_IEEE80211_PRISM, ieee802.11_prism)
#endif
#ifdef ARPHRD_VOID
	__ADD(ARPHRD_VOID,void)
#endif
#ifdef ARPHRD_NONE
	__ADD(ARPHRD_NONE,nohdr)
#endif
};

char * nl_llproto2str(int llproto, char *buf, size_t len)
{
	return __type2str(llproto, buf, len, llprotos, ARRAY_SIZE(llprotos));
}

int nl_str2llproto(const char *name)
{
	return __str2type(name, llprotos, ARRAY_SIZE(llprotos));
}

/** @} */


/**
 * @name Ethernet Protocol Translations
 * @{
 */

static const struct trans_tbl ether_protos[] = {
	__ADD(ETH_P_LOOP,loop)
	__ADD(ETH_P_PUP,pup)
	__ADD(ETH_P_PUPAT,pupat)
	__ADD(ETH_P_IP,ip)
	__ADD(ETH_P_X25,x25)
	__ADD(ETH_P_ARP,arp)
	__ADD(ETH_P_BPQ,bpq)
	__ADD(ETH_P_IEEEPUP,ieeepup)
	__ADD(ETH_P_IEEEPUPAT,ieeepupat)
	__ADD(ETH_P_DEC,dec)
	__ADD(ETH_P_DNA_DL,dna_dl)
	__ADD(ETH_P_DNA_RC,dna_rc)
	__ADD(ETH_P_DNA_RT,dna_rt)
	__ADD(ETH_P_LAT,lat)
	__ADD(ETH_P_DIAG,diag)
	__ADD(ETH_P_CUST,cust)
	__ADD(ETH_P_SCA,sca)
	__ADD(ETH_P_TEB,teb)
	__ADD(ETH_P_RARP,rarp)
	__ADD(ETH_P_ATALK,atalk)
	__ADD(ETH_P_AARP,aarp)
#ifdef ETH_P_8021Q
	__ADD(ETH_P_8021Q,802.1q)
#endif
	__ADD(ETH_P_IPX,ipx)
	__ADD(ETH_P_IPV6,ipv6)
	__ADD(ETH_P_PAUSE,pause)
	__ADD(ETH_P_SLOW,slow)
#ifdef ETH_P_WCCP
	__ADD(ETH_P_WCCP,wccp)
#endif
	__ADD(ETH_P_PPP_DISC,ppp_disc)
	__ADD(ETH_P_PPP_SES,ppp_ses)
	__ADD(ETH_P_MPLS_UC,mpls_uc)
	__ADD(ETH_P_MPLS_MC,mpls_mc)
	__ADD(ETH_P_ATMMPOA,atmmpoa)
	__ADD(ETH_P_LINK_CTL,link_ctl)
	__ADD(ETH_P_ATMFATE,atmfate)
	__ADD(ETH_P_PAE,pae)
	__ADD(ETH_P_AOE,aoe)
	__ADD(ETH_P_TIPC,tipc)
	__ADD(ETH_P_1588,ieee1588)
	__ADD(ETH_P_FCOE,fcoe)
	__ADD(ETH_P_FIP,fip)
	__ADD(ETH_P_EDSA,edsa)
	__ADD(ETH_P_EDP2,edp2)
	__ADD(ETH_P_802_3,802.3)
	__ADD(ETH_P_AX25,ax25)
	__ADD(ETH_P_ALL,all)
	__ADD(ETH_P_802_2,802.2)
	__ADD(ETH_P_SNAP,snap)
	__ADD(ETH_P_DDCMP,ddcmp)
	__ADD(ETH_P_WAN_PPP,wan_ppp)
	__ADD(ETH_P_PPP_MP,ppp_mp)
	__ADD(ETH_P_LOCALTALK,localtalk)
	__ADD(ETH_P_CAN,can)
	__ADD(ETH_P_PPPTALK,ppptalk)
	__ADD(ETH_P_TR_802_2,tr_802.2)
	__ADD(ETH_P_MOBITEX,mobitex)
	__ADD(ETH_P_CONTROL,control)
	__ADD(ETH_P_IRDA,irda)
	__ADD(ETH_P_ECONET,econet)
	__ADD(ETH_P_HDLC,hdlc)
	__ADD(ETH_P_ARCNET,arcnet)
	__ADD(ETH_P_DSA,dsa)
	__ADD(ETH_P_TRAILER,trailer)
	__ADD(ETH_P_PHONET,phonet)
	__ADD(ETH_P_IEEE802154,ieee802154)
	__ADD(ETH_P_CAIF,caif)
};

char *nl_ether_proto2str(int eproto, char *buf, size_t len)
{
	return __type2str(eproto, buf, len, ether_protos,
			    ARRAY_SIZE(ether_protos));
}

int nl_str2ether_proto(const char *name)
{
	return __str2type(name, ether_protos, ARRAY_SIZE(ether_protos));
}

/** @} */

/**
 * @name IP Protocol Translations
 * @{
 */

char *nl_ip_proto2str(int proto, char *buf, size_t len)
{
	struct protoent *p = getprotobynumber(proto);

	if (p) {
		snprintf(buf, len, "%s", p->p_name);
		return buf;
	}

	snprintf(buf, len, "0x%x", proto);
	return buf;
}

int nl_str2ip_proto(const char *name)
{
	struct protoent *p = getprotobyname(name);
	unsigned long l;
	char *end;

	if (p)
		return p->p_proto;

	l = strtoul(name, &end, 0);
	if (l == ULONG_MAX || *end != '\0')
		return -NLE_OBJ_NOTFOUND;

	return (int) l;
}

/** @} */

/**
 * @name Dumping Helpers
 * @{
 */

/**
 * Handle a new line while dumping
 * @arg params		Dumping parameters
 *
 * This function must be called before dumping any onto a
 * new line. It will ensure proper prefixing as specified
 * by the dumping parameters.
 *
 * @note This function will NOT dump any newlines itself
 */
void nl_new_line(struct nl_dump_params *params)
{
	params->dp_line++;

	if (params->dp_prefix) {
		int i;
		for (i = 0; i < params->dp_prefix; i++) {
			if (params->dp_fd)
				fprintf(params->dp_fd, " ");
			else if (params->dp_buf)
				strncat(params->dp_buf, " ",
					params->dp_buflen -
					strlen(params->dp_buf) - 1);
		}
	}

	if (params->dp_nl_cb)
		params->dp_nl_cb(params, params->dp_line);
}

static void dump_one(struct nl_dump_params *parms, const char *fmt,
		     va_list args)
{
	if (parms->dp_fd)
		vfprintf(parms->dp_fd, fmt, args);
	else if (parms->dp_buf || parms->dp_cb) {
		char *buf = NULL;
		if (vasprintf(&buf, fmt, args) >= 0) {
			if (parms->dp_cb)
				parms->dp_cb(parms, buf);
			else
				strncat(parms->dp_buf, buf,
					parms->dp_buflen -
					strlen(parms->dp_buf) - 1);
			free(buf);
		}
	}
}


/**
 * Dump a formatted character string
 * @arg params		Dumping parameters
 * @arg fmt		printf style formatting string
 * @arg ...		Arguments to formatting string
 *
 * Dumps a printf style formatting string to the output device
 * as specified by the dumping parameters.
 */
void nl_dump(struct nl_dump_params *params, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	dump_one(params, fmt, args);
	va_end(args);
}

void nl_dump_line(struct nl_dump_params *parms, const char *fmt, ...)
{
	va_list args;

	nl_new_line(parms);

	va_start(args, fmt);
	dump_one(parms, fmt, args);
	va_end(args);
}


/** @} */

/** @cond SKIP */

int __trans_list_add(int i, const char *a, struct nl_list_head *head)
{
	struct trans_list *tl;

	tl = calloc(1, sizeof(*tl));
	if (!tl)
		return -NLE_NOMEM;

	tl->i = i;
	tl->a = strdup(a);

	nl_list_add_tail(&tl->list, head);

	return 0;
}

void __trans_list_clear(struct nl_list_head *head)
{
	struct trans_list *tl, *next;

	nl_list_for_each_entry_safe(tl, next, head, list) {
		free(tl->a);
		free(tl);
	}

	nl_init_list_head(head);
}

char *__type2str(int type, char *buf, size_t len,
		 const struct trans_tbl *tbl, size_t tbl_len)
{
	size_t i;
	for (i = 0; i < tbl_len; i++) {
		if (tbl[i].i == type) {
			snprintf(buf, len, "%s", tbl[i].a);
			return buf;
		}
	}

	snprintf(buf, len, "0x%x", type);
	return buf;
}

char *__list_type2str(int type, char *buf, size_t len,
		      struct nl_list_head *head)
{
	struct trans_list *tl;

	nl_list_for_each_entry(tl, head, list) {
		if (tl->i == type) {
			snprintf(buf, len, "%s", tl->a);
			return buf;
		}
	}

	snprintf(buf, len, "0x%x", type);
	return buf;
}

char *__flags2str(int flags, char *buf, size_t len,
		  const struct trans_tbl *tbl, size_t tbl_len)
{
	size_t i;
	int tmp = flags;

	memset(buf, 0, len);

	for (i = 0; i < tbl_len; i++) {
		if (tbl[i].i & tmp) {
			tmp &= ~tbl[i].i;
			strncat(buf, tbl[i].a, len - strlen(buf) - 1);
			if ((tmp & flags))
				strncat(buf, ",", len - strlen(buf) - 1);
		}
	}

	return buf;
}

int __str2type(const char *buf, const struct trans_tbl *tbl, size_t tbl_len)
{
	unsigned long l;
	char *end;
	size_t i;

	if (*buf == '\0')
		return -NLE_INVAL;

	for (i = 0; i < tbl_len; i++)
		if (!strcasecmp(tbl[i].a, buf))
			return tbl[i].i;

	l = strtoul(buf, &end, 0);
	if (l == ULONG_MAX || *end != '\0')
		return -NLE_OBJ_NOTFOUND;

	return (int) l;
}

int __list_str2type(const char *buf, struct nl_list_head *head)
{
	struct trans_list *tl;
	unsigned long l;
	char *end;

	if (*buf == '\0')
		return -NLE_INVAL;

	nl_list_for_each_entry(tl, head, list) {
		if (!strcasecmp(tl->a, buf))
			return tl->i;
	}

	l = strtoul(buf, &end, 0);
	if (l == ULONG_MAX || *end != '\0')
		return -NLE_OBJ_NOTFOUND;

	return (int) l;
}

int __str2flags(const char *buf, const struct trans_tbl *tbl, size_t tbl_len)
{
	int flags = 0;
	size_t i;
	size_t len; /* ptrdiff_t ? */
	char *p = (char *) buf, *t;

	for (;;) {
		if (*p == ' ')
			p++;
	
		t = strchr(p, ',');
		len = t ? t - p : strlen(p);
		for (i = 0; i < tbl_len; i++)
			if (len == strlen(tbl[i].a) &&
			    !strncasecmp(tbl[i].a, p, len))
				flags |= tbl[i].i;

		if (!t)
			return flags;

		p = ++t;
	}

	return 0;
}

void dump_from_ops(struct nl_object *obj, struct nl_dump_params *params)
{
	int type = params->dp_type;

	if (type < 0 || type > NL_DUMP_MAX)
		BUG();

	params->dp_line = 0;

	if (params->dp_dump_msgtype) {
#if 0
		/* XXX */
		char buf[64];

		dp_dump_line(params, 0, "%s ",
			     nl_cache_mngt_type2name(obj->ce_ops,
			     			     obj->ce_ops->co_protocol,
						     obj->ce_msgtype,
						     buf, sizeof(buf)));
#endif
		params->dp_pre_dump = 1;
	}

	if (obj->ce_ops->oo_dump[type])
		obj->ce_ops->oo_dump[type](obj, params);
}

/**
 * Check for library capabilities
 *
 * @arg	capability	capability identifier
 *
 * Check whether the loaded libnl library supports a certain capability.
 * This is useful so that applications can workaround known issues of
 * libnl that are fixed in newer library versions, without
 * having a hard dependency on the new version. It is also useful, for
 * capabilities that cannot easily be detected using autoconf tests.
 * The capabilities are integer constants with name NL_CAPABILITY_*.
 *
 * As this function is intended to detect capabilities at runtime,
 * you might not want to depend during compile time on the NL_CAPABILITY_*
 * names. Instead you can use their numeric values which are guaranteed not to
 * change meaning.
 *
 * @return non zero if libnl supports a certain capability, 0 otherwise.
 **/
int nl_has_capability (int capability)
{
	static const uint8_t caps[ ( NL_CAPABILITY_MAX + 7 ) / 8  ] = {
#define _NL_ASSERT(expr) ( 0 * sizeof(struct { unsigned int x: ( (!!(expr)) ? 1 : -1 ); }) )
#define _NL_SETV(i, r, v) \
		( _NL_ASSERT( (v) == 0 || (i) * 8 + (r) == (v) - 1 ) + \
		  ( (v) == 0 ? 0 : (1 << (r)) ) )
#define _NL_SET(i, v0, v1, v2, v3, v4, v5, v6, v7) \
		[(i)] = ( \
			_NL_SETV((i), 0, (v0)) | _NL_SETV((i), 4, (v4)) | \
			_NL_SETV((i), 1, (v1)) | _NL_SETV((i), 5, (v5)) | \
			_NL_SETV((i), 2, (v2)) | _NL_SETV((i), 6, (v6)) | \
			_NL_SETV((i), 3, (v3)) | _NL_SETV((i), 7, (v7)) )
		_NL_SET(0,
			NL_CAPABILITY_ROUTE_BUILD_MSG_SET_SCOPE,
			NL_CAPABILITY_ROUTE_LINK_VETH_GET_PEER_OWN_REFERENCE,
			NL_CAPABILITY_ROUTE_LINK_CLS_ADD_ACT_OWN_REFERENCE,
			NL_CAPABILITY_NL_CONNECT_RETRY_GENERATE_PORT_ON_ADDRINUSE,
			0,
			0,
			0,
			0),
#undef _NL_SET
#undef _NL_SETV
#undef _NL_ASSERT
	};

	if (capability <= 0 || capability > NL_CAPABILITY_MAX)
		return 0;
	capability--;
	return (caps[capability / 8] & (1 << (capability % 8))) != 0;
}

/** @endcond */

/** @} */
