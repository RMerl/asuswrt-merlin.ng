/*
 * Java-Script implementation for wbd demo page
 *
 * $ Copyright Open Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: wbd_demo.js 761035 2018-05-04 14:59:23Z pj888946 $
 */

var g_isHTTPDWebserver = 1
var g_urlGetWbdMasterinfo;
var g_urlGetConfig;
var g_urlGetWbdMasterlogs;
var g_wbdarray;
var g_prevwbdarray;
var g_wbddiffjson;
var g_wbdMasterInfoTimeout = [];		/* To Hold the settimeout timer */
var g_wbdTimeOutVal = 1000;		/* Timeout value for calling wbd info */
var g_isLayoutRequired = true;
var g_weakclientlist = new weakclientlist();
var g_wbdDefRSSI2G = -65			/* Default rssi value */
var g_wbdDefRSSI5G = -65;			/* Default rssi value */
var TWO_G = 2;					/* 2g band define */
var FIVE_G = 5;					/* 5g band define */
var g_wbdMasterLogsTimeout = [];		/* To hold timeout for master logs. */
var TIMER_MASTERINFO = 1 << 0;			/* Master info timer flag.  */
var TIMER_MASTERLOGS = 1 << 1;			/* Master logs timer flag. */

/* Return default RSSI based on band */
function getDefaultRSSI(band)
{
	var rssi = (band == 2 ) ? g_wbdDefRSSI2G : g_wbdDefRSSI5G;

	return rssi
}

/* Returns formated rssi value. */
function getFormatedRSSI(band, rssi, sta_status) {
	var rssi_data = "> " + getDefaultRSSI(band) + "dBm (Strong)";

	if (rssi != 0) {
		rssi_data = rssi + "dBm";
		if (sta_status.length > 0) {
			rssi_data += " (" + sta_status + ")";
		}
	}

	return rssi_data;
}

/* To initialize some variables on page load */
function initWbdPage()
{
	if (g_isHTTPDWebserver == 1) {
		g_urlGetWbdMasterinfo = window.location.protocol + '//' + window.location.host +'/wbd.cgi';
		g_urlGetConfig = window.location.protocol + '//' + window.location.host +'/wbd.cgi';
		g_urlGetWbdMasterlogs = window.location.protocol + '//' + window.location.host +'/wbd.cgi';
	} else {
		g_urlGetWbdMasterinfo = 'http://localhost/wbd.php';
	}
}

/* Generates ids from bssid */
function getIdFromBssid(bssid)
{
	var temp = bssid.replace(/:/g,'');

	return temp;
}

/* Clear all the timers */
function clearAllWbdTimers(flag)
{
	if (flag & TIMER_MASTERINFO) {
		for (i = 0; i < g_wbdMasterInfoTimeout.length; i++) {
			clearTimeout(g_wbdMasterInfoTimeout[i]);
		}
		g_wbdMasterInfoTimeout = [];
	}

	if (flag & TIMER_MASTERLOGS) {
		for (i = 0; i < g_wbdMasterLogsTimeout.length; i++) {
			clearTimeout(g_wbdMasterLogsTimeout[i]);
		}
		g_wbdMasterLogsTimeout = [];
	}
}

/* Gets the config */
function getConfigInfo()
{
	var array = [];

	$.ajax({
		type:'GET',
		url:g_urlGetConfig,
		data:{"Cmd" : "config"},
		async:true,
		success:function(result) {
			if (g_isHTTPDWebserver == 1)
				array = result;
			else
				array = JSON.parse(result);
			if (array.RefreshInterval != undefined)
				g_wbdTimeOutVal = array.RefreshInterval * 1000;

			if (array.RSSI5G != undefined)
				g_wbdDefRSSI5G = array.RSSI5G;

			if (array.RSSI2G != undefined)
				g_wbdDefRSSI2G = array. RSSI2G
		}
	});
	array = [];
}

/* Common function to request from the server */
function getMasterInfoFromServer(request)
{
	var array = [];

	$.ajax({
		type:'GET',
		url:g_urlGetWbdMasterinfo,
		data:request,
		async:false,
		success:function(result) {
			if (g_isHTTPDWebserver == 1)
				array = result;
			else
				array = JSON.parse(result);
			g_wbdarray = array;
			$(document).trigger("data-available", [g_wbdarray]);
		}
	});
	array = [];
}

/* Function for requesting master info. */
function getMasterInfo()
{
	var request = {"Cmd" : "info"};

	clearAllWbdTimers(TIMER_MASTERINFO);
	getMasterInfoFromServer(request);
	g_wbdMasterInfoTimeout.push(setTimeout(function(){ getMasterInfo(); }, g_wbdTimeOutVal));
}

/* Function to request master logs from server. */
function getMasterLogsFromServer(request)
{
	var array = [];

	$.ajax({
		type:'GET',
		url:g_urlGetWbdMasterlogs,
		data:request,
		async:false,
		success:function(result) {
			if (g_isHTTPDWebserver == 1) {
				array = result;
			} else {
				array = JSON.parse(result);
			}

			displayLogMessages(array.Data.MasterLogs);
		}
	});
	array = [];
}

/* Function for requesting master logs. */
function getMasterLogs()
{
	var request = {"Cmd" : "logs"};

	clearAllWbdTimers(TIMER_MASTERLOGS);
	getMasterLogsFromServer(request);
	g_wbdMasterLogsTimeout.push(setTimeout(function(){ getMasterLogs(); }, g_wbdTimeOutVal));
}

/* Function to display logs messages. */
function displayLogMessages(message)
{
	var msg = message.replace(/\\n/g, "<br/>");
	var $elem = "<p>" + msg + "</p>";

	$("#stamsgs").empty();
	$("#stamsgs").append($elem);
}

/* Function to clear the master logs. */
function clearMasterLogs()
{
	var request = {"cmd" : "clearlogs"};
	$.ajax({
		type:'GET',
		url:g_urlGetWbdMasterlogs,
		data:request,
		async:true,
		error:function(result) {
			console.log("Failed to clear the logs");
		}
	});
}

/* Function for creating data rows. */
function createDataRows(band, bssid, aparr)
{
	var id = getIdFromBssid(bssid);

	for(var idx = 0; idx < aparr.MACList.length; idx++) {
		var $row = $("<tr>");
		var rssi = getFormatedRSSI(band, aparr.MACList[idx].RSSI, aparr.MACList[idx].Status);

		$("<td>").text(aparr.MACList[idx].MAC).appendTo($row);
		$("<td>").text(rssi).appendTo($row);
		$("#" + id + " > tbody").append($row);

		if(aparr.MACList[idx].Status === "Weak") {
			console.log("weak-sta-entry point 2");
			$row.addClass("weakstacolor");
			var obj = {"MAC":{"data":aparr.MACList[idx].MAC}, "RSSI":{"data":aparr.MACList[idx].RSSI},"Status":{"data":aparr.MACList[idx].Status}};
			$(document).trigger("weak-sta", [ {'BSSID':bssid, 'client':obj} ]);
		}
	}
}

/* Function for generating table elements. */
function tableelem(divId, heading, id)
{
	var $elem = $("<div>", {id:divId, class:"innerdivcommon commonbdr"})

	$("<h3>", {text:heading}).appendTo($elem);
	$("#tableTemplate").clone().attr("id", id).appendTo($elem);
	return $elem;
}

/* Function for creating inner layout */
function createPageInnerLayout(aparr)
{
	var id = getIdFromBssid(aparr.BSSID);
	var heading = aparr.SlaveType + ': ' + aparr.BSSID + ' ' + aparr.MapFlags + ' ' + aparr.Channel;
	var divId = id +"div";

	return tableelem(divId, heading, id);
}

/* Function for creating outer layout */
function createPageOuterLayout(g_wbdarray)
{
	/* 2G blanket layout */
	if (g_wbdarray.Data.TwoG  !== undefined && g_wbdarray.Data.TwoG.length != 0) {
		for(var idx = 0; idx < g_wbdarray.Data.TwoG.length; idx++) {
			$("#2gdiv").append(createPageInnerLayout(g_wbdarray.Data.TwoG[idx]));
			createDataRows(TWO_G, g_wbdarray.Data.TwoG[idx].BSSID, g_wbdarray.Data.TwoG[idx]);
		}
	} else {
		$("#2gdiv").remove();
	}

	/* 5G Low blanket layout */
	if (g_wbdarray.Data.FiveGLow !== undefined && g_wbdarray.Data.FiveGLow.length != 0) {
		for(var idx = 0; idx < g_wbdarray.Data.FiveGLow.length; idx++) {
			$("#5gdiv").append(createPageInnerLayout(g_wbdarray.Data.FiveGLow[idx]));
			createDataRows(FIVE_G, g_wbdarray.Data.FiveGLow[idx].BSSID, g_wbdarray.Data.FiveGLow[idx]);
		}
	} else {
		$("#5gdiv").remove();
	}

	/* 5G High blanket layout */
	if (g_wbdarray.Data.FiveGHigh !== undefined && g_wbdarray.Data.FiveGHigh.length != 0) {
		for(var idx = 0; idx < g_wbdarray.Data.FiveGHigh.length; idx++) {
			$("#5gdivH").append(createPageInnerLayout(g_wbdarray.Data.FiveGHigh[idx]));
			createDataRows(FIVE_G, g_wbdarray.Data.FiveGHigh[idx].BSSID, g_wbdarray.Data.FiveGHigh[idx]);
		}
	} else {
		$("#5gdivH").remove();
	}
}

function updateChannelInfo(g_wbdarray)
{
	var id, txt, arr;

	/* 2G blanket layout */
	if (g_wbdarray.Data.TwoG  !== undefined && g_wbdarray.Data.TwoG.length != 0) {
		for(var idx = 0; idx < g_wbdarray.Data.TwoG.length; idx++) {
			arr = g_wbdarray.Data.TwoG[idx];
			id = getIdFromBssid(arr.BSSID);
			id = id + 'div';
			txt = arr.SlaveType + ': ' + arr.BSSID + ' ' + arr.MapFlags + ' ' + arr.Channel;
			$("#2gdiv > div").map(function(){
				if (this.id === id) {
					var str = $.trim($(this).children("h3").text());
					if (str != txt) {
						$(this).children("h3").text(txt);
					}
				}
			});
		}
	}

	 /* 5G Low blanket layout */
	if (g_wbdarray.Data.FiveGLow !== undefined && g_wbdarray.Data.FiveGLow.length != 0) {
		for(var idx = 0; idx < g_wbdarray.Data.FiveGLow.length; idx++) {
			arr = g_wbdarray.Data.FiveGLow[idx];
			id = getIdFromBssid(arr.BSSID);
			id = id + 'div';
			txt = arr.SlaveType + ': ' + arr.BSSID + ' ' + arr.MapFlags + ' ' + arr.Channel;
			$("#5gdiv > div").map(function(){
				if (this.id === id) {
					var str = $.trim($(this).children("h3").text());
					if (str != txt) {
						$(this).children("h3").text(txt);
					}
				}
			});
		}
	}

	/* 5G High blanket layout */
	if (g_wbdarray.Data.FiveGHigh !== undefined && g_wbdarray.Data.FiveGHigh.length != 0) {
		for(var idx = 0; idx < g_wbdarray.Data.FiveGHigh.length; idx++) {
			arr = g_wbdarray.Data.FiveGHigh[idx];
			id = getIdFromBssid(arr.BSSID);
			id = id + 'div';
			txt = arr.SlaveType + ': ' + arr.BSSID + ' ' + arr.MapFlags + ' ' + arr.Channel;
			$("#5gdivH > div").map(function(){
				if (this.id === id) {
					var str = $.trim($(this).children("h3").text());
					if (str != txt) {
						$(this).children("h3").text(txt);
					}
				}
			});
		}
	}
}

/* Function for returnig the diff of json objects */
var jsonObjDiff = function() {
	return {
		VALUE_CREATED: 'created',
		VALUE_UPDATED: 'updated',
		VALUE_DELETED: 'deleted',
		VALUE_UNCHANGED: 'unchanged',
		map: function(obj1, obj2) {
			if (this.isValue(obj1) || this.isValue(obj2)) {
				return {type: this.compareValues(obj1, obj2), data: obj2 || obj1};
			}
			var diff = {};
			for (var key in obj1) {
				var value2 = undefined;
				if ('undefined' != typeof(obj2[key])) {
				    value2 = obj2[key];
				}

				diff[key] = this.map(obj1[key], value2);
			}
			for (var key in obj2) {
				if ('undefined' != typeof(diff[key])) {
				    continue;
				}

				diff[key] = this.map(undefined, obj2[key]);
			}

			return diff;

		},
		compareValues: function(value1, value2) {
			if (value1 === value2) {
				return this.VALUE_UNCHANGED;
			}
			if ('undefined' == typeof(value1)) {
				return this.VALUE_CREATED;
			}
			if ('undefined' == typeof(value2)) {
				return this.VALUE_DELETED;
			}

			return this.VALUE_UPDATED;
		},
		isArray: function(obj) {
			return {}.toString.apply(obj) === '[object Array]';
		},
		isObject: function(obj) {
			return {}.toString.apply(obj) === '[object Object]';
		},
		isValue: function(obj) {
			return !this.isObject(obj) && !this.isArray(obj);
		}
	}
}();

/* Function for copying nested objects. */
function deepcopy(obj)
{
	var copy = obj, item;

	if (obj && typeof obj === 'object') {
		copy = Object.prototype.toString.call(obj) === '[object Array]' ? [] : {};

		for(item in obj) {
			copy[item] = deepcopy(obj[item]);
		}
	}

	return copy;
}

/* Function for creating data rows. */
function createStaDataRows(band, bssid, aparr)
{
	var id = getIdFromBssid(bssid);

	for(var idx = 0; idx < aparr.length; idx++) {
		var $row = $("<tr>");
		var rssi =  getFormatedRSSI(band, aparr[idx].RSSI, aparr[idx].Status);

		$("<td>").text(aparr[idx].MAC).appendTo($row);
		$("<td>").text(rssi).appendTo($row);
		$("#" + id + " > tbody").append($row);

		if(aparr[idx].Status === "Weak") {
			console.log("weak-sta-entry point 3");
			$row.addClass("weakstacolor");
			var obj = {"MAC":{"data":aparr[idx].MAC}, "RSSI":{"data":aparr[idx].RSSI},"Status":{"data":aparr[idx].Status}};
			$(document).trigger("weak-sta", [ {'BSSID':bssid, 'client':obj} ]);
		}
	}
}

/* Identification of weak sta */
function parseInnerObjects(arr, len)
{
	var id = getIdFromBssid(arr.BSSID.data);
	var band = arr.Band.data;

	for(var i = 0; i < 10; i++) {
		if (arr.MACList[i] !== undefined) {
			/* New sta added */
			if (arr.MACList[i].type == "created") {
				console.log("Mac is created");
				console.log(JSON.stringify(arr));
				var staarr = [];
				staarr.push(arr.MACList[i].data);
				createStaDataRows(band, arr.BSSID.data, staarr);
				$(document).trigger("sta-steered", [ {'BSSID':arr.BSSID.data, 'client':arr.MACList[i].data} ]);
			} else if (arr.MACList[i].type == "deleted") {	/* Sta is deleted */
				console.log("Mac is deleted");
				$("#" + id + " tbody tr").each(function() {
					if( $(this).find("td").eq(0).text() === arr.MACList[i].data.MAC) {
						 $(this).remove();
					}
				});
			} else if( arr.MACList[i].MAC.type === 'updated') {	/* MAC is updated */
				console.log("Mac is updated");
				var rssi = getFormatedRSSI(band, arr.MACList[i].RSSI.data, arr.MACList[i].status.data);
				$("#" + id + " tbody tr").eq(i).find("td").eq(0).html(arr.MACList[i].MAC.data);
				$("#" + id + " tbody tr").eq(i).find("td").eq(1).html(rssi);
			} else {
				/* only rssi or status is updated. */
				if ( (arr.MACList[i].MAC.type !== 'updated') ||
					(arr.MACList[i].RSSI.type === 'updated') ||
					(arr.MACList[i].Status.type === 'updated') ) {
					$("#" + id + " tbody tr").each(function() {
						if( $(this).find("td").eq(0).text() === arr.MACList[i].MAC.data) {
							var rssi = getFormatedRSSI(band, arr.MACList[i].RSSI.data, arr.MACList[i].Status.data);
							$(this).find("td").eq(1).html(rssi);
							if(arr.MACList[i].Status.data === "Weak") {
								$(this).css("color", "red");
								if (arr.MACList[i].Status.type === "updated") {
									console.log("weak-sta-entry point");
									$(document).trigger("weak-sta", [ {'BSSID':arr.BSSID.data, 'client':arr.MACList[i]} ]);
								}
							} else {
								$(this).css("color", "black");
							}
						}
					});
				}
			}
		}
	}
}

/* Weak sta list */
function weakclientlist()
{
	this.list = [];

	this.addobj = function (obj) {
		this.list.push(obj);
		$(document).trigger("weak-sta-msg", [ obj ]);
	}

	this.removeobj = function (obj) {
		for (var i = this.list.length - 1; i >= 0; i--) {
			if (this.list[i].MAC === obj.MAC) {
				this.list.splice(i, 1);
				break;
			}
		}
	}

	this.updatelist = function (obj) {
		var ispresent = false;
		for (var i = this.list.length - 1; i >= 0; i--) {
			if (obj.Status === "Weak") {
				if (this.list[i].MAC === obj.MAC) {
					ispresent = true;
					break;
				}
			} else {
				if (this.list[i].MAC == obj.MAC) {
					$(document).trigger("weak-sta-steer-msg", [ {"from":this.list[i], "to":obj} ]);
					this.list.splice(i, 1);
				}
			}
		}

		if ((ispresent === false) && (obj.Status === "Weak")) {
			this.addobj(obj);
		}
	}
}

/* Weak sta details */
function weakclient(bssid, mac, rssi, status, spanid, timerobj)
{
	this.BSSID = bssid;
	this.MAC = mac;
	this.RSSI = rssi;
	this.Status = status;
	this.spanid = spanid;
	this.timer = timerobj;

}

/* Weak sta steer handler */
$(document).on("sta-steered", function(event, eventdata) {
	console.log("sta steered detected");
	console.log(JSON.stringify(eventdata));
	var num = Math.floor((Math.random()*10000) + 1);
	var spanid = getIdFromBssid(eventdata.BSSID) + num + 'span';

	var weaksta = new weakclient(eventdata.BSSID, eventdata.client.MAC, eventdata.client.RSSI, eventdata.client.Status, spanid, new timerobj().instance());
	g_weakclientlist.updatelist(weaksta);
});

/* Weak sta handler */
$(document).on("weak-sta", function(event, eventdata) {
	console.log("weak sta detected");
	console.log(JSON.stringify(eventdata));
	var num = Math.floor((Math.random()*10000) + 1);
	var spanid = getIdFromBssid(eventdata.BSSID) + num + 'span';

	var weaksta = new weakclient(eventdata.BSSID, eventdata.client.MAC.data, eventdata.client.RSSI.data, eventdata.client.Status.data, spanid, new timerobj().instance());
	g_weakclientlist.updatelist(weaksta);
});

/* Count down timer handling. */
var timerobj = function() {
	this.interval;
}

timerobj.prototype.instance = function() {
	return this;
};

timerobj.prototype.start = function(element, seconds) {
	var timer = seconds;
	this.interval = setInterval(function() {
		element.text(timer);
		if(--timer < 0)
			timer = seconds; }, 1000);
	return this;
};

timerobj.prototype.stop = function() {
	clearInterval(this.interval);
	return this;
};

/* Parse the json diff and update the ui */
function parseJsonDiffAndUpdate(arr, len, tag)
{
	for(var i = 0; i < len; i++) {
		if(arr[i].type === 'created') {
			$(document).trigger("created", [{"tag":tag, "obj":arr[i]}]);
		} else if(arr[i].type === 'deleted') {
			$(document).trigger("deleted", [{"tag":tag, "obj":arr[i]}]);
		} else {
			parseInnerObjects(arr[i], len);
		}
	}
}

function arrangeArr(arr1, arr2)
{
	/* 2G data */
	if (arr1.Data.TwoG  !== undefined && arr1.Data.TwoG.length != 0 &&
		arr2.Data.TwoG !== undefined && arr2.Data.TwoG.length != 0) {
		for(var idx = 0; idx < arr1.Data.TwoG.length; idx++) {
			for(var jdx = 0; jdx < arr2.Data.TwoG.length; jdx++) {
				if (arr1.Data.TwoG[idx].BSSID == arr2.Data.TwoG[jdx].BSSID) {
					var tmp = arr2.Data.TwoG[idx];
					arr2.Data.TwoG[idx] = arr2.Data.TwoG[jdx];
					arr2.Data.TwoG[jdx] = tmp;
				}

			}
		}
	}

	/* 5GL data */
	if (arr1.Data.FiveGLow  !== undefined && arr1.Data.FiveGLow.length != 0 &&
		arr2.Data.FiveGLow !== undefined && arr2.Data.FiveGLow.length != 0) {
		for(var idx = 0; idx < arr1.Data.FiveGLow.length; idx++) {
			for(var jdx = 0; jdx < arr2.Data.FiveGLow.length; jdx++) {
				if (arr1.Data.FiveGLow[idx].BSSID == arr2.Data.FiveGLow[jdx].BSSID) {
					var tmp = arr2.Data.FiveGLow[idx];
					arr2.Data.FiveGLow[idx] = arr2.Data.FiveGLow[jdx];
					arr2.Data.FiveGLow[jdx] = tmp;
				}

			}
		}
	}

	/* 5GH data */
	if (arr1.Data.FiveGHigh  !== undefined && arr1.Data.FiveGHigh.length != 0 &&
		arr2.Data.FiveGHigh !== undefined && arr2.Data.FiveGHigh.length != 0) {
		for(var idx = 0; idx < arr1.Data.FiveGHigh.length; idx++) {
			for(var jdx = 0; jdx < arr2.Data.FiveGHigh.length; jdx++) {
				if (arr1.Data.FiveGHigh[idx].BSSID == arr2.Data.FiveGHigh[jdx].BSSID) {
					var tmp = arr2.Data.FiveGHigh[idx];
					arr2.Data.FiveGHigh[idx] = arr2.Data.FiveGHigh[jdx];
					arr2.Data.FiveGHigh[jdx] = tmp;
				}

			}
		}
	}
}

/* Event handler for creating the page layout */
$(document).on("data-available", function(event, eventdata) {
	if (g_isLayoutRequired == true) {
		createPageOuterLayout(g_wbdarray);
		g_isLayoutRequired = false;
		g_prevwbdarray = deepcopy(g_wbdarray);
	} else {
		var fiveGLlen = g_prevwbdarray.Data["FiveGLow"].length > g_wbdarray.Data["FiveGLow"].length ?
			g_prevwbdarray.Data["FiveGLow"].length : g_wbdarray.Data["FiveGLow"].length;

		var fiveGHlen = g_prevwbdarray.Data["FiveGHigh"].length > g_wbdarray.Data["FiveGHigh"].length ?
			g_prevwbdarray.Data["FiveGHigh"].length : g_wbdarray.Data["FiveGHigh"].length;

		var twoGlen = g_prevwbdarray.Data["TwoG"].length > g_wbdarray.Data["TwoG"].length ?
			g_prevwbdarray.Data["TwoG"].length : g_wbdarray.Data["TwoG"].length;

		updateChannelInfo(g_wbdarray);
		g_wbddiffjson = '';
		arrangeArr(g_prevwbdarray, g_wbdarray);
		g_wbddiffjson = jsonObjDiff.map(g_prevwbdarray, g_wbdarray);
		console.log(JSON.stringify(g_wbddiffjson));
		parseJsonDiffAndUpdate(g_wbddiffjson.Data["FiveGLow"], fiveGLlen, "5GL");
		parseJsonDiffAndUpdate(g_wbddiffjson.Data["FiveGHigh"], fiveGHlen, "5GH");
		parseJsonDiffAndUpdate(g_wbddiffjson.Data["TwoG"], twoGlen, "2G");
		g_prevwbdarray = deepcopy(g_wbdarray);
	}
});

/* Event handler for creating the node in the layout */
$(document).on("created", function(event, eventdata) {
	console.log("created node");
	console.log(JSON.stringify(eventdata));
	var id = getIdFromBssid(eventdata.obj.data.BSSID);
	var heading = eventdata.obj.data.SlaveType + ': ' + eventdata.obj.data.BSSID + ' ' +
		eventdata.obj.data.MapFlags + ' ' + eventdata.obj.data.Channel;
	var divid = id+"div";

	if (eventdata.tag === "5GL") {
		 $("#5gdiv").append(tableelem(divid, heading, id));
		createDataRows(FIVE_G, eventdata.obj.data.BSSID, eventdata.obj.data);
	} else if(eventdata.tag === "5GH") {
		 $("#5gdivH").append(tableelem(divid, heading, id));
		createDataRows(FIVE_G, eventdata.obj.data.BSSID, eventdata.obj.data);
	} else if(eventdata.tag === "2G") {
		 $("#2gdiv").append(tableelem(divid, heading, id));
		createDataRows(TWO_G, eventdata.obj.data.BSSID, eventdata.obj.data);
	}
});

/* Event handler for deleting the node in the layout */
$(document).on("deleted", function(event, eventdata) {
	var id = getIdFromBssid(eventdata.obj.data.BSSID);
	$("#" + id + 'div').remove();
});

/* Function which will get called when DOM for the page gets loaded. */
$(document).ready(function() {
	clearAllWbdTimers(TIMER_MASTERINFO | TIMER_MASTERLOGS);
	initWbdPage();
	getConfigInfo();
	getMasterInfo();
	getMasterLogs();
	/* button click event handler */
	$("#clearlogs").click(function(){
		$("#stamsgs").empty();
		clearMasterLogs();
	});
});
