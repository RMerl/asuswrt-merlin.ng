/*
 * Jquery implementation for Zero Wait DFS Demo Tab
 *
 * $ Copyright Open Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: zerowaitdfs.js 682079 2017-01-31 10:30:10Z spalanga $
 */

var isHTTPDWebserver = 1;
var g_urlGetAPStatus = ''; /* URL of webserver */
var g_wlName = ''; /* Name of the wl interface on which zero wait dfs is supported */
var g_isZWDSupported = 0; /* Whether the zero wait DFS is supported or not. Getting from server */
var g_zwdTimeOutVal = 5000; /* Timeout value for calling AP status */
var g_apStatusCallCount = 0;

var g_timerAPStatus = []; /* To Hold the settimeout timer */

/* All the request names in common object */
var g_zwdRequestNames = {
	ZWD_REQUEST : 'VisReqZWD',
	ZWD_IS_SUPPORTED : 'VisZWDIsSupported',
	ZWD_GET_DFS_CHANNELS : 'VisZWDGetDFSChannels',
	ZWD_GET_AP_STATUS : 'VisZWDGetApStatus',
	ZWD_MOVE_TO_DFS : 'VisZWDMoveToDFS',
	ZWD_SIMULATE_RADAR : 'VisZWDSimulateRadar'
};

/* To initialize some variables on page load */
function initZWDPage()
{
	if (isHTTPDWebserver == 1) {
		g_urlGetAPStatus = window.location.protocol + '//' + window.location.host +'/json.cgi';
	} else {
		g_urlGetAPStatus = 'http://localhost/zerowaitdfs.php';
	}
}

/* Clear all the timers */
function clearAllZWDTimers()
{
	for (i = 0; i < g_timerAPStatus.length; i++) {
		clearTimeout(g_timerAPStatus[i]);
	}
	g_timerAPStatus = [];
}

/* Parse the DFS channel response and populate select box */
function parseDFSChannelResp(ctrlId, array)
{
	var html = '';

	if (array.DFSChannels.length > 0) {
		html += '<option value="0">Select Channel</option>';
		for (var i = 0; i < array.DFSChannels.length; i++) {
			html += '<option value="' + array.DFSChannels[i] + '">';
			html += array.DFSChannels[i]+'</option>';
		}
		/* Remove the existing items */
		$(ctrlId).html('');
		/* Append the new items */
		$(ctrlId).append(html);
	} else {
		html = '<option>Select DFS Channel</option>'
		$(ctrlId).html(html);
	}
}

/* Parse Zero wait dfs supported response */
function parseIsZWDSupportedResp(array)
{
	g_isZWDSupported = array.IsSupported;
	/*if (g_isZWDSupported == 0) {
		$("#zerowaitdfscontent").hide();
		var errorelem = '<div id="errormsgid" class="errormsg"><center><b>'
		errorelem += 'Zero Wait DFS is not supported</b></center></div>';
		$("#showerrmessage").append(errorelem);
		return;
	}*/
	g_wlName = array.wlname;
}

/* Function for populating the associated station's to a particular AP in the table */
function drawAssociatedSTATable(array) {
	$('.stalisttblclass tbody tr').remove();
	for (i = 0; i < array.length; i++) {
		element = '<tr><td>' + (i + 1) + '</td>';
		element += '<td align="left">' + array[i].MAC + '</td>';
		element += '<td align="right" style="padding-right:10%;">' + array[i].RSSI + '</td>';
		element += '</tr>';
		$('.stalisttblclass tbody').append(element);
	}
}

/* Parses the AP status response and populates the UI */
function parseAPStatusResp(array)
{
	/* Update primary core div fields */
	$('.primarycoretbl tbody tr').remove();

	/* Add Channel */
	baseelem = '<tr><td align="right"><b>Channel : </b></td><td>&nbsp;'
	baseelem += array.APStatus.PrimaryCore.Channel + '</td></tr>';
	/* Add Tx / Rx Chains */
	baseelem += '<tr><td align="right"><b>Tx / Rx Chains : </b></td><td>&nbsp;'
	baseelem += array.APStatus.PrimaryCore.Chains + '</td></tr>';
	/* Add Phy rate */
	baseelem += '<tr><td align="right"><b>Phy Rate : </b></td><td>&nbsp;'
	baseelem += array.APStatus.PrimaryCore.PhyRate + '</td></tr>';
	/* Add n rate */
	baseelem += '<tr><td align="right"><b>NRate : </b></td><td>&nbsp;'
	baseelem += array.APStatus.PrimaryCore.nrate + '</td></tr>';

	$('.primarycoretbl tbody').append(baseelem);

	$('.scancoretbl tbody tr').remove();
	/* If the DFS move is happening show the scan core div and update the values */
	if (array.APStatus.DFSProcessing == 1) {
		g_apStatusCallCount = 0;
		g_zwdTimeOutVal = 1000; /* If scan core is active fetch every 1 second */
		$('.scancoredivclass').show();
		/* Add Channel */
		baseelem = '<tr><td align="right"><b>Channel : </b></td><td>&nbsp;'
		baseelem += array.APStatus.ScanCore.Channel + '</td></tr>';
		/* Add Tx/ Rx chains */
		baseelem += '<tr><td align="right"><b>Tx / Rx Chains : </b></td><td>&nbsp;'
		baseelem += array.APStatus.ScanCore.Chains + '</td></tr>';
		/* Add Time elapsed */
		baseelem += '<tr><td align="right"><b>Time Elapsed : </b></td><td>&nbsp;'
		baseelem += array.APStatus.ScanCore.Elapsed + ' Seconds</td></tr>';

		$('.scancoretbl tbody').append(baseelem);
	} else { /* else hide the scan core div */
		$('.scancoredivclass').hide();
		/* checking g_apStatusCallCount is to continue get ap status for 5 more seconds */
		if ((g_zwdTimeOutVal == 1000) && (g_apStatusCallCount > 5)) {
			g_zwdTimeOutVal = 5000; /* Change it back to 5 seconds as we are not in scan core */
		}
		g_apStatusCallCount++;
	}
}

/* Parse Move to DFS channel response */
function parseMoveToDFSResp(array)
{
	if (array.Status == "Success") {
		g_zwdTimeOutVal = 1000; /* Call every 1 second till we get DFS processing result */
		getAPStatus();
	} else { /* Show alert only in case of failure */
		alert(array.Status);
	}
}

/* Parse Simulate Radar response */
function parseSimulateRadarResp(array)
{
	if (array.Status == "Success") {
		getAPStatus();
	} else { /* Show alert only in case of failure */
		alert(array.Status);
	}
}

/* Common function to parse all the response from server */
function parseZWDResponse(array)
{
	if (array.Req == g_zwdRequestNames.ZWD_GET_DFS_CHANNELS) {
		parseDFSChannelResp('#dfschannelselectctrl', array);
	} else if (array.Req == g_zwdRequestNames.ZWD_IS_SUPPORTED) {
		parseIsZWDSupportedResp(array);
	} else if (array.Req == g_zwdRequestNames.ZWD_GET_AP_STATUS) {
		parseAPStatusResp(array);
	} else if (array.Req == g_zwdRequestNames.ZWD_MOVE_TO_DFS) {
		parseMoveToDFSResp(array);
	} else if (array.Req == g_zwdRequestNames.ZWD_SIMULATE_RADAR) {
		parseSimulateRadarResp(array);
	}
}

/* Common function to request from the server */
function getZWDDetFromServer(request)
{
	var array = [];

	$.ajax({
		type:'GET',
		url:g_urlGetAPStatus,
		data:request,
		async:false,
		success:function(result) {
			if (isHTTPDWebserver == 1)
				array = result;
			else
				array = JSON.parse(result);
			parseZWDResponse(array);
		}
	});
	array = [];
}

/* Get all DFS (Radar) Channels from driver */
function getAllDFSChannels()
{
	var request = {
		"Req" : g_zwdRequestNames.ZWD_REQUEST,
		"ZWDType" : g_zwdRequestNames.ZWD_GET_DFS_CHANNELS,
		"wlname" : g_wlName
	};

	getZWDDetFromServer(request);
}

/* Check whether zero wait dfs is supported or not */
function getIsZWDSupported()
{
	var request = {
		"Req" : g_zwdRequestNames.ZWD_REQUEST,
		"ZWDType" : g_zwdRequestNames.ZWD_IS_SUPPORTED
	};

	getZWDDetFromServer(request);
}

/* Get the status of ZWD */
function getAPStatus()
{
	var request = {
		"Req" : g_zwdRequestNames.ZWD_REQUEST,
		"ZWDType" : g_zwdRequestNames.ZWD_GET_AP_STATUS,
		"wlname" : g_wlName
	};

	clearAllZWDTimers();
	getZWDDetFromServer(request);
	g_timerAPStatus.push(setTimeout(function(){getAPStatus(g_zwdTimeOutVal);}, g_zwdTimeOutVal));
}

/* On clicking Move to DFS button */
function onClickSwitchToDFSButton()
{
	var channel = $('#dfschannelselectctrl').val();

	if (channel == 0)
		alert('Select DFS Channel');
	else {
		var request = {
			"Req" : g_zwdRequestNames.ZWD_REQUEST,
			"ZWDType" : g_zwdRequestNames.ZWD_MOVE_TO_DFS,
			"Channel" : channel,
			"wlname" : g_wlName
		};
		getZWDDetFromServer(request);
	}
}

/* On clicking simulate radar button */
function onClickSimulateRadarButton()
{
	var core = $('#coreselectctrl').val();

	if (core == 0)
		alert('Select Core');
	else {
		var request = {
			"Req" : g_zwdRequestNames.ZWD_REQUEST,
			"ZWDType" : g_zwdRequestNames.ZWD_SIMULATE_RADAR,
			"Core" : core,
			"wlname" : g_wlName
		};
		getZWDDetFromServer(request);
	}
}

/*****
* Function which will get called when DOM for the page gets loaded.START
*****/
$(document).ready(function() {
	clearAllZWDTimers();
	initZWDPage();

	/* Hide scan core div initially. It will be shown depending on the server output */
	$('.scancoredivclass').hide();
	getIsZWDSupported();
	//if (g_isZWDSupported) {
		getAllDFSChannels();
		getAPStatus();
	//}
});

/* All callback functions (for ex : button clicks) */
$(function() {
	/* On clicking Switch to DFS button */
	$('#switchtodfsbtn').click(function(event) {
		onClickSwitchToDFSButton();
	});

	/* On clicking Simulate Radar button */
	$('#triggerradarbtn').click(function(event) {
		onClickSimulateRadarButton();
	});
});

$(window).bind('beforeunload',function() {
	$('#dfschannelselectctrl > option:eq(0)').prop('selected', true);
	$('#coreselectctrl > option:eq(0)').prop('selected', true);
})

/* Function to load the top menu bar */
function zeroWaitDFSTopNavBar()
{
	document.write(
		'<div id="topnav" class="blackbrdrd">\
			<div id="containerdiv">\
				<ul class="custommenu">\
					<li><a href="index.asp">Home</a></li>\
					<li class="sitesurveytab">\
						<a href="visindex.asp"  class="selected">Site Survey</a>\
					</li>\
					<li class="chnlcapacitytab">\
						<a href="channelcapacity.asp">Channel Statistics</a>\
					</li>\
					<li class="metricstab">\
						<a href="metrics.asp">Advanced Troubleshooting</a>\
					</li>\
					<li class="zerowaitdfs">\
						<a href="zerowaitdfs.asp">Zero Wait DFS</a>\
					</li>\
					<li>\
						<a href="configure.asp">Configure</a>\
					</li>\
				</ul>\
			</div>\
		</div>'
	);
}