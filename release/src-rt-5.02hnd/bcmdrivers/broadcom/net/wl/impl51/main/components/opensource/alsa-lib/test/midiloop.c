#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/time.h>
#include "../include/asoundlib.h"
#include <string.h>
#include <signal.h>

static void usage(void)
{
	fprintf(stderr, "Usage: midiloop [options]\n");
	fprintf(stderr, "  options:\n");
	fprintf(stderr, "    -v: verbose mode\n");
	fprintf(stderr, "    -i <rawmidi device> : test input device\n");
	fprintf(stderr, "    -o <rawmidi device> : test output device\n");
}

int stop = 0;

void sighandler(int dummy ATTRIBUTE_UNUSED)
{
	stop=1;
}

long long timediff(struct timeval t1, struct timeval t2)
{
	signed long l;

	t1.tv_sec -= t2.tv_sec;
	l = (signed long) t1.tv_usec - (signed long) t2.tv_usec;
	if (l < 0) {
		t1.tv_sec--;
		l = -l;
		l %= 1000000;
	}
	return ((long long)t1.tv_sec * (long long)1000000) + (long long)l;
}

int writepattern(snd_rawmidi_t *handle_out, unsigned char *obuf)
{
	int patsize, i;

	patsize = 0;
	for (i = 0; i < 15; i++) {
		obuf[patsize++] = 0x90 + i;
		obuf[patsize++] = 0x40;
		obuf[patsize++] = 0x3f;
		obuf[patsize++] = 0xb0 + i;
		obuf[patsize++] = 0x2e;
		obuf[patsize++] = 0x7a;
		obuf[patsize++] = 0x80 + i;
		obuf[patsize++] = 0x23;
		obuf[patsize++] = 0x24;
		obuf[patsize++] = 0xf0;
		obuf[patsize++] = i;
		obuf[patsize++] = 0xf7;
	}
	i = snd_rawmidi_write(handle_out, obuf, patsize);
	if (i != patsize) {
		printf("Written only %i bytes from %i bytes\n", i, patsize);
		exit(EXIT_FAILURE);
	}
	return patsize;
}

int main(int argc, char** argv)
{
	int i, j, k, opos, ipos, patsize;
	int err;
	int verbose = 0;
	snd_rawmidi_t *handle_in = NULL, *handle_out = NULL;
	unsigned char ibuf[512], obuf[512];
	char *iname = "hw:0,0", *oname = "hw:0,0";
	struct timeval start, end;
	long long diff;
	snd_rawmidi_status_t *istat, *ostat;
	
	for (i = 1 ; i<argc ; i++) {
		if (argv[i][0]=='-') {
			if (!strcmp(argv[i], "--help")) {
				usage();
				return 0;
			}
			switch (argv[i][1]) {
				case 'h':
					usage();
					return 0;
				case 'v':
					verbose = 1;
					break;
				case 'i':
					if (i + 1 < argc)
						iname = argv[++i];
					break;
				case 'o':
					if (i + 1 < argc)
						oname = argv[++i];
					break;
			}			
		}
	}

	if (iname == NULL)
		iname = oname;
	if (oname == NULL)
		oname = iname;

	if (verbose) {
		fprintf(stderr, "Using: \n");
		fprintf(stderr, "  Input: %s  Output: %s\n", iname, oname);
	}
	
	err = snd_rawmidi_open(&handle_in, NULL, iname, SND_RAWMIDI_NONBLOCK);
	if (err) {
		fprintf(stderr,"snd_rawmidi_open %s failed: %d\n",iname,err);
		exit(EXIT_FAILURE);
	}

	err = snd_rawmidi_open(NULL, &handle_out, oname, 0);
	if (err) {
		fprintf(stderr,"snd_rawmidi_open %s failed: %d\n",oname,err);
		exit(EXIT_FAILURE);
	}

	signal(SIGINT, sighandler);

	i = snd_rawmidi_read(handle_in, ibuf, sizeof(ibuf));
	if (i > 0) {
		printf("Read ahead: %i\n", i);
		for (j = 0; j < i; j++)
			printf("%02x:", ibuf[j]);
		printf("\n");
		exit(EXIT_FAILURE);
	}

	snd_rawmidi_nonblock(handle_in, 0);

	patsize = writepattern(handle_out, obuf);
	gettimeofday(&start, NULL);
	patsize = writepattern(handle_out, obuf);

	k = ipos = opos = err = 0;
	while (!stop) {
		i = snd_rawmidi_read(handle_in, ibuf, sizeof(ibuf));
		for (j = 0; j < i; j++, ipos++)
			if (obuf[k] != ibuf[j]) {
				printf("ipos = %i, i[0x%x] != o[0x%x]\n", ipos, ibuf[j], obuf[k]);
				if (opos > 0)
					stop = 1;
			} else {
				printf("match success: ipos = %i, opos = %i [%i:0x%x]\n", ipos, opos, k, obuf[k]);
				k++; opos++;
				if (k >= patsize) {
					patsize = writepattern(handle_out, obuf);
					k = 0;
				}
			}
	}

	gettimeofday(&end, NULL);

	printf("End...\n");

	snd_rawmidi_status_alloca(&istat);
	snd_rawmidi_status_alloca(&ostat);
	err = snd_rawmidi_status(handle_in, istat);
	if (err < 0)
		fprintf(stderr, "input stream status error: %d\n", err);
	err = snd_rawmidi_status(handle_out, ostat);
	if (err < 0)
		fprintf(stderr, "output stream status error: %d\n", err);
	printf("input.status.avail = %i\n", snd_rawmidi_status_get_avail(istat));
	printf("input.status.xruns = %i\n", snd_rawmidi_status_get_xruns(istat));
	printf("output.status.avail = %i\n", snd_rawmidi_status_get_avail(ostat));
	printf("output.status.xruns = %i\n", snd_rawmidi_status_get_xruns(ostat));

	diff = timediff(end, start);
	printf("Time diff: %Liusec (%Li bytes/sec)\n", diff, ((long long)opos * 1000000) / diff);

	if (verbose) {
		fprintf(stderr,"Closing\n");
	}
	
	snd_rawmidi_drain(handle_in); 
	snd_rawmidi_close(handle_in);	
	snd_rawmidi_drain(handle_out); 
	snd_rawmidi_close(handle_out);	

	return 0;
}
