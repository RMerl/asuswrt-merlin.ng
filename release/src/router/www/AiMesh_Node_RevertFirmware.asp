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
<title><#Web_Title#> - Revert Firmware</title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="css/confirm_block.css">
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/confirm_block.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/httpApi.js"></script>
<style>
.confirm_block{
	position:absolute;
	width:100%;
	top:5%;
	left:0%;
	text-align:center;
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

.desc_container {
	margin-top: 2%;
	font-family: Helvetica, Arial;
	font-size: 16pt;
	color: #EEE;
	text-align: left;
	font-weight: bolder;
}
.AiMesh_revertfw_bg {
	position: absolute;
	right: 0;
	top: 0;
	left: 0;
	bottom: 0;
	margin: auto;
	width: 80%;
	height: 90%;
}
.AiMesh_fw_loading {
	position: relative;
	top: 20%;
	margin: auto;
}
#desc {
	font-size:18px; color:#FFFFFF;
	line-height:21px;
}

/*for mobile device*/
@media screen and (orientation: portrait) and (max-width: 1000px) {
	.desc_container {
		width: 50%;
		font-size: 5vw;
		text-align: center;
		margin: 2vh auto;
	}
}
@media screen and (orientation: landscape) and (max-width: 1000px) {
	.desc_container {
		width: 50%;
		font-size: 7vw;
		text-align: center;
		margin: 2% auto;
	}
}
</style>
<script>
function isSupport(_ptn){
	var ui_support = [<% get_ui_support(); %>][0];
	return (ui_support[_ptn]) ? ui_support[_ptn] : 0;
}
var odm_support = isSupport("odm");

var webs_state_update = '<% nvram_get("webs_state_update"); %>';
var webs_state_upgrade = '<% nvram_get("webs_state_upgrade"); %>';
var webs_state_error = '<% nvram_get("webs_state_error"); %>';

var pipefw_support = ('<% nvram_get("rc_support"); %>'.indexOf(' pipefw') != -1) ? true : false;
var urlfw_support = ('<% nvram_get("rc_support"); %>'.indexOf(' urlfw') != -1) ? true : false;
var cfg_sync_support = ('<% nvram_get("rc_support"); %>'.indexOf(' cfg_sync') != -1) ? true : false;

var based_modelid = httpApi.nvramGet(["productid"]).productid;
var odmpid = httpApi.nvramGet(["odmpid"]).odmpid;
var support_site_modelid = (odmpid == "")? based_modelid : odmpid;

var webs_state_info = httpApi.nvramGet(["webs_state_info"]).webs_state_info;
var webs_state_REQinfo = httpApi.nvramGet(["webs_state_REQinfo"]).webs_state_REQinfo;
var webs_state_info_beta = httpApi.nvramGet(["webs_state_info_beta"]).webs_state_info_beta;

var firmver = httpApi.nvramGet(["firmver"]).firmver;
var buildno = httpApi.nvramGet(["buildno"]).buildno;
var extendno = httpApi.nvramGet(["extendno"]).extendno;
var FWString = '';
FWString = firmver+"."+buildno;
FWString += "_"+extendno;

var frsmodel =  (httpApi.nvramGet(["webs_update_odm"]).webs_update_odm != "")? httpApi.nvramGet(["webs_update_odm"]).webs_update_odm:httpApi.nvramGet(["productid"]).productid;
var firmver_org =  httpApi.nvramGet(["firmver_org"]).firmver_org;
var buildno_org =  httpApi.nvramGet(["buildno_org"]).buildno_org;
var extendno_org =  httpApi.nvramGet(["extendno_org"]).extendno_org;
var RevertFWver = '';
var isSame_org = false;
var isUnderREQ = false;
if(firmver_org!="" && buildno_org!="" && extendno_org!=""){
	RevertFWver = firmver_org+"."+buildno_org+"_"+extendno_org;
	//alert(RevertFWver +" | "+ FWString);
	isSame_org = (RevertFWver==FWString)? true:false;
	isUnderREQ = checkUnderREQ();
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

var note_display=0;
function show_revertfw_release_note() {
	var notice="";
	//alert(event.data.model_name+"/"+event.data.newfwver);
	if($(".confirm_block").length > 0)
		$(".confirm_block").remove();

	//document.revertfw_note.model.value = event.data.model_name+"test";
	document.revertfw_note.model.value = frsmodel;
	document.revertfw_note.version.value = RevertFWver;
	document.revertfw_note.submit();

	notice = "<#FW_note#><br><br>If you would like to revert to previous firmware version, we suggest you temporarily disable auto firmware update to make sure you grant every upgrade.";		//Untranslated
	
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
		margin: "25px 0px 0px 25px",
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

function initial() {
	if(odm_support)
		document.body.className = "bg-odm";
	else
		document.body.className = "bg";

	if(RevertFWver != "" && !isSame_org && !isUnderREQ){
		show_revertfw_release_note();
	}
	else{
		$('#desc').html("Previous version is currently unavailable.");
	}
}
function dr_advise() {
	htmlbodyforIE = document.getElementsByTagName("html");  //this both for IE&FF, use "html" but not "body" because <!DOCTYPE html PUBLIC.......>
	htmlbodyforIE[0].style.overflow = "hidden";	  //hidden the Y-scrollbar for preventing from user scroll it.

	document.getElementById("hiddenMask").style.width = "100%";
	document.getElementById("hiddenMask").style.height = "100%";
	document.getElementById("hiddenMask").style.visibility = "visible";
}
function cancel_dr_advise(){
	document.getElementById("hiddenMask").style.visibility = "hidden";
	htmlbodyforIE = parent.document.getElementsByTagName("html");  //this both for IE&FF, use "html" but not "body" because <!DOCTYPE html PUBLIC.......>
	htmlbodyforIE[0].style.overflow = "scroll";	  //hidden the Y-scrollbar for preventing from user scroll it.	
	window.scrollTo(0, 0);//x-axis , y-axis	
}

function LoadingProgress(seconds) {
	document.getElementById("LoadingBar").style.visibility = "visible";
	y = y + progress;
	if(typeof(seconds) == "number" && seconds >= 0){
		if(seconds != 0){
			document.getElementById("proceeding_img").style.width = Math.round(y) + "%";
			document.getElementById("proceeding_img_text").innerHTML = Math.round(y) + "%";

			if(document.getElementById("loading_block1")){
				document.getElementById("proceeding_img_text").style.width = document.getElementById("loading_block1").clientWidth;
				document.getElementById("proceeding_img_text").style.marginLeft = "175px";
			}
			--seconds;
			setTimeout("LoadingProgress("+seconds+");", 1000);
		}
		else{
			document.getElementById("proceeding_img_text").innerHTML = "<#Main_alert_proceeding_desc3#>";
			y = 0;
			if(location.pathname.indexOf("AiMesh_Node_RevertFirmware") < 0) {
				setTimeout("hideLoadingBar();",1000);
				window.open(location, '_self').close();
			}
		}
	}
}

function showLoadingBar(seconds) {
	if(window.scrollTo)
		window.scrollTo(0,0);
	
	htmlbodyforIE = document.getElementsByTagName("html");  //this both for IE&FF, use "html" but not "body" because <!DOCTYPE html PUBLIC.......>
	htmlbodyforIE[0].style.overflow = "hidden";	  //hidden the Y-scrollbar for preventing from user scroll it.

	document.getElementById("LoadingBar").style.width = "100%";
	document.getElementById("LoadingBar").style.height = "100%";

	loadingSeconds = seconds;
	progress = 100/loadingSeconds;
	y = 0;
	LoadingProgress(seconds);
}
function hideLoadingBar(){
	document.getElementById("LoadingBar").style.visibility = "hidden";
}

var dead = 0;
function detect_httpd() {
	$.ajax({
		url: '/httpd_check.xml',
		dataType: 'xml',
		timeout: 1500,
		error: function(xhr) {
			if(dead > 5) {
				document.getElementById('loading_block1').style.display = "none";
				document.getElementById('loading_block2').style.display = "none";
				document.getElementById('loading_block3').style.display = "";
				document.getElementById('loading_block3').innerHTML = "<div><#FIRM_ok_aimesh#></div>";
				document.getElementById('tr_closeWindow').style.display = "";
			}
			else {
				dead++;
			}
			setTimeout("detect_httpd();", 1000);
		},

		success: function(){
			document.getElementById('loading_block1').style.display = "none";
			document.getElementById('loading_block2').style.display = "none";
			document.getElementById('loading_block3').style.display = "";
			document.getElementById('loading_block3').innerHTML = "<div><#FIRM_ok_aimesh#></div>";
			document.getElementById('tr_closeWindow').style.display = "";
		}
	});
}
function closeWindow() {
	window.open(location, '_self').close();
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
								document.revert_fw.flag.value="";
								document.revert_fw.action_mode.value="apply";
								document.revert_fw.action_script.value="reboot";
								document.revert_fw.submit();
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
	//disableCheckChangedStatus();			
	dr_advise();
	document.getElementById("drword").innerHTML = "&nbsp;&nbsp;&nbsp;<#fw_downloading#>...";
	isRevertDownloading();
}
</script>
</head>
<body onload="initial();">
<div id="LoadingBar" class="popup_bar_bg">
<table cellpadding="5" cellspacing="0" id="loadingBarBlock" class="loadingBarBlock AiMesh_fw_loading" align="center">
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
	<tr id="tr_closeWindow" style="display:none;text-align:center;">
		<td>
			<input type="button" class="button_gen" style="margin-bottom:10px;" onclick="closeWindow()" value="<#CTL_close#>" />
		</td>
	</tr>
</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>
<div id="Loading" class="popup_bg"></div><!--for uniform show, useless but have exist-->

<div id="hiddenMask" class="popup_bg">
	<table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise AiMesh_fw_loading" align="center" style="height:100px;">
		<tr>
		<td>
			<div class="drword" id="drword" style="">&nbsp;&nbsp;&nbsp;&nbsp;<#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...</div>
		</td>
		</tr>
	</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="revert_fw" action="start_apply.htm" target="hidden_frame"><!-- enctype="multipart/form-data"-->
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="AiMesh_Node_RevertFirmware.asp">
<input type="hidden" name="next_page" value="AiMesh_Node_RevertFirmware.asp">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="start_revert_fw">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="webs_update_trigger" value="">
<div class='AiMesh_revertfw_bg'>
	<div id="desc" class="desc_container"></div>
<div>
</form>
<form method="post" name="revertfw_note" action="applyapp.cgi" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="AiMesh_Node_RevertFirmware.asp">
<input type="hidden" name="next_page" value="AiMesh_Node_RevertFirmware.asp">
<input type="hidden" name="action_mode" value="revertfw_release_note">
<input type="hidden" name="model" value="" disabled>
<input type="hidden" name="version" value="" disabled>
</form>
</body>
</html>
