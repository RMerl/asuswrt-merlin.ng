/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2012 Vincent Bernat <bernat@luffy.cx>
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

#include "client.h"
#include <string.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

/**
 * An element of the environment (a key and a value).
 */
struct cmd_env_el {
	TAILQ_ENTRY(cmd_env_el) next; /**< Next environment element */
	const char *key;	      /**< Key for this element */
	const char *value;	      /**< Value for this element */
};

/**
 * A stack element.
 */
struct cmd_env_stack {
	TAILQ_ENTRY(cmd_env_stack) next; /**< Next element, down the stack */
	struct cmd_node *el;		 /**< Stored element */
};

/**
 * Structure representing an environment for the current command.
 *
 * An environment is a list of values stored for use for the function executing
 * as well as the current command, the current position in the command and a
 * stack for cmd_node
 */
struct cmd_env {
	TAILQ_HEAD(, cmd_env_el) elements; /**< List of environment variables */
	TAILQ_HEAD(, cmd_env_stack) stack; /**< Stack */
	int argc;		/**< Number of argument in the command */
	int argp;		/**< Current argument */
	const char **argv;	/**< Arguments */
};

/**
 * Structure representing a command node.
 *
 * Such a node contains a token accepted to enter the node (or @c NULL if there
 * is no token needed), a documentation string to present the user, a function
 * to validate the user input (or @c NULL if no function is needed) and a
 * function to execute when entering the node. Because we can enter a node just
 * by completing, the execution part should have no other effect than modifying
 * the environment, with the exception of execution on @c NEWLINE (which cannot
 * happen when completing).
 */
struct cmd_node {
	TAILQ_ENTRY(cmd_node) next; /**< Next sibling */

	const char *token;	/**< Token to enter this cnode */
	const char *doc;	/**< Documentation string */
	int privileged;		/**< Privileged command? */
	int lock;		/**< Lock required for execution? */
	int hidden;		/**< Hidden command? */

	/**
	 * Function validating entry in this node. Can be @c NULL.
	 */
	int(*validate)(struct cmd_env*, void *);
	/**
	 * Function to execute when entering this node. May be @c NULL.
	 *
	 * This function can alter the environment
	 */
	int(*execute)(struct lldpctl_conn_t*, struct writer*,
	    struct cmd_env*, void *);
	void *arg;		/**< Magic argument for the previous two functions */

	/* List of possible subentries */
	TAILQ_HEAD(, cmd_node) subentries; /* List of subnodes */
};

/**
 * Create a root node.
 *
 * @return the root node.
 */
struct cmd_node*
commands_root(void)
{
	struct cmd_node *new = calloc(1, sizeof(struct cmd_node));
	if (new == NULL) fatalx("lldpctl", "out of memory");
	TAILQ_INIT(&new->subentries);
	return new;
}

/**
 * Make a node accessible only to privileged users.
 *
 * @param node node to change privileges
 * @return the modified node
 *
 * The node is modified. It is returned to ease chaining.
 */
struct cmd_node*
commands_privileged(struct cmd_node *node)
{
	if (node) node->privileged = 1;
	return node;
}

/**
 * Make a node accessible only with a lock.
 *
 * @param node node to use lock to execute
 * @return the modified node
 *
 * The node is modified. It is returned to ease chaining.
 */
struct cmd_node*
commands_lock(struct cmd_node *node)
{
	if (node) node->lock = 1;
	return node;
}

/**
 * Hide a node from help or completion.
 *
 * @param node node to hide
 * @return the modified node
 *
 * The node is modified. It is returned to ease chaining.
 */
struct cmd_node*
commands_hidden(struct cmd_node *node)
{
	if (node) node->hidden = 1;
	return node;
}

/**
 * Create a new node acessible by any user.
 *
 * @param root  The node we want to attach this node.
 * @param token Token to enter this node. Or @c NULL if no token is needed.
 * @param doc   Documentation for this node.
 * @param validate Function that should return 1 if we can enter the node.
 * @param execute  Function that should return 1 on successful execution of this node.
 * @param arg      Magic argument for precedent functions.
 * @return  the newly created node
 */
struct cmd_node*
commands_new(struct cmd_node *root,
    const char *token, const char *doc,
    int(*validate)(struct cmd_env*, void *),
    int(*execute)(struct lldpctl_conn_t*, struct writer*,
	struct cmd_env*, void *),
    void *arg)
{
	struct cmd_node *new = calloc(1, sizeof(struct cmd_node));
	if (new == NULL)
		fatalx("lldpctl", "out of memory");
	new->token = token;
	new->doc = doc;
	new->validate = validate;
	new->execute = execute;
	new->arg = arg;
	TAILQ_INIT(&new->subentries);
	TAILQ_INSERT_TAIL(&root->subentries, new, next);
	return new;
}

/**
 * Free a command tree.
 *
 * @param root The node we want to free.
 */
void
commands_free(struct cmd_node *root)
{
	struct cmd_node *subcmd, *subcmd_next;
	for (subcmd = TAILQ_FIRST(&root->subentries); subcmd != NULL;
	     subcmd = subcmd_next) {
		subcmd_next = TAILQ_NEXT(subcmd, next);
		TAILQ_REMOVE(&root->subentries, subcmd, next);
		commands_free(subcmd);
	}
	free(root);
}

/**
 * Return the current argument in the environment. This can be @c NEWLINE or
 * @c NULL.
 *
 * @param env The environment.
 * @return current argument.
 */
const char*
cmdenv_arg(struct cmd_env *env)
{
	if (env->argp < env->argc)
		return env->argv[env->argp];
	if (env->argp == env->argc)
		return NEWLINE;
	return NULL;
}

/**
 * Get a value from the environment.
 *
 * @param env The environment.
 * @param key The key for the requested value.
 * @return @c NULL if not found or the requested value otherwise. If no value is
 *         associated, return the key.
 */
const char*
cmdenv_get(struct cmd_env *env, const char *key)
{
	struct cmd_env_el *el;
	TAILQ_FOREACH(el, &env->elements, next)
		if (!strcmp(el->key, key))
			return el->value ? el->value : el->key;
	return NULL;
}

/**
 * Put a value in the environment.
 *
 * @param env The environment.
 * @param key The key for the value.
 * @param value The value.
 * @return 0 on success, -1 otherwise.
 */
int
cmdenv_put(struct cmd_env *env,
    const char *key, const char *value)
{
	struct cmd_env_el *el = malloc(sizeof(struct cmd_env_el));
	if (el == NULL) {
		log_warn("lldpctl", "unable to allocate memory for new environment variable");
		return -1;
	}
	el->key = key;
	el->value = value;
	TAILQ_INSERT_TAIL(&env->elements, el, next);
	return 0;
}

/**
 * Pop some node from the execution stack.
 *
 * This allows to resume parsing on a previous state. Useful to call after
 * parsing optional arguments.
 *
 * @param env The environment.
 * @param n How many element we want to pop.
 * @return 0 on success, -1 otherwise.
 */
int
cmdenv_pop(struct cmd_env *env, int n)
{
	while (n-- > 0) {
		if (TAILQ_EMPTY(&env->stack)) {
			log_warnx("lldpctl", "environment stack is empty");
			return -1;
		}
		struct cmd_env_stack *first = TAILQ_FIRST(&env->stack);
		TAILQ_REMOVE(&env->stack,
		    first, next);
		free(first);
	}
	return 0;
}

/**
 * Push some node on the execution stack.
 *
 * @param env The environment.
 * @param node The node to push.
 * @return 0 on success, -1 on error.
 */
static int
cmdenv_push(struct cmd_env *env, struct cmd_node *node)
{
	struct cmd_env_stack *el = malloc(sizeof(struct cmd_env_stack));
	if (el == NULL) {
		log_warn("lldpctl", "not enough memory to allocate a stack element");
		return -1;
	}
	el->el = node;
	TAILQ_INSERT_HEAD(&env->stack, el, next);
	return 0;
}

/**
 * Return the top of the stack, without poping it.
 *
 * @param env The environment.
 * @return the top element or @c NULL is the stack is empty.
 */
static struct cmd_node*
cmdenv_top(struct cmd_env *env)
{
	if (TAILQ_EMPTY(&env->stack)) return NULL;
	return TAILQ_FIRST(&env->stack)->el;
}

/**
 * Free execution environment.
 *
 * @param env The environment.
 */
static void
cmdenv_free(struct cmd_env *env)
{
	while (!TAILQ_EMPTY(&env->stack)) cmdenv_pop(env, 1);

	struct cmd_env_el *first;
	while (!TAILQ_EMPTY(&env->elements)) {
		first = TAILQ_FIRST(&env->elements);
		TAILQ_REMOVE(&env->elements, first, next);
		free(first);
	}
}

struct candidate_word {
	TAILQ_ENTRY(candidate_word) next;
	const char *word;
	const char *doc;
	int hidden;
};

/**
 * Execute or complete a command from the given node.
 *
 * @param conn    Connection to lldpd.
 * @param w       Writer for output.
 * @param root    Root node we want to start from.
 * @param argc    Number of arguments.
 * @param argv    Array of arguments.
 * @param word    Completed word. Or NULL when no completion is required.
 * @param all     When completing, display possible completions even if only one choice is possible.
 * @param priv    Is the current user privileged?
 * @return 0 on success, -1 otherwise.
 */
static int
_commands_execute(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_node *root, int argc, const char **argv,
    char **word, int all, int priv)
{
	int n, rc = 0, completion = (word != NULL);
	int help = 0;		/* Are we asking for help? */
	int complete = 0;	/* Are we asking for possible completions? */
	int needlock = 0;	/* Do we need a lock? */
	struct cmd_env env = {
		.elements = TAILQ_HEAD_INITIALIZER(env.elements),
		.stack = TAILQ_HEAD_INITIALIZER(env.stack),
		.argc = argc,
		.argv = argv,
		.argp = 0
	};
	static int lockfd = -1;
	cmdenv_push(&env, root);
	if (!completion)
		for (n = 0; n < argc; n++)
			log_debug("lldpctl", "argument %02d: `%s`", n, argv[n]);
	if (completion) *word = NULL;

#define CAN_EXECUTE(candidate) \
	((!candidate->privileged || priv || complete) && \
	    (!candidate->validate ||			\
		candidate->validate(&env, candidate->arg) == 1))

	/* When completion is in progress, we use the same algorithm than for
	 * execution until we reach the cursor position. */
	struct cmd_node *current = NULL;
	while ((current = cmdenv_top(&env))) {
		if (!completion) {
			help = !!cmdenv_get(&env, "help"); /* Are we asking for help? */
			complete = !!cmdenv_get(&env, "complete"); /* Or completion? */
		}

		struct cmd_node *candidate, *best = NULL;
		const char *token = (env.argp < env.argc) ? env.argv[env.argp] :
		    (env.argp == env.argc && !help && !complete) ? NEWLINE : NULL;
		if (token == NULL ||
		    (completion && env.argp == env.argc - 1))
			goto end;
		if (!completion)
			log_debug("lldpctl", "process argument %02d: `%s`",
			    env.argp, token);
		TAILQ_FOREACH(candidate, &current->subentries, next) {
			if (candidate->token &&
			    !strncmp(candidate->token, token, strlen(token)) &&
			    CAN_EXECUTE(candidate)) {
				if (candidate->token &&
				    !strcmp(candidate->token, token)) {
					/* Exact match */
					best = candidate;
					needlock = needlock || candidate->lock;
					break;
				}
				if (!best) best = candidate;
				else {
					if (!completion)
						log_warnx("lldpctl", "ambiguous token: %s (%s or %s)",
						    token, candidate->token, best->token);
					rc = -1;
					goto end;
				}
			}
		}
		if (!best) {
			/* Take first that validate */
			TAILQ_FOREACH(candidate, &current->subentries, next) {
				if (!candidate->token &&
				    CAN_EXECUTE(candidate)) {
					best = candidate;
					needlock = needlock || candidate->lock;
					break;
				}
			}
		}
		if (!best && env.argp == env.argc) goto end;
		if (!best) {
			if (!completion)
				log_warnx("lldpctl", "unknown command from argument %d: `%s`",
				    env.argp + 1, token);
			rc = -1;
			goto end;
		}

		/* Push and execute */
		cmdenv_push(&env, best);
		if (best->execute) {
			struct sockaddr_un su;
			if (needlock) {
				if (lockfd == -1) {
					log_debug("lldpctl", "reopen %s for locking", ctlname);
					if ((lockfd = socket(PF_UNIX, SOCK_STREAM, 0)) == -1) {
						log_warn("lldpctl", "cannot open for lock %s", ctlname);
						rc = -1;
						goto end;
					}
					su.sun_family = AF_UNIX;
					strlcpy(su.sun_path, ctlname, sizeof(su.sun_path));
					if (connect(lockfd, (struct sockaddr *)&su, sizeof(struct sockaddr_un)) == -1) {
						log_warn("lldpctl", "cannot connect to socket %s", ctlname);
						rc = -1;
						close(lockfd); lockfd = -1;
						goto end;
					}
				}
				if (lockf(lockfd, F_LOCK, 0) == -1) {
					log_warn("lldpctl", "cannot get lock on %s", ctlname);
					rc = -1;
					close(lockfd); lockfd = -1;
					goto end;
				}
			}
			rc = best->execute(conn, w, &env, best->arg) != 1 ? -1 : rc;
			if (needlock && lockf(lockfd, F_ULOCK, 0) == -1) {
				log_warn("lldpctl", "cannot unlock %s", ctlname);
				close(lockfd); lockfd = -1;
			}
			if (rc == -1) goto end;
		}
		env.argp++;
	}
end:
	if (!completion && !help && !complete) {
		if (rc == 0 && env.argp != env.argc + 1) {
			log_warnx("lldpctl", "incomplete command");
			rc = -1;
		}
	} else if (rc == 0 && (env.argp == env.argc - 1 || help || complete)) {
		/* We need to complete. Let's build the list of candidate words. */
		struct cmd_node *candidate = NULL;
		size_t maxl = 10;		    /* Max length of a word */
		TAILQ_HEAD(, candidate_word) words; /* List of subnodes */
		TAILQ_INIT(&words);
		current = cmdenv_top(&env);
		if (!TAILQ_EMPTY(&current->subentries)) {
			TAILQ_FOREACH(candidate, &current->subentries, next) {
				if ((!candidate->token || help || complete ||
					!strncmp(env.argv[env.argc - 1], candidate->token,
					    strlen(env.argv[env.argc -1 ]))) &&
				    CAN_EXECUTE(candidate)) {
					struct candidate_word *cword =
					    malloc(sizeof(struct candidate_word));
					if (!cword) break;
					cword->word = candidate->token;
					cword->doc = candidate->doc;
					cword->hidden = candidate->hidden;
					if (cword->word && strlen(cword->word) > maxl)
						maxl = strlen(cword->word);
					TAILQ_INSERT_TAIL(&words, cword, next);
				}
			}
		}
		if (!TAILQ_EMPTY(&words)) {
			/* Search if there is a common prefix, then return it. */
			char prefix[maxl + 2]; /* Extra space may be added at the end */
			struct candidate_word *cword, *cword_next;
			memset(prefix, 0, maxl+2);
			for (size_t n = 0; n < maxl; n++) {
				int c = 1; /* Set to 0 to exit outer loop */
				TAILQ_FOREACH(cword, &words, next) {
					c = 0;
					if (cword->hidden) continue;
					if (cword->word == NULL) break;
					if (!strcmp(cword->word, NEWLINE)) break;
					if (strlen(cword->word) == n) break;
					if (prefix[n] == '\0') prefix[n] = cword->word[n];
					else if (prefix[n] != cword->word[n]) break;
					c = 1;
				}
				if (c == 0) {
					prefix[n] = '\0';
					break;
				}
			}
			/* If the prefix is complete, add a space, otherwise,
			 * just return it as is. */
			if (!all && !help && !complete && strcmp(prefix, NEWLINE) &&
			    strlen(prefix) > 0 &&
			    strlen(env.argv[env.argc-1]) < strlen(prefix)) {
				TAILQ_FOREACH(cword, &words, next) {
					if (cword->word && !strcmp(prefix, cword->word)) {
						prefix[strlen(prefix)] = ' ';
						break;
					}
				}
				*word = strdup(prefix);
			} else {
				/* No common prefix, print possible completions */
				if (!complete)
					fprintf(stderr, "\n-- \033[1;34m%s\033[0m\n",
					    current->doc ? current->doc : "Help");
				TAILQ_FOREACH(cword, &words, next) {
					if (cword->hidden) continue;

					char fmt[100];
					if (!complete) {
						snprintf(fmt, sizeof(fmt),
						    "%s%%%ds%s  %%s\n",
						    "\033[1m", (int)maxl, "\033[0m");
						fprintf(stderr, fmt,
						    cword->word ? cword->word : "WORD",
						    cword->doc ?  cword->doc  : "...");
					} else {
						if (!cword->word || !strcmp(cword->word, NEWLINE))
							continue;
						fprintf(stdout, "%s %s\n",
						    cword->word ? cword->word : "WORD",
						    cword->doc ?  cword->doc  : "...");
					}
				}
			}
			for (cword = TAILQ_FIRST(&words); cword != NULL;
			     cword = cword_next) {
				cword_next = TAILQ_NEXT(cword, next);
				TAILQ_REMOVE(&words, cword, next);
				free(cword);
			}
		}
	}
	cmdenv_free(&env);
	return rc;
}

/**
 * Complete the given command.
 */
char *
commands_complete(struct cmd_node *root, int argc, const char **argv,
    int all, int privileged)
{
	char *word = NULL;
	if (_commands_execute(NULL, NULL, root, argc, argv,
		&word, all, privileged) == 0)
		return word;
	return NULL;
}

/**
 * Execute the given commands.
 */
int
commands_execute(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_node *root, int argc, const char **argv, int privileged)
{
	return _commands_execute(conn, w, root, argc, argv, NULL, 0, privileged);
}

/**
 * Check if the environment does not contain the given key.
 *
 * @param env The environment.
 * @param key The key to search for.
 * @return 1 if the environment does not contain the key. 0 otherwise.
 */
int
cmd_check_no_env(struct cmd_env *env, void *key)
{
	return cmdenv_get(env, (const char*)key) == NULL;
}

/**
 * Check if the environment does contain the given key.
 *
 * @param env The environment.
 * @param key The key to search for. Can be a comma separated list.
 * @return 1 if the environment does contain the key. 0 otherwise.
 */
int
cmd_check_env(struct cmd_env *env, void *key)
{
	struct cmd_env_el *el;
	const char *list = key;
	int count = 0;
	TAILQ_FOREACH(el, &env->elements, next) {
		if (contains(list, el->key))
			count++;
	}
	while ((list = strchr(list, ','))) { list++; count--; }
	return (count == 1);
}

/**
 * Store the given key in the environment.
 *
 * @param conn The connection.
 * @param w    The writer (not used).
 * @param env The environment.
 * @param key The key to store.
 * @return 1 if the key was stored
 */
int
cmd_store_env(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *key)
{
	return cmdenv_put(env, key, NULL) != -1;
}

/**
 * Store the given key in the environment and pop one element from the stack.
 *
 * @param conn The connection.
 * @param w    The writer (not used).
 * @param env The environment.
 * @param key The key to store.
 * @return 1 if the key was stored
 */
int
cmd_store_env_and_pop(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *key)
{
	return (cmd_store_env(conn, w, env, key) != -1 &&
	    cmdenv_pop(env, 1) != -1);
}

/**
 * Store the given key with a value being the current keyword in the environment
 * and pop X elements from the stack.
 *
 * @param conn The connection.
 * @param w    The writer (not used).
 * @param env The environment.
 * @param key The key to store.
 * @return 1 if the key was stored
 */
int
cmd_store_env_value_and_pop(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *key)
{
	return (cmdenv_put(env, key, cmdenv_arg(env)) != -1 &&
	    cmdenv_pop(env, 1) != -1);
}
int
cmd_store_env_value_and_pop2(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *key)
{
	return (cmdenv_put(env, key, cmdenv_arg(env)) != -1 &&
	    cmdenv_pop(env, 2) != -1);
}
int
cmd_store_env_value(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *key)
{
	return (cmdenv_put(env, key, cmdenv_arg(env)) != -1);
}
int
cmd_store_env_value_and_pop3(struct lldpctl_conn_t *conn, struct writer *w,
    struct cmd_env *env, void *key)
{
	return (cmdenv_put(env, key, cmdenv_arg(env)) != -1 &&
	    cmdenv_pop(env, 3) != -1);
}
int
cmd_store_something_env_value_and_pop2(const char *what,
    struct cmd_env *env, void *value)
{
	return (cmdenv_put(env, what, value) != -1 &&
	    cmdenv_pop(env, 2) != -1);
}
int
cmd_store_something_env_value(const char *what,
    struct cmd_env *env, void *value)
{
	return (cmdenv_put(env, what, value) != -1);
}

/**
 * Provide an iterator on all interfaces contained in "ports".
 *
 * @warning This function is not reentrant. It uses static variables to keep
 * track of ports that have already been provided. Moreover, to release all
 * resources, the iterator should be used until its end.
 *
 * @param conn The connection.
 * @param env  The environment.
 * @return The next interface in the set of ports (or in all ports if no `ports`
 *         variable is present in the environment)
 */
lldpctl_atom_t*
cmd_iterate_on_interfaces(struct lldpctl_conn_t *conn, struct cmd_env *env)
{
	static lldpctl_atom_iter_t *iter = NULL;
	static lldpctl_atom_t *iface_list = NULL;
	static lldpctl_atom_t *iface = NULL;
	const char *interfaces = cmdenv_get(env, "ports");

	do {
		if (iter == NULL) {
			iface_list = lldpctl_get_interfaces(conn);
			if (!iface_list) {
				log_warnx("lldpctl", "not able to get the list of interfaces. %s",
				    lldpctl_last_strerror(conn));
				return NULL;
			}
			iter = lldpctl_atom_iter(iface_list);
			if (!iter) return NULL;
		} else {
			iter = lldpctl_atom_iter_next(iface_list, iter);
			if (iface) {
				lldpctl_atom_dec_ref(iface);
				iface = NULL;
			}
			if (!iter) {
				lldpctl_atom_dec_ref(iface_list);
				return NULL;
			}
		}

		iface = lldpctl_atom_iter_value(iface_list, iter);
	} while (interfaces && !contains(interfaces,
		lldpctl_atom_get_str(iface, lldpctl_k_interface_name)));

	return iface;
}

/**
 * Provide an iterator on all ports contained in "ports", as well as the
 * default port.
 *
 * @warning This function is not reentrant. It uses static variables to keep
 * track of ports that have already been provided. Moreover, to release all
 * resources, the iterator should be used until its end.
 *
 * @param conn The connection.
 * @param env  The environment.
 * @param name Name of the interface (for logging purpose)
 * @return The next interface in the set of ports (or in all ports if no `ports`
 *         variable is present in the environment), including the default port
 *         if no `ports` variable is present in the environment.
 */
lldpctl_atom_t*
cmd_iterate_on_ports(struct lldpctl_conn_t *conn, struct cmd_env *env, const char **name)
{
	static int put_default = 0;
	static lldpctl_atom_t *last_port = NULL;
	const char *interfaces = cmdenv_get(env, "ports");

	if (last_port) {
		lldpctl_atom_dec_ref(last_port);
		last_port = NULL;
	}
	if (!put_default) {
		lldpctl_atom_t *iface = cmd_iterate_on_interfaces(conn, env);
		if (iface) {
			*name = lldpctl_atom_get_str(iface, lldpctl_k_interface_name);
			last_port = lldpctl_get_port(iface);
			return last_port;
		}
		if (!interfaces) {
			put_default = 1;
			*name = "(default)";
			last_port = lldpctl_get_default_port(conn);
			return last_port;
		}
		return NULL;
	} else {
		put_default = 0;
		return NULL;
	}
}

/**
 * Restrict the command to some ports.
 */
void
cmd_restrict_ports(struct cmd_node *root)
{
	/* Restrict to some ports. */
	commands_new(
		commands_new(root,
		    "ports",
		    "Restrict configuration to some ports",
		    cmd_check_no_env, NULL, "ports"),
		NULL,
		"Restrict configuration to the specified ports (comma-separated list)",
		NULL, cmd_store_env_value_and_pop2, "ports");
}

/**
 * Restrict the command to specific protocol
 */
void
cmd_restrict_protocol(struct cmd_node *root)
{
	/* Restrict to some ports. */
	commands_new(
		commands_new(root,
		    "protocol",
		    "Restrict to specific protocol",
		    cmd_check_no_env, NULL, "protocol"),
		NULL,
		"Restrict display to the specified protocol",
		NULL, cmd_store_env_value_and_pop2, "protocol");
}
