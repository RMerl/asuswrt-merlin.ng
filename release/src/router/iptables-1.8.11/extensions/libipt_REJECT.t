:INPUT,FORWARD,OUTPUT
-j REJECT;-j REJECT --reject-with icmp-port-unreachable;OK
-j REJECT --reject-with icmp-net-unreachable;=;OK
-j REJECT --reject-with icmp-host-unreachable;=;OK
-j REJECT --reject-with icmp-port-unreachable;=;OK
-j REJECT --reject-with icmp-proto-unreachable;=;OK
-j REJECT --reject-with icmp-net-prohibited;=;OK
-j REJECT --reject-with icmp-host-prohibited;=;OK
-j REJECT --reject-with icmp-admin-prohibited;=;OK
