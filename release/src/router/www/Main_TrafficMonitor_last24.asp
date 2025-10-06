<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<meta http-equiv="X-UA-Compatible" content="IE=Edge" />
<title><#Web_Title#> - <#traffic_monitor#> : <#menu4_2_2#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/httpApi.js"></script>
<script language="JavaScript" type="text/javascript" src="client_function.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/chart.min.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/hammer.min.js"></script>
<script language="JavaScript" type="text/javascript" src="/js/chartjs-plugin-zoom.min.js"></script>
<script language="JavaScript" type="text/javascript" src="help.js"></script>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>

<style>
	.statcell { width:25% !important; text-align:left !important; }
	.wgsheader:first-letter { text-transform: capitalize; }

@keyframes spin {
    from { transform: rotate(0deg); }
    to { transform: rotate(360deg); }
}

.spinner-container {
    display: flex;
    justify-content: center;
    margin-top: 20px;
}

.spinner {
    width: 30px;
    height: 30px;
    border: 3px solid rgba(0, 0, 0, 0.1);
    border-top-color: #000;
    border-radius: 50%;
    animation: spin 1s linear infinite;
}

</style>

<script type='text/javascript'>
var nvram = httpApi.nvramGet(["bond_wan", "rc_support", "wans_lanport", "rstats_enable"])

var speed_history = {};
var chartObj = {};

const samplesPerHour = 120;
const samplesMax = 2880;
const updateInt = 30;

// disable auto log out
AUTOLOGOUT_MAX_MINUTE = 0;

function init()
{
	if (nvram.rstats_enable != '1') return;

	if(bwdpi_support){
		document.getElementById('content_title').innerHTML = "<#traffic_monitor#>";
	}

	update_traffic();
}

function switchPage(page){
	if(page == "1")
		location.href = "/Main_TrafficMonitor_realtime.asp";
	else if(page == "2")
		return false;
	else if(page == "4")
		location.href = "/Main_TrafficMonitor_monthly.asp";
	else if(page == "3")
		location.href = "/Main_TrafficMonitor_daily.asp";
}

function init_data_object(){
	for (var ifname in speed_history) {
		if (ifname == "_next") continue;

		speed_history[ifname].friendly = get_friendly_name(ifname);

/* Canvas */
		var htmldata = '<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">';
		htmldata += '<thead><tr><td>' + speed_history[ifname].friendly + '</td></tr></thead>';
		htmldata += '<tr>';
		htmldata += '<td style="padding:14px;" width="100%"><canvas style="cursor:crosshair; background-color:#2f3e44;border-radius:10px;width: 100% !important; height:270px;" id="' + ifname + '_Chart"></canvas>';
		htmldata += '<div style="padding-top: 14px; display: flex; justify-content: space-between;">';
		htmldata += '<div class="hint-color">Current In: <span style="color: white;" id="' + ifname + '_RX_current"></span></div><div class="hint-color">Avg In: <span style="color: white;" id="'+ ifname + '_RX_avg"></span></div><div class="hint-color">Max In: <span style="color: white;" id="' + ifname + '_RX_max"></div><div class="hint-color">Total In: <span style="color: white;" id="' + ifname + '_RX_total"></div></div>';
		htmldata += '<div style="display: flex; justify-content: space-between;">';
		htmldata += '<div class="hint-color">Current Out: <span style="color: white;" id="' + ifname + '_TX_current"></span></div><div class="hint-color">Avg Out: <span style="color: white;"id="'+ ifname + '_TX_avg"></span></div><div class="hint-color">Max Out: <span style="color: white;" id="' + ifname + '_TX_max"></div><div class="hint-color">Total Out: <span style="color: white;" id="' + ifname + '_TX_total"></div>';
		htmldata += '</div></td>'
		htmldata += '</tr></table>';

		if (ifname == "INTERNET")	// Always insert at the top
			document.getElementById("graph_content").insertAdjacentHTML("afterbegin", htmldata);
		else
			document.getElementById("graph_content").insertAdjacentHTML("beforeend", htmldata);

/* Chart objects */
		chartObj[ifname] = {};
	}
}

function get_friendly_name(ifname){
	var title;

	switch(ifname){
		case "INTERNET":
		case "INTERNET0":
			if(dualWAN_support){
				if(wans_dualwan_array[0] == "usb"){
					if(gobi_support)
						title = "<#Mobile_title#>";
					else
						title = "USB Modem";
				}
				else if(wans_dualwan_array[0] == "wan"){
					title = "WAN";
					if (based_modelid == "TUF-AX4200" || based_modelid == "TUF-AX6000")
						title = "2.5G WAN";
					if (based_modelid == "GT-AXY16000" || based_modelid == "RT-AX89U" || based_modelid == "TUF-AX4200" || based_modelid == "TUF-AX6000") {
						if (nvram.bond_wan == '1' && nvram.rc_support.indexOf("wanbonding") != -1)
							title = "Bond";
					}
				}
				else if(wans_dualwan_array[0] == "wan2"){
					if (based_modelid == "GT-AXY16000" || based_modelid == "RT-AX89U")
						title = "10G base-T";
					else
						title = "WAN2";
				}
				else if(wans_dualwan_array[0] == "lan") {
					title = "LAN Port " + nvram.wans_lanport;
					if (based_modelid == "TUF-AX4200" || based_modelid == "TUF-AX6000") {
						if (nvram.wans_lanport == '5')
							title = "2.5G LAN";
					}
				}
				else if(wans_dualwan_array[0] == "dsl")
					title = "DSL WAN";
				else if(wans_dualwan_array[0] == "sfp+")
					title = "10G SFP+";
				else
					title = "<#dualwan_primary#>";
			}
			else
				title = "<#Internet#>";

			return title;

		case "INTERNET1":
			if(wans_dualwan_array[1] == "usb"){
				if(gobi_support)
					title = "<#Mobile_title#>";
				else
					title = "USB Modem";
			}
			else if(wans_dualwan_array[1] == "wan"){
				title = "WAN";
				if (based_modelid == "TUF-AX4200" || based_modelid == "TUF-AX6000")
					title = "2.5G WAN";
				if (based_modelid == "GT-AXY16000" || based_modelid == "RT-AX89U" || based_modelid == "TUF-AX4200" || based_modelid == "TUF-AX6000") {
					if (nvram.bond_wan == '1' && nvram.rc_support.indexOf("wanbonding") != -1)
						title = "Bond";
				}
			}
			else if(wans_dualwan_array[1] == "wan2"){
				if (based_modelid == "GT-AXY16000" || based_modelid == "RT-AX89U")
					title = "10G base-T";
				else
					title = "WAN2";
			}
			else if(wans_dualwan_array[1] == "lan") {
				title = "LAN Port " + nvram.wans_lanport;
				if (based_modelid == "TUF-AX4200" || based_modelid == "TUF-AX6000") {
					if (nvram.wans_lanport == '5')
						title = "2.5G LAN";
				}
			}
			else if(wans_dualwan_array[1] == "sfp+")
				title = "10G SFP+";
			else
				title = "<#dualwan_secondary#>";

			return title;

		case "BRIDGE":
			return "LAN";
		case "WIRED":
			return "<#tm_wired#>";

		case "WIRELESS0":
		case "WIRELESS1":
		case "WIRELESS2":
		case "WIRELESS3":
			var num = ifname.substr(8);
			return "Wireless " + wl_nband_title[num];
	}

/* Handle multi-instanced interfaces */
	if (ifname.search(/WAGGR/) > -1){
		var bs_port_id = ifname.substr(5);
		if (bs_port_id == 0)
			return "bond-slave (WAN)";
		else if (bs_port_id >= 1 && bs_port_id <= 8)
			return "bond-slave (LAN Port "+bs_port_id+")";
		else if (bs_port_id == 30)
			return "bond-slave (10G base-T)";
		else if (bs_port_id == 31)
			return "bond-slave (10G SFP+)";
		else
			return "NotUsed";
	}
	else if (ifname.search(/LACPW/) > -1){
		var num = ifname.substr(5);
		return "bond-slave (WAN"+num+")";
	}
	else if (ifname.search("LACP") > -1){
		var num = ifname.substr(4);
		return "bond-slave (LAN Port "+num+")";
	}

	/* No friendly name, return as-is */
	return ifname;
}

function format_rate(value, isspeed = 0){
	var unit = " KB";
	value = value / 1024;

	if (value > 1024) {
		value = value / 1024;
		unit = " MB";
	}

	if (value > 1024) {
		value = value / 1024;
		unit = " GB";
	}

	if (isspeed == 1)
		unit += "/s";

	return parseInt(value) + unit;
}

function update_traffic(){
	$.ajax({
		url: '/update.cgi',
		dataType: 'script',
		data: {'output': 'bandwidth', 'arg0': 'speed'},
		error: function(xhr) {
			setTimeout("update_traffic()", 1000);
		},
		success: function(response){
			if (typeof(speed_history) == 'undefined') {
				setTimeout("update_traffic()", 1000);
				return;
			}

			if (Object.keys(chartObj).length === 0) {
				document.getElementById("pageloading").style.display = "none";
				init_data_object();
			}

			for (var ifname in speed_history) {
				var ifdata = speed_history[ifname];
				var index, value;

				if ((ifname == "_next") || (typeof(ifdata.rx) == 'undefined') || (typeof(ifdata.tx) == 'undefined')) {
					continue;
				}

/* Update total/avg/max values */
				ifdata.rx_total = ifdata.rx_max = 0;
				ifdata.tx_total = ifdata.tx_max = 0;
				for (index = (ifdata.rx.length - samplesMax); index < ifdata.rx.length; ++index) {
					value = ifdata.rx[index];
					if (value > ifdata.rx_max) ifdata.rx_max = value;
							ifdata.rx_total += value;
					value = ifdata.tx[index];
					if (value > ifdata.tx_max) ifdata.tx_max = value;
							ifdata.tx_total += value;
				}
				ifdata.rx_avg = ifdata.rx_total / (ifdata.count ? ifdata.count : samplesMax) / updateInt;
				ifdata.tx_avg = ifdata.tx_total / (ifdata.count ? ifdata.count : samplesMax) / updateInt;
				ifdata.rx_max /= updateInt;
				ifdata.tx_max /= updateInt;

/* Output */
				drawGraph(ifname);
				document.getElementById(ifname + "_RX_current").innerHTML = format_rate(speed_history[ifname].rx[speed_history[ifname].rx.length-1] / updateInt, 1);
				document.getElementById(ifname + "_TX_current").innerHTML = format_rate(speed_history[ifname].tx[speed_history[ifname].tx.length-1] / updateInt, 1);
				document.getElementById(ifname + "_RX_avg").innerHTML = format_rate(ifdata.rx_avg ,1);
				document.getElementById(ifname + "_TX_avg").innerHTML = format_rate(ifdata.tx_avg ,1);
				document.getElementById(ifname + "_RX_max").innerHTML = format_rate(ifdata.rx_max ,1);
				document.getElementById(ifname + "_TX_max").innerHTML = format_rate(ifdata.tx_max ,1);
				document.getElementById(ifname + "_RX_total").innerHTML = format_rate(ifdata.rx_total, 0);
				document.getElementById(ifname + "_TX_total").innerHTML = format_rate(ifdata.tx_total, 0);
			}
			setTimeout("update_traffic()", updateInt * 1000);
		}
	});
}


function drawGraph(ifname){
	var now = new Date();
	var currentHour = now.getHours();

	if (chartObj[ifname].obj != undefined) {
		chartObj[ifname].obj.update();
		return;
	}

	var ctx = document.getElementById(ifname + '_Chart').getContext("2d");

	chartObj[ifname].obj = new Chart(ctx, {
		type: "line",
		data: {
			labels: Array.from({ length: samplesMax }, (v, i) => i),
			datasets: [
				{
					label: speed_history[ifname].friendly + " In",
					data: speed_history[ifname].rx,
					backgroundColor: "rgba(76, 143, 192, 0.3)",
					borderColor: "rgba(76, 143, 192, 1)",
					borderWidth: "1",
					pointStyle: "line",
					lineTension: "0",
					fill: { target: "origin"}
				},
				{
					label: speed_history[ifname].friendly + " Out",
					data: speed_history[ifname].tx,
					backgroundColor: "rgba(76, 192, 143, 0.3)",
					borderColor: "rgba(76, 192, 143, 1)",
					borderWidth: "1",
					pointStyle: "line",
					lineTension: "0",
					fill: { target: "origin"}
				}
			]
		},
		options: {
			responsive: true,
			animation: false,
			segmentShowStroke : false,
			segmentStrokeColor : "#000",
			pointRadius : 0,
			interaction: {
				mode: 'index',
				intersect: false
			},
			plugins: {
				tooltip: {
					position: 'nearest',
					intersect: false,
					mode: 'index',
					displayColors: false,
					bodySpacing: 6,
					callbacks: {
						title: function(tooltipItems) {
						// tooltipItems is an array, take the first item
							var item = tooltipItems[0];
							var sampleIndex = item.dataIndex;

						// Calculate hours and minutes
							const totalMinutesAgo = (samplesMax - sampleIndex - 1) * (updateInt / 60);
							const tooltipTime = new Date();
							tooltipTime.setHours(now.getHours() - Math.floor(totalMinutesAgo / 60));
							tooltipTime.setMinutes(now.getMinutes() - (totalMinutesAgo % 60));

						// Format HH:MM
							const hh = tooltipTime.getHours().toString().padStart(2, '0');
							const mm = tooltipTime.getMinutes().toString().padStart(2, '0');
							return "Time: " + hh + ":" + mm;
						},
						label: function(context) {
							var label = context.dataset.label || '';
							var value = context.parsed.y;
							return label + " - " + format_rate(value / updateInt, 1);
						}
					}
				},
				legend: {
					display: true,
					position: "top",
					labels: {color: "#CCC"}
				},
				zoom: {
					limits: {
						x: {
							min: 0,
							max: samplesMax - 1,
							minRange: samplesPerHour
						}
					},
					zoom: {
						wheel: { enabled: true },
						pinch: { enabled: true },
						mode: "x",
					},
					pan: {
						enabled: true,
						mode: "x",
					}
				}
			},
			scales: {
				x: {
					type: 'linear',
					min: 0,
					max: samplesMax - 1,
					grid: { color: "#282828" },
					ticks: {
						color: "#CCC",
						stepSize: samplesPerHour,
						callback: function(value) {
							var hourOffset = Math.floor(value / samplesPerHour);
							var labelHour = (currentHour - (24 - hourOffset)) % 24;
							return (labelHour < 0 ? labelHour + 24 : labelHour) + ":00";
						}
					}
				},
				y: {
					grace: "5%",
					min: 0,
					grid: { color: "#282828" },
					ticks: {
						color: "#CCC",
						callback: function(value, index, ticks) {return format_rate(value / updateInt, 1);}
					}
				}
			}
		}
	});
}

</script>
</head>

<body onload="show_menu();init();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="apply.cgi" target="hidden_frame">
<input type="hidden" name="current_page" value="Main_TrafficMonitor_last24.asp">
<input type="hidden" name="next_page" value="Main_TrafficMonitor_last24.asp">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="zoom" value="3">

<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td width="23">&nbsp;</td>

<!--=====Beginning of Main Menu=====-->
	<td valign="top" width="202">
	  <div id="mainMenu"></div>
	  <div id="subMenu"></div>
	</td>

	<td valign="top">
		<div id="tabMenu" class="submenuBlock"></div>
		<!--===================================Beginning of Main Content===========================================-->
		<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
			<tr>
				<td align="left"  valign="top">
					<table width="100%" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
						<tbody>
							<tr>
								<td bgcolor="#4D595D" valign="top">
									<table width="740px" border="0" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="TMTable">
										<tr>
											<td>
												<table width="100%">
													<tr>
														<td  class="formfonttitle" align="left">
															<div id="content_title" style="margin-top:5px;"><#Menu_TrafficManager#> - <#traffic_monitor#></div>
														</td>
														<td>
															<div align="right">
																<select id="page_select" onchange="switchPage(this.options[this.selectedIndex].value)" class="input_option" style="margin-top:8px;">
																	<option value="1"><#menu4_2_1#></option>
																	<option value="2" selected><#menu4_2_2#></option>
																	<option value="3"><#menu4_2_3#></option>
																	<option value="4">Monthly</option>
																</select>
															</div>
												</td></tr></table>
											</td>
										</tr>
										<tr>
											<td height="5"><div class="splitLine"></div></td>
										</tr>
										<tr>
											<td id="graph_content">
												<div class="spinner-container">
													<div id="pageloading" class="spinner"></div>
												</div>
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
	</td>
	</tr>
</table>

<div id="footer"></div>
</body>
</html>


