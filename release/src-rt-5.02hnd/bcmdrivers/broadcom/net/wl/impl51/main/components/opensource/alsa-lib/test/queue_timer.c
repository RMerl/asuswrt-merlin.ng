#include <unistd.h>
#include <sys/time.h>
#include <alsa/asoundlib.h>
#include <alsa/seq.h>

void normalize(struct timeval *tv)
{
    if (tv->tv_sec == 0) {
	while (tv->tv_usec <= -1000000) { tv->tv_usec += 1000000; --tv->tv_sec; }
	while (tv->tv_usec >=  1000000) { tv->tv_usec -= 1000000; ++tv->tv_sec; }
    } else if (tv->tv_sec < 0) {
	while (tv->tv_usec <= -1000000) { tv->tv_usec += 1000000; --tv->tv_sec; }
	while (tv->tv_usec > 0) { tv->tv_usec -= 1000000; ++tv->tv_sec; }
    } else { 
	while (tv->tv_usec >= 1000000) { tv->tv_usec -= 1000000; ++tv->tv_sec; }
	while (tv->tv_usec < 0) { tv->tv_usec += 1000000; --tv->tv_sec; }
    }
}

int
main(int argc ATTRIBUTE_UNUSED, char **argv ATTRIBUTE_UNUSED)
{
    snd_seq_t *handle;
    int portid;
    /* int npfd;
       struct pollfd *pfd;
    */
    int queue;
    /* int i;
       int rval;'
    */
    struct timeval starttv, prevdiff;
    int countdown = -1;
    /* snd_seq_queue_timer_t *timer;
       snd_timer_id_t *timerid;
    */

    if (snd_seq_open(&handle, "hw", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
	fprintf(stderr, "failed to open ALSA sequencer interface\n");
	return 1;
    }

    snd_seq_set_client_name(handle, "generator");

    if ((portid = snd_seq_create_simple_port
	 (handle, "generator",
	  SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ, 0)) < 0) {
	fprintf(stderr, "failed to create ALSA sequencer port\n");
	return 1;
    }

    if ((queue = snd_seq_alloc_queue(handle)) < 0) {
	fprintf(stderr, "failed to create ALSA sequencer queue\n");
	return 1;
    }
/*
    snd_seq_queue_timer_alloca(&timer);
    snd_seq_get_queue_timer(handle, queue, timer);
    snd_timer_id_alloca(&timerid);
    snd_timer_id_set_class(timerid, SND_TIMER_CLASS_PCM);
    snd_timer_id_set_sclass(timerid, SND_TIMER_SCLASS_NONE);
    snd_timer_id_set_card(timerid, 0);
    snd_timer_id_set_device(timerid, 0);
    snd_timer_id_set_subdevice(timerid, 0);
    snd_seq_queue_timer_set_id(timer, timerid);
    snd_seq_set_queue_timer(handle, queue, timer);
*/
    snd_seq_start_queue(handle, queue, 0);
    snd_seq_drain_output(handle);

    gettimeofday(&starttv, 0);
    prevdiff.tv_sec = 0;
    prevdiff.tv_usec = 0;

    while (countdown != 0) {

	snd_seq_queue_status_t *status;
	const snd_seq_real_time_t *rtime;
	struct timeval tv, diff, diffdiff;
	struct timespec ts;

	snd_seq_queue_status_alloca(&status);

	snd_seq_get_queue_status(handle, queue, status);
	rtime = snd_seq_queue_status_get_real_time(status);

	gettimeofday(&tv, 0);

	tv.tv_sec -= starttv.tv_sec;
	tv.tv_usec -= starttv.tv_usec;
	normalize(&tv);

	diff.tv_sec = tv.tv_sec - rtime->tv_sec;
	diff.tv_usec = tv.tv_usec - rtime->tv_nsec / 1000;
	normalize(&diff);

	diffdiff.tv_sec = diff.tv_sec - prevdiff.tv_sec;
	diffdiff.tv_usec = diff.tv_usec - prevdiff.tv_usec;
	normalize(&diffdiff);
	prevdiff = diff;

	fprintf(stderr, " real time: %12ld sec %8ld usec\nqueue time: %12ld sec %8ld usec\n      diff: %12ld sec %8ld usec\n  diffdiff: %12ld sec %8ld usec\n",
		tv.tv_sec, tv.tv_usec,
		(long)rtime->tv_sec, (long)rtime->tv_nsec / 1000,
		diff.tv_sec, diff.tv_usec,
		(long)diffdiff.tv_sec, (long)diffdiff.tv_usec);

	if (diffdiff.tv_usec >  5000 ||
	    diffdiff.tv_usec < -5000) {
	    fprintf(stderr, "oops! queue slipped\n");
	    if (tv.tv_sec < 5) {
		fprintf(stderr, "(ignoring in first few seconds)\n");
	    } else {
		countdown = 2;
	    }
	} else {
	    if (countdown > 0) --countdown;
	}

	fprintf(stderr, "\n");
//	sleep(1);
	ts.tv_sec = 0;
	ts.tv_nsec = 500000000;
	nanosleep(&ts, 0);
    }
    return EXIT_SUCCESS;
}
