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
<title><#Permission_Management#> - <#Permission_Management_Devices#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="/device-map/device-map.css">
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
	generate_device_table();
	showDropdownClientList('setClientIP', 'mac', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');
});


function device_object(name, mac, type, type_name, description, group_array){
	this.name = name;
	this.mac = mac;
	this.type = type;
	this.type_name = type_name;
	this.description = description;
	this.group = group_array;
}

function device_group_object(active, name, device_array){
	this.active = (active == 1) ? true : false;
	this.name = name;
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
		var device_array = new Array();
		device_array = object.owned_device;


		info.group.push(group_index);
		info.group[group_index] = new device_group_object(group_active, group_name, device_array);
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

function generate_device_table(){
	var code = "";
	code += '<tr>';
	code += '<th style="width:450px;"><#ShareNode_DeviceName_itemname#></th>';
	code += '<th style="width:150px;"><#MAC_Address#></th>';
	code += '<th style="width:200px;"><#Device_type#></th>';
	code += '<th style="width:200px;"><#Description#></th>';
	//code += '<th style="width:350px;">Group Name</th>';
	code += '<th style="width:60px;"><#CTL_modify#></th>';
	code += '<th style="width:60px;"><#CTL_del#></th>';
	code += '</tr>';

	if(info.device.length == "0"){
		code += "<tr>";
		code += "<td colspan='6' style='color:#FC0;text-align:center;'><#IPConnection_VSList_Norule#></td>";
		code += "</tr>";
	}
	else{
		for(i=0;i<info.device.length;i++){
			var device_index = info.device[i];
			var device_obj = info.device[device_index];
			var device_name = device_obj.name;
			var device_mac = device_obj.mac;
			var device_type = device_obj.type;
			var device_type_name = device_type_array[device_type].name;
			var device_description = device_obj.description;
			code += '<tr>';
			code += '<td><div>' + device_name + '</div></td>';
			code += '<td><div>' + device_mac + '</div></td>';
			code += '<td><div>' + device_type_name + '</div></td>';
			code += '<td><div>' + device_description + '</div></td>';
			//code += '<td><div>' +info.device[info.device[i]].group + '</div></td>';
			code += '<td><div class="modifyAccountBtn_add" onclick=\'show_modifyTable("' + device_obj.mac + '");\'></div></td>';
			code += '<td><div class="deleteAccountBtn_add" onclick=\'delete_device("' + info.device[info.device[i]].mac +'")\'></div></td>';		
			code += '</tr>';
		}
	}

	$("#device_table").html(code);
}

function show_addTable(flag){
	var code = "";
	cal_panel_block("addTable", 0.2);
	$("#addTable").show();
	$("#device_info").show();
	$("#addTable").css({"height":"600px", "margin-top":"-100px"});
	$("#device_mac").prop("disabled", false);
		
	//generate device type option
	for(i=0;i<device_type_array.length;i++){
		if(device_type_array[i].name != "")
			code += '<option value="' + device_type_array[i].number + '">' + device_type_array[i].name + '</option>';
	}
		
	$("#device_type").html(code);

	//generate group table
	code = "";		//reset
	code += '<tr>';
	code += '<th style="width:35px;"><input type="checkbox" onclick="enable_group_all(this);"></th>';
	code += '<th style="width:94%;">Group Name</th>';
	code += '</tr>';
	for(i=0;i<info.group.length;i++){
		var group_index = info.group[i];
		var group_obj = info.group[group_index];
		var checked_state = (group_obj.name == "default") ? "checked" : "";
		code += '<tr>';
		code += '<td style="width:35px;"><input id="' + group_index + '" type="checkbox" '+ checked_state +'></td>';
		code += '<td>' + group_obj.name + '</td>';
		code += '<tr>';
	}

	$("#group_table").html(code);
	$("#add_device_btn").click(function(){
		add_device(flag);
	});	
}

function hide_addTable(){
	$("#addTable").hide();
	$("#device_info").hide();

	//reset value
	$("#device_name").val("");
	$("#device_mac").val("");
	$("#device_type").val("1");
	$("#device_desc").val("");
	
	//unbind button event
	$("#add_device_btn").unbind("click");	
}

function show_modifyTable(target){
	cal_panel_block("addTable", 0.2);
	$("#addTable").show();
	$("#device_info").show();
	$("#addTable").css({"height":"600px", "margin-top":"-100px"});
	$("#device_mac").prop("disabled", true);

	var code = "";
	for(i=0;i<device_type_array.length;i++){
		if(device_type_array[i].name != "")
			code += '<option value="' + device_type_array[i].number + '">' + device_type_array[i].name + '</option>';
	}
		
	$("#device_type").html(code);

	//generate group table
	var device_index = target;
	var device_obj = info.device[device_index];
	var device_name = device_obj.name;
	var device_mac = device_obj.mac;
	var devoce_type = device_obj.type;
	var device_type_name = device_obj.type_name;
	var device_description = device_obj.description;
	var group_array = device_obj.group;

	$("#device_name").val(device_name);
	$("#device_mac").val(device_mac);
	$("#device_type").val(devoce_type);
	$("#device_desc").val(device_description);

	code = "";		//reset
	code += '<tr>';
	code += '<th style="width:35px;"><input type="checkbox" onclick="enable_group_all(this);"></th>';
	code += '<th style="width:94%;">Group Name</th>';
	code += '</tr>';

	for(i=0;i<info.group.length;i++){
		var group_index = info.group[i];
		var group_obj = info.group[group_index];
		var group_name = group_obj.name;
		var checked_state = (group_array.indexOf(group_name) != -1) ? "checked" : "";
		code += '<tr>';
		code += '<td style="width:35px;"><input id="' + group_index + '" type="checkbox" ' + checked_state + '></td>';
		code += '<td>' + group_obj.name + '</td>';
		code += '<tr>';
	}
		
	$("#group_table").html(code);
	$("#add_device_btn").click(function(){
		add_device("modify", target);
	});
}

function add_device(flag, target){
	var device_name = $("#device_name").val();
	var device_mac = $("#device_mac").val();
	var device_type = $("#device_type").val();
	var device_type_name = device_type_array[device_type];
	var device_description = $("#device_desc").val();

	// validate form
	if(!Block_chars(document.getElementById("device_name"), ["<", ">"])){
		return false;
	}
	else if(document.getElementById("device_name").value == ''){
		alert("<#File_Pop_content_alert_desc1#>");
		document.getElementById("device_name").focus();
		return false;
	}

	if(document.getElementById("device_mac").value == ""){
		alert("<#JS_fieldblank#>");
		document.getElementById("device_mac").focus();
		return false;
	}

	if(!check_macaddr(document.getElementById("device_mac"), check_hwaddr_flag(document.getElementById("device_mac")))){
		document.getElementById("device_mac").focus();
		document.getElementById("device_mac").select();
		return false;	
	}

	if(flag != "modify"){
		for(i=0;i<info.device.length;i++){
			var device_index = info.device[i];
			if(document.getElementById("device_mac").value == info.device[device_index].mac){
				alert("This device already exists.");
				document.getElementById("device_mac").focus();
				return false;
			}
		}
	}

	if(!Block_chars(document.getElementById("device_desc"), ["<", ">"])){
		return false;
	}

	var checked_count = 0;
	for(i=0;i<info.group.length;i++){		//find the group name of the new device
		var group_index = info.group[i];
		var group_obj = info.group[group_index];
		var checked_state = $("#" + group_index)[0].checked;
		if(checked_state){
			checked_count++;
		}
	}

	if(checked_count == 0){
		alert("Must assign at least one group!");
		return false;
	}

	// end validate form


	document.form.action = "apply.cgi";
	document.form.action_mode.value = "pms_apply";
	document.form.pms_action.value = "device_update";
	document.form.pms_dev_active.value = "1";
	document.form.pms_dev_mac.value = device_mac;
	document.form.pms_dev_devname.value = device_name;
	document.form.pms_dev_devtype.value = device_type;
	document.form.pms_dev_desc.value = device_description;

	// to generate pms_devgroup format
	var group_array = new Array();
	for(i=0;i<info.group.length;i++){		//find the group name of the new device
		var group_index = info.group[i];
		var group_obj = info.group[group_index];
		var checked_state = $("#" + group_index)[0].checked;
		if(checked_state){
			group_array.push(group_obj.name);
		}
	}

	var code = "";
	for(i=0;i<group_array.length;i++){
		if(i != 0)
			code += ">";

		code += group_array[i];
	}

	var group_length = group_array.length;;
	if(group_length == 0)
		group_length = 1;

	// end pms_devgroup format

	document.form.pms_devgroup.value = code;
	document.form.pms_devgroup_num.value = group_length;
	document.form.submit();

	setTimeout("update_list();", 500);
	hide_addTable();
}

function delete_device(target){
	document.form.action = "apply.cgi";
	document.form.action_mode.value = "pms_apply";
	document.form.pms_action.value = "device_delete";
	document.form.pms_dev_mac.value = target;

	document.form.submit();
	setTimeout("update_list();", 500);
}

function applyRule(){
	showLoading(5);
	document.form.action_script.value = "restart_pms_device;";
	document.form.submit();
}

function check_macaddr(obj,flag){ //control hint of input mac address
	if(flag == 1){
		alert("<#LANHostConfig_ManualDHCPMacaddr_itemdesc#>");
		return false;
	}else if(flag ==2){
		alert("<#IPConnection_x_illegal_mac#>");
		return false;		
	}else{
		return true;
	}	
}


function pullLANIPList(obj){	
	var element = document.getElementById('ClientList_Block_PC');
	var isMenuopen = element.offsetWidth > 0 || element.offsetHeight > 0;
	if(isMenuopen == 0){		
		obj.src = "/images/arrow-top.gif"
		element.style.display = 'block';		
		document.getElementById("device_mac").focus();		
	}
	else
		hideClients_Block();
}

function hideClients_Block(){
	document.getElementById("pull_arrow").src = "/images/arrow-down.gif";
	document.getElementById('ClientList_Block_PC').style.display='none';
}

function setClientIP(macaddr){
	document.getElementById("device_mac").value = macaddr;
	var target_obj = clientList[macaddr];
	document.getElementById("device_name").value = target_obj.name;
	document.getElementById("device_type").value = target_obj.type;

	hideClients_Block();
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
    	generate_device_table();
    }
  });	
}

function enable_group_all(obj){
	var check_state = obj.checked;
	for(i=0;i<info.group.length;i++){
		var index = info.group[i];
		$("#"+index).prop("checked", check_state);
	}
}
</script>
</head>

<body>
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="PermissionManagement_Devices.asp">
<input type="hidden" name="next_page" value="PermissionManagement_Devices.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_wait" value="3">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">

<input type="hidden" name="pms_action" value="">
<input type="hidden" name="pms_dev_active" value="">
<input type="hidden" name="pms_dev_mac" value="">
<input type="hidden" name="pms_dev_devname" value="">
<input type="hidden" name="pms_dev_devtype" value="">
<input type="hidden" name="pms_dev_desc" value="">
<input type="hidden" name="pms_devgroup" value="">
<input type="hidden" name="pms_devgroup_num" value="">
</form>

<div id="addTable" class="contentM_qis" style="height:670px;">
	<div id="device_info" style="display:none">
		<table width="97%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" style="margin: 20px 10px;">
			<thead>
				<tr>
					<td colspan="6">Create a new device</td>
				</tr>
			</thead>		  
			<tr>
				<th width="30%" style="font-family: Calibri;font-weight: bolder;"><#ShareNode_DeviceName_itemname#></th>			
				<td>
					<input id="device_name" type="text" maxlength="32"class="input_32_table" style="height: 23px;" value="" autocorrect="off" autocapitalize="off">
					<img id="pull_arrow" height="14px;" src="/images/arrow-down.gif" style="position:absolute;" onclick="pullLANIPList(this);" title="<#select_client#>">
					<div id="ClientList_Block_PC" style="margin:0 0 0 2px" class="clientlist_dropdown"></div>					
				</td>
			</tr>
			<tr>
				<th width="30%" style="font-family: Calibri;font-weight: bolder;"><#MAC_Address#></th>			
				<td>
					<input id="device_mac" type="text" maxlength="17"class="input_32_table" style="height: 23px;" value="" autocorrect="off" autocapitalize="off" onKeyPress="return validator.isHWAddr(this,event)">
				</td>
			</tr>	

			<tr>
				<th width="30%" style="font-family: Calibri;font-weight: bolder;"><#Device_type#></th>			
				<td>
					<select id="device_type" class="input_option"></select>					
				</td>
			</tr>
			<tr>
				<th width="30%" style="font-family: Calibri;font-weight: bolder;"><#Description#></th>			
				<td>
					<input id="device_desc" type="text" maxlength="32"class="input_32_table" style="height: 23px;" value="" autocorrect="off" autocapitalize="off">
				</td>
			</tr>			
		</table>
		<div style="display:flex;margin: 0 20px;">
			<div style="width:100%;">
				<div>Please assign the device to at least one group.</div>
				<div id="current_group" style="padding: 10px 0 0 20px;color:#FC0"></div>
			</div>
		</div>		
		<div style="height:220px;overflow:auto;">
			<table id="group_table" width="97%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" style="margin: 20px 10px;"></table>
		</div>				
		<div class="apply_gen" style="margin-top:20px;margin-bottom:10px;background-color: #2B373B;">
			<input name="button" type="button" class="button_gen" onclick="hide_addTable();" value="<#CTL_Cancel#>"/>
			<input id="add_device_btn" name="button" type="button" class="button_gen" onclick="" value="<#CTL_apply#>"/>
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
									<div class="formfonttitle"><#Permission_Management#> - <#Permission_Management_Devices#></div>
									<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
									<div style="margin-top:20px;" class="formfontdesc"><#PM_Devices_desc#></div>								
									<div>
										<div style="display:flex">
											<div style="font-weight:900;padding-left:10px;line-height:34px;">Device Table</div>
											<div style="margin-left:10px;">
												<div class="createAccountBtn_add" onclick="show_addTable('new');"></div>
											</div>										
										</div>									
										<table id="device_table" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable"></table>
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
