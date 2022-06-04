# Test cases for Multi-AP
# Copyright (c) 2018, The Linux Foundation
#
# This software may be distributed under the terms of the BSD license.
# See README for more details.

import hostapd

def test_multi_ap_association(dev, apdev):
    """Multi-AP association in backhaul BSS"""
    run_multi_ap_association(dev, apdev, 1)
    dev[1].connect("multi-ap", psk="12345678", scan_freq="2412",
                   wait_connect=False)
    ev = dev[1].wait_event(["CTRL-EVENT-DISCONNECTED",
                            "CTRL-EVENT-CONNECTED",
                            "CTRL-EVENT-ASSOC-REJECT"],
                           timeout=5)
    dev[1].request("DISCONNECT")
    if ev is None:
        raise Exception("Connection result not reported")
    if "CTRL-EVENT-ASSOC-REJECT" not in ev:
        raise Exception("Association rejection not reported")
    if "status_code=12" not in ev:
        raise Exception("Unexpected association status code: " + ev)

def test_multi_ap_association_shared_bss(dev, apdev):
    """Multi-AP association in backhaul BSS (with fronthaul BSS enabled)"""
    run_multi_ap_association(dev, apdev, 3)
    dev[1].connect("multi-ap", psk="12345678", scan_freq="2412")

def run_multi_ap_association(dev, apdev, multi_ap, wait_connect=True):
    params = hostapd.wpa2_params(ssid="multi-ap", passphrase="12345678")
    if multi_ap:
        params["multi_ap"] = str(multi_ap)
    hapd = hostapd.add_ap(apdev[0], params)

    dev[0].connect("multi-ap", psk="12345678", scan_freq="2412",
                   multi_ap_backhaul_sta="1", wait_connect=wait_connect)

def test_multi_ap_disabled_on_ap(dev, apdev):
    """Multi-AP association attempt when disabled on AP"""
    run_multi_ap_association(dev, apdev, 0, wait_connect=False)
    ev = dev[0].wait_event(["CTRL-EVENT-DISCONNECTED",
                            "CTRL-EVENT-CONNECTED"],
                           timeout=5)
    dev[0].request("DISCONNECT")
    if ev is None:
        raise Exception("Connection result not reported")
    if "CTRL-EVENT-DISCONNECTED" not in ev:
        raise Exception("Unexpected connection result")

def test_multi_ap_fronthaul_on_ap(dev, apdev):
    """Multi-AP association attempt when only fronthaul BSS on AP"""
    run_multi_ap_association(dev, apdev, 2, wait_connect=False)
    ev = dev[0].wait_event(["CTRL-EVENT-DISCONNECTED",
                            "CTRL-EVENT-CONNECTED",
                            "CTRL-EVENT-ASSOC-REJECT"],
                           timeout=5)
    dev[0].request("DISCONNECT")
    if ev is None:
        raise Exception("Connection result not reported")
    if "CTRL-EVENT-DISCONNECTED" not in ev:
        raise Exception("Unexpected connection result")

def run_multi_ap_wps(dev, apdev, params, multi_ap_bssid=None):
    """Helper for running Multi-AP WPS tests

    dev[0] does multi_ap WPS, dev[1] does normal WPS. apdev[0] is the fronthaul
    BSS. If there is a separate backhaul BSS, it must have been set up by the
    caller. params are the normal SSID parameters, they will be extended with
    the WPS parameters. multi_ap_bssid must be given if it is not equal to the
    fronthaul BSSID."""

    if multi_ap_bssid is None:
        multi_ap_bssid = apdev[0]['bssid']
    params.update({"wps_state": "2", "eap_server": "1"})

    # WPS with multi-ap station dev[0]
    hapd = hostapd.add_ap(apdev[0], params)
    hapd.request("WPS_PBC")
    if "PBC Status: Active" not in hapd.request("WPS_GET_STATUS"):
        raise Exception("PBC status not shown correctly")

    dev[0].request("WPS_PBC multi_ap=1")
    dev[0].wait_connected(timeout=20)
    status = dev[0].get_status()
    if status['wpa_state'] != 'COMPLETED' or status['bssid'] != multi_ap_bssid:
        raise Exception("Not fully connected")
    if status['ssid'] != params['multi_ap_backhaul_ssid'].strip('"'):
        raise Exception("Unexpected SSID %s != %s" % (status['ssid'], params["multi_ap_backhaul_ssid"]))
    if status['pairwise_cipher'] != 'CCMP':
        raise Exception("Unexpected encryption configuration %s" % status['pairwise_cipher'])
    if status['key_mgmt'] != 'WPA2-PSK':
        raise Exception("Unexpected key_mgmt")

    status = hapd.request("WPS_GET_STATUS")
    if "PBC Status: Disabled" not in status:
        raise Exception("PBC status not shown correctly")
    if "Last WPS result: Success" not in status:
        raise Exception("Last WPS result not shown correctly")
    if "Peer Address: " + dev[0].own_addr() not in status:
        raise Exception("Peer address not shown correctly")

    if len(dev[0].list_networks()) != 1:
        raise Exception("Unexpected number of network blocks")

    # WPS with non-Multi-AP station dev[1]
    hapd.request("WPS_PBC")
    if "PBC Status: Active" not in hapd.request("WPS_GET_STATUS"):
        raise Exception("PBC status not shown correctly")

    dev[1].request("WPS_PBC")
    dev[1].wait_connected(timeout=20)
    status = dev[1].get_status()
    if status['wpa_state'] != 'COMPLETED' or status['bssid'] != apdev[0]['bssid']:
        raise Exception("Not fully connected")
    if status['ssid'] != params["ssid"]:
        raise Exception("Unexpected SSID")
    # Fronthaul may be something else than WPA2-PSK so don't test it.

    status = hapd.request("WPS_GET_STATUS")
    if "PBC Status: Disabled" not in status:
        raise Exception("PBC status not shown correctly")
    if "Last WPS result: Success" not in status:
        raise Exception("Last WPS result not shown correctly")
    if "Peer Address: " + dev[1].own_addr() not in status:
        raise Exception("Peer address not shown correctly")

    if len(dev[1].list_networks()) != 1:
        raise Exception("Unexpected number of network blocks")

def test_multi_ap_wps_shared(dev, apdev):
    """WPS on shared fronthaul/backhaul AP"""
    ssid = "multi-ap-wps"
    passphrase = "12345678"
    params = hostapd.wpa2_params(ssid=ssid, passphrase=passphrase)
    params.update({"multi_ap": "3",
                   "multi_ap_backhaul_ssid": '"%s"' % ssid,
                   "multi_ap_backhaul_wpa_passphrase": passphrase})
    run_multi_ap_wps(dev, apdev, params)

def test_multi_ap_wps_shared_psk(dev, apdev):
    """WPS on shared fronthaul/backhaul AP using PSK"""
    ssid = "multi-ap-wps"
    psk = "1234567890abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    params = hostapd.wpa2_params(ssid=ssid)
    params.update({"wpa_psk": psk,
                   "multi_ap": "3",
                   "multi_ap_backhaul_ssid": '"%s"' % ssid,
                   "multi_ap_backhaul_wpa_psk": psk})
    run_multi_ap_wps(dev, apdev, params)

def test_multi_ap_wps_split(dev, apdev):
    """WPS on split fronthaul and backhaul AP"""
    backhaul_ssid = "multi-ap-backhaul-wps"
    backhaul_passphrase = "87654321"
    params = hostapd.wpa2_params(ssid="multi-ap-fronthaul-wps",
                                 passphrase="12345678")
    params.update({"multi_ap": "2",
                   "multi_ap_backhaul_ssid": '"%s"' % backhaul_ssid,
                   "multi_ap_backhaul_wpa_passphrase": backhaul_passphrase})
    params_backhaul = hostapd.wpa2_params(ssid=backhaul_ssid,
                                          passphrase=backhaul_passphrase)
    params_backhaul.update({"multi_ap": "1"})
    hapd_backhaul = hostapd.add_ap(apdev[1], params_backhaul)

    run_multi_ap_wps(dev, apdev, params, hapd_backhaul.own_addr())

def test_multi_ap_wps_split_psk(dev, apdev):
    """WPS on split fronthaul and backhaul AP"""
    backhaul_ssid = "multi-ap-backhaul-wps"
    backhaul_psk = "1234567890abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    params = hostapd.wpa2_params(ssid="multi-ap-fronthaul-wps",
                                 passphrase="12345678")
    params.update({"multi_ap": "2",
                   "multi_ap_backhaul_ssid": '"%s"' % backhaul_ssid,
                   "multi_ap_backhaul_wpa_psk": backhaul_psk})
    params_backhaul = hostapd.wpa2_params(ssid=backhaul_ssid)
    params_backhaul.update({"multi_ap": "1", "wpa_psk": backhaul_psk})
    hapd_backhaul = hostapd.add_ap(apdev[1], params_backhaul)

    run_multi_ap_wps(dev, apdev, params, hapd_backhaul.own_addr())

def test_multi_ap_wps_split_mixed(dev, apdev):
    """WPS on split fronthaul and backhaul AP with mixed-mode fronthaul"""
    backhaul_ssid = "multi-ap-backhaul-wps"
    backhaul_passphrase = "87654321"
    params = hostapd.wpa_mixed_params(ssid="multi-ap-fronthaul-wps",
                                      passphrase="12345678")
    params.update({"multi_ap": "2",
                   "multi_ap_backhaul_ssid": '"%s"' % backhaul_ssid,
                   "multi_ap_backhaul_wpa_passphrase": backhaul_passphrase})
    params_backhaul = hostapd.wpa2_params(ssid=backhaul_ssid,
                                          passphrase=backhaul_passphrase)
    params_backhaul.update({"multi_ap": "1"})
    hapd_backhaul = hostapd.add_ap(apdev[1], params_backhaul)

    run_multi_ap_wps(dev, apdev, params, hapd_backhaul.own_addr())

def test_multi_ap_wps_split_open(dev, apdev):
    """WPS on split fronthaul and backhaul AP with open fronthaul"""
    backhaul_ssid = "multi-ap-backhaul-wps"
    backhaul_passphrase = "87654321"
    params = {"ssid": "multi-ap-wps-fronthaul", "multi_ap": "2",
              "multi_ap_backhaul_ssid": '"%s"' % backhaul_ssid,
              "multi_ap_backhaul_wpa_passphrase": backhaul_passphrase}
    params_backhaul = hostapd.wpa2_params(ssid=backhaul_ssid,
                                          passphrase=backhaul_passphrase)
    params_backhaul.update({"multi_ap": "1"})
    hapd_backhaul = hostapd.add_ap(apdev[1], params_backhaul)

    run_multi_ap_wps(dev, apdev, params, hapd_backhaul.own_addr())

def test_multi_ap_wps_fail_non_multi_ap(dev, apdev):
    """Multi-AP WPS on non-WPS AP fails"""

    params = hostapd.wpa2_params(ssid="non-multi-ap-wps", passphrase="12345678")
    params.update({"wps_state": "2", "eap_server": "1"})

    hapd = hostapd.add_ap(apdev[0], params)
    hapd.request("WPS_PBC")
    if "PBC Status: Active" not in hapd.request("WPS_GET_STATUS"):
        raise Exception("PBC status not shown correctly")

    dev[0].scan_for_bss(apdev[0]['bssid'], freq="2412")
    dev[0].request("WPS_PBC %s multi_ap=1" % apdev[0]['bssid'])
    # Since we will fail to associate and WPS doesn't even get started, there
    # isn't much we can do except wait for timeout. For PBC, it is not possible
    # to change the timeout from 2 minutes. Instead of waiting for the timeout,
    # just check that WPS doesn't finish within reasonable time.
    for i in range(2):
        ev = dev[0].wait_event(["WPS-SUCCESS", "WPS-FAIL",
                                "CTRL-EVENT-DISCONNECTED"], timeout=10)
        if ev and "WPS-" in ev:
            raise Exception("WPS operation completed: " + ev)
    dev[0].request("WPS_CANCEL")
