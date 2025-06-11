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

<% get_connlist_array(); %>
<% get_ipv6clients_array(); %>

var sortdir = 0;
var sortfield = 0;
var filter = Array(6);

function initial() {
	show_menu();
	connarray.pop();	// Remove last empty element
	draw_conntrack_table()
}

function compIPV6(input) {
	input = input.replace(/\b(?:0+:){2,}/, ':');
	return input.replace(/(^|:)0{1,4}/g, ':');
}

function set_filter(field, o) {
	filter[field] = o.value.toLowerCase();
	draw_conntrack_table()
}

function draw_conntrack_table(){
	var i, j, qosclass, label;
	var tracklen, shownlen = 0;
	var code;
	var clientObj, clientName;
	var srchost, srctitle, dsthost, dsttitle;
	var index, colindex, i;

	tracklen = connarray.length
	if (tracklen == 0) {
		showhide("tracked_filters", 0);
		document.getElementById('tracked_connections').innerHTML = "";
		document.getElementById("connblock").innerHTML = '<span style="color:#FFCC00;">No active connections.</span>';
		return;
	}

	showhide("tracked_filters", 1);

	code = '<table width="100%" cellpadding="4" class="FormTable_table"><thead><tr><td colspan="6">Connections</td></tr></thead>' +
		'<tr><th width="10%" id="track_header_0" style="cursor: pointer;" onclick="setsort(0); draw_conntrack_table()">Proto</th>' +
		'<th width="23%" id="track_header_1" style="cursor: pointer;" onclick="setsort(1); draw_conntrack_table()">NAT IP</th>' +
		'<th width="12%" id="track_header_2" style="cursor: pointer;" onclick="setsort(2); draw_conntrack_table()">NAT Port</th>' +
		'<th width="23%" id="track_header_3" style="cursor: pointer;" onclick="setsort(3); draw_conntrack_table()">Destination IP</th>' +
		'<th width="12%" id="track_header_4" style="cursor: pointer;" onclick="setsort(4); draw_conntrack_table()">Port</th>' +
		'<th width="20%" id="track_header_5" style="cursor: pointer;" onclick="setsort(5); draw_conntrack_table()">State</th></tr>';

	connarray.sort(table_sort);
	// Generate table
	for (i = 0; (i < tracklen); i++){

		// Compress IPv6
		if (connarray[i][1].indexOf(":") >= 0)
			connarray[i][1] = compIPV6(connarray[i][1]);
		else
			connarray[i][1] = connarray[i][1];

		if (connarray[i][3].indexOf(":") >= 0)
			connarray[i][3] = compIPV6(connarray[i][3]);
		else
			connarray[i][3] = connarray[i][3];

		// Retrieve IPv6 hostname from objects pushed by httpd
		if (connarray[i][1].indexOf(":") >= 0 && ipv6clientarray[connarray[i][1]] != undefined) {
			clientName = ipv6clientarray[connarray[i][1]];
		} else {
			// Retrieve hostname from networkmap
			clientObj = clientFromIP(connarray[i][1]);
			if (clientObj) {
				clientName = (clientObj.nickName == "") ? clientObj.name : clientObj.nickName;
			} else {
				srchost = connarray[i][1];
				clientName = "";
			}
		}
		srchost = (clientName == "") ? connarray[i][1] : clientName;
		srctitle = connarray[i][1];

                if (connarray[i][3].indexOf(":") >= 0 && ipv6clientarray[connarray[i][3]] != undefined) {
                        clientName = ipv6clientarray[connarray[i][3]];
		} else {
			clientObj = clientFromIP(connarray[i][3]);
			if (clientObj) {
				clientName = (clientObj.nickName == "") ? clientObj.name : clientObj.nickName;
			} else {
				clientName = "";
			}
		}
		dsthost = (clientName == "") ? connarray[i][3] : clientName;
		dsttitle = connarray[i][3];


		// Filter in place?
		var filtered = 0;
		for (j = 0; j < 6; j++) {
			if (filter[j]) {
				switch (j) {
					case 1:
						if (srchost.toLowerCase().indexOf(filter[1].toLowerCase()) < 0 &&
						    connarray[i][1].toLowerCase().indexOf(filter[1]) < 0)
							filtered = 1;
						break;
					case 3:
						if (dsthost.toLowerCase().indexOf(filter[3].toLowerCase()) < 0 &&
						    connarray[i][3].toLowerCase().indexOf(filter[3]) < 0)
							filtered = 1;
						break;
					default:
						if (connarray[i][j].toLowerCase().indexOf(filter[j]) < 0) {
						filtered = 1;
					}
				}
				if (filtered) break;
			}
		}
		if (filtered) continue;

		shownlen++;

		// Output row
		code += "<tr><td>" + connarray[i][0] + "</td>";
		code += "<td title=\"" + srctitle + "\"" + (srchost.length > 36 ? "style=\"font-size: 80%;\"" : "") +">" +
	                  srchost + "</td>";
		code += "<td>" + connarray[i][2] + "</td>";
		code += "<td title=\"" + dsttitle + "\"" + (dsthost.length > 36 ? "style=\"font-size: 80%;\"" : "") + ">" +
		          dsthost + "</td>";
		code += "<td>" + connarray[i][4] + "</td>";
		code += "<td>" + connarray[i][5] + "</td>";
	}

	code += "</tbody></table>";

	document.getElementById('connblock').innerHTML = code;
	document.getElementById('track_header_' + sortfield).style.boxShadow = "rgb(255, 204, 0) 0px " + (sortdir == 1 ? "1" : "-1") + "px 0px 0px inset";
}


function setsort(newfield) {
	if (newfield != sortfield) {
		sortdir = 0;
		sortfield = newfield;
	 } else {
		sortdir = (sortdir ? 0 : 1);
	}
}


function table_sort(a, b){
	var aa, bb;

	switch (sortfield) {
		case 0:		// Proto
		case 1:		// Source IP
		case 3:		// Destination IP
			if (sortdir) {
				aa = full_IPv6(a[sortfield].toString());
				bb = full_IPv6(b[sortfield].toString());
				if (aa == bb) return 0;
				else if (aa > bb) return -1;
				else return 1;
			} else {
				aa = full_IPv6(a[sortfield].toString());
				bb = full_IPv6(b[sortfield].toString());
				if (aa == bb) return 0;
				else if (aa > bb) return 1;
				else return -1;
			}
			break;
		case 2:		// Local Port
		case 4:		// Remote Port
			if (sortdir)
				return parseInt(b[sortfield]) - parseInt(a[sortfield]);
			else
				return parseInt(a[sortfield]) - parseInt(b[sortfield]);
			break;
		case 5:		// Label
			if (sortdir) {
		                aa = a[sortfield];
			        bb = b[sortfield];
				if(aa == bb) return 0;
				else if(aa > bb) return -1;
				else return 1;
			} else {
				aa = a[sortfield];
				bb = b[sortfield];
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

										<table cellpadding="4" width="100%" class="FormTable_table" id="tracked_filters" style="display:none;"><thead><tr><td colspan="6">Filter connections</td></tr></thead>
											<tr>
												<th width="10%">Proto</th>
												<th width="23%">NAT IP</th>
												<th width="12%">NAT Port</th>
												<th width="23%">Destination IP</th>
												<th width="12%">Port</th>
												<th width="20%">State</th>
											</tr>
											<tr>
												<td><select class="input_option" onchange="set_filter(0, this);">
														<option value="">any</option>
														<option value="tcp">tcp</option>
														<option value="udp">udp</option>
												</select></td>
												<td><input type="text" class="input_15_table" maxlength="39" oninput="set_filter(1, this);"></input></td>
												<td><input type="text" class="input_6_table" maxlength="5" oninput="set_filter(2, this);"></input></td>
												<td><input type="text" class="input_15_table" maxlength="39" oninput="set_filter(3, this);"></input></td>
												<td><input type="text" class="input_6_table" maxlength="5" oninput="set_filter(4, this);"></input></td>
												<td><select class="input_option" onchange="set_filter(5, this);">
														<option value="">any</option>
														<option value="ASSURED">Assured</option>
														<option value="ESTABLISHED">Established</option>
														<option value="CLOSE">Close</option>
														<option value="TIME_WAIT">Time Wait</option>
														<option value="FIN_WAIT">FIN Wait</option>
														<option value="SYN_">SYN Recv/Sent</option>
														<option value="UNREPLIED">Unreplied</option
													</select></td>
												<!-- <td><input type="text" class="input_18_table" maxlength="48" oninput="set_filter(5, this);"></input></td> -->
											</tr>
										</table>

										<div style="margin-top:8px">
											<div id="connblock"></div>
										</div>
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
