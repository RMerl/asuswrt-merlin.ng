#!/bin/sh
# usage: sign_openssl_simple.sh signable_file_in signature_file_out  key_in_pem_format 

openssl dgst -sign $3 -keyform pem -sha256 -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -out $2 $1

