﻿<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - VPN Status</title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">

<style>
	.statcell { width:25% !important; text-align:left !important; }
	.wgsheader:first-letter { text-transform: capitalize; }
</style>

<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/httpApi.js"></script>
<script>
wan_route_x = '<% nvram_get("wan_route_x"); %>';
wan_nat_x = '<% nvram_get("wan_nat_x"); %>';
wan_proto = '<% nvram_get("wan_proto"); %>';

var overlib_str0 = new Array();	//Viz add 2013.04 for record longer VPN client username/pwd
var overlib_str1 = new Array();	//Viz add 2013.04 for record longer VPN client username/pwd
vpnc_clientlist_array = decodeURIComponent('<% nvram_char_to_ascii("","vpnc_clientlist"); %>');
var wgs_object = {};

function initial(){
	show_menu();

	setTimeout("refresh_vpn_data()",1000);

	if (ipsec_srv_support)
		setTimeout("refresh_ipsec_data()",1200);

	if (wireguard_support)
		build_wgsc_array();
}


function refresh_vpn_data(){
	$.ajax({
		url: 'ajax_vpn_status.asp',
		dataType: 'script',
		error: function(xhr){
			refresh_vpn_data();
		},
		success: function(response){
			document.getElementById("pageloading").style.display = "none";
			display_vpn_data();
			if (wireguard_support) {
				display_wg_data();
		        }
		}
	});
}


var state_srv_run = " - Running";
var state_srv_stop = " - <span style=\"background-color: transparent; color: white;\">Stopped</span>";
var state_clnt_ced = " - Connected";
var state_clnt_cing = " - Connecting...";
var state_clnt_err = " - Error connecting";
var state_clnt_disc = " - <span style=\"background-color: transparent; color: white;\">Stopped</span>";

function display_vpn_data(){
	var state_desc, tmp;

	if (openvpnd_support) {
		if (server1pid > 0) {
			document.getElementById("ovpnserver1_Block_Running").innerHTML = state_srv_run;
			showhide("ovpnserver1", 1);
		} else {
			showhide("ovpnserver1", 0);
		}

		if (server2pid > 0) {
			document.getElementById("ovpnserver2_Block_Running").innerHTML = state_srv_run;
			showhide("ovpnserver2", 1);
		} else {
			showhide("ovpnserver2", 0);
		}

		for (var unit = 1; unit < 6; unit++) {
			switch (unit) {
				case 1:
					client_state = vpnc_state_t1;
					client_errno = vpnc_errno_t1;
					tmp = "<% nvram_get("vpn_client1_addr"); %>";
					client_server = " ("+ tmp.shorter(42) +
					                " <% nvram_get("vpn_client1_proto"); %>" +
					                ":<% nvram_get("vpn_client1_port"); %>)";
					client_desc = "<span style=\"background-color: transparent; color: white;\"><% nvram_get("vpn_client1_desc"); %></span>";
					break;
				case 2:
					client_state = vpnc_state_t2;
					client_errno = vpnc_errno_t2;
					tmp = "<% nvram_get("vpn_client2_addr"); %>";
					client_server = " ("+ tmp.shorter(42) +
					                " <% nvram_get("vpn_client2_proto"); %>" +
					                ":<% nvram_get("vpn_client2_port"); %>)";
					client_desc = "<span style=\"background-color: transparent; color: white;\"><% nvram_get("vpn_client2_desc"); %></span>";
					break;
				case 3:
					client_state = vpnc_state_t3;
					client_errno = vpnc_errno_t3;
					tmp = "<% nvram_get("vpn_client3_addr"); %>";
					client_server = " ("+ tmp.shorter(42) +
					                " <% nvram_get("vpn_client3_proto"); %>" +
					                ":<% nvram_get("vpn_client3_port"); %>)";
					client_desc = "<span style=\"background-color: transparent; color: white;\"><% nvram_get("vpn_client3_desc"); %></span>";
					break;
				case 4:
					client_state = vpnc_state_t4;
					client_errno = vpnc_errno_t4;
					tmp = "<% nvram_get("vpn_client4_addr"); %>";
					client_server = " ("+ tmp.shorter(42) +
					                " <% nvram_get("vpn_client4_proto"); %>" +
					                ":<% nvram_get("vpn_client4_port"); %>)";
					client_desc = "<span style=\"background-color: transparent; color: white;\"><% nvram_get("vpn_client4_desc"); %></span>";
					break;
				case 5:
					client_state = vpnc_state_t5;
					client_errno = vpnc_errno_t5;
					tmp = "<% nvram_get("vpn_client5_addr"); %>";
					client_server = " ("+ tmp.shorter(42) +
					                " <% nvram_get("vpn_client5_proto"); %>" +
					                ":<% nvram_get("vpn_client5_port"); %>)";
					client_desc = "<span style=\"background-color: transparent; color: white;\"><% nvram_get("vpn_client5_desc"); %></span>";
					break;
			}

			switch (client_state) {
				case "0":
					document.getElementById("ovpnclient"+unit+"_Block_Running").innerHTML = client_desc + state_clnt_disc;
					showhide("ovpnclient"+unit, 0);
					break;
				case "1":
					document.getElementById("ovpnclient"+unit+"_Block_Running").innerHTML = client_desc + state_clnt_cing + client_server;
					showhide("ovpnclient"+unit, 1);
					break;
				case "2":
					document.getElementById("ovpnclient"+unit+"_Block_Running").innerHTML = client_desc + state_clnt_ced + client_server;
					showhide("ovpnclient"+unit, 1);
					break;
				case "-1":
					code = state_clnt_err;
					if (client_errno == 1 || client_errno == 2 || client_errno == 3)
						code += " - <#vpn_openvpn_conflict#>";
					else if(client_errno == 4 || client_errno == 5 || client_errno == 6)
						code += " - <#qis_fail_desc1#>";
					document.getElementById("ovpnclient"+unit+"_Block_Running").innerHTML = client_desc + code;
					showhide("ovpnclient"+unit, 1);
					break;
			}
		}

		parseOVPNStatus(vpn_server1_status, "ovpnserver1_Block", "", "");
		parseOVPNStatus(vpn_server2_status, "ovpnserver2_Block", "", "");
		parseOVPNStatus(vpn_client1_status, "ovpnclient1_Block", vpn_client1_ip, vpn_client1_rip);
		parseOVPNStatus(vpn_client2_status, "ovpnclient2_Block", vpn_client2_ip, vpn_client2_rip);
		parseOVPNStatus(vpn_client3_status, "ovpnclient3_Block", vpn_client3_ip, vpn_client3_rip);
		parseOVPNStatus(vpn_client4_status, "ovpnclient4_Block", vpn_client4_ip, vpn_client4_rip);
		parseOVPNStatus(vpn_client5_status, "ovpnclient5_Block", vpn_client5_ip, vpn_client5_rip);
	}


	if (pptpd_support) {
		if (pptpdpid > 0) {
			document.getElementById("pptp_Block_Running").innerHTML = state_srv_run;
			showhide("pptpserver", 1);
		} else {
			document.getElementById("pptp_Block_Running").innerHTML = state_srv_stop;
			showhide("pptpserver", 0);
		}
		parsePPTPClients();
	}

	if ( (vpnc_support) && (vpnc_clientlist_array != "") ) {
		showhide("vpnc", 1);
		show_vpnc_rulelist();
	} else {
		showhide("vpnc", 0);
	}

	if(ipsec_srv_support) {
		if('<% nvram_get("ipsec_server_enable"); %>' == "1") {
			document.getElementById("ipsec_srv_Block_Running").innerHTML = state_srv_run;
			showhide("ipsecsrv", 1);
		} else {
			document.getElementById("ipsec_srv_Block_Running").innerHTML = state_srv_stop;
			showhide("ipsecsrv", 0);
		}
	}
}


function applyRule(){
	showLoading();
	document.form.submit();
}


function parsePPTPClients() {
	text = document.form.status_pptp.value;

	if (text == "") {
		return;
	}

	var lines = text.split('\n');

	code = '<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable_table"><thead><tr><td colspan="4">Connected Clients</td></tr></thead><tr>';
	code += '<th style="text-align:left;">Username</th><th style="text-align:left;">Interface</th><th style="text-align:left;">Remote IP</th><th style="text-align:left;">Client IP</th>';

	for (i = 0; i < lines.length; ++i)
	{
		var done = false;

		var fields = lines[i].split(' ');
		if ( fields.length != 5 ) continue;

		code +='<tr><td style="text-align:left;">' + fields[4] + '</td><td style="text-align:left;">' + fields[1] + '</td><td style="text-align:left;">' + fields[2] + '</td><td style="text-align:left;">' + fields[3] +'</td></tr>';
	}
	code +='</table>';

	document.getElementById('pptp_Block').innerHTML = code;
}


function parseOVPNStatus(text, block, ipaddress, ripaddress){
	document.getElementById(block).innerHTML = "";
	var code = "";

	var lines = text.split('>');
	var staticStats = false;

	var routeTableEntries = new Array();
	var clientTableEntries = new Array();
	var statsTableEntries = new Array();
	var staticstatsTableEntries = new Array();

	var clientPtr = 0;
	var routePtr = 0;
	var statsPtr = 0;
	var staticstatsPtr = 0;

// Parse data
	for (i = 0; text != '' && i < lines.length; ++i)
	{
		var done = false;

		var fields = lines[i].split(',');
		if (fields.length == 0) continue;

		switch (fields[0])
		{
		case "TITLE":
			break;
                case "TIME":
                case "Updated":
			break;
		case "HEADER":
			switch (fields[1])
			{
			case "CLIENT_LIST":
				clientTableHeaders = fields.slice(2,fields.length-1);
				break;
			case "ROUTING_TABLE":
				routeTableHeaders = fields.slice(2,fields.length-1);
				break;
			default:
				break;
			}
			break;
		case "CLIENT_LIST":
			clientTableEntries[clientPtr++] = fields.slice(1,fields.length-1);
			break;
		case "ROUTING_TABLE":
			routeTableEntries[routePtr++] = fields.slice(1,fields.length-1);
			break;
//		case "GLOBAL_STATS":
//			statsTableEntries[statsPtr++] = fields.slice(1);
//			break;
		case "OpenVPN STATISTICS":
			staticStats = true;
			break;
		case "END":
			done = true;
			break;
		default:
			if (staticStats)
			{
				staticstatsTableEntries[staticstatsPtr++] = fields;
			}
			break;
		}
		if (done) break;
	}


/* Spit it out */

/*** Clients ***/

	if (clientPtr > 0) {
		code = '<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable_table"><thead><tr><td colspan="6">Clients</td></tr></thead><tr>';

// Headers
		// Common Name, Username
		code +='<th style="width:25%;text-align:left;">' + clientTableHeaders[0] + '<br><span style="color: cyan; background: transparent;">' + clientTableHeaders[8] + '</span></th>';
		// Real IPv4, merged with Virtual IPv4
		code +='<th style="width:20%;text-align:left;">' + clientTableHeaders[1] + '<br><span style="color: cyan; background: transparent;">' + clientTableHeaders[2] + '</span></th>';
		// dl/up amount
		code +='<th style="width:16%;text-align:left;">' + clientTableHeaders[4].replace("Bytes","MBytes") + '</th>';
		code +='<th style="width:16%;text-align:left;">' + clientTableHeaders[5].replace("Bytes","MBytes") + '</th>';
		// Connected since
		code +='<th style="width:23%;text-align:left;">' + clientTableHeaders[6] + '</th>';
		code += '</tr>';

// Clients
		for (i = 0; i < clientTableEntries.length; ++i)
		{
			code += '<tr>';
			// Common Name, Username
			if (clientTableEntries[i][8] == "UNDEF") {
				clientTableEntries[i][8] = "";
			}
			code += '<td style="vertical-align:top; white-space:nowrap; text-align:left;">' + clientTableEntries[i][0] + '<br><span style="color: cyan; background: transparent;">' + clientTableEntries[i][8] +'</span></td>';
			// Real IP, Virtual IP
			code += '<td style="vertical-align:top; text-align:left;">' + clientTableEntries[i][1] + '<br><span style="color: cyan; background: transparent;">' + clientTableEntries[i][2] +'</span></td>';
			// dl/up amount
			code += '<td style="vertical-align:top; text-align:left;">' + Number(clientTableEntries[i][4]/1024/1024).toFixed(2).toLocaleString() + '</td>';
			code += '<td style="vertical-align:top; text-align:left;">' + Number(clientTableEntries[i][5]/1024/1024).toFixed(2).toLocaleString() + '</td>';
			// Connected Since
			code += '<td style="vertical-align:top; text-align:left;">' + clientTableEntries[i][6] + '</td>';
			code += '</tr>';
		}
		code += '</table><br>';
		document.getElementById(block).innerHTML += code;
	}

/*** Routes ***/

	if (routePtr > 0) {
		code = '<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable_table"><thead><tr><td colspan="' + routeTableHeaders.length + '">Routes</td></tr></thead><tr>';

		// Headers
		for (i = 0; i < routeTableHeaders.length; ++i)
		{
			code +='<th style="text-align:left;">' + routeTableHeaders[i] + '</th>';
		}
		code += '</tr>';

		// Routes
		for (i = 0; i < routeTableEntries.length; ++i)
		{
			code += '<tr>';
			for (j = 0; j < routeTableEntries[i].length; ++j)
			{
				code += '<td style="white-space:nowrap; text-align:left;">' + routeTableEntries[i][j] + '</td>';
			}
			code += '</tr>';
		}
		code += '</table><br>';
		document.getElementById(block).innerHTML += code;
	}

	// Reset it, since we don't know which block we'll show next
	code = "";


/*** Stats ***/

	if (statsPtr > 0) {
		code += '<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable_table"><thead><tr><td colspan="4">Statistics</td></tr></thead>';

		for (i = 0; i < statsTableEntries.length; ++i)
		{
			if (i % 2 == 0) code += '<tr>';
			code += '<th class="statcell">' + statsTableEntries[i][0] +'</th>';
			code += '<td class="statcell">' + Number(statsTableEntries[i][1]).toLocaleString() +'</td>';
			if (i % 2 == 1) code += '</tr>';
		}
		if (i % 2 == 0) code += '</tr>';
		code += '</table>';
	}


/*** Static Stats ***/

	if (staticstatsPtr > 0) {
		code += '<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable_table"><thead><tr><td colspan="4">Statistics</td></tr></thead>';

		if (ipaddress != "") {
			code += '<tr><th class="statcell">Public IP</th>';
			code += '<td class="statcell">' + ripaddress +'</td>';
			code += '<th class="statcell">Local IP</th>';
			code += '<td class="statcell">' + ipaddress +'</td></tr>';
		}

		for (i = 0; i < staticstatsTableEntries.length; ++i)
		{
			if (i % 2 == 0) code += '<tr>';
			code += '<th class="statcell">' + staticstatsTableEntries[i][0] +'</th>';
			code += '<td class="statcell">' + Number(staticstatsTableEntries[i][1]).toLocaleString() +'</td>';
			if (i % 2 == 1) code += '</tr>';
		}
		if (i % 2 == 0) code += '</tr>';
		code += '</table>';
	}
	document.getElementById(block).innerHTML += code;
}


function parseIPSecData(profileName){
	var ikeversion = (profileName == "Host-to-Netv2" ? "IKEv2" : "IKEv1")
	var code = "<table width='100%' border='1' align='center' cellpadding='4' cellspacing='0' bordercolor='#6b8fa3' class='FormTable_table'><thead><tr><td colspan='5'>Connected Clients - " + ikeversion + "</td></tr></thead><tr>";
	code += "<th style='text-align:left;white-space:nowrap;'>Remote IP</th>";
	code += "<th style='text-align:left;white-space:nowrap;'><#statusTitle_Client#></th>";
	code += "<th style='text-align:left;white-space:nowrap;'><#Access_Time#></th>";
	code += "<th style='text-align:left;white-space:nowrap;'><#vpn_ipsec_XAUTH#> <#Permission_Management_Users#></th>";
	code += "<th style='text-align:left;white-space:nowrap;'>PSKR Auth Time</th>";

	var statusText = [[""], ["<#Connected#>"], ["<#Connecting_str#>"], ["<#Connecting_str#>"]];
	var profileName_array = ipsec_connect_status_array[profileName].split("<");
	for (i = 0; i < profileName_array.length; i += 1) {
		if(profileName_array[i] != "") {
			var profileName_col = profileName_array[i].split(">");
			code += "<tr><td style='text-align:left;'>" + profileName_col[0] + "</td>";
			code += "<td style='text-align:left;'>" + statusText[profileName_col[1]] + "</td>";
			code += "<td style='text-align:left;'>" + profileName_col[2] + "</td>";
			code += "<td style='text-align:left;'>" + profileName_col[3] + "</td>";
			code += "<td style='text-align:left;'>" + profileName_col[4] + "</td></tr>";
		}
	}

	code +='</table>';
	document.getElementById('ipsec_srv_Block').innerHTML += code;
}


function show_vpnc_rulelist(){
	if(vpnc_clientlist_array[0] == "<")
		vpnc_clientlist_array = vpnc_clientlist_array.split("<")[1];

	var vpnc_clientlist_row = vpnc_clientlist_array.split('<');
	var code = "";
	code +='<table style="margin-bottom:30px;" width="70%" border="1" align="left" cellpadding="4" cellspacing="0" class="list_table" id="vpnc_clientlist_table">';
	code +='<tr><th style="height:30px; width:20%;">Status</th>';
	code +='<th style="width:65%;"><div><#IPConnection_autofwDesc_itemname#></div></th>';
	code +='<th style="width:15%;"><div><#QIS_internet_vpn_type#></div></th></tr>';
	if(vpnc_clientlist_array == "")
		code +='<tr><td style="color:#FFCC00;" colspan="6"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		for(var i=0; i<vpnc_clientlist_row.length; i++){
			overlib_str0[i] = "";
			overlib_str1[i] = "";
			code +='<tr id="row'+i+'">';

			var vpnc_clientlist_col = vpnc_clientlist_row[i].split('>');

			if(vpnc_clientlist_col[1] == document.form.vpnc_proto.value.toUpperCase() &&
				vpnc_clientlist_col[2] == document.form.vpnc_heartbeat_x.value &&
				vpnc_clientlist_col[3] == document.form.vpnc_pppoe_username.value)
			{
				if(vpnc_state_t == 0) // initial
					code +='<td><img src="/images/InternetScan.gif"></td>';
				else if(vpnc_state_t == 1) // disconnect
					code +='<td style="color: red;">Error!</td>';
				else // connected
					code +='<td><span>Connected<span></td>';
			}
			else
				code +='<td>Disconnected</td>';

			for(var j=0; j<vpnc_clientlist_col.length; j++){
				if(j == 0){
					if(vpnc_clientlist_col[0].length >32){
						overlib_str0[i] += vpnc_clientlist_col[0];
						vpnc_clientlist_col[0]=vpnc_clientlist_col[0].substring(0, 30)+"...";
						code +='<td style="text-align: left;"title="'+overlib_str0[i]+'">'+ vpnc_clientlist_col[0] +'</td>';
					}else{
						code +='<td style="text-align: left;">'+ vpnc_clientlist_col[0] +'</td>';
					}
				}
				else if(j == 1){
					code += '<td>'+ vpnc_clientlist_col[1] +'</td>';
				}
			}
		}
	}
	code +='</table>';

	document.getElementById("vpnc_clientlist_Block").innerHTML = code;
}

function refresh_ipsec_data() {
	$.ajax({
		url: '/ajax_ipsec.asp',
		dataType: 'script',
		timeout: 1500,
		success: function() {
			document.getElementById('ipsec_srv_Block').innerHTML = "";
			ipsec_connect_status_array = [];
			for(var i = 0; i < ipsec_connect_status.length; i += 1) {
				ipsec_connect_status_array[ipsec_connect_status[i][0]] = ipsec_connect_status[i][1];
			}
			if(ipsec_connect_status_array["Host-to-Net"] != undefined) {
				var connected_count = (ipsec_connect_status_array["Host-to-Net"].split("<").length);
				if(connected_count > 0) {
					parseIPSecData("Host-to-Net");
				}
			}
			if(ipsec_connect_status_array["Host-to-Netv2"] != undefined) {
				var connected_count = (ipsec_connect_status_array["Host-to-Netv2"].split("<").length);
				if(connected_count > 0) {
					parseIPSecData("Host-to-Netv2");
				}
			}
		}
	});

	setTimeout("refresh_ipsec_data()", 2000)
}


function display_wg_data(){
	if ("<% nvram_get("wgs_enable"); %>" == "1") {
		document.getElementById("wgserver_Block_Running").innerHTML = state_srv_run;
		parseWGSStatus("wgserver_Block");
		showhide("wgserver", 1);
	} else {
		document.getElementById("wgserver_Block_Running").innerHTML = state_srv_stop;
		showhide("wgserver", 0);
	}

	for (var unit = 1; unit < 6; unit++) {
		switch (unit) {
			case 1:
				client_state = "<% sysinfo("wgcstatus.1"); %>";
				tmp = "<% nvram_get("wgc1_ep_addr"); %>";
				client_server = " ("+ tmp.shorter(42) +
				                " - port <% nvram_get("wgc1_ep_port"); %>)";
				desc = "<% nvram_get("wgc1_desc"); %>";
				if (desc == "")
					desc = "Client " + unit;
				client_desc = "<span style=\"background-color: transparent; color: white;\">" + desc + "</span>";
				local_ip = wgc1_ip;
				remote_ip = wgc1_rip;
				break;
			case 2:
				client_state = "<% sysinfo("wgcstatus.2"); %>";
				tmp = "<% nvram_get("wgc2_ep_addr"); %>";
				client_server = " ("+ tmp.shorter(42) +
				                " - port <% nvram_get("wgc2_ep_port"); %>)";
				desc = "<% nvram_get("wgc2_desc"); %>";
				if (desc == "")
					desc = "Client " + unit;
				client_desc = "<span style=\"background-color: transparent; color: white;\">" + desc + "</span>";
				local_ip = wgc2_ip;
				remote_ip = wgc2_rip;
				break;
			case 3:
				client_state = "<% sysinfo("wgcstatus.3"); %>";
				tmp = "<% nvram_get("wgc3_ep_addr"); %>";
				client_server = " ("+ tmp.shorter(42) +
				                " - port <% nvram_get("wgc3_ep_port"); %>)";
				desc = "<% nvram_get("wgc3_desc"); %>";
				if (desc == "")
					desc = "Client " + unit;
                                client_desc = "<span style=\"background-color: transparent; color: white;\">" + desc + "</span>";
				local_ip = wgc3_ip;
				remote_ip = wgc3_rip;
				break;
			case 4:
				client_state = "<% sysinfo("wgcstatus.4"); %>";
				tmp = "<% nvram_get("wgc4_ep_addr"); %>";
				client_server = " ("+ tmp.shorter(42) +
				                " - port <% nvram_get("wgc4_ep_port"); %>)";
				desc = "<% nvram_get("wgc4_desc"); %>";
				if (desc == "")
					desc = "Client " + unit;
                                client_desc = "<span style=\"background-color: transparent; color: white;\">" + desc + "</span>";
				local_ip = wgc4_ip;
				remote_ip = wgc4_rip;
				break;
			case 5:
				client_state = "<% sysinfo("wgcstatus.5"); %>";
				tmp = "<% nvram_get("wgc5_ep_addr"); %>";
				client_server = " ("+ tmp.shorter(42) +
				                " - port <% nvram_get("wgc5_ep_port"); %>)";
				desc = "<% nvram_get("wgc5_desc"); %>";
				if (desc == "")
					desc = "Client " + unit;
                                client_desc = "<span style=\"background-color: transparent; color: white;\">" + desc + "</span>";
				local_ip = wgc5_ip;
				remote_ip = wgc5_rip;
				break;
		}

		switch (client_state) {
			case "0":
				document.getElementById("wgclient"+unit+"_Block_Running").innerHTML = client_desc + state_clnt_disc;
				showhide("wgclient"+unit, 0);
				break;
			case "1":
				document.getElementById("wgclient"+unit+"_Block_Running").innerHTML = client_desc + state_clnt_ced + client_server;
				get_wgc_data(unit, "wgclient"+unit+"_Block", local_ip, remote_ip);
				showhide("wgclient"+unit, 1);
				break;
		}
	}

	setTimeout("display_wg_data()",2000);
}


function get_wgc_data(_unit, _block, _local_ip, _remote_ip) {
	$.ajax({
		url: '/appGet.cgi?hook=nvram_dump(\"wgc.log\",\"' + _unit +'\")',
		dataType: 'text',
		error: function(xhr){
			setTimeout("get_wgc_data(_unit, _block);", 1000);
		},
		success: function(response){
			var got_peer = 0;
			var code = "<table width='100%' border='1' align='center' cellpadding='4' cellspacing='0' bordercolor='#6b8fa3' class='FormTable_table'><thead><tr><td colspan='2'>Client Status</tr></thead>";
			code += "<tr><th class='wgsheader' style='text-align:left;'>Local IP</td><td style='text-align:left;'>" + _local_ip + "</td></tr>";
			code += "<tr><th class='wgsheader' style='text-align:left;'>Public IP</td><td style='text-align:left;'>" + _remote_ip + "</td></tr>";
			data = response.toString().slice(23).split("\n");
			for (i = 0; i < data.length; ++i) {
				var fields = data[i].split(/:(.*)/s);
				if (fields.length < 2) continue;
				if (fields[0].trim() == "preshared key") continue;

				if (fields[0] == "peer") {
					got_peer = 1;
				}
				if (got_peer == 1) {
					code += "<tr><th class='wgsheader' style='text-align:left;'>" + fields[0] + "</td><td style='text-align:left;'>" + fields[1] + "</td></tr>";
				}
			}
			code += "</table>";
			document.getElementById(_block).innerHTML = code;
		}
	});
}

function build_wgsc_array() {
	var wgsc_settings = httpApi.nvramGet(["wgs1_c1_name", "wgs1_c1_pub",
	                                      "wgs1_c2_name", "wgs1_c2_pub",
	                                      "wgs1_c3_name", "wgs1_c3_pub",
	                                      "wgs1_c4_name", "wgs1_c4_pub",
	                                      "wgs1_c5_name", "wgs1_c5_pub",
	                                      "wgs1_c6_name", "wgs1_c6_pub",
	                                      "wgs1_c7_name", "wgs1_c7_pub",
	                                      "wgs1_c8_name", "wgs1_c8_pub",
	                                      "wgs1_c9_name", "wgs1_c9_pub",
	                                      "wgs1_c10_name", "wgs1_c10_pub"]);

	for (index = 1; index < 11; index++) {
		wgs_object[wgsc_settings["wgs1_c" + index + "_pub"]] = wgsc_settings["wgs1_c" + index +"_name"];
	}
}

function parseWGSStatus(_block) {
	$.ajax({
		url: '/appGet.cgi?hook=nvram_dump(\"wgs.log\",\"wgs.sh\")',
		dataType: 'text',
		error: function(xhr){
			setTimeout("parseWGSStatus(_block);", 1000);
		},
		success: function(response){
			var code = "<table width='100%' border='1' align='center' cellpadding='4' cellspacing='0' bordercolor='#6b8fa3' class='FormTable_table'><thead><tr><td colspan='2'>Peers</tr></thead>";
			var active_peer = 0, have_peers = 0;
			var data = response.toString().slice(23,-4).split("\n");
			for (var i = 0; i < data.length; ++i) {
				var fields = data[i].split(/:(.*)/s);
				if (fields.length < 2) continue;
				if (fields[0] == "peer") {
					if (is_wgsc_connected(fields[1].trim())) {
						active_peer = 1;
						have_peers = 1;
						code += "<tr><th colspan='2' style='text-align:left;color:#FFCC00;text-decoration:bold;'>Peer: " + wgs_object[fields[1].trim()] + "</th></tr>";
						code += "<tr><th class='wgsheader' style='text-align:left;'>Public ID</td><td style='text-align:left;'>" + fields[1] + "</td></tr>";
					} else {
						active_peer = 0;
					}
				} else if (active_peer == 1) {
					code += "<tr><th class='wgsheader' style='text-align:left;'>" + fields[0] + "</td><td style='text-align:left;'>" + fields[1] + "</td></tr>";
				}
			}
			code += "</table>";
			if (have_peers == 1)
				document.getElementById(_block).innerHTML = code;
		}
	});
}


function is_wgsc_connected(_pubkey) {
	var state = 0;

	var get_wgsc_status = httpApi.hookGet("get_wgsc_status", true);
	if(get_wgsc_status.client_status != undefined){
		$.each(get_wgsc_status.client_status, function(index, value){
			if (value.pub == _pubkey) {
				if(value.status == "1") {
					state = 1;
					return;
				} else {
					state = 0;
					return;
				}
			}
		});
	}
	return state;
}

</script>
</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_VPNStatus.asp">
<input type="hidden" name="next_page" value="Advanced_VPNStatus.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="0">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="SystemCmd" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="status_pptp" value="<% nvram_dump("pptp_connected",""); %>">
<input type="hidden" name="vpnc_proto" value="<% nvram_get("vpnc_proto"); %>">
<input type="hidden" name="vpnc_pppoe_username" value="<% nvram_get("vpnc_pppoe_username"); %>">
<input type="hidden" name="vpnc_heartbeat_x" value="<% nvram_get("vpnc_heartbeat_x"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
    <td width="17">&nbsp;</td>
    <td valign="top" width="202">
      <div id="mainMenu"></div>
      <div id="subMenu"></div></td>
    <td valign="top">
        <div id="tabMenu" class="submenuBlock"></div>

      <!--===================================Beginning of Main Content===========================================-->
      <table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
        <tr>
          <td valign="top">
            <table width="760px" border="0" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTitle" id="FormTitle">
                <tbody>
                <tr bgcolor="#4D595D">
                <td valign="top">
                <div>&nbsp;</div>
                <div class="formfonttitle">VPN - Status</div>
				<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
				<div id="pageloading" style="padding-left:20px; padding-top:20px; font-size:150%;" style="padding-left:10px;" class="hint-color">Loading...<img src="/images/InternetScan.gif"></div>
				<table width="100%" style="margin-bottom:20px;display:none;" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" id="pptpserver" class="FormTable">
					<thead>
						<tr>
							<td>PPTP VPN Server<span id="pptp_Block_Running" style="background: transparent;"></span></td>
						</tr>
					</thead>
					<tr>
						<td style="border: none;">
							<div id="pptp_Block"></div>
						</td>
					</tr>

				</table>
				<table width="100%" id="ovpnserver1" style="margin-bottom:20px;display:none;" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
						<tr>
							<td>OpenVPN Server 1<span id="ovpnserver1_Block_Running" style="background: transparent;"></span></td>
						</tr>
					</thead>
					<tr>
						<td style="border: none;">
							<div id="ovpnserver1_Block"></div>
						</td>
					</tr>

				</table>
				<table width="100%" id="ovpnserver2" style="margin-bottom:20px;display:none;" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
						<tr>
							<td>OpenVPN Server 2<span id="ovpnserver2_Block_Running" style="background: transparent;"></span></td>
						</tr>
					</thead>
					<tr>
						<td style="border: none;">
							<div id="ovpnserver2_Block"></div>
						</td>
					</tr>

				</table>
				<table width="100%" id="ipsecsrv" style="margin-bottom:20px;display:none;" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
						<tr>
                                                        <td>IPSec Server<span id="ipsec_srv_Block_Running" style="background: transparent;"></span></td>
						</tr>
					</thead>
					<tr>
						<td style="border: none;">
							<div id="ipsec_srv_Block"></div>
						</td>
					</tr>
                                </table>
				<table width="100%" id="wgserver" style="margin-bottom:20px;display:none;" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
						<tr>
							<td>WireGuard Server <span id="wgserver_Block_Running" style="background: transparent;"></span></td>
						</tr>
					</thead>
					<tr>
						<td style="border: none;">
							<div id="wgserver_Block"></div>
						</td>
					</tr>
				</table>
				<table width="100%" id="ovpnclient1" style="margin-bottom:20px;display:none;" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
						<tr>
							<td>OpenVPN <span id="ovpnclient1_Block_Running" style="background: transparent;"></span></td>
						</tr>
					</thead>
					<tr>
						<td style="border: none;">
							<div id="ovpnclient1_Block"></div>
						</td>
					</tr>

				</table>
				<table width="100%" id="ovpnclient2" style="margin-bottom:20px;display:none;" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
						<tr>
							<td>OpenVPN <span id="ovpnclient2_Block_Running" style="background: transparent;"></span></td>
						</tr>
					</thead>
					<tr>
						<td style="border: none;">
							<div id="ovpnclient2_Block"></div>
						</td>
					</tr>

				</table>
				<table width="100%" id="ovpnclient3" style="margin-bottom:20px;display:none;" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
						<tr>
							<td>OpenVPN <span id="ovpnclient3_Block_Running" style="background: transparent;"></span></td>
						</tr>
					</thead>
					<tr>
						<td style="border: none;">
							<div id="ovpnclient3_Block"></div>
						</td>
					</tr>

				</table>
				<table width="100%" id="ovpnclient4" style="margin-bottom:20px;display:none;" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
						<tr>
							<td>OpenVPN <span id="ovpnclient4_Block_Running" style="background: transparent;"></span></td>
						</tr>
					</thead>
					<tr>
						<td style="border: none;">
							<div id="ovpnclient4_Block"></div>
						</td>
					</tr>

				</table>
				<table width="100%" id="ovpnclient5" style="margin-bottom:20px;display:none;" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
					<thead>
						<tr>
							<td>OpenVPN <span id="ovpnclient5_Block_Running" style="background: transparent;"></span></td>
						</tr>
					</thead>
					<tr>
						<td style="border: none;">
							<div id="ovpnclient5_Block"></div>
						</td>
					</tr>

				</table>
				<table width="100%" id="vpnc" style="margin-bottom:20px;display:none;" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
						<tr>
							<td>PPTP/L2TP Clients<span id="vpnc_Block_Running" style="background: transparent;"></span></td>
						</tr>
					</thead>
					<tr>
						<td style="border: none;">
							<div id="vpnc_clientlist_Block"></div>
						</td>
					</tr>

				</table>

				<table width="100%" id="wgclient1" style="margin-bottom:20px;display:none;" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
						<tr>
							<td>WireGuard <span id="wgclient1_Block_Running" style="background: transparent;"></span></td>
						</tr>
					</thead>
					<tr>
						<td style="border: none;">
							<div id="wgclient1_Block"></div>
						</td>
					</tr>

				</table>
				<table width="100%" id="wgclient2" style="margin-bottom:20px;display:none;" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
						<tr>
							<td>WireGuard <span id="wgclient2_Block_Running" style="background: transparent;"></span></td>
						</tr>
					</thead>
					<tr>
						<td style="border: none;">
							<div id="wgclient2_Block"></div>
						</td>
					</tr>
				</table>
				<table width="100%" id="wgclient3" style="margin-bottom:20px;display:none;" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
						<tr>
							<td>WireGuard <span id="wgclient3_Block_Running" style="background: transparent;"></span></td>
						</tr>
					</thead>
					<tr>
						<td style="border: none;">
							<div id="wgclient3_Block"></div>
						</td>
					</tr>

				</table>
				<table width="100%" id="wgclient4" style="margin-bottom:20px;display:none;" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
						<tr>
							<td>WireGuard <span id="wgclient4_Block_Running" style="background: transparent;"></span></td>
						</tr>
					</thead>
					<tr>
						<td style="border: none;">
							<div id="wgclient4_Block"></div>
						</td>
					</tr>

				</table>
				<table width="100%" id="wgclient5" style="margin-bottom:20px;display:none;" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
					<thead>
						<tr>
							<td>WireGuard <span id="wgclient5_Block_Running" style="background: transparent;"></span></td>
						</tr>
					</thead>
					<tr>
						<td style="border: none;">
							<div id="wgclient5_Block"></div>
						</td>
					</tr>

				</table>



				<div class="apply_gen">
					<input name="button" type="button" class="button_gen" onclick="applyRule();" value="<#CTL_refresh#>"/>
				</div>
			  </td></tr>
	        </tbody>
            </table>
            </form>
            </td>

       </tr>
      </table>
      <!--===================================Ending of Main Content===========================================-->
    </td>
    <td width="10" align="center" valign="top">&nbsp;</td>
  </tr>
</table>
<div id="footer"></div>
</body>
</html>


