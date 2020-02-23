/* simple test for index to interface name API */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <libnfnetlink/libnfnetlink.h>

int main()
{
	int i;
	struct nlif_handle *h;

	h = nlif_open();
	if (h == NULL) {
		perror("nlif_open");
		exit(EXIT_FAILURE);
	}

	nlif_query(h);

	for (i=0; i<64; i++) {
		char name[IFNAMSIZ];

		if (nlif_index2name(h, i, name) == -1)
			continue;
		printf("index (%d) is %s\n", i, name);
	}

	nlif_close(h);
}
