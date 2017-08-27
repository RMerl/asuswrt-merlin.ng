/*
    Copyright (c)  2006, 2007		Dmitry Butskoy
					<buc@citadel.stu.neva.ru>
    License:  GPL v2 or any later

    See COPYING for the status of this software.
*/

#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <math.h>

#include "traceroute.h"


static struct pollfd *pfd = NULL;
static unsigned int num_polls = 0;


void add_poll (int fd, int events) {
	int i;

	for (i = 0; i < num_polls && pfd[i].fd > 0; i++) ;

	if (i == num_polls) {
	    pfd = realloc (pfd, ++num_polls * sizeof (*pfd));
	    if (!pfd)  error ("realloc");
	}

	pfd[i].fd = fd;
	pfd[i].events = events;
}


void del_poll (int fd) {
	int i;

	for (i = 0; i < num_polls && pfd[i].fd != fd; i++) ;

	if (i < num_polls)  pfd[i].fd = -1;    /*  or just zero it...  */
}


static int cleanup_polls (void) {
	int i;

	for (i = 0; i < num_polls && pfd[i].fd > 0; i++) ;

	if (i < num_polls) {	/*  a hole have found   */
	    int j;

	    for (j = i + 1; j < num_polls; j++) {
		if (pfd[j].fd > 0) {
		    pfd[i++] = pfd[j];
		    pfd[j].fd = -1;
		}
	    }
	}

	return i;
}


void do_poll (double timeout, void (*callback) (int fd, int revents)) {
	int nfds;
	int msecs = ceil (timeout * 1000);

	while ((nfds = cleanup_polls ()) > 0) {
	    int i, n;

	    n = poll (pfd, nfds, msecs);

	    if (n <= 0) {
		if (n == 0 || errno == EINTR)
			return;
		error ("poll");
	    }

	    for (i = 0; n && i < num_polls; i++) {
		if (pfd[i].revents) {
		    callback (pfd[i].fd, pfd[i].revents);
		    n--;
		}
	    }

	    msecs = 0;	    /*  no more wait, just eat all the pending   */
	}

	return;
}

