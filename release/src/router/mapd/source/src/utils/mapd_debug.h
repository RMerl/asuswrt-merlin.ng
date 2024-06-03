#ifndef mapd_DEBUG_H
#define mapd_DEBUG_H


extern int mapd_debug_level;
extern int mapd_debug_timestamp;
#ifdef CONFIG_DEBUG_SYSLOG
extern int mapd_debug_syslog;
#endif /* CONFIG_DEBUG_SYSLOG */

/* Debugging function - conditional printf and hex dump. Driver wrappers can
 * use these for debugging purposes. */

enum {
	MSG_EXCESSIVE, MSG_MSGDUMP, MSG_DEBUG, MSG_INFO, MSG_WARNING, MSG_ERROR, MSG_OFF
};

#define PRINT_MAC(a) a[0],a[1],a[2],a[3],a[4],a[5]

#ifdef CONFIG_NO_STDOUT_DEBUG

#define debug(...) do { } while (0)
#define debug_syslog(...) do { } while (0)
#define warn(...) do { } while (0)
#define err(...) do { } while (0)
#define always(...) do { } while (0)
#define mapd_debug_print_timestamp() do { } while (0)
#define mapd_printf(args...) do { } while (0)
#define mapd_hexdump(l,t,b,le) do { } while (0)
#define mapd_hexdump_buf(l,t,b) do { } while (0)
#define mapd_hexdump_key(l,t,b,le) do { } while (0)
#define mapd_hexdump_buf_key(l,t,b) do { } while (0)
#define mapd_hexdump_ascii(l,t,b,le) do { } while (0)
#define mapd_hexdump_ascii_key(l,t,b,le) do { } while (0)
#define mapd_debug_open_file(p) do { } while (0)
#define mapd_debug_close_file() do { } while (0)
#define mapd_debug_setup_stdout() do { } while (0)
#define mapd_dbg(args...) do { } while (0)

static inline int mapd_debug_reopen_file(void)
{
	return 0;
}

#else /* CONFIG_NO_STDOUT_DEBUG */

int mapd_debug_open_file(const char *path);
int mapd_debug_reopen_file(void);
void mapd_debug_close_file(void);
void mapd_debug_setup_stdout(void);

/**
 * mapd_debug_printf_timestamp - Print timestamp for debug output
 *
 * This function prints a timestamp in seconds_from_1970.microsoconds
 * format if debug output has been configured to include timestamps in debug
 * messages.
 */
void mapd_debug_print_timestamp(void);

/**
 * mapd_printf - conditional printf
 * @level: priority level (MSG_*) of the message
 * @fmt: printf format string, followed by optional arguments
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration.
 *
 * Note: New line '\n' is added to the end of the text when printing to stdout.
 */
void  mapd_printf_debug(const char *func, int line, int level, const char *fmt, ...)
PRINTF_FORMAT(4, 5);

void  mapd_printf_raw(const char *func, int line,int level, const char *fmt, ...)
PRINTF_FORMAT(4, 5);


#define mapd_printf(...) mapd_printf_debug(__func__, __LINE__, __VA_ARGS__);
#define excess_debug(...) mapd_printf(MSG_EXCESSIVE, __VA_ARGS__)
#define debug(...) mapd_printf(MSG_DEBUG, __VA_ARGS__)
#define debug_syslog(...) debug(__VA_ARGS__)
#define warn(...) mapd_printf(MSG_WARNING, __VA_ARGS__)
#define always(...) mapd_printf_raw(__func__, __LINE__,MSG_OFF, __VA_ARGS__)
#define err(...) mapd_printf(MSG_ERROR, __VA_ARGS__)
#define info(...) mapd_printf(MSG_INFO, __VA_ARGS__)

/**
 * mapd_hexdump - conditional hex dump
 * @level: priority level (MSG_*) of the message
 * @title: title of for the message
 * @buf: data buffer to be dumped
 * @len: length of the buf
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration. The contents of buf is printed out has hex dump.
 */
void mapd_hexdump(int level, const char *title, const void *buf, size_t len);

/**
 * mapd_hexdump_key - conditional hex dump, hide keys
 * @level: priority level (MSG_*) of the message
 * @title: title of for the message
 * @buf: data buffer to be dumped
 * @len: length of the buf
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration. The contents of buf is printed out has hex dump. This works
 * like mapd_hexdump(), but by default, does not include secret keys (passwords,
 * etc.) in debug output.
 */
void mapd_hexdump_key(int level, const char *title, const void *buf, size_t len);

/**
 * mapd_hexdump_ascii - conditional hex dump
 * @level: priority level (MSG_*) of the message
 * @title: title of for the message
 * @buf: data buffer to be dumped
 * @len: length of the buf
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration. The contents of buf is printed out has hex dump with both
 * the hex numbers and ASCII characters (for printable range) are shown. 16
 * bytes per line will be shown.
 */
void mapd_hexdump_ascii(int level, const char *title, const void *buf,
		       size_t len);

/**
 * mapd_hexdump_ascii_key - conditional hex dump, hide keys
 * @level: priority level (MSG_*) of the message
 * @title: title of for the message
 * @buf: data buffer to be dumped
 * @len: length of the buf
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration. The contents of buf is printed out has hex dump with both
 * the hex numbers and ASCII characters (for printable range) are shown. 16
 * bytes per line will be shown. This works like mapd_hexdump_ascii(), but by
 * default, does not include secret keys (passwords, etc.) in debug output.
 */
void mapd_hexdump_ascii_key(int level, const char *title, const void *buf,
			   size_t len);

/*
 * mapd_dbg() behaves like mapd_msg(), but it can be removed from build to reduce
 * binary size. As such, it should be used with debugging messages that are not
 * needed in the control interface while mapd_msg() has to be used for anything
 * that needs to shown to control interface monitors.
 */
#define mapd_dbg(args...) mapd_msg(args)

#endif /* CONFIG_NO_STDOUT_DEBUG */


#ifdef CONFIG_DEBUG_SYSLOG

void mapd_debug_open_syslog(void);
void mapd_debug_close_syslog(void);

#else /* CONFIG_DEBUG_SYSLOG */

static inline void mapd_debug_open_syslog(void)
{
}

static inline void mapd_debug_close_syslog(void)
{
}

#endif /* CONFIG_DEBUG_SYSLOG */

#ifdef CONFIG_DEBUG_LINUX_TRACING

int mapd_debug_open_linux_tracing(void);
void mapd_debug_close_linux_tracing(void);

#else /* CONFIG_DEBUG_LINUX_TRACING */

static inline int mapd_debug_open_linux_tracing(void)
{
	return 0;
}

static inline void mapd_debug_close_linux_tracing(void)
{
    return;
}

#endif /* CONFIG_DEBUG_LINUX_TRACING */


#ifdef RELEASE_EXCLUDE
#define mapd_ASSERT(a) \
	do { 	 \
		if (!(a)) {					       \
		printf("mapd_ASSERT FAILED '" #a "' "	       \
				"%s %s:%d\n",			       \
				__FUNCTION__, __FILE__, __LINE__);      \
		}	 \
	} while (0)
#else
#define mapd_ASSERT(a)						       \
	do {							       \
		if (!(a)) {					       \
			printf("mapd_ASSERT FAILED '" #a "' "	       \
			       "%s %s:%d\n",			       \
			       __FUNCTION__, __FILE__, __LINE__);      \
			exit(1);				       \
		}						       \
	} while (0)
#endif

const char * debug_level_str(int level);
int str_to_debug_level(const char *s);

static inline void hex_dump(const char *title, const void *buf, size_t len)
{
	mapd_hexdump(MSG_EXCESSIVE, title, buf, len);
}

#endif /* mapd_DEBUG_H */
