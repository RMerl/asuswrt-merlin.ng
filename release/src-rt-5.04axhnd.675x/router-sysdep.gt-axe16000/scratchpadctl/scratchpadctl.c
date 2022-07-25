/***********************************************************************
 *
 *  Copyright (c) 2007-2010  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:DUAL/GPL:standard

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


// #define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

/* for the Broadcom board driver ioctl codes, in bcmdrivers/opensource/include/bcm963xx */
#include "board.h"
#include "bcm_boarddriverctl.h"

/* forward function declarations */
int processListCmd(void);
int processDumpCmd(const char *key, const int isascii);
int processDeleteCmd(const char *key);
int processSetCmd(const char *key, const char *value);
int processClearAllCmd(void);

void usage(int exitCode)
{
   printf("usage: scratchpadctl cmd \n\n");
   printf("   supported cmds:\n");
   printf("   list        : list all keys\n");
   printf("   dump <key>  : dump the contents of the specified key\n");
   printf("   delete <key>: delete the specified key and its contents\n");
   printf("   set <key> <content>: set the specified key to content\n");
   printf("   clearall    : delete all keys and contents\n");
   printf("   help        : print usage and exit\n");
   printf("\n");

   exit(exitCode);
}


int main(int argc, char *argv[])
{
   int rv=0;
   char *cmd=NULL;

   if (argc < 2)
   {
      printf("a command must be given\n");
      usage(-1);
   }

   cmd = argv[1];


   if (!strcmp(cmd, "list"))
   {
      if (argc != 2)
      {
         printf("the list command does not take arguments\n");
         usage(-1);
      }
      rv = processListCmd();
   }
   else if (!strcmp(cmd, "dump"))
   {
      if (argc < 3)
      {
         printf("must specify key for the dump command\n");
         usage(-1);
      }
      else if (argc > 3)
      {
         printf("too many arguments for the dump command\n");
         usage(-1);
      }
      rv = processDumpCmd(argv[2], 0);
   }
   else if (!strcmp(cmd, "adump"))
   {
      if (argc < 3)
      {
         printf("must specify key for the dump command\n");
         usage(-1);
      }
      else if (argc > 3)
      {
         printf("too many arguments for the dump command\n");
         usage(-1);
      }
      rv = processDumpCmd(argv[2], 1);
   }
   else if (!strcmp(cmd, "delete"))
   {
      if (argc < 3)
      {
         printf("must specify key for the delete command\n");
         usage(-1);
      }
      else if (argc > 3)
      {
         printf("too many arguments for the delete command\n");
         usage(-1);
      }
      rv = processDeleteCmd(argv[2]);
   }
   else if (!strcmp(cmd, "set"))
   {
      if (argc < 4)
      {
         printf("must specify key and string content\n");
         usage(-1);
      }
      else if (argc > 4)
      {
         printf("too many arguments for the set command\n");
         usage(-1);
      }
      rv = processSetCmd(argv[2], argv[3]);
   }
   else if (!strcmp(cmd, "clearall"))
   {
      if (argc != 2)
      {
         printf("the clearall command does not take arguments\n");
         usage(-1);
      }
      rv = processClearAllCmd();
   }
   else if (!strcmp(cmd, "help") ||
            !strcmp(cmd, "--help") ||
            !strcmp(cmd, "-h"))
   {
      usage(0);
   }
   else
   {
      printf("Unrecognized cmd (%s) \n", cmd);
      usage(-1);
   }

   return rv;
}


int do_boardIoctl(unsigned int ioc, int action, const char *key,
                  char *buf, unsigned int len)
{
   int rv;

   rv = devCtl_boardIoctl(ioc, action, (char*)key, 0, len, buf);

   return rv;
}

int processListCmd()
{
   int rc;
   char buf[1024]={0};

   rc =  do_boardIoctl(BOARD_IOCTL_FLASH_LIST, SCRATCH_PAD, NULL,
                       buf, sizeof(buf));
   if (rc < 0)
   {
      fprintf(stderr, "error during list cmd, rc=%d errno=%d", rc, errno);
   }
   else
   {
      int i=0;
      while (i < rc)
      {
         printf("%s\n", &buf[i]);

         /* find the null terminator and advance beyond that
          * to get to the next key */
         while (i < rc && buf[i] != 0)
         {
            i++;
         }
         i++;
      }
   }

   return rc;
}


/** Return 1 if the given buffer contains non-printable binary char;
 *  Otherwise, return 0;
 */
int isBinaryBuf(const char *buf, int len)
{
   int i;

   for (i=0; i < len; i++)
   {
      if (buf[i] < '!' || buf[i] > '}')
      {
         return 1;
      }
   }

   return 0;
}

int processDumpCmd(const char *key, const int isascii)
{
   int rc;
   char buf[4096]={0};

   rc =  do_boardIoctl(BOARD_IOCTL_FLASH_READ, SCRATCH_PAD, key,
                       buf, sizeof(buf));
   if (rc == 0)
   {
      printf("key %s not found in Persistent Scratch Pad\n", key);
      rc = -1;
   }
   else if (rc < 0)
   {
      printf("key %s too large to display, need %d bytes", key, (rc * -1));
   }
   else
   {
      if ((isascii == 0) && isBinaryBuf(buf, rc))
      {
         int i=0, j;
         printf("dumping key %s (dataLength=%d bytes)\n", key, rc);

         while (i < rc)
         {
            printf("%04d: ", i);
            for (j=0; j < 16 && i < rc; j++, i++)
            {
               printf("%02x", buf[i]);
            }
            printf("\n");
         }
      }
      else
      {
         printf("%s\n", buf);
      }
   }

   return rc;
}


int processDeleteCmd(const char *key)
{
   int rc;

   /* delete is just a set with a NULL buffer */
   rc =  do_boardIoctl(BOARD_IOCTL_FLASH_WRITE, SCRATCH_PAD, key,
                       NULL, 0);

   return rc;
}


int processSetCmd(const char *key, const char *value)
{
   int rc;

   rc =  do_boardIoctl(BOARD_IOCTL_FLASH_WRITE, SCRATCH_PAD, key,
                       (char *) value, strlen(value));

   return rc;
}


int processClearAllCmd(void)
{
   int rc;

   rc =  do_boardIoctl(BOARD_IOCTL_FLASH_WRITE, SCRATCH_PAD, "",
                       "", -1);

   return rc;
}
