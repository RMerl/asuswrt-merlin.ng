/* Copyright (c) 2016 FUJITSU LIMITED */
#include <stdio.h>
#include "internal.h"

int fjes_dump_regs(struct ethtool_drvinfo *info __maybe_unused,
		   struct ethtool_regs *regs)
{
	u32 *regs_buff = (u32 *)regs->data;

	if (regs->version != 1)
		return -1;

	/* Information registers */
	fprintf(stdout,
		"0x0000: OWNER_EPID    (Owner EPID)                       0x%08X\n",
		regs_buff[0]);

	fprintf(stdout,
		"0x0004: MAX_EP        (Maximum EP)                       0x%08X\n",
		regs_buff[1]);

	/* Device Control registers */
	fprintf(stdout,
		"0x0010: DCTL          (Device Control)                   0x%08X\n",
		regs_buff[4]);

	/* Command Control registers */
	fprintf(stdout,
		"0x0020: CR            (Command request)                  0x%08X\n",
		regs_buff[8]);

	fprintf(stdout,
		"0x0024: CS            (Command status)                   0x%08X\n",
		regs_buff[9]);

	fprintf(stdout,
		"0x0028: SHSTSAL       (Share status address Low)         0x%08X\n",
		regs_buff[10]);

	fprintf(stdout,
		"0x002C: SHSTSAH       (Share status address High)        0x%08X\n",
		regs_buff[11]);

	fprintf(stdout,
		"0x0034: REQBL         (Request Buffer length)            0x%08X\n",
		regs_buff[13]);

	fprintf(stdout,
		"0x0038: REQBAL        (Request Buffer Address Low)       0x%08X\n",
		regs_buff[14]);

	fprintf(stdout,
		"0x003C: REQBAH        (Request Buffer Address High)      0x%08X\n",
		regs_buff[15]);

	fprintf(stdout,
		"0x0044: RESPBL        (Response Buffer Length)           0x%08X\n",
		regs_buff[17]);

	fprintf(stdout,
		"0x0048: RESPBAL       (Response Buffer Address Low)      0x%08X\n",
		regs_buff[18]);

	fprintf(stdout,
		"0x004C: RESPBAH       (Response Buffer Address High)     0x%08X\n",
		regs_buff[19]);

	/* Interrupt Control registers */
	fprintf(stdout,
		"0x0080: IS            (Interrupt status)                 0x%08X\n",
		regs_buff[32]);

	fprintf(stdout,
		"0x0084: IMS           (Interrupt mask set)               0x%08X\n",
		regs_buff[33]);

	fprintf(stdout,
		"0x0088: IMC           (Interrupt mask clear)             0x%08X\n",
		regs_buff[34]);

	fprintf(stdout,
		"0x008C: IG            (Interrupt generator)              0x%08X\n",
		regs_buff[35]);

	fprintf(stdout,
		"0x0090: ICTL          (Interrupt control)                0x%08X\n",
		regs_buff[36]);

	return 0;
}
