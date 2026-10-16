:INPUT,FORWARD,OUTPUT
-m tcpmss --mss 42;;FAIL
-p tcp -m tcpmss --mss 42;=;OK
-p tcp -m tcpmss --mss :;-p tcp -m tcpmss --mss 0:65535;OK
-p tcp -m tcpmss --mss :42;-p tcp -m tcpmss --mss 0:42;OK
-p tcp -m tcpmss --mss 42:;-p tcp -m tcpmss --mss 42:65535;OK
-p tcp -m tcpmss --mss 42:42;-p tcp -m tcpmss --mss 42;OK
-p tcp -m tcpmss --mss 42:12345;=;OK
-p tcp -m tcpmss --mss 42:65536;;FAIL
-p tcp -m tcpmss --mss 65535:1000;;FAIL
