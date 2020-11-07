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


#define _GNU_SOURCE
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
#include "bcm_ulog.h"



/* global vars */
char *cmd=NULL;
char *arg1=NULL;
int verbose=0;


/* forward function declarations */
int do_check(int action);
int do_invalidate(int action);
int do_getPsiSize(int ioc);
int do_copyprimary2backup(void);



void usage(int exitCode)
{
   printf("usage: psictl cmd [options] [args] \n\n");
   printf("where:\n");
   printf("   cmd: check, invalidate, copyprimary2backup, getprimarysize, getbackupsize\n");
   printf("   options: -v or --verbose\n");
   printf("\nexamples:\n");
   printf("    psictl check primary|backup\n");
   printf("    psictl check -v primary|backup\n");
   printf("    psictl invalidate primary|backup\n");
   printf("    psictl invalidate --verbose primary|backup\n");
   printf("    psictl copyprimary2backup\n");
   printf("    psictl getprimarysize\n");
   printf("    psictl getbackupsize\n");
   printf("\n");

   exit(exitCode);
}

/* see userspace/private/apps/stress for more complex long_options usage */
struct option long_options[] =
{
    {"verbose", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'}
};


int main(int argc, char *argv[])
{
   int rv=0;
   int c=0;
   int option_index=0;

   if (argc < 2)
   {
      usage(-1);
   }

   cmd = argv[1];
//   printf("cmd=%s\n", cmd);


   while (c != -1)
   {
      c = getopt_long(argc, argv, "v", long_options, &option_index);

      if (c == -1)
          break;

      switch (c)
      {
      case 'v':
          printf("setting debug\n");
	  bcmuLog_setLevel(BCMULOG_LEVEL_DEBUG);
          verbose=1;
          break;

      case 'h':
           usage(0);
           break;

      default:
            usage(-1);
            break;
      }
   }

//   printf("At end of options processing: argc=%d optind=%d\n", argc, optind);

   if (!strcmp(cmd, "help"))
   {
      usage(0);
   }
   else if (!strcmp(cmd, "check"))
   {
      int action=0;

      if (optind + 2 != argc)
      {
         printf("check requires a target arg\n");
         usage(-1);
      }

      arg1 = argv[optind+1];
      bcmuLog_debug("arg1=%s", arg1);
      if (!strcmp(arg1, "primary"))
      {
         action = PERSISTENT;
      }
      else if (!strcmp(arg1, "backup"))
      {
         action = BACKUP_PSI;
      }
      else
      {
         printf("check arg must be either primary or backup\n");
         usage(-1);
      }

      rv = do_check(action);
   }
   else if (!strcmp(cmd, "invalidate"))
   {
      int action=0;

      if (optind + 2 != argc)
      {
         printf("invalidate requires a target arg\n");
         usage(-1);
      }

      arg1 = argv[optind+1];
      bcmuLog_debug("arg1=%s", arg1);
      if (!strcmp(arg1, "primary"))
      {
         action = PERSISTENT;
      }
      else if (!strcmp(arg1, "backup"))
      {
         action = BACKUP_PSI;
      }
      else
      {
         printf("invalidate arg must be either primary or backup\n");
         usage(-1);
      }

      rv = do_invalidate(action);
   }
   else if (!strcmp(cmd, "copyprimary2backup"))
   {
      if (optind + 1 != argc)
      {
         fprintf(stderr, "%s does not take any args\n", cmd);
         usage(-1);
      }

      rv = do_copyprimary2backup();
   }
   else if (!strcmp(cmd, "getprimarysize"))
   {
      int s;

      if (optind + 1 != argc)
      {
         fprintf(stderr, "%s does not take any args\n", cmd);
         usage(-1);
      }

      s = do_getPsiSize(BOARD_IOCTL_GET_PSI_SIZE);
      if (s < 0)
      {
         rv = -1;
      }
      else
      {
         printf("%d\n", s);
      }
   }
   else if (!strcmp(cmd, "getbackupsize"))
   {
      int s;

      if (optind + 1 != argc)
      {
         fprintf(stderr, "%s does not take any args\n", cmd);
         usage(-1);
      }

      s = do_getPsiSize(BOARD_IOCTL_GET_BACKUP_PSI_SIZE);
      if (s < 0)
      {
         rv = -1;
      }
      else
      {
         printf("%d\n", s);
      }
   }
   else
   {
      fprintf(stderr, "unrecognized command %s\n", cmd);
      usage(-1);
   }

   return rv;
}


int is_valid(const char *buf)
{
   /* currently, only checks the first 4 bytes.  Could check more if we want... */
   /* see oal_mdm.c:getConfigHeaderInfo */

   /* compressed config file starts with <compres */
   if (buf[0] == '<' && buf[1] == 'c' && buf[2] == 'o' && buf[3] == 'm')
   {
      return 1;
   }

   /* CRC uncompressed config file starts with <crc= */
   if (buf[0] == '<' && buf[1] == 'c' && buf[2] == 'r' && buf[3] == 'c')
   {
      return 1;
   }

   /* plain uncompressed config file starts with <?xml version */
   if (buf[0] == '<' && buf[1] == '?' && buf[2] == 'x' && buf[3] == 'm')
   {
      return 1;
   }

   return 0;
}


int do_check(int action)
{
   unsigned char buf[64];
   int rv;

   /*
    * Read in the first 64 bytes of the specified PSI region and try to
    * figure out if it looks reasonably valid.  Does not guarantee it is
    * absolutely valid.  When CMS boots and loads the config file, it will
    * do a full validation.
    */
   memset(buf, 0, sizeof(buf));

   bcmuLog_debug("reading %d bytes from %s",
                sizeof(buf),
                ((action == PERSISTENT) ? "PRIMARY_PSI" : "BACKUP_PSI"));

   rv = devCtl_boardIoctl(BOARD_IOCTL_FLASH_READ,
                action, (char*)buf, sizeof(buf), 0, NULL);

   bcmuLog_debug("rv=%d", rv);

   if (verbose)
   {
      unsigned int i;
      for (i=0; i < sizeof(buf); i+= 8)
      {
         bcmuLog_debug("%02x %02x %02x %02x %02x %02x %02x %02x   %c %c %c %c %c %c %c %c",
                 buf[i],   buf[i+1], buf[i+2], buf[i+3],
                 buf[i+4], buf[i+5], buf[i+6], buf[i+7],
                 buf[i],   buf[i+1], buf[i+2], buf[i+3],
                 buf[i+4], buf[i+5], buf[i+6], buf[i+7]);
      }
   }

   if (rv == 0)
   {
      int v;
      v = is_valid((char *) buf);
      if (v)
      {
         printf("valid\n");
         rv = 0;
      }
      else
      {
         printf("invalid\n");
         rv = -1;
      }
   }
   else
   {
      fprintf(stderr, "BOARD_IOCTL_FLASH_READ failed, rv=%d errno=%d", rv, errno);
      if (action == BACKUP_PSI)
      {
         fprintf(stderr, "check that BACKUP_PSI is enabled in CFE\n");
      }
   }

   return rv;
}


int do_invalidate(int action)
{
   char buf[1024];
   int rv;

   /*
    * CMS code will do a cmsImg_getRealConfigFlashSize and malloc a
    * buffer of that length and fill with zero's and write.  Here, we just
    * write 1KB of zeros.  Maybe later we can have a "full invalidate" option.
    */
   memset(buf, 0, sizeof(buf));

   bcmuLog_debug("writing %d bytes of zeros to %s",
                sizeof(buf),
                ((action == PERSISTENT) ? "PRIMARY_PSI" : "BACKUP_PSI"));

   rv = devCtl_boardIoctl(BOARD_IOCTL_FLASH_WRITE,
               action, buf, sizeof(buf), 0, NULL);

   bcmuLog_debug("rv=%d", rv);
   if (rv != 0)
   {
      fprintf(stderr, "invalidate failed, rv=%d errno=%d\n",
              rv, errno);
      if (action == BACKUP_PSI)
      {
         fprintf(stderr, "check that BACKUP_PSI is enabled in CFE\n");
      }
   }

   return rv;
}

int do_getPsiSize(int ioc)
{
   BOARD_IOCTL_PARMS ioctlParms;
   int fd;
   int rv;

   fd = open(BOARD_DEVICE_NAME, O_RDWR);
   if (fd < 0)
   {
      fprintf(stderr, "could not open %s, errno=%d", BOARD_DEVICE_NAME, errno);
   }

   memset(&ioctlParms, 0, sizeof(ioctlParms));

   rv = ioctl(fd, ioc, &ioctlParms);

   bcmuLog_debug("rv=%d result=%d", rv, ioctlParms.result);

   close(fd);

   if (rv < 0)
   {
      fprintf(stderr, "BOARD_IOCTL_GET_PSI_SIZE failed, rc=%d errno=%d\n", rv, errno);
      if ((unsigned int)ioc == BOARD_IOCTL_GET_BACKUP_PSI_SIZE)
      {
         fprintf(stderr, "check that BACKUP_PSI is enabled in CFE\n");
      }

      return rv;
   }

   return ioctlParms.result;
}


int do_copyprimary2backup()
{
   int primarysize, backupsize;
   char *buf;
   int rv;

   primarysize = do_getPsiSize(BOARD_IOCTL_GET_PSI_SIZE);
   backupsize = do_getPsiSize(BOARD_IOCTL_GET_BACKUP_PSI_SIZE);

   bcmuLog_debug("primarysize=%d backupsize=%d", primarysize, backupsize);

   if (backupsize <= 0)
   {
      fprintf(stderr, "backup PSI size is %d!!\n", backupsize);
      fprintf(stderr, "check that BACKUP_PSI is enabled in CFE\n");
      return -1;
   }

   if (primarysize > backupsize)
   {
      /* these sizes are the sizes of the storage area, not the actual
       * amount of data in them.  The device driver does not report the
       * actual number of valid data read because it does not know what is
       * valid or not.
       */
      fprintf(stderr, "primary psi (%d) is bigger than backup psi (%d)!\n",
              primarysize, backupsize);
      return -1;
   }

   buf = malloc(primarysize);
   if (buf == NULL)
   {
      fprintf(stderr, "malloc of %d bytes failed\n", primarysize);
      return -1;
   }
   memset(buf, 0, primarysize);

   bcmuLog_debug("reading %d bytes from primary", primarysize);

   /* read primary */
   rv = devCtl_boardIoctl(BOARD_IOCTL_FLASH_READ,
               PERSISTENT, buf, primarysize, 0, NULL);

   bcmuLog_debug("read rv=%d", rv);
   if (rv < 0)
   {
      fprintf(stderr, "read failed, rv=%d errno=%d\n", rv, errno);
      free(buf);
      return -1;
   }

   /* write backup */
   rv = devCtl_boardIoctl(BOARD_IOCTL_FLASH_WRITE,
               BACKUP_PSI, buf, primarysize, 0, NULL);

   bcmuLog_debug("write rv=%d", rv);

   free(buf);

   return rv;
}
