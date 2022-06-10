/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file dirauth_options.inc
 * @brief Declare configuration options for a single hidden service.
 *
 * Note that this options file behaves differently from most, since it
 * is not used directly by the options manager.  Instead, it is applied to
 * a group of hidden service options starting with a HiddenServiceDir and
 * extending up to the next HiddenServiceDir.
 **/

/** Holds configuration for a single hidden service. */
BEGIN_CONF_STRUCT(hs_opts_t)

CONF_VAR(HiddenServiceDir, FILENAME, 0, NULL)
CONF_VAR(HiddenServiceDirGroupReadable, BOOL, 0, "0")
CONF_VAR(HiddenServicePort, LINELIST, 0, NULL)
// "-1" means "auto" here.
CONF_VAR(HiddenServiceVersion, INT, 0, "-1")
CONF_VAR(HiddenServiceAllowUnknownPorts, BOOL, 0, "0")
CONF_VAR(HiddenServiceMaxStreams, POSINT, 0, "0")
CONF_VAR(HiddenServiceMaxStreamsCloseCircuit, BOOL, 0, "0")
CONF_VAR(HiddenServiceNumIntroductionPoints, POSINT, 0, "3")
CONF_VAR(HiddenServiceExportCircuitID, STRING, 0, NULL)
CONF_VAR(HiddenServiceEnableIntroDoSDefense, BOOL, 0, "0")
CONF_VAR(HiddenServiceEnableIntroDoSRatePerSec, POSINT, 0, "25")
CONF_VAR(HiddenServiceEnableIntroDoSBurstPerSec, POSINT, 0, "200")
CONF_VAR(HiddenServiceOnionBalanceInstance, BOOL, 0, "0")

END_CONF_STRUCT(hs_opts_t)
