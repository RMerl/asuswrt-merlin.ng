/* Checks if a file consists of line of strictly monotonically
 * increasing numbers. An expected start and end number may
 * be set.
 *
 * Params
 * -f<filename> file to process, if not given stdin is processed.
 * -s<starting number> -e<ending number>
 * default for s is 0. -e should be given (else it is also 0)
 * -d may be specified, in which case duplicate messages are permitted.
 * -m number of messages permitted to be missing without triggering a
 *    failure. This is necessary for some failover tests, where it is
 *    impossible to totally guard against messagt loss. By default, NO
 *    message is permitted to be lost.
 * -T anticipate truncation (which means specified payload length may be
 *    more than actual payload (which may have been truncated)
 * -i increment between messages (default: 1). Can be used for tests which
 *    intentionally leave consistent gaps in the message numbering.
 *
 * Part of the testbench for rsyslog.
 *
 * Copyright 2009-2018 Rainer Gerhards and Adiscon GmbH.
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
#include <string.h>
#if defined(_AIX)
	#include  <unistd.h>
#else
	#include <getopt.h>
#endif

int main(int argc, char *argv[])
{
	FILE *fp;
	int val;
	int i;
	int ret = 0;
	int scanfOK;
	int verbose = 0;
	int bHaveExtraData = 0;
	int bAnticipateTruncation = 0;
	int dupsPermitted = 0;
	int start = 0, end = 0;
	int opt;
	int lostok = 0; /* how many messages are OK to be lost? */
	int nDups = 0;
	int increment = 1;
	int reachedEOF;
	int edLen;	/* length of extra data */
	static char edBuf[500*1024]; /* buffer for extra data (pretty large to be on the save side...) */
	static char ioBuf[sizeof(edBuf)+1024];
	char *file = NULL;

	while((opt = getopt(argc, argv, "i:e:f:ds:vm:ET")) != EOF) {
		switch((char)opt) {
		case 'f':
			file = optarg;
			break;
		case 'd':
			dupsPermitted = 1;
			break;
		case 'i':
			increment = atoi(optarg);
			break;
		case 'e':
			end = atoi(optarg);
			break;
		case 's':
			start = atoi(optarg);
			break;
		case 'v':
			++verbose;
			break;
		case 'm':
			lostok = atoi(optarg);
			break;
		case 'E':
			bHaveExtraData = 1;
			break;
		case 'T':
			bAnticipateTruncation = 1;
			break;
		default:printf("Invalid call of chkseq, optchar='%c'\n", opt);
			printf("Usage: chkseq file -sstart -eend -d -E\n");
			exit(1);
		}
	}

	if(start > end) {
		printf("start must be less than or equal end!\n");
		exit(1);
	}

	if(verbose) {
		printf("chkseq: start %d, end %d\n", start, end);
	}

	/* read file */
	if(file == NULL) {
		fp = stdin;
	} else {
		fp = fopen(file, "r");
	}
	if(fp == NULL) {
		printf("error opening file '%s'\n", file);
		perror(file);
		exit(1);
	}

	for(i = start ; i < end+1 ; i += increment) {
		if(bHaveExtraData) {
			if(fgets(ioBuf, sizeof(ioBuf), fp) == NULL) {
				scanfOK = 0;
			} else {
				scanfOK = sscanf(ioBuf, "%d,%d,%s\n", &val, &edLen, edBuf) == 3 ? 1 : 0;
			}
			if(edLen != (int) strlen(edBuf)) {
				if (bAnticipateTruncation == 1) {
					if (edLen < (int) strlen(edBuf)) {
						 printf("extra data length specified %d, but actually is %ld in"
							" record %d (truncation was anticipated, but payload should"
							" have been smaller than data-length, not larger)\n",
							edLen, (long) strlen(edBuf), i);
						exit(1);
					}
				} else {
					printf("extra data length specified %d, but actually is %ld in record %d\n",
						   edLen, (long) strlen(edBuf), i);
					exit(1);
				}
			}
		} else {
			if(fgets(ioBuf, sizeof(ioBuf), fp) == NULL) {
				scanfOK = 0;
			} else {
				scanfOK = sscanf(ioBuf, "%d\n", &val) == 1 ? 1 : 0;
			}
		}
		if(!scanfOK) {
			printf("scanf error in index i=%d\n", i);
			exit(1);
		}
		while(val > i && lostok > 0) {
			--lostok;
			printf("message %d missing (ok due to -m [now %d])\n", i, lostok);
			++i;
		}
		if(val != i) {
			if(val == i - 1 && dupsPermitted) {
				--i;
				++nDups;
			} else {
				printf("read value %d, but expected value %d\n", val, i);
				exit(1);
			}
		}
	}

	if(i - 1 != end) {
		printf("only %d records in file, expected %d\n", i - 1, end);
		exit(1);
	}

	int c = getc(fp);
	if(c == EOF) {
		reachedEOF = 1;
	} else {
		ungetc(c, fp);
		/* if duplicates are permitted, we need to do a final check if we have duplicates at the
		 * end of file.
		 */
		if(dupsPermitted) {
			i = end;
			while(!feof(fp)) {
				if(bHaveExtraData) {
					if(fgets(ioBuf, sizeof(ioBuf), fp) == NULL) {
						scanfOK = 0;
					} else {
						scanfOK = sscanf(ioBuf, "%d,%d,%s\n", &val,
							&edLen, edBuf) == 3 ? 1 : 0;
					}
					if(edLen != (int) strlen(edBuf)) {
						if (bAnticipateTruncation == 1) {
							if (edLen < (int) strlen(edBuf)) {
								 printf("extra data length specified %d, but "
									"actually is %ld in record %d (truncation was"
									" anticipated, but payload should have been "
									"smaller than data-length, not larger)\n",
									edLen, (long) strlen(edBuf), i);
								exit(1);
							}
						} else {
							 printf("extra data length specified %d, but actually "
								"is %ld in record %d\n",
								edLen, (long) strlen(edBuf), i);
							exit(1);
						}
					}
				} else {
					if(fgets(ioBuf, sizeof(ioBuf), fp) == NULL) {
						scanfOK = 0;
					} else {
						scanfOK = sscanf(ioBuf, "%d\n", &val) == 1 ? 1 : 0;
					}
				}

				if(val != i) {
					reachedEOF = 0;
					goto breakIF;
				}
			}
			reachedEOF = feof(fp) ? 1 : 0;
		} else {
			reachedEOF = 0;
		}
	}

breakIF:
	if(nDups != 0)
		printf("info: had %d duplicates (this is no error)\n", nDups);

	if(!reachedEOF) {
		printf("end of processing, but NOT end of file! First line of extra data is:\n");
		for(c = fgetc(fp) ; c != '\n' && c != EOF ; c = fgetc(fp))
			putchar(c);
		putchar('\n');
		exit(1);
	}

	exit(ret);
}
