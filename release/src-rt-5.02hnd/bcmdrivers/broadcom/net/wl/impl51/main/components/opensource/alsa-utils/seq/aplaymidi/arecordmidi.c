/*
 * arecordmidi.c - record standard MIDI files from sequencer ports
 *
 * Copyright (c) 2004-2005 Clemens Ladisch <clemens@ladisch.de>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

/* TODO: sequencer queue timer selection */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <sys/poll.h>
#include <alsa/asoundlib.h>
#include "aconfig.h"
#include "version.h"

#define BUFFER_SIZE 4088

/* linked list of buffers, stores data as in the .mid file */
struct buffer {
	struct buffer *next;
	unsigned char buf[BUFFER_SIZE];
};

struct smf_track {
	int size;			/* size of entire data */
	int cur_buf_size;		/* size of cur_buf */
	struct buffer *cur_buf;
	snd_seq_tick_time_t last_tick;	/* end of track */
	unsigned char last_command;	/* used for running status */
	int used;			/* anything record on this track */
	struct buffer first_buf;	/* list head */
};

/* timing/sysex + 16 channels */
#define TRACKS_PER_PORT 17

/* metronome settings */
/* TODO: create options for this */
#define METRONOME_CHANNEL 9
#define METRONOME_STRONG_NOTE 34
#define METRONOME_WEAK_NOTE 33
#define METRONOME_VELOCITY 100
#define METRONOME_PROGRAM 0

static snd_seq_t *seq;
static int client;
static int port_count;
static snd_seq_addr_t *ports;
static int queue;
static int smpte_timing = 0;
static int beats = 120;
static int frames;
static int ticks = 0;
static FILE *file;
static int channel_split;
static int num_tracks;
static struct smf_track *tracks;
static volatile sig_atomic_t stop = 0;
static int use_metronome = 0;
static snd_seq_addr_t metronome_port;
static int metronome_weak_note = METRONOME_WEAK_NOTE;
static int metronome_strong_note = METRONOME_STRONG_NOTE;
static int metronome_velocity = METRONOME_VELOCITY;
static int metronome_program = METRONOME_PROGRAM;
static int metronome_channel = METRONOME_CHANNEL;
static int ts_num = 4; /* time signature: numerator */
static int ts_div = 4; /* time signature: denominator */
static int ts_dd = 2; /* time signature: denominator as a power of two */


/* prints an error message to stderr, and dies */
static void fatal(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);
	fputc('\n', stderr);
	exit(EXIT_FAILURE);
}

/* memory allocation error handling */
static void check_mem(void *p)
{
	if (!p)
		fatal("Out of memory");
}

/* error handling for ALSA functions */
static void check_snd(const char *operation, int err)
{
	if (err < 0)
		fatal("Cannot %s - %s", operation, snd_strerror(err));
}

static void init_seq(void)
{
	int err;

	/* open sequencer */
	err = snd_seq_open(&seq, "default", SND_SEQ_OPEN_DUPLEX, 0);
	check_snd("open sequencer", err);

	/* find out our client's id */
	client = snd_seq_client_id(seq);
	check_snd("get client id", client);

	/* set our client's name */
	err = snd_seq_set_client_name(seq, "arecordmidi");
	check_snd("set client name", err);
}

/* parses one or more port addresses from the string */
static void parse_ports(const char *arg)
{
	char *buf, *s, *port_name;
	int err;

	/* make a copy of the string because we're going to modify it */
	buf = strdup(arg);
	check_mem(buf);

	for (port_name = s = buf; s; port_name = s + 1) {
		/* Assume that ports are separated by commas.  We don't use
		 * spaces because those are valid in client names. */
		s = strchr(port_name, ',');
		if (s)
			*s = '\0';

		++port_count;
		ports = realloc(ports, port_count * sizeof(snd_seq_addr_t));
		check_mem(ports);

		err = snd_seq_parse_address(seq, &ports[port_count - 1], port_name);
		if (err < 0)
			fatal("Invalid port %s - %s", port_name, snd_strerror(err));
	}

	free(buf);
}

/* parses the metronome port address */
static void init_metronome(const char *arg)
{
	int err;

	err = snd_seq_parse_address(seq, &metronome_port, arg);
	if (err < 0)
		fatal("Invalid port %s - %s", arg, snd_strerror(err));
	use_metronome = 1;
}

/* parses time signature specification */
static void time_signature(const char *arg)
{
	long x = 0;
	char *sep;

	x = strtol(arg, &sep, 10);
	if (x < 1 || x > 64 || *sep != ':')
		fatal("Invalid time signature (%s)", arg);
	ts_num = x;
	x = strtol(++sep, NULL, 10);
	if (x < 1 || x > 64)
		fatal("Invalid time signature (%s)", arg);
	ts_div = x;
	for (ts_dd = 0; x > 1; x /= 2)
		++ts_dd;
}

/*
 * Metronome implementation
 */
static void metronome_note(unsigned char note, unsigned int tick)
{
	snd_seq_event_t ev;
	snd_seq_ev_clear(&ev);
	snd_seq_ev_set_note(&ev, metronome_channel, note, metronome_velocity, 1);
	snd_seq_ev_schedule_tick(&ev, queue, 0, tick);
	snd_seq_ev_set_source(&ev, port_count);
	snd_seq_ev_set_subs(&ev);
	snd_seq_event_output(seq, &ev);
}

static void metronome_echo(unsigned int tick)
{
	snd_seq_event_t ev;
	snd_seq_ev_clear(&ev);
	ev.type = SND_SEQ_EVENT_USR0;
	snd_seq_ev_schedule_tick(&ev, queue, 0, tick);
	snd_seq_ev_set_source(&ev, port_count);
	snd_seq_ev_set_dest(&ev, client, port_count);
	snd_seq_event_output(seq, &ev);
}

static void metronome_pattern(unsigned int tick)
{
	int j, t, duration;

	t = tick;
	duration = ticks * 4 / ts_div;
	for (j = 0; j < ts_num; j++) {
		metronome_note(j ? metronome_weak_note : metronome_strong_note, t);
		t += duration;
	}
	metronome_echo(t);
	snd_seq_drain_output(seq);
}

static void metronome_set_program(void)
{
	snd_seq_event_t ev;

	snd_seq_ev_clear(&ev);
	snd_seq_ev_set_pgmchange(&ev, metronome_channel, metronome_program);
	snd_seq_ev_set_source(&ev, port_count);
	snd_seq_ev_set_subs(&ev);
	snd_seq_event_output(seq, &ev);
}

static void init_tracks(void)
{
	int i;

	/* MIDI RP-019 says we need at least one track per port */
	num_tracks = port_count;
	/* Allocate one track for each possible channel.
	 * Empty tracks won't be written to the file. */
	if (channel_split)
		num_tracks *= TRACKS_PER_PORT;

	tracks = calloc(num_tracks, sizeof(struct smf_track));
	check_mem(tracks);
	for (i = 0; i < num_tracks; ++i)
		tracks[i].cur_buf = &tracks[i].first_buf;
}

static void create_queue(void)
{
	snd_seq_queue_tempo_t *tempo;
	int err;

	queue = snd_seq_alloc_named_queue(seq, "arecordmidi");
	check_snd("create queue", queue);

	snd_seq_queue_tempo_alloca(&tempo);
	if (!smpte_timing) {
		snd_seq_queue_tempo_set_tempo(tempo, 60000000 / beats);
		snd_seq_queue_tempo_set_ppq(tempo, ticks);
	} else {
		/*
		 * ALSA doesn't know about the SMPTE time divisions, so
		 * we pretend to have a musical tempo with the equivalent
		 * number of ticks/s.
		 */
		switch (frames) {
		case 24:
			snd_seq_queue_tempo_set_tempo(tempo, 500000);
			snd_seq_queue_tempo_set_ppq(tempo, 12 * ticks);
			break;
		case 25:
			snd_seq_queue_tempo_set_tempo(tempo, 400000);
			snd_seq_queue_tempo_set_ppq(tempo, 10 * ticks);
			break;
		case 29:
			snd_seq_queue_tempo_set_tempo(tempo, 100000000);
			snd_seq_queue_tempo_set_ppq(tempo, 2997 * ticks);
			break;
		case 30:
			snd_seq_queue_tempo_set_tempo(tempo, 500000);
			snd_seq_queue_tempo_set_ppq(tempo, 15 * ticks);
			break;
		default:
			fatal("Invalid SMPTE frames %d", frames);
		}
	}
	err = snd_seq_set_queue_tempo(seq, queue, tempo);
	if (err < 0)
		fatal("Cannot set queue tempo (%u/%i)",
		      snd_seq_queue_tempo_get_tempo(tempo),
		      snd_seq_queue_tempo_get_ppq(tempo));
}

static void create_ports(void)
{
	snd_seq_port_info_t *pinfo;
	int i, err;
	char name[32];

	snd_seq_port_info_alloca(&pinfo);

	/* common information for all our ports */
	snd_seq_port_info_set_capability(pinfo,
					 SND_SEQ_PORT_CAP_WRITE |
					 SND_SEQ_PORT_CAP_SUBS_WRITE);
	snd_seq_port_info_set_type(pinfo,
				   SND_SEQ_PORT_TYPE_MIDI_GENERIC |
				   SND_SEQ_PORT_TYPE_APPLICATION);
	snd_seq_port_info_set_midi_channels(pinfo, 16);

	/* we want to know when the events got delivered to us */
	snd_seq_port_info_set_timestamping(pinfo, 1);
	snd_seq_port_info_set_timestamp_queue(pinfo, queue);

	/* our port number is the same as our port index */
	snd_seq_port_info_set_port_specified(pinfo, 1);
	for (i = 0; i < port_count; ++i) {
		snd_seq_port_info_set_port(pinfo, i);

		sprintf(name, "arecordmidi port %i", i);
		snd_seq_port_info_set_name(pinfo, name);

		err = snd_seq_create_port(seq, pinfo);
		check_snd("create port", err);
	}

	/* create an optional metronome port */
	if (use_metronome) {
		snd_seq_port_info_set_port(pinfo, port_count);
		snd_seq_port_info_set_name(pinfo, "arecordmidi metronome");
		snd_seq_port_info_set_capability(pinfo,
						 SND_SEQ_PORT_CAP_READ |
						 SND_SEQ_PORT_CAP_WRITE);
		snd_seq_port_info_set_type(pinfo, SND_SEQ_PORT_TYPE_APPLICATION);
		snd_seq_port_info_set_midi_channels(pinfo, 0);
		snd_seq_port_info_set_timestamping(pinfo, 0);
		err = snd_seq_create_port(seq, pinfo);
		check_snd("create metronome port", err);
	}
}

static void connect_ports(void)
{
	int i, err;

	for (i = 0; i < port_count; ++i) {
		err = snd_seq_connect_from(seq, i, ports[i].client, ports[i].port);
		if (err < 0)
			fatal("Cannot connect from port %d:%d - %s",
			      ports[i].client, ports[i].port, snd_strerror(err));
	}

	/* subscribe the metronome port */
	if (use_metronome) {
	        err = snd_seq_connect_to(seq, port_count, metronome_port.client, metronome_port.port);
		if (err < 0)
	    		fatal("Cannot connect to port %d:%d - %s",
			      metronome_port.client, metronome_port.port, snd_strerror(err));
	}
}

/* records a byte to be written to the .mid file */
static void add_byte(struct smf_track *track, unsigned char byte)
{
	/* make sure we have enough room in the current buffer */
	if (track->cur_buf_size >= BUFFER_SIZE) {
		track->cur_buf->next = calloc(1, sizeof(struct buffer));
		if (!track->cur_buf->next)
			fatal("out of memory");
		track->cur_buf = track->cur_buf->next;
		track->cur_buf_size = 0;
	}

	track->cur_buf->buf[track->cur_buf_size++] = byte;
	track->size++;
}

/* record a variable-length quantity */
static void var_value(struct smf_track *track, int v)
{
	if (v >= (1 << 28))
		add_byte(track, 0x80 | ((v >> 28) & 0x03));
	if (v >= (1 << 21))
		add_byte(track, 0x80 | ((v >> 21) & 0x7f));
	if (v >= (1 << 14))
		add_byte(track, 0x80 | ((v >> 14) & 0x7f));
	if (v >= (1 << 7))
		add_byte(track, 0x80 | ((v >> 7) & 0x7f));
	add_byte(track, v & 0x7f);
}

/* record the delta time from the last event */
static void delta_time(struct smf_track *track, const snd_seq_event_t *ev)
{
	int diff = ev->time.tick - track->last_tick;
	if (diff < 0)
		diff = 0;
	var_value(track, diff);
	track->last_tick = ev->time.tick;
}

/* record a status byte (or not if we can use running status) */
static void command(struct smf_track *track, unsigned char cmd)
{
	if (cmd != track->last_command)
		add_byte(track, cmd);
	track->last_command = cmd < 0xf0 ? cmd : 0;
}

/* put port numbers into all tracks */
static void record_port_numbers(void)
{
	int i;

	for (i = 0; i < num_tracks; ++i) {
		var_value(&tracks[i], 0);
		add_byte(&tracks[i], 0xff);
		add_byte(&tracks[i], 0x21);
		var_value(&tracks[i], 1);
		if (channel_split)
			add_byte(&tracks[i], i / TRACKS_PER_PORT);
		else
			add_byte(&tracks[i], i);
	}
}

static void record_event(const snd_seq_event_t *ev)
{
	unsigned int i;
	struct smf_track *track;

	/* ignore events without proper timestamps */
	if (ev->queue != queue || !snd_seq_ev_is_tick(ev))
		return;

	/* determine which track to record to */
	i = ev->dest.port;
	if (i == port_count) {
		if (ev->type == SND_SEQ_EVENT_USR0)
			metronome_pattern(ev->time.tick);
		return;
	}
	if (channel_split) {
		i *= TRACKS_PER_PORT;
		if (snd_seq_ev_is_channel_type(ev))
			i += 1 + (ev->data.note.channel & 0xf);
	}
	if (i >= num_tracks)
		return;
	track = &tracks[i];

	switch (ev->type) {
	case SND_SEQ_EVENT_NOTEON:
		delta_time(track, ev);
		command(track, MIDI_CMD_NOTE_ON | (ev->data.note.channel & 0xf));
		add_byte(track, ev->data.note.note & 0x7f);
		add_byte(track, ev->data.note.velocity & 0x7f);
		break;
	case SND_SEQ_EVENT_NOTEOFF:
		delta_time(track, ev);
		command(track, MIDI_CMD_NOTE_OFF | (ev->data.note.channel & 0xf));
		add_byte(track, ev->data.note.note & 0x7f);
		add_byte(track, ev->data.note.velocity & 0x7f);
		break;
	case SND_SEQ_EVENT_KEYPRESS:
		delta_time(track, ev);
		command(track, MIDI_CMD_NOTE_PRESSURE | (ev->data.note.channel & 0xf));
		add_byte(track, ev->data.note.note & 0x7f);
		add_byte(track, ev->data.note.velocity & 0x7f);
		break;
	case SND_SEQ_EVENT_CONTROLLER:
		delta_time(track, ev);
		command(track, MIDI_CMD_CONTROL | (ev->data.control.channel & 0xf));
		add_byte(track, ev->data.control.param & 0x7f);
		add_byte(track, ev->data.control.value & 0x7f);
		break;
	case SND_SEQ_EVENT_PGMCHANGE:
		delta_time(track, ev);
		command(track, MIDI_CMD_PGM_CHANGE | (ev->data.control.channel & 0xf));
		add_byte(track, ev->data.control.value & 0x7f);
		break;
	case SND_SEQ_EVENT_CHANPRESS:
		delta_time(track, ev);
		command(track, MIDI_CMD_CHANNEL_PRESSURE | (ev->data.control.channel & 0xf));
		add_byte(track, ev->data.control.value & 0x7f);
		break;
	case SND_SEQ_EVENT_PITCHBEND:
		delta_time(track, ev);
		command(track, MIDI_CMD_BENDER | (ev->data.control.channel & 0xf));
		add_byte(track, (ev->data.control.value + 8192) & 0x7f);
		add_byte(track, ((ev->data.control.value + 8192) >> 7) & 0x7f);
		break;
	case SND_SEQ_EVENT_CONTROL14:
		/* create two commands for MSB and LSB */
		delta_time(track, ev);
		command(track, MIDI_CMD_CONTROL | (ev->data.control.channel & 0xf));
		add_byte(track, ev->data.control.param & 0x7f);
		add_byte(track, (ev->data.control.value >> 7) & 0x7f);
		if ((ev->data.control.param & 0x7f) < 0x20) {
			delta_time(track, ev);
			/* running status */
			add_byte(track, (ev->data.control.param & 0x7f) + 0x20);
			add_byte(track, ev->data.control.value & 0x7f);
		}
		break;
	case SND_SEQ_EVENT_NONREGPARAM:
		delta_time(track, ev);
		command(track, MIDI_CMD_CONTROL | (ev->data.control.channel & 0xf));
		add_byte(track, MIDI_CTL_NONREG_PARM_NUM_LSB);
		add_byte(track, ev->data.control.param & 0x7f);
		delta_time(track, ev);
		add_byte(track, MIDI_CTL_NONREG_PARM_NUM_MSB);
		add_byte(track, (ev->data.control.param >> 7) & 0x7f);
		delta_time(track, ev);
		add_byte(track, MIDI_CTL_MSB_DATA_ENTRY);
		add_byte(track, (ev->data.control.value >> 7) & 0x7f);
		delta_time(track, ev);
		add_byte(track, MIDI_CTL_LSB_DATA_ENTRY);
		add_byte(track, ev->data.control.value & 0x7f);
		break;
	case SND_SEQ_EVENT_REGPARAM:
		delta_time(track, ev);
		command(track, MIDI_CMD_CONTROL | (ev->data.control.channel & 0xf));
		add_byte(track, MIDI_CTL_REGIST_PARM_NUM_LSB);
		add_byte(track, ev->data.control.param & 0x7f);
		delta_time(track, ev);
		add_byte(track, MIDI_CTL_REGIST_PARM_NUM_MSB);
		add_byte(track, (ev->data.control.param >> 7) & 0x7f);
		delta_time(track, ev);
		add_byte(track, MIDI_CTL_MSB_DATA_ENTRY);
		add_byte(track, (ev->data.control.value >> 7) & 0x7f);
		delta_time(track, ev);
		add_byte(track, MIDI_CTL_LSB_DATA_ENTRY);
		add_byte(track, ev->data.control.value & 0x7f);
		break;
	case SND_SEQ_EVENT_SYSEX:
		if (ev->data.ext.len == 0)
			break;
		delta_time(track, ev);
		if (*(unsigned char*)ev->data.ext.ptr == 0xf0)
			command(track, 0xf0), i = 1;
		else
			command(track, 0xf7), i = 0;
		var_value(track, ev->data.ext.len - i);
		for (; i < ev->data.ext.len; ++i)
			add_byte(track, ((unsigned char*)ev->data.ext.ptr)[i]);
		break;
	default:
		return;
	}
	track->used = 1;
}

static void finish_tracks(void)
{
	snd_seq_queue_status_t *queue_status;
	int tick, i, err;

	snd_seq_queue_status_alloca(&queue_status);

	err = snd_seq_get_queue_status(seq, queue, queue_status);
	check_snd("get queue status", err);
	tick = snd_seq_queue_status_get_tick_time(queue_status);

	/* make length of first track the recording length */
	var_value(&tracks[0], tick - tracks[0].last_tick);
	add_byte(&tracks[0], 0xff);
	add_byte(&tracks[0], 0x2f);
	var_value(&tracks[0], 0);

	/* finish other tracks */
	for (i = 1; i < num_tracks; ++i) {
		var_value(&tracks[i], 0);
		add_byte(&tracks[i], 0xff);
		add_byte(&tracks[i], 0x2f);
		var_value(&tracks[i], 0);
	}
}

static void write_file(void)
{
	int used_tracks, time_division, i;
	struct buffer *buf;

	used_tracks = 0;
	for (i = 0; i < num_tracks; ++i)
		used_tracks += !!tracks[i].used;

	/* header id and length */
	fwrite("MThd\0\0\0\6", 1, 8, file);
	/* type 0 or 1 */
	fputc(0, file);
	fputc(used_tracks > 1, file);
	/* number of tracks */
	fputc((used_tracks >> 8) & 0xff, file);
	fputc(used_tracks & 0xff, file);
	/* time division */
	time_division = ticks;
	if (smpte_timing)
		time_division |= (0x100 - frames) << 8;
	fputc(time_division >> 8, file);
	fputc(time_division & 0xff, file);

	for (i = 0; i < num_tracks; ++i) {
		if (!tracks[i].used)
			continue;
		/* track id */
		fwrite("MTrk", 1, 4, file);
		/* data length */
		fputc((tracks[i].size >> 24) & 0xff, file);
		fputc((tracks[i].size >> 16) & 0xff, file);
		fputc((tracks[i].size >> 8) & 0xff, file);
		fputc(tracks[i].size & 0xff, file);
		/* track contents */
		for (buf = &tracks[i].first_buf; buf; buf = buf->next)
			fwrite(buf->buf, 1, buf == tracks[i].cur_buf
			       ? tracks[i].cur_buf_size : BUFFER_SIZE, file);
	}
}

static void list_ports(void)
{
	snd_seq_client_info_t *cinfo;
	snd_seq_port_info_t *pinfo;

	snd_seq_client_info_alloca(&cinfo);
	snd_seq_port_info_alloca(&pinfo);

	puts(" Port    Client name                      Port name");

	snd_seq_client_info_set_client(cinfo, -1);
	while (snd_seq_query_next_client(seq, cinfo) >= 0) {
		int client = snd_seq_client_info_get_client(cinfo);

		if (client == SND_SEQ_CLIENT_SYSTEM)
			continue; /* don't show system timer and announce ports */
		snd_seq_port_info_set_client(pinfo, client);
		snd_seq_port_info_set_port(pinfo, -1);
		while (snd_seq_query_next_port(seq, pinfo) >= 0) {
			/* port must understand MIDI messages */
			if (!(snd_seq_port_info_get_type(pinfo)
			      & SND_SEQ_PORT_TYPE_MIDI_GENERIC))
				continue;
			/* we need both READ and SUBS_READ */
			if ((snd_seq_port_info_get_capability(pinfo)
			     & (SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ))
			    != (SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ))
				continue;
			printf("%3d:%-3d  %-32.32s %s\n",
			       snd_seq_port_info_get_client(pinfo),
			       snd_seq_port_info_get_port(pinfo),
			       snd_seq_client_info_get_name(cinfo),
			       snd_seq_port_info_get_name(pinfo));
		}
	}
}

static void help(const char *argv0)
{
	fprintf(stderr, "Usage: %s [options] outputfile\n"
		"\nAvailable options:\n"
		"  -h,--help                  this help\n"
		"  -V,--version               show version\n"
		"  -l,--list                  list input ports\n"
		"  -p,--port=client:port,...  source port(s)\n"
		"  -b,--bpm=beats             tempo in beats per minute\n"
		"  -f,--fps=frames            resolution in frames per second (SMPTE)\n"
		"  -t,--ticks=ticks           resolution in ticks per beat or frame\n"
		"  -s,--split-channels        create a track for each channel\n"
		"  -m,--metronome=client:port play a metronome signal\n"
		"  -i,--timesig=nn:dd         time signature\n",
		argv0);
}

static void version(void)
{
	fputs("arecordmidi version " SND_UTIL_VERSION_STR "\n", stderr);
}

static void sighandler(int sig)
{
	stop = 1;
}

int main(int argc, char *argv[])
{
	static const char short_options[] = "hVlp:b:f:t:sdm:i:";
	static const struct option long_options[] = {
		{"help", 0, NULL, 'h'},
		{"version", 0, NULL, 'V'},
		{"list", 0, NULL, 'l'},
		{"port", 1, NULL, 'p'},
		{"bpm", 1, NULL, 'b'},
		{"fps", 1, NULL, 'f'},
		{"ticks", 1, NULL, 't'},
		{"split-channels", 0, NULL, 's'},
		{"dump", 0, NULL, 'd'},
		{"metronome", 1, NULL, 'm'},
		{"timesig", 1, NULL, 'i'},
		{ }
	};

	char *filename = NULL;
	int do_list = 0;
	struct pollfd *pfds;
	int npfds;
	int c, err;

	init_seq();

	while ((c = getopt_long(argc, argv, short_options,
				long_options, NULL)) != -1) {
		switch (c) {
		case 'h':
			help(argv[0]);
			return 0;
		case 'V':
			version();
			return 0;
		case 'l':
			do_list = 1;
			break;
		case 'p':
			parse_ports(optarg);
			break;
		case 'b':
			beats = atoi(optarg);
			if (beats < 4 || beats > 6000)
				fatal("Invalid tempo");
			smpte_timing = 0;
			break;
		case 'f':
			frames = atoi(optarg);
			if (frames != 24 && frames != 25 &&
			    frames != 29 && frames != 30)
				fatal("Invalid number of frames/s");
			smpte_timing = 1;
			break;
		case 't':
			ticks = atoi(optarg);
			if (ticks < 1 || ticks > 0x7fff)
				fatal("Invalid number of ticks");
			break;
		case 's':
			channel_split = 1;
			break;
		case 'd':
			fputs("The --dump option isn't supported anymore, use aseqdump instead.\n", stderr);
			break;
		case 'm':
			init_metronome(optarg);
			break;
		case 'i':
			time_signature(optarg);
			break;
		default:
			help(argv[0]);
			return 1;
		}
	}

	if (do_list) {
		list_ports();
		return 0;
	}

	if (port_count < 1) {
		fputs("Pleast specify a source port with --port.\n", stderr);
		return 1;
	}

	if (!ticks)
		ticks = smpte_timing ? 40 : 384;
	if (smpte_timing && ticks > 0xff)
		ticks = 0xff;

	if (optind >= argc) {
		fputs("Please specify a file to record to.\n", stderr);
		return 1;
	}
	filename = argv[optind];

	init_tracks();
	create_queue();
	create_ports();
	connect_ports();
	if (port_count > 1)
		record_port_numbers();

	/* record tempo */
	if (!smpte_timing) {
		int usecs_per_quarter = 60000000 / beats;
		var_value(&tracks[0], 0); /* delta time */
		add_byte(&tracks[0], 0xff);
		add_byte(&tracks[0], 0x51);
		var_value(&tracks[0], 3);
		add_byte(&tracks[0], usecs_per_quarter >> 16);
		add_byte(&tracks[0], usecs_per_quarter >> 8);
		add_byte(&tracks[0], usecs_per_quarter);

		/* time signature */
		var_value(&tracks[0], 0); /* delta time */
		add_byte(&tracks[0], 0xff);
		add_byte(&tracks[0], 0x58);
		var_value(&tracks[0], 4);
		add_byte(&tracks[0], ts_num);
		add_byte(&tracks[0], ts_dd);
		add_byte(&tracks[0], 24); /* MIDI clocks per metronome click */
		add_byte(&tracks[0], 8); /* notated 32nd-notes per MIDI quarter note */
	}
	
	/* always write at least one track */
	tracks[0].used = 1;

	file = fopen(filename, "wb");
	if (!file)
		fatal("Cannot open %s - %s", filename, strerror(errno));

	err = snd_seq_start_queue(seq, queue, NULL);
	check_snd("start queue", err);
	snd_seq_drain_output(seq);

	err = snd_seq_nonblock(seq, 1);
	check_snd("set nonblock mode", err);
	
	if (use_metronome) {
		metronome_set_program();
		metronome_pattern(0);
	}

	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);

	npfds = snd_seq_poll_descriptors_count(seq, POLLIN);
	pfds = alloca(sizeof(*pfds) * npfds);
	for (;;) {
		snd_seq_poll_descriptors(seq, pfds, npfds, POLLIN);
		if (poll(pfds, npfds, -1) < 0)
			break;
		do {
			snd_seq_event_t *event;
			err = snd_seq_event_input(seq, &event);
			if (err < 0)
				break;
			if (event)
				record_event(event);
		} while (err > 0);
		if (stop)
			break;
	}

	finish_tracks();
	write_file();

	fclose(file);
	snd_seq_close(seq);
	return 0;
}
