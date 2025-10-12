/*
 * Jquery implementation for iQoS Network Summary Page
 *
 * $ Copyright Open Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: iQoSNetworkSummary.js, v 1.0 2015-02-12 15:25:43 vbemblek $
 */

/*****
* Function which will get called when DOM for the page gets loaded.START
*****/

var gDeviceListTimers = [];
var gDeviceList = [];
var gGetConfiguration = [];
/* To store Down bytes and Upbytes for all the mac addresses which are requested individually and sent to plot the pie chart */
var gDownBWUpBW = [];
var isHTTPDServer = 1;
var giQosEnabled = 0;
var refreshTime = 5000;

/*****
* Function To Check Valid Page Navigation.START
*****/
function isValideState()
{
	if (giQosEnabled == 0) {
		alert('Please Enable iQoS in Quick Settings' + '\n' + 'To Access iQoS Settings / Device Summary');
		return false;
	}
}

/*****
* Function For initializing the localhost.START
*****/
function initPage()
{
	if (isHTTPDServer == 1) {
		urlDeviceList = window.location.protocol + '//' + window.location.host +'/iQoS.cgi?Req=DeviceList';
		urlDeviceSummary  = window.location.protocol + '//' + window.location.host +'/iQoS.cgi?Req=DeviceSummary';
		urlGetConfig  = window.location.protocol + '//' + window.location.host +'/iQoS.cgi?Req=GetConfig';
		urlSetConfig = window.location.protocol + '//' + window.location.host +'/iQoS.cgi?Req=SetConfig';
	} else {
		urlDeviceList = "http://localhost/devicelist.php";
		urlDeviceSummary = "http://localhost/DeviceSummary.php";
		urlGetConfig = "http://localhost/getconfig.php";
		urlSetConfig = "http://localhost/setconfig.php";
	}
}
/*****
* Function For initializing the localhost.END
*****/
/*****
* Function For getting the configuration.START
*****/
function getConfiguration()
{
	var array = [];

	$.ajax({
		type:"GET",
		url:urlGetConfig,
		success:function(result){
			if (isHTTPDServer == 1)
				array = result;
			else
				array = JSON.parse(result);

			refreshTime = array.RefreshInterval * 1000;
			giQosEnabled = array.EnableiQoS;
			gGetConfiguration = array;
			OnChangeConfiguration(array, true)
		}
	});
}
/*****
* Function For getting the configuration.END
*****/
/*****
* Function For getting the getDeviceList.START
*****/
function getDeviceList()
{
	var array = [];

	for(i=0;i<gDeviceListTimers.length;i++){
		clearTimeout(gDeviceListTimers[i]);
	}
	gDeviceListTimers = [];

	$.ajax({
		type:"GET",
		url:urlDeviceList,
		async:false,
		timeout:refreshTime,
		success:function(result){
			if (isHTTPDServer == 1)
				array = result;
			else
				array = JSON.parse(result);

			gDeviceList = array;
			populateDeviceList(array);
			gDeviceListTimers.push(setTimeout(function(){getDeviceList();},refreshTime));
		}
	});
	gDownBWUpBW = [];
	getDeviceSummary(array);
	PopulatePieCharts();
}
/*****
* Function For getting the getDeviceList.END
*****/
/*****
* Function For getting the upbytes and downbytes.START
*****/
function getUpByteAndDownByte(array)
{
	for (var i = 0; i < array.Device.length; i++) {
		var name = '';
		var ipaddrs = '';

		for (var j = 0; j < gDeviceList.Device.length; j++) {
			if (gDeviceList.Device[j].MacAddr == array.Device[i].MacAddr) {
				var devName = gDeviceList.Device[j].DevName;

				name = gDeviceList.Device[j].Name;
				if(devName != undefined && name != "")
					name += '-' + devName;

				ipaddrs = gDeviceList.Device[j].IPAddr;
				if(devName != undefined && name == "")
					ipaddrs += '-' + devName;

				var tmpArray = {Name:name,
					IPAddr:ipaddrs,
					UpBW:array.Device[i].UpBW,
					DownBW:array.Device[i].DownBW
					};
				gDownBWUpBW.push(tmpArray);
				break;
			}
		}
	}
}
/*****
* Function For getting the upbytes and downbytes.END
*****/

/*****
* Function For getting the DeviceSummary.START
*****/
function getDeviceSummary(devicelistarray)
{
	var array = [];
	var macarray = [];

	if (devicelistarray.length <= 0)
		return;

	for(var i =0; i < devicelistarray.Device.length; i++) {
		macarray.push(devicelistarray.Device[i].MacAddr);
	}

	$.ajax({
		type:"GET",
		url:urlDeviceSummary+'&MacAddr='+macarray,
		async:false,
		timeout:refreshTime,
		success:function(result){
			if (isHTTPDServer == 1)
				array = result;
			else
				array = JSON.parse(result);
			getUpByteAndDownByte(array);
		}
	});
}
/*****
* Function For getting the DeviceSummary.END
*****/
/*****
* Function For Populating PieChart.START
*****/
function PopulatePieCharts()
{
	drawUpstreamPie();
	drawDownstreamPie();
}
/*****
* Function For Populating PieChart.END
*****/
/*****
* Function to send data and set configuration to backend .START
*****/
function setConfiguration()
{
	var upbw = 0;
	var downbw = 0;
	var bwauto;
	var enableiqos;

	if($("#checkenableiqos").prop('checked'))
		enableiqos = '1';
	else
		enableiqos = '0';

	if($("#checkautobandwidth").prop('checked'))
		bwauto = '1';
	else
		bwauto = '0';

	if(bwauto == '0' && enableiqos == '1'){
		var downbwMbps;
		var upbwMbps;

		downbwMbps = $('input:text[name=download]').val();
		upbwMbps = $('input:text[name=upload]').val();

		if( (downbwMbps <= 0) || (downbwMbps == "")){
			alert("Download speed cannot be 0 or empty");
			return;
		}
		else
			downbwMbps = $('input:text[name=download]').val();

		if( (upbwMbps <= 0) || (upbwMbps == "")){
			alert("Upload speed cannot be 0 or empty");
			return;
		}
		else
			upbwMbps = $('input:text[name=upload]').val();

		downbw = (downbwMbps * 1024)/8;
		upbw = (upbwMbps * 1024)/8;
		downbw = downbw.toFixed(2);
		upbw = upbw.toFixed(2);
	}
	var datatosent = {Req : 'SetConfig',
					EnableiQoS : enableiqos,
					BWAuto : bwauto,
					DownBW : downbw,
					UpBW: upbw}

	var request = $.ajax({
					url: urlSetConfig,
					type: "POST",
					data: JSON.stringify(datatosent),
					dataType: "json",
					success: function(data) {
						/* hide the progress bar */
						$(".cmnprogressbar").hideProgressBar();
						alert("Settings Stored");
						getConfiguration();
					}
	});
}
/*****
* Function to send data and set configuration to backend .END
*****/
/*****
* Function to Change configuration .START
*****/
function OnChangeConfiguration(array, flagReset)
{
	var DownBWMbps = (array.DownBW * 8)/1024;
	var UpBWMbps = (array.UpBW * 8)/1024;

	DownBWMbps = DownBWMbps.toFixed(2);
	UpBWMbps = UpBWMbps.toFixed(2);

	if(array.EnableiQoS)
		$('[name="enablecheck"]')[0].checked = true;
	else
		$('[name="enablecheck"]')[0].checked = false;

	if(array.EnableiQoS) {
		if(array.BWAuto) {
			$('[name="autoenablecheck"]')[0].checked = true;
			$("#downloadspeedtext").attr("disabled", "disabled");
			$("#uploadspeedtext").attr("disabled", "disabled");
			$("#bandwidthenabledid").removeClass('bandwidthdisabled');
			$("#bandwidthenabledid").addClass('bandwidthenabled');
			$('.bandwidthenabled tr td').attr('disabled',true);
		}
		else {
			$('[name="autoenablecheck"]')[0].checked = false;
			$("#bandwidthenabledid").removeClass('bandwidthenabled');
			$("#bandwidthenabledid").addClass('bandwidthdisabled');
			$("#downloadspeedtext").removeAttr("disabled");
			$("#uploadspeedtext").removeAttr("disabled");
		}
		$('[name="autoenablecheck"]')[0].disabled = false;
		$("#auto_bw_lbl").removeClass('disabled');
	}
	else {
		$('[name="autoenablecheck"]')[0].checked = false;
		$('[name="autoenablecheck"]')[0].disabled = true;
		$("#auto_bw_lbl").addClass('disabled');
	}
	$("#downloadspeedtext").val(DownBWMbps);
	$("#uploadspeedtext").val(UpBWMbps);
}
/*****
* Function to Change configuration .END
*****/

/*****
* Function to populate DeviceList .START
*****/
function populateDeviceList(array)
{
	$baseelem ='';
	var backcolor;
	var i;
	var deviceImage;
	var devName;

	/* left side Layout Page*/
	for(i = 0;i < array.Device.length; i+=2) {
		deviceImage = GetIcon("devtype", array.Device[i].DevTypeInt);
		devName = array.Device[i].DevName;
		if(devName != undefined) {
			$baseelem += '<tr><td style="padding-right:10px;"><div class="networklayouttextbox">' ;
			if (array.Device[i].Name == "")
				$baseelem += array.Device[i].IPAddr + '-' + devName +'<br/>'+array.Device[i].DevType + '<br/>' + array.Device[i].IPAddr + '</div></td>';
			else
				$baseelem += array.Device[i].Name + '-' + devName +'<br/>'+array.Device[i].DevType + '<br/>' + array.Device[i].IPAddr + '</div></td>';
		}
		else
			$baseelem += '<tr><td style="padding-right:10px;"><div class="networklayouttextbox">'+array.Device[i].Name+'<br/>'+array.Device[i].DevType+'<br/>'+array.Device[i].IPAddr+'</div></td>';

		$baseelem += '<td align="center"><img src="' + deviceImage + '" width=50px></img></td>';
		$baseelem += '<td><div class = "networklayoutpriorityline cmnlineclass cmnhorizontalline"></div></td></tr>';
	}

	$('.networklayoutmapleftcol tbody tr').remove();
	$('.networklayoutmapleftcol tbody').append($baseelem);

	/* right side Layout Page*/
	$baseelem ='';
	for(i = 1;i < array.Device.length; i+=2) {
		deviceImage = GetIcon("devtype", array.Device[i].DevTypeInt);
		devName = array.Device[i].DevName;
		$baseelem += '<tr><td><div class = "networklayoutpriorityline cmnlineclass cmnhorizontalline"></div></td>';
		$baseelem += '<td align="center" style="padding-right:10px;"><img src="' + deviceImage + '" width=50px></img></td>';

		if(devName != undefined) {
			$baseelem += '<td><div class="networklayouttextbox">';
			if(array.Device[i].Name != "")
				$baseelem += array.Device[i].Name + '-' + devName +'<br/>'+array.Device[i].DevType+'<br/>'+array.Device[i].IPAddr+'</div></td></tr>';
			else
				$baseelem += array.Device[i].IPAddr + '-' + devName +'<br/>'+array.Device[i].DevType+'<br/>'+array.Device[i].IPAddr+'</div></td></tr>';
		}
		else
			$baseelem += '<td><div class="networklayouttextbox">'+array.Device[i].Name+'<br/>'+array.Device[i].DevType+'<br/>'+array.Device[i].IPAddr+'</div></td></tr>';
	}

	$('.networklayoutmaprightcol tbody tr').remove();
	$('.networklayoutmaprightcol tbody').append($baseelem);

	var height = $('.networklayoutmapleftcol').height();
	if (height <= 300)
		height = 300;
	$('.separator').attr('rowspan',1);
	$('.separator').find('div.networklayoutseperator').height(height);
}

/*****
* Function to populate DeviceList .END
*****/
function labelFormatter(label, series) {
		return "<div style='font-size:8pt; text-align:center; padding:2px; color:white;'>" + Math.round(series.percent) + "%</div>";
}
/*****
* Function to Draw UpStream Pie Chart .START
*****/
function drawUpstreamPie()
{
	var pieChartData = [];
	var i;
	var zeroPie = false;

	for(i = 0; i < gDownBWUpBW.length; i++){
		if(gDownBWUpBW[i].UpBW > 0) {
			zeroPie = true;
			break;
		}
	}

	if(zeroPie == true) {
		for(i = 0; i < gDownBWUpBW.length; i++){
			pieChartData.push({
				data: (gDownBWUpBW[i].UpBW),
				label: gDownBWUpBW[i].Name != ""?gDownBWUpBW[i].Name:gDownBWUpBW[i].IPAddr
			});
		}
	}
	else {
		pieChartData.push({
				data: 10,
				color: '#C0C0C0',
				label: 'Unused'
		});
	}

	var options = {
			series: {
				pie: {
					show: true,
					radius: 1,
					label: {
						show: true,
						radius: 3/4,
						formatter: labelFormatter,
						background: {
							opacity: 0.5
						}
					}
				}
			},
			grid: {
					  hoverable: true
					},
					tooltip: true,
					tooltipOpts: {
						content: "%p.0%, %s", // show percentages, rounding to 2 decimal places
						shifts: {
							x: 20,
							y: 0
						},
						defaultTheme: false
					},
				legend: {
						show: true,
						palcement: 'outsideGrid',
						container: $("#upstreampiechartlegend")
					}
         };

    $.plot($("#upstreampiechart"), pieChartData, options);
}
/*****
* Function to Draw UpStream Pie Chart .END
*****/

/*****
* Function to Draw DownStream Pie Chart .END
*****/
function drawDownstreamPie()
{
	var pieChartData = [];
	var i;
	var zeroPie = false;

	for(i = 0; i < gDownBWUpBW.length; i++){
		if(gDownBWUpBW[i].UpBW > 0) {
			zeroPie = true;
			break;
		}
	}
	if(zeroPie == true) {
		for(i = 0; i < gDownBWUpBW.length; i++){
			pieChartData.push({
				data: (gDownBWUpBW[i].DownBW),
				label: gDownBWUpBW[i].Name != ""?gDownBWUpBW[i].Name:gDownBWUpBW[i].IPAddr
			});
		}
	}
	else {
		pieChartData.push({
				data: 10,
				color: '#C0C0C0',
				label: 'Unused'
			});
		}

		var options = {
			series: {
						pie: {
							show: true,
							radius: 1,
							label: {
								show: true,
								radius: 3/4,
								formatter: labelFormatter,
								background: {
									opacity: 0.5
								}
							}
						}
					},
					 grid: {
						hoverable: true
					},
					tooltip: true,
					tooltipOpts: {
						content: "%p.0%, %s", // show percentages, rounding to 2 decimal places
						shifts: {
							x: 20,
							y: 0
						},
						defaultTheme: false
					},
					legend: {
						show: true,
						palcement: 'outsideGrid',
						container: $("#downstreampiechartlegend")
					}
			};

    $.plot($("#downstreampiechart"), pieChartData, options);
}
/*****
* Function to Draw DownStream Pie Chart .END
*****/
/*****
* Function which will get called when DOM for the page gets loaded.START
*****/
$(document).ready(function()
{
	initPage();

	for(i=0;i<gDeviceListTimers.length;i++){
		clearTimeout(gDeviceListTimers[i]);
	}
	gDeviceListTimers = [];

	$("#testbandwidthbtn").click(function() {
		window.open("http://www.speedtest.net/", '_blank');
	});

	getConfiguration();

	$('#checkautobandwidth').click(function(event) {
		if ($(this).is(":checked")) {
			$("#bandwidthenabledid").removeClass('bandwidthdisabled');
			$("#bandwidthenabledid").addClass('bandwidthenabled');
			$("#downloadspeedtext").attr("disabled", "disabled");
			$("#uploadspeedtext").attr("disabled", "disabled");
		}
		else {
			$("#bandwidthenabledid").removeClass('bandwidthenabled');
			$("#bandwidthenabledid").addClass('bandwidthdisabled');
			$("#downloadspeedtext").removeAttr("disabled");
			$("#uploadspeedtext").removeAttr("disabled");
		}
	});


	$( "#quicksubmitbtn" ).click(function() {
		/* Show the progress bar */
		$(".cmnprogressbar").showProgressBar();
		setConfiguration();
		getConfiguration();
	});

	$( "#quickresetbtn" ).click(function() {
		getConfiguration();
	});

	$("#checkenableiqos").click(function() {
		if ($("#checkenableiqos").is(':checked')) {
			$('[name="autoenablecheck"]')[0].disabled = false;
			$("#auto_bw_lbl").removeClass('disabled');
			if ($("#checkautobandwidth").is(':checked')) {
				$("#downloadspeedtext").attr("disabled", "disabled");
				$("#uploadspeedtext").attr("disabled", "disabled");
				$("#bandwidthenabledid").removeClass('bandwidthdisabled');
				$("#bandwidthenabledid").addClass('bandwidthenabled');
			}
			else {
				$('[name="autoenablecheck"]')[0].disabled = false;
				$("#downloadspeedtext").removeAttr("disabled");
				$("#uploadspeedtext").removeAttr("disabled");
				$("#bandwidthenabledid").removeClass('bandwidthenabled');
				$("#bandwidthenabledid").addClass('bandwidthdisabled');
			}
		}
		else {
			$('[name="autoenablecheck"]')[0].disabled = true;
			$("#downloadspeedtext").attr("disabled", "disabled");
			$("#uploadspeedtext").attr("disabled", "disabled");
			$("#bandwidthenabledid").removeClass('bandwidthdisabled');
			$("#bandwidthenabledid").addClass('bandwidthenabled');
			$("#auto_bw_lbl").addClass('disabled');
		}
	});
	getDeviceList();
});
