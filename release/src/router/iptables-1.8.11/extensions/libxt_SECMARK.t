:INPUT,FORWARD,OUTPUT
*security
-j SECMARK --selctx system_u:object_r:firewalld_exec_t:s0;=;OK
-j SECMARK;;FAIL
