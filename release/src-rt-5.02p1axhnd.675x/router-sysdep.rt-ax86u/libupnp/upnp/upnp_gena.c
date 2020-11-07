/*
 * Broadcom UPnP library GENA
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: upnp_gena.c 775737 2019-06-12 03:07:33Z $
 */
#include <upnp.h>

/*
 * Variables
 */
static  unsigned short unique_id_count = 1;

/* Get the subscriber chain of the focus interface */
UPNP_SCBRCHAIN *
get_subscriber_chain(UPNP_CONTEXT *context, UPNP_SERVICE *service)
{
	UPNP_SCBRCHAIN	*scbrchain;

	/* Search the subscriber chain of the focused interface */
	for (scbrchain = service->scbrchain;
	     scbrchain;
	     scbrchain = scbrchain->next) {
		if (scbrchain->ifp == context->focus_ifp)
			break;
	}

	return scbrchain;
}

/* Get the evented state variable's value of the focus interface */
UPNP_EVENT *
get_event(UPNP_CONTEXT *context, UPNP_STATE_VAR *state_var)
{
	UPNP_EVENT *event;

	for (event = state_var->event;
	     event;
	     event = event->next) {
		if (event->ifp == context->focus_ifp)
			break;
	}

	return event;
}

/* Search GENA event lists for a target event */
UPNP_STATE_VAR	*
find_event_var(UPNP_CONTEXT *context, UPNP_SERVICE *service, char *name)
{
	UPNP_STATE_VAR *statevar;

	for (statevar = service->event_var_list;
	     statevar;
	     statevar = statevar->next) {
		if (strcmp(statevar->name, name) == 0)
			break;
	}

	return statevar;
}

/* Remove subscriber from list */
void
delete_subscriber(UPNP_SCBRCHAIN *scbrchain, UPNP_SUBSCRIBER *subscriber)
{
	/* Remove from queue */
	if (subscriber->prev) {
		subscriber->prev->next = subscriber->next;
	}
	else {
		/* first node, re-configure the list pointer */
		scbrchain->subscriberlist = subscriber->next;
	}

	if (subscriber->next)
		subscriber->next->prev = subscriber->prev;

	free(subscriber);
	return;
}

/* Parse the header, CALLBACK, to get host address (ip, port) and uri */
char *
parse_callback(UPNP_CONTEXT *context, char *callback, struct in_addr *ipaddr, unsigned short *port)
{
	char *p;
	int pos;
	char host[sizeof("255.255.255.255:65535")];
	char *uri;
	int host_port;
	struct in_addr host_ip;
	UPNP_INTERFACE *iflist;
	bool lan_match=FALSE;

	/* callback: "<http://192.168.2.13:5000/notify>" */
	if (memcmp(callback, "<http://", 8) != 0) {
		/* not a HTTP URL, return NULL pointer */
		return 0;
	}

	/* make <http:/'\0'192.168.2.13:5000/notify'\0' */
	pos = strcspn(callback, ">");
	callback[pos] = 0;

	callback[7] = 0;

	/* Locate uri */
	p = callback + 8;
	pos = strcspn(p, "/");
	if (pos > sizeof(host)-1)
		return 0;

	if (p[pos] != '/')
		uri = callback + 6;
	else
		uri = p + pos;

	strncpy(host, p, pos);
	host[pos] = 0;

	/* make port */
	host_port = 0;

	pos = strcspn(host, ":");
	if (host[pos] == ':')
		host_port = atoi(host + pos + 1);

	if (host_port > 65535)
		return 0;

	if (host_port == 0)
		host_port = 80;

	/* make ip */
	host[pos] = 0;
	if (inet_aton(host, &host_ip) == 0) {
#ifdef DBG_CODE
		printf("%s:%d: inet_aton failed for %s\n", __FUNCTION__, __LINE__, host);
#endif
		return 0;
	}

	iflist = context->iflist;
	while (iflist != NULL) {
#ifdef DBG_CODE
		char *s_ipaddr;
		char *s_netmask;
		s_ipaddr = strdup(inet_ntoa(iflist->ipaddr));
		s_netmask = strdup(inet_ntoa(iflist->netmask));
		printf("ifname = %s  ipaddr = %s  netmask = %s\n",
			iflist->ifname, s_ipaddr, s_netmask);
		free(s_ipaddr);
		free(s_netmask);
#endif

		/* Check if callback URL is in the same subnet of registered LAN interfaces */
		if ((host_ip.s_addr & iflist->netmask.s_addr) ==
				(iflist->ipaddr.s_addr & iflist->netmask.s_addr)) {
			lan_match = TRUE;
			break;
		}

		iflist = iflist->next;
	}

	if (lan_match != TRUE) {
#ifdef DBG_CODE
		printf("%s:%d: lan_match failed for %s\n", __FUNCTION__, __LINE__, host);
#endif
		return 0;
	}

	*ipaddr = host_ip;
	*port = host_port;
	return uri;
}

/* Consume the input buffer after sending the notification */
static int
gena_read_sock(int s, char *buf, int len, int flags)
{
	long     rc;
	fd_set   ibits;
	struct   timeval tv = {1, 0};   /* wait for at most 1 seconds */
	int      n_read;

	FD_ZERO(&ibits);
	FD_SET(s, &ibits);

	rc = select(s+1, &ibits, 0, 0, &tv);
	if (rc <= 0)
		return -1;

	if (FD_ISSET(s, &ibits)) {
		n_read = recv(s, buf, len, flags);
		return n_read;
	}

	return -1;
}

/* Send out property event changes message */
int
notify_prop_change(UPNP_CONTEXT *context, UPNP_SUBSCRIBER *subscriber)
{
	struct sockaddr_in sin;

	int s;
	int len;
	UPNP_DEVICE *device = context->focus_ifp->device;
	int connect_retries = device->gena_connect_retries;
	int err = -1;
	int reuse = 1;
	int flags = 0;

	if (connect_retries == 0 || connect_retries > 20)
		connect_retries = 20;

	/* open socket */
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		upnp_syslog(LOG_ERR, "Cannot open socket!");
		goto error_out;
	}

#if defined(linux)
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		upnp_syslog(LOG_ERR, "Cannot set socket option (SO_REUSEADDR)");
	}
#endif // endif

	/* set non-blocking IO to have connect() return without pending */
	flags = fcntl(s, F_GETFL, 0);
	if (flags == -1) {
		upnp_syslog(LOG_ERR, "fcntl(): F_GETFL read file status failed");
		goto error_out;
	}

	if ((fcntl(s, F_SETFL, flags | O_NONBLOCK)) == -1) {
		upnp_syslog(LOG_ERR, "fcntl(): cannot set socket as non-blocking IO");
		goto error_out;
	}

	/* Fill socket address to connect */
	memset(&sin, 0, sizeof(sin));

	sin.sin_family = AF_INET;
	sin.sin_port = htons(subscriber->port);
	sin.sin_addr = subscriber->ipaddr;

	/* connect */
	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		struct timeval tv;
		fd_set fds;
		int i;

		FD_ZERO(&fds);
		FD_SET(s, &fds);

		/* timeout after device maximum retries */
		for (i = 0; i < connect_retries; i++) {
			tv.tv_sec = 0;
			tv.tv_usec = 500000;
			if (select(s+1, 0, &fds, 0, &tv) > 0) {
				if (FD_ISSET(s, &fds))
					break;
			}
		}

		/* Reach maximum waits, go out */
		if (i == connect_retries) {
			upnp_syslog(LOG_ERR, "Notify connect failed!");
			goto error_out;
		}
	}

	/* clear non-blocking IO to have connect() return without pending */
	flags = fcntl(s, F_GETFL, 0);
	if (flags == -1) {
		upnp_syslog(LOG_ERR, "fcntl(): F_GETFL read file status failed");
		goto error_out;
	}

	if ((fcntl(s, F_SETFL, flags & ~O_NONBLOCK)) == -1) {
		upnp_syslog(LOG_ERR, "fcntl(): cannot clear socket as blocking IO");
		goto error_out;
	}

	/* Send message out */
	len = strlen(context->head_buffer);
	if (send(s, context->head_buffer, len, 0) == -1) {
		upnp_syslog(LOG_ERR, "Notify failed");
		goto error_out;
	}

	/* try to read response */
	gena_read_sock(s, context->head_buffer, GENA_MAX_BODY, 0);

	/* no error */
	err = 0;

error_out:
	if (s >= 0)
		close(s);

	return err;
}

/* Construct property event changes message */
int
submit_prop_event_message(UPNP_CONTEXT *context,
	UPNP_SERVICE *service, UPNP_SUBSCRIBER *subscriber)
{
	UPNP_STATE_VAR *statevar = service->event_var_list;

	unsigned char host[sizeof("255.255.255.255:65535") + 1];
	char *p;
	int len;
	int err;

	/* construct body */
	p = context->body_buffer;
	len = snprintf(context->body_buffer, sizeof(context->body_buffer),
		"<e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\">\r\n");
	p += len;

	for (statevar = service->event_var_list;
	     statevar;
	     statevar = statevar->next) {
		UPNP_EVENT *event;

		event = get_event(context, statevar);
		if (event == 0)
			continue;

		/* If had not been retrieved, get the variable */
		if (event->init == FALSE) {
			if ((*statevar->func)(context, service, &event->tlv) == 0) {
				event->init = TRUE;
			}
		}

		/* Translate the duplicated value to output string */
		if (upnp_tlv_translate(&event->tlv) == NULL)
			continue;

		/* new subscription */
		if (subscriber->seq == 0) {
			len = snprintf(p, sizeof(context->body_buffer) - len,
				"<e:property><%s>%s</%s></e:property>\r\n",
				statevar->name,
				event->tlv.text,
				statevar->name);
			p += len;
		}
		else if (event->notify) {
			/* state changed */
			len = snprintf(p, sizeof(context->body_buffer) - len,
				"<e:property><%s>%s</%s></e:property>\r\n",
				statevar->name,
				event->tlv.text,
				statevar->name);
			p += len;
		}
	}

	strcat(context->body_buffer, "</e:propertyset>\r\n\r\n");

	/* construct header */
	/* make host */
	upnp_host_addr(host, subscriber->ipaddr, subscriber->port);

	/* Make notify string */
	snprintf(context->head_buffer, sizeof(context->head_buffer),
		"NOTIFY %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Content-Type: text/xml\r\n"
		"Content-Length: %d\r\n"
		"NT: upnp:event\r\n"
		"NTS: upnp:propchange\r\n"
		"SID: %s\r\n"
		"SEQ: %d\r\n"
		"Connection: close\r\n\r\n"
		"%s",
		subscriber->uri,
		host,
		(int)strlen(context->body_buffer),
		subscriber->sid,
		subscriber->seq,
		context->body_buffer);

	/* Send out */
	err = notify_prop_change(context, subscriber);

	return err;
}

/* Submit property events to subscribers */
void
gena_notify(UPNP_CONTEXT *context, UPNP_SERVICE *service, char *sid, char *ipaddr)
{
	UPNP_SCBRCHAIN	*scbrchain;
	UPNP_SUBSCRIBER	*subscriber;
	struct in_addr addr;

	/* walk through the subscribers */
	scbrchain = get_subscriber_chain(context, service);
	if (scbrchain == 0)
		return;

	/* Get specific peer to notify */
	if (ipaddr)
		inet_aton(ipaddr, &addr);

	/* Loop for all the subscriber list to send notify */
	subscriber = scbrchain->subscriberlist;
	while (subscriber) {
		if (!ipaddr || subscriber->ipaddr.s_addr == addr.s_addr) {
			/* If the sid is given, only send this one out */
			if (sid) {
				/* for specific URL */
				if (strcmp(sid, subscriber->sid) == 0) {
					submit_prop_event_message(
						context, service, subscriber);
					subscriber->seq++;
					break;
				}
			}
			else {
				/* for all */
				submit_prop_event_message(
					context, service, subscriber);
				subscriber->seq++;
			}
		}

		subscriber = subscriber->next;
	}

	return;
}

/* Completion function after gena_notify */
void
gena_notify_complete(UPNP_CONTEXT *context, UPNP_SERVICE *service)
{
	UPNP_STATE_VAR *statevar;
	UPNP_EVENT *event;

	for (statevar = service->event_var_list;
	     statevar;
	     statevar = statevar->next) {
		event = get_event(context, statevar);

		/*
		 * After notification complete,
		 * we should reset all the changed flag to false.
		 */
		if (event && event->notify == TRUE)
			event->notify = FALSE;
	}

	service->evented = FALSE;
	return;
}

/* GENA unusubscription process routine */
int
unsubscribe(UPNP_CONTEXT *context)
{
	UPNP_SERVICE *service;
	UPNP_SUBSCRIBER *subscriber;
	UPNP_SCBRCHAIN *scbrchain;
	UPNP_DEVICE *device;

	char *gena_sid = upnp_msg_get(context, "SID");

	/* Should not happen */
	if (!gena_sid)
		return R_PRECONDITION_FAIL;

	/* find event */
	service = upnp_get_service_by_event_url(context, context->url);
	if (service == 0)
		return R_NOT_FOUND;

	/* find SID */
	scbrchain = get_subscriber_chain(context, service);
	if (scbrchain == 0)
		return R_NOT_FOUND;

	/* Search in the subscriber list to find the matched SID */
	subscriber = scbrchain->subscriberlist;
	while (subscriber) {
		if (strcmp(subscriber->sid, gena_sid) == 0)
			break;

		subscriber = subscriber->next;
	}

	if (subscriber == 0)
		return R_PRECONDITION_FAIL;

	/* service dependent things */
	device = context->focus_ifp->device;
	if (device->notify)
		(*device->notify)(context, service, subscriber, DEVICE_NOTIFY_UNSUBSCRIBE);

	/* Delete this subscriber */
	delete_subscriber(scbrchain, subscriber);

	/* send reply */
	strcpy(context->head_buffer,
		"HTTP/1.1 200 OK\r\n"
		"Connection: close\r\n"
		"\r\n");

	send(context->fd, context->head_buffer, strlen(context->head_buffer), 0);

	return 0;
}

/* Get a unique id string */
int
get_unique_id(char *unique_id, unsigned int size)
{
	unsigned int curr_time;
	unsigned short id = ++unique_id_count;
	unsigned int pid = (unsigned int)upnp_pid();

	if (size < 36)
		return FALSE;

	/* get current time */
	curr_time = (unsigned int)time(0);

	/*
	 * use uuid format is more friendly, the wpa_supplicant control point
	 * always convert sid to a uuid format.
	 */
	snprintf(unique_id, size, "%08x-%04x-%04x-%04x-%04x%08x",
		curr_time, pid, pid, pid, pid, (unsigned int)id);

	return TRUE;
}

/* GENA subscription process routine */
int
subscribe(UPNP_CONTEXT *context)
{
	UPNP_SERVICE *service;
	UPNP_SUBSCRIBER *subscriber;
	UPNP_SCBRCHAIN *scbrchain;
	UPNP_DEVICE *device = context->focus_ifp->device;

	int infinite = FALSE;
	int interval = 3600;
	time_t now;
	char time_buf[TIME_BUF_LEN];
	char timeout[64];

	char *gena_timeout = upnp_msg_get(context, "TIMEOUT");
	char *gena_sid = upnp_msg_get(context, "SID");
	char *gena_callback = upnp_msg_get(context, "CALLBACK");

	/* find event */
	service = upnp_get_service_by_event_url(context, context->url);
	if (service == 0)
		return R_NOT_FOUND;

	/* process subscription time interval */
	if (gena_timeout) {
		char *ptr;

		/*
		 * If the header, TIMEOUT, is given,
		 * the value should begin with "Second-".
		 */
		if (memcmp(gena_timeout, "Second-", 7) != 0)
			return R_PRECONDITION_FAIL;

		/* "infinite" means always no timed-out */
		ptr = gena_timeout + 7;
		if (strcmp(ptr, "infinite") == 0) {
			infinite = TRUE;
		}
		else {
			/* Convert the value to subscriber time */
			interval = atoi(ptr);
			if (interval == 0) {
				interval = 3600;
			}
		}
	}
	else {
		/* No TIMEOUT header, use the default value */
		snprintf(timeout, sizeof(timeout), "Second-%d", 3600);
		gena_timeout = timeout;
	}

	/*
	 * process SID and Callback
	 */
	scbrchain = get_subscriber_chain(context, service);
	if (scbrchain == 0)
		return R_NOT_FOUND;

	/* new subscription */
	if (gena_sid == 0) {
		struct in_addr ipaddr;
		unsigned short port;
		char *uri;
		int len;
		int scbrchk = 0;

		/* Check if callback assigned */
		if (gena_callback == 0)
			return R_ERROR;

		uri = parse_callback(context, gena_callback, &ipaddr, &port);
		if (uri == 0)
			return R_BAD_REQUEST;

		/* Find exist subscriber and free it */
		subscriber = scbrchain->subscriberlist;
		while (subscriber) {
			if (device->scbrchk) {
				scbrchk = (*device->scbrchk)(context, service, subscriber,
				                             ipaddr, port, uri);
			}
			else {
				/* Default qualify rule */
				if (subscriber->ipaddr.s_addr == ipaddr.s_addr &&
					subscriber->port == port &&
					strcmp(subscriber->uri, uri) == 0)
					scbrchk = 1;
			}

			if (scbrchk) {
				delete_subscriber(scbrchain, subscriber);
				break;
			}

			subscriber = subscriber->next;
		}

		/*
		 * There may be multiple subscribers for the same
		 * callback, create a new subscriber anyway.
		 */
		len = sizeof(*subscriber) + strlen(uri) + 1;
		subscriber = (UPNP_SUBSCRIBER *)calloc(1, len);
		if (subscriber == 0)
			return R_ERROR;

		/* Setup subscriber data */
		subscriber->ipaddr = ipaddr;
		subscriber->port = port;

		subscriber->uri = (char *)(subscriber + 1);
		strcpy(subscriber->uri, uri);

		strcpy(subscriber->sid, "uuid:");
		get_unique_id(subscriber->sid+5, sizeof(subscriber->sid)-5-1);

		/* insert queue */
		subscriber->next = scbrchain->subscriberlist;
		if (scbrchain->subscriberlist)
			scbrchain->subscriberlist->prev = subscriber;

		scbrchain->subscriberlist = subscriber;

		/* set sequence number */
		subscriber->seq = 0;
	}
	else {
		/*
		 * This is the case, the subscriber wants to
		 * extend the subscription time.
		 */
		subscriber = scbrchain->subscriberlist;
		while (subscriber) {
			if (strcmp(subscriber->sid, gena_sid) == 0)
				break;

			subscriber = subscriber->next;
		}

		if (subscriber == 0)
			return R_PRECONDITION_FAIL;
	}

	/* update expiration time */
	if (infinite) {
		subscriber->expire_time = 0;
	}
	else {
		now = time(0);

		subscriber->expire_time = now + interval;
		if (subscriber->expire_time == 0)
			subscriber->expire_time = 1;
	}

	/* send reply */
	upnp_gmt_time(time_buf);

	snprintf(context->head_buffer, sizeof(context->head_buffer),
		"HTTP/1.1 200 OK\r\n"
		"Server: POSIX, UPnP/1.0 %s/%s\r\n"
		"Date: %s\r\n"
		"SID: %s\r\n"
		"Timeout: %s\r\n"
		"Connection: close\r\n"
		"\r\n",
		"UPnP Stack",
		UPNP_VERSION_STR,
		time_buf,
		subscriber->sid,
		gena_timeout);

	send(context->fd, context->head_buffer, strlen(context->head_buffer), 0);

	/*
	 * send initial property change notifications,
	 * if it is the first subscription
	 */
	if (subscriber->seq == 0)
		gena_notify(context, service, subscriber->sid, NULL);

	/* service dependent things */
	if (device->notify)
		(*device->notify)(context, service, subscriber, DEVICE_NOTIFY_SUBSCRIBE);

	return 0;
}

/* GENA process entry */
int
gena_process(UPNP_CONTEXT *context)
{
	char *nt = upnp_msg_get(context, "NT");
	char *sid = upnp_msg_get(context, "SID");
	char *callback = upnp_msg_get(context, "CALLBACK");

	/* Process subscribe request */
	if (context->method == METHOD_SUBSCRIBE) {
		/*
		 * CALLBACK+NT and SID are mutual exclusive,
		 * when the SID is not given, NT and CALLBACK should
		 * be both existent.
		 */
		if (sid == 0) {
			if (nt == 0 || strcmp(nt, "upnp:event") != 0 || callback == 0)
				return R_BAD_REQUEST;
		}
		else {
			/* Got SID, NT and CALLBACK must be null */
			if (nt || callback)
				return R_BAD_REQUEST;
		}

		return subscribe(context);
	}
	else {
		/*
		 * Process unsubscribe request.
		 * We must have SID, meanwhile the NT and CALLBACK
		 * must be null.
		 */
		if (sid == 0) {
			return R_PRECONDITION_FAIL;
		}
		else {
			if (nt || callback)
				return R_BAD_REQUEST;
		}

		return unsubscribe(context);
	}
}

/* Check and remove expired subscribers */
void
gena_timeout(UPNP_CONTEXT *context)
{
	UPNP_INTERFACE	*ifp = context->focus_ifp;
	UPNP_SERVICE	*service;
	UPNP_SUBSCRIBER *subscriber;
	UPNP_SUBSCRIBER *temp;
	UPNP_SCBRCHAIN	*scbrchain;
	time_t now;

	now = time(0);

	/*
	 * Check all services of the focus device
	 * with the give interface.
	 */
	for (service = ifp->device->service_table;
	     service && service->event_url;
	     service++) {
		/* Find the subscriber list of this service */
		scbrchain = get_subscriber_chain(context, service);
		if (scbrchain == 0)
			continue;

		/* Check expiration */
		subscriber = scbrchain->subscriberlist;
		while (subscriber) {
			temp = subscriber->next;

			/* If timed-out, remove this subscriber */
			if (subscriber->expire_time &&
				(now > subscriber->expire_time)) {
				/* Special patch for NTP */
				if ((now - context->upnp_last_time) > 631123200)
					subscriber->expire_time += now - context->upnp_last_time;
				else {
					if (ifp->device->notify) {
						(*ifp->device->notify)(context, service, subscriber,
							DEVICE_NOTIFY_TIMEOUT);
					}

					delete_subscriber(scbrchain, subscriber);
				}
			}

			subscriber = temp;
		}
	}
}

/* Alarm function called by UPnP IPC functions */
void
gena_event_alarm(UPNP_CONTEXT *context, char *service_name,
	int num, char *headers[], char *ipaddr)
{
	UPNP_SERVICE *service;
	char *name, *value;
	UPNP_STATE_VAR *statevar;
	UPNP_EVENT *event;
	int i;

	service = upnp_get_service_by_name(context, service_name);
	if (service == 0)
		return;

	/* Update the specific state variable */
	if (num != 0) {
		for (i = 0; i < num; i++) {
			name = headers[i];
			value = strchr(name, '=');
			if (!value)
				continue;

			*value++ = 0;

			/* Find the statevar */
			statevar = find_event_var(context, service, name);
			if (statevar == 0)
				continue;

			event = get_event(context, statevar);
			if (event == 0)
				continue;

			/* Update event variable */
			if (upnp_tlv_convert(&event->tlv, value) != 0)
				continue;

			event->init = TRUE;
			event->notify = TRUE;
		}
	}
	else {
		/* Reset the state variables to unread state */
		for (statevar = service->event_var_list;
		     statevar;
		     statevar = statevar->next) {

			event = get_event(context, statevar);
			if (event == 0)
				continue;

			/* Reset to unread state */
			event->init = FALSE;
			event->notify = TRUE;
		}
	}

	/* Notify the subscriber that state variable changed */
	gena_notify(context, service, 0, ipaddr);
	gena_notify_complete(context, service);
	return;
}

/* Initialize the subscriber chain */
void
subscriber_init(UPNP_CONTEXT *context, UPNP_SERVICE *service)
{
	UPNP_SCBRCHAIN	*scbrchain;

	/* Check if the interface subscriber initialized */
	scbrchain = get_subscriber_chain(context, service);
	if (scbrchain) {
		upnp_syslog(LOG_INFO, "service %s - sschain not clean", service->name);
		return;
	}

	/* Make a new one */
	scbrchain = (UPNP_SCBRCHAIN *)malloc(sizeof(*scbrchain));
	if (scbrchain == 0)
		return;

	scbrchain->ifp = context->focus_ifp;
	scbrchain->subscriberlist = 0;

	/* Prepend to the original chain */
	scbrchain->next = service->scbrchain;
	service->scbrchain = scbrchain;
	return;
}

/* Clear all the the subscribers */
void
subscriber_shutdown(UPNP_CONTEXT *context, UPNP_SERVICE *service)
{
	UPNP_SCBRCHAIN	*scbrchain, *prev;

	/* Find the sscbr_chain */
	for (prev = 0, scbrchain = service->scbrchain;
	     scbrchain;
	     prev = scbrchain, scbrchain = scbrchain->next) {

		if (scbrchain->ifp == context->focus_ifp)
			break;
	}

	if (scbrchain == 0)
		return;

	/* Free subscribers */
	while (scbrchain->subscriberlist)
		delete_subscriber(scbrchain, scbrchain->subscriberlist);

	/* Free this chain */
	if (prev == 0)
		service->scbrchain = scbrchain->next;
	else
		prev->next = scbrchain->next;

	free(scbrchain);
	return;
}

/* Initialize the Gena event list */
void
event_vars_init(UPNP_CONTEXT *context, UPNP_SERVICE *service)
{
	UPNP_STATE_VAR *statevar, *tail;

	/* Chain all the evented variable together */
	if (service->event_var_list == 0) {
		for (tail = 0, statevar = service->statevar_table;
		     statevar->name;
		     statevar++) {
			/* Do event variable prepend */
			if (statevar->eflag) {
				statevar->next = 0;

				/* link to tail */
				if (tail == 0)
					service->event_var_list = statevar;
				else
					tail->next = statevar;

				tail = statevar;
			}
		}

		if (tail == 0)
			return;
	}

	/* Intialize event for the focus interface */
	for (statevar = service->event_var_list;
	     statevar;
	     statevar = statevar->next) {

		UPNP_EVENT *event;

		event = get_event(context, statevar);
		if (event) {
			upnp_syslog(LOG_INFO, "service %s - statevar %s is not clean",
				service->name, statevar->name);
			continue;
		}

		event = (struct upnp_event *)malloc(sizeof(*event));
		if (event == 0)
			return;

		memset(event, 0, sizeof(*event));

		event->ifp = context->focus_ifp;
		event->init = FALSE;
		event->notify = FALSE;

		/* Init value */
		upnp_tlv_init(&event->tlv, statevar->type);

		/* Prepend this value */
		event->next = statevar->event;
		statevar->event = event;
	}

	return;
}

/* Clear the Gena event list */
void
event_vars_shutdown(UPNP_CONTEXT *context, UPNP_SERVICE *service)
{
	UPNP_STATE_VAR *statevar;

	for (statevar = service->event_var_list;
		 statevar;
		 statevar = statevar->next)
	{
		UPNP_EVENT *event;
		UPNP_EVENT *prev, *next;
		prev = 0;
		event = statevar->event;
		while (event) {
			next = event->next;

			if (event->ifp == context->focus_ifp) {
				if (prev == 0)
					statevar->event = event->next;
				else
					prev->next = event->next;

				/* Free evented tlv if any */
				upnp_tlv_deinit(&event->tlv);

				free(event);
			}
			else {
				prev = event;
			}
			event = next;
		}
	}

	return;
}

/* Initialize GENA subscribers and event sources */
int
gena_init(UPNP_CONTEXT *context)
{
	UPNP_SERVICE	*service;

	for (service = context->focus_ifp->device->service_table;
	     service && service->event_url;
	     service++) {
		event_vars_init(context, service);
		subscriber_init(context, service);
	}

	return 0;
}

/* Do GEAN subscribers and event variables clean up */
int
gena_shutdown(UPNP_CONTEXT *context)
{
	UPNP_SERVICE	*service;

	for (service = context->focus_ifp->device->service_table;
	     service && service->event_url;
	     service++) {
		subscriber_shutdown(context, service);
		event_vars_shutdown(context, service);
	}

	return 0;
}
