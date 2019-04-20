<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>

<!--Feedback_Info.asp-->
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="/images/favicon.png">
<link rel="icon" href="/images/favicon.png">
<title><#Web_Title#> - <#menu_feedback#></title>
<link rel="stylesheet" type="text/css" href="/index_style.css">
<link rel="stylesheet" type="text/css" href="/form_style.css">
<link rel="stylesheet" type="text/css" href="/other.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script>
var fb_state = httpApi.nvramGet(["fb_state"], true).fb_state;

var firmver = httpApi.nvramGet(["firmver"], true).firmver;
var buildno = httpApi.nvramGet(["buildno"], true).buildno;
var extendno = httpApi.nvramGet(["extendno"], true).extendno;
var fb_split_files = Number(httpApi.nvramGet(["fb_split_files"], true).fb_split_files);

var FWString = '';
FWString = firmver+"."+buildno;
FWString += "_"+extendno;


function initial(){
	show_menu();
	check_info();
	$("#bind_google")
		.attr('target','_self')
		.attr("href", "Advanced_Feedback.asp?provider=google&reload=1")
		.attr("style", "text-decoration:underline;color:#FFCC00;");
}

function check_info(){
	//0:initial  1:Success  2.Failed  3.Limit?  4.dla
	if(wan_diag_state == "4"){	
		document.getElementById("fb_send_debug_log").style.display = "";
		document.getElementById("Email_subject").href = "mailto:xdsl_feedback@asus.com?Subject="+based_modelid;
		get_debug_log_info();
	}
	else{
		if(dsl_support){
			document.getElementById("fb_success_dsl_0").style.display = "";
			document.getElementById("fb_success_dsl_1").style.display = "";
		}
		else{
			document.getElementById("fb_success_router_0").style.display = "";
			document.getElementById("fb_success_router_1").style.display = "";
		}
	} 	

	if(dsl_support && fb_state == "2"){
		document.getElementById("fb_fail_dsl").style.display = "";
		document.getElementById("fb_fail_textarea").style.display = "";
		show_dbg_files(fb_split_files, "dsl");
	}
	else if(fb_state == "2"){
		document.getElementById("fb_fail_router").style.display = "";
		document.getElementById("fb_fail_textarea").style.display = "";
		show_dbg_files(fb_split_files, "rt");
	}
}

function show_dbg_files(seg, type){
	if(type == "dsl"){

		switch(seg){

			case 1:
				document.getElementById("dbg_dsl_file").style.display = "";
				break;
			case 2:
				document.getElementById("dbg_dsl_seg_a").style.display = "";
				document.getElementById("dbg_dsl_seg_b").style.display = "";
				break;
			case 3:
				document.getElementById("dbg_dsl_seg_a").style.display = "";
				document.getElementById("dbg_dsl_seg_b").style.display = "";
				document.getElementById("dbg_dsl_seg_c").style.display = "";
				break;
			case 4:
				document.getElementById("dbg_dsl_seg_a").style.display = "";
				document.getElementById("dbg_dsl_seg_b").style.display = "";
				document.getElementById("dbg_dsl_seg_c").style.display = "";
				document.getElementById("dbg_dsl_seg_d").style.display = "";
				break;
			default:
				document.getElementById("dbg_dsl_file").style.display = "";
				break;
		}
	}
	else{

		switch(seg){

			case 1:
				document.getElementById("dbg_rt_file").style.display = "";
				break;
			case 2:
				document.getElementById("dbg_rt_seg_a").style.display = "";
				document.getElementById("dbg_rt_seg_b").style.display = "";
				break;
			case 3:
				document.getElementById("dbg_rt_seg_a").style.display = "";
				document.getElementById("dbg_rt_seg_b").style.display = "";
				document.getElementById("dbg_rt_seg_c").style.display = "";
				break;
			case 4:
				document.getElementById("dbg_rt_seg_a").style.display = "";
				document.getElementById("dbg_rt_seg_b").style.display = "";
				document.getElementById("dbg_rt_seg_c").style.display = "";
				document.getElementById("dbg_rt_seg_d").style.display = "";
				break;
			default:
				document.getElementById("dbg_rt_file").style.display = "";
				break;
		}
	}
}

function get_debug_log_info(){

	var desc = "DSL DIAGNOSTIC LOG\n";
	desc += "----------------------------------------------------------------------\n";

	desc += "Model: "+based_modelid+"\n";
	desc += "Firmware Version: "+FWString+"\n";
	desc += "Inner Version: <% nvram_get("innerver"); %>\n";
	desc += "DSL Firmware Version: <% nvram_get("dsllog_fwver"); %>\n";
	desc += "DSL Driver Version:  <% nvram_get("dsllog_drvver"); %>\n\n";

	desc += "PIN Code: <% nvram_get("secret_code"); %>\n";
	desc += "MAC Address: <% nvram_get("lan_hwaddr"); %>\n\n";

	desc += "<#feedback_capturing_duration#>: <% nvram_get("dslx_diag_duration"); %>\n";
	desc += "DSL connection: <% nvram_get("fb_availability"); %>\n";

	document.uiForm.fb_send_debug_log_content.value = desc;
	
}

function redirect(){
	document.location.href = "Advanced_Feedback.asp";
}

function reset_diag_state(){	
	document.diagform.dslx_diag_state.value = 0;	
	document.diagform.submit();
	setTimeout("location.href='TCC.log.gz';", 300);
}

function get_feedback_tarball(){
	setTimeout("location.href='fb_data.tgz.gz';", 300);
}

function get_split_feedback(seg){
	switch(seg) {
		case 1:
			setTimeout("location.href='fb_data.tgz.gz';", 300);
			break;
		case "a":
	setTimeout("location.href='fb_data.tgz.gz.part.a';", 300);
			break;
		case "b":
	setTimeout("location.href='fb_data.tgz.gz.part.b';", 300);
			break;
		case "c":
	setTimeout("location.href='fb_data.tgz.gz.part.c';", 300);
			break;
		case "d":
	setTimeout("location.href='fb_data.tgz.gz.part.d';", 300);
			break;
		default:
			setTimeout("location.href='fb_data.tgz.gz';", 300);
			break;
	}
}

</script>
<style>
.feedback_info_0{
	font-size:20px;
	margin-left:25px;
	text-shadow: 1px 1px 0px black;
	font-family: Arial, Helvetica, sans-serif;
	font-weight: bolder;
}

.feedback_info_1{
	font-size:13px;
	margin-left:30px;
	font-family: Arial, Helvetica, sans-serif;
}
</style>	
</head>

<body onload="initial();" onunLoad="return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="diagform" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="dslx_diag_state" value="<% nvram_get("dslx_diag_state"); %>">
</form>
<FORM METHOD="POST" ACTION="" name="uiForm" target="hidden_frame">
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
<table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
<tbody>
<tr>
<td bgcolor="#4D595D" valign="top">
<div>&nbsp;</div>
		  <div class="formfonttitle"><#menu5_6#> - <#menu_feedback#></div>
<div style="margin:10px 0 10px 5px;" class="splitLine"></div>

<div id="fb_success_dsl_0" style="display:none;">
	<br>
	<br>
	<div class="feedback_info_0"><#feedback_thanks#></div>
	<br>
</div>

<div id="fb_success_router_0" style="display:none;">
        <br>
        <br>
        <div class="feedback_info_0"><#feedback_thanks#></div>
        <br>
</div>

<div id="fb_fail_dsl" style="display:none;" class="feedback_info_1">
	<#feedback_fail0#>
	<br><br>
	<#feedback_fail1#> : ( <a href="mailto:xdsl_feedback@asus.com?Subject=<%nvram_get("productid");%>" target="_top" style="color:#FFCC00;">xdsl_feedback@asus.com </a>) <#feedback_fail2#>
	<br>
	<#feedback_fail3#> :
	<br>
	<ul>
		<li id="dbg_dsl_file" style="display:none;"><span onClick="get_split_feedback(1);" style="text-decoration: underline; color:#FFCC00; cursor:pointer;">debug file</span></li>
		<li id="dbg_dsl_seg_a" style="display:none;"><span onClick="get_split_feedback('a');" style="text-decoration: underline; color:#FFCC00; cursor:pointer;">get_split_feedback_a</span></li>
		<li id="dbg_dsl_seg_b" style="display:none;"><span onClick="get_split_feedback('b');" style="text-decoration: underline; color:#FFCC00; cursor:pointer;">get_split_feedback_b</span></li>
		<li id="dbg_dsl_seg_c" style="display:none;"><span onClick="get_split_feedback('c');" style="text-decoration: underline; color:#FFCC00; cursor:pointer;">get_split_feedback_c</span></li>
		<li id="dbg_dsl_seg_d" style="display:none;"><span onClick="get_split_feedback('d');" style="text-decoration: underline; color:#FFCC00; cursor:pointer;">get_split_feedback_d</span></li>
	</ul>
</div>

<div id="fb_fail_router" style="display:none;" class="feedback_info_1">
	<#feedback_fail0#>
	<br><br>
	<#feedback_fail1#> : ( <a href="mailto:router_feedback@asus.com?Subject=<%nvram_get("productid");%>" target="_top" style="color:#FFCC00;">router_feedback@asus.com </a>) <#feedback_fail2#>
	&nbsp;<#feedback_fail_BindGoogle#>
	<br>
	<#feedback_fail3#> :
	<br>
	<ul>
		<li id="dbg_rt_file" style="display:none;"><span onClick="get_split_feedback(1);" style="text-decoration: underline; color:#FFCC00; cursor:pointer;">debug file</span></li>
		<li id="dbg_rt_seg_a" style="display:none;"><span onClick="get_split_feedback('a');" style="text-decoration: underline; color:#FFCC00; cursor:pointer;">get_split_feedback_a</span></li>
		<li id="dbg_rt_seg_b" style="display:none;"><span onClick="get_split_feedback('b');" style="text-decoration: underline; color:#FFCC00; cursor:pointer;">get_split_feedback_b</span></li>
		<li id="dbg_rt_seg_c" style="display:none;"><span onClick="get_split_feedback('c');" style="text-decoration: underline; color:#FFCC00; cursor:pointer;">get_split_feedback_c</span></li>
		<li id="dbg_rt_seg_d" style="display:none;"><span onClick="get_split_feedback('d');" style="text-decoration: underline; color:#FFCC00; cursor:pointer;">get_split_feedback_d</span></li>
	</ul>
</div>

<div id="fb_fail_textarea" style="display:none;">
	<textarea name="fb_fail_content" cols="70" rows="10" style="width:90%;margin-left:25px;font-family:'Courier New', Courier, mono; font-size:13px;background:#475A5F;color:#FFFFFF;" readonly><% nvram_dump("fb_fail_content", ""); %></textarea>
	<br>
</div>

<div id="fb_success_dsl_1" style="display:none;">
	<br>
	<div class="feedback_info_1">We are working hard to improve the firmware of <#Web_Title2#> and your feedback is very important to us. We will use your feedbacks and comments to strive to improve your ASUS experience.</div>
	<br>	
</div>

<div id="fb_success_router_1" style="display:none;">	
	<br>
	<div class="feedback_info_1"> 
	<#feedback_success_rt#>
	</div>
	<br>
	<br>
</div>	

<div id="fb_send_debug_log" style="display:none;">
	<br>
	<br>
	<div class="feedback_info_0">The debug log of diagnostic DSL captured.</div>
	<br>
	<br>
	<div class="feedback_info_1">Please send us an email directly ( <a id="Email_subject" href="" target="_top" style="color:#FFCC00;">xdsl_feedback@asus.com</a> ). Simply copy from following text area and paste as mail content. <br><div onClick="reset_diag_state();" style="text-decoration: underline; font-family:Lucida Console; cursor:pointer;">Click here to download the debug log and add as mail attachment.</div></div>
	<br>
	<textarea name="fb_send_debug_log_content" cols="70" rows="15" style="width:90%; margin-left:25px; font-family:'Courier New', Courier, mono; font-size:13px;background:#475A5F;color:#FFFFFF;" readonly></textarea>
	<br>	
</div>

<div id="fb_deny" style="display:none;">
</div>	<div class="apply_gen">
			<input class="button_gen" onclick="redirect();" type="button" value="<#Main_alert_proceeding_desc3#>"/>
</div>
</td>
</tr>
</tbody>
</table>
</td>
</form>
</tr>
</table>
</td>
<td width="10" align="center" valign="top">&nbsp;</td>
</tr>
</table>
<div id="footer"></div>
</body>
</html>

