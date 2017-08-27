/*
 * detail.c	Process the detail file
 *
 * Version:	$Id$
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
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2007  The FreeRADIUS server project
 * Copyright 2007  Alan DeKok <aland@deployingradius.com>
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>
#include <freeradius-devel/detail.h>
#include <freeradius-devel/process.h>
#include <freeradius-devel/rad_assert.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_GLOB_H
#include <glob.h>
#endif

#include <fcntl.h>

#ifdef WITH_DETAIL

#define USEC (1000000)

static FR_NAME_NUMBER state_names[] = {
	{ "unopened", STATE_UNOPENED },
	{ "unlocked", STATE_UNLOCKED },
	{ "header", STATE_HEADER },
	{ "reading", STATE_READING },
	{ "queued", STATE_QUEUED },
	{ "running", STATE_RUNNING },
	{ "no-reply", STATE_NO_REPLY },
	{ "replied", STATE_REPLIED },

	{ NULL, 0 }
};

/*
 *	If we're limiting outstanding packets, then mark the response
 *	as being sent.
 */
int detail_send(rad_listen_t *listener, REQUEST *request)
{
	int rtt;
	struct timeval now;
	listen_detail_t *data = listener->data;

	rad_assert(request->listener == listener);
	rad_assert(listener->send == detail_send);

	/*
	 *	This request timed out.  Remember that, and tell the
	 *	caller it's OK to read more "detail" file stuff.
	 */
	if (request->reply->code == 0) {
		data->delay_time = data->retry_interval * USEC;
		data->signal = 1;
		data->state = STATE_NO_REPLY;

		RDEBUG("Detail - No response configured for request %d.  Will retry in %d seconds",
		       request->number, data->retry_interval);

		radius_signal_self(RADIUS_SIGNAL_SELF_DETAIL);
		return 0;
	}

	/*
	 *	We call gettimeofday a lot.  But it should be OK,
	 *	because there's nothing else to do.
	 */
	gettimeofday(&now, NULL);

	/*
	 *	If we haven't sent a packet in the last second, reset
	 *	the RTT.
	 */
	now.tv_sec -= 1;
	if (timercmp(&data->last_packet, &now, <)) {
		data->has_rtt = false;
	}
	now.tv_sec += 1;

	/*
	 *	Only one detail packet may be outstanding at a time,
	 *	so it's safe to update some entries in the detail
	 *	structure.
	 *
	 *	We keep smoothed round trip time (SRTT), but not round
	 *	trip timeout (RTO).  We use SRTT to calculate a rough
	 *	load factor.
	 */
	rtt = now.tv_sec - request->packet->timestamp.tv_sec;
	rtt *= USEC;
	rtt += now.tv_usec;
	rtt -= request->packet->timestamp.tv_usec;

	/*
	 *	If we're proxying, the RTT is our processing time,
	 *	plus the network delay there and back, plus the time
	 *	on the other end to process the packet.  Ideally, we
	 *	should remove the network delays from the RTT, but we
	 *	don't know what they are.
	 *
	 *	So, to be safe, we over-estimate the total cost of
	 *	processing the packet.
	 */
	if (!data->has_rtt) {
		data->has_rtt = true;
		data->srtt = rtt;
		data->rttvar = rtt / 2;

	} else {
		data->rttvar -= data->rttvar >> 2;
		data->rttvar += (data->srtt - rtt);
		data->srtt -= data->srtt >> 3;
		data->srtt += rtt >> 3;
	}

	/*
	 *	Calculate the time we wait before sending the next
	 *	packet.
	 *
	 *	rtt / (rtt + delay) = load_factor / 100
	 */
	data->delay_time = (data->srtt * (100 - data->load_factor)) / (data->load_factor);

	/*
	 *	Cap delay at 4 packets/s.  If the end system can't
	 *	handle this, then it's very broken.
	 */
	if (data->delay_time > (USEC / 4)) data->delay_time= USEC / 4;

	RDEBUG3("Received response for request %d.  Will read the next packet in %d seconds",
		request->number, data->delay_time / USEC);

	data->last_packet = now;
	data->signal = 1;
	data->state = STATE_REPLIED;
	data->counter++;
	radius_signal_self(RADIUS_SIGNAL_SELF_DETAIL);

	return 0;
}


/*
 *	Open the detail file, if we can.
 *
 *	FIXME: create it, if it's not already there, so that the main
 *	server select() will wake us up if there's anything to read.
 */
static int detail_open(rad_listen_t *this)
{
	struct stat st;
	listen_detail_t *data = this->data;
	char *filename = data->filename;

	rad_assert(data->state == STATE_UNOPENED);
	data->delay_time = USEC;

	/*
	 *	Open detail.work first, so we don't lose
	 *	accounting packets.  It's probably better to
	 *	duplicate them than to lose them.
	 *
	 *	Note that we're not writing to the file, but
	 *	we've got to open it for writing in order to
	 *	establish the lock, to prevent rlm_detail from
	 *	writing to it.
	 *
	 *	This also means that if we're doing globbing,
	 *	this file will be read && processed before the
	 *	file globbing is done.
	 */
	this->fd = open(data->filename_work, O_RDWR);
	if (this->fd < 0) {
		DEBUG2("Polling for detail file %s", filename);

		/*
		 *	Try reading the detail file.  If it
		 *	doesn't exist, we can't do anything.
		 *
		 *	Doing the stat will tell us if the file
		 *	exists, even if we don't have permissions
		 *	to read it.
		 */
		if (stat(filename, &st) < 0) {
#ifdef HAVE_GLOB_H
			unsigned int i;
			int found;
			time_t chtime;
			glob_t files;

			memset(&files, 0, sizeof(files));
			if (glob(filename, 0, NULL, &files) != 0) {
				globfree(&files);
				return 0;
			}

			chtime = 0;
			found = -1;
			for (i = 0; i < files.gl_pathc; i++) {
				if (stat(files.gl_pathv[i], &st) < 0) continue;

				if ((i == 0) ||
				    (st.st_ctime < chtime)) {
					chtime = st.st_ctime;
					found = i;
				}
			}

			if (found < 0) {
				globfree(&files);
				return 0;
			}

			filename = strdup(files.gl_pathv[found]);
			globfree(&files);
#else
			return 0;
#endif
		}

		/*
		 *	Open it BEFORE we rename it, just to
		 *	be safe...
		 */
		this->fd = open(filename, O_RDWR);
		if (this->fd < 0) {
			ERROR("Detail - Failed to open %s: %s",
			       filename, strerror(errno));
			if (filename != data->filename) free(filename);
			return 0;
		}

		/*
		 *	Rename detail to detail.work
		 */
		DEBUG("Detail - Renaming %s -> %s", filename, data->filename_work);
		if (rename(filename, data->filename_work) < 0) {
			if (filename != data->filename) free(filename);
			close(this->fd);
			this->fd = -1;
			return 0;
		}
		if (filename != data->filename) free(filename);
	} /* else detail.work existed, and we opened it */

	rad_assert(data->vps == NULL);
	rad_assert(data->fp == NULL);

	data->state = STATE_UNLOCKED;

	data->client_ip.af = AF_UNSPEC;
	data->timestamp = 0;
	data->offset = 0;
	data->packets = 0;
	data->tries = 0;

	return 1;
}


/*
 *	FIXME: add a configuration "exit when done" so that the detail
 *	file reader can be used as a one-off tool to update stuff.
 *
 *	The time sequence for reading from the detail file is:
 *
 *	t_0		signalled that the server is idle, and we
 *			can read from the detail file.
 *
 *	t_rtt		the packet has been processed successfully,
 *			wait for t_delay to enforce load factor.
 *
 *	t_rtt + t_delay wait for signal that the server is idle.
 *
 */
int detail_recv(rad_listen_t *listener)
{
	char		key[256], op[8], value[1024];
	vp_cursor_t	cursor;
	VALUE_PAIR	*vp;
	RADIUS_PACKET	*packet;
	char		buffer[2048];
	listen_detail_t *data = listener->data;

	/*
	 *	We may be in the main thread.  It needs to update the
	 *	timers before we try to read from the file again.
	 */
	if (data->signal) return 0;

	switch (data->state) {
		case STATE_UNOPENED:
	open_file:
			rad_assert(listener->fd < 0);

			if (!detail_open(listener)) return 0;

			rad_assert(data->state == STATE_UNLOCKED);
			rad_assert(listener->fd >= 0);

			/* FALL-THROUGH */

			/*
			 *	Try to lock fd.  If we can't, return.
			 *	If we can, continue.  This means that
			 *	the server doesn't block while waiting
			 *	for the lock to open...
			 */
		case STATE_UNLOCKED:
			/*
			 *	Note that we do NOT block waiting for
			 *	the lock.  We've re-named the file
			 *	above, so we've already guaranteed
			 *	that any *new* detail writer will not
			 *	be opening this file.  The only
			 *	purpose of the lock is to catch a race
			 *	condition where the execution
			 *	"ping-pongs" between radiusd &
			 *	radrelay.
			 */
			if (rad_lockfd_nonblock(listener->fd, 0) < 0) {
				/*
				 *	Close the FD.  The main loop
				 *	will wake up in a second and
				 *	try again.
				 */
				close(listener->fd);
				listener->fd = -1;
				data->state = STATE_UNOPENED;
				return 0;
			}

			data->fp = fdopen(listener->fd, "r");
			if (!data->fp) {
				ERROR("FATAL: Failed to re-open detail file %s: %s",
				       data->filename, strerror(errno));
				fr_exit(1);
			}

			/*
			 *	Look for the header
			 */
			data->state = STATE_HEADER;
			data->delay_time = USEC;
			data->vps = NULL;

			/* FALL-THROUGH */

		case STATE_HEADER:
		do_header:
			data->tries = 0;
			if (!data->fp) {
				data->state = STATE_UNOPENED;
				goto open_file;
			}

			{
				struct stat buf;

				if (fstat(listener->fd, &buf) < 0) {
					ERROR("Failed to stat "
					       "detail file %s: %s",
				       		data->filename,
				       		strerror(errno));

				       	goto cleanup;
				}
				if (((off_t) ftell(data->fp)) == buf.st_size) {
					goto cleanup;
				}
			}

			/*
			 *	End of file.  Delete it, and re-set
			 *	everything.
			 */
			if (feof(data->fp)) {
			cleanup:
				DEBUG("Detail - unlinking %s",
				      data->filename_work);
				unlink(data->filename_work);
				if (data->fp) fclose(data->fp);
				data->fp = NULL;
				listener->fd = -1;
				data->state = STATE_UNOPENED;
				rad_assert(data->vps == NULL);

				if (data->one_shot) {
					INFO("Finished reading \"one shot\" detail file - Exiting");
					radius_signal_self(RADIUS_SIGNAL_SELF_EXIT);
				}

				return 0;
			}

			/*
			 *	Else go read something.
			 */
			break;

			/*
			 *	Read more value-pair's, unless we're
			 *	at EOF.  In that case, queue whatever
			 *	we have.
			 */
		case STATE_READING:
			if (data->fp && !feof(data->fp)) break;
			data->state = STATE_QUEUED;

			/* FALL-THROUGH */

		case STATE_QUEUED:
			goto alloc_packet;

			/*
			 *	Periodically check what's going on.
			 *	If the request is taking too long,
			 *	retry it.
			 */
		case STATE_RUNNING:
			if (time(NULL) < (data->running + data->retry_interval)) {
				return 0;
			}

			DEBUG("No response to detail request.  Retrying");
			data->state = STATE_NO_REPLY;
			/* FALL-THROUGH */

			/*
			 *	If there's no reply, keep
			 *	retransmitting the current packet
			 *	forever.
			 */
		case STATE_NO_REPLY:
			data->state = STATE_QUEUED;
			goto alloc_packet;

			/*
			 *	We have a reply.  Clean up the old
			 *	request, and go read another one.
			 */
		case STATE_REPLIED:
			pairfree(&data->vps);
			data->state = STATE_HEADER;
			goto do_header;
	}

	paircursor(&cursor, &data->vps);

	/*
	 *	Read a header, OR a value-pair.
	 */
	while (fgets(buffer, sizeof(buffer), data->fp)) {
		data->offset = ftell(data->fp); /* for statistics */

		/*
		 *	Badly formatted file: delete it.
		 *
		 *	FIXME: Maybe flag an error?
		 */
		if (!strchr(buffer, '\n')) {
			pairfree(&data->vps);
			goto cleanup;
		}

		/*
		 *	We're reading VP's, and got a blank line.
		 *	Queue the packet.
		 */
		if ((data->state == STATE_READING) &&
		    (buffer[0] == '\n')) {
			data->state = STATE_QUEUED;
			break;
		}

		/*
		 *	Look for date/time header, and read VP's if
		 *	found.  If not, keep reading lines until we
		 *	find one.
		 */
		if (data->state == STATE_HEADER) {
			int y;

			if (sscanf(buffer, "%*s %*s %*d %*d:%*d:%*d %d", &y)) {
				data->state = STATE_READING;
			}
			continue;
		}

		/*
		 *	We have a full "attribute = value" line.
		 *	If it doesn't look reasonable, skip it.
		 *
		 *	FIXME: print an error for badly formatted attributes?
		 */
		if (sscanf(buffer, "%255s %8s %1023s", key, op, value) != 3) {
			WDEBUG2("Skipping badly formatted line %s",
			       buffer);
			continue;
		}

		/*
		 *	Should be =, :=, +=, ...
		 */
		if (!strchr(op, '=')) continue;

		/*
		 *	Skip non-protocol attributes.
		 */
		if (!strcasecmp(key, "Request-Authenticator")) continue;

		/*
		 *	Set the original client IP address, based on
		 *	what's in the detail file.
		 *
		 *	Hmm... we don't set the server IP address.
		 *	or port.  Oh well.
		 */
		if (!strcasecmp(key, "Client-IP-Address")) {
			data->client_ip.af = AF_INET;
			if (ip_hton(value, AF_INET, &data->client_ip) < 0) {
				ERROR("Failed parsing Client-IP-Address");

				pairfree(&data->vps);
				goto cleanup;
			}
			continue;
		}

		/*
		 *	The original time at which we received the
		 *	packet.  We need this to properly calculate
		 *	Acct-Delay-Time.
		 */
		if (!strcasecmp(key, "Timestamp")) {
			data->timestamp = atoi(value);

			vp = paircreate(data, PW_PACKET_ORIGINAL_TIMESTAMP, 0);
			if (vp) {
				vp->vp_date = (uint32_t) data->timestamp;
				vp->type = VT_DATA;
		    		pairinsert(&cursor, vp);
			}
			continue;
		}

		/*
		 *	Read one VP.
		 *
		 *	FIXME: do we want to check for non-protocol
		 *	attributes like radsqlrelay does?
		 */
		vp = NULL;
		if ((userparse(data, buffer, &vp) > 0) &&
		    (vp != NULL)) {
		    	pairinsert(&cursor, vp);
		}
	}

	/*
	 *	Some kind of error.
	 *
	 *	FIXME: Leave the file in-place, and warn the
	 *	administrator?
	 */
	if (ferror(data->fp)) goto cleanup;

	data->tries = 0;
	data->packets++;

	/*
	 *	Process the packet.
	 */
 alloc_packet:
	data->tries++;

	/*
	 *	The writer doesn't check that the record was
	 *	completely written.  If the disk is full, this can
	 *	result in a truncated record.  When that happens,
	 *	treat it as EOF.
	 */
	if (data->state != STATE_QUEUED) {
		ERROR("Truncated record: treating it as EOF for detail file %s", data->filename_work);
		goto cleanup;
	}

	/*
	 *	We're done reading the file, but we didn't read
	 *	anything.  Clean up, and don't return anything.
	 */
	if (!data->vps) {
		data->state = STATE_HEADER;
		if (!data->fp || feof(data->fp)) goto cleanup;
		return 0;
	}

	/*
	 *	Allocate the packet.  If we fail, it's a serious
	 *	problem.
	 */
	packet = rad_alloc(NULL, 1);
	if (!packet) {
		ERROR("FATAL: Failed allocating memory for detail");
		fr_exit(1);
	}

	memset(packet, 0, sizeof(*packet));
	packet->sockfd = -1;
	packet->src_ipaddr.af = AF_INET;
	packet->src_ipaddr.ipaddr.ip4addr.s_addr = htonl(INADDR_NONE);
	packet->code = PW_ACCOUNTING_REQUEST;
	gettimeofday(&packet->timestamp, NULL);

	/*
	 *	Remember where it came from, so that we don't
	 *	proxy it to the place it came from...
	 */
	if (data->client_ip.af != AF_UNSPEC) {
		packet->src_ipaddr = data->client_ip;
	}

	vp = pairfind(packet->vps, PW_PACKET_SRC_IP_ADDRESS, 0, TAG_ANY);
	if (vp) {
		packet->src_ipaddr.af = AF_INET;
		packet->src_ipaddr.ipaddr.ip4addr.s_addr = vp->vp_ipaddr;
	} else {
		vp = pairfind(packet->vps, PW_PACKET_SRC_IPV6_ADDRESS, 0, TAG_ANY);
		if (vp) {
			packet->src_ipaddr.af = AF_INET6;
			memcpy(&packet->src_ipaddr.ipaddr.ip6addr,
			       &vp->vp_ipv6addr, sizeof(vp->vp_ipv6addr));
		}
	}

	vp = pairfind(packet->vps, PW_PACKET_DST_IP_ADDRESS, 0, TAG_ANY);
	if (vp) {
		packet->dst_ipaddr.af = AF_INET;
		packet->dst_ipaddr.ipaddr.ip4addr.s_addr = vp->vp_ipaddr;
	} else {
		vp = pairfind(packet->vps, PW_PACKET_DST_IPV6_ADDRESS, 0, TAG_ANY);
		if (vp) {
			packet->dst_ipaddr.af = AF_INET6;
			memcpy(&packet->dst_ipaddr.ipaddr.ip6addr,
			       &vp->vp_ipv6addr, sizeof(vp->vp_ipv6addr));
		}
	}

	/*
	 *	Generate packet ID, ports, IP via a counter.
	 */
	packet->id = data->counter & 0xff;
	packet->src_port = 1024 + ((data->counter >> 8) & 0xff);
	packet->dst_port = 1024 + ((data->counter >> 16) & 0xff);

	packet->dst_ipaddr.af = AF_INET;
	packet->dst_ipaddr.ipaddr.ip4addr.s_addr = htonl((INADDR_LOOPBACK & ~0xffffff) | ((data->counter >> 24) & 0xff));

	/*
	 *	If everything's OK, this is a waste of memory.
	 *	Otherwise, it lets us re-send the original packet
	 *	contents, unmolested.
	 */
	packet->vps = paircopy(packet, data->vps);

	/*
	 *	Prefer the Event-Timestamp in the packet, if it
	 *	exists.  That is when the event occurred, whereas the
	 *	"Timestamp" field is when we wrote the packet to the
	 *	detail file, which could have been much later.
	 */
	vp = pairfind(packet->vps, PW_EVENT_TIMESTAMP, 0, TAG_ANY);
	if (vp) {
		data->timestamp = vp->vp_integer;
	}

	/*
	 *	Look for Acct-Delay-Time, and update
	 *	based on Acct-Delay-Time += (time(NULL) - timestamp)
	 */
	vp = pairfind(packet->vps, PW_ACCT_DELAY_TIME, 0, TAG_ANY);
	if (!vp) {
		vp = paircreate(packet, PW_ACCT_DELAY_TIME, 0);
		rad_assert(vp != NULL);
		pairadd(&packet->vps, vp);
	}
	if (data->timestamp != 0) {
		vp->vp_integer += time(NULL) - data->timestamp;
	}

	/*
	 *	Set the transmission count.
	 */
	vp = pairfind(packet->vps, PW_PACKET_TRANSMIT_COUNTER, 0, TAG_ANY);
	if (!vp) {
		vp = paircreate(packet, PW_PACKET_TRANSMIT_COUNTER, 0);
		rad_assert(vp != NULL);
		pairadd(&packet->vps, vp);
	}
	vp->vp_integer = data->tries;

	if (debug_flag) {
		fr_printf_log("detail_recv: Read packet from %s\n", data->filename_work);
		for (vp = paircursor(&cursor, &packet->vps);
		     vp;
		     vp = pairnext(&cursor)) {
			debug_pair(vp);
		}
	}

	/*
	 *	Don't bother doing limit checks, etc.
	 */
	if (!request_receive(listener, packet, &data->detail_client,
			     rad_accounting)) {
		rad_free(&packet);
		data->state = STATE_NO_REPLY;	/* try again later */
		return 0;
	}

	data->state = STATE_RUNNING;
	data->running = packet->timestamp.tv_sec;

	return 1;
}


/*
 *	Free detail-specific stuff.
 */
void detail_free(rad_listen_t *this)
{
	listen_detail_t *data = this->data;

	talloc_free(data->filename);
	data->filename = NULL;
	pairfree(&data->vps);

	if (data->fp != NULL) {
		fclose(data->fp);
		data->fp = NULL;
	}
}


int detail_print(rad_listen_t const *this, char *buffer, size_t bufsize)
{
	if (!this->server) {
		return snprintf(buffer, bufsize, "%s",
				((listen_detail_t *)(this->data))->filename);
	}

	return snprintf(buffer, bufsize, "detail file %s as server %s",
			((listen_detail_t *)(this->data))->filename,
			this->server);
}

/*
 *	Overloaded to return delay times.
 */
int detail_encode(rad_listen_t *this, UNUSED REQUEST *request)
{
	listen_detail_t *data = this->data;

	/*
	 *	We haven't sent a packet... delay things a bit.
	 */
	if (!data->signal) {
		int delay = (data->poll_interval - 1) * USEC;

		/*
		 *	Add +/- 0.25s of jitter
		 */
		delay += (USEC * 3) / 4;
		delay += fr_rand() % (USEC / 2);

		DEBUG2("Detail listener %s state %s signalled %d waiting %d.%06d sec",
		       data->filename,
		       fr_int2str(state_names, data->state, "?"), data->signal,
		       (delay / USEC), delay % USEC);

		return delay;
	}

	data->signal = 0;

	DEBUG2("Detail listener %s state %s signalled %d waiting %d.%06d sec",
	       data->filename, fr_int2str(state_names, data->state, "?"),
	       data->signal,
	       data->delay_time / USEC,
	       data->delay_time % USEC);

	return data->delay_time;
}


/*
 *	Overloaded to return "should we fix delay times"
 */
int detail_decode(rad_listen_t *this, UNUSED REQUEST *request)
{
	listen_detail_t *data = this->data;

	return data->signal;
}


static const CONF_PARSER detail_config[] = {
	{ "detail",   PW_TYPE_FILE_OUTPUT | PW_TYPE_DEPRECATED,
	  offsetof(listen_detail_t, filename), NULL,  NULL },
	{ "filename",   PW_TYPE_FILE_OUTPUT | PW_TYPE_REQUIRED,
	  offsetof(listen_detail_t, filename), NULL,  NULL },
	{ "load_factor",   PW_TYPE_INTEGER,
	  offsetof(listen_detail_t, load_factor), NULL, STRINGIFY(10)},
	{ "poll_interval",   PW_TYPE_INTEGER,
	  offsetof(listen_detail_t, poll_interval), NULL, STRINGIFY(1)},
	{ "retry_interval",   PW_TYPE_INTEGER,
	  offsetof(listen_detail_t, retry_interval), NULL, STRINGIFY(30)},
	{ "one_shot",   PW_TYPE_BOOLEAN,
	  offsetof(listen_detail_t, one_shot), NULL, NULL},
	{ "max_outstanding",   PW_TYPE_INTEGER,
	  offsetof(listen_detail_t, load_factor), NULL, NULL},

	{ NULL, -1, 0, NULL, NULL }		/* end the list */
};

extern int check_config;

/*
 *	Parse a detail section.
 */
int detail_parse(CONF_SECTION *cs, rad_listen_t *this)
{
	int		rcode;
	listen_detail_t *data;
	RADCLIENT	*client;
	char buffer[2048];

	if (check_config) return 0;

	data = this->data;

	rcode = cf_section_parse(cs, data, detail_config);
	if (rcode < 0) {
		cf_log_err_cs(cs, "Failed parsing listen section");
		return -1;
	}

	/*
	 *	We don't do duplicate detection for "detail" sockets.
	 */
	this->nodup = true;
	this->synchronous = false;

	if (!data->filename) {
		cf_log_err_cs(cs, "No detail file specified in listen section");
		return -1;
	}

	if ((data->load_factor < 1) || (data->load_factor > 100)) {
		cf_log_err_cs(cs, "Load factor must be between 1 and 100");
		return -1;
	}

	if ((data->poll_interval < 1) || (data->poll_interval > 20)) {
		cf_log_err_cs(cs, "poll_interval must be between 1 and 20");
		return -1;
	}

	if (data->max_outstanding == 0) data->max_outstanding = 1;

	/*
	 *	If the filename is a glob, use "detail.work" as the
	 *	work file name.
	 */
	if ((strchr(data->filename, '*') != NULL) ||
	    (strchr(data->filename, '[') != NULL)) {
		char *p;

#ifndef HAVE_GLOB_H
		WARN("Detail file \"%s\" appears to use file globbing, but it is not supported on this system.",
		     data->filename);
#endif
		strlcpy(buffer, data->filename, sizeof(buffer));
		p = strrchr(buffer, FR_DIR_SEP);
		if (p) {
			p[1] = '\0';
		} else {
			buffer[0] = '\0';
		}
		strlcat(buffer, "detail.work",
			sizeof(buffer) - strlen(buffer));

	} else {
		snprintf(buffer, sizeof(buffer), "%s.work", data->filename);
	}

	free(data->filename_work);
	data->filename_work = strdup(buffer); /* FIXME: leaked */

	data->vps = NULL;
	data->fp = NULL;
	data->state = STATE_UNOPENED;
	data->delay_time = data->poll_interval * USEC;
	data->signal = 1;

	/*
	 *	Initialize the fake client.
	 */
	client = &data->detail_client;
	memset(client, 0, sizeof(*client));
	client->ipaddr.af = AF_INET;
	client->ipaddr.ipaddr.ip4addr.s_addr = INADDR_NONE;
	client->prefix = 0;
	client->longname = client->shortname = data->filename;
	client->secret = client->shortname;
	client->nas_type = strdup("none");

	return 0;
}
#endif
