/*
 *  Template MIB group implementation - at.c
 *
 */

/* Portions of this file are subject to the following copyright(s).  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 */
/*
 * Portions of this file are copyrighted by:
 * Copyright © 2003 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/data_access/interface.h>
#include "../at.h"
#include <linux/if_arp.h>

/*
 * at used to be allocated every time we needed to look at the arp cache.
 * This cause us to parse /proc/net/arp twice for each request and didn't
 * allow us to filter things like we'd like to.  So now we use it
 * semi-statically.  We initialize it to size 0 and if we need more room
 * we realloc room for ARP_CACHE_INCR more entries in the table.
 * We never release what we've taken . . .
 */
#define ARP_CACHE_INCR 1024

static int arptab_size, arptab_current;
static struct arptab *at;

void
ARP_Scan_Init(void)
{
    static time_t   tm = 0;     /* Time of last scan */
    FILE           *in;
    int             i, j;
    char            line[128];
    int             za, zb, zc, zd;
    char            ifname[21];
    char            mac[3*MAX_MAC_ADDR_LEN+1];
    char           *tok;

    arptab_current = 0;         /* Anytime this is called we need to reset 'current' */

    if (time(NULL) < tm + 1) {  /*Our cool one second cache implementation :-) */
        return;
    }

    in = fopen("/proc/net/arp", "r");
    if (!in) {
        snmp_log_perror("mibII/at: Cannot open /proc/net/arp");
        arptab_size = 0;
        return;
    }

    /*
     * Get rid of the header line
     */
    fgets(line, sizeof(line), in);

    i = 0;
    while (fgets(line, sizeof(line), in)) {
        static int      arptab_curr_max_size;
        u_long          tmp_a;
        unsigned int    tmp_flags;

        if (i >= arptab_curr_max_size) {
            struct arptab  *newtab = (struct arptab *)
                realloc(at, (sizeof(struct arptab) *
                             (arptab_curr_max_size + ARP_CACHE_INCR)));
            if (newtab == NULL) {
                snmp_log(LOG_ERR,
                         "Error allocating more space for arpcache.  "
                         "Cache will continue to be limited to %d entries",
                         arptab_curr_max_size);
                newtab = at;
                break;
            } else {
                arptab_curr_max_size += ARP_CACHE_INCR;
                at = newtab;
            }
        }
        if (7 != sscanf(line, "%d.%d.%d.%d 0x%*x 0x%x %s %*[^ ] %20s\n",
                        &za, &zb, &zc, &zd, &tmp_flags, mac, ifname)) {
            snmp_log(LOG_ERR, "Bad line in /proc/net/arp: %s", line);
            continue;
        }
        /*
         * Invalidated entries have their flag set to 0.
         * * We want to ignore them
         */
        if (tmp_flags == 0) {
            continue;
        }
        ifname[sizeof(ifname)-1] = 0; /* make sure name is null terminated */
        at[i].at_flags = tmp_flags;
        tmp_a = ((u_long) za << 24) |
            ((u_long) zb << 16) | ((u_long) zc << 8) | ((u_long) zd);
        at[i].at_iaddr.s_addr = htonl(tmp_a);
        at[i].if_index = netsnmp_access_interface_index_find(ifname);

        for (j=0,tok=strtok(mac, ":"); tok != NULL; tok=strtok(NULL, ":"),j++) {
        	at[i].at_enaddr[j] = strtol(tok, NULL, 16);
        }
        at[i].at_enaddr_len = j;
        i++;
    }
    arptab_size = i;

    fclose(in);
    time(&tm);
}

int
ARP_Scan_Next(in_addr_t * IPAddr, char *PhysAddr, int *PhysAddrLen,
              u_long * ifType, u_short * ifIndex)
{
    if (arptab_current >= arptab_size)
        return 0;

    /*
     * copy values
     */
    *IPAddr = at[arptab_current].at_iaddr.s_addr;
    *ifType = at[arptab_current].at_flags & ATF_PERM ?
        4 /*static */ : 3 /*dynamic */;
    *ifIndex = at[arptab_current].if_index;
    memcpy(PhysAddr, &at[arptab_current].at_enaddr,
           sizeof(at[arptab_current].at_enaddr));
    *PhysAddrLen = at[arptab_current].at_enaddr_len;

    /*
     * increment to point next entry
     */
    arptab_current++;
    /*
     * return success
     */
    return 1;
}
