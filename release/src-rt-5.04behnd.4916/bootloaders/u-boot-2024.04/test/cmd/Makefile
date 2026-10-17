# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2013 Google, Inc
# Copyright 2022-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>

obj-y += cmd_ut_cmd.o

ifdef CONFIG_HUSH_PARSER
obj-$(CONFIG_CONSOLE_RECORD) += test_echo.o
endif
ifdef CONFIG_CONSOLE_RECORD
obj-$(CONFIG_CMD_PAUSE) += test_pause.o
endif
obj-y += exit.o mem.o
obj-$(CONFIG_CMD_ADDRMAP) += addrmap.o
obj-$(CONFIG_CMD_BDI) += bdinfo.o
obj-$(CONFIG_CMD_FDT) += fdt.o
obj-$(CONFIG_CONSOLE_TRUETYPE) += font.o
obj-$(CONFIG_CMD_HISTORY) += history.o
obj-$(CONFIG_CMD_LOADM) += loadm.o
obj-$(CONFIG_CMD_MEM_SEARCH) += mem_search.o
obj-$(CONFIG_CMD_MEMORY) += mem_copy.o
ifdef CONFIG_CMD_PCI
obj-$(CONFIG_CMD_PCI_MPS) += pci_mps.o
endif
obj-$(CONFIG_CMD_PINMUX) += pinmux.o
obj-$(CONFIG_CMD_PWM) += pwm.o
obj-$(CONFIG_CMD_SEAMA) += seama.o
ifdef CONFIG_SANDBOX
obj-$(CONFIG_CMD_MBR) += mbr.o
obj-$(CONFIG_CMD_READ) += rw.o
obj-$(CONFIG_CMD_SETEXPR) += setexpr.o
obj-$(CONFIG_ARM_FFA_TRANSPORT) += armffa.o
endif
obj-$(CONFIG_CMD_TEMPERATURE) += temperature.o
obj-$(CONFIG_CMD_WGET) += wget.o
