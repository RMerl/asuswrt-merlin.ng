#ifndef __TIME_UTIL_H__
#define __TIME_UTIL_H__

char* alloc_time_string(const char* tf, int is_msec, char** time_string);
void  dealloc_time_string(char* ts);

// compare with local time
int aae_timegm(struct tm *ptm);
int is_device_ticket_expired(const char *exp_utc_time_str);
int is_device_ticket_expired_by_utc_ts(const time_t timet_expire_utc);
#endif
