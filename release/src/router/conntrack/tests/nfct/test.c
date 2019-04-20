/*
 * (c) 2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * Extremely simple test utility for the command line tools.
 *
 * Based on test-conntrack.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

#define PATH "/usr/sbin"

int main(int argc, char *argv[])
{
	int ret, ok = 0, bad = 0, line;
	FILE *fp;
	DIR *d;
	char buf[1024];
	struct dirent *dent;
	char file[1024];

	if (argc < 2) {
		fprintf(stderr, "Usage: %s directory\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	d = opendir(argv[1]);
	if (d == NULL) {
		perror("opendir");
		exit(EXIT_FAILURE);
	}

	setenv("PATH", PATH, 1);

	while ((dent = readdir(d)) != NULL) {

		sprintf(file, "%s/%s", argv[1], dent->d_name);

		line = 0;

		fp = fopen(file, "r");
		if (fp == NULL) {
			perror("cannot find testsuite file");
			exit(EXIT_FAILURE);
		}

		while (fgets(buf, sizeof(buf), fp)) {
			char *res;

			line++;

			if (buf[0] == '#' || buf[0] == ' ')
				continue;

			res = strchr(buf, ';');
			if (!res) {
				printf("malformed file %s at line %d\n", 
					dent->d_name, line);
				exit(EXIT_FAILURE);
			}
			*res = '\0';
			res+=2;

			printf("(%d) Executing: %s\n", line, buf);

			ret = system(buf);

			if (WIFEXITED(ret) &&
			    WEXITSTATUS(ret) == EXIT_SUCCESS) {
			    	if (res[0] == 'O' &&
				    res[1] == 'K')
					ok++;
				else {
					bad++;
					printf("^----- BAD\n");
				}
			} else {
				if (res[0] == 'B' &&
				    res[1] == 'A' &&
				    res[2] == 'D')
					ok++;
				else {
					bad++;
					printf("^----- BAD\n");
				}
			}
			printf("=====\n");
		}
		fclose(fp);
	}
	closedir(d);

	fprintf(stdout, "OK: %d BAD: %d\n", ok, bad);
}
