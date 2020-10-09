/*
 * parse_qtype.c
 */

#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include "quotaio.h"

#define PARSE_DELIM ":,"

int parse_quota_types(const char *in_str, unsigned int *qtype_bits,
		      char **err_token)
{
	char	*buf, *token, *next, *tmp;
	unsigned int qtype = *qtype_bits;
	int	len, ret = 0;

	if (!in_str)
		return 0;

	len = strlen(in_str);
	buf = malloc(len + 1);
	if (!buf)
		return ENOMEM;
	strcpy(buf, in_str);

	for (token = buf, next = strtok_r(buf, PARSE_DELIM, &tmp);
	     token && *token; token = next) {
		int	not = 0;
		char	*p = token;

		if (*p == '^') {
			not = 1;
			p++;
		}
		if (!strcmp(p, "usr") || !strcmp(p, "usrquota")) {
			if (not)
				qtype &= ~QUOTA_USR_BIT;
			else
				qtype |= QUOTA_USR_BIT;
		} else if (!strcmp(p, "grp") || !strcmp(p, "grpquota")) {
			if (not)
				qtype &= ~QUOTA_GRP_BIT;
			else
				qtype |= QUOTA_GRP_BIT;
		} else if (!strcmp(p, "prj") || !strcmp(p, "prjquota")) {
			if (not)
				qtype &= ~QUOTA_PRJ_BIT;
			else
				qtype |= QUOTA_PRJ_BIT;
		} else {
			if (err_token) {
				*err_token = malloc(strlen(token) + 1);
				if (*err_token)
					strcpy(*err_token, token);
			}
			ret = EINVAL;
			goto errout;
		}
#ifdef DEBUG_PROGRAM
		printf("word: %s\n", token);
#endif
		next = strtok_r(NULL, PARSE_DELIM, &tmp);
	}
	*qtype_bits = qtype;
errout:
	free(buf);
	return ret;
}

#ifdef DEBUG_PROGRAM
int main(int argc, char **argv)
{
	unsigned int qtype_bits = 0;
	int ret;
	char *err_token = 0;

	ret = parse_quota_types(argv[1], &qtype_bits, &err_token);
	printf("parse_quota_types returns %d, %d\n", ret, qtype_bits);
	if (err_token)
		printf("err_token is %s\n", err_token);
	return 0;
}
#endif
