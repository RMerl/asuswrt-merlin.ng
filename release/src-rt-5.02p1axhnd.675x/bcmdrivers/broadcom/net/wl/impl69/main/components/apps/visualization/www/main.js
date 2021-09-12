/*
 * Jquery implementation for Site Survey Tab
 *
 * $ Copyright Open Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: main.js 682079 2017-01-31 10:30:10Z spalanga $
 */

var aplistplotgraph;
var timer = [];
var reftwogchnl = [0,1,2,3,4,5,6,7,8,9,10,11,12];
var reffivegchnl = [35,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,98,100,102,104,106,108,
	110,112,114,116,118,120,122,124,126,128,130,132,134,136,138,140,142,144,146,147,149,151,153,
	155,157,159,161,163,165,167];
var optionsfiveg = [35,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,100,102,104,106,108,110,112,
	114,116,118,120,122,124,126,128,132,134,136,138,140,142,144,149,151,153,155,157,159,161,165,
	167];
var twoglabels = [[0,0],[1,1],[2,2],[3,3],[4,4],[5,5],[6,6],[7,7],[8,8],[9,9],[10,10],[11,11],
	[12,12],[13,13]];
var fiveglabels = [[0,''],[1,36],[2,''],[3,40],[4,''],[5,44],[6,''],[7,48],[8,''],[9,52],
	[10,''],[11,56],[12,''],[13,60],[14,''],[15,64],[16,'.'],[17,'.'],[18,'.'],[19,100],
	[20,''],[21,104],[22,''],[23,108],[24,''],[25,112],[26,''],[27,116],[28,''],[29,120],
	[30,''],[31,124],[32,''],[33,128],[34,''],[35,132],[36,''],[37,136],[38,''],[39,140],
	[40,''],[41,144],[42,'.'],[43,'.'],[44,149],[45,''],[46,153],[47,''],[48,157],[49,''],
	[50,161],[51,''],[52,165],[53,'']];

var stacurrent = [];
var timerdata = [];

var bssids = [];
var curselindex = '';
var curdut = '';

var prevSelBSSID = ''; /* Previous selected BSSID -> Network address */
var currentSelBSSID = ''; /* Current selected BSSID -> Network address */
var prevtableindex = '';
var remoteIPAddress = '';
var isRemoteEnabled = 0;
var urlremotesettings = '';
var urlconfigsettings = '';
var urlscanresult = '';
var isHTTPDWebserver = 1;
/* This will have the timestamp of the last scan happened in DUT */
var gPrevTimeStamp = 0;
/* TRUE whenever the scan button is clicked. Make it 0 when we get the latest scan result */
var gIsScanBtnClicked = 0;
/* TRUE if the Populate scan result has not been called. false after first time call */
var gIsFirstTimePopulate = 1;
/* Holds the value from the cookies about data collection is started or not */
var gIsStartDataCollection = 0;
/* To hold all the SSID, BSSID, RSSI details for animating the SSID labels */
var g_animateobj = [];
var g_timeranimate = []; /* Timer for animate SSID labels */
var g_IsDisableAnimate = 1; /* Whether to disable animate SSID label or not */

var gAllBandsArr = []; /* For storing the bands and its SSID and BSSID's */
var gScanReqCount = 0; /* For storing the no of scan requests sent. */
/* To get the IsStarted flag from the cookies. This will be 1 if the data collection is started */
function getIsStartedDatacollectionFromCookies()
{
	var tmpIsStartFlag = unescape($.cookie('isstarted'));
	var tmpValue;
	if (tmpIsStartFlag.length > 0) {
		tmpValue = tmpIsStartFlag.split(',');
	}
	return Number(tmpValue[0]);
}

/* Hides the progress bar displayed after scan button is clicked */
function hideprogress()
{
	$(".progressbarclass").hideProgressBar();
}

function initPage()
{
	hideprogress();

	if (isHTTPDWebserver == 1) {
		urlremotesettings = window.location.protocol + '//' + window.location.host +'/json.cgi';
		urlscanresult = window.location.protocol + '//' + window.location.host +'/json.cgi';
		urlconfigsettings = window.location.protocol + '//' + window.location.host +'/json.cgi';
	} else {
		urlremotesettings = 'http://localhost/remotesettings.php';
		urlscanresult = 'http://localhost/scanresult.php';
		urlconfigsettings = 'http://localhost/counterdata.php';
	}
	initCommonPage();
}

function isRemoteDebuggingEnabled()
{
	if (isRemoteEnabled == 1 && remoteIPAddress != "127.0.0.1")
		return 1;
	return 0;
}

/* This is to convert the BSSID into some ID. This removes all the character ':' */
function getIDFromBSSID(tmpBSSID)
{
	var tmpvar = tmpBSSID.replace(/:/g, ''); /* Remove all the character ':' */
	tmpvar = 'id' + tmpvar; /* Add string 'id' in the begining */

	return tmpvar;
}

function GetIndexFromSSID(selectedbssid)
{
	for (var i = 0; i < bssids.length; i++) {
		if ((bssids[i] == selectedbssid)) {
			return i;
		}
	}
	return -1;
}

function MainTainSelection(selectedsta)
{
	HighlightSeries(selectedsta.Networkaddress);
	$('#aplisttableid tr td:nth-child(2)').each(function() {
		if ($(this).text() == selectedsta.Networkaddress) {
			$(this).closest('tr').css("background-color","#6BAAFC");
			$(this).closest('tr').css("color","white");
		}
	});
}

function HighlightSeries(selectedbssid)
{
	var pltsrs = aplistplotgraph.getData();
	var i = GetIndexFromSSID(selectedbssid);
	curselindex = i;

	if (i == -1)
		return;
	if ((i < pltsrs.length) && (pltsrs[i].bssid == selectedbssid)) {
		aplistplotgraph.highlightSeries(pltsrs[i],'#C60F0F', i);

		var lbl = '#' + getIDFromBSSID(selectedbssid);
		if (g_IsDisableAnimate == 0) {
			$(lbl).css('display', '');
			$(lbl).css("padding", "3");
			$(lbl).css("height", "20");
			$(lbl).css("z-index", 2000);
		} else {
			$(lbl).css("z-index", 1000);
		}
		$(lbl).css("font-size", "20px");
		$(lbl).css("font-weight", "bold");
		$(lbl).css("background", "#C60F0F");
		$(lbl).css("color", "white");
		$(lbl).addClass("border");
	}
}

function UnHighlightSeries(incurselindex)
{
	var pltsrs = aplistplotgraph.getData();
	var i = incurselindex;
	var curdutidx = GetIndexFromSSID(curdut);

	if (i == -1)
		return;
	if ((i < pltsrs.length) && (pltsrs[i].bssid == prevSelBSSID)) {
		aplistplotgraph.unHighlightSeries(pltsrs[i], i);
		var lbl = '#' + getIDFromBSSID(prevSelBSSID);
		$(lbl).css("font-size", "12pt");
		$(lbl).css("font-weight", "normal");
		if (g_IsDisableAnimate == 0) {
			$(lbl).css("background-color", "#CDD6DB");
			$(lbl).css("padding", "0");
			$(lbl).css("height", "15");
			$(lbl).css("z-index", 1000);
		} else {
			$(lbl).css("background", "transparent");
			$(lbl).css("z-index", 1);
		}
		if (i == curdutidx) /* If it is the current DUT color should be red */
			$(lbl).css("color", "#990000");
		else
			$(lbl).css("color", "#005568");
	}
}

/* Get the string for RSSI level to store it in animate object
 * This string is used as name in JSON object to hold all the
 * SSID's in that RSSI range
 */
function getStringFromRSSILevel(rssi)
{
	if (rssi < 0 && rssi >= -20)
		return "zero";
	else if (rssi < -20 && rssi >= -40)
		return "twenty";
	else if (rssi < -40 && rssi >= -60)
		return "fourty";
	else if (rssi < -60 && rssi >= -80)
		return "sixty";
	else if (rssi < -80 && rssi >= -100)
		return "eighty";
}

/* Here we are going to divide each AP's based on the
 * RSSI levels (0 to -20, -20 to -40 ect..) for each center channel.
 * Prepare JSON object for holding SSID, BSSID and RSSI objects for animating the SSID labels
 * The complete structure is stored as JSON object. Format of JSON object is,
 * [ {"Channel" : 36, "ChannelObjs" : [ { "RSSILevel" : "twenty", "NextIDX" : 0, "RSSIObjs" : [
 * { "SSID" : "Some", "BSSID" : "AA:BB:CC:DD:EE:FF"} ] } ] } ]
 */
function prepareAnimateObject()
{
	g_animateobj = [];

	/* Parse through all the surrounding AP's and prepare the JSON object */
	for (i = 0; i < stacurrent.length; i++) {
		var tmpfound = false; /* Whether the Channel is found in the JSON object or not */
		var j = 0, k = 0;
		var tmpobj = [];

		/* Check whether the channel is already in JSON object or not.
		 * If present no need to add JSON object for that channel
		 */
		for (j = 0; j < g_animateobj.length; j++) {
			if (g_animateobj[j].Channel == stacurrent[i].Channel) {
				tmpfound = true;
				break;
			}
		}
		var tmprssistr = getStringFromRSSILevel(stacurrent[i].Signal);

		/* Create object for SSID label to add it to the global JSON object */
		tmpobj["SSID"] = stacurrent[i].SSID;
		tmpobj["BSSID"] = stacurrent[i].Networkaddress;
		/* If channel is found in JSON object no need to create channel object */
		if (tmpfound) {
			var tmprssifound = false;
			/* Now check whether RSSI level object also present
			 * in the global JSON object
			 */
			for (k = 0; k < g_animateobj[j].ChannelObjs.length; k++) {
				if (tmprssistr == g_animateobj[j].ChannelObjs[k].RSSILevel) {
					tmprssifound = true;
					g_animateobj[j].ChannelObjs[k].RSSIObjs.push(tmpobj);
				}
			}
			/* If the RSSI level object is not found need to cerate the JSON object
			 * for that RSSI level and add the SSID label object to it
			 */
			if (!tmprssifound) {
				var tmprssiobj = [];
				tmprssiobj["RSSILevel"] = tmprssistr;
				tmprssiobj["NextIDX"] = -1;
				tmprssiobj["RSSIObjs"] = [];
				tmprssiobj["RSSIObjs"].push(tmpobj);
				g_animateobj[j].ChannelObjs.push(tmprssiobj);
			}
		} else { /* Here create the new Channel object, create the RSSI object */
			var tmpchobjs = [];
			var tmprssiobj = [];
			tmprssiobj["RSSILevel"] = tmprssistr;
			tmprssiobj["NextIDX"] = -1;
			tmprssiobj["RSSIObjs"] = [];
			tmprssiobj["RSSIObjs"].push(tmpobj);
			tmpchobjs["Channel"] = stacurrent[i].Channel;
			tmpchobjs["ChannelObjs"] = [];
			tmpchobjs["ChannelObjs"].push(tmprssiobj);
			g_animateobj.push(tmpchobjs);
		}
	}
}

/* As the SSID labels whose signal strength is same overlaps each other, we came out with SSID
 * animation solution. Here we are going to divide each AP's based on the
 * RSSI levels (0 to -20, -20 to -40 ect..) for each center channel.
 * In each RSSI level if there are more than one AP, we will show only one label at a time which
 * will keeps on changing for every 2 secons showing the nect AP's label.
 * If there is only one AP in that range then there will not be any animation
 */
function animateSSIDLabels()
{
	/* Parse each channel objects for finding the SSID's in RSSI range */
	for (i = 0; i < g_animateobj.length; i++) {
		/* For each Channel */
		for (j = 0; j < g_animateobj[i].ChannelObjs.length; j++) {
			/* If there is only one AP in this RSSI range just show the SSID label. No animation */
			if (g_animateobj[i].ChannelObjs[j].RSSIObjs.length == 1) {
				var tmpbssid = getIDFromBSSID(g_animateobj[i].ChannelObjs[j].RSSIObjs[0].BSSID);
				$("#"+tmpbssid).css('display', '');
			} else { /* Check the next index to show */
				var tmpnextidx = g_animateobj[i].ChannelObjs[j].NextIDX;
				if (tmpnextidx == -1) {
					tmpnextidx = 0;
				} else {
					/* Now hide the already showing SSID label */
					if (g_animateobj[i].ChannelObjs[j].RSSIObjs[tmpnextidx].BSSID != currentSelBSSID) {
						var tmpbssid = getIDFromBSSID(g_animateobj[i].ChannelObjs[j].RSSIObjs[tmpnextidx].BSSID);
						$("#"+tmpbssid).css('display', 'none');
					}
					/* Increment the index to show next SSID */
					tmpnextidx++;
					if (tmpnextidx >= g_animateobj[i].ChannelObjs[j].RSSIObjs.length)
						tmpnextidx = 0;
				}
				/* Show the next SSID label */
				var tmpbssid = getIDFromBSSID(g_animateobj[i].ChannelObjs[j].RSSIObjs[tmpnextidx].BSSID);
				$("#"+tmpbssid).css('z-index', 3000);
				$("#"+tmpbssid).css('display', '');
				/* Update the next index */
				g_animateobj[i].ChannelObjs[j].NextIDX = tmpnextidx;
			}
		}
	}
	/* Clear the old timer and create a timer for 2 seconds */
	for (i = 0; i < g_timeranimate.length; i++) {
		clearTimeout(g_timeranimate[i]);
	}
	g_timeranimate = [];
	g_timeranimate.push(setTimeout(function(){animateSSIDLabels();}, 2000));
}

function getScanError(err)
{
	var errval = "";
	if (err & 1) {
		errval = "Unable to perform scan.Showing cached results"
	}

	return errval;
}

function displayError(err)
{
	console.log(err);
	$elem = '<span class="errorstyle">' + err +'</span>';

	$("#errmsg").empty();
	$("#errmsg").append($elem);
}

/* Populate array to plot RSSI and draw AP list table from the output data */
function drawGraphAndTableList(filterval)
{
	var selectedsta = '';
	var $element = '';
	var xaxislabels = [];
	var graphlabel = [];
	var graphdata = [];
	var chnlfilter = $("#chnlselectcntrl option:selected").text();
	var bndwthfltr = $("#bndwthselectcntrl option:selected").text();
	var curdutidx = -1;
	var errinfo = "";

	if (g_IsDisableAnimate == 0)
		prepareAnimateObject();

	bssids = [];
	for (i = 0, idx = 0; i<stacurrent.length; i++) {
		/* Apply the channel and bandwidth filter */
		if (chnlfilter != 'Select Channel' && stacurrent[i].Channel != chnlfilter) {
			continue;
		} else if (bndwthfltr != 'Select Bandwidth' && stacurrent[i].Bandwidth != bndwthfltr) {
			continue;
		}

		if (stacurrent[i].Networkaddress == currentSelBSSID) {
			selectedsta = stacurrent[i];
		}
		if (stacurrent[i].IsCurrent == 1) {
			curdut = stacurrent[i].Networkaddress;
			curdutidx = idx;
			errinfo = getScanError(stacurrent[i].ErrorInfo);
		}

		temp = [];
		var index;
		if (filterval == '2') { /* If 2.4 GHz */
			if (stacurrent[i].Channel > 14)
				continue;
			index = $.inArray(stacurrent[i].Channel,reftwogchnl);
		} else if (filterval == '5') {
			if (stacurrent[i].Channel < 14)
				continue;
			index = $.inArray(stacurrent[i].Channel,reffivegchnl);
		}

		if (filterval == '2') { /* If 2.4 GHz */
			if (stacurrent[i].Bandwidth == '20') {
				temp.push([index-2,-100],[index,stacurrent[i].Signal],[index+2,-100]);
			} else if (stacurrent[i].Bandwidth == '40') {
				temp.push([index-4,-100],[index,stacurrent[i].Signal],[index+4,-100]);
			} else if (stacurrent[i].Bandwidth == '80') {
				temp.push([index-8,-100],[index,stacurrent[i].Signal],[index+8,-100]);
			}
		} else {
			if (stacurrent[i].Bandwidth == '20') {
				temp.push([index-1,-100],[index,stacurrent[i].Signal],[index+1,-100]);
			} else if (stacurrent[i].Bandwidth == '40') {
				temp.push([index-2,-100],[index,stacurrent[i].Signal],[index+2,-100]);
			} else if (stacurrent[i].Bandwidth == '80') {
				temp.push([index-4,-100],[index,stacurrent[i].Signal],[index+4,-100]);
			}
		}
		graphdata.push(temp);
		graphlabel.push(stacurrent[i].SSID);

		bssids.push(stacurrent[i].Networkaddress);
		idx++;
	}

	/* populate the AP list table */
	$('#aplisttableid tbody tr').remove();
	var cntrchnl;
	var cnterchnl;
	for (i = 0; i<stacurrent.length; i++) {
		/* Apply the channel and bandwidth filter */
		if (chnlfilter != 'Select Channel' && stacurrent[i].Channel != chnlfilter) {
			continue;
		} else if (bndwthfltr != 'Select Bandwidth' &&stacurrent[i].Bandwidth != bndwthfltr) {
			continue;
		}
		if (stacurrent[i].Bandwidth >= 40) {
			if (stacurrent[i].Bandwidth == 80 || stacurrent[i].Bandwidth == 160) {
				cntrchnl = stacurrent[i].ControlChannel;// + '/80';
			} else {
				if (stacurrent[i].ControlChannel > stacurrent[i].Channel) {
					cntrchnl = stacurrent[i].ControlChannel + 'u';
				} else {
					cntrchnl = stacurrent[i].ControlChannel + 'l';
				}
			}
			cnterchnl = stacurrent[i].Channel;
		} else {
			cntrchnl = stacurrent[i].ControlChannel;
			cnterchnl = stacurrent[i].Channel;
		}

		if (stacurrent[i].Networkaddress == curdut)
			$element += '<tr class="curduttr">';
		else
			$element += '<tr>';
		$element += '<td align="left">'+ stacurrent[i].SSID + '</td>';
		$element += '<td class="network" align="left">'+ stacurrent[i].Networkaddress; + '</td>';
		var sgnstrngth = stacurrent[i].Signal;
		if (stacurrent[i].Networkaddress == curdut) {
			$element += '<td><span><div class="wtht1" style="width:80px;">';
			$element += '<div class = "wtht1 brcm" style="width:'+ (((100+sgnstrngth)*80)/100) +'px;">';
			$element += '</div></div></span>'+ sgnstrngth + '</td>';
		}
		else {
			$element += '<td><span><div class="wtht1" style="width:80px;">';
			$element += '<div class = "wtht1 blu" style="width:'+ (((100+sgnstrngth)*80)/100) +'px;">';
			$element += '</div></div></span>'+ sgnstrngth + '</td>';
		}
		$element += '<td align="right" style="padding-right:2%;">' + stacurrent[i].SNR + '</td>';
		$element += '<td align="right" style="padding-right:2%;">'+ stacurrent[i].Bandwidth + '</td>';
		$element += '<td align="right" style="padding-right:2%;">'+ cnterchnl + '</td>';
		$element += '<td align="right" style="padding-right:2%;">'+ cntrchnl + '</td>';
		$element += '<td align="right" style="padding-right:3%;">'+ stacurrent[i].Speed + '</td>';
		$element += '<td>'+ stacurrent[i].Type + '</td>';
		$element += '<td>'+ stacurrent[i].Security + '</td>';
		$element += '</tr>';
	}
	if (filterval == '2') { /* If 2.4 GHz */
		xaxislabels=twoglabels;
	} else if (filterval == '5') {
		xaxislabels=fiveglabels;
	}

	$("#aplistplaceholder").find('table > tbody').append($element).trigger('update');

	var $sort = $('#aplisttableid').get(0).config.sortList;
	setTimeout(function() {$("#aplisttableid").trigger("sorton",[$sort])},50);

	drawRssiGraph(graphdata,graphlabel,xaxislabels,curdutidx);

	if (selectedsta != '') {
		curselindex = GetIndexFromSSID(selectedsta.Networkaddress);
		MainTainSelection(selectedsta);
	} else {
		currentSelBSSID = '';
		curselindex = -1;
	}

	if (errinfo.length > 0) {
		displayError(errinfo);
	}
}

function populateTheApListTable(selectedBand)
{
	var array = [];

	gScanReqCount++;
	if (gIsScanBtnClicked == 1) // If scan button is clicked display the progress bar
		$(".progressbarclass").showProgressBar();
	$.get(
	urlscanresult,
	{Req:'ScanResults',DutMac:selectedBand.mac,FreqBand:selectedBand.band,DoScan:gIsScanBtnClicked},
	function(resultdata,status) {
		if (isHTTPDWebserver == 1)
			array = resultdata;
		else
			array = JSON.parse(resultdata);

		stacurrent = array[1];

		gIsScanBtnClicked = 0;
		/*
		 * If it is not first time populate check the previous time stamp and current time stamp
		 * to check whether to update the UI or not
		 */
		if (gIsFirstTimePopulate == 0) {
			if (stacurrent.length > 0 && array[0].Timestamp == gPrevTimeStamp && gScanReqCount < 5) {
				if (gIsStartDataCollection == 0)
					return;
				if (selectedBand.mac == $("#bandselectcntrl").val()) {
					/* This is to get the scan result after clicking the scan button, so making
					 * the request every 3 seconds. So that the uI refreshes faster once dcon
					 * gets the data from collector. 3 seconds is taken such that
					 * it should not burden the dcon and also to refresh the data faster.
					 */
					timer.push(setTimeout(function() {populateTheApListTable(selectedBand);},3000));
				}
				return;
			} else {
				hideprogress();
			}
			gPrevTimeStamp = array[0].Timestamp; /* store the timestamp */
			gScanReqCount = 0; /* Reset the scan count to 0. */
		}

		drawGraphAndTableList(selectedBand.band);
		if (gIsFirstTimePopulate == 1) {
			gIsFirstTimePopulate = 0;
			if (selectedBand.mac == $("#bandselectcntrl").val())
				timer.push(setTimeout(function() {populateTheApListTable(selectedBand);},
					timerdata[0]));
		}
	});
}

function drawRssiGraph(graphdata, graphlabel, xaxislabels, curdutidx)
{
	var dataset = [];

	for (i = 0; i < graphdata.length; i++) {
		if (i == curdutidx) {
			dataset.push({
				label:graphlabel[i],
				data:graphdata[i],
				curvedLines:{apply:true},
				color:"#990000",
				bssid:bssids[i]});
		}
		else {
			dataset.push({
				label:graphlabel[i],
				data:graphdata[i],
				curvedLines:{apply:true},
				color:"#005568",
				bssid:bssids[i]});
		}
	}
	var options = {
		series:{
			curvedLines:{
				active:true
			},
			lines:{
				show:true,
				lineWidth:2
			},
			clickable:true,
			highlightColor:"red"
		},
		highlightSeries: {
			autoHighlight: true
		},
		grid:{
			hoverable: false,
			backgroundColor: { colors: ["#CDD6DB", "#E6EDF0"] },
			markings: function (axes) {
				var markings = [];
				for (var x = Math.floor(axes.yaxis.min); x < axes.yaxis.max; x += 20)
					markings.push({ yaxis: { from: x, to: x+.25 }, color: "white" });
				return markings;
			}
		},
		legend:{
			show:false
		},
		xaxis:{
			axisLabel:'Channels',
			show:true,
			tickLength:1,
			ticks:xaxislabels,
			autoscaleMargin:.01,
			axisLabelFontSizePixels: 22,
			axisLabelUseCanvas: true,
			axisLabelFontFamily: 'Arial',
			axisLabelPadding: 20,
		},
		yaxis:{
			axisLabel:'Signal Strength [dBm]',
			min:-100,
			max:0,
			show:true,
			tickLength:1,
			axisLabelFontSizePixels: 22,
			axisLabelUseCanvas: true,
			axisLabelFontFamily: 'Arial',
			axisLabelPadding: 15,
		},
	};

	aplistplotgraph = $.plot($("#rssigraphplaceholder"),dataset,options);

	for (i = 0; i < graphdata.length; i++) {
		var ssid = graphlabel[i];
		var tmpbssid = getIDFromBSSID(bssids[i]);
		var lncolor = "#005568";
		if (i == curdutidx)
			lncolor = "#990000";
		else
			lncolor = "#005568";
		$.each(aplistplotgraph.getData()[i].data, function(i, el) {
			var o = aplistplotgraph.pointOffset({x:el[0],y:el[1]});
			if (el[1] != '-100') {
				if (g_IsDisableAnimate == 0) {
					$('<div class="data-point-label fontfamly" id="'+tmpbssid+'">'+ssid+'</div>').css({
						position:'absolute',
						left: o.left-40,
						top: o.top -17,
						color:lncolor,
						//padding:'5px',
						display:'none',
						"z-index":i+1000,
						"background-color":"#CDD6DB",
						"border-radius": 5,
						height:15,
					}).appendTo(aplistplotgraph.getPlaceholder());
				} else {
					$('<div class="data-point-label fontfamly" id="'+tmpbssid+'">'+ssid+'</div>').css({
						position:'absolute',
						left: o.left-40,
						top: o.top -20,
						color:lncolor,
						padding:'5px',
					}).appendTo(aplistplotgraph.getPlaceholder());
				}
			}
		});
	}
	if (g_IsDisableAnimate == 0)
		animateSSIDLabels();
}

function drawchnlfilters(chnl)
{
	$("#chnlselectcntrl").find('option').remove();
	if (chnl.length > 0) {
		var $element = '<option>Select Channel</option>'
		for (i = 1; i < chnl.length-1; i++) {
			$element += '<option>' + chnl[i] + '</option>';
		}
		$("#chnlselectcntrl").append($element);
	}
}

function LoadTheScanresults()
{
	var timerval;
	timerval = unescape($.cookie('timerinterval'));
	if (timerval.length > 0) {
		timerdata = timerval.split(',');
	}
	timerdata[0] = Number(timerdata[0]);
	gIsStartDataCollection = getIsStartedDatacollectionFromCookies();

	/* If there are atleast one interface(2G or 5G select first one by default */
	if (gAllBandsArr.length > 0) {
		if (isRemoteDebuggingEnabled() == 0) {
			populateTheApListTable(gAllBandsArr[0]);
		}
		if (gAllBandsArr[0].band == '2') /* If 2.4 GHz band */
			drawchnlfilters(reftwogchnl);
		else
			drawchnlfilters(optionsfiveg);

		$("#aplisttableid").tablesorter({
			sortList: [[2,1]],
			headers: {
				/* For control channel column which has both numeric and non numeric values */
				6: { sorter: "digit" }
			}
		});
	}
}

function initalizeChangeBandSelection()
{
	gIsScanBtnClicked = 0;
	hideprogress();
	gPrevTimeStamp = 0;
	gScanReqCount = 0;
	$("#errmsg").empty();
}

function onBandSelection(selectedBand)
{
	if (isRemoteDebuggingEnabled())
		return;
	initalizeChangeBandSelection();
	if (selectedBand.band == '2') /* If 2.4 GHz */
		drawchnlfilters(reftwogchnl);
	else
		drawchnlfilters(optionsfiveg);
	for (i = 0; i < timer.length; i++) {
		clearTimeout(timer[i]);
	}
	populateTheApListTable(selectedBand);
}

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
		$("#rssigraphplaceholder1").hide();
		$("#aplistplaceholder").hide();
		var errorstr = 'Remote debugging is enabled. Please go to <b>'+remoteIPAddress+'</b>.';
		var errorelem = '<div id="errormsgid" class="errormsg">'+errorstr+'</div>';
		$("#showerrmessage").append(errorelem);
		return 1;
	}
	return 0;
}

/* On applying any filter */
function onChangeFilterValue()
{
	var selectedBand = getBandChangeData('#bandselectcntrl', gAllBandsArr);

	drawGraphAndTableList(selectedBand.band);
}

$(document).ready(function()
{
	$("#bandselectcntrl").change(function() {
		var selectedBand = getBandChangeData(this, gAllBandsArr);

		onBandSelection(selectedBand);
	});

	/* On changeing the channel filter */
	$("#chnlselectcntrl").change(function() {
		onChangeFilterValue();
	});

	/* On changing bandwidth filter */
	$("#bndwthselectcntrl").change(function() {
		onChangeFilterValue();
	});

	InitConfig();
	initPage();

	gAllBandsArr = getBands();
	parseBandsAndInsert('#bandselectcntrl', gAllBandsArr);

	if (retrieveRemoteDebug() == 1)
		return;
	/* Get the animate NVRAM value */
	var tmp = getNVRAMValue('vis_disable_animate');
	g_IsDisableAnimate = parseInt(tmp, 10);

	setTimeout(LoadTheScanresults,100);
});

$(function() {
$(document).on("click",'#aplisttableid > tbody > tr',function() {
	var myindex = ($(this).closest("tr").index());

	if (prevSelBSSID == '' && currentSelBSSID == '') {
		currentSelBSSID = prevSelBSSID =  $(this).find("td:eq(1)").html();
		prevtableindex = myindex;
	} else {
		currentSelBSSID = $(this).find("td:eq(1)").html();
	}

	if (currentSelBSSID != prevSelBSSID) {
		$('#aplisttableid tr td:nth-child(2)').each(function() {
			if ($(this).text() == prevSelBSSID) {
				if (prevtableindex % 2 == 0) {
					$(this).closest('tr').css("background-color","#CDD6DB");
				} else {
					$(this).closest('tr').css("background-color","#E6EDF0");
				}
				if (curdut == prevSelBSSID)
					$(this).closest('tr').css("color","#990000");
				else
					$(this).closest('tr').css("color","black");
				UnHighlightSeries(curselindex);
			}
		});
		prevSelBSSID = currentSelBSSID;
	}
	prevtableindex = myindex;
	HighlightSeries($(this).find(".network").html());
	$(this).css("background-color","#3399FF");
	$(this).css("color","white");
});

$('#scanbutton').click(function(event)
{
		/* Not required to call scan if the data collection is not started */
		if (gIsStartDataCollection == 0)
			return;
		/* If there are no bands(2G or 5G) dont scan */
		if (gAllBandsArr.length > 0) {
			gIsScanBtnClicked = 1;
			var selectedBand = getBandChangeData('#bandselectcntrl', gAllBandsArr);

			for (i = 0; i < timer.length; i++) {
				clearTimeout(timer[i]);
			}
			populateTheApListTable(selectedBand);
		}
	});
});

$(window).bind('beforeunload',function() {
	$('#bandselectcntrl > option:eq(0)').prop('selected', true);
	$('#chnlselectcntrl > option:eq(0)').prop('selected', true);
	$('#bndwthselectcntrl > option:eq(0)').prop('selected', true);
})
