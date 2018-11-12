/*
 * snmpps.c - display process table on a network entity via SNMP.
 *
 */

/* Portions of this file are subject to the following copyright(s).  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 */
/***********************************************************************
        Copyright 1988, 1989, 1991, 1992 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of CMU not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/

#include <net-snmp/net-snmp-config.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <sys/types.h>
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#include <stdio.h>
#include <ctype.h>
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#elif HAVE_CURSES_H
#include <curses.h>
#endif
#include <signal.h>

#include <net-snmp/net-snmp-includes.h>

void
usage(void)
{
    fprintf(stderr, "Usage: snmpps [-Cp] [-Ca] [-C m | n | t] AGENT");
    snmp_parse_args_usage(stderr);
    fprintf(stderr, "\n\n");
    snmp_parse_args_descriptions(stderr);
    fprintf(stderr, "\nsnmpps options:\n");
    fprintf(stderr,
            "\t-Cp\tShow hrSWRunPath instead of hrSWRunName\n");
    fprintf(stderr,
            "\t-Ca\tShow hrSWRunParameters in addition to hrSWRunName/Path\n");
    fprintf(stderr,
            "\t-Ct\tSort processes according to CPU time used\n");
    fprintf(stderr,
            "\t-Cm\tSort processes according to memory usage\n");
    fprintf(stderr,
            "\t-Cn\tSort processes by PID number (default)\n");
}

int   command_args = 0,
      command_path = 0;
int   topsort = 'c';
char *progname;

static void
optProc(int argc, char *const *argv, int opt)
{
    switch (opt) {
    case 'C':
        while (*optarg) {
            switch (*optarg++) {
            case 'a':
                command_args = 1;
                break;
            case 'p':
                command_path = 1;
                break;
            case 'm':
                topsort = 'm';
                break;
            case 'n':
                topsort = 'n';
                break;
            case 't':
                topsort = 't';
                break;

            default:
                fprintf(stderr,
                        "Unknown flag passed to -C: %c\n", optarg[-1]);
                exit(1);
            }
        }
    }
}

struct hrSWRunTable {
    u_long  hrSWRunIndex;
    char   *hrSWRunName;
    char   *hrSWRunPath;
    char   *hrSWRunParameters;
    u_long  hrSWRunID;
    u_long  hrSWRunType;
    u_long  hrSWRunStatus;
    u_long  hrSWRunPerfMem;
    u_long  hrSWRunPerfCPU;
    u_long  hrSWRunPerfCPUInc;
};

struct cpuStats {
    u_long user;
    u_long nice;
    u_long system;
    u_long idle;
    u_long wait;
    u_long kernel;
    u_long intr;
    u_long softintr;
    u_long steal;
    u_long guest;
    u_long guestnice;
};

struct memStats {
    u_long totalSwap;
    u_long availSwap;
    u_long totalReal;
    u_long availReal;
    u_long shared;
    u_long buffer;
    u_long cached;
};

struct laStats {
    u_long la1;
    u_long la5;
    u_long la15;
};


int
add(netsnmp_pdu *pdu, const char *mibnodename,
    oid * index, size_t indexlen)
{
    oid             base[MAX_OID_LEN];
    size_t          base_length = MAX_OID_LEN;

    memset(base, 0, MAX_OID_LEN * sizeof(oid));

    if (!snmp_parse_oid(mibnodename, base, &base_length)) {
        snmp_perror(mibnodename);
        fprintf(stderr, "couldn't find mib node %s, giving up\n",
                mibnodename);
#if HAVE_CURSES_H
        endwin();
#endif
        exit(1);
    }

    if (index && indexlen) {
        memcpy(&(base[base_length]), index, indexlen * sizeof(oid));
        base_length += indexlen;
    }
    DEBUGMSGTL(("add", "created: "));
    DEBUGMSGOID(("add", base, base_length));
    DEBUGMSG(("add", "\n"));
    snmp_add_null_var(pdu, base, base_length);

    return base_length;
}

netsnmp_variable_list *
collect_procs(netsnmp_session *ss, netsnmp_pdu *pdu,
        oid * base, size_t base_length)
{
    netsnmp_pdu    *response;
    int             running = 1;
    netsnmp_variable_list *saved = NULL, **vlpp = &saved;
    int             status;

    while (running) {
        /*
         * gotta catch em all, gotta catch em all! 
         */
        status = snmp_synch_response(ss, pdu, &response);
        if (status != STAT_SUCCESS || !response) {
            snmp_sess_perror(progname, ss);
#if HAVE_CURSES_H
            endwin();
#endif
            exit(1);
        }
        if (response->errstat != SNMP_ERR_NOERROR) {
            fprintf(stderr, "%s: Error in packet: %s\n", progname,
                    snmp_errstring(response->errstat));
#if HAVE_CURSES_H
            endwin();
#endif
            exit(1);
        }
        if (snmp_oid_compare(response->variables->name,
                             SNMP_MIN(base_length,
                                      response->variables->name_length),
                             base, base_length) != 0)
            running = 0;
        else if (response->variables->type == SNMP_NOSUCHINSTANCE ||
                    response->variables->type == SNMP_NOSUCHOBJECT ||
                response->variables->type == SNMP_ENDOFMIBVIEW)
            running = 0;
        else {
            /*
             * get response 
             */
            *vlpp = response->variables;
            (*vlpp)->next_variable = NULL;      /* shouldn't be any, but just in case */

            /*
             * create the next request 
             */
            pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
            snmp_add_null_var(pdu, (*vlpp)->name, (*vlpp)->name_length);

            /*
             * finish loop setup 
             */
            vlpp = &((*vlpp)->next_variable);
            response->variables = NULL; /* ahh, forget about it */
        }
        snmp_free_pdu(response);
    }
    return saved;
}


int
collect_perf(netsnmp_session *ss, struct hrSWRunTable **fproc)
{
    netsnmp_pdu    *pdu;
    netsnmp_pdu    *response;
    netsnmp_variable_list *vlp;
    oid             base[MAX_OID_LEN];
    size_t          base_length;
    int status, count = 0;
    struct hrSWRunTable *procs = malloc(sizeof(struct hrSWRunTable));

    pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
    base_length =
        add(pdu, "HOST-RESOURCES-MIB:hrSWRunIndex", NULL, 0);
    memcpy(base, pdu->variables->name, base_length * sizeof(oid));

    vlp = collect_procs(ss, pdu, base, base_length);

    while (vlp) {
        size_t len;
        struct hrSWRunTable proc;
        netsnmp_variable_list *vlp2;

        pdu = snmp_pdu_create(SNMP_MSG_GET);

        add(pdu, "HOST-RESOURCES-MIB:hrSWRunName",
            &vlp->name[base_length], vlp->name_length - base_length);
        add(pdu, "HOST-RESOURCES-MIB:hrSWRunID",
            &vlp->name[base_length], vlp->name_length - base_length);
        add(pdu, "HOST-RESOURCES-MIB:hrSWRunPath",
            &vlp->name[base_length], vlp->name_length - base_length);
        add(pdu, "HOST-RESOURCES-MIB:hrSWRunParameters",
            &vlp->name[base_length], vlp->name_length - base_length);
        add(pdu, "HOST-RESOURCES-MIB:hrSWRunType",
            &vlp->name[base_length], vlp->name_length - base_length);
        add(pdu, "HOST-RESOURCES-MIB:hrSWRunStatus",
            &vlp->name[base_length], vlp->name_length - base_length);
        add(pdu, "HOST-RESOURCES-MIB:hrSWRunPerfCPU",
            &vlp->name[base_length], vlp->name_length - base_length);
        add(pdu, "HOST-RESOURCES-MIB:hrSWRunPerfMem",
            &vlp->name[base_length], vlp->name_length - base_length);

        response = NULL;
        status = snmp_synch_response(ss, pdu, &response);
        if (status != STAT_SUCCESS || !response) {
            snmp_sess_perror(progname, ss);
#if HAVE_CURSES_H
            endwin();
#endif
            exit(1);
        }
        if (response->errstat != SNMP_ERR_NOERROR) {
            vlp = vlp->next_variable;
            continue;
        }

        memset(&proc, 0, sizeof(proc));

        proc.hrSWRunIndex = vlp->name[base_length];

        vlp2 = response->variables;
        if (vlp2->type == SNMP_NOSUCHINSTANCE) goto next;
        len = vlp2->val_len;
        proc.hrSWRunName = malloc(len+1);
        memcpy(proc.hrSWRunName, vlp2->val.string, len);
        proc.hrSWRunName[len] = '\0';

        vlp2 = vlp2->next_variable;
        if (vlp2->type == SNMP_NOSUCHINSTANCE) goto next;
        proc.hrSWRunID = *vlp2->val.integer;

        vlp2 = vlp2->next_variable;
        if (vlp2->type == SNMP_NOSUCHINSTANCE) goto next;
        len = vlp2->val_len;
        proc.hrSWRunPath = malloc(len+1);
        memcpy(proc.hrSWRunPath, vlp2->val.string, len);
        proc.hrSWRunPath[len] = '\0';

        vlp2 = vlp2->next_variable;
        if (vlp2->type == SNMP_NOSUCHINSTANCE) goto next;
        len = vlp2->val_len;
        proc.hrSWRunParameters = malloc(len+1);
        memcpy(proc.hrSWRunParameters, vlp2->val.string, len);
        proc.hrSWRunParameters[len] = '\0';

        vlp2 = vlp2->next_variable;
        if (vlp2->type == SNMP_NOSUCHINSTANCE) goto next;
        proc.hrSWRunType = *vlp2->val.integer;

        vlp2 = vlp2->next_variable;
        if (vlp2->type == SNMP_NOSUCHINSTANCE) goto next;
        proc.hrSWRunStatus = *vlp2->val.integer;

        vlp2 = vlp2->next_variable;
        if (vlp2->type == SNMP_NOSUCHINSTANCE) goto next;
        proc.hrSWRunPerfCPU = *vlp2->val.integer;

        vlp2 = vlp2->next_variable;
        if (vlp2->type == SNMP_NOSUCHINSTANCE) goto next;
        proc.hrSWRunPerfMem = *vlp2->val.integer;

        count++;
        procs = realloc(procs, count*sizeof(procs[0]));
        procs[count-1] = proc;

        snmp_free_pdu(response);
        vlp2 = vlp;
        vlp = vlp->next_variable;
        free(vlp2);
        continue;

next:
        if (proc.hrSWRunName) free(proc.hrSWRunName);
        if (proc.hrSWRunPath) free(proc.hrSWRunPath);
        if (proc.hrSWRunParameters) free(proc.hrSWRunParameters);
        snmp_free_pdu(response);
        vlp2 = vlp;
        vlp = vlp->next_variable;
        free(vlp2);
    }
    *fproc = procs;
    return count;
}


void free_perf(struct hrSWRunTable *procs, int nproc)
{
    int i;
    for (i = 0; i < nproc; i++) {
        struct hrSWRunTable *proc = procs+i;
        if (proc->hrSWRunName) free(proc->hrSWRunName);
        if (proc->hrSWRunPath) free(proc->hrSWRunPath);
        if (proc->hrSWRunParameters) free(proc->hrSWRunParameters);
    }
    free(procs);
}


int collect_cpu(netsnmp_session *ss, struct cpuStats *cpu)
{
    netsnmp_pdu    *pdu;
    netsnmp_pdu    *response;
    int status;
    int ret = 0;

    pdu = snmp_pdu_create(SNMP_MSG_GET);
    add(pdu, "UCD-SNMP-MIB:ssCpuRawUser.0", NULL, 0);
    add(pdu, "UCD-SNMP-MIB:ssCpuRawNice.0", NULL, 0);
    add(pdu, "UCD-SNMP-MIB:ssCpuRawSystem.0", NULL, 0);
    add(pdu, "UCD-SNMP-MIB:ssCpuRawIdle.0", NULL, 0);
    add(pdu, "UCD-SNMP-MIB:ssCpuRawWait.0", NULL, 0);
    add(pdu, "UCD-SNMP-MIB:ssCpuRawKernel.0", NULL, 0);
    add(pdu, "UCD-SNMP-MIB:ssCpuRawInterrupt.0", NULL, 0);
    add(pdu, "UCD-SNMP-MIB:ssCpuRawSoftIRQ.0", NULL, 0);
    add(pdu, "UCD-SNMP-MIB:ssCpuRawSteal.0", NULL, 0);
    add(pdu, "UCD-SNMP-MIB:ssCpuRawGuest.0", NULL, 0);
    add(pdu, "UCD-SNMP-MIB:ssCpuRawGuestNice.0", NULL, 0);

    status = snmp_synch_response(ss, pdu, &response);
    memset(cpu, 0, sizeof(*cpu));
    if (status != STAT_SUCCESS || !response ||
            response->errstat != SNMP_ERR_NOERROR) {
        goto out;
    }
    else {
        netsnmp_variable_list *vlp = response->variables;
        if (vlp->type == SNMP_NOSUCHOBJECT) goto out;
        cpu->user = *vlp->val.integer;
        vlp = vlp->next_variable;
        if (vlp->type == SNMP_NOSUCHOBJECT) goto out;
        cpu->nice = *vlp->val.integer;
        vlp = vlp->next_variable;
        if (vlp->type == SNMP_NOSUCHOBJECT) goto out;
        cpu->system = *vlp->val.integer;
        vlp = vlp->next_variable;
        if (vlp->type == SNMP_NOSUCHOBJECT) goto out;
        cpu->idle = *vlp->val.integer;
        vlp = vlp->next_variable;
        if (vlp->type == SNMP_NOSUCHOBJECT) goto out;
        cpu->wait = *vlp->val.integer;
        vlp = vlp->next_variable;
        if (vlp->type == SNMP_NOSUCHOBJECT) goto out;
        cpu->kernel = *vlp->val.integer;
        vlp = vlp->next_variable;

        ret = 1;

        if (vlp->type == SNMP_NOSUCHOBJECT) goto out;
        cpu->intr = *vlp->val.integer;
        vlp = vlp->next_variable;
        if (vlp->type == SNMP_NOSUCHOBJECT) goto out;
        cpu->softintr = *vlp->val.integer;
        vlp = vlp->next_variable;
        if (vlp->type == SNMP_NOSUCHOBJECT) goto out;
        cpu->steal = *vlp->val.integer;
        vlp = vlp->next_variable;
        if (vlp->type == SNMP_NOSUCHOBJECT) goto out;
        cpu->guest = *vlp->val.integer;
        vlp = vlp->next_variable;
        if (vlp->type == SNMP_NOSUCHOBJECT) goto out;
        cpu->guestnice = *vlp->val.integer;
    }
out:
    if (response) snmp_free_pdu(response);
    return ret;
}


int collect_mem(netsnmp_session *ss, struct memStats *mem)
{
    netsnmp_pdu    *pdu;
    netsnmp_pdu    *response;
    int status;
    int ret = 0;

    pdu = snmp_pdu_create(SNMP_MSG_GET);
    add(pdu, "UCD-SNMP-MIB:memTotalSwap.0", NULL, 0);
    add(pdu, "UCD-SNMP-MIB:memAvailSwap.0", NULL, 0);
    add(pdu, "UCD-SNMP-MIB:memTotalReal.0", NULL, 0);
    add(pdu, "UCD-SNMP-MIB:memAvailReal.0", NULL, 0);
    add(pdu, "UCD-SNMP-MIB:memShared.0", NULL, 0);
    add(pdu, "UCD-SNMP-MIB:memBuffer.0", NULL, 0);
    add(pdu, "UCD-SNMP-MIB:memCached.0", NULL, 0);

    status = snmp_synch_response(ss, pdu, &response);
    memset(mem, 0, sizeof(*mem));
    if (status != STAT_SUCCESS || !response ||
            response->errstat != SNMP_ERR_NOERROR) {
        goto out;
    }
    else {
        netsnmp_variable_list *vlp = response->variables;
        if (vlp->type == SNMP_NOSUCHOBJECT) goto out;
        mem->totalSwap = *vlp->val.integer;
        vlp = vlp->next_variable;
        if (vlp->type == SNMP_NOSUCHOBJECT) goto out;
        mem->availSwap = *vlp->val.integer;
        vlp = vlp->next_variable;
        if (vlp->type == SNMP_NOSUCHOBJECT) goto out;
        mem->totalReal = *vlp->val.integer;
        vlp = vlp->next_variable;
        if (vlp->type == SNMP_NOSUCHOBJECT) goto out;
        mem->availReal = *vlp->val.integer;
        vlp = vlp->next_variable;

        ret = 1;

        if (vlp->type == SNMP_NOSUCHOBJECT) goto out;
        mem->shared = *vlp->val.integer;
        vlp = vlp->next_variable;
        if (vlp->type == SNMP_NOSUCHOBJECT) goto out;
        mem->buffer = *vlp->val.integer;
        vlp = vlp->next_variable;
        if (vlp->type == SNMP_NOSUCHOBJECT) goto out;
        mem->cached = *vlp->val.integer;
    }
out:
    if (response) snmp_free_pdu(response);
    return ret;
}


char *format_centisec(char *buf, size_t len, unsigned long csec)
{
    long sec, min, hour, day;

    day = csec / 100 / 3600 / 24;
    csec -= day * 100 * 3600 * 24;
    hour = csec / 100 / 3600;
    csec -= hour * 100 * 3600;
    min = csec / 100 / 60;
    csec -= min * 100 * 60;
    sec = csec / 100;
    csec -= sec * 100;

    if (day)
        snprintf(buf, len, "%ldd%02ld:%02ld:%02ld.%02lu", day, hour, min, sec, csec);
    else if (hour)
        snprintf(buf, len, "%ld:%02ld:%02ld.%02lu", hour, min, sec, csec);
    else if (min)
        snprintf(buf, len, "%ld:%02ld.%02lu", min, sec, csec);
    else
        snprintf(buf, len, "%ld.%02lu", sec, csec);
    return buf;
}


char *format_sec(char *buf, size_t len, unsigned long csec)
{
    long sec, min, hour, day;

    day = csec / 100 / 3600 / 24;
    csec -= day * 100 * 3600 * 24;
    hour = csec / 100 / 3600;
    csec -= hour * 100 * 3600;
    min = csec / 100 / 60;
    csec -= min * 100 * 60;
    sec = csec / 100;
    csec -= sec * 100;

    if (day)
        snprintf(buf, len, "%ldd%02ld:%02ld:%02ld", day, hour, min, sec);
    else if (hour)
        snprintf(buf, len, "%ld:%02ld:%02ld.%02lu", hour, min, sec, csec);
    else if (min)
        snprintf(buf, len, "%ld:%02ld.%02lu", min, sec, csec);
    else
        snprintf(buf, len, "%ld.%02lu", sec, csec);
    return buf;
}


char *format_humanmem(char *buf, size_t len, unsigned long mem)
{
    if (mem >= 1024*1024)
        snprintf(buf, len, "%4.2fGB", (float)mem/(1024*1024));
    else if (mem >= 1024)
        snprintf(buf, len, "%4.2fMB", (float)mem/1024);
    else
        snprintf(buf, len, "%4.2fkB", (float)mem);
    return buf;
}


int cpucomp(const void *v1, const void *v2)
{
    const struct hrSWRunTable *p1 = v1, *p2 = v2;
    return p2->hrSWRunPerfCPUInc - p1->hrSWRunPerfCPUInc;
}


int memcomp(const void *v1, const void *v2)
{
    const struct hrSWRunTable *p1 = v1, *p2 = v2;
    return p2->hrSWRunPerfMem - p1->hrSWRunPerfMem;
}


int totcomp(const void *v1, const void *v2)
{
    const struct hrSWRunTable *p1 = v1, *p2 = v2;
    return p2->hrSWRunPerfCPU - p1->hrSWRunPerfCPU;
}


int pidcomp(const void *v1, const void *v2)
{
    const struct hrSWRunTable *p1 = v1, *p2 = v2;
    return p1->hrSWRunIndex - p2->hrSWRunIndex;
}


int
snmpps(int argc, char *argv[])
{
    netsnmp_session session, *ss;
    int             arg;
    struct hrSWRunTable *procs;
    int             count, pinx = 0;

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

    if (arg != argc) {
        fprintf(stderr, "snmpps: extra argument: %s\n", argv[arg]);
        exit(1);
    }

    SOCK_STARTUP;

    /*
     * Open an SNMP session.
     */
    ss = snmp_open(&session);
    if (ss == NULL) {
        /*
         * diagnose snmp_open errors with the input netsnmp_session pointer 
         */
        snmp_sess_perror("snmpps", &session);
        SOCK_CLEANUP;
        exit(1);
    }

    count = collect_perf(ss, &procs);
    if (count == 0) {
        fprintf(stderr, "snmpps: no processes found\n");
        exit(1);
    }

    switch (topsort) {
    case 'm':
        qsort(procs, count, sizeof(procs[0]), memcomp);
        break;
    case 't':
        qsort(procs, count, sizeof(procs[0]), totcomp);
        break;
    }

    printf("%7s %4s %6s %10s %11s %-10s\n",
        "Index", "Type", "Status", "Memory", "CPU", "Command");

    while (pinx < count) {
        struct hrSWRunTable *proc = procs+pinx;
        const char *hr_status, *hr_type;
        char b1[15], b2[20];

        switch (proc->hrSWRunType) {
        case 1: hr_type = "Unkn"; break;
        case 2: hr_type = "Os"; break;
        case 3: hr_type = "Drvr"; break;
        case 4: hr_type = "Appl"; break;
        default: hr_type = "?"; break;
        }

        switch (proc->hrSWRunStatus) {
        case 1: hr_status = "Run"; break;
        case 2: hr_status = "Wait"; break;
        case 3: hr_status = "Event"; break;
        case 4: hr_status = "Inval"; break;
        default: hr_status = "?"; break;
        }

        printf("%7lu %4s %6s %10s %11.11s %s %s\n",
               proc->hrSWRunIndex,
               hr_type,
               hr_status,
               format_humanmem(b1, sizeof b1, proc->hrSWRunPerfMem),
               format_centisec(b2, sizeof b2, proc->hrSWRunPerfCPU),
               command_path && proc->hrSWRunPath[0] ? proc->hrSWRunPath : proc->hrSWRunName,
               command_args ? proc->hrSWRunParameters : "");

        pinx++;
    }

    snmp_close(ss);
    SOCK_CLEANUP;
    return 0;
}


#if HAVE_CURSES_H
static void endtop(int sig)
{
    endwin();
    exit(1);
}


int snmptop(int argc, char **argv)
{
    netsnmp_session session, *ss;
    int             arg;
    struct hrSWRunTable *oproc;
    int             ocount = 0;
    int             show_idle = 1;
    int             show_os = 1;
    char            ch;
    struct cpuStats oldCpu;
    struct memStats mem;
    int             has_cpu, has_mem; 

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

    if (arg != argc) {
        fprintf(stderr, "snmptop: extra argument: %s\n", argv[arg]);
        exit(1);
    }

    SOCK_STARTUP;

    /*
     * Open an SNMP session.
     */
    ss = snmp_open(&session);
    if (ss == NULL) {
        /*
         * diagnose snmp_open errors with the input netsnmp_session pointer 
         */
        snmp_sess_perror("snmptop", &session);
        SOCK_CLEANUP;
        exit(1);
    }

    ocount = collect_perf(ss, &oproc);
    if (ocount == 0) {
        fprintf(stderr, "snmptop: no processes found\n");
        exit(1);
    }

    collect_cpu(ss, &oldCpu);

    signal(SIGINT, endtop);
    initscr();
    cbreak();
    noecho();
    nonl();
    halfdelay(50);

    while ((ch = getch()) != 'q') {
        int ncount;
        struct hrSWRunTable *nproc;
        int oinx = 0, ninx = 0, line = 0;
        netsnmp_pdu    *pdu;
        netsnmp_pdu    *response = NULL;
        int status;
        time_t clock;
        struct tm *ptm;
        char uptime[40];
        char timestr[40];
        char b1[15], b2[15], b3[15], b4[15];
        struct cpuStats newCpu;

        if (ch == 'c' || ch == 'm' || ch == 'n' || ch == 't') topsort = ch;
        if (ch == 'i') show_idle = !show_idle;
        if (ch == 'o') show_os = !show_os;
        if (ch == 'a') command_args = !command_args;
        if (ch == 'p') command_path = !command_path;

        ncount = collect_perf(ss, &nproc);

        while (oinx < ocount && ninx < ncount) {
            if (oproc[oinx].hrSWRunIndex == nproc[ninx].hrSWRunIndex) {
                nproc[ninx].hrSWRunPerfCPUInc = nproc[ninx].hrSWRunPerfCPU-oproc[oinx].hrSWRunPerfCPU;
                ninx++;
                oinx++;
            }
            else if (nproc[oinx].hrSWRunIndex < oproc[ninx].hrSWRunIndex)
                oinx++;
            else {
                nproc[ninx].hrSWRunPerfCPUInc = nproc[ninx].hrSWRunPerfCPU;
                ninx++;
            }
        }
        while (ninx < ncount) {
            nproc[ninx].hrSWRunPerfCPUInc = nproc[ninx].hrSWRunPerfCPU;
            ninx++;
        }

        switch (topsort) {
        case 'c':
            qsort(nproc, ncount, sizeof(nproc[0]), cpucomp);
            break;
        case 'm':
            qsort(nproc, ncount, sizeof(nproc[0]), memcomp);
            break;
        case 't':
            qsort(nproc, ncount, sizeof(nproc[0]), totcomp);
            break;
        }

        has_cpu = collect_cpu(ss, &newCpu);
        has_mem = collect_mem(ss, &mem);

        pdu = snmp_pdu_create(SNMP_MSG_GET);
        add(pdu, "HOST-RESOURCES-MIB:hrSystemUptime.0", NULL, 0);
        status = snmp_synch_response(ss, pdu, &response);
        if (status != STAT_SUCCESS || !response ||
                response->errstat != SNMP_ERR_NOERROR) {
            uptime[0] = '\0';
        }
        else {
            netsnmp_variable_list *vlp = response->variables;
            if (vlp->type == SNMP_NOSUCHINSTANCE) abort();
            uptime_string_n(*vlp->val.integer, uptime, sizeof(uptime));
        }
        snmp_free_pdu(response);

        clock = time(NULL);
        ptm = localtime(&clock);
        strftime(timestr, sizeof(timestr), "%H:%M:%S", ptm);

        clear();
        move(0, 0);
        printw("%s %s%s", session.peername, uptime[0] ? "up " : "", uptime);
        move(0, COLS-strlen(timestr)-1);
        printw("%s", timestr);
        if (has_cpu) {
            struct cpuStats deltaCpu;
            u_long sumCpu;

            deltaCpu.user = newCpu.user - oldCpu.user;
            deltaCpu.nice = newCpu.nice - oldCpu.nice;
            deltaCpu.system = newCpu.system - oldCpu.system;
            deltaCpu.idle = newCpu.idle - oldCpu.idle;
            deltaCpu.wait = newCpu.wait - oldCpu.wait;
            deltaCpu.kernel = newCpu.kernel - oldCpu.kernel;
            deltaCpu.intr = newCpu.intr - oldCpu.intr;
            deltaCpu.softintr = newCpu.softintr - oldCpu.softintr;
            deltaCpu.steal = newCpu.steal - oldCpu.steal;
            deltaCpu.guest = newCpu.guest - oldCpu.guest;
            deltaCpu.guestnice = newCpu.guestnice - oldCpu.guestnice;
            oldCpu = newCpu;
            sumCpu = deltaCpu.user + deltaCpu.nice
                + deltaCpu.system + deltaCpu.idle
                + deltaCpu.wait + deltaCpu.kernel + deltaCpu.steal
                + deltaCpu.intr + deltaCpu.softintr
                + deltaCpu.guest + deltaCpu.guestnice;

            printw("\nCPU%%: %4.1fUs %4.1fSy %4.1fId %3.1fWa %3.1fNi %3.1fKe %3.1fHi %3.1fSi %3.1fSt %3.1fGu %3.1fGN",
                (float)deltaCpu.user*100/sumCpu,
                (float)deltaCpu.system*100/sumCpu,
                (float)deltaCpu.idle*100/sumCpu,
                (float)deltaCpu.wait*100/sumCpu,
                (float)deltaCpu.nice*100/sumCpu,
                (float)deltaCpu.kernel*100/sumCpu,
                (float)deltaCpu.intr*100/sumCpu,
                (float)deltaCpu.softintr*100/sumCpu,
                (float)deltaCpu.steal*100/sumCpu,
                (float)deltaCpu.guest*100/sumCpu,
                (float)deltaCpu.guestnice*100/sumCpu);
            line++;
        }

        if (has_mem) {
            printw("\nMem:  %10s Total %10s Used %10s Free %10s Buffer",
                format_humanmem(b1, sizeof b1, mem.totalReal),
                format_humanmem(b2, sizeof b2, mem.totalReal-mem.availReal),
                format_humanmem(b3, sizeof b3, mem.availReal),
                format_humanmem(b4, sizeof b4, mem.buffer));
            line++;
            printw("\nSwap: %10s Total %10s Used %10s Free %10s Cached",
                format_humanmem(b1, sizeof b1, mem.totalSwap),
                format_humanmem(b2, sizeof b2, mem.totalSwap-mem.availSwap),
                format_humanmem(b3, sizeof b3, mem.availSwap),
                format_humanmem(b4, sizeof b4, mem.cached));
            line++;
        }

        printw("\n%7s %4s %6s %10s %11s %5s %-10s",
            "Index", "Type", "Status", "Memory", "Total CPU", "%CPU", "Command");
        line++;
        ninx = 0;
        while (line < LINES && ninx < ncount) {
            struct hrSWRunTable *proc = nproc+ninx;
            const char *hr_status, *hr_type;

            ninx++;
            if (proc->hrSWRunPerfCPUInc == 0 && !show_idle)
                continue;
            if (proc->hrSWRunType != 4 && !show_os)
                continue;

            line++;

            switch (proc->hrSWRunType) {
            case 1: hr_type = "Unkn"; break;
            case 2: hr_type = "Os"; break;
            case 3: hr_type = "Drvr"; break;
            case 4: hr_type = "Appl"; break;
            default: hr_type = "?"; break;
            }

            switch (proc->hrSWRunStatus) {
            case 1: hr_status = "Run"; break;
            case 2: hr_status = "Wait"; break;
            case 3: hr_status = "Event"; break;
            case 4: hr_status = "Inval"; break;
            default: hr_status = "?"; break;
            }

            printw("\n%7lu %4s %6s %10s %11s %5.1f %s %s",
                   proc->hrSWRunIndex,
                   hr_type,
                   hr_status,
                   format_humanmem(b1, sizeof b1, proc->hrSWRunPerfMem),
                   format_sec(b2,sizeof b2, proc->hrSWRunPerfCPU),
                   (float)proc->hrSWRunPerfCPUInc/5,
                   command_path && proc->hrSWRunPath[0] ? proc->hrSWRunPath : proc->hrSWRunName,
                   command_args ? proc->hrSWRunParameters : "");
        }
        refresh();

        qsort(nproc, ncount, sizeof(nproc[0]), pidcomp);
        free_perf(oproc, ocount);
        oproc = nproc;
        ocount = ncount;
    }
    endwin();

    snmp_close(ss);
    SOCK_CLEANUP;
    return 0;
}
#else
int snmptop(int argc, char **argv)
{
    fprintf(stderr, "%s: curses not detected during configure\n", progname);
    return 1;
}
#endif


int main(int argc, char **argv)
{
    progname = strrchr(argv[0], '/');
    if (progname) progname++;
    else progname = argv[0];
    if (strncmp(progname, "lt-", 3) == 0) progname += 3;

    if (strcmp(progname, "snmpps") == 0) return snmpps(argc, argv);
    if (strcmp(progname, "snmptop") == 0) return snmptop(argc, argv);
    fprintf(stderr, "%s: unknown prognam name\n", progname);
    exit(1);
}
