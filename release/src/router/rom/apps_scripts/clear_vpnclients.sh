#!/bin/sh

echo "!!! Removing OpenVPN clients 3 through 5 from nvram and JFFS"
echo "--- Creating backup in /jffs/backup_openvpn.tgz ..."

nvram show 2>/dev/null | grep -E "vpn_client3|vpn_client4|vpn_client5" > /tmp/backup_ovpnclients.sh
sed -i 's/^/nvram set /' /tmp/backup_ovpnclients.sh

tar -zcf /jffs/backup_openvpn.tgz /tmp/backup_ovpnclients.sh /jffs/openvpn/vpn_*client3* /jffs/openvpn/vpn_*client4* /jffs/openvpn/vpn_*client5* >/dev/null 2>/dev/null


echo "--- Clearing OpenVPN client 3"
NVARS="vpn_client3_addr vpn_client3_adns vpn_client3_bridge vpn_client3_cipher vpn_client3_cn vpn_client3_comp vpn_client3_connretry vpn_client3_crypt vpn_client3_custom3 vpn_client3_desc vpn_client3_digest vpn_client3_enforce vpn_client3_errno vpn_client3_fw vpn_client3_gw vpn_client3_hmac vpn_client3_if vpn_client3_local vpn_client3_nat vpn_client3_ncp_ciphers vpn_client3_nm vpn_client3_password vpn_client3_port vpn_client3_proto vpn_client3_remote vpn_client3_reneg vpn_client3_rg vpn_client3_rgw vpn_client3_rip vpn_client3_state vpn_client3_tlsremote vpn_client3_userauth vpn_client3_username vpn_client3_useronly vpn_client3_verb vpn_client3_custom vpn_client3_custom2 vpn_client3_retry vpn_client3_ncp_enable"

for varname in $NVARS
do
	nvram unset $varname
done

rm -f /jffs/openvpn/vpn_crt_client3* /jffs/openvpn/vpn_client3*

echo "--- Clearing OpenVPN client 4"
NVARS="vpn_client4_addr vpn_client4_adns vpn_client4_bridge vpn_client4_cipher vpn_client4_cn vpn_client4_comp vpn_client4_connretry vpn_client4_crypt vpn_client4_custom3 vpn_client4_desc vpn_client4_digest vpn_client4_enforce vpn_client4_errno vpn_client4_fw vpn_client4_gw vpn_client4_hmac vpn_client4_if vpn_client4_local vpn_client4_nat vpn_client4_ncp_ciphers vpn_client4_nm vpn_client4_password vpn_client4_port vpn_client4_proto vpn_client4_remote vpn_client4_reneg vpn_client4_rg vpn_client4_rgw vpn_client4_rip vpn_client4_state vpn_client4_tlsremote vpn_client4_userauth vpn_client4_username vpn_client4_useronly vpn_client4_verb vpn_client4_custom vpn_client4_custom2 vpn_client4_retry vpn_client4_ncp_enable"

for varname in $NVARS
do
	nvram unset $varname
done

rm -f /jffs/openvpn/vpn_crt_client4* /jffs/openvpn/vpn_client4*

echo "--- Clearing OpenVPN client 5"
NVARS="vpn_client5_addr vpn_client5_adns vpn_client5_bridge vpn_client5_cipher vpn_client5_cn vpn_client5_comp vpn_client5_connretry vpn_client5_crypt vpn_client5_custom3 vpn_client5_desc vpn_client5_digest vpn_client5_enforce vpn_client5_errno vpn_client5_fw vpn_client5_gw vpn_client5_hmac vpn_client5_if vpn_client5_local vpn_client5_nat vpn_client5_ncp_ciphers vpn_client5_nm vpn_client5_password vpn_client5_port vpn_client5_proto vpn_client5_remote vpn_client5_reneg vpn_client5_rg vpn_client5_rgw vpn_client5_rip vpn_client5_state vpn_client5_tlsremote vpn_client5_userauth vpn_client5_username vpn_client5_useronly vpn_client5_verb vpn_client5_custom vpn_client5_custom2 vpn_client5_retry vpn_client5_ncp_enable"

for varname in $NVARS
do
	nvram unset $varname
done

echo "!!! Done!"

rm -f /jffs/openvpn/vpn_crt_client5* /jffs/openvpn/vpn_client5*

nvram commit

