# Testing utilities
# Copyright (c) 2013-2019, Jouni Malinen <j@w1.fi>
#
# This software may be distributed under the terms of the BSD license.
# See README for more details.

import binascii
import os
import socket
import struct
import subprocess
import time
import remotehost
import logging
logger = logging.getLogger()

def get_ifnames():
    ifnames = []
    with open("/proc/net/dev", "r") as f:
        lines = f.readlines()
        for l in lines:
            val = l.split(':', 1)
            if len(val) == 2:
                ifnames.append(val[0].strip(' '))
    return ifnames

class HwsimSkip(Exception):
    def __init__(self, reason):
        self.reason = reason
    def __str__(self):
        return self.reason

class alloc_fail(object):
    def __init__(self, dev, count, funcs):
        self._dev = dev
        self._count = count
        self._funcs = funcs
    def __enter__(self):
        cmd = "TEST_ALLOC_FAIL %d:%s" % (self._count, self._funcs)
        if "OK" not in self._dev.request(cmd):
            raise HwsimSkip("TEST_ALLOC_FAIL not supported")
    def __exit__(self, type, value, traceback):
        if type is None:
            if self._dev.request("GET_ALLOC_FAIL") != "0:%s" % self._funcs:
                raise Exception("Allocation failure did not trigger")

class fail_test(object):
    def __init__(self, dev, count, funcs):
        self._dev = dev
        self._count = count
        self._funcs = funcs
    def __enter__(self):
        cmd = "TEST_FAIL %d:%s" % (self._count, self._funcs)
        if "OK" not in self._dev.request(cmd):
            raise HwsimSkip("TEST_FAIL not supported")
    def __exit__(self, type, value, traceback):
        if type is None:
            if self._dev.request("GET_FAIL") != "0:%s" % self._funcs:
                raise Exception("Test failure did not trigger")

def wait_fail_trigger(dev, cmd, note="Failure not triggered", max_iter=40,
		      timeout=0.05):
    for i in range(0, max_iter):
        if dev.request(cmd).startswith("0:"):
            break
        if i == max_iter - 1:
            raise Exception(note)
        time.sleep(timeout)

def require_under_vm():
    with open('/proc/1/cmdline', 'r') as f:
        cmd = f.read()
        if "inside.sh" not in cmd:
            raise HwsimSkip("Not running under VM")

def iface_is_in_bridge(bridge, ifname):
    fname = "/sys/class/net/"+ifname+"/brport/bridge"
    if not os.path.exists(fname):
        return False
    if not os.path.islink(fname):
        return False
    truebridge = os.path.basename(os.readlink(fname))
    if bridge == truebridge:
        return True
    return False

def skip_with_fips(dev, reason="Not supported in FIPS mode"):
    res = dev.get_capability("fips")
    if res and 'FIPS' in res:
        raise HwsimSkip(reason)

def get_phy(ap, ifname=None):
    phy = "phy3"
    try:
        hostname = ap['hostname']
    except:
        hostname = None
    host = remotehost.Host(hostname)

    if ifname == None:
        ifname = ap['ifname']
    status, buf = host.execute(["iw", "dev", ifname, "info"])
    if status != 0:
        raise Exception("iw " + ifname + " info failed")
    lines = buf.split("\n")
    for line in lines:
        if "wiphy" in line:
            words = line.split()
            phy = "phy" + words[1]
            break
    return phy

def parse_ie(buf):
    ret = {}
    data = binascii.unhexlify(buf)
    while len(data) >= 2:
        ie, elen = struct.unpack('BB', data[0:2])
        data = data[2:]
        if elen > len(data):
            break
        ret[ie] = data[0:elen]
        data = data[elen:]
    return ret

def wait_regdom_changes(dev):
    for i in range(10):
        ev = dev.wait_event(["CTRL-EVENT-REGDOM-CHANGE"], timeout=0.1)
        if ev is None:
            break

def clear_country(dev):
    logger.info("Try to clear country")
    id = dev[1].add_network()
    dev[1].set_network(id, "mode", "2")
    dev[1].set_network_quoted(id, "ssid", "country-clear")
    dev[1].set_network(id, "key_mgmt", "NONE")
    dev[1].set_network(id, "frequency", "2412")
    dev[1].set_network(id, "scan_freq", "2412")
    dev[1].select_network(id)
    ev = dev[1].wait_event(["CTRL-EVENT-CONNECTED"])
    if ev:
        dev[0].connect("country-clear", key_mgmt="NONE", scan_freq="2412")
        dev[1].request("DISCONNECT")
        dev[0].wait_disconnected()
        dev[0].request("DISCONNECT")
        dev[0].request("ABORT_SCAN")
        time.sleep(1)
        dev[0].dump_monitor()
        dev[1].dump_monitor()

def clear_regdom(hapd, dev, count=1):
    disable_hapd(hapd)
    clear_regdom_dev(dev, count)

def disable_hapd(hapd):
    if hapd:
        hapd.request("DISABLE")
        time.sleep(0.1)

def clear_regdom_dev(dev, count=1):
    for i in range(count):
        dev[i].request("DISCONNECT")
    for i in range(count):
        dev[i].disconnect_and_stop_scan()
    subprocess.call(['iw', 'reg', 'set', '00'])
    wait_regdom_changes(dev[0])
    country = dev[0].get_driver_status_field("country")
    logger.info("Country code at the end: " + country)
    if country != "00":
        clear_country(dev)
    for i in range(count):
        dev[i].flush_scan_cache()

def radiotap_build():
    radiotap_payload = struct.pack('BB', 0x08, 0)
    radiotap_payload += struct.pack('BB', 0, 0)
    radiotap_payload += struct.pack('BB', 0, 0)
    radiotap_hdr = struct.pack('<BBHL', 0, 0, 8 + len(radiotap_payload),
                               0xc002)
    return radiotap_hdr + radiotap_payload

def start_monitor(ifname, freq=2412):
    subprocess.check_call(["iw", ifname, "set", "type", "monitor"])
    subprocess.call(["ip", "link", "set", "dev", ifname, "up"])
    subprocess.check_call(["iw", ifname, "set", "freq", str(freq)])

    ETH_P_ALL = 3
    sock = socket.socket(socket.AF_PACKET, socket.SOCK_RAW,
                         socket.htons(ETH_P_ALL))
    sock.bind((ifname, 0))
    sock.settimeout(0.5)
    return sock

def stop_monitor(ifname):
    subprocess.call(["ip", "link", "set", "dev", ifname, "down"])
    subprocess.call(["iw", ifname, "set", "type", "managed"])
