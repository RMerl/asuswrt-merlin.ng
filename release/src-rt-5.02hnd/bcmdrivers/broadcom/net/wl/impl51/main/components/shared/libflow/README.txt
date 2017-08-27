/*
 * <<Broadcom-WL-IPTag/Open:>>
 */
A userspace library for flow management - used to issue a flow suspend,
resume and delete.

The public APIs can be found in flow_api.h. Need to supply a 5 tuple: SIP,
SPORT, DIP, DPORT, and PROTO:

typedef union
{
	struct in_addr ip_v4;
	struct in6_addr ip_v6;
} ip_address_t;

/* Suspend a flow */
int flow_suspend(int family,
                 ip_address_t *src_addr, uint16_t src_port,
                 ip_address_t *dst_addr, uint16_t dst_port,
                 uint8_t protocol);

/* Resume a flow */
int flow_resume(int family,
                ip_address_t *src_addr, uint16_t src_port,
                ip_address_t *dst_addr, uint16_t dst_port,
                uint8_t protocol);

/* Delete a flow entry */
int flow_delete(int family,
                ip_address_t *src_addr, uint16_t src_port,
                ip_address_t *dst_addr, uint16_t dst_port,
                uint8_t protocol);

NOTE: this library is dependent on Netfilter's conntrack user level library to
support flow DELETE.

You will need the following netfilter libraries (http://netfilter.org).

libmnl-1.0.3 - minimalist library
libnfnetlink-1.0.1 - netlink library
libnetfilter_conntrack-1.0.2 - conntrack library

Building libflow
----------------
Enable CONFIG_LIBFLOW in src/router/.config.

Flow Example
------------
The example/ subdirectory is a standalone program that links with libflow
to provide flow suspend, resume and delete functionality.

Usage:

# flow_test
Usage: flow_test <src-ip> <src-port> <dst-ip> <dst-port> <protocol> <suspend|resume|delete>

FLOW behavior
------------
CTF will create the flows and enable fast path by default. To change the
behavior to only create the flows and pass the packets up the network
stack, you will need to set the nvram variable:

	nvram set nga_enable=1

With this configuration, the CTF will create the flows. However, packets
for the flow are still sent up the network stack. To resume CTF fast path,
call flow_resume() API.
