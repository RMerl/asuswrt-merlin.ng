# SPDX-License-Identifier: GPL-2.0
#
# Makefile for Open vSwitch.
#

obj-$(CONFIG_OPENVSWITCH) += openvswitch.o

openvswitch-y := \
	actions.o \
	datapath.o \
	dp_notify.o \
	flow.o \
	flow_netlink.o \
	flow_table.o \
	meter.o \
	openvswitch_trace.o \
	vport.o \
	vport-internal_dev.o \
	vport-netdev.o

ifneq ($(CONFIG_NF_CONNTRACK),)
openvswitch-y += conntrack.o
endif

ifdef BCM_KF # defined(CONFIG_BCM_KF_OVS)
openvswitch-y += ../../../bcmkernel/net/openvswitch/ovs_bcm_blog.o
CFLAGS_datapath.o = -I../bcmkernel/net/openvswitch
CFLAGS_flow.o = -I../bcmkernel/net/openvswitch
CFLAGS_flow_table.o = -I../bcmkernel/net/openvswitch
CFLAGS_flow_netlink.o = -I../bcmkernel/net/openvswitch
endif # BCM_KF # defined(CONFIG_BCM_KF_OVS)

obj-$(CONFIG_OPENVSWITCH_VXLAN)+= vport-vxlan.o
obj-$(CONFIG_OPENVSWITCH_GENEVE)+= vport-geneve.o
obj-$(CONFIG_OPENVSWITCH_GRE)	+= vport-gre.o

CFLAGS_openvswitch_trace.o = -I$(src)
