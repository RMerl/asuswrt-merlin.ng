/*
 * BPF program compilation tool
 *
 * Generates decimal output, similar to `tcpdump -ddd ...`.
 * Unlike tcpdump, will generate for any given link layer type.
 *
 * Written by Willem de Bruijn (willemb@google.com)
 * Copyright Google, Inc. 2013
 * Licensed under the GNU General Public License version 2 (GPLv2)
*/

#include <pcap.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	struct bpf_program program;
	struct bpf_insn *ins;
	int i, dlt = DLT_RAW;
	pcap_t *pcap;

	if (argc < 2 || argc > 3) {
		fprintf(stderr, "Usage:    %s [link] '<program>'\n\n"
				"          link is a pcap linklayer type:\n"
				"          one of EN10MB, RAW, SLIP, ...\n\n"
				"Examples: %s RAW 'tcp and greater 100'\n"
				"          %s EN10MB 'ip proto 47'\n'",
				argv[0], argv[0], argv[0]);
		return 1;
	}

	if (argc == 3) {
		dlt = pcap_datalink_name_to_val(argv[1]);
		if (dlt == -1) {
			fprintf(stderr, "Unknown datalinktype: %s\n", argv[1]);
			return 1;
		}
	}

	pcap = pcap_open_dead(dlt, 65535);
	if (!pcap) {
		fprintf(stderr, "Memory allocation failure\n");
		return 1;
	}
	if (pcap_compile(pcap, &program, argv[argc - 1], 1,
				PCAP_NETMASK_UNKNOWN)) {
		fprintf(stderr, "Compilation error\n");
		pcap_close(pcap);
		return 1;
	}

	printf("%d,", program.bf_len);
	ins = program.bf_insns;
	for (i = 0; i < program.bf_len-1; ++ins, ++i)
		printf("%u %u %u %u,", ins->code, ins->jt, ins->jf, ins->k);

	printf("%u %u %u %u\n", ins->code, ins->jt, ins->jf, ins->k);

	pcap_freecode(&program);
	pcap_close(pcap);
	return 0;
}

