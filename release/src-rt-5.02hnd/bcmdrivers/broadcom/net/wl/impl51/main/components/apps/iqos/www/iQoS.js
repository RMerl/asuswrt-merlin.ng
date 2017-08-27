/*
 * Jquery implementation for Device Summary Tab
 *
 * $ Copyright Open Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: iQoS.js,v 1.0 2015-02-12 15:25:43 vbemblek $
 */

/*****
* Function which will get called when DOM for the page gets loaded.START
*****/

var gDeviceListTimers = [];
var gcategoryPerDeviceList = new categoryPerDeviceList(); /* List of all selected categories per device */
var gDeviceList = [];
var gDeviceSummary = [];
var gLinkXVal = [];
var gLinkYVal = [];
var gComboval = '';
var gComboindex = -1;
var isHTTPDServer = 1;
var urldevicelist;
var urldevicesummary;
var MbpstoBytes = (1024 * 1024); //Conversion of bits to mbps and from bytes to MB
var KbpstoBytes = (1024); //Conversion of bits to kbps and from bytes to KB
var refreshTime = 5000;
var maxToBeSelected = 6;
var urlGetConfig;
var urlNetworkSummary;
var isHTTPDServer = 1;

function initPage()
{
	if (isHTTPDServer == 1) {
		urldevicelist = 'http://'+ window.location.host +'/iQoS.cgi?Req=DeviceList';
		urldevicesummary = 'http://'+ window.location.host +'/iQoS.cgi?Req=DeviceSummary';
		urlGetConfig  = 'http://'+ window.location.host +'/iQoS.cgi?Req=GetConfig';
		urlNetworkSummary = 'http://'+ window.location.host +'/iQoSNetworkSummary.asp';
	} else {
		urldevicelist = "http://localhost/devicelist.php";
		urldevicesummary = "http://localhost/DeviceSummary.php";
	}
}

/*****
* Function To Check iQoS State.START
*****/
function CheckiQoSState(array)
{
	if (array.EnableiQoS == 0) {
		alert('Please Enable iQoS In Quick Settings' + '\n' + 'To Access Device Summary');
		$(location).attr('href',urlNetworkSummary);
		return;
	}
}

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
			CheckiQoSState(array);
		}
	});
}

/*****
* Function For appending devname and devtype name.
*****/
function prepareDeviceDisplayName(devType, devName, displayName)
{
	if (devType != undefined)
		displayName += "-" + devType;

	if (devName != undefined)
		displayName += "-" + devName;

	return displayName;
}

/*****
* Function For updating the combo list details.
* if name is empty we are updating the IPAddress of the device in the Combo list. START
*****/
function populateDeviceList(array)
{
	var bDeviceFound = 0;
	var ipAddr, devName, devType, displayName, Name, typeid;
	var elem;
	$('#devlist').empty();

	for (i = 0; i < array.Device.length; i++)
	{
		ipAddr = array.Device[i].IPAddr;
		devName = array.Device[i].DevName;
		devType = array.Device[i].DevType;
		Name = array.Device[i].Name;
		typeid = gDeviceList.Device[i].DevTypeInt;
		if(Name.length == 0) {
			displayName = prepareDeviceDisplayName(devType, devName, ipAddr);
			elem = '<option value="' + displayName + '"data-typeid="' + typeid + '">' + displayName + '</option>';
			$("#devlist").append(elem);
		} else {
			displayName = prepareDeviceDisplayName(devType, devName, Name);
			elem = '<option value="' + displayName + '"data-typeid="' + typeid + '">' + displayName + '</option>';
			$("#devlist").append(elem);
		}

		if ( gComboval == displayName) {
			gComboindex = i;
			bDeviceFound = 1;
		}
	}

	if(bDeviceFound == 0 && array.Device.length > 0) {
		gComboindex = 0;
		devName = array.Device[0].DevName;
		ipAddr = array.Device[0].IPAddr;
		devType = array.Device[0].DevType;
		Name = array.Device[0].Name;
		if (Name.length == 0) {
			gComboval = prepareDeviceDisplayName(devType, devName, ipAddr);
		} else {
			gComboval = prepareDeviceDisplayName(devType, devName, Name);
		}
		PopulateDeviceListDetails(array, gComboindex);
	}
}
/*****
* Function For updating the combo list details. END
*****/

/*****
* Function For updating the device list details.START
*****/
function PopulateDeviceListDetails(array, index)
{
	var $IPAddress;
	var $OSData;
	var $LastUsed;

	$('.systemdetailstable tr').remove();
	$IPAddress = '<tr><td>IP Address</td><td>: '+array.Device[index].IPAddr+'</td></tr>';
	$OSData += '<tr><td>OS</td><td>: '+array.Device[index].OS+'</td></tr>';
	$LastUsed += '<tr><td>Last Internet Access</td><td>: ' + ConvertEpochTimeToString(array.Device[index].LastUsed) + '</td></tr>';

	$('.systemdetailstable').append($IPAddress);
	$('.systemdetailstable').append($OSData);
	$('.systemdetailstable').append($LastUsed);
}
/*****
* Function For updating the device list details.END
*****/
/*****
* Function For Converting data from bytes to Mbps and Kbps.START
*****/
function DataConversionFromBytes(tobeconverted, DataTag)
{
	var ConvertedData = tobeconverted;
	if(DataTag == true)
	{
	/* as here ConvertedData is in term of bits so for bytes devide by 8 */
		if(ConvertedData >=MbpstoBytes)
			return( (ConvertedData/MbpstoBytes).toFixed(2) + " Mbps");
		else if(ConvertedData >=KbpstoBytes)
			return((ConvertedData/KbpstoBytes).toFixed(2) + " Kbps");
		else
			return(ConvertedData + " bps");
	}
	else
	{
		if(ConvertedData >=MbpstoBytes)
			return( (ConvertedData/MbpstoBytes).toFixed(2) + " Mb");
		else if(ConvertedData >=KbpstoBytes)
			return((ConvertedData/KbpstoBytes).toFixed(2) + " Kb");
		else
			return(ConvertedData + " bytes");
	}
}
/*****
* Function For Converting data from bytes to Mbps and Kbps.START
*****/
/*****
* Function For updating the Device summary.START
*****/
function populateDeviceSummary(array)
{
	var UpStreamBandwidth;
	var DownStreamBandwidth;
	var UpLinkBandwidth;
	var TotaldataDownloaded;
	var TotaldataUploaded;

	var upStream = DataConversionFromBytes(array.UpBW, true);
	var downStream = DataConversionFromBytes(array.DownBW, true);
	var upLinkBand = DataConversionFromBytes(array.BW, true);
	var totalDownloadedData = DataConversionFromBytes(array.DownBytes, false);
	var totalUploadedData = DataConversionFromBytes(array.UpBytes, false);

	$('.datatransferdetails tr').remove();

	DownStreamBandwidth = '<tr><td>Current Downstream Bandwidth Used</td><td>: '+downStream+' </td></tr>';
	UpStreamBandwidth += '<tr><td>Current Upstream Bandwidth Used</td><td>: '+upStream+' </td></tr>';
	UpLinkBandwidth += '<tr><td>Bandwidth Used in Network (Downstream + Upstream)</td><td>: '+upLinkBand+' </td></tr>';
	TotaldataDownloaded += '<tr><td>Total data Downloaded in last Session(' + array.RefreshInterval + ' Seconds)</td><td>: '+totalDownloadedData+'</td></tr>';
	TotaldataUploaded += '<tr><td>Total data Uploaded in last Session(' + array.RefreshInterval + ' Seconds)</td><td>: '+totalUploadedData+'</td></tr>';

	$('.datatransferdetails').append(UpStreamBandwidth);
	$('.datatransferdetails').append(DownStreamBandwidth);
	$('.datatransferdetails').append(UpLinkBandwidth);
	$('.datatransferdetails').append(TotaldataDownloaded);
	$('.datatransferdetails').append(TotaldataUploaded);

	populateApplicationList(array);
	drawLinkRate(array);
	populateSelectCategoryBox(array);
	drawBandwidthUsagebyCategory(array);
}
/*****
* Function For updating the Device summary.END
*****/
/*****
* Function For Filling the progress bar and the Application Packet Details.START
*****/
function populateApplicationList(array)
{
	var $element ='';
	var i, maxband = 0, incr= 0, nappcount = 0;
	var left=0;
	var vlineelem = '';
	var start = 0;

	$('.applicationlist tr').remove();

	var apps = array.Apps;

	for (i = 0; i < apps.length; i++) {
		if (apps[i].Name == undefined)
			continue;
		//bandwidth to Mbps....
		apps[i].BW = apps[i].BW/MbpstoBytes;

		if (maxband < apps[i].BW)
			maxband = apps[i].BW;
	}
	//Rounding off the bandwidth to fill the progress bar
	if (maxband > 50)
		maxband += (100-(maxband%100));
	else if (maxband > 10)
		maxband = 50;
	else if(maxband > 1)
		maxband = 10;
	else
		maxband = 1;

	incr = maxband/10;

	for (i = 0; i < 21; i++) {
		if (i %2 == 0) {
			vlineelem += '<div class="vlines" style="left:'+left+'%;">'+start+'</div>';
			start += incr;
			start = Math.round(start*100)/100;
		}
		else
			vlineelem += '<div class="vlines1" style="left:'+left+'%;"></div>';
		/* Gap between two lines in the bandwidth axis */
		left += 4.37;
	}
	$('.bandwidthyaxis').html(vlineelem);

	for (i = 0; i < apps.length; i++) {
		if (apps[i].Name == undefined)
			continue;
		var tofill = (apps[i].BW * 100)/maxband;

		$element += '<tr><td class="Line1"></td>';
		$element += '<td class="RectTextbox1">'+apps[i].Name+'<br>';
		$element += '<font COLOR=#008bb0><b>'+apps[i].Category+'</b></font></td>';
		$element += '<td class="RectTextbox2">Last Access:'+ConvertEpochTimeToString(apps[i].LastUsed)+'<br/>Downloaded Packets:'+apps[i].DownPkts+'<br/>Uploaded Packets: '+apps[i].UpPkts+'</td>';
		$element += '<td class="bandwidthusage"><div class="bandwidthusagediv" style="width:'+tofill+'%;"></div></td>'
		$element += '</tr>';
		nappcount++;
	}

	$('.applicationlist').append($element);

	$("#DeviceApplicationTable").height(nappcount*56+140);
	$(".vertical-line").height(nappcount*56);
}
/*****
* Function For Filling the progress bar and the Application Packet Details.END
*****/
function formatter(val, axis) {
    var d = new Date(val);
	var ret = ""+d.getMinutes()+":"+d.getSeconds()+"";

	return ret;
}
/*****
* Function For drawing the LinkRate Graph.START
*****/
function drawLinkRate(array)
{
	var dataval1 = [];
	var xaxisticks = [];
	var i, deviceBW = 0;
	var tempData = [];
	var dt, time;

	if (gLinkYVal.length >= 10) {
		gLinkXVal.splice(0,1);
		gLinkYVal.splice(0,1);
	}
	dt = new Date();
	time = dt.getMinutes() + ":" + dt.getSeconds();
	deviceBW = parseInt(array.DownBW, 10) + parseInt(array.UpBW, 10); /* Since  DownBW and UpBW comes as String */

	gLinkXVal.push(time); // get current time and add
	gLinkYVal.push(deviceBW); // change to MBPS
	for(i = 0; i < gLinkYVal.length; i++){
		tempData.push([i,(gLinkYVal[i]/MbpstoBytes).toFixed(3)]);
	}
	dataval1.push(tempData);

	for(i=0;i<gLinkXVal.length;i++){
		xaxisticks.push([i,gLinkXVal[i]]);
	}

	var options = {
		series:{
			lines:{
				show:true,
				align: 'center',
			},
			points:{
				show:true,
			}
		},
		xaxis:{
			show:true,
			axisLabel:'Time(Seconds)',
			tickLength:1,
			ticks:xaxisticks,
			autoscaleMargin:.05,
			axisLabelUseCanvas: true,
			axisLabelFontSizePixels: 16,
			axisLabelFontFamily: 'Arial, Verdana',
			axisLabelPadding: 10,
			//tickFormatter:formatter,
		},
		yaxis:{
			show:true,
			axisLabel: 'Mbps',
			axisLabelUseCanvas: true,
			axisLabelFontSizePixels: 16,
			axisLabelFontFamily: 'Arial, Verdana',
			axisLabelPadding: 3,
			min:0,
		},

		 legend: {
            labelBoxBorderColor: "none",
            position: "right"
        },
	};

	var dataset = [];
	dataset.push({hoverable:true,data:dataval1[0]});

	$.plot($("#LinkRateGraphContent"), dataset, options);
}
/*****
* Function For drawing the LinkRate Graph.END
*****/

/* Function for consolidating the different network protocol apps categories into one */
function updateCategories(cat)
{
	var protoCategoryInt = [18,19,20]; /* Three categories of n/w protocols */
	var bw = 0, name = '', upbytes = 0, downbytes = 0, isdirty = false;
	var jobj = {};

	/* Consolidate all the 3 n/w protocols data into one. */
	for (i = cat.length-1; i >= 0; i--) {
		if ($.inArray(cat[i].CategoryInt, protoCategoryInt) >= 0) {
			bw += parseInt(cat[i].BW, 10);
			name = cat[i].Name;
			upbytes += parseInt(cat[i].UpBytes, 10);
			downbytes += parseInt(cat[i].DownBytes, 10);
			cat.splice(i, 1);
			isdirty = true;
		}
	}

	/* Add consolidated data into array. */
	if (isdirty == true) {
		jobj["Name"] = name;
		jobj["BW"] = bw.toString();
		jobj["CategoryInt"] = protoCategoryInt[0];
		jobj["UpBytes"] = upbytes.toString();
		jobj["DownBytes"] = downbytes.toString();
		cat.push(jobj);
	}
}

/*****
* Function For drawing the Bandwidth usage category Graph.START
*****/
function drawBandwidthUsagebyCategory(array)
{
	var dataval1 = [];
	var xaxisticks = [];
	var tempData = [];
	var dataset = [];
	var i, j;
	var separators = [' ','-'];
	var cat = $.extend(true, [], array.Categories); /* cat having its own copy of categories */
	var selectedarray = [];

	/* Consolidate the n/w protocols categories. */
	updateCategories(cat);

	selectedarray = gcategoryPerDeviceList.filteredList(array.MacAddr).slice(0);

	for(i = 0, j = 0; i < cat.length; i++){
		if ($.inArray(cat[i].Name, selectedarray) >= 0) {
			cat[i].BW = (cat[i].BW/MbpstoBytes).toFixed(3);
			tempData.push([++j,cat[i].BW]);
		}
	}
	dataval1.push(tempData);

	for(i=0, j = 0;i<cat.length;i++){
		if ($.inArray(cat[i].Name, selectedarray) >= 0) {
			var xval = cat[i].Name;
			$(separators).each(function (index, element) {
				xval = xval.replace(element, element + '<br/>');
			});
			xaxisticks.push([++j,xval]);
		}
	}

	var options = {
			series: {
				bars: {
					show: true
				}
			},
			bars: {
				align: "right",
				barWidth: 0.5
			},
			xaxis: {
				show:true,
				axisLabel:'Categories',
				axisLabelUseCanvas: true,
				axisLabelFontSizePixels: 16,
				axisLabelFontFamily: 'Arial, Verdana',
				axisLabelPadding: 10,
				ticks: xaxisticks,
				tickLength:1,
				min:0,
				max:maxToBeSelected,
			},
			yaxis: {
				show:true,
				axisLabel: 'Mbps',
				axisLabelUseCanvas: true,
				axisLabelFontSizePixels: 16,
				axisLabelFontFamily: 'Arial, Verdana',
				axisLabelPadding: 10,
				min:0,
			}
		};

	dataset.push({hoverable:true,data:dataval1[0]});

	$.plot($("#BandwidthBarGraph"), dataset, options);
}
/*****
* Function For drawing the Bandwidth usage category Graph.END
*****/

/*****
* categoryPerDevice maintains the fixed no of selected categories
* for specified mac address.
* Provides the addCategory and removeCategory helper functions.
*****/
function categoryPerDevice(mac) {
	this.mac = mac;
	this.array = [];

	this.addCategory = function (category) {
		this.array.push(category);
	};

	this.removeCategory = function (category) {
		for(var i = this.array.length - 1; i >= 0; i--) {
			if(this.array[i] === category) {
				this.array.splice(i, 1);
				break;
			}
		}
	};
}

/*****
* categoryPerDeviceList maintains the list of categoryPerDevice objs
* Provides the addObj removeObj isDeviceAdded filteredList updateList
* helper functions for the list.
*****/
function categoryPerDeviceList() {
	this.list = [];

	this.addObj = function (obj) {
		this.list.push(obj);
	};

	this.removeObj = function(obj) {
		for(var i = this.list.length - 1; i >= 0; i--) {
			if (this.list[i].mac === obj) {
				this.list.splice(i, 1);
				break;
			}
		}
	};

	this.isDeviceAdded = function(obj) {
		var ret = false;

		for(var i = this.list.length - 1; i >= 0; i--) {
			if (this.list[i].mac === obj) {
				ret = true;
				break;
			}
		}

		return ret;
	}

	this.filteredList = function(obj) {
		var ret = [];

		for(var i = this.list.length - 1; i >= 0; i--) {
			if (this.list[i].mac === obj) {
				ret = this.list[i].array.slice(0);
				break;
			}
		}

		return ret;
	}

	this.filteredListLength = function(obj) {
		var ret;

		for(var i = this.list.length - 1; i >= 0; i--) {
			if (this.list[i].mac === obj) {
				ret = this.list[i].array.length;
				break;
			}
		}

		return ret;
	}

	this.updateList = function(mac, cat, opr) {
		for(var i = this.list.length - 1; i >= 0; i--) {
			if (this.list[i].mac === mac) {
				switch (opr) {
					case '+':
						this.list[i].addCategory(cat);
						break;
					case '-':
						this.list[i].removeCategory(cat);
						break;
					default:
						break;
				}
				break;
			}
		}
	}
}

/*****
* Function For updating the selected category list array.
*****/
function updateSelectedArrayList(cat, mac)
{
	var selectedarray = [];

	selectedarray = gcategoryPerDeviceList.filteredList(mac).slice(0);

	for (var i = 0; i < cat.length; i++) {
		if (($.inArray(cat[i].Name, selectedarray) < 0) &&
			(gcategoryPerDeviceList.filteredListLength(mac) < maxToBeSelected)) {
				gcategoryPerDeviceList.updateList(mac, cat[i].Name, '+');
		}
	}
}

/*****
* Function For populating the category box.START
* gcategoryPerDeviceList array holds the categories checked by user
* between different sessions of refreshtime. while selectedarray holds the
* category values to be displayed in the graph for current session.
*****/
function populateSelectCategoryBox(array)
{
	var i, j;
	var element = '';
	var selectedLen = 0, len;
	var cat = $.extend(true, [], array.Categories); /* cat having its own copy */
	var catnames = [], selectedarray = [];

	/* Consolidate all the n/w protocol categories into one. */
	updateCategories(cat);
	/*
	 * check whether the selected cats are already in per device selected category list
	 * if not add to them else use as selected array.
	 */
	if (gcategoryPerDeviceList.isDeviceAdded(array.MacAddr)) {
		/* Length is still < maxToBeSelected so need to add the category */
		if (gcategoryPerDeviceList.filteredListLength(array.MacAddr) < maxToBeSelected) {
			updateSelectedArrayList(cat, array.MacAddr);
		}
		selectedarray = gcategoryPerDeviceList.filteredList(array.MacAddr).slice(0);
	} else {
		var cat_obj = new categoryPerDevice(array.MacAddr);
		len = cat.length > maxToBeSelected ? maxToBeSelected : cat.length;
		for (i = 0; i < len; i++) {
			cat_obj.addCategory(cat[i].Name);
			selectedarray.push(cat[i].Name);
		}
		gcategoryPerDeviceList.addObj(cat_obj);
	}

	element += '<tr><td>Select Category List:Max('+maxToBeSelected+')</td></tr>';
	for(i = 0; i < cat.length; i++) {
		if( ($.inArray(cat[i].Name, selectedarray) >= 0) && selectedLen < maxToBeSelected) {
			element += '<tr><td><input id="selectcategorychk" type="checkbox" value="'+cat[i].Name+'" checked="checked">'+cat[i].Name+'</input></td></tr>';
			$('#selectcategorychk').not(':checked').prop("disabled",true);
			selectedLen++;
		} else {
			element += '<tr><td><input id="selectcategorychk" type="checkbox" value="'+cat[i].Name+'">'+cat[i].Name+'</input></td></tr>';
			$('#selectcategorychk').not(':checked').prop("disabled",false);
		}
	}

	$('.selectcategorytbl tr').remove();
	$('.selectcategorytbl').append(element);
	$('.selectcategorytbl').data('mac', array.MacAddr);
}
/*****
* Function For populating the category box.END
*****/
function populateApplication(array)
{
	updateApplicationSummary();
	if (array.Req == 'DeviceSummary') {
		gDeviceSummary = array.Device[0];
		populateDeviceSummary(array.Device[0]);
	}
}
/*****
* Function For updating Image based on the Device type.START
*****/
function populateDeviceImage(typeid)
{
	$('.toplogo tr td:nth-child(3)').empty();
	$('.DeviceImages').empty();

	deviceImage = GetIcon("devtype", typeid);
	$('.toplogo tr td:nth-child(3)').append('<image src='+deviceImage+' width="100px"  style="height=100px;"/>');
	$('.DeviceImages').append('<image src='+deviceImage+' width="7%"  style="position: relative; margin-top: 0px;"/>');
}
/*****
* Function For updating Image based on the Device type.END
*****/
/*****
* Function For updating the summary and Application Name.START
*****/
function updateApplicationSummary()
{
	var deviceSelectedElem;
	var appSelectedElem;

	var typeid = $("#devlist option:selected").data('typeid');
	var devname = $("#devlist option:selected").text();
	$('.deviceselectedtbl tr').remove();
	$('.appselectedtbl tr').remove();

	$deviceSelectedElem = '<tr><td>Summary : '+devname+'</td></tr>';
	$appSelectedElem = '<tr><td>Application : '+devname+'</td></tr>';

	$('.deviceselectedtbl').append($deviceSelectedElem);
	$('.appselectedtbl').append($appSelectedElem);

	populateDeviceImage(typeid);
}
/*****
* Function For updating the summary and Application Name.END
*****/

function getDeviceList(devname)
{
	var array = [];

	for(i=0;i<gDeviceListTimers.length;i++){
		clearTimeout(gDeviceListTimers[i]);
	}
	gDeviceListTimers = [];

	$.ajax({
		type:"GET",
		url:urldevicelist,
		async:false,
		timeout:refreshTime,
		success:function(result){
			if (isHTTPDServer == 1)
				array = result;
			else
				array = JSON.parse(result);

			gDeviceList = array;
			populateDeviceList(array);
			if (gComboval.length <= 0) {
				$("#devlist option:first-child").attr("selected","selected");
				gComboindex = 0;
				PopulateDeviceListDetails(array,gComboindex);
			}
			else
				$('#devlist').val(gComboval);
			gDeviceListTimers.push(setTimeout(function(){getDeviceList(devname);},refreshTime));
		}
	});

	if (gComboindex >= 0 && jQuery.isEmptyObject(array) == false && array.Device.length > gComboindex)
		getApplicationList(gComboval, array.Device[gComboindex].MacAddr);
	else
		gDeviceListTimers.push(setTimeout(function(){getDeviceList(devname);},refreshTime));
}

function getApplicationList(devname, mac)
{
	var arrayApps = [];

	$.ajax({
		type:"GET",
		url:urldevicesummary+'&MacAddr='+mac,
		async:false,
		timeout:refreshTime,
		success:function(result){
			if (isHTTPDServer == 1)
				arrayApps = result;
			else
				arrayApps = JSON.parse(result);
			populateApplication(arrayApps);
		}
	});
}

function onSelectDevice(devname)
{
	for(i=0;i<gDeviceListTimers.length;i++){
		clearTimeout(gDeviceListTimers[i]);
	}
	gDeviceListTimers = [];

	gLinkXVal = [];
	gLinkYVal = [];

	getDeviceList(devname);

}

function showcategory()
{
	var top = $("#selectcategorybtn").position().top;
	var left = $("#selectcategorybtn").position().left;
	$('.selectcategorytbl').css('top', top+33);
	$('.selectcategorytbl').css('left', left);
	$('.selectcategorytbl').toggle();
	$('.selectcategorytbl').css("z-index", 1000);
}
/*****
* Function which will get called when DOM for the page gets loaded.START
*****/
$(document).ready(function()
{
	initPage();

	getConfiguration();

	for(i=0;i<gDeviceListTimers.length;i++){
		clearTimeout(gDeviceListTimers[i]);
	}
	gDeviceListTimers = [];

	$("#devlist").change(function() {
		gComboval = $(this).val();
		gComboindex = $("#devlist option:selected").index();
		$('#devlist').val(gComboval);
		onSelectDevice($(this).val());
		PopulateDeviceListDetails(gDeviceList, gComboindex);
		updateApplicationSummary();

	});
	$('body').click(function(evt) {
		if($(evt.target).parents('.selectcategory').length==0) {
			$('.selectcategorytbl').css('display','none');
		}
	});
	$('#selectcategorybtn').click(function(e) {
		showcategory();
	});

	/****
	 * In below click event handler gcategoryPerDeviceList array maintains the selected
	 * categories for the all the sessions and the items can be filtered using mac address.
	****/
	$('.selectcategorytbl').on('click', '#selectcategorychk', function(){
		var mac = $('.selectcategorytbl').data('mac');
		if($(this).is(":checked")) {
			var countchecked = $(".selectcategorytbl input[type=checkbox]:checked").length;
			if (countchecked > maxToBeSelected) {
				$(this).removeAttr('checked');
				$('.selectcategorytbl input[type=checkbox]').not(':checked').attr("disabled",true);
			} else {
				$('.selectcategorytbl input[type=checkbox]').not(':checked').attr("disabled",false);
				gcategoryPerDeviceList.updateList(mac, $(this).val(), '+');
				drawBandwidthUsagebyCategory(gDeviceSummary);
			}
		} else {
			gcategoryPerDeviceList.updateList(mac, $(this).val(), '-');
			drawBandwidthUsagebyCategory(gDeviceSummary);
			$('.selectcategorytbl input[type=checkbox]').not(':checked').attr("disabled", false);
		}
	});

	$('.selectcategorytbl').css('display','none');
	var combotext = $("#devlist option:selected").text();
	onSelectDevice(combotext);
});
