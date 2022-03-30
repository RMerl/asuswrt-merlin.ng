subdir-ccflags-y				+= -I$(KERNEL_DIR)/net/bridge \
						   -I$(KERNEL_DIR)/net/l2tp

ifneq ($(CONFIG_BCM_BPM),)
obj-y						+= net/core/gbpm.o
endif
ifneq ($(CONFIG_BCM_DPI),)
obj-y						+= net/core/dpi.o
endif
obj-$(CONFIG_BCM_SGS)				+= net/core/sgs.o
ifneq ($(CONFIG_BCM_INGQOS),)
obj-y						+= net/core/iqos.o
endif
obj-$(CONFIG_BCM_KF_IPV6)			+= net/ipv6/bcm_addrconf.o
obj-$(CONFIG_BCM_KF_NBUFF)			+= net/core/bcm_skbuff.o \
						   net/core/nbuff.o
obj-$(CONFIG_BCM_LOG)				+= src/bcm_log.o
obj-$(CONFIG_BCM_KF_NETDEV_EXT)		+= net/core/bcm_netdev_path.o \
						   net/core/bcm_netdevice.o
obj-$(CONFIG_BCM_FEED_RING_TRACKER)	+= net/core/tracker.o
ifneq ($(CONFIG_BRIDGE),)
obj-$(CONFIG_BCM_KF_NETDEV_EXT)		+= net/core/bcm_sysfs.o
endif
ifneq ($(CONFIG_BCM_MAP),)
obj-y						+= net/ipv4/bcm_icmp.o
endif
obj-$(CONFIG_BCM_SKB_FREE_THREAD)		+= net/core/bcm_skb_free.o
obj-$(CONFIG_BCM_SW_GSO)			+= net/core/bcm_sw_gso.o
obj-$(CONFIG_BLOG)				+= net/core/blog.o \
						   net/core/blog_tunnels.o \
						   net/core/blog_rule.o \
						   net/core/vlanctl_bind.o
obj-$(CONFIG_BLOG_LOCALIN_TCP)			+= net/core/bcm_tcp_accel.o
obj-$(CONFIG_BRIDGE)				+= net/bridge/bcm_br_fdb.o \
						   net/bridge/bcm_br_hooks.o \
						   net/bridge/bcm_br_hooks_ieee1905.o \
						   net/bridge/bcm_br_hooks_mcast.o \
						   net/bridge/bcm_br_hooks_local_switching.o \
						   net/bridge/bcm_br_hooks_maclimit.o
obj-$(CONFIG_BRIDGE_EBT_BLOG)			+= net/netfilter/ebt_blog.o
obj-$(CONFIG_BRIDGE_EBT_FTOS_T)			+= net/netfilter/ebt_ftos.o
obj-$(CONFIG_BRIDGE_EBT_IP)				+= net/netfilter/ebt_ip_extend.o
obj-$(CONFIG_BRIDGE_EBT_IP6)			+= net/netfilter/ebt_ip6_extend.o
obj-$(CONFIG_BRIDGE_EBT_QOS_MAP)		+= net/netfilter/ebt_qos_map.o
obj-$(CONFIG_BRIDGE_EBT_SKBVLAN)		+= net/netfilter/ebt_skbvlan_m.o
obj-$(CONFIG_BRIDGE_EBT_SKIPLOG_T)		+= net/netfilter/ebt_skiplog.o
obj-$(CONFIG_BRIDGE_EBT_TIME)			+= net/netfilter/ebt_time.o
obj-$(CONFIG_BRIDGE_EBT_U32)			+= net/netfilter/ebt_u32.o
obj-$(CONFIG_BRIDGE_EBT_VTAG)			+= net/netfilter/ebt_vtag.o
obj-$(CONFIG_BRIDGE_EBT_WMM_MARK)		+= net/netfilter/ebt_wmm_mark.o
obj-$(CONFIG_BRIDGE_EBT_REJECT)			+= net/netfilter/ebt_reject.o
ifneq ($(CONFIG_NF_CONNTRACK),)
obj-y						+= net/netfilter/bcm_nf_conntrack.o
endif
ifneq ($(CONFIG_NF_CT_NETLINK),)
obj-y						+= net/netfilter/nf_conntrack_netlink.o
endif
obj-$(CONFIG_NF_CONNTRACK_IPSEC)		+= net/netfilter/nf_conntrack_ipsec.o
obj-$(CONFIG_NF_CONNTRACK_RTSP)			+= net/netfilter/nf_conntrack_rtsp.o
obj-$(CONFIG_NF_DYNDSCP)			+= net/netfilter/nf_dyndscp.o
obj-$(CONFIG_NF_NAT_IPSEC)			+= net/netfilter/nf_nat_ipsec.o
obj-$(CONFIG_NF_NAT_PROTO_ESP)			+= net/netfilter/nf_nat_proto_esp.o
obj-$(CONFIG_NF_NAT_PT)				+= net/netfilter/nf_nat_pt.o
obj-$(CONFIG_NF_NAT_RTSP)			+= net/netfilter/nf_nat_rtsp.o
obj-$(CONFIG_NETFILTER_XT_MATCH_BLOG)		+= net/netfilter/xt_blog.o
obj-$(CONFIG_NETFILTER_XT_MATCH_FLOWLABEL)	+= net/netfilter/xt_flowlabel.o
obj-$(CONFIG_NETFILTER_XT_MATCH_MAC)		+= net/netfilter/xt_mac_extend.o
obj-$(CONFIG_NETFILTER_XT_TARGET_SKIPLOG)	+= net/netfilter/xt_SKIPLOG.o
