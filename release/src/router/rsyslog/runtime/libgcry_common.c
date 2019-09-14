/* libgcry_common.c
 * This file hosts functions both being used by the rsyslog runtime as
 * well as tools who do not use the runtime (so we can maintain the
 * code at a single place).
 *
 * Copyright 2013 Adiscon GmbH.
 *
 * This file is part of rsyslog.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *       -or-
 *       see COPYING.ASL20 in the source distribution
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#if HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <gcrypt.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "rsyslog.h" /* we need data typedefs */
#include "libgcry.h"


/* read a key from a key file
 * @param[out] key - key buffer, must be freed by caller
 * @param[out] keylen - length of buffer
 * @returns 0 if OK, something else otherwise (we do not use
 *            iRet as this is also called from non-rsyslog w/o runtime)
 *	      on error, errno is set and can be queried
 * The key length is limited to 64KiB to prevent DoS.
 * Note well: key is a blob, not a C string (NUL may be present!)
 */
int
gcryGetKeyFromFile(const char *const fn, char **const key, unsigned *const keylen)
{
	struct stat sb;
	int r = -1;

	const int fd = open(fn, O_RDONLY);
	if(fd < 0) goto done;
	if(fstat(fd, &sb) == -1) goto done;
	if(sb.st_size > 64*1024) {
		errno = EMSGSIZE;
		goto done;
	}
	if((*key = malloc(sb.st_size)) == NULL) goto done;
	if(read(fd, *key, sb.st_size) != sb.st_size) goto done;
	*keylen = sb.st_size;
	r = 0;
done:
	if(fd >= 0) {
		close(fd);
	}
	return r;
}


/* execute the child process (must be called in child context
 * after fork).
 */

static void
execKeyScript(char *cmd, int pipefd[])
{
	char *newargv[] = { NULL };
	char *newenviron[] = { NULL };

	dup2(pipefd[0], STDIN_FILENO);
	dup2(pipefd[1], STDOUT_FILENO);

	/* finally exec child */
fprintf(stderr, "pre execve: %s\n", cmd);
	execve(cmd, newargv, newenviron);
	/* switch to?
	execlp((char*)program, (char*) program, (char*)arg, NULL);
	*/

	/* we should never reach this point, but if we do, we terminate */
	return;
}


static int
openPipe(char *cmd, int *fd)
{
	int pipefd[2];
	pid_t cpid;
	int r;

	if(pipe(pipefd) == -1) {
		r = 1; goto done;
	}

	cpid = fork();
	if(cpid == -1) {
		r = 1; goto done;
	}

	if(cpid == 0) {
		/* we are the child */
		execKeyScript(cmd, pipefd);
		exit(1);
	}

	close(pipefd[1]);
	*fd = pipefd[0];
	r = 0;
done:	return r;
}


/* Read a character from the program's output. */
// TODO: highly unoptimized version, should be used in buffered
// mode
static int
readProgChar(int fd, char *c)
{
	int r;
	if(read(fd, c, 1) != 1) {
		r = 1; goto done;
	}
	r = 0;
done:	return r;
}

/* Read a line from the script. Line is terminated by LF, which
 * is NOT put into the buffer.
 * buf must be 64KiB
 */
static int
readProgLine(int fd, char *buf)
{
	char c;
	int r;
	unsigned i;
	
	for(i = 0 ; i < 64*1024 ; ++i) {
		if((r = readProgChar(fd, &c)) != 0) goto done;
		if(c == '\n')
			break;
		buf[i] = c;
	};
	if(i >= 64*1024) {
		r = 1; goto done;
	}
	buf[i] = '\0';
	r = 0;
done:	return r;
}
static int
readProgKey(int fd, char *buf, unsigned keylen)
{
	char c;
	int r;
	unsigned i;
	
	for(i = 0 ; i < keylen ; ++i) {
		if((r = readProgChar(fd, &c)) != 0) goto done;
		buf[i] = c;
	};
	r = 0;
done:	return r;
}

int
gcryGetKeyFromProg(char *cmd, char **key, unsigned *keylen)
{
	int r;
	int fd;
	char rcvBuf[64*1024];

	if((r = openPipe(cmd, &fd)) != 0) goto done;
	if((r = readProgLine(fd, rcvBuf)) != 0) goto done;
	if(strcmp(rcvBuf, "RSYSLOG-KEY-PROVIDER:0")) {
		r = 2; goto done;
	}
	if((r = readProgLine(fd, rcvBuf)) != 0) goto done;
	*keylen = atoi(rcvBuf);
	if((*key = malloc(*keylen)) == NULL) {
		r = -1; goto done;
	}
	if((r = readProgKey(fd, *key, *keylen)) != 0) goto done;
done:	return r;
}
