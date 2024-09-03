#
# Copyright (c) 2011 Thomas Graf <tgraf@suug.ch>
#

"""VLAN network link

"""

from __future__ import absolute_import


from ... import core as netlink
from ..  import capi as capi
class VLANLink(object):
    def __init__(self, link):
        self._link = link

    @property
    @netlink.nlattr(type=int)
    def id(self):
        """vlan identifier"""
        return capi.rtnl_link_vlan_get_id(self._link)

    @id.setter
    def id(self, value):
        capi.rtnl_link_vlan_set_id(self._link, int(value))

    @property
    @netlink.nlattr(type=str)
    def flags(self):
        """ VLAN flags
        Setting this property will *Not* reset flags to value you supply in
        Examples:
        link.flags = '+xxx' # add xxx flag
        link.flags = 'xxx'  # exactly the same
        link.flags = '-xxx' # remove xxx flag
        link.flags = [ '+xxx', '-yyy' ] # list operation
        """
        flags = capi.rtnl_link_vlan_get_flags(self._link)
        return capi.rtnl_link_vlan_flags2str(flags, 256)[0].split(',')

    def _set_flag(self, flag):
        if flag.startswith('-'):
            i = capi.rtnl_link_vlan_str2flags(flag[1:])
            capi.rtnl_link_vlan_unset_flags(self._link, i)
        elif flag.startswith('+'):
            i = capi.rtnl_link_vlan_str2flags(flag[1:])
            capi.rtnl_link_vlan_set_flags(self._link, i)
        else:
            i = capi.rtnl_link_vlan_str2flags(flag)
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
        return 'vlan-id {0}'.format(self.id)

def init(link):
    link.vlan = VLANLink(link._rtnl_link)
    return link.vlan
