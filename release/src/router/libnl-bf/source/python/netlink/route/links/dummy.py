#
# Copyright (c) 2011 Thomas Graf <tgraf@suug.ch>
#

"""Dummy

"""

__version__ = "1.0"
__all__ = ['init']

import netlink.core as netlink
import netlink.route.capi as capi

class DummyLink(object):
	def __init__(self, link):
        	self._rtnl_link = link

	def brief(self):
        	return 'dummy'
	
def init(link):
	link.dummy = DummyLink(link._rtnl_link)
        return link.dummy
