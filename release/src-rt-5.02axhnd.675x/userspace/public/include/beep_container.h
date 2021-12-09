/***********************************************************************
 *
 * Copyright (c) 2017  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2017:DUAL/GPL:standard
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 *
 * :>
 *
 ************************************************************************/

#ifndef CONTAINER_H
#define CONTAINER_H

#define CONT_BLKIO_MAX_ENTRIES  16
#define CONT_DEV_ACCESS_MAX_ENTRIES  16
#define CONT_DEVICES_MAX_ENTRIES  16
#define CONT_HOOK_POINT_MAX_ENTRIES  16
#define CONT_HOOK_POINT_ARGS_MAX_ENTRIES  16

#define CONT_USERNAME_LEN_MAX    32
#define CONT_CONTNAME_LEN_MAX    64

#define CONT_SCMP_ARGS_MAX          8
#define CONT_SCMP_ACTION_LEN_MAX    32

#define CONT_PROCESS_ARGS_MAX      8
#define CONT_PROCESS_ARGS_LEN_MAX  255

typedef enum 
{
   CONT_TOOL_LXC = 0,
   CONT_TOOL_DOCKER,
   CONT_TOOL_MAX,
} ContContainerToolType_t;

typedef enum 
{
   CONT_BLKIO_READ_BPS_DEV = 0,
   CONT_BLKIO_WRITE_BPS_DEV,
   CONT_BLKIO_READ_IOPS_DEV,
   CONT_BLKIO_WRITE_IOPS_DEV
} ContBlockIoType_t;

typedef struct
{
   ContBlockIoType_t type;
   long long major;
   long long minor;
   unsigned long long rate;
} ContBlockIo_t;

typedef struct
{
   int allow;
   long long major;
   long long minor;
   char devType[8];
   char accessType[8];
} ContDevAccessList_t;

typedef struct
{
   char type[8];
   char path[32];
   long long major;
   long long minor;
} ContDevicesList_t;

typedef struct
{
    char args[CONT_PROCESS_ARGS_MAX][CONT_PROCESS_ARGS_LEN_MAX+1];
} ContProcess_t;

typedef struct contResource_t
{
   unsigned long long cpu;
   long long realtimeRuntime;
   char cpus[64];
   unsigned long long memory;
   unsigned long long flash;
   ContBlockIo_t blkio[CONT_BLKIO_MAX_ENTRIES];
   ContDevAccessList_t devAccess[CONT_DEV_ACCESS_MAX_ENTRIES];
} ContResource_t;

typedef struct
{
   char path[1024];
   char args[CONT_HOOK_POINT_ARGS_MAX_ENTRIES][1024];
   unsigned long long timeout;
} ContHookPoint_t;

typedef struct
{
   ContHookPoint_t presetup[CONT_HOOK_POINT_MAX_ENTRIES];
   ContHookPoint_t prestart[CONT_HOOK_POINT_MAX_ENTRIES];
   ContHookPoint_t poststart[CONT_HOOK_POINT_MAX_ENTRIES];
   ContHookPoint_t poststop[CONT_HOOK_POINT_MAX_ENTRIES];
   int presetupHookEntries;
   int prestartHookEntries;
   int poststartHookEntries;
   int poststopHookEntries;
} ContHooks_t;

typedef struct
{
   unsigned int index;
   unsigned long long value;
   unsigned long long valueTwo;
   char op[CONT_SCMP_ACTION_LEN_MAX];
} ContSeccompArgs_t;

typedef struct
{
   char names[CONT_SCMP_ARGS_MAX][CONT_SCMP_ACTION_LEN_MAX];
   char action[CONT_SCMP_ACTION_LEN_MAX];
   ContSeccompArgs_t args[CONT_SCMP_ARGS_MAX];
} ContSeccompSyscall_t;

typedef struct
{
   char defaultAction[CONT_SCMP_ACTION_LEN_MAX];
   char architectures[CONT_SCMP_ARGS_MAX][CONT_SCMP_ACTION_LEN_MAX];
   ContSeccompSyscall_t syscalls[CONT_SCMP_ARGS_MAX];
} ContSeccomp_t;

typedef struct
{
   ContContainerToolType_t toolType;
   char containerName[CONT_CONTNAME_LEN_MAX];
   char path[1024];
   char appName[64];
   char library[1024];
   char username[CONT_USERNAME_LEN_MAX+1];
   int uid;
   int maxUser;
   int isPrivileged;
   char ntwkBridge[32];
   char ntwkDns[64];
   char ntwkMac[32];
   int fullFS;
   ContProcess_t process;
   ContResource_t resource;
   ContDevicesList_t devices[CONT_DEVICES_MAX_ENTRIES];
   ContHooks_t hooks;
   ContSeccomp_t scmp;
} ContainerSetup_t;

typedef enum
{
   CONT_MEDIA_TYPE_EXECUTABLE  = 0,   
   CONT_MEDIA_TYPE_TARBALL,
   CONT_MEDIA_TYPE_LAST,
} contMediaType;

typedef enum
{
   CONTRET_SUCCESS = 0,      /**<Success. */
   CONTRET_INTERNAL_ERROR ,  /**< Internal error. */
   CONTRET_SYSTEM_RESOURCE_EXCEEDED ,/**< System resources exceeded */
} ContRet;

typedef enum 
{
   CONT_PRIVILEGE_NONE = 0,
   CONT_PRIVILEGE_NORM,
   CONT_PRIVILEGE_HIGH,
   CONT_PRIVILEGE_GEN_NONE,
} ContPrivilegLevel_t;

int contCreateContainerDir(const char *path, ContContainerToolType_t tool,
                           contMediaType mediaType, char *unpackPath,
                           int unpackPathLen, int limitFlash, int fullFS);
ContRet contSetupContainer(const ContainerSetup_t *conf);
ContRet contRestoreContainer(const char *container_name, const char *path,
                             int isPrivileged, const char *username, 
                             int limitFlash, int fullFS);
int contStopContainer(const char *container_name);
int contStartContainer(const char *path, const char *container_name,
                       int needCMS, int isRoot, int needNetwork, int appCont);
int contDestroyContainer(const char *container_name, const char *path,
                         int limitFlash, int fullFS);
int contNetworkAddress(const char *container_name, char *addr, int addrLen);
int contGetContainerDataDir(const char *path, int limitFlash, int isEE,
                            char *dataDir, int dataDirLen, int fullFS);
int contGetEeContainerDuDir(const char *path, int limitFlash, int isEE,
                            char *duDir, int duDirLen);
int contCleanupContainerMountPoint(const char *container_name, const char *path,
                                   int limitFlash, int fullFS);
int contGetStateByName(const char *container_name);

/*****************************************************************************
*  FUNCTION:  contGetContainerNameByPid
*  DESCRIPTION:
*     Get the container name by PID. If pid is 0, the container name of the
*     caller will be returned.  If failed, containerName will be an
*     empty string.
*  PARAMETERS:
*     pid (IN) PID
*     containerName (OUT) container name buffer
*     containerNameSize (IN) container name buffer size
*  RETURNS:
*     void
******************************************************************************
*/
void contGetContainerNameByPid(unsigned int pid,
                               char *containerName, int containerNameSize);

#endif /* #ifndef CONTAINER_H */
