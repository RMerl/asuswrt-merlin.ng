/*
 * Copyright (c) 2013, NLNet Labs, Verisign, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the names of the copyright holders nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Verisign, Inc. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include <getdns/getdns.h>
#include <getdns/getdns_extra.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <limits.h>
#ifndef HAVE_GETOPT
#include "getopt.h"
#else
#include <unistd.h>
#endif
#if defined(ENABLE_SYSTEMD)
#include <systemd/sd-daemon.h>
#endif

#include "configfile.h"
#include "log.h"
#include "server.h"
#include "util.h"

#if defined(ENABLE_WINDOWS_SERVICE)
#include "windowsservice.h"
#else
#define STUBBYPIDFILE RUNSTATEDIR"/stubby.pid"
#endif

void
print_usage(FILE *out)
{
	char *home_conf_fn = home_config_file();
	char *system_conf_fn = system_config_file();
	fprintf(out, "usage: " STUBBY_PACKAGE " [<option> ...] \\\n");
	fprintf(out, "\t-C\t<filename>\n");
	fprintf(out, "\t\tRead settings from config file <filename>\n");
	fprintf(out, "\t\tThe getdns context will be configured with these settings\n");
	fprintf(out, "\t\tThe file should be in YAML format with an extension of .yml.\n");
	fprintf(out, "\t\t(The old JSON dict format (.conf) is also still supported when\n");
	fprintf(out, "\t\tspecified on the command line.)\n");
	fprintf(out, "\t\tBy default, the configuration file location is obtained\n");
	fprintf(out, "\t\tby looking for YAML files in the following order:\n");
	fprintf(out, "\t\t\t\"%s\"\n", home_conf_fn);
	fprintf(out, "\t\t\t\"%s\"\n", system_conf_fn);
	fprintf(out, "\t\tA default file (Using Strict mode) is installed as\n");
	fprintf(out, "\t\t\t\"%s\"\n", system_conf_fn);
#if !defined(STUBBY_ON_WINDOWS)
	fprintf(out, "\t-g\tRun stubby in background (default is foreground)\n");
#endif
	fprintf(out, "\t-h\tPrint this help\n");
	fprintf(out, "\t-i\tValidate and print the configuration only. Useful to validate config file\n");
	fprintf(out, "\t\tcontents. Note: does not attempt to bind to the listen addresses.\n");
	fprintf(out, "\t-l\tEnable logging of all logs (same as -v 7)\n");
	fprintf(out, "\t-v\tSpecify logging level (overrides -l option). Values are\n");
	fprintf(out, "\t\t\t0: EMERG  - %s\n", GETDNS_LOG_EMERG_TEXT);
	fprintf(out, "\t\t\t1: ALERT  - %s\n", GETDNS_LOG_ALERT_TEXT);
	fprintf(out, "\t\t\t2: CRIT   - %s\n", GETDNS_LOG_CRIT_TEXT);
	fprintf(out, "\t\t\t3: ERROR  - %s\n", GETDNS_LOG_ERR_TEXT);
	fprintf(out, "\t\t\t4: WARN   - %s\n", GETDNS_LOG_WARNING_TEXT);
	fprintf(out, "\t\t\t5: NOTICE - %s\n", GETDNS_LOG_NOTICE_TEXT);
	fprintf(out, "\t\t\t6: INFO   - %s\n", GETDNS_LOG_INFO_TEXT);
	fprintf(out, "\t\t\t7: DEBUG  - %s\n", GETDNS_LOG_DEBUG_TEXT);
	fprintf(out, "\t-V\tPrint the " STUBBY_PACKAGE " version\n");
	free(home_conf_fn);
	free(system_conf_fn);
}

void
print_version(FILE *out)
{
	fprintf(out, STUBBY_PACKAGE_STRING "\n");
}

int
main(int argc, char **argv)
{
	const char *custom_config_fn = NULL;
	int print_api_info = 0;
	int run_in_foreground = 1;
	int log_connections = 0;
	int dnssec_validation = 0;
#if defined(ENABLE_WINDOWS_SERVICE)
	int windows_service = 0;
	const char *windows_service_arg = NULL;
#endif
	getdns_context  *context = NULL;
	getdns_return_t r;
	int opt;
	long log_level = 7; 
	char *ep;

	while ((opt = getopt(argc, argv, "C:ighlv:w:V")) != -1) {
		switch (opt) {
		case 'C':
			custom_config_fn = optarg;
			break;
		case 'g':
			run_in_foreground = 0;
			break;
		case 'h':
			print_usage(stdout);
			exit(EXIT_SUCCESS);
		case 'i':
			print_api_info = 1;
			break;
		case 'l':
			log_connections = 1;
			break;
		case 'v':
			log_connections = 1;
			errno = 0;
			log_level = strtol(optarg, &ep, 10);
			if (log_level < 0 ||  log_level > 7 || *ep != '\0' || 
			    (errno == ERANGE &&
			    (log_level == LONG_MAX || log_level == LONG_MIN)) ) {
				stubby_error("Log level '%s' is invalid or out of range (0-7)", optarg);
				exit(EXIT_FAILURE);
			}
			break;
#if defined(ENABLE_WINDOWS_SERVICE)
		case 'w':
			windows_service = 1;
			windows_service_arg = optarg;
			break;
#endif
                case 'V':
			print_version(stdout);
			exit(EXIT_SUCCESS);
		default:
			print_usage(stderr);
			exit(EXIT_FAILURE);
		}
	}

	stubby_log(NULL,GETDNS_LOG_UPSTREAM_STATS, GETDNS_LOG_INFO,
		   "Stubby version: %s", STUBBY_PACKAGE_STRING);

#if defined(ENABLE_WINDOWS_SERVICE)
	if ( windows_service ) {
		windows_service_command(windows_service_arg, log_connections ? log_level : 0, custom_config_fn);
		exit(EXIT_SUCCESS);
	}
#endif

	if ((r = getdns_context_create(&context, 1))) {
		stubby_error("Create context failed: %s",
		        stubby_getdns_strerror(r));
		return r;
	}
	if (log_connections)
		stubby_set_getdns_logging(context, (int)log_level);

	init_config(context);
	if ( !read_config(context, custom_config_fn, &dnssec_validation) )
		exit(EXIT_FAILURE);

	if (print_api_info) {
		char *api_information_str = config_get_api_info(context);
		fprintf(stdout, "%s\n", api_information_str);
		free(api_information_str);
		fprintf(stderr, "Result: Config file syntax is valid.\n");
		r = EXIT_SUCCESS;
		goto tidy_and_exit;
	}

	if ( !server_listen(context, dnssec_validation) ) {
		r = EXIT_FAILURE;
		goto tidy_and_exit;
	}
	
#if !defined(STUBBY_ON_WINDOWS)
	if (!run_in_foreground) {
		pid_t pid;
		char pid_str[1024], *endptr;
		FILE *fh = fopen(STUBBYPIDFILE, "r");
		do {
			pid_t running;

			if (!fh || !fgets(pid_str, sizeof(pid_str), fh))
				break;

			running = strtol(pid_str, &endptr, 10);
			if (endptr == pid_str)
				break;

			if (kill(running, 0) < 0 && errno == ESRCH)
				break;

			stubby_error("Not starting because a running "
				     "stubby was found on pid: %d", running);
			exit(EXIT_FAILURE);
		} while(0);
		if (fh)
			(void) fclose(fh);

		pid = fork();
		if (pid == -1) {
			perror("Could not fork of stubby daemon\n");
			r = GETDNS_RETURN_GENERIC_ERROR;

		} else if (pid) {
			fh = fopen(STUBBYPIDFILE, "w");
			if (fh) {
				fprintf(fh, "%d", (int)pid);
				fclose(fh);
			} else {
				stubby_error("Could not write pid to "
					     "\"%s\": %s", STUBBYPIDFILE,
					     strerror(errno));
				exit(EXIT_FAILURE);
			}
		} else {
#ifdef SIGPIPE
			(void)signal(SIGPIPE, SIG_IGN);
#endif
#ifdef ENABLE_SYSTEMD
			sd_notifyf(0, "READY=1\nMAINPID=%u", getpid());
#endif
			getdns_context_run(context);
		}
	} else
#endif
	{
		/* Report basic config options which specifically affect privacy and validation*/
		stubby_log(NULL,GETDNS_LOG_UPSTREAM_STATS, GETDNS_LOG_INFO,
			   "DNSSEC Validation is %s", dnssec_validation==1 ? "ON":"OFF");
		size_t transport_count = 0;
		getdns_transport_list_t *transport_list;
		getdns_context_get_dns_transport_list(context, 
		                                 &transport_count, &transport_list);
		stubby_log(NULL,GETDNS_LOG_UPSTREAM_STATS, GETDNS_LOG_INFO,
			   "Transport list is:");
		for (size_t i = 0; i < transport_count; i++) {
			char* transport_name;
			switch (transport_list[i]) {
				case GETDNS_TRANSPORT_UDP:
					transport_name = "UDP";
					break;
				case GETDNS_TRANSPORT_TCP:
					transport_name = "TCP";
					break;
				case GETDNS_TRANSPORT_TLS:
					transport_name = "TLS";
					break;
				default:
					transport_name = "Unknown transport type";
					break;
				}
			stubby_log(NULL,GETDNS_LOG_UPSTREAM_STATS, GETDNS_LOG_INFO,
			                 "  - %s", transport_name);
		}
		free(transport_list);
		getdns_tls_authentication_t auth;
		getdns_context_get_tls_authentication(context, &auth);
		stubby_log(NULL,GETDNS_LOG_UPSTREAM_STATS, GETDNS_LOG_INFO,
			   "Privacy Usage Profile is %s",
			   auth==GETDNS_AUTHENTICATION_REQUIRED ?
			   "Strict (Authentication required)":"Opportunistic");
		stubby_log(NULL,GETDNS_LOG_UPSTREAM_STATS, GETDNS_LOG_INFO,
			   "(NOTE a Strict Profile only applies when TLS is the ONLY transport!!)");
		stubby_log(NULL,GETDNS_LOG_UPSTREAM_STATS, GETDNS_LOG_DEBUG,
			   "Starting DAEMON....");
#ifdef SIGPIPE
		(void)signal(SIGPIPE, SIG_IGN);
#endif
#ifdef ENABLE_SYSTEMD
		sd_notify(0, "READY=1");
#endif
		getdns_context_run(context);
	}

tidy_and_exit:
	getdns_context_destroy(context);

	delete_config();

	return r;
}
