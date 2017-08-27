/**
 * \file async.c
 * \brief Async notification helpers
 * \author Abramo Bagnara <abramo@alsa-project.org>
 * \date 2001
 */
/*
 *  Async notification helpers
 *  Copyright (c) 2001 by Abramo Bagnara <abramo@alsa-project.org>
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include "pcm/pcm_local.h"
#include "control/control_local.h"
#include <signal.h>

#ifdef SND_ASYNC_RT_SIGNAL
/** async signal number */
static int snd_async_signo;

void snd_async_init(void) __attribute__ ((constructor));

void snd_async_init(void)
{
	snd_async_signo = __libc_allocate_rtsig(0);
	if (snd_async_signo < 0) {
		SNDERR("Unable to find a RT signal to use for snd_async");
		exit(1);
	}
}
#else
/** async signal number */
static const int snd_async_signo = SIGIO;
#endif

static LIST_HEAD(snd_async_handlers);

static void snd_async_handler(int signo ATTRIBUTE_UNUSED, siginfo_t *siginfo, void *context ATTRIBUTE_UNUSED)
{
	int fd;
	struct list_head *i;
	//assert(siginfo->si_code == SI_SIGIO);
	fd = siginfo->si_fd;
	list_for_each(i, &snd_async_handlers) {
		snd_async_handler_t *h = list_entry(i, snd_async_handler_t, glist);
		if (h->fd == fd && h->callback)
			h->callback(h);
	}
}

/**
 * \brief Registers an async handler.
 * \param handler The function puts the pointer to the new async handler
 *                object at the address specified by \p handler.
 * \param fd The file descriptor to be associated with the callback.
 * \param callback The async callback function.
 * \param private_data Private data for the async callback function.
 * \result Zero if successful, otherwise a negative error code.
 *
 * This function associates the callback function with the given file,
 * and saves this association in a \c snd_async_handler_t object.
 *
 * Whenever the \c SIGIO signal is raised for the file \p fd, the callback
 * function will be called with its parameter pointing to the async handler
 * object returned by this function.
 *
 * The ALSA \c sigaction handler for the \c SIGIO signal automatically
 * multiplexes the notifications to the registered async callbacks.
 * However, the application is responsible for instructing the device driver
 * to generate the \c SIGIO signal.
 *
 * The \c SIGIO signal may have been replaced with another signal,
 * see #snd_async_handler_get_signo.
 *
 * When the async handler isn't needed anymore, you must delete it with
 * #snd_async_del_handler.
 *
 * \see snd_async_add_pcm_handler, snd_async_add_ctl_handler
 */
int snd_async_add_handler(snd_async_handler_t **handler, int fd, 
			  snd_async_callback_t callback, void *private_data)
{
	snd_async_handler_t *h;
	int was_empty;
	assert(handler);
	h = malloc(sizeof(*h));
	if (!h)
		return -ENOMEM;
	h->fd = fd;
	h->callback = callback;
	h->private_data = private_data;
	was_empty = list_empty(&snd_async_handlers);
	list_add_tail(&h->glist, &snd_async_handlers);
	INIT_LIST_HEAD(&h->hlist);
	*handler = h;
	if (was_empty) {
		int err;
		struct sigaction act;
		memset(&act, 0, sizeof(act));
		act.sa_flags = SA_RESTART | SA_SIGINFO;
		act.sa_sigaction = snd_async_handler;
		sigemptyset(&act.sa_mask);
		err = sigaction(snd_async_signo, &act, NULL);
		if (err < 0) {
			SYSERR("sigaction");
			return -errno;
		}
	}
	return 0;
}

/**
 * \brief Deletes an async handler.
 * \param handler Handle of the async handler to delete.
 * \result Zero if successful, otherwise a negative error code.
 */
int snd_async_del_handler(snd_async_handler_t *handler)
{
	int err = 0;
	assert(handler);
	list_del(&handler->glist);
	if (list_empty(&snd_async_handlers)) {
		struct sigaction act;
		memset(&act, 0, sizeof(act));
		act.sa_flags = 0;
		act.sa_handler = SIG_DFL;
		err = sigaction(snd_async_signo, &act, NULL);
		if (err < 0) {
			SYSERR("sigaction");
			return -errno;
		}
	}
	if (handler->type == SND_ASYNC_HANDLER_GENERIC)
		goto _end;
	if (!list_empty(&handler->hlist))
		list_del(&handler->hlist);
	if (!list_empty(&handler->hlist))
		goto _end;
	switch (handler->type) {
#ifdef BUILD_PCM
	case SND_ASYNC_HANDLER_PCM:
		err = snd_pcm_async(handler->u.pcm, -1, 1);
		break;
#endif
	case SND_ASYNC_HANDLER_CTL:
		err = snd_ctl_async(handler->u.ctl, -1, 1);
		break;
	default:
		assert(0);
	}
 _end:
	free(handler);
	return err;
}

/**
 * \brief Returns the signal number assigned to an async handler.
 * \param handler Handle to an async handler.
 * \result The signal number if successful, otherwise a negative error code.
 *
 * The signal number for async handlers usually is \c SIGIO,
 * but wizards can redefine it to a realtime signal
 * when compiling the ALSA library.
 */
int snd_async_handler_get_signo(snd_async_handler_t *handler)
{
	assert(handler);
	return snd_async_signo;
}

/**
 * \brief Returns the file descriptor assigned to an async handler.
 * \param handler Handle to an async handler.
 * \result The file descriptor if successful, otherwise a negative error code.
 */
int snd_async_handler_get_fd(snd_async_handler_t *handler)
{
	assert(handler);
	return handler->fd;
}

/**
 * \brief Returns the private data assigned to an async handler.
 * \param handler Handle to an async handler.
 * \result The \c private_data value registered with the async handler.
 */
void *snd_async_handler_get_callback_private(snd_async_handler_t *handler)
{
	assert(handler);
	return handler->private_data;
}
