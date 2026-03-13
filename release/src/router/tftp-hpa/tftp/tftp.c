/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "common/tftpsubs.h"

/*
 * TFTP User Program -- Protocol Machines
 */
#include "extern.h"

extern union sock_addr peeraddr; /* filled in by main */
extern int f;                    /* the opened socket */
extern int trace;
extern int verbose;
extern int rexmtval;
extern int maxtimeout;

#define PKTSIZE    SEGSIZE+4
char ackbuf[PKTSIZE];
int timeout;
extern sigjmp_buf toplevel;
sigjmp_buf timeoutbuf;

static void nak(int, const char *);
static int makerequest(int, const char *, struct tftphdr *, const char *);
static void printstats(const char *, unsigned long);
static void startclock(void);
static void stopclock(void);
static void timer(int);
static void tpacket(const char *, struct tftphdr *, int);

/*
 * Send the requested file.
 */
void tftp_sendfile(int fd, const char *name, const char *mode)
{
    struct tftphdr *ap;         /* data and ack packets */
    struct tftphdr *dp;
    int n;
    volatile int is_request;
    volatile u_short block;
    volatile int size, convert;
    volatile off_t amount;
    union sock_addr from;
    socklen_t fromlen;
    FILE *file;
    u_short ap_opcode, ap_block;

    startclock();               /* start stat's clock */
    dp = r_init();              /* reset fillbuf/read-ahead code */
    ap = (struct tftphdr *)ackbuf;
    convert = !strcmp(mode, "netascii");
    file = fdopen(fd, convert ? "rt" : "rb");
    block = 0;
    is_request = 1;             /* First packet is the actual WRQ */
    amount = 0;

    bsd_signal(SIGALRM, timer);
    do {
        if (is_request) {
            size = makerequest(WRQ, name, dp, mode) - 4;
        } else {
            /*      size = read(fd, dp->th_data, SEGSIZE);   */
            size = readit(file, &dp, convert);
            if (size < 0) {
                nak(errno + 100, NULL);
                break;
            }
            dp->th_opcode = htons((u_short) DATA);
            dp->th_block = htons((u_short) block);
        }
        timeout = 0;
        (void)sigsetjmp(timeoutbuf, 1);

        if (trace)
            tpacket("sent", dp, size + 4);
        n = sendto(f, dp, size + 4, 0,
                   &peeraddr.sa, SOCKLEN(&peeraddr));
        if (n != size + 4) {
            perror("tftp: sendto");
            goto abort;
        }
        read_ahead(file, convert);
        for (;;) {
            alarm(rexmtval);
            do {
                fromlen = sizeof(from);
                n = recvfrom(f, ackbuf, sizeof(ackbuf), 0,
                             &from.sa, &fromlen);
            } while (n <= 0);
            alarm(0);
            if (n < 0) {
                perror("tftp: recvfrom");
                goto abort;
            }
            sa_set_port(&peeraddr, SOCKPORT(&from));  /* added */
            if (trace)
                tpacket("received", ap, n);
            /* should verify packet came from server */
            ap_opcode = ntohs((u_short) ap->th_opcode);
            ap_block = ntohs((u_short) ap->th_block);
            if (ap_opcode == ERROR) {
                printf("Error code %d: %s\n", ap_block, ap->th_msg);
                goto abort;
            }
            if (ap_opcode == ACK) {
                int j;

                if (ap_block == block) {
                    break;
                }
                /* On an error, try to synchronize
                 * both sides.
                 */
                j = synchnet(f);
                if (j && trace) {
                    printf("discarded %d packets\n", j);
                }
                /*
                 * RFC1129/RFC1350: We MUST NOT re-send the DATA
                 * packet in response to an invalid ACK.  Doing so
                 * would cause the Sorcerer's Apprentice bug.
                 */
            }
        }
        if (!is_request)
            amount += size;
        is_request = 0;
        block++;
    } while (size == SEGSIZE || block == 1);
  abort:
    fclose(file);
    stopclock();
    if (amount > 0)
        printstats("Sent", amount);
}

/*
 * Receive a file.
 */
void tftp_recvfile(int fd, const char *name, const char *mode)
{
    struct tftphdr *ap;
    struct tftphdr *dp;
    int n;
    volatile u_short block;
    volatile int size, firsttrip;
    volatile unsigned long amount;
    union sock_addr from;
    socklen_t fromlen;
    FILE *file;
    volatile int convert;       /* true if converting crlf -> lf */
    u_short dp_opcode, dp_block;

    startclock();
    dp = w_init();
    ap = (struct tftphdr *)ackbuf;
    convert = !strcmp(mode, "netascii");
    file = fdopen(fd, convert ? "wt" : "wb");
    block = 1;
    firsttrip = 1;
    amount = 0;

    bsd_signal(SIGALRM, timer);
    do {
        if (firsttrip) {
            size = makerequest(RRQ, name, ap, mode);
            firsttrip = 0;
        } else {
            ap->th_opcode = htons((u_short) ACK);
            ap->th_block = htons((u_short) block);
            size = 4;
            block++;
        }
        timeout = 0;
        (void)sigsetjmp(timeoutbuf, 1);
      send_ack:
        if (trace)
            tpacket("sent", ap, size);
        if (sendto(f, ackbuf, size, 0, &peeraddr.sa,
                   SOCKLEN(&peeraddr)) != size) {
            alarm(0);
            perror("tftp: sendto");
            goto abort;
        }
        write_behind(file, convert);
        for (;;) {
            alarm(rexmtval);
            do {
                fromlen = sizeof(from);
                n = recvfrom(f, dp, PKTSIZE, 0,
                             &from.sa, &fromlen);
            } while (n <= 0);
            alarm(0);
            if (n < 0) {
                perror("tftp: recvfrom");
                goto abort;
            }
            sa_set_port(&peeraddr, SOCKPORT(&from));  /* added */
            if (trace)
                tpacket("received", dp, n);
            /* should verify client address */
            dp_opcode = ntohs((u_short) dp->th_opcode);
            dp_block = ntohs((u_short) dp->th_block);
            if (dp_opcode == ERROR) {
                printf("Error code %d: %s\n", dp_block, dp->th_msg);
                goto abort;
            }
            if (dp_opcode == DATA) {
                int j;

                if (dp_block == block) {
                    break;      /* have next packet */
                }
                /* On an error, try to synchronize
                 * both sides.
                 */
                j = synchnet(f);
                if (j && trace) {
                    printf("discarded %d packets\n", j);
                }
                if (dp_block == (block - 1)) {
                    goto send_ack;      /* resend ack */
                }
            }
        }
        /*      size = write(fd, dp->th_data, n - 4); */
        size = writeit(file, &dp, n - 4, convert);
        if (size < 0) {
            nak(errno + 100, NULL);
            break;
        }
        amount += size;
    } while (size == SEGSIZE);
  abort:                       /* ok to ack, since user */
    ap->th_opcode = htons((u_short) ACK);       /* has seen err msg */
    ap->th_block = htons((u_short) block);
    (void)sendto(f, ackbuf, 4, 0, (struct sockaddr *)&peeraddr,
                 SOCKLEN(&peeraddr));
    write_behind(file, convert);        /* flush last buffer */
    fclose(file);
    stopclock();
    if (amount > 0)
        printstats("Received", amount);
}

static int
makerequest(int request, const char *name,
            struct tftphdr *tp, const char *mode)
{
    char *cp;

    tp->th_opcode = htons((u_short) request);
    cp = (char *)&(tp->th_stuff);
    strcpy(cp, name);
    cp += strlen(name);
    *cp++ = '\0';
    strcpy(cp, mode);
    cp += strlen(mode);
    *cp++ = '\0';
    return (cp - (char *)tp);
}

static const char *const errmsgs[] = {
    "Undefined error code",     /* 0 - EUNDEF */
    "File not found",           /* 1 - ENOTFOUND */
    "Access denied",            /* 2 - EACCESS */
    "Disk full or allocation exceeded", /* 3 - ENOSPACE */
    "Illegal TFTP operation",   /* 4 - EBADOP */
    "Unknown transfer ID",      /* 5 - EBADID */
    "File already exists",      /* 6 - EEXISTS */
    "No such user",             /* 7 - ENOUSER */
    "Failure to negotiate RFC2347 options"      /* 8 - EOPTNEG */
};

#define ERR_CNT (sizeof(errmsgs)/sizeof(const char *))

/*
 * Send a nak packet (error message).
 * Error code passed in is one of the
 * standard TFTP codes, or a UNIX errno
 * offset by 100.
 */
static void nak(int error, const char *msg)
{
    struct tftphdr *tp;
    int length;

    tp = (struct tftphdr *)ackbuf;
    tp->th_opcode = htons((u_short) ERROR);
    tp->th_code = htons((u_short) error);

    if (error >= 100) {
        /* This is a Unix errno+100 */
        if (!msg)
            msg = strerror(error - 100);
        error = EUNDEF;
    } else {
        if ((unsigned)error >= ERR_CNT)
            error = EUNDEF;

        if (!msg)
            msg = errmsgs[error];
    }

    tp->th_code = htons((u_short) error);

    length = strlen(msg) + 1;
    memcpy(tp->th_msg, msg, length);
    length += 4;                /* Add space for header */

    if (trace)
        tpacket("sent", tp, length);
    if (sendto(f, ackbuf, length, 0, &peeraddr.sa,
               SOCKLEN(&peeraddr)) != length)
        perror("nak");
}

static void tpacket(const char *s, struct tftphdr *tp, int n)
{
    static const char *opcodes[] =
        { "#0", "RRQ", "WRQ", "DATA", "ACK", "ERROR", "OACK" };
    char *cp, *file;
    u_short op = ntohs((u_short) tp->th_opcode);

    if (op < RRQ || op > ERROR)
        printf("%s opcode=%x ", s, op);
    else
        printf("%s %s ", s, opcodes[op]);
    switch (op) {

    case RRQ:
    case WRQ:
        n -= 2;
        file = cp = (char *)&(tp->th_stuff);
        cp = strchr(cp, '\0');
        printf("<file=%s, mode=%s>\n", file, cp + 1);
        break;

    case DATA:
        printf("<block=%d, %d bytes>\n", ntohs(tp->th_block), n - 4);
        break;

    case ACK:
        printf("<block=%d>\n", ntohs(tp->th_block));
        break;

    case ERROR:
        printf("<code=%d, msg=%s>\n", ntohs(tp->th_code), tp->th_msg);
        break;
    }
}

struct timeval tstart;
struct timeval tstop;

static void startclock(void)
{
    (void)gettimeofday(&tstart, NULL);
}

static void stopclock(void)
{

    (void)gettimeofday(&tstop, NULL);
}

static void printstats(const char *direction, unsigned long amount)
{
    double delta;

    delta = (tstop.tv_sec + (tstop.tv_usec / 100000.0)) -
        (tstart.tv_sec + (tstart.tv_usec / 100000.0));
    if (verbose) {
        printf("%s %lu bytes in %.1f seconds", direction, amount, delta);
        printf(" [%.0f bit/s]", (amount * 8.) / delta);
        putchar('\n');
    }
}

static void timer(int sig)
{
    int save_errno = errno;

    (void)sig;                  /* Shut up unused warning */

    timeout += rexmtval;
    if (timeout >= maxtimeout) {
        printf("Transfer timed out.\n");
        errno = save_errno;
        siglongjmp(toplevel, -1);
    }
    errno = save_errno;
    siglongjmp(timeoutbuf, 1);
}
