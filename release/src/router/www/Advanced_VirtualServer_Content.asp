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
<title><#Web_Title#> - <#menu5_3_4#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="device-map/device-map.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/client_function.js"></script>
<script language="JavaScript" type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" language="JavaScript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script type="text/javascript" src="form.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<style type="text/css">
.contentM_qis{
	position:absolute;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius: 5px;
	z-index: 200;
	display: none;
	margin-left: 35%;
	top: 290px;
	width: 600px;
	box-shadow: 1px 5px 10px #000;
	font-size: 12px;
	color: #FFFFFF;
	padding: 20px;
}
</style>
<script>
var wItem = new Array(new Array("", "", "TCP"),
											new Array("FTP", "20,21", "TCP"),
											new Array("FTP_ALG", "2021", "TCP"),
											new Array("TELNET", "23", "TCP"),
											new Array("SMTP", "25", "TCP"),
											new Array("DNS", "53", "UDP"),
											new Array("FINGER", "79", "TCP"),
											new Array("HTTP", "80", "TCP"),
											new Array("POP3", "110", "TCP"),
											new Array("SNMP", "161", "UDP"),
											new Array("SNMP TRAP", "162", "UDP"),
											new Array("GRE", "47", "OTHER"),
											new Array("IPv6 Tunnel", "41", "OTHER"),
											new Array("IPSec VPN", "500,4500", "UDP"),
											new Array("Open VPN", "1194", "UDP"),
											new Array("PPTP VPN", "1723", "TCP"),
											new Array("L2TP / IPSec VPN", "500,1701,4500", "UDP")
											);

var wItem2 = new Array(new Array("", "", "TCP"),
											 new Array("Age of Empires", "2302:2400,6073", "BOTH"),
											 new Array("BitTorrent", "6881:6889", "TCP"),
											 new Array("Counter Strike(TCP)", "27030:27039", "TCP"),
											 new Array("Counter Strike(UDP)", "27000:27015,1200", "UDP"),
											 new Array("PlayStation2", "4658,4659", "BOTH"),
											 new Array("Warcraft III", "6112:6119,4000", "BOTH"),
											 new Array("WOW", "3724", "BOTH"),
											 new Array("Xbox Live", "3074", "BOTH"));

<% login_state_hook(); %>

var vts_rulelist_array = [];

var ctf_disable = '<% nvram_get("ctf_disable"); %>';
var wans_mode ='<% nvram_get("wans_mode"); %>';
var dual_wan_lb_status = (check_dual_wan_status().status == "1" && check_dual_wan_status().mode == "lb") ? true : false;
var support_dual_wan_unit_flag = (mtwancfg_support && dual_wan_lb_status) ? true : false;
var rulelist_table = ["vts_rulelist_0"];
var rulelist_nv = ["vts_rulelist"];
if(support_dual_wan_unit_flag){
	rulelist_table.push("vts_rulelist_1");
	rulelist_nv.push("vts1_rulelist");
}
var profileMaxNum = 64;
var usb_port_conflict_faq = "https://www.asus.com/support/FAQ/1037906";
var usb_port_conflict_current = false;
var vts_enable_current = "0";
function initial(){
	show_menu();
	httpApi.faqURL("1037906", function(url){
		document.getElementById("faq").href = url;
		usb_port_conflict_faq = url;
		if($("#ftpPortConflict").find("#ftp_port_conflict_faq").length)
			$("#ftpPortConflict").find("#ftp_port_conflict_faq").attr("href", usb_port_conflict_faq);
	});
	//parse nvram to array
	var parseNvramToArray = function(oriNvram) {
		var parseArray = [];
		var oriNvramRow = decodeURIComponent(oriNvram).split('<');

		for(var i = 0; i < oriNvramRow.length; i += 1) {
			if(oriNvramRow[i] != "") {
				var oriNvramCol = oriNvramRow[i].split('>');
				var eachRuleArray = new Array();
				var serviceName = oriNvramCol[0];
				eachRuleArray["serviceName"] = serviceName;
				var sourceIP = (oriNvramCol[5] == undefined) ? "" : oriNvramCol[5];
				eachRuleArray["sourceIP"] = sourceIP;
				var externalPort = oriNvramCol[1];
				eachRuleArray["externalPort"] = externalPort;
				var internalIP = oriNvramCol[2];
				eachRuleArray["internalIP"] = internalIP;
				var internalPort = oriNvramCol[3];
				eachRuleArray["internalPort"] = internalPort;
				var protocol = oriNvramCol[4];
				eachRuleArray["protocol"] = protocol;
				parseArray.push(eachRuleArray);
			}
		}
		return parseArray;
	};
	vts_rulelist_array["vts_rulelist_0"] = parseNvramToArray('<% nvram_char_to_ascii("","vts_rulelist"); %>');
	if(support_dual_wan_unit_flag)
		vts_rulelist_array["vts_rulelist_1"] = parseNvramToArray('<% nvram_char_to_ascii("","vts1_rulelist"); %>');

	Object.keys(vts_rulelist_array).forEach(function(key) {
		gen_vts_ruleTable_Block(key);
	});
	
	loadAppOptions();
	loadGameOptions();
	setTimeout("showDropdownClientList('setClientIP', 'ip', 'all', 'ClientList_Block', 'pull_arrow', 'online');", 1000);
	Object.keys(vts_rulelist_array).forEach(function(key) {
		showvts_rulelist(vts_rulelist_array[key], key);
	});

	if(!parent.usb_support)
		document.getElementById('FTP_desc').style.display = "none";

	//if(dualWAN_support && wans_mode == "lb")
	//	document.getElementById("lb_note").style.display = "";

	if('<% get_parameter("af"); %>' == 'KnownApps' && '<% get_parameter("item"); %>' == 'ftp'){
		if(vts_rulelist_array["vts_rulelist_0"].length < profileMaxNum) {
			var KnownApps = document.form.KnownApps;
			KnownApps.options[1].selected = 1;
			change_wizard(KnownApps, 'KnownApps');
			saveProfile("new");
		}
	}

	if(based_modelid == "GT-AC5300" || based_modelid == "GT-AC9600")
		document.getElementById("VSGameList").parentNode.style.display = "none";

	vts_enable_current = httpApi.nvramGet(["vts_enable_x"]).vts_enable_x;
	usb_port_conflict_current = httpApi.ftp_port_conflict_check.conflict();
	if(vts_enable_current == "1" && usb_port_conflict_current){
		$("#ftpPortConflict").show();
		var text = httpApi.ftp_port_conflict_check.port_forwarding.hint;
		text += "<br>";
		text += "<a id='ftp_port_conflict_faq' href='" + usb_port_conflict_faq + "' target='_blank' style='text-decoration:underline;color:#FC0;'><#FAQ_Find#></a>";
		$("#ftpPortConflict").html(text);
	}
}

function isChange(){
	if((document.form.vts_enable_x[0].checked == true && '<% nvram_get("vts_enable_x"); %>' == '0') || 
				(document.form.vts_enable_x[1].checked == true && '<% nvram_get("vts_enable_x"); %>' == '1')){
		return true;
	}
	else if(document.form.vts_rulelist.value != decodeURIComponent('<% nvram_char_to_ascii("","vts_rulelist"); %>')){
		return true;
	}
	else
		return false;
}

function applyRule(_status){
	/* 2014.04 Viz: No need to reboot for ctf enable models.
	if(ctf_disable == '0' && isChange()){
		document.form.action_script.value = "reboot";
		document.form.action_wait.value = "<% get_default_reboot_time(); %>";
	}*/	
	var obj = {
		"action_mode": "apply",
		"rc_service": "restart_firewall",
	}
	obj["vts_enable_x"] = _status;
	httpApi.nvramSet(obj);

	if(_status == "1" && httpApi.ftp_port_conflict_check.usb_ftp.enabled() && httpApi.ftp_port_conflict_check.port_forwarding.use_usb_ftp_port()){
		var hint = httpApi.ftp_port_conflict_check.port_forwarding.hint;
		hint += "\n";
		hint += "<#FAQ_Find#> : ";
		hint += usb_port_conflict_faq;
		alert(hint);
	}

	showLoading();
	document.form.submit();
}

function loadAppOptions(){
	var item_name = "";
	free_options(document.form.KnownApps);
	add_option(document.form.KnownApps, "<#Select_menu_default#>", 0, 1);
	for(var i = 1; i < wItem.length; i++) {
		item_name = wItem[i][0] + " (" + wItem[i][1] + ")";
		add_option(document.form.KnownApps, item_name, i, 0);
	}
}

function loadGameOptions(){
	var item_name = "";
	free_options(document.form.KnownGames);
	add_option(document.form.KnownGames, "<#Select_menu_default#>", 0, 1);
	for(var i = 1; i < wItem2.length; i++) {
		item_name = wItem2[i][0] + " (" + wItem2[i][1] + ")";
		add_option(document.form.KnownGames, item_name, i, 0);
	}
}

function change_wizard(o, id){
	if(id == "KnownApps"){
		var set_famous_server_value = function() {
			for(var i = 0; i < wItem.length; ++i){
				if(wItem[i][0] != null && o.value == i){
					if(wItem[i][2] == "TCP")
						document.getElementById("vts_proto_x").options[0].selected = 1;
					else if(wItem[i][2] == "UDP")
						document.getElementById("vts_proto_x").options[1].selected = 1;
					else if(wItem[i][2] == "BOTH")
						document.getElementById("vts_proto_x").options[2].selected = 1;
					else if(wItem[i][2] == "OTHER")
						document.getElementById("vts_proto_x").options[3].selected = 1;
					
					document.getElementById("vts_ipaddr_x").value = login_ip_str();
					document.getElementById("vts_port_x").value = wItem[i][1];
					document.getElementById("vts_desc_x").value = wItem[i][0]+" Server";
					break;
				}
			}
		};
		set_famous_server_value();

		var set_famous_ftp_value = function() {
			if(document.form.KnownApps.options[1].selected == 1){
				if(!parent.usb_support){
					document.getElementById("vts_port_x").value = "21";
				}
				document.getElementById("vts_lport_x").value = "21";
			}
			else {
				document.getElementById("vts_lport_x").value = "";
			}	
		};

		set_famous_ftp_value();
		document.getElementById("KnownApps").value = 0;
	}
	else if(id == "KnownGames"){
		var set_famous_game_value = function() {
			document.getElementById("vts_lport_x").value = "";
			for(var i = 0; i < wItem2.length; ++i){
				if(wItem2[i][0] != null && o.value == i){
					if(wItem2[i][2] == "TCP")
						document.getElementById("vts_proto_x").options[0].selected = 1;
					else if(wItem2[i][2] == "UDP")
						document.getElementById("vts_proto_x").options[1].selected = 1;
					else if(wItem2[i][2] == "BOTH")
						document.getElementById("vts_proto_x").options[2].selected = 1;
					else if(wItem2[i][2] == "OTHER")
						document.getElementById("vts_proto_x").options[3].selected = 1;
					
					document.getElementById("vts_ipaddr_x").value = login_ip_str();
					document.getElementById("vts_port_x").value = wItem2[i][1];
					document.getElementById("vts_desc_x").value = wItem2[i][0];

					break;
				}
			}
		};
		set_famous_game_value();
		document.getElementById("KnownGames").value = 0;
	}
}

/*------------ Mouse event of fake LAN IP select menu {-----------------*/
function setClientIP(num){
	document.getElementById('vts_ipaddr_x').value = num;
	hideClients_Block();
}

function pullLANIPList(obj){
	var element = document.getElementById('ClientList_Block');
	var isMenuopen = element.offsetWidth > 0 || element.offsetHeight > 0;
	if(isMenuopen == 0) {
		obj.src = "/images/arrow-top.gif"
		element.style.display = 'block';
		document.getElementById("vts_ipaddr_x").focus();
	}
	else
		hideClients_Block();
}

function hideClients_Block(){
	document.getElementById("pull_arrow").src = "/images/arrow-down.gif";
	document.getElementById('ClientList_Block').style.display = 'none';
	validator.validIPForm(document.getElementById("vts_ipaddr_x"), 0);
}
/*----------} Mouse event of fake LAN IP select menu-----------------*/

function validate_multi_range(val, mini, maxi, obj){

	var rangere=new RegExp("^([0-9]{1,5})\:([0-9]{1,5})$", "gi");
	if(rangere.test(val)){
		if(!validator.eachPort(obj, RegExp.$1, mini, maxi) || !validator.eachPort(obj, RegExp.$2, mini, maxi)){
				return false;								
		}else if(parseInt(RegExp.$1) >= parseInt(RegExp.$2)){
				alert("<#JS_validport#>");	
				return false;												
		}else				
			return true;	
	}else{
		if(!validate_single_range(val, mini, maxi)){	
					return false;											
				}
				return true;								
			}	
}
function validate_single_range(val, min, max) {
	for(j=0; j<val.length; j++){		//is_number
		if (val.charAt(j)<'0' || val.charAt(j)>'9'){			
			alert('<#JS_validrange#> ' + min + ' <#JS_validrange_to#> ' + max);
			return false;
		}
	}
	
	if(val < min || val > max) {		//is_in_range		
		alert('<#JS_validrange#> ' + min + ' <#JS_validrange_to#> ' + max);
		return false;
	}else	
		return true;
}	
var parse_port="";
function check_multi_range(obj, mini, maxi, allow_range){
	_objValue = obj.value.replace(/[-~]/gi,":");	// "~-" to ":"
	var PortSplit = _objValue.split(",");
	for(i=0;i<PortSplit.length;i++){
		PortSplit[i] = PortSplit[i].replace(/(^\s*)|(\s*$)/g, ""); 		// "\space" to ""
		PortSplit[i] = PortSplit[i].replace(/(^0*)/g, ""); 		// "^0" to ""	
		
		if(PortSplit[i] == "" ||PortSplit[i] == 0){
			alert("<#JS_fieldblank#>");
			obj.focus();
			obj.select();			
			return false;
		}
		if(allow_range)
			res = validate_multi_range(PortSplit[i], mini, maxi, obj);
		else	res = validate_single_range(PortSplit[i], mini, maxi, obj);
		if(!res){
			obj.focus();
			obj.select();
			return false;
		}						
		
		if(i ==PortSplit.length -1)
			parse_port = parse_port + PortSplit[i];
		else
			parse_port = parse_port + PortSplit[i] + ",";
			
	}
	obj.value = parse_port;
	parse_port ="";
	return true;	
}

function del_Row(_this){
	if(!confirm("<#VPN_Fusion_Delete_Alert#>"))
		return false;

	var row_idx = $(_this).closest("*[row_tr_idx]").attr( "row_tr_idx" );
	var wan_idx = $(_this).closest("*[wanUnitID]").attr( "wanUnitID" );
	
	vts_rulelist_array["vts_rulelist_" + wan_idx + ""].splice(row_idx, 1);
	showvts_rulelist(vts_rulelist_array["vts_rulelist_" + wan_idx + ""], "vts_rulelist_" + wan_idx + "");

	var obj = {
		"action_mode": "apply",
		"rc_service": "restart_firewall",
	}
	obj["" +  rulelist_nv[wan_idx] + ""] = parseArrayToNvram(vts_rulelist_array["vts_rulelist_" + wan_idx + ""]);
	httpApi.nvramSet(obj);
	if(vts_enable_current == "1"){
		var usb_port_conflict_mod = httpApi.ftp_port_conflict_check.conflict();
		if(usb_port_conflict_current && !usb_port_conflict_mod){//true to false, not conflict
			$("#ftpPortConflict").hide();
			$("#ftpPortConflict").empty();
		}
		usb_port_conflict_current = usb_port_conflict_mod;
	}
}

function showvts_rulelist(_arrayData, _tableID) {
	var wan_idx = _tableID.split("_")[2];
	var code = "";
	code += '<table width="100%" cellspacing="0" cellpadding="4" align="center" class="list_table" style="word-break:break-word;">';
	if(_arrayData.length == 0)
		code += '<tr><td style="color:#FFCC00;" colspan="8"><#IPConnection_VSList_Norule#></td></tr>';
	else {
		for(var i = 0; i < _arrayData.length; i += 1) {
			var eachValue = _arrayData[i];
			code += '<tr row_tr_idx="' + i + '">';
			code += '<td width="16%">' + eachValue.serviceName + '</td>';
			code += '<td width="15%">' + eachValue.externalPort + '</td>';
			code += '<td width="15%">' + eachValue.internalPort + '</td>';
			code += '<td width="17%">' + eachValue.internalIP + '</td>';
			code += '<td width="8%">' + eachValue.protocol + '</td>';
			code += '<td width="17%">' + eachValue.sourceIP + '</td>';
			code += '<td width="6%"><input class="edit_btn" onclick="editProfile(\'edit\', this);" value=""/></td>';
			code += '<td width="6%"><input class="remove_btn" onclick="del_Row(this);" value=""/></td>';
			code += '</tr>';
		}
	}
	code += '</table>';
	document.getElementById("vts_rulelist_Block_" + wan_idx + "").innerHTML = code;	     
}

function changeBgColor(obj, num){
	if(obj.checked)
 		document.getElementById("row" + num).style.background='#FF9';
	else
 		document.getElementById("row" + num).style.background='#FFF';
}
function gen_vts_ruleTable_Block(_tableID) {
	var html = "";
	html += '<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" style="word-break:break-word;">';

	html += '<thead>';
	html += '<tr>';
	var wan_title = "";
	var wan_idx = _tableID.split("_")[2];
	if(support_dual_wan_unit_flag) {
		switch(wan_idx) {
			case "0" :
				wan_title = "<#dualwan_primary#>&nbsp;";
				break;
			case "1" :
				wan_title = "<#dualwan_secondary#>&nbsp;";
				break;
		}
	}
	html += '<td colspan="8">' + wan_title + '<#IPConnection_VSList_title#>&nbsp;(<#List_limit#>&nbsp;' + profileMaxNum + ')</td>';
	html += '</tr>';
	html += '</thead>';

	html += '<tr>';
	html += '<th width="16%"><#BM_UserList1#></th>';
	html += '<th width="15%"><#IPConnection_VSList_External_Port#></th>';
	html += '<th width="15%"><#IPConnection_VSList_Internal_Port#></th>';
	html += '<th width="17%"><#IPConnection_VSList_Internal_IP#></th>';
	html += '<th width="8%"><#IPConnection_VServerProto_itemname#></th>';
	html += '<th width="17%"><#IPConnection_VSList_SourceTarget#></th>';
	html += '<th width="6%"><#pvccfg_edit#></th>';
	html += '<th width="6%"><#CTL_del#></th>';
	html += '</tr>';

	html += '</table>';
	document.getElementById("vts_rulelist_Table_" + wan_idx + "").innerHTML = html;
}
function parseArrayToNvram(_dataArray) {
	var tmp_value = "";
	for(var i = 0; i < _dataArray.length; i += 1) {
		tmp_value += "<";
		var serviceName = ((_dataArray[i]["serviceName"] != undefined) ? _dataArray[i]["serviceName"] : "");
		tmp_value += serviceName + ">";
		var externalPort = ((_dataArray[i]["externalPort"] != undefined) ? _dataArray[i]["externalPort"] : "");
		tmp_value += externalPort + ">";
		var internalIP = ((_dataArray[i]["internalIP"] != undefined) ? _dataArray[i]["internalIP"] : "");
		tmp_value += internalIP + ">";
		var internalPort = ((_dataArray[i]["internalPort"] != undefined) ? _dataArray[i]["internalPort"] : "");
		tmp_value += internalPort + ">";
		var protocol = ((_dataArray[i]["protocol"] != undefined) ? _dataArray[i]["protocol"] : "");
		tmp_value += protocol + ">";
		var sourceIP = ((_dataArray[i]["sourceIP"] != undefined) ? _dataArray[i]["sourceIP"] : "");
		tmp_value += sourceIP;
	}
	return tmp_value;
}
function editProfile(_mode, _this) {
	if(_mode == "new") {
		$('#wans_unit').find('option').remove().end()
			.append("<option value='0'><#dualwan_primary#></option>")
			.append("<option value='1'><#dualwan_secondary#></option>")
			.append("<option value='2'>BOTH</option>")
			.val('0');

		var upper = profileMaxNum;
		var have_upper_num = 0;
		var rulelist_length = rulelist_table.length;
		for(var i = 0; i <rulelist_length; i += 1) {
			var rule_num = vts_rulelist_array[rulelist_table[i]].length;
			if(rule_num >= upper){
				have_upper_num++;
				$("#wans_unit option[value='2']").remove();//for both
				$("#wans_unit option[value='" + i + "']").remove();
			}
		}

		if(rulelist_length == have_upper_num){
			alert("<#JS_itemlimit1#> " + upper + " <#JS_itemlimit2#>");
			return false;
		}
	}

	$("#tr_wan_unit").show();
	$("#wans_unit").prop("selectedIndex", 0);
	$("#vts_desc_x").val("");
	$("#vts_proto_x").prop("selectedIndex", 0);
	$("#vts_port_x").val("");
	$("#vts_lport_x").val("");
	$("#vts_ipaddr_x").val("");
	$("#vts_target_x").val("");
	$("#saveProfile").unbind("click");
	$("#profile_setting").fadeIn(300);
	adjust_panel_block_top("profile_setting", 100);
	cal_panel_block("profile_setting", 0.25);

	if(_mode == "edit") {
		$("#tr_wan_unit").hide();

		var row_idx = $(_this).closest("*[row_tr_idx]").attr( "row_tr_idx" );
		var wan_idx = $(_this).closest("*[wanUnitID]").attr( "wanUnitID" );
		$("#vts_desc_x").val(vts_rulelist_array["vts_rulelist_" + wan_idx + ""][row_idx].serviceName);
		$('#vts_proto_x option[value="' + vts_rulelist_array["vts_rulelist_" + wan_idx + ""][row_idx].protocol +'"]').prop("selected", true);
		$("#vts_port_x").val(vts_rulelist_array["vts_rulelist_" + wan_idx + ""][row_idx].externalPort);
		$("#vts_lport_x").val(vts_rulelist_array["vts_rulelist_" + wan_idx + ""][row_idx].internalPort);
		$("#vts_ipaddr_x").val(vts_rulelist_array["vts_rulelist_" + wan_idx + ""][row_idx].internalIP);
		$("#vts_target_x").val(vts_rulelist_array["vts_rulelist_" + wan_idx + ""][row_idx].sourceIP);
		$("#saveProfile").click(
			function() {
				saveProfile("edit", wan_idx, row_idx);
			}
		);
	}
	else if(_mode == "new") {
		if(!support_dual_wan_unit_flag)
			$("#tr_wan_unit").hide();
		$("#saveProfile").click(
			function() {
				saveProfile("new");
			}
		);
	}
}
function saveProfile(_mode, _wanIdx, _rowIdx) {
	var checkCurrectData = function(_currectData, _addData) {
		//match(Source Target + Port Range + Protocol) is not accepted
		var vts_rulelist_array_temp = $.extend(true, [], _currectData);
		var add_ruleList_array_temp = $.extend(true, [], _addData);
		if(vts_rulelist_array_temp.length > 0) {
			//filter Service Name Local IP and Local Port
			delete add_ruleList_array_temp.serviceName;
			delete add_ruleList_array_temp.internalIP;
			delete add_ruleList_array_temp.internalPort;
			if(add_ruleList_array_temp.protocol == "BOTH") { // BOTH is TCP and UDP
				for(var i = 0; i < vts_rulelist_array_temp.length; i += 1) {
					var currentRuleArrayTemp = $.extend(true, [], vts_rulelist_array_temp[i]);
					//filter Service Name Local IP and Local Port
					delete currentRuleArrayTemp.serviceName;
					delete currentRuleArrayTemp.internalIP;
					delete currentRuleArrayTemp.internalPort;
					if(add_ruleList_array_temp.sourceIP == currentRuleArrayTemp.sourceIP && 
						add_ruleList_array_temp.externalPort == currentRuleArrayTemp.externalPort && 
						currentRuleArrayTemp.protocol != "OTHER")
					{
						alert("<#JS_duplicate#>");
						document.getElementById("vts_port_x").focus();
						document.getElementById("vts_port_x").select();
						return false;
					}
				}
			}
			else {
				for(var i = 0; i < vts_rulelist_array_temp.length; i += 1) {
					var currentRuleArrayTemp = $.extend(true, [], vts_rulelist_array_temp[i]);
					delete currentRuleArrayTemp.serviceName;
					delete currentRuleArrayTemp.internalIP;
					delete currentRuleArrayTemp.internalPort;
					if(add_ruleList_array_temp.sourceIP == currentRuleArrayTemp.sourceIP && 
						add_ruleList_array_temp.externalPort == currentRuleArrayTemp.externalPort && 
						add_ruleList_array_temp.protocol == currentRuleArrayTemp.protocol)
					{
						alert("<#JS_duplicate#>");
						document.getElementById("vts_port_x").focus();
						document.getElementById("vts_port_x").select();
						return false;
					}
					if(currentRuleArrayTemp.protocol == "BOTH" && add_ruleList_array_temp.protocol != "OTHER") {
						if(add_ruleList_array_temp.sourceIP == currentRuleArrayTemp.sourceIP && add_ruleList_array_temp.externalPort == currentRuleArrayTemp.externalPort) {
							alert("<#JS_duplicate#>");
							document.getElementById("vts_port_x").focus();
							document.getElementById("vts_port_x").select();
							return false;
						}
					}
				}
			}
		}

		return true;
	};

	if(!Block_chars(document.getElementById("vts_desc_x"), ["<" ,">" ,"'" ,"%"]))
		return false;
	if(!Block_chars(document.getElementById("vts_port_x"), ["<" ,">"]))
		return false;
	if(document.getElementById("vts_proto_x").value == "OTHER") {
		document.getElementById("vts_lport_x").value = "";
		if (!check_multi_range(document.getElementById("vts_port_x"), 1, 255, false))
			return false;
	}
	if(!check_multi_range(document.getElementById("vts_port_x"), 1, 65535, true))
		return false;
	if(document.getElementById("vts_lport_x").value.length > 0
			&& !validator.numberRange(document.getElementById("vts_lport_x"), 1, 65535)) {
		return false;
	}
	if(document.getElementById("vts_ipaddr_x").value == "") {
		alert("<#JS_fieldblank#>");
		document.getElementById("vts_ipaddr_x").focus();
		document.getElementById("vts_ipaddr_x").select();
		return false;
	}
	if(!validator.validIPForm(document.getElementById("vts_ipaddr_x"), 0))
		return false;
	if(!validator.ipv4cidr(document.getElementById("vts_target_x"), 0))
		return false;

	var profileArray = new Array();
	profileArray["serviceName"] = $("#vts_desc_x").val();
	profileArray["protocol"] = $("#vts_proto_x").val();
	profileArray["externalPort"] = $("#vts_port_x").val();
	profileArray["internalPort"] = $("#vts_lport_x").val();
	profileArray["internalIP"] = $("#vts_ipaddr_x").val();
	profileArray["sourceIP"] = $("#vts_target_x").val();

	if(_mode == "new") {
		if(support_dual_wan_unit_flag) {
			if($("#wans_unit").val() == "2") {
				for(var i = 0; i < rulelist_table.length; i += 1) {
					 if(!(checkCurrectData(vts_rulelist_array[rulelist_table[i]], profileArray)))
						return false;
				}
				rulelist_table.forEach(function(element) {
					vts_rulelist_array[element].push(profileArray);
				});
			}
			else {
				 if(!(checkCurrectData(vts_rulelist_array["vts_rulelist_" + $("#wans_unit").val() + ""], profileArray)))
					return false;
					vts_rulelist_array["vts_rulelist_" + $("#wans_unit").val() + ""].push(profileArray);
			}
		}
		else {
			if(!(checkCurrectData(vts_rulelist_array["vts_rulelist_0"], profileArray)))
				return false;
			vts_rulelist_array["vts_rulelist_0"].push(profileArray);
		}
	}
	else if(_mode == "edit") {
		vts_rulelist_array["vts_rulelist_" + _wanIdx + ""][_rowIdx] = profileArray;
	}

	var obj = {
		"action_mode": "apply",
		"rc_service": "restart_firewall",
	}

	rulelist_table.forEach(function(element, idx) {
		showvts_rulelist(vts_rulelist_array[element], element);
		obj["" +  rulelist_nv[idx] + ""] = parseArrayToNvram(vts_rulelist_array[element]);
	});
	httpApi.nvramSet(obj);
	$("#profile_setting").fadeOut(300,function(){
		if(vts_enable_current == "1"){
			var usb_port_conflict_mod = httpApi.ftp_port_conflict_check.conflict();
			if(!usb_port_conflict_current && usb_port_conflict_mod){//false to true, first time conflict
				$("#ftpPortConflict").show();
				var text = httpApi.ftp_port_conflict_check.port_forwarding.hint;
				text += "<br>";
				text += "<a id='ftp_port_conflict_faq' href='" + usb_port_conflict_faq + "' target='_blank' style='text-decoration:underline;color:#FC0;'><#FAQ_Find#></a>";
				$("#ftpPortConflict").html(text);

				var hint = httpApi.ftp_port_conflict_check.port_forwarding.hint;
				hint += "\n";
				hint += "<#FAQ_Find#> : ";
				hint += usb_port_conflict_faq;
				alert(hint);
			}
			else if(usb_port_conflict_current && !usb_port_conflict_mod){//true to false, not conflict
				$("#ftpPortConflict").hide();
				$("#ftpPortConflict").empty();
			}
			usb_port_conflict_current = usb_port_conflict_mod;
		}
	});
}
function cancelProfile() {
	$("#profile_setting").fadeOut(300);
}
</script>
</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame" >
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_VirtualServer_Content.asp">
<input type="hidden" name="next_page" value="Advanced_VirtualServer_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_firewall">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<table class="content" align="center" cellpadding="0" cellspacing="0" >
	<tr>
		<td width="17">&nbsp;</td>
		<td valign="top" width="202">
			<div  id="mainMenu"></div>
			<div  id="subMenu"></div>
		</td>
		<td valign="top">
			<div id="tabMenu" class="submenuBlock"></div>
			<!--===================================Beginning of Main Content===========================================-->
			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
				<tr>
					<td valign="top" >
						<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
							<tbody>
							<tr>
								<td bgcolor="#4D595D" valign="top"  >
								<div>&nbsp;</div>
								<div class="formfonttitle"><#menu5_3#> - <#menu5_3_4#></div>
								<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
								<div>
									<div id="ftpPortConflict" class="formfontdesc" style="display:none;color:#FFCC00;"></div>
									<div class="formfontdesc"><#IPConnection_VServerEnable_sectiondesc#></div>
									<ul style="margin-left:-25px; *margin-left:10px;">
										<div class="formfontdesc"><li><#FirewallConfig_Port80_itemdesc#></div>
										<div class="formfontdesc" id="FTP_desc"><li><#FirewallConfig_FTPPrompt_itemdesc#></div>
									</ul>
								</div>

								<div class="formfontdesc" style="margin-top:-10px;">
									<a id="faq" href="" target="_blank" style="font-family:Lucida Console;text-decoration:underline;"><#menu5_3_4#>&nbspFAQ</a>
								</div>
								<div class="formfontdesc" id="lb_note" style="color:#FFCC00; display:none;"><#lb_note_portForwarding#></div>

								<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
									<thead>
										<tr>
											<td colspan="4"><#t2BC#></td>
										</tr>
									</thead>
									<tr>
										<th><#IPConnection_VServerEnable_itemname#><input type="hidden" name="vts_num_x_0" value="<% nvram_get("vts_num_x"); %>" readonly="1" /></th>
										<td>
											<div align="center" class="left" style="width:94px; float:left; cursor:pointer;" id="radio_VTS_enable"></div>
											<script type="text/javascript">
												$('#radio_VTS_enable').iphoneSwitch('<% nvram_get("vts_enable_x"); %>',
													function(){
														applyRule(1);
													},
													function(){
														applyRule(0);
													}
												);
											</script>
										</td>
									</tr>
								</table>
								<div id="vts_rulelist_Table_0" wanUnitID="0"></div>
								<div id="vts_rulelist_Block_0" wanUnitID="0"></div>

								<div id="vts_rulelist_Table_1" wanUnitID="1"></div>
								<div id="vts_rulelist_Block_1" wanUnitID="1"></div>
								<div class="apply_gen">
									<input class="button_gen" onclick="editProfile('new')" type="button" value="<#vpnc_step1#>">
								</div>
								</td>
							</tr>
							</tbody>
						</table>
					</td>
				</tr>
			</table>
			<!--===================================Ending of Main Content===========================================-->		
		</td>
		<td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
<div id="profile_setting"  class="contentM_qis pop_div_bg">
	<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
		<thead>
			<tr>
				<td colspan="2"><#vpn_ipsec_Quick_Select#></td>
			</tr>
		</thead>
		<tr>
			<th><#IPConnection_VSList_groupitemdesc#></th>
			<td id="vts_rulelist">
				<select name="KnownApps" id="KnownApps" class="input_option" onchange="change_wizard(this, 'KnownApps');"></select>
			</td>
		</tr>
		<tr>
			<th><#IPConnection_VSList_gameitemdesc#></th>
			<td id="VSGameList">
				<select name="KnownGames" id="KnownGames" class="input_option" onchange="change_wizard(this, 'KnownGames');"></select>
			</td>
		</tr>
	</table>
	<br>
	<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
		<thead>
			<tr>
				<td colspan="2"><#vpn_openvpn_CustomConf#></td>
			</tr>
		</thead>
		<tr id="tr_wan_unit" style="display:none;">
			<th><#dualwan_unit#></th>
			<td>
				<select id="wans_unit" class="input_option">
					<option value="0"><#dualwan_primary#></option>
					<option value="1"><#dualwan_secondary#></option>
					<option value="2">BOTH</option>
				</select>
			</td>
		</tr>
		<tr>
			<th><#BM_UserList1#></th>
			<td>
				<input type="text" maxlength="30" class="input_25_table" id="vts_desc_x" onKeyPress="return validator.isString(this, event);" autocomplete="off" autocorrect="off" autocapitalize="off"/>
				<span><#feedback_optional#></span>
			</td>
		</tr>
		<tr>
			<th><#IPConnection_VServerProto_itemname#></th>
			<td>
				<select id="vts_proto_x" class="input_option">
					<option value="TCP">TCP</option>
					<option value="UDP">UDP</option>
					<option value="BOTH">BOTH</option>
					<option value="OTHER">OTHER</option>
				</select>
			</td>
		</tr>
		<tr>
			<th><#IPConnection_VSList_External_Port#></th>
			<td>
				<input type="text" maxlength="60" class="input_25_table" id="vts_port_x" onKeyPress="return validator.isPortRange(this, event);" autocomplete="off" autocorrect="off" autocapitalize="off"/>
			</td>
		</tr>
		<tr>
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,26);"><#IPConnection_VSList_Internal_Port#></a></th>
			<td>
				<input type="text" maxlength="5" class="input_25_table" id="vts_lport_x" onKeyPress="return validator.isNumber(this,event);" autocomplete="off" autocorrect="off" autocapitalize="off"/>
				<span><#feedback_optional#></span>
			</td>
		</tr>
		<tr>
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,25);"><#IPConnection_VSList_Internal_IP#></a></th>
			<td>
				<input type="text" maxlength="15" class="input_25_table" id="vts_ipaddr_x" align="left" onkeypress="return validator.isIPAddr(this, event);" style="float:left;" onClick="hideClients_Block();" autocomplete="off" autocorrect="off" autocapitalize="off">
				<img id="pull_arrow" class="pull_arrow" height="16px;" src="images/arrow-down.gif" align="right" onclick="pullLANIPList(this);" title="<#select_IP#>">
				<div id="ClientList_Block" class="clientlist_dropdown" style="margin-left:2px;margin-top:27px;width:238px;"></div>
			</td>
		</tr>
		<tr>
			<th><#IPConnection_VSList_SourceTarget#></th>
			<td>
				<input type="text" maxlength="18" class="input_25_table" id="vts_target_x" onKeyPress="return validator.isIPAddrPlusNetmask(this, event)" autocomplete="off" autocorrect="off" autocapitalize="off"/>
				<span><#feedback_optional#></span>
			</td>
		</tr>
	</table>
	<div style="color:#FC0;margin:10px 0px;">
		* <#IPConnection_VSList_External_Port#>
		<br>
		<#IPConnection_VSList_External_Port_desc#>
		<br>
		<#IPConnection_VSList_External_Port_desc1#>
		<br>
		<#IPConnection_VSList_External_Port_desc2#>
		<br>
		<#IPConnection_VSList_External_Port_desc3#>
		<br><br>
		* <#IPConnection_VSList_SourceTarget#>
		<br>
		<#IPConnection_VSList_SourceTarget_desc#>
	</div>
	<div style="margin-top:15px;text-align:center;">
		<input class="button_gen" type="button" onclick="cancelProfile();" value="<#CTL_Cancel#>">
		<input id="saveProfile" class="button_gen" type="button" value="<#CTL_ok#>">
	</div>
</div>
<div id="footer"></div>
</form>
</body>
</html>
