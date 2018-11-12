/*
 * swinst_pkginfo.c:
 *     hrSWInstalledTable data access:
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>

#include <stdio.h>
#include <sys/stat.h>
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
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

#ifdef HAVE_PKGLOCS_H
#include <pkglocs.h>
#endif
#ifdef HAVE_PKGINFO_H
#include <pkginfo.h>
#endif

#ifdef HAVE_PKG_H
#define restrict
#include <pkg.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/library/container.h>
#include <net-snmp/library/snmp_debug.h>
#include <net-snmp/data_access/swinst.h>
#include "swinst_private.h"

netsnmp_feature_require(date_n_time)

   /*
    * Location of package directory.
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
#if defined(PKGLOC)
    snprintf( pkg_directory, SNMP_MAXPATH, "%s", PKGLOC );
#elif defined(hpux9)
    snprintf( pkg_directory, SNMP_MAXPATH, "/system" );
#elif defined(hpux10) ||  defined(hpux11) 
    snprintf( pkg_directory, SNMP_MAXPATH, "/var/adm/sw/products" );
#elif defined(freebsd2) || defined(openbsd)
    snprintf( pkg_directory, SNMP_MAXPATH, "/var/db/pkg" );
#elif defined(linux)
    snprintf( pkg_directory, SNMP_MAXPATH, "/var/cache/hrmib" );
#else
    pkg_directory[0] = '\0';
    snmp_log( LOG_ERR, "SWInst: No package directory\n" );
#endif

    DEBUGMSGTL(("swinst:init:arch"," package directory = %s\n",
                                         pkg_directory));
    return;
}

void
netsnmp_swinst_arch_shutdown(void)
{
    pkg_directory[0] = '\0';
    return;
}

/* ---------------------------------------------------------------------
 */
int
netsnmp_swinst_arch_load( netsnmp_container *container, u_int flags)
{
    DIR                  *d;
    struct dirent        *dp;
    struct stat           stat_buf;
#ifdef HAVE_PKGINFO
    char                 *v, *c;
#endif
    char                  buf[ BUFSIZ ];
    unsigned char        *cp;
    time_t                install_time;
    size_t                date_len;
    int                   i = 1;
    netsnmp_swinst_entry *entry;
#ifdef HAVE_LIBPKG
    struct pkgdb *db = NULL;
    struct pkgdb_it *it = NULL;
    struct pkg *pkg = NULL;
    char pkgname[ SNMP_MAXPATH ];
    char pkgdate[ BUFSIZ ];
    int pkgng = 0;

    if (pkg_init(NULL, NULL)) {
        snmp_log( LOG_ERR, "SWInst: error initializing pkgng db\n" );
        return 1;
    }

    if (pkgdb_open(&db, PKGDB_DEFAULT) != EPKG_OK) {
	snmp_log( LOG_ERR, "SWInst: error opening pkgng db\n" );
	return 1;
    }

    if (pkg_status(NULL) == PKG_STATUS_ACTIVE) {
	pkgng = 1;
    } else {
	snmp_log( LOG_INFO, "SWInst: not a pkgng system\n" );
    }

    /* if we are using FreeBSD's pkgng */
    if (pkgng) {
        if ((it = pkgdb_query(db, NULL, MATCH_ALL)) == NULL) {
            snmp_log( LOG_ERR, "SWInst: error querying pkgng db\n" );
            return 1;
        }

        while (pkgdb_it_next(it, &pkg, PKG_LOAD_BASIC) == EPKG_OK) {
            pkg_snprintf(pkgname, sizeof(pkgname), "%n-%v", pkg, pkg);
            pkg_snprintf(pkgdate, sizeof(pkgdate), "%t", pkg);

            entry = netsnmp_swinst_entry_create( i++ );

            if (NULL == entry)
                continue;   /* error already logged by function */

            CONTAINER_INSERT(container, entry);

            entry->swName_len = snprintf( entry->swName, sizeof(entry->swName),
                                          "%s", pkgname );
            if (entry->swName_len >= sizeof(entry->swName))
                entry->swName_len = sizeof(entry->swName)-1;

            install_time = atoi(pkgdate);
            cp = date_n_time( &install_time, &date_len );
            memcpy( entry->swDate, cp, date_len );
            entry->swDate_len = date_len;
        }

        pkgdb_it_free(it);
        pkgdb_close(db);
        pkg_shutdown();
    } else {
#endif /* HAVE_LIBPKG */
        if ( !pkg_directory[0] ) {
            return 1;    /* Can't report installed packages
                         if there isn't a list of them! */
        }

        d = opendir( pkg_directory );
        if (!d)
            return 1;

        while ((dp = readdir(d)) != NULL) {
            if ( '.' == dp->d_name[0] )
                continue;
            snprintf( buf, BUFSIZ, "%s/%s", pkg_directory, dp->d_name );
            if (stat( buf, &stat_buf ) < 0)
                continue;
            entry = netsnmp_swinst_entry_create( i++ );
            if (NULL == entry)
                continue;   /* error already logged by function */
            CONTAINER_INSERT(container, entry);

#ifdef HAVE_PKGINFO
	    v = pkgparam( dp->d_name, "VERSION" );
	    c = pkgparam( dp->d_name, "CATEGORY" );

	    entry->swName_len = snprintf( entry->swName, sizeof(entry->swName),
					  "%s-%s", dp->d_name, v );
	    if (entry->swName_len >= sizeof(entry->swName))
		entry->swName_len = sizeof(entry->swName)-1;
	    entry->swType = (NULL != strstr( c, "system"))
			    ? 2      /* operatingSystem */
			    : 4;     /*  application    */

	    /* pkgparam() return values must be freed. */
	    free(c);
	    free(v);
#else
	    entry->swName_len = snprintf( entry->swName, sizeof(entry->swName),
					  "%s", dp->d_name );
	    if (entry->swName_len >= sizeof(entry->swName))
		entry->swName_len = sizeof(entry->swName)-1;

	    /* no information about O/S vs application packages ??? */
#endif

	    install_time = stat_buf.st_mtime;
	    cp = date_n_time( &install_time, &date_len );
	    memcpy( entry->swDate, cp, date_len );
	    entry->swDate_len = date_len;
	}
	closedir( d );
#ifdef HAVE_LIBPKG
    }
#endif

    DEBUGMSGTL(("swinst:load:arch"," loaded %d entries\n",
                (int)CONTAINER_SIZE(container)));

    return 0;
}
