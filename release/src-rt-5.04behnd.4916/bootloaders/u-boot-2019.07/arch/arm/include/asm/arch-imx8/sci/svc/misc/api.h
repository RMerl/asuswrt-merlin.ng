/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#ifndef SC_MISC_API_H
#define SC_MISC_API_H

/* Defines for sc_misc_boot_status_t */
#define SC_MISC_BOOT_STATUS_SUCCESS	0U	/* Success */
#define SC_MISC_BOOT_STATUS_SECURITY	1U	/* Security violation */

/* Defines for sc_misc_seco_auth_cmd_t */
#define SC_MISC_SECO_AUTH_SECO_FW	0U   /* SECO Firmware */
#define SC_MISC_SECO_AUTH_HDMI_TX_FW	1U   /* HDMI TX Firmware */
#define SC_MISC_SECO_AUTH_HDMI_RX_FW	2U   /* HDMI RX Firmware */

/* Defines for sc_misc_temp_t */
#define SC_MISC_TEMP			0U	/* Temp sensor */
#define SC_MISC_TEMP_HIGH		1U	/* Temp high alarm */
#define SC_MISC_TEMP_LOW		2U	/* Temp low alarm */

/* Defines for sc_misc_seco_auth_cmd_t */
#define SC_MISC_AUTH_CONTAINER	0U	/* Authenticate container */
#define SC_MISC_VERIFY_IMAGE	1U	/* Verify image */
#define SC_MISC_REL_CONTAINER	2U	/* Release container */

typedef u8 sc_misc_boot_status_t;

#endif /* SC_MISC_API_H */
