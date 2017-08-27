/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */
#ifndef FR_LOG_H
#define FR_LOG_H
/*
 * $Id$
 *
 * @file log.h
 * @brief Structures and prototypes for logging.
 *
 * @copyright 2013 Alan DeKok <aland@freeradius.org>
 */
RCSIDH(log_h, "$Id$")

#ifdef __cplusplus
extern "C" {
#endif

typedef enum log_type {
	L_AUTH = 2,		//!< Authentication message.
	L_INFO = 3,		//!< Informational message.
	L_ERR = 4,		//!< Error message.
	L_WARN = 5,		//!< Warning.
	L_PROXY	= 6,		//!< Proxy messages
	L_ACCT = 7,		//!< Accounting messages

	L_DBG = 16,		//!< Only displayed when debugging is enabled.
	L_DBG_WARN = 17,	//!< Warning only displayed when debugging is enabled.
	L_DBG_ERR = 18,		//!< Error only displayed when debugging is enabled.
	L_DBG_WARN2 = 19,	//!< Less severe warning only displayed when debugging is enabled.
	L_DBG_ERR2 = 20		//!< Less severe error only displayed when debugging is enabled.
} log_type_t;

typedef enum log_debug {
	L_DBG_LVL_MIN = -1,	//!< Hack for stupid GCC warnings (comparison with 0 always true)
	L_DBG_LVL_OFF = 0,	//!< No debug messages.
	L_DBG_LVL_1,		//!< Highest priority debug messages (-x).
	L_DBG_LVL_2,		//!< 2nd highest priority debug messages (-xx | -X).
	L_DBG_LVL_3,		//!< 3rd highest priority debug messages (-xxx | -Xx).
	L_DBG_LVL_MAX		//!< Lowest priority debug messages (-xxxx | -Xxx).
} log_debug_t;

typedef enum log_dst {
	L_DST_STDOUT = 0,	//!< Log to stdout.
	L_DST_FILES,		//!< Log to a file on disk.
	L_DST_SYSLOG,		//!< Log to syslog.
	L_DST_STDERR,		//!< Log to stderr.
	L_DST_NULL,		//!< Discard log messages.
	L_DST_NUM_DEST
} log_dst_t;

typedef struct fr_log_t {
	int		colourise;	//!< Prefix log messages with VT100 escape codes to change text
					//!< colour.
	int		fd;		//!< File descriptor to write messages to.
	log_dst_t	dest;		//!< Log destination.
	char		*file;		//!< Path to log file.
	char		*debug_file;	//!< Path to debug log file.
} fr_log_t;

typedef		void (*radlog_func_t)(log_type_t lvl, log_debug_t priority, REQUEST *, char const *, ...);

extern FR_NAME_NUMBER const syslog_str2fac[];
extern FR_NAME_NUMBER const log_str2dst[];
extern fr_log_t default_log;

int		vradlog(log_type_t lvl, char const *fmt, va_list ap);
int		radlog(log_type_t lvl, char const *fmt, ...)
#ifdef __GNUC__
		__attribute__ ((format (printf, 2, 3)))
#endif
;
void 		vp_listdebug(VALUE_PAIR *vp);
bool		radlog_debug_enabled(log_type_t type, log_debug_t lvl, REQUEST *request);
void		radlog_request(log_type_t lvl, log_debug_t priority, REQUEST *request, char const *msg, ...)
#ifdef __GNUC__
		__attribute__ ((format (printf, 4, 5)))
#endif
;

void log_talloc(char const *message);
void log_talloc_report(TALLOC_CTX *ctx);

/*
 *	Logging macros.
 *
 *	For server code, do not call radlog, vradlog et al directly, use one of the logging macros instead.
 *
 *	R*			- Macros prefixed with an R will automatically prepend request information to the
 *				  log messages.
 *	INFO | WARN | ERROR	- Macros containing these words will be displayed at all log levels.
 *	*DEBUG* 		- Macros with the word DEBUG, will only be displayed if the server or request debug
 *				  level is above 0.
 *	*[IWE]DEBUG[0-9]?	- Macros with I, W, E as (or just after) the prefix, will log with the priority
 *				  specified by the integer if the server or request log level at or above that integer.
 *				  If there is no integer the level is 1. The I|W|E prefix determines the type
 *				  (INFO, WARN, ERROR), if there is no I|W|E prefix the DEBUG type will be used.
 */

/*
 *	Log server driven messages like threadpool exhaustion and connection failures
 */
#define _SL(_l, _p, _f, ...)	if (debug_flag >= _p) radlog(_l, _f, ## __VA_ARGS__)

#define DEBUG_ENABLED		radlog_debug_enabled(L_DBG, L_DBG_LVL_1, NULL)
#define DEBUG_ENABLED2		radlog_debug_enabled(L_DBG, L_DBG_LVL_2, NULL)
#define DEBUG_ENABLED3		radlog_debug_enabled(L_DBG, L_DBG_LVL_3, NULL)
#define DEBUG_ENABLED4		radlog_debug_enabled(L_DBG, L_DBG_LVL_MAX, NULL)

#define AUTH(fmt, ...)		_SL(L_AUTH, L_DBG_LVL_OFF, fmt, ## __VA_ARGS__)
#define ACCT(fmt, ...)		_SL(L_ACCT, L_DBG_LVL_OFF, fmt, ## __VA_ARGS__)
#define PROXY(fmt, ...)		_SL(L_PROXY, L_DBG_LVL_OFF, fmt, ## __VA_ARGS__)

#define DEBUG(fmt, ...)		_SL(L_DBG, L_DBG_LVL_1, fmt, ## __VA_ARGS__)
#define DEBUG2(fmt, ...)	_SL(L_DBG, L_DBG_LVL_2, fmt, ## __VA_ARGS__)
#define DEBUG3(fmt, ...)	_SL(L_DBG, L_DBG_LVL_3, fmt, ## __VA_ARGS__)
#define DEBUG4(fmt, ...)	_SL(L_DBG, L_DBG_LVL_MAX, fmt, ## __VA_ARGS__)

#define INFO(fmt, ...)		_SL(L_INFO, L_DBG_LVL_OFF, fmt, ## __VA_ARGS__)
#define DEBUGI(fmt, ...)	_SL(L_INFO, L_DBG_LVL_1, fmt, ## __VA_ARGS__)

#define WARN(fmt, ...)		_SL(L_WARN, L_DBG_LVL_OFF, fmt, ## __VA_ARGS__)
#define WDEBUG(fmt, ...)	_SL(L_DBG_WARN, L_DBG_LVL_1, fmt, ## __VA_ARGS__)
#define WDEBUG2(fmt, ...)	_SL(L_DBG_WARN, L_DBG_LVL_2, fmt, ## __VA_ARGS__)

#define ERROR(fmt, ...)		_SL(L_ERR, L_DBG_LVL_OFF, fmt, ## __VA_ARGS__)
#define EDEBUG(fmt, ...)	_SL(L_DBG_ERR, L_DBG_LVL_1, fmt, ## __VA_ARGS__)
#define EDEBUG2(fmt, ...)	_SL(L_DBG_ERR, L_DBG_LVL_2, fmt, ## __VA_ARGS__)

/*
 *	Log request driven messages which including elements from the current request, like section and module
 *
 *	If a REQUEST * is available, these functions should be used.
 */
#define _RL(_l, _p, _f, ...)	if (request && request->radlog) request->radlog(_l, _p, request, _f, ## __VA_ARGS__)
#define _RM(_l, _p, _f, ...)	do { \
					if(request) { \
						module_failure_msg(request, _f, ## __VA_ARGS__); \
						_RL(_l, _p, _f, ## __VA_ARGS__); \
					} \
				} while(0)

#define RDEBUG_ENABLED		radlog_debug_enabled(L_DBG, L_DBG_LVL_1, request)
#define RDEBUG_ENABLED2		radlog_debug_enabled(L_DBG, L_DBG_LVL_2, request)
#define RDEBUG_ENABLED3		radlog_debug_enabled(L_DBG, L_DBG_LVL_3, request)
#define RDEBUG_ENABLED4		radlog_debug_enabled(L_DBG, L_DBG_LVL_MAX, request)

#define RAUTH(fmt, ...)		_RL(L_AUTH, L_DBG_LVL_OFF, fmt, ## __VA_ARGS__)
#define RACCT(fmt, ...)		_RL(L_ACCT, L_DBG_LVL_OFF, fmt, ## __VA_ARGS__)
#define RPROXY(fmt, ...)	_RL(L_PROXY, L_DBG_LVL_OFF, fmt, ## __VA_ARGS__)

#define RDEBUG(fmt, ...)	_RL(L_DBG, L_DBG_LVL_1, fmt, ## __VA_ARGS__)
#define RDEBUG2(fmt, ...)	_RL(L_DBG, L_DBG_LVL_2, fmt, ## __VA_ARGS__)
#define RDEBUG3(fmt, ...)	_RL(L_DBG, L_DBG_LVL_3, fmt, ## __VA_ARGS__)
#define RDEBUG4(fmt, ...)	_RL(L_DBG, L_DBG_LVL_MAX, fmt, ## __VA_ARGS__)

#define RINFO(fmt, ...)		_RL(L_INFO, L_DBG_LVL_OFF, fmt, ## __VA_ARGS__)
#define RIDEBUG(fmt, ...)	_RL(L_INFO, L_DBG_LVL_1, fmt, ## __VA_ARGS__)
#define RIDEBUG2(fmt, ...)	_RL(L_INFO, L_DBG_LVL_2, fmt, ## __VA_ARGS__)

#define RWARN(fmt, ...)		_RL(L_DBG_WARN, L_DBG_LVL_OFF, fmt, ## __VA_ARGS__)
#define RWDEBUG(fmt, ...)	_RL(L_DBG_WARN, L_DBG_LVL_1, fmt, ## __VA_ARGS__)
#define RWDEBUG2(fmt, ...)	_RL(L_DBG_WARN, L_DBG_LVL_2, fmt, ## __VA_ARGS__)

#define RERROR(fmt, ...)	_RM(L_DBG_ERR, L_DBG_LVL_OFF, fmt, ## __VA_ARGS__)
#define REDEBUG(fmt, ...)	_RM(L_DBG_ERR, L_DBG_LVL_1, fmt, ## __VA_ARGS__)
#define REDEBUG2(fmt, ...)	_RM(L_DBG_ERR, L_DBG_LVL_2, fmt, ## __VA_ARGS__)
#define REDEBUG3(fmt, ...)	_RM(L_DBG_ERR, L_DBG_LVL_3, fmt, ## __VA_ARGS__)
#define REDEBUG4(fmt, ...)	_RM(L_DBG_ERR, L_DBG_LVL_MAX, fmt, ## __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* FR_LOG_H */
