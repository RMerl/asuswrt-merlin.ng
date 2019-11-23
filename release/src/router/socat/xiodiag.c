/* source: xiodiag.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains source for some diagnostics */

#include "xiosysincludes.h"
#include "xioopen.h"

#include "xiodiag.h"


const char *ddirection[] = {
   "reading", "writing", "reading and writing" } ;

/* compliant with S_ macros (sys/stat.h) */
const char *filetypenames[] = {
   "undef", "named pipe", "character device", "undef",
   "block device", "undef", "directory", "undef",
   "regular file", "undef", "symbolic link", "undef",
   "local socket", "undef", "undef", "\"MT\"?"} ;
