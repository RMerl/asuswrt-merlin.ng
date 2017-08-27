#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "../include/asoundlib.h"

void show_status(void *handle)
{
	int err;
	snd_timer_status_t *status;
	
	snd_timer_status_alloca(&status);
	if ((err = snd_timer_status(handle, status)) < 0) {
		fprintf(stderr, "timer status %i (%s)\n", err, snd_strerror(err));
		return;
	}
	printf("STATUS:\n");
	printf("  resolution = %li\n", snd_timer_status_get_resolution(status));
	printf("  lost = %li\n", snd_timer_status_get_lost(status));
	printf("  overrun = %li\n", snd_timer_status_get_overrun(status));
	printf("  queue = %li\n", snd_timer_status_get_queue(status));
}

void read_loop(void *handle, int master_ticks, int timeout)
{
	int count, err;
	struct pollfd *fds;
	snd_timer_read_t tr;
	
	count = snd_timer_poll_descriptors_count(handle);
	fds = calloc(count, sizeof(struct pollfd));
	if (fds == NULL) {
		fprintf(stderr, "malloc error\n");
		exit(EXIT_FAILURE);
	}
	while (master_ticks-- > 0) {
		if ((err = snd_timer_poll_descriptors(handle, fds, count)) < 0) {
			fprintf(stderr, "snd_timer_poll_descriptors error: %s\n", snd_strerror(err));
			exit(EXIT_FAILURE);
		}
		if ((err = poll(fds, count, timeout)) < 0) {
			fprintf(stderr, "poll error %i (%s)\n", err, strerror(err));
			exit(EXIT_FAILURE);
		}
		if (err == 0) {
			fprintf(stderr, "timer time out!!\n");
			exit(EXIT_FAILURE);
		}
		while (snd_timer_read(handle, &tr, sizeof(tr)) == sizeof(tr)) {
			printf("TIMER: resolution = %uns, ticks = %u\n",
				tr.resolution, tr.ticks);
		}
	}
	free(fds);
}

static void async_callback(snd_async_handler_t *ahandler)
{
	snd_timer_t *handle = snd_async_handler_get_timer(ahandler);
	int *acount = snd_async_handler_get_callback_private(ahandler);
	snd_timer_read_t tr;
	
	while (snd_timer_read(handle, &tr, sizeof(tr)) == sizeof(tr)) {
		printf("TIMER: resolution = %uns, ticks = %u\n",
			tr.resolution, tr.ticks);
	}
	(*acount)++;
}

int main(int argc, char *argv[])
{
	int idx, err;
	int class = SND_TIMER_CLASS_GLOBAL;
	int sclass = SND_TIMER_CLASS_NONE;
	int card = 0;
	int device = SND_TIMER_GLOBAL_SYSTEM;
	int subdevice = 0;
	int list = 0;
	int async = 0;
	int acount = 0;
	snd_timer_t *handle;
	snd_timer_id_t *id;
	snd_timer_info_t *info;
	snd_timer_params_t *params;
	char timername[64];
	snd_async_handler_t *ahandler;

	snd_timer_id_alloca(&id);
	snd_timer_info_alloca(&info);
	snd_timer_params_alloca(&params);

	idx = 1;
	while (idx < argc) {
		if (!strncmp(argv[idx], "class=", 5)) {
			class = atoi(argv[idx]+6);
		} else if (!strncmp(argv[idx], "sclass=", 6)) {
			sclass = atoi(argv[idx]+7);
		} else if (!strncmp(argv[idx], "card=", 5)) {
			card = atoi(argv[idx]+5);
		} else if (!strncmp(argv[idx], "device=", 7)) {
			device = atoi(argv[idx]+7);
		} else if (!strncmp(argv[idx], "subdevice=", 10)) {
			subdevice = atoi(argv[idx]+10);
		} else if (!strcmp(argv[idx], "list")) {
			list = 1;
		} else if (!strcmp(argv[idx], "async")) {
			async = 1;
		}
		idx++;
	}
	if (class == SND_TIMER_CLASS_SLAVE && sclass == SND_TIMER_SCLASS_NONE) {
		fprintf(stderr, "slave class is not set\n");
		exit(EXIT_FAILURE);
	}
	if (list) {
		snd_timer_query_t *qhandle;
		if ((err = snd_timer_query_open(&qhandle, "hw", 0)) < 0) {
			fprintf(stderr, "snd_timer_query_open error: %s\n", snd_strerror(err));
			exit(EXIT_FAILURE);
		}
		snd_timer_id_set_class(id, SND_TIMER_CLASS_NONE);
		while (1) {
			if ((err = snd_timer_query_next_device(qhandle, id)) < 0) {
				fprintf(stderr, "timer next device error: %s\n", snd_strerror(err));
				break;
			}
			if (snd_timer_id_get_class(id) < 0)
				break;
			printf("Timer device: class %i, sclass %i, card %i, device %i, subdevice %i\n",
					snd_timer_id_get_class(id),
					snd_timer_id_get_sclass(id),
					snd_timer_id_get_card(id),
					snd_timer_id_get_device(id),
					snd_timer_id_get_subdevice(id));
		}
		snd_timer_query_close(qhandle);
		exit(EXIT_SUCCESS);
	}
	sprintf(timername, "hw:CLASS=%i,SCLASS=%i,CARD=%i,DEV=%i,SUBDEV=%i", class, sclass, card, device, subdevice);
	if ((err = snd_timer_open(&handle, timername, SND_TIMER_OPEN_NONBLOCK))<0) {
		fprintf(stderr, "timer open %i (%s)\n", err, snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	printf("Using timer class %i, slave class %i, card %i, device %i, subdevice %i\n", class, sclass, card, device, subdevice);
	if ((err = snd_timer_info(handle, info)) < 0) {
		fprintf(stderr, "timer info %i (%s)\n", err, snd_strerror(err));
		exit(0);
	}
	printf("Timer info:\n");
	printf("  slave = %s\n", snd_timer_info_is_slave(info) ? "yes" : "no");
	printf("  card = %i\n", snd_timer_info_get_card(info));
	printf("  id = '%s'\n", snd_timer_info_get_id(info));
	printf("  name = '%s'\n", snd_timer_info_get_name(info));
	printf("  average resolution = %li\n", snd_timer_info_get_resolution(info));
	snd_timer_params_set_auto_start(params, 1);
	if (!snd_timer_info_is_slave(info)) {
		snd_timer_params_set_ticks(params, (1000000000 / snd_timer_info_get_resolution(info)) / 50); /* 50Hz */
		if (snd_timer_params_get_ticks(params) < 1)
			snd_timer_params_set_ticks(params, 1);
		printf("Using %li tick(s)\n", snd_timer_params_get_ticks(params));
	} else {
		snd_timer_params_set_ticks(params, 1);
	}
	if ((err = snd_timer_params(handle, params)) < 0) {
		fprintf(stderr, "timer params %i (%s)\n", err, snd_strerror(err));
		exit(0);
	}
	show_status(handle);
	if (async) {
		err = snd_async_add_timer_handler(&ahandler, handle, async_callback, &acount);
		if (err < 0) {
			fprintf(stderr, "unable to add async handler %i (%s)\n", err, snd_strerror(err));
			exit(EXIT_FAILURE);
		}
	}
	if ((err = snd_timer_start(handle)) < 0) {
		fprintf(stderr, "timer start %i (%s)\n", err, snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	if (async) {
		/* because all other work is done in the signal handler,
		   suspend the process */
		while (acount < 25)
			sleep(1);
		snd_timer_stop(handle);
	} else {
		read_loop(handle, 25, snd_timer_info_is_slave(info) ? 10000 : 25);
	}
	show_status(handle);
	snd_timer_close(handle);
	printf("Done\n");
	return EXIT_SUCCESS;
}
