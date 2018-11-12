/*
 * swinst_rpm.c:
 *     hrSWInstalledTable data access:
 */
#include <net-snmp/net-snmp-config.h>

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
#include <sys/stat.h>

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/library/container.h>
#include <net-snmp/library/snmp_debug.h>
#include <net-snmp/data_access/swinst.h>
#include "swinst_private.h"

config_require(date_n_time)

char pkg_directory[SNMP_MAXBUF];
static char apt_fmt[SNMP_MAXBUF];
static char file[SNMP_MAXBUF];

/* ---------------------------------------------------------------------
 */
void
netsnmp_swinst_arch_init(void)
{
    strlcpy(pkg_directory, "/var/lib/dpkg/info", sizeof(pkg_directory));
    snprintf(apt_fmt, SNMP_MAXBUF, "%%%d[^#]#%%%d[^#]#%%%d[^#]#%%%d[^#]#%%%d[^#]#%%%d[^#]#%%%ds",
	SNMP_MAXBUF-1, SNMP_MAXBUF-1, SNMP_MAXBUF-1, SNMP_MAXBUF-1,
	SNMP_MAXBUF-1, SNMP_MAXBUF-1, SNMP_MAXBUF-1);
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
    FILE *p = popen("dpkg-query --show --showformat '${Package}#${Version}#${Section}#${Priority}#${Essential}#${Architecture}#${Status}\n'", "r");
    char package[SNMP_MAXBUF];
    char version[SNMP_MAXBUF];
    char section[SNMP_MAXBUF];
    char priority[SNMP_MAXBUF];
    char essential[SNMP_MAXBUF];
    char arch[SNMP_MAXBUF];
    char status[SNMP_MAXBUF];
    char buf[BUFSIZ];
    struct stat stat_buf;
    netsnmp_swinst_entry *entry;
    u_char *date_buf;
    size_t date_len;
    int i = 1;

    if (p == NULL) {
	snmp_perror("dpkg-list");
	return 1;
    }

    while (fgets(buf, BUFSIZ, p)) {
	DEBUGMSG(("swinst_apt", "entry: %s\n", buf));
        entry = netsnmp_swinst_entry_create( i++ );
        if (NULL == entry)
            continue;   /* error already logged by function */
        CONTAINER_INSERT(container, entry);

	sscanf(buf, apt_fmt, package, version, section, priority, essential, arch, status);
	if (strstr(status, "not-installed"))
	    continue;

        entry->swName_len = snprintf( entry->swName, sizeof(entry->swName),
                                      "%s_%s_%s", package, version, arch);
	if (entry->swName_len >= sizeof(entry->swName))
	    entry->swName_len = sizeof(entry->swName)-1;
        entry->swType = (strcmp(essential, "yes") == 0)
                        ? 2      /* operatingSystem */
                        : 4;     /*  application    */

        /* get the last mod date */
        snprintf(file, sizeof(file), "%s/%s.list", pkg_directory, package);
        if(stat(file, &stat_buf) != -1) {
            date_buf = date_n_time(&stat_buf.st_mtime, &date_len);
            entry->swDate_len = date_len;
            memcpy(entry->swDate, date_buf, entry->swDate_len);
        } else {
            /* somewhy some files include :arch in .list name */
            snprintf(file, sizeof(file), "%s/%s:%s.list", pkg_directory, package, arch);
            if(stat(file, &stat_buf) != -1) {
                date_buf = date_n_time(&stat_buf.st_mtime, &date_len);
                entry->swDate_len = date_len;
                memcpy(entry->swDate, date_buf, entry->swDate_len);
            }
        }
        /* FIXME, or fallback to whatever nonsesnse was here before, or leave it uninitialied?
             else {
        entry->swDate_len = 8;
	memcpy(entry->swDate, "\0\0\1\1\0\0\0\0", 8);
    }
        */
    }
    pclose(p);
    DEBUGMSGTL(("swinst:load:arch"," loaded %d entries\n",
                (int) CONTAINER_SIZE(container)));

    return 0;
}
