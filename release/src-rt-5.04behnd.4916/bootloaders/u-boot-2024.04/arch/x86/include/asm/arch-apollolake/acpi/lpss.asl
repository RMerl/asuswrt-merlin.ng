/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Intel Corp.
 * (Written by Lance Zhao <lijian.zhao@intel.com> for Intel Corp.)
 */

scope (\_SB.PCI0) {

	/* LPIO1 PWM */
	Device(PWM) {
		Name (_ADR, 0x001A0000)
		Name (_DDN, "Intel(R) PWM Controller")
	}

	/* LPIO1 HS-UART #1 */
	Device(URT1) {
		Name (_ADR, 0x00180000)
		Name (_DDN, "Intel(R) HS-UART Controller #1")
	}

	/* LPIO1 HS-UART #2 */
	Device(URT2) {
		Name (_ADR, 0x00180001)
		Name (_DDN, "Intel(R) HS-UART Controller #2")
	}

	/* LPIO1 HS-UART #3 */
	Device(URT3) {
		Name (_ADR, 0x00180002)
		Name (_DDN, "Intel(R) HS-UART Controller #3")
	}

	/* LPIO1 HS-UART #4 */
	Device(URT4) {
		Name (_ADR, 0x00180003)
		Name (_DDN, "Intel(R) HS-UART Controller #4")
	}

	/* LPIO1 SPI */
	Device(SPI1) {
		Name (_ADR, 0x00190000)
		Name (_DDN, "Intel(R) SPI Controller #1")
	}

	/* LPIO1 SPI #2 */
	Device(SPI2) {
		Name (_ADR, 0x00190001)
		Name (_DDN, "Intel(R) SPI Controller #2")
	}

	/* LPIO1 SPI #3 */
	Device(SPI3) {
		Name (_ADR, 0x00190002)
		Name (_DDN, "Intel(R) SPI Controller #3")
	}


	/* LPIO2 I2C #0 */
	Device(I2C0) {
		Name (_ADR, 0x00160000)
		Name (_DDN, "Intel(R) I2C Controller #0")
	}

	/* LPIO2 I2C #1 */
	Device(I2C1) {
		Name (_ADR, 0x00160001)
		Name (_DDN, "Intel(R) I2C Controller #1")
	}

	/* LPIO2 I2C #2 */
	Device(I2C2) {
		Name (_ADR, 0x00160002)
		Name (_DDN, "Intel(R) I2C Controller #2")
	}

	/* LPIO2 I2C #3 */
	Device(I2C3) {
		Name (_ADR, 0x00160003)
		Name (_DDN, "Intel(R) I2C Controller #3")
	}

	/* LPIO2 I2C #4 */
	Device(I2C4) {
		Name (_ADR, 0x00170000)
		Name (_DDN, "Intel(R) I2C Controller #4")
	}

	/* LPIO2 I2C #5 */
	Device(I2C5) {
		Name (_ADR, 0x00170001)
		Name (_DDN, "Intel(R) I2C Controller #5")
	}

	/* LPIO2 I2C #6 */
	Device(I2C6) {
		Name (_ADR, 0x00170002)
		Name (_DDN, "Intel(R) I2C Controller #6")
	}

	/* LPIO2 I2C #7 */
	Device(I2C7) {
		Name (_ADR, 0x00170003)
		Name (_DDN, "Intel(R) I2C Controller #7")
	}
}
