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
<title><#Web_Title#> - Classification</title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="usp_style.css">
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/js/chart.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/js/table/table.js"></script>
<script type="text/javascript" src="/client_function.js"></script>

<style>
span.cat0{
	background-color:#B3645B;
}
span.cat1{
	background-color:#B98F53;
}
span.cat2{
	background-color:#C6B36A;
}
span.cat3{
	background-color:#849E75;
}
span.cat4{
	background-color:#2B6692;
}
span.cat5{
	background-color:#7C637A;
}
span.cat6{
	background-color:#4C8FC0;
}
span.cat7{
	background-color:#6C604F;
}
span.catrow{
	padding: 4px 8px 4px 8px; color: white !important;
	border-radius: 5px; border: 1px #2C2E2F solid;
	white-space: nowrap;
}
</style>

<script>
var qos_type ="<% nvram_get("qos_type"); %>";

if ("<% nvram_get("qos_enable"); %>" == 0) {	// QoS disabled
	var qos_mode = 0;
	var qos_default = 0;
}else if (bwdpi_support && (qos_type == "1")) {	// aQoS
	var qos_mode = 2;
	var qos_default = 4;
} else if (qos_type == "0") {			// tQoS
	var qos_mode = 1;
	var qos_default = "<% nvram_get("qos_default"); %>";
} else if (qos_type == "2") {			// BW limiter
	var qos_mode = 3;
	var qos_default = 0;
} else if (qos_type == "3") {			// GeForce Now
	var qos_mode = 4;
	var qos_default = 4;
} else {					// invalid mode
	var qos_mode = 0;
	var qos_default = 0;
}

if (qos_mode == 2) {
	var bwdpi_app_rulelist = "<% nvram_get("bwdpi_app_rulelist"); %>".replace(/&#60/g, "<");
	var bwdpi_app_rulelist_row = bwdpi_app_rulelist.split("<");
	if (bwdpi_app_rulelist == "" || bwdpi_app_rulelist_row.length != 9){
		bwdpi_app_rulelist = "9,20<8<4<0,5,6,15,17<4,13<13,24<1,3,14<7,10,11,21,23<";
		bwdpi_app_rulelist_row = bwdpi_app_rulelist.split("<");
	}
	var category_title = ["Net Control Packets", "<#Adaptive_Game#>", "<#Adaptive_Stream#>","<#Adaptive_Message#>", "<#Adaptive_eLearning#>","<#Adaptive_WebSurf#>","<#Adaptive_FileTransfer#>", "<#Adaptive_Others#>"];
	var cat_id_array = [[9,20], [8], [4], [0,5,6,15,17], [4,13], [13,24], [1,3,14], [7,10,11,21,23]];
} else {
	var category_title = ["", "Highest", "High", "Medium", "Low", "Lowest"];
}

var pie_obj_ul, pie_obj_dl;
var refreshRate;
var timedEvent = 0;
var sortdir = 0;
var sortfield = 5;
var filter = Array(6);
const maxshown = 500;
const maxrendered = 750;

var color = ["#B3645B","#B98F53","#C6B36A","#849E75","#2B6692","#7C637A","#4C8FC0", "#6C604F"];
var labels_array = [];

var pieOptions = {
        segmentShowStroke : false,
        segmentStrokeColor : "#000",
        animationEasing : "easeOutQuart",
        animationSteps : 100,
        animateScale : true,
	legend : { display : false },
	tooltips: {
		callbacks: {
			title: function (tooltipItem, data) { return data.labels[tooltipItem[0].index]; },
			label: function (tooltipItem, data) {
				var value = data.datasets[tooltipItem.datasetIndex].data[tooltipItem.index];
				var orivalue = value;
				var total = eval(data.datasets[tooltipItem.datasetIndex].data.join("+"));
				var unit = " bytes";
				if (value > 1024) {
					value = value / 1024;
					unit = " KB";
				}
				if (value > 1024) {
					value = value / 1024;
					unit = " MB";
				}
				if (value > 1024) {
					value = value / 1024;
					unit = " GB";
				}
				return value.toFixed(2) + unit + ' ( ' + parseFloat(orivalue * 100 / total).toFixed(2) + '% )';
		              
			},
		}
	},

}

function comma(n){
	n = '' + n;
	var p = n;
	while ((n = n.replace(/(\d+)(\d{3})/g, '$1,$2')) != p) p = n;
	return n;
}

function initial(){
	show_menu();
	refreshRate = document.getElementById('refreshrate').value;
	get_data();
}

function get_qos_class(category, appid){
	var i, j, catlist, rules;

	if (category == 0 && appid == 0)
		return qos_default;

	for (i=0; i < bwdpi_app_rulelist_row.length-2; i++){
		rules = bwdpi_app_rulelist_row[i];

		// Add categories missing from nvram but always found in qosd.conf
		if (i == 0)
			rules += ",18,19";
		else if (i == 4)
			rules += ",28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43";
		else if (i == 5)
			rules += ",12";

		catlist = rules.split(",");
		for (j=0; j < catlist.length; j++) {
			if (catlist[j] == category){
				return i;
			}
		}
	}
	return qos_default;
}

function compIPV6(input) {
	input = input.replace(/\b(?:0+:){2,}/, ':');
	return input.replace(/(^|:)0{1,4}/g, ':');
}

function set_filter(field, o) {
	filter[field] = o.value.toLowerCase();
	draw_conntrack_table();
}

function draw_conntrack_table(){
	var i, j, qosclass, label;
	var tracklen, shownlen = 0;
	var code;
	var clientObj, clientName;
	var srchost, srctitle, dsthost, dsttitle;

	tracklen = bwdpi_conntrack.length;

	if (tracklen == 0 ) {
		showhide("tracked_filters", 0);
		document.getElementById('tracked_connections').innerHTML = "";
		return;
	}

	showhide("tracked_filters", 1);

	genClientList();

	code = '<table cellpadding="4" class="FormTable_table"><thead><tr><td colspan="6">Tracked connections (total: ' + tracklen + ')</td></tr></thead>' +
		'<tr><th width="5%" id="track_header_0" style="cursor: pointer;" onclick="setsort(0); draw_conntrack_table()">Proto</th>' +
		'<th width="28%" id="track_header_1" style="cursor: pointer;" onclick="setsort(1); draw_conntrack_table()">Source IP</th>' +
		'<th width="6%" id="track_header_2" style="cursor: pointer;" onclick="setsort(2); draw_conntrack_table()">Port</th>' +
		'<th width="28%" id="track_header_3" style="cursor: pointer;" onclick="setsort(3); draw_conntrack_table()">Destination IP</th>' +
		'<th width="6%" id="track_header_4" style="cursor: pointer;" onclick="setsort(4); draw_conntrack_table()">Port</th>' +
		'<th width="27%" id="track_header_5" style="cursor: pointer;" onclick="setsort(5); draw_conntrack_table()">Application</th></tr>';

	if (tracklen > maxrendered) {
		document.getElementById('refreshrate').value = "0";
		refreshRate = 0;
		document.getElementById('toomanyconns').style.display = "";
		document.getElementById('refreshrate').disabled = true;
	}

	bwdpi_conntrack.sort(table_sort);

	// Generate table
	for (i = 0; (i < tracklen && shownlen < maxshown); i++){

		// Compress IPv6
		if (bwdpi_conntrack[i][1].indexOf(":") >= 0)
			bwdpi_conntrack[i][1] = compIPV6(bwdpi_conntrack[i][1]);
		else
			bwdpi_conntrack[i][1] = bwdpi_conntrack[i][1];

		if (bwdpi_conntrack[i][3].indexOf(":") >= 0)
			bwdpi_conntrack[i][3] = compIPV6(bwdpi_conntrack[i][3]);
		else
			bwdpi_conntrack[i][3] = bwdpi_conntrack[i][3];

		// Retrieve hostname from networkmap
		clientObj = clientFromIP(bwdpi_conntrack[i][1]);
		if (clientObj) {
			clientName = (clientObj.nickName == "") ? clientObj.name : clientObj.nickName;
		} else {
			srchost = bwdpi_conntrack[i][1];
			clientName = "";
		}
		srchost = (clientName == "") ? bwdpi_conntrack[i][1] : clientName;
		srctitle = bwdpi_conntrack[i][1];

		clientObj = clientFromIP(bwdpi_conntrack[i][3]);
		if (clientObj) {
			clientName = (clientObj.nickName == "") ? clientObj.name : clientObj.nickName;
		} else {
			clientName = "";
		}
		dsthost = (clientName == "") ? bwdpi_conntrack[i][3] : clientName;
		dsttitle = bwdpi_conntrack[i][3];


		// Filter in place?
		var filtered = 0;
		for (j = 0; j < 6; j++) {
			if (filter[j]) {
				switch (j) {
					case 1:
						if (srchost.toLowerCase().indexOf(filter[1].toLowerCase()) < 0 &&
						    bwdpi_conntrack[i][1].toLowerCase().indexOf(filter[1]) < 0)
							filtered = 1;
						break;
					case 3:
						if (dsthost.toLowerCase().indexOf(filter[3].toLowerCase()) < 0 &&
						    bwdpi_conntrack[i][3].toLowerCase().indexOf(filter[3]) < 0)
							filtered = 1;
						break;
					default:
						if (bwdpi_conntrack[i][j].toLowerCase().indexOf(filter[j]) < 0) {
						filtered = 1;
					}
				}
				if (filtered) break;
			}
		}
		if (filtered) continue;

		shownlen++;

		// Get QoS priority
		qosclass = get_qos_class(bwdpi_conntrack[i][7], bwdpi_conntrack[i][6]);

		// Get priority label
		if (bwdpi_conntrack[i][7] == 0 && bwdpi_conntrack[i][6] == 0)
			label = "Default (" + labels_array[qosclass] + ")";
		else
			label = labels_array[qosclass];


		// Output row
		code += "<tr><td>" + bwdpi_conntrack[i][0] + "</td>";
		code += "<td title=\"" + srctitle + "\"" + (srchost.length > 36 ? "style=\"font-size: 80%;\"" : "") +">" +
	                  srchost + "</td>";
		code += "<td>" + bwdpi_conntrack[i][2] + "</td>";
		code += "<td title=\"" + dsttitle + "\"" + (dsthost.length > 36 ? "style=\"font-size: 80%;\"" : "") + ">" +
		          dsthost + "</td>";
		code += "<td>" + bwdpi_conntrack[i][4] + "</td>";
		code += "<td><span title=\"" + label + "\" class=\"catrow cat" +
	                  qosclass + "\"" + (bwdpi_conntrack[i][5].length > 27 ? "style=\"font-size: 75%;\"" : "") + ">" +
	                  bwdpi_conntrack[i][5] + "</span></td></tr>";
	}

	if (shownlen == maxshown)
		code += '<tr><td colspan="6"><span style="text-align: center;">List truncated to ' + maxshown + ' elements - use a filter</td></tr>';

	code += "</tbody></table>";

	document.getElementById('tracked_connections').innerHTML = code;
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


function redraw(){
	var code;

	switch (qos_mode) {
		case 0:		// Disabled
			document.getElementById('dl_tr').style.display = "none";
			document.getElementById('ul_tr').style.display = "none";
			document.getElementById('no_qos_notice').style.display = "";
			return;

		case 3:		// Bandwith Limiter
			document.getElementById('dl_tr').style.display = "none";
			document.getElementById('ul_tr').style.display = "none";
			document.getElementById('limiter_notice').style.display = "";
			return;

		case 1:         // Traditional
			document.getElementById('dl_tr').style.display = "none";
			document.getElementById('tqos_notice').style.display = "";
			break;

		case 2:		// Adaptive
		case 4:		// GeForce Now
			if (pie_obj_dl != undefined) pie_obj_dl.destroy();
			var ctx_dl = document.getElementById("pie_chart_dl").getContext("2d");
			tcdata_lan_array.sort(function(a,b) {return a[0]-b[0]} );
			code = draw_chart(tcdata_lan_array, ctx_dl, "dl");
			document.getElementById('legend_dl').innerHTML = code;
			break;
	}

	if (pie_obj_ul != undefined) pie_obj_ul.destroy();
	var ctx_ul = document.getElementById("pie_chart_ul").getContext("2d");
	tcdata_wan_array.sort(function(a,b) {return a[0]-b[0]} );
	code = draw_chart(tcdata_wan_array, ctx_ul, "ul");
	document.getElementById('legend_ul').innerHTML = code;

	pieOptions.animation = false;	// Only animate first time
}


function get_data() {
	if (timedEvent) {
		clearTimeout(timedEvent);
		timedEvent = 0;
	}

	$.ajax({
		url: '/ajax_gettcdata.asp',
		dataType: 'script',
		error: function(xhr){
			get_data();
		},
		success: function(response){
			redraw();
			if (qos_mode == 2) draw_conntrack_table();
			if (refreshRate > 0)
				timedEvent = setTimeout("get_data();", refreshRate * 1000);
		}
	});
}


function draw_chart(data_array, ctx, pie) {
	var code = '<table><thead style="text-align:left;"><tr><th style="padding-left:5px;">Class</th><th style="padding-left:5px;">Total</th><th style="padding-left:20px;">Rate</th><th style="padding-left:20px;">Packet rate</th></tr></thead>';
	var values_array = [];
	var i, index, label;

	labels_array = [];

	for (i=0; i < data_array.length-1; i++){
		var value = parseInt(data_array[i][1]);
		var tcclass = parseInt(data_array[i][0]);
		var rate, label;

		if (qos_mode == 2) {
			for(index=0;index<cat_id_array.length;index++){
				if(cat_id_array[index] == bwdpi_app_rulelist_row[i]){
					break;
				}
			}

			if (index == cat_id_array.length)
				label = "Unknown";
			else
				label = category_title[index];
		} else {
			tcclass = tcclass / 10;
			label = category_title[tcclass];
			if (label == undefined)
				label = "Class " + tcclass;
		}
		labels_array.push(label);
		values_array.push(value);

		if ((qos_mode == 2 && i == qos_default) ||
		    ((qos_mode == 1 || qos_mode == 4) && tcclass-1 == qos_default))
			label = label + "<span style=\"font-size: 75%; font-style: italic;\"> (Default)</span>";

		var unit = " Bytes";
		if (value > 1024) {
			value = value / 1024;
			unit = " KB";
		}
		if (value > 1024) {
			value = value / 1024;
			unit = " MB";
		}
		if (value > 1024) {
			value = value / 1024;
			unit = " GB";
		}

		code += '<tr><td style="word-wrap:break-word;padding-left:5px;padding-right:5px;border:1px #2C2E2F solid; border-radius:5px;background-color:'+color[i]+';margin-right:10px;line-height:20px;">' + label + '</td>';
		code += '<td style="text-align:right;padding-left:5px;">' + value.toFixed(2) + unit + '</td>';
		rate = comma(data_array[i][2]);
		code += '<td style="text-align:right;padding-left:20px;">' + rate.replace(/([0-9,])([a-zA-Z])/g, '$1 $2') + '</td>';
		rate = comma(data_array[i][3]);
		code += '<td style="text-align:right;padding-left:20px;">' + rate.replace(/([0-9,])([a-zA-Z])/g, '$1 $2') + '</td></tr>';
	}
	code += '</table>';

	var pieData = {labels: labels_array,
		datasets: [
			{data: values_array,
			backgroundColor: color,
			hoverBackgroundColor: color,
			borderColor: "#444",
			borderWidth: "1"
		}]
	};

	var pie_obj = new Chart(ctx,{
	    type: 'pie',
	    data: pieData,
	    options: pieOptions
	});

	if (pie == "ul")
		pie_obj_ul = pie_obj;
	else
		pie_obj_dl = pie_obj;

	return code;
}

</script>
</head>
<body onload="initial();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="current_page" value="/QoS_Stats.asp">
<input type="hidden" name="next_page" value="/QoS_Stats.asp">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="flag" value="">

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
            <table width="760px" border="0" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTitle" id="FormTitle">
                <tbody>
                <tr bgcolor="#4D595D">
                <td valign="top">
	                <div>&nbsp;</div>
		        <div class="formfonttitle">Traffic classification</div>
			<div style="margin:10px 0 10px 5px;" class="splitLine"></div>

			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
				<tr>
					<th>Automatically refresh data every</th>
					<td>
						<select name="refreshrate" class="input_option" onchange="refreshRate = this.value; get_data();" id="refreshrate">
							<option value="0">No refresh</option>
							<option value="3" selected>3 seconds</option>
							<option value="5">5 seconds</option>
							<option value="10">10 seconds</option>
						</select>
						<span id="toomanyconns" style="display:none; color:#FFCC00;">Disabled - too many tracked connections.</span>
					</td>
				</tr>
			</table>
			<br>

			<div id="limiter_notice" style="display:none;font-size:125%;color:#FFCC00;">Note: Statistics not available in Bandwidth Limiter mode.</div>
			<div id="no_qos_notice" style="display:none;font-size:125%;color:#FFCC00;">Note: QoS is not enabled.</div>
			<div id="tqos_notice" style="display:none;font-size:125%;color:#FFCC00;">Note: Traditional QoS only classifies uploaded traffic.</div>
			<table>
				<tr id="dl_tr">
					<td style="padding-right:50px;font-size:125%;color:#FFCC00;"><div>Download</div><canvas id="pie_chart_dl" width="200" height="200"></canvas></td>
					<td><span id="legend_dl"></span></td>
				</tr>
				<tr style="height:50px;"><td colspan="2">&nbsp;</td></tr>
                                <tr id="ul_tr">
                                        <td style="padding-right:50px;font-size:125%;color:#FFCC00;"><div>Upload</div><canvas id="pie_chart_ul" width="200" height="200"></canvas></td>
                                        <td><span id="legend_ul"></span></td>
                                </tr>
			</table>
			<br>
			<table cellpadding="4" class="FormTable_table" id="tracked_filters" style="display:none;"><thead><tr><td colspan="6">Filter connections</td></tr></thead>
				<tr>
					<th width="5%">Proto</th>
					<th width="28%">Source IP</th>
					<th width="6%">Port</th>
					<th width="28%">Destination IP</th>
					<th width="6%">Port</th>
					<th width="27%">Application</th>
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
					<td><input type="text" class="input_18_table" maxlength="48" oninput="set_filter(5, this);"></input></td>
				</tr>
			</table>
			<div id="tracked_connections">
			<br>
			<div class="apply_gen" style="padding-top: 25px;"><input type="button" onClick="location.href=location.href" value="<#CTL_refresh#>" class="button_gen"></div>
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
</form>
<div id="footer"></div>
</body>
</html>
