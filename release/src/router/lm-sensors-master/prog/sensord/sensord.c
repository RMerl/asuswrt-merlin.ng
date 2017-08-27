/*
 * sensord
 *
 * A daemon that periodically logs sensor information to syslog.
 *
 * Copyright (c) 1999-2002 Merlin Hughes <merlin@merlin.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "args.h"
#include "sensord.h"

static int logOpened = 0;

static volatile sig_atomic_t done = 0;
static volatile sig_atomic_t reload = 0;

#define LOG_BUFFER 4096

#include <stdarg.h>

void sensorLog(int priority, const char *fmt, ...)
{
	static char buffer[1 + LOG_BUFFER];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buffer, LOG_BUFFER, fmt, ap);
	buffer[LOG_BUFFER] = '\0';
	va_end(ap);
	if (sensord_args.debug || (priority < LOG_DEBUG)) {
		if (logOpened) {
			syslog(priority, "%s", buffer);
		} else {
			fprintf(stderr, "%s\n", buffer);
			fflush(stderr);
		}
	}
}

static void signalHandler(int sig)
{
	switch (sig) {
	case SIGTERM:
		done = 1;
		break;
	case SIGHUP:
		reload = 1;
		break;
	}
}

static int sensord(void)
{
	int ret = 0;
	int scanValue = 0, logValue = 0;
	/*
	 * First RRD update at next RRD timeslot to prevent failures due
	 * one timeslot updated twice on restart for example.
	 */
	int rrdValue = sensord_args.rrdTime - time(NULL) %
		sensord_args.rrdTime;

	sensorLog(LOG_INFO, "sensord started");

	while (!done) {
		if (reload) {
			ret = reloadLib(sensord_args.cfgFile);
			if (ret)
				sensorLog(LOG_NOTICE, "configuration reload"
					  " error");
			reload = 0;
		}
		if (sensord_args.scanTime && (scanValue <= 0)) {
			if ((ret = scanChips()))
				sensorLog(LOG_NOTICE,
					  "sensor scan error (%d)", ret);
			scanValue += sensord_args.scanTime;
		}
		if (sensord_args.logTime && (logValue <= 0)) {
			if ((ret = readChips()))
				sensorLog(LOG_NOTICE,
					  "sensor read error (%d)", ret);
			logValue += sensord_args.logTime;
		}
		if (sensord_args.rrdTime && sensord_args.rrdFile &&
		    (rrdValue <= 0)) {
			if ((ret = rrdUpdate()))
				sensorLog(LOG_NOTICE,
					  "rrd update error (%d)", ret);
			/*
			 * The amount of time to wait is computed using the
			 * same method as in RRD instead of simply adding the
			 * interval.
			 */
			rrdValue = sensord_args.rrdTime - time(NULL) %
				sensord_args.rrdTime;
		}
		if (!done) {
			int a = sensord_args.logTime ? logValue : INT_MAX;
			int b = sensord_args.scanTime ? scanValue : INT_MAX;
			int c = (sensord_args.rrdTime && sensord_args.rrdFile)
				? rrdValue : INT_MAX;
			int sleepTime = (a < b) ? ((a < c) ? a : c) :
				((b < c) ? b : c);
			sleep(sleepTime);
			scanValue -= sleepTime;
			logValue -= sleepTime;
			rrdValue -= sleepTime;
		}
	}

	sensorLog(LOG_INFO, "sensord stopped");

	return ret;
}

static void openLog(void)
{
	openlog("sensord", 0, sensord_args.syslogFacility);
	logOpened = 1;
}

static void install_sighandler(void)
{
	struct sigaction new;
	int ret;

	memset(&new, 0, sizeof(struct sigaction));
	new.sa_handler = signalHandler;
	sigemptyset(&new.sa_mask);
	new.sa_flags = SA_RESTART;

	ret = sigaction(SIGTERM, &new, NULL);
	if (ret == -1) {
		fprintf(stderr, "Could not set sighandler for SIGTERM: %s\n",
			strerror(errno));
		exit(EXIT_FAILURE);
	}

	ret = sigaction(SIGHUP, &new, NULL);
	if (ret == -1) {
		fprintf(stderr, "Could not set sighandler for SIGHUP: %s\n",
			strerror(errno));
		exit(EXIT_FAILURE);
	}
}

static void daemonize(void)
{
	int pid;
	struct stat fileStat;
	FILE *file;

	if (chdir("/") < 0) {
		perror("chdir()");
		exit(EXIT_FAILURE);
	}

	if (!(stat(sensord_args.pidFile, &fileStat)) &&
	    ((!S_ISREG(fileStat.st_mode)) || (fileStat.st_size > 11))) {
		fprintf(stderr,
			"Error: PID file `%s' already exists and looks suspicious.\n",
			sensord_args.pidFile);
		exit(EXIT_FAILURE);
	}

	if (!(file = fopen(sensord_args.pidFile, "w"))) {
		fprintf(stderr, "fopen(\"%s\"): %s\n", sensord_args.pidFile,
			strerror(errno));
		exit(EXIT_FAILURE);
	}

	install_sighandler();

	if ((pid = fork()) == -1) {
		perror("fork()");
		exit(EXIT_FAILURE);
	} else if (pid != 0) {
		fprintf(file, "%d\n", pid);
		fclose(file);

		freeChips();
		if (unloadLib())
			exit(EXIT_FAILURE);

		exit(EXIT_SUCCESS);
	}

	if (setsid() < 0) {
		perror("setsid()");
		exit(EXIT_FAILURE);
	}

	fclose(file);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}

static void undaemonize(void)
{
	unlink(sensord_args.pidFile);
	closelog();
}

int main(int argc, char **argv)
{
	int ret = 0;

	if (parseArgs(argc, argv) ||
	    parseChips(argc, argv))
		exit(EXIT_FAILURE);

	if (loadLib(sensord_args.cfgFile)) {
		freeChips();
		exit(EXIT_FAILURE);
	}

	if (!sensord_args.doCGI)
		openLog();

	if (sensord_args.rrdFile) {
		ret = rrdInit();
		if (ret) {
			freeChips();
			exit(EXIT_FAILURE);
		}
	}

	if (sensord_args.doCGI) {
		ret = rrdCGI();
	} else {
		daemonize();
		ret = sensord();
		undaemonize();
	}

	freeChips();
	if (unloadLib())
		exit(EXIT_FAILURE);

	return ret;
}
