#ifndef _mastiff_nt_h_
#define _mastiff_nt_h_

#include <signal.h>

#define MASTIFF_PID_PATH           "/tmp/mastiff.pid"
#define AAEWS_PID_PATH           "/tmp/aaews.pid"

// Signal definition
#define AAE_SIG_REMOTE_CONNECTION_TURNED_ON SIGUSR1
#define AAE_SIG_EULA_FLAG_SIGNED SIGUSR2
#define AAE_SIG_CHECK_ACCOUNT_LINKING_STATUS SIGRTMIN


#define AAEWS_SIG_ACTION SIGUSR1
enum {
	AAEWS_ACTION_SIP_UNREGISTER = 1,
	AAEWS_ACTION_SIP_REGISTER,
	AAEWS_ACTION_SDK_DEINIT,
	AAEWS_ACTION_SDK_INIT,
};

#endif