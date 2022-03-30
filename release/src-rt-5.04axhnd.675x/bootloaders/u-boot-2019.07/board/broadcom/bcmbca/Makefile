# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2019 Broadcom Ltd
#


# ccflags-y += -save-temps

ifndef CONFIG_SPL_BUILD
# u-boot proper
CFLAGS_board.o := $(if $(BUILD_TAG),-DBUILD_TAG='"$(BUILD_TAG)"')
obj-y += board.o
obj-y += spl_env.o
obj-$(CONFIG_BCMBCA_BOARD_SDK) += board_sdk.o
obj-y += bca_common.o
obj-$(CONFIG_BCMBCA_BOARD_SDK) += sdk_test_commands.o
obj-$(CONFIG_BCMBCA_PMC) += pmc_commands.o
obj-$(CONFIG_BCMBCA_HTTPD) += httpd/
obj-$(CONFIG_BCM_BOOTSTATE) += bcm_bootstate.o 
obj-$(CONFIG_BCMBCA_BOARD_SDK) += board_secure.o
obj-$(CONFIG_BCMBCA_BOARD_SDK) += board_secure_sdk.o
obj-$(CONFIG_BCMBCA_BOARD_SDK) += board_secure_utils.o
obj-$(CONFIG_BCMBCA_BOARD_SDK) += board_secure_fit.o
obj-$(CONFIG_BCMBCA_BOARD_SDK) += mini-gmp/
obj-$(CONFIG_BCMBCA_BUTTON) += bcmbca_button.o reset_button.o
obj-y += $(subst ",,$(CONFIG_BCMBCA_EXTRA_BOARD_OBJECTS))
endif

ifndef CONFIG_TPL_BUILD
# SPL
ifndef CONFIG_BCMBCA_BOARD_TK_PROG
# Standard SPL
obj-$(CONFIG_SPL_BUILD)	+= boot_blob.o
obj-$(CONFIG_SPL_BUILD)	+= boot_flash.o
obj-$(CONFIG_SPL_BUILD)	+= board_spl.o
CFLAGS_board_spl.o := $(if $(BUILD_TAG),-DBUILD_TAG='"$(BUILD_TAG)"')
obj-$(CONFIG_SPL_BUILD)	+= spl_env.o
obj-$(CONFIG_SPL_BUILD)	+= early_abort.o
obj-$(CONFIG_SPL_BUILD) += board_secure.o
obj-$(CONFIG_SPL_BUILD) += board_secure_spl.o
else
# TK SPL
ifdef CONFIG_BCMBCA_BOARD_TK_PROG
obj-$(CONFIG_SPL_BUILD)	+= board_tk_prog.o
obj-$(CONFIG_SPL_BUILD) += board_secure.o
obj-$(CONFIG_SPL_BUILD) += board_secure_utils.o
obj-$(CONFIG_SPL_BUILD) += board_secure_spl.o
obj-$(CONFIG_SPL_BUILD) += mini-gmp/
endif
endif
endif

# TPL
obj-$(CONFIG_TPL_BUILD)	+= board_tpl.o
CFLAGS_board_tpl.o := $(if $(BUILD_TAG),-DBUILD_TAG='"$(BUILD_TAG)"')
obj-$(CONFIG_TPL_BUILD)	+= bca_common.o
obj-$(CONFIG_TPL_BUILD)	+= spl_env.o
bootstate-$(CONFIG_BCM_BOOTSTATE) += bcm_bootstate.o
obj-$(CONFIG_TPL_BUILD)	+= $(bootstate-y)
obj-$(CONFIG_BCMBCA_IKOS) += board_ikos.o
obj-$(CONFIG_TPL_BUILD) += board_secure.o
obj-$(CONFIG_TPL_BUILD) += board_secure_tpl.o
obj-$(CONFIG_TPL_BUILD) += board_secure_fit.o
obj-$(CONFIG_TPL_BUILD) += board_secure_utils.o
obj-$(CONFIG_TPL_BUILD) += mini-gmp/
