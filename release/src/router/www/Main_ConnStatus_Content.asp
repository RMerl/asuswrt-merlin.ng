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
<title><#Web_Title#> - Connections</title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="/js/table/table.css">
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/client_function.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/table/table.js"></script>
<script>

var connarray = Array();
var connarray_route = Array();

<% get_connlist_array(); %>
<% get_ipv6clients_array(); %>

var sortdir_nat = 0;
var sortfield_nat = 0;
var sortdir_route = 0;
var sortfield_route = 0;
var filter = [];
filter["nat"] = Array(6);
filter["route"] = Array(6);
var sortfield_global = 0;

function initial() {
	show_menu();

// Remove last empty elements of each array
	connarray.pop();
	connarray_route.pop();

	draw_table("nat");
	draw_table("route");
}

function compIPV6(input) {
	input = input.replace(/\b(?:0+:){2,}/, ':');
	return input.replace(/(^|:)0{1,4}/g, ':');
}

function set_filter(field, o, type) {
	filter[type][field] = o.value.toLowerCase();
	draw_table(type);
}

function draw_table(type){
	var i, j;
	var tracklen, shownlen = 0;
	var code;
	var clientObj, clientName;
	var srchost, dsthost;

	if (type == "nat") {
		dataarray = connarray;
		sortfield_global = sortfield_nat;
		sortdir_global = sortdir_nat;
	} else if (type == "route") {
		dataarray = connarray_route;
		sortfield_global = sortfield_route;
		sortdir_global = sortdir_route;
	} else
		return;

	tracklen = dataarray.length

	if (type == "nat") {
		if (tracklen == 0) {
			document.getElementById("connblock_nat").innerHTML = '<div class="hint-color" style="font-size: 125%;padding-top:50px;">No active NAT connections.</div>';
			return;
		}
		document.getElementById('tracked_filters').style.display = "";
		document.getElementById('connblock_header').style.display = "";
		document.getElementById('connblock_nat').innerHTML = "";
		code = '<table width="100%" cellpadding="4" class="FormTable_table"><thead><tr><td colspan="6">NAT Connections</td></tr></thead>' +
		       '<tr><th width="8%" id="track_header_0" style="cursor: pointer;" onclick="setsort(0,\'nat\'); draw_table(\'nat\')">Proto</th>' +
		       '<th width="25%" id="track_header_1" style="cursor: pointer;" onclick="setsort(1, \'nat\'); draw_table(\'nat\')">NAT IP</th>' +
		       '<th width="11%" id="track_header_2" style="cursor: pointer;" onclick="setsort(2, \'nat\'); draw_table(\'nat\')">NAT Port</th>' +
		       '<th width="25%" id="track_header_3" style="cursor: pointer;" onclick="setsort(3, \'nat\'); draw_table(\'nat\')">Destination IP</th>' +
		       '<th width="11%" id="track_header_4" style="cursor: pointer;" onclick="setsort(4, \'nat\'); draw_table(\'nat\')">Port</th>' +
		       '<th width="20%" id="track_header_5" style="cursor: pointer;" onclick="setsort(5, \'nat\'); draw_table(\'nat\')">State</th></tr>';
	} else if (type == "route") {
		if (tracklen == 0) {
			document.getElementById("connblock_route").innerHTML = '<div class="hint-color" style="font-size: 125%;padding-top:50px;">No active IPv6 connections.</div>';
			return;
		}
		document.getElementById('tracked_filters_route').style.display = "";
		document.getElementById('connblock_route_header').style.display = "";
		document.getElementById('connblock_route').innerHTML = "";
		code = '<table width="100%" cellpadding="4" class="FormTable_table"><thead><tr><td colspan="6">Connections</td></tr></thead>' +
		       '<tr><th width="8%" id="track_header_route_0" style="cursor: pointer;" onclick="setsort(0, \'route\'); draw_table(\'route\')">Proto</th>' +
		       '<th width="25%" id="track_header_route_1" style="cursor: pointer;" onclick="setsort(1, \'route\'); draw_table(\'route\')">Local IP</th>' +
		       '<th width="11%" id="track_header_route_2" style="cursor: pointer;" onclick="setsort(2, \'route\'); draw_table(\'route\')">Port</th>' +
		       '<th width="25%" id="track_header_route_3" style="cursor: pointer;" onclick="setsort(3, \'route\'); draw_table(\'route\')">Destination IP</th>' +
		       '<th width="11%" id="track_header_route_4" style="cursor: pointer;" onclick="setsort(4, \'route\'); draw_table(\'route\')">Port</th>' +
		       '<th width="20%" id="track_header_route_5" style="cursor: pointer;" onclick="setsort(5, \'route\'); draw_table(\'route\')">State</th></tr>';
	}

	dataarray.sort(table_sort);

	// Generate table
	for (i = 0; (i < tracklen); i++) {
		// Compress IPv6
		if (dataarray[i][1].indexOf(":") >= 0)
			dataarray[i][1] = compIPV6(dataarray[i][1]);
		else
			dataarray[i][1] = dataarray[i][1];

		if (dataarray[i][3].indexOf(":") >= 0)
			dataarray[i][3] = compIPV6(dataarray[i][3]);
		else
			dataarray[i][3] = dataarray[i][3];

		// Retrieve IPv6 hostname from objects pushed by httpd
		if (dataarray[i][1].indexOf(":") >= 0 && ipv6clientarray[dataarray[i][1]] != undefined) {
			clientName = ipv6clientarray[dataarray[i][1]];
		} else {
			// Retrieve hostname from networkmap
			clientObj = clientFromIP(dataarray[i][1]);
			if (clientObj) {
				clientName = (clientObj.nickName == "") ? clientObj.name : clientObj.nickName;
			} else {
				srchost = dataarray[i][1];
				clientName = "";
			}
		}
		srchost = (clientName == "") ? dataarray[i][1] : clientName;

		if (dataarray[i][3].indexOf(":") >= 0 && ipv6clientarray[dataarray[i][3]] != undefined) {
			clientName = ipv6clientarray[dataarray[i][3]];
		} else {
			clientObj = clientFromIP(dataarray[i][3]);
			if (clientObj) {
				clientName = (clientObj.nickName == "") ? clientObj.name : clientObj.nickName;
			} else {
				clientName = "";
			}
		}
		dsthost = (clientName == "") ? dataarray[i][3] : clientName;

		// Filter in place?
		var filtered = 0;
		for (j = 0; (j < 6 && !filtered); j++) {
			if (filter[type][j]) {
				switch (j) {
					case 1:
						if (srchost.toLowerCase().indexOf(filter[type][1].toLowerCase()) < 0 &&
						    dataarray[i][1].toLowerCase().indexOf(filter[type][1]) < 0)
							filtered = 1;
						break;
					case 3:
						if (dsthost.toLowerCase().indexOf(filter[type][3].toLowerCase()) < 0 &&
						    dataarray[i][3].toLowerCase().indexOf(filter[type][3]) < 0)
							filtered = 1;
						break;
					default:
						if (dataarray[i][j].toLowerCase().indexOf(filter[type][j]) < 0) {
						filtered = 1;
					}
				}
			}
		}
		if (filtered) continue;

		shownlen++;

		// Output row
		code += "<tr><td>" + dataarray[i][0] + "</td>" +
		        "<td title=\"" + dataarray[i][1] + "\"" + (srchost.length > 36 ? "style=\"font-size: 80%;\"" : "") +">" + srchost + "</td>" +
		        "<td>" + dataarray[i][2] + "</td>" +
		        "<td title=\"" + dataarray[i][3] + "\"" + (dsthost.length > 36 ? "style=\"font-size: 80%;\"" : "") + ">" + dsthost + "</td>" +
		        "<td>" + dataarray[i][4] + "</td>" +
		        "<td>" + dataarray[i][5] + "</td>";
	}

	if (shownlen == 0) {
		code += '<tr><td colspan="6" class="hint-color" style="text-align:center;">No results.</td></tr>';
	}
	code += "</tbody></table>";

	if (type == "nat") {
		document.getElementById('connblock_nat').innerHTML = code;
		document.getElementById('track_header_' + sortfield_nat).style.boxShadow = "rgb(255, 204, 0) 0px " + (sortdir_nat == 1 ? "1" : "-1") + "px 0px 0px inset";
	} else if (type == "route") {
		document.getElementById('connblock_route').innerHTML = code;
		document.getElementById('track_header_route_' + sortfield_route).style.boxShadow = "rgb(255, 204, 0) 0px " + (sortdir_route == 1 ? "1" : "-1") + "px 0px 0px inset";
	}
}


function setsort(newfield, type) {
	if (type == "nat") {
		if (newfield != sortfield_nat) {
			sortdir_nat = 0;
			sortfield_nat = newfield;
		 } else {
			sortdir_nat = (sortdir_nat ? 0 : 1);
		}
	} else if (type == "route") {
		if (newfield != sortfield_route) {
			sortdir_route = 0;
			sortfield_route = newfield;
		 } else {
			sortdir_route = (sortdir_route ? 0 : 1);
		}
	}
}


function table_sort(a, b){
	var aa, bb;

	switch (sortfield_global) {
		case 0:		// Proto
		case 1:		// Source IP
		case 3:		// Destination IP
			if (sortdir_global) {
				aa = full_IPv6(a[sortfield_global].toString());
				bb = full_IPv6(b[sortfield_global].toString());
				if (aa == bb) return 0;
				else if (aa > bb) return -1;
				else return 1;
			} else {
				aa = full_IPv6(a[sortfield_global].toString());
				bb = full_IPv6(b[sortfield_global].toString());
				if (aa == bb) return 0;
				else if (aa > bb) return 1;
				else return -1;
			}
			break;
		case 2:		// Local Port
		case 4:		// Remote Port
			if (sortdir_global)
				return parseInt(b[sortfield_global]) - parseInt(a[sortfield_global]);
			else
				return parseInt(a[sortfield_global]) - parseInt(b[sortfield_global]);
			break;
		case 5:		// Label
			if (sortdir_global) {
				aa = a[sortfield_global];
				bb = b[sortfield_global];
				if(aa == bb) return 0;
				else if(aa > bb) return -1;
				else return 1;
			} else {
				aa = a[sortfield_global];
				bb = b[sortfield_global];
				if(aa == bb) return 0;
				else if(aa > bb) return 1;
				else return -1;
			}
			break;
	}
}

</script>
</head>

<body onload="initial();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" action="apply.cgi" target="hidden_frame">
<input type="hidden" name="current_page" value="Main_ConnStatus_Content.asp">
<input type="hidden" name="next_page" value="Main_ConnStatus_Content.asp">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="first_time" value="">
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
						<td valign="top">
							<table width="760px" border="0" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTitle" id="FormTitle">
								<tbody>
								<tr bgcolor="#4D595D">
									<td valign="top">
										<div>&nbsp;</div>
										<div class="formfonttitle"><#System_Log#> - <#System_act_connections#></div>
										<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
										<div class="formfontdesc"><#System_log_connections#></div>
										<div class="formfontdesc">Click on a column header to sort by that field.</div>

										<div id="connblock_header" style="display:none;"><span style="color:#FFCC00; font-size:larger;">NAT connections</span></div>
										<table cellpadding="4" width="100%" class="FormTable_table" id="tracked_filters" style="display:none;"><thead><tr><td colspan="6">Filter NAT connections</td></tr></thead>
											<tr>
												<th width="8%">Proto</th>
												<th width="25%">NAT IP</th>
												<th width="11%">NAT Port</th>
												<th width="25%">Destination IP</th>
												<th width="11%">Port</th>
												<th width="20%">State</th>
											</tr>
											<tr>
												<td><select class="input_option" onchange="set_filter(0, this, 'nat');">
														<option value="">any</option>
														<option value="tcp">tcp</option>
														<option value="udp">udp</option>
												</select></td>
												<td><input type="text" class="input_15_table" maxlength="39" oninput="set_filter(1, this, 'nat');"></input></td>
												<td><input type="text" class="input_6_table" maxlength="5" oninput="set_filter(2, this, 'nat');"></input></td>
												<td><input type="text" class="input_15_table" maxlength="39" oninput="set_filter(3, this, 'nat');"></input></td>
												<td><input type="text" class="input_6_table" maxlength="5" oninput="set_filter(4, this, 'nat');"></input></td>
												<td><select class="input_option" onchange="set_filter(5, this, 'nat');">
														<option value="">any</option>
														<option value="ASSURED">Assured</option>
														<option value="ESTABLISHED">Established</option>
														<option value="CLOSE">Close</option>
														<option value="TIME_WAIT">Time Wait</option>
														<option value="FIN_WAIT">FIN Wait</option>
														<option value="SYN_">SYN Recv/Sent</option>
														<option value="UNREPLIED">Unreplied</option
													</select></td>
											</tr>
										</table>

										<div id="connblock_nat"></div>

										<div id="connblock_route_header" style="display:none; margin-top:50px;"><span style="color:#FFCC00; font-size:larger;">Routed IPv6 connections</span></div>
										<table cellpadding="4" width="100%" class="FormTable_table" id="tracked_filters_route" style="display:none;"><thead><tr><td colspan="6">Filter routed connections</td></tr></thead>
											<tr>
												<th width="8%">Proto</th>
												<th width="25%">Local IP</th>
												<th width="11%">Port</th>
												<th width="25%">Destination IP</th>
												<th width="11%">Port</th>
												<th width="20%">State</th>
											</tr>
											<tr>
												<td><select class="input_option" onchange="set_filter(0, this, 'route');">
														<option value="">any</option>
														<option value="tcp">tcp</option>
														<option value="udp">udp</option>
												</select></td>
												<td><input type="text" class="input_15_table" maxlength="39" oninput="set_filter(1, this, 'route');"></input></td>
												<td><input type="text" class="input_6_table" maxlength="5" oninput="set_filter(2, this, 'route');"></input></td>
												<td><input type="text" class="input_15_table" maxlength="39" oninput="set_filter(3, this, 'route');"></input></td>
												<td><input type="text" class="input_6_table" maxlength="5" oninput="set_filter(4, this, 'route');"></input></td>
												<td><select class="input_option" onchange="set_filter(5, this, 'route');">
														<option value="">any</option>
														<option value="ASSURED">Assured</option>
														<option value="ESTABLISHED">Established</option>
														<option value="CLOSE">Close</option>
														<option value="TIME_WAIT">Time Wait</option>
														<option value="FIN_WAIT">FIN Wait</option>
														<option value="SYN_">SYN Recv/Sent</option>
														<option value="UNREPLIED">Unreplied</option
													</select></td>
											</tr>
										</table>

										<div id="connblock_route"></div>

										<br>
										<div class="apply_gen">
											<input type="button" onClick="location.reload();" value="<#CTL_refresh#>" class="button_gen">
										</div>
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
</body>
</html>
