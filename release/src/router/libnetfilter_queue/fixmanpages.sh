#!/bin/bash -p
#set -x
function main
{
  set -e
  cd doxygen/man/man3
  rm -f _*
  setgroup LibrarySetup nfq_open
    add2group nfq_close nfq_bind_pf nfq_unbind_pf
  setgroup Parsing nfq_get_msg_packet_hdr
    add2group nfq_get_nfmark nfq_get_timestamp nfq_get_indev nfq_get_physindev
    add2group nfq_get_outdev nfq_get_physoutdev nfq_get_indev_name
    add2group nfq_get_physindev_name nfq_get_outdev_name
    add2group nfq_get_physoutdev_name nfq_get_packet_hw
    add2group nfq_get_skbinfo
    add2group nfq_get_uid nfq_get_gid
    add2group nfq_get_secctx nfq_get_payload
  setgroup Queue nfq_fd
    add2group nfq_create_queue nfq_destroy_queue nfq_handle_packet nfq_set_mode
    add2group nfq_set_queue_flags nfq_set_queue_maxlen nfq_set_verdict
    add2group nfq_set_verdict2 nfq_set_verdict_batch
    add2group nfq_set_verdict_batch2 nfq_set_verdict_mark
  setgroup ipv4 nfq_ip_get_hdr
    add2group nfq_ip_set_transport_header nfq_ip_mangle nfq_ip_snprintf
    setgroup ip_internals nfq_ip_set_checksum
  setgroup ipv6 nfq_ip6_get_hdr
    add2group nfq_ip6_set_transport_header nfq_ip6_mangle nfq_ip6_snprintf
  setgroup nfq_cfg nfq_nlmsg_cfg_put_cmd
    add2group nfq_nlmsg_cfg_put_params nfq_nlmsg_cfg_put_qmaxlen
  setgroup nfq_verd nfq_nlmsg_verdict_put
    add2group nfq_nlmsg_verdict_put_mark nfq_nlmsg_verdict_put_pkt
  setgroup nlmsg nfq_nlmsg_parse
    add2group nfq_nlmsg_put
  setgroup pktbuff pktb_alloc
    add2group pktb_data pktb_len pktb_mangle pktb_mangled
    add2group pktb_free
    setgroup otherfns pktb_tailroom
      add2group pktb_mac_header pktb_network_header pktb_transport_header
      setgroup uselessfns pktb_push
        add2group pktb_pull pktb_put pktb_trim
  setgroup tcp nfq_tcp_get_hdr
    add2group nfq_tcp_get_payload nfq_tcp_get_payload_len
    add2group nfq_tcp_snprintf nfq_tcp_mangle_ipv4 nfq_tcp_mangle_ipv6
    setgroup tcp_internals nfq_tcp_compute_checksum_ipv4
      add2group nfq_tcp_compute_checksum_ipv6
  setgroup udp nfq_udp_get_hdr
    add2group nfq_udp_get_payload nfq_udp_get_payload_len
    add2group nfq_udp_mangle_ipv4 nfq_udp_mangle_ipv6 nfq_udp_snprintf
    setgroup udp_internals nfq_udp_compute_checksum_ipv4
      add2group nfq_udp_compute_checksum_ipv6
  setgroup Printing nfq_snprintf_xml
}
function setgroup
{
  mv $1.3 $2.3
  BASE=$2
}
function add2group
{
  for i in $@
  do
    ln -sf $BASE.3 $i.3
  done
}
main
