
/* <:copyright-BRCM:2013:DUAL/GPL:standard
/* 
/*    Copyright (c) 2013 Broadcom 
/*    All Rights Reserved
/* 
/* This program is free software; you can redistribute it and/or modify
/* it under the terms of the GNU General Public License, version 2, as published by
/* the Free Software Foundation (the "GPL").
/* 
/* This program is distributed in the hope that it will be useful,
/* but WITHOUT ANY WARRANTY; without even the implied warranty of
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/* GNU General Public License for more details.
/* 
/* 
/* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
/* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
/* Boston, MA 02111-1307, USA.
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
