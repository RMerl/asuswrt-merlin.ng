<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - <#VALN_Profile#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<link rel="stylesheet" type="text/css" href="/css/icon.css">
<link rel="stylesheet" type="text/css" href="/SDN/sdn.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/form.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script type="text/javascript" src="/SDN/sdn.js"></script>
<style>
	.center-align{
		text-align:center;
	}
	#vlan_profile_Block{
		font-family: Arial, Helvetica, MS UI Gothic, MS P Gothic, Microsoft Yahei UI, sans-serif;
		width:750px;
		height:360px;
	}

	.add_btn{
		background: transparent url(images/New_ui/add.png) no-repeat;
		background-position: -1px -1px;
		border:0;
		height:32px;
		width:32px;
		cursor:pointer;
	}
	.add_btn_disabled{
		background: transparent url(images/New_ui/add.png) no-repeat;
		background-position: -1px -34px;
		border:0;
		height:32px;
		width:32px;
	}
	.remove_btn{
		background: transparent url(images/New_ui/delete.png) no-repeat;
		background-position: -1px -1px;
		border:0;
		height:32px;
		width:32px;
		cursor:pointer;
	}
	.remove_btn_disabled{
		background: transparent url(images/New_ui/delete.png) no-repeat;
		background-position: -1px -34px;
		border:0;
		height:32px;
		width:32px;
	}
.clientlist_viewlist {
	position: absolute;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius: 5px;
	z-index: 200;
	background-color:#444f53;
	margin-left: 140px;
	margin-top: -920px;
	width:950px;
	height:auto;
	box-shadow: 3px 3px 10px #000;
	display:block;
	overflow: auto;
}
</style>
<script>
var sdn_with_vid_json = [];
var cfg_clientlist = httpApi.hookGet("get_cfg_clientlist");

var str_descContent = "VLAN profile here refers to create a VLAN only network. If you want to create VLAN with DHCP, please go to $feature$";
str_descContent = str_descContent.replace("$feature$", Guest_Network_naming);


var convertRulelistToJson = function(attrArray, rulelist) {
	var rulelist_json = [];

	var each_rule = rulelist.split("<");
	var convertAtoJ = function(rule_array) {
		var rule_json = {}
		$.each(attrArray, function(index, value) {
			rule_json[value] = rule_array[index];
		});
		return rule_json;
	}

	$.each(each_rule, function(index, value) {
		if (value != "") {
			var one_rule_array = value.split(">");
			var one_rule_json = convertAtoJ(one_rule_array);
			if (!one_rule_json.error) rulelist_json.push(one_rule_json);
		}
	});

	return rulelist_json;
}

var add_back_missing_apg_dut_list = function(dut_list) {
	$.each(meshList, function(idx, meshNodeMac) {
		if (dut_list.indexOf(meshNodeMac) == -1) dut_list += "<" + meshNodeMac + ">>"
	})
	return dut_list;
}

apgRuleTable = [
	"mac",
	"wifiband",
	"lanport"
];

vlanRuleTable = [
	"vlan_idx",
	"vid",
	"port_isolation"
];

sdnRuleTable = [
	"idx",
	"sdn_name",
	"sdn_enable",
	"vlan_idx",
	"subnet_idx",
	"apg_idx",
	"vpnc_idx",
	"vpns_idx",
	"dns_filter_idx",
	"urlf_idx",
	"nwf_idx",
	"cp_idx",
	"gre_idx",
	"firewall_idx",
	"kill_switch",
	"access_host_service",
	"wan_idx",
	"pppoe-relay"
];

vlanTrunklistTable = [
	"mac",
	"port",
	"profile"
];

vlanBlklistTable = [
	"mac",
	"port",
	"profile"
];

var sdn_rl = decodeURIComponent(httpApi.nvramCharToAscii(["sdn_rl"]).sdn_rl);
var vlan_rl = decodeURIComponent(httpApi.nvramCharToAscii(["vlan_rl"]).vlan_rl);
var sdn_rl_json = convertRulelistToJson(sdnRuleTable, sdn_rl);
var vlan_rl_json = convertRulelistToJson(vlanRuleTable, vlan_rl);
var apg_dutList = {};
var meshList = [];

var vlan_rl_vid_array = [];
$.each(vlan_rl_json, function(idx, vlan) {
	vlan_rl_vid_array.push(vlan.vid);
});

var vlan_trunklist_orig = decodeURIComponent(httpApi.nvramCharToAscii(["vlan_trunklist"]).vlan_trunklist);
var vlan_trunklist_tmp = vlan_trunklist_orig+"_";	//easy to do replace
var vlan_trunklist_array = vlan_trunklist_orig.split("<");
var vlan_trunklist_port_array = [];
var vlan_trunklist_port_array_one_mac = [];	//2-dim by mac
var vlan_trunklist_port = "";
var vlan_trunklist_port_vid_array = [];
var vlan_rl_vid_array_tmp = [];

var vlan_trunklist_string = "";
var vlan_trunklist_json = [];
var vlan_trunklist_json_tmp = [];

if (vlan_trunklist_array.length > 1) {
	for (var b = 1; b < vlan_trunklist_array.length; b++) {	//DUT Macs
		vlan_trunklist_port_array_one_mac[b] = vlan_trunklist_array[b].split(">");
		for (var c = 0; c < vlan_trunklist_port_array_one_mac[b].length; c++) {	//binded LAN ports

			vlan_trunklist_port_array.push(vlan_trunklist_port_array_one_mac[b][c]);
			//collect each mac & port binding

			if (c > 0) {
				vlan_rl_vid_array_tmp = vlan_rl_vid_array;
				vlan_trunklist_port = vlan_trunklist_port_array_one_mac[b][c].split("#")[0];
				vlan_trunklist_port_vid_array = vlan_trunklist_port_array_one_mac[b][c].split("#")[1].split(",");
				vlan_rl_vid_array_tmp = vlan_trunklist_port_vid_array;
				
				//suppose 1st vid (vlan_rl_vid_array_tmp[0]) to bethe only one vid binded
				vlan_trunklist_string += "<"+vlan_trunklist_port_array_one_mac[b][0]+">"+vlan_trunklist_port+">"+vlan_rl_vid_array_tmp[0];
			}
		}
	}
	vlan_trunklist_json = convertRulelistToJson(vlanTrunklistTable, vlan_trunklist_string);
	vlan_trunklist_json_tmp = convertRulelistToJson(vlanTrunklistTable, vlan_trunklist_string);
}

$.each(sdn_rl_json, function(idx, _sdn_rl) {

	var apg_dut_list = decodeURIComponent(httpApi.nvramCharToAscii(["apg" + _sdn_rl.apg_idx + "_dut_list"])["apg" + _sdn_rl.apg_idx + "_dut_list"])
	apg_dut_list = add_back_missing_apg_dut_list(apg_dut_list);
	apg_dutList["apg" + _sdn_rl.apg_idx + "_dut_list"] = convertRulelistToJson(apgRuleTable, apg_dut_list)

	var oneRule = {};
	var vlanArrayIdx = -1;
	if(_sdn_rl.vlan_idx > 0){	//exclude : sdn without VID
		$.each(vlan_rl_json, function(_idx, _vlan_rl) {
			if (_sdn_rl.vlan_idx == _vlan_rl.vlan_idx) 
				vlanArrayIdx = _idx;
		})

		oneRule.apg_name = decodeURIComponent(httpApi.nvramCharToAscii(["apg" + _sdn_rl.apg_idx + "_ssid"])["apg" + _sdn_rl.apg_idx + "_ssid"]);
		oneRule.vid = vlan_rl_json[vlanArrayIdx].vid;
		oneRule.iso = vlan_rl_json[vlanArrayIdx].port_isolation;
		oneRule.vlan_idx = vlan_rl_json[vlanArrayIdx].vlan_idx;
		oneRule.apgIdx = _sdn_rl.apg_idx;

		if(oneRule.apgIdx > 0){
			sdn_with_vid_json.push(oneRule);
		}
	}

});

function arrayRemove(arr, value) {
	return arr.filter(function(ele) {
		return ele != value;
	});
}

	function initial(){
		show_menu();
		$('.button_gen').hide();
		var vlan_switch_array = { "VLAN" : ["VLAN", "Advanced_VLAN_Switch_Content.asp"], "Profile" : ["Profile", "Advanced_VLAN_Profile_Content.asp"]};
		$('#divSwitchMenu').html(gen_switch_menu(vlan_switch_array, "Profile"));
		$('#descContent').html(str_descContent);

		init_sdn_all_list();
		show_vlan_profile();
	}

	function show_vlan_profile(){
		
		var $target_div = $("#divVLANProfile");
		var table_id = "table_vlanProfile";
		$('#divVLANProfile').empty().show();

		var $table_bg = $("<table>").addClass("FormTable center-align").appendTo($target_div);
		$table_bg.attr("id", table_id)
			.attr('border', '1')
			.attr('cellpadding', '4')
			.attr('cellspacing', '0')
			.css('width', '100%');
		var $table_thead = $("<thead>").appendTo($table_bg);
		var $table_thead_tr = $("<tr>").appendTo($table_thead);
		//var $table_thead_title = $("<td>").html("<#GuestNetwork_ProfileList#>&nbsp;(<#List_limit#>&nbsp;"+sdn_maximum+")")
		var $table_thead_title = $("<td>").html("<#GuestNetwork_ProfileList#>")
								.attr('colSpan', '6')
								.appendTo($table_thead_tr);

		var $table_th_tr = $("<tr>").appendTo($table_bg);
		var $table_th_name = $("<th>").html("<#QIS_finish_wireless_item1#>").css({'text-align':'center','width':'50%'}).appendTo($table_th_tr);
		var $table_th_vid = $("<th>").html("<#WANVLANIDText#>").css({'text-align':'center','width':'20%'}).appendTo($table_th_tr);
		var $table_th_iso = $("<th>").html("<#VLAN_port_iso#>").css({'text-align':'center','width':'15%'}).appendTo($table_th_tr); 
		var $table_th_edit = $("<th>").html("<#list_add_delete#>").css({'text-align':'center','width':'15%'}).appendTo($table_th_tr); // / <#CTL_modify#>

		var $table_edit_tr = $("<tr>").appendTo($table_bg);
		var $table_edit_name = $("<td>").appendTo($table_edit_tr);
		
		var $edit_name = $("<div>--</div>").appendTo($table_edit_name);
		var $table_edit_vid = $("<td>").appendTo($table_edit_tr);
		var $edit_vid = $("<input type='text' maxlength='4' class='input_6_table' name='edit_vid' id='edit_vid' autocorrect='off' autocapitalize='off'>").appendTo($table_edit_vid);
		var $table_edit_iso = $("<td>").appendTo($table_edit_tr);
		var $edit_iso = $("<input type='checkbox' id='iso_checkbox' value=''>").appendTo($table_edit_iso);

		var $table_edit_edit = $("<td>").appendTo($table_edit_tr);
		if($('.button_gen').css('display') == 'none'){
			var $edit_add = $("<input type='button' class='add_btn' onClick='addRow_Group("+sdn_maximum+");' id='profile_add' value=''>").appendTo($table_edit_edit);
		}
		else{
			var $edit_add = $("<input type='button' class='add_btn_disabled' onClick='' id='profile_add' value=''>").appendTo($table_edit_edit);	
		}

		//Draw vlan_rl_json first and get apgX_ssid from vlan_idx
		var matched_ssid_array = [];
		var matched_ssid_array_tmp = [];
		var matched_apgx_array = [];
		var matched_apgx_array_tmp = [];
		$.each(vlan_rl_json, function(i, profile_vlan_rl) {

			matched_ssid_array_tmp = [];
			matched_apgx_array_tmp = [];
			$.each(sdn_with_vid_json, function(j, profile) {
				if(profile.vlan_idx == profile_vlan_rl.vlan_idx){
					matched_ssid_array_tmp.push(profile.apg_name);
					matched_apgx_array_tmp.push(profile.apgIdx);
				}
			});
			matched_ssid_array[i] = matched_ssid_array_tmp;
			matched_apgx_array[i] = matched_apgx_array_tmp;

			var tr_id = "Profile_"+i;
			var $table_tr = $("<tr>").attr("id",tr_id).appendTo($table_bg);

			if(matched_ssid_array[i] != ""){
				var network_name = matched_ssid_array[i].join('<br>');
				var $table_td_name = $("<td>").addClass("list_edit").html(network_name).appendTo($table_tr);
			}
			else{
				var $table_td_name = $("<td>").addClass("list_edit").html("--").appendTo($table_tr);	
			}
			var $table_td_vid = $("<td>").addClass("list_edit").html(profile_vlan_rl.vid).appendTo($table_tr);
			var $table_td_iso = $("<td>").addClass("list_edit").appendTo($table_tr);
			var iso_id = "iso_checkbox"+i;
			var $table_input_iso = $("<input type='checkbox' value=''>");
			$table_input_iso.attr("id", iso_id).attr("disabled",true).appendTo($table_td_iso);
			if(profile_vlan_rl.port_isolation==1){
				$table_input_iso.attr("checked",true);
			}
			else{
				$table_input_iso.attr("checked",false);
			}

			var $table_td_edit = $("<td>").appendTo($table_tr);

			var $edit_delete = $("<input type='button' class='remove_btn' onClick='del_Row(this);' value=''>").appendTo($table_td_edit);

			//click to edit profile
			$table_tr.on("click", "td.list_edit", function(e){
				$("#vlan_profile_Block").remove();

				var divObj = document.createElement("div");
				divObj.setAttribute("id","vlan_profile_Block");
				divObj.className = "clientlist_viewlist";
				document.body.appendChild(divObj);
				cal_panel_block("vlan_profile_Block", 0.045);

				create_vlan_profile_view(matched_apgx_array[i], matched_ssid_array[i], $table_tr.find("td:eq(1)").html(), $table_tr.find("input[type=checkbox]").prop('checked'));
			});
		});	

	}

var vlan_view_hide_flag = false;
function hide_vlan_profile_view_block() {

	if(vlan_view_hide_flag)
	{
		close_vlan_profile_View();
	}
	vlan_view_hide_flag=true;
}
function show_vlan_profile_view_block() {
	vlan_view_hide_flag = false;
}
function close_vlan_profile_View() {
	
	$('.button_gen').show();
	$("#vlan_profile_Block").fadeOut();
}

var choosed_rl_index="";
var choosed_rl_vid="";
var choosed_rl_iso="";
function create_vlan_profile_view( _apgx_array, _ssid_array, _vid, _iso){
	
	//register event to detect mouse click
	document.body.onclick = function() {hide_vlan_profile_view_block();}
	vlan_view_hide_flag = false;

	document.getElementById("vlan_profile_Block").onclick = function() {show_vlan_profile_view_block();}

	var sdn_with_vid_json_index = sdn_with_vid_json.findIndex(function(item, i){
  		return item.vid === _vid
	});

	var vlanrl_json_index = vlan_rl_json.findIndex(function(item, i){
  		return item.vid === _vid
	});

	if(sdn_with_vid_json_index>=0){
		choosed_rl_index = sdn_with_vid_json[sdn_with_vid_json_index].vlan_idx;
		choosed_rl_vid = sdn_with_vid_json[sdn_with_vid_json_index].vid;
		choosed_rl_iso = sdn_with_vid_json[sdn_with_vid_json_index].iso;
		var network_name = _ssid_array.join('<br>');
		var apgx_id_string = _apgx_array.join(',');
	}
	else{
		choosed_rl_index = vlan_rl_json[vlanrl_json_index].vlan_idx;
		choosed_rl_vid = vlan_rl_json[vlanrl_json_index].vid;
		choosed_rl_iso = vlan_rl_json[vlanrl_json_index].port_isolation;
		var network_name = "--";
		var apgx_id_string = "--";
	}

	var code="";

	code += "<div style='margin-top:15px;margin-left:15px;float:left;font-size:15px;color:#93A9B1;'>Edit profile</div>";
	code += "<div style='float:right;'><img src='/images/button-close.gif' style='width:30px;cursor:pointer' onclick='close_vlan_profile_View();'></div>";

	code += "<table border='0' align='center' cellpadding='0' cellspacing='0' style='width:100%;padding:0 15px 15px 15px;'><tbody><tr><td>";

	code += "<table width='100%' border='1' align='center' cellpadding='0' cellspacing='0' class='FormTable_table' style='margin-top:15px;'>";

	code += "<tr height='40px' align='left'>";
	code += "<th class='IE8HACK' width='40%'><#QIS_finish_wireless_item1#></th>";
	code += "<td class='IE8HACK' width='60%' style='padding:10px;text-align:left;'><div id='update_name'></div></td>";
	code += "</tr>";

	code += "<tr height='40px'>";
	code += "<th><#WANVLANIDText#></th>";
	code += "<td style='padding:10px;text-align:left;'>";
	code += "<input type='text' maxlength='4' class='input_6_table' name='update_vid' id='update_vid' autocorrect='off' autocapitalize='off' value=''>";
	code += "</td>";
	code += "</tr>";

	code += "<tr height='40px'>";
	code += "<th><#VLAN_port_iso#></th>";
	code += "<td style='padding:10px;text-align:left;'>";
	code += "<input type='radio' name='update_iso' class='input' value='1'><#checkbox_Yes#>";
	code += "<input type='radio' name='update_iso' class='input' value='0'><#checkbox_No#>";
	code += "</td>";
	code += "</tr>";
	
	code += "</table>";

	code += "</td></tr></tbody></table>";

	code += "<div style='margin-top:10px;margin-bottom:20px;width:100%;text-align:center;'>";
	code += "<input class='button_gen' type='button' onclick='close_vlan_profile_View()' value='<#CTL_Cancel#>'>";
	code += "<input class='button_gen' type='button' onclick='Update_vlan_rl(\""+apgx_id_string+"\","+choosed_rl_index+","+choosed_rl_vid+","+choosed_rl_iso+")' style='margin-left:15px;' value='<#CTL_ok#>'>";
	code += "</div>";

	$("#vlan_profile_Block").html(code);

	$("#update_name").html(network_name);
	$("#update_vid").val(_vid);
	if(_iso)
		$('input:radio[name=update_iso]').filter('[value=1]').prop('checked', true);
	else
		$('input:radio[name=update_iso]').filter('[value=0]').prop('checked', true);

	$("#vlan_profile_Block").fadeIn();
}

var vlan_rl_str_post = "";
function Update_vlan_rl( _apgx_str, _vlan_idx, _vlan_vid, _vlan_iso){
	//  _apgx_str, _vlan_idx  == "--" 的調整
	const apgx_id_array = _apgx_str.split(",");
	var choosed_apgx_array_length = apgx_id_array.length;
	var vlan_rl_index_update = vlan_rl_json.findIndex(function(item, i){
		return item.vid === _vlan_vid.toString();
	});
	
	if(validPanelForm(_vlan_vid)){

		vlan_rl_json[vlan_rl_index_update].vid = $("#update_vid").val();
		vlan_rl_json[vlan_rl_index_update].port_isolation = $('input:radio[name=update_iso]').filter('[value=1]').prop('checked')? "1":"0";

		//Update vlan_rl post data
		vlan_rl_str_post = Update_vlan_rl_post_data();

		//update vlan_trunklist
		if(vlan_trunklist_json.length > 1){
			var updated_vlan_trunklist = update_vlan_trunklist(vlan_rl_json[vlan_rl_index_update], _vlan_vid);
			nvramUpdate_obj = {"action_mode": "apply", "rc_service": "restart_net_and_phy;", "vlan_rl": vlan_rl_str_post, "vlan_trunklist": updated_vlan_trunklist};
		}
		else{
			nvramUpdate_obj = {"action_mode": "apply", "rc_service": "restart_net_and_phy;", "vlan_rl": vlan_rl_str_post};	//saveNvram
		}

		close_vlan_profile_View();
		show_vlan_profile();
	}
}

var nvramUpdate_obj = "";
function Update_vlan_rl_post_data(){
	var vlan_rl_str = "";
	$.each(vlan_rl_json, function(index, profile){

			vlan_rl_str += "<";
			vlan_rl_str += profile.vlan_idx;
			vlan_rl_str += ">";
			vlan_rl_str += profile.vid;
			vlan_rl_str += ">";
			vlan_rl_str += profile.port_isolation;
			vlan_rl_str += ">";
	});
	return vlan_rl_str;
}

function update_vlan_trunklist(update_profile, _orig_vid){

	var trunklist = "";
	var target_term11 = "#"+_orig_vid+",";
	var target_term12 = "#"+_orig_vid+">";
	var target_term13 = "#"+_orig_vid+"<";
	var target_term14 = "#"+_orig_vid+"_";
	var target_term21 = ","+_orig_vid+",";
	var target_term22 = ","+_orig_vid+">";
	var target_term23 = ","+_orig_vid+"<";
	var target_term24 = ","+_orig_vid+"_";
	var replace_term11 = "#"+update_profile.vid+",";
	var replace_term12 = "#"+update_profile.vid+">";
	var replace_term13 = "#"+update_profile.vid+"<";
	var replace_term14 = "#"+update_profile.vid+"_";
	var replace_term21 = ","+update_profile.vid+",";
	var replace_term22 = ","+update_profile.vid+">";
	var replace_term23 = ","+update_profile.vid+"<";
	var replace_term24 = ","+update_profile.vid+"_";

	trunklist = vlan_trunklist_tmp.replaceAll(target_term11, replace_term11);
	trunklist = trunklist.replaceAll(target_term12, replace_term12);
	trunklist = trunklist.replaceAll(target_term13, replace_term13);
	trunklist = trunklist.replaceAll(target_term14, replace_term14);
	trunklist = trunklist.replaceAll(target_term21, replace_term21);
	trunklist = trunklist.replaceAll(target_term22, replace_term22);
	trunklist = trunklist.replaceAll(target_term23, replace_term23);
	trunklist = trunklist.replaceAll(target_term24, replace_term24);
	if(trunklist[trunklist.length - 1] == "_"){	//remove last "_" char
		trunklist = trunklist.substring(0, trunklist.length - 1);
	}

	vlan_trunklist_tmp = trunklist+"_";	//eazy to replace
	return trunklist;
}

function remove_no_binding_mac(list){	//<DUT1 MAC>1#vid1,vid2>2#vid1,vid2<DUT2 MAC>3#vid3,vid4>4#vid1,vid2  //<0C:9D:92:47:06:50>
	var list_array_mac = list.split("<");
	var list_array_one_mac = [];
	var return_list = "";
	for (var a = 1; a < list_array_mac.length; a++) {	//DUT Macs
		list_array_one_mac[a] = list_array_mac[a].split(">");
		if(list_array_one_mac[a].length >= 2 && list_array_one_mac[a][1] != ""){
			return_list += "<";
			return_list += list_array_one_mac[a][0];	//mac

			for (var b=1;b < list_array_one_mac[a].length; b++){
				return_list += ">";
				return_list += list_array_one_mac[a][1];	//port binding
			}
		}
	}

	return return_list;
}

function del_Row(r){

if(confirm("<#VPN_Fusion_Delete_Alert#>")){
	var del_vid = "";
	var del_vlan_index = "";
	var i=r.parentNode.parentNode.rowIndex;
	document.getElementById('table_vlanProfile').deleteRow(i);

	var val = r.parentNode.parentNode.children[1].innerHTML;	//vid value
	var rm_rl_index = vlan_rl_json.findIndex(function(item, i){
  		return item.vid === val
	});

	del_vid = vlan_rl_json[rm_rl_index].vid;
	del_vlan_index = vlan_rl_json[rm_rl_index].vlan_idx;
	
	var specific_data = sdn_all_rl_json.filter(function(item, index, array){
		return (item.sdn_rl.vlan_idx == del_vlan_index);
	})[0];

	var del_idx = "";
	$.each(sdn_all_rl_json, function(index, item) {
		if(item.sdn_rl.idx == specific_data.sdn_rl.idx){
			del_idx = index;
			return false;
		}
	});

	if(del_idx !== ""){

		var sdn_all_rl_tmp = JSON.parse(JSON.stringify(sdn_all_rl_json));
		var del_sdn_rl = sdn_all_rl_tmp.splice(del_idx, 1);
		var del_sdn_all_rl = parse_JSONToStr_del_sdn_all_rl(del_sdn_rl);
			vlan_rl_json = vlan_rl_json.filter(function(item, index, array){
					return (item.vlan_idx != del_sdn_rl[0].vlan_rl.vlan_idx);
			});
		var vlan_rl_tmp = JSON.parse(JSON.stringify(vlan_rl_json));
		var sdn_all_list = parse_JSONToStr_sdn_all_list({"sdn_all_rl":sdn_all_rl_tmp, "vlan_rl":vlan_rl_tmp});

		//check apgX_dut_list with port binding
		//console.log( _profile_data.apg_rl.dut_list);
		var do_restart_net_and_phy = false;	// do restart_net_and_phy or not
		if(specific_data.apg_rl.dut_list != undefined){
			var apgX_dut_list_arr = specific_data.apg_rl.dut_list.split("<");

			$.each(apgX_dut_list_arr, function(index, dut_info){
				if(dut_info != ""){
					var del_idx_dut_list_array_by_mac = dut_info.split(">");
					if(del_idx_dut_list_array_by_mac[2] != ""){	//port binding
						do_restart_net_and_phy = true;
					}
				}
			});
		}

		var nvramSet_obj = {"action_mode": "apply", "rc_service": "restart_net_and_phy;"};

		//update vlan_trunklist
		if( vlan_trunklist_orig != "" && specific_data.vlan_rl.vid > 1){	//vid :2~4093
			var updated_vlan_trunklist = rm_vid_from_vlan_trunklist( specific_data.vlan_rl.vid );
			nvramSet_obj.vlan_trunklist = updated_vlan_trunklist;
			if(vlan_rl_tmp==""){	//without vid for vlan_trunklist
				nvramSet_obj.vlan_trunklist = "";
			}
		}

		nvramSet_obj["apg" + del_sdn_rl[0].apg_rl.apg_idx + "_enable"] = "0";
		if(httpApi.nvramGet(["qos_enable"]).qos_enable == "1"){
			nvramSet_obj.rc_service += "restart_qos;restart_firewall;";
		}
		$.extend(nvramSet_obj, sdn_all_list);
		$.extend(nvramSet_obj, del_sdn_all_rl);
		if(!httpApi.app_dataHandler){
			showLoading();
		}

		//console.log(nvramSet_obj);
		var showLoading_status = get_showLoading_status(nvramSet_obj.rc_service);
		if(isWLclient()){
			showLoading(showLoading_status.time);
			setTimeout(function(){
				showWlHintContainer();
			}, showLoading_status.time*1000);
			check_isAlive_and_redirect({"page": "Advanced_VLAN_Profile_Content.asp", "time": showLoading_status.time});
		}
		httpApi.nvramSet(nvramSet_obj, function(){
			if(isWLclient()) return;
			showLoading(showLoading_status.time);
			setTimeout(function() {
				if(!isMobile() && showLoading_status.disconnect){
					check_isAlive_and_redirect({"page": "Advanced_VLAN_Profile_Content.asp", "time": showLoading_status.time});
				}
				else{
					refreshpage();
				}
			}, showLoading_status.time*1000);
		});
		
	}
}

}

function rm_vid_from_vlan_trunklist(_rm_vid){

	var trunklist = "";
	//case: remove choosed vid
	$.each(vlan_trunklist_json, function(idx, items) {

		if(vlan_trunklist_json[idx].profile == _rm_vid){
			//alert(vlan_trunklist_json[idx].mac+"/"+vlan_trunklist_json[idx].port);		//matched mac & port

			for(var z=vlan_trunklist_port_array_one_mac.length-1; z>0; z--){	//by mac
				if(vlan_trunklist_json[idx].mac == vlan_trunklist_port_array_one_mac[z][0]){

					for(var x=vlan_trunklist_port_array_one_mac[z].length-1; x>0; x--){
						if(vlan_trunklist_port_array_one_mac[z][x].indexOf(vlan_trunklist_json[idx].port+"#") >= 0){
							vlan_trunklist_port_array_one_mac[z].splice(x, 1);
						}
					}
				}
			}

			vlan_trunklist_json_tmp.splice(idx, 1);
		}
	});
	vlan_trunklist_json = vlan_trunklist_json_tmp;	//sync

	vlan_trunklist_tmp = "";	//clean vlan_trunklist_tmp
	for(var t=1; t<vlan_trunklist_port_array_one_mac.length; t++){

		if(vlan_trunklist_port_array_one_mac[t].length >= 2){	//remove mac if no lan port binded
			for(var s=0; s<vlan_trunklist_port_array_one_mac[t].length; s++){
				vlan_trunklist_tmp += (s==0)?"<":">";
				vlan_trunklist_tmp += vlan_trunklist_port_array_one_mac[t][s];
			}
		}
	}

	vlan_trunklist_tmp += "_";	//eazy to replace

	var target_term11 = "#"+_rm_vid+",";
	var target_term12 = "#"+_rm_vid+">";
	var target_term13 = "#"+_rm_vid+"<";
	var target_term14 = "#"+_rm_vid+"_";
	var target_term21 = ","+_rm_vid+",";
	var target_term22 = ","+_rm_vid+">";
	var target_term23 = ","+_rm_vid+"<";
	var target_term24 = ","+_rm_vid+"_";	
	var replace_term11 = "#";
	var replace_term12 = ">";
	var replace_term13 = "<";
	var replace_term14 = "_";
	var replace_term21 = ",";
	var replace_term22 = ">";
	var replace_term23 = "<";
	var replace_term24 = "_";
	
	//case: remove only one vid 
	trunklist = vlan_trunklist_tmp.replaceAll(target_term11, replace_term11);
	for(var x=1; x<=8; x++){
		trunklist = trunklist.replaceAll(x+target_term12, replace_term12);
		trunklist = trunklist.replaceAll(x+target_term13, replace_term13);
		trunklist = trunklist.replaceAll(x+target_term14, replace_term14);
	}
	trunklist = trunklist.replaceAll(target_term21, replace_term21);
	trunklist = trunklist.replaceAll(target_term22, replace_term22);
	trunklist = trunklist.replaceAll(target_term23, replace_term23);
	trunklist = trunklist.replaceAll(target_term24, replace_term24);
	if(trunklist[trunklist.length - 1] == "_"){	//remove last "_" char
		trunklist = trunklist.substring(0, trunklist.length - 1);
	}

	vlan_trunklist_tmp = trunklist+"_";	//eazy to replace
	return trunklist;
}

function addRow_Group(sdn_maximum){
	if(sdn_all_rl_json.length >= sdn_maximum){
		alert("<#AiMesh_Binding_Rule_Maxi#>");
		return false;	
	}

	if(validForm()){

		var nvramSet_obj = {"action_mode": "apply", "rc_service": "restart_wireless;restart_net_and_phy;"};

		//SDN profile
		var selected_sdn_idx = "";
		Get_Wizard_Customized_from_VLAN(nvramSet_obj);
		
		//vlan update
		var vlan_idx_add = get_rl_new_idx_VLAN(vlan_rl_json);
		var vlan_vid_add = $("#edit_vid").val();
		var vlan_iso_add = $("#iso_checkbox").prop("checked")?"1":"0";
		vlan_rl_json.push(
			{vlan_idx: vlan_idx_add, vid: vlan_vid_add, port_isolation: vlan_iso_add}
		);

		//update vlan_rl 
		var vlan_rl_add = gen_vlan_rl(vlan_rl_json);
		nvramSet_obj.vlan_rl = vlan_rl_add;

		httpApi.nvramSet(nvramSet_obj, function(){

			var showLoading_status = get_showLoading_status(nvramSet_obj.rc_service);
			showLoading(showLoading_status.time);
			setTimeout(function() {
				if(!isMobile() && showLoading_status.disconnect){
					check_isAlive_and_redirect({"page": "Advanced_VLAN_Profile_Content.asp", "time": showLoading_status.time});
				}
				else{
					refreshpage();
				}
			}, showLoading_status.time*1000);
		});

	}
}

var vlan_vid_add_tmp = "";
function get_new_sdn_profile_VLAN(){
	var sdn_rl_profile = get_new_sdn_rl();
	var vlan_rl_profile = get_new_vlan_rl();
	var subnet_rl_profile = get_new_subnet_rl_VLAN(sdn_rl_profile);
	var apg_rl_profile = get_new_apg_rl();
	sdn_rl_profile.vlan_idx = vlan_rl_profile.vlan_idx;
	sdn_rl_profile.subnet_idx = subnet_rl_profile.subnet_idx;
	sdn_rl_profile.apg_idx = apg_rl_profile.apg_idx;

	var dut_list = "";
	$.each(cfg_clientlist, function(index, node_info){
		var wifi_band_set = {"mac":"", "wifi_band":0};
		var wifi_band_info = httpApi.aimesh_get_node_wifi_band(node_info);
		$.each(wifi_band_info, function(index, band_info){
			if(band_info.no_used > 0){
				wifi_band_set.mac = node_info.mac;
				wifi_band_set.wifi_band += band_info.band;
			}
		});
		if(wifi_band_set.wifi_band != 0){
			dut_list += "<" + wifi_band_set.mac + ">" + wifi_band_set.wifi_band + ">";
		}
	});
	apg_rl_profile.dut_list = dut_list;
	var sdn_obj = {};
	sdn_obj["sdn_" + sdn_rl_profile.idx + ""] = JSON.parse(JSON.stringify(new sdn_all_rl_attr()));
	sdn_obj["sdn_" + sdn_rl_profile.idx + ""]["sdn_rl"] = JSON.parse(JSON.stringify(sdn_rl_profile));
	sdn_obj["sdn_" + sdn_rl_profile.idx + ""]["vlan_rl"] = vlan_rl_profile;
	sdn_obj["sdn_" + sdn_rl_profile.idx + ""]["subnet_rl"] = subnet_rl_profile;
	sdn_obj["sdn_" + sdn_rl_profile.idx + ""]["apg_rl"] = apg_rl_profile;
	return sdn_obj;

	function get_new_sdn_rl(){
		var idx = 1;
		idx = get_rl_new_idx(idx, "sdn");
		var sdn_profile = new sdn_rl_attr();
		sdn_profile.idx = idx.toString();
		return (JSON.parse(JSON.stringify(sdn_profile)));
	}
	function get_new_vlan_rl(){
		var vlan_idx = 1;
		vlan_idx = get_vlan_rl_new_idx(vlan_idx, "vlan");

		var vid = 52;
		vid = get_vlan_rl_new_vid(vid);

		vlan_vid_add_tmp = vid;

		var vlan_profile = new vlan_rl_attr();
		vlan_profile.vlan_idx = vlan_idx.toString();
		vlan_profile.vid = vid.toString();
		return (JSON.parse(JSON.stringify(vlan_profile)));

		function get_vlan_rl_new_idx(start_idx, category){
			var new_idx = parseInt(start_idx);
			for(new_idx; new_idx < (new_idx + sdn_maximum); new_idx+=1){
				var specific_data = vlan_rl_json.filter(function(item, index, array){
					return (item.vlan_idx == new_idx);
				})[0];
				if(specific_data == undefined){
					break;
				}
			}
			return new_idx;
		}
		function get_vlan_rl_new_vid(start_vid){
			var new_vid = start_vid;
			for(start_vid; start_vid < (start_vid + sdn_maximum); start_vid+=1){
				var specific_data = vlan_rl_json.filter(function(item, index, array){
					return (item.vid == start_vid);
				})[0];
				if(specific_data == undefined){
					new_vid = start_vid;
					break;
				}
			}
			return new_vid;
		}
	}
	function get_new_apg_rl(){
		var apg_idx = 1;
		apg_idx = get_rl_new_idx(apg_idx, "apg");
		var apg_profile = new apg_rl_attr();
		apg_profile.apg_idx = apg_idx.toString();
		apg_profile.enable = "1";
		apg_profile.ssid = "ASUSTEST";
		apg_profile.security = "";
		apg_profile.hide_ssid = "0";
		apg_profile.bw_limit = "<0>>";
		apg_profile.timesched = "0";
		apg_profile.sched = "";
		apg_profile.ap_isolate = "0";
		apg_profile.macmode = "0";
		apg_profile.maclist = "";
		apg_profile.iot_max_cmpt = "";
		apg_profile.dut_list = "";
		return (JSON.parse(JSON.stringify(apg_profile)));
	}
	function get_rl_new_idx(start_idx, category){
		var new_idx = parseInt(start_idx);
		var compare_key = "idx";
		switch(category){
			case "sdn":
				compare_key = "idx";
				break;
			case "vlan":
				compare_key = "vlan_idx";
				break;
			case "subnet":
				compare_key = "subnet_idx";
				break;
			case "apg":
				compare_key = "apg_idx";
				break;
		}
		for(new_idx; new_idx < (new_idx + sdn_maximum); new_idx+=1){
			var specific_data = sdn_all_rl_json.filter(function(item, index, array){
				return (item.sdn_rl[compare_key] == new_idx);
			})[0];
			if(specific_data == undefined){
				break;
			}
		}
		return new_idx;
	}

	function get_new_subnet_rl_VLAN(sdn_profile){
		var subnet_idx = 1;
		subnet_idx = get_rl_new_idx(subnet_idx, "subnet");
		var dhcp_info = httpApi.nvramDefaultGet(["lan_ipaddr", "lan_netmask", "dhcp_enable_x", "dhcp_start", "dhcp_end", "dhcp_lease",
			"lan_domain", "dhcp_dns1_x", "dhcp_dns2_x", "dhcp_wins_x", "dhcp_static_x"]);
		var ipaddr = get_subnet_rl_new_ipaddr_VLAN();
		var ipaddr_substr = ipaddr.substr(0,ipaddr.lastIndexOf("."));
		var ipaddr_min = ipaddr_substr + "." + "2";
		var ipaddr_max = ipaddr_substr + "." + "254";

		var subnet_profile = new subnet_rl_attr();
		subnet_profile.subnet_idx = subnet_idx.toString();

		var ipaddr_subnet_arr = ipaddr.split(".");

		subnet_profile.ifname = "br" + (parseInt(sdn_profile.idx) + 51);
		subnet_profile.addr = ipaddr;
		subnet_profile.netmask = dhcp_info.lan_netmask;
		//subnet_profile.dhcp_enable = dhcp_info.dhcp_enable_x;
		subnet_profile.dhcp_enable = 0;
		subnet_profile.dhcp_min = ipaddr_min;
		subnet_profile.dhcp_max = ipaddr_max;
		subnet_profile.dhcp_lease = dhcp_info.dhcp_lease;
		subnet_profile.domain_name = dhcp_info.lan_domain;
		subnet_profile.dns = dhcp_info.dhcp_dns1_x + "," + dhcp_info.dhcp_dns2_x;
		subnet_profile.wins = dhcp_info.dhcp_wins_x;
		subnet_profile.dhcp_static = dhcp_info.dhcp_static_x;
		subnet_profile.dhcp_unit = "";
		return (JSON.parse(JSON.stringify(subnet_profile)));

		function get_subnet_rl_new_ipaddr_VLAN(){	//Add available vid&subnet assignment 
			var exist_ipaddr_arr = [];
			exist_ipaddr_arr.push(get_network_num(httpApi.nvramGet(["lan_ipaddr"]).lan_ipaddr));
			$.each(sdn_all_rl_json, function(index, sdn_all_rl){
				if(!$.isEmptyObject(sdn_all_rl.subnet_rl)){
					exist_ipaddr_arr.push(get_network_num(sdn_all_rl.subnet_rl.addr));
				}
			});
			return (function(){
			
				var init_subnet = 100;
				var ipaddr_arr = httpApi.nvramDefaultGet(["lan_ipaddr"]).lan_ipaddr.split(".");
				var new_ipaddr = "";
				if(document.form.edit_vid.value>=2 && document.form.edit_vid.value<=254){
					new_ipaddr = ipaddr_arr[0] + "." + ipaddr_arr[1] + "." + document.form.edit_vid.value;	//VID subnet
					if($.inArray(new_ipaddr, exist_ipaddr_arr) != "-1"){
						for(init_subnet; init_subnet < 255; init_subnet += 1){
							new_ipaddr = ipaddr_arr[0] + "." + ipaddr_arr[1] + "." + init_subnet;
							if($.inArray(new_ipaddr, exist_ipaddr_arr) == "-1") break;
						}
					}

				}
				else{
					for(init_subnet; init_subnet < 255; init_subnet += 1){
						new_ipaddr = ipaddr_arr[0] + "." + ipaddr_arr[1] + "." + init_subnet;
						if($.inArray(new_ipaddr, exist_ipaddr_arr) == "-1") break;
					}
				}
				return new_ipaddr + "." + ipaddr_arr[3];
			})();
			
			function get_network_num(_ipaddr){
				return _ipaddr.substr(0, _ipaddr.lastIndexOf("."));
			}
		}
	}
}

function Get_Wizard_Customized_from_VLAN(nvramSet_obj){

	var sdn_obj = get_new_sdn_profile_VLAN();
	var sdn_idx = Object.keys(sdn_obj);
	var sdn_profile = sdn_obj[sdn_idx];
	selected_sdn_idx = sdn_profile.sdn_rl.idx;
	
	var rc_append = "";
	sdn_profile.sdn_rl.sdn_name = "Customized";
	sdn_profile.apg_rl.ssid = "VLAN_"+$("#divVLANProfile").find("#edit_vid").val();
	var dut_list = "";
	var wifi_band = "0";
	dut_list = get_dut_list(wifi_band);
	sdn_profile.apg_rl.dut_list = dut_list;
	sdn_profile.apg_rl.sched = "";
	sdn_profile.apg_rl.timesched = "0";
	sdn_profile.apg_rl.bw_limit = "<0>>";
	sdn_profile.sdn_access_rl = [];
	sdn_profile.apg_rl.ap_isolate = "0";
	var sdn_all_rl_tmp = JSON.parse(JSON.stringify(sdn_all_rl_json));
	sdn_all_rl_tmp.push(JSON.parse(JSON.stringify(sdn_profile)));
	var sdn_all_list = parse_JSONToStr_sdn_all_list({"sdn_all_rl":sdn_all_rl_tmp});
	if(rc_append != "")
		nvramSet_obj.rc_service = nvramSet_obj.rc_service + rc_append;
	$.extend(nvramSet_obj, sdn_all_list);
	var apgX_rl = parse_apg_rl_to_apgX_rl(sdn_profile.apg_rl);
	$.extend(nvramSet_obj, apgX_rl);

}

function gen_vlan_rl(vlanrl_json){
	var new_vlan_rl = "";
	for(var x=1;x<=sdn_maximum;x++){
		$.each(vlanrl_json, function(index, profile){
			if(profile.vlan_idx == x){
				new_vlan_rl = new_vlan_rl+"<"+profile.vlan_idx+">"+profile.vid+">"+profile.port_isolation+">";
			}
		});
	}
	return new_vlan_rl;
}

function validForm(){
	if(!validator.numberRange(document.form.edit_vid, 1, 4093)){
		return false;		
	}

	//duplicate vid check
	var dup_vid = 0;
	$.each(vlan_rl_json, function(index, profile){
		if(document.form.edit_vid.value == profile.vid){
			dup_vid = 1;
		}
	});

	if(dup_vid){
		alert("<#JS_duplicate#>");
		document.form.edit_vid.focus();
		return false;
	}
	else{
		return true;
	}
}

function validPanelForm(_vid){

	if(!validator.numberRange(document.getElementById("update_vid"), 1, 4093)){
		return false;		
	}

	//duplicate vid check，except itself
	var dup_vid = 0;
	$.each(vlan_rl_json, function(index, profile){
		if(document.getElementById("update_vid").value == _vid){	//no changed
			dup_vid = 0;
		}
		else if(document.getElementById("update_vid").value == profile.vid){
			dup_vid = 1;
		}
	});

	if(dup_vid){
		alert("<#JS_duplicate#>");
		document.getElementById("update_vid").focus();
		return false;
	}
	else{
		return true;
	}

}

var vlan_rl_attr = function(){
	this.vlan_idx = "";
	this.vid = "";
	this.port_isolation = "0";
};

function get_rl_new_idx_VLAN(rl){

	var vlan_rl_tmp = get_vlan_rl_PostData(rl);
	var new_idx = 1;
	var vlan_array = vlan_rl_tmp.split("<");
	var vlan_length = vlan_array.length;
	for(var j=1;j<vlan_length;j++){
		if( vlan_array[j].split(">")[0] != j ){
			new_idx = j;
			return new_idx.toString();	//pick one
		}
	}

	new_idx = vlan_length;
	return new_idx.toString();	//last one
}

function applyRule(){
	var showLoading_status = get_showLoading_status(nvramUpdate_obj.rc_service);
	if(isWLclient()){
		showLoading(showLoading_status.time);
		setTimeout(function(){
			showWlHintContainer();
		}, showLoading_status.time*1000);
		check_isAlive_and_redirect({"page": "Advanced_VLAN_Profile_Content.asp", "time": showLoading_status.time});
	}
	httpApi.nvramSet(nvramUpdate_obj, function() {
		if(isWLclient()) return;
		showLoading(showLoading_status.time);
		setTimeout(function() {
			if(!isMobile() && showLoading_status.disconnect){
				check_isAlive_and_redirect({"page": "Advanced_VLAN_Profile_Content.asp", "time": showLoading_status.time});
			}
			else{
				refreshpage();
			}
		}, showLoading_status.time*1000);
	});
}

function get_vlan_rl_PostData(vlanrlJson) {
	var vlan_rl_PostData = "";	
	var vlan_rl_FlatArray = [];
	for(var x=1;x<=vlanrlJson.length+1;x++){

		$.each(vlanrlJson, function(idx, items) {

			if(x == items.vlan_idx){
				var vlan_rl_List = [];
				for (var attr in items) {
					vlan_rl_List.push(items[attr])
				}

				vlan_rl_FlatArray.push(vlan_rl_List.join(">"));
			}
		})
		vlan_rl_PostData = "<" + vlan_rl_FlatArray.join("<");
	}
	
	return vlan_rl_PostData;
}

function get_showLoading_status(_rc_service){
	var result = {"time": 5, "disconnect": false};//restart_net_and_phy will disconnect and logout
	if(_rc_service != undefined){
		if(_rc_service.indexOf("restart_net_and_phy;") >= 0){
			result.time = httpApi.hookGet("get_default_reboot_time");
			result.disconnect = true;
		}
		else if (_rc_service.indexOf("restart_wireless;") >= 0){
			result.time = 20;
		}
		else if(_rc_service.indexOf("restart_chilli;restart_uam_srv;") >= 0){
			result.time = 30;
		}
	}
	return result;
}

function check_isAlive_and_redirect(_parm){
	var page = "";
	var time = 30;
	var interval_time = 2;
	if(_parm != undefined){
		if(_parm.page != undefined && _parm.page != "") page = _parm.page;
		if(_parm.time != undefined && _parm.time != "" && !isNaN(_parm.time)) time = parseInt(_parm.time);
	}
	var lan_ipaddr = httpApi.nvramGet(["lan_ipaddr"]).lan_ipaddr;
	setTimeout(function(){
		var interval_isAlive = setInterval(function(){
			httpApi.isAlive("", lan_ipaddr, function(){ clearInterval(interval_isAlive); top.location.href = "/" + page + "";});
		}, 1000*interval_time);
	}, 1000*(time - interval_time));
}
</script>
</head>

<body onload="initial();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_VLAN_Switch_Content.asp">
<input type="hidden" name="next_page" value="Advanced_VLAN_Switch_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply_new">
<input type="hidden" name="action_script" value="restart_net">
<input type="hidden" name="action_wait" value="10">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
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
								<td bgcolor="#4D595D" valign="top">
									<div>&nbsp;</div>
									<div class="formfonttitle">VLAN</div>
									
									<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
									<div class="formfontdesc"><#VALN_Profile#></div>
									<div id="divSwitchMenu" style="margin-top:-40px;float:right;"></div>
									<div id="descContent" class="formfontdesc"></div>
									
									<div id="divVLANProfile" style="width:100%;display:none;"></div>
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
			<!--===================================End of Main Content===========================================-->
		</td>
		<td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
</form>	
<div id="footer"></div>
</body>
</html>

