# SPDX-License-Identifier: GPL-2.0+

libs-y += arch/arc/cpu/$(CPU)/
libs-y += arch/arc/lib/

# MetaWare debugger doesn't support PIE (position-independent executable)
# so the only way to load U-Boot in MDB is to fake it by:
#   1. Reset PIE flag in ELF header
#   2. Strip all debug information from elf
ifdef CONFIG_SYS_LITTLE_ENDIAN
	EXEC_TYPE_OFFSET=16
else
	EXEC_TYPE_OFFSET=17
endif

mdbtrick: u-boot
	$(Q)printf '\x02' | dd of=u-boot bs=1 seek=$(EXEC_TYPE_OFFSET) count=1 \
		conv=notrunc &> /dev/null
	$(Q)$(CROSS_COMPILE)strip -g u-boot
