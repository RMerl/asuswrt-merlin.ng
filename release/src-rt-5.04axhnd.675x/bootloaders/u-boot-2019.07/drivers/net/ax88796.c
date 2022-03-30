// SPDX-License-Identifier: GPL-2.0+
/*
 * (c) 2007 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 */
#include <common.h>
#include "ax88796.h"

/*
 * Set 1 bit data
 */
static void ax88796_bitset(u32 bit)
{
	/* DATA1 */
	if( bit )
		EEDI_HIGH;
	else
		EEDI_LOW;

	EECLK_LOW;
	udelay(1000);
	EECLK_HIGH;
	udelay(1000);
	EEDI_LOW;
}

/*
 * Get 1 bit data
 */
static u8 ax88796_bitget(void)
{
	u8 bit;

	EECLK_LOW;
	udelay(1000);
	/* DATA */
	bit = EEDO;
	EECLK_HIGH;
	udelay(1000);

	return bit;
}

/*
 * Send COMMAND to EEPROM
 */
static void ax88796_eep_cmd(u8 cmd)
{
	ax88796_bitset(BIT_DUMMY);
	switch(cmd){
		case MAC_EEP_READ:
			ax88796_bitset(1);
			ax88796_bitset(1);
			ax88796_bitset(0);
			break;

		case MAC_EEP_WRITE:
			ax88796_bitset(1);
			ax88796_bitset(0);
			ax88796_bitset(1);
			break;

		case MAC_EEP_ERACE:
			ax88796_bitset(1);
			ax88796_bitset(1);
			ax88796_bitset(1);
			break;

		case MAC_EEP_EWEN:
			ax88796_bitset(1);
			ax88796_bitset(0);
			ax88796_bitset(0);
			break;

		case MAC_EEP_EWDS:
			ax88796_bitset(1);
			ax88796_bitset(0);
			ax88796_bitset(0);
			break;
		default:
			break;
	}
}

static void ax88796_eep_setaddr(u16 addr)
{
	int i ;

	for( i = 7 ; i >= 0 ; i-- )
		ax88796_bitset(addr & (1 << i));
}

/*
 * Get data from EEPROM
 */
static u16 ax88796_eep_getdata(void)
{
	ushort data = 0;
	int i;

	ax88796_bitget();	/* DUMMY */
	for( i = 0 ; i < 16 ; i++ ){
		data <<= 1;
		data |= ax88796_bitget();
	}
	return data;
}

static void ax88796_mac_read(u8 *buff)
{
	int i ;
	u16 data;
	u16 addr = 0;

	for( i = 0 ; i < 3; i++ )
	{
		EECS_HIGH;
		EEDI_LOW;
		udelay(1000);
		/* READ COMMAND */
		ax88796_eep_cmd(MAC_EEP_READ);
		/* ADDRESS */
		ax88796_eep_setaddr(addr++);
		/* GET DATA */
		data = ax88796_eep_getdata();
		*buff++ = (uchar)(data & 0xff);
		*buff++ = (uchar)((data >> 8) & 0xff);
		EECLK_LOW;
		EEDI_LOW;
		EECS_LOW;
	}
}

int get_prom(u8* mac_addr, u8* base_addr)
{
	u8 prom[32];
	int i;

	ax88796_mac_read(prom);
	for (i = 0; i < 6; i++){
		mac_addr[i] = prom[i];
	}
	return 1;
}
