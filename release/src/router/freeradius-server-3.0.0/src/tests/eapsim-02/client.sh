#!/bin/sh

( echo 'User-Name = "eapsim"';
  echo 'Cleartext-Password = "md5md5"';
  echo 'NAS-IP-Address = marajade.sandelman.ottawa.on.ca';
  echo 'EAP-Code = Response';
  echo 'EAP-Id = 210';
  echo 'EAP-Type-Identify = "eapsim';
  echo 'Message-Authenticator = 0';
  echo 'NAS-Port = 0' ) >req.txt
  
../../main/radeapclient -x localhost auth testing123 <req.txt


