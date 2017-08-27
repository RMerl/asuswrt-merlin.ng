s/\(Sending Access-Request of id\).*\(to 127.0.0.1:1812\)/\1 999 \2/
s/\(Message-Authenticator = 0x\).*/\1ABCDABCDABCDABCDABCDABCDABCDABCD/
s/\(State = 0x\).*/\1ABCDABCDABCDABCDABCDABCDABCDABCD/
s/\(rad_recv: Access-Challenge packet from host 127.0.0.1:1812, id\)=.*,\( length=.*\)/\1=999,\2/
s/\(rad_recv: Access-Accept packet from host 127.0.0.1:1812, id\)=.*,\( length=.*\)/\1=999,\2/
s/\(EAP-Message = 0x..\)\(.*\)/\1XX/
s/\(EAP-Id = \).*/\1YY/
s/\(EAP-Type-MD5 = \).*/\1MD5/


