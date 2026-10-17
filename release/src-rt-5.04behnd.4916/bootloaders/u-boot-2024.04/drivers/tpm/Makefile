# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2011 The Chromium OS Authors. All rights reserved.

obj-$(CONFIG_$(SPL_TPL_)TPM) += tpm-uclass.o

obj-$(CONFIG_TPM_ATMEL_TWI) += tpm_atmel_twi.o
obj-$(CONFIG_TPM_TIS_INFINEON) += tpm_tis_infineon.o
obj-$(CONFIG_TPM_TIS_LPC) += tpm_tis_lpc.o
obj-$(CONFIG_TPM_TIS_SANDBOX) += tpm_tis_sandbox.o sandbox_common.o
obj-$(CONFIG_TPM_ST33ZP24_I2C) += tpm_tis_st33zp24_i2c.o
obj-$(CONFIG_TPM_ST33ZP24_SPI) += tpm_tis_st33zp24_spi.o

obj-$(CONFIG_$(SPL_TPL_)TPM2_CR50_I2C) += cr50_i2c.o
obj-$(CONFIG_TPM2_TIS_SANDBOX) += tpm2_tis_sandbox.o sandbox_common.o
obj-$(CONFIG_TPM2_TIS_SPI) += tpm2_tis_core.o tpm2_tis_spi.o
obj-$(CONFIG_TPM2_TIS_I2C) += tpm2_tis_core.o tpm2_tis_i2c.o
obj-$(CONFIG_TPM2_FTPM_TEE) += tpm2_ftpm_tee.o
obj-$(CONFIG_TPM2_MMIO) += tpm2_tis_core.o tpm2_tis_mmio.o
