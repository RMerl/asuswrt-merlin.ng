/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2001-2024  CommScope, Inc
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * @file uri.h
 *
 * Uniform Resource Identifier (URI) utility functions
 *
 * Future specifications and related documentation should
 * use the general term "URI" rather than the more restrictive terms
 * "URL" and "URN" [RFC3305]
 *
 * @version $Id: //depot/nmd/exodus/pace/libs/libnetutil/include/libnetutil/uri.h#1 $
 */

#ifndef _LIBNETUTIL_URI_H_
#define _LIBNETUTIL_URI_H_ 1

#include <sys/types.h>

/* RFC4395 defines an IANA-maintained register of URI Schemes.
 *
 * See http://www.iana.org/assignments/uri-schemes.html
 */
enum nu_uri_e {
    URISCHEME_UNKNOWN,

    URISCHEME_HTTP,   /* See RFC2616, RFC3986 */
    URISCHEME_HTTPS,  /* See RFC2618, RFC3986 */
    URISCHEME_FTP,    /* See RFC1738 */
    URISCHEME_SFTP,   // SSH FTP
    URISCHEME_TELNET, /* See RFC1738, RFC4248 */
    URISCHEME_FILE,   /* See RFC1738 */
    URISCHEME_MAILTO, /* See RFC2368 */
    URISCHEME_LDAP,   /* See RFC1959, RFC2255, RFC4516 */
    URISCHEME_SIP,    /* See RFC3261 */
    URISCHEME_SIPS,   /* See RFC3261 */
    URISCHEME_TEL,    /* See RFC3966 */

    /* 2Wire schemes */
    URISCHEME_CIFS,
    URISCHEME_TWCSS,
    URISCHEME_RPC,
    URISCHEME_TWRPC,
    URISCHEME_TWRPCS,
};

typedef struct nu_uri_s {
    enum nu_uri_e u_type;

    const char *u_scheme;
    size_t u_schemesz;

    /* Per RFC 1738 Section 3.1. Common Internet Scheme Syntax
     *   //<user>:<password>@<host>:<port>/<url-path>
     *
     * Per RFC 2396 Section 3. URI Syntactic Components
     *     <scheme>:<scheme-specifc-part>
     *   This generic URI syntax considers of four main components
     *     <scheme>://<authority><path>?<query>
     *
     * Per RFC 3986 Section 3. Syntax Components
     *   URI         = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
     *
     *   hier-part   = "//" authority path-abempty
     *               / path-absolute
     *               / path-rootless
     *               / path-empty
     *
     * From the above RFCs, the following elements are parsed out
     * in a generic fashion.
     */

    /* Authority component */
    const char *u_user;
    size_t u_usersz;
    const char *u_password;
    size_t u_passwordsz;
    const char *u_host;
    size_t u_hostsz;
    const char *u_port;
    size_t u_portsz;

    /* Path component */
    const char *u_path; /* start of the path */
    size_t u_pathsz;

    /* Query component */
    const char *u_query;
    size_t u_querysz;

    /* Fragment component */
    const char *u_fragment;
    size_t u_fragmentsz;
} nu_uri_t;

/* Split the passed in string into its component parts */
int nu_uri_from_str(nu_uri_t *uri, const char *str);

/* Reassemble a string from it's split component parts */
int nu_uri_to_str(nu_uri_t *nu_uri, char *str, int len);


#endif /* !_LIBNETUTIL_URI_H_ */
