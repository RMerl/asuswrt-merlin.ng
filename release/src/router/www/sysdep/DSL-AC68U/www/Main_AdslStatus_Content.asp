<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - <#menu_dsl_log#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">

<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<style>
.FormTable_log{
	font-size:12px;
	font-family:Arial, Helvetica, sans-serif;
	border: 1px solid #000000;
	border-collapse: collapse;
}
.FormTable_log thead td{
	color: #FFFFFF;
	font-size:12px;
	background-color:#405054;
}
.FormTable_log thead th{
	color: #FFFFFF;
	font-size:12px;
	background-color:#4D595D;
	text-align:left;
	font-weight:bolder;
	border: 1px solid #222;
	padding: 3px;
	padding-left: 10px;
	border-collapse: collapse;
 	background: #92A0A5; /* Old browsers */
	background: -moz-linear-gradient(top, #92A0A5  0%, #66757C 100%); /* FF3.6+ */
	background: -webkit-gradient(linear, left top, left bottom, color-stop(0%,#92A0A5 ), color-stop(100%,#66757C)); /* Chrome,Safari4+ */
	background: -webkit-linear-gradient(top, #92A0A5  0%, #66757C 100%); /* Chrome10+,Safari5.1+ */
	background: -o-linear-gradient(top,  #92A0A5 0%, #66757C 100%); /* Opera 11.10+ */
	background: -ms-linear-gradient(top,  #92A0A5  0%, #66757C 100%); /* IE10+ */
	background: linear-gradient(to bottom, #92A0A5  0%, #66757C 100%); /* W3C */
	filter: progid:DXImageTransform.Microsoft.gradient( startColorstr='#92A0A5', endColorstr='#66757C',GradientType=0 ); /* IE6-9 */
}
.FormTable_log th{
	font-family:Arial, Helvetica, sans-serif;
	background-color:#1F2D35;
	color:#FFFFFF;	/*Viz add*/
	font-weight:normal;
	line-height:15px;
	height: 30px;
	text-align:left;
	font-size:12px;
	padding-left: 10px;
	border: 1px solid #222;
	border-collapse: collapse;
	background:	#2F3A3E;
}
.FormTable_log td{
	padding-left: 10px;
	background-color:#475A5F;
	border: 1px solid #222;
	border-collapse: collapse;
}
.FormTable_log td span{
	background-color:#475a5f;
	color:#FFCC00;
}
.color_ch{
	color:#CC2200;
}
</style>
<script>

var adsl_timestamp = parseInt("<% nvram_get("adsl_timestamp"); %>");
var sync_status = "<% nvram_get("dsltmp_adslsyncsts"); %>";
var adsl_timestamp_update = parseInt("<% nvram_get("adsl_timestamp"); %>");
var sync_status_update = "<% nvram_get("dsltmp_adslsyncsts"); %>";
var status_isVDSLmode = "<% nvram_get("dsllog_xdslmode"); %>";
var adsl_boottime = boottime - adsl_timestamp;
var dsl_type = "<% nvram_get("dsllog_adsltype"); %>".replace("_", " ");

var SystemVendorID_orig = "<% nvram_get("dsllog_sysvid"); %>";
var SystemVendorModelID_orig = "<% nvram_get("dsllog_sysvmid"); %>";
var ModemVendorID_orig = "<% nvram_get("dsllog_modemvid"); %>";

var log_Opmode;
var log_AdslType;
var log_SNRMarginDown;
var log_SNRMarginUp;
var log_AttenDown;
var log_AttenUp;
var log_TCM;
var log_PathModeDown;
var log_IntDepthDown;
var log_PathModeUp;
var log_IntDepthUp;
var log_DataRateDown;
var log_DataRateUp;
var log_AttainDown;
var log_AttainUp;
var log_PowerDown;
var log_PowerUp;
var log_INPDown;
var log_INPUp;
var log_INPSHINEDown;
var log_INPSHINEUp;
var log_INPREINDown;
var log_INPREINUp;
var log_CRCDown;
var log_CRCUp;
var log_FarEndVendorID;
var log_VDSL_CurrentProfile;
var log_VDSLBAND_SNRMarginDown;
var log_VDSLBAND_SNRMarginUp;
var log_VDSLBAND_LATNDown;
var log_VDSLBAND_LATNUp;
var log_VDSLBAND_SATNDown;
var log_VDSLBAND_SATNUp;

function display_basic_dsl_information(){
	if(status_isVDSLmode == "VDSL")
	{
		document.getElementById("tr_VDSL_CurrentProfile").style.display = "";
	}
	else
	{
		document.getElementById("tr_VDSL_CurrentProfile").style.display = "none";
	}
}

function display_line_stats(){
	if(sync_status_update == "up")
	{
		document.getElementById("line_stats").style.display = "";
	}
	else
	{
		document.getElementById("line_stats").style.display = "none";
	}
}

function display_vdsl_band_status(){
	if(status_isVDSLmode == "VDSL")
	{
		document.getElementById("vdsl_band_status").style.display = "";
	}
	else
	{
		document.getElementById("vdsl_band_status").style.display = "none";
	}
}

function update_log(){
	$.ajax({
		url: 'ajax_AdslStatus.asp',
		dataType: 'script',
		error: function(xhr){
				setTimeout("update_log();", 1000);
			},
 	
		success: function(){
				if(adsl_timestamp_update != "" && sync_status != sync_status_update){
					adsl_boottime = boottime - adsl_timestamp_update;
					showadslbootTime();
				}				
				sync_status = sync_status_update;

				document.getElementById("div_lineState").innerHTML = sync_status_update;

				display_basic_dsl_information();
				if(sync_status_update == "up")
				{
					document.getElementById("div_Opmode").innerHTML = log_Opmode;
					document.getElementById("div_AdslType").innerHTML = log_AdslType;
					document.getElementById("div_FarEndVendorID").innerHTML = log_FarEndVendorID;
					if(status_isVDSLmode == "VDSL")
					{
						document.getElementById("div_VDSL_CurrentProfile").innerHTML = log_VDSL_CurrentProfile;
					}
				}
				else
				{
					document.getElementById("div_Opmode").innerHTML = "";
					document.getElementById("div_AdslType").innerHTML = "";
					document.getElementById("div_FarEndVendorID").innerHTML = "";
					document.getElementById("div_VDSL_CurrentProfile").innerHTML = "";
				}

				display_line_stats();
				if(sync_status_update == "up")
				{
				document.getElementById("div_SNRMarginDown").innerHTML = log_SNRMarginDown;
				document.getElementById("div_SNRMarginUp").innerHTML = log_SNRMarginUp;
				document.getElementById("div_AttenDown").innerHTML = log_AttenDown;
				document.getElementById("div_AttenUp").innerHTML = log_AttenUp;
				document.getElementById("div_TCMDown").innerHTML = log_TCM;
				document.getElementById("div_TCMUp").innerHTML = log_TCM;
				document.getElementById("div_PathModeDown").innerHTML = log_PathModeDown;
				document.getElementById("div_IntDepthDown").innerHTML = log_IntDepthDown;
				document.getElementById("div_PathModeUp").innerHTML = log_PathModeUp;
				document.getElementById("div_IntDepthUp").innerHTML = log_IntDepthUp;
				document.getElementById("div_DataRateDown").innerHTML = log_DataRateDown;
				document.getElementById("div_DataRateUp").innerHTML = log_DataRateUp;
				document.getElementById("div_AttainDown").innerHTML = log_AttainDown;
				document.getElementById("div_AttainUp").innerHTML = log_AttainUp;
				document.getElementById("div_PowerDown").innerHTML = log_PowerDown;
				document.getElementById("div_PowerUp").innerHTML = log_PowerUp;
				document.getElementById("div_INPDown").innerHTML = log_INPDown;
				document.getElementById("div_INPUp").innerHTML = log_INPUp;
				document.getElementById("div_INPSHINEDown").innerHTML = log_INPSHINEDown;
				document.getElementById("div_INPSHINEUp").innerHTML = log_INPSHINEUp;
				document.getElementById("div_INPREINDown").innerHTML = log_INPREINDown;
				document.getElementById("div_INPREINUp").innerHTML = log_INPREINUp;
				document.getElementById("div_CRCDown").innerHTML = log_CRCDown;
				document.getElementById("div_CRCUp").innerHTML = log_CRCUp;
				}

				display_vdsl_band_status();
				if(status_isVDSLmode == "VDSL")
				{
					//VDSL Band Status:SNR Margin
					document.getElementById("div_VDSLBAND_SNRMarginDown1").innerHTML = log_VDSLBAND_SNRMarginDown[0].replace("N/A","-");
					document.getElementById("div_VDSLBAND_SNRMarginDown2").innerHTML = log_VDSLBAND_SNRMarginDown[1].replace("N/A","-");
					document.getElementById("div_VDSLBAND_SNRMarginDown3").innerHTML = log_VDSLBAND_SNRMarginDown[2].replace("N/A","-");
					document.getElementById("div_VDSLBAND_SNRMarginDown4").innerHTML = log_VDSLBAND_SNRMarginDown[3].replace("N/A","-");
					document.getElementById("div_VDSLBAND_SNRMarginUp0").innerHTML = log_VDSLBAND_SNRMarginUp[0].replace("N/A","-");
					document.getElementById("div_VDSLBAND_SNRMarginUp1").innerHTML = log_VDSLBAND_SNRMarginUp[1].replace("N/A","-");
					document.getElementById("div_VDSLBAND_SNRMarginUp2").innerHTML = log_VDSLBAND_SNRMarginUp[2].replace("N/A","-");
					document.getElementById("div_VDSLBAND_SNRMarginUp3").innerHTML = log_VDSLBAND_SNRMarginUp[3].replace("N/A","-");
					document.getElementById("div_VDSLBAND_SNRMarginUp4").innerHTML = log_VDSLBAND_SNRMarginUp[4].replace("N/A","-");

					//VDSL Band Status:Line Attenuation
					document.getElementById("div_VDSLBAND_LATNDown1").innerHTML = log_VDSLBAND_LATNDown[0].replace("N/A","-");
					document.getElementById("div_VDSLBAND_LATNDown2").innerHTML = log_VDSLBAND_LATNDown[1].replace("N/A","-");
					document.getElementById("div_VDSLBAND_LATNDown3").innerHTML = log_VDSLBAND_LATNDown[2].replace("N/A","-");
					document.getElementById("div_VDSLBAND_LATNDown4").innerHTML = log_VDSLBAND_LATNDown[3].replace("N/A","-");
					document.getElementById("div_VDSLBAND_LATNUp0").innerHTML = log_VDSLBAND_LATNUp[0].replace("N/A","-");
					document.getElementById("div_VDSLBAND_LATNUp1").innerHTML = log_VDSLBAND_LATNUp[1].replace("N/A","-");
					document.getElementById("div_VDSLBAND_LATNUp2").innerHTML = log_VDSLBAND_LATNUp[2].replace("N/A","-");
					document.getElementById("div_VDSLBAND_LATNUp3").innerHTML = log_VDSLBAND_LATNUp[3].replace("N/A","-");
					document.getElementById("div_VDSLBAND_LATNUp4").innerHTML = log_VDSLBAND_LATNUp[4].replace("N/A","-");

					//VDSL Band Status:Signal Attenuation
					document.getElementById("div_VDSLBAND_SATNDown1").innerHTML = log_VDSLBAND_SATNDown[0].replace("N/A","-");
					document.getElementById("div_VDSLBAND_SATNDown2").innerHTML = log_VDSLBAND_SATNDown[1].replace("N/A","-");
					document.getElementById("div_VDSLBAND_SATNDown3").innerHTML = log_VDSLBAND_SATNDown[2].replace("N/A","-");
					document.getElementById("div_VDSLBAND_SATNDown4").innerHTML = log_VDSLBAND_SATNDown[3].replace("N/A","-");
					document.getElementById("div_VDSLBAND_SATNUp0").innerHTML = log_VDSLBAND_SATNUp[0].replace("N/A","-");
					document.getElementById("div_VDSLBAND_SATNUp1").innerHTML = log_VDSLBAND_SATNUp[1].replace("N/A","-");
					document.getElementById("div_VDSLBAND_SATNUp2").innerHTML = log_VDSLBAND_SATNUp[2].replace("N/A","-");
					document.getElementById("div_VDSLBAND_SATNUp3").innerHTML = log_VDSLBAND_SATNUp[3].replace("N/A","-");
					document.getElementById("div_VDSLBAND_SATNUp4").innerHTML = log_VDSLBAND_SATNUp[4].replace("N/A","-");
				}

				setTimeout("update_log();", 5000);
			}	
	});		
}

function initial(){
	show_menu();
	display_basic_dsl_information();
	display_line_stats();
	display_vdsl_band_status();
	showadslbootTime();
	document.getElementById("div_AdslType").innerHTML = dsl_type;
	document.getElementById("tr_SystemVendorID").style.display = (SystemVendorID_orig != "")? "":"none";
	document.getElementById("tr_SystemVendorModelID").style.display = (SystemVendorModelID_orig != "")? "":"none";
	document.getElementById("tr_ModemVendorID").style.display = (ModemVendorID_orig != "")? "":"none";
	setTimeout("update_log();", 5000);
}

function showadslbootTime(){
	if(adsl_timestamp_update != "" && sync_status_update == "up")
	{
		if(adsl_boottime < 0)
			adsl_boottime = boottime - adsl_timestamp_update;
		Days = Math.floor(adsl_boottime / (60*60*24));
		Hours = Math.floor((adsl_boottime / 3600) % 24);
		Minutes = Math.floor(adsl_boottime % 3600 / 60);
		Seconds = Math.floor(adsl_boottime % 60);

		document.getElementById("boot_days").innerHTML = Days;
		document.getElementById("boot_hours").innerHTML = Hours;
		document.getElementById("boot_minutes").innerHTML = Minutes;
		document.getElementById("boot_seconds").innerHTML = Seconds;
		adsl_boottime += 1;
		setTimeout("showadslbootTime()", 1000);
	}
	else
	{
		document.getElementById("boot_days").innerHTML = "0";
		document.getElementById("boot_hours").innerHTML = "0";
		document.getElementById("boot_minutes").innerHTML = "0";
		document.getElementById("boot_seconds").innerHTML = "0";
	}
}

</script>
</head>

<body onload="initial();" onunLoad="return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" action="apply.cgi" target="hidden_frame">
<input type="hidden" name="current_page" value="Main_AdslStatus_Content.asp">
<input type="hidden" name="next_page" value="Main_AdslStatus_Content.asp">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
</form>
<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="17">&nbsp;</td>
		<td valign="top" width="202">
			<div id="mainMenu"></div>
			<div id="subMenu"></div>
		</td>

		<td valign="top">
			<div id="tabMenu" class="submenuBlock"></div>

			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
				<tr>
					<td align="left" valign="top">

			<table width="760px" border="0" cellpadding="5" cellspacing="0" bordercolor="#6b8fa3"  class="FormTitle" id="FormTitle">
				<tr>
					<td bgcolor="#4D595D" height="400px" colspan="3" valign="top">
						<div>&nbsp;</div>
						<div class="formfonttitle"><#System_Log#> - <#menu_dsl_log#></div>
						<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
						<div class="formfontdesc">This page shows the detailed DSL status.</div>
						<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable_log">
							<thead>
							<tr>
								<th colspan="2">DSL Information</th>
							</tr>
							</thead>
							<tr>
								<th width="36%">DSL <#FW_item2#></th>
								<td colspan="2">
									<% nvram_get("dsllog_fwver"); %>
								</td>
							</tr>
							<tr>
								<th><#adsl_fw_ver_itemname#></th>
								<td colspan="2">
									<% nvram_get("dsllog_drvver"); %>
								</td>
							</tr>
							<tr>
								<th><#adsl_link_sts_itemname#></th>
								<td colspan="2">
									<div id="div_lineState"><% nvram_get("dsltmp_adslsyncsts"); %></div>
								</td>
							</tr>
							<tr>
								<th>DSL <#General_x_SystemUpTime_itemname#></th>
								<td colspan="2">
									<span id="boot_days"></span> <#Day#> <span id="boot_hours"></span> <#Hour#> <span id="boot_minutes"></span> <#Minute#> <span id="boot_seconds"></span> <#Second#>
								</td>
							</tr>
							
							<tr>
								<th><#dslsetting_disc1#></th>
								<td colspan="2">
									<div id="div_Opmode"><% nvram_get("dsllog_opmode"); %></div>
								</td>
							</tr>
							<tr>
								<th><#dslsetting_disc2#></th>
								<td colspan="2">
									<div id="div_AdslType"></div>
								</td>
							</tr>
							<tr>
								<th>DSL Exchange (DSLAM)</th>
								<td colspan="2">
									<div id="div_FarEndVendorID"><% nvram_get("dsllog_farendvendorid"); %></div>
								</td>
							</tr>
							<tr id="tr_VDSL_CurrentProfile">
								<th>Current Profile</th>
								<td colspan="2">
									<div id="div_VDSL_CurrentProfile"><% nvram_get("dsllog_vdslcurrentprofile"); %></div>
								</td>
							</tr>
							<tr id="tr_SystemVendorID" style="display:none;">
								<th>System Vendor ID</th>
								<td colspan="2">
									<div><% nvram_get("dsllog_sysvid"); %></div>
								</td>
							</tr>
							<tr id="tr_SystemVendorModelID" style="display:none;">
								<th>System Vendor Model ID</th>
								<td colspan="2">
									<div><% nvram_get("dsllog_sysvmid"); %></div>
								</td>
							</tr>
							<tr id="tr_ModemVendorID" style="display:none;">
								<th>Modem Vendor ID</th>
								<td colspan="2">
									<div><% nvram_get("dsllog_modemvid"); %></div>
								</td>
							</tr>
						</table>
					</td>
				</tr>


				<tr id="line_stats">
					<td bgcolor="#4D595D" colspan="3" valign="top">
						<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable_log">
							<thead>
							<tr>
								<th colspan="3">Line Stats</th>
							</tr>
							<tr>
								<td width="36%" style="background-color:#2F3A3E"></td>
								<td width="32%">Downstream</td>
								<td width="32%">Upstream</td>
							</tr>
							</thead>
							<tr>
								<th>TCM(Trellis Coded Modulation)</th>
								<td>
									<div id="div_TCMDown"><% nvram_get("dsllog_tcm"); %></div>
								</td>
								<td>
									<div id="div_TCMUp"><% nvram_get("dsllog_tcm"); %></div>
								</td>
							</tr>
							<tr>
								<th>SNR</th>
								<td>
									<div id="div_SNRMarginDown"><% nvram_get("dsllog_snrmargindown"); %></div>
								</td>
								<td>
									<div id="div_SNRMarginUp"><% nvram_get("dsllog_snrmarginup"); %></div>
								</td>
							</tr>
							<tr>
								<th>Line Attenuation</th>
								<td>
									<div id="div_AttenDown"><% nvram_get("dsllog_attendown"); %></div>
								</td>
								<td>
									<div id="div_AttenUp"><% nvram_get("dsllog_attenup"); %></div>
								</td>
							</tr>
							<tr>
								<th>Path Mode</th>
								<td>
									<div id="div_PathModeDown"><% nvram_get("dsllog_pathmodedown"); %></div>
								</td>
								<td>
									<div id="div_PathModeUp"><% nvram_get("dsllog_pathmodeup"); %></div>
								</td>
							</tr>
							<tr>
								<th>Interleave Depth</th>
								<td>
									<div id="div_IntDepthDown"><% nvram_get("dsllog_interleavedepthdown"); %></div>
								</td>
								<td>
									<div id="div_IntDepthUp"><% nvram_get("dsllog_interleavedepthup"); %></div>
								</td>
							</tr>
							<tr>
								<th>Data Rate</th>
								<td>
									<div id="div_DataRateDown"><% nvram_get("dsllog_dataratedown"); %></div>
								</td>
								<td>
									<div id="div_DataRateUp"><% nvram_get("dsllog_datarateup"); %></div>
								</td>
							</tr>
							<tr>
								<th>MAX Rate</th>
								<td>
									<div id="div_AttainDown"><% nvram_get("dsllog_attaindown"); %></div>
								</td>
								<td>
									<div id="div_AttainUp"><% nvram_get("dsllog_attainup"); %></div>
								</td>
							</tr>
							<tr>
								<th>POWER</th>
								<td>
									<div id="div_PowerDown"><% nvram_get("dsllog_powerdown"); %></div>
								</td>
								<td>
									<div id="div_PowerUp"><% nvram_get("dsllog_powerup"); %></div>
								</td>
							</tr>
							<tr>
								<th>INP</th>
								<td>
									<div id="div_INPDown"><% nvram_get("dsllog_inpdown"); %></div>
								</td>
								<td>
									<div id="div_INPUp"><% nvram_get("dsllog_inpup"); %></div>
								</td>
							</tr>
							<tr>
								<th>INP-SHINE</th>
								<td>
									<div id="div_INPSHINEDown"><% nvram_get("dsllog_inpshinedown"); %></div>
								</td>
								<td>
									<div id="div_INPSHINEUp"><% nvram_get("dsllog_inpshineup"); %></div>
								</td>
							</tr>
							<tr>
								<th>INP-REIN</th>
								<td>
									<div id="div_INPREINDown"><% nvram_get("dsllog_inpreindown"); %></div>
								</td>
								<td>
									<div id="div_INPREINUp"><% nvram_get("dsllog_inpreinup"); %></div>
								</td>
							</tr>
							<tr>
								<th>CRC</th>
								<td>
									<div id="div_CRCDown"><% nvram_get("dsllog_crcdown"); %></div>
								</td>
								<td>
									<div id="div_CRCUp"><% nvram_get("dsllog_crcup"); %></div>
								</td>
							</tr>
						</table>
					</td>
				</tr>



				<tr id="vdsl_band_status">
					<td bgcolor="#4D595D" colspan="3" valign="top">
						<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable_log">
							<thead>
							<tr>
								<th colspan="10">VDSL Band Status</th>
							</tr>

							<tr>
								<td width="36%" style="background-color:#2F3A3E"></td>
								<td width="7%">U0</td>
								<td width="7%">D1</td>
								<td width="7%">U1</td>
								<td width="7%">D2</td>
								<td width="7%">U2</td>
								<td width="7%">D3</td>
								<td width="7%">U3</td>
								<td width="7%">D4</td>
								<td width="8%">U4</td>
							</tr>
							</thead>
							<tr>
								<th>Line Attenuation (dB)</th>
								<td style="color:#3CF;">
									<div id="div_VDSLBAND_LATNUp0"></div>
								</td>
								<td style="color:#FF9000;">
									<div id="div_VDSLBAND_LATNDown1"></div>
								</td>
								<td style="color:#3CF;">
									<div id="div_VDSLBAND_LATNUp1"></div>
								</td>
								<td style="color:#FF9000;">
									<div id="div_VDSLBAND_LATNDown2"></div>
								</td>
								<td style="color:#3CF;">
									<div id="div_VDSLBAND_LATNUp2"></div>
								</td>
								<td style="color:#FF9000;">
									<div id="div_VDSLBAND_LATNDown3"></div>
								</td>
								<td style="color:#3CF;">
									<div id="div_VDSLBAND_LATNUp3"></div>
								</td>
								<td style="color:#FF9000;">
									<div id="div_VDSLBAND_LATNDown4"></div>
								</td>
								<td style="color:#3CF;">
									<div id="div_VDSLBAND_LATNUp4"></div>
								</td>
							</tr>

							<tr>
								<th>Signal Attenuation (dB)</th>
								<td style="color:#3CF;">
									<div id="div_VDSLBAND_SATNUp0"></div>
								</td>
								<td style="color:#FF9000;">
									<div id="div_VDSLBAND_SATNDown1"></div>
								</td>
								<td style="color:#3CF;">
									<div id="div_VDSLBAND_SATNUp1"></div>
								</td>
								<td style="color:#FF9000;">
									<div id="div_VDSLBAND_SATNDown2"></div>
								</td>
								<td style="color:#3CF;">
									<div id="div_VDSLBAND_SATNUp2"></div>
								</td>
								<td style="color:#FF9000;">
									<div id="div_VDSLBAND_SATNDown3"></div>
								</td>
								<td style="color:#3CF;">
									<div id="div_VDSLBAND_SATNUp3"></div>
								</td>
								<td style="color:#FF9000;">
									<div id="div_VDSLBAND_SATNDown4"></div>
								</td>
								<td style="color:#3CF;">
									<div id="div_VDSLBAND_SATNUp4"></div>
								</td>
							</tr>

							<tr>
								<th>SNR Margin (dB)</th>
								<td style="color:#3CF;">
									<div id="div_VDSLBAND_SNRMarginUp0"></div>
								</td>
								<td style="color:#FF9000;">
									<div id="div_VDSLBAND_SNRMarginDown1"></div>
								</td>
								<td style="color:#3CF;">
									<div id="div_VDSLBAND_SNRMarginUp1"></div>
								</td>
								<td style="color:#FF9000;">
									<div id="div_VDSLBAND_SNRMarginDown2"></div>
								</td>
								<td style="color:#3CF;">
									<div id="div_VDSLBAND_SNRMarginUp2"></div>
								</td>
								<td style="color:#FF9000;">
									<div id="div_VDSLBAND_SNRMarginDown3"></div>
								</td>
								<td style="color:#3CF;">
									<div id="div_VDSLBAND_SNRMarginUp3"></div>
								</td>
								<td style="color:#FF9000;">
									<div id="div_VDSLBAND_SNRMarginDown4"></div>
								</td>
								<td style="color:#3CF;">
									<div id="div_VDSLBAND_SNRMarginUp4"></div>
								</td>
							</tr>
						</table>
					</td>
				</tr>

				<tr class="apply_gen" valign="top">
					<td width="100%" align="center">
						<form method="post" name="form3" action="apply.cgi">
							<input type="hidden" name="current_page" value="Main_AdslStatus_Content.asp">
							<input type="hidden" name="action_mode" value=" Refresh ">
							<input type="button" onClick="location.href=location.href" value="<#CTL_refresh#>" class="button_gen">
						</form>
					</td>
				</tr>
			</table>

					</td>
				</tr>
			</table>

		</td>
		<td width="10" align="center" valign="top"></td>
	</tr>
</table>
<div id="footer"></div>
		</form>
</body>
</html>
