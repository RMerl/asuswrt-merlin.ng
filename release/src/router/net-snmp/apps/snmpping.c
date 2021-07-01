/*
 * Portions of this file are subject to the following copyright(s).  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply.
 */
/*
 * Copyright (c) 2013, Arista Networks, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Arista Networks, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include <net-snmp/net-snmp-config.h>

#include <signal.h>

#include <ctype.h>

#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <sys/types.h>
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif

#include <math.h>

#include <net-snmp/net-snmp-includes.h>

#include "inet_ntop.h"

/* XXX */
#define INETADDRESSTYPE_IPV4    1
#define INETADDRESSTYPE_IPV6    2

#define PINGCTLADMINSTATUS_ENABLED 1

/* Target info */
int targetAddrType;
u_char targetAddr[16];
int targetAddrLen;

char *targetName;

/* Parameters */
int pings = 15;
int datasize = 0;
/* todo: timeout, data fill, ownerindex, testname */

/* Control-C? */
int interrupted = 0;

void
usage(void)
{
    fprintf(stderr, "Usage: snmpping ");
    snmp_parse_args_usage(stderr);
    fprintf(stderr, " DESTINATION\n\n");
    snmp_parse_args_descriptions(stderr);
    fprintf(stderr, "\nsnmpping options:\n");
    fprintf(stderr, "\t-Cc<pings>\tSpecify the number of pings (1-15)\n");
    fprintf(stderr, "\t-Cs<size>\tSpecify the amount of extra data (0-65507)\n");
}

static void
optProc(int argc, char *const *argv, int opt)
{
    char *endptr = NULL;

    switch (opt) {
    case 'C':
        while (*optarg) {
            switch (*optarg++) {
            case 'c':
                pings = strtol(optarg, &endptr, 0);
                if (pings < 1 || pings > 15) {
                    /* out of range */
                    usage();
                    exit(1);
                }

                optarg = endptr;
                if (isspace((unsigned char)(*optarg))) {
                    return;
                }
                break;

            case 's':
                datasize = strtol(optarg, &endptr, 0);
                if (datasize < 0 || datasize > 65507) {
                    /* out of range */
                    usage();
                    exit(1);
                }

                optarg = endptr;
                if (isspace((unsigned char)(*optarg))) {
                    return;
                }
                break;


            default:
                fprintf(stderr,
                        "Unknown flag passed to -C: %c\n", optarg[-1]);
                exit(1);
            }
        }
    }
}

void sigint(int sig)
{
    interrupted = 1;
    printf("[interrupted]\n");
}

struct pingResultsTable {
    int   pingResultsOperStatus;
    int   pingResultsMinRtt;
    int   pingResultsMaxRtt;
    int   pingResultsAverageRtt;
    int   pingResultsProbeResponses;
    int   pingResultsSentProbes;
    int   pingResultsRttSumOfSquares;
    char *pingResultsLastGoodProbe;
};

struct pingProbeHistoryTable {
    int   pingProbeHistoryIndex;
    int   pingProbeHistoryResponse;
    int   pingProbeHistoryStatus;
    char *pingProbeHistoryTime;
};

const char *
inetaddresstop(u_char *addr, int addrlen, int addrtype)
{
    int type;
    static char buf[INET6_ADDRSTRLEN];

    switch (addrtype) {
    case INETADDRESSTYPE_IPV4:
        type = AF_INET;
        break;
    case INETADDRESSTYPE_IPV6:
        type = AF_INET6;
        break;
    default:
        buf[0] = '?';
        buf[1] = '\0';
        return buf;
    }
    return inet_ntop(type, addr, buf, sizeof(buf));
}

int
add_var(netsnmp_pdu *pdu, const char *mibnodename,
    oid * index, size_t indexlen,
    u_char type, const void *value, size_t len)
{
    oid             base[MAX_OID_LEN];
    size_t          base_length = MAX_OID_LEN;

    memset(base, 0, MAX_OID_LEN * sizeof(oid));

    if (!snmp_parse_oid(mibnodename, base, &base_length)) {
        snmp_perror(mibnodename);
        fprintf(stderr, "couldn't find mib node %s, giving up\n",
                mibnodename);
        exit(1);
    }

    if (index && indexlen) {
        memcpy(&(base[base_length]), index, indexlen * sizeof(oid));
        base_length += indexlen;
    }
    DEBUGMSGTL(("add", "created: "));
    DEBUGMSGOID(("add", base, base_length));
    DEBUGMSG(("add", "\n"));
    snmp_varlist_add_variable(&pdu->variables, base, base_length,
          type, value, len);

    return base_length;
}

int
add(netsnmp_pdu *pdu, const char *mibnodename,
    oid * index, size_t indexlen)
{
   return add_var(pdu, mibnodename, index, indexlen, ASN_NULL, NULL, 0);
}

int
my_synch_response(netsnmp_session *ss, netsnmp_pdu *pdu, netsnmp_pdu **response)
{
    int status;

    status = snmp_synch_response(ss, pdu, response);
    if (status == STAT_SUCCESS) {
        if (*response) {
            if ((*response)->errstat == SNMP_ERR_NOERROR) {
                return 0;
            } else {
                fprintf(stderr, "Error in packet.\nReason: %s\n",
                        snmp_errstring((*response)->errstat));
                if ((*response)->errindex != 0) {
                    int             count;
                    netsnmp_variable_list *vars;
                    fprintf(stderr, "Failed object: ");
                    for (count = 1, vars = (*response)->variables;
                         vars && count != (*response)->errindex;
                         vars = vars->next_variable, count++)
                        /*EMPTY*/;
                    if (vars)
                        fprint_objid(stderr, vars->name,
                                     vars->name_length);
                    else
                        fprintf(stderr, "??? (errindex=%ld)",
                                        (*response)->errindex);
                    fprintf(stderr, "\n");
                }
                return 2;
            }
        }
    } else if (status == STAT_TIMEOUT) {
        fprintf(stderr, "Timeout: No Response from %s\n",
                ss->peername);
        return 1;
    } else {                    /* status == STAT_ERROR */
        snmp_sess_perror("snmpping", ss);
        return 1;
    }
    return 0;
}

int
cleanup_ctlTable(netsnmp_session *ss, oid * index, size_t indexlen)
{
    netsnmp_pdu    *pdu;
    netsnmp_pdu    *response;
    int             rowStatus;
    int             status;

    pdu = snmp_pdu_create(SNMP_MSG_SET);
    rowStatus = RS_DESTROY;
    add_var(pdu, "DISMAN-PING-MIB::pingCtlRowStatus", index, indexlen, ASN_INTEGER,
          &rowStatus, sizeof(rowStatus));
    status = my_synch_response(ss, pdu, &response);
    if (response)
        snmp_free_pdu(response);
    return status;
}

int
start_ping(netsnmp_session *ss, oid * index, size_t indexlen, char *pingDest)
{
    netsnmp_pdu    *pdu;
    netsnmp_pdu    *response;
    int             adminStatus, rowStatus, storageType;
    int             status;
    struct addrinfo *dest, hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_CANONNAME;
    status = getaddrinfo(pingDest, NULL, &hints, &dest);
    if (status != 0) {
        fprintf(stderr, "snmpping: %s: %s\n", pingDest, gai_strerror(status));
        return 1;
    }

    /*
     * Destroy any previously-existing row.  We could get fancy
     * and try to reuse it, but that is way more complex.
     */
    cleanup_ctlTable(ss, index, indexlen);

    switch (dest->ai_family) {
       case AF_INET:
          targetAddrType = INETADDRESSTYPE_IPV4;
          targetAddrLen = sizeof(struct in_addr);
          memcpy(targetAddr, &((struct sockaddr_in *)dest->ai_addr)->sin_addr, targetAddrLen);
          break;
#ifdef NETSNMP_ENABLE_IPV6
       case AF_INET6:
          targetAddrType = INETADDRESSTYPE_IPV6;
          targetAddrLen = sizeof(struct in6_addr);
          memcpy(targetAddr, &((struct sockaddr_in6 *)dest->ai_addr)->sin6_addr, sizeof(struct in6_addr));
          break;
#endif
       default:
          fprintf(stderr, "Unsupported address family\n");
          return 3;
    }

    if (dest->ai_canonname) {
        targetName = strdup(dest->ai_canonname);
    } else {
        targetName = strdup(pingDest);
    }

    freeaddrinfo(dest);

    pdu = snmp_pdu_create(SNMP_MSG_SET);
    add_var(pdu, "DISMAN-PING-MIB::pingCtlTargetAddressType", index, indexlen, ASN_INTEGER,
          &targetAddrType, sizeof(targetAddrType));
    add_var(pdu, "DISMAN-PING-MIB::pingCtlTargetAddress", index, indexlen, ASN_OCTET_STR,
          &targetAddr, targetAddrLen);
    /* Rely on DEFVAL to keep the PDU small */
    if (pings != 1) {
        add_var(pdu, "DISMAN-PING-MIB::pingCtlProbeCount", index, indexlen, ASN_UNSIGNED,
              &pings, sizeof(pings));
    }
    if (datasize != 0) {
        add_var(pdu, "DISMAN-PING-MIB::pingCtlDataSize", index, indexlen, ASN_UNSIGNED,
              &datasize, sizeof(datasize));
    }
    adminStatus = PINGCTLADMINSTATUS_ENABLED;
    add_var(pdu, "DISMAN-PING-MIB::pingCtlAdminStatus", index, indexlen, ASN_INTEGER,
          &adminStatus, sizeof(adminStatus));
    storageType = ST_VOLATILE;  /* don't ask for this to be saved, we're only going to delete it */
    add_var(pdu, "DISMAN-PING-MIB::pingCtlStorageType", index, indexlen, ASN_INTEGER,
          &storageType, sizeof(storageType));
    rowStatus = RS_CREATEANDGO;
    add_var(pdu, "DISMAN-PING-MIB::pingCtlRowStatus", index, indexlen, ASN_INTEGER,
          &rowStatus, sizeof(rowStatus));
    status = my_synch_response(ss, pdu, &response);
    if (response)
        snmp_free_pdu(response);
    if (status == 0) {
        printf("PING %s (%s) from %s with %d bytes of extra data\n", targetName,
              inetaddresstop(targetAddr, targetAddrLen, targetAddrType), ss->peername,
              datasize);
    }
    return status;
}

int
wait_for_completion(netsnmp_session *ss, oid * index, size_t indexlen)
{
    int             running = 1;
    int             status;
    int             pingStatus;
    int             sent;
    int             responses, prev_responses = 0;
    int             tries = 0;
    netsnmp_pdu    *pdu;
    netsnmp_pdu    *response;
    netsnmp_variable_list *vlp;

    while (running && !interrupted) {
        pdu = snmp_pdu_create(SNMP_MSG_GET);
        add(pdu, "DISMAN-PING-MIB::pingResultsOperStatus", index, indexlen);
        add(pdu, "DISMAN-PING-MIB::pingResultsSentProbes", index, indexlen);
        add(pdu, "DISMAN-PING-MIB::pingResultsProbeResponses", index, indexlen);

        status = snmp_synch_response(ss, pdu, &response);
        if (status != STAT_SUCCESS || !response) {
            snmp_sess_perror("snmpping", ss);
            if (status == STAT_TIMEOUT)
                goto retry;
            running = 0;
            goto out;
        }
        if (response->errstat != SNMP_ERR_NOERROR) {
            fprintf(stderr, "snmpping: Error in packet: %s\n",
                    snmp_errstring(response->errstat));
            running = 0;
            goto out;
        }

        vlp = response->variables;
        if (vlp->type == SNMP_NOSUCHINSTANCE) {
            DEBUGMSGTL(("ping", "no-such-instance for pingResultsOperStatus\n"));
            goto retry;
        }
        pingStatus = *vlp->val.integer;

        vlp = vlp->next_variable;
        if (vlp->type == SNMP_NOSUCHINSTANCE) {
            DEBUGMSGTL(("ping", "no-such-instance for pingResultsSentProbes\n"));
            goto retry;
        }
        sent = *vlp->val.integer;

        vlp = vlp->next_variable;
        if (vlp->type == SNMP_NOSUCHINSTANCE) {
            DEBUGMSGTL(("ping", "no-such-instance for pingResultsProbeResponses\n"));
            goto retry;
        }
        responses = *vlp->val.integer;
#define PINGRESULTSOPERSTATUS_ENABLED   1 /* XXX */
#define PINGRESULTSOPERSTATUS_DISABLED  2 /* XXX */
#define PINGRESULTSOPERSTATUS_COMPLETED 3 /* XXX */

        if (responses > prev_responses || pingStatus == PINGRESULTSOPERSTATUS_COMPLETED) {
            DEBUGMSGTL(("ping", "responses %d (was %d), status %d\n", responses, prev_responses, pingStatus));

            /* collect results between prev_responses and responses by walking probeHistoryTable */
            prev_responses = responses;
        }

        /*
         * Observed behavior: before the test has run, operStatus can be
         * disabled, and then can turn to enabled, so we can't just stop
         * if it's disabled.  However, it doesn't always go to completed.
         * So, we say we're completed if it's completed, *or* if it's
         * disabled and we've sent at least one probe.
         */
        if (pingStatus == PINGRESULTSOPERSTATUS_COMPLETED ||
            (pingStatus == PINGRESULTSOPERSTATUS_DISABLED && sent > 0)) {
            running = 0;
            goto out;
        }

        /* sleep before asking again */
        sleep(1);

        if (0) {
retry:
            if (tries++ < 5) {
                /* we can try again */
                sleep(1);
            } else {
                if (status == STAT_TIMEOUT)
                    fprintf(stderr, "snmpping: too many timeouts.\n");
                else
                    fprintf(stderr, "snmpping: pingResultsTable entry never created.\n");
                running = 0;
            }
        }

out:
        if (response)
            snmp_free_pdu(response);
    }
    return 0;
}

int
overall_stats(netsnmp_session *ss, oid * index, size_t indexlen)
{
    netsnmp_pdu    *pdu;
    netsnmp_pdu    *response;
    netsnmp_variable_list *vlp;
    int status;
    struct pingResultsTable result;

    pdu = snmp_pdu_create(SNMP_MSG_GET);
    add(pdu, "DISMAN-PING-MIB::pingResultsOperStatus", index, indexlen);
    add(pdu, "DISMAN-PING-MIB::pingResultsMinRtt", index, indexlen);
    add(pdu, "DISMAN-PING-MIB::pingResultsMaxRtt", index, indexlen);
    add(pdu, "DISMAN-PING-MIB::pingResultsAverageRtt", index, indexlen);
    add(pdu, "DISMAN-PING-MIB::pingResultsProbeResponses", index, indexlen);
    add(pdu, "DISMAN-PING-MIB::pingResultsSentProbes", index, indexlen);
    add(pdu, "DISMAN-PING-MIB::pingResultsRttSumOfSquares", index, indexlen);

    status = snmp_synch_response(ss, pdu, &response);
    if (status != STAT_SUCCESS || !response) {
        snmp_sess_perror("snmpping", ss);
        goto out;
    }
    if (response->errstat != SNMP_ERR_NOERROR) {
        fprintf(stderr, "snmpping: Error in packet: %s\n",
                snmp_errstring(response->errstat));
        goto out;
    }

    vlp = response->variables;
    if (vlp->type == SNMP_NOSUCHOBJECT) goto parseerr;
    result.pingResultsOperStatus = *vlp->val.integer;
    vlp = vlp->next_variable;
    if (vlp->type == SNMP_NOSUCHOBJECT) goto parseerr;
    result.pingResultsMinRtt = *vlp->val.integer;
    vlp = vlp->next_variable;
    if (vlp->type == SNMP_NOSUCHOBJECT) goto parseerr;
    result.pingResultsMaxRtt = *vlp->val.integer;
    vlp = vlp->next_variable;
    if (vlp->type == SNMP_NOSUCHOBJECT) goto parseerr;
    result.pingResultsAverageRtt = *vlp->val.integer;
    vlp = vlp->next_variable;
    if (vlp->type == SNMP_NOSUCHOBJECT) goto parseerr;
    result.pingResultsProbeResponses = *vlp->val.integer;
    vlp = vlp->next_variable;
    if (vlp->type == SNMP_NOSUCHOBJECT) goto parseerr;
    result.pingResultsSentProbes = *vlp->val.integer;
    vlp = vlp->next_variable;
    if (vlp->type == SNMP_NOSUCHOBJECT) goto parseerr;
    result.pingResultsRttSumOfSquares = *vlp->val.integer;
    vlp = vlp->next_variable;

    printf( "--- %s ping statistics ---\n", targetName );
    printf( "%d packets transmitted, %d received, %d%% packet loss\n",
            result.pingResultsSentProbes, result.pingResultsProbeResponses,
            result.pingResultsSentProbes ?
                ( ( result.pingResultsSentProbes -
                    result.pingResultsProbeResponses ) * 100 /
                  result.pingResultsSentProbes ) : 0 );
    if (result.pingResultsProbeResponses) {
        double stddev;

        stddev = result.pingResultsRttSumOfSquares;
        stddev /= result.pingResultsProbeResponses;
        stddev -= result.pingResultsAverageRtt * result.pingResultsAverageRtt;
        /*
         * If the RTT is less than 1.0, the sum of squares can be
         * smaller than the number of responses, resulting in a
         * negative stddev.  Clamp the stddev to 0.
         */
        if (stddev < 0)
            stddev = 0.0;
        printf( "rtt min/avg/max/stddev = %d/%d/%d/%d ms\n",
                result.pingResultsMinRtt,
                result.pingResultsAverageRtt,
                result.pingResultsMaxRtt,
                (int)sqrt( stddev ));
    }
    if (0) {
parseerr:
        fprintf(stderr, "snmpping: Error parsing response packet\n");
    }

out:
    if (response)
        snmp_free_pdu(response);
    return 0;
}

#ifdef WIN32
/* To do: port this function to the Win32 platform. */
const char *getlogin(void)
{
    return "";
}
#endif

int main(int argc, char **argv)
{
    netsnmp_session session, *ss;
    int ret;
    int arg;
    oid index[66], *idx;
    int indexlen, i;
    int usernameLen, testnameLen;
    char username[33];
    char testname[33];
    char *p;

    /*
     * get the common command line arguments 
     */
    switch (arg = snmp_parse_args(argc, argv, &session, "C:", optProc)) {
    case NETSNMP_PARSE_ARGS_ERROR:
        exit(1);
    case NETSNMP_PARSE_ARGS_SUCCESS_EXIT:
        exit(0);
    case NETSNMP_PARSE_ARGS_ERROR_USAGE:
        usage();
        exit(1);
    default:
        break;
    }

    if (arg >= argc) {
        fprintf(stderr, "Please specify a destination host.\n");
        usage();
        exit(1);
    }

    SOCK_STARTUP;

    /*
     * open an SNMP session 
     */
    ss = snmp_open(&session);
    if (ss == NULL) {
        /*
         * diagnose snmp_open errors with the input netsnmp_session pointer 
         */
        snmp_sess_perror("snmpping", &session);
        exit(1);
    }

    if (session.securityModel == SNMP_SEC_MODEL_USM) {
        strncpy(username, session.securityName, sizeof(username) - 1);
        username[32] = '\0';
        usernameLen = strlen(username); /* TODO session.securityNameLen */
    } else {
        strncpy(username, getlogin(), sizeof(username) - 1);
        username[32] = '\0';
        usernameLen = strlen(username);
    }
    if (1 /* !have-testname-arg */) {
        snprintf(testname, sizeof(testname) - 1, "snmpping-%d", getpid());
        testname[32] = '\0';
        testnameLen = strlen(testname);
    }
    idx = index;
    *idx++ = usernameLen;
    p = username;
    for (i = 0; i < usernameLen; i++) {
        *idx++ = *p++;
    }
    *idx++ = testnameLen;
    p = testname;
    for (i = 0; i < testnameLen; i++) {
        *idx++ = *p++;
    }
    indexlen = idx - index;
    ret = start_ping( ss, index, indexlen, argv[ arg ] );
    if ( ret != 0 ) {
        return ret;
    }

    signal(SIGINT, sigint);

    wait_for_completion( ss, index, indexlen );
    overall_stats( ss, index, indexlen );
    cleanup_ctlTable( ss, index, indexlen );

    snmp_close(ss);
    SOCK_CLEANUP;
    return 0;
}
