#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <syslog.h>
#include <ev.h>

void skipd_daemonize(char * path)
{
#ifndef __MINGW32__
    /* Our process ID and Session ID */
    pid_t pid, sid;

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    /* If we got a good PID, then
       we can exit the parent process. */
    if (pid > 0) {
        FILE *file = fopen(path, "w");
        if (file == NULL) {
            fprintf(stderr, "Invalid pid file\n");
        }

        fprintf(file, "%d", pid);
        fclose(file);
        exit(EXIT_SUCCESS);
    }


    /* Cancel certain signals */
    signal(SIGCHLD, SIG_DFL); /* A child process dies */
    signal(SIGTSTP, SIG_IGN); /* Various TTY signals */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGHUP, SIG_IGN); /* Ignore hangup signal */

    /* Change the file mode mask */
    umask(0);

    /* Open any logs here */

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
        /* Log the failure */
        exit(EXIT_FAILURE);
    }

    /* Change the current working directory */
    if ((chdir("/")) < 0) {
        /* Log the failure */
        exit(EXIT_FAILURE);
    }

    /* Redirect standard files to /dev/null */
    if (!freopen("/dev/null", "r", stdin)) {
        fprintf(stderr, "unable to freopen() stdin, code %d (%s)",
                                                   errno, strerror(errno));
    }

    if (!freopen("/dev/null", "w", stdout)) {
        fprintf(stderr, "unable to freopen() stdout, code %d (%s)",
                                                   errno, strerror(errno));
    }

    if (!freopen("/dev/null", "w", stderr)) {
        fprintf(stderr, "unable to freopen() stderr, code %d (%s)",
                                                   errno, strerror(errno));
    }

    /* Close out the standard file descriptors */
    /* close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO); */
#endif
}
