import netlink.capi as nl
import netlink.genl.capi as genl
import nl80211
import sys
import traceback

class test_class:
	def __init__(self):
		self.done = 1;

def freq_to_ch(freq):
	if freq == 2484:
		return 14;

	if freq < 2484:
		return (freq - 2407) / 5;

	# FIXME: dot11ChannelStartingFactor (802.11-2007 17.3.8.3.2)
	if freq < 45000:
		return freq/5 - 1000;

	if freq >= 58320 and freq <= 64800:
		return (freq - 56160) / 2160;

	return 0;

def handle_freq(attr, pol):
	e, fattr = nl.py_nla_parse_nested(nl80211.NL80211_FREQUENCY_ATTR_MAX, attr, pol)
	if nl80211.NL80211_FREQUENCY_ATTR_FREQ in fattr:
		freq = nl.nla_get_u32(fattr[nl80211.NL80211_FREQUENCY_ATTR_FREQ])
		sys.stdout.write("\t\tfreq %d MHz [%d]" % (freq, freq_to_ch(freq)))
	if nl80211.NL80211_FREQUENCY_ATTR_MAX_TX_POWER in fattr and not (nl80211.NL80211_FREQUENCY_ATTR_DISABLED in fattr):
		sys.stdout.write(" (%.1f dBm)" % (0.01 * nl.nla_get_u32(fattr[nl80211.NL80211_FREQUENCY_ATTR_MAX_TX_POWER])))
	if nl80211.NL80211_FREQUENCY_ATTR_DISABLED in fattr:
		sys.stdout.write(" (disabled)")
	sys.stdout.write("\n")

def handle_band(attr, fpol):
	e, battr = nl.py_nla_parse_nested(nl80211.NL80211_BAND_ATTR_MAX, attr, None)
	print("\tband %d:" % nl.nla_type(attr))
	if nl80211.NL80211_BAND_ATTR_FREQS in battr:
		for fattr in nl.nla_get_nested(battr[nl80211.NL80211_BAND_ATTR_FREQS]):
			handle_freq(fattr, fpol)

def cipher_name(suite):
	suite_val = '%02x%02x%02x%02x' % tuple(reversed(suite))
	if suite_val == '000fac01':
		return "WEP40 (00-0f-ac:1)"
	elif suite_val == '000fac05':
		return "WEP104 (00-0f-ac:5)"
	elif suite_val == '000fac02':
		return "TKIP (00-0f-ac:2)"
	elif suite_val == '000fac04':
		return "CCMP (00-0f-ac:4)"
	elif suite_val == '000fac06':
		return "CMAC (00-0f-ac:6)"
	elif suite_val == '000fac08':
		return "GCMP (00-0f-ac:8)"
	elif suite_val == '00147201':
		return "WPI-SMS4 (00-14-72:1)"
	else:
		return suite_val

def msg_handler(m, a):
	try:
		e, attr = genl.py_genlmsg_parse(nl.nlmsg_hdr(m), 0,
						nl80211.NL80211_ATTR_MAX, None)
		if nl80211.NL80211_ATTR_WIPHY_NAME in attr:
			print('wiphy %s' % nl.nla_get_string(attr[nl80211.NL80211_ATTR_WIPHY_NAME]))
		if nl80211.NL80211_ATTR_WIPHY_BANDS in attr:
			fpol = nl.nla_policy_array(nl80211.NL80211_FREQUENCY_ATTR_MAX + 1)
			fpol[nl80211.NL80211_FREQUENCY_ATTR_FREQ].type = nl.NLA_U32
			fpol[nl80211.NL80211_FREQUENCY_ATTR_DISABLED].type = nl.NLA_FLAG
			fpol[nl80211.NL80211_FREQUENCY_ATTR_PASSIVE_SCAN].type = nl.NLA_FLAG
			fpol[nl80211.NL80211_FREQUENCY_ATTR_NO_IBSS].type = nl.NLA_FLAG
			fpol[nl80211.NL80211_FREQUENCY_ATTR_RADAR].type = nl.NLA_FLAG
			fpol[nl80211.NL80211_FREQUENCY_ATTR_MAX_TX_POWER].type = nl.NLA_U32

			nattrs = nl.nla_get_nested(attr[nl80211.NL80211_ATTR_WIPHY_BANDS])
			for nattr in nattrs:
				handle_band(nattr, fpol)
		if nl80211.NL80211_ATTR_CIPHER_SUITES in attr:
			ciphers = nl.nla_data(attr[nl80211.NL80211_ATTR_CIPHER_SUITES])
			num = len(ciphers) / 4
			if num > 0:
				print("\tSupported Ciphers:");
				for i in range(0, num, 4):
					print("\t\t* %s" % cipher_name(ciphers[i:i+4]))
		if nl80211.NL80211_ATTR_SUPPORTED_IFTYPES in attr:
			print("\tSupported interface modes:")
			ifattr = nl.nla_get_nested(attr[nl80211.NL80211_ATTR_SUPPORTED_IFTYPES])
			for nl_mode in ifattr:
				print("\t\t* %s" % nl80211.nl80211_iftype2str[nl.nla_type(nl_mode)])
		if nl80211.NL80211_ATTR_SOFTWARE_IFTYPES in attr:
			print("\tsoftware interface modes (can always be added):")
			ifattr = nl.nla_get_nested(attr[nl80211.NL80211_ATTR_SOFTWARE_IFTYPES])
			for nl_mode in ifattr:
				print("\t\t* %s" % nl80211.nl80211_iftype2str[nl.nla_type(nl_mode)])
		return nl.NL_SKIP
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
	genl.genlmsg_put(m, 0, 0, family, 0, 0, nl80211.NL80211_CMD_GET_WIPHY, 0)
	nl.nla_put_u32(m, nl80211.NL80211_ATTR_WIPHY, 7)

	err = nl.nl_send_auto_complete(s, m);
	if err < 0:
		nl.nlmsg_free(msg)

	while cbd.done > 0 and not err < 0:
		err = nl.nl_recvmsgs(s, rx_cb)
except Exception as e:
	(t, v, tb) = sys.exc_info()
	print v.message
	traceback.print_tb(tb)
