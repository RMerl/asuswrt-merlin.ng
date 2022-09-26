#ifndef __AUPNPC_UTIL_H__
#define __AUPNPC_UTIL_H__

//debug message
extern void aupnpc_print(const char *file, const char *format, ...);
#define AUPNPC_LOG_PATH	"/tmp/aupnpc.log"
#define AUPNPC_IPC_LOG_PATH	"/tmp/aupnpc_ipc.log"
#define AUPNC_MAX_LOG_SIZE 10*1024

#define AUPNPC_DBG(fmt, args...) \
	do { \
		aupnpc_print(AUPNPC_LOG_PATH, fmt, ##args); \
	} while(0)

#define AUPNPC_IPC_DBG(fmt, args...) \
	do { \
		aupnpc_print(AUPNPC_IPC_LOG_PATH, fmt, ##args); \
	} while(0)


//ipc message
#define AUPNPC_ACT_ADD 1
#define AUPNPC_ACT_DEL  2
typedef struct _AUPNPC_MSG
{
	int act;	//1//1: add, 2:del other is invalid
	char protocol[4];	//TCP/UDP
	int ext_port;
	char src_ip[16];	//IPv4 only now.
	unsigned int duration;
} AUPNPC_MSG;
extern int valid_aupnpc_msg(const AUPNPC_MSG *msg);

//ipc functions
extern int send_msg_to_ipc_socket(const char *ipc_socket_path, const char *msg, const size_t msg_len, char *rsp_msg, const int rsp_msg_max_len, const int timeout, const char *dbg_file);
extern int read_msg_from_ipc_socket(const int fd, char *msg, const int msg_max_len, const char *rsp_msg, const size_t rsp_msg_len, const int timeout, const char *dbg_file);

//event list and event queue
typedef struct _event_list EVENT_LIST;
struct _event_list
{
	EVENT_LIST *next;
	AUPNPC_MSG *data;
};
extern int evq_push_msg(EVENT_LIST **head, AUPNPC_MSG *msg);
extern AUPNPC_MSG *evq_pop_msg(EVENT_LIST **head);


//misc
#define SAFE_FREE(x) if(x){free(x); x=NULL;}

#endif
