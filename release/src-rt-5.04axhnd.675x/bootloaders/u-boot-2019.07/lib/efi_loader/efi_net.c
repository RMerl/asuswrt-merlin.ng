// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI application network access support
 *
 *  Copyright (c) 2016 Alexander Graf
 */

#include <common.h>
#include <efi_loader.h>
#include <malloc.h>

static const efi_guid_t efi_net_guid = EFI_SIMPLE_NETWORK_PROTOCOL_GUID;
static const efi_guid_t efi_pxe_guid = EFI_PXE_BASE_CODE_PROTOCOL_GUID;
static struct efi_pxe_packet *dhcp_ack;
static bool new_rx_packet;
static void *new_tx_packet;
static void *transmit_buffer;

/*
 * The notification function of this event is called in every timer cycle
 * to check if a new network packet has been received.
 */
static struct efi_event *network_timer_event;
/*
 * This event is signaled when a packet has been received.
 */
static struct efi_event *wait_for_packet;

/**
 * struct efi_net_obj - EFI object representing a network interface
 *
 * @header:	EFI object header
 * @net:	simple network protocol interface
 * @net_mode:	status of the network interface
 * @pxe:	PXE base code protocol interface
 * @pxe_mode:	status of the PXE base code protocol
 */
struct efi_net_obj {
	struct efi_object header;
	struct efi_simple_network net;
	struct efi_simple_network_mode net_mode;
	struct efi_pxe pxe;
	struct efi_pxe_mode pxe_mode;
};

/*
 * efi_net_start() - start the network interface
 *
 * This function implements the Start service of the
 * EFI_SIMPLE_NETWORK_PROTOCOL. See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:	pointer to the protocol instance
 * Return:	status code
 */
static efi_status_t EFIAPI efi_net_start(struct efi_simple_network *this)
{
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p", this);

	/* Check parameters */
	if (!this) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	if (this->mode->state != EFI_NETWORK_STOPPED)
		ret = EFI_ALREADY_STARTED;
	else
		this->mode->state = EFI_NETWORK_STARTED;
out:
	return EFI_EXIT(ret);
}

/*
 * efi_net_stop() - stop the network interface
 *
 * This function implements the Stop service of the
 * EFI_SIMPLE_NETWORK_PROTOCOL. See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:	pointer to the protocol instance
 * Return:	status code
 */
static efi_status_t EFIAPI efi_net_stop(struct efi_simple_network *this)
{
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p", this);

	/* Check parameters */
	if (!this) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	if (this->mode->state == EFI_NETWORK_STOPPED)
		ret = EFI_NOT_STARTED;
	else
		this->mode->state = EFI_NETWORK_STOPPED;
out:
	return EFI_EXIT(ret);
}

/*
 * efi_net_initialize() - initialize the network interface
 *
 * This function implements the Initialize service of the
 * EFI_SIMPLE_NETWORK_PROTOCOL. See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:	pointer to the protocol instance
 * @extra_rx:	extra receive buffer to be allocated
 * @extra_tx:	extra transmit buffer to be allocated
 * Return:	status code
 */
static efi_status_t EFIAPI efi_net_initialize(struct efi_simple_network *this,
					      ulong extra_rx, ulong extra_tx)
{
	int ret;
	efi_status_t r = EFI_SUCCESS;

	EFI_ENTRY("%p, %lx, %lx", this, extra_rx, extra_tx);

	/* Check parameters */
	if (!this) {
		r = EFI_INVALID_PARAMETER;
		goto out;
	}

	/* Setup packet buffers */
	net_init();
	/* Disable hardware and put it into the reset state */
	eth_halt();
	/* Set current device according to environment variables */
	eth_set_current();
	/* Get hardware ready for send and receive operations */
	ret = eth_init();
	if (ret < 0) {
		eth_halt();
		this->mode->state = EFI_NETWORK_STOPPED;
		r = EFI_DEVICE_ERROR;
		goto out;
	} else {
		this->mode->state = EFI_NETWORK_INITIALIZED;
	}
out:
	return EFI_EXIT(r);
}

/*
 * efi_net_reset() - reinitialize the network interface
 *
 * This function implements the Reset service of the
 * EFI_SIMPLE_NETWORK_PROTOCOL. See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:			pointer to the protocol instance
 * @extended_verification:	execute exhaustive verification
 * Return:			status code
 */
static efi_status_t EFIAPI efi_net_reset(struct efi_simple_network *this,
					 int extended_verification)
{
	EFI_ENTRY("%p, %x", this, extended_verification);

	return EFI_EXIT(EFI_CALL(efi_net_initialize(this, 0, 0)));
}

/*
 * efi_net_shutdown() - shut down the network interface
 *
 * This function implements the Shutdown service of the
 * EFI_SIMPLE_NETWORK_PROTOCOL. See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:	pointer to the protocol instance
 * Return:	status code
 */
static efi_status_t EFIAPI efi_net_shutdown(struct efi_simple_network *this)
{
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p", this);

	/* Check parameters */
	if (!this) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	eth_halt();
	this->mode->state = EFI_NETWORK_STOPPED;

out:
	return EFI_EXIT(ret);
}

/*
 * efi_net_receive_filters() - mange multicast receive filters
 *
 * This function implements the ReceiveFilters service of the
 * EFI_SIMPLE_NETWORK_PROTOCOL. See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:		pointer to the protocol instance
 * @enable:		bit mask of receive filters to enable
 * @disable:		bit mask of receive filters to disable
 * @reset_mcast_filter:	true resets contents of the filters
 * @mcast_filter_count:	number of hardware MAC addresses in the new filters list
 * @mcast_filter:	list of new filters
 * Return:		status code
 */
static efi_status_t EFIAPI efi_net_receive_filters
		(struct efi_simple_network *this, u32 enable, u32 disable,
		 int reset_mcast_filter, ulong mcast_filter_count,
		 struct efi_mac_address *mcast_filter)
{
	EFI_ENTRY("%p, %x, %x, %x, %lx, %p", this, enable, disable,
		  reset_mcast_filter, mcast_filter_count, mcast_filter);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

/*
 * efi_net_station_address() - set the hardware MAC address
 *
 * This function implements the StationAddress service of the
 * EFI_SIMPLE_NETWORK_PROTOCOL. See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:	pointer to the protocol instance
 * @reset:	if true reset the address to default
 * @new_mac:	new MAC address
 * Return:	status code
 */
static efi_status_t EFIAPI efi_net_station_address
		(struct efi_simple_network *this, int reset,
		 struct efi_mac_address *new_mac)
{
	EFI_ENTRY("%p, %x, %p", this, reset, new_mac);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

/*
 * efi_net_statistics() - reset or collect statistics of the network interface
 *
 * This function implements the Statistics service of the
 * EFI_SIMPLE_NETWORK_PROTOCOL. See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:	pointer to the protocol instance
 * @reset:	if true, the statistics are reset
 * @stat_size:	size of the statistics table
 * @stat_table:	table to receive the statistics
 * Return:	status code
 */
static efi_status_t EFIAPI efi_net_statistics(struct efi_simple_network *this,
					      int reset, ulong *stat_size,
					      void *stat_table)
{
	EFI_ENTRY("%p, %x, %p, %p", this, reset, stat_size, stat_table);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

/*
 * efi_net_mcastiptomac() - translate multicast IP address to MAC address
 *
 * This function implements the Statistics service of the
 * EFI_SIMPLE_NETWORK_PROTOCOL. See the Unified Extensible Firmware Interface
 * (UEFI) specification for details.
 *
 * @this:	pointer to the protocol instance
 * @ipv6:	true if the IP address is an IPv6 address
 * @ip:		IP address
 * @mac:	MAC address
 * Return:	status code
 */
static efi_status_t EFIAPI efi_net_mcastiptomac(struct efi_simple_network *this,
						int ipv6,
						struct efi_ip_address *ip,
						struct efi_mac_address *mac)
{
	EFI_ENTRY("%p, %x, %p, %p", this, ipv6, ip, mac);

	return EFI_EXIT(EFI_INVALID_PARAMETER);
}

/**
 * efi_net_nvdata() - read or write NVRAM
 *
 * This function implements the GetStatus service of the Simple Network
 * Protocol. See the UEFI spec for details.
 *
 * @this:		the instance of the Simple Network Protocol
 * @readwrite:		true for read, false for write
 * @offset:		offset in NVRAM
 * @buffer_size:	size of buffer
 * @buffer:		buffer
 * Return:		status code
 */
static efi_status_t EFIAPI efi_net_nvdata(struct efi_simple_network *this,
					  int read_write, ulong offset,
					  ulong buffer_size, char *buffer)
{
	EFI_ENTRY("%p, %x, %lx, %lx, %p", this, read_write, offset, buffer_size,
		  buffer);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

/**
 * efi_net_get_status() - get interrupt status
 *
 * This function implements the GetStatus service of the Simple Network
 * Protocol. See the UEFI spec for details.
 *
 * @this:		the instance of the Simple Network Protocol
 * @int_status:		interface status
 * @txbuf:		transmission buffer
 */
static efi_status_t EFIAPI efi_net_get_status(struct efi_simple_network *this,
					      u32 *int_status, void **txbuf)
{
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p, %p, %p", this, int_status, txbuf);

	efi_timer_check();

	/* Check parameters */
	if (!this) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	switch (this->mode->state) {
	case EFI_NETWORK_STOPPED:
		ret = EFI_NOT_STARTED;
		goto out;
	case EFI_NETWORK_STARTED:
		ret = EFI_DEVICE_ERROR;
		goto out;
	default:
		break;
	}

	if (int_status) {
		/* We send packets synchronously, so nothing is outstanding */
		*int_status = EFI_SIMPLE_NETWORK_TRANSMIT_INTERRUPT;
		if (new_rx_packet)
			*int_status |= EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT;
	}
	if (txbuf)
		*txbuf = new_tx_packet;

	new_tx_packet = NULL;
out:
	return EFI_EXIT(ret);
}

/**
 * efi_net_transmit() - transmit a packet
 *
 * This function implements the Transmit service of the Simple Network Protocol.
 * See the UEFI spec for details.
 *
 * @this:		the instance of the Simple Network Protocol
 * @header_size:	size of the media header
 * @buffer_size:	size of the buffer to receive the packet
 * @buffer:		buffer to receive the packet
 * @src_addr:		source hardware MAC address
 * @dest_addr:		destination hardware MAC address
 * @protocol:		type of header to build
 * Return:		status code
 */
static efi_status_t EFIAPI efi_net_transmit
		(struct efi_simple_network *this, size_t header_size,
		 size_t buffer_size, void *buffer,
		 struct efi_mac_address *src_addr,
		 struct efi_mac_address *dest_addr, u16 *protocol)
{
	efi_status_t ret = EFI_SUCCESS;

	EFI_ENTRY("%p, %lu, %lu, %p, %p, %p, %p", this,
		  (unsigned long)header_size, (unsigned long)buffer_size,
		  buffer, src_addr, dest_addr, protocol);

	efi_timer_check();

	/* Check parameters */
	if (!this || !buffer) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	/* We do not support jumbo packets */
	if (buffer_size > PKTSIZE_ALIGN) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	if (header_size) {
		/*
		 * TODO: We would need to create the header
		 * if header_size != 0
		 */
		ret = EFI_UNSUPPORTED;
		goto out;
	}

	switch (this->mode->state) {
	case EFI_NETWORK_STOPPED:
		ret = EFI_NOT_STARTED;
		goto out;
	case EFI_NETWORK_STARTED:
		ret = EFI_DEVICE_ERROR;
		goto out;
	default:
		break;
	}

	/* Ethernet packets always fit, just bounce */
	memcpy(transmit_buffer, buffer, buffer_size);
	net_send_packet(transmit_buffer, buffer_size);

	new_tx_packet = buffer;

out:
	return EFI_EXIT(ret);
}

/**
 * efi_net_receive() - receive a packet from a network interface
 *
 * This function implements the Receive service of the Simple Network Protocol.
 * See the UEFI spec for details.
 *
 * @this:		the instance of the Simple Network Protocol
 * @header_size:	size of the media header
 * @buffer_size:	size of the buffer to receive the packet
 * @buffer:		buffer to receive the packet
 * @src_addr:		source MAC address
 * @dest_addr:		destination MAC address
 * @protocol:		protocol
 * Return:		status code
 */
static efi_status_t EFIAPI efi_net_receive
		(struct efi_simple_network *this, size_t *header_size,
		 size_t *buffer_size, void *buffer,
		 struct efi_mac_address *src_addr,
		 struct efi_mac_address *dest_addr, u16 *protocol)
{
	efi_status_t ret = EFI_SUCCESS;
	struct ethernet_hdr *eth_hdr;
	size_t hdr_size = sizeof(struct ethernet_hdr);
	u16 protlen;

	EFI_ENTRY("%p, %p, %p, %p, %p, %p, %p", this, header_size,
		  buffer_size, buffer, src_addr, dest_addr, protocol);

	/* Execute events */
	efi_timer_check();

	/* Check parameters */
	if (!this || !buffer || !buffer_size) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	switch (this->mode->state) {
	case EFI_NETWORK_STOPPED:
		ret = EFI_NOT_STARTED;
		goto out;
	case EFI_NETWORK_STARTED:
		ret = EFI_DEVICE_ERROR;
		goto out;
	default:
		break;
	}

	if (!new_rx_packet) {
		ret = EFI_NOT_READY;
		goto out;
	}
	/* Check that we at least received an Ethernet header */
	if (net_rx_packet_len < sizeof(struct ethernet_hdr)) {
		new_rx_packet = false;
		ret = EFI_NOT_READY;
		goto out;
	}
	/* Fill export parameters */
	eth_hdr = (struct ethernet_hdr *)net_rx_packet;
	protlen = ntohs(eth_hdr->et_protlen);
	if (protlen == 0x8100) {
		hdr_size += 4;
		protlen = ntohs(*(u16 *)&net_rx_packet[hdr_size - 2]);
	}
	if (header_size)
		*header_size = hdr_size;
	if (dest_addr)
		memcpy(dest_addr, eth_hdr->et_dest, ARP_HLEN);
	if (src_addr)
		memcpy(src_addr, eth_hdr->et_src, ARP_HLEN);
	if (protocol)
		*protocol = protlen;
	if (*buffer_size < net_rx_packet_len) {
		/* Packet doesn't fit, try again with bigger buffer */
		*buffer_size = net_rx_packet_len;
		ret = EFI_BUFFER_TOO_SMALL;
		goto out;
	}
	/* Copy packet */
	memcpy(buffer, net_rx_packet, net_rx_packet_len);
	*buffer_size = net_rx_packet_len;
	new_rx_packet = false;
out:
	return EFI_EXIT(ret);
}

/**
 * efi_net_set_dhcp_ack() - take note of a selected DHCP IP address
 *
 * This function is called by dhcp_handler().
 */
void efi_net_set_dhcp_ack(void *pkt, int len)
{
	int maxsize = sizeof(*dhcp_ack);

	if (!dhcp_ack)
		dhcp_ack = malloc(maxsize);

	memcpy(dhcp_ack, pkt, min(len, maxsize));
}

/**
 * efi_net_push() - callback for received network packet
 *
 * This function is called when a network packet is received by eth_rx().
 *
 * @pkt:	network packet
 * @len:	length
 */
static void efi_net_push(void *pkt, int len)
{
	new_rx_packet = true;
	wait_for_packet->is_signaled = true;
}

/**
 * efi_network_timer_notify() - check if a new network packet has been received
 *
 * This notification function is called in every timer cycle.
 *
 * @event	the event for which this notification function is registered
 * @context	event context - not used in this function
 */
static void EFIAPI efi_network_timer_notify(struct efi_event *event,
					    void *context)
{
	struct efi_simple_network *this = (struct efi_simple_network *)context;

	EFI_ENTRY("%p, %p", event, context);

	/*
	 * Some network drivers do not support calling eth_rx() before
	 * initialization.
	 */
	if (!this || this->mode->state != EFI_NETWORK_INITIALIZED)
		goto out;

	if (!new_rx_packet) {
		push_packet = efi_net_push;
		eth_rx();
		push_packet = NULL;
	}
out:
	EFI_EXIT(EFI_SUCCESS);
}

/**
 * efi_net_register() - register the simple network protocol
 *
 * This gets called from do_bootefi_exec().
 */
efi_status_t efi_net_register(void)
{
	struct efi_net_obj *netobj = NULL;
	efi_status_t r;

	if (!eth_get_dev()) {
		/* No network device active, don't expose any */
		return EFI_SUCCESS;
	}

	/* We only expose the "active" network device, so one is enough */
	netobj = calloc(1, sizeof(*netobj));
	if (!netobj)
		goto out_of_resources;

	/* Allocate an aligned transmit buffer */
	transmit_buffer = calloc(1, PKTSIZE_ALIGN + PKTALIGN);
	if (!transmit_buffer)
		goto out_of_resources;
	transmit_buffer = (void *)ALIGN((uintptr_t)transmit_buffer, PKTALIGN);

	/* Hook net up to the device list */
	efi_add_handle(&netobj->header);

	/* Fill in object data */
	r = efi_add_protocol(&netobj->header, &efi_net_guid,
			     &netobj->net);
	if (r != EFI_SUCCESS)
		goto failure_to_add_protocol;
	r = efi_add_protocol(&netobj->header, &efi_guid_device_path,
			     efi_dp_from_eth());
	if (r != EFI_SUCCESS)
		goto failure_to_add_protocol;
	r = efi_add_protocol(&netobj->header, &efi_pxe_guid,
			     &netobj->pxe);
	if (r != EFI_SUCCESS)
		goto failure_to_add_protocol;
	netobj->net.revision = EFI_SIMPLE_NETWORK_PROTOCOL_REVISION;
	netobj->net.start = efi_net_start;
	netobj->net.stop = efi_net_stop;
	netobj->net.initialize = efi_net_initialize;
	netobj->net.reset = efi_net_reset;
	netobj->net.shutdown = efi_net_shutdown;
	netobj->net.receive_filters = efi_net_receive_filters;
	netobj->net.station_address = efi_net_station_address;
	netobj->net.statistics = efi_net_statistics;
	netobj->net.mcastiptomac = efi_net_mcastiptomac;
	netobj->net.nvdata = efi_net_nvdata;
	netobj->net.get_status = efi_net_get_status;
	netobj->net.transmit = efi_net_transmit;
	netobj->net.receive = efi_net_receive;
	netobj->net.mode = &netobj->net_mode;
	netobj->net_mode.state = EFI_NETWORK_STARTED;
	memcpy(netobj->net_mode.current_address.mac_addr, eth_get_ethaddr(), 6);
	netobj->net_mode.hwaddr_size = ARP_HLEN;
	netobj->net_mode.max_packet_size = PKTSIZE;
	netobj->net_mode.if_type = ARP_ETHER;

	netobj->pxe.mode = &netobj->pxe_mode;
	if (dhcp_ack)
		netobj->pxe_mode.dhcp_ack = *dhcp_ack;

	/*
	 * Create WaitForPacket event.
	 */
	r = efi_create_event(EVT_NOTIFY_WAIT, TPL_CALLBACK,
			     efi_network_timer_notify, NULL, NULL,
			     &wait_for_packet);
	if (r != EFI_SUCCESS) {
		printf("ERROR: Failed to register network event\n");
		return r;
	}
	netobj->net.wait_for_packet = wait_for_packet;
	/*
	 * Create a timer event.
	 *
	 * The notification function is used to check if a new network packet
	 * has been received.
	 *
	 * iPXE is running at TPL_CALLBACK most of the time. Use a higher TPL.
	 */
	r = efi_create_event(EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_NOTIFY,
			     efi_network_timer_notify, &netobj->net, NULL,
			     &network_timer_event);
	if (r != EFI_SUCCESS) {
		printf("ERROR: Failed to register network event\n");
		return r;
	}
	/* Network is time critical, create event in every timer cycle */
	r = efi_set_timer(network_timer_event, EFI_TIMER_PERIODIC, 0);
	if (r != EFI_SUCCESS) {
		printf("ERROR: Failed to set network timer\n");
		return r;
	}

	return EFI_SUCCESS;
failure_to_add_protocol:
	printf("ERROR: Failure to add protocol\n");
	return r;
out_of_resources:
	free(netobj);
	/* free(transmit_buffer) not needed yet */
	printf("ERROR: Out of memory\n");
	return EFI_OUT_OF_RESOURCES;
}
