:INPUT,FORWARD,OUTPUT
-j REJECT;-j REJECT --reject-with icmp6-port-unreachable;OK
# manpage for IPv6 variant of REJECT does not show up for some reason?
-j REJECT --reject-with icmp6-no-route;=;OK
-j REJECT --reject-with icmp6-adm-prohibited;=;OK
-j REJECT --reject-with icmp6-addr-unreachable;=;OK
-j REJECT --reject-with icmp6-port-unreachable;=;OK
-j REJECT --reject-with icmp6-policy-fail;=;OK
-j REJECT --reject-with icmp6-reject-route;=;OK
-p tcp -j REJECT --reject-with tcp-reset;=;OK
-j REJECT --reject-with tcp-reset;;FAIL
