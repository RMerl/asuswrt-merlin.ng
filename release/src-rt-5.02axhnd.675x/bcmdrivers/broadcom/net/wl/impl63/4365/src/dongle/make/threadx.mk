#
# Make settings for threadx components
#
# Copyright 2020 Broadcom
#
# This program is the proprietary software of Broadcom and/or
# its licensors, and may only be used, duplicated, modified or distributed
# pursuant to the terms and conditions of a separate, written license
# agreement executed between you and Broadcom (an "Authorized License").
# Except as set forth in an Authorized License, Broadcom grants no license
# (express or implied), right to use, or waiver of any kind with respect to
# the Software, and Broadcom expressly reserves all rights in and to the
# Software and all intellectual property rights therein.  IF YOU HAVE NO
# AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
# WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
# THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use
# all reasonable efforts to protect the confidentiality thereof, and to
# use this information only in connection with your use of Broadcom
# integrated circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
# REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
# OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
# DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
# NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
# ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
# CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
# OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
# BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
# SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
# IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
# IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
# ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
# OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
# NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
# $Id: threadx.mk 708017 2017-06-29 14:11:45Z $
#

ifeq ($(TARGET_CPU),cm3)
THREADX_SRC_PATH := $(SRCBASE)/../components/threadx/cortex-m3/gnu
endif
ifeq ($(TARGET_CPU),cr4)
THREADX_SRC_PATH := $(SRCBASE)/../components/threadx/cortex-r4/gnu
endif
ifeq ($(TARGET_CPU),ca7)
THREADX_SRC_PATH := $(SRCBASE)/../components/threadx/cortex-a7/gnu
endif

vpath %.c $(SRCBASE)/dongle/rte
vpath %.c $(THREADX_SRC_PATH)
vpath %.S $(THREADX_SRC_PATH)

EXTRA_IFLAGS += -I$(SRCBASE)/dongle/rte
EXTRA_IFLAGS += -I$(THREADX_SRC_PATH)

# ThreadX basic configuration
ifeq ($(TARGET_CPU),cr4)
# These ar defined for cr4 port only
THREADX_DFLAGS	:= -D__THUMB_INTERWORK
THREADX_DFLAGS	+= -DTX_ENABLE_FIQ_SUPPORT
# This is defined by defalut in cm3 port
THREADX_DFLAGS	+= -DTX_TIMER_PROCESS_IN_ISR
endif
ifeq ($(TARGET_CPU),ca7)
# These ar defined for ca7 port only
THREADX_DFLAGS	:= -D__THUMB_INTERWORK
THREADX_DFLAGS	+= -DTX_ENABLE_FIQ_SUPPORT
# This is defined by defalut in cm3 port
THREADX_DFLAGS	+= -DTX_TIMER_PROCESS_IN_ISR
endif
ifeq ($(TARGET_CPU),cm3)
THREADX_DFLAGS	:= -DTX_ENABLE_STACK_CHECKING
endif
THREADX_DFLAGS	+= -DTX_MAX_PRIORITIES=32
# force to include tx_user.h and rte_tx_user_ext.h
THREADX_DFLAGS	+= -DTX_INCLUDE_USER_DEFINE_FILE -DRTE_TX_USER_EXT

# optimize for speed
ifeq ($(TARGET_CPU),cr4)
ifeq ($(call opt,txdebug),1)
	THREADX_DFLAGS += -DTX_ENABLE_STACK_CHECKING
else
	THREADX_DFLAGS	+= -DTX_DISABLE_NOTIFY_CALLBACKS
	THREADX_DFLAGS	+= -DTX_DISABLE_ERROR_CHECKING
endif
# These are defined by default in cm3 port
THREADX_DFLAGS	+= -DTX_DISABLE_PREEMPTION_THRESHOLD
# These are defined for cr4 port only
THREADX_DFLAGS	+= -DTX_DISABLE_REDUNDANT_CLEARING
THREADX_DFLAGS	+= -DTX_INLINE_THREAD_RESUME_SUSPEND
THREADX_DFLAGS	+= -DTX_REACTIVATE_INLINE
THREADX_DFLAGS	+= -DTX_NOT_INTERRUPTABLE
endif

ifeq ($(TARGET_CPU),ca7)
ifeq ($(call opt,txdebug),1)
	THREADX_DFLAGS += -DTX_ENABLE_STACK_CHECKING
else
	THREADX_DFLAGS	+= -DTX_DISABLE_NOTIFY_CALLBACKS
	THREADX_DFLAGS	+= -DTX_DISABLE_ERROR_CHECKING
endif
# These are defined by default in cm3 port
THREADX_DFLAGS	+= -DTX_DISABLE_PREEMPTION_THRESHOLD
# These are defined for ca7 port only
THREADX_DFLAGS	+= -DTX_DISABLE_REDUNDANT_CLEARING
THREADX_DFLAGS	+= -DTX_INLINE_THREAD_RESUME_SUSPEND
THREADX_DFLAGS	+= -DTX_REACTIVATE_INLINE
THREADX_DFLAGS	+= -DTX_NOT_INTERRUPTABLE
endif

# It is undefined when TX_ENABLE_STACK_CHECKING is enabled
# THREADX_DFLAGS	+= -DTX_DISABLE_ERROR_CHECKING

# Enable Broadcom profiling
ifeq ($(BRCM_ENABLE_THREAD_PROFILE),1)
THREADX_DFLAGS  += -DBRCM_ENABLE_THREAD_PROFILE
endif

# ThreadX files
ifeq ($(TARGET_CPU),cr4)
THREADX_OBJECTS := \
	tx_thread_interrupt_disable.o tx_thread_interrupt_restore.o \
	tx_thread_fiq_context_save.o tx_thread_fiq_context_restore.o \
	tx_thread_fiq_nesting_start.o tx_thread_fiq_nesting_end.o \
	tx_thread_irq_nesting_start.o tx_thread_irq_nesting_end.o \
	tx_thread_vectored_context_save.o
endif
ifeq ($(TARGET_CPU),ca7)
THREADX_OBJECTS := \
	tx_thread_interrupt_disable.o tx_thread_interrupt_restore.o \
	tx_thread_fiq_context_save.o tx_thread_fiq_context_restore.o \
	tx_thread_fiq_nesting_start.o tx_thread_fiq_nesting_end.o \
	tx_thread_irq_nesting_start.o tx_thread_irq_nesting_end.o \
	tx_thread_vectored_context_save.o
endif
ifeq ($(TARGET_CPU),cm3)
THREADX_OBJECTS :=
endif
THREADX_OBJECTS += \
	tx_thread_stack_build.o tx_thread_schedule.o tx_thread_system_return.o \
	tx_thread_context_save.o tx_thread_context_restore.o tx_timer_interrupt.o \
	tx_thread_interrupt_control.o tx_block_allocate.o tx_block_pool_cleanup.o \
	tx_block_pool_create.o tx_block_pool_delete.o tx_block_pool_info_get.o \
	tx_block_pool_initialize.o tx_block_pool_performance_info_get.o \
	tx_block_pool_performance_system_info_get.o tx_block_pool_prioritize.o \
	tx_block_release.o tx_byte_allocate.o tx_byte_pool_cleanup.o \
	tx_byte_pool_create.o tx_byte_pool_delete.o tx_byte_pool_info_get.o \
	tx_byte_pool_initialize.o tx_byte_pool_performance_info_get.o \
	tx_byte_pool_performance_system_info_get.o tx_byte_pool_prioritize.o \
	tx_byte_pool_search.o tx_byte_release.o tx_event_flags_cleanup.o \
	tx_event_flags_create.o tx_event_flags_delete.o tx_event_flags_get.o \
	tx_event_flags_info_get.o tx_event_flags_initialize.o \
	tx_event_flags_performance_info_get.o tx_event_flags_performance_system_info_get.o \
	tx_event_flags_set.o tx_event_flags_set_notify.o tx_initialize_high_level.o \
	tx_initialize_kernel_enter.o tx_initialize_kernel_setup.o \
	tx_mutex_cleanup.o tx_mutex_create.o tx_mutex_delete.o tx_mutex_get.o \
	tx_mutex_info_get.o tx_mutex_initialize.o tx_mutex_performance_info_get.o \
	tx_mutex_performance_system_info_get.o tx_mutex_prioritize.o tx_mutex_priority_change.o \
	tx_mutex_put.o tx_queue_cleanup.o tx_queue_create.o \
	tx_queue_delete.o tx_queue_flush.o tx_queue_front_send.o tx_queue_info_get.o \
	tx_queue_initialize.o tx_queue_performance_info_get.o \
	tx_queue_performance_system_info_get.o tx_queue_prioritize.o tx_queue_receive.o \
	tx_queue_send.o tx_queue_send_notify.o tx_semaphore_ceiling_put.o \
	tx_semaphore_cleanup.o tx_semaphore_create.o tx_semaphore_delete.o tx_semaphore_get.o \
	tx_semaphore_info_get.o tx_semaphore_initialize.o \
	tx_semaphore_performance_info_get.o tx_semaphore_performance_system_info_get.o \
	tx_semaphore_prioritize.o tx_semaphore_put.o tx_semaphore_put_notify.o \
	tx_thread_create.o tx_thread_delete.o tx_thread_entry_exit_notify.o \
	tx_thread_identify.o tx_thread_info_get.o tx_thread_initialize.o \
	tx_thread_performance_info_get.o tx_thread_performance_system_info_get.o \
	tx_thread_preemption_change.o tx_thread_priority_change.o tx_thread_relinquish.o \
	tx_thread_reset.o tx_thread_resume.o tx_thread_shell_entry.o tx_thread_sleep.o \
	tx_thread_stack_analyze.o tx_thread_stack_error_handler.o \
	tx_thread_stack_error_notify.o tx_thread_suspend.o tx_thread_system_preempt_check.o \
	tx_thread_system_resume.o tx_thread_system_suspend.o \
	tx_thread_terminate.o tx_thread_time_slice.o tx_thread_time_slice_change.o \
	tx_thread_timeout.o tx_thread_wait_abort.o tx_time_get.o \
	tx_time_set.o tx_timer_activate.o tx_timer_change.o tx_timer_create.o \
	tx_timer_deactivate.o tx_timer_delete.o tx_timer_expiration_process.o \
	tx_timer_info_get.o tx_timer_initialize.o tx_timer_performance_info_get.o \
	tx_timer_performance_system_info_get.o tx_timer_system_activate.o \
	tx_timer_system_deactivate.o tx_timer_thread_entry.o tx_trace_enable.o \
	tx_trace_disable.o tx_trace_initialize.o tx_trace_interrupt_control.o \
	tx_trace_isr_enter_insert.o tx_trace_isr_exit_insert.o tx_trace_object_register.o \
	tx_trace_object_unregister.o tx_trace_user_event_insert.o \
	tx_trace_buffer_full_notify.o tx_trace_event_filter.o tx_trace_event_unfilter.o \
	txe_block_allocate.o txe_block_pool_create.o txe_block_pool_delete.o \
	txe_block_pool_info_get.o txe_block_pool_prioritize.o txe_block_release.o \
	txe_byte_allocate.o txe_byte_pool_create.o txe_byte_pool_delete.o \
	txe_byte_pool_info_get.o txe_byte_pool_prioritize.o txe_byte_release.o \
	txe_event_flags_create.o txe_event_flags_delete.o txe_event_flags_get.o \
	txe_event_flags_info_get.o txe_event_flags_set.o \
	txe_event_flags_set_notify.o txe_mutex_create.o txe_mutex_delete.o \
	txe_mutex_get.o txe_mutex_info_get.o txe_mutex_prioritize.o \
	txe_mutex_put.o txe_queue_create.o txe_queue_delete.o txe_queue_flush.o \
	txe_queue_front_send.o txe_queue_info_get.o txe_queue_prioritize.o \
	txe_queue_receive.o txe_queue_send.o txe_queue_send_notify.o \
	txe_semaphore_ceiling_put.o txe_semaphore_create.o txe_semaphore_delete.o \
	txe_semaphore_get.o txe_semaphore_info_get.o txe_semaphore_prioritize.o \
	txe_semaphore_put.o txe_semaphore_put_notify.o txe_thread_create.o \
	txe_thread_delete.o txe_thread_entry_exit_notify.o txe_thread_info_get.o \
	txe_thread_preemption_change.o txe_thread_priority_change.o \
	txe_thread_relinquish.o txe_thread_reset.o txe_thread_resume.o \
	txe_thread_suspend.o txe_thread_terminate.o txe_thread_time_slice_change.o \
	txe_thread_wait_abort.o txe_timer_activate.o txe_timer_change.o txe_timer_create.o \
	txe_timer_deactivate.o txe_timer_delete.o txe_timer_info_get.o

# ThreadX extensions provided by Express Logic
THREADX_EXT_OBJECTS += tx_low_power.o

# ThreadX RTE files
ifneq ($(wildcard $(THREADX_SRC_PATH)/tx_initialize_kernel_setup.c),)
        RTOS_OBJECTS += $(THREADX_OBJECTS) $(THREADX_EXT_OBJECTS)
else
        RTOS_EXT_OBJECTS := $(THREADX_OBJECTS) $(THREADX_EXT_OBJECTS)
endif

RTOS_OBJECTS += threadx_osl_ext.o threadx.o threadx_low_power.o
ifeq ($(TARGET_ARCH),arm)
	RTOS_OBJECTS  += threadx_arm.o
endif

EXTRA_DFLAGS += $(THREADX_DFLAGS)
EXTRA_DFLAGS += -DTHREADX -DTHREAD_SUPPORT
EXTRA_DFLAGS += -DIRQMODE_SHARED_STACK
