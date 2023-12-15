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

#include "tcpprep_api.h"
#include "config.h"
#include "common.h"
#include "tcpprep_opts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void print_comment(const char *);
extern void print_info(const char *);
extern void print_stats(const char *);

/**
 * \brief Initialize a new tcpprep context
 *
 * Allocates memory and stuff like that.  Always returns a buffer or completely
 * fails by calling exit() on malloc failure.
 */
tcpprep_t *
tcpprep_init()
{
    tcpprep_t *ctx;
    int i;

    ctx = safe_malloc(sizeof(tcpprep_t));
    ctx->options = safe_malloc(sizeof(tcpprep_opt_t));

    ctx->options->bpf.optimize = BPF_OPTIMIZE;

    for (i = DEFAULT_LOW_SERVER_PORT; i <= DEFAULT_HIGH_SERVER_PORT; i++) {
        ctx->options->services.tcp[i] = 1;
        ctx->options->services.udp[i] = 1;
    }

    return ctx;
}

/**
 * Closes & free's all memory related to a tcpprep context
 */
void
tcpprep_close(tcpprep_t *ctx)
{
    tcpr_cache_t *cache, *cache_nxt;
    tcpr_cidr_t *cidr, *cidr_nxt;
    tcpprep_opt_t *options;

    assert(ctx);
    options = ctx->options;

    if (options->pcap != NULL)
        pcap_close(options->pcap);

#ifdef ENABLE_VERBOSE
    safe_free(options->tcpdump_args);
#endif
    safe_free(options->comment);
    safe_free(options->maclist);

    cache = options->cachedata;
    while (cache != NULL) {
        cache_nxt = cache->next;
        safe_free(cache);
        cache = cache_nxt;
    }

    cidr = options->cidrdata;
    while (cidr != NULL) {
        cidr_nxt = cidr->next;
        safe_free(cidr);
        cidr = cidr_nxt;
    }

    safe_free(options);

    safe_free(ctx->outfile);
    safe_free(ctx->pcapfile);
    safe_free(ctx);
}

/**
 * \brief When using AutoOpts, call to do post argument processing
 * Used to process the autoopts arguments
 */
int
tcpprep_post_args(tcpprep_t *ctx, int argc, char *argv[])
{
    char myargs[MYARGS_LEN];
    size_t bufsize;
    char *endptr;
    char *tempstr;

    memset(myargs, 0, MYARGS_LEN);

    /* print_comment and print_info don't return */
    if (HAVE_OPT(PRINT_COMMENT))
        print_comment(OPT_ARG(PRINT_COMMENT));

    if (HAVE_OPT(PRINT_INFO))
        print_info(OPT_ARG(PRINT_INFO));

    if (HAVE_OPT(PRINT_STATS))
        print_stats(OPT_ARG(PRINT_STATS));

    if (!HAVE_OPT(CACHEFILE) && !HAVE_OPT(PCAP))
        err(-1, "Must specify an output cachefile (-o) and input pcap (-i)");

    if (!ctx->options->mode)
        err(-1, "Must specify a processing mode: -a, -c, -r, -p");

#ifdef DEBUG
    if (HAVE_OPT(DBUG))
        debug = OPT_VALUE_DBUG;
#endif

#ifdef ENABLE_VERBOSE
    if (HAVE_OPT(VERBOSE)) {
        ctx->options->verbose = 1;
    }

    if (HAVE_OPT(DECODE))
        ctx->tcpdump.args = safe_strdup(OPT_ARG(DECODE));
#endif

    /*
     * if we are to include the cli args, then prep it for the
     * cache file header
     */
    if (!ctx->options->nocomment) {
        int i;

        /* copy all of our args to myargs */
        for (i = 1; i < argc; i++) {
            /* skip the -C <comment> */
            if (strcmp(argv[i], "-C") == 0) {
                i += 2;
                continue;
            }

            strlcat(myargs, argv[i], MYARGS_LEN);
            strlcat(myargs, " ", MYARGS_LEN);
        }

        /* remove trailing space */
        myargs[strlen(myargs) - 1] = 0;

        dbgx(1, "Comment args length: %zu", strlen(myargs));
    }

    /* setup or options.comment buffer so that we get args\ncomment */
    if (ctx->options->comment != NULL) {
        strlcat(myargs, "\n", MYARGS_LEN);
        bufsize = strlen(ctx->options->comment) + strlen(myargs) + 1;
        ctx->options->comment = (char *)safe_realloc(ctx->options->comment, bufsize);

        tempstr = strdup(ctx->options->comment);
        strlcpy(ctx->options->comment, myargs, bufsize);
        strlcat(ctx->options->comment, tempstr, bufsize);
        safe_free(tempstr);
    } else {
        bufsize = strlen(myargs) + 1;
        ctx->options->comment = (char *)safe_malloc(bufsize);
        strlcpy(ctx->options->comment, myargs, bufsize);
    }

    dbgx(1, "Final comment length: %zu", strlen(ctx->options->comment));

    /* copy over our min/max mask */
    ctx->options->min_mask = OPT_VALUE_MINMASK;

    ctx->options->max_mask = OPT_VALUE_MAXMASK;

    if (ctx->options->min_mask <= ctx->options->max_mask)
        errx(-1,
             "Min network mask len (%d) must be less then max network mask len (%d)",
             ctx->options->min_mask,
             ctx->options->max_mask);

    ctx->options->ratio = strtod(OPT_ARG(RATIO), &endptr);
    if (endptr == OPT_ARG(RATIO))
        err(-1, "Ratio supplied is not a number.");

    if (ctx->options->ratio < 0)
        err(-1, "Ratio must be a non-negative number.");

    return 0;
}
