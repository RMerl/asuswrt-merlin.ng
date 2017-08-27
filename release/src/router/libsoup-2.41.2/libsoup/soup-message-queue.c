/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-message-queue.c: Message queue
 *
 * Copyright (C) 2003 Novell, Inc.
 * Copyright (C) 2008 Red Hat, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "soup-message-queue.h"
#include "soup.h"

/* This is an internal structure used by #SoupSession and its
 * subclasses to keep track of the status of messages currently being
 * processed.
 *
 * The #SoupMessageQueue itself is mostly just a linked list of
 * #SoupMessageQueueItem, with some added cleverness to allow the list
 * to be walked safely while other threads / re-entrant loops are
 * adding items to and removing items from it. In particular, this is
 * handled by refcounting items and then keeping "removed" items in
 * the list until their ref_count drops to 0, but skipping over the
 * "removed" ones when walking the queue.
 **/

struct _SoupMessageQueue {
	SoupSession *session;

	GMutex mutex;
	SoupMessageQueueItem *head, *tail;
};

SoupMessageQueue *
soup_message_queue_new (SoupSession *session)
{
	SoupMessageQueue *queue;

	queue = g_slice_new0 (SoupMessageQueue);
	queue->session = session;
	g_mutex_init (&queue->mutex);
	return queue;
}

void
soup_message_queue_destroy (SoupMessageQueue *queue)
{
	g_return_if_fail (queue->head == NULL);

	g_mutex_clear (&queue->mutex);
	g_slice_free (SoupMessageQueue, queue);
}

static void
queue_message_restarted (SoupMessage *msg, gpointer user_data)
{
	SoupMessageQueueItem *item = user_data;

	if (item->proxy_addr) {
		g_object_unref (item->proxy_addr);
		item->proxy_addr = NULL;
	}
	if (item->proxy_uri) {
		soup_uri_free (item->proxy_uri);
		item->proxy_uri = NULL;
	}

	g_cancellable_reset (item->cancellable);
}

/**
 * soup_message_queue_append:
 * @queue: a #SoupMessageQueue
 * @msg: a #SoupMessage
 * @callback: the callback for @msg
 * @user_data: the data to pass to @callback
 *
 * Creates a new #SoupMessageQueueItem and appends it to @queue.
 *
 * Return value: the new item, which you must unref with
 * soup_message_queue_unref_item() when you are done with.
 **/
SoupMessageQueueItem *
soup_message_queue_append (SoupMessageQueue *queue, SoupMessage *msg,
			   SoupSessionCallback callback, gpointer user_data)
{
	SoupMessageQueueItem *item;

	item = g_slice_new0 (SoupMessageQueueItem);
	item->session = g_object_ref (queue->session);
	item->async_context = soup_session_get_async_context (item->session);
	item->queue = queue;
	item->msg = g_object_ref (msg);
	item->callback = callback;
	item->callback_data = user_data;
	item->cancellable = g_cancellable_new ();

	g_signal_connect (msg, "restarted",
			  G_CALLBACK (queue_message_restarted), item);

	/* Note: the initial ref_count of 1 represents the caller's
	 * ref; the queue's own ref is indicated by the absence of the
	 * "removed" flag.
	 */
	item->ref_count = 1;

	g_mutex_lock (&queue->mutex);
	if (queue->head) {
		queue->tail->next = item;
		item->prev = queue->tail;
		queue->tail = item;
	} else
		queue->head = queue->tail = item;

	g_mutex_unlock (&queue->mutex);
	return item;
}

/**
 * soup_message_queue_item_ref:
 * @item: a #SoupMessageQueueItem
 *
 * Refs @item.
 **/ 
void
soup_message_queue_item_ref (SoupMessageQueueItem *item)
{
	item->ref_count++;
}

/**
 * soup_message_queue_item_unref:
 * @item: a #SoupMessageQueueItem
 *
 * Unrefs @item; use this on a #SoupMessageQueueItem that you are done
 * with (but that you aren't passing to
 * soup_message_queue_item_next()).
 **/ 
void
soup_message_queue_item_unref (SoupMessageQueueItem *item)
{
	g_mutex_lock (&item->queue->mutex);

	/* Decrement the ref_count; if it's still non-zero OR if the
	 * item is still in the queue, then return.
	 */
	if (--item->ref_count || !item->removed) {
		g_mutex_unlock (&item->queue->mutex);
		return;
	}

	g_warn_if_fail (item->conn == NULL);

	/* OK, @item is dead. Rewrite @queue around it */
	if (item->prev)
		item->prev->next = item->next;
	else
		item->queue->head = item->next;
	if (item->next)
		item->next->prev = item->prev;
	else
		item->queue->tail = item->prev;

	g_mutex_unlock (&item->queue->mutex);

	/* And free it */
	g_signal_handlers_disconnect_by_func (item->msg,
					      queue_message_restarted, item);
	g_object_unref (item->session);
	g_object_unref (item->msg);
	g_object_unref (item->cancellable);
	g_clear_object (&item->proxy_addr);
	g_clear_pointer (&item->proxy_uri, soup_uri_free);
	g_clear_object (&item->task);
	if (item->io_source) {
		g_source_destroy (item->io_source);
		g_source_unref (item->io_source);
	}
	g_slice_free (SoupMessageQueueItem, item);
}

/**
 * soup_message_queue_lookup:
 * @queue: a #SoupMessageQueue
 * @msg: a #SoupMessage
 *
 * Finds the #SoupMessageQueueItem for @msg in @queue. You must unref
 * the item with soup_message_queue_unref_item() when you are done
 * with it.
 *
 * Return value: the queue item for @msg, or %NULL
 **/ 
SoupMessageQueueItem *
soup_message_queue_lookup (SoupMessageQueue *queue, SoupMessage *msg)
{
	SoupMessageQueueItem *item;

	g_mutex_lock (&queue->mutex);

	item = queue->tail;
	while (item && (item->removed || item->msg != msg))
		item = item->prev;

	if (item)
		item->ref_count++;

	g_mutex_unlock (&queue->mutex);
	return item;
}

/**
 * soup_message_queue_first:
 * @queue: a #SoupMessageQueue
 *
 * Gets the first item in @queue. You must unref the item by calling
 * soup_message_queue_unref_item() on it when you are done.
 * (soup_message_queue_next() does this for you automatically, so you
 * only need to unref the item yourself if you are not going to
 * finishing walking the queue.)
 *
 * Return value: the first item in @queue.
 **/ 
SoupMessageQueueItem *
soup_message_queue_first (SoupMessageQueue *queue)
{
	SoupMessageQueueItem *item;

	g_mutex_lock (&queue->mutex);

	item = queue->head;
	while (item && item->removed)
		item = item->next;

	if (item)
		item->ref_count++;

	g_mutex_unlock (&queue->mutex);
	return item;
}

/**
 * soup_message_queue_next:
 * @queue: a #SoupMessageQueue
 * @item: a #SoupMessageQueueItem
 *
 * Unrefs @item and gets the next item after it in @queue. As with
 * soup_message_queue_first(), you must unref the returned item
 * yourself with soup_message_queue_unref_item() if you do not finish
 * walking the queue.
 *
 * Return value: the next item in @queue.
 **/ 
SoupMessageQueueItem *
soup_message_queue_next (SoupMessageQueue *queue, SoupMessageQueueItem *item)
{
	SoupMessageQueueItem *next;

	g_mutex_lock (&queue->mutex);

	next = item->next;
	while (next && next->removed)
		next = next->next;
	if (next)
		next->ref_count++;

	g_mutex_unlock (&queue->mutex);
	soup_message_queue_item_unref (item);
	return next;
}

/**
 * soup_message_queue_remove:
 * @queue: a #SoupMessageQueue
 * @item: a #SoupMessageQueueItem
 *
 * Removes @item from @queue. Note that you probably also need to call
 * soup_message_queue_unref_item() after this.
 **/ 
void
soup_message_queue_remove (SoupMessageQueue *queue, SoupMessageQueueItem *item)
{
	g_return_if_fail (!item->removed);

	g_mutex_lock (&queue->mutex);
	item->removed = TRUE;
	g_mutex_unlock (&queue->mutex);
}
