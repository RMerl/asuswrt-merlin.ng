:INPUT,FORWARD,OUTPUT
--stp-type 1;=;OK
! --stp-type 1;=;OK
--stp-flags 0x1;--stp-flags topology-change -j CONTINUE;OK
! --stp-flags topology-change;=;OK
--stp-root-prio 1 -j ACCEPT;=;OK
! --stp-root-prio 1 -j ACCEPT;=;OK
--stp-root-addr 0d:ea:d0:0b:ee:f0;=;OK
! --stp-root-addr 0d:ea:d0:0b:ee:f0;=;OK
--stp-root-addr 0d:ea:d0:00:00:00/ff:ff:ff:00:00:00;=;OK
! --stp-root-addr 0d:ea:d0:00:00:00/ff:ff:ff:00:00:00;=;OK
--stp-root-cost 1;=;OK
! --stp-root-cost 1;=;OK
--stp-sender-prio 1;=;OK
! --stp-sender-prio 1;=;OK
--stp-sender-addr de:ad:be:ef:00:00;=;OK
! --stp-sender-addr de:ad:be:ef:00:00;=;OK
--stp-sender-addr de:ad:be:ef:00:00/ff:ff:ff:ff:00:00;=;OK
! --stp-sender-addr de:ad:be:ef:00:00/ff:ff:ff:ff:00:00;=;OK
--stp-port 1;=;OK
! --stp-port 1;=;OK
--stp-msg-age 1;=;OK
! --stp-msg-age 1;=;OK
--stp-max-age 1;=;OK
! --stp-max-age 1;=;OK
--stp-hello-time 1;=;OK
! --stp-hello-time 1;=;OK
--stp-forward-delay 1;=;OK
! --stp-forward-delay 1;=;OK
--stp-root-prio :2;--stp-root-prio 0:2;OK
--stp-root-prio 2:;--stp-root-prio 2:65535;OK
--stp-root-prio 1:2;=;OK
--stp-root-prio 1:1;--stp-root-prio 1;OK
--stp-root-prio 2:1;;FAIL
--stp-root-cost :2;--stp-root-cost 0:2;OK
--stp-root-cost 2:;--stp-root-cost 2:4294967295;OK
--stp-root-cost 1:2;=;OK
--stp-root-cost 1:1;--stp-root-cost 1;OK
--stp-root-cost 2:1;;FAIL
--stp-sender-prio :2;--stp-sender-prio 0:2;OK
--stp-sender-prio 2:;--stp-sender-prio 2:65535;OK
--stp-sender-prio 1:2;=;OK
--stp-sender-prio 1:1;--stp-sender-prio 1;OK
--stp-sender-prio 2:1;;FAIL
--stp-port :;--stp-port 0:65535;OK
--stp-port :2;--stp-port 0:2;OK
--stp-port 2:;--stp-port 2:65535;OK
--stp-port 1:2;=;OK
--stp-port 1:1;--stp-port 1;OK
--stp-port 2:1;;FAIL
--stp-msg-age :;--stp-msg-age 0:65535;OK
--stp-msg-age :2;--stp-msg-age 0:2;OK
--stp-msg-age 2:;--stp-msg-age 2:65535;OK
--stp-msg-age 1:2;=;OK
--stp-msg-age 1:1;--stp-msg-age 1;OK
--stp-msg-age 2:1;;FAIL
--stp-max-age :;--stp-max-age 0:65535;OK
--stp-max-age :2;--stp-max-age 0:2;OK
--stp-max-age 2:;--stp-max-age 2:65535;OK
--stp-max-age 1:2;=;OK
--stp-max-age 1:1;--stp-max-age 1;OK
--stp-max-age 2:1;;FAIL
--stp-hello-time :;--stp-hello-time 0:65535;OK
--stp-hello-time :2;--stp-hello-time 0:2;OK
--stp-hello-time 2:;--stp-hello-time 2:65535;OK
--stp-hello-time 1:2;=;OK
--stp-hello-time 1:1;--stp-hello-time 1;OK
--stp-hello-time 2:1;;FAIL
--stp-forward-delay :;--stp-forward-delay 0:65535;OK
--stp-forward-delay :2;--stp-forward-delay 0:2;OK
--stp-forward-delay 2:;--stp-forward-delay 2:65535;OK
--stp-forward-delay 1:2;=;OK
--stp-forward-delay 1:1;--stp-forward-delay 1;OK
--stp-forward-delay 2:1;;FAIL
