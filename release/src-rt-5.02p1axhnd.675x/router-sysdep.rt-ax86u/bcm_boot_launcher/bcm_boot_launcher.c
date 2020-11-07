/***********************************************************************
 *
 * <:copyright-BRCM:2011:DUAL/GPL:standard
 * 
 *    Copyright (c) 2011 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 *
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include "bcm_ulog.h"

struct element
{
   struct element *next;
   struct element *prev;
   char *filename;
};


int start=0;
int stop=0;
char *dir = "/etc/rc3.d";
struct element *list_head=NULL;
static int build_list(void);
static int insert_element(const char *filename);
static void run_list(void);
static void free_list(void);


static void usage(void)
{
   printf("usage: bcm_boot_launcher [-v] [-d dir] start|stop\n");
   printf("  by default, start mode on %s\n", dir);
   exit(-1);
}

int main(int argc, char **argv)
{
   int c;
   int rc;

   while ((c = getopt(argc, argv, "vspd:")) != -1)
   {
      switch(c) {
      case 'v':
         bcmuLog_setLevel(BCMULOG_LEVEL_DEBUG);
         break;

      case 'd':
         dir = optarg;
         break;

      default:
         usage();
      }
   }

   if (optind+1 != argc)
   {
      usage();
   }

   if (!strcmp(argv[optind], "start"))
   {
      start = 1;
   }
   else if (!strcmp(argv[optind], "stop"))
   {
      stop = 1;
   }
   else
   {
      usage();
   }

   if (start)
   {
      bcmuLog_debug("start mode on dir=%s", dir);
   }
   else
   {
      bcmuLog_debug("stop mode on dir=%s", dir);
   }

   rc = build_list();
   if (rc < 0)
   {
      bcmuLog_error("could not build list of scripts, aborting startup");
      free_list();
      exit(-1);
   }

   run_list();

   free_list();

   exit(0);
}


/** build a doubly linked list of the scripts that we should execute in
 * lexicographical order.
 *
 * @return 0 on success.
 */
int build_list()
{
   DIR *d;
   struct dirent *dent;
   int rc=0;

   if (NULL == (d = opendir(dir)))
   {
      bcmuLog_error("could not open %s, errno=%d", dir, errno);
      return -1;
   }

   while (NULL != (dent = readdir(d)))
   {
      if ((start && dent->d_name[0] == 'S') ||
          (stop && dent->d_name[0] == 'K'))
      {
         rc = insert_element(dent->d_name);
         if (rc < 0)
            break;
      }
   }

   closedir(d);

   return rc;

#ifdef unittest
      insert_element("S30apps");
      insert_element("S10drivers");
      insert_element("S05init");
      insert_element("S70smd");
      insert_element("S40custom");
      insert_element("S80final");
      return 0;
#endif
}

int insert_element(const char *filename)
{
   struct element *ele;
   struct element *curr;

   if (NULL == (ele = malloc(sizeof(struct element))))
   {
      return -1;
   }

   if (NULL == (ele->filename = malloc(strlen(filename)+1)))
   {
      free(ele);
      return -1;
   }

   strcpy(ele->filename, filename);
   ele->next = NULL;
   ele->prev = NULL;

   /* insert */
   if (list_head == NULL)
   {
      list_head = ele;
      ele->next = ele;
      ele->prev = ele;
      return 0;
   }

   curr = list_head;
   do {
      if (strcmp(ele->filename, curr->filename) < 0)
      {
         if (list_head == curr)
         {
            list_head = ele;
         }
         ele->next = curr;
         ele->prev = curr->prev;
         curr->prev->next = ele;
         curr->prev = ele;
         return 0;
      }
      curr = curr->next;
   } while (curr != list_head);


   /* went through the whole list without insertion, insert at end */
   ele->next = list_head;
   ele->prev = list_head->prev;
   list_head->prev->next = ele;
   list_head->prev = ele;

   return 0;
}


void run_list()
{
   struct element *curr;
   char cmdline[1024]={0};
   int rc;

   if (NULL == list_head)
   {
      return;
   }

   curr = list_head;
   do {
      char *action=NULL;
      if (start)
      {
         snprintf(cmdline, sizeof(cmdline)-1, "%s/%s start", dir, curr->filename);
         action = "starting";
      }
      else
      {
         snprintf(cmdline, sizeof(cmdline)-1, "%s/%s stop", dir, curr->filename);
         action = "stopping";
      }

      bcmuLog_debug("%s: %s", action, cmdline);
      if (0 != (rc = system(cmdline)))
      {
         bcmuLog_error("%s: %s returned %d", action, cmdline, rc);
      }

      curr = curr->next;
   } while (curr != list_head);
}


void free_list()
{
   struct element *curr;
   struct element *tmp;

   if (NULL == list_head)
   {
      return;
   }

   list_head->prev->next = NULL;
   curr = list_head;
   while (curr != NULL)
   {
      free(curr->filename);
      tmp = curr->next;
      free(curr);
      curr = tmp;
   }
}
