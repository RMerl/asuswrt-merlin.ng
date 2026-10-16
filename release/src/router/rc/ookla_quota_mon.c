/*
 * ookla_quota_mon - background quota monitor for Ookla Speedtest
 *
 * Usage: ookla_quota_mon <ookla_pid> <phase1>
 *
 *   ookla_pid : PID of the running ookla process (used as session ID)
 *   phase1    : "1" if Phase 1 free-100-use quota applies
 *               (RTCONFIG_HND_ROUTER_BE_4916), "0" otherwise
 *
 * The monitor polls the ookla process up to OOKLA_MON_TIMEOUT seconds
 * (matches the UI test_timeout of 120 s defined in internet_speed.html).
 * When the process exits it inspects OOKLA_RESULT for a successful
 * "type":"result" line and increments the ookla_quota nvram counter.
 *
 * OOKLA_SESSION verifies the monitor is watching the current run and not
 * a stale session from a previous test.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>

#include <bcmnvram.h>
#include <shutils.h>
#include <shared.h>

/* Maximum wait time in seconds — equals UI test_timeout (120 000 ms) */
#define OOKLA_MON_TIMEOUT  120

/* ------------------------------------------------------------------ */

static int
read_file_str(const char *path, char *buf, int len)
{
	FILE *fp = fopen(path, "r");
	if (!fp) return 0;
	if (!fgets(buf, len, fp)) { fclose(fp); buf[0] = '\0'; return 0; }
	fclose(fp);
	buf[strcspn(buf, "\n")] = '\0';
	return 1;
}

static int
write_file_str(const char *path, const char *str)
{
	FILE *fp = fopen(path, "w");
	if (!fp) return 0;
	fprintf(fp, "%s\n", str);
	fclose(fp);
	return 1;
}

static int
result_has_success(void)
{
	FILE *fp;
	char buf[4096];
	size_t n;

	if ((fp = fopen(OOKLA_RESULT, "r")) == NULL) return 0;

	/* "type":"result" is always on the last line; read only the tail */
	if (fseek(fp, -(long)sizeof(buf), SEEK_END) != 0)
		rewind(fp);  /* file smaller than buf */

	n = fread(buf, 1, sizeof(buf) - 1, fp);
	fclose(fp);

	if (!n) return 0;
	buf[n] = '\0';
	return strstr(buf, "\"type\":\"result\"") != NULL ? 1 : 0;
}

static void
do_quota_increment(int phase1)
{
	int phase1_cnt = 0, daily_cnt = 0;
	long daily_ts = 0;
	time_t now;
	char buf[20];

	/* Validate nvram quota format */
	if (sscanf(nvram_safe_get("ookla_quota"), "%d<%ld<%d",
	           &phase1_cnt, &daily_ts, &daily_cnt) != 3
	    || phase1_cnt < 0    || phase1_cnt > OOKLA_QUOTA_PHASE1_MAX
	    || daily_ts < 0
	    || daily_cnt < 0 || daily_cnt > OOKLA_QUOTA_DAILY_MAX)
		return;

	if (phase1 && phase1_cnt < OOKLA_QUOTA_PHASE1_MAX) {
		/* Phase 1 (RTCONFIG_HND_ROUTER_BE_4916): increment phase1_cnt */
		snprintf(buf, sizeof(buf), "%d<%ld<%d", phase1_cnt + 1, daily_ts, daily_cnt);
	} else {
		/* Phase 2: 24-hour counting period */
		now = time(NULL);
		if (((long)now - daily_ts) >= OOKLA_QUOTA_WINDOW_SEC || daily_ts > (long)now) {
			daily_ts  = (long)now;
			daily_cnt = 0;
		}
		snprintf(buf, sizeof(buf), "%d<%ld<%d", phase1_cnt, daily_ts, daily_cnt + 1);
	}

	nvram_set("ookla_quota", buf);
	nvram_commit();
}

/* ------------------------------------------------------------------ */

int
ookla_quota_mon_main(int argc, char *argv[])
{
	pid_t  ookla_pid;
	int    phase1;
	int    i;
	char   session[16]         = {0};
	char   current_session[16] = {0};

	if (argc < 3) return 1;

	ookla_pid = (pid_t)atoi(argv[1]);
	phase1    = atoi(argv[2]);

	if (ookla_pid <= 0) return 1;

	snprintf(session, sizeof(session), "%d", (int)ookla_pid);

	/* Poll until ookla exits or timeout (1 s steps, max OOKLA_MON_TIMEOUT) */
	for (i = 0; i < OOKLA_MON_TIMEOUT; i++) {
		if (kill(ookla_pid, 0) != 0) break;
		sleep(1);
	}

	/* Verify this session is still the active one */
	if (!read_file_str(OOKLA_SESSION, current_session, sizeof(current_session)))
		return 0;
	if (strcmp(session, current_session) != 0)
		return 0;

	/* Require a successful result line */
	if (!result_has_success()) return 0;

	do_quota_increment(phase1);
	return 0;
}
