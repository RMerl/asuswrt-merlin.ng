/*
 * Copyright (c) 2020, NLNet Labs, Sinodun
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

#include "config.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <getdns/getdns.h>
#include <getdns/getdns_extra.h>

#include "configfile.h"
#include "log.h"
#include "server.h"
#include "util.h"

static int dnssec_validation = 0;

#if defined(STUBBY_ON_WINDOWS)
#define DEBUG_ON(...) do { \
                        struct timeval tv_dEbUgSyM; \
                        struct tm tm_dEbUgSyM; \
                        char buf_dEbUgSyM[10]; \
                        time_t tsec_dEbUgSyM; \
                        \
                        gettimeofday(&tv_dEbUgSyM, NULL); \
                        tsec_dEbUgSyM = (time_t) tv_dEbUgSyM.tv_sec; \
                        gmtime_s(&tm_dEbUgSyM, (const time_t *) &tsec_dEbUgSyM); \
                        strftime(buf_dEbUgSyM, 10, "%H:%M:%S", &tm_dEbUgSyM); \
                        fprintf(stderr, "[%s.%.6d] ", buf_dEbUgSyM, (int)tv_dEbUgSyM.tv_usec); \
                        fprintf(stderr, __VA_ARGS__); \
                } while (0)
#else
#define DEBUG_ON(...) do { \
                        struct timeval tv_dEbUgSyM; \
                        struct tm tm_dEbUgSyM; \
                        char buf_dEbUgSyM[10]; \
                        \
                        gettimeofday(&tv_dEbUgSyM, NULL); \
                        gmtime_r(&tv_dEbUgSyM.tv_sec, &tm_dEbUgSyM); \
                        strftime(buf_dEbUgSyM, 10, "%H:%M:%S", &tm_dEbUgSyM); \
                        fprintf(stderr, "[%s.%.6d] ", buf_dEbUgSyM, (int)tv_dEbUgSyM.tv_usec); \
                        fprintf(stderr, __VA_ARGS__); \
                } while (0)
#endif
#define DEBUG_OFF(...) do {} while (0)

#if defined(SERVER_DEBUG) && SERVER_DEBUG
#include <time.h>
#define DEBUG_SERVER(...) DEBUG_ON(__VA_ARGS__)
#else
#define DEBUG_SERVER(...) DEBUG_OFF(__VA_ARGS__)
#endif

typedef struct dns_msg {
        getdns_transaction_t  request_id;
        getdns_dict          *request;
        getdns_resolution_t   rt;
        uint32_t              ad_bit;
        uint32_t              do_bit;
        uint32_t              cd_bit;
        int                   has_edns0;
} dns_msg;

#if defined(SERVER_DEBUG) && SERVER_DEBUG
#define SERVFAIL(error,r,msg,resp_p) do { \
        if (r)  DEBUG_SERVER("%s: %s\n", error, stubby_getdns_strerror(r)); \
        else    DEBUG_SERVER("%s\n", error); \
        servfail(msg, resp_p); \
        } while (0)
#else
#define SERVFAIL(error,r,msg,resp_p) servfail(msg, resp_p)
#endif

static void servfail(dns_msg *msg, getdns_dict **resp_p)
{
        getdns_dict *dict;

        if (*resp_p)
                getdns_dict_destroy(*resp_p);
        if (!(*resp_p = getdns_dict_create()))
                return;
        if (msg) {
                if (!getdns_dict_get_dict(msg->request, "header", &dict))
                        getdns_dict_set_dict(*resp_p, "header", dict);
                if (!getdns_dict_get_dict(msg->request, "question", &dict))
                        getdns_dict_set_dict(*resp_p, "question", dict);
                (void) getdns_dict_set_int(*resp_p, "/header/ra",
                    msg->rt == GETDNS_RESOLUTION_RECURSING ? 1 : 0);
        }
        (void) getdns_dict_set_int(
            *resp_p, "/header/rcode", GETDNS_RCODE_SERVFAIL);
        (void) getdns_dict_set_int(*resp_p, "/header/qr", 1);
        (void) getdns_dict_set_int(*resp_p, "/header/ad", 0);
}

static getdns_return_t _handle_edns0(
    getdns_dict *response, int has_edns0)
{
        getdns_return_t r;
        getdns_list *additional;
        size_t len, i;
        getdns_dict *rr;
        uint32_t rr_type;
        char remove_str[100] = "/replies_tree/0/additional/";

        if ((r = getdns_dict_set_int(
            response, "/replies_tree/0/header/do", 0)))
                return r;
        if ((r = getdns_dict_get_list(response, "/replies_tree/0/additional",
            &additional)))
                return r;
        if ((r = getdns_list_get_length(additional, &len)))
                return r;
        for (i = 0; i < len; i++) {
                if ((r = getdns_list_get_dict(additional, i, &rr)))
                        return r;
                if ((r = getdns_dict_get_int(rr, "type", &rr_type)))
                        return r;
                if (rr_type != GETDNS_RRTYPE_OPT)
                        continue;
                if (has_edns0) {
                        (void) getdns_dict_set_int(rr, "do", 0);
                        break;
                }
                (void) snprintf(remove_str + 27, 60, "%d", (int)i);
                if ((r = getdns_dict_remove_name(response, remove_str)))
                        return r;
                break;
        }
        return GETDNS_RETURN_GOOD;
}

static void request_cb(
    getdns_context *context, getdns_callback_type_t callback_type,
    getdns_dict *response, void *userarg, getdns_transaction_t transaction_id)
{
        dns_msg *msg = (dns_msg *)userarg;
        uint32_t qid;
        getdns_return_t r = GETDNS_RETURN_GOOD;
        uint32_t n, rcode, dnssec_status = GETDNS_DNSSEC_INDETERMINATE;
        getdns_list *options;
        size_t n_options;
        uint32_t arcount;
        char i_as_jptr[80];

#if defined(SERVER_DEBUG) && SERVER_DEBUG
        getdns_bindata *qname;
        char *qname_str, *unknown_qname = "<unknown_qname>";

        if (getdns_dict_get_bindata(msg->request, "/question/qname", &qname)
        ||  getdns_convert_dns_name_to_fqdn(qname, &qname_str))
                qname_str = unknown_qname;

        DEBUG_SERVER("reply for: %p %"PRIu64" %d (edns0: %d, do: %d, ad: %d,"
            " cd: %d, qname: %s)\n", (void *)msg, transaction_id, (int)callback_type,
            msg->has_edns0, msg->do_bit, msg->ad_bit, msg->cd_bit, qname_str);

        if (qname_str != unknown_qname)
                free(qname_str);
#else
        (void)transaction_id;
#endif
        assert(msg);

        if (callback_type != GETDNS_CALLBACK_COMPLETE)
                SERVFAIL("Callback type not complete",
                    (int)callback_type, msg, &response);

        else if (!response)
                SERVFAIL("Missing response", 0, msg, &response);

        else if ((r = getdns_dict_get_int(msg->request, "/header/id", &qid)) ||
            (r=getdns_dict_set_int(response,"/replies_tree/0/header/id",qid)))
                SERVFAIL("Could not copy QID", r, msg, &response);

        else if (getdns_dict_get_int(
            response, "/replies_tree/0/header/rcode", &rcode))
                SERVFAIL("No reply in replies tree", 0, msg, &response);

        /* answers when CD or not BOGUS */
        else if (!msg->cd_bit && !getdns_dict_get_int(
            response, "/replies_tree/0/dnssec_status", &dnssec_status)
            && dnssec_status == GETDNS_DNSSEC_BOGUS)
                SERVFAIL("DNSSEC status was bogus", 0, msg, &response);

        else if (rcode == GETDNS_RCODE_SERVFAIL)
                servfail(msg, &response);

        /* RRsigs when DO and (CD or not BOGUS)
         * Implemented in conversion to wireformat function by checking for DO
         * bit.  In recursing resolution mode we have to copy the do bit from
         * the request, because libunbound has it in the answer always.
         */
        else if (msg->rt == GETDNS_RESOLUTION_RECURSING && !msg->do_bit &&
            (r = _handle_edns0(response, msg->has_edns0)))
                SERVFAIL("Could not handle EDNS0", r, msg, &response);

        /* AD when (DO or AD) and SECURE (But only when we perform validation natively) */
        else if (dnssec_validation &&
            (r = getdns_dict_set_int(response,"/replies_tree/0/header/ad",
            ((msg->do_bit || msg->ad_bit)
            && (  (!msg->cd_bit && dnssec_status == GETDNS_DNSSEC_SECURE)
               || ( msg->cd_bit && !getdns_dict_get_int(response,
                    "/replies_tree/0/dnssec_status", &dnssec_status)
                  && dnssec_status == GETDNS_DNSSEC_SECURE ))) ? 1 : 0)))
                SERVFAIL("Could not set AD bit", r, msg, &response);

        else if ((dnssec_validation || msg->rt == GETDNS_RESOLUTION_RECURSING)
            && (r =  getdns_dict_set_int(
            response, "/replies_tree/0/header/cd", msg->cd_bit)))
                SERVFAIL("Could not copy CD bit", r, msg, &response);

        else if (msg->rt == GETDNS_RESOLUTION_STUB)
                ; /* following checks are for RESOLUTION_RECURSING only */

        else if ((r = getdns_dict_get_int(
            response, "/replies_tree/0/header/ra", &n)))
                SERVFAIL("Could not get RA bit from reply", r, msg, &response);

        else if (n == 0)
                SERVFAIL("Recursion not available", 0, msg, &response);

        if (!getdns_dict_get_int(response, "/replies_tree/0/header/arcount", &arcount)
        &&  arcount > 0
        &&  snprintf( i_as_jptr, sizeof(i_as_jptr)
                    , "/replies_tree/0/additional/%d/rdata/options"
                    , (int)(arcount - 1))
        &&  !getdns_dict_get_list(response, i_as_jptr, &options)
        &&  !getdns_list_get_length(options, &n_options)) {
                int i;
                int options_changed = 0;

                for (i = 0; i < (int)n_options; i++) {
                        getdns_dict *option;
                        uint32_t option_code;
                        uint8_t a_byte;
                        uint16_t a_word;

                        (void) snprintf(i_as_jptr, sizeof(i_as_jptr),
                            "/replies_tree/0/additional/%d/rdata/options/%d",
                            (int)(arcount - 1), i);

                        if (getdns_dict_get_dict(response, i_as_jptr, &option)
                        ||  getdns_dict_get_int(option, "option_code", &option_code))
                                continue;

                        switch (option_code) {
                        case  8: /* CLIENT SUBNET */
                                if (getdns_context_get_edns_client_subnet_private
                                    (context, &a_byte) || !a_byte)
                                        continue;
                                break;
                        case 11: /* KeepAlive (remove always) */
                                break;
                        case 12: /* Padding */
                                if (getdns_context_get_tls_query_padding_blocksize
                                    (context, &a_word) || !a_word)
                                        continue;
                                break;
                        default:
                                continue;
                        }
                        if (!getdns_dict_remove_name(response, i_as_jptr)) {
                                options_changed++;
                                i -= 1;
                                n_options -= 1;
                        }
                }
                if (options_changed) {
                        (void) snprintf( i_as_jptr, sizeof(i_as_jptr)
                                       , "/replies_tree/0/additional/%d/rdata/rdata_raw"
                                       , (int)(arcount - 1));
                        (void) getdns_dict_remove_name(response, i_as_jptr);
                }
        }
        if ((r = getdns_reply(context, response, msg->request_id))) {
                stubby_error("Could not reply: %s", stubby_getdns_strerror(r));
                /* Cancel reply */
                (void) getdns_reply(context, NULL, msg->request_id);
        }
        if (msg) {
                getdns_dict_destroy(msg->request);
                free(msg);
        }
        if (response)
                getdns_dict_destroy(response);
}

static void incoming_request_handler(getdns_context *context,
    getdns_callback_type_t callback_type, getdns_dict *request,
    void *userarg, getdns_transaction_t request_id)
{
        getdns_bindata *qname;
        char *qname_str = NULL;
        uint32_t qtype;
        uint32_t qclass;
        getdns_return_t r;
        getdns_dict *header;
        uint32_t n;
        getdns_list *list;
        getdns_transaction_t transaction_id = 0;
        getdns_dict *qext = NULL;
        dns_msg *msg = NULL;
        getdns_dict *response = NULL;
        size_t i, len;
        getdns_list *additional;
        getdns_dict *rr;
        uint32_t rr_type;

        (void)callback_type;
        (void)userarg;

        if (!(qext = getdns_dict_create_with_context(context)) ||
            !(msg = malloc(sizeof(dns_msg))))
                goto error;

        /* pass through the header and the OPT record */
        n = 0;
        msg->request_id = request_id;
        msg->request = request;
        msg->ad_bit = msg->do_bit = msg->cd_bit = 0;
        msg->has_edns0 = 0;
        msg->rt = GETDNS_RESOLUTION_RECURSING;
        (void) getdns_dict_get_int(request, "/header/ad", &msg->ad_bit);
        (void) getdns_dict_get_int(request, "/header/cd", &msg->cd_bit);
        if (!getdns_dict_get_list(request, "additional", &additional)) {
                if (getdns_list_get_length(additional, &len))
                        len = 0;
                for (i = 0; i < len; i++) {
                        if (getdns_list_get_dict(additional, i, &rr))
                                break;
                        if (getdns_dict_get_int(rr, "type", &rr_type))
                                break;
                        if (rr_type != GETDNS_RRTYPE_OPT)
                                continue;
                        msg->has_edns0 = 1;
                        (void) getdns_dict_get_int(rr, "do", &msg->do_bit);
                        break;
                }
        }
        if ((r = getdns_context_get_resolution_type(context, &msg->rt)))
                stubby_error("Could get resolution type from context: %s",
                    stubby_getdns_strerror(r));

        if (msg->rt == GETDNS_RESOLUTION_STUB) {
                (void)getdns_dict_set_int(
                    qext , "/add_opt_parameters/do_bit", msg->do_bit);
                if (!getdns_dict_get_dict(request, "header", &header))
                        (void)getdns_dict_set_dict(qext, "header", header);

        }
        if (msg->cd_bit && dnssec_validation)
                getdns_dict_set_int(qext, "dnssec_return_all_statuses",
                    GETDNS_EXTENSION_TRUE);

        if (!getdns_dict_get_int(request, "/additional/0/extended_rcode",&n))
                (void)getdns_dict_set_int(
                    qext, "/add_opt_parameters/extended_rcode", n);

        if (!getdns_dict_get_int(request, "/additional/0/version", &n))
                (void)getdns_dict_set_int(
                    qext, "/add_opt_parameters/version", n);

        if (!getdns_dict_get_int(
            request, "/additional/0/udp_payload_size", &n))
                (void)getdns_dict_set_int(qext,
                    "/add_opt_parameters/maximum_udp_payload_size", n);

        if (!getdns_dict_get_list(
            request, "/additional/0/rdata/options", &list))
                (void)getdns_dict_set_list(qext,
                    "/add_opt_parameters/options", list);

        if ((r = getdns_dict_get_bindata(request,"/question/qname",&qname)))
                stubby_error("Could not get qname from query: %s",
                    stubby_getdns_strerror(r));

        else if ((r = getdns_convert_dns_name_to_fqdn(qname, &qname_str)))
                stubby_error("Could not convert qname: %s",
                    stubby_getdns_strerror(r));

        else if ((r=getdns_dict_get_int(request,"/question/qtype",&qtype)))
                stubby_error("Could get qtype from query: %s",
                    stubby_getdns_strerror(r));

        else if ((r=getdns_dict_get_int(request,"/question/qclass",&qclass)))
                stubby_error("Could get qclass from query: %s",
                    stubby_getdns_strerror(r));

        else if ((r = getdns_dict_set_int(qext, "specify_class", qclass)))
                stubby_error("Could set class from query: %s",
                    stubby_getdns_strerror(r));

        else if ((r = getdns_general(context, qname_str, qtype,
            qext, msg, &transaction_id, request_cb)))
                stubby_error("Could not schedule query: %s",
                    stubby_getdns_strerror(r));
        else {
                DEBUG_SERVER("scheduled: %p %"PRIu64" for %s %d\n",
                    (void *)msg, transaction_id, qname_str, (int)qtype);
                getdns_dict_destroy(qext);
                free(qname_str);
                return;
        }
error:
        if (qname_str)
                free(qname_str);
        if (qext)
                getdns_dict_destroy(qext);
        servfail(msg, &response);
#if defined(SERVER_DEBUG) && SERVER_DEBUG
        do {
                char *request_str = getdns_pretty_print_dict(request);
                char *response_str = getdns_pretty_print_dict(response);
                DEBUG_SERVER("request error, request: %s\n, response: %s\n"
                            , request_str, response_str);
                free(response_str);
                free(request_str);
        } while(0);
#endif
        if ((r = getdns_reply(context, response, request_id))) {
                stubby_error("Could not reply: %s",
                    stubby_getdns_strerror(r));
                /* Cancel reply */
                getdns_reply(context, NULL, request_id);
        }
        if (msg) {
                if (msg->request)
                        getdns_dict_destroy(msg->request);
                free(msg);
        }
        if (response)
                getdns_dict_destroy(response);
}

int server_listen(getdns_context *context, int validate_dnssec)
{
        const getdns_list *listen_list;

        dnssec_validation = validate_dnssec;

        listen_list = get_config_listen_list();
        if ( !listen_list )
                return 0;
        if ( getdns_context_set_listen_addresses(
                     context, listen_list, NULL, incoming_request_handler) ) {
                stubby_error("error: Could not bind on given addresses: %s", strerror(errno));
                return 0;
        }

        return 1;
}
