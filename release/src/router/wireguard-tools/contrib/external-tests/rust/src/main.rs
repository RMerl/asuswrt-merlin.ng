/* Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved. */

extern crate snow;
extern crate base64;
extern crate time;
extern crate byteorder;
extern crate crypto;
extern crate pnet;

use byteorder::{ByteOrder, BigEndian, LittleEndian};
use crypto::blake2s::Blake2s;
use snow::NoiseBuilder;
use pnet::packet::Packet;
use pnet::packet::ip::IpNextHeaderProtocols;
use pnet::packet::ipv4::{MutableIpv4Packet, self};
use pnet::packet::icmp::{MutableIcmpPacket, IcmpTypes, echo_reply, echo_request, self};
use std::net::*;
use std::str::FromStr;

static TEST_SERVER: &'static str = "demo.wireguard.com:12913";

fn memcpy(out: &mut [u8], data: &[u8]) {
	out[..data.len()].copy_from_slice(data);
}

fn main() {
	let socket = UdpSocket::bind("0.0.0.0:0").unwrap();

	let their_public = base64::decode(&"qRCwZSKInrMAq5sepfCdaCsRJaoLe5jhtzfiw7CjbwM=").unwrap();
	let my_private = base64::decode(&"WAmgVYXkbT2bCtdcDwolI88/iVi/aV3/PHcUBTQSYmo=").unwrap();
	let my_preshared = base64::decode(&"FpCyhws9cxwWoV4xELtfJvjJN+zQVRPISllRWgeopVE=").unwrap();

	let mut noise = NoiseBuilder::new("Noise_IKpsk2_25519_ChaChaPoly_BLAKE2s".parse().unwrap())
		.local_private_key(&my_private[..])
		.remote_public_key(&their_public[..])
		.prologue("WireGuard v1 zx2c4 Jason@zx2c4.com".as_bytes())
		.psk(2, &my_preshared[..])
		.build_initiator().unwrap();

	let now = time::get_time();
	let mut tai64n = [0; 12];
	BigEndian::write_i64(&mut tai64n[0..], 4611686018427387914 + now.sec);
	BigEndian::write_i32(&mut tai64n[8..], now.nsec);
	let mut initiation_packet = [0; 148];
	initiation_packet[0] = 1; /* Type: Initiation */
	initiation_packet[1] = 0; /* Reserved */
	initiation_packet[2] = 0; /* Reserved */
	initiation_packet[3] = 0; /* Reserved */
	LittleEndian::write_u32(&mut initiation_packet[4..], 28); /* Sender index: 28 (arbitrary) */
	noise.write_message(&tai64n, &mut initiation_packet[8..]).unwrap();
	let mut mac_key_input = [0; 40];
	let mut mac_key = [0; 32];
	memcpy(&mut mac_key_input, b"mac1----");
	memcpy(&mut mac_key_input[8..], &their_public);
	Blake2s::blake2s(&mut mac_key, &mac_key_input, &[0; 0]);
	let mut mac = [0; 16];
	Blake2s::blake2s(&mut mac, &initiation_packet[0..116], &mac_key);
	memcpy(&mut initiation_packet[116..], &mac);
	socket.send_to(&initiation_packet, TEST_SERVER).unwrap();

	let mut response_packet = [0; 92];
	socket.recv_from(&mut response_packet).unwrap();
	assert!(response_packet[0] == 2 /* Type: Response */);
	assert!(response_packet[1] == 0 /* Reserved */);
	assert!(response_packet[2] == 0 /* Reserved */);
	assert!(response_packet[3] == 0 /* Reserved */);
	let their_index = LittleEndian::read_u32(&response_packet[4..]);
	let our_index = LittleEndian::read_u32(&response_packet[8..]);
	assert!(our_index == 28);
	let payload_len = noise.read_message(&response_packet[12..60], &mut []).unwrap();
	assert!(payload_len == 0);
	noise = noise.into_transport_mode().unwrap();

	let mut icmp_packet = [0; 48];
	{
		let mut ipv4 = MutableIpv4Packet::new(&mut icmp_packet).unwrap();
		ipv4.set_version(4);
		ipv4.set_header_length(5);
		ipv4.set_total_length(37);
		ipv4.set_ttl(20);
		ipv4.set_next_level_protocol(IpNextHeaderProtocols::Icmp);
		ipv4.set_source(Ipv4Addr::from_str("10.189.129.2").unwrap());
		ipv4.set_destination(Ipv4Addr::from_str("10.189.129.1").unwrap());
		let checksum = ipv4::checksum(&ipv4.to_immutable());
		ipv4.set_checksum(checksum);
	}
	{
		let mut icmp = echo_request::MutableEchoRequestPacket::new(&mut icmp_packet[20..]).unwrap();
		icmp.set_icmp_type(IcmpTypes::EchoRequest);
		icmp.set_icmp_code(echo_request::IcmpCodes::NoCode);
		icmp.set_identifier(921);
		icmp.set_sequence_number(438);
		icmp.set_payload(b"WireGuard");
	}
	{
		let mut icmp = MutableIcmpPacket::new(&mut icmp_packet[20..]).unwrap();
		let checksum = icmp::checksum(&icmp.to_immutable());
		icmp.set_checksum(checksum);
	}

	let mut ping_packet = [0; 80];
	ping_packet[0] = 4; /* Type: Data */
	ping_packet[1] = 0; /* Reserved */
	ping_packet[2] = 0; /* Reserved */
	ping_packet[3] = 0; /* Reserved */
	LittleEndian::write_u32(&mut ping_packet[4..], their_index);
	LittleEndian::write_u64(&mut ping_packet[8..], 0);
	noise.write_message(&icmp_packet, &mut ping_packet[16..]).unwrap();
	socket.send_to(&ping_packet, TEST_SERVER).unwrap();

	socket.recv_from(&mut ping_packet).unwrap();
	assert!(ping_packet[0] == 4 /* Type: Data */);
	assert!(ping_packet[1] == 0 /* Reserved */);
	assert!(ping_packet[2] == 0 /* Reserved */);
	assert!(ping_packet[3] == 0 /* Reserved */);
	let our_index_received = LittleEndian::read_u32(&ping_packet[4..]);
	assert!(our_index_received == 28);
	let nonce = LittleEndian::read_u64(&ping_packet[8..]);
	assert!(nonce == 0);
	let payload_len = noise.read_message(&ping_packet[16..], &mut icmp_packet).unwrap();
	assert!(payload_len == 48);
	let icmp_reply = echo_reply::EchoReplyPacket::new(&icmp_packet[20..37]).unwrap();
	assert!(icmp_reply.get_icmp_type() == IcmpTypes::EchoReply && icmp_reply.get_icmp_code() == echo_reply::IcmpCodes::NoCode);
	assert!(icmp_reply.get_identifier() == 921 && icmp_reply.get_sequence_number() == 438);
	assert!(icmp_reply.payload() == b"WireGuard");

	let mut keepalive_packet = [0; 32];
	keepalive_packet[0] = 4; /* Type: Data */
	keepalive_packet[1] = 0; /* Reserved */
	keepalive_packet[2] = 0; /* Reserved */
	keepalive_packet[3] = 0; /* Reserved */
	LittleEndian::write_u32(&mut keepalive_packet[4..], their_index);
	LittleEndian::write_u64(&mut keepalive_packet[8..], 1);
	let empty_payload = [0; 0]; /* Empty payload means keepalive */
	noise.write_message(&empty_payload, &mut keepalive_packet[16..]).unwrap();
	socket.send_to(&keepalive_packet, TEST_SERVER).unwrap();
}
