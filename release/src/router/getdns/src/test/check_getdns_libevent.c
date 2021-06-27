/**
 * \file
 * \brief Testing interfaces to getdns
 *
 * This source was taken from the original pseudo-implementation by
 * Paul Hoffman.
 */

/*
 * Copyright (c) 2013, NLNet Labs, Verisign, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the names of the copyright holders nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Verisign, Inc. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "check_getdns_eventloop.h"

#include "getdns/getdns_ext_libevent.h"
#include "check_getdns_libevent.h"
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif
#include <check.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#include "check_getdns_common.h"

void run_event_loop_impl(struct getdns_context* context, void* eventloop) {
    struct event_base* base = (struct event_base*) eventloop;
    (void)context; /* unused parameter */
    event_base_dispatch(base);
}

void* create_eventloop_impl(struct getdns_context* context) {
    struct event_base* result = event_base_new();
    ck_assert_msg(result != NULL, "Event base creation failed");
    ASSERT_RC(getdns_extension_set_libevent_base(context, result),
        GETDNS_RETURN_GOOD,
        "Return code from getdns_extension_set_libevent_base()");
    return result;
}
