// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2015,2016 Felipe F. Tonello <eu@felipetonello.com>
 *  Copyright (C) 2016 ROLI Ltd.
 *
 */

#include <glib.h>

/* Avoid linkage problem on unit-tests */
#ifndef MIDI_TEST
#include "src/backtrace.h"
#define MIDI_ASSERT(_expr) btd_assert(_expr)
#else
#define MIDI_ASSERT(_expr) g_assert(_expr)
#endif
#include "libmidi.h"

inline static void buffer_append_byte(struct midi_buffer *buffer,
                                      const uint8_t byte)
{
	buffer->data[buffer->len++] = byte;
}

inline static void buffer_append_data(struct midi_buffer *buffer,
                                      const uint8_t *data, size_t size)
{
	memcpy(buffer->data + buffer->len, data, size);
	buffer->len += size;
}

inline static uint8_t buffer_reverse_get(struct midi_buffer *buffer, size_t i)
{
	MIDI_ASSERT(buffer->len > i);
	return buffer->data[buffer->len - (i + 1)];
}

inline static void buffer_reverse_set(struct midi_buffer *buffer, size_t i,
                                      const uint8_t byte)
{
	MIDI_ASSERT(buffer->len > i);
	buffer->data[buffer->len - (i + 1)] = byte;
}

inline static size_t parser_get_available_size(struct midi_write_parser *parser)
{
	return parser->stream_size - parser->midi_stream.len;
}

inline static uint8_t sysex_get(const snd_seq_event_t *ev, size_t i)
{
	MIDI_ASSERT(ev->data.ext.len > i);
	return ((uint8_t*)ev->data.ext.ptr)[i];
}

inline static void append_timestamp_high_maybe(struct midi_write_parser *parser)
{
	uint8_t timestamp_high = 0x80;

	if (midi_write_has_data(parser))
		return;

	/* Make sure timesampt_high is assigned a non-zero value */
	do {
		/* convert µs to ms */
		parser->rtime = g_get_monotonic_time() / 1000;
		timestamp_high |= (parser->rtime & 0x1F80) >> 7;
	} while (timestamp_high == 0x80);

	/* set timestampHigh */
	buffer_append_byte(&parser->midi_stream, timestamp_high);
}

inline static void append_timestamp_low(struct midi_write_parser *parser)
{
	const uint8_t timestamp_low = 0x80 | (parser->rtime & 0x7F);
	buffer_append_byte(&parser->midi_stream, timestamp_low);
}

int midi_write_init(struct midi_write_parser *parser, size_t buffer_size)
{
	int err;

	parser->rtime = 0;
	parser->rstatus = SND_SEQ_EVENT_NONE;
	parser->stream_size = buffer_size;

	parser->midi_stream.data = malloc(buffer_size);
	if (!parser->midi_stream.data)
		return -ENOMEM;

	parser->midi_stream.len = 0;

	err = snd_midi_event_new(buffer_size, &parser->midi_ev);
	if (err < 0)
		free(parser->midi_stream.data);

	return err;
}

int midi_read_init(struct midi_read_parser *parser)
{
	int err;

	parser->rstatus = 0;
	parser->rtime = -1;
	parser->timestamp = 0;
	parser->timestamp_low = 0;
	parser->timestamp_high = 0;

	parser->sysex_stream.data = malloc(MIDI_SYSEX_MAX_SIZE);
	if (!parser->sysex_stream.data)
		return -ENOMEM;

	parser->sysex_stream.len = 0;

	err = snd_midi_event_new(MIDI_MSG_MAX_SIZE, &parser->midi_ev);
	if (err < 0)
		free(parser->sysex_stream.data);

	return err;
}

/* Algorithm:
   1) check initial timestampLow:
       if used_sysex == 0, then tsLow = 1, else tsLow = 0
   2) calculate sysex size of current packet:
   2a) first check special case:
       if midi->out_length - 1 (tsHigh) - tsLow ==
          sysex_length - used_sysex
       size is: min(midi->out_length - 1 - tsLow,
                    sysex_length - used_sysex - 1)
   2b) else size is: min(midi->out_length - 1 - tsLow,
                     sysex_length - used_sysex)
   3) check if packet contains F7: fill respective tsLow byte
*/
static void read_ev_sysex(struct midi_write_parser *parser, const snd_seq_event_t *ev,
                          midi_read_ev_cb write_cb, void *user_data)
{
	unsigned int used_sysex = 0;

	/* We need at least 2 bytes (timestampLow + F0) */
	if (parser_get_available_size(parser) < 2) {
		/* send current message and start new one */
		write_cb(parser, user_data);
		midi_write_reset(parser);
		append_timestamp_high_maybe(parser);
	}

	/* timestampLow on initial F0 */
	if (sysex_get(ev, 0) == 0xF0)
		append_timestamp_low(parser);

	do {
		unsigned int size_of_sysex;

		append_timestamp_high_maybe(parser);

		size_of_sysex = MIN(parser_get_available_size(parser),
		                    ev->data.ext.len - used_sysex);

		if (parser_get_available_size(parser) == ev->data.ext.len - used_sysex)
			size_of_sysex--;

		buffer_append_data(&parser->midi_stream,
		                    ev->data.ext.ptr + used_sysex,
		                    size_of_sysex);
		used_sysex += size_of_sysex;

		if (parser_get_available_size(parser) <= 1 &&
		    buffer_reverse_get(&parser->midi_stream, 0) != 0xF7) {
			write_cb(parser, user_data);
			midi_write_reset(parser);
		}
	} while (used_sysex < ev->data.ext.len);

	/* check for F7 and update respective timestampLow byte */
	if (midi_write_has_data(parser) &&
	    buffer_reverse_get(&parser->midi_stream, 0) == 0xF7) {
		/* remove 0xF7 from buffer, append timestamp and add 0xF7 back again */
		parser->midi_stream.len--;
		append_timestamp_low(parser);
		buffer_append_byte(&parser->midi_stream, 0xF7);
	}
}

static void read_ev_others(struct midi_write_parser *parser, const snd_seq_event_t *ev,
                           midi_read_ev_cb write_cb, void *user_data)
{
	int length;

	/* check for running status */
	if (parser->rstatus != ev->type) {
		snd_midi_event_reset_decode(parser->midi_ev);
		append_timestamp_low(parser);
	}

	/* each midi message has timestampLow byte to follow */
	length = snd_midi_event_decode(parser->midi_ev,
	                               parser->midi_stream.data +
	                               parser->midi_stream.len,
	                               parser_get_available_size(parser),
	                               ev);

	if (length == -ENOMEM) {
		/* remove previously added timestampLow */
		if (parser->rstatus != ev->type)
			parser->midi_stream.len--;
		write_cb(parser, user_data);
		/* cleanup state for next packet */
		snd_midi_event_reset_decode(parser->midi_ev);
		midi_write_reset(parser);
		append_timestamp_high_maybe(parser);
		append_timestamp_low(parser);
		length = snd_midi_event_decode(parser->midi_ev,
		                               parser->midi_stream.data +
		                               parser->midi_stream.len,
		                               parser_get_available_size(parser),
		                               ev);
	}

	if (length > 0)
		parser->midi_stream.len += length;
}

void midi_read_ev(struct midi_write_parser *parser, const snd_seq_event_t *ev,
                  midi_read_ev_cb write_cb, void *user_data)
{
	MIDI_ASSERT(write_cb);

	append_timestamp_high_maybe(parser);

	/* SysEx is special case:
	   SysEx has two timestampLow bytes, before F0 and F7
	*/
	if (ev->type == SND_SEQ_EVENT_SYSEX)
		read_ev_sysex(parser, ev, write_cb, user_data);
	else
		read_ev_others(parser, ev, write_cb, user_data);

	parser->rstatus = ev->type;

	if (parser_get_available_size(parser) == 0) {
		write_cb(parser, user_data);
		midi_write_reset(parser);
	}
}

static void update_ev_timestamp(struct midi_read_parser *parser,
                                snd_seq_event_t *ev, uint16_t ts_low)
{
	int delta_timestamp;
	int delta_rtime;
	int64_t rtime_current;
	uint16_t timestamp;

	/* time_low overwflow results on time_high to increment by one */
	if (parser->timestamp_low > ts_low)
		parser->timestamp_high++;

	timestamp = (parser->timestamp_high << 7) | parser->timestamp_low;

	rtime_current = g_get_monotonic_time() / 1000; /* convert µs to ms */
	delta_timestamp = timestamp - (int)parser->timestamp;
	delta_rtime = rtime_current - parser->rtime;

	if (delta_rtime > MIDI_MAX_TIMESTAMP)
		parser->rtime = rtime_current;
	else {

		/* If delta_timestamp is way to big than delta_rtime,
		   this means that the device sent a message in the past,
		   so we have to compensate for this. */
		if (delta_timestamp > 7000 && delta_rtime < 1000)
			delta_timestamp = 0;

		/* check if timestamp did overflow */
		if (delta_timestamp < 0) {
			/* same timestamp in the past problem */
			if ((delta_timestamp + MIDI_MAX_TIMESTAMP) > 7000 &&
			    delta_rtime < 1000)
				delta_timestamp = 0;
			else
				delta_timestamp = delta_timestamp + MIDI_MAX_TIMESTAMP;
		}

		parser->rtime += delta_timestamp;
	}

	parser->timestamp += delta_timestamp;
	if (parser->timestamp > MIDI_MAX_TIMESTAMP)
		parser->timestamp %= MIDI_MAX_TIMESTAMP + 1;

	/* set event timestamp */
	/* TODO: update event timestamp here! */
}

static size_t handle_end_of_sysex(struct midi_read_parser *parser,
                                  snd_seq_event_t *ev,
                                  const uint8_t *data,
                                  size_t sysex_length)
{
	uint8_t time_low;

	/* At this time, timestampLow is copied as the last byte,
	   instead of 0xF7 */
	buffer_append_data(&parser->sysex_stream, data, sysex_length);

	time_low = buffer_reverse_get(&parser->sysex_stream, 0) & 0x7F;

	/* Remove timestamp byte */
	buffer_reverse_set(&parser->sysex_stream, 0, 0xF7);

	/* Update event */
	update_ev_timestamp(parser, ev, time_low);
	snd_seq_ev_set_sysex(ev, parser->sysex_stream.len,
	                     parser->sysex_stream.data);

	return sysex_length + 1; /* +1 because of timestampLow */
}

/* Searches the end of a SysEx message that contains a timestampLow
 * before the SysEx end byte. Returns the number of bytes of valid
 * SysEx payload in the buffer.
 */
static size_t sysex_eox_len(const uint8_t *data, size_t size)
{
	size_t i = 0;

	MIDI_ASSERT(size > 0);

	if (data[i] == 0xF0)
		i++;

	/* Search for timestamp low */
	while (i < size) {
		if ((data[i] & 0x80)) {
			i++;
			break;
		}
		i++;
	}

	return (i < size && data[i] == 0xF7) ? i : 0;
}


size_t midi_read_raw(struct midi_read_parser *parser, const uint8_t *data,
                    size_t size, snd_seq_event_t *ev /* OUT */)
{
	size_t midi_size = 0;
	size_t i = 0;
	bool err = false;

	if (parser->timestamp_high == 0)
		parser->timestamp_high = data[i++] & 0x3F;

	snd_midi_event_reset_encode(parser->midi_ev);

	/* timestamp byte */
	if (data[i] & 0x80) {
		update_ev_timestamp(parser, ev, data[i] & 0x7F);

		/* check for wrong BLE-MIDI message size */
		if (++i == size) {
			err = true;
			goto _finish;
		}
	}

	/* cleanup sysex_stream if message is broken or is a new SysEx */
	if (data[i] >= 0x80 && data[i] != 0xF7 && parser->sysex_stream.len > 0)
		parser->sysex_stream.len = 0;

	switch (data[i]) {
	case 0xF8 ... 0XFF:
		/* System Real-Time Messages */
		midi_size = 1;
		break;

		/* System Common Messages */
	case 0xF0: /* SysEx Start */ {
		size_t sysex_length;

		/* cleanup Running Status Message */
		parser->rstatus = 0;

		sysex_length = sysex_eox_len(data + i, size - i);
		/* Search for End of SysEx message in one BLE message */
		if (sysex_length > 0) {
			midi_size = handle_end_of_sysex(parser, ev, data + i,
			                                sysex_length);
		} else {
			buffer_append_data(&parser->sysex_stream, data + i, size - i);
			err = true; /* Not an actual error, just incomplete message */
			midi_size = size - i;
		}

		goto _finish;
	}

	case 0xF1:
	case 0xF3:
		midi_size = 2;
		break;
	case 0xF2:
		midi_size = 3;
		break;
	case 0xF4:
	case 0xF5: /* Ignore */
		i++;
		err = true;
		goto _finish;
		break;
	case 0xF6:
		midi_size = 1;
		break;
	case 0xF7: /* SysEx End */
		buffer_append_byte(&parser->sysex_stream, 0xF7);
		snd_seq_ev_set_sysex(ev, parser->sysex_stream.len,
		                     parser->sysex_stream.data);

		midi_size = 1; /* timestampLow was alredy processed */
		goto _finish;

	case 0x80 ... 0xEF:
		/*
		 * Channel Voice Messages, Channel Mode Messages
		 * and Control Change Messages.
		 */
		parser->rstatus = data[i];
		midi_size = (data[i] >= 0xC0 && data[i] <= 0xDF) ? 2 : 3;
		break;

	case 0x00 ... 0x7F:

		/* Check for SysEx messages */
		if (parser->sysex_stream.len > 0) {
			size_t sysex_length;

			sysex_length = sysex_eox_len(data + i, size - i);
			if (sysex_length > 0) {
				midi_size = handle_end_of_sysex(parser, ev, data + i,
				                                sysex_length);
			} else {
				buffer_append_data(&parser->sysex_stream, data + i, size - i);
				err = true; /* Not an actual error, just incomplete message */
				midi_size = size - i;
			}

			goto _finish;
		}

		/* Running State Message was not set */
		if (parser->rstatus == 0) {
			midi_size = 1;
			err = true;
			goto _finish;
		}

		snd_midi_event_encode_byte(parser->midi_ev, parser->rstatus, ev);
		midi_size = (parser->rstatus >= 0xC0 && parser->rstatus <= 0xDF) ? 1 : 2;
		break;
	}

	if ((i + midi_size) > size) {
		err = true;
		goto _finish;
	}

	snd_midi_event_encode(parser->midi_ev, data + i, midi_size, ev);

_finish:
	if (err)
		ev->type = SND_SEQ_EVENT_NONE;

	return i + midi_size;
}
