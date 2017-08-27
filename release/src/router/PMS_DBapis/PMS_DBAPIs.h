
#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sqlite3APIs.h>

// define
#define PMS_DB_FILE "/jffs/.sys/Permission/PMS_data.db3"

// struct
#ifndef U32
#define U32
typedef unsigned int u32;
#endif
typedef struct _pms_account_info_t_ PMS_ACCOUNT_INFO_T;
typedef struct _pms_account_group_info_t_ PMS_ACCOUNT_GROUP_INFO_T;
typedef struct _pms_device_info_t_ PMS_DEVICE_INFO_T;
typedef struct _pms_device_group_info_t_ PMS_DEVICE_GROUP_INFO_T;
typedef struct _pms_account_group_match_info_t_ PMS_ACC_GROUP_MATCH_INFO_T;
typedef struct _pms_owned_info_t_ PMS_OWNED_INFO_T;

struct _pms_account_info_t_
{
	u32 active;
	char *name;
	char *passwd;
	char *desc;
	char *email;

	u32 owned_group_num; // the number of owned_group
	PMS_OWNED_INFO_T *owned_group;
	PMS_ACCOUNT_INFO_T *next;
};

struct _pms_account_group_info_t_
{
	u32 active;
	char *name;
	char *desc;

	u32 owned_account_num; // the number of owned_account
	PMS_OWNED_INFO_T *owned_account;
	PMS_ACCOUNT_GROUP_INFO_T *next;
};

struct _pms_device_info_t_
{
	u32 active;
	char *mac;
	char *devname;
	u32 devtype;
	char *desc;

	u32 owned_group_num; // the number of owned_group
	PMS_OWNED_INFO_T *owned_group;
	PMS_DEVICE_INFO_T *next;
};

struct _pms_device_group_info_t_
{
	u32 active;
	char *name;
	char *desc;

	u32 owned_device_num; // the number of owned_device
	PMS_OWNED_INFO_T *owned_device;
	PMS_DEVICE_GROUP_INFO_T *next;
};

struct _pms_account_group_match_info_t_
{
	char *name;
	char *group;

	PMS_ACC_GROUP_MATCH_INFO_T *next;
};

struct _pms_owned_info_t_
{
	void *member;

	PMS_OWNED_INFO_T *next;
};

typedef struct{
	int action;
	int (*cb_acc)(sqlite3 *, PMS_ACCOUNT_INFO_T *, char *reserved);
	int (*cb_group)(sqlite3 *, PMS_ACCOUNT_GROUP_INFO_T *, char *reserved);
}_PMS_ActionAccAPIs; 

typedef struct{
	int action;
	int (*cb_acc)(sqlite3 *, PMS_DEVICE_INFO_T *, char *reserved);
	int (*cb_group)(sqlite3 *, PMS_DEVICE_GROUP_INFO_T *, char *reserved);
}_PMS_ActionDevAPIs; 

enum{
	PMS_ACTION_ADD,
	PMS_ACTION_DELETE,
	PMS_ACTION_UPDATE,
	PMS_ACTION_MODIFY,
	PMS_ACTION_GET_FULL,
	PMS_ACTION_GET_NAME,
};

// PMS_DBAPIs.c
extern PMS_ACCOUNT_INFO_T *PMS_list_Account_new(int, char *, ...);
extern PMS_ACCOUNT_GROUP_INFO_T *PMS_list_AccountGroup_new(int, char *, ...);
extern PMS_DEVICE_INFO_T *PMS_list_Device_new(int, char *, ...);
extern PMS_DEVICE_GROUP_INFO_T *PMS_list_DeviceGroup_new(int, char *, ...);
extern int PMS_ActionAccountInfo(int , void *, int , ...);
extern int PMS_ActionDeviceInfo(int , void *, int , ...);
extern void PMS_list_ACCOUNT_free(PMS_ACCOUNT_INFO_T *);
extern void PMS_list_ACCOUNT_GROUP_free(PMS_ACCOUNT_GROUP_INFO_T *);
extern void PMS_list_DEVICE_free(PMS_DEVICE_INFO_T *);
extern void PMS_list_DEVICE_GROUP_free(PMS_DEVICE_GROUP_INFO_T *);
extern int PMS_ActAccMatchInfo(int , int, char *);
extern int PMS_ActAccGroupMatchInfo(int, int, char *);
extern int PMS_ActDevMatchInfo(int , int, char *);
extern int PMS_ActDevGroupMatchInfo(int, int, char *);
extern int PMS_CreateAllTables();
extern int PMS_GetAccountInfo(int, PMS_ACCOUNT_INFO_T **, PMS_ACCOUNT_GROUP_INFO_T **, int *, int *);
extern int PMS_GetDeviceInfo(int, PMS_DEVICE_INFO_T **, PMS_DEVICE_GROUP_INFO_T **, int *, int *);
extern void PMS_FreeAccInfo(PMS_ACCOUNT_INFO_T **, PMS_ACCOUNT_GROUP_INFO_T **);
extern void PMS_FreeDevInfo(PMS_DEVICE_INFO_T **, PMS_DEVICE_GROUP_INFO_T **);
