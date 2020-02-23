#***********************************************************************
#
#  Copyright (c) 2006-2013  Broadcom Corporation
#  All Rights Reserved
#
#***********************************************************************/

#
# In most cases, you only need to modify this first section.
#
LIB = device2.a

REAL_OBJS := rcl2_device.o stl2_device.o \
             rcl2_ip.o stl2_ip.o rut2_ip.o rut2_ip6.o \
             rut2_ipservicecfg.o rut2_ipservicecfg6.o \
             rcl2_bridging.o stl2_bridging.o rut2_bridging.o \
             rcl2_ppp.o stl2_ppp.o  rut2_ppp.o \
             rcl2_interfacestack.o stl2_interfacestack.o \
             rut2_lan.o rcl2_hosts.o stl2_hosts.o \
             rcl2_ethernet.o stl2_ethernet.o rut2_ethernet.o \
             rcl2_ethernetlink.o stl2_ethernetlink.o \
             rcl2_ethernetvlantermination.o stl2_ethernetvlantermination.o rut2_ethernetvlantermination.o \
             rut2_nat.o \
             rcl2_dhcpv4.o stl2_dhcpv4.o rut2_dhcpv4.o \
             rcl2_qos.o stl2_qos.o rut2_qos_queue_stats.o \
             rut2_qos_class.o rut2_qos_queue.o rut2_qos_policer.o \
             rcl2_route.o stl2_route.o rut2_route.o rut2_route6.o \
             rcl2_neighbordiscovery.o stl2_neighbordiscovery.o rut2_neighbordiscovery.o \
             rcl2_routeradvertisement.o stl2_routeradvertisement.o \
             rcl2_dns.o stl2_dns.o rut2_dns.o rut2_dns6.o \
             rcl2_dslite.o stl2_dslite.o \
             rcl2_ipv6rd.o stl2_ipv6rd.o rut2_iptunnel.o\
             rcl2_dhcpv6.o stl2_dhcpv6.o rcl2_dhcpv6client.o stl2_dhcpv6client.o rcl2_dhcpv6server.o stl2_dhcpv6server.o\
             rut2_dhcpv6.o \
             rcl2_ipdiag.o stl2_ipdiag.o \
             rut2_util.o \
             rcl2_cert.o stl2_cert.o rut2_cert.o \
             rut2_wan.o rut2_wan6.o \
             rcl2_nat.o stl2_nat.o \
             rut2_iptables.o \
             rcl2_firewall.o stl2_firewall.o \
             rut2_multicast.o \
             rut2_routeradvertisement.o \
             rcl2_status_memory.o stl2_status_memory.o rut2_status_memory.o \
             rcl2_status_process.o stl2_status_process.o rut2_status_process.o \
             rcl2_gatewayInfo.o stl2_gatewayInfo.o \
             rcl2_macfilter.o stl2_macfilter.o \
             rcl2_system.o stl2_system.o \
             rcl2_ipsec.o stl2_ipsec.o rut2_ipsec.o \
             rcl2_tr69c.o stl2_tr69c.o \
             rcl2_ethlag.o rut2_ethlag.o stl2_ethlag.o

             
GENERATED_OBJS := 

all: $(LIB)

clean: generic_clean



#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
# You do not need to modify this part.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common

ifneq ($(strip $(BRCM_VOICE_SUPPORT)),)
include $(BUILD_DIR)/make.voice
endif

ifneq ($(strip $(BUILD_BRCM_UNFWLCFG)),)
REAL_OBJS += rcl2_unfwifi.o stl2_unfwifi.o rut2_wifi.o rut2_unfwifi.o
else
REAL_OBJS += rcl2_wifi.o stl2_wifi.o rut2_wifi.o rcl2_wifi_accesspoint.o stl2_wifi_accesspoint.o \
             rcl2_wifi_endpoint.o stl2_wifi_endpoint.o rcl2_wifi_neighboring.o stl2_wifi_neighboring.o
endif

ifneq ($(strip $(BUILD_MAP)),)
REAL_OBJS += rcl2_map.o stl2_map.o
endif

ifneq ($(strip $(BUILD_XMPP)),)
REAL_OBJS += rcl2_xmpp.o stl2_xmpp.o rut2_xmpp.o
endif

ifneq ($(strip $(BUILD_HOMEPLUG)),)
REAL_OBJS += rcl2_homeplug.o stl2_homeplug.o rut2_homeplug.o
endif

ifneq ($(strip $(BUILD_GPON)),)
REAL_OBJS += rcl2_optical.o stl2_optical.o
endif

ifneq ($(strip $(BUILD_LIBMODSW)),)
REAL_OBJS += rcl2_modsw.o stl2_modsw.o rut2_modsw.o
endif

ifneq ($(strip $(BUILD_MODSW_OSGIEE)),)
REAL_OBJS += rut2_modsw_osgiee.o
endif

ifneq ($(strip $(BUILD_MODSW_LINUXEE)),)
REAL_OBJS += rut2_modsw_linuxee.o
endif

ifneq ($(strip $(BUILD_IEEE1905)),)
REAL_OBJS += rcl2_ieee1905.o stl2_ieee1905.o rut2_ieee1905.o
endif

ifneq ($(strip $(BUILD_MoCACTL)),)
REAL_OBJS += rcl2_moca.o stl2_moca.o rut2_moca.o
endif

ifneq ($(strip $(BUILD_TR69_PERIODIC_STATISTICS)),)
REAL_OBJS += rcl2_periodicstats.o stl2_periodicstats.o
endif

ifneq ($(strip $(BUILD_DSL)),)
REAL_OBJS += rcl2_atm.o stl2_atm.o rut2_atm.o rcl2_ptm.o stl2_ptm.o rut2_ptm.o rcl2_dsl.o stl2_dsl.o rut2_dsl.o
REAL_OBJS += rcl2_dslbonding.o stl2_dslbonding.o
ifeq ($(strip $(BRCM_PHY_GFAST)),y)
REAL_OBJS += rcl2_fast.o stl2_fast.o rut2_fast.o
endif
endif

ifneq ($(strip $(BUILD_ZEBRA)),)
REAL_OBJS += rut2_rip.o
endif

ifneq ($(strip $(BUILD_SNTP)),)
REAL_OBJS += rcl2_time.o stl2_time.o rut2_time.o
endif

ifneq ($(strip $(BUILD_BUSYBOX_NTPD)),)
REAL_OBJS += rcl2_time.o stl2_time.o rut2_time.o
endif

ifneq ($(strip $(BRCM_DRIVER_USB)),)
REAL_OBJS += rcl2_usb.o stl2_usb.o
endif

ifneq ($(strip $(BUILD_L2TPAC)),)
REAL_OBJS += rcl2_l2tpac.o stl2_l2tpac.o rut2_l2tpac.o
endif

ifeq ($(strip $(BRCM_MULTIPLE_TR69C_SUPPORT)),2)
REAL_OBJS += rcl2_tr69c_ene.o
REAL_OBJS += stl2_tr69c_ene.o
endif


OBJS = $(REAL_OBJS) $(GENERATED_OBJS)

#
# Private apps and libs are allowed to include header files from the
# private and public directories
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
ALLOWED_INCLUDE_PATHS := -I. -I.. -I../.. \
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/include  \
                         -I$(BUILD_DIR)/userspace/private/include/$(OALDIR) \
                         -I$(BCM_FSBUILD_DIR)/public/include \
                         -I$(BCM_FSBUILD_DIR)/private/include


ALLOWED_INCLUDE_PATHS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  \
                         -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)

ALLOWED_INCLUDE_PATHS += -I$(KERNEL_LINKS_DIR)

ifeq ($(BUILD_RDPACTL),y)
ALLOWED_INCLUDE_PATHS += $(INC_RDP_FLAGS)
endif

ifeq ($(strip $(BUILD_IEEE1905)),y)
ALLOWED_INCLUDE_PATHS += -I$(BUILD_DIR)/userspace/private/apps/ieee1905
endif


# treat all warnings as errors
# CUSTOM_CFLAGS += -Werror -Wfatal-errors # NEW_FORBID_WARNINGS


ifeq ($(strip $(BUILD_RDPACTL)),y)
CFLAGS += -DSUPPORT_RDPA
endif

CFLAGS += $(SSP_TYP_COMPILER_OPTS)
BCM_LD_FLAGS += -L$(INSTALL_DIR)/lib $(SSP_TYP_LIBS)


#
# Implicit rule will make the .c into a .o
# Implicit rule is $(CC) -c $(CPPFLAGS) $(CFLAGS)
# See Section 10.2 of Gnu Make manual
# 


$(LIB): $(OBJS)
	$(AR) -rcs $@ $(OBJS)

#
# Include the rule for making dependency files.
# The '-' in front of the second include suppresses
# error messages when make cannot find the .d files.
# It will just regenerate them.
# See Section 4.14 of Gnu Make.
#

include $(BUILD_DIR)/make.deprules

-include $(REAL_OBJS:.o=.d)
