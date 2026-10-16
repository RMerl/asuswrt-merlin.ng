/*
 * Copyright (c) 2013 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pcap/pcap.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

static const char *iface = "lo";
static uint16_t port;
static const char *chain = "SYNPROXY";

static int parse_packet(const char *host, const uint8_t *data)
{
	const struct iphdr *iph = (void *)data + 14;
	const struct tcphdr *th = (void *)iph + iph->ihl * 4;
	int length;
	uint8_t *ptr;

	if (!th->syn || !th->ack)
		return 0;

	printf("-A %s -d %s -p tcp --dport %u "
	       "-m state --state UNTRACKED,INVALID "
	       "-j SYNPROXY ", chain, host, port);

	/* ECE && !CWR */
	if (th->res2 == 0x1)
		printf("--ecn ");

	length = th->doff * 4 - sizeof(*th);
	ptr = (uint8_t *)(th + 1);
	while (length > 0) {
		int opcode = *ptr++;
		int opsize;

		switch (opcode) {
		case TCPOPT_EOL:
			return 1;
		case TCPOPT_NOP:
			length--;
			continue;
		default:
			opsize = *ptr++;
			if (opsize < 2)
				return 1;
			if (opsize > length)
				return 1;

			switch (opcode) {
			case TCPOPT_MAXSEG:
				if (opsize == TCPOLEN_MAXSEG)
					printf("--mss %u ", ntohs(*(uint16_t *)ptr));
				break;
			case TCPOPT_WINDOW:
				if (opsize == TCPOLEN_WINDOW)
					printf("--wscale %u ", *ptr);
				break;
			case TCPOPT_TIMESTAMP:
				if (opsize == TCPOLEN_TIMESTAMP)
					printf("--timestamp ");
				break;
			case TCPOPT_SACK_PERMITTED:
				if (opsize == TCPOLEN_SACK_PERMITTED)
					printf("--sack-perm ");
				break;
			}

			ptr += opsize - 2;
			length -= opsize;
		}
	}
	printf("\n");
	return 1;
}

static void probe_host(const char *host)
{
	struct sockaddr_in sin;
	char pcap_errbuf[PCAP_ERRBUF_SIZE];
	struct pcap_pkthdr pkthdr;
	const uint8_t *data;
	struct bpf_program fp;
	pcap_t *ph;
	int fd;

	ph = pcap_create(iface, pcap_errbuf);
	if (ph == NULL) {
		perror("pcap_create");
		goto err1;
	}

	if (pcap_setnonblock(ph, 1, pcap_errbuf) == -1) {
		perror("pcap_setnonblock");
		goto err2;
	}

	if (pcap_setfilter(ph, &fp) == -1) {
		pcap_perror(ph, "pcap_setfilter");
		goto err2;
	}

	if (pcap_activate(ph) != 0) {
		pcap_perror(ph, "pcap_activate");
		goto err2;
	}

	if (pcap_compile(ph, &fp, "src host 127.0.0.1 and tcp and src port 80",
			 1, PCAP_NETMASK_UNKNOWN) == -1) {
		pcap_perror(ph, "pcap_compile");
		goto err2;
	}

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket");
		goto err3;
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family		= AF_INET;
	sin.sin_port		= htons(port);
	sin.sin_addr.s_addr	= inet_addr(host);

	if (connect(fd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		perror("connect");
		goto err4;
	}

	for (;;) {
		data = pcap_next(ph, &pkthdr);
		if (data == NULL)
			break;
		if (parse_packet(host, data))
			break;
	}

	close(fd);

err4:
	close(fd);
err3:
	pcap_freecode(&fp);
err2:
	pcap_close(ph);
err1:
	return;
}

enum {
	OPT_HELP	= 'h',
	OPT_IFACE	= 'i',
	OPT_PORT	= 'p',
	OPT_CHAIN	= 'c',
};

static const struct option options[] = {
	{ .name = "help",  .has_arg = false, .val = OPT_HELP },
	{ .name = "iface", .has_arg = true,  .val = OPT_IFACE },
	{ .name = "port" , .has_arg = true,  .val = OPT_PORT },
	{ .name = "chain", .has_arg = true,  .val = OPT_CHAIN },
	{ }
};

static void print_help(const char *name)
{
	printf("%s [ options ] address...\n"
	       "\n"
	       "Options:\n"
	       " -i/--iface        Outbound interface\n"
	       " -p/--port         Port number to probe\n"
	       " -c/--chain        Chain name to use for rules\n"
	       " -h/--help         Show this help\n",
	       name);
}

int main(int argc, char **argv)
{
	int optidx = 0, c;

	for (;;) {
		c = getopt_long(argc, argv, "hi:p:c:", options, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case OPT_IFACE:
			iface = optarg;
			break;
		case OPT_PORT:
			port = atoi(optarg);
			break;
		case OPT_CHAIN:
			chain = optarg;
			break;
		case OPT_HELP:
			print_help(argv[0]);
			exit(0);
		case '?':
			print_help(argv[0]);
			exit(1);
		}
	}

	argc -= optind;
	argv += optind;

	while (argc > 0) {
		probe_host(*argv);
		argc--;
		argv++;
	}
	return 0;
}
