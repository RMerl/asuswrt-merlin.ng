#ifndef _mastiff_nt_h_
#define _mastiff_nt_h_

#include <signal.h>

#define MASTIFF_PID_PATH           "/tmp/mastiff.pid"

// Signal definition
#define AAE_SIG_REMOTE_CONNECTION_TURNED_ON SIGUSR1
#define AAE_SIG_EULA_FLAG_SIGNED SIGUSR2
#define AAE_SIG_CHECK_ACCOUNT_LINKING_STATUS SIGRTMIN

#endif