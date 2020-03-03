/*
 * iperf, Copyright (c) 2014, 2016, The Regents of the University of
 * California, through Lawrence Berkeley National Laboratory (subject
 * to receipt of any required approvals from the U.S. Dept. of
 * Energy).  All rights reserved.
 *
 * If you have questions about your rights to use or distribute this
 * software, please contact Berkeley Lab's Technology Transfer
 * Department at TTD@lbl.gov.
 *
 * NOTICE.  This software is owned by the U.S. Department of Energy.
 * As such, the U.S. Government has been granted for itself and others
 * acting on its behalf a paid-up, nonexclusive, irrevocable,
 * worldwide license in the Software to reproduce, prepare derivative
 * works, and perform publicly and display publicly.  Beginning five
 * (5) years after the date permission to assert copyright is obtained
 * from the U.S. Department of Energy, and subject to any subsequent
 * five (5) year renewals, the U.S. Government is granted for itself
 * and others acting on its behalf a paid-up, nonexclusive,
 * irrevocable, worldwide license in the Software to reproduce,
 * prepare derivative works, distribute copies to the public, perform
 * publicly and display publicly, and to permit others to do so.
 *
 * This code is distributed under a BSD style license, see the LICENSE
 * file for complete information.
 */
/* iperf_util.c
 *
 * Iperf utility functions
 *
 */
#include "iperf_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <time.h>
#include <errno.h>

#include "cjson.h"

/* make_cookie
 *
 * Generate and return a cookie string
 *
 * Iperf uses this function to create test "cookies" which
 * server as unique test identifiers. These cookies are also
 * used for the authentication of stream connections.
 */

void
make_cookie(char *cookie)
{
    static int randomized = 0;
    char hostname[500];
    struct timeval tv;
    char temp[1000];

    if ( ! randomized )
        srandom((int) time(0) ^ getpid());

    /* Generate a string based on hostname, time, randomness, and filler. */
    (void) gethostname(hostname, sizeof(hostname));
    (void) gettimeofday(&tv, 0);
    (void) snprintf(temp, sizeof(temp), "%s.%ld.%06ld.%08lx%08lx.%s", hostname, (unsigned long int) tv.tv_sec, (unsigned long int) tv.tv_usec, (unsigned long int) random(), (unsigned long int) random(), "1234567890123456789012345678901234567890");

    /* Now truncate it to 36 bytes and terminate. */
    memcpy(cookie, temp, 36);
    cookie[36] = '\0';
}


/* is_closed
 *
 * Test if the file descriptor fd is closed.
 * 
 * Iperf uses this function to test whether a TCP stream socket
 * is closed, because accepting and denying an invalid connection
 * in iperf_tcp_accept is not considered an error.
 */

int
is_closed(int fd)
{
    struct timeval tv;
    fd_set readset;

    FD_ZERO(&readset);
    FD_SET(fd, &readset);
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    if (select(fd+1, &readset, NULL, NULL, &tv) < 0) {
        if (errno == EBADF)
            return 1;
    }
    return 0;
}


double
timeval_to_double(struct timeval * tv)
{
    double d;

    d = tv->tv_sec + tv->tv_usec / 1000000;

    return d;
}

int
timeval_equals(struct timeval * tv0, struct timeval * tv1)
{
    if ( tv0->tv_sec == tv1->tv_sec && tv0->tv_usec == tv1->tv_usec )
	return 1;
    else
	return 0;
}

double
timeval_diff(struct timeval * tv0, struct timeval * tv1)
{
    double time1, time2;
    
    time1 = tv0->tv_sec + (tv0->tv_usec / 1000000.0);
    time2 = tv1->tv_sec + (tv1->tv_usec / 1000000.0);

    time1 = time1 - time2;
    if (time1 < 0)
        time1 = -time1;
    return time1;
}


int
delay(int64_t ns)
{
    struct timespec req, rem;

    req.tv_sec = 0;

    while (ns >= 1000000000L) {
        ns -= 1000000000L;
        req.tv_sec += 1;
    }

    req.tv_nsec = ns;

    while (nanosleep(&req, &rem) == -1)
        if (EINTR == errno)
            memcpy(&req, &rem, sizeof(rem));
        else
            return -1;
    return 0;
}

# ifdef DELAY_SELECT_METHOD
int
delay(int us)
{
    struct timeval tv;

    tv.tv_sec = 0;
    tv.tv_usec = us;
    (void) select(1, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &tv);
    return 1;
}
#endif


void
cpu_util(double pcpu[3])
{
    static struct timeval last;
    static clock_t clast;
    static struct rusage rlast;
    struct timeval temp;
    clock_t ctemp;
    struct rusage rtemp;
    double timediff;
    double userdiff;
    double systemdiff;

    if (pcpu == NULL) {
        gettimeofday(&last, NULL);
        clast = clock();
	getrusage(RUSAGE_SELF, &rlast);
        return;
    }

    gettimeofday(&temp, NULL);
    ctemp = clock();
    getrusage(RUSAGE_SELF, &rtemp);

    timediff = ((temp.tv_sec * 1000000.0 + temp.tv_usec) -
                (last.tv_sec * 1000000.0 + last.tv_usec));
    userdiff = ((rtemp.ru_utime.tv_sec * 1000000.0 + rtemp.ru_utime.tv_usec) -
                (rlast.ru_utime.tv_sec * 1000000.0 + rlast.ru_utime.tv_usec));
    systemdiff = ((rtemp.ru_stime.tv_sec * 1000000.0 + rtemp.ru_stime.tv_usec) -
                  (rlast.ru_stime.tv_sec * 1000000.0 + rlast.ru_stime.tv_usec));

    pcpu[0] = (((ctemp - clast) * 1000000.0 / CLOCKS_PER_SEC) / timediff) * 100;
    pcpu[1] = (userdiff / timediff) * 100;
    pcpu[2] = (systemdiff / timediff) * 100;
}

const char *
get_system_info(void)
{
    static char buf[1024];
    struct utsname  uts;

    memset(buf, 0, 1024);
    uname(&uts);

    snprintf(buf, sizeof(buf), "%s %s %s %s %s", uts.sysname, uts.nodename, 
	     uts.release, uts.version, uts.machine);

    return buf;
}


const char *
get_optional_features(void)
{
    static char features[1024];
    unsigned int numfeatures = 0;

    snprintf(features, sizeof(features), "Optional features available: ");

#if defined(HAVE_CPU_AFFINITY)
    if (numfeatures > 0) {
	strncat(features, ", ", 
		sizeof(features) - strlen(features) - 1);
    }
    strncat(features, "CPU affinity setting", 
	sizeof(features) - strlen(features) - 1);
    numfeatures++;
#endif /* HAVE_CPU_AFFINITY */
    
#if defined(HAVE_FLOWLABEL)
    if (numfeatures > 0) {
	strncat(features, ", ", 
		sizeof(features) - strlen(features) - 1);
    }
    strncat(features, "IPv6 flow label", 
	sizeof(features) - strlen(features) - 1);
    numfeatures++;
#endif /* HAVE_FLOWLABEL */
    
#if defined(HAVE_SCTP)
    if (numfeatures > 0) {
	strncat(features, ", ", 
		sizeof(features) - strlen(features) - 1);
    }
    strncat(features, "SCTP", 
	sizeof(features) - strlen(features) - 1);
    numfeatures++;
#endif /* HAVE_SCTP */
    
#if defined(HAVE_TCP_CONGESTION)
    if (numfeatures > 0) {
	strncat(features, ", ", 
		sizeof(features) - strlen(features) - 1);
    }
    strncat(features, "TCP congestion algorithm setting", 
	sizeof(features) - strlen(features) - 1);
    numfeatures++;
#endif /* HAVE_TCP_CONGESTION */
    
#if defined(HAVE_SENDFILE)
    if (numfeatures > 0) {
	strncat(features, ", ",
		sizeof(features) - strlen(features) - 1);
    }
    strncat(features, "sendfile / zerocopy",
	sizeof(features) - strlen(features) - 1);
    numfeatures++;
#endif /* HAVE_SENDFILE */

#if defined(HAVE_SO_MAX_PACING_RATE)
    if (numfeatures > 0) {
	strncat(features, ", ",
		sizeof(features) - strlen(features) - 1);
    }
    strncat(features, "socket pacing",
	sizeof(features) - strlen(features) - 1);
    numfeatures++;
#endif /* HAVE_SO_MAX_PACING_RATE */

    if (numfeatures == 0) {
	strncat(features, "None", 
		sizeof(features) - strlen(features) - 1);
    }

    return features;
}

/* Helper routine for building cJSON objects in a printf-like manner.
**
** Sample call:
**   j = iperf_json_printf("foo: %b  bar: %d  bletch: %f  eep: %s", b, i, f, s);
**
** The four formatting characters and the types they expect are:
**   %b  boolean           int
**   %d  integer           int64_t
**   %f  floating point    double
**   %s  string            char *
** If the values you're passing in are not these exact types, you must
** cast them, there is no automatic type coercion/widening here.
**
** The colons mark the end of field names, and blanks are ignored.
**
** This routine is not particularly robust, but it's not part of the API,
** it's just for internal iperf3 use.
*/
cJSON*
iperf_json_printf(const char *format, ...)
{
    cJSON* o;
    va_list argp;
    const char *cp;
    char name[100];
    char* np;
    cJSON* j;

    o = cJSON_CreateObject();
    if (o == NULL)
        return NULL;
    va_start(argp, format);
    np = name;
    for (cp = format; *cp != '\0'; ++cp) {
	switch (*cp) {
	    case ' ':
	    break;
	    case ':':
	    *np = '\0';
	    break;
	    case '%':
	    ++cp;
	    switch (*cp) {
		case 'b':
		j = cJSON_CreateBool(va_arg(argp, int));
		break;
		case 'd':
		j = cJSON_CreateNumber(va_arg(argp, int64_t));
		break;
		case 'f':
		j = cJSON_CreateNumber(va_arg(argp, double));
		break;
		case 's':
		j = cJSON_CreateString(va_arg(argp, char *));
		break;
		default:
		va_end(argp);
		return NULL;
	    }
	    if (j == NULL) {
	    	va_end(argp);
	    	return NULL;
	    }
	    cJSON_AddItemToObject(o, name, j);
	    np = name;
	    break;
	    default:
	    *np++ = *cp;
	    break;
	}
    }
    va_end(argp);
    return o;
}

/* Debugging routine to dump out an fd_set. */
void
iperf_dump_fdset(FILE *fp, char *str, int nfds, fd_set *fds)
{
    int fd;
    int comma;

    fprintf(fp, "%s: [", str);
    comma = 0;
    for (fd = 0; fd < nfds; ++fd) {
        if (FD_ISSET(fd, fds)) {
	    if (comma)
		fprintf(fp, ", ");
	    fprintf(fp, "%d", fd);
	    comma = 1;
	}
    }
    fprintf(fp, "]\n");
}
