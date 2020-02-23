#
# Copyright (c) 2011 Thomas Graf <tgraf@suug.ch>
#

"""Module providing access to network links

This module provides an interface to view configured network links,
modify them and to add and delete virtual network links.

The following is a basic example:
    import netlink.core as netlink
    import netlink.route.link as link

    sock = netlink.Socket()
    sock.connect(netlink.NETLINK_ROUTE)

    cache = link.LinkCache()	# create new empty link cache
    cache.refill(sock)		# fill cache with all configured links
    eth0 = cache['eth0']		# lookup link "eth0"
    print eth0			# print basic configuration

The module contains the following public classes:

  - Link -- Represents a network link. Instances can be created directly
        via the constructor (empty link objects) or via the refill()
        method of a LinkCache.
  - LinkCache -- Derived from netlink.Cache, holds any number of
         network links (Link instances). Main purpose is to keep
         a local list of all network links configured in the
         kernel.

The following public functions exist:
  - get_from_kernel(socket, name)

"""

from __future__ import absolute_import

__version__ = '0.1'
__all__ = [
    'LinkCache',
    'Link',
    'get_from_kernel',
]

import socket
from .. import core as netlink
from .. import capi as core_capi
from .  import capi as capi
from .links  import inet as inet
from .. import util as util

# Link statistics definitions
RX_PACKETS = 0
TX_PACKETS = 1
RX_BYTES = 2
TX_BYTES = 3
RX_ERRORS = 4
TX_ERRORS = 5
RX_DROPPED = 6
TX_DROPPED = 7
RX_COMPRESSED = 8
TX_COMPRESSED = 9
RX_FIFO_ERR = 10
TX_FIFO_ERR = 11
RX_LEN_ERR = 12
RX_OVER_ERR = 13
RX_CRC_ERR = 14
RX_FRAME_ERR = 15
RX_MISSED_ERR = 16
TX_ABORT_ERR = 17
TX_CARRIER_ERR = 18
TX_HBEAT_ERR = 19
TX_WIN_ERR = 20
COLLISIONS = 21
MULTICAST = 22
IP6_INPKTS = 23
IP6_INHDRERRORS = 24
IP6_INTOOBIGERRORS = 25
IP6_INNOROUTES = 26
IP6_INADDRERRORS = 27
IP6_INUNKNOWNPROTOS = 28
IP6_INTRUNCATEDPKTS = 29
IP6_INDISCARDS = 30
IP6_INDELIVERS = 31
IP6_OUTFORWDATAGRAMS = 32
IP6_OUTPKTS = 33
IP6_OUTDISCARDS = 34
IP6_OUTNOROUTES = 35
IP6_REASMTIMEOUT = 36
IP6_REASMREQDS = 37
IP6_REASMOKS = 38
IP6_REASMFAILS = 39
IP6_FRAGOKS = 40
IP6_FRAGFAILS = 41
IP6_FRAGCREATES = 42
IP6_INMCASTPKTS = 43
IP6_OUTMCASTPKTS = 44
IP6_INBCASTPKTS = 45
IP6_OUTBCASTPKTS = 46
IP6_INOCTETS = 47
IP6_OUTOCTETS = 48
IP6_INMCASTOCTETS = 49
IP6_OUTMCASTOCTETS = 50
IP6_INBCASTOCTETS = 51
IP6_OUTBCASTOCTETS = 52
ICMP6_INMSGS = 53
ICMP6_INERRORS = 54
ICMP6_OUTMSGS = 55
ICMP6_OUTERRORS = 56

class LinkCache(netlink.Cache):
    """Cache of network links"""

    def __init__(self, family=socket.AF_UNSPEC, cache=None):
        if not cache:
            cache = self._alloc_cache_name('route/link')

        self._info_module = None
        self._protocol = netlink.NETLINK_ROUTE
        self._nl_cache = cache
        self._set_arg1(family)

    def __getitem__(self, key):
        if type(key) is int:
            link = capi.rtnl_link_get(self._nl_cache, key)
        else:
            link = capi.rtnl_link_get_by_name(self._nl_cache, key)

        if link is None:
            raise KeyError()
        else:
            return Link.from_capi(link)

    @staticmethod
    def _new_object(obj):
        return Link(obj)

    def _new_cache(self, cache):
        return LinkCache(family=self.arg1, cache=cache)

class Link(netlink.Object):
    """Network link"""

    def __init__(self, obj=None):
        netlink.Object.__init__(self, 'route/link', 'link', obj)
        self._rtnl_link = self._obj2type(self._nl_object)

        if self.type:
            self._module_lookup('netlink.route.links.' + self.type)

        self.inet = inet.InetLink(self)
        self.af = {'inet' : self.inet }

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, tb):
        if exc_type is None:
            self.change()
        else:
            return false

    @classmethod
    def from_capi(cls, obj):
        return cls(capi.link2obj(obj))

    @staticmethod
    def _obj2type(obj):
        return capi.obj2link(obj)

    def __cmp__(self, other):
        return self.ifindex - other.ifindex

    @staticmethod
    def _new_instance(obj):
        if not obj:
            raise ValueError()

        return Link(obj)

    @property
    @netlink.nlattr(type=int, immutable=True, fmt=util.num)
    def ifindex(self):
        """interface index"""
        return capi.rtnl_link_get_ifindex(self._rtnl_link)

    @ifindex.setter
    def ifindex(self, value):
        capi.rtnl_link_set_ifindex(self._rtnl_link, int(value))

        # ifindex is immutable but we assume that if _orig does not
        # have an ifindex specified, it was meant to be given here
        if capi.rtnl_link_get_ifindex(self._orig) == 0:
            capi.rtnl_link_set_ifindex(self._orig, int(value))

    @property
    @netlink.nlattr(type=str, fmt=util.bold)
    def name(self):
        """Name of link"""
        return capi.rtnl_link_get_name(self._rtnl_link)

    @name.setter
    def name(self, value):
        capi.rtnl_link_set_name(self._rtnl_link, value)

        # name is the secondary identifier, if _orig does not have
        # the name specified yet, assume it was meant to be specified
        # here. ifindex will always take priority, therefore if ifindex
        # is specified as well, this will be ignored automatically.
        if capi.rtnl_link_get_name(self._orig) is None:
            capi.rtnl_link_set_name(self._orig, value)

    @property
    @netlink.nlattr(type=str, fmt=util.string)
    def flags(self):
        """Flags
        Setting this property will *Not* reset flags to value you supply in
        Examples:
        link.flags = '+xxx' # add xxx flag
        link.flags = 'xxx'  # exactly the same
        link.flags = '-xxx' # remove xxx flag
        link.flags = [ '+xxx', '-yyy' ] # list operation
        """
        flags = capi.rtnl_link_get_flags(self._rtnl_link)
        return capi.rtnl_link_flags2str(flags, 256)[0].split(',')

    def _set_flag(self, flag):
        if flag.startswith('-'):
            i = capi.rtnl_link_str2flags(flag[1:])
            capi.rtnl_link_unset_flags(self._rtnl_link, i)
        elif flag.startswith('+'):
            i = capi.rtnl_link_str2flags(flag[1:])
            capi.rtnl_link_set_flags(self._rtnl_link, i)
        else:
            i = capi.rtnl_link_str2flags(flag)
            capi.rtnl_link_set_flags(self._rtnl_link, i)

    @flags.setter
    def flags(self, value):
        if not (type(value) is str):
            for flag in value:
                self._set_flag(flag)
        else:
            self._set_flag(value)

    @property
    @netlink.nlattr(type=int, fmt=util.num)
    def mtu(self):
        """Maximum Transmission Unit"""
        return capi.rtnl_link_get_mtu(self._rtnl_link)

    @mtu.setter
    def mtu(self, value):
        capi.rtnl_link_set_mtu(self._rtnl_link, int(value))

    @property
    @netlink.nlattr(type=int, immutable=True, fmt=util.num)
    def family(self):
        """Address family"""
        return capi.rtnl_link_get_family(self._rtnl_link)

    @family.setter
    def family(self, value):
        capi.rtnl_link_set_family(self._rtnl_link, value)

    @property
    @netlink.nlattr(type=str, fmt=util.addr)
    def address(self):
        """Hardware address (MAC address)"""
        a = capi.rtnl_link_get_addr(self._rtnl_link)
        return netlink.AbstractAddress(a)

    @address.setter
    def address(self, value):
        capi.rtnl_link_set_addr(self._rtnl_link, value._addr)

    @property
    @netlink.nlattr(type=str, fmt=util.addr)
    def broadcast(self):
        """Hardware broadcast address"""
        a = capi.rtnl_link_get_broadcast(self._rtnl_link)
        return netlink.AbstractAddress(a)

    @broadcast.setter
    def broadcast(self, value):
        capi.rtnl_link_set_broadcast(self._rtnl_link, value._addr)

    @property
    @netlink.nlattr(type=str, immutable=True, fmt=util.string)
    def qdisc(self):
        """Name of qdisc (cannot be changed)"""
        return capi.rtnl_link_get_qdisc(self._rtnl_link)

    @qdisc.setter
    def qdisc(self, value):
        capi.rtnl_link_set_qdisc(self._rtnl_link, value)

    @property
    @netlink.nlattr(type=int, fmt=util.num)
    def txqlen(self):
        """Length of transmit queue"""
        return capi.rtnl_link_get_txqlen(self._rtnl_link)

    @txqlen.setter
    def txqlen(self, value):
        capi.rtnl_link_set_txqlen(self._rtnl_link, int(value))

    @property
    @netlink.nlattr(type=str, immutable=True, fmt=util.string)
    def arptype(self):
        """Type of link (cannot be changed)"""
        type_ = capi.rtnl_link_get_arptype(self._rtnl_link)
        return core_capi.nl_llproto2str(type_, 64)[0]

    @arptype.setter
    def arptype(self, value):
        i = core_capi.nl_str2llproto(value)
        capi.rtnl_link_set_arptype(self._rtnl_link, i)

    @property
    @netlink.nlattr(type=str, immutable=True, fmt=util.string, title='state')
    def operstate(self):
        """Operational status"""
        operstate = capi.rtnl_link_get_operstate(self._rtnl_link)
        return capi.rtnl_link_operstate2str(operstate, 32)[0]

    @operstate.setter
    def operstate(self, value):
        i = capi.rtnl_link_str2operstate(value)
        capi.rtnl_link_set_operstate(self._rtnl_link, i)

    @property
    @netlink.nlattr(type=str, immutable=True, fmt=util.string)
    def mode(self):
        """Link mode"""
        mode = capi.rtnl_link_get_linkmode(self._rtnl_link)
        return capi.rtnl_link_mode2str(mode, 32)[0]

    @mode.setter
    def mode(self, value):
        i = capi.rtnl_link_str2mode(value)
        capi.rtnl_link_set_linkmode(self._rtnl_link, i)

    @property
    @netlink.nlattr(type=str, fmt=util.string)
    def alias(self):
        """Interface alias (SNMP)"""
        return capi.rtnl_link_get_ifalias(self._rtnl_link)

    @alias.setter
    def alias(self, value):
        capi.rtnl_link_set_ifalias(self._rtnl_link, value)

    @property
    @netlink.nlattr(type=str, fmt=util.string)
    def type(self):
        """Link type"""
        return capi.rtnl_link_get_type(self._rtnl_link)

    @type.setter
    def type(self, value):
        if capi.rtnl_link_set_type(self._rtnl_link, value) < 0:
            raise NameError('unknown info type')

        self._module_lookup('netlink.route.links.' + value)

    def get_stat(self, stat):
        """Retrieve statistical information"""
        if type(stat) is str:
            stat = capi.rtnl_link_str2stat(stat)
            if stat < 0:
                raise NameError('unknown name of statistic')

        return capi.rtnl_link_get_stat(self._rtnl_link, stat)

    def enslave(self, slave, sock=None):
        if not sock:
            sock = netlink.lookup_socket(netlink.NETLINK_ROUTE)

        return capi.rtnl_link_enslave(sock._sock, self._rtnl_link, slave._rtnl_link)

    def release(self, slave, sock=None):
        if not sock:
            sock = netlink.lookup_socket(netlink.NETLINK_ROUTE)

        return capi.rtnl_link_release(sock._sock, self._rtnl_link, slave._rtnl_link)

    def add(self, sock=None, flags=None):
        if not sock:
            sock = netlink.lookup_socket(netlink.NETLINK_ROUTE)

        if not flags:
            flags = netlink.NLM_F_CREATE

        ret = capi.rtnl_link_add(sock._sock, self._rtnl_link, flags)
        if ret < 0:
            raise netlink.KernelError(ret)

    def change(self, sock=None, flags=0):
        """Commit changes made to the link object"""
        if sock is None:
            sock = netlink.lookup_socket(netlink.NETLINK_ROUTE)

        if not self._orig:
            raise netlink.NetlinkError('Original link not available')
        ret = capi.rtnl_link_change(sock._sock, self._orig, self._rtnl_link, flags)
        if ret < 0:
            raise netlink.KernelError(ret)

    def delete(self, sock=None):
        """Attempt to delete this link in the kernel"""
        if sock is None:
            sock = netlink.lookup_socket(netlink.NETLINK_ROUTE)

        ret = capi.rtnl_link_delete(sock._sock, self._rtnl_link)
        if ret < 0:
            raise netlink.KernelError(ret)

    ###################################################################
    # private properties
    #
    # Used for formatting output. USE AT OWN RISK
    @property
    def _state(self):
        if 'up' in self.flags:
            buf = util.good('up')
            if 'lowerup' not in self.flags:
                buf += ' ' + util.bad('no-carrier')
        else:
            buf = util.bad('down')
        return buf

    @property
    def _brief(self):
        return self._module_brief() + self._foreach_af('brief')

    @property
    def _flags(self):
        ignore = [
            'up',
            'running',
            'lowerup',
        ]
        return ','.join([flag for flag in self.flags if flag not in ignore])

    def _foreach_af(self, name, args=None):
        buf = ''
        for af in self.af:
            try:
                func = getattr(self.af[af], name)
                s = str(func(args))
                if len(s) > 0:
                    buf += ' ' + s
            except AttributeError:
                pass
        return buf

    def format(self, details=False, stats=False, indent=''):
        """Return link as formatted text"""
        fmt = util.MyFormatter(self, indent)

        buf = fmt.format('{a|ifindex} {a|name} {a|arptype} {a|address} '\
                 '{a|_state} <{a|_flags}> {a|_brief}')

        if details:
            buf += fmt.nl('\t{t|mtu} {t|txqlen} {t|weight} '\
                      '{t|qdisc} {t|operstate}')
            buf += fmt.nl('\t{t|broadcast} {t|alias}')

            buf += self._foreach_af('details', fmt)

        if stats:
            l = [['Packets', RX_PACKETS, TX_PACKETS],
                 ['Bytes', RX_BYTES, TX_BYTES],
                 ['Errors', RX_ERRORS, TX_ERRORS],
                 ['Dropped', RX_DROPPED, TX_DROPPED],
                 ['Compressed', RX_COMPRESSED, TX_COMPRESSED],
                 ['FIFO Errors', RX_FIFO_ERR, TX_FIFO_ERR],
                 ['Length Errors', RX_LEN_ERR, None],
                 ['Over Errors', RX_OVER_ERR, None],
                 ['CRC Errors', RX_CRC_ERR, None],
                 ['Frame Errors', RX_FRAME_ERR, None],
                 ['Missed Errors', RX_MISSED_ERR, None],
                 ['Abort Errors', None, TX_ABORT_ERR],
                 ['Carrier Errors', None, TX_CARRIER_ERR],
                 ['Heartbeat Errors', None, TX_HBEAT_ERR],
                 ['Window Errors', None, TX_WIN_ERR],
                 ['Collisions', None, COLLISIONS],
                 ['Multicast', None, MULTICAST],
                 ['', None, None],
                 ['Ipv6:', None, None],
                 ['Packets', IP6_INPKTS, IP6_OUTPKTS],
                 ['Bytes', IP6_INOCTETS, IP6_OUTOCTETS],
                 ['Discards', IP6_INDISCARDS, IP6_OUTDISCARDS],
                 ['Multicast Packets', IP6_INMCASTPKTS, IP6_OUTMCASTPKTS],
                 ['Multicast Bytes', IP6_INMCASTOCTETS, IP6_OUTMCASTOCTETS],
                 ['Broadcast Packets', IP6_INBCASTPKTS, IP6_OUTBCASTPKTS],
                 ['Broadcast Bytes', IP6_INBCASTOCTETS, IP6_OUTBCASTOCTETS],
                 ['Delivers', IP6_INDELIVERS, None],
                 ['Forwarded', None, IP6_OUTFORWDATAGRAMS],
                 ['No Routes', IP6_INNOROUTES, IP6_OUTNOROUTES],
                 ['Header Errors', IP6_INHDRERRORS, None],
                 ['Too Big Errors', IP6_INTOOBIGERRORS, None],
                 ['Address Errors', IP6_INADDRERRORS, None],
                 ['Unknown Protocol', IP6_INUNKNOWNPROTOS, None],
                 ['Truncated Packets', IP6_INTRUNCATEDPKTS, None],
                 ['Reasm Timeouts', IP6_REASMTIMEOUT, None],
                 ['Reasm Requests', IP6_REASMREQDS, None],
                 ['Reasm Failures', IP6_REASMFAILS, None],
                 ['Reasm OK', IP6_REASMOKS, None],
                 ['Frag Created', None, IP6_FRAGCREATES],
                 ['Frag Failures', None, IP6_FRAGFAILS],
                 ['Frag OK', None, IP6_FRAGOKS],
                 ['', None, None],
                 ['ICMPv6:', None, None],
                 ['Messages', ICMP6_INMSGS, ICMP6_OUTMSGS],
                 ['Errors', ICMP6_INERRORS, ICMP6_OUTERRORS]]

            buf += '\n\t%s%s%s%s\n' % (33 * ' ', util.title('RX'),
                           15 * ' ', util.title('TX'))

            for row in l:
                row[0] = util.kw(row[0])
                row[1] = self.get_stat(row[1]) if row[1] else ''
                row[2] = self.get_stat(row[2]) if row[2] else ''
                buf += '\t{0[0]:27} {0[1]:>16} {0[2]:>16}\n'.format(row)

            buf += self._foreach_af('stats')

        return buf

def get(name, sock=None):
    """Lookup Link object directly from kernel"""
    if not name:
        raise ValueError()

    if not sock:
        sock = netlink.lookup_socket(netlink.NETLINK_ROUTE)

    link = capi.get_from_kernel(sock._sock, 0, name)
    if not link:
        return None

    return Link.from_capi(link)

_link_cache = LinkCache()

def resolve(name):
    _link_cache.refill()
    return _link_cache[name]
