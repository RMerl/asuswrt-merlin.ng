/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * board.h
 *
 * (C) Copyright 2016
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * TI AM335x boards information header
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#ifndef _BOARD_H_
#define _BOARD_H_

/* Definition to control the GPIOs (for LEDs and Reset) */
#define GPIO_TO_PIN(bank, gpio) (32 * (bank) + (gpio))

static inline int board_is_b_sample(void)
{
#if defined CONFIG_B_SAMPLE
	return 1;
#else
	return 0;
#endif
}

static inline int board_is_c_sample(void)
{
#if defined CONFIG_C_SAMPLE
	return 1;
#else
	return 0;
#endif
}

static inline int board_is_c3_sample(void)
{
#if defined CONFIG_C3_SAMPLE
	return 1;
#else
	return 0;
#endif
}

static inline int board_is_series(void)
{
#if defined CONFIG_SERIES
	return 1;
#else
	return 0;
#endif
}

/*
 * Definitions for pinmuxing header and Board ID strings
 */
#if defined CONFIG_B_SAMPLE
# define BOARD_ID_STR "SHC B-Sample\n"
#elif defined CONFIG_B2_SAMPLE
# define BOARD_ID_STR "SHC B2-Sample\n"
#elif defined CONFIG_C_SAMPLE
# if defined(CONFIG_SHC_NETBOOT)
#  define BOARD_ID_STR "#### NETBOOT ####\nSHC C-Sample\n"
# elif defined(CONFIG_SHC_SDBOOT)
#  define BOARD_ID_STR "#### SDBOOT ####\nSHC C-Sample\n"
# else
#  define BOARD_ID_STR "SHC C-Sample\n"
# endif
#elif defined CONFIG_C2_SAMPLE
# if defined(CONFIG_SHC_ICT)
#  define BOARD_ID_STR "#### ICT ####\nSHC C2-Sample\n"
# elif defined(CONFIG_SHC_NETBOOT)
#  define BOARD_ID_STR "#### NETBOOT ####\nSHC C2-Sample\n"
# elif defined(CONFIG_SHC_SDBOOT)
#  define BOARD_ID_STR "#### SDBOOT ####\nSHC C2-Sample\n"
# else
#  define BOARD_ID_STR "SHC C2-Sample\n"
# endif
#elif defined CONFIG_C3_SAMPLE
# if defined(CONFIG_SHC_ICT)
#  define BOARD_ID_STR "#### ICT ####\nSHC C3-Sample\n"
# elif defined(CONFIG_SHC_NETBOOT)
#  define BOARD_ID_STR "#### NETBOOT ####\nSHC C3-Sample\n"
# elif defined(CONFIG_SHC_SDBOOT)
#  define BOARD_ID_STR "#### SDBOOT ####\nSHC C3-Sample\n"
# else
#  define BOARD_ID_STR "SHC C3-Sample\n"
# endif
#elif defined CONFIG_SERIES
# if defined(CONFIG_SHC_ICT)
#  define BOARD_ID_STR "#### ICT ####\nSHC\n"
# elif defined(CONFIG_SHC_NETBOOT)
#  define BOARD_ID_STR "#### NETBOOT ####\nSHC\n"
# elif defined(CONFIG_SHC_SDBOOT)
#  define BOARD_ID_STR "#### SDBOOT ####\nSHC\n"
# else
#  define BOARD_ID_STR "SHC\n"
# endif
#else
# define BOARD_ID_STR "Unknown device!\n"
#endif

/*
 * Definitions for GPIO pin assignments
 */
#if defined CONFIG_B_SAMPLE

# define LED_PWR_BL_GPIO   GPIO_TO_PIN(1, 17)
# define LED_PWR_RD_GPIO   GPIO_TO_PIN(1, 18)
# define LED_PWR_GN_GPIO   GPIO_TO_PIN(1, 19)
# define LED_CONN_BL_GPIO  GPIO_TO_PIN(0, 26)
# define LED_CONN_RD_GPIO  GPIO_TO_PIN(0, 22)
# define LED_CONN_GN_GPIO  GPIO_TO_PIN(0, 23)
# define RESET_GPIO        GPIO_TO_PIN(1, 29)
# define WIFI_REGEN_GPIO   GPIO_TO_PIN(1, 16)
# define WIFI_RST_GPIO     GPIO_TO_PIN(0, 27)
# define ZIGBEE_RST_GPIO   GPIO_TO_PIN(3, 18)
# define BIDCOS_RST_GPIO   GPIO_TO_PIN(0, 12)
# define ENOC_RST_GPIO     GPIO_TO_PIN(1, 22)

#else

# define LED_PWR_BL_GPIO   GPIO_TO_PIN(0, 22)
# define LED_PWR_RD_GPIO   GPIO_TO_PIN(0, 23)
# define LED_LAN_BL_GPIO   GPIO_TO_PIN(1, 17)
# define LED_LAN_RD_GPIO   GPIO_TO_PIN(0, 26)
# define LED_CLOUD_BL_GPIO GPIO_TO_PIN(1, 18)
# define LED_CLOUD_RD_GPIO GPIO_TO_PIN(2, 2)
# define LED_PWM_GPIO      GPIO_TO_PIN(1, 19)
# define RESET_GPIO        GPIO_TO_PIN(1, 29)
# define WIFI_REGEN_GPIO   GPIO_TO_PIN(1, 16)
# define WIFI_RST_GPIO     GPIO_TO_PIN(0, 27)
# define ZIGBEE_RST_GPIO   GPIO_TO_PIN(3, 18)
# define BIDCOS_RST_GPIO   GPIO_TO_PIN(1, 24)
# define Z_WAVE_RST_GPIO   GPIO_TO_PIN(1, 21)
# define ENOC_RST_GPIO     GPIO_TO_PIN(1, 22)

#endif

#define BACK_BUTTON_GPIO    GPIO_TO_PIN(1, 29)
#define FRONT_BUTTON_GPIO   GPIO_TO_PIN(1, 25)

/* Reset is on GPIO pin 29 of GPIO bank 1 */
#define RESET_MASK	(0x1 << 29)

#define HDR_MAGIC	0x43485342
#define HDR_ETH_ALEN	6
#define HDR_NAME_LEN	8
#define HDR_REV_LEN	8
#define HDR_SER_LEN	16
#define HDR_ROOT_LEN	12
#define HDR_FATC_LEN	12

/*
* SHC parameters held in On-Board I²C EEPROM device.
*
* Header Format
*
*  Name     Size   Contents
*-------------------------------------------------------------
*  Magic     4     0x42 0x53 0x48 0x43  [BSHC]
*
*  Version   2     0x0100 for v1.0
*
*  Lenght    2     The length of the complete structure, not only this header
*
*  Eth-MAC   6     Ethernet MAC Address
*                  SHC Pool: 7C:AC:B2:00:10:01 - TBD
*
*  --- Further values follow, not important for Bootloader ---
*/

struct  shc_eeprom {
	u32  magic;
	u16  version;
	u16  lenght;
	uint8_t mac_addr[HDR_ETH_ALEN];
};

void enable_uart0_pin_mux(void);
void enable_shc_board_pin_mux(void);
void enable_shc_board_pwm_pin_mux(void);

#endif
