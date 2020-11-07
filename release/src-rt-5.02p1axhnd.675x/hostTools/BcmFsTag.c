
/* <:copyright-BRCM:2013:DUAL/GPL:standard
/* 
/*    Copyright (c) 2013 Broadcom 
/*    All Rights Reserved
/* 
/* Unless you and Broadcom execute a separate written software license
/* agreement governing use of this software, this software is licensed
/* to you under the terms of the GNU General Public License version 2
/* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
/* with the following added to such license:
/* 
/*    As a special exception, the copyright holders of this software give
/*    you permission to link this software with independent modules, and
/*    to copy and distribute the resulting executable under terms of your
/*    choice, provided that you also meet, for each linked independent
/*    module, the terms and conditions of the license of that module.
/*    An independent module is a module which is not derived from this
/*    software.  The special exception does not apply to any modifications
/*    of the software.
/* 
/* Not withstanding the above, under no circumstances may you combine
/* this software in any way with any other Broadcom software provided
/* under a license other than the GPL, without Broadcom's express prior
/* written consent.
/* 
 * :> 

 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "bcm_hwdefs.h"

void
usage (char *pgm)
{
  fprintf (stderr, "usage: %s [-s spares]  [-j|-u] erasesize\n Erasesize in bytes or K\n",
	   pgm);
}

int
main (int argc, char **argv)
{
  int opt;
  int i;
  int erase_size;
  int spares = 0;
  int remain;
  char *fstype = "error";
  while ((opt = getopt (argc, argv, "jus:q")) != -1)
    {
      switch (opt)
	{
	case 'j':
	  fstype = BCM_BCMFS_TYPE_JFFS2;
	  break;
	case 'u':
	  fstype = BCM_BCMFS_TYPE_UBIFS;
	  break;
	case 's':
	  spares = atoi(optarg);
	  break;
	case 'q':
	  fstype = BCM_BCMFS_TYPE_SQUBIFS;
	  break;
	default:
	  usage (argv[0]);
	  exit (1);
	}
    }
  if (optind + 1 != argc)
    {
      usage (argv[0]);
      exit (2);
    }
  erase_size = atoi (argv[optind]);
  if (erase_size < 2048)
    {
      erase_size = erase_size * 1024;
    }
  for (i = 0; i < erase_size * (spares + 1)  - 256; i++)
    {
      putchar (0xff);
    }
  remain = 256;
  for (i = 0; i < 4; i++)
    {
      remain = remain - printf ("%s%s%c", BCM_BCMFS_TAG, fstype, 0);
    }
  for (i = 0; i < remain; i++)
    {
      putchar (0xff);
    }
}
