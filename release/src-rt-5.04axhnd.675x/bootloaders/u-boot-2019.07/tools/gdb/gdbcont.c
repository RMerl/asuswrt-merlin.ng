// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Murray Jensen <Murray.Jensen@csiro.au>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "serial.h"
#include "error.h"
#include "remote.h"

char *serialdev = "/dev/term/b";
speed_t speed = B230400;
int verbose = 0;

int
main(int ac, char **av)
{
    int c, sfd;

    if ((pname = strrchr(av[0], '/')) == NULL)
	pname = av[0];
    else
	pname++;

    while ((c = getopt(ac, av, "b:p:v")) != EOF)
	switch (c) {

	case 'b':
	    if ((speed = cvtspeed(optarg)) == B0)
		Error("can't decode baud rate specified in -b option");
	    break;

	case 'p':
	    serialdev = optarg;
	    break;

	case 'v':
	    verbose = 1;
	    break;

	default:
	usage:
	    fprintf(stderr, "Usage: %s [-b bps] [-p dev] [-v]\n", pname);
	    exit(1);
	}
    if (optind != ac)
	goto usage;

    if (verbose)
	fprintf(stderr, "Opening serial port and sending continue...\n");

    if ((sfd = serialopen(serialdev, speed)) < 0)
	Perror("open of serial device '%s' failed", serialdev);

    remote_desc = sfd;
    remote_reset();
    remote_continue();

    if (serialclose(sfd) < 0)
	Perror("close of serial device '%s' failed", serialdev);

    if (verbose)
	fprintf(stderr, "Done.\n");

    return (0);
}
