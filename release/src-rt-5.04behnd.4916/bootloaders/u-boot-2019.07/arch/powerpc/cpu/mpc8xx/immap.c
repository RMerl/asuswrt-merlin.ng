// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * MPC8xx Internal Memory Map Functions
 */

#include <common.h>
#include <command.h>

#include <asm/immap_8xx.h>
#include <asm/cpm_8xx.h>
#include <asm/iopin_8xx.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

static int do_siuinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;
	sysconf8xx_t __iomem *sc = &immap->im_siu_conf;

	printf("SIUMCR= %08x SYPCR = %08x\n",
	       in_be32(&sc->sc_siumcr), in_be32(&sc->sc_sypcr));
	printf("SWT   = %08x\n", in_be32(&sc->sc_swt));
	printf("SIPEND= %08x SIMASK= %08x\n",
	       in_be32(&sc->sc_sipend), in_be32(&sc->sc_simask));
	printf("SIEL  = %08x SIVEC = %08x\n",
	       in_be32(&sc->sc_siel), in_be32(&sc->sc_sivec));
	printf("TESR  = %08x SDCR  = %08x\n",
	       in_be32(&sc->sc_tesr), in_be32(&sc->sc_sdcr));
	return 0;
}

static int do_memcinfo(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;
	memctl8xx_t __iomem *memctl = &immap->im_memctl;
	int nbanks = 8;
	uint __iomem *p = &memctl->memc_br0;
	int i;

	for (i = 0; i < nbanks; i++, p += 2)
		printf("BR%-2d  = %08x OR%-2d  = %08x\n",
		       i, in_be32(p), i, in_be32(p + 1));

	printf("MAR   = %08x", in_be32(&memctl->memc_mar));
	printf(" MCR   = %08x\n", in_be32(&memctl->memc_mcr));
	printf("MAMR  = %08x MBMR  = %08x",
	       in_be32(&memctl->memc_mamr), in_be32(&memctl->memc_mbmr));
	printf("\nMSTAT =     %04x\n", in_be16(&memctl->memc_mstat));
	printf("MPTPR =     %04x MDR   = %08x\n",
	       in_be16(&memctl->memc_mptpr), in_be32(&memctl->memc_mdr));
	return 0;
}

static int do_carinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;
	car8xx_t __iomem *car = &immap->im_clkrst;

	printf("SCCR  = %08x\n", in_be32(&car->car_sccr));
	printf("PLPRCR= %08x\n", in_be32(&car->car_plprcr));
	printf("RSR   = %08x\n", in_be32(&car->car_rsr));
	return 0;
}

static int counter;

static void header(void)
{
	char *data = "\
       --------------------------------        --------------------------------\
       00000000001111111111222222222233        00000000001111111111222222222233\
       01234567890123456789012345678901        01234567890123456789012345678901\
       --------------------------------        --------------------------------\
    ";
	int i;

	if (counter % 2)
		putc('\n');
	counter = 0;

	for (i = 0; i < 4; i++, data += 79)
		printf("%.79s\n", data);
}

static void binary(char *label, uint value, int nbits)
{
	uint mask = 1 << (nbits - 1);
	int i, second = (counter++ % 2);

	if (second)
		putc(' ');
	puts(label);
	for (i = 32 + 1; i != nbits; i--)
		putc(' ');

	while (mask != 0) {
		if (value & mask)
			putc('1');
		else
			putc('0');
		mask >>= 1;
	}

	if (second)
		putc('\n');
}

#define PA_NBITS	16
#define PA_NB_ODR	 8
#define PB_NBITS	18
#define PB_NB_ODR	16
#define PC_NBITS	12
#define PD_NBITS	13

static int do_iopinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;
	iop8xx_t __iomem *iop = &immap->im_ioport;
	ushort __iomem *l, *r;
	uint __iomem *R;

	counter = 0;
	header();

	/*
	 * Ports A & B
	 */

	l = &iop->iop_padir;
	R = &immap->im_cpm.cp_pbdir;
	binary("PA_DIR", in_be16(l++), PA_NBITS);
	binary("PB_DIR", in_be32(R++), PB_NBITS);
	binary("PA_PAR", in_be16(l++), PA_NBITS);
	binary("PB_PAR", in_be32(R++), PB_NBITS);
	binary("PA_ODR", in_be16(l++), PA_NB_ODR);
	binary("PB_ODR", in_be32(R++), PB_NB_ODR);
	binary("PA_DAT", in_be16(l++), PA_NBITS);
	binary("PB_DAT", in_be32(R++), PB_NBITS);

	header();

	/*
	 * Ports C & D
	 */

	l = &iop->iop_pcdir;
	r = &iop->iop_pddir;
	binary("PC_DIR", in_be16(l++), PC_NBITS);
	binary("PD_DIR", in_be16(r++), PD_NBITS);
	binary("PC_PAR", in_be16(l++), PC_NBITS);
	binary("PD_PAR", in_be16(r++), PD_NBITS);
	binary("PC_SO ", in_be16(l++), PC_NBITS);
	binary("      ", 0, 0);
	r++;
	binary("PC_DAT", in_be16(l++), PC_NBITS);
	binary("PD_DAT", in_be16(r++), PD_NBITS);
	binary("PC_INT", in_be16(l++), PC_NBITS);

	header();
	return 0;
}

/*
 * set the io pins
 * this needs a clean up for smaller tighter code
 * use *uint and set the address based on cmd + port
 */
static int do_iopset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	uint rcode = 0;
	iopin_t iopin;
	static uint port;
	static uint pin;
	static uint value;
	static enum {
		DIR,
		PAR,
		SOR,
		ODR,
		DAT,
		INT
	} cmd = DAT;

	if (argc != 5) {
		puts("iopset PORT PIN CMD VALUE\n");
		return 1;
	}
	port = argv[1][0] - 'A';
	if (port > 3)
		port -= 0x20;
	if (port > 3)
		rcode = 1;
	pin = simple_strtol(argv[2], NULL, 10);
	if (pin > 31)
		rcode = 1;


	switch (argv[3][0]) {
	case 'd':
		if (argv[3][1] == 'a')
			cmd = DAT;
		else if (argv[3][1] == 'i')
			cmd = DIR;
		else
			rcode = 1;
		break;
	case 'p':
		cmd = PAR;
		break;
	case 'o':
		cmd = ODR;
		break;
	case 's':
		cmd = SOR;
		break;
	case 'i':
		cmd = INT;
		break;
	default:
		printf("iopset: unknown command %s\n", argv[3]);
		rcode = 1;
	}
	if (argv[4][0] == '1')
		value = 1;
	else if (argv[4][0] == '0')
		value = 0;
	else
		rcode = 1;
	if (rcode == 0) {
		iopin.port = port;
		iopin.pin = pin;
		iopin.flag = 0;
		switch (cmd) {
		case DIR:
			if (value)
				iopin_set_out(&iopin);
			else
				iopin_set_in(&iopin);
			break;
		case PAR:
			if (value)
				iopin_set_ded(&iopin);
			else
				iopin_set_gen(&iopin);
			break;
		case SOR:
			if (value)
				iopin_set_opt2(&iopin);
			else
				iopin_set_opt1(&iopin);
			break;
		case ODR:
			if (value)
				iopin_set_odr(&iopin);
			else
				iopin_set_act(&iopin);
			break;
		case DAT:
			if (value)
				iopin_set_high(&iopin);
			else
				iopin_set_low(&iopin);
			break;
		case INT:
			if (value)
				iopin_set_falledge(&iopin);
			else
				iopin_set_anyedge(&iopin);
			break;
		}
	}
	return rcode;
}

static void prbrg(int n, uint val)
{
	uint extc = (val >> 14) & 3;
	uint cd = (val & CPM_BRG_CD_MASK) >> 1;
	uint div16 = (val & CPM_BRG_DIV16) != 0;

	ulong clock = gd->cpu_clk;

	printf("BRG%d:", n);

	if (val & CPM_BRG_RST)
		puts(" RESET");
	else
		puts("      ");

	if (val & CPM_BRG_EN)
		puts("  ENABLED");
	else
		puts(" DISABLED");

	printf(" EXTC=%d", extc);

	if (val & CPM_BRG_ATB)
		puts(" ATB");
	else
		puts("    ");

	printf(" DIVIDER=%4d", cd);
	if (extc == 0 && cd != 0) {
		uint baudrate;

		if (div16)
			baudrate = (clock / 16) / (cd + 1);
		else
			baudrate = clock / (cd + 1);

		printf("=%6d bps", baudrate);
	} else {
		puts("           ");
	}

	if (val & CPM_BRG_DIV16)
		puts(" DIV16");
	else
		puts("      ");

	putc('\n');
}

static int do_brginfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;
	cpm8xx_t __iomem *cp = &immap->im_cpm;
	uint __iomem *p = &cp->cp_brgc1;
	int i = 1;

	while (i <= 4)
		prbrg(i++, in_be32(p++));

	return 0;
}

#ifdef CONFIG_CMD_REGINFO
void print_reginfo(void)
{
	immap_t __iomem     *immap  = (immap_t __iomem *)CONFIG_SYS_IMMR;
	sit8xx_t __iomem *timers = &immap->im_sit;

	printf("\nSystem Configuration registers\n"
		"\tIMMR\t0x%08X\n", get_immr());
	do_siuinfo(NULL, 0, 0, NULL);

	printf("Memory Controller Registers\n");
	do_memcinfo(NULL, 0, 0, NULL);

	printf("\nSystem Integration Timers\n");
	printf("\tTBSCR\t0x%04X\tRTCSC\t0x%04X\n",
	       in_be16(&timers->sit_tbscr), in_be16(&timers->sit_rtcsc));
	printf("\tPISCR\t0x%04X\n", in_be16(&timers->sit_piscr));
}
#endif

/***************************************************/

U_BOOT_CMD(
	siuinfo,	1,	1,	do_siuinfo,
	"print System Interface Unit (SIU) registers",
	""
);

U_BOOT_CMD(
	memcinfo,	1,	1,	do_memcinfo,
	"print Memory Controller registers",
	""
);

U_BOOT_CMD(
	carinfo,	1,	1,	do_carinfo,
	"print Clocks and Reset registers",
	""
);

U_BOOT_CMD(
	iopinfo,	1,	1,	do_iopinfo,
	"print I/O Port registers",
	""
);

U_BOOT_CMD(
	iopset,	5,	0,	do_iopset,
	"set I/O Port registers",
	"PORT PIN CMD VALUE\nPORT: A-D, PIN: 0-31, CMD: [dat|dir|odr|sor], VALUE: 0|1"
);

U_BOOT_CMD(
	brginfo,	1,	1,	do_brginfo,
	"print Baud Rate Generator (BRG) registers",
	""
);
