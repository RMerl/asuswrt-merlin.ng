/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * @file dirauth_options.inc
 * @brief Declare configuration options for the crypto_ops module.
 **/

/** Holds configuration about our directory authority options. */
BEGIN_CONF_STRUCT(dirauth_options_t)

/** If non-zero, always vote the Fast flag for any relay advertising
 * this amount of capacity or more. */
CONF_VAR(AuthDirFastGuarantee, MEMUNIT, 0, "100 KB")

/** If non-zero, this advertised capacity or more is always sufficient
 * to satisfy the bandwidth requirement for the Guard flag. */
CONF_VAR(AuthDirGuardBWGuarantee, MEMUNIT, 0, "2 MB")

/** Boolean: are we on IPv6?  */
CONF_VAR(AuthDirHasIPv6Connectivity, BOOL, 0, "0")

/** True iff we should list bad exits, and vote for all other exits as
 * good. */
CONF_VAR(AuthDirListBadExits, BOOL, 0, "0")

/** True iff we should list middle-only relays, and vote for all other
 * relays as possibly suitable for other positions. */
CONF_VAR(AuthDirListMiddleOnly, BOOL, 0, "0")

/** Do not permit more than this number of servers per IP address. */
CONF_VAR(AuthDirMaxServersPerAddr, POSINT, 0, "2")

/** Boolean: Do we enforce key-pinning? */
CONF_VAR(AuthDirPinKeys, BOOL, 0, "1")

/** Bool (default: 1): Switch for the shared random protocol. Only
 * relevant to a directory authority. If off, the authority won't
 * participate in the protocol. If on (default), a flag is added to the
 * vote indicating participation. */
CONF_VAR(AuthDirSharedRandomness, BOOL, 0, "1")

/** Bool (default: 1): When testing routerinfos as a directory authority,
 * do we enforce Ed25519 identity match? */
/* NOTE: remove this option someday. */
CONF_VAR(AuthDirTestEd25519LinkKeys, BOOL, 0, "1")

/**
 * Bool (default 1): As an authority, should we launch tests for
 * reachability, and use those results to vote on "Running"?  If 0,
 * we assume that every relay is Running.
 **/
CONF_VAR(AuthDirTestReachability, BOOL, 0, "1")

/** Authority only: key=value pairs that we add to our networkstatus
 * consensus vote on the 'params' line. */
CONF_VAR(ConsensusParams, LINELIST, 0, NULL)

/** Authority only: minimum number of measured bandwidths we must see
 * before we only believe measured bandwidths to assign flags. */
CONF_VAR(MinMeasuredBWsForAuthToIgnoreAdvertised, INT, 0, "500")

/** As directory authority, accept hidden service directories after what
 * time? */
CONF_VAR(MinUptimeHidServDirectoryV2, INTERVAL, 0, "96 hours")

/** Which versions of tor should we tell users to run? */
CONF_VAR(RecommendedVersions, LINELIST, 0, NULL)

/** Which versions of tor should we tell users to run on clients? */
CONF_VAR(RecommendedClientVersions, LINELIST, 0, NULL)

/** Which versions of tor should we tell users to run on relays? */
CONF_VAR(RecommendedServerVersions, LINELIST, 0, NULL)

/** Relays which should be voted Guard regardless of uptime and bandwidth. */
CONF_VAR(AuthDirVoteGuard, ROUTERSET, 0, NULL)

/** If a relay's uptime is at least this value, then it is always considered
 * stable, regardless of the rest of the network. This way we resist attacks
 * where an attacker doubles the size of the network using allegedly
 * high-uptime nodes, displacing all the current guards. */
CONF_VAR(AuthDirVoteStableGuaranteeMinUptime, INTERVAL, 0, "30 days")

/** If a relay's MTBF is at least this value, then it is always stable. See
 * above. */
CONF_VAR(AuthDirVoteStableGuaranteeMTBF, INTERVAL, 0, "5 days")

/** A relay with at least this much weighted time known can be considered
 * familiar enough to be a guard. */
CONF_VAR(AuthDirVoteGuardGuaranteeTimeKnown, INTERVAL, 0, "8 days")

/** A relay with sufficient WFU is around enough to be a guard. */
CONF_VAR(AuthDirVoteGuardGuaranteeWFU, DOUBLE, 0, "0.98")

/** The Guard flag bandwidth performance threshold fraction that is the
 * fraction representing who gets the Guard flag out of all measured
 * bandwidth. */
CONF_VAR(AuthDirVoteGuardBwThresholdFraction, DOUBLE, 0, "0.75")

/** If an authority has been around for less than this amount of time, it
 * does not believe its reachability information is accurate.  Only
 * altered on testing networks. */
CONF_VAR(TestingAuthDirTimeToLearnReachability, INTERVAL, 0, "30 minutes")

  /** Relays in a testing network which should be voted Exit
   * regardless of exit policy. */
CONF_VAR(TestingDirAuthVoteExit, ROUTERSET, 0, NULL)
CONF_VAR(TestingDirAuthVoteExitIsStrict, BOOL, 0, "0")

/** Relays in a testing network which should be voted Guard
 * regardless of uptime and bandwidth. */
CONF_VAR(TestingDirAuthVoteGuard, ROUTERSET, 0, NULL)
CONF_VAR(TestingDirAuthVoteGuardIsStrict, BOOL, 0, "0")

/** Relays in a testing network which should be voted HSDir
 * regardless of uptime and DirPort. */
CONF_VAR(TestingDirAuthVoteHSDir, ROUTERSET, 0, NULL)
CONF_VAR(TestingDirAuthVoteHSDirIsStrict, BOOL, 0, "0")

/** Minimum value for the Exit flag threshold on testing networks. */
CONF_VAR(TestingMinExitFlagThreshold, MEMUNIT, 0, "0")

/** Minimum value for the Fast flag threshold on testing networks. */
CONF_VAR(TestingMinFastFlagThreshold, MEMUNIT, 0, "0")

/** Boolean: is this an authoritative directory that's willing to recommend
 * versions? */
CONF_VAR(VersioningAuthoritativeDirectory, BOOL, 0, "0")

/** Boolean: Under bandwidth pressure, if set to 1, the authority will always
 * answer directory requests from relays but will start sending 503 error code
 * for the other connections. If set to 0, all connections are considered the
 * same and the authority will try to answer them all regardless of bandwidth
 * pressure or not. */
CONF_VAR(AuthDirRejectRequestsUnderLoad, BOOL, 0, "1")

END_CONF_STRUCT(dirauth_options_t)
