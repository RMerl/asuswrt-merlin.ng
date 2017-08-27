#
# Copyright (c) 2011 Thomas Graf <tgraf@suug.ch>
#

__all__ = [
	'TcCache',
	'Tc',
	'QdiscCache',
	'Qdisc',
	'TcClassCache',
	'TcClass']

import socket
import sys
import netlink.core as netlink
import netlink.capi as core_capi
import netlink.route.capi as capi
import netlink.util as util

import netlink.route.link as Link

TC_PACKETS = 0
TC_BYTES = 1
TC_RATE_BPS = 2
TC_RATE_PPS = 3
TC_QLEN = 4
TC_BACKLOG = 5
TC_DROPS = 6
TC_REQUEUES = 7
TC_OVERLIMITS = 9

TC_H_ROOT = 0xFFFFFFFF
TC_H_INGRESS = 0xFFFFFFF1

STAT_PACKETS = 0
STAT_BYTES = 1
STAT_RATE_BPS = 2
STAT_RATE_PPS = 3
STAT_QLEN = 4
STAT_BACKLOG = 5
STAT_DROPS = 6
STAT_REQUEUES = 7
STAT_OVERLIMITS = 8
STAT_MAX = STAT_OVERLIMITS


###########################################################################
# Handle
class Handle(object):
	""" Traffic control handle

	Representation of a traffic control handle which uniquely identifies
	each traffic control object in its link namespace.

	handle = tc.Handle('10:20')
	handle = tc.handle('root')
	print int(handle)
	print str(handle)
	"""
	def __init__(self, val=None):
        	if type(val) is str:
                        val = capi.tc_str2handle(val)
        	elif not val:
                        val = 0

        	self._val = int(val)

	def __cmp__(self, other):
		if other is None:
			other = 0

		if isinstance(other, Handle):
			return int(self) - int(other)
		elif isinstance(other, int):
			return int(self) - other
		else:
			raise TypeError()

	def __int__(self):
        	return self._val

        def __str__(self):
        	return capi.rtnl_tc_handle2str(self._val, 64)[0]

	def isroot(self):
        	return self._val == TC_H_ROOT or self._val == TC_H_INGRESS

###########################################################################
# TC Cache
class TcCache(netlink.Cache):
	"""Cache of traffic control object"""

	def __getitem__(self, key):
        	raise NotImplementedError()

###########################################################################
# Tc Object
class Tc(netlink.Object):
	def __cmp__(self, other):
        	diff = self.ifindex - other.ifindex
		if diff == 0:
			diff = int(self.handle) - int(other.handle)
		return diff

        def _tc_module_lookup(self):
                self._module_lookup(self._module_path + self.kind,
                		    'init_' + self._name)

	@property
	def root(self):
		"""True if tc object is a root object"""
        	return self.parent.isroot()

	#####################################################################
	# ifindex
        @property
        def ifindex(self):
                """interface index"""
                return capi.rtnl_tc_get_ifindex(self._rtnl_tc)

	@ifindex.setter
        def ifindex(self, value):
                capi.rtnl_tc_set_ifindex(self._rtnl_tc, int(value))

	#####################################################################
	# link
        @property
        def link(self):
		link = capi.rtnl_tc_get_link(self._rtnl_tc)
                if not link:
                        return None

                return Link.Link.from_capi(link)

        @link.setter
        def link(self, value):
        	capi.rtnl_tc_set_link(self._rtnl_tc, value._link)

	#####################################################################
	# mtu
        @property
        def mtu(self):
                return capi.rtnl_tc_get_mtu(self._rtnl_tc)

	@mtu.setter
        def mtu(self, value):
                capi.rtnl_tc_set_mtu(self._rtnl_tc, int(value))

	#####################################################################
	# mpu
        @property
        def mpu(self):
                return capi.rtnl_tc_get_mpu(self._rtnl_tc)

	@mpu.setter
        def mpu(self, value):
                capi.rtnl_tc_set_mpu(self._rtnl_tc, int(value))

	#####################################################################
	# overhead
        @property
        def overhead(self):
                return capi.rtnl_tc_get_overhead(self._rtnl_tc)

	@overhead.setter
        def overhead(self, value):
                capi.rtnl_tc_set_overhead(self._rtnl_tc, int(value))

	#####################################################################
	# linktype
        @property
        def linktype(self):
                return capi.rtnl_tc_get_linktype(self._rtnl_tc)

	@linktype.setter
        def linktype(self, value):
                capi.rtnl_tc_set_linktype(self._rtnl_tc, int(value))

	#####################################################################
	# handle
        @property
        def handle(self):
                return Handle(capi.rtnl_tc_get_handle(self._rtnl_tc))

	@handle.setter
        def handle(self, value):
                capi.rtnl_tc_set_handle(self._rtnl_tc, int(value))

	#####################################################################
	# parent
        @property
        def parent(self):
                return Handle(capi.rtnl_tc_get_parent(self._rtnl_tc))

	@parent.setter
        def parent(self, value):
                capi.rtnl_tc_set_parent(self._rtnl_tc, int(value))

	#####################################################################
	# kind
        @property
        def kind(self):
                return capi.rtnl_tc_get_kind(self._rtnl_tc)

	@kind.setter
        def kind(self, value):
                capi.rtnl_tc_set_kind(self._rtnl_tc, value)
                self._tc_module_lookup()

	def get_stat(self, id):
        	return capi.rtnl_tc_get_stat(self._rtnl_tc, id)

        @property
        def _dev(self):
        	buf = util.kw('dev') + ' '

                if self.link:
			return buf + util.string(self.link.name)
                else:
			return buf + util.num(self.ifindex)

	def brief(self, title, nodev=False, noparent=False):
		ret = title + ' {a|kind} {a|handle}'

		if not nodev:
			ret += ' {a|_dev}'

		if not noparent:
			ret += ' {t|parent}'

		return ret + self._module_brief()

	def details(self):
		return '{t|mtu} {t|mpu} {t|overhead} {t|linktype}'

	@property
        def packets(self):
        	return self.get_stat(STAT_PACKETS)

	@property
        def bytes(self):
        	return self.get_stat(STAT_BYTES)

        @property
        def qlen(self):
        	return self.get_stat(STAT_QLEN)

        def stats(self, fmt):
        	return fmt.nl('{t|packets} {t|bytes} {t|qlen}')
        	
###########################################################################
# Queueing discipline cache
class QdiscCache(netlink.Cache):
	"""Cache of qdiscs"""

	def __init__(self, cache=None):
        	if not cache:
                        cache = self._alloc_cache_name("route/qdisc")

		self._protocol = netlink.NETLINK_ROUTE
                self._nl_cache = cache

#	def __getitem__(self, key):
#        	if type(key) is int:
#                        link = capi.rtnl_link_get(self._this, key)
#                elif type(key) is str:
#                        link = capi.rtnl_link_get_by_name(self._this, key)
#
#		if qdisc is None:
#                        raise KeyError()
#		else:
#                        return Qdisc._from_capi(capi.qdisc2obj(qdisc))

	def _new_object(self, obj):
        	return Qdisc(obj)

	def _new_cache(self, cache):
		return QdiscCache(cache=cache)

###########################################################################
# Qdisc Object
class Qdisc(Tc):
	"""Queueing discipline"""

	def __init__(self, obj=None):
		netlink.Object.__init__(self, "route/qdisc", "qdisc", obj)
                self._module_path = 'netlink.route.qdisc.'
		self._rtnl_qdisc = self._obj2type(self._nl_object)
		self._rtnl_tc = capi.obj2tc(self._nl_object)

		netlink.add_attr('qdisc.handle', fmt=util.handle)
		netlink.add_attr('qdisc.parent', fmt=util.handle)
		netlink.add_attr('qdisc.kind', fmt=util.bold)

                if self.kind:
                        self._tc_module_lookup()

	@classmethod
        def from_capi(cls, obj):
        	return cls(capi.qdisc2obj(obj))

	def _obj2type(self, obj):
        	return capi.obj2qdisc(obj)

	def _new_instance(self, obj):
		if not obj:
			raise ValueError()

                return Qdisc(obj)

	@property
	def childs(self):
		ret = []

		if int(self.handle):
			ret += get_cls(self.ifindex, parent=self.handle)

			if self.root:
				ret += get_class(self.ifindex, parent=TC_H_ROOT)

			ret += get_class(self.ifindex, parent=self.handle)

		return ret

#	#####################################################################
#	# add()
#	def add(self, socket, flags=None):
#        	if not flags:
#                        flags = netlink.NLM_F_CREATE
#
#		ret = capi.rtnl_link_add(socket._sock, self._link, flags)
#		if ret < 0:
#			raise netlink.KernelError(ret)
#
#	#####################################################################
#	# change()
#	def change(self, socket, flags=0):
#		"""Commit changes made to the link object"""
#		if not self._orig:
#			raise NetlinkError("Original link not available")
#        	ret = capi.rtnl_link_change(socket._sock, self._orig, self._link, flags)
#                if ret < 0:
#                        raise netlink.KernelError(ret)
#
#	#####################################################################
#	# delete()
#	def delete(self, socket):
#		"""Attempt to delete this link in the kernel"""
#        	ret = capi.rtnl_link_delete(socket._sock, self._link)
#                if ret < 0:
#                        raise netlink.KernelError(ret)

	###################################################################
	#
	# format(details=False, stats=False)
	#
	def format(self, details=False, stats=False, nodev=False,
		   noparent=False, indent=''):
        	"""Return qdisc as formatted text"""
		fmt = util.MyFormatter(self, indent)

		buf = fmt.format(self.brief('qdisc', nodev, noparent))
		
		if details:
			buf += fmt.nl('\t' + self.details())

                if stats:
                        buf += self.stats(fmt)
                	
#		if stats:
#			l = [['Packets', RX_PACKETS, TX_PACKETS],
#			     ['Bytes', RX_BYTES, TX_BYTES],
#			     ['Errors', RX_ERRORS, TX_ERRORS],
#			     ['Dropped', RX_DROPPED, TX_DROPPED],
#			     ['Compressed', RX_COMPRESSED, TX_COMPRESSED],
#			     ['FIFO Errors', RX_FIFO_ERR, TX_FIFO_ERR],
#			     ['Length Errors', RX_LEN_ERR, None],
#			     ['Over Errors', RX_OVER_ERR, None],
#			     ['CRC Errors', RX_CRC_ERR, None],
#			     ['Frame Errors', RX_FRAME_ERR, None],
#			     ['Missed Errors', RX_MISSED_ERR, None],
#			     ['Abort Errors', None, TX_ABORT_ERR],
#			     ['Carrier Errors', None, TX_CARRIER_ERR],
#			     ['Heartbeat Errors', None, TX_HBEAT_ERR],
#			     ['Window Errors', None, TX_WIN_ERR],
#			     ['Collisions', None, COLLISIONS],
#			     ['Multicast', None, MULTICAST],
#			     ['', None, None],
#			     ['Ipv6:', None, None],
#			     ['Packets', IP6_INPKTS, IP6_OUTPKTS],
#			     ['Bytes', IP6_INOCTETS, IP6_OUTOCTETS],
#			     ['Discards', IP6_INDISCARDS, IP6_OUTDISCARDS],
#			     ['Multicast Packets', IP6_INMCASTPKTS, IP6_OUTMCASTPKTS],
#			     ['Multicast Bytes', IP6_INMCASTOCTETS, IP6_OUTMCASTOCTETS],
#			     ['Broadcast Packets', IP6_INBCASTPKTS, IP6_OUTBCASTPKTS],
#			     ['Broadcast Bytes', IP6_INBCASTOCTETS, IP6_OUTBCASTOCTETS],
#			     ['Delivers', IP6_INDELIVERS, None],
#			     ['Forwarded', None, IP6_OUTFORWDATAGRAMS],
#			     ['No Routes', IP6_INNOROUTES, IP6_OUTNOROUTES],
#			     ['Header Errors', IP6_INHDRERRORS, None],
#			     ['Too Big Errors', IP6_INTOOBIGERRORS, None],
#			     ['Address Errors', IP6_INADDRERRORS, None],
#			     ['Unknown Protocol', IP6_INUNKNOWNPROTOS, None],
#			     ['Truncated Packets', IP6_INTRUNCATEDPKTS, None],
#			     ['Reasm Timeouts', IP6_REASMTIMEOUT, None],
#			     ['Reasm Requests', IP6_REASMREQDS, None],
#			     ['Reasm Failures', IP6_REASMFAILS, None],
#			     ['Reasm OK', IP6_REASMOKS, None],
#			     ['Frag Created', None, IP6_FRAGCREATES],
#			     ['Frag Failures', None, IP6_FRAGFAILS],
#			     ['Frag OK', None, IP6_FRAGOKS],
#			     ['', None, None],
#			     ['ICMPv6:', None, None],
#			     ['Messages', ICMP6_INMSGS, ICMP6_OUTMSGS],
#			     ['Errors', ICMP6_INERRORS, ICMP6_OUTERRORS]]
#
#			buf += '\n\t%s%s%s%s\n' % (33 * ' ', util.title('RX'),
#                        			   15 * ' ', util.title('TX'))
#
#			for row in l:
#				row[0] = util.kw(row[0])
#                                row[1] = self.get_stat(row[1]) if row[1] else ''
#                                row[2] = self.get_stat(row[2]) if row[2] else ''
#				buf += '\t{0:27} {1:>16} {2:>16}\n'.format(*row)

		return buf

###########################################################################
# Traffic class cache
class TcClassCache(netlink.Cache):
	"""Cache of traffic classes"""

	def __init__(self, ifindex, cache=None):
        	if not cache:
                        cache = self._alloc_cache_name("route/class")

		self._protocol = netlink.NETLINK_ROUTE
                self._nl_cache = cache
		self._set_arg1(ifindex)

	def _new_object(self, obj):
        	return TcClass(obj)

	def _new_cache(self, cache):
		return TcClassCache(self.arg1, cache=cache)

###########################################################################
# Traffic Class Object
class TcClass(Tc):
	"""Traffic Class"""

	def __init__(self, obj=None):
		netlink.Object.__init__(self, "route/class", "class", obj)
                self._module_path = 'netlink.route.qdisc.'
		self._rtnl_class = self._obj2type(self._nl_object)
		self._rtnl_tc = capi.obj2tc(self._nl_object)

		netlink.add_attr('class.handle', fmt=util.handle)
		netlink.add_attr('class.parent', fmt=util.handle)
		netlink.add_attr('class.kind', fmt=util.bold)

                if self.kind:
                        self._tc_module_lookup()

	@classmethod
        def from_capi(cls, obj):
        	return cls(capi.class2obj(obj))

	def _obj2type(self, obj):
        	return capi.obj2class(obj)

	def _new_instance(self, obj):
		if not obj:
			raise ValueError()

                return TcClass(obj)

	@property
	def childs(self):
		ret = []

		# classes can have classifiers, child classes and leaf
		# qdiscs
		ret += get_cls(self.ifindex, parent=self.handle)
		ret += get_class(self.ifindex, parent=self.handle)
		ret += get_qdisc(self.ifindex, parent=self.handle)

		return ret

	###################################################################
	#
	# format(details=False, stats=False)
	#
	def format(self, details=False, stats=False, nodev=False,
		   noparent=False, indent=''):
        	"""Return class as formatted text"""
		fmt = util.MyFormatter(self, indent)

		buf = fmt.format(self.brief('class', nodev, noparent))

		if details:
			buf += fmt.nl('\t' + self.details())
                	
		return buf

###########################################################################
# Classifier Cache
class ClassifierCache(netlink.Cache):
	"""Cache of traffic classifiers objects"""

	def __init__(self, ifindex, parent, cache=None):
        	if not cache:
                        cache = self._alloc_cache_name("route/cls")

		self._protocol = netlink.NETLINK_ROUTE
                self._nl_cache = cache
		self._set_arg1(ifindex)
		self._set_arg2(int(parent))

	def _new_object(self, obj):
        	return Classifier(obj)

	def _new_cache(self, cache):
		return ClassifierCache(self.arg1, self.arg2, cache=cache)

###########################################################################
# Classifier Object
class Classifier(Tc):
	"""Classifier"""

	def __init__(self, obj=None):
		netlink.Object.__init__(self, "route/cls", "cls", obj)
                self._module_path = 'netlink.route.cls.'
		self._rtnl_cls = self._obj2type(self._nl_object)
		self._rtnl_tc = capi.obj2tc(self._nl_object)

		netlink.add_attr('cls.handle', fmt=util.handle)
		netlink.add_attr('cls.parent', fmt=util.handle)
		netlink.add_attr('cls.kind', fmt=util.bold)

	@classmethod
        def from_capi(cls, obj):
        	return cls(capi.cls2obj(obj))

	def _obj2type(self, obj):
        	return capi.obj2cls(obj)

	def _new_instance(self, obj):
		if not obj:
			raise ValueError()

                return Classifier(obj)

	#####################################################################
	# priority
        @property
        def priority(self):
                return capi.rtnl_cls_get_prio(self._rtnl_cls)

	@priority.setter
        def priority(self, value):
                capi.rtnl_cls_set_prio(self._rtnl_cls, int(value))

	#####################################################################
	# protocol
        @property
        def protocol(self):
                return capi.rtnl_cls_get_protocol(self._rtnl_cls)

	@protocol.setter
        def protocol(self, value):
                capi.rtnl_cls_set_protocol(self._rtnl_cls, int(value))

	@property
	def childs(self):
		return []

	###################################################################
	#
	# format(details=False, stats=False)
	#
	def format(self, details=False, stats=False, nodev=False,
		   noparent=False, indent=''):
        	"""Return class as formatted text"""
		fmt = util.MyFormatter(self, indent)

		buf = fmt.format(self.brief('classifier', nodev, noparent))
		buf += fmt.format(' {t|priority} {t|protocol}')

		if details:
			buf += fmt.nl('\t' + self.details())
                	
		return buf

_qdisc_cache = QdiscCache()

def get_qdisc(ifindex, handle=None, parent=None):
	l = []

	_qdisc_cache.refill()

	for qdisc in _qdisc_cache:
		if qdisc.ifindex == ifindex and \
		   (handle == None or qdisc.handle == handle) and \
		   (parent == None or qdisc.parent == parent):
		   	l.append(qdisc)
	
	return l

_class_cache = {}

def get_class(ifindex, parent, handle=None):
	l = []

	try:
		cache = _class_cache[ifindex]
	except KeyError:
		cache = TcClassCache(ifindex)
		_class_cache[ifindex] = cache
	
	cache.refill()

	for cl in cache:
		if (parent == None or cl.parent == parent) and \
		   (handle == None or cl.handle == handle):
			l.append(cl)

	return l

_cls_cache = {}

def get_cls(ifindex, parent, handle=None):
	l = []

	try:
		chain = _cls_cache[ifindex]
	except KeyError:
		_cls_cache[ifindex] = {}

	try:
		cache = _cls_cache[ifindex][parent]
	except KeyError:
		cache = ClassifierCache(ifindex, parent)
		_cls_cache[ifindex][parent] = cache
	
	cache.refill()

	for cls in cache:
		if handle == None or cls.handle == handle:
			l.append(cls)

	return l
