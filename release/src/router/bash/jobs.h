/* jobs.h -- structures and definitions used by the jobs.c file. */

/* Copyright (C) 1993-2015 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#if !defined (_JOBS_H_)
#  define _JOBS_H_

#include "quit.h"
#include "siglist.h"

#include "stdc.h"

#include "posixwait.h"

/* Defines controlling the fashion in which jobs are listed. */
#define JLIST_STANDARD       0
#define JLIST_LONG	     1
#define JLIST_PID_ONLY	     2
#define JLIST_CHANGED_ONLY   3
#define JLIST_NONINTERACTIVE 4

/* I looked it up.  For pretty_print_job ().  The real answer is 24. */
#define LONGEST_SIGNAL_DESC 24

/* The max time to sleep while retrying fork() on EAGAIN failure */
#define FORKSLEEP_MAX	16

/* We keep an array of jobs.  Each entry in the array is a linked list
   of processes that are piped together.  The first process encountered is
   the group leader. */

/* Values for the `running' field of a struct process. */
#define PS_DONE		0
#define PS_RUNNING	1
#define PS_STOPPED	2
#define PS_RECYCLED	4

/* Each child of the shell is remembered in a STRUCT PROCESS.  A circular
   chain of such structures is a pipeline. */
typedef struct process {
  struct process *next;	/* Next process in the pipeline.  A circular chain. */
  pid_t pid;		/* Process ID. */
  WAIT status;		/* The status of this command as returned by wait. */
  int running;		/* Non-zero if this process is running. */
  char *command;	/* The particular program that is running. */
} PROCESS;

struct pipeline_saver {
  struct process *pipeline;
  struct pipeline_saver *next;
};

/* PALIVE really means `not exited' */
#define PSTOPPED(p)	(WIFSTOPPED((p)->status))
#define PRUNNING(p)	((p)->running == PS_RUNNING)
#define PALIVE(p)	(PRUNNING(p) || PSTOPPED(p))

#define PEXITED(p)	((p)->running == PS_DONE)
#if defined (RECYCLES_PIDS)
#  define PRECYCLED(p)	((p)->running == PS_RECYCLED)
#else
#  define PRECYCLED(p)	(0)
#endif
#define PDEADPROC(p)	(PEXITED(p) || PRECYCLED(p))

#define get_job_by_jid(ind)	(jobs[(ind)])

/* A description of a pipeline's state. */
typedef enum { JNONE = -1, JRUNNING = 1, JSTOPPED = 2, JDEAD = 4, JMIXED = 8 } JOB_STATE;
#define JOBSTATE(job)	(jobs[(job)]->state)
#define J_JOBSTATE(j)	((j)->state)

#define STOPPED(j)	(jobs[(j)]->state == JSTOPPED)
#define RUNNING(j)	(jobs[(j)]->state == JRUNNING)
#define DEADJOB(j)	(jobs[(j)]->state == JDEAD)

#define INVALID_JOB(j)	((j) < 0 || (j) >= js.j_jobslots || get_job_by_jid(j) == 0)

/* Values for the FLAGS field in the JOB struct below. */
#define J_FOREGROUND 0x01 /* Non-zero if this is running in the foreground.  */
#define J_NOTIFIED   0x02 /* Non-zero if already notified about job state.   */
#define J_JOBCONTROL 0x04 /* Non-zero if this job started under job control. */
#define J_NOHUP      0x08 /* Don't send SIGHUP to job if shell gets SIGHUP. */
#define J_STATSAVED  0x10 /* A process in this job had had status saved via $! */
#define J_ASYNC	     0x20 /* Job was started asynchronously */

#define IS_FOREGROUND(j)	((jobs[j]->flags & J_FOREGROUND) != 0)
#define IS_NOTIFIED(j)		((jobs[j]->flags & J_NOTIFIED) != 0)
#define IS_JOBCONTROL(j)	((jobs[j]->flags & J_JOBCONTROL) != 0)
#define IS_ASYNC(j)		((jobs[j]->flags & J_ASYNC) != 0)

typedef struct job {
  char *wd;	   /* The working directory at time of invocation. */
  PROCESS *pipe;   /* The pipeline of processes that make up this job. */
  pid_t pgrp;	   /* The process ID of the process group (necessary). */
  JOB_STATE state; /* The state that this job is in. */
  int flags;	   /* Flags word: J_NOTIFIED, J_FOREGROUND, or J_JOBCONTROL. */
#if defined (JOB_CONTROL)
  COMMAND *deferred;	/* Commands that will execute when this job is done. */
  sh_vptrfunc_t *j_cleanup; /* Cleanup function to call when job marked JDEAD */
  PTR_T cleanarg;	/* Argument passed to (*j_cleanup)() */
#endif /* JOB_CONTROL */
} JOB;

struct jobstats {
  /* limits */
  long c_childmax;
  /* child process statistics */
  int c_living;		/* running or stopped child processes */
  int c_reaped;		/* exited child processes still in jobs list */
  int c_injobs;		/* total number of child processes in jobs list */
  /* child process totals */
  int c_totforked;	/* total number of children this shell has forked */
  int c_totreaped;	/* total number of children this shell has reaped */
  /* job counters and indices */
  int j_jobslots;	/* total size of jobs array */
  int j_lastj;		/* last (newest) job allocated */
  int j_firstj;		/* first (oldest) job allocated */
  int j_njobs;		/* number of non-NULL jobs in jobs array */
  int j_ndead;		/* number of JDEAD jobs in jobs array */
  /* */
  int j_current;	/* current job */
  int j_previous;	/* previous job */
  /* */
  JOB *j_lastmade;	/* last job allocated by stop_pipeline */
  JOB *j_lastasync;	/* last async job allocated by stop_pipeline */
};

/* Revised to accommodate new hash table bgpids implementation. */
typedef pid_t ps_index_t;

struct pidstat {
  ps_index_t bucket_next;
  ps_index_t bucket_prev;

  pid_t pid;
  bits16_t status;		/* only 8 bits really needed */
};

struct bgpids {
  struct pidstat *storage;	/* storage arena */

  ps_index_t head;
  ps_index_t nalloc;

  int npid;
};

#define NO_PIDSTAT (ps_index_t)-1

#define NO_JOB  -1	/* An impossible job array index. */
#define DUP_JOB -2	/* A possible return value for get_job_spec (). */
#define BAD_JOBSPEC -3	/* Bad syntax for job spec. */

/* A value which cannot be a process ID. */
#define NO_PID (pid_t)-1

#define ANY_PID (pid_t)-1

/* System calls. */
#if !defined (HAVE_UNISTD_H)
extern pid_t fork (), getpid (), getpgrp ();
#endif /* !HAVE_UNISTD_H */

/* Stuff from the jobs.c file. */
extern struct jobstats js;

extern pid_t original_pgrp, shell_pgrp, pipeline_pgrp;
extern volatile pid_t last_made_pid, last_asynchronous_pid;
extern int asynchronous_notification;

extern JOB **jobs;

extern void making_children __P((void));
extern void stop_making_children __P((void));
extern void cleanup_the_pipeline __P((void));
extern void discard_last_procsub_child __P((void));
extern void save_pipeline __P((int));
extern PROCESS *restore_pipeline __P((int));
extern void start_pipeline __P((void));
extern int stop_pipeline __P((int, COMMAND *));
extern int discard_pipeline __P((PROCESS *));
extern void append_process __P((char *, pid_t, int, int));

extern void delete_job __P((int, int));
extern void nohup_job __P((int));
extern void delete_all_jobs __P((int));
extern void nohup_all_jobs __P((int));

extern int count_all_jobs __P((void));

extern void terminate_current_pipeline __P((void));
extern void terminate_stopped_jobs __P((void));
extern void hangup_all_jobs __P((void));
extern void kill_current_pipeline __P((void));

#if defined (__STDC__) && defined (pid_t)
extern int get_job_by_pid __P((int, int));
extern void describe_pid __P((int));
#else
extern int get_job_by_pid __P((pid_t, int));
extern void describe_pid __P((pid_t));
#endif

extern void list_one_job __P((JOB *, int, int, int));
extern void list_all_jobs __P((int));
extern void list_stopped_jobs __P((int));
extern void list_running_jobs __P((int));

extern pid_t make_child __P((char *, int));

extern int get_tty_state __P((void));
extern int set_tty_state __P((void));

extern int job_exit_status __P((int));
extern int job_exit_signal __P((int));

extern int wait_for_single_pid __P((pid_t, int));
extern void wait_for_background_pids __P((void));
extern int wait_for __P((pid_t));
extern int wait_for_job __P((int));
extern int wait_for_any_job __P((void));

extern void wait_sigint_cleanup __P((void));

extern void notify_and_cleanup __P((void));
extern void reap_dead_jobs __P((void));
extern int start_job __P((int, int));
extern int kill_pid __P((pid_t, int, int));
extern int initialize_job_control __P((int));
extern void initialize_job_signals __P((void));
extern int give_terminal_to __P((pid_t, int));

extern void run_sigchld_trap __P((int));

extern int freeze_jobs_list __P((void));
extern void unfreeze_jobs_list __P((void));
extern int set_job_control __P((int));
extern void without_job_control __P((void));
extern void end_job_control __P((void));
extern void restart_job_control __P((void));
extern void set_sigchld_handler __P((void));
extern void ignore_tty_job_signals __P((void));
extern void default_tty_job_signals __P((void));
extern void get_original_tty_job_signals __P((void));

extern void init_job_stats __P((void));

extern void close_pgrp_pipe __P((void));
extern void save_pgrp_pipe __P((int *, int));
extern void restore_pgrp_pipe __P((int *));

extern void set_maxchild __P((int));

extern int job_control;		/* set to 0 in nojobs.c */

#endif /* _JOBS_H_ */
