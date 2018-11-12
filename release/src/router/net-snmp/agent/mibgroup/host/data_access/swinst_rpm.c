/*
 * swinst_rpm.c:
 *     hrSWInstalledTable data access:
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_RPM_RPMLIB_H
#include <rpm/rpmlib.h>
#endif
#ifdef HAVE_RPM_RPMLIB_H
#include <rpm/header.h>
#endif
#ifdef HAVE_RPMGETPATH		/* HAVE_RPM_RPMMACRO_H */
#include <rpm/rpmmacro.h>
#endif
#ifdef HAVE_RPM_RPMTS_H
#include <rpm/rpmts.h>
#include <rpm/rpmdb.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/library/container.h>
#include <net-snmp/library/snmp_debug.h>
#include <net-snmp/data_access/swinst.h>
#include "swinst_private.h"

netsnmp_feature_require(date_n_time)

   /*
    * Location of RPM package directory.
    * Used for:
    *    - reporting hrSWInstalledLast* objects
    *    - detecting when the cached contents are out of date.
    */
char pkg_directory[SNMP_MAXPATH];

/* ---------------------------------------------------------------------
 */
void
netsnmp_swinst_arch_init(void)
{
    char        *rpmdbpath = NULL;
    const char  *dbpath;
    struct stat  stat_buf;

#ifdef HAVE_RPMGETPATH
    rpmReadConfigFiles( NULL, NULL );
    rpmdbpath = rpmGetPath( "%{_dbpath}", NULL );
    dbpath = rpmdbpath;
#else
    dbpath = "/var/lib/rpm";   /* Most likely */
#endif

    snprintf( pkg_directory, SNMP_MAXPATH, "%s/Packages", dbpath );
    SNMP_FREE(rpmdbpath);
    dbpath = NULL;
    if (-1 == stat( pkg_directory, &stat_buf )) {
        snmp_log(LOG_ERR, "Can't find directory of RPM packages");
        pkg_directory[0] = '\0';
    }
}

void
netsnmp_swinst_arch_shutdown(void)
{
     /* Nothing to do */
     return;
}

/* ---------------------------------------------------------------------
 */
int
netsnmp_swinst_arch_load( netsnmp_container *container, u_int flags)
{
    rpmts                 ts;

    rpmdbMatchIterator    mi;
    Header                h;
#if HAVE_HEADERGET
    const char           *g;
    rpmtd                 td_name, td_version, td_release, td_group, td_time;
#else
    char                 *n, *v, *r, *g;
    int32_t              *t;
#endif
    time_t                install_time;
    size_t                date_len;
    int                   i = 1;
    netsnmp_swinst_entry *entry;

#if HAVE_HEADERGET
    td_name = rpmtdNew();
    td_version = rpmtdNew();
    td_release = rpmtdNew();
    td_group = rpmtdNew();
    td_time = rpmtdNew();
#endif
    ts = rpmtsCreate();
    rpmtsSetVSFlags( ts, (_RPMVSF_NOSIGNATURES|_RPMVSF_NODIGESTS));

    mi = rpmtsInitIterator( ts, RPMDBI_PACKAGES, NULL, 0);
    if (mi == NULL)
	NETSNMP_LOGONCE((LOG_ERR, "rpmdbOpen() failed\n"));

    while (NULL != (h = rpmdbNextIterator( mi )))
    {
        const u_char *dt;

        entry = netsnmp_swinst_entry_create( i++ );
        if (NULL == entry)
            continue;   /* error already logged by function */
        CONTAINER_INSERT(container, entry);

        h = headerLink( h );
#if HAVE_HEADERGET
        headerGet(h, RPMTAG_NAME, td_name, HEADERGET_EXT);
        headerGet(h, RPMTAG_VERSION, td_version, HEADERGET_EXT);
        headerGet(h, RPMTAG_RELEASE, td_release, HEADERGET_EXT);
        headerGet(h, RPMTAG_GROUP, td_group, HEADERGET_EXT);
        headerGet(h, RPMTAG_INSTALLTIME, td_time, HEADERGET_EXT);
        entry->swName_len = snprintf( entry->swName, sizeof(entry->swName),
                                      "%s-%s-%s", rpmtdGetString(td_name),
                                      rpmtdGetString(td_version),
                                      rpmtdGetString(td_release));
        install_time = rpmtdGetNumber(td_time);
        g = rpmtdGetString(td_group);
#else
        headerGetEntry( h, RPMTAG_NAME,        NULL, (void**)&n, NULL);
        headerGetEntry( h, RPMTAG_VERSION,     NULL, (void**)&v, NULL);
        headerGetEntry( h, RPMTAG_RELEASE,     NULL, (void**)&r, NULL);
        headerGetEntry( h, RPMTAG_GROUP,       NULL, (void**)&g, NULL);
        headerGetEntry( h, RPMTAG_INSTALLTIME, NULL, (void**)&t, NULL);
        entry->swName_len = snprintf( entry->swName, sizeof(entry->swName),
                                      "%s-%s-%s", n, v, r);
        install_time = *t;
#endif
        entry->swType = (g && NULL != strstr( g, "System Environment"))
                        ? 2      /* operatingSystem */
                        : 4;     /*  application    */
        if (entry->swName_len > sizeof(entry->swName))
            entry->swName_len = sizeof(entry->swName);

        dt = date_n_time( &install_time, &date_len );
        if (date_len != 8 && date_len != 11) {
            snmp_log(LOG_ERR, "Bogus length from date_n_time for %s", entry->swName);
            entry->swDate_len = 0;
        }
        else {
            entry->swDate_len = date_len;
            memcpy(entry->swDate, dt, entry->swDate_len);
        }

#if HAVE_HEADERGET
        rpmtdFreeData(td_name);
        rpmtdFreeData(td_version);
        rpmtdFreeData(td_release);
        rpmtdFreeData(td_group);
        rpmtdFreeData(td_time);
#endif
        headerFree( h );
    }
    rpmdbFreeIterator( mi );
    rpmtsFree( ts );
#if HAVE_HEADERGET
    rpmtdFree(td_name);
    rpmtdFree(td_version);
    rpmtdFree(td_release);
    rpmtdFree(td_group);
    rpmtdFree(td_time);
#endif

    DEBUGMSGTL(("swinst:load:arch", "loaded %d entries\n",
                (int)CONTAINER_SIZE(container)));

    return 0;
}
