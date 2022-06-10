/* Copyright (c) 2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file dos_options.inc
 * @brief Declare configuration options for the DoS module.
 **/

BEGIN_CONF_STRUCT(dos_options_t)

/** Autobool: Is the DoS connection mitigation subsystem enabled? */
CONF_VAR(DoSConnectionEnabled, AUTOBOOL, 0, "auto")

/** Autobool: Is the circuit creation DoS mitigation subsystem enabled? */
CONF_VAR(DoSCircuitCreationEnabled, AUTOBOOL, 0, "auto")

/** Minimum concurrent connection needed from one single address before any
 * defense is used. */
CONF_VAR(DoSCircuitCreationMinConnections, POSINT, 0, "0")

/** Circuit rate used to refill the token bucket. */
CONF_VAR(DoSCircuitCreationRate, POSINT, 0, "0")

/** Maximum allowed burst of circuits. Reaching that value, the address is
 * detected as malicious and a defense might be used. */
CONF_VAR(DoSCircuitCreationBurst, POSINT, 0, "0")

/** When an address is marked as malicious, what defense should be used
 * against it. See the dos_cc_defense_type_t enum. */
CONF_VAR(DoSCircuitCreationDefenseType, INT, 0, "0")

/** For how much time (in seconds) the defense is applicable for a malicious
 * address. A random time delta is added to the defense time of an address
 * which will be between 1 second and half of this value. */
CONF_VAR(DoSCircuitCreationDefenseTimePeriod, INTERVAL, 0, "0")

/** Maximum concurrent connection allowed per address. */
CONF_VAR(DoSConnectionMaxConcurrentCount, POSINT, 0, "0")

/** When an address is reaches the maximum count, what defense should be
 * used against it. See the dos_conn_defense_type_t enum. */
CONF_VAR(DoSConnectionDefenseType, INT, 0, "0")

/** Autobool: Do we refuse single hop client rendezvous? */
CONF_VAR(DoSRefuseSingleHopClientRendezvous, AUTOBOOL, 0, "auto")

/** Allowed burst of client connection allowed per address. */
CONF_VAR(DoSConnectionConnectBurst, POSINT, 0, "0")

/** Allowed rate of client connection allowed per address. */
CONF_VAR(DoSConnectionConnectRate, POSINT, 0, "0")

/** For how much time (in seconds) the connection connect rate defense is
* applicable for a malicious address. A random time delta is added to the
* defense time of an address which will be between 1 second and half of this
* value. */
CONF_VAR(DoSConnectionConnectDefenseTimePeriod, INTERVAL, 0, "0")

END_CONF_STRUCT(dos_options_t)
