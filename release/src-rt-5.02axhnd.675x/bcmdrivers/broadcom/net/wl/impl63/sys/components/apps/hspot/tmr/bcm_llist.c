/*
 * Linked list manipulation routines
 * See discussion below for more info
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
 * $Id: bcm_llist.c,v 1.5 2010-12-21 23:05:53 $
 */

#include <stdio.h>
#include <typedefs.h>
#include <bcm_llist.h>
#include <debug.h>

/* Discussion
 * The functions herein provide a means of adding and deleting a list member
 * from a given singly linked list.
 * You must create your list with the next pointer as the first element in the
 * list element structure.
 */

#ifdef TARGETOS_symbian
extern int SymbianOslPrintf(const char *format, ...);
#define printf SymbianOslPrintf
#endif // endif

typedef struct list {
	struct list * next;
} list_t;

/* Adds member pnew to list *head if possible */
int bcm_llist_add_member(void *pplhd, void *plmember)
{
	list_t **head = (list_t **)pplhd;
	list_t *pnew = (list_t *)plmember;
	char *funstr = "bcm_llist_add_member";

	if (pnew == NULL) {
		PRINT(("%s: can't add NULL member\n", funstr));
		return -1;
	}
	pnew->next = *head;
	*head = pnew;
	return 0;
}

/* Removes member pdel from list *head if possible */
int bcm_llist_del_member(void *pplhd, void *plmember)
{
	list_t **head = (list_t **)pplhd;
	list_t *pdel = (list_t *)plmember;
	list_t *pprev, *plist;
	char *funstr = "bcm_llist_del_member";

	PRINT_TRACE(("%s: Requested to delete member %p from list %p\n",
		funstr, pdel, *head));

	if (*head == NULL) {
		PRINT_TRACE(("%s: list empty\n", funstr));
		return -1;
	}
	if (pdel == NULL) {
		PRINT(("%s: can't delete NULL member\n", funstr));
		return -1;
	}

	for (plist = *head, pprev = NULL; plist; ) {
		if (plist == pdel) {
			/* first entry? */
			if (pprev == NULL)
				*head = plist->next;
			else
				pprev->next = plist->next;

			return 0;

		}
		/* advancd */
		pprev = plist;
		plist = plist->next;
	}

	/* not found */
	PRINT_TRACE(("%s: member %p not found in list %p\n",
		funstr, pdel, *head));
	return -1;
}

/* Removes member containing "arg" from list *head if possible,
 * If successful returns pointer to that member, otherwise NULL
 */
void * bcm_llist_del_membercmp(void *pplhd, void *arg, bool (*pcmp)(void *, void *))
{
	list_t **head = (list_t **)pplhd;
	list_t *pprev, *plist;
	char *funstr = "bcm_llist_del_member";

	PRINT_TRACE(("%s: Requested to delete member with %p from list %p\n",
		funstr, arg, *head));

	if (*head == NULL) {
		PRINT_TRACE(("%s: list empty\n", funstr));
		return NULL;
	}
	if (pcmp == NULL) {
		PRINT(("%s: comparison fun NULL, bailing \n", funstr));
		return NULL;
	}

	for (plist = *head, pprev = NULL; plist; ) {
		if ((*pcmp)(plist, arg)) {
			/* first entry? */
			if (pprev == NULL)
				*head = plist->next;
			else
				pprev->next = plist->next;

			return plist;

		}
		/* advancd */
		pprev = plist;
		plist = plist->next;
	}

	/* not found */
	PRINT_TRACE(("%s: member %p not found in list %p\n",
		funstr, arg, *head));
	return NULL;
}
