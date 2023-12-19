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

#include "config.h"
#include "defines.h"
#include "common.h"

#include <regex.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

/**
 * parses /etc/services so we know which ports are service ports
 */
void
parse_services(const char *file, tcpr_services_t *services)
{
    FILE *service = NULL;
    char service_line[MAXLINE], port[10], proto[10];
    regex_t preg;
    size_t nmatch = 3;
    regmatch_t pmatch[3];
    static const char regex[] = "([0-9]+)/(tcp|udp)"; /* matches the port as pmatch[1], service pmatch[2] */

    assert(file);
    assert(services);

    dbgx(1, "Parsing %s", file);
    memset(service_line, '\0', MAXLINE);

    /* mark all ports not a service */
    memset(&services->tcp[0], '\0', sizeof(services->tcp));
    memset(&services->udp[0], '\0', sizeof(services->udp));

    if ((service = fopen(file, "r")) == NULL) {
        errx(-1, "Unable to open service file: %s\n%s", file, strerror(errno));
    }

    /* compile our regexes */
    if ((regcomp(&preg, regex, REG_ICASE|REG_EXTENDED)) != 0) {
        errx(-1, "Unable to compile regex: %s", regex);
    }

    /* parse the entire file */
    while ((fgets(service_line, MAXLINE, service)) != NULL) {
        /* zero out our vars */
        memset(port, '\0', 10);
        memset(proto, '\0', 10);

        dbgx(4, "Processing: %s", service_line);

        /* look for format of 1234/tcp */
        if ((regexec(&preg, service_line, nmatch, pmatch, 0)) == 0) { /* matches */
            uint16_t portc;
            /* strip out the port & proto from the line */
            strncpy(port, &service_line[pmatch[1].rm_so], (pmatch[1].rm_eo - pmatch[1].rm_so));
            strncpy(proto, &service_line[pmatch[2].rm_so], (pmatch[2].rm_eo - pmatch[2].rm_so));

            /* convert port[] into an integer */
            portc = (uint16_t)strtol(port, NULL, 10);

            /* update appropriate service array with the server port */
            if (strcmp(proto, "tcp") == 0) {
                dbgx(3, "Setting TCP/%d as a server port", portc);
                services->tcp[portc] = 1; /* mark it as a service port */
            } else if (strcmp(proto, "udp") == 0) {
                dbgx(3, "Setting UDP/%d as a server port", portc);
                services->udp[portc] = 1;
            } else {
                warnx("Skipping unknown protocol service %s/%d", proto, portc);
            }
        }
    }

    regfree(&preg);
    fclose(service);
}
