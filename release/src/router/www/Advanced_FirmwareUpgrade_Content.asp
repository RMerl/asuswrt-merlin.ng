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
<title><#Web_Title#> - <#menu5_6_3#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="css/confirm_block.css"></script>
<link rel="stylesheet" type="text/css" href="/device-map/amesh.css" />
<style>
.FormTable{
 	margin-top:10px;	
}	
.Bar_container{
	width:85%;
	height:21px;
	border:1px inset #999;
	margin:0 auto;
	margin-top:20px \9;
	background-color:#FFFFFF;
	z-index:100;
}
#proceeding_img_text{
	position:absolute; 
	z-index:101; 
	font-size:11px; color:#000000; 
	line-height:21px;
	width: 83%;
}
#proceeding_img{
 	height:21px;
	background:#C0D1D3 url(/images/proceeding_img.gif);
}

.button_helplink{
	font-weight: bolder;
	text-shadow: 1px 1px 0px black;
	text-align: center;
	vertical-align: middle;
	background: transparent url(/images/New_ui/contentbt_normal.png) no-repeat scroll center top;
	_background: transparent url(/images/New_ui/contentbt_normal_ie6.png) no-repeat scroll center top;
	border:0;
	color: #FFFFFF;
	height:33px;
	width:122px;
	font-family:Verdana;
	font-size:12px;
	overflow:visible;
	cursor:pointer;
	outline: none; /* for Firefox */
	hlbr:expression(this.onFocus=this.blur()); /* for IE */
	white-space:normal;
}
.button_helplink:hover{
	font-weight: bolder;
	background:url(/images/New_ui/contentbt_over.png) no-repeat scroll center top;
	height:33px;
 	width:122px;
	cursor:pointer;
	outline: none; /* for Firefox */
 	hlbr:expression(this.onFocus=this.blur()); /* for IE */
}
</style>

<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/validator.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/confirm_block.js"></script>
<script language="JavaScript" type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script language="JavaScript" type="text/javascript" src="/form.js"></script>
<script>
var webs_state_update = '<% nvram_get("webs_state_update"); %>';
var webs_state_upgrade = '<% nvram_get("webs_state_upgrade"); %>';
var webs_state_error = '<% nvram_get("webs_state_error"); %>';
var webs_state_info = '<% nvram_get("webs_state_info"); %>';
var webs_state_info_beta = '<% nvram_get("webs_state_info_beta"); %>';

var confirm_show = '<% get_parameter("confirm_show"); %>';
var webs_release_note= "";

var fwdl_percent="";
var varload = 0;
var helplink = "";
var dpi_engine_status = <%bwdpi_engine_status();%>;
var sig_ver_ori = '<% nvram_get("bwdpi_sig_ver"); %>';
var sig_update_t = '<% nvram_get("sig_update_t"); %>';
if(cfg_sync_support){
	var cfg_check = '<% nvram_get("cfg_check"); %>';
	var cfg_upgrade = '<% nvram_get("cfg_upgrade"); %>';
}


var download_srv = '<% nvram_get("firmware_server"); %>';
if (download_srv == "") {
	download_url = "https://asuswrt.lostrealm.ca/download";
	download_url_alpha = download_url;
	download_url_beta = download_url;
} else {
	download_url = download_srv + "/" + based_modelid;
	download_url_alpha = download_srv + "/alpha";
	download_url_beta = download_url + "/beta";
}

var amesh_offline_flag = false;
var interval_update_AiMesh_fw_status;

var firmver = '<% nvram_get("firmver"); %>';
var buildno = '<% nvram_get("buildno"); %>';
//var rcno = '<% nvram_get("rcno"); %>';
var extendno = '<% nvram_get("extendno"); %>';
var FWString = '';

FWString = buildno;
//if(rcno.length > 0)
//	FWString += "rc"+rcno;
if ((extendno != "") && (extendno != "0"))
	FWString += "_"+extendno;

function initial(){
	show_menu();

	showtext(document.getElementById("FWString"), FWString);

	if(amesh_support && (isSwMode("rt") || isSwMode("ap"))) {
		$(".aimesh_manual_fw_update_hint").css("display", "block");
		var get_cfg_clientlist_ori = [<% get_cfg_clientlist(); %>];
		var have_node = false;
		var get_cfg_clientlist = [];
		get_cfg_clientlist = get_cfg_clientlist_ori[0];
		$("#fw_version_tr").empty();
		var html = "";
		html += "<tr>";
		html += "<th>Check Update</th>";/* untranslated */
		html += "<td>";
		html += '<div id="update_div" style="display:none;">';
		html += '<input type="button" id="update" name="update" class="button_gen" onclick="show_offline_msg(true);" value="<#liveupdate#>" />';
		html += '<span id="beta_firmware_path_span" style="display:none;">';
		html += '<input type="checkbox" name="beta_firmware_path" id="beta_firmware_path" onclick="change_firmware_path(this.checked==true);"  <% nvram_match("firmware_path", "1", "checked"); %>><#get_beta#></input>';
		html += '</span>';
		html += '<div><input type="button" id="amas_update" class="button_gen" style="display:none;" onclick="cfgsync_firmware_upgrade();" value="<#CTL_upgrade#>"/><div>';
		html += '</div>';
		html += '<div id="linkpage_div" class="button_helplink" style="margin-left:200px;margin-top:-38px;display:none;">';
		html += '<a id="linkpage" target="_blank"><div style="padding-top:5px;"><#liveupdate#></div></a>';
		html += '</div>';
		html += '<div id="check_states">';
		html += '<span id="update_states"></span>';
		html += '<img id="update_scan" style="display:none;" src="images/InternetScan.gif" />';
		html += '</div>';
		html += "</td>";
		html += "</tr>";
		$("#fw_version_tr").before(html);

		var mac_id = '<% get_lan_hwaddr(); %>'.replace(/:/g, "");
		html = "";
		html += "<tr style='height:15px;'></tr>";
		html += "<tr>";
		html += "<td class='aimesh_node_category_bg' colspan='2'>AiMesh router</td>";/*untranslated*/
		html += "</tr>";
		html += "<tr>";
		html += "<th>";
		html += "<#Web_Title2#>";
		html += "</th>";
		html += "</th>";
		html += "<td id='amas_" + mac_id + "'>";
		html += "<div id='current_version'>Current Version : " + FWString + "</div>";/*untranslated*/
		html += "<div>";
		html += "Manual Firmware Update : ";/*untranslated*/
		html += "<span class='aimesh_fw_update_offline' style='margin-left:0px;' onclick='open_AiMesh_router_fw_upgrade();'><#CTL_upload#></span>";
		html += "</div>";
		html += "<div id='checkNewFW' style='display:none;'><#ADSL_FW_item3#> : <span class='checkFWReuslt'></span></div>";
		html += "</td>";
		html += "</tr>";
		$("#fw_version_tr").before(html);

		for (var idx in get_cfg_clientlist) {
			if (get_cfg_clientlist.hasOwnProperty(idx)) {
				if(idx == "0")
					continue;//filter CAP
				var model_name = get_cfg_clientlist[idx].model_name;
				var fwver = get_cfg_clientlist[idx].fwver;
				var online = get_cfg_clientlist[idx].online;
				var mac = get_cfg_clientlist[idx].mac;
				var mac_id = mac.replace(/:/g, "");
				var ip = get_cfg_clientlist[idx].ip;
				var alias = "My Home";
				if("config" in get_cfg_clientlist[idx]) {
					if("misc" in get_cfg_clientlist[idx].config) {
						if("cfg_alias" in get_cfg_clientlist[idx].config.misc) {
							if(get_cfg_clientlist[idx].config.misc.cfg_alias != "")
								alias = get_cfg_clientlist[idx].config.misc.cfg_alias;
						}
					}
				}
				html = "";
				if(!have_node) {
					html += "<tr>";
					html += "<td class='aimesh_node_category_bg' colspan='2'>AiMesh node</td>";/*untranslated*/
					html += "</tr>";
				}
				html += "<tr>";
				html += "<th>";
				html += model_name + " ( " + mac + " )";
				html += "<br>";
				html += "Location : " + alias;/* untranslated */
				html += "</th>";
				html += "<td id='amas_" + mac_id + "'>";
				html += "<div id='current_version'>Current Version : " + fwver + "</div>";/*untranslated*/
				html += "<div>";
				html += "Manual Firmware Update : ";/*untranslated*/
				if(online == "0") {
					html += "<span class='aimesh_fw_update_offline' style='margin-left:0px;' onclick='show_offline_msg(false);'>Offline</span>";/*untranslated*/
					amesh_offline_flag = true;
				}
				else
					html += "<span class='aimesh_fw_update_offline' style='margin-left:0px;' onclick='open_AiMesh_node_fw_upgrade(\"" + ip + "\");'><#CTL_upload#></span>";
				html += "</div>";
				html += "<div id='checkNewFW' style='display:none;'><#ADSL_FW_item3#> : <span class='checkFWReuslt'></span></div>";
				html += "</td>";
				html += "</tr>";
				$("#fw_version_tr").before(html);
				have_node = true;
			}
		}
		$("#fw_version_tr").remove();
		interval_update_AiMesh_fw_status = setInterval(update_AiMesh_fw, 5000);
	}


	if(bwdpi_support){
		if(dpi_engine_status.DpiEngine == 1)
			document.getElementById("sig_ver_field").style.display="";
		else
			document.getElementById("sig_ver_field").style.display="none";
			
		if(sig_ver_ori == "")
			document.getElementById("sig_ver_word").innerHTML = "1.008";
		else
			document.getElementById("sig_ver_word").innerHTML = sig_ver_ori;

		if(sig_update_t == "" || sig_update_t == "0")
			document.getElementById("sig_update_date").innerHTML = "";
		else
			document.getElementById("sig_update_date").innerHTML = "&nbsp;&nbsp;"+transferTimeFormat(sig_update_t*1000);
	}
	
	if(cfg_sync_support){
		if(cfg_upgrade != "" && cfg_upgrade != "10"){   //Show firmware is still downloading or fw upgrade loading bar if doing webs_upgrade.sh 
			startDownloading();
		}
	}
	else{
		if(webs_state_upgrade != "" && webs_state_upgrade != "1"){   //Show firmware is still downloading or fw upgrade loading bar if doing webs_upgrade.sh 
			startDownloading();
		}
	}

	if(no_update_support){
		document.getElementById("update_div").style.display = "none";
		document.getElementById("beta_firmware_path_span").style.display = "none";
		document.getElementById("linkpage_div").style.display = "none";
	}
	else{
		if(!live_update_support || !HTTPS_support || ("<% nvram_get("firmware_check_enable"); %>" != "1")){
			document.getElementById("update_div").style.display = "none";
			document.getElementById("beta_firmware_path_span").style.display = "none";
			document.getElementById("linkpage_div").style.display = "";
			document.getElementById("linkpage").style.display = "";
			document.getElementById("fw_check_link").style.display = "none";
			helplink = download_url;
			document.getElementById("linkpage").href = helplink;
		} 
		else{
			document.getElementById("update_div").style.display = "";
			document.getElementById("beta_firmware_path_span").style.display = "";
			document.getElementById("linkpage_div").style.display = "none";
			change_firmware_path(document.getElementById("beta_firmware_path").checked==true);
			if(confirm_show.length > 0 && confirm_show == 1){
				do_show_confirm(webs_state_info_beta, confirm_show, current_firmware_path);	//Show beta path result
			}
			else if(confirm_show.length > 0 && confirm_show == 0){
				if(amesh_support && (isSwMode("rt") || isSwMode("ap"))) {
					var interval = setInterval(function() {
						if(link_status != undefined) {
							clearInterval(interval);
							show_offline_msg(true);
						}
					}, 100);
				}
				else
					do_show_confirm(webs_state_info, confirm_show, current_firmware_path);	//Show formal path result
			}
		}
	}

	/* Viz remarked 2016.06.17		
	if(!live_update_support || !HTTPS_support || ("<% nvram_get("firmware_check_enable"); %>" != "1") || exist_firmver[0] == 9){
		document.getElementById('auto_upgrade_setting').style.display = "none";
	}
	else{
		document.firmware_form.upgrade_date_x_Sun.checked = getDateCheck(document.firmware_form.fw_schedule.value, 0);
		document.firmware_form.upgrade_date_x_Mon.checked = getDateCheck(document.firmware_form.fw_schedule.value, 1);
		document.firmware_form.upgrade_date_x_Tue.checked = getDateCheck(document.firmware_form.fw_schedule.value, 2);
		document.firmware_form.upgrade_date_x_Wed.checked = getDateCheck(document.firmware_form.fw_schedule.value, 3);
		document.firmware_form.upgrade_date_x_Thu.checked = getDateCheck(document.firmware_form.fw_schedule.value, 4);
		document.firmware_form.upgrade_date_x_Fri.checked = getDateCheck(document.firmware_form.fw_schedule.value, 5);
		document.firmware_form.upgrade_date_x_Sat.checked = getDateCheck(document.firmware_form.fw_schedule.value, 6);
		document.firmware_form.upgrade_time_x_hour.value = getfirmwareTimeRange(document.firmware_form.fw_schedule.value, 0);
		document.firmware_form.upgrade_time_x_min.value = getfirmwareTimeRange(document.firmware_form.fw_schedule.value, 1);
		document.getElementById('auto_upgrade_setting').style.display = "";
		hide_upgrade_option('<% nvram_get("fw_schedule_enable"); %>');		
	}
	*/

	if(based_modelid == "RT-AC68R"){	//MODELDEP	//id: asus_link is in string tag #FW_desc0#
		document.getElementById("asus_link").href = "http://www.asus.com/us/supportonly/RT-AC68R/";
		document.getElementById("asus_link").innerHTML = "http://www.asus.com/us/supportonly/RT-AC68R/";
	}

	if(based_modelid == "RT-AC68A"){        //MODELDEP : Spec special fine tune
		document.getElementById("fw_note2").style.display = "none";
		document.getElementById("fw_note3").style.display = "none";
		inputCtrl(document.form.file, 0);
		inputCtrl(document.form.upload, 0);
	}
	else{
		inputCtrl(document.form.file, 1);
		inputCtrl(document.form.upload, 1);
	}

	document.getElementById("fw_note3").style.display = "none";

	if(amesh_support && (isSwMode("rt") || isSwMode("ap"))) {
		$("#beta_firmware_path_span").css("display", "none");
		$("#manually_upgrade_tr").css("display", "none");
		$("#productid_tr").css("display", "none");
		document.form.file.onchange = function() {
			submitForm();
		}
	}

}

var dead = 0;
var note_display=0;	//formal path
function detect_firmware(flag){
	$.ajax({
		url: '/detect_firmware.asp',
		dataType: 'script',
		error: function(xhr){
			dead++;
			if(dead < 30)
				setTimeout("detect_firmware();", 1000);
			else{
  				document.getElementById('update_scan').style.display="none";
  				document.getElementById('update_states').innerHTML="Unable to connect to the update server.";
				document.getElementById('update').disabled = false;
			}
		},

		success: function(){
			if(cfg_sync_support){
				if(cfg_check == "0" || cfg_check == "1" || cfg_check == "5"){
					setTimeout("detect_firmware();", 1000);
				}
				else{	// got fw info
					if(cfg_check == "2"){	//1:wget fail
						document.getElementById('update_scan').style.display="none";
						if(document.start_update.firmware_path.value==1){	//Beta Firmware not available yet
							document.getElementById('update_states').innerHTML="No beta firmware available now.";	/* untranslated */
						}
						else{
							document.getElementById('update_states').innerHTML="Unable to connect to the update server.";
						}
						document.getElementById('update').disabled = false;
					}
					else if(cfg_check == "4"){	//3: FW check/RSA check fail
						document.getElementById('update_scan').style.display="none";
						document.getElementById('update_states').innerHTML="<#FIRM_fail_desc#><br><#FW_desc1#>";
						document.getElementById('update').disabled = false;

					}
					else{
						document.getElementById('update_scan').style.display="none";
						document.getElementById('update_states').innerHTML="";
						document.getElementById('update').disabled = false;
						var check_webs_state_info = webs_state_info;
						if(document.start_update.firmware_path.value==1){		//check beta path
							check_webs_state_info = webs_state_info_beta;
							note_display=1;
						}
						else{
							note_display=0;
						}

						if(amesh_support && (isSwMode("rt") || isSwMode("ap")))
							show_amas_fw_result();
						else
							do_show_confirm(check_webs_state_info, document.start_update.firmware_path.value, current_firmware_path);

					}
				}
			}
			else{
				if(webs_state_update == "0"){
					setTimeout("detect_firmware();", 1000);
				}
				else{	// got fw info
					if(webs_state_error == "1"){	//1:wget fail
						document.getElementById('update_scan').style.display="none";
						if(document.start_update.firmware_path.value==1){	//Beta Firmware not available yet
							document.getElementById('update_states').innerHTML="No beta firmware available now.";	/* untranslated */
						}
						else{
							document.getElementById('update_states').innerHTML="Unable to connect to the update server.";
						}
						document.getElementById('update').disabled = false;
					}
					else if(webs_state_error == "3"){	//3: FW check/RSA check fail
						document.getElementById('update_scan').style.display="none";
						document.getElementById('update_states').innerHTML="<#FIRM_fail_desc#><br><#FW_desc1#>";
						document.getElementById('update').disabled = false;

					}
					else{
						document.getElementById('update_scan').style.display="none";
						document.getElementById('update_states').innerHTML="";
						document.getElementById('update').disabled = false;
						var check_webs_state_info = webs_state_info;
						if(document.start_update.firmware_path.value==1){		//check beta path
							check_webs_state_info = webs_state_info_beta;
							note_display=1;
						}
						else{
							note_display=0;
						}
						var new_fw_found = do_show_confirm(check_webs_state_info, document.start_update.firmware_path.value, current_firmware_path);

						if (!new_fw_found && current_firmware_path == 1) {	// No beta? check release too
							note_display = 0;
							do_show_confirm(webs_state_info, 0, current_firmware_path);
						}
					}
				}
			}
		}
	});
}

function do_show_confirm(FWVer, CheckPath, CurrentPath){

					if(isNewFW(FWVer, CheckPath, CurrentPath)){	//check_path, current_path
						document.getElementById('update_scan').style.display="none";
						document.getElementById('update_states').style.display="none";													
						if(CheckPath==1 && FWVer.search(/alpha|beta/) != -1){	//for beta
								confirm_asus({
         					title: "Beta Firmware Available",
         					ribbon: "ribbon-red",
         					ribbon_wrapper: "ribbon-wrapper-red",
         					contentA: "There is a new beta firmware available.  These are pre-release test versions made available to obtain early user feedback, or to address issues fixed since the last official release.  Please note that beta firmware may introduce new issues or may not function as well as a stable release. Install only on devices that are not business critical.<br>",		/* untranslated */
         					contentC: "<br><#ADSL_FW_note#> Visit the download site to manually download and upgrade your router",
         					left_button: "<#CTL_Cancel#>",
         					left_button_callback: function(){confirm_cancel();},
         					left_button_args: {},
         					right_button: "Visit download site",
         					right_button_callback: function(){	
							if (webs_state_info_beta.indexOf("alpha") != -1) {
								window.open(download_url_alpha);
							} else if (webs_state_info_beta.indexOf("beta") != -1) {
								window.open(download_url_beta);
							} else {
								window.open(download_url);
							}
									},
         					right_button_args: {},
         					iframe: "get_release_note1.asp",
         					margin: "100px 0px 0px 25px",
         					note_display_flag: note_display
     						});
						}
						else{
							confirm_asus({
         					title: "New Firmware Available",
         					contentA: "There is a newer firmware available.  For security reasons it is usually recommended to update to the latest version available.  Please review the release notes below.<br>",
         					contentC: "<br><#ADSL_FW_note#> Visit the download site to manually download and upgrade your router",
         					left_button: "<#CTL_Cancel#>",
         					left_button_callback: function(){confirm_cancel();},
         					left_button_args: {},
         					right_button: "Visit download site",
							right_button_callback: function(){
										if(cfg_sync_support){
											window.open(download_url);
											//cfgsync_firmware_upgrade();
										}
										else{
											window.open(download_url);
											//document.start_update.action_mode.value="apply";
											//document.start_update.action_script.value="start_webs_upgrade";
											//document.start_update.submit();
										}
									},

         					right_button_args: {},
         					iframe: "get_release_note0.asp",
         					margin: "100px 0px 0px 25px",
         					note_display_flag: note_display
     					});
						}     		     				
					return 1;
					}
					else{
						document.getElementById('update_scan').style.display="none";
						
						if(note_display==1 && FWVer.length < 5){	//for beta path, length should be longer than 17 (e.g. 3004_380_0-g123456)
							document.getElementById('update_states').style.display="";
							document.getElementById('update_states').innerHTML="No beta firmware available now.";	/* untranslated */
						}
						else{
							document.getElementById('update_states').style.display="";
							document.getElementById('update_states').innerHTML="<#is_latest#>";
						}
						return 0;
					}

}

function cfgsync_firmware_check(){
	$.ajax({
			url: '/apply.cgi?action_mode=firmware_check',
			dataType: 'script',
			error: function(xhr) {
				setTimeout("cfgsync_firmware_check();", 1000);
			},

			success: function(response){
				setTimeout("detect_firmware();", 5000);
			}
	});
}

function cfgsync_firmware_upgrade(){
	$.ajax({
			url: '/apply.cgi?action_mode=firmware_upgrade',
			dataType: 'script',
			error: function(xhr) {
				setTimeout("cfgsync_firmware_upgrade();", 1000);
			},

			success: function(response){
				setTimeout("startDownloading();", 1000);
			}
	});
}

function detect_update(firmware_path){
	if(sw_mode != "1" || (link_status == "2" && link_auxstatus == "0") || (link_status == "2" && link_auxstatus == "2")){
		if(cfg_sync_support){
			cfgsync_firmware_check();
		}
		else{
			document.start_update.action_mode.value="apply";
			document.start_update.action_script.value="start_webs_update";
			document.start_update.submit();
		}
		document.getElementById('update_states').style.display="";
		document.getElementById('update_states').innerHTML="Contacting the update server...";
		document.getElementById('update_scan').style.display="";
		document.getElementById('update').disabled = true;
	}
	else if(dualwan_enabled &&
				((first_link_status == "2" && first_link_auxstatus == "0") || (first_link_status == "2" && first_link_auxstatus == "2")) ||
				((secondary_link_status == "2" && secondary_link_auxstatus == "0") || (secondary_link_status == "2" && secondary_link_auxstatus == "2"))){
		document.start_update.action_mode.value="apply";
		document.start_update.action_script.value="start_webs_update";
		document.getElementById('update_states').style.display="";
		document.getElementById('update_states').innerHTML="Contacting the update server...";
		document.getElementById('update_scan').style.display="";
		document.getElementById('update').disabled = true;
		document.start_update.submit();
	}		
	else{
		document.getElementById('update_scan').style.display="none";
		document.getElementById('update_states').style.display="";
		document.getElementById('update_states').innerHTML="Unable to connect to the update server.";
		return false;	
	}
}

var dead = 0;
function detect_httpd(){
	$.ajax({
		url: '/httpd_check.xml',
		dataType: 'xml',
		timeout: 1500,
		error: function(xhr){
			if(dead > 5){
				document.getElementById('loading_block1').style.display = "none";
				document.getElementById('loading_block2').style.display = "none";
				document.getElementById('loading_block3').style.display = "";
				document.getElementById('loading_block3').innerHTML = "<div><#FIRM_reboot_manually#></div>";
			}
			else{
				dead++;
			}

			setTimeout("detect_httpd();", 1000);
		},

		success: function(){
			location.href = '<% abs_index_page(); %>';
		}
	});
}

var rebooting = 0;
function isDownloading(){
	$.ajax({
    		url: '/ajax_fwdl_percent.asp',
    		dataType: 'script',
				timeout: 1500,
    		error: function(xhr){
					
					rebooting++;
					if(rebooting < 30){
							setTimeout("isDownloading();", 1000);
					}
					else{							
							document.getElementById("drword").innerHTML = "<#connect_failed#>";
							return false;
					}

    		},
    		success: function(){
				if(cfg_sync_support){
					if(cfg_upgrade == "1"){
						document.getElementById("drword").innerHTML = "&nbsp;&nbsp;&nbsp;<#fw_downloading#>...";
						setTimeout("isDownloading();", 1000);
					}
					else{
						if(cfg_upgrade == "2"){
							document.getElementById("drword").innerHTML = "<#connect_failed#>";
							return false;
						}
						else if(cfg_upgrade == "3"){
							document.getElementById("drword").innerHTML = "<#FIRM_fail_desc#><br><#FW_desc1#>";
							return false;
						}
						else{		// start upgrading
							document.getElementById("hiddenMask").style.visibility = "hidden";
							showLoadingBar(270);
							setTimeout("detect_httpd();", 272000);
							return false;
						}
					}
				}
				else{
					if(webs_state_upgrade == 0){
						document.getElementById("drword").innerHTML = "&nbsp;&nbsp;&nbsp;<#fw_downloading#>..."+fwdl_percent;
						setTimeout("isDownloading();", 1000);
					}
					else{ 	// webs_upgrade.sh is done
						
						if(webs_state_error == 1){
							document.getElementById("drword").innerHTML = "<#connect_failed#>";
							return false;
						}
						else if(webs_state_error == 2){
							document.getElementById("drword").innerHTML = "Memory space is NOT enough to upgrade on internet. Please wait for rebooting.<br><#FW_desc1#>";	/* untranslated */ //Untranslated.	fw_size_higher_mem
							return false;
						}
						else if(webs_state_error == 3){
							document.getElementById("drword").innerHTML = "<#FIRM_fail_desc#><br><#FW_desc1#>";
							return false;
						}
						else{		// start upgrading
							document.getElementById("hiddenMask").style.visibility = "hidden";
							showLoadingBar(270);
							setTimeout("detect_httpd();", 272000);
							return false;
						}

					}
				}
  			}
  		});
}

function startDownloading(){
	disableCheckChangedStatus();			
	dr_advise();
	document.getElementById("drword").innerHTML = "&nbsp;&nbsp;&nbsp;<#fw_downloading#>...";
	isDownloading();
}

function check_zip(obj){
	var reg = new RegExp("^.*.(zip|ZIP|rar|RAR|7z|7Z)$", "gi");
	if(reg.test(obj.value)){
			alert("<#FW_note_unzip#>");
			obj.focus();
			obj.select();
			return false;
	}
	else
			return true;		
}

function submitForm(){
	if(!check_zip(document.form.file))
			return;
	else {
		var status = onSubmitCtrlOnly(document.form.upload, 'Upload1');
		if(amesh_support && status && (isSwMode("rt") || isSwMode("ap"))) {
			if(interval_update_AiMesh_fw_status) {
				clearInterval(interval_update_AiMesh_fw_status);
				interval_update_AiMesh_fw_status = false;
			}
		}
	}
}

function sig_version_check(){
	document.getElementById("sig_check").disabled = true;
	$("#sig_status").show();
	document.sig_update.submit();
	$("#sig_status").html("Signature checking ...");	/* Untranslated */
	document.getElementById("sig_update_scan").style.display = "";
	setTimeout("sig_check_status();", 8000);
}

var sdead=0;
function sig_check_status(){
	$.ajax({
    	url: '/detect_firmware.asp',
    	dataType: 'script',
    	timeout: 3000,
    	error:	function(xhr){
			sdead++;
			if(sdead < 20){				
				setTimeout("sig_check_status();", 1000);
			}
			else{
				return;
			}
    		},
    	success: function(){
			$("#sig_status").show();
			if(sig_state_flag == 0){		// no need upgrade
				$("#sig_status").html("Signature is up to date");	/* Untranslated */
				document.getElementById("sig_update_scan").style.display = "none";
				document.getElementById("sig_check").disabled = false;
			}
			else if(sig_state_flag == 1){
				if(sig_state_error != 0){		// update error
					$("#sig_status").html("Signature update failed");	/* Untranslated */
					document.getElementById("sig_update_scan").style.display = "none";
					document.getElementById("sig_check").disabled = false;
				}
				else{
					if(sig_state_upgrade == 1){		//update complete						
						update_sig_ver();
						document.getElementById("sig_check").disabled = false;
					}
					else{		//updating
						$("#sig_status").html("Signature is updating");	/* Untranslated */
						setTimeout("sig_check_status();", 1000);
					}				
				}			
			}
  		}
  	});
}

function update_sig_ver(){
	$.ajax({
    	url: '/detect_firmware.asp',
    	dataType: 'script',
		timeout: 3000,
    	error:	function(xhr){
    		setTimeout('update_sig_ver();', 1000);
    	},
    	success: function(){
    		if(sig_ver_ori == sig_ver){    			
    			setTimeout('update_sig_ver();', 1000);
    		}	
    		else{
    			document.getElementById("sig_update_date").innerHTML = "";
    			document.getElementById("sig_update_scan").style.display = "none";
    			$("#sig_status").html("Signature update completely");	/* Untranslated */
    			$("#sig_ver_word").html(sig_ver);
    		}			
  		}
  	});
}

function check_Timefield_checkbox(){	// To check Date checkbox checked or not and control Time field disabled or not
	if( document.firmware_form.upgrade_date_x_Sun.checked == true 
		|| document.firmware_form.upgrade_date_x_Mon.checked == true 
		|| document.firmware_form.upgrade_date_x_Tue.checked == true
		|| document.firmware_form.upgrade_date_x_Wed.checked == true
		|| document.firmware_form.upgrade_date_x_Thu.checked == true
		|| document.firmware_form.upgrade_date_x_Fri.checked == true	
		|| document.firmware_form.upgrade_date_x_Sat.checked == true	){
			inputCtrl(document.firmware_form.upgrade_time_x_hour,1);
			inputCtrl(document.firmware_form.upgrade_time_x_min,1);
	}
	else{
			inputCtrl(document.firmware_form.upgrade_time_x_hour,0);
			inputCtrl(document.firmware_form.upgrade_time_x_min,0);
	}		
}

function hide_upgrade_option(flag){
	document.firmware_form.upgrade_date_x_Sun.disabled = (flag == 1) ? false : true;
	document.firmware_form.upgrade_date_x_Mon.disabled = (flag == 1) ? false : true;
	document.firmware_form.upgrade_date_x_Tue.disabled = (flag == 1) ? false : true;
	document.firmware_form.upgrade_date_x_Wed.disabled = (flag == 1) ? false : true;
	document.firmware_form.upgrade_date_x_Thu.disabled = (flag == 1) ? false : true;
	document.firmware_form.upgrade_date_x_Fri.disabled = (flag == 1) ? false : true;
	document.firmware_form.upgrade_date_x_Sat.disabled = (flag == 1) ? false : true;
	
	(flag == 1) ? inputCtrl(document.firmware_form.upgrade_time_x_hour,1) : inputCtrl(document.firmware_form.upgrade_time_x_hour,0);
	(flag == 1) ? inputCtrl(document.firmware_form.upgrade_time_x_min,1) : inputCtrl(document.firmware_form.upgrade_time_x_min,0);
	
	if(flag==1)
		check_Timefield_checkbox();
}

function getfirmwareTimeRange(str, pos)
{
	if (pos == 0)
		return str.substring(7,9);
	else if (pos == 1)
		return str.substring(9,11);
}

function setfirmwareTimeRange(rd, rh, rm)
{
	return(rd.value+rh.value+rm.value);
}

function updateDateTime()
{	
	if(document.firmware_form.fw_schedule_enable.value == "1"){
		document.firmware_form.fw_schedule.value = setDateCheck(
					document.firmware_form.upgrade_date_x_Sun,
					document.firmware_form.upgrade_date_x_Mon,
					document.firmware_form.upgrade_date_x_Tue,
					document.firmware_form.upgrade_date_x_Wed,
					document.firmware_form.upgrade_date_x_Thu,
					document.firmware_form.upgrade_date_x_Fri,
					document.firmware_form.upgrade_date_x_Sat);
		document.firmware_form.fw_schedule.value = setfirmwareTimeRange(
					document.firmware_form.fw_schedule,
					document.firmware_form.upgrade_time_x_hour,
					document.firmware_form.upgrade_time_x_min);
	}	
}

function applyRule(){
	
	if(validForm()){
		if(live_update_support){
			updateDateTime();	
		}	
		
		showLoading();
		document.firmware_form.submit();
	}
}

function validForm(){
	
	if(live_update_support){
		if(!document.firmware_form.upgrade_date_x_Sun.checked && !document.firmware_form.upgrade_date_x_Mon.checked &&
			!document.firmware_form.upgrade_date_x_Tue.checked && !document.firmware_form.upgrade_date_x_Wed.checked &&
			!document.firmware_form.upgrade_date_x_Thu.checked && !document.firmware_form.upgrade_date_x_Fri.checked &&
			!document.firmware_form.upgrade_date_x_Sat.checked && document.firmware_form.fw_schedule_enable.value == "1")
			{
				alert(Untranslated.filter_lw_date_valid);
				document.firmware_form.upgrade_date_x_Sun.focus();
				return false;
		}
		else
				return true;
	}
	else
			return true;
}

function change_firmware_path(flag){	
	if(flag)
		document.start_update.firmware_path.value = 1;	//beta path
	else
		document.start_update.firmware_path.value = 0;	//stable path		
}

function transferTimeFormat(time){
	if(time == 0){
		return "";
	}

	var t = new Date();
	t.setTime(time);
	var year = t.getFullYear();
	var month = t.getMonth() + 1;
	if(month < 10){
		month  = "0" + month;
	}
	
	var date = t.getDate();
	if(date < 10){
		date = "0" + date;
	}
	
	var hour = t.getHours();
	if(hour < 10){
		hour = "0" + hour;
	}
			
	var minute = t.getMinutes();
	if(minute < 10){
		minute = "0" + minute;
	}

	var date_format = "<#FW_updated#> : " + year + "/" + month + "/" + date + " " + hour + ":" + minute;
	return date_format;
}
function show_offline_msg(_checkFlag) {
	if(!amesh_offline_flag && _checkFlag) {
		$("#amas_update").css("display", "none");
		$(".checkFWReuslt").empty();
		$(".checkFWReuslt").removeClass("aimesh_fw_release_note");
		detect_update(document.start_update.firmware_path.value);
		return;
	}

	var $offlineHtml = $('<div>');
	$offlineHtml.attr({"id" : "amesh_offline_msg"});
	$offlineHtml.addClass("amesh_popup_bg");
	$offlineHtml.css("display", "none");
	$offlineHtml.attr({"onselectstart" : "return false"});
	$offlineHtml.appendTo($('body'));

	var $amesh_hint_offline = $('<div>');
	$amesh_hint_offline.addClass("amesh_hint_text");
	$amesh_hint_offline.css("color", "#FC0");
	$amesh_hint_offline.html("There are offline AiMesh node in your AiMesh system and offline AiMesh node will not able to be updated. (Online AiMesh node could still be updated.)");/*untranslated*/
	$offlineHtml.append($amesh_hint_offline);

	var $amesh_hint_text = $('<div>');
	$amesh_hint_text.addClass("amesh_hint_text");
	$amesh_hint_text.html("Offline tips :");/*untranslated*/
	$offlineHtml.append($amesh_hint_text);

	var $amesh_hint_content = $('<div>');
	$amesh_hint_content.addClass("amesh_hint_text");
	$amesh_hint_content.css("margin-left", "auto");
	$offlineHtml.append($amesh_hint_content);

	var $msg_item =  $('<ol>');
	var msg_text = "<li>Make sure your AiMesh node is power on.</li>";/*untranslated*/
	msg_text += "<li>Reboot this AiMesh node and try again.</li>";/*untranslated*/
	msg_text += "<li>If you are using Wi-Fi connection, please try to find a place closer to other AiMesh node.</li>";/*untranslated*/
	msg_text += "<li>If you are using wired connection, please make sure cable are installed properly.</li>";/*untranslated*/
	msg_text += "<li>If still no help, please try to reset this AiMesh node by \"Reset button\" and try to add again.</li>";/*untranslated*/
	$msg_item.html(msg_text);
	$amesh_hint_content.append($msg_item);

	var $amesh_action_bg = $('<div>');
	$amesh_action_bg.addClass("amesh_action_bg");
	$offlineHtml.append($amesh_action_bg);

	var $amesh_ok = $('<input/>');
	$amesh_ok.addClass("button_gen");
	$amesh_ok.attr({"type" : "button", "value" : "<#CTL_ok#>"});
	$amesh_action_bg.append($amesh_ok);
	$amesh_ok.click(
		function() {
			if($('.amesh_popup_bg').length == 1) {
				$('.amesh_popup_bg').remove();
			}
			if(_checkFlag) {
				detect_update(document.start_update.firmware_path.value);
			}
		}
	);

	$("#amesh_offline_msg").fadeIn(300);
	cal_panel_block("amesh_offline_msg", 0.25);
	adjust_panel_block_top("amesh_offline_msg", 200);
}
function show_amas_fw_result() {
	$.ajax({
		url: '/ajax_onboarding.asp',
		dataType: 'script',
		error: function(xhr) {
			amas_fw_detect();
		},
		success: function() {
			document.getElementById('update_states').style.display = "none";
			document.getElementById('update_states').innerHTML = "";
			document.getElementById('update_scan').style.display = "none";
			for (var idx in get_cfg_clientlist) {
				if(get_cfg_clientlist.hasOwnProperty(idx)) {
					var model_name = get_cfg_clientlist[idx].model_name;
					var mac = get_cfg_clientlist[idx].mac;
					var newfwver = get_cfg_clientlist[idx].newfwver;
					var mac_id = mac.replace(/:/g, "");
					var ck_fw_result = "<#is_latest#>";
					var online = get_cfg_clientlist[idx].online;
					$("#amas_" + mac_id + "").children().find(".checkFWReuslt").html(ck_fw_result);
					if(newfwver != "") {
						ck_fw_result = newfwver;
						$("#amas_" + mac_id + "").children().find(".checkFWReuslt").addClass("aimesh_fw_release_note");
						$("#amas_" + mac_id + "").children().find(".checkFWReuslt").html(ck_fw_result);
						$("#amas_update").css("display", "");
						$("#amas_" + mac_id + "").children().find(".checkFWReuslt").click({"model_name": model_name, "newfwver": newfwver}, show_fw_relese_note);
					}
					if(online == "1")
						$("#amas_" + mac_id + "").children("#checkNewFW").css("display", "");
				}
			}
		}
	});
}
function show_fw_relese_note(event) {
	if($(".confirm_block").length > 0)
		$(".confirm_block").remove();

	document.amas_release_note.model.value = event.data.model_name;
	document.amas_release_note.version.value = event.data.newfwver;
	document.amas_release_note.submit();
	confirm_asus({
		title: "New Firmware Available",
		contentA: "<#exist_new#><br>",
		contentC: "<br><#ADSL_FW_note#> <#Main_alert_proceeding_desc5#>",
		left_button: "",
		left_button_callback: {},
		left_button_args: {},
		right_button: "<#CTL_close#>",
		right_button_callback: function(){confirm_cancel();},
		right_button_args: {},
		iframe: "get_release_note_amas.asp",
		margin: "100px 0px 0px 25px",
		note_display_flag: note_display
	});

	setTimeout(function(){check_fw_relese_note_status();}, 2000);
}
function check_fw_relese_note_status() {
	if($(".confirm_block").length > 0) {
		$.ajax({
			url: '/ajax_onboarding.asp',
			dataType: 'script',
			success: function() {
				switch(cfg_note) {
					case "0" :
						check_fw_relese_note_status();
						break;
					case "1" :
						show_fw_relese_note_result(true);
						break;
					case "2" :
					case "3" :
						show_fw_relese_note_result(false);
						break;
				}
			}
		});
	}
}
function show_fw_relese_note_result(_status) {
	if($(".confirm_block").children().find("#status_iframe").contents().find("#amas_relese_note").length == 0)
		show_fw_relese_note_result(_status);

	if(_status) {
		$(".confirm_block").children().find("#status_iframe").attr("src", "get_release_note_amas.asp");//reload
		$(".confirm_block").children().find("#status_iframe").load(function() {
			$(".confirm_block").children().find("#status_iframe").contents().find("#amas_relese_note").css("display", "");
			$(".confirm_block").children().find("#status_iframe").contents().find("#amas_relese_note_hint").css("display", "none");
		});
	}
	else
		$(".confirm_block").children().find("#status_iframe").contents().find("#amas_relese_note_hint").val("Fail to grab release note");/* untranslated */
}
function open_AiMesh_node_fw_upgrade(_ip) {
	var url = "http://" + _ip + "/AiMesh_Node_FirmwareUpgrade.asp";
	var window_width = 550;
	var window_height = 450;
	var window_top = screen.availHeight / 2 - window_height / 2;
	var window_left = screen.availWidth / 2 - window_width / 2;
	window.open(url, '_new' ,'width=' + window_width + ',height=' + window_height + ', top=' + window_top + ',left=' + window_left + ',menubar=no,scrollbars=no,toolbar=no,resizable=no,status=no,location=no');
}
function open_AiMesh_router_fw_upgrade() {
	document.form.file.click();
}
function update_AiMesh_fw() {
	$.ajax({
		url: '/ajax_onboarding.asp',
		dataType: 'script',
		success: function(){
			for (var idx in get_cfg_clientlist) {
				if(get_cfg_clientlist.hasOwnProperty(idx)) {
					var mac = get_cfg_clientlist[idx].mac;
					var fwver = get_cfg_clientlist[idx].fwver;
					var mac_id = mac.replace(/:/g, "");
					$("#amas_" + mac_id + "").children("#current_version").html("Current Version : " + fwver + "");/* untranslated */
				}
			}
		}
	});
}
</script>
</head>
<body onload="initial();">

<div id="TopBanner"></div>

<div id="LoadingBar" class="popup_bar_bg">
<table cellpadding="5" cellspacing="0" id="loadingBarBlock" class="loadingBarBlock" align="center">
	<tr>
		<td height="80">
		<div id="loading_block1" class="Bar_container">
			<span id="proceeding_img_text"></span>
			<div id="proceeding_img"></div>
		</div>
		<div id="loading_block2" style="margin:5px auto; width:85%;"><#FIRM_ok_desc#><br><#Main_alert_proceeding_desc5#></div>
		<div id="loading_block3" style="margin:5px auto;width:85%; font-size:12pt;"></div>
		</td>
	</tr>
</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>
<div id="Loading" class="popup_bg"></div><!--for uniform show, useless but have exist-->

<div id="hiddenMask" class="popup_bg">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center" style="height:100px;">
		<tr>
		<td>
			<div class="drword" id="drword" style="">&nbsp;&nbsp;&nbsp;&nbsp;<#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...</div>
		</td>
		</tr>
	</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" action="upgrade.cgi" name="form" target="hidden_frame" enctype="multipart/form-data">
<input type="hidden" name="current_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">

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
		<td align="left" valign="top" >

		<table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
		<tbody>
		<tr>
		  <td bgcolor="#4D595D" valign="top"  >
		  <div>&nbsp;</div>
		  <div class="formfonttitle"><#menu5_6#> - <#menu5_6_3#></div>
		  <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
		  <div class="formfontdesc"><strong><#FW_note#></strong>
				<ol>
					<li><#FW_n0#></li>
					<li><#FW_n1#></li>
					<li id="fw_note2"><#FW_n2#></li>
					<li id="fw_note3"><#FW_desc0#></li>
				</ol>
		  </div>
		  <br>

		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
			<thead>
				<tr>
					<td colspan="2"><#FW_item2#></td>	
				</tr>	
			</thead>	
			<tr id="productid_tr">
				<th><#FW_item1#></th>
				<td><#Web_Title2#></td>
			</tr>
<!--###HTML_PREP_START###-->
<!--###HTML_PREP_ELSE###-->
<!--
[DSL-N55U][DSL-N55U-B]
{ADSL firmware version}
			<tr>
				<th><#adsl_fw_ver_itemname#></th>
				<td><input type="text" class="input_15_table" value="<% nvram_dump("adsl/tc_fw_ver_short.txt",""); %>" readonly="1" autocorrect="off" autocapitalize="off"></td>
			</tr>
			<tr>
				<th>RAS</th>
				<td><input type="text" class="input_20_table" value="<% nvram_dump("adsl/tc_ras_ver.txt",""); %>" readonly="1" autocorrect="off" autocapitalize="off"></td>
			</tr>
[DSL-AC68U]
                        <tr>
                                <th>DSL <#FW_item2#></th>
                                <td><% nvram_get("dsllog_fwver"); %></td>
                        </tr>
                        <tr>
                                <th><#adsl_fw_ver_itemname#></th>
                                <td><% nvram_get("dsllog_drvver"); %></td>
                        </tr>
-->

<!--###HTML_PREP_END###-->
			<tr id="sig_ver_field" style="display:none">
				<th><#sig_ver#></th>
				<td >
					<div style="height:33px;margin-top:5px;"><span id="sig_ver_word" style="color:#FFFFFF;"></span><span id="sig_update_date"></span></div>
					<div style="margin-left:200px;margin-top:-38px;">
						<input type="button" id="sig_check" name="sig_check" class="button_gen" onclick="sig_version_check();" value="<#liveupdate#>">
					</div>
					<div>
						<span id="sig_status" style="display:none"></span>
						<img id="sig_update_scan" style="display:none;" src="images/InternetScan.gif">
					</div>
				</td>
			</tr>

			<tr id="fw_check_link">
				<th>Scheduled new firmware check</th>
				<td>Click <a href="Tools_OtherSettings.asp#fwcheck" style="text-decoration:underline;color:#FFCC00;">here</a> to toggle new firmware notification.</td>
			</tr>
			<tr id="fw_version_tr">
				<th><#FW_item2#></th>
				<td>
					<div id="FWString" style="height:33px;margin-top:5px;"></div>
					<div id="update_div" style="margin-left:200px;margin-top:-38px;display:none;">
						<input type="button" id="update" name="update" class="button_gen" onclick="detect_update(document.start_update.firmware_path.value);" value="<#liveupdate#>" />
						<span id="beta_firmware_path_span" style="display:none;">
							<input type="checkbox" name="beta_firmware_path" id="beta_firmware_path" onclick="change_firmware_path(this.checked==true);"  <% nvram_match("firmware_path", "1", "checked"); %>><#get_beta#></input>
						</span>
					</div>
					<div id="linkpage_div" class="button_helplink" style="margin-left:200px;margin-top:-38px;display:none;">
						<a id="linkpage" target="_blank"><div style="padding-top:5px;"><#liveupdate#></div></a>
					</div>
					<div id="check_states">
						<span id="update_states"></span>
						<img id="update_scan" style="display:none;" src="images/InternetScan.gif" />
					</div>
				</td>
			</tr>
			<tr id="manually_upgrade_tr">
				<th><#FW_item5#></th>
				<td>
					<input type="file" name="file" class="input" style="color:#FFCC00;*color:#000;width: 194px;">
					<input type="button" name="upload" class="button_gen" onclick="submitForm()" value="<#CTL_upload#>" />
				</td>
			</tr>			
		</table>
		<div class="aimesh_manual_fw_update_hint" style="display:none;">
			Note : Manual firmware update will update this AiMesh router / node only, if you are using AiMesh system, please make sure you are uploading proper firmware version.<!-- untranslated -->
		</div>
		
</form>
		
<form method="post" name="firmware_form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="next_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="flag" value="">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_time">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="fw_schedule_enable" value="<% nvram_get("fw_schedule_enable"); %>">
<input type="hidden" name="fw_schedule" value="<% nvram_get("fw_schedule"); %>">
		<table id="auto_upgrade_setting" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" style="display:none;">
			<thead>
			<tr>
				<td colspan="2">Auto Upgrade Setting</td>	<!-- untranslated -->	
			</tr>	
			</thead>
			<tr>
				<th>Instal Newer Ofiicial Firmware Automatically</th>		<!-- untranslated -->
				<td>
					<div class="left" style="width:94px; float:left; cursor:pointer;" id="radio_firmware_schedule_enable"></div>
					<script type="text/javascript">
						$('#radio_firmware_schedule_enable').iphoneSwitch('<% nvram_get("fw_schedule_enable"); %>', 
						function() {
							document.firmware_form.fw_schedule_enable.value = 1;
							hide_upgrade_option(1);
						},
						function() {
							document.firmware_form.fw_schedule_enable.value = 0;
							hide_upgrade_option(0);
						}
						);
					</script>		
				</td>	
			</tr>
			<tr>
				<th>Day To Install</th>	<!-- untranslated -->
				<td>
					<input type="checkbox" name="upgrade_date_x_Sun" class="input" onclick="check_Timefield_checkbox();"><#date_Sun_itemdesc#>
					<input type="checkbox" name="upgrade_date_x_Mon" class="input" onclick="check_Timefield_checkbox();"><#date_Mon_itemdesc#>
					<input type="checkbox" name="upgrade_date_x_Tue" class="input" onclick="check_Timefield_checkbox();"><#date_Tue_itemdesc#>
					<input type="checkbox" name="upgrade_date_x_Wed" class="input" onclick="check_Timefield_checkbox();"><#date_Wed_itemdesc#>
					<input type="checkbox" name="upgrade_date_x_Thu" class="input" onclick="check_Timefield_checkbox();"><#date_Thu_itemdesc#>
					<input type="checkbox" name="upgrade_date_x_Fri" class="input" onclick="check_Timefield_checkbox();"><#date_Fri_itemdesc#>
					<input type="checkbox" name="upgrade_date_x_Sat" class="input" onclick="check_Timefield_checkbox();"><#date_Sat_itemdesc#>
				</td>	
			</tr>
			<tr>
				<th>Time To Install</th>	<!-- untranslated -->
				<td>
					<input type="text" maxlength="2" class="input_3_table" name="upgrade_time_x_hour" onKeyPress="return validator.isNumber(this,event);" onblur="validator.timeRange(this, 0);" autocorrect="off" autocapitalize="off"> : 
					<input type="text" maxlength="2" class="input_3_table" name="upgrade_time_x_min" onKeyPress="return validator.isNumber(this,event);" onblur="validator.timeRange(this, 1);" autocorrect="off" autocapitalize="off">
				</td>	
			</tr>
			<tr align="center">	
				<td colspan="2" >
					<input type="button" name="apply" class="button_gen" onclick="applyRule()" value="<#CTL_apply#>" />
				</td>	
			</tr>
		</table>
		
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
</form>

<form method="post" name="start_update" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="next_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="flag" value="liveUpdate">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="firmware_path" value="0">
</form>
<form method="post" name="sig_update" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="next_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="start_sig_check">
<input type="hidden" name="action_wait" value="">
</form>
<form method="post" name="amas_release_note" action="/applyapp.cgi" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="next_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="action_mode" value="release_note">
<input type="hidden" name="model" value="">
<input type="hidden" name="version" value="">
</form>
</body>
</html>
