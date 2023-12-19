/*
 * mod.c
 *
 * Copyright (c) 2001 Dug Song <dugsong@monkey.org>
 * Copyright (c) 2007-2010 Aaron Turner.
 *
 * $Id$
 */

#include "mod.h"
#include "defines.h"
#include "config.h"
#include "common.h"
#include "argv.h"
#include "lib/queue.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ARGS 128

struct rule {
    struct mod *mod;
    void *data;
    TAILQ_ENTRY(rule) next;
};

/*
 * new modules must be registered here.
 */
extern struct mod mod_delay;
extern struct mod mod_drop;
extern struct mod mod_dup;
extern struct mod mod_echo;
extern struct mod mod_ip_chaff;
extern struct mod mod_ip_frag;
extern struct mod mod_ip_opt;
extern struct mod mod_ip_ttl;
extern struct mod mod_ip_tos;
extern struct mod mod_ip6_qos;
extern struct mod mod_ip6_opt;
extern struct mod mod_order;
extern struct mod mod_print;
extern struct mod mod_tcp_chaff;
extern struct mod mod_tcp_opt;
extern struct mod mod_tcp_seg;

static struct mod *mods[] = {&mod_delay,
                             &mod_drop,
                             &mod_dup,
                             &mod_echo,
                             &mod_ip_chaff,
                             &mod_ip_frag,
                             &mod_ip_opt,
                             &mod_ip_ttl,
                             &mod_ip_tos,
                             &mod_ip6_qos,
                             &mod_ip6_opt,
                             &mod_order,
                             &mod_print,
                             &mod_tcp_chaff,
                             &mod_tcp_opt,
                             &mod_tcp_seg,
                             NULL};

static TAILQ_HEAD(head, rule) rules;

void
mod_usage(void)
{
    struct mod **m;

    for (m = mods; *m != NULL; m++) {
        fprintf(stderr, "       %s\n", (*m)->usage);
    }
}

int
mod_open(const char *script, char *errbuf)
{
    FILE *fp;
    struct mod **m;
    struct rule *rule = NULL;
    char *argv[MAX_ARGS], buf[BUFSIZ];
    int i, argc, ret = 0;

    TAILQ_INIT(&rules);

    /* open the config/script file */
    if ((fp = fopen(script, "r")) == NULL) {
        sprintf(errbuf, "couldn't open %s", script);
        return (-1);
    }
    dbg(1, "opened config file...");
    /* read the file, one line at a time... */
    for (i = 1; fgets(buf, sizeof(buf), fp) != NULL; i++) {
        /* skip comments & blank lines */
        if (*buf == '#' || *buf == '\r' || *buf == '\n')
            continue;

        /* parse the line into an array */
        if ((argc = argv_create(buf, MAX_ARGS, argv)) < 1) {
            sprintf(errbuf, "couldn't parse arguments (line %d)", i);
            ret = -1;
            break;
        }

        dbgx(1, "argc = %d, %s, %s, %s", argc, argv[0], argv[1], argv[2]);
        /* check first keyword against modules */
        for (m = mods; *m != NULL; m++) {
            if (strcasecmp((*m)->name, argv[0]) == 0) {
                dbgx(1, "comparing %s to %s", argv[0], (*m)->name);
                break;
            }
        }

        /* do we have a match? */
        if (*m == NULL) {
            sprintf(errbuf, "unknown directive '%s' (line %d)", argv[0], i);
            ret = -1;
            break;
        }

        /* allocate memory for our rule */
        if ((rule = calloc(1, sizeof(*rule))) == NULL) {
            sprintf(errbuf, "calloc");
            ret = -1;
            break;
        }
        rule->mod = *m;

        /* pass the remaining args to the rule */
        if (rule->mod->open != NULL && (rule->data = rule->mod->open(argc, argv)) == NULL) {
            sprintf(errbuf, "invalid argument to directive '%s' (line %d)", rule->mod->name, i);
            ret = -1;
            break;
        }
        /* append the rule to the rule list */
        TAILQ_INSERT_TAIL(&rules, rule, next);
    }

    /* close the file */
    fclose(fp);
    dbg(1, "close file...");

    if (ret == 0) {
        buf[0] = '\0';
        TAILQ_FOREACH(rule, &rules, next)
        {
            strlcat(buf, rule->mod->name, sizeof(buf));
            strlcat(buf, " -> ", sizeof(buf));
        }
        buf[strlen(buf) - 4] = '\0';
        sprintf(errbuf, "wtf: %s", buf);
    }

    if (rule)
        free(rule);

    return (ret);
}

void
mod_apply(struct pktq *pktq)
{
    struct rule *rule;

    TAILQ_FOREACH(rule, &rules, next)
    {
        rule->mod->apply(rule->data, pktq);
    }
}

void
mod_close(void)
{
    struct rule *rule;

    TAILQ_FOREACH_REVERSE(rule, &rules, next, head)
    {
        if (rule->mod->close != NULL)
            rule->data = rule->mod->close(rule->data);
        TAILQ_REMOVE(&rules, rule, next);
    }
}
