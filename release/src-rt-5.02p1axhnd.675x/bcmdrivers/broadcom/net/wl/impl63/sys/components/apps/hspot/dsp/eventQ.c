/*
 * Event queue utility.
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id:$
 */

#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <errno.h>
#include "trace.h"
#include "eventQ.h"

#define MAX_NAME_LENGTH		32

struct eventQ
{
	char name[MAX_NAME_LENGTH + 1];
	int queueDepth;
	size_t eventSize;
	mqd_t mq;
};

/* flush event queue */
static void flush(eventQT *eventq)
{
	char *data;

	data = malloc(eventq->eventSize);
	if (data == 0)
		return;

	while (eventQReceive(eventq, data) != -1)
	{}

	free(data);
}

/* create event queue */
static eventQT *create(char *name, int queueDepth, size_t eventSize)
{
	eventQT *eventq;
	struct mq_attr attr;

	eventq = malloc(sizeof(*eventq));
	if (eventq == 0)
		return 0;
	memset(eventq, 0, sizeof(*eventq));
	strncpy(eventq->name, name, MAX_NAME_LENGTH);
	eventq->queueDepth = queueDepth;
	eventq->eventSize = eventSize;

	/* event queue attributes */
	memset(&attr, 0, sizeof(attr));
	attr.mq_maxmsg = eventq->queueDepth;
	attr.mq_msgsize = eventq->eventSize;

	/* create event queue */
	eventq->mq = mq_open(eventq->name,
		O_RDWR | O_NONBLOCK | O_CREAT,
		S_IRWXU | S_IRWXG, &attr);
	if (eventq->mq == (mqd_t)-1) {
		TRACE(TRACE_ERROR, "failed to create event queue\n");
		perror("eventQCreate");
		free(eventq);
		return 0;
	}

	/* queue may not be empty if not a clean shutdown */
	flush(eventq);

	return eventq;
}

/* create event queue */
eventQT *eventQCreate(char *name, int queueDepth, size_t eventSize)
{
	eventQT *eventq;

	/* previous queue may be lingering around if not a clean shutdown */
	/* delete it first */
	eventq = create(name, queueDepth, eventSize);
	eventQDelete(eventq);

	return create(name, queueDepth, eventSize);
}

/* delete event queue */
void eventQDelete(eventQT *eventq)
{
	mq_close(eventq->mq);
	mq_unlink(eventq->name);
	free(eventq);
}

/* post to event queue */
int eventQSend(eventQT *eventq, char *event)
{
	return mq_send(eventq->mq, event, eventq->eventSize, 0);
}

/* retrieve from event queue */
int eventQReceive(eventQT *eventq, char *event)
{
	return mq_receive(eventq->mq, event, eventq->eventSize, 0);
}
