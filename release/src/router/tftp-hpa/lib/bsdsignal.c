/*
 * bsdsignal.c
 *
 * Use sigaction() to simulate BSD signal()
 */

#include "config.h"

void (*bsd_signal(int signum, void (*handler) (int))) (int) {
    struct sigaction action, oldaction;

    memset(&action, 0, sizeof action);
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, signum);
    action.sa_flags = SA_RESTART;

    if (sigaction(signum, &action, &oldaction) == -1) {
#ifdef SIG_ERR
        return SIG_ERR;
#else
        return NULL;
#endif
    }

    return oldaction.sa_handler;
}
