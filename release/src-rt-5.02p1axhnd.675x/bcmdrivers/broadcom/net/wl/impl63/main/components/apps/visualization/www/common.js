/*
 * Jquery implementation for Some of the common functions used in more than one places
 *
 * $ Copyright Open Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: common.js 682079 2017-01-31 10:30:10Z spalanga $
 */

var urlremotesettings = '';
var urlconfigsettings = '';
var gURLBands = ''
var isHTTPDWebserver = 1;

var g_UrlCommonGetNVRAM = '' /* Webserver URL to get NVRAM variable */

function initCommonPage()
{
	if (isHTTPDWebserver == 1) {
		urlremotesettings = window.location.protocol + '//' + window.location.host +'/json.cgi';
		urlconfigsettings = window.location.protocol + '//' + window.location.host +'/json.cgi';
		gURLBands = window.location.protocol + '//' + window.location.host +'/json.cgi';
		g_UrlCommonGetNVRAM = window.location.protocol + '//' + window.location.host +'/json.cgi';
	} else {
		urlremotesettings = 'http://localhost/remotesettings.php';
		urlconfigsettings = 'http://localhost/counterdata.php';
		gURLBands = 'http://localhost/getallbands.php';
		g_UrlCommonGetNVRAM = 'http://localhost/getnvram.php';
	}
}

function commonTopNavBar()
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
					<li>\
						<a href="configure.asp">Configure</a>\
					</li>\
				</ul>\
			</div>\
		</div>'
	);
}

function retrieveRemoteDebugConfig()
{
	var array = [];
	$.ajax({
		type:'GET',
		url:urlremotesettings,
		data:{Req:'RemoteSettings'},
		async:false,
		success:function(result){
			if (isHTTPDWebserver == 1)
				array = result;
			else
				array = JSON.parse(result);
			var remoteenabledflag = [];
			var remotedconip = [];
			remoteenabledflag.push(array.IsEnabled);
			remotedconip.push(array.ServerIP);
			$.cookie('remoteenabledflag',escape(remoteenabledflag.join(',')));
			$.cookie('remotedconip',escape(remotedconip.join(',')));
		}
	});
}

/* Function gets the NVRAM value from the Web server */
function getNVRAMValue(nvramName)
{
	var array = [];
	var tmpValue = '0';

	$.ajax({
		type:'GET',
		url:g_UrlCommonGetNVRAM,
		data:{Req:'VisGetNVRAM',NVRAMName:nvramName},
		async:false,
		success:function(result){
			if (isHTTPDWebserver == 1)
				array = result;
			else
				array = JSON.parse(result);
			tmpValue = array.NVRAMValue;
		}
	});

	return tmpValue;
}

/* Gets all the Bands(DUT's) from server */
function getBands()
{
	var array = [];

	$.ajax({
		type:'GET',
		url:gURLBands,
		data:{Req:'GetBands'},
		async:false,
		success:function(result){
			if (isHTTPDWebserver == 1)
				array = result;
			else
				array = JSON.parse(result);
		}
	});

	return array;
}

/* Parses the All bands(DUTS) info and insert it into band selection control */
function parseBandsAndInsert(ctrlId, allBands)
{
	var html = '';

	if (allBands.length > 0) {
		/* Show the bands as 'Band - SSID'. The Mac address will be the value which is unique */
		for (var i = 0; i < allBands.length; i++) {
			html += '<option value="' + allBands[i].mac + '">';
			if (allBands[i].band == '2')
				html += allBands[i].band + '.4 GHz - ' + allBands[i].ssid +'</option>';
			else
				html += allBands[i].band + ' GHz - ' + allBands[i].ssid +'</option>';
		}

		/* Remove the existing items */
		$(ctrlId).html('');
		/* Append the new items */
		$(ctrlId).append(html);
	} else {
		html = '<option>No Interfaces</option>'
		$(ctrlId).html(html);
	}
}

/* Gets the band changed data. Gets the current selected band */
function getBandChangeData(ctrlId, allBands)
{
	var val = $(ctrlId).val();
	var tmpData = { ssid : '', bssid : '', band : '', mac : ''};

	for (var i = 0; i < allBands.length; i++) {
		/* check in the array for the selected mac address(Value property of control */
		if (allBands[i].mac == val) {
			return allBands[i];
		}
	}

	return tmpData;
}

function InitConfig(){
	initPage();
	retrieveRemoteDebugConfig();
	var array = [];
	$.ajax({
		type:'GET',
		url:urlconfigsettings,
		data:{Req:'ConfigSettings'},
		async:false,
		success:function(result){
			if (isHTTPDWebserver == 1)
				array = result;
			else
				array = JSON.parse(result);
			fillTheCookies(array);
		}
	});
}

function fillTheCookies(array){
	var matricstabcounter = [];
	var matricstabperstacounters = [];
	var channelcounters = [];
	var channelcapacitygraphs = [];
	var intervalarr = [];
	var isstartflag = [];
	intervalarr.push(array[0][0].Interval*1000);
	isstartflag.push(array[0][0].StartStop);
	
	for(i=0;i<array[1].length;i++){
		if(array[1][i].Tab == 'Metrics' && array[1][i].PerSTA == 0 && array[1][i].Enable == 1){
		matricstabcounter.push(array[1][i].Name);
		}
		if(array[1][i].Tab == 'Metrics' && array[1][i].PerSTA == 1 && array[1][i].Enable == 1){
		matricstabperstacounters.push(array[1][i].Name);
		}
		if(array[1][i].Tab == 'Channels' && array[1][i].Enable == 1){
		channelcounters.push(array[1][i].Name);
		}
		if(array[1][i].Tab == 'Channel Statistics' && array[1][i].Enable == 1){
		channelcapacitygraphs.push(array[1][i].Name);
		}
	}
	$.cookie('timerinterval',escape(intervalarr.join(',')));
	$.cookie('matricscntr',escape(matricstabcounter.join(',')));
	$.cookie('matricscntrpsta',escape(matricstabperstacounters.join(',')));
	$.cookie('channelscntr',escape(channelcounters.join(',')));
	$.cookie('channelcapacitygrph',escape(channelcapacitygraphs.join(',')));
	$.cookie('isstarted',escape(isstartflag.join(',')));
}

function getCommonIdFromName(name)
{
	var tmpvar = name.replace(/\s/g, ''); /* Remove all the space */
	tmpvar = 'id' + tmpvar; /* Add string 'id' in the begining */

	return tmpvar;
}

/* Plugin for progress control */
(function($) {

	var progressBarTimer = ''; /* Timer for progress bar */
	var progressInterval = 300; /* Interval between each progress */

	/* Function to show the progress bar. This creates the progress bar and applies the css
	 * Also creates the timer
	 */
	$.fn.showProgressBar = function( options ) {

		var elements = '';

		/* Establish our default settings */
		var settings = $.extend({
			progressInterval:300
		}, options);

		/* css for each block in the progress control */
		var barCSS = {
			"background-color": "#2187e7",
			"background-image": "-moz-linear-gradient(45deg, #2187e7 25%, #a0eaff",
			"background-image": "-webkit-linear-gradient(45deg, #2187e7 25%, #a0eaff",
			"background-image": "-o-linear-gradient(45deg, #2187e7 25%, #a0eaff",
			"background-image": "-ms-linear-gradient(45deg, #2187e7 25%, #a0eaff",
			"background-image": "linear-gradient(45deg, #2187e7 25%, #a0eaff",
			"border-left": "1px solid #111",
			"border-top": "1px solid #111",
			"border-right": "1px solid #333",
			"border-bottom": "1px solid #333",
			"width": "10px",
			"height": "10px",
			"float": "left",
			"margin-left": "5px",
			"position": "relative"
		}

		this.empty();
		/* Creates 5 blocks inside the progress bar */
		elements = '<div id="block_1" class="samplebar"></div>';
		elements += '<div id="block_2" class="samplebar"></div>';
		elements += '<div id="block_3" class="samplebar"></div>';
		elements += '<div id="block_4" class="samplebar"></div>';
		elements += '<div id="block_5" class="samplebar"></div>';
		this.append(elements);

		/* Apply the CSS for all the blocks */
		$('.samplebar').css(barCSS);

		/* Show the progress bar */
		this.css('display', '');
		/* Animate the progress bar */
		animateUpdate();

		return this;
	}

	/* Distroys the progress bar and hides the div */
	$.fn.hideProgressBar = function() {

		if (progressBarTimer != '')
			clearTimeout(progressBarTimer);
		this.empty();
		this.css('display', 'none');

		return this;
	}

	/* Animates the progress bar. Its called by the timer */
	function animateUpdate()
	{
		/* Concept is first all the blocks will be with opacity 0.1. Starting from middle
		 * and changing the opacity of either side of the block to 1. If all the block is with
		 * opacity 1 change all the opacity to 0.1 and animate again
		 */
		/* check if block 3 is lighter */
		if ($("#block_3").css('opacity') < '1') {
			$("#block_3").css('opacity', '1');
		} else if ($("#block_2").css('opacity') < '1') { /* Check if block 2 is lighter */
			$("#block_2").css('opacity', '1');
			$("#block_4").css('opacity', '1');
		} else if ($("#block_1").css('opacity') < '1') { /* Check if block 1 is lighter */
			$("#block_1").css('opacity', '1');
			$("#block_5").css('opacity', '1');
		} else { /* Change all block's opacity to 1 */
			$("#block_1").css('opacity', '0.1');
			$("#block_2").css('opacity', '0.1');
			$("#block_3").css('opacity', '0.1');
			$("#block_4").css('opacity', '0.1');
			$("#block_5").css('opacity', '0.1');
		}
		if (progressBarTimer != '')
			clearTimeout(progressBarTimer);
		progressBarTimer = setTimeout(animateUpdate, progressInterval);

		return;
	}
}(jQuery));

/* Adds Stations to AP STA relation diagram from the JSON object from server*/
function drawAP(array, selectedMAC)
{
	var macarr = [];
	var classname;

	$('.apstamapleftcol tbody tr').remove();
	$('.apstamaprightcol tbody tr').remove();
	$('.centerap tbody tr').remove();

	/* for each STA's */
	$baseelem ='';
	for (i = 1;i < array.length; i++) {
		for (j = 0; j < array[i].length; j+=2) {
			macarr.push(array[i][j].MAC);
			if (selectedMAC == array[i][j].MAC)
				classname = 'selectedstabox';
			else
				classname = 'stabox';
			if (array[i][j].RSSI < -60)
				connectorclassname = 'staapconnectorlow';
			else if (array[i][j].RSSI < -40)
				connectorclassname = 'staapconnectormedium';
			else
				connectorclassname = 'staapconnectorgood';

			$baseelem += '<tr><td><img src="download.jpg" width="50px"></img></td>';
			$baseelem += '<td><div id="'+array[i][j].MAC+'" class="'+classname+' border" ';
			$baseelem += 'title="MAC: ' + array[i][j].MAC + ', RSSI: ' + array[i][j].RSSI + '"';
			$baseelem += 'data-mac = "'+ array[i][j].MAC +'">' + array[i][j].MAC + '</div></td>';
			$baseelem += '<td><div class = "staapconnectorgood">'+(j+1)+'</div></td></tr>';
		}
	}
	$('.apstamapleftcol tbody').append($baseelem);
	/* This is center AP */
	$baseelem = '<tr><td align="right"><b>SSID : </b></td><td>&nbsp;' + array[0].SSID + '</td></tr>';
	$baseelem += '<tr><td align="right"><b>BSSID : </b></td><td>&nbsp;' + array[0].BSSID + '</td></tr>';
	$baseelem += '<tr><td align="right"><b>Channel : </b></td><td>&nbsp;'+array[0].Channel +'</td></tr>';
	$('.centerap tbody').append($baseelem);

	/* for each STA's on the right side of AP*/
	$baseelem ='';
	for (i = 1;i < array.length; i++) {
		for (j = 1; j < array[i].length; j+=2) {
			var connectorclassname = '';
			macarr.push(array[i][j].MAC);
			if (selectedMAC == array[i][j].MAC)
				classname = 'selectedstabox';
			else
				classname = 'stabox';
			if (array[i][j].RSSI < -60)
				connectorclassname = 'staapconnectorlow';
			else if (array[i][j].RSSI < -40)
				connectorclassname = 'staapconnectormedium';
			else
				connectorclassname = 'staapconnectorgood';

			$baseelem += '<tr><td><div class = "'+connectorclassname+'">'+(j+1)+'</div></td>';
			$baseelem += '<td><div id="'+array[i][j].MAC+'" class="'+classname+' border" ';
			$baseelem += 'title="MAC: ' + array[i][j].MAC + ', RSSI: ' + array[i][j].RSSI + '"';
			$baseelem += 'data-mac = "'+ array[i][j].MAC +'">' + array[i][j].MAC + '</div></td>';
			$baseelem += '<td><img src="download.jpg" width="50px"></img></td></tr>';
		}
	}
	$('.apstamaprightcol tbody').append($baseelem);

	var height = $('.apstamapleftcol').height();
	if (height <= 220)
		height = 210;
	$('.sprator').attr('rowspan',array[1].length);
	$('.sprator').find('div.staseparator').height(height);

	return macarr;
}
