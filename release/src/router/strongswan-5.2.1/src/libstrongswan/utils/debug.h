/*
 * Copyright (C) 2006 Martin Willi
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup debug debug
 * @{ @ingroup utils
 */

#ifndef DEBUG_H_
#define DEBUG_H_

typedef enum debug_t debug_t;
typedef enum level_t level_t;

#include <stdio.h>

#include <utils/utils.h>

/**
 * Debug message group.
 */
enum debug_t {
	/** daemon specific */
	DBG_DMN,
	/** IKE_SA_MANAGER */
	DBG_MGR,
	/** IKE_SA */
	DBG_IKE,
	/** CHILD_SA */
	DBG_CHD,
	/** job processing */
	DBG_JOB,
	/** configuration backends */
	DBG_CFG,
	/** kernel interface */
	DBG_KNL,
	/** networking/sockets */
	DBG_NET,
	/** low-level encoding/decoding (ASN.1, X.509 etc.) */
	DBG_ASN,
	/** message encoding/decoding */
	DBG_ENC,
	/** trusted network connect */
	DBG_TNC,
	/** integrity measurement client */
	DBG_IMC,
	/** integrity measurement verifier */
	DBG_IMV,
	/** platform trust service */
	DBG_PTS,
	/** libtls */
	DBG_TLS,
	/** applications other than daemons */
	DBG_APP,
	/** libipsec */
	DBG_ESP,
	/** libstrongswan */
	DBG_LIB,
	/** number of groups */
	DBG_MAX,
	/** pseudo group with all groups */
	DBG_ANY = DBG_MAX,
};

/**
 * short names of debug message group.
 */
extern enum_name_t *debug_names;

/**
 * short names of debug message group, lower case.
 */
extern enum_name_t *debug_lower_names;

/**
 * Debug levels used to control output verbosity.
 */
enum level_t {
	/** absolutely silent */
	LEVEL_SILENT = -1,
	/** most important auditing logs */
	LEVEL_AUDIT =   0,
	/** control flow */
	LEVEL_CTRL =    1,
	/** diagnose problems */
	LEVEL_DIAG =    2,
	/** raw binary blobs */
	LEVEL_RAW =     3,
	/** including sensitive data (private keys) */
	LEVEL_PRIVATE = 4,
};

#ifndef DEBUG_LEVEL
# define DEBUG_LEVEL 4
#endif /* DEBUG_LEVEL */

/** debug macros, they call the dbg function hook */
#if DEBUG_LEVEL >= 0
# define DBG0(group, fmt, ...) dbg(group, 0, fmt, ##__VA_ARGS__)
#endif /* DEBUG_LEVEL */
#if DEBUG_LEVEL >= 1
# define DBG1(group, fmt, ...) dbg(group, 1, fmt, ##__VA_ARGS__)
#endif /* DEBUG_LEVEL */
#if DEBUG_LEVEL >= 2
# define DBG2(group, fmt, ...) dbg(group, 2, fmt, ##__VA_ARGS__)
#endif /* DEBUG_LEVEL */
#if DEBUG_LEVEL >= 3
# define DBG3(group, fmt, ...) dbg(group, 3, fmt, ##__VA_ARGS__)
#endif /* DEBUG_LEVEL */
#if DEBUG_LEVEL >= 4
# define DBG4(group, fmt, ...) dbg(group, 4, fmt, ##__VA_ARGS__)
#endif /* DEBUG_LEVEL */

#ifndef DBG0
# define DBG0(...) {}
#endif
#ifndef DBG1
# define DBG1(...) {}
#endif
#ifndef DBG2
# define DBG2(...) {}
#endif
#ifndef DBG3
# define DBG3(...) {}
#endif
#ifndef DBG4
# define DBG4(...) {}
#endif

/** dbg function hook, uses dbg_default() by default */
extern void (*dbg) (debug_t group, level_t level, char *fmt, ...);

/** default logging function */
void dbg_default(debug_t group, level_t level, char *fmt, ...);

/** set the level logged by dbg_default() */
void dbg_default_set_level(level_t level);

/** set the stream logged by dbg_default() to */
void dbg_default_set_stream(FILE *stream);

#endif /** DEBUG_H_ @}*/
