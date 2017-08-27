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
<title><#Permission_Management#> - <#Permission_Management_Groups#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/disk_functions.js"></script>
<script type="text/javascript" src="/form.js"></script>
<script type="text/javascript" language="JavaScript" src="/validator.js"></script>
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
info.group = new Array();
info.account = new Array();

/* To get account & account of group information*/
var account_list = <% pms_account_info(); %>;
var group_list = <% pms_accgroup_info(); %>;

$(document).ready(function (){
	show_menu();
	collect_info();
	generate_group_table();
});

function group_object(active, name, desc, account_array){
	this.active = (active == 1) ? true : false;
	this.name = name;
	this.description = desc;
	this.members = account_array;
}

function account_object(active, name, org_name, password, description, group_array){
	this.active = (active == 1) ? true : false;
	this.name = name;
	this.org_name = org_name;
	this.password = password;
	this.description = description;
	this.group = group_array;
}

function collect_info(){
	info.group = [];
	info.account = [];

	//collect group info
	for(i=0;i<group_list.length;i++){
		var object = group_list[i];
		var group_index = "_" + decodeURIComponent(object.name);
		var group_active  = object.active;
		var group_name = decodeURIComponent(object.name);
		var group_desc = object.desc;
		var account_array = new Array();
		account_array = object.owned_account;

		info.group.push(group_index);
		info.group[group_index] = new group_object(group_active, group_name, group_desc, account_array);
	}
	
	//colletc account info
	for(i=0;i<account_list.length;i++){
		var object = account_list[i];
		var account_index = "_" + decodeURIComponent(object.name);
		var account_active = object.active;
		var account_name = decodeURIComponent(object.name);
		var account_org_name = object.name;
		var account_password = object.passwd;
		var account_desc = object.desc;
		var group_array = new Array();
		group_array = object.owned_group;

		info.account.push(account_index);
		info.account[account_index] = new account_object(account_active, account_name, account_org_name, account_password, account_desc, group_array);
	}
}

function generate_group_table(){
	var code = "";
	code += '<tr>';
	//code += '<th style="width:35px;"><input id="" type="checkbox" onclick="" value=""></th>';
	code += '<th style="width:350px;">Group Name</th>';
	code += '<th style="width:350px;"><#Description#></th>';
	code += '<th style="width:60px;"><#CTL_modify#></th>';
	code += '<th style="width:60px;"><#CTL_del#></th>';
	code += '</tr>';

	for(i=0;i<info.group.length;i++){
		var group_index = info.group[i];
		var group_obj = info.group[group_index];
		var checked_state = group_obj.active ? "checked" : "";
		code += '<tr>';			
		//code += '<td><div><input id="' + group_index + '" type="checkbox" onclick="" value="" ' + checked_state + '></div></td>';
		code += '<td><div>' + group_obj.name + '</div></td>';
		code += '<td><div>' + group_obj.description + '</div></td>';
		code += '<td><div class="modifyAccountBtn_add" onclick=\'show_modifyTable("' + group_obj.name + '");\'></div></td>';
		code += '<td>';
		if((group_obj.name != "Administrator") && (group_obj.name != "FrontDesk") && (group_obj.name != "default"))
			code += '<div class="deleteAccountBtn_add" onclick=\'delete_group("' + group_obj.name + '");\'></div>';
	
		code += '</td>';
		code += '</tr>';
	}

	$("#group_table").html(code);
}

function show_addTable(type, flag){
	var code = "";
	cal_panel_block("addTable", 0.2);
	$("#addTable").show();
	$("#addTable").css({"height":"600px", "margin-top":"-100px"});
		
	//generate group table
	code += '<tr>';
	code += '<th style="width:35px;"><input type="checkbox" onclick="enable_account_all(this);" value=""></th>';
	code += '<th style="width:94%;"><#Username#></th>';
	code += '</tr>';
	for(i=0;i<info.account.length;i++){
		var account_index = info.account[i];
		var account_obj = info.account[account_index];
		code += '<tr>';
		code += '<td style="width:35px;"><input id="' + account_index + '" type="checkbox" onclick="" value=""></td>';
		code += '<td>' + account_obj.name + '</td>';
		code += '<tr>';
	}
		
	$("#account_table").html(code);
	$("#add_group_btn").click(function(){
		add_group(flag);
	});		
}

function add_group(flag, target){
	var group_active = $("#group_active").val();
	var group_name = $("#group_name").val();
	var group_desc = $("#group_description").val();
	
	// validate form
	if(!Block_chars(document.getElementById("group_name"), ["<", ">"])){
		return false;
	}
	else if(document.getElementById("group_name").value == ''){
		alert("<#File_Pop_content_alert_desc1#>");
		document.getElementById("group_name").focus();
		return false;
	}

	var alert_str = validator.hostName(document.getElementById("group_name"));

	if(alert_str != ""){
		alert(alert_str);
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

	if(!Block_chars(document.getElementById("group_description"), ["<", ">"]))
		return false;

	document.form.action = "apply.cgi";
	document.form.action_mode.value = "pms_apply";
	if(flag != "modify")
		document.form.pms_action.value = "accgroup_create";
	else
		document.form.pms_action.value = "accgroup_modify";
	document.form.pms_accgrp_active.value = group_active;
	document.form.pms_accgrp_name.value = decodeURIComponent(group_name);
	document.form.pms_accgrp_desc.value = group_desc;

	var account_array = new Array();
	for(i=0;i<info.account.length;i++){		//find the account name of the new group
		var account_index = info.account[i];
		var account_obj = info.account[account_index];
		var checked_state = $("#" + account_index)[0].checked;
		if(checked_state){
			account_array.push(account_obj.org_name);
		}	
	}	

	var code = "";
	for(i=0;i<account_array.length;i++){
	
		if(i != 0)
			code += ">";

		code += account_array[i];
	}

	var account_length = account_array.length;
	if(account_length == 0)
		account_length = 1;

	document.form.pms_grpacc.value = code;
	document.form.pms_grpacc_num.value = account_length;
	document.form.submit();
	setTimeout("update_list();", 500);

	hide_addTable();	
}

function hide_addTable(){
	$("#addTable").hide();
	$("#group_active").val("1");
	$("#group_name").val("");
	$("#group_description").val("")
	$("#add_group_btn").unbind("click");
	$("#group_name").prop("disabled", false);
}

function show_modifyTable(target){
	cal_panel_block("addTable", 0.2);
	$("#addTable").show();
	var code = "";	
	var group_index = "_" + target;
	var group_obj = info.group[group_index];
	var group_active = group_obj.active;
	var group_name = group_obj.name;
	var group_desc = group_obj.description;
	$("#group_active").val(group_active ? 1 : 0);
	$("#group_name").val(group_name);
	document.form.org_accgrp_name.value = group_name;
	$("#group_description").val(group_desc);

	code += '<tr>';
	code += '<th style="width:35px;"><input type="checkbox" onclick="" value=""></th>';
	code += '<th style="width:94%;">Assign users to this group</th>';
	code += '</tr>';

	var account_array = group_obj.members;
	for(i=0;i<info.account.length;i++){
		var account_index = info.account[i];
		var account_obj = info.account[account_index];
		var account_name = account_obj.name;
		var account_org_name = account_obj.org_name;
		var checked_state = (account_array.indexOf(account_org_name) != -1) ? "checked" : "";

		code += '<tr>';
		code += '<td style="width:35px;"><input id="' + account_index + '" type="checkbox" onclick="" ' + checked_state + '></td>';
		code += '<td>' + account_name + '</td>';
		code += '<tr>';
	}

	$("#account_table").html(code);		
	if((target == "Administrator") || (target == "FrontDesk")){
		$("#group_name").prop("disabled", true);
	}
	
	$("#add_group_btn").click(function(){
		add_group("modify", target);
	});
}

function delete_group(target){
	document.form.action = "apply.cgi";
	document.form.action_mode.value = "pms_apply";
	document.form.pms_action.value = "accgroup_delete";
	document.form.pms_accgrp_name.value = target;

	/*var group_index = "_" + target;
	var group_sequence = info.group.indexOf(group_index);
	info.group.splice(group_sequence, 1);
	generate_group_table();*/

	document.form.submit();
	setTimeout("update_list();", 500);
}

function applyRule(){
	showLoading(5);
	document.form.action = "start_apply.htm";
	document.form.action_script.value = "restart_pms_account;restart_radiusd";
	document.form.action_mode.value = "apply";
	document.form.submit();
}

function update_list(){
  $.ajax({
    url: '/getAccountList.asp',
    dataType: 'script',	
    error: function(xhr) {
		//setTimeout("update_list();", 10000);
    },
    success: function(response){
    	account_list = account_list_hook;
    	group_list = group_list_hook;
    	collect_info();
    	generate_group_table();
    }
  });	
}

function enable_account_all(obj){
	var check_state = obj.checked;
	for(i=0;i<info.account.length;i++){
		var index = info.account[i];
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
<input type="hidden" name="current_page" value="PermissionManagement_UserGroups.asp">
<input type="hidden" name="next_page" value="PermissionManagement_UserGroups.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_wait" value="3">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">

<input type="hidden" name="pms_action" value="">
<input type="hidden" name="pms_accgrp_active" value="">
<input type="hidden" name="pms_accgrp_name" value="">
<input type="hidden" name="org_accgrp_name" value="">
<input type="hidden" name="pms_accgrp_desc" value="">
<input type="hidden" name="pms_grpacc" value="">
<input type="hidden" name="pms_grpacc_num" value="">

</form>

<div id="addTable" class="contentM_qis" style="height:670px;">
	<div>
		<table width="97%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" style="margin: 20px 10px;">
			<thead>
				<tr>
					<td colspan="6">Create a new group</td>
				</tr>
			</thead>
			<tr>
				<th width="30%" style="font-family: Calibri;font-weight: bolder;"><#Status_Active#></th>			
				<td>
					<select id="group_active" class="input_option" disabled>
						<option value="1" selected><#WLANConfig11b_WirelessCtrl_button1name#></option>
						<option value="0"><#WLANConfig11b_WirelessCtrl_buttonname#></option>
					</select>					
				</td>
			</tr>		  
			<tr>
				<th width="30%" style="font-family: Calibri;font-weight: bolder;">Group Name</th>			
				<td>
					<input id="group_name" type="text" maxlength="32" class="input_32_table" style="height: 23px;" value="" autocorrect="off" autocapitalize="off">
				</td>
			</tr>	
			<!--tr>
				<th width="30%" style="font-family: Calibri;font-weight: bolder;"><#HSDPAConfig_Password_itemname#></th>			
				<td>
					<input id="account_password" type="text" maxlength="32"class="input_32_table" style="height: 23px;" value="" autocorrect="off" autocapitalize="off">
				</td>
			</tr>
			<tr>
				<th width="30%" style="font-family: Calibri;font-weight: bolder;">Confirm Password</th>			
				<td>
					<input id="account_password_confirm" type="text" maxlength="32"class="input_32_table" style="height: 23px;" value="" autocorrect="off" autocapitalize="off">
				</td>
			</tr-->
			<tr>
				<th width="30%" style="font-family: Calibri;font-weight: bolder;"><#Description#></th>			
				<td>
					<input id="group_description" type="text" maxlength="32"class="input_32_table" style="height: 23px;" value="" autocorrect="off" autocapitalize="off">
				</td>
			</tr>			
		</table>
		<div style="display:flex;margin: 0 20px;">
			<div style="width:100%;">
				<div>Assign users to this group</div>
				<div id="current_account" style="padding: 10px 0 0 20px;color:#FC0"></div>
			</div>
		</div>
		<div style="height:325px;overflow:auto;">
			<table id="account_table" width="97%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" style="margin: 20px 10px;"></table>
		</div>		
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
									<div class="formfonttitle"><#Permission_Management#> - <#Permission_Management_Groups#></div>
									<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
									<div style="margin-top:20px ;" class="formfontdesc"><#PM_UsersGroups_desc#></div>
									<div>
										<div style="display:flex">
											<div style="font-weight:900;padding-left:10px;line-height:34px;"><#PM_Account_Table#></div>
											<div style="margin-left:10px;">
												<div class="createAccountBtn_add" onclick="show_addTable('account', 'new');"></div>
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
