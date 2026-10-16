:INPUT,FORWARD
-j SYNPROXY --sack-perm --timestamp --mss 1460 --wscale 9;;FAIL
-p tcp -m tcp --dport 42 -m conntrack --ctstate INVALID,UNTRACKED -j SYNPROXY --sack-perm --timestamp --wscale 9 --mss 1460;=;OK
