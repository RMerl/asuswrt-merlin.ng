/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * SPDX-License-Identifier: curl
 *
 ***************************************************************************/
#include "tool_setup.h"

#include <sys/stat.h>

#ifdef WIN32
#include <tchar.h>
#endif

#include <signal.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#define ENABLE_CURLX_PRINTF
/* use our own printf() functions */
#include "curlx.h"

#include "tool_cfgable.h"
#include "tool_doswin.h"
#include "tool_msgs.h"
#include "tool_operate.h"
#include "tool_vms.h"
#include "tool_main.h"
#include "tool_libinfo.h"
#include "tool_stderr.h"

/*
 * This is low-level hard-hacking memory leak tracking and similar. Using
 * the library level code from this client-side is ugly, but we do this
 * anyway for convenience.
 */
#include "memdebug.h" /* keep this as LAST include */

#ifdef __VMS
/*
 * vms_show is a global variable, used in main() as parameter for
 * function vms_special_exit() to allow proper curl tool exiting.
 * Its value may be set in other tool_*.c source files thanks to
 * forward declaration present in tool_vms.h
 */
int vms_show = 0;
#endif

#ifdef __MINGW32__
/*
 * There seems to be no way to escape "*" in command-line arguments with MinGW
 * when command-line argument globbing is enabled under the MSYS shell, so turn
 * it off.
 */
extern int _CRT_glob;
int _CRT_glob = 0;
#endif /* __MINGW32__ */

/* if we build a static library for unit tests, there is no main() function */
#ifndef UNITTESTS

#if defined(HAVE_PIPE) && defined(HAVE_FCNTL)
/*
 * Ensure that file descriptors 0, 1 and 2 (stdin, stdout, stderr) are
 * open before starting to run.  Otherwise, the first three network
 * sockets opened by curl could be used for input sources, downloaded data
 * or error logs as they will effectively be stdin, stdout and/or stderr.
 *
 * fcntl's F_GETFD instruction returns -1 if the file descriptor is closed,
 * otherwise it returns "the file descriptor flags (which typically can only
 * be FD_CLOEXEC, which is not set here).
 */
static int main_checkfds(void)
{
  int fd[2];
  while((fcntl(STDIN_FILENO, F_GETFD) == -1) ||
        (fcntl(STDOUT_FILENO, F_GETFD) == -1) ||
        (fcntl(STDERR_FILENO, F_GETFD) == -1))
    if(pipe(fd))
      return 1;
  return 0;
}
#else
#define main_checkfds() 0
#endif

#ifdef CURLDEBUG
static void memory_tracking_init(void)
{
  char *env;
  /* if CURL_MEMDEBUG is set, this starts memory tracking message logging */
  env = curlx_getenv("CURL_MEMDEBUG");
  if(env) {
    /* use the value as file name */
    char fname[CURL_MT_LOGFNAME_BUFSIZE];
    if(strlen(env) >= CURL_MT_LOGFNAME_BUFSIZE)
      env[CURL_MT_LOGFNAME_BUFSIZE-1] = '\0';
    strcpy(fname, env);
    curl_free(env);
    curl_dbg_memdebug(fname);
    /* this weird stuff here is to make curl_free() get called before
       curl_dbg_memdebug() as otherwise memory tracking will log a free()
       without an alloc! */
  }
  /* if CURL_MEMLIMIT is set, this enables fail-on-alloc-number-N feature */
  env = curlx_getenv("CURL_MEMLIMIT");
  if(env) {
    char *endptr;
    long num = strtol(env, &endptr, 10);
    if((endptr != env) && (endptr == env + strlen(env)) && (num > 0))
      curl_dbg_memlimit(num);
    curl_free(env);
  }
}
#else
#  define memory_tracking_init() Curl_nop_stmt
#endif

/*
 * This is the main global constructor for the app. Call this before
 * _any_ libcurl usage. If this fails, *NO* libcurl functions may be
 * used, or havoc may be the result.
 */
static CURLcode main_init(struct GlobalConfig *config)
{
  CURLcode result = CURLE_OK;

#if defined(__DJGPP__) || defined(__GO32__)
  /* stop stat() wasting time */
  _djstat_flags |= _STAT_INODE | _STAT_EXEC_MAGIC | _STAT_DIRSIZE;
#endif

  /* Initialise the global config */
  config->showerror = FALSE;          /* show errors when silent */
  config->styled_output = TRUE;       /* enable detection */
  config->parallel_max = PARALLEL_DEFAULT;

  /* Allocate the initial operate config */
  config->first = config->last = malloc(sizeof(struct OperationConfig));
  if(config->first) {
    /* Perform the libcurl initialization */
    result = curl_global_init(CURL_GLOBAL_DEFAULT);
    if(!result) {
      /* Get information about libcurl */
      result = get_libcurl_info();

      if(!result) {
        /* Initialise the config */
        config_init(config->first);
        config->first->global = config;
      }
      else {
        errorf(config, "error retrieving curl library information");
        free(config->first);
      }
    }
    else {
      errorf(config, "error initializing curl library");
      free(config->first);
    }
  }
  else {
    errorf(config, "error initializing curl");
    result = CURLE_FAILED_INIT;
  }

  return result;
}

static void free_globalconfig(struct GlobalConfig *config)
{
  Curl_safefree(config->trace_dump);

  if(config->trace_fopened && config->trace_stream)
    fclose(config->trace_stream);
  config->trace_stream = NULL;

  Curl_safefree(config->libcurl);
}

/*
 * This is the main global destructor for the app. Call this after
 * _all_ libcurl usage is done.
 */
static void main_free(struct GlobalConfig *config)
{
  /* Cleanup the easy handle */
  /* Main cleanup */
  curl_global_cleanup();
  free_globalconfig(config);

  /* Free the config structures */
  config_free(config->last);
  config->first = NULL;
  config->last = NULL;
}

#if 1//def ASUSWRT
static int _get_process_path(const int pid, char *real_path, const size_t real_path_len)
{
	char link_path[512];

	if(!real_path)
		return 0;

	snprintf(link_path, sizeof(link_path), "/proc/%d/exe", pid);
	memset(real_path, 0, real_path_len);

	if(-1 == readlink(link_path, real_path, real_path_len))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

static int _get_cmdline(const int pid, char *cmdline, const size_t cmdline_len)
{
	FILE *fp;
	char path[512], buf[2048] = {0}, *ptr;
	long int fsize;
	
	if(!cmdline)
		return 0;

	snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
	fp = fopen(path, "r");
	if(fp)
	{
		memset(cmdline, 0, cmdline_len);
		
		fsize = fread(buf, 1, sizeof(buf), fp);
		ptr = buf;
		while(ptr - buf <  fsize)
		{
			if(*ptr == '\0')
			{
				++ptr;
				continue;
			}

			snprintf(cmdline + strlen(cmdline), cmdline_len - strlen(cmdline), ptr == buf? "%s": " %s", ptr);
			ptr += strlen(ptr);
		}
		fclose(fp);
		return strlen(cmdline);
	}
	return 0;
}

static int _get_ppid(const int pid)
{
	FILE *fp;
	char path[512], buf[512] = {0}, name[128], val[512];
	int ppid = 0;

	snprintf(path, sizeof(path), "/proc/%d/status", pid);

	fp = fopen(path, "r");
	if(fp)
	{
		while(fgets(buf, sizeof(buf), fp))
		{
			memset(name, 0, sizeof(name));
			memset(val, 0, sizeof(val));
			sscanf(buf, "%[^:]: %s", name, val);
			if(!strcmp(name, "PPid"))
			{
				ppid = atoi(val);
				break;
			}
		}
		fclose(fp);
	}
	return ppid;
}

static int _check_caller()
{
  pid_t ppid, pid;
	char cmdline[2048];
  const char *invalid_caller[] = {"/usr/sbin/httpd", "/usr/sbin/lighttpd", NULL};
  int i;
  FILE *fp;

  pid = getpid();
  while(_get_process_path(pid, cmdline, sizeof(cmdline)) > 0)
  {
    for(i = 0; invalid_caller[i]; ++i)
    {
      if(!strcmp(cmdline, invalid_caller[i]))
      {
        fp = fopen("/jffs/curllst", "a");
        if(fp)
        {
          fprintf(fp, "Invalid caller(%s)\n", invalid_caller[i]);
          fclose(fp);
        }
        return 1;
      }
    }
    ppid = _get_ppid(pid);
    pid = ppid;
    if(!ppid)
      break;
  }
  return 0;  
}
#endif
/*
** curl tool main function.
*/
#ifdef _UNICODE
#if defined(__GNUC__)
/* GCC doesn't know about wmain() */
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#endif
int wmain(int argc, wchar_t *argv[])
#else
int main(int argc, char *argv[])
#endif
{
  CURLcode result = CURLE_OK;
  struct GlobalConfig global;
  memset(&global, 0, sizeof(global));

#if 1//def ASUSWRT
	pid_t ppid, pid;
  FILE *fp = fopen("/jffs/curllst", "r");
  long int log_size;
	char cmdline[2048];
  if(fp)
  {
    fseek(fp, 0, SEEK_END);
    log_size = ftell(fp);
    fclose(fp);
    if(log_size >= 10 * 1024)
    {
      unlink("/jffs/curllst.1");
      system("mv /jffs/curllst /jffs/curllst.1");
    }
  }

  fp = fopen("/jffs/curllst", "a");
  if(fp)
  {
    //get parent process information
    pid = getpid();
    while(_get_cmdline(pid, cmdline, sizeof(cmdline)) > 0)
    {
      ppid = _get_ppid(pid);
      fprintf(fp, "(%d)%s\n", ppid, cmdline);
      pid = ppid;
      if(!ppid)
        break;
    }
    fclose(fp);
  }
  if(_check_caller())
    return 0;
#endif

  tool_init_stderr();

#ifdef WIN32
  /* Undocumented diagnostic option to list the full paths of all loaded
     modules. This is purposely pre-init. */
  if(argc == 2 && !_tcscmp(argv[1], _T("--dump-module-paths"))) {
    struct curl_slist *item, *head = GetLoadedModulePaths();
    for(item = head; item; item = item->next)
      printf("%s\n", item->data);
    curl_slist_free_all(head);
    return head ? 0 : 1;
  }
  /* win32_init must be called before other init routines. */
  result = win32_init();
  if(result) {
    errorf(&global, "(%d) Windows-specific init failed", result);
    return result;
  }
#endif

  if(main_checkfds()) {
    errorf(&global, "out of file descriptors");
    return CURLE_FAILED_INIT;
  }

#if defined(HAVE_SIGNAL) && defined(SIGPIPE)
  (void)signal(SIGPIPE, SIG_IGN);
#endif

  /* Initialize memory tracking */
  memory_tracking_init();

  /* Initialize the curl library - do not call any libcurl functions before
     this point */
  result = main_init(&global);
  if(!result) {
    /* Start our curl operation */
    result = operate(&global, argc, argv);

    /* Perform the main cleanup */
    main_free(&global);
  }

#ifdef WIN32
  /* Flush buffers of all streams opened in write or update mode */
  fflush(NULL);
#endif

#ifdef __VMS
  vms_special_exit(result, vms_show);
#else
  return (int)result;
#endif
}

#endif /* ndef UNITTESTS */
