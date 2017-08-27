/**
 * \file control/cards.c
 * \brief Basic Soundcard Operations
 * \author Jaroslav Kysela <perex@perex.cz>
 * \date 1998-2001
 */
/*
 *  Soundcard Operations - main file
 *  Copyright (c) 1998 by Jaroslav Kysela <perex@perex.cz>
 *
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "control_local.h"

#ifndef DOC_HIDDEN
#define SND_FILE_CONTROL	ALSA_DEVICE_DIRECTORY "controlC%i"
#define SND_FILE_LOAD		ALOAD_DEVICE_DIRECTORY "aloadC%i"
#endif

static int snd_card_load2(const char *control)
{
	int open_dev;
	snd_ctl_card_info_t info;

	open_dev = snd_open_device(control, O_RDONLY);
	if (open_dev >= 0) {
		if (ioctl(open_dev, SNDRV_CTL_IOCTL_CARD_INFO, &info) < 0) {
			int err = -errno;
			close(open_dev);
			return err;
		}
		close(open_dev);
		return info.card;
	} else {
		return -errno;
	}
}

static int snd_card_load1(int card)
{
	int res;
	char control[sizeof(SND_FILE_CONTROL) + 10];

	sprintf(control, SND_FILE_CONTROL, card);
	res = snd_card_load2(control);
#ifdef SUPPORT_ALOAD
	if (res < 0) {
		char aload[sizeof(SND_FILE_LOAD) + 10];
		sprintf(aload, SND_FILE_LOAD, card);
		res = snd_card_load2(aload);
	}
#endif
	return res;
}

/**
 * \brief Try to load the driver for a card.
 * \param card Card number.
 * \return 1 if driver is present, zero if driver is not present
 */
int snd_card_load(int card)
{
	return !!(snd_card_load1(card) >= 0);
}

/**
 * \brief Try to determine the next card.
 * \param rcard pointer to card number
 * \result zero if success, otherwise a negative error code
 *
 * Tries to determine the next card from given card number.
 * If card number is -1, then the first available card is
 * returned. If the result card number is -1, no more cards
 * are available.
 */
int snd_card_next(int *rcard)
{
	int card;
	
	if (rcard == NULL)
		return -EINVAL;
	card = *rcard;
	card = card < 0 ? 0 : card + 1;
	for (; card < 32; card++) {
		if (snd_card_load(card)) {
			*rcard = card;
			return 0;
		}
	}
	*rcard = -1;
	return 0;
}

/**
 * \brief Convert card string to an integer value.
 * \param string String containing card identifier
 * \return zero if success, otherwise a negative error code
 *
 * The accepted format is an integer value in ASCII representation
 * or the card identifier (the id parameter for sound-card drivers).
 * The control device name like /dev/snd/controlC0 is accepted, too.
 */
int snd_card_get_index(const char *string)
{
	int card, err;
	snd_ctl_t *handle;
	snd_ctl_card_info_t info;

	if (!string || *string == '\0')
		return -EINVAL;
	if ((isdigit(*string) && *(string + 1) == 0) ||
	    (isdigit(*string) && isdigit(*(string + 1)) && *(string + 2) == 0)) {
		if (sscanf(string, "%i", &card) != 1)
			return -EINVAL;
		if (card < 0 || card > 31)
			return -EINVAL;
		err = snd_card_load1(card);
		if (err >= 0)
			return card;
		return err;
	}
	if (string[0] == '/')	/* device name */
		return snd_card_load2(string);
	for (card = 0; card < 32; card++) {
#ifdef SUPPORT_ALOAD
		if (! snd_card_load(card))
			continue;
#endif
		if (snd_ctl_hw_open(&handle, NULL, card, 0) < 0)
			continue;
		if (snd_ctl_card_info(handle, &info) < 0) {
			snd_ctl_close(handle);
			continue;
		}
		snd_ctl_close(handle);
		if (!strcmp((const char *)info.id, string))
			return card;
	}
	return -ENODEV;
}

/**
 * \brief Obtain the card name.
 * \param card Card number
 * \param name Result - card name corresponding to card number
 * \result zero if success, otherwise a negative error code
 */
int snd_card_get_name(int card, char **name)
{
	snd_ctl_t *handle;
	snd_ctl_card_info_t info;
	int err;
	
	if (name == NULL)
		return -EINVAL;
	if ((err = snd_ctl_hw_open(&handle, NULL, card, 0)) < 0)
		return err;
	if ((err = snd_ctl_card_info(handle, &info)) < 0) {
		snd_ctl_close(handle);
		return err;
	}
	snd_ctl_close(handle);
	*name = strdup((const char *)info.name);
	if (*name == NULL)
		return -ENOMEM;
	return 0;
}

/**
 * \brief Obtain the card long name.
 * \param card Card number
 * \param name Result - card long name corresponding to card number
 * \result zero if success, otherwise a negative error code
 */
int snd_card_get_longname(int card, char **name)
{
	snd_ctl_t *handle;
	snd_ctl_card_info_t info;
	int err;
	
	if (name == NULL)
		return -EINVAL;
	if ((err = snd_ctl_hw_open(&handle, NULL, card, 0)) < 0)
		return err;
	if ((err = snd_ctl_card_info(handle, &info)) < 0) {
		snd_ctl_close(handle);
		return err;
	}
	snd_ctl_close(handle);
	*name = strdup((const char *)info.longname);
	if (*name == NULL)
		return -ENOMEM;
	return 0;
}
