/*
 * dup2.c
 *
 * Ersatz dup2() for really ancient systems
 */

#include "config.h"

int dup2(int oldfd, int newfd)
{
    int rv, nfd;

    close(newfd);

    nfd = rv = dup(oldfd);

    if (rv >= 0 && rv != newfd) {
        rv = dup2(oldfd, newfd);
        close(nfd);
    }

    return rv;
}
