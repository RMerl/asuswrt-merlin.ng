/*
 *   Copyright (c) 2001-2010 Aaron Turner <aturner at synfin dot net>
 *   Copyright (c) 2013-2022 Fred Klassen <tcpreplay at appneta dot com> - AppNeta
 *
 *   The Tcpreplay Suite of tools is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation, either version 3 of the
 *   License, or with the authors permission any later version.
 *
 *   The Tcpreplay Suite is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with the Tcpreplay Suite.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "defines.h"
#include "config.h"

#define TRACE_MAX_ENTRIES 15000

struct timestamp_trace_entry {
    COUNTER skip_length;
    COUNTER size;
    COUNTER bytes_sent;
    COUNTER now_us;
    COUNTER tx_us;
    COUNTER next_tx_us;
    COUNTER sent_bits;
    struct timeval timestamp;
};
typedef struct timestamp_trace_entry timestamp_trace_entry_t;

#ifdef TIMESTAMP_TRACE
uint32_t trace_num;
timestamp_trace_entry_t timestamp_trace_entry_array[TRACE_MAX_ENTRIES];

static inline void
update_current_timestamp_trace_entry(COUNTER bytes_sent, COUNTER now_us, COUNTER tx_us, COUNTER next_tx_us)
{
    if (trace_num >= TRACE_MAX_ENTRIES)
        return;

    if (!now_us) {
        struct timeval now;
        gettimeofday(&now, NULL);
        now_us = TIMSTAMP_TO_MICROSEC(&now);
    }

    timestamp_trace_entry_array[trace_num].bytes_sent = bytes_sent;
    timestamp_trace_entry_array[trace_num].now_us = now_us;
    timestamp_trace_entry_array[trace_num].tx_us = tx_us;
    timestamp_trace_entry_array[trace_num].next_tx_us = next_tx_us;
}

static inline void
add_timestamp_trace_entry(COUNTER size, struct timeval *timestamp, COUNTER skip_length)
{
    if (trace_num >= TRACE_MAX_ENTRIES)
        return;

    timestamp_trace_entry_array[trace_num].skip_length = skip_length;
    timestamp_trace_entry_array[trace_num].size = size;
    timestamp_trace_entry_array[trace_num].timestamp.tv_sec = timestamp->tv_sec;
    timestamp_trace_entry_array[trace_num].timestamp.tv_usec = timestamp->tv_usec;
    ++trace_num;
}

static inline void
dump_timestamp_trace_array(const struct timeval *start, const struct timeval *stop, const COUNTER bps)
{
    uint32_t i;
    COUNTER start_us = TIMEVAL_TO_MICROSEC(start);

    printf("dump_timestamp_trace_array: start=%zd.%06zd stop=%zd.%06zd start_us=%llu traces=%u bps=%llu\n",
           start->tv_sec,
           start->tv_usec,
           stop->tv_sec,
           stop->tv_usec,
           start_us,
           trace_num,
           bps);
    for (i = 0; i < trace_num; ++i) {
        long long int delta = timestamp_trace_entry_array[i].tx_us - timestamp_trace_entry_array[i].next_tx_us;

        printf("timestamp=%zd.%zd, size=%llu now_us=%llu tx_us=%llu next_tx_us=%llu delta=%lld bytes_sent=%llu skip=%llu\n",
               timestamp_trace_entry_array[i].timestamp.tv_sec,
               timestamp_trace_entry_array[i].timestamp.tv_usec,
               timestamp_trace_entry_array[i].size,
               timestamp_trace_entry_array[i].now_us,
               timestamp_trace_entry_array[i].tx_us,
               timestamp_trace_entry_array[i].next_tx_us,
               delta,
               timestamp_trace_entry_array[i].bytes_sent,
               timestamp_trace_entry_array[i].skip_length);
    }
}
#else
static inline void
update_current_timestamp_trace_entry(COUNTER UNUSED(bytes_sent),
                                     COUNTER UNUSED(now_us),
                                     COUNTER UNUSED(tx_us),
                                     COUNTER UNUSED(next_tx_us))
{}
static inline void
add_timestamp_trace_entry(COUNTER UNUSED(size), struct timeval *UNUSED(timestamp), COUNTER UNUSED(skip_length))
{}
static inline void
dump_timestamp_trace_array(const struct timeval *UNUSED(start),
                           const struct timeval *UNUSED(stop),
                           const COUNTER UNUSED(bps))
{}
#endif /* TIMESTAMP_TRACE */
