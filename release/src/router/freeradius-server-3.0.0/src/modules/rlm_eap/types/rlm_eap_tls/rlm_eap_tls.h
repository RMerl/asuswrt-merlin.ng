/*
 * rlm_eap_tls.h
 *
 * Version:     $Id$
 *
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
 *
 * Copyright 2001  hereUare Communications, Inc. <raghud@hereuare.com>
 * Copyright 2003  Alan DeKok <aland@freeradius.org>
 * Copyright 2006  The FreeRADIUS server project
 */
#ifndef _RLM_EAP_TLS_H
#define _RLM_EAP_TLS_H

RCSIDH(rlm_eap_tls_h, "$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>

#include "eap_tls.h"

typedef struct rlm_eap_tls_t {
	/*
	 *	TLS configuration
	 */
	char	*tls_conf_name;
	fr_tls_server_conf_t *tls_conf;

	/*
	 *	Virtual server for checking certificates
	 */
  	char	*virtual_server;
} rlm_eap_tls_t;

#endif /* _RLM_EAP_TLS_H */
