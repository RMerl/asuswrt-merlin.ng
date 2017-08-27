/*
 *  Simple event decoder
 */

static char *event_names[256] = {
	[SND_SEQ_EVENT_SYSTEM]=	"System",
	[SND_SEQ_EVENT_RESULT]=	"Result",
	[SND_SEQ_EVENT_NOTE]=	"Note",
	[SND_SEQ_EVENT_NOTEON]=	"Note On",
	[SND_SEQ_EVENT_NOTEOFF]=	"Note Off",
	[SND_SEQ_EVENT_KEYPRESS]=	"Key Pressure",
	[SND_SEQ_EVENT_CONTROLLER]=	"Controller",
	[SND_SEQ_EVENT_PGMCHANGE]=	"Program Change",
	[SND_SEQ_EVENT_CHANPRESS]=	"Channel Pressure",
	[SND_SEQ_EVENT_PITCHBEND]=	"Pitchbend",
	[SND_SEQ_EVENT_CONTROL14]=	"Control14",
	[SND_SEQ_EVENT_NONREGPARAM]=	"Nonregparam",
	[SND_SEQ_EVENT_REGPARAM]=		"Regparam",
	[SND_SEQ_EVENT_SONGPOS]=	"Song Position",
	[SND_SEQ_EVENT_SONGSEL]=	"Song Select",
	[SND_SEQ_EVENT_QFRAME]=	"Qframe",
	[SND_SEQ_EVENT_TIMESIGN]=	"SMF Time Signature",
	[SND_SEQ_EVENT_KEYSIGN]=	"SMF Key Signature",
	[SND_SEQ_EVENT_START]=	"Start",
	[SND_SEQ_EVENT_CONTINUE]=	"Continue",
	[SND_SEQ_EVENT_STOP]=	"Stop",
	[SND_SEQ_EVENT_SETPOS_TICK]=	"Set Position Tick",
	[SND_SEQ_EVENT_SETPOS_TIME]=	"Set Position Time",
	[SND_SEQ_EVENT_TEMPO]=	"Tempo",
	[SND_SEQ_EVENT_CLOCK]=	"Clock",
	[SND_SEQ_EVENT_TICK]=	"Tick",
	[SND_SEQ_EVENT_TUNE_REQUEST]=	"Tune Request",
	[SND_SEQ_EVENT_RESET]=	"Reset",
	[SND_SEQ_EVENT_SENSING]=	"Active Sensing",
	[SND_SEQ_EVENT_ECHO]=	"Echo",
	[SND_SEQ_EVENT_OSS]=	"OSS",
	[SND_SEQ_EVENT_CLIENT_START]=	"Client Start",
	[SND_SEQ_EVENT_CLIENT_EXIT]=	"Client Exit",
	[SND_SEQ_EVENT_CLIENT_CHANGE]=	"Client Change",
	[SND_SEQ_EVENT_PORT_START]=	"Port Start",
	[SND_SEQ_EVENT_PORT_EXIT]=	"Port Exit",
	[SND_SEQ_EVENT_PORT_CHANGE]=	"Port Change",
	[SND_SEQ_EVENT_PORT_SUBSCRIBED]=	"Port Subscribed",
	[SND_SEQ_EVENT_PORT_UNSUBSCRIBED]=	"Port Unsubscribed",
	[SND_SEQ_EVENT_USR0]=	"User 0",
	[SND_SEQ_EVENT_USR1]=	"User 1",
	[SND_SEQ_EVENT_USR2]=	"User 2",
	[SND_SEQ_EVENT_USR3]=	"User 3",
	[SND_SEQ_EVENT_USR4]=	"User 4",
	[SND_SEQ_EVENT_USR5]=	"User 5",
	[SND_SEQ_EVENT_USR6]=	"User 6",
	[SND_SEQ_EVENT_USR7]=	"User 7",
	[SND_SEQ_EVENT_USR8]=	"User 8",
	[SND_SEQ_EVENT_USR9]=	"User 9",
	[SND_SEQ_EVENT_SYSEX]=	"Sysex",
	[SND_SEQ_EVENT_BOUNCE]=	"Bounce",
	[SND_SEQ_EVENT_USR_VAR0]=	"User Var0",
	[SND_SEQ_EVENT_USR_VAR1]=	"User Var1",
	[SND_SEQ_EVENT_USR_VAR2]=	"User Var2",
	[SND_SEQ_EVENT_USR_VAR3]=	"User Var3",
	[SND_SEQ_EVENT_USR_VAR4]=	"User Var4",
	[SND_SEQ_EVENT_NONE]=	"None",
};

int decode_event(snd_seq_event_t * ev)
{
	char *space = "         ";

	printf("EVENT>>> Type = %d, flags = 0x%x", ev->type, ev->flags);
	switch (ev->flags & SND_SEQ_TIME_STAMP_MASK) {
	case SND_SEQ_TIME_STAMP_TICK:
		printf(", time = %d ticks",
		       ev->time.tick);
		break;
	case SND_SEQ_TIME_STAMP_REAL:
		printf(", time = %d.%09d",
		       (int)ev->time.time.tv_sec,
		       (int)ev->time.time.tv_nsec);
		break;
	}
	printf("\n%sSource = %d.%d, dest = %d.%d, queue = %d\n",
	       space,
	       ev->source.client,
	       ev->source.port,
	       ev->dest.client,
	       ev->dest.port,
	       ev->queue);

	if (event_names[ev->type])
		printf("%sEvent = %s", space, event_names[ev->type]);
	else
		printf("%sEvent = Reserved %d\n", space, ev->type);
	/* decode the actual event data... */
	switch (ev->type) {
	case SND_SEQ_EVENT_NOTE:
		printf("; ch=%d, note=%d, velocity=%d, off_velocity=%d, duration=%d\n",
		       ev->data.note.channel,
		       ev->data.note.note,
		       ev->data.note.velocity,
		       ev->data.note.off_velocity,
		       ev->data.note.duration);
		break;

	case SND_SEQ_EVENT_NOTEON:
	case SND_SEQ_EVENT_NOTEOFF:
	case SND_SEQ_EVENT_KEYPRESS:
		printf("; ch=%d, note=%d, velocity=%d\n",
		       ev->data.note.channel,
		       ev->data.note.note,
		       ev->data.note.velocity);
		break;
		
	case SND_SEQ_EVENT_CONTROLLER:
		printf("; ch=%d, param=%i, value=%i\n",
		       ev->data.control.channel,
		       ev->data.control.param,
		       ev->data.control.value);
		break;

	case SND_SEQ_EVENT_PGMCHANGE:
		printf("; ch=%d, program=%i\n",
		       ev->data.control.channel,
		       ev->data.control.value);
		break;
			
	case SND_SEQ_EVENT_CHANPRESS:
	case SND_SEQ_EVENT_PITCHBEND:
		printf("; ch=%d, value=%i\n",
		       ev->data.control.channel,
		       ev->data.control.value);
		break;
			
	case SND_SEQ_EVENT_SYSEX:
		{
			unsigned char *sysex = (unsigned char *) ev + sizeof(snd_seq_event_t);
			unsigned int c;
			
			printf("; len=%d [", ev->data.ext.len);
			
			for (c = 0; c < ev->data.ext.len; c++) {
				printf("%02x%s", sysex[c], c < ev->data.ext.len - 1 ? ":" : "");
			}
			printf("]\n");
		}
		break;
			
	case SND_SEQ_EVENT_QFRAME:
		printf("; frame=0x%02x\n", ev->data.control.value);
		break;
		
	case SND_SEQ_EVENT_CLOCK:
	case SND_SEQ_EVENT_START:
	case SND_SEQ_EVENT_CONTINUE:
	case SND_SEQ_EVENT_STOP:
		printf("; queue = %i\n", ev->data.queue.queue);
		break;

	case SND_SEQ_EVENT_SENSING:
		printf("\n");
		break;

	case SND_SEQ_EVENT_ECHO:
		{
			int i;
				
			printf("; ");
			for (i = 0; i < 8; i++) {
				printf("%02i%s", ev->data.raw8.d[i], i < 7 ? ":" : "\n");
			}
		}
		break;
			
	case SND_SEQ_EVENT_CLIENT_START:
	case SND_SEQ_EVENT_CLIENT_EXIT:
	case SND_SEQ_EVENT_CLIENT_CHANGE:
		printf("; client=%i\n", ev->data.addr.client);
		break;

	case SND_SEQ_EVENT_PORT_START:
	case SND_SEQ_EVENT_PORT_EXIT:
	case SND_SEQ_EVENT_PORT_CHANGE:
		printf("; client=%i, port = %i\n", ev->data.addr.client, ev->data.addr.port);
		break;

	case SND_SEQ_EVENT_PORT_SUBSCRIBED:
	case SND_SEQ_EVENT_PORT_UNSUBSCRIBED:
		printf("; %i:%i -> %i:%i\n",
		       ev->data.connect.sender.client, ev->data.connect.sender.port,
		       ev->data.connect.dest.client, ev->data.connect.dest.port);
		break;

	default:
		printf("; not implemented\n");
	}


	switch (ev->flags & SND_SEQ_EVENT_LENGTH_MASK) {
	case SND_SEQ_EVENT_LENGTH_FIXED:
		return sizeof(snd_seq_event_t);

	case SND_SEQ_EVENT_LENGTH_VARIABLE:
		return sizeof(snd_seq_event_t) + ev->data.ext.len;
	}

	return 0;
}

void event_decoder_start_timer(snd_seq_t *handle, int queue,
			       int client ATTRIBUTE_UNUSED,
			       int port ATTRIBUTE_UNUSED)
{
	int err;

	if ((err = snd_seq_start_queue(handle, queue, NULL))<0)
		fprintf(stderr, "Timer event output error: %s\n", snd_strerror(err));
	while (snd_seq_drain_output(handle)>0)
		sleep(1);
}

void event_decoder(snd_seq_t *handle, int argc, char *argv[])
{
	snd_seq_event_t *ev;
	snd_seq_port_info_t *pinfo;
	snd_seq_port_subscribe_t *sub;
	snd_seq_addr_t addr;
	int client, port, queue, max, err, v1, v2;
	char *ptr;
	struct pollfd *pfds;

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
	if ((err = snd_seq_nonblock(handle, 1))<0)
		fprintf(stderr, "Cannot set nonblock mode: %s\n", snd_strerror(err));
	snd_seq_port_info_alloca(&pinfo);
	snd_seq_port_info_set_name(pinfo, "Input");
	snd_seq_port_info_set_type(pinfo, SND_SEQ_PORT_TYPE_MIDI_GENERIC);
	snd_seq_port_info_set_capability(pinfo, SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_WRITE);
	if ((err = snd_seq_create_port(handle, pinfo)) < 0) {
		fprintf(stderr, "Cannot create input port: %s\n", snd_strerror(err));
		return;
	}
	port = snd_seq_port_info_get_port(pinfo);
	event_decoder_start_timer(handle, queue, client, port);

	snd_seq_port_subscribe_alloca(&sub);
	addr.client = SND_SEQ_CLIENT_SYSTEM;
	addr.port = SND_SEQ_PORT_SYSTEM_ANNOUNCE;
	snd_seq_port_subscribe_set_sender(sub, &addr);
	addr.client = client;
	addr.port = port;
	snd_seq_port_subscribe_set_dest(sub, &addr);
	snd_seq_port_subscribe_set_queue(sub, queue);
	snd_seq_port_subscribe_set_time_update(sub, 1);
	snd_seq_port_subscribe_set_time_real(sub, 1);
	if ((err = snd_seq_subscribe_port(handle, sub))<0) {
		fprintf(stderr, "Cannot subscribe announce port: %s\n", snd_strerror(err));
		return;
	}

	addr.client = SND_SEQ_CLIENT_SYSTEM;
	addr.port = SND_SEQ_PORT_SYSTEM_TIMER;
	snd_seq_port_subscribe_set_sender(sub, &addr);
	if ((err = snd_seq_subscribe_port(handle, sub))<0) {
		fprintf(stderr, "Cannot subscribe timer port: %s\n", snd_strerror(err));
		return;
	}

	for (max = 0; max < argc; max++) {
		ptr = argv[max];
		if (!ptr)
			continue;
		snd_seq_port_subscribe_set_time_real(sub, 0);
		if (tolower(*ptr) == 'r') {
			snd_seq_port_subscribe_set_time_real(sub, 1);
			ptr++;
		}
		if (sscanf(ptr, "%i.%i", &v1, &v2) != 2) {
			fprintf(stderr, "Wrong argument '%s'...\n", argv[max]);
			return;
		}
		addr.client = v1;
		addr.port = v2;
		snd_seq_port_subscribe_set_sender(sub, &addr);
		if ((err = snd_seq_subscribe_port(handle, sub))<0) {
			fprintf(stderr, "Cannot subscribe port %i from client %i: %s\n", v2, v1, snd_strerror(err));
			return;
		}
	}
	
	max = snd_seq_poll_descriptors_count(handle, POLLIN);
	pfds = alloca(sizeof(*pfds) * max);
	while (1) {
		snd_seq_poll_descriptors(handle, pfds, max, POLLIN);
		if (poll(pfds, max, -1) < 0)
			break;
		do {
			if ((err = snd_seq_event_input(handle, &ev))<0)
				break;
			if (!ev)
				continue;
			decode_event(ev);
			snd_seq_free_event(ev);
		} while (err > 0);
	}
}
