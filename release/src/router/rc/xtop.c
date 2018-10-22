
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <stdint.h>
#include <limits.h>
#include <dirent.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <timer_utils.h>

typedef struct top_status_t {
	unsigned long ticks;
	unsigned pcpu; /* delta of ticks */
	unsigned pid, ppid;
} top_status_t;

typedef struct jiffy_counts_t {
	/* Linux 2.4.x has only first four */
	unsigned long long usr, nic, sys, idle;
	unsigned long long iowait, irq, softirq, steal;
	unsigned long long total;
	unsigned long long busy;
} jiffy_counts_t;

typedef struct procps_status_t {
	/* Everything below must contain no ptrs to malloc'ed data:
	 * it is memset(0) for each process in procps_scan() */
	unsigned long vsz, rss; /* we round it to kbytes */
	unsigned long stime, utime;
	unsigned long start_time;
	unsigned pid;
	unsigned ppid;
	unsigned pgid;
	unsigned sid;
	unsigned uid;
	unsigned gid;
	unsigned tty_major,tty_minor;
	char state[4];
	char comm[16];
	int last_seen_on_cpu;
} procps_status_t;

typedef struct save_hist {
	unsigned long ticks;
	pid_t pid;
} save_hist;

static top_status_t *top;
static jiffy_counts_t cur_jif, prev_jif;
static jiffy_counts_t *cpu_jif, *cpu_prev_jif;
static char line_buf[80];

#define LATE_TASK	-100
#define MAX_PD 		20

typedef struct xtop_process_info {
	char *xtask;
	pid_t xpd;
	void (*xfunc)(void);
	unsigned int xratio;
	unsigned int tct;	/* task's cpu-threshold */
	unsigned int exc;	/* excute it after exc reach ceil */
	unsigned long pticks; 
	unsigned long pre_pticks;
} xpi_t;
xpi_t xpis[MAX_PD];

static unsigned long systicks_total, systicks_busy, pre_systicks_total, pre_systicks_busy, sysdiff_total, sysdiff_busy;
static char *tasks;

static void xtop_restart_mdns(void);
static void stop_xbusy(void);

#define HCPU_CEIL	3	/* when hcpu loops 3 times */
struct highcpu_task_handler {
	const char *taskname;
	unsigned int tct;	/* task's cpu-threshold */
	void (*func)(void);
};

static struct highcpu_task_handler xtop_hcpu_task_t[] =
{
	{"avahi-daemon", 40, xtop_restart_mdns},
	{"xbusy", 80, stop_xbusy},	/* test only */
        {0, 0, 0, 0}
};
static int hh_xnum = sizeof(xtop_hcpu_task_t)/sizeof(struct highcpu_task_handler);

#define NORMAL_PERIOD           3*TIMER_HZ      /* minisecond */
static void xtop_loop(struct timer_entry *timer, void *data);
static void xtop_config(struct timer_entry *timer, void *data);	
static void xtop_free(struct timer_entry *timer, void *data);	
static void xtop_reap(struct timer_entry *timer, void *data);	

static struct task_table xtop_task_t[] =
{       /* sig, *timer, *func, *data, expires */
        {SIGALRM, 0, xtop_loop, 0, NORMAL_PERIOD},
        {SIGUSR1, 0, xtop_config, 0, 0},
        {SIGTERM, 0, xtop_free, 0, 0},
        {SIGCHLD, 0, xtop_reap, 0, 0},
        {0, 0, 0, 0, 0}
};
static int xtop_xnum = sizeof(xtop_task_t)/sizeof(struct task_table);
static int next_expires = NORMAL_PERIOD;
static int xtop_dbg = 0;

static pid_t 
xtop_get_pid_by_name(char *name)
{
        pid_t           pid = 0;
        DIR             *dir;
        struct dirent   *next;

	if(!name)	return 0;

        if ((dir = opendir("/proc")) == NULL) {
                perror("Cannot open /proc");
                return -1;
        }

        while ((next = readdir(dir)) != NULL) {
                FILE *fp;
                char filename[256];
                char buffer[256];

                if (!isdigit(*next->d_name))
                        continue;

                sprintf(filename, "/proc/%s/cmdline", next->d_name);
                fp = fopen(filename, "r");
                if (!fp) {
                        continue;
                }
                buffer[0] = '\0';
                fgets(buffer, 256, fp);
                fclose(fp);

                if (strstr(buffer, name)) {
                        pid = strtol(next->d_name, NULL, 0);
                        break;
                }
        }
        closedir(dir);

        return pid;
}

static void 
xtop_restart_mdns() 
{
	int i;
        char *avadbg = nvram_match("ava_verb", "1")?"--debug":NULL;
        char *avahi_daemon_argv[] = {"avahi-daemon", avadbg, NULL};
        pid_t pid;

	for(i=0; i<3; ++i) {
		if(xtop_dbg)	printf("Terminate avahi-daemon..(%d)\n", i);
		if (xtop_get_pid_by_name("avahi-daemon") > 0) {
			killall("avahi-daemon", SIGTERM);
		} else
			break;
		sleep(2);
	}

	if (xtop_get_pid_by_name("avahi-daemon") > 0) {
		for(i=0; i<3; ++i) {
			if(xtop_dbg)	printf("Kill avahi-daemon..(%d)\n", i);
			if (xtop_get_pid_by_name("avahi-daemon") > 0) {
				killall("avahi-daemon", SIGKILL);
			} else
				break;
			sleep(2);
		}
	}

	if (xtop_get_pid_by_name("avahi-daemon") > 0) {
		printf("! Failed stop avahi-daemon\n");
		return;
	}

        return _eval(avahi_daemon_argv, NULL, 0, &pid);
}

static void 
stop_xbusy() 
{
	eval("killall", "xbusy");
}

static int 
read_cpu_jiffy(FILE *fp, jiffy_counts_t *p_jif)
{
	static const char fmt[] = "cpu %llu %llu %llu %llu %llu %llu %llu %llu";
	int ret;

	if (!fgets(line_buf, sizeof(line_buf), fp) || line_buf[0] != 'c' /* not "cpu" */)
		return 0;
	ret = sscanf(line_buf, fmt,
		&p_jif->usr, &p_jif->nic, &p_jif->sys, &p_jif->idle,
		&p_jif->iowait, &p_jif->irq, &p_jif->softirq,
		&p_jif->steal);
	if (ret >= 4) {
		p_jif->total = p_jif->usr + p_jif->nic + p_jif->sys + p_jif->idle
		+ p_jif->iowait + p_jif->irq + p_jif->softirq + p_jif->steal;
		/* procps 2.x does not count iowait as busy time */
		p_jif->busy = p_jif->total - p_jif->idle - p_jif->iowait;
	}
    
	pre_systicks_total = systicks_total;
	systicks_total = p_jif->total;
	if(!pre_systicks_total) pre_systicks_total = systicks_total;
	sysdiff_total = systicks_total - pre_systicks_total;

	pre_systicks_busy = systicks_busy;
	systicks_busy = p_jif->busy;
	if(!pre_systicks_busy) pre_systicks_busy = systicks_busy;
	sysdiff_busy = systicks_busy - pre_systicks_busy;

	return ret;
}   

static void 
get_jiffy_counts()
{
	FILE* fp = fopen("/proc/stat", "r");

	prev_jif = cur_jif;
	if (read_cpu_jiffy(fp, &cur_jif) < 4)
		printf("can't read '%s'", "/proc/stat");

	fclose(fp);
}

#define BUFSIZE	256
static int 
read_to_buf(const char *filename, void *buf)
{
	int fd;
	/* open_read_close() would do two reads, checking for EOF.
	* When you have 10000 /proc/$NUM/stat to read, it isn't desirable */
	ssize_t ret = -1;
	fd = open(filename, O_RDONLY);
	if (fd >= 0) {
		ret = read(fd, buf, BUFSIZE-1);
		close(fd);
	}
	((char *)buf)[ret > 0 ? ret : 0] = '\0';
	return ret;
}


static procps_status_t* 
procps_scan()
{
	char filename[sizeof("/proc/%u/stat") + sizeof(int)];
	char buf[BUFSIZE], *cp;
	int n, i, r;
	int tty;
	unsigned long vsz, rss;
	long tasknice;
	procps_status_t* sp;

	sp = malloc(sizeof(procps_status_t)); 

	for(i=0; i<MAX_PD; ++i) {
		if(xpis[i].xpd == LATE_TASK) {
			xpis[i].xpd = (r=xtop_get_pid_by_name(xpis[i].xtask))>0?r:LATE_TASK;
		}
		if(xpis[i].xpd <= 0) {
			if(xpis[i].xpd!=LATE_TASK && xpis[i].xtask) {
				free(xpis[i].xtask);
				memset(&xpis[i], 0, sizeof(xpis[i]));
			}
			continue;
		}

		memset(sp, 0, sizeof(*sp));

		sprintf(filename, "/proc/%u/stat", xpis[i].xpd);
		n = read_to_buf(filename, buf);
		if (n < 0) {
			if(xpis[i].xtask) free(xpis[i].xtask);
			memset(&xpis[i], 0, sizeof(xpis[i]));
			continue;
		}

		cp = strrchr(buf, ')');
		if(!cp) {
			if(xpis[i].xtask) free(xpis[i].xtask);
			memset(&xpis[i], 0, sizeof(xpis[i]));
			continue;
		}

		n = sscanf(cp+2,
                	"%c %u "               /* state, ppid */
                	"%u %u %d %*s "        /* pgid, sid, tty, tpgid */
                	"%*s %*s %*s %*s %*s " /* flags, min_flt, cmin_flt, maj_flt, cmaj_flt */
                	"%lu %lu "             /* utime, stime */
                	"%*s %*s %*s "         /* cutime, cstime, priority */
                	"%ld "                 /* nice */
                	"%*s %*s "             /* timeout, it_real_value */
                	"%lu "                 /* start_time */
                	"%lu "                 /* vsize */
                	"%lu "                 /* rss */
                	,
                	sp->state, &sp->ppid,
                	&sp->pgid, &sp->sid, &tty,
                	&sp->utime, &sp->stime,
                	&tasknice,
                	&sp->start_time,
                	&vsz,
                	&rss
                	);
            	if (n < 11) {
			if(xpis[i].xtask) free(xpis[i].xtask);
			memset(&xpis[i], 0, sizeof(xpis[i]));
			continue;
		}

            	if (n == 11)
			sp->last_seen_on_cpu = 0;

		xpis[i].pre_pticks = xpis[i].pticks;
		xpis[i].pticks = sp->utime + sp->stime;
		if(!xpis[i].pre_pticks) xpis[i].pre_pticks = xpis[i].pticks;

		if(sp->state[0] == 'S')
			xpis[i].xratio = (float)(xpis[i].pticks - xpis[i].pre_pticks)/(float)(sysdiff_total)*100;
		else	/* R, D whatever */
			xpis[i].xratio = (float)(xpis[i].pticks - xpis[i].pre_pticks)/(float)(sysdiff_busy)*100;

		if(xtop_dbg) {
			printf("ps chk n(%d), p(%s)(%d)(%c)(%u): \n", n, xpis[i].xtask, xpis[i].xpd, sp->state[0], sp->ppid);
			printf("ps chk pticks=%lu/%lu(%lu), sysdiff=%lu/%lu, xratio[%d]=%u\n", xpis[i].pticks, xpis[i].pre_pticks, xpis[i].pticks-xpis[i].pre_pticks, sysdiff_total, sysdiff_busy, i, xpis[i].xratio);
		}
	}

	if(sp) free(sp);
	return NULL;
}

static void
xtop_exc()
{
	int i, r;

	for(i=0; i<MAX_PD; ++i) {
		if(xpis[i].xpd <= 0)	continue;

		if(xpis[i].xratio >= xpis[i].tct) {
			if(xpis[i].exc > HCPU_CEIL) {
				if(xtop_dbg)
					printf("\nready to exec [%s](%d) since reach ceil\n", xpis[i].xtask, xpis[i].xpd);
				if(xpis[i].xfunc)
					xpis[i].xfunc();
				else
					printf("! [%s] lose exec handler\n", xpis[i].xtask);
				sleep(1);
				xpis[i].xratio = xpis[i].exc = xpis[i].pticks = xpis[i].pre_pticks = 0;
				xpis[i].xpd = (r=xtop_get_pid_by_name(xpis[i].xtask))>0?r:LATE_TASK;
			} else {
				if(xtop_dbg)
					printf("[%s] reach hcpu [%u]\n", xpis[i].xtask, xpis[i].exc);
				xpis[i].exc++;
			}
		} else
			xpis[i].exc = 0;
	}
}

static void* 
get_fptr_by_name(char *name, struct highcpu_task_handler *hh, unsigned int *tct)
{
	int i;

	for(i=0; i<hh_xnum-1; ++i) {
		if(hh[i].taskname && hh[i].func && strstr(hh[i].taskname, name)) {
			*tct = hh[i].tct;
			return hh[i].func;
		}
	}

	return NULL;
}

static void 
xtop_free(struct timer_entry *timer, void *data)
{
	int i;

	for(i=0; i<MAX_PD; ++i)
		if(xpis[i].xtask) free(xpis[i].xtask);

	printf("bye\n");
	exit(1);
}

static void 
xtop_reap(struct timer_entry *timer, void *data)
{
	chld_reap(SIGCHLD);
}

static void 
xtop_config(struct timer_entry *timer, void *data)
{
	int i, r;
	const char s[1]=" ";
	char *token, *stmp;

	for(i=0; i<MAX_PD; ++i)
		if(xpis[i].xtask) free(xpis[i].xtask);
	memset(xpis, 0, sizeof(xpis));
	systicks_total = pre_systicks_total = 
	systicks_busy = pre_systicks_busy = 0;

	tasks = nvram_safe_get("xtop_tasklist");
	next_expires = nvram_get_int("xtop_exp")?:NORMAL_PERIOD;
	xtop_dbg = nvram_match("xdbg", "1")?1:0;
	stmp = strdup(tasks);
	token = strtok(stmp, s)?:stmp;

	if(xtop_dbg) {
		printf("%s:%s/%d\n", __func__, tasks, next_expires);
	}
	i=0;
	while( token!= NULL && i<MAX_PD){
		xpis[i].xpd = (r=(xtop_get_pid_by_name(token)))>0?r:LATE_TASK;
		xpis[i].xtask = strdup(token);
		xpis[i].xfunc = get_fptr_by_name(token, xtop_hcpu_task_t, &xpis[i].tct);
		if(xtop_dbg) {
			printf("reset task[%d:%s](%d)(%u)<%p>\n", i, xpis[i].xtask, xpis[i].xpd, xpis[i].tct, xpis[i].xfunc);
		}
		i++;
		token = strtok(NULL, s);
	}

	free(stmp);
}

static void 
dump_xratio()
{
	int i;

	if(!xtop_dbg)	return;

	printf("xratio: ");
	for(i=0; i<MAX_PD; ++i) {
		if(xpis[i].xpd > 0)
			printf("[%s(%u):%u] ", xpis[i].xtask, xpis[i].xpd, xpis[i].xratio);
	}
	printf("\n\n");
}

static void 
dump_hh()
{
	int i;
	struct highcpu_task_handler *hh = xtop_hcpu_task_t;
	int hnum = sizeof(xtop_hcpu_task_t)/sizeof(struct highcpu_task_handler);

	if(!xtop_dbg)	return;

	printf("\n\n\npre-chk pidsof avahi-daemon:%d\n", xtop_get_pid_by_name("avahi-daemon"));
	printf("\npre-chk pidsof xbusy:%d\n\n\n", xtop_get_pid_by_name("xbusy"));

	printf("hcpu tasks handles:(%p)(%p)\n", xtop_restart_mdns, stop_xbusy);
	for(i=0; i<hnum; ++i) {
		printf("%s (%u)-->%p\n", hh[i].taskname, hh[i].tct, hh[i].func);
	}
}

static void
xtop_loop(struct timer_entry *timer, void *data)
{
	get_jiffy_counts();
	procps_scan();
	xtop_exc();

	dump_xratio();

	mod_timer(timer, next_expires);
}

int
xtop_main(int argc, char *argvs[])
{
        next_expires = NORMAL_PERIOD;
	xtop_config(NULL, NULL);
	dump_hh();

        tasks_run(xtop_task_t, xtop_xnum, NORMAL_PERIOD);

	return 0;
}
