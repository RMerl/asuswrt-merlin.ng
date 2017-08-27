#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <regex.h>
#include "test.h"

/*
 * Checks whether the regular expression matches the entire MIDI data, printed
 * as hex.
 */
static int midi_matches_regex(unsigned char *midi, int count, const char *regex)
{
	char *text;
	regex_t re;
	regmatch_t match;
	int i;

	text = malloc(2 * count + 1);
	if (!text)
		return 0;
	for (i = 0; i < count; ++i)
		sprintf(text + 2 * i, "%02x", midi[i]);
	if (regcomp(&re, regex, REG_EXTENDED) != 0) {
		free(text);
		return 0;
	}
	i = regexec(&re, text, 1, &match, 0);
	i = i == 0 && match.rm_so == 0 && match.rm_eo == strlen(text);
	regfree(&re);
	free(text);
	return i;
}

static void test_decode(void)
{
	snd_midi_event_t *midi_event;
	snd_seq_event_t ev;
	unsigned char buf[50];
	int count;

	if (ALSA_CHECK(snd_midi_event_new(256 /* ? */, &midi_event)) < 0)
		return;

#define DECODE() snd_midi_event_decode(midi_event, buf, sizeof(buf), &ev)
#define BUF_MATCHES(str) midi_matches_regex(buf, count, str)
#define DECODES_TO(str) ((count = DECODE()), BUF_MATCHES(str))

	snd_seq_ev_clear(&ev);

	snd_seq_ev_set_fixed(&ev);
	ev.type = SND_SEQ_EVENT_NONE;
	TEST_CHECK(DECODE() == -ENOENT);

	snd_seq_ev_set_noteoff(&ev, 1, 2, 3);
	TEST_CHECK(DECODES_TO("810203"));

	snd_seq_ev_set_noteon(&ev, 4, 5, 6);
	TEST_CHECK(DECODES_TO("940506"));

	snd_seq_ev_set_keypress(&ev, 7, 8, 9);
	TEST_CHECK(DECODES_TO("a70809"));

	snd_seq_ev_set_controller(&ev, 10, 11, 12);
	TEST_CHECK(DECODES_TO("ba0b0c"));

	snd_seq_ev_set_pgmchange(&ev, 13, 14);
	TEST_CHECK(DECODES_TO("cd0e"));

	snd_seq_ev_set_chanpress(&ev, 15, 16);
	TEST_CHECK(DECODES_TO("df10"));

	snd_seq_ev_set_pitchbend(&ev, 1, 0x222);
	TEST_CHECK(DECODES_TO("e12244"));

	snd_seq_ev_set_sysex(&ev, 6, "\xf0\x7e\x7f\x06\x01\xf7");
	TEST_CHECK(DECODES_TO("f07e7f0601f7"));

	snd_seq_ev_set_fixed(&ev);
	ev.type = SND_SEQ_EVENT_QFRAME;
	ev.data.control.value = 3;
	TEST_CHECK(DECODES_TO("f103"));

	ev.type = SND_SEQ_EVENT_SONGPOS;
	ev.data.control.value = 0x444;
	TEST_CHECK(DECODES_TO("f24408"));

	ev.type = SND_SEQ_EVENT_SONGSEL;
	ev.data.control.value = 5;
	TEST_CHECK(DECODES_TO("f305"));

	ev.type = SND_SEQ_EVENT_TUNE_REQUEST;
	TEST_CHECK(DECODES_TO("f6"));

	ev.type = SND_SEQ_EVENT_CLOCK;
	TEST_CHECK(DECODES_TO("f8"));

	ev.type = SND_SEQ_EVENT_START;
	TEST_CHECK(DECODES_TO("fa"));

	ev.type = SND_SEQ_EVENT_CONTINUE;
	TEST_CHECK(DECODES_TO("fb"));

	ev.type = SND_SEQ_EVENT_STOP;
	TEST_CHECK(DECODES_TO("fc"));

	ev.type = SND_SEQ_EVENT_SENSING;
	TEST_CHECK(DECODES_TO("fe"));

	ev.type = SND_SEQ_EVENT_RESET;
	TEST_CHECK(DECODES_TO("ff"));

	ev.type = SND_SEQ_EVENT_CONTROL14;
	ev.data.control.channel = 6;
	ev.data.control.param = 7;
	ev.data.control.value = 0x888;
	/*
	 * This regular expression catches all allowed combinations of LSB/MSB
	 * order and running status.
	 */
	TEST_CHECK(DECODES_TO("b6(0711(b6)?2708|2708(b6)?0711)"));

	ev.type = SND_SEQ_EVENT_NONREGPARAM;
	ev.data.control.channel = 9;
	ev.data.control.param = 0xaaa;
	ev.data.control.value = 0xbbb;
	TEST_CHECK(DECODES_TO("b9(622a(b9)?6315|6315(b9)?622a)(b9)?(0617(b9)?263b|263b(b9)?0617)"));

	ev.type = SND_SEQ_EVENT_REGPARAM;
	ev.data.control.channel = 12;
	ev.data.control.param = 0xddd;
	ev.data.control.value = 0xeee;
	TEST_CHECK(DECODES_TO("bc(645d(bc)?651b|651b(bc)?645d)(bc)?(061d(bc)?266e|266e(bc)?061d)"));

	/* no running status after SysEx */
	snd_seq_ev_set_pgmchange(&ev, 0, 0x11);
	TEST_CHECK(DECODES_TO("c011"));
	snd_seq_ev_set_sysex(&ev, 6, "\xf0\x7e\x7f\x09\x02\xf7");
	TEST_CHECK(DECODES_TO("f07e7f0902f7"));
	snd_seq_ev_set_pgmchange(&ev, 0, 0x11);
	TEST_CHECK(DECODES_TO("c011"));

	/* no running status for non-realtime common messages */
	ev.type = SND_SEQ_EVENT_QFRAME;
	ev.data.control.value = 0x11;
	TEST_CHECK(DECODES_TO("f111"));
	TEST_CHECK(DECODES_TO("f111"));

	/* buffer overflow */
	TEST_CHECK(snd_midi_event_decode(midi_event, buf, 1, &ev) == -ENOMEM);

	snd_midi_event_free(midi_event);
}

static void test_reset_decode(void)
{
	snd_midi_event_t *midi_event;
	snd_seq_event_t ev;
	unsigned char buf[50];
	int count;

	if (ALSA_CHECK(snd_midi_event_new(256 /* ? */, &midi_event)) < 0)
		return;

	snd_seq_ev_clear(&ev);

	snd_seq_ev_set_noteon(&ev, 1, 2, 3);
	TEST_CHECK(DECODES_TO("910203"));

	snd_midi_event_reset_decode(midi_event);

	TEST_CHECK(DECODES_TO("910203"));

	snd_midi_event_free(midi_event);
}

static void test_encode(void)
{
	snd_midi_event_t *midi_event;
	snd_seq_event_t ev;

	if (ALSA_CHECK(snd_midi_event_new(256, &midi_event)) < 0)
		return;

#define ENCODE(str) snd_midi_event_encode(midi_event, \
					  (const unsigned char *)str, \
					  sizeof(str) - 1, &ev)
	TEST_CHECK(ENCODE("\x81\x02\x03") == 3);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_NOTEOFF);
	TEST_CHECK((ev.flags & SND_SEQ_EVENT_LENGTH_MASK) == SND_SEQ_EVENT_LENGTH_FIXED);
	TEST_CHECK(ev.data.note.channel == 1);
	TEST_CHECK(ev.data.note.note == 2);
	TEST_CHECK(ev.data.note.velocity == 3);

	TEST_CHECK(ENCODE("\x94\x05\x06") == 3);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_NOTEON);
	TEST_CHECK(ev.data.note.channel == 4);
	TEST_CHECK(ev.data.note.note == 5);
	TEST_CHECK(ev.data.note.velocity == 6);

	TEST_CHECK(ENCODE("\xa7\x08\x09") == 3);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_KEYPRESS);
	TEST_CHECK(ev.data.note.channel == 7);
	TEST_CHECK(ev.data.note.note == 8);
	TEST_CHECK(ev.data.note.velocity == 9);

	TEST_CHECK(ENCODE("\xba\x0b\x0c") == 3);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_CONTROLLER);
	TEST_CHECK(ev.data.control.channel == 10);
	TEST_CHECK(ev.data.control.param == 11);
	TEST_CHECK(ev.data.control.value == 12);

	TEST_CHECK(ENCODE("\xcd\x0e") == 2);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_PGMCHANGE);
	TEST_CHECK(ev.data.control.channel == 13);
	TEST_CHECK(ev.data.control.value == 14);

	TEST_CHECK(ENCODE("\xdf\x10") == 2);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_CHANPRESS);
	TEST_CHECK(ev.data.control.channel == 15);
	TEST_CHECK(ev.data.control.value == 16);

	TEST_CHECK(ENCODE("\xe1\x22\x33") == 3);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_PITCHBEND);
	TEST_CHECK(ev.data.control.channel == 1);
	TEST_CHECK(ev.data.control.value == -1630);

	TEST_CHECK(ENCODE("\xf0\x7f\x7f\x04\x01\x7f\x7f\xf7") == 8);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_SYSEX);
	TEST_CHECK((ev.flags & SND_SEQ_EVENT_LENGTH_MASK) == SND_SEQ_EVENT_LENGTH_VARIABLE);
	TEST_CHECK(ev.data.ext.len == 8);
	TEST_CHECK(!memcmp(ev.data.ext.ptr, "\xf0\x7f\x7f\x04\x01\x7f\x7f\xf7", 8));

	TEST_CHECK(ENCODE("\xf1\x04") == 2);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_QFRAME);
	TEST_CHECK(ev.data.control.value == 4);

	TEST_CHECK(ENCODE("\xf2\x55\x66") == 3);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_SONGPOS);
	TEST_CHECK(ev.data.control.value == 13141);

	TEST_CHECK(ENCODE("\xf3\x07") == 2);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_SONGSEL);
	TEST_CHECK(ev.data.control.value == 7);

	TEST_CHECK(ENCODE("\xf6") == 1);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_TUNE_REQUEST);

	TEST_CHECK(ENCODE("\xf8") == 1);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_CLOCK);

	TEST_CHECK(ENCODE("\xfa") == 1);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_START);

	TEST_CHECK(ENCODE("\xfb") == 1);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_CONTINUE);

	TEST_CHECK(ENCODE("\xfc") == 1);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_STOP);

	TEST_CHECK(ENCODE("\xfe") == 1);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_SENSING);

	TEST_CHECK(ENCODE("\xff") == 1);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_RESET);

	TEST_CHECK(ENCODE("\xc1\xf8") == 2);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_CLOCK);
	TEST_CHECK(ENCODE("\x22") == 1);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_PGMCHANGE);
	TEST_CHECK(ev.data.control.channel == 1);
	TEST_CHECK(ev.data.control.value == 0x22);
	TEST_CHECK(ENCODE("\xf8") == 1);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_CLOCK);
	TEST_CHECK(ENCODE("\x33") == 1);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_PGMCHANGE);
	TEST_CHECK(ev.data.control.channel == 1);
	TEST_CHECK(ev.data.control.value == 0x33);

	TEST_CHECK(ENCODE("\xc1\xf6") == 2);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_TUNE_REQUEST);
	TEST_CHECK(ENCODE("\x44\x44") == 2);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_NONE);

	snd_midi_event_free(midi_event);
}

static void test_reset_encode(void)
{
	snd_midi_event_t *midi_event;
	snd_seq_event_t ev;

	if (ALSA_CHECK(snd_midi_event_new(256, &midi_event)) < 0)
		return;

	TEST_CHECK(ENCODE("\x91\x02") == 2);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_NONE);

	snd_midi_event_reset_encode(midi_event);

	TEST_CHECK(ENCODE("\x03") == 1);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_NONE);

	snd_midi_event_free(midi_event);
}

static void test_init(void)
{
	snd_midi_event_t *midi_event;
	snd_seq_event_t ev;
	unsigned char buf[50];
	int count;

	if (ALSA_CHECK(snd_midi_event_new(256, &midi_event)) < 0)
		return;

	snd_seq_ev_set_noteon(&ev, 1, 2, 3);
	TEST_CHECK(DECODES_TO("910203"));

	TEST_CHECK(ENCODE("\x94\x05") == 2);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_NONE);

	snd_midi_event_init(midi_event);

	snd_seq_ev_set_noteon(&ev, 1, 2, 3);
	TEST_CHECK(DECODES_TO("910203"));

	TEST_CHECK(ENCODE("\x06") == 1);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_NONE);

	snd_midi_event_free(midi_event);
}

static void test_encode_byte(void)
{
	snd_midi_event_t *midi_event;
	snd_seq_event_t ev;

	if (ALSA_CHECK(snd_midi_event_new(256, &midi_event)) < 0)
		return;

#define ENCODE_BYTE(c) snd_midi_event_encode_byte(midi_event, c, &ev)
	TEST_CHECK(ENCODE_BYTE(0x81) == 0);
	TEST_CHECK(ENCODE_BYTE(0x02) == 0);
	TEST_CHECK(ENCODE_BYTE(0x03) == 1);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_NOTEOFF);
	TEST_CHECK((ev.flags & SND_SEQ_EVENT_LENGTH_MASK) == SND_SEQ_EVENT_LENGTH_FIXED);
	TEST_CHECK(ev.data.note.channel == 1);
	TEST_CHECK(ev.data.note.note == 2);
	TEST_CHECK(ev.data.note.velocity == 3);
	TEST_CHECK(ENCODE_BYTE(0x04) == 0);
	TEST_CHECK(ENCODE_BYTE(0xf8) == 1);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_CLOCK);
	TEST_CHECK(ENCODE_BYTE(0x05) == 1);
	TEST_CHECK(ev.type == SND_SEQ_EVENT_NOTEOFF);
	TEST_CHECK(ev.data.note.channel == 1);
	TEST_CHECK(ev.data.note.note == 4);
	TEST_CHECK(ev.data.note.velocity == 5);

	snd_midi_event_free(midi_event);
}

int main(void)
{
	test_decode();
	test_reset_decode();
	test_encode();
	test_reset_encode();
	test_encode_byte();
	test_init();
	return TEST_EXIT_CODE();
}
