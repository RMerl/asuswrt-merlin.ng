// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_snp
 *
 * Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This unit test covers the Simple Network Protocol as well as
 * the CopyMem and SetMem boottime services.
 *
 * A DHCP discover message is sent. The test is successful if a
 * DHCP reply is received.
 *
 * TODO: Once ConnectController and DisconnectController are implemented
 *	 we should connect our code as controller.
 */

#include <efi_selftest.h>
#include <net.h>

/*
 * MAC address for broadcasts
 */
static const u8 BROADCAST_MAC[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

struct dhcp_hdr {
	u8 op;
#define BOOTREQUEST 1
#define BOOTREPLY 2
	u8 htype;
# define HWT_ETHER 1
	u8 hlen;
# define HWL_ETHER 6
	u8 hops;
	u32 xid;
	u16 secs;
	u16 flags;
#define DHCP_FLAGS_UNICAST	0x0000
#define DHCP_FLAGS_BROADCAST	0x0080
	u32 ciaddr;
	u32 yiaddr;
	u32 siaddr;
	u32 giaddr;
	u8 chaddr[16];
	u8 sname[64];
	u8 file[128];
};

/*
 * Message type option.
 */
#define DHCP_MESSAGE_TYPE	0x35
#define DHCPDISCOVER		1
#define DHCPOFFER		2
#define DHCPREQUEST		3
#define DHCPDECLINE		4
#define DHCPACK			5
#define DHCPNAK			6
#define DHCPRELEASE		7

struct dhcp {
	struct ethernet_hdr eth_hdr;
	struct ip_udp_hdr ip_udp;
	struct dhcp_hdr dhcp_hdr;
	u8 opt[128];
} __packed;

static struct efi_boot_services *boottime;
static struct efi_simple_network *net;
static struct efi_event *timer;
static const efi_guid_t efi_net_guid = EFI_SIMPLE_NETWORK_PROTOCOL_GUID;
/* IP packet ID */
static unsigned int net_ip_id;

/*
 * Compute the checksum of the IP header. We cover even values of length only.
 * We cannot use net/checksum.c due to different CFLAGS values.
 *
 * @buf:	IP header
 * @len:	length of header in bytes
 * Return:	checksum
 */
static unsigned int efi_ip_checksum(const void *buf, size_t len)
{
	size_t i;
	u32 sum = 0;
	const u16 *pos = buf;

	for (i = 0; i < len; i += 2)
		sum += *pos++;

	sum = (sum >> 16) + (sum & 0xffff);
	sum += sum >> 16;
	sum = ~sum & 0xffff;

	return sum;
}

/*
 * Transmit a DHCPDISCOVER message.
 */
static efi_status_t send_dhcp_discover(void)
{
	efi_status_t ret;
	struct dhcp p = {};

	/*
	 * Fill Ethernet header
	 */
	boottime->copy_mem(p.eth_hdr.et_dest, (void *)BROADCAST_MAC, ARP_HLEN);
	boottime->copy_mem(p.eth_hdr.et_src, &net->mode->current_address,
			   ARP_HLEN);
	p.eth_hdr.et_protlen = htons(PROT_IP);
	/*
	 * Fill IP header
	 */
	p.ip_udp.ip_hl_v	= 0x45;
	p.ip_udp.ip_len		= htons(sizeof(struct dhcp) -
					sizeof(struct ethernet_hdr));
	p.ip_udp.ip_id		= htons(++net_ip_id);
	p.ip_udp.ip_off		= htons(IP_FLAGS_DFRAG);
	p.ip_udp.ip_ttl		= 0xff; /* time to live */
	p.ip_udp.ip_p		= IPPROTO_UDP;
	boottime->set_mem(&p.ip_udp.ip_dst, 4, 0xff);
	p.ip_udp.ip_sum		= efi_ip_checksum(&p.ip_udp, IP_HDR_SIZE);

	/*
	 * Fill UDP header
	 */
	p.ip_udp.udp_src	= htons(68);
	p.ip_udp.udp_dst	= htons(67);
	p.ip_udp.udp_len	= htons(sizeof(struct dhcp) -
					sizeof(struct ethernet_hdr) -
					sizeof(struct ip_hdr));
	/*
	 * Fill DHCP header
	 */
	p.dhcp_hdr.op		= BOOTREQUEST;
	p.dhcp_hdr.htype	= HWT_ETHER;
	p.dhcp_hdr.hlen		= HWL_ETHER;
	p.dhcp_hdr.flags	= htons(DHCP_FLAGS_UNICAST);
	boottime->copy_mem(&p.dhcp_hdr.chaddr,
			   &net->mode->current_address, ARP_HLEN);
	/*
	 * Fill options
	 */
	p.opt[0]	= 0x63; /* DHCP magic cookie */
	p.opt[1]	= 0x82;
	p.opt[2]	= 0x53;
	p.opt[3]	= 0x63;
	p.opt[4]	= DHCP_MESSAGE_TYPE;
	p.opt[5]	= 0x01; /* length */
	p.opt[6]	= DHCPDISCOVER;
	p.opt[7]	= 0x39; /* maximum message size */
	p.opt[8]	= 0x02; /* length */
	p.opt[9]	= 0x02; /* 576 bytes */
	p.opt[10]	= 0x40;
	p.opt[11]	= 0xff; /* end of options */

	/*
	 * Transmit DHCPDISCOVER message.
	 */
	ret = net->transmit(net, 0, sizeof(struct dhcp), &p, NULL, NULL, 0);
	if (ret != EFI_SUCCESS)
		efi_st_error("Sending a DHCP request failed\n");
	else
		efi_st_printf("DHCP Discover\n");
	return ret;
}

/*
 * Setup unit test.
 *
 * Create a 1 s periodic timer.
 * Start the network driver.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * Return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	efi_status_t ret;

	boottime = systable->boottime;

	/*
	 * Create a timer event.
	 */
	ret = boottime->create_event(EVT_TIMER, TPL_CALLBACK, NULL, NULL,
				     &timer);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to create event\n");
		return EFI_ST_FAILURE;
	}
	/*
	 * Set timer period to 1s.
	 */
	ret = boottime->set_timer(timer, EFI_TIMER_PERIODIC, 10000000);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to set timer\n");
		return EFI_ST_FAILURE;
	}
	/*
	 * Find an interface implementing the SNP protocol.
	 */
	ret = boottime->locate_protocol(&efi_net_guid, NULL, (void **)&net);
	if (ret != EFI_SUCCESS) {
		net = NULL;
		efi_st_error("Failed to locate simple network protocol\n");
		return EFI_ST_FAILURE;
	}
	/*
	 * Check hardware address size.
	 */
	if (!net->mode) {
		efi_st_error("Mode not provided\n");
		return EFI_ST_FAILURE;
	}
	if (net->mode->hwaddr_size != ARP_HLEN) {
		efi_st_error("HwAddressSize = %u, expected %u\n",
			     net->mode->hwaddr_size, ARP_HLEN);
		return EFI_ST_FAILURE;
	}
	/*
	 * Check that WaitForPacket event exists.
	 */
	if (!net->wait_for_packet) {
		efi_st_error("WaitForPacket event missing\n");
		return EFI_ST_FAILURE;
	}
	if (net->mode->state == EFI_NETWORK_INITIALIZED) {
		/*
		 * Shut down network adapter.
		 */
		ret = net->shutdown(net);
		if (ret != EFI_SUCCESS) {
			efi_st_error("Failed to shut down network adapter\n");
			return EFI_ST_FAILURE;
		}
	}
	if (net->mode->state == EFI_NETWORK_STARTED) {
		/*
		 * Stop network adapter.
		 */
		ret = net->stop(net);
		if (ret != EFI_SUCCESS) {
			efi_st_error("Failed to stop network adapter\n");
			return EFI_ST_FAILURE;
		}
	}
	/*
	 * Start network adapter.
	 */
	ret = net->start(net);
	if (ret != EFI_SUCCESS && ret != EFI_ALREADY_STARTED) {
		efi_st_error("Failed to start network adapter\n");
		return EFI_ST_FAILURE;
	}
	if (net->mode->state != EFI_NETWORK_STARTED) {
		efi_st_error("Failed to start network adapter\n");
		return EFI_ST_FAILURE;
	}
	/*
	 * Initialize network adapter.
	 */
	ret = net->initialize(net, 0, 0);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to initialize network adapter\n");
		return EFI_ST_FAILURE;
	}
	if (net->mode->state != EFI_NETWORK_INITIALIZED) {
		efi_st_error("Failed to initialize network adapter\n");
		return EFI_ST_FAILURE;
	}
	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * A DHCP discover message is sent. The test is successful if a
 * DHCP reply is received within 10 seconds.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	efi_status_t ret;
	struct efi_event *events[2];
	efi_uintn_t index;
	union {
		struct dhcp p;
		u8 b[PKTSIZE];
	} buffer;
	struct efi_mac_address srcaddr;
	struct efi_mac_address destaddr;
	size_t buffer_size;
	u8 *addr;

	/*
	 * The timeout is to occur after 10 s.
	 */
	unsigned int timeout = 10;

	/* Setup may have failed */
	if (!net || !timer) {
		efi_st_error("Cannot execute test after setup failure\n");
		return EFI_ST_FAILURE;
	}

	/* Check media connected */
	ret = net->get_status(net, NULL, NULL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to get status");
		return EFI_ST_FAILURE;
	}
	if (net->mode && net->mode->media_present_supported &&
	    !net->mode->media_present) {
		efi_st_error("Network media is not connected");
		return EFI_ST_FAILURE;
	}

	/*
	 * Send DHCP discover message
	 */
	ret = send_dhcp_discover();
	if (ret != EFI_SUCCESS)
		return EFI_ST_FAILURE;

	/*
	 * If we would call WaitForEvent only with the WaitForPacket event,
	 * our code would block until a packet is received which might never
	 * occur. By calling WaitFor event with both a timer event and the
	 * WaitForPacket event we can escape this blocking situation.
	 *
	 * If the timer event occurs before we have received a DHCP reply
	 * a further DHCP discover message is sent.
	 */
	events[0] = timer;
	events[1] = net->wait_for_packet;
	for (;;) {
		/*
		 * Wait for packet to be received or timer event.
		 */
		boottime->wait_for_event(2, events, &index);
		if (index == 0) {
			/*
			 * The timer event occurred. Check for timeout.
			 */
			--timeout;
			if (!timeout) {
				efi_st_error("Timeout occurred\n");
				return EFI_ST_FAILURE;
			}
			/*
			 * Send further DHCP discover message
			 */
			ret = send_dhcp_discover();
			if (ret != EFI_SUCCESS)
				return EFI_ST_FAILURE;
			continue;
		}
		/*
		 * Receive packets until buffer is empty
		 */
		for (;;) {
			buffer_size = sizeof(buffer);
			ret = net->receive(net, NULL, &buffer_size, &buffer,
					   &srcaddr, &destaddr, NULL);
			if (ret == EFI_NOT_READY) {
				/* The received buffer is empty. */
				break;
			}

			if (ret != EFI_SUCCESS) {
				efi_st_error("Failed to receive packet");
				return EFI_ST_FAILURE;
			}
			/*
			 * Check the packet is meant for this system.
			 * Unfortunately QEMU ignores the broadcast flag.
			 * So we have to check for broadcasts too.
			 */
			if (memcmp(&destaddr, &net->mode->current_address, ARP_HLEN) &&
			    memcmp(&destaddr, BROADCAST_MAC, ARP_HLEN))
				continue;
			/*
			 * Check this is a DHCP reply
			 */
			if (buffer.p.eth_hdr.et_protlen != ntohs(PROT_IP) ||
			    buffer.p.ip_udp.ip_hl_v != 0x45 ||
			    buffer.p.ip_udp.ip_p != IPPROTO_UDP ||
			    buffer.p.ip_udp.udp_src != ntohs(67) ||
			    buffer.p.ip_udp.udp_dst != ntohs(68) ||
			    buffer.p.dhcp_hdr.op != BOOTREPLY)
				continue;
			/*
			 * We successfully received a DHCP reply.
			 */
			goto received;
		}
	}
received:
	/*
	 * Write a log message.
	 */
	addr = (u8 *)&buffer.p.ip_udp.ip_src;
	efi_st_printf("DHCP reply received from %u.%u.%u.%u (%pm) ",
		      addr[0], addr[1], addr[2], addr[3], &srcaddr);
	if (!memcmp(&destaddr, BROADCAST_MAC, ARP_HLEN))
		efi_st_printf("as broadcast message.\n");
	else
		efi_st_printf("as unicast message.\n");

	return EFI_ST_SUCCESS;
}

/*
 * Tear down unit test.
 *
 * Close the timer event created in setup.
 * Shut down the network adapter.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int teardown(void)
{
	efi_status_t ret;
	int exit_status = EFI_ST_SUCCESS;

	if (timer) {
		/*
		 * Stop timer.
		 */
		ret = boottime->set_timer(timer, EFI_TIMER_STOP, 0);
		if (ret != EFI_SUCCESS) {
			efi_st_error("Failed to stop timer");
			exit_status = EFI_ST_FAILURE;
		}
		/*
		 * Close timer event.
		 */
		ret = boottime->close_event(timer);
		if (ret != EFI_SUCCESS) {
			efi_st_error("Failed to close event");
			exit_status = EFI_ST_FAILURE;
		}
	}
	if (net) {
		/*
		 * Shut down network adapter.
		 */
		ret = net->shutdown(net);
		if (ret != EFI_SUCCESS) {
			efi_st_error("Failed to shut down network adapter\n");
			exit_status = EFI_ST_FAILURE;
		}
		if (net->mode->state != EFI_NETWORK_STARTED) {
			efi_st_error("Failed to shutdown network adapter\n");
			return EFI_ST_FAILURE;
		}
		/*
		 * Stop network adapter.
		 */
		ret = net->stop(net);
		if (ret != EFI_SUCCESS) {
			efi_st_error("Failed to stop network adapter\n");
			exit_status = EFI_ST_FAILURE;
		}
		if (net->mode->state != EFI_NETWORK_STOPPED) {
			efi_st_error("Failed to stop network adapter\n");
			return EFI_ST_FAILURE;
		}
	}

	return exit_status;
}

EFI_UNIT_TEST(snp) = {
	.name = "simple network protocol",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.teardown = teardown,
#ifdef CONFIG_SANDBOX
	/*
	 * Running this test on the sandbox requires setting environment
	 * variable ethact to a network interface connected to a DHCP server and
	 * ethrotate to 'no'.
	 */
	.on_request = true,
#endif
};
