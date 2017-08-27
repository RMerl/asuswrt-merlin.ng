/*
 *   MIDI file player for ALSA sequencer 
 *   (type 0 only!, the library that is used doesn't support merging of tracks)
 *
 *   Copyright (c) 1998 by Frank van de Pol <F.K.W.van.de.Pol@inter.nl.net>
 *
 *   Modified so that this uses alsa-lib
 *   1999 Jan. by Isaku Yamahata <yamahata@kusm.kyoto-u.ac.jp>
 *
 *   19990604	Takashi Iwai <iwai@ww.uni-erlangen.de>
 *	- use blocking mode
 *	- fix tempo event bug
 *	- add command line options
 *
 *   19990827	Takashi Iwai <iwai@ww.uni-erlangen.de>
 *	- use snd_seq_alloc_queue()
 *
 *   19990916	Takashi Iwai <iwai@ww.uni-erlangen.de>
 *	- use middle-level sequencer routines and macros
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

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "midifile.h"		/* SMF library header */
#include "midifile.c"		/* SMF library code */

#include "../include/asoundlib.h"

/* send the real-time time stamps (instead of midi ticks) to the ALSA sequencer */
static int use_realtime = 0;

/* control the event buffering by using a blocking mode */
static int use_blocking_mode = 1;

/* default destination queue, client and port numbers */
#define DEST_CLIENT_NUMBER	65
#define DEST_PORT_NUMBER	0

/* event pool size */
#define WRITE_POOL_SIZE		200
#define WRITE_POOL_SPACE	10
#define READ_POOL_SIZE		10	/* we need to read the pool only for echoing */

static FILE *F;
static snd_seq_t *seq_handle = NULL;
static int ppq = 96;
static int slave_ppq = 96;

static double local_secs = 0;
static int local_ticks = 0;
static int local_tempo = 500000;

static int dest_queue = -1;
static int shared_queue = 0;
static int tick_offset = 0;
static int dest_client = DEST_CLIENT_NUMBER;
static int dest_port = DEST_PORT_NUMBER;
static int my_port = 0;

static int verbose = 0;
static int slave   = 0;		/* allow external sync */

#define VERB_INFO	1
#define VERB_MUCH	2
#define VERB_EVENT	3

static void alsa_start_timer(void);
static void alsa_stop_timer(void);
static void wait_start(void);


static inline double tick2time_dbl(int tick)
{
	return local_secs + ((double) (tick - local_ticks) * (double) local_tempo * 1.0E-6 / (double) ppq);
}

static void tick2time(snd_seq_real_time_t * tm, int tick)
{
	double secs = tick2time_dbl(tick);
	tm->tv_sec = secs;
	tm->tv_nsec = (secs - tm->tv_sec) * 1.0E9;
}

static void write_ev(snd_seq_event_t *ev)
{
	int rc;

	if (use_blocking_mode) {
		rc = snd_seq_event_output(seq_handle, ev);
		if (rc < 0) {
			printf("written = %i (%s)\n", rc, snd_strerror(rc));
			exit(1);
		}
		return;
	}
	while ((rc = snd_seq_event_output(seq_handle, ev)) < 0) {
		int npfds = snd_seq_poll_descriptors_count(seq_handle, POLLOUT);
		struct pollfd *pfds = alloca(sizeof(*pfds) * npfds);
		snd_seq_poll_descriptors(seq_handle, pfds, npfds, POLLOUT);
		if ((rc = poll(pfds, npfds, -1)) < 0) {
			printf("poll error = %i (%s)\n", rc, snd_strerror(errno));
			exit(1);
		}
	}
}

/* read the byte */
static int mygetc(void)
{
	return getc(F);
}

/* print out the text */
static void mytext(int type ATTRIBUTE_UNUSED, int leng, char *msg)
{
	char *p;
	char *ep = msg + leng;

	if (verbose >= VERB_INFO) {
		for (p = msg; p < ep; p++)
			putchar(isprint(*p) ? *p : '?');
		putchar('\n');
	}
}

static void do_header(int format, int ntracks, int division)
{
	snd_seq_queue_tempo_t *tempo;

	if (verbose >= VERB_INFO)
		printf("smf format %d, %d tracks, %d ppq\n", format, ntracks, division);
	ppq = division;

	if (format != 0 || ntracks != 1) {
		printf("This player does not support merging of tracks.\n");
		if (! shared_queue)
			alsa_stop_timer();
		exit(1);
	}
	/* set the ppq */
	snd_seq_queue_tempo_alloca(&tempo);
	/* ppq must be set before starting the timer */
	if (snd_seq_get_queue_tempo(seq_handle, dest_queue, tempo) < 0) {
    		perror("get_queue_tempo");
    		exit(1);
	}
	if ((slave_ppq = snd_seq_queue_tempo_get_ppq(tempo)) != ppq) {
		snd_seq_queue_tempo_set_ppq(tempo, ppq);
		if (snd_seq_set_queue_tempo(seq_handle, dest_queue, tempo) < 0) {
    			perror("set_queue_tempo");
    			if (!slave && !shared_queue)
    				exit(1);
			else
				printf("different PPQ %d in SMF from queue PPQ %d\n", ppq, slave_ppq);
		} else
			slave_ppq = ppq;
		if (verbose >= VERB_INFO)
			printf("ALSA Timer updated, PPQ = %d\n", snd_seq_queue_tempo_get_ppq(tempo));
	}

	/* start playing... */
	if (slave) {
		if (verbose >= VERB_INFO)
			printf("Wait till timer starts...\n");	
		wait_start();
		if (verbose >= VERB_INFO)
			printf("Go!\n");	
	} else if (shared_queue) {
		snd_seq_queue_status_t *stat;
		snd_seq_queue_status_alloca(&stat);
		snd_seq_get_queue_status(seq_handle, dest_queue, stat);
		tick_offset = snd_seq_queue_status_get_tick_time(stat);
		fprintf(stderr, "tick offset = %d\n", tick_offset);
	} else {
		alsa_start_timer();
		tick_offset = 0;
	}
}

/* fill the event time */
static void set_event_time(snd_seq_event_t *ev, unsigned int currtime)
{
	if (use_realtime) {
		snd_seq_real_time_t rtime;
		if (ppq != slave_ppq)
			currtime = (currtime * slave_ppq) / ppq;
		tick2time(&rtime, currtime);
		snd_seq_ev_schedule_real(ev, dest_queue, 0, &rtime);
	} else {
		if (ppq != slave_ppq)
			currtime = (currtime * slave_ppq) / ppq;
		currtime += tick_offset;
		snd_seq_ev_schedule_tick(ev, dest_queue, 0, currtime);
	}
}

/* fill the normal event header */
static void set_event_header(snd_seq_event_t *ev)
{
	snd_seq_ev_clear(ev);
	snd_seq_ev_set_dest(ev, dest_client, dest_port);
	snd_seq_ev_set_source(ev, my_port);
	set_event_time(ev, Mf_currtime);
}

/* start the timer */
static void alsa_start_timer(void)
{
	snd_seq_start_queue(seq_handle, dest_queue, NULL);
}

/* stop the timer */
static void alsa_stop_timer(void)
{
	snd_seq_event_t ev;
	set_event_header(&ev);
	snd_seq_stop_queue(seq_handle, dest_queue, &ev);
}

/* change the tempo */
static void do_tempo(int us)
{
	snd_seq_event_t ev;

	if (verbose >= VERB_MUCH) {
		double bpm;
		bpm = 60.0E6 / (double) us;
		printf("Tempo %d us/beat, %.2f bpm\n", us, bpm);
	}

	/* store the new tempo and timestamp of the tempo change */
	local_secs = tick2time_dbl(Mf_currtime);
	local_ticks = Mf_currtime;
	local_tempo = us;

	set_event_header(&ev);
	if (!slave)
		snd_seq_change_queue_tempo(seq_handle, dest_queue, us, &ev);
}

static void do_noteon(int chan, int pitch, int vol)
{
	snd_seq_event_t ev;

	if (verbose >= VERB_EVENT)
		printf("%ld: NoteOn (%d) %d %d\n", Mf_currtime, chan, pitch, vol);
	set_event_header(&ev);
	snd_seq_ev_set_noteon(&ev, chan, pitch, vol);
	write_ev(&ev);
}


static void do_noteoff(int chan, int pitch, int vol)
{
	snd_seq_event_t ev;

	if (verbose >= VERB_EVENT)
		printf("%ld: NoteOff (%d) %d %d\n", Mf_currtime, chan, pitch, vol);
	set_event_header(&ev);
	snd_seq_ev_set_noteoff(&ev, chan, pitch, vol);
	write_ev(&ev);
}


static void do_program(int chan, int program)
{
	snd_seq_event_t ev;

	if (verbose >= VERB_EVENT)
		printf("%ld: Program (%d) %d\n", Mf_currtime, chan, program);
	set_event_header(&ev);
	snd_seq_ev_set_pgmchange(&ev, chan, program);
	write_ev(&ev);
}


static void do_parameter(int chan, int control, int value)
{
	snd_seq_event_t ev;

	if (verbose >= VERB_EVENT)
		printf("%ld: Control (%d) %d %d\n", Mf_currtime, chan, control, value);
	set_event_header(&ev);
	snd_seq_ev_set_controller(&ev, chan, control, value);
	write_ev(&ev);
}


static void do_pitchbend(int chan, int lsb, int msb)
{	/* !@#$% lsb & msb are in the wrong order in docs */
	snd_seq_event_t ev;

	if (verbose >= VERB_EVENT)
		printf("%ld: Pitchbend (%d) %d %d\n", Mf_currtime, chan, lsb, msb);
	set_event_header(&ev);
	snd_seq_ev_set_pitchbend(&ev, chan, (lsb + (msb << 7)) - 8192);
	write_ev(&ev);
}

static void do_pressure(int chan, int pitch, int pressure)
{
	snd_seq_event_t ev;

	if (verbose >= VERB_EVENT)
		printf("%ld: KeyPress (%d) %d %d\n", Mf_currtime, chan, pitch, pressure);
	set_event_header(&ev);
	snd_seq_ev_set_keypress(&ev, chan, pitch, pressure);
	write_ev(&ev);
}

static void do_chanpressure(int chan, int pressure)
{
	snd_seq_event_t ev;

	if (verbose >= VERB_EVENT)
		printf("%ld: ChanPress (%d) %d\n", Mf_currtime, chan, pressure);
	set_event_header(&ev);
	snd_seq_ev_set_chanpress(&ev, chan, pressure);
	write_ev(&ev);
}

static void do_sysex(int len, char *msg)
{
	snd_seq_event_t ev;

	if (verbose >= VERB_MUCH) {
		int c;
		printf("%ld: Sysex, len=%d\n", Mf_currtime, len);
		for (c = 0; c < len; c++) {
			printf(" %02x", (unsigned char)msg[c]);
			if (c % 16 == 15)
				putchar('\n');
		}
		if (c % 16 != 15)
			putchar('\n');
	}

	set_event_header(&ev);
	snd_seq_ev_set_sysex(&ev, len, msg);
	write_ev(&ev);
}

static snd_seq_event_t *wait_for_event(void)
{
	int left;
	snd_seq_event_t *input_event;
  
	if (use_blocking_mode) {
		/* read the event - blocked until any event is read */
		left = snd_seq_event_input(seq_handle, &input_event);
	} else {
		/* read the event - using select syscall */
		while ((left = snd_seq_event_input(seq_handle, &input_event)) >= 0 &&
		       input_event == NULL) {
			int npfds = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
			struct pollfd *pfds = alloca(sizeof(*pfds) * npfds);
			snd_seq_poll_descriptors(seq_handle, pfds, npfds, POLLIN);
			if ((left = poll(pfds, npfds, -1)) < 0) {
				printf("poll error = %i (%s)\n", errno, snd_strerror(errno));
				exit(1);
			}
		}
	}

	if (left < 0) {
		printf("alsa_sync error!:%s\n", snd_strerror(left));
		return NULL;
	}

	return input_event;
}

/* synchronize to the end of the event */
static void alsa_sync(void)
{
	/* send the echo event to the self client. */
	if (verbose >= VERB_MUCH)
		printf("alsa_sync syncing...\n");
	/* dump the buffer */
	snd_seq_drain_output(seq_handle);
	snd_seq_sync_output_queue(seq_handle);
	if (verbose >= VERB_MUCH)
		printf("alsa_sync synced\n");
	sleep(1); /* give a time for note releasing.. */
}


/* wait for the start of the queue */
static void wait_start(void)
{
	snd_seq_event_t *input_event;

	/* wait for the start event from the system timer */
	for (;;) {
		input_event = wait_for_event();
		if (input_event) {
			if (verbose >= VERB_MUCH)
				printf("wait_start got event. type=%d, flags=%d\n",
				       input_event->type, input_event->flags);
			if (input_event->type == SND_SEQ_EVENT_START &&
			    input_event->data.queue.queue == dest_queue) {
				snd_seq_free_event(input_event);
				break;
			}
			snd_seq_free_event(input_event);
		}
	}
	if (verbose >= VERB_MUCH)
		printf("start received\n");
}


/* print the usage */
static void usage(void)
{
	fprintf(stderr, "usage: playmidi1 [options] [file]\n");
	fprintf(stderr, "  options:\n");
	fprintf(stderr, "  -v: verbose mode\n");
	fprintf(stderr, "  -a client:port : set destination address (default=%d:%d)\n",
		DEST_CLIENT_NUMBER, DEST_PORT_NUMBER);
	fprintf(stderr, "  -q queue: use the specified queue\n");
	fprintf(stderr, "  -s queue: slave mode (allow external clock synchronization)\n");
	fprintf(stderr, "  -r : play on real-time mode\n");
	fprintf(stderr, "  -b : play on non-blocking mode\n");
}

int main(int argc, char *argv[])
{
	int tmp;
	int c;
	snd_seq_addr_t dest_addr;
	const char *addr = "65:0";

	while ((c = getopt(argc, argv, "s:a:p:q:vrb")) != -1) {
		switch (c) {
		case 'v':
			verbose++;
			break;
		case 'a':
		case 'p':
			addr = optarg;
			break;
		case 'q':
			dest_queue = atoi(optarg);
			if (dest_queue < 0) {
				fprintf(stderr, "invalid queue number %d\n", dest_queue);
				exit(1);
			}
			break;
		case 's':
			slave = 1;
			dest_queue = atoi(optarg);
			if (dest_queue < 0) {
				fprintf(stderr, "invalid queue number %d\n", dest_queue);
				exit(1);
			}
			break;
		case 'r':
			use_realtime = 1;
			break;
		case 'b':
			use_blocking_mode = 0;
			break;
		default:
			usage();
			exit(1);
		}
	}

	if (verbose >= VERB_INFO) {
		if (use_realtime)
			printf("ALSA MIDI Player, feeding events to real-time queue\n");
		else
			printf("ALSA MIDI Player, feeding events to song queue\n");
	}

	/* open the sequencer device */
	/* Here we open the device in read/write for slave mode. */
	tmp = snd_seq_open(&seq_handle, "hw", slave ? SND_SEQ_OPEN_DUPLEX : SND_SEQ_OPEN_OUTPUT, 0);
	if (tmp < 0) {
		perror("open /dev/snd/seq");
		exit(1);
	}
	
	tmp = snd_seq_nonblock(seq_handle, !use_blocking_mode);
	if (tmp < 0) {
		perror("block_mode");
		exit(1);
	}
			
	/* set the name */
	/* set the event filter to receive only the echo event */
	/* if running in slave mode, also listen for a START event */
	if (slave)
		snd_seq_set_client_event_filter(seq_handle, SND_SEQ_EVENT_START);
	snd_seq_set_client_name(seq_handle, "MIDI file player");

	/* create the port */
	my_port = snd_seq_create_simple_port(seq_handle, "Port 0",
					     SND_SEQ_PORT_CAP_WRITE |
					     SND_SEQ_PORT_CAP_READ,
					     SND_SEQ_PORT_TYPE_MIDI_GENERIC);
	if (my_port < 0) {
		perror("create port");
		exit(1);
	}
	
	if (snd_seq_parse_address(seq_handle, &dest_addr, addr) < 0) {
		perror("invalid destination address");
		exit(1);
	}
	dest_client = dest_addr.client;
	dest_port = dest_addr.port;

	/* set the queue */
	if (dest_queue >= 0) {
		shared_queue = 1;
		if (snd_seq_set_queue_usage(seq_handle, dest_queue, 1) < 0) {
			perror("use queue");
			exit(1);
		}
	} else {
		shared_queue = 0;
		dest_queue = snd_seq_alloc_queue(seq_handle);
		if (dest_queue < 0) {
			perror("alloc queue");
			exit(1);
		}
	}

	/* set the subscriber */
	tmp = snd_seq_connect_to(seq_handle, my_port, dest_client, dest_port);
	if (tmp < 0) {
		perror("subscribe");
		exit(1);
	}

	/* subscribe for the timer START event */	
	if (slave) {	
		tmp = snd_seq_connect_from(seq_handle, my_port,
					   SND_SEQ_CLIENT_SYSTEM,
					   dest_queue + 16 /*snd_seq_queue_sync_port(dest_queue)*/);
		if (tmp < 0) {
			perror("subscribe");
			exit(1);
		}	
	}
	
	/* change the pool size */
	if (snd_seq_set_client_pool_output(seq_handle, WRITE_POOL_SIZE) < 0 ||
	    snd_seq_set_client_pool_input(seq_handle, READ_POOL_SIZE) < 0 ||
	    snd_seq_set_client_pool_output_room(seq_handle, WRITE_POOL_SPACE) < 0) {
		perror("pool");
		exit(1);
	}
	
	if (optind < argc) {
		F = fopen(argv[optind], "r");
		if (F == NULL) {
			fprintf(stderr, "playmidi1: can't open file %s\n", argv[optind]);
			exit(1);
		}
	} else
		F = stdin;

	Mf_header = do_header;
	Mf_tempo = do_tempo;
	Mf_getc = mygetc;
	Mf_text = mytext;

	Mf_noteon = do_noteon;
	Mf_noteoff = do_noteoff;
	Mf_program = do_program;
	Mf_parameter = do_parameter;
	Mf_pitchbend = do_pitchbend;
	Mf_pressure = do_pressure;
	Mf_chanpressure = do_chanpressure;
	Mf_sysex = do_sysex;

	/* go.. go.. go.. */
	mfread();

	alsa_sync();
	if (! shared_queue)
		alsa_stop_timer();

	snd_seq_close(seq_handle);

	if (verbose >= VERB_INFO) {
		printf("Stopping at %f s,  tick %f\n",
		       tick2time_dbl(Mf_currtime + 1), (double) (Mf_currtime + 1));
	}

	exit(0);
}
