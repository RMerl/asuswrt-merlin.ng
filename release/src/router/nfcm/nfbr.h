#ifndef __NFBR_H__
#define __NFBR_H__

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

#include <libbridge.h>
#include <libbridge_private.h>
#include <brctl.h>

extern int br_cmd_showmacs(int argc, char *const* argv);
extern int br_cmd_showstp(int argc, char *const* argv);

#endif //__NFBR_H__
