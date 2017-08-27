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
<title><#Permission_Management#> - <#Permission_Management_Users#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/disk_functions.js"></script>
<script type="text/javascript" src="/form.js"></script>
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
info.group = new Array();
info.account = new Array();

/* To get account & account of group information*/
var account_list = <% pms_account_info(); %>;
var group_list = <% pms_accgroup_info(); %>;
/*var group_list = [ { "active": 1, "name": "Administrator", "desc": "", "owned_account": [ "admin" ], "owned_account_num": 1 }, 
				   { "active": 0, "name": "FrontDesk", "desc": "", "owned_account": [ "admin" ], "owned_account_num": 1 },
				   { "active": 1, "name": "default", "desc": "", "owned_account": [ ], "owned_account_num": 0 } ];*/

/*var account_list = [ { "active": 1, "name": "admin", "passwd": "11111", "email": "", "desc": "", "owned_group": [ "Administrator", "FrontDesk" ], "owned_group_num": 2 },
					 { "active": 1, "name": "guest", "passwd": "guest", "email": "", "desc": "", "owned_group": [ "FrontDesk" ], "owned_group_num": 1 } ];*/
$(document).ready(function (){
	show_menu();
	collect_info();
	generate_account_table();
});

function group_object(active, name, org_name, desc, account_array){
	this.active = (active == 1) ? true : false;
	this.name = name;
	this.org_name = org_name;
	this.description = desc;
	this.members = account_array;
}

function account_object(active, name, password, description, group_array){
	this.active = (active == 1) ? true : false;
	this.name = name;
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
		var group_org_name = object.name;
		var group_desc = object.desc;
		var account_array = new Array();
		account_array = object.owned_account;

		info.group.push(group_index);
		info.group[group_index] = new group_object(group_active, group_name, group_org_name, group_desc, account_array);
	}
	
	//colletc account info
	for(i=0;i<account_list.length;i++){
		var object = account_list[i];
		var account_index = "_" + decodeURIComponent(object.name);
		var account_active = object.active;
		var account_name = decodeURIComponent(object.name);
		var account_password = object.passwd;
		var account_desc = object.desc;
		var group_array = new Array();
		group_array = object.owned_group;

		info.account.push(account_index);
		info.account[account_index] = new account_object(account_active, account_name, account_password, account_desc, group_array);
	}
}

function generate_account_table(){
	var code = "";
	code += '<tr>';
	//code += '<th style="width:35px;"><input id="account_all" type="checkbox" onclick="enable_account(this);"></th>';
	code += '<th style="width:350px;"><#Username#></th>';
	code += '<th style="width:350px;"><#Description#></th>';
	code += '<th style="width:60px;"><#CTL_modify#></th>';
	code += '<th style="width:60px;"><#CTL_del#></th>';
	code += '</tr>';
	
	for(i=0;i<info.account.length;i++){
		var account_index = info.account[i];
		var account_obj = info.account[account_index];
		var checked_state = account_obj.active ? "checked" : "";
		code += '<tr>';			
		//code += '<td><div><input id="' + account_index + '" type="checkbox" onclick="enable_account(this);" value="" ' + checked_state + '></div></td>';
		code += '<td><div>' + account_obj.name + '</div></td>';
		code += '<td><div>' + account_obj.description + '</div></td>';
		code += '<td><div class="modifyAccountBtn_add" onclick=\'show_modifyTable("account", "' + account_obj.name + '");\'></div></td>';
		code += '<td>';
		if((account_obj.name != "admin") && (account_obj.name != "guest"))
			code += '<div class="deleteAccountBtn_add" onclick=\'delete_account("' + account_obj.name + '");\'></div>';
		
		code += '</td>';
		code += '</tr>';
	}

	$("#account_table").html(code);
}

function show_addTable(type, flag){
	cal_panel_block("addTable", 0.2);
	var code = "";
	$("#addTable").show();
	$("#addTable").css({"height":"600px", "margin-top":"-100px"});
		
	//generate group table
	code += '<tr>';
	code += '<th style="width:35px;"><input type="checkbox" onclick="enable_group_all(this)" value=""></th>';
	code += '<th style="width:94%;">Group Name</th>';
	code += '</tr>';
	for(i=0;i<info.group.length;i++){
		var group_index = info.group[i];
		var group_obj = info.group[group_index];
		var checked_state = (group_obj.name == "default") ? "checked" : "";
		code += '<tr>';
		code += '<td style="width:35px;"><input id="' + group_index + '" type="checkbox" onclick="" '+ checked_state +'></td>';
		code += '<td>' + group_obj.name + '</td>';
		code += '<tr>';
	}
		
	$("#group_table").html(code);
	$("#add_account_btn").click(function(){
		add_account(flag);
	});		
}

function add_account(flag, target){
	var account_active = $("#account_active").val();
	var account_name = $("#account_name").val();
	var account_password = $("#account_password").val();
	var account_desc = $("#account_description").val();

	// validate form
	if(!Block_chars(document.getElementById("account_name"), ["<", ">"])){
		return false;
	}
	else if(document.getElementById("account_name").value == ''){
		alert("<#File_Pop_content_alert_desc1#>");
		document.getElementById("account_name").focus();
		return false;
	}

	var alert_str = validator.hostName(document.getElementById("account_name"));

	if(alert_str != ""){
		alert(alert_str);
		document.getElementById("account_name").focus();
		return false;
	}
	
	if(flag != "modify"){
		for(i=0;i<info.account.length;i++){
			var account_index = info.account[i];
			if(document.getElementById("account_name").value == info.account[account_index].name){
				alert("<#File_Pop_content_alert_desc5#>");
				document.getElementById("account_name").focus();
				return false;
			}
		}
	}

	if(!Block_chars(document.getElementById("account_password"), ["<", ">"])){
		return false;
	}
	else if(document.getElementById("account_password").value == ''){
		alert("<#File_Pop_content_alert_desc6#>");
		document.getElementById("account_password").focus();
		return false;
	}
	else if(document.getElementById("account_password").value != document.getElementById("account_password_confirm").value){
		alert("<#File_Pop_content_alert_desc7#>");
		document.getElementById("account_password").focus();
		return false;
	}

	if(!Block_chars(document.getElementById("account_description"), ["<", ">"]))
		return false;

	var checked_count = 0;
	for(i=0;i<info.group.length;i++){		//find the group name of the new account
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

	//end validate form
	document.form.action = "apply.cgi";
	document.form.action_mode.value = "pms_apply";
	if(flag != "modify")
		document.form.pms_action.value = "account_create";
	else
		document.form.pms_action.value = "account_modify";
	document.form.pms_acc_active.value = account_active;
	document.form.pms_acc_name.value = decodeURIComponent(account_name);
	document.form.pms_acc_passwd.value = decodeURIComponent(account_password);
	document.form.pms_acc_email.value = "";
	document.form.pms_acc_desc.value = account_desc;

		var group_array = new Array();

		for(i=0;i<info.group.length;i++){		//find the group name of the new account
			var group_index = info.group[i];
			var group_obj = info.group[group_index];
			var checked_state = $("#" + group_index)[0].checked;
			if(checked_state){
				group_array.push(group_obj.org_name);
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

	document.form.pms_accgroup.value = code;
	document.form.pms_accgroup_num.value = group_length;
	document.form.submit();
	setTimeout("update_list();", 500);
	
	hide_addTable();	
}

function hide_addTable(){
	$("#addTable").hide();
	$("#account_active").val("1");
	$("#account_name").val("");
	$("#account_password").val("")
	$("#account_password_confirm").val("")
	$("#account_description").val("")
	$("#add_account_btn").unbind("click");
	$("#account_name").prop("disabled", false);
}

function show_modifyTable(type, target){
	cal_panel_block("addTable", 0.2);
	$("#addTable").show();
	var code = "";	
	var account_index = "_" + target;
	var account_obj = info.account[account_index];
	var account_active = account_obj.active;
	var account_name = account_obj.name;
	var account_password = account_obj.password;
	var account_desc = account_obj.description;
	$("account_active").val(account_active ? 1 : 0);
	$("#account_name").val(account_name);
	document.form.org_account.value = account_name;
	$("#account_password").val(account_password);
	$("#account_password_confirm").val(account_password);
	$("#account_description").val(account_desc);
		
	code += '<tr>';
	code += '<th style="width:35px;"><input type="checkbox" onclick="enable_group_all(this)" value=""></th>';
	code += '<th style="width:94%;">Group Name</th>';
	code += '</tr>';
	var group_array = account_obj.group;
	for(i=0;i<info.group.length;i++){
		var group_index = info.group[i];
		var group_obj = info.group[group_index];
		var group_name = group_obj.name;
		var group_org_name = group_obj.org_name;
		var checked_state = (group_array.indexOf(group_org_name) != -1) ? "checked" : "";

		code += '<tr>';
		code += '<td style="width:35px;"><input id="' + group_index + '" type="checkbox" onclick="" ' + checked_state + '></td>';
		code += '<td>' + group_name + '</td>';
		code += '<tr>';
	}

	$("#group_table").html(code);		
	if(target == "admin"){
		$("#account_name").prop("disabled", true);
	}
	
	$("#add_account_btn").click(function(){
		add_account("modify", target);
	});
}

function delete_account(target){
	document.form.action = "apply.cgi";
	document.form.action_mode.value = "pms_apply";
	document.form.pms_action.value = "account_delete";
	document.form.pms_acc_name.value = target;
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
    	generate_account_table();
    }
  });	
}

function enable_account(obj){
	var id = obj.id;
	var check_state = obj.checked;
	if(id == "account_all"){
		for(i=0;i<info.account.length;i++){
			var index = info.account[i];
			$("#"+index).prop("checked", check_state);
			info.account[index].active = (check_state == "checked") ? true : false;
		}
	}
	else{
		info.account[id].active = (check_state == "checked") ? true : false;
	}
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
<input type="hidden" name="current_page" value="PermissionManagement_Users.asp">
<input type="hidden" name="next_page" value="PermissionManagement_Users.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_wait" value="3">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="pms_action" value="">
<input type="hidden" name="pms_acc_active" value="">
<input type="hidden" name="pms_acc_name" value="">
<input type="hidden" name="org_account" value="">
<input type="hidden" name="pms_acc_passwd" value="">
<input type="hidden" name="pms_acc_email" value="">
<input type="hidden" name="pms_acc_desc" value="">
<input type="hidden" name="pms_accgroup" value="">
<input type="hidden" name="pms_accgroup_num" value="">
</form>

<div id="addTable" class="contentM_qis" style="height:670px;">
	<div>
		<table width="97%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" style="margin: 20px 10px;">
			<thead>
				<tr>
					<td colspan="6">Create a new user</td>
				</tr>
			</thead>
			<tr>
				<th width="30%" style="font-family: Calibri;font-weight: bolder;"><#Status_Active#></th>
				<td>
					<select id="account_active" class="input_option" disabled>
						<option value="1" selected><#WLANConfig11b_WirelessCtrl_button1name#></option>
						<option value="0"><#WLANConfig11b_WirelessCtrl_buttonname#></option>
					</select>					
				</td>
			</tr>		  
			<tr>
				<th width="30%" style="font-family: Calibri;font-weight: bolder;"><#Username#></th>			
				<td>
					<input id="account_name" type="text" maxlength="32" class="input_32_table" style="height: 23px;" value="" autocorrect="off" autocapitalize="off">
				</td>
			</tr>	
			<tr>
				<th width="30%" style="font-family: Calibri;font-weight: bolder;"><#HSDPAConfig_Password_itemname#></th>			
				<td>
					<input id="account_password" type="password" maxlength="32"class="input_32_table" style="height: 23px;" value="" autocorrect="off" autocapitalize="off">
				</td>
			</tr>
			<tr>
				<th width="30%" style="font-family: Calibri;font-weight: bolder;">Confirm Password</th>			
				<td>
					<input id="account_password_confirm" type="password" maxlength="32"class="input_32_table" style="height: 23px;" value="" autocorrect="off" autocapitalize="off">
				</td>
			</tr>
			<tr>
				<th width="30%" style="font-family: Calibri;font-weight: bolder;"><#Description#></th>			
				<td>
					<input id="account_description" type="text" maxlength="32"class="input_32_table" style="height: 23px;" value="" autocorrect="off" autocapitalize="off">
				</td>
			</tr>			
		</table>
		<div style="display:flex;margin: 0 20px;">
			<div style="width:100%;">
				<div>Please assign the user to at least one group.</div>
				<div id="current_group" style="padding: 10px 0 0 20px;color:#FC0"></div>
			</div>
		</div>
		<div style="height:220px;overflow:auto;">
			<table id="group_table" width="97%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" style="margin: 20px 10px;"></table>
		</div>		
		<div class="apply_gen" style="margin-top:20px;margin-bottom:10px;background-color: #2B373B;">
			<input name="button" type="button" class="button_gen" onclick="hide_addTable();" value="<#CTL_Cancel#>"/>
			<input id="add_account_btn" name="button" type="button" class="button_gen" onclick="" value="<#CTL_apply#>"/>
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
									<div class="formfonttitle"><#Permission_Management#> - <#Permission_Management_Users#></div>
									<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
									<div style="margin-top:20px ;" class="formfontdesc"><#PM_Users_desc#></div>
									<div>
										<div style="display:flex">
											<div style="font-weight:900;padding-left:10px;line-height:34px;"><#PM_Account_Table#></div>
											<div style="margin-left:10px;">
												<div class="createAccountBtn_add" onclick="show_addTable('account', 'new');"></div>
											</div>																			
										</div>									
										<table id="account_table" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable"></table>
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
