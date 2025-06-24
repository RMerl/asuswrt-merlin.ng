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

<title><#Web_Title#> - <#menu5_1_4#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="/device-map/device-map.css">
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/client_function.js"></script>
<script language="JavaScript" type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" src="/form.js"></script>
<style>
#pull_arrow{
 	float:center;
 	cursor:pointer;
 	border:2px outset #EFEFEF;
 	background-color:#CCC;
 	padding:3px 2px 4px 0px;
	*margin-left:-3px;
	*margin-top:1px;
}
</style>
<script>
<% wl_get_parameter(); %>

// merge wl_maclist_x
var wl_maclist_x_array = '<% nvram_get("wl_maclist_x"); %>';

var manually_maclist_list_array = new Array();
var manually_maclist_list_ori_array = new Array();
var wl_macmode_ori = ('<% nvram_get("wl_macmode"); %>' == "disabled") ? "allow" : '<% nvram_get("wl_macmode"); %>';

var current_page = window.location.pathname.split("/").pop();
var faq_index_tmp = get_faq_index(FAQ_List, current_page, 1);

function initial(){
	if(isSwMode("RP") && concurrep_support){
		document.form.wl_subunit.value = 1;
		if('<% nvram_get("wl_subunit"); %>' != '1') change_wl_unit();
	}

	show_menu();
	regen_band(document.form.wl_unit);
	if(lantiq_support){
		checkWLReady();
	}	

	if(isSupport("noWiFi"))
		document.getElementById("wl_unit_field").style.display = "none";

	var wl_maclist_x_row = wl_maclist_x_array.split('&#60');
	var clientName = "New device";
	for(var i = 1; i < wl_maclist_x_row.length; i += 1) {
		var _index = wl_maclist_x_row[i].toUpperCase();
		if(clientList[_index]) {
			clientName = (clientList[_index].nickName == "") ? clientList[_index].name : clientList[_index].nickName;
		}
		else {
			clientName = "New device";
		}

		manually_maclist_list_array[_index] = clientName;
		manually_maclist_list_ori_array[_index] = clientName;
	}

	if((sw_mode == 2 || sw_mode == 4) && document.form.wl_unit.value == '<% nvram_get("wlc_band"); %>' && !concurrep_support){
		for(var i=3; i>=3; i--)
			document.getElementById("MainTable1").deleteRow(i);
		for(var i=2; i>=0; i--)
			document.getElementById("MainTable2").deleteRow(i);
		document.getElementById("repeaterModeHint").style.display = "";
		document.getElementById("wl_maclist_x_Block").style.display = "none";
		document.getElementById("submitBtn").style.display = "none";
	}
	else{
		show_wl_maclist_x();
	}

	setTimeout("showDropdownClientList('setClientmac', 'mac', 'wl', 'WL_MAC_List_Block', 'pull_arrow', 'all');", 1000);
	check_macMode();
}

function show_wl_maclist_x(){
	var code = "";
	var clientListEventData = [];
	code += '<table width="100%" cellspacing="0" cellpadding="4" align="center" class="list_table"  id="wl_maclist_x_table">';
	if(Object.keys(manually_maclist_list_array).length == 0)
		code += '<tr><td style="color:#FFCC00;"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		//user icon
		var userIconBase64 = "NoIcon";
		var clientName, deviceType, deviceVendor;
		Object.keys(manually_maclist_list_array).forEach(function(key) {
			var clientMac = key.toUpperCase();
			var clientIconID = "clientIcon_" + clientMac.replace(/\:/g, "");
			if(clientList[clientMac]) {
				clientName = (clientList[clientMac].nickName == "") ? clientList[clientMac].name : clientList[clientMac].nickName;
				deviceType = clientList[clientMac].type;
				deviceVendor = clientList[clientMac].vendor;
			}
			else {
				clientName = "New device";
				deviceType = 0;
				deviceVendor = "";
			}
			code +='<tr id="row_'+clientMac+'">';
			code +='<td width="80%" align="center">';
			code += '<table style="width:100%;"><tr><td style="width:35%;height:56px;border:0px;float:right;">';
			if(clientList[clientMac] == undefined) {
				code += '<div id="' + clientIconID + '" class="clientIcon type0"></div>';
			}
			else {
				if(usericon_support) {
					userIconBase64 = getUploadIcon(clientMac.replace(/\:/g, ""));
				}
				if(userIconBase64 != "NoIcon") {
                    if(clientList[clientMac].isUserUplaodImg){
					    code += '<div id="' + clientIconID + '" class="clientIcon"><img class="imgUserIcon_card" src="' + userIconBase64 + '"></div>';
                    }else{
                        code += '<div id="' + clientIconID + '" class="clientIcon"><i class="type" style="--svg:url(' + userIconBase64 + ')"></i></div>';
                    }
				}
				else if(deviceType != "0" || deviceVendor == "") {
					code += '<div id="' + clientIconID + '" class="clientIcon"><i class="type'+deviceType+'"></i></div>';
				}
				else if(deviceVendor != "" ) {
					var vendorIconClassName = getVendorIconClassName(deviceVendor.toLowerCase());
					if(vendorIconClassName != "" && !downsize_4m_support) {
						code += '<div id="' + clientIconID + '" class="clientIcon"><i class="vendor-icon '+ vendorIconClassName +'"></i></div>';
					}
					else {
						code += '<div id="' + clientIconID + '" class="clientIcon"><i class="type' + deviceType + '"></i></div>';
					}
				}
			}
			code += '</td><td style="width:60%;border:0px;">';
			code += '<div>' + clientName + '</div>';
			code += '<div>' + clientMac + '</div>';
			code += '</td></tr></table>';
			code += '</td>';
			code += '<td width="20%"><input type="button" class=\"remove_btn\" onclick=\"deleteRow(this, \'' + clientMac + '\');\" value=\"\"/></td></tr>';
			if(validator.mac_addr(clientMac))
				clientListEventData.push({"mac" : clientMac, "name" : "", "ip" : "", "callBack" : "ACL"});
		});
	}
	code += '</table>';
	document.getElementById("wl_maclist_x_Block").innerHTML = code;
	for(var i = 0; i < clientListEventData.length; i += 1) {
		var clientIconID = "clientIcon_" + clientListEventData[i].mac.replace(/\:/g, "");
		var clientIconObj = $("#wl_maclist_x_Block").children("#wl_maclist_x_table").find("#" + clientIconID + "")[0];
		var paramData = JSON.parse(JSON.stringify(clientListEventData[i]));
		paramData["obj"] = clientIconObj;
		$("#wl_maclist_x_Block").children("#wl_maclist_x_table").find("#" + clientIconID + "").click(paramData, popClientListEditTable);
	}
}

function deleteRow(r, delMac){
	var i = r.parentNode.parentNode.rowIndex;
	delete manually_maclist_list_array[delMac];
	document.getElementById('wl_maclist_x_table').deleteRow(i);
	if(Object.keys(manually_maclist_list_array).length == 0)
		show_wl_maclist_x();
}

function addRow(obj, upper){
	var rule_num = document.getElementById('wl_maclist_x_table').rows.length;
	var item_num = document.getElementById('wl_maclist_x_table').rows[0].cells.length;
	var mac = obj.value.toUpperCase();

	if(rule_num >= upper){
		alert("<#AiMesh_Binding_Rule_Maxi#>\n<#AiMesh_Delete_Unused_Rule#>");
		return false;	
	}	
	
	if(mac==""){
		alert("<#JS_fieldblank#>");
		obj.focus();
		obj.select();			
		return false;
	}else if(!check_macaddr(obj, check_hwaddr_flag(obj, 'inner'))){
		obj.focus();
		obj.select();	
		return false;	
	}
		
		//Viz check same rule
	for(i=0; i<rule_num; i++){
		for(j=0; j<item_num-1; j++){	
			if(manually_maclist_list_array[mac] != null){
				alert("<#JS_duplicate#>");
				return false;
			}	
		}		
	}		

	if(document.form.enable_mac[0].checked){
		if(isSupport("amas") && isSupport("force_roaming") && isSupport("sta_ap_bind")){
			var rule_hint = "";
			var acl_and_client_bind_allow_maximum = 0;
			if(isSupport("acl96") && document.form.wl_macmode_show.value == "deny")
				acl_and_client_bind_allow_maximum = 96;
			else if(!isSupport("acl96")){
				var cfg_re_maxnum = httpApi.hookGet("get_onboardingstatus").cfg_re_maxnum;
				acl_and_client_bind_allow_maximum = 64 - (cfg_re_maxnum * 2);
			}

			if(acl_and_client_bind_allow_maximum > 0){
				var sta_binding_count = 0;
				var sta_binding_list = decodeURIComponent(httpApi.nvramCharToAscii(["sta_binding_list"], true).sta_binding_list);
				var each_node_rule = sta_binding_list.split("<");
				$.each(each_node_rule, function(index, value){
					if(value != ""){
						var node_client_rule = value.split(">");
						var node_mac = "";
						$.each(node_client_rule, function(index, value){
							switch(index){
								case 2://client list
									var each_client = value.split("|");
									$.each(each_client, function(index, value){
										sta_binding_count++;
									});
									break;
							}
						});
					}
				});

				var current_rule_count = Object.keys(manually_maclist_list_array).length;
				if((sta_binding_count + current_rule_count) >= acl_and_client_bind_allow_maximum) {
					rule_hint = "<#AiMesh_Binding_Rule_Maxi#>";
					rule_hint += "\n";
					rule_hint += "<#AiMesh_Delete_Unused_Rule#>";
					if(sta_binding_count != 0){
						rule_hint += "\n";
						rule_hint += "- AiMesh client binding rule";/* untranslated */
					}
					if(current_rule_count != 0){
						rule_hint += "\n";
						rule_hint += "- <#FirewallConfig_MFList_groupitemname#>";
					}
				}
			}

			if(rule_hint != ""){
				alert(rule_hint);
				return false;
			}
		}
	}

	if(clientList[mac]) {
		manually_maclist_list_array[mac] = (clientList[mac].nickName == "") ? clientList[mac].name : clientList[mac].nickName;
	}
	else {
		manually_maclist_list_array[mac] = "New device";
	}

	obj.value = "";
	show_wl_maclist_x();
}

function applyRule(){
	if(lantiq_support && wave_ready != 1){
		alert(`<#Wireless_ready#>`);
		return false;
	}
	
	var rule_num = document.getElementById('wl_maclist_x_table').rows.length;
	var item_num = document.getElementById('wl_maclist_x_table').rows[0].cells.length;
	var tmp_value = "";

	Object.keys(manually_maclist_list_array).forEach(function(key) {
		key = key.toUpperCase();
		tmp_value += "<" + key;
	});

	if(tmp_value == "<"+"<#IPConnection_VSList_Norule#>" || tmp_value == "<")
		tmp_value = "";	

	if(document.form.enable_mac[1].checked)	
		document.form.wl_macmode.value = "disabled";
	else
		document.form.wl_macmode.value = document.form.wl_macmode_show.value;
	
	if(prevent_lock(tmp_value)){
		document.form.wl_maclist_x.value = tmp_value;
		showLoading(3);

		let nvramSetObj = {'action_mode': 'apply', 'rc_service': 'restart_wireless;'};
		const wl_macmode = document.form.wl_macmode.value;
		if (isSupport("noWiFi")) {
			const wl_ifname_count = 4;
			for (let i = 0; i < wl_ifname_count; i++) {
				nvramSetObj[`wl${i}_maclist_x`] = tmp_value;
				nvramSetObj[`wl${i}_macmode`] = wl_macmode;
			}
		}
		else {
			nvramSetObj[`wl_maclist_x`] = tmp_value;
			nvramSetObj[`wl_macmode`] = wl_macmode;
			nvramSetObj['wl_unit'] = document.form.wl_unit.value;
			nvramSetObj['wl_subunit'] = document.form.wl_subunit.value;
		}
		httpApi.nvramSet(nvramSetObj);
	}
}

function done_validating(action){
	refreshpage();
}

function prevent_lock(rule_num){
	if(document.form.wl_macmode.value == "allow" && rule_num == ""){
		alert("<#FirewallConfig_MFList_accept_hint1#>");
		return false;
	}

	return true;
}

function change_wl_unit(){
	FormActions("apply.cgi", "change_wl_unit", "", "");
	document.form.target = "";
	document.form.submit();
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

//Viz add 2013.01 pull out WL client mac START
function pullWLMACList(obj){	
	var element = document.getElementById('WL_MAC_List_Block');
	var isMenuopen = element.offsetWidth > 0 || element.offsetHeight > 0;
	if(isMenuopen == 0){		
		obj.src = "/images/unfold_less.svg"
		element.style.display = "block";
		document.form.wl_maclist_x_0.focus();		
	}
	else
		hideClients_Block();
}

function hideClients_Block(){
	document.getElementById("pull_arrow").src = "/images/unfold_more.svg";
	document.getElementById("WL_MAC_List_Block").style.display="none";
}

function setClientmac(macaddr){
	document.form.wl_maclist_x_0.value = macaddr;
	hideClients_Block();
}
//Viz add 2013.01 pull out WL client mac END

function check_macMode(){
	if(document.form.wl_macmode.value == "disabled"){
		document.form.enable_mac[1].checked = true;
	}
	else{
		document.form.enable_mac[0].checked = true;
	}	
	
	enable_macMode();
}

function enable_macMode(){
	if(document.form.enable_mac[0].checked){
		document.getElementById('mac_filter_mode').style.display = "";
		document.getElementById('MainTable2').style.display = "";
		document.getElementById('wl_maclist_x_Block').style.display = "";
		document.form.wl_maclist_x.disabled = false;
	}	
	else{
		document.getElementById('mac_filter_mode').style.display = "none";
		document.getElementById('MainTable2').style.display = "none";
		document.getElementById('wl_maclist_x_Block').style.display = "none";
		document.form.wl_maclist_x.disabled = true;
	}	
}

function checkWLReady(){
	$.ajax({
	    url: '/ajax_wl_ready.asp',
	    dataType: 'script',	
	    error: function(xhr) {
			setTimeout("checkWLReady();", 1000);
	    },
	    success: function(response){
	    	if(wave_ready != 1){
	    		$("#lantiq_ready").show();
	    		setTimeout("checkWLReady();", 1000);
	    	}
	    	else{
	    		$("#lantiq_ready").hide();
	    	}	
	    }
  	});
}
function change_wl_macmode(){
	if(wl_macmode_ori != document.form.wl_macmode_show.value){
		manually_maclist_list_array = [];
		$("#change_filter_mode_hint").css("display", "flex");
	}
	else{
		manually_maclist_list_array = [];
		Object.keys(manually_maclist_list_ori_array).forEach(function(key) {
			var clientMac = key.toUpperCase();
			var clientName = manually_maclist_list_ori_array[key];
			manually_maclist_list_array[clientMac] = clientName;
		});
		$("#change_filter_mode_hint").css("display", "none");
	}

	show_wl_maclist_x();
}
</script>
</head>

<body onload="initial();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="17">&nbsp;</td>		
		<td valign="top" width="202">	
			<div  id="mainMenu"></div>	
			<div  id="subMenu"></div>		
		</td>						
    <td valign="top">
	<div id="tabMenu" class="submenuBlock"></div>
		<!--===================================Beginning of Main Content===========================================-->
<input type="hidden" name="current_page" value="Advanced_ACL_Content.asp">
<input type="hidden" name="next_page" value="Advanced_ACL_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply_new">
<input type="hidden" name="action_wait" value="3">
<input type="hidden" name="action_script" value="restart_wireless">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="wl_maclist_x" value="">
<input type="hidden" name="wl_macmode" value="<% nvram_get("wl_macmode"); %>">
<input type="hidden" name="wl_subunit" value="-1">

<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
	<tr>
		<td valign="top" >
		
		<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
		<tbody>
			<tr>
				<td bgcolor="#4D595D" valign="top">
				<div class="container">

					<div>&nbsp;</div>
					<div class="formfonttitle"><#menu5_1#> - <#menu5_1_4#></div>
					<div class="formfonttitle_help"><i onclick="show_feature_desc(`<#HOWTOSETUP#>`)" class="icon_help"></i></div>
					<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
					<div class="formfontdesc"><#DeviceSecurity11a_display1_sectiondesc#></div>
					<div id="lantiq_ready" style="display:none;color:#FC0;margin-left:5px;font-size:13px;">Wireless is setting...</div>
					<table id="MainTable1" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
						<thead>
						  <tr>
							<td colspan="2"><#t2BC#></td>
						  </tr>
						</thead>		

						<tr id="wl_unit_field">
							<th><#Interface#></th>
							<td>
								<select name="wl_unit" class="input_option" onChange="change_wl_unit();">
									<option class="content_input_fd" value="0" <% nvram_match("wl_unit", "0","selected"); %>>2.4GHz</option>
									<option class="content_input_fd" value="1" <% nvram_match("wl_unit", "1","selected"); %>>5GHz</option>
								</select>			
							</td>
					  </tr>

						<tr id="repeaterModeHint" style="display:none;">
							<td colspan="2" style="color:#FFCC00;height:30px;" align="center"><#page_not_support_mode_hint#></td>
						</tr>
						<tr>
							<th width="30%"><#enable_macmode#></th>
							<td>
								<input type="radio" name="enable_mac" value="0" onclick="enable_macMode();"><#checkbox_Yes#>
								<input type="radio" name="enable_mac" value="1" onclick="enable_macMode();"><#checkbox_No#>
							</td>
						</tr>
						<tr id="mac_filter_mode">
							<th width="30%" >
								<a class="hintstyle" href="javascript:void(0);" onClick="openHint(18,1);"><#FirewallConfig_MFMethod_itemname#></a>
							</th>
							<td>
								<select name="wl_macmode_show" class="input_option" onChange="change_wl_macmode();">
									<option class="content_input_fd" value="allow" <% nvram_match("wl_macmode", "allow","selected"); %>><#FirewallConfig_MFMethod_item1#></option>
									<option class="content_input_fd" value="deny" <% nvram_match("wl_macmode", "deny","selected"); %>><#FirewallConfig_MFMethod_item2#></option>
								</select>
								<span id="change_filter_mode_hint" style="margin-top:4px;display:none;">The "MAC filter list" will be removed when you switch the "MAC Filter Mode".</span>
							</td>
						</tr>
					</table>
					<table id="MainTable2" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table">
						<thead>
							<tr>
								<td colspan="2"><#FirewallConfig_MFList_groupitemname#>&nbsp;(<#List_limit#>&nbsp;64)</td>
							</tr>
						</thead>
							<tr>
								<th width="80%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,10);"><#Client_Name#> (<#PPPConnection_x_MacAddressForISP_itemname#>)</th> 
								<th width="20%"><#list_add_delete#></th>
							</tr>
							<tr>
								<td width="80%">
									<input type="text" maxlength="17" class="input_macaddr_table" name="wl_maclist_x_0" onKeyPress="return validator.isHWAddr(this,event)" onClick="hideClients_Block();" autocorrect="off" autocapitalize="off" placeholder="ex: <% nvram_get("lan_hwaddr"); %>" style="width:255px;">
									<img id="pull_arrow" class="pull_arrow_acl_business" height="14px;" src="/images/unfold_more.svg" style="position:absolute;" onclick="pullWLMACList(this);" title="<#select_wireless_MAC#>">
									<div id="WL_MAC_List_Block" class="clientlist_dropdown" style="margin-left:167px;"></div>
								</td>
								<td width="20%">	
									<input type="button" class="add_btn" onClick="addRow(document.form.wl_maclist_x_0, 64);" value="">
								</td>
							</tr>      		
					</table>
						<div id="wl_maclist_x_Block"></div>			
						<div id="submitBtn" class="apply_gen">
							<input class="button_gen" onclick="applyRule()" type="button" value="<#CTL_apply#>"/>
						</div>


					</div>	<!-- for .container  -->
					<div class="popup_container popup_element_second"></div>

				</td>
			</tr>
		</tbody>
		</table>
		</td>
</form>
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
