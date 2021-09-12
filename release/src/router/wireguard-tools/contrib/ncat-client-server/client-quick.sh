#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.

set -e

echo "[!] Warning: This server is for testing purposes only. You may not use this server for abusive or illegal purposes."

echo "[+] Generating private key."
privatekey="$(wg genkey)"

echo "[+] Sending public key to server."
exec 7<>/dev/tcp/demo.wireguard.com/42912
wg pubkey <<<"$privatekey" >&7

echo "[+] Parsing server response."
IFS=: read -r status server_pubkey server_port internal_ip <&7
[[ $status == OK ]] || exit 1

echo "[+] Writing config file."
[[ $UID -eq 0 ]] || sudo=sudo
$sudo sh -c 'umask 077; mkdir -p /etc/wireguard; cat > /etc/wireguard/demo.conf' <<_EOF
[Interface]
PrivateKey = $privatekey
Address = $internal_ip/24
DNS = 8.8.8.8, 8.8.4.4, 1.1.1.1, 1.0.0.1

[Peer]
PublicKey = $server_pubkey
Endpoint = demo.wireguard.com:$server_port
AllowedIPs = 0.0.0.0/0
_EOF

echo "[+] Success. Run \`wg-quick up demo\` to turn on the tunnel to the demo server and \`wg-quick down demo\` to turn it off."
