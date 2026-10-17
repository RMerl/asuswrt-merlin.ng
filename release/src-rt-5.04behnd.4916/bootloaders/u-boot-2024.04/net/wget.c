// SPDX-License-Identifier: GPL-2.0
/*
 * WGET/HTTP support driver based on U-BOOT's nfs.c
 * Copyright Duncan Hare <dh@synoia.com> 2017
 */

#include <asm/global_data.h>
#include <command.h>
#include <common.h>
#include <display_options.h>
#include <env.h>
#include <image.h>
#include <lmb.h>
#include <mapmem.h>
#include <net.h>
#include <net/tcp.h>
#include <net/wget.h>
#include <stdlib.h>

DECLARE_GLOBAL_DATA_PTR;

/* The default, change with environment variable 'httpdstp' */
#define SERVER_PORT		80

static const char bootfile1[] = "GET ";
static const char bootfile3[] = " HTTP/1.0\r\n\r\n";
static const char http_eom[] = "\r\n\r\n";
static const char http_ok[] = "200";
static const char content_len[] = "Content-Length";
static const char linefeed[] = "\r\n";
static struct in_addr web_server_ip;
static int our_port;
static int wget_timeout_count;

struct pkt_qd {
	uchar *pkt;
	unsigned int tcp_seq_num;
	unsigned int len;
};

/*
 * This is a control structure for out of order packets received.
 * The actual packet bufers are in the kernel space, and are
 * expected to be overwritten by the downloaded image.
 */
#define PKTQ_SZ (PKTBUFSRX / 4)
static struct pkt_qd pkt_q[PKTQ_SZ];
static int pkt_q_idx;
static unsigned long content_length;
static unsigned int packets;

static unsigned int initial_data_seq_num;

static enum  wget_state current_wget_state;

static char *image_url;
static unsigned int wget_timeout = WGET_TIMEOUT;

static enum net_loop_state wget_loop_state;

/* Timeout retry parameters */
static u8 retry_action;			/* actions for TCP retry */
static unsigned int retry_tcp_ack_num;	/* TCP retry acknowledge number*/
static unsigned int retry_tcp_seq_num;	/* TCP retry sequence number */
static int retry_len;			/* TCP retry length */

static ulong wget_load_size;

/**
 * wget_init_max_size() - initialize maximum load size
 *
 * Return:	0 if success, -1 if fails
 */
static int wget_init_load_size(void)
{
	struct lmb lmb;
	phys_size_t max_size;

	lmb_init_and_reserve(&lmb, gd->bd, (void *)gd->fdt_blob);

	max_size = lmb_get_free_size(&lmb, image_load_addr);
	if (!max_size)
		return -1;

	wget_load_size = max_size;

	return 0;
}

/**
 * store_block() - store block in memory
 * @src: source of data
 * @offset: offset
 * @len: length
 */
static inline int store_block(uchar *src, unsigned int offset, unsigned int len)
{
	ulong store_addr = image_load_addr + offset;
	ulong newsize = offset + len;
	uchar *ptr;

	if (IS_ENABLED(CONFIG_LMB)) {
		ulong end_addr = image_load_addr + wget_load_size;

		if (!end_addr)
			end_addr = ULONG_MAX;

		if (store_addr < image_load_addr ||
		    store_addr + len > end_addr) {
			printf("\nwget error: ");
			printf("trying to overwrite reserved memory...\n");
			return -1;
		}
	}

	ptr = map_sysmem(store_addr, len);
	memcpy(ptr, src, len);
	unmap_sysmem(ptr);

	if (net_boot_file_size < (offset + len))
		net_boot_file_size = newsize;

	return 0;
}

/**
 * wget_send_stored() - wget response dispatcher
 *
 * WARNING, This, and only this, is the place in wget.c where
 * SEQUENCE NUMBERS are swapped between incoming (RX)
 * and outgoing (TX).
 * Procedure wget_handler() is correct for RX traffic.
 */
static void wget_send_stored(void)
{
	u8 action = retry_action;
	int len = retry_len;
	unsigned int tcp_ack_num = retry_tcp_seq_num + (len == 0 ? 1 : len);
	unsigned int tcp_seq_num = retry_tcp_ack_num;
	unsigned int server_port;
	uchar *ptr, *offset;

	server_port = env_get_ulong("httpdstp", 10, SERVER_PORT) & 0xffff;

	switch (current_wget_state) {
	case WGET_CLOSED:
		debug_cond(DEBUG_WGET, "wget: send SYN\n");
		current_wget_state = WGET_CONNECTING;
		net_send_tcp_packet(0, server_port, our_port, action,
				    tcp_seq_num, tcp_ack_num);
		packets = 0;
		break;
	case WGET_CONNECTING:
		pkt_q_idx = 0;
		net_send_tcp_packet(0, server_port, our_port, action,
				    tcp_seq_num, tcp_ack_num);

		ptr = net_tx_packet + net_eth_hdr_size() +
			IP_TCP_HDR_SIZE + TCP_TSOPT_SIZE + 2;
		offset = ptr;

		memcpy(offset, &bootfile1, strlen(bootfile1));
		offset += strlen(bootfile1);

		memcpy(offset, image_url, strlen(image_url));
		offset += strlen(image_url);

		memcpy(offset, &bootfile3, strlen(bootfile3));
		offset += strlen(bootfile3);
		net_send_tcp_packet((offset - ptr), server_port, our_port,
				    TCP_PUSH, tcp_seq_num, tcp_ack_num);
		current_wget_state = WGET_CONNECTED;
		break;
	case WGET_CONNECTED:
	case WGET_TRANSFERRING:
	case WGET_TRANSFERRED:
		net_send_tcp_packet(0, server_port, our_port, action,
				    tcp_seq_num, tcp_ack_num);
		break;
	}
}

static void wget_send(u8 action, unsigned int tcp_seq_num,
		      unsigned int tcp_ack_num, int len)
{
	retry_action = action;
	retry_tcp_ack_num = tcp_ack_num;
	retry_tcp_seq_num = tcp_seq_num;
	retry_len = len;

	wget_send_stored();
}

void wget_fail(char *error_message, unsigned int tcp_seq_num,
	       unsigned int tcp_ack_num, u8 action)
{
	printf("wget: Transfer Fail - %s\n", error_message);
	net_set_timeout_handler(0, NULL);
	wget_send(action, tcp_seq_num, tcp_ack_num, 0);
}

void wget_success(u8 action, unsigned int tcp_seq_num,
		  unsigned int tcp_ack_num, int len, int packets)
{
	printf("Packets received %d, Transfer Successful\n", packets);
	wget_send(action, tcp_seq_num, tcp_ack_num, len);
}

/*
 * Interfaces of U-BOOT
 */
static void wget_timeout_handler(void)
{
	if (++wget_timeout_count > WGET_RETRY_COUNT) {
		puts("\nRetry count exceeded; starting again\n");
		wget_send(TCP_RST, 0, 0, 0);
		net_start_again();
	} else {
		puts("T ");
		net_set_timeout_handler(wget_timeout +
					WGET_TIMEOUT * wget_timeout_count,
					wget_timeout_handler);
		wget_send_stored();
	}
}

#define PKT_QUEUE_OFFSET 0x20000
#define PKT_QUEUE_PACKET_SIZE 0x800

static void wget_connected(uchar *pkt, unsigned int tcp_seq_num,
			   u8 action, unsigned int tcp_ack_num, unsigned int len)
{
	uchar *pkt_in_q;
	char *pos;
	int hlen, i;
	uchar *ptr1;

	pkt[len] = '\0';
	pos = strstr((char *)pkt, http_eom);

	if (!pos) {
		debug_cond(DEBUG_WGET,
			   "wget: Connected, data before Header %p\n", pkt);
		pkt_in_q = (void *)image_load_addr + PKT_QUEUE_OFFSET +
			(pkt_q_idx * PKT_QUEUE_PACKET_SIZE);

		ptr1 = map_sysmem((phys_addr_t)pkt_in_q, len);
		memcpy(ptr1, pkt, len);
		unmap_sysmem(ptr1);

		pkt_q[pkt_q_idx].pkt = pkt_in_q;
		pkt_q[pkt_q_idx].tcp_seq_num = tcp_seq_num;
		pkt_q[pkt_q_idx].len = len;
		pkt_q_idx++;

		if (pkt_q_idx >= PKTQ_SZ) {
			printf("wget: Fatal error, queue overrun!\n");
			net_set_state(NETLOOP_FAIL);

			return;
		}
	} else {
		debug_cond(DEBUG_WGET, "wget: Connected HTTP Header %p\n", pkt);
		/* sizeof(http_eom) - 1 is the string length of (http_eom) */
		hlen = pos - (char *)pkt + sizeof(http_eom) - 1;
		pos = strstr((char *)pkt, linefeed);
		if (pos > 0)
			i = pos - (char *)pkt;
		else
			i = hlen;
		printf("%.*s", i,  pkt);

		current_wget_state = WGET_TRANSFERRING;

		if (strstr((char *)pkt, http_ok) == 0) {
			debug_cond(DEBUG_WGET,
				   "wget: Connected Bad Xfer\n");
			initial_data_seq_num = tcp_seq_num + hlen;
			wget_loop_state = NETLOOP_FAIL;
			wget_send(action, tcp_seq_num, tcp_ack_num, len);
		} else {
			debug_cond(DEBUG_WGET,
				   "wget: Connctd pkt %p  hlen %x\n",
				   pkt, hlen);
			initial_data_seq_num = tcp_seq_num + hlen;

			pos = strstr((char *)pkt, content_len);
			if (!pos) {
				content_length = -1;
			} else {
				pos += sizeof(content_len) + 2;
				strict_strtoul(pos, 10, &content_length);
				debug_cond(DEBUG_WGET,
					   "wget: Connected Len %lu\n",
					   content_length);
			}

			net_boot_file_size = 0;

			if (len > hlen) {
				if (store_block(pkt + hlen, 0, len - hlen) != 0) {
					wget_loop_state = NETLOOP_FAIL;
					wget_fail("wget: store error\n", tcp_seq_num, tcp_ack_num, action);
					net_set_state(NETLOOP_FAIL);
					return;
				}
			}

			debug_cond(DEBUG_WGET,
				   "wget: Connected Pkt %p hlen %x\n",
				   pkt, hlen);

			for (i = 0; i < pkt_q_idx; i++) {
				int err;

				ptr1 = map_sysmem(
					(phys_addr_t)(pkt_q[i].pkt),
					pkt_q[i].len);
				err = store_block(ptr1,
					  pkt_q[i].tcp_seq_num -
					  initial_data_seq_num,
					  pkt_q[i].len);
				unmap_sysmem(ptr1);
				debug_cond(DEBUG_WGET,
					   "wget: Connctd pkt Q %p len %x\n",
					   pkt_q[i].pkt, pkt_q[i].len);
				if (err) {
					wget_loop_state = NETLOOP_FAIL;
					wget_fail("wget: store error\n", tcp_seq_num, tcp_ack_num, action);
					net_set_state(NETLOOP_FAIL);
					return;
				}
			}
		}
	}
	wget_send(action, tcp_seq_num, tcp_ack_num, len);
}

/**
 * wget_handler() - TCP handler of wget
 * @pkt: pointer to the application packet
 * @dport: destination TCP port
 * @sip: source IP address
 * @sport: source TCP port
 * @tcp_seq_num: TCP sequential number
 * @tcp_ack_num: TCP acknowledgment number
 * @action: TCP action (SYN, ACK, FIN, etc)
 * @len: packet length
 *
 * In the "application push" invocation, the TCP header with all
 * its information is pointed to by the packet pointer.
 */
static void wget_handler(uchar *pkt, u16 dport,
			 struct in_addr sip, u16 sport,
			 u32 tcp_seq_num, u32 tcp_ack_num,
			 u8 action, unsigned int len)
{
	enum tcp_state wget_tcp_state = tcp_get_tcp_state();

	net_set_timeout_handler(wget_timeout, wget_timeout_handler);
	packets++;

	switch (current_wget_state) {
	case WGET_CLOSED:
		debug_cond(DEBUG_WGET, "wget: Handler: Error!, State wrong\n");
		break;
	case WGET_CONNECTING:
		debug_cond(DEBUG_WGET,
			   "wget: Connecting In len=%x, Seq=%u, Ack=%u\n",
			   len, tcp_seq_num, tcp_ack_num);
		if (!len) {
			if (wget_tcp_state == TCP_ESTABLISHED) {
				debug_cond(DEBUG_WGET,
					   "wget: Cting, send, len=%x\n", len);
				wget_send(action, tcp_seq_num, tcp_ack_num,
					  len);
			} else {
				printf("%.*s", len,  pkt);
				wget_fail("wget: Handler Connected Fail\n",
					  tcp_seq_num, tcp_ack_num, action);
			}
		}
		break;
	case WGET_CONNECTED:
		debug_cond(DEBUG_WGET, "wget: Connected seq=%u, len=%x\n",
			   tcp_seq_num, len);
		if (!len) {
			wget_fail("Image not found, no data returned\n",
				  tcp_seq_num, tcp_ack_num, action);
		} else {
			wget_connected(pkt, tcp_seq_num, action, tcp_ack_num, len);
		}
		break;
	case WGET_TRANSFERRING:
		debug_cond(DEBUG_WGET,
			   "wget: Transferring, seq=%x, ack=%x,len=%x\n",
			   tcp_seq_num, tcp_ack_num, len);

		if (tcp_seq_num >= initial_data_seq_num &&
		    store_block(pkt, tcp_seq_num - initial_data_seq_num,
				len) != 0) {
			wget_fail("wget: store error\n",
				  tcp_seq_num, tcp_ack_num, action);
			net_set_state(NETLOOP_FAIL);
			return;
		}

		switch (wget_tcp_state) {
		case TCP_FIN_WAIT_2:
			wget_send(TCP_ACK, tcp_seq_num, tcp_ack_num, len);
			fallthrough;
		case TCP_SYN_SENT:
		case TCP_SYN_RECEIVED:
		case TCP_CLOSING:
		case TCP_FIN_WAIT_1:
		case TCP_CLOSED:
			net_set_state(NETLOOP_FAIL);
			break;
		case TCP_ESTABLISHED:
			wget_send(TCP_ACK, tcp_seq_num, tcp_ack_num,
				  len);
			wget_loop_state = NETLOOP_SUCCESS;
			break;
		case TCP_CLOSE_WAIT:     /* End of transfer */
			current_wget_state = WGET_TRANSFERRED;
			wget_send(action | TCP_ACK | TCP_FIN,
				  tcp_seq_num, tcp_ack_num, len);
			break;
		}
		break;
	case WGET_TRANSFERRED:
		printf("Packets received %d, Transfer Successful\n", packets);
		net_set_state(wget_loop_state);
		break;
	}
}

#define RANDOM_PORT_START 1024
#define RANDOM_PORT_RANGE 0x4000

/**
 * random_port() - make port a little random (1024-17407)
 *
 * Return: random port number from 1024 to 17407
 *
 * This keeps the math somewhat trivial to compute, and seems to work with
 * all supported protocols/clients/servers
 */
static unsigned int random_port(void)
{
	return RANDOM_PORT_START + (get_timer(0) % RANDOM_PORT_RANGE);
}

#define BLOCKSIZE 512

void wget_start(void)
{
	image_url = strchr(net_boot_file_name, ':');
	if (image_url > 0) {
		web_server_ip = string_to_ip(net_boot_file_name);
		++image_url;
		net_server_ip = web_server_ip;
	} else {
		web_server_ip = net_server_ip;
		image_url = net_boot_file_name;
	}

	debug_cond(DEBUG_WGET,
		   "wget: Transfer HTTP Server %pI4; our IP %pI4\n",
		   &web_server_ip, &net_ip);

	/* Check if we need to send across this subnet */
	if (net_gateway.s_addr && net_netmask.s_addr) {
		struct in_addr our_net;
		struct in_addr server_net;

		our_net.s_addr = net_ip.s_addr & net_netmask.s_addr;
		server_net.s_addr = net_server_ip.s_addr & net_netmask.s_addr;
		if (our_net.s_addr != server_net.s_addr)
			debug_cond(DEBUG_WGET,
				   "wget: sending through gateway %pI4",
				   &net_gateway);
	}
	debug_cond(DEBUG_WGET, "URL '%s'\n", image_url);

	if (net_boot_file_expected_size_in_blocks) {
		debug_cond(DEBUG_WGET, "wget: Size is 0x%x Bytes = ",
			   net_boot_file_expected_size_in_blocks * BLOCKSIZE);
		print_size(net_boot_file_expected_size_in_blocks * BLOCKSIZE,
			   "");
	}
	debug_cond(DEBUG_WGET,
		   "\nwget:Load address: 0x%lx\nLoading: *\b", image_load_addr);

	if (IS_ENABLED(CONFIG_LMB)) {
		if (wget_init_load_size()) {
			printf("\nwget error: ");
			printf("trying to overwrite reserved memory...\n");
			net_set_state(NETLOOP_FAIL);
			return;
		}
	}

	net_set_timeout_handler(wget_timeout, wget_timeout_handler);
	tcp_set_tcp_handler(wget_handler);

	wget_timeout_count = 0;
	current_wget_state = WGET_CLOSED;

	our_port = random_port();

	/*
	 * Zero out server ether to force arp resolution in case
	 * the server ip for the previous u-boot command, for example dns
	 * is not the same as the web server ip.
	 */

	memset(net_server_ethaddr, 0, 6);

	wget_send(TCP_SYN, 0, 0, 0);
}

#if (IS_ENABLED(CONFIG_CMD_DNS))
int wget_with_dns(ulong dst_addr, char *uri)
{
	int ret;
	char *s, *host_name, *file_name, *str_copy;

	/*
	 * Download file using wget.
	 *
	 * U-Boot wget takes the target uri in this format.
	 *  "<http server ip>:<file path>"  e.g.) 192.168.1.1:/sample/test.iso
	 * Need to resolve the http server ip address before starting wget.
	 */
	str_copy = strdup(uri);
	if (!str_copy)
		return -ENOMEM;

	s = str_copy + strlen("http://");
	host_name = strsep(&s, "/");
	if (!s) {
		log_err("Error: invalied uri, no file path\n");
		ret = -EINVAL;
		goto out;
	}
	file_name = s;

	/* TODO: If the given uri has ip address for the http server, skip dns */
	net_dns_resolve = host_name;
	net_dns_env_var = "httpserverip";
	if (net_loop(DNS) < 0) {
		log_err("Error: dns lookup of %s failed, check setup\n", net_dns_resolve);
		ret = -EINVAL;
		goto out;
	}
	s = env_get("httpserverip");
	if (!s) {
		ret = -EINVAL;
		goto out;
	}

	strlcpy(net_boot_file_name, s, sizeof(net_boot_file_name));
	strlcat(net_boot_file_name, ":/", sizeof(net_boot_file_name)); /* append '/' which is removed by strsep() */
	strlcat(net_boot_file_name, file_name, sizeof(net_boot_file_name));
	image_load_addr = dst_addr;
	ret = net_loop(WGET);

out:
	free(str_copy);

	return ret;
}
#endif

/**
 * wget_validate_uri() - validate the uri for wget
 *
 * @uri:	uri string
 *
 * This function follows the current U-Boot wget implementation.
 * scheme: only "http:" is supported
 * authority:
 *   - user information: not supported
 *   - host: supported
 *   - port: not supported(always use the default port)
 *
 * Uri is expected to be correctly percent encoded.
 * This is the minimum check, control codes(0x1-0x19, 0x7F, except '\0')
 * and space character(0x20) are not allowed.
 *
 * TODO: stricter uri conformance check
 *
 * Return:	true on success, false on failure
 */
bool wget_validate_uri(char *uri)
{
	char c;
	bool ret = true;
	char *str_copy, *s, *authority;

	for (c = 0x1; c < 0x21; c++) {
		if (strchr(uri, c)) {
			log_err("invalid character is used\n");
			return false;
		}
	}
	if (strchr(uri, 0x7f)) {
		log_err("invalid character is used\n");
		return false;
	}

	if (strncmp(uri, "http://", 7)) {
		log_err("only http:// is supported\n");
		return false;
	}
	str_copy = strdup(uri);
	if (!str_copy)
		return false;

	s = str_copy + strlen("http://");
	authority = strsep(&s, "/");
	if (!s) {
		log_err("invalid uri, no file path\n");
		ret = false;
		goto out;
	}
	s = strchr(authority, '@');
	if (s) {
		log_err("user information is not supported\n");
		ret = false;
		goto out;
	}
	s = strchr(authority, ':');
	if (s) {
		log_err("user defined port is not supported\n");
		ret = false;
		goto out;
	}

out:
	free(str_copy);

	return ret;
}
