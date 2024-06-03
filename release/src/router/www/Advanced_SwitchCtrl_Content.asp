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
<title><#Web_Title#> - <#Switch_itemname#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<style>
.perNode_app_table{
	width: 740px;
	position: absolute;
	left: 50%;
	margin-top: 30px;
	margin-left: -370px;
}

.businessStyle:link{
	color: #262626 !important;
}

#autowan_hint_div{
	position: absolute;
	z-index: 1000;
	width: 600px;
	height: 550px;
	margin-left: 40%;
	background-color: #232629;
	box-shadow: 3px 3px 10px #000;
	border-radius: 4px;
	border: 2px solid #818181;
	font-size: 13px;
}

.port_img{
	position: absolute;
	width: 480px;
	height: 330px;
	background: url('images/model_port.png') no-repeat center;
}

.port_plugin_img{
	position: absolute;
	width: 480px;
	height: 330px;
	background: url('images/wanport_plugin.png') no-repeat center;
}

</style>

<script>
$(document).ready(function(){
	if(isSupport("BUSINESS") && parent.webWrapper == undefined){
		$('td[bgcolor="#4D595D"]').css("background", "transparent");
		$('div[class="formfontdesc"]').css({"background-color": "#ececec", "color": "#262626", "font-weight": "bolder", "background-color": "#ececec", "padding": "15px"});
		$('input[class="button_gen"]').css({"background-color": "#FFF", "color": "#006ce1", "border": "1px solid #CCC", "border-radius": "8px", "height":"50px"});
		$('a[class="hintstyle"]').addClass("businessStyle");
	}
})

var lacp_enabled = '<% nvram_get("lacp_enabled"); %>' == 1 ?true: false;
var bonding_policy_support = false;
if( lacp_support
&& (based_modelid == "GT-AC5300" || based_modelid == "RT-AC86U" || based_modelid == "GT-AC2900" || based_modelid == "RT-AC87U" || based_modelid == "RT-AC5300" || based_modelid == "RT-AC88U" || based_modelid == "RT-AC3100")){
	bonding_policy_support = true;
	var bonding_policy_value = '<% nvram_get("bonding_policy"); %>';
}

var jumbo_frame_enable_ori = '<% nvram_get("jumbo_frame_enable"); %>';
var ctf_disable_force_ori = '<% nvram_get("ctf_disable"); %>';
var lacp_enabled_ori = '<% nvram_get("lacp_enabled"); %>';
var wans_lanport = '<% nvram_get("wans_lanport"); %>';
var iptv_port_settings_orig = '<%nvram_get("iptv_port_settings"); %>' == ""? "12": '<%nvram_get("iptv_port_settings"); %>';
var switch_wantag_orig = '<% nvram_get("switch_wantag"); %>';
var switch_stb_x_orig = '<% nvram_get("switch_stb_x"); %>';
var no_jumbo_frame_support = isSupport("no_jumbo_frame");
var wans_extwan = '<% nvram_get("wans_extwan"); %>';
var autowan_enable = '<% nvram_get("autowan_enable"); %>';
var lacp_ifnames_x = httpApi.nvramGet(["lacp_ifnames_x"], true).lacp_ifnames_x;

const bonding_port_settings = get_bonding_ports(based_modelid);
var stbPortMappings = [<% get_stbPortMappings();%>][0];

function disable_lacp_if_conflicts_with_dualwan(){
	let conflict_lanport_text = "";
	let wan_lanport_num = "";
	let hint_str1 = "<#PortConflict_DisableFunc_Reason#>";
	let hint_str2 = "";
	let note_str = "";

	if(wans_dualwan_array.indexOf("lan") != -1)
		wan_lanport_num = wans_lanport;
	else if(autowan_enable != "1" && (based_modelid == "GT10" || based_modelid == "RT-AXE7800") && wans_extwan == "1")
		wan_lanport_num = "1";
	else
		wan_lanport_num = "";

	for(var i = 0; i < bonding_port_settings.length; i++){
		if(wan_lanport_num == bonding_port_settings[i].val){
			conflict_lanport_text = bonding_port_settings[i].text.toUpperCase();
			break;
		}
	}

	if(is_GTBE_series){
		let disable_first_option = false;
		let disable_two_10g_option = false;

		if(wans_dualwan_array.indexOf("wan") != -1 && wans_extwan == "0"){//10G WAN/LAN1 conflict
			if(conflict_lanport_text == ""){
				conflict_lanport_text = "10G WAN/LAN1";
			}
			else
				conflict_lanport_text +=", 10G WAN/LAN1";

			disable_two_10g_option = true;
		}

		if(conflict_lanport_text.indexOf("LAN5") != -1)//1G LAN5
			disable_first_option = true;

		if(conflict_lanport_text.indexOf("LAN6") != -1){//10G LAN6
			disable_first_option = true;
			disable_two_10g_option = true;
		}

		if(disable_first_option && disable_two_10g_option){
			hint_str2 = "<#ChangeSettings_Hint#>".replace("setting_link", "setting_link_1");
			note_str = hint_str1.replace("%1$@", conflict_lanport_text).replace("%2$@", "WAN") + " " + hint_str2;

			document.form.lacp_enabled.style.display = "none";
			document.getElementById("lacp_note").innerHTML = note_str;
			document.getElementById("setting_link_1").href = "http://"+"<#Web_DOMAIN_NAME#>"+"/Advanced_WANPort_Content.asp";
			document.getElementById("lacp_desc").style.display = "";
			document.form.lacp_enabled.disabled = true;
		}
		else if(disable_first_option){//disable "1G LAN5 and 10G LAN6" option
			hint_str2 = "<#ChangeSettings_Hint#>".replace("setting_link", "setting_link_3");
			note_str = hint_str1.replace("%1$@", conflict_lanport_text).replace("%2$@", "WAN") + " " + hint_str2;

			$("input:radio[name='lacp_port_select']").filter('[value="0"]').attr('disabled', true).css("cursor", "unset");
			$("#disable_first_option_hint").html(note_str);
			$("#disable_first_option_hint").show();
			$("#lacp_first_option").css("color", "#848c98");
			$("#setting_link_3").attr("href", "http://"+"<#Web_DOMAIN_NAME#>"+"/Advanced_WANPort_Content.asp");
		}
		else if(disable_two_10g_option){// disable two 10g opton
			hint_str2 = "<#ChangeSettings_Hint#>".replace("setting_link", "setting_link_4");
			note_str = hint_str1.replace("%1$@", conflict_lanport_text).replace("%2$@", "WAN") + " " + hint_str2;

			$("input:radio[name='lacp_port_select']").filter('[value="1"]').attr('disabled', true).css("cursor", "unset");
			$("#disable_two_10g_hint").html(note_str);
			$("#disable_two_10g_hint").show();
			$("#two_10g_lacp").css("color", "#848c98");
			$("#setting_link_4").attr("href", "http://"+"<#Web_DOMAIN_NAME#>"+"/Advanced_WANPort_Content.asp");
		}

	}
	else if(conflict_lanport_text != ""){
		hint_str2 = "<#ChangeSettings_Hint#>".replace("setting_link", "setting_link_1");
		note_str = hint_str1.replace("%1$@", conflict_lanport_text).replace("%2$@", "WAN") + " " + hint_str2;

		document.form.lacp_enabled.style.display = "none";
		document.getElementById("lacp_note").innerHTML = note_str;
		document.getElementById("setting_link_1").href = "http://"+"<#Web_DOMAIN_NAME#>"+"/Advanced_WANPort_Content.asp";
		document.getElementById("lacp_desc").style.display = "";
		document.form.lacp_enabled.disabled = true;
	}
}

function disable_lacp_if_conflicts_with_iptv(){
	let hint_str1 = "<#PortConflict_SamePort_Hint#>";
	let hint_str2 = "";
	let note_str = "";
	let disable_lacp = false;

	for(var i = 0; i < bonding_port_settings.length; i++){
		if(bonding_port_settings[i].val == switch_stb_x_orig){
			disable_lacp = true;
			break;
		}
		else{
			for(var j = 0; j < stbPortMappings.length; j++){
				if(switch_stb_x_orig == stbPortMappings[j].value && stbPortMappings[j].comboport_value_list.length != 0){
					var value_list = stbPortMappings[j].comboport_value_list.split(" ");
					for(var k = 0; k < value_list.length; k++){
						if(bonding_port_settings[i].val == value_list[k]){
							disable_lacp = true;
							break;
						}
					}
				}

				if(disable_lacp)
					break;
			}
		}
	}

	if(disable_lacp){
		hint_str2 = "<#ChangeSettings_Hint#>".replace("setting_link", "setting_link_2");
		note_str = hint_str1.replace("%1$@", "<#NAT_lacp#>").replace("%2$@", "IPTV") + " " + hint_str2
		document.form.lacp_enabled.style.display = "none";
		document.getElementById("lacp_note").innerHTML = note_str;
		document.getElementById("setting_link_2").href = "http://"+"<#Web_DOMAIN_NAME#>"+"/Advanced_IPTV_Content.asp";
		document.getElementById("lacp_desc").style.display = "";
		document.form.lacp_enabled.value = "0";
	}
}

function initial(){
	if((based_modelid == "RT-AX89U" || based_modelid == "GT-AXY16000")){
			document.form.aqr_hwnat_type.disabled = false;
			document.form.aqr_link_speed.disabled = false;
			document.form.aqr_ipg.disabled = false;
			document.form.sfpp_hwnat_type.disabled = false;
			document.form.sfpp_max_speed.disabled = false;
			document.form.sfpp_force_on.disabled = false;
			document.getElementById("aqr_hwnat_type_tr").style.display = "";
			document.getElementById("aqr_link_speed_tr").style.display = "";
			document.getElementById("aqr_ipg_tr").style.display = "";
			document.getElementById("sfpp_hwnat_type_tr").style.display = "";
			document.getElementById("sfpp_max_speed_tr").style.display = "";
			document.getElementById("sfpp_force_on_tr").style.display = "";
	}
	if(qca_support){
		var nataccel = '<% nvram_get("qca_sfe"); %>';
		var nataccel_status = '<% nat_accel_status(); %>';

		if(nataccel == '1' && nataccel_status == '1'){
			document.getElementById("natAccelDesc").innerHTML = "<#NAT_Acceleration_enable#>";
		}
		else{
			document.getElementById("natAccelDesc").innerHTML = "<#NAT_Acceleration_ctf_disable#>";
		}
	}
	else if(mtk_support){
		var nataccel = '<% nvram_get("hwnat"); %>';
		var nataccel_status = '<% nat_accel_status(); %>';

		if(nataccel == '1' && nataccel_status == '1'){
			document.getElementById("MTKnatAccelDesc").innerHTML = "<#NAT_Acceleration_enable#>";
		}
		else{
			document.getElementById("MTKnatAccelDesc").innerHTML = "<#NAT_Acceleration_ctf_disable#>";
		}
	}
	else{
		var ctf_disable = '<% nvram_get("ctf_disable"); %>';
		var ctf_fa_mode = '<% nvram_get("ctf_fa_mode"); %>';

		if(ctf_disable == 1){
			document.getElementById("ctfLevelDesc").innerHTML = "<#NAT_Acceleration_ctf_disable#>";
		}
		else{
			if(ctf_fa_mode == '2')
				document.getElementById("ctfLevelDesc").innerHTML = "<#NAT_Acceleration_ctf_fa_mode2#>";
			else
				document.getElementById("ctfLevelDesc").innerHTML = "<#NAT_Acceleration_ctf_fa_mode1#>";
		}
	}

	if (re_mode == "1"){
		$("#tabMenu").addClass("perNode_app_table");
		$(".submenuBlock").css("margin-top", "initial");
		show_loading_obj();
	} else {
		$("#content_table").addClass("content");
		$("#tabMenu").addClass("app_table app_table_usb");
		show_menu();
	}

	$("#tabMenu").css("display", "");

	if(lacp_support){
		document.getElementById("lacp_tr").style.display = "";
		document.form.lacp_enabled.disabled = false;
		if(lacp_enabled){
			document.getElementById("lacp_desc").style.display = "";
			if(bonding_policy_support){
				document.form.bonding_policy.value = bonding_policy_value;
			}
			show_related_settings(document.form.lacp_enabled);
		}
		else
			document.getElementById("lacp_desc").style.display = "none";
	}
	else{
		document.form.lacp_enabled.disabled = true;
		document.form.bonding_policy.disabled = true;
	}

	if(qca_support){
		if(lyra_hide_support){
			document.getElementById("jumbo_tr").style.display = "none";
			document.form.jumbo_frame_enable.disabled = true;
		}

		if(wifison_ready != "1" && sw_mode == "1"){
			document.getElementById("qca_tr").style.display = "";
			document.form.qca_sfe.disabled = false;
		}
	}
	else if(mtk_support || based_modelid == "RT-ACRH18" || based_modelid == "4G-AC86U" || based_modelid == "4G-AX56" || based_modelid == "RT-AX53U" || based_modelid == "RT-AX54" || based_modelid == "XD4S"){//MTK
		document.getElementById("mtk_tr").style.display = "";
		document.form.hwnat.disabled = false;
		document.getElementById("ctf_tr").style.display = "none";
		document.form.ctf_disable_force.disabled = true;
	}
	else{
		//MODELDEP
		if(bonding_port_settings.length == 2){
			var new_str = "";
			new_str = document.getElementById("lacp_note").innerHTML.replace(/LAN2/g, bonding_port_settings[1].text);
			document.getElementById("lacp_note").innerHTML = new_str.replace(/LAN1/g, bonding_port_settings[0].text);
		}

		if(hnd_support){
			document.getElementById("ctf_tr").style.display = "none";
			document.form.ctf_disable_force.disabled = true;
		}
		else{
			if(sw_mode == "1" || sw_mode == "5"){
				document.getElementById("ctf_tr").style.display = "";
				document.form.ctf_disable_force.disabled = false;
			}
		}
	}

	if(lacp_support){
		disable_lacp_if_conflicts_with_dualwan();
		if(!is_GTBE_series)
			disable_lacp_if_conflicts_with_iptv();

		if(based_modelid == "RT-AXE7800" && wan_bonding_support){
			var bond_wan = httpApi.nvramGet(["bond_wan"], true).bond_wan;
			if(bond_wan == "1"){
				document.form.lacp_enabled.style.display = "none";
				document.getElementById("lacp_note").innerHTML = note_str;
				document.getElementById("lacp_desc").style.display = "";
				document.form.lacp_enabled.value = "0";
			}
		}

		if(is_GTBE_series && document.form.lacp_enabled.disabled == false){
			var hint_str = "Enable Bonding (802.3ad) support for your wired client and then connect it to your Router.";
			$("#lacp_note").html(hint_str);

			if(lacp_enabled_ori == "1"){
				if(lacp_ifnames_x == "eth0 eth3")
					document.form.lacp_port_select[1].checked = true;
				else
					document.form.lacp_port_select[0].checked = true;
			}
		}
	}

	if(no_jumbo_frame_support)
		$("#jumbo_tr").hide();
}

function applyRule(){
	var setting_changed = false;
	if((jumbo_frame_enable_ori != document.form.jumbo_frame_enable.value)
	|| (!document.form.ctf_disable_force.disabled && ctf_disable_force_ori != document.form.ctf_disable_force.value)
	|| (lacp_enabled_ori != document.form.lacp_enabled.value) ){
		setting_changed = true
	}

	if(based_modelid == "GT-AC5300" && (lacp_enabled_ori != document.form.lacp_enabled.value)){
		var msg = "Enable Bonding/ Link aggregation will change settings of IPTV/ VOIP ports on IPTV page to LAN1/ LAN2. Are you sure to do it?";//untranslated
		if(document.form.lacp_enabled.value == "1" && iptv_port_settings_orig == "56"){
			if(confirm(msg)){
				document.form.iptv_port_settings.disabled = false;
				document.form.iptv_port_settings.value = "12";
			}
			else{
				document.form.lacp_enabled.value = "0";
				show_related_settings(document.form.lacp_enabled);
				document.form.lacp_enabled.focus();
				return;
			}
		}
	}

	var wan_lacp_conflict = false;
	if(is_GTBE_series){
		if(lacp_support && document.form.lacp_enabled.value == "1"){
			if(document.form.lacp_port_select[1].checked){
				if(isSupport("autowan")){
					if(autowan_enable == "1" || (autowan_enable == "0" && wans_dualwan_array.indexOf("wan") != -1 && wans_extwan == "0"))
						wan_lacp_conflict = true;
				}
				else if(wans_dualwan_array.indexOf("wan") != -1 && wans_extwan == "0"){
					wan_lacp_conflict = true;
				}

				if(lacp_ifnames_x == ""){
					$('<input>').attr({
						type: 'hidden',
						name: "lacp_ifnames_x",
						value: "eth0 eth3"
					}).appendTo('form');
					setting_changed = true;
				}
			}
			else{
				if(lacp_ifnames_x == "eth0 eth3"){
					$('<input>').attr({
						type: 'hidden',
						name: "lacp_ifnames_x",
						value: ""
					}).appendTo('form');
					setting_changed = true;
				}
			}
		}
	}
	else{
		if(isSupport("autowan") && autowan_enable == "1")
			wan_lacp_conflict = true;
	}

	if(!setting_changed){	// only change the bonding policy
		document.form.action_script.value = "restart_net_and_phy";
		document.form.action_wait.value = "35";
	}

	if(lantiq_support){
		document.form.action_script.value = "restart_wan_if;restart_firewall";
		document.form.action_wait.value = "10";
	
		if(!setting_changed){	// only change the bonding policy
			document.form.action_script.value += ";restart_net_and_phy";
			document.form.action_wait.value = "35";
		}
	}

	if(wan_lacp_conflict){
		var hint_str = "To ensure that there are no conflicts, when you enable %1$@, the WAN port will be change to %2$@ only. Please make sure that your WAN cable is correctly plugged into the %2$@. Are you sure to continue?"
		var msg = "";
		if(is_GTBE_series)
			msg = hint_str.replace("%1$@", "<#NAT_lacp#>").replaceAll("%2$@", "2.5G WAN");
		else
			msg = hint_str.replace("%1$@", "<#NAT_lacp#>").replaceAll("%2$@", get_default_wan_name());

		$("#autowan_hint").html(msg);
		$("#autowan_hint_div").show();
		if(check_file_exists('images/model_port.png') && check_file_exists('images/wanport_plugin.png')){
			setTimeout(function(){
					if($(".port_plugin_img").is(":visible"))
						$(".port_plugin_img").fadeOut(500);
					else
						setTimeout(function(){$(".port_plugin_img").fadeIn(500);}, 500);

					if($("#autowan_hint_div").is(":visible"))
						setTimeout(arguments.callee, 1500);
				}, 1500);
		}
		else{
			$("#schematic_diagram").hide();
			$("#autowan_hint_div").css("height", "auto");
			$("#autowan_hint").css("margin", "50px 0");
			$("#hint_action_div").css("margin-bottom", "30px");
		}
	}
	else{
		if(document.form.action_script.value == "reboot"){
			if(confirm("<#AiMesh_Node_Reboot#>")){
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

function show_related_settings(obj){
	if(obj.value == "1"){
		if(bonding_policy_support){
			document.getElementById("lacp_policy_tr").style.display = "";
			document.form.bonding_policy.disabled = false;
		}

		if(is_GTBE_series)
			$("#lacp_port_selection").css("display", "flex");

		document.getElementById("lacp_desc").style.display = "";
	}
	else{
		if(bonding_policy_support){
			document.getElementById("lacp_policy_tr").style.display = "none";
			document.form.bonding_policy.disabled = true;
		}

		document.getElementById("lacp_desc").style.display = "none";
		if(is_GTBE_series)
			$("#lacp_port_selection").hide();
	}
}

function close_autowan_hint(){
	$("#autowan_hint_div").hide();
}

function confirm_autowan_change(){
	$("#autowan_hint_div").hide();

	$('<input>').attr({
		type: 'hidden',
		name: "autowan_enable",
		value: "0"
	}).appendTo('form');

	$('<input>').attr({
		type: 'hidden',
		name: "wans_dualwan",
		value: "wan none"
	}).appendTo('form');

	if(is_GTBE_series){
		$('<input>').attr({
			type: 'hidden',
			name: "wans_extwan",
			value: "1"
		}).appendTo('form');

		$('<input>').attr({
			type: 'hidden',
			name: "lacp_ifnames_x",
			value: "eth0 eth3"
		}).appendTo('form');
	}
	else{
		$('<input>').attr({
			type: 'hidden',
			name: "wans_extwan",
			value: "0"
		}).appendTo('form');
	}

	setTimeout(function(){
				if(document.form.action_script.value == "reboot"){
					if(confirm("<#AiMesh_Node_Reboot#>")){
						showLoading();
						document.form.submit();
					}
				}
				else{
					showLoading();
					document.form.submit();
				}
			}, 100);
}

function isEmpty(obj)
{
	for (var name in obj){
		return false;
	}

	return true;
};

function get_default_wan_name(){
	var default_wan_name = "WAN";
	var eth_wan_list = httpApi.hookGet("get_ethernet_wan_list", true);

	if(!isEmpty(eth_wan_list)){
		default_wan_name = eth_wan_list["wan"].wan_name;
	}

	return default_wan_name;
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
<input type="hidden" name="current_page" value="Advanced_SwitchCtrl_Content.asp">
<input type="hidden" name="next_page" value="Advanced_SwitchCtrl_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="reboot">
<input type="hidden" name="action_wait" value="<% nvram_get("reboot_time"); %>">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="iptv_port_settings" value="<% nvram_get("iptv_port_settings"); %>" disabled>
<input type="hidden" name="sfpp_force_on" value="<% nvram_get("sfpp_force_on"); %>" disabled>
<!--===================================Beginning of Auto WAN Detection Confirm===========================================-->
<div id="autowan_hint_div" style="display: none;">
	<div style="width: 100%; height: 100%; display: flex; flex-direction: column; justify-content: space-evenly; align-items: center;">
		<div id="autowan_hint" style="width: 80%; line-height: 20px;"></div>
		<div id="schematic_diagram" style="width: 480px; height: 330px;">
			<div class="port_img"></div>
			<div class="port_plugin_img"></div>
		</div>
		<div id="hint_action_div" style="display: flex; width: 80%; justify-content: space-evenly; margin-bottom: 30px;">
			<input class="button_gen" type="button" value="<#CTL_Cancel#>" onclick="close_autowan_hint();">
			<input class="button_gen" type="button" value="<#CTL_ok#>" onclick="confirm_autowan_change();">
		</div>
	</div>
</div>
<!--===================================End of Auto WAN Detection Confirm===========================================-->
<table id="content_table" align="center" cellspacing="0" style="margin:auto;">
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
				  						<div class="formfonttitle"><#menu5_2#> - <#Switch_itemname#></div>
		      							<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
										<div class="formfontdesc"><#SwitchCtrl_desc#></div>
										<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
											<tr id="jumbo_tr">
												<th><#jumbo_frame#></th>
												<td>
													<select name="jumbo_frame_enable" class="input_option">
														<option class="content_input_fd" value="0" <% nvram_match("jumbo_frame_enable", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
														<option class="content_input_fd" value="1" <% nvram_match("jumbo_frame_enable", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
													</select>
												</td>
											</tr>

											<tr id="ctf_tr" style="display: none;">
		      									<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(29,2);"><#NAT_Acceleration#></a></th>
												<td>
													<select name="ctf_disable_force" class="input_option" disabled>
														<option class="content_input_fd" value="1" <% nvram_match("ctf_disable_force", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
														<option class="content_input_fd" value="0" <% nvram_match("ctf_disable_force", "0","selected"); %>><#Auto#></option>
													</select>
													&nbsp
													<span id="ctfLevelDesc"></span>
												</td>
											</tr>

											<tr id="qca_tr" style="display: none;">
												<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(29,2);"><#NAT_Acceleration#></a></th>
												<td>
													<select name="qca_sfe" class="input_option" disabled>
														<option class="content_input_fd" value="0" <% nvram_match("qca_sfe", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
														<option class="content_input_fd" value="1" <% nvram_match("qca_sfe", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
													</select>
												&nbsp
													<span id="natAccelDesc"></span>
												</td>
											</tr>

											<tr id="mtk_tr" style="display: none;">
												<th><#NAT_Acceleration#></th>
												<td>
													<select name="hwnat" class="input_option" disabled>
														<option class="content_input_fd" value="0" <% nvram_match("hwnat", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
														<option class="content_input_fd" value="1" <% nvram_match("hwnat", "1","selected"); %>><#Auto#></option>
													</select>
													&nbsp
												<span id="MTKnatAccelDesc"></span>
												</td>
											</tr>

											<tr style="display:none">
												<th><#SwitchCtrl_Enable_GRO#></th>
												<td>
													<input type="radio" name="gro_disable_force" value="0" <% nvram_match("gro_disable_force", "0", "checked"); %>><#checkbox_Yes#>
													<input type="radio" name="gro_disable_force" value="1" <% nvram_match("gro_disable_force", "1", "checked"); %>><#checkbox_No#>
												</td>
											</tr>       

											<tr>
											<th>Spanning-Tree Protocol</th>
												<td>
													<select name="lan_stp" class="input_option">
														<option class="content_input_fd" value="0" <% nvram_match("lan_stp", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
														<option class="content_input_fd" value="1" <% nvram_match("lan_stp", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
													</select>
												</td>
											</tr>

											<tr id="aqr_hwnat_type_tr" style="display:none">
												<th>10G base-T port acceleration type</th><!--untranslated-->
												<td>
													<select name="aqr_hwnat_type" class="input_option" disabled>
														<option value="0" <% nvram_match("aqr_hwnat_type", "0","selected"); %>><#Auto#></option>
														<option value="1" <% nvram_match("aqr_hwnat_type", "1","selected"); %>>PPE + NSS</option>
														<option value="2" <% nvram_match("aqr_hwnat_type", "2","selected"); %>>NSS</option>
													</select>
												</td>
											</tr>

											<tr id="aqr_link_speed_tr" style="display:none">
												<th>10G base-T port link speed</th><!--untranslated-->
												<td>
													<select name="aqr_link_speed" class="input_option" disabled>
														<option value="0" <% nvram_match("aqr_link_speed", "0","selected"); %>><#Auto#></option>
														<option value="1000" <% nvram_match("aqr_link_speed", "1000","selected"); %>>1Gbps</option>
														<option value="2500" <% nvram_match("aqr_link_speed", "2500","selected"); %>>2.5Gbps</option>
														<option value="5000" <% nvram_match("aqr_link_speed", "5000","selected"); %>>5Gbps</option>
														<option value="10000" <% nvram_match("aqr_link_speed", "10000","selected"); %>>10Gbps</option>
													</select>
												</td>
											</tr>

											<tr id="aqr_ipg_tr" style="display:none">
												<th>10G base-T interpacket gap</th><!--untranslated-->
												<td>
													<select name="aqr_ipg" class="input_option" disabled>
														<option value="96" <% nvram_match("aqr_ipg", "96","selected"); %>><#CTL_Default#></option>
														<option value="128" <% nvram_match("aqr_ipg", "128","selected"); %>>128 bit times</option>
													</select>
												</td>
											</tr>

											<tr id="sfpp_hwnat_type_tr" style="display:none">
												<th>SFP+ port acceleration type</th><!--untranslated-->
												<td>
													<select name="sfpp_hwnat_type" class="input_option" disabled>
														<option value="0" <% nvram_match("sfpp_hwnat_type", "0","selected"); %>><#Auto#></option>
														<option value="1" <% nvram_match("sfpp_hwnat_type", "1","selected"); %>>PPE + NSS</option>
														<option value="2" <% nvram_match("sfpp_hwnat_type", "2","selected"); %>>NSS</option>
													</select>
												</td>
											</tr>

											<tr id="sfpp_max_speed_tr" style="display:none">
												<th>SFP+ port maximum link speed</th><!--untranslated-->
												<td>
													<select name="sfpp_max_speed" class="input_option" disabled>
														<option value="0" <% nvram_match("sfpp_max_speed", "0","selected"); %>><#Auto#></option>
														<option value="1000" <% nvram_match("sfpp_max_speed", "1000","selected"); %>>1Gbps</option>
														<option value="10000" <% nvram_match("sfpp_max_speed", "10000","selected"); %>>10Gbps</option>
													</select>
												</td>
											</tr>

											<tr id="sfpp_force_on_tr" style="display:none">
												<th>SFP+ port TX clock</th><!--untranslated-->
												<td>
													<input type="radio" name="sfpp_force_on" value="0" <% nvram_match("sfpp_force_on", "0", "checked"); %>><#Auto#>
													<input type="radio" name="sfpp_force_on" value="1" <% nvram_match("sfpp_force_on", "1", "checked"); %>>ON<!--untranslated-->
												</td>
											</tr>

											<tr id="lacp_tr" style="display:none;">
		      									<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(29,1);"><#NAT_lacp#></a></th>
												<td>
													<select name="lacp_enabled" class="input_option"  onchange="show_related_settings(this);" disabled>
														<option class="content_input_fd" value="0" <% nvram_match("lacp_enabled", "0","selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
														<option class="content_input_fd" value="1" <% nvram_match("lacp_enabled", "1","selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
													</select>
													<div id="lacp_desc" style="display:none; margin-top: 5px;"><span id="lacp_note"><#NAT_lacp_note#></span></div>
													<div id = "lacp_port_selection" style="display: none; flex-direction: column; margin-top: 5px;">
														<div style="display: flex;"><input type="radio" name="lacp_port_select" value="0" checked><div id="lacp_first_option">1G LAN5 and 10G LAN6</div></div>
														<div id= "disable_first_option_hint" style="margin-left: 20px; color: #FFCC00;"></div>
														<div style="display: flex;"><div><input type="radio" name="lacp_port_select" value="1"></div><div id="two_10g_lacp">10G WAN/LAN1 and 10G LAN6</div></div>
														<div id= "disable_two_10g_hint" style="margin-left: 20px; color: #FFCC00;"></div>
													</div>
												</td>
											</tr> 
											<tr id="lacp_policy_tr" style="display:none">
												<th><#SwitchCtrl_BondingPolicy#></th>
												<td>
													<select name="bonding_policy" class="input_option" disabled>
														<option value="0"><#CTL_Default#></option>
														<option value="1"><#SwitchCtrl_BondingPolicy_src#></option>
														<option value="2"><#SwitchCtrl_BondingPolicy_dest#></option>
													</select>
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
				</tr>
			</table>				
		</td>
	    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
</form>
<div id="footer"></div>
</body>
</html>
