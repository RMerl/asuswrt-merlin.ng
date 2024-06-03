// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <i2c.h>
#include <net.h>
#include <linux/mtd/st_smi.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/spr_emi.h>
#include <asm/arch/spr_defs.h>

#define CPU		0
#define DDR		1
#define SRAM_REL	0xD2801000

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_CMD_NET)
static int i2c_read_mac(uchar *buffer);
#endif

int dram_init(void)
{
	/* Store complete RAM size and return */
	gd->ram_size = get_ram_size(PHYS_SDRAM_1, PHYS_SDRAM_1_MAXSIZE);

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = gd->ram_size;

	return 0;
}

int board_early_init_f()
{
#if defined(CONFIG_ST_SMI)
	smi_init();
#endif
	return 0;
}
int misc_init_r(void)
{
#if defined(CONFIG_CMD_NET)
	uchar mac_id[6];

	if (!eth_env_get_enetaddr("ethaddr", mac_id) && !i2c_read_mac(mac_id))
		eth_env_set_enetaddr("ethaddr", mac_id);
#endif
	env_set("verify", "n");

#if defined(CONFIG_SPEAR_USBTTY)
	env_set("stdin", "usbtty");
	env_set("stdout", "usbtty");
	env_set("stderr", "usbtty");

#ifndef CONFIG_SYS_NO_DCACHE
	dcache_enable();
#endif
#endif
	return 0;
}

#ifdef CONFIG_SPEAR_EMI
struct cust_emi_para {
	unsigned int tap;
	unsigned int tsdp;
	unsigned int tdpw;
	unsigned int tdpr;
	unsigned int tdcs;
};

/* EMI timing setting of m28w640hc of linux kernel */
const struct cust_emi_para emi_timing_m28w640hc = {
	.tap = 0x10,
	.tsdp = 0x05,
	.tdpw = 0x0a,
	.tdpr = 0x0a,
	.tdcs = 0x05,
};

/* EMI timing setting of bootrom */
const struct cust_emi_para emi_timing_bootrom = {
	.tap = 0xf,
	.tsdp = 0x0,
	.tdpw = 0xff,
	.tdpr = 0x111,
	.tdcs = 0x02,
};

void spear_emi_init(void)
{
	const struct cust_emi_para *p = &emi_timing_m28w640hc;
	struct emi_regs *emi_regs_p = (struct emi_regs *)CONFIG_SPEAR_EMIBASE;
	unsigned int cs;
	unsigned int val, tmp;

	val = readl(CONFIG_SPEAR_RASBASE);

	if (val & EMI_ACKMSK)
		tmp = 0x3f;
	else
		tmp = 0x0;

	writel(tmp, &emi_regs_p->ack);

	for (cs = 0; cs < CONFIG_SYS_MAX_FLASH_BANKS; cs++) {
		writel(p->tap, &emi_regs_p->bank_regs[cs].tap);
		writel(p->tsdp, &emi_regs_p->bank_regs[cs].tsdp);
		writel(p->tdpw, &emi_regs_p->bank_regs[cs].tdpw);
		writel(p->tdpr, &emi_regs_p->bank_regs[cs].tdpr);
		writel(p->tdcs, &emi_regs_p->bank_regs[cs].tdcs);
		writel(EMI_CNTL_ENBBYTERW | ((val & 0x18) >> 3),
		       &emi_regs_p->bank_regs[cs].control);
	}
}
#endif

int spear_board_init(ulong mach_type)
{
	gd->bd->bi_arch_number = mach_type;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_BOOT_PARAMS_ADDR;

#ifdef CONFIG_SPEAR_EMI
	spear_emi_init();
#endif
	return 0;
}

#if defined(CONFIG_CMD_NET)
static int i2c_read_mac(uchar *buffer)
{
	u8 buf[2];

	i2c_read(CONFIG_I2C_CHIPADDRESS, MAGIC_OFF, 1, buf, MAGIC_LEN);

	/* Check if mac in i2c memory is valid */
	if ((buf[0] == MAGIC_BYTE0) && (buf[1] == MAGIC_BYTE1)) {
		/* Valid mac address is saved in i2c eeprom */
		i2c_read(CONFIG_I2C_CHIPADDRESS, MAC_OFF, 1, buffer, MAC_LEN);
		return 0;
	}

	return -1;
}

static int write_mac(uchar *mac)
{
	u8 buf[2];

	buf[0] = (u8)MAGIC_BYTE0;
	buf[1] = (u8)MAGIC_BYTE1;
	i2c_write(CONFIG_I2C_CHIPADDRESS, MAGIC_OFF, 1, buf, MAGIC_LEN);

	buf[0] = (u8)~MAGIC_BYTE0;
	buf[1] = (u8)~MAGIC_BYTE1;

	i2c_read(CONFIG_I2C_CHIPADDRESS, MAGIC_OFF, 1, buf, MAGIC_LEN);

	/* check if valid MAC address is saved in I2C EEPROM or not? */
	if ((buf[0] == MAGIC_BYTE0) && (buf[1] == MAGIC_BYTE1)) {
		i2c_write(CONFIG_I2C_CHIPADDRESS, MAC_OFF, 1, mac, MAC_LEN);
		puts("I2C EEPROM written with mac address \n");
		return 0;
	}

	puts("I2C EEPROM writing failed\n");
	return -1;
}
#endif

int do_chip_config(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	void (*sram_setfreq) (unsigned int, unsigned int);
	unsigned int frequency;
#if defined(CONFIG_CMD_NET)
	unsigned char mac[6];
#endif

	if ((argc > 3) || (argc < 2))
		return cmd_usage(cmdtp);

	if ((!strcmp(argv[1], "cpufreq")) || (!strcmp(argv[1], "ddrfreq"))) {

		frequency = simple_strtoul(argv[2], NULL, 0);

		if (frequency > 333) {
			printf("Frequency is limited to 333MHz\n");
			return 1;
		}

		sram_setfreq = memcpy((void *)SRAM_REL, setfreq, setfreq_sz);

		if (!strcmp(argv[1], "cpufreq")) {
			sram_setfreq(CPU, frequency);
			printf("CPU frequency changed to %u\n", frequency);
		} else {
			sram_setfreq(DDR, frequency);
			printf("DDR frequency changed to %u\n", frequency);
		}

		return 0;

#if defined(CONFIG_CMD_NET)
	} else if (!strcmp(argv[1], "ethaddr")) {

		u32 reg;
		char *e, *s = argv[2];
		for (reg = 0; reg < 6; ++reg) {
			mac[reg] = s ? simple_strtoul(s, &e, 16) : 0;
			if (s)
				s = (*e) ? e + 1 : e;
		}
		write_mac(mac);

		return 0;
#endif
	} else if (!strcmp(argv[1], "print")) {
#if defined(CONFIG_CMD_NET)
		if (!i2c_read_mac(mac)) {
			printf("Ethaddr (from i2c mem) = %pM\n", mac);
		} else {
			printf("Ethaddr (from i2c mem) = Not set\n");
		}
#endif
		return 0;
	}

	return cmd_usage(cmdtp);
}

U_BOOT_CMD(chip_config, 3, 1, do_chip_config,
	   "configure chip",
	   "chip_config cpufreq/ddrfreq frequency\n"
#if defined(CONFIG_CMD_NET)
	   "chip_config ethaddr XX:XX:XX:XX:XX:XX\n"
#endif
	   "chip_config print");
