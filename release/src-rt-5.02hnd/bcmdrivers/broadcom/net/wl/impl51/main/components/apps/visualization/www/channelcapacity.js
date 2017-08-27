/*
 * Jquery implementation for Channel Statistics Tab
 *
 * $ Copyright Open Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: channelcapacity.js 616026 2016-01-29 11:14:36Z pankajj $
 */

var timers = [];
var allchnls = [36,40,44,48,52,56,60,64,'.',100,104,108,112,116,120,124,128,132,136,140,144,'.',149,153,157,161,165];
var reftwogchnl20 = [1,2,3,4,5,6,7,8,9,10,11];
var reftwogchnl40 = ['','',3,4,5,6,7,8,9,'',''];
var adjacentchnlsxaxislbls = [];
var associatedSSIDs = []; /* To store the all associated STA's SSID's. This is to show it in tooltip */
var associatedSTARSSIs = []; /* To store the all associated STA's RSSI's. This is to show it in tooltip */

var timerval = unescape($.cookie('timerinterval'));
var timerdata = [];
if (timerval.length > 0) {
	timerdata = timerval.split(',');
}
timerdata[0] = Number(timerdata[0]);

var hiddenid=[];
var remoteIPAddress = '';
var isRemoteEnabled = 0;
var urlremotesettings = '';
var urlchcapacity = '';
var urlapstainfo = '';
var isHTTPDWebserver = 1;

var g_associatedstaID = 'associatedsta';
var g_timerassocsta = [];
var g_timerstadtls = [];
var gAllBandsArr = []; /* For storing the bands and its SSID and BSSID's */

var TIMER_CHANNEL_CAPACITY	= 1 << 0;
var TIMER_ASSOCSTA			= 1 << 1;
var TIMER_STADTLS	=	1 << 2
var g_selectedstamac = "";

/* To initialize some variables on page load */
function initPage()
{
	if (isHTTPDWebserver == 1) {
		urlremotesettings = 'http://'+ window.location.host +'/json.cgi';
		urlchcapacity = 'http://'+ window.location.host +'/json.cgi';
		urlapstainfo = 'http://'+ window.location.host +'/json.cgi';
		urlselectedstainfodtls = 'http://'+ window.location.host +'/json.cgi';
	} else {
		urlremotesettings = 'http://localhost/remotesettings.php';
		urlchcapacity = 'http://localhost/channelcapacity.php';
		urlapstainfo = 'http://localhost/stationdetails.php';
		urlselectedstainfodtls = 'http://localhost/stainfo.php';
	}

	initCommonPage();
}

/* Return 1 if remote debugging is enabled */
function isRemoteDebuggingEnabled()
{
	if (isRemoteEnabled == 1 && remoteIPAddress != "127.0.0.1")
		return 1;
	return 0;
}

/* Clear all the timers */
function clearAllTimers(flag)
{
	if (flag & TIMER_CHANNEL_CAPACITY) {
		for (i = 0; i <timers.length; i++) {
			clearTimeout(timers[i]);
		}
		timers = [];
	}

	if (flag & TIMER_ASSOCSTA) {
		for (i = 0; i < g_timerassocsta.length; i++) {
			clearTimeout(g_timerassocsta[i]);
		}
		g_timerassocsta = [];
	}

	if (flag & TIMER_STADTLS) {
		for (i = 0; i < g_timerstadtls.length; i++) {
			clearTimeout(g_timerstadtls[i]);
		}
		g_timerstadtls = [];
	}
}

/*
 * Draw the channel distribution table. Draws control and extension channels
 * for all the surrounding AP and also for the AP on which visualization is running
 * Here everything is drawn using HTML tables.
 */
function drawchnldistribution(chnldata, freqband, indx)
{
	var outer = [];
	var content = [];
	var grcontent = '<div class="chmaplegend"><font color="#C46618">C - Control Channel</font></br><font color="#396B98">E - Extension Channel</font>';
	var height = 0;

	grcontent += '<table class="chnldisttb">';
	height = (chnldata[1][indx].XValue.length+3) * 35; // Calculate the height of the table

	for (idx = 0; idx < chnldata[1][indx].XValue.length; idx++) { // For each adjacent AP
		outer = chnldata[1][indx].XValue[idx];
		content += '<tr>'; // For each AP add one row. i.e. tr in HTML
		// Add one column and fill the SSID of the AP in that place
		if (outer[0].IsCurrent == 0) // If it is not the AP on which visualization is running, use same color and font
			content += '<td class="apnames">'+outer[0].ssid+'</td>';
		else // If it is not the AP on which visualization is running, use different color and font
			content += '<td class="apnames currentap">'+outer[0].ssid+'</td>';
		for (i = 0; i < allchnls.length; i++) { // For all the channels
			var found = 0;
			if (outer[0].ctrlch == allchnls[i]) { // For control channel add the column
				if (outer[0].IsCurrent == 0) // If it is not the AP on which visualization is running
					content += '<td class="apchnlbox ctrl">C</td>';
				else
					content += '<td class="apchnlbox currentctrl">C</td>';
				found = 1;
			} else { // For extension channels add the columns
				for (eidx = 0; eidx < outer[0].extch.length; eidx++) { // For all extension channels
					if (outer[0].extch[eidx] == allchnls[i]) {
						if (outer[0].IsCurrent == 0) // If it is not the AP on which visualization is running
							content += '<td class="apchnlbox ext">E</td>';
						else
							content += '<td class="apchnlbox currentext">E</td>';
						found = 1;
						break;
					}
				}
			}
			if (found == 0) // If there is no control and extension channel for that channel number add empty column
			{
				content += '<td class="apchnlbox empty"></td>';
			}
		}
		content += '</tr>'; //End of one AP
	}
	content += '<tr><td class="apnames"></td>'; // Add one empty row for the gap between table and the x axis labels
	for (i = 0; i < allchnls.length; i++) { // Add the xaxis labels. i.e channel numbers
		content += '<td class="chnls">'+allchnls[i]+'</td>';
	}
	content += '</tr>';
	grcontent += '<tr><td class="apnames"></td></tr>';

	grcontent += content;
	grcontent += '</table>';
	grcontent += '<div class="xaxislabel">Channels</div>';
	$('#chnldistributiondiv').empty();
	$('#chnldistributiondiv').css('height',height+'px');
	$('#chnldistributiondiv').append(grcontent);

	outer = [];
	content = [];
}

/* On Changing Band selection */
function onChannelCapacityBandSelection(selectedBand)
{
	clearAllTimers(TIMER_CHANNEL_CAPACITY | TIMER_ASSOCSTA | TIMER_STADTLS);
	if (selectedBand.band == 2) { /* If 2.4GHz Band */
		/* Hide all 80 MHz graphs */
		for (i = 0; i < hiddenid.length; i++)
			$(hiddenid[i]).css('display','none');
		/* Hide channel distribution graph */
		$("#chnldistributiontbl").css('display','none');
	} else {
		/* Show all 80 MHz graphs */
		for (i = 0; i < hiddenid.length; i++)
			$(hiddenid[i]).css('display','');
		/* Show channel distribution graph */
		$("#chnldistributiontbl").css('display','');
	}
	var arr = getConfigSettings();
	if (arr.length > 0) {
		DisplayStaApInfo(false, selectedBand);
		FetchDataForChCapacity(false, selectedBand);
	} else if ($("#errormsgid").length == 0) {
		$("#frequencybandselector").append(errorelem);
	}
	arr = [];
}

/* Get the remote debugging settings from the cookie */
function retrieveRemoteDebug()
{
	var tmpcokie1, tmpcokie2;
	tmpcokie1 = unescape($.cookie('remoteenabledflag'));
	tmpcokie2 = unescape($.cookie('remotedconip'));
	if(tmpcokie1.length > 0){
		isRemoteEnabled = Number(tmpcokie1.split(','));
	} else {
		isRemoteEnabled = 0;
	}
	if(tmpcokie2.length > 0){
		remoteIPAddress = tmpcokie2.split(',');
	} else {
		remoteIPAddress = '';
	}
	if (isRemoteDebuggingEnabled()) { // If remote debugging enabled, show message
		$("#frequencybandselector").hide();
		var errorelem = '<div id="errormsgid" class="errormsg">Remote debugging is enabled. Please go to <b>"'+remoteIPAddress+'"</b>.</div>';
		$("#showerrmessage").append(errorelem);
		return 1;
	}
	return 0;
}

/*****
* Function which will get called when DOM for the page gets loaded.START
*****/
$(document).ready(function() {
	clearAllTimers(TIMER_CHANNEL_CAPACITY | TIMER_ASSOCSTA | TIMER_STADTLS);
	initPage();
	if (retrieveRemoteDebug() == 1)
		return;

	var errorelem = '<div id="errormsgid" class="errormsg">Please enable the graph in configure page</div>';
	$("#bandselectcntrl").change(function() { // On changing the select control
		var selectedBand = getBandChangeData(this, gAllBandsArr);
		onChannelCapacityBandSelection(selectedBand);
	});
	hiddenid = [];
	var arr = getConfigSettings();
	if (arr.length > 0) {
		gAllBandsArr = getBands();
		parseBandsAndInsert('#bandselectcntrl', gAllBandsArr);

		/* If there are atleast one interface(2G or 5G select first one by default */
		if (gAllBandsArr.length > 0) {
			DisplayStaApInfo(true, gAllBandsArr[0]);
			FetchDataForChCapacity(true, gAllBandsArr[0]);
		}
	} else if ($("#errormsgid").length == 0) {
		$("#frequencybandselector").append(errorelem);
	}
	arr = [];
});

/* Check if the channel statistics graph show is selected in config from the cookie */
function getConfigSettings() {
	var shared = unescape($.cookie('channelcapacitygrph'));
	var array = [];
	if (shared.length > 0) {
		array = shared.split(',');
	}
	return array;
}

/* Draws the Associated STA diagram */
function drawAssociatedSTADiagram(islayoutreq, chnldata)
{
	var element = '';
	var textdescription = 'Shows stations associated with AP.';

	/* Create the outer layout */
	var tmpelement = createOuterLayoutTable("Associated Station's", g_associatedstaID, textdescription);
	$("#channelcapacitygraphcontent").append(tmpelement);

	element = '<tr><td width="90%" align="center">';
	element += '<table class="apstamappingtbl fontfamly">';
	element += '<tbody><tr>\
						<td width=30% align="right">\
							<table class="apstamapleftcol fontfamily">\
								<tbody></tbody>\
							</table>\
						</td>\
						<td class="sprator" align="left" width=5px>\
							<div class="staseparator"></div>\
						</td>\
						<td width=40%>\
							<table class="cmntable">\
								<tr>\
									<td align="center">\
										<div class="boxdiv">\
											<font class="fontonwhite">\
											<table class="centerap">\
												<tbody>\
													<tr>\
														<td align="right">SSID</td>\
														<td> : </td>\
													</tr>\
													<tr>\
														<td align="right">BSSID</td>\
														<td> : </td>\
													</tr>\
													<tr>\
														<td align="right">Channel</td>\
														<td> : </td>\
													</tr>\
												</tbody>\
											</table>\
											</font>\
											<img src="ap-image.jpg" width="100px"/>\
										</div>\
										<div class="verticalline"></div>\
										<div class="horizontalline"></div>\
									</td>\
								</tr>\
							</table>\
						</td>\
						<td class="sprator" align="left" width=5px>\
							<div class="staseparator"></div>\
						</td>\
						<td width=30% align="left">\
							<table class="apstamaprightcol fontfamily">\
								<tbody></tbody>\
							</table>\
						</td>\
					</tr>'
	element += '</tbody></table></td></tr>';
	$('#'+g_associatedstaID).append(element);
}

/* Gets the Associated STA information from server */
function DisplayStaApInfo(islayoutreq, selectedBand) {
	var array = [];
	var macarr = [];

	clearAllTimers(TIMER_ASSOCSTA);

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
			if (islayoutreq)
				drawAssociatedSTADiagram(islayoutreq, array);
			macarr = drawAP(array, g_selectedstamac); /* Pass the empty mac address as there is no selection here */
			macarr = [];
		}
	});
	array = [];
	if (selectedBand.mac == $("#bandselectcntrl").val())
		g_timerassocsta.push(setTimeout(function(){DisplayStaApInfo(false, selectedBand);},timerdata[0]));
}

/* Fetch the data for channel statistics page from web server */
function FetchDataForChCapacity(islayoutreq, selectedBand) {
	var chnldata;

	clearAllTimers(TIMER_CHANNEL_CAPACITY);
	$.ajax({
		type:'GET',
		url:urlchcapacity,
		data:{Req:'ChannelCapacity', DutMac:selectedBand.mac, FreqBand:selectedBand.band},
		success:function(result){
			if (isHTTPDWebserver == 1)
				chnldata = result;
			else
				chnldata = JSON.parse(result);
			drawchnlcapcityHeadings(chnldata);
			drawLayout(chnldata,islayoutreq, selectedBand);
			drawchnlcapacitygraphs(chnldata,selectedBand);
		}
	});
	chnldata = [];
	if (selectedBand.mac == $("#bandselectcntrl").val())
		timers.push(setTimeout(function(){FetchDataForChCapacity(false, selectedBand);},timerdata[0]));
}

/* Draw the heading contents like current channel, bandwidth and available capacity */
function drawchnlcapcityHeadings(chnldata) {
	$('#channelcapacityhdrid > tbody > tr').remove();

	$element = '<tr><td>' + 'Current Channel' + '</td><td>:<td><td> ' + chnldata[0].Channel +'</td><tr>';
	$('#channelcapacityhdrid').append($element);

	$element = '<tr><td>' + 'Current Channel BandWidth' + '</td><td>:<td><td> ' + chnldata[0].Bandwidth + ' MHz' + '</td><tr>';
	$('#channelcapacityhdrid').append($element);

	$element = '<tr><td>' + 'Current Available Capacity' + '</td><td>:<td><td> ' + chnldata[0].Capacity + '%' + '</td><tr>';
	$('#channelcapacityhdrid').append($element);
}

/* Creates the outer group box with text description for each different stats */
function createOuterLayoutTable(heading, tblid, description)
{
	var tmpelement;

	tmpelement = '<table id="'+tblid+'" class="mainheadingtable fontfamly cmngraphbox">';
	tmpelement += '<thead class="cmngraphboxbkgrnd-h1"><tr><th><div class="thtextheadingid">'+heading+'</div></th></tr></thead>';
	tmpelement += '<tbody><tr><td><div class="graphdescription">'+ description +'</br></br></div></td></tr></tbody></table></br></br>';

	return tmpelement;
}

/* Draw the complete page layout */
function drawLayout(chnldata, islayoutreq, selectedBand)
{
	if (islayoutreq == true) { // Only if layout is required
		var id = 11111;
		for (i = 0; i < chnldata[1].length; i++) {
			if (chnldata[1][i].Heading == 'ChannelMap') { // Only for channel map (channel distribution) graph
				var textdescription = 'For each AP, the graph shows the control channel and extension channels. Extension channel is any channel spanned by the AP that is not a control channel.';
				$element ='<table id ="chnldistributiontbl" class="mainheadingtable fontfamly cmngraphbox" style="margin-bottom:10px;"><thead class="cmngraphboxbkgrnd-h1"><tr>';
				$element = $element + '<th><div class="thtextheadingid">'+"Channel Distribution"+'</div></th></tr></thead><tbody><tr><td>';
				$element = $element + '<div class="graphdescription">'+ textdescription +'</div></td></tr><tr><td>';
				$element = $element + '<table><tr><td class="yaxislabel";>AP</td><td>';
				$element = $element + '<div id=' +'"chnldistributiondiv"'+' style="width:98%;height:380px; padding:0px; position:relative;"></div></td></tr></table></td>';
				$element = $element + '</tr></tbody></table>';
				$("#channelcapacitygraphcontent").append($element);
				if (selectedBand.band == 2) {
					$("#chnldistributiontbl").css('display','none');
				}
			} else {
				/* Create the graph layout for each bandwidth */
				$element ='<tr><td><table id ="'+ id + i+'"class="channelcapacitytable fontfamly cmngraphbox" style="margin-bottom:50px;">';
				$element = $element + '<thead class="cmngraphboxbkgrnd-h2"><tr><th><div class="thtextheadingid">'+ chnldata[1][i].Bandwidth+'MHz</div></th></tr></thead><tbody><tr><td>';
				$element = $element + '<div id=' +'"'+ chnldata[1][i].Placeholderid +'"'+' style="width:90%;height:240px; padding:0px; position:relative;"></div></td>';
				$element = $element + '</tr></tbody></table></td></tr>';
				/* Create a outer group box if the heading is 20 MHz and add the outer box to main division and
				 * add graph group box to outer box
				 */
				if (chnldata[1][i].Heading == 'Channel Capacity 20 MHz') {
					var textdescription = 'Shows bandwidth that is available for use in each channel.';
					var tmpelement = createOuterLayoutTable("Channel Capacity", "allchcapacityid", textdescription);
					$("#channelcapacitygraphcontent").append(tmpelement);
				} else if (chnldata[1][i].Heading == 'Interference 20 MHz') {
					var textdescription = 'Shows intereference level in each channel.';
					var tmpelement = createOuterLayoutTable("Interference", "allinterferenceid", textdescription);
					$("#channelcapacitygraphcontent").append(tmpelement);
				} else if (chnldata[1][i].Heading == 'Adjacent Channels 20 MHz') {
					var textdescription = 'Shows highest RSSI on each adjacent channel. Associated stations in channel on which the WiFi Insight is running.';
					var tmpelement = createOuterLayoutTable("Adjacent Channels", "alladjacentchnlid", textdescription);
					$("#channelcapacitygraphcontent").append(tmpelement);
				}

				/* Now add each bandwidths graph layout to corresponding outer box */
				if (chnldata[1][i].CHStatType == 'ChannelCapacity')
					$('#allchcapacityid').append($element);
				else if (chnldata[1][i].CHStatType == 'Interference')
					$('#allinterferenceid').append($element);
				else if (chnldata[1][i].CHStatType == 'AdjacentChannels')
					$('#alladjacentchnlid').append($element);

				/* Add 80 MHz layout elements to array to hide it later for 2.4 GHz */
				if (chnldata[1][i].Bandwidth == 80) {
					var tmphidden;
					tmphidden = "#" + id + i;
					hiddenid.push(tmphidden);
					/* If the selected band is 2.4 GHz then hide it */
					if (selectedBand.band == 2)
						$(tmphidden).css('display','none');
				}
			}
		}
	}
}

/*
 * Draw adjacent channel graphs which shows the adjacent AP's and the STA's connected on the
 * Channel in which visualization is running
 */
function drawadjacentchannelsgraphs(chnldata, freqband, indx) {
	var dataval1 = []; // To hold adjacent channel ap's to be drawn
	var dataval2 = []; // To hold STA's connected to the current Ap
	var dataset = [];
	var xaxislbl = [];
	var dataindex = [];
	var graphlabel = [];
	var placeholder;
	var k = 0;
	var xlabel = "Center Channels"
	var xstart = 0.3; // From which position to start the x axis labels
	var baralign = "center";
	var gapbtwnstas = 0.1;
	var staxstart = 0.4;
	var staindex = 0, curchnlindex = -1;
	var xmin = null;
	var xmax = null;

	/* Only for current bandwidth the SSID comes in the JSON array. SO for that only clear the old array to add the new SSIDs */
	if (chnldata[0].Bandwidth == chnldata[1][indx].Bandwidth) {
		associatedSSIDs = [];
		associatedSTARSSIs = [];
	}

	// In 2.4 GHz dont show 80 MHz channels
	if (freqband == 2 && (chnldata[1][indx].Bandwidth == 80))
		return;
	if (freqband == 2) { /* If 2.4 GHz band */
		// All the channels for xaxis label
		adjacentchnlsxaxislbls = chnldata[1][indx].Bandwidth == 40?reftwogchnl40:reftwogchnl20;
		xstart = chnldata[1][indx].Bandwidth == 40?1:1; // From where to start drawing the bar
		staxstart = -0.2; // From where to start drawing STA bar's
		xmin = 0;
		xmax = 12;
	} else {
		if (chnldata[1][indx].Bandwidth == 20) { // Get the xaxis label for only 20 MHz, 40 and 80 MHz channels remains the same
			adjacentchnlsxaxislbls = [];
			adjacentchnlsxaxislbls = chnldata[0].AllChnls;
		}
		xmin = -1;
		xstart = chnldata[1][indx].Bandwidth == 80?0.3:chnldata[1][indx].Bandwidth == 40?0.3:1; // From where to start drawing the bar
		baralign = chnldata[1][indx].Bandwidth == 20?"center":"left";
		xmax = adjacentchnlsxaxislbls.length+1;
	}

	// Copy xaxis labels
	for (i = 0; i <adjacentchnlsxaxislbls.length; i++) {
		xaxislbl.push([k+1,adjacentchnlsxaxislbls[i]]);
		dataindex.push(k);
		k++;
	}

	// Populate adjacent channel bars
	for (i = 0; i < 1; i++) {
		var tempData = [];
		for (j = 0; j < dataindex.length; j++) {
			var index;
			index = $.inArray(chnldata[1][indx].XValue[j],adjacentchnlsxaxislbls);
			tempData.push([index+xstart,chnldata[1][indx].YValue[i][dataindex[j]]+100]);
		}
		dataval1.push(tempData);
		tempData = [];
	}

	placeholder = "#"+chnldata[1][indx].Placeholderid;
	//Find the index of the STA from xaxis labels
	if (chnldata[1][indx].AXValue.length > 0)
		staindex = freqband==2?$.inArray(chnldata[1][indx].AXValue[0],reftwogchnl20):$.inArray(chnldata[1][indx].AXValue[0],adjacentchnlsxaxislbls);
	staindex++;

	var barwidth = 0.4; // Bar width of adjacent AP
	var stabarwidth = 0.1 // Bar width of the STA's bar
	var maxstastoshow = 4; // Maximum number of STA's to be shown
	if (freqband == 2) {
		barwidth = chnldata[0].Bandwidth == 20?0.8:0.8;
		maxstastoshow = chnldata[0].Bandwidth == 20?4:4;
		gapbtwnstas = 0.1;
	} else {
		barwidth = chnldata[1][indx].Bandwidth == 80?7.3:chnldata[1][indx].Bandwidth == 40?3.3:1.2;
		stabarwidth = chnldata[1][indx].Bandwidth == 80?0.6:chnldata[1][indx].Bandwidth == 40?0.6:0.6;
		maxstastoshow = chnldata[0].Bandwidth == 80?8:chnldata[0].Bandwidth == 40?4:2;
		gapbtwnstas = chnldata[0].Bandwidth == 80?0.3:chnldata[0].Bandwidth == 40?0.2:0.1;
		xlabel = "Channels";
	}
	maxstastoshow = chnldata[1][indx].AYValue[0].length>maxstastoshow?maxstastoshow:chnldata[1][indx].AYValue[0].length;
	/* Add All SSID's and RSSI to global array to show it in tooltip */
	for (j = 0; j < chnldata[1][indx].SSIDs.length; j++) {
		associatedSSIDs.push(chnldata[1][indx].SSIDs[j]);
		associatedSTARSSIs.push(chnldata[1][indx].AYValue[0][j]);
	}

	// Populate STA's connected to the AP
	for (i = 0; i < 1; i++) {
		var tempData = [];
		var d = staindex - stabarwidth+staxstart; // Find the index from where STA drawing should start
		var gap = barwidth - (stabarwidth * maxstastoshow); // Gap between each STA
		gap = gap /(maxstastoshow+1);
		for (j = 0; j < maxstastoshow; j++) {
			tempData.push([d,chnldata[1][indx].AYValue[i][j]+100]);
			if (chnldata[1][indx].SSIDs.length > 0)
				graphlabel.push(chnldata[1][indx].SSIDs[j]);
			d += stabarwidth+gapbtwnstas;
		}
		dataval2.push(tempData);
		tempData = [];
	}

	var options = {
		series:{
			bars:{
				show:true,
				align:baralign,
				barWidth: barwidth,
				horizontal:false,
				fill:1,
			}
		},
		colors:["#009999","#3B6E9C","#3EA818","#9B211C","#10D1AA"],
		grid: {
			hoverable: true,
			clickable: true,
			backgroundColor: { colors: ["#CDD6DB", "#E6EDF0"] },
			markings: function (axes) {
				var markings = [];
				for (var x = Math.floor(axes.yaxis.min); x < axes.yaxis.max; x += 20)
					markings.push({ yaxis: { from: x, to: x+0.5 }, color: "white" });
				return markings;
			}
		},
		legend:{
			 labelBoxBorderColor: "none",
			 position: "right"
		},
		xaxis:{
			axisLabel: xlabel,
			show:true,
			tickLength:1,
			ticks:xaxislbl,
			min:xmin,
			max:xmax,
			axisLabelFontSizePixels: 22,
			axisLabelUseCanvas: true,
			axisLabelFontFamily: 'Arial',
			axisLabelPadding: 20,
		},
		yaxis:{
			axisLabel:"Signal Strength [dBm]",
			min:0,
			max:100,
			show:true,
			tickLength:1,
			axisLabelFontSizePixels: 18,
			axisLabelUseCanvas: true,
			axisLabelFontFamily: 'Arial',
			axisLabelPadding: 15,
			tickFormatter: function (v, axis) {
					return v-100;
				},
			}
		};

	for (i = 0; i < dataval1.length; i++) {
		dataset.push({label:chnldata[1][indx].BarHeading[i],hoverable:true,data:dataval1[i]});
	}

	dataset.push({label:chnldata[1][indx].BarHeading[1], hoverable:true, data:dataval2[0], bars:{barWidth:stabarwidth, align:"center"}});

	var temp = $.plot($(placeholder),dataset,options);
	attachToolTipForAdjacentChannelGraph(placeholder);

	for (i = 0; i < graphlabel.length; i++) {
		$.each(temp.getData()[1].data,function(i,el) {
			var ssid = graphlabel[i];
			var o = temp.pointOffset({x:el[0],y:el[1]});
			if (el[1] != '-100') {
				$('<div class="data-point-label ssidbartop" id = "'+ ssid +'">'+ssid+'</div>').css({
					position:'absolute',
					left: o.left-4,
					top: o.top+el[1],
				}).appendTo(temp.getPlaceholder());
			}
		});
	}
	dataval1 = [];
	dataval2 = [];
	dataset = [];
	xaxislbl = [];
	dataindex = [];
	graphlabel = [];
}

/* Draw the channel capacity and intereference graphs */
function drawchnlcapacitygraphs(chnldata, selectedBand) {
	// For all the graphs to be plotted in channel statistics page
	for (indx = 0; indx < chnldata[1].length; indx++) {
		if (chnldata[1][indx].Heading == 'ChannelMap') { // If it is channel Map
			if (selectedBand.band == 2) // No channel map graph in 2.4 GHz band
				continue;
			drawchnldistribution(chnldata, selectedBand.band, indx);
			continue;
		} else if (chnldata[1][indx].CHStatType == 'AdjacentChannels') { // If it is adjacent channel graphs
			drawadjacentchannelsgraphs(chnldata, selectedBand.band, indx);
			continue;
		}
		var dataval1 = []; // To hold the bar's to be drawn
		var dataset = [];
		var xaxislbl = []; // To hold xaxis labels
		var dataindex = [];
		var placeholder;
		var k = 0;
		var xlabel = "Center Channels"
		var barwidth = 0.2;
		var secondbarcolor = '#A0A0A0';
		var firstbarcolor = '#417AB0';
		var xstart = 0.3; // From which position to start the x axis labels
		var xmin = null;
		var xmax = null;
		var baralign = "center";

		if (selectedBand.band == 2 && (chnldata[1][indx].Bandwidth == 80)) // In 2.4 GHz dont show 80 MHz graph
			continue;
		if (selectedBand.band == 2) { /* If 2.4 GHz band */
			adjacentchnlsxaxislbls = chnldata[1][indx].Bandwidth == 40?reftwogchnl40:reftwogchnl20; // All the channels for xaxis label
			if (chnldata[1][indx].Total > 1) { //If there are more than 1 bars the start point will differ
				xstart = chnldata[1][indx].Bandwidth == 40?0.85:0.85;
				barwidth = 0.4;
			} else {//Else it will start from the xaxis point itself
				xstart = chnldata[1][indx].Bandwidth == 40?1:1;
				barwidth = 0.8;
			}
			xmin = 0;
			xmax = 12;
		} else {
			if (chnldata[1][indx].Bandwidth == 20) { // Get the xaxis label for only 20 MHz, 40 and 80 MHz channels remains the same
				adjacentchnlsxaxislbls = [];
				adjacentchnlsxaxislbls = chnldata[0].AllChnls;
			}
			if (chnldata[1][indx].Total > 1) { //If there are more than 1 bars the start point will differ
				xstart = chnldata[1][indx].Bandwidth == 80?0.3:chnldata[1][indx].Bandwidth == 40?0.3:0.6;
				barwidth = chnldata[1][indx].Bandwidth == 80?3.7:chnldata[1][indx].Bandwidth == 40?1.8:0.8;
			} else { // Else it will start from the xaxis point itself for 20 as the bar is aligned center else different as bars are aligned left
				xstart = chnldata[1][indx].Bandwidth == 80?0.3:chnldata[1][indx].Bandwidth == 40?0.3:1;
				barwidth = chnldata[1][indx].Bandwidth == 80?7.3:chnldata[1][indx].Bandwidth == 40?3.3:1.2;
			}
			baralign = chnldata[1][indx].Bandwidth == 20?"center":"left"; // center only for 20. left aligned for others
			xmin = -1;
			xmax = adjacentchnlsxaxislbls.length+1;
		}
		// Populate the xaxis labels
		for (i = 0; i < adjacentchnlsxaxislbls.length; i++) {
			xaxislbl.push([k+1,adjacentchnlsxaxislbls[i]]);
			dataindex.push(k);
			k++;
		}

		// Populate bar colors
		if (chnldata[1][indx].Total > 1) {
			firstbarcolor = '#417AB0';
		} else {
			firstbarcolor = chnldata[1][indx].CHStatType == 'ChannelCapacity'?'##3EA818':'#95B8D7';
		}

		// Populate the bar's
		// If it is channel capacity graphs there will be maximum of 3 graphs
		if (chnldata[1][indx].CHStatType == 'ChannelCapacity') {
			for (i = 0; i <chnldata[1][indx].Total; i++) { // For all the different Y values for channel capacit(busy, idle and channel capacity)
				var tempData = [];
				for (j = 0; j < dataindex.length; j++) { // For each x labels
					var index;
					index = $.inArray(chnldata[1][indx].XValue[j],adjacentchnlsxaxislbls);
					if (index < 0)
						continue;
					if (i != 2) // If the garph is not channel capacity it will be stacked in the same x value
						tempData.push([ index+xstart ,chnldata[1][indx].YValue[i][dataindex[j]]]);
					else // Else the bar will be next to each other
						tempData.push([ index+xstart + barwidth ,chnldata[1][indx].YValue[i][dataindex[j]]]);
				}
				dataval1.push(tempData);
				tempData = [];
			}
		} else { // If it is interference graphs there will be maximum of 2 graphs
			for (i = 0; i < chnldata[1][indx].Total; i++) { // For all the different Y values for intereference (wifi and non-wifi)
					var tempData = [];
					for (j = 0; j < dataindex.length; j++) { // For each x labels
						var index;
						index = $.inArray(chnldata[1][indx].XValue[j],adjacentchnlsxaxislbls);
						if (index < 0)
							continue;
						if (i != 1) // First bar will start from the point
							tempData.push([index+xstart,chnldata[1][indx].YValue[i][dataindex[j]]]);
						else // Next bar will be next to the first one
							tempData.push([ index+xstart + barwidth ,chnldata[1][indx].YValue[i][dataindex[j]]]);
					}
					dataval1.push(tempData);
					tempData = [];
				}
		}
		placeholder = "#"+chnldata[1][indx].Placeholderid;

		if (chnldata[1][indx].CHStatType == 'ChannelCapacity') { // second bar color for channel capacity
			secondbarcolor = '#A0A0A0';
		} else { // second bar color for intereference
			secondbarcolor = '#95B8D7';
		}

		var options = {
			series:{
				bars:{
					show:true,
					align:baralign,
					barWidth: barwidth,
					horizontal:false,
					fill:1
				}
			},
			colors:[firstbarcolor,secondbarcolor,"#3EA818","#9B211C","#10D1AA"],
			//colors:["#D3D3D3","#95B8D7","#8CCD45"],
			grid: {
				hoverable: true,
				clickable: true,
				backgroundColor: { colors: ["#CDD6DB", "#E6EDF0"] },
				markings: function (axes) {
					var markings = [];
					for (var x = Math.floor(axes.yaxis.min); x < axes.yaxis.max; x += 20)
						markings.push({ yaxis: { from: x, to: x+0.5 }, color: "white" });
					return markings;
				}
			},
			legend:{
				 labelBoxBorderColor: "none",
				 position: "right"
			},
			xaxis:{
				axisLabel:xlabel,
				show:true,
				tickLength:1,
				ticks:xaxislbl,
				min:xmin,
				max:xmax,
				axisLabelFontSizePixels: 22,
				axisLabelUseCanvas: true,
				axisLabelFontFamily: 'Arial',
				axisLabelPadding: 20,
			},
			yaxis:{
				axisLabel:"Percentage [%]",
				show:true,
				tickLength:1,
				max:100,
				min:0,
				axisLabelFontSizePixels: 22,
				axisLabelUseCanvas: true,
				axisLabelFontFamily: 'Arial',
				axisLabelPadding: 15,
				}
			};

		for (i = 0; i < dataval1.length; i++) {
			if (chnldata[1][indx].CHStatType == 'ChannelCapacity') { // Channel capcity graphs are stacked
				dataset.push({label:chnldata[1][indx].BarHeading[i], hoverable:true, stack:true, data:dataval1[i]});
			} else { // Intereference graph is adjacent bars
				dataset.push({label:chnldata[1][indx].BarHeading[i], hoverable:true, data:dataval1[i]});
			}
		}

		var temp = $.plot($(placeholder),dataset,options);
		attachToolTip(placeholder);
		dataval1 = [];
		dataset = [];
		xaxislbl = [];
		dataindex = [];
	}
}

/* Function to show the tooltip for channel capacity and interference graphs */
function attachToolTip(placeholder) {
	$(placeholder).bind("plothover", function(event, pos, item) {
		$("#tooltipdiv").remove();
		if (item) {
			var tooltip;
			tooltip =  item.series.label + ': ' + item.series.data[item.dataIndex][1] + '%';
			 $('<div class="tooltip border fontfamly" id="tooltipdiv">' + tooltip + '</div>')
                .css({
                    top: (item.pageY - 30) + 'px',
                    left: (item.pageX + 10) + 'px'})
                .appendTo("body").fadeIn(200);
		}
	});
}

/*
 * Function to show the tooltip for adjacent channel graphs
 * For associated STA we need to list all the SSID's and RSSI and for adjacent AP's only RSSI for that AP
 */
function attachToolTipForAdjacentChannelGraph(placeholder) {
	$(placeholder).bind("plothover",function(event,pos,item) {
		$("#tooltipdiv").remove();
		if (item) {
			var tooltip;

			/* If associated STA's show all associated STA's SSID's and RSSI values as a list */
			if (item.series.label == 'Associated STA') {
				var i;
				tooltip = '<b>All ' + item.series.label + 's</b></br>';
				tooltip += '<table style="padding:5px"><tr><th>SSID</th><th>RSSI [dbM]</th></tr>';
				tooltip += '<tbody>';
				for (i = 0; i < associatedSSIDs.length; i++) { /* For every associated STA add the SSID and RSSI to tooltip */
					tooltip += '<tr><td>' + associatedSSIDs[i] + '</td><td>' + associatedSTARSSIs[i] + '</td></tr>';
				}
				tooltip += '</tbody></table>';
			} else { /* If adjacent AP, show only that AP's RSSI */
				var actualval = item.series.data[item.dataIndex][1] - 100;
				tooltip =  item.series.label + 's Signal Strength : ' + actualval + ' dbM';
			}
			 $('<div class="adjacentchnltooltip border fontfamly" id="tooltipdiv">' + tooltip + '</div>')
                .css({
                    top: (item.pageY - 30) + 'px',
                    left: (item.pageX + 10) + 'px'})
                .appendTo("body").fadeIn(200);
		}
	});
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

$(function() {
	/* On clicking AP-STA relation diagram's right side */
	$(document).on("click",'.apstamaprightcol > tbody > tr',function() {
		onSelectSTARow($(this).find('td:eq(1)').text());
	});

	/* On clicking AP-STA relation diagram's left side */
	$(document).on("click",'.apstamapleftcol > tbody > tr',function() {
		onSelectSTARow($(this).find('td:eq(1)').text());
	});
});

/* On selecting STA row form image or from table row */
function onSelectSTARow(mac)
{
	g_selectedstamac = mac;
	/* To select the div in diagram, search through each div for stabox */
	$(".stabox").each(function() {
		if ($(this).attr('data-mac') == g_selectedstamac) {
			$(this).addClass('selectedstabox');
			var $dialog = $("#stainfopopup").dialog({
				modal:true,
				resizable:false,
				/* override the dialog close button with 'X' */
				create:function(event, ui) {
					var wdgt = $(this).dialog("widget");
					$(".ui-dialog-titlebar button", wdgt).removeClass("ui-button-icon-only");
					$(".ui-dialog-titlebar", wdgt).addClass("cmngraphboxbkgrnd-h1");
					$(".ui-button-icon-primary", wdgt).remove();
					$(".ui-button-text", wdgt).text("X");
					$(".ui-button-text", wdgt).addClass("dlg-close-button");
					$("#stainfodtlshdr", wdgt).addClass("hdrbdr");
				},
				close:function(event, ui) {
					g_selectedstamac = "";
					clearAllTimers(TIMER_STADTLS);
				}
			});
			pullSelectedStaInfodtls(g_selectedstamac);
		}
	});
}

/* pull selected sta info from server */
function pullSelectedStaInfodtls(mac)
{
	var selectedBand = getBandChangeData('#bandselectcntrl', gAllBandsArr);
	var array = [];
	clearAllTimers(TIMER_STADTLS)
	$.ajax({
		type:"GET",
		url:urlselectedstainfodtls,
		data:{Req:'RRMSTAStats', DutMac:selectedBand.mac, StaMac:mac, FreqBand:selectedBand.band},
		async:true,
		timeout:5000,
		success:function(result) {
			if (isHTTPDWebserver == 1)
				array = result;
			else
				array = JSON.parse(result);
			drawstainfodtlsdlg(array);
		}
	});
	array = [];
	g_timerstadtls.push(setTimeout(function(){pullSelectedStaInfodtls(mac);},timerdata[0]));
}

var generatestainfotableelem = function(isouipresent, url)
{
	var $element = "", idx, jdx;
	for (idx = 2, jdx = idx + 1; idx < arguments.length - 1; ){
			if (isouipresent && arguments[idx] == "OUI : ") {
				$element += '<li>' +  arguments[idx] + '<a href= ' + url +'target="_blank" class="oui_ref">' + arguments[jdx] + '</a></li>';
			} else {
				$element += '<li>' +  arguments[idx] + arguments[jdx] + '</li>';
			}
			idx += 2;
			jdx += 2;
	}
	return $element;
}
/* draw sta info in dialog box */
function drawstainfodtlsdlg(array)
{
	$(".stainfodata").remove();
	var url = '"http://www.coffer.com/mac_find/?string=' + array.AllData.OUI + '"';
	var $element = '<div class="stainfodata fontfamly cmtxtclr"> <ul id="stadtldata" class="stadtldata">';
	var mimocapabilities = "", status = "";

	/* MIMO Capabilities */
	if (array.AllData.TXStream > 0) {
		mimocapabilities = array.AllData.TXStream + 'X' + array.AllData.RXStream;
	} else {
		mimocapabilities = "Not Available";
	}

	/*Status (Active or Idle)*/
	if (array.AllData.Active < 10) {
		status = "Active" + "( Idle for " + array.AllData.Active + " seconds)";
	} else {
		status = "Idle" + "(Idle for " + array.AllData.Active + " seconds)";
	}
	$element += '<br/>';
	$element += generatestainfotableelem( false, "", "MAC : ", array.AllData.MAC);
	$element += '<br/><br/>';
	$element += generatestainfotableelem( true, url, "OUI : ", array.AllData.OUI);
	$element += '<br/><br/>';
	$element += generatestainfotableelem( false, "", "Status : ", status);
	$element += '<br/><br/>';
	$element += generatestainfotableelem( false, "", "MIMO Capabilities : ", mimocapabilities);
	$element += '<br/><br/>';
	$element += generatestainfotableelem( false, "", "Generation : ", array.AllData.Generation);
	$element += '</ul></div>';
	$('#stainfopopdtls').append($element);
}

$(window).bind('beforeunload',function() {
	$('#bandselectcntrl > option:eq(0)').prop('selected', true);
})
