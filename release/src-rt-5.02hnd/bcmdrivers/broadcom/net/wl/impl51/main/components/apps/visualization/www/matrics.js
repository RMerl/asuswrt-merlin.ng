/*
 * Jquery implementation for Advanced Troubleshooting Tab
 *
 * $ Copyright Open Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: matrics.js 624114 2016-03-10 09:01:50Z pankajj $
 */

var staarr = [];
var staclkarr = [];
var timerassocsta = [];
var mac;
var timerval = unescape($.cookie('timerinterval'));
var timerdata = [];
var timercounters = [];
var g_timerampdu = [];
var packetQueueData = [];
var packetQueueSTAsData = [];
var remoteIPAddress = '';
var isRemoteEnabled = 0;
var urlremotesettings = '';
var urlapstainfo = '';
var urlglitchcounter = '';
var urlpacketqueuestats = '';
var urlampdu = '';
var isHTTPDWebserver = 1;

var ampduPiechartData = []; /* For storing all AMPDU data to show it in tooltip */
var gAMPDUXYLabel = []; /* For storing AMPDU's X-Axis and Y-Axis labels to show it in tooltip */
var gChanimData = ''; /* For storing all chanim data for customized chanim graph */

var gAllBandsArr = []; /* For storing the bands and its SSID and BSSID's */
var g_noerrmsg = 0; /* redraws the sta pkt queue stats layout for sta rrm stats */
var gSTADetTableIDs = [];

var TIMER_AMPDU	= 1 << 0;
var TIMER_ASSOCSTA = 1 << 1;
var TIMER_COUNTERS = 1 << 2;

if (timerval.length > 0) {
	timerdata = timerval.split(',');
}
timerdata[0] = Number(timerdata[0]);

/* To initialize global variables */
function initPage()
{
	if (isHTTPDWebserver == 1) {
		urlremotesettings = 'http://'+ window.location.host +'/json.cgi';
		urlapstainfo = 'http://'+ window.location.host +'/json.cgi';
		urlglitchcounter  = 'http://'+ window.location.host +'/json.cgi';
		urlpacketqueuestats = 'http://'+ window.location.host +'/json.cgi';
		urlampdu = 'http://'+ window.location.host +'/json.cgi';
	} else {
		urlremotesettings = 'http://localhost/remotesettings.php';
		urlapstainfo = 'http://localhost/stationdetails.php';
		urlglitchcounter = 'http://localhost/Matrics.php';
		urlpacketqueuestats = 'http://localhost/Matrics.php';
		urlampdu = 'http://localhost/Matrics.php';
	}

	initCommonPage();
}

/* Clear all the timers based on the flag */
function clearAllMetricsTimers(flag)
{
	if (flag & TIMER_AMPDU) {
		for (i = 0; i <g_timerampdu.length; i++) {
			clearTimeout(g_timerampdu[i]);
		}
		g_timerampdu = [];
	}

	if (flag & TIMER_ASSOCSTA) {
		for (i = 0; i < timerassocsta.length; i++) {
			clearTimeout(timerassocsta[i]);
		}
		timerassocsta = [];
	}

	if (flag & TIMER_COUNTERS) {
		for (i = 0; i < timercounters.length; i++) {
			clearTimeout(timercounters[i]);
		}
		timercounters = [];
	}
}

/* Function checks whether remote debugging is enabled or not */
function isRemoteDebuggingEnabled()
{
	if (isRemoteEnabled == 1 && remoteIPAddress != "127.0.0.1")
		return 1;
	return 0;
}

/* Gets the remote debugging settings from the cookies */
function retrieveRemoteDebug()
{
	var tmpcokie1, tmpcokie2;

	tmpcokie1 = unescape($.cookie('remoteenabledflag'));
	tmpcokie2 = unescape($.cookie('remotedconip'));
	if (tmpcokie1.length > 0) {
		isRemoteEnabled = Number(tmpcokie1.split(','));
	} else {
		isRemoteEnabled = 0;
	}
	if (tmpcokie2.length > 0) {
		remoteIPAddress = tmpcokie2.split(',');
	} else {
		remoteIPAddress = '';
	}
	if (isRemoteDebuggingEnabled()) {
		$("#frequencybandselector").hide();
		$(".ampdutable").hide();
		$("#countermatrics").hide();
		var errorelem = '<div id="errormsgid" class="errormsg">Remote debugging is enabled. Please go to <b>"'+remoteIPAddress+'"</b>.</div>';
		$("#showerrmessage").append(errorelem);
		return 1;
	}
	return 0;
}

/* To show the station's detail table(packet queue stats or glitch count of STA etc...
 * This is on clicking the station
 */
function showStationsDetailTable()
{
	$("#advancedperstadetdiv").show();
	$("#packetqueuestatsgrphtbl").show();
}

/* To hide the station's detail table */
function hideStationsDetailTable()
{
	$("#advancedperstadetdiv").hide();
	$("#packetqueuestatsgrphtbl > tbody > tr").remove();
	$("#packetqueuestatsgrphtbl").hide();

	/* Clear all the STA's detailed graphs sub elements */
	for (i = 0; i < gSTADetTableIDs.length; i++) {
		$("#"+gSTADetTableIDs[i]+" > tbody > tr").remove();
	}
	gSTADetTableIDs = [];
	/* Clear all the elements from DIV which is used for holder STA's advanced details */
	$("#advancedperstadetdiv").empty();
}

/* On band selection changed */
function onChannelBandSelection(layoutreq, selectedBand)
{
	if (isRemoteDebuggingEnabled())
		return;

	clearAllMetricsTimers(TIMER_AMPDU | TIMER_ASSOCSTA | TIMER_COUNTERS);

	mac = 0;
	staclkarr = [];
	DisplayStaApInfo(selectedBand);
	plotthecounters(layoutreq, selectedBand);
	makeAMPDURequest(layoutreq, selectedBand);

	hideStationsDetailTable();
}

$(document).ready(function() {
	$("#bandselectcntrl").change(function() {
		var selectedBand = getBandChangeData(this, gAllBandsArr);
		onChannelBandSelection(false, selectedBand);
	});
	initPage();
	if (retrieveRemoteDebug() == 1)
		return;
	hideStationsDetailTable();
	gAllBandsArr = getBands();
	parseBandsAndInsert('#bandselectcntrl', gAllBandsArr);
	/* If there are atleast one interface(2G or 5G select first one by default */
	if (gAllBandsArr.length > 0) {
		onChannelBandSelection(true, gAllBandsArr[0]);
	}

   $('tr').click(function () {
      $(this).siblings('tr').removeClass('hilite');
      $(this).toggleClass('hilite');
   });
});

/* Gets the Associated STA information from server */
function DisplayStaApInfo(selectedBand) {
	var array = [];
	var macarr = [];

	clearAllMetricsTimers(TIMER_ASSOCSTA);

	$.ajax({
		type:"GET",
		url:urlapstainfo,
		data:{Req:'AssociatedSTA',DutMac:selectedBand.mac, FreqBand:selectedBand.band},
		async:true,
		timeout:5000,
		success:function(result) {
			if (isHTTPDWebserver == 1)
				array = result;
			else
				array = JSON.parse(result);
			macarr = drawAP(array, mac);
			staarr = macarr.slice();
			drawStainfoTable(array);
			selectSTAinTable();
		}
	});
	if (selectedBand.mac == $("#bandselectcntrl").val())
		timerassocsta.push(setTimeout(function(){DisplayStaApInfo(selectedBand);},timerdata[0]));

	array = [];
	macarr = [];
}

/* Function for populating the associated station's to a particular AP in the table */
function drawStainfoTable(array) {
	$('.apstainfolisttable tbody tr').remove();
	for (i = 0; i < array[1].length; i++) {
		$element = '<tr><td>' + (i + 1) + '</td>';
		$element = $element + '<td align="left">' + array[1][i].MAC + '</td>';
		$element = $element + '<td align="right" style="padding-right:10%;">' + array[1][i].RSSI + '</td>';
		$element = $element + '<td align="right" style="padding-right:10%;">' + array[1][i].PhyRate + '</td>';
		$element = $element + '</tr>';
		$('.apstainfolisttable tbody').append($element);
	}
}

/* Common function which helps in computing the layout of the ampdu graph
 * as to how many piecharts needs to be drawn in a row
 */
function computeAMPDULayout(start, end, data)
{
	var $element = "<tr>", $element1 = "<tr>";

	if (data != null) {
		if (end > 5)
			end = end -1;
		for (var headerindex = start-1; headerindex < end; headerindex++) {
			$element = $element + '<th>' + data[0][headerindex].BarHeading + '</th><th></th>';
		}
		$element = $element + '</tr>';
		for (var ampdudataindex = start-1; ampdudataindex < end; ampdudataindex++) {
			if (ampdudataindex != 0) {
				$element1 = $element1 + '<td><div class="ampdu-container"> <div id = "placeholder'
				+ ampdudataindex + '" class="ampdu-placeholder"></div></div></td><td><div id="ampdulegend'
				+ ampdudataindex + '" class="ampdulegend-class"><div></td>';
			} else {
				$element1 = $element1 + '<td><div class="ampdu-container"> <div id = "placeholder'
				+ '" class="ampdu-placeholder"></div></div></td><td><div id="ampdulegend" '
				+ 'class="ampdulegend-class"></div></td>';
			}
		}
		$element1 = $element1 + '</tr>';
		$(".ampdutable").append($element+$element1);
	}
}

/* Creates the AMPDU layout */
function createAMPDUTable(data) {
	/* the start value and end value we send to computeAMPDULayout decides how many pie charts you want to draw in a row
	 * here since we want 5 pie charts in a row we give values 1-5, 6-10 etc
	 */
	for (var index = 1; index < data[0].length; index = index + 5)
		computeAMPDULayout(index, index + 4, data);
}

/* Label formatter function for AMPDU labels */
function labelFormatter(label, series) {
	return "<div style='font-size:8pt; text-align:center; padding:2px; color:white;'>" + label + "<br/>" + (series.percentage).toFixed(2) + "%</div>";
}

/* Draws the pie chart for AMPDU stats */
function drawPieChart(islayoutreq, data) {
		var piePlaceHolder = "#placeholder";
		var pieContainer = "#ampdu-container";
		var pieLegendContainer = "#ampdulegend";
		var pieChartsData = [];
		var pieChartXYLabel = [];

		if (islayoutreq == true)
			createAMPDUTable(data);
		if (data.length != 0) {
			for (var outerindex = 0; outerindex < data[0].length; outerindex++) {
				var pieChartData = [];
				for (var innerindex = 0; innerindex < data[0][outerindex].XValue.length; innerindex++) {
					/* If both rate and percent is zero don't plot */
					if (data[0][outerindex].YValue[0][innerindex] == 0 && data[0][outerindex].YValue[1][innerindex] == 0)
						continue;
					pieChartData.push({
						label: data[0][outerindex].XValue[innerindex],
						data: data[0][outerindex].YValue[0][innerindex],
						percentage:data[0][outerindex].YValue[1][innerindex]
					});
				}
				pieChartsData.push(pieChartData);
				pieChartXYLabel.push({
					xaxis: data[0][outerindex].XAxis,
					yaxis: data[0][outerindex].YAxis
				});
				pieChartData = "";
			}
			for (var index = 0; index < data[0].length; index++) {
				if (index != 0) {
					piePlaceHolder = "#placeholder" + index;
					pieContainer = "#ampdu-container" + index;
					pieLegendContainer = "#ampdulegend" + index;
				}
				$.plot(piePlaceHolder, pieChartsData[index], {
					series: {
						pie: {
								show: true,
								radius: 1,
								label: {
									show: true,
									radius: 2/3,
									formatter: labelFormatter,
									threshold: 0.1
								}
							},
					},
					legend: {
						show: true,
						palcement: 'outsideGrid',
						container:pieLegendContainer
					},
					grid: {
						hoverable: true
					}
				});
				displayTooltipForAMPDU(piePlaceHolder);
			}
		}
		ampduPiechartData = pieChartsData;
		gAMPDUXYLabel = pieChartXYLabel;
		pieChartsData = "";
		pieChartXYLabel = "";
}

/* Function to show the tooltip for AMPDU piechart graph */
function displayTooltipForAMPDU(placeholder)
{
	$(placeholder).bind("plothover",function(event,pos,item) {
		$("#tooltipdiv").remove();
		if (item) {
			var piePlaceHolder = "#placeholder";
			var index = 0;
			var tooltip = '';

			/* All ampdu stats data is stored in global array. To find which ampdu stats tooltip
			 * to show, we need to get the index. To get the index compare the placeholders
			 * If matches take the index
			 */
			if (piePlaceHolder != placeholder) {
				for (index = 0; ampduPiechartData.length; index++) {
					piePlaceHolder = "#placeholder" + index;
					if (piePlaceHolder == placeholder)
						break;
				}
			}
			if (index == ampduPiechartData.length) /* If there is no index found */
				return;

			/* Create a table for all the MCS index, MCS rate and percentage */
			tooltip += '<table style="padding:5px"><tr>';
			tooltip += '<th style="padding-right:5px">'+gAMPDUXYLabel[index].xaxis+'</th>';
			tooltip += '<th>'+gAMPDUXYLabel[index].yaxis+'</th>';
			tooltip += '<th style="padding-left:5px">Percent</th></tr>';
			tooltip += '<tbody>';
			for (var i = 0; i < ampduPiechartData[index].length; i++) {
				if (item.series.label == ampduPiechartData[index][i].label) {
					tooltip += '<tr><td><u><b>' + ampduPiechartData[index][i].label + '</b></u></td>'
					tooltip +=  '<td align="right"><u><b>' + ampduPiechartData[index][i].data + '</b></u></td>';
					tooltip +=  '<td align="right"><u><b>' + ampduPiechartData[index][i].percentage + '</b></u></td></tr>';
				} else {
					tooltip += '<tr><td>' + ampduPiechartData[index][i].label + '</td>'
					tooltip +=  '<td align="right">' + ampduPiechartData[index][i].data + '</td>';
					tooltip +=  '<td align="right">' + ampduPiechartData[index][i].percentage + '</td></tr>';
				}
			}
			tooltip += '</tbody></table>';

			$('<div class="tooltip border fontfamly" id="tooltipdiv" style = "background-color:'+item.series.color+'">' + tooltip + '</div>')
				.css({
						top: (item.pageY - 30) + 'px',
						left: (item.pageX + 10) + 'px',
						opacity: 0.9,
						width: '200px'})
					.appendTo(placeholder).fadeIn(200);
		}
	});
}

/* Request for AMPDU stats to server */
function makeAMPDURequest(islayoutreq, selectedBand) {
	var shared = unescape($.cookie('matricscntr'));
	var req;
	var array = [];

	if (shared.length > 0) {
		array = shared.split(',');
	}
	// if "AMPDU TX Without SGI" is present in the cookie then
	// only make the request to the backend for AMPDU stats
	if (array[0] == 'AMPDU TX Without SGI') {
		clearAllMetricsTimers(TIMER_AMPDU);

		req = 'AMPDUStatistics';
		$.ajax({
			type:"GET",
			async:true,
			timeout:5000,
			url:urlampdu,
			data:{Req:req,DutMac:selectedBand.mac, FreqBand:selectedBand.band},
			success:function(result) {
				if (isHTTPDWebserver == 1)
					array = result;
				else
					array = JSON.parse(result);
				drawPieChart(islayoutreq, array);
				islayoutreq = false;
			}
		});
		if (selectedBand.mac == $("#bandselectcntrl").val())
			g_timerampdu.push(setTimeout(function(){makeAMPDURequest(islayoutreq, selectedBand);},timerdata[0]));
	}
}

/* This draws the layout for customized graph. Now only for chanim.
 * This creates the graph placeholder and also all the checkboxes.
 */
function drawCustomizedLayoutforCounters(data, islayoutreq)
{
	if (islayoutreq == true) {
		var elementadd = '';
		var nCountInRow = 15;

		$element ='<table class="ampdustatstable fontfamly cmngraphbox">';
		$element += '<thead class="cmngraphboxbkgrnd-h1"><tr><th>'+data.Heading+'</th></tr></thead>';
		$element += '<tbody><tr><td width="90%">';
		elementadd += '<table><tr>';
		/* Each check box is created in a table with each column for each check box
		 * This is to align all the checkboxes properly
		 */
		for (var index = 0; index < data.Total; index++) {
			/* In one row add only 7 columns. So find out where to add the next row */
			if ((index % nCountInRow) == 0)
				elementadd += '</tr><tr>';
			elementadd += '<td><input type="checkbox" class="cmncheckboxclass" id="checkchanimid" ';
			elementadd += 'style="width:25px;height:15px;margin-left:30px;" ';
			elementadd += 'value=' + '"'+ data.BarHeading[index] + '">' + data.BarHeading[index];
			elementadd += '</td>';
		}
		elementadd += '</tr></table>';
		$element += elementadd;
		$element += '</td></tr><tr><td width="90%">';
		$element += '<div id=' +'"'+ data.Placeholderid +'"'+' style="width:90%;height:400px; padding:0px; position:relative;"></div></td>';
		$element += '</tr></tbody></table>';
		$("#counterstatsplace").append($element);
	}
}

/* To draw chanim statistics */
function drawTheChanimCounters(array, islayoutreq, selectedBand)
{
	var data;
	var req;
	var checkedArray = [];

	req = 'Chanim Statistics';
	/* If chanim statistics is not checked in config page dont draw the graph */
	if ($.inArray(req, array) == -1)
		return;

	$.ajax({
		type:"GET",
		async:true,
		timeout:5000,
		url:urlglitchcounter,
		data:{Req:req, DutMac:selectedBand.mac, FreqBand:selectedBand.band},
		success:function(result) {
			if (isHTTPDWebserver == 1)
				data = result;
			else
				data = JSON.parse(result);
			drawCustomizedLayoutforCounters(data[0][0],islayoutreq);

			/* Get all the check boxes selected */
			$.each($("#checkchanimid:checked"),function() {
				checkedArray.push($(this).val());
			});

			gChanimData = data[0][0]; /* store it in global variable for later use */
			drawTimeSeriesStatsForCustom(checkedArray, gChanimData, "#"+gChanimData.Placeholderid);
		}
	});
}

/* Draws glitch counter, chanim and packet queue stats */
function plotthecounters(islayoutreq, selectedBand) {
	var shared = unescape($.cookie('matricscntr'));
	var array = [];

	clearAllMetricsTimers(TIMER_COUNTERS);
	if (shared.length > 0) {
		array = shared.split(',');
	}
	if (array.length > 0) {
		index = 0;
		drawthecounter(array, index, islayoutreq, selectedBand);
	}
	drawTheChanimCounters(array, islayoutreq, selectedBand);
	// only when a sta is selected then along with the glitch counter
	// draw the packetqueue stats so that the xaxis values matches
	if (mac != 0) {
		pullAdvancedPerSTADet(false, selectedBand);
	}
	if (selectedBand.mac == $("#bandselectcntrl").val())
		timercounters.push(setTimeout(function(){plotthecounters(false,selectedBand);},timerdata[0]));
}

/* Requests glitch counter statistics from server */
function drawthecounter(array, index, islayoutreq, selectedBand) {
	var data;
	var req;
	req = 'Glitch Counter Statistics';
	$.ajax({
		type:"GET",
		async:true,
		timeout:5000,
		url:urlglitchcounter,
		data:{Req:req, DutMac:selectedBand.mac, FreqBand:selectedBand.band},
		success:function(result) {
			if (isHTTPDWebserver == 1)
				data = result;
			else
				data = JSON.parse(result);
			drawLayout(data[0][0],islayoutreq);
			drawTimeSeriesStats(data[0][0], "#"+data[0][0].Placeholderid, true, true);
		}
	});
}

/* Draws layout for glicth counter statistics */
function drawLayout(data, islayoutreq) {
	if (islayoutreq == true) {
		var $element ='<table class="ampdustatstable fontfamly cmngraphbox"><thead class="cmngraphboxbkgrnd-h1"><tr><th>'+data.Heading+'</th></tr></thead><tbody>';
		for (index = 0; index<data.Total; index++) {
			var tempData = [];
			if ((index + 2) % 2 == 0) {
				placerholderid = data.Placeholderid + (index + 1).toString();
				$element += '<tr><td width="90%"><br/>';
				$element += '<div id=' +'"'+ placerholderid +'"'+' style="width:90%;height:400px; padding:0px; position:relative;"></div></td></tr>';
			}
		}
		$element += '</tbody></table>';
		$("#counterstatsplace").append($element);
	}
}

/* Function changes selection class of the div which is selected */
function unSelectSTADiv(oldmac)
{
	var isFound = false;

	$(".stabox").each(function() {
		if ($(this).attr('data-mac') == oldmac) {
			isFound = true;
			$(this).removeClass('selectedstabox');
			$(this).addClass('stabox');
		}
	});
	if (isFound == false) {
		$(".selectedstabox").each(function() {
			if ($(this).attr('data-mac') == oldmac) {
				$(this).removeClass('selectedstabox');
				$(this).addClass('stabox');
			}
		});
	}
}

/* Selects associated STA in table */
function selectSTAinTable()
{
	/* now select row in table based on mac */
	$('.apstainfolisttable tr td:nth-child(2)').each(function() {
		if ($(this).text() == mac) {
			$(this).closest('tr').css("background-color","#6BAAFC");
			$(this).closest('tr').css("color","white");
		}
	});
}

/* Unselects associated STA in table */
function UnselectSTAinTable(oldmac)
{
	/* now select row in table based on mac */
	var i = 0;

	unSelectSTADiv(oldmac);
	$('.apstainfolisttable tr td:nth-child(2)').each(function() {
		if ($(this).text() == oldmac) {
			if ((i%2) == 0)
				$(this).closest('tr').css("background-color","#CDD6DB");
			else
				$(this).closest('tr').css("background-color","#E6EDF0");
			$(this).closest('tr').css("color","black");
		}
		i++;
	});
}

/* Draw the layout for showing STA statistics like packet queue etc.. */
function drawAdvancedPerSTALayout(indata)
{
	var element = [];
	$("#advancedperstadetdiv").empty();
	for (var index = 0; index < indata.Total; index++) {
		element += '<table id="'+getCommonIdFromName(indata.Data[index].Heading)+'" class="cmnstasstatsgrphtbl fontfamly cmngraphbox">';
		element += '<thead class="cmngraphboxbkgrnd-h1">';
		element += '<tr><th>'+indata.Data[index].Heading+'</th>';
		if (indata.Data[index].Heading == 'Packet Queue Statistics') {
			element += '<th></th></tr></thead><tbody><tr><td class="packetqueuesmallhead">From AP Perspective</td>';
			element += '<td class="packetqueuesmallhead">From STA Perspective</td></tr></tbody></table>';
		}
		else
			element += '</tr></thead><tbody></tbody></table>';
	}
	$("#advancedperstadetdiv").append(element);

	element = [];

	/* Now draw inner layout for graphs */
	for (var outerindex = 0; outerindex < indata.Total; outerindex++) {
		var data = indata.Data[outerindex].Data;
		var tmpid = getCommonIdFromName(indata.Data[outerindex].Heading);
		gSTADetTableIDs.push(tmpid);
		for (var tmpindex = 0; tmpindex < data.length; tmpindex++) {
			for (var index = 0; index < data[tmpindex].Total; index++) {
				if ((index + 2) % 2 == 0) {
					placerholderid = data[tmpindex].Placeholderid + (index + 1).toString();
					if (indata.Data[outerindex].Heading == 'Packet Queue Statistics') {
						element = '<td style="width:50%;"><table class="packetqueuegraphtable">';
						element += '<thead class="cmclr"><tr><th></th></tr></thead>';
						element += '<tbody><tr><td>';
						element += '<div id="'+ placerholderid +'"' +'" style="width:90%;height:400px;padding:0px;position:relative;">';
						element += '</div></td></tr></tbody></br></table></td>';
						/* Draw another graph for STA's on the side */
						if (tmpindex < indata.Data[outerindex].STAData.length) {
							staplaceholder = indata.Data[outerindex].STAData[tmpindex].Placeholderid + (index + 1).toString();
							element += '<td style="width:50%;"><table class="packetqueuegraphtable">';
							element += '<thead class="cmclr"><tr><th></th></tr></thead>';
							element += '<tbody><tr><td>';
							element += '<div id="'+ staplaceholder +'"' +'" style="width:90%;height:400px;padding:0px;position:relative;">';
							element += '</div></td></tr></tbody></br></table></td>';
							g_noerrmsg = 1;
						} else {
							if (!g_noerrmsg)
								element += '<td class="packetqueuesmallhead"> RRM Stats are not available. </td>';
							g_noerrmsg = 1;
						}
					} else {
						element = '<td style="width:100%;"><table class="packetqueuegraphtable">';
						element += '<thead class="cmclr"><tr><th></th></tr></thead>';
						element += '<tbody><tr><td>';
						element += '<div id="'+ placerholderid +'"' +'" style="width:90%;height:400px;padding:0px;position:relative;">';
						element += '</div></td></tr></tbody></br></table></td>';
					}
					var max = 1;
					var trow = $('#'+tmpid+' > tbody > tr:last');
					if (!trow.length || trow.children('td').length >= max) {
						$('#'+tmpid+' > tbody').append("<tr>");
					}
					$('#'+tmpid+' > tbody > tr:last').append(element);
					element = [];
				}
			}
		}
		data = [];
	}
}

/* Requests Stats Per Station from server */
function fetchAdvancedPerSTADet(arraycookies, islayoutreq, mac, selectedBand) {
	var data;
	var tmpid;
	var islayoutredrawreq;
	$.ajax({
		type:"GET",
		async:true,
		timeout:5000,
		url:urlpacketqueuestats,
		data:{Req:'AdvancedPerStationDetails', DutMac:selectedBand.mac, StaMac:mac, FreqBand:selectedBand.band},
		success:function(result) {
			if (isHTTPDWebserver == 1)
				data = result;
			else
				data = JSON.parse(result);

			islayoutredrawreq = (islayoutreq || (data.StaDataAvailability == "true" && g_noerrmsg == 1));
			/* draw the layout for first time as well as when sta data gets available */
			if (islayoutredrawreq) {
				drawAdvancedPerSTALayout(data.AllData);
			}

			if (data.StaDataAvailability == "true") {
				g_noerrmsg = 2; /* once layout is created no need to recreate it every time */
			}

			for (var index = 0; index < data.AllData.Total; index++) {
				var innerdata = data.AllData.Data[index];
				if (innerdata.Heading == 'Packet Queue Statistics') {
					/* For customized graph */
					packetQueueData = innerdata.Data;
					packetQueueSTAsData = innerdata.STAData;
					tmpid = getCommonIdFromName(innerdata.Heading);
					/* For packet queue stats, needs to draw two graphs side by side
					 * One for AP and one for STA
					 */
					for (var outerindex = 0; outerindex < innerdata.Data.length; outerindex++) {
						var tmpdata = innerdata.Data[outerindex];
						drawTimeSeriesStats(tmpdata, "#"+tmpdata.Placeholderid, true, true);
						if (outerindex < innerdata.STAData.length) {
							var tmpdatainner = innerdata.STAData[outerindex];
							drawTimeSeriesStats(tmpdatainner, "#"+tmpdatainner.Placeholderid, true, true);
						}
						tmpdata = [];
					}
				} else {
					for (var outerindex = 0; outerindex < innerdata.Data.length; outerindex++) {
						var tmpdata = innerdata.Data[outerindex];
						drawTimeSeriesStats(tmpdata, "#"+tmpdata.Placeholderid, true, true);
						tmpdata = [];
					}
				}
				innerdata = [];
			}
			createCustomizedGraphTable(arraycookies, islayoutredrawreq, tmpid, packetQueueData, packetQueueSTAsData);
			packetQueueData = null;
			packetQueueSTAsData = null;
		}
	});
	data = [];
}

function pullAdvancedPerSTADet(islayoutreq, selectedBand)
{
	var staval = $.inArray(mac, staarr);

	if (staval == -1) {
		hideStationsDetailTable();
		UnselectSTAinTable(mac);
		mac = 0;
		staclkarr = [];
	} else {
		var shared = unescape($.cookie('matricscntrpsta'));
		var arraycookies = [];

		if (shared.length > 0) {
			arraycookies = shared.split(',');
		}
		index = 0;
		fetchAdvancedPerSTADet(arraycookies, islayoutreq, mac, selectedBand);
		arraycookies = [];
		shared = [];
	}
}

/* On selecting associated STA from table or from image show the packet queue stats */
function onSelectingAssociatedSTA()
{
	if (staclkarr[staclkarr.length - 1] != mac) {
		staclkarr.push(mac);
		hideStationsDetailTable();
		showStationsDetailTable();
		// When the sta is clicked, only once the packet queue stats are plotted with the layout
		// from the following code. For the rest of the time the packet queue stats are plotted
		// from the same place where the other counters like glitch are plotted. This approach is
		// taken because xaxis values across the different graph plottings are same
		var selectedBand = getBandChangeData('#bandselectcntrl', gAllBandsArr);
		pullAdvancedPerSTADet(true, selectedBand);
	}
}

/* On selecting STA row form image or from table row */
function onSelectSTARow(newmac)
{
	var isFound = false;
	var oldmac = mac;

	/* If same mac is already selected return */
	if (mac == newmac)
		return;

	UnselectSTAinTable(oldmac);
	mac = newmac;
	/* To select the div in diagram, search through each div for stabox */
	$(".stabox").each(function() {
		if ($(this).attr('data-mac') == mac) {
			$(this).addClass('selectedstabox');
		}
	});
	selectSTAinTable();

	onSelectingAssociatedSTA();
}

$(function() {
	/* On clicking associated STA's table row */
	$(document).on("click",'#apstalisttable > tbody > tr',function() {
		onSelectSTARow($(this).find('td:eq(1)').text());
	});

	/* On clicking AP-STA relation diagram's right side */
	$(document).on("click",'.apstamaprightcol > tbody > tr',function() {
		onSelectSTARow($(this).find('td:eq(1)').text());
	});

	/* On clicking AP-STA relation diagram's left side */
	$(document).on("click",'.apstamapleftcol > tbody > tr',function() {
		onSelectSTARow($(this).find('td:eq(1)').text());
	});
});

/* Requests packet queue stats from server */
function fetchPacketQueueStats(arraycookies, index, islayoutreq, mac, selectedBand) {
	var data;

	$.ajax({
		type:"GET",
		async:true,
		timeout:5000,
		url:urlpacketqueuestats,
		data:{Req:'Packet Queue Statistics',DutMac:selectedBand.mac,StaMac:mac, FreqBand:selectedBand.band},
		success:function(result) {
			if (isHTTPDWebserver == 1)
				data = result;
			else
				data = JSON.parse(result);
			packetQueueData = data[0];
			for (var i = 0; i < data[0].length; i++) {
				drawPQSLayout(data[0][i],islayoutreq);
				drawTimeSeriesStats(data[0][i], "#"+data[0][i].Placeholderid, true, true);
			}
		}
	});
}

function populateCustomizedGraphTableElement(dataToPlot, tblid, graphplaceholder, checkboxid)
{
	var elementadd = '';
	var element = '';
	var increment = 0;
	var nCount = 0;
	var nCountInRow = 3;

	elementadd += '<table><tr>';
	for (var i = 0; i < dataToPlot.length; i++) {
		for (var j = 0; j < dataToPlot[i].Total; j++) {
			if ((nCount % nCountInRow) == 0)
				elementadd += '</tr><tr>';
			elementadd += '<td><input type="checkbox" id="' + checkboxid + increment++ + '" ';
			elementadd += 'style="width:25px;height:15px;margin-left:30px;" ';
			elementadd += 'value=' + '"'+ dataToPlot[i].BarHeading[j] + ';"';
			elementadd += '>' + dataToPlot[i].BarHeading[j] +'</td>';
			nCount++;
		}
	}
	elementadd += '</tr></table>';
	element = '<td style="width:50%;"><table class="packetqueuegraphtable">';
	element += '<caption align="center"><font class="fontonwhite"><h2><u><br><br>Customized Graph</u></h2>';
	element += '<br><h3><u>Select the counters to plot</u> </h3><br></font></caption>';
	element += '<thead class="cmclr"><tr><th></th></tr></thead>';
	element += '<tbody><tr><td>'+ elementadd + '</td></tr><tr><td>\
	<div id="'+ graphplaceholder +'" \
	style="width:90%;height:400px;padding:0px;position:relative;"></div></td></tr></tbody>\
	</table></td>';

	elementadd = [];

	return element;
}

/* Creates layout for customized garph of packet queue stats */
function createCustomizedGraphTable(arraycookies, islayoutreq, tblid, dataToPlot, STAdataToPlot) {
	if (islayoutreq == true) {
		var element = populateCustomizedGraphTableElement(dataToPlot, tblid, "customplaceholder", "checkbox");
		var elementSTA = populateCustomizedGraphTableElement(STAdataToPlot, tblid, "STAcustomplaceholder", "STAcheckbox");

		element += elementSTA;
		var max = 1;
		var trow = $('#'+tblid+' > tbody > tr:last');
		if (!trow.length || trow.children('td').length >= max) {
			$('#'+tblid+' > tbody').append('<tr>');
		}

		$('#'+tblid+' > tbody > tr:last').append(element);
		elementSTA = [];
		element = [];
	}
	fillCustomizedGraphWithData(arraycookies, dataToPlot, "customplaceholder", "checkbox");
	fillCustomizedGraphWithData(arraycookies, STAdataToPlot, "STAcustomplaceholder", "STAcheckbox");
}

/* Fills packet queue stats customized graph */
function fillCustomizedGraphWithData(arraycookies, dataToPlot, graphplaceholder, checkboxid) {
	// dataToPlot object consists of all the other objects of the packet queue statistics
	// create a one temporary object which contains the members of all the objects together in one object from the dataToPlot object
	// and feed that into the jquery plot function
	var packetQueueTemp = null;
	var packetQueueTemp1 = null;
	var tmpLength = 0;
	var concatYValue = [];
	var concatBarHeading = [];
	var concatTotal = '';
	// Deep copy
	var packetQueueTemp = jQuery.extend(true, {}, dataToPlot[0]);
	var packetQueueTemp1 = jQuery.extend(true, {}, dataToPlot[0]);

	packetQueueTemp1.YValue = [];
	packetQueueTemp1.BarHeading = [];
	packetQueueTemp1.Total = [];
	drawTimeSeriesStatsForCustom(arraycookies, packetQueueTemp1,"#"+graphplaceholder);

	for (var i = 0; i < dataToPlot.length; i++) {
		tmpLength += dataToPlot[i].Total;
		concatYValue = concatYValue.concat(dataToPlot[i].YValue);
		concatBarHeading = concatBarHeading.concat(dataToPlot[i].BarHeading);
		concatTotal += dataToPlot[i].Total;
	}
	packetQueueTemp.YValue = concatYValue;
	packetQueueTemp.BarHeading = concatBarHeading;
	packetQueueTemp.Total = concatTotal;
	// from packet queue array of objects create another temporary object which contains the members of all the objects which are selected in the UI
	var tot = 1;
	for (var i = 0; i < tmpLength; i++) {
		if ($("#" + checkboxid + i).is(":checked")) {
			packetQueueTemp1.YValue.push(packetQueueTemp.YValue[i]);
			packetQueueTemp1.Total = tot++;
			packetQueueTemp1.BarHeading.push(packetQueueTemp.BarHeading[i]);
		}
	}
	// plot the second temporary object which only contains the data of the items selected in the customized graph
	if (packetQueueTemp1.YValue != 0) {
		drawTimeSeriesStatsForCustom(arraycookies,packetQueueTemp1,"#"+graphplaceholder);
	}
	// set the object reference to null so that browser garbage collector can free the memory otherwise it will leak
	packetQueueTemp = null;
	packetQueueTemp1 = null;
	dataToPlot = null;
	// All the graphs are getting refreshed every 5 second and we will maintain the same limit for customized graph too
	// and not refresh every 500ms which was done earlier and hence commenting the code
	// needs to be put in timer so that checking/unchecking items must remove that plot from the customized graph
	//customgraphtimer.push(setTimeout(function(){fillCustomizedGraphWithData(arraycookies, dataToPlot);},500));
}

/* Draws layout for packet queue stats */
function drawPQSLayout(data, islayoutreq) {
	var placerholderid;

	if (islayoutreq == true) {
		for (var index = 0; index<data.Total; index++) {
			var tempData = [];
			if ((index + 2) % 2 == 0) {
				placerholderid = data.Placeholderid + (index + 1).toString();
				$element = '<td style="width:100%;"><table class="packetqueuegraphtable"><thead class="cmclr"><tr><th></th></tr></thead>'; //'+data.Heading+'
				$element = $element + '<tbody><tr><td><div id="'+ placerholderid +'"' +'" style="width:90%;height:400px;padding:0px;position:relative;"></div></td></tr></tbody></br></table></td>';
				var max = 1;
				var trow = $("#packetqueuestatsgrphtbl > tbody > tr:last");
				if (!trow.length || trow.children('td').length >= max) {
					$("#packetqueuestatsgrphtbl > tbody").append("<tr>");
				}
				$("#packetqueuestatsgrphtbl > tbody > tr:last").append($element);
			}
		}
	}
}

/* Draws time series graph */
function drawTimeSeriesStats(data, placeholder, iscolorlabel, isShowLegend) {
	var barHeadingCount = 0;
	for (var index = 0; index < data.Total; index++) {
		var dataval1 = [];
		var tempData = [];
		if ((index + 2) % 2 == 0) {
			for (var index1 = 0; index1 < data.XValue.length; index1++) {
				tempData.push([data.XValue[index1]*1000,data.YValue[index][index1]]); // need to do *1000
			}
			dataval1.push(tempData);
			if (data.YValue[index+1] != undefined) {
				var tempData = [];
				for (var index2 = 0; index2 < data.XValue.length; index2++) {
					tempData.push([data.XValue[index2]*1000,data.YValue[index+1][index2]]); // need to do *1000
				}
				dataval1.push(tempData);
			}
			var options = {
				series:{
					lines:{
						show:true,
						lineWidth:3
					},
					points:{
						show:true,
					}
				},
				colors:["#27801F","#5C48EF","#B80DAA","#9B211C","#10D1AA"],
				grid: {
					hoverable: true,
					clickable: true,
					backgroundColor: { colors: ["#CDD6DB", "#E6EDF0"] }
				},
				xaxis:{
					timeformat: "%M:%S",
					mode:"time",
					axisLabel:data.XAxis,
					show:true,
				},
				yaxes:(data.YValue[index+1] != undefined)?[{
					position: "left",
					axisLabel:data.YAxis+' ('+data.BarHeading[barHeadingCount]+')',
					min:0,
					show:true,
					panRange:false
				},{
					position: "right",
					axisLabel:data.YAxis+' ('+data.BarHeading[barHeadingCount+1]+')',
					min:0,
					show:true,
					panRange:false
				}]:[{
					position: "left",
					axisLabel:data.YAxis+' ('+data.BarHeading[barHeadingCount]+')',
					min:0,
					show:true,
					panRange:false
				},{
				}],
				 legend: {
					show : isShowLegend,
					labelBoxBorderColor: "none",
					position: "right"
				},
				pan:{
					interactive: true,
					cursor: "move",
					frameRate: 20
				}
			};
			var dataset = [];
			var count = 0;
			for (var index3 = 0; index3 < dataval1.length; index3++) {
				dataset.push({label:data.BarHeading[barHeadingCount],hoverable:true,data:dataval1[index3],yaxis: ++count});
				barHeadingCount++;
			}
			placerholderid = "#" + data.Placeholderid + (index + 1).toString();
			var pltrsdata = $.plot($(placerholderid),dataset, options);
			displayTheTooltipForGraphs(placerholderid);

			/* If the lable and series color should be same for some plots */
			if (iscolorlabel == true) {
				var series = pltrsdata.getData();
				/* for all series */
				for (var i = 0; i < series.length; i++) {
					var placeholder = '';
					if ((i % 2) == 0)
						placeholder = placerholderid + ' > .yaxisLabel'; /* First y axis */
					else
						placeholder = placerholderid + ' > .y2axisLabel'; /* Second y axis */
					/* Apply the series color to yaxis label */
					$(placeholder).css('color', series[i].color);
				}
			}
		}
	}
}

/* Draws time series graph for customized graphs */
function drawTimeSeriesStatsForCustom(array, data, placeholder) {
	var dataval1 = [];
	var tmpBarHeading = [];

	for (i = 0; i < data.Total; i++) {
		var tempData = [];
		if ($.inArray(data.BarHeading[i], array) != -1) {
			for (j = 0; j < data.XValue.length; j++) {
				tempData.push([data.XValue[j]*1000,data.YValue[i][j]]); // need to do *1000
			}
			dataval1.push(tempData);
			tmpBarHeading.push(data.BarHeading[i]);
		}
	}
	var options = {
		series:{
			lines:{
				show:true,
				lineWidth:3
			},
			points:{
				show:true,
			}
		},
		colors:["#27801F","#5C48EF","#B80DAA","#9B211C","#10D1AA"],
		grid: {
			hoverable: true,
			clickable: true,
			backgroundColor: { colors: ["#CDD6DB", "#E6EDF0"] }
		},
		xaxis:{
			timeformat: "%M:%S",
			mode:"time",
			axisLabel:data.XAxis,
			show:true,
		},
		yaxis:{
			axisLabel:data.YAxis,
			show:true,
			panRange:false
		},
		 legend: {
			labelBoxBorderColor: "none",
			position: "right"
		},
		pan:{
			interactive: true,
			cursor: "move", 
			frameRate: 20
		}
	};

	var dataset = [];
	for (i = 0; i < dataval1.length; i++) {
		dataset.push({label:tmpBarHeading[i],hoverable:true,data:dataval1[i]});
	}
	$.plot($(placeholder),dataset,options);
	displayTheTooltipForGraphs(placeholder);
	tmpBarHeading = [];
}

/* Function for showing the tool tip for associated STA diagram */
$(function() {
	var $tooltiptext;
	$(document).on({
		mouseenter:function(e) {
			$tooltiptext = $(this).attr('title');
			$(this).attr('title','');
			$('<p class="tooltip fontfamly border"></p>')
				.text($tooltiptext)
				.appendTo('body')
				.css('top', (e.pageY - 10) + 'px')
				.css('left', (e.pageX + 20) + 'px')
				.fadeIn('slow');
		},
		mouseleave:function(e) {
			$('.tooltip').remove();
			$(this).attr('title',$tooltiptext);
		}
	},'div.stabox');

	$(document).on({
		mouseenter:function(e) {
			$tooltiptext = $(this).attr('title');
			$(this).attr('title','');
			$('<p class="tooltip fontfamly border"></p>')
				.text($tooltiptext)
				.appendTo('body')
				.css('top', (e.pageY - 10) + 'px')
				.css('left', (e.pageX + 20) + 'px')
				.fadeIn('slow');
		},
		mouseleave:function(e) {
			$('.tooltip').remove();
			$(this).attr('title',$tooltiptext);
		}
	},'div.selectedstabox');
});

/* Displays the tool tip for graphs */
function displayTheTooltipForGraphs(placeholder) {
	$(placeholder).bind("plothover",function(event, pos, item) {
		$("#tooltipdiv").remove();
		if (item) {
			var tooltip = item.series.data[item.dataIndex][1];

			 $('<div class="tooltip border fontfamly" id="tooltipdiv">' + tooltip + '</div>')
				.css({
					top: (item.pageY - 30) + 'px',
					left: (item.pageX + 10) + 'px'})
				.appendTo("body").fadeIn(200);
		}
	});
}

function associatePanning(placeholder) {
	$(placeholder).bind("plotpan",function(event,plot){});
}

/* Event handler for check box created dynamically for customized chanim stats*/
$(document).on('click', '#checkchanimid', function() {
	var checkedArray = [];

	$.each($("#checkchanimid:checked"),function() {
		checkedArray.push($(this).val());
	});

	drawTimeSeriesStatsForCustom(checkedArray, gChanimData, "#"+gChanimData.Placeholderid);
	checkedArray = [];
});

$(window).bind('beforeunload',function() {
	$('#bandselectcntrl > option:eq(0)').prop('selected', true);
})