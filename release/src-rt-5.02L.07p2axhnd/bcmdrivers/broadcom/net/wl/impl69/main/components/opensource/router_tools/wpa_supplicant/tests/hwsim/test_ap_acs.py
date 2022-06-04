# Test cases for automatic channel selection with hostapd
# Copyright (c) 2013-2018, Jouni Malinen <j@w1.fi>
#
# This software may be distributed under the terms of the BSD license.
# See README for more details.

import logging
logger = logging.getLogger()
import time

import hostapd
from utils import skip_with_fips, alloc_fail, fail_test, HwsimSkip, clear_regdom
from test_ap_ht import clear_scan_cache
from test_dfs import wait_dfs_event

def force_prev_ap_on_24g(ap):
    # For now, make sure the last operating channel was on 2.4 GHz band to get
    # sufficient survey data from mac80211_hwsim.
    hostapd.add_ap(ap, {"ssid": "open"})
    time.sleep(0.1)
    hostapd.remove_bss(ap)

def force_prev_ap_on_5g(ap):
    # For now, make sure the last operating channel was on 5 GHz band to get
    # sufficient survey data from mac80211_hwsim.
    hostapd.add_ap(ap, {"ssid": "open", "hw_mode": "a",
                        "channel": "36", "country_code": "US"})
    time.sleep(0.1)
    hostapd.remove_bss(ap)

def wait_acs(hapd, return_after_acs=False):
    ev = hapd.wait_event(["ACS-STARTED", "ACS-COMPLETED", "ACS-FAILED",
                          "AP-ENABLED", "AP-DISABLED"], timeout=5)
    if not ev:
        raise Exception("ACS start timed out")
    if "ACS-STARTED" not in ev:
        raise Exception("Unexpected ACS event: " + ev)

    state = hapd.get_status_field("state")
    if state != "ACS":
        raise Exception("Unexpected interface state")

    ev = hapd.wait_event(["ACS-COMPLETED", "ACS-FAILED", "AP-ENABLED",
                          "AP-DISABLED"], timeout=20)
    if not ev:
        raise Exception("ACS timed out")
    if "ACS-COMPLETED" not in ev:
        raise Exception("Unexpected ACS event: " + ev)

    if return_after_acs:
        return

    ev = hapd.wait_event(["AP-ENABLED", "AP-DISABLED"], timeout=5)
    if not ev:
        raise Exception("AP setup timed out")
    if "AP-ENABLED" not in ev:
        raise Exception("Unexpected ACS event: " + ev)

    state = hapd.get_status_field("state")
    if state != "ENABLED":
        raise Exception("Unexpected interface state")

def test_ap_acs(dev, apdev):
    """Automatic channel selection"""
    force_prev_ap_on_24g(apdev[0])
    params = hostapd.wpa2_params(ssid="test-acs", passphrase="12345678")
    params['channel'] = '0'
    hapd = hostapd.add_ap(apdev[0], params, wait_enabled=False)
    wait_acs(hapd)

    freq = hapd.get_status_field("freq")
    if int(freq) < 2400:
        raise Exception("Unexpected frequency")

    dev[0].connect("test-acs", psk="12345678", scan_freq=freq)

def test_ap_acs_chanlist(dev, apdev):
    """Automatic channel selection with chanlist set"""
    force_prev_ap_on_24g(apdev[0])
    params = hostapd.wpa2_params(ssid="test-acs", passphrase="12345678")
    params['channel'] = '0'
    params['chanlist'] = '1 6 11'
    hapd = hostapd.add_ap(apdev[0], params, wait_enabled=False)
    wait_acs(hapd)

    freq = hapd.get_status_field("freq")
    if int(freq) < 2400:
        raise Exception("Unexpected frequency")

    dev[0].connect("test-acs", psk="12345678", scan_freq=freq)

def test_ap_acs_freqlist(dev, apdev):
    """Automatic channel selection with freqlist set"""
    force_prev_ap_on_24g(apdev[0])
    params = hostapd.wpa2_params(ssid="test-acs", passphrase="12345678")
    params['channel'] = '0'
    params['freqlist'] = '2412 2437 2462'
    hapd = hostapd.add_ap(apdev[0], params, wait_enabled=False)
    wait_acs(hapd)

    freq = int(hapd.get_status_field("freq"))
    if freq not in [2412, 2437, 2462]:
        raise Exception("Unexpected frequency: " + freq)

    dev[0].connect("test-acs", psk="12345678", scan_freq=str(freq))

def test_ap_acs_invalid_chanlist(dev, apdev):
    """Automatic channel selection with invalid chanlist"""
    force_prev_ap_on_24g(apdev[0])
    params = hostapd.wpa2_params(ssid="test-acs", passphrase="12345678")
    params['channel'] = '0'
    params['chanlist'] = '15-18'
    hapd = hostapd.add_ap(apdev[0], params, no_enable=True)
    res = hapd.request("ENABLE")
    if "OK" in res:
        raise Exception("ENABLE command succeeded unexpectedly")

def test_ap_multi_bss_acs(dev, apdev):
    """hostapd start with a multi-BSS configuration file using ACS"""
    skip_with_fips(dev[0])
    force_prev_ap_on_24g(apdev[0])

    # start the actual test
    hapd = hostapd.add_iface(apdev[0], 'multi-bss-acs.conf')
    hapd.enable()
    wait_acs(hapd)

    freq = hapd.get_status_field("freq")
    if int(freq) < 2400:
        raise Exception("Unexpected frequency")

    dev[0].connect("bss-1", key_mgmt="NONE", scan_freq=freq)
    dev[1].connect("bss-2", psk="12345678", scan_freq=freq)
    dev[2].connect("bss-3", psk="qwertyuiop", scan_freq=freq)

def test_ap_acs_40mhz(dev, apdev):
    """Automatic channel selection for 40 MHz channel"""
    run_ap_acs_40mhz(dev, apdev, '[HT40+]')

def test_ap_acs_40mhz_plus_or_minus(dev, apdev):
    """Automatic channel selection for 40 MHz channel (plus or minus)"""
    run_ap_acs_40mhz(dev, apdev, '[HT40+][HT40-]')

def run_ap_acs_40mhz(dev, apdev, ht_capab):
    clear_scan_cache(apdev[0])
    force_prev_ap_on_24g(apdev[0])
    params = hostapd.wpa2_params(ssid="test-acs", passphrase="12345678")
    params['channel'] = '0'
    params['ht_capab'] = ht_capab
    hapd = hostapd.add_ap(apdev[0], params, wait_enabled=False)
    wait_acs(hapd)

    freq = hapd.get_status_field("freq")
    if int(freq) < 2400:
        raise Exception("Unexpected frequency")
    sec = hapd.get_status_field("secondary_channel")
    if int(sec) == 0:
        raise Exception("Secondary channel not set")

    dev[0].connect("test-acs", psk="12345678", scan_freq=freq)

def test_ap_acs_40mhz_minus(dev, apdev):
    """Automatic channel selection for HT40- channel"""
    clear_scan_cache(apdev[0])
    force_prev_ap_on_24g(apdev[0])
    params = hostapd.wpa2_params(ssid="test-acs", passphrase="12345678")
    params['channel'] = '0'
    params['ht_capab'] = '[HT40-]'
    params['acs_num_scans'] = '1'
    params['chanlist'] = '1 11'
    hapd = hostapd.add_ap(apdev[0], params, wait_enabled=False)
    ev = hapd.wait_event(["AP-ENABLED", "AP-DISABLED"], timeout=10)
    if not ev:
        raise Exception("ACS start timed out")
    # HT40- is not currently supported in hostapd ACS, so do not try to connect
    # or verify that this operation succeeded.

def test_ap_acs_5ghz(dev, apdev):
    """Automatic channel selection on 5 GHz"""
    try:
        hapd = None
        force_prev_ap_on_5g(apdev[0])
        params = hostapd.wpa2_params(ssid="test-acs", passphrase="12345678")
        params['hw_mode'] = 'a'
        params['channel'] = '0'
        params['country_code'] = 'US'
        hapd = hostapd.add_ap(apdev[0], params, wait_enabled=False)
        wait_acs(hapd)
        freq = hapd.get_status_field("freq")
        if int(freq) < 5000:
            raise Exception("Unexpected frequency")

        dev[0].connect("test-acs", psk="12345678", scan_freq=freq)
        dev[0].wait_regdom(country_ie=True)
    finally:
        clear_regdom(hapd, dev)

def test_ap_acs_5ghz_40mhz(dev, apdev):
    """Automatic channel selection on 5 GHz for 40 MHz channel"""
    try:
        hapd = None
        force_prev_ap_on_5g(apdev[0])
        params = hostapd.wpa2_params(ssid="test-acs", passphrase="12345678")
        params['hw_mode'] = 'a'
        params['channel'] = '0'
        params['ht_capab'] = '[HT40+]'
        params['country_code'] = 'US'
        hapd = hostapd.add_ap(apdev[0], params, wait_enabled=False)
        wait_acs(hapd)
        freq = hapd.get_status_field("freq")
        if int(freq) < 5000:
            raise Exception("Unexpected frequency")

        sec = hapd.get_status_field("secondary_channel")
        if int(sec) == 0:
            raise Exception("Secondary channel not set")

        dev[0].connect("test-acs", psk="12345678", scan_freq=freq)
        dev[0].wait_regdom(country_ie=True)
    finally:
        clear_regdom(hapd, dev)

def test_ap_acs_vht(dev, apdev):
    """Automatic channel selection for VHT"""
    try:
        hapd = None
        force_prev_ap_on_5g(apdev[0])
        params = hostapd.wpa2_params(ssid="test-acs", passphrase="12345678")
        params['hw_mode'] = 'a'
        params['channel'] = '0'
        params['ht_capab'] = '[HT40+]'
        params['country_code'] = 'US'
        params['ieee80211ac'] = '1'
        params['vht_oper_chwidth'] = '1'
        hapd = hostapd.add_ap(apdev[0], params, wait_enabled=False)
        wait_acs(hapd)
        freq = hapd.get_status_field("freq")
        if int(freq) < 5000:
            raise Exception("Unexpected frequency")

        sec = hapd.get_status_field("secondary_channel")
        if int(sec) == 0:
            raise Exception("Secondary channel not set")

        dev[0].connect("test-acs", psk="12345678", scan_freq=freq)
        dev[0].wait_regdom(country_ie=True)
    finally:
        clear_regdom(hapd, dev)

def test_ap_acs_vht40(dev, apdev):
    """Automatic channel selection for VHT40"""
    try:
        hapd = None
        force_prev_ap_on_5g(apdev[0])
        params = hostapd.wpa2_params(ssid="test-acs", passphrase="12345678")
        params['hw_mode'] = 'a'
        params['channel'] = '0'
        params['ht_capab'] = '[HT40+]'
        params['country_code'] = 'US'
        params['ieee80211ac'] = '1'
        params['vht_oper_chwidth'] = '0'
        params['acs_num_scans'] = '1'
        params['chanlist'] = '36 149'
        hapd = hostapd.add_ap(apdev[0], params, wait_enabled=False)
        wait_acs(hapd)
        freq = hapd.get_status_field("freq")
        if int(freq) < 5000:
            raise Exception("Unexpected frequency")

        sec = hapd.get_status_field("secondary_channel")
        if int(sec) == 0:
            raise Exception("Secondary channel not set")

        dev[0].connect("test-acs", psk="12345678", scan_freq=freq)
        dev[0].wait_regdom(country_ie=True)
    finally:
        clear_regdom(hapd, dev)

def test_ap_acs_vht160(dev, apdev):
    """Automatic channel selection for VHT160"""
    try:
        hapd = None
        force_prev_ap_on_5g(apdev[0])
        params = hostapd.wpa2_params(ssid="test-acs", passphrase="12345678")
        params['hw_mode'] = 'a'
        params['channel'] = '0'
        params['ht_capab'] = '[HT40+]'
        params['country_code'] = 'ZA'
        params['ieee80211ac'] = '1'
        params['vht_oper_chwidth'] = '2'
        params['ieee80211d'] = '1'
        params['ieee80211h'] = '1'
        params['chanlist'] = '100'
        params['acs_num_scans'] = '1'
        hapd = hostapd.add_ap(apdev[0], params, wait_enabled=False)
        ev = hapd.wait_event(["AP-ENABLED", "AP-DISABLED"], timeout=10)
        if not ev:
            raise Exception("ACS start timed out")
        # VHT160 is not currently supported in hostapd ACS, so do not try to
        # enforce successful AP start.
        if "AP-ENABLED" in ev:
            freq = hapd.get_status_field("freq")
            if int(freq) < 5000:
                raise Exception("Unexpected frequency")
            dev[0].connect("test-acs", psk="12345678", scan_freq=freq)
            dev[0].wait_regdom(country_ie=True)
    finally:
        clear_regdom(hapd, dev)

def test_ap_acs_vht160_scan_disable(dev, apdev):
    """Automatic channel selection for VHT160 and DISABLE during scan"""
    force_prev_ap_on_5g(apdev[0])
    params = hostapd.wpa2_params(ssid="test-acs", passphrase="12345678")
    params['hw_mode'] = 'a'
    params['channel'] = '0'
    params['ht_capab'] = '[HT40+]'
    params['country_code'] = 'ZA'
    params['ieee80211ac'] = '1'
    params['vht_oper_chwidth'] = '2'
    params["vht_oper_centr_freq_seg0_idx"] = "114"
    params['ieee80211d'] = '1'
    params['ieee80211h'] = '1'
    hapd = hostapd.add_ap(apdev[0], params, wait_enabled=False)
    time.sleep(3)
    clear_regdom(hapd, dev)

def test_ap_acs_bias(dev, apdev):
    """Automatic channel selection with bias values"""
    force_prev_ap_on_24g(apdev[0])
    params = hostapd.wpa2_params(ssid="test-acs", passphrase="12345678")
    params['channel'] = '0'
    params['acs_chan_bias'] = '1:0.8 3:1.2 6:0.7 11:0.8'
    hapd = hostapd.add_ap(apdev[0], params, wait_enabled=False)
    wait_acs(hapd)

    freq = hapd.get_status_field("freq")
    if int(freq) < 2400:
        raise Exception("Unexpected frequency")

    dev[0].connect("test-acs", psk="12345678", scan_freq=freq)

def test_ap_acs_survey(dev, apdev):
    """Automatic channel selection using acs_survey parameter"""
    force_prev_ap_on_24g(apdev[0])
    params = hostapd.wpa2_params(ssid="test-acs", passphrase="12345678")
    params['channel'] = 'acs_survey'
    params['acs_num_scans'] = '1'
    hapd = hostapd.add_ap(apdev[0], params, wait_enabled=False)
    wait_acs(hapd)

    freq = hapd.get_status_field("freq")
    if int(freq) < 2400:
        raise Exception("Unexpected frequency")

    dev[0].connect("test-acs", psk="12345678", scan_freq=freq)

def test_ap_acs_errors(dev, apdev):
    """Automatic channel selection failures"""
    clear_scan_cache(apdev[0])
    force_prev_ap_on_24g(apdev[0])
    params = hostapd.wpa2_params(ssid="test-acs", passphrase="12345678")
    params['channel'] = '0'
    params['acs_num_scans'] = '2'
    params['chanlist'] = '1'
    hapd = hostapd.add_ap(apdev[0], params, no_enable=True)

    with alloc_fail(hapd, 1, "acs_request_scan"):
        if "FAIL" not in hapd.request("ENABLE"):
            raise Exception("Unexpected success for ENABLE")

    hapd.dump_monitor()
    with fail_test(hapd, 1, "acs_request_scan"):
        if "FAIL" not in hapd.request("ENABLE"):
            raise Exception("Unexpected success for ENABLE")

    hapd.dump_monitor()
    with fail_test(hapd, 1, "acs_scan_complete"):
        hapd.enable()
        ev = hapd.wait_event(["AP-ENABLED", "AP-DISABLED"], timeout=10)
        if not ev:
            raise Exception("ACS start timed out")

    hapd.dump_monitor()
    with fail_test(hapd, 1, "acs_request_scan;acs_scan_complete"):
        hapd.enable()
        ev = hapd.wait_event(["AP-ENABLED", "AP-DISABLED"], timeout=10)
        if not ev:
            raise Exception("ACS start timed out")

def test_ap_acs_dfs(dev, apdev, params):
    """Automatic channel selection, HT scan, and DFS [long]"""
    if not params['long']:
        raise HwsimSkip("Skip test case with long duration due to --long not specified")
    try:
        hapd = None
        force_prev_ap_on_5g(apdev[0])
        params = hostapd.wpa2_params(ssid="test-acs", passphrase="12345678")
        params['hw_mode'] = 'a'
        params['channel'] = '0'
        params['ht_capab'] = '[HT40+]'
        params['country_code'] = 'US'
        params['ieee80211d'] = '1'
        params['ieee80211h'] = '1'
        params['acs_num_scans'] = '1'
        params['chanlist'] = '52 56 60 64'
        hapd = hostapd.add_ap(apdev[0], params, wait_enabled=False)
        wait_acs(hapd, return_after_acs=True)

        wait_dfs_event(hapd, "DFS-CAC-START", 5)
        ev = wait_dfs_event(hapd, "DFS-CAC-COMPLETED", 70)
        if "success=1" not in ev:
            raise Exception("CAC failed")

        ev = hapd.wait_event(["AP-ENABLED"], timeout=5)
        if not ev:
            raise Exception("AP setup timed out")

        state = hapd.get_status_field("state")
        if state != "ENABLED":
            raise Exception("Unexpected interface state")

        freq = int(hapd.get_status_field("freq"))
        if freq not in [5260, 5280, 5300, 5320]:
            raise Exception("Unexpected frequency: %d" % freq)

        dev[0].connect("test-acs", psk="12345678", scan_freq=str(freq))
        dev[0].wait_regdom(country_ie=True)
    finally:
        if hapd:
            hapd.request("DISABLE")
        dev[0].disconnect_and_stop_scan()
        hostapd.cmd_execute(apdev[0], ['iw', 'reg', 'set', '00'])
        dev[0].wait_event(["CTRL-EVENT-REGDOM-CHANGE"], timeout=0.5)
        dev[0].flush_scan_cache()

def test_ap_acs_exclude_dfs(dev, apdev, params):
    """Automatic channel selection, exclude DFS"""
    try:
        hapd = None
        force_prev_ap_on_5g(apdev[0])
        params = hostapd.wpa2_params(ssid="test-acs", passphrase="12345678")
        params['hw_mode'] = 'a'
        params['channel'] = '0'
        params['ht_capab'] = '[HT40+]'
        params['country_code'] = 'US'
        params['ieee80211d'] = '1'
        params['ieee80211h'] = '1'
        params['acs_num_scans'] = '1'
        params['acs_exclude_dfs'] = '1'
        hapd = hostapd.add_ap(apdev[0], params, wait_enabled=False)
        wait_acs(hapd)

        state = hapd.get_status_field("state")
        if state != "ENABLED":
            raise Exception("Unexpected interface state")

        freq = int(hapd.get_status_field("freq"))
        if freq in [5260, 5280, 5300, 5320,
                    5500, 5520, 5540, 5560, 5580, 5600, 5620, 5640, 5660, 5680]:
            raise Exception("Unexpected frequency: %d" % freq)

        dev[0].connect("test-acs", psk="12345678", scan_freq=str(freq))
        dev[0].wait_regdom(country_ie=True)
    finally:
        if hapd:
            hapd.request("DISABLE")
        dev[0].disconnect_and_stop_scan()
        hostapd.cmd_execute(apdev[0], ['iw', 'reg', 'set', '00'])
        dev[0].wait_event(["CTRL-EVENT-REGDOM-CHANGE"], timeout=0.5)
        dev[0].flush_scan_cache()

def test_ap_acs_vht160_dfs(dev, apdev, params):
    """Automatic channel selection 160 MHz, HT scan, and DFS [long]"""
    if not params['long']:
        raise HwsimSkip("Skip test case with long duration due to --long not specified")
    try:
        hapd = None
        force_prev_ap_on_5g(apdev[0])
        params = hostapd.wpa2_params(ssid="test-acs", passphrase="12345678")
        params['hw_mode'] = 'a'
        params['channel'] = '0'
        params['ht_capab'] = '[HT40+]'
        params['country_code'] = 'US'
        params['ieee80211ac'] = '1'
        params['vht_oper_chwidth'] = '2'
        params['ieee80211d'] = '1'
        params['ieee80211h'] = '1'
        params['acs_num_scans'] = '1'
        hapd = hostapd.add_ap(apdev[0], params, wait_enabled=False)
        wait_acs(hapd, return_after_acs=True)

        wait_dfs_event(hapd, "DFS-CAC-START", 5)
        ev = wait_dfs_event(hapd, "DFS-CAC-COMPLETED", 70)
        if "success=1" not in ev:
            raise Exception("CAC failed")

        ev = hapd.wait_event(["AP-ENABLED"], timeout=5)
        if not ev:
            raise Exception("AP setup timed out")

        state = hapd.get_status_field("state")
        if state != "ENABLED":
            raise Exception("Unexpected interface state")

        freq = int(hapd.get_status_field("freq"))
        if freq not in [5180, 5500]:
            raise Exception("Unexpected frequency: %d" % freq)

        dev[0].connect("test-acs", psk="12345678", scan_freq=str(freq))
        dev[0].wait_regdom(country_ie=True)
    finally:
        if hapd:
            hapd.request("DISABLE")
        dev[0].disconnect_and_stop_scan()
        hostapd.cmd_execute(apdev[0], ['iw', 'reg', 'set', '00'])
        dev[0].wait_event(["CTRL-EVENT-REGDOM-CHANGE"], timeout=0.5)
        dev[0].flush_scan_cache()
