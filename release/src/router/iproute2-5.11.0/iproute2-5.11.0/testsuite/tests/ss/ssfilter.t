#!/bin/sh

. lib/generic.sh

# % ./misc/ss -Htna
# LISTEN  0    128    0.0.0.0:22       0.0.0.0:*
# ESTAB   0    0     10.0.0.1:22      10.0.0.1:36266
# ESTAB   0    0     10.0.0.1:36266   10.0.0.1:22
# ESTAB   0    0     10.0.0.1:22      10.0.0.2:50312
export TCPDIAG_FILE="$(dirname $0)/ss1.dump"

ts_log "[Testing ssfilter]"

ts_ss "$0" "Match dport = 22" -Htna dport = 22
test_on "ESTAB 0      0      10.0.0.1:36266 10.0.0.1:22"

ts_ss "$0" "Match dport 22" -Htna dport 22
test_on "ESTAB 0      0      10.0.0.1:36266 10.0.0.1:22"

ts_ss "$0" "Match (dport)" -Htna '( dport = 22 )'
test_on "ESTAB 0      0      10.0.0.1:36266 10.0.0.1:22"

ts_ss "$0" "Match src = 0.0.0.0" -Htna src = 0.0.0.0
test_on "LISTEN 0      128    0.0.0.0:22 0.0.0.0:\*"

ts_ss "$0" "Match src 0.0.0.0" -Htna src 0.0.0.0
test_on "LISTEN 0      128    0.0.0.0:22 0.0.0.0:\*"

ts_ss "$0" "Match src sport" -Htna src 0.0.0.0 sport = 22
test_on "LISTEN 0      128    0.0.0.0:22 0.0.0.0:\*"

ts_ss "$0" "Match src and sport" -Htna src 0.0.0.0 and sport = 22
test_on "LISTEN 0      128    0.0.0.0:22 0.0.0.0:\*"

ts_ss "$0" "Match src and sport and dport" -Htna src 10.0.0.1 and sport = 22 and dport = 50312
test_on "ESTAB 0      0      10.0.0.1:22 10.0.0.2:50312"

ts_ss "$0" "Match src and sport and (dport)" -Htna 'src 10.0.0.1 and sport = 22 and ( dport = 50312 )'
test_on "ESTAB 0      0      10.0.0.1:22 10.0.0.2:50312"

ts_ss "$0" "Match src and (sport and dport)" -Htna 'src 10.0.0.1 and ( sport = 22 and dport = 50312 )'
test_on "ESTAB 0      0      10.0.0.1:22 10.0.0.2:50312"

ts_ss "$0" "Match (src and sport) and dport" -Htna '( src 10.0.0.1 and sport = 22 ) and dport = 50312'
test_on "ESTAB 0      0      10.0.0.1:22 10.0.0.2:50312"

ts_ss "$0" "Match (src or src) and dst" -Htna '( src 0.0.0.0 or src 10.0.0.1 ) and dst 10.0.0.2'
test_on "ESTAB 0      0      10.0.0.1:22 10.0.0.2:50312"
