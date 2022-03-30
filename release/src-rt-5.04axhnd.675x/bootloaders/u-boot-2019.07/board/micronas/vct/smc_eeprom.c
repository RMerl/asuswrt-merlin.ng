/*
 * (C) Copyright 2008 Stefan Roese <sr@denx.de>, DENX Software Engineering
 *
 * Copyright 2005, Seagate Technology LLC
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#undef DEBUG

#include <common.h>
#include <command.h>
#include <config.h>
#include <net.h>

#include "vct.h"

#define SMSC9118_BASE		CONFIG_DRIVER_SMC911X_BASE
#define BYTE_TEST		(SMSC9118_BASE + 0x64)
#define GPIO_CFG		(SMSC9118_BASE + 0x88)
#define MAC_CSR_CMD		(SMSC9118_BASE + 0xA4)
#define  MAC_CSR_CMD_CSR_BUSY	(0x80000000)
#define  MAC_CSR_CMD_RNW	(0x40000000)
#define  MAC_RD_CMD(reg)	((reg & 0x000000FF) |			\
				 (MAC_CSR_CMD_CSR_BUSY | MAC_CSR_CMD_RNW))
#define  MAC_WR_CMD(reg)	((reg & 0x000000FF) |		\
				 (MAC_CSR_CMD_CSR_BUSY))
#define MAC_CSR_DATA		(SMSC9118_BASE + 0xA8)
#define E2P_CMD			(SMSC9118_BASE + 0xB0)
#define  E2P_CMD_EPC_BUSY_	(0x80000000UL)	/* Self Clearing */
#define  E2P_CMD_EPC_CMD_	(0x70000000UL)	/* R/W */
#define  E2P_CMD_EPC_CMD_READ_	(0x00000000UL)	/* R/W */
#define  E2P_CMD_EPC_CMD_EWDS_	(0x10000000UL)	/* R/W */
#define  E2P_CMD_EPC_CMD_EWEN_	(0x20000000UL)	/* R/W */
#define  E2P_CMD_EPC_CMD_WRITE_	(0x30000000UL)	/* R/W */
#define  E2P_CMD_EPC_CMD_WRAL_	(0x40000000UL)	/* R/W */
#define  E2P_CMD_EPC_CMD_ERASE_	(0x50000000UL)	/* R/W */
#define  E2P_CMD_EPC_CMD_ERAL_	(0x60000000UL)	/* R/W */
#define  E2P_CMD_EPC_CMD_RELOAD_ (0x70000000UL)	/* R/W */
#define  E2P_CMD_EPC_TIMEOUT_	(0x00000200UL)	/* R */
#define  E2P_CMD_MAC_ADDR_LOADED_ (0x00000100UL) /* RO */
#define  E2P_CMD_EPC_ADDR_	(0x000000FFUL)	/* R/W */
#define E2P_DATA		(SMSC9118_BASE + 0xB4)

#define MAC_ADDRH		(0x2)
#define MAC_ADDRL		(0x3)

#define MAC_TIMEOUT		200

#define HIBYTE(word)		((u8)(((u16)(word)) >> 8))
#define LOBYTE(word)		((u8)(((u16)(word)) & 0x00FFU))
#define HIWORD(dword)		((u16)(((u32)(dword)) >> 16))
#define LOWORD(dword)		((u16)(((u32)(dword)) & 0x0000FFFFUL))

static int mac_busy(int req_to)
{
	int timeout = req_to;

	while (timeout--) {
		if (!(smc911x_reg_read(MAC_CSR_CMD) & MAC_CSR_CMD_CSR_BUSY))
			goto done;
	}
	return 1;		/* Timeout */

done:
	return 0;		/* No timeout */
}

static ulong get_mac_reg(int reg)
{
	ulong reg_val = 0xffffffff;

	if (smc911x_reg_read(MAC_CSR_CMD) & MAC_CSR_CMD_CSR_BUSY) {
		printf("get_mac_reg: previous command not complete\n");
		goto done;
	}

	smc911x_reg_write(MAC_CSR_CMD, MAC_RD_CMD(reg));
	udelay(10000);

	if (mac_busy(MAC_TIMEOUT) == 1) {
		printf("get_mac_reg: timeout waiting for response from MAC\n");
		goto done;
	}

	reg_val = smc911x_reg_read(MAC_CSR_DATA);

done:
	return (reg_val);
}

static ulong eeprom_enable_access(void)
{
	ulong gpio;

	gpio = smc911x_reg_read(GPIO_CFG);
	debug("%s: gpio= 0x%08lx ---> 0x%08lx\n", __func__, gpio,
	      (gpio & 0xFF0FFFFFUL));

	smc911x_reg_write(GPIO_CFG, (gpio & 0xFF0FFFFFUL));
	return gpio;
}

static void eeprom_disable_access(ulong gpio)
{
	debug("%s: gpio= 0x%08lx\n", __func__, gpio);
	smc911x_reg_write(GPIO_CFG, gpio);
}

static int eeprom_is_mac_address_loaded(void)
{
	int ret;

	ret = smc911x_reg_read(MAC_CSR_CMD) & E2P_CMD_MAC_ADDR_LOADED_;
	debug("%s: ret = %x\n", __func__, ret);

	return ret;
}

static int eeprom_read_location(unchar address, u8 *data)
{
	ulong timeout = 100000;
	ulong temp = 0;

	if ((temp = smc911x_reg_read(E2P_CMD)) & E2P_CMD_EPC_BUSY_) {
		printf("%s: Busy at start, E2P_CMD=0x%08lX\n", __func__, temp);
		return 0;
	}

	smc911x_reg_write(E2P_CMD,
			  (E2P_CMD_EPC_BUSY_ | E2P_CMD_EPC_CMD_READ_ |
			   ((ulong) address)));

	while ((timeout > 0) && (smc911x_reg_read(E2P_CMD) & E2P_CMD_EPC_BUSY_)) {
		udelay(10);
		timeout--;
	}

	if (timeout == 0) {
		printf("Timeout\n");
		return 0;
	}
	(*data) = (unchar) (smc911x_reg_read(E2P_DATA));
	debug("%s: ret = %x\n", __func__, (*data));

	return 1;
}

static int eeprom_enable_erase_and_write(void)
{
	ulong timeout = 100000;

	if (smc911x_reg_read(E2P_CMD) & E2P_CMD_EPC_BUSY_) {
		printf("%s: Busy at start\n", __func__);
		return 0;
	}
	smc911x_reg_write(E2P_CMD, (E2P_CMD_EPC_BUSY_ | E2P_CMD_EPC_CMD_EWEN_));

	while ((timeout > 0) && (smc911x_reg_read(E2P_CMD) & E2P_CMD_EPC_BUSY_)) {
		udelay(10);
		timeout--;
	}

	if (timeout == 0) {
		printf("Timeout[1]\n");
		return 0;
	}

	return 1;
}

static int eeprom_disable_erase_and_write(void)
{
	ulong timeout = 100000;

	if (smc911x_reg_read(E2P_CMD) & E2P_CMD_EPC_BUSY_) {
		printf("%s: Busy at start\n", __func__);
		return 0;
	}
	smc911x_reg_write(E2P_CMD, (E2P_CMD_EPC_BUSY_ | E2P_CMD_EPC_CMD_EWDS_));

	while ((timeout > 0) && (smc911x_reg_read(E2P_CMD) & E2P_CMD_EPC_BUSY_)) {
		udelay(10);
		timeout--;
	}

	if (timeout == 0) {
		printf("Timeout[2]\n");
		return 0;
	}

	return 1;
}

static int eeprom_write_location(unchar address, unchar data)
{
	ulong timeout = 100000;

	debug("%s: address: %x data = %x\n", __func__, address, data);

	if (smc911x_reg_read(E2P_CMD) & E2P_CMD_EPC_BUSY_) {
		printf("%s: Busy at start\n", __func__);
		return 0;
	}

	smc911x_reg_write(E2P_DATA, ((ulong) data));
	smc911x_reg_write(E2P_CMD,
			  (E2P_CMD_EPC_BUSY_ | E2P_CMD_EPC_CMD_WRITE_ |
			   ((ulong) address)));

	while ((timeout > 0) && (smc911x_reg_read(E2P_CMD) & E2P_CMD_EPC_BUSY_)) {
		udelay(10);
		timeout--;
	}

	if (timeout == 0) {
		printf("Timeout[3]\n");
		return 0;
	}

	return 1;
}

static int eeprom_erase_all(void)
{
	ulong timeout = 100000;

	if (smc911x_reg_read(E2P_CMD) & E2P_CMD_EPC_BUSY_) {
		printf("%s: Busy at start\n", __func__);
		return 0;
	}

	smc911x_reg_write(E2P_CMD, (E2P_CMD_EPC_BUSY_ | E2P_CMD_EPC_CMD_ERAL_));

	while ((timeout > 0) && (smc911x_reg_read(E2P_CMD) & E2P_CMD_EPC_BUSY_)) {
		udelay(10);
		timeout--;
	}

	if (timeout == 0) {
		printf("Timeout[4]\n");
		return 0;
	}

	return 1;
}

static int eeprom_reload(void)
{
	ulong timeout = 100000;

	if (smc911x_reg_read(E2P_CMD) & E2P_CMD_EPC_BUSY_) {
		printf("%s: Busy at start\n", __func__);
		return -1;
	}
	smc911x_reg_write(E2P_CMD,
			  (E2P_CMD_EPC_BUSY_ | E2P_CMD_EPC_CMD_RELOAD_));

	while ((timeout > 0) && (smc911x_reg_read(E2P_CMD) & E2P_CMD_EPC_BUSY_)) {
		udelay(10);
		timeout--;
	}

	if (timeout == 0)
		return 0;

	return 1;
}

static int eeprom_save_mac_address(ulong dwHi16, ulong dwLo32)
{
	int result = 0;

	debug("%s: dwHI: 0x%08lx dwLO: %08lx, \n", __func__, dwHi16, dwLo32);

	if (!eeprom_enable_erase_and_write())
		goto DONE;
	if (!eeprom_erase_all())
		goto DONE;
	if (!eeprom_write_location(0, 0xA5))
		goto DONE;
	if (!eeprom_write_location(1, LOBYTE(LOWORD(dwLo32))))
		goto DONE;
	if (!eeprom_write_location(2, HIBYTE(LOWORD(dwLo32))))
		goto DONE;
	if (!eeprom_write_location(3, LOBYTE(HIWORD(dwLo32))))
		goto DONE;
	if (!eeprom_write_location(4, HIBYTE(HIWORD(dwLo32))))
		goto DONE;
	if (!eeprom_write_location(5, LOBYTE(LOWORD(dwHi16))))
		goto DONE;
	if (!eeprom_write_location(6, HIBYTE(LOWORD(dwHi16))))
		goto DONE;
	if (!eeprom_disable_erase_and_write())
		goto DONE;

	result = 1;

DONE:
	return result;
}

static int do_eeprom_dump(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unchar data = 0, index = 0;
	ulong gpio_old_val;

	gpio_old_val = eeprom_enable_access();

	printf("EEPROM content: \n");
	for (index = 0; index < 8; index++) {
		if (eeprom_read_location(index, &data))
			printf("%02x ", data);
		else
			printf("FAILED");
	}

	eeprom_disable_access(gpio_old_val);
	printf("\n");

	return 0;
}

static int do_eeprom_erase_all(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	eeprom_erase_all();

	return 0;
}

static int do_eeprom_save_mac(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong hi16, lo32;
	unchar ethaddr[6], i;
	ulong gpio;
	char *tmp, *end;

	tmp = argv[1];
	for (i = 0; i < 6; i++) {
		ethaddr[i] = tmp ? simple_strtoul(tmp, &end, 16) : 0;
		if (tmp)
			tmp = (*end) ? end + 1 : end;
	}

	hi16 = (ethaddr[5] << 8) | (ethaddr[4]);
	lo32 = (ethaddr[3] << 24) | (ethaddr[2] << 16) |
		(ethaddr[1] << 8) | (ethaddr[0]);

	gpio = eeprom_enable_access();

	eeprom_save_mac_address(hi16, lo32);

	eeprom_reload();

	/* Check new values */
	if (eeprom_is_mac_address_loaded()) {
		ulong mac_hi16, mac_lo32;

		mac_hi16 = get_mac_reg(MAC_ADDRH);
		mac_lo32 = get_mac_reg(MAC_ADDRL);
		printf("New MAC address: %lx, %lx\n", mac_hi16, mac_lo32);
	} else {
		printf("Address is not reloaded \n");
	}
	eeprom_disable_access(gpio);

	return 0;
}

U_BOOT_CMD(smcee, 1, 0, do_eeprom_erase_all,
	   "smcee   - Erase content of SMC EEPROM",);

U_BOOT_CMD(smced, 1, 0, do_eeprom_dump,
	   "smced   - Dump content of SMC EEPROM",);

U_BOOT_CMD(smcew, 2, 0, do_eeprom_save_mac,
	   "smcew   - Write MAC address to SMC EEPROM\n",
	   "aa:bb:cc:dd:ee:ff  new mac address");
