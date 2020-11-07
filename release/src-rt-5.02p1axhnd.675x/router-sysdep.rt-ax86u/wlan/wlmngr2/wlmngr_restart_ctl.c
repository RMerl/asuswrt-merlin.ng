/*
* <:copyright-BRCM:2011:proprietary:standard
*
*    Copyright (c) 2011 Broadcom
*    All Rights Reserved
*
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
*
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
*
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
:>
*/
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <wlmngr.h>
#include <wlcsm_lib_api.h>
#include <signal.h>
#include <bcmnvram.h>

static void wlmngr_mark_radio(int index);

#define MAX_RESTATING_ALLOWED (16) /*it shouldn't hit 16, max should be number of radio */
#define RESTARING_QUIET_PERIOD (3) /*Period in seconds without restarting request coming */
#define RST_RD (g_restaring_queue.rd)
#define RST_WR (g_restaring_queue.wr) 

#define RST_FILLING (g_restaring_queue.filling) /* to indicate if there is request coming in quiet period */

#define RST_MARK (g_restaring_queue.radio_bits) /* mark restarting readio  */
#define RST_MARK_RADIO(index) wlmngr_mark_radio(index)
#define RST_IS_RADIO_MARKED(index) (g_restaring_queue.radio_bits & (1<<index))

#define RST_STATUS (g_restaring_queue.status)
#define RST_DIRECTION (g_restaring_queue.direction)
#define RST_ENQUEUE(request) g_restaring_queue.queue[g_restaring_queue.wr++]=(request)
#define RST_DEQUEUE() g_restaring_queue.queue[g_restaring_queue.rd++]

#ifdef WLCSM_DEBUG
#define RST_QUEUE_LOCK() do {pthread_mutex_lock(&g_restaring_queue.queue_lock); fprintf(stderr, "%s:%d: rst queue lock..\n",__FUNCTION__,__LINE__);} while(0)
#define RST_QUEUE_UNLOCK() do {pthread_mutex_unlock(&g_restaring_queue.queue_lock); fprintf(stderr, "%s:%d:	rst queue unlock\n",__FUNCTION__,__LINE__);} while(0)
#else
#define RST_QUEUE_LOCK() pthread_mutex_lock(&g_restaring_queue.queue_lock)
#define RST_QUEUE_UNLOCK() pthread_mutex_unlock(&g_restaring_queue.queue_lock)
#endif

#define RST_QUEUE_EMPTY() (g_restaring_queue.rd==g_restaring_queue.wr)
#define RST_QUEUE_FULL() ((g_restaring_queue.rd==(g_restaring_queue.wr+1)) ||(g_restaring_queue.wr==(MAX_RESTATING_ALLOWED-1) && g_restaring_queue.rd==0))
#define RST_QUEUE_WRAP_RD() do { if(g_restaring_queue.rd==MAX_RESTATING_ALLOWED) g_restaring_queue.rd=0; } while(0)
#define RST_QUEUE_WRAP_WR() do { if(g_restaring_queue.wr==MAX_RESTATING_ALLOWED) g_restaring_queue.wr=0; } while(0)

static void _wlmngr_restart_process(void *arg);

#define RST_QUEUE_CHECK() do { \
    RST_QUEUE_LOCK(); \
    if(!RST_QUEUE_EMPTY()) goto radio_restart; \
    RST_QUEUE_UNLOCK();} while(0)

int g_pid=0;

enum  restarting_status { ENU_RST_READY, ENU_RST_TOBE_SCHEDULED,ENU_RST_ONGOING };

struct restarting_queue {
    enum restarting_status status;
    unsigned char filling; /* indicates there are request keep coming */
    unsigned char wr;
    unsigned char rd;
    unsigned char radio_bits; /* bitmap to indidcate which radio restarts */
    unsigned char direction; /*direction access no need lock,once set, never change*/
    unsigned int queue[MAX_RESTATING_ALLOWED];
    pthread_mutex_t queue_lock;
} g_restaring_queue;

static void wlmngr_mark_radio(int index) {
#if defined(BUILD_BCM_WLAN_DPDCTL)
    char key[32];
    char *val;

    snprintf(key, sizeof(key), "wl%d_dpd", index);
    val = nvram_get(key);

    /* If power_down nvram is present and is set to power down,
       do not mark the interface as available
     */
    if ( val && atoi(val) == 1) {
        fprintf(stderr, "RESTART_CTRL Skip wl%d as it is powered down\n", index);
    } else
#endif /* BUILD_BCM_WLAN_DPDCTL */
    {
        g_restaring_queue.radio_bits |= (1<<index);
    }

    return;
}

static int _enqueue(int i) {
    if(RST_QUEUE_FULL()) {
        return -1;
    } else {
        RST_ENQUEUE(i);
        RST_QUEUE_WRAP_WR();
    }
    return 0;
}

static int _dequeue(unsigned int *value) {
    if(RST_QUEUE_EMPTY()) {
        return -1;
    }
    *value=RST_DEQUEUE();
    RST_QUEUE_WRAP_RD();
    return 0;
}

static void _wlmngr_post_start_handler(void *arg) {
    int i,save_dm=0;

    if(!nvram_get("_default_restored_")) {
        wlcsm_nvram_set("_default_restored_","y");
		save_dm=1;
    }

    if(RST_DIRECTION==WL_SYNC_TO_DM || save_dm) {
        for(i=0; i<WL_MAX_NUM_RADIO; i++) {
            if(RST_IS_RADIO_MARKED(i))
                wlcsm_dm_save_config(i+1,0,!arg?wlmngr_thread_lock:NULL);
        }
    }
    wlmngr_handle_bridge_setup();
    _wlmngr_bridge_ipalias();
    wlmngr_startServices();
    for(i=0; i<WL_MAX_NUM_RADIO; i++) {
        if(RST_IS_RADIO_MARKED(i)) {
            wlmngr_post_setup(i);
            wlmngr_getCurrentChannel(i);
            wlmngr_update_stalist(i);


        }
    }

}


void _wlmngr_restart_thread(void *arg) {
    int queue_empty=1;
    RST_QUEUE_LOCK();
    /* if it is first restart request, it will be wait for some
     * time to allow following request coming to prevent more
     * unnecessary restarting */
    if(RST_STATUS==ENU_RST_TOBE_SCHEDULED && g_wlmngr_ready_for_event)  {
        if(RST_FILLING) {
            RST_FILLING=0;
            RST_QUEUE_UNLOCK();
            /* wait another 3 seconds to see if there is still request coming
             * in as there are requests coming in during the last 3 seconds*/
            alarm(RESTARING_QUIET_PERIOD);
            return;
        }
        RST_QUEUE_UNLOCK();
    } else {
        RST_QUEUE_UNLOCK();
    }

    brcm_get_lock("wps",200);
    do {
        _wlmngr_restart_process(NULL);
        RST_QUEUE_LOCK();
        queue_empty=RST_QUEUE_EMPTY();
        if(queue_empty) {
            RST_STATUS=ENU_RST_READY;
            _wlmngr_post_start_handler(arg);
        }
        RST_QUEUE_UNLOCK();
    } while(!queue_empty);
    brcm_release_lock("wps");
    wlcsm_nvram_unset("map_reonboard");
    wlcsm_nvram_unset("_wlrestart_");
    WLMNGR_RESTART_UNLOCK();
    wlmngr_postStart_Service();
#ifdef BCM_WBD
    /* have wbd start out of restart lock as wbd may init all wifi restart */
    wlmngr_start_wbd();
#endif
    fprintf(stderr, "--WL RESTART DONE--\n");
}


void _wlmngr_restarting_signal_handler(int sig, siginfo_t* info, void* vp) {
    pthread_t _restart_thread;
    if (info->si_pid != g_pid && info->si_pid!=0) {
        fprintf(stderr, "%s:%d:	 signal from different pid:g_pid:%d,%d\n",__FUNCTION__,__LINE__,g_pid,info->si_pid);
        return;
    }

    if(pthread_create(&_restart_thread,NULL,(void *)&_wlmngr_restart_thread,NULL)) {
        fprintf(stderr, "%s:%d:	could not restart wireless from separated thread\n",__FUNCTION__,__LINE__);
    } else
        pthread_detach(_restart_thread);
}



static void _wlmngr_restart_process(void *arg) {

    unsigned int radio_idx,idx=0;
    int from,ret,i,start_idx,direction;
    int restarting_all=0;


    /* get wps_lock to handle on when WPS is change config */
    wlmngr_stopServices();

    /* if start a thread and queue is empty, then return */
    RST_QUEUE_LOCK();

    if(!arg && RST_QUEUE_EMPTY()) {
        RST_QUEUE_UNLOCK();
        return;
    }
radio_restart:
    /* if it is starting directly,use arg as index */
    if(arg) {
        radio_idx= (*(unsigned int *)(arg));
    }
    else if(_dequeue((unsigned int *)&radio_idx)) {
        RST_QUEUE_UNLOCK();
        return;
    }
    RST_STATUS=ENU_RST_ONGOING;

    from=WLCSM_MNGR_CMD_GET_SOURCE(radio_idx);
    direction = (WLCSM_MNGR_CMD_GET_SOURCE(radio_idx)>=WLCSM_MNGR_RESTART_FROM_MDM)?WL_SYNC_FROM_DM:WL_SYNC_TO_DM;
    RST_QUEUE_UNLOCK();

    ret=from;

    idx=WLCSM_MNGR_CMD_GET_IDX(radio_idx);
    if(from==WLCSM_MNGR_RESTART_NVRAM || (from==WLCSM_MNGR_RESTART_MDM_ALL))
        restarting_all=1;
    else
        RST_MARK_RADIO(idx);

    if(direction==WL_SYNC_FROM_DM)  {

        /*changes are in DM, we need to load DM settings and apply it*/
        if(restarting_all)
            WLCSM_DM_SELECT(SYS_DM_NAME,0,wlmngr_thread_lock,ret);
        else
            WLCSM_DM_SELECT(SYS_DM_NAME,idx+1,wlmngr_thread_lock,ret);

        if(ret) {
            g_dm_loaded=ret;
            fprintf(stderr," DM init error, thus,we will return from here, do not need to continue as DM has problem   \r\n" );
            goto restart_done;
        } else if(!g_dm_loaded) {
            wlmngr_hspot_default();
            g_dm_loaded=1;
        }
        RST_QUEUE_CHECK(); /*dequeue restarting queues in case there is any */
    }

    wlcsm_dm_get_bridge_info_adjust(direction,g_ifbuf);

    if(restarting_all) {
        /* when sync to DM, we already have all the information here for all radio, following request will not needed
         * thus, here dequeue all request */
        RST_QUEUE_LOCK();
        while(direction==WL_SYNC_TO_DM && !RST_QUEUE_EMPTY()) {
            _dequeue((unsigned int *)&start_idx);
        }
        RST_QUEUE_UNLOCK();
        
        for(i=0; i<WL_WIFI_RADIO_NUMBER; i++) {
            RST_MARK_RADIO(i);
        }
    }

    act_wl_cnt = 0;
    for (i=0; i<WL_WIFI_RADIO_NUMBER; i++) {
        act_wl_cnt += (WL_RADIO_WLENBL(i) == TRUE) ? 1 : 0;
        if(RST_IS_RADIO_MARKED(i)) {
            wlmngr_pre_setup(i);
        }
    }

    for (i=0; i<WL_WIFI_RADIO_NUMBER; i++) {
        if(RST_IS_RADIO_MARKED(i)) {
            wlmngr_get_thread_lock();
            wlmmgr_vars_adjust(i,direction);
            wlmngr_write_wl_nvram(i);
            wlmngr_nvram_adjust(i,direction);
            wlmngr_release_thread_lock();
            wlmngr_setup(i);
        }
    }

    RST_QUEUE_CHECK(); /*dequeue restarting queues in case there is any */
restart_done:
    return;

}

int wlmngr_handle_wlcsm_cmd_halt_restart(t_WLCSM_MNGR_VARHDR  *hdr, char *interface,char *action) {
    int queue_status; ;
    do {
        RST_QUEUE_LOCK();
        queue_status=RST_STATUS;
        RST_QUEUE_UNLOCK();
        usleep(1000);
    } while(queue_status!=ENU_RST_READY);
    g_wifi_autorestart_enable=0;
    return 0;
}

int wlmngr_handle_wlcsm_cmd_resume_restart(t_WLCSM_MNGR_VARHDR  *hdr, char *interface,char *action) {

    /* if it is not halted before, then resume will do nothing */
    if(g_wifi_autorestart_enable) return 0;
    g_wifi_autorestart_enable=1;
    RST_QUEUE_LOCK();
    if(!RST_QUEUE_EMPTY()) {
        RST_STATUS=ENU_RST_ONGOING; /* do not need wait for  seconds */
        RST_MARK=0;
        RST_FILLING=0;
        alarm(1);
    }
    RST_QUEUE_UNLOCK();
    return 0;
}

int restarting_enqueue(unsigned int request) {
    int i=0,ret=0;
    int direction = (WLCSM_MNGR_CMD_GET_SOURCE(request)>WLCSM_MNGR_RESTART_FROM_MDM)?WL_SYNC_FROM_DM:WL_SYNC_TO_DM;
    RST_QUEUE_LOCK();
#ifdef WLCSM_DEBUG
    if(RST_STATUS == ENU_RST_READY  && !RST_QUEUE_EMPTY()) {
        fprintf(stderr, "ATTENTION:%s:%d: queue is not empty but status is ready, but How could this happen???\n",__FUNCTION__,__LINE__);
    } else if(RST_STATUS != ENU_RST_READY  && RST_QUEUE_EMPTY()) {
        fprintf(stderr, "%s:%d:	is handling the last restart request and no more request in queue \n",__FUNCTION__,__LINE__);
    }
#endif
    /* when currently wifi is not restarting and start is enabled, we will
     * start the thread to read the restarting queue and do the real work */
    if(RST_STATUS == ENU_RST_READY ) {
        wlcsm_nvram_set("_wlrestart_","1");
        WLMNGR_RESTART_LOCK();
        _enqueue(request);
        RST_DIRECTION=direction;
        RST_STATUS=ENU_RST_TOBE_SCHEDULED;
        RST_MARK=0;
        RST_FILLING=0;

        if(g_wifi_autorestart_enable) {
            fprintf(stderr, "%s:%d:	to start restarting thread when status is ready. \n",__FUNCTION__,__LINE__);
            alarm(RESTARING_QUIET_PERIOD);
        }
    } else {
        if(RST_DIRECTION == direction ) {
            for(i=RST_RD; i<RST_WR; i++) {
                if(request==g_restaring_queue.queue[i])
                    break;
            }
            RST_FILLING=1;
            /*if no same request in the queue, enqueue it */
            if(i==g_restaring_queue.wr)
                _enqueue(request);
            else
                WLCSM_TRACE(WLCSM_TRACE_DBG, "Same request already in queue,no need to add it in queue\n");

        } else {
            fprintf(stderr, "!!!restart request get ignoredL%08x as current queue direction is:%d\n",RST_DIRECTION,request);
            ret=-1;
        }
    }
    RST_QUEUE_UNLOCK();
    return ret;

}

#ifdef WLCSM_DEBUG
int wlmngr_get_restarting_status(void) {
    int status;
    RST_QUEUE_LOCK();
    status=RST_STATUS;
    RST_QUEUE_UNLOCK();
    return status;
}
#endif

/* creat restarting ctrl thread */
int restarting_thread_init(void) {
    struct sigaction action, old_action;
    int radio_idx=0;
    pthread_mutex_init(&g_restaring_queue.queue_lock,NULL);
    g_pid=getpid();
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_sigaction = _wlmngr_restarting_signal_handler;
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART|SA_SIGINFO; /* restart syscalls if possible */
    if (sigaction(SIGALRM, &action, &old_action) < 0) {
        perror("Signal error");
        return -1;
    }
    WLCSM_MNGR_CMD_SET_CMD(radio_idx, WLCSM_MNGR_CMD_RESTART);
    WLCSM_MNGR_CMD_SET_SOURCE(radio_idx,WLCSM_MNGR_RESTART_MDM_ALL);
    restarting_enqueue(radio_idx);
    return 0;
}

int  wlmngr_handle_wlcsm_cmd_restart(t_WLCSM_MNGR_VARHDR *hdr,char *varName,char *varValue)
{
    int wait=WLCSM_MNGR_CMD_GET_WAIT(hdr->radio_idx);
    int queue_status; ;
    WLCSM_SET_TRACE("wlmngr");
    restarting_enqueue(hdr->radio_idx);
    if(wait) {
        do {
            RST_QUEUE_LOCK();
            queue_status=RST_STATUS;
            RST_QUEUE_UNLOCK();
            sleep(1);
        } while(queue_status!=ENU_RST_READY);
    }
    return 0;
}
