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

#include "signal_handler.h"
#include "defines.h"
#include "config.h"
#include "common.h"
#include "tcpreplay_api.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

struct timeval suspend_time;
static struct timeval suspend_start;
static struct timeval suspend_end;

static void suspend_handler(int signo);
static void continue_handler(int signo);
static void abort_handler(int signo);

extern tcpreplay_t *ctx;

/***************************************************************
 * This code is for pausing/restarting tcpreplay using SIGUSR1 *
 * for abort code on SIGINT                                    *
 ***************************************************************/

/**
 * \brief init_signal_handlers
 *
 * Initialize signal handlers to be used in tcpreplay.
 */
void
init_signal_handlers()
{
    signal(SIGUSR1, suspend_handler);
    signal(SIGCONT, continue_handler);
    signal(SIGINT, abort_handler);

    reset_suspend_time();
}

/**
 * \brief reset_suspend_time
 *
 * Reset time values for suspend signal.
 */
void
reset_suspend_time()
{
    timerclear(&suspend_time);
    timerclear(&suspend_start);
    timerclear(&suspend_end);
}

/**
 * \brief suspend signal handler
 *
 * Signal handler for signal SIGUSR1. SIGSTOP cannot be
 * caught, so SIGUSR1 is caught and it throws SIGSTOP.
 */
static void
suspend_handler(int signo)
{
    if (signo != SIGUSR1) {
        warnx("suspend_handler() got the wrong signal: %d", signo);
        return;
    }

    if (gettimeofday(&suspend_start, NULL) < 0)
        errx(-1, "gettimeofday(): %s", strerror(errno));

    kill(getpid(), SIGSTOP);
}

/**
 * \brief continue_handler
 *
 * Signal handler for continue signal.
 */
static void
continue_handler(int signo)
{
    struct timeval suspend_delta;

    if (signo != SIGCONT) {
        warnx("continue_handler() got the wrong signal: %d", signo);
        return;
    }

    if (gettimeofday(&suspend_end, NULL) < 0)
        errx(-1, "gettimeofday(): %s", strerror(errno));

    timersub(&suspend_end, &suspend_start, &suspend_delta);
    timeradd(&suspend_time, &suspend_delta, &suspend_time);
}

/**
 * \brief abort handler
 *
 * Signal handler for Ctrl-C
 */
static void
abort_handler(int signo)
{
    if (signo == SIGINT && ctx) {
        notice(" User interrupt...");
        ctx->abort = true;
        tcpreplay_abort(ctx);
    }
}
