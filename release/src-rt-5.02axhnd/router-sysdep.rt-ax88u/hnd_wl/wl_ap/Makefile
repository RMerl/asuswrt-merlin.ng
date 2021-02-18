#
# Makefile for the Broadcom wl driver
#
# Copyright (C) 2019, Broadcom. All Rights Reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# <<Broadcom-WL-IPTag/Open:>>
#
# $Id: Makefile 693762 2017-04-05 02:06:54Z $
#

SRCBASE_OFFSET := ../../hnd_shared/../../../main/src
ROUTERBASE_OFFSET := ../../hnd_shared/../router

ifeq ($(CONFIG_WL_AP),)
CONFIG_WL_AP := "wlconfig_lx_router_ap"
endif
CONFIG_WL_CONF = $(CONFIG_WL_AP)

wl_suffix := ap

TARGET := wl_$(wl_suffix)
WLCONF_H := ../wlconf.h
WLCONF_O := ../wlconf.o

# kbuild part of makefile
include $(src)/../wl_generic.mk
