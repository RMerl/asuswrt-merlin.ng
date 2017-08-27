/*
	bwdpi_check.c for keep wred / data_colld alive
*/

#include <rc.h>
#include <bwdpi_common.h>

static void check_wred_alive()
{
	int enabled = check_bwdpi_nvram_setting();

	if(enabled){
		BWDPI_DBG("start_wrs and start_data_colld\n");
		// start wrs
		start_wrs();
		// start data_colld
		start_dc(NULL);
		alarm(120);
	}
}

static int sig_cur = -1;

static void catch_sig(int sig)
{
	sig_cur = sig;
	
	if (sig == SIGTERM)
	{
		remove("/var/run/bwdpi_wred_check.pid");
		exit(0);
	}
	else if(sig == SIGALRM)
	{
		check_wred_alive();
	}
}

int bwdpi_wred_alive_main(int argc, char **argv)
{
	FILE *fp;
	sigset_t sigs_to_catch;

	/* write pid */
	if ((fp = fopen("/var/run/bwdpi_wred_alive.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	/* set the signal handler */
	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGTERM);
	sigaddset(&sigs_to_catch, SIGALRM);
	sigprocmask(SIG_UNBLOCK, &sigs_to_catch, NULL);

	signal(SIGTERM, catch_sig);
	signal(SIGALRM, catch_sig);

	while(1)
	{
		alarm(120);
		pause();
	}

	return 0;
}
