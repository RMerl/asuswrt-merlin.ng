/* SPDX-License-Identifier: GPL-2.0+
  *
  * Copyright 2019 Broadcom Ltd.
 */

#ifndef _ITC_CHANNEL_DEFS_H_
#define _ITC_CHANNEL_DEFS_H_

/* tunnel indicies */
enum rpc_tunnel_idx {
	RPC_TUNNEL_ARM_SMC_NS = 0,	    /* ARM<->SMC NS */
	RPC_TUNNEL_VFLASH_SMC_NS = 1,	/* VFLASH<->SMC NS */
	RPC_TUNNEL_AVS_SMC_NS = 2,	    /* AVS<->SMC NS */
	RPC_TUNNEL_MAX	       	    /* Total number of supported tunnels */
};

/* service indicies */
enum rpc_svc_idx {
	RPC_SERVICE_INIT = 0,	/* special channel init and handshake messages */
	RPC_SERVICE_MISC,	/* misc. messages/services */
	RPC_SERVICE_FLASH_MTD,	/* vFlash MTD */
	RPC_SERVICE_FLASH = RPC_SERVICE_FLASH_MTD,
	RPC_SERVICE_NONVOL,	/* Split mode nonvol support */
	RPC_SERVICE_EROUTER,	/* erouter - Linux user space */
	RPC_SERVICE_EROUTER_PM,	/* erouter - Linux user space power management */
	RPC_SERVICE_WPS_CTL,	/* wireless reset button */
	RPC_SERVICE_BASH_USER,	/* Linux bash shell service */
	RPC_SERVICE_TEST_USER,	/* Linux user space test service */
	RPC_SERVICE_GPIO,	/* GPIO */
	RPC_SERVICE_PINCTRL,	/* Pinctrl */
	RPC_SERVICE_LED,	/* LED */
	RPC_SERVICE_FLASH_BIO,	/* Flash block I/O */
	RPC_SERVICE_PWR,	/* Power Domains */
	RPC_SERVICE_CLK,	/* Clock Domains */
	RPC_SERVICE_SYS,	/* System-wide Info & Control */
	RPC_SERVICE_BBS,	/* BBS Control */
	RPC_SERVICE_BA,		/* Boot Assist */
	RPC_SERVICE_AVS,	/* Adaptive Voltage Scaling service */
	RPC_SERVICE_UBCAP,	/* UBUS Capture */
	RPC_SERVICE_FPM,	/* FPM */
	RPC_SERVICE_RTC,	/* Real-Time Clock */
	RPC_SERVICE_SE,     /* Secure Element (RSVD1) */
	RPC_SERVICE_CR,     /* Challenge-Response (RSVD2) */
	RPC_SERVICE_ALERT,  /* Alert */
	RPC_SERVICE_OTP,    /* One-Time Programmable */
	RPC_SERVICE_BP3,    /* BP3 */
	RPC_MAX_SERVICES	/* Total number of supported services */
};

#endif
