#include <stdio.h>
#include <unistd.h>
#include <ev.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>

#define NFCM_TIME_PERIOD (5)

int nfcm_time_period = NFCM_TIME_PERIOD;

// ddprintf is copid from cprintf() in shutils.c
// ddprintf will print messages to console
void ddprintf(const char *format, ...)
{ 
    FILE *f;
    int nfd;
    va_list args;

    if((nfd = open("/dev/console", O_WRONLY | O_NONBLOCK)) >= 0){
        if((f = fdopen(nfd, "w")) != NULL){
            va_start(args, format);
            vfprintf(f, format, args);
            va_end(args);
            fclose(f);
        } else {
            close(nfd);
        }
    }
}

void alt_start_nfcm()
{
	char *nfcm_argv[] = {"nfcm", NULL};
	pid_t pid;

    int enb = nvram_get_int("nfcm_enable");
    if (!enb) 
        return;

#if 0
	if(getpid() != 1) { //not rc init process
		notify_rc("start_nfcm");
		return;
	}
#endif

	killall("nfcm", SIGTERM);

	_eval(nfcm_argv, NULL, 0, &pid);

	return;
}

void alt_stop_nfcm()
{
#if 0
	if(getpid() != 1) { //not rc init process
		notify_rc("stop_nfcm");
		return;
	}
#endif

	killall("nfcm", SIGTERM);

	return;
}

void nfcm_time_func(struct ev_loop *loop, ev_timer *w, int e)
{
    if(!pids("nfcm"))
        alt_start_nfcm();
}

int ev_timer_nfcm(struct ev_loop *loop, ev_timer *timer)
{
    ev_init(timer, nfcm_time_func);
    timer->repeat = nfcm_time_period;
    ev_timer_again(loop, timer);

    return 0;
}

int alt_watchdog_main(int argc, char *argv[])
{
    struct ev_loop *loop = EV_DEFAULT;
    ev_timer timer_nfcm;

	ddprintf("alternative watchdog start...\n");

#ifdef RTCONFIG_NFCM
	nfcm_check();
#endif

    // work alarmer
    ev_timer_nfcm(loop, &timer_nfcm);

    // main loop
    ev_run(loop, 0);

	return 0;
}
