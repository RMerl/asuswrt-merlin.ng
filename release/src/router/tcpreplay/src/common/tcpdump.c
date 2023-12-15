/* $Id$ */

/*
 *   Copyright (c) 2001-2010 Aaron Turner <aturner at synfin dot net>
 *   Copyright (c) 2013-2022 Fred Klassen <tcpreplay at appneta dot com> - AppNeta
 *
 *   The Tcpreplay Suite of tools is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation, either version 3 of the
 *   License, or with the authors permission any later version.
 *
 *   The Tcpreplay Suite is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with the Tcpreplay Suite.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This code allows us to use tcpdump to print packet decodes.
 * Basically, we create a local AF_UNIX socketpair, fork a copy
 * of ourselves, link 1/2 of the pair to STDIN of the child and
 * replace the child with tcpdump.  We then send a "pcap" file
 * over the socket so that tcpdump can print it's decode to STDOUT.
 *
 * Idea and a lot of code stolen from Christain Kreibich's
 *  <christian@whoop.org> libnetdude 0.4 code.  Any bugs are mine. :)
 *
 * This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors
 */

#include "defines.h"
#include "config.h"
#include "common.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#include "tcpdump.h"

char *options_vec[OPTIONS_VEC_SIZE];
static int tcpdump_fill_in_options(char *opt);
static int can_exec(const char *filename);

/**
 * given a packet, print a decode of via tcpdump
 */
int
tcpdump_print(tcpdump_t *tcpdump, struct pcap_pkthdr *pkthdr, const u_char *data)
{
    struct pollfd poller;
    int res, total;
    char decode[TCPDUMP_DECODE_LEN];
    struct compact_pkthdr {
        struct {
            uint32_t ts_sec;
            uint32_t ts_usec;
        } ts;
        uint32_t caplen; /* length of portion present */
        uint32_t len;    /* length this packet (off wire) */
    } actual_pkthdr;

    assert(tcpdump);
    assert(pkthdr);
    assert(data);

    /* convert header to file-format packet header */
    actual_pkthdr.ts.ts_sec = (uint32_t)pkthdr->ts.tv_sec;
    actual_pkthdr.ts.ts_usec = (uint32_t)pkthdr->ts.tv_sec;
    actual_pkthdr.caplen = pkthdr->caplen;
    actual_pkthdr.len = pkthdr->len;

    total = 0;
header_again:
    poller.fd = PARENT_WRITE_FD;
    poller.events = POLLOUT;
    poller.revents = 0;

    /* wait until we can write the header to the tcpdump pipe */
    res = poll(&poller, 1, TCPDUMP_POLL_TIMEOUT);
    if (res < 0)
        errx(-1,
             "Error writing header to fd %d during poll() to write to tcpdump\n%s",
             PARENT_WRITE_FD,
             strerror(errno));

    if (res == 0)
        err(-1,
            "poll() timeout... tcpdump seems to be having a problem keeping up\n"
            "Try increasing TCPDUMP_POLL_TIMEOUT");

#ifdef DEBUG
    if (debug >= 5) {
        if (write(tcpdump->debugfd, (char *)&actual_pkthdr, sizeof(actual_pkthdr)) != sizeof(actual_pkthdr))
            errx(-1, "Error writing pcap file header to tcpdump debug\n%s", strerror(errno));
    }
#endif
    /* res > 0 if we get here */
    while (total != sizeof(actual_pkthdr) &&
           (res = (int)write(PARENT_WRITE_FD, &actual_pkthdr + total, sizeof(actual_pkthdr) - total))) {
        if (res < 0) {
            if (errno == EAGAIN)
                goto header_again;

            errx(-1, "Error writing pcap file header to tcpdump\n%s", strerror(errno));
        }

        total += res;
    }

    total = 0;
data_again:
    /* wait until we can write data to the tcpdump pipe */
    poller.fd = PARENT_WRITE_FD;
    poller.events = POLLOUT;
    poller.revents = 0;

    res = poll(&poller, 1, TCPDUMP_POLL_TIMEOUT);
    if (res < 0)
        errx(-1, "Error writing to fd %d during poll() to write to tcpdump\n%s", PARENT_WRITE_FD, strerror(errno));

    if (res == 0)
        err(-1,
            "poll() timeout... tcpdump seems to be having a problem keeping up\n"
            "Try increasing TCPDUMP_POLL_TIMEOUT");

#ifdef DEBUG
    if (debug >= 5) {
        if (write(tcpdump->debugfd, data, pkthdr->caplen) != (ssize_t)pkthdr->caplen)
            errx(-1, "Error writing packet data to tcpdump debug\n%s", strerror(errno));
    }
#endif

    while (total != (ssize_t)pkthdr->caplen &&
           (res = (int)write(PARENT_WRITE_FD, data + total, pkthdr->caplen - total))) {
        if (res < 0) {
            if (errno == EAGAIN)
                goto data_again;

            errx(-1, "Error writing packet data to tcpdump\n%s", strerror(errno));
        }

        total += res;
    }

    /* Wait for output from tcpdump */
    poller.fd = PARENT_READ_FD;
    poller.events = POLLIN;
    poller.revents = 0;

    res = poll(&poller, 1, TCPDUMP_POLL_TIMEOUT);
    if (res < 0)
        errx(-1, "Error out to fd %d during poll() to read from tcpdump\n%s", PARENT_READ_FD, strerror(errno));

    if (res == 0)
        err(-1,
            "poll() timeout... tcpdump seems to be having a problem keeping up\n"
            "Try increasing TCPDUMP_POLL_TIMEOUT");

    while ((res = (int)read(PARENT_READ_FD, decode, TCPDUMP_DECODE_LEN))) {
        if (res < 0) {
            if (errno == EAGAIN)
                break;

            errx(-1, "Error reading tcpdump decode: %s", strerror(errno));
        }

        decode[min(res, TCPDUMP_DECODE_LEN - 1)] = 0;
        dbgx(4, "read %d byte from tcpdump", res);
        printf("%s", decode);
    }

    return TRUE;
}

/**
 * init our tcpdump handle using the given pcap handle
 * Basically, this starts up tcpdump as a child and communicates
 * to it via a pair of sockets (stdout/stdin)
 */
int
tcpdump_open(tcpdump_t *tcpdump, pcap_t *pcap)
{
    assert(tcpdump);
    assert(pcap);

    if (tcpdump->pid != 0) {
        warn("tcpdump process already running");
        return FALSE;
    }

    /* is tcpdump executable? */
    if (!can_exec(TCPDUMP_BINARY)) {
        errx(-1, "Unable to execute tcpdump binary: %s", TCPDUMP_BINARY);
    }

#ifdef DEBUG
    strlcpy(tcpdump->debugfile, TCPDUMP_DEBUG, sizeof(tcpdump->debugfile));
    if (debug >= 5) {
        dbgx(5, "Opening tcpdump debug file: %s", tcpdump->debugfile);

        if ((tcpdump->debugfd =
                     open(tcpdump->debugfile, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE | S_IRGRP | S_IROTH)) ==
            -1) {
            errx(-1, "Error opening tcpdump debug file: %s\n%s", tcpdump->debugfile, strerror(errno));
        }
    }
#endif

    /* copy over the args */
    dbg(2, "Prepping tcpdump options...");
    tcpdump_fill_in_options(tcpdump->args);

    dbg(2, "Starting tcpdump...");

    /* create our pipe to send packet data to tcpdump via */
    if (pipe(tcpdump->pipes[PARENT_READ_PIPE]) < 0 || pipe(tcpdump->pipes[PARENT_WRITE_PIPE]) < 0)
        errx(-1, "Unable to create pipe: %s", strerror(errno));

    if ((tcpdump->pid = fork()) < 0)
        errx(-1, "Fork failed: %s", strerror(errno));

    dbgx(2, "tcpdump pid: %d", tcpdump->pid);

    if (tcpdump->pid > 0) {
        /* parent - we're still in tcpreplay */

        /* close fds not required by parent */
        dbgx(2, "[parent] closing child read/write fd %d/%d", CHILD_READ_FD, CHILD_WRITE_FD);
        close(CHILD_READ_FD);
        close(CHILD_WRITE_FD);
        CHILD_READ_FD = 0;
        CHILD_WRITE_FD = 0;

        /* send the pcap file header to tcpdump */
        FILE *writer = fdopen(PARENT_WRITE_FD, "w");
        if ((pcap_dump_fopen(pcap, writer)) == NULL) {
            warnx("[parent] pcap_dump_fopen(): %s", pcap_geterr(pcap));
            return FALSE;
        }

        pcap_dump_flush((pcap_dumper_t *)writer);

        if (fcntl(PARENT_WRITE_FD, F_SETFL, O_NONBLOCK) < 0)
            warnx("[parent] Unable to fcntl write pipe:\n%s", strerror(errno));

        if (fcntl(PARENT_READ_FD, F_SETFL, O_NONBLOCK) < 0)
            warnx("[parent] Unable to fnctl read pip:\n%s", strerror(errno));
    } else {
        dbg(2, "[child] started the kid");

        /* we're in the child process - run "tcpdump  <options> -r -" */
        if (dup2(CHILD_READ_FD, STDIN_FILENO) != STDIN_FILENO) {
            errx(-1, "[child] Unable to duplicate socket to stdin: %s", strerror(errno));
        }

        if (dup2(CHILD_WRITE_FD, STDOUT_FILENO) != STDOUT_FILENO) {
            errx(-1, "[child] Unable to duplicate socket to stdout: %s", strerror(errno));
        }

        /*
         * Close sockets not required by child. The exec'ed program must
         * not know that they ever existed.
         */
        dbgx(2, "[child] closing in fds %d/%d/%d/%d", CHILD_READ_FD, CHILD_WRITE_FD, PARENT_READ_FD, PARENT_WRITE_FD);
        close(CHILD_READ_FD);
        close(CHILD_WRITE_FD);
        close(PARENT_READ_FD);
        close(PARENT_WRITE_FD);

        /* exec tcpdump */
        dbg(2, "[child] Exec'ing tcpdump...");
        if (execv(TCPDUMP_BINARY, options_vec) < 0)
            errx(-1, "Unable to exec tcpdump: %s", strerror(errno));

        dbg(2, "[child] tcpdump done!");
    }

    return TRUE;
}

/**
 * shutdown tcpdump
 */
void
tcpdump_close(tcpdump_t *tcpdump)
{
    if (!tcpdump)
        return;

    if (tcpdump->pid <= 0)
        return;

    dbgx(2, "[parent] killing tcpdump pid: %d", tcpdump->pid);

    kill(tcpdump->pid, SIGKILL);
    close(PARENT_READ_FD);
    close(PARENT_WRITE_FD);

    if (waitpid(tcpdump->pid, NULL, 0) != tcpdump->pid)
        errx(-1, "[parent] Error in waitpid: %s", strerror(errno));

    tcpdump->pid = 0;
    PARENT_READ_FD = 0;
    PARENT_WRITE_FD = 0;
}

/**
 * copy the string of args (*opt) to the vector (**opt_vec)
 * for a max of opt_len.  Returns the number of options
 * in the vector
 */
static int
tcpdump_fill_in_options(char *opt)
{
    char options[256];
    char *arg, *newarg;
    int i = 1, arglen;
    char *token = NULL;

    /* zero out our options_vec for execv() */
    memset(options_vec, '\0', sizeof(options_vec));

    /* first arg should be the binary (by convention) */
    options_vec[0] = TCPDUMP_BINARY;

    /* prep args */
    memset(options, '\0', 256);
    if (opt != NULL) {
        strlcat(options, opt, sizeof(options));
    }
    strlcat(options, TCPDUMP_ARGS, sizeof(options));
    dbgx(2, "[child] Will execute: tcpdump %s", options);

    /* process args */

    /* process the first argument */
    arg = strtok_r(options, OPT_DELIM, &token);
    arglen = (int)strlen(arg) + 2; /* -{arg}\0 */
    newarg = (char *)safe_malloc(arglen);
    strlcat(newarg, "-", arglen);
    strlcat(newarg, arg, arglen);
    options_vec[i++] = newarg;

    /* process the remaining args
     * note that i < OPTIONS_VEC_SIZE - 1
     * because: a) we need to add '-' as an option to the end
     * b) because the array has to be null terminated
     */
    while (((arg = strtok_r(NULL, OPT_DELIM, &token)) != NULL) && (i < OPTIONS_VEC_SIZE - 1)) {
        arglen = (int)strlen(arg) + 2;
        newarg = (char *)safe_malloc(arglen);
        strlcat(newarg, "-", arglen);
        strlcat(newarg, arg, arglen);
        options_vec[i++] = newarg;
    }

    /* tell -r to read from stdin */
    options_vec[i] = "-";

    return i;
}

/**
 * can we exec the given file?
 */
static int
can_exec(const char *filename)
{
    struct stat st;

    assert (filename);

    if (filename[0] == '\0')
        return FALSE;

    /* Stat the file to see if it's executable and
       if the user may run it.
    */
    if (lstat(filename, &st) < 0)
        return FALSE;

    if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH))
        return TRUE;

    return FALSE;
}
