#!/usr/bin/python
# Copyright 2017-2019, The Tor Project, Inc
# See LICENSE for licensing information

"""
hs_ntor_ref.py

This module is a reference implementation of the modified ntor protocol
proposed for Tor hidden services in proposal 224 (Next Generation Hidden
Services) in section [NTOR-WITH-EXTRA-DATA].

The modified ntor protocol is a single-round protocol, with three steps in total:

    1: Client generates keys and sends them to service via INTRODUCE cell

    2: Service computes key material based on client's keys, and sends its own
       keys to client via RENDEZVOUS cell

    3: Client computes key material as well.

It's meant to be used to validate Tor's HS ntor implementation by conducting
various integration tests. Specifically it conducts the following three tests:

- Tests our Python implementation by running the whole protocol in Python and
  making sure that results are consistent.

- Tests little-t-tor ntor implementation. We use this Python code to instrument
  little-t-tor and carry out the handshake by using little-t-tor code. The
  small C wrapper at src/test/test-hs-ntor-cl is used for this Python module to
  interface with little-t-tor.

- Cross-tests Python and little-t-tor implementation by running half of the
  protocol in Python code and the other in little-t-tor. This is actually two
  tests so that all parts of the protocol are run both by little-t-tor and
  Python.

It requires the curve25519 python module from the curve25519-donna package.

The whole logic and concept for this test suite was taken from ntor_ref.py.

                *** DO NOT USE THIS IN PRODUCTION. ***
"""

# Future imports for Python 2.7, mandatory in 3.0
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import struct
import os, sys
import binascii
import subprocess

try:
    import curve25519
    curve25519mod = curve25519.keys
except ImportError:
    curve25519 = None
    import slownacl_curve25519
    curve25519mod = slownacl_curve25519

import hashlib
try:
    import sha3
except ImportError:
    # In python 3.6, the sha3 functions are in hashlib whether we
    # import sha3 or not.
    sha3 = None

try:
    # Pull the sha3 functions in.
    from hashlib import sha3_256, shake_256
    def shake_squeeze(obj, n):
        return obj.digest(n)
except ImportError:
    if hasattr(sha3, "SHA3256"):
        # If this happens, then we have the old "sha3" module which
        # hashlib and pysha3 superseded.
        sha3_256 = sha3.SHA3256
        shake_256 = sha3.SHAKE256
        def shake_squeeze(obj, n):
            return obj.squeeze(n)
    else:
        # error code 77 tells automake to skip this test
        sys.exit(77)

# Import Nick's ntor reference implementation in Python
# We are gonna use a few of its utilities.
from ntor_ref import hash_nil
from ntor_ref import PrivateKey

# String constants used in this protocol
PROTOID = b"tor-hs-ntor-curve25519-sha3-256-1"
T_HSENC    = PROTOID + b":hs_key_extract"
T_HSVERIFY = PROTOID + b":hs_verify"
T_HSMAC    = PROTOID + b":hs_mac"
M_HSEXPAND = PROTOID + b":hs_key_expand"

INTRO_SECRET_LEN = 161
REND_SECRET_LEN = 225
AUTH_INPUT_LEN = 199

# Implements MAC(k,m) = H(htonll(len(k)) | k | m)
def mac(k,m):
    def htonll(num):
        return struct.pack('!q', num)

    s = sha3_256()
    s.update(htonll(len(k)))
    s.update(k)
    s.update(m)
    return s.digest()

######################################################################

# Functions that implement the modified HS ntor protocol

"""As client compute key material for INTRODUCE cell as follows:

  intro_secret_hs_input = EXP(B,x) | AUTH_KEY | X | B | PROTOID
  info = m_hsexpand | subcredential
  hs_keys = KDF(intro_secret_hs_input | t_hsenc | info, S_KEY_LEN+MAC_LEN)
  ENC_KEY = hs_keys[0:S_KEY_LEN]
  MAC_KEY = hs_keys[S_KEY_LEN:S_KEY_LEN+MAC_KEY_LEN]
"""
def intro2_ntor_client(intro_auth_pubkey_str, intro_enc_pubkey,
                       client_ephemeral_enc_pubkey, client_ephemeral_enc_privkey, subcredential):

    dh_result = client_ephemeral_enc_privkey.get_shared_key(intro_enc_pubkey, hash_nil)
    secret =  dh_result + intro_auth_pubkey_str + client_ephemeral_enc_pubkey.serialize() + intro_enc_pubkey.serialize() + PROTOID
    assert(len(secret) == INTRO_SECRET_LEN)
    info = M_HSEXPAND + subcredential

    kdf = shake_256()
    kdf.update(secret + T_HSENC + info)
    key_material = shake_squeeze(kdf, 64*8)

    enc_key = key_material[0:32]
    mac_key = key_material[32:64]

    return enc_key, mac_key

"""Wrapper over intro2_ntor_client()"""
def client_part1(intro_auth_pubkey_str, intro_enc_pubkey,
                 client_ephemeral_enc_pubkey, client_ephemeral_enc_privkey, subcredential):
    enc_key, mac_key = intro2_ntor_client(intro_auth_pubkey_str, intro_enc_pubkey, client_ephemeral_enc_pubkey, client_ephemeral_enc_privkey, subcredential)
    assert(enc_key)
    assert(mac_key)

    return enc_key, mac_key

"""As service compute key material for INTRODUCE cell as follows:

  intro_secret_hs_input = EXP(X,b) | AUTH_KEY | X | B | PROTOID
  info = m_hsexpand | subcredential
  hs_keys = KDF(intro_secret_hs_input | t_hsenc | info, S_KEY_LEN+MAC_LEN)
  HS_DEC_KEY = hs_keys[0:S_KEY_LEN]
  HS_MAC_KEY = hs_keys[S_KEY_LEN:S_KEY_LEN+MAC_KEY_LEN]
"""
def intro2_ntor_service(intro_auth_pubkey_str, client_enc_pubkey, service_enc_privkey, service_enc_pubkey, subcredential):
    dh_result = service_enc_privkey.get_shared_key(client_enc_pubkey, hash_nil)
    secret = dh_result + intro_auth_pubkey_str + client_enc_pubkey.serialize() + service_enc_pubkey.serialize() + PROTOID
    assert(len(secret) == INTRO_SECRET_LEN)
    info = M_HSEXPAND + subcredential

    kdf = shake_256()
    kdf.update(secret + T_HSENC + info)
    key_material = shake_squeeze(kdf, 64*8)

    enc_key = key_material[0:32]
    mac_key = key_material[32:64]

    return enc_key, mac_key

"""As service compute key material for INTRODUCE and REDNEZVOUS cells.

  Use intro2_ntor_service() to calculate the INTRODUCE key material, and use
  the following computations to do the RENDEZVOUS ones:

      rend_secret_hs_input = EXP(X,y) | EXP(X,b) | AUTH_KEY | B | X | Y | PROTOID
      NTOR_KEY_SEED = MAC(rend_secret_hs_input, t_hsenc)
      verify = MAC(rend_secret_hs_input, t_hsverify)
      auth_input = verify | AUTH_KEY | B | Y | X | PROTOID | "Server"
      AUTH_INPUT_MAC = MAC(auth_input, t_hsmac)
"""
def service_part1(intro_auth_pubkey_str, client_enc_pubkey, intro_enc_privkey, intro_enc_pubkey, subcredential):
    intro_enc_key, intro_mac_key = intro2_ntor_service(intro_auth_pubkey_str, client_enc_pubkey, intro_enc_privkey, intro_enc_pubkey, subcredential)
    assert(intro_enc_key)
    assert(intro_mac_key)

    service_ephemeral_privkey = PrivateKey()
    service_ephemeral_pubkey = service_ephemeral_privkey.get_public()

    dh_result1 = service_ephemeral_privkey.get_shared_key(client_enc_pubkey, hash_nil)
    dh_result2 = intro_enc_privkey.get_shared_key(client_enc_pubkey, hash_nil)
    rend_secret_hs_input = dh_result1 + dh_result2 + intro_auth_pubkey_str + intro_enc_pubkey.serialize() + client_enc_pubkey.serialize() + service_ephemeral_pubkey.serialize() + PROTOID
    assert(len(rend_secret_hs_input) == REND_SECRET_LEN)

    ntor_key_seed = mac(rend_secret_hs_input, T_HSENC)
    verify = mac(rend_secret_hs_input, T_HSVERIFY)
    auth_input = verify + intro_auth_pubkey_str + intro_enc_pubkey.serialize() + service_ephemeral_pubkey.serialize() + client_enc_pubkey.serialize() + PROTOID + b"Server"
    assert(len(auth_input) == AUTH_INPUT_LEN)
    auth_input_mac = mac(auth_input, T_HSMAC)

    assert(ntor_key_seed)
    assert(auth_input_mac)
    assert(service_ephemeral_pubkey)

    return intro_enc_key, intro_mac_key, ntor_key_seed, auth_input_mac, service_ephemeral_pubkey

"""As client compute key material for rendezvous cells as follows:

  rend_secret_hs_input = EXP(Y,x) | EXP(B,x) | AUTH_KEY | B | X | Y | PROTOID
  NTOR_KEY_SEED = MAC(ntor_secret_input, t_hsenc)
  verify = MAC(ntor_secret_input, t_hsverify)
  auth_input = verify | AUTH_KEY | B | Y | X | PROTOID | "Server"
  AUTH_INPUT_MAC = MAC(auth_input, t_hsmac)
"""
def client_part2(intro_auth_pubkey_str, client_ephemeral_enc_pubkey, client_ephemeral_enc_privkey,
                 intro_enc_pubkey, service_ephemeral_rend_pubkey):
    dh_result1 = client_ephemeral_enc_privkey.get_shared_key(service_ephemeral_rend_pubkey, hash_nil)
    dh_result2 = client_ephemeral_enc_privkey.get_shared_key(intro_enc_pubkey, hash_nil)
    rend_secret_hs_input = dh_result1 + dh_result2 + intro_auth_pubkey_str + intro_enc_pubkey.serialize() + client_ephemeral_enc_pubkey.serialize() + service_ephemeral_rend_pubkey.serialize() + PROTOID
    assert(len(rend_secret_hs_input) == REND_SECRET_LEN)

    ntor_key_seed = mac(rend_secret_hs_input, T_HSENC)
    verify = mac(rend_secret_hs_input, T_HSVERIFY)
    auth_input = verify + intro_auth_pubkey_str + intro_enc_pubkey.serialize() + service_ephemeral_rend_pubkey.serialize() + client_ephemeral_enc_pubkey.serialize() + PROTOID + b"Server"
    assert(len(auth_input) == AUTH_INPUT_LEN)
    auth_input_mac = mac(auth_input, T_HSMAC)

    assert(ntor_key_seed)
    assert(auth_input_mac)

    return ntor_key_seed, auth_input_mac

#################################################################################

"""
Utilities for communicating with the little-t-tor ntor wrapper to conduct the
integration tests
"""

PROG = "./src/test/test-hs-ntor-cl"
if sys.version_info[0] >= 3:
    enhex=lambda s: binascii.b2a_hex(s).decode("ascii")
else:
    enhex=lambda s: binascii.b2a_hex(s)
dehex=lambda s: binascii.a2b_hex(s.strip())

def tor_client1(intro_auth_pubkey_str, intro_enc_pubkey,
                client_ephemeral_enc_privkey, subcredential):
    p = subprocess.Popen([PROG, "client1",
                          enhex(intro_auth_pubkey_str),
                          enhex(intro_enc_pubkey.serialize()),
                          enhex(client_ephemeral_enc_privkey.serialize()),
                          enhex(subcredential)],
                         stdout=subprocess.PIPE)
    return map(dehex, p.stdout.readlines())

def tor_server1(intro_auth_pubkey_str, intro_enc_privkey,
                client_ephemeral_enc_pubkey, subcredential):
    p = subprocess.Popen([PROG, "server1",
                          enhex(intro_auth_pubkey_str),
                          enhex(intro_enc_privkey.serialize()),
                          enhex(client_ephemeral_enc_pubkey.serialize()),
                          enhex(subcredential)],
                         stdout=subprocess.PIPE)
    return map(dehex, p.stdout.readlines())

def tor_client2(intro_auth_pubkey_str, client_ephemeral_enc_privkey,
                intro_enc_pubkey, service_ephemeral_rend_pubkey, subcredential):
    p = subprocess.Popen([PROG, "client2",
                          enhex(intro_auth_pubkey_str),
                          enhex(client_ephemeral_enc_privkey.serialize()),
                          enhex(intro_enc_pubkey.serialize()),
                          enhex(service_ephemeral_rend_pubkey.serialize()),
                          enhex(subcredential)],
                         stdout=subprocess.PIPE)
    return map(dehex, p.stdout.readlines())

##################################################################################

# Perform a pure python ntor test
def do_pure_python_ntor_test():
    # Initialize all needed key material
    client_ephemeral_enc_privkey = PrivateKey()
    client_ephemeral_enc_pubkey = client_ephemeral_enc_privkey.get_public()
    intro_enc_privkey = PrivateKey()
    intro_enc_pubkey = intro_enc_privkey.get_public()
    intro_auth_pubkey_str = os.urandom(32)
    subcredential = os.urandom(32)

    client_enc_key, client_mac_key = client_part1(intro_auth_pubkey_str, intro_enc_pubkey, client_ephemeral_enc_pubkey, client_ephemeral_enc_privkey, subcredential)

    service_enc_key, service_mac_key, service_ntor_key_seed, service_auth_input_mac, service_ephemeral_pubkey = service_part1(intro_auth_pubkey_str, client_ephemeral_enc_pubkey, intro_enc_privkey, intro_enc_pubkey, subcredential)

    assert(client_enc_key == service_enc_key)
    assert(client_mac_key == service_mac_key)

    client_ntor_key_seed, client_auth_input_mac = client_part2(intro_auth_pubkey_str, client_ephemeral_enc_pubkey, client_ephemeral_enc_privkey,
                                                               intro_enc_pubkey, service_ephemeral_pubkey)

    assert(client_ntor_key_seed == service_ntor_key_seed)
    assert(client_auth_input_mac == service_auth_input_mac)

    print("DONE: python dance [%s]" % repr(client_auth_input_mac))

# Perform a pure little-t-tor integration test.
def do_little_t_tor_ntor_test():
    # Initialize all needed key material
    subcredential = os.urandom(32)
    client_ephemeral_enc_privkey = PrivateKey()
    client_ephemeral_enc_pubkey = client_ephemeral_enc_privkey.get_public()
    intro_enc_privkey = PrivateKey()
    intro_enc_pubkey = intro_enc_privkey.get_public() # service-side enc key
    intro_auth_pubkey_str = os.urandom(32)

    client_enc_key, client_mac_key = tor_client1(intro_auth_pubkey_str, intro_enc_pubkey,
                                                 client_ephemeral_enc_privkey, subcredential)
    assert(client_enc_key)
    assert(client_mac_key)

    service_enc_key, service_mac_key, service_ntor_auth_mac, service_ntor_key_seed, service_eph_pubkey = tor_server1(intro_auth_pubkey_str,
                                                                                                                     intro_enc_privkey,
                                                                                                                     client_ephemeral_enc_pubkey,
                                                                                                                     subcredential)
    assert(service_enc_key)
    assert(service_mac_key)
    assert(service_ntor_auth_mac)
    assert(service_ntor_key_seed)

    assert(client_enc_key == service_enc_key)
    assert(client_mac_key == service_mac_key)

    # Turn from bytes to key
    service_eph_pubkey = curve25519mod.Public(service_eph_pubkey)

    client_ntor_auth_mac, client_ntor_key_seed  = tor_client2(intro_auth_pubkey_str, client_ephemeral_enc_privkey,
                                                              intro_enc_pubkey, service_eph_pubkey, subcredential)
    assert(client_ntor_auth_mac)
    assert(client_ntor_key_seed)

    assert(client_ntor_key_seed == service_ntor_key_seed)
    assert(client_ntor_auth_mac == service_ntor_auth_mac)

    print("DONE: tor dance [%s]" % repr(client_ntor_auth_mac))

"""
Do mixed test as follows:
    1. C -> S (python mode)
    2. C <- S (tor mode)
    3. Client computes keys (python mode)
"""
def do_first_mixed_test():
    subcredential = os.urandom(32)

    client_ephemeral_enc_privkey = PrivateKey()
    client_ephemeral_enc_pubkey = client_ephemeral_enc_privkey.get_public()
    intro_enc_privkey = PrivateKey()
    intro_enc_pubkey = intro_enc_privkey.get_public() # service-side enc key

    intro_auth_pubkey_str = os.urandom(32)

    # Let's do mixed
    client_enc_key, client_mac_key = client_part1(intro_auth_pubkey_str, intro_enc_pubkey,
                                                  client_ephemeral_enc_pubkey, client_ephemeral_enc_privkey,
                                                  subcredential)

    service_enc_key, service_mac_key, service_ntor_auth_mac, service_ntor_key_seed, service_eph_pubkey = tor_server1(intro_auth_pubkey_str,
                                                                                                                     intro_enc_privkey,
                                                                                                                     client_ephemeral_enc_pubkey,
                                                                                                                     subcredential)
    assert(service_enc_key)
    assert(service_mac_key)
    assert(service_ntor_auth_mac)
    assert(service_ntor_key_seed)
    assert(service_eph_pubkey)

    assert(client_enc_key == service_enc_key)
    assert(client_mac_key == service_mac_key)

    # Turn from bytes to key
    service_eph_pubkey = curve25519mod.Public(service_eph_pubkey)

    client_ntor_key_seed, client_auth_input_mac = client_part2(intro_auth_pubkey_str, client_ephemeral_enc_pubkey, client_ephemeral_enc_privkey,
                                                               intro_enc_pubkey, service_eph_pubkey)

    assert(client_auth_input_mac == service_ntor_auth_mac)
    assert(client_ntor_key_seed == service_ntor_key_seed)

    print("DONE: 1st mixed dance [%s]" % repr(client_auth_input_mac))

"""
Do mixed test as follows:
    1. C -> S (tor mode)
    2. C <- S (python mode)
    3. Client computes keys (tor mode)
"""
def do_second_mixed_test():
    subcredential = os.urandom(32)

    client_ephemeral_enc_privkey = PrivateKey()
    client_ephemeral_enc_pubkey = client_ephemeral_enc_privkey.get_public()
    intro_enc_privkey = PrivateKey()
    intro_enc_pubkey = intro_enc_privkey.get_public() # service-side enc key

    intro_auth_pubkey_str = os.urandom(32)

    # Let's do mixed
    client_enc_key, client_mac_key = tor_client1(intro_auth_pubkey_str, intro_enc_pubkey,
                                                 client_ephemeral_enc_privkey, subcredential)
    assert(client_enc_key)
    assert(client_mac_key)

    service_enc_key, service_mac_key, service_ntor_key_seed, service_ntor_auth_mac, service_ephemeral_pubkey = service_part1(intro_auth_pubkey_str, client_ephemeral_enc_pubkey, intro_enc_privkey, intro_enc_pubkey, subcredential)

    client_ntor_auth_mac, client_ntor_key_seed  = tor_client2(intro_auth_pubkey_str, client_ephemeral_enc_privkey,
                                                              intro_enc_pubkey, service_ephemeral_pubkey, subcredential)
    assert(client_ntor_auth_mac)
    assert(client_ntor_key_seed)

    assert(client_ntor_key_seed == service_ntor_key_seed)
    assert(client_ntor_auth_mac == service_ntor_auth_mac)

    print("DONE: 2nd mixed dance [%s]" % repr(client_ntor_auth_mac))

def do_mixed_tests():
    do_first_mixed_test()
    do_second_mixed_test()

if __name__ == '__main__':
    do_pure_python_ntor_test()
    do_little_t_tor_ntor_test()
    do_mixed_tests()
