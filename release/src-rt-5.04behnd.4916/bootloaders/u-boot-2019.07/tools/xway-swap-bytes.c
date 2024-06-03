#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef BUFSIZ
# define BUFSIZ 4096
#endif

#undef BUFSIZ
# define BUFSIZ 64
int main (void)
{
	short ibuff[BUFSIZ], obuff[BUFSIZ];
	int rc, i, len;

	while ((rc = read (0, ibuff, sizeof (ibuff))) > 0) {
		memset (obuff, 0, sizeof (obuff));
		for (i = 0; i < (rc + 1) / 2; i++) {
			obuff[i] = ibuff[i ^ 1];
		}

		len = (rc + 1) & ~1;

		if (write (1, obuff, len) != len) {
			perror ("read error");
			return (EXIT_FAILURE);
		}

		memset (ibuff, 0, sizeof (ibuff));
	}

	if (rc < 0) {
		perror ("read error");
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}
