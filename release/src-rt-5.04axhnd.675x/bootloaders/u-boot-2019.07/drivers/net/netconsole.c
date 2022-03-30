// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <command.h>
#include <stdio_dev.h>
#include <net.h>

#ifndef CONFIG_NETCONSOLE_BUFFER_SIZE
#define CONFIG_NETCONSOLE_BUFFER_SIZE 512
#endif

static char input_buffer[CONFIG_NETCONSOLE_BUFFER_SIZE];
static int input_size; /* char count in input buffer */
static int input_offset; /* offset to valid chars in input buffer */
static int input_recursion;
static int output_recursion;
static int net_timeout;
static uchar nc_ether[6]; /* server enet address */
static struct in_addr nc_ip; /* server ip */
static short nc_out_port; /* target output port */
static short nc_in_port; /* source input port */
static const char *output_packet; /* used by first send udp */
static int output_packet_len;
/*
 * Start with a default last protocol.
 * We are only interested in NETCONS or not.
 */
enum proto_t net_loop_last_protocol = BOOTP;

static void nc_wait_arp_handler(uchar *pkt, unsigned dest,
				 struct in_addr sip, unsigned src,
				 unsigned len)
{
	net_set_state(NETLOOP_SUCCESS); /* got arp reply - quit net loop */
}

static void nc_handler(uchar *pkt, unsigned dest, struct in_addr sip,
		       unsigned src, unsigned len)
{
	if (input_size)
		net_set_state(NETLOOP_SUCCESS); /* got input - quit net loop */
}

static void nc_timeout_handler(void)
{
	net_set_state(NETLOOP_SUCCESS);
}

static int is_broadcast(struct in_addr ip)
{
	static struct in_addr netmask;
	static struct in_addr our_ip;
	static int env_changed_id;
	int env_id = get_env_id();

	/* update only when the environment has changed */
	if (env_changed_id != env_id) {
		netmask = env_get_ip("netmask");
		our_ip = env_get_ip("ipaddr");

		env_changed_id = env_id;
	}

	return (ip.s_addr == ~0 || /* 255.255.255.255 (global bcast) */
		((netmask.s_addr & our_ip.s_addr) ==
		 (netmask.s_addr & ip.s_addr) && /* on the same net and */
		 (netmask.s_addr | ip.s_addr) == ~0)); /* bcast to our net */
}

static int refresh_settings_from_env(void)
{
	const char *p;
	static int env_changed_id;
	int env_id = get_env_id();

	/* update only when the environment has changed */
	if (env_changed_id != env_id) {
		if (env_get("ncip")) {
			nc_ip = env_get_ip("ncip");
			if (!nc_ip.s_addr)
				return -1;	/* ncip is 0.0.0.0 */
			p = strchr(env_get("ncip"), ':');
			if (p != NULL) {
				nc_out_port = simple_strtoul(p + 1, NULL, 10);
				nc_in_port = nc_out_port;
			}
		} else {
			nc_ip.s_addr = ~0; /* ncip is not set, so broadcast */
		}

		p = env_get("ncoutport");
		if (p != NULL)
			nc_out_port = simple_strtoul(p, NULL, 10);
		p = env_get("ncinport");
		if (p != NULL)
			nc_in_port = simple_strtoul(p, NULL, 10);

		if (is_broadcast(nc_ip))
			/* broadcast MAC address */
			memset(nc_ether, 0xff, sizeof(nc_ether));
		else
			/* force arp request */
			memset(nc_ether, 0, sizeof(nc_ether));
	}
	return 0;
}

/**
 * Called from net_loop in net/net.c before each packet
 */
void nc_start(void)
{
	refresh_settings_from_env();
	if (!output_packet_len || memcmp(nc_ether, net_null_ethaddr, 6)) {
		/* going to check for input packet */
		net_set_udp_handler(nc_handler);
		net_set_timeout_handler(net_timeout, nc_timeout_handler);
	} else {
		/* send arp request */
		uchar *pkt;
		net_set_arp_handler(nc_wait_arp_handler);
		pkt = (uchar *)net_tx_packet + net_eth_hdr_size() +
			IP_UDP_HDR_SIZE;
		memcpy(pkt, output_packet, output_packet_len);
		net_send_udp_packet(nc_ether, nc_ip, nc_out_port, nc_in_port,
				    output_packet_len);
	}
}

int nc_input_packet(uchar *pkt, struct in_addr src_ip, unsigned dest_port,
	unsigned src_port, unsigned len)
{
	int end, chunk;

	if (dest_port != nc_in_port || !len)
		return 0; /* not for us */

	if (src_ip.s_addr != nc_ip.s_addr && !is_broadcast(nc_ip))
		return 0; /* not from our client */

	debug_cond(DEBUG_DEV_PKT, "input: \"%*.*s\"\n", len, len, pkt);

	if (input_size == sizeof(input_buffer))
		return 1; /* no space */
	if (len > sizeof(input_buffer) - input_size)
		len = sizeof(input_buffer) - input_size;

	end = input_offset + input_size;
	if (end >= sizeof(input_buffer))
		end -= sizeof(input_buffer);

	chunk = len;
	/* Check if packet will wrap in input_buffer */
	if (end + len >= sizeof(input_buffer)) {
		chunk = sizeof(input_buffer) - end;
		/* Copy the second part of the pkt to start of input_buffer */
		memcpy(input_buffer, pkt + chunk, len - chunk);
	}
	/* Copy first (or only) part of pkt after end of current valid input*/
	memcpy(input_buffer + end, pkt, chunk);

	input_size += len;

	return 1;
}

static void nc_send_packet(const char *buf, int len)
{
#ifdef CONFIG_DM_ETH
	struct udevice *eth;
#else
	struct eth_device *eth;
#endif
	int inited = 0;
	uchar *pkt;
	uchar *ether;
	struct in_addr ip;

	debug_cond(DEBUG_DEV_PKT, "output: \"%*.*s\"\n", len, len, buf);

	eth = eth_get_dev();
	if (eth == NULL)
		return;

	if (!memcmp(nc_ether, net_null_ethaddr, 6)) {
		if (eth_is_active(eth))
			return;	/* inside net loop */
		output_packet = buf;
		output_packet_len = len;
		input_recursion = 1;
		net_loop(NETCONS); /* wait for arp reply and send packet */
		input_recursion = 0;
		output_packet_len = 0;
		return;
	}

	if (!eth_is_active(eth)) {
		if (eth_is_on_demand_init()) {
			if (eth_init() < 0)
				return;
			eth_set_last_protocol(NETCONS);
		} else {
			eth_init_state_only();
		}

		inited = 1;
	}
	pkt = (uchar *)net_tx_packet + net_eth_hdr_size() + IP_UDP_HDR_SIZE;
	memcpy(pkt, buf, len);
	ether = nc_ether;
	ip = nc_ip;
	net_send_udp_packet(ether, ip, nc_out_port, nc_in_port, len);

	if (inited) {
		if (eth_is_on_demand_init())
			eth_halt();
		else
			eth_halt_state_only();
	}
}

static int nc_stdio_start(struct stdio_dev *dev)
{
	int retval;

	nc_out_port = 6666; /* default port */
	nc_in_port = nc_out_port;

	retval = refresh_settings_from_env();
	if (retval != 0)
		return retval;

	/*
	 * Initialize the static IP settings and buffer pointers
	 * incase we call net_send_udp_packet before net_loop
	 */
	net_init();

	return 0;
}

static void nc_stdio_putc(struct stdio_dev *dev, char c)
{
	if (output_recursion)
		return;
	output_recursion = 1;

	nc_send_packet(&c, 1);

	output_recursion = 0;
}

static void nc_stdio_puts(struct stdio_dev *dev, const char *s)
{
	int len;

	if (output_recursion)
		return;
	output_recursion = 1;

	len = strlen(s);
	while (len) {
		int send_len = min(len, (int)sizeof(input_buffer));
		nc_send_packet(s, send_len);
		len -= send_len;
		s += send_len;
	}

	output_recursion = 0;
}

static int nc_stdio_getc(struct stdio_dev *dev)
{
	uchar c;

	input_recursion = 1;

	net_timeout = 0;	/* no timeout */
	while (!input_size)
		net_loop(NETCONS);

	input_recursion = 0;

	c = input_buffer[input_offset++];

	if (input_offset >= sizeof(input_buffer))
		input_offset -= sizeof(input_buffer);
	input_size--;

	return c;
}

static int nc_stdio_tstc(struct stdio_dev *dev)
{
#ifdef CONFIG_DM_ETH
	struct udevice *eth;
#else
	struct eth_device *eth;
#endif

	if (input_recursion)
		return 0;

	if (input_size)
		return 1;

	eth = eth_get_dev();
	if (eth_is_active(eth))
		return 0;	/* inside net loop */

	input_recursion = 1;

	net_timeout = 1;
	net_loop(NETCONS);	/* kind of poll */

	input_recursion = 0;

	return input_size != 0;
}

int drv_nc_init(void)
{
	struct stdio_dev dev;
	int rc;

	memset(&dev, 0, sizeof(dev));

	strcpy(dev.name, "nc");
	dev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_INPUT;
	dev.start = nc_stdio_start;
	dev.putc = nc_stdio_putc;
	dev.puts = nc_stdio_puts;
	dev.getc = nc_stdio_getc;
	dev.tstc = nc_stdio_tstc;

	rc = stdio_register(&dev);

	return (rc == 0) ? 1 : rc;
}
