<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<title><#Web_Title#> - <#AiProtection_filter#></title>
<link rel="shortcut icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="device-map/device-map.css">
<script type="text/javascript" src="state.js"></script>
<script type="text/javascript" src="popup.js"></script>
<script type="text/javascript" src="general.js"></script>
<script type="text/javascript" src="help.js"></script>
<script type="text/javascript" src="validator.js"></script>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="form.js"></script>
<script type="text/javascript" src="switcherplugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" src="client_function.js"></script>
<style>
#switch_menu{
	text-align:right
}
#switch_menu span{
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
window.onresize = function() {
	if(document.getElementById("agreement_panel").style.display == "block") {
		cal_panel_block("agreement_panel", 0.25);
	}
} 

var wrs_filter = "<% nvram_get("wrs_rulelist"); %>".replace(/&#62/g, ">").replace(/&#60/g, "<");
var wrs_id_array = [["1,2,3,4,5,6,8", "9,10,14,15,16,25,26", "11"],
					["24", "50", "52", "51", "53,89", "42"],
					["56,70,71", "57"],
					["69", "23", "43", "41"]];	 

var curState = '<% nvram_get("wrs_enable"); %>';
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
var info = new Object();
info.device = new Array();
info.group = new Array();

function initial(){
	show_menu();
	translate_category_id();
	collect_info();
	genMain_table();
	if('<% nvram_get("wrs_enable"); %>' == 1)
		showhide("list_table",1);
	else
		showhide("list_table",0);
	
	generate_group_list();
	//showDropdownClientList('setClientIP', 'mac', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');
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

function pullLANIPList(obj){
	var element = document.getElementById('ClientList_Block_PC');
	var isMenuopen = element.offsetWidth > 0 || element.offsetHeight > 0;
	if(isMenuopen == 0){		
		obj.src = "/images/arrow-top.gif"
		element.style.display = 'block';		
		document.form.PC_devicename.focus();
	}
	else
		hideClients_Block();
}

function hideClients_Block(){
	document.getElementById("pull_arrow").src = "/images/arrow-down.gif";
	document.getElementById('ClientList_Block_PC').style.display='none';
}

function setClientIP(macaddr){
	document.form.PC_devicename.value = macaddr;
	hideClients_Block();
}

var previous_obj = "";
function show_subCategory(obj){
	var sub_category_state = obj.className
	var sub_category = $(obj).siblings()[1];
	var category_desc = $(obj).siblings()[2];
	
	if(sub_category_state == "closed"){	
		sub_category.style.display = "";
		if(category_desc)
			category_desc.style.display = "none";
		
		obj.setAttribute("class", "opened");
		if(previous_obj != ""){		//Hide another category
			$(previous_obj).siblings()[1].style.display = "none";
			if($(previous_obj).siblings()[2])
				$(previous_obj).siblings()[2].style.display = "";
			
			previous_obj.setAttribute("class", "closed");	
		}

		previous_obj = obj;
	}
	else{		
		sub_category.style.display = "none";
		if($(previous_obj).siblings()[2])
			$(previous_obj).siblings()[2].style.display = "";
		
		obj.setAttribute("class", "closed");		
		if($(previous_obj).siblings()[1] = sub_category){			//To handle open, close the same category
			$(previous_obj).siblings()[1] = "";
			previous_obj = "";	
		}
	}
}
	
function set_category(obj, flag){
	var checked_flag = 0;

	if(flag == 0){
		var children_length = $(obj).siblings()[1].children.length;
		if(obj.checked == true){
			for(i=0; i<children_length; i++){
				$(obj).siblings()[1].children[i].children[1].checked = true;	
			}
		}
		else{
			for(i=0; i<children_length; i++){
				$(obj).siblings()[1].children[i].children[1].checked = false;	
			}	
		}
	}
	else{
		var parent_category = $(obj.parentNode.parentNode).siblings()[1];
		var sibling = $(obj.parentNode).siblings();
		var sibling_length = $(obj.parentNode).siblings().length;
		
		if(obj.checked == true){
			if(parent_category.checked != true)
				parent_category.checked = true;
		}
		else{
			for(i=0; i<sibling_length; i++){
				if(sibling[i].children[1].checked == true){
					checked_flag = 1;			
				}
			}

			if(checked_flag == 0){
				parent_category.checked =false;		
			}	
		}
	}	
}

function deleteRow_main(obj){
	 var item_index = obj.parentNode.parentNode.rowIndex;
		document.getElementById(obj.parentNode.parentNode.parentNode.parentNode.id).deleteRow(item_index);
	
	var target_mac = obj.parentNode.parentNode.children[1].title;
	var wrs_filter_row = wrs_filter.split("<");
	var wrs_filter_temp = "";
	for(i=0;i<wrs_filter_row.length;i++){
		var wrs_filter_col = wrs_filter_row[i].split(">");	
			if(wrs_filter_col[1] != target_mac){
				if(wrs_filter_temp == ""){
					wrs_filter_temp += wrs_filter_row[i];			
				}
				else{
					wrs_filter_temp += "<" + wrs_filter_row[i];				
				}			
			}
	}

	wrs_filter = wrs_filter_temp;
	genMain_table();
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

function addRow_main(obj, length){	
	var category_array = $(obj.parentNode).siblings()[2].children;
	var subCategory_array;
	var category_checkbox;
	var enable_checkbox = $(obj.parentNode).siblings()[0].children[0];
	var first_catID = 0;
	var invalid_char = "";
	var blank_category = 0;
	var wrs_filter_row =  wrs_filter.split("<");
	var wrs_filter_col = "";
		
	if(document.form.PC_devicename.value == ""){
		alert("<#JS_fieldblank#>");
		document.form.PC_devicename.focus();
		return false;
	}
	
	for(i=0; i < category_array.length;i++){
		subCategory_array = category_array[i].children[2].children;	
		for(j=0;j<subCategory_array.length;j++){	
			category_checkbox = category_array[i].children[2].children[j].children[1];	
			if(category_checkbox.checked)
				blank_category = 1;
		}
	}
	for(i=0;i<wrs_filter_row.length;i++){
		if(wrs_filter_row[i] != "") {
			wrs_filter_col = wrs_filter_row[i].split(">");
			if(wrs_filter_col[1].toUpperCase() == document.form.PC_devicename.value.toUpperCase()){
				alert("<#JS_duplicate#>");
				document.form.PC_devicename.value = "";
				return false;
			}
		}
	}
	
	if(blank_category == 0){
		alert("The Content Category can not be empty");
		return false;
	}

	if(wrs_filter == ""){
		wrs_filter += enable_checkbox.checked ? 1:0;
	}	
	else{
		wrs_filter += "<";
		wrs_filter += enable_checkbox.checked ? 1:0;
	}	

	wrs_filter += ">@" + document.form.PC_devicename.value + ">";

	/* To check which checkbox is checked*/
	for(i=0; i < category_array.length;i++){
		first_catID = 0;
		subCategory_array = category_array[i].children[2].children;	
		for(j=0;j<subCategory_array.length;j++){	
			category_checkbox = category_array[i].children[2].children[j].children[1];
			if(first_catID == 0){	
				wrs_filter += category_checkbox.checked ? 1:0;
				first_catID = 1;				
			}
			else{	
				wrs_filter += ",";
				wrs_filter += category_checkbox.checked ? 1:0;		
			}
			
			if(category_checkbox.checked)
				blank_category = 1;
		}
		
		if(i != category_array.length -1)
			wrs_filter += ">";
	}
	
	document.form.PC_devicename.value = "";
	genMain_table();	
}
					 
function genMain_table(){
	var category_name = ["<#AiProtection_filter_Adult#>", "<#AiProtection_filter_message#>", "<#AiProtection_filter_p2p#>", "<#AiProtection_filter_stream#>"];
	var sub_category_name = [["<#AiProtection_filter_Adult1#>", "<#AiProtection_filter_Adult2#>", "<#AiProtection_filter_Adult3#>"],
							 ["<#AiProtection_filter_Adult4#>", "Social Networking", "E-Mail", "<#AiProtection_filter_Adult5#>", "<#AiProtection_filter_Adult6#>", "<#AiProtection_filter_Adult7#>"],
							 ["<#AiProtection_filter_p2p1#>", "<#AiProtection_filter_p2p2#>"],
							 ["<#AiProtection_filter_stream2#>", "<#AiProtection_filter_stream3#>", "Photo Searches", "Search Engine"]];
	
	var category_desc = ["<#AiProtection_filter_Adult_desc#>", 
						 "<#AiProtection_filter_message_desc#>", 
						 "<#AiProtection_filter_p2p_desc#>", 
						 "<#AiProtection_filter_stream_desc#>"];

	var wrs_filter_row = wrs_filter.split("<");
	var code = "";	
	code += '<table width="100%" border="1" cellspacing="0" cellpadding="4" align="center" class="FormTable_table" id="mainTable_table">';
	code += '<thead><tr>';
	//code += '<td colspan="5"><#ConnectedClient#>&nbsp;(<#List_limit#>&nbsp;16)</td>';
	code += '<td colspan="5">Rule List&nbsp;(<#List_limit#>&nbsp;16)</td>';/*untranslated*/
	code += '</tr></thead>';	
	code += '<tbody>';
	code += '<tr>';
	code += '<th width="5%" height="30px" title="<#select_all#>">';
	code += '<input id="selAll" type="checkbox" onclick="selectAll(this, 0);" value="">';
	code += '</th>';
	code += '<th width="40%">Group Name</th>';/*untranslated*/
	code += '<th width="40%"><#AiProtection_filter_category#></th>';
	code += '<th width="10%"><#list_add_delete#></th>';
	code += '</tr>';
	code += '<tr id="main_element">';	
	code += '<td style="border-bottom:2px solid #000;" title="<#WLANConfig11b_WirelessCtrl_button1name#>/<#btn_disable#>">';
	code += '<input type="checkbox" checked="">';
	code += '</td>';
	code += '<td style="border-bottom:2px solid #000;">';
	code += '<input type="text" maxlength="17" style="margin-left:10px;float:left;width:255px;" class="input_20_table" name="PC_devicename" onkeypress="return validator.isHWAddr(this,event)" onclick="hideClients_Block();" placeholder="ex: Group Name" autocorrect="off" autocapitalize="off">';
	code += '<img id="pull_arrow" height="14px;" src="/images/arrow-down.gif" onclick="pullLANIPList(this);" title="<#select_client#>">';
	code += '<div id="ClientList_Block_PC" class="clientlist_dropdown" style="margin-top:25px;margin-left:10px;"></div>';	
	code += '</td>';
	code += '<td style="border-bottom:2px solid #000;text-align:left;">';
	for(i=0;i<category_name.length;i++){
		code += '<div style="font-size:14px;font-weight:bold">';
		code += '<img class="closed" src="/images/Tree/vert_line_s10.gif" onclick="show_subCategory(this);">';
		code += '<input type="checkbox" onclick="set_category(this, 0);">'+category_name[i];
		code += '<div style="display:none;font-size:12px;">';
		for(j=0;j<sub_category_name[i].length;j++){
			code += '<div style="margin-left:20px;"><img src="/images/Tree/vert_line_s01.gif"><input type="checkbox" onclick="set_category(this, 1);">' + sub_category_name[i][j] + '</div>';		
		}
	
		code += '</div>';
		code += '<div style="margin-left:25px;color:#FC0;font-style:italic;font-size:12px;font-weight:normal;">'+ category_desc[i] +'</div>';
		code += '</div>';	
	}
	
	code += '</td>';
	code += '<td style="border-bottom:2px solid #000;"><input class="add_btn" type="button" onclick="addRow_main(this, 16)" value=""></td>';
	code += '</tr>';
	if(wrs_filter == ""){
		code += '<tr><td style="color:#FFCC00;" colspan="10"><#IPConnection_VSList_Norule#></td></tr>';
	}
	else{
		for(k=0;k<wrs_filter_row.length;k++){
			var wrs_filter_col = wrs_filter_row[k].split('>');

			//user icon
			var userIconBase64 = "NoIcon";
			var clientName, clientMac, clientIP, deviceType, deviceVender;
			var clientMac = wrs_filter_col[1];
			var clientObj = clientList[clientMac];
			if(clientObj) {
				clientName = (clientObj.nickName == "") ? clientObj.name : clientObj.nickName;
				clientIP = clientObj.ip;
				deviceType = clientObj.type;
				deviceVender = clientObj.vendor;
			}
			else {
				group_name = wrs_filter_col[1].split("@");
				clientName = group_name[1];
				clientIP = "offline";
				deviceType = 0;
				deviceVender = "";
			}

			code += '<tr>';
			code += '<td title="<#WLANConfig11b_WirelessCtrl_button1name#>/<#btn_disable#>">';
			if(wrs_filter_col[0] == 1)
				code += '<input type="checkbox" checked>';
			else	
				code += '<input type="checkbox">';
							
			code += '</td>';

			code +='<td title="' + clientMac + '">';
			code += '<table style="width:100%;"><tr><td style="width:40%;height:56px;border:0px;float:right;margin-right:20px;">';
			if(clientObj == undefined) {
				code += '<div class="clientIcon type0" onClick="popClientListEditTable(\'' + clientMac + '\', this, \'' + clientName + '\', \'' + clientIP + '\', \'WebProtector\')"></div>';
			}
			else {
				if(usericon_support) {
					userIconBase64 = getUploadIcon(clientMac.replace(/\:/g, ""));
				}
				if(userIconBase64 != "NoIcon") {
					code += '<div style="text-align:center;" onClick="popClientListEditTable(\'' + clientMac + '\', this, \'' + clientName + '\', \'' + clientIP + '\', \'WebProtector\')"><img class="imgUserIcon_card" src="' + userIconBase64 + '"></div>';
				}
				else if(deviceType != "0" || deviceVender == "") {
					code += '<div class="clientIcon type' + deviceType + '" onClick="popClientListEditTable(\'' + clientMac + '\', this, \'' + clientName + '\', \'' + clientIP + '\', \'WebProtector\')"></div>';
				}
				else if(deviceVender != "" ) {
					var venderIconClassName = getVenderIconClassName(deviceVender.toLowerCase());
					if(venderIconClassName != "" && !downsize_4m_support) {
						code += '<div class="venderIcon ' + venderIconClassName + '" onClick="popClientListEditTable(\'' + clientMac + '\', this, \'' + clientName + '\', \'' + clientIP + '\', \'WebProtector\')"></div>';
					}
					else {
						code += '<div class="clientIcon type' + deviceType + '" onClick="popClientListEditTable(\'' + clientMac + '\', this, \'' + clientName + '\', \'' + clientIP + '\', \'WebProtector\')"></div>';
					}
				}
			}
			code += '</td><td style="width:60%;border:0px;text-align:left;">';
			code += '<div>' + clientName + '</div>';
			//code += '<div>' + clientMac + '</div>';
			code += '</td></tr></table>';
			code +='</td>';
			
			code += '<td style="text-align:left;">';	
			for(i=0;i<category_name.length;i++){
				var cate_flag_array = new Array(wrs_filter_col[i+2]);
				var cate_flag_array_col = cate_flag_array[0].split(",");
				var cate_flag = cate_flag_array[0].indexOf('1');

				code += '<div>';
				code += '<img class="closed" src="/images/Tree/vert_line_s10.gif" onclick="show_subCategory(this);">';
				if(cate_flag != -1){
					code += '<input type="checkbox" onclick="set_category(this, 0);" checked>'+category_name[i];
				}
				else{
					code += '<input type="checkbox" onclick="set_category(this, 0);">'+category_name[i];
				}

				code += '<div style="display:none;">';
				for(j=0;j<sub_category_name[i].length;j++){
					if(cate_flag_array_col[j] == 1){
						code += '<div style="margin-left:20px;"><img src="/images/Tree/vert_line_s01.gif"><input type="checkbox" onclick="set_category(this, 1);" checked>' + sub_category_name[i][j] + '</div>';
					}
					else{
						code += '<div style="margin-left:20px;"><img src="/images/Tree/vert_line_s01.gif"><input type="checkbox" onclick="set_category(this, 1);">' + sub_category_name[i][j] + '</div>';
					}
				}
			
				code += '</div>';
				code += '</div>';
			}
			
			code += '</td>';
			code += '<td><input class="remove_btn" type="button" onclick="deleteRow_main(this);"></td>';
			code += '</tr>';
		}
	}
	
	code += '</tbody>';	
	code += '</table>';
	document.getElementById('mainTable').innerHTML = code;
	generate_group_list();
	//showDropdownClientList('setClientIP', 'mac', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');
}

function edit_table(){
	var wrs_filter_temp = "";
	var first_element = $('#main_element').siblings();
	
	for(k=1;k<first_element.length;k++){
		var enable_checkbox = first_element[k].children[0].children[0].checked ? 1:0;
		var target_mac = first_element[k].children[1].title;
		var category_array = first_element[k].children[2].children;
		var subCategory_array;
		var category_checkbox;
		var first_catID = 0;
		var blank_category = 0;
		
		if(k == 1)
			wrs_filter_temp += enable_checkbox;
		else{
			wrs_filter_temp += "<" + enable_checkbox;;
		}
		
		wrs_filter_temp += ">" + target_mac + ">";
		
		for(i=0; i < category_array.length;i++){
			first_catID = 0;
			subCategory_array = category_array[i].children[2].children;
			for(j=0;j<subCategory_array.length;j++){
				category_checkbox = category_array[i].children[2].children[j].children[1];
				if(category_checkbox.checked)
					blank_category = 1;
				
				if(first_catID == 0){	
					wrs_filter_temp += category_checkbox.checked ? 1:0;	
					first_catID = 1;				
				}
				else{	
					wrs_filter_temp += ",";
					wrs_filter_temp += category_checkbox.checked ? 1:0;		
				}
			}			
			
			if(i != category_array.length -1)
				wrs_filter_temp += ">";
		}		
		
		if(blank_category == 0){
				alert("The Content Category can not be empty");
				return false;
		}
	}
	
	
	wrs_filter = wrs_filter_temp;
	return true;
}

var ctf_disable = '<% nvram_get("ctf_disable"); %>';
var ctf_fa_mode = '<% nvram_get("ctf_fa_mode"); %>';					
function applyRule(){
	var wrs_filter_row = "";
	if(document.form.PC_devicename.value != ""){
		alert("You must press add icon to add a new rule first.");
		return false;
	}
	
	if(wrs_filter != ""){
		if(edit_table())
			wrs_filter_row =  wrs_filter.split("<");
		else	
			return false;
	}	

	var wrs_rulelist = "";
	for(i=0;i<wrs_filter_row.length;i++){
		var wrs_filter_col = wrs_filter_row[i].split(">");
		for(j=0;j<wrs_filter_col.length;j++){
			if(j == 0){		
				if(wrs_rulelist == ""){
					wrs_rulelist += wrs_filter_col[j] + ">";
				}	
				else{	
					wrs_rulelist += "<" + wrs_filter_col[j] + ">";					
				}	
			}
			else if(j == 1){
				wrs_rulelist += wrs_filter_col[j] + ">";
			}
			else{
				var cate_id_array = wrs_filter_col[j].split(",");
				var wrs_first_cate = 0;
				for(k=0;k<cate_id_array.length;k++){
					if(cate_id_array[k] == 1){						
						if(wrs_first_cate == 0){						
							if(wrs_id_array[j-2][k] != ""){
								wrs_rulelist += wrs_id_array[j-2][k];
								wrs_first_cate = 1;
							}
						}
						else{
							if(wrs_id_array[j-2][k] != ""){
								wrs_rulelist += "," + wrs_id_array[j-2][k];
							}
						}	
					}						
				}
				
				if(j != wrs_filter_col.length-1){
					wrs_rulelist += ">";
				}
			}
		}	
	}

	document.form.action_script.value = "restart_wrs;restart_firewall";
	document.form.wrs_rulelist.value = wrs_rulelist;
	if(ctf_disable == 0 && ctf_fa_mode == 2){
		if(!confirm(Untranslated.ctf_fa_hint)){
			return false;
		}	
		else{
			document.form.action_script.value = "reboot";
			document.form.action_wait.value = "<% nvram_get("reboot_time"); %>";
		}	
	}

	if(reset_wan_and_nat(document.form, document.form.wrs_enable.value)) {
		showLoading();
		document.form.submit();
	}
}

function translate_category_id(){
	var mapping_array_init = [[0,0,0], [0,0,0,0,0,0], [0,0], [0,0,0,0]];
	var wrs_filter_row = "";
	if(wrs_filter != "")
		wrs_filter_row =  wrs_filter.split("<");
	
	if(wrs_filter != ""){
		wrs_filter_row =  wrs_filter.split("<");	
	}
	
	var wrs_filter_temp = "";
	for(i=0;i<wrs_filter_row.length;i++){
		var wrs_filter_col = wrs_filter_row[i].split(">");
		for(j=0;j<wrs_filter_col.length;j++){
			if(j == 0){
				if(wrs_filter_temp == "")
					wrs_filter_temp += wrs_filter_col[j] + ">";
				else
					wrs_filter_temp += "<" + wrs_filter_col[j] + ">";
			}
			else if(j == 1){
				wrs_filter_temp += wrs_filter_col[j] + ">";	
			}
			else{
				for(k=0;k<mapping_array_init[j-2].length;k++){		
					if(wrs_filter_col[j] != ""){
						if(wrs_id_array[j-2][k] != ""){
							if(wrs_id_array[j-2][k] != "" && (wrs_filter_col[j].indexOf(wrs_id_array[j-2][k]) != -1)){
								mapping_array_init[j-2][k] = 1;
							}
						}				
					}
					else{
						mapping_array_init[j-2][k] = 0;
					}
						
					if(k == 0)
						wrs_filter_temp += mapping_array_init[j-2][k];	
					else
						wrs_filter_temp += "," + mapping_array_init[j-2][k];						
				}	
	
				if(j != wrs_filter_col.length-1 )
					wrs_filter_temp += ">";
			}
		}

		mapping_array_init = [[0,0,0], [0,0,0,0,0,0], [0,0], [0,0,0,0]];
	}

	wrs_filter = wrs_filter_temp;
}

function show_tm_eula(){
	if(document.form.preferred_lang.value == "JP"){
		$.get("JP_tm_eula.htm", function(data){
			document.getElementById('agreement_panel').innerHTML= data;
			adjust_TM_eula_height("agreement_panel");
		});
	}
	else{
		$.get("tm_eula.htm", function(data){
			document.getElementById('agreement_panel').innerHTML= data;
			adjust_TM_eula_height("agreement_panel");
		});
	}
	dr_advise();
	cal_panel_block("agreement_panel", 0.25);
	$("#agreement_panel").fadeIn(300);
}

function cancel(){
	$("#agreement_panel").fadeOut(100);
	$('#iphone_switch').animate({backgroundPosition: -37}, "slow", function() {});
	curState = 0;
	document.getElementById("hiddenMask").style.visibility = "hidden";
	htmlbodyforIE = parent.document.getElementsByTagName("html");  //this both for IE&FF, use "html" but not "body" because <!DOCTYPE html PUBLIC.......>
	htmlbodyforIE[0].style.overflow = "scroll";	  //hidden the Y-scrollbar for preventing from user scroll it.
}

function eula_confirm(){
	document.form.TM_EULA.value = 1;
	document.form.wrs_enable.value = 1;	
	applyRule();
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

	$("#ClientList_Block_PC").html(code);
}

function setGroup(name){
	document.form.PC_devicename.value = name;
	hideClients_Block();
}

</script>
</head>

<body onload="initial();" onunload="unload_body();" onselectstart="return false;">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<div id="agreement_panel" class="eula_panel_container"></div>
<div id="hiddenMask" class="popup_bg" style="z-index:999;">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center"></table>
	<!--[if lte IE 6.5]><script>alert("<#ALERT_TO_CHANGE_BROWSER#>");</script><![endif]-->
</div>
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="AiProtection_WebProtector.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>" disabled>
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="wrs_enable" value="<% nvram_get("wrs_enable"); %>">
<input type="hidden" name="wrs_enable_ori" value="<% nvram_get("wrs_enable"); %>">
<input type="hidden" name="wrs_rulelist" value="">
<input type="hidden" name="TM_EULA" value="<% nvram_get("TM_EULA"); %>">
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
						<tr>
							<td bgcolor="#4D595D" valign="top">
								<div>&nbsp;</div>
								<!--div class="formfonttitle"></div-->
								<div style="margin-top:-5px;">
									<table width="730px">
										<tr>
											<td align="left">
												<div class="formfonttitle" style="width:400px"><#menu5_5#> - Web Content Filters</div>
											</td>
										</tr>
									</table>
								</div>
								<div style="margin:0px 0px 10px 5px;"><img src="/images/New_ui/export/line_export.png"></div>
								<div id="PC_desc">
									<table width="700px" style="margin-left:25px;">
										<tr>
											<!--td>
												<img id="guest_image" src="/images/New_ui/Web_Apps_Restriction.png">
											</td>
											<td>&nbsp;&nbsp;</td-->
											<td style="font-style: italic;font-size: 14px;">
												<span>Web Content Filters allows you to block access to unwanted websites.</span>
												<!--ol>
													<li><#AiProtection_filter_desc2#></li>
													<li><#AiProtection_filter_desc3#></li>
													<li><#AiProtection_filter_desc4#></li>
												</ol>
												<span><#AiProtection_filter_note#></span-->
												<!--div><a style="text-decoration:underline;" href="http://www.asus.com/support/FAQ/1008720/" target="_blank"><#Parental_Control#> FAQ</a></div-->
											</td>
										</tr>
									</table>
								</div>
								<br>
			<!--=====Beginning of Main Content=====-->
								<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
									<tr>
										<th>Enable Web Content Filters</th>
										<td>
											<div align="center" class="left" style="width:94px; float:left; cursor:pointer;" id="radio_web_restrict_enable"></div>
											<div class="iphone_switch_container" style="height:32px; width:74px; position: relative; overflow: hidden">
												<script type="text/javascript">
													$('#radio_web_restrict_enable').iphoneSwitch('<% nvram_get("wrs_enable"); %>',
														function(){
															curState = 1;
															if(document.form.TM_EULA.value == 0){
																show_tm_eula();
																return;
															}	
															
															document.form.wrs_enable.value = 1;	
															showhide("list_table",1);
															
														},
														function(){
															document.form.wrs_enable.value = 0;
															showhide("list_table",0);
															document.form.wrs_rulelist.disabled = true;
															if(document.form.wrs_enable_ori.value == 1){
																applyRule();
															}
															else{
																curState = 0;
															}
																														
														}
													);
												</script>			
											</div>
										</td>
									</tr>								
									<tr style="display:none;">
										<th>Enable Block Notification page</th>
										<td>
											<div align="center" class="left" style="width:94px; float:left; cursor:pointer;" id="block_notification_enable"></div>
											<div class="iphone_switch_container" style="height:32px; width:74px; position: relative; overflow: hidden">
												<script type="text/javascript">
													$('#block_notification_enable').iphoneSwitch('<% nvram_get(""); %>',
														function(){
									
														},
														function(){
																					
														}
													);
												</script>			
											</div>
										</td>			
									</tr>									
								</table>				
								<table id="list_table" width="100%" border="0" align="center" cellpadding="0" cellspacing="0" style="display:none">
									<tr>
										<td valign="top" align="center">
											<div id="mainTable" style="margin-top:10px;"></div> 
											<div id="ctrlBtn" style="text-align:center;margin-top:20px;">
												<input class="button_gen" type="button" onclick="applyRule();" value="<#CTL_apply#>">											
												<div style="width:135px;height:55px;position:absolute;bottom:5px;right:5px;background-image:url('images/New_ui/tm_logo_power.png');"></div>
											</div>
										</td>	
									</tr>
								</table>
							</td>
						</tr>
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

