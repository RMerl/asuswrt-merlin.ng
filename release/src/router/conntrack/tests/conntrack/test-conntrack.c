/* 
 * Very simple test-tool for the command line tool `conntrack'.
 * This code is released under GPLv2 or any later at your option.
 *
 * gcc test-conntrack.c -o test
 *
 * Do not forget that you need *root* or CAP_NET_ADMIN capabilities ;-)
 *
 * (c) 2008 Pablo Neira Ayuso <pablo@netfilter.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

#define CT_PROG "../../src/conntrack"

int main()
{
	int ret, ok = 0, bad = 0, line;
	FILE *fp;
	char buf[1024];
	struct dirent **dents;
	struct dirent *dent;
	char file[1024];
	int i,n;

	n = scandir("testsuite", &dents, NULL, alphasort);

	for (i = 0; i < n; i++) {
		dent = dents[i];

		if (dent->d_name[0] == '.')
			continue;

		sprintf(file, "testsuite/%s", dent->d_name);

		line = 0;

		fp = fopen(file, "r");
		if (fp == NULL) {
			perror("cannot find testsuite file");
			exit(EXIT_FAILURE);
		}

		while (fgets(buf, sizeof(buf), fp)) {
			char tmp[1024] = CT_PROG, *res;
			tmp[strlen(CT_PROG)] = ' ';

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

			strcpy(tmp + strlen(CT_PROG) + 1, buf);
			printf("(%d) Executing: %s\n", line, tmp);

			fflush(stdout);
			ret = system(tmp);

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

	for (i = 0; i < n; i++)
		free(dents[i]);

	free(dents);

	fprintf(stdout, "OK: %d BAD: %d\n", ok, bad);
}
