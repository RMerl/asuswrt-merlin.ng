// SPDX-License-Identifier: GPL-2.0+
/*
 * Marubun MR-SHPC-01 PCMCIA controller device driver
 *
 * (c) 2007 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 */

#include <common.h>
#include <config.h>
#include <pcmcia.h>
#include <asm/io.h>

#undef CONFIG_PCMCIA

#if defined(CONFIG_CMD_PCMCIA)
#define	CONFIG_PCMCIA
#endif

#if defined(CONFIG_IDE)
#define	CONFIG_PCMCIA
#endif

#if defined(CONFIG_PCMCIA)

/* MR-SHPC-01 register */
#define MRSHPC_MODE	(CONFIG_SYS_MARUBUN_MRSHPC + 4)
#define MRSHPC_OPTION   (CONFIG_SYS_MARUBUN_MRSHPC + 6)
#define MRSHPC_CSR      (CONFIG_SYS_MARUBUN_MRSHPC + 8)
#define MRSHPC_ISR      (CONFIG_SYS_MARUBUN_MRSHPC + 10)
#define MRSHPC_ICR      (CONFIG_SYS_MARUBUN_MRSHPC + 12)
#define MRSHPC_CPWCR    (CONFIG_SYS_MARUBUN_MRSHPC + 14)
#define MRSHPC_MW0CR1   (CONFIG_SYS_MARUBUN_MRSHPC + 16)
#define MRSHPC_MW1CR1   (CONFIG_SYS_MARUBUN_MRSHPC + 18)
#define MRSHPC_IOWCR1   (CONFIG_SYS_MARUBUN_MRSHPC + 20)
#define MRSHPC_MW0CR2   (CONFIG_SYS_MARUBUN_MRSHPC + 22)
#define MRSHPC_MW1CR2   (CONFIG_SYS_MARUBUN_MRSHPC + 24)
#define MRSHPC_IOWCR2   (CONFIG_SYS_MARUBUN_MRSHPC + 26)
#define MRSHPC_CDCR     (CONFIG_SYS_MARUBUN_MRSHPC + 28)
#define MRSHPC_PCIC_INFO (CONFIG_SYS_MARUBUN_MRSHPC + 30)

int pcmcia_on (void)
{
	printf("Enable PCMCIA " PCMCIA_SLOT_MSG "\n");

	/* Init */
	outw( 0x0000 , MRSHPC_MODE );

	if ((inw(MRSHPC_CSR) & 0x000c) == 0){	/* if card detect is true */
		if ((inw(MRSHPC_CSR) & 0x0080) == 0){
			outw(0x0674 ,MRSHPC_CPWCR);  /* Card Vcc is 3.3v? */
		}else{
			outw(0x0678 ,MRSHPC_CPWCR);  /* Card Vcc is 5V */
		}
		udelay( 100000 );   /* wait for power on */
	}else{
		return 1;
	}
	/*
	 *	PC-Card window open
	 *	flag == COMMON/ATTRIBUTE/IO
	 */
	/* common window open */
	outw(0x8a84,MRSHPC_MW0CR1); /* window 0xb8400000 */
	if ((inw(MRSHPC_CSR) & 0x4000) != 0)
		outw(0x0b00,MRSHPC_MW0CR2); /* common mode & bus width 16bit SWAP = 1 */
	else
		outw(0x0300,MRSHPC_MW0CR2); /* common mode & bus width 16bit SWAP = 0 */

	/* attribute window open */
	outw(0x8a85,MRSHPC_MW1CR1); /* window 0xb8500000 */
	if ((inw(MRSHPC_CSR) & 0x4000) != 0)
		outw(0x0a00,MRSHPC_MW1CR2); /* attribute mode & bus width 16bit SWAP = 1 */
	else
		outw(0x0200,MRSHPC_MW1CR2); /* attribute mode & bus width 16bit SWAP = 0 */

	/* I/O window open */
	outw(0x8a86,MRSHPC_IOWCR1); /* I/O window 0xb8600000 */
	outw(0x0008,MRSHPC_CDCR);   /* I/O card mode */
	if ((inw(MRSHPC_CSR) & 0x4000) != 0)
		outw(0x0a00,MRSHPC_IOWCR2); /* bus width 16bit SWAP = 1 */
	else
		outw(0x0200,MRSHPC_IOWCR2); /* bus width 16bit SWAP = 0 */

	outw(0x0000,MRSHPC_ISR);
	outw(0x2000,MRSHPC_ICR);
	outb(0x00,(CONFIG_SYS_MARUBUN_MW2 + 0x206));
	outb(0x42,(CONFIG_SYS_MARUBUN_MW2 + 0x200));

	return 0;
}

int pcmcia_off (void)
{
	printf ("Disable PCMCIA " PCMCIA_SLOT_MSG "\n");

	return 0;
}

#endif /* CONFIG_PCMCIA */
