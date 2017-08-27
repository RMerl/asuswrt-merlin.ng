/***********************************************************************
 *
 *  Copyright (c) 2007-2016  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2016:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

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

#define WDT_DBG       0
#define MIN_TIMEOUT_SECS     4
#define MAX_TIMEOUT_SECS     80
#define DEFAULT_TIMEOUT_SECS MAX_TIMEOUT_SECS
#define WDTD_SLEEP_DIV 4
#define WDTD_NAME "wdtd"
#define EXIT_SUCCESS 0  
#define EXIT_FAILURE 1  

int wd_fd=0, ret;

void usage(char * appName)
{
   printf("Usage: %s [-d] [-t <timeout>] start \n", appName);
   printf("       %s [-p <pid of wdtd daemon>] stop \n", appName);
   printf("       %s ping \n", appName);
   printf("                                                                        \n" );
   printf("options:                                                                \n" );
   printf("       -t : Specify a timeout in [%d-%d] seconds for the watchdog timer \n", MIN_TIMEOUT_SECS, MAX_TIMEOUT_SECS);
   printf("       -d : start the wdtd daemon which pings the watchdog              \n" );
   printf("            every timeout/4 secsonds                                    \n" );
   printf("       -p : stop the wdtd daemon which has the specified pid            \n" );
   printf("                                                                        \n" );
   printf("Note:                                                                   \n" );
   printf("       The watchdog timer can be controlled using 2 mechanisms:         \n" );
   printf("       (1) Direct asynchronous control: In this method the watchdog is  \n" );
   printf("           directly controlled by the invoking script                   \n" );
   printf("                                                                        \n" );
   printf("              %s -t 10 start                                            \n", appName);
   printf("              %s ping                                                   \n", appName);
   printf("              %s ping                                                   \n", appName);
   printf("              ...                                                       \n" );
   printf("              %s stop                                                   \n", appName);
   printf("                                                                        \n" );
   printf("       (2) Daemonized: In this method the watchdog is pinged by the wdtd\n" );
   printf("           daemon which runs every timeout/4 seconds                    \n" );
   printf("              %s -t 10 -d start                                         \n", appName);
   printf("              ...                                                       \n" );
   printf("              %s -p <pid of wdtd daemon> stop                           \n", appName);
   exit(1);
}

void sighandler (int signum, siginfo_t *info, void *context) 
{
   int opts;
   fprintf (
      stderr,
      "Recieved %d from pid %u, uid %u.\n",
      info->si_signo,
      info->si_pid,
      info->si_uid
   );
   if( wd_fd )
   {
      /* Stop the wd */
      opts = WDIOS_DISABLECARD;
      ioctl(wd_fd, WDIOC_SETOPTIONS, &opts);
      close(wd_fd);
   }
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

void daemonize(char * name, int timeout)  
{  
   pid_t pid, sid;  
   int fd;   
 
   /* already a daemon */  
   if ( getppid() == 1 ) return;  
 
   /* Fork off the parent process */  
   pid = fork();  

   if (pid < 0)    
   {     
       exit(EXIT_FAILURE);  
   }     
 
   if (pid > 0)    
   {     
       /* Dump child's pid for system */
       fprintf( stdout,"%d\n", pid);
       /*Killing the Parent Process*/
       exit(EXIT_SUCCESS);   
   }     
 
#if WDT_DBG 
   /* At this point we are executing as the child process */  
   setlogmask (LOG_UPTO (LOG_NOTICE));
   openlog (WDTD_NAME, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
   syslog (LOG_NOTICE, 
      "Daemon started with ping interval = %ds, timeout = %ds", 
          timeout/WDTD_SLEEP_DIV, timeout);
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
       dup2 (fd, STDERR_FILENO);  
 
       if (fd > 2)  
       {  
           close (fd);  
       }  
   }  

   /*resetting File Creation Mask */  
   umask(0);  

   /* Ping watchdog every x seconds */
   while(1)
   {
#if WDT_DBG 
      syslog (LOG_NOTICE, "Pinging Watchdog every %ds", timeout/WDTD_SLEEP_DIV);
#endif
      ret = write(wd_fd, "\0", 1);
      sleep(timeout/WDTD_SLEEP_DIV);
   }
}  

int main(int argc, char *argv[])
{
   unsigned int opts = 0;
   unsigned int timeout = DEFAULT_TIMEOUT_SECS;
   unsigned int launch_wdtd = 0;
   int c;
   int i;

   /* Init signal handler */
   install_sighandler();

   /* Basic argument check */
   if (argc < 2)
   {
      usage(argv[0]);
   }
   
   /* Main argument check */
   if (!strcmp(argv[argc-1], "start") || !strcmp(argv[argc-1], "ping"))
   {
      if (!strcmp(argv[argc-1], "start"))
      {
         while ( (c = getopt(argc, argv, "dt:")) != -1) 
         {
            switch (c) 
            {
               case 'd':
               {
                  launch_wdtd = 1;
                  break; 
               }
               case 't':
               {
                  timeout = atoi(optarg);
                  if ( timeout > MAX_TIMEOUT_SECS || timeout < MIN_TIMEOUT_SECS )
                  {
                     printf("Invalid Timeout value!\n");
                     usage(argv[0]);
                  }
                  break;
               }
               default:
                  usage(argv[0]);
            }
         }
      }

      /* Open watchdog -- also pings the watchdog */
      wd_fd = open("/dev/watchdog", O_WRONLY);
      if (wd_fd == -1) 
      {
         perror("watchdog");
         exit(EXIT_FAILURE);
      }
 
      /* Set timeout -- also pings the watchdog */
      if (!strcmp(argv[argc-1], "start"))
         ret = ioctl(wd_fd, WDIOC_SETTIMEOUT, &timeout);

      /* Start wdtd daemon */
      if (launch_wdtd)
      {
#if WDT_DBG 
         printf("Launching Watchdog Daemon\n");
#endif	 
         for( i=0; i<argc; i++)
         {
            memset( argv[i], '\0', strlen(argv[i]));
         }
         daemonize( argv[0], timeout );
      }
   }
   else if (!strcmp(argv[argc-1], "stop"))
   {
      while ( (c = getopt(argc, argv, "p:")) != -1) 
      {
         switch (c) 
         {
            case 'p':
            {
               /* Shutdown wdtd daemon */
               kill(atoi(optarg), SIGHUP);
               exit(EXIT_SUCCESS);
               break;
            }
            default:
            {
               usage(argv[0]);
            }
         }
      }

      /* Stop the wd */
      wd_fd = open("/dev/watchdog", O_WRONLY);
      if (wd_fd == -1) 
      {
         perror("watchdog");
         exit(EXIT_FAILURE);
      }
      opts = WDIOS_DISABLECARD;
      ioctl(wd_fd, WDIOC_SETOPTIONS, &opts);
   }
   else  
   {
      usage(argv[0]);
   }

   if( wd_fd )
      close(wd_fd);

   return ret;
}

