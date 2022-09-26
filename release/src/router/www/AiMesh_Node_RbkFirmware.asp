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
<title><#Web_Title#> - Rollback Firmware</title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="css/confirm_block.css">
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/httpApi.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/confirm_block.js"></script>
<style>
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

.clientlist_viewlist {
        position: absolute;
        -webkit-border-radius: 5px;
        -moz-border-radius: 5px;
        border-radius: 5px;
        z-index: 200;
        background-color:#444f53;
        margin-left: 140px;
        height:auto;
        box-shadow: 3px 3px 10px #000;
        display:block;
        overflow: auto;
}
#rbk_Block{
	font-family: Arial, Helvetica, MS UI Gothic, MS P Gothic, Microsoft Yahei UI, sans-serif;
	/*width:800px;*/
	margin-top: 0px;
}
.rbk_title{
	margin-top:15px;
	margin-left:15px;
	float:left;
	font-size:18px;
	color:#93A9B1;
}

.current_fw_release_note {
		color: #FC0;
        text-decoration: underline;
        cursor: pointer;
}
.rbk_close{
	float:right;
}
.td_rbk_fwver{
	text-decoration: underline;
	cursor: pointer;	
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
var rbkfw_support = ('<% nvram_get("rc_support"); %>'.indexOf(' rbkfw') != -1) ? true : false;

var rbk_count = 0;
var _rollback_info = 0;
if(rbkfw_support){
	_rollback_info = JSON.parse('<% get_rbkList(); %>');
	//_rollback_info =  JSON.parse('{"0":"3004_386_46092-g0d2214a","1":"3004_386_45956-g23134c9","2":"3004_386_44470-g53bccb8","3":"3004_386_42643-g16dc577","4":"3004_386_41634-g08f88ae","5":"3004_384_82211-gd4e86b8"}');
	rbk_count = Object.keys(_rollback_info).length;
}


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

var frsmodel =  (httpApi.nvramGet(["webs_state_odm"]).webs_state_odm != "")? httpApi.nvramGet(["webs_state_odm"]).webs_state_odm:httpApi.nvramGet(["productid"]).productid;
var firmver_org =  httpApi.nvramGet(["firmver_org"]).firmver_org;
var buildno_org =  httpApi.nvramGet(["buildno_org"]).buildno_org;
var extendno_org =  httpApi.nvramGet(["extendno_org"]).extendno_org;
var RevertFWver = '';

function show_rbk_selector(){

	if($("#rbk_Block"))
		$("#rbk_Block").remove();
	$("html, body").animate({ scrollTop: 0 }, "fast");

	var divObj = document.createElement("div");
	divObj.setAttribute("id","rbk_Block");
	divObj.className = "clientlist_viewlist";
	document.body.appendChild(divObj);
	//document.getElementsByClassName("AiMesh_revertfw_bg")[0].appendChild(divObj);
	//cal_panel_block("rbk_Block", 0.2);

	create_rbkfw_view();
}
function create_rbkfw_view(){

	var divObj = document.createElement("div");
	divObj.id = "div_rbk_title";
	divObj.className = "rbk_title";
	divObj.innerHTML = "Firmware Rollback Selector";	//Untranslated
	$(divObj).appendTo('#rbk_Block');

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
	theadObj.innerHTML = "<tr height='28px'><td id='td_all_list_title' colspan='2'>Choose a firmware version</td></tr>";	/* Untranslated */
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
			{"model_name": httpApi.nvramGet(["webs_state_odm"],1).webs_state_odm, "fwver": _rollback_info[j] }, show_current_release_note
		);
	}


	var divObj_bot = document.createElement("div");
	divObj_bot.id = "div_rbk_bot";
	divObj_bot.style = "margin-top:10px;margin-bottom:20px;width:100%;text-align:center;";
	divObj_bot.innerHTML = "<input class='button_gen' type='button' onclick='close_rbk_selector()' value='<#CTL_Cancel#>'><input class='button_gen' type='button' onclick='rbk_fw_confirm()' style='margin-left:15px;' value='Rollback'>";
	$(divObj_bot).appendTo('#rbk_Block');

}
function close_rbk_selector(){
	Window.close();
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
		note_display_flag: "0"
	});

	$(".confirm_block").css( "zIndex", 10001 );
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
function rbk_fw_confirm(){
	if($('input[name=rbk_opt]:checked').val() != undefined){
		document.rollback_fw.model.value = httpApi.nvramGet(["webs_state_odm"],1).webs_state_odm;
		document.rollback_fw.version.value = $('input[name=rbk_opt]:checked').val();
		$("#rbk_Block").fadeOut();
		//$("#Loading").css('visibility', 'hidden');
		startRevertDownloading();
		document.rollback_fw.submit();
	}
	else{
		alert("<#JS_fieldblank#>");
		return;
	}
}
function initial() {
	if(odm_support)
		document.body.className = "bg-odm";
	else
		document.body.className = "bg";

	if(rbkfw_support && rbk_count > 0 && httpApi.nvramGet(["webs_state_odm"],1).webs_state_odm != 0 && httpApi.nvramGet(["webs_state_odm"],1).webs_state_odm != ""){
		show_rbk_selector();
	}
	else{
		$('#desc').html("Firmware rollback is currently unavailable.");
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
			if(location.pathname.indexOf("AiMesh_Node_RbkFirmware") < 0) {
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
								document.rollback_fw.flag.value="";
								document.rollback_fw.action_mode.value="apply";
								document.rollback_fw.action_script.value="reboot";
								document.rollback_fw.submit();
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

<form method="post" name="rollback_fw" action="applyapp.cgi" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="AiMesh_Node_RbkFirmware.asp">
<input type="hidden" name="next_page" value="AiMesh_Node_RbkFirmware.asp">
<input type="hidden" name="action_mode" value="rbk_fw">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="model" value="">
<input type="hidden" name="version" value="">
<div class='AiMesh_revertfw_bg'>
	<div id="desc" class="desc_container"></div>
<div>
</form>
<form method="post" name="revertfw_note" action="applyapp.cgi" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="AiMesh_Node_RbkFirmware.asp">
<input type="hidden" name="next_page" value="AiMesh_Node_RbkFirmware.asp">
<input type="hidden" name="action_mode" value="revertfw_release_note">
<input type="hidden" name="model" value="" disabled>
<input type="hidden" name="version" value="" disabled>
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
