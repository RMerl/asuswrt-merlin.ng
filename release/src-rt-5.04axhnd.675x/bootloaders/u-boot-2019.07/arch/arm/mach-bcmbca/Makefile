# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2019 Broadcom Ltd
#

obj-y += mmu_setup.o pinmux.o lowlevel_init.o
obj-$(CONFIG_OF_SYSTEM_SETUP) += bcmbca-dtsetup.o dt_helper.o
obj-$(CONFIG_$(SPL_TPL_)STACKPROTECTOR) += stackprot.o

obj-$(CONFIG_BCM63158) += bcm63158/
obj-$(CONFIG_BCM63146) += bcm63146/
obj-$(CONFIG_BCM6858) += bcm6858/
obj-$(CONFIG_BCM6856) += bcm6856/
obj-$(CONFIG_BCM63178) += bcm63178/
obj-$(CONFIG_BCM47622) += bcm47622/
obj-$(CONFIG_BCM6756) += bcm6756/
obj-$(CONFIG_BCM4908) += bcm4908/
obj-$(CONFIG_BCM4912) += bcm4912/
obj-$(CONFIG_BCM6813) += bcm6813/
obj-$(CONFIG_BCM63138) += bcm63138/
obj-$(CONFIG_BCM63148) += bcm63148/
obj-$(CONFIG_BCM6846) += bcm6846/
obj-$(CONFIG_BCM6878) += bcm6878/
obj-$(CONFIG_BCM6855) += bcm6855/

obj-$(CONFIG_BCMBCA_PMC) += pmc/
obj-$(CONFIG_BCMBCA_UBUS) += ubus/
obj-$(CONFIG_BCMBCA_OTP) += otp/
obj-$(CONFIG_BCMBCA_RNG) += rng/

ifndef CONFIG_SPL_BUILD
# u-boot proper
obj-$(CONFIG_BCMBCA_RDP) += rdp/
obj-$(CONFIG_BCMBCA_XRDP_146) += xrdp/

else # SPL_BUILD

# TPL and SPL
obj-$(CONFIG_SPL_BUILD) += bcm_fdtdec.o
obj-$(CONFIG_NAND) += bcmbca_nand_spl.o
obj-$(CONFIG_BCMBCA_BOARD_TK_PROG) += otp_tk/

ifndef CONFIG_TPL_BUILD
# SPL ONLY
ifdef CONFIG_BCMBCA_DDRC
obj-$(CONFIG_SPL_BUILD)	+= spl_ddrinit.o
ifdef CONFIG_BCMBCA_DDR_REGINIT
obj-$(CONFIG_SPL_BUILD) += ddrinit_reg.o
else # BCMBCA_DDR_REGINIT
ifdef CONFIG_BCMBCA_DPFE
obj-$(CONFIG_SPL_BUILD)	+= ddrinit_dpfe.o
subdir-$(CONFIG_SPL_BUILD) += bcmbca_dpfe
else # BCMBCA_DPFE
subdir-$(CONFIG_SPL_BUILD) += bcmbca_ddr
endif # BCMBCA_DPFE
endif # BCMBCA_DDR_REGINIT
endif # BCMBCA_DDRC

endif # TPL_BUILD

endif # SPL_BUILD
