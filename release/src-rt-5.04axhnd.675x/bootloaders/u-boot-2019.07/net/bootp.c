/*
 *	Based on LiMon - BOOTP.
 *
 *	Copyright 1994, 1995, 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000 Roland Borde
 *	Copyright 2000 Paolo Scaffardi
 *	Copyright 2000-2004 Wolfgang Denk, wd@denx.de
 */

#include <common.h>
#include <command.h>
#include <efi_loader.h>
#include <net.h>
#include <net/tftp.h>
#include "bootp.h"
#ifdef CONFIG_LED_STATUS
#include <status_led.h>
#endif
#ifdef CONFIG_BOOTP_RANDOM_DELAY
#include "net_rand.h"
#endif

#define BOOTP_VENDOR_MAGIC	0x63825363	/* RFC1048 Magic Cookie */

/*
 * The timeout for the initial BOOTP/DHCP request used to be described by a
 * counter of fixed-length timeout periods. TIMEOUT_COUNT represents
 * that counter
 *
 * Now that the timeout periods are variable (exponential backoff and retry)
 * we convert the timeout count to the absolute time it would have take to
 * execute that many retries, and keep sending retry packets until that time
 * is reached.
 */
#ifndef CONFIG_NET_RETRY_COUNT
# define TIMEOUT_COUNT	5		/* # of timeouts before giving up */
#else
# define TIMEOUT_COUNT	(CONFIG_NET_RETRY_COUNT)
#endif
#define TIMEOUT_MS	((3 + (TIMEOUT_COUNT * 5)) * 1000)

#define PORT_BOOTPS	67		/* BOOTP server UDP port */
#define PORT_BOOTPC	68		/* BOOTP client UDP port */

#ifndef CONFIG_DHCP_MIN_EXT_LEN		/* minimal length of extension list */
#define CONFIG_DHCP_MIN_EXT_LEN 64
#endif

#ifndef CONFIG_BOOTP_ID_CACHE_SIZE
#define CONFIG_BOOTP_ID_CACHE_SIZE 4
#endif

u32		bootp_ids[CONFIG_BOOTP_ID_CACHE_SIZE];
unsigned int	bootp_num_ids;
int		bootp_try;
ulong		bootp_start;
ulong		bootp_timeout;
char net_nis_domain[32] = {0,}; /* Our NIS domain */
char net_hostname[32] = {0,}; /* Our hostname */
char net_root_path[64] = {0,}; /* Our bootpath */

static ulong time_taken_max;

#if defined(CONFIG_CMD_DHCP)
static dhcp_state_t dhcp_state = INIT;
static u32 dhcp_leasetime;
static struct in_addr dhcp_server_ip;
static u8 dhcp_option_overload;
#define OVERLOAD_FILE 1
#define OVERLOAD_SNAME 2
static void dhcp_handler(uchar *pkt, unsigned dest, struct in_addr sip,
			unsigned src, unsigned len);

/* For Debug */
#if 0
static char *dhcpmsg2str(int type)
{
	switch (type) {
	case 1:	 return "DHCPDISCOVER"; break;
	case 2:	 return "DHCPOFFER";	break;
	case 3:	 return "DHCPREQUEST";	break;
	case 4:	 return "DHCPDECLINE";	break;
	case 5:	 return "DHCPACK";	break;
	case 6:	 return "DHCPNACK";	break;
	case 7:	 return "DHCPRELEASE";	break;
	default: return "UNKNOWN/INVALID MSG TYPE"; break;
	}
}
#endif
#endif

static void bootp_add_id(ulong id)
{
	if (bootp_num_ids >= ARRAY_SIZE(bootp_ids)) {
		size_t size = sizeof(bootp_ids) - sizeof(id);

		memmove(bootp_ids, &bootp_ids[1], size);
		bootp_ids[bootp_num_ids - 1] = id;
	} else {
		bootp_ids[bootp_num_ids] = id;
		bootp_num_ids++;
	}
}

static bool bootp_match_id(ulong id)
{
	unsigned int i;

	for (i = 0; i < bootp_num_ids; i++)
		if (bootp_ids[i] == id)
			return true;

	return false;
}

static int check_reply_packet(uchar *pkt, unsigned dest, unsigned src,
			      unsigned len)
{
	struct bootp_hdr *bp = (struct bootp_hdr *)pkt;
	int retval = 0;

	if (dest != PORT_BOOTPC || src != PORT_BOOTPS)
		retval = -1;
	else if (len < sizeof(struct bootp_hdr) - OPT_FIELD_SIZE)
		retval = -2;
	else if (bp->bp_op != OP_BOOTREPLY)
		retval = -3;
	else if (bp->bp_htype != HWT_ETHER)
		retval = -4;
	else if (bp->bp_hlen != HWL_ETHER)
		retval = -5;
	else if (!bootp_match_id(net_read_u32(&bp->bp_id)))
		retval = -6;
	else if (memcmp(bp->bp_chaddr, net_ethaddr, HWL_ETHER) != 0)
		retval = -7;

	debug("Filtering pkt = %d\n", retval);

	return retval;
}

/*
 * Copy parameters of interest from BOOTP_REPLY/DHCP_OFFER packet
 */
static void store_net_params(struct bootp_hdr *bp)
{
#if !defined(CONFIG_BOOTP_SERVERIP)
	struct in_addr tmp_ip;
	bool overwrite_serverip = true;

#if defined(CONFIG_BOOTP_PREFER_SERVERIP)
	overwrite_serverip = false;
#endif

	net_copy_ip(&tmp_ip, &bp->bp_siaddr);
	if (tmp_ip.s_addr != 0 && (overwrite_serverip || !net_server_ip.s_addr))
		net_copy_ip(&net_server_ip, &bp->bp_siaddr);
	memcpy(net_server_ethaddr,
	       ((struct ethernet_hdr *)net_rx_packet)->et_src, 6);
	if (
#if defined(CONFIG_CMD_DHCP)
	    !(dhcp_option_overload & OVERLOAD_FILE) &&
#endif
	    (strlen(bp->bp_file) > 0) &&
	    !net_boot_file_name_explicit) {
		copy_filename(net_boot_file_name, bp->bp_file,
			      sizeof(net_boot_file_name));
	}

	debug("net_boot_file_name: %s\n", net_boot_file_name);

	/* Propagate to environment:
	 * don't delete exising entry when BOOTP / DHCP reply does
	 * not contain a new value
	 */
	if (*net_boot_file_name)
		env_set("bootfile", net_boot_file_name);
#endif
	net_copy_ip(&net_ip, &bp->bp_yiaddr);
}

static int truncate_sz(const char *name, int maxlen, int curlen)
{
	if (curlen >= maxlen) {
		printf("*** WARNING: %s is too long (%d - max: %d)"
			" - truncated\n", name, curlen, maxlen);
		curlen = maxlen - 1;
	}
	return curlen;
}

#if !defined(CONFIG_CMD_DHCP)

static void bootp_process_vendor_field(u8 *ext)
{
	int size = *(ext + 1);

	debug("[BOOTP] Processing extension %d... (%d bytes)\n", *ext,
	      *(ext + 1));

	net_boot_file_expected_size_in_blocks = 0;

	switch (*ext) {
		/* Fixed length fields */
	case 1:			/* Subnet mask */
		if (net_netmask.s_addr == 0)
			net_copy_ip(&net_netmask, (struct in_addr *)(ext + 2));
		break;
	case 2:			/* Time offset - Not yet supported */
		break;
		/* Variable length fields */
	case 3:			/* Gateways list */
		if (net_gateway.s_addr == 0)
			net_copy_ip(&net_gateway, (struct in_addr *)(ext + 2));
		break;
	case 4:			/* Time server - Not yet supported */
		break;
	case 5:			/* IEN-116 name server - Not yet supported */
		break;
	case 6:
		if (net_dns_server.s_addr == 0)
			net_copy_ip(&net_dns_server,
				    (struct in_addr *)(ext + 2));
#if defined(CONFIG_BOOTP_DNS2)
		if ((net_dns_server2.s_addr == 0) && (size > 4))
			net_copy_ip(&net_dns_server2,
				    (struct in_addr *)(ext + 2 + 4));
#endif
		break;
	case 7:			/* Log server - Not yet supported */
		break;
	case 8:			/* Cookie/Quote server - Not yet supported */
		break;
	case 9:			/* LPR server - Not yet supported */
		break;
	case 10:		/* Impress server - Not yet supported */
		break;
	case 11:		/* RPL server - Not yet supported */
		break;
	case 12:		/* Host name */
		if (net_hostname[0] == 0) {
			size = truncate_sz("Host Name",
				sizeof(net_hostname), size);
			memcpy(&net_hostname, ext + 2, size);
			net_hostname[size] = 0;
		}
		break;
	case 13:		/* Boot file size */
		if (size == 2)
			net_boot_file_expected_size_in_blocks =
				ntohs(*(ushort *)(ext + 2));
		else if (size == 4)
			net_boot_file_expected_size_in_blocks =
				ntohl(*(ulong *)(ext + 2));
		break;
	case 14:		/* Merit dump file - Not yet supported */
		break;
	case 15:		/* Domain name - Not yet supported */
		break;
	case 16:		/* Swap server - Not yet supported */
		break;
	case 17:		/* Root path */
		if (net_root_path[0] == 0) {
			size = truncate_sz("Root Path",
				sizeof(net_root_path), size);
			memcpy(&net_root_path, ext + 2, size);
			net_root_path[size] = 0;
		}
		break;
	case 18:		/* Extension path - Not yet supported */
		/*
		 * This can be used to send the information of the
		 * vendor area in another file that the client can
		 * access via TFTP.
		 */
		break;
		/* IP host layer fields */
	case 40:		/* NIS Domain name */
		if (net_nis_domain[0] == 0) {
			size = truncate_sz("NIS Domain Name",
				sizeof(net_nis_domain), size);
			memcpy(&net_nis_domain, ext + 2, size);
			net_nis_domain[size] = 0;
		}
		break;
#if defined(CONFIG_CMD_SNTP) && defined(CONFIG_BOOTP_NTPSERVER)
	case 42:	/* NTP server IP */
		net_copy_ip(&net_ntp_server, (struct in_addr *)(ext + 2));
		break;
#endif
		/* Application layer fields */
	case 43:		/* Vendor specific info - Not yet supported */
		/*
		 * Binary information to exchange specific
		 * product information.
		 */
		break;
		/* Reserved (custom) fields (128..254) */
	}
}

static void bootp_process_vendor(u8 *ext, int size)
{
	u8 *end = ext + size;

	debug("[BOOTP] Checking extension (%d bytes)...\n", size);

	while ((ext < end) && (*ext != 0xff)) {
		if (*ext == 0) {
			ext++;
		} else {
			u8 *opt = ext;

			ext += ext[1] + 2;
			if (ext <= end)
				bootp_process_vendor_field(opt);
		}
	}

	debug("[BOOTP] Received fields:\n");
	if (net_netmask.s_addr)
		debug("net_netmask : %pI4\n", &net_netmask);

	if (net_gateway.s_addr)
		debug("net_gateway	: %pI4", &net_gateway);

	if (net_boot_file_expected_size_in_blocks)
		debug("net_boot_file_expected_size_in_blocks : %d\n",
		      net_boot_file_expected_size_in_blocks);

	if (net_hostname[0])
		debug("net_hostname  : %s\n", net_hostname);

	if (net_root_path[0])
		debug("net_root_path  : %s\n", net_root_path);

	if (net_nis_domain[0])
		debug("net_nis_domain : %s\n", net_nis_domain);

#if defined(CONFIG_CMD_SNTP) && defined(CONFIG_BOOTP_NTPSERVER)
	if (net_ntp_server.s_addr)
		debug("net_ntp_server : %pI4\n", &net_ntp_server);
#endif
}

/*
 *	Handle a BOOTP received packet.
 */
static void bootp_handler(uchar *pkt, unsigned dest, struct in_addr sip,
			  unsigned src, unsigned len)
{
	struct bootp_hdr *bp;

	debug("got BOOTP packet (src=%d, dst=%d, len=%d want_len=%zu)\n",
	      src, dest, len, sizeof(struct bootp_hdr));

	bp = (struct bootp_hdr *)pkt;

	/* Filter out pkts we don't want */
	if (check_reply_packet(pkt, dest, src, len))
		return;

	/*
	 *	Got a good BOOTP reply.	 Copy the data into our variables.
	 */
#if defined(CONFIG_LED_STATUS) && defined(CONFIG_LED_STATUS_BOOT_ENABLE)
	status_led_set(CONFIG_LED_STATUS_BOOT, CONFIG_LED_STATUS_OFF);
#endif

	store_net_params(bp);		/* Store net parameters from reply */

	/* Retrieve extended information (we must parse the vendor area) */
	if (net_read_u32((u32 *)&bp->bp_vend[0]) == htonl(BOOTP_VENDOR_MAGIC))
		bootp_process_vendor((uchar *)&bp->bp_vend[4], len);

	net_set_timeout_handler(0, (thand_f *)0);
	bootstage_mark_name(BOOTSTAGE_ID_BOOTP_STOP, "bootp_stop");

	debug("Got good BOOTP\n");

	net_auto_load();
}
#endif

/*
 *	Timeout on BOOTP/DHCP request.
 */
static void bootp_timeout_handler(void)
{
	ulong time_taken = get_timer(bootp_start);

	if (time_taken >= time_taken_max) {
#ifdef CONFIG_BOOTP_MAY_FAIL
		char *ethrotate;

		ethrotate = env_get("ethrotate");
		if ((ethrotate && strcmp(ethrotate, "no") == 0) ||
		    net_restart_wrap) {
			puts("\nRetry time exceeded\n");
			net_set_state(NETLOOP_FAIL);
		} else
#endif
		{
			puts("\nRetry time exceeded; starting again\n");
			net_start_again();
		}
	} else {
		bootp_timeout *= 2;
		if (bootp_timeout > 2000)
			bootp_timeout = 2000;
		net_set_timeout_handler(bootp_timeout, bootp_timeout_handler);
		bootp_request();
	}
}

#define put_vci(e, str)						\
	do {							\
		size_t vci_strlen = strlen(str);		\
		*e++ = 60;	/* Vendor Class Identifier */	\
		*e++ = vci_strlen;				\
		memcpy(e, str, vci_strlen);			\
		e += vci_strlen;				\
	} while (0)

static u8 *add_vci(u8 *e)
{
	char *vci = NULL;
	char *env_vci = env_get("bootp_vci");

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_NET_VCI_STRING)
	vci = CONFIG_SPL_NET_VCI_STRING;
#elif defined(CONFIG_BOOTP_VCI_STRING)
	vci = CONFIG_BOOTP_VCI_STRING;
#endif

	if (env_vci)
		vci = env_vci;

	if (vci)
		put_vci(e, vci);

	return e;
}

/*
 *	Initialize BOOTP extension fields in the request.
 */
#if defined(CONFIG_CMD_DHCP)
static int dhcp_extended(u8 *e, int message_type, struct in_addr server_ip,
			struct in_addr requested_ip)
{
	u8 *start = e;
	u8 *cnt;
#ifdef CONFIG_LIB_UUID
	char *uuid;
#endif
	int clientarch = -1;

#if defined(CONFIG_BOOTP_VENDOREX)
	u8 *x;
#endif
#if defined(CONFIG_BOOTP_SEND_HOSTNAME)
	char *hostname;
#endif

	*e++ = 99;		/* RFC1048 Magic Cookie */
	*e++ = 130;
	*e++ = 83;
	*e++ = 99;

	*e++ = 53;		/* DHCP Message Type */
	*e++ = 1;
	*e++ = message_type;

	*e++ = 57;		/* Maximum DHCP Message Size */
	*e++ = 2;
	*e++ = (576 - 312 + OPT_FIELD_SIZE) >> 8;
	*e++ = (576 - 312 + OPT_FIELD_SIZE) & 0xff;

	if (server_ip.s_addr) {
		int tmp = ntohl(server_ip.s_addr);

		*e++ = 54;	/* ServerID */
		*e++ = 4;
		*e++ = tmp >> 24;
		*e++ = tmp >> 16;
		*e++ = tmp >> 8;
		*e++ = tmp & 0xff;
	}

	if (requested_ip.s_addr) {
		int tmp = ntohl(requested_ip.s_addr);

		*e++ = 50;	/* Requested IP */
		*e++ = 4;
		*e++ = tmp >> 24;
		*e++ = tmp >> 16;
		*e++ = tmp >> 8;
		*e++ = tmp & 0xff;
	}
#if defined(CONFIG_BOOTP_SEND_HOSTNAME)
	hostname = env_get("hostname");
	if (hostname) {
		int hostnamelen = strlen(hostname);

		*e++ = 12;	/* Hostname */
		*e++ = hostnamelen;
		memcpy(e, hostname, hostnamelen);
		e += hostnamelen;
	}
#endif

#ifdef CONFIG_BOOTP_PXE_CLIENTARCH
	clientarch = CONFIG_BOOTP_PXE_CLIENTARCH;
#endif

	if (env_get("bootp_arch"))
		clientarch = env_get_ulong("bootp_arch", 16, clientarch);

	if (clientarch > 0) {
		*e++ = 93;	/* Client System Architecture */
		*e++ = 2;
		*e++ = (clientarch >> 8) & 0xff;
		*e++ = clientarch & 0xff;
	}

	*e++ = 94;	/* Client Network Interface Identifier */
	*e++ = 3;
	*e++ = 1;	/* type field for UNDI */
	*e++ = 0;	/* major revision */
	*e++ = 0;	/* minor revision */

#ifdef CONFIG_LIB_UUID
	uuid = env_get("pxeuuid");

	if (uuid) {
		if (uuid_str_valid(uuid)) {
			*e++ = 97;	/* Client Machine Identifier */
			*e++ = 17;
			*e++ = 0;	/* type 0 - UUID */

			uuid_str_to_bin(uuid, e, UUID_STR_FORMAT_STD);
			e += 16;
		} else {
			printf("Invalid pxeuuid: %s\n", uuid);
		}
	}
#endif

	e = add_vci(e);

#if defined(CONFIG_BOOTP_VENDOREX)
	x = dhcp_vendorex_prep(e);
	if (x)
		return x - start;
#endif

	*e++ = 55;		/* Parameter Request List */
	 cnt = e++;		/* Pointer to count of requested items */
	*cnt = 0;
#if defined(CONFIG_BOOTP_SUBNETMASK)
	*e++  = 1;		/* Subnet Mask */
	*cnt += 1;
#endif
#if defined(CONFIG_BOOTP_TIMEOFFSET)
	*e++  = 2;
	*cnt += 1;
#endif
#if defined(CONFIG_BOOTP_GATEWAY)
	*e++  = 3;		/* Router Option */
	*cnt += 1;
#endif
#if defined(CONFIG_BOOTP_DNS)
	*e++  = 6;		/* DNS Server(s) */
	*cnt += 1;
#endif
#if defined(CONFIG_BOOTP_HOSTNAME)
	*e++  = 12;		/* Hostname */
	*cnt += 1;
#endif
#if defined(CONFIG_BOOTP_BOOTFILESIZE)
	*e++  = 13;		/* Boot File Size */
	*cnt += 1;
#endif
#if defined(CONFIG_BOOTP_BOOTPATH)
	*e++  = 17;		/* Boot path */
	*cnt += 1;
#endif
#if defined(CONFIG_BOOTP_NISDOMAIN)
	*e++  = 40;		/* NIS Domain name request */
	*cnt += 1;
#endif
#if defined(CONFIG_BOOTP_NTPSERVER)
	*e++  = 42;
	*cnt += 1;
#endif
	/* no options, so back up to avoid sending an empty request list */
	if (*cnt == 0)
		e -= 2;

	*e++  = 255;		/* End of the list */

	/* Pad to minimal length */
#ifdef	CONFIG_DHCP_MIN_EXT_LEN
	while ((e - start) < CONFIG_DHCP_MIN_EXT_LEN)
		*e++ = 0;
#endif

	return e - start;
}

#else
/*
 * Warning: no field size check - change CONFIG_BOOTP_* at your own risk!
 */
static int bootp_extended(u8 *e)
{
	u8 *start = e;

	*e++ = 99;		/* RFC1048 Magic Cookie */
	*e++ = 130;
	*e++ = 83;
	*e++ = 99;

#if defined(CONFIG_CMD_DHCP)
	*e++ = 53;		/* DHCP Message Type */
	*e++ = 1;
	*e++ = DHCP_DISCOVER;

	*e++ = 57;		/* Maximum DHCP Message Size */
	*e++ = 2;
	*e++ = (576 - 312 + OPT_FIELD_SIZE) >> 16;
	*e++ = (576 - 312 + OPT_FIELD_SIZE) & 0xff;
#endif

	add_vci(e);

#if defined(CONFIG_BOOTP_SUBNETMASK)
	*e++ = 1;		/* Subnet mask request */
	*e++ = 4;
	e   += 4;
#endif

#if defined(CONFIG_BOOTP_GATEWAY)
	*e++ = 3;		/* Default gateway request */
	*e++ = 4;
	e   += 4;
#endif

#if defined(CONFIG_BOOTP_DNS)
	*e++ = 6;		/* Domain Name Server */
	*e++ = 4;
	e   += 4;
#endif

#if defined(CONFIG_BOOTP_HOSTNAME)
	*e++ = 12;		/* Host name request */
	*e++ = 32;
	e   += 32;
#endif

#if defined(CONFIG_BOOTP_BOOTFILESIZE)
	*e++ = 13;		/* Boot file size */
	*e++ = 2;
	e   += 2;
#endif

#if defined(CONFIG_BOOTP_BOOTPATH)
	*e++ = 17;		/* Boot path */
	*e++ = 32;
	e   += 32;
#endif

#if defined(CONFIG_BOOTP_NISDOMAIN)
	*e++ = 40;		/* NIS Domain name request */
	*e++ = 32;
	e   += 32;
#endif
#if defined(CONFIG_BOOTP_NTPSERVER)
	*e++ = 42;
	*e++ = 4;
	e   += 4;
#endif

	*e++ = 255;		/* End of the list */

	/*
	 * If nothing in list, remove it altogether. Some DHCP servers get
	 * upset by this minor faux pas and do not respond at all.
	 */
	if (e == start + 3) {
		printf("*** Warning: no DHCP options requested\n");
		e -= 3;
	}

	return e - start;
}
#endif

void bootp_reset(void)
{
	bootp_num_ids = 0;
	bootp_try = 0;
	bootp_start = get_timer(0);
	bootp_timeout = 250;
}

void bootp_request(void)
{
	uchar *pkt, *iphdr;
	struct bootp_hdr *bp;
	int extlen, pktlen, iplen;
	int eth_hdr_size;
#ifdef CONFIG_BOOTP_RANDOM_DELAY
	ulong rand_ms;
#endif
	u32 bootp_id;
	struct in_addr zero_ip;
	struct in_addr bcast_ip;
	char *ep;  /* Environment pointer */

	bootstage_mark_name(BOOTSTAGE_ID_BOOTP_START, "bootp_start");
#if defined(CONFIG_CMD_DHCP)
	dhcp_state = INIT;
#endif

	ep = env_get("bootpretryperiod");
	if (ep != NULL)
		time_taken_max = simple_strtoul(ep, NULL, 10);
	else
		time_taken_max = TIMEOUT_MS;

#ifdef CONFIG_BOOTP_RANDOM_DELAY		/* Random BOOTP delay */
	if (bootp_try == 0)
		srand_mac();

	if (bootp_try <= 2)	/* Start with max 1024 * 1ms */
		rand_ms = rand() >> (22 - bootp_try);
	else		/* After 3rd BOOTP request max 8192 * 1ms */
		rand_ms = rand() >> 19;

	printf("Random delay: %ld ms...\n", rand_ms);
	mdelay(rand_ms);

#endif	/* CONFIG_BOOTP_RANDOM_DELAY */

	printf("BOOTP broadcast %d\n", ++bootp_try);
	pkt = net_tx_packet;
	memset((void *)pkt, 0, PKTSIZE);

	eth_hdr_size = net_set_ether(pkt, net_bcast_ethaddr, PROT_IP);
	pkt += eth_hdr_size;

	/*
	 * Next line results in incorrect packet size being transmitted,
	 * resulting in errors in some DHCP servers, reporting missing bytes.
	 * Size must be set in packet header after extension length has been
	 * determined.
	 * C. Hallinan, DS4.COM, Inc.
	 */
	/* net_set_udp_header(pkt, 0xFFFFFFFFL, PORT_BOOTPS, PORT_BOOTPC,
		sizeof (struct bootp_hdr)); */
	iphdr = pkt;	/* We need this later for net_set_udp_header() */
	pkt += IP_UDP_HDR_SIZE;

	bp = (struct bootp_hdr *)pkt;
	bp->bp_op = OP_BOOTREQUEST;
	bp->bp_htype = HWT_ETHER;
	bp->bp_hlen = HWL_ETHER;
	bp->bp_hops = 0;
	/*
	 * according to RFC1542, should be 0 on first request, secs since
	 * first request otherwise
	 */
	bp->bp_secs = htons(get_timer(bootp_start) / 1000);
	zero_ip.s_addr = 0;
	net_write_ip(&bp->bp_ciaddr, zero_ip);
	net_write_ip(&bp->bp_yiaddr, zero_ip);
	net_write_ip(&bp->bp_siaddr, zero_ip);
	net_write_ip(&bp->bp_giaddr, zero_ip);
	memcpy(bp->bp_chaddr, net_ethaddr, 6);
	copy_filename(bp->bp_file, net_boot_file_name, sizeof(bp->bp_file));

	/* Request additional information from the BOOTP/DHCP server */
#if defined(CONFIG_CMD_DHCP)
	extlen = dhcp_extended((u8 *)bp->bp_vend, DHCP_DISCOVER, zero_ip,
			       zero_ip);
#else
	extlen = bootp_extended((u8 *)bp->bp_vend);
#endif

	/*
	 *	Bootp ID is the lower 4 bytes of our ethernet address
	 *	plus the current time in ms.
	 */
	bootp_id = ((u32)net_ethaddr[2] << 24)
		| ((u32)net_ethaddr[3] << 16)
		| ((u32)net_ethaddr[4] << 8)
		| (u32)net_ethaddr[5];
	bootp_id += get_timer(0);
	bootp_id = htonl(bootp_id);
	bootp_add_id(bootp_id);
	net_copy_u32(&bp->bp_id, &bootp_id);

	/*
	 * Calculate proper packet lengths taking into account the
	 * variable size of the options field
	 */
	iplen = BOOTP_HDR_SIZE - OPT_FIELD_SIZE + extlen;
	pktlen = eth_hdr_size + IP_UDP_HDR_SIZE + iplen;
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_set_udp_header(iphdr, bcast_ip, PORT_BOOTPS, PORT_BOOTPC, iplen);
	net_set_timeout_handler(bootp_timeout, bootp_timeout_handler);

#if defined(CONFIG_CMD_DHCP)
	dhcp_state = SELECTING;
	net_set_udp_handler(dhcp_handler);
#else
	net_set_udp_handler(bootp_handler);
#endif
	net_send_packet(net_tx_packet, pktlen);
}

#if defined(CONFIG_CMD_DHCP)
static void dhcp_process_options(uchar *popt, uchar *end)
{
	int oplen, size;
#if defined(CONFIG_CMD_SNTP) && defined(CONFIG_BOOTP_TIMEOFFSET)
	int *to_ptr;
#endif

	while (popt < end && *popt != 0xff) {
		oplen = *(popt + 1);
		switch (*popt) {
		case 0:
			oplen = -1; /* Pad omits len byte */
			break;
		case 1:
			net_copy_ip(&net_netmask, (popt + 2));
			break;
#if defined(CONFIG_CMD_SNTP) && defined(CONFIG_BOOTP_TIMEOFFSET)
		case 2:		/* Time offset	*/
			to_ptr = &net_ntp_time_offset;
			net_copy_u32((u32 *)to_ptr, (u32 *)(popt + 2));
			net_ntp_time_offset = ntohl(net_ntp_time_offset);
			break;
#endif
		case 3:
			net_copy_ip(&net_gateway, (popt + 2));
			break;
		case 6:
			net_copy_ip(&net_dns_server, (popt + 2));
#if defined(CONFIG_BOOTP_DNS2)
			if (*(popt + 1) > 4)
				net_copy_ip(&net_dns_server2, (popt + 2 + 4));
#endif
			break;
		case 12:
			size = truncate_sz("Host Name",
				sizeof(net_hostname), oplen);
			memcpy(&net_hostname, popt + 2, size);
			net_hostname[size] = 0;
			break;
		case 15:	/* Ignore Domain Name Option */
			break;
		case 17:
			size = truncate_sz("Root Path",
				sizeof(net_root_path), oplen);
			memcpy(&net_root_path, popt + 2, size);
			net_root_path[size] = 0;
			break;
		case 28:	/* Ignore Broadcast Address Option */
			break;
#if defined(CONFIG_CMD_SNTP) && defined(CONFIG_BOOTP_NTPSERVER)
		case 42:	/* NTP server IP */
			net_copy_ip(&net_ntp_server, (popt + 2));
			break;
#endif
		case 51:
			net_copy_u32(&dhcp_leasetime, (u32 *)(popt + 2));
			break;
		case 52:
			dhcp_option_overload = popt[2];
			break;
		case 53:	/* Ignore Message Type Option */
			break;
		case 54:
			net_copy_ip(&dhcp_server_ip, (popt + 2));
			break;
		case 58:	/* Ignore Renewal Time Option */
			break;
		case 59:	/* Ignore Rebinding Time Option */
			break;
		case 66:	/* Ignore TFTP server name */
			break;
		case 67:	/* Bootfile option */
			if (!net_boot_file_name_explicit) {
				size = truncate_sz("Bootfile",
						   sizeof(net_boot_file_name),
						   oplen);
				memcpy(&net_boot_file_name, popt + 2, size);
				net_boot_file_name[size] = 0;
			}
			break;
		default:
#if defined(CONFIG_BOOTP_VENDOREX)
			if (dhcp_vendorex_proc(popt))
				break;
#endif
			printf("*** Unhandled DHCP Option in OFFER/ACK:"
			       " %d\n", *popt);
			break;
		}
		popt += oplen + 2;	/* Process next option */
	}
}

static void dhcp_packet_process_options(struct bootp_hdr *bp)
{
	uchar *popt = (uchar *)&bp->bp_vend[4];
	uchar *end = popt + BOOTP_HDR_SIZE;

	if (net_read_u32((u32 *)&bp->bp_vend[0]) != htonl(BOOTP_VENDOR_MAGIC))
		return;

	dhcp_option_overload = 0;

	/*
	 * The 'options' field MUST be interpreted first, 'file' next,
	 * 'sname' last.
	 */
	dhcp_process_options(popt, end);

	if (dhcp_option_overload & OVERLOAD_FILE) {
		popt = (uchar *)bp->bp_file;
		end = popt + sizeof(bp->bp_file);
		dhcp_process_options(popt, end);
	}

	if (dhcp_option_overload & OVERLOAD_SNAME) {
		popt = (uchar *)bp->bp_sname;
		end = popt + sizeof(bp->bp_sname);
		dhcp_process_options(popt, end);
	}
}

static int dhcp_message_type(unsigned char *popt)
{
	if (net_read_u32((u32 *)popt) != htonl(BOOTP_VENDOR_MAGIC))
		return -1;

	popt += 4;
	while (*popt != 0xff) {
		if (*popt == 53)	/* DHCP Message Type */
			return *(popt + 2);
		if (*popt == 0)	{
			/* Pad */
			popt += 1;
		} else {
			/* Scan through all options */
			popt += *(popt + 1) + 2;
		}
	}
	return -1;
}

static void dhcp_send_request_packet(struct bootp_hdr *bp_offer)
{
	uchar *pkt, *iphdr;
	struct bootp_hdr *bp;
	int pktlen, iplen, extlen;
	int eth_hdr_size;
	struct in_addr offered_ip;
	struct in_addr zero_ip;
	struct in_addr bcast_ip;

	debug("dhcp_send_request_packet: Sending DHCPREQUEST\n");
	pkt = net_tx_packet;
	memset((void *)pkt, 0, PKTSIZE);

	eth_hdr_size = net_set_ether(pkt, net_bcast_ethaddr, PROT_IP);
	pkt += eth_hdr_size;

	iphdr = pkt;	/* We'll need this later to set proper pkt size */
	pkt += IP_UDP_HDR_SIZE;

	bp = (struct bootp_hdr *)pkt;
	bp->bp_op = OP_BOOTREQUEST;
	bp->bp_htype = HWT_ETHER;
	bp->bp_hlen = HWL_ETHER;
	bp->bp_hops = 0;
	bp->bp_secs = htons(get_timer(bootp_start) / 1000);
	/* Do not set the client IP, your IP, or server IP yet, since it
	 * hasn't been ACK'ed by the server yet */

	/*
	 * RFC3046 requires Relay Agents to discard packets with
	 * nonzero and offered giaddr
	 */
	zero_ip.s_addr = 0;
	net_write_ip(&bp->bp_giaddr, zero_ip);

	memcpy(bp->bp_chaddr, net_ethaddr, 6);
	copy_filename(bp->bp_file, net_boot_file_name, sizeof(bp->bp_file));

	/*
	 * ID is the id of the OFFER packet
	 */

	net_copy_u32(&bp->bp_id, &bp_offer->bp_id);

	/*
	 * Copy options from OFFER packet if present
	 */

	/* Copy offered IP into the parameters request list */
	net_copy_ip(&offered_ip, &bp_offer->bp_yiaddr);
	extlen = dhcp_extended((u8 *)bp->bp_vend, DHCP_REQUEST,
		dhcp_server_ip, offered_ip);

	iplen = BOOTP_HDR_SIZE - OPT_FIELD_SIZE + extlen;
	pktlen = eth_hdr_size + IP_UDP_HDR_SIZE + iplen;
	bcast_ip.s_addr = 0xFFFFFFFFL;
	net_set_udp_header(iphdr, bcast_ip, PORT_BOOTPS, PORT_BOOTPC, iplen);

#ifdef CONFIG_BOOTP_DHCP_REQUEST_DELAY
	udelay(CONFIG_BOOTP_DHCP_REQUEST_DELAY);
#endif	/* CONFIG_BOOTP_DHCP_REQUEST_DELAY */
	debug("Transmitting DHCPREQUEST packet: len = %d\n", pktlen);
	net_send_packet(net_tx_packet, pktlen);
}

/*
 *	Handle DHCP received packets.
 */
static void dhcp_handler(uchar *pkt, unsigned dest, struct in_addr sip,
			 unsigned src, unsigned len)
{
	struct bootp_hdr *bp = (struct bootp_hdr *)pkt;

	debug("DHCPHandler: got packet: (src=%d, dst=%d, len=%d) state: %d\n",
	      src, dest, len, dhcp_state);

	/* Filter out pkts we don't want */
	if (check_reply_packet(pkt, dest, src, len))
		return;

	debug("DHCPHandler: got DHCP packet: (src=%d, dst=%d, len=%d) state: "
	      "%d\n", src, dest, len, dhcp_state);

	if (net_read_ip(&bp->bp_yiaddr).s_addr == 0)
		return;

	switch (dhcp_state) {
	case SELECTING:
		/*
		 * Wait an appropriate time for any potential DHCPOFFER packets
		 * to arrive.  Then select one, and generate DHCPREQUEST
		 * response.  If filename is in format we recognize, assume it
		 * is a valid OFFER from a server we want.
		 */
		debug("DHCP: state=SELECTING bp_file: \"%s\"\n", bp->bp_file);
#ifdef CONFIG_SYS_BOOTFILE_PREFIX
		if (strncmp(bp->bp_file,
			    CONFIG_SYS_BOOTFILE_PREFIX,
			    strlen(CONFIG_SYS_BOOTFILE_PREFIX)) == 0) {
#endif	/* CONFIG_SYS_BOOTFILE_PREFIX */
			dhcp_packet_process_options(bp);
			efi_net_set_dhcp_ack(pkt, len);

			debug("TRANSITIONING TO REQUESTING STATE\n");
			dhcp_state = REQUESTING;

			net_set_timeout_handler(5000, bootp_timeout_handler);
			dhcp_send_request_packet(bp);
#ifdef CONFIG_SYS_BOOTFILE_PREFIX
		}
#endif	/* CONFIG_SYS_BOOTFILE_PREFIX */

		return;
		break;
	case REQUESTING:
		debug("DHCP State: REQUESTING\n");

		if (dhcp_message_type((u8 *)bp->bp_vend) == DHCP_ACK) {
			dhcp_packet_process_options(bp);
			/* Store net params from reply */
			store_net_params(bp);
			dhcp_state = BOUND;
			printf("DHCP client bound to address %pI4 (%lu ms)\n",
			       &net_ip, get_timer(bootp_start));
			net_set_timeout_handler(0, (thand_f *)0);
			bootstage_mark_name(BOOTSTAGE_ID_BOOTP_STOP,
					    "bootp_stop");

			net_auto_load();
			return;
		}
		break;
	case BOUND:
		/* DHCP client bound to address */
		break;
	default:
		puts("DHCP: INVALID STATE\n");
		break;
	}
}

void dhcp_request(void)
{
	bootp_request();
}
#endif	/* CONFIG_CMD_DHCP */
