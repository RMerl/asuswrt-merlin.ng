/* Error code definitions
 *
 * Copyright (C) 2003-2004  Narcis Ilisei <inarcis2002@hotpop.com>
 * Copyright (C) 2010-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, visit the Free Software Foundation
 * website at http://www.gnu.org/licenses/gpl-2.0.html or write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#ifndef INADYN_ERROR_H_
#define INADYN_ERROR_H_

#define RC_OK                           0
#define RC_ERROR                        1
#define RC_INVALID_POINTER              2
#define RC_OUT_OF_MEMORY                3
#define RC_BUFFER_OVERFLOW              4
#define RC_PIDFILE_EXISTS_ALREADY       5

#define RC_TCP_SOCKET_CREATE_ERROR      10
#define RC_TCP_BAD_PARAMETER            11
#define RC_TCP_INVALID_REMOTE_ADDR      12
#define RC_TCP_CONNECT_FAILED           13
#define RC_TCP_SEND_ERROR               14
#define RC_TCP_RECV_ERROR               15

#define RC_TCP_OBJECT_NOT_INITIALIZED   16
#define RC_HTTP_OBJECT_NOT_INITIALIZED  22

#define RC_HTTPS_NO_TRUSTED_CA_STORE    31
#define RC_HTTPS_OUT_OF_MEMORY          32
#define RC_HTTPS_FAILED_CONNECT         33
#define RC_HTTPS_FAILED_GETTING_CERT    34
#define RC_HTTPS_SEND_ERROR             36
#define RC_HTTPS_RECV_ERROR             37
#define RC_HTTPS_SNI_ERROR              38
#define RC_HTTPS_INVALID_REQUEST        39

#define RC_DDNS_INVALID_CHECKIP_RSP     42
#define RC_DDNS_INVALID_OPTION          45
#define RC_DDNS_RSP_NOHOST              47
#define RC_DDNS_RSP_NOTOK               48
#define RC_DDNS_RSP_RETRY_LATER         49
#define RC_DDNS_RSP_AUTH_FAIL           50
#define RC_DDNS_RSP_TOO_FREQUENT        51

#define RC_OS_INVALID_IP_ADDRESS        61
#define RC_OS_FORK_FAILURE              62
#define RC_OS_CHANGE_PERSONA_FAILURE    63
#define RC_OS_INVALID_UID               64
#define RC_OS_INVALID_GID               65
#define RC_OS_INSTALL_SIGHANDLER_FAILED 66

#define RC_FILE_IO_ACCESS_ERROR         73
#define RC_FILE_IO_MISSING_FILE         74

#define RC_RESTART                      255

#define DO(fn)       { int rc = fn; if (rc) return rc; }
#define TRY(fn)      {     rc = fn; if (rc) break; }
#define ASSERT(cond) { if (!cond) return RC_INVALID_POINTER; }

const char *error_str(int rc);

#endif /* INADYN_ERROR_H_ */

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
