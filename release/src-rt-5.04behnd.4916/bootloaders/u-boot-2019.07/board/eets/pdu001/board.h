/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * board.h
 *
 * EETS GmbH PDU001 board information header
 *
 * Copyright (C) 2018 EETS GmbH - http://www.eets.ch/
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * We have two pin mux functions that must exist. First we need I2C0 to
 * access the TPS65910 PMIC located on the M2 computing module.
 * Second, if we want low-level debugging or a early UART (ie. before the
 * pin controller driver is running), we need one of the UART ports UART0 to
 * UART5 (usually UART3 since it is wired to K2).
 * In case of I2C0 access we explicitly don't rely on the the ROM but we could
 * do so as we use the primary mode (mode 0) for I2C0.
 * All other multiplexing and pin configuration is done by the DT once it
 * gets parsed by the pin controller driver.
 * However we relay on the ROM to configure the pins of MMC0 (eMMC) as well
 * as MMC1 (microSD card-cage) since these are our boot devices.
 */
void enable_uart0_pin_mux(void);
void enable_uart1_pin_mux(void);
void enable_uart2_pin_mux(void);
void enable_uart3_pin_mux(void);
void enable_uart4_pin_mux(void);
void enable_uart5_pin_mux(void);
void enable_uart_pin_mux(u32 addr);
void enable_i2c0_pin_mux(void);

#endif
