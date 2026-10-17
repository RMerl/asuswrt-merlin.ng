/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018 ABB
 * Valentin Longchamp <valentin.longchamp@hitachi-powergrids.com>
 */

#ifndef __QRIO_H
#define __QRIO_H

/* QRIO GPIO ports */
#define QRIO_GPIO_A		0x40
#define QRIO_GPIO_B		0x60

void show_qrio(void);
bool qrio_get_selftest_pin(void);
bool qrio_get_pgy_pres_pin(void);
int qrio_get_gpio(u8 port_off, u8 gpio_nr);
void qrio_set_opendrain_gpio(u8 port_off, u8 gpio_nr, u8 val);
void qrio_set_gpio(u8 port_off, u8 gpio_nr, bool value);
void qrio_gpio_direction_output(u8 port_off, u8 gpio_nr, bool value);
void qrio_gpio_direction_input(u8 port_off, u8 gpio_nr);

/* QRIO Periphery reset configurations */
#define PRSTCFG_POWUP_UNIT_CORE_RST	0x0
#define PRSTCFG_POWUP_UNIT_RST		0x1
#define PRSTCFG_POWUP_RST		0x3

void qrio_prst(u8 bit, bool en, bool wden);
void qrio_wdmask(u8 bit, bool wden);
void qrio_prstcfg(u8 bit, u8 mode);
void qrio_set_leds(void);
void qrio_enable_app_buffer(void);
void qrio_cpuwd_flag(bool flag);
bool qrio_reason_unitrst(void);

/* QRIO uP reset request configurations */
#define UPREQ_UNIT_RST		0x0
#define UPREQ_CORE_RST		0x1

void qrio_uprstreq(u8 mode);

void i2c_deblock_gpio_cfg(void);
#endif /* __QRIO_H */
