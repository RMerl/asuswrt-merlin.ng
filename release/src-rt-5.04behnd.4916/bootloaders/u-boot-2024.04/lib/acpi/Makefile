# SPDX-License-Identifier: GPL-2.0+
#

obj-y += acpi.o

ifdef CONFIG_$(SPL_TPL_)GENERATE_ACPI_TABLE

obj-$(CONFIG_$(SPL_)ACPIGEN) += acpigen.o
obj-$(CONFIG_$(SPL_)ACPIGEN) += acpi_device.o
obj-$(CONFIG_$(SPL_)ACPIGEN) += acpi_dp.o
obj-$(CONFIG_$(SPL_)ACPIGEN) += acpi_table.o
obj-y += acpi_writer.o

# With QEMU the ACPI tables come from there, not from U-Boot
ifndef CONFIG_QFW_ACPI
obj-y += base.o
obj-y += csrt.o
obj-y += mcfg.o

# Sandbox does not build a .asl file
ifndef CONFIG_SANDBOX
obj-y += dsdt.o
endif

obj-y += facs.o
obj-y += ssdt.o
endif

endif # GENERATE_ACPI_TABLE
