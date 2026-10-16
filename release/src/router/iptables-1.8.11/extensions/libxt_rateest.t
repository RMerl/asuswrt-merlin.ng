:INPUT,FORWARD,OUTPUT
%iptables -I INPUT -j RATEEST --rateest-name RE1 --rateest-interval 250.0ms --rateest-ewmalog 500.0ms
-m rateest --rateest RE1 --rateest-lt --rateest-bps 8bit;=;OK
-m rateest --rateest RE1 --rateest-eq --rateest-pps 5;=;OK
-m rateest --rateest RE1 --rateest-gt --rateest-bps 5kbit;-m rateest --rateest RE1 --rateest-gt --rateest-bps 5000bit;OK
-m rateest --rateest-delta --rateest RE1 --rateest-bps1 8bit --rateest-lt --rateest-bps2 16bit;=;OK
%iptables -I INPUT -j RATEEST --rateest-name RE2 --rateest-interval 250.0ms --rateest-ewmalog 500.0ms
-m rateest --rateest1 RE1 --rateest-lt --rateest-bps --rateest2 RE2;=;OK
-m rateest --rateest-delta --rateest1 RE1 --rateest-pps1 0 --rateest-lt --rateest-pps2 42 --rateest2 RE2;=;OK
-m rateest --rateest-delta --rateest RE1 --rateest-bps1 8bit --rateest-eq --rateest-bps2 16bit;=;OK
-m rateest --rateest-delta --rateest RE1 --rateest-bps1 8bit --rateest-gt --rateest-bps2 16bit;=;OK
-m rateest --rateest-delta --rateest RE1 --rateest-pps1 8 --rateest-lt --rateest-pps2 9;=;OK
-m rateest --rateest-delta --rateest RE1 --rateest-pps1 8 --rateest-eq --rateest-pps2 9;=;OK
-m rateest --rateest-delta --rateest RE1 --rateest-pps1 8 --rateest-gt --rateest-pps2 9;=;OK
%iptables -D INPUT -j RATEEST --rateest-name RE1 --rateest-interval 250.0ms --rateest-ewmalog 500.0ms
%iptables -D INPUT -j RATEEST --rateest-name RE2 --rateest-interval 250.0ms --rateest-ewmalog 500.0ms
