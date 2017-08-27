/*
 * Copyright (c) Dan Harkins, 2012
 *
 *  Copyright holder grants permission for redistribution and use in source
 *  and binary forms, with or without modification, provided that the
 *  following conditions are met:
 *     1. Redistribution of source code must retain the above copyright
 *	notice, this list of conditions, and the following disclaimer
 *	in all source files.
 *     2. Redistribution in binary form must retain the above copyright
 *	notice, this list of conditions, and the following disclaimer
 *	in the documentation and/or other materials provided with the
 *	distribution.
 *
 *  "DISCLAIMER OF LIABILITY
 *
 *  THIS SOFTWARE IS PROVIDED BY DAN HARKINS ``AS IS'' AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INDUSTRIAL LOUNGE BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE."
 *
 * This license and distribution terms cannot be changed. In other words,
 * this code cannot simply be copied and put under a different distribution
 * license (including the GNU public license).
 */

#ifndef _RLM_EAP_PWD_H
#define _RLM_EAP_PWD_H

#include "eap_pwd.h"

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>

typedef struct eap_pwd_conf {
    int group;
    int fragment_size;
    char *server_id;
    char *virtual_server;
} EAP_PWD_CONF;

typedef struct _eap_pwd_t {
    EAP_PWD_CONF *conf;
    BN_CTX *bnctx;
} eap_pwd_t;

#endif  /* _RLM_EAP_PWD_H */
