/*
 *  Atomic read/write
 *  Copyright (c) 2001 by Abramo Bagnara <abramo@alsa-project.org>
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */
  
#include <stdlib.h>
#include <time.h>
#include <sched.h>
#include "iatomic.h"

void snd_atomic_read_wait(snd_atomic_read_t *t)
{
	volatile const snd_atomic_write_t *w = t->write;
	unsigned int loops = 0;
	struct timespec ts;
	while (w->begin != w->end) {
		if (loops < MAX_SPIN_COUNT) {
			sched_yield();
			loops++;
			continue;
		}
		loops = 0;
		ts.tv_sec = 0;
		ts.tv_nsec = SPIN_SLEEP_DURATION;
		nanosleep(&ts, NULL);
	}
}
