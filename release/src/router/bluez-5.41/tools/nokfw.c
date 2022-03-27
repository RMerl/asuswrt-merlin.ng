/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2013  Intel Corporation
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/mman.h>

struct neg_cmd {
	uint8_t  ack;
	uint16_t baud;
	uint16_t unused1;
	uint8_t  proto;
	uint16_t sys_clk;
	uint16_t unused2;
} __attribute__ ((packed));

struct alive_pkt {
	uint8_t  mid;
	uint8_t  unused;
} __attribute__ ((packed));

static void print_cmd(uint16_t opcode, const uint8_t *buf, uint8_t plen)
{
	switch (opcode) {
	case 0x0c43:
		printf(" Write_Inquiry_Scan_Type [type=%u]", buf[0]);
		break;
	case 0x0c47:
		printf(" Write_Page_Scan_Type [type=%u]", buf[0]);
		break;
	case 0xfc01:
		printf(" Write_BD_ADDR [bdaddr=%02x:%02x:%02x:%02x:%02x:%02x]",
			buf[5], buf[4], buf[3], buf[2], buf[1], buf[0]);
		break;
	case 0xfc0b:
		printf(" Write_Local_Supported_Features");
		printf(" [features=%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x]",
					buf[0], buf[1], buf[2], buf[3],
					buf[4], buf[5], buf[6], buf[7]);
		break;
	case 0xfc0a:
		printf(" Super_Peek_Poke [type=%u]", buf[0]);
		break;
	case 0xfc15:
		printf(" FM_RDS_Command [register=0x%02x,mode=%u]",
							buf[0], buf[1]);
		break;
	case 0xfc18:
		printf(" Update_UART_Baud_Rate");
		break;
	case 0xfc1c:
		printf(" Write_SCO_PCM_Int_Param");
		break;
	case 0xfc1e:
		printf(" Write_PCM_Data_Format_Param");
		break;
	case 0xfc22:
		printf(" Write_SCO_Time_Slot [slot=%u]", buf[0]);
		break;
	case 0xfc41:
		printf(" Write_Collaboration_Mode");
		break;
	case 0xfc4c:
		printf(" Write_RAM [address=0x%08x]",
			buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24);
		break;
	case 0xfc4e:
		printf(" Launch_RAM [address=0x%08x]",
			buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24);
		break;
	case 0xfc61:
		printf(" Write_PCM_Pins");
		break;
	}
}

static void analyze_memory(const uint8_t *buf, size_t len)
{
	const uint8_t *ptr = buf;
	const struct neg_cmd *neg;
	const struct alive_pkt *alive;
	uint16_t pkt_len, opcode;
	uint8_t pkt_type, plen;

	while (ptr < buf + len) {
		pkt_len = ptr[0] | ptr[1] << 8;
		pkt_type = ptr[2];

		printf("len=%-3u type=%u,", pkt_len, pkt_type);

		switch (pkt_type) {
		case 0x01:
			opcode = ptr[3] | ptr[4] << 8;
			plen = ptr[5];
			printf("%-5s opcode=0x%04x plen=%-3u", "cmd",
							opcode, plen);
			print_cmd(opcode, ptr + 6, plen);
			break;
		case 0x06:
			plen = ptr[3];
			printf("%-5s plen=%-2u", "neg", plen);
			neg = (void *) (ptr + 4);
			printf(" [ack=%u baud=%u proto=0x%02x sys_clk=%u]",
				neg->ack, neg->baud, neg->proto, neg->sys_clk);
			break;
		case 0x07:
			plen = ptr[3];
			printf("%-5s plen=%-2u", "alive", plen);
			alive = (void *) (ptr + 4);
			printf(" [mid=0x%02x]", alive->mid);
			break;
		case 0x08:
			opcode = ptr[3] | ptr[4] << 8;
			plen = ptr[5];
			printf("%-5s opcode=0x%04x plen=%-3u", "radio",
							opcode, plen);
			print_cmd(opcode, ptr + 6, plen);
			break;
		default:
			printf("unknown");
			break;
		}

		printf("\n");

		ptr += pkt_len + 2;
	}
}

static void analyze_file(const char *pathname)
{
	struct stat st;
	void *map;
	int fd;

	printf("Analyzing %s\n", pathname);

	fd = open(pathname, O_RDONLY | O_CLOEXEC);
	if (fd < 0) {
		perror("Failed to open file");
		return;
	}

	if (fstat(fd, &st) < 0) {
		fprintf(stderr, "Failed get file size\n");
		close(fd);
		return;
	}

	if (st.st_size == 0) {
		fprintf(stderr, "Empty file\n");
		close(fd);
		return;
	}

	map = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (!map || map == MAP_FAILED) {
		fprintf(stderr, "Failed to map file\n");
		close(fd);
		return;
        }

	analyze_memory(map, st.st_size);

	munmap(map, st.st_size);
	close(fd);
}

static void usage(void)
{
	printf("Nokia Bluetooth firmware analyzer\n"
		"Usage:\n");
	printf("\tnokfw [options] <file>\n");
	printf("Options:\n"
		"\t-h, --help             Show help options\n");
}

static const struct option main_options[] = {
	{ "version", no_argument,       NULL, 'v' },
	{ "help",    no_argument,       NULL, 'h' },
	{ }
};

int main(int argc, char *argv[])
{
	int i;

	for (;;) {
		int opt;

		opt = getopt_long(argc, argv, "vh", main_options, NULL);
		if (opt < 0)
			break;

		switch (opt) {
		case 'v':
			printf("%s\n", VERSION);
			return EXIT_SUCCESS;
		case 'h':
			usage();
			return EXIT_SUCCESS;
		default:
			return EXIT_FAILURE;
		}
	}

	if (argc - optind < 1) {
		fprintf(stderr, "No input firmware files provided\n");
		return EXIT_FAILURE;
	}

	for (i = optind; i < argc; i++)
		analyze_file(argv[i]);

	return EXIT_SUCCESS;
}
