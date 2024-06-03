/*
 * wpa_supplicant/hostapd / Debug prints
 * Copyright (c) 2002-2013, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef WPA_DEBUG_H
#define WPA_DEBUG_H

#include "wpabuf.h"

#ifdef CONFIG_DEBUG_SYSLOG
extern int wpa_debug_syslog;
#endif /* CONFIG_DEBUG_SYSLOG */

#define DPP_EVENT_AUTH_SUCCESS "DPP-AUTH-SUCCESS "
#define DPP_EVENT_AUTH_INIT_FAILED "DPP-AUTH-INIT-FAILED "
#define DPP_EVENT_NOT_COMPATIBLE "DPP-NOT-COMPATIBLE "
#define DPP_EVENT_RESPONSE_PENDING "DPP-RESPONSE-PENDING "
#define DPP_EVENT_SCAN_PEER_QR_CODE "DPP-SCAN-PEER-QR-CODE "
#define DPP_EVENT_AUTH_DIRECTION "DPP-AUTH-DIRECTION "
#define DPP_EVENT_CONF_RECEIVED "DPP-CONF-RECEIVED "
#define DPP_EVENT_CONF_SENT "DPP-CONF-SENT "
#define DPP_EVENT_CONF_FAILED "DPP-CONF-FAILED "
#define DPP_EVENT_CONFOBJ_AKM "DPP-CONFOBJ-AKM "
#define DPP_EVENT_CONFOBJ_SSID "DPP-CONFOBJ-SSID "
#define DPP_EVENT_CONFOBJ_PASS "DPP-CONFOBJ-PASS "
#define DPP_EVENT_CONFOBJ_PSK "DPP-CONFOBJ-PSK "
#define DPP_EVENT_CONNECTOR "DPP-CONNECTOR "
#define DPP_EVENT_C_SIGN_KEY "DPP-C-SIGN-KEY "
#define DPP_EVENT_NET_ACCESS_KEY "DPP-NET-ACCESS-KEY "
#define DPP_EVENT_MISSING_CONNECTOR "DPP-MISSING-CONNECTOR "
#define DPP_EVENT_NETWORK_ID "DPP-NETWORK-ID "
#define DPP_EVENT_RX "DPP-RX "
#define DPP_EVENT_TX "DPP-TX "
#define DPP_EVENT_TX_STATUS "DPP-TX-STATUS "
#define DPP_EVENT_FAIL "DPP-FAIL "
#define DPP_EVENT_PKEX_T_LIMIT "DPP-PKEX-T-LIMIT "
#define DPP_EVENT_INTRO "DPP-INTRO "
#define DPP_EVENT_CONF_REQ_RX "DPP-CONF-REQ-RX "



/* Debugging function - conditional printf and hex dump. Driver wrappers can
 * use these for debugging purposes. */

enum {
	MSG_INFO1,
	MSG_ERROR,
	MSG_DEBUG,
	MSG_MSGDUMP,
	MSG_WARNING,
	MSG_EXCESSIVE,
};

int wpa_debug_open_file(const char *path);
int wpa_debug_reopen_file(void);
void wpa_debug_close_file(void);
void wpa_debug_setup_stdout(void);

/**
 * wpa_debug_printf_timestamp - Print timestamp for debug output
 *
 * This function prints a timestamp in seconds_from_1970.microsoconds
 * format if debug output has been configured to include timestamps in debug
 * messages.
 */
void wpa_debug_print_timestamp(void);

/**
 * wpa_printf - conditional printf
 * @level: priority level (MSG_*) of the message
 * @fmt: printf format string, followed by optional arguments
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration.
 *
 * Note: New line '\n' is added to the end of the text when printing to stdout.
 */
void wpa_printf(int level, const char *fmt, ...)
PRINTF_FORMAT(2, 3);

/**
 * wpa_hexdump - conditional hex dump
 * @level: priority level (MSG_*) of the message
 * @title: title of for the message
 * @buf: data buffer to be dumped
 * @len: length of the buf
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration. The contents of buf is printed out has hex dump.
 */
void wpa_hexdump(int level, const char *title, const void *buf, size_t len);

static inline void wpa_hexdump_buf(int level, const char *title,
				   const struct wpabuf *buf)
{
	wpa_hexdump(level, title, buf ? wpabuf_head(buf) : NULL,
		    buf ? wpabuf_len(buf) : 0);
}

/**
 * wpa_hexdump_key - conditional hex dump, hide keys
 * @level: priority level (MSG_*) of the message
 * @title: title of for the message
 * @buf: data buffer to be dumped
 * @len: length of the buf
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration. The contents of buf is printed out has hex dump. This works
 * like wpa_hexdump(), but by default, does not include secret keys (passwords,
 * etc.) in debug output.
 */
void wpa_hexdump_key(int level, const char *title, const void *buf, size_t len);

static inline void wpa_hexdump_buf_key(int level, const char *title,
				       const struct wpabuf *buf)
{
	wpa_hexdump_key(level, title, buf ? wpabuf_head(buf) : NULL,
			buf ? wpabuf_len(buf) : 0);
}

/**
 * wpa_hexdump_ascii - conditional hex dump
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
void wpa_hexdump_ascii(int level, const char *title, const void *buf,
		       size_t len);

/**
 * wpa_hexdump_ascii_key - conditional hex dump, hide keys
 * @level: priority level (MSG_*) of the message
 * @title: title of for the message
 * @buf: data buffer to be dumped
 * @len: length of the buf
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration. The contents of buf is printed out has hex dump with both
 * the hex numbers and ASCII characters (for printable range) are shown. 16
 * bytes per line will be shown. This works like wpa_hexdump_ascii(), but by
 * default, does not include secret keys (passwords, etc.) in debug output.
 */
void wpa_hexdump_ascii_key(int level, const char *title, const void *buf,
			   size_t len);

#endif /* WPA_DEBUG_H */
