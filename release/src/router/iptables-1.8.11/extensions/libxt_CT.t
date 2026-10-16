:PREROUTING,OUTPUT
*raw
-j CT --notrack;=;OK
-j CT --ctevents new,related,destroy,reply,assured,protoinfo,helper,mark;=;OK
-j CT --expevents new;=;OK
# ERROR: cannot find: iptables -I PREROUTING -t raw -j CT --zone 0
# -j CT --zone 0;=;OK
-j CT --zone 65535;=;OK
-j CT --zone 65536;;FAIL
-j CT --zone -1;;FAIL
# ERROR: should fail: iptables -A PREROUTING -t raw -j CT
# -j CT;;FAIL
@nfct timeout add test inet tcp ESTABLISHED 100
# cannot load: iptables -A PREROUTING -t raw -j CT --timeout test
# -j CT --timeout test;=;OK
@nfct timeout del test
@nfct helper add rpc inet tcp
# cannot load: iptables -A PREROUTING -t raw -j CT --helper rpc
# -j CT --helper rpc;=;OK
@nfct helper del rpc
