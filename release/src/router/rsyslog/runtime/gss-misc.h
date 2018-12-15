/* Definitions for gssutil class. This implements a session of the
 * plain TCP server.
 *
 * Copyright 2008 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of rsyslog.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *       -or-
 *       see COPYING.ASL20 in the source distribution
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef	GSS_MISC_H_INCLUDED
#define	GSS_MISC_H_INCLUDED 1

#include <gssapi/gssapi.h>
#include "obj.h"

/* interfaces */
BEGINinterface(gssutil) /* name must also be changed in ENDinterface macro! */
	int (*recv_token)(int s, gss_buffer_t tok);
	int (*send_token)(int s, gss_buffer_t tok);
	void (*display_status)(char *m, OM_uint32 maj_stat, OM_uint32 min_stat);
	void (*display_ctx_flags)(OM_uint32 flags);
ENDinterface(gssutil)
#define gssutilCURR_IF_VERSION 1 /* increment whenever you change the interface structure! */


/* prototypes */
PROTOTYPEObj(gssutil);

/* the name of our library binary */
#define LM_GSSUTIL_FILENAME "lmgssutil"

#endif /* #ifndef GSS_MISC_H_INCLUDED */
