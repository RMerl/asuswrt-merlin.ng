/***********************************************************************
 *
 *  Copyright (c) 2013  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2013:proprietary:standard
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
 * :>
 *
 * $Change: 111969 $
 ***********************************************************************/

/*
 * IEEE1905 Linked list
 */

#include <stdlib.h>
#include "ieee1905_linkedlist.h"

void i5LlItemAdd(void *parent, void *list, void *child)
{
  i5_ll_listitem *childitem = (i5_ll_listitem *)child;
  i5_ll_listitem *listitem = (i5_ll_listitem *)list;

  childitem->parent = (i5_ll_listitem *)parent;
  childitem->next = listitem->next;
  listitem->next = childitem;

  return;
}

void i5LlItemAppend(void *parent, void *list, void *child)
{
  i5_ll_listitem *childitem = (i5_ll_listitem *)child;
  i5_ll_listitem *listitem = (i5_ll_listitem *)list;

  while (listitem->next != NULL) {
    listitem = listitem->next;
  }

  childitem->next = NULL;
  listitem->next = childitem;
  childitem->parent = (i5_ll_listitem *)parent;

  return;
}

int i5LlItemRemove(void *list, void *child)
{
  i5_ll_listitem *childitem = (i5_ll_listitem *)child;
  i5_ll_listitem *listitem = (i5_ll_listitem *)list;

  while ((listitem != NULL) && (listitem->next != child)) {
    listitem = listitem->next;
  }
  if ((listitem != NULL) && (listitem->next == child)) {
    listitem->next = childitem->next;
    return 0;
  }
  return -1;
}

int i5LlItemFree(void *list, void *child)
{
  if (i5LlItemRemove(list, child) == 0) {
    free(child);
    return 0;
  }
  return -1;
}

void i5LlSearchItemPush(i5_ll_search_item_type *search_list, void *item)
{
  i5_ll_search_item_type *search_item;

  search_item = (i5_ll_search_item_type *)malloc(sizeof(i5_ll_search_item_type));
  search_item->item = (void *) item;
  i5LlItemAdd(NULL, search_list, search_item);
}

void *i5LlSearchItemPop(i5_ll_search_item_type *search_list)
{
  i5_ll_search_item_type *search_item;
  void *item = NULL;

  search_item = (i5_ll_search_item_type *)search_list->ll.next;
  if (search_item) {
    item = search_item->item;
    if (i5LlItemRemove(search_list, search_item) == 0) {
      free(search_item);
    }
  }
  return item;
}
