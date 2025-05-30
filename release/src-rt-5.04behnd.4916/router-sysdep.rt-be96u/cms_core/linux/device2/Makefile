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
             rcl2_lanConfigSecurity.o stl2_lanConfigSecurity.o \
             rcl2_dns.o stl2_dns.o rut2_dns.o rut2_dns6.o \
             rcl2_dslite.o stl2_dslite.o \
             rcl2_ipv6rd.o stl2_ipv6rd.o rut2_iptunnel.o\
             rcl2_dhcpv6.o stl2_dhcpv6.o rcl2_dhcpv6client.o stl2_dhcpv6client.o rcl2_dhcpv6server.o stl2_dhcpv6server.o\
             rut2_dhcpv6.o \
             rut2_util.o \
             rcl2_cert.o stl2_cert.o rut2_cert.o \
             rut2_wan.o rut2_wan6.o \
             rcl2_nat.o stl2_nat.o \
             rut2_iptables.o \
             rcl2_firewall.o stl2_firewall.o \
             rut2_configfile.o \
             rut2_multicast.o \
             rut2_firmware.o \
             rut2_routeradvertisement.o \
             rcl2_macfilter.o stl2_macfilter.o \
             rcl2_ipsec.o stl2_ipsec.o rut2_ipsec.o \
             rcl2_ethlag.o rut2_ethlag.o stl2_ethlag.o \
             rcl2_ethcablediag.o rut2_ethcablediag.o stl2_ethcablediag.o \
             rcl2_vxlan.o stl2_vxlan.o rut2_vxlan.o \
             rcl2_gre.o stl2_gre.o rut2_gre.o \
	     rcl2_afc.o stl2_afc.o

             
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


ifneq ($(strip $(BUILD_MAP)),)
REAL_OBJS += rcl2_map.o stl2_map.o rut2_map.o
endif

ifneq ($(strip $(BUILD_GPON))$(strip $(BUILD_EPON_SDK))$(strip $(BUILD_WANCONF)),)
REAL_OBJS += rcl2_optical.o stl2_optical.o
endif

ifneq ($(strip $(BUILD_ZEBRA)),)
REAL_OBJS += rut2_rip.o
endif

ifneq ($(strip $(BUILD_BCMBUSYBOX_NTPD))$(strip $(BUILD_BRCM_BDK)),)
REAL_OBJS += rcl2_time.o stl2_time.o rut2_time.o
endif

ifneq ($(strip $(BUILD_L2TPAC)),)
REAL_OBJS += rcl2_l2tpac.o stl2_l2tpac.o rut2_l2tpac.o
endif

ifneq ($(strip $(BUILD_L2TPNS)),)
REAL_OBJS += rcl2_l2tpns.o stl2_l2tpns.o rut2_l2tpns.o
endif

ifneq ($(strip $(BUILD_ACCEL_PPTP)),)
REAL_OBJS += rcl2_pptpac.o stl2_pptpac.o rut2_pptpac.o
endif

ifneq ($(strip $(BUILD_ACCEL_PPTPNS)),)
REAL_OBJS += rcl2_pptpns.o stl2_pptpns.o rut2_pptpns.o
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
