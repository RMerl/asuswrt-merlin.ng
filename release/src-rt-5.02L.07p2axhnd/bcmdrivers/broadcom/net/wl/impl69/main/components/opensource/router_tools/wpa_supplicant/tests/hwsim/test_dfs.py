# Test cases for DFS
# Copyright (c) 2013-2019, Jouni Malinen <j@w1.fi>
#
# This software may be distributed under the terms of the BSD license.
# See README for more details.

from remotehost import remote_compatible
import os
import subprocess
import time
import logging
logger = logging.getLogger()

import hwsim_utils
import hostapd
from utils import *

def wait_dfs_event(hapd, event, timeout):
    dfs_events = ["DFS-RADAR-DETECTED", "DFS-NEW-CHANNEL",
                  "DFS-CAC-START", "DFS-CAC-COMPLETED",
                  "DFS-NOP-FINISHED", "AP-ENABLED", "AP-CSA-FINISHED"]
    ev = hapd.wait_event(dfs_events, timeout=timeout)
    if not ev:
        raise Exception("DFS event timed out")
    if event and event not in ev:
        raise Exception("Unexpected DFS event: " + ev + " (expected: %s)" % event)
    return ev

def start_dfs_ap(ap, ssid="dfs", ht=True, ht40=False,
                 ht40minus=False, vht80=False, vht20=False, chanlist=None,
                 channel=None, country="FI", rrm_beacon_report=False):
    ifname = ap['ifname']
    logger.info("Starting AP " + ifname + " on DFS channel")
    hapd = hostapd.add_ap(ap, {}, no_enable=True)
    hapd.set("ssid", ssid)
    hapd.set("country_code", country)
    hapd.set("ieee80211d", "1")
    hapd.set("ieee80211h", "1")
    hapd.set("hw_mode", "a")
    hapd.set("channel", "52")
    if not ht:
        hapd.set("ieee80211n", "0")
    if ht40:
        hapd.set("ht_capab", "[HT40+]")
    elif ht40minus:
        hapd.set("ht_capab", "[HT40-]")
        hapd.set("channel", "56")
    if vht80:
        hapd.set("ieee80211ac", "1")
        hapd.set("vht_oper_chwidth", "1")
        hapd.set("vht_oper_centr_freq_seg0_idx", "58")
    if vht20:
        hapd.set("ieee80211ac", "1")
        hapd.set("vht_oper_chwidth", "0")
        hapd.set("vht_oper_centr_freq_seg0_idx", "0")
    if chanlist:
        hapd.set("chanlist", chanlist)
    if channel:
        hapd.set("channel", str(channel))
    if rrm_beacon_report:
        hapd.set("rrm_beacon_report", "1")
    hapd.enable()

    ev = wait_dfs_event(hapd, "DFS-CAC-START", 5)
    if "DFS-CAC-START" not in ev:
        raise Exception("Unexpected DFS event: " + ev)

    state = hapd.get_status_field("state")
    if state != "DFS":
        raise Exception("Unexpected interface state: " + state)

    return hapd

def dfs_simulate_radar(hapd):
    logger.info("Trigger a simulated radar event")
    phyname = hapd.get_driver_status_field("phyname")
    radar_file = '/sys/kernel/debug/ieee80211/' + phyname + '/hwsim/dfs_simulate_radar'
    with open(radar_file, 'w') as f:
        f.write('1')

def test_dfs(dev, apdev):
    """DFS CAC functionality on clear channel"""
    try:
        hapd = None
        hapd = start_dfs_ap(apdev[0], country="US")

        ev = wait_dfs_event(hapd, "DFS-CAC-COMPLETED", 70)
        if "success=1" not in ev:
            raise Exception("CAC failed")
        if "freq=5260" not in ev:
            raise Exception("Unexpected DFS freq result")

        ev = hapd.wait_event(["AP-ENABLED"], timeout=5)
        if not ev:
            raise Exception("AP setup timed out")

        state = hapd.get_status_field("state")
        if state != "ENABLED":
            raise Exception("Unexpected interface state")

        freq = hapd.get_status_field("freq")
        if freq != "5260":
            raise Exception("Unexpected frequency")

        dev[0].connect("dfs", key_mgmt="NONE")
        dev[0].wait_regdom(country_ie=True)
        hwsim_utils.test_connectivity(dev[0], hapd)

        hapd.request("RADAR DETECTED freq=5260 ht_enabled=1 chan_width=1")
        ev = hapd.wait_event(["DFS-RADAR-DETECTED"], timeout=10)
        if ev is None:
            raise Exception("DFS-RADAR-DETECTED event not reported")
        if "freq=5260" not in ev:
            raise Exception("Incorrect frequency in radar detected event: " + ev)
        ev = hapd.wait_event(["DFS-NEW-CHANNEL"], timeout=70)
        if ev is None:
            raise Exception("DFS-NEW-CHANNEL event not reported")
        if "freq=5260" in ev:
            raise Exception("Channel did not change after radar was detected")

        ev = hapd.wait_event(["AP-CSA-FINISHED"], timeout=70)
        if ev is None:
            raise Exception("AP-CSA-FINISHED event not reported")
        if "freq=5260" in ev:
            raise Exception("Channel did not change after radar was detected(2)")
        time.sleep(1)
        hwsim_utils.test_connectivity(dev[0], hapd)
    finally:
        clear_regdom(hapd, dev)

def test_dfs_etsi(dev, apdev, params):
    """DFS and uniform spreading requirement for ETSI [long]"""
    if not params['long']:
        raise HwsimSkip("Skip test case with long duration due to --long not specified")
    try:
        hapd = None
        hapd = start_dfs_ap(apdev[0])

        ev = wait_dfs_event(hapd, "DFS-CAC-COMPLETED", 70)
        if "success=1" not in ev:
            raise Exception("CAC failed")
        if "freq=5260" not in ev:
            raise Exception("Unexpected DFS freq result")

        ev = hapd.wait_event(["AP-ENABLED"], timeout=5)
        if not ev:
            raise Exception("AP setup timed out")

        state = hapd.get_status_field("state")
        if state != "ENABLED":
            raise Exception("Unexpected interface state")

        freq = hapd.get_status_field("freq")
        if freq != "5260":
            raise Exception("Unexpected frequency")

        dev[0].connect("dfs", key_mgmt="NONE")
        dev[0].wait_regdom(country_ie=True)
        hwsim_utils.test_connectivity(dev[0], hapd)

        hapd.request("RADAR DETECTED freq=%s ht_enabled=1 chan_width=1" % freq)
        ev = hapd.wait_event(["DFS-RADAR-DETECTED"], timeout=5)
        if ev is None:
            raise Exception("DFS-RADAR-DETECTED event not reported")
        if "freq=%s" % freq not in ev:
            raise Exception("Incorrect frequency in radar detected event: " + ev)
        ev = hapd.wait_event(["DFS-NEW-CHANNEL"], timeout=5)
        if ev is None:
            raise Exception("DFS-NEW-CHANNEL event not reported")
        if "freq=%s" % freq in ev:
            raise Exception("Channel did not change after radar was detected")

        ev = hapd.wait_event(["AP-CSA-FINISHED", "DFS-CAC-START"], timeout=10)
        if ev is None:
            raise Exception("AP-CSA-FINISHED or DFS-CAC-START event not reported")
        if "DFS-CAC-START" in ev:
            # The selected new channel requires CAC
            ev = wait_dfs_event(hapd, "DFS-CAC-COMPLETED", 70)
            if "success=1" not in ev:
                raise Exception("CAC failed")

            ev = hapd.wait_event(["AP-ENABLED"], timeout=5)
            if not ev:
                raise Exception("AP setup timed out")
            ev = hapd.wait_event(["AP-STA-CONNECTED"], timeout=30)
            if not ev:
                raise Exception("STA did not reconnect on new DFS channel")
        else:
            # The new channel did not require CAC - try again
            if "freq=%s" % freq in ev:
                raise Exception("Channel did not change after radar was detected(2)")
            time.sleep(1)
        hwsim_utils.test_connectivity(dev[0], hapd)
    finally:
        clear_regdom(hapd, dev)

def test_dfs_radar1(dev, apdev):
    """DFS CAC functionality with radar detected during initial CAC"""
    try:
        hapd = None
        hapd = start_dfs_ap(apdev[0])
        time.sleep(1)

        dfs_simulate_radar(hapd)

        ev = wait_dfs_event(hapd, "DFS-CAC-COMPLETED", 5)
        if ev is None:
            raise Exception("Timeout on DFS aborted event")
        if "success=0 freq=5260" not in ev:
            raise Exception("Unexpected DFS aborted event contents: " + ev)

        ev = wait_dfs_event(hapd, "DFS-RADAR-DETECTED", 5)
        if "freq=5260" not in ev:
            raise Exception("Unexpected DFS radar detection freq")

        ev = wait_dfs_event(hapd, "DFS-NEW-CHANNEL", 5)
        if "freq=5260" in ev:
            raise Exception("Unexpected DFS new freq")

        ev = wait_dfs_event(hapd, None, 5)
        if "AP-ENABLED" in ev:
            logger.info("Started AP on non-DFS channel")
        else:
            logger.info("Trying to start AP on another DFS channel")
            if "DFS-CAC-START" not in ev:
                raise Exception("Unexpected DFS event: " + ev)
            if "freq=5260" in ev:
                raise Exception("Unexpected DFS CAC freq")

            ev = wait_dfs_event(hapd, "DFS-CAC-COMPLETED", 70)
            if "success=1" not in ev:
                raise Exception("CAC failed")
            if "freq=5260" in ev:
                raise Exception("Unexpected DFS freq result - radar channel")

            ev = hapd.wait_event(["AP-ENABLED"], timeout=5)
            if not ev:
                raise Exception("AP setup timed out")

            state = hapd.get_status_field("state")
            if state != "ENABLED":
                raise Exception("Unexpected interface state")

            freq = hapd.get_status_field("freq")
            if freq == "5260":
                raise Exception("Unexpected frequency: " + freq)

        dev[0].connect("dfs", key_mgmt="NONE")
        dev[0].wait_regdom(country_ie=True)
    finally:
        clear_regdom(hapd, dev)

def test_dfs_radar2(dev, apdev):
    """DFS CAC functionality with radar detected after initial CAC"""
    try:
        hapd = None
        hapd = start_dfs_ap(apdev[0], ssid="dfs2", ht40=True)

        ev = hapd.wait_event(["AP-ENABLED"], timeout=70)
        if not ev:
            raise Exception("AP2 setup timed out")

        dfs_simulate_radar(hapd)

        ev = wait_dfs_event(hapd, "DFS-RADAR-DETECTED", 5)
        if "freq=5260 ht_enabled=1 chan_offset=1 chan_width=2" not in ev:
            raise Exception("Unexpected DFS radar detection freq from AP2")

        ev = wait_dfs_event(hapd, "DFS-NEW-CHANNEL", 5)
        if "freq=5260" in ev:
            raise Exception("Unexpected DFS new freq for AP2")

        wait_dfs_event(hapd, None, 5)
    finally:
        clear_regdom(hapd, dev)

@remote_compatible
def test_dfs_radar_on_non_dfs_channel(dev, apdev):
    """DFS radar detection test code on non-DFS channel"""
    params = {"ssid": "radar"}
    hapd = hostapd.add_ap(apdev[0], params)

    hapd.request("RADAR DETECTED freq=5260 ht_enabled=1 chan_width=1")
    hapd.request("RADAR DETECTED freq=2412 ht_enabled=1 chan_width=1")

def test_dfs_radar_chanlist(dev, apdev):
    """DFS chanlist when radar is detected"""
    try:
        hapd = None
        hapd = start_dfs_ap(apdev[0], chanlist="40 44")
        time.sleep(1)

        dfs_simulate_radar(hapd)

        ev = wait_dfs_event(hapd, "DFS-CAC-COMPLETED", 5)
        if ev is None:
            raise Exception("Timeout on DFS aborted event")
        if "success=0 freq=5260" not in ev:
            raise Exception("Unexpected DFS aborted event contents: " + ev)

        ev = wait_dfs_event(hapd, "DFS-RADAR-DETECTED", 5)
        if "freq=5260" not in ev:
            raise Exception("Unexpected DFS radar detection freq")

        ev = wait_dfs_event(hapd, "DFS-NEW-CHANNEL", 5)
        if "freq=5200 chan=40" not in ev and "freq=5220 chan=44" not in ev:
            raise Exception("Unexpected DFS new freq: " + ev)

        ev = wait_dfs_event(hapd, None, 5)
        if "AP-ENABLED" not in ev:
            raise Exception("Unexpected DFS event: " + ev)
        dev[0].connect("dfs", key_mgmt="NONE")
        dev[0].wait_regdom(country_ie=True)
    finally:
        clear_regdom(hapd, dev)

def test_dfs_radar_chanlist_vht80(dev, apdev):
    """DFS chanlist when radar is detected and VHT80 configured"""
    try:
        hapd = None
        hapd = start_dfs_ap(apdev[0], chanlist="36", ht40=True, vht80=True)
        time.sleep(1)

        dfs_simulate_radar(hapd)

        ev = wait_dfs_event(hapd, "DFS-CAC-COMPLETED", 5)
        if ev is None:
            raise Exception("Timeout on DFS aborted event")
        if "success=0 freq=5260" not in ev:
            raise Exception("Unexpected DFS aborted event contents: " + ev)

        ev = wait_dfs_event(hapd, "DFS-RADAR-DETECTED", 5)
        if "freq=5260" not in ev:
            raise Exception("Unexpected DFS radar detection freq")

        ev = wait_dfs_event(hapd, "DFS-NEW-CHANNEL", 5)
        if "freq=5180 chan=36 sec_chan=1" not in ev:
            raise Exception("Unexpected DFS new freq: " + ev)

        ev = wait_dfs_event(hapd, None, 5)
        if "AP-ENABLED" not in ev:
            raise Exception("Unexpected DFS event: " + ev)
        dev[0].connect("dfs", key_mgmt="NONE")
        dev[0].wait_regdom(country_ie=True)

        if hapd.get_status_field('vht_oper_centr_freq_seg0_idx') != "42":
            raise Exception("Unexpected seg0 idx")
    finally:
        clear_regdom(hapd, dev)

def test_dfs_radar_chanlist_vht20(dev, apdev):
    """DFS chanlist when radar is detected and VHT40 configured"""
    try:
        hapd = None
        hapd = start_dfs_ap(apdev[0], chanlist="36", vht20=True)
        time.sleep(1)

        dfs_simulate_radar(hapd)

        ev = wait_dfs_event(hapd, "DFS-CAC-COMPLETED", 5)
        if ev is None:
            raise Exception("Timeout on DFS aborted event")
        if "success=0 freq=5260" not in ev:
            raise Exception("Unexpected DFS aborted event contents: " + ev)

        ev = wait_dfs_event(hapd, "DFS-RADAR-DETECTED", 5)
        if "freq=5260" not in ev:
            raise Exception("Unexpected DFS radar detection freq")

        ev = wait_dfs_event(hapd, "DFS-NEW-CHANNEL", 5)
        if "freq=5180 chan=36 sec_chan=0" not in ev:
            raise Exception("Unexpected DFS new freq: " + ev)

        ev = wait_dfs_event(hapd, None, 5)
        if "AP-ENABLED" not in ev:
            raise Exception("Unexpected DFS event: " + ev)
        dev[0].connect("dfs", key_mgmt="NONE")
        dev[0].wait_regdom(country_ie=True)
    finally:
        clear_regdom(hapd, dev)

def test_dfs_radar_no_ht(dev, apdev):
    """DFS chanlist when radar is detected and no HT configured"""
    try:
        hapd = None
        hapd = start_dfs_ap(apdev[0], chanlist="36", ht=False)
        time.sleep(1)

        dfs_simulate_radar(hapd)

        ev = wait_dfs_event(hapd, "DFS-CAC-COMPLETED", 5)
        if ev is None:
            raise Exception("Timeout on DFS aborted event")
        if "success=0 freq=5260" not in ev:
            raise Exception("Unexpected DFS aborted event contents: " + ev)

        ev = wait_dfs_event(hapd, "DFS-RADAR-DETECTED", 5)
        if "freq=5260 ht_enabled=0" not in ev:
            raise Exception("Unexpected DFS radar detection freq: " + ev)

        ev = wait_dfs_event(hapd, "DFS-NEW-CHANNEL", 5)
        if "freq=5180 chan=36 sec_chan=0" not in ev:
            raise Exception("Unexpected DFS new freq: " + ev)

        ev = wait_dfs_event(hapd, None, 5)
        if "AP-ENABLED" not in ev:
            raise Exception("Unexpected DFS event: " + ev)
        dev[0].connect("dfs", key_mgmt="NONE")
        dev[0].wait_regdom(country_ie=True)
    finally:
        clear_regdom(hapd, dev)

def test_dfs_radar_ht40minus(dev, apdev):
    """DFS chanlist when radar is detected and HT40- configured"""
    try:
        hapd = None
        hapd = start_dfs_ap(apdev[0], chanlist="36", ht40minus=True)
        time.sleep(1)

        dfs_simulate_radar(hapd)

        ev = wait_dfs_event(hapd, "DFS-CAC-COMPLETED", 5)
        if ev is None:
            raise Exception("Timeout on DFS aborted event")
        if "success=0 freq=5280 ht_enabled=1 chan_offset=-1" not in ev:
            raise Exception("Unexpected DFS aborted event contents: " + ev)

        ev = wait_dfs_event(hapd, "DFS-RADAR-DETECTED", 5)
        if "freq=5280 ht_enabled=1 chan_offset=-1" not in ev:
            raise Exception("Unexpected DFS radar detection freq: " + ev)

        ev = wait_dfs_event(hapd, "DFS-NEW-CHANNEL", 5)
        if "freq=5180 chan=36 sec_chan=1" not in ev:
            raise Exception("Unexpected DFS new freq: " + ev)

        ev = wait_dfs_event(hapd, None, 5)
        if "AP-ENABLED" not in ev:
            raise Exception("Unexpected DFS event: " + ev)
        dev[0].connect("dfs", key_mgmt="NONE")
        dev[0].wait_regdom(country_ie=True)
        dev[0].request("STA_AUTOCONNECT 0")
    finally:
        clear_regdom(hapd, dev)
        dev[0].request("STA_AUTOCONNECT 1")

def test_dfs_ht40_minus(dev, apdev, params):
    """DFS CAC functionality on channel 104 HT40- [long]"""
    if not params['long']:
        raise HwsimSkip("Skip test case with long duration due to --long not specified")
    try:
        hapd = None
        hapd = start_dfs_ap(apdev[0], ht40minus=True, channel=104)

        ev = wait_dfs_event(hapd, "DFS-CAC-COMPLETED", 70)
        if "success=1" not in ev:
            raise Exception("CAC failed")
        if "freq=5520" not in ev:
            raise Exception("Unexpected DFS freq result")

        ev = hapd.wait_event(["AP-ENABLED"], timeout=5)
        if not ev:
            raise Exception("AP setup timed out")

        state = hapd.get_status_field("state")
        if state != "ENABLED":
            raise Exception("Unexpected interface state")

        freq = hapd.get_status_field("freq")
        if freq != "5520":
            raise Exception("Unexpected frequency")

        dev[0].connect("dfs", key_mgmt="NONE", scan_freq="5520")
        dev[0].wait_regdom(country_ie=True)
        hwsim_utils.test_connectivity(dev[0], hapd)
    finally:
        clear_regdom(hapd, dev)

def test_dfs_cac_restart_on_enable(dev, apdev):
    """DFS CAC interrupted and restarted"""
    try:
        hapd = None
        hapd = start_dfs_ap(apdev[0])
        time.sleep(0.1)
        subprocess.check_call(['ip', 'link', 'set', 'dev', hapd.ifname, 'down'])
        ev = wait_dfs_event(hapd, "DFS-CAC-COMPLETED", 5)
        if ev is None:
            raise Exception("Timeout on DFS aborted event")
        if "success=0 freq=5260" not in ev:
            raise Exception("Unexpected DFS aborted event contents: " + ev)
        time.sleep(0.1)
        subprocess.check_call(['ip', 'link', 'set', 'dev', hapd.ifname, 'up'])

        ev = wait_dfs_event(hapd, "DFS-CAC-START", 5)
        if "DFS-CAC-START" not in ev:
            raise Exception("Unexpected DFS event: " + ev)
        hapd.disable()

    finally:
        clear_regdom(hapd, dev)

def test_dfs_rrm(dev, apdev, params):
    """DFS with RRM [long]"""
    if not params['long']:
        raise HwsimSkip("Skip test case with long duration due to --long not specified")
    try:
        hapd = None
        hapd = start_dfs_ap(apdev[0], country="US", rrm_beacon_report=True)

        ev = wait_dfs_event(hapd, "DFS-CAC-COMPLETED", 70)
        if "success=1" not in ev or "freq=5260" not in ev:
            raise Exception("Unexpected DFS freq result")
        ev = hapd.wait_event(["AP-ENABLED"], timeout=5)
        if not ev:
            raise Exception("AP setup timed out")

        dev[0].connect("dfs", key_mgmt="NONE", scan_freq="5260")
        dev[0].wait_regdom(country_ie=True)
        hapd.wait_sta()
        hwsim_utils.test_connectivity(dev[0], hapd)
        addr = dev[0].own_addr()
        token = hapd.request("REQ_BEACON " + addr + " " + "51000000000002ffffffffffff")
        if "FAIL" in token:
            raise Exception("REQ_BEACON failed")
        ev = hapd.wait_event(["BEACON-RESP-RX"], timeout=10)
        if ev is None:
            raise Exception("Beacon report response not received")
    finally:
        clear_regdom(hapd, dev)
