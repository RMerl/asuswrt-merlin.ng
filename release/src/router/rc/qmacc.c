#include "rc.h"

void start_qmacc(void)
{
	pid_t pid;
	char *cmd[] = {"qmacc_asus_monitor.sh", NULL};

	stop_qmacc();

	if(getpid()!=1) {
		notify_rc("start_qmacc");
		return;
	}

	_eval(cmd, NULL, 0, &pid);
}


void stop_qmacc(void)
{
	doSystem("killall qmacc_asus_monitor.sh");
	if (pidof("qmacc-asus") > 0)
		doSystem("killall qmacc-asus");
}
