/* rsyslog testbench tool to mangle .qi files
 *
 * Copyright (C) 2016 by Rainer Gerhards
 * Released uner ASL 2.0
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

static int debug = 0;

void
usage(void)
{
	fprintf(stderr, "mangle_qi -d -q <.qi-file>\n"
		"-d enables debug messages\n");
	exit(1);
}

void
processQI(FILE *const __restrict__ qi)
{
	char lnbuf[4096];
	char propname[64];
	int rectype;
	int length;
	int queuesize;
	int i;
	int c;
	fgets(lnbuf, sizeof(lnbuf), qi);
	fputs(lnbuf, stdout);
	/* we now read the queue size line */
	/* note: this is quick and dirty, no error checks
	 * are done!
	 */
	fgetc(qi); /* skip '+' */
	for(i = 0 ; (c = fgetc(qi)) != ':' ; ++i) {
		propname[i] = c;
	}
	propname[i] = '\0';
	if(strcmp(propname, "iQueueSize")) {
		fprintf(stderr, ".qi file format unknown: line 2 does "
			"not contain iQueueSize property, instead '%s'\n",
			propname);
		exit(1);
	}

	rectype = 0;
	for(c = fgetc(qi) ; isdigit(c) ; c = fgetc(qi))
		rectype = rectype * 10 + c - '0';

	length = 0;
	for(c = fgetc(qi) ; isdigit(c) ; c = fgetc(qi))
		length = length * 10 + c - '0';

	queuesize = 0;
	for(c = fgetc(qi) ; isdigit(c) ; c = fgetc(qi))
		queuesize = queuesize * 10 + c - '0';

	int maxval_for_length = 10;
	for(i = 1 ; i < length ; ++i)
		maxval_for_length *= 10;	/* simulate int-exp() */

	if(debug) {
		fprintf(stderr, "rectype: %d\n", rectype);
		fprintf(stderr, "length: %d\n", length);
		fprintf(stderr, "queuesize: %d\n", queuesize);
		fprintf(stderr, "maxval_for_length: %d\n", maxval_for_length);
	}
	
	queuesize += 1; /* fake invalid queue size */
	if(queuesize > maxval_for_length)
		++length;

	/* ready to go, write mangled queue size */
	printf("+%s:%d:%d:%d:", propname, rectype, length, queuesize);
	/* copy rest of file */
	while((c = fgetc(qi)) != EOF)
		putchar(c);
}

int
main(int argc, char *argv[])
{
	char *qifile;
	FILE *qi;
	int opt;
	while((opt = getopt(argc, argv, "dq:")) != -1) {
		switch (opt) {
		case 'q':	qifile = optarg;
				break;
		case 'd':	debug = 1;
				break;
		default:	usage();
				break;
		}
	}

	if((qi = fopen(qifile, "r")) == NULL) {
		perror(qifile);
		exit(1);
	}
	processQI(qi);
	return 0;
}
