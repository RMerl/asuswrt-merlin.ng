:INPUT,FORWARD,OUTPUT
-p icmp -m icmp --icmp-type any;=;OK
# XXX: output uses the number, better use the name?
-p icmp -m icmp --icmp-type echo-reply;-p icmp -m icmp --icmp-type 0;OK
-p icmp -m icmp --icmp-type destination-unreachable;-p icmp -m icmp --icmp-type 3;OK
# it does not acccept name/name, should we accept this?
# ERROR: cannot load: iptables -A INPUT -p icmp -m icmp --icmp-type destination-unreachable/network-unreachable
# -p icmp -m icmp --icmp-type destination-unreachable/network-unreachable;=;OK
-m icmp;;FAIL
# we accept "iptables -I INPUT -p tcp -m tcp", why not this below?
# ERROR: cannot load: iptables -A INPUT -p icmp -m icmp
# -p icmp -m icmp;=;OK
-p icmp -m icmp --icmp-type 255/255;=;OK
-p icmp -m icmp --icmp-type 255/0:255;-p icmp -m icmp --icmp-type any;OK
