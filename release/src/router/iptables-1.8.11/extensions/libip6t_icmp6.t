:INPUT,FORWARD,OUTPUT
-m icmpv6;;FAIL
-p ipv6-icmp -m icmp6 --icmpv6-type 1/0;=;OK
-p ipv6-icmp -m icmp6 --icmpv6-type 2;=;OK
# cannot use option twice:
-p ipv6-icmp -m icmp6 --icmpv6-type no-route --icmpv6-type packet-too-big;;FAIL
-p ipv6-icmp -m icmp6 --icmpv6-type mld-listener-query;-p ipv6-icmp -m icmp6 --icmpv6-type 130;OK
-p ipv6-icmp -m icmp6 --icmpv6-type mld-listener-report;-p ipv6-icmp -m icmp6 --icmpv6-type 131;OK
-p ipv6-icmp -m icmp6 --icmpv6-type mld-listener-done;-p ipv6-icmp -m icmp6 --icmpv6-type 132;OK
-p ipv6-icmp -m icmp6 --icmpv6-type mld-listener-reduction;-p ipv6-icmp -m icmp6 --icmpv6-type 132;OK
