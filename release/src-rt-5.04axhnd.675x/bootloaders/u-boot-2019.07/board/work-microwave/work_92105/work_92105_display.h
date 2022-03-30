/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * work_92105 display support interface
 *
 * (C) Copyright 2014  DENX Software Engineering GmbH
 * Written-by: Albert ARIBAUD <albert.aribaud@3adev.fr>
 *
 * The work_92105 display is a HD44780-compatible module
 * controlled through a MAX6957AAX SPI port expander, two
 * MAX518 I2C DACs and native LPC32xx GPO 15.
 */

void work_92105_display_init(void);
