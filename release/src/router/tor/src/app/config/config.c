/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file config.c
 * \brief Code to interpret the user's configuration of Tor.
 *
 * This module handles torrc configuration file, including parsing it,
 * combining it with torrc.defaults and the command line, allowing
 * user changes to it (via editing and SIGHUP or via the control port),
 * writing it back to disk (because of SAVECONF from the control port),
 * and -- most importantly, acting on it.
 *
 * The module additionally has some tools for manipulating and
 * inspecting values that are calculated as a result of the
 * configured options.
 *
 * <h3>How to add new options</h3>
 *
 * To add new items to the torrc, there are a minimum of three places to edit:
 * <ul>
 *   <li>The or_options_t structure in or_options_st.h, where the options are
 *       stored.
 *   <li>The option_vars_ array below in this module, which configures
 *       the names of the torrc options, their types, their multiplicities,
 *       and their mappings to fields in or_options_t.
 *   <li>The manual in doc/man/tor.1.txt, to document what the new option
 *       is, and how it works.
 * </ul>
 *
 * Additionally, you might need to edit these places too:
 * <ul>
 *   <li>options_validate_cb() below, in case you want to reject some possible
 *       values of the new configuration option.
 *   <li>options_transition_allowed() below, in case you need to
 *       forbid some or all changes in the option while Tor is
 *       running.
 *   <li>options_transition_affects_workers(), in case changes in the option
 *       might require Tor to relaunch or reconfigure its worker threads.
 *       (This function is now in the relay module.)
 *   <li>options_transition_affects_descriptor(), in case changes in the
 *       option might require a Tor relay to build and publish a new server
 *       descriptor.
 *       (This function is now in the relay module.)
 *   <li>options_act() and/or options_act_reversible(), in case there's some
 *       action that needs to be taken immediately based on the option's
 *       value.
 * </ul>
 *
 * <h3>Changing the value of an option</h3>
 *
 * Because of the SAVECONF command from the control port, it's a bad
 * idea to change the value of any user-configured option in the
 * or_options_t.  If you want to sometimes do this anyway, we recommend
 * that you create a secondary field in or_options_t; that you have the
 * user option linked only to the secondary field; that you use the
 * secondary field to initialize the one that Tor actually looks at; and that
 * you use the one Tor looks as the one that you modify.
 **/

#define CONFIG_PRIVATE
#include "core/or/or.h"
#include "app/config/config.h"
#include "lib/confmgt/confmgt.h"
#include "app/config/statefile.h"
#include "app/main/main.h"
#include "app/main/subsysmgr.h"
#include "core/mainloop/connection.h"
#include "core/mainloop/mainloop.h"
#include "core/mainloop/netstatus.h"
#include "core/or/channel.h"
#include "core/or/circuitlist.h"
#include "core/or/circuitmux.h"
#include "core/or/circuitmux_ewma.h"
#include "core/or/circuitstats.h"
#include "core/or/connection_edge.h"
#include "trunnel/conflux.h"
#include "core/or/dos.h"
#include "core/or/policies.h"
#include "core/or/relay.h"
#include "core/or/scheduler.h"
#include "feature/client/addressmap.h"
#include "feature/client/bridges.h"
#include "feature/client/entrynodes.h"
#include "feature/client/transports.h"
#include "feature/control/control.h"
#include "feature/control/control_auth.h"
#include "feature/control/control_events.h"
#include "feature/dircache/dirserv.h"
#include "feature/dirclient/dirclient_modes.h"
#include "feature/hibernate/hibernate.h"
#include "feature/hs/hs_config.h"
#include "feature/hs/hs_pow.h"
#include "feature/metrics/metrics.h"
#include "feature/nodelist/dirlist.h"
#include "feature/nodelist/networkstatus.h"
#include "feature/nodelist/nickname.h"
#include "feature/nodelist/nodelist.h"
#include "feature/nodelist/routerlist.h"
#include "feature/nodelist/routerset.h"
#include "feature/relay/dns.h"
#include "feature/relay/ext_orport.h"
#include "feature/relay/routermode.h"
#include "feature/relay/relay_config.h"
#include "feature/relay/transport_config.h"
#include "lib/geoip/geoip.h"
#include "feature/stats/geoip_stats.h"
#include "lib/compress/compress.h"
#include "lib/confmgt/structvar.h"
#include "lib/crypt_ops/crypto_init.h"
#include "lib/crypt_ops/crypto_rand.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/encoding/confline.h"
#include "lib/net/resolve.h"
#include "lib/sandbox/sandbox.h"
#include "lib/version/torversion.h"

#ifdef ENABLE_NSS
#include "lib/crypt_ops/crypto_nss_mgt.h"
#else
#include "lib/crypt_ops/crypto_openssl_mgt.h"
#endif

#ifdef _WIN32
#include <shlobj.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "lib/meminfo/meminfo.h"
#include "lib/osinfo/uname.h"
#include "lib/osinfo/libc.h"
#include "lib/process/daemon.h"
#include "lib/process/pidfile.h"
#include "lib/process/restrict.h"
#include "lib/process/setuid.h"
#include "lib/process/process.h"
#include "lib/net/gethostname.h"
#include "lib/thread/numcpus.h"

#include "lib/encoding/keyval.h"
#include "lib/fs/conffile.h"
#include "lib/evloop/procmon.h"

#include "feature/dirauth/authmode.h"
#include "feature/dirauth/dirauth_config.h"

#include "core/or/connection_st.h"
#include "core/or/port_cfg_st.h"

#ifdef HAVE_SYSTEMD
#   if defined(__COVERITY__) && !defined(__INCLUDE_LEVEL__)
/* Systemd's use of gcc's __INCLUDE_LEVEL__ extension macro appears to confuse
 * Coverity. Here's a kludge to unconfuse it.
 */
#   define __INCLUDE_LEVEL__ 2
#endif /* defined(__COVERITY__) && !defined(__INCLUDE_LEVEL__) */
#include <systemd/sd-daemon.h>
#endif /* defined(HAVE_SYSTEMD) */

/* Prefix used to indicate a Unix socket in a FooPort configuration. */
static const char unix_socket_prefix[] = "unix:";
/* Prefix used to indicate a Unix socket with spaces in it, in a FooPort
 * configuration. */
static const char unix_q_socket_prefix[] = "unix:\"";

/* limits for TCP send and recv buffer size used for constrained sockets */
#define MIN_CONSTRAINED_TCP_BUFFER 2048
#define MAX_CONSTRAINED_TCP_BUFFER 262144  /* 256k */

/** macro to help with the bulk rename of *DownloadSchedule to
 * *DownloadInitialDelay . */
#ifndef COCCI
#define DOWNLOAD_SCHEDULE(name) \
  { (#name "DownloadSchedule"), (#name "DownloadInitialDelay"), 0, 1 }
#else
#define DOWNLOAD_SCHEDULE(name) { NULL, NULL, 0, 1 }
#endif /* !defined(COCCI) */

/** A list of abbreviations and aliases to map command-line options, obsolete
 * option names, or alternative option names, to their current values. */
static const config_abbrev_t option_abbrevs_[] = {
  PLURAL(AuthDirBadDirCC),
  PLURAL(AuthDirBadExitCC),
  PLURAL(AuthDirInvalidCC),
  PLURAL(AuthDirMiddleOnlyCC),
  PLURAL(AuthDirRejectCC),
  PLURAL(EntryNode),
  PLURAL(ExcludeNode),
  PLURAL(FirewallPort),
  PLURAL(LongLivedPort),
  PLURAL(HiddenServiceNode),
  PLURAL(HiddenServiceExcludeNode),
  PLURAL(NumCPU),
  PLURAL(RendNode),
  PLURAL(RecommendedPackage),
  PLURAL(RendExcludeNode),
  PLURAL(StrictEntryNode),
  PLURAL(StrictExitNode),
  PLURAL(StrictNode),
  { "l", "Log", 1, 0},
  { "AllowUnverifiedNodes", "AllowInvalidNodes", 0, 0},
  { "AutomapHostSuffixes", "AutomapHostsSuffixes", 0, 0},
  { "AutomapHostOnResolve", "AutomapHostsOnResolve", 0, 0},
  { "BandwidthRateBytes", "BandwidthRate", 0, 0},
  { "BandwidthBurstBytes", "BandwidthBurst", 0, 0},
  { "DirFetchPostPeriod", "StatusFetchPeriod", 0, 0},
  { "DirServer", "DirAuthority", 0, 0}, /* XXXX later, make this warn? */
  { "MaxConn", "ConnLimit", 0, 1},
  { "MaxMemInCellQueues", "MaxMemInQueues", 0, 0},
  { "ORBindAddress", "ORListenAddress", 0, 0},
  { "DirBindAddress", "DirListenAddress", 0, 0},
  { "SocksBindAddress", "SocksListenAddress", 0, 0},
  { "UseHelperNodes", "UseEntryGuards", 0, 0},
  { "NumHelperNodes", "NumEntryGuards", 0, 0},
  { "UseEntryNodes", "UseEntryGuards", 0, 0},
  { "NumEntryNodes", "NumEntryGuards", 0, 0},
  { "ResolvConf", "ServerDNSResolvConfFile", 0, 1},
  { "SearchDomains", "ServerDNSSearchDomains", 0, 1},
  { "ServerDNSAllowBrokenResolvConf", "ServerDNSAllowBrokenConfig", 0, 0},
  { "PreferTunnelledDirConns", "PreferTunneledDirConns", 0, 0},
  { "BridgeAuthoritativeDirectory", "BridgeAuthoritativeDir", 0, 0},
  { "HashedControlPassword", "__HashedControlSessionPassword", 1, 0},
  { "VirtualAddrNetwork", "VirtualAddrNetworkIPv4", 0, 0},
  { "SocksSocketsGroupWritable", "UnixSocksGroupWritable", 0, 1},
  { "_HSLayer2Nodes", "HSLayer2Nodes", 0, 1 },
  { "_HSLayer3Nodes", "HSLayer3Nodes", 0, 1 },

  DOWNLOAD_SCHEDULE(ClientBootstrapConsensusAuthority),
  DOWNLOAD_SCHEDULE(ClientBootstrapConsensusAuthorityOnly),
  DOWNLOAD_SCHEDULE(ClientBootstrapConsensusFallback),
  DOWNLOAD_SCHEDULE(TestingBridge),
  DOWNLOAD_SCHEDULE(TestingBridgeBootstrap),
  DOWNLOAD_SCHEDULE(TestingClient),
  DOWNLOAD_SCHEDULE(TestingClientConsensus),
  DOWNLOAD_SCHEDULE(TestingServer),
  DOWNLOAD_SCHEDULE(TestingServerConsensus),

  { NULL, NULL, 0, 0},
};

/** dummy instance of or_options_t, used for type-checking its
 * members with CONF_CHECK_VAR_TYPE. */
DUMMY_TYPECHECK_INSTANCE(or_options_t);

/** An entry for config_vars: "The option <b>varname</b> has type
 * CONFIG_TYPE_<b>conftype</b>, and corresponds to
 * or_options_t.<b>member</b>"
 */
#define VAR(varname,conftype,member,initvalue)                          \
  CONFIG_VAR_ETYPE(or_options_t, varname, conftype, member, 0, initvalue)

/* As VAR, but uses a type definition in addition to a type enum. */
#define VAR_D(varname,conftype,member,initvalue)                        \
  CONFIG_VAR_DEFN(or_options_t, varname, conftype, member, 0, initvalue)

#define VAR_NODUMP(varname,conftype,member,initvalue)             \
  CONFIG_VAR_ETYPE(or_options_t, varname, conftype, member,       \
                   CFLG_NODUMP, initvalue)
#define VAR_NODUMP_IMMUTABLE(varname,conftype,member,initvalue)   \
  CONFIG_VAR_ETYPE(or_options_t, varname, conftype, member,       \
                   CFLG_NODUMP | CFLG_IMMUTABLE, initvalue)
#define VAR_INVIS(varname,conftype,member,initvalue)              \
  CONFIG_VAR_ETYPE(or_options_t, varname, conftype, member,       \
                   CFLG_NODUMP | CFLG_NOSET | CFLG_NOLIST, initvalue)

#define V(member,conftype,initvalue)            \
  VAR(#member, conftype, member, initvalue)

#define VAR_IMMUTABLE(varname, conftype, member, initvalue)             \
  CONFIG_VAR_ETYPE(or_options_t, varname, conftype, member,             \
                   CFLG_IMMUTABLE, initvalue)

#define V_IMMUTABLE(member,conftype,initvalue)                 \
  VAR_IMMUTABLE(#member, conftype, member, initvalue)

/** As V, but uses a type definition instead of a type enum */
#define V_D(member,type,initvalue)              \
  VAR_D(#member, type, member, initvalue)

/** An entry for config_vars: "The option <b>varname</b> is obsolete." */
#define OBSOLETE(varname) CONFIG_VAR_OBSOLETE(varname)

/**
 * Macro to declare *Port options.  Each one comes in three entries.
 * For example, most users should use "SocksPort" to configure the
 * socks port, but TorBrowser wants to use __SocksPort so that it
 * isn't stored by SAVECONF.  The SocksPortLines virtual option is
 * used to query both options from the controller.
 */
#define VPORT(member)                                           \
  VAR(#member "Lines", LINELIST_V, member ## _lines, NULL),     \
  VAR(#member, LINELIST_S, member ## _lines, NULL),             \
  VAR_NODUMP("__" #member, LINELIST_S, member ## _lines, NULL)

/** UINT64_MAX as a decimal string */
#define UINT64_MAX_STRING "18446744073709551615"

/** Array of configuration options.  Until we disallow nonstandard
 * abbreviations, order is significant, since the first matching option will
 * be chosen first.
 */
static const config_var_t option_vars_[] = {
  V(AccountingMax,               MEMUNIT,  "0 bytes"),
  VAR("AccountingRule",          STRING,   AccountingRule_option,  "max"),
  V(AccountingStart,             STRING,   NULL),
  V(Address,                     LINELIST, NULL),
  V(AddressDisableIPv6,          BOOL,     "0"),
  OBSOLETE("AllowDotExit"),
  OBSOLETE("AllowInvalidNodes"),
  V(AllowNonRFC953Hostnames,     BOOL,     "0"),
  OBSOLETE("AllowSingleHopCircuits"),
  OBSOLETE("AllowSingleHopExits"),
  V(AlternateBridgeAuthority,    LINELIST, NULL),
  V(AlternateDirAuthority,       LINELIST, NULL),
  OBSOLETE("AlternateHSAuthority"),
  V(AssumeReachable,             BOOL,     "0"),
  V(AssumeReachableIPv6,         AUTOBOOL, "auto"),
  OBSOLETE("AuthDirBadDir"),
  OBSOLETE("AuthDirBadDirCCs"),
  V(AuthDirBadExit,              LINELIST, NULL),
  V(AuthDirBadExitCCs,           CSV,      ""),
  V(AuthDirInvalid,              LINELIST, NULL),
  V(AuthDirInvalidCCs,           CSV,      ""),
  V(AuthDirMiddleOnly,           LINELIST, NULL),
  V(AuthDirMiddleOnlyCCs,        CSV,      ""),
  V(AuthDirReject,               LINELIST, NULL),
  V(AuthDirRejectCCs,            CSV,      ""),
  OBSOLETE("AuthDirRejectUnlisted"),
  OBSOLETE("AuthDirListBadDirs"),
  OBSOLETE("AuthDirMaxServersPerAuthAddr"),
  VAR("AuthoritativeDirectory",  BOOL, AuthoritativeDir,    "0"),
  V(AutomapHostsOnResolve,       BOOL,     "0"),
  V(AutomapHostsSuffixes,        CSV,      ".onion,.exit"),
  V(AvoidDiskWrites,             BOOL,     "0"),
  V(BandwidthBurst,              MEMUNIT,  "1 GB"),
  V(BandwidthRate,               MEMUNIT,  "1 GB"),
  V(BridgeAuthoritativeDir,      BOOL,     "0"),
  VAR("Bridge",                  LINELIST, Bridges,    NULL),
  V(BridgePassword,              STRING,   NULL),
  V(BridgeRecordUsageByCountry,  BOOL,     "1"),
  V(BridgeRelay,                 BOOL,     "0"),
  V(BridgeDistribution,          STRING,   NULL),
  VAR_IMMUTABLE("CacheDirectory",FILENAME, CacheDirectory_option, NULL),
  V(CacheDirectoryGroupReadable, AUTOBOOL,     "auto"),
  V(CellStatistics,              BOOL,     "0"),
  V(PaddingStatistics,           BOOL,     "1"),
  V(OverloadStatistics,          BOOL,     "1"),
  V(LearnCircuitBuildTimeout,    BOOL,     "1"),
  V(CircuitBuildTimeout,         INTERVAL, "0"),
  OBSOLETE("CircuitIdleTimeout"),
  V(CircuitsAvailableTimeout,    INTERVAL, "0"),
  V(CircuitStreamTimeout,        INTERVAL, "0"),
  V(CircuitPriorityHalflife,     DOUBLE,  "-1.0"), /*negative:'Use default'*/
  V(ClientDNSRejectInternalAddresses, BOOL,"1"),
#if defined(HAVE_MODULE_RELAY) || defined(TOR_UNIT_TESTS)
  /* The unit tests expect the ClientOnly default to be 0. */
  V(ClientOnly,                  BOOL,     "0"),
#else
  /* We must be a Client if the relay module is disabled. */
  V(ClientOnly,                  BOOL,     "1"),
#endif /* defined(HAVE_MODULE_RELAY) || defined(TOR_UNIT_TESTS) */
  V(ClientPreferIPv6ORPort,      AUTOBOOL, "auto"),
  V(ClientPreferIPv6DirPort,     AUTOBOOL, "auto"),
  OBSOLETE("ClientAutoIPv6ORPort"),
  V(ClientRejectInternalAddresses, BOOL,   "1"),
  V(ClientTransportPlugin,       LINELIST, NULL),
  V(ClientUseIPv6,               BOOL,     "1"),
  V(ClientUseIPv4,               BOOL,     "1"),
  V(CompiledProofOfWorkHash,     AUTOBOOL, "auto"),
  V(ConfluxEnabled,              AUTOBOOL, "auto"),
  VAR("ConfluxClientUX",         STRING,   ConfluxClientUX_option,
          "throughput"),
  V(ConnLimit,                   POSINT,     "1000"),
  V(ConnDirectionStatistics,     BOOL,     "0"),
  V(ConstrainedSockets,          BOOL,     "0"),
  V(ConstrainedSockSize,         MEMUNIT,  "8192"),
  V(ContactInfo,                 STRING,   NULL),
  OBSOLETE("ControlListenAddress"),
  VPORT(ControlPort),
  V(ControlPortFileGroupReadable,BOOL,     "0"),
  V(ControlPortWriteToFile,      FILENAME, NULL),
  V(ControlSocket,               LINELIST, NULL),
  V(ControlSocketsGroupWritable, BOOL,     "0"),
  V(UnixSocksGroupWritable,    BOOL,     "0"),
  V(CookieAuthentication,        BOOL,     "0"),
  V(CookieAuthFileGroupReadable, BOOL,     "0"),
  V(CookieAuthFile,              FILENAME, NULL),
  V(CountPrivateBandwidth,       BOOL,     "0"),
  VAR_IMMUTABLE("DataDirectory", FILENAME, DataDirectory_option, NULL),
  V(DataDirectoryGroupReadable,  BOOL,     "0"),
  V(DisableOOSCheck,             BOOL,     "1"),
  V(DisableNetwork,              BOOL,     "0"),
  V(DirAllowPrivateAddresses,    BOOL,     "0"),
  OBSOLETE("DirListenAddress"),
  V(DirPolicy,                   LINELIST, NULL),
  VPORT(DirPort),
  V(DirPortFrontPage,            FILENAME, NULL),
  VAR("DirReqStatistics",        BOOL,     DirReqStatistics_option, "1"),
  VAR("DirAuthority",            LINELIST, DirAuthorities, NULL),
#if defined(HAVE_MODULE_RELAY) || defined(TOR_UNIT_TESTS)
  /* The unit tests expect the DirCache default to be 1. */
  V(DirCache,                    BOOL,     "1"),
#else
  /* We can't be a DirCache if the relay module is disabled. */
  V(DirCache,                    BOOL,     "0"),
#endif /* defined(HAVE_MODULE_RELAY) || defined(TOR_UNIT_TESTS) */
  /* A DirAuthorityFallbackRate of 0.1 means that 0.5% of clients try an
   * authority when all fallbacks are up, and 2% try an authority when 25% of
   * fallbacks are down. (We rebuild the list when 25% of fallbacks are down).
   *
   * We want to reduce load on authorities, but keep these two figures within
   * an order of magnitude, so there isn't too much load shifting to
   * authorities when fallbacks go down. */
  V(DirAuthorityFallbackRate,    DOUBLE,   "0.1"),
  V_IMMUTABLE(DisableAllSwap,    BOOL,     "0"),
  V_IMMUTABLE(DisableDebuggerAttachment,   BOOL,     "1"),
  OBSOLETE("DisableIOCP"),
  OBSOLETE("DisableV2DirectoryInfo_"),
  OBSOLETE("DynamicDHGroups"),
  VPORT(DNSPort),
  OBSOLETE("DNSListenAddress"),
  V(DormantClientTimeout,        INTERVAL, "24 hours"),
  V(DormantTimeoutEnabled,       BOOL,     "1"),
  V(DormantTimeoutDisabledByIdleStreams,   BOOL,     "1"),
  V(DormantOnFirstStartup,       BOOL,      "0"),
  V(DormantCanceledByStartup,    BOOL,      "0"),
  V(DownloadExtraInfo,           BOOL,     "0"),
  V(TestingEnableConnBwEvent,    BOOL,     "0"),
  V(TestingEnableCellStatsEvent, BOOL,     "0"),
  OBSOLETE("TestingEnableTbEmptyEvent"),
  V(EnforceDistinctSubnets,      BOOL,     "1"),
  V_D(EntryNodes,                ROUTERSET,   NULL),
  V(EntryStatistics,             BOOL,     "0"),
  OBSOLETE("TestingEstimatedDescriptorPropagationTime"),
  V_D(ExcludeNodes,              ROUTERSET, NULL),
  V_D(ExcludeExitNodes,          ROUTERSET, NULL),
  OBSOLETE("ExcludeSingleHopRelays"),
  V_D(ExitNodes,                 ROUTERSET, NULL),
  /* Researchers need a way to tell their clients to use specific
   * middles that they also control, to allow safe live-network
   * experimentation with new padding machines. */
  V_D(MiddleNodes,               ROUTERSET, NULL),
  V(ExitPolicy,                  LINELIST, NULL),
  V(ExitPolicyRejectPrivate,     BOOL,     "1"),
  V(ExitPolicyRejectLocalInterfaces, BOOL, "0"),
  V(ExitPortStatistics,          BOOL,     "0"),
  V(ExtendAllowPrivateAddresses, BOOL,     "0"),
  V(ExitRelay,                   AUTOBOOL, "auto"),
  VPORT(ExtORPort),
  V(ExtORPortCookieAuthFile,     FILENAME,   NULL),
  V(ExtORPortCookieAuthFileGroupReadable, BOOL, "0"),
  V(ExtraInfoStatistics,         BOOL,     "1"),
  V(ExtendByEd25519ID,           AUTOBOOL, "auto"),
  V(FallbackDir,                 LINELIST, NULL),

  V(UseDefaultFallbackDirs,      BOOL,     "1"),

  OBSOLETE("FallbackNetworkstatusFile"),
  V(FascistFirewall,             BOOL,     "0"),
  V(FirewallPorts,               CSV,      ""),
  OBSOLETE("FastFirstHopPK"),
  V(FetchDirInfoEarly,           BOOL,     "0"),
  V(FetchDirInfoExtraEarly,      BOOL,     "0"),
  V(FetchServerDescriptors,      BOOL,     "1"),
  V(FetchHidServDescriptors,     BOOL,     "1"),
  V(FetchUselessDescriptors,     BOOL,     "0"),
  OBSOLETE("FetchV2Networkstatus"),
  V(GeoIPExcludeUnknown,         AUTOBOOL, "auto"),
#ifdef _WIN32
  V(GeoIPFile,                   FILENAME, "<default>"),
  V(GeoIPv6File,                 FILENAME, "<default>"),
#else
  V(GeoIPFile,                   FILENAME,
    SHARE_DATADIR PATH_SEPARATOR "tor" PATH_SEPARATOR "geoip"),
  V(GeoIPv6File,                 FILENAME,
    SHARE_DATADIR PATH_SEPARATOR "tor" PATH_SEPARATOR "geoip6"),
#endif /* defined(_WIN32) */
  OBSOLETE("Group"),
  V(GuardLifetime,               INTERVAL, "0 minutes"),
  V(HeartbeatPeriod,             INTERVAL, "6 hours"),
  V(MainloopStats,               BOOL,     "0"),
  V(HashedControlPassword,       LINELIST, NULL),
  OBSOLETE("HidServDirectoryV2"),
  OBSOLETE("HiddenServiceAuthorizeClient"),
  OBSOLETE("HidServAuth"),
  VAR("HiddenServiceDir",    LINELIST_S, RendConfigLines,    NULL),
  VAR("HiddenServiceDirGroupReadable",  LINELIST_S, RendConfigLines, NULL),
  VAR("HiddenServiceOptions",LINELIST_V, RendConfigLines,    NULL),
  VAR("HiddenServicePort",   LINELIST_S, RendConfigLines,    NULL),
  VAR("HiddenServiceVersion",LINELIST_S, RendConfigLines,    NULL),
  VAR("HiddenServiceAllowUnknownPorts",LINELIST_S, RendConfigLines, NULL),
  VAR("HiddenServiceMaxStreams",LINELIST_S, RendConfigLines, NULL),
  VAR("HiddenServiceMaxStreamsCloseCircuit",LINELIST_S, RendConfigLines, NULL),
  VAR("HiddenServiceNumIntroductionPoints", LINELIST_S, RendConfigLines, NULL),
  VAR("HiddenServiceExportCircuitID", LINELIST_S,  RendConfigLines, NULL),
  VAR("HiddenServiceEnableIntroDoSDefense", LINELIST_S, RendConfigLines, NULL),
  VAR("HiddenServiceEnableIntroDoSRatePerSec",
      LINELIST_S, RendConfigLines, NULL),
  VAR("HiddenServiceEnableIntroDoSBurstPerSec",
      LINELIST_S, RendConfigLines, NULL),
  VAR("HiddenServiceOnionBalanceInstance",
      LINELIST_S, RendConfigLines, NULL),
  VAR("HiddenServicePoWDefensesEnabled", LINELIST_S, RendConfigLines, NULL),
  VAR("HiddenServicePoWQueueRate", LINELIST_S, RendConfigLines, NULL),
  VAR("HiddenServicePoWQueueBurst", LINELIST_S, RendConfigLines, NULL),
  VAR("HiddenServiceStatistics", BOOL, HiddenServiceStatistics_option, "1"),
  V(ClientOnionAuthDir,          FILENAME, NULL),
  OBSOLETE("CloseHSClientCircuitsImmediatelyOnTimeout"),
  OBSOLETE("CloseHSServiceRendCircuitsImmediatelyOnTimeout"),
  V_IMMUTABLE(HiddenServiceSingleHopMode,  BOOL,     "0"),
  V_IMMUTABLE(HiddenServiceNonAnonymousMode,BOOL,    "0"),
  V(HTTPProxy,                   STRING,   NULL),
  V(HTTPProxyAuthenticator,      STRING,   NULL),
  V(HTTPSProxy,                  STRING,   NULL),
  V(HTTPSProxyAuthenticator,     STRING,   NULL),
  VPORT(HTTPTunnelPort),
  V(IPv6Exit,                    BOOL,     "0"),
  VAR("ServerTransportPlugin",   LINELIST, ServerTransportPlugin,  NULL),
  V(ServerTransportListenAddr,   LINELIST, NULL),
  V(ServerTransportOptions,      LINELIST, NULL),
  V(SigningKeyLifetime,          INTERVAL, "30 days"),
  V(Socks4Proxy,                 STRING,   NULL),
  V(Socks5Proxy,                 STRING,   NULL),
  V(Socks5ProxyUsername,         STRING,   NULL),
  V(Socks5ProxyPassword,         STRING,   NULL),
  V(TCPProxy,                    STRING,   NULL),
  VAR_IMMUTABLE("KeyDirectory",  FILENAME, KeyDirectory_option, NULL),
  V(KeyDirectoryGroupReadable,   AUTOBOOL, "auto"),
  VAR_D("HSLayer2Nodes",         ROUTERSET,  HSLayer2Nodes,  NULL),
  VAR_D("HSLayer3Nodes",         ROUTERSET,  HSLayer3Nodes,  NULL),
  V(KeepalivePeriod,             INTERVAL, "5 minutes"),
  V_IMMUTABLE(KeepBindCapabilities,        AUTOBOOL, "auto"),
  VAR("Log",                     LINELIST, Logs,             NULL),
  V(LogMessageDomains,           BOOL,     "0"),
  V(LogTimeGranularity,          MSEC_INTERVAL, "1 second"),
  V(TruncateLogFile,             BOOL,     "0"),
  V_IMMUTABLE(SyslogIdentityTag, STRING,   NULL),
  OBSOLETE("AndroidIdentityTag"),
  V(LongLivedPorts,              CSV,
        "21,22,706,1863,5050,5190,5222,5223,6523,6667,6697,8300"),
  VAR("MapAddress",              LINELIST, AddressMap,           NULL),
  V(MaxAdvertisedBandwidth,      MEMUNIT,  "1 GB"),
  V(MaxCircuitDirtiness,         INTERVAL, "10 minutes"),
  V(MaxClientCircuitsPending,    POSINT,     "32"),
  V(MaxConsensusAgeForDiffs,     INTERVAL, "0 seconds"),
  VAR("MaxMemInQueues",          MEMUNIT,   MaxMemInQueues_raw, "0"),
  OBSOLETE("MaxOnionsPending"),
  V(MaxOnionQueueDelay,          MSEC_INTERVAL, "0"),
  V(MaxUnparseableDescSizeToLog, MEMUNIT, "10 MB"),
  VPORT(MetricsPort),
  V(MetricsPortPolicy,           LINELIST, NULL),
  V(TestingMinTimeToReportBandwidth,    INTERVAL, "1 day"),
  VAR("MyFamily",                LINELIST, MyFamily_lines,       NULL),
  V(NewCircuitPeriod,            INTERVAL, "30 seconds"),
  OBSOLETE("NamingAuthoritativeDirectory"),
  OBSOLETE("NATDListenAddress"),
  VPORT(NATDPort),
  V(Nickname,                    STRING,   NULL),
  OBSOLETE("PredictedPortsRelevanceTime"),
  OBSOLETE("WarnUnsafeSocks"),
  VAR("NodeFamily",              LINELIST, NodeFamilies,         NULL),
  V_IMMUTABLE(NoExec,            BOOL,     "0"),
  V(NumCPUs,                     POSINT,     "0"),
  V(NumDirectoryGuards,          POSINT,     "0"),
  V(NumEntryGuards,              POSINT,     "0"),
  V(NumPrimaryGuards,            POSINT,     "0"),
  V(OfflineMasterKey,            BOOL,     "0"),
  OBSOLETE("ORListenAddress"),
  VPORT(ORPort),
  V(OutboundBindAddress,         LINELIST,   NULL),
  V(OutboundBindAddressOR,       LINELIST,   NULL),
  V(OutboundBindAddressExit,     LINELIST,   NULL),
  V(OutboundBindAddressPT,       LINELIST,   NULL),

  OBSOLETE("PathBiasDisableRate"),
  V(PathBiasCircThreshold,       INT,      "-1"),
  V(PathBiasNoticeRate,          DOUBLE,   "-1"),
  V(PathBiasWarnRate,            DOUBLE,   "-1"),
  V(PathBiasExtremeRate,         DOUBLE,   "-1"),
  V(PathBiasScaleThreshold,      INT,      "-1"),
  OBSOLETE("PathBiasScaleFactor"),
  OBSOLETE("PathBiasMultFactor"),
  V(PathBiasDropGuards,          AUTOBOOL, "0"),
  OBSOLETE("PathBiasUseCloseCounts"),

  V(PathBiasUseThreshold,       INT,      "-1"),
  V(PathBiasNoticeUseRate,          DOUBLE,   "-1"),
  V(PathBiasExtremeUseRate,         DOUBLE,   "-1"),
  V(PathBiasScaleUseThreshold,      INT,      "-1"),

  V(PathsNeededToBuildCircuits,  DOUBLE,   "-1"),
  V(PerConnBWBurst,              MEMUNIT,  "0"),
  V(PerConnBWRate,               MEMUNIT,  "0"),
  V_IMMUTABLE(PidFile,           FILENAME,   NULL),
  V_IMMUTABLE(TestingTorNetwork, BOOL,     "0"),

  V(TestingLinkCertLifetime,          INTERVAL, "2 days"),
  V(TestingAuthKeyLifetime,          INTERVAL, "2 days"),
  V(TestingLinkKeySlop,              INTERVAL, "3 hours"),
  V(TestingAuthKeySlop,              INTERVAL, "3 hours"),
  V(TestingSigningKeySlop,           INTERVAL, "1 day"),

  OBSOLETE("OptimisticData"),
  OBSOLETE("PortForwarding"),
  OBSOLETE("PortForwardingHelper"),
  OBSOLETE("PreferTunneledDirConns"),
  V(ProtocolWarnings,            BOOL,     "0"),
  V(PublishServerDescriptor,     CSV,      "1"),
  V(PublishHidServDescriptors,   BOOL,     "1"),
  V(ReachableAddresses,          LINELIST, NULL),
  V(ReachableDirAddresses,       LINELIST, NULL),
  V(ReachableORAddresses,        LINELIST, NULL),
  OBSOLETE("RecommendedPackages"),
  V(ReducedConnectionPadding,    BOOL,     "0"),
  V(ConnectionPadding,           AUTOBOOL, "auto"),
  V(RefuseUnknownExits,          AUTOBOOL, "auto"),
  V(CircuitPadding,              BOOL,     "1"),
  V(ReconfigDropsBridgeDescs,    BOOL,     "0"),
  V(ReducedCircuitPadding,       BOOL,     "0"),
  V(RejectPlaintextPorts,        CSV,      ""),
  V(RelayBandwidthBurst,         MEMUNIT,  "0"),
  V(RelayBandwidthRate,          MEMUNIT,  "0"),
  V(RephistTrackTime,            INTERVAL, "24 hours"),
  V_IMMUTABLE(RunAsDaemon,       BOOL,     "0"),
  V(ReducedExitPolicy,           BOOL,     "0"),
  OBSOLETE("RunTesting"), // currently unused
  V_IMMUTABLE(Sandbox,           BOOL,     "0"),
  V(SafeLogging,                 STRING,   "1"),
  V(SafeSocks,                   BOOL,     "0"),
  V(ServerDNSAllowBrokenConfig,  BOOL,     "1"),
  V(ServerDNSAllowNonRFC953Hostnames, BOOL,"0"),
  V(ServerDNSDetectHijacking,    BOOL,     "1"),
  V(ServerDNSRandomizeCase,      BOOL,     "1"),
  V(ServerDNSResolvConfFile,     FILENAME,   NULL),
  V(ServerDNSSearchDomains,      BOOL,     "0"),
  V(ServerDNSTestAddresses,      CSV,
      "www.google.com,www.mit.edu,www.yahoo.com,www.slashdot.org"),
  OBSOLETE("SchedulerLowWaterMark__"),
  OBSOLETE("SchedulerHighWaterMark__"),
  OBSOLETE("SchedulerMaxFlushCells__"),
  V(KISTSchedRunInterval,        MSEC_INTERVAL, "0 msec"),
  V(KISTSockBufSizeFactor,       DOUBLE,   "1.0"),
  V(Schedulers,                  CSV,      "KIST,KISTLite,Vanilla"),
  V(ShutdownWaitLength,          INTERVAL, "30 seconds"),
  OBSOLETE("SocksListenAddress"),
  V(SocksPolicy,                 LINELIST, NULL),
  VPORT(SocksPort),
  V(SocksTimeout,                INTERVAL, "2 minutes"),
  V(SSLKeyLifetime,              INTERVAL, "0"),
  OBSOLETE("StrictEntryNodes"),
  OBSOLETE("StrictExitNodes"),
  V(StrictNodes,                 BOOL,     "0"),
  OBSOLETE("Support022HiddenServices"),
  V(TestSocks,                   BOOL,     "0"),
  V_IMMUTABLE(TokenBucketRefillInterval,   MSEC_INTERVAL, "100 msec"),
  OBSOLETE("Tor2webMode"),
  OBSOLETE("Tor2webRendezvousPoints"),
  OBSOLETE("TLSECGroup"),
  V(TrackHostExits,              CSV,      NULL),
  V(TrackHostExitsExpire,        INTERVAL, "30 minutes"),
  OBSOLETE("TransListenAddress"),
  VPORT(TransPort),
  V(TransProxyType,              STRING,   "default"),
  OBSOLETE("TunnelDirConns"),
  V(UpdateBridgesFromAuthority,  BOOL,     "0"),
  V(UseBridges,                  BOOL,     "0"),
  VAR("UseEntryGuards",          BOOL,     UseEntryGuards_option, "1"),
  OBSOLETE("UseEntryGuardsAsDirGuards"),
  V(UseGuardFraction,            AUTOBOOL, "auto"),
  V(VanguardsLiteEnabled,        AUTOBOOL, "auto"),
  V(UseMicrodescriptors,         AUTOBOOL, "auto"),
  OBSOLETE("UseNTorHandshake"),
  VAR("__AlwaysCongestionControl",  BOOL, AlwaysCongestionControl, "0"),
  VAR("__SbwsExit",  BOOL, SbwsExit, "0"),
  V_IMMUTABLE(User,              STRING,   NULL),
  OBSOLETE("UserspaceIOCPBuffers"),
  OBSOLETE("V1AuthoritativeDirectory"),
  OBSOLETE("V2AuthoritativeDirectory"),
  VAR("V3AuthoritativeDirectory",BOOL, V3AuthoritativeDir,   "0"),
  V(TestingV3AuthInitialVotingInterval, INTERVAL, "30 minutes"),
  V(TestingV3AuthInitialVoteDelay, INTERVAL, "5 minutes"),
  V(TestingV3AuthInitialDistDelay, INTERVAL, "5 minutes"),
  V(TestingV3AuthVotingStartOffset, INTERVAL, "0"),
  V(V3AuthVotingInterval,        INTERVAL, "1 hour"),
  V(V3AuthVoteDelay,             INTERVAL, "5 minutes"),
  V(V3AuthDistDelay,             INTERVAL, "5 minutes"),
  V(V3AuthNIntervalsValid,       POSINT,     "3"),
  V(V3AuthUseLegacyKey,          BOOL,     "0"),
  V(V3BandwidthsFile,            FILENAME, NULL),
  V(GuardfractionFile,           FILENAME, NULL),
  OBSOLETE("VoteOnHidServDirectoriesV2"),
  V(VirtualAddrNetworkIPv4,      STRING,   "127.192.0.0/10"),
  V(VirtualAddrNetworkIPv6,      STRING,   "[FE80::]/10"),
  V(WarnPlaintextPorts,          CSV,      "23,109,110,143"),
  OBSOLETE("UseFilteringSSLBufferevents"),
  OBSOLETE("__UseFilteringSSLBufferevents"),
  VAR_NODUMP("__ReloadTorrcOnSIGHUP",   BOOL,  ReloadTorrcOnSIGHUP,      "1"),
  VAR_NODUMP("__AllDirActionsPrivate",  BOOL,  AllDirActionsPrivate,     "0"),
  VAR_NODUMP("__DisablePredictedCircuits",BOOL,DisablePredictedCircuits, "0"),
  VAR_NODUMP_IMMUTABLE("__DisableSignalHandlers", BOOL,
                       DisableSignalHandlers,    "0"),
  VAR_NODUMP("__LeaveStreamsUnattached",BOOL,  LeaveStreamsUnattached,   "0"),
  VAR_NODUMP("__HashedControlSessionPassword", LINELIST,
             HashedControlSessionPassword,
      NULL),
  VAR_NODUMP("__OwningControllerProcess",STRING,
                       OwningControllerProcess, NULL),
  VAR_NODUMP_IMMUTABLE("__OwningControllerFD", UINT64, OwningControllerFD,
             UINT64_MAX_STRING),
  V(TestingServerDownloadInitialDelay, CSV_INTERVAL, "0"),
  V(TestingClientDownloadInitialDelay, CSV_INTERVAL, "0"),
  V(TestingServerConsensusDownloadInitialDelay, CSV_INTERVAL, "0"),
  V(TestingClientConsensusDownloadInitialDelay, CSV_INTERVAL, "0"),
  /* With the ClientBootstrapConsensus*Download* below:
   * Clients with only authorities will try:
   *  - at least 3 authorities over 10 seconds, then exponentially backoff,
   *    with the next attempt 3-21 seconds later,
   * Clients with authorities and fallbacks will try:
   *  - at least 2 authorities and 4 fallbacks over 21 seconds, then
   *    exponentially backoff, with the next attempts 4-33 seconds later,
   * Clients will also retry when an application request arrives.
   * After a number of failed requests, clients retry every 3 days + 1 hour.
   *
   * Clients used to try 2 authorities over 10 seconds, then wait for
   * 60 minutes or an application request.
   *
   * When clients have authorities and fallbacks available, they use these
   * schedules: (we stagger the times to avoid thundering herds) */
  V(ClientBootstrapConsensusAuthorityDownloadInitialDelay, CSV_INTERVAL, "6"),
  V(ClientBootstrapConsensusFallbackDownloadInitialDelay, CSV_INTERVAL, "0"),
  /* When clients only have authorities available, they use this schedule: */
  V(ClientBootstrapConsensusAuthorityOnlyDownloadInitialDelay, CSV_INTERVAL,
    "0"),
  /* We don't want to overwhelm slow networks (or mirrors whose replies are
   * blocked), but we also don't want to fail if only some mirrors are
   * blackholed. Clients will try 3 directories simultaneously.
   * (Relays never use simultaneous connections.) */
  V(ClientBootstrapConsensusMaxInProgressTries, POSINT, "3"),
  /* When a client has any running bridges, check each bridge occasionally,
    * whether or not that bridge is actually up. */
  V(TestingBridgeDownloadInitialDelay, CSV_INTERVAL,"10800"),
  /* When a client is just starting, or has no running bridges, check each
   * bridge a few times quickly, and then try again later. These schedules
   * are much longer than the other schedules, because we try each and every
   * configured bridge with this schedule. */
  V(TestingBridgeBootstrapDownloadInitialDelay, CSV_INTERVAL, "0"),
  V(TestingClientMaxIntervalWithoutRequest, INTERVAL, "10 minutes"),
  V(TestingDirConnectionMaxStall, INTERVAL, "5 minutes"),
  OBSOLETE("TestingConsensusMaxDownloadTries"),
  OBSOLETE("ClientBootstrapConsensusMaxDownloadTries"),
  OBSOLETE("ClientBootstrapConsensusAuthorityOnlyMaxDownloadTries"),
  OBSOLETE("TestingDescriptorMaxDownloadTries"),
  OBSOLETE("TestingMicrodescMaxDownloadTries"),
  OBSOLETE("TestingCertMaxDownloadTries"),
  VAR_INVIS("___UsingTestNetworkDefaults", BOOL, UsingTestNetworkDefaults_,
            "0"),

  END_OF_CONFIG_VARS
};

/** List of default directory authorities */
static const char *default_authorities[] = {
#ifndef COCCI
#include "auth_dirs.inc"
#endif
  NULL
};

/** List of fallback directory authorities. The list is generated by opt-in of
 * relays that meet certain stability criteria.
 */
static const char *default_fallbacks[] = {
#ifndef COCCI
#include "fallback_dirs.inc"
#endif
  NULL
};

/** Override default values with these if the user sets the TestingTorNetwork
 * option. */
static const struct {
  const char *k;
  const char *v;
} testing_tor_network_defaults[] = {
#ifndef COCCI
#include "testnet.inc"
#endif
  { NULL, NULL }
};

#undef VAR
#undef V
#undef OBSOLETE

static const config_deprecation_t option_deprecation_notes_[] = {
  /* Deprecated since 0.3.2.0-alpha. */
  { "HTTPProxy", "It only applies to direct unencrypted HTTP connections "
    "to your directory server, which your Tor probably wasn't using." },
  { "HTTPProxyAuthenticator", "HTTPProxy is deprecated in favor of HTTPSProxy "
    "which should be used with HTTPSProxyAuthenticator." },
  /* End of options deprecated since 0.3.2.1-alpha */

  /* Options deprecated since 0.3.2.2-alpha */
  { "ReachableDirAddresses", "It has no effect on relays, and has had no "
    "effect on clients since 0.2.8." },
  { "ClientPreferIPv6DirPort", "It has no effect on relays, and has had no "
    "effect on clients since 0.2.8." },
  /* End of options deprecated since 0.3.2.2-alpha. */

  /* Options deprecated since 0.4.3.1-alpha. */
  { "ClientAutoIPv6ORPort", "This option is unreliable if a connection isn't "
    "reliably dual-stack."},
  /* End of options deprecated since 0.4.3.1-alpha. */

  { NULL, NULL }
};

#ifdef _WIN32
static char *get_windows_conf_root(void);
#endif

static int options_check_transition_cb(const void *old,
                                       const void *new,
                                       char **msg);
static int validate_data_directories(or_options_t *options);
static int write_configuration_file(const char *fname,
                                    const or_options_t *options);

static void init_libevent(const or_options_t *options);
static int opt_streq(const char *s1, const char *s2);
static int parse_outbound_addresses(or_options_t *options, int validate_only,
                                    char **msg);
static void config_maybe_load_geoip_files_(const or_options_t *options,
                                           const or_options_t *old_options);
static int options_validate_cb(const void *old_options, void *options,
                               char **msg);
static void cleanup_protocol_warning_severity_level(void);
static void set_protocol_warning_severity_level(int warning_severity);
static void options_clear_cb(const config_mgr_t *mgr, void *opts);
static setopt_err_t options_validate_and_set(const or_options_t *old_options,
                                             or_options_t *new_options,
                                             char **msg_out);
struct listener_transaction_t;
static void options_rollback_listener_transaction(
                                           struct listener_transaction_t *xn);

/** Magic value for or_options_t. */
#define OR_OPTIONS_MAGIC 9090909

/** Configuration format for or_options_t. */
static const config_format_t options_format = {
  .size = sizeof(or_options_t),
  .magic = {
   "or_options_t",
   OR_OPTIONS_MAGIC,
   offsetof(or_options_t, magic_),
  },
  .abbrevs = option_abbrevs_,
  .deprecations = option_deprecation_notes_,
  .vars = option_vars_,
  .legacy_validate_fn = options_validate_cb,
  .check_transition_fn = options_check_transition_cb,
  .clear_fn = options_clear_cb,
  .has_config_suite = true,
  .config_suite_offset = offsetof(or_options_t, subconfigs_),
};

/*
 * Functions to read and write the global options pointer.
 */

/** Command-line and config-file options. */
static or_options_t *global_options = NULL;
/** The fallback options_t object; this is where we look for options not
 * in torrc before we fall back to Tor's defaults. */
static or_options_t *global_default_options = NULL;
/** Name of most recently read torrc file. */
static char *torrc_fname = NULL;
/** Name of the most recently read torrc-defaults file.*/
static char *torrc_defaults_fname = NULL;
/** Result of parsing the command line. */
static parsed_cmdline_t *global_cmdline = NULL;
/** List of port_cfg_t for all configured ports. */
static smartlist_t *configured_ports = NULL;
/** True iff we're currently validating options, and any calls to
 * get_options() are likely to be bugs. */
static int in_option_validation = 0;
/** True iff we have run options_act_once_on_startup() */
static bool have_set_startup_options = false;

/* A global configuration manager to handle all configuration objects. */
static config_mgr_t *options_mgr = NULL;

/** Return the global configuration manager object for torrc options. */
STATIC const config_mgr_t *
get_options_mgr(void)
{
  if (PREDICT_UNLIKELY(options_mgr == NULL)) {
    options_mgr = config_mgr_new(&options_format);
    int rv = subsystems_register_options_formats(options_mgr);
    tor_assert(rv == 0);
    config_mgr_freeze(options_mgr);
  }
  return options_mgr;
}

#define CHECK_OPTIONS_MAGIC(opt) STMT_BEGIN                      \
    config_check_toplevel_magic(get_options_mgr(), (opt));       \
  STMT_END

/** Returns the currently configured options. */
MOCK_IMPL(or_options_t *,
get_options_mutable, (void))
{
  tor_assert(global_options);
  tor_assert_nonfatal(! in_option_validation);
  return global_options;
}

/** Returns the currently configured options */
MOCK_IMPL(const or_options_t *,
get_options,(void))
{
  return get_options_mutable();
}

/**
 * True iff we have noticed that this is a testing tor network, and we
 * should use the corresponding defaults.
 **/
static bool testing_network_configured = false;

/** Return a set of lines for any default options that we want to override
 * from those set in our config_var_t values. */
static config_line_t *
get_options_defaults(void)
{
  int i;
  config_line_t *result = NULL, **next = &result;

  if (testing_network_configured) {
    for (i = 0; testing_tor_network_defaults[i].k; ++i) {
      config_line_append(next,
                         testing_tor_network_defaults[i].k,
                         testing_tor_network_defaults[i].v);
      next = &(*next)->next;
    }
  }

  return result;
}

/** Change the current global options to contain <b>new_val</b> instead of
 * their current value; take action based on the new value; free the old value
 * as necessary.  Returns 0 on success, -1 on failure.
 */
int
set_options(or_options_t *new_val, char **msg)
{
  or_options_t *old_options = global_options;
  global_options = new_val;
  /* Note that we pass the *old* options below, for comparison. It
   * pulls the new options directly out of global_options. */
  if (options_act_reversible(old_options, msg)<0) {
    tor_assert(*msg);
    global_options = old_options;
    return -1;
  }
  if (subsystems_set_options(get_options_mgr(), new_val) < 0 ||
      options_act(old_options) < 0) { /* acting on the options failed. die. */
    if (! tor_event_loop_shutdown_is_pending()) {
      log_err(LD_BUG,
              "Acting on config options left us in a broken state. Dying.");
      tor_shutdown_event_loop_and_exit(1);
    }
    global_options = old_options;
    return -1;
  }
  /* Issues a CONF_CHANGED event to notify controller of the change. If Tor is
   * just starting up then the old_options will be undefined. */
  if (old_options && old_options != global_options) {
    config_line_t *changes =
      config_get_changes(get_options_mgr(), old_options, new_val);
    control_event_conf_changed(changes);
    config_free_lines(changes);
  }

  if (old_options != global_options) {
    or_options_free(old_options);
    /* If we are here it means we've successfully applied the new options and
     * that the global options have been changed to the new values. We'll
     * check if we need to remove or add periodic events. */
    periodic_events_on_new_options(global_options);
  }

  return 0;
}

/** Release additional memory allocated in options
 */
static void
options_clear_cb(const config_mgr_t *mgr, void *opts)
{
  (void)mgr;
  CHECK_OPTIONS_MAGIC(opts);
  or_options_t *options = opts;

  routerset_free(options->ExcludeExitNodesUnion_);
  if (options->NodeFamilySets) {
    SMARTLIST_FOREACH(options->NodeFamilySets, routerset_t *,
                      rs, routerset_free(rs));
    smartlist_free(options->NodeFamilySets);
  }
  if (options->SchedulerTypes_) {
    SMARTLIST_FOREACH(options->SchedulerTypes_, int *, i, tor_free(i));
    smartlist_free(options->SchedulerTypes_);
  }
  if (options->FilesOpenedByIncludes) {
    SMARTLIST_FOREACH(options->FilesOpenedByIncludes, char *, f, tor_free(f));
    smartlist_free(options->FilesOpenedByIncludes);
  }
  tor_free(options->DataDirectory);
  tor_free(options->CacheDirectory);
  tor_free(options->KeyDirectory);
  tor_free(options->BridgePassword_AuthDigest_);
  tor_free(options->command_arg);
  tor_free(options->master_key_fname);
  config_free_lines(options->MyFamily);
}

/** Release all memory allocated in options
 */
STATIC void
or_options_free_(or_options_t *options)
{
  config_free(get_options_mgr(), options);
}

/** Release all memory and resources held by global configuration structures.
 */
void
config_free_all(void)
{
  or_options_free(global_options);
  global_options = NULL;
  or_options_free(global_default_options);
  global_default_options = NULL;

  parsed_cmdline_free(global_cmdline);

  if (configured_ports) {
    SMARTLIST_FOREACH(configured_ports,
                      port_cfg_t *, p, port_cfg_free(p));
    smartlist_free(configured_ports);
    configured_ports = NULL;
  }

  tor_free(torrc_fname);
  tor_free(torrc_defaults_fname);

  cleanup_protocol_warning_severity_level();

  have_set_startup_options = false;

  config_mgr_free(options_mgr);
}

/** Make <b>address</b> -- a piece of information related to our operation as
 * a client -- safe to log according to the settings in options->SafeLogging,
 * and return it.
 *
 * (We return "[scrubbed]" if SafeLogging is "1", and address otherwise.)
 */
const char *
safe_str_client_opts(const or_options_t *options, const char *address)
{
  tor_assert(address);
  if (!options) {
    options = get_options();
  }

  if (options->SafeLogging_ == SAFELOG_SCRUB_ALL)
    return "[scrubbed]";
  else
    return address;
}

/** Make <b>address</b> -- a piece of information of unspecified sensitivity
 * -- safe to log according to the settings in options->SafeLogging, and
 * return it.
 *
 * (We return "[scrubbed]" if SafeLogging is anything besides "0", and address
 * otherwise.)
 */
const char *
safe_str_opts(const or_options_t *options, const char *address)
{
  tor_assert(address);
  if (!options) {
    options = get_options();
  }

  if (options->SafeLogging_ != SAFELOG_SCRUB_NONE)
    return "[scrubbed]";
  else
    return address;
}

/** Equivalent to escaped(safe_str_client(address)).  See reentrancy note on
 * escaped(): don't use this outside the main thread, or twice in the same
 * log statement. */
const char *
escaped_safe_str_client(const char *address)
{
  if (get_options()->SafeLogging_ == SAFELOG_SCRUB_ALL)
    return "[scrubbed]";
  else
    return escaped(address);
}

/** Equivalent to escaped(safe_str(address)).  See reentrancy note on
 * escaped(): don't use this outside the main thread, or twice in the same
 * log statement. */
const char *
escaped_safe_str(const char *address)
{
  if (get_options()->SafeLogging_ != SAFELOG_SCRUB_NONE)
    return "[scrubbed]";
  else
    return escaped(address);
}

/**
 * The severity level that should be used for warnings of severity
 * LOG_PROTOCOL_WARN.
 *
 * We keep this outside the options, and we use an atomic_counter_t, in case
 * one thread needs to use LOG_PROTOCOL_WARN while an option transition is
 * happening in the main thread.
 */
static atomic_counter_t protocol_warning_severity_level;

/** Return the severity level that should be used for warnings of severity
 * LOG_PROTOCOL_WARN. */
int
get_protocol_warning_severity_level(void)
{
  return (int) atomic_counter_get(&protocol_warning_severity_level);
}

/** Set the protocol warning severity level to <b>severity</b>. */
static void
set_protocol_warning_severity_level(int warning_severity)
{
  atomic_counter_exchange(&protocol_warning_severity_level,
                          warning_severity);
}

/**
 * Initialize the log warning severity level for protocol warnings. Call
 * only once at startup.
 */
void
init_protocol_warning_severity_level(void)
{
  atomic_counter_init(&protocol_warning_severity_level);
  set_protocol_warning_severity_level(LOG_WARN);
}

/**
 * Tear down protocol_warning_severity_level.
 */
static void
cleanup_protocol_warning_severity_level(void)
{
  /* Destroying a locked mutex is undefined behaviour. This mutex may be
   * locked, because multiple threads can access it. But we need to destroy
   * it, otherwise re-initialisation will trigger undefined behaviour.
   * See #31735 for details. */
  atomic_counter_destroy(&protocol_warning_severity_level);
}

/** Add the default directory authorities directly into the trusted dir list,
 * but only add them insofar as they share bits with <b>type</b>.
 * Each authority's bits are restricted to the bits shared with <b>type</b>.
 * If <b>type</b> is ALL_DIRINFO or NO_DIRINFO (zero), add all authorities. */
STATIC void
add_default_trusted_dir_authorities(dirinfo_type_t type)
{
  int i;
  for (i=0; default_authorities[i]; i++) {
    if (parse_dir_authority_line(default_authorities[i], type, 0)<0) {
      log_err(LD_BUG, "Couldn't parse internal DirAuthority line %s",
              default_authorities[i]);
    }
  }
}

/** Add the default fallback directory servers into the fallback directory
 * server list. */
MOCK_IMPL(void,
add_default_fallback_dir_servers,(void))
{
  int i;
  for (i=0; default_fallbacks[i]; i++) {
    if (parse_dir_fallback_line(default_fallbacks[i], 0)<0) {
      log_err(LD_BUG, "Couldn't parse internal FallbackDir line %s",
              default_fallbacks[i]);
    }
  }
}

/** Look at all the config options for using alternate directory
 * authorities, and make sure none of them are broken. Also, warn the
 * user if we changed any dangerous ones.
 */
static int
validate_dir_servers(const or_options_t *options,
                     const or_options_t *old_options)
{
  config_line_t *cl;

  if (options->DirAuthorities &&
      (options->AlternateDirAuthority || options->AlternateBridgeAuthority)) {
    log_warn(LD_CONFIG,
             "You cannot set both DirAuthority and Alternate*Authority.");
    return -1;
  }

  /* do we want to complain to the user about being partitionable? */
  if ((options->DirAuthorities &&
       (!old_options ||
        !config_lines_eq(options->DirAuthorities,
                         old_options->DirAuthorities))) ||
      (options->AlternateDirAuthority &&
       (!old_options ||
        !config_lines_eq(options->AlternateDirAuthority,
                         old_options->AlternateDirAuthority)))) {
    log_warn(LD_CONFIG,
             "You have used DirAuthority or AlternateDirAuthority to "
             "specify alternate directory authorities in "
             "your configuration. This is potentially dangerous: it can "
             "make you look different from all other Tor users, and hurt "
             "your anonymity. Even if you've specified the same "
             "authorities as Tor uses by default, the defaults could "
             "change in the future. Be sure you know what you're doing.");
  }

  /* Now go through the four ways you can configure an alternate
   * set of directory authorities, and make sure none are broken. */
  for (cl = options->DirAuthorities; cl; cl = cl->next)
    if (parse_dir_authority_line(cl->value, NO_DIRINFO, 1)<0)
      return -1;
  for (cl = options->AlternateBridgeAuthority; cl; cl = cl->next)
    if (parse_dir_authority_line(cl->value, NO_DIRINFO, 1)<0)
      return -1;
  for (cl = options->AlternateDirAuthority; cl; cl = cl->next)
    if (parse_dir_authority_line(cl->value, NO_DIRINFO, 1)<0)
      return -1;
  for (cl = options->FallbackDir; cl; cl = cl->next)
    if (parse_dir_fallback_line(cl->value, 1)<0)
      return -1;
  return 0;
}

/** Look at all the config options and assign new dir authorities
 * as appropriate.
 */
int
consider_adding_dir_servers(const or_options_t *options,
                            const or_options_t *old_options)
{
  config_line_t *cl;
  int need_to_update =
    !smartlist_len(router_get_trusted_dir_servers()) ||
    !smartlist_len(router_get_fallback_dir_servers()) || !old_options ||
    !config_lines_eq(options->DirAuthorities, old_options->DirAuthorities) ||
    !config_lines_eq(options->FallbackDir, old_options->FallbackDir) ||
    (options->UseDefaultFallbackDirs != old_options->UseDefaultFallbackDirs) ||
    !config_lines_eq(options->AlternateBridgeAuthority,
                     old_options->AlternateBridgeAuthority) ||
    !config_lines_eq(options->AlternateDirAuthority,
                     old_options->AlternateDirAuthority);

  if (!need_to_update)
    return 0; /* all done */

  /* "You cannot set both DirAuthority and Alternate*Authority."
   * Checking that this restriction holds allows us to simplify
   * the unit tests. */
  tor_assert(!(options->DirAuthorities &&
               (options->AlternateDirAuthority
                || options->AlternateBridgeAuthority)));

  /* Start from a clean slate. */
  clear_dir_servers();

  if (!options->DirAuthorities) {
    /* then we may want some of the defaults */
    dirinfo_type_t type = NO_DIRINFO;
    if (!options->AlternateBridgeAuthority) {
      type |= BRIDGE_DIRINFO;
    }
    if (!options->AlternateDirAuthority) {
      type |= V3_DIRINFO | EXTRAINFO_DIRINFO | MICRODESC_DIRINFO;
      /* Only add the default fallback directories when the DirAuthorities,
       * AlternateDirAuthority, and FallbackDir directory config options
       * are set to their defaults, and when UseDefaultFallbackDirs is 1. */
      if (!options->FallbackDir && options->UseDefaultFallbackDirs) {
        add_default_fallback_dir_servers();
      }
    }
    /* if type == NO_DIRINFO, we don't want to add any of the
     * default authorities, because we've replaced them all */
    if (type != NO_DIRINFO)
      add_default_trusted_dir_authorities(type);
  }

  for (cl = options->DirAuthorities; cl; cl = cl->next)
    if (parse_dir_authority_line(cl->value, NO_DIRINFO, 0)<0)
      return -1;
  for (cl = options->AlternateBridgeAuthority; cl; cl = cl->next)
    if (parse_dir_authority_line(cl->value, NO_DIRINFO, 0)<0)
      return -1;
  for (cl = options->AlternateDirAuthority; cl; cl = cl->next)
    if (parse_dir_authority_line(cl->value, NO_DIRINFO, 0)<0)
      return -1;
  for (cl = options->FallbackDir; cl; cl = cl->next)
    if (parse_dir_fallback_line(cl->value, 0)<0)
      return -1;
  return 0;
}

/**
 * Make sure that <b>directory</b> exists, with appropriate ownership and
 * permissions (as modified by <b>group_readable</b>). If <b>create</b>,
 * create the directory if it is missing. Return 0 on success.
 * On failure, return -1 and set *<b>msg_out</b>.
 */
static int
check_and_create_data_directory(int create,
                                const char *directory,
                                int group_readable,
                                const char *owner,
                                char **msg_out)
{
  cpd_check_t cpd_opts = create ? CPD_CREATE : CPD_CHECK;
  if (group_readable)
      cpd_opts |= CPD_GROUP_READ;
  if (check_private_dir(directory,
                        cpd_opts,
                        owner) < 0) {
    tor_asprintf(msg_out,
                 "Couldn't %s private data directory \"%s\"",
                 create ? "create" : "access",
                 directory);
    return -1;
  }

#ifndef _WIN32
  if (group_readable) {
    /* Only new dirs created get new opts, also enforce group read. */
    if (chmod(directory, 0750)) {
      log_warn(LD_FS,"Unable to make %s group-readable: %s",
               directory, strerror(errno));
    }
  }
#endif /* !defined(_WIN32) */

  return 0;
}

/**
 * Ensure that our keys directory exists, with appropriate permissions.
 * Return 0 on success, -1 on failure.
 */
int
create_keys_directory(const or_options_t *options)
{
  /* Make sure DataDirectory exists, and is private. */
  cpd_check_t cpd_opts = CPD_CREATE;
  if (options->DataDirectoryGroupReadable)
    cpd_opts |= CPD_GROUP_READ;
  if (check_private_dir(options->DataDirectory, cpd_opts, options->User)) {
    log_err(LD_OR, "Can't create/check datadirectory %s",
            options->DataDirectory);
    return -1;
  }

  /* Check the key directory. */
  if (check_private_dir(options->KeyDirectory, CPD_CREATE, options->User)) {
    return -1;
  }
  return 0;
}

/* Helps determine flags to pass to switch_id. */
static int have_low_ports = -1;

/** Take case of initial startup tasks that must occur before any of the
 * transactional option-related changes are allowed. */
static int
options_act_once_on_startup(char **msg_out)
{
  if (have_set_startup_options)
    return 0;

  const or_options_t *options = get_options();
  const bool running_tor = options->command == CMD_RUN_TOR;

  if (!running_tor)
    return 0;

  /* Daemonize _first_, since we only want to open most of this stuff in
   * the subprocess.  Libevent bases can't be reliably inherited across
   * processes. */
  if (options->RunAsDaemon) {
    if (! start_daemon_has_been_called())
      subsystems_prefork();
    /* No need to roll back, since you can't change the value. */
    if (start_daemon())
      subsystems_postfork();
  }

#ifdef HAVE_SYSTEMD
  /* Our PID may have changed, inform supervisor */
  sd_notifyf(0, "MAINPID=%ld\n", (long int)getpid());
#endif

  /* Set up libevent.  (We need to do this before we can register the
   * listeners as listeners.) */
  init_libevent(options);

  /* This has to come up after libevent is initialized. */
  control_initialize_event_queue();

  /*
   * Initialize the scheduler - this has to come after
   * options_init_from_torrc() sets up libevent - why yes, that seems
   * completely sensible to hide the libevent setup in the option parsing
   * code!  It also needs to happen before init_keys(), so it needs to
   * happen here too.  How yucky. */
  scheduler_init();

  /* Attempt to lock all current and future memory with mlockall() only once.
   * This must happen before setuid. */
  if (options->DisableAllSwap) {
    if (tor_mlockall() == -1) {
      *msg_out = tor_strdup("DisableAllSwap failure. Do you have proper "
                        "permissions?");
      return -1;
    }
  }

  have_set_startup_options = true;
  return 0;
}

/**
 * Change our user ID if we're configured to do so.
 **/
static int
options_switch_id(char **msg_out)
{
  const or_options_t *options = get_options();

  /* Setuid/setgid as appropriate */
  if (options->User) {
    tor_assert(have_low_ports != -1);
    unsigned switch_id_flags = 0;
    if (options->KeepBindCapabilities == 1) {
      switch_id_flags |= SWITCH_ID_KEEP_BINDLOW;
      switch_id_flags |= SWITCH_ID_WARN_IF_NO_CAPS;
    }
    if (options->KeepBindCapabilities == -1 && have_low_ports) {
      switch_id_flags |= SWITCH_ID_KEEP_BINDLOW;
    }
    if (switch_id(options->User, switch_id_flags) != 0) {
      /* No need to roll back, since you can't change the value. */
      *msg_out = tor_strdup("Problem with User value. See logs for details.");
      return -1;
    }
  }

  return 0;
}

/**
 * Helper. Given a data directory (<b>datadir</b>) and another directory
 * (<b>subdir</b>) with respective group-writable permissions
 * <b>datadir_gr</b> and <b>subdir_gr</b>, compute whether the subdir should
 * be group-writeable.
 **/
static int
compute_group_readable_flag(const char *datadir,
                            const char *subdir,
                            int datadir_gr,
                            int subdir_gr)
{
  if (subdir_gr != -1) {
    /* The user specified a default for "subdir", so we always obey it. */
    return subdir_gr;
  }

  /* The user left the subdir_gr option on "auto." */
  if (0 == strcmp(subdir, datadir)) {
    /* The directories are the same, so we use the group-readable flag from
     * the datadirectory */
    return datadir_gr;
  } else {
    /* The directories are different, so we default to "not group-readable" */
    return 0;
  }
}

/**
 * Create our DataDirectory, CacheDirectory, and KeyDirectory, and
 * set their permissions correctly.
 */
STATIC int
options_create_directories(char **msg_out)
{
  const or_options_t *options = get_options();
  const bool running_tor = options->command == CMD_RUN_TOR;

  /* Ensure data directory is private; create if possible. */
  /* It's okay to do this in "options_act_reversible()" even though it isn't
   * actually reversible, since you can't change the DataDirectory while
   * Tor is running. */
  if (check_and_create_data_directory(running_tor /* create */,
                                      options->DataDirectory,
                                      options->DataDirectoryGroupReadable,
                                      options->User,
                                      msg_out) < 0) {
    return -1;
  }

  /* We need to handle the group-readable flag for the cache directory and key
   * directory specially, since they may be the same as the data directory */
  const int key_dir_group_readable = compute_group_readable_flag(
                                        options->DataDirectory,
                                        options->KeyDirectory,
                                        options->DataDirectoryGroupReadable,
                                        options->KeyDirectoryGroupReadable);

  if (check_and_create_data_directory(running_tor /* create */,
                                      options->KeyDirectory,
                                      key_dir_group_readable,
                                      options->User,
                                      msg_out) < 0) {
    return -1;
  }

  const int cache_dir_group_readable = compute_group_readable_flag(
                                        options->DataDirectory,
                                        options->CacheDirectory,
                                        options->DataDirectoryGroupReadable,
                                        options->CacheDirectoryGroupReadable);

  if (check_and_create_data_directory(running_tor /* create */,
                                      options->CacheDirectory,
                                      cache_dir_group_readable,
                                      options->User,
                                      msg_out) < 0) {
    return -1;
  }

  return 0;
}

/** Structure to represent an incomplete configuration of a set of
 * listeners.
 *
 * This structure is generated by options_start_listener_transaction(), and is
 * either committed by options_commit_listener_transaction() or rolled back by
 * options_rollback_listener_transaction(). */
typedef struct listener_transaction_t {
  bool set_conn_limit; /**< True if we've set the connection limit */
  unsigned old_conn_limit; /**< If nonzero, previous connlimit value. */
  smartlist_t *new_listeners; /**< List of new listeners that we opened. */
} listener_transaction_t;

/**
 * Start configuring our listeners based on the current value of
 * get_options().
 *
 * The value <b>old_options</b> holds either the previous options object,
 * or NULL if we're starting for the first time.
 *
 * On success, return a listener_transaction_t that we can either roll back or
 * commit.
 *
 * On failure return NULL and write a message into a newly allocated string in
 * *<b>msg_out</b>.
 **/
static listener_transaction_t *
options_start_listener_transaction(const or_options_t *old_options,
                                   char **msg_out)
{
  listener_transaction_t *xn = tor_malloc_zero(sizeof(listener_transaction_t));
  xn->new_listeners = smartlist_new();
  or_options_t *options = get_options_mutable();
  const bool running_tor = options->command == CMD_RUN_TOR;

  if (! running_tor) {
    return xn;
  }

  int n_ports=0;
  /* We need to set the connection limit before we can open the listeners. */
  if (! sandbox_is_active()) {
    if (set_max_file_descriptors((unsigned)options->ConnLimit,
                                 &options->ConnLimit_) < 0) {
      *msg_out = tor_strdup("Problem with ConnLimit value. "
                            "See logs for details.");
      goto rollback;
    }
    xn->set_conn_limit = true;
    if (old_options)
      xn->old_conn_limit = (unsigned)old_options->ConnLimit;
  } else {
    tor_assert(old_options);
    options->ConnLimit_ = old_options->ConnLimit_;
  }

  /* Adjust the port configuration so we can launch listeners. */
  /* 31851: some ports are relay-only */
  if (parse_ports(options, 0, msg_out, &n_ports, NULL)) {
    if (!*msg_out)
      *msg_out = tor_strdup("Unexpected problem parsing port config");
    goto rollback;
  }

  /* Set the hibernation state appropriately.*/
  consider_hibernation(time(NULL));

  /* Launch the listeners.  (We do this before we setuid, so we can bind to
   * ports under 1024.)  We don't want to rebind if we're hibernating or
   * shutting down. If networking is disabled, this will close all but the
   * control listeners, but disable those. */
  /* 31851: some listeners are relay-only */
  if (!we_are_hibernating()) {
    if (retry_all_listeners(xn->new_listeners,
                            options->DisableNetwork) < 0) {
      *msg_out = tor_strdup("Failed to bind one of the listener ports.");
      goto rollback;
    }
  }
  if (options->DisableNetwork) {
    /* Aggressively close non-controller stuff, NOW */
    log_notice(LD_NET, "DisableNetwork is set. Tor will not make or accept "
               "non-control network connections. Shutting down all existing "
               "connections.");
    connection_mark_all_noncontrol_connections();
    /* We can't complete circuits until the network is re-enabled. */
    note_that_we_maybe_cant_complete_circuits();
  }

#if defined(HAVE_NET_IF_H) && defined(HAVE_NET_PFVAR_H)
  /* Open /dev/pf before (possibly) dropping privileges. */
  if (options->TransPort_set &&
      options->TransProxyType_parsed == TPT_DEFAULT) {
    if (get_pf_socket() < 0) {
      *msg_out = tor_strdup("Unable to open /dev/pf for transparent proxy.");
      goto rollback;
    }
  }
#endif /* defined(HAVE_NET_IF_H) && defined(HAVE_NET_PFVAR_H) */

  return xn;

 rollback:
  options_rollback_listener_transaction(xn);
  return NULL;
}

/**
 * Finish configuring the listeners that started to get configured with
 * <b>xn</b>.  Frees <b>xn</b>.
 **/
static void
options_commit_listener_transaction(listener_transaction_t *xn)
{
  tor_assert(xn);
  if (xn->set_conn_limit) {
    or_options_t *options = get_options_mutable();
    /*
     * If we adjusted the conn limit, recompute the OOS threshold too
     *
     * How many possible sockets to keep in reserve?  If we have lots of
     * possible sockets, keep this below a limit and set ConnLimit_high_thresh
     * very close to ConnLimit_, but if ConnLimit_ is low, shrink it in
     * proportion.
     *
     * Somewhat arbitrarily, set socks_in_reserve to 5% of ConnLimit_, but
     * cap it at 64.
     */
    int socks_in_reserve = options->ConnLimit_ / 20;
    if (socks_in_reserve > 64) socks_in_reserve = 64;

    options->ConnLimit_high_thresh = options->ConnLimit_ - socks_in_reserve;
    options->ConnLimit_low_thresh = (options->ConnLimit_ / 4) * 3;
    log_info(LD_GENERAL,
             "Recomputed OOS thresholds: ConnLimit %d, ConnLimit_ %d, "
             "ConnLimit_high_thresh %d, ConnLimit_low_thresh %d",
             options->ConnLimit, options->ConnLimit_,
             options->ConnLimit_high_thresh,
             options->ConnLimit_low_thresh);

    /* Give the OOS handler a chance with the new thresholds */
    connection_check_oos(get_n_open_sockets(), 0);
  }

  smartlist_free(xn->new_listeners);
  tor_free(xn);
}

/**
 * Revert the listener configuration changes that that started to get
 * configured with <b>xn</b>.  Frees <b>xn</b>.
 **/
static void
options_rollback_listener_transaction(listener_transaction_t *xn)
{
  if (! xn)
    return;

  or_options_t *options = get_options_mutable();

  if (xn->set_conn_limit && xn->old_conn_limit)
    set_max_file_descriptors(xn->old_conn_limit, &options->ConnLimit_);

  SMARTLIST_FOREACH(xn->new_listeners, connection_t *, conn,
  {
    log_notice(LD_NET, "Closing partially-constructed %s",
               connection_describe(conn));
    connection_close_immediate(conn);
    connection_mark_for_close(conn);
  });

  smartlist_free(xn->new_listeners);
  tor_free(xn);
}

/** Structure to represent an incomplete configuration of a set of logs.
 *
 * This structure is generated by options_start_log_transaction(), and is
 * either committed by options_commit_log_transaction() or rolled back by
 * options_rollback_log_transaction(). */
typedef struct log_transaction_t {
  /** Previous lowest severity of any configured log. */
  int old_min_log_level;
  /** True if we have marked the previous logs to be closed */
  bool logs_marked;
  /** True if we initialized the new set of logs */
  bool logs_initialized;
  /** True if our safelogging configuration is different from what it was
   * previously (or if we are starting for the first time). */
  bool safelogging_changed;
} log_transaction_t;

/**
 * Start configuring our logs based on the current value of get_options().
 *
 * The value <b>old_options</b> holds either the previous options object,
 * or NULL if we're starting for the first time.
 *
 * On success, return a log_transaction_t that we can either roll back or
 * commit.
 *
 * On failure return NULL and write a message into a newly allocated string in
 * *<b>msg_out</b>.
 **/
STATIC log_transaction_t *
options_start_log_transaction(const or_options_t *old_options,
                              char **msg_out)
{
  const or_options_t *options = get_options();
  const bool running_tor = options->command == CMD_RUN_TOR;

  log_transaction_t *xn = tor_malloc_zero(sizeof(log_transaction_t));
  xn->old_min_log_level = get_min_log_level();
  xn->safelogging_changed = !old_options ||
    old_options->SafeLogging_ != options->SafeLogging_;

  if (! running_tor)
    goto done;

  mark_logs_temp(); /* Close current logs once new logs are open. */
  xn->logs_marked = true;
  /* Configure the tor_log(s) */
  if (options_init_logs(old_options, options, 0)<0) {
    *msg_out = tor_strdup("Failed to init Log options. See logs for details.");
    options_rollback_log_transaction(xn);
    xn = NULL;
    goto done;
  }

  xn->logs_initialized = true;

 done:
  return xn;
}

/**
 * Finish configuring the logs that started to get configured with <b>xn</b>.
 * Frees <b>xn</b>.
 **/
STATIC void
options_commit_log_transaction(log_transaction_t *xn)
{
  const or_options_t *options = get_options();
  tor_assert(xn);

  if (xn->logs_marked) {
    log_severity_list_t *severity =
      tor_malloc_zero(sizeof(log_severity_list_t));
    close_temp_logs();
    add_callback_log(severity, control_event_logmsg);
    logs_set_pending_callback_callback(control_event_logmsg_pending);
    control_adjust_event_log_severity();
    tor_free(severity);
    tor_log_update_sigsafe_err_fds();
  }

  if (xn->logs_initialized) {
    flush_log_messages_from_startup();
  }

  {
    const char *badness = NULL;
    int bad_safelog = 0, bad_severity = 0, new_badness = 0;
    if (options->SafeLogging_ != SAFELOG_SCRUB_ALL) {
      bad_safelog = 1;
      if (xn->safelogging_changed)
        new_badness = 1;
    }
    if (get_min_log_level() >= LOG_INFO) {
      bad_severity = 1;
      if (get_min_log_level() != xn->old_min_log_level)
        new_badness = 1;
    }
    if (bad_safelog && bad_severity)
      badness = "you disabled SafeLogging, and "
        "you're logging more than \"notice\"";
    else if (bad_safelog)
      badness = "you disabled SafeLogging";
    else
      badness = "you're logging more than \"notice\"";
    if (new_badness)
      log_warn(LD_GENERAL, "Your log may contain sensitive information - %s. "
               "Don't log unless it serves an important reason. "
               "Overwrite the log afterwards.", badness);
  }

  tor_free(xn);
}

/**
 * Revert the log configuration changes that that started to get configured
 * with <b>xn</b>.  Frees <b>xn</b>.
 **/
STATIC void
options_rollback_log_transaction(log_transaction_t *xn)
{
  if (!xn)
    return;

  if (xn->logs_marked) {
    rollback_log_changes();
    control_adjust_event_log_severity();
  }

  tor_free(xn);
}

/**
 * Fetch the active option list, and take actions based on it. All of
 * the things we do in this function should survive being done
 * repeatedly, OR be done only once when starting Tor.  If present,
 * <b>old_options</b> contains the previous value of the options.
 *
 * This function is only truly "reversible" _after_ the first time it
 * is run.  The first time that it runs, it performs some irreversible
 * tasks in the correct sequence between the reversible option changes.
 *
 * Option changes should only be marked as "reversible" if they cannot
 * be validated before switching them, but they can be switched back if
 * some other validation fails.
 *
 * Return 0 if all goes well, return -1 if things went badly.
 */
MOCK_IMPL(STATIC int,
options_act_reversible,(const or_options_t *old_options, char **msg))
{
  const bool first_time = ! have_set_startup_options;
  log_transaction_t *log_transaction = NULL;
  listener_transaction_t *listener_transaction = NULL;
  int r = -1;

  /* The ordering of actions in this function is not free, sadly.
   *
   * First of all, we _must_ daemonize before we take all kinds of
   * initialization actions, since they need to happen in the
   * subprocess.
   */
  if (options_act_once_on_startup(msg) < 0)
    goto rollback;

  /* Once we've handled most of once-off initialization, we need to
   * open our listeners before we switch IDs.  (If we open listeners first,
   * we might not be able to bind to low ports.)
   */
  listener_transaction = options_start_listener_transaction(old_options, msg);
  if (listener_transaction == NULL)
    goto rollback;

  if (first_time) {
    if (options_switch_id(msg) < 0)
      goto rollback;
  }

  /* On the other hand, we need to touch the file system _after_ we
   * switch IDs: otherwise, we'll be making directories and opening files
   * with the wrong permissions.
   */
  if (first_time) {
    if (options_create_directories(msg) < 0)
      goto rollback;
  }

  /* Bail out at this point if we're not going to be a client or server:
   * we don't run Tor itself. */
  log_transaction = options_start_log_transaction(old_options, msg);
  if (log_transaction == NULL)
    goto rollback;

  // Commit!
  r = 0;

  options_commit_log_transaction(log_transaction);

  options_commit_listener_transaction(listener_transaction);

  goto done;

 rollback:
  r = -1;
  tor_assert(*msg);

  options_rollback_log_transaction(log_transaction);
  options_rollback_listener_transaction(listener_transaction);

 done:
  return r;
}

/** If we need to have a GEOIP ip-to-country map to run with our configured
 * options, return 1 and set *<b>reason_out</b> to a description of why. */
int
options_need_geoip_info(const or_options_t *options, const char **reason_out)
{
  int bridge_usage = should_record_bridge_info(options);
  int routerset_usage =
    routerset_needs_geoip(options->EntryNodes) ||
    routerset_needs_geoip(options->ExitNodes) ||
    routerset_needs_geoip(options->MiddleNodes) ||
    routerset_needs_geoip(options->ExcludeExitNodes) ||
    routerset_needs_geoip(options->ExcludeNodes) ||
    routerset_needs_geoip(options->HSLayer2Nodes) ||
    routerset_needs_geoip(options->HSLayer3Nodes);

  if (routerset_usage && reason_out) {
    *reason_out = "We've been configured to use (or avoid) nodes in certain "
      "countries, and we need GEOIP information to figure out which ones they "
      "are.";
  } else if (bridge_usage && reason_out) {
    *reason_out = "We've been configured to see which countries can access "
      "us as a bridge, and we need GEOIP information to tell which countries "
      "clients are in.";
  }
  return bridge_usage || routerset_usage;
}

/* Used in the various options_transition_affects* functions. */
#define YES_IF_CHANGED_BOOL(opt) \
  if (!CFG_EQ_BOOL(old_options, new_options, opt)) return 1;
#define YES_IF_CHANGED_INT(opt) \
  if (!CFG_EQ_INT(old_options, new_options, opt)) return 1;
#define YES_IF_CHANGED_STRING(opt) \
  if (!CFG_EQ_STRING(old_options, new_options, opt)) return 1;
#define YES_IF_CHANGED_LINELIST(opt) \
  if (!CFG_EQ_LINELIST(old_options, new_options, opt)) return 1;
#define YES_IF_CHANGED_SMARTLIST(opt) \
  if (!CFG_EQ_SMARTLIST(old_options, new_options, opt)) return 1;
#define YES_IF_CHANGED_ROUTERSET(opt) \
  if (!CFG_EQ_ROUTERSET(old_options, new_options, opt)) return 1;

/**
 * Return true if changing the configuration from <b>old</b> to <b>new</b>
 * affects the guard subsystem.
 */
static int
options_transition_affects_guards(const or_options_t *old_options,
                                  const or_options_t *new_options)
{
  /* NOTE: Make sure this function stays in sync with
   * node_passes_guard_filter */
  tor_assert(old_options);
  tor_assert(new_options);

  YES_IF_CHANGED_BOOL(UseEntryGuards);
  YES_IF_CHANGED_BOOL(UseBridges);
  YES_IF_CHANGED_BOOL(ClientUseIPv4);
  YES_IF_CHANGED_BOOL(ClientUseIPv6);
  YES_IF_CHANGED_BOOL(FascistFirewall);
  YES_IF_CHANGED_ROUTERSET(ExcludeNodes);
  YES_IF_CHANGED_ROUTERSET(EntryNodes);
  YES_IF_CHANGED_SMARTLIST(FirewallPorts);
  YES_IF_CHANGED_LINELIST(Bridges);
  YES_IF_CHANGED_LINELIST(ReachableORAddresses);
  YES_IF_CHANGED_LINELIST(ReachableDirAddresses);

  return 0;
}

/** Fetch the active option list, and take actions based on it. All of the
 * things we do should survive being done repeatedly.  If present,
 * <b>old_options</b> contains the previous value of the options.
 *
 * Return 0 if all goes well, return -1 if it's time to die.
 *
 * Note: We haven't moved all the "act on new configuration" logic
 * the options_act* functions yet.  Some is still in do_hup() and other
 * places.
 */
MOCK_IMPL(STATIC int,
options_act,(const or_options_t *old_options))
{
  config_line_t *cl;
  or_options_t *options = get_options_mutable();
  int running_tor = options->command == CMD_RUN_TOR;
  char *msg=NULL;
  const int transition_affects_guards =
    old_options && options_transition_affects_guards(old_options, options);

  if (options->NoExec || options->Sandbox) {
    tor_disable_spawning_background_processes();
  }

  /* disable ptrace and later, other basic debugging techniques */
  {
    /* Remember if we already disabled debugger attachment */
    static int disabled_debugger_attach = 0;
    /* Remember if we already warned about being configured not to disable
     * debugger attachment */
    static int warned_debugger_attach = 0;
    /* Don't disable debugger attachment when we're running the unit tests. */
    if (options->DisableDebuggerAttachment && !disabled_debugger_attach &&
        running_tor) {
      int ok = tor_disable_debugger_attach();
      /* LCOV_EXCL_START the warned_debugger_attach is 0 can't reach inside. */
      if (warned_debugger_attach && ok == 1) {
        log_notice(LD_CONFIG, "Disabled attaching debuggers for unprivileged "
                   "users.");
      }
      /* LCOV_EXCL_STOP */
      disabled_debugger_attach = (ok == 1);
    } else if (!options->DisableDebuggerAttachment &&
               !warned_debugger_attach) {
      log_notice(LD_CONFIG, "Not disabling debugger attaching for "
                 "unprivileged users.");
      warned_debugger_attach = 1;
    }
  }

  /* Write control ports to disk as appropriate */
  control_ports_write_to_file();

  if (running_tor && !have_lockfile()) {
    if (try_locking(options, 1) < 0)
      return -1;
  }

  {
    int warning_severity = options->ProtocolWarnings ? LOG_WARN : LOG_INFO;
    set_protocol_warning_severity_level(warning_severity);
  }

  if (consider_adding_dir_servers(options, old_options) < 0) {
    // XXXX This should get validated earlier, and committed here, to
    // XXXX lower opportunities for reaching an error case.
    return -1;
  }

  if (hs_service_non_anonymous_mode_enabled(options)) {
    log_warn(LD_GENERAL, "This copy of Tor was compiled or configured to run "
             "in a non-anonymous mode. It will provide NO ANONYMITY.");
  }

  /* 31851: OutboundBindAddressExit is relay-only */
  if (parse_outbound_addresses(options, 0, &msg) < 0) {
    // LCOV_EXCL_START
    log_warn(LD_BUG, "Failed parsing previously validated outbound "
             "bind addresses: %s", msg);
    tor_free(msg);
    return -1;
    // LCOV_EXCL_STOP
  }

  if (options->Bridges) {
    mark_bridge_list();
    for (cl = options->Bridges; cl; cl = cl->next) {
      bridge_line_t *bridge_line = parse_bridge_line(cl->value);
      if (!bridge_line) {
        // LCOV_EXCL_START
        log_warn(LD_BUG,
                 "Previously validated Bridge line could not be added!");
        return -1;
        // LCOV_EXCL_STOP
      }
      bridge_add_from_config(bridge_line);
    }
    sweep_bridge_list();
  }

  if (running_tor && hs_config_service_all(options, 0)<0) {
    // LCOV_EXCL_START
    log_warn(LD_BUG,
       "Previously validated hidden services line could not be added!");
    return -1;
    // LCOV_EXCL_STOP
  }

  if (running_tor && hs_config_client_auth_all(options, 0) < 0) {
    // LCOV_EXCL_START
    log_warn(LD_BUG, "Previously validated client authorization for "
                     "hidden services could not be added!");
    return -1;
    // LCOV_EXCL_STOP
  }

  if (running_tor && !old_options &&
      options->OwningControllerFD != UINT64_MAX) {
    const unsigned ctrl_flags =
      CC_LOCAL_FD_IS_OWNER |
      CC_LOCAL_FD_IS_AUTHENTICATED;
    tor_socket_t ctrl_sock = (tor_socket_t)options->OwningControllerFD;
    if (control_connection_add_local_fd(ctrl_sock, ctrl_flags) < 0) {
      log_warn(LD_CONFIG, "Could not add local controller connection with "
               "given FD.");
      return -1;
    }
  }

  /* Load state */
  if (! or_state_loaded() && running_tor) {
    if (or_state_load())
      return -1;
    if (options_act_dirauth_mtbf(options) < 0)
      return -1;
  }

  /* 31851: some of the code in these functions is relay-only */
  mark_transport_list();
  pt_prepare_proxy_list_for_config_read();
  if (!options->DisableNetwork) {
    if (options->ClientTransportPlugin) {
      for (cl = options->ClientTransportPlugin; cl; cl = cl->next) {
        if (pt_parse_transport_line(options, cl->value, 0, 0) < 0) {
          // LCOV_EXCL_START
          log_warn(LD_BUG,
                   "Previously validated ClientTransportPlugin line "
                   "could not be added!");
          return -1;
          // LCOV_EXCL_STOP
        }
      }
    }
  }

  if (options_act_server_transport(old_options) < 0)
    return -1;

  sweep_transport_list();
  sweep_proxy_list();

  /* Start the PT proxy configuration. By doing this configuration
     here, we also figure out which proxies need to be restarted and
     which not. */
  if (pt_proxies_configuration_pending() && !net_is_disabled())
    pt_configure_remaining_proxies();

  /* Bail out at this point if we're not going to be a client or server:
   * we want to not fork, and to log stuff to stderr. */
  if (!running_tor)
    return 0;

  /* Finish backgrounding the process */
  if (options->RunAsDaemon) {
    /* We may be calling this for the n'th time (on SIGHUP), but it's safe. */
    finish_daemon(options->DataDirectory);
  }

  if (options_act_relay(old_options) < 0)
    return -1;

  /* Write our PID to the PID file. If we do not have write permissions we
   * will log a warning and exit. */
  if (options->PidFile && !sandbox_is_active()) {
    if (write_pidfile(options->PidFile) < 0) {
      log_err(LD_CONFIG, "Unable to write PIDFile %s",
              escaped(options->PidFile));
      return -1;
    }
  }

  /* Register addressmap directives */
  config_register_addressmaps(options);
  parse_virtual_addr_network(options->VirtualAddrNetworkIPv4, AF_INET,0,NULL);
  parse_virtual_addr_network(options->VirtualAddrNetworkIPv6, AF_INET6,0,NULL);

  /* Update address policies. */
  if (policies_parse_from_options(options) < 0) {
    /* This should be impossible, but let's be sure. */
    log_warn(LD_BUG,"Error parsing already-validated policy options.");
    return -1;
  }

  if (init_control_cookie_authentication(options->CookieAuthentication) < 0) {
    log_warn(LD_CONFIG,"Error creating control cookie authentication file.");
    return -1;
  }

  monitor_owning_controller_process(options->OwningControllerProcess);

  /* reload keys as needed for rendezvous services. */
  if (hs_service_load_all_keys() < 0) {
    log_warn(LD_GENERAL,"Error loading rendezvous service keys");
    return -1;
  }

  /* Inform the scheduler subsystem that a configuration changed happened. It
   * might be a change of scheduler or parameter. */
  scheduler_conf_changed();

  if (options_act_relay_accounting(old_options) < 0)
    return -1;

  /* Change the cell EWMA settings */
  cmux_ewma_set_options(options, networkstatus_get_latest_consensus());

  /* Update the BridgePassword's hashed version as needed.  We store this as a
   * digest so that we can do side-channel-proof comparisons on it.
   */
  if (options->BridgePassword) {
    char *http_authenticator;
    http_authenticator = alloc_http_authenticator(options->BridgePassword);
    if (!http_authenticator) {
      // XXXX This should get validated in options_validate().
      log_warn(LD_BUG, "Unable to allocate HTTP authenticator. Not setting "
               "BridgePassword.");
      return -1;
    }
    options->BridgePassword_AuthDigest_ = tor_malloc(DIGEST256_LEN);
    crypto_digest256(options->BridgePassword_AuthDigest_,
                     http_authenticator, strlen(http_authenticator),
                     DIGEST_SHA256);
    tor_free(http_authenticator);
  }

  config_maybe_load_geoip_files_(options, old_options);

  if (geoip_is_loaded(AF_INET) && options->GeoIPExcludeUnknown) {
    /* ExcludeUnknown is true or "auto" */
    const int is_auto = options->GeoIPExcludeUnknown == -1;
    int changed;

    changed  = routerset_add_unknown_ccs(&options->ExcludeNodes, is_auto);
    changed += routerset_add_unknown_ccs(&options->ExcludeExitNodes, is_auto);

    if (changed)
      routerset_add_unknown_ccs(&options->ExcludeExitNodesUnion_, is_auto);
  }

  /* Check for transitions that need action. */
  if (old_options) {
    int revise_trackexithosts = 0;
    int revise_automap_entries = 0;
    int abandon_circuits = 0;
    if ((options->UseEntryGuards && !old_options->UseEntryGuards) ||
        options->UseBridges != old_options->UseBridges ||
        (options->UseBridges &&
         !config_lines_eq(options->Bridges, old_options->Bridges)) ||
        !routerset_equal(old_options->ExcludeNodes,options->ExcludeNodes) ||
        !routerset_equal(old_options->ExcludeExitNodes,
                         options->ExcludeExitNodes) ||
        !routerset_equal(old_options->EntryNodes, options->EntryNodes) ||
        !routerset_equal(old_options->ExitNodes, options->ExitNodes) ||
        !routerset_equal(old_options->HSLayer2Nodes,
                         options->HSLayer2Nodes) ||
        !routerset_equal(old_options->HSLayer3Nodes,
                         options->HSLayer3Nodes) ||
        !routerset_equal(old_options->MiddleNodes, options->MiddleNodes) ||
        options->StrictNodes != old_options->StrictNodes) {
      log_info(LD_CIRC,
               "Changed to using entry guards or bridges, or changed "
               "preferred or excluded node lists. "
               "Abandoning previous circuits.");
      abandon_circuits = 1;
    }

    if (transition_affects_guards) {
      if (options->ReconfigDropsBridgeDescs)
        routerlist_drop_bridge_descriptors();
      if (guards_update_all()) {
        abandon_circuits = 1;
      }
    }

    if (abandon_circuits) {
      circuit_mark_all_unused_circs();
      circuit_mark_all_dirty_circs_as_unusable();
      revise_trackexithosts = 1;
    }

    if (!smartlist_strings_eq(old_options->TrackHostExits,
                              options->TrackHostExits))
      revise_trackexithosts = 1;

    if (revise_trackexithosts)
      addressmap_clear_excluded_trackexithosts(options);

    if (!options->AutomapHostsOnResolve &&
        old_options->AutomapHostsOnResolve) {
        revise_automap_entries = 1;
    } else {
      if (!smartlist_strings_eq(old_options->AutomapHostsSuffixes,
                                options->AutomapHostsSuffixes))
        revise_automap_entries = 1;
      else if (!opt_streq(old_options->VirtualAddrNetworkIPv4,
                          options->VirtualAddrNetworkIPv4) ||
               !opt_streq(old_options->VirtualAddrNetworkIPv6,
                          options->VirtualAddrNetworkIPv6))
        revise_automap_entries = 1;
    }

    if (revise_automap_entries)
      addressmap_clear_invalid_automaps(options);

    if (options_act_bridge_stats(old_options) < 0)
      return -1;

    if (dns_reset())
      return -1;

    if (options_act_relay_bandwidth(old_options) < 0)
      return -1;

    if (options->BandwidthRate != old_options->BandwidthRate ||
        options->BandwidthBurst != old_options->BandwidthBurst)
      connection_bucket_adjust(options);

    if (options->MainloopStats != old_options->MainloopStats) {
      reset_main_loop_counters();
    }
  }

  /* 31851: These options are relay-only, but we need to disable them if we
   * are in client mode. In 29211, we will disable all relay options in
   * client mode. */
  /* Only collect directory-request statistics on relays and bridges. */
  options->DirReqStatistics = options->DirReqStatistics_option &&
    server_mode(options);
  options->HiddenServiceStatistics =
    options->HiddenServiceStatistics_option && server_mode(options);

  /* Only collect other relay-only statistics on relays. */
  if (!public_server_mode(options)) {
    options->CellStatistics = 0;
    options->EntryStatistics = 0;
    options->ConnDirectionStatistics = 0;
    options->ExitPortStatistics = 0;
  }

  bool print_notice = 0;
  if (options_act_relay_stats(old_options, &print_notice) < 0)
    return -1;
  if (options_act_dirauth_stats(old_options, &print_notice) < 0)
    return -1;
  if (print_notice)
    options_act_relay_stats_msg();

  if (options_act_relay_desc(old_options) < 0)
    return -1;

  if (options_act_dirauth(old_options) < 0)
    return -1;

  /* We may need to reschedule some directory stuff if our status changed. */
  if (old_options) {
    if (!bool_eq(dirclient_fetches_dir_info_early(options),
                 dirclient_fetches_dir_info_early(old_options)) ||
        !bool_eq(dirclient_fetches_dir_info_later(options),
                 dirclient_fetches_dir_info_later(old_options)) ||
        !config_lines_eq(old_options->Bridges, options->Bridges)) {
      /* Make sure update_router_have_minimum_dir_info() gets called. */
      router_dir_info_changed();
      /* We might need to download a new consensus status later or sooner than
       * we had expected. */
      update_consensus_networkstatus_fetch_time(time(NULL));
    }
  }

  if (options_act_relay_dos(old_options) < 0)
    return -1;
  if (options_act_relay_dir(old_options) < 0)
    return -1;

  return 0;
}

/**
 * Enumeration to describe the syntax for a command-line option.
 **/
typedef enum {
  /** Describe an option that does not take an argument. */
  ARGUMENT_NONE = 0,
  /** Describes an option that takes a single argument. */
  ARGUMENT_NECESSARY = 1,
  /** Describes an option that takes a single optional argument. */
  ARGUMENT_OPTIONAL = 2
} takes_argument_t;

/** Table describing arguments that Tor accepts on the command line,
 * other than those that are the same as in torrc. */
static const struct {
  /** The string that the user has to provide. */
  const char *name;
  /** Optional short name. */
  const char *short_name;
  /** Does this option accept an argument? */
  takes_argument_t takes_argument;
  /** If not CMD_RUN_TOR, what should Tor do when it starts? */
  tor_cmdline_mode_t command;
  /** If nonzero, set the quiet level to this. 1 is "hush", 2 is "quiet" */
  int quiet;
} CMDLINE_ONLY_OPTIONS[] = {
  { .name="--torrc-file",
    .short_name="-f",
    .takes_argument=ARGUMENT_NECESSARY },
  { .name="--allow-missing-torrc" },
  { .name="--defaults-torrc",
    .takes_argument=ARGUMENT_NECESSARY },
  { .name="--hash-password",
    .takes_argument=ARGUMENT_NECESSARY,
    .command=CMD_HASH_PASSWORD,
    .quiet=QUIET_HUSH  },
  { .name="--dump-config",
    .takes_argument=ARGUMENT_OPTIONAL,
    .command=CMD_DUMP_CONFIG,
    .quiet=QUIET_SILENT },
  { .name="--list-fingerprint",
    .takes_argument=ARGUMENT_OPTIONAL,
    .command=CMD_LIST_FINGERPRINT },
  { .name="--keygen",
    .command=CMD_KEYGEN },
  { .name="--key-expiration",
    .takes_argument=ARGUMENT_OPTIONAL,
    .command=CMD_KEY_EXPIRATION },
  { .name="--format",
    .takes_argument=ARGUMENT_NECESSARY },
  { .name="--newpass" },
  { .name="--no-passphrase" },
  { .name="--passphrase-fd",
    .takes_argument=ARGUMENT_NECESSARY },
  { .name="--verify-config",
    .command=CMD_VERIFY_CONFIG },
  { .name="--ignore-missing-torrc" },
  { .name="--quiet",
    .quiet=QUIET_SILENT },
  { .name="--hush",
    .quiet=QUIET_HUSH },
  { .name="--version",
    .command=CMD_IMMEDIATE,
    .quiet=QUIET_HUSH },
  { .name="--list-modules",
    .command=CMD_IMMEDIATE,
    .quiet=QUIET_HUSH },
  { .name="--library-versions",
    .command=CMD_IMMEDIATE,
    .quiet=QUIET_HUSH },
  { .name="--help",
    .short_name="-h",
    .command=CMD_IMMEDIATE,
    .quiet=QUIET_HUSH  },
  { .name="--list-torrc-options",
    .command=CMD_IMMEDIATE,
    .quiet=QUIET_HUSH },
  { .name="--list-deprecated-options",
    .command=CMD_IMMEDIATE },
  { .name="--nt-service" },
  { .name="-nt-service" },
  { .name="--dbg-dump-subsystem-list",
    .command=CMD_IMMEDIATE,
    .quiet=QUIET_HUSH },
  { .name=NULL },
};

/** Helper: Read a list of configuration options from the command line.  If
 * successful, return a newly allocated parsed_cmdline_t; otherwise return
 * NULL.
 *
 * If <b>ignore_errors</b> is set, try to recover from all recoverable
 * errors and return the best command line we can.
 */
parsed_cmdline_t *
config_parse_commandline(int argc, char **argv, int ignore_errors)
{
  parsed_cmdline_t *result = tor_malloc_zero(sizeof(parsed_cmdline_t));
  result->command = CMD_RUN_TOR;
  config_line_t *param = NULL;

  config_line_t **new_cmdline = &result->cmdline_opts;
  config_line_t **new = &result->other_opts;

  char *s, *arg;
  int i = 1;

  while (i < argc) {
    unsigned command = CONFIG_LINE_NORMAL;
    takes_argument_t want_arg = ARGUMENT_NECESSARY;
    int is_cmdline = 0;
    int j;
    bool is_a_command = false;

    for (j = 0; CMDLINE_ONLY_OPTIONS[j].name != NULL; ++j) {
      if (!strcmp(argv[i], CMDLINE_ONLY_OPTIONS[j].name) ||
          (CMDLINE_ONLY_OPTIONS[j].short_name &&
           !strcmp(argv[i], CMDLINE_ONLY_OPTIONS[j].short_name))) {
        is_cmdline = 1;
        want_arg = CMDLINE_ONLY_OPTIONS[j].takes_argument;
        if (CMDLINE_ONLY_OPTIONS[j].command != CMD_RUN_TOR) {
          is_a_command = true;
          result->command = CMDLINE_ONLY_OPTIONS[j].command;
        }
        quiet_level_t quiet = CMDLINE_ONLY_OPTIONS[j].quiet;
        if (quiet > result->quiet_level)
          result->quiet_level = quiet;
        break;
      }
    }

    s = argv[i];

    /* Each keyword may be prefixed with one or two dashes. */
    if (*s == '-')
      s++;
    if (*s == '-')
      s++;
    /* Figure out the command, if any. */
    if (*s == '+') {
      s++;
      command = CONFIG_LINE_APPEND;
    } else if (*s == '/') {
      s++;
      command = CONFIG_LINE_CLEAR;
      /* A 'clear' command has no argument. */
      want_arg = 0;
    }

    const int is_last = (i == argc-1);

    if (want_arg == ARGUMENT_NECESSARY && is_last) {
      if (ignore_errors) {
        arg = tor_strdup("");
      } else {
        log_warn(LD_CONFIG,"Command-line option '%s' with no value. Failing.",
            argv[i]);
        parsed_cmdline_free(result);
        return NULL;
      }
    } else if (want_arg == ARGUMENT_OPTIONAL &&
               /* optional arguments may never start with '-'. */
               (is_last || argv[i+1][0] == '-')) {
      arg = tor_strdup("");
      want_arg = ARGUMENT_NONE; // prevent skipping the next flag.
    } else {
      arg = (want_arg != ARGUMENT_NONE) ? tor_strdup(argv[i+1]) :
                                              tor_strdup("");
    }

    param = tor_malloc_zero(sizeof(config_line_t));
    param->key = is_cmdline ? tor_strdup(argv[i]) :
                 tor_strdup(config_expand_abbrev(get_options_mgr(), s, 1, 1));
    param->value = arg;
    param->command = command;
    param->next = NULL;
    log_debug(LD_CONFIG, "command line: parsed keyword '%s', value '%s'",
        param->key, param->value);

    if (is_a_command) {
      result->command_arg = param->value;
    }

    if (is_cmdline) {
      *new_cmdline = param;
      new_cmdline = &((*new_cmdline)->next);
    } else {
      *new = param;
      new = &((*new)->next);
    }

    i += want_arg ? 2 : 1;
  }

  return result;
}

/** Release all storage held by <b>cmdline</b>. */
void
parsed_cmdline_free_(parsed_cmdline_t *cmdline)
{
  if (!cmdline)
    return;
  config_free_lines(cmdline->cmdline_opts);
  config_free_lines(cmdline->other_opts);
  tor_free(cmdline);
}

/** Return true iff key is a valid configuration option. */
int
option_is_recognized(const char *key)
{
  return config_find_option_name(get_options_mgr(), key) != NULL;
}

/** Return the canonical name of a configuration option, or NULL
 * if no such option exists. */
const char *
option_get_canonical_name(const char *key)
{
  return config_find_option_name(get_options_mgr(), key);
}

/** Return a canonical list of the options assigned for key.
 */
config_line_t *
option_get_assignment(const or_options_t *options, const char *key)
{
  return config_get_assigned_option(get_options_mgr(), options, key, 1);
}

/** Try assigning <b>list</b> to the global options. You do this by duping
 * options, assigning list to the new one, then validating it. If it's
 * ok, then throw out the old one and stick with the new one. Else,
 * revert to old and return failure.  Return SETOPT_OK on success, or
 * a setopt_err_t on failure.
 *
 * If not success, point *<b>msg</b> to a newly allocated string describing
 * what went wrong.
 */
setopt_err_t
options_trial_assign(config_line_t *list, unsigned flags, char **msg)
{
  int r;
  or_options_t *trial_options = config_dup(get_options_mgr(), get_options());

  if ((r=config_assign(get_options_mgr(), trial_options,
                       list, flags, msg)) < 0) {
    or_options_free(trial_options);
    return r;
  }
  const or_options_t *cur_options = get_options();

  return options_validate_and_set(cur_options, trial_options, msg);
}

/** Print a usage message for tor. */
static void
print_usage(void)
{
  printf(
"Copyright (c) 2001-2004, Roger Dingledine\n"
"Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson\n"
"Copyright (c) 2007-2021, The Tor Project, Inc.\n\n"
"tor -f <torrc> [args]\n"
"See man page for options, or https://www.torproject.org/ for "
"documentation.\n");
}

/** Print all non-obsolete torrc options. */
static void
list_torrc_options(void)
{
  smartlist_t *vars = config_mgr_list_vars(get_options_mgr());
  SMARTLIST_FOREACH_BEGIN(vars, const config_var_t *, var) {
    /* Possibly this should check listable, rather than (or in addition to)
     * settable. See ticket 31654.
     */
    if (! config_var_is_settable(var)) {
      /* This variable cannot be set, or cannot be set by this name. */
      continue;
    }
    printf("%s\n", var->member.name);
  } SMARTLIST_FOREACH_END(var);
  smartlist_free(vars);
}

/** Print all deprecated but non-obsolete torrc options. */
static void
list_deprecated_options(void)
{
  smartlist_t *deps = config_mgr_list_deprecated_vars(get_options_mgr());
  /* Possibly this should check whether the variables are listable,
   * but currently it does not.  See ticket 31654. */
  SMARTLIST_FOREACH(deps, const char *, name,
                    printf("%s\n", name));
  smartlist_free(deps);
}

/** Print all compile-time modules and their enabled/disabled status. */
static void
list_enabled_modules(void)
{
  static const struct {
    const char *name;
    bool have;
  } list[] = {
    { "relay", have_module_relay() },
    { "dirauth", have_module_dirauth() },
    { "dircache", have_module_dircache() },
    { "pow", have_module_pow() }
  };

  for (unsigned i = 0; i < sizeof list / sizeof list[0]; i++) {
    printf("%s: %s\n", list[i].name, list[i].have ? "yes" : "no");
  }
}

/** Prints compile-time and runtime library versions. */
static void
print_library_versions(void)
{
  printf("Tor version %s. \n", get_version());
  printf("Library versions\tCompiled\t\tRuntime\n");
  printf("Libevent\t\t%-15s\t\t%s\n",
                    tor_libevent_get_header_version_str(),
                    tor_libevent_get_version_str());
#ifdef ENABLE_OPENSSL
  printf("OpenSSL \t\t%-15s\t\t%s\n",
                     crypto_openssl_get_header_version_str(),
                     crypto_openssl_get_version_str());
#endif
#ifdef ENABLE_NSS
  printf("NSS \t\t%-15s\t\t%s\n",
                crypto_nss_get_header_version_str(),
                crypto_nss_get_version_str());
#endif
  if (tor_compress_supports_method(ZLIB_METHOD)) {
    printf("Zlib    \t\t%-15s\t\t%s\n",
                      tor_compress_version_str(ZLIB_METHOD),
                      tor_compress_header_version_str(ZLIB_METHOD));
  }
  if (tor_compress_supports_method(LZMA_METHOD)) {
    printf("Liblzma \t\t%-15s\t\t%s\n",
                      tor_compress_version_str(LZMA_METHOD),
                      tor_compress_header_version_str(LZMA_METHOD));
  }
  if (tor_compress_supports_method(ZSTD_METHOD)) {
    printf("Libzstd \t\t%-15s\t\t%s\n",
                      tor_compress_version_str(ZSTD_METHOD),
                      tor_compress_header_version_str(ZSTD_METHOD));
  }
  if (tor_libc_get_name()) {
    printf("%-7s \t\t%-15s\t\t%s\n",
           tor_libc_get_name(),
           tor_libc_get_header_version_str(),
           tor_libc_get_version_str());
  }
  //TODO: Hex versions?
}

/** Handles the --no-passphrase command line option. */
static int
handle_cmdline_no_passphrase(tor_cmdline_mode_t command)
{
  if (command == CMD_KEYGEN) {
    get_options_mutable()->keygen_force_passphrase = FORCE_PASSPHRASE_OFF;
    return 0;
  } else {
    log_err(LD_CONFIG, "--no-passphrase specified without --keygen!");
    return -1;
  }
}

/** Handles the --format command line option. */
static int
handle_cmdline_format(tor_cmdline_mode_t command, const char *value)
{
  if (command == CMD_KEY_EXPIRATION) {
    // keep the same order as enum key_expiration_format
    const char *formats[] = { "iso8601", "timestamp" };
    int format = -1;
    for (unsigned i = 0; i < ARRAY_LENGTH(formats); i++) {
      if (!strcmp(value, formats[i])) {
        format = i;
        break;
      }
    }

    if (format < 0) {
      log_err(LD_CONFIG, "Invalid --format value %s", escaped(value));
      return -1;
    } else {
      get_options_mutable()->key_expiration_format = format;
    }
    return 0;
  } else {
    log_err(LD_CONFIG, "--format specified without --key-expiration!");
    return -1;
  }
}

/** Handles the --newpass command line option. */
static int
handle_cmdline_newpass(tor_cmdline_mode_t command)
{
  if (command == CMD_KEYGEN) {
    get_options_mutable()->change_key_passphrase = 1;
    return 0;
  } else {
    log_err(LD_CONFIG, "--newpass specified without --keygen!");
    return -1;
  }
}

/** Handles the --passphrase-fd command line option. */
static int
handle_cmdline_passphrase_fd(tor_cmdline_mode_t command, const char *value)
{
  if (get_options()->keygen_force_passphrase == FORCE_PASSPHRASE_OFF) {
    log_err(LD_CONFIG, "--no-passphrase specified with --passphrase-fd!");
    return -1;
  } else if (command != CMD_KEYGEN) {
    log_err(LD_CONFIG, "--passphrase-fd specified without --keygen!");
    return -1;
  } else {
    int ok = 1;
    long fd = tor_parse_long(value, 10, 0, INT_MAX, &ok, NULL);
    if (fd < 0 || ok == 0) {
      log_err(LD_CONFIG, "Invalid --passphrase-fd value %s", escaped(value));
      return -1;
    }
    get_options_mutable()->keygen_passphrase_fd = (int)fd;
    get_options_mutable()->use_keygen_passphrase_fd = 1;
    get_options_mutable()->keygen_force_passphrase = FORCE_PASSPHRASE_ON;
    return 0;
  }
}

/** Handles the --master-key command line option. */
static int
handle_cmdline_master_key(tor_cmdline_mode_t command, const char *value)
{
  if (command != CMD_KEYGEN) {
    log_err(LD_CONFIG, "--master-key without --keygen!");
    return -1;
  } else {
    get_options_mutable()->master_key_fname = tor_strdup(value);
    return 0;
  }
}

/* Return true if <b>options</b> is using the default authorities, and false
 * if any authority-related option has been overridden. */
int
using_default_dir_authorities(const or_options_t *options)
{
  return (!options->DirAuthorities && !options->AlternateDirAuthority);
}

/** Return a new empty or_options_t.  Used for testing. */
or_options_t *
options_new(void)
{
  or_options_t *options = config_new(get_options_mgr());
  options->command = CMD_RUN_TOR;
  return options;
}

/** Set <b>options</b> to hold reasonable defaults for most options.
 * Each option defaults to zero. */
void
options_init(or_options_t *options)
{
  config_init(get_options_mgr(), options);
  config_line_t *dflts = get_options_defaults();
  char *msg=NULL;
  if (config_assign(get_options_mgr(), options, dflts,
                    CAL_WARN_DEPRECATIONS, &msg)<0) {
    log_err(LD_BUG, "Unable to set default options: %s", msg);
    tor_free(msg);
    tor_assert_unreached();
  }
  config_free_lines(dflts);
  tor_free(msg);
}

/** Return a string containing a possible configuration file that would give
 * the configuration in <b>options</b>.  If <b>minimal</b> is true, do not
 * include options that are the same as Tor's defaults.
 */
char *
options_dump(const or_options_t *options, int how_to_dump)
{
  const or_options_t *use_defaults;
  int minimal;
  switch (how_to_dump) {
    case OPTIONS_DUMP_MINIMAL:
      use_defaults = global_default_options;
      minimal = 1;
      break;
    case OPTIONS_DUMP_ALL:
      use_defaults = NULL;
      minimal = 0;
      break;
    default:
      log_warn(LD_BUG, "Bogus value for how_to_dump==%d", how_to_dump);
      return NULL;
  }

  return config_dump(get_options_mgr(), use_defaults, options, minimal, 0);
}

/** Return 0 if every element of sl is a string holding a decimal
 * representation of a port number, or if sl is NULL.
 * Otherwise set *msg and return -1. */
static int
validate_ports_csv(smartlist_t *sl, const char *name, char **msg)
{
  int i;
  tor_assert(name);

  if (!sl)
    return 0;

  SMARTLIST_FOREACH(sl, const char *, cp,
  {
    i = atoi(cp);
    if (i < 1 || i > 65535) {
      tor_asprintf(msg, "Port '%s' out of range in %s", cp, name);
      return -1;
    }
  });
  return 0;
}

/** If <b>value</b> exceeds ROUTER_MAX_DECLARED_BANDWIDTH, write
 * a complaint into *<b>msg</b> using string <b>desc</b>, and return -1.
 * Else return 0.
 */
int
config_ensure_bandwidth_cap(uint64_t *value, const char *desc, char **msg)
{
  if (*value > ROUTER_MAX_DECLARED_BANDWIDTH) {
    /* This handles an understandable special case where somebody says "2gb"
     * whereas our actual maximum is 2gb-1 (INT_MAX) */
    --*value;
  }
  if (*value > ROUTER_MAX_DECLARED_BANDWIDTH) {
    tor_asprintf(msg, "%s (%"PRIu64") must be at most %d",
                 desc, (*value),
                 ROUTER_MAX_DECLARED_BANDWIDTH);
    return -1;
  }
  return 0;
}

/** Highest allowable value for CircuitsAvailableTimeout.
 * If this is too large, client connections will stay open for too long,
 * incurring extra padding overhead. */
#define MAX_CIRCS_AVAILABLE_TIME (24*60*60)

/** Lowest allowable value for MaxCircuitDirtiness; if this is too low, Tor
 * will generate too many circuits and potentially overload the network. */
#define MIN_MAX_CIRCUIT_DIRTINESS 10

/** Highest allowable value for MaxCircuitDirtiness: prevents time_t
 * overflows. */
#define MAX_MAX_CIRCUIT_DIRTINESS (30*24*60*60)

/** Lowest allowable value for CircuitStreamTimeout; if this is too low, Tor
 * will generate too many circuits and potentially overload the network. */
#define MIN_CIRCUIT_STREAM_TIMEOUT 10

/** Lowest recommended value for CircuitBuildTimeout; if it is set too low
 * and LearnCircuitBuildTimeout is off, the failure rate for circuit
 * construction may be very high.  In that case, if it is set below this
 * threshold emit a warning.
 * */
#define RECOMMENDED_MIN_CIRCUIT_BUILD_TIMEOUT (10)

/**
 * Validate <b>new_options</b>.  If it is valid, and it is a reasonable
 * replacement for <b>old_options</b>, replace the previous value of the
 * global options, and return return SETOPT_OK.
 *
 * If it is not valid, then free <b>new_options</b>, set *<b>msg_out</b> to a
 * newly allocated error message, and return an error code.
 */
static setopt_err_t
options_validate_and_set(const or_options_t *old_options,
                         or_options_t *new_options,
                         char **msg_out)
{
  setopt_err_t rv;
  validation_status_t vs;

  in_option_validation = 1;
  vs = config_validate(get_options_mgr(), old_options, new_options, msg_out);

  if (vs == VSTAT_TRANSITION_ERR) {
    rv = SETOPT_ERR_TRANSITION;
    goto err;
  } else if (vs < 0) {
    rv = SETOPT_ERR_PARSE;
    goto err;
  }
  in_option_validation = 0;

  if (set_options(new_options, msg_out)) {
    rv = SETOPT_ERR_SETTING;
    goto err;
  }

  rv = SETOPT_OK;
  new_options = NULL; /* prevent free */
 err:
  in_option_validation = 0;
  tor_assert(new_options == NULL || rv != SETOPT_OK);
  or_options_free(new_options);
  return rv;
}

#ifdef TOR_UNIT_TESTS
/**
 * Return 0 if every setting in <b>options</b> is reasonable, is a
 * permissible transition from <b>old_options</b>, and none of the
 * testing-only settings differ from <b>default_options</b> unless in
 * testing mode.  Else return -1.  Should have no side effects, except for
 * normalizing the contents of <b>options</b>.
 *
 * On error, tor_strdup an error explanation into *<b>msg</b>.
 */
int
options_validate(const or_options_t *old_options, or_options_t *options,
                 char **msg)
{
  validation_status_t vs;
  vs = config_validate(get_options_mgr(), old_options, options, msg);
  return vs < 0 ? -1 : 0;
}
#endif /* defined(TOR_UNIT_TESTS) */

#define REJECT(arg) \
  STMT_BEGIN *msg = tor_strdup(arg); return -1; STMT_END
#if defined(__GNUC__) && __GNUC__ <= 3
#define COMPLAIN(args...) \
  STMT_BEGIN log_warn(LD_CONFIG, args); STMT_END
#else
#define COMPLAIN(args, ...)                                     \
  STMT_BEGIN log_warn(LD_CONFIG, args, ##__VA_ARGS__); STMT_END
#endif /* defined(__GNUC__) && __GNUC__ <= 3 */

/** Log a warning message iff <b>filepath</b> is not absolute.
 * Warning message must contain option name <b>option</b> and
 * an absolute path that <b>filepath</b> will resolve to.
 *
 * In case <b>filepath</b> is absolute, do nothing.
 *
 * Return 1 if there were relative paths; 0 otherwise.
 */
static int
warn_if_option_path_is_relative(const char *option,
                                const char *filepath)
{
  if (filepath && path_is_relative(filepath)) {
    char *abs_path = make_path_absolute(filepath);
    COMPLAIN("Path for %s (%s) is relative and will resolve to %s."
             " Is this what you wanted?", option, filepath, abs_path);
    tor_free(abs_path);
    return 1;
  }
  return 0;
}

/** Scan <b>options</b> for occurrences of relative file/directory
 * paths and log a warning whenever one is found.
 *
 * Return 1 if there were relative paths; 0 otherwise.
 */
static int
warn_about_relative_paths(const or_options_t *options)
{
  tor_assert(options);
  int n = 0;
  const config_mgr_t *mgr = get_options_mgr();

  smartlist_t *vars = config_mgr_list_vars(mgr);
  SMARTLIST_FOREACH_BEGIN(vars, const config_var_t *, cv) {
    config_line_t *line;
    if (cv->member.type != CONFIG_TYPE_FILENAME)
      continue;
    const char *name = cv->member.name;
    line = config_get_assigned_option(mgr, options, name, 0);
    if (line)
      n += warn_if_option_path_is_relative(name, line->value);
    config_free_lines(line);
  } SMARTLIST_FOREACH_END(cv);
  smartlist_free(vars);

  for (config_line_t *hs_line = options->RendConfigLines; hs_line;
       hs_line = hs_line->next) {
    if (!strcasecmp(hs_line->key, "HiddenServiceDir"))
      n += warn_if_option_path_is_relative("HiddenServiceDir",hs_line->value);
  }
  return n != 0;
}

/* Validate options related to the scheduler. From the Schedulers list, the
 * SchedulerTypes_ list is created with int values so once we select the
 * scheduler, which can happen anytime at runtime, we don't have to parse
 * strings and thus be quick.
 *
 * Return 0 on success else -1 and msg is set with an error message. */
static int
options_validate_scheduler(or_options_t *options, char **msg)
{
  tor_assert(options);
  tor_assert(msg);

  if (!options->Schedulers || smartlist_len(options->Schedulers) == 0) {
    REJECT("Empty Schedulers list. Either remove the option so the defaults "
           "can be used or set at least one value.");
  }
  /* Ok, we do have scheduler types, validate them. */
  if (options->SchedulerTypes_) {
    SMARTLIST_FOREACH(options->SchedulerTypes_, int *, iptr, tor_free(iptr));
    smartlist_free(options->SchedulerTypes_);
  }
  options->SchedulerTypes_ = smartlist_new();
  SMARTLIST_FOREACH_BEGIN(options->Schedulers, const char *, type) {
    int *sched_type;
    if (!strcasecmp("KISTLite", type)) {
      sched_type = tor_malloc_zero(sizeof(int));
      *sched_type = SCHEDULER_KIST_LITE;
      smartlist_add(options->SchedulerTypes_, sched_type);
    } else if (!strcasecmp("KIST", type)) {
      sched_type = tor_malloc_zero(sizeof(int));
      *sched_type = SCHEDULER_KIST;
      smartlist_add(options->SchedulerTypes_, sched_type);
    } else if (!strcasecmp("Vanilla", type)) {
      sched_type = tor_malloc_zero(sizeof(int));
      *sched_type = SCHEDULER_VANILLA;
      smartlist_add(options->SchedulerTypes_, sched_type);
    } else {
      tor_asprintf(msg, "Unknown type %s in option Schedulers. "
                        "Possible values are KIST, KISTLite and Vanilla.",
                   escaped(type));
      return -1;
    }
  } SMARTLIST_FOREACH_END(type);

  if (options->KISTSockBufSizeFactor < 0) {
    REJECT("KISTSockBufSizeFactor must be at least 0");
  }

  /* Don't need to validate that the Interval is less than anything because
   * zero is valid and all negative values are valid. */
  if (options->KISTSchedRunInterval > KIST_SCHED_RUN_INTERVAL_MAX) {
    tor_asprintf(msg, "KISTSchedRunInterval must not be more than %d (ms)",
                 KIST_SCHED_RUN_INTERVAL_MAX);
    return -1;
  }

  return 0;
}

/* Validate options related to single onion services.
 * Modifies some options that are incompatible with single onion services.
 * On failure returns -1, and sets *msg to an error string.
 * Returns 0 on success. */
STATIC int
options_validate_single_onion(or_options_t *options, char **msg)
{
  /* The two single onion service options must have matching values. */
  if (options->HiddenServiceSingleHopMode &&
      !options->HiddenServiceNonAnonymousMode) {
    REJECT("HiddenServiceSingleHopMode does not provide any server anonymity. "
           "It must be used with HiddenServiceNonAnonymousMode set to 1.");
  }
  if (options->HiddenServiceNonAnonymousMode &&
      !options->HiddenServiceSingleHopMode) {
    REJECT("HiddenServiceNonAnonymousMode does not provide any server "
           "anonymity. It must be used with HiddenServiceSingleHopMode set to "
           "1.");
  }

  /* Now that we've checked that the two options are consistent, we can safely
   * call the hs_service_* functions that abstract these options. */

  /* If you run an anonymous client with an active Single Onion service, the
   * client loses anonymity. */
  const int client_port_set = (options->SocksPort_set ||
                               options->TransPort_set ||
                               options->NATDPort_set ||
                               options->DNSPort_set ||
                               options->HTTPTunnelPort_set);
  if (hs_service_non_anonymous_mode_enabled(options) && client_port_set) {
    REJECT("HiddenServiceNonAnonymousMode is incompatible with using Tor as "
           "an anonymous client. Please set Socks/Trans/NATD/DNSPort to 0, or "
           "revert HiddenServiceNonAnonymousMode to 0.");
  }

  if (hs_service_allow_non_anonymous_connection(options)
      && options->UseEntryGuards) {
    /* Single Onion services only use entry guards when uploading descriptors;
     * all other connections are one-hop. Further, Single Onions causes the
     * hidden service code to do things which break the path bias
     * detector, and it's far easier to turn off entry guards (and
     * thus the path bias detector with it) than to figure out how to
     * make path bias compatible with single onions.
     */
    log_notice(LD_CONFIG,
               "HiddenServiceSingleHopMode is enabled; disabling "
               "UseEntryGuards.");
    options->UseEntryGuards = 0;
  }

  return 0;
}

/**
 * Legacy validation/normalization callback for or_options_t.  See
 * legacy_validate_fn_t for more information.
 */
static int
options_validate_cb(const void *old_options_, void *options_, char **msg)
{
  if (old_options_)
    CHECK_OPTIONS_MAGIC(old_options_);
  CHECK_OPTIONS_MAGIC(options_);
  const or_options_t *old_options = old_options_;
  or_options_t *options = options_;

  config_line_t *cl;
  int n_ports=0;
  int world_writable_control_socket=0;

  tor_assert(msg);
  *msg = NULL;

  if (parse_ports(options, 1, msg, &n_ports,
                  &world_writable_control_socket) < 0)
    return -1;

#ifndef HAVE_SYS_UN_H
  if (options->ControlSocket || options->ControlSocketsGroupWritable) {
    *msg = tor_strdup("Unix domain sockets (ControlSocket) not supported "
                      "on this OS/with this build.");
    return -1;
  }
#else /* defined(HAVE_SYS_UN_H) */
  if (options->ControlSocketsGroupWritable && !options->ControlSocket) {
    *msg = tor_strdup("Setting ControlSocketsGroupWritable without setting "
                      "a ControlSocket makes no sense.");
    return -1;
  }
#endif /* !defined(HAVE_SYS_UN_H) */

  /* Set UseEntryGuards from the configured value, before we check it below.
   * We change UseEntryGuards when it's incompatible with other options,
   * but leave UseEntryGuards_option with the original value.
   * Always use the value of UseEntryGuards, not UseEntryGuards_option. */
  options->UseEntryGuards = options->UseEntryGuards_option;

  if (options_validate_relay_os(old_options, options, msg) < 0)
    return -1;

  /* 31851: OutboundBindAddressExit is unused in client mode */
  if (parse_outbound_addresses(options, 1, msg) < 0)
    return -1;

  if (validate_data_directories(options)<0)
    REJECT("Invalid DataDirectory");

  /* need to check for relative paths after we populate
   * options->DataDirectory (just above). */
  if (warn_about_relative_paths(options) && options->RunAsDaemon) {
    REJECT("You have specified at least one relative path (see above) "
           "with the RunAsDaemon option. RunAsDaemon is not compatible "
           "with relative paths.");
  }

  if (options_validate_relay_info(old_options, options, msg) < 0)
    return -1;

  /* 31851: this function is currently a no-op in client mode */
  check_network_configuration(server_mode(options));

  /* Validate the tor_log(s) */
  if (options_init_logs(old_options, options, 1)<0)
    REJECT("Failed to validate Log options. See logs for details.");

  /* XXXX require that the only port not be DirPort? */
  /* XXXX require that at least one port be listened-upon. */
  if (n_ports == 0 && !options->RendConfigLines)
    log_warn(LD_CONFIG,
        "SocksPort, TransPort, NATDPort, DNSPort, and ORPort are all "
        "undefined, and there aren't any hidden services configured.  "
        "Tor will still run, but probably won't do anything.");

  options->TransProxyType_parsed = TPT_DEFAULT;
#ifdef USE_TRANSPARENT
  if (options->TransProxyType) {
    if (!strcasecmp(options->TransProxyType, "default")) {
      options->TransProxyType_parsed = TPT_DEFAULT;
    } else if (!strcasecmp(options->TransProxyType, "pf-divert")) {
#if !defined(OpenBSD) && !defined(DARWIN)
      /* Later versions of OS X have pf */
      REJECT("pf-divert is a OpenBSD-specific "
             "and OS X/Darwin-specific feature.");
#else
      options->TransProxyType_parsed = TPT_PF_DIVERT;
#endif /* !defined(OpenBSD) && !defined(DARWIN) */
    } else if (!strcasecmp(options->TransProxyType, "tproxy")) {
#if !defined(__linux__)
      REJECT("TPROXY is a Linux-specific feature.");
#else
      options->TransProxyType_parsed = TPT_TPROXY;
#endif
    } else if (!strcasecmp(options->TransProxyType, "ipfw")) {
#ifndef KERNEL_MAY_SUPPORT_IPFW
      /* Earlier versions of OS X have ipfw */
      REJECT("ipfw is a FreeBSD-specific "
             "and OS X/Darwin-specific feature.");
#else
      options->TransProxyType_parsed = TPT_IPFW;
#endif /* !defined(KERNEL_MAY_SUPPORT_IPFW) */
    } else {
      REJECT("Unrecognized value for TransProxyType");
    }

    if (strcasecmp(options->TransProxyType, "default") &&
        !options->TransPort_set) {
      REJECT("Cannot use TransProxyType without any valid TransPort.");
    }
  }
#else /* !defined(USE_TRANSPARENT) */
  if (options->TransPort_set)
    REJECT("TransPort is disabled in this build.");
#endif /* defined(USE_TRANSPARENT) */

  if (options->TokenBucketRefillInterval <= 0
      || options->TokenBucketRefillInterval > 1000) {
    REJECT("TokenBucketRefillInterval must be between 1 and 1000 inclusive.");
  }

  if (options->AssumeReachable && options->AssumeReachableIPv6 == 0) {
    REJECT("Cannot set AssumeReachable 1 and AssumeReachableIPv6 0.");
  }

  if (options->ExcludeExitNodes || options->ExcludeNodes) {
    options->ExcludeExitNodesUnion_ = routerset_new();
    routerset_union(options->ExcludeExitNodesUnion_,options->ExcludeExitNodes);
    routerset_union(options->ExcludeExitNodesUnion_,options->ExcludeNodes);
  }

  if (options->NodeFamilies) {
    options->NodeFamilySets = smartlist_new();
    for (cl = options->NodeFamilies; cl; cl = cl->next) {
      routerset_t *rs = routerset_new();
      if (routerset_parse(rs, cl->value, cl->key) == 0) {
        smartlist_add(options->NodeFamilySets, rs);
      } else {
        routerset_free(rs);
      }
    }
  }

  if (options->ExcludeNodes && options->StrictNodes) {
    COMPLAIN("You have asked to exclude certain relays from all positions "
             "in your circuits. Expect hidden services and other Tor "
             "features to be broken in unpredictable ways.");
  }

  if (options_validate_dirauth_mode(old_options, options, msg) < 0)
    return -1;

  if (options->FetchDirInfoExtraEarly && !options->FetchDirInfoEarly)
    REJECT("FetchDirInfoExtraEarly requires that you also set "
           "FetchDirInfoEarly");

  if (options->ConnLimit <= 0) {
    tor_asprintf(msg,
        "ConnLimit must be greater than 0, but was set to %d",
        options->ConnLimit);
    return -1;
  }

  if (options->PathsNeededToBuildCircuits >= 0.0) {
    if (options->PathsNeededToBuildCircuits < 0.25) {
      log_warn(LD_CONFIG, "PathsNeededToBuildCircuits is too low. Increasing "
               "to 0.25");
      options->PathsNeededToBuildCircuits = 0.25;
    } else if (options->PathsNeededToBuildCircuits > 0.95) {
      log_warn(LD_CONFIG, "PathsNeededToBuildCircuits is too high. Decreasing "
               "to 0.95");
      options->PathsNeededToBuildCircuits = 0.95;
    }
  }

  if (options->MaxClientCircuitsPending <= 0 ||
      options->MaxClientCircuitsPending > MAX_MAX_CLIENT_CIRCUITS_PENDING) {
    tor_asprintf(msg,
                 "MaxClientCircuitsPending must be between 1 and %d, but "
                 "was set to %d", MAX_MAX_CLIENT_CIRCUITS_PENDING,
                 options->MaxClientCircuitsPending);
    return -1;
  }

  if (validate_ports_csv(options->FirewallPorts, "FirewallPorts", msg) < 0)
    return -1;

  if (validate_ports_csv(options->LongLivedPorts, "LongLivedPorts", msg) < 0)
    return -1;

  if (validate_ports_csv(options->RejectPlaintextPorts,
                         "RejectPlaintextPorts", msg) < 0)
    return -1;

  if (validate_ports_csv(options->WarnPlaintextPorts,
                         "WarnPlaintextPorts", msg) < 0)
    return -1;

  if (options->FascistFirewall && !options->ReachableAddresses) {
    if (options->FirewallPorts && smartlist_len(options->FirewallPorts)) {
      /* We already have firewall ports set, so migrate them to
       * ReachableAddresses, which will set ReachableORAddresses and
       * ReachableDirAddresses if they aren't set explicitly. */
      smartlist_t *instead = smartlist_new();
      config_line_t *new_line = tor_malloc_zero(sizeof(config_line_t));
      new_line->key = tor_strdup("ReachableAddresses");
      /* If we're configured with the old format, we need to prepend some
       * open ports. */
      SMARTLIST_FOREACH(options->FirewallPorts, const char *, portno,
      {
        int p = atoi(portno);
        if (p<0) continue;
        smartlist_add_asprintf(instead, "*:%d", p);
      });
      new_line->value = smartlist_join_strings(instead,",",0,NULL);
      /* These have been deprecated since 0.1.1.5-alpha-cvs */
      log_notice(LD_CONFIG,
          "Converting FascistFirewall and FirewallPorts "
          "config options to new format: \"ReachableAddresses %s\"",
          new_line->value);
      options->ReachableAddresses = new_line;
      SMARTLIST_FOREACH(instead, char *, cp, tor_free(cp));
      smartlist_free(instead);
    } else {
      /* We do not have FirewallPorts set, so add 80 to
       * ReachableDirAddresses, and 443 to ReachableORAddresses. */
      if (!options->ReachableDirAddresses) {
        config_line_t *new_line = tor_malloc_zero(sizeof(config_line_t));
        new_line->key = tor_strdup("ReachableDirAddresses");
        new_line->value = tor_strdup("*:80");
        options->ReachableDirAddresses = new_line;
        log_notice(LD_CONFIG, "Converting FascistFirewall config option "
            "to new format: \"ReachableDirAddresses *:80\"");
      }
      if (!options->ReachableORAddresses) {
        config_line_t *new_line = tor_malloc_zero(sizeof(config_line_t));
        new_line->key = tor_strdup("ReachableORAddresses");
        new_line->value = tor_strdup("*:443");
        options->ReachableORAddresses = new_line;
        log_notice(LD_CONFIG, "Converting FascistFirewall config option "
            "to new format: \"ReachableORAddresses *:443\"");
      }
    }
  }

  if ((options->ReachableAddresses ||
       options->ReachableORAddresses ||
       options->ReachableDirAddresses ||
       options->ClientUseIPv4 == 0) &&
      server_mode(options))
    REJECT("Servers must be able to freely connect to the rest "
           "of the Internet, so they must not set Reachable*Addresses "
           "or FascistFirewall or FirewallPorts or ClientUseIPv4 0.");

  if (options->UseBridges &&
      server_mode(options))
    REJECT("Servers must be able to freely connect to the rest "
           "of the Internet, so they must not set UseBridges.");

  /* If both of these are set, we'll end up with funny behavior where we
   * demand enough entrynodes be up and running else we won't build
   * circuits, yet we never actually use them. */
  if (options->UseBridges && options->EntryNodes)
    REJECT("You cannot set both UseBridges and EntryNodes.");

  /* If we have UseBridges as 1 and UseEntryGuards as 0, we end up bypassing
   * the use of bridges */
  if (options->UseBridges && !options->UseEntryGuards)
    REJECT("Setting UseBridges requires also setting UseEntryGuards.");

  options->MaxMemInQueues =
    compute_real_max_mem_in_queues(options->MaxMemInQueues_raw,
                                   server_mode(options));
  options->MaxMemInQueues_low_threshold = (options->MaxMemInQueues / 4) * 3;

  if (!options->SafeLogging ||
      !strcasecmp(options->SafeLogging, "0")) {
    options->SafeLogging_ = SAFELOG_SCRUB_NONE;
  } else if (!strcasecmp(options->SafeLogging, "relay")) {
    options->SafeLogging_ = SAFELOG_SCRUB_RELAY;
  } else if (!strcasecmp(options->SafeLogging, "1")) {
    options->SafeLogging_ = SAFELOG_SCRUB_ALL;
  } else {
    tor_asprintf(msg,
                     "Unrecognized value '%s' in SafeLogging",
                     escaped(options->SafeLogging));
    return -1;
  }

 options->ConfluxClientUX = CONFLUX_UX_HIGH_THROUGHPUT;
 if (options->ConfluxClientUX_option) {
    if (!strcmp(options->ConfluxClientUX_option, "latency"))
      options->ConfluxClientUX = CONFLUX_UX_MIN_LATENCY;
    else if (!strcmp(options->ConfluxClientUX_option, "throughput"))
      options->ConfluxClientUX = CONFLUX_UX_HIGH_THROUGHPUT;
    else if (!strcmp(options->ConfluxClientUX_option, "latency_lowmem"))
      options->ConfluxClientUX = CONFLUX_UX_LOW_MEM_LATENCY;
    else if (!strcmp(options->ConfluxClientUX_option, "throughput_lowmem"))
      options->ConfluxClientUX = CONFLUX_UX_LOW_MEM_THROUGHPUT;
    else
      REJECT("ConfluxClientUX must be 'latency', 'throughput, "
             "'latency_lowmem', or 'throughput_lowmem'");
  }

  if (options_validate_publish_server(old_options, options, msg) < 0)
    return -1;

  if (options_validate_relay_padding(old_options, options, msg) < 0)
    return -1;

  /* Check the Single Onion Service options */
  if (options_validate_single_onion(options, msg) < 0)
    return -1;

  if (options->CircuitsAvailableTimeout > MAX_CIRCS_AVAILABLE_TIME) {
    // options_t is immutable for new code (the above code is older),
    // so just make the user fix the value themselves rather than
    // silently keep a shadow value lower than what they asked for.
    REJECT("CircuitsAvailableTimeout is too large. Max is 24 hours.");
  }

  if (options->EntryNodes && !options->UseEntryGuards) {
    REJECT("If EntryNodes is set, UseEntryGuards must be enabled.");
  }

  if (!(options->UseEntryGuards) &&
      (options->RendConfigLines != NULL) &&
      !hs_service_allow_non_anonymous_connection(options)) {
    log_warn(LD_CONFIG,
             "UseEntryGuards is disabled, but you have configured one or more "
             "hidden services on this Tor instance.  Your hidden services "
             "will be very easy to locate using a well-known attack -- see "
             "https://freehaven.net/anonbib/#hs-attack06 for details.");
  }

  if (options->NumPrimaryGuards && options->NumEntryGuards &&
      options->NumEntryGuards > options->NumPrimaryGuards) {
    REJECT("NumEntryGuards must not be greater than NumPrimaryGuards.");
  }

  if (options->EntryNodes &&
      routerset_is_list(options->EntryNodes) &&
      (routerset_len(options->EntryNodes) == 1) &&
      (options->RendConfigLines != NULL)) {
    tor_asprintf(msg,
             "You have one single EntryNodes and at least one hidden service "
             "configured. This is bad because it's very easy to locate your "
             "entry guard which can then lead to the deanonymization of your "
             "hidden service -- for more details, see "
             "https://bugs.torproject.org/tpo/core/tor/14917. "
             "For this reason, the use of one EntryNodes with an hidden "
             "service is prohibited until a better solution is found.");
    return -1;
  }

  /* Inform the hidden service operator that pinning EntryNodes can possibly
   * be harmful for the service anonymity. */
  if (options->EntryNodes &&
      routerset_is_list(options->EntryNodes) &&
      (options->RendConfigLines != NULL)) {
    log_warn(LD_CONFIG,
             "EntryNodes is set with multiple entries and at least one "
             "hidden service is configured. Pinning entry nodes can possibly "
             "be harmful to the service anonymity. Because of this, we "
             "recommend you either don't do that or make sure you know what "
             "you are doing. For more details, please look at "
             "https://bugs.torproject.org/tpo/core/tor/21155.");
  }

  /* Single Onion Services: non-anonymous hidden services */
  if (hs_service_non_anonymous_mode_enabled(options)) {
    log_warn(LD_CONFIG,
             "HiddenServiceNonAnonymousMode is set. Every hidden service on "
             "this tor instance is NON-ANONYMOUS. If "
             "the HiddenServiceNonAnonymousMode option is changed, Tor will "
             "refuse to launch hidden services from the same directories, to "
             "protect your anonymity against config errors. This setting is "
             "for experimental use only.");
  }

  if (!options->LearnCircuitBuildTimeout && options->CircuitBuildTimeout &&
      options->CircuitBuildTimeout < RECOMMENDED_MIN_CIRCUIT_BUILD_TIMEOUT) {
    log_warn(LD_CONFIG,
        "CircuitBuildTimeout is shorter (%d seconds) than the recommended "
        "minimum (%d seconds), and LearnCircuitBuildTimeout is disabled.  "
        "If tor isn't working, raise this value or enable "
        "LearnCircuitBuildTimeout.",
        options->CircuitBuildTimeout,
        RECOMMENDED_MIN_CIRCUIT_BUILD_TIMEOUT );
  } else if (!options->LearnCircuitBuildTimeout &&
             !options->CircuitBuildTimeout) {
    int severity = LOG_NOTICE;
    /* Be a little quieter if we've deliberately disabled
     * LearnCircuitBuildTimeout. */
    if (circuit_build_times_disabled_(options, 1)) {
      severity = LOG_INFO;
    }
    log_fn(severity, LD_CONFIG, "You disabled LearnCircuitBuildTimeout, but "
           "didn't specify a CircuitBuildTimeout. I'll pick a plausible "
           "default.");
  }

  if (options->DormantClientTimeout < 10*60 && !options->TestingTorNetwork) {
    REJECT("DormantClientTimeout is too low. It must be at least 10 minutes.");
  }

  if (options->PathBiasNoticeRate > 1.0) {
    tor_asprintf(msg,
              "PathBiasNoticeRate is too high. "
              "It must be between 0 and 1.0");
    return -1;
  }
  if (options->PathBiasWarnRate > 1.0) {
    tor_asprintf(msg,
              "PathBiasWarnRate is too high. "
              "It must be between 0 and 1.0");
    return -1;
  }
  if (options->PathBiasExtremeRate > 1.0) {
    tor_asprintf(msg,
              "PathBiasExtremeRate is too high. "
              "It must be between 0 and 1.0");
    return -1;
  }
  if (options->PathBiasNoticeUseRate > 1.0) {
    tor_asprintf(msg,
              "PathBiasNoticeUseRate is too high. "
              "It must be between 0 and 1.0");
    return -1;
  }
  if (options->PathBiasExtremeUseRate > 1.0) {
    tor_asprintf(msg,
              "PathBiasExtremeUseRate is too high. "
              "It must be between 0 and 1.0");
    return -1;
  }

  if (options->MaxCircuitDirtiness < MIN_MAX_CIRCUIT_DIRTINESS) {
    log_warn(LD_CONFIG, "MaxCircuitDirtiness option is too short; "
             "raising to %d seconds.", MIN_MAX_CIRCUIT_DIRTINESS);
    options->MaxCircuitDirtiness = MIN_MAX_CIRCUIT_DIRTINESS;
  }

  if (options->MaxCircuitDirtiness > MAX_MAX_CIRCUIT_DIRTINESS) {
    log_warn(LD_CONFIG, "MaxCircuitDirtiness option is too high; "
             "setting to %d days.", MAX_MAX_CIRCUIT_DIRTINESS/86400);
    options->MaxCircuitDirtiness = MAX_MAX_CIRCUIT_DIRTINESS;
  }

  if (options->CircuitStreamTimeout &&
      options->CircuitStreamTimeout < MIN_CIRCUIT_STREAM_TIMEOUT) {
    log_warn(LD_CONFIG, "CircuitStreamTimeout option is too short; "
             "raising to %d seconds.", MIN_CIRCUIT_STREAM_TIMEOUT);
    options->CircuitStreamTimeout = MIN_CIRCUIT_STREAM_TIMEOUT;
  }

  if (options->HeartbeatPeriod &&
      options->HeartbeatPeriod < MIN_HEARTBEAT_PERIOD &&
      !options->TestingTorNetwork) {
    log_warn(LD_CONFIG, "HeartbeatPeriod option is too short; "
             "raising to %d seconds.", MIN_HEARTBEAT_PERIOD);
    options->HeartbeatPeriod = MIN_HEARTBEAT_PERIOD;
  }

  if (options->KeepalivePeriod < 1)
    REJECT("KeepalivePeriod option must be positive.");

  if (config_ensure_bandwidth_cap(&options->BandwidthRate,
                           "BandwidthRate", msg) < 0)
    return -1;
  if (config_ensure_bandwidth_cap(&options->BandwidthBurst,
                           "BandwidthBurst", msg) < 0)
    return -1;

  if (options_validate_relay_bandwidth(old_options, options, msg) < 0)
    return -1;

  if (options->BandwidthRate > options->BandwidthBurst)
    REJECT("BandwidthBurst must be at least equal to BandwidthRate.");

  if (options_validate_relay_accounting(old_options, options, msg) < 0)
    return -1;

  if (options_validate_relay_mode(old_options, options, msg) < 0)
    return -1;

  if (options->HTTPProxy) { /* parse it now */
    if (tor_addr_port_lookup(options->HTTPProxy,
                        &options->HTTPProxyAddr, &options->HTTPProxyPort) < 0)
      REJECT("HTTPProxy failed to parse or resolve. Please fix.");
    if (options->HTTPProxyPort == 0) { /* give it a default */
      options->HTTPProxyPort = 80;
    }
  }

  if (options->HTTPProxyAuthenticator) {
    if (strlen(options->HTTPProxyAuthenticator) >= 512)
      REJECT("HTTPProxyAuthenticator is too long (>= 512 chars).");
  }

  if (options->HTTPSProxy) { /* parse it now */
    if (tor_addr_port_lookup(options->HTTPSProxy,
                        &options->HTTPSProxyAddr, &options->HTTPSProxyPort) <0)
      REJECT("HTTPSProxy failed to parse or resolve. Please fix.");
    if (options->HTTPSProxyPort == 0) { /* give it a default */
      options->HTTPSProxyPort = 443;
    }
  }

  if (options->HTTPSProxyAuthenticator) {
    if (strlen(options->HTTPSProxyAuthenticator) >= 512)
      REJECT("HTTPSProxyAuthenticator is too long (>= 512 chars).");
  }

  if (options->Socks4Proxy) { /* parse it now */
    if (tor_addr_port_lookup(options->Socks4Proxy,
                        &options->Socks4ProxyAddr,
                        &options->Socks4ProxyPort) <0)
      REJECT("Socks4Proxy failed to parse or resolve. Please fix.");
    if (options->Socks4ProxyPort == 0) { /* give it a default */
      options->Socks4ProxyPort = 1080;
    }
  }

  if (options->Socks5Proxy) { /* parse it now */
    if (tor_addr_port_lookup(options->Socks5Proxy,
                            &options->Socks5ProxyAddr,
                            &options->Socks5ProxyPort) <0)
      REJECT("Socks5Proxy failed to parse or resolve. Please fix.");
    if (options->Socks5ProxyPort == 0) { /* give it a default */
      options->Socks5ProxyPort = 1080;
    }
  }

  if (options->TCPProxy) {
    int res = parse_tcp_proxy_line(options->TCPProxy, options, msg);
    if (res < 0) {
      return res;
    }
  }

  /* Check if more than one exclusive proxy type has been enabled. */
  if (!!options->Socks4Proxy + !!options->Socks5Proxy +
      !!options->HTTPSProxy + !!options->TCPProxy > 1)
    REJECT("You have configured more than one proxy type. "
           "(Socks4Proxy|Socks5Proxy|HTTPSProxy|TCPProxy)");

  /* Check if the proxies will give surprising behavior. */
  if (options->HTTPProxy && !(options->Socks4Proxy ||
                              options->Socks5Proxy ||
                              options->HTTPSProxy ||
                              options->TCPProxy)) {
    log_warn(LD_CONFIG, "HTTPProxy configured, but no SOCKS proxy, "
             "HTTPS proxy, or any other TCP proxy configured. Watch out: "
             "this configuration will proxy unencrypted directory "
             "connections only.");
  }

  if (options->Socks5ProxyUsername) {
    size_t len;

    len = strlen(options->Socks5ProxyUsername);
    if (len < 1 || len > MAX_SOCKS5_AUTH_FIELD_SIZE)
      REJECT("Socks5ProxyUsername must be between 1 and 255 characters.");

    if (!options->Socks5ProxyPassword)
      REJECT("Socks5ProxyPassword must be included with Socks5ProxyUsername.");

    len = strlen(options->Socks5ProxyPassword);
    if (len < 1 || len > MAX_SOCKS5_AUTH_FIELD_SIZE)
      REJECT("Socks5ProxyPassword must be between 1 and 255 characters.");
  } else if (options->Socks5ProxyPassword)
    REJECT("Socks5ProxyPassword must be included with Socks5ProxyUsername.");

  if (options->HashedControlPassword) {
    smartlist_t *sl = decode_hashed_passwords(options->HashedControlPassword);
    if (!sl) {
      REJECT("Bad HashedControlPassword: wrong length or bad encoding");
    } else {
      SMARTLIST_FOREACH(sl, char*, cp, tor_free(cp));
      smartlist_free(sl);
    }
  }

  if (options->HashedControlSessionPassword) {
    smartlist_t *sl = decode_hashed_passwords(
                                  options->HashedControlSessionPassword);
    if (!sl) {
      REJECT("Bad HashedControlSessionPassword: wrong length or bad encoding");
    } else {
      SMARTLIST_FOREACH(sl, char*, cp, tor_free(cp));
      smartlist_free(sl);
    }
  }

  if (options->OwningControllerProcess) {
    const char *validate_pspec_msg = NULL;
    if (tor_validate_process_specifier(options->OwningControllerProcess,
                                       &validate_pspec_msg)) {
      tor_asprintf(msg, "Bad OwningControllerProcess: %s",
                   validate_pspec_msg);
      return -1;
    }
  }

  if ((options->ControlPort_set || world_writable_control_socket) &&
      !options->HashedControlPassword &&
      !options->HashedControlSessionPassword &&
      !options->CookieAuthentication) {
    log_warn(LD_CONFIG, "Control%s is %s, but no authentication method "
             "has been configured.  This means that any program on your "
             "computer can reconfigure your Tor.  That's bad!  You should "
             "upgrade your Tor controller as soon as possible.",
             options->ControlPort_set ? "Port" : "Socket",
             options->ControlPort_set ? "open" : "world writable");
  }

  if (options->CookieAuthFileGroupReadable && !options->CookieAuthFile) {
    log_warn(LD_CONFIG, "CookieAuthFileGroupReadable is set, but will have "
             "no effect: you must specify an explicit CookieAuthFile to "
             "have it group-readable.");
  }

  for (cl = options->NodeFamilies; cl; cl = cl->next) {
    routerset_t *rs = routerset_new();
    if (routerset_parse(rs, cl->value, cl->key)) {
      routerset_free(rs);
      return -1;
    }
    routerset_free(rs);
  }

  if (validate_addr_policies(options, msg) < 0)
    return -1;

  /* If FallbackDir is set, we don't UseDefaultFallbackDirs */
  if (options->UseDefaultFallbackDirs && options->FallbackDir) {
    log_info(LD_CONFIG, "You have set UseDefaultFallbackDirs 1 and "
             "FallbackDir(s). Ignoring UseDefaultFallbackDirs, and "
             "using the FallbackDir(s) you have set.");
  }

  if (validate_dir_servers(options, old_options) < 0)
    REJECT("Directory authority/fallback line did not parse. See logs "
           "for details.");

  if (options->UseBridges && !options->Bridges)
    REJECT("If you set UseBridges, you must specify at least one bridge.");

  for (cl = options->Bridges; cl; cl = cl->next) {
      bridge_line_t *bridge_line = parse_bridge_line(cl->value);
      if (!bridge_line)
        REJECT("Bridge line did not parse. See logs for details.");
      bridge_line_free(bridge_line);
  }

  for (cl = options->ClientTransportPlugin; cl; cl = cl->next) {
    if (pt_parse_transport_line(options, cl->value, 1, 0) < 0)
      REJECT("Invalid client transport line. See logs for details.");
  }

  if (options_validate_server_transport(old_options, options, msg) < 0)
    return -1;

  if (options->ConstrainedSockets) {
    /* If the user wants to constrain socket buffer use, make sure the desired
     * limit is between MIN|MAX_TCPSOCK_BUFFER in k increments. */
    if (options->ConstrainedSockSize < MIN_CONSTRAINED_TCP_BUFFER ||
        options->ConstrainedSockSize > MAX_CONSTRAINED_TCP_BUFFER ||
        options->ConstrainedSockSize % 1024) {
      tor_asprintf(msg,
          "ConstrainedSockSize is invalid.  Must be a value between %d and %d "
          "in 1024 byte increments.",
          MIN_CONSTRAINED_TCP_BUFFER, MAX_CONSTRAINED_TCP_BUFFER);
      return -1;
    }
  }

  if (options_validate_dirauth_schedule(old_options, options, msg) < 0)
    return -1;

  if (hs_config_service_all(options, 1) < 0)
    REJECT("Failed to configure rendezvous options. See logs for details.");

  /* Parse client-side authorization for hidden services. */
  if (hs_config_client_auth_all(options, 1) < 0)
    REJECT("Failed to configure client authorization for hidden services. "
           "See logs for details.");

  if (parse_virtual_addr_network(options->VirtualAddrNetworkIPv4,
                                 AF_INET, 1, msg)<0)
    return -1;
  if (parse_virtual_addr_network(options->VirtualAddrNetworkIPv6,
                                 AF_INET6, 1, msg)<0)
    return -1;

  if (options->TestingTorNetwork &&
      !(options->DirAuthorities ||
        (options->AlternateDirAuthority &&
         options->AlternateBridgeAuthority))) {
    REJECT("TestingTorNetwork may only be configured in combination with "
           "a non-default set of DirAuthority or both of "
           "AlternateDirAuthority and AlternateBridgeAuthority configured.");
  }

#define CHECK_DEFAULT(arg)                                              \
  STMT_BEGIN                                                            \
    if (!config_is_same(get_options_mgr(),options,                      \
                        dflt_options,#arg)) {                           \
      or_options_free(dflt_options);                                    \
      REJECT(#arg " may only be changed in testing Tor "                \
             "networks!");                                              \
    }                                                                   \
  STMT_END

  /* Check for options that can only be changed from the defaults in testing
     networks.  */
  if (! options->TestingTorNetwork && !options->UsingTestNetworkDefaults_) {
    or_options_t *dflt_options = options_new();
    options_init(dflt_options);
    /* 31851: some of these options are dirauth or relay only */
    CHECK_DEFAULT(TestingV3AuthInitialVotingInterval);
    CHECK_DEFAULT(TestingV3AuthInitialVoteDelay);
    CHECK_DEFAULT(TestingV3AuthInitialDistDelay);
    CHECK_DEFAULT(TestingV3AuthVotingStartOffset);
    CHECK_DEFAULT(TestingAuthDirTimeToLearnReachability);
    CHECK_DEFAULT(TestingServerDownloadInitialDelay);
    CHECK_DEFAULT(TestingClientDownloadInitialDelay);
    CHECK_DEFAULT(TestingServerConsensusDownloadInitialDelay);
    CHECK_DEFAULT(TestingClientConsensusDownloadInitialDelay);
    CHECK_DEFAULT(TestingBridgeDownloadInitialDelay);
    CHECK_DEFAULT(TestingBridgeBootstrapDownloadInitialDelay);
    CHECK_DEFAULT(TestingClientMaxIntervalWithoutRequest);
    CHECK_DEFAULT(TestingDirConnectionMaxStall);
    CHECK_DEFAULT(TestingAuthKeyLifetime);
    CHECK_DEFAULT(TestingLinkCertLifetime);
    CHECK_DEFAULT(TestingSigningKeySlop);
    CHECK_DEFAULT(TestingAuthKeySlop);
    CHECK_DEFAULT(TestingLinkKeySlop);
    CHECK_DEFAULT(TestingMinTimeToReportBandwidth);
    or_options_free(dflt_options);
  }
#undef CHECK_DEFAULT

  if (!options->ClientDNSRejectInternalAddresses &&
      !(options->DirAuthorities ||
        (options->AlternateDirAuthority && options->AlternateBridgeAuthority)))
    REJECT("ClientDNSRejectInternalAddresses used for default network.");

  if (options_validate_relay_testing(old_options, options, msg) < 0)
    return -1;
  if (options_validate_dirauth_testing(old_options, options, msg) < 0)
    return -1;

  if (options->TestingClientMaxIntervalWithoutRequest < 1) {
    REJECT("TestingClientMaxIntervalWithoutRequest is way too low.");
  } else if (options->TestingClientMaxIntervalWithoutRequest > 3600) {
    COMPLAIN("TestingClientMaxIntervalWithoutRequest is insanely high.");
  }

  if (options->TestingDirConnectionMaxStall < 5) {
    REJECT("TestingDirConnectionMaxStall is way too low.");
  } else if (options->TestingDirConnectionMaxStall > 3600) {
    COMPLAIN("TestingDirConnectionMaxStall is insanely high.");
  }

  if (options->ClientBootstrapConsensusMaxInProgressTries < 1) {
    REJECT("ClientBootstrapConsensusMaxInProgressTries must be greater "
           "than 0.");
  } else if (options->ClientBootstrapConsensusMaxInProgressTries
             > 100) {
    COMPLAIN("ClientBootstrapConsensusMaxInProgressTries is insanely "
             "high.");
  }

  if (options->TestingEnableConnBwEvent &&
      !options->TestingTorNetwork && !options->UsingTestNetworkDefaults_) {
    REJECT("TestingEnableConnBwEvent may only be changed in testing "
           "Tor networks!");
  }

  if (options->TestingEnableCellStatsEvent &&
      !options->TestingTorNetwork && !options->UsingTestNetworkDefaults_) {
    REJECT("TestingEnableCellStatsEvent may only be changed in testing "
           "Tor networks!");
  }

  if (options->TestingTorNetwork) {
    log_warn(LD_CONFIG, "TestingTorNetwork is set. This will make your node "
                        "almost unusable in the public Tor network, and is "
                        "therefore only advised if you are building a "
                        "testing Tor network!");
  }

  if (options_validate_scheduler(options, msg) < 0) {
    return -1;
  }

  return 0;
}

#undef REJECT
#undef COMPLAIN

/* Given the value that the user has set for MaxMemInQueues, compute the
 * actual maximum value.  We clip this value if it's too low, and autodetect
 * it if it's set to 0. */
STATIC uint64_t
compute_real_max_mem_in_queues(const uint64_t val, bool is_server)
{
#define MIN_SERVER_MB 64
#define MIN_UNWARNED_SERVER_MB 256
#define MIN_UNWARNED_CLIENT_MB 64
  uint64_t result;

  if (val == 0) {
#define ONE_GIGABYTE (UINT64_C(1) << 30)
#define ONE_MEGABYTE (UINT64_C(1) << 20)
    /* The user didn't pick a memory limit.  Choose a very large one
     * that is still smaller than the system memory */
    static int notice_sent = 0;
    size_t ram = 0;
    if (get_total_system_memory(&ram) < 0) {
      /* We couldn't determine our total system memory!  */
#if SIZEOF_VOID_P >= 8
      /* 64-bit system.  Let's hope for 8 GB. */
      result = 8 * ONE_GIGABYTE;
#else
      /* (presumably) 32-bit system. Let's hope for 1 GB. */
      result = ONE_GIGABYTE;
#endif /* SIZEOF_VOID_P >= 8 */
    } else {
      /* We detected the amount of memory available. */
      uint64_t avail = 0;

#if SIZEOF_SIZE_T > 4
/* On a 64-bit platform, we consider 8GB "very large". */
#define RAM_IS_VERY_LARGE(x) ((x) >= (8 * ONE_GIGABYTE))
#else
/* On a 32-bit platform, we can't have 8GB of ram. */
#define RAM_IS_VERY_LARGE(x) (0)
#endif /* SIZEOF_SIZE_T > 4 */

      if (RAM_IS_VERY_LARGE(ram)) {
        /* If we have 8 GB, or more, RAM available, we set the MaxMemInQueues
         * to 0.4 * RAM. The idea behind this value is that the amount of RAM
         * is more than enough for a single relay and should allow the relay
         * operator to run two relays if they have additional bandwidth
         * available.
         */
        avail = (ram / 5) * 2;
      } else {
        /* If we have less than 8 GB of RAM available, we use the "old" default
         * for MaxMemInQueues of 0.75 * RAM.
         */
        avail = (ram / 4) * 3;
      }

      /* Make sure it's in range from 0.25 GB to 8 GB for 64-bit and 0.25 to 2
       * GB for 32-bit. */
      if (avail > MAX_DEFAULT_MEMORY_QUEUE_SIZE) {
        /* If you want to use more than this much RAM, you need to configure
           it yourself */
        result = MAX_DEFAULT_MEMORY_QUEUE_SIZE;
      } else if (avail < ONE_GIGABYTE / 4) {
        result = ONE_GIGABYTE / 4;
      } else {
        result = avail;
      }
    }
    if (is_server && ! notice_sent) {
      log_notice(LD_CONFIG, "%sMaxMemInQueues is set to %"PRIu64" MB. "
                 "You can override this by setting MaxMemInQueues by hand.",
                 ram ? "Based on detected system memory, " : "",
                 (result / ONE_MEGABYTE));
      notice_sent = 1;
    }
    return result;
  } else if (is_server && val < ONE_MEGABYTE * MIN_SERVER_MB) {
    /* We can't configure less than this much on a server.  */
    log_warn(LD_CONFIG, "MaxMemInQueues must be at least %d MB on servers "
             "for now. Ideally, have it as large as you can afford.",
             MIN_SERVER_MB);
    return MIN_SERVER_MB * ONE_MEGABYTE;
  } else if (is_server && val < ONE_MEGABYTE * MIN_UNWARNED_SERVER_MB) {
    /* On a server, if it's less than this much, we warn that things
     * may go badly. */
    log_warn(LD_CONFIG, "MaxMemInQueues is set to a low value; if your "
             "relay doesn't work, this may be the reason why.");
    return val;
  } else if (! is_server && val < ONE_MEGABYTE * MIN_UNWARNED_CLIENT_MB) {
    /* On a client, if it's less than this much, we warn that things
     * may go badly. */
    log_warn(LD_CONFIG, "MaxMemInQueues is set to a low value; if your "
             "client doesn't work, this may be the reason why.");
    return val;
  } else {
    /* The value was fine all along */
    return val;
  }
}

/** Helper: return true iff s1 and s2 are both NULL, or both non-NULL
 * equal strings. */
static int
opt_streq(const char *s1, const char *s2)
{
  return 0 == strcmp_opt(s1, s2);
}

/** Check if any config options have changed but aren't allowed to. */
static int
options_check_transition_cb(const void *old_,
                            const void *new_val_,
                            char **msg)
{
  CHECK_OPTIONS_MAGIC(old_);
  CHECK_OPTIONS_MAGIC(new_val_);

  const or_options_t *old = old_;
  const or_options_t *new_val = new_val_;

  if (BUG(!old))
    return 0;

#define BAD_CHANGE_TO(opt, how) do {                                    \
    *msg = tor_strdup("While Tor is running"how", changing " #opt       \
                      " is not allowed");                               \
    return -1;                                                          \
  } while (0)

  if (sandbox_is_active()) {
#define SB_NOCHANGE_STR(opt)                      \
    if (! CFG_EQ_STRING(old, new_val, opt))       \
      BAD_CHANGE_TO(opt," with Sandbox active")
#define SB_NOCHANGE_LINELIST(opt)                  \
    if (! CFG_EQ_LINELIST(old, new_val, opt))      \
      BAD_CHANGE_TO(opt," with Sandbox active")
#define SB_NOCHANGE_INT(opt)                       \
    if (! CFG_EQ_INT(old, new_val, opt))           \
      BAD_CHANGE_TO(opt," with Sandbox active")

    SB_NOCHANGE_LINELIST(Address);
    SB_NOCHANGE_STR(ServerDNSResolvConfFile);
    SB_NOCHANGE_STR(DirPortFrontPage);
    SB_NOCHANGE_STR(CookieAuthFile);
    SB_NOCHANGE_STR(ExtORPortCookieAuthFile);
    SB_NOCHANGE_LINELIST(Logs);
    SB_NOCHANGE_INT(ConnLimit);

    if (server_mode(old) != server_mode(new_val)) {
      *msg = tor_strdup("Can't start/stop being a server while "
                        "Sandbox is active");
      return -1;
    }
  }

#undef SB_NOCHANGE_LINELIST
#undef SB_NOCHANGE_STR
#undef SB_NOCHANGE_INT
#undef BAD_CHANGE_TO
#undef NO_CHANGE_BOOL
#undef NO_CHANGE_INT
#undef NO_CHANGE_STRING
  return 0;
}

#ifdef _WIN32
/** Return the directory on windows where we expect to find our application
 * data. */
static char *
get_windows_conf_root(void)
{
  static int is_set = 0;
  static char path[MAX_PATH*2+1];
  TCHAR tpath[MAX_PATH] = {0};

  LPITEMIDLIST idl;
  IMalloc *m;
  HRESULT result;

  if (is_set)
    return path;

  /* Find X:\documents and settings\username\application data\ .
   * We would use SHGetSpecialFolder path, but that wasn't added until IE4.
   */
#ifdef ENABLE_LOCAL_APPDATA
#define APPDATA_PATH CSIDL_LOCAL_APPDATA
#else
#define APPDATA_PATH CSIDL_APPDATA
#endif
  if (!SUCCEEDED(SHGetSpecialFolderLocation(NULL, APPDATA_PATH, &idl))) {
    getcwd(path,MAX_PATH);
    is_set = 1;
    log_warn(LD_CONFIG,
             "I couldn't find your application data folder: are you "
             "running an ancient version of Windows 95? Defaulting to \"%s\"",
             path);
    return path;
  }
  /* Convert the path from an "ID List" (whatever that is!) to a path. */
  result = SHGetPathFromIDList(idl, tpath);
#ifdef UNICODE
  wcstombs(path,tpath,sizeof(path));
  path[sizeof(path)-1] = '\0';
#else
  strlcpy(path,tpath,sizeof(path));
#endif /* defined(UNICODE) */

  /* Now we need to free the memory that the path-idl was stored in.  In
   * typical Windows fashion, we can't just call 'free()' on it. */
  SHGetMalloc(&m);
  if (m) {
    m->lpVtbl->Free(m, idl);
    m->lpVtbl->Release(m);
  }
  if (!SUCCEEDED(result)) {
    return NULL;
  }
  strlcat(path,"\\tor",MAX_PATH);
  is_set = 1;
  return path;
}
#endif /* defined(_WIN32) */

/** Return the default location for our torrc file (if <b>defaults_file</b> is
 * false), or for the torrc-defaults file (if <b>defaults_file</b> is true). */
static const char *
get_default_conf_file(int defaults_file)
{
#ifdef DISABLE_SYSTEM_TORRC
  (void) defaults_file;
  return NULL;
#elif defined(_WIN32)
  if (defaults_file) {
    static char defaults_path[MAX_PATH+1];
    tor_snprintf(defaults_path, MAX_PATH, "%s\\torrc-defaults",
                 get_windows_conf_root());
    return defaults_path;
  } else {
    static char path[MAX_PATH+1];
    tor_snprintf(path, MAX_PATH, "%s\\torrc",
                 get_windows_conf_root());
    return path;
  }
#else
  return defaults_file ? CONFDIR "/torrc-defaults" : CONFDIR "/torrc";
#endif /* defined(DISABLE_SYSTEM_TORRC) || ... */
}

/** Learn config file name from command line arguments, or use the default.
 *
 * If <b>defaults_file</b> is true, we're looking for torrc-defaults;
 * otherwise, we're looking for the regular torrc_file.
 *
 * Set *<b>using_default_fname</b> to true if we're using the default
 * configuration file name; or false if we've set it from the command line.
 *
 * Set *<b>ignore_missing_torrc</b> to true if we should ignore the resulting
 * filename if it doesn't exist.
 */
static char *
find_torrc_filename(const config_line_t *cmd_arg,
                    int defaults_file,
                    int *using_default_fname, int *ignore_missing_torrc)
{
  char *fname=NULL;
  const config_line_t *p_index;
  const char *fname_opt = defaults_file ? "--defaults-torrc" : "-f";
  const char *fname_long_opt = defaults_file ? "--defaults-torrc" :
                                               "--torrc-file";
  const char *ignore_opt = defaults_file ? NULL : "--ignore-missing-torrc";
  const char *keygen_opt = "--keygen";

  if (defaults_file)
    *ignore_missing_torrc = 1;

  for (p_index = cmd_arg; p_index; p_index = p_index->next) {
    // options_init_from_torrc ensures only the short or long name is present
    if (!strcmp(p_index->key, fname_opt) ||
        !strcmp(p_index->key, fname_long_opt)) {
      if (fname) {
        log_warn(LD_CONFIG, "Duplicate %s options on command line.",
            p_index->key);
        tor_free(fname);
      }
      fname = expand_filename(p_index->value);

      {
        char *absfname;
        absfname = make_path_absolute(fname);
        tor_free(fname);
        fname = absfname;
      }

      *using_default_fname = 0;
    } else if ((ignore_opt && !strcmp(p_index->key, ignore_opt)) ||
               (keygen_opt && !strcmp(p_index->key, keygen_opt))) {
      *ignore_missing_torrc = 1;
    }
  }

  if (*using_default_fname) {
    /* didn't find one, try CONFDIR */
    const char *dflt = get_default_conf_file(defaults_file);
    file_status_t st = file_status(dflt);
    if (dflt && (st == FN_FILE || st == FN_EMPTY)) {
      fname = tor_strdup(dflt);
    } else {
#ifndef _WIN32
      char *fn = NULL;
      if (!defaults_file) {
        fn = expand_filename("~/.torrc");
      }
      if (fn) {
        file_status_t hmst = file_status(fn);
        if (hmst == FN_FILE || hmst == FN_EMPTY || dflt == NULL) {
          fname = fn;
        } else {
          tor_free(fn);
          fname = tor_strdup(dflt);
        }
      } else {
        fname = dflt ? tor_strdup(dflt) : NULL;
      }
#else /* defined(_WIN32) */
      fname = dflt ? tor_strdup(dflt) : NULL;
#endif /* !defined(_WIN32) */
    }
  }
  return fname;
}

/** Read the torrc from standard input and return it as a string.
 * Upon failure, return NULL.
 */
static char *
load_torrc_from_stdin(void)
{
   size_t sz_out;

   return read_file_to_str_until_eof(STDIN_FILENO,SIZE_MAX,&sz_out);
}

/** Load a configuration file from disk, setting torrc_fname or
 * torrc_defaults_fname if successful.
 *
 * If <b>defaults_file</b> is true, load torrc-defaults; otherwise load torrc.
 *
 * Return the contents of the file on success, and NULL on failure.
 */
static char *
load_torrc_from_disk(const config_line_t *cmd_arg, int defaults_file)
{
  char *fname=NULL;
  char *cf = NULL;
  int using_default_torrc = 1;
  int ignore_missing_torrc = 0;
  char **fname_var = defaults_file ? &torrc_defaults_fname : &torrc_fname;

  if (*fname_var == NULL) {
    fname = find_torrc_filename(cmd_arg, defaults_file,
                                &using_default_torrc, &ignore_missing_torrc);
    tor_free(*fname_var);
    *fname_var = fname;
  } else {
    fname = *fname_var;
  }
  log_debug(LD_CONFIG, "Opening config file \"%s\"", fname?fname:"<NULL>");

  /* Open config file */
  file_status_t st = fname ? file_status(fname) : FN_EMPTY;
  if (fname == NULL ||
      !(st == FN_FILE || st == FN_EMPTY) ||
      !(cf = read_file_to_str(fname,0,NULL))) {
    if (using_default_torrc == 1 || ignore_missing_torrc) {
      if (!defaults_file)
        log_notice(LD_CONFIG, "Configuration file \"%s\" not present, "
            "using reasonable defaults.", fname);
      tor_free(fname); /* sets fname to NULL */
      *fname_var = NULL;
      cf = tor_strdup("");
    } else {
      log_warn(LD_CONFIG,
          "Unable to open configuration file \"%s\".", fname);
      goto err;
    }
  } else {
    log_notice(LD_CONFIG, "Read configuration file \"%s\".", fname);
  }

  return cf;
 err:
  tor_free(fname);
  *fname_var = NULL;
  return NULL;
}

/** Read a configuration file into <b>options</b>, finding the configuration
 * file location based on the command line.  After loading the file
 * call options_init_from_string() to load the config.
 * Return 0 if success, -1 if failure, and 1 if we succeeded but should exit
 * anyway. */
int
options_init_from_torrc(int argc, char **argv)
{
  char *cf=NULL, *cf_defaults=NULL;
  int retval = -1;
  char *errmsg=NULL;
  const config_line_t *cmdline_only_options;

  /* Go through command-line variables */
  if (global_cmdline == NULL) {
    /* Or we could redo the list every time we pass this place.
     * It does not really matter */
    global_cmdline = config_parse_commandline(argc, argv, 0);
    if (global_cmdline == NULL) {
      goto err;
    }
  }
  cmdline_only_options = global_cmdline->cmdline_opts;

  if (config_line_find(cmdline_only_options, "-h") ||
      config_line_find(cmdline_only_options, "--help")) {
    print_usage();
    return 1;
  }
  if (config_line_find(cmdline_only_options, "--list-torrc-options")) {
    /* For validating whether we've documented everything. */
    list_torrc_options();
    return 1;
  }
  if (config_line_find(cmdline_only_options, "--list-deprecated-options")) {
    /* For validating whether what we have deprecated really exists. */
    list_deprecated_options();
    return 1;
  }
  if (config_line_find(cmdline_only_options, "--dbg-dump-subsystem-list")) {
    subsystems_dump_list();
    return 1;
  }

  if (config_line_find(cmdline_only_options, "--version")) {
    printf("Tor version %s.\n",get_version());
#ifdef ENABLE_GPL
    printf("This build of Tor is covered by the GNU General Public License "
            "(https://www.gnu.org/licenses/gpl-3.0.en.html)\n");
#endif
    printf("Tor is running on %s with Libevent %s, "
            "%s %s, Zlib %s, Liblzma %s, Libzstd %s and %s %s as libc.\n",
            get_uname(),
            tor_libevent_get_version_str(),
            crypto_get_library_name(),
            crypto_get_library_version_string(),
            tor_compress_supports_method(ZLIB_METHOD) ?
            tor_compress_version_str(ZLIB_METHOD) : "N/A",
            tor_compress_supports_method(LZMA_METHOD) ?
            tor_compress_version_str(LZMA_METHOD) : "N/A",
            tor_compress_supports_method(ZSTD_METHOD) ?
            tor_compress_version_str(ZSTD_METHOD) : "N/A",
            tor_libc_get_name() ?
            tor_libc_get_name() : "Unknown",
            tor_libc_get_version_str());
    printf("Tor compiled with %s version %s\n",
            strcmp(COMPILER_VENDOR, "gnu") == 0?
            COMPILER:COMPILER_VENDOR, COMPILER_VERSION);

    return 1;
  }

  if (config_line_find(cmdline_only_options, "--list-modules")) {
    list_enabled_modules();
    return 1;
  }

  if (config_line_find(cmdline_only_options, "--library-versions")) {
    print_library_versions();
    return 1;
  }

  int command = global_cmdline->command;
  const char *command_arg = global_cmdline->command_arg;
  /* "immediate" has already been handled by this point. */
  tor_assert(command != CMD_IMMEDIATE);

  if (command == CMD_HASH_PASSWORD) {
    cf_defaults = tor_strdup("");
    cf = tor_strdup("");
  } else {
    cf_defaults = load_torrc_from_disk(cmdline_only_options, 1);
    const config_line_t *f_line = config_line_find(cmdline_only_options, "-f");
    const config_line_t *f_line_long = config_line_find(cmdline_only_options,
                                                        "--torrc-file");
    if (f_line && f_line_long) {
      log_err(LD_CONFIG, "-f and --torrc-file cannot be used together.");
      retval = -1;
      goto err;
    } else if (f_line_long) {
      f_line = f_line_long;
    }

    const int read_torrc_from_stdin =
    (f_line != NULL && strcmp(f_line->value, "-") == 0);

    if (read_torrc_from_stdin) {
      cf = load_torrc_from_stdin();
    } else {
      cf = load_torrc_from_disk(cmdline_only_options, 0);
    }

    if (!cf) {
      if (config_line_find(cmdline_only_options, "--allow-missing-torrc")) {
        cf = tor_strdup("");
      } else {
        goto err;
      }
    }
  }

  retval = options_init_from_string(cf_defaults, cf, command, command_arg,
                                    &errmsg);
  if (retval < 0)
    goto err;

  if (config_line_find(cmdline_only_options, "--no-passphrase")) {
    if (handle_cmdline_no_passphrase(command) < 0) {
      retval = -1;
      goto err;
    }
  }

  const config_line_t *format_line = config_line_find(cmdline_only_options,
                                                      "--format");
  if (format_line) {
    if (handle_cmdline_format(command, format_line->value) < 0) {
      retval = -1;
      goto err;
    }
  } else {
    get_options_mutable()->key_expiration_format =
      KEY_EXPIRATION_FORMAT_ISO8601;
  }

  if (config_line_find(cmdline_only_options, "--newpass")) {
    if (handle_cmdline_newpass(command) < 0) {
      retval = -1;
      goto err;
    }
  }

  const config_line_t *fd_line = config_line_find(cmdline_only_options,
                                                  "--passphrase-fd");
  if (fd_line) {
    if (handle_cmdline_passphrase_fd(command, fd_line->value) < 0) {
      retval = -1;
      goto err;
    }
  }

  const config_line_t *key_line = config_line_find(cmdline_only_options,
                                                   "--master-key");
  if (key_line) {
    if (handle_cmdline_master_key(command, key_line->value) < 0) {
      retval = -1;
      goto err;
    }
  }

 err:
  tor_free(cf);
  tor_free(cf_defaults);
  if (errmsg) {
    log_warn(LD_CONFIG,"%s", errmsg);
    tor_free(errmsg);
  }
  return retval < 0 ? -1 : 0;
}

/** Load the options from the configuration in <b>cf</b>, validate
 * them for consistency and take actions based on them.
 *
 * Return 0 if success, negative on error:
 *  * -1 for general errors.
 *  * -2 for failure to parse/validate,
 *  * -3 for transition not allowed
 *  * -4 for error while setting the new options
 */
setopt_err_t
options_init_from_string(const char *cf_defaults, const char *cf,
                         int command, const char *command_arg,
                         char **msg)
{
  bool retry = false;
  or_options_t *oldoptions, *newoptions, *newdefaultoptions=NULL;
  config_line_t *cl;
  int retval;
  setopt_err_t err = SETOPT_ERR_MISC;
  int cf_has_include = 0;
  tor_assert(msg);

  oldoptions = global_options; /* get_options unfortunately asserts if
                                  this is the first time we run*/

  newoptions = options_new();
  options_init(newoptions);
  newoptions->command = command;
  newoptions->command_arg = command_arg ? tor_strdup(command_arg) : NULL;

  smartlist_t *opened_files = smartlist_new();
  for (int i = 0; i < 2; ++i) {
    const char *body = i==0 ? cf_defaults : cf;
    if (!body)
      continue;

    /* get config lines, assign them */
    retval = config_get_lines_include(body, &cl, 1,
                                      body == cf ? &cf_has_include : NULL,
                                      opened_files);
    if (retval < 0) {
      err = SETOPT_ERR_PARSE;
      goto err;
    }
    retval = config_assign(get_options_mgr(), newoptions, cl,
                           CAL_WARN_DEPRECATIONS, msg);
    config_free_lines(cl);
    if (retval < 0) {
      err = SETOPT_ERR_PARSE;
      goto err;
    }
    if (i==0)
      newdefaultoptions = config_dup(get_options_mgr(), newoptions);
  }

  if (newdefaultoptions == NULL) {
    newdefaultoptions = config_dup(get_options_mgr(), global_default_options);
  }

  /* Go through command-line variables too */
  {
    config_line_t *other_opts = NULL;
    if (global_cmdline) {
      other_opts = global_cmdline->other_opts;
    }
    retval = config_assign(get_options_mgr(), newoptions,
                           other_opts,
                           CAL_WARN_DEPRECATIONS, msg);
  }
  if (retval < 0) {
    err = SETOPT_ERR_PARSE;
    goto err;
  }

  newoptions->IncludeUsed = cf_has_include;
  newoptions->FilesOpenedByIncludes = opened_files;
  opened_files = NULL; // prevent double-free.

  /* If this is a testing network configuration, change defaults
   * for a list of dependent config options, and try this function again. */
  if (newoptions->TestingTorNetwork && ! testing_network_configured) {
    // retry with the testing defaults.
    testing_network_configured = true;
    retry = true;
    goto err;
  }

  err = options_validate_and_set(oldoptions, newoptions, msg);
  if (err < 0) {
    newoptions = NULL; // This was already freed in options_validate_and_set.
    goto err;
  }

  or_options_free(global_default_options);
  global_default_options = newdefaultoptions;

  return SETOPT_OK;

 err:
  in_option_validation = 0;
  if (opened_files) {
    SMARTLIST_FOREACH(opened_files, char *, f, tor_free(f));
    smartlist_free(opened_files);
  }
  or_options_free(newdefaultoptions);
  or_options_free(newoptions);
  if (*msg) {
    char *old_msg = *msg;
    tor_asprintf(msg, "Failed to parse/validate config: %s", old_msg);
    tor_free(old_msg);
  }
  if (retry)
    return options_init_from_string(cf_defaults, cf, command, command_arg,
                                    msg);
  return err;
}

/** Return the location for our configuration file.  May return NULL.
 */
const char *
get_torrc_fname(int defaults_fname)
{
  const char *fname = defaults_fname ? torrc_defaults_fname : torrc_fname;

  if (fname)
    return fname;
  else
    return get_default_conf_file(defaults_fname);
}

/** Adjust the address map based on the MapAddress elements in the
 * configuration <b>options</b>
 */
void
config_register_addressmaps(const or_options_t *options)
{
  smartlist_t *elts;
  config_line_t *opt;
  const char *from, *to, *msg;

  addressmap_clear_configured();
  elts = smartlist_new();
  for (opt = options->AddressMap; opt; opt = opt->next) {
    smartlist_split_string(elts, opt->value, NULL,
                           SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, 2);
    if (smartlist_len(elts) < 2) {
      log_warn(LD_CONFIG,"MapAddress '%s' has too few arguments. Ignoring.",
               opt->value);
      goto cleanup;
    }

    from = smartlist_get(elts,0);
    to = smartlist_get(elts,1);

    if (to[0] == '.' || from[0] == '.') {
      log_warn(LD_CONFIG,"MapAddress '%s' is ambiguous - address starts with a"
              "'.'. Ignoring.",opt->value);
      goto cleanup;
    }

    if (addressmap_register_auto(from, to, 0, ADDRMAPSRC_TORRC, &msg) < 0) {
      log_warn(LD_CONFIG,"MapAddress '%s' failed: %s. Ignoring.", opt->value,
               msg);
      goto cleanup;
    }

    if (smartlist_len(elts) > 2)
      log_warn(LD_CONFIG,"Ignoring extra arguments to MapAddress.");

  cleanup:
    SMARTLIST_FOREACH(elts, char*, cp, tor_free(cp));
    smartlist_clear(elts);
  }
  smartlist_free(elts);
}

/** As addressmap_register(), but detect the wildcarded status of "from" and
 * "to", and do not steal a reference to <b>to</b>. */
/* XXXX move to connection_edge.c */
int
addressmap_register_auto(const char *from, const char *to,
                         time_t expires,
                         addressmap_entry_source_t addrmap_source,
                         const char **msg)
{
  int from_wildcard = 0, to_wildcard = 0;

  *msg = "whoops, forgot the error message";

  if (!strcmp(to, "*") || !strcmp(from, "*")) {
    *msg = "can't remap from or to *";
    return -1;
  }
  /* Detect asterisks in expressions of type: '*.example.com' */
  if (!strncmp(from,"*.",2)) {
    from += 2;
    from_wildcard = 1;
  }
  if (!strncmp(to,"*.",2)) {
    to += 2;
    to_wildcard = 1;
  }

  if (to_wildcard && !from_wildcard) {
    *msg =  "can only use wildcard (i.e. '*.') if 'from' address "
      "uses wildcard also";
    return -1;
  }

  if (address_is_invalid_destination(to, 1)) {
    *msg = "destination is invalid";
    return -1;
  }

  addressmap_register(from, tor_strdup(to), expires, addrmap_source,
                      from_wildcard, to_wildcard, 0);

  return 0;
}

/**
 * As add_file_log, but open the file as appropriate.
 */
STATIC int
open_and_add_file_log(const log_severity_list_t *severity,
                      const char *filename, int truncate_log)
{
  int open_flags = O_WRONLY|O_CREAT;
  open_flags |= truncate_log ? O_TRUNC : O_APPEND;

  int fd = tor_open_cloexec(filename, open_flags, 0640);
  if (fd < 0)
    return -1;

  return add_file_log(severity, filename, fd);
}

/**
 * Try to set our global log granularity from `options->LogGranularity`,
 * adjusting it as needed so that we are an even divisor of a second, or an
 * even multiple of seconds. Return 0 on success, -1 on failure.
 **/
static int
options_init_log_granularity(const or_options_t *options,
                             int validate_only)
{
  if (options->LogTimeGranularity <= 0) {
    log_warn(LD_CONFIG, "Log time granularity '%d' has to be positive.",
             options->LogTimeGranularity);
    return -1;
  } else if (1000 % options->LogTimeGranularity != 0 &&
             options->LogTimeGranularity % 1000 != 0) {
    int granularity = options->LogTimeGranularity;
    if (granularity < 40) {
      do granularity++;
      while (1000 % granularity != 0);
    } else if (granularity < 1000) {
      granularity = 1000 / granularity;
      while (1000 % granularity != 0)
        granularity--;
      granularity = 1000 / granularity;
    } else {
      granularity = 1000 * ((granularity / 1000) + 1);
    }
    log_warn(LD_CONFIG, "Log time granularity '%d' has to be either a "
                        "divisor or a multiple of 1 second. Changing to "
                        "'%d'.",
             options->LogTimeGranularity, granularity);
    if (!validate_only)
      set_log_time_granularity(granularity);
  } else {
    if (!validate_only)
      set_log_time_granularity(options->LogTimeGranularity);
  }

  return 0;
}

/**
 * Initialize the logs based on the configuration file.
 */
STATIC int
options_init_logs(const or_options_t *old_options, const or_options_t *options,
                  int validate_only)
{
  config_line_t *opt;
  int ok;
  smartlist_t *elts;
  int run_as_daemon =
#ifdef _WIN32
               0;
#else
               options->RunAsDaemon;
#endif

  if (options_init_log_granularity(options, validate_only) < 0)
    return -1;

  ok = 1;
  elts = smartlist_new();

  if (options->Logs == NULL && !run_as_daemon && !validate_only) {
    /* When no logs are given, the default behavior is to log nothing (if
       RunAsDaemon is set) or to log based on the quiet level otherwise. */
    add_default_log_for_quiet_level(quiet_level);
  }

  for (opt = options->Logs; opt; opt = opt->next) {
    log_severity_list_t *severity;
    const char *cfg = opt->value;
    severity = tor_malloc_zero(sizeof(log_severity_list_t));
    if (parse_log_severity_config(&cfg, severity) < 0) {
      log_warn(LD_CONFIG, "Couldn't parse log levels in Log option 'Log %s'",
               opt->value);
      ok = 0; goto cleanup;
    }

    smartlist_split_string(elts, cfg, NULL,
                           SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, 2);

    if (smartlist_len(elts) == 0)
      smartlist_add_strdup(elts, "stdout");

    if (smartlist_len(elts) == 1 &&
        (!strcasecmp(smartlist_get(elts,0), "stdout") ||
         !strcasecmp(smartlist_get(elts,0), "stderr"))) {
      int err = smartlist_len(elts) &&
        !strcasecmp(smartlist_get(elts,0), "stderr");
      if (!validate_only) {
        if (run_as_daemon) {
          log_warn(LD_CONFIG,
                   "Can't log to %s with RunAsDaemon set; skipping stdout",
                   err?"stderr":"stdout");
        } else {
          add_stream_log(severity, err?"<stderr>":"<stdout>",
                         fileno(err?stderr:stdout));
        }
      }
      goto cleanup;
    }
    if (smartlist_len(elts) == 1) {
      if (!strcasecmp(smartlist_get(elts,0), "syslog")) {
#ifdef HAVE_SYSLOG_H
        if (!validate_only) {
          add_syslog_log(severity, options->SyslogIdentityTag);
        }
#else
        log_warn(LD_CONFIG, "Syslog is not supported on this system. Sorry.");
#endif /* defined(HAVE_SYSLOG_H) */
        goto cleanup;
      }

      /* We added this workaround in 0.4.5.x; we can remove it in 0.4.6 or
       * later */
      if (!strcasecmp(smartlist_get(elts, 0), "android")) {
#ifdef HAVE_SYSLOG_H
        log_warn(LD_CONFIG, "The android logging API is no longer supported;"
                            " adding a syslog instead. The 'android' logging "
                            " type will no longer work in the future.");
        if (!validate_only) {
          add_syslog_log(severity, options->SyslogIdentityTag);
        }
#else /* !defined(HAVE_SYSLOG_H) */
        log_warn(LD_CONFIG, "The android logging API is no longer supported.");
#endif /* defined(HAVE_SYSLOG_H) */
        goto cleanup;
      }
    }

    if (smartlist_len(elts) == 2 &&
        !strcasecmp(smartlist_get(elts,0), "file")) {
      if (!validate_only) {
        char *fname = expand_filename(smartlist_get(elts, 1));
        /* Truncate if TruncateLogFile is set and we haven't seen this option
           line before. */
        int truncate_log = 0;
        if (options->TruncateLogFile) {
          truncate_log = 1;
          if (old_options) {
            config_line_t *opt2;
            for (opt2 = old_options->Logs; opt2; opt2 = opt2->next)
              if (!strcmp(opt->value, opt2->value)) {
                truncate_log = 0;
                break;
              }
          }
        }
        if (open_and_add_file_log(severity, fname, truncate_log) < 0) {
          log_warn(LD_CONFIG, "Couldn't open file for 'Log %s': %s",
                   opt->value, strerror(errno));
          ok = 0;
        }
        tor_free(fname);
      }
      goto cleanup;
    }

    log_warn(LD_CONFIG, "Bad syntax on file Log option 'Log %s'",
             opt->value);
    ok = 0; goto cleanup;

  cleanup:
    SMARTLIST_FOREACH(elts, char*, cp, tor_free(cp));
    smartlist_clear(elts);
    tor_free(severity);
  }
  smartlist_free(elts);

  if (ok && !validate_only)
    logs_set_domain_logging(options->LogMessageDomains);

  return ok?0:-1;
}

/** Given a smartlist of SOCKS arguments to be passed to a transport
 *  proxy in <b>args</b>, validate them and return -1 if they are
 *  corrupted. Return 0 if they seem OK. */
static int
validate_transport_socks_arguments(const smartlist_t *args)
{
  char *socks_string = NULL;
  size_t socks_string_len;

  tor_assert(args);
  tor_assert(smartlist_len(args) > 0);

  SMARTLIST_FOREACH_BEGIN(args, const char *, s) {
    if (!string_is_key_value(LOG_WARN, s)) { /* items should be k=v items */
      log_warn(LD_CONFIG, "'%s' is not a k=v item.", s);
      return -1;
    }
  } SMARTLIST_FOREACH_END(s);

  socks_string = pt_stringify_socks_args(args);
  if (!socks_string)
    return -1;

  socks_string_len = strlen(socks_string);
  tor_free(socks_string);

  if (socks_string_len > MAX_SOCKS5_AUTH_SIZE_TOTAL) {
    log_warn(LD_CONFIG, "SOCKS arguments can't be more than %u bytes (%lu).",
             MAX_SOCKS5_AUTH_SIZE_TOTAL,
             (unsigned long) socks_string_len);
    return -1;
  }

  return 0;
}

/** Deallocate a bridge_line_t structure. */
/* private */ void
bridge_line_free_(bridge_line_t *bridge_line)
{
  if (!bridge_line)
    return;

  if (bridge_line->socks_args) {
    SMARTLIST_FOREACH(bridge_line->socks_args, char*, s, tor_free(s));
    smartlist_free(bridge_line->socks_args);
  }
  tor_free(bridge_line->transport_name);
  tor_free(bridge_line);
}

/** Parse the contents of a string, <b>line</b>, containing a Bridge line,
 * into a bridge_line_t.
 *
 * Validates that the IP:PORT, fingerprint, and SOCKS arguments (given to the
 * Pluggable Transport, if a one was specified) are well-formed.
 *
 * Returns NULL If the Bridge line could not be validated, and returns a
 * bridge_line_t containing the parsed information otherwise.
 *
 * Bridge line format:
 * Bridge [transport] IP:PORT [id-fingerprint] [k=v] [k=v] ...
 */
/* private */ bridge_line_t *
parse_bridge_line(const char *line)
{
  smartlist_t *items = NULL;
  char *addrport=NULL, *fingerprint=NULL;
  char *field=NULL;
  bridge_line_t *bridge_line = tor_malloc_zero(sizeof(bridge_line_t));

  items = smartlist_new();
  smartlist_split_string(items, line, NULL,
                         SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, -1);
  if (smartlist_len(items) < 1) {
    log_warn(LD_CONFIG, "Too few arguments to Bridge line.");
    goto err;
  }

  /* first field is either a transport name or addrport */
  field = smartlist_get(items, 0);
  smartlist_del_keeporder(items, 0);

  if (string_is_C_identifier(field)) {
    /* It's a transport name. */
    bridge_line->transport_name = field;
    if (smartlist_len(items) < 1) {
      log_warn(LD_CONFIG, "Too few items to Bridge line.");
      goto err;
    }
    addrport = smartlist_get(items, 0); /* Next field is addrport then. */
    smartlist_del_keeporder(items, 0);
  } else {
    addrport = field;
  }

  if (tor_addr_port_parse(LOG_INFO, addrport,
                          &bridge_line->addr, &bridge_line->port, 443)<0) {
    log_warn(LD_CONFIG, "Error parsing Bridge address '%s'", addrport);
    goto err;
  }

  /* If transports are enabled, next field could be a fingerprint or a
     socks argument. If transports are disabled, next field must be
     a fingerprint. */
  if (smartlist_len(items)) {
    if (bridge_line->transport_name) { /* transports enabled: */
      field = smartlist_get(items, 0);
      smartlist_del_keeporder(items, 0);

      /* If it's a key=value pair, then it's a SOCKS argument for the
         transport proxy... */
      if (string_is_key_value(LOG_DEBUG, field)) {
        bridge_line->socks_args = smartlist_new();
        smartlist_add(bridge_line->socks_args, field);
      } else { /* ...otherwise, it's the bridge fingerprint. */
        fingerprint = field;
      }

    } else { /* transports disabled: */
      fingerprint = smartlist_join_strings(items, "", 0, NULL);
    }
  }

  /* Handle fingerprint, if it was provided. */
  if (fingerprint) {
    if (strlen(fingerprint) != HEX_DIGEST_LEN) {
      log_warn(LD_CONFIG, "Key digest for Bridge is wrong length.");
      goto err;
    }
    if (base16_decode(bridge_line->digest, DIGEST_LEN,
                      fingerprint, HEX_DIGEST_LEN) != DIGEST_LEN) {
      log_warn(LD_CONFIG, "Unable to decode Bridge key digest.");
      goto err;
    }
  }

  /* If we are using transports, any remaining items in the smartlist
     should be k=v values. */
  if (bridge_line->transport_name && smartlist_len(items)) {
    if (!bridge_line->socks_args)
      bridge_line->socks_args = smartlist_new();

    /* append remaining items of 'items' to 'socks_args' */
    smartlist_add_all(bridge_line->socks_args, items);
    smartlist_clear(items);

    tor_assert(smartlist_len(bridge_line->socks_args) > 0);
  }

  if (bridge_line->socks_args) {
    if (validate_transport_socks_arguments(bridge_line->socks_args) < 0)
      goto err;
  }

  goto done;

 err:
  bridge_line_free(bridge_line);
  bridge_line = NULL;

 done:
  SMARTLIST_FOREACH(items, char*, s, tor_free(s));
  smartlist_free(items);
  tor_free(addrport);
  tor_free(fingerprint);

  return bridge_line;
}

/** Parse the contents of a TCPProxy line from <b>line</b> and put it
 * in <b>options</b>. Return 0 if the line is well-formed, and -1 if it
 * isn't.
 *
 * This will mutate only options->TCPProxyProtocol, options->TCPProxyAddr,
 * and options->TCPProxyPort.
 *
 * On error, tor_strdup an error explanation into *<b>msg</b>.
 */
STATIC int
parse_tcp_proxy_line(const char *line, or_options_t *options, char **msg)
{
  int ret = 0;
  tor_assert(line);
  tor_assert(options);
  tor_assert(msg);

  smartlist_t *sl = smartlist_new();
  /* Split between the protocol and the address/port. */
  smartlist_split_string(sl, line, " ",
                         SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, 2);

  /* The address/port is not specified. */
  if (smartlist_len(sl) < 2) {
    *msg = tor_strdup("TCPProxy has no address/port. Please fix.");
    goto err;
  }

  char *protocol_string = smartlist_get(sl, 0);
  char *addrport_string = smartlist_get(sl, 1);

  /* The only currently supported protocol is 'haproxy'. */
  if (strcasecmp(protocol_string, "haproxy")) {
    *msg = tor_strdup("TCPProxy protocol is not supported. Currently "
                      "the only supported protocol is 'haproxy'. "
                      "Please fix.");
    goto err;
  } else {
    /* Otherwise, set the correct protocol. */
    options->TCPProxyProtocol = TCP_PROXY_PROTOCOL_HAPROXY;
  }

  /* Parse the address/port. */
  if (tor_addr_port_lookup(addrport_string, &options->TCPProxyAddr,
                           &options->TCPProxyPort) < 0) {
    *msg = tor_strdup("TCPProxy address/port failed to parse or resolve. "
                      "Please fix.");
    goto err;
  }

  /* Success. */
  ret = 0;
  goto end;

 err:
  ret = -1;
 end:
  SMARTLIST_FOREACH(sl, char *, cp, tor_free(cp));
  smartlist_free(sl);
  return ret;
}

/** Read the contents of a ClientTransportPlugin or ServerTransportPlugin
 * line from <b>line</b>, depending on the value of <b>server</b>. Return 0
 * if the line is well-formed, and -1 if it isn't.
 *
 * If <b>validate_only</b> is 0, the line is well-formed, and the transport is
 * needed by some bridge:
 * - If it's an external proxy line, add the transport described in the line to
 * our internal transport list.
 * - If it's a managed proxy line, launch the managed proxy.
 */
int
pt_parse_transport_line(const or_options_t *options,
                     const char *line, int validate_only,
                     int server)
{

  smartlist_t *items = NULL;
  int r;
  const char *transports = NULL;
  smartlist_t *transport_list = NULL;
  char *type = NULL;
  char *addrport = NULL;
  tor_addr_t addr;
  uint16_t port = 0;
  int socks_ver = PROXY_NONE;

  /* managed proxy options */
  int is_managed = 0;
  char **proxy_argv = NULL;
  char **tmp = NULL;
  int proxy_argc, i;
  int is_useless_proxy = 1;

  int line_length;

  /* Split the line into space-separated tokens */
  items = smartlist_new();
  smartlist_split_string(items, line, NULL,
                         SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, -1);
  line_length = smartlist_len(items);

  if (line_length < 3) {
    log_warn(LD_CONFIG,
             "Too few arguments on %sTransportPlugin line.",
             server ? "Server" : "Client");
    goto err;
  }

  /* Get the first line element, split it to commas into
     transport_list (in case it's multiple transports) and validate
     the transport names. */
  transports = smartlist_get(items, 0);
  transport_list = smartlist_new();
  smartlist_split_string(transport_list, transports, ",",
                         SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, 0);
  SMARTLIST_FOREACH_BEGIN(transport_list, const char *, transport_name) {
    /* validate transport names */
    if (!string_is_C_identifier(transport_name)) {
      log_warn(LD_CONFIG, "Transport name is not a C identifier (%s).",
               transport_name);
      goto err;
    }

    /* see if we actually need the transports provided by this proxy */
    if (!validate_only && transport_is_needed(transport_name))
      is_useless_proxy = 0;
  } SMARTLIST_FOREACH_END(transport_name);

  type = smartlist_get(items, 1);
  if (!strcmp(type, "exec")) {
    is_managed = 1;
  } else if (server && !strcmp(type, "proxy")) {
    /* 'proxy' syntax only with ServerTransportPlugin */
    is_managed = 0;
  } else if (!server && !strcmp(type, "socks4")) {
    /* 'socks4' syntax only with ClientTransportPlugin */
    is_managed = 0;
    socks_ver = PROXY_SOCKS4;
  } else if (!server && !strcmp(type, "socks5")) {
    /* 'socks5' syntax only with ClientTransportPlugin */
    is_managed = 0;
    socks_ver = PROXY_SOCKS5;
  } else {
    log_warn(LD_CONFIG,
             "Strange %sTransportPlugin type '%s'",
             server ? "Server" : "Client", type);
    goto err;
  }

  if (is_managed && options->Sandbox) {
    log_warn(LD_CONFIG,
             "Managed proxies are not compatible with Sandbox mode."
             "(%sTransportPlugin line was %s)",
             server ? "Server" : "Client", escaped(line));
    goto err;
  }

  if (is_managed && options->NoExec) {
    log_warn(LD_CONFIG,
             "Managed proxies are not compatible with NoExec mode; ignoring."
             "(%sTransportPlugin line was %s)",
             server ? "Server" : "Client", escaped(line));
    r = 0;
    goto done;
  }

  if (is_managed) {
    /* managed */

    if (!server && !validate_only && is_useless_proxy) {
      log_info(LD_GENERAL,
               "Pluggable transport proxy (%s) does not provide "
               "any needed transports and will not be launched.",
               line);
    }

    /*
     * If we are not just validating, use the rest of the line as the
     * argv of the proxy to be launched. Also, make sure that we are
     * only launching proxies that contribute useful transports.
     */

    if (!validate_only && (server || !is_useless_proxy)) {
      proxy_argc = line_length - 2;
      tor_assert(proxy_argc > 0);
      proxy_argv = tor_calloc((proxy_argc + 1), sizeof(char *));
      tmp = proxy_argv;

      for (i = 0; i < proxy_argc; i++) {
        /* store arguments */
        *tmp++ = smartlist_get(items, 2);
        smartlist_del_keeporder(items, 2);
      }
      *tmp = NULL; /* terminated with NULL, just like execve() likes it */

      /* kickstart the thing */
      if (server) {
        pt_kickstart_server_proxy(transport_list, proxy_argv);
      } else {
        pt_kickstart_client_proxy(transport_list, proxy_argv);
      }
    }
  } else {
    /* external */

    /* ClientTransportPlugins connecting through a proxy is managed only. */
    if (!server && (options->Socks4Proxy || options->Socks5Proxy ||
                    options->HTTPSProxy || options->TCPProxy)) {
      log_warn(LD_CONFIG, "You have configured an external proxy with another "
                          "proxy type. (Socks4Proxy|Socks5Proxy|HTTPSProxy|"
                          "TCPProxy)");
      goto err;
    }

    if (smartlist_len(transport_list) != 1) {
      log_warn(LD_CONFIG,
               "You can't have an external proxy with more than "
               "one transport.");
      goto err;
    }

    addrport = smartlist_get(items, 2);

    if (tor_addr_port_lookup(addrport, &addr, &port) < 0) {
      log_warn(LD_CONFIG,
               "Error parsing transport address '%s'", addrport);
      goto err;
    }

    if (!port) {
      log_warn(LD_CONFIG,
               "Transport address '%s' has no port.", addrport);
      goto err;
    }

    if (!validate_only) {
      log_info(LD_DIR, "%s '%s' at %s.",
               server ? "Server transport" : "Transport",
               transports, fmt_addrport(&addr, port));

      if (!server) {
        transport_add_from_config(&addr, port,
                                  smartlist_get(transport_list, 0),
                                  socks_ver);
      }
    }
  }

  r = 0;
  goto done;

 err:
  r = -1;

 done:
  SMARTLIST_FOREACH(items, char*, s, tor_free(s));
  smartlist_free(items);
  if (transport_list) {
    SMARTLIST_FOREACH(transport_list, char*, s, tor_free(s));
    smartlist_free(transport_list);
  }

  return r;
}

/**
 * Parse a flag describing an extra dirport for a directory authority.
 *
 * Right now, the supported format is exactly:
 * `{upload,download,voting}=http://[IP:PORT]/`.
 * Other URL schemes, and other suffixes, might be supported in the future.
 *
 * Only call this function if `flag` starts with one of the above strings.
 *
 * Return 0 on success, and -1 on failure.
 *
 * If `ds` is provided, then add any parsed dirport to `ds`.  If `ds` is NULL,
 * take no action other than parsing.
 **/
static int
parse_dirauth_dirport(dir_server_t *ds, const char *flag)
{
  tor_assert(flag);

  auth_dirport_usage_t usage;

  if (!strcasecmpstart(flag, "upload=")) {
    usage = AUTH_USAGE_UPLOAD;
  } else if (!strcasecmpstart(flag, "download=")) {
    usage = AUTH_USAGE_DOWNLOAD;
  } else if (!strcasecmpstart(flag, "vote=")) {
    usage = AUTH_USAGE_VOTING;
  } else {
    // We shouldn't get called with a flag that we don't recognize.
    tor_assert_nonfatal_unreached();
    return -1;
  }

  const char *eq = strchr(flag, '=');
  tor_assert(eq);
  const char *target = eq + 1;

  // Find the part inside the http://{....}/
  if (strcmpstart(target, "http://")) {
    log_warn(LD_CONFIG, "Unsupported URL scheme in authority flag %s", flag);
    return -1;
  }
  const char *addr = target + strlen("http://");

  const char *eos = strchr(addr, '/');
  size_t addr_len;
  if (eos && strcmp(eos, "/")) {
    log_warn(LD_CONFIG, "Unsupported URL prefix in authority flag %s", flag);
    return -1;
  } else if (eos) {
    addr_len = eos - addr;
  } else {
    addr_len = strlen(addr);
  }

  // Finally, parse the addr:port part.
  char *addr_string = tor_strndup(addr, addr_len);
  tor_addr_port_t dirport;
  memset(&dirport, 0, sizeof(dirport));
  int rv = tor_addr_port_parse(LOG_WARN, addr_string,
                               &dirport.addr, &dirport.port, -1);
  if (ds != NULL && rv == 0) {
    trusted_dir_server_add_dirport(ds, usage, &dirport);
  } else if (rv == -1) {
    log_warn(LD_CONFIG, "Unable to parse address in authority flag %s",flag);
  }

  tor_free(addr_string);
  return rv;
}

/** Read the contents of a DirAuthority line from <b>line</b>. If
 * <b>validate_only</b> is 0, and the line is well-formed, and it
 * shares any bits with <b>required_type</b> or <b>required_type</b>
 * is NO_DIRINFO (zero), then add the dirserver described in the line
 * (minus whatever bits it's missing) as a valid authority.
 * Return 0 on success or filtering out by type,
 * or -1 if the line isn't well-formed or if we can't add it. */
STATIC int
parse_dir_authority_line(const char *line, dirinfo_type_t required_type,
                         int validate_only)
{
  smartlist_t *items = NULL;
  int r;
  char *addrport=NULL, *address=NULL, *nickname=NULL, *fingerprint=NULL;
  tor_addr_port_t ipv6_addrport, *ipv6_addrport_ptr = NULL;
  uint16_t dir_port = 0, or_port = 0;
  char digest[DIGEST_LEN];
  char v3_digest[DIGEST_LEN];
  dirinfo_type_t type = 0;
  double weight = 1.0;
  smartlist_t *extra_dirports = smartlist_new();

  memset(v3_digest, 0, sizeof(v3_digest));

  items = smartlist_new();
  smartlist_split_string(items, line, NULL,
                         SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, -1);
  if (smartlist_len(items) < 1) {
    log_warn(LD_CONFIG, "No arguments on DirAuthority line.");
    goto err;
  }

  if (is_legal_nickname(smartlist_get(items, 0))) {
    nickname = smartlist_get(items, 0);
    smartlist_del_keeporder(items, 0);
  }

  while (smartlist_len(items)) {
    char *flag = smartlist_get(items, 0);
    if (TOR_ISDIGIT(flag[0]))
      break;
    if (!strcasecmp(flag, "hs") ||
               !strcasecmp(flag, "no-hs")) {
      log_warn(LD_CONFIG, "The DirAuthority options 'hs' and 'no-hs' are "
               "obsolete; you don't need them any more.");
    } else if (!strcasecmp(flag, "bridge")) {
      type |= BRIDGE_DIRINFO;
    } else if (!strcasecmp(flag, "no-v2")) {
      /* obsolete, but may still be contained in DirAuthority lines generated
         by various tools */;
    } else if (!strcasecmpstart(flag, "orport=")) {
      int ok;
      char *portstring = flag + strlen("orport=");
      or_port = (uint16_t) tor_parse_long(portstring, 10, 1, 65535, &ok, NULL);
      if (!ok)
        log_warn(LD_CONFIG, "Invalid orport '%s' on DirAuthority line.",
                 portstring);
    } else if (!strcmpstart(flag, "weight=")) {
      int ok;
      const char *wstring = flag + strlen("weight=");
      weight = tor_parse_double(wstring, 0, (double)UINT64_MAX, &ok, NULL);
      if (!ok) {
        log_warn(LD_CONFIG, "Invalid weight '%s' on DirAuthority line.",flag);
        weight=1.0;
      }
    } else if (!strcasecmpstart(flag, "v3ident=")) {
      char *idstr = flag + strlen("v3ident=");
      if (strlen(idstr) != HEX_DIGEST_LEN ||
          base16_decode(v3_digest, DIGEST_LEN,
                        idstr, HEX_DIGEST_LEN) != DIGEST_LEN) {
        log_warn(LD_CONFIG, "Bad v3 identity digest '%s' on DirAuthority line",
                 flag);
      } else {
        type |= V3_DIRINFO|EXTRAINFO_DIRINFO|MICRODESC_DIRINFO;
      }
    } else if (!strcasecmpstart(flag, "ipv6=")) {
      if (ipv6_addrport_ptr) {
        log_warn(LD_CONFIG, "Redundant ipv6 addr/port on DirAuthority line");
      } else {
        if (tor_addr_port_parse(LOG_WARN, flag+strlen("ipv6="),
                                &ipv6_addrport.addr, &ipv6_addrport.port,
                                -1) < 0
            || tor_addr_family(&ipv6_addrport.addr) != AF_INET6) {
          log_warn(LD_CONFIG, "Bad ipv6 addr/port %s on DirAuthority line",
                   escaped(flag));
          goto err;
        }
        ipv6_addrport_ptr = &ipv6_addrport;
      }
    } else if (!strcasecmpstart(flag, "upload=") ||
               !strcasecmpstart(flag, "download=") ||
               !strcasecmpstart(flag, "vote=")) {
      // We'll handle these after creating the authority object.
      smartlist_add(extra_dirports, flag);
      flag =  NULL; // prevent double-free.
    } else {
      log_warn(LD_CONFIG, "Unrecognized flag '%s' on DirAuthority line",
               flag);
    }
    tor_free(flag);
    smartlist_del_keeporder(items, 0);
  }

  if (smartlist_len(items) < 2) {
    log_warn(LD_CONFIG, "Too few arguments to DirAuthority line.");
    goto err;
  }
  addrport = smartlist_get(items, 0);
  smartlist_del_keeporder(items, 0);

  if (tor_addr_port_split(LOG_WARN, addrport, &address, &dir_port) < 0) {
    log_warn(LD_CONFIG, "Error parsing DirAuthority address '%s'.", addrport);
    goto err;
  }

  if (!string_is_valid_ipv4_address(address)) {
    log_warn(LD_CONFIG, "Error parsing DirAuthority address '%s' "
                        "(invalid IPv4 address)", address);
    goto err;
  }

  if (!dir_port) {
    log_warn(LD_CONFIG, "Missing port in DirAuthority address '%s'",addrport);
    goto err;
  }

  fingerprint = smartlist_join_strings(items, "", 0, NULL);
  if (strlen(fingerprint) != HEX_DIGEST_LEN) {
    log_warn(LD_CONFIG, "Key digest '%s' for DirAuthority is wrong length %d.",
             fingerprint, (int)strlen(fingerprint));
    goto err;
  }
  if (base16_decode(digest, DIGEST_LEN,
                    fingerprint, HEX_DIGEST_LEN) != DIGEST_LEN) {
    log_warn(LD_CONFIG, "Unable to decode DirAuthority key digest.");
    goto err;
  }

  if (validate_only) {
    SMARTLIST_FOREACH_BEGIN(extra_dirports, const char *, cp) {
      if (parse_dirauth_dirport(NULL, cp) < 0)
        goto err;
    } SMARTLIST_FOREACH_END(cp);
  }

  if (!validate_only && (!required_type || required_type & type)) {
    dir_server_t *ds;
    if (required_type)
      type &= required_type; /* pare down what we think of them as an
                              * authority for. */
    log_debug(LD_DIR, "Trusted %d dirserver at %s:%d (%s)", (int)type,
              address, (int)dir_port, (char*)smartlist_get(items,0));
    if (!(ds = trusted_dir_server_new(nickname, address, dir_port, or_port,
                                      ipv6_addrport_ptr,
                                      digest, v3_digest, type, weight)))
      goto err;

    SMARTLIST_FOREACH_BEGIN(extra_dirports, const char *, cp) {
      if (parse_dirauth_dirport(ds, cp) < 0)
        goto err;
    } SMARTLIST_FOREACH_END(cp);
    dir_server_add(ds);
  }

  r = 0;
  goto done;

 err:
  r = -1;

 done:
  SMARTLIST_FOREACH(extra_dirports, char*, s, tor_free(s));
  smartlist_free(extra_dirports);
  SMARTLIST_FOREACH(items, char*, s, tor_free(s));
  smartlist_free(items);
  tor_free(addrport);
  tor_free(address);
  tor_free(nickname);
  tor_free(fingerprint);
  return r;
}

/** Read the contents of a FallbackDir line from <b>line</b>. If
 * <b>validate_only</b> is 0, and the line is well-formed, then add the
 * dirserver described in the line as a fallback directory. Return 0 on
 * success, or -1 if the line isn't well-formed or if we can't add it. */
int
parse_dir_fallback_line(const char *line,
                        int validate_only)
{
  int r = -1;
  smartlist_t *items = smartlist_new(), *positional = smartlist_new();
  int orport = -1;
  uint16_t dirport;
  tor_addr_t addr;
  int ok;
  char id[DIGEST_LEN];
  char *address=NULL;
  tor_addr_port_t ipv6_addrport, *ipv6_addrport_ptr = NULL;
  double weight=1.0;

  memset(id, 0, sizeof(id));
  smartlist_split_string(items, line, NULL,
                         SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, -1);
  SMARTLIST_FOREACH_BEGIN(items, const char *, cp) {
    const char *eq = strchr(cp, '=');
    ok = 1;
    if (! eq) {
      smartlist_add(positional, (char*)cp);
      continue;
    }
    if (!strcmpstart(cp, "orport=")) {
      orport = (int)tor_parse_long(cp+strlen("orport="), 10,
                                   1, 65535, &ok, NULL);
    } else if (!strcmpstart(cp, "id=")) {
      ok = base16_decode(id, DIGEST_LEN, cp+strlen("id="),
                         strlen(cp)-strlen("id=")) == DIGEST_LEN;
    } else if (!strcasecmpstart(cp, "ipv6=")) {
      if (ipv6_addrport_ptr) {
        log_warn(LD_CONFIG, "Redundant ipv6 addr/port on FallbackDir line");
      } else {
        if (tor_addr_port_parse(LOG_WARN, cp+strlen("ipv6="),
                                &ipv6_addrport.addr, &ipv6_addrport.port,
                                -1) < 0
            || tor_addr_family(&ipv6_addrport.addr) != AF_INET6) {
          log_warn(LD_CONFIG, "Bad ipv6 addr/port %s on FallbackDir line",
                   escaped(cp));
          goto end;
        }
        ipv6_addrport_ptr = &ipv6_addrport;
      }
    } else if (!strcmpstart(cp, "weight=")) {
      int num_ok;
      const char *wstring = cp + strlen("weight=");
      weight = tor_parse_double(wstring, 0, (double)UINT64_MAX, &num_ok, NULL);
      if (!num_ok) {
        log_warn(LD_CONFIG, "Invalid weight '%s' on FallbackDir line.", cp);
        weight=1.0;
      }
    }

    if (!ok) {
      log_warn(LD_CONFIG, "Bad FallbackDir option %s", escaped(cp));
      goto end;
    }
  } SMARTLIST_FOREACH_END(cp);

  if (smartlist_len(positional) != 1) {
    log_warn(LD_CONFIG, "Couldn't parse FallbackDir line %s", escaped(line));
    goto end;
  }

  if (tor_digest_is_zero(id)) {
    log_warn(LD_CONFIG, "Missing identity on FallbackDir line");
    goto end;
  }

  if (orport <= 0) {
    log_warn(LD_CONFIG, "Missing orport on FallbackDir line");
    goto end;
  }

  if (tor_addr_port_split(LOG_INFO, smartlist_get(positional, 0),
                          &address, &dirport) < 0 ||
      tor_addr_parse(&addr, address)<0) {
    log_warn(LD_CONFIG, "Couldn't parse address:port %s on FallbackDir line",
             (const char*)smartlist_get(positional, 0));
    goto end;
  }

  if (!validate_only) {
    dir_server_t *ds;
    ds = fallback_dir_server_new(&addr, dirport, orport, ipv6_addrport_ptr,
                                 id, weight);
    if (!ds) {
      log_warn(LD_CONFIG, "Couldn't create FallbackDir %s", escaped(line));
      goto end;
    }
    dir_server_add(ds);
  }

  r = 0;

 end:
  SMARTLIST_FOREACH(items, char *, cp, tor_free(cp));
  smartlist_free(items);
  smartlist_free(positional);
  tor_free(address);
  return r;
}

/** Allocate and return a new port_cfg_t with reasonable defaults.
 *
 * <b>namelen</b> is the length of the unix socket name
 * (typically the filesystem path), not including the trailing NUL.
 * It should be 0 for ports that are not zunix sockets. */
port_cfg_t *
port_cfg_new(size_t namelen)
{
  tor_assert(namelen <= SIZE_T_CEILING - sizeof(port_cfg_t) - 1);
  port_cfg_t *cfg = tor_malloc_zero(sizeof(port_cfg_t) + namelen + 1);

  /* entry_cfg flags */
  cfg->entry_cfg.ipv4_traffic = 1;
  cfg->entry_cfg.ipv6_traffic = 1;
  cfg->entry_cfg.prefer_ipv6 = 0;
  cfg->entry_cfg.dns_request = 1;
  cfg->entry_cfg.onion_traffic = 1;
  cfg->entry_cfg.prefer_ipv6_virtaddr = 1;
  cfg->entry_cfg.session_group = SESSION_GROUP_UNSET;
  cfg->entry_cfg.isolation_flags = ISO_DEFAULT;

  /* Other flags default to 0 due to tor_malloc_zero */
  return cfg;
}

/** Free all storage held in <b>port</b> */
void
port_cfg_free_(port_cfg_t *port)
{
  tor_free(port);
}

/** Warn for every port in <b>ports</b> of type <b>listener_type</b> that is
 * on a publicly routable address. */
static void
warn_nonlocal_client_ports(const smartlist_t *ports,
                           const char *portname,
                           const int listener_type)
{
  SMARTLIST_FOREACH_BEGIN(ports, const port_cfg_t *, port) {
    if (port->type != listener_type)
      continue;
    if (port->is_unix_addr) {
      /* Unix sockets aren't accessible over a network. */
    } else if (!tor_addr_is_internal(&port->addr, 1)) {
      log_warn(LD_CONFIG, "You specified a public address '%s' for %sPort. "
               "Other people on the Internet might find your computer and "
               "use it as an open proxy. Please don't allow this unless you "
               "have a good reason.",
               fmt_addrport(&port->addr, port->port), portname);
    } else if (!tor_addr_is_loopback(&port->addr)) {
      log_notice(LD_CONFIG, "You configured a non-loopback address '%s' "
                 "for %sPort. This allows everybody on your local network to "
                 "use your machine as a proxy. Make sure this is what you "
                 "wanted.",
                 fmt_addrport(&port->addr, port->port), portname);
    }
  } SMARTLIST_FOREACH_END(port);
}

/** Given a list of port_cfg_t in <b>ports</b>, warn if any controller port
 * there is listening on any non-loopback address.  If <b>forbid_nonlocal</b>
 * is true, then emit a stronger warning and remove the port from the list.
 */
static void
warn_nonlocal_controller_ports(smartlist_t *ports, unsigned forbid_nonlocal)
{
  int warned = 0;
  SMARTLIST_FOREACH_BEGIN(ports, port_cfg_t *, port) {
    if (port->type != CONN_TYPE_CONTROL_LISTENER)
      continue;
    if (port->is_unix_addr)
      continue;
    if (!tor_addr_is_loopback(&port->addr)) {
      if (forbid_nonlocal) {
        if (!warned)
          log_warn(LD_CONFIG,
                 "You have a ControlPort set to accept "
                 "unauthenticated connections from a non-local address.  "
                 "This means that programs not running on your computer "
                 "can reconfigure your Tor, without even having to guess a "
                 "password.  That's so bad that I'm closing your ControlPort "
                 "for you.  If you need to control your Tor remotely, try "
                 "enabling authentication and using a tool like stunnel or "
                 "ssh to encrypt remote access.");
        warned = 1;
        port_cfg_free(port);
        SMARTLIST_DEL_CURRENT(ports, port);
      } else {
        log_warn(LD_CONFIG, "You have a ControlPort set to accept "
                 "connections from a non-local address.  This means that "
                 "programs not running on your computer can reconfigure your "
                 "Tor.  That's pretty bad, since the controller "
                 "protocol isn't encrypted!  Maybe you should just listen on "
                 "127.0.0.1 and use a tool like stunnel or ssh to encrypt "
                 "remote connections to your control port.");
        return; /* No point in checking the rest */
      }
    }
  } SMARTLIST_FOREACH_END(port);
}

/**
 * Take a string (<b>line</b>) that begins with either an address:port, a
 * port, or an AF_UNIX address, optionally quoted, prefixed with
 * "unix:". Parse that line, and on success, set <b>addrport_out</b> to a new
 * string containing the beginning portion (without prefix).  Iff there was a
 * unix: prefix, set <b>is_unix_out</b> to true.  On success, also set
 * <b>rest_out</b> to point to the part of the line after the address portion.
 *
 * Return 0 on success, -1 on failure.
 */
int
port_cfg_line_extract_addrport(const char *line,
                               char **addrport_out,
                               int *is_unix_out,
                               const char **rest_out)
{
  tor_assert(line);
  tor_assert(addrport_out);
  tor_assert(is_unix_out);
  tor_assert(rest_out);

  line = eat_whitespace(line);

  if (!strcmpstart(line, unix_q_socket_prefix)) {
    // It starts with unix:"
    size_t sz;
    *is_unix_out = 1;
    *addrport_out = NULL;
    line += strlen(unix_socket_prefix); /* No 'unix:', but keep the quote */
    *rest_out = unescape_string(line, addrport_out, &sz);
    if (!*rest_out || (*addrport_out && sz != strlen(*addrport_out))) {
      tor_free(*addrport_out);
      return -1;
    }
    *rest_out = eat_whitespace(*rest_out);
    return 0;
  } else {
    // Is there a unix: prefix?
    if (!strcmpstart(line, unix_socket_prefix)) {
      line += strlen(unix_socket_prefix);
      *is_unix_out = 1;
    } else {
      *is_unix_out = 0;
    }

    const char *end = find_whitespace(line);
    if (BUG(!end)) {
      end = strchr(line, '\0'); // LCOV_EXCL_LINE -- this can't be NULL
    }
    tor_assert(end && end >= line);
    *addrport_out = tor_strndup(line, end - line);
    *rest_out = eat_whitespace(end);
    return 0;
  }
}

static void
warn_client_dns_cache(const char *option, int disabling)
{
  if (disabling)
    return;

  warn_deprecated_option(option,
      "Client-side DNS caching enables a wide variety of route-"
      "capture attacks. If a single bad exit node lies to you about "
      "an IP address, caching that address would make you visit "
      "an address of the attacker's choice every time you connected "
      "to your destination.");
}

/**
 * Parse port configuration for a single port type.
 *
 * Read entries of the "FooPort" type from the list <b>ports</b>.  Syntax is
 * that FooPort can have any number of entries of the format
 *  "[Address:][Port] IsolationOptions".
 *
 * In log messages, describe the port type as <b>portname</b>.
 *
 * If no address is specified, default to <b>defaultaddr</b>.  If no
 * FooPort is given, default to defaultport (if 0, there is no default).
 *
 * If CL_PORT_NO_STREAM_OPTIONS is set in <b>flags</b>, do not allow stream
 * isolation options in the FooPort entries.
 *
 * If CL_PORT_WARN_NONLOCAL is set in <b>flags</b>, warn if any of the
 * ports are not on a local address.  If CL_PORT_FORBID_NONLOCAL is set,
 * this is a control port with no password set: don't even allow it.
 *
 * If CL_PORT_SERVER_OPTIONS is set in <b>flags</b>, do not allow stream
 * isolation options in the FooPort entries; instead allow the
 * server-port option set.
 *
 * If CL_PORT_TAKES_HOSTNAMES is set in <b>flags</b>, allow the options
 * {No,}IPv{4,6}Traffic.
 *
 * On success, if <b>out</b> is given, add a new port_cfg_t entry to
 * <b>out</b> for every port that the client should listen on.  Return 0
 * on success, -1 on failure.
 */
int
port_parse_config(smartlist_t *out,
                  const config_line_t *ports,
                  const char *portname,
                  int listener_type,
                  const char *defaultaddr,
                  int defaultport,
                  const unsigned flags)
{
  smartlist_t *elts;
  int retval = -1;
  const unsigned is_control = (listener_type == CONN_TYPE_CONTROL_LISTENER);
  const unsigned is_ext_orport = (listener_type == CONN_TYPE_EXT_OR_LISTENER);
  const unsigned allow_no_stream_options = flags & CL_PORT_NO_STREAM_OPTIONS;
  const unsigned use_server_options = flags & CL_PORT_SERVER_OPTIONS;
  const unsigned warn_nonlocal = flags & CL_PORT_WARN_NONLOCAL;
  const unsigned forbid_nonlocal = flags & CL_PORT_FORBID_NONLOCAL;
  const unsigned default_to_group_writable =
    flags & CL_PORT_DFLT_GROUP_WRITABLE;
  const unsigned takes_hostnames = flags & CL_PORT_TAKES_HOSTNAMES;
  const unsigned is_unix_socket = flags & CL_PORT_IS_UNIXSOCKET;
  int got_zero_port=0, got_nonzero_port=0;
  char *unix_socket_path = NULL;
  port_cfg_t *cfg = NULL;
  bool addr_is_explicit = false;
  tor_addr_t default_addr = TOR_ADDR_NULL;

  /* Parse default address. This can fail for Unix socket so the default_addr
   * will simply be made UNSPEC. */
  if (defaultaddr) {
    tor_addr_parse(&default_addr, defaultaddr);
  }

  /* If there's no FooPort, then maybe make a default one. */
  if (! ports) {
    if (defaultport && defaultaddr && out) {
       cfg = port_cfg_new(is_unix_socket ? strlen(defaultaddr) : 0);
       cfg->type = listener_type;
       if (is_unix_socket) {
         tor_addr_make_unspec(&cfg->addr);
         memcpy(cfg->unix_addr, defaultaddr, strlen(defaultaddr) + 1);
         cfg->is_unix_addr = 1;
       } else {
         cfg->port = defaultport;
         tor_addr_parse(&cfg->addr, defaultaddr);
       }
       smartlist_add(out, cfg);
    }
    return 0;
  }

  /* At last we can actually parse the FooPort lines.  The syntax is:
   * [Addr:](Port|auto) [Options].*/
  elts = smartlist_new();
  char *addrport = NULL;

  for (; ports; ports = ports->next) {
    tor_addr_t addr;
    tor_addr_make_unspec(&addr);
    int port, ok,
        has_used_unix_socket_only_option = 0,
        is_unix_tagged_addr = 0;
    uint16_t ptmp=0;
    const char *rest_of_line = NULL;

    if (port_cfg_line_extract_addrport(ports->value,
                          &addrport, &is_unix_tagged_addr, &rest_of_line)<0) {
      log_warn(LD_CONFIG, "Invalid %sPort line with unparsable address",
               portname);
      goto err;
    }
    if (strlen(addrport) == 0) {
      log_warn(LD_CONFIG, "Invalid %sPort line with no address", portname);
      goto err;
    }

    /* Split the remainder... */
    smartlist_split_string(elts, rest_of_line, NULL,
                           SPLIT_SKIP_SPACE|SPLIT_IGNORE_BLANK, 0);

    /* Let's start to check if it's a Unix socket path. */
    if (is_unix_tagged_addr) {
#ifndef HAVE_SYS_UN_H
      log_warn(LD_CONFIG, "Unix sockets not supported on this system.");
      goto err;
#endif
      unix_socket_path = addrport;
      addrport = NULL;
    }

    if (unix_socket_path &&
        ! conn_listener_type_supports_af_unix(listener_type)) {
      log_warn(LD_CONFIG, "%sPort does not support unix sockets", portname);
      goto err;
    }

    if (unix_socket_path) {
      port = 1;
    } else if (is_unix_socket) {
      if (BUG(!addrport))
        goto err; // LCOV_EXCL_LINE unreachable, but coverity can't tell that
      unix_socket_path = tor_strdup(addrport);
      if (!strcmp(addrport, "0"))
        port = 0;
      else
        port = 1;
    } else if (!strcasecmp(addrport, "auto")) {
      port = CFG_AUTO_PORT;
      tor_addr_copy(&addr, &default_addr);
    } else if (!strcasecmpend(addrport, ":auto")) {
      char *addrtmp = tor_strndup(addrport, strlen(addrport)-5);
      port = CFG_AUTO_PORT;
      if (tor_addr_port_lookup(addrtmp, &addr, &ptmp)<0 || ptmp) {
        log_warn(LD_CONFIG, "Invalid address '%s' for %sPort",
                 escaped(addrport), portname);
        tor_free(addrtmp);
        goto err;
      }
      tor_free(addrtmp);
    } else {
      /* Try parsing integer port before address, because, who knows?
       * "9050" might be a valid address. */
      port = (int) tor_parse_long(addrport, 10, 0, 65535, &ok, NULL);
      if (ok) {
        tor_addr_copy(&addr, &default_addr);
        addr_is_explicit = false;
      } else if (tor_addr_port_lookup(addrport, &addr, &ptmp) == 0) {
        if (ptmp == 0) {
          log_warn(LD_CONFIG, "%sPort line has address but no port", portname);
          goto err;
        }
        port = ptmp;
        addr_is_explicit = true;
      } else {
        log_warn(LD_CONFIG, "Couldn't parse address %s for %sPort",
                 escaped(addrport), portname);
        goto err;
      }
    }

    /* Default port_cfg_t object initialization */
    cfg = port_cfg_new(unix_socket_path ? strlen(unix_socket_path) : 0);

    cfg->explicit_addr = addr_is_explicit;
    if (unix_socket_path && default_to_group_writable)
      cfg->is_group_writable = 1;

    /* Now parse the rest of the options, if any. */
    if (use_server_options) {
      /* This is a server port; parse advertising options */
      SMARTLIST_FOREACH_BEGIN(elts, char *, elt) {
        if (!strcasecmp(elt, "NoAdvertise")) {
          cfg->server_cfg.no_advertise = 1;
        } else if (!strcasecmp(elt, "NoListen")) {
          cfg->server_cfg.no_listen = 1;
#if 0
        /* not implemented yet. */
        } else if (!strcasecmp(elt, "AllAddrs")) {

          all_addrs = 1;
#endif /* 0 */
        } else if (!strcasecmp(elt, "IPv4Only")) {
          cfg->server_cfg.bind_ipv4_only = 1;
        } else if (!strcasecmp(elt, "IPv6Only")) {
          cfg->server_cfg.bind_ipv6_only = 1;
        } else {
          log_warn(LD_CONFIG, "Unrecognized %sPort option '%s'",
                   portname, escaped(elt));
        }
      } SMARTLIST_FOREACH_END(elt);

      if (cfg->server_cfg.no_advertise && cfg->server_cfg.no_listen) {
        log_warn(LD_CONFIG, "Tried to set both NoListen and NoAdvertise "
                 "on %sPort line '%s'",
                 portname, escaped(ports->value));
        goto err;
      }
      if (cfg->server_cfg.bind_ipv4_only &&
          cfg->server_cfg.bind_ipv6_only) {
        log_warn(LD_CONFIG, "Tried to set both IPv4Only and IPv6Only "
                 "on %sPort line '%s'",
                 portname, escaped(ports->value));
        goto err;
      }
      if (cfg->server_cfg.bind_ipv4_only &&
          tor_addr_family(&addr) != AF_INET) {
        if (cfg->explicit_addr) {
          log_warn(LD_CONFIG, "Could not interpret %sPort address as IPv4",
                   portname);
          goto err;
        }
        /* This ORPort is IPv4Only but the default address is IPv6, ignore it
         * since this will be configured with an IPv4 default address. */
        goto ignore;
      }
      if (cfg->server_cfg.bind_ipv6_only &&
          tor_addr_family(&addr) != AF_INET6) {
        if (cfg->explicit_addr) {
          log_warn(LD_CONFIG, "Could not interpret %sPort address as IPv6",
                   portname);
          goto err;
        }
        /* This ORPort is IPv6Only but the default address is IPv4, ignore it
         * since this will be configured with an IPv6 default address. */
        goto ignore;
      }
    } else {
      /* This is a client port; parse isolation options */
      SMARTLIST_FOREACH_BEGIN(elts, char *, elt) {
        int no = 0, isoflag = 0;
        const char *elt_orig = elt;

        if (!strcasecmpstart(elt, "SessionGroup=")) {
          int group = (int)tor_parse_long(elt+strlen("SessionGroup="),
                                          10, 0, INT_MAX, &ok, NULL);
          if (!ok || allow_no_stream_options) {
            log_warn(LD_CONFIG, "Invalid %sPort option '%s'",
                     portname, escaped(elt));
            goto err;
          }
          if (cfg->entry_cfg.session_group >= 0) {
            log_warn(LD_CONFIG, "Multiple SessionGroup options on %sPort",
                     portname);
            goto err;
          }
          cfg->entry_cfg.session_group = group;
          continue;
        }

        if (!strcasecmpstart(elt, "No")) {
          no = 1;
          elt += 2;
        }

        if (!strcasecmp(elt, "GroupWritable")) {
          cfg->is_group_writable = !no;
          has_used_unix_socket_only_option = 1;
          continue;
        } else if (!strcasecmp(elt, "WorldWritable")) {
          cfg->is_world_writable = !no;
          has_used_unix_socket_only_option = 1;
          continue;
        } else if (!strcasecmp(elt, "RelaxDirModeCheck")) {
          cfg->relax_dirmode_check = !no;
          has_used_unix_socket_only_option = 1;
          continue;
        }

        if (allow_no_stream_options) {
          log_warn(LD_CONFIG, "Unrecognized %sPort option '%s'",
                   portname, escaped(elt));
          continue;
        }

        if (takes_hostnames) {
          if (!strcasecmp(elt, "IPv4Traffic")) {
            cfg->entry_cfg.ipv4_traffic = ! no;
            continue;
          } else if (!strcasecmp(elt, "IPv6Traffic")) {
            cfg->entry_cfg.ipv6_traffic = ! no;
            continue;
          } else if (!strcasecmp(elt, "PreferIPv6")) {
            cfg->entry_cfg.prefer_ipv6 = ! no;
            continue;
          } else if (!strcasecmp(elt, "DNSRequest")) {
            cfg->entry_cfg.dns_request = ! no;
            continue;
          } else if (!strcasecmp(elt, "OnionTraffic")) {
            cfg->entry_cfg.onion_traffic = ! no;
            continue;
          } else if (!strcasecmp(elt, "OnionTrafficOnly")) {
            /* Only connect to .onion addresses.  Equivalent to
             * NoDNSRequest, NoIPv4Traffic, NoIPv6Traffic. The option
             * NoOnionTrafficOnly is not supported, it's too confusing. */
            if (no) {
              log_warn(LD_CONFIG, "Unsupported %sPort option 'No%s'. Use "
                       "DNSRequest, IPv4Traffic, and/or IPv6Traffic instead.",
                       portname, escaped(elt));
            } else {
              cfg->entry_cfg.ipv4_traffic = 0;
              cfg->entry_cfg.ipv6_traffic = 0;
              cfg->entry_cfg.dns_request = 0;
            }
            continue;
          }
        }
        if (!strcasecmp(elt, "CacheIPv4DNS")) {
          warn_client_dns_cache(elt, no); // since 0.2.9.2-alpha
          cfg->entry_cfg.cache_ipv4_answers = ! no;
          continue;
        } else if (!strcasecmp(elt, "CacheIPv6DNS")) {
          warn_client_dns_cache(elt, no); // since 0.2.9.2-alpha
          cfg->entry_cfg.cache_ipv6_answers = ! no;
          continue;
        } else if (!strcasecmp(elt, "CacheDNS")) {
          warn_client_dns_cache(elt, no); // since 0.2.9.2-alpha
          cfg->entry_cfg.cache_ipv4_answers = ! no;
          cfg->entry_cfg.cache_ipv6_answers = ! no;
          continue;
        } else if (!strcasecmp(elt, "UseIPv4Cache")) {
          warn_client_dns_cache(elt, no); // since 0.2.9.2-alpha
          cfg->entry_cfg.use_cached_ipv4_answers = ! no;
          continue;
        } else if (!strcasecmp(elt, "UseIPv6Cache")) {
          warn_client_dns_cache(elt, no); // since 0.2.9.2-alpha
          cfg->entry_cfg.use_cached_ipv6_answers = ! no;
          continue;
        } else if (!strcasecmp(elt, "UseDNSCache")) {
          warn_client_dns_cache(elt, no); // since 0.2.9.2-alpha
          cfg->entry_cfg.use_cached_ipv4_answers = ! no;
          cfg->entry_cfg.use_cached_ipv6_answers = ! no;
          continue;
        } else if (!strcasecmp(elt, "PreferIPv6Automap")) {
          cfg->entry_cfg.prefer_ipv6_virtaddr = ! no;
          continue;
        } else if (!strcasecmp(elt, "PreferSOCKSNoAuth")) {
          cfg->entry_cfg.socks_prefer_no_auth = ! no;
          continue;
        } else if (!strcasecmp(elt, "KeepAliveIsolateSOCKSAuth")) {
          cfg->entry_cfg.socks_iso_keep_alive = ! no;
          continue;
        } else if (!strcasecmp(elt, "ExtendedErrors")) {
          cfg->entry_cfg.extended_socks5_codes = ! no;
          continue;
        }

        if (!strcasecmpend(elt, "s"))
          elt[strlen(elt)-1] = '\0'; /* kill plurals. */

        if (!strcasecmp(elt, "IsolateDestPort")) {
          isoflag = ISO_DESTPORT;
        } else if (!strcasecmp(elt, "IsolateDestAddr")) {
          isoflag = ISO_DESTADDR;
        } else if (!strcasecmp(elt, "IsolateSOCKSAuth")) {
          isoflag = ISO_SOCKSAUTH;
        } else if (!strcasecmp(elt, "IsolateClientProtocol")) {
          isoflag = ISO_CLIENTPROTO;
        } else if (!strcasecmp(elt, "IsolateClientAddr")) {
          isoflag = ISO_CLIENTADDR;
        } else {
          log_warn(LD_CONFIG, "Unrecognized %sPort option '%s'",
                   portname, escaped(elt_orig));
        }

        if (no) {
          cfg->entry_cfg.isolation_flags &= ~isoflag;
        } else {
          cfg->entry_cfg.isolation_flags |= isoflag;
        }
      } SMARTLIST_FOREACH_END(elt);
    }

    if (port)
      got_nonzero_port = 1;
    else
      got_zero_port = 1;

    if (cfg->entry_cfg.dns_request == 0 &&
        listener_type == CONN_TYPE_AP_DNS_LISTENER) {
      log_warn(LD_CONFIG, "You have a %sPort entry with DNS disabled; that "
               "won't work.", portname);
      goto err;
    }
    if (cfg->entry_cfg.ipv4_traffic == 0 &&
        cfg->entry_cfg.ipv6_traffic == 0 &&
        cfg->entry_cfg.onion_traffic == 0 &&
        listener_type != CONN_TYPE_AP_DNS_LISTENER) {
      log_warn(LD_CONFIG, "You have a %sPort entry with all of IPv4 and "
               "IPv6 and .onion disabled; that won't work.", portname);
      goto err;
    }
    if (cfg->entry_cfg.dns_request == 1 &&
        cfg->entry_cfg.ipv4_traffic == 0 &&
        cfg->entry_cfg.ipv6_traffic == 0 &&
        listener_type != CONN_TYPE_AP_DNS_LISTENER) {
      log_warn(LD_CONFIG, "You have a %sPort entry with DNSRequest enabled, "
               "but IPv4 and IPv6 disabled; DNS-based sites won't work.",
               portname);
      goto err;
    }
    if (has_used_unix_socket_only_option && !unix_socket_path) {
      log_warn(LD_CONFIG, "You have a %sPort entry with GroupWritable, "
               "WorldWritable, or RelaxDirModeCheck, but it is not a "
               "unix socket.", portname);
      goto err;
    }
    if (!(cfg->entry_cfg.isolation_flags & ISO_SOCKSAUTH) &&
        cfg->entry_cfg.socks_iso_keep_alive) {
      log_warn(LD_CONFIG, "You have a %sPort entry with both "
               "NoIsolateSOCKSAuth and KeepAliveIsolateSOCKSAuth set.",
               portname);
      goto err;
    }
    if (unix_socket_path &&
        (cfg->entry_cfg.isolation_flags & ISO_CLIENTADDR)) {
      /* `IsolateClientAddr` is nonsensical in the context of AF_LOCAL.
       * just silently remove the isolation flag.
       */
      cfg->entry_cfg.isolation_flags &= ~ISO_CLIENTADDR;
    }
    if (out && port) {
      size_t namelen = unix_socket_path ? strlen(unix_socket_path) : 0;
      if (unix_socket_path) {
        tor_addr_make_unspec(&cfg->addr);
        memcpy(cfg->unix_addr, unix_socket_path, namelen + 1);
        cfg->is_unix_addr = 1;
        tor_free(unix_socket_path);
      } else {
        tor_addr_copy(&cfg->addr, &addr);
        cfg->port = port;
      }
      cfg->type = listener_type;
      if (! (cfg->entry_cfg.isolation_flags & ISO_SOCKSAUTH))
        cfg->entry_cfg.socks_prefer_no_auth = 1;
      smartlist_add(out, cfg);
      /* out owns cfg now, don't re-use or free it */
      cfg = NULL;
    }

 ignore:
    tor_free(cfg);
    SMARTLIST_FOREACH(elts, char *, cp, tor_free(cp));
    smartlist_clear(elts);
    tor_free(addrport);
    tor_free(unix_socket_path);
  }

  if (warn_nonlocal && out) {
    if (is_control)
      warn_nonlocal_controller_ports(out, forbid_nonlocal);
    else if (is_ext_orport)
      port_warn_nonlocal_ext_orports(out, portname);
    else
      warn_nonlocal_client_ports(out, portname, listener_type);
  }

  if (got_zero_port && got_nonzero_port) {
    log_warn(LD_CONFIG, "You specified a nonzero %sPort along with '%sPort 0' "
             "in the same configuration. Did you mean to disable %sPort or "
             "not?", portname, portname, portname);
    goto err;
  }

  retval = 0;
 err:
  /* There are two ways we can error out:
   *  1. part way through the loop: cfg needs to be freed;
   *  2. ending the loop normally: cfg is always NULL.
   *     In this case, cfg has either been:
   *     - added to out, then set to NULL, or
   *     - freed and set to NULL (because out is NULL, or port is 0).
   */
  tor_free(cfg);

  /* Free the other variables from the loop.
   * elts is always non-NULL here, but it may or may not be empty. */
  SMARTLIST_FOREACH(elts, char *, cp, tor_free(cp));
  smartlist_free(elts);
  tor_free(unix_socket_path);
  tor_free(addrport);

  return retval;
}

/** Return the number of ports which are actually going to listen with type
 * <b>listenertype</b>.  Do not count no_listen ports.  Only count unix
 * sockets if count_sockets is true. */
int
port_count_real_listeners(const smartlist_t *ports, int listenertype,
                     int count_sockets)
{
  int n = 0;
  SMARTLIST_FOREACH_BEGIN(ports, port_cfg_t *, port) {
    if (port->server_cfg.no_listen)
      continue;
    if (!count_sockets && port->is_unix_addr)
      continue;
    if (port->type != listenertype)
      continue;
    ++n;
  } SMARTLIST_FOREACH_END(port);
  return n;
}

/** Parse all ports from <b>options</b>. On success, set *<b>n_ports_out</b>
 * to the number of ports that are listed, update the *Port_set values in
 * <b>options</b>, and return 0.  On failure, set *<b>msg</b> to a
 * description of the problem and return -1.
 *
 * If <b>validate_only</b> is false, set configured_client_ports to the
 * new list of ports parsed from <b>options</b>.
 **/
STATIC int
parse_ports(or_options_t *options, int validate_only,
            char **msg, int *n_ports_out,
            int *world_writable_control_socket)
{
  smartlist_t *ports;
  int retval = -1;

  ports = smartlist_new();

  *n_ports_out = 0;

  const unsigned gw_flag = options->UnixSocksGroupWritable ?
    CL_PORT_DFLT_GROUP_WRITABLE : 0;
  if (port_parse_config(ports,
             options->SocksPort_lines,
             "Socks", CONN_TYPE_AP_LISTENER,
             "127.0.0.1", 9050,
             ((validate_only ? 0 : CL_PORT_WARN_NONLOCAL)
              | CL_PORT_TAKES_HOSTNAMES | gw_flag)) < 0) {
    *msg = tor_strdup("Invalid SocksPort configuration");
    goto err;
  }
  if (port_parse_config(ports,
                        options->DNSPort_lines,
                        "DNS", CONN_TYPE_AP_DNS_LISTENER,
                        "127.0.0.1", 0,
                        CL_PORT_WARN_NONLOCAL|CL_PORT_TAKES_HOSTNAMES) < 0) {
    *msg = tor_strdup("Invalid DNSPort configuration");
    goto err;
  }
  if (port_parse_config(ports,
                        options->TransPort_lines,
                        "Trans", CONN_TYPE_AP_TRANS_LISTENER,
                        "127.0.0.1", 0,
                        CL_PORT_WARN_NONLOCAL) < 0) {
    *msg = tor_strdup("Invalid TransPort configuration");
    goto err;
  }
  if (port_parse_config(ports,
                        options->NATDPort_lines,
                        "NATD", CONN_TYPE_AP_NATD_LISTENER,
                        "127.0.0.1", 0,
                        CL_PORT_WARN_NONLOCAL) < 0) {
    *msg = tor_strdup("Invalid NatdPort configuration");
    goto err;
  }
  if (port_parse_config(ports,
                        options->HTTPTunnelPort_lines,
                        "HTTP Tunnel", CONN_TYPE_AP_HTTP_CONNECT_LISTENER,
                        "127.0.0.1", 0,
                        ((validate_only ? 0 : CL_PORT_WARN_NONLOCAL)
                         | CL_PORT_TAKES_HOSTNAMES | gw_flag)) < 0) {
    *msg = tor_strdup("Invalid HTTPTunnelPort configuration");
    goto err;
  }
  if (metrics_parse_ports(options, ports, msg) < 0) {
    goto err;
  }

  {
    unsigned control_port_flags = CL_PORT_NO_STREAM_OPTIONS |
      CL_PORT_WARN_NONLOCAL;
    const int any_passwords = (options->HashedControlPassword ||
                               options->HashedControlSessionPassword ||
                               options->CookieAuthentication);
    if (! any_passwords)
      control_port_flags |= CL_PORT_FORBID_NONLOCAL;
    if (options->ControlSocketsGroupWritable)
      control_port_flags |= CL_PORT_DFLT_GROUP_WRITABLE;

    if (port_parse_config(ports,
                          options->ControlPort_lines,
                          "Control", CONN_TYPE_CONTROL_LISTENER,
                          "127.0.0.1", 0,
                          control_port_flags) < 0) {
      *msg = tor_strdup("Invalid ControlPort configuration");
      goto err;
    }

    if (port_parse_config(ports, options->ControlSocket,
                          "ControlSocket",
                          CONN_TYPE_CONTROL_LISTENER, NULL, 0,
                          control_port_flags | CL_PORT_IS_UNIXSOCKET) < 0) {
      *msg = tor_strdup("Invalid ControlSocket configuration");
      goto err;
    }
  }

  if (port_parse_ports_relay(options, msg, ports, &have_low_ports) < 0)
    goto err;

  *n_ports_out = smartlist_len(ports);

  retval = 0;

  /* Update the *Port_set options.  The !! here is to force a boolean out of
     an integer. */
  port_update_port_set_relay(options, ports);
  options->SocksPort_set =
    !! port_count_real_listeners(ports, CONN_TYPE_AP_LISTENER, 1);
  options->TransPort_set =
    !! port_count_real_listeners(ports, CONN_TYPE_AP_TRANS_LISTENER, 1);
  options->NATDPort_set =
    !! port_count_real_listeners(ports, CONN_TYPE_AP_NATD_LISTENER, 1);
  options->HTTPTunnelPort_set =
    !! port_count_real_listeners(ports, CONN_TYPE_AP_HTTP_CONNECT_LISTENER, 1);
  /* Use options->ControlSocket to test if a control socket is set */
  options->ControlPort_set =
    !! port_count_real_listeners(ports, CONN_TYPE_CONTROL_LISTENER, 0);
  options->DNSPort_set =
    !! port_count_real_listeners(ports, CONN_TYPE_AP_DNS_LISTENER, 1);

  if (world_writable_control_socket) {
    SMARTLIST_FOREACH(ports, port_cfg_t *, p,
      if (p->type == CONN_TYPE_CONTROL_LISTENER &&
          p->is_unix_addr &&
          p->is_world_writable) {
        *world_writable_control_socket = 1;
        break;
      });
  }

  if (!validate_only) {
    if (configured_ports) {
      SMARTLIST_FOREACH(configured_ports,
                        port_cfg_t *, p, port_cfg_free(p));
      smartlist_free(configured_ports);
    }
    configured_ports = ports;
    ports = NULL; /* prevent free below. */
  }

 err:
  if (ports) {
    SMARTLIST_FOREACH(ports, port_cfg_t *, p, port_cfg_free(p));
    smartlist_free(ports);
  }
  return retval;
}

/* Does port bind to IPv4? */
int
port_binds_ipv4(const port_cfg_t *port)
{
  return tor_addr_family(&port->addr) == AF_INET ||
         (tor_addr_family(&port->addr) == AF_UNSPEC
          && !port->server_cfg.bind_ipv6_only);
}

/* Does port bind to IPv6? */
int
port_binds_ipv6(const port_cfg_t *port)
{
  return tor_addr_family(&port->addr) == AF_INET6 ||
         (tor_addr_family(&port->addr) == AF_UNSPEC
          && !port->server_cfg.bind_ipv4_only);
}

/** Return a list of port_cfg_t for client ports parsed from the
 * options. */
MOCK_IMPL(const smartlist_t *,
get_configured_ports,(void))
{
  if (!configured_ports)
    configured_ports = smartlist_new();
  return configured_ports;
}

/** Return an address:port string representation of the address
 *  where the first <b>listener_type</b> listener waits for
 *  connections. Return NULL if we couldn't find a listener. The
 *  string is allocated on the heap and it's the responsibility of the
 *  caller to free it after use.
 *
 *  This function is meant to be used by the pluggable transport proxy
 *  spawning code, please make sure that it fits your purposes before
 *  using it. */
char *
get_first_listener_addrport_string(int listener_type)
{
  static const char *ipv4_localhost = "127.0.0.1";
  static const char *ipv6_localhost = "[::1]";
  const char *address;
  uint16_t port;
  char *string = NULL;

  if (!configured_ports)
    return NULL;

  SMARTLIST_FOREACH_BEGIN(configured_ports, const port_cfg_t *, cfg) {
    if (cfg->server_cfg.no_listen)
      continue;

    if (cfg->type == listener_type &&
        tor_addr_family(&cfg->addr) != AF_UNSPEC) {

      /* We found the first listener of the type we are interested in! */

      /* If a listener is listening on INADDR_ANY, assume that it's
         also listening on 127.0.0.1, and point the transport proxy
         there: */
      if (tor_addr_is_null(&cfg->addr))
        address = tor_addr_is_v4(&cfg->addr) ? ipv4_localhost : ipv6_localhost;
      else
        address = fmt_and_decorate_addr(&cfg->addr);

      /* If a listener is configured with port 'auto', we are forced
         to iterate all listener connections and find out in which
         port it ended up listening: */
      if (cfg->port == CFG_AUTO_PORT) {
        port = router_get_active_listener_port_by_type_af(listener_type,
                                                  tor_addr_family(&cfg->addr));
        if (!port)
          return NULL;
      } else {
        port = cfg->port;
      }

      tor_asprintf(&string, "%s:%u", address, port);

      return string;
    }

  } SMARTLIST_FOREACH_END(cfg);

  return NULL;
}

/** Find and return the first configured advertised `port_cfg_t` of type @a
 * listener_type in @a address_family. */
static const port_cfg_t *
portconf_get_first_advertised(int listener_type, int address_family)
{
  const port_cfg_t *first_port = NULL;
  const port_cfg_t *first_port_explicit_addr = NULL;

  if (address_family == AF_UNSPEC)
    return NULL;

  const smartlist_t *conf_ports = get_configured_ports();
  SMARTLIST_FOREACH_BEGIN(conf_ports, const port_cfg_t *, cfg) {
    if (cfg->type == listener_type && !cfg->server_cfg.no_advertise) {
      if ((address_family == AF_INET && port_binds_ipv4(cfg)) ||
          (address_family == AF_INET6 && port_binds_ipv6(cfg))) {
        if (cfg->explicit_addr && !first_port_explicit_addr) {
          first_port_explicit_addr = cfg;
        } else if (!first_port) {
          first_port = cfg;
        }
      }
    }
  } SMARTLIST_FOREACH_END(cfg);

  /* Prefer the port with the explicit address if any. */
  return (first_port_explicit_addr) ? first_port_explicit_addr : first_port;
}

/** Return the first advertised port of type <b>listener_type</b> in
 * <b>address_family</b>. Returns 0 when no port is found, and when passed
 * AF_UNSPEC. */
int
portconf_get_first_advertised_port(int listener_type, int address_family)
{
  const port_cfg_t *cfg;
  cfg = portconf_get_first_advertised(listener_type, address_family);

  return cfg ? cfg->port : 0;
}

/** Return the first advertised address of type <b>listener_type</b> in
 * <b>address_family</b>. Returns NULL if there is no advertised address,
 * and when passed AF_UNSPEC. */
const tor_addr_t *
portconf_get_first_advertised_addr(int listener_type, int address_family)
{
  const port_cfg_t *cfg;
  cfg = portconf_get_first_advertised(listener_type, address_family);

  return cfg ? &cfg->addr : NULL;
}

/** Return 1 if a port exists of type <b>listener_type</b> on <b>addr</b> and
 * <b>port</b>. If <b>check_wildcard</b> is true, INADDR[6]_ANY and AF_UNSPEC
 * addresses match any address of the appropriate family; and port -1 matches
 * any port.
 * To match auto ports, pass CFG_PORT_AUTO. (Does not match on the actual
 * automatically chosen listener ports.) */
int
port_exists_by_type_addr_port(int listener_type, const tor_addr_t *addr,
                              int port, int check_wildcard)
{
  if (!configured_ports || !addr)
    return 0;
  SMARTLIST_FOREACH_BEGIN(configured_ports, const port_cfg_t *, cfg) {
    if (cfg->type == listener_type) {
      if (cfg->port == port || (check_wildcard && port == -1)) {
        /* Exact match */
        if (tor_addr_eq(&cfg->addr, addr)) {
          return 1;
        }
        /* Skip wildcard matches if we're not doing them */
        if (!check_wildcard) {
          continue;
        }
        /* Wildcard matches IPv4 */
        const int cfg_v4 = port_binds_ipv4(cfg);
        const int cfg_any_v4 = tor_addr_is_null(&cfg->addr) && cfg_v4;
        const int addr_v4 = tor_addr_family(addr) == AF_INET ||
                            tor_addr_family(addr) == AF_UNSPEC;
        const int addr_any_v4 = tor_addr_is_null(&cfg->addr) && addr_v4;
        if ((cfg_any_v4 && addr_v4) || (cfg_v4 && addr_any_v4)) {
          return 1;
        }
        /* Wildcard matches IPv6 */
        const int cfg_v6 = port_binds_ipv6(cfg);
        const int cfg_any_v6 = tor_addr_is_null(&cfg->addr) && cfg_v6;
        const int addr_v6 = tor_addr_family(addr) == AF_INET6 ||
                            tor_addr_family(addr) == AF_UNSPEC;
        const int addr_any_v6 = tor_addr_is_null(&cfg->addr) && addr_v6;
        if ((cfg_any_v6 && addr_v6) || (cfg_v6 && addr_any_v6)) {
          return 1;
        }
      }
    }
  } SMARTLIST_FOREACH_END(cfg);
  return 0;
}

/* Like port_exists_by_type_addr_port, but accepts a host-order IPv4 address
 * instead. */
int
port_exists_by_type_addr32h_port(int listener_type, uint32_t addr_ipv4h,
                                 int port, int check_wildcard)
{
  tor_addr_t ipv4;
  tor_addr_from_ipv4h(&ipv4, addr_ipv4h);
  return port_exists_by_type_addr_port(listener_type, &ipv4, port,
                                       check_wildcard);
}

/** Allocate and return a good value for the DataDirectory based on
 *  <b>val</b>, which may be NULL.  Return NULL on failure. */
static char *
get_data_directory(const char *val)
{
#ifdef _WIN32
  if (val) {
    return tor_strdup(val);
  } else {
    return tor_strdup(get_windows_conf_root());
  }
#else /* !defined(_WIN32) */
  const char *d = val;
  if (!d)
    d = "~/.tor";

  if (!strcmpstart(d, "~/")) {
    char *fn = expand_filename(d);
    if (!fn) {
      log_warn(LD_CONFIG,"Failed to expand filename \"%s\".", d);
      return NULL;
    }
    if (!val && !strcmp(fn,"/.tor")) {
      /* If our homedir is /, we probably don't want to use it. */
      /* Default to LOCALSTATEDIR/tor which is probably closer to what we
       * want. */
      log_warn(LD_CONFIG,
               "Default DataDirectory is \"~/.tor\".  This expands to "
               "\"%s\", which is probably not what you want.  Using "
               "\"%s"PATH_SEPARATOR"tor\" instead", fn, LOCALSTATEDIR);
      tor_free(fn);
      fn = tor_strdup(LOCALSTATEDIR PATH_SEPARATOR "tor");
    }
    return fn;
  }
  return tor_strdup(d);
#endif /* defined(_WIN32) */
}

/** Check and normalize the values of options->{Key,Data,Cache}Directory;
 * return 0 if it is sane, -1 otherwise. */
static int
validate_data_directories(or_options_t *options)
{
  tor_free(options->DataDirectory);
  options->DataDirectory = get_data_directory(options->DataDirectory_option);
  if (!options->DataDirectory)
    return -1;
  if (strlen(options->DataDirectory) > (512-128)) {
    log_warn(LD_CONFIG, "DataDirectory is too long.");
    return -1;
  }

  tor_free(options->KeyDirectory);
  if (options->KeyDirectory_option) {
    options->KeyDirectory = get_data_directory(options->KeyDirectory_option);
    if (!options->KeyDirectory)
      return -1;
  } else {
    /* Default to the data directory's keys subdir */
    tor_asprintf(&options->KeyDirectory, "%s"PATH_SEPARATOR"keys",
                 options->DataDirectory);
  }

  tor_free(options->CacheDirectory);
  if (options->CacheDirectory_option) {
    options->CacheDirectory = get_data_directory(
                                             options->CacheDirectory_option);
    if (!options->CacheDirectory)
      return -1;
  } else {
    /* Default to the data directory. */
    options->CacheDirectory = tor_strdup(options->DataDirectory);
  }

  return 0;
}

/** This string must remain the same forevermore. It is how we
 * recognize that the torrc file doesn't need to be backed up. */
#define GENERATED_FILE_PREFIX "# This file was generated by Tor; " \
  "if you edit it, comments will not be preserved"
/** This string can change; it tries to give the reader an idea
 * that editing this file by hand is not a good plan. */
#define GENERATED_FILE_COMMENT "# The old torrc file was renamed " \
  "to torrc.orig.1, and Tor will ignore it"

/** Save a configuration file for the configuration in <b>options</b>
 * into the file <b>fname</b>.  If the file already exists, and
 * doesn't begin with GENERATED_FILE_PREFIX, rename it.  Otherwise
 * replace it.  Return 0 on success, -1 on failure. */
static int
write_configuration_file(const char *fname, const or_options_t *options)
{
  char *old_val=NULL, *new_val=NULL, *new_conf=NULL;
  int rename_old = 0, r;

  if (!fname)
    return -1;

  switch (file_status(fname)) {
    /* create backups of old config files, even if they're empty */
    case FN_FILE:
    case FN_EMPTY:
      old_val = read_file_to_str(fname, 0, NULL);
      if (!old_val || strcmpstart(old_val, GENERATED_FILE_PREFIX)) {
        rename_old = 1;
      }
      tor_free(old_val);
      break;
    case FN_NOENT:
      break;
    case FN_ERROR:
    case FN_DIR:
    default:
      log_warn(LD_CONFIG,
               "Config file \"%s\" is not a file? Failing.", fname);
      return -1;
  }

  if (!(new_conf = options_dump(options, OPTIONS_DUMP_MINIMAL))) {
    log_warn(LD_BUG, "Couldn't get configuration string");
    goto err;
  }

  tor_asprintf(&new_val, "%s\n%s\n\n%s",
               GENERATED_FILE_PREFIX, GENERATED_FILE_COMMENT, new_conf);

  if (rename_old) {
    char *fn_tmp = NULL;
    tor_asprintf(&fn_tmp, CONFIG_BACKUP_PATTERN, fname);
    file_status_t fn_tmp_status = file_status(fn_tmp);
    if (fn_tmp_status == FN_DIR || fn_tmp_status == FN_ERROR) {
      log_warn(LD_CONFIG,
               "Config backup file \"%s\" is not a file? Failing.", fn_tmp);
      tor_free(fn_tmp);
      goto err;
    }

    log_notice(LD_CONFIG, "Renaming old configuration file to \"%s\"", fn_tmp);
    if (replace_file(fname, fn_tmp) < 0) {
      log_warn(LD_FS,
               "Couldn't rename configuration file \"%s\" to \"%s\": %s",
               fname, fn_tmp, strerror(errno));
      tor_free(fn_tmp);
      goto err;
    }
    tor_free(fn_tmp);
  }

  if (write_str_to_file(fname, new_val, 0) < 0)
    goto err;

  r = 0;
  goto done;
 err:
  r = -1;
 done:
  tor_free(new_val);
  tor_free(new_conf);
  return r;
}

/**
 * Save the current configuration file value to disk.  Return 0 on
 * success, -1 on failure.
 **/
int
options_save_current(void)
{
  /* This fails if we can't write to our configuration file.
   *
   * If we try falling back to datadirectory or something, we have a better
   * chance of saving the configuration, but a better chance of doing
   * something the user never expected. */
  return write_configuration_file(get_torrc_fname(0), get_options());
}

/** Return the number of cpus configured in <b>options</b>.  If we are
 * told to auto-detect the number of cpus, return the auto-detected number. */
int
get_num_cpus(const or_options_t *options)
{
  if (options->NumCPUs == 0) {
    int n = compute_num_cpus();
    return (n >= 1) ? n : 1;
  } else {
    return options->NumCPUs;
  }
}

/**
 * Initialize the libevent library.
 */
static void
init_libevent(const or_options_t *options)
{
  tor_libevent_cfg_t cfg;

  tor_assert(options);

  configure_libevent_logging();
  /* If the kernel complains that some method (say, epoll) doesn't
   * exist, we don't care about it, since libevent will cope.
   */
  suppress_libevent_log_msg("Function not implemented");

  memset(&cfg, 0, sizeof(cfg));
  cfg.num_cpus = get_num_cpus(options);
  cfg.msec_per_tick = options->TokenBucketRefillInterval;

  tor_libevent_initialize(&cfg);

  suppress_libevent_log_msg(NULL);
}

/** Return a newly allocated string holding a filename relative to the
 * directory in <b>options</b> specified by <b>roottype</b>.
 * If <b>sub1</b> is present, it is the first path component after
 * the data directory.  If <b>sub2</b> is also present, it is the second path
 * component after the data directory.  If <b>suffix</b> is present, it
 * is appended to the filename.
 *
 * Note: Consider using macros in config.h that wrap this function;
 * you should probably never need to call it as-is.
 */
MOCK_IMPL(char *,
options_get_dir_fname2_suffix,(const or_options_t *options,
                               directory_root_t roottype,
                               const char *sub1, const char *sub2,
                               const char *suffix))
{
  tor_assert(options);

  const char *rootdir = NULL;
  switch (roottype) {
    case DIRROOT_DATADIR:
      rootdir = options->DataDirectory;
      break;
    case DIRROOT_CACHEDIR:
      rootdir = options->CacheDirectory;
      break;
    case DIRROOT_KEYDIR:
      rootdir = options->KeyDirectory;
      break;
    default:
      tor_assert_unreached();
      break;
  }
  tor_assert(rootdir);

  if (!suffix)
    suffix = "";

  char *fname = NULL;

  if (sub1 == NULL) {
    tor_asprintf(&fname, "%s%s", rootdir, suffix);
    tor_assert(!sub2); /* If sub2 is present, sub1 must be present. */
  } else if (sub2 == NULL) {
    tor_asprintf(&fname, "%s"PATH_SEPARATOR"%s%s", rootdir, sub1, suffix);
  } else {
    tor_asprintf(&fname, "%s"PATH_SEPARATOR"%s"PATH_SEPARATOR"%s%s",
                 rootdir, sub1, sub2, suffix);
  }

  return fname;
}

/** Check whether the data directory has a private subdirectory
 * <b>subdir</b>. If not, try to create it. Return 0 on success,
 * -1 otherwise. */
int
check_or_create_data_subdir(const char *subdir)
{
  char *statsdir = get_datadir_fname(subdir);
  int return_val = 0;

  if (check_private_dir(statsdir, CPD_CREATE, get_options()->User) < 0) {
    log_warn(LD_HIST, "Unable to create %s/ directory!", subdir);
    return_val = -1;
  }
  tor_free(statsdir);
  return return_val;
}

/** Create a file named <b>fname</b> with contents <b>str</b> in the
 * subdirectory <b>subdir</b> of the data directory. <b>descr</b>
 * should be a short description of the file's content and will be
 * used for the warning message, if it's present and the write process
 * fails. Return 0 on success, -1 otherwise.*/
int
write_to_data_subdir(const char* subdir, const char* fname,
                     const char* str, const char* descr)
{
  char *filename = get_datadir_fname2(subdir, fname);
  int return_val = 0;

  if (write_str_to_file(filename, str, 0) < 0) {
    log_warn(LD_HIST, "Unable to write %s to disk!", descr ? descr : fname);
    return_val = -1;
  }
  tor_free(filename);
  return return_val;
}

/** Helper to implement GETINFO functions about configuration variables (not
 * their values).  Given a "config/names" question, set *<b>answer</b> to a
 * new string describing the supported configuration variables and their
 * types. */
int
getinfo_helper_config(control_connection_t *conn,
                      const char *question, char **answer,
                      const char **errmsg)
{
  (void) conn;
  (void) errmsg;
  if (!strcmp(question, "config/names")) {
    smartlist_t *sl = smartlist_new();
    smartlist_t *vars = config_mgr_list_vars(get_options_mgr());
    SMARTLIST_FOREACH_BEGIN(vars, const config_var_t *, var) {
      /* don't tell controller about invisible options */
      if (! config_var_is_listable(var))
        continue;
      const char *type = struct_var_get_typename(&var->member);
      if (!type)
        continue;
      smartlist_add_asprintf(sl, "%s %s\n",var->member.name,type);
    } SMARTLIST_FOREACH_END(var);
    *answer = smartlist_join_strings(sl, "", 0, NULL);
    SMARTLIST_FOREACH(sl, char *, c, tor_free(c));
    smartlist_free(sl);
    smartlist_free(vars);
  } else if (!strcmp(question, "config/defaults")) {
    smartlist_t *sl = smartlist_new();
    int dirauth_lines_seen = 0, fallback_lines_seen = 0;
    /* Possibly this should check whether the variables are listable,
     * but currently it does not.  See ticket 31654. */
    smartlist_t *vars = config_mgr_list_vars(get_options_mgr());
    SMARTLIST_FOREACH_BEGIN(vars, const config_var_t *, var) {
      if (var->initvalue != NULL) {
        if (strcmp(var->member.name, "DirAuthority") == 0) {
          /*
           * Count dirauth lines we have a default for; we'll use the
           * count later to decide whether to add the defaults manually
           */
          ++dirauth_lines_seen;
        }
        if (strcmp(var->member.name, "FallbackDir") == 0) {
          /*
           * Similarly count fallback lines, so that we can decide later
           * to add the defaults manually.
           */
          ++fallback_lines_seen;
        }
        char *val = esc_for_log(var->initvalue);
        smartlist_add_asprintf(sl, "%s %s\n",var->member.name,val);
        tor_free(val);
      }
    } SMARTLIST_FOREACH_END(var);
    smartlist_free(vars);

    if (dirauth_lines_seen == 0) {
      /*
       * We didn't see any directory authorities with default values,
       * so add the list of default authorities manually.
       */

      /*
       * default_authorities is defined earlier in this file and
       * is a const char ** NULL-terminated array of dirauth config
       * lines.
       */
      for (const char **i = default_authorities; *i != NULL; ++i) {
        char *val = esc_for_log(*i);
        smartlist_add_asprintf(sl, "DirAuthority %s\n", val);
        tor_free(val);
      }
    }

    if (fallback_lines_seen == 0 &&
        get_options()->UseDefaultFallbackDirs == 1) {
      /*
       * We didn't see any explicitly configured fallback mirrors,
       * so add the defaults to the list manually.
       *
       * default_fallbacks is included earlier in this file and
       * is a const char ** NULL-terminated array of fallback config lines.
       */
      const char **i;

      for (i = default_fallbacks; *i != NULL; ++i) {
        char *val = esc_for_log(*i);
        smartlist_add_asprintf(sl, "FallbackDir %s\n", val);
        tor_free(val);
      }
    }

    *answer = smartlist_join_strings(sl, "", 0, NULL);
    SMARTLIST_FOREACH(sl, char *, c, tor_free(c));
    smartlist_free(sl);
  }
  return 0;
}

/* Check whether an address has already been set against the options
 * depending on address family and destination type. Any existing
 * value will lead to a fail, even if it is the same value. If not
 * set and not only validating, copy it into this location too.
 * Returns 0 on success or -1 if this address is already set.
 */
static int
verify_and_store_outbound_address(sa_family_t family, tor_addr_t *addr,
       outbound_addr_t type, or_options_t *options, int validate_only)
{
  if (type>=OUTBOUND_ADDR_MAX || (family!=AF_INET && family!=AF_INET6)) {
    return -1;
  }
  int fam_index=0;
  if (family==AF_INET6) {
    fam_index=1;
  }
  tor_addr_t *dest=&options->OutboundBindAddresses[type][fam_index];
  if (!tor_addr_is_null(dest)) {
    return -1;
  }
  if (!validate_only) {
    tor_addr_copy(dest, addr);
  }
  return 0;
}

/* Parse a list of address lines for a specific destination type.
 * Will store them into the options if not validate_only. If a
 * problem occurs, a suitable error message is store in msg.
 * Returns 0 on success or -1 if any address is already set.
 */
static int
parse_outbound_address_lines(const config_line_t *lines, outbound_addr_t type,
           or_options_t *options, int validate_only, char **msg)
{
  tor_addr_t addr;
  sa_family_t family;
  while (lines) {
    family = tor_addr_parse(&addr, lines->value);
    if (verify_and_store_outbound_address(family, &addr, type,
                                 options, validate_only)) {
      if (msg)
        tor_asprintf(msg, "Multiple%s%s outbound bind addresses "
                     "configured: %s",
                     family==AF_INET?" IPv4":(family==AF_INET6?" IPv6":""),
                     type==OUTBOUND_ADDR_OR?" OR":
                     (type==OUTBOUND_ADDR_EXIT?" exit":
                     (type==OUTBOUND_ADDR_PT?" PT":"")), lines->value);
      return -1;
    }
    lines = lines->next;
  }
  return 0;
}

/** Parse outbound bind address option lines. If <b>validate_only</b>
 * is not 0 update OutboundBindAddresses in <b>options</b>.
 * Only one address can be set for any of these values.
 * On failure, set <b>msg</b> (if provided) to a newly allocated string
 * containing a description of the problem and return -1.
 */
static int
parse_outbound_addresses(or_options_t *options, int validate_only, char **msg)
{
  if (!validate_only) {
    memset(&options->OutboundBindAddresses, 0,
           sizeof(options->OutboundBindAddresses));
  }

  if (parse_outbound_address_lines(options->OutboundBindAddress,
                                   OUTBOUND_ADDR_ANY, options,
                                   validate_only, msg) < 0) {
    goto err;
  }

  if (parse_outbound_address_lines(options->OutboundBindAddressOR,
                                   OUTBOUND_ADDR_OR, options, validate_only,
                                   msg) < 0) {
    goto err;
  }

  if (parse_outbound_address_lines(options->OutboundBindAddressExit,
                                   OUTBOUND_ADDR_EXIT, options, validate_only,
                                   msg)  < 0) {
    goto err;
  }

  if (parse_outbound_address_lines(options->OutboundBindAddressPT,
                                   OUTBOUND_ADDR_PT, options, validate_only,
                                   msg) < 0) {
    goto err;
  }

  return 0;
 err:
  return -1;
}

/** Load one of the geoip files, <a>family</a> determining which
 * one. <a>default_fname</a> is used if on Windows and
 * <a>fname</a> equals "<default>". */
static void
config_load_geoip_file_(sa_family_t family,
                        const char *fname,
                        const char *default_fname)
{
  const or_options_t *options = get_options();
  const char *msg = "";
  int severity = options_need_geoip_info(options, &msg) ? LOG_WARN : LOG_INFO;
  int r;

#ifdef _WIN32
  char *free_fname = NULL; /* Used to hold any temporary-allocated value */
  /* XXXX Don't use this "<default>" junk; make our filename options
   * understand prefixes somehow. -NM */
  if (!strcmp(fname, "<default>")) {
    const char *conf_root = get_windows_conf_root();
    tor_asprintf(&free_fname, "%s\\%s", conf_root, default_fname);
    fname = free_fname;
  }
  r = geoip_load_file(family, fname, severity);
  tor_free(free_fname);
#else /* !defined(_WIN32) */
  (void)default_fname;
  r = geoip_load_file(family, fname, severity);
#endif /* defined(_WIN32) */

  if (r < 0 && severity == LOG_WARN) {
    log_warn(LD_GENERAL, "%s", msg);
  }
}

/** Load geoip files for IPv4 and IPv6 if <a>options</a> and
 * <a>old_options</a> indicate we should. */
static void
config_maybe_load_geoip_files_(const or_options_t *options,
                               const or_options_t *old_options)
{
  /* XXXX Reload GeoIPFile on SIGHUP. -NM */

  if (options->GeoIPFile &&
      ((!old_options || !opt_streq(old_options->GeoIPFile,
                                   options->GeoIPFile))
       || !geoip_is_loaded(AF_INET))) {
    config_load_geoip_file_(AF_INET, options->GeoIPFile, "geoip");
    /* Okay, now we need to maybe change our mind about what is in
     * which country. We do this for IPv4 only since that's what we
     * store in node->country. */
    refresh_all_country_info();
  }
  if (options->GeoIPv6File &&
      ((!old_options || !opt_streq(old_options->GeoIPv6File,
                                   options->GeoIPv6File))
       || !geoip_is_loaded(AF_INET6))) {
    config_load_geoip_file_(AF_INET6, options->GeoIPv6File, "geoip6");
  }
}

/** Initialize cookie authentication (used so far by the ControlPort
 *  and Extended ORPort).
 *
 *  Allocate memory and create a cookie (of length <b>cookie_len</b>)
 *  in <b>cookie_out</b>.
 *  Then write it down to <b>fname</b> and prepend it with <b>header</b>.
 *
 *  If <b>group_readable</b> is set, set <b>fname</b> to be readable
 *  by the default GID.
 *
 *  If the whole procedure was successful, set
 *  <b>cookie_is_set_out</b> to True. */
int
init_cookie_authentication(const char *fname, const char *header,
                           int cookie_len, int group_readable,
                           uint8_t **cookie_out, int *cookie_is_set_out)
{
  char cookie_file_str_len = strlen(header) + cookie_len;
  char *cookie_file_str = tor_malloc(cookie_file_str_len);
  int retval = -1;

  /* We don't want to generate a new cookie every time we call
   * options_act(). One should be enough. */
  if (*cookie_is_set_out) {
    retval = 0; /* we are all set */
    goto done;
  }

  /* If we've already set the cookie, free it before re-setting
     it. This can happen if we previously generated a cookie, but
     couldn't write it to a disk. */
  if (*cookie_out)
    tor_free(*cookie_out);

  /* Generate the cookie */
  *cookie_out = tor_malloc(cookie_len);
  crypto_rand((char *)*cookie_out, cookie_len);

  /* Create the string that should be written on the file. */
  memcpy(cookie_file_str, header, strlen(header));
  memcpy(cookie_file_str+strlen(header), *cookie_out, cookie_len);
  if (write_bytes_to_file(fname, cookie_file_str, cookie_file_str_len, 1)) {
    log_warn(LD_FS,"Error writing auth cookie to %s.", escaped(fname));
    goto done;
  }

#ifndef _WIN32
  if (group_readable) {
    if (chmod(fname, 0640)) {
      log_warn(LD_FS,"Unable to make %s group-readable.", escaped(fname));
    }
  }
#else /* defined(_WIN32) */
  (void) group_readable;
#endif /* !defined(_WIN32) */

  /* Success! */
  log_info(LD_GENERAL, "Generated auth cookie file in '%s'.", escaped(fname));
  *cookie_is_set_out = 1;
  retval = 0;

 done:
  memwipe(cookie_file_str, 0, cookie_file_str_len);
  tor_free(cookie_file_str);
  return retval;
}

/**
 * Return true if any option is set in <b>options</b> to make us behave
 * as a client.
 */
int
options_any_client_port_set(const or_options_t *options)
{
  return (options->SocksPort_set ||
          options->TransPort_set ||
          options->NATDPort_set ||
          options->DNSPort_set ||
          options->HTTPTunnelPort_set);
}
