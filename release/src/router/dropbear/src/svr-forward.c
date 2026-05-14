/*
 * Dropbear SSH
 * 
 * Copyright (c) 2002,2003 Matt Johnston
 * Copyright (c) 2004 by Mihnea Stoenescu
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */

#include "includes.h"
#include "ssh.h"
#include "forward.h"
#include "dbutil.h"
#include "session.h"
#include "buffer.h"
#include "packet.h"
#include "listener.h"
#include "runopts.h"
#include "auth.h"

void svr_recv_msg_global_request(void) {

    char* reqname = NULL;
    unsigned int namelen;
    unsigned int wantreply = 0;
    int ret = DROPBEAR_FAILURE;

    TRACE(("enter recv_msg_global_request_remote"))

    reqname = buf_getstring(ses.payload, &namelen);
    wantreply = buf_getbool(ses.payload);

#if DROPBEAR_SVR_REMOTEANYFWD
    if (svr_opts.noremotefwd || !svr_pubkey_allows_tcpfwd()) {
        TRACE(("remote forwarding is disabled"));
        goto out;
    }
#else
    /* No request handlers */
    goto out;
#endif

    if (namelen > MAX_NAME_LEN) {
        TRACE(("name len is wrong: %d", namelen))
        goto out;
    }

    if (0) {}
#if DROPBEAR_SVR_REMOTETCPFWD
    else if (strcmp("tcpip-forward", reqname) == 0) {
        ret = svr_remotetcpreq(wantreply);
        /* svr_remotetcpreq sends its on reply if needed,
         * don't send another in out: below */
        wantreply = 0;
    } else if (strcmp("cancel-tcpip-forward", reqname) == 0) {
        ret = svr_cancelremotetcp();
    }
#endif
#if DROPBEAR_SVR_REMOTESTREAMFWD
    else if (strcmp("streamlocal-forward@openssh.com", reqname) == 0) {
        ret = svr_remotestreamlocalreq();
    } else if (strcmp("cancel-streamlocal-forward@openssh.com", reqname) == 0) {
        ret = svr_cancelremotestreamlocal();
    }
#endif
    else {
        TRACE(("unhandled request '%s'", reqname))
    }

out:
    if (wantreply) {
        if (ret == DROPBEAR_SUCCESS) {
            send_msg_request_success();
        } else {
            send_msg_request_failure();
        }
    }

    m_free(reqname);

    TRACE(("leave recv_msg_global_request"))
}
