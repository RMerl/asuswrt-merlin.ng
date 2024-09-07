/*
 *
 * Copyright (C) 2019, Broadband Forum
 * Copyright (C) 2016-2019  CommScope, Inc
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
 * \file dllist.h
 *
 * Implements a doubly linked list library
 *
 */

#ifndef DLLIST_H
#define DLLIST_H

//-----------------------------------------------------------------------------------------
// Doubly Linked list types
typedef struct double_link_t_tag
{
   struct double_link_t_tag *next;
   struct double_link_t_tag *prev;
} double_link_t;

typedef struct
{
   double_link_t *head;
   double_link_t *tail;
} double_linked_list_t;

//-----------------------------------------------------------------------------------------
// Doubly linked list API
void DLLIST_Init(double_linked_list_t *list);
bool DLLIST_IsItemInList(double_linked_list_t *list, double_link_t *item_to_check);
void DLLIST_LinkToHead(double_linked_list_t *list, void *item_to_add);
void DLLIST_LinkToTail(double_linked_list_t *list, void *item_to_add);
void DLLIST_Unlink(double_linked_list_t *list, void *item_to_remove);
void DLLIST_InsertLinkBefore(void *insert_point, double_linked_list_t *list, void *item_to_add);
void DLLIST_MoveLink(double_linked_list_t *dest_list, double_linked_list_t *src_list, void *item_to_move);

#endif // DLLIST_H
