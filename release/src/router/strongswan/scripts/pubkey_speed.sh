#!/bin/bash

DIR=$(dirname `readlink -f $0`)

function rsatest {
  echo -n " e=3    "; openssl genrsa -3 $1 2>/dev/null| $DIR/pubkey_speed "$2" rsa $3
  echo -n " e=f4   "; openssl genrsa -f4 $1 2>/dev/null| $DIR/pubkey_speed "$2" rsa $3
}

function rsatestall {
  echo "testing: $1"
  rsatest 512 "$1" 5000
  rsatest 768 "$1" 5000
  rsatest 1024 "$1" 1000
  rsatest 1536 "$1" 500
  rsatest 2048 "$1" 100
  rsatest 3072 "$1" 10
  rsatest 4096 "$1" 5
  rsatest 6144 "$1" 2
  rsatest 8192 "$1" 1
}

function ecdsatest {
  openssl ecparam -genkey -name $1 -noout | $DIR/pubkey_speed "$2" ecdsa $3

}

function ecdsatestall {
  echo "testing: $1"
  ecdsatest prime256v1 "$1" 4000
  ecdsatest secp384r1 "$1" 1000
  ecdsatest secp521r1 "$1" 500
}

rsatestall "gmp gcrypt pem pkcs1"
rsatestall "gcrypt pem pkcs1"
rsatestall "openssl pem"
rsatestall "botan pem"
ecdsatestall "openssl pem"
ecdsatestall "botan pem"
