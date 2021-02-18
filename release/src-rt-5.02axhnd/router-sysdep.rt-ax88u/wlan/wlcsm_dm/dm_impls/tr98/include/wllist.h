/*
<:copyright-BRCM:2016:proprietary:standard 

   Copyright (c) 2016 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
*/
#ifndef __WL_LIST__
#define __WL_LIST__


#define INIT_LIST_HEAD(head) 	 ({ \
	  head=NULL;    })

#define list_add(new, head, TYPE) ({    \
    new->next = (TYPE *)head;	\
    head = (TYPE *)new;  })

#define list_get(head, TYPE, MEMBER, entry, value, cmp)  ({   \
              for ( entry =head; entry != NULL; entry=entry->next) { \
                 if ( !cmp(entry->MEMBER, value) ) {   \
                    break; \
                 }  \
              }                     })

#define list_get_next(priv, head, entry)  ({       \
    if (priv == NULL )       \
        entry = head;         \
    else                     \
        entry = priv->next;   })

#define list_for_each(pos, head) \
	for (pos = head; pos != NULL; pos = pos->next)
	

#define list_del_all(head, TYPE)  ({   \
	      TYPE *_pos;  \
              for ( _pos =head, head=NULL; _pos != NULL; head=_pos, _pos=_pos->next) { \
                 if (  head != NULL ) {   \
                     free(head);  \
                 }  \
              }  \
              if (  head != NULL ) {   \
                 free(head);  \
              }  \
              head = NULL;          })



#define list_del(head, TYPE, MEMBER, value, cmp)  ({   \
	      TYPE *_pos, *_priv_pos;  \
              char _found = 0; \
              for ( _pos =head, _priv_pos=NULL; _pos != NULL; _priv_pos=_pos, _pos=_pos->next) { \
                 if ( !cmp(_pos->MEMBER, value) ) {   \
                    _found = 1;   \
                    break; \
                 }  \
              }  \
              if (  _found  ) {   \
                 if ( _priv_pos == NULL ) \
                    head = head->next; \
                 else  \
                    _priv_pos->next = _pos->next; \
                 free(_pos);                   \
              }                   })

#define list_remove(head, TYPE, entry)  ({   \
	      TYPE *_pos, *_priv_pos;  \
              char _found = 0; \
              for ( _pos =head, _priv_pos=NULL; _pos != NULL; _priv_pos=_pos, _pos=_pos->next) { \
                 if ( _pos == entry ) {   \
                    _found = 1;   \
                    break; \
                 }  \
              }  \
              if (  _found  ) {   \
                 if ( _priv_pos == NULL ) \
                    head = head->next; \
                 else  \
                    _priv_pos->next = _pos->next; \
                 free(_pos);                   \
              }                   })

 #endif

