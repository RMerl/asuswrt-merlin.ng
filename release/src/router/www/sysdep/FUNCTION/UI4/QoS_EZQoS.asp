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
<title><#Web_Title#> - <#EZQoS#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="usp_style.css">
<link rel="stylesheet" type="text/css" href="device-map/device-map.css">
<link rel="stylesheet" type="text/css" href="css/icon.css">
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/calendar/jquery-ui.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" src="/form.js"></script>
<script type="text/javascript" src="client_function.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/asus_policy.js?v=4"></script>
<style>
*{
	box-sizing: border-box;
}
.QISform_wireless{
	width:600px;
	font-size:12px;
	color:#FFFFFF;
	margin-top:10px;
	*margin-left:10px;
}

.QISform_wireless th{
	padding-left:10px;
	*padding-left:30px;
	font-size:12px;
	font-weight:bolder;
	color: #FFFFFF;
	text-align:left;
}

#priority_panel{
	width:740px;
	margin-top:55px;
	margin-left:5px;
	position:absolute;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius: 5px;
	z-index:200;
	background-color:#2B373B;
	box-shadow: 3px 3px 10px #000;
	display:none;
	/*behavior: url(/PIE.htc);*/
}

.description_down{
	margin-top:10px;
	margin-left:10px;
	padding-left:5px;
	font-weight:bold;
	line-height:140%;
	color:#ffffff;
}

#category_list {
	width: 99%;
	height: 590px;
}

#category_list div{
	width:85%;
	height:70px;
	background-color:#7B7B7B;
	color:white;
	margin:15px auto;
	text-align:center;
	border-radius:20px;
	line-height:75px;
	font-family: Arial, Helvetica, sans-serif;
	font-size:16px;
	font-weight:bold;
	box-shadow: 5px 5px 10px 0px black
}

#category_list div:hover{
	background-color:#66B3FF;
	color:#000;
	cursor:pointer;
	cursor:hand;
}

.priority{
	text-align:center;
	font-size:16px;
	font-weight:bold;
	color:#FC0;
}

.priority_highest{
	margin:10px 0px -10px 0px;
}

.priority_lowest{
	margin-bottom: 10px;
}

.Quick_Setup_title{
	font-size: 16px;
	font-weight: bold;
	text-align: center;
}

#Game_act, #Game:hover,
#Media_act, #Media:hover,
#Web_act, #Web:hover,
#eLearning_act, #eLearning:hover,
#videoConference_act, #videoConference:hover,
#Customize_act, #Customize:hover{
	width: 96px;
	height: 96px;
}

#Game,
#Media,
#Web,
#eLearning,
#videoConference,
#Customize{
	width: 84px;
	height: 84px;	
}

#Game, #Game_act, #Game:hover,
#Media, #Media_act, #Media:hover,
#Web, #Web_act, #Web:hover,
#eLearning, #eLearning_act, #eLearning:hover,
#videoConference, #videoConference_act, #videoConference:hover,
#Customize, #Customize_act, #Customize:hover{
	background-size: 100%;
	background-repeat: no-repeat;
	margin: auto;
}

#Game{
	background-image:url('images/New_ui/QoS_quick/game.svg');
}

#Game_act, #Game:hover{
	background-image:url('images/New_ui/QoS_quick/game_act.svg');
}

#Media{
	background-image:url('images/New_ui/QoS_quick/media.svg');
}

#Media_act, #Media:hover{
	background-image:url('images/New_ui/QoS_quick/media_act.svg');
}

#Web{
	background-image:url('images/New_ui/QoS_quick/web.svg');
}

#Web_act, #Web:hover{
	background-image:url('images/New_ui/QoS_quick/web_act.svg');
}

#eLearning{
	background-image:url('images/New_ui/QoS_quick/elearning.svg');
}

#eLearning_act, #eLearning:hover{
	background-image:url('images/New_ui/QoS_quick/elearning_act.svg');	
}

#videoConference{
	background-image:url('images/New_ui/QoS_quick/video_conference.svg');
}

#videoConference_act, #videoConference:hover{
	background-image:url('images/New_ui/QoS_quick/video_conference_act.svg');
}

#Customize{
	background-image:url('images/New_ui/QoS_quick/customize.svg');
}

#Customize_act, #Customize:hover{;
	background-image:url('images/New_ui/QoS_quick/customize_act.svg');
}


.bandwidth_setting{
	display:inline-block;
	min-width: 90px;
	text-align:center;
	border:1px solid #279FD9;
	height:25px;
	line-height:25px;
	padding: 0 5px;
}

.bandwidth_setting_left{
	border-top-left-radius: 5px;
	border-bottom-left-radius: 5px;
}

.bandwidth_setting_right{
	border-top-right-radius: 5px;
	border-bottom-right-radius: 5px;
	margin-left: -3px;
}
.bandwidth_setting_central{
	margin-left: -3px;
}

.menu:checked + label{
	background: #279FD9;
}

.cancel{
	border: 2px solid #898989;
	border-radius:50%;
	background-color: #898989;
}
.check{
	border: 2px solid #093;
	border-radius:50%;
	background-color: #093;
}
.cancel, .check{
	transition: .35s;
}
.cancel:active, .check:active{
  transform: scale(1.5,1.5);
  opacity: 0.5;
  transition: 0;
}
.all_enable{
	border: 1px solid #393;
	color: #393;
}
.all_disable{
	border: 1px solid #999;
	color: #999;
}
</style>
<script>
// WAN
<% wanlink(); %>
<% first_wanlink(); %>
<% secondary_wanlink(); %>

	var wans_dualwan_orig = '<% nvram_get("wans_dualwan"); %>';
	var dualwan_type = wans_dualwan_orig.split(" ");
	var wans_flag = (wans_dualwan_orig.search("none") != -1 || !parent.dualWAN_support) ? 0 : 1;
if(dualwan_type.length == 2){	
	var dualwan_primary = dualwan_type[0].toUpperCase();
	var dualwan_primary_display = (dualwan_primary=="LAN") ? "Ethernet LAN":dualwan_primary;	
	var dualwan_secondary = dualwan_type[1].toUpperCase();
	var dualwan_secondary_display = (dualwan_secondary=="LAN") ? "Ethernet LAN":dualwan_secondary;
	var dw_primary = dualwan_type[0].toLowerCase();
	var dw_secondary = dualwan_type[1].toLowerCase();
}
	var wans_lanport_orig = httpApi.nvramGet(["wans_lanport"]).wans_lanport;
	var dsllink_statusstr = "";
	var dsllink_statusstr_secondary = "";
	if(wans_flag == 1){	//dual_wan enabled
		dsllink_statusstr = first_wanlink_statusstr();
		dsllink_statusstr_secondary = secondary_wanlink_statusstr();
	}
	else
		dsllink_statusstr = wanlink_statusstr();

var dsl_DataRateUp = parseInt("<% nvram_get("dsllog_datarateup"); %>");			//connection UL speed
var dsl_DataRateDown = parseInt("<% nvram_get("dsllog_dataratedown"); %>");		//connection DL speed

//HND_ROUTER HW NAT (fc_disable/runner_disable) ON: 0/0 ; OFF: 1/1
var fc_disable_orig = '<% nvram_get("fc_disable"); %>';
var runner_disable_orig = '<% nvram_get("runner_disable"); %>';

var qos_xobw_orig = parseInt(httpApi.nvramGet(["qos_xobw"], true).qos_xobw);	//shapping UL speed /PrimaryWAN
var qos_xobw1_orig = parseInt(httpApi.nvramGet(["qos_xobw1"], true).qos_xobw1);	//shapping UL speed /SecondaryWAN

var bwdpi_app_rulelist = "<% nvram_get("bwdpi_app_rulelist"); %>".replace(/&#60/g, "<");
var category_title = ["", "<#Adaptive_Game#>", "<#Adaptive_Stream#>", "<#Adaptive_Message#>", "<#Adaptive_WebSurf#>","<#Adaptive_FileTransfer#>", "<#Adaptive_Others#>", "<#Adaptive_eLearning#>"];
var cat_id_array = [[9,20], [8], [4], [0,5,6,15,17], [13,24], [1,3,14], [7,10,11,21,23], [4,13]];
var ctf_disable = '<% nvram_get("ctf_disable"); %>';
var ctf_disable_force  = '<% nvram_get("ctf_disable_force "); %>';
var ctf_fa_mode = '<% nvram_get("ctf_fa_mode"); %>';
var qos_bw_rulelist = "<% nvram_get("qos_bw_rulelist"); %>".replace(/&#62/g, ">").replace(/&#60/g, "<");
var select_all_checked = 0;

var faq_href = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=110";

if(geforceNow_support){
	var orig_nvgfn_enable = httpApi.nvramGet(["nvgfn_enable"], true).nvgfn_enable;
}

function show_up_down(value){
	if(value){
		document.getElementById('upload_tr').style.display = "";
		document.getElementById('download_tr').style.display = "";
		if(mtwancfg_support){
			document.getElementById('wan_1_tr').style.display = "";
			if(dualwan_primary_display=="Ethernet LAN"){
				$("#dualwan_primary_title").html("<#dualwan_primary#> - "+dualwan_primary_display+" (LAN Port "+wans_lanport_orig+")");
			}
			else{
				$("#dualwan_primary_title").html("<#dualwan_primary#> - "+dualwan_primary_display);
			}
			if(dw_secondary == "none"){
				document.getElementById('wan_2_tr').style.display = "none";
				document.getElementById('upload2_tr').style.display = "none";
				document.getElementById('download2_tr').style.display = "none";
			}else{
				document.getElementById('wan_2_tr').style.display = "";
				if(dualwan_secondary_display=="Ethernet LAN"){
					$("#dualwan_secondary_title").html("<#dualwan_secondary#> - "+dualwan_secondary_display+" (LAN Port "+wans_lanport_orig+")");
				}
				else{
					$("#dualwan_secondary_title").html("<#dualwan_secondary#> - "+dualwan_secondary_display);
				}
				document.getElementById('upload2_tr').style.display = "";
				document.getElementById('download2_tr').style.display = "";
			}
		}
	}
	else {
		document.getElementById('upload_tr').style.display = "none";
		document.getElementById('download_tr').style.display = "none";
		if(mtwancfg_support){
			document.getElementById('wan_1_tr').style.display = "none";
			document.getElementById('wan_2_tr').style.display = "none";
			document.getElementById('upload2_tr').style.display = "none";
			document.getElementById('download2_tr').style.display = "none";
		}
	}
}

var GN_with_BandwidthLimeter = false;
if(wl_info.band2g_support != '-1'){
	var gn_array_2g_length = gn_array_2g.length;
	for(var i=0;i<gn_array_2g_length;i++){
		if(gn_array_2g[i][18] == "1"){	//GN with Bandwidth Limiter
			GN_with_BandwidthLimeter = true;
		}
	}
}
if(wl_info.band5g_support != '-1'){
	var gn_array_5g_length = gn_array_5g.length;
	for(var j=0;j<gn_array_5g_length;j++){
		if(gn_array_5g[j][18] == "1"){	//GN with Bandwidth Limiter
			GN_with_BandwidthLimeter = true;
		}
	}
}
if(wl_info.band5g_2_support != '-1' || wl_info.band6g_support != '-1'){
	var gn_array_5g_2_length = gn_array_5g_2.length;
	for(var k=0;k<gn_array_5g_2_length;k++){
		if(gn_array_5g_2[k][18] == "1"){	//GN with Bandwidth Limiter
			GN_with_BandwidthLimeter = true;
		}
	}
}
var GN_with_Amazon_WSS_enabled = (amazon_wss_support && ((httpApi.amazon_wss.getStatue(0, 2) == "1") ? true : false));

if(pm_support) {
	var device_list = [<% pms_device_info(); %>][0];
	var group_list = [<% pms_devgroup_info(); %>][0];
	var device_option_array = [
	["0", ""],["1", "Windows device"], ["2", "Router"], ["3", ""], ["4", "NAS/Server"], ["5", "IP Cam"], ["6", "MacBook"], ["7", "Game Console"], ["8", ""], ["9", "Android Phone"],
	["10", "iPhone"], ["11", "Apple TV"], ["12", "Set-Top Box"], ["13", ""], ["14", "iMac"], ["15", "ROG"], ["16", ""], ["17", ""], ["18", "Printer"], ["19", "Windows Phone"], ["20", "Android Tablet"],
	["21", "iPad"], ["22", "Linux Device"], ["23", "Smart TV"], ["24", "Repeater"], ["25", "Kindle"], ["26", "Scanner"], ["27", "Chromecast"], ["28", "ASUS Smartphone"],
	["29", "ASUS Pad"], ["30", "Windows"], ["31", "Android"], ["32", "Mac OS"]
	];

	var device_type_array = new Array();
	for(i=0;i<device_option_array.length;i++){
		device_type_array.push(device_option_array[i][0]);
		device_type_array[device_option_array[i][0]] = {
			number: device_option_array[i][0],
			name: device_option_array[i][1]
		}
	}

	var info = new Object();
	info.device = new Array();
	info.group = new Array();
}

function initial(){
	show_menu();
	document.getElementById("faq").href=faq_href;

	if(downsize_4m_support || downsize_8m_support)
		document.getElementById("guest_image").parentNode.style.display = "none";

	if((document.form.qos_ibw.value == "0" || document.form.qos_ibw.value == "")&& (document.form.qos_obw.value == "0" || document.form.qos_obw.value == "")){
		document.getElementById("auto").checked = true;
	}
	else{
		document.getElementById("manu").checked = true;
	}

	var qos_type = document.form.qos_type.value;
	if(document.form.qos_enable_orig.value == "1"){
		change_qos_type(qos_type);

		document.getElementById('qos_type_tr').style.display = "";
		if(adaptiveqos_support){
			document.getElementById('int_type').style.display = "";
			document.getElementById('int_type_link').style.display = "";
			change_qos_type(document.form.qos_type_orig.value);
		}
		else
			show_settings("NonAdaptive");
	}
	else{	//qos disabled
		document.getElementById('settingSelection').style.display = "none";
		show_up_down(0);
		document.getElementById('qos_type_tr').style.display = "none";
		if(adaptiveqos_support){
			document.getElementById('int_type').style.display = "";
			document.getElementById('int_type_link').style.display = "";
			document.getElementById('bandwidth_setting_tr').style.display = "none";
			show_settings("NonAdaptive");
		}
	}

	if(adaptiveqos_support){
		document.getElementById('content_title').innerHTML = "<#menu5_3_2#> - <#Adaptive_QoS_Conf#>";
		if(document.form.qos_enable.value == "1"){
			if(qos_type == 0){              //Traditional Type
				add_option(document.getElementById("settingSelection"), '<#qos_user_rules#>', 3, 0);
				add_option(document.getElementById("settingSelection"), '<#qos_user_prio#>', 4, 0);
			}
			else{		//Adaptive Type or else
				document.getElementById('settingSelection').style.display = "none";
			}
		}
		else{		// hide select option if qos disable
			document.getElementById('settingSelection').style.display = "none";
		}
	}
	else{
		if(qos_type == 0){		//Traditional Type
			add_option(document.getElementById("settingSelection"), '<#qos_user_rules#>', 3, 0);
			add_option(document.getElementById("settingSelection"), '<#qos_user_prio#>', 4, 0);
		}
		else{	//Bandwidth Limiter
			document.getElementById('settingSelection').style.display = "none";
		}

		document.getElementById('content_title').innerHTML = "<#Menu_TrafficManager#> - <#menu5_3_2#>";
		document.getElementById('function_int_desc').style.display = "none";
	}

	/* MODELDEP */
	if(based_modelid == "RT-AC85U" || based_modelid == "RT-AC85P" || based_modelid == "RT-AC65U"){
		if(document.form.qos_type_orig.value == "1"){
			document.getElementById('bandwidth_setting_tr').style.display = "none";
			document.form.qos_type_radio[1].checked = true;
		}
		document.getElementById('function_int_desc').style.display = "none";
		document.getElementById('int_type').style.display = "none";
		document.getElementById('int_type_link').style.display = "none";
		show_settings("NonAdaptive");
	}
	

	if(pm_support) {
		collect_info();
		generate_group_list();
	}
	init_changeScale();

	if(geforceNow_support){
		document.getElementById("GeForce_upnp").style.display = "";
		document.getElementById("GeForceNow_item").style.display = "";
	}

	if((isFirefox || isOpera) && document.getElementById("FormTitle"))
		document.getElementById("FormTitle").className = "FormTitle";

	if(!adaptiveqos_support){
		$('#qos_desc').html('<#EzQoS_desc_QoS#>');
		$('label[for="trad_type"]').html('<#EzQoS_type_QoS#>')
		$('#bandwidth_setting_tr').hide();
	}
}

function device_object(name, mac, type, type_name, description, group_array){
	this.name = name;
	this.mac = mac;
	this.type = type;
	this.type_name = type_name;
	this.description = description;
	this.group = group_array;
}

function device_group_object(active, name, description, device_array){
	this.active = (active == 1) ? true : false;
	this.name = name;
	this.description = description;
	this.members = device_array;
}

function collect_info(){
	info.group = [];
	info.device = [];

	//collect group info
	for(i=0;i<group_list.length;i++){
		var object = group_list[i];
		var group_index = "_" + object.name;
		var group_active  = object.active;
		var group_name = object.name;
		var group_description = object.desc;
		var device_array = new Array();
		device_array = object.owned_device;

		info.group.push(group_index);
		info.group[group_index] = new device_group_object(group_active, group_name, group_description, device_array);
	}

	//colletc device info
	for(i=0;i<device_list.length;i++){
		var object = device_list[i];
		var device_index = object.mac;
		var device_name = object.name;
		var device_mac = object.mac;
		var device_type = object.devtype;
		var device_type_name = device_type_array[device_type].name;
		var device_description = object.desc;
		var group_array = new Array();
		group_array = object.device_group;

		info.device.push(device_index);
		info.device[device_index] = new device_object(device_name, device_mac, device_type, device_type_name, device_description, group_array);
	}
}

function init_changeScale(){
	var upload = document.form.qos_obw.value;
	var download = document.form.qos_ibw.value;

	if(mtwancfg_support){
		if(dw_primary == "dsl"
		&& ((upload == "" || upload == "0") && (download == "" || download == "0"))
		&& qos_xobw_orig > 0
		){
			document.form.obw.value = (qos_xobw_orig/1024).toFixed(2);
			document.form.ibw.value = (dsl_DataRateDown/1024).toFixed(2);
		}
		else if(dw_primary == "dsl" && dsllink_statusstr == "Connected"
		&& ((upload == "" || upload == "0") && (download == "" || download == "0"))
		&& dsl_DataRateUp > 0
		){

			document.form.obw.value = (dsl_DataRateUp/1024).toFixed(2);
			document.form.ibw.value = (dsl_DataRateDown/1024).toFixed(2);
		}
		else{
	
			document.form.obw.value = (upload/1024).toFixed(2);
			document.form.ibw.value = (download/1024).toFixed(2);
		}
	}
	else{

		document.form.obw.value = (upload/1024).toFixed(2);
		document.form.ibw.value = (download/1024).toFixed(2);
	}

	if(mtwancfg_support &&  wans_flag == "1") {
		var upload1 = document.form.qos_obw1.value;
		var download1 = document.form.qos_ibw1.value;

		if(dw_secondary == "dsl"
		&& ((upload1 == "" || upload1 == "0") && (download1 == "" || download1 == "0"))
		&& qos_xobw1_orig > 0
		){
			document.form.obw1.value = (qos_xobw1_orig/1024).toFixed(2);
			document.form.ibw1.value = (dsl_DataRateDown/1024).toFixed(2);
		}
		else if(dw_secondary == "dsl" && dsllink_statusstr_secondary == "Connected"
		&& ((upload1 == "" || upload1 == "0") && (download1 == "" || download1 == "0"))
		&& dsl_DataRateUp > 0
		){

			document.form.obw1.value = (dsl_DataRateUp/1024).toFixed(2);
			document.form.ibw1.value = (dsl_DataRateDown/1024).toFixed(2);
		}
		else{
			document.form.obw1.value = (upload1/1024).toFixed(2);
			document.form.ibw1.value = (download1/1024).toFixed(2);
		}
	}
}

function switchPage(page){
	if(page == "1")
		location.href = "/QoS_EZQoS.asp";
	else if(page == "2")
		location.href = "/AdaptiveQoS_Adaptive.asp";	//remove 2015.07
	else if(page == "3")
		location.href = "/Advanced_QOSUserRules_Content.asp";
	else if(page == "4")
		location.href = "/Advanced_QOSUserPrio_Content.asp";
	else
		return false;
}

function validForm(){
	var error_obw=0;
	var error_obw1=0;
	var error_ibw=0;
	var error_ibw1=0;
	if(document.form.qos_enable.value == 0 && document.form.qos_enable_orig.value == 0){
		if(geforceNow_support){
			if(document.form.nvgfn_enable.value == orig_nvgfn_enable){
				return false;
			}
		}
		else{
			return false;
		}
	}

	if(document.form.qos_enable.value == "1"){
		var qos_type = document.form.qos_type.value;
		if(qos_type == 1) {
			if(!reset_wan_to_fo.check_status())
				return false;
		}

		if(qos_type != 2){	//not Bandwidth Limiter

			if( ((qos_type == 1 && document.form.bw_setting_name[1].checked == true ) || qos_type == 0 || qos_type == 3) && (document.form.obw.value.length == 0 || document.form.obw.value == 0)){		// To check field is 0 && Traditional QoS
				alert("<#QoS_invalid_zero#>");
				error_obw++;

			}
			else if( ((qos_type == 1 && document.form.bw_setting_name[1].checked == true ) || qos_type == 0 || qos_type == 3) && !validator.rangeFloat(document.form.obw, 0, 9999999999, "")){
				error_obw++;
			}

			if(error_obw > 0){
				if(mtwancfg_support){
					if(qos_xobw_orig > 0){
						document.form.obw.value = (qos_xobw_orig/1024).toFixed(2);
					}
					else if(dw_primary == "dsl" && dsllink_statusstr == "Connected" && dsl_DataRateUp > 0){
						document.form.obw.value = (dsl_DataRateUp/1024).toFixed(2);
					}
					else if(dw_primary == "dsl"){
						document.form.obw.value = "100";
					}
					else{
						document.form.obw.value = "1000";	
					}
				}
				else{

					if(qos_xobw_orig > 0){
						document.form.obw.value = (qos_xobw_orig/1024).toFixed(2);
					}
					else{
						document.form.obw.value = "1000";       
					}
				}
				return false;
			}

			if( ((qos_type == 1 && document.form.bw_setting_name[1].checked == true ) || qos_type == 0 || qos_type == 3) && (document.form.ibw.value.length == 0 || document.form.ibw.value == 0)){		// To check field is 0 && Traditional QoS
				alert("<#QoS_invalid_zero#>");
				error_ibw++;
			}
			else if( ((qos_type == 1 && document.form.bw_setting_name[1].checked == true ) || qos_type == 0 || qos_type == 3) && !validator.rangeFloat(document.form.ibw, 0, 9999999999, "")){
				error_ibw++;
			}

			if(error_ibw > 0){
				if(mtwancfg_support){
					if(dw_primary == "dsl"  && dsllink_statusstr == "Connected" && dsl_DataRateDown > 0){
						document.form.ibw.value = (dsl_DataRateDown/1024).toFixed(2);
					}
					else if(dw_primary == "dsl"){
						document.form.ibw.value = "300";
					}
					else{
						document.form.ibw.value = "1000";
					}
				}
				else{

					document.form.ibw.value = "1000";
				}
				return false;
			}

			if(qos_type == 1 && document.getElementById('auto').checked){
				document.form.obw.value = 0;
				document.form.ibw.value = 0;
			}

			if(qos_xobw_orig > 0){
				if((qos_xobw_orig/1024).toFixed(2) < parseFloat(document.form.obw.value)){
					alert("<#value_lower_than#> "+ (qos_xobw_orig/1024).toFixed(2));
					document.form.obw.focus();
					document.form.obw.select();
					return false;
				}
			}
			document.form.qos_obw.disabled = false;
			document.form.qos_ibw.disabled = false;
			document.form.qos_obw.value = document.form.obw.value*1024;
			document.form.qos_ibw.value = document.form.ibw.value*1024;



			if(mtwancfg_support && wans_flag == "1") {

				if( ((qos_type == 1 && document.form.bw_setting_name[1].checked == true ) || qos_type == 0 || qos_type == 3) && (document.form.obw1.value.length == 0 || document.form.obw1.value == 0)){		// To check field is 0 && Traditional QoS
					alert("<#QoS_invalid_zero#>");
					error_obw1++;

				}
				else if( ((qos_type == 1 && document.form.bw_setting_name[1].checked == true ) || qos_type == 0 || qos_type == 3) && !validator.rangeFloat(document.form.obw1, 0, 9999999999, "")){
					error_obw1++;
				}

				if(error_obw1 > 0){
				
					if(qos_xobw1_orig > 0){
						document.form.obw1.value = (qos_xobw1_orig/1024).toFixed(2);
					}
					else if(dw_secondary == "dsl" && dsllink_statusstr_secondary == "Connected" && dsl_DataRateUp > 0){
						document.form.obw1.value = (dsl_DataRateUp/1024).toFixed(2);
					}
					else if(dw_secondary == "dsl"){
						document.form.obw1.value = "100";
					}
					else{
						document.form.obw1.value = "1000";	
					}
					
					return false;
				}

				if( ((qos_type == 1 && document.form.bw_setting_name[1].checked == true ) || qos_type == 0 || qos_type == 3) && (document.form.ibw1.value.length == 0 || document.form.ibw1.value == 0)){		// To check field is 0 && Traditional QoS
					alert("<#QoS_invalid_zero#>");
					error_ibw1++;
				}
				else if( ((qos_type == 1 && document.form.bw_setting_name[1].checked == true ) || qos_type == 0 || qos_type == 3) && !validator.rangeFloat(document.form.ibw1, 0, 9999999999, "")){
					error_ibw1++;
				}

				if(error_ibw1 > 0){
					if(dw_secondary == "dsl"  && dsllink_statusstr_secondary == "Connected" && dsl_DataRateDown > 0){
						document.form.ibw1.value = (dsl_DataRateDown/1024).toFixed(2);
					}
					else if(dw_secondary == "dsl"){
						document.form.ibw1.value = "300";
					}
					else{
						document.form.ibw1.value = "1000";	
					}
					
					return false;
				}

				if(qos_type == 1 && document.getElementById('auto').checked){
					document.form.obw1.value = 0;
					document.form.ibw1.value = 0;
				}

				if(qos_xobw1_orig > 0){
					if((qos_xobw1_orig/1024).toFixed(2) < parseFloat(document.form.obw1.value)){
						alert("<#value_lower_than#> "+(qos_xobw1_orig/1024).toFixed(2));
						document.form.obw1.focus();
						document.form.obw1.select();
						return false;
					}
				}

				document.form.qos_obw1.disabled = false;
				document.form.qos_ibw1.disabled = false;
				document.form.qos_obw1.value = document.form.obw1.value*1024;
				document.form.qos_ibw1.value = document.form.ibw1.value*1024;
			}

			if(qos_type == 1){	//Adaptive QoS
				if(document.getElementById("Game_act")
				|| document.getElementById("Media_act")
				|| document.getElementById("Web_act")
				|| document.getElementById("eLearning_act")
				|| document.getElementById("videoConference_act")
				|| document.getElementById("Customize_act")){
					document.form.bwdpi_app_rulelist.disabled = "";
					if(document.getElementById("Game_act")){
						document.form.bwdpi_app_rulelist.value = "9,20<8<4<0,5,6,15,17<4,13<13,24<1,3,14<7,10,11,21,23<game";
					}						
					else if(document.getElementById("Media_act")){
						document.form.bwdpi_app_rulelist.value = "9,20<4<0,5,6,15,17<4,13<8<13,24<1,3,14<7,10,11,21,23<media";
					}						
					else if(document.getElementById("Web_act")){
						document.form.bwdpi_app_rulelist.value = "9,20<13,24<4<0,5,6,15,17<4,13<8<1,3,14<7,10,11,21,23<web";
					}
					else if(document.getElementById("eLearning_act")){
						document.form.bwdpi_app_rulelist.value = "9,20<4,13<4<0,5,6,15,17<13,24<8<1,3,14<7,10,11,21,23<eLearning";
					}
					else if(document.getElementById("videoConference_act")){
						document.form.bwdpi_app_rulelist.value = "9,20<0,5,6,15,17<4<4,13<13,24<8<1,3,14<7,10,11,21,23<videoConference";
					}						
					else{
						document.form.bwdpi_app_rulelist.value = bwdpi_app_rulelist;
					}				
				}
				else{
					alert("<#Adaptive_QoS_priority_yet#>");
					return false;
				}
			}
		}
		else{		//Bandwidth Limiter
			if(document.form.PC_devicename.value != ""){
				alert("<#JS_add_rule#>");
				return false;
			}

			document.form.qos_bw_rulelist.disabled = false;
			document.form.qos_bw_rulelist.value = qos_bw_rulelist;
		}
	}

	return true;
}

function determineActionScript(){
	if(geforceNow_support && document.form.qos_enable.value == "0" && document.form.qos_enable_orig.value == "0" &&
		(document.form.nvgfn_enable.value != orig_nvgfn_enable)){
		document.form.action_script.value = "restart_upnp;";
	}
	else if( lantiq_support || Rawifi_support || (ctf_disable == "1" || ctf_disable_force == "1") ||  //BULECAVE; MTK Models; HW NAT OFF
			 (document.form.qos_enable_orig.value == "1" && document.form.qos_enable.value == "0") ||   //qos enable => disable
			 ((document.form.qos_enable_orig.value == document.form.qos_enable.value) && 				//qos_enable and qos_type no change
		 	  (document.form.qos_type_orig.value == document.form.qos_type.value)) ){
		document.form.action_script.value = "restart_qos;restart_firewall;";
		document.form.action_wait.value = "15";
	}
	else if(document.form.qos_enable.value == "1" && document.form.qos_type.value == "1" && (ctf_fa_mode != "2" || qca_support)){
		//BCM: Support FA but disable FA ,or not support FA. QCA Models
		document.form.action_script.value = "restart_qos;restart_firewall;";
		document.form.action_wait.value = "15";
	}
	else if(document.form.qos_enable.value == "1" && document.form.qos_type.value == "1" && (fc_disable_orig != "" && runner_disable_orig != "")){ //HND Router
		document.form.action_script.value = "restart_qos;restart_firewall;";
		document.form.action_wait.value = "15";
	}
	else{
		document.form.action_script.value = "reboot";
		document.form.action_wait.value = "<% get_default_reboot_time(); %>";
	}

	if((document.form.qos_type_orig.value != document.form.qos_type.value) && document.form.qos_type.value == "0")
		document.form.next_page.value = "Advanced_QOSUserRules_Content.asp";
}

function submitQoS(){
	if(validForm()){
		if(document.form.qos_enable.value == "1" && document.form.qos_type.value == "1" && document.form.TM_EULA.value == "0"){
			if(policy_status.TM == 0 || policy_status.TM_time == ''){
                const policyModal = new PolicyModalComponent({
                    policy: "TM",
                    agreeCallback: eula_confirm,
                });
                policyModal.show();
            }else{
                eula_confirm();
            }
		}
		else{
			if(	document.form.qos_enable.value == "1" && document.form.qos_type_orig.value != document.form.qos_type.value &&
				document.form.qos_type.value == 0 && lantiq_support){
				var hwnatPrompt = "NAT traffic will be processed by CPU if traditional QoS is enabled. Are you sure want to enable?";
				if(!confirm(hwnatPrompt)) return false;
			}

			if(reset_wan_to_fo.change_status){
				reset_wan_to_fo.change_wan_mode(document.form);
			}	

			if(GN_with_Amazon_WSS_enabled && 
				((document.form.qos_enable_orig.value == "1" && document.form.qos_enable.value == "0") || //Qos enable to disable
				(document.form.qos_type_orig.value == "2" && document.form.qos_type.value != "2"))){ // Qos type is change from Bandwidth Limiter to other.
					document.form.action_script.value = "restart_wireless;" + document.form.action_script.value;
					if (lantiq_support)
						document.form.action_wait.value = "60"; // for extend the time to let Amazon WSS ebtable rule ready, or it will block all clients
					var postData = {
						"do_rc": "0",
						"wss_enable": "0"
					};
					var parmData = {
						"wl_unit": "0",
						"wl_subunit": "2",
						"async": false
					};
					httpApi.amazon_wss.set(postData, parmData);
					var append_hidden_item = function(_item, _value){
						var NewInput = document.createElement("input");
						NewInput.type = "hidden";
						NewInput.name =  _item;
						NewInput.value = _value;
						document.form.appendChild(NewInput);
					};
					append_hidden_item("wl0.2_bss_enabled", "0");
			}

			determineActionScript();

			if(document.form.action_script.value=="reboot"){
				if(confirm("<#AiMesh_Node_Reboot#>")){
					showLoading();
					document.form.submit();
				}
			}
			else{
				showLoading();
				document.form.submit();
			}
		}
	}
	else
		return;
}

function change_qos_type(value){
	/* MODELDEP */
	if(value=="1" && (based_modelid == "RT-AC85U" || based_modelid == "RT-AC85P" || based_modelid == "RT-AC65U")){	//Force change to 0
		value = 0;
	}
	if(value == 0){		//Traditional QoS
		document.getElementById('int_type').checked = false;
		document.getElementById('trad_type').checked = true;
		document.getElementById('bw_limit_type').checked = false;
		if(geforceNow_support)
			document.getElementById('GeForce_type').checked = false;
		document.getElementById('bandwidth_setting_tr').style.display = "none";
		show_up_down(1);
		document.getElementById('list_table').style.display = "none";
		show_settings("NonAdaptive");
	}
	else if(value == 1){		//Adaptive QoS
		document.getElementById('int_type').checked = true;
		document.getElementById('trad_type').checked = false;
		document.getElementById('bw_limit_type').checked = false;
		document.getElementById('bandwidth_setting_tr').style.display = "";
		if(geforceNow_support)
			document.getElementById('GeForce_type').checked = false;
		document.getElementById('list_table').style.display = "none";
		if(document.getElementById("auto").checked){
			show_up_down(0);
		}
		else{
			show_up_down(1);
		}

		show_settings("Adaptive_quick");
	}
	else if(value == 2){		// Bandwidth Limiter
		document.getElementById('int_type').checked = false;
		document.getElementById('trad_type').checked = false;
		document.getElementById('bw_limit_type').checked = true;
		if(geforceNow_support)
			document.getElementById('GeForce_type').checked = false;
		document.getElementById('bandwidth_setting_tr').style.display = "none";
		show_up_down(0);
		document.getElementById('list_table').style.display = "table";
		show_settings("NonAdaptive");
		genMain_table();
		if(!pm_support)
			showDropdownClientList('setClientIP', 'name>mac', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');
	}
	else if(value == 3) {		// GeforceNow QoS
		document.getElementById('int_type').checked = false;
		document.getElementById('trad_type').checked = false;
		document.getElementById('bw_limit_type').checked = false;
		document.getElementById('GeForce_type').checked = true;
		document.getElementById('bandwidth_setting_tr').style.display = "none";
		show_up_down(1);
		document.getElementById('list_table').style.display = "none";
		show_settings("NonAdaptive");
	}
	else {
		alert("no such qos");
	}

	document.form.qos_type.value = value;

	if(value != 2){
		var alert_hint = "";
		if(GN_with_BandwidthLimeter)
			alert_hint += "<#Guest_Network_disable_BWL#>";
		if(GN_with_Amazon_WSS_enabled){
			if(alert_hint != "")
				alert_hint += "\n";
			alert_hint += "<#Guest_Network_disable_AmazonWiFi#>";
		}
		if(alert_hint != "")
			alert(alert_hint);
	}
}

function show_settings(flag){
	if(!adaptiveqos_support){
		document.getElementById("quick_setup_desc").style.display = "none";
		document.getElementById("quick_setup_table").style.display = "none";
	}
	else{
		if(flag == "NonAdaptive"){
			document.getElementById("quick_setup_desc").style.display = "none";
			document.getElementById("quick_setup_table").style.display = "none";
		}
		else if(flag == "Adaptive_category"){
			document.getElementById("quick_setup_desc").style.display = "none";
			document.getElementById("quick_setup_table").style.display = "none";
		}
		else{	//Adaptive_quick
			document.getElementById("quick_setup_desc").style.display = "";
			document.getElementById("quick_setup_table").style.display = "";
			check_actived();
		}
	}
}

function check_actived(){
	if(document.getElementById("Game_act")) document.getElementById("Game_act").id = "Game";
	if(document.getElementById("Media_act")) document.getElementById("Media_act").id = "Media";
	if(document.getElementById("Web_act")) document.getElementById("Web_act").id = "Web";
	if(document.getElementById("eLearning_act")) document.getElementById("eLearning_act").id = "eLearning";
	if(document.getElementById("videoConference_act")) document.getElementById("videoConference_act").id = "videoConference";
	if(document.getElementById("Customize_act")) document.getElementById("Customize_act").id = "Customize";

	//default APP priority of QoS
	if(bwdpi_app_rulelist == "9,20<8<4<0,5,6,15,17<13,24<1,3,14<7,10,11,21,23<<"		// old version
	|| bwdpi_app_rulelist == "9,20<8<4<0,5,6,15,17<4,13<13,24<1,3,14<7,10,11,21,23<"){	// new version
		return;
	}

	if(bwdpi_app_rulelist.indexOf('game') != -1){
		if(document.getElementById("Game")){
			document.getElementById("Game").id = "Game_act";
		}
	}
	else if(bwdpi_app_rulelist.indexOf('media') != -1){
		if(document.getElementById("Media")){
			document.getElementById("Media").id = "Media_act";
		}	
	}
	else if(bwdpi_app_rulelist.indexOf('web') != -1){
		if(document.getElementById("Web")){
			document.getElementById("Web").id = "Web_act";
		}
	}
	else if(bwdpi_app_rulelist.indexOf('eLearning') != -1){
		if(document.getElementById("eLearning")){
			document.getElementById("eLearning").id = "eLearning_act";
		}
	}
	else if(bwdpi_app_rulelist.indexOf('videoConference') != -1){
		if(document.getElementById("videoConference")){
			document.getElementById("videoConference").id = "videoConference_act";
		}
	}
	else{
		if(document.getElementById("Customize")){
			document.getElementById("Customize").id = "Customize_act";
		}
	}
}

function clickEvent(obj){
	var stitle;

	if(document.getElementById("Game_act")) document.getElementById("Game_act").id = "Game";
	if(document.getElementById("Media_act")) document.getElementById("Media_act").id = "Media";
	if(document.getElementById("Web_act")) document.getElementById("Web_act").id = "Web";
	if(document.getElementById("eLearning_act")) document.getElementById("eLearning_act").id = "eLearning";
	if(document.getElementById("videoConference_act")) document.getElementById("videoConference_act").id = "videoConference";
	if(document.getElementById("Customize_act")) document.getElementById("Customize_act").id = "Customize";

	if(obj.id.indexOf("Game") >= 0){
		document.getElementById("Game").id = "Game_act";
	}
	else if(obj.id.indexOf("Media") >= 0){
		obj.id = "Media_act";
	}
	else if(obj.id.indexOf("Web") >= 0){
		obj.id = "Web_act";
	}
	else if(obj.id.indexOf("eLearning") >= 0){
		obj.id = "eLearning_act";
	}
	else if(obj.id.indexOf("videoConference") >= 0){
		obj.id = "videoConference_act";
	}
	else if(obj.id.indexOf("Customize") >= 0){
		obj.id = "Customize_act";
		show_settings("NonAdaptive");
	}
	else
		alert("mouse over on wrong place!");
}

function set_priority(flag){
	if(flag == 'on'){
		$("#priority_panel").fadeIn(300);
		gen_category_block();
		register_event1();
	}
}

function regen_priority(obj){
	var priority_array = obj.children;
	var rule_temp = "";
	rule_temp += "9,20<";		//always at first priority
	for(i=0;i<priority_array.length;i++){
		rule_temp += cat_id_array[priority_array[i].id] + "<";
	}

	bwdpi_app_rulelist = rule_temp+"customize";
}

function gen_category_block(){
	bwdpi_app_rulelist = bwdpi_app_rulelist.replace(/&#60/g, "<");
	var bwdpi_app_rulelist_row = bwdpi_app_rulelist.split("<");
	if(bwdpi_app_rulelist == "" || bwdpi_app_rulelist_row.length != 9){	//Avoid customized app list cannot show out
		bwdpi_app_rulelist = "9,20<8<4<0,5,6,15,17<4,13<13,24<1,3,14<7,10,11,21,23<";
		bwdpi_app_rulelist_row = bwdpi_app_rulelist.split("<");
	}
	else{
		if(bwdpi_app_rulelist.indexOf('4,13') == -1){		// old version transforms to new version
			if(bwdpi_app_rulelist.indexOf('game') != -1){
				bwdpi_app_rulelist = '9,20<8<4<0,5,6,15,17<4,13<13,24<1,3,14<7,10,11,21,23<game';
			}
			else if(bwdpi_app_rulelist.indexOf('media') != -1){
				bwdpi_app_rulelist = '9,20<4<0,5,6,15,17<4,13<8<13,24<1,3,14<7,10,11,21,23<media';
			}
			else if(bwdpi_app_rulelist.indexOf('web') != -1){
				bwdpi_app_rulelist = '9,20<13,24<4<0,5,6,15,17<4,13<8<1,3,14<7,10,11,21,23<web';
			}
			else{	// default state, transform default value
				bwdpi_app_rulelist = "9,20<8<4<0,5,6,15,17<4,13<13,24<1,3,14<7,10,11,21,23<";
			}

			bwdpi_app_rulelist_row = bwdpi_app_rulelist.split("<");
		}
	}


	var index = 0;
	var code = "";
	
	for(i=1;i<bwdpi_app_rulelist_row.length-1;i++){
		for(j=1;j<cat_id_array.length;j++){
			if(cat_id_array[j] == bwdpi_app_rulelist_row[i]){
				index = j;
				break;
			}
		}

		code += '<div id='+ index +'>'+ category_title[index] +'</div>';
	}

	document.getElementById('category_list').innerHTML = code;
	register_overHint();
}

function cancel_priority_panel() {
	bwdpi_app_rulelist = "<% nvram_get("bwdpi_app_rulelist"); %>".replace(/&#60/g, "<");
	$("#priority_panel").fadeOut(300);
	setTimeout("change_qos_type(document.form.qos_type.value);", 300);
}

function save_priority(){
	regen_priority(document.getElementById("category_list"));
	$("#priority_panel").fadeOut(300);
	setTimeout("change_qos_type(document.form.qos_type.value);", 300);
}

function register_event1(){
	$(function() {
		$("#category_list").sortable({
			stop: function(event, ui){
				regen_priority(this);
			}
		});

		$("#category_list").disableSelection();
	});
}

function register_overHint(){
	document.getElementById('1').onmouseover = function(){overHint(91);}
	document.getElementById('1').onmouseout = function(){nd();}
	document.getElementById('2').onmouseover = function(){overHint(92);}
	document.getElementById('2').onmouseout = function(){nd();}
	document.getElementById('3').onmouseover = function(){overHint(93);}
	document.getElementById('3').onmouseout = function(){nd();}
	document.getElementById('4').onmouseover = function(){overHint(94);}
	document.getElementById('4').onmouseout = function(){nd();}
	document.getElementById('5').onmouseover = function(){overHint(95);}
	document.getElementById('5').onmouseout = function(){nd();}
	document.getElementById('6').onmouseover = function(){overHint(96);}
	document.getElementById('6').onmouseout = function(){nd();}
	if(document.getElementById('7')){
		document.getElementById('7').onmouseover = function(){overHint(97);}
		document.getElementById('7').onmouseout = function(){nd();}
	}
}

function bandwidth_setting(){
	if(document.getElementById("auto").checked){
		show_up_down(0);
	}
	else{
		show_up_down(1);
	}
}

function register_event(obj){
	$("#"+obj).click(function(){
		if(this.className == "cancel"){
			$(this).removeClass("cancel").addClass("check");
			$(this).children().removeClass("icon_cancel").addClass("icon_check");
		}
		else{
			$(this).removeClass("check").addClass("cancel");
			$(this).children().removeClass("icon_check").addClass("icon_cancel");
		}
	});
}

function pullLANIPList(obj){
	var element = document.getElementById('ClientList_Block_PC');
	var isMenuopen = element.offsetWidth > 0 || element.offsetHeight > 0;
	if(isMenuopen == 0){
		obj.src = "/images/unfold_less.svg"
		element.style.display = 'block';
		document.form.PC_devicename.focus();
	}
	else
		hideClients_Block();
}

function hideClients_Block(){
	document.getElementById("pull_arrow").src = "/images/unfold_more.svg";
	document.getElementById('ClientList_Block_PC').style.display='none';
}
var PC_mac = "";
var PC_name = "";
function setClientIP(devname, macaddr){
	document.form.PC_devicename.value = devname;
	PC_mac = macaddr;
	PC_name = devname;
	hideClients_Block();
	showDropdownClientList('setClientIP', 'name>mac', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');
}

function deleteRow_main(obj){
	var item_index = obj.parentNode.parentNode.rowIndex;
		document.getElementById(obj.parentNode.parentNode.parentNode.parentNode.id).deleteRow(item_index);

	var target_mac = "";
	if(pm_support)
		target_mac = "@" + obj.parentNode.parentNode.children[1].title;
	else
		target_mac = obj.parentNode.parentNode.children[1].title;
	var qos_bw_rulelist_row = qos_bw_rulelist.split("<");
	var qos_bw_rulelist_temp = "";
	var priority = 0;
	for(i=0;i<qos_bw_rulelist_row.length;i++){
		var qos_bw_rulelist_col = qos_bw_rulelist_row[i].split(">");
			if(qos_bw_rulelist_col[1] != target_mac){
				var string_temp = qos_bw_rulelist_row[i].substring(0,qos_bw_rulelist_row[i].length-qos_bw_rulelist_col[4].length) + priority;	// reorder priority number
				priority++;

				if(qos_bw_rulelist_temp == ""){
					qos_bw_rulelist_temp += string_temp;
				}
				else{
					qos_bw_rulelist_temp += "<" + string_temp;
				}
			}
	}

	qos_bw_rulelist = qos_bw_rulelist_temp;
	genMain_table();
}

function addRow_main(obj, length){
	if(document.form.PC_devicename.value == "" || document.getElementById("download_rate").value == "" || document.getElementById("upload_rate").value == ""){
		return true;
	}

	var enable_checkbox = $(obj.parentNode).siblings()[0].children[0];
	var invalid_char = "";
	var qos_bw_rulelist_row =  qos_bw_rulelist.split("<");
	var max_priority = 0;
	if(qos_bw_rulelist != "")
		max_priority = qos_bw_rulelist_row.length;

	if(qos_bw_rulelist_row.length >= length){
		alert("<#JS_itemlimit1#> " + length + " <#JS_itemlimit2#>");
		return false;
	}

	if(!validator.string(document.form.PC_devicename))
		return false;

	if(document.form.PC_devicename.value == ""){
		alert("<#JS_fieldblank#>");
		document.form.PC_devicename.focus();
		return false;
	}

	if(PC_mac != "" && PC_name == document.form.PC_devicename.value){
		if(qos_bw_rulelist.search(PC_mac+">") > -1 && PC_mac != ""){		//check same target
			alert("<#JS_duplicate#>");
			document.form.PC_devicename.focus();
			return false;
		}
	}
	else{
		if(qos_bw_rulelist.search(document.form.PC_devicename.value+">") > -1){
			alert("<#JS_duplicate#>");
			document.form.PC_devicename.focus();
			return false;
		}
	}

	if(document.getElementById("download_rate").value == ""){
		alert("<#JS_fieldblank#>");
		document.getElementById("download_rate").focus();
		return false;
	}
	else if(isNaN(document.getElementById("download_rate").value) || document.getElementById("download_rate").value < 0.1){
		alert("<#min_bound#> : 0.1 Mb/s");
		document.getElementById("download_rate").focus();
		return false;
	}

	if(document.getElementById("upload_rate").value == ""){
		alert("<#JS_fieldblank#>");
		document.getElementById("upload_rate").focus();
		return false;
	}
	else if(isNaN(document.getElementById("upload_rate").value) || document.getElementById("upload_rate").value < 0.1){
		alert("<#min_bound#> : 0.1 Mb/s");
		document.getElementById("upload_rate").focus();
		return false;
	}

	for(var i = 0; i < document.form.PC_devicename.value.length; ++i){
		if(document.form.PC_devicename.value.charAt(i) == '<' || document.form.PC_devicename.value.charAt(i) == '>'){
			invalid_char += document.form.PC_devicename.value.charAt(i);
			document.form.PC_devicename.focus();
			alert("<#JS_validstr2#> ' "+invalid_char + " '");
			return false;
		}
	}

	if(!pm_support) {
		if(PC_mac == "" || (PC_mac != "" && PC_name != document.form.PC_devicename.value)) {
			if(document.form.PC_devicename.value.split(":").length == 6) { //mac
				if(!validator.mac_addr(document.form.PC_devicename.value)) {
					document.form.PC_devicename.focus();
					alert("<#LANHostConfig_ManualDHCPMacaddr_itemdesc#>");
					return false;
				}
			}
			else if(document.form.PC_devicename.value.split(".").length == 4) { //ip
				if(!validator.ipv4_addr(document.form.PC_devicename.value)) { //single ip
					if(!validator.ipv4_addr_range(document.form.PC_devicename.value)) { //ip range
						document.form.PC_devicename.focus();
						alert(document.form.PC_devicename.value + " <#JS_validip#>");
						return false;
					}
				}
			}
			else {
				document.form.PC_devicename.focus();
				alert(document.form.PC_devicename.value + " <#Manual_Setting_JS_invalid#>");
				return false;
			}
		}
	}

	if(qos_bw_rulelist == ""){
		qos_bw_rulelist += enable_checkbox.className == "check" ? 1:0;
	}
	else{
		qos_bw_rulelist += "<";
		qos_bw_rulelist += enable_checkbox.className == "check" ? 1:0;
	}

	if(PC_mac == "") {
		if(pm_support)
			qos_bw_rulelist += ">@" + document.form.PC_devicename.value + ">";
		else
			qos_bw_rulelist += ">" + document.form.PC_devicename.value + ">";
	}
	else {
		if(PC_name == document.form.PC_devicename.value)
			qos_bw_rulelist += ">" + PC_mac + ">";
		else
			qos_bw_rulelist += ">" + document.form.PC_devicename.value + ">";
	}


	qos_bw_rulelist += document.getElementById("download_rate").value*1024 + ">" + document.getElementById("upload_rate").value*1024;
	qos_bw_rulelist += ">" + max_priority;
	PC_mac = "";
	PC_name = "";
	max_priority++;
	document.form.PC_devicename.value = "";
	genMain_table();
}

function genMain_table(){
	var qos_bw_rulelist_row = qos_bw_rulelist.split("<");
	var code = "";
	code += '<table width="100%" border="1" cellspacing="0" cellpadding="4" align="center" class="FormTable_table" id="mainTable_table">';
	code += '<thead><tr>';
	if(pm_support)
		code += '<td colspan="5"><#PM_Rule_List#>&nbsp;(<#List_limit#>&nbsp;32)</td>';
	else
		code += '<td colspan="5"><#ConnectedClient#>&nbsp;(<#List_limit#>&nbsp;32)</td>';
	code += '</tr></thead>';
	code += '<tbody>';
	code += '<tr>';
	code += '<th style="width:60px" height="30px" title="<#select_all#>">';
	if(select_all_checked == 1)
		code += '<div><div id="selAll" class="all_enable" style="margin: auto;width:40px;" onclick="enable_check(this);"><#All#></div></div>';
	else
		code += '<div><div id="selAll" class="all_disable" style="margin: auto;width:40px;" onclick="enable_check(this);"><#All#></div></div>';

	code += '</th>';
	if(pm_support)
		code += '<th style="width:330px" title="Target can be a group name, IP or IP range(ex:192.168.1.5-10)"><#NetworkTools_target#></th>';
	else
		code += '<th style="width:330px"><#NetworkTools_target#></th>';
	code += '<th style="width:130px"><#download_bandwidth#></th>';
	code += '<th style="width:130px"><#upload_bandwidth#></th>';
	code += '<th style="width:90px"><#list_add_delete#></th>';
	code += '</tr>';

	code += '<tr id="main_element">';
	code += '<td><div id="enable_button" class="check" style="width:22px;height:22px;margin:0 auto;display:none"><div style="width:16px;height:16px;margin: 1px auto" class="icon_check"></div></div>-</td>';
	code += '<td>';
    code += '<div class="clientlist_dropdown_main" style="width: 100%">';
	if(pm_support)
		code += '<input type="text" class="input_20_table" name="PC_devicename" onkeyup="device_filter(this);check_field();" placeholder="Please select the Device Group Name" autocorrect="off" autocapitalize="off" autocomplete="off" disabled>';/*untranslated*/
	else
		code += '<input type="text" class="input_20_table" name="PC_devicename" onkeyup="device_filter(this);check_field();" placeholder="<#AiProtection_client_select#>" autocorrect="off" autocapitalize="off" autocomplete="off">';
	code += '<img id="pull_arrow" src="/images/unfold_more.svg" onclick="pullLANIPList(this);" title="<#select_client#>">';
	code += '<div id="ClientList_Block_PC" class="clientlist_dropdown"></div>';
	code += '</div></td>';
	code += '<td style="border-bottom:0;text-align:right;"><input type="text" id="download_rate" class="input_6_table" maxlength="6" onkeypress="return validator.bandwidth_code(this, event);" onkeyup="check_field();"><span style="margin: 0 5px;color:#262626;">Mb/s</span></td>';
	code += '<td style="border-bottom:0;text-align:right;"><input type="text" id="upload_rate" class="input_6_table" maxlength="6" onkeypress="return validator.bandwidth_code(this, event);" onkeyup="check_field();"><span style="margin: 0 5px;color:#262626;">Mb/s</span></td>';
	code += '<td style="border-bottom:0;"><div id="add_delete" class="add_btn_disabled" style="margin:0 auto" onclick="addRow_main(this, 32)"></div></td>';
	code += '</tr>';

	if(qos_bw_rulelist == ""){
		code += '<tr><td style="color:#EF4444;" colspan="10"><#IPConnection_VSList_Norule#></td></tr>';
	}
	else{
		for(k=0;k< qos_bw_rulelist_row.length;k++){
			var qos_bw_rulelist_col = qos_bw_rulelist_row[k].split('>');
			var apps_client_name = "";

			var apps_client_mac = "";
			if(pm_support)
				apps_client_mac = qos_bw_rulelist_col[1].split("@")[1];
			else
				apps_client_mac = qos_bw_rulelist_col[1];
			var clientObj = clientList[apps_client_mac];
			if(clientObj == undefined) {
				apps_client_name = "";
			}
			else {
				apps_client_name = (clientObj.nickName == "") ? clientObj.name : clientObj.nickName;
			}

			if(qos_bw_rulelist_col[0] == 1){
				code += '<tr>';
				code += '<td style="">';
				code += '<div><div id="'+k+'" class="check" style="width:22px;height:22px;margin:0 auto" onclick="enable_check(this);"><div style="width:16px;height:16px;margin: 3px auto" class="icon_check"></div></div></div>';
			}else{
				code += '<tr style="color:#4d4d4d">';
				code += '<td style="">';
				code += '<div><div id="'+k+'" class="cancel" style="width:22px;height:22px;margin:0 auto" onclick="enable_check(this)"><div style="width:16px;height:16px;margin: 3px auto" class="icon_cancel"></div></div></div>';
			}
			code += '</td>';

			if(apps_client_name != "")
				code += '<td title="' + apps_client_mac + '">'+ apps_client_name + '<br>(' +  apps_client_mac +')</td>';
			else
				code += '<td title="' + apps_client_mac + '">' + apps_client_mac + '</td>';

			code += '<td style="text-align:center;">'+qos_bw_rulelist_col[2]/1024+' Mb/s</td>';
			code += '<td style="text-align:center;">'+qos_bw_rulelist_col[3]/1024+' Mb/s</td>';
			code += '<td><div class="remove" style="margin:0 auto" onclick="deleteRow_main(this);"></td>';
			code += '</tr>';

		}
	}

	code += '</tbody>';
	code += '</table>';
	document.getElementById('mainTable').innerHTML = code;
	showDropdownClientList('setClientIP', 'name>mac', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');
	for(k=0;k< qos_bw_rulelist_row.length;k++){
		register_event(k);
	}
	register_event("enable_button");
	if(pm_support)
		generate_group_list();
}

function device_filter(obj){
	var target_obj = document.getElementById("ClientList_Block_PC");
	if(obj.value == ""){
		hideClients_Block();
		showDropdownClientList('setClientIP', 'name>mac', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');
	}
	else{
		obj.src = "/images/unfold_less.svg"
		document.getElementById("ClientList_Block_PC").style.display = 'block';
		document.form.PC_devicename.focus();
		var code = "";
		for(var i = 0; i < clientList.length; i += 1) {
			var clientObj = clientList[clientList[i]];
			var clientName = (clientObj.nickName == "") ? clientObj.name : clientObj.nickName;
			if(clientList[i].toLowerCase().indexOf(obj.value.toLowerCase()) == -1 && clientName.toLowerCase().indexOf(obj.value.toLowerCase()) == -1)
				continue;

			code += '<div><a title=' + clientList[i] + '><div style="height:auto;" onclick="setClientIP(\'' + clientName + '\', \'' + clientObj.mac + '\');"><strong>' + clientName + '</strong> ';
			code += ' </div><!--[if lte IE 6.5]><iframe class="hackiframe2"></iframe><![endif]--></a></div>';
		}

		document.getElementById("ClientList_Block_PC").innerHTML = code;

		if(document.getElementById("ClientList_Block_PC").childNodes.length == "0")	hideClients_Block();
	}
}

function enable_check(obj){
	if(qos_bw_rulelist == "")
		return true;

	var qos_bw_rulelist_row = qos_bw_rulelist.split("<");
	var rulelist_row_temp = "";
	for(i=0;i<qos_bw_rulelist_row.length;i++){
		var qos_bw_rulelist_col = qos_bw_rulelist_row[i].split(">");
		var rulelist_col_temp = "";
		for(j=0;j<qos_bw_rulelist_col.length;j++){
			if(i == obj.id && j == 0){
				qos_bw_rulelist_col[j] = (obj.className == "cancel") ? 1 : 0;
			}
			else if(obj.id == "selAll" && j == 0){
				qos_bw_rulelist_col[j] = (obj.className == "all_enable") ? 1 : 0;
			}

			rulelist_col_temp += qos_bw_rulelist_col[j];
			if(j != qos_bw_rulelist_col.length-1)
				rulelist_col_temp += ">";
		}

		rulelist_row_temp += rulelist_col_temp;
		if(i != qos_bw_rulelist_row.length-1)
			rulelist_row_temp += "<";

		rulelist_col_temp = "";
	}

	if(obj.className == "all_disable")
		select_all_checked = "1";
	else
		select_all_checked = "0";


	qos_bw_rulelist = rulelist_row_temp;
	genMain_table();
}

function check_field(){
	if(document.form.PC_devicename.value != "" && document.getElementById("download_rate").value != "" && document.getElementById("upload_rate").value != ""){
		$("#add_delete").removeClass("add_btn_disabled").addClass("add_enable");
	}
	else{
		$("#add_delete").removeClass("add_enable").addClass("add_btn_disabled");
	}
}

function eula_confirm(){
	document.form.TM_EULA.value = 1;
	submitQoS();
}

function cancel(){
	refreshpage();
}

function generate_group_list(){
	var code = '';
	for(i=0;i<info.group.length;i++){
		var group_index = info.group[i];
		var group_obj = info.group[group_index];
		var group_name = group_obj.name;
		var group_description = group_obj.description;
		code += '<a id="'+ group_index +'" title="'+ group_description +'"><div onclick="setGroup(\''+ group_name +'\');"><strong>'+ group_name +'</strong></div></a>';
	}

	code += '<a><div onclick="" style="text-align:right;text-decoration:underline;color:#A0A0A0;"><strong style="cursor:pointer;" onclick="location.href=\'PermissionManagement_DeviceGroups.asp\'">Generate Device Group</strong></div></a>';

	$("#ClientList_Block_PC").html(code);
}

function setGroup(name){
	document.form.PC_devicename.value = name;
	hideClients_Block();
}
</script>
</head>
<body onload="initial();" id="body_id" onunload="unload_body();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<div id="hiddenMask" class="popup_bg" style="z-index:999;">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center"></table>
	<!--[if lte IE 6.5.]><script>alert("<#ALERT_TO_CHANGE_BROWSER#>");</script><![endif]-->
</div>

<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<table id="main_table" class="content" align="center" cellpadding="0" cellspacing="0">
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
			<div id="priority_panel">
				<table class="QISform_wireless" border=0 align="center" cellpadding="5" cellspacing="0">
				<form method="post" name="PriorityForm" action="/start_apply.htm" target="hidden_frame">
					<input type="hidden" name="current_page" value="QoS_EZQoS.asp">
					<input type="hidden" name="next_page" value="QoS_EZQoS.asp">
					<input type="hidden" name="modified" value="0">
					<input type="hidden" name="flag" value="background">
					<input type="hidden" name="action_mode" value="apply">
					<input type="hidden" name="action_script" value="saveNvram">
					<input type="hidden" name="action_wait" value="1">
					<input type="hidden" name="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
					<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
					<input type="hidden" name="bwdpi_app_rulelist_edit" value="<% nvram_get("bwdpi_app_rulelist"); %>">
					<tr>
						<div class="description_down"><#Adaptive_QoS#></div>
					</tr>
					<tr>
						<div style="margin-left:30px; margin-top:10px;">
							<div class="formfontdesc" style="line-height:20px;font-size:14px;"><#Adaptive_QoS_desc#></div>
						</div>
						<div style="margin:5px;*margin-left:-5px;width: 730px; height: 2px;" class="splitLine"></div>
					</tr>
					<tr>
						<td valign="top">
							<table width="100%" border="0" cellpadding="4" cellspacing="0">
								<tbody>
								<tr>
									<td valign="top">
										<table id="category_table" width="100%">
										<tr>
											<td colspan="2">
												<div class="priority priority_highest"><#Highest#></div>
											</td>
										</tr>
										<tr>
											<td colspan="2">
												<div id="category_list"></div>
											</td>
										</tr>
										<tr>
											<td colspan="2">
												<div class="priority priority_lowest"><#Lowest#></div>
											</td>
										</tr>
									</table>
									</td>
								</tr>
								</tbody>
							</table>
							<div style="margin-bottom:10px;width:100%;text-align:center;">
								<input class="button_gen" id="btn_cancel_priority" type="button" onclick="cancel_priority_panel();" value="<#CTL_Cancel#>">
								<input class="button_gen" type="button" onclick="save_priority();" value="<#CTL_onlysave#>">
							</div>
						</td>
					</tr>
				</form>
				</table>
			</div>

			<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
			<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
			<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
			<input type="hidden" name="current_page" value="QoS_EZQoS.asp">
			<input type="hidden" name="next_page" value="QoS_EZQoS.asp">
			<input type="hidden" name="group_id" value="">
			<input type="hidden" name="action_mode" value="apply">
			<input type="hidden" name="action_script" value="reboot">
			<input type="hidden" name="action_wait" value="<% get_default_reboot_time(); %>">
			<input type="hidden" name="flag" value="">
			<input type="hidden" name="TM_EULA" value="<% nvram_get("TM_EULA"); %>">
			<input type="hidden" name="qos_enable" value="<% nvram_get("qos_enable"); %>">
			<input type="hidden" name="qos_enable_orig" value="<% nvram_get("qos_enable"); %>">
			<input type="hidden" name="qos_type_orig" value="<% nvram_get("qos_type"); %>">
			<input type="hidden" name="qos_type" value="<% nvram_get("qos_type"); %>">
			<input type="hidden" name="qos_obw" value="<% nvram_get("qos_obw"); %>" disabled>
			<input type="hidden" name="qos_ibw" value="<% nvram_get("qos_ibw"); %>" disabled>
			<input type="hidden" name="qos_obw1" value="<% nvram_get("qos_obw1"); %>" disabled>
			<input type="hidden" name="qos_ibw1" value="<% nvram_get("qos_ibw1"); %>" disabled>
			<input type="hidden" name="bwdpi_app_rulelist" value="<% nvram_get("bwdpi_app_rulelist"); %>" disabled>
			<input type="hidden" name="qos_bw_rulelist" value="" disabled>

			<table width="100%" border="0" align="left" cellpadding="0" cellspacing="0" class="FormTitle" id="FormTitle" style="height:820px;">
				<tr>
					<td bgcolor="#4D595D" valign="top">
						<table width="100%" border="0" cellpadding="4" cellspacing="0">
							<tr>
								<td bgcolor="#4D595D" valign="top">
									<table width="100%">
										<tr style="height:30px;">
											<td  class="formfonttitle" align="left">
												<div id="content_title"></div>
											</td>
											<td align="right" >
												<div style="width:300px">
													<select id="settingSelection" onchange="switchPage(this.options[this.selectedIndex].value)" class="input_option">
														<option value="1"><#Adaptive_QoS_Conf#></option>
													</select>
												</div>
											</td>
										</tr>
									</table>
								</td>
							</tr>
							<tr>
								<td height="5" bgcolor="#4D595D" valign="top">
									<div class="splitLine"></div>
								</td>
							</tr>
							<tr>
								<td height="30" align="left" valign="top" bgcolor="#4D595D">
									<div>
										<table style="width:100%;">
											<tr>
												<td style="width:130px">
													<div id="guest_image" style="background: url(images/New_ui/QoS.png);width: 143px;height: 87px;"></div>
												</td>
												<td>&nbsp&nbsp</td>
												<td style="font-size: 14px;">
													<div id="function_desc" class="formfontdesc" style="">
														<#EzQoS_desc#>
														<ul>
															<li id="function_int_desc"><#EzQoS_desc_Adaptive#></li>
															<li id="qos_desc"><#EzQoS_desc_Traditional#></li>
															<li><#EzQoS_desc_Bandwidth_Limiter#></li>
														</ul>
														<#EzQoS_desc_note#>
													</div>
													<div class="formfontdesc">
														<a id="faq" href="" target="_blank" style="text-decoration:underline;">QoS FAQ</a>
													</div>
												</td>
											</tr>
										</table>
									</div>
								</td>
							</tr>
							<tr>
								<td valign="top">
									<table style="margin-left:3px;" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
										<tr id="GeForce_upnp" style="display: none;">
											<th><#GeForce_NOW_itemname#></th>
											<td colspan="2">
												<input type="radio" name="nvgfn_enable" class="input" value="1" <% nvram_match("nvgfn_enable", "1", "checked"); %>><#checkbox_Yes#>
												<input type="radio" name="nvgfn_enable" class="input" value="0" <% nvram_match("nvgfn_enable", "0", "checked"); %>><#checkbox_No#>
											</td>
										</tr>
										<tr>
											<th><#Enable_QoS#></th>
											<td colspan="2">
												<div class="left" style="width:94px; float:left; cursor:pointer;" id="radio_qos_enable"></div>
													<script type="text/javascript">
														$('#radio_qos_enable').iphoneSwitch('<% nvram_get("qos_enable"); %>',
															 function() {
																document.form.qos_enable.value = "1";
																if(document.form.qos_enable_orig.value != "1"){
																	if(document.getElementById('int_type').checked == true && adaptiveqos_support)
																		document.form.next_page.value = "QoS_EZQoS.asp";
																	else if(document.getElementById('trad_type').checked)		//Traditional QoS
																		document.form.next_page.value = "Advanced_QOSUserRules_Content.asp";
																}

																if(document.form.qos_type.value != 2){
																	show_up_down(1);
																}
																else{
																	show_up_down(0);
																}

																document.getElementById('qos_type_tr').style.display = "";
																if(adaptiveqos_support){
																	document.getElementById('qos_enable_hint').style.display = "";
																}
																change_qos_type(document.form.qos_type_orig.value);
															 },
															 function() {
																document.form.qos_enable.value = "0";
																show_up_down(0);
																document.getElementById('qos_type_tr').style.display = "none";
																document.getElementById('bandwidth_setting_tr').style.display = "none";
																document.getElementById('list_table').style.display = "none";

																var alert_hint = "";
																if(GN_with_BandwidthLimeter)
																	alert_hint += "<#Guest_Network_disable_BWL#>";
																if(GN_with_Amazon_WSS_enabled){
																	if(alert_hint != "")
																	alert_hint += "\n";
																	alert_hint += "<#Guest_Network_disable_AmazonWiFi#>";
																}
																if(alert_hint != "")
																	alert(alert_hint);

																if(adaptiveqos_support){

																	document.getElementById('qos_enable_hint').style.display = "none";
																	show_settings("NonAdaptive");
																}
															 }
														);
													</script>
												<div id="qos_enable_hint" style="color:#FC0;margin:5px 0px 0px 100px;display:none"><#QzQoS_note#></div>
											</td>
										</tr>
										<tr id="qos_type_tr" style="display:none">
											<th><#QoS_Type#></th>
											<td colspan="3">
												<input id="int_type" name="qos_type_radio" value="1" onClick="change_qos_type(this.value);" style="display:none;" type="radio" <% nvram_match("qos_type", "1","checked"); %>><a id="int_type_link" class="hintstyle" style="display:none;" href="javascript:void(0);" onClick="openHint(20, 5);"><label for="int_type"><#Adaptive_QoS#></label></a>
												<input id="trad_type" name="qos_type_radio" value="0" onClick="change_qos_type(this.value);" type="radio" <% nvram_match("qos_type", "0","checked"); %>><a class="hintstyle" href="javascript:void(0);" onClick="openHint(20, 6);"><label for="trad_type"><#EzQoS_type_traditional#></label></a>
												<input id="bw_limit_type" name="qos_type_radio" value="2" onClick="change_qos_type(this.value);" type="radio" <% nvram_match("qos_type", "2","checked"); %>><a class="hintstyle" href="javascript:void(0);" onClick="openHint(20, 7)"><label for="bw_limit_type"><#Bandwidth_Limiter#></label></a>
												<span id="GeForceNow_item" style="display: none;"><input id="GeForce_type" name="qos_type_radio" value="3" onClick="change_qos_type(this.value);" type="radio" <% nvram_match("qos_type", "3","checked"); %>><a class="hintstyle" href="javascript:void(0);"><label for="GeForce_type">GeForce NOW QoS</label></a></span>
											</td>
										</tr>
										<tr id="bandwidth_setting_tr" style="display:none">
											<th><#Bandwidth_Setting#></th>
											<td colspan="2">
												<input id="auto" name="bw_setting_name" onClick="bandwidth_setting();" type="radio"><label for="auto"><#Auto_Setting_btn#></label>
												<input id="manu" name="bw_setting_name" onClick="bandwidth_setting();" type="radio"><label for="manu"><#Manual_Setting_btn#></label>
											</td>
										</tr>
										<tr id="wan_1_tr" style="display:none">
											<th colspan=3 id="dualwan_primary_title"></th>
										</tr>
										<tr id="upload_tr">
											<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(20, 2);"><#upload_bandwidth#></a></th>
											<td>
												<input type="text" maxlength="10" class="input_6_table short_input" id="obw" name="obw" onKeyPress="return validator.isNumberFloat(this,event);" class="input_15_table" value="" autocorrect="off" autocapitalize="off">
												<label style="margin-left:5px;">Mb/s</label>
											</td>
											<td rowspan="2" style="width:250px;">
												<div>
													<ul style="padding:0 10px;margin:5px 0;">
														<li><#EzQoS_bandwidth_note1#></li>
													</ul>
												</div>

											</td>
										</tr>
										<tr id="download_tr">
											<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(20, 2);"><#download_bandwidth#></a></th>
											<td>
												<input type="text" maxlength="10" class="input_6_table short_input" id="ibw" name="ibw" onKeyPress="return validator.isNumberFloat(this,event);" class="input_15_table" value="" autocorrect="off" autocapitalize="off">
												<label style="margin-left:5px;">Mb/s</label>
											</td>
										</tr>
										<tr id="wan_2_tr" style="display:none">
											<th colspan=3 id="dualwan_secondary_title"></th>
										</tr>
										<tr id="upload2_tr" style="display:none">
											<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(20, 2);"><#upload_bandwidth#></a></th>
											<td>
												<input type="text" maxlength="10" id="obw1" name="obw1" onKeyPress="return validator.isNumberFloat(this,event);" class="input_15_table" value="" autocorrect="off" autocapitalize="off">
												<label style="margin-left:5px;">Mb/s</label>
											</td>
											<td rowspan="2" style="width:250px;">
												<div>
													<ul style="padding:0 10px;margin:5px 0;">
														<li><#EzQoS_bandwidth_note1#></li>
													</ul>
												</div>
											</td>
										</tr>
										<tr id="download2_tr" style="display:none">
											<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(20, 2);"><#download_bandwidth#></a></th>
											<td>
												<input type="text" maxlength="10" id="ibw1" name="ibw1" onKeyPress="return validator.isNumberFloat(this,event);" class="input_15_table" value="" autocorrect="off" autocapitalize="off">
												<label style="margin-left:5px;">Mb/s</label>
											</td>
										</tr>
									</table>
								</td>
							</tr>
						</table>

						<table id="quick_setup_desc" width="98%" border="0" style="margin-top:5px;margin-left:5px;display:none;">
							<tr>
								<td height="30" align="left" valign="top" bgcolor="#4D595D">
									<div class="formfontdesc" style="line-height:20px;font-size:14px;"><#Adaptive_QoS_priority#></div>
								</td>
							</tr>
						</table>

						<div id="quick_setup_table" style="display:none;">
							<div style="display:flex;align-items: center;justify-content: center;">
								<div style="width: 120px;height:174px;">
									<div style="display:flex;height:132px;">
										<div id="Game" onclick="clickEvent(this);" onmouseover="overHint(86);" onmouseout="nd();"></div>
									</div>				
									<div class="Quick_Setup_title"><#AiProtection_filter_stream1#></div>
								</div>
								<div style="width: 120px;height:174px;">
									<div style="display:flex;height:132px;">
										<div id="Media" onclick="clickEvent(this);" onmouseover="overHint(87);" onmouseout="nd();"></div>
									</div>		
									<div class="Quick_Setup_title"><#AiProtection_filter_stream2#></div>
								</div>
								<div style="width: 120px;height:174px;">
									<div style="display:flex;height:132px;">
										<div id="Web" onclick="clickEvent(this);" onmouseover="overHint(88);" onmouseout="nd();"></div>
									</div>					
									<div class="Quick_Setup_title"><#Adaptive_WebSurf#></div>
								</div>
								<div style="width: 120px;height:174px;">
									<div style="display:flex;height:132px;">
										<div id="eLearning" onclick="clickEvent(this);" onmouseover="overHint(104);" onmouseout="nd();"></div>
									</div>								
									<div class="Quick_Setup_title"><#Adaptive_eLearning#></div>
								</div>
								<div style="width: 120px;height:174px;">
									<div style="display:flex;height:132px;">
										<div id="videoConference" onclick="clickEvent(this);" onmouseover="overHint(103);" onmouseout="nd();"></div>
									</div>		
									<div class="Quick_Setup_title"><#Adaptive_Message#></div>
								</div>
								<div style="width: 120px;height:174px;">
									<div style="display:flex;height:132px;">
										<div id="Customize" onclick="clickEvent(this);set_priority('on');" onmouseover="overHint(85);" onmouseout="nd();"></div>
									</div>
									<div class="Quick_Setup_title"><#Customize#></div>
								</div>
							</div>
						</div>

						<table id="list_table" width="100%" border="0" cellpadding="0" cellspacing="0" style="padding-left:8px;">
							<tr>
								<td valign="top" align="center">
									<div id="mainTable" style="margin-top:10px;"></div>
								</td>
							</tr>
						</table>

						<table width="100%">
							<tr>
								<td>
									<div class="apply_gen">
										<div style="" class="titlebtn" align="center" onClick="submitQoS();"><span><#CTL_apply#></span></div>
									</div>
								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>
		<!--===================================End of Main Content===========================================-->
		</td>
	</tr>
</table>
</form>
<div id="footer"></div>
</body>
</html>
