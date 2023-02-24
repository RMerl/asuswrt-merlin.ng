/*
 * libusbmuxd.c
 *
 * Copyright (C) 2009-2019 Nikias Bassen <nikias@gmx.li>
 * Copyright (C) 2009-2014 Martin Szulecki <m.szulecki@libimobiledevice.org>
 * Copyright (C) 2009 Paul Sladen <libiphone@paul.sladen.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
  #define USBMUXD_API __declspec( dllexport )
#else
  #ifdef HAVE_FVISIBILITY
    #define USBMUXD_API __attribute__((visibility("default")))
  #else
    #define USBMUXD_API
  #endif
#endif

#ifndef EPROTO
#define EPROTO 134
#endif
#ifndef EBADMSG
#define EBADMSG 104
#endif
#ifndef ECONNREFUSED
#define ECONNREFUSED 107
#endif

#include <unistd.h>
#include <signal.h>

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#ifndef HAVE_SLEEP
#define sleep(x) Sleep(x*1000)
#endif
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#if defined(HAVE_PROGRAM_INVOCATION_SHORT_NAME) && !defined(HAVE_PROGRAM_INVOCATION_SHORT_NAME_ERRNO_H)
extern char *program_invocation_short_name;
#endif
#ifdef __APPLE__
extern int _NSGetExecutablePath(char* buf, uint32_t* bufsize);
#include <sys/stat.h>
#endif
#endif

#ifdef HAVE_INOTIFY
#include <sys/inotify.h>
#include <sys/select.h>
#define EVENT_SIZE  (sizeof (struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))
#define USBMUXD_DIRNAME "/var/run"
#define USBMUXD_SOCKET_NAME "usbmuxd"
static int use_inotify = 1;
#endif /* HAVE_INOTIFY */

#ifndef HAVE_STPNCPY
static char* stpncpy(char *dst, const char *src, size_t len)
{
	size_t n = strlen(src);
	if (n > len)
		n = len;
	return strncpy(dst, src, len) + n;
}
#endif

#include <plist/plist.h>
#define PLIST_CLIENT_VERSION_STRING PACKAGE_STRING
#define PLIST_LIBUSBMUX_VERSION 3

static char *bundle_id = NULL;
static char *prog_name = NULL;

// usbmuxd public interface
#include "usbmuxd.h"
// usbmuxd protocol
#include "usbmuxd-proto.h"
// socket utility functions
#include "socket.h"
// misc utility functions
#include "collection.h"
// threads
#include "thread.h"

static int libusbmuxd_debug = 0;
#ifndef PACKAGE
#define PACKAGE "libusbmuxd"
#endif
#define LIBUSBMUXD_DEBUG(level, format, ...) if (level <= libusbmuxd_debug) fprintf(stderr, ("[" PACKAGE "] " format), __VA_ARGS__); fflush(stderr);
#define LIBUSBMUXD_ERROR(format, ...) LIBUSBMUXD_DEBUG(0, format, __VA_ARGS__)

static struct collection devices;
static THREAD_T devmon = THREAD_T_NULL;
static int listenfd = -1;
static int running = 0;
static int cancelling = 0;

static volatile int use_tag = 0;
static volatile int proto_version = 1;
static volatile int try_list_devices = 1;

struct usbmuxd_subscription_context {
	usbmuxd_event_cb_t callback;
	void *user_data;
};

static struct usbmuxd_subscription_context *event_ctx = NULL;

static struct collection listeners;
thread_once_t listener_init_once = THREAD_ONCE_INIT;
mutex_t listener_mutex;

/**
 * Finds a device info record by its handle.
 * if the record is not found, NULL is returned.
 */
static usbmuxd_device_info_t *devices_find(uint32_t handle)
{
	FOREACH(usbmuxd_device_info_t *dev, &devices) {
		if (dev && dev->handle == handle) {
			return dev;
		}
	} ENDFOREACH
	return NULL;
}

/**
 * Creates a socket connection to usbmuxd.
 * For Mac/Linux it is a unix domain socket,
 * for Windows it is a tcp socket.
 */
static int connect_usbmuxd_socket()
{
	char *usbmuxd_socket_addr = getenv("USBMUXD_SOCKET_ADDRESS");
	if (usbmuxd_socket_addr) {
		if (strncmp(usbmuxd_socket_addr, "UNIX:", 5) == 0) {
#if defined(WIN32) || defined(__CYGWIN__)
			/* not supported, ignore */
#else
			if (usbmuxd_socket_addr[5] != '\0') {
				return socket_connect_unix(usbmuxd_socket_addr+5);
			}
#endif
		} else {
			uint16_t port = 0;
			char *p = strrchr(usbmuxd_socket_addr, ':');
			if (p) {
				char *endp = NULL;
				long l_port = strtol(p+1, &endp, 10);
				if (endp && *endp == '\0') {
					if (l_port > 0 && l_port < 65536) {
						port = (uint16_t)l_port;
					}
				}
			}
			if (p && port > 0) {
				char *connect_addr = NULL;
				if (usbmuxd_socket_addr[0] == '[') {
					connect_addr = strdup(usbmuxd_socket_addr+1);
					connect_addr[p - usbmuxd_socket_addr - 1] = '\0';
					p = strrchr(connect_addr, ']');
					if (p) {
						*p = '\0';
					}
				} else {
					connect_addr = strdup(usbmuxd_socket_addr);
					connect_addr[p - usbmuxd_socket_addr] = '\0';
				}
				if (connect_addr && *connect_addr != '\0') {
					int res = socket_connect(connect_addr, port);
#ifdef HAVE_INOTIFY
					use_inotify = 0;
#endif
					free(connect_addr);
					return res;
				}
				free(connect_addr);
			}
		}
	}
#if defined(WIN32) || defined(__CYGWIN__)
	return socket_connect("127.0.0.1", USBMUXD_SOCKET_PORT);
#else
	return socket_connect_unix(USBMUXD_SOCKET_FILE);
#endif
}

static void sanitize_udid(usbmuxd_device_info_t *devinfo)
{
	if (!devinfo)
		return;
	if (strlen(devinfo->udid) == 24) {
		memmove(&devinfo->udid[9], &devinfo->udid[8], 17);
		devinfo->udid[8] = '-';
	}
	if (strcasecmp(devinfo->udid, "ffffffffffffffffffffffffffffffffffffffff") == 0) {
		sprintf(devinfo->udid + 32, "%08x", devinfo->handle);
	}
}

static usbmuxd_device_info_t *device_info_from_plist(plist_t props)
{
	usbmuxd_device_info_t* devinfo = NULL;
	plist_t n = NULL;
	uint64_t val = 0;
	char *strval = NULL;

	devinfo = (usbmuxd_device_info_t*)malloc(sizeof(usbmuxd_device_info_t));
	if (!devinfo)
		return NULL;
	memset(devinfo, 0, sizeof(usbmuxd_device_info_t));

	n = plist_dict_get_item(props, "DeviceID");
	if (n && plist_get_node_type(n) == PLIST_UINT) {
		plist_get_uint_val(n, &val);
		devinfo->handle = (uint32_t)val;
	}

	n = plist_dict_get_item(props, "ProductID");
	if (n && plist_get_node_type(n) == PLIST_UINT) {
		plist_get_uint_val(n, &val);
		devinfo->product_id = (uint32_t)val;
	}

	n = plist_dict_get_item(props, "SerialNumber");
	if (n && plist_get_node_type(n) == PLIST_STRING) {
		plist_get_string_val(n, &strval);
		if (strval) {
			char *t = stpncpy(devinfo->udid, strval, sizeof(devinfo->udid)-1);
			*t = '\0';
			sanitize_udid(devinfo);
			free(strval);
		}
	}

	n = plist_dict_get_item(props, "ConnectionType");
	if (n && plist_get_node_type(n) == PLIST_STRING) {
		plist_get_string_val(n, &strval);
		if (strval) {
			if (strcmp(strval, "USB") == 0) {
				devinfo->conn_type = CONNECTION_TYPE_USB;
			} else if (strcmp(strval, "Network") == 0) {
				devinfo->conn_type = CONNECTION_TYPE_NETWORK;
				n = plist_dict_get_item(props, "NetworkAddress");
				if (n && plist_get_node_type(n) == PLIST_DATA) {
					char *netaddr = NULL;
					uint64_t addr_len = 0;
					plist_get_data_val(n, &netaddr, &addr_len);
					if (netaddr && addr_len > 0 && addr_len < sizeof(devinfo->conn_data)) {
						memcpy(devinfo->conn_data, netaddr, addr_len);
					}
					free(netaddr);
				}
			} else {
				LIBUSBMUXD_ERROR("%s: Unexpected ConnectionType '%s'\n", __func__, strval);
			}
			free(strval);
		}
	}

	if (!devinfo->udid[0]) {
		LIBUSBMUXD_ERROR("%s: Failed to get SerialNumber (UDID)!\n", __func__);
		free(devinfo);
		return NULL;
	}
	if (!devinfo->conn_type) {
		LIBUSBMUXD_ERROR("%s: Failed to get ConnectionType!\n", __func__);
		free(devinfo);
		devinfo = NULL;
	} else if (devinfo->conn_type == CONNECTION_TYPE_NETWORK && !devinfo->conn_data[0]) {
		LIBUSBMUXD_ERROR("%s: Failed to get EscapedFullServiceName!\n", __func__);
		free(devinfo);
		devinfo = NULL;
	}

	return devinfo;
}

static usbmuxd_device_info_t *device_info_from_device_record(struct usbmuxd_device_record *dev)
{
	if (!dev) {
		return NULL;
	}
	usbmuxd_device_info_t *devinfo = (usbmuxd_device_info_t*)malloc(sizeof(usbmuxd_device_info_t));
	if (!devinfo) {
		LIBUSBMUXD_ERROR("%s: Out of memory while allocating device info object\n", __func__);
		return NULL;
	}

	devinfo->handle = dev->device_id;
	devinfo->product_id = dev->product_id;
	char *t = stpncpy(devinfo->udid, dev->serial_number, sizeof(devinfo->udid)-2);
	*t = '\0';
	sanitize_udid(devinfo);

	return devinfo;
}

static int receive_packet(int sfd, struct usbmuxd_header *header, void **payload, int timeout)
{
	int recv_len;
	struct usbmuxd_header hdr;
	char *payload_loc = NULL;

	header->length = 0;
	header->version = 0;
	header->message = 0;
	header->tag = 0;

	recv_len = socket_receive_timeout(sfd, &hdr, sizeof(hdr), 0, timeout);
	if (recv_len < 0) {
		if (!cancelling) {
			LIBUSBMUXD_DEBUG(1, "%s: Error receiving packet: %s\n", __func__, strerror(-recv_len));
		}
		return recv_len;
	} else if ((size_t)recv_len < sizeof(hdr)) {
		LIBUSBMUXD_DEBUG(1, "%s: Received packet is too small, got %d bytes!\n", __func__, recv_len);
		return recv_len;
	}

	uint32_t payload_size = hdr.length - sizeof(hdr);
	if (payload_size > 0) {
		payload_loc = (char*)malloc(payload_size);
		uint32_t rsize = 0;
		do {
			int res = socket_receive_timeout(sfd, payload_loc + rsize, payload_size - rsize, 0, 5000);
			if (res < 0) {
				break;
			}
			rsize += res;
		} while (rsize < payload_size);
		if (rsize != payload_size) {
			LIBUSBMUXD_DEBUG(1, "%s: Error receiving payload of size %d (bytes received: %d)\n", __func__, payload_size, rsize);
			free(payload_loc);
			return -EBADMSG;
		}
	}

	if (hdr.message == MESSAGE_PLIST) {
		char *message = NULL;
		plist_t plist = NULL;
		plist_from_xml(payload_loc, payload_size, &plist);
		free(payload_loc);

		if (!plist) {
			LIBUSBMUXD_DEBUG(1, "%s: Error getting plist from payload!\n", __func__);
			return -EBADMSG;
		}

		plist_t node = plist_dict_get_item(plist, "MessageType");
		if (!node || plist_get_node_type(node) != PLIST_STRING) {
			*payload = plist;
			hdr.length = sizeof(hdr);
			memcpy(header, &hdr, sizeof(hdr));
			return hdr.length;
		}

		plist_get_string_val(node, &message);
		if (message) {
			uint64_t val = 0;
			if (strcmp(message, "Result") == 0) {
				/* result message */
				uint32_t dwval = 0;
				plist_t n = plist_dict_get_item(plist, "Number");
				plist_get_uint_val(n, &val);
				*payload = malloc(sizeof(uint32_t));
				dwval = val;
				memcpy(*payload, &dwval, sizeof(dwval));
				hdr.length = sizeof(hdr) + sizeof(dwval);
				hdr.message = MESSAGE_RESULT;
			} else if (strcmp(message, "Attached") == 0) {
				/* device add message */
				usbmuxd_device_info_t *devinfo = NULL;
				plist_t props = plist_dict_get_item(plist, "Properties");
				if (!props) {
					LIBUSBMUXD_DEBUG(1, "%s: Could not get properties for message '%s' from plist!\n", __func__, message);
					free(message);
					plist_free(plist);
					return -EBADMSG;
				}

				devinfo = device_info_from_plist(props);
				if (!devinfo) {
					LIBUSBMUXD_DEBUG(1, "%s: Could not create device info object from properties!\n", __func__);
					free(message);
					plist_free(plist);
					return -EBADMSG;
				}
				*payload = (void*)devinfo;
				hdr.length = sizeof(hdr) + sizeof(usbmuxd_device_info_t);
				hdr.message = MESSAGE_DEVICE_ADD;
			} else if (strcmp(message, "Detached") == 0) {
				/* device remove message */
				uint32_t dwval = 0;
				plist_t n = plist_dict_get_item(plist, "DeviceID");
				if (n) {
					plist_get_uint_val(n, &val);
					*payload = malloc(sizeof(uint32_t));
					dwval = val;
					memcpy(*payload, &dwval, sizeof(dwval));
					hdr.length = sizeof(hdr) + sizeof(dwval);
					hdr.message = MESSAGE_DEVICE_REMOVE;
				}
			} else if (strcmp(message, "Paired") == 0) {
				/* device pair message */
				uint32_t dwval = 0;
				plist_t n = plist_dict_get_item(plist, "DeviceID");
				if (n) {
					plist_get_uint_val(n, &val);
					*payload = malloc(sizeof(uint32_t));
					dwval = val;
					memcpy(*payload, &dwval, sizeof(dwval));
					hdr.length = sizeof(hdr) + sizeof(dwval);
					hdr.message = MESSAGE_DEVICE_PAIRED;
				}
			} else {
				char *xml = NULL;
				uint32_t len = 0;
				plist_to_xml(plist, &xml, &len);
				LIBUSBMUXD_DEBUG(1, "%s: Unexpected message '%s' in plist:\n%s\n", __func__, message, xml);
				free(xml);
				free(message);
				plist_free(plist);
				return -EBADMSG;
			}
			free(message);
		}
		plist_free(plist);
	} else if (hdr.message == MESSAGE_DEVICE_ADD) {
		usbmuxd_device_info_t *devinfo = device_info_from_device_record((struct usbmuxd_device_record*)payload_loc);
		free(payload_loc);
		*payload = devinfo;
	} else {
		*payload = payload_loc;
	}

	memcpy(header, &hdr, sizeof(hdr));

	return hdr.length;
}

/**
 * Retrieves the result code to a previously sent request.
 */
static int usbmuxd_get_result(int sfd, uint32_t tag, uint32_t *result, void **result_plist)
{
	struct usbmuxd_header hdr;
	int recv_len;
	uint32_t *res = NULL;

	if (!result) {
		return -EINVAL;
	}
	*result = -1;
	if (result_plist) {
		*result_plist = NULL;
	}

	recv_len = receive_packet(sfd, &hdr, (void**)&res, 5000);
	if (recv_len < 0 || (size_t)recv_len < sizeof(hdr)) {
		free(res);
		return (recv_len < 0 ? recv_len : -EPROTO);
	}

	if (hdr.message == MESSAGE_RESULT) {
		int ret = 0;
		if (hdr.tag != tag) {
			LIBUSBMUXD_DEBUG(1, "%s: WARNING: tag mismatch (%d != %d). Proceeding anyway.\n", __func__, hdr.tag, tag);
		}
		if (res) {
			memcpy(result, res, sizeof(uint32_t));
			ret = 1;
		}
		free(res);
		return ret;
	} else if (hdr.message == MESSAGE_PLIST) {
		if (!result_plist) {
			LIBUSBMUXD_DEBUG(1, "%s: MESSAGE_PLIST result but result_plist pointer is NULL!\n", __func__);
			return -1;
		}
		*result_plist = (plist_t)res;
		*result = RESULT_OK;
		return 1;
	}

	LIBUSBMUXD_DEBUG(1, "%s: Unexpected message of type %d received!\n", __func__, hdr.message);
	free(res);
	return -EPROTO;
}

static int send_packet(int sfd, uint32_t message, uint32_t tag, void *payload, uint32_t payload_size)
{
	struct usbmuxd_header header;

	header.length = sizeof(struct usbmuxd_header);
	header.version = proto_version;
	header.message = message;
	header.tag = tag;
	if (payload && (payload_size > 0)) {
		header.length += payload_size;
	}
	int sent = socket_send(sfd, &header, sizeof(header));
	if (sent != sizeof(header)) {
		LIBUSBMUXD_DEBUG(1, "%s: ERROR: could not send packet header\n", __func__);
		return -1;
	}
	if (payload && (payload_size > 0)) {
		uint32_t ssize = 0;
		do {
			int res = socket_send(sfd, (char*)payload + ssize, payload_size - ssize);
			if (res < 0) {
				break;
			}
			ssize += res;
		} while (ssize < payload_size);
		sent += ssize;
	}
	if (sent != (int)header.length) {
		LIBUSBMUXD_DEBUG(1, "%s: ERROR: could not send whole packet (sent %d of %d)\n", __func__, sent, header.length);
		socket_close(sfd);
		return -1;
	}
	return sent;
}

static int send_plist_packet(int sfd, uint32_t tag, plist_t message)
{
	int res;
	char *payload = NULL;
	uint32_t payload_size = 0;

	plist_to_xml(message, &payload, &payload_size);
	res = send_packet(sfd, MESSAGE_PLIST, tag, payload, payload_size);
	free(payload);

	return res;
}

static void get_bundle_id()
{
#if defined (__APPLE__)
	char CONTENTS_INFO_PLIST[] = "Contents/Info.plist";
	char* execpath = malloc(1024);
	uint32_t size = 1024;
	if (_NSGetExecutablePath(execpath, &size) != 0) {
		free(execpath);
		return;
	}
	// strip off executable name
	char *p = execpath + strlen(execpath) - 1;
	while (p > execpath && *p != '/') p--;
	if (*p == '/') *p = '\0';
	// now walk back trying to find "/Contents/MacOS", and strip it off
	int macos_found = 0;
	while (p > execpath) {
		p--;
		if (*p != '/') continue;
		if (strcmp(p, "/.") == 0) {
			*p = '\0';
		} else if (!macos_found && strcmp(p, "/MacOS") == 0) {
			*p = '\0';
			macos_found++;
		} else if (macos_found && strcmp(p, "/Contents") == 0) {
			*p = '\0';
			break;
		} else {
			break;
		}
	}
	// now just append "/Contents/Info.plist"
	size_t len = strlen(execpath) + sizeof(CONTENTS_INFO_PLIST) + 1;
	char *infopl = malloc(len);
	snprintf(infopl, len, "%s/%s", execpath, CONTENTS_INFO_PLIST);
	free(execpath);
	struct stat fst;
	fst.st_size = 0;
	if (stat(infopl, &fst) != 0) {
		free(infopl);
		return;
	}
	size_t fsize = fst.st_size;
	if (fsize < 8) {
		free(infopl);
		return;
	}
	FILE *f = fopen(infopl, "r");
	free(infopl);
	if (!f)
		return;
	char *buf = malloc(fsize);
	if (!buf)
		return;
	if (fread(buf, 1, fsize, f) == fsize) {
		plist_t pl = NULL;
		if (memcmp(buf, "bplist00", 8) == 0) {
			plist_from_bin(buf, fst.st_size, &pl);
		} else {
			plist_from_xml(buf, fst.st_size, &pl);
		}
		if (pl) {
			plist_t bid = plist_dict_get_item(pl, "CFBundleIdentifier");
			if (plist_get_node_type(bid) == PLIST_STRING) {
				plist_get_string_val(bid, &bundle_id);
			}
			plist_free(pl);
		}
	}
	free(buf);
	fclose(f);
#endif
}

static void get_prog_name()
{
#if defined(__APPLE__) || defined(__FreeBSD__)
	const char *pname = getprogname();
	if (pname) {
		prog_name = strdup(pname);
	}
#elif defined (WIN32)
	TCHAR *_pname = malloc((MAX_PATH+1) * sizeof(TCHAR));
	if (GetModuleFileName(NULL, _pname, MAX_PATH+1) > 0) {
		char* pname = NULL;
	#if defined(UNICODE) || defined(_UNICODE)
		char* __pname = NULL;
		int l = WideCharToMultiByte(CP_UTF8, 0, _pname, -1, NULL, 0, NULL, NULL);
		if (l > 0) {
			__pname = malloc(l);
			if (WideCharToMultiByte(CP_UTF8, 0, _pname, -1, __pname, l, NULL, NULL) > 0) {
				pname = __pname;
			}
		}
	#else
		pname = _pname;
	#endif
		if (pname) {
			char *p = pname+strlen(pname)-1;
			while (p > pname && *p != '\\' && *p != '/') p--;
			if (*p == '\\' || *p == '/') p++;
			prog_name = strdup(p);
		}
	#if defined(UNICODE) || defined(_UNICODE)
		free(__pname);
	#endif
	}
	free(_pname);
#elif defined (HAVE_PROGRAM_INVOCATION_SHORT_NAME)
	char *pname = program_invocation_short_name;
	if (pname) {
		prog_name = strdup(pname);
	}
#elif defined (__linux__)
	FILE *f = fopen("/proc/self/stat", "r");
	if (!f) {
		return;
	}
	char *tmpbuf = malloc(512);
	size_t r = fread(tmpbuf, 1, 512, f);
	if (r > 0) {
		char *p = tmpbuf;
		while ((p-tmpbuf < r) && (*p != '(') && (*p != '\0')) p++;
		if (*p == '(') {
			p++;
			char *pname = p;
			while ((p-tmpbuf < r) && (*p != ')') && (*p != '\0')) p++;
			if (*p == ')') {
				*p = '\0';
				prog_name = strdup(pname);
			}
		}
	}
	free(tmpbuf);
	fclose(f);
#else
	#warning FIXME: no method to determine program name
#endif
}

static plist_t create_plist_message(const char* message_type)
{
	if (!bundle_id) {
		get_bundle_id();
	}
	if (!prog_name) {
		get_prog_name();
	}
	plist_t plist = plist_new_dict();
	if (bundle_id) {
		plist_dict_set_item(plist, "BundleID", plist_new_string(bundle_id));
	}
	plist_dict_set_item(plist, "ClientVersionString", plist_new_string(PLIST_CLIENT_VERSION_STRING));
	plist_dict_set_item(plist, "MessageType", plist_new_string(message_type));
	if (prog_name) {
		plist_dict_set_item(plist, "ProgName", plist_new_string(prog_name));
	}
	plist_dict_set_item(plist, "kLibUSBMuxVersion", plist_new_uint(PLIST_LIBUSBMUX_VERSION));
	return plist;
}

static int send_listen_packet(int sfd, uint32_t tag)
{
	int res = 0;
	if (proto_version == 1) {
		/* construct message plist */
		plist_t plist = create_plist_message("Listen");

		res = send_plist_packet(sfd, tag, plist);
		plist_free(plist);
	} else {
		/* binary packet */
		res = send_packet(sfd, MESSAGE_LISTEN, tag, NULL, 0);
	}
	return res;
}

static int send_connect_packet(int sfd, uint32_t tag, uint32_t device_id, uint16_t port)
{
	int res = 0;
	if (proto_version == 1) {
		/* construct message plist */
		plist_t plist = create_plist_message("Connect");
		plist_dict_set_item(plist, "DeviceID", plist_new_uint(device_id));
		plist_dict_set_item(plist, "PortNumber", plist_new_uint(htons(port)));

		res = send_plist_packet(sfd, tag, plist);
		plist_free(plist);
	} else {
		/* binary packet */
		struct {
			uint32_t device_id;
			uint16_t port;
			uint16_t reserved;
		} conninfo;

		conninfo.device_id = device_id;
		conninfo.port = htons(port);
		conninfo.reserved = 0;

		res = send_packet(sfd, MESSAGE_CONNECT, tag, &conninfo, sizeof(conninfo));
	}
	return res;
}

static int send_list_devices_packet(int sfd, uint32_t tag)
{
	int res = -1;

	/* construct message plist */
	plist_t plist = create_plist_message("ListDevices");

	res = send_plist_packet(sfd, tag, plist);
	plist_free(plist);

	return res;
}

static int send_read_buid_packet(int sfd, uint32_t tag)
{
	int res = -1;

	/* construct message plist */
	plist_t plist = create_plist_message("ReadBUID");

	res = send_plist_packet(sfd, tag, plist);
	plist_free(plist);

	return res;
}

static int send_pair_record_packet(int sfd, uint32_t tag, const char* msgtype, const char* pair_record_id, uint32_t device_id, plist_t data)
{
	int res = -1;

	/* construct message plist */
	plist_t plist = create_plist_message(msgtype);
	plist_dict_set_item(plist, "PairRecordID", plist_new_string(pair_record_id));
	if (data) {
		plist_dict_set_item(plist, "PairRecordData", plist_copy(data));
	}
	if (device_id > 0) {
		plist_dict_set_item(plist, "DeviceID", plist_new_uint(device_id));
	}

	res = send_plist_packet(sfd, tag, plist);
	plist_free(plist);

	return res;
}

/**
 * Generates an event, i.e. calls the callback function.
 * A reference to a populated usbmuxd_event_t with information about the event
 * and the corresponding device will be passed to the callback function.
 */
static void generate_event(const usbmuxd_device_info_t *dev, enum usbmuxd_event_type event)
{
	usbmuxd_event_t ev;

	if (!dev) {
		return;
	}

	ev.event = event;
	memcpy(&ev.device, dev, sizeof(usbmuxd_device_info_t));

	mutex_lock(&listener_mutex);
	FOREACH(struct usbmuxd_subscription_context* context, &listeners) {
		context->callback(&ev, context->user_data);
	} ENDFOREACH
	mutex_unlock(&listener_mutex);
}

static int usbmuxd_listen_poll()
{
	int sfd;

	sfd = connect_usbmuxd_socket();
	if (sfd < 0) {
		while (1) {
			mutex_lock(&listener_mutex);
			int num = collection_count(&listeners);
			mutex_unlock(&listener_mutex);
			if (num <= 0) {
				break;
			}
			if ((sfd = connect_usbmuxd_socket()) >= 0) {
				break;
			}
			sleep(1);
		}
	}

	return sfd;
}

#ifdef HAVE_INOTIFY
#ifndef HAVE_PSELECT
static int pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timespec *timeout, const sigset_t *sigmask)
{
	int ready;
	struct timeval tv;
	struct timeval *p_timeout;
	sigset_t origmask;

	if (timeout) {
		tv.tv_sec = timeout->tv_sec;
		tv.tv_usec = timeout->tv_nsec / 1000;
		p_timeout = &tv;
	} else {
		p_timeout = NULL;
	}

	pthread_sigmask(SIG_SETMASK, sigmask, &origmask);
	ready = select(nfds, readfds, writefds, exceptfds, p_timeout);
	pthread_sigmask(SIG_SETMASK, &origmask, NULL);

	return ready;
}
#endif

static int usbmuxd_listen_inotify()
{
	int inot_fd;
	int watch_d;
	int sfd;

	if (!use_inotify) {
		return -2;
	}

	sfd = connect_usbmuxd_socket();
	if (sfd >= 0)
		return sfd;

	sfd = -1;
	inot_fd = inotify_init ();
	if (inot_fd < 0) {
		LIBUSBMUXD_DEBUG(1, "%s: Failed to setup inotify\n", __func__);
		return -2;
	}

	/* inotify is setup, listen for events that concern us */
	watch_d = inotify_add_watch (inot_fd, USBMUXD_DIRNAME, IN_CREATE);
	if (watch_d < 0) {
		LIBUSBMUXD_DEBUG(1, "%s: Failed to setup watch descriptor for socket dir\n", __func__);
		close (inot_fd);
		return -2;
	}

	while (1) {
		fd_set rfds;
		struct timespec tv = {1, 0};

		FD_ZERO(&rfds);
		FD_SET(inot_fd, &rfds);
		int r = pselect(inot_fd+1, &rfds, NULL, NULL, &tv, NULL);
		if (r < 0) {
			break;
		} else if (r == 0) {
			continue;
		}

		ssize_t len, i;
		char buff[EVENT_BUF_LEN] = {0};

		i = 0;
		len = read (inot_fd, buff, EVENT_BUF_LEN -1);
		if (len < 0)
			goto end;
		while (i < len) {
			struct inotify_event *pevent = (struct inotify_event *) & buff[i];

			/* check that it's ours */
			if (pevent->mask & IN_CREATE &&
				pevent->len &&
				pevent->name[0] != 0 &&
				strcmp(pevent->name, USBMUXD_SOCKET_NAME) == 0) {
				/* retry if usbmuxd isn't ready yet */
				int retry = 10;
				while (--retry >= 0) {
					if ((sfd = connect_usbmuxd_socket ()) >= 0) {
						break;
					}
					sleep(1);
				}
				goto end;
			}
			i += EVENT_SIZE + pevent->len;
		}
	}

end:
	inotify_rm_watch(inot_fd, watch_d);
	close(inot_fd);

	return sfd;
}
#endif /* HAVE_INOTIFY */

/**
 * Tries to connect to usbmuxd and wait if it is not running.
 */
static int usbmuxd_listen()
{
	int sfd;
	uint32_t res = -1;
	int tag;

retry:

#ifdef HAVE_INOTIFY
	sfd = usbmuxd_listen_inotify();
	if (sfd == -2)
		sfd = usbmuxd_listen_poll();
#else
	sfd = usbmuxd_listen_poll();
#endif

	if (sfd < 0) {
		if (!cancelling) {
			LIBUSBMUXD_DEBUG(1, "%s: ERROR: usbmuxd was supposed to be running here...\n", __func__);
		}
		return sfd;
	}

	tag = ++use_tag;
	if (send_listen_packet(sfd, tag) <= 0) {
		LIBUSBMUXD_DEBUG(1, "%s: ERROR: could not send listen packet\n", __func__);
		socket_close(sfd);
		return -1;
	}
	if ((usbmuxd_get_result(sfd, tag, &res, NULL) == 1) && (res != 0)) {
		socket_close(sfd);
		if ((res == RESULT_BADVERSION) && (proto_version == 1)) {
			proto_version = 0;
			goto retry;
		}
		LIBUSBMUXD_DEBUG(1, "%s: ERROR: did not get OK but %d\n", __func__, res);
		return -1;
	}
	return sfd;
}

/**
 * Waits for an event to occur, i.e. a packet coming from usbmuxd.
 * Calls generate_event to pass the event via callback to the client program.
 */
static int get_next_event(int sfd)
{
	struct usbmuxd_header hdr;
	void *payload = NULL;

	/* block until we receive something */
	if (receive_packet(sfd, &hdr, &payload, 0) < 0) {
		if (!cancelling) {
			LIBUSBMUXD_DEBUG(1, "%s: Error in usbmuxd connection, disconnecting all devices!\n", __func__);
		}
		// when then usbmuxd connection fails,
		// generate remove events for every device that
		// is still present so applications know about it
		FOREACH(usbmuxd_device_info_t *dev, &devices) {
			generate_event(dev, UE_DEVICE_REMOVE);
			collection_remove(&devices, dev);
			free(dev);
		} ENDFOREACH
		return -EIO;
	}

	if ((hdr.length > sizeof(hdr)) && !payload) {
		LIBUSBMUXD_DEBUG(1, "%s: Invalid packet received, payload is missing!\n", __func__);
		return -EBADMSG;
	}

	if (hdr.message == MESSAGE_DEVICE_ADD) {
		usbmuxd_device_info_t *devinfo = (usbmuxd_device_info_t*)payload;
		collection_add(&devices, devinfo);
		generate_event(devinfo, UE_DEVICE_ADD);
		payload = NULL;
	} else if (hdr.message == MESSAGE_DEVICE_REMOVE) {
		uint32_t handle;
		usbmuxd_device_info_t *devinfo;

		memcpy(&handle, payload, sizeof(uint32_t));

		devinfo = devices_find(handle);
		if (!devinfo) {
			LIBUSBMUXD_DEBUG(1, "%s: WARNING: got device remove message for handle %d, but couldn't find the corresponding handle in the device list. This event will be ignored.\n", __func__, handle);
		} else {
			generate_event(devinfo, UE_DEVICE_REMOVE);
			collection_remove(&devices, devinfo);
			free(devinfo);
		}
	} else if (hdr.message == MESSAGE_DEVICE_PAIRED) {
		uint32_t handle;
		usbmuxd_device_info_t *devinfo;

		memcpy(&handle, payload, sizeof(uint32_t));

		devinfo = devices_find(handle);
		if (!devinfo) {
			LIBUSBMUXD_DEBUG(1, "%s: WARNING: got paired message for device handle %d, but couldn't find the corresponding handle in the device list. This event will be ignored.\n", __func__, handle);
		} else {
			generate_event(devinfo, UE_DEVICE_PAIRED);
		}
	} else if (hdr.length > 0) {
		LIBUSBMUXD_DEBUG(1, "%s: Unexpected message type %d length %d received!\n", __func__, hdr.message, hdr.length);
	}
	free(payload);
	return 0;
}

static void device_monitor_cleanup(void* data)
{
	FOREACH(usbmuxd_device_info_t *dev, &devices) {
		collection_remove(&devices, dev);
		free(dev);
	} ENDFOREACH
	collection_free(&devices);

	socket_close(listenfd);
	listenfd = -1;
}

/**
 * Device Monitor thread function.
 *
 * This function sets up a connection to usbmuxd
 */
static void *device_monitor(void *data)
{
	running = 1;
	collection_init(&devices);
	cancelling = 0;

#ifdef HAVE_THREAD_CLEANUP
	thread_cleanup_push(device_monitor_cleanup, NULL);
#endif
	do {

		listenfd = usbmuxd_listen();
		if (listenfd < 0) {
			continue;
		}

		while (running) {
			int res = get_next_event(listenfd);
			if (res < 0) {
				break;
			}
		}

		mutex_lock(&listener_mutex);
		if (collection_count(&listeners) == 0) {
			running = 0;
		}
		mutex_unlock(&listener_mutex);
	} while (running);

#ifdef HAVE_THREAD_CLEANUP
	thread_cleanup_pop(1);
#else
	device_monitor_cleanup(NULL);
#endif

	return NULL;
}

static void init_listeners(void)
{
	collection_init(&listeners);
	mutex_init(&listener_mutex);
}

USBMUXD_API int usbmuxd_events_subscribe(usbmuxd_subscription_context_t *ctx, usbmuxd_event_cb_t callback, void *user_data)
{
	if (!ctx || !callback) {
		return -EINVAL;
	}

	thread_once(&listener_init_once, init_listeners);

	mutex_lock(&listener_mutex);
	*ctx = malloc(sizeof(struct usbmuxd_subscription_context));
	if (!*ctx) {
		mutex_unlock(&listener_mutex);
		LIBUSBMUXD_ERROR("ERROR: %s: malloc failed\n", __func__);
		return -ENOMEM;
	}
	(*ctx)->callback = callback;
	(*ctx)->user_data = user_data;

	collection_add(&listeners, *ctx);

	if (devmon == THREAD_T_NULL || !thread_alive(devmon)) {
		mutex_unlock(&listener_mutex);
		int res = thread_new(&devmon, device_monitor, NULL);
		if (res != 0) {
			free(*ctx);
			LIBUSBMUXD_DEBUG(1, "%s: ERROR: Could not start device watcher thread!\n", __func__);
			return res;
		}
	} else {
		/* we need to submit DEVICE_ADD events to the new listener */
		FOREACH(usbmuxd_device_info_t *dev, &devices) {
			if (dev) {
				usbmuxd_event_t ev;
				ev.event = UE_DEVICE_ADD;
				memcpy(&ev.device, dev, sizeof(usbmuxd_device_info_t));
				(*ctx)->callback(&ev, (*ctx)->user_data);
			}
		} ENDFOREACH
		mutex_unlock(&listener_mutex);
	}

	return 0;
}

USBMUXD_API int usbmuxd_events_unsubscribe(usbmuxd_subscription_context_t ctx)
{
	int ret = 0;
	int num = 0;

	if (!ctx) {
		return -EINVAL;
	}

	mutex_lock(&listener_mutex);
	if (collection_remove(&listeners, ctx) == 0) {
		FOREACH(usbmuxd_device_info_t *dev, &devices) {
			if (dev) {
				usbmuxd_event_t ev;
				ev.event = UE_DEVICE_REMOVE;
				memcpy(&ev.device, dev, sizeof(usbmuxd_device_info_t));
				(ctx)->callback(&ev, (ctx)->user_data);
			}
		} ENDFOREACH
		free(ctx);
	}
	num = collection_count(&listeners);
	mutex_unlock(&listener_mutex);

	if (num == 0) {
		int res = 0;
		cancelling = 1;
		socket_shutdown(listenfd, SHUT_RDWR);
		if (thread_alive(devmon)) {
			if (thread_cancel(devmon) < 0) {
				running = 0;
			}
#if defined(HAVE_INOTIFY) && !defined(HAVE_PTHREAD_CANCEL)
			pthread_kill(devmon, SIGINT);
#endif
			res = thread_join(devmon);
			thread_free(devmon);
			devmon = THREAD_T_NULL;
		}
		if ((res != 0) && (res != ESRCH)) {
			ret = res;
		}
	}

	return ret;
}

USBMUXD_API int usbmuxd_subscribe(usbmuxd_event_cb_t callback, void *user_data)
{
	if (!callback) {
		return -EINVAL;
	}

	if (event_ctx) {
		usbmuxd_events_unsubscribe(event_ctx);
		event_ctx = NULL;
	}
	return usbmuxd_events_subscribe(&event_ctx, callback, user_data);
}

USBMUXD_API int usbmuxd_unsubscribe(void)
{
	int res = usbmuxd_events_unsubscribe(event_ctx);
	event_ctx = NULL;
	return res;
}

USBMUXD_API int usbmuxd_get_device_list(usbmuxd_device_info_t **device_list)
{
	int sfd;
	int tag;
	int listen_success = 0;
	uint32_t res;
	struct collection tmpdevs;
	usbmuxd_device_info_t *newlist = NULL;
	struct usbmuxd_header hdr;
	int dev_cnt = 0;
	void *payload = NULL;

	*device_list = NULL;

retry:
	sfd = connect_usbmuxd_socket();
	if (sfd < 0) {
		LIBUSBMUXD_DEBUG(1, "%s: error opening socket!\n", __func__);
		return sfd;
	}

	tag = ++use_tag;
	if ((proto_version == 1) && (try_list_devices)) {
		if (send_list_devices_packet(sfd, tag) > 0) {
			plist_t list = NULL;
			if ((usbmuxd_get_result(sfd, tag, &res, &list) == 1) && (res == 0)) {
				plist_t devlist = plist_dict_get_item(list, "DeviceList");
				if (devlist && plist_get_node_type(devlist) == PLIST_ARRAY) {
					collection_init(&tmpdevs);
					uint32_t numdevs = plist_array_get_size(devlist);
					uint32_t i;
					for (i = 0; i < numdevs; i++) {
						plist_t pdev = plist_array_get_item(devlist, i);
						plist_t props = plist_dict_get_item(pdev, "Properties");
						usbmuxd_device_info_t *devinfo = device_info_from_plist(props);
						if (!devinfo) {
							socket_close(sfd);
							LIBUSBMUXD_DEBUG(1, "%s: Could not create device info object from properties!\n", __func__);
							plist_free(list);
							return -1;
						}
						collection_add(&tmpdevs, devinfo);
					}
					plist_free(list);
					goto got_device_list;
				}
			} else {
				if (res == RESULT_BADVERSION) {
					proto_version = 0;
				}
				socket_close(sfd);
				try_list_devices = 0;
				plist_free(list);
				goto retry;
			}
			plist_free(list);
		}
	}

	tag = ++use_tag;
	if (send_listen_packet(sfd, tag) > 0) {
		res = -1;
		// get response
		if ((usbmuxd_get_result(sfd, tag, &res, NULL) == 1) && (res == 0)) {
			listen_success = 1;
		} else {
			socket_close(sfd);
			if ((res == RESULT_BADVERSION) && (proto_version == 1)) {
				proto_version = 0;
				goto retry;
			}
			LIBUSBMUXD_DEBUG(1, "%s: Did not get response to scan request (with result=0)...\n", __func__);
			return res;
		}
	}

	if (!listen_success) {
		socket_close(sfd);
		LIBUSBMUXD_DEBUG(1, "%s: Could not send listen request!\n", __func__);
		return -1;
	}

	collection_init(&tmpdevs);

	// receive device list
	while (1) {
		if (receive_packet(sfd, &hdr, &payload, 100) > 0) {
			if (hdr.message == MESSAGE_DEVICE_ADD) {
				usbmuxd_device_info_t *devinfo = payload;
				collection_add(&tmpdevs, devinfo);
				payload = NULL;
			} else if (hdr.message == MESSAGE_DEVICE_REMOVE) {
				uint32_t handle;
				usbmuxd_device_info_t *devinfo = NULL;

				memcpy(&handle, payload, sizeof(uint32_t));

				FOREACH(usbmuxd_device_info_t *di, &tmpdevs) {
					if (di && di->handle == handle) {
						devinfo = di;
						break;
					}
				} ENDFOREACH
				if (devinfo) {
					collection_remove(&tmpdevs, devinfo);
					free(devinfo);
				}
			} else {
				LIBUSBMUXD_DEBUG(1, "%s: Unexpected message %d\n", __func__, hdr.message);
			}
			free(payload);
		} else {
			// we _should_ have all of them now.
			// or perhaps an error occured.
			break;
		}
	}

got_device_list:

	// explicitly close connection
	socket_close(sfd);

	// create copy of device info entries from collection
	newlist = (usbmuxd_device_info_t*)malloc(sizeof(usbmuxd_device_info_t) * (collection_count(&tmpdevs) + 1));
	dev_cnt = 0;
	FOREACH(usbmuxd_device_info_t *di, &tmpdevs) {
		if (di) {
			memcpy(&newlist[dev_cnt], di, sizeof(usbmuxd_device_info_t));
			free(di);
			dev_cnt++;
		}
	} ENDFOREACH
	collection_free(&tmpdevs);

	memset(&newlist[dev_cnt], 0, sizeof(usbmuxd_device_info_t));
	*device_list = newlist;

	return dev_cnt;
}

USBMUXD_API int usbmuxd_device_list_free(usbmuxd_device_info_t **device_list)
{
	if (device_list) {
		free(*device_list);
	}
	return 0;
}

USBMUXD_API int usbmuxd_get_device_by_udid(const char *udid, usbmuxd_device_info_t *device)
{
	usbmuxd_device_info_t *dev_list = NULL;
	usbmuxd_device_info_t *dev = NULL;
	int result = 0;
	int i;

	if (!device) {
		return -EINVAL;
	}
	if (usbmuxd_get_device_list(&dev_list) < 0) {
		return -ENODEV;
	}

	for (i = 0; dev_list[i].handle > 0; i++) {
	 	if (!udid) {
			if (dev_list[i].conn_type == CONNECTION_TYPE_USB) {
				dev = &dev_list[i];
				break;
			}
		} else if (!strcmp(udid, dev_list[i].udid)) {
			if (dev_list[i].conn_type == CONNECTION_TYPE_USB) {
				dev = &dev_list[i];
				break;
			}
		}
	}

	if (dev) {
		device->handle = dev->handle;
		device->product_id = dev->product_id;
		char *t = stpncpy(device->udid, dev->udid, sizeof(device->udid)-1);
		*t = '\0';
		device->conn_type = dev->conn_type;
		memcpy(device->conn_data, dev->conn_data, sizeof(device->conn_data));
		result = 1;
	}

	usbmuxd_device_list_free(&dev_list);

	return result;
}

USBMUXD_API int usbmuxd_get_device(const char *udid, usbmuxd_device_info_t *device, enum usbmux_lookup_options options)
{
	usbmuxd_device_info_t *dev_list = NULL;
	usbmuxd_device_info_t *dev_network = NULL;
	usbmuxd_device_info_t *dev_usbmuxd = NULL;
	usbmuxd_device_info_t *dev = NULL;
	int result = 0;
	int i;

	if (!device) {
		return -EINVAL;
	}
	if (usbmuxd_get_device_list(&dev_list) < 0) {
		return -ENODEV;
	}

	if (options == 0) {
		options = DEVICE_LOOKUP_USBMUX;
	}

	for (i = 0; dev_list[i].handle > 0; i++) {
		if (!udid) {
			if ((options & DEVICE_LOOKUP_USBMUX) && (dev_list[i].conn_type == CONNECTION_TYPE_USB)) {
				dev_usbmuxd = &dev_list[i];
				break;
			} else if ((options & DEVICE_LOOKUP_NETWORK) && (dev_list[i].conn_type == CONNECTION_TYPE_NETWORK)) {
				dev_network = &dev_list[i];
				break;
			}
		} else if (!strcmp(udid, dev_list[i].udid)) {
			if ((options & DEVICE_LOOKUP_USBMUX) && (dev_list[i].conn_type == CONNECTION_TYPE_USB)) {
				dev_usbmuxd = &dev_list[i];
			} else if ((options & DEVICE_LOOKUP_NETWORK) && (dev_list[i].conn_type == CONNECTION_TYPE_NETWORK)) {
				dev_network = &dev_list[i];
			}
		}
		if (dev_usbmuxd && dev_network) {
			break;
		}
	}

	if (dev_network && dev_usbmuxd) {
		dev = (options & DEVICE_LOOKUP_PREFER_NETWORK) ? dev_network : dev_usbmuxd;
	} else if (dev_network) {
		dev = dev_network;
	} else if (dev_usbmuxd) {
		dev = dev_usbmuxd;
	}

	if (dev) {
		device->handle = dev->handle;
		device->product_id = dev->product_id;
		char *t = stpncpy(device->udid, dev->udid, sizeof(device->udid)-1);
		*t = '\0';
		device->conn_type = dev->conn_type;
		memcpy(device->conn_data, dev->conn_data, sizeof(device->conn_data));
		result = 1;
	}

	free(dev_list);

	return result;
}

USBMUXD_API int usbmuxd_connect(const uint32_t handle, const unsigned short port)
{
	int sfd;
	int tag;
	int connected = 0;
	int result = EBADF;

retry:
	sfd = connect_usbmuxd_socket();
	if (sfd < 0) {
		result = errno;
		LIBUSBMUXD_DEBUG(1, "%s: Error: Connection to usbmuxd failed: %s\n", __func__, strerror(result));
		return -result;
	}

	tag = ++use_tag;
	if (send_connect_packet(sfd, tag, (uint32_t)handle, (uint16_t)port) <= 0) {
		LIBUSBMUXD_DEBUG(1, "%s: Error sending connect message!\n", __func__);
	} else {
		// read ACK
		uint32_t res = -1;
		LIBUSBMUXD_DEBUG(2, "%s: Reading connect result...\n", __func__);
		if (usbmuxd_get_result(sfd, tag, &res, NULL) == 1) {
			if (res == 0) {
				LIBUSBMUXD_DEBUG(2, "%s: Connect success!\n", __func__);
				connected = 1;
			} else {
				if ((res == RESULT_BADVERSION) && (proto_version == 1)) {
					proto_version = 0;
					socket_close(sfd);
					goto retry;
				}
				LIBUSBMUXD_DEBUG(1, "%s: Connect failed, Error code=%d\n", __func__, res);
				if (res == RESULT_CONNREFUSED) {
					result = ECONNREFUSED;
				} else if (res == RESULT_BADDEV) {
					result = ENODEV;
				} else {
					result = EBADF;
				}
			}
		}
	}

	if (connected) {
		return sfd;
	}

	socket_close(sfd);

	return -result;
}

USBMUXD_API int usbmuxd_disconnect(int sfd)
{
	return socket_close(sfd);
}

USBMUXD_API int usbmuxd_send(int sfd, const char *data, uint32_t len, uint32_t *sent_bytes)
{
	int num_sent;

	if (sfd < 0) {
		return -EINVAL;
	}

	num_sent = socket_send(sfd, (void*)data, len);
	if (num_sent < 0) {
		*sent_bytes = 0;
		num_sent = errno;
		LIBUSBMUXD_DEBUG(1, "%s: Error %d when sending: %s\n", __func__, num_sent, strerror(num_sent));
		return -num_sent;
	} else if ((uint32_t)num_sent < len) {
		LIBUSBMUXD_DEBUG(1, "%s: Warning: Did not send enough (only %d of %d)\n", __func__, num_sent, len);
	}

	*sent_bytes = num_sent;

	return 0;
}

USBMUXD_API int usbmuxd_recv_timeout(int sfd, char *data, uint32_t len, uint32_t *recv_bytes, unsigned int timeout)
{
	int num_recv = socket_receive_timeout(sfd, (void*)data, len, 0, timeout);
	if (num_recv < 0) {
		*recv_bytes = 0;
		return num_recv;
	}

	*recv_bytes = num_recv;

	return 0;
}

USBMUXD_API int usbmuxd_recv(int sfd, char *data, uint32_t len, uint32_t *recv_bytes)
{
	return usbmuxd_recv_timeout(sfd, data, len, recv_bytes, 5000);
}

USBMUXD_API int usbmuxd_read_buid(char **buid)
{
	int sfd;
	int tag;
	int ret = -1;

	if (!buid) {
		return -EINVAL;
	}
	*buid = NULL;

	sfd = connect_usbmuxd_socket();
	if (sfd < 0) {
		LIBUSBMUXD_DEBUG(1, "%s: Error: Connection to usbmuxd failed: %s\n", __func__, strerror(errno));
		return sfd;
	}

	proto_version = 1;
	tag = ++use_tag;
	if (send_read_buid_packet(sfd, tag) <= 0) {
		LIBUSBMUXD_DEBUG(1, "%s: Error sending ReadBUID message!\n", __func__);
	} else {
		uint32_t rc = 0;
		plist_t pl = NULL;
		ret = usbmuxd_get_result(sfd, tag, &rc, &pl);
		if ((ret == 1) && (rc == 0)) {
			plist_t node = plist_dict_get_item(pl, "BUID");
			if (node && plist_get_node_type(node) == PLIST_STRING) {
				plist_get_string_val(node, buid);
			}
			ret = 0;
		} else if (ret == 1) {
			ret = -(int)rc;
		}
		plist_free(pl);
	}
	socket_close(sfd);

	return ret;
}

USBMUXD_API int usbmuxd_read_pair_record(const char* record_id, char **record_data, uint32_t *record_size)
{
	int sfd;
	int tag;
	int ret = -1;

	if (!record_id || !record_data || !record_size) {
		return -EINVAL;
	}
	*record_data = NULL;
	*record_size = 0;

	sfd = connect_usbmuxd_socket();
	if (sfd < 0) {
		LIBUSBMUXD_DEBUG(1, "%s: Error: Connection to usbmuxd failed: %s\n",
				__func__, strerror(errno));
		return sfd;
	}

	proto_version = 1;
	tag = ++use_tag;

	if (send_pair_record_packet(sfd, tag, "ReadPairRecord", record_id, 0, NULL) <= 0) {
		LIBUSBMUXD_DEBUG(1, "%s: Error sending ReadPairRecord message!\n", __func__);
	} else {
		uint32_t rc = 0;
		plist_t pl = NULL;
		ret = usbmuxd_get_result(sfd, tag, &rc, &pl);
		if ((ret == 1) && (rc == 0)) {
			plist_t node = plist_dict_get_item(pl, "PairRecordData");
			if (node && plist_get_node_type(node) == PLIST_DATA) {
				uint64_t int64val = 0;
				plist_get_data_val(node, record_data, &int64val);
				if (*record_data && int64val > 0) {
					*record_size = (uint32_t)int64val;
					ret = 0;
				}
			}
		} else if (ret == 1) {
			ret = -(int)rc;
		}
		plist_free(pl);
	}
	socket_close(sfd);

	return ret;
}

USBMUXD_API int usbmuxd_save_pair_record_with_device_id(const char* record_id, uint32_t device_id, const char *record_data, uint32_t record_size)
{
	int sfd;
	int tag;
	int ret = -1;

	if (!record_id || !record_data || !record_size) {
		return -EINVAL;
	}

	sfd = connect_usbmuxd_socket();
	if (sfd < 0) {
		LIBUSBMUXD_DEBUG(1, "%s: Error: Connection to usbmuxd failed: %s\n",
				__func__, strerror(errno));
		return sfd;
	}

	proto_version = 1;
	tag = ++use_tag;

	plist_t data = plist_new_data(record_data, record_size);
	if (send_pair_record_packet(sfd, tag, "SavePairRecord", record_id, device_id, data) <= 0) {
		LIBUSBMUXD_DEBUG(1, "%s: Error sending SavePairRecord message!\n", __func__);
	} else {
		uint32_t rc = 0;
		ret = usbmuxd_get_result(sfd, tag, &rc, NULL);
		if ((ret == 1) && (rc == 0)) {
			ret = 0;
		} else if (ret == 1) {
			ret = -(int)rc;
			LIBUSBMUXD_DEBUG(1, "%s: Error: saving pair record failed: %d\n", __func__, ret);
		}
	}
	plist_free(data);
	socket_close(sfd);

	return ret;
}

USBMUXD_API int usbmuxd_save_pair_record(const char* record_id, const char *record_data, uint32_t record_size)
{
	return usbmuxd_save_pair_record_with_device_id(record_id, 0, record_data, record_size);
}

USBMUXD_API int usbmuxd_delete_pair_record(const char* record_id)
{
	int sfd;
	int tag;
	int ret = -1;

	if (!record_id) {
		return -EINVAL;
	}

	sfd = connect_usbmuxd_socket();
	if (sfd < 0) {
		LIBUSBMUXD_DEBUG(1, "%s: Error: Connection to usbmuxd failed: %s\n",
				__func__, strerror(errno));
		return sfd;
	}

	proto_version = 1;
	tag = ++use_tag;

	if (send_pair_record_packet(sfd, tag, "DeletePairRecord", record_id, 0, NULL) <= 0) {
		LIBUSBMUXD_DEBUG(1, "%s: Error sending DeletePairRecord message!\n", __func__);
	} else {
		uint32_t rc = 0;
		ret = usbmuxd_get_result(sfd, tag, &rc, NULL);
		if ((ret == 1) && (rc == 0)) {
			ret = 0;
		} else if (ret == 1) {
			ret = -(int)rc;
			LIBUSBMUXD_DEBUG(1, "%s: Error: deleting pair record failed: %d\n", __func__, ret);
		}
	}
	socket_close(sfd);

	return ret;
}

USBMUXD_API void libusbmuxd_set_use_inotify(int set)
{
#ifdef HAVE_INOTIFY
	use_inotify = set;
#endif
	return;
}

USBMUXD_API void libusbmuxd_set_debug_level(int level)
{
	libusbmuxd_debug = level;
	socket_set_verbose(level);
}
