/*
 * Copyright (C) 2007-2016  B.A.T.M.A.N. contributors:
 *
 * Andreas Langer <an.langer@gmx.de>, Marek Lindner <mareklindner@neomailbox.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */



#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/if_ether.h>

#include "main.h"
#include "ping.h"
#include "functions.h"
#include "packet.h"
#include "bat-hosts.h"
#include "debugfs.h"


char is_aborted = 0;


static void ping_usage(void)
{
	fprintf(stderr, "Usage: batctl [options] ping [parameters] mac|bat-host|host_name|IPv4_address \n");
	fprintf(stderr, "parameters:\n");
	fprintf(stderr, " \t -c ping packet count \n");
	fprintf(stderr, " \t -h print this help\n");
	fprintf(stderr, " \t -i interval in seconds\n");
	fprintf(stderr, " \t -t timeout in seconds\n");
	fprintf(stderr, " \t -R record route\n");
	fprintf(stderr, " \t -T don't try to translate mac to originator address\n");
}

static void sig_handler(int sig)
{
	switch (sig) {
	case SIGINT:
	case SIGTERM:
		is_aborted = 1;
		break;
	default:
		break;
	}
}

int ping(char *mesh_iface, int argc, char **argv)
{
	struct batadv_icmp_packet_rr icmp_packet_out, icmp_packet_in;
	struct timeval tv;
	struct ether_addr *dst_mac = NULL, *rr_mac = NULL;
	struct bat_host *bat_host, *rr_host;
	ssize_t read_len;
	fd_set read_socket;
	int ret = EXIT_FAILURE, ping_fd = -1, res, optchar, found_args = 1;
	int loop_count = -1, loop_interval = 0, timeout = 1, rr = 0, i;
	unsigned int seq_counter = 0, packets_out = 0, packets_in = 0, packets_loss;
	char *dst_string, *mac_string, *rr_string;
	double time_delta;
	float min = 0.0, max = 0.0, avg = 0.0, mdev = 0.0;
	uint8_t last_rr_cur = 0, last_rr[BATADV_RR_LEN][ETH_ALEN];
	size_t packet_len;
	char *debugfs_mnt;
	char icmp_socket[MAX_PATH+1];
	int disable_translate_mac = 0;

	while ((optchar = getopt(argc, argv, "hc:i:t:RT")) != -1) {
		switch (optchar) {
		case 'c':
			loop_count = strtol(optarg, NULL , 10);
			if (loop_count < 1)
				loop_count = -1;
			found_args += ((*((char*)(optarg - 1)) == optchar ) ? 1 : 2);
			break;
		case 'h':
			ping_usage();
			return EXIT_SUCCESS;
		case 'i':
			loop_interval = strtol(optarg, NULL , 10);
			if (loop_interval < 1)
				loop_interval = 1;
			found_args += ((*((char*)(optarg - 1)) == optchar ) ? 1 : 2);
			break;
		case 't':
			timeout = strtol(optarg, NULL , 10);
			if (timeout < 1)
				timeout = 1;
			found_args += ((*((char*)(optarg - 1)) == optchar ) ? 1 : 2);
			break;
		case 'R':
			rr = 1;
			found_args++;
			break;
		case 'T':
			disable_translate_mac = 1;
			found_args += 1;
			break;
		default:
			ping_usage();
			return EXIT_FAILURE;
		}
	}

	if (argc <= found_args) {
		fprintf(stderr, "Error - target mac address or bat-host name not specified\n");
		ping_usage();
		return EXIT_FAILURE;
	}

	dst_string = argv[found_args];
	bat_hosts_init(0);
	bat_host = bat_hosts_find_by_name(dst_string);

	if (bat_host)
		dst_mac = &bat_host->mac_addr;

	if (!dst_mac) {
		dst_mac = resolve_mac(dst_string);

		if (!dst_mac) {
			fprintf(stderr, "Error - mac address of the ping destination could not be resolved and is not a bat-host name: %s\n", dst_string);
			goto out;
		}
	}

	if (!disable_translate_mac)
		dst_mac = translate_mac(mesh_iface, dst_mac);

	mac_string = ether_ntoa_long(dst_mac);
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);

	debugfs_mnt = debugfs_mount(NULL);
	if (!debugfs_mnt) {
		fprintf(stderr, "Error - can't mount or find debugfs\n");
		goto out;
	}

	debugfs_make_path(SOCKET_PATH_FMT, mesh_iface, icmp_socket, sizeof(icmp_socket));

	ping_fd = open(icmp_socket, O_RDWR);

	if (ping_fd < 0) {
		fprintf(stderr, "Error - can't open a connection to the batman adv kernel module via the socket '%s': %s\n",
				icmp_socket, strerror(errno));
		printf("Check whether the module is loaded and active.\n");
		goto out;
	}

	packet_len = sizeof(struct batadv_icmp_packet);

	memset(&icmp_packet_out, 0, sizeof(icmp_packet_out));
	memcpy(&icmp_packet_out.dst, dst_mac, ETH_ALEN);
	icmp_packet_out.packet_type = BATADV_ICMP;
	icmp_packet_out.version = BATADV_COMPAT_VERSION;
	icmp_packet_out.msg_type = BATADV_ECHO_REQUEST;
	icmp_packet_out.ttl = 50;
	icmp_packet_out.seqno = 0;

	if (rr) {
		packet_len = sizeof(struct batadv_icmp_packet_rr);
		icmp_packet_out.rr_cur = 1;
		memset(&icmp_packet_out.rr, 0, BATADV_RR_LEN * ETH_ALEN);
		memset(last_rr, 0, BATADV_RR_LEN * ETH_ALEN);
	} else {
		((struct batadv_icmp_packet *)&icmp_packet_out)->reserved = 0;
	}

	printf("PING %s (%s) %zu(%zu) bytes of data\n", dst_string, mac_string,
		packet_len, packet_len + 28);

	while (!is_aborted) {
		tv.tv_sec = timeout;
		tv.tv_usec = 0;

		if (loop_count == 0)
			break;

		if (loop_count > 0)
			loop_count--;

		icmp_packet_out.seqno = htons(++seq_counter);

		if (write(ping_fd, (char *)&icmp_packet_out, packet_len) < 0) {
			fprintf(stderr, "Error - can't write to batman adv kernel file '%s': %s\n", icmp_socket, strerror(errno));
			goto sleep;
		}

read_packet:
		start_timer();

		FD_ZERO(&read_socket);
		FD_SET(ping_fd, &read_socket);

		res = select(ping_fd + 1, &read_socket, NULL, NULL, &tv);

		if (is_aborted)
			break;

		packets_out++;

		if (res == 0) {
			printf("Reply from host %s timed out\n", dst_string);
			goto sleep;
		}

		if (res < 0)
			goto sleep;

		read_len = read(ping_fd, (char *)&icmp_packet_in, packet_len);

		if (read_len < 0) {
			fprintf(stderr, "Error - can't read from batman adv kernel file '%s': %s\n", icmp_socket, strerror(errno));
			goto sleep;
		}

		if ((size_t)read_len < packet_len) {
			printf("Warning - dropping received packet as it is smaller than expected (%zu): %zd\n",
				packet_len, read_len);
			goto sleep;
		}

		/* after receiving an unexpected seqno we keep waiting for our answer */
		if (htons(seq_counter) != icmp_packet_in.seqno)
			goto read_packet;

		switch (icmp_packet_in.msg_type) {
		case BATADV_ECHO_REPLY:
			time_delta = end_timer();
			printf("%zd bytes from %s icmp_seq=%hu ttl=%d time=%.2f ms",
					read_len, dst_string,
					ntohs(icmp_packet_in.seqno),
					icmp_packet_in.ttl,
					time_delta);

			if (read_len == sizeof(struct batadv_icmp_packet_rr)) {
				if (last_rr_cur == icmp_packet_in.rr_cur
					&& !memcmp(last_rr, icmp_packet_in.rr, BATADV_RR_LEN * ETH_ALEN)) {

					printf("\t(same route)");

				} else {
					printf("\nRR: ");

					for (i = 0; i < BATADV_RR_LEN
						&& i < icmp_packet_in.rr_cur; i++) {

						rr_mac = (struct ether_addr *)&icmp_packet_in.rr[i];
						rr_host = bat_hosts_find_by_mac((char *)rr_mac);
						if (rr_host)
							rr_string = rr_host->name;
						else
							rr_string = ether_ntoa_long(rr_mac);
						printf("\t%s\n", rr_string);

						if (memcmp(rr_mac, dst_mac, ETH_ALEN) == 0)
							printf("\t%s\n", rr_string);
					}

					last_rr_cur = icmp_packet_in.rr_cur;
					memcpy(last_rr, icmp_packet_in.rr, BATADV_RR_LEN * ETH_ALEN);
				}
			}

			printf("\n");

			if ((time_delta < min) || (min == 0.0))
				min = time_delta;
			if (time_delta > max)
				max = time_delta;
			avg += time_delta;
			mdev += time_delta * time_delta;
			packets_in++;
			break;
		case BATADV_DESTINATION_UNREACHABLE:
			printf("From %s: Destination Host Unreachable (icmp_seq %hu)\n", dst_string, ntohs(icmp_packet_in.seqno));
			break;
		case BATADV_TTL_EXCEEDED:
			printf("From %s: Time to live exceeded (icmp_seq %hu)\n", dst_string, ntohs(icmp_packet_in.seqno));
			break;
		case BATADV_PARAMETER_PROBLEM:
			fprintf(stderr, "Error - the batman adv kernel module version (%d) differs from ours (%d)\n",
				icmp_packet_in.version, BATADV_COMPAT_VERSION);
			printf("Please make sure to use compatible versions!\n");
			goto out;
		default:
			printf("Unknown message type %d len %zd received\n",
			       icmp_packet_in.msg_type, read_len);
			break;
		}

sleep:
		if (loop_interval > 0)
			sleep(loop_interval);
		else if ((tv.tv_sec != 0) || (tv.tv_usec != 0))
			select(0, NULL, NULL, NULL, &tv);
	}

	if (packets_out == 0)
		packets_loss = 0;
	else
		packets_loss = ((packets_out - packets_in) * 100) / packets_out;

	if (packets_in) {
		avg /= packets_in;
		mdev /= packets_in;
		mdev = mdev - avg * avg;
		if (mdev > 0.0)
			mdev = sqrt(mdev);
		else
			mdev = 0.0;
	} else {
		avg = 0.0;
		mdev = 0.0;
	}

	printf("--- %s ping statistics ---\n", dst_string);
	printf("%u packets transmitted, %u received, %u%% packet loss\n",
		packets_out, packets_in, packets_loss);
	printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n",
		min, avg, max, mdev);

	if (packets_in)
		ret = EXIT_SUCCESS;
	else
		ret = EXIT_NOSUCCESS;

out:
	bat_hosts_free();
	if (ping_fd >= 0)
		close(ping_fd);
	return ret;
}
