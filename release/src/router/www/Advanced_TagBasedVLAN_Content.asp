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
<title><#Web_Title#> - VLAN</title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<script type="text/javascript" src="state.js"></script>
<script type="text/javascript" src="general.js"></script>
<script type="text/javascript" src="popup.js"></script>
<script type="text/javascript" src="help.js"></script>
<script type="text/javaScript" src="js/jquery.js"></script>
<script type="text/javascript" src="switcherplugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" src="validator.js"></script>
<script type="text/javascript" src="js/subnet_rule.js"></script>
<style>
.contentM_connection{
	position:absolute;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius: 5px;
	z-index:500;
	background-color:#2B373B;
	margin-left: 34%;
	margin-top: 10px;
	width:650px;
	display:none;
	box-shadow: 3px 3px 10px #000;
}

.pvid_div{
	width: 306px;
	height: 20px;
	margin: 0 auto;
}

.pvid_select_div{
	display: table-cell;
	width: 25%;
	float: left;
}

.pvid_select{
	width: 55px;
	background-color: #204C4C;
	color: #FFFFFF;
	border-style: none;
	margin-left: 10px;
}
</style>

<script>
<% wanlink(); %>
var PortType_Value = {"None": "00", "Untagged": "01", "Tagged": "10", "Override": "11"};
var PortType_Text_Value = [ {"type_text": "None", "value": "00"},
							{"type_text": "<#VLAN_Untagged#>", "value": "01"},
							{"type_text": "<#VLAN_Tagged#>", "value": "10"},
							{"type_text": "<#VLAN_Override#>", "value": "11"}];
var Interface_Text_Value = [{"if_text": "WAN", "value": "wan0", "type": "00"},
							{"if_text": "WAN2", "value": "wan1", "type": "00"},
					  		{"if_text": "LAN 1", "value": "lan1", "type": "00"},
					  		{"if_text": "LAN 2", "value": "lan2", "type": "00"},
					  		{"if_text": "LAN 3", "value": "lan3", "type": "00"},
					  		{"if_text": "LAN 4", "value": "lan4", "type": "00"},
					  		{"if_text": "LAN 5", "value": "lan5", "type": "00"},
					  		{"if_text": "LAN 6", "value": "lan6", "type": "00"},
					  		{"if_text": "LAN 7", "value": "lan7", "type": "00"},
					  		{"if_text": "LAN 8", "value": "lan8", "type": "00"},
					  		{"if_text": "<% nvram_char_to_ascii("","wl0_ssid"); %>", "value": "wl0", "type": "00"},
					  		{"if_text": "<% nvram_char_to_ascii("","wl0.1_ssid"); %>", "value": "wl0.1", "type": "00"},
					  		{"if_text": "<% nvram_char_to_ascii("","wl0.2_ssid"); %>", "value": "wl0.2", "type": "00"},
					  		{"if_text": "<% nvram_char_to_ascii("","wl0.3_ssid"); %>", "value": "wl0.3", "type": "00"},
					  		{"if_text": "<% nvram_char_to_ascii("","wl0.4_ssid"); %>", "value": "wl0.4", "type": "00"},
					  		{"if_text": "<% nvram_char_to_ascii("","wl0.5_ssid"); %>", "value": "wl0.5", "type": "00"},
					  		{"if_text": "<% nvram_char_to_ascii("","wl0.6_ssid"); %>", "value": "wl0.6", "type": "00"},
					  		{"if_text": "<% nvram_char_to_ascii("","wl1_ssid"); %>", "value": "wl1", "type": "00"},
					  		{"if_text": "<% nvram_char_to_ascii("","wl1.1_ssid"); %>", "value": "wl1.1", "type": "00"},
					  		{"if_text": "<% nvram_char_to_ascii("","wl1.2_ssid"); %>", "value": "wl1.2", "type": "00"},
					  		{"if_text": "<% nvram_char_to_ascii("","wl1.3_ssid"); %>", "value": "wl1.3", "type": "00"},
					  		{"if_text": "<% nvram_char_to_ascii("","wl1.4_ssid"); %>", "value": "wl1.4", "type": "00"},
					  		{"if_text": "<% nvram_char_to_ascii("","wl1.5_ssid"); %>", "value": "wl1.5", "type": "00"},
					  		{"if_text": "<% nvram_char_to_ascii("","wl1.6_ssid"); %>", "value": "wl1.6", "type": "00"}];
var vlan_if_list = decodeURIComponent("<% nvram_char_to_ascii("","vlan_if_list"); %>");
var vlan_wl_array = [];
var all_interface = "";
var all_interfaces_array = [];
var bind_interface_array = [];
//<1>14>0>0001>0000000F>0000>0000>subnet1>1>0
var vlan_rulelist = decodeURIComponent("<% nvram_char_to_ascii("","vlan_rulelist"); %>");
var MAX_VLAN_NUM = 8;
var wan_num = 2;
var wireless_num = 7;
var lan_num = 8;
var selected_row = 0;
var vlan_enable_array = [];
var vlan_used_vid_array = [];
var subnet_array = [];
var old_tGatewayIP = "";
var lan1_pvid_array = [];
var lan2_pvid_array = [];
var lan3_pvid_array = [];
var lan4_pvid_array = [];
var lan5_pvid_array = [];
var lan6_pvid_array = [];
var lan7_pvid_array = [];
var lan8_pvid_array = [];
var default_lanip = '<% nvram_get("lan_ipaddr"); %>';
var default_gateway_ip = ('<% nvram_get("dhcp_gateway_x"); %>' == "")? default_lanip:'<% nvram_get("dhcp_gateway_x"); %>';
var default_netmaks = '<% nvram_get("lan_netmask"); %>';
var default_subnet = default_gateway_ip + "/" + netmask_to_bits(default_netmaks);
var vlan1_wl_array = [];
var vlan_pvid_list_array = decodeURIComponent('<% nvram_char_to_ascii("","vlan_pvid_list"); %>').split('>');
var switch_stb_x = '<% nvram_get("switch_stb_x"); %>';
var iptv_lanport_list = [];
if(switch_stb_x == "1")
	iptv_lanport_list = ["lan1"];
else if(switch_stb_x == "2")
	iptv_lanport_list = ["lan2"];
else if(switch_stb_x == "3")
	iptv_lanport_list = ["lan3"]
else if(switch_stb_x == "4")
	iptv_lanport_list = ["lan4"];
else if(switch_stb_x == "5")
	iptv_lanport_list = ["lan1", "lan2"];
else if(switch_stb_x == "6" || switch_stb_x == "8")
	iptv_lanport_list = ["lan3", "lan4"];

var lan_as_wan = (wans_dualwan_array.getIndexByValue("lan") != -1)? 1:0;
var wans_lanport = "lan" + '<% nvram_get("wans_lanport"); %>';
var orig_lan_trunk_type = '<% nvram_get("lan_trunk_type"); %>';

function initial(){
	show_menu();
	parse_LanToLanRoute_to_object();
	parse_vlan_if_list();
	show_vlan_rulelist();
	show_pvid_list();
	document.getElementById("default_subnet_desc").innerHTML = default_subnet +" (<#Setting_factorydefault_value#>)";
}

function applyRule(){
	if(check_duplicate_subnet()){
		document.form.vlan_rulelist.value = vlan_rulelist;
		document.form.subnet_rulelist.value = subnet_rulelist;
		save_LanToLanRoute();
		document.form.subnet_rulelist_ext.value = subnet_rulelist_ext;
		set_pvid_list();
		showLoading();
		document.form.submit();
	}
	else
		return;

}

function htmlEncode_decodeURI(value){
	return htmlEnDeCode.htmlEncode(decodeURIComponent(value));
}

function check_duplicate_subnet(){
	var vlan_rulelist_row = vlan_rulelist.split('<');
	var vlan_used_subnets = [];
	var duplicate_subnets = [];
	for(var i = 1; i < vlan_rulelist_row.length; i++){
		var vlan_rulelist_col = vlan_rulelist_row[i].split('>');
		vlan_used_subnets.push(vlan_rulelist_col[7])
	}
	vlan_used_subnets.sort();

	for(var i = 0; i < vlan_used_subnets.length; i++){
		if (vlan_used_subnets[i + 1] == vlan_used_subnets[i]) {
        	duplicate_subnets.push(vlan_used_subnets[i]);
    	}
	}

	if(duplicate_subnets.length == 0)
		return true;
	else{
		var duplicate_subnets_string = duplicate_subnets.join(', ');
		alert("<#Subnet#>: "+ duplicate_subnets_string + "\n" +"<#TBVLAN_DupSubnet_Warning#>");
		return false;
	}
}

function showSettingTable(show, edit){
	var cur_subnet = "";

	if(show == 0){
		$("#VlanSettings_table").fadeOut(300);
		document.form.vlan_id.value = "";
		document.form.vlan_prio.value = "";
		bind_interface_array.length = 0;
	}
	else{
		if(typeof(edit) == "number" && edit){
			document.getElementById("addRuleBtn").onclick = function(){
				change_vlan_rule();
			};
		}
		else{
			document.getElementById("vlan_id").style.display = "";
			document.getElementById("default_vlan_id").style.display = "none";
			document.getElementById("vlan_prio").style.display = "";
			document.getElementById("default_vlan_prio").style.display = "none";
			document.getElementById("subnet_sel").style.display = "";
			document.getElementById("default_subnet_desc").style.display = "none";
			document.getElementById("addRuleBtn").onclick = function(){
				add_vlan_rule();
			};
			selected_row = 999;//mean to add a new vlan rule
		}

		generate_sunbet_options(document.form.subnet_list);
		if(typeof(edit) == "number" && edit){
			for(var j = 0; j < document.form.subnet_list.length; j++){
				if(vlan_rulelist_table.rows[selected_row].cells[5].innerHTML == document.form.subnet_list.options[j].text){
					document.form.subnet_list.selectedIndex = j;
					break;
				}
			}
		}

		if($("#default_subnet_desc").css("display") != 'none')
			cur_subnet = default_subnet;
		else
			cur_subnet = document.form.subnet_list.value;

		show_binding_list();
		show_inf_list();

		get_LanToLanRoute(cur_subnet);
		generate_LanToLanRoute_options();
		show_LanToLanRoute_list();

		change_interface_type(document.form.interface_list);
		$("#VlanSettings_table").fadeIn(300);
	}
}

function inf_value_to_text(val){
	for(var i = 0; i < Interface_Text_Value.length; i++){
		if(Interface_Text_Value[i].value == val)
			return decodeURIComponent(Interface_Text_Value[i].if_text);
	}
}

function inf_text_to_value(text){
	for(var i = 0; i < Interface_Text_Value.length; i++){
		if(decodeURIComponent(Interface_Text_Value[i].if_text) == text)
			return Interface_Text_Value[i].value;
	}
}

function show_inf_list(){
	var binding_inf_table = document.getElementById("binding_inf_table");
	var inf_text = "";
	var add_option = 1;

	document.form.interface_list.options.length = 0;
	for(var j = 0; j < all_interfaces_array.length; j++){
		inf_text = inf_value_to_text(all_interfaces_array[j]);
		add_option = 1;
		for( var k = 0; k < binding_inf_table.rows.length; k++){
			if( inf_text == binding_inf_table.rows[k].cells[0].innerHTML){
				add_option = 0;
				break;
			}
		}

		if(add_option)
			add_infoption_bytext(inf_text);
	}

	if(document.form.interface_list.options.length == 0)
		document.getElementById("addInterface_tr").style.display = "none";
}

function change_interface_type(obj){
	var inf = obj.value;
	var select = document.getElementById("port_type");
	var length = select.length;
	var option;

	select.options.length = 0;
	for( var i = 0; i < wan_num + lan_num + (wireless_num*2); i++){
		if(inf == Interface_Text_Value[i].value){
			var inf_3 = inf.substr(0,3);
			if( inf_3 == "wan" || inf_3 == "usb"){
				select.options[select.length] = new Option("<#VLAN_Tagged#>", PortType_Value.Tagged);
			}
			else if(inf_3 == "lan"){
				select.options[select.length] = new Option("<#VLAN_Untagged#>", PortType_Value.Untagged);
				select.options[select.length] = new Option("<#VLAN_Tagged#>", PortType_Value.Tagged);
			}
			else if(inf_3.slice(0,2) == "wl"){
				select.options[select.length] = new Option("<#VLAN_Untagged#>", PortType_Value.Untagged);
			}
		}
	}
}

function set_port_type(interface_value, port_type){
	for(var i = 0; i < Interface_Text_Value.length; i++){
		if(Interface_Text_Value[i].value == interface_value)
			Interface_Text_Value[i].type = port_type;
	}
}

function clear_port_settings(){
	for(var i = 0; i < Interface_Text_Value.length; i++){
		Interface_Text_Value[i].type = PortType_Value.None;
	}
}

function update_vlan1_rule(){
	var vlan_rulelist_row = vlan_rulelist.split('<');
	var vlan1_col = vlan_rulelist_row[1].split('>');
	var wireless_value_str_2g = "";
	var wireless_value_str_5g = "";
	var tmp_value = 0;

	for(var i = wan_num + lan_num; i < wan_num + lan_num + wireless_num; i++){
		var wireless_index_2g = i - wan_num - lan_num;
		for(var j = 0; j < vlan1_wl_array.length; j++){
			if(Interface_Text_Value[i].value == vlan1_wl_array[j]){
				tmp_value = tmp_value | (1 << wireless_index_2g);
				break;
			}
		}
	}
	wireless_value_str_2g = (tmp_value + (0x10000)).toString(16).substr(-4).toUpperCase();
	vlan1_col[5] = wireless_value_str_2g;

	tmp_value = 0;
	for(var i = (wan_num + lan_num + wireless_num); i < (wan_num + lan_num + wireless_num*2); i++){
		var wireless_index_5g = i - wan_num - lan_num - wireless_num;
		for(var j = 0; j < vlan1_wl_array.length; j++){
			if(Interface_Text_Value[i].value == vlan1_wl_array[j]){
				tmp_value = tmp_value | (1 << wireless_index_5g);
				break;
			}
		}
	}
	wireless_value_str_5g = (tmp_value + (0x10000)).toString(16).substr(-4).toUpperCase();
	vlan1_col[6] = wireless_value_str_5g;

	vlan_rulelist_row[1] = vlan1_col.join('>');
	vlan_rulelist = vlan_rulelist_row.join('<');
}

function add_BindingInf(){
	var interface_select = document.form.interface_list;
	set_port_type(interface_select.options[interface_select.selectedIndex].value, document.form.port_type.value);
	Do_addRow_Group();
}

function Do_addRow_Group(){
	var interface_select = document.form.interface_list;
	var interface_text = document.form.interface_list.options[document.form.interface_list.selectedIndex].text;
	var interface_value = document.form.interface_list.options[document.form.interface_list.selectedIndex].value;
	var inf_element = { "text": interface_text,
					    "value": interface_value,
					    "type": document.form.port_type[document.form.port_type.selectedIndex].value };
	bind_interface_array.push(inf_element);
	interface_select.remove(interface_select.selectedIndex);
	if(interface_select.length == 0)
		document.getElementById("addInterface_tr").style.display = "none";
	change_interface_type(interface_select);
	show_binding_list();

	//Remove wireless interface from all_interfaces_array and vlan1_wl_array
	if(interface_value.slice(0, 2) == "wl"){
		remove_from_inf_list(interface_value);
		Object.keys(vlan1_wl_array).forEach(function(key){
			if(vlan1_wl_array[key] == interface_value)
				vlan1_wl_array.splice(key, 1);
		});
		update_vlan1_rule();
	}
}

function add_infoption_bytext(interface_text){
	var interface_select = document.form.interface_list;
	var option = document.createElement("option");
	document.getElementById("addInterface_tr").style.display = "";
	option.text = interface_text;
	for(var i = 0; i < Interface_Text_Value.length; i++){
		if(interface_text == decodeURIComponent(Interface_Text_Value[i].if_text)){
			option.value = Interface_Text_Value[i].value;
			break;
		}
	}
	interface_select.add(option);
	change_interface_type(interface_select);
}

function del_BindingInf(r){
	var index = r.parentNode.parentNode.rowIndex;
	var binding_inf_table = document.getElementById('binding_inf_table');
	var del_interface_text = htmlEnDeCode.htmlDecode(binding_inf_table.rows[index].cells[0].innerHTML);
	var del_interface_value = inf_text_to_value(del_interface_text);

	//Add wireless interfaces back to all_interfaces_array and vlan1_wl_array
	if(del_interface_value.slice(0, 2) == "wl"){
		add_to_inf_list(del_interface_text);
		vlan1_wl_array.push(del_interface_value);
		update_vlan1_rule();
	}

	binding_inf_table.deleteRow(index);
  	bind_interface_array.length = 0;
	for(var k = 0; k < binding_inf_table.rows.length; k++){
		var inf_element = {"text": htmlEnDeCode.htmlDecode(binding_inf_table.rows[k].cells[0].innerHTML),
						   "value": inf_text_to_value(htmlEnDeCode.htmlDecode(binding_inf_table.rows[k].cells[0].innerHTML)),
						   "type": find_portType_value(htmlEnDeCode.htmlDecode(binding_inf_table.rows[k].cells[1].innerHTML))};
		bind_interface_array.push(inf_element);
	}

	show_inf_list();
	if(bind_interface_array.length == 0)
		show_binding_list();
}

function show_binding_list(){
	var code = "";

	code +='<table width="97%" cellspacing="0" cellpadding="4" align="center" class="list_table" id="binding_inf_table">';
	
	if(bind_interface_array.length == 0)
		code +="<tr><td style='color:#FFCC00;' colspan='3'><#TBVLAN_NoBindingInf#></td>";
	else{
		Object.keys(bind_interface_array).forEach(function(key){
			code +='<tr id="row'+key+'">';
			var wid=[40, 40];
			code +='<td width="'+wid[0]+'%">'+ htmlEnDeCode.htmlEncode(bind_interface_array[key].text) +'</td>';
			code +='<td width="'+wid[1]+'%">'+ htmlEnDeCode.htmlEncode(find_portType_text(bind_interface_array[key].type)) +'</td>';
			if(selected_row == 0 && bind_interface_array[key].value.substr(0, 2) == "wl")
				code +='<td width="20%"></td>';
			else
				code +='<td width="20%"><input class="remove_btn" onclick="del_BindingInf(this);" value=""/></td>';
			code += '</tr>';
		});
	}	
	code +='</table>';
	document.getElementById('binding_inflist_Block').innerHTML = code;
}

function validSettings(change_index){
	var vid_used = 0;

	if(change_index != 0){
		if(document.form.subnet_list.value == "new"){
			alert("<#TBVLAN_AddSubnet_Warning#>");
			return false;
		}
		
		Object.keys(vlan_used_vid_array).forEach(function(key) {
			if(document.form.vlan_id.value == vlan_used_vid_array[key]){
				if(key != change_index)
					vid_used = 1;
			}
		});

		if(document.form.vlan_id.value.length == 0){
			document.form.vlan_id.focus();
			return false;
		}
		else if(vid_used){
			alert("<#TBVLAN_VID_Warning#>");
			document.form.vlan_id.focus();
			return false;
		}
		else if(document.form.vlan_id.value.length > 0 && change_index != 0 && !validator.rangeNull(document.form.vlan_id, 2, 4094, ""))
	    	return false;

		if(document.form.vlan_prio.value.length == 0){
			document.form.vlan_prio.focus();
			return false;
		}
		else if(document.form.vlan_prio.value.length > 0 && !validator.range(document.form.vlan_prio, 0, 7))
			return false;
	}
	if(bind_interface_array.length == 0){
		alert("<#TBVLAN_BindingInf_Hint#>");
		return false;
	}

	return true;
}

function add_vlan_rule(){
	var vlan_rule = "";
	var tmp_value = 0;

	if(validSettings()){
		var wan_value_str = "";
		for(var i = 0; i < wan_num; i++){
			for(var j = 0; j < bind_interface_array.length; j++){
				if(decodeURIComponent(Interface_Text_Value[i].if_text) == bind_interface_array[j].text){
					tmp_value = tmp_value | (1 << i);
					if(bind_interface_array[j].type == PortType_Value.Untagged)//untagged
						tmp_value = tmp_value | (1 << (i+8));
					break;
				}
			}
		}
		wan_value_str = (tmp_value + (0x10000)).toString(16).substr(-4).toUpperCase();

		var lan_value_str = "";
		tmp_value = 0;
		for(var i = wan_num; i < wan_num + lan_num; i++){
			var lan_index = i - wan_num;
			for(var j = 0; j < bind_interface_array.length; j++){
				if(decodeURIComponent(Interface_Text_Value[i].if_text) == bind_interface_array[j].text){
					tmp_value = tmp_value | (1 << lan_index);
					if(bind_interface_array[j].type == PortType_Value.Untagged)//untagged
						tmp_value = tmp_value | (1 << (lan_index+16));
					break;
				}
			}
		}
		lan_value_str = (tmp_value + (0x100000000)).toString(16).substr(-8).toUpperCase();

		var wireless_value_str_2g = "";
		tmp_value = 0;
		for(var i = wan_num + lan_num; i < wan_num + lan_num + wireless_num; i++){
			var wireless_index_2g = i - wan_num - lan_num;
			for(var j = 0; j < bind_interface_array.length; j++){
				if(decodeURIComponent(Interface_Text_Value[i].if_text) == bind_interface_array[j].text){
					tmp_value = tmp_value | (1 << wireless_index_2g);
					break;
				}
			}
		}
		wireless_value_str_2g = (tmp_value + (0x10000)).toString(16).substr(-4).toUpperCase();

		var wireless_value_str_5g = "";
		tmp_value = 0;
		for(var i = (wan_num + lan_num + wireless_num); i < (wan_num + lan_num + wireless_num*2); i++){
			var wireless_index_5g = i - wan_num - lan_num - wireless_num;
			for(var j = 0; j < bind_interface_array.length; j++){
				if(decodeURIComponent(Interface_Text_Value[i].if_text) == bind_interface_array[j].text){
					tmp_value = tmp_value | (1 << wireless_index_5g);
					break;
				}
			}
		}
		wireless_value_str_5g = (tmp_value + (0x10000)).toString(16).substr(-4).toUpperCase();

		vlan_rule = "<1>" + document.form.vlan_id.value + ">" + document.form.vlan_prio.value + ">" + 
					wan_value_str + ">" + lan_value_str + ">" + wireless_value_str_2g + ">" + wireless_value_str_5g + ">" + document.form.subnet_list.value+">1>0";
		vlan_rulelist = vlan_rulelist + vlan_rule;

		if($("#default_subnet_desc").css("display") != 'none')
			update_LanToLanRoute_array(default_subnet);
		else
			update_LanToLanRoute_array(document.form.subnet_list.value);

		showSettingTable(0);
		show_vlan_rulelist();

		if(vlan_rulelist.split('<').length == MAX_VLAN_NUM + 1){
			document.getElementById("add_vlan_btn").className = "add_btn_disabled";
			document.getElementById("add_vlan_btn").disabled = true;
		}
		else{
			document.getElementById("add_vlan_btn").className = "add_btn";
			document.getElementById("add_vlan_btn").disabled = false;
		}
	}
}

function show_vlan_rulelist(){
	var vlan_rulelist_row = vlan_rulelist.split('<');
	var vlan_rulelist_col = "";
	var code = "";
	var Untagged = parseInt(PortType_Value.Untagged, 2);
	var Tagged = parseInt(PortType_Value.Tagged, 2);
	var Override = parseInt(PortType_Value.Override, 2);

	vlan_used_vid_array = [];
	vlan1_wl_array = [];
	reset_all_pvid_array();
	clear_port_settings(); //Reset port used number.

	code +='<table width="98%" align="center" cellpadding="4" cellspacing="0"  class="list_table" id="vlan_rulelist_table">';
	
	if(vlan_rulelist == "")
		code +='<tr><td style="color:#FFCC00;" colspan="8">No vlan rules</td>'; /*untranslated*/
	else{
		for(var i = 0; i < vlan_rulelist_row.length; i++){
			if(vlan_rulelist_row[i].length == 0)
				continue;

			code +='<tr id="row'+i+'">';
			vlan_rulelist_col = vlan_rulelist_row[i].split('>');
			var wid = [12, 7, 7, 0, 0, 0, 21, 19];
			var wired_inf = "";
			var wireless_inf = "";
			var port_type_value = 0;

			for(var j = 0; j < vlan_rulelist_col.length - 2; j++){
				if( j == 0 ){
					vlan_enable_array.push(vlan_rulelist_col[0]);
					code +='<td style="width:'+wid[j]+'%">';
					if(vlan_rulelist_col[1] != "1"){
						code += '<div class="left" style="width:74px; float:left; cursor:pointer;" id="vlan_rule_enable'+ i + '"></div>';
						code += '<div class="clear"></div>';
					}
					code += '</td>';
				}
				else if( j == 3 || j == 4 ){ //Wired Interfaces
					if( j == 3 && vlan_rulelist_col[3] != "0"){ //WAN
						var wan_value = parseInt(vlan_rulelist_col[3].slice(-2), 16);
						for(var k = 0; k < wan_num; k++){
							var isMember = (wan_value >> k) & 1;
							if(isMember){
								if( wired_inf != "")
									wired_inf += ", ";
								wired_inf = wired_inf + htmlEncode_decodeURI(Interface_Text_Value[k].if_text);
							}
						}
					}
					else if( j == 4 && vlan_rulelist_col[4] != "0"){ //LAN
						var lan_value = parseInt(vlan_rulelist_col[4].slice(-4), 16);
						var untagged_value = parseInt(vlan_rulelist_col[4].slice(0, 4), 16);
						for(var k = 0; k < lan_num; k++){
							var isMember = (lan_value >> k) & 1;
							var isUntagged = (untagged_value >> k) & 1;
							var is_iptv_port = 0;
							var is_wan_port = 0;

							if(isMember){
								if( wired_inf != "")
									wired_inf += "<br>";

								for(var m = 0; m < iptv_lanport_list.length; m++){
									if(Interface_Text_Value[wan_num+k].value == iptv_lanport_list[m])
										is_iptv_port = 1;
								}

								if(lan_as_wan && Interface_Text_Value[wan_num+k].value == wans_lanport)
									is_wan_port = 1;

								if(is_iptv_port || is_wan_port){
									if(isUntagged)
										wired_inf = wired_inf + "<span style='color:#2F3A3E;' title='<#TBVLAN_IPTVPort_Hint#>'>" + htmlEncode_decodeURI(Interface_Text_Value[wan_num+k].if_text) + "<sup>U</sup></span>";
									else
										wired_inf = wired_inf + "<span style='color:#2F3A3E;' title='<#TBVLAN_IPTVPort_Hint#>'>" + htmlEncode_decodeURI(Interface_Text_Value[wan_num+k].if_text) + "<sup>T</sup></span>";
								}
								else{
									if(isUntagged)
										wired_inf = wired_inf + htmlEncode_decodeURI(Interface_Text_Value[wan_num+k].if_text) + "<sup>U</sup>";
									else
										wired_inf = wired_inf + "<span style='color:#00c6ff;'>" + htmlEncode_decodeURI(Interface_Text_Value[wan_num+k].if_text)+ "<sup>T</sup></span>";
								}


								//Add PVID to lanx_pvid_array
								add_to_pvid_array(Interface_Text_Value[wan_num+k].value.slice(3, 4), vlan_rulelist_col[1]);
							}
						}
					}
					if( j == 4)
						code +='<td style="word-break:break-all">'+ wired_inf +'</td>';
				}
				else if(j == 5 || j == 6){ //Wireless Interfaces
					if(vlan_rulelist_col[j] != "0"){
						var wl_value = parseInt(vlan_rulelist_col[j].slice(-2), 16);
						for(var k = 0; k < wireless_num; k++){
							var isMember = (wl_value >> k) & 1;
							var is_cp_fbwifi = 1;

							if(isMember){
								for(var m = 0; m < vlan_wl_array.length; m++){
									if(j == 5 && Interface_Text_Value[wan_num + lan_num + k].value == vlan_wl_array[m])
										is_cp_fbwifi = 0;
									else if(j == 6 && Interface_Text_Value[wan_num + lan_num + wireless_num + k].value == vlan_wl_array[m])
										is_cp_fbwifi = 0;
								}

								if( wireless_inf != "")
									wireless_inf += "<br>";

								if( j == 5){
									if(is_cp_fbwifi)
										wireless_inf = wireless_inf + "<span style='color:#2F3A3E;' title='This interface is used by Captive Portal or Free Wifi and is disabled in this VLAN.'>" + htmlEncode_decodeURI(Interface_Text_Value[wan_num + lan_num + k].if_text) + "</span>"; //untranslated
									else
										wireless_inf = wireless_inf + htmlEncode_decodeURI(Interface_Text_Value[wan_num + lan_num + k].if_text);

									if(i != 1)
										remove_from_inf_list(Interface_Text_Value[wan_num + lan_num + k].value);
									else
										vlan1_wl_array.push(Interface_Text_Value[wan_num + lan_num + k].value);
								}
								else{
									if(is_cp_fbwifi)
										wireless_inf = wireless_inf + "<span style='color:#2F3A3E;' title='This interface is used by Captive Portal or Free Wifi and is disabled in this VLAN.'>" + htmlEncode_decodeURI(Interface_Text_Value[wan_num + lan_num + wireless_num + k].if_text) + "</span>";//untranslated
									else
										wireless_inf = wireless_inf + htmlEncode_decodeURI(Interface_Text_Value[wan_num + lan_num + wireless_num + k].if_text);

									if(i != 1)
										remove_from_inf_list(Interface_Text_Value[wan_num + lan_num + wireless_num + k].value);
									else
										vlan1_wl_array.push(Interface_Text_Value[wan_num + lan_num + wireless_num + k].value);
								}
							}
						}
					}
					if(j == 6)
						code +='<td style="width:'+wid[j]+'%; word-break:break-all">'+ wireless_inf +'</td>';
				}
				else{
					if( j == 1)//VID
						vlan_used_vid_array.push(vlan_rulelist_col[1]);
					if(j == 7 && vlan_rulelist_col[7] == "default"){//subnet
						code +='<td style="width:'+wid[j]+'%">' + default_subnet + " (<#Setting_factorydefault_value#>)" + '</td>';
					}
					else
						code +='<td style="width:'+wid[j]+'%">'+ vlan_rulelist_col[j] +'</td>';
				}
			}

			if(i == 1){//default
				code +='<td colspan="2" style="width:14%"><input class="edit_btn" onclick="edit_vlanrule(this);" value=""/></td>';
			}
			else{
				code +='<td style="width:7%"><input class="edit_btn" onclick="edit_vlanrule(this);" value=""/></td>';
				code +='<td style="width:7%"><input class="remove_btn" onclick="del_vlanrule(this);" value=""/></td>';
			}
			code += '</tr>';
		}
	}
	code +='</table>';
	document.getElementById('vlan_rulelist_Block').innerHTML = code;

	for(var i = 0; i < vlan_rulelist_row.length; i++){
		if(vlan_rulelist_row[i].length == 0)
			continue;
		vlan_rulelist_col = vlan_rulelist_row[i].split('>');
		var obj_id = "#vlan_rule_enable" + i;

		$(obj_id).iphoneSwitch(vlan_rulelist_col[0],
			function(index) {
				enable_vlan_rule(index, "1");
			},
			function(index) {
				enable_vlan_rule(index, "0");
			}
		);
	}

	update_pvid_options();

	if(vlan_rulelist_row.length == MAX_VLAN_NUM + 1){
		document.getElementById("add_vlan_btn").className = "add_btn_disabled";
		document.getElementById("add_vlan_btn").disabled = true;
	}
	else{
		document.getElementById("add_vlan_btn").className = "add_btn";
		document.getElementById("add_vlan_btn").disabled = false;
	}
}

function del_vlanrule(r){
	var vlan_rulelist_row = vlan_rulelist.split('<');
	var vlan_rulelist_table = document.getElementById('vlan_rulelist_table');
	var index = r.parentNode.parentNode.rowIndex;
	var vlan_rulelist_row_index = index + 1;
	var wl_inf_str = vlan_rulelist_table.rows[index].cells[4].innerHTML;
	var wl_inf_array = wl_inf_str.split('<br>');

	for(var i = 0; i < wl_inf_array.length; i++){//Add wireless interface back to all_interfaces_array
		add_to_inf_list(wl_inf_array[i]);
		vlan1_wl_array.push(inf_text_to_value(wl_inf_array[i]));
		update_vlan1_rule();
		vlan_rulelist_row = vlan_rulelist.split('<');
	}

	vlan_rulelist_row.splice(vlan_rulelist_row_index, 1);
	vlan_rulelist_table.deleteRow(index);

	//update vlan_rulelist
	vlan_rulelist = vlan_rulelist_row.join("<");
	show_vlan_rulelist();

	if(vlan_rulelist_row.length == MAX_VLAN_NUM + 1){
		document.getElementById("add_vlan_btn").className = "add_btn_disabled";
		document.getElementById("add_vlan_btn").disabled = true;
	}
	else{
		document.getElementById("add_vlan_btn").className = "add_btn";
		document.getElementById("add_vlan_btn").disabled = false;
	}
}

function edit_vlanrule(r){
	var vlan_rulelist_row = vlan_rulelist.split('<');
	var vlan_rulelist_table = document.getElementById('vlan_rulelist_table');
	selected_row = r.parentNode.parentNode.rowIndex;
	var vlan_rulelist_col = vlan_rulelist_row[selected_row + 1].split('>');
	var binding_list = "";

	clear_port_settings();
	document.form.vlan_id.value = vlan_rulelist_table.rows[selected_row].cells[1].innerHTML;
	document.form.vlan_prio.value = vlan_rulelist_table.rows[selected_row].cells[2].innerHTML;

	if(vlan_rulelist_table.rows[selected_row].cells[3].innerHTML != ""){
		var lan_binding_array = vlan_rulelist_table.rows[selected_row].cells[3].innerHTML.split("<br>");
		for(var i = 0; i < lan_binding_array.length; i++){
			if(lan_binding_array[i].indexOf("<span") != -1)
				lan_binding_array[i] = lan_binding_array[i].substring(lan_binding_array[i].indexOf(">") + 1, lan_binding_array[i].indexOf("<sup"));
			else
				lan_binding_array[i] = lan_binding_array[i].substring(lan_binding_array[i].indexOf("LAN"), lan_binding_array[i].indexOf("<sup"));
		}
		binding_list = lan_binding_array.join("<br>");
	}

	if(binding_list != "" && vlan_rulelist_table.rows[selected_row].cells[4].innerHTML != "")
		binding_list += "<br>"

	binding_list += htmlEnDeCode.htmlDecode(vlan_rulelist_table.rows[selected_row].cells[4].innerHTML);
	var binding_list_array = binding_list.split("<br>");
	for(var i = 0; i < binding_list_array.length; i++){
		if(binding_list_array[i].indexOf("<span") != -1){
			binding_list_array[i] = binding_list_array[i].substring(binding_list_array[i].indexOf(">") + 1, binding_list_array[i].indexOf("</span"));
		}
	}

	var index = 0;
	for(var j = 0; j < binding_list_array.length; j++){
		for( var k = 0; k < Interface_Text_Value.length; k++){
			if( binding_list_array[j] == decodeURIComponent(Interface_Text_Value[k].if_text)){
				var value = -1;
				if( k >= 0 && k < wan_num){ //WAN
					index = k;
					value = parseInt(vlan_rulelist_col[3].slice(0, 2), 16) >> index & 1;
				}
				else if( k >= wan_num && k < wan_num + lan_num){ //LAN
					index = k - wan_num;
					value = parseInt(vlan_rulelist_col[4].slice(0, 4), 16) >> index & 1;
				}
				else if( k >= (wan_num + lan_num) && k < (wan_num + lan_num + wireless_num)){ // 2.4G
					value = 1; //always untagged
				}
				else if( k >= (wan_num + lan_num + wireless_num) && k < (wan_num + lan_num + wireless_num*2)){ // 5G
					value = 1; //always untagged
				}
				if( value == 1)
					Interface_Text_Value[k].type = PortType_Value.Untagged;
				else if ( value == 0)
					Interface_Text_Value[k].type = PortType_Value.Tagged;

				var inf_element = {"text": decodeURIComponent(Interface_Text_Value[k].if_text),
								   "value": Interface_Text_Value[k].value,
								   "type": Interface_Text_Value[k].type};
				bind_interface_array.push(inf_element);
			}
		}
	}

	if(vlan_rulelist_table.rows[selected_row].cells[5].innerHTML.indexOf(default_subnet) != -1){//Default VLAN
		document.getElementById("vlan_id").style.display = "none";
		document.getElementById("default_vlan_id").style.display = "";
		document.getElementById("default_vlan_id").innerHTML = vlan_rulelist_table.rows[selected_row].cells[1].innerHTML;
		document.getElementById("vlan_prio").style.display = "none";
		document.getElementById("default_vlan_prio").style.display = "";
		document.getElementById("default_vlan_prio").innerHTML = vlan_rulelist_table.rows[selected_row].cells[2].innerHTML;
		document.getElementById("subnet_sel").style.display = "none";
		document.getElementById("default_subnet_desc").style.display = "";
	}
	else{
		document.getElementById("vlan_id").style.display = "";
		document.getElementById("default_vlan_id").style.display = "none";
		document.getElementById("vlan_prio").style.display = "";
		document.getElementById("default_vlan_prio").style.display = "none";
		document.getElementById("subnet_sel").style.display = "";
		document.getElementById("default_subnet_desc").style.display = "none";
	}

	showSettingTable(1, 1);
}

function find_portType_text(value){
	for(var i = 0; i < PortType_Text_Value.length; i++){
		if(PortType_Text_Value[i].value == value)
			return PortType_Text_Value[i].type_text;
	}
}

function find_portType_value(text){
	for(var i = 0; i < PortType_Text_Value.length; i++){
		if(PortType_Text_Value[i].type_text == text)
			return PortType_Text_Value[i].value;
	}
}

function change_vlan_rule(){
	var vlan_rulelist_row = vlan_rulelist.split('<');
	var vlan_rulelist_col = vlan_rulelist_row[selected_row + 1].split('>');
	var tmp_value = 0;
	var wired_interface = "";
	var wireless_interface = "";

	if(validSettings(selected_row)){
		document.getElementById('vlan_rulelist_table').rows[selected_row].cells[1].innerHTML = document.form.vlan_id.value;
		vlan_rulelist_col[1] = document.form.vlan_id.value;
		document.getElementById('vlan_rulelist_table').rows[selected_row].cells[2].innerHTML = document.form.vlan_prio.value;
		vlan_rulelist_col[2] = document.form.vlan_prio.value;

		var wan_value_str = "";
		for(var i = 0; i < wan_num; i++){
			for(var j = 0; j < bind_interface_array.length; j++){
				if(decodeURIComponent(Interface_Text_Value[i].if_text) == bind_interface_array[j].text){
					if(wired_interface != "")
						wired_interface += ", ";
					wired_interface += htmlEncode_decodeURI(Interface_Text_Value[i].if_text);
					tmp_value = tmp_value | (1 << i);
					if(bind_interface_array[j].type == PortType_Value.Untagged)//untagged
						tmp_value = tmp_value | (1 << (i+8));
					break;
				}
			}
		}
		wan_value_str = (tmp_value + (0x10000)).toString(16).substr(-4).toUpperCase();
		vlan_rulelist_col[3] = wan_value_str;

		var lan_value_str = "";
		tmp_value = 0;
		for(var i = wan_num; i < wan_num + lan_num; i++){
			var lan_index = i - wan_num;
			for(var j = 0; j < bind_interface_array.length; j++){
				if(decodeURIComponent(Interface_Text_Value[i].if_text) == bind_interface_array[j].text){
					if(wired_interface != "")
						wired_interface += ", ";
					wired_interface += htmlEncode_decodeURI(Interface_Text_Value[i].if_text);
					tmp_value = tmp_value | (1 << lan_index);
					if(bind_interface_array[j].type == PortType_Value.Untagged)//untagged
						tmp_value = tmp_value | (1 << (lan_index+16));
					break;
				}
			}
		}
		lan_value_str = (tmp_value + (0x100000000)).toString(16).substr(-8).toUpperCase();
		vlan_rulelist_col[4] = lan_value_str;

		var wireless_value_str_2g = "";
		tmp_value = 0;
		for(var i = wan_num + lan_num; i < wan_num + lan_num + wireless_num; i++){
			var wireless_index_2g = i - wan_num - lan_num;
			for(var j = 0; j < bind_interface_array.length; j++){
				if(decodeURIComponent(Interface_Text_Value[i].if_text) == bind_interface_array[j].text){
					if(wireless_interface != "")
						wireless_interface += ", ";
					wireless_interface += htmlEncode_decodeURI(Interface_Text_Value[i].if_text);
					tmp_value = tmp_value | (1 << wireless_index_2g);
					break;
				}
			}
		}
		wireless_value_str_2g = (tmp_value + (0x10000)).toString(16).substr(-4).toUpperCase();
		vlan_rulelist_col[5] = wireless_value_str_2g;

		var wireless_value_str_5g = "";
		tmp_value = 0;
		for(var i = (wan_num + lan_num + wireless_num); i < (wan_num + lan_num + wireless_num*2); i++){
			var wireless_index_5g = i - wan_num - lan_num - wireless_num;
			for(var j = 0; j < bind_interface_array.length; j++){
				if(decodeURIComponent(Interface_Text_Value[i].if_text) == bind_interface_array[j].text){
					if(wireless_interface != "")
						wireless_interface += ", ";
					wireless_interface += htmlEncode_decodeURI(Interface_Text_Value[i].if_text);
					tmp_value = tmp_value | (1 << wireless_index_5g);
					break;
				}
			}
		}
		wireless_value_str_5g = (tmp_value + (0x10000)).toString(16).substr(-4).toUpperCase();
		vlan_rulelist_col[6] = wireless_value_str_5g;

		document.getElementById('vlan_rulelist_table').rows[selected_row].cells[3].innerHTML = wired_interface;
		document.getElementById('vlan_rulelist_table').rows[selected_row].cells[4].innerHTML = wireless_interface;
		if(selected_row != 0){
			var subnet_list = document.form.subnet_list;
			document.getElementById('vlan_rulelist_table').rows[selected_row].cells[5].innerHTML = subnet_list.options[subnet_list.selectedIndex].value;
			vlan_rulelist_col[7] = subnet_list.options[subnet_list.selectedIndex].value;
		}
		vlan_rulelist_row[selected_row + 1] = vlan_rulelist_col.join(">");
		vlan_rulelist = vlan_rulelist_row.join("<");

		if($("#default_subnet_desc").css("display") != 'none')
			update_LanToLanRoute_array(default_subnet);
		else
			update_LanToLanRoute_array(document.form.subnet_list.value);

		showSettingTable(0);
		show_vlan_rulelist();
	}
}

function enable_vlan_rule(index, enable){
	var index_int = parseInt(index);
	var vlan_rulelist_row = vlan_rulelist.split("<");
	var vlan_rulelist_col = vlan_rulelist_row[index_int].split('>');

	vlan_enable_array.splice(index-1, 1, enable);
	vlan_rulelist_col[0] = enable;
	vlan_rulelist_row[index_int] = vlan_rulelist_col.join('>');
	vlan_rulelist = vlan_rulelist_row.join('<');
}

function generate_sunbet_options(select){
	select.length = 0;
	subnet_array = [];

	for(var i = 1; i < subnet_rulelist_row.length; i++){
		var subnet_rulelist_col = subnet_rulelist_row[i].split('>');
		var option = document.createElement("option");
		var option_text = "";
		var subnet = "";

		subnet = subnet_rulelist_col[0] + '/' + netmask_to_bits(subnet_rulelist_col[1]);
		subnet_array.push(subnet);

		if(i > 1){
			option_text = subnet_rulelist_col[0] + '/' + netmask_to_bits(subnet_rulelist_col[1]);
			option.value = option_text;
			option.text = option_text;
			select.add(option);
		}
	}

	if(select.length < 7){
		var option = document.createElement("option");
		option.value = "new";
		option.text = "<#VLAN_New_Subnet#>";
		select.add(option);
	}	
}

function show_subnet_edit(){
	var select =  document.form.selSubnet;

	generate_sunbet_options(select);

	$("#subnet_div").fadeIn(300);
	switchSubnetValue(document.form.subnet_list.options[document.form.subnet_list.selectedIndex].value);
}

function hide_subnet_edit(){
	$("#subnet_div").fadeOut(300);
}

function switchSubnetValue(selectSubnet) {
	var select =  document.form.selSubnet;
	var getwayIP = "";

	for(var i = 0; i < select.length; i++){
		if( select.options[i].value == selectSubnet){
			select.selectedIndex = i;

			break;
		}
	}

	document.form.tGatewayIP.value = "";
	document.form.radioDHCPEnable[0].checked = true;
	document.form.tSubnetMask.value = "";
	document.form.tDHCPStart.value = "";
	document.form.tDHCPEnd.value = "";
	document.form.tLeaseTime.value = "86400";

	getwayIP =  selectSubnet.substring(0, selectSubnet.indexOf('/'));

	if(selectSubnet != "new"){
		for(var i = 1; i < 9; i +=1 ) {
			if(subnet_rulelist_row[i] != undefined) {
				var subnet_rulelist_col = subnet_rulelist_row[i].split('>');
				if(subnet_rulelist_col[0] != undefined && getwayIP == subnet_rulelist_col[0]) {
					document.form.tGatewayIP.value = subnet_rulelist_col[0];
					old_tGatewayIP = document.form.tGatewayIP.value;
					if (subnet_rulelist_col[2] === "0") {
						document.form.radioDHCPEnable[1].checked = true;
					}
					document.form.tSubnetMask.value = subnet_rulelist_col[1];
					document.form.tDHCPStart.value = subnet_rulelist_col[3];
					document.form.tDHCPEnd.value = subnet_rulelist_col[4];
					document.form.tLeaseTime.value = subnet_rulelist_col[5];
				}
			}
		}
	}
}

function calculatorIPPoolRange() {
	var gatewayIPArray = document.form.tGatewayIP.value.split(".");
	var netMaskArray = document.form.tSubnetMask.value.split(".");
	var ipPoolStartArray  = new Array();
	var ipPoolEndArray  = new Array();
	var ipPoolStart = "";
	var ipPoolEnd = "";

	ipPoolStartArray[0] = (gatewayIPArray[0] & 0xFF) & (netMaskArray[0] & 0xFF);
	ipPoolStartArray[1] = (gatewayIPArray[1] & 0xFF) & (netMaskArray[1] & 0xFF);
	ipPoolStartArray[2] = (gatewayIPArray[2] & 0xFF) & (netMaskArray[2] & 0xFF);
	ipPoolStartArray[3] = (gatewayIPArray[3] & 0xFF) & (netMaskArray[3] & 0xFF);
	ipPoolStartArray[3] += 1;

	ipPoolEndArray[0] = (gatewayIPArray[0] & 0xFF) | (~netMaskArray[0] & 0xFF);
	ipPoolEndArray[1] = (gatewayIPArray[1] & 0xFF) | (~netMaskArray[1] & 0xFF);
	ipPoolEndArray[2] = (gatewayIPArray[2] & 0xFF) | (~netMaskArray[2] & 0xFF);
	ipPoolEndArray[3] = (gatewayIPArray[3] & 0xFF) | (~netMaskArray[3] & 0xFF);
	ipPoolEndArray[3] -= 1;

	ipPoolStart = ipPoolStartArray[0] + "." + ipPoolStartArray[1] + "." + ipPoolStartArray[2] + "." + ipPoolStartArray[3];
	if(inet_network(ipPoolStart) <= inet_network(document.form.tGatewayIP.value)) {
		ipPoolStart = ipPoolStartArray[0] + "." + ipPoolStartArray[1] + "." + ipPoolStartArray[2] + "." + (parseInt(ipPoolStartArray[3]) + 1);
	}
	ipPoolEnd = ipPoolEndArray[0] + "." + ipPoolEndArray[1] + "." + ipPoolEndArray[2] + "." + ipPoolEndArray[3];

	return ipPoolStart + ">" + ipPoolEnd;
}

function checkIPLegality() {
	//check IP legal
	if(document.form.tGatewayIP.value !== "") {
		if(!validator.isLegalIP(document.form.tGatewayIP)) {
			document.form.tGatewayIP.focus();
			document.form.tGatewayIP.select();
			return false;
		}
	}

	//setting IP pool range
	if(document.form.tGatewayIP.value !== "" && document.form.tSubnetMask.value !== "") {
		var ipPoolRangeArray = calculatorIPPoolRange().split(">");
		document.form.tDHCPStart.value = ipPoolRangeArray[0];
		document.form.tDHCPEnd.value = ipPoolRangeArray[1];
	}
}

function checkMaskLegality() {
	//check IP legal
	if(document.form.tSubnetMask.value !== "") {
		if(!validator.isLegalMask(document.form.tSubnetMask)) {
			return false;
		}
	}
	
	//setting IP pool range
	if(document.form.tGatewayIP.value !== "" && document.form.tSubnetMask.value !== "") {
		var ipPoolRangeArray = calculatorIPPoolRange().split(">");
		document.form.tDHCPStart.value = ipPoolRangeArray[0];
		document.form.tDHCPEnd.value = ipPoolRangeArray[1];
	}
}

function checkGatewayIP() {
	var lanIPAddr = document.form.tGatewayIP.value;
	var lanNetMask = document.form.tSubnetMask.value;
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

	//2.check Lan IP
	ipConflict = checkIPConflict("LAN", lanIPAddr, lanNetMask);
	if(ipConflict.state) {
		alertMsg("LAN", ipConflict.ipAddr, ipConflict.netLegalRangeStart, ipConflict.netLegalRangeEnd);
		return false;
	}

	//3.check PPTP
	if(pptpd_support) {
		ipConflict = checkIPConflict("PPTP", lanIPAddr, lanNetMask);
		if(ipConflict.state) {
			alertMsg("PPTP", ipConflict.ipAddr, ipConflict.netLegalRangeStart, ipConflict.netLegalRangeEnd);
			return false;
		}
	}

	//4.check OpenVPN
	if(openvpnd_support) {
		ipConflict = checkIPConflict("OpenVPN", lanIPAddr, lanNetMask);
		if(ipConflict.state) {
			alertMsg("OpenVPN", ipConflict.ipAddr, ipConflict.netLegalRangeStart, ipConflict.netLegalRangeEnd);
			return false;
		}
	}

	//5.check existed Subnet IP address
	for(var i = 2; i < subnet_rulelist_row.length; i++) { //skip default subnet, because it's checked in 'LAN' case.
		var subnet_rulelist_col = subnet_rulelist_row[i].split('>');
		ipConflict = checkIPConflict("SUBNET", lanIPAddr, lanNetMask, subnet_rulelist_col[0], subnet_rulelist_col[1]);
		if(ipConflict.state) {
			alertMsg("Subnet", ipConflict.ipAddr, ipConflict.netLegalRangeStart, ipConflict.netLegalRangeEnd);
			return false;
		}
	}

	return true;
}

function validSubnetForm() {
	if(old_tGatewayIP != document.form.tGatewayIP.value){
		if(!validator.isLegalIP(document.form.tGatewayIP)) {
			document.form.tGatewayIP.focus();
			document.form.tGatewayIP.select();
			return false;
		}

		if(!checkGatewayIP()) {
			document.form.tGatewayIP.focus();
			document.form.tGatewayIP.select();
			return false;
		}
	}

	if(document.form.tSubnetMask.value == "") {
		alert("<#JS_fieldblank#>");
		document.form.tSubnetMask.focus();
		document.form.tSubnetMask.select();
		return false;
	}
	else if(!validator.isLegalMask(document.form.tSubnetMask)) {
		return false;
	}

	if(document.form.tDHCPStart.value == "") {
		alert("<#JS_fieldblank#>");
		document.form.tDHCPStart.focus();
		document.form.tDHCPStart.select();
		return false;
	}
	else if(document.form.tDHCPEnd.value == "") {
		alert("<#JS_fieldblank#>");
		document.form.tDHCPEnd.focus();
		document.form.tDHCPEnd.select();
		return false;
	}
	else {
		//1.check IP whether is legal or not
		if(!validator.isLegalIP(document.form.tDHCPStart)) {
			document.form.tDHCPStart.focus();
			document.form.tDHCPStart.select();
			return false;
		}
		else if(!validator.isLegalIP(document.form.tDHCPEnd)) {
			document.form.tDHCPEnd.focus();
			document.form.tDHCPEnd.select();
			return false;
		}

		//2.check IP pool range is legal
		var ipGateway = inet_network(document.form.tGatewayIP.value);
		var ipPoolStart = inet_network(document.form.tDHCPStart.value);
		var ipPoolEnd = inet_network(document.form.tDHCPEnd.value);
		var ipPoolRangeArray = calculatorIPPoolRange().split(">");
		var ipPoolLegalStart = inet_network(ipPoolRangeArray[0]);
		var ipPoolLegalEnd = inet_network(ipPoolRangeArray[1]);
		if(ipPoolLegalStart > ipPoolStart || ipPoolLegalEnd < ipPoolStart) {
			alert(document.form.tDHCPStart.value + " <#JS_validip#>");
			document.form.tDHCPStart.focus();
			document.form.tDHCPStart.select();
			return false;
		}
		else if(ipPoolLegalEnd < ipPoolEnd || ipPoolLegalStart > ipPoolEnd) {
			alert(document.form.tDHCPEnd.value + " <#JS_validip#>");
			document.form.tDHCPEnd.focus();
			document.form.tDHCPEnd.select();
			return false;
		}

		//3.check whether the End IP > Start IP or not
		if(ipPoolStart > ipPoolEnd) {
			alert(alert_over + document.form.tDHCPStart.value);
			document.form.tDHCPEnd.focus();
			document.form.tDHCPEnd.select();
			return false;
		}
		
		//4.check IP pool start/end whether conflic gateway IP
		if(ipPoolStart === ipGateway) {
			alert("Conflict with Gateway IP " + document.form.tGatewayIP.value);
			document.form.tDHCPStart.focus();
			document.form.tDHCPStart.select();
			return false;
		}
		else if(ipPoolEnd === ipGateway) {
			alert("Conflict with Gateway IP " + document.form.tGatewayIP.value);
			document.form.tDHCPEnd.focus();
			document.form.tDHCPEnd.select();
			return false;
		}	
	}

	if(!validator.numberRange(document.form.tLeaseTime, 120, 604800)) {
		return false;
	}

	return true;
}

function handleSubnetRulelist() {
	var selSubnet = document.form.selSubnet.value;
	var getwayIP = selSubnet.substring(0, document.form.selSubnet.value.indexOf('/'));

	if(selSubnet == "default" || (getwayIP!= "" && subnet_rulelist.search(getwayIP) !== -1)) {
		subnet_rulelist = "";
		for(var i = 1; i < subnet_rulelist_row.length; i++ ) {
			var subnet_rulelist_col = subnet_rulelist_row[i].split('>');
			if(i == 1 && selSubnet == "default"){
				subnet_rulelist += "<";
				subnet_rulelist += document.form.tGatewayIP.value;
				subnet_rulelist += ">";
				subnet_rulelist += document.form.tSubnetMask.value;
				subnet_rulelist += ">";
				subnet_rulelist += (document.form.radioDHCPEnable[0].checked)? "1":"0";
				subnet_rulelist += ">";
				subnet_rulelist += document.form.tDHCPStart.value;
				subnet_rulelist += ">";
				subnet_rulelist += document.form.tDHCPEnd.value;
				subnet_rulelist += ">";
				subnet_rulelist += document.form.tLeaseTime.value;
				subnet_rulelist += ">";
				subnet_rulelist += subnet_rulelist_col[6];
				subnet_rulelist += ">";
				subnet_rulelist += subnet_rulelist_col[7];
				subnet_rulelist += ">";
				subnet_rulelist += subnet_rulelist_col[8];
				subnet_rulelist += ">";
				subnet_rulelist += subnet_rulelist_col[9];
				subnet_rulelist += ">";
				subnet_rulelist += subnet_rulelist_col[10];
				subnet_rulelist += ">";
				subnet_rulelist += subnet_rulelist_col[11];
			}
			else if(i > 1 && subnet_rulelist_col[0] == getwayIP) {
				subnet_rulelist += "<";
				subnet_rulelist += document.form.tGatewayIP.value;
				subnet_rulelist += ">";
				subnet_rulelist += document.form.tSubnetMask.value;
				subnet_rulelist += ">";
				subnet_rulelist += (document.form.radioDHCPEnable[0].checked)? "1":"0";
				subnet_rulelist += ">";
				subnet_rulelist += document.form.tDHCPStart.value;
				subnet_rulelist += ">";
				subnet_rulelist += document.form.tDHCPEnd.value;
				subnet_rulelist += ">";
				subnet_rulelist += document.form.tLeaseTime.value;
				subnet_rulelist += ">";
				subnet_rulelist += subnet_rulelist_col[6];
				subnet_rulelist += ">";
				subnet_rulelist += subnet_rulelist_col[7];
				subnet_rulelist += ">";
				subnet_rulelist += subnet_rulelist_col[8];
				subnet_rulelist += ">";
				subnet_rulelist += subnet_rulelist_col[9];
				subnet_rulelist += ">";
				subnet_rulelist += subnet_rulelist_col[10];
				subnet_rulelist += ">";
				subnet_rulelist += subnet_rulelist_col[11];
			}
			else {
				subnet_rulelist += "<";
				subnet_rulelist += subnet_rulelist_row[i];
			}
		}
	}
	else {
		subnet_rulelist += "<";
		subnet_rulelist += document.form.tGatewayIP.value;
		subnet_rulelist += ">";
		subnet_rulelist += document.form.tSubnetMask.value;
		subnet_rulelist += ">";
		subnet_rulelist += (document.form.radioDHCPEnable[0].checked)? "1":"0";
		subnet_rulelist += ">";
		subnet_rulelist += document.form.tDHCPStart.value;
		subnet_rulelist += ">";
		subnet_rulelist += document.form.tDHCPEnd.value;
		subnet_rulelist += ">";
		subnet_rulelist += document.form.tLeaseTime.value;
		subnet_rulelist += ">>>>";
		subnet_rulelist += "0";
		subnet_rulelist += ">";
	}
}

function update_subnet_rulelist(){
	subnet_rulelist_row = subnet_rulelist.split('<');
}

function saveSubnetSettings(){
	if(validSubnetForm()){
		var gateway_netmask = document.form.tGatewayIP.value + "/" + netmask_to_bits(document.form.tSubnetMask.value);

		handleSubnetRulelist();
		update_subnet_rulelist();
		parse_LanToLanRoute_to_object();
		get_LanToLanRoute(gateway_netmask);
		generate_LanToLanRoute_options();
		show_LanToLanRoute_list();
		hide_subnet_edit();
		generate_sunbet_options(document.form.subnet_list);
		for(var i = 0; i < document.form.subnet_list.length; i++){
			if(document.form.subnet_list.options[i].value == gateway_netmask)
				document.form.subnet_list.selectedIndex = i;
		}
	}
}

function parse_vlan_if_list(){
	var vlan_if_array = vlan_if_list.split(">");
	var inf_value = "";
	var tmpVal = 0;
	
	//WAN
	tmpVal = parseInt(vlan_if_array[0], 16);
	for(var i = 0; i < 8; i++){
		if((tmpVal >> i & 1) == 1){
			inf_value = "wan" + i;
			all_interfaces_array.push(inf_value);
		}
	}

	//LAN
	tmpVal = parseInt(vlan_if_array[1], 16);
	for(var i = 0; i < 16; i++){
		if((tmpVal >> i & 1) == 1){
			inf_value = "lan" + (i + 1);
			all_interfaces_array.push(inf_value);
		}
	}

	//Wireless 2.4G
	tmpVal = parseInt(vlan_if_array[2], 16);
	for(var i = 0; i < 16; i++){
		if((tmpVal >> i & 1) == 1){
			if(i == 0)
				inf_value = "wl0";
			else
				inf_value = "wl0." + i;

			all_interfaces_array.push(inf_value);
			vlan_wl_array.push(inf_value);
		}
	}

	//Wireless 5G
	tmpVal = parseInt(vlan_if_array[3], 16);
	for(var i = 0; i < 16; i++){
		if((tmpVal >> i & 1) == 1){
			if( i == 0)
				inf_value = "wl1";
			else
				inf_value = "wl1." + i;

			all_interfaces_array.push(inf_value);
			vlan_wl_array.push(inf_value);
		}
	}

	all_interfaces_array.sort();
}

function remove_from_inf_list(inf_value){
	for(var i = 0; i < all_interfaces_array.length; i++){
		if(all_interfaces_array[i] == inf_value)
			all_interfaces_array.splice(i, 1);
	}
}

function add_to_inf_list(inf_text){
	for(var i = 0; i < Interface_Text_Value.length; i++){
		if(decodeURIComponent(Interface_Text_Value[i].if_text) == inf_text){
			all_interfaces_array.push(Interface_Text_Value[i].value);
			break;
		}
	}
	all_interfaces_array.sort();
}

function reset_all_pvid_array(){
	lan1_pvid_array = [];
	lan2_pvid_array = [];
	lan3_pvid_array = [];
	lan4_pvid_array = [];
	lan5_pvid_array = [];
	lan6_pvid_array = [];
	lan7_pvid_array = [];
	lan8_pvid_array = [];
}

function add_to_pvid_array(lan_index, pvid){
	switch(lan_index){
		case "1":
			lan1_pvid_array.push(pvid);
			break;
		case "2":
			lan2_pvid_array.push(pvid);
			break;
		case "3":
			lan3_pvid_array.push(pvid);
			break;
		case "4":
			lan4_pvid_array.push(pvid);
			break;
		case "5":
			lan5_pvid_array.push(pvid);
			break;
		case "6":
			lan6_pvid_array.push(pvid);
			break;
		case "7":
			lan7_pvid_array.push(pvid);
			break;
		case "8":
			lan8_pvid_array.push(pvid);
			break;
	}
}

function update_pvid_options(){
	for(var lan_index = 1; lan_index <= 8; lan_index++){
		var obj_id = "lan" + lan_index + "_pvid_select";
		var select_obj = document.getElementById(obj_id);
		select_obj.options.length = 0;

		switch(lan_index){
			case 1:
				for(var i = 0; i < lan1_pvid_array.length; i++){
					var option = document.createElement("option");
					option.value = lan1_pvid_array[i];
					option.text = lan1_pvid_array[i];
					select_obj.add(option);
				}
				break;
			case 2:
				for(var i = 0; i < lan2_pvid_array.length; i++){
					var option = document.createElement("option");
					option.value = lan2_pvid_array[i];
					option.text = lan2_pvid_array[i];
					select_obj.add(option);
				}
				break;
			case 3:
				for(var i = 0; i < lan3_pvid_array.length; i++){
					var option = document.createElement("option");
					option.value = lan3_pvid_array[i];
					option.text = lan3_pvid_array[i];
					select_obj.add(option);
				}
				break;
			case 4:
				for(var i = 0; i < lan4_pvid_array.length; i++){
					var option = document.createElement("option");
					option.value = lan4_pvid_array[i];
					option.text = lan4_pvid_array[i];
					select_obj.add(option);
				}
				break;
			case 5:
				for(var i = 0; i < lan5_pvid_array.length; i++){
					var option = document.createElement("option");
					option.value = lan5_pvid_array[i];
					option.text = lan5_pvid_array[i];
					select_obj.add(option);
				}
				break;
			case 6:
				for(var i = 0; i < lan6_pvid_array.length; i++){
					var option = document.createElement("option");
					option.value = lan6_pvid_array[i];
					option.text = lan6_pvid_array[i];
					select_obj.add(option);
				}
				break;
			case 7:
				for(var i = 0; i < lan7_pvid_array.length; i++){
					var option = document.createElement("option");
					option.value = lan7_pvid_array[i];
					option.text = lan7_pvid_array[i];
					select_obj.add(option);
				}
				break;
			case 8:
				for(var i = 0; i < lan8_pvid_array.length; i++){
					var option = document.createElement("option");
					option.value = lan8_pvid_array[i];
					option.text = lan8_pvid_array[i];
					select_obj.add(option);
				}
				break;
		}		
	}

	show_pvid_list();
}

function show_pvid_list(){
	for(var lan_index = 1; lan_index <= 8; lan_index++){
		var obj_id = "lan" + lan_index + "_pvid_select";
		var select_obj = document.getElementById(obj_id);

		for(var i = 0; i < select_obj.length; i++){
			if(select_obj.options[i].value == vlan_pvid_list_array[lan_index - 1]){
				select_obj.selectedIndex = i;
				break;
			}
		}
	}
}

function set_pvid_list(){
	var lan1_pvid = document.form.lan1_pvid_select.value;
	var lan2_pvid = document.form.lan2_pvid_select.value;
	var lan3_pvid = document.form.lan3_pvid_select.value;
	var lan4_pvid = document.form.lan4_pvid_select.value;
	var lan5_pvid = document.form.lan5_pvid_select.value;
	var lan6_pvid = document.form.lan6_pvid_select.value;
	var lan7_pvid = document.form.lan7_pvid_select.value;
	var lan8_pvid = document.form.lan8_pvid_select.value;
	document.form.vlan_pvid_list.value = lan1_pvid + ">" + lan2_pvid + ">" + lan3_pvid + ">" + 
		lan4_pvid + ">" + lan5_pvid + ">" + lan6_pvid + ">" + lan7_pvid + ">" + lan8_pvid;
}

function isCurrentRoute(subnet){
	for(var i = 0; i < cur_LanToLanRoute.length; i++){
		if(subnet == cur_LanToLanRoute[i]){
			return 1;
		}
	}

	return 0;
}

function isDupOption(select, value){
	for(var i = 0; i < select.length; i++){
		if(select.options[i].value == value)
			return 1;
	}

	return 0;
}

function generate_LanToLanRoute_options(){
	var select = document.getElementById("other_subnet_select");
	var vid = document.form.vlan_id.value;

	select.length = 0;
	for(var i = 0; i < subnet_array.length; i++){
		var other_subnet =  document.createElement("option");;
		var subnet =  subnet_array[i];

		if(!isCurrentRoute(subnet) && !isDupOption(select, subnet)){
			if( ($("#default_subnet_desc").css("display") != 'none' && subnet == default_subnet) ||
				($("#subnet_sel").css("display") != 'none' && subnet == document.form.subnet_list.value) )
				continue;
			else{
				if(subnet == default_subnet){
					other_subnet.text = default_subnet + " (<#Setting_factorydefault_value#>)";
				}
				else{
					other_subnet.text = subnet;
				}
				other_subnet.value = subnet;
			}

			select.add(other_subnet);
		}
	}

	if(select.length == 0)
		document.getElementById("addSubnet_tr").style.display = "none";
	else
		document.getElementById("addSubnet_tr").style.display = "";
}

function show_LanToLanRoute_list(){
	var code = "";

	code +='<table width="97%" cellspacing="0" cellpadding="4" align="center" class="list_table" id="LanToLanRoute_list">';

	if(cur_LanToLanRoute.length == 0)
		code +="<tr><td style='color:#FFCC00;' colspan='3'>There is no LAN-to-LAN route.</td>";
	else{
		Object.keys(cur_LanToLanRoute).forEach(function(key){
			code +='<tr id="row'+key+'">';
			if(htmlEnDeCode.htmlEncode(cur_LanToLanRoute[key]) == default_subnet)
				code +='<td width="80%">'+ default_subnet + " (<#Setting_factorydefault_value#>)" +' </td>';
			else
				code +='<td width="80%">'+ htmlEnDeCode.htmlEncode(cur_LanToLanRoute[key]) +'</td>';
			code +='<td width="20%"><input class="remove_btn" onclick="del_LanToLanRoute(this);" value=""/></td>';
			code += '</tr>';
		});
	}
	code +='</table>';
	document.getElementById('lanToLanRoute_Block').innerHTML = code;
}

function add_LanToLanRoute(){
	var select = document.getElementById("other_subnet_select");

	cur_LanToLanRoute.push(document.form.other_subnet_select.value);
	show_LanToLanRoute_list();
	select.options.remove(select.selectedIndex);
	if(select.length == 0)
		document.getElementById("addSubnet_tr").style.display = "none";
}

function del_LanToLanRoute(r){
	var select = document.getElementById("other_subnet_select");
	var index = r.parentNode.parentNode.rowIndex;
	var LanToLanRoute_table = document.getElementById("LanToLanRoute_list");
	var del_route = htmlEnDeCode.htmlDecode(LanToLanRoute_table.rows[index].cells[0].innerHTML);

	if(del_route.indexOf(default_subnet) != -1)
		del_route = default_subnet;

	for(var i = 0; i < cur_LanToLanRoute.length; i++){
		if(del_route == cur_LanToLanRoute[i]){
			cur_LanToLanRoute.splice(i, 1);
			break;
		}
	}

	LanToLanRoute_table.deleteRow(index);
	generate_LanToLanRoute_options();
}
</script>
</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>
<div id="hiddenMask" class="popup_bg">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
		<tr>
		<td>
			<div class="drword" id="drword" style="height:110px;"><#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...
				<br/>
				<br/>
	    </div>
		  <div class="drImg"><img src="images/alertImg.png"></div>
			<div style="height:70px;"></div>
		</td>
		</tr>
	</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_TagBasedVLAN_Content.asp">
<input type="hidden" name="next_page" value="Advanced_TagBasedVLAN_Content.asp">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="reboot">
<input type="hidden" name="action_wait" value="<% get_default_reboot_time(); %>">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="vlan_enable" value='<% nvram_get("vlan_enable"); %>'>
<input type="hidden" name="vlan_rulelist" value="">
<input type="hidden" name="vlan_pvid_list" value="">
<input type="hidden" name="subnet_rulelist" value='<% nvram_get("subnet_rulelist"); %>'>
<input type="hidden" name="subnet_rulelist_ext" value='<% nvram_get("subnet_rulelist_ext"); %>'>
<input type="hidden" name="lan_trunk_type" value='<% nvram_get("lan_trunk_type"); %>' disabled>
<input type="hidden" name="lan_trunk_0" value='<% nvram_get("lan_trunk_0"); %>' disabled>
<input type="hidden" name="lan_trunk_1" value='<% nvram_get("lan_trunk_1"); %>' disabled>

<div id="subnet_div" class="contentM_connection" style="z-index:600;">
	<table border="0" align="center" cellpadding="5" cellspacing="5">
		<tr>
			<td align="left">
			<span class="formfonttitle"><#TBVLAN_EditSubnetProfile#></span>
			<div style="width:630px; height:2px;overflow:hidden;position:relative;top:5px;" class="splitLine"></div>
			<div style="margin-left:5px;"><#TBVLAN_Subnet_desc#></div>
			</td>
		</tr>
		<tr>
			<td>
				<table id="tbVLANGroup" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
					<thead>
						<tr>
							<td colspan="2"><#t2BC#></td>
						</tr>
					</thead>
					<tr>
						<th><#Subnet#></th>
						<td>
							<select name="selSubnet" class="input_option" onchange="switchSubnetValue(this.value)">
							</select>
						</td>
					<tr>
						<th><#LANHostConfig_DHCPServerConfigurable_itemname#></th>
						<td>
							<input type="radio" value="1" name="radioDHCPEnable" class="content_input_fd" checked><#checkbox_Yes#>
							<input type="radio" value="0" name="radioDHCPEnable" class="content_input_fd"><#checkbox_No#>
						</td>
					</tr>
					<tr>
						<th><#RouterConfig_GWStaticGW_itemname#></th>
						<td>
							<input type="text" maxlength="15" class="input_25_table" name="tGatewayIP" onchange="checkIPLegality();" onKeyPress="return validator.isIPAddr(this,event);">
						</td>
					</tr>
					<tr>
						<th><#IPConnection_x_ExternalSubnetMask_itemname#></th>
						<td>
							<input type="text" maxlength="15" class="input_25_table" name="tSubnetMask" onchange="checkMaskLegality();" onKeyPress="return validator.isIPAddr(this,event);">
						</td>
					</tr>
					<tr>
						<th><#LANHostConfig_MinAddress_itemname#></th>
						<td>
							<input type="text" maxlength="15" class="input_25_table" name="tDHCPStart"  onKeyPress="return validator.isIPAddr(this,event);">
						</td>
					</tr>
					<tr>
						<th><#LANHostConfig_MaxAddress_itemname#></th>
						<td>
							<input type="text" maxlength="15" class="input_25_table" name="tDHCPEnd"  onKeyPress="return validator.isIPAddr(this,event);">
						</td>
					</tr>
					<tr>
						<th><#LANHostConfig_LeaseTime_itemname#></th>
						<td>
							<input type="text" maxlength="6" class="input_25_table" name="tLeaseTime" onKeyPress="return validator.isNumber(this,event);">
						</td>
					</tr>
				</table>
			</td>
		</tr>
	</table>
	<div style="margin-top:5px;padding-bottom:10px;width:100%;text-align:center;">
		<input class="button_gen" type="button" onclick="hide_subnet_edit();" value="<#CTL_Cancel#>">
		<input class="button_gen" type="button" onclick="saveSubnetSettings();" value="<#CTL_ok#>">
	</div>
</div>
<div id="VlanSettings_table"  class="contentM_connection">
	<table id="t2BC" width="97%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" style="margin-top:10px;margin-bottom:10px;">
		<thead>
			<tr>
				<td colspan="2"><#t2BC#></td>
			</tr>
		</thead>
		<tr id="vlan_id_tr">
			<th width="30%"><#WANVLANIDText#></th>
			<td id="vlan_id">
				<input type="text" name="vlan_id" class="input_6_table" maxlength="4" value="" onKeyPress="return validator.isNumber(this, event);">
			</td>
			<td id="default_vlan_id" style="display:none;"></td>
		</tr>
		<tr id="vlan_prio_tr">
		  	<th width="30%"><#BM_UserList4#></th>
		  	<td id="vlan_prio">
				<input type="text" name="vlan_prio" class="input_6_table" maxlength="1" value="" onKeyPress="return validator.isNumber(this, event);">
		  	</td>
		  	<td id="default_vlan_prio" style="display:none;"></td>
		</tr>
		<tr>
			<th><#Subnet#></th>
			<td id="subnet_sel" align="left">
				<select id="subnet_list" class="input_option" name="subnet_list" onchange="get_LanToLanRoute(this.value);generate_LanToLanRoute_options();show_LanToLanRoute_list();">
				</select>
				<input id="subnet_btn" class="edit_btn" onclick="show_subnet_edit()" value=""/>
			</td>
			<td id="default_subnet_desc" style="word-break: keep-all;display:none;"><#Setting_factorydefault_value#></td>
		</tr>
	</table>
<div id="LanToLanRoute_Support">
	<div id="LanToLanRoute_div">
	<table width="97%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable_table" style="margin-top:10px;">
		<thead>
			<tr>
				<td colspan="3">LAN-to-LAN route</td>
			</tr>
		</thead>
		<tr>
		<th width="80%">Other Subnet</th>
		<th width="20%"><#list_add_delete#></th>
		</tr>
		<tr id="addSubnet_tr">
			<td>
				<select id="other_subnet_select" class="input_option" name="other_subnet_select"></select>
			</td>
			<td><input type="button" class="add_btn" onClick="add_LanToLanRoute();" name="add_route_btn" value=""></td>
		</tr>
	</table>
	<div id="lanToLanRoute_Block"></div>
	</div>
</div>
	<table width="97%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable_table" style="margin-top:10px;">
		<thead>
			<tr>
				<td colspan="3" id="binding_inf_head"><#TBVLAN_BindingInf#></td>
	   		</tr>
	  	</thead>
        <tr>
        	<th width="40%"><#wan_interface#></th>
			<th width="40%"><#vpn_openvpn_interface#></th>
            <th width="20%"><#list_add_delete#></th>
		</tr>
		<tr id="addInterface_tr">
			<td>
				<select id="interface_list" class="input_option" name="interface_list" onchange="change_interface_type(this);"></select>
			</td>
            <td>
            	<select id="port_type" class="input_option" name="port_type" onchange=""></select>
	    	<td><input type="button" class="add_btn" onClick="add_BindingInf();" name="add_inf_btn" value=""></td>
		</tr>
	</table>
	<div id="binding_inflist_Block"></div>
	<div class="apply_gen" style="margin-top:20px;margin-bottom:10px;background-color: #2B373B;" id="applyDiv">
		<input name="cancelBtn" type="button" class="button_gen" onclick="showSettingTable(0);" value="<#CTL_Cancel#>"/>
		<input id="addRuleBtn" name="addRuleBtn" type="button" class="button_gen" onclick="add_vlan_rule()" value="<#CTL_ok#>"/>
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
		<td align="left" valign="top">
	  		<table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
				<tbody>
					<tr>
						<td bgcolor="#4D595D" valign="top"  >
							<div>&nbsp;</div>
							<div class="formfonttitle">VLAN - <#TBVLAN_Title#></div>
							<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
							<div class="formfontdesc"><#VLAN_desc#></div>
							<table width="98%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:20px;">
							<thead>
								<tr>
									<td colspan="2"><#t2BC#></td>
								</tr>
							</thead>
							<tr>
								<th><#TBVLAN_EnableVLAN#></th>
								<td>
									<div class="left" style="width:94px; float:left; cursor:pointer;" id="vlan_enable"></div>
									<div class="clear">
										<script type="text/javascript">
											$('#vlan_enable').iphoneSwitch('<% nvram_get("vlan_enable"); %>',
												function() {
													if(orig_lan_trunk_type != "0"){
														if(!confirm("Enable VLAN feature will disable bonding function in \'LAN > Switch Control\' page, Are you sure to continue?")){//untranslated
															curState = "0";
															$('#vlan_enable').find('.iphone_switch').animate({backgroundPosition: -37}, "slow");
															return false;
														}
														else{
															curState = "1";
															document.form.lan_trunk_type.disabled = false;
															document.form.lan_trunk_0.disabled = false;
															document.form.lan_trunk_1.disabled = false;															
															document.form.lan_trunk_type.value = "0";
															document.form.lan_trunk_0.value = "0";
															document.form.lan_trunk_1.value = "0";
														}
													}
													document.form.vlan_enable.value = "1";
												},
												function() {
												 	document.form.vlan_enable.value = "0";
												}
											);
										</script>
									</div>
								</td>

							</tr>
							<tr>
								<th><#TBVLAN_PVID#></th>
								<td>
								<div id="pvid_config_div" style="width: 310px; margin: 30px 8px 0px auto; display:table-cell;">
									<div style="height: 168px; margin-top: -5px; margin-bottom: -45px; background-image: url('images/New_ui/pvid.png'); background-repeat: no-repeat; background-position: center;">
									</div>
									
									<div class="pvid_div" style="position:relative; top:-71px;">
										<div id="lan1_div" class="pvid_select_div">
											<select id="lan1_pvid_select" class="pvid_select">
									  		</select>
										</div>
										<div id="lan3_div" class="pvid_select_div">
											<select id="lan3_pvid_select" class="pvid_select">
									  		</select>
										</div>
										<div id="lan5_div" class="pvid_select_div">
											<select id="lan5_pvid_select" class="pvid_select">
									  		</select>
										</div>
										<div id="lan7_div" class="pvid_select_div">
											<select id="lan7_pvid_select" class="pvid_select">
									  		</select>
										</div>
									</div>

									<div class="pvid_div" style="position:relative; top:-41px;">
										<div id="lan2_div" class="pvid_select_div">
											<select id="lan2_pvid_select" class="pvid_select">
									  		</select>
										</div>
										<div id="lan4_div" class="pvid_select_div">
											<select id="lan4_pvid_select" class="pvid_select">
									  		</select>
										</div>
										<div id="lan6_div" class="pvid_select_div">
											<select id="lan6_pvid_select" class="pvid_select">
									  		</select>
										</div>
										<div id="lan8_div" class="pvid_select_div">
											<select id="lan8_pvid_select" class="pvid_select">
									  		</select>
										</div>
									</div>
								</div>
								</td>
							</tr>
							</table>
							<div style="width:97%; display:table; margin-top:30px; margin-left:8px; font-size:12px; font-weight:800; font-weight:800;">
								<div style="width:50%; display:table-cell;">
									<div style="display:table-cell; vertical-align:bottom;"><#TBVLAN_Layer2VLAN#>&nbsp;(<#List_limit#>&nbsp;8)</div>
									<div style="display:table-cell;"><input id="add_vlan_btn" type="button" class="add_btn" onClick="showSettingTable(1);" value=""></div>
								</div>
								<div style="width:50%; display:table-cell;">
									<div style="display:table-cell; vertical-align:bottom; padding-left:120px;">U&nbsp;:&nbsp;<#TBVLAN_UPort#></div>
									<div style="color:#00c6ff; display:table-cell; vertical-align:bottom; padding-left:10px;">T&nbsp;:&nbsp;<#TBVLAN_TPort#></div>
								</div>
							</div>
							<table width="98%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" id="vlan_list_table" style="margin-top:5px;">
				    			<tr>
								<th style="width:12%"><#WLANConfig11b_WirelessCtrl_button1name#>/&nbsp;<#WLANConfig11b_WirelessCtrl_buttonname#></th>
				      				<th style="width:7%"><#WANVLANIDText#></th>
				    				<th style="width:7%"><#BM_UserList4#></th>
				      				<th><#Wired_Interface#></th>
									<th style="width:21%"><#TBVLAN_WirelessInf#></th>
				      				<th style="width:19%"><#Subnet#></th>
				      				<th style="width:7%"><#pvccfg_edit#></th>
				      				<th style="width:7%"><#CTL_del#></th>
				    			</tr>
							</table>
					
							<div id="vlan_rulelist_Block"></div>
							<div class="apply_gen">
								<input class="button_gen" onclick="applyRule()" type="button" value="<#CTL_apply#>"/>
							</div>
						</td>
					</tr>
				</tbody>
			</table>
	  	</td>
	</tr>
</table>
</td>
</tr>
</table>
<div id="footer"></div>
</form>
<form method="post" name="save_form" action="apply.cgi" target="hidden_frame">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="subnet_rulelist" value="">
</form>
</body>
</html>
