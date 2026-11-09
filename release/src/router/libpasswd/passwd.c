#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <shadow.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <crypt.h>
#include <errno.h>

/* Self-contained safe exec helper (fork+pipe+execvp, no shell). This file
 * builds as a standalone shared library (only linked against -lcrypt), so
 * it cannot call into shared/shutils.c's safe_popen_argv() without adding
 * a new cross-library dependency; this is a local equivalent instead. */
static int local_popen_argv(const char *path, char *const argv[], pid_t *child_pid)
{
	int pipefd[2];
	pid_t pid;

	if (pipe(pipefd) == -1)
		return -1;

	pid = fork();
	if (pid < 0) {
		close(pipefd[0]);
		close(pipefd[1]);
		return -1;
	}

	if (pid == 0) {
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);
		execvp(path, argv);
		_exit(errno);
	}

	close(pipefd[1]);
	if (child_pid)
		*child_pid = pid;

	return pipefd[0];
}

static void local_pclose(pid_t pid)
{
	int status;

	if (pid <= 0)
		return;

	while (waitpid(pid, &status, 0) == -1 && errno == EINTR)
		continue;
}

static int asus_libpasswd_openssl_crypt(char *key, char *salt, char *out, int out_len)
{
	dbg("asus_openssl_crypt: check toolchain crypt() support\n");
	int scheme_id = 0, ret = 0;
	char scheme_arg[4] = {0}, crypt_buf[256] = {0};
	char *argv[7];
	int fd;
	pid_t pid;

	if(salt && strlen(salt) > 4){
		if(!strncmp(salt, "$1$", 3))
			scheme_id = 1;
		else if(!strncmp(salt, "$5$", 3))
			scheme_id = 5;
		else
			return ret;
	}else
		return ret;

	snprintf(scheme_arg, sizeof(scheme_arg), "-%d", scheme_id);
	argv[0] = "openssl";
	argv[1] = "passwd";
	argv[2] = scheme_arg;
	argv[3] = "-salt";
	argv[4] = salt+3;
	argv[5] = key;
	argv[6] = NULL;

	fd = local_popen_argv("openssl", argv, &pid);
	if(fd >= 0){
		FILE *p_fp = fdopen(fd, "r");
		if(p_fp){
			if(fgets(crypt_buf, sizeof(crypt_buf), p_fp)){
				if (strlen(crypt_buf) > 0)
					crypt_buf[strlen(crypt_buf)-1] = '\0';
			}
			fclose(p_fp);
		}else
			close(fd);
		local_pclose(pid);
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
		asus_libpasswd_openssl_crypt(passwd, salt, crypt_buf, sizeof(crypt_buf));
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



