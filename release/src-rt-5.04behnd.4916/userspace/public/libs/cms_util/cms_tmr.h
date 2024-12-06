/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
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
:>
 *
 ************************************************************************/

#ifndef __CMS_TMR_H__
#define __CMS_TMR_H__


/*!\file cms_tmr.h
 * \brief Header file for the CMS Event Timer API.
 *  This is in the cms_util library.
 *
 */

/** Event handler type definition
 */
typedef void (*CmsEventHandler)(void*);


/** Max length (including NULL character) of an event timer name.
 *
 * When an event timer is created, the caller can give it a name
 * to help with debugging and lookup.  Name is optional.
 */
#define CMS_EVENT_TIMER_NAME_LENGTH  32


/** Initialize a timer handle.
 *
 * @param tmrHandle (OUT) On successful return, a handle to be used for
 *                        future handle operation is returned.
 *
 * @return CmsRet enum.
 */
CmsRet cmsTmr_init(void **tmrHandle);


/** Clean up a timer handle, including stopping and deleting all 
 *  unexpired timers and freeing the timer handle itself.
 *
 * @param tmrHandle (IN/OUT) Timer handle returned by cmsTmr_init().
 */
void cmsTmr_cleanup(void **tmrHandle);


/** Create a new event timer which will expire in the specified number of 
 *  milliseconds.
 *
 * Since lookups are done using a combination of the handler func and
 * context data, there must not be an existing timer event in the handle
 * with the same handler func and context data.  (We could allow 
 * multiple entries with the same func and ctxData, but we will have to
 * clarify what it means to cancel a timer, cancel all or cancel the
 * next timer.)
 *
 * @param tmrHandle (IN/OUT) Pointer to timer handle that was returned by cmsTmr_init().
 * @param func      (IN)     The handler func.
 * @param ctxData   (IN)     Optional data to be passed in with the handler func.
 * @param ms        (IN)     Timer expiration value in milliseconds.
 * @param name      (IN)     Optional name of this timer event.
 *
 * @return CmsRet enum.
 */   
CmsRet cmsTmr_set(void *tmrHandle, CmsEventHandler func, void *ctxData, UINT32 ms, const char *name);


/** Stop an event timer and delete it.
 *
 * The event timer is found by matching the callback func and ctxData.
 *
 * @param tmrHandle (IN/OUT) Pointer to the event timer handle;
 * @param func      (IN) The event handler.
 * @param *handle   (IN) Argument passed to the event handler.
 */   
void cmsTmr_cancel(void *tmrHandle, CmsEventHandler func, void *ctxData);


/** Pause an event timer without deleting it.
 *
 * Stops the event from triggering but keeps it in the queue to
 * be resumed later. Pausing an event that was already paused has no effect.
 * The event timer is found by matching the callback func and ctxData.
 *
 * @param tmrHandle (IN/OUT) Pointer to the event timer handle;
 * @param func      (IN) The event handler.
 * @param *handle   (IN) Argument passed to the event handler.
 *
 * @return CmsRet enum.
 */   
CmsRet cmsTmr_pause(void *handle, CmsEventHandler func, void *ctxData);


/** Resume an event timer after it was paused.
 *
 * Resumes a paused event with the remaining time from when it was paused.
 * Resuming an event that was not paused has no effect.
 * The event timer is found by matching the callback func and ctxData.
 *
 * @param tmrHandle (IN/OUT) Pointer to the event timer handle;
 * @param func      (IN) The event handler.
 * @param *handle   (IN) Argument passed to the event handler.
 *
 * @return CmsRet enum.
 */   
CmsRet cmsTmr_resume(void *handle, CmsEventHandler func, void *ctxData);


/** Replace current event timer with a new event timer which will 
 *  expire in the specified number of milliseconds.  The current event timer
 *  has shorter expire time than the current event in list.  In other words,
 *  this routine will do nothing if the new event timer has longer expire time
 *  than the one in the timer list.
 *
 * @param tmrHandle (IN/OUT) Pointer to timer handle that was returned by cmsTmr_init().
 * @param func      (IN)     The handler func.
 * @param ctxData   (IN)     Optional data to be passed in with the handler func.
 * @param ms        (IN)     Timer expiration value in milliseconds.
 * @param name      (IN)     Optional name of this timer event.
 *
 * @return CmsRet enum.
 */   
CmsRet cmsTmr_replaceIfSooner(void *tmrHandle, CmsEventHandler func, void *ctxData, UINT32 ms, const char *name);


/** Get the number of milliseconds until the next event is due to expire.
 *
 * @param tmrHandle (IN)  Pointer to timer handle that was returned by cmsTmr_init().
 * @param ms        (OUT) Number of milliseconds until the next event.
 *
 * @return CmsRet enum.  Specifically, CMSRET_SUCCESS if there is a next event.
 *         If there are no more events in the timer handle, CMSRET_NO_MORE_INSTANCES
 *         will be returned and the parameter ms is set to MAX_UINT32.
 */
CmsRet cmsTmr_getTimeToNextEvent(const void *tmrHandle, UINT32 *ms);


/** Get the number of timer events in the timer handle.
 *
 * @param tmrHandle (IN)  Pointer to timer handle that was returned by cmsTmr_init().
 *
 * @return The number of timer events in the given handle.
 */
UINT32 cmsTmr_getNumberOfEvents(const void *tmrHandle);


/** Execute all events which have expired.
 *
 * This function will call the handler func with the ctxData for all
 * timer events that have expired.  There may be 0, 1, 2, etc. handler
 * functions called by this function.  It is up to the caller of this
 * function to call this function at the appropriate time (using the
 * value of cmsTmr_getTimeToNextEvent() and cmsTmr_getEventCount() as a guide).
 *
 * Once an event is executed, it is deleted and freed.  
 *
 * @param tmrHandle (IN/OUT) Pointer to timer handle that was returned by cmsTmr_init().
 *
 */
void cmsTmr_executeExpiredEvents(void *tmrHandle);


/** Return time remaining for timer matching specified handler func and 
 *  context data (event).
 *  
 * @param tmrHandle (IN) Pointer to timer handle that was returned by cmsTmr_init().
 * @param func      (IN) The handler func.
 * @param ctxData   (IN) Optional data to be passed in with the handler func.
 * @param ms        (OUT) Number of milliseconds remaining for the event. 
 *                        Set to 0 if the event is expired or not found.
 *
 * @return CmsRet enum.  CMSRET_SUCCESS if the event is found. 
 *                       CMSRET_OBJECT_NOT_FOUND if a matching event is not found.
 */
CmsRet cmsTmr_getTimeRemaining(const void *handle, CmsEventHandler func, void *ctxData, UINT32 *ms);


/** Return true if the specified handler func and context data (event) is set.
 *  
 * @param tmrHandle (IN) Pointer to timer handle that was returned by cmsTmr_init().
 * @param func      (IN) The handler func.
 * @param ctxData   (IN) Optional data to be passed in with the handler func.
 *
 * @return TRUE if specified event is present, otherwise, FALSE.
 */   
UBOOL8 cmsTmr_isEventPresent(const void *tmrHandle, CmsEventHandler func, void *ctxData);


/** Use debug logging to dump out all timers in the timer handle.
 *  
 * @param tmrHandle (IN) Pointer to timer handle that was returned by cmsTmr_init().
 *
 */   
void cmsTmr_dumpEvents(const void *tmrHandle);


#endif  /* __CMS_TMR_H__ */
