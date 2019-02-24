/*
 * Copyright (c) 2014 Vyronas Tsingaras (vtsingaras@it.auth.gr)
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/* for vasprintf() */
#define _GNU_SOURCE
#include "ext_auth_listener.h"

#include <daemon.h>
#include <utils/process.h>

#include <stdio.h>
#include <unistd.h>

typedef struct private_ext_auth_listener_t private_ext_auth_listener_t;

/**
 * Private data of an ext_auth_listener_t object.
 */
struct private_ext_auth_listener_t {

	/**
	 * Public ext_auth_listener_listener_t interface.
	 */
	ext_auth_listener_t public;

	/**
	 * Path to authorization program
	 */
	char *script;
};

/**
 * Allocate and push a format string to the environment
 */
static bool push_env(char *envp[], u_int count, char *fmt, ...)
{
	int i = 0;
	char *str;
	va_list args;

	while (envp[i])
	{
		if (++i + 1 >= count)
		{
			return FALSE;
		}
	}
	va_start(args, fmt);
	if (vasprintf(&str, fmt, args) >= 0)
	{
		envp[i] = str;
	}
	va_end(args);
	return envp[i] != NULL;
}

/**
 * Free all allocated environment strings
 */
static void free_env(char *envp[])
{
	int i;

	for (i = 0; envp[i]; i++)
	{
		free(envp[i]);
	}
}

METHOD(listener_t, authorize, bool,
	private_ext_auth_listener_t *this, ike_sa_t *ike_sa,
	bool final, bool *success)
{
	if (final)
	{
		FILE *shell;
		process_t *process;
		char *envp[32] = {};
		int out, retval;

		*success = FALSE;

		push_env(envp, countof(envp), "IKE_UNIQUE_ID=%u",
				 ike_sa->get_unique_id(ike_sa));
		push_env(envp, countof(envp), "IKE_NAME=%s",
				 ike_sa->get_name(ike_sa));

		push_env(envp, countof(envp), "IKE_LOCAL_HOST=%H",
				 ike_sa->get_my_host(ike_sa));
		push_env(envp, countof(envp), "IKE_REMOTE_HOST=%H",
				 ike_sa->get_other_host(ike_sa));

		push_env(envp, countof(envp), "IKE_LOCAL_ID=%Y",
				 ike_sa->get_my_id(ike_sa));
		push_env(envp, countof(envp), "IKE_REMOTE_ID=%Y",
				 ike_sa->get_other_id(ike_sa));

		if (ike_sa->has_condition(ike_sa, COND_EAP_AUTHENTICATED) ||
			ike_sa->has_condition(ike_sa, COND_XAUTH_AUTHENTICATED))
		{
			push_env(envp, countof(envp), "IKE_REMOTE_EAP_ID=%Y",
					 ike_sa->get_other_eap_id(ike_sa));
		}

		process = process_start_shell(envp, NULL, &out, NULL,
									  "2>&1 %s", this->script);
		if (process)
		{
			shell = fdopen(out, "r");
			if (shell)
			{
				while (TRUE)
				{
					char resp[128], *e;

					if (fgets(resp, sizeof(resp), shell) == NULL)
					{
						if (ferror(shell))
						{
							DBG1(DBG_CFG, "error reading from ext-auth script");
						}
						break;
					}
					else
					{
						e = resp + strlen(resp);
						if (e > resp && e[-1] == '\n')
						{
							e[-1] = '\0';
						}
						DBG1(DBG_CHD, "ext-auth: %s", resp);
					}
				}
				fclose(shell);
			}
			else
			{
				close(out);
			}
			if (process->wait(process, &retval))
			{
				if (retval == EXIT_SUCCESS)
				{
					*success = TRUE;
				}
				else
				{
					DBG1(DBG_CFG, "rejecting IKE_SA for ext-auth result: %d",
						 retval);
				}
			}
		}
		free_env(envp);
	}
	return TRUE;
}

METHOD(ext_auth_listener_t, destroy, void,
	private_ext_auth_listener_t *this)
{
	free(this);
}

/**
 * See header
 */
ext_auth_listener_t *ext_auth_listener_create(char *script)
{
	private_ext_auth_listener_t *this;

	INIT(this,
		.public = {
			.listener = {
				.authorize = _authorize,
			},
			.destroy = _destroy,
		},
		.script = script,
	);

	return &this->public;
}
