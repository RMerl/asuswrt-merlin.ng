#
# Copyright (c) 2011 Thomas Graf <tgraf@suug.ch>
#

"""Module providing access to network addresses
"""

from __future__ import absolute_import


__version__ = '1.0'
__all__ = [
    'AddressCache',
    'Address']

import datetime
from .. import core as netlink
from .  import capi as capi
from .  import link as Link
from .. import util as util

class AddressCache(netlink.Cache):
    """Cache containing network addresses"""

    def __init__(self, cache=None):
        if not cache:
            cache = self._alloc_cache_name('route/addr')

        self._protocol = netlink.NETLINK_ROUTE
        self._nl_cache = cache

    def __getitem__(self, key):
        # Using ifindex=0 here implies that the local address itself
        # is unique, otherwise the first occurence is returned.
        return self.lookup(0, key)

    def lookup(self, ifindex, local):
        if type(local) is str:
            local = netlink.AbstractAddress(local)

        addr = capi.rtnl_addr_get(self._nl_cache, ifindex,
                      local._nl_addr)
        if addr is None:
            raise KeyError()

        return Address._from_capi(addr)

    @staticmethod
    def _new_object(obj):
        return Address(obj)

    @staticmethod
    def _new_cache(cache):
        return AddressCache(cache=cache)

class Address(netlink.Object):
    """Network address"""

    def __init__(self, obj=None):
        netlink.Object.__init__(self, 'route/addr', 'address', obj)
        self._rtnl_addr = self._obj2type(self._nl_object)

    @classmethod
    def _from_capi(cls, obj):
        return cls(capi.addr2obj(obj))

    @staticmethod
    def _obj2type(obj):
        return capi.obj2addr(obj)

    def __cmp__(self, other):
        # sort by:
        #    1. network link
        #    2. address family
        #    3. local address (including prefixlen)
        diff = self.ifindex - other.ifindex

        if diff == 0:
            diff = self.family - other.family
            if diff == 0:
                diff = capi.nl_addr_cmp(self.local, other.local)

        return diff

    @staticmethod
    def _new_instance(obj):
        return Address(obj)

    @property
    @netlink.nlattr(type=int, immutable=True, fmt=util.num)
    def ifindex(self):
        """interface index"""
        return capi.rtnl_addr_get_ifindex(self._rtnl_addr)

    @ifindex.setter
    def ifindex(self, value):
        link = Link.resolve(value)
        if not link:
            raise ValueError()

        self.link = link

    @property
    @netlink.nlattr(type=str, fmt=util.string)
    def link(self):
        link = capi.rtnl_addr_get_link(self._rtnl_addr)
        if not link:
            return None

        return Link.Link.from_capi(link)

    @link.setter
    def link(self, value):
        if type(value) is str:
            try:
                value = Link.resolve(value)
            except KeyError:
                raise ValueError()

        capi.rtnl_addr_set_link(self._rtnl_addr, value._rtnl_link)

        # ifindex is immutable but we assume that if _orig does not
        # have an ifindex specified, it was meant to be given here
        if capi.rtnl_addr_get_ifindex(self._orig) == 0:
            capi.rtnl_addr_set_ifindex(self._orig, value.ifindex)

    @property
    @netlink.nlattr(type=str, fmt=util.string)
    def label(self):
        """address label"""
        return capi.rtnl_addr_get_label(self._rtnl_addr)

    @label.setter
    def label(self, value):
        capi.rtnl_addr_set_label(self._rtnl_addr, value)

    @property
    @netlink.nlattr(type=str, fmt=util.string)
    def flags(self):
        """Flags

        Setting this property will *Not* reset flags to value you supply in

        Examples:
        addr.flags = '+xxx' # add xxx flag
        addr.flags = 'xxx'  # exactly the same
        addr.flags = '-xxx' # remove xxx flag
        addr.flags = [ '+xxx', '-yyy' ] # list operation
        """
        flags = capi.rtnl_addr_get_flags(self._rtnl_addr)
        return capi.rtnl_addr_flags2str(flags, 256)[0].split(',')

    def _set_flag(self, flag):
        if flag.startswith('-'):
            i = capi.rtnl_addr_str2flags(flag[1:])
            capi.rtnl_addr_unset_flags(self._rtnl_addr, i)
        elif flag.startswith('+'):
            i = capi.rtnl_addr_str2flags(flag[1:])
            capi.rtnl_addr_set_flags(self._rtnl_addr, i)
        else:
            i = capi.rtnl_addr_str2flags(flag)
            capi.rtnl_addr_set_flags(self._rtnl_addr, i)

    @flags.setter
    def flags(self, value):
        if type(value) is list:
            for flag in value:
                self._set_flag(flag)
        else:
            self._set_flag(value)

    @property
    @netlink.nlattr(type=int, immutable=True, fmt=util.num)
    def family(self):
        """Address family"""
        fam = capi.rtnl_addr_get_family(self._rtnl_addr)
        return netlink.AddressFamily(fam)

    @family.setter
    def family(self, value):
        if not isinstance(value, netlink.AddressFamily):
            value = netlink.AddressFamily(value)

        capi.rtnl_addr_set_family(self._rtnl_addr, int(value))

    @property
    @netlink.nlattr(type=int, fmt=util.num)
    def scope(self):
        """Address scope"""
        scope = capi.rtnl_addr_get_scope(self._rtnl_addr)
        return capi.rtnl_scope2str(scope, 32)[0]

    @scope.setter
    def scope(self, value):
        if type(value) is str:
            value = capi.rtnl_str2scope(value)
        capi.rtnl_addr_set_scope(self._rtnl_addr, value)

    @property
    @netlink.nlattr(type=str, immutable=True, fmt=util.addr)
    def local(self):
        """Local address"""
        a = capi.rtnl_addr_get_local(self._rtnl_addr)
        return netlink.AbstractAddress(a)

    @local.setter
    def local(self, value):
        a = netlink.AbstractAddress(value)
        capi.rtnl_addr_set_local(self._rtnl_addr, a._nl_addr)

        # local is immutable but we assume that if _orig does not
        # have a local address specified, it was meant to be given here
        if capi.rtnl_addr_get_local(self._orig) is None:
            capi.rtnl_addr_set_local(self._orig, a._nl_addr)

    @property
    @netlink.nlattr(type=str, fmt=util.addr)
    def peer(self):
        """Peer address"""
        a = capi.rtnl_addr_get_peer(self._rtnl_addr)
        return netlink.AbstractAddress(a)

    @peer.setter
    def peer(self, value):
        a = netlink.AbstractAddress(value)
        capi.rtnl_addr_set_peer(self._rtnl_addr, a._nl_addr)

    @property
    @netlink.nlattr(type=str, fmt=util.addr)
    def broadcast(self):
        """Broadcast address"""
        a = capi.rtnl_addr_get_broadcast(self._rtnl_addr)
        return netlink.AbstractAddress(a)

    @broadcast.setter
    def broadcast(self, value):
        a = netlink.AbstractAddress(value)
        capi.rtnl_addr_set_broadcast(self._rtnl_addr, a._nl_addr)

    @property
    @netlink.nlattr(type=str, fmt=util.addr)
    def multicast(self):
        """multicast address"""
        a = capi.rtnl_addr_get_multicast(self._rtnl_addr)
        return netlink.AbstractAddress(a)

    @multicast.setter
    def multicast(self, value):
        try:
            a = netlink.AbstractAddress(value)
        except ValueError as err:
            raise AttributeError('multicast', err)

        capi.rtnl_addr_set_multicast(self._rtnl_addr, a._nl_addr)

    @property
    @netlink.nlattr(type=str, fmt=util.addr)
    def anycast(self):
        """anycast address"""
        a = capi.rtnl_addr_get_anycast(self._rtnl_addr)
        return netlink.AbstractAddress(a)

    @anycast.setter
    def anycast(self, value):
        a = netlink.AbstractAddress(value)
        capi.rtnl_addr_set_anycast(self._rtnl_addr, a._nl_addr)

    @property
    @netlink.nlattr(type=int, immutable=True, fmt=util.num)
    def valid_lifetime(self):
        """Valid lifetime"""
        msecs = capi.rtnl_addr_get_valid_lifetime(self._rtnl_addr)
        if msecs == 0xFFFFFFFF:
            return None
        else:
            return datetime.timedelta(seconds=msecs)

    @valid_lifetime.setter
    def valid_lifetime(self, value):
        capi.rtnl_addr_set_valid_lifetime(self._rtnl_addr, int(value))

    @property
    @netlink.nlattr(type=int, immutable=True, fmt=util.num)
    def preferred_lifetime(self):
        """Preferred lifetime"""
        msecs = capi.rtnl_addr_get_preferred_lifetime(self._rtnl_addr)
        if msecs == 0xFFFFFFFF:
            return None
        else:
            return datetime.timedelta(seconds=msecs)

    @preferred_lifetime.setter
    def preferred_lifetime(self, value):
        capi.rtnl_addr_set_preferred_lifetime(self._rtnl_addr, int(value))

    @property
    @netlink.nlattr(type=int, immutable=True, fmt=util.num)
    def create_time(self):
        """Creation time"""
        hsec = capi.rtnl_addr_get_create_time(self._rtnl_addr)
        return datetime.timedelta(milliseconds=10*hsec)

    @property
    @netlink.nlattr(type=int, immutable=True, fmt=util.num)
    def last_update(self):
        """Last update"""
        hsec = capi.rtnl_addr_get_last_update_time(self._rtnl_addr)
        return datetime.timedelta(milliseconds=10*hsec)

    def add(self, socket=None, flags=None):
        if not socket:
            socket = netlink.lookup_socket(netlink.NETLINK_ROUTE)

        if not flags:
            flags = netlink.NLM_F_CREATE

        ret = capi.rtnl_addr_add(socket._sock, self._rtnl_addr, flags)
        if ret < 0:
            raise netlink.KernelError(ret)

    def delete(self, socket, flags=0):
        """Attempt to delete this address in the kernel"""
        ret = capi.rtnl_addr_delete(socket._sock, self._rtnl_addr, flags)
        if ret < 0:
            raise netlink.KernelError(ret)

    ###################################################################
    # private properties
    #
    # Used for formatting output. USE AT OWN RISK
    @property
    def _flags(self):
        return ','.join(self.flags)

    def format(self, details=False, stats=False, nodev=False, indent=''):
        """Return address as formatted text"""
        fmt = util.MyFormatter(self, indent)

        buf = fmt.format('{a|local!b}')

        if not nodev:
            buf += fmt.format(' {a|ifindex}')

        buf += fmt.format(' {a|scope}')

        if self.label:
            buf += fmt.format(' "{a|label}"')

        buf += fmt.format(' <{a|_flags}>')

        if details:
            buf += fmt.nl('\t{t|broadcast} {t|multicast}') \
                 + fmt.nl('\t{t|peer} {t|anycast}')

            if self.valid_lifetime:
                buf += fmt.nl('\t{s|valid-lifetime!k} '\
                       '{a|valid_lifetime}')

            if self.preferred_lifetime:
                buf += fmt.nl('\t{s|preferred-lifetime!k} '\
                       '{a|preferred_lifetime}')

        if stats and (self.create_time or self.last_update):
            buf += self.nl('\t{s|created!k} {a|create_time}'\
                   ' {s|last-updated!k} {a|last_update}')

        return buf
