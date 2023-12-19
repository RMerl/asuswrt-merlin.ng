/*
 * mod_dup.c
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

#define DUP_FIRST 1
#define DUP_LAST 2
#define DUP_RANDOM 3

struct dup_data {
    rand_t *rnd;
    int which;
    int percent;
};

void *
dup_close(void *d)
{
    struct dup_data *data = (struct dup_data *)d;

    if (data != NULL) {
        rand_close(data->rnd);
        free(data);
    }
    return (NULL);
}

void *
dup_open(int argc, char *argv[])
{
    struct dup_data *data;

    if (argc != 3)
        return (NULL);

    if ((data = malloc(sizeof(*data))) == NULL)
        return (NULL);

    data->rnd = rand_open();

    if (strcasecmp(argv[1], "first") == 0)
        data->which = DUP_FIRST;
    else if (strcasecmp(argv[1], "last") == 0)
        data->which = DUP_LAST;
    else if (strcasecmp(argv[1], "random") == 0)
        data->which = DUP_RANDOM;
    else
        return (dup_close(data));

    if ((data->percent = (int)strtol(argv[2], NULL, 10)) <= 0 || data->percent > 100)
        return (dup_close(data));

    return (data);
}

int
dup_apply(void *d, struct pktq *pktq)
{
    struct dup_data *data = (struct dup_data *)d;
    struct pkt *pkt, *new;

    if (data->percent < 100 && (rand_uint16(data->rnd) % 100) > data->percent)
        return (0);

    if (data->which == DUP_FIRST)
        pkt = TAILQ_FIRST(pktq);
    else if (data->which == DUP_LAST)
        pkt = TAILQ_LAST(pktq, pktq);
    else
        pkt = pktq_random(data->rnd, pktq);

    if (!pkt)
        return -1;

    new = pkt_dup(pkt);
    if (!new)
        return -1;

    TAILQ_INSERT_AFTER(pktq, pkt, new, pkt_next);

    return (0);
}

struct mod mod_dup = {
        "dup",                            /* name */
        "dup first|last|random <prob-%>", /* usage */
        dup_open,                         /* open */
        dup_apply,                        /* apply */
        dup_close                         /* close */
};
