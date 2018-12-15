/* generates random data for later use in test cases. Of course,
 * we could generate random data during the testcase itself, but
 * the core idea is that we record the random data so that we have
 * a chance to reproduce a problem should it occur. IMHO this
 * provides the best compromise, by a) having randomness but
 * b) knowing what was used during the test.
 *
 * Params
 * -f	output file name (stdout if not given)
 * -s	size of test data, plain number is size in k, 1MB default
 * -u   uses /dev/urandom instead of libc random number generator
 *      (when available). Note that this is usually much slower.
 *
 * Part of the testbench for rsyslog.
 *
 * Copyright 2010-2016 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of rsyslog.
 *
 * Rsyslog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rsyslog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rsyslog.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>

#define EXIT_FAILURE 1

static char *fileName = NULL;		/* name of output file */
static int tryUseURandom = 0;		/* try to use /dev/urandom? */
static long long fileSize = 1024*1024;	/* file size in K, 1MB default */


/* generate the random file. This code really can be improved (e.g. read /dev/urandom
 * when available)
 */
static void
genFile()
{
	long i;
	FILE *fp;
	FILE *rfp = NULL;

	if(fileName == NULL) {
		fp = stdout;
	} else {
		if((fp = fopen(fileName, "w")) == NULL) {
			perror(fileName);
		}
	}

	/* try to use /dev/urandom, if available */
	if(tryUseURandom)
		rfp = fopen("/dev/urandom", "r");

	if(rfp == NULL) {
		/* fallback, use libc random number generator */
		for(i = 0 ; i < fileSize ; ++i) {
			if(fputc((char) rand(), fp) == EOF) {
				perror(fileName);
				exit(1);
			}
		}
	} else {
		/* use /dev/urandom */
		printf("using /dev/urandom");
		for(i = 0 ; i < fileSize ; ++i) {
			if(fputc(fgetc(rfp), fp) == EOF) {
				perror(fileName);
				exit(1);
			}
		}
	}

	if(fileName != NULL)
		fclose(fp);
}


/* Run the test.
 * rgerhards, 2009-04-03
 */
int main(int argc, char *argv[])
{
	int ret = 0;
	int opt;

	srand(time(NULL));	/* seed is good enough for our needs */

	while((opt = getopt(argc, argv, "f:s:u")) != -1) {
		switch (opt) {
		case 'f':	fileName = optarg;
				break;
		case 's':	fileSize = atol(optarg) * 1024;
				break;
		case 'u':	tryUseURandom = 1;
				break;
		default:	printf("invalid option '%c' or value missing - terminating...\n", opt);
				exit (1);
				break;
		}
	}

	printf("generating random data file '%s' of %ldkb - may take a short while...\n",
		fileName, (long) (fileSize / 1024));
	genFile();

	exit(ret);
}
