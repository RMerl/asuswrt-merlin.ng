/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __asn_i2c_h
#define __asn_i2c_h

struct sandbox_i2c_priv {
	bool test_mode;
};

/**
 * struct i2c_emul_uc_plat - information about the emulator for this device
 *
 * This is used by devices in UCLASS_I2C_EMUL to record information about the
 * device being emulated. It is accessible with dev_get_uclass_plat()
 *
 * @dev: Device being emulated
 * @idx: of-platdata index, set up by the device's bind() method if of-platdata
 *	is in use
 */
struct i2c_emul_uc_plat {
	struct udevice *dev;
	int idx;
};

#endif /* __asn_i2c_h */
