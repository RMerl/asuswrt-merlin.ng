 /*
 * Copyright 2015, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <nt_db.h>
#include <nt_common.h>
#include <linklist.h>
#include <hashtable.h>

#ifndef __libnt_h__
#define __libnt_h__

/* IFTTT DEBUG DEFINE SETTING 
---------------------------------*/
#define COMMON_IFTTT_DEBUG                     "/tmp/IFTTT_ALEXA"
#define COMMON_IFTTT_LOG_FILE                  "/tmp/IFTTT_ALEXA.log"

#define IFTTT_DEBUG(fmt,args...) \
	if(isFileExist(COMMON_IFTTT_DEBUG) > 0) { \
		Debug2File(COMMON_IFTTT_LOG_FILE, "[%s:(%d)]"fmt, __FUNCTION__, __LINE__, ##args); \
	}

/* nt_share.c */
extern NOTIFY_EVENT_T *initial_nt_event();
extern void nt_event_free(NOTIFY_EVENT_T *e);
extern int send_notify_event(void *data, char *socketpath);
extern int send_trigger_event(NOTIFY_EVENT_T *event);
extern int eInfo_get_idx_by_evalue(int e);
extern char *eInfo_get_eName(int e);
extern int eInfo_get_eAppsid(int e);
extern int eInfo_get_eType(int e);
extern int GetDebugValue(char *path);
extern void SEND_NT_EVENT(int event, const char *msg);

/* nt_utility.c */
#define xfree(ptr) __xfree(ptr); ptr = NULL
#define xvstrsep(buf, sep, args...) _xvstrsep(buf, sep, args, NULL)
extern void Debug2Console(const char * format, ...);
extern void Debug2File(const char *FilePath, const char * format, ...);
extern int isFileExist(char *fname);
extern int isDirectoryExist(char *fname);
extern int get_pid_num_by_name(char *pidName);
extern void StampToDate(unsigned long timestamp, char *date);
extern int xfile_lock(char *tag);
extern void xfile_unlock(int lockfd);
extern int f_read_string(const char *path, char *buffer, int max);
extern int f_write_string(const char *path, const char *buffer, unsigned flags, unsigned cmode);
extern void *xmalloc(size_t size);
extern void *xrealloc(void *ptr, size_t size);
extern char *xstrdup(const char *str);
extern void __xfree(void *ptr);
extern int nextprime(int n);
extern int strfind(const char *str, char ch);
extern size_t filesize(const char *file);
extern void rmEndhar(char *str);
extern int _xvstrsep(char *buf, const char *sep, ...);
extern int x_get_mac(unsigned char *mac_address);

/* nt_db_stat.c */

/* #### API for NC #### */
extern int NT_DBCommand(char *action, NOTIFY_DATABASE_T *input);

/* #### API for httpd #### */
extern int NT_DBAction(struct list *event_list, char *action, NOTIFY_DATABASE_T *input, char *count);
extern int NT_DBActionAPP(struct list *event_list, char *action, NOTIFY_DATABASE_T *input, char *page, char *count);
extern void NT_DBFree(struct list *event_list);
extern int NT_DBCount();

/* #### API for Normal #### */
extern NOTIFY_DATABASE_T *initial_db_input();
extern void db_input_free(NOTIFY_DATABASE_T *input);

#endif
