import netlink.capi as nl
import netlink.genl.capi as genl
import nl80211
import sys
import traceback

class test_class:
	def __init__(self):
		self.done = 1;

def msg_handler(m, a):
	try:
		e, attr = genl.py_genlmsg_parse(nl.nlmsg_hdr(m), 0,
						nl80211.NL80211_ATTR_MAX, None)
		if nl80211.NL80211_ATTR_WIPHY in attr:
			thiswiphy = nl.nla_get_u32(attr[nl80211.NL80211_ATTR_WIPHY])
			print("phy#%d" % thiswiphy)
		if nl80211.NL80211_ATTR_IFNAME in attr:
			print("\tinterface %s" % nl.nla_get_string(attr[nl80211.NL80211_ATTR_IFNAME]));
		if nl80211.NL80211_ATTR_IFINDEX in attr:
			print("\tifindex %d" % nl.nla_get_u32(attr[nl80211.NL80211_ATTR_IFINDEX]))
		if nl80211.NL80211_ATTR_WDEV in attr:
			print("\twdev 0x%lx" % nl.nla_get_u64(attr[nl80211.NL80211_ATTR_WDEV]))
		if nl80211.NL80211_ATTR_MAC in attr:
			print("\tmac %02x:%02x:%02x:%02x:%02x:%02x" % tuple(nl.nla_data(attr[nl80211.NL80211_ATTR_MAC])))
		if nl80211.NL80211_ATTR_SSID in attr:
			print("\tssid ", nl.nla_data(attr[nl80211.NL80211_ATTR_SSID]))
		if nl80211.NL80211_ATTR_IFTYPE in attr:
			iftype = nl.nla_get_u32(attr[nl80211.NL80211_ATTR_IFTYPE])
			print("\ttype %s" % nl80211.nl80211_iftype2str[iftype])
		if nl80211.NL80211_ATTR_WIPHY_FREQ in attr:
			freq = nl.nla_get_u32(attr[nl80211.NL80211_ATTR_WIPHY_FREQ])

			sys.stdout.write("\tfreq %d MHz" % freq);

			if nl80211.NL80211_ATTR_CHANNEL_WIDTH in attr:
				chanw = nl.nla_get_u32(attr[nl80211.NL80211_ATTR_CHANNEL_WIDTH])
				sys.stdout.write(", width: %s" % nl80211.nl80211_chan_width2str[chanw])
				if nl80211.NL80211_ATTR_CENTER_FREQ1 in attr:
					sys.stdout.write(", center1: %d MHz" %
						nl.nla_get_u32(attr[nl80211.NL80211_ATTR_CENTER_FREQ1]))
				if nl80211.NL80211_ATTR_CENTER_FREQ2 in attr:
					sys.stdout.write(", center2: %d MHz" %
						nl.nla_get_u32(attr[nl80211.NL80211_ATTR_CENTER_FREQ2]))
			elif nl80211.NL80211_ATTR_WIPHY_CHANNEL_TYPE in attr:
				channel_type = nl.nla_get_u32(attr[nl80211.NL80211_ATTR_WIPHY_CHANNEL_TYPE])
				sys.stdout.write(" %s" % nl80211.nl80211_channel_type2str(channel_type));

			sys.stdout.write("\n");
		return nl.NL_SKIP;
	except Exception as e:
		(t,v,tb) = sys.exc_info()
		print v.message
		traceback.print_tb(tb)

def error_handler(err, a):
	a.done = err.error
	return nl.NL_STOP

def finish_handler(m, a):
	return nl.NL_SKIP

def ack_handler(m, a):
	a.done = 0
	return nl.NL_STOP

try:
	cbd = test_class()
	tx_cb = nl.nl_cb_alloc(nl.NL_CB_DEFAULT)
	rx_cb = nl.nl_cb_clone(tx_cb)
	s = nl.nl_socket_alloc_cb(tx_cb)
	nl.py_nl_cb_err(rx_cb, nl.NL_CB_CUSTOM, error_handler, cbd);
	nl.py_nl_cb_set(rx_cb, nl.NL_CB_FINISH, nl.NL_CB_CUSTOM, finish_handler, cbd);
	nl.py_nl_cb_set(rx_cb, nl.NL_CB_ACK, nl.NL_CB_CUSTOM, ack_handler, cbd);
	nl.py_nl_cb_set(rx_cb, nl.NL_CB_VALID, nl.NL_CB_CUSTOM, msg_handler, cbd);

	genl.genl_connect(s)
	family = genl.genl_ctrl_resolve(s, 'nl80211')
	m = nl.nlmsg_alloc()
	genl.genlmsg_put(m, 0, 0, family, 0, 0, nl80211.NL80211_CMD_GET_INTERFACE, 0)
	nl.nla_put_u32(m, nl80211.NL80211_ATTR_IFINDEX, nl.if_nametoindex('wlan0'))

	err = nl.nl_send_auto_complete(s, m);
	if err < 0:
		nl.nlmsg_free(msg)

	while cbd.done > 0 and not err < 0:
		err = nl.nl_recvmsgs(s, rx_cb)

except Exception as e:
	(t, v, tb) = sys.exc_info()
	print v.message
	traceback.print_tb(tb)
