/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Library to support early TI EVM EEPROM handling
 *
 * Copyright (C) 2015-2016 Texas Instruments Incorporated - https://www.ti.com
 */

#ifndef __BOARD_DETECT_H
#define __BOARD_DETECT_H

/* TI EEPROM MAGIC Header identifier */
#include <linux/bitops.h>
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

/* AM6x TI EVM EEPROM Definitions */
#define TI_AM6_EEPROM_RECORD_BOARD_ID		0x01
#define TI_AM6_EEPROM_RECORD_BOARD_INFO		0x10
#define TI_AM6_EEPROM_RECORD_DDR_INFO		0x11
#define TI_AM6_EEPROM_RECORD_DDR_SPD		0x12
#define TI_AM6_EEPROM_RECORD_MAC_INFO		0x13
#define TI_AM6_EEPROM_RECORD_END_LIST		0xFE

/*
 * Common header for AM6x TI EVM EEPROM records. Used to encapsulate the config
 * EEPROM in its entirety as well as for individual records contained within.
 */
struct ti_am6_eeprom_record_header {
	u8 id;
	u16 len;
} __attribute__ ((__packed__));

/* AM6x TI EVM EEPROM board ID structure */
struct ti_am6_eeprom_record_board_id {
	u32 magic_number;
	struct ti_am6_eeprom_record_header header;
} __attribute__ ((__packed__));

/* AM6x TI EVM EEPROM board info structure */
#define AM6_EEPROM_HDR_NAME_LEN			16
#define AM6_EEPROM_HDR_VERSION_LEN		2
#define AM6_EEPROM_HDR_PROC_NR_LEN		4
#define AM6_EEPROM_HDR_VARIANT_LEN		2
#define AM6_EEPROM_HDR_PCB_REV_LEN		2
#define AM6_EEPROM_HDR_SCH_BOM_REV_LEN		2
#define AM6_EEPROM_HDR_SW_REV_LEN		2
#define AM6_EEPROM_HDR_VID_LEN			2
#define AM6_EEPROM_HDR_BLD_WK_LEN		2
#define AM6_EEPROM_HDR_BLD_YR_LEN		2
#define AM6_EEPROM_HDR_4P_NR_LEN		6
#define AM6_EEPROM_HDR_SERIAL_LEN		4

struct ti_am6_eeprom_record_board_info {
	char name[AM6_EEPROM_HDR_NAME_LEN];
	char version[AM6_EEPROM_HDR_VERSION_LEN];
	char proc_number[AM6_EEPROM_HDR_PROC_NR_LEN];
	char variant[AM6_EEPROM_HDR_VARIANT_LEN];
	char pcb_revision[AM6_EEPROM_HDR_PCB_REV_LEN];
	char schematic_bom_revision[AM6_EEPROM_HDR_SCH_BOM_REV_LEN];
	char software_revision[AM6_EEPROM_HDR_SW_REV_LEN];
	char vendor_id[AM6_EEPROM_HDR_VID_LEN];
	char build_week[AM6_EEPROM_HDR_BLD_WK_LEN];
	char build_year[AM6_EEPROM_HDR_BLD_YR_LEN];
	char board_4p_number[AM6_EEPROM_HDR_4P_NR_LEN];
	char serial[AM6_EEPROM_HDR_SERIAL_LEN];
} __attribute__ ((__packed__));

/* Memory location to keep a copy of the AM6 board info record */
#define TI_AM6_EEPROM_BD_INFO_DATA ((struct ti_am6_eeprom_record_board_info *) \
					     TI_SRAM_SCRATCH_BOARD_EEPROM_START)

/* AM6x TI EVM EEPROM DDR info structure */
#define TI_AM6_EEPROM_DDR_CTRL_INSTANCE_MASK		GENMASK(1, 0)
#define TI_AM6_EEPROM_DDR_CTRL_INSTANCE_SHIFT		0
#define TI_AM6_EEPROM_DDR_CTRL_SPD_DATA_LOC_MASK	GENMASK(3, 2)
#define TI_AM6_EEPROM_DDR_CTRL_SPD_DATA_LOC_NA		(0 << 2)
#define TI_AM6_EEPROM_DDR_CTRL_SPD_DATA_LOC_BOARDID	(2 << 2)
#define TI_AM6_EEPROM_DDR_CTRL_SPD_DATA_LOC_I2C51	(3 << 2)
#define TI_AM6_EEPROM_DDR_CTRL_MEM_TYPE_MASK		GENMASK(5, 4)
#define TI_AM6_EEPROM_DDR_CTRL_MEM_TYPE_DDR3		(0 << 4)
#define TI_AM6_EEPROM_DDR_CTRL_MEM_TYPE_DDR4		(1 << 4)
#define TI_AM6_EEPROM_DDR_CTRL_MEM_TYPE_LPDDR4		(2 << 4)
#define TI_AM6_EEPROM_DDR_CTRL_IF_DATA_WIDTH_MASK	GENMASK(7, 6)
#define TI_AM6_EEPROM_DDR_CTRL_IF_DATA_WIDTH_16		(0 << 6)
#define TI_AM6_EEPROM_DDR_CTRL_IF_DATA_WIDTH_32		(1 << 6)
#define TI_AM6_EEPROM_DDR_CTRL_IF_DATA_WIDTH_64		(2 << 6)
#define TI_AM6_EEPROM_DDR_CTRL_DEV_DATA_WIDTH_MASK	GENMASK(9, 8)
#define TI_AM6_EEPROM_DDR_CTRL_DEV_DATA_WIDTH_8		(0 << 8)
#define TI_AM6_EEPROM_DDR_CTRL_DEV_DATA_WIDTH_16	(1 << 8)
#define TI_AM6_EEPROM_DDR_CTRL_DEV_DATA_WIDTH_32	(2 << 8)
#define TI_AM6_EEPROM_DDR_CTRL_RANKS_2			BIT(10)
#define TI_AM6_EEPROM_DDR_CTRL_DENS_MASK		GENMASK(13, 11)
#define TI_AM6_EEPROM_DDR_CTRL_DENS_1GB			(0 << 11)
#define TI_AM6_EEPROM_DDR_CTRL_DENS_2GB			(1 << 11)
#define TI_AM6_EEPROM_DDR_CTRL_DENS_4GB			(2 << 11)
#define TI_AM6_EEPROM_DDR_CTRL_DENS_8GB			(3 << 11)
#define TI_AM6_EEPROM_DDR_CTRL_DENS_12GB		(4 << 11)
#define TI_AM6_EEPROM_DDR_CTRL_DENS_16GB		(5 << 11)
#define TI_AM6_EEPROM_DDR_CTRL_DENS_24GB		(6 << 11)
#define TI_AM6_EEPROM_DDR_CTRL_DENS_32GB		(7 << 11)
#define TI_AM6_EEPROM_DDR_CTRL_ECC			BIT(14)

struct ti_am6_eeprom_record_ddr_info {
	u16 ddr_control;
} __attribute__ ((__packed__));

/* AM6x TI EVM EEPROM DDR SPD structure */
#define TI_AM6_EEPROM_DDR_SPD_INSTANCE_MASK		GENMASK(1, 0)
#define TI_AM6_EEPROM_DDR_SPD_INSTANCE_SHIFT		0
#define TI_AM6_EEPROM_DDR_SPD_MEM_TYPE_MASK		GENMASK(4, 3)
#define TI_AM6_EEPROM_DDR_SPD_MEM_TYPE_DDR3		(0 << 3)
#define TI_AM6_EEPROM_DDR_SPD_MEM_TYPE_DDR4		(1 << 3)
#define TI_AM6_EEPROM_DDR_SPD_MEM_TYPE_LPDDR4		(2 << 3)
#define TI_AM6_EEPROM_DDR_SPD_DATA_LEN			512

struct ti_am6_eeprom_record_ddr_spd {
	u16 spd_control;
	u8 data[TI_AM6_EEPROM_DDR_SPD_DATA_LEN];
} __attribute__ ((__packed__));

/* AM6x TI EVM EEPROM MAC info structure */
#define TI_AM6_EEPROM_MAC_INFO_INSTANCE_MASK		GENMASK(2, 0)
#define TI_AM6_EEPROM_MAC_INFO_INSTANCE_SHIFT		0
#define TI_AM6_EEPROM_MAC_ADDR_COUNT_MASK		GENMASK(7, 3)
#define TI_AM6_EEPROM_MAC_ADDR_COUNT_SHIFT		3
#define TI_AM6_EEPROM_MAC_ADDR_MAX_COUNT		32

struct ti_am6_eeprom_record_mac_info {
	u16 mac_control;
	u8 mac_addr[TI_AM6_EEPROM_MAC_ADDR_MAX_COUNT][TI_EEPROM_HDR_ETH_ALEN];
} __attribute__ ((__packed__));

struct ti_am6_eeprom_record {
	struct ti_am6_eeprom_record_header header;
	union {
		struct ti_am6_eeprom_record_board_info board_info;
		struct ti_am6_eeprom_record_ddr_info ddr_info;
		struct ti_am6_eeprom_record_ddr_spd ddr_spd;
		struct ti_am6_eeprom_record_mac_info mac_info;
	} data;
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

/*
 * Maximum number of Ethernet MAC addresses extracted from the AM6x on-board
 * EEPROM during the initial probe and carried forward in SRAM.
 */
#define AM6_EEPROM_HDR_NO_OF_MAC_ADDR	8

/**
 * struct ti_am6_eeprom - Null terminated, usable EEPROM contents, as extracted
 *	from the AM6 on-board EEPROM. Note that we only carry a subset of data
 *	at this time to be considerate about memory consumption.
 * @header:		Magic number for data validity indication
 * @name:		NULL terminated name
 * @version:		NULL terminated version
 * @software_revision:	NULL terminated software revision
 * @serial:		Board serial number
 * @mac_addr_cnt:	Number of MAC addresses stored in this object
 * @mac_addr:		MAC addresses
 */
struct ti_am6_eeprom {
	u32 header;
	char name[AM6_EEPROM_HDR_NAME_LEN + 1];
	char version[AM6_EEPROM_HDR_VERSION_LEN + 1];
	char software_revision[AM6_EEPROM_HDR_SW_REV_LEN + 1];
	char serial[AM6_EEPROM_HDR_SERIAL_LEN + 1];
	u8 mac_addr_cnt;
	char mac_addr[AM6_EEPROM_HDR_NO_OF_MAC_ADDR][TI_EEPROM_HDR_ETH_ALEN];
};

#define TI_AM6_EEPROM_DATA ((struct ti_am6_eeprom *) \
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
 * ti_emmc_boardid_get() - Fetch board ID information from eMMC
 *
 * ep in SRAM is populated by the this function that is currently
 * based on BeagleBone AI, but could be made more general across AM*
 * platforms.
 */
int __maybe_unused ti_emmc_boardid_get(void);

/**
 * ti_i2c_eeprom_dra7_get() - Consolidated eeprom data for DRA7 TI EVMs
 * @bus_addr:	I2C bus address
 * @dev_addr:	I2C slave address
 */
int ti_i2c_eeprom_dra7_get(int bus_addr, int dev_addr);

/**
 * ti_i2c_eeprom_am6_get() - Consolidated eeprom data for AM6x TI EVMs and
 *			     associated daughter cards, parsed into user-
 *			     provided data structures
 * @bus_addr:	I2C bus address
 * @dev_addr:	I2C slave address
 * @ep:		Pointer to structure receiving AM6-specific header data
 * @mac_addr:	Pointer to memory receiving parsed MAC addresses. May be
 *		NULL to skip MAC parsing.
 * @mac_addr_max_cnt: Maximum number of MAC addresses that can be stored into
 *		      mac_addr. May be NULL to skip MAC parsing.
 * @mac_addr_cnt: Pointer to a location returning how many MAC addressed got
 *		  actually parsed.
 */
int __maybe_unused ti_i2c_eeprom_am6_get(int bus_addr, int dev_addr,
					 struct ti_am6_eeprom *ep,
					 char **mac_addr,
					 u8 mac_addr_max_cnt,
					 u8 *mac_addr_cnt);

/**
 * ti_i2c_eeprom_am6_get_base() - Consolidated eeprom data for AM6x TI EVMs
 * @bus_addr:	I2C bus address
 * @dev_addr:	I2C slave address
 */
int __maybe_unused ti_i2c_eeprom_am6_get_base(int bus_addr, int dev_addr);

#ifdef CONFIG_TI_I2C_BOARD_DETECT
/**
 * board_ti_is() - Board detection logic for TI EVMs
 * @name_tag:	Tag used in eeprom for the board
 *
 * Return: false if board information does not match OR eeprom wasn't read.
 *	   true otherwise
 */
bool board_ti_is(char *name_tag);

/**
 * board_ti_k3_is() - Board detection logic for TI K3 EVMs
 * @name_tag:	Tag used in eeprom for the board
 *
 * Return: false if board information does not match OR eeprom wasn't read.
 *	   true otherwise
 */
bool board_ti_k3_is(char *name_tag);

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
 * set_board_info_env_am6() - Setup commonly used board information environment
 *			      vars for AM6-type boards
 * @name:	Name of the board
 *
 * If name is NULL, default_name is used.
 */
void set_board_info_env_am6(char *name);

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
 * board_ti_am6_set_ethaddr- Sets the ethaddr environment from EEPROM
 * @index: The first eth<index>addr environment variable to set
 * @count: The number of MAC addresses to process
 *
 * EEPROM should be already read before calling this function. The EEPROM
 * contains n dedicated MAC addresses. This function sets the ethaddr
 * environment variable for all the available MAC addresses starting
 * from eth<index>addr.
 */
void board_ti_am6_set_ethaddr(int index, int count);

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
#else
static inline bool board_ti_is(char *name_tag) { return false; };
static inline bool board_ti_k3_is(char *name_tag) { return false; };
static inline bool board_ti_rev_is(char *rev_tag, int cmp_len)
{ return false; };
static inline char *board_ti_get_rev(void) { return NULL; };
static inline char *board_ti_get_config(void) { return NULL; };
static inline char *board_ti_get_name(void) { return NULL; };
static inline bool board_ti_was_eeprom_read(void) { return false; };
static inline int ti_i2c_eeprom_am_set(const char *name, const char *rev)
{ return -EINVAL; };
#endif

#endif	/* __BOARD_DETECT_H */
