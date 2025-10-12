import netlink.core as netlink
import netlink.route.capi as capi
import netlink.route.link as link

sock = netlink.lookup_socket(netlink.NETLINK_ROUTE)

cache = link.LinkCache()
cache.refill(sock)

testtap1 = cache['testtap1']
print testtap1

lbr = link.Link()
lbr.type = 'bridge'
lbr.name = 'testbridge'
print lbr
lbr.add()

cache.refill(sock)
lbr = cache['testbridge']
print lbr

lbr.enslave(testtap1)
cache.refill(sock)
testtap1 = cache['testtap1']

print capi.rtnl_link_is_bridge(lbr._rtnl_link)
print capi.rtnl_link_get_master(testtap1._rtnl_link)
