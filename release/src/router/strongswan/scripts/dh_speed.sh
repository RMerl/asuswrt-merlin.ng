#!/bin/bash

DIR=$(dirname `readlink -f $0`)

# we run an unprinted group, as it seems the first run is inaccurate (cache?)

function modptest {
  $DIR/dh_speed "$1" 400 modp768 modp768 modp1024 modp1024s160 modp1536 modp2048 modp2048s224 modp2048s256 | tail -n 7
  $DIR/dh_speed "$1" 100 modp1024 modp3072 modp4096 | tail -n 2
  $DIR/dh_speed "$1" 5 modp2048 modp6144 modp8192 | tail -n 2
}

echo "testing gmp"
# gmp needs an RNG plugin, pick gcrypt
modptest "gmp gcrypt"

echo "testing curve25519"
# curve25519 needs an RNG plugin, pick gcrypt
$DIR/dh_speed "curve25519 gcrypt" 300 curve25519 curve25519 | tail -n 1

echo "testing gcrypt"
modptest "gcrypt"

echo "testing openssl"
modptest "openssl"
$DIR/dh_speed "openssl" 300 ecp192 ecp192 ecp224 ecp256 ecp384 ecp521 | tail -n 5

echo "testing botan"
modptest "botan"
$DIR/dh_speed "botan" 300 ecp256 ecp256 ecp384 ecp521 | tail -n 3
$DIR/dh_speed "botan" 300 curve25519 curve25519 | tail -n 1
