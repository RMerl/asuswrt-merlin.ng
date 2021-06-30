#include <net-snmp/net-snmp-config.h>

#include <stdio.h>
#if HAVE_DIRENT_H
#include <dirent.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <net-snmp/agent/netsnmp_close_fds.h>

/**
 * Close all file descriptors larger than @fd.
 */
void netsnmp_close_fds(int fd)
{
#if defined(HAVE_FORK)
    DIR            *dir NETSNMP_ATTRIBUTE_UNUSED;
    struct dirent  *ent NETSNMP_ATTRIBUTE_UNUSED;
    int             i, largest_fd = -1;

    if (fd < -1)
        fd = -1;

#ifdef __linux__
    if ((dir = opendir("/proc/self/fd"))) {
        while ((ent = readdir(dir))) {
            if (sscanf(ent->d_name, "%d", &i) == 1) {
                if (i > largest_fd)
                    largest_fd = i;
            }
        }
        closedir(dir);
    }
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
    if ((dir = opendir("/dev/fd"))) {
        while ((ent = readdir(dir))) {
            if (sscanf(ent->d_name, "%d", &i) == 1) {
                if (i > largest_fd)
                    largest_fd = i;
            }
        }
        closedir(dir);
    }
#endif
    if (largest_fd < 0) {
#ifdef HAVE_GETDTABLESIZE
        largest_fd = getdtablesize() - 1;
#else
        largest_fd = sysconf(_SC_OPEN_MAX);
#endif
    }

    for (i = largest_fd; i > fd && i >= 0; i--)
        close(i);
#endif
}
