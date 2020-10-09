
#pragma once

#include <sys/types.h>

__inline struct group * getgrnam(char* g){return 0;}

struct group
  {
    char *gr_name;
    char *gr_passwd;
    __gid_t gr_gid;
    char **gr_mem;
  };

#define getgrgid(i) NULL
