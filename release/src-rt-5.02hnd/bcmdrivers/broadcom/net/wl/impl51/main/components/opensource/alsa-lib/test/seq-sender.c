
#ifdef USE_PCM
/*
 *  PCM timer layer
 */

int pcard = 0;
int pdevice = 0;
int period_size = 1024;

void set_hwparams(snd_pcm_t *phandle)
{
	int err;
	snd_pcm_hw_params_t *params;

	err = snd_output_stdio_attach(&log, stderr, 0);
	if (err < 0) {
		fprintf(stderr, "cannot attach output stdio\n");
		exit(0);
	}

	snd_pcm_hw_params_alloca(&params);
	err = snd_pcm_hw_params_any(phandle, params);
	if (err < 0) {
		fprintf(stderr, "Broken configuration for this PCM: no configurations available\n");
		exit(0);
	}

	err = snd_pcm_hw_params_set_access(phandle, params,
					   SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		fprintf(stderr, "Access type not available\n");
		exit(0);
	}
	err = snd_pcm_hw_params_set_format(phandle, params, SND_PCM_FORMAT_S16_LE);
	if (err < 0) {
		fprintf(stderr, "cannot set format\n");
		exit(0);
	}
	err = snd_pcm_hw_params_set_channels(phandle, params, 2);
	if (err < 0) {
		fprintf(stderr, "cannot set channels 2\n");
		exit(0);
	}
	err = snd_pcm_hw_params_set_rate_near(phandle, params, 44100, 0);
	if (err < 0) {
		fprintf(stderr, "cannot set rate\n");
		exit(0);
	}
	err = snd_pcm_hw_params_set_period_size_near(phandle, params, period_size);
	if (err < 0) {
		fprintf(stderr, "cannot set period size\n");
		exit(0);
	}
	err = snd_pcm_hw_params(phandle, params);
	if (err < 0) {
		fprintf(stderr, "Unable to install hw params:\n");
		exit(0);
	}
	snd_pcm_hw_params_dump(params, log);
}

#endif
/*
 *  Simple event sender
 */

void event_sender_start_timer(snd_seq_t *handle,
			      int client ATTRIBUTE_UNUSED,
			      int queue,
			      snd_pcm_t *phandle ATTRIBUTE_UNUSED)
{
	int err;
	
#ifdef USE_PCM
	if (phandle) {
		snd_pcm_playback_info_t pinfo;
		snd_seq_queue_timer_t qtimer;

		if ((err = snd_pcm_playback_info(phandle, &pinfo)) < 0) {
			fprintf(stderr, "Playback info error: %s\n", snd_strerror(err));
			exit(0);
		}
		bzero(&qtimer, sizeof(qtimer));
		qtimer.type = SND_SEQ_TIMER_MASTER;
		/* note: last bit from the subdevices specifies playback */
		/* or capture direction for the timer specification */
		qtimer.number = SND_TIMER_PCM(pcard, pdevice, pinfo.subdevice << 1);
		if ((err = snd_seq_set_queue_timer(handle, queue, &qtimer)) < 0) {
			fprintf(stderr, "Sequencer PCM timer setup failed: %s\n", snd_strerror(err));
			exit(0);
		}
	}	
#endif
	if ((err = snd_seq_start_queue(handle, queue, NULL))<0)
		fprintf(stderr, "Timer event output error: %s\n", snd_strerror(err));
	snd_seq_drain_output(handle);
}

void event_sender_filter(snd_seq_t *handle)
{
	int err;

	if ((err = snd_seq_set_client_event_filter(handle, SND_SEQ_EVENT_ECHO)) < 0) {
		fprintf(stderr, "Unable to set client info: %s\n", snd_strerror(err));
		return;
	}
}

void send_event(snd_seq_t *handle, int queue, int client, int port,
                snd_seq_addr_t *dest, int *time)
{
	int err;
	snd_seq_event_t ev;
	
	bzero(&ev, sizeof(ev));
	ev.queue = queue;
	ev.source.client = ev.dest.client = client;
	ev.source.port = ev.dest.port = port;
	ev.flags = SND_SEQ_TIME_STAMP_REAL | SND_SEQ_TIME_MODE_ABS;
	ev.time.time.tv_sec = *time; (*time)++;
	ev.type = SND_SEQ_EVENT_ECHO;
	if ((err = snd_seq_event_output(handle, &ev))<0)
		fprintf(stderr, "Event output error: %s\n", snd_strerror(err));
	ev.dest = *dest;
	ev.type = SND_SEQ_EVENT_PGMCHANGE;
	ev.data.control.channel = 0;
	ev.data.control.value = 16;
	if ((err = snd_seq_event_output(handle, &ev))<0)
		fprintf(stderr, "Event output error: %s\n", snd_strerror(err));
	ev.type = SND_SEQ_EVENT_NOTE;
	ev.data.note.channel = 0;
	ev.data.note.note = 64 + (queue*2);
	ev.data.note.velocity = 127;
	ev.data.note.off_velocity = 127;
	ev.data.note.duration = 500;	/* 0.5sec */
	if ((err = snd_seq_event_output(handle, &ev))<0)
		fprintf(stderr, "Event output error: %s\n", snd_strerror(err));
	if ((err = snd_seq_drain_output(handle))<0)
		fprintf(stderr, "Event drain error: %s\n", snd_strerror(err));
}

void event_sender(snd_seq_t *handle, int argc, char *argv[])
{
	snd_seq_event_t *ev;
	snd_seq_port_info_t *pinfo;
	snd_seq_port_subscribe_t *sub;
	snd_seq_addr_t addr;
	struct pollfd *pfds;
	int client, port, queue, max, err, v1, v2, time = 0, pcm_flag = 0;
	char *ptr;
	snd_pcm_t *phandle = NULL;

	if (argc < 1) {
		fprintf(stderr, "Invalid destination...\n");
		return;
	}

	if ((client = snd_seq_client_id(handle))<0) {
		fprintf(stderr, "Cannot determine client number: %s\n", snd_strerror(client));
		return;
	}
	printf("Client ID = %i\n", client);
	if ((queue = snd_seq_alloc_queue(handle))<0) {
		fprintf(stderr, "Cannot allocate queue: %s\n", snd_strerror(queue));
		return;
	}
	printf("Queue ID = %i\n", queue);
	event_sender_filter(handle);
	if ((err = snd_seq_nonblock(handle, 1))<0)
		fprintf(stderr, "Cannot set nonblock mode: %s\n", snd_strerror(err));

	snd_seq_port_info_alloca(&pinfo);
	snd_seq_port_info_set_capability(pinfo, SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_READ);
	snd_seq_port_info_set_name(pinfo, "Output");
	if ((err = snd_seq_create_port(handle, pinfo)) < 0) {
		fprintf(stderr, "Cannot create output port: %s\n", snd_strerror(err));
		return;
	}
	port = snd_seq_port_info_get_port(pinfo);

	snd_seq_port_subscribe_alloca(&sub);
	addr.client = client;
	addr.port = port;
	snd_seq_port_subscribe_set_sender(sub, &addr);

	for (max = 0; max < argc; max++) {
		ptr = argv[max];
		if (!ptr)
			continue;
		if (!strcmp(ptr, "pcm")) {
			pcm_flag = 1;
			continue;
		}
		if (sscanf(ptr, "%i.%i", &v1, &v2) != 2) {
			fprintf(stderr, "Wrong argument '%s'...\n", argv[max]);
			return;
		}
		addr.client = v1;
		addr.port = v2;
		snd_seq_port_subscribe_set_dest(sub, &addr);
		if ((err = snd_seq_subscribe_port(handle, sub))<0) {
			fprintf(stderr, "Cannot subscribe port %i from client %i: %s\n", v2, v1, snd_strerror(err));
			return;
		}
	}

	printf("Destination client = %i, port = %i\n", addr.client, addr.port);

#ifdef USE_PCM
	if (pcm_flag) {
		if ((err = snd_pcm_open(&phandle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
			fprintf(stderr, "Playback open error: %s\n", snd_strerror(err));
			exit(0);
		}
		set_hwparams(phandle);
		pbuf = calloc(1, period_size * 4);
		if (pbuf == NULL) {
			fprintf(stderr, "No enough memory...\n");
			exit(0);
		}
	}
#endif
	event_sender_start_timer(handle, client, queue, phandle);
	
	/* send the first event */
	send_event(handle, queue, client, port, &addr, &time);
#ifdef USE_PCM
	if (phandle)
		max += snd_pcm_poll_descriptors_count(phandle);
#endif
	pfds = alloca(sizeof(*pfds) * max);
	while (1) {
		int nseqs = snd_seq_poll_descriptors_count(handle, POLLOUT|POLLIN);
		if (snd_seq_event_output_pending(handle))
			snd_seq_poll_descriptors(handle, pfds, nseqs, POLLOUT|POLLIN);
		else
			snd_seq_poll_descriptors(handle, pfds, nseqs, POLLIN);
		max = nseqs;
#ifdef USE_PCM
		if (phandle) {
			int pmax = snd_pcm_poll_descriptors_count(phandle);
			snd_seq_poll_descriptors(phandle, pfds + max, pmax);
			max += pmax;
		}
#endif
		if (poll(pfds, max, -1) < 0)
			break;
#ifdef USE_PCM
		if (phandle && (pfds[nseqs].revents & POLLOUT)) {
			if (snd_pcm_writei(phandle, pbuf, period_size) != period_size) {
				fprintf(stderr, "Playback write error!!\n");
				exit(0);
			}
		}
#endif
		if (pfds[0].revents & POLLOUT)
			snd_seq_drain_output(handle);
		if (pfds[0].revents & POLLIN) {
			do {
				if ((err = snd_seq_event_input(handle, &ev))<0)
					break;
				if (!ev)
					continue;
				if (ev->type == SND_SEQ_EVENT_ECHO)
					send_event(handle, queue, client, port, &addr, &time);
				decode_event(ev);
				snd_seq_free_event(ev);
			} while (err > 0);
		}
	}
}
