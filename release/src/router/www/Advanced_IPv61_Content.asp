<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - IPv6</title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<script type="text/javascript" src="state.js"></script>
<script type="text/javascript" src="general.js"></script>
<script type="text/javascript" src="popup.js"></script>
<script type="text/javascript" src="help.js"></script>
<script type="text/javascript" src="validator.js"></script>
<script language="JavaScript" type="text/JavaScript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script>

<% wan_get_parameter(); %>
var wan_proto_orig = '<% nvram_get("wan1_proto"); %>';
var ipv61_proto_orig = '<% nvram_get("ipv61_service"); %>';
var ipv61_tun6rd_dhcp = '<% nvram_get("ipv61_6rd_dhcp"); %>';
var wan1_ipaddr = '<% nvram_get("wan1_ipaddr"); %>';
var machine_name = '<% get_machine_name(); %>';
var machine_arm = (machine_name.search("arm") == -1) ? false : true;
var ipv61_dhcp_start_orig = '<% nvram_get("ipv61_dhcp_start"); %>';
var ipv61_dhcp_end_orig = '<% nvram_get("ipv61_dhcp_end"); %>';

if(yadns_support){
	var yadns_enable = '<% nvram_get("yadns_enable_x"); %>';
	var yadns_mode = '<% nvram_get("yadns_mode"); %>';
}

var ipv6_unit = '<% nvram_get("ipv6_unit"); %>';
var ipv6_service_opt = new Array(	new Array("<#btn_disable#>", "disabled"),
									new Array("Native", "dhcp6"),
									new Array("<#IPv6_static_IP#>", "other"),
									new Array("Tunnel 6to4", "6to4"),
									new Array("Tunnel 6in4", "6in4"),
									new Array("Tunnel 6rd", "6rd")
								);
/*
new Array("SLAAC", "slaac"),
new Array("ICMPv6", "icmp6")
*/

function initial(){	
	show_menu();	
	// https://www.asus.com/US/support/FAQ/113990
	httpApi.faqURL("113990", function(url){document.getElementById("faq").href=url;});
	if(!IPv6_Passthrough_support){
		$("#ipv61_service option[value='ipv6pt']").remove();
		$("#ipv61_service option[value='flets']").remove();
	}

	if(ipv61_proto_orig == "static6"){ // legacy
		ipv61_proto_orig = "other";
		document.form.ipv61_service.value = ipv61_proto_orig;
	}
	
	showInputfield(ipv61_proto_orig);

	if(yadns_support){
		if(yadns_enable != 0 && yadns_mode != -1){
			document.getElementById("yadns_hint").style.display = "";
			document.getElementById("yadns_hint").innerHTML = "<span><#YandexDNS_settings_hint#></span>";
		}
	}

	document.form.wan_selection.selectedIndex = parseInt(ipv6_unit);
	genWANSoption();
}

function showInputfield(v){
	if(v == "dhcp6"){
		if(wan_proto_orig == "l2tp" || wan_proto_orig == "pptp" || wan_proto_orig == "pppoe"){
			inputCtrl(document.form.ipv61_ifdev_select, 1);
			showInputfield2('ipv61_ifdev', document.form.ipv61_ifdev_select.value);
		}else{
			inputCtrl(document.form.ipv61_ifdev_select, 0);
			showInputfield2('ipv61_ifdev', 0);
		}
		inputCtrl(document.form.ipv61_dhcp_pd[0], 1);
		inputCtrl(document.form.ipv61_dhcp_pd[1], 1);
		inputCtrl(document.form.ipv61_tun_v4end, 0);
		inputCtrl(document.form.ipv61_relay, 0);
		inputCtrl(document.form.ipv61_6rd_dhcp[0], 0);
		inputCtrl(document.form.ipv61_6rd_dhcp[1], 0);
		inputCtrl(document.form.ipv61_6rd_prefix, 0);
		inputCtrl(document.form.ipv61_6rd_prefixlen, 0);
		inputCtrl(document.form.ipv61_6rd_router, 0);
		inputCtrl(document.form.ipv61_6rd_ip4size, 0);
		inputCtrl(document.form.ipv61_tun_addr, 0);
		inputCtrl(document.form.ipv61_tun_addrlen, 0);
		inputCtrl(document.form.ipv61_tun_peer, 0);
		inputCtrl(document.form.ipv61_tun_mtu, 0);
		inputCtrl(document.form.ipv61_tun_ttl, 0);
		document.getElementById("ipv61_wan_setting").style.display="none";
		inputCtrl(document.form.ipv61_ipaddr, 0);
		inputCtrl(document.form.ipv61_prefix_len_wan, 0);
		inputCtrl(document.form.ipv61_gateway, 0);		
		document.getElementById("ipv61_lan_setting").style.display="";
		inputCtrl(document.form.ipv61_prefix, 0);		
		inputCtrl(document.form.ipv61_prefix_length, 0);
		document.getElementById("ipv61_prefix_r").style.display = "";
		document.getElementById("ipv61_prefix_length_r").style.display="";
		
		inputCtrl(document.form.ipv61_autoconf_type[0], 1);
		inputCtrl(document.form.ipv61_autoconf_type[1], 1);
		
		if(v != ipv61_proto_orig){

			document.form.ipv61_autoconf_type[0].checked = true;
			showInputfield2('ipv61_autoconf_type', 0);
			document.form.ipv61_prefix.value = "";
			document.form.ipv61_prefix_length.value = "";
			document.form.ipv61_rtr_addr.value = "";
			document.getElementById("ipv61_prefix_span").innerHTML = "";
			document.getElementById("ipv61_prefix_length_span").innerHTML = "";
			document.getElementById("ipv61_ipaddr_span").innerHTML = "";
		}
		else{

			if('<% nvram_get("ipv61_autoconf_type"); %>' == 0)
				document.form.ipv61_autoconf_type[0].checked = true;
			else
				document.form.ipv61_autoconf_type[1].checked = true;
			showInputfield2('ipv61_autoconf_type', '<% nvram_get("ipv61_autoconf_type"); %>');
			document.form.ipv61_prefix.value = '<% nvram_get("ipv61_prefix"); %>';
			document.form.ipv61_prefix_length.value = '<% nvram_get("ipv61_prefix_length"); %>';
			document.form.ipv61_rtr_addr.value = '<% nvram_get("ipv61_rtr_addr"); %>';
			document.getElementById("ipv61_prefix_span").innerHTML = '<% nvram_get("ipv61_prefix"); %>';
			document.getElementById("ipv61_prefix_length_span").innerHTML = '<% nvram_get("ipv61_prefix_length"); %>';
			document.getElementById("ipv61_ipaddr_span").innerHTML = '<% nvram_get("ipv61_rtr_addr"); %>';
		}

		var enable_pd = (document.form.ipv61_dhcp_pd[1].checked) ? '0' : '1';
		showInputfield2('ipv61_dhcp_pd', enable_pd);

		document.getElementById("ipv61_dns_setting").style.display="";
		inputCtrl(document.form.ipv61_dnsenable[0], 1);
		inputCtrl(document.form.ipv61_dnsenable[1], 1);
		var enable_dns = (document.form.ipv61_dnsenable[1].checked) ? '0' : '1';
		showInputfield2('ipv61_dnsenable', enable_dns);
		
		document.getElementById("auto_config").style.display="";

	}
	else if(IPv6_Passthrough_support && (v == "ipv6pt" || v == "flets")){
		if((wan_proto_orig == "l2tp" || wan_proto_orig == "pptp" || wan_proto_orig == "pppoe") && v == "ipv6pt")
			inputCtrl(document.form.ipv61_ifdev_select, 1);
		else
			inputCtrl(document.form.ipv61_ifdev_select, 0);
		inputCtrl(document.form.ipv61_dhcp_pd[0], 0);
		inputCtrl(document.form.ipv61_dhcp_pd[1], 0);
		showInputfield2('ipv61_ifdev', 0);
		inputCtrl(document.form.ipv61_tun_v4end, 0);
		inputCtrl(document.form.ipv61_relay, 0);
		inputCtrl(document.form.ipv61_6rd_dhcp[0], 0);
		inputCtrl(document.form.ipv61_6rd_dhcp[1], 0);
		inputCtrl(document.form.ipv61_6rd_prefix, 0);
		inputCtrl(document.form.ipv61_6rd_prefixlen, 0);
		inputCtrl(document.form.ipv61_6rd_router, 0);
		inputCtrl(document.form.ipv61_6rd_ip4size, 0);
		inputCtrl(document.form.ipv61_tun_addr, 0);
		inputCtrl(document.form.ipv61_tun_addrlen, 0);
		inputCtrl(document.form.ipv61_tun_peer, 0);
		inputCtrl(document.form.ipv61_tun_mtu, 0);
		inputCtrl(document.form.ipv61_tun_ttl, 0);
		document.getElementById("ipv61_wan_setting").style.display="none";
		inputCtrl(document.form.ipv61_ipaddr, 0);
		inputCtrl(document.form.ipv61_prefix_len_wan, 0);
		inputCtrl(document.form.ipv61_gateway, 0);		
		document.getElementById("ipv61_lan_setting").style.display="none";
		inputCtrl(document.form.ipv61_prefix, 0);
		inputCtrl(document.form.ipv61_prefix_length, 0);
		document.getElementById("ipv61_prefix_r").style.display="none";
		document.getElementById("ipv61_prefix_length_r").style.display="none";
		inputCtrl(document.form.ipv61_autoconf_type[0], 0);
		inputCtrl(document.form.ipv61_autoconf_type[1], 0);
		inputCtrl(document.form.ipv61_dhcp_start_start, 0);
		inputCtrl(document.form.ipv61_dhcp_end_end, 0);
		inputCtrl(document.form.ipv61_dhcp_lifetime, 0);
		document.getElementById("ipv61_ipaddr_r").style.display="none";

		document.getElementById("ipv61_dns_setting").style.display="";
		inputCtrl(document.form.ipv61_dnsenable[0], 1);
		inputCtrl(document.form.ipv61_dnsenable[1], 1);
		var enable_dns = (document.form.ipv61_dnsenable[1].checked) ? '0' : '1';
		showInputfield2('ipv61_dnsenable', enable_dns);
		
		document.getElementById("auto_config").style.display="none";
	}
	else if(v == "6to4"){
		inputCtrl(document.form.ipv61_ifdev_select, 0);
		inputCtrl(document.form.ipv61_dhcp_pd[0], 0);
		inputCtrl(document.form.ipv61_dhcp_pd[1], 0);
		showInputfield2('ipv61_ifdev', 0);
		inputCtrl(document.form.ipv61_tun_v4end, 0);
		inputCtrl(document.form.ipv61_relay, 1);
		inputCtrl(document.form.ipv61_6rd_dhcp[0], 0);
		inputCtrl(document.form.ipv61_6rd_dhcp[1], 0);
		inputCtrl(document.form.ipv61_6rd_prefix, 0);
		inputCtrl(document.form.ipv61_6rd_prefixlen, 0);
		inputCtrl(document.form.ipv61_6rd_router, 0);
		inputCtrl(document.form.ipv61_6rd_ip4size, 0);
		inputCtrl(document.form.ipv61_tun_addr, 0);
		inputCtrl(document.form.ipv61_tun_addrlen, 0);
		inputCtrl(document.form.ipv61_tun_peer, 0);
		inputCtrl(document.form.ipv61_tun_mtu, 1);
		inputCtrl(document.form.ipv61_tun_ttl, 1);
		document.getElementById("ipv61_wan_setting").style.display="none";
		inputCtrl(document.form.ipv61_ipaddr, 0);
		inputCtrl(document.form.ipv61_prefix_len_wan, 0);
		inputCtrl(document.form.ipv61_gateway, 0);		
		document.getElementById("ipv61_lan_setting").style.display="";
		inputCtrl(document.form.ipv61_prefix, 0);
		inputCtrl(document.form.ipv61_prefix_length, 0);
		document.getElementById("ipv61_prefix_r").style.display="";
		document.getElementById("ipv61_prefix_length_r").style.display="";
		var calc_hex = calcIP6(wan1_ipaddr);
		document.getElementById("ipv61_prefix_span").innerHTML="2002:"+calc_hex+"::";
		document.getElementById("ipv61_prefix_length_span").innerHTML="48";
		inputCtrl(document.form.ipv61_rtr_addr, 0);
		inputCtrl(document.form.ipv61_autoconf_type[0], 0);
		inputCtrl(document.form.ipv61_autoconf_type[1], 0);
		inputCtrl(document.form.ipv61_dhcp_start_start, 0);
		inputCtrl(document.form.ipv61_dhcp_end_end, 0);
		inputCtrl(document.form.ipv61_dhcp_lifetime, 0);
		document.getElementById("ipv61_ipaddr_r").style.display="";
		if(v != ipv61_proto_orig){				
			document.getElementById("ipv61_ipaddr_span").innerHTML = "";
		}else{
			document.getElementById("ipv61_ipaddr_span").innerHTML = '<% nvram_get("ipv61_rtr_addr"); %>';
		}

		document.getElementById("ipv61_dns_setting").style.display="";
		inputCtrl(document.form.ipv61_dnsenable[0], 0);
		inputCtrl(document.form.ipv61_dnsenable[1], 0);
		showInputfield2('ipv61_dnsenable', '0');
		
		document.getElementById("auto_config").style.display="";

	}
	else if(v == "6in4"){
		inputCtrl(document.form.ipv61_ifdev_select, 0);
		inputCtrl(document.form.ipv61_dhcp_pd[0], 0);
		inputCtrl(document.form.ipv61_dhcp_pd[1], 0);
		showInputfield2('ipv61_ifdev', 0);
		inputCtrl(document.form.ipv61_tun_v4end, 1);
		inputCtrl(document.form.ipv61_relay, 0);
		inputCtrl(document.form.ipv61_6rd_dhcp[0], 0);
		inputCtrl(document.form.ipv61_6rd_dhcp[1], 0);
		inputCtrl(document.form.ipv61_6rd_prefix, 0);
		inputCtrl(document.form.ipv61_6rd_prefixlen, 0);
		inputCtrl(document.form.ipv61_6rd_router, 0);
		inputCtrl(document.form.ipv61_6rd_ip4size, 0);
		inputCtrl(document.form.ipv61_tun_addr, 1);
		inputCtrl(document.form.ipv61_tun_addrlen, 1);
		inputCtrl(document.form.ipv61_tun_peer, 1);
		inputCtrl(document.form.ipv61_tun_mtu, 1);
		inputCtrl(document.form.ipv61_tun_ttl, 1);
		document.getElementById("ipv61_wan_setting").style.display="none";
		inputCtrl(document.form.ipv61_ipaddr, 0);
		inputCtrl(document.form.ipv61_prefix_len_wan, 0);
		inputCtrl(document.form.ipv61_gateway, 0);		
		document.getElementById("ipv61_lan_setting").style.display="";
		inputCtrl(document.form.ipv61_prefix, 1);
		inputCtrl(document.form.ipv61_prefix_length, 1);
		document.getElementById("ipv61_prefix_r").style.display="none";
		document.getElementById("ipv61_prefix_length_r").style.display="none";
		inputCtrl(document.form.ipv61_rtr_addr, 0);
		inputCtrl(document.form.ipv61_autoconf_type[0], 0);
		inputCtrl(document.form.ipv61_autoconf_type[1], 0);
		inputCtrl(document.form.ipv61_dhcp_start_start, 0);
		inputCtrl(document.form.ipv61_dhcp_end_end, 0);
		inputCtrl(document.form.ipv61_dhcp_lifetime, 0);
		document.getElementById("ipv61_ipaddr_r").style.display="";
		if(v != ipv61_proto_orig){
				document.form.ipv61_prefix.value = "";
				document.form.ipv61_prefix_length.value = "";			
				document.getElementById("ipv61_ipaddr_span").innerHTML = "";
		}else{
				document.getElementById("ipv61_ipaddr_span").innerHTML = "<% nvram_get("ipv61_rtr_addr"); %>";	
		}

		document.getElementById("ipv61_dns_setting").style.display="";
		inputCtrl(document.form.ipv61_dnsenable[0], 0);
		inputCtrl(document.form.ipv61_dnsenable[1], 0);
		showInputfield2('ipv61_dnsenable', '0');
		
		document.getElementById("auto_config").style.display="";

	}
	else if(v == "6rd"){
		inputCtrl(document.form.ipv61_ifdev_select, 0);
		inputCtrl(document.form.ipv61_dhcp_pd[0], 0);
		inputCtrl(document.form.ipv61_dhcp_pd[1], 0);
		showInputfield2('ipv61_ifdev', 0);
		inputCtrl(document.form.ipv61_tun_v4end, 0);
		inputCtrl(document.form.ipv61_relay, 0);
		inputCtrl(document.form.ipv61_6rd_dhcp[0], 1);
		inputCtrl(document.form.ipv61_6rd_dhcp[1], 1);
		var enable = (document.form.ipv61_6rd_dhcp[1].checked) ? 1 : 0;
		inputCtrl(document.form.ipv61_6rd_prefix, enable);
		inputCtrl(document.form.ipv61_6rd_prefixlen, enable);
		inputCtrl(document.form.ipv61_6rd_router, enable);
		inputCtrl(document.form.ipv61_6rd_ip4size, enable);
		inputCtrl(document.form.ipv61_tun_addr, 0);
		inputCtrl(document.form.ipv61_tun_addrlen, 0);
		inputCtrl(document.form.ipv61_tun_peer, 0);
		inputCtrl(document.form.ipv61_tun_mtu, 1);
		inputCtrl(document.form.ipv61_tun_ttl, 1);
		document.getElementById("ipv61_wan_setting").style.display="none";
		inputCtrl(document.form.ipv61_ipaddr, 0);
		inputCtrl(document.form.ipv61_prefix_len_wan, 0);
		inputCtrl(document.form.ipv61_gateway, 0);		
		document.getElementById("ipv61_lan_setting").style.display="";
		inputCtrl(document.form.ipv61_prefix, 0);
		inputCtrl(document.form.ipv61_prefix_length, 0);
		document.getElementById("ipv61_prefix_r").style.display="";
		document.getElementById("ipv61_prefix_length_r").style.display="";
		inputCtrl(document.form.ipv61_rtr_addr, 0);
		inputCtrl(document.form.ipv61_autoconf_type[0], 0);
		inputCtrl(document.form.ipv61_autoconf_type[1], 0);
		inputCtrl(document.form.ipv61_dhcp_start_start, 0);
		inputCtrl(document.form.ipv61_dhcp_end_end, 0);
		inputCtrl(document.form.ipv61_dhcp_lifetime, 0);
		document.getElementById("ipv61_ipaddr_r").style.display="";
		var enable = (document.form.ipv61_6rd_dhcp[1].checked) ? '0' : '1';
		showInputfield2('ipv61_6rd_dhcp', enable);
		document.getElementById("ipv61_dns_setting").style.display="";
		inputCtrl(document.form.ipv61_dnsenable[0], 0);
		inputCtrl(document.form.ipv61_dnsenable[1], 0);
		showInputfield2('ipv61_dnsenable', '0');
		
		document.getElementById("auto_config").style.display="";

	}
	else if(v == "other"){
		if(wan_proto_orig == "l2tp" || wan_proto_orig == "pptp" || wan_proto_orig == "pppoe")
			inputCtrl(document.form.ipv61_ifdev_select, 1);
		else
			inputCtrl(document.form.ipv61_ifdev_select, 0);
		inputCtrl(document.form.ipv61_dhcp_pd[0], 0);
		inputCtrl(document.form.ipv61_dhcp_pd[1], 0);
		showInputfield2('ipv61_ifdev', 0);
		inputCtrl(document.form.ipv61_tun_v4end, 0);
		inputCtrl(document.form.ipv61_relay, 0);
		inputCtrl(document.form.ipv61_6rd_dhcp[0], 0);
		inputCtrl(document.form.ipv61_6rd_dhcp[1], 0);		
		inputCtrl(document.form.ipv61_6rd_prefix, 0);
		inputCtrl(document.form.ipv61_6rd_prefixlen, 0);
		inputCtrl(document.form.ipv61_6rd_router, 0);
		inputCtrl(document.form.ipv61_6rd_ip4size, 0);		
		inputCtrl(document.form.ipv61_tun_addr, 0);
		inputCtrl(document.form.ipv61_tun_addrlen, 0);
		inputCtrl(document.form.ipv61_tun_peer, 0);
		inputCtrl(document.form.ipv61_tun_mtu, 0);
		inputCtrl(document.form.ipv61_tun_ttl, 0);		
		document.getElementById("ipv61_wan_setting").style.display="";
		inputCtrl(document.form.ipv61_ipaddr, 1);
		inputCtrl(document.form.ipv61_prefix_len_wan, 1);
		inputCtrl(document.form.ipv61_gateway, 1);
		if(v != ipv61_proto_orig){
				document.form.ipv61_ipaddr.value="";
				document.form.ipv61_prefix_len_wan.value="";
				document.form.ipv61_gateway.value="";
		}
		document.getElementById("ipv61_lan_setting").style.display="";
		document.getElementById("ipv61_prefix_r").style.display="";
		document.getElementById("ipv61_prefix_length_r").style.display="none";
		
		inputCtrl(document.form.ipv61_prefix, 0);
		inputCtrl(document.form.ipv61_prefix_length, 1);
		inputCtrl(document.form.ipv61_rtr_addr, 1);
		inputCtrl(document.form.ipv61_autoconf_type[0], 1);
		inputCtrl(document.form.ipv61_autoconf_type[1], 1);
		
		if(v != ipv61_proto_orig){
				
				document.form.ipv61_autoconf_type[0].checked = true;
				showInputfield2('ipv61_autoconf_type', 0);
				document.getElementById("ipv61_prefix_span").innerHTML = "";
				document.form.ipv61_prefix_length.value = "";
				document.form.ipv61_rtr_addr.value = "";
		}
		else{
				if("<% nvram_get("ipv61_autoconf_type"); %>" == 0)
					document.form.ipv61_autoconf_type[0].checked = true;
				else
					document.form.ipv61_autoconf_type[1].checked = true;
				showInputfield2('ipv61_autoconf_type', "<% nvram_get("ipv61_autoconf_type"); %>");
				document.getElementById("ipv61_prefix_span").innerHTML = "<% nvram_get("ipv61_prefix"); %>";
				document.form.ipv61_prefix_length.value = "<% nvram_get("ipv61_prefix_length"); %>";
				document.form.ipv61_rtr_addr.value = "<% nvram_get("ipv61_rtr_addr"); %>";
		}
		document.getElementById("ipv61_ipaddr_r").style.display="none";
		document.getElementById("ipv61_dns_setting").style.display="";
		inputCtrl(document.form.ipv61_dnsenable[0], 0);
		inputCtrl(document.form.ipv61_dnsenable[1], 0);
		showInputfield2('ipv61_dnsenable', '0');
		
		document.getElementById("auto_config").style.display="";
		
	}	
	else{		// disabled
		inputCtrl(document.form.ipv61_ifdev_select, 0);
		inputCtrl(document.form.ipv61_dhcp_pd[0], 0);
		inputCtrl(document.form.ipv61_dhcp_pd[1], 0);
		showInputfield2('ipv61_ifdev', 0);
		inputCtrl(document.form.ipv61_tun_v4end, 0);
		inputCtrl(document.form.ipv61_relay, 0);
		inputCtrl(document.form.ipv61_6rd_dhcp[0], 0);
		inputCtrl(document.form.ipv61_6rd_dhcp[1], 0);
		inputCtrl(document.form.ipv61_6rd_prefix, 0);
		inputCtrl(document.form.ipv61_6rd_prefixlen, 0);
		inputCtrl(document.form.ipv61_6rd_router, 0);
		inputCtrl(document.form.ipv61_6rd_ip4size, 0);
		inputCtrl(document.form.ipv61_tun_addr, 0);
		inputCtrl(document.form.ipv61_tun_addrlen, 0);
		inputCtrl(document.form.ipv61_tun_peer, 0);
		inputCtrl(document.form.ipv61_tun_mtu, 0);
		inputCtrl(document.form.ipv61_tun_ttl, 0);
		document.getElementById("ipv61_wan_setting").style.display="none";
		inputCtrl(document.form.ipv61_ipaddr, 0);
		inputCtrl(document.form.ipv61_prefix_len_wan, 0);
		inputCtrl(document.form.ipv61_gateway, 0);		
		document.getElementById("ipv61_lan_setting").style.display="none";
		inputCtrl(document.form.ipv61_prefix, 0);
		inputCtrl(document.form.ipv61_prefix_length, 0);
		document.getElementById("ipv61_prefix_r").style.display="none";
		document.getElementById("ipv61_prefix_length_r").style.display="none";
		inputCtrl(document.form.ipv61_rtr_addr, 0);
		inputCtrl(document.form.ipv61_autoconf_type[0], 0);
		inputCtrl(document.form.ipv61_autoconf_type[1], 0);
		inputCtrl(document.form.ipv61_dhcp_start_start, 0);
		inputCtrl(document.form.ipv61_dhcp_end_end, 0);
		inputCtrl(document.form.ipv61_dhcp_lifetime, 0);
		document.getElementById("ipv61_ipaddr_r").style.display="none";
		document.getElementById("ipv61_dns_setting").style.display="none";
		inputCtrl(document.form.ipv61_dnsenable[0], 0);
		inputCtrl(document.form.ipv61_dnsenable[1], 0);
		showInputfield2('ipv61_dnsenable', '1');
		
		document.getElementById("auto_config").style.display="none";
		
	}		
	
	if(v != ipv61_proto_orig){
		update_info(0);
	}else{
		update_info(1);
	}
	
}

// Viz 2013.08 modify for dhcp-pd {
function showInputfield2(s, v){
	var enable = (v=='0') ? 1 : 0;
	if(s=='ipv61_6rd_dhcp'){
		inputCtrl(document.form.ipv61_6rd_prefix, enable);
		inputCtrl(document.form.ipv61_6rd_prefixlen, enable);
		inputCtrl(document.form.ipv61_6rd_router, enable);
		inputCtrl(document.form.ipv61_6rd_ip4size, enable);

		if(v != ipv61_tun6rd_dhcp || document.form.ipv61_service.value != ipv61_proto_orig){
			document.getElementById("ipv61_prefix_span").innerHTML = "";
			document.getElementById("ipv61_prefix_length_span").innerHTML = "";
			document.getElementById("ipv61_ipaddr_span").innerHTML = "";
		}else{
			document.getElementById("ipv61_prefix_span").innerHTML = "<% nvram_get("ipv61_prefix"); %>";
			document.getElementById("ipv61_prefix_length_span").innerHTML = "<% nvram_get("ipv61_prefix_length"); %>";
			document.getElementById("ipv61_ipaddr_span").innerHTML = "<% nvram_get("ipv61_rtr_addr"); %>";
		}
	
	}else if(s=='ipv61_dnsenable'){
		inputCtrl(document.form.ipv61_dns1, enable);
		inputCtrl(document.form.ipv61_dns2, enable);
		inputCtrl(document.form.ipv61_dns3, enable);
		
	}else if(s=='ipv61_dhcp_pd'){
		inputCtrl(document.form.ipv61_rtr_addr, enable);
		inputCtrl(document.form.ipv61_prefix_length, enable);

		if(enable){
			document.getElementById("ipv61_ipaddr_r").style.display = "none";
			document.getElementById("ipv61_prefix_length_r").style.display = "none";
				
		}else{
			document.getElementById("ipv61_ipaddr_r").style.display = "";
			document.getElementById("ipv61_prefix_length_r").style.display = "";
			document.getElementById("ipv61_prefix_length_span").innerHTML = "";
		}
		
		if(document.form.ipv61_autoconf_type[0].checked == true){
			showInputfield2('ipv61_autoconf_type', '0');
		}else{
			showInputfield2('ipv61_autoconf_type', '1');
		}
	}else if(s=='ipv61_autoconf_type'){
		
		if(document.form.ipv61_dhcp_pd[0].checked == true)
			document.getElementById("ipv61_prefix_span").innerHTML = GetIPv6_split(document.getElementById('ipv61_ipaddr_span').innerHTML)+"::";
		else
			document.getElementById("ipv61_prefix_span").innerHTML = GetIPv6_split(document.form.ipv61_rtr_addr.value)+"::";
		if(document.getElementById("ipv61_prefix_span").innerHTML == "::")
			document.getElementById("ipv61_prefix_span").innerHTML = "";
		inputCtrl(document.form.ipv61_dhcp_start_start, !enable);
		inputCtrl(document.form.ipv61_dhcp_end_end, !enable);
		inputCtrl(document.form.ipv61_dhcp_lifetime, !enable);
		
		if(!document.form.ipv61_dhcp_pd[0].checked || document.form.ipv61_service.value == "other"){	//ipv61_dhcp_pd for dhcp6
			if(document.form.ipv61_rtr_addr.value != "")
				var IPv61_rtr_addr_split = GetIPv6_split(document.form.ipv61_rtr_addr.value);			
			else
				var IPv61_rtr_addr_split = "";
						
			document.form.ipv61_prefix_span_for_start.value = IPv61_rtr_addr_split;
			document.form.ipv61_prefix_span_for_end.value = IPv61_rtr_addr_split;
			
			
			if(ipv61_dhcp_start_orig != "" && ipv61_dhcp_end_orig != ""){
				document.form.ipv61_dhcp_start_start.value = ipv61_dhcp_start_orig.split("::")[1];
				document.form.ipv61_dhcp_end_end.value = ipv61_dhcp_end_orig.split("::")[1];	
			}else{
				document.form.ipv61_dhcp_start_start.value = 1000;
				document.form.ipv61_dhcp_end_end.value = 2000;
			}
			
		}
		else if(!document.form.ipv61_dhcp_pd[1].checked){
			if(document.getElementById('ipv61_ipaddr_span').innerHTML != "")
				var IPv61_rtr_addr_split = GetIPv6_split(document.getElementById('ipv61_ipaddr_span').innerHTML);
			else
				var IPv61_rtr_addr_split = "";
				
				document.form.ipv61_prefix_span_for_start.value = IPv61_rtr_addr_split;
				document.form.ipv61_prefix_span_for_end.value = IPv61_rtr_addr_split;
				
				
			if(ipv61_dhcp_start_orig != "" && ipv61_dhcp_end_orig != ""){
				document.form.ipv61_dhcp_start_start.value = ipv61_dhcp_start_orig.split("::")[1];
				document.form.ipv61_dhcp_end_end.value = ipv61_dhcp_end_orig.split("::")[1];	
			}else{
				document.form.ipv61_dhcp_start_start.value = 1000;
				document.form.ipv61_dhcp_end_end.value = 2000;
			}			
		}
		else if(ipv61_dhcp_start_orig != "" && ipv61_dhcp_end_orig != ""){
				document.form.ipv61_prefix_span_for_start.value = ipv61_dhcp_start_orig.split("::")[0];
				document.form.ipv61_prefix_span_for_end.value = ipv61_dhcp_end_orig.split("::")[0];
				document.form.ipv61_dhcp_start_start.value = ipv61_dhcp_start_orig.split("::")[1];
				document.form.ipv61_dhcp_end_end.value = ipv61_dhcp_end_orig.split("::")[1];
		}
	}else if(s=='ipv61_ifdev'){
		var enable = (document.form.ipv61_service.value == "dhcp6" && v == "ppp") ? 1 : 0;
		inputCtrl(document.form._ipv61_accept_defrtr[0], enable);
		inputCtrl(document.form._ipv61_accept_defrtr[1], enable);
	}
}
// } Viz 2013.08 modify for dhcp-pd 


// test if WAN IP & Gateway & DNS IP is a valid IP
// DNS IP allows to input nothing
function valid_IP(obj_name, obj_flag){
		// A : 1.0.0.0~126.255.255.255
		// B : 127.0.0.0~127.255.255.255 (forbidden)
		// C : 128.0.0.0~255.255.255.254
		var A_class_start = inet_network("1.0.0.0");
		var A_class_end = inet_network("126.255.255.255");
		var B_class_start = inet_network("127.0.0.0");
		var B_class_end = inet_network("127.255.255.255");
		var C_class_start = inet_network("128.0.0.0");
		var C_class_end = inet_network("255.255.255.255");
		
		var ip_obj = obj_name;
		var ip_num = inet_network(ip_obj.value);

		if(obj_flag == "DNS" && ip_num == -1){ //DNS allows to input nothing
			return true;
		}
		
		if(obj_flag == "GW" && ip_num == -1){ //GW allows to input nothing
			return true;
		}
		
		if(ip_num > A_class_start && ip_num < A_class_end){
		   obj_name.value = ipFilterZero(ip_obj.value);
			return true;
		}
		else if(ip_num > B_class_start && ip_num < B_class_end){
			alert(ip_obj.value+" <#JS_validip#>");
			ip_obj.value = "";
			ip_obj.focus();
			ip_obj.select();
			return false;
		}
		else if(ip_num > C_class_start && ip_num < C_class_end){
		        obj_name.value = ipFilterZero(ip_obj.value);
			return true;
		}
		else{
			alert(ip_obj.value+" <#JS_validip#>");
			ip_obj.value = "";
			ip_obj.focus();
			ip_obj.select();
			return false;
		}
}

function ipv6_valid(obj){
	//var rangere=new RegExp("^[a-f0-9]{1,4}:([a-f0-9]{0,4}:){2,6}[a-f0-9]{1,4}$", "gi");	
	var rangere=new RegExp("^((([0-9A-Fa-f]{1,4}:){7}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}:[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){5}:([0-9A-Fa-f]{1,4}:)?[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){4}:([0-9A-Fa-f]{1,4}:){0,2}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){3}:([0-9A-Fa-f]{1,4}:){0,3}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){2}:([0-9A-Fa-f]{1,4}:){0,4}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|(([0-9A-Fa-f]{1,4}:){0,5}:((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|(::([0-9A-Fa-f]{1,4}:){0,5}((\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b)\\.){3}(\\b((25[0-5])|(1\\d{2})|(2[0-4]\\d)|(\\d{1,2}))\\b))|([0-9A-Fa-f]{1,4}::([0-9A-Fa-f]{1,4}:){0,5}[0-9A-Fa-f]{1,4})|(::([0-9A-Fa-f]{1,4}:){0,6}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){1,7}:))$", "gi");
	if(rangere.test(obj.value)){
			//alert(obj.value+"good");	
			return true;
	}else{
			alert(obj.value+" <#JS_validip#>");
			obj.focus();
			obj.select();			
			return false;
	}	
}

function GetIPv6_split(obj){
	
	var Split_1_IPv6 = obj.split("::");
	var Split_1_IPv6_pos = obj.search("::");
	var Split_2_IPv6 = obj.split(":");
	var return_prefix = "";
	if(Split_1_IPv6.length >1){
		if(Split_1_IPv6[0].substring(0,Split_1_IPv6_pos).split(":").length >4){	//get ipv6_prefix by Split_2_IPv6[0]~[3]
			for(i=0;i<4;i++){
				return_prefix += Split_2_IPv6[i];
				if(i<3)
					return_prefix += ":";
			}
		}else{
			return_prefix = Split_1_IPv6[0];
		}		
	}else if(Split_2_IPv6.length > 1){
		for(j=0;j<4;j++){
			return_prefix += Split_2_IPv6[j];
			if(j<3)
				return_prefix += ":";
		}		
	}	
	return return_prefix;
}

function validIPv6_dhcp(obj){
	var dhcpre=new RegExp("^([0-9A-Fa-f]{1,4})$", "gi");
	if(!dhcpre.test(obj.value)){
		alert(obj.value +" <#JS_validip#>");
		obj.focus();
		obj.select();			
		return false;
	}
	return true;
}


function calcIP6(ip) {
	var octet = ip.split(".");
	base = 16;
	var octet1 = parseInt(octet[0]);
	var octet2 = parseInt(octet[1]);
	var octet3 = parseInt(octet[2]);
	var octet4 = parseInt(octet[3]);
	hextet21 = octet1.toString(base);
	hextet22 = octet2.toString(base);
	hextet31 = octet3.toString(base);
	hextet32 = octet4.toString(base);

	var hextetval21 = (octet1 < 16) ? "0" + hextet21 : hextet21;	
	var hextetval22 = (octet2 < 16) ? "0" + hextet22 : hextet22;
	var hextetval31 = (octet3 < 16) ? "0" + hextet31 : hextet31;
	var hextetval32 = (octet4 < 16) ? "0" + hextet32 : hextet32;

	var Calc_hex = hextetval21+hextetval22+":"+hextetval31+hextetval32;
	return Calc_hex;
}

function validForm(){
	
	if(document.form.ipv61_service.value=="other"){
		if(!ipv6_valid(document.form.ipv61_ipaddr) || 
				!validator.range(document.form.ipv61_prefix_len_wan, 3, 128)){
				return false;
		}
		if(document.form.ipv61_gateway.value != "" &&
				!ipv6_valid(document.form.ipv61_gateway)){
				return false;
		}
		
				// stateful autconf eats 16 bits of 128
		if(!validator.range(document.form.ipv61_prefix_length, 3,
				document.form.ipv61_autoconf_type[1].checked ? 112 : 126) ||
				!ipv6_valid(document.form.ipv61_rtr_addr)){
				return false;
		}

		if(document.form.ipv61_autoconf_type[1].checked){
							
			if(!validIPv6_dhcp(document.form.ipv61_dhcp_start_start))
				return false;
			if(!validIPv6_dhcp(document.form.ipv61_dhcp_end_end))
				return false;											
																
			if(parseInt("0x"+document.form.ipv61_dhcp_start_start.value) > parseInt("0x"+document.form.ipv61_dhcp_end_end.value)){
				alert("<#vlaue_haigher_than#> "+document.form.ipv61_dhcp_start_start.value);
				document.form.ipv61_dhcp_end_end.focus();
				document.form.ipv61_dhcp_end_end.select();    									
				return false;
			}

			if(!validator.range(document.form.ipv61_dhcp_lifetime, 120, 604800)){
				document.form.ipv61_dhcp_lifetime.focus();
				document.form.ipv61_dhcp_lifetime.select();
				return false;	
			}
		}
		
	}else if(document.form.ipv61_service.value=="dhcp6"){
		if(document.form.ipv61_dhcp_pd[1].checked){
			// stateful autconf eats 16 bits of 128
			if(!validator.range(document.form.ipv61_prefix_length, 3,
				document.form.ipv61_autoconf_type[1].checked ? 112 : 126) ||
				!ipv6_valid(document.form.ipv61_rtr_addr)){
				return false;	
			}
		}

		if(document.form.ipv61_autoconf_type[1].checked){
								
			if(!validIPv6_dhcp(document.form.ipv61_dhcp_start_start))
				return false;
			if(!validIPv6_dhcp(document.form.ipv61_dhcp_end_end))
				return false;											
																	
			if(parseInt("0x"+document.form.ipv61_dhcp_start_start.value) > parseInt("0x"+document.form.ipv61_dhcp_end_end.value)){
				alert("<#vlaue_haigher_than#> "+document.form.ipv61_dhcp_start_start.value);
				document.form.ipv61_dhcp_end_end.focus();
				document.form.ipv61_dhcp_end_end.select();
				return false;
			}

			if(!validator.range(document.form.ipv61_dhcp_lifetime, 120, 604800)){
				document.form.ipv61_dhcp_lifetime.focus();
				document.form.ipv61_dhcp_lifetime.select();
				return false;	
			}
		}			
	}else if(document.form.ipv61_service.value=="6to4" ||document.form.ipv61_service.value=="6in4" ||document.form.ipv61_service.value=="6rd" ){
		
			if(!validator.rangeAllowZero(document.form.ipv61_tun_mtu,1280,1480,0))  return false;  //MTU
			if(!validator.rangeAllowZero(document.form.ipv61_tun_ttl,0,255,255))  return false;  //TTL				

	}	
	

	if((document.form.ipv61_service.value=="dhcp6" && document.form.ipv61_dnsenable[1].checked) ||
	   (IPv6_Passthrough_support &&
	   (document.form.ipv61_service.value=="ipv6pt" || document.form.ipv61_service.value=="flets") && document.form.ipv61_dnsenable[1].checked) ||
	    document.form.ipv61_service.value=="other" ||
	    document.form.ipv61_service.value=="6to4" || document.form.ipv61_service.value=="6in4" || document.form.ipv61_service.value=="6rd"){
		if(document.form.ipv61_dns1.value != "")
			if(!ipv61_valid(document.form.ipv61_dns1)) return false;
		if(document.form.ipv61_dns2.value != "")
			if(!ipv61_valid(document.form.ipv61_dns2)) return false;
		if(document.form.ipv61_dns3.value != "")
			if(!ipv61_valid(document.form.ipv61_dns3)) return false;
	}

	if(document.form.ipv61_service.value=="6to4"){
			if(!valid_IP(document.form.ipv61_relay, "")) return false;  //6to4 tun relay	
	}
	
	if(document.form.ipv61_service.value=="6in4"){
			if(!validator.ipRange(document.form.ipv61_tun_v4end, "")) return false;  //6in4 tun endpoint	
			if(!ipv6_valid(document.form.ipv61_tun_addr)) return false;  //6in4 Client IPv6 Address			
			if(!validator.range(document.form.ipv61_tun_addrlen, 3, 128))  return false;
			if(document.form.ipv61_tun_peer.value != "" && !ipv6_valid(document.form.ipv61_tun_peer)) return false;
			if(!validator.range(document.form.ipv61_prefix_length, 3, 126) ||
					!ipv6_valid(document.form.ipv61_prefix)){
					return false;
			}
	}		
	
	if(document.form.ipv61_service.value=="6rd" && document.form.ipv61_6rd_dhcp[1].checked){
			if(!validator.ipRange(document.form.ipv61_6rd_router, "")) return false;  //6rd ip4 router
			if(!validator.range(document.form.ipv61_6rd_ip4size, 0, 32)) return false;  //6rd ip4 router mask length
			if(!ipv6_valid(document.form.ipv61_6rd_prefix) ||
					!validator.range(document.form.ipv61_6rd_prefixlen, 3,
					126 - (32 - document.form.ipv61_6rd_ip4size.value))){
					return false;
			}
	}
	
	return true;
}


function applyRule(){
	if(validForm()){

		if(document.form.ipv61_service.value=="dhcp6"){
			if(document.form.ipv61_dhcp_pd[1].checked){
					
				document.form.ipv61_prefix_length.disabled = false;
				document.form.ipv61_prefix.disabled = false;
			}

			document.form.ipv61_accept_defrtr.disabled = false;
			document.form.ipv61_accept_defrtr.value = document.form._ipv61_accept_defrtr[0].checked?1:0;

			if(document.form.ipv61_autoconf_type[1].checked){
				document.form.ipv61_dhcp_start.disabled = false;
				document.form.ipv61_dhcp_start.value = document.form.ipv61_prefix_span_for_start.value +"::"+document.form.ipv61_dhcp_start_start.value;
				document.form.ipv61_dhcp_end.disabled = false;
				document.form.ipv61_dhcp_end.value = document.form.ipv61_prefix_span_for_end.value +"::"+document.form.ipv61_dhcp_end_end.value;
			}
		}

		if(IPv6_Passthrough_support && document.form.ipv61_service.value=="flets"){
			inputCtrl(document.form.ipv61_ifdev_select, 1);
			document.form.ipv61_ifdev.value = "eth";
		}else if(document.form.ipv61_ifdev_select.disabled){	// set ipv6_ifdev="ppp" while interface is disabled.
			document.form.ipv61_ifdev.value = "ppp";
		}else{
			document.form.ipv61_ifdev.value = document.form.ipv61_ifdev_select.value;
		}			
				
		if(document.form.ipv61_service.value!="other"
				&& (document.form.ipv61_service.value!="dhcp6" && document.form.ipv61_dhcp_pd.value!="0"))	//clean up ipv61_rtr_addr if not other or dhcp_pd=1
				document.form.ipv61_rtr_addr.value = "";

		//Start:  Viz add to store ipv61_prefix_length & ipv61_rtr_addr for 'other' 2014.12.08
		if(document.form.ipv61_service.value == "other"){
			document.form.ipv61_prefix_length_s.value = document.form.ipv61_prefix_length.value;
			document.form.ipv61_rtr_addr_s.value = document.form.ipv61_rtr_addr.value;

			if(document.form.ipv61_autoconf_type[1].checked){
				document.form.ipv61_dhcp_start.disabled = false;
				document.form.ipv61_dhcp_start.value = document.form.ipv61_prefix_span_for_start.value +"::"+document.form.ipv61_dhcp_start_start.value;
				document.form.ipv61_dhcp_end.disabled = false;
				document.form.ipv61_dhcp_end.value = document.form.ipv61_prefix_span_for_end.value +"::"+document.form.ipv61_dhcp_end_end.value;
			}
		}
		
		if(document.form.ipv61_service.value == "6in4"){
				document.form.ipv61_prefix_length_s.value = document.form.ipv61_prefix_length.value;
				document.form.ipv61_prefix_s.value = document.form.ipv61_prefix.value;
		}
		//End

		document.form.ipv61_radvd.value = document.form._ipv61_radvd[0].checked?1:0;

		/*if(machine_arm)	//Viz 2013.06 Don't need to reboot anymore
		{ // MODELDEP: Machine ARM structure
			if((document.form.ipv61_service.value == "disabled" && ipv61_proto_orig != "disabled") ||
				 (document.form.ipv61_service.value != "disabled" && ipv61_proto_orig == "disabled"))
    		FormActions("start_apply.htm", "apply", "reboot", "<% get_default_reboot_time(); %>");
		}*/

		/*if(based_modelid == "RT-AC66U" || based_modelid == "RT-N66U" )	//Viz 2014.4.16: SDK 6.x need to shut down CTF while switch 6in4, do reboot
		{ // MODELDEP: RT-AC66U, RT-N66U
			if((document.form.ipv61_service.value != ipv61_proto_orig)
				&& (document.form.ipv61_service.value == "6in4" || ipv61_proto_orig == "6in4"))
    		FormActions("start_apply.htm", "apply", "reboot", "<% get_default_reboot_time(); %>");
		}*/
	
		showLoading();

		setTimeout(function(){
			document.form.submit();
		},1500);
	}
}

/*------------ get IPv6 info Start -----------------*/
function update_info(flag){
	if(flag == 0)
			return false;
			
		$.ajax({
				url: '/update_IPv6state.asp',
				dataType: 'script',
				timeout: 1500,
				error: function(xhr){
						setTimeout("update_info();", 1500);
				},
				success: function(response){
						showInfo();
				}
		});
	
}	

function showInfo(){
	if(document.form.ipv61_service.value == ipv61_proto_orig){
		if(document.getElementById("ipv61_prefix_r").style.display == ""){
				document.getElementById("ipv61_prefix_span").innerHTML = state_ipv61_prefix;
		}
		if(document.getElementById("ipv61_prefix_length_r").style.display == ""){
				document.getElementById("ipv61_prefix_length_span").innerHTML = state_ipv61_prefix_length;
		}
		if(document.getElementById("ipv61_ipaddr_r").style.display == ""){
				document.getElementById("ipv61_ipaddr_span").innerHTML = state_ipv61_rtr_addr;
		}
		setTimeout("update_info();", 1500);
	}
}
/*------------- get IPv6 info end ----------------------------*/

function changeWANUnit(obj){
	if( obj.selectedIndex == 0){
		document.ipv6_form.current_page.value = "Advanced_IPv6_Content.asp";
		document.ipv6_form.ipv6_unit.value = "0";
	}else if( obj.selectedIndex == 1){
		document.ipv6_form.current_page.value = "Advanced_IPv61_Content.asp";
		document.ipv6_form.ipv6_unit.value = "1";
	}

	FormActions("apply.cgi", "change_ipv6_unit", "", "");
	document.ipv6_form.target = "";
	document.ipv6_form.submit();
}

function genWANSoption(){
	for(i=0; i<wans_dualwan_array.length; i++){
		var wans_dualwan_NAME = wans_dualwan_array[i].toUpperCase();
        //MODELDEP: DSL-N55U, DSL-N55U-B, DSL-AC68U, DSL-AC68R
        if(wans_dualwan_NAME != "USB"){
	        if(wans_dualwan_NAME == "LAN" && 
	          (productid == "DSL-N55U" || productid == "DSL-N55U-B" || productid == "DSL-AC68U" || productid == "DSL-AC68R")) 
	        	wans_dualwan_NAME = "Ethernet WAN";
			else if(wans_dualwan_NAME == "LAN")
	        	wans_dualwan_NAME = "LAN Port " + '<% nvram_get("wans_lanport"); %>';
				
			document.form.wan_selection.options[i] = new Option(wans_dualwan_NAME, i);
		}

		if(based_modelid == "GT-AXY16000" || based_modelid == "RT-AX89U"){
			if(wans_dualwan_NAME == "WAN2")
				document.form.wan_selection.options[i] = new Option("10G base-T", i);
			else if(wans_dualwan_NAME == "SFP+")
				document.form.wan_selection.options[i] = new Option("10G SFP+", i);
		}
	}

	document.form.wan_selection.selectedIndex = parseInt(ipv6_unit);
}

</script>
</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>
<div id="hiddenMask" class="popup_bg">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
		<tr>
		<td>
			<div class="drword" id="drword" style="height:110px;"><#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...
				<br/>
				<br/>
	    </div>
		  <div class="drImg"><img src="images/alertImg.png"></div>
			<div style="height:70px;"></div>
		</td>
		</tr>
	</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_IPv61_Content.asp">
<input type="hidden" name="next_page" value="Advanced_IPv61_Content.asp">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply_new">
<input type="hidden" name="action_script" value="restart_net">
<input type="hidden" name="action_wait" value="30">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="wan_unit" value="0">
<input type="hidden" name="ipv61_ifdev" value="">
<input type="hidden" name="ipv61_dhcp_start" value="<% nvram_get("ipv61_dhcp_start"); %>" disabled>
<input type="hidden" name="ipv61_dhcp_end" value="<% nvram_get("ipv61_dhcp_start"); %>" disabled>
<input type="hidden" name="ipv61_prefix_length_s" value="">
<input type="hidden" name="ipv61_rtr_addr_s" value="">
<input type="hidden" name="ipv61_prefix_s" value="">
<input type="hidden" name="ipv61_radvd" value="<% nvram_get("ipv61_radvd"); %>">
<input type="hidden" name="ipv61_accept_defrtr" value="<% nvram_get("ipv61_accept_defrtr"); %>" disabled>
<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td width="17">&nbsp;</td>
	
	<!--=====Beginning of Main Menu=====-->
	<td valign="top" width="202">
	  <div id="mainMenu"></div>
	  <div id="subMenu"></div>
	</td>
	
	<td valign="top">
	<div id="tabMenu" class="submenuBlock"></div>
		<!--===================================Beginning of Main Content===========================================-->
	<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
	<tr>
		<td align="left" valign="top">
	  <table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
		<tbody>
		<tr>
			<td bgcolor="#4D595D" valign="top">
				<div>&nbsp;</div>
				<div class="formfonttitle">IPv6</div>
	      		<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
	      <div class="formfontdesc"><#LANHostConfig_display6_sectiondesc#></div>
				<div class="formfontdesc" style="margin-top:-10px;">
					<a id="faq" href="" target="_blank" style="font-family:Lucida Console;text-decoration:underline;">IPv6 FAQ</a>
				</div>
				  
			<!--================================basic_config start=================================================-->
			<table id="basic_config" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
			  	<thead>
				  	<tr>
						<td colspan="2"><#t2BC#></td>
				  	</tr>
			  	</thead>
				<tr>
					<th><#IPv6_WAN_Selection#></th>
		     		<td>
		     			<select name="wan_selection" class="input_option" onchange="changeWANUnit(this);">
		     			</select>
		     		</td>
				</tr>
				<tr>
					<th><#Connectiontype#></th>
		     		<td>
							<select id="ipv61_service" name="ipv61_service" class="input_option" onchange="showInputfield(this.value);">
								<option value="disabled" <% nvram_match("ipv61_service", "disabled", "selected"); %>><#btn_disable#></option>
								<option value="dhcp6" <% nvram_match("ipv61_service", "dhcp6", "selected"); %>>Native</option>
								<option value="other" <% nvram_match("ipv61_service", "other", "selected"); %>><#IPv6_static_IP#></option>
								<option value="ipv6pt" <% nvram_match("ipv61_service", "ipv6pt", "selected"); %>>Passthrough</option>
								<option value="flets" <% nvram_match("ipv61_service", "flets", "selected"); %>>FLET'S IPv6 service</option>
								<option value="6to4" <% nvram_match("ipv61_service", "6to4", "selected"); %>>Tunnel 6to4</option>
								<option value="6in4" <% nvram_match("ipv61_service", "6in4", "selected"); %>>Tunnel 6in4</option>
								<option value="6rd" <% nvram_match("ipv61_service", "6rd", "selected"); %>>Tunnel 6rd</option>
								<!--option value="slaac" <% nvram_match("ipv61_service", "slaac", "selected"); %>>SLAAC</option-->
								<!--option value="icmp6" <% nvram_match("ipv61_service", "icmp6", "selected"); %>>ICMPv6</option-->
							</select>
		     		</td>
	     		</tr>
	     			     	
				<tr>
					<th><#wan_interface#></th>
		     		<td>
						<select name="ipv61_ifdev_select" class="input_option" onchange="showInputfield2('ipv61_ifdev', this.value);">
							<option class="content_input_fd" value="ppp" <% nvram_match("ipv61_ifdev", "ppp","selected"); %>>PPP</option>
							<option class="content_input_fd" value="eth" <% nvram_match("ipv61_ifdev", "eth","selected"); %>><#wan_ethernet#></option>
						</select>
		     		</td>
		     	</tr>
	     	
				<tr style="display:none;"><!-- Viz add dhcp-pd 2013.08-->
					<th>DHCP-PD</th>
		     		<td>
						<input type="radio" name="ipv61_dhcp_pd" class="input" value="1" onclick="showInputfield2('ipv61_dhcp_pd', this.value);" <% nvram_match("ipv61_dhcp_pd", "1","checked"); %>><#WLANConfig11b_WirelessCtrl_button1name#>
						<input type="radio" name="ipv61_dhcp_pd" class="input" value="0" onclick="showInputfield2('ipv61_dhcp_pd', this.value);" <% nvram_match("ipv61_dhcp_pd", "0","checked"); %>><#btn_disable#>
		     		</td>
		     	</tr>
		     	<tr style="display:none;"><!-- Viz add ipv6_accept_defrtr 2019.01-->
					<th>Accept Default Route</th>		<!-- Untranslated -->
					<td>
						<input type="radio" name="_ipv61_accept_defrtr" class="input" value="1" <% nvram_match("ipv61_accept_defrtr", "1","checked"); %>><#WLANConfig11b_WirelessCtrl_button1name#>
						<input type="radio" name="_ipv61_accept_defrtr" class="input" value="0" <% nvram_match("ipv61_accept_defrtr", "0","checked"); %>><#btn_disable#>
					</td>
				</tr>

				<tr style="display:none;">
					<th><#IPv6_tun_v4end#></th>
		     		<td>
						<input type="text" maxlength="15" class="input_15_table" name="ipv61_tun_v4end" value="<% nvram_get("ipv61_tun_v4end"); %>" autocorrect="off" autocapitalize="off">
		     		</td>
		     	</tr>

				<tr style="display:none;">
					<th><#IPv6_relay#></th>
		     		<td>
						<input type="text" maxlength="15" class="input_15_table" name="ipv61_relay" value='<% nvram_get("ipv61_relay"); %>' autocorrect="off" autocapitalize="off">
		     		</td>
		     	</tr>

				<tr style="display:none;">
					<th><#ipv6_6rd_dhcp_option#></th>
		     		<td>
						<input type="radio" name="ipv61_6rd_dhcp" class="input" value="1" onclick="showInputfield2('ipv61_6rd_dhcp', this.value);" <% nvram_match("ipv61_6rd_dhcp", "1","checked"); %>><#WLANConfig11b_WirelessCtrl_button1name#>
						<input type="radio" name="ipv61_6rd_dhcp" class="input" value="0" onclick="showInputfield2('ipv61_6rd_dhcp', this.value);" <% nvram_match("ipv61_6rd_dhcp", "0","checked"); %>><#btn_disable#>
		     		</td>
	     		</tr>
				<tr style="display:none;">
					<th><#ipv6_6rd_Prefix#></th>
		     		<td>
						<input type="text" maxlength="39" class="input_32_table" name="ipv61_6rd_prefix" value='<% nvram_get("ipv61_6rd_prefix"); %>' autocorrect="off" autocapitalize="off">
		     		</td>
	     		</tr>		     	
				<tr style="display:none;">
					<th><#IPv6_Prefix_Length#></th>
		     		<td>
							<input type="text" maxlength="3" class="input_3_table" name="ipv61_6rd_prefixlen" value="<% nvram_get("ipv61_6rd_prefixlen"); %>" autocorrect="off" autocapitalize="off">
		     		</td>
	     		</tr>
				<tr style="display:none;">
					<th><#ipv6_6rd_router#></th>
		     		<td>
							<input type="text" maxlength="15" class="input_15_table" name="ipv61_6rd_router" value='<% nvram_get("ipv61_6rd_router"); %>' autocorrect="off" autocapitalize="off">
		     		</td>
	     		</tr>
				<tr style="display:none;">
					<th><#ipv6_6rd_ip4size#></th>
		     		<td>
							<input type="text" maxlength="2" class="input_3_table" name="ipv61_6rd_ip4size" value='<% nvram_get("ipv61_6rd_ip4size"); %>' autocorrect="off" autocapitalize="off">
		     		</td>
	     		</tr>
				<tr style="display:none;">
					<th><#ipv6_client_ip#></th>
		     		<td>
						<input type="text" maxlength="39" class="input_32_table" name="ipv61_tun_addr" value='<% nvram_get("ipv61_tun_addr"); %>' autocorrect="off" autocapitalize="off">
		     		</td>
	     		</tr>		     	
				<tr style="display:none;">
					<th><#IPv6_Prefix_Length#></th>
		     		<td>
							<input type="text" maxlength="3" class="input_3_table" name="ipv61_tun_addrlen" value="<% nvram_get("ipv61_tun_addrlen"); %>" autocorrect="off" autocapitalize="off">
		     		</td>
	     		</tr>
				<tr style="display:none;">
					<th><#ipv6_peer_addr#></th>
		     		<td>
						<input type="text" maxlength="39" class="input_32_table" name="ipv61_tun_peer" value='<% nvram_get("ipv61_tun_peer"); %>' autocorrect="off" autocapitalize="off">
		     		</td>
	     		</tr>
				<tr style="display:none;">
					<th><#tunnel_MTU#></th>
		     		<td>
						<input type="text" maxlength="4" class="input_6_table" name="ipv61_tun_mtu" value='<% nvram_get("ipv61_tun_mtu"); %>' autocorrect="off" autocapitalize="off">
		     		</td>
	     		</tr>
				<tr style="display:none;">
					<th><#tunnel_TTL#></th>
		     		<td>
						<input type="text" maxlength="3" class="input_6_table" name="ipv61_tun_ttl" value='<% nvram_get("ipv61_tun_ttl"); %>' autocorrect="off" autocapitalize="off">
		     		</td>
	     		</tr>
			</table>	
			<!--=====================================basic_config end===================================-->
			
			<!--=====================================IPv6 WAN setting start=============================-->
			<table id="ipv61_wan_setting" style="margin-top:8px;" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
				<thead>
					<tr>
						<td colspan="2"><#IPv6_WAN_Setting#></td>
				  	</tr>
				</thead>
				<tr>
					<th><#IPv6_wan_addr#></th>
	     			<td>
					  	<input type="text" maxlength="39" class="input_32_table" name="ipv61_ipaddr" value='<% nvram_get("ipv61_ipaddr"); %>' autocorrect="off" autocapitalize="off">
					</td>
				</tr>
				<tr>
					<th><#IPv6_wan_Prefix_len#></th>
					<td>
								<input type="text" maxlength="3" class="input_3_table" name="ipv61_prefix_len_wan" value="<% nvram_get("ipv61_prefix_len_wan"); %>" autocorrect="off" autocapitalize="off">
	     			</td>
	     		</tr>
				<tr>
					<th><#IPv6_wan_gateway#></th>
	     			<td>
					  	<input type="text" maxlength="39" class="input_32_table" name="ipv61_gateway" value='<% nvram_get("ipv61_gateway"); %>' autocorrect="off" autocapitalize="off">
					</td>
				</tr>
			</table>
			<!--=====================================IPv6 WAN setting  end =============================-->
			
			<!--=====================================IPv6 LAN setting start=============================-->
			<table id="ipv61_lan_setting" style="margin-top:8px;" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
				  <thead>
				  <tr>
						<td colspan="2"><#IPv6_LAN_Setting#></td>
				  </tr>
				  </thead>
				  
					<tr style="display:none;">
						<th><#ipv6_lan_addr#></th>	<!-- for Native w/o DHCP_PD-->
		     		<td>
						  <input type="text" maxlength="39" class="input_32_table" name="ipv61_rtr_addr" value='<% nvram_get("ipv61_rtr_addr"); %>' onBlur="if(document.form.ipv61_autoconf_type[1].checked){showInputfield2('ipv61_autoconf_type', '1');}else{showInputfield2('ipv61_autoconf_type', '0');}" autocorrect="off" autocapitalize="off">
		     		</td>
		     	</tr>
					<tr id="ipv61_ipaddr_r">
						<th><#ipv6_lan_addr#></th>	<!-- for other ipv6 proto -->
		     		<td>
						  <div id="ipv61_ipaddr_span" name="ipv61_ipaddr_span" style="color:#FFFFFF;margin-left:8px;"></div>
		     		</td>
		     	</tr>				  
				  
					<tr>
						<th><#Prefix_lan_Length#></th>
						<td>
								<input type="text" maxlength="3" class="input_3_table" name="ipv61_prefix_length" value="<% nvram_get("ipv61_prefix_length"); %>" autocorrect="off" autocapitalize="off">
		     		</td>
		     	</tr>
					<tr id="ipv61_prefix_length_r">
						<th><#Prefix_lan_Length#></th>
						<td>
						  <div id="ipv61_prefix_length_span" name="ipv61_prefix_length_span" style="color:#FFFFFF;margin-left:8px;"></div>
		     		</td>
		     	</tr>
		     			
					<tr>
						<th><#IPv6_lan_Prefix#></th>
		     		<td>
						  	<input type="text" maxlength="39" class="input_32_table" name="ipv61_prefix" value='<% nvram_get("ipv61_prefix"); %>' autocorrect="off" autocapitalize="off">
						</td>
					</tr>
					<tr id="ipv61_prefix_r">
						<th><#IPv6_lan_Prefix#></th>
		     		<td>
						  <div id="ipv61_prefix_span" name="ipv61_prefix_span" style="color:#FFFFFF;margin-left:8px;"></div>
						</td>
					</tr>		     	     	
		     	
					<tr>
		     		<th><#ipv6_auto_config#></th>	
		     		<td>
							<input type="radio" name="ipv61_autoconf_type" class="input" value="0" onclick="showInputfield2('ipv61_autoconf_type', '0');" <% nvram_match("ipv61_autoconf_type", "0","checked"); %>>Stateless
							<input type="radio" name="ipv61_autoconf_type" class="input" value="1" onclick="showInputfield2('ipv61_autoconf_type', '1');" <% nvram_match("ipv61_autoconf_type", "1","checked"); %>>Stateful
						</td>	
		    	</tr>
					<tr>
		     		<th><#LANHostConfig_MinAddress_itemname#></th>	<!-- DHCP pool start -->
		     		<td>
		     				<input type="text" maxlength="19" class="input_20_table" name="ipv61_prefix_span_for_start" style="color:#BBBBBB" readonly autocorrect="off" autocapitalize="off">		     				
		     				::
		     				<input type="text" maxlength="4" class="input_6_table" name="ipv61_dhcp_start_start" autocorrect="off" autocapitalize="off" >
		     		</td>
		    	</tr>

		     	<tr>
		     		<th><#LANHostConfig_MaxAddress_itemname#></th>	<!-- DHCP pool end -->
		     		<td>
		     			<input type="text" maxlength="19" class="input_20_table" name="ipv61_prefix_span_for_end" style="color:#BBBBBB" readonly autocorrect="off" autocapitalize="off">
		     			::
		     			<input type="text" maxlength="4" class="input_6_table" name="ipv61_dhcp_end_end" autocorrect="off" autocapitalize="off">
		     		</td>
		    	</tr>
		    	
		     	<tr>
		     		<th><#LANHostConfig_LeaseTime_itemname#></th>	
		     		<td>
		     			<input type="text" maxlength="6" class="input_6_table" name="ipv61_dhcp_lifetime" value='<% nvram_get("ipv61_dhcp_lifetime"); %>' onkeypress="return validator.isNumber(this,event)" autocorrect="off" autocapitalize="off">
		     		</td>
		    	</tr>		    	
			</table>
			<!--=====================================IPv6 LAN setting end===============================-->
			
			<!--=====================================IPv6 DNS setting start=============================-->  	
			<table id="ipv61_dns_setting" style="margin-top:8px;" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
				  <thead>
				  <tr>
						<td colspan="2"><#IPv6_DNS_Setting#></td>
				  </tr>
				  </thead>		
					<tr style="display:none;">
						<th><#IPConnection_x_DNSServerEnable_itemname#></th>
		     		<td>
								<input type="radio" name="ipv61_dnsenable" class="input" value="1" onclick="showInputfield2('ipv61_dnsenable', this.value);" <% nvram_match("ipv61_dnsenable", "1","checked"); %>><#WLANConfig11b_WirelessCtrl_button1name#>
								<input type="radio" name="ipv61_dnsenable" class="input" value="0" onclick="showInputfield2('ipv61_dnsenable', this.value);" <% nvram_match("ipv61_dnsenable", "0","checked"); %>><#btn_disable#>
								<div id="yadns_hint" style="display:none;"></div>
		     		</td>
		     	</tr>
					<tr style="display:none;">
						<th><#ipv6_dns_serv#> 1</th>
		     		<td>
						  <input type="text" maxlength="39" class="input_32_table" name="ipv61_dns1" value='<% nvram_get("ipv61_dns1"); %>' autocorrect="off" autocapitalize="off">
		     		</td>
		     	</tr>
					<tr style="display:none;">
						<th><#ipv6_dns_serv#> 2</th>
		     		<td>
						  <input type="text" maxlength="39" class="input_32_table" name="ipv61_dns2" value='<% nvram_get("ipv61_dns2"); %>' autocorrect="off" autocapitalize="off">
		     		</td>
		     	</tr>
					<tr style="display:none;">
						<th><#ipv6_dns_serv#> 3</th>
		     		<td>
						  <input type="text" maxlength="39" class="input_32_table" name="ipv61_dns3" value='<% nvram_get("ipv61_dns3"); %>' autocorrect="off" autocapitalize="off">
		     		</td>
		     	</tr>		     	
			</table>
			<!--=====================================IPv6 DNS setting end==========================-->
			
			<!--=====================================Auto Config start=============================-->  
			<table id="auto_config" style="margin-top:8px;display:none;" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
				  <thead>
				  <tr>
						<td colspan="2"><#ipv6_auto_config#></td>
				  </tr>
				  </thead>		
				  <tr>
						<th><#Enable_Router_AD#></th>
		     			<td>
							<input type="radio" name="_ipv61_radvd" class="input" value="1" <% nvram_match("ipv61_radvd", "1","checked"); %>><#WLANConfig11b_WirelessCtrl_button1name#>
							<input type="radio" name="_ipv61_radvd" class="input" value="0" <% nvram_match("ipv61_radvd", "0","checked"); %>><#btn_disable#>
		     			</td>
		     	  </tr>
			</table>
			<!--====================================Auto Config end===============================-->  	
				
				<div class="apply_gen">
					<input class="button_gen" onclick="applyRule()" type="button" value="<#CTL_apply#>"/>
				</div>
			</td>
		</tr>
		</tbody>	
	  </table>
		</td>
	</tr>
	</table>				
	<!--===================================End of Main Content===========================================-->
	</td>
  <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
</form>					

<form method="post" name="ipv6_form" action="/apply.cgi" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_IPv61_Content.asp">
<input type="hidden" name="next_page" value="Advanced_IPv61_Content.asp">
<input type="hidden" name="action_mode" value="change_ipv6_unit">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="ipv6_unit" value="">
</form>

<div id="footer"></div>
</body>
</html>
