/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2016-2024  CommScope, Inc
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \file dllist.c
 *
 * Implements a doubly linked list library
 *
 */

#include "common_defs.h"
#include "dllist.h"

// Uncomment the following define to perform checks on the integrity of the linked list, when inserting/removing items
//#define PERFORM_INTEGRITY_CHECKS

/*********************************************************************//**
**
** DLLIST_Init
**
** Initialises a doubly linked list
**
** \param   list - pointer to the list header structure
**
** \return  None
**
**************************************************************************/
void DLLIST_Init(double_linked_list_t *list)
{
    list->head = NULL;
    list->tail = NULL;
}

/*********************************************************************//**
**
** DLLIST_IsItemInList
**
** Returns true if an item is in the given list
** NOTE: This function is only used for debug checking purposes
**
** \param   list - pointer to the list header structure
** \param   item_to_move - pointer to the item
**
** NOTE: The item to move must contain the double_link_t structure at the start of itself
**
** \return  true if the item is in the list, false otherwise
**
**************************************************************************/
bool DLLIST_IsItemInList(double_linked_list_t *list, double_link_t *item_to_check)
{
    double_link_t *item;

    item = list->head;
    while (item != NULL)
    {
        if (item==item_to_check)
        {
            return true;
        }

        // move to next item in linked list
        item = item->next;
    }

    // Got to end of list and item not found
    return false;
}

/*********************************************************************//**
**
** DLLIST_LinkToHead
**
** Add an item to the head of a doubly linked list
** NOTE: The item to add must contain the double_link_t structure at the start of itself
**
** \param   list - pointer to the list header structure to which the item is to be added
** \param   item_to_add - pointer to the item to add.
**
** \return  None
**
**************************************************************************/
void DLLIST_LinkToHead(double_linked_list_t *list, void *item_to_add)
{
    double_link_t *item = (double_link_t *)item_to_add;
    double_link_t *head_item;

#ifdef PERFORM_INTEGRITY_CHECKS
    USP_ASSERT(DLLIST_IsItemInList(list, item)==false);
#endif

    if (list->head == NULL)
    {
        USP_ASSERT(list->tail==NULL);
        list->tail = item;   // Addition to an empty list
        item->next = NULL;
    }
    else
    {
        head_item = list->head;
        head_item->prev = item; // Normal addition to head
        item->next = head_item;
    }

    list->head = item;
    item->prev = NULL;
}

/*********************************************************************//**
**
** DLLIST_LinkToTail
**
** Add an item to the tail of a doubly linked list
** NOTE: The item to add must contain the double_link_t structure at the start of itself
**
** \param   list - pointer to the list header structure to which the item is to be added
** \param   item_to_add - pointer to the item to add.
**
** \return  None
**
**************************************************************************/
void DLLIST_LinkToTail(double_linked_list_t *list, void *item_to_add)
{
    double_link_t *item = (double_link_t *)item_to_add;
    double_link_t *tail_item;

#ifdef PERFORM_INTEGRITY_CHECKS
    USP_ASSERT(DLLIST_IsItemInList(list, item)==false);
#endif

    if (list->tail == NULL)
    {
        USP_ASSERT(list->head==NULL);
        list->head = item;   // Addition to an empty list
        item->prev = NULL;
    }
    else
    {
        tail_item = list->tail;
        tail_item->next = item; // Normal addition to tail
        item->prev = tail_item;
    }

    list->tail = item;
    item->next = NULL;
}

/*********************************************************************//**
**
** DLLIST_Unlink
**
** Removes an item from a doubly linked list
** (the item can be anywhere in the linked list, the head and tail pointers will be updated correctly for all cases)
** NOTE: this function does not free the item from memory, just removes it from the list
** NOTE: The item to remove must contain the double_link_t structure at the start of itself
**
** \param   list - pointer to the list header structure to which the item is to be removed
** \param   item_to_remove - pointer to the item to remove
**
** \return  None
**
**************************************************************************/
void DLLIST_Unlink(double_linked_list_t *list, void *item_to_remove)
{
    double_link_t *item = (double_link_t *)item_to_remove;

#ifdef PERFORM_INTEGRITY_CHECKS
    USP_ASSERT(DLLIST_IsItemInList(list, item)==true);
#endif

    if (item->prev == NULL)
    {
        list->head = item->next;   // Removal at head
    }
    else
    {
        (item->prev)->next = item->next; // Normal removal at middle/tail
    }

    if (item->next == NULL)
    {
        list->tail = item->prev;   // Removal at tail
    }
    else
    {
        (item->next)->prev = item->prev; // Normal removal at head/middle
    }

    // Ensure that the item does not contain references to the items that did surround it in the doubly linked list
    item->next = NULL;
    item->prev = NULL;
}


/*********************************************************************//**
**
** DLLIST_InsertLinkBefore
**
** Inserts an element before the specified element in the list
** NOTE: This function cannot be called to insert at the end of a list
** NOTE: Both the insert point item and the item to add must contain the double_link_t structure at the start of itself
**
** \param   insert_point - pointer to the item, which we want to be after the item we're about to insert
** \param   list - pointer to the list header structure to which the item is to be added
** \param   item_to_remove - pointer to the item to add
**
** \return  None
**
**************************************************************************/
void DLLIST_InsertLinkBefore(void *insert_point, double_linked_list_t *list, void *item_to_add)
{
    double_link_t *item_after = (double_link_t *)insert_point;
    double_link_t *item = (double_link_t *)item_to_add;
    double_link_t *item_before;

#ifdef PERFORM_INTEGRITY_CHECKS
    USP_ASSERT(DLLIST_IsItemInList(list, item_after)==true);
#endif

    item_before = item_after->prev;

    if (item_before != NULL)
    {
        item_before->next = item; // Insertion at middle of list
    }
    else
    {
        list->head = item;        // Insertion at head of List
    }

    item->next = item_after;

    item_after->prev = item;
    item->prev = item_before;
}

/*********************************************************************//**
**
** DLLIST_MoveLink
**
** Moves an item from one doubly linked list to the tail of another
** NOTE: the source and destination lists can be the same.
** NOTE: The item to move must contain the double_link_t structure at the start of itself
**
** \param   dest_list - pointer to the list header structure to which the item is to be added at the tail
** \param   src_list - pointer to the list header structure to which the item is to be removed
** \param   item_to_move - pointer to the item to move
**
** \return  None
**
**************************************************************************/
void DLLIST_MoveLink(double_linked_list_t *dest_list, double_linked_list_t *src_list, void *item_to_move)
{
    // Unlink the item from the src list
    DLLIST_Unlink(src_list, item_to_move);

    // And link the item to the tail of the destination list
    DLLIST_LinkToTail(dest_list, item_to_move);
}


