# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2006-2009
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

ifeq ($(CONFIG_$(SPL_TPL_)DM_SERIAL),y)
obj-y += serial-uclass.o
else
obj-y += serial.o
endif

obj-$(CONFIG_PL01X_SERIAL) += serial_pl01x.o
obj-$(CONFIG_PL011_SERIAL) += serial_pl01x.o
obj-$(CONFIG_$(SPL_)SYS_NS16550_SERIAL) += serial_ns16550.o

obj-$(CONFIG_ALTERA_UART) += altera_uart.o
obj-$(CONFIG_ALTERA_JTAG_UART) += altera_jtag_uart.o
obj-$(CONFIG_AR933X_UART) += serial_ar933x.o
obj-$(CONFIG_ARM_DCC) += arm_dcc.o
obj-$(CONFIG_ATMEL_USART) += atmel_usart.o
obj-$(CONFIG_BCM6345_SERIAL) += serial_bcm6345.o
obj-$(CONFIG_COREBOOT_SERIAL) += serial_coreboot.o
obj-$(CONFIG_CORTINA_UART) += serial_cortina.o
obj-$(CONFIG_DEBUG_SBI_CONSOLE) += serial_sbi.o
obj-$(CONFIG_EFI_APP) += serial_efi.o
obj-$(CONFIG_MCFUART) += serial_mcf.o
obj-$(CONFIG_SYS_NS16550) += ns16550.o
obj-$(CONFIG_S5P_SERIAL) += serial_s5p.o
obj-$(CONFIG_MXC_UART) += serial_mxc.o
obj-$(CONFIG_MESON_SERIAL) += serial_meson.o
obj-$(CONFIG_INTEL_MID_SERIAL) += serial_intel_mid.o
obj-$(CONFIG_ROCKCHIP_SERIAL) += serial_rockchip.o
obj-$(CONFIG_XILINX_UARTLITE) += serial_xuartlite.o
obj-$(CONFIG_SANDBOX_SERIAL) += sandbox.o
obj-$(CONFIG_SCIF_CONSOLE) += serial_sh.o
obj-$(CONFIG_SEMIHOSTING_SERIAL) += serial_semihosting.o
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
obj-$(CONFIG_MSM_GENI_SERIAL) += serial_msm_geni.o
obj-$(CONFIG_MXS_AUART_SERIAL) += serial_mxs.o
obj-$(CONFIG_MVEBU_A3700_UART) += serial_mvebu_a3700.o
obj-$(CONFIG_MPC8XX_CONS) += serial_mpc8xx.o
obj-$(CONFIG_NULLDEV_SERIAL) += serial_nulldev.o
obj-$(CONFIG_NPCM_SERIAL) += serial_npcm.o
obj-$(CONFIG_OCTEON_SERIAL_BOOTCMD) += serial_octeon_bootcmd.o
obj-$(CONFIG_OCTEON_SERIAL_PCIE_CONSOLE) += serial_octeon_pcie_console.o
obj-$(CONFIG_OWL_SERIAL) += serial_owl.o
obj-$(CONFIG_OMAP_SERIAL) += serial_omap.o
obj-$(CONFIG_MTK_SERIAL) += serial_mtk.o
obj-$(CONFIG_MT7620_SERIAL) += serial_mt7620.o
obj-$(CONFIG_HTIF_CONSOLE) += serial_htif.o
obj-$(CONFIG_SIFIVE_SERIAL) += serial_sifive.o
obj-$(CONFIG_XEN_SERIAL) += serial_xen.o
obj-$(CONFIG_S5P4418_PL011_SERIAL) += serial_s5p4418_pl011.o

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_USB_TTY) += usbtty.o
endif
