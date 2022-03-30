##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
# file the following copyright and licenses apply:
#
# Copyright 2015 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################
AM_CFLAGS = -D_ANSC_LINUX
AM_CFLAGS += -D_ANSC_USER
AM_CFLAGS += -D_ANSC_LITTLE_ENDIAN_

AM_CPPFLAGS = -Wall -Werror
ACLOCAL_AMFLAGS = -I m4
hardware_platform = i686-linux-gnu

lib_LTLIBRARIES = libhal_wifi.la
libhal_wifi_la_CPPFLAGS = -I$(top_srcdir)/./include $(CPPFLAGS)
libhal_wifi_la_SOURCES = wifi_hal.c
libhal_wifi_la_LDFLAGS =
bin_PROGRAMS = wifi_api
wifi_api_SOURCES = wifi_api.c
wifi_api_LDADD = libhal_wifi.la

if INCLUDE_WIFI_HAL_OEM
libhal_wifi_la_SOURCES += wifi_hal_oem.c
endif
