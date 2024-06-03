/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Theobroma Systems Design und Consulting GmbH
 */

#ifndef __RK_HDMI_H__
#define __RK_HDMI_H__

struct rkhdmi_driverdata {
	/* configuration */
	u8 i2c_clk_high;
	u8 i2c_clk_low;
	const char * const *regulator_names;
	u32 regulator_names_cnt;
	/* setters/getters */
	int (*set_input_vop)(struct udevice *dev);
	int (*clk_config)(struct udevice *dev);
};

struct rk_hdmi_priv {
	struct dw_hdmi hdmi;
	void *grf;
};

/**
 * rk_hdmi_read_edid() - read the attached HDMI/DVI monitor's EDID
 *
 * N.B.: The buffer should be large enough to hold 2 EDID blocks, as
 *       this function calls dw_hdmi_read_edid, which ignores buf_size
 *       argument and assumes that there's always enough space for 2
 *       EDID blocks.
 *
 * @dev:	device
 * @buf:	output buffer for the EDID
 * @buf_size:	number of bytes in the buffer
 * @return number of bytes read if OK, -ve if something went wrong
 */
int rk_hdmi_read_edid(struct udevice *dev, u8 *buf, int buf_size);

/**
 * rk_hdmi_probe_regulators() - probe (autoset + enable) regulators
 *
 * Probes a list of regulators by performing autoset and enable
 * operations on them.  The list of regulators is an array of string
 * pointers and any individual regulator-probe may fail without
 * counting as an error.
 *
 * @dev:	device
 * @names:	array of string-pointers to regulator names to probe
 * @cnt:	number of elements in the 'names' array
 */
void rk_hdmi_probe_regulators(struct udevice *dev,
			      const char * const *names, int cnt);
/**
 * rk_hdmi_ofdata_to_platdata() - common ofdata_to_platdata implementation
 *
 * @dev:	device
 * @return 0 if OK, -ve if something went wrong
 */
int rk_hdmi_ofdata_to_platdata(struct udevice *dev);

/**
 * rk_hdmi_probe() - common probe implementation
 *
 * Performs the following, common initialisation steps:
 * 1. checks for HPD (i.e. a HDMI monitor being attached)
 * 2. initialises the Designware HDMI core
 * 3. initialises the Designware HDMI PHY
 *
 * @dev:	device
 * @return 0 if OK, -ve if something went wrong
 */
int rk_hdmi_probe(struct udevice *dev);

#endif
