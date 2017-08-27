#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <getopt.h>
#include "../include/asoundlib.h"

#include "seq-decoder.c"
#include "seq-sender.c"

#define SEQ_VERSION "0.0.1"

#define HELPID_HELP             1000
#define HELPID_DEBUG            1001
#define HELPID_VERBOSE		1002
#define HELPID_VERSION          1003

int max_clients;
int max_ports;
int max_queues;
int debug = 0;
int verbose = 0;

void set_name(snd_seq_t *handle)
{
	int err;
	char name[64];
	
	sprintf(name, "SeqUtil - %i", getpid());
	if ((err = snd_seq_set_client_name(handle, name)) < 0) {
		fprintf(stderr, "Set client info error: %s\n", snd_strerror(err));
		exit(0);
	}
}

void system_info(snd_seq_t *handle)
{
	int err;
	snd_seq_system_info_t *sysinfo;
	
	snd_seq_system_info_alloca(&sysinfo);
	if ((err = snd_seq_system_info(handle, sysinfo))<0) {
		fprintf(stderr, "System info error: %s\n", snd_strerror(err));
		exit(0);
	}
	max_clients = snd_seq_system_info_get_clients(sysinfo);
	max_ports = snd_seq_system_info_get_ports(sysinfo);
	max_queues = snd_seq_system_info_get_ports(sysinfo);
}

void show_system_info(snd_seq_t *handle ATTRIBUTE_UNUSED)
{
	printf("System info\n");
	printf("  Max queues    : %i\n", max_queues);
	printf("  Max clients   : %i\n", max_clients);
	printf("  Max ports     : %i\n", max_ports);
}

void show_queue_status(snd_seq_t *handle, int queue)
{
	int err, idx, min, max;
	snd_seq_queue_status_t *status;

	snd_seq_queue_status_alloca(&status);
	min = queue < 0 ? 0 : queue;
	max = queue < 0 ? max_queues : queue + 1;
	for (idx = min; idx < max; idx++) {
		if ((err = snd_seq_get_queue_status(handle, idx, status))<0) {
			if (err == -ENOENT)
				continue;
			fprintf(stderr, "Client %i info error: %s\n", idx, snd_strerror(err));
			exit(0);
		}
		printf("Queue %i info\n", snd_seq_queue_status_get_queue(status));
		printf("  Tick          : %u\n", snd_seq_queue_status_get_tick_time(status)); 
		printf("  Realtime      : %i.%i\n",
		       snd_seq_queue_status_get_real_time(status)->tv_sec,
		       snd_seq_queue_status_get_real_time(status)->tv_nsec);
		printf("  Flags         : 0x%x\n", snd_seq_queue_status_get_status(status));
	}
}

void show_port_info(snd_seq_t *handle, int client, int port)
{
	int err, idx, min, max;
	snd_seq_port_info_t *info;

	snd_seq_port_info_alloca(&info);
	min = port < 0 ? 0 : port;
	max = port < 0 ? max_ports : port + 1;
	for (idx = min; idx < max; idx++) {
		if ((err = snd_seq_get_any_port_info(handle, client, idx, info))<0) {
			if (err == -ENOENT)
				continue;
			fprintf(stderr, "Port %i/%i info error: %s\n", client, idx, snd_strerror(err));
			exit(0);
		}
		printf("  Port %i info\n", idx);
		printf("    Client        : %i\n", snd_seq_port_info_get_client(info));
		printf("    Port          : %i\n", snd_seq_port_info_get_port(info));
		printf("    Name          : %s\n", snd_seq_port_info_get_name(info));
		printf("    Capability    : 0x%x\n", snd_seq_port_info_get_capability(info));
		printf("    Type          : 0x%x\n", snd_seq_port_info_get_type(info));
		//printf("    Midi channels : %i\n", info.midi_channels);
		//printf("    Synth voices  : %i\n", info.synth_voices);
		printf("    Output subs   : %i\n", snd_seq_port_info_get_write_use(info));
		printf("    Input subs    : %i\n", snd_seq_port_info_get_read_use(info));
	}
}

void show_client_info(snd_seq_t *handle, int client)
{
	int err, idx, min, max;
	snd_seq_client_info_t *info;

	snd_seq_client_info_alloca(&info);
	min = client < 0 ? 0 : client;
	max = client < 0 ? max_clients : client + 1;
	for (idx = min; idx < max; idx++) {
		if ((err = snd_seq_get_any_client_info(handle, idx, info))<0) {
			if (err == -ENOENT)
				continue;
			fprintf(stderr, "Client %i info error: %s\n", idx, snd_strerror(err));
			exit(0);
		}
		printf("Client %i info\n", idx);
		if (verbose)
			printf("  Client        : %i\n", snd_seq_client_info_get_client(info));
		printf("  Type          : %s\n", snd_seq_client_info_get_type(info) == SND_SEQ_KERNEL_CLIENT ? "kernel" : "user");
		printf("  Name          : %s\n", snd_seq_client_info_get_name(info));
	}
}

static void help(void)
{
	printf("Usage: seq <options> command\n");
	printf("\nAvailable options:\n");
	printf("  -h,--help       this help\n");
	printf("  -d,--debug      debug mode\n");
	printf("  -v,--verbose    verbose mode\n");
	printf("  -V,--version    print version of this program\n");
	printf("\nAvailable commands:\n");
	printf("  system          show basic sequencer info\n");
	printf("  queue [#]       show all queues or specified queue\n");
	printf("  client [#]      show all clients or specified client\n");
	printf("  port <client> [#]  show all ports or specified port for specified client\n");
	printf("  decoder         event decoder\n");
	printf("  sender <client.port> [<client.port>] ...   event sender\n");
}

int main(int argc, char *argv[])
{
	int morehelp, err, arg, arg1;
	snd_seq_t *handle;
	static struct option long_option[] =
	{
		{"help", 0, NULL, HELPID_HELP},
		{"debug", 0, NULL, HELPID_DEBUG},
		{"verbose", 0, NULL, HELPID_VERBOSE},
		{"version", 0, NULL, HELPID_VERSION},
		{NULL, 0, NULL, 0},
        };
        
        morehelp = 0;
	
	while (1) {
		int c;

		if ((c = getopt_long(argc, argv, "hdvV", long_option, NULL)) < 0)
			break;
		switch (c) {
		case 'h':
		case HELPID_HELP:
			morehelp++;
			break;
		case 'd':
		case HELPID_DEBUG:
			debug = 1;
			break;
		case 'v':
		case HELPID_VERBOSE:
			verbose = 1;
			break;
		case 'V':
		case HELPID_VERSION:
			printf("alsactl version " SEQ_VERSION "\n");
			return 1;
		default:
			fprintf(stderr, "\07Invalid switch or option needs an argument.\n");
			morehelp++;
		}
	}
        if (morehelp) {
                help();
                return 1;
        }
	if (argc - optind <= 0) {
		fprintf(stderr, "seq: Specify command...\n");
		return 0;
	}
	if ((err = snd_seq_open(&handle, "hw", SND_SEQ_OPEN_DUPLEX, 0))<0) {
		fprintf(stderr, "Open error: %s\n", snd_strerror(err));
		exit(0);
	}
	set_name(handle);
	system_info(handle);

        if (!strcmp(argv[optind], "system")) {
		show_system_info(handle);
	} else if (!strcmp(argv[optind], "queue")) {
		arg = argc - optind > 1 ? atoi(argv[optind + 1]) : -1;
		show_queue_status(handle, arg);
	} else if (!strcmp(argv[optind], "client")) {
		arg = argc - optind > 1 ? atoi(argv[optind + 1]) : -1;
		show_client_info(handle, arg);
	} else if (!strcmp(argv[optind], "port")) {
		arg = argc - optind > 1 ? atoi(argv[optind + 1]) : -1;
		if (arg < 0) {
			fprintf(stderr, "Specify port...\n");
			exit(0);
		}
		arg1 = argc - optind > 2 ? atoi(argv[optind + 2]) : -1;
		show_port_info(handle, arg, arg1);
	} else if (!strcmp(argv[optind], "decoder")) {
		event_decoder(handle, argc - optind - 1, argv + optind + 1);
	} else if (!strcmp(argv[optind], "sender")) {
		event_sender(handle, argc - optind - 1, argv + optind + 1);
	} else {
		help();
	}
	exit(1);
}
