/***
  This file is part of avahi.

  avahi is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  avahi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
  Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with avahi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <errno.h>
#include <stdio.h>

#include <avahi-common/llist.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-core/log.h>
#include <avahi-core/publish.h>

#include "main.h"
#include "static-aliases.h"

typedef struct StaticAlias StaticAlias;

struct StaticAlias {
    AvahiSEntryGroup *group;
    int iteration;

    char *alias;

    AVAHI_LLIST_FIELDS(StaticAlias, aliases);
};

static AVAHI_LLIST_HEAD(StaticAlias, aliases) = NULL;
static int current_iteration = 0;

static void add_static_alias_to_server(StaticAlias *a);
static void remove_static_alias_from_server(StaticAlias *a);

static void entry_group_callback(AvahiServer *s, AVAHI_GCC_UNUSED AvahiSEntryGroup *eg, AvahiEntryGroupState state, void* userdata) {
    StaticAlias *a;

    assert(s);
    assert(eg);

    a = userdata;

    switch (state) {

        case AVAHI_ENTRY_GROUP_COLLISION:
            avahi_log_error("Alias name conflict for \"%s\", not established.", a->alias);
            break;

        case AVAHI_ENTRY_GROUP_ESTABLISHED:
            avahi_log_notice ("Alias name \"%s\" successfully established.", a->alias);
            break;

        case AVAHI_ENTRY_GROUP_FAILURE:
            avahi_log_notice ("Failed to establish alias name \"%s\": %s.", a->alias, avahi_strerror (avahi_server_errno (s)));
            break;

        case AVAHI_ENTRY_GROUP_UNCOMMITED:
        case AVAHI_ENTRY_GROUP_REGISTERING:
            ;
    }
}

static StaticAlias *static_alias_new(void) {
    StaticAlias *a;

    a = avahi_new(StaticAlias, 1);

    a->group = NULL;
    a->alias = NULL;
    a->iteration = current_iteration;

    AVAHI_LLIST_PREPEND(StaticAlias, aliases, aliases, a);

    return a;
}

static void static_alias_free(StaticAlias *a) {
    assert(a);

    AVAHI_LLIST_REMOVE(StaticAlias, aliases, aliases, a);

    if (a->group)
        avahi_s_entry_group_free (a->group);

    avahi_free(a->alias);

    avahi_free(a);
}

static StaticAlias *static_alias_find(const char *alias) {
    StaticAlias *a;

    assert(alias);

    for (a = aliases; a; a = a->aliases_next)
        if (!strcmp(a->alias, alias))
            return a;

    return NULL;
}

static void add_static_alias_to_server(StaticAlias *a)
{

    if (!a->group)
        if (!(a->group = avahi_s_entry_group_new (avahi_server, entry_group_callback, a))) {
            avahi_log_error("avahi_s_entry_group_new() failed: %s", avahi_strerror(avahi_server_errno(avahi_server)));
            return;
        }

    if (avahi_s_entry_group_is_empty(a->group)) {
        int err;

        if ((err = avahi_server_add_cname(avahi_server, a->group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, a->alias, NULL, NULL)) < 0) {
            avahi_log_error ("Alias name %s: avahi_server_add_cname failure: %s", a->alias, avahi_strerror(err));
            return;
        }

        avahi_s_entry_group_commit (a->group);
    }
}

static void remove_static_alias_from_server(StaticAlias *a)
{
    if (a->group)
        avahi_s_entry_group_reset (a->group);
}

void static_aliases_add_to_server(void) {
    StaticAlias *a;

    for (a = aliases; a; a = a->aliases_next)
        add_static_alias_to_server(a);
}

void static_aliases_remove_from_server(void) {
    StaticAlias *a;

    for (a = aliases; a; a = a->aliases_next)
        remove_static_alias_from_server(a);
}

void static_aliases_register(const AvahiServerConfig *config) {
    StaticAlias *a, *next;
    char **alias;

    current_iteration++;

    if (!config)
        config = avahi_server_get_config(avahi_server);

    for (alias = config->aliases; alias && *alias; alias++) {
        if (!(a = static_alias_find(*alias))) {
            a = static_alias_new();
            a->alias = *alias;

            avahi_log_info("Loading new alias name %s.", a->alias);
        }

        a->iteration = current_iteration;
    }

    for (a = aliases; a; a = next) {
        next = a->aliases_next;

        if (a->iteration != current_iteration) {
            avahi_log_info("Alias name %s vanished, removing.", a->alias);
            static_alias_free(a);
        }
    }
}

void static_aliases_free_all (void)
{
    while(aliases)
        static_alias_free(aliases);
}
