#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <shadow.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <crypt.h>

static int asus_libpasswd_openssl_crypt(char *key, char *salt, char *out, int out_len)
{
	dbg("asus_openssl_crypt: check toolchain crypt() support\n");
	int scheme_id = 0, ret = 0;
	int stdin_pipe[2], stdout_pipe[2];
	pid_t pid;
	char crypt_buf[256] = {0};
	char salt_arg[128] = {0};
	char scheme_arg[4] = {0};

	if(salt && strlen(salt) > 4){
		if(!strncmp(salt, "$1$", 3))
			scheme_id = 1;
		else if(!strncmp(salt, "$5$", 3))
			scheme_id = 5;
		else
			return ret;
	}else
		return ret;

	/* Sanitize salt to prevent argument injection.
	 * Key is passed via stdin so it never touches the shell. */
	if(strpbrk(salt+3, "'\";&|`$(){}[]\\!#~\n\r "))
		return ret;

	snprintf(scheme_arg, sizeof(scheme_arg), "-%d", scheme_id);
	strlcpy(salt_arg, salt+3, sizeof(salt_arg));

	/* Use pipe+fork+exec to pass password via stdin to openssl,
	 * avoiding shell injection and /proc/cmdline exposure. */
	if(pipe(stdin_pipe) < 0)
		return ret;
	if(pipe(stdout_pipe) < 0) {
		close(stdin_pipe[0]);
		close(stdin_pipe[1]);
		return ret;
	}

	pid = fork();
	if(pid < 0) {
		close(stdin_pipe[0]); close(stdin_pipe[1]);
		close(stdout_pipe[0]); close(stdout_pipe[1]);
		return ret;
	}

	if(pid == 0) {
		/* Child: stdin from pipe, stdout to pipe */
		close(stdin_pipe[1]);
		close(stdout_pipe[0]);
		dup2(stdin_pipe[0], STDIN_FILENO);
		dup2(stdout_pipe[1], STDOUT_FILENO);
		close(stdin_pipe[0]);
		close(stdout_pipe[1]);
		execlp("openssl", "openssl", "passwd", scheme_arg,
			"-salt", salt_arg, "-stdin", NULL);
		_exit(127);
	}

	/* Parent: write password to child's stdin, read hash from stdout */
	close(stdin_pipe[0]);
	close(stdout_pipe[1]);

	write(stdin_pipe[1], key, strlen(key));
	write(stdin_pipe[1], "\n", 1);
	close(stdin_pipe[1]);

	ssize_t n = read(stdout_pipe[0], crypt_buf, sizeof(crypt_buf) - 1);
	close(stdout_pipe[0]);

	int status;
	waitpid(pid, &status, 0);

	if(n > 0) {
		crypt_buf[n] = '\0';
		/* Strip trailing newline */
		if(n > 0 && crypt_buf[n-1] == '\n')
			crypt_buf[n-1] = '\0';
	}

	if(crypt_buf[0] != '\0'){
		if(!strncmp(crypt_buf, salt, strlen(salt))){
			strlcpy(out, crypt_buf, out_len);
			ret = 1;
		}
	}

	return ret;
}

int compare_passwd_in_shadow(const char *username, const char *passwd)
{
	char *salt, *correct, *p, *supplied;
	struct spwd *shadow_entry;
	char crypt_buf[256] = {0};

	if(!username || !passwd)
		return 0;

	shadow_entry = getspnam(username);

	if(!shadow_entry)
		return 0;

	correct = shadow_entry->sp_pwdp;

	salt = strdup(correct);

	if (salt == NULL)
		goto ERROR;

	p = strchr(salt + 1, '$');
	if (p == NULL)
		goto ERROR;

	p = strchr(p + 1, '$');
	if (p == NULL)
		goto ERROR;
	p[1] = 0;

	supplied = crypt(passwd, salt);

	if (supplied == NULL) {
		asus_libpasswd_openssl_crypt((char *)passwd, salt, crypt_buf, sizeof(crypt_buf));
		supplied = crypt_buf;
	}
	if(supplied == NULL || *supplied == '\0')
		goto ERROR;

	free(salt);
	return !strcmp(supplied, correct);

ERROR:
	if(salt)
		free(salt);
	return 0;
}
