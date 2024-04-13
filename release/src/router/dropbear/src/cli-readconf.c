/*
 * Dropbear - a SSH2 server
 *
 * Copyright (c) 2023 TJ Kolev
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */

#include "dbutil.h"
#include "runopts.h"

#if DROPBEAR_USE_SSH_CONFIG

#define TOKEN_CHARS " =\t\n"

static const size_t MAX_CONF_LINE = 200;

typedef enum {
	opInvalid = -1,
	opHost,
	opHostName,
	opHostPort,
	opLoginUser,
	opIdentityFile,
} cfg_option;

static const struct {
	const char *name;
	cfg_option option;
}
config_options[] = {
	/* Start of config section. */
	{ "host", opHost },

	{ "hostname", opHostName },
	{ "port", opHostPort },
	{ "user", opLoginUser },
	{ "identityfile", opIdentityFile },

	/* End loop condition. */
	{ NULL, opInvalid },
};

void read_config_file(char* filename, FILE* config_file, cli_runopts* options) {
	DEBUG1(("Reading configuration data '%.200s'", filename));

	char *line = NULL;
	int linenum = 0;
	buffer *buf = NULL;

	char* cfg_key;
	char* cfg_val;
	char* saveptr;

	int in_host_section = 0;

	buf = buf_new(MAX_CONF_LINE);
	line = buf->data;
	while (buf_getline(buf, config_file) == DROPBEAR_SUCCESS) {
		/* Update line number counter. */
		linenum++;

		/* Add nul terminator */
		if (buf->len == buf->size) {
			dropbear_exit("Long line %d", linenum);
		}
		buf_setpos(buf, buf->len);
		buf_putbyte(buf, '\0');
		buf_setpos(buf, 0);

		char* commentStart = strchr(line, '#');
		if (NULL != commentStart) {
			*commentStart = '\0'; /* Drop the comments. */
		}

		cfg_key = strtok_r(line, TOKEN_CHARS, &saveptr);
		if (NULL == cfg_key) {
			continue;
		}

		cfg_option cfg_opt = opInvalid;
		for (int i = 0; config_options[i].name; i++) {
			if (0 == strcasecmp(cfg_key, config_options[i].name)) {
				cfg_opt = config_options[i].option;
				break;
			}
		}

		if (opInvalid == cfg_opt) {
			dropbear_exit("Unhandled key %s at '%s':%d.", cfg_key, filename, linenum);
		}


		cfg_val = strtok_r(NULL, TOKEN_CHARS, &saveptr);
		if (NULL == cfg_val) {
			dropbear_exit("Missing value for key %s at '%s':%d.", cfg_key, filename, linenum);
		}

		if (in_host_section) {
			if (opHost == cfg_opt) {
				/* Hit the next host section. Done reading config. */
				break;
			}
			switch (cfg_opt) {
				case opHostName: {
					/* The host name is the alias given on the command line.
					 * Set the actual remote host specified in the config.
					 */
					m_free(options->remotehost);
					options->remotehost = m_strdup(cfg_val);
					options->remotehostfixed = 1; /* Subsequent command line parsing should leave it alone. */
					break;
				}

				case opHostPort: {
					m_free(options->remoteport);
					options->remoteport = m_strdup(cfg_val);
					break;
				}

				case opLoginUser: {
					m_free(options->username);
					options->username = m_strdup(cfg_val);
					break;
				}

				case opIdentityFile: {
#if DROPBEAR_CLI_PUBKEY_AUTH
					char* key_file_path;
					if (strncmp(cfg_val, "~/", 2) == 0) {
						key_file_path = expand_homedir_path(cfg_val);
					} else if (cfg_val[0] != '/') {
						char* config_dir = dirname(filename);
						int path_len = strlen(config_dir) + strlen(cfg_val) + 10;
						key_file_path = m_malloc(path_len);
						snprintf(key_file_path, path_len, "%s/%s", config_dir, cfg_val);
					} else {
						key_file_path = m_strdup(cfg_val);
					}
					loadidentityfile(key_file_path, 1);
					m_free(key_file_path);
#else
					dropbear_exit("This version of the code does not support identity file. %s at '%s':%d.", cfg_key, filename, linenum);
#endif
					break;
				}

				default: {
					dropbear_exit("Unsupported configuration option %s at '%s':%d.", cfg_key, filename, linenum);
				}
			}
		}
		else
		{
			if (opHost != cfg_opt || 0 != strcmp(cfg_val, options->remotehost)) {
				/* Not our host section. */
				continue;
			}
			in_host_section = 1;
		}
	}
	buf_free(buf);
}

#endif /* DROPBEAR_USE_SSH_CONFIG */
