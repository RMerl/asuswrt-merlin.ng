// SPDX-License-Identifier: GPL-2.0+
/*
 * caddy.c -- esd VME8349 support for "missing" access modes in TSI148.
 * Copyright (c) 2009 esd gmbh.
 *
 * Reinhard Arlt <reinhard.arlt@esd-electronics.com>
 */

#include <common.h>
#include <console.h>
#include <ioports.h>
#include <mpc83xx.h>
#include <asm/mpc8349_pci.h>
#include <pci.h>
#include <asm/mmu.h>
#include <asm/io.h>

#include "caddy.h"

static struct caddy_interface *caddy_interface;

void generate_answer(struct caddy_cmd *cmd, uint32_t status, uint32_t *result)
{
	struct caddy_answer *answer;
	uint32_t ptr;

	answer = &caddy_interface->answer[caddy_interface->answer_in];
	memset((void *)answer, 0, sizeof(struct caddy_answer));
	answer->answer = cmd->cmd;
	answer->issue = cmd->issue;
	answer->status = status;
	memcpy(answer->par, result, 5 * sizeof(result[0]));
	ptr = caddy_interface->answer_in + 1;
	ptr = ptr & (ANSWER_SIZE - 1);
	if (ptr != caddy_interface->answer_out)
		caddy_interface->answer_in = ptr;
}

int do_caddy(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long base_addr;
	uint32_t ptr;
	struct caddy_cmd *caddy_cmd;
	uint32_t result[5];
	uint16_t data16;
	uint8_t data8;
	uint32_t status;
	pci_dev_t dev;
	void *pci_ptr;

	if (argc < 2) {
		puts("Missing parameter\n");
		return 1;
	}

	base_addr = simple_strtoul(argv[1], NULL, 16);
	caddy_interface = (struct caddy_interface *) base_addr;

	memset((void *)caddy_interface, 0, sizeof(struct caddy_interface));
	memcpy((void *)&caddy_interface->magic[0], &CADDY_MAGIC, 16);

	while (ctrlc() == 0) {
		if (caddy_interface->cmd_in != caddy_interface->cmd_out) {
			memset(result, 0, 5 * sizeof(result[0]));
			status = 0;
			caddy_cmd = &caddy_interface->cmd[caddy_interface->cmd_out];
			pci_ptr = (void *)CONFIG_SYS_PCI1_IO_PHYS +
				(caddy_cmd->addr & 0x001fffff);

			switch (caddy_cmd->cmd) {
			case CADDY_CMD_IO_READ_8:
				result[0] = in_8(pci_ptr);
				break;

			case CADDY_CMD_IO_READ_16:
				result[0] = in_be16(pci_ptr);
				break;

			case CADDY_CMD_IO_READ_32:
				result[0] = in_be32(pci_ptr);
				break;

			case CADDY_CMD_IO_WRITE_8:
				data8 = caddy_cmd->par[0] & 0x000000ff;
				out_8(pci_ptr, data8);
				break;

			case CADDY_CMD_IO_WRITE_16:
				data16 = caddy_cmd->par[0] & 0x0000ffff;
				out_be16(pci_ptr, data16);
				break;

			case CADDY_CMD_IO_WRITE_32:
				out_be32(pci_ptr, caddy_cmd->par[0]);
				break;

			case CADDY_CMD_CONFIG_READ_8:
				dev = PCI_BDF(caddy_cmd->par[0],
					      caddy_cmd->par[1],
					      caddy_cmd->par[2]);
				status = pci_read_config_byte(dev,
							      caddy_cmd->addr,
							      &data8);
				result[0] = data8;
				break;

			case CADDY_CMD_CONFIG_READ_16:
				dev = PCI_BDF(caddy_cmd->par[0],
					      caddy_cmd->par[1],
					      caddy_cmd->par[2]);
				status = pci_read_config_word(dev,
							      caddy_cmd->addr,
							      &data16);
				result[0] = data16;
				break;

			case CADDY_CMD_CONFIG_READ_32:
				dev = PCI_BDF(caddy_cmd->par[0],
					      caddy_cmd->par[1],
					      caddy_cmd->par[2]);
				status = pci_read_config_dword(dev,
							       caddy_cmd->addr,
							       &result[0]);
				break;

			case CADDY_CMD_CONFIG_WRITE_8:
				dev = PCI_BDF(caddy_cmd->par[0],
					      caddy_cmd->par[1],
					      caddy_cmd->par[2]);
				data8 = caddy_cmd->par[3] & 0x000000ff;
				status = pci_write_config_byte(dev,
							       caddy_cmd->addr,
							       data8);
				break;

			case CADDY_CMD_CONFIG_WRITE_16:
				dev = PCI_BDF(caddy_cmd->par[0],
					      caddy_cmd->par[1],
					      caddy_cmd->par[2]);
				data16 = caddy_cmd->par[3] & 0x0000ffff;
				status = pci_write_config_word(dev,
							       caddy_cmd->addr,
							       data16);
				break;

			case CADDY_CMD_CONFIG_WRITE_32:
				dev = PCI_BDF(caddy_cmd->par[0],
					      caddy_cmd->par[1],
					      caddy_cmd->par[2]);
				status = pci_write_config_dword(dev,
								caddy_cmd->addr,
								caddy_cmd->par[3]);
				break;

			default:
				status = 0xffffffff;
				break;
			}

			generate_answer(caddy_cmd, status, &result[0]);

			ptr = caddy_interface->cmd_out + 1;
			ptr = ptr & (CMD_SIZE - 1);
			caddy_interface->cmd_out = ptr;
		}

		caddy_interface->heartbeat++;
	}

	return 0;
}

U_BOOT_CMD(
	caddy,	2,	0,	do_caddy,
	"Start Caddy server.",
	"Start Caddy server with Data structure a given addr\n"
	);
