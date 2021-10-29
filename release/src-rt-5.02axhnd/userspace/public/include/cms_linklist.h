/***********************************************************************
 *
 *  Copyright (c) 2010  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
:>
 * 
 ************************************************************************/

#ifndef __CMS_LINKLIST_H__
#define __CMS_LINKLIST_H__

typedef enum
{
   KEY_INT=0,
   KEY_STRING=1
} *pLIST_KEY_TYPE, LIST_KEY_TYPE;

typedef struct entryType
{
   struct entryType *next;
   LIST_KEY_TYPE keyType;       
   void *key;              /* key is used for sorting */
   void *data;
} *pENTRY_TYPE, ENTRY_TYPE;

typedef struct listType
{
   struct entryType *head;
   struct entryType *tail;
} *pLIST_TYPE, LIST_TYPE;

/** Add an entry to the front of the list
 *
 * @param pEntry (IN) Pointer to the entry to be added to list.  Caller
 *                    pre-allocated and initialize data and key.
 * @param pList (IN)  Pointer to the list.  The list's head and tail will be updated.
 * @return CmsRet enum.
 */
CmsRet addFront(pENTRY_TYPE pEntry, pLIST_TYPE pList);

/** Add an entry to the end of the list
 *
 * @param pEntry (IN) Pointer to the entry to be added to list.  Caller
 *                    pre-allocated and initialize data and key.
 * @param pList (IN)  Pointer to the list.  The list's head and tail will b e updated
 * @return CmsRet enum.
 */
CmsRet addEnd(pENTRY_TYPE pEntry, pLIST_TYPE pList);

/** Remove the first entry of the list, and return to caller.
 *
 * @param pList (IN)  Pointer to the list.  The list's head and tail will be updated
 * @return pointer to the entry.  Caller needs to free data, key and entryType.
 */
void *removeFront(pLIST_TYPE pList);

/** Remove the last entry of the list, and return to caller.
 *
 * @param pList (IN)  Pointer to the list.  The list's head and tail will be updated
 * @return pointer to the entry.  Caller needs to free data, key and entryType.
 */
void *removeEnd(pLIST_TYPE pList);

/** Remove the last entry of the list, and return to caller.
 *
 * @param pList (IN)  Pointer to the list.  The list's head and tail will be updated
 * @param key (IN)  Pointer to the key which is used to look for the entry.
 * @param type (IN)  Key type, integer or string
 * @return pointer to the entry.  Caller needs to free data, key and entryType.
 */
void *removeEntry(pLIST_TYPE pList, const void *key, LIST_KEY_TYPE type);

/** Find a particular entry in the list with matching key.
 *
 * @param pList (IN)  Pointer to the list.  The list's head and tail will be updated
 * @param key (IN)  Pointer to the key which is used to look for the entry.
 * @param type (IN)  Key type, integer or string
 * @param prev (OUT)  Pointer to store found entry's previous location.
 * @param ptr (OUT)  Pointer to store found entry's location. 
 * @return 1 if found, 0 if not found
 */
int findEntry(pLIST_TYPE pList, const void *key, LIST_KEY_TYPE type, pENTRY_TYPE *prevPtr, pENTRY_TYPE *ptr);

void *removeFoundEntry(pENTRY_TYPE ptr, pENTRY_TYPE prevPtr, pLIST_TYPE pList);
void *removeIntEntry(pLIST_TYPE pList, int key);
void *removeStrEntry(pLIST_TYPE pList, const char *key);
void sortIntList(pLIST_TYPE pList);
void printList(const pLIST_TYPE pList, LIST_KEY_TYPE type);
int findIntEntry(pLIST_TYPE pList, int key, pENTRY_TYPE *prevPtr, pENTRY_TYPE *ptr);

#endif /* __CMS_LINKLIST_H__ */
