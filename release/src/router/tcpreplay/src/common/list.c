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
 * A generic method to parse a list of integers which are
 * delimited by commas and dashes to indicate individual
 * numbers and ranges
 * Provides both a way to process the list and determine
 * if an integer exists in the list.
 */

#include "defines.h"
#include "config.h"
#include "common.h"
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/**
 * Creates a new tcpr_list entry.  Malloc's memory.
 */
tcpr_list_t *
new_list()
{
    tcpr_list_t *newlist;

    newlist = (tcpr_list_t *)safe_malloc(sizeof(tcpr_list_t));
    return (newlist);
}

/**
 * Processes a string (ourstr) containing the list in human readable
 * format and places the data in **list and finally returns 1 for
 * success, 0 for fail.
 */
int
parse_list(tcpr_list_t **listdata, char *ourstr)
{
    tcpr_list_t *listcur, *list_ptr;
    char *this = NULL;
    char *first, *second;
    int rcode;
    regex_t preg;
    char regex[] = "^[0-9]+(-[0-9]+)?$";
    char *token = NULL;
    u_int i;

    /* compile the regex first */
    if ((rcode = regcomp(&preg, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
        char ebuf[EBUF_SIZE];
        regerror(rcode, &preg, ebuf, sizeof(ebuf));
        errx(-1, "Unable to compile regex (%s): %s", regex, ebuf);
    }

    /* first iteration */
    this = strtok_r(ourstr, ",", &token);
    first = this;
    second = NULL;

    /* regex test */
    if (this == NULL || regexec(&preg, this, 0, NULL, 0) != 0) {
        warnx("Unable to parse: %s", this);
        regfree(&preg);
        return 0;
    }

    *listdata = new_list();
    list_ptr = *listdata;
    listcur = list_ptr;

    for (i = 0; i < strlen(this); i++) {
        if (this[i] == '-') {
            this[i] = '\0';
            second = &this[i + 1];
        }
    }

    list_ptr->min = strtoull(first, NULL, 0);
    if (second != NULL) {
        list_ptr->max = strtoull(second, NULL, 0);
    } else {
        list_ptr->max = list_ptr->min;
    }

    while (1) {
        this = strtok_r(NULL, ",", &token);
        if (this == NULL)
            break;

        first = this;
        second = NULL;

        /* regex test */
        if (regexec(&preg, this, 0, NULL, 0) != 0) {
            warnx("Unable to parse: %s", this);
            regfree(&preg);
            return 0;
        }

        listcur->next = new_list();
        listcur = listcur->next;

        for (i = 0; i < strlen(this); i++) {
            if (this[i] == '-') {
                this[i] = '\0';
                second = &this[i + 1];
            }
        }

        listcur->min = strtoull(first, NULL, 0);
        if (second != NULL) {
            listcur->max = strtoull(second, NULL, 0);
        } else {
            listcur->max = listcur->min;
        }
    }

    regfree(&preg);

    return 1;
}

/**
 * Checks to see if the given integer exists in the LIST.
 * Return 1 if in the list, otherwise 0
 */
tcpr_dir_t
check_list(tcpr_list_t *list, COUNTER value)
{
    tcpr_list_t *current;
    current = list;

    do {
        if ((current->min != 0) && (current->max != 0)) {
            if ((value >= current->min) && (value <= current->max))
                return 1;
        } else if (current->min == 0) {
            if (value <= current->max)
                return 1;
        } else if (current->max == 0) {
            if (value >= current->min)
                return 1;
        }

        if (current->next != NULL)
            current = current->next;
        else
            current = NULL;
    } while (current != NULL);

    return 0;
}

/**
 * Free's all the memory associated with the given LIST
 */
void
free_list(tcpr_list_t *list)
{
    /* recursively go down the list */
    if (list->next != NULL)
        free_list(list->next);

    safe_free(list);
}
