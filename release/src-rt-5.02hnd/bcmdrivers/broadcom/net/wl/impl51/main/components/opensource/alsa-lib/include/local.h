/*
 *  ALSA lib - local header file
 *  Copyright (c) 2000 by Abramo Bagnara <abramo@alsa-project.org>
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

#ifndef __LOCAL_H
#define __LOCAL_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <endian.h>
#include <stdarg.h>
#include <sys/poll.h>
#include <errno.h>

#include "config.h"
#ifdef SUPPORT_RESMGR
#include <resmgr.h>
#endif
#ifdef HAVE_LIBDL
#include <dlfcn.h>
#else
#define RTLD_NOW	0
#endif

#define _snd_config_iterator list_head
#define _snd_interval sndrv_interval
#define _snd_pcm_info sndrv_pcm_info
#define _snd_pcm_hw_params sndrv_pcm_hw_params
#define _snd_pcm_sw_params sndrv_pcm_sw_params
#define _snd_pcm_status sndrv_pcm_status

#define _snd_ctl_card_info sndrv_ctl_card_info
#define _snd_ctl_elem_id sndrv_ctl_elem_id
#define _snd_ctl_elem_list sndrv_ctl_elem_list
#define _snd_ctl_elem_info sndrv_ctl_elem_info
#define _snd_ctl_elem_value sndrv_ctl_elem_value
#define _snd_ctl_event sndrv_ctl_event

#define _snd_rawmidi_info sndrv_rawmidi_info
#define _snd_rawmidi_params sndrv_rawmidi_params
#define _snd_rawmidi_status sndrv_rawmidi_status

#define _snd_hwdep_info sndrv_hwdep_info
#define _snd_hwdep_dsp_status sndrv_hwdep_dsp_status
#define _snd_hwdep_dsp_image sndrv_hwdep_dsp_image

#define _snd_seq_queue_tempo sndrv_seq_queue_tempo
#define _snd_seq_client_info sndrv_seq_client_info
#define _snd_seq_port_info sndrv_seq_port_info
#define _snd_seq_system_info sndrv_seq_system_info
#define _snd_seq_queue_info sndrv_seq_queue_info
#define _snd_seq_queue_status sndrv_seq_queue_status
#define _snd_seq_queue_timer sndrv_seq_queue_timer
#define _snd_seq_port_subscribe sndrv_seq_port_subscribe
#define _snd_seq_query_subscribe sndrv_seq_query_subs
#define _snd_seq_client_pool sndrv_seq_client_pool
#define _snd_seq_remove_events sndrv_seq_remove_events

#define sndrv_seq_addr	snd_seq_addr
#define sndrv_seq_tick_time_t	snd_seq_tick_time_t
#define sndrv_seq_real_time	snd_seq_real_time
#define sndrv_seq_timestamp	snd_seq_timestamp
#define sndrv_seq_event		snd_seq_event


#define _snd_timer_id sndrv_timer_id
#define _snd_timer_ginfo sndrv_timer_ginfo
#define _snd_timer_gparams sndrv_timer_gparams
#define _snd_timer_gstatus sndrv_timer_gstatus
#define _snd_timer_select sndrv_timer_select
#define _snd_timer_info sndrv_timer_info
#define _snd_timer_params sndrv_timer_params
#define _snd_timer_status sndrv_timer_status

#define ALSA_LIBRARY_BUILD

#include <sound/asound.h>
#include <sound/asoundef.h>
#include "alsa-symbols.h"
#include "version.h"
#include "global.h"
#include "input.h"
#include "output.h"
#include "error.h"
#include "conf.h"
#include "pcm.h"
#include "pcm_plugin.h"
#include "rawmidi.h"
#include "timer.h"
#include "hwdep.h"
#include "control.h"
#include "mixer.h"
#include "seq_event.h"
#include "seq.h"
#include <sound/asequencer.h>
#include "seqmid.h"
#include "seq_midi_event.h"
#include "list.h"

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define SND_LITTLE_ENDIAN
#endif
#if __BYTE_ORDER == __BIG_ENDIAN
#define SND_BIG_ENDIAN
#endif

struct _snd_async_handler {
	enum {
		SND_ASYNC_HANDLER_GENERIC,
		SND_ASYNC_HANDLER_CTL,
		SND_ASYNC_HANDLER_PCM,
		SND_ASYNC_HANDLER_TIMER,
	} type;
	int fd;
	union {
		snd_ctl_t *ctl;
		snd_pcm_t *pcm;
		snd_timer_t *timer;
	} u;
	snd_async_callback_t callback;
	void *private_data;
	struct list_head glist;
	struct list_head hlist;
};

typedef enum _snd_set_mode {
	SND_CHANGE,
	SND_TRY,
	SND_TEST,
} snd_set_mode_t;

size_t page_align(size_t size);
size_t page_size(void);
size_t page_ptr(size_t object_offset, size_t object_size, size_t *offset, size_t *mmap_offset);

int safe_strtol(const char *str, long *val);

int snd_send_fd(int sock, void *data, size_t len, int fd);
int snd_receive_fd(int sock, void *data, size_t len, int *fd);

/*
 * error messages
 */
#ifndef NDEBUG
#define CHECK_SANITY(x) x
extern snd_lib_error_handler_t snd_err_msg;
#define SNDMSG(args...) snd_err_msg(__FILE__, __LINE__, __FUNCTION__, 0, ##args)
#define SYSMSG(args...) snd_err_msg(__FILE__, __LINE__, __FUNCTION__, errno, ##args)
#else
#define CHECK_SANITY(x) 0 /* not evaluated */
#define SNDMSG(args...) /* nop */
#define SYSMSG(args...) /* nop */
#endif

/*
 */
#define HAVE_GNU_LD
#define HAVE_ELF
#define HAVE_ASM_PREVIOUS_DIRECTIVE

/* Stolen from libc-symbols.h in GNU glibc */

/* When a reference to SYMBOL is encountered, the linker will emit a
   warning message MSG.  */

#define ASM_NAME(name) __SYMBOL_PREFIX name

#ifdef HAVE_GNU_LD
# ifdef HAVE_ELF

/* We want the .gnu.warning.SYMBOL section to be unallocated.  */
#  ifdef HAVE_ASM_PREVIOUS_DIRECTIVE
#   define __make_section_unallocated(section_string)	\
  asm (".section " section_string "\n\t.previous");
#  elif defined HAVE_ASM_POPSECTION_DIRECTIVE
#   define __make_section_unallocated(section_string)	\
  asm (".pushsection " section_string "\n\t.popsection");
#  else
#   define __make_section_unallocated(section_string)
#  endif

/* Tacking on "\n\t#" to the section name makes gcc put it's bogus
   section attributes on what looks like a comment to the assembler.  */
#  ifdef HAVE_SECTION_QUOTES
#   define link_warning(symbol, msg) \
  __make_section_unallocated (".gnu.warning." ASM_NAME(#symbol)) \
  static const char __evoke_link_warning_##symbol[]	\
    __attribute__ ((section (".gnu.warning." ASM_NAME(#symbol) "\"\n\t#\""))) = msg;
#  else
#   define link_warning(symbol, msg) \
  __make_section_unallocated (".gnu.warning." ASM_NAME(#symbol)) \
  static const char __evoke_link_warning_##symbol[]	\
    __attribute__ ((section (".gnu.warning." ASM_NAME(#symbol) "\n\t#"))) = msg;
#  endif
# else
#  define link_warning(symbol, msg)		\
  asm (".stabs \"" msg "\",30,0,0,0\n\t"	\
       ".stabs \"" ASM_NAME(#symbol) "\",1,0,0,0\n");
# endif
#else
/* We will never be heard; they will all die horribly.  */
# define link_warning(symbol, msg)
#endif

static inline int snd_open_device(const char *filename, int fmode)
{
	int fd;

#ifdef O_CLOEXEC
	fmode |= O_CLOEXEC;
#endif
	fd = open(filename, fmode);

/* open with resmgr */
#ifdef SUPPORT_RESMGR
	if (fd < 0) {
		if (errno == EAGAIN || errno == EBUSY)
			return fd;
		if (! access(filename, F_OK))
			fd = rsm_open_device(filename, fmode);
	}
#endif
	if (fd >= 0)
		fcntl(fd, F_SETFD, FD_CLOEXEC);
	return fd;
}

/* make local functions really local */
#define snd_dlobj_cache_lookup \
	snd1_dlobj_cache_lookup
#define snd_dlobj_cache_add \
	snd1_dlobj_cache_add
#define snd_dlobj_cache_cleanup \
	snd1_dlobj_cache_cleanup
#define snd_config_set_hop \
	snd1_config_set_hop
#define snd_config_check_hop \
	snd1_config_check_hop
#define snd_config_search_alias_hooks \
	snd1_config_search_alias_hooks

/* dlobj cache */
void *snd_dlobj_cache_lookup(const char *name);
int snd_dlobj_cache_add(const char *name, void *dlobj, void *open_func);
void snd_dlobj_cache_cleanup(void);

/* for recursive checks */
void snd_config_set_hop(snd_config_t *conf, int hop);
int snd_config_check_hop(snd_config_t *conf);
#define SND_CONF_MAX_HOPS	64

int snd_config_search_alias_hooks(snd_config_t *config,
                                  const char *base, const char *key,
				  snd_config_t **result);

#endif
