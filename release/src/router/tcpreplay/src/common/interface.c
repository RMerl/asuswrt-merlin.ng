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

#include "interface.h"
#include "config.h"
#include "common.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

/**
 * Method takes a user specified device name and returns
 * the canonical name for that device.  This allows me to
 * create named interface aliases on platforms like Windows
 * which use horrifically long interface names
 *
 * Returns NULL on error
 */
char *
get_interface(interface_list_t *list, const char *alias)
{
    interface_list_t *ptr;

    assert(alias);

    ptr = list;

    while (ptr) {
        /* check both the alias & name fields */
        if (strcmp(alias, ptr->alias) == 0 || strcmp(alias, ptr->name) == 0)
            return (ptr->name);

        ptr = ptr->next;
    }

    return (NULL);
}

/**
 * Get all available interfaces as an interface_list *
 */
interface_list_t *
get_interface_list(void)
{
    interface_list_t *list_head, *list_ptr;
    char ebuf[PCAP_ERRBUF_SIZE], testnic[320];
    pcap_if_t *pcap_if, *pcap_if_ptr;
    int i = 0;
    DIR *dir;
    struct dirent *dirdata;
#ifdef HAVE_NETMAP
    nmreq_t nmr;
#endif
#ifdef HAVE_NETMAP
    int netmap_version;
#endif

#ifndef HAVE_WIN32
    /* Unix just has a warning about being root */
    if (geteuid() != 0)
        warn("May need to run as root to get access to all network interfaces.");
#endif

#ifdef HAVE_NETMAP
    netmap_version = get_netmap_version();
#endif

    if (pcap_findalldevs(&pcap_if, ebuf) < 0)
        errx(-1, "Error: %s", ebuf);

    pcap_if_ptr = pcap_if;
    list_head = (interface_list_t *)safe_malloc(sizeof(interface_list_t));
    list_ptr = list_head;

    while (pcap_if_ptr != NULL) {
        if (i > 0) {
            list_ptr->next = (interface_list_t *)safe_malloc(sizeof(interface_list_t));
            list_ptr = list_ptr->next;
        }
        strlcpy(list_ptr->name, pcap_if_ptr->name, sizeof(list_ptr->name));
        dbgx(3, "Adding %s to interface list", list_ptr->name);

        /* description is usually null under Unix */
        if (pcap_if_ptr->description != NULL)
            strlcpy(list_ptr->description, pcap_if_ptr->description, sizeof(list_ptr->description));

        sprintf(list_ptr->alias, "%%%d", i++);
        list_ptr->flags = pcap_if_ptr->flags;
#ifdef HAVE_LIBPCAP_NETMAP
        /*
         * add the syntaxes supported by netmap-libpcap
         *
         * available at http://code.google.com/p/netmap-libpcap/
         */
        if (!(pcap_if_ptr->flags & PCAP_IF_LOOPBACK) && strcmp("any", pcap_if_ptr->name)) {
#endif
#ifdef HAVE_NETMAP
            int fd = -1;

            if (netmap_version != -1 && (fd = open("/dev/netmap", O_RDWR)) < 0)
                continue;

            bzero(&nmr, sizeof(nmr));
            strncpy(nmr.nr_name, pcap_if_ptr->name, sizeof(nmr.nr_name));
            nmr.nr_version = netmap_version;
            if (fd > 0 && ioctl(fd, NIOCGINFO, &nmr) == 0) {
#endif /* HAVE_NETMAP */
#if defined HAVE_LIBPCAP_NETMAP || defined HAVE_NETMAP
                list_ptr->next = (interface_list_t *)safe_malloc(sizeof(interface_list_t));
                list_ptr = list_ptr->next;
                snprintf(list_ptr->name, sizeof(list_ptr->name), "vale:%s", pcap_if_ptr->name);
                sprintf(list_ptr->alias, "%%%d", i++);
                list_ptr->flags = pcap_if_ptr->flags;

                list_ptr->next = (interface_list_t *)safe_malloc(sizeof(interface_list_t));
                list_ptr = list_ptr->next;
                snprintf(list_ptr->name, sizeof(list_ptr->name), "netmap:%s", pcap_if_ptr->name);
                sprintf(list_ptr->alias, "%%%d", i++);
                list_ptr->flags = pcap_if_ptr->flags;
#endif /* HAVE_LIBPCAP_NETMAP  || HAVE_NETMAP */
#ifdef HAVE_NETMAP
                if (netmap_version >= 10) {
                    int x;

                    list_ptr->next = (interface_list_t *)safe_malloc(sizeof(interface_list_t));
                    list_ptr = list_ptr->next;
                    snprintf(list_ptr->name, sizeof(list_ptr->name), "netmap:%s!", pcap_if_ptr->name);
                    sprintf(list_ptr->alias, "%%%d", i++);
                    list_ptr->flags = pcap_if_ptr->flags;

                    list_ptr->next = (interface_list_t *)safe_malloc(sizeof(interface_list_t));
                    list_ptr = list_ptr->next;
                    snprintf(list_ptr->name, sizeof(list_ptr->name), "netmap:%s*", pcap_if_ptr->name);
                    sprintf(list_ptr->alias, "%%%d", i++);
                    list_ptr->flags = pcap_if_ptr->flags;

                    list_ptr->next = (interface_list_t *)safe_malloc(sizeof(interface_list_t));
                    list_ptr = list_ptr->next;
                    snprintf(list_ptr->name, sizeof(list_ptr->name), "netmap:%s^", pcap_if_ptr->name);
                    sprintf(list_ptr->alias, "%%%d", i++);
                    list_ptr->flags = pcap_if_ptr->flags;
                    for (x = 0; x < nmr.nr_rx_rings; ++x) {
                        list_ptr->next = (interface_list_t *)safe_malloc(sizeof(interface_list_t));
                        list_ptr = list_ptr->next;
                        snprintf(list_ptr->name, sizeof(list_ptr->name), "netmap:%s-%d", pcap_if_ptr->name, x);
                        sprintf(list_ptr->alias, "%%%d", i++);
                        list_ptr->flags = pcap_if_ptr->flags;
                    }
                }
            }
            close(fd);
#endif /* HAVE_NETMAP */
#ifdef HAVE_LIBPCAP_NETMAP
        }
#endif /* HAVE_LIBPCAP_NETMAP */
#ifdef HAVE_PF_RING_PCAP
        list_ptr->next = (interface_list_t *)safe_malloc(sizeof(interface_list_t));
        list_ptr = list_ptr->next;
        snprintf(list_ptr->name, sizeof(list_ptr->name), "zc:%s", pcap_if_ptr->name);
        sprintf(list_ptr->alias, "%%%d", i++);
        list_ptr->flags = pcap_if_ptr->flags;
#endif
        pcap_if_ptr = pcap_if_ptr->next;
    }
    pcap_freealldevs(pcap_if);

    /* look for khial device: https://github.com/boundary/khial */
    if ((dir = opendir("/dev/char")) != NULL) {
        while ((dirdata = readdir(dir)) != NULL) {
            if (strncmp(dirdata->d_name, "testpackets", strlen("testpackets")) == 0) {
                if (i > 0) {
                    list_ptr->next = (interface_list_t *)safe_malloc(sizeof(interface_list_t));
                    list_ptr = list_ptr->next;
                }
                dbgx(3, "Adding %s to interface list", dirdata->d_name);
                snprintf(testnic, sizeof(testnic), "/dev/char/%s", dirdata->d_name);
                strlcpy(list_ptr->name, testnic, sizeof(list_ptr->name));
                snprintf(testnic, sizeof(testnic), "khial pseudo-nic: %s", dirdata->d_name);
                strlcpy(list_ptr->description, testnic, sizeof(list_ptr->description));
                strlcpy(list_ptr->alias, dirdata->d_name, sizeof(list_ptr->alias));
                i += 1;
            }
        }
        closedir(dir);
    }

    dbg(1, "xxx get_interface_list end");
    return (list_head);
}

/**
 * Prints all the available interfaces found by get_interface_list()
 */
void
list_interfaces(interface_list_t *list)
{
    interface_list_t *ptr;

    if (list == NULL) {
        printf("No network interfaces available");
        return;
    }

    printf("Available network interfaces:\n");

#ifdef HAVE_WIN32 /* Win32 has alias/name/description */
    printf("Alias\tName\tDescription\n");
#endif

    ptr = list;

    do {
        if (!(ptr->flags & PCAP_IF_LOOPBACK)) {
#ifdef HAVE_WIN32
            printf("%s\t%s\n\t%s\n", ptr->alias, ptr->name, ptr->description);
#else
            printf("%s\n", ptr->name);
#endif
        }
        ptr = ptr->next;
    } while (ptr != NULL);
}
