/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright 2019 NXP
 */

#ifndef SC_SECO_API_H
#define SC_SECO_API_H

/* Includes */

#include <firmware/imx/sci/types.h>

/* Defines */
#define SC_SECO_AUTH_CONTAINER          0U   /* Authenticate container */
#define SC_SECO_VERIFY_IMAGE            1U   /* Verify image */
#define SC_SECO_REL_CONTAINER           2U   /* Release container */
#define SC_SECO_AUTH_SECO_FW            3U   /* SECO Firmware */
#define SC_SECO_AUTH_HDMI_TX_FW         4U   /* HDMI TX Firmware */
#define SC_SECO_AUTH_HDMI_RX_FW         5U   /* HDMI RX Firmware */
#define SC_SECO_EVERIFY_IMAGE           6U   /* Enhanced verify image */

#define SC_SECO_RNG_STAT_UNAVAILABLE    0U  /* Unable to initialize the RNG */
#define SC_SECO_RNG_STAT_INPROGRESS     1U  /* Initialization is on-going */
#define SC_SECO_RNG_STAT_READY          2U  /* Initialized */

/* Types */

/*
 * This type is used to issue SECO authenticate commands.
 */
typedef u8 sc_seco_auth_cmd_t;

/*
 * This type is used to return the RNG initialization status.
 */
typedef u32 sc_seco_rng_stat_t;

#endif /* SC_SECO_API_H */
