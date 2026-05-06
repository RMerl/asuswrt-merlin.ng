#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <shutils.h>
#include <shared.h>
#include <bcmnvram.h>

#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define SEND_CONTENT	"/tmp/send.tmp"
#define EMAIL_CONF	"/tmp/email.conf"

int sendm_main(int argc, char **argv)
{
	FILE *fp;
	char *np, *ns, *mta, *mport, *myname, *mymail, *auth, *auth_user, *auth_pass, *to_mail, cmd_buf[128];
	
	mta = nvram_safe_get("smtp_gw");
	mport = nvram_safe_get("smtp_port");
	myname = nvram_safe_get("myname");
	mymail = nvram_safe_get("mymail");
	auth = nvram_get_int("smtp_auth")==1?"LOGIN":"PLAIN";
	auth_user = nvram_safe_get("auth_user");
	auth_pass = nvram_safe_get("auth_pass");
	to_mail = nvram_safe_get("to_mail");

	if ((fp = fopen(EMAIL_CONF, "w+")) != NULL) {
		fprintf(fp, "SMTP_SERVER = '%s'\n", mta);
		fprintf(fp, "SMTP_PORT = '%s'\n", mport);
		fprintf(fp, "MY_NAME = '%s'\n", myname);
		fprintf(fp, "MY_EMAIL = '%s'\n", mymail);
		fprintf(fp, "USE_TLS = 'true'\n");
		fprintf(fp, "SMTP_AUTH = '%s'\n", auth);
		fprintf(fp, "SMTP_AUTH_USER = '%s'\n", auth_user);
		fprintf(fp, "SMTP_AUTH_PASS = '%s'\n", auth_pass);
	}
	fclose(fp);

	np = nvram_safe_get("sendmsg");
	ns = nvram_safe_get("sendsub");

	if(!np || !*np)
		np = "no report";
	if(!ns || !*ns)
		ns = "report";

	if((fp = fopen(SEND_CONTENT, "w+")) != NULL){
		fprintf(fp, "%s", np);
	}
	fclose(fp);

	/* Use fork+execvp to avoid command injection via ns/to_mail.
	 * Redirect stdin from SEND_CONTENT so email reads the body directly. */
	{
		int fd = open(SEND_CONTENT, O_RDONLY);
		if (fd < 0) return -1;

		pid_t pid = fork();
		if (pid < 0) { close(fd); return -1; }

		if (pid == 0) {
			/* child: redirect stdin from SEND_CONTENT */
			dup2(fd, STDIN_FILENO);
			close(fd);
			char *argv_exec[] = { "email", "-c", EMAIL_CONF, "-s", ns, (char *)to_mail, NULL };
			execvp("email", argv_exec);
			_exit(errno);
		}

		close(fd);
		int status = 0;
		waitpid(pid, &status, 0);
		return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
	}
}
