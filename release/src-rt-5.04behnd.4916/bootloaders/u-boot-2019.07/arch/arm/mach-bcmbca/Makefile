# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2019 Broadcom Ltd
#

obj-y += mmu_setup.o pinmux.o lowlevel_init.o
CFLAGS_bcmbca-dtsetup.o := $(if $(BUILD_TAG),-DBUILD_TAG='"$(BUILD_TAG)"')
obj-$(CONFIG_OF_SYSTEM_SETUP) += bcmbca-dtsetup.o dt_helper.o
obj-$(CONFIG_$(SPL_TPL_)STACKPROTECTOR) += stackprot.o

obj-$(CONFIG_BCM63158) += bcm63158/
obj-$(CONFIG_BCM63146) += bcm63146/
obj-$(CONFIG_BCM6858) += bcm6858/
obj-$(CONFIG_BCM6856) += bcm6856/
obj-$(CONFIG_BCM63178) += bcm63178/
obj-$(CONFIG_BCM47622) += bcm47622/
obj-$(CONFIG_BCM6756) += bcm6756/
obj-$(CONFIG_BCM6765) += bcm6765/
obj-$(CONFIG_BCM6766) += bcm6766/
obj-$(CONFIG_BCM4908) += bcm4908/
obj-$(CONFIG_BCM4912) += bcm4912/
obj-$(CONFIG_BCM6813) += bcm6813/
obj-$(CONFIG_BCM63138) += bcm63138/
obj-$(CONFIG_BCM63148) += bcm63148/
obj-$(CONFIG_BCM6846) += bcm6846/
obj-$(CONFIG_BCM6878) += bcm6878/
obj-$(CONFIG_BCM6855) += bcm6855/
obj-$(CONFIG_BCM6888) += bcm6888/
obj-$(CONFIG_BCM6837) += bcm6837/
obj-$(CONFIG_BCM68880) += bcm68880/

obj-$(CONFIG_BCMBCA_PMC) += pmc/
obj-$(CONFIG_BCMBCA_UBUS4) += ubus/
obj-$(CONFIG_BCMBCA_OTP) += otp/
obj-$(CONFIG_BCMBCA_RNG) += rng/
obj-$(CONFIG_BCMBCA_STRAP) += strap/
obj-$(CONFIG_BCMBCA_ITC_RPC) += itc_rpc/
obj-$(CONFIG_BCMBCA_VFBIO) += vfbio.o

ifndef CONFIG_SPL_BUILD
# u-boot proper
obj-$(CONFIG_BCMBCA_RDP) += rdp/

else # SPL_BUILD

# TPL and SPL
obj-$(CONFIG_SPL_BUILD) += bcm_fdtdec.o
obj-$(CONFIG_NAND) += bcmbca_nand_spl.o
obj-$(CONFIG_BCMBCA_BOARD_TK_PROG) += otp_tk/

ifndef CONFIG_TPL_BUILD
# SPL ONLY
ifndef CONFIG_SMC_BASED
obj-$(CONFIG_SPL_BUILD)	+= bcmbca_swreg_spl.o
endif
ifdef CONFIG_BCMBCA_DDRC
obj-$(CONFIG_SPL_BUILD)	+= spl_ddrinit.o
ifdef CONFIG_BCMBCA_DDR_REGINIT
obj-$(CONFIG_SPL_BUILD) += ddrinit_reg.o
else # BCMBCA_DDR_REGINIT
ifdef CONFIG_BCMBCA_DPFE
obj-$(CONFIG_SPL_BUILD)	+= ddrinit_dpfe.o
endif # BCMBCA_DPFE
endif # BCMBCA_DDR_REGINIT
endif # BCMBCA_DDRC

endif # TPL_BUILD

endif # SPL_BUILD
