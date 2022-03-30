# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2006-2009
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

ifdef CONFIG_SPL_BUILD

ifeq ($(CONFIG_$(SPL_TPL_)BUILD)$(CONFIG_$(SPL_TPL_)DM_SERIAL),yy)
obj-y += serial-uclass.o
else
obj-y += serial.o
endif

else

ifdef CONFIG_DM_SERIAL
obj-y += serial-uclass.o
else
obj-y += serial.o
endif

endif

ifdef CONFIG_DM_SERIAL
obj-$(CONFIG_PL01X_SERIAL) += serial_pl01x.o
else
obj-$(CONFIG_PL010_SERIAL) += serial_pl01x.o
obj-$(CONFIG_PL011_SERIAL) += serial_pl01x.o
obj-$(CONFIG_SYS_NS16550_SERIAL) += serial_ns16550.o
endif

obj-$(CONFIG_ALTERA_UART) += altera_uart.o
obj-$(CONFIG_ALTERA_JTAG_UART) += altera_jtag_uart.o
obj-$(CONFIG_AR933X_UART) += serial_ar933x.o
obj-$(CONFIG_ARM_DCC) += arm_dcc.o
obj-$(CONFIG_ATMEL_USART) += atmel_usart.o
obj-$(CONFIG_BCM6345_SERIAL) += serial_bcm6345.o
obj-$(CONFIG_EFI_APP) += serial_efi.o
obj-$(CONFIG_LPC32XX_HSUART) += lpc32xx_hsuart.o
obj-$(CONFIG_MCFUART) += mcfuart.o
obj-$(CONFIG_SYS_NS16550) += ns16550.o
obj-$(CONFIG_S5P) += serial_s5p.o
obj-$(CONFIG_MXC_UART) += serial_mxc.o
obj-$(CONFIG_PXA_SERIAL) += serial_pxa.o
obj-$(CONFIG_MESON_SERIAL) += serial_meson.o
obj-$(CONFIG_INTEL_MID_SERIAL) += serial_intel_mid.o
ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_ROCKCHIP_SERIAL) += serial_rockchip.o
endif
obj-$(CONFIG_XILINX_UARTLITE) += serial_xuartlite.o
obj-$(CONFIG_SANDBOX_SERIAL) += sandbox.o
obj-$(CONFIG_SCIF_CONSOLE) += serial_sh.o
obj-$(CONFIG_ZYNQ_SERIAL) += serial_zynq.o
obj-$(CONFIG_FSL_LPUART) += serial_lpuart.o
obj-$(CONFIG_FSL_LINFLEXUART) += serial_linflexuart.o
obj-$(CONFIG_ARC_SERIAL) += serial_arc.o
obj-$(CONFIG_UNIPHIER_SERIAL) += serial_uniphier.o
obj-$(CONFIG_STM32_SERIAL) += serial_stm32.o
obj-$(CONFIG_STI_ASC_SERIAL) += serial_sti_asc.o
obj-$(CONFIG_PIC32_SERIAL) += serial_pic32.o
obj-$(CONFIG_BCM283X_MU_SERIAL) += serial_bcm283x_mu.o
obj-$(CONFIG_BCM283X_PL011_SERIAL) += serial_bcm283x_pl011.o
obj-$(CONFIG_MSM_SERIAL) += serial_msm.o
obj-$(CONFIG_MVEBU_A3700_UART) += serial_mvebu_a3700.o
obj-$(CONFIG_MPC8XX_CONS) += serial_mpc8xx.o
obj-$(CONFIG_NULLDEV_SERIAL) += serial_nulldev.o
obj-$(CONFIG_OWL_SERIAL) += serial_owl.o
obj-$(CONFIG_OMAP_SERIAL) += serial_omap.o
obj-$(CONFIG_MTK_SERIAL) += serial_mtk.o
obj-$(CONFIG_SIFIVE_SERIAL) += serial_sifive.o

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_USB_TTY) += usbtty.o
endif
