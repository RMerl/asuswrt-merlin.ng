/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Library to support early TI EVM EEPROM handling
 *
 * Copyright (C) 2015-2016 Texas Instruments Incorporated - http://www.ti.com
 */

#ifndef __BOARD_DETECT_H
#define __BOARD_DETECT_H

/* TI EEPROM MAGIC Header identifier */
#define TI_EEPROM_HEADER_MAGIC	0xEE3355AA
#define TI_DEAD_EEPROM_MAGIC	0xADEAD12C

#define TI_EEPROM_HDR_NAME_LEN		8
#define TI_EEPROM_HDR_REV_LEN		4
#define TI_EEPROM_HDR_SERIAL_LEN	12
#define TI_EEPROM_HDR_CONFIG_LEN	32
#define TI_EEPROM_HDR_NO_OF_MAC_ADDR	3
#define TI_EEPROM_HDR_ETH_ALEN		6

/**
 * struct ti_am_eeprom - This structure holds data read in from the
 *                     AM335x, AM437x, AM57xx TI EVM EEPROMs.
 * @header: This holds the magic number
 * @name: The name of the board
 * @version: Board revision
 * @serial: Board serial number
 * @config: Reserved
 * @mac_addr: Any MAC addresses written in the EEPROM
 *
 * The data is this structure is read from the EEPROM on the board.
 * It is used for board detection which is based on name. It is used
 * to configure specific TI boards. This allows booting of multiple
 * TI boards with a single MLO and u-boot.
 */
struct ti_am_eeprom {
	unsigned int header;
	char name[TI_EEPROM_HDR_NAME_LEN];
	char version[TI_EEPROM_HDR_REV_LEN];
	char serial[TI_EEPROM_HDR_SERIAL_LEN];
	char config[TI_EEPROM_HDR_CONFIG_LEN];
	char mac_addr[TI_EEPROM_HDR_NO_OF_MAC_ADDR][TI_EEPROM_HDR_ETH_ALEN];
} __attribute__ ((__packed__));

/* DRA7 EEPROM MAGIC Header identifier */
#define DRA7_EEPROM_HEADER_MAGIC	0xAA5533EE
#define DRA7_EEPROM_HDR_NAME_LEN	16
#define DRA7_EEPROM_HDR_CONFIG_LEN	4

/**
 * struct dra7_eeprom - This structure holds data read in from the DRA7 EVM
 *			EEPROMs.
 * @header: This holds the magic number
 * @name: The name of the board
 * @version_major: Board major version
 * @version_minor: Board minor version
 * @config: Board specific config options
 * @emif1_size: Size of DDR attached to EMIF1
 * @emif2_size: Size of DDR attached to EMIF2
 *
 * The data is this structure is read from the EEPROM on the board.
 * It is used for board detection which is based on name. It is used
 * to configure specific DRA7 boards. This allows booting of multiple
 * DRA7 boards with a single MLO and u-boot.
 */
struct dra7_eeprom {
	u32 header;
	char name[DRA7_EEPROM_HDR_NAME_LEN];
	u16 version_major;
	u16 version_minor;
	char config[DRA7_EEPROM_HDR_CONFIG_LEN];
	u32 emif1_size;
	u32 emif2_size;
} __attribute__ ((__packed__));

/**
 * struct ti_common_eeprom - Null terminated, usable EEPROM contents.
 * header:	Magic number
 * @name:	NULL terminated name
 * @version:	NULL terminated version
 * @serial:	NULL terminated serial number
 * @config:	NULL terminated Board specific config options
 * @mac_addr:	MAC addresses
 * @emif1_size:	Size of the ddr available on emif1
 * @emif2_size:	Size of the ddr available on emif2
 */
struct ti_common_eeprom {
	u32 header;
	char name[TI_EEPROM_HDR_NAME_LEN + 1];
	char version[TI_EEPROM_HDR_REV_LEN + 1];
	char serial[TI_EEPROM_HDR_SERIAL_LEN + 1];
	char config[TI_EEPROM_HDR_CONFIG_LEN + 1];
	char mac_addr[TI_EEPROM_HDR_NO_OF_MAC_ADDR][TI_EEPROM_HDR_ETH_ALEN];
	u64 emif1_size;
	u64 emif2_size;
};

#define TI_EEPROM_DATA ((struct ti_common_eeprom *)\
				TI_SRAM_SCRATCH_BOARD_EEPROM_START)

/**
 * ti_i2c_eeprom_am_get() - Consolidated eeprom data collection for AM* TI EVMs
 * @bus_addr:	I2C bus address
 * @dev_addr:	I2C slave address
 *
 * ep in SRAM is populated by the this AM generic function that consolidates
 * the basic initialization logic common across all AM* platforms.
 */
int ti_i2c_eeprom_am_get(int bus_addr, int dev_addr);

/**
 * ti_i2c_eeprom_dra7_get() - Consolidated eeprom data for DRA7 TI EVMs
 * @bus_addr:	I2C bus address
 * @dev_addr:	I2C slave address
 */
int ti_i2c_eeprom_dra7_get(int bus_addr, int dev_addr);

/**
 * board_ti_is() - Board detection logic for TI EVMs
 * @name_tag:	Tag used in eeprom for the board
 *
 * Return: false if board information does not match OR eeprom wasn't read.
 *	   true otherwise
 */
bool board_ti_is(char *name_tag);

/**
 * board_ti_rev_is() - Compare board revision for TI EVMs
 * @rev_tag:	Revision tag to check in eeprom
 * @cmp_len:	How many chars to compare?
 *
 * NOTE: revision information is often messed up (hence the str len match) :(
 *
 * Return: false if board information does not match OR eeprom wasn't read.
 *	   true otherwise
 */
bool board_ti_rev_is(char *rev_tag, int cmp_len);

/**
 * board_ti_get_rev() - Get board revision for TI EVMs
 *
 * Return: Empty string if eeprom wasn't read.
 *	   Board revision otherwise
 */
char *board_ti_get_rev(void);

/**
 * board_ti_get_config() - Get board config for TI EVMs
 *
 * Return: Empty string if eeprom wasn't read.
 *	   Board config otherwise
 */
char *board_ti_get_config(void);

/**
 * board_ti_get_name() - Get board name for TI EVMs
 *
 * Return: Empty string if eeprom wasn't read.
 *	   Board name otherwise
 */
char *board_ti_get_name(void);

/**
 * board_ti_get_eth_mac_addr() - Get Ethernet MAC address from EEPROM MAC list
 * @index:	0 based index within the list of MAC addresses
 * @mac_addr:	MAC address contained at the index is returned here
 *
 * Does not sanity check the mac_addr. Whatever is stored in EEPROM is returned.
 */
void board_ti_get_eth_mac_addr(int index, u8 mac_addr[TI_EEPROM_HDR_ETH_ALEN]);

/**
 * board_ti_get_emif1_size() - Get size of the DDR on emif1 for TI EVMs
 *
 * Return: NULL if eeprom wasn't read or emif1_size is not available.
 */
u64 board_ti_get_emif1_size(void);

/**
 * board_ti_get_emif2_size() - Get size of the DDR on emif2 for TI EVMs
 *
 * Return: NULL if eeprom wasn't read or emif2_size is not available.
 */
u64 board_ti_get_emif2_size(void);

/**
 * set_board_info_env() - Setup commonly used board information environment vars
 * @name:	Name of the board
 *
 * If name is NULL, default_name is used.
 */
void set_board_info_env(char *name);

/**
 * board_ti_set_ethaddr- Sets the ethaddr environment from EEPROM
 * @index: The first eth<index>addr environment variable to set
 *
 * EEPROM should be already read before calling this function.
 * The EEPROM contains 2 MAC addresses which define the MAC address
 * range (i.e. first and last MAC address).
 * This function sets the ethaddr environment variable for all
 * the available MAC addresses starting from eth<index>addr.
 */
void board_ti_set_ethaddr(int index);

/**
 * board_ti_was_eeprom_read() - Check to see if the eeprom contents have been read
 *
 * This function is useful to determine if the eeprom has already been read and
 * its contents have already been loaded into memory. It utiltzes the magic
 * number that the header value is set to upon successful eeprom read.
 */
bool board_ti_was_eeprom_read(void);

/**
 * ti_i2c_eeprom_am_set() - Setup the eeprom data with predefined values
 * @name:	Name of the board
 * @rev:	Revision of the board
 *
 * In some cases such as in RTC-only mode, we are able to skip reading eeprom
 * and wasting i2c based initialization time by using predefined flags for
 * detecting what platform we are booting on. For those platforms, provide
 * a handy function to pre-program information.
 *
 * NOTE: many eeprom information such as serial number, mac address etc is not
 * available.
 *
 * Return: 0 if all went fine, else return error.
 */
int ti_i2c_eeprom_am_set(const char *name, const char *rev);

#endif	/* __BOARD_DETECT_H */
