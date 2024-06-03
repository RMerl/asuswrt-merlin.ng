// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2005-2009
 * BuS Elektronik GmbH & Co.KG <esw@bus-elektonik.de>
 *
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <command.h>
#include "asm/m5282.h"
#include <bmp_layout.h>
#include <status_led.h>
#include <bus_vcxk.h>

/*---------------------------------------------------------------------------*/

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_VIDEO
unsigned long display_width;
unsigned long display_height;
#endif

/*---------------------------------------------------------------------------*/

int checkboard (void)
{
	puts("Board: EB+CPU5282 (BuS Elektronik GmbH & Co. KG)\n");
#if (CONFIG_SYS_TEXT_BASE ==  CONFIG_SYS_INT_FLASH_BASE)
	puts("       Boot from Internal FLASH\n");
#endif
	return 0;
}

int dram_init(void)
{
	int size, i;

	size = 0;
	MCFSDRAMC_DCR = MCFSDRAMC_DCR_RTIM_6 |
			MCFSDRAMC_DCR_RC((15 * CONFIG_SYS_CLK / 1000000) >> 4);
	asm (" nop");
#ifdef CONFIG_SYS_SDRAM_BASE0
	MCFSDRAMC_DACR0 = MCFSDRAMC_DACR_BASE(CONFIG_SYS_SDRAM_BASE0)|
		MCFSDRAMC_DACR_CASL(1) | MCFSDRAMC_DACR_CBM(3) |
		MCFSDRAMC_DACR_PS_32;
	asm (" nop");

	MCFSDRAMC_DMR0 = MCFSDRAMC_DMR_BAM_16M | MCFSDRAMC_DMR_V;
	asm (" nop");

	MCFSDRAMC_DACR0 |= MCFSDRAMC_DACR_IP;
	asm (" nop");
	for (i = 0; i < 10; i++)
		asm (" nop");

	*(unsigned long *)(CONFIG_SYS_SDRAM_BASE0) = 0xA5A5A5A5;
	asm (" nop");
	MCFSDRAMC_DACR0 |= MCFSDRAMC_DACR_RE;
	asm (" nop");

	for (i = 0; i < 2000; i++)
		asm (" nop");

	MCFSDRAMC_DACR0 |= MCFSDRAMC_DACR_IMRS;
	asm (" nop");
	/* write SDRAM mode register */
	*(unsigned long *)(CONFIG_SYS_SDRAM_BASE0 + 0x80440) = 0xA5A5A5A5;
	asm (" nop");
	size += CONFIG_SYS_SDRAM_SIZE0 * 1024 * 1024;
#endif
#ifdef CONFIG_SYS_SDRAM_BASE1xx
	MCFSDRAMC_DACR1 = MCFSDRAMC_DACR_BASE (CONFIG_SYS_SDRAM_BASE1)
			| MCFSDRAMC_DACR_CASL (1)
			| MCFSDRAMC_DACR_CBM (3)
			| MCFSDRAMC_DACR_PS_16;

	MCFSDRAMC_DMR1 = MCFSDRAMC_DMR_BAM_16M | MCFSDRAMC_DMR_V;

	MCFSDRAMC_DACR1 |= MCFSDRAMC_DACR_IP;

	*(unsigned short *) (CONFIG_SYS_SDRAM_BASE1) = 0xA5A5;
	MCFSDRAMC_DACR1 |= MCFSDRAMC_DACR_RE;

	for (i = 0; i < 2000; i++)
		asm (" nop");

	MCFSDRAMC_DACR1 |= MCFSDRAMC_DACR_IMRS;
	*(unsigned int *) (CONFIG_SYS_SDRAM_BASE1 + 0x220) = 0xA5A5;
	size += CONFIG_SYS_SDRAM_SIZE1 * 1024 * 1024;
#endif
	gd->ram_size = size;

	return 0;
}

#if defined(CONFIG_SYS_DRAM_TEST)
int testdram (void)
{
	uint *pstart = (uint *) CONFIG_SYS_MEMTEST_START;
	uint *pend = (uint *) CONFIG_SYS_MEMTEST_END;
	uint *p;

	printf("SDRAM test phase 1:\n");
	for (p = pstart; p < pend; p++)
		*p = 0xaaaaaaaa;

	for (p = pstart; p < pend; p++) {
		if (*p != 0xaaaaaaaa) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf("SDRAM test phase 2:\n");
	for (p = pstart; p < pend; p++)
		*p = 0x55555555;

	for (p = pstart; p < pend; p++) {
		if (*p != 0x55555555) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf("SDRAM test passed.\n");
	return 0;
}
#endif

#if defined(CONFIG_HW_WATCHDOG)

void hw_watchdog_init(void)
{
	char *s;
	int enable;

	enable = 1;
	s = env_get("watchdog");
	if (s != NULL)
		if ((strncmp(s, "off", 3) == 0) || (strncmp(s, "0", 1) == 0))
			enable = 0;
	if (enable)
		MCFGPTA_GPTDDR  |= (1<<2);
	else
		MCFGPTA_GPTDDR  &= ~(1<<2);
}

void hw_watchdog_reset(void)
{
	MCFGPTA_GPTPORT  ^= (1<<2);
}
#endif

int misc_init_r(void)
{
#ifdef	CONFIG_HW_WATCHDOG
	hw_watchdog_init();
#endif
	return 1;
}

void __led_toggle(led_id_t mask)
{
	MCFGPTA_GPTPORT ^= (1 << 3);
}

void __led_init(led_id_t mask, int state)
{
	__led_set(mask, state);
	MCFGPTA_GPTDDR  |= (1 << 3);
}

void __led_set(led_id_t mask, int state)
{
	if (state == CONFIG_LED_STATUS_ON)
		MCFGPTA_GPTPORT |= (1 << 3);
	else
		MCFGPTA_GPTPORT &= ~(1 << 3);
}

#if defined(CONFIG_VIDEO)

int drv_video_init(void)
{
	char *s;
#ifdef CONFIG_SPLASH_SCREEN
	unsigned long splash;
#endif
	printf("Init Video as ");
	s = env_get("displaywidth");
	if (s != NULL)
		display_width = simple_strtoul(s, NULL, 10);
	else
		display_width = 256;

	s = env_get("displayheight");
	if (s != NULL)
		display_height = simple_strtoul(s, NULL, 10);
	else
		display_height = 256;

	printf("%lu x %lu pixel matrix\n", display_width, display_height);

	MCFCCM_CCR &= ~MCFCCM_CCR_SZEN;
	MCFGPIO_PEPAR &= ~MCFGPIO_PEPAR_PEPA2;

	vcxk_init(display_width, display_height);

#ifdef CONFIG_SPLASH_SCREEN
	s = env_get("splashimage");
	if (s != NULL) {
		splash = simple_strtoul(s, NULL, 16);
		vcxk_acknowledge_wait();
		video_display_bitmap(splash, 0, 0);
	}
#endif
	return 0;
}
#endif

/*---------------------------------------------------------------------------*/

#ifdef CONFIG_VIDEO
int do_brightness(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int rcode = 0;
	ulong side;
	ulong bright;

	switch (argc) {
	case 3:
		side = simple_strtoul(argv[1], NULL, 10);
		bright = simple_strtoul(argv[2], NULL, 10);
		if ((side >= 0) && (side <= 3) &&
			(bright >= 0) && (bright <= 1000)) {
			vcxk_setbrightness(side, bright);
			rcode = 0;
		} else {
			printf("parameters out of range\n");
			printf("Usage:\n%s\n", cmdtp->usage);
			rcode = 1;
		}
		break;
	default:
		printf("Usage:\n%s\n", cmdtp->usage);
		rcode = 1;
		break;
	}
	return rcode;
}

/*---------------------------------------------------------------------------*/

U_BOOT_CMD(
	bright,	3,	0,	do_brightness,
	"sets the display brightness\n",
	" <side> <0..1000>\n        side: 0/3=both; 1=first; 2=second\n"
);

#endif

/* EOF EB+MCF-EV123.c */
