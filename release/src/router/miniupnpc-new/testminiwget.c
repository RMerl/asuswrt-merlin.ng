/* $Id: testminiwget.c,v 1.7 2018/01/16 01:01:05 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * Project : miniupnp
 * Author : Thomas Bernard
 * Copyright (c) 2005-2018 Thomas Bernard
 * This software is subject to the conditions detailed in the
 * LICENCE file provided in this distribution.
 * */
#include <stdio.h>
#include <stdlib.h>
#include "miniwget.h"

/**
 * This program uses the miniwget / miniwget_getaddr function
 * from miniwget.c in order to retrieve a web ressource using
 * a GET HTTP method, and store it in a file.
 */
int main(int argc, char * * argv)
{
	void * data;
	int size, writtensize;
	FILE *f;
	char addr[64];
	int status_code = -1;

	if(argc < 3) {
		fprintf(stderr, "Usage:\t%s url file\n", argv[0]);
		fprintf(stderr, "Example:\t%s http://www.google.com/ out.html\n", argv[0]);
		return 1;
	}
	data = miniwget_getaddr(argv[1], &size, addr, sizeof(addr), 0, &status_code);
	if(!data || (status_code != 200)) {
		if(data) free(data);
		fprintf(stderr, "Error %d fetching %s\n", status_code, argv[1]);
		return 1;
	}
	printf("local address : %s\n", addr);
	printf("got %d bytes\n", size);
	f = fopen(argv[2], "wb");
	if(!f) {
		fprintf(stderr, "Cannot open file %s for writing\n", argv[2]);
		free(data);
		return 1;
	}
	writtensize = fwrite(data, 1, size, f);
	if(writtensize != size) {
		fprintf(stderr, "Could only write %d bytes out of %d to %s\n",
		        writtensize, size, argv[2]);
	} else {
		printf("%d bytes written to %s\n", writtensize, argv[2]);
	}
	fclose(f);
	free(data);
	return 0;
}

