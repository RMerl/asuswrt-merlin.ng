<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge" />
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - Site Survey</title>
<link rel="stylesheet" type="text/css" href="/form_style.css">
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" href="./mobile/css/qis.css"></link>
<link rel="stylesheet" href="./mobile/css/icon.css"></link>
<style>
p{
	font-weight: bolder;
}
a{
	color: #FFFFFF !important;
}
</style>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/tmmenu.js"></script>
<script language="JavaScript" type="text/javascript" src="/client_function.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>

<script type="text/JavaScript">
var aplist = new Array();
var wlc_scan_state = '<% nvram_get("wlc_scan_state"); %>';
var _wlc_ssid;
var _sw_mode;

var isrescan = 0;

overlib_str_tmp = "";
overlib.isOut = true;

var iserror = 0;
var waitingTime = 120;


function initial(){
	show_menu();

	<%radio_status();%>

	if (radio_2 == 0)
		E("radio2warn").style.display = "";
	if ((band5g_support) && (radio_5 == 0))
		E("radio5warn").style.display = "";

	update_site_info();
	showSiteTable();

}

function doSorter(_flag, _Method, flip){
	if(aplist.length > 1){

		// Set field to sort
		sorter.indexFlag = _flag;

		// Remember data type for this field
		sorter.lastType = _Method;

		// Flip sort order (unless told not to)
		if (flip) sorter.sortingMethod = (sorter.sortingMethod == "increase") ? "decrease" : "increase";

		// doSorter
		eval("aplist.sort(sorter."+_Method+"_"+sorter.sortingMethod+");");
	}

	// show Table
	showSiteTable();
}

// suit for 2 dimention array
var sorter = {
	"sortingMethod" : "decrease",
	"indexFlag" : 5, // default sort is by signal
	"lastType" : "num", // Last data type
	"num_increase" : function(a, b){
		return parseInt(a[sorter.indexFlag]) - parseInt(b[sorter.indexFlag]);
	},
	"num_decrease" : function(a, b){
		return parseInt(b[sorter.indexFlag]) - parseInt(a[sorter.indexFlag]);
	},
	"str_increase" : function(a, b){
		if(a[sorter.indexFlag].toUpperCase() == b[sorter.indexFlag].toUpperCase()) return 0;
		else if(a[sorter.indexFlag].toUpperCase() > b[sorter.indexFlag].toUpperCase()) return 1;
		else return -1;
	},
	"str_decrease" : function(a, b){
		if(a[sorter.indexFlag].toUpperCase() == b[sorter.indexFlag].toUpperCase()) return 0;
		else if(a[sorter.indexFlag].toUpperCase() > b[sorter.indexFlag].toUpperCase()) return -1;
		else return 1;
	}
}
// end

function update_site_info(){
	$.ajax({
		url: '/apscan.asp',
		dataType: 'script',
		error: function(xhr){
			iserror++;
			if(iserror < 2)
				setTimeout("update_site_info();", 1000);
		},
		success: function(response){
			if(wlc_scan_state != 5) {
				setTimeout("update_site_info();", 2000);
			}
			if(isrescan == 0){ // rescan onLoading
				isrescan++;
				rescan();
				wlc_scan_state = 0;
			}
			doSorter(sorter.indexFlag, sorter.lastType, false);
		}
	});
}


function addBorder(field) {
	if (sorter.indexFlag == field) {
		if (sorter.sortingMethod == "decrease")
			return "border-top:1px solid #334044;border-bottom:1px solid #FC0;";
		else
			return "border-top:1px solid #FC0;border-bottom:1px solid #334044;";
	} else {
		return "border-top:1px solid #334044;border-bottom:1px solid #334044;";
	}
}

var htmlCode_tmp = "";
function showSiteTable(){
	var htmlCode = "";
	var overlib_str = "";

	document.getElementById("SearchingIcon").style.display = "none";

	htmlCode +='<table style="width:100%;" border="0" cellspacing="0" cellpadding="4" align="center" class="FormTable_table" id="aplist_table">';

	if(wlc_scan_state != 5){ // on scanning
		htmlCode +='<tr><th style="text-align:center;" colspan="5"><span style="color:#FFCC00;line-height:25px;"><#APSurvey_action_searching_AP#></span>&nbsp;<img style="margin-top:10px;" src="/images/InternetScan.gif"></th></tr>';
	}
	else{ // show ap list
		if ((aplist.length) && (aplist[0].length == 0)) {
			htmlCode +='<tr><td style="text-align:center;" colspan="5"><span style="color:#FFCC00;line-height:25px;"><#APSurvey_action_searching_noresult#></span>&nbsp;<img style="margin-top:10px;" src="/images/InternetScan.gif"></td></tr>';
		}
		else{
			htmlCode += '<tr><th onclick="doSorter(1, \'str\', true);" style="cursor:pointer;' + addBorder(1) + '"><#Wireless_name#></th>';
			htmlCode += '<th onclick="doSorter(2, \'num\', true);" width="15%" style="text-align:center;cursor:pointer;line-height:120%;' + addBorder(2) + '"><#WLANConfig11b_Channel_itemname#></th>';
			htmlCode += '<th onclick="doSorter(3, \'str\', true);" width="27%" style="cursor:pointer;' + addBorder(3) + '"><#QIS_finish_wireless_item2#></th>';
	                htmlCode += '<th onclick="doSorter(0, \'str\', true);" width="10%" style="text-align:center;cursor:pointer;line-height:120%;' + addBorder(0) + '">Band</th>';
			htmlCode += '<th onclick="doSorter(5, \'num\', true);" width="10%" id="sigTh" style="text-align:center;cursor:pointer;' + addBorder(5) + '">Signal</tr>';

			for(var i = 0; i < aplist.length; i++){
				if(aplist[i][1] == null)
					continue;
				else if(aplist[i][1].search("%FFFF") != -1)
					continue;

				overlib_str = "<p><#MAC_Address#>:</p>" + aplist[i][6];

				// initial
				htmlCode += '<tr>';

				//ssid
				ssid_str=decodeURIComponent(handle_show_str(aplist[i][1]));
				htmlCode += '<td id="ssid" onclick="oui_query_full_vendor(\'' + aplist[i][6].toUpperCase() +'\');overlib_str_tmp=\''+ overlib_str +'\';return overlib(\''+ overlib_str +'\');" onmouseout="nd();" style="cursor:pointer; text-decoration:underline;">' + ssid_str + '</td>';

				// channel
				htmlCode += '<td width="15%" style="text-align:center;">' + aplist[i][2] + ' (' + aplist[i][7] + ')</td>';

				// security
				if(aplist[i][3] == "Open System" && aplist[i][4] == "NONE")
					htmlCode += '<td width="27%">' + aplist[i][3] + '<img src="/images/New_ui/networkmap/unlock.png"></td>';
				else if(aplist[i][4] == "WEP")
					htmlCode += '<td width="27%">WEP</td>';
				else
					htmlCode += '<td width="27%">' + aplist[i][3] +' (' + aplist[i][4] + ')</td>';
				// band
				if(aplist[i][0] == "2G")
					htmlCode += '<td width="10%" style="text-align:center;">2.4GHz</td>';
				else
					htmlCode += '<td width="10%" style="text-align:center;">5GHz</td>';

				// signal
				htmlCode += '<td width="10%" style="text-align:center;"><span title="' + aplist[i][5] + '%"><div style="margin-left:13px;"' +
				            'class="icon_wifi_'+ Math.ceil(aplist[i][5]/25) +
				            (aplist[i][4] == "NONE" ? '' : '_lock') +
				            ' ap_icon"></div></span></td></tr>';
			}
			document.form.rescanButton.disabled = false;
			document.form.rescanButton.className = "button_gen";
		}
	}
	htmlCode +='</table>';

	if(htmlCode != htmlCode_tmp){
		document.getElementById("wlscan_table").innerHTML = htmlCode;
		htmlCode_tmp = htmlCode;
	}
}


function rescan(){
	document.form.rescanButton.disabled = true;
	document.form.rescanButton.className = "button_gen_dis";

	isrescan = 120; // stop rescan
	document.getElementById("SearchingIcon").style.display = "";
	document.form.flag.value = "sitesurvey";
	document.form.target = "hidden_frame";
	document.form.action_wait.value	= "1";
	document.form.action_script.value = "restart_wlcscan";
	document.form.submit();
}

</script>
</head>

<body onload="initial();" onunload="">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" action="/start_apply2.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_Wireless_Survey.asp">
<input type="hidden" name="next_page" value="Advanced_Wireless_Survey.asp">
<input type="hidden" name="prev_page" value="Advanced_Wireless_Survey.asp">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="flag" value="sitesurvey">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_wait" value="1">
<input type="hidden" name="action_script" value="restart_wlcscan">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="SystemCmd" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="wl0_ssid" value="<% nvram_char_to_ascii("", "wl0_ssid"); %>" disabled>
<input type="hidden" name="wl1_ssid" value="<% nvram_char_to_ascii("", "wl1_ssid"); %>" disabled>


<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
    <td width="17">&nbsp;</td>
    <td valign="top" width="202">
      <div id="mainMenu"></div>
      <div id="subMenu"></div></td>
    <td valign="top">
        <div id="tabMenu" class="submenuBlock"></div>

      <!--===================================Beginning of Main Content===========================================-->
      <table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
        <tr>
          <td valign="top">
            <table width="760px" border="0" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTitle" id="FormTitle" height="600px">
                <tbody>
                <tr bgcolor="#4D595D">
                <td valign="top">
	                <div>&nbsp;</div>
			<div class="formfonttitle">Wireless - Visible Networks</div>
			<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
			<span style="display:none; color:#FFCC00; padding-right:20px;" id="radio2warn">2.4 GHz radio is disabled - cannot scan that band!</span>
			<span style="display:none; color:#FFCC00;" id="radio5warn">5 GHz radio is disabled - cannot scan that band!</span>

			<div class="apply_gen" valign="top">
				<input disabled type="button" id="rescanButton" value="<#QIS_rescan#>" onclick="rescan();" class="button_gen_dis">
				<img id="SearchingIcon" style="display:none;" src="/images/InternetScan.gif">
			</div>
			<div id="wlscan_table" style="overflow-y:auto;height:650px;margin-left:10px;margin-right:10px;margin-top:20px;vertical-align:top;"></div>
		</td>
		</tr>
	        </tbody>
            </table>
            </form>
            </td>

       </tr>
      </table>
      <!--===================================Ending of Main Content===========================================-->
    </td>
    <td width="10" align="center" valign="top">&nbsp;</td>
  </tr>
</table>
<div id="footer"></div>
</body>
</html>
