s/\(Sending Access-Request of id\).*\(to 127.0.0.1:1812\)/\1 999 \2/
s/\(Message-Authenticator = 0x\).*/\1ABCDABCDABCDABCDABCDABCDABCDABCD/
s/\(State = 0x\).*/\1ABCDABCDABCDABCDABCDABCDABCDABCD/
s/\(rad_recv: Access-Challenge packet from host 127.0.0.1:1812, id\)=.*,\( length=.*\)/\1=999,\2/
s/\(rad_recv: Access-Accept packet from host 127.0.0.1:1812, id\)=.*,\( length=.*\)/\1=999,\2/
s/\(EAP-Message = 0x..\)\(.*\)/\1XX/
s/\(EAP-Id = \).*/\1YY/
s/\(EAP-Type-MD5 = \).*/\1MD5/
s/\(EAP-Type-SIM = 0x0b0000010d0000abcd1234abcd1234abcd1234abcd1234bcd1234abcd1234abcd1234abcd1234acd1234abcd1234abcd1234abcd1234ab0b050000\)................................/\1XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX/
s/\(EAP-Type-SIM = 0x0b0000010d0000101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f0b050000\)................................/\1XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX/
s/\(EAP-Type-SIM = 0x0b0000010d00003000000000000000000000000000000031000000000000000000000000000000320000000000000000000000000000000b050000\)................................/\1XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX/

s/\(EAP-Sim-MAC = 0x\)..................................../\1YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY/
s/DATA: (96)    01../DATA: (96)    01YY/
s/DATA: (40)    02../DATA: (40)    02YY/
s/hmac-sha1 mac(20): ........_........_........_........_......../hmac-sha1 mac(20): XXXXXXXX_XXXXXXXX_XXXXXXXX_XXXXXXXX_XXXXXXXX/
 




