/***********************************************************************
 *
 * <:copyright-BRCM:2006:DUAL/GPL:standard
 * 
 *    Copyright (c) 2006 Broadcom 
 *    All Rights Reserved
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

#ifndef __PRCTL_H__
#define __PRCTL_H__

#include "cms.h"
/*
 * Technically, I should not include linux specific header here.
 * But I really don't want to redefine all the LINUX signals just
 * for the sake of OS independence....
 */
#include <signal.h> 


/*!\file prctl.h
 * \brief Header file for the CMS Process Control API.
 *
 *  Process control functions are included in the cms_util library.
 *  Process control is intended primary for smd, which is supposed
 *  to manage all process spawning, and for the unittests.  The RCL
 *  handler functions may also use them as we slowly transition from
 *  the 3.x implementation to the new CMS (4.x) implementation.
 *
 *  Typically, management applications should not need to call process
 *  control functions directly.  Instead, they should interact with the
 *  MDM and let the RCL handler functions spawn the appropriate daemons
 *  or spawn commands to do system actions.
 */

typedef enum
{
   SPAWN_AND_RETURN=1,  /**< spawn the process and return. */
   SPAWN_AND_WAIT  =2   /**< spawn the process and wait for it to exit.
                         *   The number of milliseconds to wait must be
                         *   specified in SpawnProcessInfo.timeout.
                         *   While waiting, the caller will block (but will
                         *   not spin in the CPU).
                         *   If the spawned process does not exit by the
                         *   timeout time, it will be terminated with a SIGKILL.
                         *   This mode will always collect the child before
                         *   returning.
                         */
} SpawnProcessMode;


typedef enum
{
   COLLECT_PID=1,  /**< Collect the specified pid, waiting indefinately. */
   COLLECT_PID_TIMEOUT=2, /**< Collect the specified pid, waiting for specified amount of time only. */
   COLLECT_ANY=3,  /**< Collect any spawned child process, waiting indefinately. */
   COLLECT_ANY_TIMEOUT=4 /**< Collect any spawned child process, waiting for specified amount of time only. */
} CollectProcessMode;


typedef enum
{
   PSTAT_RUNNING=1, /**< process is running. */
   PSTAT_EXITED=2,  /**< process has exited. */
   PSTAT_UNKNOWN=3,  /**< unable to determine process status due to error */
} SpawnedProcessStatus;



/** Info needed to spawn a process.
 *
 */
typedef struct 
{
   const char *exe;  /**< full path and name of executable */
   const char *args; /**< args to pass to executable */
   SpawnProcessMode spawnMode;  /**< Various spawning options. */
   UINT32 timeout;   /**< If SPAWN_AND_WAIT, how many milliseconds to wait before returning. */
   SINT32 stdinFd;   /**< Standard input of spawned process tied to this, -1 means /dev/null */
   SINT32 stdoutFd;  /**< Standard output of spawned process tied to this, -1 means /dev/null */
   SINT32 stderrFd;  /**< Standard error of spawned process tied to this, -1 means /dev/null */
   SINT32 serverFd;  /**< Special hack for smd dynamic launch service.
                      *   This server fd, if not -1, is tied to CMS_DYNAMIC_LAUNCH_SERVER_FD */
   SINT32 serverFd2;  /**< Special hack to support http and https connections concurrently. */
   SINT32 maxFd;      /**< Close all fd's up to and including this, but excluding
                       *   the stdinFd, stdoutFd, stderrFd, and serverFd */
   UBOOL8 inheritSigint;  /**< Check if the process needs to inherit sigint. */
   UBOOL8 setSched;  /**< set child's scheduling policy and priority */
   SINT32 schedPolicy; /**< scheduling policy (if setSched is 1).  See sched.h for values. */
   SINT32 schedPriority; /**< scheduling priority (if setSched is 1) */
   UINT32 cpuMask;     /**< allowed CPU mask (if non-zero) */
   const char *cpuGroupName; /**< put process in this cpu group (if not NULL) */
} SpawnProcessInfo;



/** Info needed to collect (wait for termination of) a process.
 *
 */
typedef struct
{
   CollectProcessMode collectMode; /**< Various collect options. */
   SINT32 pid; /**< If collecting a specific process, the pid of the process to collect. */
   UINT32 timeout; /**< If using a collect mode with timeout, the number of milliseconds to wait. */
} CollectProcessInfo;


/** Info about a spawned process, returned by spawnProcess and collectProcess.
 *
 */
typedef struct
{
   SINT32  pid;          /**< pid of the process that this structure is reporting on. */
   SpawnedProcessStatus status; /**< current status of process. */
   SINT32  signalNumber; /**< If process exited due to Linux signal, the signal number is stored here. */
   SINT32  exitCode;     /**< If process exited and not due to Linux signal, the exit code is stored here. */
} SpawnedProcessInfo;


/** Spawn a process, possibly also wait for its completion.
 *
 * The child process's actions for all signals are set to default action.
 *
 * @param collectInfo (IN) parameters affecting which command to spawn.
 * @param procInfo   (OUT) Info about the spawned process.
 *
 * @return CmsRet enum.
 */
extern CmsRet prctl_spawnProcess(const SpawnProcessInfo *spawnInfo, SpawnedProcessInfo *procInfo);



/** Wait for a specific process or any process to exit, with a possible timeout.
 *
 * @param collectInfo (IN) parameters affecting which process to collect and
 *                         possible timeout for getting a process.
 * @param procInfo   (OUT) Info about the collected process, if any.
 *
 * @return CmsRet enum.  Specifically, CMSRET_SUCCESS means a process was
 *         collected.  CMSRET_TIMED_OUT means a process was not collected
 *         during the timeout period (timeout period could be 0).
 *         CMSRET_INVALID_ARGUMENTS means other error.
 */
extern CmsRet prctl_collectProcess(const CollectProcessInfo *collectInfo, SpawnedProcessInfo *procInfo);

extern CmsRet prctl_terminateProcessGracefully(SINT32 pid);

extern CmsRet prctl_terminateProcessForcefully(SINT32 pid);

extern CmsRet prctl_signalProcess(SINT32 pid, SINT32 sig);


/** Set a process's scheduler params
 *
 * @param pid      (IN) pid to set (if 0, will affect calling process)
 * @param policy   (IN) scheduling policy, same as sched_setscheduler
 * @param priority (IN) priority, same as sched_setscheduler
 *
 * @return CmsRet enum.
 */
extern CmsRet prctl_setScheduler(SINT32 pid, SINT32 policy, SINT32 priority);


/** Set a process's CPU mask
 *
 * @param pid      (IN) pid to set (if 0, will affect calling process)
 * @param cpuMask  (IN) bitmask of CPU's which this pid is allowed to run on
 *
 * @return CmsRet enum.
 */
extern CmsRet prctl_setCpuMask(SINT32 pid, UINT32 cpuMask);


/** Set a process's cgroups membership
 *
 * @param pid       (IN) pid to set (if 0, will affect calling process)
 * @param groupBase (IN) path to the base of the cgroups tree
 * @param groupName (IN) name of the group
 *
 * @return CmsRet enum.
 */
extern CmsRet prctl_setCgroup(SINT32 pid, const char *groupBase, const char *groupName);


/** Execute the given command and wait indefinately for it to exit.
 *
 * Ported over from 3.x bcmSystemMute() to ease porting effort.  This function can be
 * used, but please consider using prctl_spawnProcess instead.  The only place
 * where you really need to use this function is if your command redirects its output
 * to another file, as in iptables -L > /tmp/iptables_out.
 * 
 * @param command (IN) The command line to execute.
 *
 * @return 0 on success, all other values indicate error.
 */
int prctl_runCommandInShellBlocking(char *command);


/** Execute the given command and wait for a limited (hard-coded) amount of time
 *  for it to exit.
 *
 * Ported over from 3.x bcmSystemMute() to ease porting effort.  This function can be
 * used, but please consider using prctl_spawnProcess instead.  The only place
 * where you really need to use this function is if your command redirects its output
 * to another file, as in iptables -L > /tmp/iptables_out.
 * 
 * @param command (IN) The command line to execute.
 *
 * @return 0 on success, all other values indicate error.
 */
int prctl_runCommandInShellWithTimeout(char *command);


/** Given a process name, return the pid.
 *
 * If there are multiple processes with the same name, there is no guarantee
 * on which pid will be returned.
 *
 * @param name (IN) the name of the process
 *
 * @return on success, the pid, otherwise, CMS_INVALID_PID
 */
int prctl_getPidByName(const char *name);

#define CONFIG_CMS_UTIL_HAS_GETPIDBYNAME   1


/** Given a pid, fill the given buffer with the process name.
 *
 * @param pid     (IN) the pid
 * @param nameBuf (IN/OUT) the buffer which will hold the process name.  On
 *                    return, the buffer is guaranteed to be NULL terminated.
 *                    If the buffer is too short to hold the full name, the
 *                    name will be truncated.
 * @param nameBufLen (IN)  the length of the nameBuf.
 *
 * @return 0 on success, -1 on error.
 */
int prctl_getNameByPid(int pid, char *nameBuf, int nameBufLen);


#endif /* __PRCTL_H__ */
