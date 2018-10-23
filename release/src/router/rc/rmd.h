#ifndef __RMD_H__
#define __RMD_H__

#define NORMAL_PERIOD           1
#define RMD_IPC_MAX_CONNECTION	5

/* Debug Print */
#define RMD_DEBUG_ERROR		0x000001
#define RMD_DEBUG_WARNING	0x000002
#define RMD_DEBUG_INFO		0x000004
#define RMD_DEBUG_EVENT		0x000008
#define RMD_DEBUG_DETAIL	0x000010

#define RMD_DEBUG "/tmp/RMD_DEBUG"

static int msglevel = 0;//OBD_DEBUG_ERROR | OBD_DEBUG_INFO | OBD_DEBUG_EVENT | OBD_DEBUG_DETAIL;

#define RMD_ERROR(fmt, arg...) \
	do { if ((msglevel & RMD_DEBUG_ERROR) || f_exists(RMD_DEBUG) > 0) \
		dbg("RMD %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define RMD_WARNING(fmt, arg...) \
	do { if ((msglevel & RMD_DEBUG_WARNING) || f_exists(RMD_DEBUG) > 0) \
		dbg("RMD %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define RMD_INFO(fmt, arg...) \
	do { if ((msglevel & RMD_DEBUG_INFO) || f_exists(RMD_DEBUG) > 0) \
		dbg("RMD %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define RMD_EVENT(fmt, arg...) \
	do { if ((msglevel & RMD_DEBUG_EVENT) || f_exists(RMD_DEBUG) > 0) \
		dbg("RMD %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define RMD_DBG(fmt, arg...) \
	do { if ((msglevel & RMD_DEBUG_DETAIL) || f_exists(RMD_DEBUG) > 0) \
		dbg("RMD %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#define RMD_PRINT(fmt, arg...) \
	do { dbg("RMD %s(%d): "fmt, __FUNCTION__, __LINE__, ##arg); \
	} while (0)

#endif
