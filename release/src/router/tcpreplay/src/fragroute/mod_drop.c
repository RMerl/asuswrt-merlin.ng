/*
 * mod_drop.c
 *
 * Copyright (c) 2001 Dug Song <dugsong@monkey.org>
 *
 * $Id$
 */

#include "config.h"
#include "mod.h"
#include "pkt.h"
#include <stdlib.h>
#include <string.h>

#define DROP_FIRST 1
#define DROP_LAST 2
#define DROP_RANDOM 3

struct drop_data {
    rand_t *rnd;
    int which;
    int percent;
};

void *
drop_close(void *d)
{
    struct drop_data *data = (struct drop_data *)d;

    if (data != NULL) {
        rand_close(data->rnd);
        free(data);
    }
    return (NULL);
}

void *
drop_open(int argc, char *argv[])
{
    struct drop_data *data;

    if (argc != 3)
        return (NULL);

    if ((data = calloc(1, sizeof(*data))) == NULL)
        return (NULL);

    data->rnd = rand_open();

    if (strcasecmp(argv[1], "first") == 0)
        data->which = DROP_FIRST;
    else if (strcasecmp(argv[1], "last") == 0)
        data->which = DROP_LAST;
    else if (strcasecmp(argv[1], "random") == 0)
        data->which = DROP_RANDOM;
    else
        return (drop_close(data));

    if ((data->percent = (int)strtol(argv[2], NULL, 10)) <= 0 || data->percent > 100)
        return (drop_close(data));

    return (data);
}

int
drop_apply(void *d, struct pktq *pktq)
{
    struct drop_data *data = (struct drop_data *)d;
    struct pkt *pkt;

    if (data->percent < 100 && (rand_uint16(data->rnd) % 100) > data->percent)
        return (0);

    if (data->which == DROP_FIRST)
        pkt = TAILQ_FIRST(pktq);
    else if (data->which == DROP_LAST)
        pkt = TAILQ_LAST(pktq, pktq);
    else
        pkt = pktq_random(data->rnd, pktq);

    if (pkt) {
        TAILQ_REMOVE(pktq, pkt, pkt_next);
        pkt_free(pkt);
    }

    return (0);
}

struct mod mod_drop = {
        "drop",                            /* name */
        "drop first|last|random <prob-%>", /* usage */
        drop_open,                         /* open */
        drop_apply,                        /* apply */
        drop_close                         /* close */
};
