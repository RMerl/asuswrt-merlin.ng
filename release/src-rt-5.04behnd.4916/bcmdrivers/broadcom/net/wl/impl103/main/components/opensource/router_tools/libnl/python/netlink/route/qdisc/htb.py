#
# Copyright (c) 2011 Thomas Graf <tgraf@suug.ch>
#

"""HTB qdisc

"""

from __future__ import absolute_import

from ... import core as netlink
from ... import util as util
from ..  import capi as capi
from ..  import tc as tc

class HTBQdisc(object):
    def __init__(self, qdisc):
        self._qdisc = qdisc

    @property
    @netlink.nlattr(type=int)
    def default_class(self):
        return tc.Handle(capi.rtnl_htb_get_defcls(self._qdisc._rtnl_qdisc))

    @default_class.setter
    def default_class(self, value):
        capi.rtnl_htb_set_defcls(self._qdisc._rtnl_qdisc, int(value))

    @property
    @netlink.nlattr('r2q', type=int)
    def r2q(self):
        return capi.rtnl_htb_get_rate2quantum(self._qdisc._rtnl_qdisc)

    @r2q.setter
    def r2q(self, value):
        capi.rtnl_htb_get_rate2quantum(self._qdisc._rtnl_qdisc,
                           int(value))

    def brief(self):
        fmt = util.MyFormatter(self)

        ret = ' {s|default-class!k} {a|default_class}'

        if self.r2q:
            ret += ' {s|r2q!k} {a|r2q}'

        return fmt.format(ret)

class HTBClass(object):
    def __init__(self, cl):
        self._class = cl

    @property
    @netlink.nlattr(type=str)
    def rate(self):
        rate = capi.rtnl_htb_get_rate(self._class._rtnl_class)
        return util.Rate(rate)

    @rate.setter
    def rate(self, value):
        capi.rtnl_htb_set_rate(self._class._rtnl_class, int(value))

    @property
    @netlink.nlattr(type=str)
    def ceil(self):
        ceil = capi.rtnl_htb_get_ceil(self._class._rtnl_class)
        return util.Rate(ceil)

    @ceil.setter
    def ceil(self, value):
        capi.rtnl_htb_set_ceil(self._class._rtnl_class, int(value))

    @property
    @netlink.nlattr(type=str)
    def burst(self):
        burst = capi.rtnl_htb_get_rbuffer(self._class._rtnl_class)
        return util.Size(burst)

    @burst.setter
    def burst(self, value):
        capi.rtnl_htb_set_rbuffer(self._class._rtnl_class, int(value))

    @property
    @netlink.nlattr(type=str)
    def ceil_burst(self):
        burst = capi.rtnl_htb_get_cbuffer(self._class._rtnl_class)
        return util.Size(burst)

    @ceil_burst.setter
    def ceil_burst(self, value):
        capi.rtnl_htb_set_cbuffer(self._class._rtnl_class, int(value))

    @property
    @netlink.nlattr(type=int)
    def prio(self):
        return capi.rtnl_htb_get_prio(self._class._rtnl_class)

    @prio.setter
    def prio(self, value):
        capi.rtnl_htb_set_prio(self._class._rtnl_class, int(value))

    @property
    @netlink.nlattr(type=int)
    def quantum(self):
        return capi.rtnl_htb_get_quantum(self._class._rtnl_class)

    @quantum.setter
    def quantum(self, value):
        capi.rtnl_htb_set_quantum(self._class._rtnl_class, int(value))

    @property
    @netlink.nlattr(type=int)
    def level(self):
        return capi.rtnl_htb_get_level(self._class._rtnl_class)

    @level.setter
    def level(self, value):
        capi.rtnl_htb_set_level(self._class._rtnl_class, int(value))

    def brief(self):
        fmt = util.MyFormatter(self)

        ret = ' {t|prio} {t|rate}'

        if self.rate != self.ceil:
            ret += ' {s|borrow-up-to!k} {a|ceil}'

        ret += ' {t|burst}'

        return fmt.format(ret)

    def details(self):
        fmt = util.MyFormatter(self)

        return fmt.nl('\t{t|level} {t|quantum}')

def init_qdisc(qdisc):
    qdisc.htb = HTBQdisc(qdisc)
    return qdisc.htb

def init_class(cl):
    cl.htb = HTBClass(cl)
    return cl.htb

#extern void rtnl_htb_set_quantum(struct rtnl_class *, uint32_t quantum);
