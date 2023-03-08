/***********************************************************************
 *
 *  Copyright (c) 2007-2016  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2016:DUAL/GPL:standard

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
 *
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>

#define WDT_DBG       0
#define MIN_TIMEOUT_SECS     4
#define MAX_TIMEOUT_SECS     80
#define DEFAULT_TIMEOUT_SECS MAX_TIMEOUT_SECS
#define WDTD_SLEEP_DIV       4
#define WDTD_NAME            "wdtd"
#define WDTD_DEVICE_NAME     "/dev/watchdog"
#define WDTD_PID_FILE        "/var/run/wdtd.pid"
#define EXIT_SUCCESS 0  
#define EXIT_FAILURE 1  


// Global file descriptor into the watchdog device in kernel
int wd_fd = -1;
unsigned int gtimeout = DEFAULT_TIMEOUT_SECS;


// Macros to make the important actions of this app more visible.
// Assumes global vars wd_fd and gtimeout are set appropriately.
#define WD_OPEN()    wd_open()
#define WD_START()   ioctl(wd_fd, WDIOC_SETTIMEOUT, &gtimeout)
#define WD_PING()    write(wd_fd, "\0", 1)
#define WD_STOP()    {int opts = WDIOS_DISABLECARD; \
                      ioctl(wd_fd, WDIOC_SETOPTIONS, &opts); }


static void wd_open()
{
   if (wd_fd != -1)
   {
      // Code path trying to open twice.  Just use the existing fd.
      printf("wdtctl: (internal error) wd_fd is already opened at %d\n", wd_fd);
      return;
   }

   // Just opening the device also pings it (assuming someone had already
   // opened it and set a timeout).  So for the "ping" operation,
   // all we do is open.  (But maybe more clear if we also ping it.)
   wd_fd = open(WDTD_DEVICE_NAME, O_WRONLY);
   if (wd_fd == -1) 
   {
      if (errno == EBUSY)
      {
          printf("Could not open %s (EBUSY).  Is there a wdtd running?\n",
                 WDTD_DEVICE_NAME);
      }
      else
      {
         perror("watchdog");
      }
      exit(EXIT_FAILURE);
   }
}


void usage(char * appName)
{
   printf("Usage: %s [-d] [-t <timeout>] start \n", appName);
   printf("       %s [-d] [-p <pid of wdtd daemon>] stop \n", appName);
   printf("       %s ping \n", appName);
   printf("                                                                        \n" );
   printf("options:                                                                \n" );
   printf("       -t : Specify a timeout in [%d-%d] seconds for the watchdog timer \n", MIN_TIMEOUT_SECS, MAX_TIMEOUT_SECS);
   printf("       -d : if used with start, start the wdtd daemon which pings the watchdog\n" );
   printf("            every timeout/4 seconds.                                    \n" );
   printf("          : if used with stop, automatically find the pid of the wdtd daemon\n");
   printf("            and stop it, which also stops the watchdog timer itself.\n");
   printf("       -p : stop the wdtd daemon which has the specified pid            \n" );
   printf("            This is the old way, using -d stop is easier.  \n");
   printf("                                                                        \n" );
   printf("Note:                                                                   \n" );
   printf("       The watchdog timer can be controlled using 2 mechanisms:         \n" );
   printf("       (1) Direct control: In this method the watchdog is  \n" );
   printf("           directly controlled by wdtctl                   \n" );
   printf("                                                                        \n" );
   printf("              %s -t 10 start                                            \n", appName);
   printf("              %s ping                                                   \n", appName);
   printf("              %s ping                                                   \n", appName);
   printf("              ...                                                       \n" );
   printf("              %s stop                                                   \n", appName);
   printf("                                                                        \n" );
   printf("       (2) Daemonized: In this method the watchdog is pinged by the wdtd\n" );
   printf("           daemon which runs every timeout/4 seconds                    \n" );
   printf("              %s -d -t 10 start                                         \n", appName);
   printf("              ...                                                       \n" );
   printf("              %s -d stop  (or... below)        \n", appName);
   printf("              %s -p <pid of wdtd daemon> stop  (this is the old way, using -d stop is easier) \n", appName);
   exit(1);
}


// Write out the pid of the wdtd daemon to a pid file so it is easier to
// find it and kill it later.  Assumes there is not wdtd daemon running
// already, which should be safe assumption since app will try to open the
// /dev/watchdog file and if wdtd daemon is already running, the open will fail.
void write_wdpid()
{
   int fd;
   char buf[32]={0};

   // Maybe could get fancier and look up the pid in /proc to make sure it
   // is not running.
   unlink(WDTD_PID_FILE);

   fd = open(WDTD_PID_FILE, O_RDWR|O_CREAT, 0644);
   if (fd < 0)
   {
      fprintf(stderr, "%s: could not open pid file!\n", WDTD_NAME);
      return;
   }

   snprintf(buf, sizeof(buf), "%d", getpid());
   write(fd, buf, strlen(buf));
   close(fd);
   return;
}

// Return the pid of the wdtd daemon, or -1 if not found.
int get_wdpid()
{
   int fd;
   int rc;
   int pid = -1;
   char buf[32]={0};

   fd = open(WDTD_PID_FILE, O_RDONLY);
   if (fd < 0)
   {
      return -1;
   }

   rc = read(fd, buf, sizeof(buf));
   if (rc > 0)
   {
      pid = atoi(buf);
   }

   close(fd);

   return pid;
}

void sighandler (int signum, siginfo_t *info, void *context) 
{
   if (info->si_signo == SIGHUP)
      fprintf(stderr, "%s: Recieved SIGHUP from pid %u\n",
              WDTD_NAME, info->si_pid);
   else
      fprintf(stderr, "%s: Recieved signal %d from pid %u\n",
              WDTD_NAME, info->si_signo, info->si_pid);

   WD_STOP();
   unlink(WDTD_PID_FILE);

   fprintf(stderr, "%s: stopped watchdog timer and exit.\n", WDTD_NAME);
   exit(EXIT_SUCCESS);
}

void install_sighandler (void)
{
   struct sigaction sa;
   memset(&sa, 0, sizeof(sa));
   sa.sa_sigaction = sighandler;
   sa.sa_flags = SA_SIGINFO;
   sigaction(SIGHUP, &sa, NULL);
}             

void daemonize(char * name)
{  
   pid_t pid, sid;  
   int fd;   

   pid = fork();  

   if (pid < 0)    
   {     
       fprintf(stderr, "%s: failed to fork/daemonize!\n", WDTD_NAME);
       exit(EXIT_FAILURE);  
   }     

   if (pid > 0)    
   {     
       // Parent is done, just exit now.
       exit(EXIT_SUCCESS);   
   }     

   /*
    * At this point we are executing as the child process.
    */
   fprintf(stderr, "%s started at pid %d (timeout=%d seconds)\n",
            WDTD_NAME, getpid(), gtimeout);

   /* Init signal handler */
   install_sighandler();

#if WDT_DBG 
   setlogmask (LOG_UPTO (LOG_NOTICE));
   openlog (WDTD_NAME, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
   syslog (LOG_NOTICE, 
      "Daemon started with ping interval = %ds, gtimeout = %ds", 
          gtimeout/WDTD_SLEEP_DIV, gtimeout);
#endif   
 
   /* Create a new SID for the child process */  
   sid = setsid();  
   if (sid < 0)    
   {  
       exit(EXIT_FAILURE);  
   }  

   /* Change the process name */  
   sprintf(name, WDTD_NAME);

   /* Change the current working directory. */  
   if ((chdir("/")) < 0)  
   {  
       exit(EXIT_FAILURE);  
   }  
 
   /* Handle opened filed descriptors */
   fd = open("/dev/null",O_RDWR, 0);  
   if (fd != -1)  
   {  
       dup2 (fd, STDIN_FILENO);  
       dup2 (fd, STDOUT_FILENO);  
       // Do not close stderr on daemon so it can still print some info
       // dup2 (fd, STDERR_FILENO);  
 
       if (fd > 2)  
       {  
           close (fd);  
       }  
   }  

   /* Write out my pidfile */
   write_wdpid();

   /*resetting File Creation Mask */  
   umask(0);  

   /* Ping watchdog every x seconds */
   while(1)
   {
#if WDT_DBG 
      syslog (LOG_NOTICE, "Pinging Watchdog every %ds", gtimeout/WDTD_SLEEP_DIV);
#endif
      WD_PING();
      sleep(gtimeout/WDTD_SLEEP_DIV);
   }
}  

int main(int argc, char *argv[])
{
   int c, i;
   int daemon_mode = 0;
   int pid = -1;
   int ret = 0;

   /* Basic argument check */
   if (argc < 2)
   {
      usage(argv[0]);
   }

   // ping is very easy to handle, just do that first and exit.
   if (!strcmp(argv[argc-1], "ping"))
   {
      if (argc != 2)  // ping takes no options
      {
         usage(argv[0]);
      }
      WD_OPEN();
      WD_PING();
      exit(EXIT_SUCCESS);
   }

   // parse args
   while ( (c = getopt(argc, argv, "dp:t:")) != -1) 
   {
      switch (c) 
      {
         case 'd':
         {
            daemon_mode = 1;  // used in both start and stop
            break; 
         }

         case 't':
         {
            if (strcmp(argv[argc-1], "start")) // used only for start
               usage(argv[0]);

            gtimeout = atoi(optarg);
            if ( gtimeout > MAX_TIMEOUT_SECS || gtimeout < MIN_TIMEOUT_SECS )
            {
               printf("Invalid Timeout value %d!\n", gtimeout);
               usage(argv[0]);
            }
            break;
         }

         case 'p':
         {
            if (strcmp(argv[argc-1], "stop")) // used only for stop
               usage(argv[0]);

            pid = atoi(optarg);
            break;
         }

         default:
             usage(argv[0]);
      }
   }

   // There is only two possible commands at this point, start or stop.
   if (!strcmp(argv[argc-1], "start"))
   {
      // If we are able to open the watchdog device, that means there is no
      // other wdtd daemon running.  So this acts as a check for daemon_mode.
      WD_OPEN();
      WD_START();  // uses global var gtimeout

      /* Start wdtd daemon */
      if (daemon_mode)
      {
         for( i=0; i<argc; i++)
         {
            memset( argv[i], '\0', strlen(argv[i]));
         }
         daemonize(argv[0]);
      }
      else
      {
         printf("Started watchdog timer (%d seconds)\n", gtimeout);
      }
   }
   else
   {
      // This must be the STOP command
      if (pid != -1)
      {
         kill(pid, SIGHUP);
      }
      else if (daemon_mode)
      {
         pid = get_wdpid();
         if (pid == -1)
         {
            printf("Could not find pid to kill\n");
            exit(EXIT_FAILURE);
         }

         kill(pid, SIGHUP);
      }
      else
      {
         WD_OPEN();
         WD_STOP();
         printf("Stopped watchdog timer\n");
      }
   }

   return ret;
}

