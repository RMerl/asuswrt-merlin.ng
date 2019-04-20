#ifndef _INCLUDE_SYSTEMD_H_
#define _INCLUDE_SYSTEMD_H_

#include <sys/types.h>

#ifdef BUILD_SYSTEMD
void sd_ct_watchdog_init(void);
void sd_ct_init(void);
void sd_ct_mainpid(pid_t pid);
void sd_ct_stop(void);
#else /* BUILD_SYSTEMD */
static inline void sd_ct_watchdog_init(void) {};
static inline void sd_ct_init(void) {};
static inline void sd_ct_mainpid(pid_t pid) {};
static inline void sd_ct_stop(void) {};
#endif /* BUILD_SYSTEMD */

#endif /* _INCLUDE_SYSTEMD_H_ */
