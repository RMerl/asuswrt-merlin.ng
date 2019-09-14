/* This is the header file for the rsyslog runtime. It must be included
 * if someone intends to use the runtime.
 *
 * Begun 2005-09-15 RGerhards
 *
 * Copyright (C) 2005-2018 by Rainer Gerhards and Adiscon GmbH
 *
 * This file is part of the rsyslog runtime library.
 *
 * The rsyslog runtime library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The rsyslog runtime library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the rsyslog runtime library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 * A copy of the LGPL can be found in the file "COPYING.LESSER" in this distribution.
 */
#ifndef INCLUDED_RSYSLOG_H
#define INCLUDED_RSYSLOG_H
#ifndef _AIX
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"
#pragma GCC diagnostic ignored "-Wredundant-decls" // TODO: remove!
#pragma GCC diagnostic ignored "-Wstrict-prototypes" // TODO: remove!
#pragma GCC diagnostic ignored "-Wswitch-default" // TODO: remove!
#if __GNUC__ >= 8
/* GCC, starting at least with version 8, is now really overdoing with it's
 * warning messages. We turn those off that either cause an enormous amount
 * of false positives or flag perfectly legal code as problematic.
 */
/* That one causes warnings when we use variable buffers for error
 * messages which may be truncated in the very unlikely case of all
 * vars using max value. If going over the max size, the engine will
 * most likely truncate due to max message size anyhow. Also, sizing
 * the buffers for max-max message size is a wast of (stack) memory.
 */
#pragma GCC diagnostic ignored "-Wformat-truncation"
/* The next one flags variable initializations within out exception handling
 * (iRet system) as problematic, even though variables are not used in those
 * cases. This would be a good diagnostic if gcc would actually check that
 * a variable is used uninitialized. Unfortunately it does not do that. But
 * the static analyzers we use as part of CI do, so we are covered in any
 * case.
 * Unfortunately ignoring this diagnostic leads to two more info lines
 * being emitted where nobody knows what the mean and why they appear :-(
 */
#pragma GCC diagnostic ignored "-Wjump-misses-init"
#endif /* if __GNUC__ >= 8 */
#endif /* ifndef AIX */

#include <pthread.h>
#include "typedefs.h"

#if defined(_AIX)
#include <sys/select.h>
/* AIXPORT : start*/
#define SRC_FD          13
#define SRCMSG          (sizeof(srcpacket))
extern int src_exists;
#endif
/* src end */

/* define a couple of attributes to improve cross-platform builds */
#if __GNUC__ > 6
	#define CASE_FALLTHROUGH __attribute__((fallthrough));
#else
	#define CASE_FALLTHROUGH
#endif

#define ATTR_NORETURN __attribute__ ((noreturn))
#define ATTR_UNUSED __attribute__((unused))
#define ATTR_NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))

/* ############################################################# *
 * #                 Some constant values                      # *
 * ############################################################# */
#define CONST_LEN_TIMESTAMP_3164 15 		/* number of chars (excluding \0!) in a RFC3164 timestamp */
#define CONST_LEN_TIMESTAMP_3339 32 		/* number of chars (excluding \0!) in a RFC3339 timestamp */

#define CONST_LEN_CEE_COOKIE 5
#define CONST_CEE_COOKIE "@cee:"

/* ############################################################# *
 * #                    Config Settings                        # *
 * ############################################################# */
#define RS_STRINGBUF_ALLOC_INCREMENT	128
/* MAXSIZE are absolute maxima, while BUFSIZE are just values after which
 * processing is more time-intense. The BUFSIZE params currently add their
 * value to the fixed size of the message object.
 */
#define CONF_TAG_MAXSIZE		512	/* a value that is deemed far too large for any valid TAG */
#define CONF_HOSTNAME_MAXSIZE		512	/* a value that is deemed far too large for any valid HOSTNAME */
#define CONF_RAWMSG_BUFSIZE		101
#define CONF_TAG_BUFSIZE		32
#define CONF_PROGNAME_BUFSIZE		16
#define CONF_HOSTNAME_BUFSIZE		32
#define CONF_PROP_BUFSIZE		16	/* should be close to sizeof(ptr) or lighly above it */
#define CONF_IPARAMS_BUFSIZE		16	/* initial size of iparams array in wti (is automatically extended) */
#define	CONF_MIN_SIZE_FOR_COMPRESS	60 	/* config param: minimum message size to try compression. The smaller
	 * the message, the less likely is any compression gain. We check for
	 * gain before we submit the message. But to do so we still need to
	 * do the (costly) compress() call. The following setting sets a size
	 * for which no call to compress() is done at all. This may result in
	 * a few more bytes being transmited but better overall performance.
	 * Note: I have not yet checked the minimum UDP packet size. It might be
	 * that we do not save anything by compressing very small messages, because
	 * UDP might need to pad ;)
	 * rgerhards, 2006-11-30
	 */

#define CONF_OMOD_NUMSTRINGS_MAXSIZE	5	/* cache for pointers to output module buffer pointers. All
	 * rsyslog-provided plugins do NOT need more than five buffers. If
	 * more are needed (future developments, third-parties), rsyslog
	 * must be recompiled with a larger parameter. Hardcoding this
	 * saves us some overhead, both in runtime in code complexity. As
	 * it is doubtful if ever more than 3 parameters are needed, the
	 * approach taken here is considered appropriate.
	 * rgerhards, 2010-06-24
	 */
#define CONF_NUM_MULTISUB		1024	/* default number of messages per multisub structure */

/* ############################################################# *
 * #                  End Config Settings                      # *
 * ############################################################# */

/* make sure we uses consistent macros, no matter what the
 * platform gives us.
 */
#undef LOG_NFACILITIES /* may be system defined, override */
#define LOG_NFACILITIES 24+1 /* plus one for our special "invld" facility! */
#define LOG_MAXPRI 191	/* highest supported valid PRI value --> RFC3164, RFC5424 */
#undef LOG_MAKEPRI
#define LOG_PRI_INVLD	(LOG_INVLD|LOG_DEBUG)
/* PRI is invalid --> special "invld.=debug" PRI code (rsyslog-specific) */

#define	LOG_EMERG	0	/* system is unusable */
#define	LOG_ALERT	1	/* action must be taken immediately */
#define	LOG_CRIT	2	/* critical conditions */
#define	LOG_ERR		3	/* error conditions */
#define	LOG_WARNING	4	/* warning conditions */
#define	LOG_NOTICE	5	/* normal but significant condition */
#define	LOG_INFO	6	/* informational */
#define	LOG_DEBUG	7	/* debug-level messages */

#define	LOG_KERN	(0<<3)	/* kernel messages */
#define	LOG_USER	(1<<3)	/* random user-level messages */
#define	LOG_MAIL	(2<<3)	/* mail system */
#define	LOG_DAEMON	(3<<3)	/* system daemons */
#define	LOG_AUTH	(4<<3)	/* security/authorization messages */
#define	LOG_SYSLOG	(5<<3)	/* messages generated internally by syslogd */
#define	LOG_LPR		(6<<3)	/* line printer subsystem */
#define	LOG_NEWS	(7<<3)	/* network news subsystem */
#define	LOG_UUCP	(8<<3)	/* UUCP subsystem */
#define	LOG_CRON	(9<<3)	/* clock daemon */
#define	LOG_AUTHPRIV	(10<<3)	/* security/authorization messages (private) */
#define	LOG_FTP		(11<<3)	/* ftp daemon */
#if defined(_AIX)		/* AIXPORT : These are necessary for AIX */
#define	LOG_ASO		(12<<3) /* Active System Optimizer. Reserved for internal use */
#define	LOG_CAA		(15<<3) /* Cluster aware AIX subsystem */
#endif
#define	LOG_LOCAL0	(16<<3)	/* reserved for local use */
#define	LOG_LOCAL1	(17<<3)	/* reserved for local use */
#define	LOG_LOCAL2	(18<<3)	/* reserved for local use */
#define	LOG_LOCAL3	(19<<3)	/* reserved for local use */
#define	LOG_LOCAL4	(20<<3)	/* reserved for local use */
#define	LOG_LOCAL5	(21<<3)	/* reserved for local use */
#define	LOG_LOCAL6	(22<<3)	/* reserved for local use */
#define	LOG_LOCAL7	(23<<3)	/* reserved for local use */
#define LOG_FAC_INVLD   24
#define	LOG_INVLD	(LOG_FAC_INVLD<<3)	/* invalid facility/PRI code */

/* we need to evaluate our argument only once, as otherwise we may
 * have side-effects (this was seen in some version).
 * Note: I know that "static inline" is not the right thing from a C99
 * PoV, but some environments treat, even in C99 mode, compile
 * non-static inline into the source even if not defined as "extern". This
 * obviously results in linker errors. Using "static inline" as below together
 * with "__attribute__((unused))" works in all cases. Note also that we
 * cannot work around this in pri2fac, as we would otherwise need to evaluate
 * pri more than once.
 */
static inline syslog_pri_t __attribute__((unused))
pri2fac(const syslog_pri_t pri)
{
	unsigned fac = pri >> 3;
	return (fac > 23) ? LOG_FAC_INVLD : fac;
}
#define pri2sev(pri) ((pri) & 0x07)

/* the rsyslog core provides information about present feature to plugins
 * asking it. Below are feature-test macros which must be used to query
 * features. Note that this must be powers of two, so that multiple queries
 * can be combined. -- rgerhards, 2009-04-27
 */
#define CORE_FEATURE_BATCHING	1
/*#define CORE_FEATURE_whatever 2 ... and so on ... */

#ifndef _PATH_CONSOLE
#define _PATH_CONSOLE	"/dev/console"
#endif


/* The error codes below are orginally "borrowed" from
 * liblogging. As such, we reserve values up to -2999
 * just in case we need to borrow something more ;)
*/
enum rsRetVal_				/** return value. All methods return this if not specified otherwise */
{
	/* the first two define are for errmsg.logError(), so that we can use the rsRetVal
	 * as an rsyslog error code. -- rgerhards, 20080-06-27
	 */
	RS_RET_NO_ERRCODE = -1,		/**< RESERVED for NO_ERRCODE errmsg.logError status name */
	RS_RET_INCLUDE_ERRNO = 1073741824, /* 2**30  - do NOT use error codes above this! */
	/* begin regular error codes */
	RS_RET_NOT_IMPLEMENTED = -7,	/**< implementation is missing (probably internal error or lazyness ;)) */
	RS_RET_OUT_OF_MEMORY = -6,	/**< memory allocation failed */
	RS_RET_PROVIDED_BUFFER_TOO_SMALL = -50,
/*< the caller provided a buffer, but the called function sees the size of this buffer is too small -
operation not carried out */
	RS_RET_TRUE = -3,		/**< to indicate a true state (can be used as TRUE, legacy) */
	RS_RET_FALSE = -2,		/**< to indicate a false state (can be used as FALSE, legacy) */
	RS_RET_NO_IRET = -8,	/**< This is a trick for the debuging system - it means no iRet is provided  */
	RS_RET_VALIDATION_RUN = -9,	/**< indicates a (config) validation run, processing not carried out */
	RS_RET_ERR = -3000,	/**< generic failure */
	RS_TRUNCAT_TOO_LARGE = -3001, /**< truncation operation where too many chars should be truncated */
	RS_RET_FOUND_AT_STRING_END = -3002, /**< some value found, but at the last pos of string */
	RS_RET_NOT_FOUND = -3003, /**< some requested value not found */
	RS_RET_MISSING_TRAIL_QUOTE = -3004, /**< an expected trailing quote is missing */
	RS_RET_NO_DIGIT = -3005,	/**< an digit was expected, but none found (mostly parsing) */
	RS_RET_NO_MORE_DATA = -3006,	/**< insufficient data, e.g. end of string during parsing */
	RS_RET_INVALID_IP = -3007,	/**< invalid ip found where valid was expected */
	RS_RET_OBJ_CREATION_FAILED = - 3008, /**< the creation of an object failed (no details available) */
	RS_RET_INOTIFY_INIT_FAILED = - 3009,
	/**< the initialization of an inotify instance failed (no details available) */
	RS_RET_FEN_INIT_FAILED = - 3010, /**< the initialization of a fen instance failed (no details available) */
	RS_RET_PARAM_ERROR = -1000,	/**< invalid parameter in call to function */
	RS_RET_MISSING_INTERFACE = -1001,/**< interface version mismatch, required missing */
	RS_RET_INVALID_CORE_INTERFACE = -1002,/**< interface provided by host invalid, can not be used */
	RS_RET_ENTRY_POINT_NOT_FOUND = -1003,/**< a requested entry point was not found */
	RS_RET_MODULE_ENTRY_POINT_NOT_FOUND = -1004,/**< a entry point requested from a module was not present in it */
	RS_RET_OBJ_NOT_AVAILABLE = -1005,
	/**< something could not be completed because the required object is not available*/
	RS_RET_LOAD_ERROR = -1006,/**< we had an error loading the object/interface and can not continue */
	RS_RET_MODULE_STILL_REFERENCED = -1007,
	/**< module could not be unloaded because it still is referenced by someone */
	RS_RET_OBJ_UNKNOWN = -1008,/**< object is unknown where required */
	RS_RET_OBJ_NOT_REGISTERED = -1009,/**< tried to unregister an object that is not registered */
	/* return states for config file processing */
	RS_RET_NONE = -2000,		/**< some value is not available - not necessarily an error */
	RS_RET_CONFLINE_UNPROCESSED = -2001,/**< config line was not processed, pass to other module */
	RS_RET_DISCARDMSG = -2002,	/**< discard message (no error state, processing request!) */
	RS_RET_INCOMPATIBLE = -2003,	/**< function not compatible with requested feature */
	RS_RET_NOENTRY = -2004,		/**< do not create an entry for (whatever) - not necessary an error */
	RS_RET_NO_SQL_STRING = -2005,	/**< string is not suitable for use as SQL */
	RS_RET_DISABLE_ACTION = -2006,  /**< action requests that it be disabled */
	RS_RET_SUSPENDED = -2007,  /**< something was suspended, not neccesarily an error */
	RS_RET_RQD_TPLOPT_MISSING = -2008,/**< a required template option is missing */
	RS_RET_INVALID_VALUE = -2009,/**< some value is invalid (e.g. user-supplied data) */
	RS_RET_INVALID_INT = -2010,/**< invalid integer */
	RS_RET_INVALID_CMD = -2011,/**< invalid command */
	RS_RET_VAL_OUT_OF_RANGE = -2012, /**< value out of range */
	RS_RET_FOPEN_FAILURE = -2013,	/**< failure during fopen, for example file not found - see errno */
	RS_RET_END_OF_LINKEDLIST = -2014,	/**< end of linked list, not an error, but a status */
	RS_RET_CHAIN_NOT_PERMITTED = -2015, /**< chaining (e.g. of config command handlers) not permitted */
	RS_RET_INVALID_PARAMS = -2016,/**< supplied parameters are invalid */
	RS_RET_EMPTY_LIST = -2017, /**< linked list is empty */
	RS_RET_FINISHED = -2018, /**< some opertion is finished, not an error state */
	RS_RET_INVALID_SOURCE = -2019, /**< source (address) invalid for some reason */
	RS_RET_ADDRESS_UNKNOWN = -2020, /**< an address is unknown - not necessarily an error */
	RS_RET_MALICIOUS_ENTITY = -2021, /**< there is an malicious entity involved */
	RS_RET_NO_KERNEL_LOGSRC = -2022, /**< no source for kernel logs can be obtained */
	RS_RET_TCP_SEND_ERROR = -2023, /**< error during TCP send process */
	RS_RET_GSS_SEND_ERROR = -2024, /**< error during GSS (via TCP) send process */
	RS_RET_TCP_SOCKCREATE_ERR = -2025, /**< error during creation of TCP socket */
	RS_RET_GSS_SENDINIT_ERROR = -2024, /**< error during GSS (via TCP) send initialization process */
	RS_RET_EOF = -2026, /**< end of file reached, not necessarily an error */
	RS_RET_IO_ERROR = -2027, /**< some kind of IO error happened */
	RS_RET_INVALID_OID = -2028, /**< invalid object ID */
	RS_RET_INVALID_HEADER = -2029, /**< invalid header */
	RS_RET_INVALID_HEADER_VERS = -2030, /**< invalid header version */
	RS_RET_INVALID_DELIMITER = -2031, /**< invalid delimiter, e.g. between params */
	RS_RET_INVALID_PROPFRAME = -2032, /**< invalid framing in serialized property */
	RS_RET_NO_PROPLINE = -2033, /**< line is not a property line */
	RS_RET_INVALID_TRAILER = -2034, /**< invalid trailer */
	RS_RET_VALUE_TOO_LOW = -2035, /**< a provided value is too low */
	RS_RET_FILE_PREFIX_MISSING = -2036, /**< a required file prefix (parameter?) is missing */
	RS_RET_INVALID_HEADER_RECTYPE = -2037, /**< invalid record type in header or invalid header */
	RS_RET_QTYPE_MISMATCH = -2038, /**< different qType when reading back a property type */
	RS_RET_NO_FILE_ACCESS = -2039, /**< covers EACCES error on file open() */
	RS_RET_FILE_NOT_FOUND = -2040, /**< file not found */
	RS_RET_TIMED_OUT = -2041, /**< timeout occured (not necessarily an error) */
	RS_RET_QSIZE_ZERO = -2042, /**< queue size is zero where this is not supported */
	RS_RET_ALREADY_STARTING = -2043, /**< something (a thread?) is already starting - not necessarily an error */
	RS_RET_NO_MORE_THREADS = -2044, /**< no more threads available, not necessarily an error */
	RS_RET_NO_FILEPREFIX = -2045, /**< file prefix is not specified where one is needed */
	RS_RET_CONFIG_ERROR = -2046, /**< there is a problem with the user-provided config settigs */
	RS_RET_OUT_OF_DESRIPTORS = -2047, /**< a descriptor table's space has been exhausted */
	RS_RET_NO_DRIVERS = -2048, /**< a required drivers missing */
	RS_RET_NO_DRIVERNAME = -2049, /**< driver name missing where one was required */
	RS_RET_EOS = -2050, /**< end of stream (of whatever) */
	RS_RET_SYNTAX_ERROR = -2051, /**< syntax error, eg. during parsing */
	RS_RET_INVALID_OCTAL_DIGIT = -2052, /**< invalid octal digit during parsing */
	RS_RET_INVALID_HEX_DIGIT = -2053, /**< invalid hex digit during parsing */
	RS_RET_INTERFACE_NOT_SUPPORTED = -2054, /**< interface not supported */
	RS_RET_OUT_OF_STACKSPACE = -2055, /**< a stack data structure is exhausted and can not be grown */
	RS_RET_STACK_EMPTY = -2056, /**< a pop was requested on a stack, but the stack was already empty */
	RS_RET_INVALID_VMOP = -2057, /**< invalid virtual machine instruction */
	RS_RET_INVALID_VAR = -2058, /**< a var or its content is unsuitable, eg. VARTYPE_NONE */
	RS_RET_INVALID_NUMBER = -2059, /**< number invalid during parsing */
	RS_RET_NOT_A_NUMBER = -2060, /**< e.g. conversion impossible because the string is not a number */
	RS_RET_OBJ_ALREADY_REGISTERED = -2061, /**< object (name) is already registered */
	RS_RET_OBJ_REGISTRY_OUT_OF_SPACE = -2062, /**< the object registry has run out of space */
	RS_RET_HOST_NOT_PERMITTED = -2063, /**< a host is not permitted to perform an action it requested */
	RS_RET_MODULE_LOAD_ERR = -2064, /**< module could not be loaded */
	RS_RET_MODULE_LOAD_ERR_PATHLEN = -2065, /**< module could not be loaded - path to long */
	RS_RET_MODULE_LOAD_ERR_DLOPEN = -2066, /**< module could not be loaded - problem in dlopen() */
	RS_RET_MODULE_LOAD_ERR_NO_INIT = -2067, /**< module could not be loaded - init() missing */
	RS_RET_MODULE_LOAD_ERR_INIT_FAILED = -2068, /**< module could not be loaded - init() failed */
	RS_RET_NO_SOCKET = -2069, /**< socket could not be obtained or was not provided */
	RS_RET_SMTP_ERROR = -2070, /**< error during SMTP transation */
	RS_RET_MAIL_NO_TO = -2071, /**< recipient for mail destination is missing */
	RS_RET_MAIL_NO_FROM = -2072, /**< sender for mail destination is missing */
	RS_RET_INVALID_PRI = -2073, /**< PRI value is invalid */
	RS_RET_MALICIOUS_HNAME = -2074, /**< remote peer is trying malicious things with its hostname */
	RS_RET_INVALID_HNAME = -2075, /**< remote peer's hostname invalid or unobtainable */
	RS_RET_INVALID_PORT = -2076, /**< invalid port value */
	RS_RET_COULD_NOT_BIND = -2077, /**< could not bind socket, defunct */
	RS_RET_GNUTLS_ERR = -2078, /**< (unexpected) error in GnuTLS call */
	RS_RET_MAX_SESS_REACHED = -2079, /**< max nbr of sessions reached, can not create more */
	RS_RET_MAX_LSTN_REACHED = -2080, /**< max nbr of listeners reached, can not create more */
	RS_RET_INVALID_DRVR_MODE = -2081, /**< tried to set mode not supported by driver */
	RS_RET_DRVRNAME_TOO_LONG = -2082, /**< driver name too long - should never happen */
	RS_RET_TLS_HANDSHAKE_ERR = -2083, /**< TLS handshake failed */
	RS_RET_TLS_CERT_ERR = -2084, /**< generic TLS certificate error */
	RS_RET_TLS_NO_CERT = -2085, /**< no TLS certificate available where one was expected */
	RS_RET_VALUE_NOT_SUPPORTED = -2086, /**< a provided value is not supported */
	RS_RET_VALUE_NOT_IN_THIS_MODE = -2087, /**< a provided value is invalid for the curret mode */
	RS_RET_INVALID_FINGERPRINT = -2088, /**< a fingerprint is not valid for this use case */
	RS_RET_CONNECTION_ABORTREQ = -2089, /**< connection was abort requested due to previous error */
	RS_RET_CERT_INVALID = -2090, /**< a x509 certificate failed validation */
	RS_RET_CERT_INVALID_DN = -2091, /**< distinguised name in x509 certificate is invalid (e.g. wrong escaping) */
	RS_RET_CERT_EXPIRED = -2092, /**< we are past a x.509 cert's expiration time */
	RS_RET_CERT_NOT_YET_ACTIVE = -2094, /**< x.509 cert's activation time not yet reached */
	RS_RET_SYS_ERR = -2095, /**< system error occured (e.g. time() returned -1, quite unexpected) */
	RS_RET_FILE_NO_STAT = -2096, /**< can not stat() a file */
	RS_RET_FILE_TOO_LARGE = -2097, /**< a file is larger than permitted */
	RS_RET_INVALID_WILDCARD = -2098, /**< a wildcard entry is invalid */
	RS_RET_CLOSED = -2099, /**< connection was closed */
	RS_RET_RETRY = -2100, /**< call should be retried (e.g. EGAIN on recv) */
	RS_RET_GSS_ERR = -2101, /**< generic error occured in GSSAPI subsystem */
	RS_RET_CERTLESS = -2102, /**< state: we run without machine cert (this may be OK) */
	RS_RET_NO_ACTIONS = -2103, /**< no active actions are configured (no output will be created) */
	RS_RET_CONF_FILE_NOT_FOUND = -2104, /**< config file or directory not found */
	RS_RET_QUEUE_FULL = -2105, /**< queue is full, operation could not be completed */
	RS_RET_ACCEPT_ERR = -2106, /**< error during accept() system call */
	RS_RET_INVLD_TIME = -2107, /**< invalid timestamp (e.g. could not be parsed) */
	RS_RET_NO_ZIP = -2108, /**< ZIP functionality is not present */
	RS_RET_CODE_ERR = -2109, /**< program code (internal) error */
	RS_RET_FUNC_NO_LPAREN = -2110, /**< left parenthesis missing after function call (rainerscript) */
	RS_RET_FUNC_MISSING_EXPR = -2111, /**< no expression after comma in function call (rainerscript) */
	RS_RET_INVLD_NBR_ARGUMENTS = -2112, /**< invalid number of arguments for function call (rainerscript) */
	RS_RET_INVLD_FUNC = -2113, /**< invalid function name for function call (rainerscript) */
	RS_RET_DUP_FUNC_NAME = -2114, /**< duplicate function name (rainerscript) */
	RS_RET_UNKNW_FUNC = -2115, /**< unkown function name (rainerscript) */
	RS_RET_ERR_RLIM_NOFILE = -2116, /**< error setting max. nbr open files process limit */
	RS_RET_ERR_CREAT_PIPE = -2117, /**< error during pipe creation */
	RS_RET_ERR_FORK = -2118, /**< error during fork() */
	RS_RET_ERR_WRITE_PIPE = -2119, /**< error writing to pipe */
	RS_RET_RSCORE_TOO_OLD = -2120, /**< rsyslog core is too old for ... (eg this plugin) */
	RS_RET_DEFER_COMMIT = -2121, /**< output plugin status: not yet committed (an OK state!) */
	RS_RET_PREVIOUS_COMMITTED = -2122, /**< output plugin status: previous record was committed (an OK state!) */
	RS_RET_ACTION_FAILED = -2123, /**< action failed and is now suspended */
	RS_RET_NON_SIZELIMITCMD = -2125, /**< size limit for file defined, but no size limit command given */
	RS_RET_SIZELIMITCMD_DIDNT_RESOLVE = -2126, /**< size limit command did not resolve situation */
	RS_RET_STREAM_DISABLED = -2127, /**< a file has been disabled (e.g. by size limit restriction) */
	RS_RET_FILENAME_INVALID = -2140, /**< filename invalid, not found, no access, ... */
	RS_RET_ZLIB_ERR = -2141, /**< error during zlib call */
	RS_RET_VAR_NOT_FOUND = -2142, /**< variable not found */
	RS_RET_EMPTY_MSG = -2143, /**< provided (raw) MSG is empty */
	RS_RET_PEER_CLOSED_CONN = -2144, /**< remote peer closed connection (information, no error) */
	RS_RET_ERR_OPEN_KLOG = -2145, /**< error opening or reading the kernel log socket */
	RS_RET_ERR_AQ_CONLOG = -2146, /**< error aquiring console log (on solaris) */
	RS_RET_ERR_DOOR = -2147, /**< some problems with handling the Solaris door functionality */
	RS_RET_NO_SRCNAME_TPL = -2150, /**< sourcename template was not specified where one was needed
(omudpspoof spoof addr) */
	RS_RET_HOST_NOT_SPECIFIED = -2151, /**< (target) host was not specified where it was needed */
	RS_RET_ERR_LIBNET_INIT = -2152, /**< error initializing libnet, e.g. because not running as root */
	RS_RET_FORCE_TERM = -2153,	/**< thread was forced to terminate by bShallShutdown, a state, not an error */
	RS_RET_RULES_QUEUE_EXISTS = -2154,/**< we were instructed to create a new
					   ruleset queue, but one already exists */
	RS_RET_NO_CURR_RULESET = -2155,/**< no current ruleset exists (but one is required) */
	RS_RET_NO_MSG_PASSING = -2156,
/*< output module interface parameter passing mode "MSG" is not available but required */
	RS_RET_RULESET_NOT_FOUND = -2157,/**< a required ruleset could not be found */
	RS_RET_NO_RULESET= -2158,/**< no ruleset name as specified where one was needed */
	RS_RET_PARSER_NOT_FOUND = -2159,/**< parser with the specified name was not found */
	RS_RET_COULD_NOT_PARSE = -2160,/**< (this) parser could not parse the message (no error, means try next one) */
	RS_RET_EINTR = -2161,		/**< EINTR occured during a system call (not necessarily an error) */
	RS_RET_ERR_EPOLL = -2162,	/**< epoll() returned with an unexpected error code */
	RS_RET_ERR_EPOLL_CTL = -2163,	/**< epol_ctll() returned with an unexpected error code */
	RS_RET_TIMEOUT = -2164,		/**< timeout occured during operation */
	RS_RET_RCV_ERR = -2165,		/**< error occured during socket rcv operation */
	RS_RET_NO_SOCK_CONFIGURED = -2166, /**< no socket (name) was configured where one is required */
	RS_RET_CONF_NOT_GLBL = -2167,	/**< $Begin not in global scope */
	RS_RET_CONF_IN_GLBL = -2168,	/**< $End when in global scope */
	RS_RET_CONF_INVLD_END = -2169,	/**< $End for wrong conf object (probably nesting error) */
	RS_RET_CONF_INVLD_SCOPE = -2170,
/*< config statement not valid in current scope (e.g. global stmt in action block) */
	RS_RET_CONF_END_NO_ACT = -2171,	/**< end of action block, but no actual action specified */
	RS_RET_NO_LSTN_DEFINED = -2172, /**< no listener defined (e.g. inside an input module) */
	RS_RET_EPOLL_CR_FAILED = -2173, /**< epoll_create() failed */
	RS_RET_EPOLL_CTL_FAILED = -2174, /**< epoll_ctl() failed */
	RS_RET_INTERNAL_ERROR = -2175, /**< rsyslogd internal error, unexpected code path reached */
	RS_RET_ERR_CRE_AFUX = -2176, /**< error creating AF_UNIX socket (and binding it) */
	RS_RET_RATE_LIMITED = -2177, /**< some messages discarded due to exceeding a rate limit */
	RS_RET_ERR_HDFS_WRITE = -2178, /**< error writing to HDFS */
	RS_RET_ERR_HDFS_OPEN = -2179, /**< error during hdfsOpen (e.g. file does not exist) */
	RS_RET_FILE_NOT_SPECIFIED = -2180, /**< file name not configured where this was required */
	RS_RET_ERR_WRKDIR = -2181, /**< problems with the rsyslog working directory */
	RS_RET_WRN_WRKDIR = -2182, /**< correctable problems with the rsyslog working directory */
	RS_RET_ERR_QUEUE_EMERGENCY = -2183, /**<  some fatal error caused queue to switch to emergency mode */
	RS_RET_OUTDATED_STMT = -2184, /**<  some outdated statement/functionality is being used in conf file */
	RS_RET_MISSING_WHITESPACE = -2185, /**<  whitespace is missing in some config construct */
	RS_RET_OK_WARN = -2186, /**<  config part: everything was OK, but a warning message was emitted */

	RS_RET_INVLD_CONF_OBJ= -2200,	/**< invalid config object (e.g. $Begin conf statement) */
	RS_RET_ERR_LIBEE_INIT = -2201,	/**< cannot obtain libee ctx */
	RS_RET_ERR_LIBLOGNORM_INIT = -2202,/**< cannot obtain liblognorm ctx */
	RS_RET_ERR_LIBLOGNORM_SAMPDB_LOAD = -2203,/**< liblognorm sampledb load failed */
	RS_RET_CMD_GONE_AWAY = -2204,/**< config directive existed, but no longer supported */
	RS_RET_ERR_SCHED_PARAMS = -2205,/**< there is a problem with configured thread scheduling params */
	RS_RET_SOCKNAME_MISSING = -2206,/**< no socket name configured where one is required */
	RS_RET_CONF_PARSE_ERROR = -2207,/**< (fatal) error parsing config file */
	RS_RET_CONF_RQRD_PARAM_MISSING = -2208,/**< required parameter in config object is missing */
	RS_RET_MOD_UNKNOWN = -2209,/**< module (config name) is unknown */
	RS_RET_CONFOBJ_UNSUPPORTED = -2210,/**< config objects (v6 conf) are not supported here */
	RS_RET_MISSING_CNFPARAMS = -2211, /**< missing configuration parameters */
	RS_RET_NO_LISTNERS = -2212, /**< module loaded, but no listeners are defined */
	RS_RET_INVLD_PROTOCOL = -2213, /**< invalid protocol specified in config file */
	RS_RET_CNF_INVLD_FRAMING = -2214, /**< invalid framing specified in config file */
	RS_RET_LEGA_ACT_NOT_SUPPORTED = -2215, /**< the module (no longer) supports legacy action syntax */
	RS_RET_MAX_OMSR_REACHED = -2216, /**< max nbr of string requests reached, not supported by core */
	RS_RET_UID_MISSING = -2217,	/**< a user id is missing (but e.g. a password provided) */
	RS_RET_DATAFAIL = -2218,	/**< data passed to action caused failure */
	/* reserved for pre-v6.5 */
	RS_RET_DUP_PARAM = -2220, /**< config parameter is given more than once */
	RS_RET_MODULE_ALREADY_IN_CONF = -2221, /**< module already in current configuration */
	RS_RET_PARAM_NOT_PERMITTED = -2222, /**< legacy parameter no longer permitted (usally already set by v2) */
	RS_RET_NO_JSON_PASSING = -2223, /**< rsyslog core does not support JSON-passing plugin API */
	RS_RET_MOD_NO_INPUT_STMT = -2224, /**< (input) module does not support input() statement */
	RS_RET_NO_CEE_MSG = -2225, /**< the message being processed is NOT CEE-enhanced */

	/**** up to 2290 is reserved for v6 use ****/
	RS_RET_RELP_ERR = -2291,	/**<< error in RELP processing */
	/**** up to 3000 is reserved for c7 use ****/
	RS_RET_JNAME_NO_ROOT = -2301, /**< root element is missing in JSON path */
	RS_RET_JNAME_INVALID = -2302, /**< JSON path is invalid */
	RS_RET_JSON_PARSE_ERR = -2303, /**< we had a problem parsing JSON (or extra data) */
	RS_RET_BSD_BLOCKS_UNSUPPORTED = -2304, /**< BSD-style config blocks are no longer supported */
	RS_RET_JNAME_NOTFOUND = -2305, /**< JSON name not found (does not exist) */
	RS_RET_INVLD_SETOP = -2305, /**< invalid variable set operation, incompatible type */
	RS_RET_RULESET_EXISTS = -2306,/**< ruleset already exists */
	RS_RET_DEPRECATED = -2307,/**< deprecated functionality is used */
	RS_RET_DS_PROP_SEQ_ERR = -2308,/**< property sequence error deserializing object */
	RS_RET_INVLD_PROP = -2309,/**< property name error (unknown name) */
	RS_RET_NO_RULEBASE = -2310,/**< mmnormalize: rulebase can not be found or otherwise invalid */
	RS_RET_INVLD_MODE = -2311,/**< invalid mode specified in configuration */
	RS_RET_INVLD_ANON_BITS = -2312,/**< mmanon: invalid number of bits to anonymize specified */
	RS_RET_REPLCHAR_IGNORED = -2313,/**< mmanon: replacementChar parameter is ignored */
	RS_RET_SIGPROV_ERR = -2320,/**< error in signature provider */
	RS_RET_CRYPROV_ERR = -2321,/**< error in cryptography encryption provider */
	RS_RET_EI_OPN_ERR = -2322,/**< error opening an .encinfo file */
	RS_RET_EI_NO_EXISTS = -2323,/**< .encinfo file does not exist (status, not necessarily error!)*/
	RS_RET_EI_WR_ERR = -2324,/**< error writing an .encinfo file */
	RS_RET_EI_INVLD_FILE = -2325,/**< header indicates the file is no .encinfo file */
	RS_RET_CRY_INVLD_ALGO = -2326,/**< user specified invalid (unkonwn) crypto algorithm */
	RS_RET_CRY_INVLD_MODE = -2327,/**< user specified invalid (unkonwn) crypto mode */
	RS_RET_QUEUE_DISK_NO_FN = -2328,/**< disk queue configured, but filename not set */
	RS_RET_CA_CERT_MISSING = -2329,/**< a CA cert is missing where one is required (e.g. TLS) */
	RS_RET_CERT_MISSING = -2330,/**< a cert is missing where one is required (e.g. TLS) */
	RS_RET_CERTKEY_MISSING = -2331,/**< a cert (private) key is missing where one is required (e.g. TLS) */
	RS_RET_STRUC_DATA_INVLD = -2349,/**< structured data is malformed */

	/* up to 2350 reserved for 7.4 */
	RS_RET_QUEUE_CRY_DISK_ONLY = -2351,/**< crypto provider only supported for disk-associated queues */
	RS_RET_NO_DATA = -2352,/**< file has no data; more a state than a real error */
	RS_RET_RELP_AUTH_FAIL = -2353,/**< RELP peer authentication failed */
	RS_RET_ERR_UDPSEND = -2354,/**< sending msg via UDP failed */
	RS_RET_LAST_ERRREPORT = -2355,/**< module does not emit more error messages as limit is reached */
	RS_RET_READ_ERR = -2356,/**< read error occured (file i/o) */
	RS_RET_CONF_PARSE_WARNING = -2357,/**< warning parsing config file */
	RS_RET_CONF_WRN_FULLDLY_BELOW_HIGHWTR = -2358,/**< warning queue full delay mark below high wtr mark */
	RS_RET_RESUMED = -2359,/**< status: action was resumed (used for reporting) */
	RS_RET_RELP_NO_TLS = -2360,/**< librel does not support TLS (but TLS requested) */
	RS_RET_STATEFILE_WRONG_FNAME  = -2361,/**< state file is for wrong file */
	RS_RET_NAME_INVALID = -2362, /**< invalid name (in RainerScript) */

	/* up to 2400 reserved for 7.5 & 7.6 */
	RS_RET_INVLD_OMOD = -2400, /**< invalid output module, does not provide proper interfaces */
	RS_RET_INVLD_INTERFACE_INPUT = -2401, /**< invalid value for "interface.input" parameter (ext progs) */
	RS_RET_PARSER_NAME_EXISTS = -2402, /**< parser name already exists */
	RS_RET_MOD_NO_PARSER_STMT = -2403, /**< (parser) module does not support parser() statement */

	/* up to 2419 reserved for 8.4.x */
	RS_RET_IMFILE_WILDCARD = -2420, /**< imfile file name contains wildcard, which may be problematic */
	RS_RET_RELP_NO_TLS_AUTH = -2421,/**< librel does not support TLS authentication (but was requested) */
	RS_RET_KAFKA_ERROR = -2422,/**< error reported by Apache Kafka subsystem. See message for details. */
	RS_RET_KAFKA_NO_VALID_BROKERS = -2423,/**< no valid Kafka brokers configured/available */
	RS_RET_KAFKA_PRODUCE_ERR = -2424,/**< error during Kafka produce function */
	RS_RET_CONF_PARAM_INVLD = -2425,/**< config parameter is invalid */
	RS_RET_KSI_ERR = -2426,/**< error in KSI subsystem */
	RS_RET_ERR_LIBLOGNORM = -2427,/**< cannot obtain liblognorm ctx */
	RS_RET_CONC_CTRL_ERR = -2428,/**< error in lock/unlock/condition/concurrent-modification operation */
	RS_RET_SENDER_GONE_AWAY = -2429,/**< warning: sender not seen for configured amount of time */
	RS_RET_SENDER_APPEARED = -2430,/**< info: new sender appeared */
	RS_RET_FILE_ALREADY_IN_TABLE = -2431,/**< in imfile: table already contains to be added file */
	RS_RET_ERR_DROP_PRIV = -2432,/**< error droping privileges */
	RS_RET_FILE_OPEN_ERROR = -2433, /**< error other than "not found" occured during open() */
	RS_RET_RENAME_TMP_QI_ERROR = -2435, /**< renaming temporary .qi file failed */
	RS_RET_ERR_SETENV = -2436, /**< error setting an environment variable */
	RS_RET_DIR_CHOWN_ERROR = -2437, /**< error during chown() */
	RS_RET_JSON_UNUSABLE = -2438, /**< JSON object is NULL or otherwise unusable */
	RS_RET_OPERATION_STATUS = -2439, /**< operational status (info) message, no error */
	RS_RET_UDP_MSGSIZE_TOO_LARGE = -2440, /**< a message is too large to be sent via UDP */
	RS_RET_NON_JSON_PROP = -2441, /**< a non-json property id is provided where a json one is requried */
	RS_RET_NO_TZ_SET = -2442, /**< system env var TZ is not set (status msg) */
	RS_RET_FS_ERR = -2443, /**< file-system error */
	RS_RET_POLL_ERR = -2444, /**< error in poll() system call */
	RS_RET_OVERSIZE_MSG = -2445, /**< message is too long (above configured max) */

	/* RainerScript error messages (range 1000.. 1999) */
	RS_RET_SYSVAR_NOT_FOUND = 1001, /**< system variable could not be found (maybe misspelled) */
	RS_RET_FIELD_NOT_FOUND = 1002, /**< field() function did not find requested field */

	/* some generic error/status codes */
	RS_RET_OK = 0,			/**< operation successful */
	RS_RET_OK_DELETE_LISTENTRY = 1,
/*< operation successful, but callee requested the deletion of an entry (special state) */
	RS_RET_TERMINATE_NOW = 2,	/**< operation successful, function is requested to terminate
					(mostly used with threads) */
	RS_RET_NO_RUN = 3,		/**< operation successful, but function does not like to be executed */
	RS_RET_IDLE = 4,		/**< operation successful, but callee is idle (e.g. because queue is empty) */
	RS_RET_TERMINATE_WHEN_IDLE = 5	/**< operation successful, function is requested to terminate when idle */
};

/* some helpful macros to work with srRetVals.
 * Be sure to call the to-be-returned variable always "iRet" and
 * the function finalizer always "finalize_it".
 */
#ifdef HAVE_BUILTIN_EXCEPT
#	define CHKiRet(code) if(__builtin_expect(((iRet = code) != RS_RET_OK), 0)) goto finalize_it
#else
#	define CHKiRet(code) if((iRet = code) != RS_RET_OK) goto finalize_it
#endif

# define CHKiConcCtrl(code)  { int tmp_CC; \
	if ((tmp_CC = code) != 0) { \
		iRet = RS_RET_CONC_CTRL_ERR; \
		errno = tmp_CC; \
		goto finalize_it; \
	} \
}

/* macro below is to be used if we need our own handling, eg for cleanup */
#define CHKiRet_Hdlr(code) if((iRet = code) != RS_RET_OK)
/* macro below is to handle failing malloc/calloc/strdup... which we almost always handle in the same way... */
#define CHKmalloc(operation) if((operation) == NULL) ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY)
/* macro below is used in conjunction with CHKiRet_Hdlr, else use ABORT_FINALIZE */
#define FINALIZE goto finalize_it;
#define DEFiRet BEGINfunc rsRetVal iRet = RS_RET_OK
#define RETiRet do{ ENDfuncIRet return iRet; }while(0)

#define ABORT_FINALIZE(errCode)			\
	do {					\
		iRet = errCode;			\
		goto finalize_it;		\
	} while (0)

/** Object ID. These are for internal checking. Each
 * object is assigned a specific ID. This is contained in
 * all Object structs (just like C++ RTTI). We can use
 * this field to see if we have been passed a correct ID.
 * Other than that, there is currently no other use for
 * the object id.
 */
enum rsObjectID
{
	OIDrsFreed = -1,		/**< assigned, when an object is freed. If this
				 *   is seen during a method call, this is an
				 *   invalid object pointer!
				 */
	OIDrsInvalid = 0,	/**< value created by calloc(), so do not use ;) */
	/* The 0x3412 is a debug aid. It helps us find object IDs in memory
	 * dumps (on X86, this is 1234 in the dump ;)
	 * If you are on an embedded device and you would like to save space
	 * make them 1 byte only.
	 */
	OIDrsCStr = 0x34120001,
	OIDrsPars = 0x34120002
};
typedef enum rsObjectID rsObjID;

/* support to set object types */
#ifdef NDEBUG
#define rsSETOBJTYPE(pObj, type)
#define rsCHECKVALIDOBJECT(x, type)
#else
#define rsSETOBJTYPE(pObj, type) pObj->OID = type;
#define rsCHECKVALIDOBJECT(x, type) {assert(x != NULL); assert(x->OID == type);}
#endif

/**
 * This macro should be used to free objects.
 * It aids in interpreting dumps during debugging.
 */
#ifdef NDEBUG
#define RSFREEOBJ(x) free(x)
#else
#define RSFREEOBJ(x) {(x)->OID = OIDrsFreed; free(x);}
#endif

extern pthread_attr_t default_thread_attr;
#ifdef HAVE_PTHREAD_SETSCHEDPARAM
extern struct sched_param default_sched_param;
extern int default_thr_sched_policy;
#endif

/* The following structure defines immutable parameters which need to
 * be passed as action parameters.
 *
 * Note that output plugins may request multiple templates. Let's say
 * an output requests n templates. Than the overall table must hold
 * n*nbrMsgs records, and each messages begins on a n-boundary. There
 * is a macro defined below to access the proper element.
 *
 * WARNING: THIS STRUCTURE IS PART OF THE ***OUTPUT MODULE INTERFACE***
 * It is passed into the doCommit() function. Do NOT modify it until
 * absolutely necessary - all output plugins need to be changed!
 *
 * If a change is "just" for internal working, consider adding a
 * separate parameter outside of this structure. Of course, it is
 * best to avoid this as well ;-)
 * rgerhards, 2013-12-04
 */
struct actWrkrIParams {
	uchar *param;
	uint32_t lenBuf;  /* length of string buffer (if string ptr) */
	uint32_t lenStr;  /* length of current string (if string ptr) */
};

/* macro to access actWrkrIParams base object:
 * param is ptr to base address
 * nActTpls is the number of templates the action has requested
 * iMsg is the message index
 * iTpl is the template index
 * This macro can be used for read and write access.
 */
#define actParam(param, nActTpls, iMsg, iTpl) (param[(iMsg*nActTpls)+iTpl])

/* for the time being, we do our own portability handling here. It
 * looks like autotools either does not yet support checks for it, or
 * I wasn't smart enough to find them ;) rgerhards, 2007-07-18
 */
#ifndef __GNUC__
#  define  __attribute__(x)  /*NOTHING*/
#endif

#ifndef O_CLOEXEC
/* of course, this limits the functionality... */
#  define O_CLOEXEC 0
#endif

/* some constants */
#define MUTEX_ALREADY_LOCKED	0
#define LOCK_MUTEX		1

/* The following prototype is convenient, even though it may not be the 100%
correct place.. -- rgerhards 2008-01-07 */
//void dbgprintf(const char *, ...) __attribute__((format(printf, 1, 2)));


#include "debug.h"
#include "obj.h"

/* the variable below is a trick: before we can init the runtime, the caller
 * may want to set a module load path. We can not do this via the glbl class
 * because it needs an initialized runtime system (and may at some point in time
 * even be loaded itself). So this is a no-go. What we do is use a single global
 * variable which may be provided with a pointer by the caller. This variable
 * resides in rsyslog.c, the main runtime file. We have not seen any realy valule
 * in providing object access functions. If you don't like that, feel free to
 * add them. -- rgerhards, 2008-04-17
 */
extern uchar *glblModPath; /* module load path */
extern void (*glblErrLogger)(const int, const int, const uchar*);

/* some runtime prototypes */
void processImInternal(void);
rsRetVal rsrtInit(const char **ppErrObj, obj_if_t *pObjIF);
rsRetVal rsrtExit(void);
int rsrtIsInit(void);
void rsrtSetErrLogger(void (*errLogger)(const int, const int, const uchar*));

void dfltErrLogger(const int, const int, const uchar *errMsg);


/* this define below is (later) intended to be used to implement empty
 * structs. TODO: check if compilers supports this and, if not, define
 * a dummy variable. This requires review of where in code empty structs
 * are already defined. -- rgerhards, 2010-07-26
 */
#ifdef OS_SOLARIS
#define EMPTY_STRUCT  int remove_me_when_first_real_member_is_added;
#else
#define EMPTY_STRUCT
#endif

/* TODO: remove this -- this is only for transition of the config system */
extern rsconf_t *ourConf; /* defined by syslogd.c, a hack for functions that do not
			     yet receive a copy, so that we can incrementially
			     compile and change... -- rgerhars, 2011-04-19 */


/* here we add some stuff from the compatibility layer. A separate include
 * would be cleaner, but would potentially require changes all over the
 * place. So doing it here is better. The respective replacement
 * functions should usually be found under ./compat -- rgerhards, 2015-05-20
 */
#ifndef HAVE_STRNDUP
char * strndup(const char *s, size_t n);
#endif

#endif /* multi-include protection */
