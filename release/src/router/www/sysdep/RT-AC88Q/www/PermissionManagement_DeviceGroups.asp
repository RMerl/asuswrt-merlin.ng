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
<title><#Permission_Management#> - <#Permission_Management_DGroups#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/disk_functions.js"></script>
<script type="text/javascript" src="/form.js"></script>
<script type="text/javascript" src="/client_function.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<style>
.contentM_qis{
	position:absolute;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius: 5px;
	z-index:200;
	background-color:#2B373B;
	margin-left:226px;
	margin-top: 10px;
	width:740px;
	box-shadow: 3px 3px 10px #000;
	display: none;
}
</style>
<script>
window.onresize = function() {
	if(document.getElementById("addTable").style.display == "block") {
		cal_panel_block("addTable", 0.2);
	}
}
var info = new Object();
info.device = new Array();
info.group = new Array();

var device_list = <% pms_device_info(); %>;
var group_list = <% pms_devgroup_info(); %>;
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

$(document).ready(function (){
	show_menu();
	collect_info();
	generate_group_table();
});


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
		var device_name = object.devname;
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

function generate_group_table(){
	var code = "";	
	code += '<tr>';
	//code += '<th style="width:50px;"><input id="" type="checkbox" onclick="" value=""></th>';
	code += '<th style="width:450px;">Group Name</th>';	
	code += '<th style="width:450px;"><#Description#></th>';	
	code += '<th style="width:60px;"><#CTL_modify#></th>';	
	code += '<th style="width:60px;"><#CTL_del#></th>';	
	code += '</tr>';

	for(i=0;i<info.group.length;i++){
		var group_index = info.group[i];
		var group_obj = info.group[group_index];
		var group_active = group_obj.active;
		var group_name = group_obj.name;
		var group_description = group_obj.description;
		var checked_state = group_active ? "checked" :  "";

		code += '<tr>';			
		//code += '<td><div><input id="' + group_index + '" type="checkbox" onclick="" ' + checked_state + '></div></td>';
		code += '<td><div>' + group_obj.name + '</div></td>';
		code += '<td><div>' + group_obj.description + '</div></td>';
		code += '<td><div class="modifyAccountBtn_add" onclick=\'show_modifyTable("group", "' + group_name + '");\'></div></td>';
		code += '<td>';
		if((group_obj.name != "Administrator") && (group_obj.name != "default"))
			code += '<div class="deleteAccountBtn_add" onclick=\'delete_group("' + group_obj.name + '");\'></div>';
		
		code += '</td>';
		code += '</tr>';
	}
	
	$("#group_table").html(code);
}

function show_addTable(flag){
	var code = "";
	cal_panel_block("addTable", 0.2);
	$("#addTable").show();
	$("#addTable").css({"height":"600px", "margin-top":"-100px"});
	$("#group_name").prop("disabled", false);
	$("#group_info").show();



		//generate group table
	code += '<tr>';
	code += '<th style="width:35px;"><input type="checkbox" onclick="enable_device_all(this);" value=""></th>';
	code += '<th style="width:94%;"><#ShareNode_DeviceName_itemname#></th>';
	code += '</tr>';
	for(i=0;i<info.device.length;i++){
		var device_index = info.device[i];
		var device_obj = info.device[device_index];
		code += '<tr>';
		code += '<td style="width:35px;"><input id="' + device_index + '" type="checkbox" onclick="" value=""></td>';
		code += '<td>' + device_obj.name + '</td>';
		code += '<tr>';
	}
		
	$("#device_table").html(code);
	$("#add_group_btn").click(function(){
		add_group(flag);
	});
}

function hide_addTable(){
	$("#addTable").hide();
	$("#group_info").hide();
	//reset
	$("#group_active").val("1");
	$("#group_name").val("");
	$("#group_desc").val("");
	
	//reset unbine button event
	$("#add_group_btn").unbind("click");
	$("#group_name").prop("disabled", false);
}

function add_group(flag, target){
	var group_active = $("#group_active").val();
	var group_name = $("#group_name").val();
	var group_description = $("#group_desc").val();

	// validate form
	if(!Block_chars(document.getElementById("group_name"), ["<", ">"])){
		return false;
	}
	else if(document.getElementById("group_name").value == ''){
		alert("<#File_Pop_content_alert_desc1#>");
		document.getElementById("group_name").focus();
		return false;
	}

	if(flag != "modify"){
		for(i=0;i<info.group.length;i++){
			var group_index = info.group[i];
			if(document.getElementById("group_name").value == info.group[group_index].name){
				alert("<#File_Pop_content_alert_desc5#>");
				document.getElementById("group_name").focus();
				return false;
			}
		}
	}	

	if(!Block_chars(document.getElementById("group_desc"), ["<", ">"]))
		return false;
	// end validate form

	document.form.action = "apply.cgi";
	document.form.action_mode.value = "pms_apply";
	document.form.pms_action.value = "devgroup_update";
	document.form.pms_devgrp_active.value = group_active;
	document.form.pms_devgrp_name.value = group_name;
	document.form.pms_grp_desc.value = group_description;

	// to generate pms_owned_device format
	var device_array = new Array();
	for(i=0;i<info.device.length;i++){
		var device_index = info.device[i];
		var device_obj = info.device[device_index];
		var device_mac = device_obj.mac;
		console.log(device_index);
		var checked_state = document.getElementById(device_index).checked;
		if(checked_state){
			device_array.push(device_mac);
		}	
	}

	var code = "";
	for(i=0;i<device_array.length;i++){
		if(i != 0)
			code += ">";

		code += device_array[i];
	}

	var device_length = device_array.length;
	if(device_length == 0)
		device_length = 1

	// end generate pms_owned_device format
	
	document.form.pms_owned_device.value = code;
	document.form.pms_owned_device_num.value = device_length;
	document.form.submit();
	setTimeout("update_list();", 500);

	$("#group_name").prop("disabled", false);
	hide_addTable();
}

function delete_group(target){
	document.form.action = "apply.cgi";
	document.form.action_mode.value = "pms_apply";
	document.form.pms_action.value = "devgroup_delete";
	document.form.pms_devgrp_name.value = target;
	document.form.submit();
	setTimeout("update_list();", 500);
}


function show_modifyTable(type, target){
	cal_panel_block("addTable", 0.2);
	$("#addTable").show();
	$("#addTable").css({"height":"600px", "margin-top":"-100px"});
	var group_index = "_" + target;
	var group_obj = info.group[group_index];
	var group_active = group_obj.active;
	var group_name = group_obj.name;
	var group_description = group_obj.description;
	$("#group_active").val(group_active ? 1 : 0);
	$("#group_name").val(group_name, true);
	$("#group_name").prop("disabled", true);
	$("#group_desc").val(group_description);
	
	var code = "";
	code += '<tr>';
	code += '<th style="width:35px;"><input type="checkbox" onclick="enable_device_all(this)" value=""></th>';
	code += '<th style="width:94%;"><#ShareNode_DeviceName_itemname#></th>';
	code += '</tr>';
	var device_array = group_obj.members;
	for(i=0;i<info.device.length;i++){
		var device_index = info.device[i];
		var device_obj = info.device[device_index];
		var checked_state = (device_array.indexOf(device_index) != -1) ? "checked" : "";
		code += '<tr>';
		code += '<td style="width:35px;"><input id="' + device_index + '" type="checkbox" '+ checked_state +'></td>';
		code += '<td>' + device_obj.name + '</td>';
		code += '<tr>';
	}

	$("#device_table").html(code);
	$("#group_info").show();
	$("#add_group_btn").click(function(){
		add_group("modify", target);
	});
}

function applyRule(){
	showLoading(5);
	document.form.action_script.value = "restart_pms_device;";
	document.form.submit();
}

function update_list(){
  $.ajax({
    url: '/getDeviceList.asp',
    dataType: 'script',	
    error: function(xhr) {
		//setTimeout("update_list();", 10000);
    },
    success: function(response){
    	device_list = device_list_hook;
    	group_list = group_list_hook;
    	collect_info();
    	generate_group_table();
    }
  });	
}

function enable_device_all(obj){
	var check_state = obj.checked;
	console.log(obj);
	for(i=0;i<info.device.length;i++){
		var index = info.device[i];
		document.getElementById(index).checked = check_state;
	}
}
</script>
</head>

<body>
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="PermissionManagement_DeviceGroups.asp">
<input type="hidden" name="next_page" value="PermissionManagement_DeviceGroups.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">

<input type="hidden" name="pms_action" value="">
<input type="hidden" name="pms_devgrp_active" value="">
<input type="hidden" name="pms_devgrp_name" value="">
<input type="hidden" name="pms_grp_desc" value="">
<input type="hidden" name="pms_owned_device" value="">
<input type="hidden" name="pms_owned_device_num" value="">
</form>

<div id="addTable" class="contentM_qis" style="height:670px;">
	<div id="group_info" style="display:none">
		<table width="97%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" style="margin: 20px 10px;">
			<thead>
				<tr>
					<td colspan="6">Create a new group</td>
				</tr>
			</thead>		  
			<tr>
				<th width="30%" style="font-family: Calibri;font-weight: bolder;"><#Status_Active#></th>			
				<td>
					<select id="group_active" class="input_option">
						<option value="1"><#WLANConfig11b_WirelessCtrl_button1name#></option>
						<option value="0"><#WLANConfig11b_WirelessCtrl_buttonname#></option>
					</select>
				</td>
			</tr>		
			<tr>
				<th width="30%" style="font-family: Calibri;font-weight: bolder;">Group Name</th>			
				<td>
					<input id="group_name" type="text" maxlength="32" class="input_32_table" style="height: 23px;" autocorrect="off" autocapitalize="off">
				</td>
			</tr>
			<tr>
				<th width="30%" style="font-family: Calibri;font-weight: bolder;"><#Description#></th>			
				<td>
					<input id="group_desc" type="text" maxlength="32" class="input_32_table" style="height: 23px;" autocorrect="off" autocapitalize="off">
				</td>
			</tr>			
		</table>
		<div style="display:flex;margin: 0 20px;">
			<div style="width:100%;">
				<div>Assign devices to this group</div>
				<div id="current_device" style="padding: 10px 0 0 20px;color:#FC0"></div>
			</div>
		</div>
		<div style="height:325px;overflow:auto;">
			<table id="device_table" width="97%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" style="margin: 20px 10px;"></table>
		</div>	
		<!--div id="device_field" style="width:95%;height:300px;background-color:#606E73;overflow-y:auto;margin: 0 15px;"></div-->		
		<div class="apply_gen" style="margin-top:20px;margin-bottom:10px;background-color: #2B373B;">
			<input name="button" type="button" class="button_gen" onclick="hide_addTable();" value="<#CTL_Cancel#>"/>
			<input id="add_group_btn" name="button" type="button" class="button_gen" onclick="" value="<#CTL_apply#>"/>
		</div>
	</div>

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
			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
				<tr>
					<td valign="top" >		
						<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
							<tbody>
							<tr>
								<td bgcolor="#4D595D" valign="top">
									<div>&nbsp;</div>
									<div class="formfonttitle"><#Permission_Management#> - <#Permission_Management_DGroups#></div>
									<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
									<div class="formfontdesc"><#PM_DGroups_desc#></div>
									<div>
										<div style="display:flex">
											<div style="font-weight:900;padding-left:10px;line-height:34px;">Group Table</div>
											<div style="margin-left:10px;">
												<div class="createAccountBtn_add" onclick="show_addTable('new');"></div>
											</div>										
										</div>									
										<table id="group_table" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable"></table>
									</div>								
									<div class="apply_gen">
										<input type="button" class="button_gen" onclick="applyRule();" value="<#CTL_apply#>"/>
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
<form method="post" name="aidiskForm" action="" target="hidden_frame">
<input type="hidden" name="motion" id="motion" value="">
<input type="hidden" name="layer_order" id="layer_order" value="">
<input type="hidden" name="test_flag" value="" disabled="disabled">
<input type="hidden" name="protocol" id="protocol" value="">
</form>
</body>
</html>
