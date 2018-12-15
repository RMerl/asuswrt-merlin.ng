/* generate an input file suitable for use by the testbench
 * Copyright (C) 2016-2018 by Pascal Withopf and Adiscon GmbH.
 * usage: ./inputfilegen num-lines > file
 * Part of rsyslog, licensed under ASL 2.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#if defined(_AIX)
	#include  <unistd.h>
#else
	#include <getopt.h>
#endif

#define DEFMSGS 5
#define NOEXTRADATA -1

int main(int argc, char* argv[])
{
	int c, i;
	int space = 0;
	int nmsgs = DEFMSGS;
	int nmsgstart = 0;
	int nchars = NOEXTRADATA;
	int errflg = 0;
	char *extradata = NULL;

	while((c=getopt(argc, argv, "pm:i:d:")) != -1) {
		switch(c) {
		case 'm':
			nmsgs = atoi(optarg);
			break;
		case 'i':
			nmsgstart = atoi(optarg);
			break;
		case 'd':
			nchars = atoi(optarg);
			break;
		case 'p':
			space = 1;
			break;
		case ':':
			fprintf(stderr, "Option -%c requires an operand\n", optopt);
			errflg++;
			break;
		case '?':
			fprintf(stderr, "Unrecognized option: -%c\n", optopt);
			errflg++;
			break;
		}
	}
	if(errflg) {
		fprintf(stderr, "Usage: -m <nmsgs> -d <nchars> -p\n");
		exit(2);
	}
	if(nchars != NOEXTRADATA) {
		extradata = (char *)malloc(nchars + 1);
		memset(extradata, 'X', nchars);
		extradata[nchars] = '\0';
	}
	for(i = nmsgstart; i < (nmsgs+nmsgstart); ++i) {
		printf("msgnum:%8.8d:", i);
		if(space==1) {
			printf("\n ");
		}
		if(nchars != NOEXTRADATA) {
			printf("%s", extradata);
		}
		printf("\n");
	}
	free(extradata);
	return 0;
}
