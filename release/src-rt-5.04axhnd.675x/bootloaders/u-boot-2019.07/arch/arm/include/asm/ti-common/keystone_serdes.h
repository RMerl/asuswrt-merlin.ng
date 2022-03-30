/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Texas Instruments Keystone SerDes driver
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef __TI_KEYSTONE_SERDES_H__
#define __TI_KEYSTONE_SERDES_H__

/* SERDES Reference clock */
enum ks2_serdes_clock {
	SERDES_CLOCK_100M,		/* 100 MHz */
	SERDES_CLOCK_122P88M,		/* 122.88 MHz */
	SERDES_CLOCK_125M,		/* 125 MHz */
	SERDES_CLOCK_156P25M,		/* 156.25 MHz */
	SERDES_CLOCK_312P5M,		/* 312.5 MHz */
};

/* SERDES Lane Baud Rate */
enum ks2_serdes_rate {
	SERDES_RATE_4P9152G,		/* 4.9152 GBaud */
	SERDES_RATE_5G,			/* 5 GBaud */
	SERDES_RATE_6P144G,		/* 6.144 GBaud */
	SERDES_RATE_6P25G,		/* 6.25 GBaud */
	SERDES_RATE_10p3125g,		/* 10.3215 GBaud */
	SERDES_RATE_12p5g,		/* 12.5 GBaud */
};

/* SERDES Lane Rate Mode */
enum ks2_serdes_rate_mode {
	SERDES_FULL_RATE,
	SERDES_HALF_RATE,
	SERDES_QUARTER_RATE,
};

/* SERDES PHY TYPE */
enum ks2_serdes_interface {
	SERDES_PHY_SGMII,
	SERDES_PHY_PCSR,		/* XGE SERDES */
};

struct ks2_serdes {
	enum ks2_serdes_clock clk;
	enum ks2_serdes_rate rate;
	enum ks2_serdes_rate_mode rate_mode;
	enum ks2_serdes_interface intf;
	u32 loopback;
};

int ks2_serdes_init(u32 base, struct ks2_serdes *serdes, u32 num_lanes);

#endif /* __TI_KEYSTONE_SERDES_H__ */
