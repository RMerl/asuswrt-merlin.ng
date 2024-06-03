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
<link rel="stylesheet" type="text/css" href="css/confirm_block.css">
<link rel="stylesheet" type="text/css" href="device-map/device-map.css">
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
	background: #D7E8F4;
	background: -webkit-linear-gradient(#F2F7EB 0%, #E4E6EE 49%, #C8D3Db 50%, #BACEDA 51%, #D7E8F4 100%);
	background: -o-linear-gradient(#F2F7EB 0%, #E4E6EE 49%, #C8D3Db 50%, #BACEDA 51%, #D7E8F4 100%);
	background: linear-gradient(#F2F7EB 0%, #E4E6EE 49%, #C8D3Db 50%, #BACEDA 51%, #D7E8F4 100%);
}
.button_helplink{
	font-weight: bolder;
	text-shadow: 1px 1px 0px black;
	text-align: center;
	vertical-align: middle;
	background: #121C1E;
	background: -webkit-linear-gradient(#233438 0%, #0F1011 100%);
	background: -o-linear-gradient(#233438 0%, #0F1011 100%);
	background: linear-gradient(#233438 0%, #0F1011 100%);
	-webkit-border-radius: 8px;
	-moz-border-radius: 8px;
	border-radius: 8px;
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
	background: #085F96;
	background: -webkit-linear-gradient(#09639C 0%, #003047 100%);
	background: -o-linear-gradient(#09639C 0%, #003047 100%);
	background: linear-gradient(#09639C 0%, #003047 100%);
	height:33px;
 	width:122px;
	cursor:pointer;
	outline: none; /* for Firefox */
 	hlbr:expression(this.onFocus=this.blur()); /* for IE */
}
.current_fw_release_note {
		color: #FC0;
        text-decoration: underline;
        cursor: pointer;
}

#rbk_Block{
	font-family: Arial, Helvetica, MS UI Gothic, MS P Gothic, Microsoft Yahei UI, sans-serif;
	width:800px;
	margin-top: -945px;
}
.rbk_title{
	margin-top:15px;
	margin-left:15px;
	float:left;
	font-size:18px;
	color:#93A9B1;
}
.rbk_close{
	float:right;
}
.td_rbk_fwver{
	text-decoration: underline;
	cursor: pointer;	
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
<script language="JavaScript" type="text/javascript" src="/js/httpApi.js"></script>
<script language="JavaScript" type="text/javascript" src="/replaceisp.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/asus_eula.js"></script>
<!-- script language="JavaScript" type="text/javascript" src="/ajax/get_rbk_info.asp"></script -->
<script>
$(function () {
	if(amesh_support && (isSwMode("rt") || isSwMode("ap")) && ameshRouter_support) {
		addNewCSS('/device-map/amesh.css');
	}
});
var webs_state_update = '<% nvram_get("webs_state_update"); %>';
var webs_state_upgrade = '<% nvram_get("webs_state_upgrade"); %>';
var webs_state_error = '<% nvram_get("webs_state_error"); %>';
var webs_state_info = '<% nvram_get("webs_state_info"); %>';
var webs_state_REQinfo = '<% nvram_get("webs_state_REQinfo"); %>';
var webs_state_flag = '<% nvram_get("webs_state_flag"); %>';

var confirm_show = '<% get_parameter("confirm_show"); %>';
var nt_flag = '<% get_parameter("flag"); %>';
var webs_release_note= "";

var fwdl_percent="";
var varload = 0;
var helplink = "";
helplink = get_helplink();
var Downloadlink = "";
Downloadlink = get_Downloadlink();
var faq_href1 = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=131";
var faq_href2 = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=107";

var is_ISP_incompatible = (in_territory_code("CX/01") || in_territory_code("CX/02") || in_territory_code("CX/03") || in_territory_code("CX/05") || in_territory_code("CX/06")
						|| in_territory_code("CT/01") || in_territory_code("CT/02") || in_territory_code("CT/03") || in_territory_code("CT/04") || in_territory_code("CT/05")
						|| in_territory_code("CH/01"));

var dpi_engine_status = <%bwdpi_engine_status();%>;
var sig_ver = '<% nvram_get("bwdpi_sig_ver"); %>';
var sig_ver_ori = '<% nvram_get("bwdpi_sig_ver"); %>';
var sig_update_t = '<% nvram_get("sig_update_t"); %>';
if(cfg_sync_support){
	var cfg_check = '<% nvram_get("cfg_check"); %>';
	var cfg_upgrade = '<% nvram_get("cfg_upgrade"); %>';
}
download_url_redir = "https://fwupdate.asuswrt-merlin.net/" + based_modelid;
download_url = "https://www.asuswrt-merlin.net/download";

/* Disable these for Asuswrt-Merlin */
afwupg_support=false;
betaupg_support=false;
revertfw_support=false;
rbkfw_support=false;

if(pipefw_support || urlfw_support){
	var hndwr_status = '<% nvram_get("hndwr"); %>';
}

var webs_update_enable_orig = httpApi.nvramGet(["webs_update_enable"],1).webs_update_enable;
var webs_update_time_orig = httpApi.nvramGet(["webs_update_time"],1).webs_update_time;

var update_time_hour_orig = webs_update_time_orig.split(":")[0].replace(/^0/, '');
update_time_hour_orig = (update_time_hour_orig=="")? "2":update_time_hour_orig;

var update_time_min_orig = (webs_update_time_orig.split(":").length==2)? webs_update_time_orig.split(":")[1].replace(/^0/, ''):"";
update_time_min_orig = (update_time_min_orig=="")? "0":update_time_min_orig;

var amesh_offline_flag = false;
var interval_update_AiMesh_fw_status;

var firmver = '<% nvram_get("firmver"); %>';
var buildno = '<% nvram_get("buildno"); %>';
var extendno = '<% nvram_get("extendno"); %>';
var FWString = '';
FWString = firmver.replace(/\./g,"") + "." + buildno;
if ((extendno != "") && (extendno != "0"))
	FWString += "_"+extendno;

if(gobi_support){
	var mobile_upgrade_md5 = "";
	var mobile_upgrade_name = "";
	var mobile_upgrade_process = "";
	var mobile_upgrade_now = "";
	var mobile_upgrade_status = "";
}

var dl_path="https://dlcdnets.asus.com/pub/ASUS/";
var dl_beta="LiveUpdate/Release/Wireless_SQ/";
var dl_file="wireless/ASUSWRT/";
var frsmodel =  (httpApi.nvramGet(["webs_state_odm"]).webs_state_odm != "")? httpApi.nvramGet(["webs_state_odm"]).webs_state_odm:httpApi.nvramGet(["productid"]).productid;
/*
var firmver_org =  httpApi.nvramGet(["firmver_org"]).firmver_org;
var buildno_org =  httpApi.nvramGet(["buildno_org"]).buildno_org;
var extendno_org =  httpApi.nvramGet(["extendno_org"]).extendno_org;

var revert_link='';
var RevertFWver = '';
var RevertFWString = '';
var isSame_org = false;
var isUnderREQ = false;
if(firmver_org!="" && buildno_org!="" && extendno_org!=""){
	RevertFWver = firmver_org+"."+buildno_org+"_"+extendno_org;
	isSame_org = (RevertFWver==FWString)? true:false;
	isUnderREQ = checkUnderREQ();
	RevertFWString = firmver_org.replace(/\./g,'')+"_"+buildno_org+"_"+extendno_org;
	RevertFWString = frsmodel+"_"+RevertFWString+"_rsa.zip";
	if(firmver_org.substring(0,1)=="9"){
		revert_link = dl_path+dl_beta+RevertFWString;
	}
	else{
		revert_link = dl_path+dl_file+RevertFWString;
	}
}

if(support_site_modelid == "GT-AC2900_SH"){
	afwupg_support=false;
	betaupg_support=false;
	revertfw_support=false;
}

var rbk_count = 0;
var _rollback_info = 0;
if(rbkfw_support){
	_rollback_info = JSON.parse('<% get_rbkList(); %>');
	rbk_count = Object.keys(_rollback_info).length;
}

function checkUnderREQ(){
	if(webs_state_REQinfo==""){
		return false;
	}
	else{
		var REQ_firmver=webs_state_REQinfo.split("_")[0];
		var REQ_buildno=webs_state_REQinfo.split("_")[1];
		var REQ_extendno=webs_state_REQinfo.split("_")[2];
		//alert("REQ: "+REQ_firmver+"_"+REQ_buildno+"_"+REQ_extendno+" | org: "+firmver_org+"_"+buildno_org+"_"+extendno_org);
		if((buildno_org < REQ_buildno) || 
			(buildno_org == REQ_buildno && firmver_org.replace(/\./g,'') < REQ_firmver) || 
			(buildno_org == REQ_buildno && firmver_org.replace(/\./g,'') == REQ_firmver && extendno_org < REQ_extendno)
		){
			return true;
		}
		else{
			return false;		
		}
	}
}
*/


function showclock(){
	JS_timeObj.setTime(systime_millsec);
	systime_millsec += 1000;
	JS_timeObj2 = JS_timeObj.toString();
	JS_timeObj2 = JS_timeObj2.substring(0,3) + ", " +
	              JS_timeObj2.substring(4,10) + "  " +
				  checkTime(JS_timeObj.getHours()) + ":" +
				  checkTime(JS_timeObj.getMinutes()) + ":" +
				  checkTime(JS_timeObj.getSeconds()) + "  " +
				  /*JS_timeObj.getFullYear() + " GMT" +
				  timezone;*/ // Viz remove GMT timezone 2011.08
				  JS_timeObj.getFullYear();
	document.getElementById("system_time").innerHTML = JS_timeObj2;
	setTimeout("showclock()", 1000);
	if(navigator.appName.indexOf("Microsoft") >= 0)
		document.getElementById("textarea").style.width = "99%";
}
function showDST(){
	var system_timezone_dut = "<% nvram_get("time_zone"); %>";
	if(system_timezone_dut.search("DST") >= 0 && "<% nvram_get("time_zone_dst"); %>" == "1"){
		document.getElementById('dstzone').style.display = "";
		document.getElementById('dstzone').innerHTML = "<#General_x_SystemTime_dst#>";
	}
}
function load_time_hour(){
	free_options(document.form.webs_update_time_x_hour);
	var j;
	for(var i = 0; i < 24; i++){
		if(i<10){
			j="0"+i;
			add_option(document.form.webs_update_time_x_hour, j, i, (i == update_time_hour_orig));
		}
		else
			add_option(document.form.webs_update_time_x_hour, i, i, (i == update_time_hour_orig));
	}
        
}
function load_time_min(){
	free_options(document.form.webs_update_time_x_min);
	var j;
	for(var i = 0; i < 60; i++){
		if(i<10){
			j="0"+i;
			add_option(document.form.webs_update_time_x_min, j, i, (i == update_time_min_orig));
		}
		else
			add_option(document.form.webs_update_time_x_min, i, i, (i == update_time_min_orig));
	}
}
function save_update_enable(flag){
	var hour_tmp="02";
	var min_tmp="00";
	
	if(flag=="on")
		document.firmware_form.webs_update_enable.value = 1;
	else if(flag=="off")
		document.firmware_form.webs_update_enable.value = 0;

	if(document.firmware_form.webs_update_enable.value==1){
		hour_tmp = (document.form.webs_update_time_x_hour.value.length==1)? "0"+document.form.webs_update_time_x_hour.value : document.form.webs_update_time_x_hour.value;
		min_tmp = (document.form.webs_update_time_x_min.value.length==1)? "0"+document.form.webs_update_time_x_min.value : document.form.webs_update_time_x_min.value;
	
		document.firmware_form.webs_update_time.value = hour_tmp+":"+min_tmp;
	}
	else
		document.firmware_form.webs_update_time.disabled = true;
			
	document.firmware_form.submit();
	if(flag == "on" || flag == "off"){
		// setTimeout("refreshpage()", 500);
	}
}
function change_beta_path(flag){
	
	document.firmware_form.webs_update_beta.value = (document.form.check_beta.checked)? 1:0;
	document.firmware_form.webs_update_ts.value = "";
	document.firmware_form.submit();
}

function initial(){
	show_menu();	
	showDST();
	load_time_hour();
	load_time_min();
	
//	document.getElementById("asus_link").href = Downloadlink;	//#FW_n2#
//	document.getElementById("asus_link2").href = helplink;	//#FW_desc0#
//	document.getElementById("faq_link1").href=faq_href1;	//#FW_n3#

	$("#FWString").append("<span class='current_fw_release_note'>"+FWString+"</span>");	//Untranslated

	if(afwupg_support && webs_update_enable_orig == 1){
		$(".current_fw_release_note").click({"model_name": "<#Web_Title2#>", "fwver": FWString}, show_current_release_note);		
	}

	if(revertfw_support && RevertFWver != ""){
		$("#FWString").append("<span class='label-fw_revert'>Revert</span>");	//Untranslated
		$(".label-fw_revert").css("margin-left", "10px")
							 .css("cursor", "pointer")
							 .css("text-decoration", "underline")
							 .click(show_revertfw_release_note);
		$("#update_div").css("margin-left", "220px");
	}

	if(rbkfw_support){
		if($('.label-fw_rbk')){
			$('.label-fw_rbk').remove();
		}
		if(rbk_count > 0 && httpApi.nvramGet(["webs_state_odm"],1).webs_state_odm != 0 && httpApi.nvramGet(["webs_state_odm"],1).webs_state_odm != "" ){
			$("#FWString").append("<span class='label-fw_rbk'><#FW_rollback#></span>");
			$(".label-fw_rbk").css("margin-left", "10px")
							 .css("cursor", "pointer")
							 .css("text-decoration", "underline")
							 .click(show_rbk_selector);
			$("#update_div").css("margin-left", "220px");
		}
	}
	
	var exist_update = 0;
	if(amesh_support && (isSwMode("rt") || isSwMode("ap")) && ameshRouter_support) {
		
		var have_node = false;
		var get_cfg_clientlist = httpApi.hookGet("get_cfg_clientlist");		
		$("#fw_version_tr").empty();
		var html = "";
		html += "<tr id='update_div' style='display:none;'>";
		html += "<th><#AiMesh_Check_Update#></th>";
		html += "<td>";
		html += '<div>';
		html += '<input type="button" id="update" name="update" class="button_gen" onclick="show_offline_msg(true);" value="<#liveupdate#>" />';
		html += '<input type="button" id="amas_update" class="button_gen" style="margin:-33px 0px 0px 200px;display:none;" onclick="cfgsync_firmware_upgrade();" value="<#CTL_upgrade#>"/>';
		html += '</div>';
		html += '<div id="check_beta_div"><input type="checkbox" name="check_beta" id="amas_beta" onclick="change_beta_path()" value="" <% nvram_match("webs_update_beta", "1", "checked"); %>/><#FW_beta_check#></div>';		// Untranslated 
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
		html += "<tr>";
		html += "<td class='aimesh_node_category_bg' colspan='2'><#AiMesh_Router#></td>";
		html += "</tr>";
		html += "<tr>";
		html += "<th>";
		html += "<#Web_Title2#>";
		html += "</th>";
		html += "</th>";
		html += "<td id='amas_" + mac_id + "' current_online='1'>";
		html += "<div id='current_version'><#ADSL_FW_item1#> : <span class='checkFWCurrent'>" + FWString + "</span>";
		if(revertfw_support){
			html += "<span class='aimesh_fw_revert' style='display:none;'>Revert</span>";	//Untranslated
		}
		if(rbkfw_support){
			html += "<span class='aimesh_fw_rbk' style='display:none;'><#FW_rollback#></span>";
		}
		html += "</div>";
		html += "<div id='amesh_manual_upload_fw'>";
		html += "<#FW_manual_update#> : ";
		html += "<span class='aimesh_fw_update_offline' style='margin-left:0px;' onclick='open_AiMesh_router_fw_upgrade();'><#CTL_upload#></span>";
		html += "</div>";
		html += "<div id='checkNewFW' class='checkNewFW' style='display:none;'><#ADSL_FW_item3#> : <span class='checkFWResult'></span></div>";
		html += "</td>";
		html += "</tr>";
		$("#fw_version_tr").before(html);
		if(afwupg_support && webs_update_enable_orig == 1){
			$("#amas_" + mac_id + "").children().find(".checkFWCurrent").addClass("aimesh_fw_release_note");
			$("#amas_" + mac_id + "").children().find(".checkFWCurrent").click({"model_name": "<#Web_Title2#>", "fwver": FWString}, show_current_release_note);
		}
		if(revertfw_support && RevertFWver != "" && !isSame_org && !isUnderREQ){
			$("#amas_" + mac_id + "").children().find(".aimesh_fw_revert").click(show_revertfw_release_note);
			$("#amas_" + mac_id + "").children().find(".aimesh_fw_revert").show();
		}
		if(rbkfw_support && rbk_count > 0 && httpApi.nvramGet(["webs_state_odm"],1).webs_state_odm != 0 && httpApi.nvramGet(["webs_state_odm"],1).webs_state_odm != ""){
			$("#amas_" + mac_id + "").children().find(".aimesh_fw_rbk").click(show_rbk_selector);
			$("#amas_" + mac_id + "").children().find(".aimesh_fw_rbk").show();
		}

		for (var idx in get_cfg_clientlist) {
			if (get_cfg_clientlist.hasOwnProperty(idx)) {
				if(idx == "0")
					continue;//filter CAP
				var frs_model_name = get_cfg_clientlist[idx].frs_model_name;
				var model_name = get_cfg_clientlist[idx].model_name;
				if (frs_model_name == "") frs_model_name = model_name;
				var ui_model_name = get_cfg_clientlist[idx].ui_model_name;
				var fwver = get_cfg_clientlist[idx].fwver;
				var online = get_cfg_clientlist[idx].online;
				var mac = get_cfg_clientlist[idx].mac;
				var mac_id = mac.replace(/:/g, "");
				var capability_value = (get_cfg_clientlist[idx].capability["4"]=="")?0:get_cfg_clientlist[idx].capability["4"];
				var ip = get_cfg_clientlist[idx].ip;
				var alias = "Home";
				var labelMac = mac;
				httpApi.getAiMeshLabelMac(model_name, mac, 
					function(_callBackMac){
						labelMac = _callBackMac;
					}
				);
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
					html += "<td class='aimesh_node_category_bg' colspan='2'><#AiMesh_Node#></td>";
					html += "</tr>";
				}
				html += "<tr>";
				html += "<th>";
				html += ui_model_name + " ( " + labelMac + " )";
				html += "<br>";
				html += "<#AiMesh_NodeLocation#> : " + htmlEnDeCode.htmlEncode(alias);
				html += "</th>";
				html += "<td id='amas_" + mac_id + "' current_online='" + online + "'>";
				if (check_is_merlin_fw(fwver))
					html += "<div id='current_version'><#ADSL_FW_item1#> : <span class='checkFWCurrent'>" + fwver.replace(/^(\d+)\.(\d+)\.(\d+).(\d+)\./, "$1$2$3$4.").replace("_0","") + "</span></div>";
				else
					html += "<div id='current_version'><#ADSL_FW_item1#> : <span class='checkFWCurrent'>" + fwver + "</span></div>";
				html += "<span class='aimesh_fw_revert_node'></span>";
				html += "<span class='aimesh_fw_rbk_node'></span>";
				html += "</div>";
				html += "<div id='manual_firmware_update'>";
				var support_manual_fw = check_AiMesh_fw_version(fwver);
				html += gen_AiMesh_fw_status(support_manual_fw, get_cfg_clientlist[idx]);
				html += "</div>";
				html += "<div id='checkNewFW' class='checkNewFW' style='display:none;'><#ADSL_FW_item3#> : <span class='checkFWResult'></span>";
				html += "</div>";
				html += "</td>";
				html += "</tr>";
				$("#fw_version_tr").before(html);

				if(afwupg_support && online == 1 && webs_update_enable_orig == 1){
					$("#amas_" + mac_id + "").children().find(".checkFWCurrent").addClass("aimesh_fw_release_note");
					$("#amas_" + mac_id + "").children().find(".checkFWCurrent").click({"model_name": ui_model_name, "fwver": fwver}, show_current_release_note);
				}
				if(support_manual_fw){
					if(online == "1")
						$("#amas_" + mac_id + "").children().find(".aimesh_fw_update_offline").click(get_cfg_clientlist[idx], open_AiMesh_node_fw_upgrade);
					else
						amesh_offline_flag = true;
				}

				if(online == 1){
					if(capability_value & 128){	//revertfw_support
						if(revertfw_support){
							$("#amas_" + mac_id + "").children().find(".aimesh_fw_revert_node").html("Revert"); 	/* Untranslated */
							$("#amas_" + mac_id + "").children().find(".aimesh_fw_revert_node").click(get_cfg_clientlist[idx], open_AiMesh_node_revertfw);
						}
						else{
							$("#amas_" + mac_id + "").children().find(".aimesh_fw_revert_node").remove();
						}
					}
					else{
							$("#amas_" + mac_id + "").children().find(".aimesh_fw_revert_node").remove();
					}

					if(capability_value & 4096){     //no_fw_manual_support
						$("#amas_" + mac_id + "").children("#manual_firmware_update").empty();
					}
					if(capability_value & 8192){     //live_update_support
						exist_update++;
					}
					
					if(capability_value & 16384){     //rbkfw_support
						if(rbkfw_support){
							$("#amas_" + mac_id + "").children().find(".aimesh_fw_rbk_node").html("<#FW_rollback#>");
							$("#amas_" + mac_id + "").children().find(".aimesh_fw_rbk_node").click(get_cfg_clientlist[idx], open_AiMesh_node_rbkfw);
						}
					}
				}

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
		if( (cfg_check == "7" && (cfg_upgrade == "1" || cfg_upgrade == "6" || cfg_upgrade == "8")) ||
			(cfg_check == "0" && cfg_upgrade == "10") ){
			startDownloading();
		}
	}
	else{
		if(webs_state_upgrade != "" && webs_state_upgrade != "1"){   //Show firmware is still downloading or fw upgrade loading bar if doing webs_upgrade.sh 
			startDownloading();
		}
	}

	if(no_update_support && exist_update==0){	//no live update in AiMesh
		$("table").remove("#auto_upgrade_setting");
		document.getElementById("update_div").style.display = "none";
		if(document.getElementById("sig_ver_field").style.display=="none"){
			document.getElementById("fw_tr").style.display = "none";
		}
		document.getElementById("linkpage_div").style.display = "none";
	}
	else{
		if((!live_update_support || !HTTPS_support)
			&& exist_update==0
		)
		{
			$("table").remove("#auto_upgrade_setting");
			document.getElementById("update_div").style.display = "none";
			if(document.getElementById("sig_ver_field").style.display=="none"){
				document.getElementById("fw_tr").style.display = "none";
			}
			document.getElementById("linkpage_div").style.display = "";
			document.getElementById("linkpage").href = helplink;
		} 
		else{
			document.getElementById("update_div").style.display = "";
			document.getElementById("linkpage_div").style.display = "none";

			if((confirm_show.length > 0 && confirm_show == 1) || nt_flag == "openReleaseNote"){
				do_show_confirm(webs_state_flag);
			}
			else if((confirm_show.length > 0 && confirm_show == 0) || nt_flag == "openReleaseNote"){
				if(amesh_support && (isSwMode("rt") || isSwMode("ap")) && ameshRouter_support) {
					var interval = setInterval(function() {
						if(link_status != undefined) {
							clearInterval(interval);
							show_offline_msg(true);
						}
					}, 100);
				}
				else{
					do_show_confirm(webs_state_flag);
				}
			}
		}

		if(afwupg_support){
			hide_upgrade_opt(webs_update_enable_orig);
			showclock();
		}

		if(!afwupg_support){
			$("table").remove("#auto_upgrade_setting");
		}
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

	if(amesh_support && (isSwMode("rt") || isSwMode("ap")) && ameshRouter_support) {
		$(".aimesh_manual_fw_update_hint").css("display", "block");
		$("#manually_upgrade_tr").css("display", "none");
		$("#productid_tr").css("display", "none");
		document.form.file.onchange = function() {
			submitForm();
		}
	}

	if(based_modelid == "DSL-AC68U"){
		$("#dsl_ac68u_fwver").show();
		$("#dsl_ac68u_drvver").show();
	}

	if(based_modelid == "DSL-N55U" || based_modelid == "DSL-N55U-B"){
		$("#dsl_n55u_fwver").show();
		$("#dsl_n55u_ras").show();
	}

	if(!betaupg_support){
		$("div").remove("#check_beta_div");
	}

	if(no_fw_manual_support){	//No manual
		$("#fw_note3").hide();
		$("div").remove("#amesh_manual_upload_fw");
		$("tr").remove("#manually_upgrade_tr");
		$(".aimesh_manual_fw_update_hint").css("display", "none");
	}

	if(is_ISP_incompatible)
	{
		var tmp_declaration = replace_isp_name("This equipment is authorized by %@ and the firmware will only be available for update online.");	//Untranslated
		$("#fw_note2").show();
		$("#fw_note2").html(tmp_declaration);
	}
	if(isSupport("is_ax5400_i1"))
	{
		$("#fw_note3").show();
		$("#fw_note3").html("Firmware upgrade is only accessible through Optus server.");	//Untranslated
	}

	if(gobi_support && (usb_index != -1) && (sim_state != "")){
		$("#modem_fw_upgrade").css("display", "");
	}

}

function replace_isp_name(_str){
	var updated_declaration = _str;
	var isp_name = "";
	if(isp_json[ttc]){
		if(document.form.preferred_lang.value=="TW"){
			isp_name = isp_json[ttc].ISP_TW;
		}
		else if(document.form.preferred_lang.value=="CN"){
			isp_name = isp_json[ttc].ISP_CN;
		}
		else{
			isp_name = isp_json[ttc].ISP_US;
		}
		updated_declaration = updated_declaration.replace("%@", isp_name);
	}
	
	return updated_declaration;
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
  				document.getElementById('update_states').innerHTML="<#info_failed#>&nbsp;<#FW_n4#>";
//				document.getElementById("faq_link2").href=faq_href2;
				document.getElementById('update').disabled = false;
			}
		},

		success: function(){
			if(cfg_sync_support){
				if(cfg_check == "" || cfg_check == "0" || cfg_check == "1" || cfg_check == "5"){
					setTimeout("detect_firmware();", 1000);
				}
				else{	// got fw info
					if(cfg_check == "2" || cfg_check == "3"){
						document.getElementById('update_scan').style.display="none";
						document.getElementById('update_states').innerHTML="<#info_failed#>&nbsp;<#FW_n4#>";
//						document.getElementById("faq_link2").href=faq_href2;
						document.getElementById('update').disabled = false;
					}
					else if(cfg_check == "7" || cfg_check == "9"){
						document.getElementById('update_scan').style.display="none";
						document.getElementById('update_states').innerHTML="";
						document.getElementById('update').disabled = false;
						var check_webs_state_info = webs_state_info;						
						note_display=0;
												
						if(amesh_support && (isSwMode("rt") || isSwMode("ap")) && ameshRouter_support)
							show_amas_fw_result();
						else
							do_show_confirm(webs_state_flag);
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
						document.getElementById('update_states').innerHTML="<#info_failed#>&nbsp;<#FW_n4#>";
//						document.getElementById("faq_link2").href=faq_href2;
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
						note_display=0;
						
						do_show_confirm(webs_state_flag);
					}
				}
			}
		}
	});
}

function do_show_confirm(flag){

					if(flag==1 || flag==2){
						document.getElementById('update_scan').style.display="none";
						document.getElementById('update_states').style.display="none";

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
//											cfgsync_firmware_upgrade();
											window.open(download_url_redir);
										}
										else{
											window.open(download_url_redir);
//											document.start_update.action_mode.value="apply";
//											document.start_update.action_script.value="stop_upgrade;start_webs_upgrade";
//											document.start_update.submit();
										}
									},
         					right_button_args: {},
         					iframe: "get_release_note0.asp",
         					margin: "100px 0px 0px 25px",
         					note_display_flag: note_display
     					});
     					
					}
					else{
						document.getElementById('update_scan').style.display="none";
						document.getElementById('update_states').style.display="";
						document.getElementById('update_states').innerHTML="<#is_latest#>";
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

function detect_update(){
	var download_info = 0;
	if(sw_mode != "1" || (link_status == "2" && link_auxstatus == "0") || (link_status == "2" && link_auxstatus == "2")){
		
		download_info++;
	}
	else if(dualwan_enabled &&
				((first_link_status == "2" && first_link_auxstatus == "0") || (first_link_status == "2" && first_link_auxstatus == "2")) ||
				((secondary_link_status == "2" && secondary_link_auxstatus == "0") || (secondary_link_status == "2" && secondary_link_auxstatus == "2"))){

		download_info++;
	}		
	else{
		document.getElementById('update_scan').style.display="none";
		document.getElementById('update_states').style.display="";
		document.getElementById('update_states').innerHTML="Unable to connect to the update server!";
		return false;	
	}

	if(download_info > 0){
		document.getElementById('update_states').style.display="";
		document.getElementById('update_states').innerHTML="Checking for available updates...";
		document.getElementById('update_scan').style.display="";
		document.getElementById('update').disabled = true;
		if(cfg_sync_support){
			cfgsync_firmware_check();
		}
		else{
			document.start_update.action_mode.value="apply";
			document.start_update.webs_update_trigger.value="FWUG";
			document.start_update.action_script.value="start_webs_update";
			document.start_update.submit();
		}
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
			location.href = "/";
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

					if(cfg_check == "7") {
						if(cfg_upgrade == "1" || cfg_upgrade == "6" || cfg_upgrade == "8"){
							document.getElementById("drword").innerHTML = "&nbsp;&nbsp;&nbsp;<#fw_downloading#>...";
							setTimeout("isDownloading();", 1000);
						}
					}
					else if(cfg_check == "0") {
						if(cfg_upgrade == "2" || cfg_upgrade == "3"){
							document.getElementById("drword").innerHTML = "<#connect_failed#>";
							return false;
						}
						else if(cfg_upgrade == "4"){
							document.getElementById("drword").innerHTML = "<#FIRM_fail_desc#><br><#FW_desc1#>";
							return false;
						}
						else if(cfg_upgrade == "10"){		// start upgrading
							document.getElementById("hiddenMask").style.visibility = "hidden";
							if(pipefw_support || urlfw_support){
								showLoadingBar(120);
								setTimeout("detect_httpd();", 122000);
								return false;
							}
							else{
								showLoadingBar(270);
								setTimeout("detect_httpd();", 272000);
								return false;
							}
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

							if(pipefw_support || urlfw_support){
								document.start_update.flag.value="";
								document.start_update.action_mode.value="apply";
								document.start_update.action_script.value="reboot";
								document.start_update.submit();
								showLoadingBar(120);
								setTimeout("detect_httpd();", 122000);
								return false;
							}
							else{
								showLoadingBar(270);
								setTimeout("detect_httpd();", 272000);
								return false;
							}
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

var revert_rebooting = 0;
function isRevertDownloading(){
	$.ajax({
    		url: '/ajax_fwdl_percent.asp',
    		dataType: 'script',
			timeout: 1500,
    		error: function(xhr){
					
					revert_rebooting++;
					if(revert_rebooting < 30){
						setTimeout("isRevertDownloading();", 1000);
					}
					else{
						document.getElementById("drword").innerHTML = "<#connect_failed#>";
						return false;
					}

    		},
    		success: function(){

					if(webs_state_upgrade == 0){
						document.getElementById("drword").innerHTML = "&nbsp;&nbsp;&nbsp;<#fw_downloading#>..."+fwdl_percent;
						setTimeout("isRevertDownloading();", 1000);
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

							if(pipefw_support || urlfw_support){
								document.start_update.flag.value="";
								document.start_update.action_mode.value="apply";
								document.start_update.action_script.value="reboot";
								document.start_update.submit();
								showLoadingBar(120);
								setTimeout("detect_httpd();", 122000);
								return false;
							}
							else{
								showLoadingBar(270);
								setTimeout("detect_httpd();", 272000);
								return false;
							}
						}

					}
  			}
  		});
}

function startRevertDownloading(){
	disableCheckChangedStatus();			
	dr_advise();
	document.getElementById("drword").innerHTML = "&nbsp;&nbsp;&nbsp;<#fw_downloading#>...";
	isRevertDownloading();
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
	if(isSupport("demoui"))
		return;
	else{
		if(!check_zip(document.form.file))
				return;
		else {
			var status = onSubmitCtrlOnly(document.form.upload, 'Upload1');
			if(amesh_support && status && (isSwMode("rt") || isSwMode("ap")) && ameshRouter_support) {
				if(interval_update_AiMesh_fw_status) {
					clearInterval(interval_update_AiMesh_fw_status);
					interval_update_AiMesh_fw_status = false;
				}
			}
		}
	}
}

function sig_version_check(){
	document.getElementById("sig_check").disabled = true;
	$("#sig_status").show();
	document.sig_update.submit();
	$("#sig_status").html("<#sig_checking#>");
	document.getElementById("sig_update_scan").style.display = "";
	setTimeout("sig_check_status();", 8000);
}

var sdead=0;
var sig_chk_count=60;
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
			--sig_chk_count;
			$("#sig_status").show();
			if(sig_state_flag == 0 && sig_state_error == 0 && sig_state_update == 1){		// no need upgrade
				$("#sig_status").html("<#sig_up2date#>");
				document.getElementById("sig_update_scan").style.display = "none";
				document.getElementById("sig_check").disabled = false;
			}
			else{
				if(sig_state_error != 0){		// update error
					$("#sig_status").html("<#sig_failed#>");
					document.getElementById("sig_update_scan").style.display = "none";
					document.getElementById("sig_check").disabled = false;
				}
				else{
					if(sig_state_flag == 1 && sig_state_update == 0 && sig_state_upgrade == 1){		//update complete
						update_sig_ver();
					}
					else{		//updating
						if(sig_chk_count < 1){
							$("#sig_status").hide();
							document.getElementById("sig_update_scan").style.display = "none";
							document.getElementById("sig_check").disabled = false;
						}
						else{
							$("#sig_status").html("<#sig_updating#>");
							setTimeout("sig_check_status();", 1000);
						}
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
		if(sig_ver == sig_ver_ori){
			setTimeout("update_sig_ver();", 1000);
		}
		else{
			document.getElementById("sig_update_date").innerHTML = "";
			document.getElementById("sig_update_scan").style.display = "none";
			document.getElementById("sig_check").disabled = false;
			$("#sig_status").html("<#sig_completed#>");
			$("#sig_ver_word").html(sig_ver);
		}
	}
  	
	});
}

function hide_upgrade_opt(flag){

	document.form.webs_update_time_x_hour.value = update_time_hour_orig;  
	document.form.webs_update_time_x_min.value = update_time_min_orig;
	(flag == 1) ? inputCtrl(document.form.webs_update_time_x_hour,1) : inputCtrl(document.form.webs_update_time_x_hour,0);
	(flag == 1) ? inputCtrl(document.form.webs_update_time_x_min,1) : inputCtrl(document.form.webs_update_time_x_min,0);
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

function validForm(){
	
	if(document.form.webs_update_time_x_hour.value.length==0){
		alert("<#JS_fieldblank#>");
		document.form.webs_update_time_x_hour.focus();
		return false;
	}
	if(document.form.webs_update_time_x_min.value.length==0){
		alert("<#JS_fieldblank#>");
		document.form.webs_update_time_x_min.focus();
		return false;
	}	

	return true;
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
		$(".checkNewFW").css("display", "none");
		$(".checkFWResult").empty();
		$(".checkFWResult").removeClass("aimesh_fw_release_note");
		detect_update();
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
	$amesh_hint_offline.html("<#FW_note_AiMesh_offline#>");
	$offlineHtml.append($amesh_hint_offline);

	var $amesh_hint_text = $('<div>');
	$amesh_hint_text.addClass("amesh_hint_text");
	$amesh_hint_text.html("<#AiMesh_OfflineTips#> :");
	$offlineHtml.append($amesh_hint_text);

	var $amesh_hint_content = $('<div>');
	$amesh_hint_content.addClass("amesh_hint_text");
	$amesh_hint_content.css("margin-left", "auto");
	$offlineHtml.append($amesh_hint_content);

	var $msg_item =  $('<ol>');
	var msg_text = "<li><#AiMesh_OfflineTips1#></li>";
	msg_text += "<li><#AiMesh_OfflineTips2#></li>";
	msg_text += "<li><#AiMesh_OfflineTips3#></li>";
	msg_text += "<li><#AiMesh_OfflineTips4#></li>";
	msg_text += "<li><#AiMesh_OfflineTips5#></li>";
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
				detect_update();
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
					var frs_model_name = get_cfg_clientlist[idx].frs_model_name;
					if (frs_model_name == "") frs_model_name = get_cfg_clientlist[idx].model_name;
					var mac = get_cfg_clientlist[idx].mac;
					var newfwver = get_cfg_clientlist[idx].newfwver;
					var mac_id = mac.replace(/:/g, "");
					var ck_fw_result = "<#is_latest#>";
					var online = get_cfg_clientlist[idx].online;
					var fwver = get_cfg_clientlist[idx].fwver;
					var product_id = get_cfg_clientlist[idx].product_id;
					$("#amas_" + mac_id + "").children().find(".checkFWResult").html(ck_fw_result);
					if(newfwver != "") {
						if (check_is_merlin_fw(fwver)) {
							ck_fw_result = newfwver.replace(/^(\d+)\.(\d+)\.(\d+).(\d+)\./, "$1$2$3$4.").replace("_",".").replace("_0","");
						} else {
							ck_fw_result = newfwver;
							$("#amas_update").css("display", "");
						}
						$("#amas_" + mac_id + "").children().find(".checkFWResult").addClass("aimesh_fw_release_note");
						$("#amas_" + mac_id + "").children().find(".checkFWResult").html(ck_fw_result);
						$("#amas_" + mac_id + "").children().find(".checkFWResult").click({"isMerlin" : check_is_merlin_fw(fwver), "model_name": frs_model_name, "product_id": product_id, "newfwver": newfwver}, show_fw_release_note);
					}
					if(online == "1")
						$("#amas_" + mac_id + "").children("#checkNewFW").css("display", "");
				}
			}
			if(rbkfw_support && rbk_count > 0 && httpApi.nvramGet(["webs_state_odm"],1).webs_state_odm != 0 && httpApi.nvramGet(["webs_state_odm"],1).webs_state_odm != ""){
				$("#amas_" + mac_id + "").children().find(".aimesh_fw_rbk").click(show_rbk_selector);
				$("#amas_" + mac_id + "").children().find(".aimesh_fw_rbk").show();
			}
		}
	});
}

function show_rbk_release_note(event) {
	if($(".confirm_block").length > 0)
		$(".confirm_block").remove();

	document.amas_release_note.model.value = event.data.model_name;
	document.amas_release_note.version.value = event.data.fwver;
	document.amas_release_note.submit();
	confirm_asus({
		title: "Release Note",
		//contentA: "<#exist_new#><br>",
		contentA: event.data.model_name+" : "+event.data.fwver,
		//contentC: "<br><#ADSL_FW_note#> <#Main_alert_proceeding_desc5#>",
		contentC: "",
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
	$(".confirm_block").css( "zIndex", 10001 );
	setTimeout(function(){check_current_release_note_status();}, 2000);
}

function show_current_release_note(event) {
	if($(".confirm_block").length > 0)
		$(".confirm_block").remove();

	document.amas_release_note.model.value = event.data.model_name;
	document.amas_release_note.version.value = event.data.fwver;
	document.amas_release_note.submit();
	confirm_asus({
		title: "Release Note",
		//contentA: "<#exist_new#><br>",
		contentA: event.data.model_name+" : "+event.data.fwver,
		//contentC: "<br><#ADSL_FW_note#> <#Main_alert_proceeding_desc5#>",
		contentC: "",
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

	setTimeout(function(){check_current_release_note_status();}, 2000);
}
function check_current_release_note_status() {
	if($(".confirm_block").length > 0) {
		$.ajax({
			url: '/ajax_onboarding.asp',
			dataType: 'script',
			success: function() {
				switch(cfg_note) {
					case "0" :
						setTimeout(function(){check_current_release_note_status();}, 1000);
						break;
					case "1" :
						show_current_release_note_result(true);
						break;
					case "2" :
					case "3" :
						show_current_release_note_result(false);
						break;
				}
			}
		});
	}
}
function show_current_release_note_result(_status) {
	
	if(_status) {
		$(".confirm_block").children().find("#status_iframe").attr("src", "get_release_note_amas.asp?flag=1");//reload and flag_show
		$(".confirm_block").children().find("#status_iframe").load();
	}
	else
		$(".confirm_block").children().find("#status_iframe").contents().find("#amas_release_note_hint").val("<#FW_rlnote_failed#>");
}

function show_fw_release_note(event) {
	if($(".confirm_block").length > 0)
		$(".confirm_block").remove();

	document.amas_release_note.model.value = event.data.model_name;
	if (event.data.isMerlin) {
		document.amas_release_note.version.value = event.data.newfwver.replace(/^(\d+)\.(\d+)\.(\d+).(\d+)\./, "$1$2$3$4.");
		if (event.data.product_id)
			siteurl = "https://fwupdate.asuswrt-merlin.net/" + event.data.product_id;
		else
			siteurl = download_url;
	} else {
		document.amas_release_note.version.value = event.data.newfwver;
		siteurl = "";
	}
	document.amas_release_note.submit();
	confirm_asus({
		title: "Release Note",
		contentA: "<#exist_new#><br>",
		//contentC: "<br><#ADSL_FW_note#> <#Main_alert_proceeding_desc5#>",
		contentC: "",
		left_button: (event.data.isMerlin ? "Visit download site" : ""),
		left_button_callback: function(){window.open(siteurl);},
		left_button_args: {},
		right_button: "<#CTL_close#>",
		right_button_callback: function(){confirm_cancel();},
		right_button_args: {},
		iframe: "get_release_note_amas.asp",
		margin: "100px 0px 0px 25px",
		note_display_flag: note_display
	});

	setTimeout(function(){check_fw_release_note_status();}, 2000);
}
function check_fw_release_note_status() {
	if($(".confirm_block").length > 0) {
		$.ajax({
			url: '/ajax_onboarding.asp',
			dataType: 'script',
			success: function() {
				switch(cfg_note) {
					case "0" :
						setTimeout(function(){check_fw_release_note_status();}, 1000);
						break;
					case "1" :
						show_fw_release_note_result(true);
						break;
					case "2" :
					case "3" :
						show_fw_release_note_result(false);
						break;
				}
			}
		});
	}
}
function show_fw_release_note_result(_status) {
	
	if(_status) {
		$(".confirm_block").children().find("#status_iframe").attr("src", "get_release_note_amas.asp?flag=1");//reload and flag_show
		$(".confirm_block").children().find("#status_iframe").load();
	}
	else
		$(".confirm_block").children().find("#status_iframe").contents().find("#amas_release_note_hint").val("<#FW_rlnote_failed#>");
}


function show_revertfw_release_note(event) {
	var notice="";
	//alert(event.data.model_name+"/"+event.data.newfwver);
	if($(".confirm_block").length > 0)
		$(".confirm_block").remove();

	document.revertfw_note.model.value = frsmodel;
	document.revertfw_note.version.value = RevertFWver;
	document.revertfw_note.submit();

	notice = "<#FW_note#><br><br>If you want to revert both your main router and node(s) firmware to the previous version, please revert node(s) first before the main router.";		//Untranslated
	if(document.firmware_form.webs_update_enable.value == 1){
		notice += "<br><br>If you would like to revert to previous firmware version, we suggest you temporarily disable auto firmware update to make sure you grant every upgrade.";		//Untranslated
	}
	confirm_asus({
		title: support_site_modelid,
		contentA: "<#FW_item2#> : "+RevertFWver+"<br>",
		contentC: notice,
		left_button: "<#CTL_close#>",
		left_button_callback: function(){confirm_cancel();},
		left_button_args: {},
		right_button: "Revert",
		right_button_callback: function(){revert_fw_confirm();},
		right_button_args: {},
		iframe: "get_release_note_revertfw.asp",
		margin: "100px 0px 0px 25px",
		note_display_flag: note_display
	});
	$(".confirm_button_gen_long_right").css("display", "none");

	setTimeout(function(){show_revertfw_release_note_result(true);}, 2000);
}
function show_revertfw_release_note_result(_status) {
	
	if(_status) {
		$(".confirm_block").children().find("#status_iframe").attr("src", "get_release_note_revertfw.asp?flag=1");//reload and flag_show
		$(".confirm_block").children().find("#status_iframe").load();
	}
	else
		$(".confirm_block").children().find("#status_iframe").contents().find("#amas_release_note_hint").val("Previous version is currently unavailable.");/* untranslated */
}
function revert_fw_confirm(){
	document.revert_fw.submit();
}
function check_reverfw_status(link) {
	urlExists(link);
}
function urlExists(url, callback){
	$.ajax({
		type: 'HEAD',
		url: url,
		cache: false,
		success: function(){
			console.log("Yes");
		},
		error: function() {
			console.log("No");
		}
	});
}

function show_rbk_selector(){

	$("#rbk_Block").remove();
	$("html, body").animate({ scrollTop: 0 }, "fast");
	document.body.style.overflow = 'hidden';
	$("#Loading").css('visibility', 'visible');
	$("#loadingBlock").css('visibility', 'hidden');

	var divObj = document.createElement("div");
	divObj.setAttribute("id","rbk_Block");
	divObj.className = "clientlist_viewlist";
	document.body.appendChild(divObj);
	$("#rbk_Block").css('zIndex', '10000');
	//cal_panel_block("rbk_Block", 0.045);
	cal_panel_block("rbk_Block", 0.2);

	create_rbkfw_view();
}
function create_rbkfw_view(){
	
	rbk_Block_view_hide_flag = false;

	document.getElementById("rbk_Block").onclick = function() {show_rbk_Block();}

	var divObj = document.createElement("div");
	divObj.id = "div_rbk_title";
	divObj.className = "rbk_title";
	divObj.innerHTML = "<#FW_rollback_selector#>";
	$(divObj).appendTo('#rbk_Block');

	var divObj2 = document.createElement("div");
	divObj2.className = "rbk_close";
	divObj2.innerHTML = "<img src='/images/button-close.gif' style='width:30px;cursor:pointer' onclick='close_rbk_selector();'>";
	$(divObj2).appendTo('#rbk_Block');	

	var divObj3 = document.createElement("div");
	divObj3.className = "splitLine";
	divObj3.style = "margin: 40px 0 10px 15px";
	$(divObj3).appendTo('#rbk_Block');


	var tableObj = document.createElement("table");
	tableObj.id = 'table_panel'; 
	tableObj.border = '0';
	tableObj.align = 'center';
	tableObj.cellpadding = '0';
	tableObj.cellspacing='0';
	tableObj.style = "width:100%;padding:0 15px 15px 15px;";
	$(tableObj).appendTo('#rbk_Block');

	var tableObj2 = document.createElement("table");
	tableObj2.id = 'table_rbk';
	tableObj2.className = "FormTable_table";
	tableObj2.border = '1';
	tableObj2.align = 'center';
	tableObj2.cellpadding = '0';
	tableObj2.cellspacing='0';
	tableObj2.style = "width:100%;margin-top:15px;";
	$(tableObj2).appendTo('#table_panel');

	var theadObj = document.createElement("thead");
	theadObj.innerHTML = "<tr height='28px'><td id='td_all_list_title' colspan='2'><#FW_rollback_choose#></td></tr>";
	$(theadObj).appendTo('#table_rbk');

	var trObj = document.createElement("tr");
	trObj.id = 'tr_title';
	trObj.height = "40px";
	$(trObj).appendTo('#table_rbk');

	var thObj = document.createElement("th");
	thObj.className = 'IE8HACK';
	thObj.width = "10%";
	$(thObj).appendTo('#tr_title');

	var thObj2 = document.createElement("th");
	thObj2.className = 'IE8HACK';
	thObj2.width = "90%";
	thObj2.innerHTML = "Firmware Version";
	$(thObj2).appendTo('#tr_title');

	var rbk_tr_list = new Array;
	var rbk_td_radio = new Array;
	var rbk_td_fwver = new Array;
	for (var j = 0; j < rbk_count-1; j++)
	{
		rbk_tr_list[j] = document.createElement("tr");
		rbk_tr_list[j].id = 'tr_rbk_option'+j;
		rbk_tr_list[j].height = "40px";
		$(rbk_tr_list[j]).appendTo('#table_rbk');

		rbk_td_radio[j] = document.createElement("td");
		rbk_td_radio[j].Name = "rbk_opt";
		rbk_td_radio[j].id = "rbk_option"+j;
		rbk_td_radio[j].innerHTML = "<input name='rbk_opt' id='rbk_select_"+j+"' type='radio' class='input' value='"+_rollback_info[j]+"'>";
		$(rbk_td_radio[j]).appendTo('#tr_rbk_option'+j);

		rbk_td_fwver[j] = document.createElement("td");
		rbk_td_fwver[j].className = "td_rbk_fwver";
		rbk_td_fwver[j].style = "padding:10px;text-align:left";
		rbk_td_fwver[j].innerHTML = _rollback_info[j];
		$(rbk_td_fwver[j]).appendTo('#tr_rbk_option'+j);
		$('#tr_rbk_option'+j).children('td').eq(1).click(
			{"model_name": httpApi.nvramGet(["webs_state_odm"],1).webs_state_odm, "fwver": _rollback_info[j] }, show_rbk_release_note
		);
	}


	var divObj_bot = document.createElement("div");
	divObj_bot.id = "div_rbk_bot";
	divObj_bot.style = "margin-top:10px;margin-bottom:20px;width:100%;text-align:center;";
	divObj_bot.innerHTML = "<input class='button_gen' type='button' onclick='close_rbk_selector()' value='<#CTL_Cancel#>'><input class='button_gen' type='button' onclick='rbk_fw_confirm()' style='margin-left:15px;' value='<#FW_rollback#>'>";
	$(divObj_bot).appendTo('#rbk_Block');

}
var rbk_Block_view_hide_flag = false;
function hide_rbk_Block() {
	if(rbk_Block_view_hide_flag)
	{
		close_rbk_selector();
	}
	rbk_Block_view_hide_flag=true;
}
function show_rbk_Block() {
	rbk_Block_view_hide_flag = false;
}
function close_rbk_selector() {
	location.reload();
}
function rbk_fw_confirm(){
	if($('input[name=rbk_opt]:checked').val() != undefined){
		document.rollback_fw.model.value = httpApi.nvramGet(["webs_state_odm"],1).webs_state_odm;
		document.rollback_fw.version.value = $('input[name=rbk_opt]:checked').val();
		$("#rbk_Block").fadeOut();
		$("#Loading").css('visibility', 'hidden');
		startRevertDownloading();
		document.rollback_fw.submit();
	}
	else{
		alert("<#JS_fieldblank#>");
		return;
	}
}

function open_AiMesh_node_rbkfw(event) {
	var url = httpApi.aimesh_get_win_open_url(event.data, "AiMesh_Node_RbkFirmware.asp");
	var window_width = 1080;
	var window_height = 640;
	var window_top = screen.availHeight / 2 - window_height / 2;
	var window_left = screen.availWidth / 2 - window_width / 2;
	window.open(url, '_new' ,'width=' + window_width + ',height=' + window_height + ', top=' + window_top + ',left=' + window_left + ',menubar=yes,scrollbars=no,toolbar=yes,resizable=yes,status=yes,location=no');
}

function open_AiMesh_node_revertfw(event) {	
	var url = httpApi.aimesh_get_win_open_url(event.data, "AiMesh_Node_RevertFirmware.asp");
	var window_width = 720;
	var window_height = 720;
	var window_top = screen.availHeight / 2 - window_height / 2;
	var window_left = screen.availWidth / 2 - window_width / 2;
	window.open(url, '_new' ,'width=' + window_width + ',height=' + window_height + ', top=' + window_top + ',left=' + window_left + ',menubar=no,scrollbars=yes,toolbar=no,resizable=no,status=no,location=no');
}

function open_AiMesh_node_fw_upgrade(event) {
	var url = httpApi.aimesh_get_win_open_url(event.data, "AiMesh_Node_FirmwareUpgrade.asp");
	var window_width = 550;
	var window_height = 550;
	var window_top = screen.availHeight / 2 - window_height / 2;
	var window_left = screen.availWidth / 2 - window_width / 2;
	window.open(url, '_new' ,'width=' + window_width + ',height=' + window_height + ', top=' + window_top + ',left=' + window_left + ',menubar=no,scrollbars=yes,toolbar=no,resizable=no,status=no,location=no');
}
function open_AiMesh_router_fw_upgrade() {
	document.form.file.click();
}
function update_AiMesh_fw() {
	$.ajax({
		url: '/ajax_onboarding.asp',
		dataType: 'script',
		success: function(){
			amesh_offline_flag = false;
			for (var idx in get_cfg_clientlist) {
				if(get_cfg_clientlist.hasOwnProperty(idx)) {
					var model_name = get_cfg_clientlist[idx].model_name;
					var mac = get_cfg_clientlist[idx].mac;
					var fwver = get_cfg_clientlist[idx].fwver;
					var ip = get_cfg_clientlist[idx].ip;
					var online = get_cfg_clientlist[idx].online;
					var mac_id = mac.replace(/:/g, "");
					var capability_value = (get_cfg_clientlist[idx].capability["4"]=="")?0:get_cfg_clientlist[idx].capability["4"];

					if (check_is_merlin_fw(fwver))
						$("#amas_" + mac_id + "").children("#current_version").html("<#ADSL_FW_item1#> : <span class='checkFWCurrent'>" + fwver.replace(/^(\d+)\.(\d+)\.(\d+).(\d+)\./, "$1$2$3$4.").replace("_0","") + "</span>");
					else {
						var current_fwver = $("#amas_" + mac_id + "").find("#current_version .checkFWCurrent").html();
						if(fwver != current_fwver){

							if(revertfw_support){
								if(idx==0){
									if(RevertFWver != "" && !isSame_org && !isUnderREQ){
										$("#amas_" + mac_id + "").children("#current_version").html("<#ADSL_FW_item1#> : <span class='checkFWCurrent'>" + 	fwver + "</span><span class='aimesh_fw_revert'>Revert</span>");
									}
								}
								else{
									$("#amas_" + mac_id + "").children("#current_version").html("<#ADSL_FW_item1#> : <span class='checkFWCurrent'>" + fwver + "</span><span class='aimesh_fw_revert_node'></span>");
								}
							}
							else{
								$("#amas_" + mac_id + "").children("#current_version").html("<#ADSL_FW_item1#> : <span class='checkFWCurrent'>" + fwver + "</span>");
							}
						}

						if(afwupg_support && online == 1 && webs_update_enable_orig == 1){
							$("#amas_" + mac_id + "").children().find(".checkFWCurrent").addClass("aimesh_fw_release_note");
							$("#amas_" + mac_id + "").children().find(".checkFWCurrent").click({"model_name": model_name, "fwver": fwver}, show_current_release_note);
						}
					}
					var support_manual_fw = check_AiMesh_fw_version(fwver);
					if(support_manual_fw){
						var last_online = $("#amas_" + mac_id + "").attr("current_online");
						if(online != last_online){
							$("#amas_" + mac_id + "").attr("current_online", online);
							$("#amas_" + mac_id + "").children("#manual_firmware_update").empty();
							$("#amas_" + mac_id + "").children("#manual_firmware_update").append(gen_AiMesh_fw_status(support_manual_fw, get_cfg_clientlist[idx]));
							if(online == "1")
								$("#amas_" + mac_id + "").children().find(".aimesh_fw_update_offline").click(get_cfg_clientlist[idx], open_AiMesh_node_fw_upgrade);
						}

						if(online == "0")
							amesh_offline_flag = true;
					}
					else{
						$("#amas_" + mac_id + "").children("#manual_firmware_update").empty();
						$("#amas_" + mac_id + "").children("#manual_firmware_update").append(gen_AiMesh_fw_status(support_manual_fw, get_cfg_clientlist[idx]));
					}
					if(capability_value & 4096){     //no_fw_manual_support
						$("#amas_" + mac_id + "").children("#manual_firmware_update").empty();
					}

					if(revertfw_support){
						if(idx==0){
							if(RevertFWver != "" && !isSame_org && !isUnderREQ){
								$("#amas_" + mac_id + "").children().find(".aimesh_fw_revert").click(show_revertfw_release_note);
							}
						}
						else{
							if(capability_value & 128){	//revertfw_support
								if(online == "1"){
									$("#amas_" + mac_id + "").children().find(".aimesh_fw_revert_node").html("Revert");
									$("#amas_" + mac_id + "").children().find(".aimesh_fw_revert_node").click(get_cfg_clientlist[idx], open_AiMesh_node_revertfw);
								}
								else{
									$("#amas_" + mac_id + "").children().find(".aimesh_fw_revert_node").remove();
								}
							}
							else{
								$("#amas_" + mac_id + "").children().find(".aimesh_fw_revert_node").remove();
							}
						}
					}

					if(rbkfw_support){
						if(capability_value & 16384){     //rbkfw_support
							if(online == "1"){
								$("#amas_" + mac_id + "").children().find(".aimesh_fw_rbk_node").html("<#FW_rollback#>");
								$("#amas_" + mac_id + "").children().find(".aimesh_fw_rbk_node").click(get_cfg_clientlist[idx], open_AiMesh_node_rbkfw);
							}
						}
					}

				}
			}
		}
	});
}
function gen_AiMesh_fw_status(_manual_status, _node_info) {
	var html = "";
	if(_manual_status) {
		html += "<#FW_manual_update#> : ";
		if(_node_info.online == "0") {
			html += "<span class='aimesh_fw_update_offline' style='margin-left:0px;' onclick='show_offline_msg(false);'><#Clientlist_OffLine#></span>";
		}
		else {
			html += "<span class='aimesh_fw_update_offline' style='margin-left:0px;' ><#CTL_upload#></span>";
		}
	}
	else {
		html += "<span class='aimesh_fw_update_offline' style='margin-left:0px;text-decoration:none;cursor:none;'><#FW_note_AiMesh_auto#></span>";
	}
	return html;
}
function check_AiMesh_fw_version(_fw) {
	var manual_status = true;
	if(support_site_modelid == "GT-AC2900_SH"){
		manual_status = false;
	}
	return manual_status;
}

function toggle_fw_check(state) {
	httpApi.nvramSet({
			"firmware_check_enable" : state,
			"action_mode": "apply"});
}

function upgrade_modem_fw(){
	if(document.modem_form.file.value.length == 0){
		alert("<#JS_Shareblanktest#>");
		document.modem_form.focus();
		return;
	}
	showLoading(300);
	document.modem_form.submit();
	setTimeout(get_mobile_fw_upgrade_status, 15000);
	setTimeout("detect_httpd();", 272000);
}


function get_mobile_fw_upgrade_status(){
	if(gobi_support){
		var upgrade_status = httpApi.nvramGet(["mobile_upgrade_md5", "mobile_upgrade_name", "mobile_upgrade_process", "mobile_upgrade_now", "mobile_upgrade_status"]);

		if(upgrade_status.mobile_upgrade_md5 != "1" && upgrade_status.mobile_upgrade_name != "1" && upgrade_status.mobile_upgrade_process != "100" && upgrade_status.mobile_upgrade_now != "1")
			setTimeout(get_mobile_fw_upgrade_status, 1000);
		else{
			if(upgrade_status.mobile_upgrade_md5 == "1" || upgrade_status.mobile_upgrade_name == "1")
				alert("<#Mobile_lte_upgrade_wrong_fw#>");
			else if(upgrade_status.mobile_upgrade_now == "1")
				alert("<#Mobile_lte_upgrade_fail#>");
			else if(upgrade_status.mobile_upgrade_status == "3")
				alert("<#Contact_customer_service#>");
		}
	}
	else
		return;
}

</script>
</head>
<body onload="initial();" class="bg">

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
		  <td bgcolor="#4D595D" valign="top">
		  <div>&nbsp;</div>
		  <div class="formfonttitle"><#menu5_6#> - <#menu5_6_3#></div>
		  <div style="margin:10px 0 10px 5px;" class="splitLine"></div>
		  <div class="formfontdesc"><strong><#FW_note#></strong>
				<ol>
					<li><#FW_n0#></li>
					<li><#FW_n1#></li>
					<li id="fw_note2"><#FW_n2#>&nbsp;<#FW_n3#></li>
					<li id="fw_note3">Get the latest firmware version from the download site at <a style="font-weight: bolder;text-decoration: underline;color:#FFFFFF;" href="https://www.asuswrt-merlin.net/download/" target="_blank">https://www.asuswrt-merlin.net/download/</a></li>
					<li style="display:none;"id="fw_note5"><#FW_n5#></li>
					</ol>
		  </div>
		  <br>

		<table id="auto_upgrade_setting" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
			<thead>
			<tr>
				<td colspan="2"><#FW_auto_upgrade#></td>
			</tr>	
			</thead>
			<tr>
				<th><#FW_auto_upgrade#></th>
				<td>
					<div align="center" class="left" style="width:75px; float:left; cursor:pointer;" id="switch_webs_update_enable"></div>
					<script type="text/javascript">
					$('#switch_webs_update_enable').iphoneSwitch('<% nvram_get("webs_update_enable"); %>',
						function(){
							ASUS_EULA.config(function(){
								hide_upgrade_opt(1);
								save_update_enable('on');
							},
								refreshpage
							)

							if(!ASUS_EULA.check("asus_pp", "AUTOUPGRADE")) return false;

							hide_upgrade_opt(1);
							save_update_enable('on');
						},
						function(){
							hide_upgrade_opt(0);
							save_update_enable('off');
						}
					);
					</script>
				</td>	
			</tr>
			<tr>
				<th><#FW_auto_time#></th>
				<td>
					<select id="webs_update_time_x_hour" name="webs_update_time_x_hour" class="input_option" onchange="save_update_enable();"></select> : 
					<select id="webs_update_time_x_min" name="webs_update_time_x_min" class="input_option" onchange="save_update_enable();"></select>
					<span id="system_time" class="devicepin" style="color:#FFFFFF;"></span>
					<br><span id="dstzone" style="display:none;margin-left:5px;color:#FFFFFF;"></span>
				</td>	
			</tr>

			<tr>
				<td colspan="2">
					<#FW_auto_upgrade_desc#>
				</td>
			</tr>
		</table>

		<table id="secur_stab_setting" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
			<thead>
			<tr>
				<td colspan="2"><#Secur_Stab_auto_upgrade#></td>
			</tr>	
			</thead>
			<tr>
				<th><#Secur_Stab_auto_upgrade#></th>
				<td>
					<div align="center" class="left" style="width:75px; float:left; cursor:pointer;" id="switch_security_update_enable"></div>
					<script type="text/javascript">
					$('#switch_security_update_enable').iphoneSwitch(httpApi.securityUpdate.get(),
						function(){
							//on
							ASUS_EULA.config(function(){httpApi.securityUpdate.set(1);},refreshpage)
							if(!ASUS_EULA.check("asus_pp", "ASD")) return false;
							if(!ASUS_EULA.check("asus_pp", "AHS")) return false;

							httpApi.securityUpdate.set(1);
						},
						function(){
							//off
							httpApi.securityUpdate.set(0);
						}
					);
					</script>
				</td>	
			</tr>

			<tr>
				<td colspan="2">
					<#Secur_Stab_auto_upgrade_desc#>
				</td>
			</tr>
		</table>

		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
			<thead>
				<tr id="fw_tr">
					<td colspan="2"><#FW_item2#></td>	
				</tr>	
			</thead>	
			<tr id="productid_tr">
				<th><#FW_item1#></th>
				<td><#Web_Title2#></td>
			</tr>

			<tr id="dsl_n55u_fwver" style="display:none;">
				<th><#adsl_fw_ver_itemname#></th>
				<td><input type="text" class="input_15_table" value="<% nvram_dump("adsl/tc_fw_ver_short.txt",""); %>" readonly="1" autocorrect="off" autocapitalize="off"></td>
			</tr>
			<tr id="dsl_n55u_ras" style="display:none;">
				<th>RAS</th>
				<td><input type="text" class="input_20_table" value="<% nvram_dump("adsl/tc_ras_ver.txt",""); %>" readonly="1" autocorrect="off" autocapitalize="off"></td>
			</tr>
			<tr id="dsl_ac68u_fwver" style="display:none;">
				<th>DSL <#FW_item2#></th>
				<td><% nvram_get("dsllog_fwver"); %></td>
			</tr>
			<tr id="dsl_ac68u_drvver" style="display:none;">
				<th><#adsl_fw_ver_itemname#></th>
				<td><% nvram_get("dsllog_drvver"); %></td>
			</tr>
			<tr id="sig_ver_field" style="display:none;">
				<th>Trend Micro: <#sig_ver#></th>
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
			<tr id="fwcheck_tr">
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(50,15);">Scheduled check for new firmware availability</a></th>
				<td>
					<input type="radio" onclick="toggle_fw_check(1);" name="firmware_check_enable" class="input" value="1" <% nvram_match("firmware_check_enable", "1", "checked"); %>><#checkbox_Yes#>
					<input type="radio" onclick="toggle_fw_check(0);" name="firmware_check_enable" class="input" value="0" <% nvram_match("firmware_check_enable", "0", "checked"); %>><#checkbox_No#>
				</td>
			</tr>
			<tr id="fw_version_tr">
				<th><#FW_item2#></th>
				<td>
					<div id="FWString" style="height:33px;margin-top:5px;"></div>
					<div id="update_div" style="margin-left:200px;margin-top:-38px;display:none;">
						<input type="button" id="update" name="update" class="button_gen" onclick="detect_update();" value="<#liveupdate#>" />						
					</div>
					<div id="check_beta_div"><input type="checkbox" name="check_beta" id="path_beta" onclick="change_beta_path()" value="" <% nvram_match("webs_update_beta", "1", "checked"); %>/><#FW_beta_check#></div>
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
		<div class="warning_desc aimesh_manual_fw_update_hint" style="display:none;">
			<#FW_note#> <#FW_note_AiMesh#>
		</div>
		
</form>

<form method="post" action="do_modem_fwupgrade.cgi" name="modem_form" target="hidden_frame" enctype="multipart/form-data">
<input type="hidden" name="current_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
		<table id="modem_fw_upgrade" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" style="display: none;">
			<thead>
				<tr>
					<td colspan="2"><#Mobile_modem_fw#></td>
				</tr>
			</thead>
			<tr><th><#Modem_fw_ver#></th><td><div id="usb_modem_act_swver"><% nvram_get("usb_modem_act_swver"); %></div></td></tr>
			<tr>
				<th><#New_modem_fw#></th>
				<td>
					<input type="file" name="file" class="input" style="color:#FFCC00;*color:#000;width: 194px;">
					<input type="button" name="upload" class="button_gen" onclick="upgrade_modem_fw()" value="<#CTL_upload#>" />
				</td>
			</tr>
		</table>
</form>

<form method="post" name="firmware_form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="next_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="flag" value="background">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="saveNvram">
<input type="hidden" name="action_wait" value="1">
<input type="hidden" name="webs_update_enable" value="<% nvram_get("webs_update_enable"); %>">
<input type="hidden" name="webs_update_time" value="<% nvram_get("webs_update_time"); %>">
<input type="hidden" name="webs_update_beta" value="<% nvram_get("webs_update_beta"); %>">
<input type="hidden" name="webs_update_ts" value="<% nvram_get("webs_update_ts"); %>">
</form>		
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

<form method="post" name="start_update" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="next_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="flag" value="liveUpdate">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="webs_update_trigger" value="">
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
<form method="post" name="revert_fw" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="next_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="stop_upgrade;start_revert_fw">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="webs_update_trigger" value="">
</form>
<form method="post" name="revertfw_note" action="/applyapp.cgi" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="next_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="action_mode" value="revertfw_release_note">
<input type="hidden" name="model" value="" disabled>
<input type="hidden" name="version" value="" disabled>
</form>
<form method="post" name="rollback_fw" action="/applyapp.cgi" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="next_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="action_mode" value="rbk_fw">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="model" value="">
<input type="hidden" name="version" value="">
</form>
</body>
</html>
