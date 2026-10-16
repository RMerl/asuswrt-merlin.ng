:INPUT,FORWARD
-m physdev --physdev-in lo;=;OK
-m physdev --physdev-is-in --physdev-in lo;=;OK
:OUTPUT,FORWARD
# xt_physdev: using --physdev-out in the OUTPUT, FORWARD and POSTROUTING chains for non-bridged traffic is not supported anymore.
# ERROR: should fail: iptables -A FORWARD -m physdev --physdev-out lo
#-m physdev --physdev-out lo;;FAIL
# ERROR: cannot load: iptables -A OUTPUT -m physdev --physdev-is-out --physdev-out lo
#-m physdev --physdev-is-out --physdev-out lo;=;OK
:FORWARD
-m physdev --physdev-in lo --physdev-is-bridged;=;OK
:POSTROUTING
*mangle
-m physdev --physdev-out lo --physdev-is-bridged;=;OK
