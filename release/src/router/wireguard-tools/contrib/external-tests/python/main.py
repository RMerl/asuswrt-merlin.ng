#!/usr/bin/python3

# SPDX-License-Identifier: MIT
# Author: Piotr Lizonczyk <plizonczyk.public@gmail.com>

import base64
import datetime
from hashlib import blake2s
import socket
import struct

from scapy.layers.inet import IP, ICMP

from noise.connection import NoiseConnection, Keypair


address = ('demo.wireguard.com', 12913)

our_private = base64.b64decode('WAmgVYXkbT2bCtdcDwolI88/iVi/aV3/PHcUBTQSYmo=')
their_public = base64.b64decode('qRCwZSKInrMAq5sepfCdaCsRJaoLe5jhtzfiw7CjbwM=')
preshared = base64.b64decode('FpCyhws9cxwWoV4xELtfJvjJN+zQVRPISllRWgeopVE=')
prologue = b'WireGuard v1 zx2c4 Jason@zx2c4.com'

noise = NoiseConnection.from_name(b'Noise_IKpsk2_25519_ChaChaPoly_BLAKE2s')
noise.set_as_initiator()
noise.set_keypair_from_private_bytes(Keypair.STATIC, our_private)
noise.set_keypair_from_public_bytes(Keypair.REMOTE_STATIC, their_public)
noise.set_psks(psk=preshared)
noise.set_prologue(prologue)
noise.start_handshake()

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)


# 1. Prepare and send handshake initiation packet
now = datetime.datetime.now()
tai = struct.pack('!qi', 4611686018427387914 + int(now.timestamp()), int(now.microsecond * 1e3))
initiation_packet = b'\x01'  # Type: initiation
initiation_packet += b'\x00' * 3  # Reserved
initiation_packet += struct.pack('<i', 28)  # Sender index: 28 (arbitrary)
initiation_packet += noise.write_message(payload=tai)
mac_key = blake2s(b'mac1----' + their_public).digest()
initiation_packet += blake2s(initiation_packet, digest_size=16, key=mac_key).digest()
initiation_packet += b'\x00' * 16

sock.sendto(initiation_packet, address)


# 2. Receive response to finalize handshake
response_packet = sock.recv(92)
assert response_packet[0] == 2  # Type: response
assert response_packet[1:4] == b'\x00' * 3  # Reserved
their_index, our_index = struct.unpack('<ii', response_packet[4:12])
assert our_index == 28
payload = noise.read_message(response_packet[12:60])
assert payload == b''
assert noise.handshake_finished


# 3. Prepare, encrypt and send ping packet
icmp_packet = ICMP(type=8, id=921, seq=438)/b'WireGuard'
ip_packet = IP(proto=1, ttl=20, src="10.189.129.2", dst="10.189.129.1", id=0)/icmp_packet
ping_packet = b'\x04'  # Type: data
ping_packet += b'\x00' * 3  # Reserved
ping_packet += struct.pack('<iq', their_index, 0)
ping_packet += noise.encrypt(bytes(ip_packet))

sock.sendto(ping_packet, address)


# 4. Retrieve ping response, decrypt and verify
encrypted_response = sock.recv(80)
assert encrypted_response[0] == 4  # Type: data
assert encrypted_response[1:4] == b'\x00' * 3  # Reserved
our_index, nonce = struct.unpack('<iq', encrypted_response[4:16])
assert our_index == 28
assert nonce == 0
ip = IP(noise.decrypt(encrypted_response[16:]))
icmp = ip[1]
payload = ip[2]
assert icmp.type == 0
assert icmp.code == 0
assert icmp.id == 921
assert icmp.seq == 438
assert payload.load == b'WireGuard'


# 5. Send keepalive
keepalive = b'\x04'  # Type: data
keepalive += b'\x00' * 3  # Reserved
keepalive += struct.pack('<iq', their_index, 1)
keepalive += noise.encrypt(b'')

sock.sendto(keepalive, address)
