/*
 *  Advanced Linux Sound Architecture Control Program - Support routines
 *  Copyright (c) by Jaroslav Kysela <perex@perex.cz>
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <alsa/asoundlib.h>
#include "alsactl.h"

int file_map(const char *filename, char **buf, size_t *bufsize)
{
	struct stat stats;
	int fd;

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		return -1;
	}

	if (fstat(fd, &stats) < 0) {
		close(fd);
		return -1;
	}

	*buf = mmap(NULL, stats.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (*buf == MAP_FAILED) {
		close(fd);
		return -1;
	}
	*bufsize = stats.st_size;

	close(fd);

	return 0;
}

void file_unmap(void *buf, size_t bufsize)
{
	munmap(buf, bufsize);
}

size_t line_width(const char *buf, size_t bufsize, size_t pos)
{
	int esc = 0;
	size_t count;
	
	for (count = pos; count < bufsize; count++) {
		if (!esc && buf[count] == '\n')
			break;
		esc = buf[count] == '\\';
	}

	return count - pos;
}

void initfailed(int cardnumber, const char *reason, int exitcode)
{
	int fp;
	char *str;
	char sexitcode[16];

	if (statefile == NULL)
		return;
	if (snd_card_get_name(cardnumber, &str) < 0)
		return;
	sprintf(sexitcode, "%i", exitcode);
	fp = open(statefile, O_WRONLY|O_CREAT|O_APPEND, 0644);
	write(fp, str, strlen(str));
	write(fp, ":", 1);
	write(fp, reason, strlen(reason));
	write(fp, ":", 1);
	write(fp, sexitcode, strlen(sexitcode));
	write(fp, "\n", 1);
	close(fp);
	free(str);
}
