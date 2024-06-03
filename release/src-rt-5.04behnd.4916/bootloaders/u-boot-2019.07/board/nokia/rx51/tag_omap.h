/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011-2012
 * Pali Rohár <pali.rohar@gmail.com>
 *
 * (C) Copyright 2011
 * marcel@mesa.nl, Mesa Consulting B.V.
 *
 * (C) Copyright 2004-2005
 * Nokia Corporation
 */


/*
 *  Code copied from maemo kernel 2.6.28 file
 *  arch/arm/plat-omap/include/mach/board.h
 *
 *  Information structures for board-specific data
 *
 *  Copyright (C) 2004	Nokia Corporation
 *  Written by Juha Yrjölä <juha.yrjola@nokia.com>
 */

/* Different peripheral ids */
#define OMAP_TAG_CLOCK		0x4f01
#define OMAP_TAG_SERIAL_CONSOLE	0x4f03
#define OMAP_TAG_USB		0x4f04
#define OMAP_TAG_LCD		0x4f05
#define OMAP_TAG_GPIO_SWITCH	0x4f06
#define OMAP_TAG_UART		0x4f07
#define OMAP_TAG_FBMEM		0x4f08
#define OMAP_TAG_STI_CONSOLE	0x4f09
#define OMAP_TAG_CAMERA_SENSOR	0x4f0a
#define OMAP_TAG_PARTITION	0x4f0b
#define OMAP_TAG_TEA5761	0x4f10
#define OMAP_TAG_TMP105		0x4f11

#define OMAP_TAG_BOOT_REASON	0x4f80
#define OMAP_TAG_FLASH_PART_STR	0x4f81
#define OMAP_TAG_VERSION_STR	0x4f82

#define OMAP_TAG_NOKIA_BT	0x4e01
#define OMAP_TAG_WLAN_CX3110X	0x4e02
#define OMAP_TAG_CBUS		0x4e03
#define OMAP_TAG_EM_ASIC_BB5	0x4e04


struct omap_clock_config {
	/* 0 for 12 MHz, 1 for 13 MHz and 2 for 19.2 MHz */
	u8 system_clock_type;
};

struct omap_serial_console_config {
	u8 console_uart;
	u32 console_speed;
};

struct omap_sti_console_config {
	unsigned enable:1;
	u8 channel;
};

struct omap_usb_config {
	/* Configure drivers according to the connectors on your board:
	 *  - "A" connector (rectagular)
	 *	... for host/OHCI use, set "register_host".
	 *  - "B" connector (squarish) or "Mini-B"
	 *	... for device/gadget use, set "register_dev".
	 *  - "Mini-AB" connector (very similar to Mini-B)
	 *	... for OTG use as device OR host, initialize "otg"
	 */
	unsigned	register_host:1;
	unsigned	register_dev:1;
	u8		otg;	/* port number, 1-based:  usb1 == 2 */

	u8		hmc_mode;

	/* implicitly true if otg:  host supports remote wakeup? */
	u8		rwc;

	/* signaling pins used to talk to transceiver on usbN:
	 *  0 == usbN unused
	 *  2 == usb0-only, using internal transceiver
	 *  3 == 3 wire bidirectional
	 *  4 == 4 wire bidirectional
	 *  6 == 6 wire unidirectional (or TLL)
	 */
	u8		pins[3];
};

struct omap_lcd_config {
	char panel_name[16];
	char ctrl_name[16];
	s16  nreset_gpio;
	u8   data_lines;
};

struct omap_fbmem_config {
	u32 start;
	u32 size;
};

struct omap_gpio_switch_config {
	char name[12];
	u16 gpio;
	u8 flags:4;
	u8 type:4;
	unsigned int key_code:24; /* Linux key code */
};

struct omap_uart_config {
	/* Bit field of UARTs present; bit 0 --> UART1 */
	unsigned int enabled_uarts;
};

struct omap_tea5761_config {
	u16 enable_gpio;
};

struct omap_partition_config {
	char name[16];
	unsigned int size;
	unsigned int offset;
	/* same as in include/linux/mtd/partitions.h */
	unsigned int mask_flags;
};

struct omap_flash_part_str_config {
	char part_table[0];
};

struct omap_boot_reason_config {
	char reason_str[12];
};

struct omap_version_config {
	char component[12];
	char version[12];
};

/*
 *  Code copied from maemo kernel 2.6.28 file
 *  arch/arm/plat-omap/include/mach/board-nokia.h
 *
 *  Information structures for Nokia-specific board config data
 *
 *  Copyright (C) 2005  Nokia Corporation
 */

struct omap_bluetooth_config {
	u8 chip_type;
	u8 bt_wakeup_gpio;
	u8 host_wakeup_gpio;
	u8 reset_gpio;
	u8 bt_uart;
	u8 bd_addr[6];
	u8 bt_sysclk;
};

struct omap_wlan_cx3110x_config {
	u8 chip_type;
	u8 reserverd;
	s16 power_gpio;
	s16 irq_gpio;
	s16 spi_cs_gpio;
};

struct omap_cbus_config {
	s16 clk_gpio;
	s16 dat_gpio;
	s16 sel_gpio;
};

struct omap_em_asic_bb5_config {
	s16 retu_irq_gpio;
	s16 tahvo_irq_gpio;
};

/*
 *  omap_tag handling
 *
 *  processing omap tag structures
 *
 *  Copyright (C) 2011  marcel@mesa.nl, Mesa Consulting B.V.
 *  Copyright (C) 2012  Pali Rohár <pali.rohar@gmail.com>
 */

/* TI OMAP specific information */
#define ATAG_BOARD	0x414f4d50

struct tag_omap_header {
	u16 tag;
	u16 size;
};

struct tag_omap {
	struct tag_omap_header hdr;
	union {
		struct omap_clock_config clock;
		struct omap_serial_console_config serial_console;
		struct omap_sti_console_config sti_console;
		struct omap_usb_config usb;
		struct omap_lcd_config lcd;
		struct omap_fbmem_config fbmem;
		struct omap_gpio_switch_config gpio_switch;
		struct omap_uart_config uart;
		struct omap_tea5761_config tea5761;
		struct omap_partition_config partition;
		struct omap_flash_part_str_config flash_part_str;
		struct omap_boot_reason_config boot_reason;
		struct omap_version_config version;
		struct omap_bluetooth_config bluetooth;
		struct omap_wlan_cx3110x_config wlan_cx3110x;
		struct omap_cbus_config cbus;
		struct omap_em_asic_bb5_config em_asic_bb5;
	} u;
};

#define tag_omap_next(t)	((struct tag_omap *)((u8 *)(t) + \
				(t)->hdr.size + sizeof(struct tag_omap_header)))

#define OMAP_TAG_HEADER_CONFIG(config, type) \
	.hdr.tag = config, \
	.hdr.size = sizeof(struct type)

#define OMAP_TAG_UART_CONFIG(p1) \
	{ \
		OMAP_TAG_HEADER_CONFIG(OMAP_TAG_UART, omap_uart_config), \
		.u.uart.enabled_uarts = p1, \
	}

#define OMAP_TAG_SERIAL_CONSOLE_CONFIG(p1, p2) \
	{ \
		OMAP_TAG_HEADER_CONFIG(OMAP_TAG_SERIAL_CONSOLE, \
			omap_serial_console_config), \
		.u.serial_console.console_uart = p1, \
		.u.serial_console.console_speed = p2, \
	}

#define OMAP_TAG_LCD_CONFIG(p1, p2, p3, p4) \
	{ \
		OMAP_TAG_HEADER_CONFIG(OMAP_TAG_LCD, omap_lcd_config), \
		.u.lcd.panel_name = p1, \
		.u.lcd.ctrl_name = p2, \
		.u.lcd.nreset_gpio = p3, \
		.u.lcd.data_lines = p4, \
	}

#define OMAP_TAG_GPIO_SWITCH_CONFIG(p1, p2, p3, p4, p5) \
	{ \
		OMAP_TAG_HEADER_CONFIG(OMAP_TAG_GPIO_SWITCH, \
			omap_gpio_switch_config), \
		.u.gpio_switch.name = p1, \
		.u.gpio_switch.gpio = p2, \
		.u.gpio_switch.flags = p3, \
		.u.gpio_switch.type = p4, \
		.u.gpio_switch.key_code = p5, \
	}

#define OMAP_TAG_WLAN_CX3110X_CONFIG(p1, p2, p3, p4, p5) \
	{ \
		OMAP_TAG_HEADER_CONFIG(OMAP_TAG_WLAN_CX3110X, \
			omap_wlan_cx3110x_config), \
		.u.wlan_cx3110x.chip_type = p1, \
		.u.wlan_cx3110x.reserverd = p2, \
		.u.wlan_cx3110x.power_gpio = p3, \
		.u.wlan_cx3110x.irq_gpio = p4, \
		.u.wlan_cx3110x.spi_cs_gpio = p5, \
	}

#define OMAP_TAG_PARTITION_CONFIG(p1, p2, p3, p4) \
	{ \
		OMAP_TAG_HEADER_CONFIG(OMAP_TAG_PARTITION, \
			omap_partition_config), \
		.u.partition.name = p1, \
		.u.partition.size = p2, \
		.u.partition.offset = p3, \
		.u.partition.mask_flags = p4, \
	}

#define OMAP_TAG_BOOT_REASON_CONFIG(p1) \
	{ \
		OMAP_TAG_HEADER_CONFIG(OMAP_TAG_BOOT_REASON, \
			omap_boot_reason_config), \
		.u.boot_reason.reason_str = p1, \
	}

#define OMAP_TAG_VERSION_STR_CONFIG(p1, p2) \
	{ \
		OMAP_TAG_HEADER_CONFIG(OMAP_TAG_VERSION_STR, \
			omap_version_config), \
		.u.version.component = p1, \
		.u.version.version = p2, \
	}
