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
<link href="/images/map-iconRouter_iphone.png" rel="apple-touch-icon" />
<title><#Web_Title#> - <#menu1#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="NM_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<link rel="stylesheet" type="text/css" href="/device-map/device-map.css">
<style type="text/css">
.contentM_qis{
	position:absolute;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius: 5px;
	z-index:200;
	background-color:#2B373B;
	display:block;
	margin-left: 23%;
	top: 15%;
	width:495px;
	height:auto;
	box-shadow: 3px 3px 10px #000;
	display: none;
	padding: 10px;
}
.contentM_usericon{
	position:absolute;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius: 5px;
	z-index:20000;
	background-color:#2B373B;
	display:block;
	margin-left: 23%;
	margin-top: 20px;
	width:650px;
	height:250px;
	box-shadow: 3px 3px 10px #000;
	display: none;
}
.contentM_qis_manual{
	position:absolute;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius: 5px;
	z-index:200;
	background-color:#2B373B;
	margin-left: -30px;
	margin-left: -100px \9; 
	margin-top:-400px;
	width:740px;
	box-shadow: 3px 3px 10px #000;
}
.imgClientIcon{
	position: relative; 
	width: 52px;
	height: 52px;
	-webkit-border-radius: 10px;
	-moz-border-radius: 10px;
	border-radius: 10px;
}
.block_all_icon{
	position: absolute;
	width: 80%;
	background-color: #f5a02b;
	height: 32px;
	border-radius: 24px;
	left: 10%;
	align-items: center;
	justify-content: center;
	top: -16px;
	cursor: pointer;
	box-shadow: 0 2px 4px 0 rgba(0,0,0,0.20), 0 1px 4px 0 rgba(60,60,60,0.30);
	font-weight: bolder;
	display: none;
}
.block_all_icon:hover{
	background-color: #FBB655;
}
.block_all_icon > div{
	position: relative;
	margin-left: 8px;
}
.block_all_icon > div:before{
	content: "";
	position: absolute;
	left: -22px;
	top: -2px;
	width: 18px;
	height: 18px;
	background-size: contain;
	background-repeat: no-repeat;
	background-image: url("data:image/jpeg;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAACXBIWXMAAAsTAAALEwEAmpwYAAAF92lUWHRYTUw6Y29tLmFkb2JlLnhtcAAAAAAAPD94cGFja2V0IGJlZ2luPSLvu78iIGlkPSJXNU0wTXBDZWhpSHpyZVN6TlRjemtjOWQiPz4gPHg6eG1wbWV0YSB4bWxuczp4PSJhZG9iZTpuczptZXRhLyIgeDp4bXB0az0iQWRvYmUgWE1QIENvcmUgNi4wLWMwMDYgNzkuMTY0NjQ4LCAyMDIxLzAxLzEyLTE1OjUyOjI5ICAgICAgICAiPiA8cmRmOlJERiB4bWxuczpyZGY9Imh0dHA6Ly93d3cudzMub3JnLzE5OTkvMDIvMjItcmRmLXN5bnRheC1ucyMiPiA8cmRmOkRlc2NyaXB0aW9uIHJkZjphYm91dD0iIiB4bWxuczp4bXA9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC8iIHhtbG5zOnhtcE1NPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvbW0vIiB4bWxuczpzdEV2dD0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wL3NUeXBlL1Jlc291cmNlRXZlbnQjIiB4bWxuczpkYz0iaHR0cDovL3B1cmwub3JnL2RjL2VsZW1lbnRzLzEuMS8iIHhtbG5zOnBob3Rvc2hvcD0iaHR0cDovL25zLmFkb2JlLmNvbS9waG90b3Nob3AvMS4wLyIgeG1wOkNyZWF0b3JUb29sPSJBZG9iZSBQaG90b3Nob3AgMjIuMiAoTWFjaW50b3NoKSIgeG1wOkNyZWF0ZURhdGU9IjIwMjEtMDgtMDRUMTU6MTQ6NDArMDg6MDAiIHhtcDpNZXRhZGF0YURhdGU9IjIwMjEtMDgtMDRUMTU6MTQ6NDArMDg6MDAiIHhtcDpNb2RpZnlEYXRlPSIyMDIxLTA4LTA0VDE1OjE0OjQwKzA4OjAwIiB4bXBNTTpJbnN0YW5jZUlEPSJ4bXAuaWlkOmYwOTZmYjFhLTY3ZDktNDkyZS1hNDk5LTdiZjUwMGU0NWNlMCIgeG1wTU06RG9jdW1lbnRJRD0iYWRvYmU6ZG9jaWQ6cGhvdG9zaG9wOmYzNDJlYWZiLTA3YmItMWI0NS05ZWI4LTY3ODExYjcxODg3MSIgeG1wTU06T3JpZ2luYWxEb2N1bWVudElEPSJ4bXAuZGlkOmEwODg4MWYwLTEwYWQtNGI4OC04MTFmLWIyZWQ3ZWVkZjliMCIgZGM6Zm9ybWF0PSJpbWFnZS9wbmciIHBob3Rvc2hvcDpDb2xvck1vZGU9IjMiIHBob3Rvc2hvcDpJQ0NQcm9maWxlPSJzUkdCIElFQzYxOTY2LTIuMSI+IDx4bXBNTTpIaXN0b3J5PiA8cmRmOlNlcT4gPHJkZjpsaSBzdEV2dDphY3Rpb249ImNyZWF0ZWQiIHN0RXZ0Omluc3RhbmNlSUQ9InhtcC5paWQ6YTA4ODgxZjAtMTBhZC00Yjg4LTgxMWYtYjJlZDdlZWRmOWIwIiBzdEV2dDp3aGVuPSIyMDIxLTA4LTA0VDE1OjE0OjQwKzA4OjAwIiBzdEV2dDpzb2Z0d2FyZUFnZW50PSJBZG9iZSBQaG90b3Nob3AgMjIuMiAoTWFjaW50b3NoKSIvPiA8cmRmOmxpIHN0RXZ0OmFjdGlvbj0ic2F2ZWQiIHN0RXZ0Omluc3RhbmNlSUQ9InhtcC5paWQ6ZjA5NmZiMWEtNjdkOS00OTJlLWE0OTktN2JmNTAwZTQ1Y2UwIiBzdEV2dDp3aGVuPSIyMDIxLTA4LTA0VDE1OjE0OjQwKzA4OjAwIiBzdEV2dDpzb2Z0d2FyZUFnZW50PSJBZG9iZSBQaG90b3Nob3AgMjIuMiAoTWFjaW50b3NoKSIgc3RFdnQ6Y2hhbmdlZD0iLyIvPiA8L3JkZjpTZXE+IDwveG1wTU06SGlzdG9yeT4gPC9yZGY6RGVzY3JpcHRpb24+IDwvcmRmOlJERj4gPC94OnhtcG1ldGE+IDw/eHBhY2tldCBlbmQ9InIiPz5gqlrZAAABEklEQVQ4jYWRsU4CQRRF7yCJ2hAKQ2Vpb0UDVnRE/4ZCWwsTwgf4CTRbQkGDf7MJLYSwwWKPBYNel2W41e688+7ceS/ojIAbSfeSSkl5CGF/jq02DoAF8M2f9sAM6Kcam8AnlzUBGnUG3rwCRkAPeALegNzq47rYrh0wqDAtYGlM14sLu3mXMGlHBiD7nbYNbBTTpEzeY217PHiwWD17Uq0JMDS+09Bhz0c1JCmE8CXpRVIh6VbS3Ex8A6WAaw57BnitGe6/JMBH/F87OIuHOdBKmBTAJn5PHerbu5ZAu8akMKYEHlWBJgas4rSHwHOMvbF6pqqAK2BMWiWQAc0TAzPqRmhrjWtgehL7koAOcJdifgCrvyzQ8BT7EAAAAABJRU5ErkJggg==");
}

i.icon-clone {
	-webkit-mask-repeat: no-repeat;
	mask-repeat: no-repeat;
	-webkit-mask-size: 100%;
	mask-size: 100%;
	-webkit-mask-image: url(/images/clone.svg);
	mask-image: url(/images/clone.svg);
	height: 20px;
	width: 20px;
	display: inline-block;
}

.icon-group-center {
	display: inline-flex;
	align-items: center;
}

.icon-clone{
    background: #c0c0c0!important;
}
.icon-clone:hover{
    background: #FFFFFF!important;
}

.tooltip {
  position: relative;
  display: inline-block;
}

.tooltip .tooltiptext {
  display: none;
  width: 60px;
  background-color: black;
  color: #fff;
  text-align: center;
  border-radius: 6px;
  padding: 5px;
  position: absolute;
  z-index: 1;
  bottom: 150%;
  left: 50%;
  margin-left: -40px;
}

.tooltip .tooltiptext::after {
  content: "";
  position: absolute;
  top: 100%;
  left: 50%;
  margin-left: -5px;
  border-width: 5px;
  border-style: solid;
  border-color: black transparent transparent transparent;
}
</style>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/md5.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/httpApi.js"></script>
<script language="JavaScript" type="text/javascript" src="/client_function.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" src="/disk_functions.js"></script>
<script language="JavaScript" type="text/javascript" src="/form.js"></script>
<script type="text/javascript" src="/js/asus_clientlist.js"></script>
<script>
//if(usb_support) addNewScript("/disk_functions.js");

var userIconBase64 = "NoIcon";
var userIconBase64_ori = "NoIcon";
var userUploadFlag = false;
var vendorIcon = "";
var userIconHideFlag = false;
var custom_usericon_del = "";
var firstTimeOpenBlock = false;
var ipBindingFlag = false;
var timeSchedulingFlag = false;
var blockInternetFlag = false;

if(location.pathname == "/"){
	if('<% nvram_get("x_Setting"); %>' == '0'){
		if(tmo_support){
			location.href = '/MobileQIS_Login.asp';
		}
		else{
			location.href = '/QIS_wizard.htm?flag=welcome';
		}
	}	
	else if('<% nvram_get("w_Setting"); %>' == '0' && !isSwMode("RP"))
		location.href = '/QIS_wizard.htm?flag=wireless';
}

// Live Update
var webs_state_update= '<% nvram_get("webs_state_update"); %>';
var webs_state_error= '<% nvram_get("webs_state_error"); %>';
var webs_state_info= '<% nvram_get("webs_state_info"); %>';


// WAN
<% wanlink(); %>
<% first_wanlink(); %>
<% secondary_wanlink(); %>
var wanstate = -1;
var wansbstate = -1;
var wanauxstate = -1;
var Dev3G = '<% nvram_get("d3g"); %>';
var flag = '<% get_parameter("flag"); %>';
var wan0_primary = '<% nvram_get("wan0_primary"); %>';
var wan1_primary = '<% nvram_get("wan1_primary"); %>';
var wans_dualwan_orig = '<% nvram_get("wans_dualwan"); %>';
var wans_mode = '<%nvram_get("wans_mode");%>';
var dhcp_staticlist_orig = decodeURIComponent('<% nvram_char_to_ascii("", "dhcp_staticlist"); %>').replace(/&#62/g, ">").replace(/&#60/g, "<");
var dhcp_staticlist_num = dhcp_staticlist_orig.split("<").length - 1;
var MULTIFILTER_ENABLE_orig = decodeURIComponent('<% nvram_char_to_ascii("", "MULTIFILTER_ENABLE"); %>').replace(/&#62/g, ">").replace(/&#60/g, "<");
var MULTIFILTER_MAC_orig = decodeURIComponent('<% nvram_char_to_ascii("", "MULTIFILTER_MAC"); %>').replace(/&#62/g, ">").replace(/&#60/g, "<");
var MULTIFILTER_DEVICENAME_orig = decodeURIComponent('<% nvram_char_to_ascii("", "MULTIFILTER_DEVICENAME"); %>').replace(/&#62/g, ">").replace(/&#60/g, "<");
var MULTIFILTER_MACFILTER_DAYTIME_orig = decodeURIComponent('<% nvram_char_to_ascii("", "MULTIFILTER_MACFILTER_DAYTIME"); %>').replace(/&#62/g, ">").replace(/&#60/g, "<");
if(isSupport("PC_SCHED_V3"))
	MULTIFILTER_MACFILTER_DAYTIME_orig = decodeURIComponent('<% nvram_char_to_ascii("", "MULTIFILTER_MACFILTER_DAYTIME_V2"); %>').replace(/&#62/g, ">").replace(/&#60/g, "<");
var MULTIFILTER_num = (MULTIFILTER_MAC_orig == "") ? 0 : MULTIFILTER_ENABLE_orig.split(">").length;
var wanlink_status = wanlink_statusstr();
var wanlink_ipaddr = wanlink_ipaddr();
var first_wanlink_status = first_wanlink_statusstr();
var first_wanlink_ipaddr = first_wanlink_ipaddr();
var secondary_wanlink_status = secondary_wanlink_statusstr();
var secondary_wanlink_ipaddr = secondary_wanlink_ipaddr();
var le_enable = '<% nvram_get("le_enable"); %>';
var le_state = '<% nvram_get("le_state"); %>';

if(gobi_support) {
	var dualwan_first_if = wans_dualwan_array[0];
	var dualwan_second_if = wans_dualwan_array[1];
}

var wans_flag = (wans_dualwan_orig.search("none") != -1 || !parent.dualWAN_support) ? 0 : 1;
var wan_ipv6_network_json =('<% wan_ipv6_network(); %>' != '{}')? JSON.parse('<% wan_ipv6_network(); %>'):{};

// USB function
var currentUsbPort = new Number();
var usbPorts = new Array();

// Wireless
window.onresize = function() {
	if(document.getElementById("edit_client_block") != null){
		if(document.getElementById("edit_client_block").style.display == "block") {
			cal_panel_block("edit_client_block", 0.23);
		}
	}
	if(document.getElementById("edit_usericon_block") != null){
		if(document.getElementById("edit_usericon_block").style.display == "block") {
			cal_panel_block("edit_usericon_block", 0.15);
		}
	}
	if(document.getElementById("notice_div") != null){
		if(document.getElementById("notice_div").style.display == "block" || document.getElementById("notice_div").style.display == "") {
			cal_panel_block("notice_div", 0.2);
		}
	}
} 

var orig_NM_container_height;
function initial(){
	var autodet_state = '<% nvram_get("autodet_state"); %>';
	var autodet_auxstate = '<% nvram_get("autodet_auxstate"); %>';	
	var wan_proto = '<% nvram_get("wan_proto"); %>';
	var wlc_band = '<% nvram_get("wlc_band"); %>';
	show_menu();
	var isIE6 = navigator.userAgent.search("MSIE 6") > -1;
	if(isIE6)
		alert("<#ALERT_TO_CHANGE_BROWSER#>");

	if(dualWAN_support && isSwMode("RT")){
		check_dualwan(wans_flag);
	}

	if(concurrent_pap){
		check_dualwan(pap_flag);
		document.getElementById("first_wan_title").style.display = "none";
		document.getElementById("primary_pap_concurrent").style.display = "";
		document.getElementById("second_wan_title").style.display = "none";
		document.getElementById("secondary_pap_concurrent").style.display = "";		
	}

	if(isSwMode("MB")){
		var wlc_auth_mode = '<% nvram_get("wlc_auth_mode"); %>';
		if(wlc_auth_mode == "") wlc_auth_mode = '<% nvram_get("wlc0_auth_mode"); %>';
		if(wlc_auth_mode == "") wlc_auth_mode = '<% nvram_get("wlc1_auth_mode"); %>';
		if(wlc_auth_mode == "") wlc_auth_mode = 'unknown';

		show_middle_status(wlc_auth_mode, 0);
	}
	else if(isSwMode("RP")){		
		if(wlc_band == '1'){
			var wl_auth_mode = '<% nvram_get("wl1.1_auth_mode_x"); %>';
			var wl_wep_x = '<% nvram_get("wl1.1_wep_x"); %>';
		}
		else if(wlc_band == '2'){
			var wl_auth_mode = '<% nvram_get("wl2.1_auth_mode_x"); %>';
			var wl_wep_x = '<% nvram_get("wl2.1_wep_x"); %>';
		}
		else{
			var wl_auth_mode = '<% nvram_get("wl0.1_auth_mode_x"); %>';
			var wl_wep_x = '<% nvram_get("wl0.1_wep_x"); %>';
		}

		show_middle_status(wl_auth_mode, wl_wep_x);
	}
	else{
		if(isSupport("sdn_mainfh")){
			const mainfh = get_sdn_main_fh_info();
			show_middle_status(mainfh[0]["auth"], 0);
		}
		else
			show_middle_status(document.form.wl_auth_mode_x.value, parseInt(document.form.wl_wep_x.value));
	}

	if(amesh_support && ((isSwMode("RT") || isSwMode("WISP")) || isSwMode("ap")) && ameshRouter_support) {
		var html = '<a id="clientStatusLink" href="device-map/amesh.asp" target="statusframe">';
		html += '<div id="iconAMesh" class="iconAMesh_dis" style="margin-top:20px;" onclick="clickEvent(this);"></div>';
		html += '</a>';
		html += '<div class="clients" id="ameshNumber" style="cursor:pointer;"><#AiMesh_Node#>: <span>0</span></div>';
		$("#ameshContainer").html(html);
		require(['/require/modules/amesh.js'], function(){
			if(typeof updateAMeshCount == "function"){
				updateAMeshCount();
				setInterval(updateAMeshCount, 5000);
			}
		});
	}
	else
		$("#ameshContainer").remove();

	set_default_choice();
	if(!parent.usb_support || usbPortMax == 0){
		$("#line3_img").hide();
		$("#line3_single").show();
		document.getElementById("clients_td").colSpan = "3";
		document.getElementById("clients_td").width = '350';
		document.getElementById("clientspace_td").style.display = "none";
		document.getElementById("usb_td").style.display = "none";
	}
	else{
		for(var i=0; i<usbPortMax; i++){
			var newDisk = function(){
				this.usbPath = "";
				this.deviceType = "";
			}

			var tmpDisk = new newDisk();
			tmpDisk.usbPath = i+1;

			var usb_code = "";
			usb_code += '<div id="usbPathContainer_' + tmpDisk.usbPath + '">';
			usb_code += '<div style="margin-top:20px;margin-bottom:10px;" id="deviceIcon_' + tmpDisk.usbPath + '"></div>';
			usb_code += '<div id="usb_text_' + tmpDisk.usbPath + '" class="usb_text">USB 2.0</div>';
			usb_code += '<div style="margin:10px 0px;">';
			usb_code += '<span id="deviceText_' + tmpDisk.usbPath + '"></span>';
			usb_code += '<select id="deviceOption_' + tmpDisk.usbPath + '" class="input_option" style="display:none;height:20px;width:130px;font-size:12px;"></select>';
			usb_code += '</div>';
			usb_code += '<div id="deviceDec_' + tmpDisk.usbPath + '"></div>';
			usb_code += '</div>';
			document.getElementById("usb_td").innerHTML += usb_code;

			show_USBDevice(tmpDisk);
		}
		
		require(['/require/modules/diskList.js?hash=' + Math.random().toString()], function(diskList){
	 		var usbDevicesList = diskList.list();
			for(var i=0; i<usbDevicesList.length; i++){
			  var new_option = new Option(usbDevicesList[i].deviceName, usbDevicesList[i].deviceIndex);
				document.getElementById('deviceOption_'+usbDevicesList[i].usbPath).options.add(new_option);
				document.getElementById('deviceOption_'+usbDevicesList[i].usbPath).style.display = "";

				if(typeof usbPorts[usbDevicesList[i].usbPath-1].deviceType == "undefined" || usbPorts[usbDevicesList[i].usbPath-1].deviceType == "")
					show_USBDevice(usbDevicesList[i]);
			}

			for(var usbIndex = 1; usbIndex <= usbPortMax; usbIndex += 1) {
				var usb_mount_count = document.getElementById("deviceOption_" + usbIndex).length;
				if( usb_mount_count >= 2) {
					var divUsbMountCount = document.createElement("div");
					divUsbMountCount.className = "usb_count_circle";
					divUsbMountCount.innerHTML = usb_mount_count;
					document.getElementById("deviceText_" + usbIndex).appendChild(divUsbMountCount);

					$(".usb_count_circle").mouseover(function(){
						return overlib(`${this.innerHTML} usb devices are plugged in <% nvram_get("productid"); %> through this port.`);
					});

					$(".usb_count_circle").mouseout(function(){
						nd();
					});

					document.getElementById('deviceOption_' + usbIndex).onchange = function(event) {
		 				require(['/require/modules/diskList.js?hash=' + Math.random().toString()], function(diskList){
							var _usbIndex = event.target.id.split("_")[1];
							var usbDevicesList = diskList.list();
							show_USBDevice(usbDevicesList[document.getElementById('deviceOption_' + _usbIndex).value]);
							setSelectedDiskOrder('iconUSBdisk_' + _usbIndex);

							if(usbDevicesList[document.getElementById('deviceOption_' + _usbIndex).value].deviceType == "modem")
								clickEvent(document.getElementById('iconModem_' + _usbIndex));
							else if(usbDevicesList[document.getElementById('deviceOption_' + _usbIndex).value].deviceType == "printer")
								clickEvent(document.getElementById('iconPrinter_' + _usbIndex));
							else
								clickEvent(document.getElementById('iconUSBdisk_' + _usbIndex));
						});
					}
				}
			}
		});
		
		check_usb3();
	}

	showMapWANStatus();

	if(!isSwMode("RT")){
		document.getElementById("wanIP_div").style.display = "none";
		document.getElementById("ddnsHostName_div").style.display = "none";
		document.getElementById("NM_connect_title").style.fontSize = "14px";
		document.getElementById("NM_connect_status").style.fontSize = "20px";

		if(isSwMode("RP") || isSwMode("MB") || isSwMode("WISP")){
			document.getElementById('wlc_band_div').style.display = "";
			document.getElementById('dataRate_div').style.display = "";
			if(Rawifi_support || Qcawifi_support){
				document.getElementById('rssi_div').style.display = "none";
			}
			else{
				if(`<% nvram_get("mlo_rp"); %>` == "1" || `<% nvram_get("mlo_mb"); %>` == "1"){
					document.getElementById('rssi_mlo_div').innerHTML = "";
					
					var mlo_bands = `<% nvram_get("mld0_ifnames"); %>`.replace(/wl/g, "").trim().split(/\s+/);
					for(var i=0; i<mlo_bands.length; i++){
						var mlo_band = mlo_bands[i];
						document.getElementById('rssi_mlo_div').innerHTML += `
							<div>
								<span style="font-size:14px;font-family: Verdana, Arial, Helvetica, sans-serif;">${wl_nband_title[mlo_band]} RSSI:</span>
								<strong id="rssi_mlo_${mlo_band}_status" class="index_status" style="font-size:14px;"></strong>
							<\div>
						`;
					}

					document.getElementById('rssi_div').style.display = "none";
					document.getElementById('rssi_mlo_div').style.display = "";
				}
				else{
					document.getElementById('rssi_div').style.display = "";
				}
			}

			document.getElementById('wlc_band_status').innerHTML = wl_nband_title[wlc_band];

			if(`<% nvram_get("mlo_rp"); %>` == "1" || `<% nvram_get("mlo_mb"); %>` == "1"){
				document.getElementById('NM_connect_title').style.display = "none";
				document.getElementById('wlc_band_status').innerHTML = "MLO";
			}
		}

		document.getElementById('NM_connect_title').innerHTML = `<#parent_AP_status#> :`;
	}
	else{
		document.getElementById("index_status").innerHTML = '<span style="word-break:break-all;">' + wanlink_ipaddr + '</span>';
		if(is_CH_sku && wan_ipv6_network_json.status != "0" && wan_ipv6_network_json.IPv6_Address != ""){
			document.getElementById("wanIP_ipv6_div").style.display = "";
			document.getElementById("index_ipv6_status").style.display = "";
			document.getElementById("index_ipv6_status").innerHTML += '<span style="word-break:break-all;">' + wan_ipv6_network_json.IPv6_Address + '</span>';
		}
		if(is_CH_sku && wan_ipv6_network_json.status != "0" && wan_ipv6_network_json.Link_Local_Address != ""){
			document.getElementById("wanIP_ipv6_div").style.display = "";
			document.getElementById("index_ipv6_ll_status").style.display = "";
			document.getElementById("index_ipv6_ll_status").innerHTML += '<span style="word-break:break-all;">' + wan_ipv6_network_json.Link_Local_Address + '</span>';
		}

		setTimeout("show_ddns_status();", 1);
		
		if(wanlink_ipaddr == '0.0.0.0' || wanlink_ipaddr == '')
			document.getElementById("wanIP_div").style.display = "none";

		if(wan_bonding_support && orig_bond_wan == "1"){
			document.getElementById("wanAggr_div").style.display = "block";
			document.getElementById('single_wan_line').style.display = "none";
			document.getElementById('primary_wan_line').style.display = "";
			document.getElementById('secondary_wan_line').style.display = "";
		}
	}

	if(smart_connect_support){
		if(localAP_support && ((isSwMode("RT") || isSwMode("WISP")) || isSwMode("ap"))){
			if((based_modelid == "RT-AC5300") && '<% nvram_get("smart_connect_x"); %>' !=0)
			show_smart_connect_status();
		}
	}

	document.list_form.dhcp_staticlist.value = dhcp_staticlist_orig;
	document.list_form.MULTIFILTER_ENABLE.value = MULTIFILTER_ENABLE_orig;
	document.list_form.MULTIFILTER_MAC.value = MULTIFILTER_MAC_orig;
	document.list_form.MULTIFILTER_DEVICENAME.value = MULTIFILTER_DEVICENAME_orig;
	if(isSupport("PC_SCHED_V3"))
		document.list_form.MULTIFILTER_MACFILTER_DAYTIME_V2.value = MULTIFILTER_MACFILTER_DAYTIME_orig;
	else
		document.list_form.MULTIFILTER_MACFILTER_DAYTIME.value = MULTIFILTER_MACFILTER_DAYTIME_orig;

	updateClientsCount();

	if(isSwMode("mb")){
		document.getElementById("wlSecurityContext").style.display = "none";
		document.getElementById("mbModeContext").style.display = "";
	}

	if(disnwmd_support) 
		$("#networkmap_switch").show();

	if(wans_flag && gobi_support){
		var eLAN_str = "<#Ethernet_wan#>:".replace(/WAN/, "LAN");
		if(dualwan_first_if == 'wan')
			document.getElementById("first_wan_title").innerHTML = "<#Ethernet_wan#>:";
		else if(dualwan_first_if == 'lan')
			document.getElementById("first_wan_title").innerHTML = eLAN_str;
		else if(dualwan_first_if == 'usb'){
			document.getElementById("iconInternet_primary").style.background = "url('images/New_ui/networkmap/Mobile-Broadband.png') no-repeat 0% 0%";
			if(gobi_support)
				document.getElementById("first_wan_title").innerHTML = "<#Mobile_title#>:";
			else
				document.getElementById("first_wan_title").innerHTML = "<#menu5_4_4#>:";
		}

		if(dualwan_second_if == 'wan')
			document.getElementById("second_wan_title").innerHTML = "<#Ethernet_wan#>:";
		else if(dualwan_second_if == 'lan')
			document.getElementById("second_wan_title").innerHTML = eLAN_str;		
		else if(dualwan_second_if == 'usb'){
			document.getElementById("iconInternet_secondary").style.background = "url('images/New_ui/networkmap/Mobile-Broadband.png') no-repeat 0% 0%";
			if(gobi_support)
				document.getElementById("second_wan_title").innerHTML = "<#Mobile_title#>:";
			else
				document.getElementById("second_wan_title").innerHTML = "<#menu5_4_4#>:";
		}
	}

	if(is_TW_sku && document.referrer.indexOf("QIS") != -1){
		if((autodet_state == 6 || autodet_auxstate == 6) && wan_proto == "dhcp"){
			notification.notiClick();
		}
	}

	orig_NM_container_height = parseInt($(".NM_radius_bottom_container").css("height"));

	if(!downsize_4m_support){
		custom_icon_list_api.paramObj.container = $(".custom_icon_list_bg");
		custom_icon_list_api.paramObj.source = "local";
		custom_icon_list_api.paramObj.select_icon_callBack = select_custom_icon;
		custom_icon_list_api.paramObj.upload_callBack = previewImage;
		custom_icon_list_api.gen_component(custom_icon_list_api.paramObj);
		$.getJSON("/ajax/extend_custom_icon.json",
			function(data){
				custom_icon_list_api.paramObj.container = $(".custom_icon_list_bg");
				custom_icon_list_api.paramObj.source = "cloud";
				custom_icon_list_api.paramObj.db = data;
				custom_icon_list_api.paramObj.select_icon_callBack = select_custom_icon;
				custom_icon_list_api.gen_component(custom_icon_list_api.paramObj);
			}
		);
	}
	var MULTIFILTER_BLOCK_ALL = httpApi.nvramGet(["MULTIFILTER_BLOCK_ALL"]).MULTIFILTER_BLOCK_ALL;
	if(MULTIFILTER_BLOCK_ALL == "1")
		$(".block_all_icon").css("display", "flex");

	if(cookie.get("show_phone_as_modem_hints") == "1"){
		$("#phone_as_modem_instructions").load("/phone_as_modem_instructions.html", function(){
			$("#phone_as_modem_div").css("display", "flex");
		});
	}
}

function show_smart_connect_status(){
	document.getElementById("SmartConnectName").style.display = "";
	document.getElementById("SmartConnectStatus").style.display = "";
	var smart_connect_x = '<% nvram_get("smart_connect_x"); %>';

        if(smart_connect_x == '0')
                document.getElementById("SmartConnectStatus").innerHTML = '<a style="color:#FFF;text-decoration:underline;" href="/Advanced_Wireless_Content.asp">Off</a>';
        else if(smart_connect_x == '1')
                document.getElementById("SmartConnectStatus").innerHTML = '<a style="color:#FFF;text-decoration:underline;" href="/Advanced_Wireless_Content.asp">On</a>';

	setTimeout("show_smart_connect_status();", 2000);
}

function show_ddns_status(){
	var ddns_enable = '<% nvram_get("ddns_enable_x"); %>';
	var ddns_server_x = '<% nvram_get("ddns_server_x"); %>';
	var ddnsName;
	var ddns_hostname_x = '<% nvram_get("ddns_hostname_x"); %>';
	var ddns_username_x = '<% nvram_get("ddns_username_x"); %>';

	switch (ddns_server_x){
		case "WWW.NAMECHEAP.COM":
			ddnsName = ddns_hostname_x + "." + ddns_username_x;
			break;
		
		default:
			ddnsName = ddns_hostname_x; 
	}

	document.getElementById("ddns_fail_hint").className = "notificationoff";
	if( ddns_enable == '0'){
        document.getElementById("ddnsHostName").innerHTML = '<a style="color:#FFF;text-decoration:underline;" href="/Advanced_ASUSDDNS_Content.asp?af=ddns_enable_x"><#btn_go#></a>';
        $('#copyDdns').hide();
    }else if(ddnsName == '')
        document.getElementById("ddnsHostName").innerHTML = '<a style="color:#FFF;text-decoration:underline;" href="/Advanced_ASUSDDNS_Content.asp?af=DDNSName"><#sign_up#></a>';
    else if(ddnsName == isMD5DDNSName())
        document.getElementById("ddnsHostName").innerHTML = '<a style="color:#FFF;text-decoration:underline;" href="/Advanced_ASUSDDNS_Content.asp?af=DDNSName"><#sign_up#></a>';
    else{
		if(ddns_server_x == "WWW.DNSOMATIC.COM"){
			document.getElementById("ddnsHostName").innerHTML = '<a style="color:#FFF;text-decoration:underline;" href="https://dnsomatic.com/" target="_blank"><#btn_go#></a>';
            $('#copyDdns').hide();
		}
		else{
            document.getElementById("ddnsHostName").innerHTML = '<span>'+ ddnsName +'</span>';
            $('#copyDdns').show();
		}

		if(ddns_enable == '1'){
			if((link_status != undefined || link_auxstatus != undefined) && !((link_status == "2" && link_auxstatus == "0") || (link_status == "2" && link_auxstatus == "2")) ) //link down
				document.getElementById("ddns_fail_hint").className = "notificationon";
					
			if( ddns_server_x == 'WWW.ASUS.COM' ) { //ASUS DDNS
				if( (ddns_return_code.indexOf('200')==-1) && (ddns_return_code.indexOf('220')==-1) && (ddns_return_code.indexOf('230')==-1))
					document.getElementById("ddns_fail_hint").className = "notificationon";
			}
			else{ //Other ddns service
				if(ddns_updated != '1' || ddns_return_code=='unknown_error' || ddns_return_code=="auth_fail")
					document.getElementById("ddns_fail_hint").className = "notificationon";
			}
        }
	}

    if(le_enable == "1" && le_state == "1")
    	document.getElementById("le_icon").style.display = "";
    else
    	document.getElementById("le_icon").style.display = "none";

	setTimeout("show_ddns_status();", 2000);
}

var isMD5DDNSName = function(){
	var macAddr = '<% nvram_get("lan_hwaddr"); %>'.toUpperCase().replace(/:/g, "");
	return "A"+hexMD5(macAddr).toUpperCase()+".asuscomm.com";
}

function set_default_choice(){
	var icon_name;
	if(flag && flag.length > 0){
		if(flag == "Internet"){
			document.getElementById("iconRouter").style.backgroundPosition = '0% 0%';
			clickEvent(document.getElementById("iconInternet"));
		}
		else if(flag == "Client"){
			document.getElementById("iconRouter").style.backgroundPosition = '0% 0%';
			clickEvent(document.getElementById("iconClient"));
		}
		else if(flag == "USBdisk"){
			document.getElementById("iconRouter").style.backgroundPosition = '0% 0%';
			clickEvent(document.getElementById("iconUSBdisk"));
		}
		else if(flag == "AMesh"){
			document.getElementById("iconRouter").style.backgroundPosition = '0% 0%';
			clickEvent(document.getElementById("iconAMesh"));
		}
		else{
			clickEvent(document.getElementById("iconRouter"));
			return;
		}

		if(flag == "Router2g")
			icon_name = "iconRouter";
		else
			icon_name = "icon"+flag;

		clickEvent(document.getElementById(icon_name));
	}
	else
		clickEvent(document.getElementById("iconRouter"));
}

function showMapWANStatus(flag){
	if(isSwMode("AP")){
		showtext(document.getElementById("NM_connect_status"), "<div style='margin-top:10px;'><#WLANConfig11b_x_APMode_itemname#></div>");
	}
	else if(isSwMode("RP")){
		showtext(document.getElementById("NM_connect_title"), "<div style='margin-top:10px;'><#statusTitle_AP#>:</div><br>");
	}
	else
		return 0;
}

function show_middle_status(auth_mode, wl_wep_x){
	var security_mode;
	switch (auth_mode){
		case "open":
				security_mode = "Open System";
				break;
		case "openowe":
				security_mode = "Enhanced Open Transition";
				break;
		case "owe":
				security_mode = "Enhanced Open";
				break;
		case "shared":
				security_mode = "Shared Key";
				break;
		case "psk":
				security_mode = "WPA-Personal";
				break;
		case "psk2":
				security_mode = "WPA2-Personal";
				break;
		case "sae":
				security_mode = "WPA3-Personal";
				break;
		case "psk2sae":
				security_mode = "WPA2/WPA3-Personal";
				break;
		case "pskpsk2":
				security_mode = "WPA-Auto-Personal";
				document.getElementById("wl_securitylevel_span").style.fontSize = "16px";
				break;
		case "wpa":
				security_mode = "WPA-Enterprise";
				break;
		case "wpa2":
				security_mode = "WPA2-Enterprise";
				break;
		case "wpa3":
				security_mode = "WPA3-Enterprise";
				break;
		case "suite-b":
				security_mode = "WPA3-Enterprise 192-bit";
				document.getElementById("wl_securitylevel_span").style.fontSize = "16px";
				break;
		case "wpawpa2":
				security_mode = "WPA-Auto-Enterprise";
				document.getElementById("wl_securitylevel_span").style.fontSize = "16px";
				break;
		case "wpa2wpa3":
				security_mode = "WPA2/WPA3-Enterprise";
				document.getElementById("wl_securitylevel_span").style.fontSize = "16px";
				break;
		case "radius":
				security_mode = "Radius with 802.1x";
				document.getElementById("wl_securitylevel_span").style.fontSize = "16px";
				break;
		case "unknown":
				security_mode = "<#CTL_Disconnect#>";
				break;
		default:
				security_mode = "Unknown Auth";	
	}
	
	document.getElementById("wl_securitylevel_span").innerHTML = security_mode;

	if(auth_mode == "open" && wl_wep_x == 0)
		document.getElementById("iflock").src = "images/New_ui/networkmap/unlock.png";
	else
		document.getElementById("iflock").src = "images/New_ui/networkmap/lock.png"
}

function show_client_status(num){
	document.getElementById("clientNumber").innerHTML = "<#Full_Clients#>: <span id='_clientNumber'>" + num + "</span>";
}

function show_USBDevice(device){
	if(!usb_support || typeof device != "object")
		return false;

	switch(device.deviceType){
		case "storage":
			disk_html(device);
			break;

		case "printer":
			printer_html(device);
			break;

		case "audio":

		case "webcam":

		case "modem":
			modem_html(device);
			break;

		default:
			no_device_html(device.usbPath);
	}

	currentUsbPort = device.usbPath-1;
	usbPorts[currentUsbPort] = device;
}

function disk_html(device){
	var icon_html_code = '';
	var usb_css = "iconUSBdisk";
	if(device.usbPath == "3")
		usb_css = "iconM2";
	icon_html_code += '<a target="statusframe">\n';
	icon_html_code += '<div id="ring_USBdisk_'+device.usbPath+'" class=' + usb_css + ' style="display:none;z-index:1;">\n';
	icon_html_code += '<div id="iconUSBdisk_'+device.usbPath+'" class=' + usb_css + ' onclick="setSelectedDiskOrder(this.id);clickEvent(this);"></div>\n';
	icon_html_code += '</div>\n';
	icon_html_code += '</a>\n';
	document.getElementById("deviceIcon_" + device.usbPath).innerHTML = icon_html_code;
	document.getElementById("usb_text_" + device.usbPath).className += " plugin";
	showDiskInfo(device);

	// show ring
	check_status(device);
	// check_status2(usb_path1_pool_error, device.usbPath);
}

function showDiskInfo(device){
	var dec_html_code = '';
	var percentbar = 0;

	if(device.mountNumber > 0){
		percentbar = simpleNum2((device.totalSize - device.totalUsed)/device.totalSize*100);
		percentbar = Math.round(100 - percentbar);

		dec_html_code += '<div id="diskquota" align="left" style="margin-top:5px;margin-bottom:10px;">\n';
		dec_html_code += '<div class="quotabar" style="width:'+ percentbar +'%;height:13px;"></div>';
		dec_html_code += '</div>\n';
	}
	else{
		dec_html_code += '<div class="style1"><strong id="diskUnmount'+ device.usbPath +'"><#DISK_UNMOUNTED#></strong></div>\n';
	}

	document.getElementById("deviceDec_"+device.usbPath).innerHTML = dec_html_code;
}

function printer_html(device){
	var icon_html_code = '';
	icon_html_code += '<a href="device-map/printer.asp" target="statusframe">\n';
	icon_html_code += '<div id="iconPrinter_' + device.usbPath + '" class="iconPrinter" onclick="clickEvent(this);"></div>\n';
	icon_html_code += '</a>\n';
	document.getElementById("deviceIcon_" + device.usbPath).innerHTML = icon_html_code;

	if(device.serialNum == '<% nvram_get("u2ec_serial"); %>')
		document.getElementById("deviceDec_" + device.usbPath).innerHTML = '<div style="margin:10px;"><#CTL_Enabled#></div>';
	else
		document.getElementById("deviceDec_" + device.usbPath).innerHTML = '<div style="margin:10px;"><#CTL_Disabled#></div>';
}

function modem_html(device){
	var icon_html_code = '';	
	icon_html_code += '<a href="device-map/modem.asp" target="statusframe">\n';
	icon_html_code += '<div id="iconModem_' + device.usbPath + '" class="iconmodem" onclick="clickEvent(this);"></div>\n';
	icon_html_code += '</a>\n';
	document.getElementById("deviceIcon_" + device.usbPath).innerHTML = icon_html_code;

	document.getElementById("deviceDec_" + device.usbPath).innerHTML = '<div style="margin:10px;">' + device.deviceName + '</div>';
}

function no_device_html(device_seat){
	var device_icon = document.getElementById("deviceIcon_"+device_seat);
	var device_dec = document.getElementById("deviceDec_"+device_seat);
	var icon_html_code = '';
	var dec_html_code = '';
	var css_icon = "iconNo";
	if(device_seat == 3)
		css_icon = "iconNoM2";
	icon_html_code += '<div class="' + css_icon + '""></div>';
	dec_html_code += '<div style="margin:10px" id="noUSB'+ device_seat +'">';

	if(rc_support.search("usbX") > -1)
		dec_html_code += '<#NoDevice#>';
	else
		dec_html_code += '<#CTL_nonsupported#>';

	dec_html_code += '</div>\n';
	device_icon.innerHTML = icon_html_code;
	device_dec.innerHTML = dec_html_code;
}

var avoidkey;
var lastClicked;
var lastName;
var clicked_device_order;

function get_clicked_device_order(){
	return clicked_device_order;
}

function clickEvent(obj){
	if(amesh_support && ((isSwMode("RT") || isSwMode("WISP")) || isSwMode("ap")) && ameshRouter_support) {
		require(['/require/modules/amesh.js'], function(){
			if(typeof initial_amesh_obj == "function")
				initial_amesh_obj();
		});	
	}
	var icon;
	var stitle;
	var seat;
	clicked_device_order = -1;

	if(obj.id.indexOf("Internet") > 0){
		if(!dualWAN_support){
			check_wan_unit();
		}

		icon = "iconInternet";
		stitle = "<#statusTitle_Internet#>";
		document.getElementById("statusframe").src = "/device-map/internet.asp";

		if(wans_flag){
			if(gobi_support) {
				var eLAN_str = "<#Ethernet_wan#>".replace(/WAN/, "LAN");
				if(obj.id.indexOf("primary") != -1){
					if(dualwan_first_if == "wan")
						stitle = "<#Ethernet_wan#> Status";
					else if(dualwan_first_if == "lan")
						stitle = eLAN_str+" Status";
					else if(dualwan_first_if == "usb"){
						if(gobi_support)
							stitle = "<#Mobile_title#> Status";
						else
							stitle = "<#menu5_4_4#> Status";
					}
					else	
						stitle = "<#statusTitle_Primary_WAN#>";
				}
				else{
					if(dualwan_second_if == "wan")
						stitle = "<#Ethernet_wan#> Status";
					else if(dualwan_second_if == "lan")
						stitle = eLAN_str+" Status";
					else if(dualwan_second_if == "usb"){
						if(gobi_support)
							stitle = "<#Mobile_title#> Status";
						else
							stitle = "<#menu5_4_4#> Status";
					}
					else
						stitle = "<#statusTitle_Secondary_WAN#>";
				}
			}
			else {
				if(obj.id.indexOf("primary") != -1)
					stitle = "<#statusTitle_Primary_WAN#>";
				else
					stitle = "<#statusTitle_Secondary_WAN#>";
			}
		}

		if(obj.id.indexOf("Internet_secondary") > 0){
			pap_click_flag = 1;
		}
		else{
			pap_click_flag = 0;
		}		
	}
	else if(obj.id.indexOf("Router") > 0){
		icon = "iconRouter";
		stitle = "<#menu5_7_1#>";
	}
	else if(obj.id.indexOf("Client") > 0){
		icon = "iconClient";
		stitle = "<#statusTitle_Client#>";
	}
	else if(obj.id.indexOf("USBdisk") > 0){
		icon = "iconUSBdisk";
		stitle = "<#statusTitle_USB_Disk#>";
		currentUsbPort = obj.id.slice(-1) - 1;
		if(currentUsbPort == "2") // 0, 1 usb, 2: M.2
			stitle = "M.2 SSD disk status";/*untranslated*/
	}
	else if(obj.id.indexOf("Modem") > 0){
		seat = obj.id.indexOf("Modem")+5;
		clicked_device_order = obj.id.slice(-1);
		currentUsbPort = obj.id.slice(-1) - 1;
		icon = "iconmodem";
		stitle = "<#menu5_4_4#>";
		document.getElementById("statusframe").src = "/device-map/modem.asp";
	}
	else if(obj.id.indexOf("Printer") > 0){
		seat = obj.id.indexOf("Printer") + 7;
		clicked_device_order = parseInt(obj.id.substring(seat, seat+1));
		currentUsbPort = obj.id.slice(-1) - 1;
		icon = "iconPrinter";
		stitle = "<#statusTitle_Printer#>";
		document.getElementById("statusframe").src = "/device-map/printer.asp";
	}
	else if(obj.id.indexOf("No") > 0){
		icon = "iconNo";
	}
	else if(obj.id.indexOf("AMesh") > 0){
		icon = "iconAMesh";
		stitle = "AiMesh";
		setTimeout(function(){
			var flag = '<% get_parameter("flag"); %>';
			var id = '<% get_parameter("id"); %>';
			if(flag != "" && id != "" && id != "donot_search")
				document.getElementById("statusframe").src = "/device-map/amesh.asp?id=" + id + "";
			else
				document.getElementById("statusframe").src = "/device-map/amesh.asp";
		}, 1);
	}
	else
		alert("mouse over on wrong place!");

	if(lastClicked){
		if(lastClicked.id.indexOf("USBdisk") > 0)
			lastClicked.style.backgroundPosition = '1px -4px';
		else if(lastClicked.id.indexOf("AMesh") > 0) {
			lastClicked.classList.remove('iconAMesh');
			lastClicked.classList.add('iconAMesh_dis');
		}
		else
			lastClicked.style.backgroundPosition = '0% 0%';
	}

	if(obj.id.indexOf("USBdisk") > 0){	// To control USB icon outter ring's color
		if(diskUtility_support){
			if(!usbPorts[obj.id.slice(-1)-1].hasErrPart){
				document.getElementById("statusframe").src = "/device-map/disk.asp";	
				obj.style.backgroundPosition = '1px -105px';
			}
			else{
				document.getElementById("statusframe").src = "/device-map/disk_utility.asp";
				obj.style.backgroundPosition = '0% -202px';
			}
		}
		else{
			document.getElementById("statusframe").src = "/device-map/disk.asp";	
			obj.style.backgroundPosition = '1px -105px';
		}
	}
	else if(((obj.id.indexOf("secondary") > 0 && dualwan_second_if == 'usb') || (obj.id.indexOf("primary") > 0 && dualwan_first_if == 'usb')) && gobi_support) {
		obj.style.backgroundPosition = '0% 100%';
	}
	else if(obj.id.indexOf("AMesh") > 0) {
		obj.classList.add('iconAMesh');
		obj.classList.remove('iconAMesh_dis');
	}
	else if(obj.id.indexOf("Client") > 0) {
		var MULTIFILTER_BLOCK_ALL = httpApi.nvramGet(["MULTIFILTER_BLOCK_ALL"]).MULTIFILTER_BLOCK_ALL;
		if(MULTIFILTER_BLOCK_ALL == "1")
			obj.style.backgroundPosition = '0% -192px';
		else
			obj.style.backgroundPosition = '0% -96px';
	}
	else{
		obj.style.backgroundPosition = '0% 101%';
	}

	document.getElementById('helpname').innerHTML = stitle;	
	avoidkey = icon;
	lastClicked = obj;
	lastName = icon;
}

function mouseEvent(obj, key){
	var icon;
	
	if(obj.id.indexOf("Internet") > 0)
		icon = "iconInternet";
	else if(obj.id.indexOf("Router") > 0)
		icon = "iconRouter";
	else if(obj.id.indexOf("Client") > 0)
		icon = "iconClient";
	else if(obj.id.indexOf("USBdisk") > 0)
		icon = "iconUSBdisk";
	else if(obj.id.indexOf("Printer") > 0)
		icon = "iconPrinter";
	else if(obj.id.indexOf("No") > 0)
		icon = "iconNo";
	else
		alert("mouse over on wrong place!");
	
	if(avoidkey != icon){
		if(key){ //when mouseover
			obj.style.background = 'url("/images/map-'+icon+'_r.gif") no-repeat';
		}
		else {  //when mouseout
			obj.style.background = 'url("/images/map-'+icon+'.gif") no-repeat';
		}
	}
}//end of mouseEvent

function showstausframe(page){
	clickEvent(document.getElementById("icon"+page));
	if(page == "Client")
		page = "clients";
	else if(page.indexOf('Internet') == 0){
		if(page == "Internet_secondary")
			document.form.dual_wan_flag.value = 1;
		else	
			document.form.dual_wan_flag.value = 0;
			
		page = "Internet";
	}
	else if(page == "Router"){
		page = isSupport("sdn_mainfh") || isSwMode("MB") ? `${page}_status` : page;
	}
	window.open("/device-map/"+page.toLowerCase()+".asp","statusframe");
}

function check_status(_device){
	var diskOrder = _device.usbPath;
	if(diskOrder == "3")
		document.getElementById('iconUSBdisk_'+diskOrder).style.backgroundImage = "url(/images/New_ui/networkmap/M2.png)";
	else 
		document.getElementById('iconUSBdisk_'+diskOrder).style.backgroundImage = "url(/images/New_ui/networkmap/USB_2.png)";
	document.getElementById('iconUSBdisk_'+diskOrder).style.backgroundPosition = '1px -4px';
	document.getElementById('iconUSBdisk_'+diskOrder).style.backgroundSize = "100%";

	document.getElementById('ring_USBdisk_'+diskOrder).style.backgroundImage = "url(/images/New_ui/networkmap/white_04.gif)";	
	document.getElementById('ring_USBdisk_'+diskOrder).style.display = "";

	if(!diskUtility_support)
		return true;

	var i, j;
	var got_code_0, got_code_1, got_code_2, got_code_3;
	for(i = 0; i < _device.partition.length; ++i){
		switch(parseInt(_device.partition[i].fsck)){
			case 0: // no error.
				got_code_0 = 1;
				break;
			case 1: // find errors.
				got_code_1 = 1;
				break;
			case 2: // proceeding...
				got_code_2 = 1;
				break;
			default: // don't or can't support.
				got_code_3 = 1;
				break;
		}
	}
	
	if(got_code_1){
		// red
		document.getElementById('iconUSBdisk_'+diskOrder).style.backgroundPosition = '1px -206px';
		document.getElementById('ring_USBdisk_'+diskOrder).style.backgroundPosition = '0px -184px';
	}
	else if(got_code_2 || got_code_3){
		// white
	}
	else{
		// blue
		document.getElementById('iconUSBdisk_'+diskOrder).style.backgroundPosition = '1px -4px';
		document.getElementById('ring_USBdisk_'+diskOrder).style.backgroundPosition = '0px -92px';
	}
}

function check_wan_unit(){   //To check wan_unit, if USB Modem plug in change wan_unit to 1
	if(wan0_primary == 1 && document.form.wan_unit.value == 1)
		change_wan_unit(0);
	else if(wan1_primary == 1 && document.form.wan_unit.value == 0)
		change_wan_unit(1);
}
function change_wan_unit(wan_unit_flag){
	document.form.wan_unit.value = wan_unit_flag;	
	document.form.wl_auth_mode_x.disabled = true;	
	document.form.wl_wep_x.disabled = true;		
	FormActions("/apply.cgi", "change_wan_unit", "", "");
	document.form.submit();
}

function show_ddns_fail_hint() {
	var str="";
	var ddns_server_x = '<% nvram_get("ddns_server_x"); %>';
	if(!isSwMode("AP") && document.getElementById("connect_status").className == "connectstatusoff")
		str = "<#Disconnected#>";
	else if(ddns_server_x == 'WWW.ASUS.COM') {
		var ddnsHint = getDDNSState(ddns_return_code, `<%nvram_get("ddns_hostname_x");%>`, `<%nvram_get("ddns_old_name");%>`);
		if(ddnsHint != "")
			str = ddnsHint;
	}
	else 
		str = "<#LANHostConfig_x_DDNS_alarm_2#>";

	if(str != "")
		overlib(str);
}

function check_dualwan(flag){
	if(flag == 0){		//single wan
		document.getElementById('single_wan_icon').style.display = "";
		document.getElementById('single_wan_status').style.display = "";
		document.getElementById('single_wan_line').style.display = "";
		document.getElementById('primary_wan_icon').style.display = "none";
		document.getElementById('secondary_wan_icon').style.display = "none";
		document.getElementById('primary_wan_line').style.display = "none";
		document.getElementById('secondary_wan_line').style.display = "none";
		document.getElementById('dual_wan_gap').style.display = "none";
	}
	else{
		document.getElementById('single_wan_icon').style.display = "none";
		document.getElementById('single_wan_status').style.display = "none";
		document.getElementById('single_wan_line').style.display = "none";
		document.getElementById('primary_wan_icon').style.display = "";
		document.getElementById('secondary_wan_icon').style.display = "";
		document.getElementById('primary_wan_line').style.display = "";
		document.getElementById('secondary_wan_line').style.display = "";
		document.getElementById('dual_wan_gap').style.display = "";
	}
}

function validForm(){
	var validateIpRange = function(ip_obj){
		var retFlag = 1
		var ip_num = inet_network(ip_obj.value);
		if(ip_num <= 0){
			alert(ip_obj.value+" <#JS_validip#>");
			ip_obj.value = document.getElementById("ipaddr_field_orig").value;
			ip_obj.focus();
			retFlag = 0;
		}
		else if(!validator.validIPForm(document.getElementById("ipaddr_field"), 0)){
			ip_obj.value = document.getElementById("ipaddr_field_orig").value;
			ip_obj.focus();
			retFlag = 0;
		}
		else if (ipBindingFlag) {
			const clientMac = document.getElementById('macaddr_field').value.toUpperCase();
			const client_sdn_idx = document.getElementById('client_sdnIdx').getAttribute('client_sdn_idx');
			const specific_sdn = sdn_rl_for_clientlist.find(item => item.sdn_rl.idx.toString() === client_sdn_idx.toString());
			const lan_ipaddr = specific_sdn ? specific_sdn.subnet_rl.addr : `<% nvram_get("lan_ipaddr"); %>`;
			const lan_netmask = specific_sdn ? specific_sdn.subnet_rl.netmask : `<% nvram_get("lan_netmask"); %>`;
			if (ip_num <= getSubnet(lan_ipaddr, lan_netmask, "head") || ip_num >= getSubnet(lan_ipaddr, lan_netmask, "end")) {
				alert(ip_obj.value+" <#JS_validip#>");
				ip_obj.value = document.getElementById("ipaddr_field_orig").value;
				ip_obj.focus();
				retFlag = 0;
			}

			const manually_dhcp_list = (()=>{
				const subnet_idx = specific_sdn && specific_sdn.sdn_rl.subnet_idx !== "0" && specific_sdn.sdn_rl.subnet_idx !== "" 
					? specific_sdn.sdn_rl.subnet_idx 
					: "0";
				return subnet_idx === "0"
					? dhcp_staticlist_orig
					: decodeURIComponent(httpApi.nvramCharToAscii([`dhcpres${subnet_idx}_rl`])[`dhcpres${subnet_idx}_rl`]);
			})();
			manually_dhcp_list.split("<").forEach(function(element, index){
				var existMac = element.split(">")[0];
				var existIP = element.split(">")[1];
				if(existIP == document.getElementById("ipaddr_field").value) {
					if(existMac.toUpperCase() != clientMac.toUpperCase()) {
						alert("<#JS_duplicate#>");
						ip_obj.value = document.getElementById("ipaddr_field_orig").value;
						ip_obj.focus();
						retFlag = 0;
					}
				}
			});
		}
		return retFlag;
	}

	if(validateIpRange(document.getElementById("ipaddr_field")) == 0)
		return false;

	showtext(document.getElementById("alert_msg1"), "");

	document.getElementById('client_name').value = document.getElementById('client_name').value.trim();
	if(document.getElementById('client_name').value.indexOf(">") != -1 || document.getElementById('client_name').value.indexOf("<") != -1){
		alert("<#JS_validstr2#> '<', '>'");
		document.getElementById('client_name').focus();
		document.getElementById('client_name').select();
		document.getElementById('client_name').value = "";
		return false;
	}

	if(utf8_ssid_support){
		var len = validator.lengthInUtf8(document.getElementById('client_name').value);
		if(len > 32){
			alert("Username cannot be greater than 32 characters.");/* untranslated */
			document.getElementById('client_name').focus();
			document.getElementById('client_name').select();
			document.getElementById('client_name').value = "";
			return false;
		}
	}
	else if(!validator.haveFullWidthChar(document.getElementById('client_name'))) {
		alert('<#JS_validchar#>');
		document.getElementById('client_name').focus();
		document.getElementById('client_name').select();
		document.getElementById('client_name').value = "";
		return false;
	}

	return true;
}	

var custom_name = decodeURIComponent('<% nvram_char_to_ascii("", "custom_clientlist"); %>').replace(/&#62/g, ">").replace(/&#60/g, "<");
function edit_confirm(){
	if(validForm()){
		document.list_form.custom_clientlist.disabled = false;
		// customize device name
		var originalCustomListArray = new Array();
		var onEditClient = new Array();
		var clientTypeNum = "";
		if($('#client_image.vendorIcon_no_hover').length > 0) {
			clientTypeNum = "0";
		}
		else {
            if($('#client_image.clientIcon_no_hover i').length > 0){
			    clientTypeNum = $('#client_image.clientIcon_no_hover i').attr('class').replace("type", "");
            }
		}
		var clientMac = document.getElementById('macaddr_field').value.toUpperCase();
		originalCustomListArray = custom_name.split('<');
		onEditClient[0] = document.getElementById('client_name').value.trim();
		onEditClient[1] = clientMac;
		onEditClient[2] = 0;
		onEditClient[3] = clientTypeNum;
		onEditClient[4] = "";
		onEditClient[5] = "";

		for(var i=0; i<originalCustomListArray.length; i++){
			if(originalCustomListArray[i].split('>')[1] != undefined) {
				if(originalCustomListArray[i].split('>')[1].toUpperCase() == clientMac){
					onEditClient[4] = originalCustomListArray[i].split('>')[4]; // set back callback for ROG device
					onEditClient[5] = originalCustomListArray[i].split('>')[5]; // set back keeparp for ROG device
					var app_group_tag = originalCustomListArray[i].split('>')[6]; // for app group tag
					if(typeof app_group_tag != "undefined")	onEditClient[6] = app_group_tag;
					var app_age_tag = originalCustomListArray[i].split('>')[7]; // for app age tag
					if(typeof app_age_tag != "undefined")	onEditClient[7] = app_age_tag;
					var app_groupid_tag = originalCustomListArray[i].split('>')[8]; // for app groupid tag
					if(typeof app_groupid_tag != "undefined")	onEditClient[8] = app_groupid_tag;
					originalCustomListArray.splice(i, 1); // remove the selected client from original list
				}
			}
		}

		originalCustomListArray.push(onEditClient.join('>'));
		custom_name = originalCustomListArray.join('<');
		document.list_form.custom_clientlist.value = custom_name;

		// static IP list
		const client_sdn_idx = document.getElementById('client_sdnIdx').getAttribute('client_sdn_idx');
		const specific_sdn = sdn_rl_for_clientlist.find(item => item.sdn_rl.idx.toString() === client_sdn_idx.toString());
		const subnet_idx = specific_sdn && specific_sdn.sdn_rl.subnet_idx !== "0" && specific_sdn.sdn_rl.subnet_idx !== ""
			? specific_sdn.sdn_rl.subnet_idx
			: "0";
		const isMainNetwork = subnet_idx === "0";
		const manually_dhcp_list = isMainNetwork
			? document.list_form.dhcp_staticlist.value
			: decodeURIComponent(httpApi.nvramCharToAscii([`dhcpres${subnet_idx}_rl`])[`dhcpres${subnet_idx}_rl`]);
		let final_manually_dhcp_list = "";
		if(ipBindingFlag) {
			if(manually_dhcp_list.indexOf(clientMac) == -1){//new
					final_manually_dhcp_list = manually_dhcp_list;
					final_manually_dhcp_list += `<${clientMac}>${document.getElementById("ipaddr_field").value}>>`;
			}
			else{//update
				manually_dhcp_list.split("<").forEach(function(element, index){
					if(element != ""){
						if(element.indexOf(clientMac) != -1){
							var client_array = element.split(">");
							var mac = client_array[0];
							if(mac == clientMac){
								const ip = document.getElementById("ipaddr_field").value;
								const dns = (client_array[2] == undefined) ? "" : client_array[2];
								const hostname = (client_array[3] == undefined) ? "" : client_array[3];
								final_manually_dhcp_list += "<" + mac + ">" + ip + ">" + dns + ">" + hostname;
							}
						}
						else
							final_manually_dhcp_list += "<" + element;
					}
				});
			}
		}
		else{
			final_manually_dhcp_list = manually_dhcp_list.split('<').filter(function(element) {
			 return element.indexOf(clientMac) === -1;
			}).join('<');
		}
		if(final_manually_dhcp_list === manually_dhcp_list || (!isSwMode("RT") && !isSwMode("WISP"))){
			document.list_form.action_script.value = "saveNvram";
			document.list_form.action_wait.value = "1";
			document.list_form.flag.value = "background";
			document.list_form.dhcp_staticlist.disabled = true;
			document.list_form.dhcp_static_x.disabled = true;
			if (isMainNetwork) {
				dhcp_staticlist_orig = document.list_form.dhcp_staticlist.value = final_manually_dhcp_list;
			}
		}
		else {
			document.list_form.action_script.value = "restart_dnsmasq";
			document.list_form.action_wait.value = "5";
			document.list_form.flag.value = "";
			document.list_form.dhcp_static_x.value = 1;
			document.list_form.dhcp_static_x.disabled = false;
			if (isMainNetwork) {
				dhcp_staticlist_orig = document.list_form.dhcp_staticlist.value = final_manually_dhcp_list;
				document.list_form.dhcp_staticlist.disabled = false;
			}
			else {
				document.list_form.dhcp_staticlist.disabled = true;
				const dhcp_static = final_manually_dhcp_list === '' ? "0" : "1";
				const dhcp_unit = dhcp_static === '1' ? subnet_idx : "";
				let subnet_rl = decodeURIComponent(httpApi.nvramCharToAscii(["subnet_rl"]).subnet_rl);
				let subnetArray = subnet_rl.split('<').filter(part => part !== '');
				for (let i = 0; i < subnetArray.length; i++) {
					let parts = subnetArray[i].split('>');
					if (parts[0] === subnet_idx) {
						parts[11] = dhcp_static;
						parts[12] = dhcp_unit;
						subnetArray[i] = parts.join('>');
					}
				}
				subnet_rl = '<' + subnetArray.join('<');
				let nvramSet_obj = {"action_mode": "apply"};
				nvramSet_obj[`dhcpres${subnet_idx}_rl`] = final_manually_dhcp_list;
				nvramSet_obj[`subnet_rl`] = subnet_rl;
				httpApi.nvramSet(nvramSet_obj, () => {
					httpApi.nvramGet([`dhcpres${subnet_idx}_rl`, `subnet_rl`], true);
				});
			}
		}

		if((isSwMode("RT") || isSwMode("WISP")) && !clientList[document.getElementById("macaddr_field").value].amesh_isRe)
			addToBlockMacList(document.getElementById("macaddr_field").value);

		//  block Mac list
		var turnOnTimeScheduling = false;
		if(document.list_form.MULTIFILTER_ALL.value == "0" && (timeSchedulingFlag || blockInternetFlag))
			turnOnTimeScheduling = true;

		if((document.list_form.MULTIFILTER_MAC.value == MULTIFILTER_MAC_orig && 
			document.list_form.MULTIFILTER_ENABLE.value == MULTIFILTER_ENABLE_orig) && 
			!turnOnTimeScheduling ||
			(!isSwMode("RT") && !isSwMode("WISP"))){
			document.list_form.MULTIFILTER_ALL.disabled = true;
			document.list_form.MULTIFILTER_ENABLE.disabled = true;
			document.list_form.MULTIFILTER_MAC.disabled = true;
			document.list_form.MULTIFILTER_DEVICENAME.disabled = true;
			if(isSupport("PC_SCHED_V3"))
				document.list_form.MULTIFILTER_MACFILTER_DAYTIME_V2.disabled = true;
			else
				document.list_form.MULTIFILTER_MACFILTER_DAYTIME.disabled = true;
		}
		else {
			document.list_form.flag.value = "";
			if(document.list_form.action_script.value == "restart_net_and_phy") {
				document.list_form.action_script.value += ";restart_firewall";
				document.list_form.action_wait.value = httpApi.hookGet("get_default_reboot_time");
			}
			else {
				document.list_form.action_script.value = "restart_firewall";
				document.list_form.action_wait.value = "1";
				document.list_form.flag.value = "background";
			}
			
			document.list_form.MULTIFILTER_ALL.disabled = false;
			document.list_form.MULTIFILTER_ALL.value = "1";
			document.list_form.MULTIFILTER_ENABLE.disabled = false;
			document.list_form.MULTIFILTER_MAC.disabled = false;
			document.list_form.MULTIFILTER_DEVICENAME.disabled = false;
			if(isSupport("PC_SCHED_V3"))
				document.list_form.MULTIFILTER_MACFILTER_DAYTIME_V2.disabled = false;
			else
				document.list_form.MULTIFILTER_MACFILTER_DAYTIME.disabled = false;
		}
		MULTIFILTER_ENABLE_orig = document.list_form.MULTIFILTER_ENABLE.value;
		MULTIFILTER_MAC_orig = document.list_form.MULTIFILTER_MAC.value;
		MULTIFILTER_DEVICENAME_orig = document.list_form.MULTIFILTER_DEVICENAME.value;
		if(isSupport("PC_SCHED_V3"))
			MULTIFILTER_MACFILTER_DAYTIME_orig = document.list_form.MULTIFILTER_MACFILTER_DAYTIME_V2.value;
		else
			MULTIFILTER_MACFILTER_DAYTIME_orig = document.list_form.MULTIFILTER_MACFILTER_DAYTIME.value;

		// handle user image
		document.list_form.custom_usericon.disabled = true;
		if(usericon_support) {
			var clientMac = document.getElementById("macaddr_field").value.replace(/\:/g, "");
			document.list_form.custom_usericon.disabled = false;
			if(userIconBase64 != "NoIcon" && (userIconBase64 != userIconBase64_ori)) {
				if(userUploadFlag)
					document.list_form.custom_usericon.value = clientMac + ">" + userIconBase64;
				else{
					document.list_form.custom_usericon.value = clientTypeNum + ">" + userIconBase64;
					document.list_form.usericon_mac.disabled = false;
					document.list_form.usericon_mac.value = clientMac;
				}
			}
			else if(userIconBase64 == "NoIcon"){
				document.list_form.custom_usericon.value = clientMac + ">noupload";
			}
			else{
				document.list_form.custom_usericon.disabled = true;
				document.list_form.usericon_mac.disabled = true;
			}
		}

		// submit list_form
		document.list_form.submit();

		// display waiting effect
		if(document.list_form.flag.value == "background"){
			document.getElementById("loadingIcon").style.display = "";
			setTimeout(function(){
				if(timeSchedulingFlag && document.getElementById("internetTimeScheduling").style.display == "none") { //if the latest internetMode is not time mode, then redirect to ParentalControl
					redirectTimeScheduling(document.getElementById('macaddr_field').value);
				}
				else {
					document.getElementById("statusframe").contentWindow.refreshpage();
				}
			}, document.list_form.action_wait.value * 1000);
		}
		else{
			hideEditBlock(); 
			setTimeout(function(){
				if(timeSchedulingFlag && document.getElementById("internetTimeScheduling").style.display == "none") { //if the latest internetMode is not time mode, then redirect to ParentalControl
					redirectTimeScheduling(document.getElementById('macaddr_field').value);
				}
				else {
					refreshpage();
				}
			}, document.list_form.action_wait.value * 1000);
		}
	}		
}

function edit_cancel(){
	document.getElementById('edit_client_block').style.display = "none";
	document.getElementById("hiddenMask").style.visibility = "hidden";
	document.getElementById("dr_sweet_advise").style.display = "";

	// disable event listener
	$(document).mouseup(function(e){});
	$("#statusframe").contents().mouseup(function(e){});
}

function edit_delete(){
	var target_mac = document.getElementById('macaddr_field').value;
	var custom_name_row = custom_name.split('<');
	var custom_name_row_temp = custom_name.split('<');
	var custom_name_temp = "";
	var match_delete_flag = 0;
	
	for(i=0;i<custom_name_row.length;i++){
		var custom_name_col = custom_name_row[i].split('>');

		if(target_mac == custom_name_col[1]){
			match_delete_flag = 1;
		}
		else{
			if(custom_name_temp != ""){
				custom_name_temp += "<";
			}
		
			for(j=0;j< custom_name_col.length;j++){
				if(j == custom_name_col.length-1)
					custom_name_temp += custom_name_col[j];
				else	
					custom_name_temp += custom_name_col[j] + ">";				
			}
		}
	}
	
	if(match_delete_flag == 1){
		document.list_form.custom_clientlist.value = custom_name_temp;
		custom_name = custom_name_temp;
		document.getElementById("loadingIcon").style.display = "";
		document.list_form.submit();
		document.getElementById("statusframe").contentWindow.refreshpage();

		setTimeout("document.getElementById('loadingIcon').style.display='none'", 3500);
		setTimeout("document.getElementById('deleteBtn').style.display='none'", 3500);
	}
}

function show_custom_image() {
	if(top.isIE8) return false;

	var display_state = $(".custom_icon_list_bg").css("display");
	if(display_state == "none") {
		$(".custom_icon_list_bg").slideDown("slow");
		document.getElementById("changeIconTitle").innerHTML = "<#CTL_close#>";
	}
	else {
		$(".custom_icon_list_bg").slideUp("slow");
		document.getElementById("changeIconTitle").innerHTML = "<#CTL_Change#>";
	}
}
function select_custom_icon($obj){
	var type = $obj.find('i').attr("class");
	var icon_url = $obj.find('i').css("mask").replace('url(','').replace(')','').replace(/\"/gi, "");
    icon_url = icon_url.replace(/\s*50%\s+50%\s*\/\s*contain\s+no-repeat\s*/, '');
    $("#client_image").empty();
    $("#client_image").append($('<i>').addClass(type).attr('style','--svg:url(' + icon_url + ')'));
	$("#client_image").removeClass().addClass("clientIcon_no_hover");
	userIconBase64 = icon_url;
	userUploadFlag = false;
}
function select_image(clientObj, useDefaultType = false){

    function useTypeIcon(clientObj){
        let type = (useDefaultType) ? "type" + clientObj.defaultType : "type" + clientObj.type;

        $("#client_image").empty();
        $("#client_image").append($('<i>').addClass(type));
        $("#client_image").removeClass().addClass("clientIcon_no_hover");
        if(vendorIcon != "" && type == "type0" && !downsize_4m_support) {
            var vendorIconClassName = getVendorIconClassName(vendorIcon.toLowerCase());
            if(vendorIconClassName != "") {
                $("#client_image").empty();
                $("#client_image").append($('<i>').addClass("vendor-icon").addClass(vendorIconClassName));
                $("#client_image").removeClass().addClass("vendorIcon_no_hover");
            }
        }

        let userImageFlag = false;
        if(!firstTimeOpenBlock) {
            if(usericon_support) {
                userIconBase64 = getUploadIcon(clientObj.mac.replace(/\:/g, ""));
                userIconBase64_ori = userIconBase64;
                if(userIconBase64 != "NoIcon") {
                    $("#client_image").empty();
                    if(clientObj.isUserUplaodImg){
                        $('#client_image').append($('<img>').addClass('clientIcon_no_hover').attr('src',userIconBase64));
                    }else{
                        $('#client_image').append($('<i>').addClass(type).attr('style','--svg:url('+userIconBase64+');'));
                    }
                    userImageFlag = true;
                }
            }
        }

        if(!userImageFlag) {
            userIconBase64 = "NoIcon";
            if(type == "type36")
                $("#client_image").find("i").addClass("flash");
        }
    }

    if(useDefaultType && clientObj.isASUS && clientObj.name!=="ASUS"){
        fetch(`https://nw-dlcdnet.asus.com/plugin/productIcons/${clientObj.name}.png`)
            .then(response => {
                if (response.status === 200) {
                    response.blob().then(blob => {
                        const reader = new FileReader();
                        reader.readAsDataURL(blob);
                        reader.onloadend = function () {
                            $("#client_image").empty();
                            $('#client_image').append($('<img>').addClass('clientIcon_no_hover').attr('src',reader.result));
                            userIconBase64 = reader.result;
                            userUploadFlag = true;
                        }
                    });
                }
            })
            .catch(error => {
                console.error('Error:', error);
                useTypeIcon(clientObj);
            });
    }else{
        useTypeIcon(clientObj);
    }
}

function hideEditBlock(){
	document.getElementById('edit_client_block').style.display = "none";
	document.getElementById('edit_usericon_block').style.display = "none";
	document.getElementById('loadingIcon').style.display = 'none';
	document.getElementById('loadingUserIcon').style.display = 'none';
	document.getElementById('deleteBtn').style.display ='none';
}

function oui_query(mac){
	var queryStr = mac.replace(/\:/g, "").splice(6,6,"");

	$.getJSON("/ajax/ouiDB.json", function(data){
		if(data != "" && data[queryStr] != undefined){
			if(document.getElementById("edit_client_block").style.display == "none") return true;
			var vendor_name = data[queryStr].trim();
			document.getElementById('manufacturer_field').value = vendor_name;
			document.getElementById('manufacturer_field').title = "";
			if(vendor_name.length > 38) {
				document.getElementById('manufacturer_field').value = vendor_name.substring(0, 36) + "..";
				document.getElementById('manufacturer_field').title = vendor_name;
			}
		}
	});
}

function popupEditBlock(clientObj){
	if(downsize_4m_support) {
		document.getElementById("changeClientIconControl").style.display = "none";
		document.getElementById("divDropClientImage").onclick = null;
	}

	if(bwdpi_support) {
		document.getElementById("time_scheduling_title").innerHTML = "<#Time_Scheduling#>";
	}

	firstTimeOpenBlock = false;
	
	var clientName = (clientObj.nickName == "") ? clientObj.name : clientObj.nickName;
	if(document.getElementById("edit_client_block").style.display != "none" && document.getElementById('client_name').value == clientName){
		$("#edit_client_block").fadeOut(300);
	}
	else{
		document.list_form.dhcp_staticlist.value = dhcp_staticlist_orig;
		document.list_form.MULTIFILTER_ENABLE.value = MULTIFILTER_ENABLE_orig;
		document.list_form.MULTIFILTER_MAC.value = MULTIFILTER_MAC_orig;
		document.list_form.MULTIFILTER_DEVICENAME.value = MULTIFILTER_DEVICENAME_orig;
		if(isSupport("PC_SCHED_V3"))
			document.list_form.MULTIFILTER_MACFILTER_DAYTIME_V2.value = MULTIFILTER_MACFILTER_DAYTIME_orig;
		else
			document.list_form.MULTIFILTER_MACFILTER_DAYTIME.value = MULTIFILTER_MACFILTER_DAYTIME_orig;
		document.getElementById("divDropClientImage").ondrop = null;
		document.getElementById("internetTimeScheduling").style.display = "none";
		if((isSwMode("RT") || isSwMode("WISP")) && !clientObj.amesh_isRe) {
			document.getElementById('tr_adv_setting').style.display = "";
		}
		else {
			document.getElementById('tr_adv_setting').style.display = "none";
		}
		$(".custom_icon_list_bg").hide();
		document.getElementById("changeIconTitle").innerHTML = "<#CTL_Change#>";

		var rssi_t = 0;
		var connectModeTip = "";
		var clientIconHtml = "";
		if(clientObj.isWL == "0") {
			rssi_t = "wired";
			connectModeTip = "<#tm_wired#>";
		}
		else {
			rssi_t = client_convRSSI(clientObj.rssi);
			switch (rssi_t) {
				case 1:
					connectModeTip = "<#Radio#>: <#PASS_score1#>\n";
					break;
				case 2:
					connectModeTip = "<#Radio#>: <#PASS_score2#>\n";
					break;
				case 3:
					connectModeTip = "<#Radio#>: <#PASS_score3#>\n";
					break;
				case 4:
					connectModeTip = "<#Radio#>: <#PASS_score4#>\n";
					break;
			}
			if(stainfo_support) {
				if(clientObj.curTx != "")
					connectModeTip += "Tx Rate: " + clientObj.curTx + "\n";
				if(clientObj.curRx != "")
					connectModeTip += "Rx Rate: " + clientObj.curRx + "\n";
				connectModeTip += "<#Access_Time#>: " + clientObj.wlConnectTime + "";
			}
		}

		if(!isSwMode("MB")){
			var radioIcon_css = "radioIcon";
			if(clientObj.isGN != "" && clientObj.isGN != undefined)
				radioIcon_css += " GN";
			clientIconHtml += '<div class="' + radioIcon_css + ' radio_' + rssi_t +'" title="' + connectModeTip + '"></div>';
			if(clientObj.isWL != 0 || (isSupport("mtlancfg") && clientObj.sdn_idx > 0)){
				var bandClass = "band_txt";
				if(navigator.userAgent.toUpperCase().match(/CHROME\/([\d.]+)/)){
					bandClass = "band_txt_chrome";
				}
				let band_text = isWL_map[clientObj.isWL]["text"];
				if(isSupport("mlo") && clientObj.mlo == "1") band_text = `MLO`;
				clientIconHtml += `<div class="band_block"><span class="${bandClass}">${band_text}</span></div>`;
			}
			document.getElementById('client_interface').innerHTML = clientIconHtml;
			document.getElementById('client_interface').title = connectModeTip;
		}

		document.getElementById('client_ipMethod').style.display = "none";
		document.getElementById('client_login').style.display = "none";
		document.getElementById('client_printer').style.display = "none";
		document.getElementById('client_iTunes').style.display = "none";
		document.getElementById('client_opMode').style.display = "none";
		document.getElementById('client_sdnIdx').style.display = "none";
		if((isSwMode("RT") || isSwMode("WISP"))) {
			document.getElementById('client_ipMethod').style.display = "";
			document.getElementById('client_ipMethod').innerHTML = clientObj.ipMethod;
			document.getElementById('client_ipMethod').onmouseover = function() {return overlib(ipState[clientObj.ipMethod]);};
			document.getElementById('client_ipMethod').onmouseout = function() {nd();};
		}
		if(clientObj.isLogin) {
			document.getElementById('client_login').style.display = "";
			document.getElementById('client_login').innerHTML = "<#Clientlist_Logged_In_User#>";
		}
		if(clientObj.isPrinter) {
			document.getElementById('client_printer').style.display = "";
			document.getElementById('client_printer').innerHTML = "<#Clientlist_Printer#>";
		}
		if(clientObj.isITunes) {
			document.getElementById('client_iTunes').style.display = "";
			document.getElementById('client_iTunes').innerHTML = "iTunes";
		}
		if(clientObj.sdn_idx > 0) {
			document.getElementById('client_sdnIdx').style.display = "";
			const sdn_profile = sdn_rl_for_clientlist.find(item => item.sdn_rl.idx == clientObj.sdn_idx) || {};
			const sdn_ssid = $.isEmptyObject(sdn_profile) ? "" : sdn_profile.apg_rl.ssid;
			document.getElementById('client_sdnIdx').innerHTML = "SDN " + sdn_ssid;
			document.getElementById('client_sdnIdx').setAttribute('client_sdn_idx', clientObj.sdn_idx);
		}
		else {
			document.getElementById('client_sdnIdx').setAttribute('client_sdn_idx', '0');
		}

		if(clientObj.opMode != 0) {
			var opModeDes = ["none", "<#wireless_router#>", "<#OP_RE_item#>", "<#OP_AP_item#>", "<#OP_MB_item#>"];
			document.getElementById('client_opMode').style.display = "";
			document.getElementById('client_opMode').innerHTML = opModeDes[clientObj.opMode];
		}
		
		document.getElementById('client_name').value = clientName;
		document.getElementById('ipaddr_field_orig').value = clientObj.ip;
		document.getElementById('ipaddr_field').value = clientObj.ip;

		document.getElementById('hostname_field').value = clientObj.hostname;

		document.getElementById('ipaddr_field').disabled = true;
		$("#ipaddr_field").addClass("client_input_text_disabled");
		if((isSwMode("RT") || isSwMode("WISP")) && !clientObj.amesh_isRe) {
			$("#ipaddr_field").removeClass("client_input_text_disabled");
			document.getElementById('ipaddr_field').disabled = false;
			document.getElementById("ipaddr_field").onkeypress = function() {
				if(!ipBindingFlag) {
					$('#radio_IPBinding_enable').click();
					ipBindingFlag = true;
				}	
			}
			document.getElementById("ipaddr_field").onblur = function() {
				if(!ipBindingFlag) {
					$('#radio_IPBinding_enable').click();
					ipBindingFlag = true;
				}				
			}
		}
		document.getElementById('macaddr_field').value = clientObj.mac;
		var deviceTitle = (clientObj.dpiDevice == "") ? clientObj.vendor : clientObj.dpiDevice;
		if(deviceTitle == undefined || deviceTitle == "") {
			document.getElementById('manufacturer_field').value = "Loading manufacturer..";
			setTimeout(function(){
				if((httpApi.isConnected(0) || httpApi.isConnected(1)) && clientObj.internetState) {
					oui_query(clientObj.mac);
				}
			}, 1000);
		}
		else {
			document.getElementById('manufacturer_field').value = deviceTitle;
			document.getElementById('manufacturer_field').title = "";
			if(deviceTitle.length > 38) {
				document.getElementById('manufacturer_field').value = deviceTitle.substring(0, 36) + "..";
				document.getElementById('manufacturer_field').title = deviceTitle;
			}
		}
	
		const specific_sdn = sdn_rl_for_clientlist.find(item => item.sdn_rl.idx.toString() === clientObj.sdn_idx.toString());
		const subnet_idx = specific_sdn && specific_sdn.sdn_rl.subnet_idx !== "0" && specific_sdn.sdn_rl.subnet_idx !== "" 
			? specific_sdn.sdn_rl.subnet_idx 
			: "0";
		const isMainNetwork = subnet_idx === "0";
		const manually_dhcp_list = isMainNetwork
			? dhcp_staticlist_orig
			: decodeURIComponent(httpApi.nvramCharToAscii([`dhcpres${subnet_idx}_rl`])[`dhcpres${subnet_idx}_rl`]);
		var setRadioIPBinding = function (state, mode, mac) {
			const manually_dhcp_maximum = (()=>{
				const maxRuleLimit = isSupport("MaxRule_extend_limit") == 0 ? 64 : isSupport("MaxRule_extend_limit");
				return isMainNetwork ? maxRuleLimit : 32;
			})();

			const manually_dhcp_list_num = manually_dhcp_list.split("<").length - 1;
			const parentctrl_maximum = (isSupport("MaxRule_parentctrl") == 0) ? 16 : isSupport("MaxRule_parentctrl");
			switch (mode) {
				case "ipBinding" :
					$('#radio_IPBinding_enable').iphoneSwitch(state,
						function(){
							if(manually_dhcp_list.search(mac) == -1) {
								if(manually_dhcp_list_num >= manually_dhcp_maximum) {
									if(confirm(stringSafeGet("<#Clientlist_IPMAC_Binding_max#>".replace("64", manually_dhcp_maximum)))) {
										location.href = isMainNetwork ? "Advanced_DHCP_Content.asp" : "SDN.asp";
									}
									else {
										document.getElementById("ipaddr_field").value = document.getElementById("ipaddr_field_orig").value;
										setRadioIPBinding(0, "ipBinding", mac);
										ipBindingFlag = false;
										return false;
									}
								}
							}
							ipBindingFlag = true;
						},
						function(){
							document.getElementById("ipaddr_field").value = document.getElementById("ipaddr_field_orig").value;
							ipBindingFlag = false;
						}
					);
					break;
				case "blockInternet" :
					$('#radio_BlockInternet_enable').iphoneSwitch(state,
						function(){
							if(MULTIFILTER_MAC_orig.search(mac) == -1) {
								if(MULTIFILTER_num >= parentctrl_maximum) {
									if(confirm(stringSafeGet("<#Clientlist_block_internet_max#>".replace("16", parentctrl_maximum)))) {
										location.href = "ParentalControl.asp" ;
									}
									else {
										setRadioIPBinding(0, "blockInternet", mac);
										return false;
									}
								}
							}
							setRadioIPBinding(0, "timeScheduling", mac);
							timeSchedulingFlag = false;
							blockInternetFlag = true;
						},
						function(){
							timeSchedulingFlag = false;
							blockInternetFlag = false;
						}
					);
					break;
				case "timeScheduling" :
					$('#radio_TimeScheduling_enable').iphoneSwitch(state,
						function(){
							if(MULTIFILTER_MAC_orig.search(mac) == -1) {
								if(MULTIFILTER_num >= parentctrl_maximum) {
									if(confirm(stringSafeGet("<#Clientlist_block_internet_max#>".replace("16", parentctrl_maximum)))) {
										location.href = "ParentalControl.asp" ;
									}
									else {
										setRadioIPBinding(0, "timeScheduling", mac);
										return false;
									}
								}
							}
							setRadioIPBinding(0, "blockInternet", mac);
							timeSchedulingFlag = true;
							blockInternetFlag = false;
						},
						function(){
							timeSchedulingFlag = false;
							blockInternetFlag = false;
						}
					);
					break;
			}
		};

		if(manually_dhcp_list.search(clientObj.mac + ">" + clientObj.ip) != -1) { //check mac>ip is combination the the radio_IPBinding_enable is manual
			setRadioIPBinding(1, "ipBinding", clientObj.mac);
			ipBindingFlag = true;
		}
		else {
			setRadioIPBinding(0, "ipBinding", clientObj.mac);
			ipBindingFlag = false;
		}

		switch(clientObj.internetMode) {
			case "allow" :
				setRadioIPBinding(0, "blockInternet", clientObj.mac);
				setRadioIPBinding(0, "timeScheduling", clientObj.mac);
				timeSchedulingFlag = false;
				blockInternetFlag = false;
				break;
			case "block" :
				setRadioIPBinding(1, "blockInternet", clientObj.mac);
				setRadioIPBinding(0, "timeScheduling", clientObj.mac);
				timeSchedulingFlag = false;
				blockInternetFlag = true;
				break;
			case "time" :
				setRadioIPBinding(0, "blockInternet", clientObj.mac);
				setRadioIPBinding(1, "timeScheduling", clientObj.mac);
				document.getElementById("internetTimeScheduling").style.display = "";
				timeSchedulingFlag = true;
				blockInternetFlag = false;
				break;
		}

		vendorIcon = clientObj.vendor;

		select_image(clientObj);

		//setting user upload icon attribute start.
		//1.check rc_support
		if(usericon_support && !downsize_4m_support) {
			//2.check browswer support File Reader and Canvas or not.
			if(isSupportFileReader() && isSupportCanvas()) {
				//Setting drop event
				var holder = document.getElementById("divDropClientImage");
				holder.ondragover = function () { return false; };
				holder.ondragend = function () { return false; };
				holder.ondrop = function (e) {
					e.preventDefault();
					var userIconLimitFlag = userIconNumLimit(document.getElementById("macaddr_field").value);
					if(userIconLimitFlag) {	//not over 100	
						var file = e.dataTransfer.files[0];
						//check image
						if(file.type.search("image") != -1) {
							var reader = new FileReader();
							reader.onload = function (event) {
                                var img = document.createElement("img");
                                img.src = event.target.result;
                                img.className="clientIcon_no_hover";
                                var mimeType = img.src.split(",")[0].split(":")[1].split(";")[0];
                                let canvas = document.createElement('canvas');
                                canvas.width=85;
                                canvas.height=85;
                                var ctx = canvas.getContext("2d");
                                ctx.clearRect(0,0,85,85);
                                $("#client_image").empty();
                                $("#client_image").append(img);
                                setTimeout(function() {
                                    ctx.drawImage(img, 0, 0, 85, 85);
                                    var dataURL = canvas.toDataURL(mimeType);
                                    userIconBase64 = dataURL;
                                }, 100); //for firefox FPS(Frames per Second) issue need delay
							};
							reader.readAsDataURL(file);
							userUploadFlag = true;
							return false;
						}
						else {
							alert("<#Setting_upload_hint#>");
							return false;
						}
					}
					else {	//over 100 then let usee select delete icon or nothing
						showClientIconList();
					}
				};
			} 
		}
		//setting user upload icon attribute end.
		
		// hide block btn
		// document.getElementById("blockBtn").style.display = (clientObj.isWL && document.maclist_form.wl0_macmode.value != "allow") ? "" : "none";
		cal_panel_block("edit_client_block", 0.23);
		$("#edit_client_block").fadeIn(300);
	}

	firstTimeOpenBlock = true;

	// hide client panel 
	$(document).mouseup(function(e){
		if(!$("#edit_client_block").is(e.target) && $("#edit_client_block").has(e.target).length === 0 && !userIconHideFlag) {
			setTimeout( function() {userIconHideFlag = false;}, 1000);
			edit_cancel();
		}
		else {
			setTimeout( function() {userIconHideFlag = false;}, 1000);
		}
	});
	$("#statusframe").contents().mouseup(function(e){
		if(!$("#edit_client_block").is(e.target) && $("#edit_client_block").has(e.target).length === 0 && !userIconHideFlag) {
			setTimeout( function() {userIconHideFlag = false;}, 1000);
			edit_cancel();
		}
		else {
			setTimeout( function() {userIconHideFlag = false;}, 1000);
		}
	});

	if(top.isIE8){
		document.getElementById('client_image').style.backgroundPosition = "50% -15% !important";
		document.getElementById('client_image').className = "clientIconIE8HACK";
	}
}

function check_usb3(){
	if(based_modelid == "BRT-AC828") {
		document.getElementById('usb_text_1').innerHTML = "USB 3.0";
		document.getElementById('usb_text_2').innerHTML = "USB 3.0";
		document.getElementById('usb_text_3').innerHTML = "M.2 SSD";
	}
	else{
		const rate_map_USB = [
			{value:"480",text:"USB 2.0"},
			{value:"5000",text:"USB 3.0"},
			{value:"10000",text:"USB 3.2"}
		];
		const get_usb_phy_port = (httpApi.hookGet("get_usb_phy_port") == undefined) ? [] : httpApi.hookGet("get_usb_phy_port");
		$.each(get_usb_phy_port, function(index, usb_info){
			const usb_text = rate_map_USB.find(function(item){
				return (item.value == usb_info.max_rate);
			});
			if(usb_text != undefined){
				const usb_idx = usb_info.label_name.substring(1,2);
				if($(`#usb_text_${usb_idx}`).length > 0){
					$(`#usb_text_${usb_idx}`).html(usb_text.text);
				}
			}
		});
	}
}

function addToBlockMacList(macAddr){
	if(document.list_form.MULTIFILTER_MAC.value.indexOf(macAddr) == -1){//new rule
		if(timeSchedulingFlag || blockInternetFlag) {
			if(document.list_form.MULTIFILTER_MAC.value == "") {
				if(timeSchedulingFlag)
					document.list_form.MULTIFILTER_ENABLE.value = "1";
				else if(blockInternetFlag)
					document.list_form.MULTIFILTER_ENABLE.value = "2";
				document.list_form.MULTIFILTER_MAC.value = macAddr;
				document.list_form.MULTIFILTER_DEVICENAME.value = document.getElementById("client_name").value.trim();
				if(isSupport("PC_SCHED_V3"))
					document.list_form.MULTIFILTER_MACFILTER_DAYTIME_V2.value = "W03E21000700<W04122000800";
				else
					document.list_form.MULTIFILTER_MACFILTER_DAYTIME.value = "<";
			}
			else {
				document.list_form.MULTIFILTER_ENABLE.value += ">";
				if(timeSchedulingFlag)
					document.list_form.MULTIFILTER_ENABLE.value += "1";
				else if(blockInternetFlag)
					document.list_form.MULTIFILTER_ENABLE.value += "2";
				document.list_form.MULTIFILTER_MAC.value += ">";
				document.list_form.MULTIFILTER_MAC.value += macAddr;
				document.list_form.MULTIFILTER_DEVICENAME.value += ">";
				document.list_form.MULTIFILTER_DEVICENAME.value += document.getElementById("client_name").value.trim();
				if(isSupport("PC_SCHED_V3"))
					document.list_form.MULTIFILTER_MACFILTER_DAYTIME_V2.value += ">W03E21000700<W04122000800";
				else
					document.list_form.MULTIFILTER_MACFILTER_DAYTIME.value += "><";
			}
		}
	}
	else {//exist rule
		document.list_form.MULTIFILTER_MAC.value.split(">").forEach(function(element, index){
			if(element.indexOf(macAddr) != -1){
				var tmpArray = document.list_form.MULTIFILTER_ENABLE.value.split(">");
				tmpArray[index] = 0;
				if(timeSchedulingFlag)
					tmpArray[index] = 1;
				else if(blockInternetFlag)
					tmpArray[index] = 2;
				document.list_form.MULTIFILTER_ENABLE.value = tmpArray.join(">");
			}
		})
	}
}

function showClientIconList() {
	var confirmFlag = true;
	confirmFlag = confirm(stringSafeGet("<#Client_Icon_overload#>"));
	if(confirmFlag) {
		edit_cancel();
		$("#edit_usericon_block").fadeIn(10);
		cal_panel_block("edit_usericon_block", 0.15);
		showClientIcon();
		document.getElementById("uploadIcon").value = "";
		return false;
	}
	else {
		document.getElementById("uploadIcon").value = "";
		return false;
	}
}

function showClientIcon() {
	genClientList();
	var uploadIconMacList = getUploadIconList().replace(/\.log/g, "");
	var custom_usericon_row = uploadIconMacList.split('>');
	var code = "";
	var clientIcon = "";
	var custom_usericon_length = custom_usericon_row.length;
	code +='<table width="95%" cellspacing="0" cellpadding="4" align="center" class="list_table" id="usericon_table">';
	if(custom_usericon_length == 1) {
		code +='<tr><td style="color:#FFCC00;" colspan="4"><#IPConnection_VSList_Norule#></td></tr>';
		document.getElementById('edit_usericon_block').style.height = "145px";
	}
	else {
		for(var i = 0; i < custom_usericon_length; i += 1) {
			if(custom_usericon_row[i] != "") {
				var formatMac = custom_usericon_row[i].slice(0,2) + ":" + custom_usericon_row[i].slice(2,4) + ":" + custom_usericon_row[i].slice(4,6) + ":" + 
								custom_usericon_row[i].slice(6,8) + ":" + custom_usericon_row[i].slice(8,10)+ ":" + custom_usericon_row[i].slice(10,12);
				code +='<tr id="row' + i + '">';
				var clientObj = clientList[formatMac];
				var clientName = "";
				if(clientObj != undefined) {
					clientName = (clientObj.nickName == "") ? clientObj.name : clientObj.nickName;
				}
				code +='<td width="45%">'+ clientName +'</td>';
				code +='<td width="30%">'+ formatMac +'</td>';
				clientIcon = getUploadIcon(custom_usericon_row[i]);
				code +='<td width="15%"><img id="imgClientIcon_'+ i +'" class="imgClientIcon" src="' + clientIcon + '"</td>';
				code +='<td width="10%"><input class="remove_btn" onclick="delClientIcon(this);" value=""/></td></tr>';
			}
		}
		document.getElementById('edit_usericon_block').style.height = (61 * custom_usericon_length + 50) + "px";
	}
	code +='</table>';
	document.getElementById("usericon_block").innerHTML = code;
}

function delClientIcon(rowdata) {
	var delIdx = rowdata.parentNode.parentNode.rowIndex;
	var delMac = rowdata.parentNode.parentNode.childNodes[1].innerHTML;
	document.getElementById("usericon_table").deleteRow(delIdx);
	custom_usericon_del += delMac + ">";
	var trCount = $( "#usericon_table tr" ).length;
	document.getElementById('edit_usericon_block').style.height = (61 * (trCount + 1) + 50) + "px";
	if(trCount == 0) {
		var code = "";
		code +='<table width="95%" cellspacing="0" cellpadding="4" align="center" class="list_table" id="usericon_table">';
		code +='<tr><td style="color:#FFCC00;" colspan="4"><#IPConnection_VSList_Norule#></td></tr>';
		code +='</table>';
		document.getElementById('edit_usericon_block').style.height = "145px";
		document.getElementById("usericon_block").innerHTML = code;
	}
}

function btUserIconEdit() {
	document.list_form.custom_clientlist.disabled = true;
	document.list_form.dhcp_staticlist.disabled = true;
	document.list_form.custom_usericon.disabled = true;
	document.list_form.custom_usericon_del.disabled = false;
	document.list_form.custom_usericon_del.value = custom_usericon_del.replace(/\:/g, "");

	// submit list_form
	document.list_form.submit();
	document.getElementById("loadingUserIcon").style.display = "";
	setTimeout(function(){
		document.getElementById("statusframe").contentWindow.refreshpage();
		custom_usericon_del = "";
		document.list_form.custom_usericon_del.disabled = true;
	}, document.list_form.action_wait.value * 1000);
}
function btUserIconCancel() {
	custom_usericon_del = "";
	$("#edit_usericon_block").fadeOut(100);
}

function previewImage($obj) {
	var userIconLimitFlag = userIconNumLimit(document.getElementById("macaddr_field").value);

	if(userIconLimitFlag) {	//not over 100
		var checkImageExtension = function (imageFileObject) {
		var  picExtension= /\.(jpg|jpeg|gif|png|bmp|ico)$/i;  //analy extension
			if (picExtension.test(imageFileObject)) 
				return true;
			else
				return false;
		};

		//1.check image extension
		if (!checkImageExtension($obj.val()))
			alert("<#Setting_upload_hint#>");
		else {
			//2.Re-drow image
			var fileReader = new FileReader(); 
			fileReader.onload = function (fileReader) {
				var img = document.createElement("img");
				img.src = fileReader.target.result;
                img.className="clientIcon_no_hover";
				var mimeType = img.src.split(",")[0].split(":")[1].split(";")[0];
                let canvas = document.createElement('canvas');
                canvas.width=85;
                canvas.height=85;
				var ctx = canvas.getContext("2d");
				ctx.clearRect(0,0,85,85);
				$("#client_image").empty();
                $("#client_image").append(img);
				setTimeout(function() {
					ctx.drawImage(img, 0, 0, 85, 85);
					var dataURL = canvas.toDataURL(mimeType);
					userIconBase64 = dataURL;
				}, 100); //for firefox FPS(Frames per Second) issue need delay
			}
			fileReader.readAsDataURL($obj.prop("files")[0]);
			userIconHideFlag = true;
			userUploadFlag = true;
		}
	}
	else {	//over 100 then let usee select delete icon or nothing
		showClientIconList();
	}
}

function updateClientsCount() {
	//When not click iconClient and not click View Client List need update client count.
	var viewlist_obj = document.getElementById("clientlist_viewlist_content");
	if(lastName != "iconClient" && (viewlist_obj == null || (viewlist_obj && viewlist_obj.style.display == "none"))){
		originData.fromNetworkmapd[0] = httpApi.hookGet("get_clientlist", true);
		genClientList();
		show_client_status(totalClientNum.online);
	}
	setTimeout("updateClientsCount();", 5000);
}
function setDefaultIcon() {
	const mac = document.getElementById("macaddr_field").value;
	select_image(clientList[mac], true);
}
function closeClientDetailView() {
	edit_cancel();
}

function cal_panel_block(obj){
	var blockmarginLeft;
	if (window.innerWidth)
		winWidth = window.innerWidth;
	else if ((document.body) && (document.body.clientWidth))
		winWidth = document.body.clientWidth;

	if (document.documentElement  && document.documentElement.clientHeight && document.documentElement.clientWidth){
		winWidth = document.documentElement.clientWidth;
	}

	if(winWidth >1050){
		winPadding = (winWidth-1050)/2;
		winWidth = 1105;
		blockmarginLeft= (winWidth*0.2)+winPadding;
	}
	else if(winWidth <=1050){
		blockmarginLeft= (winWidth)*0.2 + document.body.scrollLeft;
	}

	document.getElementById(obj).style.marginLeft = blockmarginLeft+"px";
}

function hide_notice(){
	$("#notice_div").hide();
	var iframe = document.getElementById("statusframe");
	iframe.contentWindow.document.form.wl0_11ax.value = "0";
	iframe.contentWindow.document.form.wl1_11ax.value = "0";
	iframe.contentWindow.document.form.wl2_11ax.value = "0";
}
function notice_apply(){
	var iframe = document.getElementById("statusframe");
	iframe.contentWindow.document.form.next_page.value = "index.asp";
	iframe.contentWindow.document.form.submit();
}

function close_phone_as_modem_hint(){
	$("#phone_as_modem_div").hide();
	cookie.unset("show_phone_as_modem_hints");
}

function copyDdnsName(e) {
	var ddnsName = decodeURIComponent('<% nvram_char_to_ascii("", "ddns_hostname_x"); %>');
	if (window.isSecureContext && navigator.clipboard) {
		navigator.clipboard.writeText(ddnsName);
	} else {
		const textArea = document.createElement("textarea");
		textArea.value = ddnsName;
		document.body.appendChild(textArea);
		textArea.select();
		try {
			document.execCommand('copy')
		} catch (err) {
			console.error('Unable to copy to clipboard', err)
		}
		document.body.removeChild(textArea)
	}
    let span = $("<span>").addClass("tooltiptext").html($(e).data('title'));
    $(e).parent().append(span);
    span.show().fadeOut(1500, function() { $(this).remove(); });
}

function showClientlistModal(){
    const clientlistModal = new ClientlistModel();
    clientlistModal.show();
}

</script>
</head>

<body onunload="return unload_body();" class="bg">
<noscript>
	<div class="popup_bg" style="visibility:visible; z-index:999;">
		<div style="margin:200px auto; width:300px; background-color:#006699; color:#FFFFFF; line-height:150%; border:3px solid #FFF; padding:5px;"><#not_support_script#></p></div>
	</div>
</noscript>

<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<div id="hiddenMask" class="popup_bg">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
		<tr>
		<td>
			<div class="drword" id="drword"><#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...
				<br>
				<div id="disconnect_hint" style="display:none;"><#Main_alert_proceeding_desc2#></div>
				<br>
		    </div>
			<div id="wireless_client_detect" style="margin-left:10px;position:absolute;display:none;width:400px">
				<img src="images/loading.gif">
				<div style="margin:-55px 0 0 75px;"><#QKSet_Internet_Setup_fail_method1#></div>
			</div> 
			<div class="drImg"><img src="images/alertImg.png"></div>
			<div style="height:100px; "></div>
		</td>
		</tr>
	</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>
<div id="phone_as_modem_div" class="phone_as_modem" style="display: none;">
	<div style="width: 95%; margin-bottom: 20px;">
		<div class="phone_as_modem_top">
			<div><#Mobile_modem_desc#></div>
			<div><img src='/images/button-close.gif' style='width:30px; cursor:pointer' onclick='close_phone_as_modem_hint();'></div>
		</div>
		<div id="phone_as_modem_instructions"></div>
	</div>
</div>

<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0" scrolling="no"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="wl_auth_mode_x" value="<% nvram_get("wl0_auth_mode_x"); %>">
<input type="hidden" name="wl_wep_x" value="<% nvram_get("wl0_wep_x"); %>">
<input type="hidden" name="wl_key_type" value="<% nvram_get("wl_key_type"); %>"><!--Lock Add 1125 for ralink platform-->
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="apps_action" value="">
<input type="hidden" name="apps_path" value="">
<input type="hidden" name="apps_name" value="">
<input type="hidden" name="apps_flag" value="">
<input type="hidden" name="wan_unit" value="<% nvram_get("wan_unit"); %>">
<input type="hidden" name="dual_wan_flag" value="">
</form>
<!-- Start for Editing client list-->
<form method="post" name="list_form" id="list_form" action="/start_apply2.htm" target="hidden_frame">
	<input type="hidden" name="current_page" value="">
	<input type="hidden" name="next_page" value="">
	<input type="hidden" name="modified" value="0">
	<input type="hidden" name="flag" value="background">
	<input type="hidden" name="action_mode" value="apply">
	<input type="hidden" name="action_script" value="saveNvram">
	<input type="hidden" name="action_wait" value="1">
	<input type="hidden" name="custom_clientlist" value="">
	<input type="hidden" name="dhcp_staticlist" value="" disabled>
	<input type="hidden" name="dhcp_static_x" value='<% nvram_get("dhcp_static_x"); %>' disabled>
	<input type="hidden" name="custom_usericon" value="">
	<input type="hidden" name="usericon_mac" value="" disabled>
	<input type="hidden" name="custom_usericon_del" value="" disabled>
	<input type="hidden" name="MULTIFILTER_ALL" value='<% nvram_get("MULTIFILTER_ALL"); %>' disabled>
	<input type="hidden" name="MULTIFILTER_ENABLE" value="" disabled>
	<input type="hidden" name="MULTIFILTER_MAC" value="" disabled>
	<input type="hidden" name="MULTIFILTER_DEVICENAME" value="" disabled>
	<input type="hidden" name="MULTIFILTER_MACFILTER_DAYTIME" value="" disabled>
	<input type="hidden" name="MULTIFILTER_MACFILTER_DAYTIME_V2" value="" disabled>
</form>

<form method="post" name="maclist_form" id="maclist_form" action="/start_apply2.htm" target="hidden_frame">
	<input type="hidden" name="current_page" value="">
	<input type="hidden" name="next_page" value="">
	<input type="hidden" name="modified" value="0">
	<input type="hidden" name="flag" value="">
	<input type="hidden" name="action_mode" value="apply_new">
	<input type="hidden" name="action_script" value="restart_wireless">
	<input type="hidden" name="action_wait" value="5">
	<input type="hidden" name="wl0_maclist_x" value="<% nvram_get("wl0_maclist_x"); %>">
	<input type="hidden" name="wl1_maclist_x" value="<% nvram_get("wl1_maclist_x"); %>">
	<input type="hidden" name="wl2_maclist_x" value="<% nvram_get("wl2_maclist_x"); %>">
	<input type="hidden" name="wl0_macmode" value="deny">
	<input type="hidden" name="wl1_macmode" value="deny">
	<input type="hidden" name="wl2_macmode" value="deny">
</form>
<!-- update Client List -->
<form method="post" name="networkmapdRefresh" action="/apply.cgi" target="hidden_frame">
	<input type="hidden" name="action_mode" value="update_client_list">
	<input type="hidden" name="action_script" value="">
	<input type="hidden" name="action_wait" value="1">
	<input type="hidden" name="current_page" value="httpd_check.xml">
	<input type="hidden" name="next_page" value="httpd_check.xml">
</form>
<!-- stop networkmapd -->
<form method="post" name="stopNetworkmapd" action="/start_apply.htm" target="hidden_frame">
	<input type="hidden" name="action_mode" value="apply">
	<input type="hidden" name="action_script" value="restart_networkmap">
	<input type="hidden" name="action_wait" value="2">
	<input type="hidden" name="current_page" value="">
	<input type="hidden" name="next_page" value="">
	<input type="hidden" name="networkmap_enable" value="<% nvram_get("networkmap_enable"); %>">
</form>
<div id="notice_div" style="width:650px;position:absolute;background: rgb(43, 55, 59);z-index:10;margin-left:300px;border-radius:10px;padding: 20px;display:none">
	<div style="margin: 5px 0 10px 0;font-size: 20px;">[Important]</div>
	<div style="margin: 20px 0;font-size: 16px;">
		802.11AX is the latest WiFi technology on the market, however there are still lots of laptops or desktops WLAN card do not support this latest technology properly, before disabling 802.11Ax compatibility mode, please make sure you have updated all your laptops or desktops WLAN card driver manually(* since most of latest WLAN drivers are not updated over the air) from intel website.
	</div>
	<div style="font-size: 16px;margin: 30px 0 10px 0;">
		<div style="padding: 3px 0">Intel WLAN driver download:</div>
		<a href="https://downloadcenter.intel.com/product/59485/Wireless-Networking" style="text-decoration: underline;">https://downloadcenter.intel.com/product/59485/Wireless-Networking</a>
		<div style="padding: 3px 0">Intel 7260 Driver: 18.33.13.4 or later version will work properly.</div>
	</div>
	<div style="font-size: 16px;margin: 10px 0 10px 0;">
		<div style="padding: 3px 0">How to update WLAN card driver manually: (FAQ: 候補)</div>
		<a href="https://downloadcenter.intel.com/product/59485/Wireless-Networking" style="text-decoration: underline;">https://downloadcenter.intel.com/product/59485/Wireless-Networking</a>
	</div>

	<div style="display:flex;justify-content: center;margin-top:50px;">
		<div style="background: #121C1E;font-size: 16px;padding: 10px 20px;margin: 0 5px;border-radius: 8px;min-width: 80px;text-align: center;cursor:pointer" onclick="hide_notice()"><#CTL_Cancel#></div>
		<div style="background: #121C1E;font-size: 16px;padding: 10px 20px;margin: 0 5px;border-radius: 8px;min-width: 80px;text-align: center;cursor:pointer" onclick="notice_apply();"><#CTL_apply#></div>
	</div>
</div>
<div id="edit_usericon_block" class="contentM_usericon">
	<table width="95%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" style="margin-top:8px;">
		<thead>
			<tr>
				<td colspan="4">Client upload icon&nbsp;(<#List_limit#>&nbsp;100)</td>
			</tr>
		</thead>
		<tr>
			<th width="45%"><#Client_Name#></th>
			<th width="30%"><#MAC_Address#></th>
			<th width="15%"><#Client_Icon#></th>
			<th width="10%"><#CTL_del#></th>
		</tr>
	</table>
	<div id="usericon_block"></div>
	<div style="margin-top:5px;padding-bottom:10px;width:100%;text-align:center;">
		<input class="button_gen" type="button" onclick="btUserIconCancel();" value="<#CTL_Cancel#>">
		<input class="button_gen" type="button" onclick="btUserIconEdit();" value="<#CTL_ok#>">
		<img id="loadingUserIcon" style="margin-left:5px;display:none;" src="/images/InternetScan.gif">
	</div>	
</div>
<div id="edit_client_block" class="contentM_qis">
	<table class="QISform_wireless" border=0 align="center" cellpadding="5" cellspacing="0" style="width:100%;">
		<tr>
			<td colspan="3">
				<div id="divClientState" class="clientState">
					<span id="client_ipMethod" class="ipMethodTag " ></span>
					<span id="client_login" class="ipMethodTag " ></span>
					<span id="client_printer" class="ipMethodTag " ></span>
					<span id="client_iTunes" class="ipMethodTag " ></span>
					<span id="client_opMode" class="ipMethodTag "></span>
					<span id="client_sdnIdx" class="ipMethodTag "></span>
				</div>
				<div id="client_interface" style="height:28px;width:28px;float:right;"></div>
			</td>
		</tr>
		<tr>
			<td colspan="3">
				<div class="clientList_line"></div>
			</td>
		</tr>
		<tr>
			<td style="text-align:center;vertical-align:top;width:85px;">
				<div id="divDropClientImage" class="client_preview_icon" title="Change client icon" onclick="show_custom_image();">
					<div id="client_image" style="width:85px;height:85px;margin:0 auto;"></div>
				</div>
				<div id="changeClientIconControl" class="changeClientIcon">
					<span title="Change to default client icon" class="IE8HACK" onclick="setDefaultIcon();"><#CTL_Default#></span>
					<span id="changeIconTitle" class="IE8HACK" title="Change client icon" style="margin-left:10px;" onclick="show_custom_image();"><#CTL_Change#></span>
				</div>
			</td>
			<td style="vertical-align:top;text-align:center;">
				<div class="clientTitle">
					<#Clientlist_name#>
				</div>
				<div  class="clientTitle" style="margin-top:10px;">
					IP
				</div>
				<div  class="clientTitle" style="margin-top:10px;">
					MAC
				</div>
				<div  class="clientTitle" style="margin-top:10px;">
					<#Clientlist_device#>
				</div>
			</td>
			<td style="vertical-align:top;width:280px;">
				<div>
					<input id="client_name" name="client_name" type="text" value="" class="input_32_table" maxlength="32" style="width:275px;" autocorrect="off" autocapitalize="off">
					<input id="hostname_field" type="hidden" value="">
				</div>
				<div style="margin-top:10px;">
					<input id="ipaddr_field_orig" type="hidden" value="" disabled="">
					<input id="ipaddr_field" type="text" value="" class="input_32_table" style="width:275px;" onkeypress="return validator.isIPAddr(this,event)" autocorrect="off" autocapitalize="off">
				</div>

				<div style="margin-top:10px;">
					<input id="macaddr_field" type="text" value="" class="input_32_table client_input_text_disabled" disabled autocorrect="off" autocapitalize="off">
				</div>
				<div style="margin-top:10px;">
					<input id="manufacturer_field" type="text" value="" class="input_32_table client_input_text_disabled" disabled>
				</div>
			</td>
		</tr>

		<tr>
			<td colspan="3">
				<div class="custom_icon_list_bg"></div>
			</td>
		</tr>
		<tr id="tr_adv_setting">
			<td colspan="3">
				<div class="clientList_line"></div>
				<div style="height:32px;width:100%;margin:5px 0;">
					<div style="width:65%;float:left;line-height:32px;">
						<span onmouseover="return overlib('Enable this button to block this device to access internet.');" onmouseout="return nd();"><#Clientlist_block_internet#></span><!--untranslated-->
					</div>
					<div class="left" style="cursor:pointer;float:right;" id="radio_BlockInternet_enable"></div>
				</div>
				<div class="clientList_line"></div>
				<div id="div_time_scheduling" style="height:32px;width:100%;margin:5px 0;">
					<div style="width:65%;float:left;line-height:32px;">
						<span id="time_scheduling_title" onmouseover='return overlib("<#ParentalCtrl_Desc_TS#>");' onmouseout="return nd();"><#Parental_Control#></span>
					</div>
					<div align="center" class="left" style="cursor:pointer;float:right;" id="radio_TimeScheduling_enable"></div>
					<div id="internetTimeScheduling" class="internetTimeEdit" style="float:right;margin-right:10px;" title="<#Time_Scheduling#>" onclick="redirectTimeScheduling(document.getElementById('macaddr_field').value);" ></div>
				</div>
				<div class="clientList_line"></div>
				<div id="div_ipmac_binding" style="height:32px;width:100%;margin:5px 0;">
					<div style="width:65%;float:left;line-height:32px;">
						<span onmouseover="return overlib('Enable this button to bind specific IP with MAC Address of this device.');" onmouseout="return nd();"><#Clientlist_IPMAC_Binding#></span><!--untranslated-->
					</div>
					<div align="center" class="left" style="cursor:pointer;float:right;" id="radio_IPBinding_enable" ></div>
				</div>
				<div class="clientList_line"></div>
			</td>
		</tr>
		<tr>
			<td colspan="3" style="text-align: center;">
				<input class="button_gen" type="button" onclick="edit_delete();" id="deleteBtn" value="<#CTL_del#>" style="display:none;">
				<input class="button_gen" type="button" id="blockBtn" value="<#Block#>" title="<#block_client#>" style="display:none;">
				<script>
					document.maclist_form.wl0_maclist_x.value = (function(){
						var wl0_maclist_x_array = '<% nvram_get("wl0_maclist_x"); %>'.split("&#60");

						if(wl_info.band5g_support){
							'<% nvram_get("wl1_maclist_x"); %>'.split("&#60").forEach(function(element, index){
								if(wl0_maclist_x_array.indexOf(element) == -1) wl0_maclist_x_array.push(element);
							});
						}

						if(wl_info.band5g_2_support || wl_info.band6g_support){
							'<% nvram_get("wl2_maclist_x"); %>'.split("&#60").forEach(function(element, index){
								if(wl0_maclist_x_array.indexOf(element) == -1) wl0_maclist_x_array.push(element);
							});
						}

						return wl0_maclist_x_array.join("<");
					})();

					document.getElementById("blockBtn").onclick = function(){
						document.maclist_form.wl0_maclist_x.value = document.maclist_form.wl0_maclist_x.value + "<" + document.getElementById("macaddr_field").value;
						document.maclist_form.wl1_maclist_x.value = document.maclist_form.wl0_maclist_x.value;
						document.maclist_form.wl2_maclist_x.value = document.maclist_form.wl0_maclist_x.value;
						document.maclist_form.submit();
						hideEditBlock();
					}
				</script>
				<input class="button_gen" type="button" onclick="closeClientDetailView();" value="<#CTL_Cancel#>">
				<input id="edit_confirm" class="button_gen" type="button" onclick="edit_confirm();" value="<#CTL_apply#>">
				<img id="loadingIcon" style="margin-left:5px;display:none;" src="/images/InternetScan.gif">
			</td>
		</tr>
	</table>
</div>
<!-- End for Editing client list-->	
<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td valign="top" width="17">&nbsp;</td>
		
		<!--=====Beginning of Main Menu=====-->
		<td valign="top" width="204">
			<div id="mainMenu"></div>
			<div id="subMenu"></div>
		</td>
		
		<td align="left" valign="top" class="bgarrow">
		
		<!--=====Beginning of Network Map=====-->
		<div id="tabMenu"></div>
		<div id="NM_shift" style="margin-top:-140px;"></div>
		<div id="NM_table" class="NM_table" >
		<div id="NM_table_div">
			<div style="width:50%;float:left;">
			<table id="_NM_table" border="0" cellpadding="0" cellspacing="0" height="720" style="opacity:.95;" >
				<tr>
					<td width="40px" rowspan="11" valign="center"></td>
					<!--== Dual WAN ==-->
					<td id="primary_wan_icon" width="160px;" align="center" class="NM_radius" valign="middle" bgcolor="#444f53" onclick="showstausframe('Internet_primary');" style="display:none;height:180px">
						<a href="/device-map/internet.asp" target="statusframe"><div id="iconInternet_primary" onclick="clickEvent(this);"></div></a>
						<div id="first_wan_title"><#dualwan_primary#>:</div>
						<div id="primary_pap_concurrent" style="display:none">
							<div style="padding: 3px 0">2.4 GHz Parent-AP</div>
							<div id="speed_info_primary" style="display:none">Link Rate:</div>
							<div id="rssi_info_primary" style="display:none">RSSI:</div>
						</div>
						<div style="padding:5px"><strong id="primary_status"></strong></div>
					</td>
					<td id="dual_wan_gap" width="40px" style="display:none">
					</td>
					<td id="secondary_wan_icon" width="160px;" align="center" class="NM_radius" valign="middle" bgcolor="#444f53" onclick="showstausframe('Internet_secondary');" style="display:none;height:180px">
						<a href="/device-map/internet.asp" target="statusframe"><div id="iconInternet_secondary" onclick="clickEvent(this);"></div></a>
						<div id="second_wan_title"><#dualwan_secondary#>:</div>
						<div id="secondary_pap_concurrent" style="display:none">
							<div style="padding: 3px 0">5 GHz Parent-AP</div>
							<div id="speed_info_secondary" style="display:none">Link Rate:</div>
							<div id="rssi_info_secondary" style="display:none">RSSI:</div>
						</div>
						<div style="padding:5px"><strong id="secondary_status"></strong></div>
					</td>
					<!--== single WAN ==-->
					<td id="single_wan_icon" align="right" class="NM_radius_left" valign="middle" bgcolor="#444f53" onclick="showstausframe('Internet');">
						<a href="/device-map/internet.asp" target="statusframe"><div id="iconInternet" onclick="clickEvent(this);"></div></a>
					</td>
					<td id="single_wan_status" colspan="2" valign="middle" bgcolor="#444f53" class="NM_radius_right" onclick="" style="padding:5px;cursor:auto;width:180px;height:130px">
						<div>
							<div id="NM_connect_title" style="font-size:12px;font-family: Verdana, Arial, Helvetica, sans-serif;"><#statusTitle_Internet#>:</div>
							<div id="NM_connect_status" class="index_status" style="font-size:14px;"><#QIS_step2#>...</div>
						</div>
						<div id="wanIP_div" style="margin-top:5px;">
							<span style="font-size:12px;font-family: Verdana, Arial, Helvetica, sans-serif;">WAN IP:</span>
							<strong id="index_status" class="index_status" style="font-size:14px;"></strong>
						</div>
						<div id="wanIP_ipv6_div" style="margin-top:5px;display:none;">
							<span style="font-size:12px;font-family: Verdana, Arial, Helvetica, sans-serif;"><#IPv6_wan_addr#>:</span><br>
							<strong id="index_ipv6_status" class="index_status" style="font-size:11px;display:none;"></strong><br>
							<strong id="index_ipv6_ll_status" class="index_status" style="font-size:11px;display:none;"></strong>
						</div>
						<div id="ddnsHostName_div" style="margin-top:5px;word-break:break-all;word-wrap:break-word;width:180px; display: inline-flex; align-items: center; gap: 3px; flex-direction: row; flex-wrap: wrap;">
							<span style="font-size:12px;font-family: Verdana, Arial, Helvetica, sans-serif;">DDNS:</span>
							<strong id="ddnsHostName" class="index_status" style="font-size:14px;"><#QIS_detectWAN_desc2#></strong>
							<div class="icon-group-center">
                                <div id="copyDdns" class="tooltip"><a onClick="copyDdnsName(this)" data-toggle="tooltip" data-title="Copied!"><i class="icon-clone"></i></a></div>
                                <span id="ddns_fail_hint" class="notificationoff" onClick="show_ddns_fail_hint();" onMouseOut="nd();"></span>
                                <span><img id="le_icon" title="Let's Encrypt" src="images/New_ui/networkmap/LE_badge_color.svg" style="width:25px; height:25px;"></span>
							</div>
						</div>
						<div id="wlc_band_div" style="margin-top:5px;display:none">
							<span style="font-size:14px;font-family: Verdana, Arial, Helvetica, sans-serif;"><#Interface#>:</span>
							<strong id="wlc_band_status" class="index_status" style="font-size:14px;"></strong>
						</div>
						<div id="dataRate_div" style="margin-top:5px;display:none">
							<span style="font-size:14px;font-family: Verdana, Arial, Helvetica, sans-serif;">Link rate:</span>
							<strong id="speed_status" class="index_status" style="font-size:14px;"></strong>
						</div>
						<div id="rssi_div" style="margin-top:5px;display:none">
							<span style="font-size:14px;font-family: Verdana, Arial, Helvetica, sans-serif;">RSSI:</span>
							<strong id="rssi_status" class="index_status" style="font-size:14px;"></strong>
						</div>
						<div id="rssi_mlo_div" style="margin-top:5px;display:none">
						</div>

						<div id="wanAggr_div" style="margin-top:5px;display:none;">
							<span style="font-size:14px;font-family: Verdana, Arial, Helvetica, sans-serif; color: #FFCC00;">WAN Aggregation:</span>
							<strong id="wan_bonding_status" class="index_status" style="font-size:14px;"></strong>
						</div>
					</td>
				</tr>
				<tr>
					<!--==line of dual wan==-->
					<td id="primary_wan_line"  height="35px" style="display:none;">
						<div id="primary_line" class="primary_wan_connected"></div>
					</td>
					<td id="secondary_wan_line" colspan="2" height="35px"  style="display:none;">
						<div id="secondary_line" class="secondary_wan_connected"></div>
					</td>
					<!--==line of single wan==-->
					<td id="single_wan_line" colspan="3" align="center" height="35px">
						<div id="single_wan" class="single_wan_connected"></div>
					</td>
				</tr>			
				<tr>
					<td align="right" bgcolor="#444f53" class="NM_radius_left" onclick="showstausframe('Router');" style="height:150px">
						<a id="iconRouterLink" href="device-map/router.asp" target="statusframe"><div id="iconRouter" onclick="clickEvent(this);"></div></a>
					</td>
					<td colspan="2" valign="middle" bgcolor="#444f53" class="NM_radius_right" onclick="showstausframe('Router');">
						<div>
						<span id="SmartConnectName" style="font-size:14px;font-family: Verdana, Arial, Helvetica, sans-serif; display:none">Smart Connect Status: </span>
						</div>
						<div>
						<strong id="SmartConnectStatus" class="index_status" style="font-size:14px; display:none"><a style="color:#FFF;text-decoration:underline;" href="/
						Advanced_Wireless_Content.asp">On</a></strong>
						</div>

						<div id="wlSecurityContext">
							<span style="font-size:14px;font-family: Verdana, Arial, Helvetica, sans-serif;"><#Security#>: </span>
							<br/>  
							<strong id="wl_securitylevel_span" class="index_status"></strong>
							<img id="iflock">
						</div>

						<div id="mbModeContext" style="display:none">
							<span style="font-size:14px;font-family: Verdana, Arial, Helvetica, sans-serif;"><#menu5_6_1#>: </span>
							<br/>
							<br/>
							<strong class="index_status"><#OP_MB_item#></strong>
						</div>
					</td>
				</tr>			
				<tr>
					<td id="line3_td" colspan="3" align="center" height="35px">
						<img id="line3_img" src="/images/New_ui/networkmap/line_two.png" style="margin-top:-5px;">
						<div id="line3_single" class="single_wan_connected" style="display:none"></div>
					</td>
				</tr>
				<tr>
					<td id="clients_td" width="150" bgcolor="#444f53" align="center" valign="top" class="NM_radius" style="padding-bottom:15px;">
						<div class="block_all_icon"><div><#Blocked#></div></div>
						<script type="text/javascript">
							$(".block_all_icon").unbind("click").click(function(e){
								e = e || event;
								e.stopPropagation();
								block_all_device_hint();
							});
						</script>
						<div id="clientsContainer" onclick="showstausframe('Client');">
							<a id="clientStatusLink" href="device-map/clients.asp" target="statusframe">
							<div id="iconClient" style="margin-top:20px;" onclick="clickEvent(this);"></div>
							</a>
							<div class="clients" id="clientNumber" style="cursor:pointer;"></div>
						</div>

						<input type="button" class="button_gen" value="<#View_List#>" style="margin-top:15px;" onClick="showClientlistModal()">

						<div id="networkmap_switch" style="display:none">
							<div align="center" class="left" style="width:94px; cursor:pointer;margin-top:10px" id="networkmap_enable_t"></div>
							<div class="iphone_switch_container" style="height:32px; width:74px; position: relative; overflow: hidden">
								<script type="text/javascript">
									$('#networkmap_enable_t').iphoneSwitch('<% nvram_get("networkmap_enable"); %>',
										function(){
											document.stopNetworkmapd.networkmap_enable.value = 1;
											document.stopNetworkmapd.submit()
										},
										function(){
											document.stopNetworkmapd.networkmap_enable.value = 0;
											document.stopNetworkmapd.submit()
										}
									);
								</script>			
							</div>
						</div>
						<div id="ameshContainer" onclick="showstausframe('AMesh');"></div>
					</td>

					<td width="36" rowspan="6" id="clientspace_td"></td>

					<td id="usb_td" width="160" bgcolor="#444f53" align="center" valign="top" class="NM_radius" style="padding-bottom:5px;min-height:420px;">
					</td>
				</tr>
			</table>
			</div>
			<div style="width:50%;float:left;">
			<table id="_NM_table" border="0" cellpadding="0" cellspacing="0" style="opacity:.95;">
				<tr>
					<td valign="top">
						<div class="statusTitle" id="statusTitle_NM">
							<div id="helpname" style="padding-top:10px;font-size:16px;"></div>
						</div>
						<div class="NM_radius_bottom_container">
							<iframe id="statusframe" class="NM_radius_bottom" style="display:none;margin-left:0px;height:760px;width:320px;\9" name="statusframe" frameborder="0"></iframe>
						</div>
						<script>
							(function(){
								const defaultRouterFrame = "/device-map/router_status.asp";
								document.getElementById("iconRouterLink").href = defaultRouterFrame;
								setTimeout(function(){
									const statusframe_src = isSwMode("RP") ? `/device-map/internet.asp` : defaultRouterFrame;
									$('#statusframe').attr('src', statusframe_src).show();
									const get_header_info = httpApi.hookGet("get_header_info");
									const domain = `${get_header_info.protocol}://${get_header_info.host}`;
									const domain_w_port = `${get_header_info.protocol}://${get_header_info.host}:${get_header_info.port}`;

									let messageTimeout;
									messageTimeout = setTimeout(() => {
										if($('#statusframe').attr('src') === statusframe_src)
											$('#statusframe').attr('src', statusframe_src);
									}, 5000);

									window.addEventListener('message', function(event){
										const msg_page = isSwMode("RP") ? `internet.asp` : `router${isSupport("sdn_mainfh")?"_status":""}.asp`;
										if(event.data == msg_page){
											const has_port = /:\d+$/.test(event.origin);
											if(has_port){
												if(event.origin !== domain_w_port){
													return;
												}
											}
											else{
												if(event.origin !== domain){
													return;
												}
											}
											clearTimeout(messageTimeout);
										}
									});
								}, 1);

								$("#statusframe").show()
									.off('load').on("load", function(){
										const $statusframe = $(this);
										$statusframe.show();
										$statusframe[0].contentWindow.onbeforeunload = function(){
											$statusframe.hide();
										};
									});
							})()
						</script>
					</td>
				</tr>
			</table>
			</div>
			<div style="clear:both;"></div>
		</div>
	</div>
<!--==============Ending of hint content=============-->
  </tr>
</table>
<div id="navtxt" class="navtext" style="position:absolute; top:50px; left:-100px; visibility:hidden; font-family:Arial, Verdana"></div>
<div id="footer"></div>
<script>
	if(flag == "Internet" || flag == "Client")
		document.getElementById("statusframe").src = "";
	initial();
</script>
</body>
</html>
