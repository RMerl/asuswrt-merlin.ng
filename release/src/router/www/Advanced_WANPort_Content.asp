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
<title><#Web_Title#> - <#dualwan#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="/js/table/table.css">
<style>
.ISPProfile{
	display:none;
}
#ClientList_Block_PC{
	border:1px outset #999;
	background-color:#576D73;
	position:absolute;
	*margin-top:26px;	
	margin-left:2px;
	*margin-left:-353px;
	width:346px;
	text-align:left;	
	height:auto;
	overflow-y:auto;
	z-index:200;
	padding: 1px;
	display:none;
}
#ClientList_Block_PC div{
	background-color:#576D73;
	height:auto;
	*height:20px;
	line-height:20px;
	text-decoration:none;
	font-family: Lucida Console;
	padding-left:2px;
}

#ClientList_Block_PC a{
	background-color:#EFEFEF;
	color:#FFF;
	font-size:12px;
	font-family:Arial, Helvetica, sans-serif;
	text-decoration:none;	
}
#ClientList_Block_PC div:hover{
	background-color:#3366FF;
	color:#FFFFFF;
	cursor:default;
}

#detect_time_confirm{
	position:absolute;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius: 5px;
	z-index:20000;
	margin-left: 40%;
	margin-top:10%;
	background-color:#232629;
	width:400px;
	box-shadow: 3px 3px 10px #000;
	font:13px Arial, Helvetica, sans-serif;
}
</style>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/validator.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" language="JavaScript" src="/js/table/table.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script>


var wans_caps = '<% nvram_get("wans_cap"); %>';
if(based_modelid == "GT-AX11000" || based_modelid == "RT-AX86U" || based_modelid == "GT-AXE11000")
	wans_caps = wans_caps + " lan2";
var wans_routing_rulelist_array = [];
var wans_flag;
var switch_stb_x = '<% nvram_get("switch_stb_x"); %>';
var wans_caps_primary;
var wans_caps_secondary;
var wandog_fb_count_orig = '<% nvram_get("wandog_fb_count"); %>';
var wandog_maxfail_orig = '<% nvram_get("wandog_maxfail"); %>';
var mobile_enable_orig = '';
if(gobi_support){
	if(usb_index == 0)
		mobile_enable_orig = '<% nvram_get("wan0_enable"); %>';
	else if(usb_index == 1)
		mobile_enable_orig = '<% nvram_get("wan1_enable"); %>';
}
var wans_mode_orig = '<% nvram_get("wans_mode"); %>';
var wans_standby_orig = '<% nvram_get("wans_standby"); %>';
var min_detect_interval = 2;
var min_fo_detect_count = 5;
var dns_probe_timeout_threshold = (httpApi.nvramGet(["dns_probe_timeout"], true).dns_probe_timeout > 0)? parseInt(httpApi.nvramGet(["dns_probe_timeout"], true).dns_probe_timeout)+1 : 1;


var country = new Array("None", "China");
var country_n_isp = new Array;
country_n_isp[0] = new Array("");
country_n_isp[1] = new Array("edu","telecom","mobile","unicom");
var curState = (wans_dualwan_array[1] != "none")? "1":"0";
var lacp_support = isSupport("lacp");
var lacp_enabled = '<% nvram_get("lacp_enabled"); %>';
var wans_lanport_orig = '<% nvram_get("wans_lanport"); %>';
var wanports_bond = '<% nvram_get("wanports_bond"); %>';
if(wan_bonding_support)
	var orig_bond_wan = httpApi.nvramGet(["bond_wan"], true).bond_wan;


function initial(){
	show_menu();
	wans_flag = (wans_dualwan_orig.search("none") != -1 || !parent.dualWAN_support) ? 0 : 1;
	if(wan_bonding_support){
		if(orig_bond_wan == 1 && (based_modelid == "RT-AX89U" || based_modelid == "GT-AXY16000")){
			// Remove 10G base-T if it's aggregated w/ WAN port
			var i = wans_caps.split(" ").indexOf("wan2");
			if(i != -1 && wanports_bond.split(" ").indexOf("30") != -1){
				var new_wans_cap = wans_caps.split(" ");
				new_wans_cap.splice(i, 1);
				wans_caps = new_wans_cap.toString().replace(/,/g," ");
			}
		}
	}
	wans_caps_primary = wans_caps;
	wans_caps_secondary = wans_caps;
	
	addWANOption(document.form.wans_primary, wans_caps_primary.split(" "));
	addWANOption(document.form.wans_second, wans_caps_secondary.split(" "));

	httpApi.faqURL("1011719", function(url){document.getElementById("dualwan_faq").href=url;});
	httpApi.faqURL("1037368", function(url){document.getElementById("network_detect_faq").href=url;});

   	document.form.wans_mode.value = wans_mode_orig;

   	//parse nvram to array
	var parseNvramToArray = function() {
		var parseArray = [];
		var oriNvram = '<% nvram_char_to_ascii("","wans_routing_rulelist"); %>';
		var oriNvramRow = decodeURIComponent(oriNvram).split('<');
		for(var i = 0; i < oriNvramRow.length; i += 1) {
			if(oriNvramRow[i] != "") {
				var oriNvramCol = oriNvramRow[i].split('>');
				var eachRuleArray = new Array();
				for(var j = 0; j < oriNvramCol.length; j += 1) {
					eachRuleArray.push(oriNvramCol[j]);
				}
				parseArray.push(eachRuleArray);
			}
		}
		return parseArray;
	};

	wans_routing_rulelist_array = parseNvramToArray();
	form_show(wans_flag);		
	setTimeout("showLANIPList();", 1000);

	if(based_modelid == "RT-AC87U"){ //MODELDEP: RT-AC87 : Quantenna port
                document.form.wans_lanport1.remove(0);   //Primary LAN1
                document.form.wans_lanport2.remove(0);   //Secondary LAN1
	}else if(based_modelid == "RT-N19"){
		document.form.wans_lanport1.remove(3);
		document.form.wans_lanport1.remove(2);
		document.form.wans_lanport2.remove(3);
		document.form.wans_lanport2.remove(2);
	}else if(based_modelid == "RT-AC95U" || based_modelid == "RT-AX95Q"){
		document.form.wans_lanport1.remove(3);
		document.form.wans_lanport2.remove(3);
	}

	if(based_modelid == "GT-AC5300"){ //MODELDEP: GT-AC5300 : TRUNK PORT
		var desc = ["LAN Port 1", "LAN Port 2", "LAN Port 5", "LAN Port 6"];
		var value = ["2", "1", "4", "3"];
		var current_value = '<% nvram_get("wans_lanport"); %>';
		add_options_x2(document.form.wans_lanport1, desc, value, current_value);
		add_options_x2(document.form.wans_lanport2, desc, value, current_value);
	}

	if(based_modelid == "RT-AC88Q" || based_modelid == "BRT-AC828" || based_modelid == "RT-AD7200") {
		var i;
		var arr = new Array(), varr = new Array();
		for(i=5;i<=8;i++)
		{
			arr.push("LAN Port " + i);
			varr.push(i);
		}
		add_options_x2(document.form.wans_lanport1, arr, varr, <% nvram_get("wans_lanport"); %>);
		add_options_x2(document.form.wans_lanport2, arr, varr, <% nvram_get("wans_lanport"); %>);
	}

	if(wan_bonding_support)
		$("#wan_aggre_desc").css("display", "");
}

function form_show(v){
	if(v == 0){ //DualWAN disabled
		inputCtrl(document.form.wans_second, 0);
		inputCtrl(document.form.wans_lb_ratio_0, 0);
		inputCtrl(document.form.wans_lb_ratio_1, 0);
		inputCtrl(document.form.wans_isp_unit[0], 0);
		inputCtrl(document.form.wans_isp_unit[1], 0);
		inputCtrl(document.form.wans_isp_unit[2], 0);
		inputCtrl(document.form.wan0_isp_country, 0);
		inputCtrl(document.form.wan0_isp_list, 0);
		inputCtrl(document.form.wan1_isp_country, 0);
		inputCtrl(document.form.wan1_isp_list, 0);		
		document.form.wans_routing_enable[1].checked = true;		
		document.form.wans_routing_enable[0].disabled = true;
		document.form.wans_routing_enable[1].disabled = true;
		document.getElementById('watchdog_table').style.display = "none";
		document.getElementById('Routing_rules_table').style.display = "none";
		if(based_modelid == "GT-AX11000" || based_modelid == "RT-AX86U" || based_modelid == "GT-AXE11000"){
			if( wans_dualwan_array[0] == "wan" && document.form.wans_extwan.value == "1"){
				document.form.wans_primary.value = "lan2";
			}
			else{
				document.form.wans_primary.value = wans_dualwan_array[0];
			}			
		}
		else{
			document.form.wans_primary.value = wans_dualwan_array[0];
		}
			
		appendLANoption1(document.form.wans_primary);
		document.getElementById("wans_mode_tr").style.display = "none";
		document.getElementById("wandog_fb_count_tr").style.display = "none";	
		document.getElementById("routing_table").style.display = "none";
		document.getElementById("wans_standby_tr").style.display = "none";
		inputCtrl(document.form.wans_standby, 0);
	}
	else{ //DualWAN enabled
		if(based_modelid == "GT-AX11000" || based_modelid == "RT-AX86U" || based_modelid == "GT-AXE11000"){
			if( (wans_dualwan_array[0] == "wan" && document.form.wans_extwan.value == "1") ||
				(wans_dualwan_array[0] == "lan" && document.form.wans_lanport.value == "5" && document.form.wans_extwan.value == "0") ){
				var wans_caps_tmp = wans_caps_secondary.split(" ");
				wans_caps_tmp = wans_caps_tmp.slice(1);
				addWANOption(document.form.wans_second, wans_caps_tmp);
				document.form.wans_primary.value = "lan2";
			}
			else{
				document.form.wans_primary.value = wans_dualwan_array[0];
			}
		}
		else{
			document.form.wans_primary.value = wans_dualwan_array[0];
		}
		
		if(wans_dualwan_array[1] == "none"){

			if(wans_dualwan_array[0] == "dsl"){
				
				if(wans_caps.search("wan") >= 0)
					document.form.wans_second.value = "wan";
				else if(wans_caps.search("wan2") >= 0)
					document.form.wans_second.value = "wan2";
				else if(wans_caps.search("sfp+") >= 0)
					document.form.wans_second.value = "sfp+";
				else if(wans_caps.search("usb") >= 0)
					document.form.wans_second.value = "usb";
				else
					document.form.wans_second.value = "lan";
			}
			else if(wans_dualwan_array[0] == "wan"){

				if(wans_caps.search("wan2") >= 0)
					document.form.wans_second.value = "wan2";
				else if(wans_caps.search("sfp+") >= 0)
					document.form.wans_second.value = "sfp+";
				else if(wans_caps.search("usb") >= 0)
					document.form.wans_second.value = "usb";
				else
					document.form.wans_second.value = "lan";
			}
			else if((wans_dualwan_array[0] == "wan" && document.form.wans_extwan.value == "1") ||
					(wans_dualwan_array[0] == "lan" && document.form.wans_lanport.value == "5" && document.form.wans_extwan.value == "0")){

				if(wans_caps.search("usb") >= 0)
					document.form.wans_second.value = "usb";
				else
					document.form.wans_second.value = "lan";
			}
			else{
				if(wans_caps.search("wan") >= 0)
					document.form.wans_second.value = "wan";
			}
		}	
		else{
			if(based_modelid == "GT-AX11000" || based_modelid == "RT-AX86U" || based_modelid == "GT-AXE11000"){
				if( (wans_dualwan_array[1] == "wan" && document.form.wans_extwan.value == "1") ||
					(wans_dualwan_array[1] == "lan" && document.form.wans_lanport.value == "5" && document.form.wans_extwan.value == "0"))
					document.form.wans_second.value = "lan2";
				else{
					document.form.wans_second.value = wans_dualwan_array[1];
				}
			}
			else{
				document.form.wans_second.value = wans_dualwan_array[1];
			}
		}
		
		appendLANoption1(document.form.wans_primary);
		appendLANoption2(document.form.wans_second);

		var replace_html = '<input type="text" name="wandog_interval" class="input_3_table" maxlength="2" value="<% nvram_get("wandog_interval"); %>" onblur="update_consume_bytes();" onKeyPress="return validator.isNumber(this, event);" placeholder="5" autocorrect="off" autocapitalize="off">';
		var new_html_str = document.getElementById("retry_intervale_setting").innerHTML.replace("$INPUT_INTERVAL", replace_html);
		document.getElementById("retry_intervale_setting").innerHTML = new_html_str;

		replace_html = '<input type="text" name="wandog_maxfail" class="input_3_table" maxlength="2" value="<% nvram_get("wandog_maxfail"); %>" onKeyPress="return validator.isNumber(this, event);" placeholder="5" autocorrect="off" autocapitalize="off">';
		new_html_str = document.getElementById("wandog_maxfail_setting").innerHTML.replace("$WANDOG_MAXFAIL", replace_html);
		document.getElementById("wandog_maxfail_setting").innerHTML = new_html_str;

		replace_html = '<input type="text" name="wandog_fb_count" class="input_3_table" maxlength="2" value="<% nvram_get("wandog_fb_count"); %>" onKeyPress="return validator.isNumber(this, event);" placeholder="5" autocorrect="off" autocapitalize="off">';
		new_html_str = document.getElementById("wandog_fbcount_setting").innerHTML.replace("$WANDOG_FB_COUNT", replace_html);
		document.getElementById("wandog_fbcount_setting").innerHTML = new_html_str;

		if(gobi_support){
			if(document.form.wans_mode.value != "lb" && (document.form.wans_primary.value == "usb" || document.form.wans_second.value == "usb")){
				document.getElementById("wans_standby_tr").style.display = "";
				inputCtrl(document.form.wans_standby, 1);
			}
			else{
				document.getElementById("wans_standby_tr").style.display = "none";
				inputCtrl(document.form.wans_standby, 0);
			}	
		}

		appendModeOption(document.form.wans_mode_option.value);
		show_wans_rules();
		document.getElementById("wans_mode_tr").style.display = "";
		appendMonitorOption(document.form.dns_probe_chk);
		appendMonitorOption(document.form.wandog_enable_chk);
	}		
}

function applyRule(){
	//if primary wan is not lan and IPTV enabled, need block
	if(based_modelid == "RT-AC88Q" || based_modelid == "BRT-AC828" || based_modelid == "RT-AD7200") {
		if(!noiptv_support) {
			var original_switch_wantag = document.form.switch_wantag.value;
			if(document.form.wans_primary.value != "lan" && original_switch_wantag != "none") {
				var confirm_flag = confirm("If the primary WAN is not 'Ethernet LAN', IPTV function will be disable. Are you sure to process?");/*untranslated*/
				if(confirm_flag) {
					document.form.switch_wantag.disabled = false;
					document.form.switch_wantag.value = "none";
					document.form.switch_stb_x.disabled = false;
					document.form.switch_stb_x.value = "0";
				}
				else {
					return false;
				}
			}
		}
		var lan_trunk_type = '<% nvram_get("lan_trunk_type"); %>';
		var primary_wan_type = document.form.wans_primary.value;
		var secondary_wan_type = document.form.wans_second.value;
		var confirmAction = function() {
			return confirm("Enable the LAN as WAN setting will cause (LAN > Switch Control > bonding) feature will be disabled, Are you sure to continue?");/*untranslated*/
		};
		if(wans_flag == 1) {
			if( (primary_wan_type == "lan" || secondary_wan_type == "lan") && lan_trunk_type != "0" ) {
				if(!confirmAction())
					return false;
			}
		}
		else {
			if(primary_wan_type == "lan" && lan_trunk_type != "0") {
				if(!confirmAction())
					return false;
			}
		}
	}

	if(wans_flag == 1){
		document.form.wans_extwan.value = 0;
		if(document.form.wans_primary.value == "lan2"){
			if(document.form.wans_second.value == "wan"){
				document.form.wans_dualwan.value = "wan lan";
				document.form.wans_lanport.value = "5";
			}
			else{
				document.form.wans_dualwan.value = "wan " + document.form.wans_second.value;
			}
			
			document.form.wans_extwan.value = 1;
		}
		else if(document.form.wans_second.value == "lan2"){
			if(document.form.wans_primary.value == "wan"){
				document.form.wans_dualwan.value = "wan lan";
				document.form.wans_lanport.value = "5";
				document.form.wans_extwan.value = 0;
			}
			else{
				document.form.wans_dualwan.value = document.form.wans_primary.value +" wan";
				document.form.wans_extwan.value = 1;
			}			
		}
		else{
			document.form.wans_dualwan.value = document.form.wans_primary.value +" "+ document.form.wans_second.value;
		}

		if(!dsl_support && (document.form.wans_dualwan.value == "usb lan" || document.form.wans_dualwan.value == "lan usb") && based_modelid != "GT-AX11000" && based_modelid != "RT-AX86U" && based_modelid != "GT-AXE11000"){
			alert("WAN port should be selected in Dual WAN.");
			document.form.wans_primary.focus();
			return;
		}

		document.form.wan_unit.value = "<% nvram_get("wan_unit"); %>";
		if(document.form.wans_mode.value == "lb"){

			if(!validator.range(document.form.wans_lb_ratio_0, 1, 9))
					return false;
			if(!validator.range(document.form.wans_lb_ratio_1, 1, 9))
					return false;

			if(wans_mode_orig != "lb" && check_bwdpi_engine_status()) {
				var confirm_flag = confirm("<#dualwan_lb_dpi_conflict#>");
				if(confirm_flag) {
					document.form.action_script.value = "dpi_disable;reboot;";
				}
				else {
					return false;
				}
			}

			document.form.wans_lb_ratio.value = document.form.wans_lb_ratio_0.value + ":" + document.form.wans_lb_ratio_1.value;
			
			
			if(document.form.wan0_isp_country.options[0].selected == true){
					document.form.wan0_routing_isp.value = country[document.form.wan0_isp_country.value];
			}else{
					document.form.wan0_routing_isp.value = country[document.form.wan0_isp_country.value].toLowerCase()+"_"+country_n_isp[document.form.wan0_isp_country.value][document.form.wan0_isp_list.value];
			}
			
			if(document.form.wan1_isp_country.options[0].selected == true){
					document.form.wan1_routing_isp.value = country[document.form.wan1_isp_country.value];
			}else{
					document.form.wan1_routing_isp.value = country[document.form.wan1_isp_country.value].toLowerCase()+"_"+country_n_isp[document.form.wan1_isp_country.value][document.form.wan1_isp_list.value];
			}
			
			save_table();
		}
		else{ //fo or fb
			document.form.wans_lb_ratio.disabled = true;
			document.form.wan0_routing_isp_enable.disabled = true;
			document.form.wan0_routing_isp.disabled = true;
			document.form.wan1_routing_isp_enable.disabled = true;
			document.form.wan1_routing_isp.disabled = true;
			document.form.wans_routing_rulelist.disabled =true;

			if(document.form.wandog_enable_chk.checked)
				document.form.wandog_enable.value = "1";
			else
				document.form.wandog_enable.value = "0";

			if(document.form.dns_probe_chk.checked)
				document.form.dns_probe.value = "1";
			else
				document.form.dns_probe.value = "0";

			if(!validator.range(document.form.wandog_interval, dns_probe_timeout_threshold, 99))
				return false;
		}
	}
	else{
		document.form.wans_mode.value = "fo";
		appendModeOption("fo");
		document.form.wans_lb_ratio.disabled = true;
		document.form.wan0_routing_isp_enable.disabled = true;
		document.form.wan0_routing_isp.disabled = true;	
		document.form.wan1_routing_isp_enable.disabled = true;
		document.form.wan1_routing_isp.disabled = true;
		document.form.wans_routing_rulelist.disabled =true;
		if(document.form.wans_primary.value == "lan2"){
			document.form.wans_dualwan.value = "wan none";
			document.form.wans_extwan.value = "1";
		}
		else{
			document.form.wans_dualwan.value = document.form.wans_primary.value + " none";
			document.form.wans_extwan.value = "0";
		}
		document.form.wan_unit.value = 0;
		document.form.wandog_enable.value = "0";
	}	

	if(document.form.wans_primary.value == "lan")
		document.form.wans_lanport.value = document.form.wans_lanport1.value;
	else if(document.form.wans_second.value =="lan")
		document.form.wans_lanport.value = document.form.wans_lanport2.value;
	else{
		if(based_modelid != "GT-AX11000" && based_modelid != "RT-AX86U" && based_modelid != "GT-AXE11000"){
			document.form.wans_lanport.disabled = true;
		}
	}

	if (document.form.wans_dualwan.value.indexOf("lan") != -1) {
		var port_conflict = false;
		var lan_port_num = document.form.wans_lanport.value;
		
		if(switch_stb_x == lan_port_num)
			port_conflict = true;
		else if (switch_stb_x == '5' && (lan_port_num == '1' || lan_port_num == '2')) 
			port_conflict = true;
		else if ((switch_stb_x == '6' || switch_stb_x == '8') && (lan_port_num == '3' || lan_port_num == '4'))
			port_conflict = true;

		if (port_conflict) {
			alert("<#RouterConfig_IPTV_conflict#>");
			return;
		}

		//Check Bonding port conflict
		if(lacp_support && lacp_enabled == "1"){
			var conflict_lanport_text = "";
			if(based_modelid == "GT-AC5300")
				var bonding_port_settings = [{"val": "4", "text": "LAN5"}, {"val": "3", "text": "LAN6"}];
			else if(based_modelid == "RT-AC86U" || based_modelid == "GT-AC2900")
				var bonding_port_settings = [{"val": "4", "text": "LAN1"}, {"val": "3", "text": "LAN2"}];
			else
				var bonding_port_settings = [{"val": "1", "text": "LAN1"}, {"val": "2", "text": "LAN2"}];
			
			for(var i = 0; i < bonding_port_settings.length; i++){
				if(lan_port_num == bonding_port_settings[i].val){
					conflict_lanport_text = bonding_port_settings[i].text.toUpperCase();
				}
			}	
			if(conflict_lanport_text != ""){
				var confirm_str = "Configure "+ conflict_lanport_text +" as WAN will disable \""+"<#NAT_lacp#>"+"\" function, are you sure to do it?";//untranslated
				if(confirm(confirm_str)){
					document.form.lacp_enabled.disabled = false;
					document.form.lacp_enabled.value = "0";
				}
				else{
					if(document.form.wans_primary.value == "lan"){
						document.form.wans_lanport1.focus();
						document.form.wans_lanport1.value = wans_lanport_orig;
					}
					else if(document.form.wans_second.value =="lan"){
						document.form.wans_lanport2.focus();
						document.form.wans_lanport2.value = wans_lanport_orig;
					}
					return false;
				}
			}
		}
	} 

	wans_dualwan_array = document.form.wans_dualwan.value.split(" "); //update wans_dualwan_array
	if(wans_dualwan_array[1] == "none")
		document.form.wan_unit.value = 0;

	if(wans_dualwan_array.indexOf("usb") == 0 && document.form.wan0_enable.value == "0"){
		document.form.wan0_enable.value = "1";
	}

	if(wans_dualwan_array.indexOf("usb") == 1 && document.form.wan1_enable.value == "0"){
		document.form.wan1_enable.value = "1";
	}

	if(wan_bonding_support){
		if(based_modelid == "RT-AX89U" || based_modelid == "GT-AXY16000"){
			// If WAN is not used, disable WAN aggregation
			if(document.form.wans_dualwan.value.split(" ").indexOf("wan") == -1){
				document.form.bond_wan.disabled = false;
				document.form.bond_wan.value = "0";
			}
		}
	}

	var reboot_time	= eval("<% get_default_reboot_time(); %>");
	if(based_modelid =="DSL-AC68U")
		reboot_time += 70;
	document.form.action_wait.value = reboot_time;

	if(document.form.wans_standby.value == "1" && document.form.wans_standby.value != wans_standby_orig){
		document.getElementById("detect_time_confirm").style.display = "block";
		document.form.detect_interval.value = min_detect_interval;
		add_option_count(document.form.detect_interval, document.form.detect_count, min_fo_detect_count);
		update_str_time();
		return;
	}

	showLoading();
	document.form.submit();

}

function addWANOption(obj, wanscapItem){
	free_options(obj);

	if( wanscapItem.indexOf("wan") >= 0 && wanscapItem.indexOf("wan2") == -1 &&
		wans_dualwan_array[1] == "none" && obj.name == "wans_primary" && curState == "0" ){
		for(i=0; i<wanscapItem.length; i++){
			if(wanscapItem[i] == "lan"){
				wanscapItem.splice(i,1);
			}
		}
	}
	
	for(i=0; i<wanscapItem.length; i++){
		if(wanscapItem[i].length > 0){
			var wanscapName = wanscapItem[i].toUpperCase();
	               //MODELDEP: DSL-N55U, DSL-N55U-B, DSL-AC68U, DSL-AC68R
			if(wanscapName == "LAN" && 
            	(productid == "DSL-N55U" || productid == "DSL-N55U-B" || productid == "DSL-AC68U" || productid == "DSL-AC68R")) 
				wanscapName = "Ethernet WAN";
			else if(wanscapName == "LAN")
				wanscapName = "Ethernet LAN";
			else if(wanscapName == "USB" && (based_modelid == "4G-AC53U" || based_modelid == "4G-AC55U" || based_modelid == "4G-AC68U"))
				wanscapName = "<#Mobile_title#>";
			else if(wanscapName == "LAN2"){
				wanscapName = "2.5G WAN";
			}

			if(based_modelid == "GT-AXY16000" || based_modelid == "RT-AX89U"){
				if(wanscapName == "WAN2")
					wanscapName = "10G base-T";
				else if(wanscapName == "SFP+")
					wanscapName = "10G SFP+";
			}

			obj.options[i] = new Option(wanscapName, wanscapItem[i]);
		}
	}
}

function changeWANProto(obj){
	if(wans_flag == 1){	//dual WAN on
		if(obj.name == "wans_primary" && (based_modelid == "RT-AX86U" || based_modelid == "GT-AX11000" || based_modelid == "GT-AXE11000")){
			var wans_caps_tmp = wans_caps_secondary.split(" ");
			if(obj.value == "lan2"){
				wans_caps_tmp = wans_caps_tmp.slice(1);
			}
			addWANOption(document.form.wans_second, wans_caps_tmp);
		}

		if(document.form.wans_primary.value == document.form.wans_second.value){
			if(obj.name == "wans_primary"){
				if (obj.value == "dsl"){
					if(wans_caps.search("wan") >= 0)
						document.form.wans_second.value = "wan";
					else if(wans_caps.search("wan2") >= 0)
						document.form.wans_second.value = "wan2";
					else if(wans_caps.search("sfp+") >= 0)
						document.form.wans_second.value = "sfp+";
					else if(wans_caps.search("usb") >= 0)
						document.form.wans_second.value = "usb";
					else
						document.form.wans_second.value = "lan";
				}
				else if(obj.value == "wan"){
					if(wans_caps.search("dsl") >= 0)
						document.form.wans_second.value = "dsl";
					else if(wans_caps.search("wan2") >= 0)
						document.form.wans_second.value = "wan2";
					else if(wans_caps.search("sfp+") >= 0)
						document.form.wans_second.value = "sfp+";
					else if(wans_caps.search("usb") >= 0)
						document.form.wans_second.value = "usb";
					else
						document.form.wans_second.value = "lan";
				}
				else if(obj.value == "wan2"){
					if(wans_caps.search("dsl") >= 0)
						document.form.wans_second.value = "dsl";
					else if(wans_caps.search("wan") >= 0)
						document.form.wans_second.value = "wan";
					else if(wans_caps.search("sfp+") >= 0)
						document.form.wans_second.value = "sfp+";
					else if(wans_caps.search("usb") >= 0)
						document.form.wans_second.value = "usb";
					else
						document.form.wans_second.value = "lan";
				}
				else if(obj.value == "sfp+"){
					if(wans_caps.search("dsl") >= 0)
						document.form.wans_second.value = "dsl";
					else if(wans_caps.search("wan") >= 0)
						document.form.wans_second.value = "wan";
					else if(wans_caps.search("wan2") >= 0)
						document.form.wans_second.value = "wan2";
					else if(wans_caps.search("usb") >= 0)
						document.form.wans_second.value = "usb";
					else
						document.form.wans_second.value = "lan";
				}
				else if(obj.value == "usb"){
					if(wans_caps.search("dsl") >= 0)
						document.form.wans_second.value = "dsl";
					else if(wans_caps.search("wan") >= 0)
						document.form.wans_second.value = "wan";
					else if(wans_caps.search("wan2") >= 0)
						document.form.wans_second.value = "wan2";
					else if(wans_caps.search("sfp+") >= 0)
						document.form.wans_second.value = "sfp+";
					else if(wans_caps.search("lan") >= 0)
						document.form.wans_second.value = "lan";
				}
				else if(obj.value == "lan2"){
					if(wans_caps.search("usb") >= 0)
						document.form.wans_second.value = "usb";
				}
				else{
					if(wans_caps.search("dsl") >= 0)
						document.form.wans_second.value = "dsl";
					else if(wans_caps.search("wan") >= 0)
						document.form.wans_second.value = "wan";
					else if(wans_caps.search("wan2") >= 0)
						document.form.wans_second.value = "wan2";
					else if(wans_caps.search("sfp+") >= 0)
						document.form.wans_second.value = "sfp+";
					else if(wans_caps.search("usb") >= 0)
						document.form.wans_second.value = "usb";
				}
			}
			else if(obj.name == "wans_second"){
				if (obj.value == "dsl"){
					if(wans_caps.search("wan") >= 0)
						document.form.wans_primary.value = "wan";
					else if(wans_caps.search("wan2") >= 0)
						document.form.wans_primary.value = "wan2";
					else if(wans_caps.search("sfp+") >= 0)
						document.form.wans_primary.value = "sfp+";
					else if(wans_caps.search("usb") >= 0)
						document.form.wans_primary.value = "usb";
					else
						document.form.wans_primary.value = "lan";
                                }
				else if(obj.value == "wan"){
					if(wans_caps.search("dsl") >= 0)
						document.form.wans_primary.value = "dsl";
					else if(wans_caps.search("wan2") >= 0)
						document.form.wans_primary.value = "wan2";
					else if(wans_caps.search("sfp+") >= 0)
						document.form.wans_primary.value = "sfp+";
					else if(wans_caps.search("usb") >= 0)
						document.form.wans_primary.value = "usb";
					else
						document.form.wans_primary.value = "lan";
				}
				else if(obj.value == "wan2"){
					if(wans_caps.search("dsl") >= 0)
						document.form.wans_primary.value = "dsl";
					else if(wans_caps.search("wan") >= 0)
						document.form.wans_primary.value = "wan";
					else if(wans_caps.search("sfp+") >= 0)
						document.form.wans_primary.value = "sfp+";
					else if(wans_caps.search("usb") >= 0)
						document.form.wans_primary.value = "usb";
					else
						document.form.wans_primary.value = "lan";
				}
				else if(obj.value == "sfp+"){
					if(wans_caps.search("dsl") >= 0)
						document.form.wans_primary.value = "dsl";
					else if(wans_caps.search("wan") >= 0)
						document.form.wans_primary.value = "wan";
					else if(wans_caps.search("wan2") >= 0)
						document.form.wans_primary.value = "wan2";
					else if(wans_caps.search("usb") >= 0)
						document.form.wans_primary.value = "usb";
					else
						document.form.wans_primary.value = "lan";
				}
				else if(obj.value == "usb"){
					if(wans_caps.search("dsl") >= 0)
						document.form.wans_primary.value = "dsl";
					else if(wans_caps.search("wan") >= 0)
						document.form.wans_primary.value = "wan";
					else if(wans_caps.search("wan2") >= 0)
						document.form.wans_primary.value = "wan2";
					else if(wans_caps.search("sfp+") >= 0)
						document.form.wans_primary.value = "sfp+";
					else
						document.form.wans_primary.value = "lan";
				}
				else if(obj.value == "lan2"){
					var wans_caps_tmp = wans_caps_secondary.split(" ");
					addWANOption(document.form.wans_second, wans_caps_tmp);
					document.form.wans_second.value = "lan2";
					if(wans_caps.search("wan") >= 0)
						document.form.wans_primary.value = "wan";
					else if(wans_caps.search("usb") >= 0)
						document.form.wans_primary.value = "usb";
				}
				else{
					if(wans_caps.search("dsl") >= 0)
						document.form.wans_primary.value = "dsl";
					else if(wans_caps.search("wan") >= 0)
						document.form.wans_primary.value = "wan";
					else if(wans_caps.search("wan2") >= 0)
						document.form.wans_primary.value = "wan2";
					else if(wans_caps.search("sfp+") >= 0)
						document.form.wans_primary.value = "sfp+";
				}
			}
		}

		if(gobi_support){
			if(document.form.wans_primary.value == "usb" || document.form.wans_second.value == "usb"){
				document.getElementById("wans_standby_tr").style.display = "";
				inputCtrl(document.form.wans_standby, 1);
			}
			else{
				document.getElementById("wans_standby_tr").style.display = "none";
				inputCtrl(document.form.wans_standby, 0);		
			}
		}		

		appendLANoption1(document.form.wans_primary);
		appendLANoption2(document.form.wans_second);
	}else
		appendLANoption1(document.form.wans_primary);
}

function appendLANoption1(obj){
	if(obj.value == "lan"){
		if(document.form.wans_lanport1){
			document.form.wans_lanport1.style.display = "";	
		}		
	}
	else if(document.form.wans_lanport1){
		document.form.wans_lanport1.style.display = "none";
	}
}

function appendLANoption2(obj){
	if(obj.value == "lan"){
		if(document.form.wans_lanport2){
			document.form.wans_lanport2.style.display = "";
		}	
	}
	else if(document.form.wans_lanport2){
		document.form.wans_lanport2.style.display = "none";
	}
}

function appendModeOption(v){
		var wandog_enable_orig = '<% nvram_get("wandog_enable"); %>';
		if(v == "lb"){
			document.getElementById("lb_note").style.display = "";
			//document.getElementById("lb_note2").style.display = "";
			inputCtrl(document.form.wans_lb_ratio_0, 1);
			inputCtrl(document.form.wans_lb_ratio_1, 1);
			document.form.wans_lb_ratio_0.value = '<% nvram_get("wans_lb_ratio"); %>'.split(':')[0];
			document.form.wans_lb_ratio_1.value = '<% nvram_get("wans_lb_ratio"); %>'.split(':')[1];
			inputCtrl(document.form.wans_isp_unit[0], 1);
			inputCtrl(document.form.wans_isp_unit[1], 1);
			inputCtrl(document.form.wans_isp_unit[2], 1);
			if('<% nvram_get("wan0_routing_isp_enable"); %>' == 1 && '<% nvram_get("wan1_routing_isp_enable"); %>' == 0){
				document.form.wans_isp_unit[1].checked =true;
				change_isp_unit(1);
			}else if('<% nvram_get("wan0_routing_isp_enable"); %>' == 0 && '<% nvram_get("wan1_routing_isp_enable"); %>' == 1){
				document.form.wans_isp_unit[2].checked =true;
				change_isp_unit(2);
			}else{
				document.form.wans_isp_unit[0].checked =true;
				change_isp_unit(0);
			}				
				
			Load_ISP_country();
			appendcountry(document.form.wan0_isp_country);
			appendcountry(document.form.wan1_isp_country);				
			inputCtrl(document.form.wans_routing_enable[0], 1);
			inputCtrl(document.form.wans_routing_enable[1], 1);
			
			document.getElementById('watchdog_table').style.display = "none";
			if('<% nvram_get("wans_routing_enable"); %>' == 1){
				document.form.wans_routing_enable[0].checked = true;				
				document.getElementById('Routing_rules_table').style.display = "";
			}
			else{
				document.form.wans_routing_enable[1].checked = true;
				document.getElementById('Routing_rules_table').style.display = "none";
			}

			document.getElementById("wandog_fb_count_tr").style.display = "none";			
			document.getElementById("routing_table").style.display = "";
			document.getElementById("fb_span").style.display = "none";
			document.form.wans_mode.value = "lb";
		}
		else{ //Failover / Failback
			document.getElementById('lb_note').style.display = "none";
			//document.getElementById("lb_note2").style.display = "none";
			inputCtrl(document.form.wans_lb_ratio_0, 0);
			inputCtrl(document.form.wans_lb_ratio_1, 0);
			inputCtrl(document.form.wans_isp_unit[0], 0);
			inputCtrl(document.form.wans_isp_unit[1], 0);
			inputCtrl(document.form.wans_isp_unit[2], 0);
			inputCtrl(document.form.wan0_isp_country, 0);
			inputCtrl(document.form.wan0_isp_list, 0);				
			inputCtrl(document.form.wan1_isp_country, 0);
			inputCtrl(document.form.wan1_isp_list, 0);				
			document.form.wans_routing_enable[1].checked = true;				
			document.form.wans_routing_enable[0].disabled = true;
			document.form.wans_routing_enable[1].disabled = true;
			document.getElementById('watchdog_table').style.display = "";
			document.getElementById('Routing_rules_table').style.display = "none";
			document.getElementById("wandog_fb_count_tr").style.display = "none";
			document.getElementById("routing_table").style.display = "none";
			document.getElementById("fb_span").style.display = "";

			if(document.form.wans_mode.value == "fb" ? true : false)
			{
				document.getElementById("fb_checkbox").checked = true;
				document.getElementById("wandog_fb_count_tr").style.display = "";
				document.form.wans_mode.value = "fb";
			}
			else
			{
				document.getElementById("fb_checkbox").checked = false;
				document.getElementById("wandog_fb_count_tr").style.display = "none";
				document.form.wans_mode.value = "fo";
			}
		}
}

function appendMonitorOption(obj){
	if(obj.name == "wandog_enable_chk"){
		if(obj.checked){
			inputCtrl(document.form.wandog_target, 1);
		}
		else{
			inputCtrl(document.form.wandog_target, 0);
		}
	}
	else if(obj.name == "dns_probe_chk"){
		if(obj.checked){
			inputCtrl(document.form.dns_probe_host, 1);
			inputCtrl(document.form.dns_probe_content, 1);
		}
		else{
			inputCtrl(document.form.dns_probe_host, 0);
			inputCtrl(document.form.dns_probe_content, 0);
		}
	}
}

function show_wans_rules(){

	var tableStruct = {
		data: wans_routing_rulelist_array,
		container: "Routing_rules_table",
		title: "<#dualwan_routing_rule_list#>",
		capability: {
			add: true,
			del: true,
			clickEdit: true,
		},
		header: [ 
			{
				"title" : "<#FirewallConfig_LanWanSrcIP_itemname#>",
				"width" : "30%"
			},
			{
				"title" : "<#FirewallConfig_LanWanDstIP_itemname#>",
				"width" : "30%"
			},
			{
				"title" : "<#dualwan_unit#>",
				"width" : "30%"
			}
		],
		createPanel: {
			inputs : [
				{
					"editMode" : "text",
					"title" : "<#FirewallConfig_LanWanSrcIP_itemname#>",
					"maxlength" : "18",
					"validator" : "dualWanRoutingRules",
					"placeholder" : "Please input IP Address or IP Address/Netmask or all."
				},
				{
					"editMode" : "text",
					"title" : "<#FirewallConfig_LanWanDstIP_itemname#>",
					"maxlength" : "18",
					"validator" : "dualWanRoutingRules",
					"placeholder" : "Please input IP Address or IP Address/Netmask or all."
				},
				{
					"editMode" : "select",
					"title" : "<#dualwan_unit#>",
					"option" : {"<#dualwan_primary#>" : "0", "<#dualwan_secondary#>" : "1"}
				},
			],
			maximum: 64
		},

		clickRawEditPanel: {
			inputs : [
				{
					"editMode" : "text",
					"maxlength" : "18",
					"validator" : "dualWanRoutingRules"
				},
				{
					"editMode" : "text",
					"maxlength" : "18",
					"validator" : "dualWanRoutingRules"
				},
				{
					"editMode" : "select",
					"option" : {"<#dualwan_primary#>" : "0", "<#dualwan_secondary#>" : "1"}
				}
			]
		},

		ruleValidation : "dualWANRoutingRules",
		ruleDuplicateValidation : "dualWANRoutingRules"
	}

	tableApi.genTableAPI(tableStruct);
}

function save_table(){
	//parse array to nvram
	var tmp_value = "";
	for(var i = 0; i < wans_routing_rulelist_array.length; i += 1) {
		if(wans_routing_rulelist_array[i].length != 0) {
			tmp_value += "<";
			for(var j = 0; j < wans_routing_rulelist_array[i].length; j += 1) {
				tmp_value += wans_routing_rulelist_array[i][j];
				if( (j + 1) != wans_routing_rulelist_array[i].length)
					tmp_value += ">";
			}
		}
	}	
	document.form.wans_routing_rulelist.value = tmp_value;
}

function change_isp_unit(v){
	inputCtrl(document.form.wan0_isp_country, 1);
	inputCtrl(document.form.wan1_isp_country, 1);
	
	if(v == 1){
			document.form.wan0_routing_isp_enable.value = 1;
			document.form.wan1_routing_isp_enable.value = 0;
			document.form.wan0_isp_country.disabled = false;
			document.form.wan0_isp_list.disabled = false;			
			document.form.wan1_isp_country.disabled = true;
			document.form.wan1_isp_list.disabled = true;
	}else if(v == 2){
			document.form.wan0_routing_isp_enable.value = 0;
			document.form.wan1_routing_isp_enable.value = 1;
			document.form.wan0_isp_country.disabled = true;
			document.form.wan0_isp_list.disabled = true;
			document.form.wan1_isp_country.disabled = false;
			document.form.wan1_isp_list.disabled = false;
	}else{
			document.form.wan0_routing_isp_enable.value = 0;
			document.form.wan1_routing_isp_enable.value = 0;
			document.form.wan0_isp_country.disabled = true;
			document.form.wan0_isp_list.disabled = true;
			document.form.wan1_isp_country.disabled = true;
			document.form.wan1_isp_list.disabled = true;			
	}	
}

function Load_ISP_country(){
	var country_num = country.length;
	for(c = 0; c < country_num; c++){
		document.form.wan0_isp_country.options[c] = new Option(country[c], c);
		if(document.form.wan0_isp_country.options[c].text.toLowerCase() == '<% nvram_get("wan0_routing_isp"); %>'.split("_")[0])
			document.form.wan0_isp_country.options[c].selected =true;
												
		document.form.wan1_isp_country.options[c] = new Option(country[c], c);		
		if(document.form.wan1_isp_country.options[c].text.toLowerCase() == '<% nvram_get("wan1_routing_isp"); %>'.split("_")[0])
			document.form.wan1_isp_country.options[c].selected =true;						
	}
	if('<% nvram_get("wan0_routing_isp"); %>' == "")
		document.form.wan0_isp_country.options[0].selected =true;
	if('<% nvram_get("wan1_routing_isp"); %>' == "")
		document.form.wan1_isp_country.options[0].selected =true;				
}

function appendcountry(obj){
	if(obj.name == "wan0_isp_country"){
		if(obj.value == 0){	//none
			document.form.wan0_isp_list.style.display = "none";
		}
		else{	//other country
			var wan0_country_isp_num = country_n_isp[obj.value].length;
			document.form.wan0_isp_list.style.display = "";
			for(j = 0; j < wan0_country_isp_num; j++){
				document.form.wan0_isp_list.options[j] = new Option(country_n_isp[obj.value][j], j);
				if(document.form.wan0_isp_list.options[j].text == '<% nvram_get("wan0_routing_isp"); %>'.split("_")[1])
					document.form.wan0_isp_list.options[j].selected =true;
			}							
		}				
	}else if(obj.name == "wan1_isp_country"){
		if(obj.value == 0){	//none
			document.form.wan1_isp_list.style.display = "none";					
		}else{	//other country
			var wan1_country_isp_num = country_n_isp[obj.value].length;
			document.form.wan1_isp_list.style.display = "";
			for(j = 0; j < wan1_country_isp_num; j++){
				document.form.wan1_isp_list.options[j] = new Option(country_n_isp[obj.value][j], j);
				if(document.form.wan1_isp_list.options[j].text == '<% nvram_get("wan1_routing_isp"); %>'.split("_")[1])
					document.form.wan1_isp_list.options[j].selected =true;
			}
					
		}		
	}
}


function showLANIPList(){
	//copy array from Main_Analysis page
	var APPListArray = [
		["Google ", "www.google.com"], ["Facebook", "www.facebook.com"], ["Youtube", "www.youtube.com"], ["Yahoo", "www.yahoo.com"],
		["Baidu", "www.baidu.com"], ["Wikipedia", "www.wikipedia.org"], ["Windows Live", "www.live.com"], ["QQ", "www.qq.com"],
		["Amazon", "www.amazon.com"], ["Twitter", "www.twitter.com"], ["Taobao", "www.taobao.com"], ["Blogspot", "www.blogspot.com"], 
		["Linkedin", "www.linkedin.com"], ["Sina", "www.sina.com"], ["eBay", "www.ebay.com"], ["MSN", "msn.com"], ["Bing", "www.bing.com"], 
		["Яндекс", "www.yandex.ru"], ["WordPress", "www.wordpress.com"], ["ВКонтакте", "www.vk.com"]
	];

	var code = "";
	for(var i = 0; i < APPListArray.length; i++){
		code += '<a><div onmouseover="over_var=1;" onmouseout="over_var=0;" onclick="setClientIP(\''+APPListArray[i][1]+'\');"><strong>'+APPListArray[i][0]+'</strong></div></a>';
	}
	code +='<!--[if lte IE 6.5]><iframe class="hackiframe2"></iframe><![endif]-->';	
	document.getElementById("ClientList_Block_PC").innerHTML = code;
}

function setClientIP(ipaddr){
	document.form.wandog_target.value = ipaddr;
	hideClients_Block();
	over_var = 0;
}

var over_var = 0;
var isMenuopen = 0;
function hideClients_Block(){
	document.getElementById("pull_arrow").src = "/images/arrow-down.gif";
	document.getElementById('ClientList_Block_PC').style.display='none';
	isMenuopen = 0;
}

function pullLANIPList(obj){
	if(isMenuopen == 0){
		obj.src = "/images/arrow-top.gif"
		document.getElementById("ClientList_Block_PC").style.display = 'block';
		document.form.wandog_target.focus();
		isMenuopen = 1;
	}
	else
		hideClients_Block();
}

function enable_lb_rules(flag){
	if(flag == "1"){
			document.getElementById('Routing_rules_table').style.display = "";
	}
	else{
			document.getElementById('Routing_rules_table').style.display = "none";
	}
}

var str0 = "";
function add_option_count(obj, obj_t, selected_flag){
	var start = 1;
	var end = 99;

	if(obj_t.name == "wandog_maxfail" || (obj_t.name == "wandog_fb_count" && document.getElementById("wandog_fb_count_tr").style.display == "") || obj_t.name == "detect_count"){
		if(obj_t.name == "wandog_fb_count" || obj_t.name == "wandog_maxfail"){
			start = 3;
			if(obj_t.name == "wandog_fb_count")
				end = parseInt(document.form.wandog_maxfail.value);
		}

		free_options(obj_t);
		for(var i = start; i <= end; i++){
			if((based_modelid == "4G-AC53U" || based_modelid == "4G-AC55U" || based_modelid == "4G-AC68U") && obj_t.name != "wandog_fb_count")
				str0= i;
			else
				str0 = i*parseInt(obj.value);

			if(selected_flag == i)
					add_option(obj_t, str0, i, 1);
			else
					add_option(obj_t, str0, i, 0);
		}
	}
	else{
		return;
	}
}

function hotstandby_act(enable){
	var confirm_str_on = "<#Wans_standby_hint#>";
	if(enable){
		if(mobile_enable_orig == "0"){
			if(confirm(confirm_str_on)){
				if(usb_index == 0)
					document.form.wan0_enable.value = "1";
				else if(usb_index == 1)					
					document.form.wan1_enable.value = "1"
			}
		}
	}
}

function update_consume_bytes(){
    var consume_warning_str;
    var interval_value = parseInt(document.form.wandog_interval.value);
    var consume_bytes;
    var MBytes = 1024*1024;

    if(based_modelid == "4G-AC53U" || based_modelid == "4G-AC55U" || based_modelid == "4G-AC68U"){
    consume_bytes = 86400/interval_value*128*30;
	consume_bytes = Math.ceil(consume_bytes/MBytes);
    consume_warning_str = "<#Detect_consume_warning1#> "+consume_bytes+" <#Detect_consume_warning2#>";
    document.getElementById("consume_bytes_warning").style.display= "";
    document.getElementById("consume_bytes_warning").innerHTML = consume_warning_str;
	}
}

function validate_interval_value(){
	if(!validator.numberRange(document.form.detect_interval, 1, 9)){
		document.form.detect_interval.focus();
	}
}

function update_str_time(){
	if(!validator.numberRange(document.form.detect_interval, 1, 9)){
		document.form.detect_interval.focus();
		return;
	}
	var detection_time = parseInt(document.form.detect_interval.value)*parseInt(document.form.detect_count.value);
	document.getElementById("str_detect_time").innerHTML = detection_time;
	document.getElementById("detection_time_value").innerHTML = detection_time;
}

function change_detect_settings(){
	document.getElementById("detect_time_confirm").style.display = "none";
	document.form.wandog_interval.value = document.form.detect_interval.value;
	document.form.wandog_maxfail.value = document.form.detect_count.value;
	showLoading();
	document.form.submit();
}

function remain_origins(){
	document.getElementById("detect_time_confirm").style.display = "none";
	showLoading();
	document.form.submit();
}

</script>
</head>
<body onload="initial();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_WANPort_Content.asp">
<input type="hidden" name="next_page" value="Advanced_WANPort_Content.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply_new">
<input type="hidden" name="action_wait" value="<% get_default_reboot_time(); %>">
<input type="hidden" name="action_script" value="reboot">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="wl_ssid" value="<% nvram_get("wl_ssid"); %>">
<input type="hidden" name="wan_unit" value="<% nvram_get("wan_unit"); %>">
<input type="hidden" name="wans_extwan" value="<% nvram_get("wans_extwan"); %>">
<input type="hidden" name="wans_dualwan" value="<% nvram_get("wans_dualwan"); %>">
<input type="hidden" name="wans_lanport" value="<% nvram_get("wans_lanport"); %>">
<input type="hidden" name="wans_lb_ratio" value="<% nvram_get("wans_lb_ratio"); %>">
<input type="hidden" name="dns_probe" value="<% nvram_get("dns_probe"); %>">
<input type="hidden" name="wandog_enable" value="<% nvram_get("wandog_enable"); %>">
<input type="hidden" name="wan0_routing_isp_enable" value="<% nvram_get("wan0_routing_isp_enable"); %>">
<input type="hidden" name="wan0_routing_isp" value="<% nvram_get("wan0_routing_isp"); %>">
<input type="hidden" name="wan1_routing_isp_enable" value="<% nvram_get("wan0_routing_isp_enable"); %>">
<input type="hidden" name="wan1_routing_isp" value="<% nvram_get("wan1_routing_isp"); %>">
<input type="hidden" name="wans_routing_rulelist" value=''>
<input type="hidden" name="wan0_enable" value="<% nvram_get("wan0_enable"); %>">
<input type="hidden" name="wan1_enable" value="<% nvram_get("wan1_enable"); %>">
<input type="hidden" name="switch_wantag" value="<% nvram_get("switch_wantag"); %>" disabled>
<input type="hidden" name="switch_stb_x" value="<% nvram_get("switch_stb_x"); %>" disabled>
<input type="hidden" name="lacp_enabled" value="<% nvram_get("lacp_enabled"); %>" disabled>
<input type="hidden" name="bond_wan" value="<% nvram_get("bond_wan"); %>" disabled>
<!--===================================Beginning of Detection Time Confirm===========================================-->
<div id="detect_time_confirm" style="display:none;">
		<!--div style="margin:20px 30px 20px;"-->
		<table width="90%" border="0" align="left" cellpadding="4" cellspacing="0" style="margin:15px 20px 15px; text-align:left;">
			<tr><td colspan="2"><#Standby_hint1#></td></tr><tr><td colspan="2"><#Standby_hint2#>&nbsp;:&nbsp;<span id="str_detect_time"></span>&nbsp;<#Second#>.</td></tr>
			<tr>
				<th style="width:30%;"><#Retry_interval#></th>
				<td>
					<input type="text" name="detect_interval" class="input_3_table" maxlength="1" value=""; placeholder="5" autocorrect="off" autocapitalize="off" onKeyPress="return validator.isNumber(this, event);" onblur="update_str_time();" style="width: 38px; margin: 0px;">&nbsp;&nbsp;<#Second#>
				</td>
			</tr>
			<tr>
				<th><#Retry_count#></th>
				<td>
					<select name="detect_count" class="input_option" onchange="update_str_time();" style="margin: 0px 0px;"></select>
					<span id="detect_tail_msg">&nbsp;( Detection Time: <span id="detection_time_value"></span>&nbsp;&nbsp;<#Second#>)</span>
				</td>
			</tr>
			</table>
		<!--/div-->
		<div style="padding-bottom:10px;width:100%;text-align:center;">
		<input id="yesButton" class="button_gen" type="button" value="<#checkbox_Yes#>" onclick="change_detect_settings();">
		<input id="noButton" class="button_gen" type="button" value="<#checkbox_No#>" onclick="remain_origins();">
		</div>	
</div>
<!--===================================End of Detection Time Confirm===========================================-->
<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="17">&nbsp;</td>		
		<td valign="top" width="202">				
			<div id="mainMenu"></div>	
			<div id="subMenu"></div>		
		</td>						
    <td valign="top">
			<div id="tabMenu" class="submenuBlock"></div>
			
			<!--===================================Beginning of Main Content===========================================-->

			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
				<tr>
					<td valign="top">
						<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
							<tbody>
								<tr>
								  <td bgcolor="#4D595D" valign="top">
									<div>&nbsp;</div>
									<div class="formfonttitle"><#menu5_3#> - <#dualwan#></div>
									<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
									<div class="formfontdesc"><#dualwan_desc#><a id="dualwan_faq" href="" target="_blank" style="margin-left:5px; text-decoration: underline;"><#dualwan#> FAQ</a></div>
									<div id="wan_aggre_desc" class="formfontdesc" style="color:#FFCC00; display:none;"><#WANAggregation_goto_WAN#></div>

									<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
										
			  						<thead>
			  						<tr>
										<td colspan="2"><#t2BC#></td>
			  						</tr>
			  						</thead>
			  						
										<tr id="wans_mode_enable_tr">
										<th><#dualwan_enable#></th>
											<td>
												<div class="left" style="width:94px; float:left; cursor:pointer;" id="ad_radio_dualwan_enable"></div>
												<div class="iphone_switch_container" style="height:32px; width:74px; position: relative; overflow: hidden">
												<script type="text/javascript">
													$('#ad_radio_dualwan_enable').iphoneSwitch(wans_dualwan_array[1] != 'none',
														function() {
															if(wan_bonding_support && orig_bond_wan == "1"){
																var msg = "<#WANAggregation_disable_WANAggregation#>";
																if(confirm(msg)){
																	document.form.bond_wan.disabled = false;
																	document.form.bond_wan.value = "0";
																}
																else{
																	$('#ad_radio_dualwan_enable').find('.iphone_switch').animate({backgroundPosition: "-38px"}, "slow");
																	return false;
																}
															}

															curState = "1";
															wans_flag = 1;
															inputCtrl(document.form.wans_second, 1);
															if(wans_caps.search("wan2") >= 0 && wans_caps.search("sfp+") == -1)
																document.form.wans_mode.value = "lb";
															else
																document.form.wans_mode.value = "fo";

															addWANOption(document.form.wans_primary, wans_caps_primary.split(" "));
															form_show(wans_flag);
														 },
														 function() {
															if(wans_caps_primary.indexOf("wan") >= 0 && wans_dualwan_array[0] == "lan"){
																var cur_parimary_wan = wans_dualwan_array[0].toUpperCase() + " Port " + wans_lanport_orig;
																var confirm_str = "The current primary wan is \"" + cur_parimary_wan + "\". Disable dual wan will change primary wan to \"Ethernet WAN\", are you sure to do it?"; //untranslated
																if(!confirm(confirm_str)){
																	curState = "1";
																	$('#ad_radio_dualwan_enable').find('.iphone_switch').animate({backgroundPosition: 0}, "slow");
																	return false;
																}
																else{
																	wans_dualwan_array[0] = "wan";
																}
															}
															curState = "0";
															wans_flag = 0;
															wans_dualwan_array[1] = "none"
															document.form.wans_dualwan.value = wans_dualwan_array.join(" ");
															document.form.wans_mode.value = "fo";
															addWANOption(document.form.wans_primary, wans_caps_primary.split(" "));
															form_show(wans_flag);
															if(wan_bonding_support){
																document.form.bond_wan.disabled = false;
																document.form.bond_wan.value = orig_bond_wan;
															}
														}
													);
												</script>
												</div>
											</td>
										</tr>

										<tr>
											<th><#dualwan_primary#></th>
											<td>
												<select name="wans_primary" class="input_option" onchange="changeWANProto(this);"></select>
												<select id="wans_lanport1" name="wans_lanport1" class="input_option" style="margin-left:7px;">
													<option value="1" <% nvram_match("wans_lanport", "1", "selected"); %>>LAN Port 1</option>
													<option value="2" <% nvram_match("wans_lanport", "2", "selected"); %>>LAN Port 2</option>
													<option value="3" <% nvram_match("wans_lanport", "3", "selected"); %>>LAN Port 3</option>
													<option value="4" <% nvram_match("wans_lanport", "4", "selected"); %>>LAN Port 4</option>
												</select>
											</td>
									  	</tr>
										<tr>
											<th><#dualwan_secondary#></th>
											<td>
												<select name="wans_second" class="input_option" onchange="changeWANProto(this);"></select>
												<select id="wans_lanport2" name="wans_lanport2" class="input_option" style="margin-left:7px;">
													<option value="1" <% nvram_match("wans_lanport", "1", "selected"); %>>LAN Port 1</option>
													<option value="2" <% nvram_match("wans_lanport", "2", "selected"); %>>LAN Port 2</option>
													<option value="3" <% nvram_match("wans_lanport", "3", "selected"); %>>LAN Port 3</option>
													<option value="4" <% nvram_match("wans_lanport", "4", "selected"); %>>LAN Port 4</option>												
												</select>											
											</td>
									  	</tr>

										<tr id="wans_mode_tr">
											<th><#dualwan_mode#></th>
											<td>
												<input type="hidden" name="wans_mode" value=''>
												<select id="wans_mode_option" class="input_option" onchange="appendModeOption(this.value);">
													<option value="fo"><#dualwan_mode_fo#></option>
													<option value="lb" <% nvram_match("wans_mode", "lb", "selected"); %>><#dualwan_mode_lb#></option>
												</select>
												<span id="wans_mode_fo" style="margin-left:5px; color:#FFF; display:none;"><#dualwan_mode_fo#></span>
												<span id="fb_span" style="display:none"><input type="checkbox" id="fb_checkbox"><#dualwan_failback_allow#></span>
										  		<script>
										  			document.getElementById("fb_checkbox").onclick = function(){
										  				document.form.wans_mode.value = (this.checked == true ? "fb" : "fo");
										  				document.getElementById("wandog_fb_count_tr").style.display = (this.checked == true ? "" : "none");
										  			}
										  		</script>
												<div id="lb_note" style="color:#FFCC00; display:none;"><#dualwan_lb_note#></div>
												<div id="lb_note2" style="color:#FFCC00; display:none;"><#dualwan_lb_note2#></div>
											</td>
									  	</tr>

							<tr id="wans_standby_tr" style="display:none;">
								<th><#Standby_str#></th>
						        <td>
									<select name="wans_standby" id="wans_standby" class="input_option" onchange="hotstandby_act(this.value);">
										<option value="1" <% nvram_match("wans_standby", "1", "selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
										<option value="0" <% nvram_match("wans_standby", "0", "selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
									</select>        
								</td>
							</tr>	

			          		<tr>
			            		<th><#dualwan_mode_lb_setting#></th>
			            		<td>
									<input type="text" maxlength="1" class="input_3_table" name="wans_lb_ratio_0" value="" onkeypress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off"/>
									&nbsp; : &nbsp;
									<input type="text" maxlength="1" class="input_3_table" name="wans_lb_ratio_1" value="" onkeypress="return validator.isNumber(this,event);" autocorrect="off" autocapitalize="off"/>												
								</td>
			          		</tr>

			          		<tr class="ISPProfile">
			          			<th><#dualwan_isp_rules#></th>
			          			<td>
			          				<input type="radio" value="0" name="wans_isp_unit" class="content_input_fd" onClick="change_isp_unit(this.value);">None
									<input type="radio" value="1" name="wans_isp_unit" class="content_input_fd" onClick="change_isp_unit(this.value);"><#dualwan_primary#>
									<input type="radio" value="2" name="wans_isp_unit" class="content_input_fd" onClick="change_isp_unit(this.value);"><#dualwan_secondary#>
			          			</td>	
			          		</tr>	
			          		
			          		<tr class="ISPProfile">
			          			<th><#dualwan_isp_primary#></th>
			          			<td>
			          				<select name="wan0_isp_country" class="input_option" onchange="appendcountry(this);" value=""></select>
									<select name="wan0_isp_list" class="input_option" style="display:none;"value=""></select>
			          			</td>	
			          		</tr>
			          		
			          		<tr class="ISPProfile">
			          			<th><#dualwan_isp_secondary#></th>
			          			<td>
			          				<select name="wan1_isp_country" class="input_option" onchange="appendcountry(this);" value=""></select>
									<select name="wan1_isp_list" class="input_option" style="display:none;"value=""></select>
			          			</td>	
			          		</tr>			          		
			          		
									</table>
									
	<!-- -----------Enable Ping time watch dog start----------------------- -->			
				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable" style="margin-top:8px;" id="watchdog_table">
					<thead>
					<tr>
						<td colspan="2"><#dualwan_pingtime_wd2#><div style="font-weight: normal; font-style: italic; margin-top: 5px;"><#FAQ_Desc#></div></td>
					</tr>
					</thead>

					<tr>
						<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(26,3);"><#Retry_interval2#></a></th>
						<td id="retry_intervale_setting"><#Every_N_Seconds#>
							<div><span id="consume_bytes_warning" style=""></span></div>
						</td>
					</tr>

					<tr>
						<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(26,5);"><div id="fo_detection_count_hd"><#dualwan_failover_trigger#></div></a></th>
						<td id="wandog_maxfail_setting"><#dualwan_failover_desc#></td>
					</tr>

					<tr id="wandog_fb_count_tr">
						<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(26,6);"><div id="fb_detection_count_hd"><#dualwan_failback_trigger#></div></a></th>
						<td id="wandog_fbcount_setting"><#dualwan_failback_desc2#></td>
					</tr>

					<tr>
						<th><#Network_Monitoring#></th>
						<td>
							<input type="checkbox" name="dns_probe_chk" value="" <% nvram_match("dns_probe", "1", "checked"); %> onClick="appendMonitorOption(this);"><div style="display: inline-block; vertical-align: middle; margin-bottom: 2px;" ><#DNS_Query#></div>
							<input type="checkbox" name="wandog_enable_chk" value="" <% nvram_match("wandog_enable", "1", "checked"); %>  onClick="appendMonitorOption(this);"><div style="display: inline-block; vertical-align: middle; margin-bottom: 2px;"><#Ping#></div>
						</td>
					</tr>

					<tr>
						<th><#Resolved_Target#></th>
				        <td>
								<input type="text" class="input_32_table" name="dns_probe_host" maxlength="255" autocorrect="off" autocapitalize="off" value="<% nvram_get("dns_probe_host"); %>">
						</td>
					</tr>

					<tr>
						<th><#Respond_IP#></th>
						<td>
								<input type="text" class="input_32_table" name="dns_probe_content" maxlength="1024" autocorrect="off" autocapitalize="off" value="<% nvram_get("dns_probe_content"); %>">
						</td>
					</tr>

					<tr>
						<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(26,2);"><#Ping_Target#></a></th>
						<td>
								<input type="text" class="input_32_table" name="wandog_target" maxlength="100" value="<% nvram_get("wandog_target"); %>" placeholder="ex: www.google.com" autocorrect="off" autocapitalize="off">
								<img id="pull_arrow" class="pull_arrow" height="14px;" src="/images/arrow-down.gif" style="position:absolute;*margin-left:-3px;*margin-top:1px;" onclick="pullLANIPList(this);" title="<#select_network_host#>" onmouseover="over_var=1;" onmouseout="over_var=0;">
								<div id="ClientList_Block_PC" name="ClientList_Block_PC" class="ClientList_Block_PC" style="display:none;"></div>
						</td>
					</tr>

				</table>
				<!-- -----------Enable Ping time watch dog end----------------------- -->									
												
				<!-- -----------Enable Routing rules table start----------------------- -->				
	    		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable" style="margin-top:8px;" id="routing_table">
					  <thead>
					  <tr>
						<td colspan="2"><#dualwan_routing_rule#></td>
					  </tr>
					  </thead>		

          				<tr>
            				<th><#dualwan_routing_rule_enable#></th>
            				<td>
						  		<input type="radio" value="1" name="wans_routing_enable" onClick="enable_lb_rules(this.value)" class="content_input_fd" <% nvram_match("wans_routing_enable", "1", "checked"); %>><#checkbox_Yes#>
		 						<input type="radio" value="0" name="wans_routing_enable" onClick="enable_lb_rules(this.value)" class="content_input_fd" <% nvram_match("wans_routing_enable", "0", "checked"); %>><#checkbox_No#>
							</td>
		  			</tr>		  			  				
          		</table>									
									
				<!-- ----------Routing Rules Table  ---------------- -->
				<div id="Routing_rules_table"></div>
        			
        	<!-- manually assigned the DHCP List end-->			
	
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
    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
</form>
<div id="footer"></div>
</body>
</html>
