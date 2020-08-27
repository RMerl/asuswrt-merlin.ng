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
<title><#Web_Title#> - <#VPN_Fusion#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="usp_style.css">
<link rel="stylesheet" type="text/css" href="/js/table/table.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" language="JavaScript" src="/client_function.js"></script>
<script type="text/javascript" language="JavaScript" src="/js/table/table.js"></script>
<script type="text/javascript" src="form.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<style type="text/css">
.contentM_qis{
	position:absolute;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius: 5px;
	z-index:200;
	display:none;
	margin-left: 30%;
	top: 290px;
	width:650px;
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

.QISform_wireless{
	width:600px;
	font-size:12px;
	color:#FFFFFF;
	margin-top:10px;
	*margin-left:10px;
}

.QISform_wireless thead{
	font-size:15px;
	line-height:20px;
	color:#FFFFFF;	
}

.QISform_wireless th{
	padding-left:10px;
	*padding-left:30px;
	font-size:12px;
	font-weight:bolder;
	color: #FFFFFF;
	text-align:left; 
}

.description_down{
	margin-top:10px;
	margin-left:10px;
	padding-left:5px;
	font-weight:bold;
	line-height:140%;
	color:#ffffff;	
}
.vpnc_ipconflict_icon {
	background-image: url(/images/New_ui/notification.png);
	background-repeat: no-repeat;
	height: 25px;
	width: 25px;
	margin: auto;
}
.vpnc_limit_hint {
	color: #FC0;
	font-weight: bolder;
	font-size: 13px;
	text-align: center;
}
.vpn_illustration {
	background-image: url('/images/vpn_illustration.png');
	height: 90px;
	background-size: 160px 90px;
	background-repeat: no-repeat;
	float: left;
	width: 24%;
	margin: 5% 0%;
}
.addRuleFrame {
	margin-top: 5px;
	height: 35px;
	padding-left: 5px;
}
.addRuleText {
	float: left;
	height: 35px;
	line-height: 35px;
	font-weight: bolder;
	font-family: Arial, Verdana, Arial, Helvetica, sans-serif;
	color: #FFFFFF;
	font-size: 16px;
}
.deactivate_text {
	color: #8D8D8D;
}
.default_disconnect {
	margin: 0 auto;
	width: 14px;
	height: 14px;
	border-radius: 50%;
	border-color: #8D8D8D;
	border-width: 2px;
	border-style: solid;
	cursor: pointer;
}
.default_connect {
	margin: 2px auto;
	width: 10px;
	height: 10px;
	border-radius: 50%;
	background-color: #08fff0;
}
.activateScan {
	background-image: url('/images/InternetScan.gif');
}
.tableApi_table {
	width: 98%;
	margin: auto;
}
</style>
<script>
var subnetIP_support_IPv6 = false;
var vpnc_clientlist_array = [];
var vpnc_pptp_options_x_list_array = [];
var restart_vpncall_flag = 0; //Viz add 2014.04 for Edit Connecting rule then restart_vpncall

var vpnc_status_array = []
var vpnc_default_wan = '<% nvram_get("vpnc_default_wan"); %>';
var vpnc_index = -1;
var vpnc_dev_policy_list_array = [];
var vpnc_dev_policy_list_array_ori = [];
var dhcp_staticlist_ori = decodeURIComponent('<% nvram_char_to_ascii("","dhcp_staticlist"); %>');

<% wanlink(); %>
<% secondary_wanlink(); %>
var ipsec_profile_client_1 = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_profile_client_1"); %>');
var ipsec_profile_client_2 = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_profile_client_2"); %>');
var ipsec_profile_client_3 = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_profile_client_3"); %>');
var ipsec_profile_client_4 = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_profile_client_4"); %>');
var ipsec_profile_client_5 = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_profile_client_5"); %>');
var ipsec_profile_client_1_ext = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_profile_client_1_ext"); %>');
var ipsec_profile_client_2_ext = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_profile_client_2_ext"); %>');
var ipsec_profile_client_3_ext = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_profile_client_3_ext"); %>');
var ipsec_profile_client_4_ext = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_profile_client_4_ext"); %>');
var ipsec_profile_client_5_ext = decodeURIComponent('<% nvram_char_to_ascii("","ipsec_profile_client_5_ext"); %>');
var all_profile_subnet_list = "";
var control_profile_flag = true;
var serverList_maxNum = 0;
var openvpnc_max = 5;

function parseNvramToArray(_oriNvram, _arrayLength) {
	var parseArray = [];
	var oriNvramRow = decodeURIComponent(_oriNvram).split('<');
	for(var i = 0; i < oriNvramRow.length; i += 1) {
		if(oriNvramRow[i] != "") {
			var oriNvramCol = oriNvramRow[i].split('>');
			var eachRuleArray = new Array();
			for(var j = 0; j < _arrayLength; j += 1) {
				if(oriNvramCol[j] != undefined)
					eachRuleArray.push(oriNvramCol[j]);
			}
			parseArray.push(eachRuleArray);
		}
	}
	return parseArray;
}
function parse_vpnc_dev_policy_list(_oriNvram) {
	var parseArray = [];
	var oriNvramRow = decodeURIComponent(_oriNvram).split('<');
	for(var i = 0; i < oriNvramRow.length; i += 1) {
		if(oriNvramRow[i] != "") {
			var oriNvramCol = oriNvramRow[i].split('>');
			var eachRuleArray = new Array();
			if(oriNvramCol.length == 4) {
				var temp_ipaddr = oriNvramCol[1];
				var temp_vpnc_idx =  oriNvramCol[3];
				var temp_active = oriNvramCol[0];
				eachRuleArray.push(temp_ipaddr);
				eachRuleArray.push(temp_vpnc_idx);
				eachRuleArray.push(temp_active);
			}
			parseArray.push(eachRuleArray);
		}
	}
	return parseArray;
}

function initial(){
	show_menu();
	vpnc_clientlist_array = parseNvramToArray('<% nvram_char_to_ascii("","vpnc_clientlist"); %>', 7);
	vpnc_pptp_options_x_list_array = parseNvramToArray('<% nvram_char_to_ascii("","vpnc_pptp_options_x_list"); %>', 1);
	vpnc_dev_policy_list_array = parse_vpnc_dev_policy_list('<% nvram_char_to_ascii("","vpnc_dev_policy_list"); %>');

	//combine vpnc_dev_policy_list_array and dhcp_staticlist
	var dhcp_staticlist_array = [];
	var dhcp_staticlist_tmp = '<% nvram_char_to_ascii("","dhcp_staticlist"); %>';
	if(dhcp_staticlist_tmp != "") {
		var dhcp_staticlist_row = decodeURIComponent(dhcp_staticlist_tmp).split("<");
		for(var i = 1; i < dhcp_staticlist_row.length; i += 1) {
			var dhcp_staticlist_col = dhcp_staticlist_row[i].split('>');
			var item_para = {"mac" : dhcp_staticlist_col[0].toUpperCase(), "dns" : (dhcp_staticlist_col[2] == undefined) ? "" : dhcp_staticlist_col[2]};
			dhcp_staticlist_array[dhcp_staticlist_col[1]] = item_para;
		}
	}
	for(var i = 0; i < vpnc_dev_policy_list_array.length; i += 1) {
		var vpnc_dev_policy_ipaddr = vpnc_dev_policy_list_array[i][0];
		var dhcp_staticlist_mac = dhcp_staticlist_array[vpnc_dev_policy_ipaddr].mac;
		var dhcp_staticlist_dns = dhcp_staticlist_array[vpnc_dev_policy_ipaddr].dns;
		if(dhcp_staticlist_dns == "")
			dhcp_staticlist_dns = "<#Setting_factorydefault_value#>";
		if(dhcp_staticlist_mac != undefined) {
			vpnc_dev_policy_list_array[i].splice(0, 0, dhcp_staticlist_mac);
			vpnc_dev_policy_list_array[i].splice(2, 0, dhcp_staticlist_dns);
		}
		else {
			vpnc_dev_policy_list_array.splice(i, 1);
		}
	}
	vpnc_dev_policy_list_array_ori = JSON.parse(JSON.stringify(vpnc_dev_policy_list_array));

	vpnc_status_array = parseNvramToArray('<% get_vpnc_status(); %>', 3);

	show_vpnc_rulelist();

	if(ipsec_cli_support) {
		update_connect_status();
		document.getElementById("ipsec_profile_client_1").value = ipsec_profile_client_1;
		document.getElementById("ipsec_profile_client_2").value = ipsec_profile_client_2;
		document.getElementById("ipsec_profile_client_3").value = ipsec_profile_client_3;
		document.getElementById("ipsec_profile_client_4").value = ipsec_profile_client_4;
		document.getElementById("ipsec_profile_client_5").value = ipsec_profile_client_5;
		document.getElementById("ipsec_profile_client_1_ext").value = ipsec_profile_client_1_ext;
		document.getElementById("ipsec_profile_client_2_ext").value = ipsec_profile_client_2_ext;
		document.getElementById("ipsec_profile_client_3_ext").value = ipsec_profile_client_3_ext;
		document.getElementById("ipsec_profile_client_4_ext").value = ipsec_profile_client_4_ext;
		document.getElementById("ipsec_profile_client_5_ext").value = ipsec_profile_client_5_ext;
		if(ipsec_profile_client_1 == "")
			document.ipsec_form.ipsec_profile_item.value = "ipsec_profile_client_1";
		else if(ipsec_profile_client_2 == "")
			document.ipsec_form.ipsec_profile_item.value = "ipsec_profile_client_2";
		else if(ipsec_profile_client_3 == "")
			document.ipsec_form.ipsec_profile_item.value = "ipsec_profile_client_3";
		else if(ipsec_profile_client_4 == "")
			document.ipsec_form.ipsec_profile_item.value = "ipsec_profile_client_4";
		else if(ipsec_profile_client_5 == "")
			document.ipsec_form.ipsec_profile_item.value = "ipsec_profile_client_5";

		while(document.ipsec_form.ipsec_local_public_interface.options.length > 0){
			document.ipsec_form.ipsec_local_public_interface.remove(0);
		}
		var wans_cap = '<% nvram_get("wans_cap"); %>'.split(" ");
		var wan_type_list = [];
		for(var i = 0; i < wans_cap.length; i += 1) {
			if(wans_cap[i] == "wan" || wans_cap[i] == "wan2" || wans_cap[i] == "usb") {
				var option_value = "";
				var option_text = "";
				option_value = wans_cap[i];
				option_text = wans_cap[i].toUpperCase();
				var option = [option_value, option_text];
				wan_type_list.push(option);
			}
		}
		var selectobject = document.ipsec_form.ipsec_local_public_interface;
		for(var i = 0; i < wan_type_list.length; i += 1) {
			var option = document.createElement("option");
			option.value = wan_type_list[i][0];
			option.text = wan_type_list[i][1];
			selectobject.add(option);
		}
		$("#ipsec_vpn_type_faq").html("IPSec Net-to-Net FAQ");/*untranslated*/
		httpApi.faqURL("1033578", function(url){document.getElementById("ipsec_vpn_type_faq").href=url;});
	}

	get_vpnc_profile_status();
	gen_exception_list_table();

	//	https://www.asus.com/support/FAQ/1033909
	httpApi.faqURL("1033909", function(url){document.getElementById("faq1").href=url;});	//this id is include in string : #VPN_Fusion_FAQ#	
}
function gen_exception_list_table() {
	//set table Struct
	var tableStruct = {
		data: vpnc_dev_policy_list_array,
		container: "exception_list_table",
		title: "<#VPN_Fusion_Exception_List#>",
		titieHint : "<#VPN_Fusion_Exception_List_hint#>",
		capability: {
			add: true,
			del: true,
			before_del_callBackFun : del_exception_list_confirm,
			clickEdit: true
		},
		header: [ 
			{
				"title" : "<#Client_Name#> (<#PPPConnection_x_MacAddressForISP_itemname#>)",
				"width" : "30%"
			},
			{
				"title" : "<#IPConnection_ExternalIPAddress_itemname#>",
				"width" : "20%"
			},
			{
				"title" : "<#VPN_Fusion_Connection_Name#>",
				"width" : "30%"
			},
			{
				"title" : "<#CTL_Activate#>",
				"width" : "10%"
			}
		],
		createPanel: {
			inputs : [
				{
					"editMode" : "clientList_showName_setMac",
					"title" : "<#Client_Name#> (<#PPPConnection_x_MacAddressForISP_itemname#>)",
					"maxlength" : "17",
					"validator" : "macAddress"
				},
				{
					"editMode" : "text_auto_set_client_ip",
					"title" : "<#IPConnection_ExternalIPAddress_itemname#>",
					"maxlength" : "15",
					"validator" : "ipAddressDHCP"
				},
				{
					"editMode" : "hidden"
				},
				{
					"editMode" : "select",
					"title" : "<#VPN_Fusion_Connection_Name#>",
					"option" : {"<#Internet#>" : "0"}
				},
				{
					"editMode" : "hidden",
					"value" : "0"
				}
			],
			maximum: 64
		},
		clickRawEditPanel: {
			inputs : [
				{
					"editMode" : "macShowClientName",
				},
				{
					"editMode" : "macShowClientName",
				},
				{
					"editMode" : "pureText",
					"styleList" : {"display":"none"}
				},
				{
					"editMode" : "select",
					"option" : {"<#Internet#>" : "0"},
					"before_edit_callBackFun" : edit_exception_list_confirm
				},
				{
					"editMode" : "activateIcon",
					"callBackFun" : control_exception_list_status
				}
			],
			callBackFun : update_exception_list
		},

		ruleDuplicateValidation : "vpncExceptionList"
	}

	for (var idx in vpnc_clientlist_array) {
		if (vpnc_clientlist_array.hasOwnProperty(idx)) {
			var optionText = vpnc_clientlist_array[idx][0];
			var optionValue = vpnc_clientlist_array[idx][6];

			tableStruct.createPanel.inputs[3].option[optionText] = optionValue;
			tableStruct.clickRawEditPanel.inputs[3].option[optionText] = optionValue;
		}
	}

	tableApi.genTableAPI(tableStruct);
}

var add_profile_flag = false;
function Add_profile(upper){
	var rule_num = vpnc_clientlist_array.length;
	if(rule_num >= upper){
		alert("<#JS_itemlimit1#> " + upper + " <#JS_itemlimit2#>");
		return false;
	}
	
	$('body').prepend(tableApi.genFullScreen());
	$('.fullScreen').fadeIn(300);

	add_profile_flag = true;
	gen_vpnc_tab_list("pptp");
	gen_vpnc_tab_list("openvpn");
	gen_vpnc_tab_list("ipsec");
	document.getElementById('importOvpnFile').style.display = "none";
	document.form.vpnc_des_edit.value = "";
	document.form.vpnc_svr_edit.value = "";
	document.form.vpnc_account_edit.value = "";
	document.form.vpnc_pwd_edit.value = "";
	document.form.selPPTPOption.value = "auto";	
	document.getElementById("pptpOptionHint").style.display = "none";
	document.vpnclientForm.vpnc_openvpn_des.value = "";
	document.vpnclientForm.vpnc_openvpn_username.value = "";
	document.vpnclientForm.vpnc_openvpn_pwd.value = "";	
	document.vpnclientForm.file.value = "";
	document.openvpnCAForm.file.value = "";
	document.getElementById("cbManualImport").checked = false;
	document.getElementById('edit_vpn_crt_client_ca').value = "";
	document.getElementById('edit_vpn_crt_client_crt').value = "";
	document.getElementById('edit_vpn_crt_client_key').value = "";
	document.getElementById('edit_vpn_crt_client_static').value = "";
	document.getElementById('edit_vpn_crt_client_crl').value = "";
	manualImport(false);
	$("#openvpnc_setting").fadeIn(300);
	tabclickhandler(0);
	document.getElementById("cancelBtn").style.display = "";
	document.getElementById("cancelBtn_openvpn").style.display = "";
	if(ipsec_cli_support)
		initialIPSecProfile();
	get_new_vpnc_index();
}

function cancel_add_rule(){
	idx_tmp = "";
	$("#openvpnc_setting_openvpn").fadeOut(300);
	$("#openvpnc_setting").fadeOut(300);
	$("body").find(".fullScreen").fadeOut(300, function() { tableApi.removeElement("fullScreen"); });
}

function addRow_Group(upper, flag, idx){
	disableOpenVpnManualformItem();
	idx = parseInt(idx);

	if(flag == 'PPTP' || flag == 'L2TP') {
		type_obj = document.form.vpnc_type;
		description_obj = document.form.vpnc_des_edit;
		server_obj = document.form.vpnc_svr_edit;
		username_obj = document.form.vpnc_account_edit;
		password_obj = document.form.vpnc_pwd_edit;
	}
	else {	//OpenVPN: openvpn
		type_obj = document.vpnclientForm.vpnc_type;
		description_obj = document.vpnclientForm.vpnc_openvpn_des;
		server_obj = document.vpnclientForm.vpnc_openvpn_unit_edit;
		username_obj = document.vpnclientForm.vpnc_openvpn_username;
		password_obj = document.vpnclientForm.vpnc_openvpn_pwd;
	}

	if(validForm(flag)) {
		duplicateCheck.tmpIdx = "";
		duplicateCheck.saveTotmpIdx(idx);
		duplicateCheck.tmpDataArray = [];
		duplicateCheck.saveToTmpDataArray(description_obj);
		duplicateCheck.saveToTmpDataArray(type_obj);
		duplicateCheck.saveToTmpDataArray(server_obj);
		duplicateCheck.saveToTmpDataArray(username_obj);
		duplicateCheck.saveToTmpDataArray(password_obj);
		if(duplicateCheck.isDuplicate()){
			alert("<#JS_duplicate#>")
			return false;
		}
		var editVPNCRuleArray = new Array();
		editVPNCRuleArray.push(description_obj.value);
		editVPNCRuleArray.push(type_obj.value);
		editVPNCRuleArray.push(server_obj.value);
		editVPNCRuleArray.push(username_obj.value);
		editVPNCRuleArray.push(password_obj.value);
		editVPNCRuleArray.push("0"); // activate status
		editVPNCRuleArray.push("" + vpnc_index + ""); // vpnc index
		if(idx >= 0) { //idx: edit row
			vpnc_clientlist_array[idx] = editVPNCRuleArray;
		}
		else {
			vpnc_clientlist_array.push(editVPNCRuleArray);

			//add new vpnc profile default status
			var new_status_array = [];
			new_status_array.push("5");
			new_status_array.push("0");
			new_status_array.push("" + vpnc_index + "");
			vpnc_status_array.push(new_status_array);
		}

		//handle vpnc_pptp_options_x_list
		handlePPTPOPtion(idx, document.form.selPPTPOption.value); //idx: NaN=Add i=edit row
		
		document.vpnclientForm.vpnc_dev_policy_list.disabled = true;
		document.vpnclientForm.vpnc_dev_policy_list_tmp.disabled = true;
		document.vpnclientForm.dhcp_static_x.disabled = true;
		document.vpnclientForm.dhcp_staticlist.disabled = true;
		document.vpnclientForm.vpnc_clientlist.disabled = false;
		document.vpnclientForm.vpnc_pptp_options_x_list.disabled = false;
		document.vpnclientForm.vpnc_clientlist.value = parseArrayToStr_vpnc_clientlist();
		document.vpnclientForm.vpnc_pptp_options_x_list.value = parseArrayToStr_vpnc_pptp_options_x_list();
		document.vpnclientForm.submit();
		
		cancel_add_rule();
		gen_exception_list_table();
		show_vpnc_rulelist();
	}
}

function handlePPTPOPtion(idx, objValue) {
	var origPPTPOptionsListArray = vpnc_pptp_options_x_list_array.slice();
	vpnc_pptp_options_x_list_array = [];
	if(idx >= 0) { // edit
		for(var i = 0; i < vpnc_clientlist_array.length; i += 1) {
			var eachRuleArray = new Array();
			if(origPPTPOptionsListArray[i] == undefined) {
				eachRuleArray.push("auto");
			}
			else {
				eachRuleArray.push(origPPTPOptionsListArray[i][0]);
			}
			vpnc_pptp_options_x_list_array.push(eachRuleArray);
		}
		var editRuleArray = new Array();
		editRuleArray.push(objValue);
		vpnc_pptp_options_x_list_array[idx] = editRuleArray;
	}
	else { // add
		for(var i = 0; i < (vpnc_clientlist_array.length - 1); i += 1) {
			var eachRuleArray = new Array();
			if(origPPTPOptionsListArray[i] == undefined) {
				eachRuleArray.push("auto");
			}
			else {
				eachRuleArray.push(origPPTPOptionsListArray[i][0]);
			}
			vpnc_pptp_options_x_list_array.push(eachRuleArray);
		}
		var newRuleArray = new Array();
		newRuleArray.push(objValue);
		vpnc_pptp_options_x_list_array.push(newRuleArray);
	}
}

var duplicateCheck = {
	tmpIdx: "",
	saveTotmpIdx: function(obj){
		this.tmpIdx = obj;	
	},	
	tmpDataArray: [],
	saveToTmpDataArray: function(obj) {
		this.tmpDataArray.push(obj.value);
	},
	isDuplicate: function(){
		var index_del = parseInt(this.tmpIdx);
		for(var i = 0; i < vpnc_clientlist_array.length; i += 1) {
			var compareDataArray = [];
			var currentEditDataArray = [];
			//compare VPN Server,Username,Password
			compareDataArray = vpnc_clientlist_array[i].slice();
			compareDataArray.splice(0, 1);
			compareDataArray.splice(4, 5);
			currentEditDataArray = this.tmpDataArray.slice();
			currentEditDataArray.splice(0, 1);
			if(i != index_del) {
				if(currentEditDataArray.toString() == compareDataArray.toString())
					return true;
			}
			//compare Description
			compareDataArray = vpnc_clientlist_array[i].slice();
			compareDataArray.splice(1);
			currentEditDataArray = this.tmpDataArray.slice();
			currentEditDataArray.splice(1);
			if(i != index_del) {
				if(currentEditDataArray.toString() == compareDataArray.toString())
					return true;
			}
		}
		return false;
	}
}

function validForm(mode){
	if(mode == "PPTP" || mode == "L2TP"){
		valid_des = document.form.vpnc_des_edit;
		valid_server = document.form.vpnc_svr_edit;
		valid_username = document.form.vpnc_account_edit;
		valid_password = document.form.vpnc_pwd_edit;

		if(valid_des.value==""){
			alert("<#JS_fieldblank#>");
			valid_des.focus();
			return false;		
		}else if(!Block_chars(valid_des, ["*", "+", "|", ":", "?", "<", ">", ",", ".", "/", ";", "[", "]", "\\", "=", "\"" ])){
			return false;		
		}

		if(valid_server.value==""){
			alert("<#JS_fieldblank#>");
			valid_server.focus();
			return false;
		}
		else{
			var isIPAddr = valid_server.value.replace(/\./g,"");
			var re = /^[0-9]+$/;
			if(!re.test(isIPAddr)) { //DDNS
				if(!Block_chars(valid_server, ["<", ">"])) {
					return false;
				}		
			}	
			else { // IP
				if(!validator.isLegalIP(valid_server,"")) {
					return false;
				}
			}
		}

		if(valid_username.value==""){
			alert("<#JS_fieldblank#>");
			valid_username.focus();
			return false;
		}else if(!Block_chars(valid_username, ["<", ">"])){
			return false;		
		}

		if(valid_password.value==""){
			alert("<#JS_fieldblank#>");
			valid_password.focus();
			return false;
		}else if(!Block_chars(valid_password, ["<", ">"])){
			return false;		
		}
			
	}
	else{		//OpenVPN
		valid_des = document.vpnclientForm.vpnc_openvpn_des;
		valid_username = document.vpnclientForm.vpnc_openvpn_username;
		valid_password = document.vpnclientForm.vpnc_openvpn_pwd;
		if(valid_des.value == ""){
			alert("<#JS_fieldblank#>");
			valid_des.focus();
			return false;		
		}
		else if(!Block_chars(valid_des, ["*", "+", "|", ":", "?", "<", ">", ",", ".", "/", ";", "[", "]", "\\", "=", "\"" ])){
			return false;		
		}
		
		if(valid_username.value != "" && !Block_chars(valid_username, ["<", ">"])){
			return false;		
		}

		if(valid_password.value != "" && !Block_chars(valid_password, ["<", ">"])){
			return false;		
		}
		
	}	
	
	return true;
}

var save_flag;	//type of Saving profile
function tabclickhandler(_type){
	var tab_id = "";
	switch(_type) {
		case 0 :
		case 1 :
			tab_id = "pptp";
			break;
		case 2 :
			tab_id = "openvpn";
			break;
		case 3 :
			tab_id = "ipsec";
			break;
	}
	document.getElementById('pptpcTitle_' + tab_id + '').className = "vpnClientTitle_td_unclick";
	document.getElementById('l2tpcTitle_' + tab_id + '').className = "vpnClientTitle_td_unclick";
	if(openvpnd_support)
		document.getElementById('opencTitle_' + tab_id + '').className = "vpnClientTitle_td_unclick";
	if(ipsec_cli_support)
		document.getElementById('ipsecTitle_' + tab_id + '').className = "vpnClientTitle_td_unclick";
	document.getElementById('openvpnc_setting').style.display = "none";
	document.getElementById('openvpnc_setting_openvpn').style.display = "none";	
	document.getElementById('openvpnc_setting_ipsec').style.display = "none";	
	document.getElementById('trPPTPOptions').style.display = "none";
	if(_type == 0){
		save_flag = "PPTP";
		document.form.vpnc_type.value = "PPTP";
		document.vpnclientForm.vpnc_type.value = "PPTP";
		document.getElementById('pptpcTitle_' + tab_id + '').className = "vpnClientTitle_td_click";
		document.getElementById('openvpnc_setting').style.display = "block";
		document.getElementById('trPPTPOptions').style.display = "";
		adjust_panel_block_top("openvpnc_setting", 200);
	}
	else if(_type == 1){
		save_flag = "L2TP";
		document.form.vpnc_type.value = "L2TP";
		document.vpnclientForm.vpnc_type.value = "L2TP";
		document.getElementById('l2tpcTitle_' + tab_id + '').className = "vpnClientTitle_td_click";
		document.getElementById('openvpnc_setting').style.display = "block";
		adjust_panel_block_top("openvpnc_setting", 200);
	}
	else if(_type == 2){
		save_flag = "OpenVPN";
		document.form.vpnc_type.value = "OpenVPN";
		document.vpnclientForm.vpnc_type.value = "OpenVPN";
		document.getElementById('opencTitle_' + tab_id + '').className = "vpnClientTitle_td_click";
		document.getElementById('openvpnc_setting_openvpn').style.display = "block";
		adjust_panel_block_top("openvpnc_setting_openvpn", 200);
		if(add_profile_flag)
			update_unit_option();
	}
	else if(_type == 3){
		if(!control_profile_flag) {
			alert("<#VPN_Fusion_Deactivate_Editor_Alert#>");
			return true;
		}
		save_flag = "IPSec";		
		update_unit_option();		
		document.form.vpnc_type.value = "IPSec";
		document.vpnclientForm.vpnc_type.value = "IPSec";
		document.getElementById('ipsecTitle_' + tab_id + '').className = "vpnClientTitle_td_click";
		document.getElementById('openvpnc_setting_ipsec').style.display = "block";
		adjust_panel_block_top("openvpnc_setting_ipsec", 50);
	}

	var set_limit_hint = function(_type, _limitNum, _name) {
		$("#tr_" + _type + "_limit_hint").css("display", "");
		var hint = _name + " : <#List_limit#> " + _limitNum;
		$("#openvpnc_setting_" + _type + "").find(".vpnc_limit_hint").html(hint);
		$("#openvpnc_setting_" + _type + "").find("input,button,textarea,select").attr("disabled", true);
		$("#cancelBtn_" + _type + "").attr("disabled", false);
	};
	var reset_limit_hint = function(_type) {
		$("#tr_" + _type + "_limit_hint").css("display", "none");
		$("#openvpnc_setting_" + _type + "").find("input,button,textarea,select").attr("disabled", false);
	}
	if (openvpn_arrayLength == openvpnc_max && openvpnd_support && add_profile_flag)
		set_limit_hint("openvpn", openvpn_arrayLength, "OpenVPN");
	else
		reset_limit_hint("openvpn");

	if(ipsec_arrayLength == 5 && ipsec_cli_support && add_profile_flag)
		set_limit_hint("ipsec", ipsec_arrayLength, "IPSec");
	else
		reset_limit_hint("ipsec");
}

function update_unit_option(){
	var vpnc_openvpn_unit_array = [];
	for(var i = 1; i <= openvpnc_max; i += 1) {
		vpnc_openvpn_unit_array["unit_" + i] = i;
	}
	for(var i = 0; i < vpnc_clientlist_array.length; i += 1) {
		if(vpnc_clientlist_array[i] != "") {
			var vpnc_proto = vpnc_clientlist_array[i][1];
			if(vpnc_proto == "OpenVPN") {
				var vpnc_openvpn_idx = vpnc_clientlist_array[i][2];
				delete vpnc_openvpn_unit_array["unit_" + vpnc_openvpn_idx];
			}
		}
	}

	// convert object into array
	var sortArray = [];
	for(var key in vpnc_openvpn_unit_array) {
		if(vpnc_openvpn_unit_array.hasOwnProperty(key))
			sortArray.push(vpnc_openvpn_unit_array[key]);
	}

	// sort items by value
	sortArray.sort(function(a, b){return b - a});
	document.vpnclientForm.vpnc_openvpn_unit_edit.value = sortArray[0];
}

var ipsec_arrayLength = 0;
var openvpn_arrayLength = 0;
function show_vpnc_rulelist(){
	var gen_default_connect = function(_idx) {
		var html = "";
		html +='<tr>';
		html +='<td width="10%">';
		html +='<div id="vpnc_default_wan_status_' + _idx + '" class="default_disconnect" onclick="set_default_connection(0);">';
		html +='</div>';
		html +='</td>';
		html +='<td width="15%">';
		if(wanConnectStatus)
			html += '<#Connected#>';
		else
			html += '<#Disconnected#>';
		html +='</td>';
		html +='<td width="30%">';
		html +='</td>';
		html +='<td width="15%">';
		html += '<#Internet#>';
		html +='</td>';
		html +='<td width="15%">';
		html +='</td>';
		html +='<td width="15%">';
		html +='</td>';
		html +='</tr>';
		return html;
	};
	all_profile_subnet_list = "";	
	ipsec_arrayLength = 0;
	openvpn_arrayLength = 0;

	if(ipsec_cli_support) {
	//create ipsec profile array start
		var ipsec_profilelist_arraylist = new Array();
		var temp_array = [];

		var push_to_profilelist_array = function(oriArray, profileIndex) {
			temp_array = [];
			if(oriArray != "") {
				temp_array = oriArray.split(">");
				temp_array.unshift(profileIndex);
				ipsec_profilelist_arraylist.push(temp_array);
			}
		};
		push_to_profilelist_array(ipsec_profile_client_1, "ipsec_profile_client_1");
		push_to_profilelist_array(ipsec_profile_client_2, "ipsec_profile_client_2");
		push_to_profilelist_array(ipsec_profile_client_3, "ipsec_profile_client_3");
		push_to_profilelist_array(ipsec_profile_client_4, "ipsec_profile_client_4");
		push_to_profilelist_array(ipsec_profile_client_5, "ipsec_profile_client_5");
		ipsec_arrayLength = ipsec_profilelist_arraylist.length;
		//create ipsec profile array end
	}

	var deactivate_array = [];
	var code = "";
	code +='<table width="98%" border="1" align="center" cellpadding="4" cellspacing="0" class="list_table" id="vpnc_clientlist_table">';
	if(vpnc_clientlist_array.length == 0 && ipsec_arrayLength == 0) {
		code += gen_default_connect(0);
		code +='<tr><td colspan="6" style="color:#FC0;"><#IPConnection_VSList_Norule#></td></tr>';
	}
	else{
		serverList_maxNum = 0;
		code += gen_default_connect(0);
		for(var i = 0; i < vpnc_clientlist_array.length; i += 1) {
			if(vpnc_clientlist_array[i] != "") {
				var vpnc_desc = vpnc_clientlist_array[i][0];
				var vpnc_proto = vpnc_clientlist_array[i][1];
				var vpnc_server = vpnc_clientlist_array[i][2];
				var vpnc_openvpn_idx = vpnc_clientlist_array[i][2];
				var vpnc_username = vpnc_clientlist_array[i][3];
				var vpnc_activate_status = vpnc_clientlist_array[i][5];
				var vpnc_profile_idx = vpnc_clientlist_array[i][6];
				if(vpnc_proto == "OpenVPN")
					openvpn_arrayLength++;

				code +='<tr id="vpnc_row_' + vpnc_profile_idx + '">';
				code +='<td width="10%"><div id="vpnc_default_wan_status_' + vpnc_profile_idx + '" class="default_disconnect" onclick="set_default_connection(' + vpnc_profile_idx + ');"></div></td>';

				//status
				code += '<td width="15%">';
				var vpnc_status = vpnc_status_array[i][0];
				var vpnc_reason = vpnc_status_array[i][1];
				var vpnc_idx = vpnc_status_array[i][2];
				if(vpnc_activate_status == "1" && vpnc_idx == vpnc_profile_idx) {
					switch(vpnc_status) {
						case "5" :
							code += "<#CTL_Disconnect#>";
							break;
						case "0" :
						case "4" :
							switch(vpnc_reason) {
								case "0" :
									code += "<img title='<#usb_initializing#>' src='/images/InternetScan.gif'>";
									break;
								case "1" :
									code += "<div title='<#vpn_openvpn_conflict#>' class='vpnc_ipconflict_icon'></div>";
									break;
								case "2" :
									code += "<img title='<#qis_fail_desc1#>' src='/images/button-close2.png' style='width:25px;'>";
									break;
								default :
									code += "<img title='<#ConnectionFailed#>' src='/images/button-close2.png' style='width:25px;'>";
							}
							break;
						case "1" :
							code += "<img title='<#CTL_Add_enrollee#>' src='/images/InternetScan.gif'>";
							break;
						case "2" :
							code += "<img title='<#Connected#>' src='/images/checked_parentctrl.png' style='width:25px;'>";
							break;
					}
				}
				else
					code +='<#CTL_Disconnect#>';

				code +='</td>';

				//Description
				if(vpnc_desc.length >28) {
					var overlib_str = vpnc_desc.substring(0, 25) + "...";
					code +='<td width="30%" title="'+vpnc_desc+'">'+ overlib_str +'</td>';
				}
				else {
					code +='<td width="30%">'+ vpnc_desc +'</td>';					
				}
				//VPN type
				code += '<td width="15%">'+ vpnc_proto +'</td>';

				if(vpnc_proto == "OpenVPN"){ 
					if(vpnc_activate_status == "1") {	//connecting
						code += '<td width="15%"><div class="activate_icon" onClick="connect_Row(this);"></div></td>';
						serverList_maxNum++;
					}
					else{			//OpenVPN is not connecting
						code += '<td width="15%"><div class="deactivate_icon" onClick="connect_Row(this);"></div></td>';
						deactivate_array.push(vpnc_profile_idx);
					}
				}
				else{
					if(vpnc_activate_status == "1") {
						code += '<td width="15%"><div class="activate_icon" onClick="connect_Row(this);"></div></td>';
						serverList_maxNum++;
					}
					else{		// This rule is not connecting
						code += '<td width="15%"><div class="deactivate_icon" onClick="connect_Row(this);"></div></td>';
						deactivate_array.push(vpnc_profile_idx);
					}
				}

				// EDIT / DEL
			 	code += '<td width="15%">';
			 	code += '<input class="edit_btn" type="button" onclick="Edit_Row(this);" value=""/>';
				code += '<input class="remove_btn" type="button" onclick="del_Row(this);" value=""/>';
			 	code += '</td>';

				code +='</tr>';
			}
		}

		if(ipsec_cli_support) {
			//creat ipsec profile row start
			control_profile_flag = true;
			for(var i = 0; i < ipsec_arrayLength; i += 1) {
				code +='<tr id=vpnc_row_' + ipsec_profilelist_arraylist[i][0] + '>';
				code +='<td width="15%"><div class="default_disconnect" onclick="set_default_connection(\'ipsec\');"></div></td>';
				if(ipsec_profilelist_arraylist[i][38] == 0) {
					code +='<td width="20%"><#CTL_Disconnect#></td>';
				}
				else {
					if(ipsec_connect_status_array[ipsec_profilelist_arraylist[i][2]]) {
						var connect_status = ipsec_connect_status_array[ipsec_profilelist_arraylist[i][2]].split("<")[1].split(">")[1];
						switch(connect_status) {
							case '1' :
								code +='<td width="20%" title="<#Connected#>"><img src="/images/checked_parentctrl.png" style="width:25px;"></td>';
								break;
							case '2' :
								code +='<td width="20%" title="<#Connecting_str#>"><img src="/images/InternetScan.gif" style="width:25px;"></td>';
								break;
							case '3' :
								code +='<td width="20%" title="<#ConnectionFailed#>"><img src="/images/button-close2.png" style="width:25px;cursor:pointer;"></td>';
								break;
						}
					}
					else {
						code +='<td width="20%" title="<#ConnectionFailed#>"><img src="/images/button-close2.png" style="width:25px;cursor:pointer;"></td>';
					}
				}
				var ipsec_idx =  ipsec_profilelist_arraylist[i][0].split("_")[3];
				var ipsec_profilename = ipsec_profilelist_arraylist[i][2].split("_c" + ipsec_idx + "")[0];
				code +='<td width="20%">' + ipsec_profilename + '</td>';
				code +='<td width="15%">IPSec</td>';
				code +='<td width="15%">';
				if(ipsec_profilelist_arraylist[i][38] == 0) {
					code += '<div class="deactivate_icon" onClick="connect_Row_IPSec(this);"></div>';
					deactivate_array.push(ipsec_profilelist_arraylist[i][0]);
				}
				else {
					code += '<div class="activate_icon" onClick="connect_Row_IPSec(this);"></div>';
					control_profile_flag = false;
				}
				code +='</td>';
				code += '<td width="15%">';
			 	code += '<input class="edit_btn" onclick="editIPSecProfile(\''+ ipsec_profilelist_arraylist[i][0] +'\');" value=""/>';
				code += '<input class="remove_btn" onclick="delIPSecProfile(this);" value=""/>';
			 	code += '</td>';
				
				code +='</tr>';
				all_profile_subnet_list += ">" + ipsec_profilelist_arraylist[i][11];
			}
			//creat ipsec profile row end
		}
	}
	
	code +='</table>';
	document.getElementById("vpnc_clientlist_Block").innerHTML = code;

	$("#vpnc_default_wan_status_" + vpnc_default_wan + "").html('<div class="default_connect"></div>');
	$("#vpnc_default_wan_status_" + vpnc_default_wan + "").css("border-color", "#08fff0");

	for(var idx in deactivate_array){
		if(deactivate_array.hasOwnProperty(idx)) {
			$("#vpnc_row_" + deactivate_array[idx]+ "").addClass("deactivate_text");
		}
	}
}

function connect_Row(rowdata) {
	var $activateItem = $(rowdata);
	var flag = $activateItem.hasClass("activate_icon") ? "disconnect" : "connect";
	var idx = rowdata.parentNode.parentNode.rowIndex;
	var idx_array = idx - 1; //because have default data, so idx - 1
	var $connectActionRow = $(rowdata.parentNode.parentNode);
	var vpnc_idx = $connectActionRow.attr('id');

	document.form.ctf_nonat_force.disabled = true;
	if(flag == "disconnect") { //Disconnect the connected rule 
		$connectActionRow.addClass("deactivate_text");
		$activateItem.addClass("deactivate_icon");
		$activateItem.removeClass("activate_icon");	
		vpnc_clientlist_array[idx_array][5] = "0";
		document.form.action_script.value = "stop_vpnc";
		serverList_maxNum--;
	}
	else { //"vpnc" making connection
		if(serverList_maxNum >= 4) {
			alert("It reached the max number of concurrent active VPN connections, please deactivate one of active VPN profile before activate a new one.");
			return;
		}
		if(isSupport("sdk7114")) {
			var pppoe_flag = false;
			var wanMax = isSupport("wanMax");
			for(var i = 0; i < wanMax; i += 1) {
				var wan_proto = httpApi.nvramGet(["wan" + i + "_proto"], true)["wan" + i + "_proto"];
				if(wan_proto == "pppoe") {
					pppoe_flag = true;
					break;
				}
			}
			var ctf_disable = httpApi.nvramGet(["ctf_disable"], true).ctf_disable;
			if(pppoe_flag && ctf_disable == "0") {
				var vpncoppp = httpApi.nvramGet(["vpncoppp"], true).vpncoppp;
				if(vpncoppp == "" || vpncoppp == "0") {
					if(confirm("<#vpnc_pppoe_dis_nat_confirm#>")) {
						document.form.ctf_nonat_force.disabled = false;
						document.form.ctf_nonat_force.value = "1";
						document.form.flag.value = "";
						document.form.action_script.value = "reboot";
						document.form.action_wait.value = httpApi.hookGet("get_default_reboot_time");
					}
					else
						return false;
				}
				else if(vpncoppp == "1") {
					document.form.ctf_nonat_force.disabled = false;
					document.form.ctf_nonat_force.value = "1";
				}
			}
		}
		$connectActionRow.removeClass("deactivate_text");
		$activateItem.addClass("activate_icon");
		$activateItem.removeClass("deactivate_icon");
		vpnc_clientlist_array[idx_array][5] = "1";
		document.form.action_script.value = "restart_vpnc";
		serverList_maxNum++;
	}

	document.form.vpnc_unit.value = idx_array;
	document.form.vpnc_pptp_options_x_list.value = parseArrayToStr_vpnc_pptp_options_x_list();
	document.form.vpnc_clientlist.value = parseArrayToStr_vpnc_clientlist();
	
	document.form.submit();
}

function setManualTable (unit) {
	switch (unit) {
		case "1" :
			document.getElementById('edit_vpn_crt_client_ca').value = vpn_crt_client1_ca[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			document.getElementById('edit_vpn_crt_client_crt').value = vpn_crt_client1_crt[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			document.getElementById('edit_vpn_crt_client_key').value = vpn_crt_client1_key[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			document.getElementById('edit_vpn_crt_client_static').value = vpn_crt_client1_static[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			document.getElementById('edit_vpn_crt_client_crl').value = vpn_crt_client1_crl[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			break;
		case "2" :
			document.getElementById('edit_vpn_crt_client_ca').value = vpn_crt_client2_ca[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			document.getElementById('edit_vpn_crt_client_crt').value = vpn_crt_client2_crt[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			document.getElementById('edit_vpn_crt_client_key').value = vpn_crt_client2_key[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			document.getElementById('edit_vpn_crt_client_static').value = vpn_crt_client2_static[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			document.getElementById('edit_vpn_crt_client_crl').value = vpn_crt_client2_crl[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			break;
		case "3" :
			document.getElementById('edit_vpn_crt_client_ca').value = vpn_crt_client3_ca[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			document.getElementById('edit_vpn_crt_client_crt').value = vpn_crt_client3_crt[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			document.getElementById('edit_vpn_crt_client_key').value = vpn_crt_client3_key[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			document.getElementById('edit_vpn_crt_client_static').value = vpn_crt_client3_static[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			document.getElementById('edit_vpn_crt_client_crl').value = vpn_crt_client3_crl[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			break;
		case "4" :
			document.getElementById('edit_vpn_crt_client_ca').value = vpn_crt_client4_ca[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			document.getElementById('edit_vpn_crt_client_crt').value = vpn_crt_client4_crt[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			document.getElementById('edit_vpn_crt_client_key').value = vpn_crt_client4_key[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			document.getElementById('edit_vpn_crt_client_static').value = vpn_crt_client4_static[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			document.getElementById('edit_vpn_crt_client_crl').value = vpn_crt_client4_crl[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			break;
		case "5" :
			document.getElementById('edit_vpn_crt_client_ca').value = vpn_crt_client5_ca[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			document.getElementById('edit_vpn_crt_client_crt').value = vpn_crt_client5_crt[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			document.getElementById('edit_vpn_crt_client_key').value = vpn_crt_client5_key[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			document.getElementById('edit_vpn_crt_client_static').value = vpn_crt_client5_static[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			document.getElementById('edit_vpn_crt_client_crl').value = vpn_crt_client5_crl[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r");
			break;
	}
}
function ovpnFileUpdate(unit){
	$.ajax({
		url: '/ajax_openvpn_server.asp',
		dataType: 'script',
		timeout: 1500,
		error: function(xhr){
			setTimeout("ovpnFileUpdate(" + unit + ");",1000);
		},	
		success: function(){
			setManualTable(unit);
		}
	});
}
var idx_tmp = "";
function Edit_Row(rowdata) {
	idx_tmp = rowdata.parentNode.parentNode.rowIndex; //update idx
	var idx = rowdata.parentNode.parentNode.rowIndex;
	idx_tmp = idx_tmp - 1;
	idx = idx - 1;
	var vpnc_activate_status = vpnc_clientlist_array[idx_tmp][5];
	if(vpnc_activate_status == "1") {
		alert("<#VPN_Fusion_Deactivate_Editor_Alert#>");
		return false;
	}
	
	add_profile_flag = false;
	document.getElementById("cancelBtn").style.display = "";
	document.getElementById("cancelBtn_openvpn").style.display = "";

	var vpnc_clientlist_edit_array = vpnc_clientlist_array.slice(idx, (idx + 1));
	var vpnc_desc = vpnc_clientlist_edit_array[0][0];
	var vpnc_proto = vpnc_clientlist_edit_array[0][1];
	var vpnc_server = vpnc_clientlist_edit_array[0][2];
	var vpnc_openvpn_idx = vpnc_clientlist_edit_array[0][2];
	var vpnc_username = vpnc_clientlist_edit_array[0][3];
	var vpnc_userpwd = vpnc_clientlist_edit_array[0][4];
	vpnc_index = vpnc_clientlist_edit_array[0][6];

	//get idx of PPTP option value
	var pptpOptionValue = "";
	if(idx >= 0){
		if(vpnc_pptp_options_x_list_array[idx] == undefined) {
			pptpOptionValue = "auto";
		}
		else {
			pptpOptionValue = vpnc_pptp_options_x_list_array[idx];
		}
	}
	else{	//default is auto
		pptpOptionValue = "auto";
	}
	
	document.form.selPPTPOption.value = pptpOptionValue;
	pptpOptionChange();

	if(vpnc_proto == "PPTP" || vpnc_proto == "L2TP") {
		gen_vpnc_tab_list("pptp");
		$("#openvpnc_setting").fadeIn(300);
		document.getElementById("pptpcTitle_pptp").style.display = "none";
		document.getElementById("trPPTPOptions").style.display = "none";
		document.getElementById("l2tpcTitle_pptp").style.display = "none";
		if(openvpnd_support)
			document.getElementById("opencTitle_pptp").style.display = "none";
		if(ipsec_cli_support)
			document.getElementById("ipsecTitle_pptp").style.display = "none";
		if(vpnc_proto == "PPTP") {
			document.getElementById("pptpcTitle_pptp").style.display = "";
			document.getElementById("trPPTPOptions").style.display = "";
		}
		else {
			document.getElementById("l2tpcTitle_pptp").style.display = "";
		}
	}
	else if(vpnc_proto == "OpenVPN") {
		gen_vpnc_tab_list("openvpn");
		$("#openvpnc_setting_openvpn").fadeIn(300);
		document.getElementById("pptpcTitle_openvpn").style.display = "none";
		document.getElementById("l2tpcTitle_openvpn").style.display = "none";
		document.getElementById("opencTitle_openvpn").style.display = "";
		if(ipsec_cli_support)
			document.getElementById("ipsecTitle_openvpn").style.display = "none";
	}

	if(vpnc_proto == "OpenVPN"){
		document.vpnclientForm.file.value = "";
		document.openvpnCAForm.file.value = "";
		document.vpnclientForm.vpnc_openvpn_des.value = vpnc_desc;
		document.vpnclientForm.vpnc_openvpn_username.value = vpnc_username;
		document.vpnclientForm.vpnc_openvpn_pwd.value = vpnc_userpwd;	

		ovpnFileUpdate(vpnc_openvpn_idx);
					
		tabclickhandler(2);
		//fixed unit
		document.vpnclientForm.vpnc_openvpn_unit_edit.value = vpnc_openvpn_idx;

		document.getElementById("caFiled").style.display = "";
		document.getElementById("manualFiled").style.display = "";
		document.getElementById('importOvpnFile').style.display = "none"; 
		document.getElementById('loadingicon').style.display = "none"; 
		document.getElementById("cbManualImport").checked = false;
		manualImport(false);
	}
	else{
		document.form.vpnc_des_edit.value = vpnc_desc;
		if(vpnc_proto == "PPTP")
			tabclickhandler(0);
		else if(vpnc_proto == "L2TP")
			tabclickhandler(1);
		else
			tabclickhandler(2);
		document.form.vpnc_svr_edit.value = vpnc_server;
		document.form.vpnc_account_edit.value = vpnc_username;
		document.form.vpnc_pwd_edit.value = vpnc_userpwd;
	}	
}

function del_Row(rowdata) {
	var idx = rowdata.parentNode.parentNode.rowIndex;
	var idx_array = idx - 1;
	var vpnc_activate_status = vpnc_clientlist_array[idx_array][5];
	if(vpnc_activate_status == "1") {
		alert("<#VPN_Fusion_Deactivate_Delete_Alert#>");
		return false;
	}

	var vpnc_clientlist_delete_array = vpnc_clientlist_array.slice(idx_array, (idx_array + 1));
	var delete_connect_type = vpnc_clientlist_delete_array[0][6];

	//check Exception List whether exist vpn profile.
	var exist_exception_list_flag = false;
	for(var idx_policy in vpnc_dev_policy_list_array){
		if (vpnc_dev_policy_list_array.hasOwnProperty(idx_policy)) {
			if(vpnc_dev_policy_list_array[idx_policy][3] == delete_connect_type) {
				exist_exception_list_flag = true;
				break;
			}
		}
	}
	if(exist_exception_list_flag) {
		if(!confirm("<#VPN_Fusion_Exception_Policies_Delete_Alert#>"))
			return false;
	}
	else {
		if(!confirm("<#VPN_Fusion_Delete_Alert#>"))
			return false;
	}

	document.getElementById("vpnc_clientlist_table").deleteRow(idx);

	vpnc_clientlist_array.splice(idx_array, 1);
	vpnc_status_array.splice(idx_array, 1);
				
	//del vpnc_pptp_options_x_list
	if(vpnc_pptp_options_x_list_array[idx_array] != undefined) {
		vpnc_pptp_options_x_list_array.splice(idx_array, 1);
	}

	//change vpnc_dev_policy_list to internet type
	var vpnc_dev_policy_list_array_temp = vpnc_dev_policy_list_array.slice();
	vpnc_dev_policy_list_array = [];
	for(var i in vpnc_dev_policy_list_array_temp){
		if (vpnc_dev_policy_list_array_temp.hasOwnProperty(i)) {
			if(vpnc_dev_policy_list_array_temp[i][3] != delete_connect_type)
				vpnc_dev_policy_list_array.push(vpnc_dev_policy_list_array_temp[i]);
		}
	}

	if(vpnc_default_wan == vpnc_clientlist_delete_array[0][6]) {
		vpnc_default_wan = 0;
		$(".default_disconnect").css("border-color", "#8D8D8D");
		$(".default_disconnect").empty();
		$("#vpnc_default_wan_status_" + vpnc_default_wan + "").html('<div class="default_connect"></div>');
		$("#vpnc_default_wan_status_" + vpnc_default_wan + "").css("border-color", "#08fff0");
		document.vpnc_default_wan_form.vpnc_default_wan_tmp.value = 0;
		document.vpnc_default_wan_form.action_script.value = "restart_default_wan";
		document.vpnc_default_wan_form.submit();
	}

	gen_exception_list_table();

	document.vpnclientForm.vpnc_clientlist.disabled = false;
	document.vpnclientForm.vpnc_pptp_options_x_list.disabled = false;
	document.vpnclientForm.vpnc_clientlist.value = parseArrayToStr_vpnc_clientlist();
	document.vpnclientForm.vpnc_pptp_options_x_list.value = parseArrayToStr_vpnc_pptp_options_x_list();

	document.vpnclientForm.dhcp_static_x.disabled = true;
	document.vpnclientForm.dhcp_staticlist.disabled = true;
	document.vpnclientForm.vpnc_dev_policy_list.disabled = false;
	document.vpnclientForm.vpnc_dev_policy_list.value = parseArrayToStr_vpnc_dev_policy_list(vpnc_dev_policy_list_array);
	if(vpnc_dev_policy_list_array.toString() != vpnc_dev_policy_list_array_ori.toString()) {
		document.vpnclientForm.vpnc_dev_policy_list_tmp.disabled = false;
		document.vpnclientForm.vpnc_dev_policy_list_tmp.value = parseArrayToStr_vpnc_dev_policy_list(vpnc_dev_policy_list_array_ori);
	}

	document.vpnclientForm.action_wait.value = "3";
	document.vpnclientForm.action_script.value  = "saveNvram";
	var dhcp_staticlist_current = parseArrayToStr_dhcp_staticlist();
	if(dhcp_staticlist_current != dhcp_staticlist_ori) {
		document.vpnclientForm.flag.value = "";
		document.vpnclientForm.action_wait.value = "35";
		document.vpnclientForm.action_script.value = "restart_net_and_phy";
		
		document.vpnclientForm.dhcp_static_x.disabled = false;
		document.vpnclientForm.dhcp_static_x.value = "1";
		document.vpnclientForm.dhcp_staticlist.disabled = false;
		document.vpnclientForm.dhcp_staticlist.value = dhcp_staticlist_current;
	}
	
	document.vpnclientForm.submit();

	vpnc_dev_policy_list_array_ori = JSON.parse(JSON.stringify(vpnc_dev_policy_list_array));

	if(vpnc_clientlist_array.length == 0)
		show_vpnc_rulelist();
}

function ImportOvpn(unit){
	if(document.vpnclientForm.file.value == "") {
		alert("<#Setting_upload_hint#>");
		return false;
	}
	document.getElementById('importOvpnFile').style.display = "none"; 
	document.getElementById('loadingicon').style.display = ""; 
	document.vpnclientForm.action = "vpnupload.cgi";
	document.vpnclientForm.enctype = "multipart/form-data";
	document.vpnclientForm.encoding = "multipart/form-data";
	document.vpnclientForm.vpn_upload_unit.value = unit;
	document.vpnclientForm.vpnc_dev_policy_list.disabled = true;
	document.vpnclientForm.vpnc_dev_policy_list_tmp.disabled = true;
	document.vpnclientForm.dhcp_static_x.disabled = true;
	document.vpnclientForm.dhcp_staticlist.disabled = true;
	document.vpnclientForm.vpnc_clientlist.disabled = true;
	document.vpnclientForm.vpnc_pptp_options_x_list.disabled = true;
	document.vpnclientForm.submit();
	setTimeout("ovpnFileChecker();",2000);
}

function manualImport(_flag){
	if(_flag){
		document.getElementById("caFiled").style.display = "";
		document.getElementById("manualFiled").style.display = "";
	}
	else{
		document.getElementById("caFiled").style.display = "none";
		document.getElementById("manualFiled").style.display = "none";
	}
}

var vpn_upload_state = "init";
function ovpnFileChecker(){
	document.getElementById("importOvpnFile").innerHTML = "<#Main_alert_proceeding_desc3#>";
	document.getElementById("manualCRList").style.color = "#FFF";
	document.getElementById("manualStatic").style.color = "#FFF";
	document.getElementById("manualKey").style.color = "#FFF";
	document.getElementById("manualCert").style.color = "#FFF";
	document.getElementById("manualCa").style.color = "#FFF";
	$.ajax({
		url: '/ajax_openvpn_server.asp',
		dataType: 'script',
		timeout: 1500,
		error: function(xhr){
			setTimeout("ovpnFileChecker();",1000);
		},	
		success: function(){
			document.getElementById('importOvpnFile').style.display = "";
			document.getElementById('loadingicon').style.display = "none";
			document.getElementById('importCA').style.display = "";
			document.getElementById('loadingiconCA').style.display = "none";
			if(vpn_upload_state == "init"){
				setTimeout("ovpnFileChecker();",1000);
			}
			else{
				setManualTable(document.vpnclientForm.vpnc_openvpn_unit_edit.value);
				
				var vpn_upload_state_tmp = "";
				vpn_upload_state_tmp = parseInt(vpn_upload_state) - 16;
				if(vpn_upload_state_tmp > -1){
					document.getElementById("importOvpnFile").innerHTML += ", Lack of Certificate Revocation List(Optional)";
					document.getElementById("manualCRList").style.color = "#FC0";
					vpn_upload_state = vpn_upload_state_tmp;
				}
					
				vpn_upload_state_tmp = parseInt(vpn_upload_state) - 8;
				if(vpn_upload_state_tmp > -1){
					document.getElementById("importOvpnFile").innerHTML += ", Lack of Static Key(Optional)";
					document.getElementById("manualStatic").style.color = "#FC0";
					vpn_upload_state = vpn_upload_state_tmp;
				}				

				vpn_upload_state_tmp = parseInt(vpn_upload_state) - 4;
				if(vpn_upload_state_tmp > -1){
					document.getElementById("importOvpnFile").innerHTML += ", Lack of Client Key!";
					document.getElementById("manualKey").style.color = "#FC0";
					vpn_upload_state = vpn_upload_state_tmp;
				}

				vpn_upload_state_tmp = parseInt(vpn_upload_state) - 2;
				if(vpn_upload_state_tmp > -1){
					document.getElementById("importOvpnFile").innerHTML += ", Lack of Client Certificate";
					document.getElementById("manualCert").style.color = "#FC0";
					vpn_upload_state = vpn_upload_state_tmp;
				}

				vpn_upload_state_tmp = parseInt(vpn_upload_state) - 1;
				if(vpn_upload_state_tmp > -1){
					document.getElementById("importOvpnFile").innerHTML += ", Lack of Certificate Authority";
					document.getElementById("manualCa").style.color = "#FC0";
				}
			}
		}
	});
}

var vpn_upload_state = "init";
function cancel_Key_panel(){
	document.getElementById("manualFiled_panel").style.display = "none";
}

function disableOpenVpnManualformItem(){
	for(var i = 1; i <= openvpnc_max; i += 1){
		if(document.openvpnManualForm["vpn_crt_client" + i + "_ca"] != undefined)
			document.openvpnManualForm["vpn_crt_client" + i + "_ca"].disabled = true;
		if(document.openvpnManualForm["vpn_crt_client" + i + "_crt"] != undefined)
			document.openvpnManualForm["vpn_crt_client" + i + "_crt"].disabled = true;
		if(document.openvpnManualForm["vpn_crt_client" + i + "_key"] != undefined)
			document.openvpnManualForm["vpn_crt_client" + i + "_key"].disabled = true;
		if(document.openvpnManualForm["vpn_crt_client" + i + "_static"] != undefined)
			document.openvpnManualForm["vpn_crt_client" + i + "_static"].disabled = true;
		if(document.openvpnManualForm["vpn_crt_client" + i + "_crl"] != undefined)
			document.openvpnManualForm["vpn_crt_client" + i + "_crl"].disabled = true;
	}
}
function saveManual(unit){
	disableOpenVpnManualformItem();
	switch (unit) {
		case "1" :
			document.openvpnManualForm.vpn_crt_client1_ca.value = document.getElementById('edit_vpn_crt_client_ca').value;
			document.openvpnManualForm.vpn_crt_client1_crt.value = document.getElementById('edit_vpn_crt_client_crt').value;
			document.openvpnManualForm.vpn_crt_client1_key.value = document.getElementById('edit_vpn_crt_client_key').value;
			document.openvpnManualForm.vpn_crt_client1_static.value = document.getElementById('edit_vpn_crt_client_static').value;
			document.openvpnManualForm.vpn_crt_client1_crl.value = document.getElementById('edit_vpn_crt_client_crl').value;
			document.openvpnManualForm.vpn_crt_client1_ca.disabled = false;
			document.openvpnManualForm.vpn_crt_client1_crt.disabled = false;
			document.openvpnManualForm.vpn_crt_client1_key.disabled = false;
			document.openvpnManualForm.vpn_crt_client1_static.disabled = false;
			document.openvpnManualForm.vpn_crt_client1_crl.disabled = false;
			break;
		case "2" :
			document.openvpnManualForm.vpn_crt_client2_ca.value = document.getElementById('edit_vpn_crt_client_ca').value;
			document.openvpnManualForm.vpn_crt_client2_crt.value = document.getElementById('edit_vpn_crt_client_crt').value;
			document.openvpnManualForm.vpn_crt_client2_key.value = document.getElementById('edit_vpn_crt_client_key').value;
			document.openvpnManualForm.vpn_crt_client2_static.value = document.getElementById('edit_vpn_crt_client_static').value;
			document.openvpnManualForm.vpn_crt_client2_crl.value = document.getElementById('edit_vpn_crt_client_crl').value;
			document.openvpnManualForm.vpn_crt_client2_ca.disabled = false;
			document.openvpnManualForm.vpn_crt_client2_crt.disabled = false;
			document.openvpnManualForm.vpn_crt_client2_key.disabled = false;
			document.openvpnManualForm.vpn_crt_client2_static.disabled = false;
			document.openvpnManualForm.vpn_crt_client2_crl.disabled = false;
			break;
		case "3" :
			document.openvpnManualForm.vpn_crt_client3_ca.value = document.getElementById('edit_vpn_crt_client_ca').value;
			document.openvpnManualForm.vpn_crt_client3_crt.value = document.getElementById('edit_vpn_crt_client_crt').value;
			document.openvpnManualForm.vpn_crt_client3_key.value = document.getElementById('edit_vpn_crt_client_key').value;
			document.openvpnManualForm.vpn_crt_client3_static.value = document.getElementById('edit_vpn_crt_client_static').value;
			document.openvpnManualForm.vpn_crt_client3_crl.value = document.getElementById('edit_vpn_crt_client_crl').value;
			document.openvpnManualForm.vpn_crt_client3_ca.disabled = false;
			document.openvpnManualForm.vpn_crt_client3_crt.disabled = false;
			document.openvpnManualForm.vpn_crt_client3_key.disabled = false;
			document.openvpnManualForm.vpn_crt_client3_static.disabled = false;
			document.openvpnManualForm.vpn_crt_client3_crl.disabled = false;
			break;
		case "4" :
			document.openvpnManualForm.vpn_crt_client4_ca.value = document.getElementById('edit_vpn_crt_client_ca').value;
			document.openvpnManualForm.vpn_crt_client4_crt.value = document.getElementById('edit_vpn_crt_client_crt').value;
			document.openvpnManualForm.vpn_crt_client4_key.value = document.getElementById('edit_vpn_crt_client_key').value;
			document.openvpnManualForm.vpn_crt_client4_static.value = document.getElementById('edit_vpn_crt_client_static').value;
			document.openvpnManualForm.vpn_crt_client4_crl.value = document.getElementById('edit_vpn_crt_client_crl').value;
			document.openvpnManualForm.vpn_crt_client4_ca.disabled = false;
			document.openvpnManualForm.vpn_crt_client4_crt.disabled = false;
			document.openvpnManualForm.vpn_crt_client4_key.disabled = false;
			document.openvpnManualForm.vpn_crt_client4_static.disabled = false;
			document.openvpnManualForm.vpn_crt_client4_crl.disabled = false;
			break;
		case "5" :
			document.openvpnManualForm.vpn_crt_client5_ca.value = document.getElementById('edit_vpn_crt_client_ca').value;
			document.openvpnManualForm.vpn_crt_client5_crt.value = document.getElementById('edit_vpn_crt_client_crt').value;
			document.openvpnManualForm.vpn_crt_client5_key.value = document.getElementById('edit_vpn_crt_client_key').value;
			document.openvpnManualForm.vpn_crt_client5_static.value = document.getElementById('edit_vpn_crt_client_static').value;
			document.openvpnManualForm.vpn_crt_client5_crl.value = document.getElementById('edit_vpn_crt_client_crl').value;
			document.openvpnManualForm.vpn_crt_client5_ca.disabled = false;
			document.openvpnManualForm.vpn_crt_client5_crt.disabled = false;
			document.openvpnManualForm.vpn_crt_client5_key.disabled = false;
			document.openvpnManualForm.vpn_crt_client5_static.disabled = false;
			document.openvpnManualForm.vpn_crt_client5_crl.disabled = false;
			break;
	}
	
	document.openvpnManualForm.submit();
	cancel_Key_panel();
}

function startImportCA(unit){
	if(document.openvpnCAForm.file.value == "") {
		alert("<#Setting_upload_hint#>");
		return false;
	}
	document.getElementById('importCA').style.display = "";
	document.getElementById('loadingiconCA').style.display = "";
	document.openvpnCAForm.vpn_upload_unit.value = unit;
	setTimeout('ovpnFileChecker();',2000);
	document.openvpnCAForm.submit();
}

function addOpenvpnProfile(){
	document.vpnclientForm.action = "/start_apply.htm";
	document.vpnclientForm.enctype = "application/x-www-form-urlencoded";
	document.vpnclientForm.encoding = "application/x-www-form-urlencoded";
	addRow_Group(16, save_flag, idx_tmp);
}
function pptpOptionChange() {
	document.getElementById("pptpOptionHint").style.display = "none";
	if(document.form.selPPTPOption.value == "+mppe-40") {
		document.getElementById("pptpOptionHint").style.display = "";
	}
}

function gen_vpnc_tab_list(_type) {
	var code = "";
	$('#divTabMenu_' + _type + '').empty();
	code += "<table width='100%' border='0' align='left' cellpadding='0' cellspacing='0' style='table-layout: fixed;'>";
	code += "<tr>";
	code += "<td align='center' id='pptpcTitle_" + _type + "' onclick='tabclickhandler(0);'>PPTP</td>";
	code += "<td align='center' id='l2tpcTitle_" + _type + "' onclick='tabclickhandler(1);'>L2TP</td>";
	if(openvpnd_support) {
		code += "<td align='center' id='opencTitle_" + _type + "' onclick='tabclickhandler(2);'>OpenVPN</td>";
	}
	if(ipsec_cli_support)
		code += "<td align='center' id='ipsecTitle_" + _type + "' onclick='tabclickhandler(3);'>IPSec</td>";
	code += "</tr>";
	code += "</table>";
	$('#divTabMenu_' + _type + '').html(code);
}
function cancel_ipsec_profile_panel() {
	$("#openvpnc_setting_ipsec").fadeOut(300);
	$("body").find(".fullScreen").fadeOut(300, function() { tableApi.removeElement("fullScreen"); });
}
function switchSettingsMode(mode) {
	if(mode == "1") {
		document.getElementById("ipsec_basic_settings").style.display = "";
		document.getElementById("ipsec_network_settings").style.display = "";
		document.getElementById("ipsec_advanced_settings").style.display = "none";
	}	
	else {
		document.getElementById("ipsec_basic_settings").style.display = "none";
		document.getElementById("ipsec_network_settings").style.display = "none";
		document.getElementById("ipsec_advanced_settings").style.display = "";
	}
}
function changeAdvDeadPeerDetection (obj) {
	if(obj.value == "0") {
		showhide("tr_adv_dpd_interval", 0);
	}
	else {
		showhide("tr_adv_dpd_interval", 1);
	}
}
function settingRadioItemCheck(obj, checkValue) {
	var radioLength = obj.length;
	for(var i = 0; i < radioLength; i += 1) {
		if(obj[i].value == checkValue) {
			obj[i].checked = true;
		}
	}
}
/* dotted-quad IP to integer */
function IPv4_dotquadA_to_intA(strbits) {
	var split = strbits.split( '.', 4 );
	var maskConvertInt = (
		parseFloat( split[0] * 16777216 )/* 2^24 */
		+ parseFloat( split[1] * 65536 )/* 2^16 */
		+ parseFloat( split[2] * 256 )/* 2^8  */
		+ parseFloat( split[3] )
	);
	var tempInt = maskConvertInt;

	var maskConvertResult = "";
	for(var i = 31; i >= 0; i --) {
		var result = tempInt - Math.pow(2,i);
		if(result == 0) {
			maskConvertResult = 32 - i;
			break;
		}
		tempInt = result;
	}
	return maskConvertResult;
}
//CIDR to netmask converion
function createNetmaskAddr(bitCount) {
	var mask = [];
	for(var i = 0; i < 4; i += 1) {
		var n = Math.min(bitCount, 8);
		mask.push(256 - Math.pow(2, 8-n));
		bitCount -= n;
	}
	return mask.join('.');
}
function clear_subnet_input(_type) {
	var subnet_node = document.getElementById("td_net_" + _type + "_private_subnet");
	while (subnet_node.firstChild) {
		subnet_node.removeChild(subnet_node.firstChild);
	}
}
function gen_subnet_input(_type, _idx, _value) {
	var subnet_input_obj = document.createElement("input");
	subnet_input_obj.type = "text";
	subnet_input_obj.className = "input_25_table";
	subnet_input_obj.id = "ipsec_" + _type + "_subnet_" + _idx;
	subnet_input_obj.value = _value;
	subnet_input_obj.autocomplete = "off";
	subnet_input_obj.autocapitalize = "off";
	subnet_input_obj.placeholder = "(ex.10.10.10.0/24)";
	if(subnetIP_support_IPv6)
		subnet_input_obj.maxLength = "39";
	else {
		subnet_input_obj.placeholder = "(ex.10.10.10.0/24)";
		subnet_input_obj.maxLength = "18";
		subnet_input_obj.onkeypress = function() {
			return validator.isIPAddrPlusNetmask(this,event);
		};
	}
	subnet_input_obj.style.marginTop = "4px";
	return subnet_input_obj;
}
function gen_subnet_add(_type) {
	var subnet_input_obj = document.createElement("input");
	subnet_input_obj.type = "text";
	subnet_input_obj.className = "add_btn";
	subnet_input_obj.style.height = "27px";
	subnet_input_obj.onclick = function() { add_subnet_item(this, _type);};
	return subnet_input_obj;
}
function gen_subnet_hint() {
	var subnet_input_obj = document.createElement("span");
	subnet_input_obj.innerHTML = " (ex.10.10.10.0/24)";
	subnet_input_obj.style.color = "#FC0";
	return subnet_input_obj;
}
function gen_subnet_del(_type) {
	var subnet_input_obj = document.createElement("input");
	subnet_input_obj.id = "btDelRemoteSubnet_" + _type;
	subnet_input_obj.type = "text";
	subnet_input_obj.className = "remove_btn";
	subnet_input_obj.style.height = "27px";
	subnet_input_obj.onclick = function() { del_subnet_item(this, _type);};
	return subnet_input_obj;
}
function initialIPSecProfile() {
	if(ipsec_profile_client_1 == "")
		document.ipsec_form.ipsec_profile_item.value = "ipsec_profile_client_1";
	else if(ipsec_profile_client_2 == "")
		document.ipsec_form.ipsec_profile_item.value = "ipsec_profile_client_2";
	else if(ipsec_profile_client_3 == "")
		document.ipsec_form.ipsec_profile_item.value = "ipsec_profile_client_3";
	else if(ipsec_profile_client_4 == "")
		document.ipsec_form.ipsec_profile_item.value = "ipsec_profile_client_4";
	else if(ipsec_profile_client_5 == "")
		document.ipsec_form.ipsec_profile_item.value = "ipsec_profile_client_5";

	document.getElementById("selSwitchMode").value = "1";
	switchSettingsMode("1");
	document.ipsec_form.ipsec_profilename.value = "";
	settingRadioItemCheck(document.ipsec_form.ipsec_remote_gateway_method, "0");
	changeRemoteGatewayMethod();
	document.ipsec_form.ipsec_remote_gateway.value = "";
	document.ipsec_form.ipsec_local_public_interface.value = "wan";
	document.ipsec_form.ipsec_preshared_key.value = "";
	settingRadioItemCheck(document.ipsec_form.ipsec_exchange, "0");
	changeExchangeMode();
	document.ipsec_form.ipsec_local_id.value = "";
	document.ipsec_form.ipsec_remote_id.value = "";

	clear_subnet_input("local");
	document.getElementById("td_net_local_private_subnet").appendChild(gen_subnet_input("local", 1, ""));
	document.getElementById("td_net_local_private_subnet").appendChild(gen_subnet_add("local"));
	document.ipsec_form.ipsec_local_port.value = "0";

	clear_subnet_input("remote");
	document.getElementById("td_net_remote_private_subnet").appendChild(gen_subnet_input("remote", 1, ""));
	document.getElementById("td_net_remote_private_subnet").appendChild(gen_subnet_add("remote"));

	document.ipsec_form.ipsec_remote_port.value = "0";
	settingRadioItemCheck(document.ipsec_form.ipsec_ike, "1");
	changeIKEVersion();
	document.ipsec_form.ipsec_keylife_p1.value = "172800";
	document.ipsec_form.ipsec_ike_isakmp.value = "500";
	document.ipsec_form.ipsec_ike_isakmp_nat.value = "4500";
	document.ipsec_form.ipsec_dpd.value = "10";
	settingRadioItemCheck(document.ipsec_form.ipsec_dead_peer_detection, "1");
	document.ipsec_form.ipsec_keylife_p2.value = "3600";
	document.ipsec_form.ipsec_keyingtries.value = "3";

	$('input:checkbox[name=ipsec_encryption_p1]').prop("checked", true);
	$('input:checkbox[name=ipsec_hash_p1]').prop("checked", true);
	$('input:checkbox[name=ipsec_dh_group_p1]').prop("checked", true);
	$('input:checkbox[name=ipsec_encryption_p2]').prop("checked", true);
	$('input:checkbox[name=ipsec_hash_p2]').prop("checked", true);
	settingRadioItemCheck(document.ipsec_form.ipsec_pfs, "1");
	changePFS();
	$('input:checkbox[name=ipsec_pfs_group]').prop("checked", true);
}
function editIPSecProfile(mode) {
	add_profile_flag = false;
	if(!control_profile_flag) {
		alert("<#VPN_Fusion_Deactivate_Editor_Alert#>");
		return true;
	}
	gen_vpnc_tab_list("ipsec");
	$("#openvpnc_setting_ipsec").fadeIn(300);
	document.getElementById("pptpcTitle_ipsec").style.display = "none";
	document.getElementById("l2tpcTitle_ipsec").style.display = "none";
	if(openvpnd_support)
		document.getElementById("opencTitle_ipsec").style.display = "none";
	document.getElementById("ipsecTitle_ipsec").style.display = "";
	tabclickhandler(3);

	var editProfileArray = [];
	var editProfileExtArray = [];
	switch (mode) {
		case "ipsec_profile_client_1" :
			editProfileArray = ipsec_profile_client_1.split(">");
			editProfileExtArray = ipsec_profile_client_1_ext.split(">");
			editProfileArray.unshift("ipsec_profile_client_1");
			document.ipsec_form.ipsec_profile_item.value = "ipsec_profile_client_1";
			editProfileArray[2] = editProfileArray[2].split("_c1")[0];
			break;
		case "ipsec_profile_client_2" :
			editProfileArray = ipsec_profile_client_2.split(">");
			editProfileExtArray = ipsec_profile_client_2_ext.split(">");
			editProfileArray.unshift("ipsec_profile_client_2");
			document.ipsec_form.ipsec_profile_item.value = "ipsec_profile_client_2";
			editProfileArray[2] = editProfileArray[2].split("_c2")[0];
			break;
		case "ipsec_profile_client_3" :
			editProfileArray = ipsec_profile_client_3.split(">");
			editProfileExtArray = ipsec_profile_client_3_ext.split(">");
			editProfileArray.unshift("ipsec_profile_client_3");
			document.ipsec_form.ipsec_profile_item.value = "ipsec_profile_client_3";
			editProfileArray[2] = editProfileArray[2].split("_c3")[0];
			break;
		case "ipsec_profile_client_4" :
			editProfileArray = ipsec_profile_client_4.split(">");
			editProfileExtArray = ipsec_profile_client_4_ext.split(">");
			editProfileArray.unshift("ipsec_profile_client_4");
			document.ipsec_form.ipsec_profile_item.value = "ipsec_profile_client_4";
			editProfileArray[2] = editProfileArray[2].split("_c4")[0];
			break;
		case "ipsec_profile_client_5" :
			editProfileArray = ipsec_profile_client_5.split(">");
			editProfileExtArray = ipsec_profile_client_5_ext.split(">");
			editProfileArray.unshift("ipsec_profile_client_5");
			document.ipsec_form.ipsec_profile_item.value = "ipsec_profile_client_5";
			editProfileArray[2] = editProfileArray[2].split("_c5")[0];
			break;
	}
	UpdatePSecProfile(editProfileArray, editProfileExtArray);
}
function delIPSecProfile(obj) {
	if(!control_profile_flag) {
		alert("<#VPN_Fusion_Deactivate_Delete_Alert#>");
		return true;
	}

	if(!confirm("<#VPN_Fusion_Delete_Alert#>"))
		return false;
	
	var delRowID = obj.parentNode.parentNode.id;
	if(delRowID == "vpnc_row_ipsec_profile_client_1") {
		ipsec_profile_client_1 = "";
		ipsec_profile_client_1_ext = "";
	}
	if(delRowID == "vpnc_row_ipsec_profile_client_2") {
		ipsec_profile_client_2 = "";
		ipsec_profile_client_2_ext = "";
	}
	if(delRowID == "vpnc_row_ipsec_profile_client_3") {
		ipsec_profile_client_3 = "";
		ipsec_profile_client_3_ext = "";
	}
	if(delRowID == "vpnc_row_ipsec_profile_client_4") {
		ipsec_profile_client_4 = "";
		ipsec_profile_client_4_ext = "";
	}
	if(delRowID == "vpnc_row_ipsec_profile_client_5") {
		ipsec_profile_client_5 = "";
		ipsec_profile_client_5_ext = "";
	}

	$("#" + delRowID + "").remove()

	document.ipsec_del_form.ipsec_profile_client_1.value = ipsec_profile_client_1;
	document.ipsec_del_form.ipsec_profile_client_2.value = ipsec_profile_client_2;
	document.ipsec_del_form.ipsec_profile_client_3.value = ipsec_profile_client_3;
	document.ipsec_del_form.ipsec_profile_client_4.value = ipsec_profile_client_4;
	document.ipsec_del_form.ipsec_profile_client_5.value = ipsec_profile_client_5;

	document.ipsec_del_form.ipsec_profile_client_1_ext.value = ipsec_profile_client_1_ext;
	document.ipsec_del_form.ipsec_profile_client_2_ext.value = ipsec_profile_client_2_ext;
	document.ipsec_del_form.ipsec_profile_client_3_ext.value = ipsec_profile_client_3_ext;
	document.ipsec_del_form.ipsec_profile_client_4_ext.value = ipsec_profile_client_4_ext;
	document.ipsec_del_form.ipsec_profile_client_5_ext.value = ipsec_profile_client_5_ext;

	document.ipsec_form.ipsec_profile_client_1.value = ipsec_profile_client_1;
	document.ipsec_form.ipsec_profile_client_2.value = ipsec_profile_client_2;
	document.ipsec_form.ipsec_profile_client_3.value = ipsec_profile_client_3;
	document.ipsec_form.ipsec_profile_client_4.value = ipsec_profile_client_4;
	document.ipsec_form.ipsec_profile_client_5.value = ipsec_profile_client_5;

	document.ipsec_form.ipsec_profile_client_1_ext.value = ipsec_profile_client_1_ext;
	document.ipsec_form.ipsec_profile_client_2_ext.value = ipsec_profile_client_2_ext;
	document.ipsec_form.ipsec_profile_client_3_ext.value = ipsec_profile_client_3_ext;
	document.ipsec_form.ipsec_profile_client_4_ext.value = ipsec_profile_client_4_ext;
	document.ipsec_form.ipsec_profile_client_5_ext.value = ipsec_profile_client_5_ext;

	document.ipsec_del_form.submit();
}
function UpdatePSecProfile(array, array_ext) {
	document.getElementById("selSwitchMode").value = "1";
	switchSettingsMode("1");
	document.ipsec_form.ipsec_profilename.value = array[2];
	settingRadioItemCheck(document.ipsec_form.ipsec_remote_gateway_method, array[3]);
	changeRemoteGatewayMethod();
	document.ipsec_form.ipsec_remote_gateway.value = array[4];
	document.ipsec_form.ipsec_local_public_interface.value = array[5];
	document.ipsec_form.ipsec_preshared_key.value = array[8];
	settingRadioItemCheck(document.ipsec_form.ipsec_exchange, array[20]);
	changeExchangeMode();
	document.ipsec_form.ipsec_local_id.value = array[21];
	document.ipsec_form.ipsec_remote_id.value = array[22];
	
	clear_subnet_input("local");
	if( array[9] != undefined) {
		var local_subnet = array[9].split("<");
		var local_subnet_idx = 1;
		for(var i = 0; i < local_subnet.length; i += 1) {
			if(local_subnet[i] != "") {
				document.getElementById("td_net_local_private_subnet").appendChild(gen_subnet_input("local", local_subnet_idx, local_subnet[i]));
				if(local_subnet_idx == 1) {
					document.getElementById("td_net_local_private_subnet").appendChild(gen_subnet_add("local"));
				}
				local_subnet_idx++;
			}
		}
		if(local_subnet_idx > 2) {
			document.getElementById("td_net_local_private_subnet").appendChild(gen_subnet_del("local"));
		}
	}
	document.ipsec_form.ipsec_local_port.value = array[10];

	clear_subnet_input("remote");
	if( array[11] != undefined) {
		var remote_subnet = array[11].split("<");
		var remote_subnet_idx = 1;
		for(var i = 0; i < remote_subnet.length; i += 1) {
			if(remote_subnet[i] != "") {
				document.getElementById("td_net_remote_private_subnet").appendChild(gen_subnet_input("remote", remote_subnet_idx, remote_subnet[i]));
				if(remote_subnet_idx == 1) {
					document.getElementById("td_net_remote_private_subnet").appendChild(gen_subnet_add("remote"));
				}
				remote_subnet_idx++;
			}
		}
		if(remote_subnet_idx > 2) {
			document.getElementById("td_net_remote_private_subnet").appendChild(gen_subnet_del("remote"));
		}
	}
	document.ipsec_form.ipsec_remote_port.value = array[12];

	settingRadioItemCheck(document.ipsec_form.ipsec_ike, array[17]);
	changeIKEVersion();
	document.ipsec_form.ipsec_keylife_p1.value = array[23];
	document.ipsec_form.ipsec_ike_isakmp.value = array[29];
	document.ipsec_form.ipsec_ike_isakmp_nat.value = array[30];
	document.ipsec_form.ipsec_dpd.value = array[31];
	settingRadioItemCheck(document.ipsec_form.ipsec_dead_peer_detection, array[32]);
	if(array[32] == "0") {
		showhide("tr_adv_dpd_interval", 0);
	}
	document.ipsec_form.ipsec_keylife_p2.value = array[35];
	document.ipsec_form.ipsec_keyingtries.value = array[36];
	$('input:checkbox[name=ipsec_encryption_p1]').prop("checked", false);
	$('input:checkbox[name=ipsec_hash_p1]').prop("checked", false);
	$('input:checkbox[name=ipsec_dh_group_p1]').prop("checked", false);
	$('input:checkbox[name=ipsec_encryption_p2]').prop("checked", false);
	$('input:checkbox[name=ipsec_hash_p2]').prop("checked", false);
	$('input:checkbox[name=ipsec_pfs_group]').prop("checked", false);
	var set_checkboxlist = function(_objName, _value) {
		var binary = parseInt(_value).toString(2);
		var binary_length = binary.length;
		var binary_reverse = binary.split('').reverse().join('');
		for(var i = 0; i < binary_length; i += 1) {
			if(binary_reverse.charAt(i) == "1") {
				var bit_to_int = 1 << i;
				$('input:checkbox[name=' + _objName + '][value=' + bit_to_int+ ']').prop("checked", true);
			}
		}
	};
	set_checkboxlist("ipsec_encryption_p1", array_ext[0]);
	set_checkboxlist("ipsec_hash_p1", array_ext[1]);
	set_checkboxlist("ipsec_dh_group_p1", array_ext[2]);
	set_checkboxlist("ipsec_encryption_p2", array_ext[3]);
	set_checkboxlist("ipsec_hash_p2", array_ext[4]);
	if(array_ext[5] == undefined) {
		settingRadioItemCheck(document.ipsec_form.ipsec_pfs, "1");
		$('input:checkbox[name=ipsec_pfs_group]').prop("checked", true);
	}
	else {
		if(array_ext[5] == "0")
			settingRadioItemCheck(document.ipsec_form.ipsec_pfs, "0");
		else {
			settingRadioItemCheck(document.ipsec_form.ipsec_pfs, "1");
			set_checkboxlist("ipsec_pfs_group", array_ext[5]);
		}
	}
	changePFS();
}
function getRadioItemCheck(obj) {
	var checkValue = "";
	var radioLength = obj.length;
	for(var i = 0; i < radioLength; i += 1) {
		if(obj[i].checked) {
			checkValue = obj[i].value;
			break;
		}
	}
	return checkValue;
}
function save_ipsec_profile_panel() {
	var validForm = function() {
		if(!validator.isEmpty(document.ipsec_form.ipsec_profilename))
			return false;
		if(!Block_chars(document.ipsec_form.ipsec_profilename, [">", "<"]))
			return false;
		if(!validator.isContainblanksStr(document.ipsec_form.ipsec_profilename)) {
			return false;
		}
		var checkDuplicateProfileName = function() {
			var dup_flag = false;
			for(var i = 1; i < 6; i += 1) {
				if(document.ipsec_form.ipsec_profile_item.value != ("ipsec_profile_client_" + i)) {
					var profileName = "";
					switch (("ipsec_profile_client_" + i)) {
						case "ipsec_profile_client_1" :
							if(ipsec_profile_client_1 != "")
								profileName = ipsec_profile_client_1.split(">")[1];
							break;
						case "ipsec_profile_client_2" :
							if(ipsec_profile_client_2 != "")
								profileName = ipsec_profile_client_2.split(">")[1];
							break;
						case "ipsec_profile_client_3" :
							if(ipsec_profile_client_3 != "")
								profileName = ipsec_profile_client_3.split(">")[1];
							break;
						case "ipsec_profile_client_4" :
							if(ipsec_profile_client_4 != "")
								profileName = ipsec_profile_client_4.split(">")[1];
							break;
						case "ipsec_profile_client_5" :
							if(ipsec_profile_client_5 != "")
								profileName = ipsec_profile_client_5.split(">")[1];
							break;
					}
					if(profileName == document.ipsec_form.ipsec_profilename.value) {
						alert("<#JS_duplicate#>");
						document.ipsec_form.ipsec_profilename.focus();
						document.ipsec_form.ipsec_profilename.select();
						dup_flag = true;
						break;
					}
				}
			}
			return dup_flag;
		};
		if(checkDuplicateProfileName()) {
			return false;
		}
		
		if(!validator.isEmpty(document.ipsec_form.ipsec_remote_gateway))
			return false;
		if(!Block_chars(document.ipsec_form.ipsec_remote_gateway, [">", "<"]))
			return false;

		if(getRadioItemCheck(document.ipsec_form.ipsec_remote_gateway_method) == "0") {
			if(!validator.ipv4_addr(document.ipsec_form.ipsec_remote_gateway.value)) {
				document.ipsec_form.ipsec_remote_gateway.focus();
				alert(document.ipsec_form.ipsec_remote_gateway.value + " <#JS_validip#>");
				return false;
			}
		}
		else if(getRadioItemCheck(document.ipsec_form.ipsec_remote_gateway_method) == "1") {
			if(!validator.domainName_flag(document.ipsec_form.ipsec_remote_gateway.value)) {
				document.ipsec_form.ipsec_remote_gateway.focus();
				alert(document.ipsec_form.ipsec_remote_gateway.value + " is invalid Domain Name");/*untranslated*/
				return false;
			}
			if(!validator.isEmpty(document.ipsec_form.ipsec_remote_id))
				return false;
		}

		if(!validator.isEmpty(document.ipsec_form.ipsec_preshared_key))
			return false;
		if(!Block_chars(document.ipsec_form.ipsec_preshared_key, [">", "<", "&", "\"", "null"]))
			return false;
		if(is_KR_sku){
			if(!validator.psk_KR(document.ipsec_form.ipsec_preshared_key))
				return false;
		}
		else{
			if(!validator.psk(document.ipsec_form.ipsec_preshared_key))
				return false;
		}
		//confirm common string combination	#JS_common_passwd#
		var is_common_string = check_common_string(document.ipsec_form.ipsec_preshared_key.value, "wpa_key");
		if(is_common_string){
			if(!confirm("<#JS_common_passwd#>")){
				document.ipsec_form.ipsec_preshared_key.focus();
				document.ipsec_form.ipsec_preshared_key.select();
				return false;
			}	
		}

		if(!validator.numberRange(document.ipsec_form.ipsec_local_port, 0, 65535)) {
			return false;
		}
		if(!validator.numberRange(document.ipsec_form.ipsec_remote_port, 0, 65535)) {
			return false;
		}
		if(!validator.numberRange(document.ipsec_form.ipsec_keylife_p1, 120, 172800)) {
			return false;
		}
		if(!validator.numberRange(document.ipsec_form.ipsec_dpd, 10, 900)) {
			return false;
		}
		if(!validator.numberRange(document.ipsec_form.ipsec_keylife_p2, 120, 172800)) {
			return false;
		}

		if(parseInt(document.ipsec_form.ipsec_keylife_p2.value) > parseInt(document.ipsec_form.ipsec_keylife_p1.value)) {
			alert("The phase 2 IKE keylift time can not be greater than phase 1 IKE keylift time");/*untranslated*/
			document.ipsec_form.ipsec_keylife_p2.focus();
			return false;
		}

		if(!Block_chars(document.ipsec_form.ipsec_local_id, [">", "<", "null"]))
			return false;
		if(!Block_chars(document.ipsec_form.ipsec_remote_id, [">", "<", "null"]))
			return false;

		var valid_subnet = function(_type) {
			var checkGatewayIP = function(_lanIPAddr, _lanNetMask) {
				var lanIPAddr = _lanIPAddr;
				var lanNetMask = _lanNetMask;
				var ipConflict;
				var alertMsg = function (type, ipAddr, netStart, netEnd) {
					alert("*Conflict with " + type + " IP: " + ipAddr + ",\n" + "Network segment is " + netStart + " ~ " + netEnd);
				};

				//1.check Wan IP
				ipConflict = checkIPConflict("WAN", lanIPAddr, lanNetMask);
				if(ipConflict.state) {
					alertMsg("WAN", ipConflict.ipAddr, ipConflict.netLegalRangeStart, ipConflict.netLegalRangeEnd);
					return false;
				}

				//2.check PPTP
				if(pptpd_support) {
					ipConflict = checkIPConflict("PPTP", lanIPAddr, lanNetMask);
					if(ipConflict.state) {
						alertMsg("PPTP", ipConflict.ipAddr, ipConflict.netLegalRangeStart, ipConflict.netLegalRangeEnd);
						return false;
					}
				}

				//3.check OpenVPN
				if(openvpnd_support) {
					ipConflict = checkIPConflict("OpenVPN", lanIPAddr, lanNetMask);
					if(ipConflict.state) {
						alertMsg("OpenVPN", ipConflict.ipAddr, ipConflict.netLegalRangeStart, ipConflict.netLegalRangeEnd);
						return false;
					}
				}

				return true;
			};
			var existSubnetItem = 1;//ike v1 only single subnet
			var ike_version = getRadioItemCheck(document.ipsec_form.ipsec_ike);
			if(ike_version == "2")
				existSubnetItem = document.getElementById("tr_net_" + _type + "_private_subnet").getElementsByClassName("input_25_table").length;
			var existSubnetItemList = "";
			var existSubnetObj = "";
			var is_ipv4 = false;
			var is_ipv6 = false;
			//var all_profile_subnet_list_array = all_profile_subnet_list.split(">");
			for(var i = 1 ; i <= existSubnetItem; i += 1) {
				existSubnetObj = document.getElementById("ipsec_" + _type + "_subnet_" + i);
				is_ipv4 = (existSubnetObj.value.indexOf(".") != -1) ? true : false;
				if(subnetIP_support_IPv6)
					is_ipv6 = (existSubnetObj.value.indexOf(":") != -1) ? true : false;
				if(!is_ipv4 && !is_ipv6) {
					alert(existSubnetObj.value + "<#JS_validip#>");
					existSubnetObj.focus();
					return false;
				}

				if(is_ipv4) {
					if(!validator.isLegalIPAndMask(existSubnetObj)) {
						return false;
					}

					var subnetIP = existSubnetObj.value.split("/")[0];
					var maskCIDR = parseInt(existSubnetObj.value.split("/")[1], 10);
					if (isNaN(maskCIDR) || (maskCIDR != 24 && maskCIDR != 23)){
						alert("Mask address must be 23 or 24.");/*untranslated*/
						existSubnetObj.focus();
						existSubnetObj.select();
						return false;
					}
					var subnetMask = createNetmaskAddr(maskCIDR);
					if(!checkGatewayIP(subnetIP, subnetMask)) {
						existSubnetObj.focus();
						existSubnetObj.select();
						return false;
					}
				}
				else if(is_ipv6) {
					if(!validator.isLegal_ipv6(existSubnetObj)) {
						return false;
					}
				}

				if(existSubnetItemList.search(existSubnetObj.value.split("/")[0]) != -1) {
					alert("Conflict with other subnet.");/*untranslated*/
					existSubnetObj.focus();
					existSubnetObj.select();
					return false;
				}
				existSubnetItemList += document.getElementById("ipsec_" + _type + "_subnet_" + i).value + ">";

				/*
				for(var j = 0 ; j < all_profile_subnet_list_array.length; j += 1) {
					if(j != document.form.ipsec_profile_item.value.split("_")[2]) {
						if(all_profile_subnet_list_array[j].search(existSubnetObj.value.split("/")[0]) != -1) {
							alert("Conflict with profile "+j+" subnet.");
							existSubnetObj.focus();
							return false;
						}
					}
				}
				*/
			}
			return true;
		};

		if(!valid_subnet("local"))
			return false;

		if(!valid_subnet("remote"))
			return false;

		if($('input:checkbox[name=ipsec_encryption_p1]:checked').length == 0) {
			alert("Please choose at least one Phase 1 Encryption.");/*untranslated*/
			return false;
		}

		if($('input:checkbox[name=ipsec_hash_p1]:checked').length == 0) {
			alert("Please choose at least one Phase 1 Hash.");/*untranslated*/
			return false;
		}

		if($('input:checkbox[name=ipsec_dh_group_p1]:checked').length == 0) {
			alert("Please choose at least one Diffile-Hellman Group");/*untranslated*/
			return false;
		}

		if($('input:checkbox[name=ipsec_encryption_p2]:checked').length == 0) {
			alert("Please choose at least one Phase 2 Encryption.");/*untranslated*/
			return false;
		}

		if($('input:checkbox[name=ipsec_hash_p2]:checked').length == 0) {
			alert("Please choose at least one Phase 2 Hash.");/*untranslated*/
			return false;
		}

		if(getRadioItemCheck(document.ipsec_form.ipsec_pfs) == "1") {
			if($('input:checkbox[name=ipsec_pfs_group]:checked').length == 0) {
				alert("Please choose at least one PFS Groups.");/*untranslated*/
				return false;
			}
		}

		return true;
	}

	if(validForm()) {
		$("#openvpnc_setting_ipsec").fadeOut(300);
		document.ipsec_form.action_script.value = "saveNvram";
		/* data structure
		1 vpn_type, profilename, remote_gateway_method, remote_gateway, 
		5 local_public_interface, local_public_ip, auth_method, auth_method_value, 
		9 local_subnet, local_port, remote_subnet, remote_port, 
		13 transport/tunnel type, virtual_ip, virtual_subnet, accessible_networks, 
		17 ike, encryption_p1, hash_p1, exchange,
		21 local id, remote id, keylife_p1, xauth,
		25 xauth_account, xauth_password, xauth_server_type, traversal,
		29 ike_isakmp, ike_isakmp_nat, ipsec_dpd, dead_peer_detection,
		33 encryption_p2, hash_p2, keylife_p2, keyingtries
		37 samba list, default Activate is 0;
		*/

		var profile_idx = 0;
		if(document.ipsec_form.ipsec_profile_item.value == "ipsec_profile_client_1") {
			profile_idx = 1;
		}
		if(document.ipsec_form.ipsec_profile_item.value == "ipsec_profile_client_2") {
			profile_idx = 2;
		}	
		if(document.ipsec_form.ipsec_profile_item.value == "ipsec_profile_client_3") {
			profile_idx = 3;
		}
		if(document.ipsec_form.ipsec_profile_item.value == "ipsec_profile_client_4") {
			profile_idx = 4;
		}
		if(document.ipsec_form.ipsec_profile_item.value == "ipsec_profile_client_5") {
			profile_idx = 5;
		}
		var result = "";
		var accessible_networks = "null";
		var get_subnet_list = function(_type) {
			var subnet_list = "";
			var existSubnetItem = 1;//ike v1 only single subnet
			var ike_version = getRadioItemCheck(document.ipsec_form.ipsec_ike);
			if(ike_version == "2")
				existSubnetItem = document.getElementById("tr_net_" + _type + "_private_subnet").getElementsByClassName("input_25_table").length;
			for(var i = 1 ; i <= existSubnetItem; i += 1) {
				subnet_list += "<" + document.getElementById("ipsec_" + _type + "_subnet_" + i).value;
			}
			return subnet_list;
		};

		var local_subnet_list = "";
		var remote_subnet_list = "";
		local_subnet_list = get_subnet_list("local");
		remote_subnet_list = get_subnet_list("remote");

		var local_public_ip = "";
		/*
		var wans_dualwan_array = '<% nvram_get("wans_dualwan"); %>'.split(" ");
		var wans_index = 0;
		for(var i = 0; i < wans_dualwan_array.length; i += 1) {
			if(wans_dualwan_array[i] == document.ipsec_form.ipsec_local_public_interface.value) {
				wans_index = i + 1;
			}
		}
		switch(wans_index) {
			case 0 :
				local_public_ip = "0.0.0.0";
				break;
			case 1 :
				local_public_ip = wanlink_ipaddr();
				break;
			case 2 :
				local_public_ip = secondary_wanlink_ipaddr();
				break;
		}
		*/

		var auth_method_vaule = document.ipsec_form.ipsec_preshared_key.value;
		var ipsec_profilename = document.ipsec_form.ipsec_profilename.value + "_c" + profile_idx;

		var profile_array = [ "", 
			"2", ipsec_profilename, getRadioItemCheck(document.ipsec_form.ipsec_remote_gateway_method), document.ipsec_form.ipsec_remote_gateway.value, 
			document.ipsec_form.ipsec_local_public_interface.value, local_public_ip, "1", auth_method_vaule, 
			local_subnet_list, document.ipsec_form.ipsec_local_port.value, remote_subnet_list, document.ipsec_form.ipsec_remote_port.value, 
			"tunnel", "", "", accessible_networks, 
			getRadioItemCheck(document.ipsec_form.ipsec_ike), "auto", "auto", getRadioItemCheck(document.ipsec_form.ipsec_exchange), 
			document.ipsec_form.ipsec_local_id.value, document.ipsec_form.ipsec_remote_id.value, document.ipsec_form.ipsec_keylife_p1.value, "0", 
			"", "", "eap-md5", "1", 
			document.ipsec_form.ipsec_ike_isakmp.value, document.ipsec_form.ipsec_ike_isakmp_nat.value, document.ipsec_form.ipsec_dpd.value, getRadioItemCheck(document.ipsec_form.ipsec_dead_peer_detection), 
			"auto", "auto", document.ipsec_form.ipsec_keylife_p2.value, document.ipsec_form.ipsec_keyingtries.value, 
			"null", 0
		];


		result = profile_array[1] + ">" + profile_array[2] + ">" + profile_array[3] + ">" + profile_array[4] + ">" + 
				profile_array[5] + ">" + profile_array[6] + ">" + profile_array[7] + ">" + profile_array[8] + ">" + 
				profile_array[9] + ">" + profile_array[10] + ">" + profile_array[11] + ">" + profile_array[12] + ">" +
				profile_array[13] + ">" + "null" + ">" + "null" + ">" +"null"+ ">" +
				profile_array[17] + ">" + profile_array[18] + ">" + profile_array[19] + ">" + profile_array[20] + ">" + 
				profile_array[21] + ">" + profile_array[22] + ">" + profile_array[23] + ">" + profile_array[24] + ">" + 
				profile_array[25] + ">" + profile_array[26] + ">" + profile_array[27] + ">" + profile_array[28] + ">" + 
				profile_array[29] + ">" + profile_array[30] + ">" + profile_array[31] + ">" + profile_array[32] + ">" + 
				profile_array[33] + ">" + profile_array[34] + ">" + profile_array[35] + ">" + profile_array[36] + ">" + 
				profile_array[37] + ">" + profile_array[38];
		
		/* data structure ext
		1 encryption_p1, hash_p1, DHGroup, encryption_p2, hash_p2, PFS
		*/
		var result_ext = "";
		var encryption_p1 = 0;
		var hash_p1 = 0;
		var dh_group = 0;
		var encryption_p2 = 0;
		var hash_p2 = 0;
		var pfs_group = 0;
		var get_checkboxlist = function(_objName) {
			var value = 0;
			$('input:checkbox[name=' + _objName + ']:checked').map(function() {
				value = value + parseInt($(this).val());
			});
			return value;
		};
		encryption_p1 = get_checkboxlist("ipsec_encryption_p1");
		hash_p1 = get_checkboxlist("ipsec_hash_p1");
		dh_group = get_checkboxlist("ipsec_dh_group_p1");
		encryption_p2 = get_checkboxlist("ipsec_encryption_p2");
		hash_p2 = get_checkboxlist("ipsec_hash_p2");
		if(getRadioItemCheck(document.ipsec_form.ipsec_pfs) == "1")
			pfs_group = get_checkboxlist("ipsec_pfs_group");
		var profile_ext_array = [encryption_p1, hash_p1, dh_group, encryption_p2, hash_p2, pfs_group];
		result_ext = profile_ext_array.join(">");

		document.getElementById(document.ipsec_form.ipsec_profile_item.value).value = result;
		document.getElementById(document.ipsec_form.ipsec_profile_item.value + "_ext").value = result_ext;

		document.ipsec_form.submit();

		switch(profile_idx) {
			case 1 :
				ipsec_profile_client_1 = result;
				ipsec_profile_client_1_ext = result_ext;
				break;
			case 2 :
				ipsec_profile_client_2 = result;
				ipsec_profile_client_2_ext = result_ext;
				break;
			case 3 :
				ipsec_profile_client_3 = result;
				ipsec_profile_client_3_ext = result_ext;
				break;
			case 4 :
				ipsec_profile_client_4 = result;
				ipsec_profile_client_4_ext = result_ext;
				break;
			case 5 :
				ipsec_profile_client_5 = result;
				ipsec_profile_client_5_ext = result_ext;
				break;
		}

		cancel_add_rule();
		gen_exception_list_table();
		show_vpnc_rulelist();
	}
}
function connect_Row_IPSec(rowdata) {
	var $activateItem = $(rowdata);
	var flag = $activateItem.hasClass("activate_icon") ? "disconnect" : "connect";
	var $connectActionRow = $(rowdata.parentNode.parentNode);
	var vpnc_idx = $connectActionRow.attr('id');
	var profileName = vpnc_idx.replace("vpnc_row_", "");
	var orgProfile = "";
	var tempProfile = "";
	var state = 0;
	var actionScript = "";
	if(flag == "disconnect") {
		$connectActionRow.addClass("deactivate_text");
		$activateItem.addClass("deactivate_icon");
		$activateItem.removeClass("activate_icon");	
		actionScript = "ipsec_stop_cli";
	}
	else {
		$connectActionRow.removeClass("deactivate_text");
		$activateItem.addClass("activate_icon");
		$activateItem.removeClass("deactivate_icon");
		state = 1;
		actionScript = "ipsec_start_cli";
	}

	switch (profileName) {
		case "ipsec_profile_client_1": 
			orgProfile = document.ipsec_form.ipsec_profile_client_1.value;
			tempProfile = orgProfile.substring(0, (orgProfile.length - 2)) + ">" + state;
			ipsec_profile_client_1 = tempProfile;
			document.ipsec_form.ipsec_profile_client_1.value = tempProfile;
			break;
		case "ipsec_profile_client_2": 
			orgProfile = document.ipsec_form.ipsec_profile_client_2.value;
			tempProfile = orgProfile.substring(0, (orgProfile.length - 2)) + ">" + state;
			ipsec_profile_client_2 = tempProfile;
			document.ipsec_form.ipsec_profile_client_2.value = tempProfile;
			break;
		case "ipsec_profile_client_3": 
			orgProfile = document.ipsec_form.ipsec_profile_client_3.value;
			tempProfile = orgProfile.substring(0, (orgProfile.length - 2)) + ">" + state;
			ipsec_profile_client_3 = tempProfile;
			document.ipsec_form.ipsec_profile_client_3.value = tempProfile;
			break;
		case "ipsec_profile_client_4": 
			orgProfile = document.ipsec_form.ipsec_profile_client_4.value;
			tempProfile = orgProfile.substring(0, (orgProfile.length - 2)) + ">" + state;
			ipsec_profile_client_4 = tempProfile;
			document.ipsec_form.ipsec_profile_client_4.value = tempProfile;
			break;
		case "ipsec_profile_client_5": 
			orgProfile = document.ipsec_form.ipsec_profile_client_5.value;
			tempProfile = orgProfile.substring(0, (orgProfile.length - 2)) + ">" + state;
			ipsec_profile_client_5 = tempProfile;
			document.ipsec_form.ipsec_profile_client_5.value = tempProfile;
			break;
	}
	document.ipsec_form.action_script.value = actionScript;
	document.ipsec_form.submit();
}
var ipsec_connect_status_array = new Array();
function update_connect_status() {
	$.ajax({
		url: '/ajax_ipsec.asp',
		dataType: 'script',
		timeout: 1500,
		error: function(xhr){
			setTimeout("update_connect_status();",1000);
		},	
		success: function() {
			ipsec_connect_status_array = [];
			for(var i = 0; i < ipsec_connect_status.length; i += 1) {
				ipsec_connect_status_array[ipsec_connect_status[i][0]] = ipsec_connect_status[i][1];
			}
			setTimeout("update_connect_status();",3000);
		}
	});
}
function changeIKEVersion() {
	var ike_version = getRadioItemCheck(document.ipsec_form.ipsec_ike);
	switch(ike_version) {
		case "1" :
			showhide("tr_adv_exchange_mode", 1);
			break;
		case "2" :
			showhide("tr_adv_exchange_mode", 0);
			settingRadioItemCheck(document.ipsec_form.ipsec_exchange, "0");
			changeExchangeMode();
			break;
	}
	controlSubnetStatus(ike_version, "local");
	controlSubnetStatus(ike_version, "remote");
}
function changeExchangeMode() {
	var clickItem = getRadioItemCheck(document.ipsec_form.ipsec_exchange);
	$("#exchange_mode_hint").css("display", "none");
	if(clickItem == "1") {
		$("#exchange_mode_hint").css("display", "");
	}
}

function parseArrayToStr_vpnc_clientlist() {
	var vpnc_clientlist_str = "";
	for(var i = 0; i < vpnc_clientlist_array.length; i += 1) {
		if(vpnc_clientlist_array[i].length != 0) {
			if(i != 0)
				vpnc_clientlist_str += "<";
			for(var j = 0; j < vpnc_clientlist_array[i].length; j += 1) {
				vpnc_clientlist_str += vpnc_clientlist_array[i][j];
				if( (j + 1) != vpnc_clientlist_array[i].length)
					vpnc_clientlist_str += ">";
			}
		}
	}
	return vpnc_clientlist_str;
}
function parseArrayToStr_vpnc_pptp_options_x_list() {
	var vpnc_pptp_options_x_list_str = "";
	for(var i = 0; i < vpnc_pptp_options_x_list_array.length; i += 1) {
		if(vpnc_pptp_options_x_list_array[i].length != 0) {
			vpnc_pptp_options_x_list_str += "<";
			for(var j = 0; j < vpnc_pptp_options_x_list_array[i].length; j += 1) {
				vpnc_pptp_options_x_list_str += vpnc_pptp_options_x_list_array[i][j];
			}
		}
	}
	return vpnc_pptp_options_x_list_str;
}
function changeRemoteGatewayMethod() {
	$("#ipsec_remote_gateway").removeAttr("maxlength");
	$('#ipsec_remote_gateway').unbind("keypress");
	var clickItem = getRadioItemCheck(document.ipsec_form.ipsec_remote_gateway_method);
	if(clickItem == "0") {
		$("#ipsec_remote_gateway").keypress(function() {
			return validator.isIPAddr(this,event);
		});
		$("#ipsec_remote_gateway").attr("maxlength", "15");
		$("#ipsec_remote_id_hint").css("display", "");
	}
	else if(clickItem == "1") {
		$("#ipsec_remote_gateway").attr("maxlength", "64");
		$("#ipsec_remote_id_hint").css("display", "none");
	}
}
function add_subnet_item(obj, _type) {
	var existSubnetItem = obj.parentNode.getElementsByClassName("input_25_table").length;
	if(existSubnetItem > 3) {
		alert("<#JS_itemlimit1#> " + existSubnetItem + " <#JS_itemlimit2#>");
	}
	else {
		var code = "";
		var divObj = document.createElement("input");
		divObj.type = "text";
		divObj.className = "input_25_table";
		divObj.autocomplete = "off";
		divObj.autocapitalize = "off";
		divObj.id = "ipsec_" + _type + "_subnet_" + (existSubnetItem + 1);
		if(subnetIP_support_IPv6)
			divObj.maxLength = "39";
		else {
			divObj.placeholder = "(ex.10.10.10.0/24)";
			divObj.maxLength = "18";
			divObj.onkeypress = function() {
				return validator.isIPAddrPlusNetmask(this,event);
			};
		}
		divObj.style.marginTop = "4px";
		//divObj.onkeypress = function(){return validator.isIPAddr(this, event);};
		obj.parentNode.appendChild(divObj);

		var removeElement = function(element) {
			element && element.parentNode && element.parentNode.removeChild(element);
		};
		if(document.getElementById("btDelRemoteSubnet_" + _type) != null) {
			removeElement(document.getElementById("btDelRemoteSubnet_" + _type));
		}
		var divObj = document.createElement("input");
		divObj.id = "btDelRemoteSubnet_" + _type;
		divObj.type = "text";
		divObj.className = "remove_btn";
		divObj.style.height = "27px";
		divObj.onclick = function() { del_subnet_item(this, _type);};
		obj.parentNode.appendChild(divObj);
	}
}
function del_subnet_item(obj, _type) {

	var delIndex = obj.parentNode.getElementsByClassName('input_25_table').length;
	var removeElement = function(element) {
	    element && element.parentNode && element.parentNode.removeChild(element);
	};
	if(document.getElementById("ipsec_" + _type + "_subnet_" + delIndex) != null) {
		removeElement(document.getElementById("ipsec_" + _type + "_subnet_"+ delIndex));
	}
	if(delIndex == "2") {
		if(document.getElementById("btDelRemoteSubnet_" + _type) != null) {
			removeElement(document.getElementById("btDelRemoteSubnet_" + _type));
		}
	}
}
function controlSubnetStatus(_ikeVersion, _type) {
	switch(_ikeVersion) {
		case "1" :
			$("#td_net_" + _type +"_private_subnet").children(".add_btn").css("display", "none");
			$("#td_net_" + _type +"_private_subnet").children(".input_25_table").css("display", "none");
			$("#td_net_" + _type +"_private_subnet").children(".input_25_table").eq(0).css("display", "");
			$("#td_net_" + _type +"_private_subnet").children(".remove_btn").css("display", "none");
			break;
		case "2" :
			$("#td_net_" + _type +"_private_subnet").children(".add_btn").css("display", "");
			$("#td_net_" + _type +"_private_subnet").children(".input_25_table").css("display", "");
			$("#td_net_" + _type +"_private_subnet").children(".remove_btn").css("display", "");
			break;
	}
}
function changePFS() {
	var clickItem = getRadioItemCheck(document.ipsec_form.ipsec_pfs);
	if(clickItem == "0")
		$("#tr_adv_pfs_group").hide();
	else
		$("#tr_adv_pfs_group").show();
}
function parseArrayToStr_vpnc_dev_policy_list(_array) {
	var vpnc_dev_policy_list = "";
	for(var i = 0; i < _array.length; i += 1) {
		if(_array[i].length != 0) {
			if(_array[i].length == 5) {
				if(i != 0)
					vpnc_dev_policy_list += "<";

				var temp_mac = _array[i][0];
				var temp_ipaddr = _array[i][1];
				var temp_dns = _array[i][2];
				var temp_vpnc_idx = _array[i][3];
				var temp_active = _array[i][4];
				var temp_destination_ip = "";
				vpnc_dev_policy_list += temp_active + ">" + temp_ipaddr + ">" + temp_destination_ip + ">" + temp_vpnc_idx;
			}
		}
	}
	return vpnc_dev_policy_list;
}
function parseArrayToStr_dhcp_staticlist() {
	var dhcp_staticlist = "";
	for(var i = 0; i < vpnc_dev_policy_list_array.length; i += 1) {
		if(vpnc_dev_policy_list_array[i].length != 0) {
			dhcp_staticlist += "<";

			if(vpnc_dev_policy_list_array[i].length == 5) {
				var temp_mac = vpnc_dev_policy_list_array[i][0];
				var temp_ip = vpnc_dev_policy_list_array[i][1];
				var temp_dns = vpnc_dev_policy_list_array[i][2];
				if(!validator.ipv4_addr(temp_dns))
					temp_dns = "";
				dhcp_staticlist += temp_mac + ">" + temp_ip + ">" + temp_dns;
			}
		}
	}
	return dhcp_staticlist;
}

function control_exception_list_status(_$this) {
	var activate_status = "0";
	if(_$this.find('.deactivate_icon').length != 0) {
		_$this.find('.deactivate_icon').each(function() {
			$(this).removeClass('deactivate_icon').addClass('activate_icon');
		});
		activate_status = "1";
		document.vpnc_dev_policy_form.action_script.value = "restart_vpnc_dev_policy";
	}
	else if(_$this.find('.activate_icon').length != 0) {
		_$this.find('.activate_icon').each(function() {
			$(this).removeClass('activate_icon').addClass('deactivate_icon');
		});
		activate_status = "0";
		document.vpnc_dev_policy_form.action_script.value = "stop_vpnc_dev_policy";
	}

	var rawIdx = _$this.closest("*[row_tr_idx]").attr( "row_tr_idx" );
	var colIdx =  _$this.closest("*[row_td_idx]").attr( "row_td_idx" );
	vpnc_dev_policy_list_array[rawIdx][colIdx] = activate_status;
}
function update_exception_list(_parArray) {
	gen_exception_list_table();
}
function set_default_connection(_idx) {
	$.ajax({
		url: '/ajax_vpn_fusion.asp',
		dataType: 'script',
		error: function(xhr) {
			setTimeout("set_default_connection(" + _idx + ");",1000);
		},
		success: function() {
			if(_idx == "ipsec") {
				alert("<#VPN_Fusion_Default_Alert#>");
			}
			else {
				if(default_wan_setted_state != "") {
					var default_wan_setted_flag = false;
					if(_idx == "0")
						default_wan_setted_flag = true;
					else {
						var default_wan_setted_state_array =  parseNvramToArray(default_wan_setted_state, 2);
						for(var i = 0; i < default_wan_setted_state_array.length; i += 1) {
							if(default_wan_setted_state_array[i][0] == _idx) {
								if(default_wan_setted_state_array[i][1] == "0")
									default_wan_setted_flag = true;
								break;
							}
						}
					}
					if(default_wan_setted_flag) {
						vpnc_default_wan = _idx;
						document.vpnc_default_wan_form.vpnc_default_wan_tmp.value = vpnc_default_wan;
						document.vpnc_default_wan_form.action_script.value = "restart_default_wan";
						document.vpnc_default_wan_form.submit();
						$(".default_disconnect").css("border-color", "#8D8D8D");
						$(".default_disconnect").empty();
						$("#vpnc_default_wan_status_" + vpnc_default_wan + "").html('<div class="default_connect"></div>');
						$("#vpnc_default_wan_status_" + vpnc_default_wan + "").css("border-color", "#08fff0");
					}
					else {
						alert("<#VPN_Fusion_Default_Alert#>");
					}
				}
			}
		}
	});
}
function get_new_vpnc_index() {
	$.ajax({
		url: '/ajax_vpn_fusion.asp',
		dataType: 'script',
		error: function(xhr) {
			setTimeout("get_new_vpnc_index();",1000);
		},
		success: function() {
			if(new_vpnc_index != "")
				vpnc_index = new_vpnc_index;
		}
	});
}
function get_vpnc_profile_status() {
	$.ajax({
		url: '/ajax_vpn_fusion.asp',
		dataType: 'script',
		error: function(xhr) {
			setTimeout("get_vpnc_profile_status();",1000);
		},
		success: function() {
			if(vpnc_profile_status != "") {
				vpnc_status_array = parseNvramToArray(vpnc_profile_status, 3);
				update_vpnc_profile_status(vpnc_status_array);
			}
			setTimeout("get_vpnc_profile_status();",1000);
		}
	});
}
function update_vpnc_profile_status(_vpnc_status_array) {
	for (var i = 0; i < _vpnc_status_array.length; i++) {
		var vpnc_status = _vpnc_status_array[i][0];
		var vpnc_reason = _vpnc_status_array[i][1];
		var vpnc_idx = _vpnc_status_array[i][2];
		if($("#vpnc_row_" + vpnc_idx + "").length > 0) {
			var status_hint = "<#CTL_Disconnect#>";
			switch(vpnc_status) {
				case "5" :
					status_hint = "<#CTL_Disconnect#>";
					break;
				case "0" :
				case "4" :
					switch(vpnc_reason) {
						case "0" :
							status_hint = "<img title='<#usb_initializing#>' src='/images/InternetScan.gif'>";
							break;
						case "1" :
							status_hint = "<div title='<#vpn_openvpn_conflict#>' class='vpnc_ipconflict_icon'></div>";
							break;
						case "2" :
							status_hint = "<img title='<#qis_fail_desc1#>' src='/images/button-close2.png' style='width:25px;'>";
							break;
						default :
							status_hint = "<img title='<#ConnectionFailed#>' src='/images/button-close2.png' style='width:25px;'>";
					}
					break;
				case "1" :
					status_hint = "<img title='<#CTL_Add_enrollee#>' src='/images/InternetScan.gif'>";
					break;
				case "2" :
					status_hint = "<img title='<#Connected#>' src='/images/checked_parentctrl.png' style='width:25px;'>";
					break;
			}

			$("#vpnc_row_" + vpnc_idx + "").children(":first").next().html(status_hint);
		}
	}
}
function applyExceptionList() {
	document.vpnc_dev_policy_form.vpnc_dev_policy_list.disabled = false;
	document.vpnc_dev_policy_form.vpnc_dev_policy_list.value = parseArrayToStr_vpnc_dev_policy_list(vpnc_dev_policy_list_array);
	if(vpnc_dev_policy_list_array.toString() != vpnc_dev_policy_list_array_ori.toString()) {
		document.vpnc_dev_policy_form.vpnc_dev_policy_list_tmp.disabled = false;
		document.vpnc_dev_policy_form.vpnc_dev_policy_list_tmp.value = parseArrayToStr_vpnc_dev_policy_list(vpnc_dev_policy_list_array_ori);
	}
	document.vpnc_dev_policy_form.vpnc_policy_unit.disabled = true;
	document.vpnc_dev_policy_form.vpnc_tmp_policy.disabled = true;
	document.vpnc_dev_policy_form.dhcp_static_x.disabled = false;
	document.vpnc_dev_policy_form.dhcp_static_x.value = "1";
	document.vpnc_dev_policy_form.dhcp_staticlist.disabled = false;
	document.vpnc_dev_policy_form.dhcp_staticlist.value = parseArrayToStr_dhcp_staticlist();

	if(dhcp_staticlist_ori != document.vpnc_dev_policy_form.dhcp_staticlist.value) {
		document.vpnc_dev_policy_form.flag.value = "";
		document.vpnc_dev_policy_form.action_wait.value = "35";
		document.vpnc_dev_policy_form.action_script.value = "restart_vpnc_dev_policy;restart_net_and_phy;";
	}
	else {
		showLoading(3);
		document.vpnc_dev_policy_form.action_wait.value = "3";
		document.vpnc_dev_policy_form.action_script.value = "restart_vpnc_dev_policy";
	}

	document.vpnc_dev_policy_form.submit();

	vpnc_dev_policy_list_array_ori = JSON.parse(JSON.stringify(vpnc_dev_policy_list_array));
}
function edit_exception_list_confirm(_parArray) {
	var activate_status = "0";
	activate_status = _parArray[4];
	if(activate_status == "1") {
		alert("<#VPN_Fusion_Deactivate_Editor_Alert#>");
		return false;
	}

	return true;
}
function del_exception_list_confirm(_parArray) {
	var activate_status = "0";
	var ipAddr = "";
	ipAddr = _parArray[1];
	activate_status = _parArray[4];
	if(activate_status == "1") {
		alert("<#VPN_Fusion_Deactivate_Delete_Alert#>");
		return false;
	}
	else {
		var manually_dhcp_list_array_ori = new Array();
		var dhcp_staticlist_array = '<% nvram_get("dhcp_staticlist"); %>';
		var dhcp_staticlist_row = dhcp_staticlist_array.split('&#60');
		for(var i = 1; i < dhcp_staticlist_row.length; i += 1) {
			var dhcp_staticlist_col = dhcp_staticlist_row[i].split('&#62');
			var item_para = {"mac" : dhcp_staticlist_col[0].toUpperCase(), "dns" : (dhcp_staticlist_col[2] == undefined) ? "" : dhcp_staticlist_col[2]};
			manually_dhcp_list_array_ori[dhcp_staticlist_col[1]] = item_para;
		}
		if(manually_dhcp_list_array_ori[ipAddr] != undefined) {
			if(!confirm("<#VPN_Fusion_Exception_Policy_And_IP_Binding_Delete_Alert#>"))
				return false;
		}
	}

	return true;
}
</script>
</head>

<body onload="initial();" onunload="unload_body();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_VPNClient_Content.asp">
<input type="hidden" name="next_page" value="Advanced_VPNClient_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="flag" value="background">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_vpnc">
<input type="hidden" name="action_wait" value="3">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="vpnc_dnsenable_x" value="<% nvram_get("vpnc_dnsenable_x"); %>">
<input type="hidden" name="vpnc_defroute_x" value="<% nvram_get("vpnc_defroute_x"); %>">
<input type="hidden" name="vpnc_clientlist" value='<% nvram_get("vpnc_clientlist"); %>'>
<input type="hidden" name="vpnc_type" value="PPTP">
<input type="hidden" name="vpn_client_unit" value="1">
<input type="hidden" name="vpnc_pptp_options_x_list" value="<% nvram_get("vpnc_pptp_options_x_list"); %>">
<input type="hidden" name="vpnc_unit" value="<% nvram_get("vpnc_unit"); %>">
<input type="hidden" name="ctf_nonat_force" value="<% nvram_get("ctf_nonat_force"); %>" disabled>
<div id="openvpnc_setting"  class="contentM_qis pop_div_bg" style="box-shadow: 1px 5px 10px #000;">
	<table class="QISform_wireless" border=0 align="center" cellpadding="5" cellspacing="0">
		<tr style="height:32px;">
			<td>
				<div id="divTabMenu_pptp"></div>
			</td>
		</tr>
		<tr>
			<td>
				<!---- vpnc_pptp/l2tp start  ---->
				<div>
				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
			 		<tr>
						<th><#IPConnection_autofwDesc_itemname#></th>
					  <td>
					  	<input type="text" maxlength="64" name="vpnc_des_edit" value="" class="input_32_table" style="float:left;" autocorrect="off" autocapitalize="off"></input>
					  </td>
					</tr>  		
					<tr>
						<th><#BOP_isp_heart_item#></th>
						<td>
							<input type="text" maxlength="64" name="vpnc_svr_edit" value="" class="input_32_table" style="float:left;" autocorrect="off" autocapitalize="off"></input>
						</td>
					</tr>  		
					<tr>
						<th><#Username#></th>
						<td>
							<input type="text" maxlength="64" name="vpnc_account_edit" value="" class="input_32_table" style="float:left;" autocomplete="off" autocorrect="off" autocapitalize="off"></input>
						</td>
					</tr>  		
					<tr>
						<th><#HSDPAConfig_Password_itemname#></th>
						<td>
							<input type="text" maxlength="64" name="vpnc_pwd_edit" value="" class="input_32_table" style="float:left;" autocomplete="off" autocorrect="off" autocapitalize="off"></input>
						</td>
					</tr>
					<tr id="trPPTPOptions">
						<th><#PPPConnection_x_PPTPOptions_itemname#></th>
						<td>
							<select name="selPPTPOption" class="input_option" onchange="pptpOptionChange();">
								<option value="auto"><#Auto#></option>
								<option value="-mppc"><#No_Encryp#></option>
								<option value="+mppe-40">MPPE 40</option>
								<option value="+mppe-128">MPPE 128</option>
							</select>
							<div id="pptpOptionHint" style="display:none;">
								<span><#PPTPOptions_OpenVPN_hint#></span>
							</div>
						</td>	
					</tr>		 
		 		</table>
		 		</div>
		 		<!---- vpnc_pptp/l2tp end  ---->		 			 	
			</td>
		</tr>
	</table>		
	<div style="margin-top:5px;padding-bottom:10px;width:100%;text-align:center;">
		<input class="button_gen" type="button" onclick="cancel_add_rule();" id="cancelBtn" value="<#CTL_Cancel#>">
		<input class="button_gen" type="button" onclick="addRow_Group(16,save_flag, idx_tmp);" value="<#CTL_ok#>">	
	</div>	
      <!--===================================Ending of openvpnc setting Content===========================================-->			
</div>
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
		<table width="95%" border="0" align="left" cellpadding="0" cellspacing="0" class="FormTitle" id="FormTitle">
  		<tr>
    		<td bgcolor="#4D595D" valign="top">
    			<table width="760px" border="0" cellpadding="4" cellspacing="0">
					<tr>
						<td bgcolor="#4D595D" valign="top">
							<div>&nbsp;</div>
							<div class="formfonttitle">VPN - <#VPN_Fusion#></div>
							<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
							<div>
								<div class='vpn_illustration'></div>
								<div class="formfontdesc" style="float:left;width:75%;">
									<#VPN_Fusion#>
									<#VPN_Fusion_desc1#>
									<br>
									<#VPN_Fusion_desc2#>
								</div>
							</div>
							<div style="clear:both;"></div>
							<div class="formfontdesc">
								<#VPN_Fusion_desc_step#>
								<br>
								1. <#VPN_Fusion_desc_step1#>
								<br>
								2. <#VPN_Fusion_desc_step2#>
								<br>
								3. <#VPN_Fusion_desc_step3#>
								<br>
								4. <#VPN_Fusion_desc_step4#>
								<br>
								5. <#VPN_Fusion_desc_step5#>
							</div>
							<div class="formfontdesc" style="margin-bottom:-10px;">
								<#VPN_Fusion_FAQ#>
							</div>
						</td>
        			</tr>
					<tr>
						<td>
							<div class="addRuleFrame">
								<div class="addRuleText"><#VPN_Fusion_Server_List#> ( <#List_limit#> 16 )</div>
								<div class="add_btn" style="float:left;" onclick="Add_profile(16)"></div>
							</div>
							<span style="color:#FC0;margin-left:5px;"><#VPN_Fusion_Server_List_hint#></span>
							<table width="98%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table">
								<tr>
									<th width="10%"><a class='hintstyle' href='javascript:void(0);' onClick='openHint(32, 1);'><#Setting_factorydefault_value#></a></th>
									<th width="15%"><#Status_Str#></th>
									<th width="30%"><#VPN_Fusion_Connection_Name#></th>
									<th width="15%"><#QIS_internet_vpn_type#></th>
									<th width="15%"><#VPN_Fusion_Activate#></th>
									<th width="15%"><#VPN_Fusion_Editor#></th>
								</tr>
							</table>
							<div id="vpnc_clientlist_Block"></div>
							
						</td>
					</tr>
					<tr>
						<td>
							<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
						</td>	
					</tr>
					<tr>
						<td>
							<div id="exception_list_table"></div>
							<div class="apply_gen">
								<input type="button" name="button" class="button_gen" onclick="applyExceptionList();" value="<#CTL_apply#>"/>
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
<!---- vpnc_IPSEc start ---->
<form method="post" name="ipsec_del_form" id="list_form" action="/start_apply2.htm" target="hidden_frame">
	<input type="hidden" name="current_page" value="Advanced_VPN_IPSec.asp">
	<input type="hidden" name="next_page" value="Advanced_VPN_IPSec.asp">
	<input type="hidden" name="modified" value="0">
	<input type="hidden" name="flag" value="background">
	<input type="hidden" name="action_mode" value="apply">
	<input type="hidden" name="action_script" value="saveNvram">
	<input type="hidden" name="action_wait" value="1">
	<input type="hidden" name="ipsec_profile_client_1" value="">
	<input type="hidden" name="ipsec_profile_client_2" value="">
	<input type="hidden" name="ipsec_profile_client_3" value="">
	<input type="hidden" name="ipsec_profile_client_4" value="">
	<input type="hidden" name="ipsec_profile_client_5" value="">
	<input type="hidden" name="ipsec_profile_client_1_ext" value="">
	<input type="hidden" name="ipsec_profile_client_2_ext" value="">
	<input type="hidden" name="ipsec_profile_client_3_ext" value="">
	<input type="hidden" name="ipsec_profile_client_4_ext" value="">
	<input type="hidden" name="ipsec_profile_client_5_ext" value="">
</form>
<form method="post" name="ipsec_form" action="/start_apply.htm" target="hidden_frame">
	<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
	<input type="hidden" name="current_page" value="Advanced_VPN_IPSec.asp">
	<input type="hidden" name="next_page" value="Advanced_VPN_IPSec.asp">
	<input type="hidden" name="group_id" value="">
	<input type="hidden" name="modified" value="0">
	<input type="hidden" name="flag" value="background">
	<input type="hidden" name="action_mode" value="apply">
	<input type="hidden" name="action_script" value="ipsec_set_cli">
	<input type="hidden" name="action_wait" value="3">
	<input type="hidden" name="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
	<input type="hidden" name="ipsec_profile_item" value="">
	<input type="hidden" name="ipsec_profile_client_1" id="ipsec_profile_client_1" value="">
	<input type="hidden" name="ipsec_profile_client_2" id="ipsec_profile_client_2" value="">
	<input type="hidden" name="ipsec_profile_client_3" id="ipsec_profile_client_3" value="">
	<input type="hidden" name="ipsec_profile_client_4" id="ipsec_profile_client_4" value="">
	<input type="hidden" name="ipsec_profile_client_5" id="ipsec_profile_client_5" value="">
	<input type="hidden" name="ipsec_profile_client_1_ext" id="ipsec_profile_client_1_ext" value="">
	<input type="hidden" name="ipsec_profile_client_2_ext" id="ipsec_profile_client_2_ext" value="">
	<input type="hidden" name="ipsec_profile_client_3_ext" id="ipsec_profile_client_3_ext" value="">
	<input type="hidden" name="ipsec_profile_client_4_ext" id="ipsec_profile_client_4_ext" value="">
	<input type="hidden" name="ipsec_profile_client_5_ext" id="ipsec_profile_client_5_ext" value="">
	<div id="openvpnc_setting_ipsec" class="contentM_qis pop_div_bg" style="box-shadow: 1px 5px 10px #000;">
	<table class="QISform_wireless" border=0 align="center" cellpadding="5" cellspacing="0">
		<tr style="height:32px;">
			<td>
				<div id="divTabMenu_ipsec"></div>
			</td>
		</tr>
		<tr id="tr_ipsec_limit_hint" style="display:none;">
			<td>
				<div class="vpnc_limit_hint"></div>
			</td>
		</tr>
		<tr>
			<td>
				<div class="formfonttitle"><#vpnc_net_client_peer#></div>
				<div class="formfontdesc">
					<#vpnc_net_client_peer_desc1#>
					<br>
					<#vpnc_net_client_peer_desc2#>
					<br>
					<a id="ipsec_vpn_type_faq" href="" target="_blank" style="text-decoration:underline;color:#FC0;"></a>
				</div>
				<!-- VPN Type table start-->
				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					<thead>
					<tr>
						<td colspan="2"><#vpn_ipsec_Quick_Select#></td>
					</tr>
					</thead>
					<tr id="tr_SettingsMode">
						<th><#vpn_Adv#></th>
						<td>
							<select id="selSwitchMode" onchange="switchSettingsMode(this.options[this.selectedIndex].value)" class="input_option">
								<option value="1" selected><#menu5_1_1#></option>
								<option value="2"><#menu5#></option>
							</select>
						</td>
					</tr>
				</table>
				<!-- VPN Type table end-->
				<!-- Basic settings table start-->
				<table id="ipsec_basic_settings" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:15px;">
					<thead>
					<tr>
						<td colspan="2"><#t2BC#></td>
					</tr>
					</thead>
					<tr>
						<th><#vpn_ipsec_VPN_Profile_Name#></th>
						<td>
							<input type="text" class="input_25_table" name="ipsec_profilename" autocomplete="off" autocorrect="off" autocapitalize="off">
						</td>
					</tr>
					<tr id="tr_remote_gateway_method">
						<th><#vpn_ipsec_Remote_Gateway_Type#></th>
						<td>
							<input type="radio" name="ipsec_remote_gateway_method" id="ipsec_remote_gateway_ip" class="input" value="0" onchange="changeRemoteGatewayMethod()" checked>
							<label for='ipsec_remote_gateway_ip' id="ipsec_remote_gateway_ip_label"><#vpn_ipsec_Static_IP#></label>
							<input type="radio" name="ipsec_remote_gateway_method" id="ipsec_remote_gateway_ddns" class="input" value="1" onchange="changeRemoteGatewayMethod()">
							<label for='ipsec_remote_gateway_ddns' id="ipsec_remote_gateway_ddns_label"><#LANHostConfig_x_LDNSServer1_itemname#></label>
						</td>
					</tr>
					<tr id="tr_remote_gateway">
						<th><#vpn_ipsec_Remote_Gateway#></th>
						<td>
							<input type="text" class="input_25_table" name="ipsec_remote_gateway" id="ipsec_remote_gateway" autocomplete="off" autocorrect="off" autocapitalize="off">
						</td>
					</tr>
					<tr>
						<th><#vpn_ipsec_Local_Interface#></th>
						<td>
							<select name="ipsec_local_public_interface" class="input_option"></select>
						</td>
					</tr>
					<tr id="tr_presharedKey">
						<th><#vpn_ipsec_PreShared_Key#></th>
						<td>
							<input id="ipsec_preshared_key" name="ipsec_preshared_key" type="password" onBlur="switchType(this, false);" onFocus="switchType(this, true);" class="input_25_table" maxlength="32" placeholder="<#vpn_preshared_key_hint#>" autocomplete="off" autocorrect="off" autocapitalize="off">
						</td>
					</tr>
					<tr id="tr_adv_local_id">
						<th><#vpn_ipsec_Local_ID#></th>
						<td>
							<input type="text" class="input_25_table" name="ipsec_local_id" placeholder="<#IPConnection_ExternalIPAddress_itemname#>、FQDN、<#AiProtection_WebProtector_EMail#> or DN" autocomplete="off" autocorrect="off" autocapitalize="off">
							<span style="color:#FC0">(Optional)<!--untranslated--></span>
						</td>
					</tr>
					<tr id="tr_adv_remote_id">
						<th><#vpn_ipsec_Remote_ID#></th>
						<td>
							<input type="text" class="input_25_table" name="ipsec_remote_id" placeholder="<#IPConnection_ExternalIPAddress_itemname#>、FQDN、<#AiProtection_WebProtector_EMail#> or DN" autocomplete="off" autocorrect="off" autocapitalize="off">
							<span id="ipsec_remote_id_hint" style="color:#FC0">(Optional)<!--untranslated--></span>
						</td>
					</tr>
				</table>
				<!-- Basic settings table end-->
				<!-- Network table start-->
				<table id="ipsec_network_settings" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:15px;">
					<thead>
					<tr>
						<td id="td_network_title" colspan="2">Network - <#Subnet#></td>
					</tr>
					</thead>
					<tr id="tr_net_local_private_subnet">
						<th><#vpn_ipsec_Local_Subnet#></th>
						<td id="td_net_local_private_subnet"></td>
					</tr>
					<tr id="tr_net_local_port">
						<th><#vpn_ipsec_Local_Port#></th>
						<td>
							<input type="text" class="input_6_table" name="ipsec_local_port" maxlength="5" value="0" onKeyPress="return validator.isNumber(this,event)" autocomplete="off" autocorrect="off" autocapitalize="off">
							<span style="color:#FC0">(0-65535)</span>
						</td>
					</tr>
					<tr id="tr_net_remote_private_subnet">
						<th><#vpn_ipsec_Remote_Subnet#></th>
						<td id="td_net_remote_private_subnet"></td>
					</tr>
					<tr id="tr_net_remote_port">
						<th><#vpn_ipsec_Remote_Port#></th>
						<td>
							<input type="text" class="input_6_table" name="ipsec_remote_port" maxlength="5" value="0" onKeyPress="return validator.isNumber(this,event)" autocomplete="off" autocorrect="off" autocapitalize="off">
							<span style="color:#FC0">(0-65535)</span>
						</td>
					</tr>
					<tr id="tr_net_transport">
						<th><#DSL_Mode#></th>
						<td>
							<#vpn_ipsec_Tunnel#>
							<!--select name="ipsec_transport" class="input_option">
								<option value="tunnel">Tunnel</option>
								<option value="transport">Transport</option>
								<option value="transport_proxy">Transport Proxy</option>
								<option value="passthrough">Passthrough</option>
								<option value="drop">Drop</option>
							</select-->
						</td>
					</tr>	
				</table>
				<!-- Network table end-->

				<!-- Advanced Settings table start-->
				<div id="ipsec_advanced_settings" style="display:none;">
					<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:15px;">
						<thead>
						<tr>
							<td colspan="2">Advanced Settings - <#vpn_ipsec_Phase_1_Negotiations#></td>
						</tr>
						</thead>
						<tr id="tr_adv_ike_version">
							<th><#vpn_ipsec_IKE_Version#></th>
							<td>
								<input type="radio" name="ipsec_ike" id="ipsec_ike_v1" class="input" value="1" onchange="changeIKEVersion()" checked>
								<label for='ipsec_ike_v1' id="ipsec_ike_v1_label">v1<!--untranslated--></label>
								<input type="radio" name="ipsec_ike" id="ipsec_ike_v2" class="input" value="2" onchange="changeIKEVersion()">
								<label for='ipsec_ike_v2' id="ipsec_ike_v2_label">v2<!--untranslated--></label>
							</td>
						</tr>
						<tr id="tr_adv_encryption_p1">
							<th><#vpn_ipsec_Encryption#></th>
							<td>
								<!--label><input type="checkbox" name="ipsec_encryption_p1" value="1">DES</label-->
								<label><input type="checkbox" name="ipsec_encryption_p1" value="2">3DES</label>
								<label><input type="checkbox" name="ipsec_encryption_p1" value="4">AES128</label>
								<!--label><input type="checkbox" name="ipsec_encryption_p1" value="8">AES192</label>
								<label><input type="checkbox" name="ipsec_encryption_p1" value="16">AES256</label-->
							</td>
						</tr>
						<tr id="tr_adv_hash_p1">
							<th><#vpn_ipsec_Hash#></th>
							<td>
								<!--label><input type="checkbox" name="ipsec_hash_p1" value="1">MD5</label-->
								<label><input type="checkbox" name="ipsec_hash_p1" value="2">SHA1</label>
								<label><input type="checkbox" name="ipsec_hash_p1" value="4">SHA256</label>
								<!--label><input type="checkbox" name="ipsec_hash_p1" value="8">SHA384</label>
								<label><input type="checkbox" name="ipsec_hash_p1" value="16">SHA512</label-->
							</td>
						</tr>
						<tr id="tr_adv_dh_group">
							<th>Diffile-Hellman Groups<!--untranslated--></th>
							<td>
								<label><input type="checkbox" name="ipsec_dh_group_p1" value="1">1</label>
								<label><input type="checkbox" name="ipsec_dh_group_p1" value="2">2</label>
								<label><input type="checkbox" name="ipsec_dh_group_p1" value="4">5</label>
								<label><input type="checkbox" name="ipsec_dh_group_p1" value="8">14</label>
								<label><input type="checkbox" name="ipsec_dh_group_p1" value="16">15</label>
								<label><input type="checkbox" name="ipsec_dh_group_p1" value="32">16</label>
								<label><input type="checkbox" name="ipsec_dh_group_p1" value="64">17</label>
								<label><input type="checkbox" name="ipsec_dh_group_p1" value="128">18</label>
							</td>
						</tr>
						<tr id="tr_adv_exchange_mode">
							<th><#vpn_ipsec_Exchange_Mode#></th>
							<td>
								<input type="radio" name="ipsec_exchange" class="input" value="1" onchange="changeExchangeMode();"><#DHCPaggressive#>
								<input type="radio" name="ipsec_exchange" class="input" value="0" onchange="changeExchangeMode();" checked><#vpn_ipsec_Main_Mode#>
								<div id="exchange_mode_hint" style="color:#FC0;margin:5px 0px;"><#vpn_ipsec_Exchange_Mode_Hint#></div>
							</td>
						</tr>
						<tr id="tr_adv_keylife_time_p1">
							<th><#vpn_ipsec_IKE_Key_Lifetime#></th>
							<td>
								<input type="text" class="input_6_table" name="ipsec_keylife_p1" maxlength="6" value="86400" onKeyPress="return validator.isNumber(this,event)" autocomplete="off" autocorrect="off" autocapitalize="off">
								<span style="color:#FC0">(120~172800) <#Second#></span>
							</td>
						</tr>
						<tr id="tr_adv_ike_isakmp" style="display:none;">
							<th><#vpn_ipsec_IKE_ISAKMP_Port#></th>
							<td>
								<input type="text" class="input_6_table" name="ipsec_ike_isakmp" maxlength="3" value="500" autocomplete="off" autocorrect="off" autocapitalize="off">
							</td>
						</tr>
						<tr id="tr_adv_ike_isakmp_nat" style="display:none;"s>
							<th><#vpn_ipsec_IKE_ISAKMP_NAT_Port#></th>
							<td>
								<input type="text" class="input_6_table" name="ipsec_ike_isakmp_nat" maxlength="4" value="4500" autocomplete="off" autocorrect="off" autocapitalize="off">
							</td>
						</tr>
						<tr id="tr_adv_dead_peer_detection">
							<th><#vpn_ipsec_DPD#></th>
							<td>
								<input type="radio" name="ipsec_dead_peer_detection" id="ipsec_dead_peer_detection_dis" class="input" value="0" onchange="changeAdvDeadPeerDetection(this)">
								<label for='ipsec_dead_peer_detection_dis' id="ipsec_dead_peer_detection_dis_label"><#btn_disable#></label>
								<input type="radio" name="ipsec_dead_peer_detection" id="ipsec_dead_peer_detection_clear" class="input" value="1" onchange="changeAdvDeadPeerDetection(this)">
								<label for='ipsec_dead_peer_detection_clear' id="ipsec_dead_peer_detection_clear_label"><#CTL_clear#></label>
								<input type="radio" name="ipsec_dead_peer_detection" id="ipsec_dead_peer_detection_hold" class="input" value="2" onchange="changeAdvDeadPeerDetection(this)">
								<label for='ipsec_dead_peer_detection_hold' id="ipsec_dead_peer_detection_hold_label"><#vpn_ipsec_Suspend#></label>
								<input type="radio" name="ipsec_dead_peer_detection" id="ipsec_dead_peer_detection_restart" class="input" value="3" onchange="changeAdvDeadPeerDetection(this)">
								<label for='ipsec_dead_peer_detection_restart' id="ipsec_dead_peer_detection_restart_label"><#vpn_ipsec_Restart#></label>
							</td>
						</tr>
						<tr id="tr_adv_dpd_interval">
							<th><#vpn_ipsec_DPD_Checking_Interval#></th>
							<td>
								<input type="text" class="input_3_table" name="ipsec_dpd" maxlength="3" value="10" onKeyPress="return validator.isNumber(this,event)" autocomplete="off" autocorrect="off" autocapitalize="off">
								<span style="color:#FC0">(10~900) <#Second#></span>
							</td>
						</tr>
					</table>
					<table id="tb_adv_phase2" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:15px;">
						<thead>
						<tr>
							<td colspan="2">Advanced Settings - <#vpn_ipsec_Phase_2_Negotiations#></td>
						</tr>
						</thead>
						<tr id="tr_adv_encryption_p2">
							<th><#vpn_ipsec_Encryption#></th>
							<td>
								<!--label><input type="checkbox" name="ipsec_encryption_p2" value="1">DES</label-->
								<label><input type="checkbox" name="ipsec_encryption_p2" value="2">3DES</label>
								<label><input type="checkbox" name="ipsec_encryption_p2" value="4">AES128</label>
								<!--label><input type="checkbox" name="ipsec_encryption_p2" value="8">AES192</label>
								<label><input type="checkbox" name="ipsec_encryption_p2" value="16">AES256</label-->
							</td>
						</tr>
						<tr id="tr_adv_hash_p2">
							<th><#vpn_ipsec_Hash#></th>
							<td>
								<!--label><input type="checkbox" name="ipsec_hash_p2" value="1">MD5</label-->
								<label><input type="checkbox" name="ipsec_hash_p2" value="2">SHA1</label>
								<label><input type="checkbox" name="ipsec_hash_p2" value="4">SHA256</label>
								<!--label><input type="checkbox" name="ipsec_hash_p2" value="8">SHA384</label>
								<label><input type="checkbox" name="ipsec_hash_p2" value="16">SHA512</label-->
							</td>
						</tr>
						<tr id="tr_adv_pfs">
							<th>Perfect Forward Secrecy (PFS)</th><!-- untranslated -->
							<td>
								<label><input type="radio" name="ipsec_pfs" class="input" value="1" onchange="changePFS();"><#WLANConfig11b_WirelessCtrl_button1name#></label>
								<label><input type="radio" name="ipsec_pfs" class="input" value="0" onchange="changePFS();"><#WLANConfig11b_WirelessCtrl_buttonname#></label>
							</td>
						</tr>
						<tr id="tr_adv_pfs_group">
							<th>PFS Groups</th><!-- untranslated -->
							<td>
								<label><input type="checkbox" name="ipsec_pfs_group" value="1">1</label>
								<label><input type="checkbox" name="ipsec_pfs_group" value="2">2</label>
								<label><input type="checkbox" name="ipsec_pfs_group" value="4">5</label>
								<label><input type="checkbox" name="ipsec_pfs_group" value="8">14</label>
								<label><input type="checkbox" name="ipsec_pfs_group" value="16">15</label>
								<label><input type="checkbox" name="ipsec_pfs_group" value="32">16</label>
								<label><input type="checkbox" name="ipsec_pfs_group" value="64">17</label>
								<label><input type="checkbox" name="ipsec_pfs_group" value="128">18</label>
							</td>
						</tr>
						<tr id="tr_adv_keylife_time_p2">
							<th><#vpn_ipsec_Key_Lifetime#></th>
							<td>
								<input type="text" class="input_6_table" name="ipsec_keylife_p2" maxlength="6" value="3600" onKeyPress="return validator.isNumber(this,event)" autocomplete="off" autocorrect="off" autocapitalize="off">
								<span style="color:#FC0">(120~172800) <#Second#></span>
							</td>
						</tr>
						<tr id="tr_adv_keyingtries_p2">
							<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(33,1);"><#vpn_ipsec_Key_Retries#></a></th>
							<td>
								<input type="text" class="input_6_table" name="ipsec_keyingtries" maxlength="2" value="3" onKeyPress="return validator.isNumber(this,event)" autocomplete="off" autocorrect="off" autocapitalize="off">
							</td>
						</tr>
					</table>
					<div style="color:#FC0;margin:10px 0px;"><#vpn_ipsec_Default_DH_Hint#></div>
				</div>
				<!-- Advanced Settings table end-->

				<div style="margin-top:15px;width:100%;text-align:center;">
					<input id="cancelBtn_ipsec" class="button_gen" type="button" onclick="cancel_ipsec_profile_panel();" value="<#CTL_Cancel#>">
					<input class="button_gen" type="button" onclick="save_ipsec_profile_panel();" value="<#CTL_onlysave#>">	
				</div>
			</td>
		</tr>
	</table>
	</div>
</form>
<!---- vpnc_OpenVPN start  ---->
<form method="post" name="vpnclientForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_VPNClient_Content.asp">
<input type="hidden" name="next_page" value="Advanced_VPNClient_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="flag" value="background">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="saveNvram">
<input type="hidden" name="action_wait" value="1">
<input type="hidden" name="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="vpnc_clientlist" value='<% nvram_get("vpnc_clientlist"); %>'>
<input type="hidden" name="vpn_upload_type" value="ovpn">
<input type="hidden" name="vpn_upload_unit" value="1">
<input type="hidden" name="vpn_client_unit" value="<% nvram_get("vpn_client_unit"); %>">
<input type="hidden" name="vpnc_type" value="PPTP">
<input type="hidden" name="vpnc_pptp_options_x_list" value="<% nvram_get("vpnc_pptp_options_x_list"); %>">
<input type="hidden" name="vpnc_openvpn_unit_edit" value="1">
<input type="hidden" name="vpnc_dev_policy_list" value='<% nvram_get("vpnc_dev_policy_list"); %>'>
<input type="hidden" name="vpnc_dev_policy_list_tmp" value=''>
<input type="hidden" name="dhcp_static_x" value='<% nvram_get("dhcp_static_x"); %>'>
<input type="hidden" name="dhcp_staticlist" value="">
<div id="openvpnc_setting_openvpn" class="contentM_qis pop_div_bg" style="box-shadow: 1px 5px 10px #000;"> 
	<table class="QISform_wireless" border=0 align="center" cellpadding="5" cellspacing="0">
		<tr style="height:32px;">
			<td>		
				<div id="divTabMenu_openvpn"></div>
			</td>
		</tr>
		<tr id="tr_openvpn_limit_hint" style="display:none;">
			<td>
				<div class="vpnc_limit_hint"></div>
			</td>
		</tr>
		<tr>
			<td>
		 		<div>
		 			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
		 				<tr>
							<th><#IPConnection_autofwDesc_itemname#></th>
							<td>
								<input type="text" maxlength="64" name="vpnc_openvpn_des" value="" class="input_32_table" style="float:left;" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>  			
						<tr>
							<th><#Username#> (option)</th>
							<td>
								<input type="text" maxlength="64" name="vpnc_openvpn_username" value="" class="input_32_table" style="float:left;" autocomplete="off" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>  
						<tr>
							<th><#HSDPAConfig_Password_itemname#> (option)</th>
							<td>
								<input type="text" maxlength="64" name="vpnc_openvpn_pwd" value="" class="input_32_table" style="float:left;" autocomplete="off" autocorrect="off" autocapitalize="off"></input>
							</td>
						</tr>
						<tr>
							<th><#vpn_openvpnc_importovpn#></th>
							<td>
								<input type="file" name="file" class="input" style="color:#FC0;*color:#000;"><br>
								<input class="button_gen" onclick="ImportOvpn(document.vpnclientForm.vpnc_openvpn_unit_edit.value);" type="button" value="<#CTL_upload#>" />
								<img id="loadingicon" style="margin-left:5px;display:none;" src="/images/InternetScan.gif">
								<span id="importOvpnFile" style="display:none;"><#Main_alert_proceeding_desc3#></span>
							</td>
						</tr> 
					</table> 	
</form>	 			
			 		<br>
					<div style="color:#FC0;margin-bottom: 10px;">
						<input type="checkbox" id="cbManualImport" class="input" onclick="manualImport(this.checked)"><#vpn_openvpnc_manual#>
					</div>
					
<form method="post" name="openvpnCAForm" action="vpnupload.cgi" target="hidden_frame" enctype="multipart/form-data">
<input type="hidden" name="current_page" value="Advanced_VPNClient_Content.asp">
<input type="hidden" name="next_page" value="Advanced_VPNClient_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="flag" value="background">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="saveNvram">
<input type="hidden" name="action_wait" value="1">
<input type="hidden" name="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="vpn_upload_type" value="ca">
<input type="hidden" name="vpn_upload_unit" value="1">
					<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable" id="caFiled" style="display:none">
					<tr>
						<th><#vpn_openvpnc_importCA#></th>
						<td>								
							<input type="file" name="file" class="input" style="color:#FC0;*color:#000;"><br>
							<input id="importCA" class="button_gen" onclick="startImportCA(document.vpnclientForm.vpnc_openvpn_unit_edit.value);" type="button" value="<#CTL_upload#>" />
							<img id="loadingiconCA" style="margin-left:5px;display:none;" src="/images/InternetScan.gif">	
						</td>
					</tr> 
					</table>
</form>				
<form method="post" name="openvpnCertForm" action="vpnupload.cgi" target="hidden_frame" enctype="multipart/form-data">												
<input type="hidden" name="current_page" value="Advanced_VPNClient_Content.asp">
<input type="hidden" name="next_page" value="Advanced_VPNClient_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="flag" value="background">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="saveNvram">
<input type="hidden" name="action_wait" value="1">
<input type="hidden" name="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="vpn_upload_type" value="cert">
<input type="hidden" name="vpn_upload_unit" value="1">
					<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable" id="certFiled" style="display:none">
						<tr>
							<th>Import Cert file</th>
							<td>
								<input type="file" name="file" class="input" style="color:#FC0;*color:#000;"><br>
								<input class="button_gen" onclick="setTimeout('ovpnFileChecker();',2000);document.openvpnCertForm.submit();" type="button" value="<#CTL_upload#>" />					  		
							</td>
						</tr> 					
					</table>
</form>
<form method="post" name="openvpnKeyForm" action="vpnupload.cgi" target="hidden_frame" enctype="multipart/form-data">						
<input type="hidden" name="current_page" value="Advanced_VPNClient_Content.asp">
<input type="hidden" name="next_page" value="Advanced_VPNClient_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="flag" value="background">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="saveNvram">
<input type="hidden" name="action_wait" value="1">
<input type="hidden" name="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="vpn_upload_type" value="key">
<input type="hidden" name="vpn_upload_unit" value="1">
					<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable" style="display:none">					
						<tr>
							<th>Import Key file</th>
							<td>
								<input type="file" name="file" class="input" style="color:#FC0;*color:#000;"><br>
								<input class="button_gen" onclick="setTimeout('ovpnFileChecker();',2000);document.openvpnKeyForm.submit();" type="button" value="<#CTL_upload#>" />
							</td>
						</tr> 					
					</table>
</form>
<form method="post" name="openvpnStaticForm" action="vpnupload.cgi" target="hidden_frame" enctype="multipart/form-data">			
<input type="hidden" name="current_page" value="Advanced_VPNClient_Content.asp">
<input type="hidden" name="next_page" value="Advanced_VPNClient_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="flag" value="background">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="saveNvram">
<input type="hidden" name="action_wait" value="1">
<input type="hidden" name="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="vpn_upload_type" value="static">
<input type="hidden" name="vpn_upload_unit" value="1">
					<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable" id="staticFiled" style="display:none">	
						<tr>
							<th>Import Static file</th>
							<td>
								<input type="file" name="file" class="input" style="color:#FC0;*color:#000;"><br>
								<input class="button_gen" onclick="setTimeout('ovpnFileChecker();',2000);document.openvpnStaticForm.submit();" type="button" value="<#CTL_upload#>" />					  		
							</td>
						</tr> 
						
					</table>
</form>
					<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable" id="manualFiled" style="display:none">
						<tr>
							<th><#Manual_Setting_btn#></th>
							<td>
		  					<input class="button_gen" onclick="document.getElementById('manualFiled_panel').style.display='';" type="button" value="<#pvccfg_edit#>">
					  	</td>
						</tr> 
					</table>
<form method="post" name="openvpnManualForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_VPNClient_Content.asp">
<input type="hidden" name="next_page" value="Advanced_VPNClient_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="flag" value="background">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="saveNvram">
<input type="hidden" name="action_wait" value="1">
<input type="hidden" name="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="vpn_upload_unit" value="1">
<input type="hidden" name="vpn_crt_client1_ca" value="" disabled>
<input type="hidden" name="vpn_crt_client1_crt" value="" disabled>
<input type="hidden" name="vpn_crt_client1_key" value="" disabled>
<input type="hidden" name="vpn_crt_client1_static" value="" disabled>
<input type="hidden" name="vpn_crt_client1_crl" value="" disabled>
<input type="hidden" name="vpn_crt_client2_ca" value="" disabled>
<input type="hidden" name="vpn_crt_client2_crt" value="" disabled>
<input type="hidden" name="vpn_crt_client2_key" value="" disabled>
<input type="hidden" name="vpn_crt_client2_static" value="" disabled>
<input type="hidden" name="vpn_crt_client2_crl" value="" disabled>
<input type="hidden" name="vpn_crt_client3_ca" value="" disabled>
<input type="hidden" name="vpn_crt_client3_crt" value="" disabled>
<input type="hidden" name="vpn_crt_client3_key" value="" disabled>
<input type="hidden" name="vpn_crt_client3_static" value="" disabled>
<input type="hidden" name="vpn_crt_client3_crl" value="" disabled>
<input type="hidden" name="vpn_crt_client4_ca" value="" disabled>
<input type="hidden" name="vpn_crt_client4_crt" value="" disabled>
<input type="hidden" name="vpn_crt_client4_key" value="" disabled>
<input type="hidden" name="vpn_crt_client4_static" value="" disabled>
<input type="hidden" name="vpn_crt_client4_crl" value="" disabled>
<input type="hidden" name="vpn_crt_client5_ca" value="" disabled>
<input type="hidden" name="vpn_crt_client5_crt" value="" disabled>
<input type="hidden" name="vpn_crt_client5_key" value="" disabled>
<input type="hidden" name="vpn_crt_client5_static" value="" disabled>
<input type="hidden" name="vpn_crt_client5_crl" value="" disabled>
					<div id="manualFiled_panel" class="contentM_qis_manual" style="display:none">
						<table class="QISform_wireless" border=0 align="center" cellpadding="5" cellspacing="0">											
							<tr>
								<td valign="top">
					     		<table class="QISform_wireless" border=0 align="center" cellpadding="5" cellspacing="0">
					         		<tr>
					         			<td>
											<div class="description_down"><#vpn_openvpn_Keys_Cert#></div>
										</td>		
									</tr>
									<tr>
										<td>
											<div style="margin-left:30px; margin-top:10px;">
												<p><#vpn_openvpn_KC_Edit1#> <span style="color:#FC0;">----- BEGIN xxx ----- </span>/<span style="color:#FC0;"> ----- END xxx -----</span> <#vpn_openvpn_KC_Edit2#>
												<p><#vpn_openvpn_KC_Limit#>
											</div>													
											<div style="margin:5px;*margin-left:-5px;width: 700px;" class="splitLine"></div>
										</td>	
									</tr>
									<tr>
										<td valign="top">
											<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
												<tr>
													<th id="manualCa">Certificate Authority</th>
													<td>
														<textarea rows="8" class="textarea_ssh_table" id="edit_vpn_crt_client_ca" name="edit_vpn_crt_client_ca" cols="65" maxlength="3999"></textarea>
													</td>
												</tr>
												<tr>
													<th id="manualCert">Client Certificate</th>
													<td>
														<textarea rows="8" class="textarea_ssh_table" id="edit_vpn_crt_client_crt" name="edit_vpn_crt_client_crt" cols="65" maxlength="3999"></textarea>
													</td>
												</tr>
												<tr>
													<th id="manualKey">Client Key</th>
													<td>
														<textarea rows="8" class="textarea_ssh_table" id="edit_vpn_crt_client_key" name="edit_vpn_crt_client_key" cols="65" maxlength="3999"></textarea>
													</td>
												</tr>
												<tr>
													<th id="manualStatic">Static Key (Optional)</th>
													<td>
														<textarea rows="8" class="textarea_ssh_table" id="edit_vpn_crt_client_static" name="edit_vpn_crt_client_static" cols="65" maxlength="3999"></textarea>
													</td>
												</tr>
												<tr>
													<th id="manualCRList">Certificate Revocation List (Optional)</th>
													<td>
														<textarea rows="8" class="textarea_ssh_table" id="edit_vpn_crt_client_crl" name="edit_vpn_crt_client_crl" cols="65" maxlength="3999"></textarea>
													</td>
												</tr>
											</table>
								  		</td>
								  	</tr>				
					  			</table>
									<div style="margin-top:5px;width:100%;text-align:center;">
										<input class="button_gen" type="button" onclick="cancel_Key_panel();" value="<#CTL_Cancel#>">
										<input class="button_gen" type="button" onclick="saveManual(document.vpnclientForm.vpnc_openvpn_unit_edit.value);" value="<#CTL_onlysave#>">	
									</div>					
								</td>
					  		</tr>
						</table>	
</form>					  
					</div>
				</div>
			</td>
		</tr>
	</table>
	<div style="margin-top:5px;padding-bottom:10px;width:100%;text-align:center;">
		<input class="button_gen" type="button" onclick="cancel_add_rule();" id="cancelBtn_openvpn" value="<#CTL_Cancel#>">
		<input class="button_gen" onclick='addOpenvpnProfile();' type="button" value="<#CTL_ok#>">
	</div>	
      <!--===================================Ending of openvpn setting Content===========================================-->			
</div>
<iframe name="vpnc_default_wan_hidden_frame" id="vpnc_default_wan_hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="vpnc_default_wan_form" action="/start_apply.htm" target="vpnc_default_wan_hidden_frame">
	<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
	<input type="hidden" name="current_page" value="Advanced_VPNClient_Content.asp">
	<input type="hidden" name="next_page" value="Advanced_VPNClient_Content.asp">
	<input type="hidden" name="group_id" value="">
	<input type="hidden" name="modified" value="0">
	<input type="hidden" name="flag" value="background">
	<input type="hidden" name="action_mode" value="apply">
	<input type="hidden" name="action_script" value="restart_default_wan">
	<input type="hidden" name="action_wait" value="3">
	<input type="hidden" name="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
	<input type="hidden" name="vpnc_default_wan_tmp" value="">
</form>
<iframe name="vpnc_dev_policy_hidden_frame" id="vpnc_dev_policy_hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="vpnc_dev_policy_form" action="/start_apply.htm" target="vpnc_dev_policy_hidden_frame">
	<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
	<input type="hidden" name="current_page" value="Advanced_VPNClient_Content.asp">
	<input type="hidden" name="next_page" value="Advanced_VPNClient_Content.asp">
	<input type="hidden" name="group_id" value="">
	<input type="hidden" name="modified" value="0">
	<input type="hidden" name="flag" value="background">
	<input type="hidden" name="action_mode" value="apply">
	<input type="hidden" name="action_script" value="saveNvram">
	<input type="hidden" name="action_wait" value="3">
	<input type="hidden" name="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
	<input type="hidden" name="vpnc_dev_policy_list" value="">
	<input type="hidden" name="vpnc_dev_policy_list_tmp" value="">
	<input type="hidden" name="vpnc_policy_unit" value="">
	<input type="hidden" name="vpnc_tmp_policy" value="">
	<input type="hidden" name="dhcp_static_x" value='<% nvram_get("dhcp_static_x"); %>'>
	<input type="hidden" name="dhcp_staticlist" value="">
</form>
<div id="footer"></div>
</body>
</html>
