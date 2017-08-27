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
<title><#Permission_Management_RADIUS#></title>
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
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
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
var info = {
	enable: false,
	clients: []
}
//var radius_client = "1>Client_1>123456>192.168.1.100<1>Client_2>1234567>192.168.1.200";
var radius_client =  decodeURIComponent('<% nvram_char_to_ascii("","radius_serv_list"); %>').replace(/&#62/g, ">");

function client_object(active, name, password, ipaddr){
	this.active = (active == 1) ? true : false;
	this.name = name;
	this.password = password;
	this.ipaddr = ipaddr;
}

$(document).ready(function (){
	show_menu();
	collect_info();
	generate_client_table();
});

function collect_info(){
	if(radius_client == "")
		return true;

	var radius_client_row = radius_client.split("<");
	for(i=0;i<radius_client_row.length;i++){
		var radius_client_col = radius_client_row[i].split(">");
		var client_active = radius_client_col[0]; 
		var client_name = radius_client_col[1];
		var client_password = radius_client_col[2];
		var client_ip_addr = radius_client_col[3];

		info.clients.push(client_ip_addr);	
		info.clients[client_ip_addr] = new client_object(client_active, client_name, client_password, client_ip_addr);
	}
}

function generate_client_table(){
	var code = "";
	code += '<tr>';
	code += '<th style="width:60px;"><div>Active</div></th>';
	code += '<th style="width:50%;">Name</th>';
	code += '<th style="width:150px">IP</th>';
	code += '<th style="width:60px;"><#CTL_modify#></th>';
	code += '<th style="width:60px;"><#CTL_del#></th>';
	code += '</tr>';
	
	if(info.clients.length == "0"){			// Empty list
		code += "<tr>";
		code += "<td colspan='5' style='color:#FC0;text-align:center;'><#IPConnection_VSList_Norule#></td>";
		code += "</tr>";
	}
	else{
		for(i=0;i<info.clients.length;i++){
			code += '<tr>';
			if(info.clients[info.clients[i]].active == "1")
				code += '<td><input id='+ info.clients[i] +' onclick="check_active(this)" type="checkbox" checked/></td>';
			else
				code += '<td><input id='+ info.clients[i] +' onclick="check_active(this)" type="checkbox" /></td>';
			
			code += '<td>' + info.clients[info.clients[i]].name + '</td>';
			code += '<td>' + info.clients[info.clients[i]].ipaddr + '</td>';
			code += '<td><div class="modifyAccountBtn_add" style="" onclick="show_addTable(\'modify\', \''+ info.clients[i] +'\')"></div></td>';
			code += '<td><div class="deleteAccountBtn_add" style="" onclick="delete_client(\'' + info.clients[i] + '\')"></div></td>';
			code += '</tr>';
		}		
	}

	$("#client_table").html(code);
}

function show_addTable(flag, target){
	cal_panel_block("addTable", 0.2);
	$("#addTable").show();
	if(flag == "modify"){
		var object = info.clients[target];
		$("#radius_client_active").val((object.active? 1 : 0));
		$("#name").val(object.name);
		$("#password").val(object.password);
		$("#confirm_password").val(object.password);
		$("#ip_address").val(object.ipaddr);
		
		$("#add_btn").click(function(){
			add_client("modify", target);
		});
	}
	else{
		$("#add_btn").click(function(){
			add_client("new", target);
		});
	}
}

function hide_addTable(){
	$("#addTable").hide();
	
	$("#add_btn").unbind("click");
	$("#radius_client_active").val("1");
	$("#name").val("");
	$("#password").val("");
	$("#confirm_password").val("");
	$("#ip_address").val("");
}

function add_client(flag, target){
	var active = ($("#radius_client_active").val() == 1 ? true : false);
	var name = $("#name").val();
	var password = $("#password").val();
	var ipaddr = $("#ip_address").val();

	// validate form
	if(!Block_chars(document.getElementById("name"), ["<", ">"])){
		return false;
	}
	else if(document.getElementById("name").value == ''){
		alert("<#File_Pop_content_alert_desc1#>");
		document.getElementById("name").focus();
		return false;
	}


	if(!validator.isLegalIP($("#ip_address")[0], "")){
		return false;		
	}
	
	var index = info.clients.indexOf(target);
	for(i=0;i<info.clients.length;i++){
		var client_index = info.clients[i];
		if(name == info.clients[client_index].name && index != i){
			alert("<#File_Pop_content_alert_desc5#>");
			document.getElementById("name").focus();
			return false;
		}
		else if(ipaddr == info.clients[client_index].ipaddr && index != i){
			alert("This IP address already exists.\nPlease enter a different one.");
			document.getElementById("ip_address").focus();
			return false;
		}
	}

	if(!Block_chars(document.getElementById("password"), ["<", ">"])){
		return false;
	}
	else if(document.getElementById("password").value == ''){
		alert("<#File_Pop_content_alert_desc6#>");
		document.getElementById("password").focus();
		return false;
	}
	else if(document.getElementById("password").value != document.getElementById("confirm_password").value){
		alert("<#File_Pop_content_alert_desc7#>");
		document.getElementById("password").focus();
		return false;
	}

	if(info.clients.length >= 16){
		alert("<#JS_itemlimit1#> " + "16" + " <#JS_itemlimit2#>");
		return false;
	}

	//end validate form
		
	if(flag == "modify"){
		if(info.clients[target].ipaddr != $("#ip_address").val()){
			var index = info.clients.indexOf(target);		//find target index in the array
			info.clients.splice(index, 1);					//deltet origin element in the array
			info.clients.push(ipaddr);
			info.clients[ipaddr] = new client_object(active, name, password, ipaddr);
		}
		else{
			info.clients[target].active = active;
			info.clients[target].name = name;
			info.clients[target].password = password;
			info.clients[target].ipaddr = ipaddr;				
		}
	}
	else{
		info.clients.push(ipaddr);
		info.clients[ipaddr] = new client_object(active, name, password, ipaddr);
	}
	
	hide_addTable();
	generate_client_table();
}

function delete_client(target){
	var index = info.clients.indexOf(target);		//find target index in the array
	info.clients.splice(index, 1);					//delete element

	generate_client_table();
}

function applyRule(_on){
	var code = "";

	for(i=0;i<info.clients.length;i++){
		if(i != 0)
			code += "<";

		var client_index = info.clients[i];
		var object = info.clients[client_index];
		var client_active = object.active ? "1" : "0";
		var client_name = object.name;
		var client_password = object.password;
		var clinet_ipaddr = object.ipaddr;

		code +=  client_active + ">" + client_name + ">" + client_password + ">" + clinet_ipaddr;
	}

	document.form.radius_serv_list.value = code;;
	document.form.submit();
}

function check_active(obj){
	var target = info.clients[obj.id];
	if(obj.checked){
		target.active = true;
	}
	else{
		target.active = false;
	}
}
</script>
</head>

<body>
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="PermissionManagement_RADIUS.asp">
<input type="hidden" name="next_page" value="PermissionManagement_RADIUS.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="action_script" value="restart_radiusd">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="radius_serv_enable" value="<% nvram_get("radius_serv_enable"); %>">
<input type="hidden" name="radius_serv_list" value="<% nvram_get("radius_serv_list"); %>">
</form>

<div id="addTable" class="contentM_qis" style="height:320px;">
	<!--div id="group_info" style="display:none"-->
		<table width="97%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" style="margin: 20px 10px;">
			<thead>
				<tr>
					<td colspan="6">Create a new RADIUS client</td>
				</tr>
			</thead>		  
			<tr>
				<th width="30%" style="font-family: Calibri;font-weight: bolder;"><#Status_Active#></th>			
				<td>
					<select id="radius_client_active" class="input_option">
						<option value="1"><#WLANConfig11b_WirelessCtrl_button1name#></option>
						<option value="0"><#WLANConfig11b_WirelessCtrl_buttonname#></option>
					</select>
				</td>
			</tr>		
			<tr>
				<th width="30%" style="font-family: Calibri;font-weight: bolder;">Name</th>			
				<td>
					<input id="name" type="text" maxlength="32"class="input_32_table" style="height: 23px;" value="" autocorrect="off" autocapitalize="off">
				</td>
			</tr>
			<tr>
				<th width="30%" style="font-family: Calibri;font-weight: bolder;">Password</th>			
				<td>
					<input id="password" type="text" maxlength="32"class="input_32_table" style="height: 23px;" value="" autocorrect="off" autocapitalize="off">
				</td>
			</tr>
			<tr>
				<th width="30%" style="font-family: Calibri;font-weight: bolder;">Confirm Password</th>			
				<td>
					<input id="confirm_password" type="text" maxlength="32"class="input_32_table" style="height: 23px;" value="" autocorrect="off" autocapitalize="off">
				</td>
			</tr>
			<tr>
				<th width="30%" style="font-family: Calibri;font-weight: bolder;">IP Address</th>			
				<td>
					<input id="ip_address" type="text" maxlength="15" class="input_15_table" style="height: 23px;" onkeypress="return validator.isIPAddr(this, event)" value="" autocorrect="off" autocapitalize="off">
				</td>
			</tr>			
		</table>			
		<div class="apply_gen" style="margin-top:20px;margin-bottom:10px;background-color: #2B373B;">
			<input type="button" class="button_gen" onclick="hide_addTable();" value="<#CTL_Cancel#>"/>
			<input id="add_btn" name="button" type="button" class="button_gen" onclick="" value="<#CTL_apply#>"/>
		</div>
	<!--/div-->	
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
									<div class="formfonttitle"><#Permission_Management#> - <#Permission_Management_RADIUS#></div>
									<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
									<div class="formfontdesc"><#PM_RADIUS_desc#></div>
									<div>
										<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
											<tr>
												<th style="width:200px;">
													<div>Enable RADIUS Server</div>
												</th>								
												<td>
													<div align="center" class="left" style="width:94px; float:left; cursor:pointer;" id="radius_server_enable"></div>
													<div class="iphone_switch_container" style="height:32px; width:74px; position: relative; overflow: hidden">
														<script type="text/javascript">
															$('#radius_server_enable').iphoneSwitch('<% nvram_get("radius_serv_enable"); %>',
																function(){
																	document.form.radius_serv_enable.value = "1";
																	if(document.form.radius_serv_enable.value == '<% nvram_get("radius_serv_enable"); %>')
																		return false;

																	applyRule(1);																													
																},
																function(){
																	document.form.radius_serv_enable.value = "0";
																	if(document.form.radius_serv_enable.value == '<% nvram_get("radius_serv_enable"); %>')
																		return false;

																	applyRule(1);	
																}
															);
														</script>			
													</div>												
												
												</td>
											</tr>
										</table>																	
									</div>
									
									<div style="margin-top:20px;">
										<div style="display:flex">
											<div style="font-weight:900;padding-left:10px;line-height:34px;">RADIUS Clients (<#List_limit#>&nbsp;16)</div>
											<div style="margin-left:10px;">
												<div class="createAccountBtn_add" onclick="show_addTable('group', 'new');"></div>
											</div>										
										</div>									
										<table id="client_table" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable"></table>
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
