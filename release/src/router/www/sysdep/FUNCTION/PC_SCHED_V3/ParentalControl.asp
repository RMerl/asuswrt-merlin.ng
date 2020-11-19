<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<title id="web_title"><#Web_Title#> - <#Parental_Control#></title>
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="ParentalControl.css">
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="usp_style.css">
<link rel="stylesheet" type="text/css" href="/calendar/fullcalendar.css">
<link rel="stylesheet" type="text/css" href="/device-map/device-map.css">
<link rel="stylesheet" type="text/css" href="/js/weekSchedule/weekSchedule.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/client_function.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/calendar/jquery-ui.js"></script> 
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" src="/js/weekSchedule/weekSchedule.js"></script>
<script type="text/javascript" src="/form.js"></script>
<style>
  #selectable .ui-selecting { background: #FECA40; }
  #selectable .ui-selected { background: #F39814; color: white; }
  #selectable .ui-unselected { background: gray; color: green; }
  #selectable .ui-unselecting { background: green; color: black; }
  #selectable { border-spacing:0px; margin-left:0px;margin-top:0px; padding: 0px; width:100%;}
  #selectable td { height: 22px; }
  
.parental_th{
	color:white;
	background:#2F3A3E;
	cursor: pointer;
	width:160px;
	height:22px;
	border-bottom:solid 1px black;
	border-right:solid 1px black;
} 
.parental_th:hover{
	background:rgb(94, 116, 124);
	cursor: pointer;
}

.checked{
	background-color:#9CB2BA;
	width:82px;
	border-bottom:solid 1px black;
	border-right:solid 1px black;
}

.disabled{
	width:82px;
	border-bottom:solid 1px black;
	border-right:solid 1px black;
}

#switch_menu{
	text-align:right
}
#switch_menu span{
	/*border:1px solid #222;*/
	
	border-radius:4px;
	font-size:16px;
	padding:3px;
}
/*#switch_menu span:hover{
	box-shadow:0px 0px 5px 3px white;
	background-color:#97CBFF;
}*/
.click:hover{
	box-shadow:0px 0px 5px 3px white;
	background-color:#97CBFF;
}
.clicked{
	background-color:#2894FF;
	box-shadow:0px 0px 5px 3px white;

}
.click{
	background:#8E8E8E;
}
</style>
<script>

var client_time_sche_json = [];
var client_time_sche_attr = function(){
	this.enable = 0;
	this.mac = "";
	this.devicename = "";
	this.offtime = "";
};

function initial(){
	var MULTIFILTER_ENABLE = '<% nvram_get("MULTIFILTER_ENABLE"); %>'.replace(/&#62/g, ">");
	var MULTIFILTER_MAC = '<% nvram_get("MULTIFILTER_MAC"); %>'.replace(/&#62/g, ">").toUpperCase();
	var MULTIFILTER_DEVICENAME = decodeURIComponent('<% nvram_char_to_ascii("","MULTIFILTER_DEVICENAME"); %>').replace(/&#62/g, ">");
	var MULTIFILTER_MACFILTER_DAYTIME_V2 = '<% nvram_get("MULTIFILTER_MACFILTER_DAYTIME_V2"); %>'.replace(/&#62/g, ">").replace(/&#60/g, "<");
	var MULTIFILTER_ENABLE_row = MULTIFILTER_ENABLE.split('>');
	var MULTIFILTER_DEVICENAME_row = MULTIFILTER_DEVICENAME.split('>');
	var MULTIFILTER_MAC_row = MULTIFILTER_MAC.split('>');
	var MULTIFILTER_MACFILTER_DAYTIME_V2_row = MULTIFILTER_MACFILTER_DAYTIME_V2.split('>');
	$.each(MULTIFILTER_ENABLE_row, function( index, value ) {
		if(value == "")
			return true;
		if(MULTIFILTER_MAC_row[index] == undefined || MULTIFILTER_DEVICENAME_row[index] == undefined)
			return true;
		var enable = value;
		var mac = MULTIFILTER_MAC_row[index];
		var devicename = "";
		if(clientList[mac])
			devicename = (clientList[mac].nickName == "") ? clientList[mac].name : clientList[mac].nickName;
		else if(MULTIFILTER_DEVICENAME[index] != undefined || MULTIFILTER_DEVICENAME[index] != "")
			devicename = MULTIFILTER_DEVICENAME_row[index];
			
		var client_time_obj = new client_time_sche_attr();
		client_time_obj.enable = enable;
		client_time_obj.mac = mac;
		client_time_obj.devicename = devicename;
		client_time_obj.offtime = MULTIFILTER_MACFILTER_DAYTIME_V2_row[index];
		client_time_sche_json.push(JSON.parse(JSON.stringify(client_time_obj)));
	});
	weekScheduleApi.data_max = (isSupport("MaxRule_PC_DAYTIME") == 0 ? 128 : isSupport("MaxRule_PC_DAYTIME"));
	show_menu();
	if(hnd_support || based_modelid == "RT-AC1200" || based_modelid == "RT-AC1200_V2" || based_modelid == "RT-AC1200GU" || based_modelid == "RT-N19"){
		$("#nat_desc").hide();
	}

	if(bwdpi_support){
		document.getElementById('guest_image').style.background = "url(images/New_ui/TimeLimits.png)";
		document.getElementById('content_title').innerHTML = "<#AiProtection_title#> - <#Time_Scheduling#>";
		document.getElementById('desc_title').innerHTML = "<#ParentalCtrl_Desc_TS#>";
		document.getElementById('web_title').innerHTML = "<#Web_Title#> - <#Time_Scheduling#>";
		document.getElementById('PC_enable').innerHTML = "<#ParentalCtrl_Enable_TS#>";
		if(isSupport("webs_filter") && isSupport("apps_filter"))
			document.getElementById('switch_menu').style.display = "";
	}
	document.getElementById('disable_NAT').href = "Advanced_SwitchCtrl_Content.asp?af=ctf_disable_force";	//this id is include in string : #ParentalCtrl_disable_NAT#

	show_footer();
	if(downsize_4m_support || downsize_8m_support){
			document.getElementById("guest_image").parentNode.style.display = "none";
	}

	if(!yadns_support && !bwdpi_support){
		document.getElementById('FormTitle').style.webkitBorderRadius = "3px";
		document.getElementById('FormTitle').style.MozBorderRadius = "3px";
		document.getElementById('FormTitle').style.BorderRadius = "3px";	
	}

	gen_mainTable();
	showDropdownClientList('setClientIP', 'mac', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');
	if(<% nvram_get("MULTIFILTER_ALL"); %>)
		$(".switch_on_content").show();
	else
		$(".switch_on_content").hide();
		
	count_time();

	//When redirect page from index.asp, auto display edit time scheduling
	var mac = cookie.get("time_scheduling_mac");
	if(mac != "" && mac != null) {
		var specific_data = client_time_sche_json.filter(function(item, index, array){
			return (item.mac == mac);
		});
		if(specific_data != undefined){
			var eventObj = [];
			eventObj["data"] = specific_data[0];
			gen_lantowanTable(eventObj);
			window.location.hash = "edit_time_anchor";
		}
		cookie.unset("time_scheduling_mac");
	}
}

/*------------ Mouse event of fake LAN IP select menu {-----------------*/
function setClientIP(macaddr){
	document.form.PC_mac.value = macaddr;
	hideClients_Block();
}

function pullLANIPList(obj){	
	var element = document.getElementById('ClientList_Block_PC');
	var isMenuopen = element.offsetWidth > 0 || element.offsetHeight > 0;
	if(isMenuopen == 0){		
		obj.src = "/images/arrow-top.gif"
		element.style.display = 'block';		
		document.form.PC_mac.focus();		
	}
	else
		hideClients_Block();
}

function hideClients_Block(){
	document.getElementById("pull_arrow").src = "/images/arrow-down.gif";
	document.getElementById('ClientList_Block_PC').style.display='none';
}
/*----------} Mouse event of fake LAN IP select menu-----------------*/

function gen_mainTable(){
	var code = "";
	var clientListEventData = [];
	code +='<table width="100%" border="1" cellspacing="0" cellpadding="4" align="center" class="FormTable_table" id="mainTable_table">';
	code +='<thead><tr><td colspan="4"><#ConnectedClient#>&nbsp;(<#List_limit#>&nbsp;'+MaxRule_parentctrl+')</td></tr></thead>';
	code += '<tr><th width="15%" height="30px" title="<#select_all#>">';
	code += '<select id="selAll" class="input_option" onchange="selectAll();">';
	code += '<option value=""><#select_all#></option>';
	code += '<option value="0"><#btn_disable#></option>';
	code += '<option value="1"><#diskUtility_time#></option>';
	code += '<option value="2"><#Block#></option>';
	code += '</select>';
	code += '</th>';
	code += '<th width="45%"><#Client_Name#> (<#PPPConnection_x_MacAddressForISP_itemname#>)</th>';
	code +='<th width="20%"><#ParentalCtrl_time#></th>';
	code +='<th width="20%"><#list_add_delete#></th></tr>';

	code += '<tr><td style="border-bottom:2px solid #000;">';
	code += '<select id="newrule_Enable" class="input_option">';
	code += '<option value="0"><#btn_disable#></option>';
	code += '<option value="1" selected><#diskUtility_time#></option>';
	code += '<option value="2"><#Block#></option>';
	code += '</select>';
	code += '</td>';
	code +='<td style="border-bottom:2px solid #000;"><input type="text" maxlength="17" style="margin-left:0px;width:255px;" class="input_20_table" name="PC_mac" onKeyPress="return validator.isHWAddr(this,event)" onClick="hideClients_Block();" autocorrect="off" autocapitalize="off" placeholder="ex: <% nvram_get("lan_hwaddr"); %>">';
	code +='<img id="pull_arrow" height="14px;" src="/images/arrow-down.gif" style="position:absolute;" onclick="pullLANIPList(this);" title="<#select_client#>">';
	code +='<div id="ClientList_Block_PC" style="margin:0 0 0 32px" class="clientlist_dropdown"></div></td>';
	code +='<td style="border-bottom:2px solid #000;">--</td>';
	code +='<td style="border-bottom:2px solid #000;"><input class="add_btn" type="button" onClick="addRow_main()" value=""></td></tr>';
	if(client_time_sche_json.length == 0)
		code += '<tr><td style="color:#FFCC00;" colspan="4"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		//user icon
		var userIconBase64 = "NoIcon";
		var clientName, deviceType, deviceVender;
		$.each(client_time_sche_json, function( index, value ) {
			var client_time_obj = value;
			var container_id = client_time_obj.mac.replace(/\:/g, "-");
			var clientIconID = "clientIcon_" + container_id;
			var clientRowID = "clientRow_" +  container_id;
			var clientEditID = "clientEdit_" +  container_id;
			var client_mac = client_time_obj.mac
			clientName = client_time_obj.devicename;
			if(clientList[client_mac]) {
				clientName = (clientList[client_mac].nickName == "") ? clientList[client_mac].name : clientList[client_mac].nickName;
				deviceType = clientList[client_mac].type;
				deviceVender = clientList[client_mac].vendor;
			}
			else {
				deviceType = 0;
				deviceVender = "";
			}
			code += '<tr id="'+clientRowID+'">';
			code += '<td>';
			code += '<select class="input_option eachrule" onchange="genEnableArray_main(this);">';
			code += '<option value="0" ' + ((client_time_obj.enable == "0") ? "selected" : "") + '><#btn_disable#></option>';
			code += '<option value="1" ' + ((client_time_obj.enable == "1") ? "selected" : "") + '><#diskUtility_time#></option>';
			code += '<option value="2" ' + ((client_time_obj.enable == "2")? "selected" : "") + '><#Block#></option>';
			code += '</select>';
			code += '</td>';
			code += '<td title="'+clientName+'">';

			code += '<table width="100%"><tr><td style="width:35%;border:0;float:right;padding-right:30px;">';
			if(clientList[client_mac] == undefined) {
				code += '<div id="' + clientIconID + '" class="clientIcon type0"></div>';
			}
			else {
				if(usericon_support) {
					userIconBase64 = getUploadIcon(client_mac.replace(/\:/g, ""));
				}
				if(userIconBase64 != "NoIcon") {
					code += '<div id="' + clientIconID + '" style="text-align:center;"><img class="imgUserIcon_card" src="' + userIconBase64 + '"></div>';
				}
				else if(deviceType != "0" || deviceVender == "") {
					code += '<div id="' + clientIconID + '" class="clientIcon type' + deviceType + '"></div>';
				}
				else if(deviceVender != "" ) {
					var venderIconClassName = getVenderIconClassName(deviceVender.toLowerCase());
					if(venderIconClassName != "" && !downsize_4m_support) {
						code += '<div id="' + clientIconID + '" class="venderIcon ' + venderIconClassName + '" style="margin-left:-2px;"></div>';
					}
					else {
						code += '<div id="' + clientIconID + '" class="clientIcon type' + deviceType + '"></div>';
					}
				}
			}
			code += '</td><td style="width:65%;text-align:left;border:0;">';
			code += '<div>' + clientName + '</div>';
			code += '<div>' + client_mac + '</div>';
			code += '</td></tr></table>';
			code += '</td>';

			code += '<td><input id=\"' + clientEditID + '\" class=\"edit_btn\" type=\"button\" value=\"\"/></td>';
			code += '<td><input class=\"remove_btn\" type=\"button\" onclick=\"deleteRow_main(this);\" value=\"\"/></td></tr>';
			if(validator.mac_addr(client_mac))
				clientListEventData.push({"mac" : client_mac, "name" : clientName, "ip" : "", "callBack" : "ParentalControl"});
		})
	}
	code += '</table>';

	document.getElementById("mainTable").style.display = "";
	document.getElementById("mainTable").innerHTML = code;
	for(var i = 0; i < clientListEventData.length; i += 1) {
		var clientIconID = "clientIcon_" + clientListEventData[i].mac.replace(/\:/g, "-");
		var clientIconObj = $("#mainTable").children("#mainTable_table").find("#" + clientIconID + "")[0];
		var paramData = JSON.parse(JSON.stringify(clientListEventData[i]));
		paramData["obj"] = clientIconObj;
		$("#mainTable").children("#mainTable_table").find("#" + clientIconID + "").click(paramData, popClientListEditTable);
	}
	$.each(client_time_sche_json, function( index, value ) {
		var client_time_obj = value;
		var clientEditID = "clientEdit_" + client_time_obj.mac.replace(/\:/g, "-");
		$("#mainTable").children("#mainTable_table").find("#" + clientEditID + "").click(client_time_obj, gen_lantowanTable);
	});
	$("#mainTable").fadeIn();
	document.getElementById("ctrlBtn").innerHTML = '<input class="button_gen" type="button" onClick="applyRule(1);" value="<#CTL_apply#>">';

	showDropdownClientList('setClientIP', 'mac', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');
	showclock();
}

function selectAll(){
	$(".eachrule").val($("#selAll").val());
	$.each(client_time_sche_json, function( index, value ) {
		var client_time_obj = value;
		client_time_obj.enable = $("#selAll").val();
	});
	$("#selAll").val("");
}

function applyRule(_on){
	var MULTIFILTER_ENABLE = "";
	var MULTIFILTER_MAC = "";
	var MULTIFILTER_DEVICENAME = "";
	var MULTIFILTER_MACFILTER_DAYTIME_V2 = "";
	$.each(client_time_sche_json, function( index, value ) {
		var client_time_obj = value;
		MULTIFILTER_ENABLE += ((index > 0) ? (">" + client_time_obj.enable) : client_time_obj.enable);
		MULTIFILTER_MAC += ((index > 0) ? (">" + client_time_obj.mac) : client_time_obj.mac);
		var clientObj = clientList[client_time_obj.mac];
		var clientName = client_time_obj.devicename;
		if(clientObj)
			clientName = (clientObj.nickName == "") ? clientObj.name : clientObj.nickName;
		MULTIFILTER_DEVICENAME += ((index > 0) ? (">" + clientName) : clientName);
		MULTIFILTER_MACFILTER_DAYTIME_V2 += ((index > 0) ? (">" + client_time_obj.offtime) : client_time_obj.offtime);
	});

	document.form.MULTIFILTER_ENABLE.value = MULTIFILTER_ENABLE;
	document.form.MULTIFILTER_MAC.value = MULTIFILTER_MAC;
	document.form.MULTIFILTER_DEVICENAME.value = MULTIFILTER_DEVICENAME;
	document.form.MULTIFILTER_MACFILTER_DAYTIME_V2.value = MULTIFILTER_MACFILTER_DAYTIME_V2;

	showLoading();
	document.form.submit();
}

function count_time(){		// To count system time
	systime_millsec += 1000;
	setTimeout("count_time()", 1000);
}

function showclock(){
	JS_timeObj.setTime(systime_millsec);
	JS_timeObj2 = JS_timeObj.toString();	
	JS_timeObj2 = JS_timeObj2.substring(0,3) + ", " +
	              JS_timeObj2.substring(4,10) + "  " +
				  checkTime(JS_timeObj.getHours()) + ":" +
				  checkTime(JS_timeObj.getMinutes()) + ":" +
				  checkTime(JS_timeObj.getSeconds()) + "  " +
				  JS_timeObj.getFullYear();
	document.getElementById("system_time").value = JS_timeObj2;
	setTimeout("showclock()", 1000);
	
	if(svc_ready == "0")
		document.getElementById('svc_hint_div').style.display = "";
	corrected_timezone();
}

function check_macaddr(obj,flag){ //control hint of input mac address
	if(flag == 1){
		var childsel=document.createElement("div");
		childsel.setAttribute("id","check_mac");
		childsel.style.color="#FFCC00";
		obj.parentNode.appendChild(childsel);
		document.getElementById("check_mac").innerHTML="<#LANHostConfig_ManualDHCPMacaddr_itemdesc#>";		
		document.getElementById("check_mac").style.display = "";
		return false;
	}else if(flag ==2){
		var childsel=document.createElement("div");
		childsel.setAttribute("id","check_mac");
		childsel.style.color="#FFCC00";
		obj.parentNode.appendChild(childsel);
		document.getElementById("check_mac").innerHTML="<#IPConnection_x_illegal_mac#>";		
		document.getElementById("check_mac").style.display = "";
		return false;		
	}else{	
		document.getElementById("check_mac") ? document.getElementById("check_mac").style.display="none" : true;
		return true;
	}	
}

function gen_lantowanTable(event){
	weekScheduleApi.PC_init_data(event.data.offtime);
	weekScheduleApi.PC_init_layout("weekScheduleBg");
	var PC_other_client_rule_num = 0;
	$.each(client_time_sche_json, function( index, value ) {
		var client_time_obj = value;
		if(event.data.mac != client_time_obj.mac && client_time_obj.offtime != "")
			PC_other_client_rule_num += client_time_obj.offtime.split("<").length;
	});
	weekScheduleApi.PC_other_client_rule_num = PC_other_client_rule_num;
	weekScheduleApi.callback_btn_cancel = cancel_lantowan;
	weekScheduleApi.callback_btn_apply = function(){
		event.data.offtime = weekScheduleApi.PC_transform_offtime_json_to_string();
		saveto_lantowan()
	};

	$(".schedule_block_on").show();
	$(".schedule_block_off").hide();
}

function addRow_main(){
	var upper = MaxRule_parentctrl;
	var invalid_char = "";
	if(<% nvram_get("MULTIFILTER_ALL"); %> != "1")
		document.form.MULTIFILTER_ALL.value = 1;

	if(client_time_sche_json.length >= upper){
		alert("<#JS_itemlimit1#> " + upper + " <#JS_itemlimit2#>");
		return false;
	}

	var all_client_total_rule_num = 2;//add new client, default 2 rules.
	$.each(client_time_sche_json, function( index, value ) {
		var client_time_obj = value;
		if(client_time_obj.offtime != "")
			all_client_total_rule_num += client_time_obj.offtime.split("<").length;
	});
	if(all_client_total_rule_num > weekScheduleApi.data_max){
		var hint = "<#weekSche_MAX_Num#>".replace("#MAXNUM", weekScheduleApi.data_max);
		hint += "\n";
		hint += "<#weekSche_MAX_Del_Hint#>";
		alert(hint);
		return false;
	}

	if(document.form.PC_mac.value == ""){
		alert("<#JS_fieldblank#>");
		document.form.PC_mac.focus();
		return false;
	}

	var specific_data = client_time_sche_json.filter(function(item, index, array){
		return (item.mac == document.form.PC_mac.value.toUpperCase());
	});
	if(specific_data.length == 1){
		alert("<#JS_duplicate#>");
		document.form.PC_mac.focus();
		return false;
	}
	
	if(!check_macaddr(document.form.PC_mac, check_hwaddr_flag(document.form.PC_mac, 'inner'))){
		document.form.PC_mac.focus();
		document.form.PC_mac.select();
		return false;
	}

	var client_time_obj = new client_time_sche_attr();
	client_time_obj.enable = $("#newrule_Enable").val();
	client_time_obj.mac = document.form.PC_mac.value.toUpperCase();
	var clientObj = clientList[document.form.PC_mac.value.toUpperCase()];
	var clientName = "New device";
	if(clientObj)
		clientName = (clientObj.nickName == "") ? clientObj.name : clientObj.nickName;
	client_time_obj.devicename = clientName;
	client_time_obj.offtime = "W03E21000700<W04122000800";
	client_time_sche_json.push(JSON.parse(JSON.stringify(client_time_obj)));
	document.form.PC_mac.value = "";
	gen_mainTable();
}

function deleteRow_main(obj){
	var client_row_id = $(obj).closest("tr").attr("id");
	var client_mac = client_row_id.replace("clientRow_", "").replace(/-/g, ':').toUpperCase();
	var remove_client_time = function(_mac, _data){
		return _data.filter(function(item, index, array) {
			if (item.mac == _mac)
				return false;
			return true;
		});
	};
	client_time_sche_json = remove_client_time(client_mac, client_time_sche_json);

	$("#mainTable").find("#" + client_row_id + "").remove();
	if(client_time_sche_json.length == 0)
		gen_mainTable();
}

function saveto_lantowan(client){
	gen_mainTable();
	$(".schedule_block_on").hide();
	$(".schedule_block_off").show();
	applyRule();
}

function cancel_lantowan(client){
	gen_mainTable();
	$(".schedule_block_on").hide();
	$(".schedule_block_off").show();
}

function genEnableArray_main(obj){
	var client_mac = $(obj).closest("tr").attr("id").replace("clientRow_", "").replace(/-/g, ':').toUpperCase();
	var specific_data = client_time_sche_json.filter(function(item, index, array){
		return (item.mac == client_mac);
	})[0];
	if(specific_data != undefined)
		specific_data.enable = $(obj).val();
}

function show_inner_tab(){
	var code = "";
	if(document.form.current_page.value == "ParentalControl.asp"){		
		code += "<span class=\"clicked\"><#Time_Scheduling#></span>";
		code += '<a href="AiProtection_WebProtector.asp">';
		code += "<span style=\"margin-left:10px\" class=\"click\"><#AiProtection_filter#></span>";
		code += '</a>';
	}
	else{
		code += '<a href="AiProtection_WebProtector.asp">';
		code += "<span class=\"click\"><#Time_Scheduling#></span>";
		code += '</a>';		
		code += "<span style=\"margin-left:10px\" class=\"clicked\"><#AiProtection_filter#></span>";	
	}
	
	document.getElementById('switch_menu').innerHTML = code;
}
</script></head>

<body onload="initial();" onunload="unload_body();" onselectstart="return false;" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="ParentalControl.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_firewall">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>" disabled>
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="MULTIFILTER_ALL" value="<% nvram_get("MULTIFILTER_ALL"); %>">
<input type="hidden" name="MULTIFILTER_ENABLE" value="<% nvram_get("MULTIFILTER_ENABLE"); %>">
<input type="hidden" name="MULTIFILTER_MAC" value="<% nvram_get("MULTIFILTER_MAC"); %>">
<input type="hidden" name="MULTIFILTER_DEVICENAME" value="<% nvram_get("MULTIFILTER_DEVICENAME"); %>">
<input type="hidden" name="MULTIFILTER_MACFILTER_DAYTIME_V2" value="<% nvram_get("MULTIFILTER_MACFILTER_DAYTIME_V2"); %>">

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
<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0" >
	<tr>
		<td valign="top" >
		
<table width="730px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
	<tbody>
	<tr>
		<td bgcolor="#4D595D" valign="top">
		<div>&nbsp;</div>
		<div style="margin-top:-5px;">
			<table width="730px">
				<tr>
					<td align="left">
						<div id="content_title" class="formfonttitle" style="width:400px"><#Parental_Control#></div>
					</td>				
					<td>
						<div id="switch_menu" style="margin:-20px 0px 0px -5px;display:none;">
							<a href="AiProtection_WebProtector.asp">
								<div style="width:168px;height:30px;border-top-left-radius:8px;border-bottom-left-radius:8px;" class="block_filter">
									<table class="block_filter_name_table"><tr><td style="line-height:13px;"><#AiProtection_filter#></td></tr></table>
								</div>
							</a>
							<div style="width:160px;height:30px;margin:-32px 0px 0px 168px;border-top-right-radius:8px;border-bottom-right-radius:8px;" class="block_filter_pressed">
								<table class="block_filter_name_table_pressed"><tr><td style="line-height:13px;"><#Time_Scheduling#></td></tr></table>
							</div>
						</div>
					<td>
				</tr>
			</table>
			<div style="margin:0 0 10px 5px;" class="splitLine"></div>
		</div>
		<div id="PC_desc">
			<table width="700px" style="margin-left:25px;">
				<tr>
					<td>
						<div id="guest_image" style="background: url(images/New_ui/parental-control.png);width: 130px;height: 87px;"></div>
					</td>
					<td>&nbsp;&nbsp;</td>
					<td style="font-style: italic;font-size: 14px;">
						<span id="desc_title"><#ParentalCtrl_Desc#></span>
						<ol>	
							<li><#ParentalCtrl_Desc1#></li>
							<li><#ParentalCtrl_Desc2#></li>
							<li><#ParentalCtrl_Desc3#></li>
							<li><#ParentalCtrl_Desc4#></li>
							<li><#ParentalCtrl_Desc5#></li>							
						</ol>
						<span id="desc_note" style="color:#FC0;"><#ADSL_FW_note#></span>
						<ol style="color:#FC0;margin:-5px 0px 3px -18px;*margin-left:18px;">
							<li><#ParentalCtrl_default#></li>
							<li id="nat_desc"><#ParentalCtrl_disable_NAT#></li>
						</ol>	
					</td>
				</tr>
			</table>
		</div>
			<!--=====Beginning of Main Content=====-->
			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable schedule_block_off">
				<tr>
					<th id="PC_enable"><#ParentalCtrl_Enable#></th>
					<td>
						<div align="center" class="left" style="width:94px; float:left; cursor:pointer;" id="radio_ParentControl_enable"></div>
						<div class="iphone_switch_container" style="height:32px; width:74px; position: relative; overflow: hidden">
							<script type="text/javascript">
								$('#radio_ParentControl_enable').iphoneSwitch('<% nvram_get("MULTIFILTER_ALL"); %>',
									function(){
											document.form.MULTIFILTER_ALL.value = 1;
											$(".switch_on_content").show();
									},
									function(){
										document.form.MULTIFILTER_ALL.value = 0;
										$(".switch_on_content").hide();
										if(document.form.MULTIFILTER_ALL.value == '<% nvram_get("MULTIFILTER_ALL"); %>')
											return false;

										applyRule(1);
									}
								);
							</script>
						</div>
					</td>
				</tr>
				<tr class="switch_on_content">
					<th><#General_x_SystemTime_itemname#></th>
					<td align="left"><input type="text" id="system_time" name="system_time" class="devicepin" value="" readonly="1" style="font-size:12px;width:200px;" autocorrect="off" autocapitalize="off">
						<div id="svc_hint_div" style="display:none;"><span onClick="location.href='Advanced_System_Content.asp?af=ntp_server0'" style="color:#FFCC00;text-decoration:underline;cursor:pointer;"><#General_x_SystemTime_syncNTP#></span></div>
							<div id="timezone_hint_div" style="display:none;"><span id="timezone_hint" onclick="location.href='Advanced_System_Content.asp?af=time_zone_select'" style="color:#FFCC00;text-decoration:underline;cursor:pointer;"></span></div>
					</td>
				</tr>
			</table>
			<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0" style="display:none" class="switch_on_content schedule_block_off">
				<tr>
					<td valign="top" align="center">
						<!-- Content -->
						<div id="mainTable" style="margin-top:10px;"></div>
						<br>
						<div id="ctrlBtn" style="text-align:center;"></div>
						<!-- Content -->
					</td>
				</tr>
			</table>
			<div class="schedule_block_on" style="display:none">
				<div id="edit_time_anchor"></div>
				<div id="weekScheduleBg"></div>
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
<div id="footer"></div>
</form>
</body>
</html>
