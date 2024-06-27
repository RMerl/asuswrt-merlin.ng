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
<title><#Web_Title#> - IPTV</title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<script type="text/javascript" src="state.js"></script>
<script type="text/javascript" src="general.js"></script>
<script type="text/javascript" src="popup.js"></script>
<script type="text/javascript" src="help.js"></script>
<script type="text/javascript" src="validator.js"></script>
<script type="text/javaScript" src="/js/jquery.js"></script>
<script type="text/javascript" src="switcherplugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>

<style>
.contentM_connection{
	position:absolute;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius: 5px;
	z-index:500;
	background-color:#2B373B;
	margin-left: 30%;
	margin-top: 10px;
	width:650px;
	display:none;
	box-shadow: 3px 3px 10px #000;
}
</style>

<script>
var original_switch_stb_x = '<% nvram_get("switch_stb_x"); %>';
var original_switch_wantag = '<% nvram_get("switch_wantag"); %>';
var original_switch_wan0tagid = '<%nvram_get("switch_wan0tagid"); %>';
var original_switch_wan0prio  = '<%nvram_get("switch_wan0prio"); %>';
var original_switch_wan1tagid = '<%nvram_get("switch_wan1tagid"); %>';
var original_switch_wan1prio  = '<%nvram_get("switch_wan1prio"); %>';
var original_switch_wan2tagid = '<%nvram_get("switch_wan2tagid"); %>';
var original_switch_wan2prio  = '<%nvram_get("switch_wan2prio"); %>';

var wans_lanport = '<% nvram_get("wans_lanport"); %>';
var wans_dualwan_orig = '<% nvram_get("wans_dualwan"); %>';

var iptv_modified = 0;
var voip_modified = 0;
var iptv_port_settings_orig = '<%nvram_get("iptv_port_settings"); %>' == ""? "12": '<%nvram_get("iptv_port_settings"); %>';
var lacp_enabled = '<% nvram_get("lacp_enabled"); %>' == 1 ?true: false;
var orig_mr_enable = '<% nvram_get("mr_enable_x"); %>';
var orig_emf_enable = '<% nvram_get("emf_enable"); %>';
var orig_wan_vpndhcp = '<% nvram_get("wan_vpndhcp"); %>';
var orig_ttl_inc_enable = '<% nvram_get("ttl_inc_enable"); %>';
var iptv_profiles = [<% get_iptvSettings();%>][0];
var isp_profiles = iptv_profiles.isp_profiles;
var port_definitions = iptv_profiles.port_definitions;
var stbPortMappings = [<% get_stbPortMappings();%>][0];
var orig_wanports_bond = '<% nvram_get("wanports_bond"); %>';
var cloud_isp_profiles = [];
if(lacp_support){
	if(based_modelid == "GT-AC5300")
		var bonding_port_settings = [{"val": "4", "text": "LAN5"}, {"val": "3", "text": "LAN6"}];
	else if(based_modelid == "RT-AC86U" || based_modelid == "GT-AC2900")
		var bonding_port_settings = [{"val": "4", "text": "LAN1"}, {"val": "3", "text": "LAN2"}];
	else if(based_modelid == "XT8PRO" || based_modelid == "BM68")
		var bonding_port_settings = [{"val": "2", "text": "LAN2"}, {"val": "3", "text": "LAN3"}];
	else
		var bonding_port_settings = [{"val": "1", "text": "LAN1"}, {"val": "2", "text": "LAN2"}];
}

if(wan_bonding_support)
	var orig_bond_wan = httpApi.nvramGet(["bond_wan"], true).bond_wan;

// get Primary WAN setting list from hook.
// cplumn 0: dsl(wan)_enable
var MSWAN_List_Pri = [ <% get_MS_WAN_list_Pri(); %> ];
var mr_mswan_idx_orig = '<% nvram_get("mr_mswan_idx"); %>';
var with_multiservice=0;
for(var i = 1; i < MSWAN_List_Pri.length; i++){
	if (MSWAN_List_Pri[i][0] == "1") {
		with_multiservice++;
	}
}

function initial(){
	show_menu();
	get_cloud_profiles();
	create_stb_select(original_switch_stb_x);
	if(mswan_support){
		update_mr_mswan_idx();
		inputCtrl(document.form.mr_mswan_idx, with_multiservice ? 1 : 0);
	}
	create_mr_select(orig_mr_enable);
	if(dsl_support) {
		document.form.action_wait.value = 20;
		$("#isp_profile_tr").hide();
		$("#mr_enable_field").show();
		$("#enable_eff_multicast_forward").show();
	}
	else{	//DSL not support
		create_ISP_select();
	}
	
	if(vdsl_support) {
		if(document.form.dslx_rmvlan.value == "1")
			document.form.dslx_rmvlan_check.checked = true;
		else
			document.form.dslx_rmvlan_check.checked = false;
	}

	if(dualWAN_support) {
		if(based_modelid == "BRT-AC828"){
			$("#IPTV_desc_DualWAN_BRTAC828").css("display", "");
			$("#mr_hint").css("display", "");

		}
		else{
			$("#IPTV_desc_DualWAN").css("display", "");
			$("#mr_hint").css("display", "none");
		}
	}
	else
		$("#IPTV_desc").css("display", "");

	if(based_modelid == "GT-AC5300" || based_modelid == "GT-AC9600"){ //MODELDEP: GT-AC5300 : TRUNK ports
		document.getElementById("port_settings").style.display = "";
		document.form.iptv_port_settings.disabled = false;
		change_port_settings(iptv_port_settings_orig);
	}
}

function get_cloud_profiles(){
	$.getJSON("http://nw-dlcdnet.asus.com/plugin/js/iptv_profile.json",
		function(data){
			Object.keys(data).forEach(function(profile_name) {
				var newProfile = {};
				if(!is_duplicate_profile(data[profile_name].switch_wantag)){
					cloud_isp_profiles.push(data[profile_name]);
					newProfile.profile_name = profile_name;
					if(data[profile_name].iptv_port == "IPTV_PORT")
						newProfile.iptv_port = port_definitions.IPTV_PORT;
					else if(data[profile_name].iptv_port == "MSTB_PORT")
						newProfile.iptv_port = port_definitions.MSTB_PORT;
					else
						newProfile.iptv_port = "";

					if(data[profile_name].voip_port == "VOIP_PORT")
						newProfile.voip_port = port_definitions.VOIP_PORT;
					else
						newProfile.voip_port = "";

					if(data[profile_name].bridge_port == "IPTV_PORT")
						newProfile.bridge_port = port_definitions.IPTV_PORT;
					else if(data[profile_name].bridge_port == "VOIP_PORT")
						newProfile.bridge_port = port_definitions.VOIP_PORT;
					else
						newProfile.bridge_port = "";
					newProfile.iptv_config = data[profile_name].iptv_config;
					newProfile.voip_config = data[profile_name].voip_config;
					newProfile.switch_wantag = data[profile_name].switch_wantag;
					newProfile.switch_stb_x = data[profile_name].switch_stb_x;
					newProfile.mr_enable_x = data[profile_name].mr_enable_x;
					newProfile.emf_enable = data[profile_name].emf_enable;
					isp_profiles.push(newProfile);
				}
			});

			isp_profiles.sort(function(a, b){
									if(a.switch_wantag == "none" || b.switch_wantag == "manual")
										return -1;
									else if(b.switch_wantag == "none" || a.switch_wantag == "manual")
										return 1;
									else if(a.switch_wantag < b.switch_wantag)
										return -1;
									else if(a.switch_wantag > b.switch_wantag)
										return 1;

									return 0;
								});
			create_ISP_select();
		}
	);
}

function is_duplicate_profile(isp){
	for(var i = 0; i < isp_profiles.length; i++){
		if(isp == isp_profiles[i].switch_wantag)
			return 1;
	}

	return 0;
}

function is_cloud_profile(isp){
	for(var i = 0; i < cloud_isp_profiles.length; i++){
		if(isp == cloud_isp_profiles[i].switch_wantag){
			return 1;
		}
	}

	return 0;
}

function get_cloud_settings(isp){
	var cloud_profile = {};
	for(var i = 0; i < cloud_isp_profiles.length; i++){
		if(isp == cloud_isp_profiles[i].switch_wantag){
			cloud_profile = cloud_isp_profiles[i];
		}
	}

	return cloud_profile;
}

function create_stb_select(switch_stb_x){
	var select = document.getElementById("switch_stb_x0");
	var option_list = iptv_profiles.stb_x_options;

	if(option_list.length > 0){
		select.length = 0;
		for(var i = 0; i < option_list.length; i++){
			var text = option_list[i].name;
			var selected = false;

			if(text == "none")
				text = "<#wl_securitylevel_0#>";

			if(option_list[i].value == switch_stb_x)
				selected = true;

			var option = new Option(text, option_list[i].value, false, selected);
			select.options.add(option);
		}
	}

	if(select.selectedIndex < 0)
		select.selectedIndex = 0;
}

function create_ISP_select(){
	var select = document.getElementById("switch_wantag");
	var text = "";
	var selected = false;
	var found = false;

	if(isp_profiles.length > 0){
		select.length = 0;
		for(var i = 0; i < isp_profiles.length; i++){
			text = isp_profiles[i].profile_name;
			selected = false;

			if(text == "none")
				text = "<#wl_securitylevel_0#>";
			else if(text == "manual")
				text = "<#Manual_Setting_btn#>";

			if(isp_profiles[i].switch_wantag == original_switch_wantag){
				found = true;
				selected = true;
			}

			var option = new Option(text, isp_profiles[i].switch_wantag, false, selected);
			select.options.add(option);
		}
	}

	if(!found){
		if(original_switch_wantag != ""){
			$('#switch_wantag').val("manual");
		}
		else
			$('#switch_wantag').val("none");
	}

	ISP_Profile_Selection(original_switch_wantag);
}

function set_manual_items(){
	var stb_port_list = iptv_profiles.stb_x_options;
	var manual_settings = get_isp_settings("manual");
	var port_name = "";

	document.getElementById("wan_iptv_x").style.display = "none";
	document.getElementById("wan_voip_x").style.display = "none";
	document.getElementById("wan_bridge_x").style.display = "none";
	document.getElementById("wan_internet_x").style.display = "";
	document.form.switch_wan0tagid.disabled = false;
	document.form.switch_wan0prio.disabled = false;
	document.form.switch_wan1tagid.disabled = false;
	document.form.switch_wan1prio.disabled = false;
	document.form.switch_wan2tagid.disabled = false;
	document.form.switch_wan2prio.disabled = false;

	document.form.switch_wan0tagid.value = original_switch_wan0tagid;
	document.form.switch_wan0prio.value = original_switch_wan0prio;
	document.form.switch_wan1tagid.value = original_switch_wan1tagid;
	document.form.switch_wan1prio.value = original_switch_wan1prio;
	document.form.switch_wan2tagid.value = original_switch_wan2tagid;
	document.form.switch_wan2prio.value = original_switch_wan2prio;

	if(manual_settings.iptv_port != ""){
		if(port_definitions.IPTV_PORT.substr(0, 3) == "LAN")
			port_name = "LAN Port " + port_definitions.IPTV_PORT.substr(3);
		else
			port_name = port_definitions.IPTV_PORT;

		document.getElementById("wan_iptv_port4_x").style.display = "";
		document.getElementById("iptv_port4").innerHTML = port_name;
	}

	if(manual_settings.voip_port != ""){
		if(port_definitions.VOIP_PORT.substr(0, 3) == "LAN")
			port_name = "LAN Port " + port_definitions.VOIP_PORT.substr(3);
		else
			port_name =port_definitions.VOIP_PORT;
		document.getElementById("wan_voip_port3_x").style.display = "";
		document.getElementById("voip_port3").innerHTML = port_name;
	}
}

function get_isp_settings(isp){
	var profile = {};

	for(var i = 0; i < isp_profiles.length; i++){
		if( isp == isp_profiles[i].switch_wantag ){
			profile = isp_profiles[i];
		}
	}

	return profile;
}

function control_wans_primary(switch_stb_x) {
	if(based_modelid == "BRT-AC828" || based_modelid == "RT-AD7200"){
		if(switch_stb_x != "0" || document.form.switch_wantag.value != "none"){
			var primary_wan = wans_dualwan_orig.split(" ")[0];
			if(primary_wan != "lan") {
				document.getElementById("tr_wans_primary").style.display = "";
				free_options(document.form.wans_lanport1);
				var i;
				var arr = new Array(), varr = new Array();
				for(i = 5;i <= 8;i += 1) {
					arr.push("LAN Port " + i);
					varr.push(i);
				}
				add_options_x2(document.form.wans_lanport1, arr, varr, wans_lanport);
				document.getElementById("cur_primary").innerHTML = primary_wan.toUpperCase();
			}
		}
		else
			document.getElementById("tr_wans_primary").style.display = "none";
	}
}

function isEmpty(obj)
{
	for (var name in obj){
		return false;
	}

	return true;
};

function ISP_Profile_Selection(isp){
	var isp_settings = get_isp_settings(isp);

	iptv_modified = 0;
	voip_modified = 0;
	if(isp == "none")
		document.getElementById("wan_stb_x").style.display = "";
	else
		document.getElementById("wan_stb_x").style.display = "none";

	if(isp == "manual" || isEmpty(isp_settings)){
		set_manual_items();
	}
	else{
		document.getElementById("wan_internet_x").style.display = "none";
		document.getElementById("wan_iptv_port4_x").style.display = "none";
		document.getElementById("wan_voip_port3_x").style.display = "none";
		document.form.switch_stb_x.value = isp_settings.switch_stb_x;
		if(is_cloud_profile(isp)){
			document.form.switch_wan0tagid.disabled = false;
			document.form.switch_wan0prio.disabled = false;
			document.form.switch_wan1tagid.disabled = false;
			document.form.switch_wan1prio.disabled = false;
			document.form.switch_wan2tagid.disabled = false;
			document.form.switch_wan2prio.disabled = false;
		}
		else{
			document.form.switch_wan0tagid.disabled = true;
			document.form.switch_wan0prio.disabled = true;
			document.form.switch_wan1tagid.disabled = true;
			document.form.switch_wan1prio.disabled = true;
			document.form.switch_wan2tagid.disabled = true;
			document.form.switch_wan2prio.disabled = true;
		}
		if(isp_settings.iptv_port != "" || isp_settings.iptv_config == "1")
			document.getElementById("wan_iptv_x").style.display = "";
		else
			document.getElementById("wan_iptv_x").style.display = "none";

		if(isp_settings.iptv_port != ""){
			document.getElementById("iptv_title").innerHTML = "IPTV STB Port";
			document.getElementById("iptv_port").innerHTML = isp_settings.iptv_port;
			document.getElementById("iptv_port").style.display = "";
		}

		if(isp_settings.iptv_config == "1"){
			document.getElementById("iptv_port").style.display = "none";
			document.getElementById("iptv_title").innerHTML = "IPTV";
			document.getElementById("iptv_settings_btn").style.display = "";
			document.getElementById("iptv_configure_status").style.display = "";
			if(check_config_state("iptv"))
				document.getElementById("iptv_configure_status").innerHTML = "<#wireless_configured#>";
			else
				document.getElementById("iptv_configure_status").innerHTML = "Unconfigured";
		}
		else{
			document.getElementById("iptv_settings_btn").style.display = "none";
			document.getElementById("iptv_configure_status").style.display = "none";
		}

		if(isp_settings.voip_port != "" || isp_settings.voip_config == "1")
			document.getElementById("wan_voip_x").style.display = "";
		else
			document.getElementById("wan_voip_x").style.display = "none";

		if(isp_settings.voip_port != ""){
			document.getElementById("voip_title").innerHTML = "VoIP Port";
			document.getElementById("voip_port").innerHTML = isp_settings.voip_port;
			document.getElementById("voip_port").style.display = "";
		}

		if(isp_settings.voip_config == "1"){
			document.getElementById("voip_port").style.display = "none";
			document.getElementById("voip_title").innerHTML = "VoIP";
			document.getElementById("voip_settings_btn").style.display = "";
			document.getElementById("voip_configure_status").style.display = "";
			if(check_config_state("voip"))
				document.getElementById("voip_configure_status").innerHTML = "<#wireless_configured#>";
			else
				document.getElementById("voip_configure_status").innerHTML = "Unconfigured";
		}
		else{
			document.getElementById("voip_settings_btn").style.display = "none";
			document.getElementById("voip_configure_status").style.display = "none";
		}

		if(isp_settings.bridge_port != ""){
			document.getElementById("wan_bridge_x").style.display = "";
			document.getElementById("bridge_port").innerHTML = isp_settings.bridge_port;
		}
		else
			document.getElementById("wan_bridge_x").style.display = "none";
	}

	/* Special Applications */
	if(isp_settings.mr_enable_x != ""){
		document.getElementById("mr_enable_field").style.display = "none";
		document.form.mr_enable_x.disabled = true;
		change_mr_enable(0);
	}
	else{
		document.getElementById("mr_enable_field").style.display = "";
		document.form.mr_enable_x.disabled = false;
		document.form.mr_enable_x.value = orig_mr_enable;
		if(document.form.mr_enable_x.selectedIndex < 0)
			document.form.mr_enable_x.selectedIndex = 0;
		change_switch_stb(isp_settings.switch_stb_x); //hnd or BLUECAVE
	}

	if(isp_settings.emf_enable != ""){
		document.getElementById("enable_eff_multicast_forward").style.display = "none";
		document.form.emf_enable.disabled = true;
	}
	else{
		document.getElementById("enable_eff_multicast_forward").style.display = "";
		document.form.emf_enable.disabled = false;
		document.form.emf_enable.value = orig_emf_enable;
	}
	/*--*/

	/* Model dependent requirements */
	control_wans_primary(isp_settings.switch_stb_x); //BRT-AC828
	if(based_modelid == "GT-AC5300" || based_modelid == "GT-AC9600"){
		change_port_settings(document.form.iptv_port_settings.value);
	}
	/*--*/
}


function validForm(){
	if (!dsl_support){
        if(document.form.switch_wantag.value == "manual"){
			document.form.switch_stb_x.disabled = false;
			if(document.form.switch_wan1tagid.value == "" && document.form.switch_wan2tagid.value == "")
				document.form.switch_stb_x.value = "0";
			else if(document.form.switch_wan1tagid.value == "" && document.form.switch_wan2tagid.value != "")
				document.form.switch_stb_x.value = "3";
			else if(document.form.switch_wan1tagid.value != "" && document.form.switch_wan2tagid.value == "")
				document.form.switch_stb_x.value = "4";
			else
				document.form.switch_stb_x.value = "6";

			if(document.form.switch_wan0tagid.value.length > 0 && !validator.rangeNull(document.form.switch_wan0tagid, 2, 4094, ""))
				return false;

			if(document.form.switch_wan1tagid.value.length > 0 && !validator.rangeNull(document.form.switch_wan1tagid, 2, 4094, ""))
				return false;

			if(document.form.switch_wan2tagid.value.length > 0 && !validator.rangeNull(document.form.switch_wan2tagid, 2, 4094, ""))
				return false;

			if(document.form.switch_wan0prio.value.length > 0 && !validator.range(document.form.switch_wan0prio, 0, 7))
				return false;

			if(document.form.switch_wan1prio.value.length > 0 && !validator.range(document.form.switch_wan1prio, 0, 7))
				return false;

			if(document.form.switch_wan2prio.value.length > 0 && !validator.range(document.form.switch_wan2prio, 0, 7))
				return false;
		}
		else if(document.form.switch_wantag.value == "none"){
			document.form.switch_stb_x.disabled = false;
			document.form.switch_stb_x.value = document.form.switch_stb_x0.value;
		}
		else{
			var isp_profile = get_isp_settings(document.form.switch_wantag.value);
			document.form.switch_stb_x.value = isp_profile.switch_stb_x;
			if(is_cloud_profile(document.form.switch_wantag.value)){
				var cloud_profile = get_cloud_settings(document.form.switch_wantag.value);

				document.form.switch_stb_x.disabled = false;
				document.form.switch_wan0tagid.value = cloud_profile.switch_wan0tagid;
				document.form.switch_wan0prio.value = cloud_profile.switch_wan0prio;
				document.form.switch_wan1tagid.value = cloud_profile.switch_wan1tagid;
				document.form.switch_wan1prio.value = cloud_profile.switch_wan1prio;
				document.form.switch_wan2tagid.value = cloud_profile.switch_wan2tagid;
				document.form.switch_wan2prio.value = cloud_profile.switch_wan2prio;
			}
		}
	}
	else{
		document.form.switch_stb_x.disabled = false;
		document.form.switch_stb_x.value = document.form.switch_stb_x0.value;
	}

	if(dualWAN_support){	// dualwan LAN port should not be equal to IPTV port
		var tmp_pri_if = wans_dualwan_orig.split(" ")[0].toUpperCase();
		var tmp_sec_if = wans_dualwan_orig.split(" ")[1].toUpperCase();
		if (tmp_pri_if == 'LAN' || tmp_sec_if == 'LAN'){
			var port_conflict = false;
			var iptv_port = document.form.switch_stb_x.value;
			var iptv_port_settings = document.form.iptv_port_settings.value;

			if(based_modelid == "GT-AC5300"){
				/* Dual WAN: "LAN Port 1" (wans_lanport: 2), "LAN Port 2" (wans_lanport:1), "LAN Port 5" (wans_lanport:4), "LAN Port 6" (wans_lanport:3) */
				if(iptv_port_settings == "56"){// LAN Port 5 (switch_stb_x: 3)  LAN Port 6 (switch_stb_x: 4)
					if((wans_lanport == "4" && iptv_port == "3") || (wans_lanport == "3" && iptv_port == "4"))
						port_conflict = true;
					else if((iptv_port == "6" || iptv_port == "8") && (wans_lanport == '4' || wans_lanport == "3"))
						port_conflict = true;
				}
				else{// LAN Port 1 (switch_stb_x: 3)  LAN Port 2 (switch_stb_x: 4)
					if((wans_lanport == "2" && iptv_port == "3") || (wans_lanport == "1" && iptv_port == "4")) //LAN 1, LAN2
						port_conflict = true;
					else if((iptv_port == "6" || iptv_port == "8") && (wans_lanport == "2" || wans_lanport == "1"))
						port_conflict = true;
				}
			}
			else{
				if(iptv_port == wans_lanport)
					port_conflict = true;
					else{
						for(var i = 0; i < stbPortMappings.length; i++){
							if(iptv_port == stbPortMappings[i].value && stbPortMappings[i].comboport_value_list.length != 0){
								var value_list = stbPortMappings[i].comboport_value_list.split(" ");
								for(var j = 0; j < value_list.length; j++){
									if(wans_lanport == value_list[j])
										port_conflict = true;
								}
							}
						}
					}
			}

			if (port_conflict) {
				alert("<#RouterConfig_IPTV_conflict#>");
				return false;
			}
		}
	}

	if(document.form.udpxy_enable_x.value != 0 && document.form.udpxy_enable_x.value != ""){	//validate UDP Proxy
		if(!validator.range(document.form.udpxy_enable_x, 1024, 65535)){
			document.form.udpxy_enable_x.focus();
			document.form.udpxy_enable_x.select();
			return false;
		}
	}

	return true;
}

function turn_off_lacp_if_conflicts(){
	var turn_off_lacp = false;

	if (!lacp_enabled)
		return;

	if(bonding_port_settings[0].val == "1"  && bonding_port_settings[1].val == "2"){
		// LAN1 and/or LAN2.
		if(document.form.switch_wantag.value == "none" && (document.form.switch_stb_x0.value == "1" || document.form.switch_stb_x0.value == "2" || document.form.switch_stb_x0.value == "5")){
			turn_off_lacp = true;
		}
	}
	else if(bonding_port_settings[0].val == "2"  && bonding_port_settings[1].val == "3"){
		//LAN 2 and/or LAN3
		if(document.form.switch_wantag.value == "none" && (document.form.switch_stb_x0.value == "2" || document.form.switch_stb_x0.value == "3" || document.form.switch_stb_x0.value == "5" || document.form.switch_stb_x0.value == "6" || document.form.switch_stb_x0.value == "8")){
			turn_off_lacp = true;
		}
	}

	if(turn_off_lacp){
		document.form.lacp_enabled.disabled = false;
		document.form.lacp_enabled.value = "0";
	}
}

var reboot_confirm=0;
function applyRule(){
	if(validForm()){

		if(!dsl_support){
			if( (original_switch_stb_x != document.form.switch_stb_x0.value)
				|| (original_switch_wantag != document.form.switch_wantag.value)
				|| ((document.form.switch_wantag.value == "manual") && ((original_switch_wan0tagid != document.form.switch_wan0tagid.value)
																	|| (original_switch_wan0prio != document.form.switch_wan0prio.value)
																	|| (original_switch_wan1tagid != document.form.switch_wan1tagid.value)
																	|| (original_switch_wan1prio != document.form.switch_wan1prio.value)
																	|| (original_switch_wan2tagid != document.form.switch_wan2tagid.value)
																	|| (original_switch_wan2prio != document.form.switch_wan2prio.value)) )
			){
				turn_off_lacp_if_conflicts();
				reboot_confirm=1;
			}
		}
		else{
			if( based_modelid == "DSL-AC68U"){
				$("#dsl_vlan_check").show();
				reboot_confirm=1;
			}
		}

		//check primary wan
		if(based_modelid == "BRT-AC828" || based_modelid == "RT-AD7200") {
			if(document.form.switch_stb_x.value != "0" || document.form.switch_wantag.value == "unifi_biz") {
				var primary_wan = wans_dualwan_orig.split(" ")[0];
				var wans_second = wans_dualwan_orig.split(" ")[1];
				var wans_dualwan_temp = "";
				if(primary_wan != "lan") {
					document.form.wans_dualwan.disabled = false;
					document.form.wans_lanport.disabled = false;
					switch(wans_second) {
						case "lan" :
							wans_dualwan_temp = wans_second + " " + primary_wan;
							break;
						default :
							wans_dualwan_temp = "lan " + wans_second;
							break;
					}
					document.form.wans_dualwan.value = wans_dualwan_temp;
					document.form.wans_lanport.value = document.form.wans_lanport1.value;
					reboot_confirm=1;
					alert("Please make sure the internet wire already plug in 'Port LAN " + document.form.wans_lanport.value + "' as primary WAN.");/*untranslated*/
				}
			}
		}

		if(document.form.wan_proto_now.disabled==true)
			document.form.wan_proto_now.disabled==false;

		if(based_modelid == "GT-AC5300" || based_modelid == "GT-AC9600"){
			if(lacp_enabled && document.form.iptv_port_settings.value == "56"){
				document.form.lacp_enabled.disabled = false;
				document.form.lacp_enabled.value = "0";
			}
		}

		if(wan_bonding_support && orig_bond_wan == "1"){
			if(wanAggr_p2_conflicts_w_stb_port(document.form.switch_stb_x.value, wanAggr_p2_num(orig_wnaports_bond))){
				var msg = "<#WANAggregation_PortConflict_hint1#>".replace(/LAN-*\D* 4/, wanAggr_p2_name(orig_wnaports_bond));
				if(confirm(msg)){
					document.form.bond_wan.disabled = false;
					document.form.bond_wan.value = "0";
				}
				else{
					document.form.switch_wantag.value = original_switch_wantag;
					ISP_Profile_Selection(original_switch_wantag);
					return false;
				}
			}
		}

		turn_off_lacp_if_conflicts();

		if(reboot_confirm==1){
        	
			if(confirm("<#AiMesh_Node_Reboot#>")){
				FormActions("start_apply.htm", "apply", "reboot", "<% get_default_reboot_time(); %>");
				showLoading();
				document.form.submit();
	        	}
        	}
		else{

			showLoading();
			document.form.submit();
		}
	}
}

function change_rmvlan(){
	if(document.form.dslx_rmvlan_check.checked == true)
		document.form.dslx_rmvlan.value = 1;
	else
		document.form.dslx_rmvlan.value = 0;
}

var original_wan_proto_now = "";
var original_dnsenable_now = "";
var currentService = "";
var curState = "";//dns_switch
function set_connection(service){
    /*
    connection_type = [iptv_connection_type, voip_connection_type];
     */
	var connection_type = new Array();
	if(document.form.switch_wantag.value == "movistar"){
		connection_type = ["static", "dhcp"];
	}

	if(document.form.switch_wantag.value != original_switch_wantag){
		if(!iptv_modified)
			document.form.wan10_proto.value = connection_type[0];
		if(!voip_modified)
			document.form.wan11_proto.value = connection_type[1];
	}

	currentService = service;
	copy_index_to_unindex(service);
	curState = document.form.wan_dnsenable_x_now.value;
	if(service == "iptv"){
		document.getElementById("con_settings_title").innerHTML = "IPTV Connection Settings";
	}
	else if(service == "voip"){
		document.getElementById("con_settings_title").innerHTML = "VoIP Connection Settings";
	}

	original_wan_proto_now = document.form.wan_proto_now.value;
	original_dnsenable_now = document.form.wan_dnsenable_x_now.value;
	change_wan_type(document.form.wan_proto_now.value);
	document.form.show_pass_1.checked = false;
	switchType(document.form.wan_pppoe_passwd_now, document.form.show_pass_1.checked, true);
	set_wandhcp_switch(document.form.wan_dhcpenable_x_now.value);
	set_dns_switch(document.form.wan_dnsenable_x_now.value);
	show_connection_settings();
}

function show_connection_settings(){
	$("#connection_settings_table").fadeIn(300);
}

// test if WAN IP & Gateway & DNS IP is a valid IP
// DNS IP allows to input nothing
function valid_IP(obj_name, obj_flag){
		// A : 1.0.0.0~126.255.255.255
		// B : 127.0.0.0~127.255.255.255 (forbidden)
		// C : 128.0.0.0~255.255.255.254
		var A_class_start = inet_network("1.0.0.0");
		var A_class_end = inet_network("126.255.255.255");
		var B_class_start = inet_network("127.0.0.0");
		var B_class_end = inet_network("127.255.255.255");
		var C_class_start = inet_network("128.0.0.0");
		var C_class_end = inet_network("255.255.255.255");
		
		var ip_obj = obj_name;
		var ip_num = inet_network(ip_obj.value);

		if(obj_flag == "DNS" && ip_num == -1){ //DNS allows to input nothing
			return true;
		}
		
		if(obj_flag == "GW" && ip_num == -1){ //GW allows to input nothing
			return true;
		}
		
		if(ip_num > A_class_start && ip_num < A_class_end){
		   obj_name.value = ipFilterZero(ip_obj.value);
			return true;
		}
		else if(ip_num > B_class_start && ip_num < B_class_end){
			alert(ip_obj.value+" <#JS_validip#>");
			ip_obj.focus();
			ip_obj.select();
			return false;
		}
		else if(ip_num > C_class_start && ip_num < C_class_end){
			obj_name.value = ipFilterZero(ip_obj.value);
			return true;
		}
		else{
			alert(ip_obj.value+" <#JS_validip#>");
			ip_obj.focus();
			ip_obj.select();
			return false;
		}
}

function check_config_state(service){
	var wan_proto = "";
	var wan_ipaddr = "";
	var pppoe_username = "";
	var pppoe_passwd = "";
	var wan_ipaddr = "";
	var connection_type = new Array();

	if(document.form.switch_wantag.value == "movistar"){
		connection_type = ["static", "dhcp"];
	}

	if(service == "iptv"){
		if(document.form.switch_wantag.value != original_switch_wantag)
			wan_proto = connection_type[0];
		else
			wan_proto = document.form.wan10_proto.value;
		username = document.form.wan10_pppoe_username.value;
		passwd = document.form.wan10_pppoe_passwd.value;
		wan_ipaddr = document.form.wan10_ipaddr_x.value;
	}
	else if(service == "voip"){
		if(document.form.switch_wantag.value != original_switch_wantag)
			wan_proto =  connection_type[1];
		else
			wan_proto = document.form.wan11_proto.value;
		username = document.form.wan11_pppoe_username.value;
		passwd = document.form.wan11_pppoe_passwd.value;
		wan_ipaddr = document.form.wan11_ipaddr_x.value;
	}

	if(wan_proto == "pppoe" || wan_proto == "pptp" || wan_proto == "l2tp"){
		if(username != "" && passwd != "")
			return true;
		else
			return false;
	}
	else if(wan_proto == "static"){
		if(wan_ipaddr != "" && wan_ipaddr != "0.0.0.0")
			return true;
		else
			return false;
	}
	else /* dhcp */
		return true;
}

function save_connection_settings(){
	/* Validate Conneciton Settings */
	if(document.form.wan_dhcpenable_x_now.value == "0"){// Set IP address by userself
		if(!valid_IP(document.form.wan_ipaddr_x_now, "")) return false;  //WAN IP
		if(!valid_IP(document.form.wan_gateway_x_now, "GW"))return false;  //Gateway IP

		if(document.form.wan_gateway_x_now.value == document.form.wan_ipaddr_x_now.value){
			document.form.wan_ipaddr_x_now.focus();
			alert("<#IPConnection_warning_WANIPEQUALGatewayIP#>");
			return false;
		}
		
		// test if netmask is valid.
		var default_netmask = "";
		var wrong_netmask = 0;
		var netmask_obj = document.form.wan_netmask_x_now;
		var netmask_num = inet_network(netmask_obj.value);
		
		if(netmask_num==0){
			var netmask_reverse_num = 0;		//Viz 2011.07 : Let netmask 0.0.0.0 pass
		}else{
		var netmask_reverse_num = ~netmask_num;
		}
		
		if(netmask_num < 0) wrong_netmask = 1;

		var test_num = netmask_reverse_num;
		while(test_num != 0){
			if((test_num+1)%2 == 0)
				test_num = (test_num+1)/2-1;
			else{
				wrong_netmask = 1;
				break;
			}
		}
		if(wrong_netmask == 1){
			alert(netmask_obj.value+" <#JS_validip#>");
			netmask_obj.value = default_netmask;
			netmask_obj.focus();
			netmask_obj.select();
			return false;
		}
	}

	if(document.form.wan_dnsenable_x_now.value == "0" && document.form.wan_proto_now.value != "dhcp" && document.form.wan_dns1_x_now.value == "" && document.form.wan_dns2_x_now.value == ""){
		document.form.wan_dns1_x_now.focus();
		alert("<#IPConnection_x_DNSServer_blank#>");
		return false;
	}
	
	if(!document.form.wan_dnsenable_x_now.value == "1"){
		if(!valid_IP(document.form.wan_dns1_x_now, "DNS")) return false;  //DNS1
		if(!valid_IP(document.form.wan_dns2_x_now, "DNS")) return false;  //DNS2
	}
	
	if(document.form.wan_proto_now.value == "pppoe"
			|| document.form.wan_proto_now.value == "pptp"
			|| document.form.wan_proto_now.value == "l2tp"
			){
		if(!validator.string(document.form.wan_pppoe_username_now)
				|| !validator.string(document.form.wan_pppoe_passwd_now)
				)
			return false;
		
		if(!validator.numberRange(document.form.wan_pppoe_idletime_now, 0, 4294967295))
			return false;
	}
	
	if(document.form.wan_proto_now.value == "pppoe"){
		if(!validator.numberRange(document.form.wan_pppoe_mtu_now, 576, 1492)
				|| !validator.numberRange(document.form.wan_pppoe_mru_now, 576, 1492))
			return false;
		
		if(!validator.string(document.form.wan_pppoe_service_now)
				|| !validator.string(document.form.wan_pppoe_ac_now))
			return false;
	}

	hide_connection_settings();
	copy_unindex_to_index(currentService);
	if(currentService == "iptv")
		document.getElementById("iptv_configure_status").innerHTML = "<#wireless_configured#>";
	else if(currentService == "voip")
		document.getElementById("voip_configure_status").innerHTML = "<#wireless_configured#>";
}

function hide_connection_settings(){
	$("#connection_settings_table").fadeOut(300);
}

function copy_index_to_unindex(service){
	if(service == "iptv"){
		document.form.wan_proto_now.value = document.form.wan10_proto.value;
		document.form.wan_dhcpenable_x_now.value = document.form.wan10_dhcpenable_x.value;
		document.form.wan_dnsenable_x_now.value = document.form.wan10_dnsenable_x.value;
		document.form.wan_pppoe_username_now.value = document.form.wan10_pppoe_username.value;
		document.form.wan_pppoe_passwd_now.value = document.form.wan10_pppoe_passwd.value;
		document.form.wan_pppoe_idletime_now.value = document.form.wan10_pppoe_idletime.value;
		document.form.wan_pppoe_mtu_now.value = document.form.wan10_pppoe_mtu.value;
		document.form.wan_pppoe_mru_now.value = document.form.wan10_pppoe_mru.value;
		document.form.wan_pppoe_service_now.value = document.form.wan10_pppoe_service.value;
		document.form.wan_pppoe_ac_now.value = document.form.wan10_pppoe_ac.value;
		document.form.wan_pppoe_options_x_now.value = document.form.wan10_pppoe_options_x.value;
		document.form.wan_pptp_options_x_now.value = document.form.wan10_pptp_options_x.value;
		document.form.wan_ipaddr_x_now.value = document.form.wan10_ipaddr_x.value;
		document.form.wan_netmask_x_now.value = document.form.wan10_netmask_x.value;
		document.form.wan_gateway_x_now.value = document.form.wan10_gateway_x.value;
		document.form.wan_dns1_x_now.value = document.form.wan10_dns1_x.value;
		document.form.wan_dns2_x_now.value = document.form.wan10_dns2_x.value;
		document.form.wan_auth_x_now.value = document.form.wan10_auth_x.value;
	}
	else if(service == "voip"){
		document.form.wan_proto_now.value = document.form.wan11_proto.value;
		document.form.wan_dhcpenable_x_now.value = document.form.wan11_dhcpenable_x.value;
		document.form.wan_dnsenable_x_now.value = document.form.wan11_dnsenable_x.value;
		document.form.wan_pppoe_username_now.value = document.form.wan11_pppoe_username.value;
		document.form.wan_pppoe_passwd_now.value = document.form.wan11_pppoe_passwd.value;
		document.form.wan_pppoe_idletime_now.value = document.form.wan11_pppoe_idletime.value;
		document.form.wan_pppoe_mtu_now.value = document.form.wan11_pppoe_mtu.value;
		document.form.wan_pppoe_mru_now.value = document.form.wan11_pppoe_mru.value;
		document.form.wan_pppoe_service_now.value = document.form.wan11_pppoe_service.value;
		document.form.wan_pppoe_ac_now.value = document.form.wan11_pppoe_ac.value;
		document.form.wan_pppoe_options_x_now.value = document.form.wan11_pppoe_options_x.value;
		document.form.wan_pptp_options_x_now.value = document.form.wan11_pptp_options_x.value;
		document.form.wan_ipaddr_x_now.value = document.form.wan11_ipaddr_x.value;
		document.form.wan_netmask_x_now.value = document.form.wan11_netmask_x.value;
		document.form.wan_gateway_x_now.value = document.form.wan11_gateway_x.value;
		document.form.wan_dns1_x_now.value = document.form.wan11_dns1_x.value;
		document.form.wan_dns2_x_now.value = document.form.wan11_dns2_x.value;
		document.form.wan_auth_x_now.value = document.form.wan11_auth_x.value;
	}
}

function copy_unindex_to_index(service){
	if(service == "iptv"){
		document.form.wan10_proto.value = document.form.wan_proto_now.value;
		document.form.wan10_dhcpenable_x.value = document.form.wan_dhcpenable_x_now.value;
		document.form.wan10_dnsenable_x.value = document.form.wan_dnsenable_x_now.value;
		document.form.wan10_pppoe_username.value = document.form.wan_pppoe_username_now.value;
		document.form.wan10_pppoe_passwd.value = document.form.wan_pppoe_passwd_now.value;
		document.form.wan10_pppoe_idletime.value = document.form.wan_pppoe_idletime_now.value;
		document.form.wan10_pppoe_mtu.value = document.form.wan_pppoe_mtu_now.value;
		document.form.wan10_pppoe_mru.value = document.form.wan_pppoe_mru_now.value;
		document.form.wan10_pppoe_service.value = document.form.wan_pppoe_service_now.value;
		document.form.wan10_pppoe_ac.value = document.form.wan_pppoe_ac_now.value;
		document.form.wan10_pppoe_options_x.value = document.form.wan_pppoe_options_x_now.value;
		document.form.wan10_pptp_options_x.value = document.form.wan_pptp_options_x_now.value;
		document.form.wan10_ipaddr_x.value = document.form.wan_ipaddr_x_now.value;
		document.form.wan10_netmask_x.value = document.form.wan_netmask_x_now.value;
		document.form.wan10_gateway_x.value = document.form.wan_gateway_x_now.value;
		document.form.wan10_dns1_x.value = document.form.wan_dns1_x_now.value;
		document.form.wan10_dns2_x.value = document.form.wan_dns2_x_now.value;
		document.form.wan10_auth_x.value = document.form.wan_auth_x_now.value;
		iptv_modified = 1;
	}
	else if(service == "voip"){
		document.form.wan11_proto.value = document.form.wan_proto_now.value;
		document.form.wan11_dhcpenable_x.value = document.form.wan_dhcpenable_x_now.value;
		document.form.wan11_dnsenable_x.value = document.form.wan_dnsenable_x_now.value;
		document.form.wan11_pppoe_username.value = document.form.wan_pppoe_username_now.value;
		document.form.wan11_pppoe_passwd.value = document.form.wan_pppoe_passwd_now.value;
		document.form.wan11_pppoe_idletime.value = document.form.wan_pppoe_idletime_now.value;
		document.form.wan11_pppoe_mtu.value = document.form.wan_pppoe_mtu_now.value;
		document.form.wan11_pppoe_mru.value = document.form.wan_pppoe_mru_now.value;
		document.form.wan11_pppoe_service.value = document.form.wan_pppoe_service_now.value;
		document.form.wan11_pppoe_ac.value = document.form.wan_pppoe_ac_now.value;
		document.form.wan11_pppoe_options_x.value = document.form.wan_pppoe_options_x_now.value;
		document.form.wan11_pptp_options_x.value = document.form.wan_pptp_options_x_now.value;
		document.form.wan11_ipaddr_x.value = document.form.wan_ipaddr_x_now.value;
		document.form.wan11_netmask_x.value = document.form.wan_netmask_x_now.value;
		document.form.wan11_gateway_x.value = document.form.wan_gateway_x_now.value;
		document.form.wan11_dns1_x.value = document.form.wan_dns1_x_now.value;
		document.form.wan11_dns2_x.value = document.form.wan_dns2_x_now.value;
		document.form.wan11_auth_x.value = document.form.wan_auth_x_now.value;
		voip_modified = 1;
	}
}

function change_wan_type(wan_type){

	if(wan_type == "pppoe"){
		document.getElementById("wan_dhcp_tr").style.display="";
		document.getElementById("dnsenable_tr").style.display = "";

		if(original_switch_wantag != document.form.switch_wantag.value || original_wan_proto_now != document.form.wan_proto_now.value){
			document.form.wan_dhcpenable_x_now.value = "1";
		}
		set_wandhcp_switch(document.form.wan_dhcpenable_x_now.value);

		if(original_switch_wantag != document.form.switch_wantag.value || original_wan_proto_now != document.form.wan_proto_now.value){
			document.form.wan_dnsenable_x_now.value = "1";
		}
		set_dns_switch(document.form.wan_dnsenable_x_now.value);

		if(document.form.wan_dnsenable_x_now.value == "1"){
			inputCtrl(document.form.wan_dns1_x_now, 0);
			inputCtrl(document.form.wan_dns2_x_now, 0);
		}
		else{
			inputCtrl(document.form.wan_dns1_x_now, 1);
			inputCtrl(document.form.wan_dns2_x_now, 1);
		}
		inputCtrl(document.form.wan_auth_x_now, 0);
		inputCtrl(document.form.wan_pppoe_username_now, 1);
		document.getElementById('tr_pppoe_password').style.display = "";
		document.form.wan_pppoe_passwd_now.disabled = false;
		inputCtrl(document.form.wan_pppoe_idletime_now, 1);
		if(document.form.wan_pppoe_idletime_now.value.length == 0)
			document.form.wan_pppoe_idletime_now.value = "0";
		inputCtrl(document.form.wan_pppoe_idletime_check, 1);
		inputCtrl(document.form.wan_pppoe_mtu_now, 1);
		if(document.form.wan_pppoe_mtu_now.value.length == 0)
			document.form.wan_pppoe_mtu_now.value = "1492";
		inputCtrl(document.form.wan_pppoe_mru_now, 1);
		if(document.form.wan_pppoe_mru_now.value.length == 0)
			document.form.wan_pppoe_mru_now.value = "1492";
		inputCtrl(document.form.wan_pppoe_service_now, 1);
		inputCtrl(document.form.wan_pppoe_ac_now, 1);
		
		inputCtrl(document.form.wan_pppoe_options_x_now, 1);
		inputCtrl(document.form.wan_pptp_options_x_now, 0);

		var wan_dhcpenable = parseInt(document.form.wan_dhcpenable_x_now.value);
		document.getElementById('IPsetting').style.display = "";
		inputCtrl(document.form.wan_ipaddr_x_now, !wan_dhcpenable);
		inputCtrl(document.form.wan_netmask_x_now, !wan_dhcpenable);
		inputCtrl(document.form.wan_gateway_x_now, !wan_dhcpenable);

	}
	else if(wan_type == "pptp" || wan_type == "l2tp"){
		document.getElementById("wan_dhcp_tr").style.display="";
		document.getElementById("dnsenable_tr").style.display = "";

		if(original_switch_wantag != document.form.switch_wantag.value || original_wan_proto_now != document.form.wan_proto_now.value){
			document.form.wan_dhcpenable_x_now.value = "0";
		}
		set_wandhcp_switch(document.form.wan_dhcpenable_x_now.value);

		var wan_dhcpenable = parseInt(document.form.wan_dhcpenable_x_now.value);
		document.getElementById('IPsetting').style.display = "";
		inputCtrl(document.form.wan_ipaddr_x_now, !wan_dhcpenable);
		inputCtrl(document.form.wan_netmask_x_now, !wan_dhcpenable);
		inputCtrl(document.form.wan_gateway_x_now, !wan_dhcpenable);

		if(original_switch_wantag != document.form.switch_wantag.value || original_wan_proto_now != document.form.wan_proto_now.value){
			document.form.wan_dnsenable_x_now.value = "0";
		}
		set_dns_switch(document.form.wan_dnsenable_x_now.value);

		if(document.form.wan_dnsenable_x_now.value == "1"){
			inputCtrl(document.form.wan_dns1_x_now, 0);
			inputCtrl(document.form.wan_dns2_x_now, 0);
		}
		else{
			inputCtrl(document.form.wan_dns1_x_now, 1);
			inputCtrl(document.form.wan_dns2_x_now, 1);
		}
		inputCtrl(document.form.wan_auth_x_now, 0);
		inputCtrl(document.form.wan_pppoe_username_now, 1);
		document.getElementById('tr_pppoe_password').style.display = "";
		document.form.wan_pppoe_passwd_now.disabled = false;
		inputCtrl(document.form.wan_pppoe_mtu_now, 0);
		inputCtrl(document.form.wan_pppoe_mru_now, 0);
		inputCtrl(document.form.wan_pppoe_service_now, 0);
		inputCtrl(document.form.wan_pppoe_ac_now, 0);
		inputCtrl(document.form.wan_pppoe_options_x_now, 1);
	
		if(wan_type == "pptp"){
			inputCtrl(document.form.wan_pppoe_idletime_now, 1);
			if(document.form.wan_pppoe_idletime_now.value.length == 0)
				document.form.wan_pppoe_idletime_now.value = "0";
			inputCtrl(document.form.wan_pppoe_idletime_check, 1);
			inputCtrl(document.form.wan_pptp_options_x_now, 1);
		}
		else if(wan_type == "l2tp"){
			inputCtrl(document.form.wan_pppoe_idletime_now, 0);
			inputCtrl(document.form.wan_pppoe_idletime_check, 0);
			inputCtrl(document.form.wan_pptp_options_x_now, 0);
		}
	}
	else if(wan_type == "static"){
		document.getElementById("wan_dhcp_tr").style.display = "none";
		document.form.wan_dhcpenable_x_now.value = "0";
		document.getElementById('IPsetting').style.display = "";
		inputCtrl(document.form.wan_ipaddr_x_now, 1);
		inputCtrl(document.form.wan_netmask_x_now, 1);
		inputCtrl(document.form.wan_gateway_x_now, 1);

		inputCtrl(document.form.wan_auth_x_now, 1);
		inputCtrl(document.form.wan_pppoe_username_now, (document.form.wan_auth_x_now.value != ""));
		document.getElementById('tr_pppoe_password').style.display = (document.form.wan_auth_x_now.value != "") ? "" : "none";
		document.form.wan_pppoe_passwd_now.disabled = (document.form.wan_auth_x_now.value != "") ? false : true;
		inputCtrl(document.form.wan_pppoe_idletime_now, 0);
		inputCtrl(document.form.wan_pppoe_idletime_check, 0);
		inputCtrl(document.form.wan_pppoe_mtu_now, 0);
		inputCtrl(document.form.wan_pppoe_mru_now, 0);
		inputCtrl(document.form.wan_pppoe_service_now, 0);
		inputCtrl(document.form.wan_pppoe_ac_now, 0);
		
		inputCtrl(document.form.wan_pppoe_options_x_now, 0);
		inputCtrl(document.form.wan_pptp_options_x_now, 0);

		document.getElementById("dnsenable_tr").style.display = "none";
		inputCtrl(document.form.wan_dns1_x_now, 1);
		inputCtrl(document.form.wan_dns2_x_now, 1);
	}
	else{	// Automatic IP or 802.11 MD or ""
		document.form.wan_dhcpenable_x_now.value = "1";
		document.getElementById('IPsetting').style.display = "none";
		inputCtrl(document.form.wan_ipaddr_x_now, 0);
		inputCtrl(document.form.wan_netmask_x_now, 0);
		inputCtrl(document.form.wan_gateway_x_now, 0);
		document.getElementById('IPsetting').style.display = "none";

		document.getElementById("dnsenable_tr").style.display = "";
		if(original_switch_wantag != document.form.switch_wantag.value || original_wan_proto_now != document.form.wan_proto_now.value){
			document.form.wan_dnsenable_x_now.value = "1";
		}
		set_dns_switch(document.form.wan_dnsenable_x_now.value);

		if(document.form.wan_dnsenable_x_now.value == "1"){
			inputCtrl(document.form.wan_dns1_x_now, 0);
			inputCtrl(document.form.wan_dns2_x_now, 0);
		}
		else{
			inputCtrl(document.form.wan_dns1_x_now, 1);
			inputCtrl(document.form.wan_dns2_x_now, 1);
		}

		inputCtrl(document.form.wan_auth_x_now, 1);	
		
		inputCtrl(document.form.wan_pppoe_username_now, (document.form.wan_auth_x_now.value != ""));
		document.getElementById('tr_pppoe_password').style.display = (document.form.wan_auth_x_now.value != "") ? "" : "none";
		document.form.wan_pppoe_passwd_now.disabled = (document.form.wan_auth_x_now.value != "") ? false : true;
		
		inputCtrl(document.form.wan_pppoe_idletime_now, 0);
		inputCtrl(document.form.wan_pppoe_idletime_check, 0);
		inputCtrl(document.form.wan_pppoe_mtu_now, 0);
		inputCtrl(document.form.wan_pppoe_mru_now, 0);
		inputCtrl(document.form.wan_pppoe_service_now, 0);
		inputCtrl(document.form.wan_pppoe_ac_now, 0);
		
		inputCtrl(document.form.wan_pppoe_options_x_now, 0);
		inputCtrl(document.form.wan_pptp_options_x_now, 0);
	}
}

function change_wan_dhcp_enable(wan_dhcpenable_flag){
	if(wan_dhcpenable_flag == "0"){
		$('#dns_switch').find('.iphone_switch').animate({backgroundPosition: -37}, "slow");
		curState = "0";
		document.form.wan_dnsenable_x_now.value = "0";
		document.getElementById("dns_switch").style.pointerEvents = 'none';
		inputCtrl(document.form.wan_dns1_x_now, 1);
		inputCtrl(document.form.wan_dns2_x_now, 1);
	}
	else if(wan_dhcpenable_flag == "1"){
		document.getElementById("dns_switch").style.pointerEvents = 'auto';
	}
}

var curWandhcpState = "";
function set_wandhcp_switch(wan_dhcpenable_flag){
	if(wan_dhcpenable_flag == "0"){
		curWandhcpState = "0";
		$('#wandhcp_switch').find('.iphone_switch').animate({backgroundPosition: -37}, "fast");
	}
	else if(wan_dhcpenable_flag == "1"){
		curWandhcpState = "1";
		$('#wandhcp_switch').find('.iphone_switch').animate({backgroundPosition: 0}, "fast");
	}
}

function set_dns_switch(wan_dnsenable_flag){
	if(wan_dnsenable_flag == "0"){
		$('#dns_switch').find('.iphone_switch').animate({backgroundPosition: -37}, "fast");
		curState = "0";
	}
	else if(wan_dnsenable_flag == "1"){
		$('#dns_switch').find('.iphone_switch').animate({backgroundPosition: 0}, "fast");
		curState = "1";
	}
}

/* password item show or not */
function pass_checked(obj){
	switchType(obj, document.form.show_pass_1.checked, true);
}

function change_port_settings(val, changed){
	if(val == "12"){
		document.getElementById("switch_stb_x0").options[1].text = "LAN1"; 	 //P1
		document.getElementById("switch_stb_x0").options[2].text = "LAN2";	 //P0
		document.getElementById("switch_stb_x0").options[3].text = "LAN1 & LAN2"; //P1+P0

		document.getElementById("voip_port").innerHTML = "LAN2";
		if($("#wan_bridge_x").css("display") != "none")
			document.getElementById("iptv_port").innerHTML = "LAN2";
		else
			document.getElementById("iptv_port").innerHTML = "LAN1";
		document.getElementById("bridge_port").innerHTML = "LAN1";
		document.getElementById("voip_port3").innerHTML = "LAN Port 2";
		document.getElementById("iptv_port4").innerHTML = "LAN Port 1";
	}
	else if(val == "56"){
		if(changed){
			var msg="<#NAT_lacp_disable_note#>";	/*Untranslated*/
			if(lacp_enabled){
				if(!confirm(msg)){
					document.form.iptv_port_settings.value = "12";
					return;
				}
			}
		}
		document.getElementById("switch_stb_x0").options[1].text = "LAN5";
		document.getElementById("switch_stb_x0").options[2].text = "LAN6";
		document.getElementById("switch_stb_x0").options[3].text = "LAN5 & LAN6";

		document.getElementById("voip_port").innerHTML = "LAN6";
		if($("#wan_bridge_x").css("display") != "none")
			document.getElementById("iptv_port").innerHTML = "LAN6";
		else
			document.getElementById("iptv_port").innerHTML = "LAN5";
		document.getElementById("bridge_port").innerHTML = "LAN5";
		document.getElementById("voip_port3").innerHTML = "LAN Port 6";
		document.getElementById("iptv_port4").innerHTML = "LAN Port 5";
	}
	show_gaming_note(val);
}

function show_gaming_note(val){
	if(val == "12")
		document.getElementById("gaming_note").innerHTML = "<#RouterConfig_GW_GamingPortsNote_12#>";
	else if(val == "56")
		document.getElementById("gaming_note").innerHTML = "<#RouterConfig_GW_GamingPortsNote_56#>";
	document.getElementById("gaming_note_div").style.display = "";
}

function create_mr_select(val)
{
	var select = document.form.mr_enable_x;

	if(improxy_support){
		add_option(select, "<#WLANConfig11b_WirelessCtrl_button1name#> IGMP", "1", 0);
	//	add_option(select, "<#WLANConfig11b_WirelessCtrl_button1name#> MLD", "2", 0);
		add_option(select, "<#WLANConfig11b_WirelessCtrl_button1name#> IGMP & MLD", "3", 0);
	}
	else
		add_option(select, "<#WLANConfig11b_WirelessCtrl_button1name#>", "1", 0);

	select.value = val;
	if(select.selectedIndex < 0)
		select.selectedIndex = 0;

	change_mr_enable(select.value);
}

function change_mr_enable(val){
	var igmp_enable = (val == "1" || val == "3") && (improxy_support || hnd_support);
	var mld_enable = (val == "2" || val == "3") && improxy_support;
	var qleave_enable = val && (val != "0");
	inputCtrl(document.form.mr_igmp_ver, igmp_enable ? 1 : 0);
	inputCtrl(document.form.mr_mld_ver, mld_enable ? 1 : 0);
	inputCtrl(document.form.mr_qleave_x, qleave_enable ? 1 : 0);
}

function update_mr_mswan_idx(){
	for(var i = 1; i < MSWAN_List_Pri.length; i++){
		if (MSWAN_List_Pri[i][0] == "1") {
			add_option(document.form.mr_mswan_idx, i, i, (mr_mswan_idx_orig==i)?1:0);
		}
	}
}

function change_switch_stb(switch_stb_x){
	if(hnd_support || based_modelid == "BLUECAVE"){
		if(switch_stb_x != "0"){
			document.getElementById("mr_enable_x").style.display = "none";
			document.getElementById("mr_disable").style.display = "";
			document.form.mr_enable_x.value = "0";
		}
		else{
			document.getElementById("mr_enable_x").style.display = "";
			document.getElementById("mr_disable").style.display = "none";
			document.form.mr_enable_x.value = orig_mr_enable;
			if(document.form.mr_enable_x.selectedIndex < 0)
				document.form.mr_enable_x.selectedIndex = 0;
		}
	}
	else
		document.getElementById("mr_disable").style.display = "none";

	change_mr_enable(document.form.mr_enable_x.value);
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
<input type="hidden" name="current_page" value="/Advanced_IPTV_Content.asp">
<input type="hidden" name="next_page" value="/Advanced_IPTV_Content.asp">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply_new">
<input type="hidden" name="action_script" value="restart_net">
<input type="hidden" name="action_wait" value="10">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="dslx_rmvlan" value='<% nvram_get("dslx_rmvlan"); %>'>
<input type="hidden" name="ttl_inc_enable" value='<% nvram_get("ttl_inc_enable"); %>'>
<input type="hidden" name="wans_dualwan" value="<% nvram_get("wans_dualwan"); %>" disabled>
<input type="hidden" name="wans_lanport" value="<% nvram_get("wans_lanport"); %>" disabled>
<input type="hidden" name="switch_wan3tagid" value="<% nvram_get("switch_wan3tagid"); %>">
<input type="hidden" name="switch_wan3prio" value="<% nvram_get("switch_wan3prio"); %>">
<input type="hidden" name="wan_vpndhcp" value="<% nvram_get("wan_vpndhcp"); %>">
<input type="hidden" name="wan_dhcpenable_x_now" value="">
<input type="hidden" name="wan_dnsenable_x_now" value="">

<input type="hidden" name="wan10_proto" value="<% nvram_get("wan10_proto"); %>">
<input type="hidden" name="wan10_dhcpenable_x" value="<% nvram_get("wan10_dhcpenable_x"); %>">
<input type="hidden" name="wan10_dnsenable_x" value="<% nvram_get("wan10_dnsenable_x"); %>">
<input type="hidden" name="wan10_pppoe_username" value="<% nvram_get("wan10_pppoe_username"); %>">
<input type="hidden" name="wan10_pppoe_passwd" value="<% nvram_get("wan10_pppoe_passwd"); %>">
<input type="hidden" name="wan10_pppoe_idletime" value="<% nvram_get("wan10_pppoe_idletime"); %>">
<input type="hidden" name="wan10_pppoe_mtu" value="<% nvram_get("wan10_pppoe_mtu"); %>">
<input type="hidden" name="wan10_pppoe_mru" value="<% nvram_get("wan10_pppoe_mru"); %>">
<input type="hidden" name="wan10_pppoe_service" value="<% nvram_get("wan10_pppoe_service"); %>">
<input type="hidden" name="wan10_pppoe_ac" value="<% nvram_get("wan10_pppoe_ac"); %>">
<input type="hidden" name="wan10_pppoe_options_x" value="<% nvram_get("wan10_pppoe_options_x"); %>">
<input type="hidden" name="wan10_pptp_options_x" value="<% nvram_get("wan10_pptp_options_x"); %>">
<input type="hidden" name="wan10_ipaddr_x" value="<% nvram_get("wan10_ipaddr_x"); %>">
<input type="hidden" name="wan10_netmask_x" value="<% nvram_get("wan10_netmask_x"); %>">
<input type="hidden" name="wan10_gateway_x" value="<% nvram_get("wan10_gateway_x"); %>">
<input type="hidden" name="wan10_dns1_x" value="<% nvram_get("wan10_dns1_x"); %>">
<input type="hidden" name="wan10_dns2_x" value="<% nvram_get("wan10_dns2_x"); %>">
<input type="hidden" name="wan10_auth_x" value="<% nvram_get("wan10_auth_x"); %>">

<input type="hidden" name="wan11_proto" value="<% nvram_get("wan11_proto"); %>">
<input type="hidden" name="wan11_dhcpenable_x" value="<% nvram_get("wan11_dhcpenable_x"); %>">
<input type="hidden" name="wan11_dnsenable_x" value="<% nvram_get("wan11_dnsenable_x"); %>">
<input type="hidden" name="wan11_pppoe_username" value="<% nvram_get("wan11_pppoe_username"); %>">
<input type="hidden" name="wan11_pppoe_passwd" value="<% nvram_get("wan11_pppoe_passwd"); %>">
<input type="hidden" name="wan11_pppoe_idletime" value="<% nvram_get("wan11_pppoe_idletime"); %>">
<input type="hidden" name="wan11_pppoe_mtu" value="<% nvram_get("wan11_pppoe_mtu"); %>">
<input type="hidden" name="wan11_pppoe_mru" value="<% nvram_get("wan11_pppoe_mru"); %>">
<input type="hidden" name="wan11_pppoe_service" value="<% nvram_get("wan11_pppoe_service"); %>">
<input type="hidden" name="wan11_pppoe_ac" value="<% nvram_get("wan11_pppoe_ac"); %>">
<input type="hidden" name="wan11_pppoe_options_x" value="<% nvram_get("wan11_pppoe_options_x"); %>">
<input type="hidden" name="wan11_pptp_options_x" value="<% nvram_get("wan11_pptp_options_x"); %>">
<input type="hidden" name="wan11_ipaddr_x" value="<% nvram_get("wan11_ipaddr_x"); %>">
<input type="hidden" name="wan11_netmask_x" value="<% nvram_get("wan11_netmask_x"); %>">
<input type="hidden" name="wan11_gateway_x" value="<% nvram_get("wan11_gateway_x"); %>">
<input type="hidden" name="wan11_dns1_x" value="<% nvram_get("wan11_dns1_x"); %>">
<input type="hidden" name="wan11_dns2_x" value="<% nvram_get("wan11_dns2_x"); %>">
<input type="hidden" name="wan11_auth_x" value="<% nvram_get("wan11_auth_x"); %>">
<input type="hidden" name="lacp_enabled" value="<% nvram_get("lacp_enabled"); %>" disabled>
<input type="hidden" name="switch_stb_x" value="<% nvram_get("switch_stb_x"); %>" disabled>
<input type="hidden" name="bond_wan" value="<% nvram_get("bond_wan"); %>" disabled>

<!---- connection settings start  ---->
<div id="connection_settings_table"  class="contentM_connection">
	<table border="0" align="center" cellpadding="5" cellspacing="5">
		<tr>
			<td align="left">
			<span id="con_settings_title" class="formfonttitle">Connection Settings</span>
			<div style="width:630px; height:2px;overflow:hidden;position:relative;left:0px;top:5px;" class="splitLine"></div>
			<div></div>
			</td>
		</tr>
		<tr>
			<td>
				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
					<tr>
						<th><#Layer3Forwarding_x_ConnectionType_itemname#></th>
						<td align="left">
							<select id="wan_proto_menu" class="input_option" name="wan_proto_now" onchange="change_wan_type(this.value);">
								<option value="dhcp"><#BOP_ctype_title1#></option>
								<option value="static"><#BOP_ctype_title5#></option>
								<option value="pppoe">PPPoE</option>
								<option value="pptp">PPTP</option>
								<option value="l2tp">L2TP</option>
							</select>
		  				</td>
					</tr>
		 		</table>
	  		</td>
		</tr>
		<tr id="IPsetting">
			<td>
				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
				<thead>
					<tr>
						<td colspan="2"><#IPConnection_ExternalIPAddress_sectionname#></td>
					</tr>
				</thead>
				<tr id="wan_dhcp_tr">
					<th><#Layer3Forwarding_x_DHCPClient_itemname#></th>
					<td>
						<div class="left" style="width:94px; float:left;" id="wandhcp_switch"></div>
						<div class="iphone_switch_container" style="height:32px; width:74px; position: relative; overflow: hidden">
							<script type="text/javascript">
								$('#wandhcp_switch').iphoneSwitch(document.form.wan_dhcpenable_x_now.value,
									function() {
										curWandhcpState = "1";
										document.form.wan_dhcpenable_x_now.value = "1";
										inputCtrl(document.form.wan_ipaddr_x_now, 0);
										inputCtrl(document.form.wan_netmask_x_now, 0);
										inputCtrl(document.form.wan_gateway_x_now, 0);
										change_wan_dhcp_enable("1")
										return true;
									},
									function() {
										curWandhcpState = "0";
										document.form.wan_dhcpenable_x_now.value = "0";
										inputCtrl(document.form.wan_ipaddr_x_now, 1);
										inputCtrl(document.form.wan_netmask_x_now, 1);
										inputCtrl(document.form.wan_gateway_x_now, 1);
										change_wan_dhcp_enable("0");
										return true;
									}
								);
							</script>
					</td>
				</tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,1);"><#IPConnection_ExternalIPAddress_itemname#></a></th>
					<td><input type="text" name="wan_ipaddr_x_now" maxlength="15" class="input_15_table" value="" onKeyPress="return validator.isIPAddr(this, event);" ></td>
				</tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,2);"><#IPConnection_x_ExternalSubnetMask_itemname#></a></th>
					<td><input type="text" name="wan_netmask_x_now" maxlength="15" class="input_15_table" value="" onKeyPress="return validator.isIPAddr(this, event);" ></td>
				</tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,3);"><#IPConnection_x_ExternalGateway_itemname#></a></th>
					<td><input type="text" name="wan_gateway_x_now" maxlength="15" class="input_15_table" value="" onKeyPress="return validator.isIPAddr(this, event);" ></td>
				</tr>
				</table>
			</td>
		</tr>
		<tr id="DNSsetting">
			<td>
				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
          		<thead>
            	<tr>
             	<td colspan="2"><#IPConnection_x_DNSServerEnable_sectionname#></td>
            	</tr>
          		</thead>
         		<tr id="dnsenable_tr">
            		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,12);"><#IPConnection_x_DNSServerEnable_itemname#></a></th>
					<td>
						<div class="left" style="width:94px; float:left;" id="dns_switch"></div>
						<div class="iphone_switch_container" style="height:32px; width:74px; position: relative; overflow: hidden"></div>
							<script type="text/javascript">
								$('#dns_switch').iphoneSwitch(document.form.wan_dnsenable_x_now.value, 
									function() {
										curState = "1";
										document.form.wan_dnsenable_x_now.value = "1";
										inputCtrl(document.form.wan_dns1_x_now, 0);
										inputCtrl(document.form.wan_dns2_x_now, 0);
										return true;
									},
									function() {
										curState = "0";
										document.form.wan_dnsenable_x_now.value = "0";
										inputCtrl(document.form.wan_dns1_x_now, 1);
										inputCtrl(document.form.wan_dns2_x_now, 1);
										return true;
									}
								);
							</script>
						<div id="yadns_hint" style="display:none;"></div>
					</td>
          		</tr>
          		<tr>
            		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,13);"><#IPConnection_x_DNSServer1_itemname#></a></th>
            		<td><input type="text" maxlength="15" class="input_15_table" name="wan_dns1_x_now" value="" onkeypress="return validator.isIPAddr(this, event)" ></td>
          		</tr>
          		<tr>
            		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,14);"><#IPConnection_x_DNSServer2_itemname#></a></th>
            		<td><input type="text" maxlength="15" class="input_15_table" name="wan_dns2_x_now" value="" onkeypress="return validator.isIPAddr(this, event)" ></td>
          		</tr>
        		</table>
	  		</td>	
		</tr>
		<tr id="PPPsetting" >
			<td>
		  		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
            	<thead>
            		<tr>
              			<td colspan="2"><#PPPConnection_UserName_sectionname#></td>
            		</tr>
            	</thead>
            	<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,29);"><#PPPConnection_Authentication_itemname#></a></th>
					<td align="left">
					    <select class="input_option" name="wan_auth_x_now" onChange="change_wan_type(document.form.wan_proto_now.value);">
						    <option value="" <% nvram_match("wan_auth_x_now", "", "selected"); %>><#wl_securitylevel_0#></option>
						    <option value="8021x-md5" <% nvram_match("wan_auth_x_now", "8021x-md5", "selected"); %>>802.1x MD5</option>
					    </select></td>
				</tr>
            	<tr>
             	 	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,4);"><#Username#></a></th>
              		<td><input type="text" maxlength="64" class="input_32_table" name="wan_pppoe_username_now" value="" onkeypress="return validator.isString(this, event)"></td>
            	</tr>
            	<tr id="tr_pppoe_password">
              		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,5);"><#PPPConnection_Password_itemname#></a></th>
              		<td>
					<div style="margin-top:2px;"><input type="password" autocapitalize="off" maxlength="64" class="input_32_table" id="wan_pppoe_passwd_now" name="wan_pppoe_passwd_now" value=""></div>
					<div style="margin-top:1px;"><input type="checkbox" name="show_pass_1" onclick="pass_checked(document.form.wan_pppoe_passwd_now);"><#QIS_show_pass#></div>
					</td>
            	</tr>
				<tr style="display:none">
              		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,6);"><#PPPConnection_IdleDisconnectTime_itemname#></a></th>
              		<td>
                		<input type="text" maxlength="10" class="input_12_table" name="wan_pppoe_idletime_now" value="" onKeyPress="return validator.isNumber(this,event);" />
                		<input type="checkbox" style="margin-left:30;display:none;" name="wan_pppoe_idletime_check" value="" />
              		</td>
            	</tr>
            	<tr>
              		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,7);"><#PPPConnection_x_PPPoEMTU_itemname#></a></th>
              		<td><input type="text" maxlength="5" name="wan_pppoe_mtu_now" class="input_6_table" value="" onKeyPress="return validator.isNumber(this,event);"/></td>
            	</tr>
            	<tr>
              		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,8);"><#PPPConnection_x_PPPoEMRU_itemname#></a></th>
              		<td><input type="text" maxlength="5" name="wan_pppoe_mru_now" class="input_6_table" value="" onKeyPress="return validator.isNumber(this,event);"/></td>
            	</tr>
            	<tr>
              		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,9);"><#PPPConnection_x_ServiceName_itemname#></a></th>
              		<td><input type="text" maxlength="32" class="input_32_table" name="wan_pppoe_service_now" value="" onkeypress="return validator.isString(this, event)"/></td>
            	</tr>
            	<tr>
              		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,10);"><#PPPConnection_x_AccessConcentrator_itemname#></a></th>
              		<td><input type="text" maxlength="32" class="input_32_table" name="wan_pppoe_ac_now" value="<% nvram_get("wan_pppoe_ac_now"); %>" onkeypress="return validator.isString(this, event)"/></td>
            	</tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,17);"><#PPPConnection_x_PPTPOptions_itemname#></a></th>
					<td>
						<select name="wan_pptp_options_x_now" class="input_option">
							<option value=""><#Auto#></option>
							<option value="-mppc"><#No_Encryp#></option>
							<option value="+mppe-40">MPPE 40</option>
							<option value="+mppe-128">MPPE 128</option>
						</select>
					</td>
				</tr>
				<tr>
					<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,18);"><#PPPConnection_x_AdditionalOptions_itemname#></a></th>
					<td><input type="text" name="wan_pppoe_options_x_now" value="<% nvram_get("wan_pppoe_options_x_now"); %>" class="input_32_table" maxlength="255" onKeyPress="return validator.isString(this, event)" onBlur="validator.string(this)"></td>
				</tr>
          </table>
        </td>
    </tr>
	</table>

	<div style="margin-top:5px;padding-bottom:10px;width:100%;text-align:center;">
		<input class="button_gen" type="button" onclick="hide_connection_settings();" value="<#CTL_Cancel#>">
		<input class="button_gen" type="button" onclick="save_connection_settings();" value="<#CTL_ok#>">
	</div>
</div>
<!---- connection settings end  ---->

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
			<div class="formfonttitle"><#menu5_2#> - IPTV</div>
			<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
			<div id="IPTV_desc" class="formfontdesc" style="display:none;"><#LANHostConfig_displayIPTV_sectiondesc#></div>
			<div id="IPTV_desc_DualWAN" class="formfontdesc" style="display:none;"><#LANHostConfig_displayIPTV_sectiondesc2#></div>
			<div id="IPTV_desc_DualWAN_BRTAC828" class="formfontdesc" style="display:none;">
				<#LANHostConfig_displayIPTV_sectiondescBRTAC828#>
			</div>

	  <table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
	  	<thead>
			<tr>
				<td colspan="2"><#Port_Mapping_item1#></td>
			</tr>
		</thead>
			<tr id="port_settings" style="display:none;">
				<th width="30%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,28);"><#RouterConfig_GW_LANPort_itemname#></a></th>
				<td>
					<select name="iptv_port_settings" class="input_option" onChange="change_port_settings(this.value, 1);" disabled>
						<option value="12" <% nvram_match( "iptv_port_settings", "12", "selected"); %>>LAN1/ LAN2</option>
						<option value="56" <% nvram_match( "iptv_port_settings", "56", "selected"); %>>LAN5/ LAN6</option>
					</select>
				<div id="gaming_note_div" style="display: none;"><span id="gaming_note"></span><div>
			</td>
			</tr>
		<tr id="isp_profile_tr">
		    	<th width="30%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,28);"><#Select_ISPfile#></a></th>
			<td>
				<select id="switch_wantag" name="switch_wantag" class="input_option" onChange="ISP_Profile_Selection(this.value)">
				</select>
			</td>
		</tr>
		<tr id="wan_stb_x">
		<th width="30%"><#Layer3Forwarding_x_STB_itemname#></th>
		<td align="left">
		    <select id="switch_stb_x0" name="switch_stb_x0" class="input_option" onchange="control_wans_primary(this.value);change_switch_stb(this.value);">
		    </select>
			<span id="dsl_vlan_check" style="color:#FFFFFF; display:none;"><input type="checkbox" name="dslx_rmvlan_check" id="dslx_rmvlan_check" value="" onClick="change_rmvlan();"> Remove VLAN TAG from DSL WAN</input></span>
		</td>
		</tr>
		<tr id="tr_wans_primary" style="display:none;">
			<th width="30%"><#dualwan_primary#></th>
			<td align="left">
				<span style="color:#FFFFFF;"><#dualwan_ethernet_lan#></span>
				<select id="wans_lanport1" name="wans_lanport1" class="input_option" style="margin-left:7px;"></select>
				<div style="margin-top:2px;"><span style="color:#FFFFFF;">( <#dualwan_primary_hint#> : </span><span id="cur_primary" style="color:#FFFFFF;"></span><span style="color:#FFFFFF;"> )</span></div>
			</td>
		</tr>
		<tr id="wan_iptv_x" style="display:none;">
			<th id="iptv_title" width="30%">IPTV STB Port</th>
			<td><span id="iptv_port">LAN4 </span><span><input id="iptv_settings_btn" class="button_gen" type="button" onclick="set_connection('iptv');" value="IPTV Connection"></span><span id="iptv_configure_status" style="margin-left: 5px; display:none;">Unconfigured</span></td>
		</tr>
		<tr id="wan_voip_x" style="display:none;">
			<th id="voip_title" width="30%">VoIP Port</th>
			<td><span id="voip_port">LAN3</span><span><input id="voip_settings_btn" class="button_gen" type="button" onclick="set_connection('voip');" value="VoIP Connection"><span id="voip_configure_status" style="margin-left: 5px; display:none;">Unconfigured</span></span>
			</td>
		</tr>
		<tr id="wan_bridge_x" style="display:none;">
			<th width="30%">Bridge Port</th>
			<td><span id="bridge_port">LAN4</span></td>
		</tr>
		<tr id="wan_internet_x" style="display: none;">
			<th width="30%"><#Internet#></th>
			<td>
				VID&nbsp;<input type="text" name="switch_wan0tagid" class="input_6_table" maxlength="4" value="" onKeyPress="return validator.isNumber(this, event);" autocorrect="off" autocapitalize="off" disabled>&nbsp;&nbsp;&nbsp;&nbsp;
				PRIO&nbsp;<input type="text" name="switch_wan0prio" class="input_3_table" maxlength="1" value="0" onKeyPress="return validator.isNumber(this, event);" autocorrect="off" autocapitalize="off" disabled>
			</td>
		</tr>
		<tr id="wan_iptv_port4_x" style="display: none;">
			<th id="iptv_port4" width="30%">LAN port 4</th>
			<td>
				VID&nbsp;<input type="text" name="switch_wan1tagid" class="input_6_table" maxlength="4" value="" onKeyPress="return validator.isNumber(this, event);" autocorrect="off" autocapitalize="off" disabled>&nbsp;&nbsp;&nbsp;&nbsp;
				PRIO&nbsp;<input type="text" name="switch_wan1prio" class="input_3_table" maxlength="1" value="0" onKeyPress="return validator.isNumber(this, event);" autocorrect="off" autocapitalize="off" disabled>
			</td>
		</tr>
		<tr id="wan_voip_port3_x" style="display: none;">
			<th id="voip_port3" width="30%">LAN port 3</th>
			<td>
				VID&nbsp;<input type="text" name="switch_wan2tagid" class="input_6_table" maxlength="4" value="" onKeyPress="return validator.isNumber(this, event);" autocorrect="off" autocapitalize="off" disabled>&nbsp;&nbsp;&nbsp;&nbsp;
				PRIO&nbsp;<input type="text" name="switch_wan2prio" class="input_3_table" maxlength="1" value="0" onKeyPress="return validator.isNumber(this, event);" autocorrect="off" autocapitalize="off" disabled>
			</td>
		</tr>
		</table>

		  <table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable" style="margin-top:10px;">
	  	<thead>
		<tr>
            	<td colspan="2"><#IPConnection_BattleNet_sectionname#></td>
            	</tr>
		</thead>

		<tr>
			<th><#RouterConfig_GWDHCPEnable_itemname#></th>
			<td>
				<select name="dr_enable_x" class="input_option">
				<option value="0" <% nvram_match("dr_enable_x", "0","selected"); %> ><#WLANConfig11b_WirelessCtrl_buttonname#></option>
				<option value="1" <% nvram_match("dr_enable_x", "1","selected"); %> >Microsoft</option>
				<option value="2" <% nvram_match("dr_enable_x", "2","selected"); %> >RFC3442</option>
				<option value="3" <% nvram_match("dr_enable_x", "3","selected"); %> >RFC3442 & Microsoft</option>
				</select>
			</td>
		</tr>
		<tr style="display:none;">
			<th>WAN index</th>
			<td>
				<select name="mr_mswan_idx" class="input_option">
					<option value="0" <% nvram_match("mr_mswan_idx", "0","selected"); %> >0</option>
				</select>
			</td>
		</tr>
		<tr id="mr_enable_field" style="display:none;">
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,11);"><#RouterConfig_GWMulticastEnable_itemname#></a></th>
			<td>
				<select id="mr_enable_x" name="mr_enable_x" class="input_option" onChange="change_mr_enable(this.value);">
					<option value="0" <% nvram_match("mr_enable_x", "0","selected"); %> ><#WLANConfig11b_WirelessCtrl_buttonname#></option>
				<!--	<option value="1" <% nvram_match("mr_enable_x", "1","selected"); %> ><#WLANConfig11b_WirelessCtrl_button1name#></option> -->
				</select>
				<span id="mr_hint" style="display:none;">( <#RouterConfig_GWMulticastEnable_hint#> )</span>
				<div id="mr_disable" style="display:none;">
					<span style="color:#FFF;"><#WLANConfig11b_WirelessCtrl_buttonname#></span>
					<span style="margin-left: 5px;"><#RouterConfig_GWMulticastEnable_hint2#></span>
				</div>
			</td>
		</tr>
		<tr style="display:none;">
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,14);"><#RouterConfig_IGMPver_itemname#></a></th>
			<td>
				<select name="mr_igmp_ver" class="input_option">
					<option value="1" <% nvram_match("mr_igmp_ver", "1","selected"); %> >IGMP v1</option>
					<option value="2" <% nvram_match("mr_igmp_ver", "2","selected"); %> >IGMP v2</option>
					<option value="3" <% nvram_match("mr_igmp_ver", "3","selected"); %> >IGMP v3</option>
				</select>
			</td>
		</tr>
		<tr style="display:none;">
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,15);"><#RouterConfig_MLDver_itemname#></a></th>
			<td>
				<select name="mr_mld_ver" class="input_option">
					<option value="1" <% nvram_match("mr_mld_ver", "1","selected"); %> >MLD v1</option>
					<option value="2" <% nvram_match("mr_mld_ver", "2","selected"); %> >MLD v2</option>
				</select>
			</td>
		</tr>
		<tr style="display:none;">
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,16);">Enable Fast Leave</a></th>
			<td>
				<select name="mr_qleave_x" class="input_option">
					<option value="0" <% nvram_match("mr_qleave_x", "0","selected"); %> ><#WLANConfig11b_WirelessCtrl_buttonname#></option>
					<option value="1" <% nvram_match("mr_qleave_x", "1","selected"); %> ><#WLANConfig11b_WirelessCtrl_button1name#></option>
				</select>
			</td>
		</tr>
		<tr id="enable_eff_multicast_forward" style="display:none;">
			<th><#WLANConfig11b_x_Emf_itemname#></th>
			<td>
				<select name="emf_enable" class="input_option">
					<option value="0" <% nvram_match("emf_enable", "0","selected"); %> ><#WLANConfig11b_WirelessCtrl_buttonname#></option>
					<option value="1" <% nvram_match("emf_enable", "1","selected"); %> ><#WLANConfig11b_WirelessCtrl_button1name#></option>
				</select>
			</td>
		</tr>
		<!-- 2008.03 James. patch for Oleg's patch. } -->
		<tr>
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(6, 6);"><#RouterConfig_IPTV_itemname#></a></th>
			<td>
				<input id="udpxy_enable_x" type="text" maxlength="5" class="input_6_table" name="udpxy_enable_x" value="<% nvram_get("udpxy_enable_x"); %>" onkeypress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off">
			</td>
		</tr>
		</table>

		<div class="apply_gen">
			<input class="button_gen" onclick="applyRule()" type="button" value="<#CTL_apply#>"/>
		</div>
		
	  </td>
	</tr>

	</tbody>	
  </table>		
					
		</td>
	</form>					
				</tr>
			</table>				
			<!--===================================End of Main Content===========================================-->
</td>

    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>
</body>
</html>
