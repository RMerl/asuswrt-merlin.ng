#include "../include/asoundlib.h"
#include <err.h>

int main(int argc, char *argv[])
{
	const char *iface = "pcm";
	char **hints, **n;
	int err;

	if (argc > 1)
		iface = argv[1];
	err = snd_device_name_hint(-1, iface, &hints);
	if (err < 0)
		errx(1, "snd_device_name_hint error: %s", snd_strerror(err));
	n = hints;
	while (*n != NULL) {
		printf("%s\n", *n);
		n++;
	}
	snd_device_name_free_hint(hints);
	return 0;
}
