/* $Id: upnpglobalvars.h,v 1.53 2025/04/06 22:30:26 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2006-2026 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef UPNPGLOBALVARS_H_INCLUDED
#define UPNPGLOBALVARS_H_INCLUDED

/*! \file upnpglobalvars.h
 * \brief global variables */

#include <time.h>
#include "upnppermissions.h"
#include "miniupnpdtypes.h"
#include "config.h"

/*! \brief name of the network interface used to access internet */
extern const char * ext_if_name;

#ifdef ENABLE_IPV6
/*! \brief name of the network interface used to access internet - for IPv6 */
extern const char * ext_if_name6;
#endif

/*! \brief STUN server host */
extern const char * ext_stun_host;
/*! \brief STUN server port */
extern uint16_t ext_stun_port;

/* file to store all leases */
#ifdef ENABLE_LEASEFILE
extern const char * lease_file;
#ifdef ENABLE_UPNPPINHOLE
extern const char * lease_file6;
#endif
#endif

/* forced ip address to use for this interface
 * when NULL, getifaddr() is used */
extern const char * use_ext_ip_addr;

/*! \brief disallow all port forwarding requests when
 * we are behind restrictive nat
 * \todo use a flag in #runtime_flags ? */
extern int disable_port_forwarding;

/* parameters to return to upnp client when asked */
extern unsigned long downstream_bitrate;
extern unsigned long upstream_bitrate;

/*! \brief statup time */
extern time_t startup_time;
#if defined(ENABLE_NATPMP) || defined(ENABLE_PCP)
/*! brief origin for "epoch time" sent into NATPMP and PCP responses */
extern time_t epoch_origin;
#endif /*  defined(ENABLE_NATPMP) || defined(ENABLE_PCP) */

extern unsigned long int min_lifetime;
extern unsigned long int max_lifetime;

/*! \brief runtime boolean flags */
extern int runtime_flags;
/*! \brief enable packet log (pf/ipf) */
#define LOGPACKETSMASK		0x0001
/*! \brief report system uptime */
#define SYSUPTIMEMASK		0x0002
#ifdef ENABLE_NATPMP
/*! \brief enable NAT-PMP and PCP */
#define ENABLENATPMPMASK	0x0004
#endif
/*! \todo unused. to remove ? */
#define CHECKCLIENTIPMASK	0x0008
/*! \brief enable secure mode */
#define SECUREMODEMASK		0x0010

/*! \brief enable UPnP */
#define ENABLEUPNPMASK		0x0020

#ifdef PF_ENABLE_FILTER_RULES
#define PFNOQUICKRULESMASK	0x0040
#endif
#ifdef ENABLE_IPV6
/*! \brief disable IPv6 */
#define IPV6DISABLEDMASK	0x0080
#endif
#ifdef ENABLE_6FC_SERVICE
#define IPV6FCFWDISABLEDMASK		0x0100
#define IPV6FCINBOUNDDISALLOWEDMASK	0x0200
#endif
#ifdef ENABLE_PCP
/*! \brief allow THIRD PARTY PCP OPTION */
#define PCP_ALLOWTHIRDPARTYMASK	0x0400
#endif
#ifdef IGD_V2
#define FORCEIGDDESCV1MASK 0x0800
#endif

/*! \brief perform STUN */
#define PERFORMSTUNMASK    0x1000

#ifdef ENABLE_AURASYNC
#define ENABLEAURASYNCMASK	0x2000
#define	AS_RGB_MIN	0
#define	AS_RGB_MAX	255
#define	AS_GROUP_MIN	0
#define	AS_GROUP_MAX	7
#define	AS_MODE_MIN	0
#define	AS_MODE_MAX	13
#define	AS_DIR_MIN	0
#define	AS_DIR_MAX	1
extern int aura_standalone;
#endif

#ifdef ENABLE_NVGFN
#define ENABLENVGFNMASK				0x4000
#define NVGFN_QOSPORT_MIN			1024
#define NVGFN_QOSPORT_MAX			65535
#define NVGFN_MCSINDEX_MIN			0
#define NVGFN_MCSINDEX_MAX			23
#define NVGFN_SPATIALSTREAMS_MIN	1
#define NVGFN_SPATIALSTREAMS_MAX	3
#define NVGFN_BANDWIDTH_MIN			1
#define NVGFN_BANDWIDTH_MAX			65535
#define NVGFN_WIFISCANINTERVAL_MIN	0
#define NVGFN_WIFISCANINTERVAL_MAX	65535
extern int gfn_only;
#endif

/*! \brief Allow the external IPv4 to be private */
#define ALLOWPRIVATEIPV4MASK 0x2000
/*! \brief do not stop on STUN detecting restricive NAT */
#define ALLOWFILTEREDSTUNMASK 0x4000

#define SETFLAG(mask)	runtime_flags |= mask
#define GETFLAG(mask)	(runtime_flags & mask)
#define CLEARFLAG(mask)	runtime_flags &= ~mask

extern const char * pidfilename;

extern char uuidvalue_igd[];	/*!< uuid of root device (IGD) */
extern char uuidvalue_wan[];	/*!< uuid of WAN Device */
extern char uuidvalue_wcd[];	/*!< uuid of WAN Connection Device */

#define SERIALNUMBER_MAX_LEN (48)
extern char serialnumber[];

#define MODELNUMBER_MAX_LEN (48)
extern char modelnumber[];

#define PRESENTATIONURL_MAX_LEN (64)
extern char presentationurl[];

#ifdef ENABLE_MANUFACTURER_INFO_CONFIGURATION
#define FRIENDLY_NAME_MAX_LEN (64)
extern char friendly_name[];

#define MANUFACTURER_NAME_MAX_LEN (64)
extern char manufacturer_name[];

#define MANUFACTURER_URL_MAX_LEN (64)
extern char manufacturer_url[];

#define MODEL_NAME_MAX_LEN (64)
extern char model_name[];

#define MODEL_DESCRIPTION_MAX_LEN (64)
extern char model_description[];

#define MODEL_URL_MAX_LEN (64)
extern char model_url[];
#endif

/*! \brief UPnP permission rules */
extern struct upnpperm * upnppermlist;
/*! \brief number of UPnP permission rules */
extern unsigned int num_upnpperm;

#ifdef PCP_SADSCP
extern struct dscp_values* dscp_values_list;
extern unsigned int num_dscp_values;
#endif

/*! \brief For automatic removal of expired rules (with LeaseDuration) */
extern unsigned int nextruletoclean_timestamp;

#ifdef USE_PF
extern const char * anchor_name;
/* queue and tag for PF rules */
extern const char * queue;
extern const char * tag;
#endif

#ifdef ENABLE_NFQUEUE
#define MAX_LAN_ADDR (8)
extern int nfqueue;
extern int n_nfqix;
extern unsigned nfqix[];
#endif

/*! \brief lan addresses to listen to SSDP traffic */
extern struct lan_addr_list lan_addrs;

#ifdef ENABLE_IPV6
/*! \brief ipv6 address used for HTTP */
extern char ipv6_addr_for_http_with_brackets[64];

/*! \brief address used to bind local services */
extern struct in6_addr ipv6_bind_addr;

#endif /* ENABLE_IPV6 */

/*! \brief path to the minissdpd unix socket */
extern const char * minissdpdsocketpath;

/*! \brief BOOTID.UPNP.ORG */
extern unsigned int upnp_bootid;
/*! \brief CONFIGID.UPNP.ORG */
extern unsigned int upnp_configid;

#ifdef RANDOMIZE_URLS
#define RANDOM_URL_MAX_LEN (16)
extern char random_url[];
#endif /* RANDOMIZE_URLS */

#ifdef DYNAMIC_OS_VERSION
extern char * os_version;
#endif

#endif /* UPNPGLOBALVARS_H_INCLUDED */
