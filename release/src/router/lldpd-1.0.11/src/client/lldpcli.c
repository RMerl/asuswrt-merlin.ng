/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2008 Vincent Bernat <bernat@luffy.cx>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <libgen.h>
#include <dirent.h>
#include <signal.h>
#include <sys/queue.h>

#include "client.h"

#ifdef HAVE___PROGNAME
extern const char	*__progname;
#else
# define __progname "lldpcli"
#endif

/* Global for completion */
static struct cmd_node *root = NULL;
const char *ctlname = NULL;

static int
is_lldpctl(const char *name)
{
	static int last_result = -1;
	if (last_result == -1 && name) {
		char *basec = strdup(name);
		if (!basec) return 0;
		char *bname = basename(basec);
		last_result = (!strcmp(bname, "lldpctl"));
		free(basec);
	}
	return (last_result == -1)?0:last_result;
}

static void
usage()
{
	fprintf(stderr, "Usage:   %s [OPTIONS ...] [COMMAND ...]\n", __progname);
	fprintf(stderr, "Version: %s\n", PACKAGE_STRING);

	fprintf(stderr, "\n");

	fprintf(stderr, "-d          Enable more debugging information.\n");
	fprintf(stderr, "-u socket   Specify the Unix-domain socket used for communication with lldpd(8).\n");
	fprintf(stderr, "-f format   Choose output format (plain, keyvalue, json, json0"
#if defined USE_XML
	    ", xml"
#endif
	    ").\n");
	if (!is_lldpctl(NULL))
		fprintf(stderr, "-c conf     Read the provided configuration file.\n");

	fprintf(stderr, "\n");

	fprintf(stderr, "see manual page lldpcli(8) for more information\n");
	exit(1);
}

static int
is_privileged()
{
	/* Check we can access the control socket with read/write
	 * privileges. The `access()` function uses the real UID and real GID,
	 * therefore we don't have to mangle with our identity. */
	return (ctlname && access(ctlname, R_OK|W_OK) == 0);
}

static char*
prompt()
{
#define CESC "\033"
	int privileged = is_privileged();
	if (isatty(STDIN_FILENO)) {
		if (privileged)
			return "[lldpcli] # ";
		return "[lldpcli] $ ";
	}
	return "";
}

static int must_exit = 0;
/**
 * Exit the interpreter.
 */
static int
cmd_exit(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	log_info("lldpctl", "quit lldpcli");
	must_exit = 1;
	return 1;
}

/**
 * Send an "update" request.
 */
static int
cmd_update(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg)
{
	log_info("lldpctl", "ask for global update");

	lldpctl_atom_t *config = lldpctl_get_configuration(conn);
	if (config == NULL) {
		log_warnx("lldpctl", "unable to get configuration from lldpd. %s",
		    lldpctl_last_strerror(conn));
		return 0;
	}
	if (lldpctl_atom_set_int(config,
		lldpctl_k_config_tx_interval, -1) == NULL) {
		log_warnx("lldpctl", "unable to ask lldpd for immediate retransmission. %s",
		    lldpctl_last_strerror(conn));
		lldpctl_atom_dec_ref(config);
		return 0;
	}
	log_info("lldpctl", "immediate retransmission requested successfully");
	lldpctl_atom_dec_ref(config);
	return 1;
}

/**
 * Pause or resume execution of lldpd.
 *
 * @param conn    The connection to lldpd.
 * @param pause   1 if we want to pause lldpd, 0 otherwise
 * @return 1 on success, 0 on error
 */
static int
cmd_pause_resume(lldpctl_conn_t *conn, int pause)
{
	lldpctl_atom_t *config = lldpctl_get_configuration(conn);
	if (config == NULL) {
		log_warnx("lldpctl", "unable to get configuration from lldpd. %s",
		    lldpctl_last_strerror(conn));
		return 0;
	}
	if (lldpctl_atom_get_int(config, lldpctl_k_config_paused) == pause) {
		log_debug("lldpctl", "lldpd is already %s",
		    pause?"paused":"resumed");
		lldpctl_atom_dec_ref(config);
		return 1;
	}
	if (lldpctl_atom_set_int(config,
		lldpctl_k_config_paused, pause) == NULL) {
		log_warnx("lldpctl", "unable to ask lldpd to %s operations. %s",
		    pause?"pause":"resume",
		    lldpctl_last_strerror(conn));
		lldpctl_atom_dec_ref(config);
		return 0;
	}
	log_info("lldpctl", "lldpd should %s operations",
	    pause?"pause":"resume");
	lldpctl_atom_dec_ref(config);
	return 1;
}
static int
cmd_pause(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg) {
	(void)w; (void)env;
	return cmd_pause_resume(conn, 1);
}
static int
cmd_resume(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *arg) {
	(void)w; (void)env;
	return cmd_pause_resume(conn, 0);
}


#ifdef HAVE_LIBREADLINE
static int
_cmd_complete(int all)
{
	char **argv = NULL;
	int argc = 0;
	int rc = 1;
	size_t len = strlen(rl_line_buffer);
	char *line = malloc(len + 2);
	if (!line) return -1;
	strlcpy(line, rl_line_buffer, len + 2);
	line[rl_point]   = 2;	/* empty character, will force a word */
	line[rl_point+1] = 0;

	if (tokenize_line(line, &argc, &argv) != 0)
		goto end;

	char *compl = commands_complete(root, argc, (const char **)argv, all, is_privileged());
	if (compl && strlen(argv[argc-1]) < strlen(compl)) {
		if (rl_insert_text(compl + strlen(argv[argc-1])) < 0) {
			free(compl);
			goto end;
		}
		free(compl);
		rc = 0;
		goto end;
	}
	/* No completion or several completion available. */
	free(compl);
	fprintf(stderr, "\n");
	rl_forced_update_display();
	rc = 0;
end:
	free(line);
	tokenize_free(argc, argv);
	return rc;
}

static int
cmd_complete(int count, int ch)
{
	return _cmd_complete(0);
}

static int
cmd_help(int count, int ch)
{
	return _cmd_complete(1);
}
#else
static char*
readline(const char *p)
{
	static char line[2048];
	fprintf(stderr, "%s", p);
	fflush(stderr);
	if (fgets(line, sizeof(line) - 2, stdin) == NULL)
		return NULL;
	return strdup(line);
}
#endif

/**
 * Execute a tokenized command and display its output.
 *
 * @param conn The connection to lldpd.
 * @param fmt  Output format.
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 * @return 0 if an error occurred, 1 otherwise
 */
static int
cmd_exec(lldpctl_conn_t *conn, const char *fmt, int argc, const char **argv)
{
	/* Init output formatter */
	struct writer *w;

	if      (strcmp(fmt, "plain")    == 0) w = txt_init(stdout);
	else if (strcmp(fmt, "keyvalue") == 0) w = kv_init(stdout);
	else if (strcmp(fmt, "json")     == 0) w = json_init(stdout, 1);
	else if (strcmp(fmt, "json0")    == 0) w = json_init(stdout, 0);
#ifdef USE_XML
	else if (strcmp(fmt, "xml")      == 0) w = xml_init(stdout);
#endif
	else {
		log_warnx("lldpctl", "unknown output format \"%s\"", fmt);
		w = txt_init(stdout);
	}

	/* Execute command */
	int rc = commands_execute(conn, w,
	    root, argc, argv, is_privileged());
	if (rc != 0) {
		log_info("lldpctl", "an error occurred while executing last command");
		w->finish(w);
		return 0;
	}
	w->finish(w);
	return 1;
}

/**
 * Execute a command line and display its output.
 *
 * @param conn The connection to lldpd.
 * @param fmt  Output format.
 * @param line Line to execute.
 * @return -1 if an error occurred, 0 if nothing was executed. 1 otherwise.
 */
static int
parse_and_exec(lldpctl_conn_t *conn, const char *fmt, const char *line)
{
	int cargc = 0; char **cargv = NULL;
	int n;
	log_debug("lldpctl", "tokenize command line");
	n = tokenize_line(line, &cargc, &cargv);
	switch (n) {
	case -1:
		log_warnx("lldpctl", "internal error while tokenizing");
		return -1;
	case 1:
		log_warnx("lldpctl", "unmatched quotes");
		return -1;
	}
	if (cargc != 0)
		n = cmd_exec(conn, fmt, cargc, (const char **)cargv);
	tokenize_free(cargc, cargv);
	return (cargc == 0)?0:
	    (n == 0)?-1:
	    1;
}

static struct cmd_node*
register_commands()
{
	root = commands_root();
	register_commands_show(root);
	register_commands_watch(root);
	commands_privileged(commands_new(
		commands_new(root, "update", "Update information and send LLDPU on all ports",
		    NULL, NULL, NULL),
		NEWLINE, "Update information and send LLDPU on all ports",
		NULL, cmd_update, NULL));
	register_commands_configure(root);
	commands_hidden(commands_new(root, "complete", "Get possible completions from a given command",
		NULL, cmd_store_env_and_pop, "complete"));
	commands_new(root, "help", "Get help on a possible command",
	    NULL, cmd_store_env_and_pop, "help");
	commands_new(
		commands_new(root, "pause", "Pause lldpd operations", NULL, NULL, NULL),
		NEWLINE, "Pause lldpd operations", NULL, cmd_pause, NULL);
	commands_new(
		commands_new(root, "resume", "Resume lldpd operations", NULL, NULL, NULL),
		NEWLINE, "Resume lldpd operations", NULL, cmd_resume, NULL);
	commands_new(
		commands_new(root, "exit", "Exit interpreter", NULL, NULL, NULL),
		NEWLINE, "Exit interpreter", NULL, cmd_exit, NULL);
	return root;
}

struct input {
	TAILQ_ENTRY(input) next;
	char *name;
};
TAILQ_HEAD(inputs, input);
static int
filter(const struct dirent *dir)
{
	if (strlen(dir->d_name) < 5) return 0;
	if (strcmp(dir->d_name + strlen(dir->d_name) - 5, ".conf")) return 0;
	return 1;
}

/**
 * Append a new input file/directory to the list of inputs.
 *
 * @param arg       Directory or file name to add.
 * @param inputs    List of inputs
 * @param acceptdir 1 if we accept a directory, 0 otherwise
 */
static void
input_append(const char *arg, struct inputs *inputs, int acceptdir, int warn)
{
	struct stat statbuf;
	if (stat(arg, &statbuf) == -1) {
		if (warn) {
			log_warn("lldpctl", "cannot find configuration file/directory %s",
			    arg);
		} else {
			log_debug("lldpctl", "cannot find configuration file/directory %s",
			    arg);
		}
		return;
	}

	if (!S_ISDIR(statbuf.st_mode)) {
		struct input *input = malloc(sizeof(struct input));
		if (!input) {
			log_warn("lldpctl", "not enough memory to process %s",
			    arg);
			return;
		}
		log_debug("lldpctl", "input: %s", arg);
		input->name = strdup(arg);
		TAILQ_INSERT_TAIL(inputs, input, next);
		return;
	}
	if (!acceptdir) {
		log_debug("lldpctl", "skip directory %s",
		    arg);
		return;
	}

	struct dirent **namelist = NULL;
	int n =	scandir(arg, &namelist, filter, alphasort);
	if (n < 0) {
		log_warnx("lldpctl", "unable to read directory %s",
		    arg);
		return;
	}
	for (int i=0; i < n; i++) {
		char *fullname;
		if (asprintf(&fullname, "%s/%s", arg, namelist[i]->d_name) != -1) {
			input_append(fullname, inputs, 0, 1);
			free(fullname);
		}
		free(namelist[i]);
	}
	free(namelist);
}

int
main(int argc, char *argv[])
{
	int ch, debug = 0, use_syslog = 0, rc = EXIT_FAILURE;
	const char *fmt = "plain";
	lldpctl_conn_t *conn = NULL;
	const char *options = is_lldpctl(argv[0])?"hdvf:u:":"hdsvf:c:C:u:";

	int gotinputs = 0, version = 0;
	struct inputs inputs;
	TAILQ_INIT(&inputs);

	ctlname = lldpctl_get_default_transport();

	signal(SIGHUP, SIG_IGN);

	/* Get and parse command line options */
	optind = 1;
	while ((ch = getopt(argc, argv, options)) != -1) {
		switch (ch) {
		case 'd':
			if (use_syslog)
				use_syslog = 0;
			else
				debug++;
			break;
		case 's':
			if (debug == 0)
				use_syslog = 1;
			else
				debug--;
			break;
		case 'h':
			usage();
			break;
		case 'u':
			ctlname = optarg;
			break;
		case 'v':
			version++;
			break;
		case 'f':
			fmt = optarg;
			break;
		case 'C':
		case 'c':
			if (!gotinputs) {
				log_init(use_syslog, debug, __progname);
				lldpctl_log_level(debug + 1);
				gotinputs = 1;
			}
			input_append(optarg, &inputs, 1, ch == 'c');
			break;
		default:
			usage();
		}
	}

	if (version) {
		version_display(stdout, "lldpcli", version > 1);
		exit(0);
	}

	if (!gotinputs) {
		log_init(use_syslog, debug, __progname);
		lldpctl_log_level(debug + 1);
	}

	/* Disable SIGPIPE */
	signal(SIGPIPE, SIG_IGN);

	/* Register commands */
	root = register_commands();

	/* Make a connection */
	log_debug("lldpctl", "connect to lldpd");
	conn = lldpctl_new_name(ctlname, NULL, NULL, NULL);
	if (conn == NULL) goto end;

	/* Process file inputs */
	while (gotinputs && !TAILQ_EMPTY(&inputs)) {
		/* coverity[use_after_free]
		   TAILQ_REMOVE does the right thing */
		struct input *first = TAILQ_FIRST(&inputs);
		log_debug("lldpctl", "process: %s", first->name);
		FILE *file = fopen(first->name, "r");
		if (file) {
			size_t n;
			ssize_t len;
			char *line;
			while (line = NULL, len = 0, (len = getline(&line, &n, file)) > 0) {
				if (line[len - 1] == '\n') {
					line[len - 1] = '\0';
					parse_and_exec(conn, fmt, line);
				}
				free(line);
			}
			free(line);
			fclose(file);
		} else {
			log_warn("lldpctl", "unable to open %s",
			    first->name);
		}
		TAILQ_REMOVE(&inputs, first, next);
		free(first->name);
		free(first);
	}

	/* Process additional arguments. First if we are lldpctl (interfaces) */
	if (is_lldpctl(NULL)) {
		char *line = NULL;
		for (int i = optind; i < argc; i++) {
			char *prev = line;
			if (asprintf(&line, "%s%s%s",
				prev?prev:"show neigh ports ", argv[i],
				(i == argc - 1)?" details":",") == -1) {
				log_warnx("lldpctl", "not enough memory to build list of interfaces");
				free(prev);
				goto end;
			}
			free(prev);
		}
		if (line == NULL && (line = strdup("show neigh details")) == NULL) {
			log_warnx("lldpctl", "not enough memory to build command line");
			goto end;
		}
		log_debug("lldpctl", "execute %s", line);
		if (parse_and_exec(conn, fmt, line) != -1)
			rc = EXIT_SUCCESS;
		free(line);
		goto end;
	}

	/* Then, if we are regular lldpcli (command line) */
	if (optind < argc) {
		const char **cargv;
		int cargc;
		cargv = &((const char **)argv)[optind];
		cargc = argc - optind;
		if (cmd_exec(conn, fmt, cargc, cargv) == 1)
			rc = EXIT_SUCCESS;
		goto end;
	}

	if (gotinputs) {
		rc = EXIT_SUCCESS;
		goto end;
	}

	/* Interactive session */
#ifdef HAVE_LIBREADLINE
	rl_bind_key('?',  cmd_help);
	rl_bind_key('\t', cmd_complete);
#endif
	char *line = NULL;
	do {
		if ((line = readline(prompt()))) {
			int n = parse_and_exec(conn, fmt, line);
			if (n != 0) {
#ifdef HAVE_READLINE_HISTORY
				add_history(line);
#endif
			}
			free(line);
		}
	} while (!must_exit && line != NULL);
	rc = EXIT_SUCCESS;

end:
	while (!TAILQ_EMPTY(&inputs)) {
		/* coverity[use_after_free]
		   TAILQ_REMOVE does the right thing */
		struct input *first = TAILQ_FIRST(&inputs);
		TAILQ_REMOVE(&inputs, first, next);
		free(first->name);
		free(first);
	}
	if (conn) lldpctl_release(conn);
	if (root) commands_free(root);
	return rc;
}
