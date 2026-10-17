/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Logging support
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __LOG_H
#define __LOG_H

#include <stdio.h>
#include <linker_lists.h>
#include <dm/uclass-id.h>
#include <linux/bitops.h>
#include <linux/list.h>

struct cmd_tbl;

/**
 * enum log_level_t - Log levels supported, ranging from most to least important
 */
enum log_level_t {
	/** @LOGL_EMERG: U-Boot is unstable */
	LOGL_EMERG = 0,
	/** @LOGL_ALERT: Action must be taken immediately */
	LOGL_ALERT,
	/** @LOGL_CRIT: Critical conditions */
	LOGL_CRIT,
	/** @LOGL_ERR: Error that prevents something from working */
	LOGL_ERR,
	/** @LOGL_WARNING: Warning may prevent optimal operation */
	LOGL_WARNING,
	/** @LOGL_NOTICE: Normal but significant condition, printf() */
	LOGL_NOTICE,
	/** @LOGL_INFO: General information message */
	LOGL_INFO,
	/** @LOGL_DEBUG: Basic debug-level message */
	LOGL_DEBUG,
	/** @LOGL_DEBUG_CONTENT: Debug message showing full message content */
	LOGL_DEBUG_CONTENT,
	/** @LOGL_DEBUG_IO: Debug message showing hardware I/O access */
	LOGL_DEBUG_IO,

	/** @LOGL_COUNT: Total number of valid log levels */
	LOGL_COUNT,
	/** @LOGL_NONE: Used to indicate that there is no valid log level */
	LOGL_NONE,

	/** @LOGL_LEVEL_MASK: Mask for valid log levels */
	LOGL_LEVEL_MASK = 0xf,
	/** @LOGL_FORCE_DEBUG: Mask to force output due to LOG_DEBUG */
	LOGL_FORCE_DEBUG = 0x10,

	/** @LOGL_FIRST: The first, most-important log level */
	LOGL_FIRST = LOGL_EMERG,
	/** @LOGL_MAX: The last, least-important log level */
	LOGL_MAX = LOGL_DEBUG_IO,
	/** @LOGL_CONT: Use same log level as in previous call */
	LOGL_CONT = -1,
};

/**
 * enum log_category_t - Log categories supported.
 *
 * Log categories between %LOGC_FIRST and %LOGC_NONE correspond to uclasses
 * (i.e. &enum uclass_id), but there are also some more generic categories.
 *
 * Remember to update log_cat_name[] after adding a new category.
 */
enum log_category_t {
	/** @LOGC_FIRST: First log category */
	LOGC_FIRST = 0,	/* First part mirrors UCLASS_... */

	/** @LOGC_NONE: Default log category */
	LOGC_NONE = UCLASS_COUNT,	/* First number is after all uclasses */
	/** @LOGC_ARCH: Related to arch-specific code */
	LOGC_ARCH,
	/** @LOGC_BOARD: Related to board-specific code */
	LOGC_BOARD,
	/** @LOGC_CORE: Related to core features (non-driver-model) */
	LOGC_CORE,
	/** @LOGC_DM: Core driver-model */
	LOGC_DM,
	/** @LOGC_DT: Device-tree */
	LOGC_DT,
	/** @LOGC_EFI: EFI implementation */
	LOGC_EFI,
	/** @LOGC_ALLOC: Memory allocation */
	LOGC_ALLOC,
	/** @LOGC_SANDBOX: Related to the sandbox board */
	LOGC_SANDBOX,
	/** @LOGC_BLOBLIST: Bloblist */
	LOGC_BLOBLIST,
	/** @LOGC_DEVRES: Device resources (``devres_...`` functions) */
	LOGC_DEVRES,
	/** @LOGC_ACPI: Advanced Configuration and Power Interface (ACPI) */
	LOGC_ACPI,
	/** @LOGC_BOOT: Related to boot process / boot image processing */
	LOGC_BOOT,
	/** @LOGC_EVENT: Related to event and event handling */
	LOGC_EVENT,
	/** @LOGC_FS: Related to filesystems */
	LOGC_FS,
	/** @LOGC_EXPO: Related to expo handling */
	LOGC_EXPO,
	/** @LOGC_COUNT: Number of log categories */
	LOGC_COUNT,
	/** @LOGC_END: Sentinel value for lists of log categories */
	LOGC_END,
	/** @LOGC_CONT: Use same category as in previous call */
	LOGC_CONT = -1,
};

/* Helper to cast a uclass ID to a log category */
static inline int log_uc_cat(enum uclass_id id)
{
	return (enum log_category_t)id;
}

/**
 * _log() - Internal function to emit a new log record
 *
 * @cat: Category of log record (indicating which subsystem generated it)
 * @level: Level of log record (indicating its severity)
 * @file: File name of file where log record was generated
 * @line: Line number in file where log record was generated
 * @func: Function where log record was generated
 * @fmt: printf() format string for log record
 * @...: Optional parameters, according to the format string @fmt
 * Return: 0 if log record was emitted, -ve on error
 */
int _log(enum log_category_t cat, enum log_level_t level, const char *file,
	 int line, const char *func, const char *fmt, ...)
		__attribute__ ((format (__printf__, 6, 7)));

/**
 * _log_buffer - Internal function to print data buffer in hex and ascii form
 *
 * @cat: Category of log record (indicating which subsystem generated it)
 * @level: Level of log record (indicating its severity)
 * @file: File name of file where log record was generated
 * @line: Line number in file where log record was generated
 * @func: Function where log record was generated
 * @addr:	Starting address to display at start of line
 * @data:	pointer to data buffer
 * @width:	data value width.  May be 1, 2, or 4.
 * @count:	number of values to display
 * @linelen:	Number of values to print per line; specify 0 for default length
 */
int _log_buffer(enum log_category_t cat, enum log_level_t level,
		const char *file, int line, const char *func, ulong addr,
		const void *data, uint width, uint count, uint linelen);

/* Define this at the top of a file to add a prefix to debug messages */
#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif

/* Use a default category if this file does not supply one */
#ifndef LOG_CATEGORY
#define LOG_CATEGORY LOGC_NONE
#endif

/*
 * This header may be including when CONFIG_LOG is disabled, in which case
 * CONFIG_LOG_MAX_LEVEL is not defined. Add a check for this.
 */
#if CONFIG_IS_ENABLED(LOG)
#define _LOG_MAX_LEVEL CONFIG_VAL(LOG_MAX_LEVEL)
#else
#define _LOG_MAX_LEVEL LOGL_INFO
#endif

#define log_emer(_fmt...)	log(LOG_CATEGORY, LOGL_EMERG, ##_fmt)
#define log_alert(_fmt...)	log(LOG_CATEGORY, LOGL_ALERT, ##_fmt)
#define log_crit(_fmt...)	log(LOG_CATEGORY, LOGL_CRIT, ##_fmt)
#define log_err(_fmt...)	log(LOG_CATEGORY, LOGL_ERR, ##_fmt)
#define log_warning(_fmt...)	log(LOG_CATEGORY, LOGL_WARNING, ##_fmt)
#define log_notice(_fmt...)	log(LOG_CATEGORY, LOGL_NOTICE, ##_fmt)
#define log_info(_fmt...)	log(LOG_CATEGORY, LOGL_INFO, ##_fmt)
#define log_debug(_fmt...)	log(LOG_CATEGORY, LOGL_DEBUG, ##_fmt)
#define log_content(_fmt...)	log(LOG_CATEGORY, LOGL_DEBUG_CONTENT, ##_fmt)
#define log_io(_fmt...)		log(LOG_CATEGORY, LOGL_DEBUG_IO, ##_fmt)
#define log_cont(_fmt...)	log(LOGC_CONT, LOGL_CONT, ##_fmt)

#ifdef LOG_DEBUG
#define _LOG_DEBUG	LOGL_FORCE_DEBUG
#ifndef DEBUG
#define DEBUG
#endif
#else
#define _LOG_DEBUG	0
#endif

#if CONFIG_IS_ENABLED(LOG)

/* Emit a log record if the level is less that the maximum */
#define log(_cat, _level, _fmt, _args...) ({ \
	int _l = _level; \
	if (_LOG_DEBUG != 0 || _l <= _LOG_MAX_LEVEL) \
		_log((enum log_category_t)(_cat), \
		     (enum log_level_t)(_l | _LOG_DEBUG), __FILE__, \
		     __LINE__, __func__, \
		      pr_fmt(_fmt), ##_args); \
	})

/* Emit a dump if the level is less that the maximum */
#define log_buffer(_cat, _level, _addr, _data, _width, _count, _linelen)  ({ \
	int _l = _level; \
	if (_LOG_DEBUG != 0 || _l <= _LOG_MAX_LEVEL) \
		_log_buffer((enum log_category_t)(_cat), \
			    (enum log_level_t)(_l | _LOG_DEBUG), __FILE__, \
			    __LINE__, __func__, _addr, _data, \
			    _width, _count, _linelen); \
	})
#else

/* Note: _LOG_DEBUG != 0 avoids a warning with clang */
#define log(_cat, _level, _fmt, _args...) ({ \
	int _l = _level; \
	if (_LOG_DEBUG != 0 || _l <= LOGL_INFO || \
	    (_DEBUG && _l == LOGL_DEBUG)) \
		printf(_fmt, ##_args); \
	})

#define log_buffer(_cat, _level, _addr, _data, _width, _count, _linelen)  ({ \
	int _l = _level; \
	if (_LOG_DEBUG != 0 || _l <= LOGL_INFO || \
	    (_DEBUG && _l == LOGL_DEBUG)) \
		print_buffer(_addr, _data, _width, _count, _linelen); \
	})
#endif

#ifdef DEBUG
#define _DEBUG	1
#else
#define _DEBUG	0
#endif

#ifdef CONFIG_SPL_BUILD
#define _SPL_BUILD	1
#else
#define _SPL_BUILD	0
#endif

#if CONFIG_IS_ENABLED(LOG)

#define debug_cond(cond, fmt, args...)					\
({									\
	if (cond)							\
		log(LOG_CATEGORY,					\
		    (enum log_level_t)(LOGL_FORCE_DEBUG | _LOG_DEBUG),	\
		    fmt, ##args);					\
})

#else /* _DEBUG */

/*
 * Output a debug text when condition "cond" is met. The "cond" should be
 * computed by a preprocessor in the best case, allowing for the best
 * optimization.
 */
#define debug_cond(cond, fmt, args...)		\
({						\
	if (cond)				\
		printf(pr_fmt(fmt), ##args);	\
})

#endif /* _DEBUG */

/* Show a message if DEBUG is defined in a file */
#define debug(fmt, args...)			\
	debug_cond(_DEBUG, fmt, ##args)

/* Show a message if not in SPL */
#define warn_non_spl(fmt, args...)			\
	debug_cond(!_SPL_BUILD, fmt, ##args)

/*
 * An assertion is run-time check done in debug mode only. If DEBUG is not
 * defined then it is skipped. If DEBUG is defined and the assertion fails,
 * then it calls panic*( which may or may not reset/halt U-Boot (see
 * CONFIG_PANIC_HANG), It is hoped that all failing assertions are found
 * before release, and after release it is hoped that they don't matter. But
 * in any case these failing assertions cannot be fixed with a reset (which
 * may just do the same assertion again).
 */
void __assert_fail(const char *assertion, const char *file, unsigned int line,
		   const char *function);

/**
 * assert() - assert expression is true
 *
 * If the expression x evaluates to false and _DEBUG evaluates to true, a panic
 * message is written and the system stalls. The value of _DEBUG is set to true
 * if DEBUG is defined before including common.h.
 *
 * The expression x is always executed irrespective of the value of _DEBUG.
 *
 * @x:		expression to test
 */
#define assert(x) \
	({ if (!(x) && _DEBUG) \
		__assert_fail(#x, __FILE__, __LINE__, __func__); })

/*
 * This one actually gets compiled in even without DEBUG. It doesn't include the
 * full pathname as it may be huge. Only use this when the user should be
 * warning, similar to BUG_ON() in linux.
 *
 * Return: true if assertion succeeded (condition is true), else false
 */
#define assert_noisy(x) \
	({ bool _val = (x); \
	if (!_val) \
		__assert_fail(#x, "?", __LINE__, __func__); \
	_val; \
	})

#if CONFIG_IS_ENABLED(LOG) && defined(CONFIG_LOG_ERROR_RETURN)
/*
 * Log an error return value, possibly with a message. Usage:
 *
 *	return log_ret(fred_call());
 *
 * or:
 *
 *	return log_msg_ret("get", fred_call());
 *
 * It is recommended to use <= 3 characters for the name since this will only
 * use 4 bytes in rodata
 */
#define log_ret(_ret) ({ \
	int __ret = (_ret); \
	if (__ret < 0) \
		log(LOG_CATEGORY, LOGL_ERR, "returning err=%d\n", __ret); \
	__ret; \
	})
#define log_msg_ret(_msg, _ret) ({ \
	int __ret = (_ret); \
	if (__ret < 0) \
		log(LOG_CATEGORY, LOGL_ERR, "%s: returning err=%d\n", _msg, \
		    __ret); \
	__ret; \
	})

/*
 * Similar to the above, but any non-zero value is consider an error, not just
 * values less than 0.
 */
#define log_retz(_ret) ({ \
	int __ret = (_ret); \
	if (__ret) \
		log(LOG_CATEGORY, LOGL_ERR, "returning err=%d\n", __ret); \
	__ret; \
	})
#define log_msg_retz(_msg, _ret) ({ \
	int __ret = (_ret); \
	if (__ret) \
		log(LOG_CATEGORY, LOGL_ERR, "%s: returning err=%d\n", _msg, \
		    __ret); \
	__ret; \
	})
#else
/* Non-logging versions of the above which just return the error code */
#define log_ret(_ret) (_ret)
#define log_msg_ret(_msg, _ret) ((void)(_msg), _ret)
#define log_retz(_ret) (_ret)
#define log_msg_retz(_msg, _ret) ((void)(_msg), _ret)
#endif

/** * enum log_rec_flags - Flags for a log record */
enum log_rec_flags {
	/** @LOGRECF_FORCE_DEBUG: Force output of debug record */
	LOGRECF_FORCE_DEBUG	= BIT(0),
	/** @LOGRECF_CONT: Continuation of previous log record */
	LOGRECF_CONT		= BIT(1),
};

/**
 * struct log_rec - a single log record
 *
 * Holds information about a single record in the log
 *
 * Members marked as 'not allocated' are stored as pointers and the caller is
 * responsible for making sure that the data pointed to is not overwritten.
 * Members marked as 'allocated' are allocated (e.g. via strdup()) by the log
 * system.
 *
 * TODO(sjg@chromium.org): Compress this struct down a bit to reduce space, e.g.
 * a single u32 for cat, level, line and force_debug
 *
 * @cat: Category, representing a uclass or part of U-Boot
 * @level: Severity level, less severe is higher
 * @line: Line number where the log record was generated
 * @flags: Flags for log record (enum log_rec_flags)
 * @file: Name of file where the log record was generated (not allocated)
 * @func: Function where the log record was generated (not allocated)
 * @msg: Log message (allocated)
 */
struct log_rec {
	enum log_category_t cat;
	enum log_level_t level;
	u16 line;
	u8 flags;
	const char *file;
	const char *func;
	const char *msg;
};

struct log_device;

enum log_device_flags {
	LOGDF_ENABLE		= BIT(0),	/* Device is enabled */
};

/**
 * struct log_driver - a driver which accepts and processes log records
 *
 * @name: Name of driver
 * @emit: Method to call to emit a log record via this device
 * @flags: Initial value for flags (use LOGDF_ENABLE to enable on start-up)
 */
struct log_driver {
	const char *name;

	/**
	 * @emit: emit a log record
	 *
	 * Called by the log system to pass a log record to a particular driver
	 * for processing. The filter is checked before calling this function.
	 */
	int (*emit)(struct log_device *ldev, struct log_rec *rec);
	unsigned short flags;
};

/**
 * struct log_device - an instance of a log driver
 *
 * Since drivers are set up at build-time we need to have a separate device for
 * the run-time aspects of drivers (currently just a list of filters to apply
 * to records send to this device).
 *
 * @next_filter_num: Sequence number of next filter filter added (0=no filters
 *	yet). This increments with each new filter on the device, but never
 *	decrements
 * @flags: Flags for this filter (enum log_device_flags)
 * @drv: Pointer to driver for this device
 * @filter_head: List of filters for this device
 * @sibling_node: Next device in the list of all devices
 */
struct log_device {
	unsigned short next_filter_num;
	unsigned short flags;
	struct log_driver *drv;
	struct list_head filter_head;
	struct list_head sibling_node;
};

enum {
	LOGF_MAX_CATEGORIES = 5,	/* maximum categories per filter */
};

/**
 * enum log_filter_flags - Flags which modify a filter
 */
enum log_filter_flags {
	/** @LOGFF_HAS_CAT: Filter has a category list */
	LOGFF_HAS_CAT	= 1 << 0,
	/** @LOGFF_DENY: Filter denies matching messages */
	LOGFF_DENY	= 1 << 1,
	/** @LOGFF_LEVEL_MIN: Filter's level is a minimum, not a maximum */
	LOGFF_LEVEL_MIN = 1 << 2,
};

/**
 * struct log_filter - criteria to filter out log messages
 *
 * If a message matches all criteria, then it is allowed. If LOGFF_DENY is set,
 * then it is denied instead.
 *
 * @filter_num: Sequence number of this filter.  This is returned when adding a
 *	new filter, and must be provided when removing a previously added
 *	filter.
 * @flags: Flags for this filter (``LOGFF_...``)
 * @cat_list: List of categories to allow (terminated by %LOGC_END). If empty
 *	then all categories are permitted. Up to %LOGF_MAX_CATEGORIES entries
 *	can be provided
 * @level: Maximum (or minimum, if %LOGFF_MIN_LEVEL) log level to allow
 * @file_list: List of files to allow, separated by comma. If NULL then all
 *	files are permitted
 * @sibling_node: Next filter in the list of filters for this log device
 */
struct log_filter {
	int filter_num;
	int flags;
	enum log_category_t cat_list[LOGF_MAX_CATEGORIES];
	enum log_level_t level;
	const char *file_list;
	struct list_head sibling_node;
};

#define LOG_DRIVER(_name) \
	ll_entry_declare(struct log_driver, _name, log_driver)

/* Get a pointer to a given driver */
#define LOG_GET_DRIVER(__name)						\
	ll_entry_get(struct log_driver, __name, log_driver)

/**
 * log_get_cat_name() - Get the name of a category
 *
 * @cat: Category to look up
 * Return: category name (which may be a uclass driver name) if found, or
 *	   "<invalid>" if invalid, or "<missing>" if not found. All error
 *	   responses begin with '<'.
 */
const char *log_get_cat_name(enum log_category_t cat);

/**
 * log_get_cat_by_name() - Look up a category by name
 *
 * @name: Name to look up
 * Return: Category, or %LOGC_NONE if not found
 */
enum log_category_t log_get_cat_by_name(const char *name);

/**
 * log_get_level_name() - Get the name of a log level
 *
 * @level: Log level to look up
 * Return: Log level name (in ALL CAPS)
 */
const char *log_get_level_name(enum log_level_t level);

/**
 * log_get_level_by_name() - Look up a log level by name
 *
 * @name: Name to look up
 * Return: Log level, or %LOGL_NONE if not found
 */
enum log_level_t log_get_level_by_name(const char *name);

/**
 * log_device_find_by_name() - Look up a log device by its driver's name
 *
 * @drv_name: Name of the driver
 * Return: the log device, or %NULL if not found
 */
struct log_device *log_device_find_by_name(const char *drv_name);

/**
 * log_has_cat() - check if a log category exists within a list
 *
 * @cat_list: List of categories to check, at most %LOGF_MAX_CATEGORIES entries
 *	long, terminated by %LC_END if fewer
 * @cat: Category to search for
 *
 * Return: ``true`` if @cat is in @cat_list, else ``false``
 */
bool log_has_cat(enum log_category_t cat_list[], enum log_category_t cat);

/**
 * log_has_file() - check if a file is with a list
 *
 * @file_list: List of files to check, separated by comma
 * @file: File to check for. This string is matched against the end of each
 *	file in the list, i.e. ignoring any preceding path. The list is
 *	intended to consist of relative pathnames, e.g. common/main.c,cmd/log.c
 *
 * Return: ``true`` if @file is in @file_list, else ``false``
 */
bool log_has_file(const char *file_list, const char *file);

/* Log format flags (bit numbers) for gd->log_fmt. See log_fmt_chars */
enum log_fmt {
	LOGF_CAT	= 0,
	LOGF_LEVEL,
	LOGF_FILE,
	LOGF_LINE,
	LOGF_FUNC,
	LOGF_MSG,

	LOGF_COUNT,
	LOGF_ALL = 0x3f,
};

/* Handle the 'log test' command */
int do_log_test(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);

/**
 * log_add_filter_flags() - Add a new filter to a log device, specifying flags
 *
 * @drv_name: Driver name to add the filter to (since each driver only has a
 *	single device)
 * @flags: Flags for this filter (``LOGFF_...``)
 * @cat_list: List of categories to allow (terminated by %LOGC_END). If empty
 *	then all categories are permitted. Up to %LOGF_MAX_CATEGORIES entries
 *	can be provided
 * @level: Maximum (or minimum, if %LOGFF_LEVEL_MIN) log level to allow
 * @file_list: List of files to allow, separated by comma. If NULL then all
 *	files are permitted
 * Return:
 *   the sequence number of the new filter (>=0) if the filter was added, or a
 *   -ve value on error
 */
int log_add_filter_flags(const char *drv_name, enum log_category_t cat_list[],
			 enum log_level_t level, const char *file_list,
			 int flags);

/**
 * log_add_filter() - Add a new filter to a log device
 *
 * @drv_name: Driver name to add the filter to (since each driver only has a
 *	single device)
 * @cat_list: List of categories to allow (terminated by %LOGC_END). If empty
 *	then all categories are permitted. Up to %LOGF_MAX_CATEGORIES entries
 *	can be provided
 * @max_level: Maximum log level to allow
 * @file_list: List of files to allow, separated by comma. If %NULL then all
 *	files are permitted
 * Return:
 *   the sequence number of the new filter (>=0) if the filter was added, or a
 *   -ve value on error
 */
static inline int log_add_filter(const char *drv_name,
				 enum log_category_t cat_list[],
				 enum log_level_t max_level,
				 const char *file_list)
{
	return log_add_filter_flags(drv_name, cat_list, max_level, file_list,
				    0);
}

/**
 * log_remove_filter() - Remove a filter from a log device
 *
 * @drv_name: Driver name to remove the filter from (since each driver only has
 *	a single device)
 * @filter_num: Filter number to remove (as returned by log_add_filter())
 * Return:
 *   0 if the filter was removed, -%ENOENT if either the driver or the filter
 *   number was not found
 */
int log_remove_filter(const char *drv_name, int filter_num);

/**
 * log_device_set_enable() - Enable or disable a log device
 *
 * Devices are referenced by their driver, so use LOG_GET_DRIVER(name) to pass
 * the driver to this function. For example if the driver is declared with
 * LOG_DRIVER(wibble) then pass LOG_GET_DRIVER(wibble) here.
 *
 * @drv: Driver of device to enable
 * @enable: true to enable, false to disable
 * Return: 0 if OK, -ENOENT if the driver was not found
 */
int log_device_set_enable(struct log_driver *drv, bool enable);

#if CONFIG_IS_ENABLED(LOG)
/**
 * log_init() - Set up the log system ready for use
 *
 * Return: 0 if OK, -%ENOMEM if out of memory
 */
int log_init(void);
#else
static inline int log_init(void)
{
	return 0;
}
#endif

/**
 * log_get_default_format() - get default log format
 *
 * The default log format is configurable via
 * %CONFIG_LOGF_FILE, %CONFIG_LOGF_LINE, and %CONFIG_LOGF_FUNC.
 *
 * Return:	default log format
 */
static inline int log_get_default_format(void)
{
	return BIT(LOGF_MSG) |
	       (IS_ENABLED(CONFIG_LOGF_FILE) ? BIT(LOGF_FILE) : 0) |
	       (IS_ENABLED(CONFIG_LOGF_LINE) ? BIT(LOGF_LINE) : 0) |
	       (IS_ENABLED(CONFIG_LOGF_FUNC) ? BIT(LOGF_FUNC) : 0);
}

#endif
