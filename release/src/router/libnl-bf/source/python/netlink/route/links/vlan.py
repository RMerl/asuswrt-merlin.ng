#
# Copyright (c) 2011 Thomas Graf <tgraf@suug.ch>
#

"""VLAN network link

"""

import netlink.core as netlink
import netlink.route.capi as capi

class VLANLink(object):
	def __init__(self, link):
        	self._link = link

	###################################################################
	# id
        @netlink.nlattr('link.vlan.id', type=int)
	@property
        def id(self):
        	"""vlan identifier"""
                return capi.rtnl_link_vlan_get_id(self._link)

        @id.setter
        def id(self, value):
        	capi.rtnl_link_vlan_set_id(self._link, int(value))

	###################################################################
	# flags
        @netlink.nlattr('link.vlan.flags', type=str)
	@property
        def flags(self):
        	"""vlan flags"""
        	flags = capi.rtnl_link_vlan_get_flags(self._link)
                return capi.rtnl_link_vlan_flags2str(flags, 256)[0].split(',')

	def _set_flag(self, flag):
                i = capi.rtnl_link_vlan_str2flags(flag[1:])
                if flag[0] == '-':
                        capi.rtnl_link_vlan_unset_flags(self._link, i)
                else:
                        capi.rtnl_link_vlan_set_flags(self._link, i)

	@flags.setter
        def flags(self, value):
        	if type(value) is list:
                        for flag in value:
                                self._set_flag(flag)
                else:
                        self._set_flag(value)

	###################################################################
	# TODO:
	#   - ingress map
	#   - egress map

	def brief(self):
		return 'vlan-id ' + self.id

def init(link):
	link.vlan = VLANLink(link._link)
        return link.vlan
