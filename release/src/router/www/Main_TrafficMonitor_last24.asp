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
<script language="JavaScript" type="text/javascript" src="/js/trafmon.js"></script>
<style>

.chartCanvas {
	cursor: crosshair;
	background-color: #2f3e44;
	border-radius: 10px;
	width: 100% !important;
	height: 270px;
	display: block;
}

.rategrid {
	display: grid;
	grid-template-columns: repeat(4, 1fr);
	column-gap: 8px;
	row-gap: 4px;
	padding-top: 14px;
}

.ratepair {
	display: inline-flex;
	align-items: center;
}

.ratelabel {
	text-align: right;
	margin-right: 4px;
	flex-shrink: 0;
}

.ratevalue {
	color: white !important;
}


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
var nvram = httpApi.nvramGet(["rstats_enable"])

var speed_history = {};
var chartObj = {};

const samplesPerHour = 120;
const samplesMax = 2880;
const updateInt = 30;
const showBitrate = 1;

const labelsColor = "#CCC";
const gridColor = "#282828";
const ticksColor = "#CCC";
const rxBorderColor = "rgba(76, 143, 192, 1)";
const rxBackgroundColor = "rgba(76, 143, 192, 0.3)";
const txBorderColor = "rgba(76, 192, 143, 1)";
const txBackgroundColor = "rgba(76, 192, 143, 0.3)";

// disable auto log out
AUTOLOGOUT_MAX_MINUTE = 0;

function init(){
	if (nvram.rstats_enable != '1') return;

	if(bwdpi_support){
		document.getElementById('content_title').innerHTML = "<#traffic_monitor#>";
	}

	update_traffic();
}

function init_data_object(){
	for (var ifname in speed_history) {
		if (ifname == "_next") continue;

		speed_history[ifname].friendly = get_friendly_ifname(ifname);

/* Canvas */
		var htmldata = '<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">';
		htmldata += '<thead><tr><td>' + speed_history[ifname].friendly + '</td></tr></thead>' +
		            '<tr>' +
		            '<td style="padding:14px;" width="100%"><canvas class="chartCanvas" id="' + ifname + '_Chart"></canvas>' +
		            '<div class="rategrid">' +
		            '<div class="ratepair"><span class="ratelabel hint-color">Current In: </span><span class="ratevalue" id="' + ifname + '_RX_current"></span></div>' +
		            '<div class="ratepair"><span class="ratelabel hint-color">Avg In: </span><span class="ratevalue" id="'+ ifname + '_RX_avg"></span></div>' +
		            '<div class="ratepair"><span class="ratelabel hint-color">Max In: </span><span class="ratevalue" id="' + ifname + '_RX_max"></span></div>' +
		            '<div class="ratepair"><span class="ratelabel hint-color">Total In: </span><span class="ratevalue" id="' + ifname + '_RX_total"></span></div>' +
		            '<div class="ratepair"><span class="ratelabel hint-color">Current Out: </span><span class="ratevalue" id="' + ifname + '_TX_current"></span></div>' +
		            '<div class="ratepair"><span class="ratelabel hint-color">Avg Out: </span><span class="ratevalue" id="'+ ifname + '_TX_avg"></span></div>' +
		            '<div class="ratepair"><span class="ratelabel hint-color">Max Out: </span><span class="ratevalue" id="' + ifname + '_TX_max"></span></div>' +
		            '<div class="ratepair"><span class="ratelabel hint-color">Total Out: </span><span class="ratevalue" id="' + ifname + '_TX_total"></span></div>' +
		            '</div>' +
		            '</td></tr></table>';

		if (ifname == "INTERNET")	// Always insert at the top
			document.getElementById("graph_content").insertAdjacentHTML("afterbegin", htmldata);
		else
			document.getElementById("graph_content").insertAdjacentHTML("beforeend", htmldata);

/* Chart objects */
		chartObj[ifname] = {};
	}
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
				document.getElementById(ifname + "_RX_current").innerHTML = rescale_data_rate(speed_history[ifname].rx[speed_history[ifname].rx.length-1] / updateInt, (showBitrate ? 2 : 1));
				document.getElementById(ifname + "_TX_current").innerHTML = rescale_data_rate(speed_history[ifname].tx[speed_history[ifname].tx.length-1] / updateInt, (showBitrate ? 2 : 1));
				document.getElementById(ifname + "_RX_avg").innerHTML = rescale_data_rate(ifdata.rx_avg ,(showBitrate ? 2 : 1));
				document.getElementById(ifname + "_TX_avg").innerHTML = rescale_data_rate(ifdata.tx_avg ,(showBitrate ? 2 : 1));
				document.getElementById(ifname + "_RX_max").innerHTML = rescale_data_rate(ifdata.rx_max ,(showBitrate ? 2 : 1));
				document.getElementById(ifname + "_TX_max").innerHTML = rescale_data_rate(ifdata.tx_max ,(showBitrate ? 2 : 1));
				document.getElementById(ifname + "_RX_total").innerHTML = rescale_data_rate(ifdata.rx_total, 0);
				document.getElementById(ifname + "_TX_total").innerHTML = rescale_data_rate(ifdata.tx_total, 0);
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
					backgroundColor: rxBackgroundColor,
					borderColor: rxBorderColor,
					borderWidth: "1",
					pointRadius: "0",
					fill: { target: "origin"}
				},
				{
					label: speed_history[ifname].friendly + " Out",
					data: speed_history[ifname].tx,
					backgroundColor: txBackgroundColor,
					borderColor: txBorderColor,
					borderWidth: "1",
					pointRadius: "0",
					fill: { target: "origin"}
				}
			]
		},
		options: {
			responsive: true,
			animation: false,
			segmentShowStroke : false,
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
							return label + " - " + rescale_data_rate(value / updateInt, (showBitrate ? 2 : 1));
						}
					}
				},
				legend: {
					display: true,
					position: "top",
					labels: {color: labelsColor}
				},
				zoom: {
					limits: {
						x: {
							min: 0,
							max: samplesMax,
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
					max: samplesMax,
					grid: { color: gridColor },
					ticks: {
						color: ticksColor,
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
					grid: { color: gridColor },
					ticks: {
						color: ticksColor,
						callback: function(value, index, ticks) {return rescale_data_rate(value / updateInt, (showBitrate ? 2 : 1));}
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
																<select id="page_select" onchange="tm_switchPage(this.options[this.selectedIndex].value, '2')" class="input_option" style="margin-top:8px;">
																	<option value="1"><#menu4_2_1#></option>
																	<option value="2" selected><#menu4_2_2#></option>
																	<option value="3"><#menu4_2_3#></option>
																	<option value="4">Monthly</option>
																	<option value="5">Settings</option>
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
