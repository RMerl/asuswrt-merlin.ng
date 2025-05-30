﻿<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - <#ipv6_info#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="/js/table/table.css">
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/client_function.js"></script>
<script type="text/javascript" src="/js/table/table.js"></script>

<style>
p{
	font-weight: bolder;
}
.tableApi_table th {
	height: 20px;
	text-align:left;
	padding-left:20px;
}
.tableApi_table td {
	text-align: left;
	padding-left:20px;
}
.data_tr {
	height: 30px;
}
</style>


<script>

<% ipv6_pinholes(); %>
<% get_ipv6net_array(); %>

overlib_str_tmp = "";
overlib.isOut = true;

function initial() {
	show_menu();

	show_ipv6config();
	if (ipv6cfgarray.length > 1)
	{
		show_ipv6clients();
	}

	if (igd2_support)
	{
		document.getElementById("pinholesdiv").style.display="";
		show_pinholes();
	}
}


function show_ipv6config() {
	var code, i, line

	code = '<table width="100%" id="ipv6config" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">';
	code += '<thead><tr><td colspan="2">IPv6 Configuration</td></tr></thead>';

	if (ipv6cfgarray.length > 1) {
		for (i = 0; i < ipv6cfgarray.length-1; ++i) {
			line = ipv6cfgarray[i];
                        code += '<tr><th>' + line[0] + '</th>';
			code += '<td>' + line[1] + '</td>';
			code += '</tr>';
		}
	} else {
		code += '<tr><td colspan="2"><span>IPv6 Not enabled.</span></td></tr>';
	}

	code += '</tr></table>';
	document.getElementById("ipv6configblock").innerHTML = code;
}


function show_ipv6clients() {
	var i, tableStruct;

	ipv6clientarray.pop();	// Remove last empty element

	if (ipv6clientarray.length > 1) {
		for (i = 0; i < ipv6clientarray.length; ++i) {
			ipv6clientarray[i][0] = htmlEnDeCode.htmlEncode(ipv6clientarray[i][0]);
			overlib_str = "<p><#MAC_Address#>:</p>" + ipv6clientarray[i][1];
			ipv6clientarray[i][1] = '<span class="ClientName" onclick="oui_query_full_vendor(\'' + ipv6clientarray[i][1].toUpperCase() +'\');overlib_str_tmp=\''+ overlib_str +'\';return overlib(\''+ overlib_str +'\');" onmouseout="nd();" style="cursor:pointer; text-decoration:underline;">'+ ipv6clientarray[i][1].toUpperCase() +'</span>';
		}

		tableStruct = {
			data: ipv6clientarray,
			container: "ipv6clientsblock",
			title: "IPv6 Clients",
			header: [
				{
					"title" : "Hostname",
					"width" : "30%",
					"sort" : "str",
					"defaultSort" : "increase"
				},
				{
					"title" : "MAC Address",
					"width" : "20%",
					"sort" : "str"
				},
				{
					"title" : "IP Address",
					"width" : "50%",
					"sort" : "str"
				}
				]
		}
		tableApi.genTableAPI(tableStruct);
	} else {
		document.getElementById("ipv6clientsblock").innerHTML = '<span style="color:#FFCC00;padding-left:12;">No IPv6 clients.</span>';
	}
}


function show_pinholes() {
	var code, i, line;

	code = '<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"class="FormTable_table">';
	code += '<thead><tr><td colspan="5">IPv6 firewall Pinholes</td></tr></thead>';
	code += '<tr><th width="10%">Proto</th>';
	code += '<th width="35%">Remote IP</th>';
	code += '<th width="10%">Port</th>';
	code += '<th width="35%">Local IP</th>';
	code += '<th width="10%">Port</th>';
	code += '</tr>';

	if ("<% nvram_get("upnp_pinhole_enable"); %>" == "0") {
		code += '<tr><td colspan="5">Pinhole support is currently disabled.</td></tr>';
	} else {
		if (pinholesarray.length > 1) {
			for (i = 0; i < pinholesarray.length-1; ++i) {
				line = pinholesarray[i];
				code += '<tr>';
				code += '<td style="color: cyan;text-align:left;" colspan="5">' + line[6] + '</td>';
				code += '</tr><tr>';
				code += '<td>' + line[0] + '</td>';
				code += '<td>' + line[1] + '</td>';
				code += '<td>' + line[2] + '</td>';
				code += '<td>' + line[3] + '</td>';
				code += '<td>' + line[4] + '</td>';
				code += '</tr>';
			}
		} else {
			code += '<tr><td colspan="5"><span>No pinholes.</span></td></tr>';
		}
	}

	code += '</tr></table>';
	document.getElementById("pinholesblock").innerHTML = code;
}

</script>
</head>

<body onload="initial();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" action="apply.cgi" target="hidden_frame">
<input type="hidden" name="current_page" value="Main_IPV6Status_Content.asp">
<input type="hidden" name="next_page" value="Main_IPV6Status_Content.asp">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">

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
							<table width="760px" border="0" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTitle" id="FormTitle">
								<tbody>
								<tr bgcolor="#4D595D">
									<td valign="top">
										<div>&nbsp;</div>
										<div class="formfonttitle"><#System_Log#> - <#ipv6_info#></div>
										<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
										<div class="formfontdesc"><#ipv6_info_desc#></div>
                                                                        
										<div style="margin-top:8px">
											<div id="ipv6configblock"></div>
										</div>
										<br>
										<div style="margin-top:8px">
											<div id="ipv6clientsblock"></div>
										</div>
										<br>
										<div id="pinholesdiv" style="display:none;">
											<div id="pinholesblock"></div>
										</div>
										<br><br>
										<div class="apply_gen">
											<input type="button" onClick="location.reload();" value="<#CTL_refresh#>" class="button_gen">
										</div>
									</td><!--==magic 2008.11 del name ,if there are name, when the form was sent, the textarea also will be sent==-->
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
</body>
</html>
