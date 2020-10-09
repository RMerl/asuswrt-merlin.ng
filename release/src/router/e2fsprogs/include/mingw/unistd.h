
#pragma once

#include_next <unistd.h>

__inline __uid_t getuid(void){return 0;}
__inline int geteuid(void){return 1;}

__inline __gid_t getgid(void){return 0;}
__inline __gid_t getegid(void){return 0;}

// no-oped sync
__inline void sync(void){};
