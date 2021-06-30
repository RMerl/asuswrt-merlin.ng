/*
 * util_funcs.c
 */
/*
 * Portions of this file are copyrighted by:
 * Copyright Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 *
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>

#include <sys/types.h>
#if HAVE_IO_H
#include <io.h>
#endif
#ifdef HAVE_SPAWN_H
#include <spawn.h>
#endif
#include <stdio.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_MALLOC_H
#include <malloc.h>
#endif
#ifdef __alpha
#ifndef _BSD
#define _BSD
#define _myBSD
#endif
#endif
#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#ifdef __alpha
#ifdef _myBSD
#undef _BSD
#undef _myBSD
#endif
#endif
#ifndef WEXITSTATUS
# define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
# define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <errno.h>
#include <signal.h>
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <ctype.h>
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_BASETSD_H
#include <basetsd.h>
#define ssize_t SSIZE_T
#endif
#if HAVE_RAISE
#define alarm raise
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_LINUX_ETHTOOL_H
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/sockios.h>
#ifdef HAVE_LINUX_ETHTOOL_NEEDS_U64
#include <linux/types.h>
typedef __u64 u64;         /* hack, so we may include kernel's ethtool.h */
typedef __u32 u32;         /* ditto */
typedef __u16 u16;         /* ditto */
typedef __u8 u8;           /* ditto */
#endif
#include <linux/ethtool.h>
#endif /* HAVE_LINUX_ETHTOOL_H */
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/library/snmp_logging.h>
#include <net-snmp/agent/netsnmp_close_fds.h>

#include "struct.h"
#include "util_funcs.h"
#include "utilities/execute.h"

#if HAVE_LIMITS_H
#include "limits.h"
#endif
#ifdef USING_UCD_SNMP_ERRORMIB_MODULE
#include "ucd-snmp/errormib.h"
#else
#define setPerrorstatus(x) snmp_log_perror(x)
#endif

netsnmp_feature_child_of(util_funcs, libnetsnmpmibs);

netsnmp_feature_child_of(shell_command, util_funcs);
netsnmp_feature_child_of(get_exten_instance, util_funcs);
netsnmp_feature_child_of(clear_cache, util_funcs);
netsnmp_feature_child_of(find_field, util_funcs);
netsnmp_feature_child_of(parse_miboid, util_funcs);
netsnmp_feature_child_of(string_append_int, util_funcs);
netsnmp_feature_child_of(internal_mib_table, util_funcs);

#if defined(HAVE_LINUX_RTNETLINK_H)
netsnmp_feature_child_of(prefix_info_all, util_funcs);
netsnmp_feature_child_of(prefix_info, prefix_info_all);
netsnmp_feature_child_of(update_prefix_info, prefix_info_all);
netsnmp_feature_child_of(delete_prefix_info, prefix_info_all);
netsnmp_feature_child_of(find_prefix_info, prefix_info_all);
netsnmp_feature_child_of(create_prefix_info, prefix_info_all);
#endif /* HAVE_LINUX_RTNETLINK_H */

#if defined(NETSNMP_EXCACHETIME) && defined(USING_UTILITIES_EXECUTE_MODULE) && defined(HAVE_EXECV)
static long     cachetime;
#endif

/** deprecated, use netsnmp_mktemp instead */
const char *
make_tempfile(void)
{
    return netsnmp_mktemp();
}

#ifndef NETSNMP_FEATURE_REMOVE_SHELL_COMMAND
int
shell_command(struct extensible *ex)
{
#if HAVE_SYSTEM
    const char     *ofname;
    char           *shellline = NULL;
    FILE           *shellout;

    ofname = make_tempfile();
    if (ofname == NULL) {
        ex->output[0] = 0;
        ex->result = 127;
        return ex->result;
    }

    if (asprintf(&shellline, "%s > %s", ex->command, ofname) >= 0) {
        ex->result = system(shellline);
        ex->result = WEXITSTATUS(ex->result);
        free(shellline);
    }
    shellout = fopen(ofname, "r");
    if (shellout != NULL) {
        if (fgets(ex->output, sizeof(ex->output), shellout) == NULL) {
            ex->output[0] = 0;
        }
        fclose(shellout);
    }
    unlink(ofname);
#else
    ex->output[0] = 0;
    ex->result = 0;
#endif
    return (ex->result);
}
#endif /* NETSNMP_FEATURE_REMOVE_SHELL_COMMAND */

#define MAXOUTPUT 300

int
exec_command(struct extensible *ex)
{
#if defined (HAVE_EXECV) || defined (WIN32)
    int             fd;
    FILE           *file;

    if ((fd = get_exec_output(ex)) != -1) {
        file = fdopen(fd, "r");
        if (fgets(ex->output, sizeof(ex->output), file) == NULL) {
            ex->output[0] = 0;
        }
        fclose(file);
        wait_on_exec(ex);
    } else
#endif /* HAVE_EXECV */
    {
        ex->output[0] = 0;
        ex->result = 0;
    }
    return (ex->result);
}

#ifndef NETSNMP_FEATURE_REMOVE_GET_EXTEN_INSTANCE
struct extensible *
get_exten_instance(struct extensible *exten, size_t inst)
{
    int             i;

    if (exten == NULL)
        return (NULL);
    for (i = 1; i != (int) inst && exten != NULL; i++)
        exten = exten->next;
    return (exten);
}
#endif /* NETSNMP_FEATURE_REMOVE_GET_EXTEN_INSTANCE */

void
wait_on_exec(struct extensible *ex)
{
#if defined(WIN32) && !defined (mingw32)
  int rc;
  if (ex->tid != 0 && ex->pid != 0) {
    HANDLE hThread = ex->tid;
    HANDLE hProcess = ex->pid;
    rc = WaitForSingleObject(hProcess, NETSNMP_TIMEOUT_WAITFORSINGLEOBJECT);
    DEBUGMSGT(("exec:wait_on_exec","WaitForSingleObject rc=(%d)\n",rc ));
    rc = CloseHandle( hThread );
    DEBUGMSGT(("exec:wait_on_exec","CloseHandle hThread=(%d)\n",rc ));
    rc = CloseHandle( hProcess );
    DEBUGMSGT(("exec:wait_on_exec","CloseHandle hProcess=(%d)\n",rc ));
    ex->pid = 0;
    ex->tid = 0;
  }
#else
#ifndef NETSNMP_EXCACHETIME
    if (ex->pid && waitpid(ex->pid, &ex->result, 0) < 0) {
        setPerrorstatus("waitpid");
    }
    ex->pid = 0;
#endif  /* NETSNMP_EXCACHETIME */
#endif  /* WIN32 */
}

#define MAXARGS 30

int
get_exec_output(struct extensible *ex)
{
#ifndef USING_UTILITIES_EXECUTE_MODULE
    ex->result = -1;
    NETSNMP_LOGONCE((LOG_WARNING, "support for run_exec_command not available\n"));
#else
#if HAVE_EXECV
    char            cachefile[STRMAX];
    char            cache[NETSNMP_MAXCACHESIZE];
    int             cachebytes;
    int             cfd;
#ifdef NETSNMP_EXCACHETIME
    long            curtime;
    static char     lastcmd[STRMAX];
    static int      lastresult;
#endif

    DEBUGMSGTL(("exec:get_exec_output","calling %s\n", ex->command));

    sprintf(cachefile, "%s/%s", get_persistent_directory(), NETSNMP_CACHEFILE);
#ifdef NETSNMP_EXCACHETIME
    curtime = time(NULL);
    if (curtime > (cachetime + NETSNMP_EXCACHETIME) ||
        strcmp(ex->command, lastcmd) != 0) {
        strlcpy(lastcmd, ex->command, sizeof(lastcmd));
        cachetime = curtime;
#endif

        cachebytes = NETSNMP_MAXCACHESIZE;
        ex->result = run_exec_command( ex->command, NULL, cache, &cachebytes );

        unlink(cachefile);
            /*
             * XXX  Use SNMP_FILEMODE_CLOSED instead of 644? 
             */
        if ((cfd = open(cachefile, O_WRONLY | O_TRUNC | O_CREAT, 0600)) < 0) {
                snmp_log(LOG_ERR,"can not create cache file\n");
                setPerrorstatus(cachefile);
#ifdef NETSNMP_EXCACHETIME
                cachetime = 0;
#endif
                return -1;
        }
        if (cachebytes > 0)
            NETSNMP_IGNORE_RESULT(write(cfd, cache, cachebytes));
        close(cfd);
#ifdef NETSNMP_EXCACHETIME
        lastresult = ex->result;
    } else {
        ex->result = lastresult;
    }
#endif
    DEBUGMSGTL(("exec:get_exec_output","using cached value\n"));
    if ((cfd = open(cachefile, O_RDONLY)) < 0) {
        snmp_log(LOG_ERR,"can not open cache file\n");
        setPerrorstatus(cachefile);
        return -1;
    }
    return (cfd);
#elif defined(HAVE__GET_OSFHANDLE) && defined(HAVE__OPEN_OSFHANDLE)
    /* MSVC and MinGW32. Cygwin has execv() and fork(). */
    int         fd;   
    
    /* Reference:  MS tech note: 190351 */
    HANDLE hOutputReadTmp, hOutputRead, hOutputWrite = NULL;
        
    HANDLE hErrorWrite;
    SECURITY_ATTRIBUTES sa;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    
    sa.nLength= sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    DEBUGMSGTL(("exec:get_exec_output","calling %s\n", ex->command));
    
    /* Child temporary output pipe with Inheritance on (sa.bInheritHandle is true) */    
    if (!CreatePipe(&hOutputReadTmp,&hOutputWrite,&sa,0)) {
      DEBUGMSGTL(("util_funcs", "get_exec_pipes CreatePipe ChildOut: %lu\n",
            GetLastError()));
      return -1;
    }
    
    /* Copy the stdout handle to the stderr handle in case the child closes one of 
     * its stdout handles. */
    if (!DuplicateHandle(GetCurrentProcess(),hOutputWrite, GetCurrentProcess(),
          &hErrorWrite,0, TRUE,DUPLICATE_SAME_ACCESS)) {
      DEBUGMSGTL(("util_funcs", "get_exec_output DuplicateHandle: %lu\n", GetLastError()));
      return -1;
    }

    /* Create new copies of the input and output handles but set bInheritHandle to 
     * FALSE so the new handle can not be inherited.  Otherwise the handles can not
     * be closed.  */
    if (!DuplicateHandle(GetCurrentProcess(), hOutputReadTmp, GetCurrentProcess(),
          &hOutputRead, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
      DEBUGMSGTL(("util_funcs", "get_exec_output DupliateHandle ChildOut: %lu\n", GetLastError()));
      CloseHandle(hErrorWrite);
      return -1;
    }   

    /* Close the temporary output and input handles */
    if (!CloseHandle(hOutputReadTmp)) {
      DEBUGMSGTL(("util_funcs", "get_exec_output CloseHandle (hOutputReadTmp): %lu\n", GetLastError()));
      CloseHandle(hErrorWrite);
      CloseHandle(hOutputRead);
      return -1;
    }
    
    /* Associates a C run-time file descriptor with an existing operating-system file handle. */
    fd = _open_osfhandle((intptr_t) hOutputRead, 0);
    
    /* Set up STARTUPINFO for CreateProcess with the handles and have it hide the window
     * for the new process. */
    ZeroMemory(&si,sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = hOutputWrite;
    si.hStdError = hErrorWrite;
    si.wShowWindow = SW_HIDE;
   
    /* Launch the process that you want to redirect.  Example snmpd.conf pass_persist:
     * pass_persist    .1.3.6.1.4.1.2021.255  c:/perl/bin/perl c:/temp/pass_persisttest
    */
    if (!CreateProcess(NULL, ex->command, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
      DEBUGMSGTL(("util_funcs","get_exec_output CreateProcess:'%s' %lu\n",ex->command, GetLastError()));
      CloseHandle(hErrorWrite);
      CloseHandle(hOutputRead);
      return -1;
    }
    
    /* Set global child process handle */
    ex->pid = pi.hProcess;
    ex->tid = pi.hThread;

    /* Close pipe handles to make sure that no handles to the write end of the
     * output pipe are maintained in this process or else the pipe will
     * not close when the child process exits and any calls to ReadFile 
     * will hang.
     */

    if (!CloseHandle(hOutputWrite)){
      DEBUGMSGTL(("util_funcs","get_exec_output CloseHandle hOutputWrite: %lu\n",
                  GetLastError()));
      return -1;
    }
    if (!CloseHandle(hErrorWrite)) {
      DEBUGMSGTL(("util_funcs","get_exec_output CloseHandle hErrorWrite: %lu\n",
                  GetLastError()));
      return -1;
    }
    return fd;
#endif       /* HAVE_EXECV */
#endif /* !defined(USING_UTILITIES_EXECUTE_MODULE) */
    return -1;
}

/*
 * Split @cmd into words and return an array with pointers to these words.
 * Store a pointer to the split command string into *@args. The caller must
 * free both *@args and the returned pointer.
 */
static NETSNMP_ATTRIBUTE_UNUSED char **
parse_cmd(char **args, const char *cmd)
{
    int             i, cnt;
    const char     *cptr1;
    char           *cptr2, **argv, **aptr;

    *args = strdup(cmd);
    if (!*args)
        return NULL;
    for (cnt = 1, cptr1 = cmd, cptr2 = *args; *cptr1 != '\0';
         cptr2++, cptr1++) {
        *cptr2 = *cptr1;
        if (*cptr1 == ' ') {
            *cptr2++ = '\0';
            cptr1 = skip_white_const(cptr1);
            if (!cptr1)
                break;
            *cptr2 = *cptr1;
            if (*cptr1)
                cnt++;
        }
    }
    argv = malloc((cnt + 1) * sizeof(argv[0]));
    if (argv == NULL) {
        free(*args);
        return NULL;
    }
    aptr = argv;
    *aptr++ = *args;
    for (cptr2 = *args, i = 1; i != cnt; cptr2++) {
        if (*cptr2 == '\0') {
            *aptr++ = cptr2 + 1;
            i++;
        }
    }
    *aptr++ = NULL;
    return argv;
}

#if defined(HAVE_EXECV)
static int
get_exec_pipes_fork(const char *cmd, int *fdIn, int *fdOut, netsnmp_pid_t *pid)
{
    int             fd[2][2];
    char           **argv, *args;

    /*
     * Setup our pipes 
     */
    if (pipe(fd[0]) || pipe(fd[1])) {
        setPerrorstatus("pipe");
        return 0;
    }
    if ((*pid = fork()) == 0) { /* First handle for the child */
        close(fd[0][1]);
        close(fd[1][0]);
        if (dup2(fd[0][0], STDIN_FILENO) < 0) {
            setPerrorstatus("dup stdin");
            return 0;
        }
        close(fd[0][0]);
        if (dup2(fd[1][1], STDOUT_FILENO) < 0) {
            setPerrorstatus("dup stdout");
            return 0;
        }
        close(fd[1][1]);

        /*
         * write standard output and standard error to pipe. 
         */
        /*
         * close all non-standard open file descriptors 
         */
        netsnmp_close_fds(STDOUT_FILENO);
        NETSNMP_IGNORE_RESULT(dup2(STDOUT_FILENO, STDERR_FILENO));

        argv = parse_cmd(&args, cmd);
        if (!argv) {
            DEBUGMSGTL(("util_funcs", "get_exec_pipes(): argv == NULL\n"));
            return 0;
        }
        DEBUGMSGTL(("util_funcs", "get_exec_pipes(): argv[0] = %s\n", argv[0]));
        execv(argv[0], argv);
        perror(argv[0]);
        free(argv);
        free(args);
        exit(1);
    } else {
        close(fd[0][0]);
        close(fd[1][1]);
        if (*pid < 0) {
            close(fd[0][1]);
            close(fd[1][0]);
            setPerrorstatus("fork");
            return 0;
        }
        *fdIn = fd[1][0];
        *fdOut = fd[0][1];
        return (1);             /* We are returning 0 for error... */
    }
}
#endif

#if defined(HAVE_POSIX_SPAWN)
static int
NETSNMP_ATTRIBUTE_UNUSED
get_exec_pipes_spawn(const char *cmd, int *fdIn, int *fdOut, netsnmp_pid_t *pid)
{
    int             fd[2][2], spawn_res;
    char           **argv, *args;
    posix_spawnattr_t attr;
    posix_spawn_file_actions_t file_actions;

    argv = parse_cmd(&args, cmd);
    if (!argv) {
        DEBUGMSGTL(("util_funcs", "get_exec_pipes(): argv == NULL\n"));
        goto err;
    }
    if (pipe(fd[0])) {
        setPerrorstatus("pipe 0");
        goto free_argv;
    }
    if (pipe(fd[1])) {
        setPerrorstatus("pipe 1");
        goto close_pipe_0;
    }
    posix_spawnattr_init(&attr);
    posix_spawn_file_actions_init(&file_actions);
    posix_spawn_file_actions_addclose(&file_actions, fd[0][1]);
    posix_spawn_file_actions_addclose(&file_actions, fd[1][0]);
    posix_spawn_file_actions_adddup2(&file_actions,  fd[0][0], STDIN_FILENO);
    posix_spawn_file_actions_addclose(&file_actions, fd[0][0]);
    posix_spawn_file_actions_adddup2(&file_actions,  fd[1][1], STDOUT_FILENO);
    posix_spawn_file_actions_addclose(&file_actions, fd[1][1]);
    posix_spawn_file_actions_adddup2(&file_actions,  STDOUT_FILENO,
                                     STDERR_FILENO);
    spawn_res = posix_spawn(pid, argv[0], &file_actions, &attr, argv, NULL);
    posix_spawn_file_actions_destroy(&file_actions);
    posix_spawnattr_destroy(&attr);
    if (spawn_res != 0) {
        setPerrorstatus("posix_spawn");
        goto close_pipe_1;
    }
    close(fd[0][0]);
    close(fd[1][1]);
    *fdIn = fd[1][0];
    *fdOut = fd[0][1];
    return 1;

close_pipe_1:
    close(fd[1][0]);
    close(fd[1][1]);

close_pipe_0:
    close(fd[0][0]);
    close(fd[0][1]);

free_argv:
    free(argv);

err:
    return 0;
}
#endif

#if defined(HAVE__GET_OSFHANDLE) && defined(HAVE__OPEN_OSFHANDLE)
static int
get_exec_pipes_win32(const char *cmd, int *fdIn, int *fdOut, netsnmp_pid_t *pid)
{
    /* MSVC and MinGW32. Cygwin has execv() and fork(). */
    /* Reference:  MS tech note: 190351 */
    HANDLE hInputWriteTmp, hInputRead, hInputWrite = NULL;
    HANDLE hOutputReadTmp, hOutputRead, hOutputWrite = NULL;
        
    HANDLE hErrorWrite;
    SECURITY_ATTRIBUTES sa;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    
    sa.nLength= sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    /* Child temporary output pipe with Inheritance on (sa.bInheritHandle is true) */    
    if (!CreatePipe(&hOutputReadTmp,&hOutputWrite,&sa,0)) {
      DEBUGMSGTL(("util_funcs", "get_exec_pipes CreatePipe ChildOut: %d\n",
                  (unsigned int)GetLastError()));
      return 0;
    }
    /* Child temporary input pipe with Inheritance on (sa.bInheritHandle is true) */    
    if (!CreatePipe(&hInputRead,&hInputWriteTmp,&sa,0)) {
        DEBUGMSGTL(("util_funcs", "get_exec_pipes CreatePipe ChildIn: %d\n",
                    (unsigned int)GetLastError()));
      return 0;
    }
    
    /* Copy the stdout handle to the stderr handle in case the child closes one of 
     * its stdout handles. */
    if (!DuplicateHandle(GetCurrentProcess(),hOutputWrite, GetCurrentProcess(),
          &hErrorWrite,0, TRUE,DUPLICATE_SAME_ACCESS)) {
        DEBUGMSGTL(("util_funcs", "get_exec_pipes DuplicateHandle: %d\n",
                    (unsigned int)GetLastError()));
        return 0;
    }

    /* Create new copies of the input and output handles but set bInheritHandle to 
     * FALSE so the new handle can not be inherited.  Otherwise the handles can not
     * be closed.  */
    if (!DuplicateHandle(GetCurrentProcess(), hOutputReadTmp,
                         GetCurrentProcess(), &hOutputRead, 0, FALSE,
                         DUPLICATE_SAME_ACCESS)) {
        DEBUGMSGTL(("util_funcs",
                    "get_exec_pipes DupliateHandle ChildOut: %d\n",
                    (unsigned int)GetLastError()));
        CloseHandle(hErrorWrite);
        return 0;
    }   
    if (!DuplicateHandle(GetCurrentProcess(),hInputWriteTmp,
          GetCurrentProcess(), &hInputWrite, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
        DEBUGMSGTL(("util_funcs","get_exec_pipes DupliateHandle ChildIn: %d\n",
                    (unsigned int)GetLastError()));
        CloseHandle(hErrorWrite);
        CloseHandle(hOutputRead);
        return 0;
    }

    /* Close the temporary output and input handles */
    if (!CloseHandle(hOutputReadTmp)) {
        DEBUGMSGTL(("util_funcs",
                    "get_exec_pipes CloseHandle (hOutputReadTmp): %d\n",
                    (unsigned int)GetLastError()));
        CloseHandle(hErrorWrite);
        CloseHandle(hOutputRead);
        CloseHandle(hInputWrite);
        return 0;
    }
    if (!CloseHandle(hInputWriteTmp)) {
        DEBUGMSGTL(("util_funcs",
                    "get_exec_pipes CloseHandle (hInputWriteTmp): %d\n",
                    (unsigned int)GetLastError()));
        CloseHandle(hErrorWrite);
        CloseHandle(hOutputRead);
        CloseHandle(hInputWrite);
        return 0;
    }
    
    /* Associates a C run-time file descriptor with an existing operating-system file handle. */
    *fdIn = _open_osfhandle((intptr_t) hOutputRead, 0);
    *fdOut = _open_osfhandle((intptr_t) hInputWrite, 0);
    
    /* Set up STARTUPINFO for CreateProcess with the handles and have it hide the window
     * for the new process. */
    ZeroMemory(&si,sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = hOutputWrite;
    si.hStdInput = hInputRead;
    si.hStdError = hErrorWrite;
    si.wShowWindow = SW_HIDE;
   
    /* Launch the process that you want to redirect.  Example snmpd.conf pass_persist:
     * pass_persist    .1.3.6.1.4.1.2021.255  c:/perl/bin/perl c:/temp/pass_persisttest
    */
    if (!CreateProcess(NULL, NETSNMP_REMOVE_CONST(char *, cmd), NULL, NULL,
		       TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
        DEBUGMSGTL(("util_funcs","get_exec_pipes CreateProcess:'%s' %d\n", cmd,
                    (unsigned int)GetLastError()));
        CloseHandle(hErrorWrite);
        CloseHandle(hOutputRead);
        CloseHandle(hInputWrite);
        return 0;
    }
    
    DEBUGMSGTL(("util_funcs","child hProcess (stored in pid): %p\n", pi.hProcess));
    DEBUGMSGTL(("util_funcs","child dwProcessId (task manager): %d\n",(int)pi.dwProcessId));

    /* Set global child process handle */
    *pid = pi.hProcess;

    /* Cleanup */
    if (!CloseHandle(pi.hThread))
      DEBUGMSGTL(("util_funcs",
		  "get_exec_pipes(%s) CloseHandle pi.hThread: %d\n",
		  cmd, (unsigned int)GetLastError()));

   /* Close pipe handles to make sure that no handles to the write end of the
     * output pipe are maintained in this process or else the pipe will
     * not close when the child process exits and any calls to ReadFile 
     * will hang.
     */

    if (!CloseHandle(hOutputWrite)){
      DEBUGMSGTL(("util_funcs",
		  "get_exec_pipes(%s) CloseHandle hOutputWrite: %d\n",
                  cmd, (unsigned int)GetLastError()));
      return 0;
    }
    if (!CloseHandle(hInputRead)) {
      DEBUGMSGTL(("util_funcs",
		  "get_exec_pipes(%s) CloseHandle hInputRead: %d\n",
                  cmd, (unsigned int)GetLastError()));
      return 0;
    }
    if (!CloseHandle(hErrorWrite)) {
      DEBUGMSGTL(("util_funcs",
		  "get_exec_pipes(%s) CloseHandle hErrorWrite: %d\n",
                  cmd, (unsigned int)GetLastError()));
      return 0;
    }
    return 1;
}
#endif                          /* WIN32 */

int
get_exec_pipes(const char *cmd, int *fdIn, int *fdOut, netsnmp_pid_t *pid)
{
#if defined(HAVE_EXECV)
    return get_exec_pipes_fork(cmd, fdIn, fdOut, pid);
#elif defined(HAVE_POSIX_SPAWN)
    return get_exec_pipes_fork(cmd, fdIn, fdOut, pid);
#elif defined(HAVE__GET_OSFHANDLE) && defined(HAVE__OPEN_OSFHANDLE)
    return get_exec_pipes_win32(cmd, fdIn, fdOut, pid);
#endif
    DEBUGMSGTL(("util_funcs",
                "get_exec_pipes() has not yet been implemented for this platform"));
    return 0;
}

#ifndef NETSNMP_FEATURE_REMOVE_CLEAR_CACHE
int
clear_cache(int action,
            u_char * var_val,
            u_char var_val_type,
            size_t var_val_len,
            u_char * statP, oid * name, size_t name_len)
{
    if (var_val_type != ASN_INTEGER) {
        snmp_log(LOG_NOTICE, "Wrong type != int\n");
        return SNMP_ERR_WRONGTYPE;
    }
#if defined(NETSNMP_EXCACHETIME) && defined(USING_UTILITIES_EXECUTE_MODULE) && defined(HAVE_EXECV)
    else {
        long            tmp = 0;
        tmp = *((long *) var_val);
        if (tmp == 1 && action == COMMIT) {
            cachetime = 0;          /* reset the cache next read */
        }
    }
#endif
    return SNMP_ERR_NOERROR;
}
#endif /* NETSNMP_FEATURE_REMOVE_CLEAR_CACHE */

void
print_mib_oid(oid name[], size_t len)
{
    char           *buffer;
    buffer = (char *) malloc(11 * len); /* maximum digit lengths for int32 + a '.' */
    if (!buffer) {
        snmp_log(LOG_ERR, "Malloc failed - out of memory?");
        return;
    }
    sprint_mib_oid(buffer, name, len);
    snmp_log(LOG_NOTICE, "Mib: %s\n", buffer);
    free(buffer);
}

void
sprint_mib_oid(char *buf, const oid *name, size_t len)
{
    int             i;

    for (i = 0; i < (int) len; i++)
        buf += sprintf(buf, ".%" NETSNMP_PRIo "u", name[i]);
}

/*
 * checkmib(): provided for backwards compatibility, do not use: 
 */
int
checkmib(struct variable *vp, oid * name, size_t * length,
         int exact, size_t * var_len, WriteMethod ** write_method, int max)
{
    /*
     * checkmib used to be header_simple_table, with reveresed boolean
     * return output.  header_simple_table() was created to match
     * header_generic(). 
     */
    return (!header_simple_table(vp, name, length, exact, var_len,
                                 write_method, max));
}

#ifndef NETSNMP_FEATURE_REMOVE_FIND_FIELD
char           *
find_field(char *ptr, int field)
{
    int             i;
    char           *init = ptr;

    if (field == NETSNMP_LASTFIELD) {
        /*
         * skip to end 
         */
        while (*ptr++);
        ptr = ptr - 2;
        /*
         * rewind a field length 
         */
        while (*ptr != 0 && isspace((unsigned char)(*ptr)) && init <= ptr)
            ptr--;
        while (*ptr != 0 && !isspace((unsigned char)(*ptr)) && init <= ptr)
            ptr--;
        if (isspace((unsigned char)(*ptr)))
            ptr++;              /* past space */
        if (ptr < init)
            ptr = init;
        if (!isspace((unsigned char)(*ptr)) && *ptr != 0)
            return (ptr);
    } else {
        if ((ptr = skip_white(ptr)) == NULL)
            return (NULL);
        for (i = 1; *ptr != 0 && i != field; i++) {
            if ((ptr = skip_not_white(ptr)) == NULL)
                return (NULL);
            if ((ptr = skip_white(ptr)) == NULL)
                return (NULL);
        }
        if (*ptr != 0 && i == field)
            return (ptr);
        return (NULL);
    }
    return (NULL);
}
#endif /* NETSNMP_FEATURE_REMOVE_FIND_FIELD */

#ifndef NETSNMP_FEATURE_REMOVE_PARSE_MIBOID
int
parse_miboid(const char *buf, oid * oidout)
{
    int             i;

    if (!buf)
        return 0;
    if (*buf == '.')
        buf++;
    for (i = 0; isdigit((unsigned char)(*buf)); i++) {
        /* Subidentifiers are unsigned values, up to 2^32-1
         * so we need to use 'strtoul' rather than 'atoi'
         */
        oidout[i] = strtoul(buf, NULL, 10) & 0xffffffff;
        while (isdigit((unsigned char)(*buf))) buf++;
        if (*buf == '.')
            buf++;
    }
    /*
     * oidout[i] = -1; hmmm 
     */
    return i;
}
#endif /* NETSNMP_FEATURE_REMOVE_PARSE_MIBOID */

#ifndef NETSNMP_FEATURE_REMOVE_STRING_APPEND_INT
void
string_append_int(char *s, int val)
{
    char            textVal[16];

    if (val < 10) {
        *s++ = '0' + val;
        *s = '\0';
        return;
    }
    sprintf(textVal, "%d", val);
    strcpy(s, textVal);
    return;
}
#endif /* NETSNMP_FEATURE_REMOVE_STRING_APPEND_INT */

#ifndef NETSNMP_FEATURE_REMOVE_INTERNAL_MIB_TABLE

struct internal_mib_table {
    int             max_size;   /* Size of the current data table */
    int             next_index; /* Index of the next free entry */
    int             current_index;      /* Index of the 'current' entry */
    int             cache_timeout;
    marker_t        cache_markerM;
    RELOAD         *reload;     /* Routine to read in the data */
    COMPARE        *compare;    /* Routine to compare two entries */
    int             data_size;  /* Size of an individual entry */
    void           *data;       /* The table itself */
};

mib_table_t
Initialise_Table(int size, int timeout, RELOAD *reload, COMPARE *compare)
{
    struct internal_mib_table *t;

    t = (struct internal_mib_table *)
        malloc(sizeof(struct internal_mib_table));
    if (t == NULL)
        return NULL;

    t->max_size = 0;
    t->next_index = 1;          /* Don't use index 0 */
    t->current_index = 1;
    t->cache_timeout = timeout;
    t->cache_markerM = NULL;
    t->reload = reload;
    t->compare = compare;
    t->data_size = size;
    t->data = NULL;

    return (mib_table_t) t;
}

#define TABLE_ADD( x, y )	((void*)((char*)(x) + y))
#define TABLE_INDEX(t, i)	(TABLE_ADD(t->data, i * t->data_size))
#define TABLE_START(t)		(TABLE_INDEX(t, 1))
#define TABLE_NEXT(t)		(TABLE_INDEX(t, t->next_index))
#define TABLE_CURRENT(t)	(TABLE_INDEX(t, t->current_index))

int
check_and_reload_table(struct internal_mib_table *table)
{
    /*
     * If the saved data is fairly recent,
     *    we don't need to reload it
     */
    if (table->cache_markerM &&
        !(netsnmp_ready_monotonic(table->cache_markerM,
                                  table->cache_timeout * 1000)))
        return 1;


    /*
     * Call the routine provided to read in the data
     *
     * N.B:  Update the cache marker *before* calling
     *   this routine, to avoid problems with recursion
     */
    netsnmp_set_monotonic_marker(&table->cache_markerM);

    table->next_index = 1;
    if (table->reload((mib_table_t) table) < 0) {
        free(table->cache_markerM);
        table->cache_markerM = NULL;
        return 0;
    }
    table->current_index = 1;
    if (table->compare != NULL) /* Sort the table */
        qsort(TABLE_START(table), table->next_index-1,
              table->data_size, table->compare);
    return 1;
}

int
Search_Table(mib_table_t t, void *entry, int exact)
{
    struct internal_mib_table *table = (struct internal_mib_table *) t;
    void           *entry2;
    int             res;

    if (!check_and_reload_table(table))
        return -1;

    if (table->compare == NULL) {
        /*
         * XXX - not sure this is right ? 
         */
        memcpy(entry, table->data, table->data_size);
        return 0;
    }

    if (table->next_index == table->current_index)
        table->current_index = 1;

    entry2 = TABLE_CURRENT(table);
    res = table->compare(entry, entry2);
    if ((res < 0) && (table->current_index != 1)) {
        table->current_index = 1;
        entry2 = TABLE_CURRENT(table);
        res = table->compare(entry, entry2);
    }

    while (res > 0) {
        table->current_index++;
        if (table->next_index == table->current_index)
            return -1;
        entry2 = TABLE_CURRENT(table);
        res = table->compare(entry, entry2);
    }

    if (exact && res != 0)
        return -1;

    if (!exact && res == 0) {
        table->current_index++;
        if (table->next_index == table->current_index)
            return -1;
        entry2 = TABLE_CURRENT(table);
    }
    memcpy(entry, entry2, table->data_size);
    return 0;
}

int
Add_Entry(mib_table_t t, void *entry)
{
    struct internal_mib_table *table = (struct internal_mib_table *) t;
    int             new_max;
    void           *new_data;   /* Used for
                                 *      a) extending the data table
                                 *      b) the next entry to use
                                 */

    if (table->max_size <= table->next_index) {
        /*
         * Table is full, so extend it to double the size
         */
        new_max = 2 * table->max_size;
        if (new_max == 0)
            new_max = 10;       /* Start with 10 entries */

        new_data = (void *) malloc(new_max * table->data_size);
        if (new_data == NULL)
            return -1;

        if (table->data) {
            memcpy(new_data, table->data,
                   table->max_size * table->data_size);
            free(table->data);
        }
        table->data = new_data;
        table->max_size = new_max;
    }

    /*
     * Insert the new entry into the data array
     */
    new_data = TABLE_NEXT(table);
    memcpy(new_data, entry, table->data_size);
    table->next_index++;
    return 0;
}

void           *
Retrieve_Table_Data(mib_table_t t, int *max_idx)
{
    struct internal_mib_table *table = (struct internal_mib_table *) t;

    if (!check_and_reload_table(table))
        return NULL;
    *max_idx = table->next_index - 1;
    return table->data;
}
#endif /* NETSNMP_FEATURE_REMOVE_INTERNAL_MIB_TABLE */

#if defined(HAVE_LINUX_RTNETLINK_H)

#ifndef NETSNMP_FEATURE_REMOVE_CREATE_PREFIX_INFO
prefix_cbx *net_snmp_create_prefix_info(unsigned long OnLinkFlag,
                                        unsigned long AutonomousFlag,
                                        char *in6ptr)
{
   prefix_cbx *node = SNMP_MALLOC_TYPEDEF(prefix_cbx);
   if(!in6ptr) {
      free(node);
      return NULL;
   }
   if(!node) {
      free(node);
      return NULL;
   }
   node->next_info = NULL;
   node->ipAddressPrefixOnLinkFlag = OnLinkFlag;
   node->ipAddressPrefixAutonomousFlag = AutonomousFlag;
   memcpy(node->in6p, in6ptr, sizeof(node->in6p));

   return node;
}
#endif /* NETSNMP_FEATURE_REMOVE_CREATE_PREFIX_INFO */

#ifndef NETSNMP_FEATURE_REMOVE_FIND_PREFIX_INFO
int net_snmp_find_prefix_info(prefix_cbx **head,
                              char *address,
                              prefix_cbx *node_to_find)
{
    int iret;
    memset(node_to_find, 0, sizeof(prefix_cbx));
    if(!*head)
       return -1;
    memcpy(node_to_find->in6p, address, sizeof(node_to_find->in6p));

    iret = net_snmp_search_update_prefix_info(head, node_to_find, 1);
    if(iret < 0) {
       DEBUGMSGTL(("util_funcs:prefix", "Unable to search the list\n"));
       return -1;
    } else if (!iret) {
       DEBUGMSGTL(("util_funcs:prefix", "Could not find prefix info\n"));
       return -1;
    } else
       return 0;
}
#endif /* NETSNMP_FEATURE_REMOVE_FIND_PREFIX_INFO */

#ifndef NETSNMP_FEATURE_REMOVE_UPDATE_PREFIX_INFO
int net_snmp_update_prefix_info(prefix_cbx **head,
                                prefix_cbx *node_to_update)
{
    int iret;
    iret = net_snmp_search_update_prefix_info(head, node_to_update, 0);
    if(iret < 0) {
       DEBUGMSGTL(("util_funcs:prefix", "Unable to update prefix info\n"));
       return -1;
    } else if (!iret) {
       DEBUGMSGTL(("util_funcs:prefix", "Unable to find the node to update\n"));
       return -1;
    } else
       return 0;
}
#endif /* NETSNMP_FEATURE_REMOVE_UPDATE_PREFIX_INFO */

int net_snmp_search_update_prefix_info(prefix_cbx **head,
                                       prefix_cbx *node_to_use,
                                       int functionality)
{

   /* We define functionality based on need                                                         *
    * 0 - Need to do a search and update. We have to provide the node_to_use structure filled fully *
    * 1 - Need to do only search. Provide the node_to_use with in6p value filled                    */

    prefix_cbx *temp_node;
    netsnmp_assert(NULL != head);
    netsnmp_assert(NULL != node_to_use);

    if(functionality > 1)
       return -1;
    if(!node_to_use)
       return -1;


    if (!functionality) {
       if (!*head) {
           *head = node_to_use;
           return 1;
       }

       for (temp_node = *head; temp_node->next_info != NULL ; temp_node = temp_node->next_info) {
            if (0 == strcmp(temp_node->in6p, node_to_use->in6p)) {
                temp_node->ipAddressPrefixOnLinkFlag = node_to_use->ipAddressPrefixOnLinkFlag;
                temp_node->ipAddressPrefixAutonomousFlag = node_to_use->ipAddressPrefixAutonomousFlag;
                return 2;
            }
       }
       temp_node->next_info = node_to_use;
       return 1;
    } else {
         for (temp_node = *head; temp_node != NULL ; temp_node = temp_node->next_info) {
              if (0 == strcmp(temp_node->in6p, node_to_use->in6p)) {
                /*need yo put sem here as i read here */
                node_to_use->ipAddressPrefixOnLinkFlag = temp_node->ipAddressPrefixOnLinkFlag;
                node_to_use->ipAddressPrefixAutonomousFlag = temp_node->ipAddressPrefixAutonomousFlag;
                return 1;
              }
         }
         return 0;
    }
}

#ifndef NETSNMP_FEATURE_REMOVE_DELETE_PREFIX_INFO
int net_snmp_delete_prefix_info(prefix_cbx **head,
                                char *address)
{

    prefix_cbx *temp_node,*prev_node;
    if(!address)
       return -1;
    if(!head)
       return -1;

    for (temp_node = *head, prev_node = NULL; temp_node;
         prev_node = temp_node, temp_node = temp_node->next_info) {

         if (strcmp(temp_node->in6p, address) == 0) {
            if (prev_node)
                prev_node->next_info = temp_node->next_info;
            else
                *head = temp_node->next_info;
            free(temp_node);
            return 1;
        }

    }
    return 0;
}
#endif /* NETSNMP_FEATURE_REMOVE_DELETE_PREFIX_INFO */

#endif /* HAVE_LINUX_RTNETLINK_H */
         
/**
 * netsnmp_get_link_settings
 * @name
 *
 * @returns  0 success
 * @returns -1 Both ETHTOOL_GLINKSETTINGS and ETHTOOL_GSET failed
 * @returns -2 HAVE_LINUX_ETHTOOL_H is not defined
 */
int netsnmp_get_link_settings(struct netsnmp_linux_link_settings *nlls,
                              int fd, const char *name)
{
    int err = -2;

#ifdef HAVE_LINUX_ETHTOOL_H
    struct ifreq ifr;

    err = -1;
    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

#ifdef ETHTOOL_GLINKSETTINGS
    {
        /*
         * For Linux kernel v5.2 __ETHTOOL_LINK_MODE_MASK_NBITS == 67 or
         * 3 32-bit words. Increase the 'nwords' constant if necessary.
         */
        enum { mask_nwords = 4 };
        struct {
            struct ethtool_link_settings elinkset;
            uint32_t masks[3 * mask_nwords];
        } data;

        memset(&data, 0, sizeof(data));
        data.elinkset.cmd = ETHTOOL_GLINKSETTINGS;
        ifr.ifr_data = &data.elinkset;
        err = ioctl(fd, SIOCETHTOOL, &ifr, name);
        /*
         * See also the struct ethtool_link_settings documentation in
         * Linux kernel header file include/uapi/linux/ethtool.h.
         */
        if (data.elinkset.link_mode_masks_nwords < 0 &&
            -data.elinkset.link_mode_masks_nwords <= mask_nwords) {
            data.elinkset.link_mode_masks_nwords =
                -data.elinkset.link_mode_masks_nwords;
            err = ioctl(fd, SIOCETHTOOL, &ifr);
        }
        if (err >= 0) {
            nlls->duplex = data.elinkset.duplex;
            nlls->speed = data.elinkset.speed;
        }
    }
#endif /* ETHTOOL_GLINKSETTINGS */

    if (err < 0) {
        struct ethtool_cmd edata;

        memset(&edata, 0, sizeof(edata));
        edata.cmd = ETHTOOL_GSET;
        ifr.ifr_data = (char *)&edata;
        err = ioctl(fd, SIOCETHTOOL, &ifr, name);
        if (err >= 0) {
            nlls->duplex = edata.duplex;
            nlls->speed = edata.speed;
#ifdef HAVE_STRUCT_ETHTOOL_CMD_SPEED_HI
            nlls->speed |= edata.speed_hi << 16;
#endif
        } else {
            err = -1;
        }
    }
#endif /* HAVE_LINUX_ETHTOOL_H */

    return err;
}
