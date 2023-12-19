# $Id$
MAKEFLAGS=-s -k
PRINTF = @PRINTF@
HOST = @host@
TARGET = @target@
BUILD = @build@
nic1 = @nic1@
nic2 = @nic2@

ENABLE_DEBUG = @debug_run_time_flag@
TCPPREP=../src/tcpprep --no-arg-comment
TCPREPLAY=../src/tcpreplay
TCPREWRITE=../src/tcprewrite
TCPBRIDGE=../src/tcpbridge

TEST_PCAP=$(srcdir)/test.pcap

EXTRA_DIST = test.pcap test.auto_bridge test.auto_client test.auto_router \
		test.auto_server test.auto_first test.cidr test.comment test.port test.mac \
		test.cidr_reverse test.mac_reverse test.regex_reverse \
		test.include_packets test.exclude_packets test.include_source \
		test.include_dest test.prep_config \
		test.primary test.regex test.secondary \
		test.rewrite_seed test.rewrite_portmap test.rewrite_endpoint \
		test.rewrite_pnat test.rewrite_pad test.rewrite_trunc \
		test.rewrite_mac test.rewrite_layer2 test.rewrite_config \
		test.rewrite_skip test.rewrite_dltuser test.rewrite_dlthdlc \
		test.rewrite_vlan802.1ad test.rewrite_vlandel test.rewrite_efcs test.rewrite_1ttl \
		test.rewrite_2ttl test.rewrite_3ttl test.rewrite_enet_subsmac \
		test.rewrite_mtutrunc test.rewrite_mac_seed test.rewrite_range_portmap \
		test.rewrite_mac_seed_keep test.rewrite_l7fuzzing test.rewrite_sequence test.rewrite_fixcsum \
		test.rewrite_fixlen_pad test.rewrite_fixlen_trunc test.rewrite_fixlen_del \
		test2.rewrite_seed test2.rewrite_portmap test2.rewrite_endpoint \
		test2.rewrite_pnat test2.rewrite_pad test2.rewrite_trunc \
		test2.rewrite_mac test2.rewrite_layer2 test2.rewrite_config \
		test2.rewrite_skip test2.rewrite_dltuser test2.rewrite_dlthdlc \
		test2.rewrite_vlan802.1ad test2.rewrite_vlandel test2.rewrite_efcs test2.rewrite_1ttl \
		test2.rewrite_mtutrunc test2.rewrite_enet_subsmac \
		test2.rewrite_2ttl test2.rewrite_3ttl test.rewrite_tos test2.rewrite_tos \
		test2.rewrite_enet_subsmac test2.rewrite_mac_seed \
		test2.rewrite_range_portmap test2.rewrite_mac_seed_keep \
		test2.rewrite_l7fuzzing test2.rewrite_sequence test2.rewrite_fixcsum \
		test2.rewrite_fixlen_pad test2.rewrite_fixlen_trunc test2.rewrite_fixlen_del

test: all
all: clearlog check tcpprep tcpreplay tcprewrite

clearlog:
	-rm test.log

check:
	$(PRINTF) "%s\n" "NOTICE: Tests must be run as root"
	$(PRINTF) "%s\n" "Sending traffic on '$(nic1)' and '$(nic2)'"

if WORDS_BIGENDIAN
STANDARD_REWRITE = standard_bigendian
REWRITE_WARN = "big"
else
STANDARD_REWRITE = standard_littleendian
REWRITE_WARN = "little"
endif

standard: standard_prep $(STANDARD_REWRITE)
	$(PRINTF) "Warning: only creating %s endian standard test files\n" $(REWRITE_WARN)

standard_prep:
	$(TCPPREP) -i $(TEST_PCAP) -o test.auto_router -a router
	$(TCPPREP) -i $(TEST_PCAP) -o test.auto_bridge -a bridge
	$(TCPPREP) -i $(TEST_PCAP) -o test.auto_client -a client
	$(TCPPREP) -i $(TEST_PCAP) -o test.auto_server -a server
	$(TCPPREP) -i $(TEST_PCAP) -o test.auto_first -a first
	$(TCPPREP) --load-opts config -o test.prep_config
	$(TCPPREP) -i $(TEST_PCAP) -o test.port -p
	$(TCPPREP) -i $(TEST_PCAP) -o test.mac -e 00:1f:f3:3c:e1:13
	$(TCPPREP) -i $(TEST_PCAP) -o test.cidr -c '96.17.211.0/24'
	$(TCPPREP) -i $(TEST_PCAP) -o test.regex -r '96.17.211.*'
	$(TCPPREP) -i $(TEST_PCAP) -o test.comment -C "This is a comment" -p
	$(TCPPREP) -i $(TEST_PCAP) -o test.mac_reverse -e 00:1f:f3:3c:e1:13 --reverse
	$(TCPPREP) -i $(TEST_PCAP) -o test.cidr_reverse -c '96.17.211.0/24' --reverse
	$(TCPPREP) -i $(TEST_PCAP) -o test.regex_reverse -r '96.17.211.*' --reverse
	$(TCPPREP) -i $(TEST_PCAP) -o test.exclude_packets -c '96.17.211.0/24' --exclude 'P:61-65,88-91'
	$(TCPPREP) -i $(TEST_PCAP) -o test.include_packets -c '96.17.211.0/24' --include 'P:61-65,88-91'
	$(TCPPREP) -i $(TEST_PCAP) -o test.include_source -c '96.17.211.0/24' --include 'S:96.0.0.0/8'
	$(TCPPREP) -i $(TEST_PCAP) -o test.include_dest -c '96.17.211.0/24' --include 'D:96.0.0.0/8'

standard_bigendian:
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_seed -s 55
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_tos --tos=50
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_portmap -r 80:8080
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_range_portmap -r 1-100:49148
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_sequence --tcp-sequence 42
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_endpoint \
		-e 10.10.0.1:10.10.0.2 -c test.auto_router
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_pnat \
		-N 96.17.211.0/24:172.16.0.0/24
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_pad -F pad
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_trunc -F trunc
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_mac \
		--enet-dmac=00:12:13:14:15:16,00:22:33:44:55:66 \
		--enet-smac=00:22:33:44:55:66,00:12:13:14:15:16  -c $(srcdir)/test.auto_router
	 $(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_enet_subsmac \
		--enet-subsmac=00:1f:f3:3c:e1:13,00:22:33:44:55:66 \
		--enet-subsmac=f8:1e:df:e5:84:3a,00:66:55:44:33:22
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_mac_seed --enet-mac-seed=42
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_mac_seed_keep --enet-mac-seed=42 \
		--enet-mac-seed-keep-bytes=3
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_layer2 \
		 --dlt=user --user-dlink=00,50,da,5d,46,55,0,7,eb,30,a4,c3,08,0
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_config --enet-vlan=add --enet-vlan-tag=45 \
		--enet-vlan-cfi=1 --enet-vlan-pri=5
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_skip \
	    --skipbroadcast --skipl2broadcast --skip-soft-errors --seed 55 \
		--enet-dmac=00:12:13:14:15:16,00:22:33:44:55:66 \
		--enet-smac=00:22:33:44:55:66,00:12:13:14:15:16  -c $(srcdir)/test.auto_router
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_dltuser --dlt=user \
		--user-dlink=0x0f,0x00,0x08,0x00 --user-dlt=104
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_dlthdlc --dlt=hdlc \
		--hdlc-control=0 --hdlc-address=0x0F
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_vlan802.1ad \
		--enet-vlan=add --enet-vlan-tag=42 --enet-vlan-cfi=1 \
		--enet-vlan-pri=2 --enet-vlan-proto=802.1ad
	$(TCPREWRITE) -i $(srcdir)/test.rewrite_config -o test.rewrite_vlandel \
		--enet-vlan=del
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_efcs --efcs
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_1ttl --ttl=58
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_2ttl --ttl=+58
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_3ttl --ttl=-58
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_mtutrunc --mtu-trunc --mtu=300
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_l7fuzzing \
		--fuzz-seed=42 --fuzz-factor=2
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_fixcsum --fixcsum
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_fixlen_pad --fixlen=pad
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_fixlen_trunc --fixlen=trunc
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.rewrite_fixlen_del --fixlen=del

standard_littleendian:
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_seed -s 55
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_tos --tos=50
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_portmap -r 80:8080
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_range_portmap -r 1-100:49148
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_sequence --tcp-sequence 42
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_endpoint \
		-e 10.10.0.1:10.10.0.2 -c test.auto_router
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_pnat \
		-N 96.17.211.0/24:172.16.0.0/24
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_pad -F pad
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_trunc -F trunc
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_mac \
		--enet-dmac=00:12:13:14:15:16,00:22:33:44:55:66 \
		--enet-smac=00:22:33:44:55:66,00:12:13:14:15:16  -c $(srcdir)/test.auto_router
	 $(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_enet_subsmac \
		--enet-subsmac=00:1f:f3:3c:e1:13,00:22:33:44:55:66 \
		--enet-subsmac=f8:1e:df:e5:84:3a,00:66:55:44:33:22
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_mac_seed --enet-mac-seed=42
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_mac_seed_keep --enet-mac-seed=42 \
		--enet-mac-seed-keep-bytes=3
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_layer2 \
		 --dlt=user --user-dlink=00,50,da,5d,46,55,0,7,eb,30,a4,c3,08,0
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_config --enet-vlan=add --enet-vlan-tag=45 \
		--enet-vlan-cfi=1 --enet-vlan-pri=5
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_skip \
	    --skipbroadcast --skipl2broadcast --skip-soft-errors --seed 55 \
		--enet-dmac=00:12:13:14:15:16,00:22:33:44:55:66 \
		--enet-smac=00:22:33:44:55:66,00:12:13:14:15:16  -c $(srcdir)/test.auto_router
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_dltuser --dlt=user \
		--user-dlink=0x0f,0x00,0x08,0x00 --user-dlt=104
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_dlthdlc --dlt=hdlc \
		--hdlc-control=0 --hdlc-address=0x0F
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_vlan802.1ad \
		--enet-vlan=add --enet-vlan-tag=42 --enet-vlan-cfi=1 \
		--enet-vlan-pri=2 --enet-vlan-proto=802.1ad
	$(TCPREWRITE) -i $(srcdir)/test.rewrite_config -o test2.rewrite_vlandel \
		--enet-vlan=del
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_efcs --efcs
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_1ttl --ttl=58
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_2ttl --ttl=+58
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_3ttl --ttl=-58
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_mtutrunc --mtu-trunc --mtu=300
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_l7fuzzing \
		--fuzz-seed=42 --fuzz-factor=2
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_fixcsum --fixcsum
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_fixlen_pad --fixlen=pad
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_fixlen_trunc --fixlen=trunc
	$(TCPREWRITE) -i $(TEST_PCAP) -o test2.rewrite_fixlen_del --fixlen=del

tcpprep: auto_router auto_bridge auto_client auto_server auto_first cidr regex \
	port mac comment print_info print_comment prep_config \
	mac_reverse cidr_reverse regex_reverse exclude_packets \
	include_packets include_source include_dest

tcprewrite: rewrite_portmap rewrite_range_portmap rewrite_endpoint \
	rewrite_pnat rewrite_trunc rewrite_pad rewrite_seed rewrite_mac \
	rewrite_layer2 rewrite_config rewrite_skip rewrite_dltuser rewrite_dlthdlc \
	rewrite_vlan802.1ad rewrite_vlandel rewrite_efcs rewrite_1ttl rewrite_2ttl rewrite_3ttl \
	rewrite_tos rewrite_mtutrunc rewrite_enet_subsmac rewrite_mac_seed \
	rewrite_mac_seed_keep rewrite_l7fuzzing rewrite_sequence rewrite_fixcsum \
	rewrite_fixlen_pad rewrite_fixlen_trunc rewrite_fixlen_del

tcpreplay: replay_basic replay_cache replay_pps replay_rate replay_top \
	replay_config replay_multi replay_pps_multi replay_precache \
	replay_stats replay_dualfile replay_maxsleep

prep_config:
	$(PRINTF) "%s" "[tcpprep] Config mode test: "
	$(PRINTF) "%s\n" "*** [tcpprep] Config mode test: " >> test.log
	$(TCPPREP) $(ENABLE_DEBUG) --load-opts=config -o test.$@1  >> test.log 2>&1
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

auto_router:
	$(PRINTF) "%s" "[tcpprep] Auto/Router mode test: "
	$(PRINTF) "%s\n" "*** [tcpprep] Auto/Router mode test: " >> test.log
	$(TCPPREP) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 -a router  >> test.log 2>&1
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

auto_bridge:
	$(PRINTF) "%s" "[tcpprep] Auto/Bridge mode test: "
	$(PRINTF) "%s\n" "*** [tcpprep] Auto/Bridge mode test: " >> test.log
	$(TCPPREP) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 -a bridge  >> test.log 2>&1
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

auto_client:
	$(PRINTF) "%s" "[tcpprep] Auto/Client mode test: "
	$(PRINTF) "%s\n" "*** [tcpprep] Auto/Client mode test: " >> test.log
	$(TCPPREP) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 -a client  >> test.log 2>&1
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

auto_server:
	$(PRINTF) "%s" "[tcpprep] Auto/Server mode test: "
	$(PRINTF) "%s\n" "*** [tcpprep] Auto/Server mode test: " >> test.log
	$(TCPPREP) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 -a server >> test.log 2>&1
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

auto_first:
	$(PRINTF) "%s" "[tcpprep] Auto/First mode test: "
	$(PRINTF) "%s\n" "*** [tcpprep] Auto/First mode test: " >> test.log
	$(TCPPREP) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 -a first >> test.log 2>&1
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

regex:
	$(PRINTF) "%s" "[tcpprep] Regex mode test: "
	$(PRINTF) "%s\n" "*** [tcpprep] Regex mode test: " >> test.log
	$(TCPPREP) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 -r '96.17.211.*' >> test.log 2>&1
	diff  $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

cidr:
	$(PRINTF) "%s" "[tcpprep] CIDR mode test: "
	$(PRINTF) "%s\n" "*** [tcpprep] CIDR mode test: " >> test.log
	$(TCPPREP) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 -c '96.17.211.0/24' >> test.log 2>&1
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

port:
	$(PRINTF) "%s" "[tcpprep] Port mode test: "
	$(PRINTF) "%s\n" "*** [tcpprep] Port mode test: " >> test.log
	$(TCPPREP) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 -p >> test.log 2>&1
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

mac:
	$(PRINTF) "%s" "[tcpprep] MAC mode test: "
	$(PRINTF) "%s\n" "*** [tcpprep] MAC mode test: " >> test.log
	$(TCPPREP) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 -e 00:1f:f3:3c:e1:13 >> test.log 2>&1
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

comment:
	$(PRINTF) "%s" "[tcpprep] Comment mode test: "
	$(PRINTF) "%s\n" "*** [tcpprep] Comment mode test: " >> test.log
	$(TCPPREP) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 -C "This is a comment" -p >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

print_comment:
	$(PRINTF) "%s" "[tcpprep] Print comment mode test: "
	$(PRINTF) "%s\n" "*** [tcpprep] Print comment mode test: " >> test.log
	$(TCPPREP) $(ENABLE_DEBUG) -P $(srcdir)/test.comment >$(srcdir)/test.$@1 >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

print_info:
	$(PRINTF) "%s" "[tcpprep] Print info mode test: "
	$(PRINTF) "%s\n" "*** [tcpprep] Print info mode test: " >> test.log
	$(TCPPREP) $(ENABLE_DEBUG) -I $(srcdir)/test.comment >$(srcdir)/test.$@1 >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi


regex_reverse:
	$(PRINTF) "%s" "[tcpprep] Regex reverse mode test: "
	$(PRINTF) "%s\n" "*** [tcpprep] Regex reverse mode test: " >> test.log
	$(TCPPREP) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 -r '96.17.211.*' --reverse >> test.log 2>&1
	diff  $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

cidr_reverse:
	$(PRINTF) "%s" "[tcpprep] CIDR reverse mode test: "
	$(PRINTF) "%s\n" "*** [tcpprep] CIDR reverse mode test: " >> test.log
	$(TCPPREP) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 -c '96.17.211.0/24' --reverse >> test.log 2>&1
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

mac_reverse:
	$(PRINTF) "%s" "[tcpprep] MAC reverse mode test: "
	$(PRINTF) "%s\n" "*** [tcpprep] MAC reverse mode test: " >> test.log
	$(TCPPREP) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 -e 00:1f:f3:3c:e1:13 --reverse >> test.log 2>&1
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

exclude_packets:
	$(PRINTF) "%s" "[tcpprep] exclude packets test: "
	$(PRINTF) "%s\n" "*** [tcpprep] exclude packets test: " >> test.log
	$(TCPPREP) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 -c '96.17.211.0/24' --exclude 'P:61-65,88-91' >> test.log 2>&1
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

include_packets:
	$(PRINTF) "%s" "[tcpprep] include packets test: "
	$(PRINTF) "%s\n" "*** [tcpprep] include packets test: " >> test.log
	$(TCPPREP) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 -c '96.17.211.0/24' --include 'P:61-65,88-91' >> test.log 2>&1
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

include_source:
	$(PRINTF) "%s" "[tcpprep] include source test: "
	$(PRINTF) "%s\n" "*** [tcpprep] include source test: " >> test.log
	$(TCPPREP) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 -c '96.17.211.0/24' --include 'S:96.0.0.0/8' >> test.log 2>&1
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

include_dest:
	$(PRINTF) "%s" "[tcpprep] include destination test: "
	$(PRINTF) "%s\n" "*** [tcpprep] include destination test: " >> test.log
	$(TCPPREP) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 -c '96.17.211.0/24' --include 'D:96.0.0.0/8' >> test.log 2>&1
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

replay_basic:
	$(PRINTF) "%s" "[tcpreplay] Basic test: "
	$(PRINTF) "%s\n" "*** [tcpreplay] Basic test: " >> test.log
	$(TCPREPLAY) $(ENABLE_DEBUG) -i $(nic1) -t $(TEST_PCAP) >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

replay_cache:
	$(PRINTF) "%s" "[tcpreplay] Cache test: "
	$(PRINTF) "%s\n" "*** [tcpreplay] Cache test: " >> test.log
	$(TCPREPLAY) $(ENABLE_DEBUG) -c $(srcdir)/test.cidr -i $(nic1) -I $(nic2) -t $(TEST_PCAP) >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

replay_accurate:
	$(PRINTF) "%s" "[tcpreplay] Accurate test: "
	$(PRINTF) "%s\n" "*** [tcpreplay] Accurate test: " >> test.log
	$(TCPREPLAY) $(ENABLE_DEBUG) -a -i $(nic1) $(TEST_PCAP) >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

replay_stats:
	$(PRINTF) "%s" "[tcpreplay] Statistics test: "
	$(PRINTF) "%s\n" "*** [tcpreplay] Statistics test: " >> test.log
	$(TCPREPLAY) $(ENABLE_DEBUG) --stats=1 -i $(nic1) $(TEST_PCAP) >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

rewrite_portmap:
	$(PRINTF) "%s" "[tcprewrite] Portmap test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] Portmap test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 -r 80:8080 >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

rewrite_range_portmap:
	$(PRINTF) "%s" "[tcprewrite] Portmap range test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] Portmap range test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 -r 1-100:49148 >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

rewrite_sequence:
	$(PRINTF) "%s" "[tcprewrite] TCP sequence test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] TCP sequence test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 --tcp-sequence 42 >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

rewrite_endpoint:
	$(PRINTF) "%s" "[tcprewrite] Endpoint test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] Endpoint test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 -e 10.10.0.1:10.10.0.2 \
	    -c $(srcdir)/test.auto_router  >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

rewrite_pnat:
	$(PRINTF) "%s" "[tcprewrite] Pseudo NAT test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] Pseudo NAT test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 \
	    -N 96.17.211.0/24:172.16.0.0/24  >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

rewrite_mac:
	$(PRINTF) "%s" "[tcprewrite] Src/Dst MAC test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] Src/Dst MAC test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 \
		--enet-dmac=00:12:13:14:15:16,00:22:33:44:55:66 \
		--enet-smac=00:22:33:44:55:66,00:12:13:14:15:16  -c $(srcdir)/test.auto_router  >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

rewrite_enet_subsmac:
	$(PRINTF) "%s" "[tcprewrite] Substitute Src/Dst MAC test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] Substitute Src/Dst MAC test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 \
		--enet-subsmac=00:1f:f3:3c:e1:13,00:22:33:44:55:66 \
		--enet-subsmac=f8:1e:df:e5:84:3a,00:66:55:44:33:22  >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t%s\n" "FAILED"; else $(PRINTF) "\t%s\n" "OK"; fi

rewrite_mac_seed:
	$(PRINTF) "%s" "[tcprewrite] Seeded MAC test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] Seeded MAC test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 \
		--enet-mac-seed=42 >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

rewrite_mac_seed_keep:
	$(PRINTF) "%s" "[tcprewrite] Seeded Keep MAC test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] Seeded Keep MAC test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 \
		--enet-mac-seed=42 --enet-mac-seed-keep-bytes=3 >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

rewrite_trunc:
	$(PRINTF) "%s" "[tcprewrite] Truncate test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] Truncate test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -F trunc -i $(TEST_PCAP) -o test.$@1 >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

rewrite_pad:
	$(PRINTF) "%s" "[tcprewrite] Pad test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] Pad test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -F pad -i $(TEST_PCAP) -o test.$@1 >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t\t%s\n" "OK"; fi

rewrite_seed:
	$(PRINTF) "%s" "[tcprewrite] Seed IP test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] Seed IP test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 -s 55 >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

rewrite_layer2:
	$(PRINTF) "%s" "[tcprewrite] Layer2 test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] Layer2 test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) --dlt=user --user-dlink=00,50,da,5d,46,55,0,7,eb,30,a4,c3,08,0 \
		-i $(TEST_PCAP) -o test.$@1 >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

rewrite_config:
	$(PRINTF) "%s" "[tcprewrite] Config/VLAN Add test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] Config/VLAN Add test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) --load-opts config  >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

rewrite_skip:
	$(PRINTF) "%s" "[tcprewrite] Skip bcast test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] Skip bcast test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 \
	    --skipbroadcast --skipl2broadcast --skip-soft-errors --seed 55 \
		--enet-dmac=00:12:13:14:15:16,00:22:33:44:55:66 \
		--enet-smac=00:22:33:44:55:66,00:12:13:14:15:16  -c $(srcdir)/test.auto_router  >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

rewrite_dltuser:
	$(PRINTF) "%s" "[tcprewrite] DLT User test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] DLT User test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 --dlt=user \
		--user-dlink=0x0f,0x00,0x08,0x00 --user-dlt=104  >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

rewrite_dlthdlc:
	$(PRINTF) "%s" "[tcprewrite] DLT Cisco HDLC test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] DLT Cisco HDLC test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 --dlt=hdlc \
		--hdlc-control=0 --hdlc-address=0x0F  >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

rewrite_vlan802.1ad:
	$(PRINTF) "%s" "[tcprewrite] VLAN 802.1ad test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] VLAN 802.1ad test: " >> test.log
	$(TCPREWRITE) -i $(TEST_PCAP) -o test.$@1 \
		--enet-vlan=add --enet-vlan-tag=42 --enet-vlan-cfi=1 \
		--enet-vlan-pri=2 --enet-vlan-proto=802.1ad
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

rewrite_vlandel:
	$(PRINTF) "%s" "[tcprewrite] VLAN Delete test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] VLAN Delete test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(srcdir)/test.rewrite_config -o test.$@1 \
		--enet-vlan=del  >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

rewrite_efcs:
	$(PRINTF) "%s" "[tcprewrite] Remove EFCS: "
	$(PRINTF) "%s\n" "*** [tcprewrite] Remove EFCS: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 --efcs >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

rewrite_1ttl:
	$(PRINTF) "%s" "[tcprewrite] Force TTL: "
	$(PRINTF) "%s\n" "*** [tcprewrite] Force TTL: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 --ttl=58  >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

rewrite_2ttl:
	$(PRINTF) "%s" "[tcprewrite] Increase TTL: "
	$(PRINTF) "%s\n" "*** [tcprewrite] Increase TTL: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 --ttl=+58  >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

rewrite_3ttl:
	$(PRINTF) "%s" "[tcprewrite] Reduce TTL: "
	$(PRINTF) "%s\n" "*** [tcprewrite] Reduce TTL: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 --ttl=-58  >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

rewrite_tos:
	$(PRINTF) "%s" "[tcprewrite] TOS test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] TOS test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 --tos=50  >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t\t%s\n" "OK"; fi

rewrite_mtutrunc:
	$(PRINTF) "%s" "[tcprewrite] MTU Truncate test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] MTU Truncate test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1  --mtu-trunc --mtu=300 >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

rewrite_l7fuzzing:
	$(PRINTF) "%s" "[tcprewrite] L7 fuzzing test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] L7 fuzzing test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 --fuzz-seed=42 --fuzz-factor=2 >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

rewrite_fixcsum:
	$(PRINTF) "%s" "[tcprewrite] Fix checksum test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] Fix checksum test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 --fixcsum >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

rewrite_fixlen_pad:
	$(PRINTF) "%s" "[tcprewrite] Fix length and pad test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] Fix length and pad test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 --fixlen=pad >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

rewrite_fixlen_trunc:
	$(PRINTF) "%s" "[tcprewrite] Fix length and truncate test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] Fix length and truncate test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 --fixlen=trunc >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t%s\n" "OK"; fi

rewrite_fixlen_del:
	$(PRINTF) "%s" "[tcprewrite] Fix length and delete test: "
	$(PRINTF) "%s\n" "*** [tcprewrite] Fix length and delete test: " >> test.log
	$(TCPREWRITE) $(ENABLE_DEBUG) -i $(TEST_PCAP) -o test.$@1 --fixlen=del >> test.log 2>&1
if WORDS_BIGENDIAN
	diff $(srcdir)/test.$@ test.$@1 >> test.log 2>&1
else
	diff $(srcdir)/test2.$@ test.$@1 >> test.log 2>&1
endif
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t%s\n" "OK"; fi

replay_pps:
	$(PRINTF) "%s" "[tcpreplay] Packets/sec test: "
	$(PRINTF) "%s\n" "*** [tcpreplay] Packets/sec test: " >> test.log
	$(TCPREPLAY) $(ENABLE_DEBUG) -i $(nic1) -p 25 $(TEST_PCAP) >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

replay_rate:
	$(PRINTF) "%s" "[tcpreplay] Mbps test: "
	$(PRINTF) "%s\n" "*** [tcpreplay] Mbps test: " >> test.log
	$(TCPREPLAY) $(ENABLE_DEBUG) -i $(nic1) -M 25.0 $(TEST_PCAP) >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t\t%s\n" "OK"; fi

replay_multi:
	$(PRINTF) "%s" "[tcpreplay] Multiplier test: "
	$(PRINTF) "%s\n" "*** [tcpreplay] Multiplier test: " >> test.log
	$(TCPREPLAY) $(ENABLE_DEBUG) -i $(nic1) -x 25.0 $(TEST_PCAP) >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

replay_pps_multi:
	$(PRINTF) "%s" "[tcpreplay] Packets/sec Multiplier test: "
	$(PRINTF) "%s\n" "*** [tcpreplay] Packets/sec Multiplier test: " >> test.log
	$(TCPREPLAY) $(ENABLE_DEBUG) -i $(nic1) -p 100 --pps-multi=5 $(TEST_PCAP) >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t%s\n" "FAILED"; else $(PRINTF) "\t%s\n" "OK"; fi

replay_top:
	$(PRINTF) "%s" "[tcpreplay] Topspeed test: "
	$(PRINTF) "%s\n" "*** [tcpreplay] Topspeed test: " >> test.log
	$(TCPREPLAY) $(ENABLE_DEBUG) -i $(nic1) -t $(TEST_PCAP) >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

replay_precache:
	$(PRINTF) "%s" "[tcpreplay] Precache test: "
	$(PRINTF) "%s\n" "*** [tcpreplay] Precache test: " >> test.log
	$(TCPREPLAY) $(ENABLE_DEBUG) -i $(nic1) --preload-pcap $(TEST_PCAP) >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

datadump_mode:
	$(PRINTF) "%s" "[tcpreplay] Data dump test: "
	$(PRINTF) "%s\n" "*** [tcpreplay] Data dump mode test: " >> test.log
	$(TCPREPLAY) $(ENABLE_DEBUG) -D -i $(nic1) -I $(nic2) -w primary.data -W secondary.data -c $(srcdir)/test.cidr -R $(TEST_PCAP) >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

replay_config:
	$(PRINTF) "%s" "[tcpreplay] Config file/VLAN add test: "
	$(PRINTF) "%s\n" "*** [tcpreplay] Config file/VLAN add test: " >> test.log
	$(TCPREPLAY) $(ENABLE_DEBUG) --load-opts=config $(TEST_PCAP) >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

replay_dualfile:
	$(PRINTF) "%s" "[tcpreplay] Dual file test: "
	$(PRINTF) "%s\n" "*** [tcpreplay] Dual file test: " >> test.log
	$(TCPREPLAY) $(ENABLE_DEBUG) -i $(nic1) -I $(nic2) --dualfile $(TEST_PCAP) $(TEST_PCAP) >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t\t%s\n" "OK"; fi

replay_maxsleep:
	$(PRINTF) "%s" "[tcpreplay] Maximum sleep test: "
	$(PRINTF) "%s\n" "*** [tcpreplay] Maximum sleep test: " >> test.log
	$(TCPREPLAY) $(ENABLE_DEBUG) -i $(nic1) --maxsleep=20 $(TEST_PCAP) $(TEST_PCAP) >> test.log 2>&1
	if [ $? ] ; then $(PRINTF) "\t\t%s\n" "FAILED"; else $(PRINTF) "\t\t%s\n" "OK"; fi

clean:
	rm -f *1 test.log core* *~ primary.data secondary.data

distclean: clean
	rm -f Makefile Makefile.in config

maintainer-clean: distclean
