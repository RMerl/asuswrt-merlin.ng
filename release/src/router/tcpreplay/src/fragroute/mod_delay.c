/*
 * mod_delay.c
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

#define DELAY_FIRST 1
#define DELAY_LAST 2
#define DELAY_RANDOM 3

struct delay_data {
    rand_t *rnd;
    int which;
    struct timeval tv;
};

void *
delay_close(void *d)
{
    struct delay_data *data = (struct delay_data *)d;

    if (data != NULL) {
        rand_close(data->rnd);
        free(data);
    }
    return (NULL);
}

void *
delay_open(int argc, char *argv[])
{
    struct delay_data *data;
    uint64_t usec;

    if (argc != 3)
        return (NULL);

    if ((data = malloc(sizeof(*data))) == NULL)
        return (NULL);

    data->rnd = rand_open();

    if (strcasecmp(argv[1], "first") == 0)
        data->which = DELAY_FIRST;
    else if (strcasecmp(argv[1], "last") == 0)
        data->which = DELAY_LAST;
    else if (strcasecmp(argv[1], "random") == 0)
        data->which = DELAY_RANDOM;
    else
        return (delay_close(data));

    if ((usec = strtol(argv[2], NULL, 10)) == 0)
        return (delay_close(data));

    usec *= 1000;
    data->tv.tv_sec = (time_t)(usec / 1000000);
    data->tv.tv_usec = (suseconds_t)(usec % 1000000);

    return (data);
}

int
delay_apply(void *d, struct pktq *pktq)
{
    struct delay_data *data = (struct delay_data *)d;
    struct pkt *pkt;

    if (data->which == DELAY_FIRST)
        pkt = TAILQ_FIRST(pktq);
    else if (data->which == DELAY_LAST)
        pkt = TAILQ_LAST(pktq, pktq);
    else
        pkt = pktq_random(data->rnd, pktq);

    memcpy(&pkt->pkt_ts, &data->tv, sizeof(pkt->pkt_ts));

    return (0);
}

struct mod mod_delay = {
        "delay",                        /* name */
        "delay first|last|random <ms>", /* usage */
        delay_open,                     /* open */
        delay_apply,                    /* apply */
        delay_close                     /* close */
};
