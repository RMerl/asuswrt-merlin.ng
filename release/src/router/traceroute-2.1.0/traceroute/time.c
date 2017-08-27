/*
    Copyright (c)  2006, 2007		Dmitry Butskoy
					<buc@citadel.stu.neva.ru>
    License:  GPL v2 or any later

    See COPYING for the status of this software.
*/

#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include "traceroute.h"


/*  Just returns current time as double, with most possible precision...  */

double get_time (void) {
	struct timeval tv;
	double d;

	gettimeofday (&tv, NULL);

	d = ((double) tv.tv_usec) / 1000000. + (unsigned long) tv.tv_sec;

	return d;
}
