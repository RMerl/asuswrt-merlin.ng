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
<title><#Web_Title#> - <#menu_dsl_setting#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script>

var log_xDSLmode;
var log_lineState;
var log_SNRMarginDown;
var dslx_ginp_try_enable = "<%nvram_get("dslx_ginp_try_enable");%>";
var dslx_ginp_try_enable_disp = "<%nvram_get("dslx_ginp_try_enable_disp");%>";

function initial(){
	show_menu();
	change_dla("<% nvram_get("dslx_dla_enable"); %>");
	hide_nonstd_vectoring(<% nvram_get("dslx_vdsl_vectoring"); %>);
	hide_vdtxpwrtestmode("<% nvram_get("dslx_vdtxpwrtestmode"); %>");
	setTimeout("update_current_SNR_margin();", 3000);

	if(dslx_ginp_try_enable_disp == 1) {
		document.getElementById('dslx_ginp_try_checkbox').style.display = "";
		if(dslx_ginp_try_enable == 1) {
			document.getElementById('dslx_ginp_try_checkbox').checked = true;
		}
		else {
			document.getElementById('dslx_ginp_try_checkbox').checked = false;
		}
		check_ginp_try(document.getElementById('dslx_ginp_try_checkbox'));
	}
	else {
		document.getElementById('dslx_ginp_try_checkbox').style.display = "none";
	}
}

function update_current_SNR_margin(){
	$.ajax({
		url: 'ajax_AdslSetting.asp',
		dataType: 'script',
		error: function(xhr){
				setTimeout("update_current_SNR_margin();", 1000);
			},

		success: function(){
				if(log_lineState == "up")
				{
					if(log_xDSLmode == "ADSL"){
						document.getElementById("id_sta_adj_adsl").style.display = "";
						document.getElementById("id_sta_adj_adsl").innerHTML = "Current SNR Margin: "+log_SNRMarginDown;
					}
					else if(log_xDSLmode == "VDSL"){
						document.getElementById("id_sta_adj_vdsl").style.display = "";
						document.getElementById("id_sta_adj_vdsl").innerHTML = "Current SNR Margin: "+log_SNRMarginDown;
					}
				}
				else{
					document.getElementById("id_sta_adj_adsl").style.display = "none";
					document.getElementById("id_sta_adj_vdsl").style.display = "none";
				}

				setTimeout("update_current_SNR_margin();", 5000);
			}
	});
}

function applyRule(){
	if(valid_form()){
		showLoading();
		document.form.submit();
	}
}

function valid_form(){
	return true;
}

function change_dla(enable){
	if(enable == "1") {	//Enable
		document.form.dslx_snrm_offset.style.display = "none";
		document.form.dslx_snrm_offset.disabled = true;		
		document.getElementById("dslx_snrm_offset_read").style.display = "";
		document.getElementById("dslx_snrm_offset_read").innerHTML = get_snrm_offset();
		document.getElementById("dslx_snrm_offset_read").title = "Auto configured by Dynamic Line Adjustment(DLA)";

		document.form.dslx_vdsl_esnp.style.display = "none";
		document.form.dslx_vdsl_esnp.disabled = true;
		document.getElementById("dslx_vdsl_esnp_read").style.display = "";
		document.getElementById("dslx_vdsl_esnp_read").innerHTML = get_vdsl_esnp();
		document.getElementById("dslx_vdsl_esnp_read").title = "Auto configured by Dynamic Line Adjustment(DLA)";
	}
	else {
		document.form.dslx_snrm_offset.style.display = "";
		document.form.dslx_snrm_offset.disabled = false;
		document.getElementById("dslx_snrm_offset_read").style.display = "none";

		document.form.dslx_vdsl_esnp.style.display = "";
		document.form.dslx_vdsl_esnp.disabled = false;
		document.getElementById("dslx_vdsl_esnp_read").style.display = "none";
	}
}

function get_snrm_offset(){
	var offset_ori = "<% nvram_get("dslx_snrm_offset"); %>";
	switch(offset_ori){
		case "0":
					return "<#btn_Disabled#>";
					break;
		case "5120":
					return "10 dB";
					break;
		case "4608":
					return "9 dB";
					break;
		case "4096":
					return "8 dB";
					break;
		case "3584":
					return "7 dB";
					break;
		case "3072":
					return "6 dB";
					break;
		case "2560":
					return "5 dB";
					break;
		case "2048":
					return "4 dB";
					break;
		case "1536":
					return "3 dB";
					break;
		case "1024":
					return "2 dB";
					break;
		case "512":
					return "1 dB";
					break;
		case "-512":
					return "-1 dB";
					break;
		case "-1024":
					return "-2 dB";
					break;
		case "-1536":
					return "-3 dB";
					break;
		case "-2048":
					return "-4 dB";
					break;
		case "-2560":
					return "-5 dB";
					break;
		case "-3072":
					return "-6 dB";
					break;
		case "-3584":
					return "-7 dB";
					break;
		case "-4096":
					return "-8 dB";
					break;
		case "-4608":
					return "-9 dB";
					break;
		case "-5120":
					return "-10 dB";
					break;
		default:
					return "<#btn_Disabled#>";
					break;
	}

}

function get_vdsl_rx_agc(){
	var dslx_vdsl_rx_agc="<%nvram_get("dslx_vdsl_rx_agc");%>";
	switch(dslx_vdsl_rx_agc){
		case "394":
			return "Stable";
		case "550":
			return "High Performance";
		case "65535":
		default:
			return "Default";
	}
}

function get_vdsl_esnp(){
	var dslx_vdsl_esnp="<%nvram_get("dslx_vdsl_esnp");%>";
	switch(dslx_vdsl_esnp){
		case "1":
			return "Stable";
		case "0":
		default:
			return "Default";
	}
}


function hide_nonstd_vectoring(_value){
document.getElementById("nonstd_vectoring").style.display = (_value == "0") ? "none" : "";
}

function hide_vdtxpwrtestmode(_value){
document.getElementById("id_vdtxpwrtestmode").style.display = (_value == "1") ? "" : "none";
}

function get_dslx_ginp(){
	var dslx_ginp="<%nvram_get("dslx_ginp");%>";
	switch(dslx_ginp){
		case "1":
			return "<#btn_Enabled#>";
		case "0":
		default:
			return "<#btn_Disabled#>";
	}
}

function check_ginp_try(obj){
	if(obj.checked)
	{
		document.form.dslx_ginp.style.display = "none";
		document.form.dslx_ginp.disabled = true;
		document.getElementById("dslx_ginp_read").style.display = "";
		document.getElementById("dslx_ginp_read").innerHTML = get_dslx_ginp();
		document.form.dslx_ginp_try_enable.value = 1;
	}
	else
	{
		document.form.dslx_ginp.style.display = "";
		document.form.dslx_ginp.disabled = false;
		document.getElementById("dslx_ginp_read").style.display = "none";
		document.form.dslx_ginp_try_enable.value = 0;
	}
}

</script>
</head>

<body onload="initial();" onunLoad="return unload_body();">
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
<input type="hidden" name="current_page" value="Advanced_ADSL_Content.asp">
<input type="hidden" name="next_page" value="Advanced_ADSL_Content.asp">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_dsl_setting">
<input type="hidden" name="action_wait" value="20">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="dslx_ginp_try_enable" value="<% nvram_get("dslx_ginp_try_enable"); %>">

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
		  <div class="formfonttitle"><#menu5_6#> - <#menu_dsl_setting#></div>
      <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
      <div class="formfontdesc"><#dslsetting_disc0#></div>

		<table width="99%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
			<thead>
			<tr>
				<td colspan="2">Common Settings</td>	
			</tr>		
			</thead>
			<tr>
				<th>
					<#dslsetting_disc1#>
				</th>
				<td>
					<select id="" class="input_option" name="dslx_modulation">
						<option value="0" <% nvram_match("dslx_modulation", "0", "selected"); %>>T1.413</option>
						<option value="1" <% nvram_match("dslx_modulation", "1", "selected"); %>>G.lite</option>
						<option value="2" <% nvram_match("dslx_modulation", "2", "selected"); %>>G.Dmt</option>
						<option value="3" <% nvram_match("dslx_modulation", "3", "selected"); %>>ADSL2</option>
						<option value="4" <% nvram_match("dslx_modulation", "4", "selected"); %>>ADSL2+</option>
						<option value="6" <% nvram_match("dslx_modulation", "6", "selected"); %>>VDSL2</option>
						<option value="5" <% nvram_match("dslx_modulation", "5", "selected"); %>>Auto Sync-Up</option>
					</select>
				</td>
			</tr>
			<tr>
				<th>
					<#dslsetting_disc2#>
				</th>
				<td>
					<select id="" class="input_option" name="dslx_annex">
						<option value="0" <% nvram_match("dslx_annex", "0", "selected"); %>>ANNEX A</option>
						<option value="1" <% nvram_match("dslx_annex", "1", "selected"); %>>ANNEX I</option>
						<option value="2" <% nvram_match("dslx_annex", "2", "selected"); %>>ANNEX A/L</option>
						<option value="3" <% nvram_match("dslx_annex", "3", "selected"); %>>ANNEX M</option>
						<option value="4" <% nvram_match("dslx_annex", "4", "selected"); %>>ANNEX A/I/J/L/M</option>
						<option value="5" <% nvram_match("dslx_annex", "5", "selected"); %>>ANNEX B</option>
						<option value="6" <% nvram_match("dslx_annex", "6", "selected"); %>>ANNEX B/J</option>
					</select>
				</td>
			</tr>
			<tr>
				<th>
					<a class="hintstyle" href="javascript:void(0);" onClick="openHint(25,10);">Dynamic Line Adjustment (DLA)</a>
				</th>
				<td>
					<select id="" class="input_option" name="dslx_dla_enable" onchange='change_dla(this.value);'>
						<option value="1" <% nvram_match("dslx_dla_enable", "1", "selected"); %>><#btn_Enabled#></option>
						<option value="0" <% nvram_match("dslx_dla_enable", "0", "selected"); %>><#btn_Disabled#></option>
					</select>
				</td>
			</tr>
			
			
			<tr>
				<th>
					<a class="hintstyle" href="javascript:void(0);" onClick="openHint(25,2);"><#dslsetting_SRA#></a>
				</th>
				<td>
					<select id="" class="input_option" name="dslx_sra">
						<option value="1" <% nvram_match("dslx_sra", "1", "selected"); %>><#btn_Enabled#></option>
						<option value="0" <% nvram_match("dslx_sra", "0", "selected"); %>><#btn_Disabled#></option>
					</select>
				</td>
			</tr>
						
			<tr>
				<th>
					<a class="hintstyle" href="javascript:void(0);" onClick="openHint(25,12);">G.INP (G.998.4)</a>
				</th>
				<td>
					<select id="" class="input_option" name="dslx_ginp">
						<option value="1" <% nvram_match("dslx_ginp", "1", "selected"); %>><#btn_Enabled#></option>
						<option value="0" <% nvram_match("dslx_ginp", "0", "selected"); %>><#btn_Disabled#></option>
					</select>
					<span id="dslx_ginp_read" style="display:none;color:#FFFFFF;"></span>
					<span id="dslx_ginp_try_checkbox" style="display:none;">
						<br/>
						<input type="checkbox" onClick="check_ginp_try(this);" <% nvram_match("dslx_ginp_try_enable", "1", "checked"); %> >Try to connect with G.INP Enabled</input><!-- Untranslated -->
					</span>
				</td>
			</tr>
			
		</table>
		
		
		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" style="margin-top:10px;">
		<thead>
		<tr>
			<td colspan="2">ADSL Settings</td>	
		</tr>		
		</thead>
		
		<tr>
			<th>
				<a class="hintstyle" href="javascript:void(0);" onClick="openHint(25,1);"><#dslsetting_Stability_Adj#></a>
			</th>
			<td>
				<span id="dslx_snrm_offset_read" name="dslx_snrm_offset_read" style="display:none;color:#FFFFFF;"></span>
				<select id="dslx_snrm_offset" class="input_option" name="dslx_snrm_offset" style="display:none;">
					<option value="0" <% nvram_match("dslx_snrm_offset", "0", "selected"); %>><#btn_Disabled#></option>
					<option value="5120" <% nvram_match("dslx_snrm_offset", "5120", "selected"); %>>10 dB</option>
					<option value="4608" <% nvram_match("dslx_snrm_offset", "4608", "selected"); %>>9 dB</option>
					<option value="4096" <% nvram_match("dslx_snrm_offset", "4096", "selected"); %>>8 dB</option>
					<option value="3584" <% nvram_match("dslx_snrm_offset", "3584", "selected"); %>>7 dB</option>
					<option value="3072" <% nvram_match("dslx_snrm_offset", "3072", "selected"); %>>6 dB</option>
					<option value="2560" <% nvram_match("dslx_snrm_offset", "2560", "selected"); %>>5 dB</option>
					<option value="2048" <% nvram_match("dslx_snrm_offset", "2048", "selected"); %>>4 dB</option>
					<option value="1536" <% nvram_match("dslx_snrm_offset", "1536", "selected"); %>>3 dB</option>
					<option value="1024" <% nvram_match("dslx_snrm_offset", "1024", "selected"); %>>2 dB</option>
					<option value="512" <% nvram_match("dslx_snrm_offset", "512", "selected"); %>>1 dB</option>
					<option value="-512" <% nvram_match("dslx_snrm_offset", "-512", "selected"); %>>-1 dB</option>
					<option value="-1024" <% nvram_match("dslx_snrm_offset", "-1024", "selected"); %>>-2 dB</option>
					<option value="-1536" <% nvram_match("dslx_snrm_offset", "-1536", "selected"); %>>-3 dB</option>
					<option value="-2048" <% nvram_match("dslx_snrm_offset", "-2048", "selected"); %>>-4 dB</option>
					<option value="-2560" <% nvram_match("dslx_snrm_offset", "-2560", "selected"); %>>-5 dB</option>
					<option value="-3072" <% nvram_match("dslx_snrm_offset", "-3072", "selected"); %>>-6 dB</option>
					<option value="-3584" <% nvram_match("dslx_snrm_offset", "-3584", "selected"); %>>-7 dB</option>
					<option value="-4096" <% nvram_match("dslx_snrm_offset", "-4096", "selected"); %>>-8 dB</option>
					<option value="-4608" <% nvram_match("dslx_snrm_offset", "-4608", "selected"); %>>-9 dB</option>
					<option value="-5120" <% nvram_match("dslx_snrm_offset", "-5120", "selected"); %>>-10 dB</option>
				</select>
				<span id="id_sta_adj_adsl" style="display:none;margin-left:10px;"></span>
			</td>
		</tr>
		<tr>
			<th>
				<a class="hintstyle" href="javascript:void(0);" onClick="openHint(25,13);">Rx AGC GAIN Adjustment</a>
			</th>
			<td>
				<select id="dslx_adsl_rx_agc" class="input_option" name="dslx_adsl_rx_agc">
					<option value="Default" <% nvram_match("dslx_adsl_rx_agc", "Default", "selected"); %>><#Setting_factorydefault_value#></option>
					<option value="Stable" <% nvram_match("dslx_adsl_rx_agc", "Stable", "selected"); %>>Stable</option>
					<option value="High Performance" <% nvram_match("dslx_adsl_rx_agc", "High Performance", "selected"); %>>High Performance</option>
				</select>
			</td>
		</tr>
		<tr>
			<th>
				<a class="hintstyle" href="javascript:void(0);" onClick="openHint(25,16);">ESNP - Enhanced Sudden Noise Protection</a>
			</th>
			<td>
				<select id="dslx_adsl_esnp" class="input_option" name="dslx_adsl_esnp">
					<option value="0" <% nvram_match("dslx_adsl_esnp", "0", "selected"); %>><#Setting_factorydefault_value#></option>
					<option value="1" <% nvram_match("dslx_adsl_esnp", "1", "selected"); %>>Stable</option>
				</select>
			</td>
		</tr>
		<tr>
			<th>
				<a class="hintstyle" href="javascript:void(0);" onClick="openHint(25,3);">Bitswap</a>
			</th>
			<td>
				<select id="" class="input_option" name="dslx_bitswap">
					<option value="1" <% nvram_match("dslx_bitswap", "1", "selected"); %>><#btn_Enabled#></option>
					<option value="0" <% nvram_match("dslx_bitswap", "0", "selected"); %>><#btn_Disabled#></option>
				</select>
			</td>
		</tr>
		
		</table>
		
		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" style="margin-top:10px;">
		<thead>
		<tr>
			<td colspan="2">VDSL Settings</td>	
		</tr>
		</thead>
		<tr>
			<th>
				<a class="hintstyle" href="javascript:void(0);" onClick="openHint(25,8);">VDSL Profile</a>
			</th>
			<td>
				<select id="" class="input_option" name="dslx_vdsl_profile">
					<option value="0" <% nvram_match("dslx_vdsl_profile", "0", "selected"); %>>30a multi mode</option>
					<option value="1" <% nvram_match("dslx_vdsl_profile", "1", "selected"); %>>17a multi mode</option>
					<option value="2" <% nvram_match("dslx_vdsl_profile", "2", "selected"); %>>12a multi mode</option>
					<option value="3" <% nvram_match("dslx_vdsl_profile", "3", "selected"); %>>8a multi mode</option>
				</select>
			</td>
		</tr>
		<tr>
			<th>
				<a class="hintstyle" href="javascript:void(0);" onClick="openHint(25,4);"><#dslsetting_Stability_Adj#></a>
			</th>
			<td>
				<select id="" class="input_option" name="dslx_vdsl_target_snrm">
					<option value="32767" <% nvram_match("dslx_vdsl_target_snrm", "32767", "selected"); %>><#btn_Disabled#></option>
					<option value="1024" <% nvram_match("dslx_vdsl_target_snrm", "1024", "selected"); %>>2 dB</option>
					<option value="1536" <% nvram_match("dslx_vdsl_target_snrm", "1536", "selected"); %>>3 dB</option>
					<option value="2048" <% nvram_match("dslx_vdsl_target_snrm", "2048", "selected"); %>>4 dB</option>
					<option value="2560" <% nvram_match("dslx_vdsl_target_snrm", "2560", "selected"); %>>5 dB</option>
					<option value="3072" <% nvram_match("dslx_vdsl_target_snrm", "3072", "selected"); %>>6 dB</option>
					<option value="3584" <% nvram_match("dslx_vdsl_target_snrm", "3584", "selected"); %>>7 dB</option>
					<option value="4096" <% nvram_match("dslx_vdsl_target_snrm", "4096", "selected"); %>>8 dB</option>
					<option value="4608" <% nvram_match("dslx_vdsl_target_snrm", "4608", "selected"); %>>9 dB</option>
					<option value="5120" <% nvram_match("dslx_vdsl_target_snrm", "5120", "selected"); %>>10 dB</option>
					<option value="5632" <% nvram_match("dslx_vdsl_target_snrm", "5632", "selected"); %>>11 dB</option>
					<option value="6144" <% nvram_match("dslx_vdsl_target_snrm", "6144", "selected"); %>>12 dB</option>
					<option value="6656" <% nvram_match("dslx_vdsl_target_snrm", "6656", "selected"); %>>13 dB</option>
					<option value="7168" <% nvram_match("dslx_vdsl_target_snrm", "7168", "selected"); %>>14 dB</option>
					<option value="7680" <% nvram_match("dslx_vdsl_target_snrm", "7680", "selected"); %>>15 dB</option>
					<option value="8192" <% nvram_match("dslx_vdsl_target_snrm", "8192", "selected"); %>>16 dB</option>
					<option value="8704" <% nvram_match("dslx_vdsl_target_snrm", "8704", "selected"); %>>17 dB</option>
					<option value="9216" <% nvram_match("dslx_vdsl_target_snrm", "9216", "selected"); %>>18 dB</option>
					<option value="9728" <% nvram_match("dslx_vdsl_target_snrm", "9728", "selected"); %>>19 dB</option>
					<option value="10240" <% nvram_match("dslx_vdsl_target_snrm", "10240", "selected"); %>>20 dB</option>
					<option value="10752" <% nvram_match("dslx_vdsl_target_snrm", "10752", "selected"); %>>21 dB</option>
					<option value="11264" <% nvram_match("dslx_vdsl_target_snrm", "11264", "selected"); %>>22 dB</option>
					<option value="11776" <% nvram_match("dslx_vdsl_target_snrm", "11776", "selected"); %>>23 dB</option>
					<option value="12288" <% nvram_match("dslx_vdsl_target_snrm", "12288", "selected"); %>>24 dB</option>
					<option value="12800" <% nvram_match("dslx_vdsl_target_snrm", "12800", "selected"); %>>25 dB</option>
					<option value="13312" <% nvram_match("dslx_vdsl_target_snrm", "13312", "selected"); %>>26 dB</option>
					<option value="13824" <% nvram_match("dslx_vdsl_target_snrm", "13824", "selected"); %>>27 dB</option>
					<option value="14336" <% nvram_match("dslx_vdsl_target_snrm", "14336", "selected"); %>>28 dB</option>
					<option value="14848" <% nvram_match("dslx_vdsl_target_snrm", "14848", "selected"); %>>29 dB</option>
					<option value="15360" <% nvram_match("dslx_vdsl_target_snrm", "15360", "selected"); %>>30 dB</option>
				</select>
				<span id="id_sta_adj_vdsl" style="display:none;margin-left:10px;"></span>
			</td>
		</tr>

		<!--dslx_vdsl_tx_gain_off-->
		<tr id="id_vdtxpwrtestmode">
			<th>
				<a class="hintstyle" href="javascript:void(0);" onClick="openHint(25,5);">Tx Power Control</a>
			</th>
			<td>
				<select id="" class="input_option" name="dslx_vdsl_tx_gain_off">
					<option value="32767" <% nvram_match("dslx_vdsl_tx_gain_off", "32767", "selected"); %>><#btn_Disabled#></option>
					<option value="30" <% nvram_match("dslx_vdsl_tx_gain_off", "30", "selected"); %>>3 dB</option>
					<option value="20" <% nvram_match("dslx_vdsl_tx_gain_off", "20", "selected"); %>>2 dB</option>
					<option value="10" <% nvram_match("dslx_vdsl_tx_gain_off", "10", "selected"); %>>1 dB</option>
					<option value="0" <% nvram_match("dslx_vdsl_tx_gain_off", "0", "selected"); %>>0 dB</option>
					<option value="-10" <% nvram_match("dslx_vdsl_tx_gain_off", "-10", "selected"); %>>-1 dB</option>
					<option value="-20" <% nvram_match("dslx_vdsl_tx_gain_off", "-20", "selected"); %>>-2 dB</option>
					<option value="-30" <% nvram_match("dslx_vdsl_tx_gain_off", "-30", "selected"); %>>-3 dB</option>
					<option value="-40" <% nvram_match("dslx_vdsl_tx_gain_off", "-40", "selected"); %>>-4 dB</option>
					<option value="-50" <% nvram_match("dslx_vdsl_tx_gain_off", "-50", "selected"); %>>-5 dB</option>
					<option value="-60" <% nvram_match("dslx_vdsl_tx_gain_off", "-60", "selected"); %>>-6 dB</option>
					<option value="-70" <% nvram_match("dslx_vdsl_tx_gain_off", "-70", "selected"); %>>-7 dB</option>
				</select>
			</td>
		</tr>

		<!--vdsl_rx_agc-->
		<tr>
			<th>
				<a class="hintstyle" href="javascript:void(0);" onClick="openHint(25,6);">Rx AGC GAIN Adjustment</a>
			</th>
			<td>
				<select id="dslx_vdsl_rx_agc" class="input_option" name="dslx_vdsl_rx_agc">
					<option value="65535" <% nvram_match("dslx_vdsl_rx_agc", "65535", "selected"); %>><#Setting_factorydefault_value#></option>
					<option value="394" <% nvram_match("dslx_vdsl_rx_agc", "394", "selected"); %>>Stable</option>
					<option value="476" <% nvram_match("dslx_vdsl_rx_agc", "476", "selected"); %>>Balance</option>
					<option value="550" <% nvram_match("dslx_vdsl_rx_agc", "550", "selected"); %>>High Performance</option>
				</select>
			</td>
		</tr>

		<!--upbo stands for upstream power back off-->
		<tr>
			<th>
				<a class="hintstyle" href="javascript:void(0);" onClick="openHint(25,7);">UPBO - Upstream Power Back Off</a>
			</th>
			<td>
				<select id="" class="input_option" name="dslx_vdsl_upbo">
					<option value="auto" <% nvram_match("dslx_vdsl_upbo", "auto", "selected"); %>><#Auto#></option>
					<option value="on" <% nvram_match("dslx_vdsl_upbo", "on", "selected"); %>><#btn_Enabled#></option>
					<option value="off" <% nvram_match("dslx_vdsl_upbo", "off", "selected"); %>><#btn_Disabled#></option>
				</select>
			</td>
		</tr>
		<tr>
			<th>
				<a class="hintstyle" href="javascript:void(0);" onClick="openHint(25,16);">ESNP - Enhanced Sudden Noise Protection</a>
			</th>
			<td>
				<span id="dslx_vdsl_esnp_read" name="dslx_vdsl_esnp_read" style="display:none;color:#FFFFFF;"></span>
				<select id="dslx_vdsl_esnp" class="input_option" name="dslx_vdsl_esnp">
					<option value="0" <% nvram_match("dslx_vdsl_esnp", "0", "selected"); %>><#Setting_factorydefault_value#></option>
					<option value="1" <% nvram_match("dslx_vdsl_esnp", "1", "selected"); %>>Stable</option>
				</select>
			</td>
		</tr>
		<tr>
			<th>
				<a class="hintstyle" href="javascript:void(0);" onClick="openHint(25,3);">Bitswap</a>
			</th>
			<td>
				<select id="" class="input_option" name="dslx_vdsl_bitswap">
					<option value="1" <% nvram_match("dslx_vdsl_bitswap", "1", "selected"); %>><#btn_Enabled#></option>
					<option value="0" <% nvram_match("dslx_vdsl_bitswap", "0", "selected"); %>><#btn_Disabled#></option>
				</select>
			</td>
		</tr>
		<tr>
			<th>
				<a class="hintstyle" href="javascript:void(0);" onClick="openHint(25,14);">G.vector (G.993.5)</a>
			</th>
			<td>
				<select id="" class="input_option" name="dslx_vdsl_vectoring" onchange="hide_nonstd_vectoring(this.value);">
					<option value="1" <% nvram_match("dslx_vdsl_vectoring", "1", "selected"); %>><#btn_Enabled#></option>
					<option value="0" <% nvram_match("dslx_vdsl_vectoring", "0", "selected"); %>><#btn_Disabled#></option>
				</select>
			</td>
		</tr>
		<tr id="nonstd_vectoring">
			<th>
				<a class="hintstyle" href="javascript:void(0);" onClick="openHint(25,15);">Non-standard G.vector (G.993.5)</a>
			</th>
			<td>
				<select id="" class="input_option" name="dslx_vdsl_nonstd_vectoring">
					<option value="1" <% nvram_match("dslx_vdsl_nonstd_vectoring", "1", "selected"); %>><#btn_Enabled#></option>
					<option value="0" <% nvram_match("dslx_vdsl_nonstd_vectoring", "0", "selected"); %>><#btn_Disabled#></option>
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
