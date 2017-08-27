/*
 * Jquery implementation for iQoS Settings Page
 *
 * $ Copyright Open Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: iQoSSettings.js, v 1.0 2015-02-12 15:25:43 vbemblek $
 */

/*****
* Function which will get called when DOM for the page gets loaded.START
*****/

var gAppCatPrio = false;
var gDevTypePrio = false;
var gMacAddrPrio = false;

var urlGetDevTypePriority;
var urlSetDevTypePriority;
var urlGetAppCatPriority;
var urlSetAppCatPriority;
var urlGetMacPriority;
var urlSetMacPriority;
var urlGetConfig;
var urlNetworkSummary;
var urlSetDefaultPrio;
var urlSetAllPriority
var isHTTPDServer = 1;

var gDevTypePrioChangedList = []; /* Array to hold the changed dev type prio */
var gAppCatPrioChangedList = []; /* Array to hold the changed app cat prio */

function initPage()
{
	if (isHTTPDServer == 1) {
		urlGetDevTypePriority = 'http://'+ window.location.host +'/iQoS.cgi?Req=GetDevTypePrio';
		urlSetDevTypePriority = 'http://'+ window.location.host +'/iQoS.cgi?Req=SetDevTypePrio';
		urlGetAppCatPriority = 'http://'+ window.location.host +'/iQoS.cgi?Req=GetAppCatPrio';
		urlSetAppCatPriority = 'http://'+ window.location.host +'/iQoS.cgi?Req=SetAppCatPrio';
		urlGetMacPriority = 'http://'+ window.location.host +'/iQoS.cgi?Req=GetMacPrio';
		urlSetMacPriority = 'http://'+ window.location.host +'/iQoS.cgi?Req=SetMacPrio';
		urlSetDefaultPrio = 'http://'+ window.location.host +'/iQoS.cgi?Req=SetDefaultPrio';
		urlSetAllPriority = 'http://'+ window.location.host +'/iQoS.cgi?Req=SetAllPrio';
		urlGetConfig  = 'http://'+ window.location.host +'/iQoS.cgi?Req=GetConfig';
		urlNetworkSummary = 'http://'+ window.location.host +'/iQoSNetworkSummary.asp';
	} else {
		urlGetDevTypePriority = "http://localhost/getdevtypepriority.php";
		urlSetDevTypePriority = 'http://localhost/setconfig.php';
		urlGetAppCatPriority = "http://localhost/getappcatpriority.php";
		urlSetAppCatPriority = 'http://localhost/setconfig.php';
		urlGetMacPriority = 'http://localhost/getmacpriority.php';
		urlSetMacPriority = 'http://localhost/setconfig.php';
	}
}

/*****
* Function To Check iQoS State.START
*****/
function CheckiQoSState(array)
{
	if (array.EnableiQoS == 0) {
		alert('Please Enable iQoS In Quick Settings' + '\n' + 'To Access Settings');
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
		success:function(result) {
			if (isHTTPDServer == 1)
				array = result;
			else
				array = JSON.parse(result);

			CheckiQoSState(array);
		}
	});
}

/*****
* Function For Appending to the DeviceTyepPriority.START
*****/
function AppendDeviceTypePriority(array)
{
	var arrayDevTypePrio = array.DevTypePrio;
	var previd = 'sortableulid';
	var deviceImage;

	/* Clear all the device type priority div's for all the priority */
	for (var j = 0; j < 8; j++) {
		$("#sortableulid"+j).html('');
	}

	/* Add the div's for corresponding priority */
	for (var i = 0; i < arrayDevTypePrio.length; i++) {
		var element = '';

		deviceImage = GetIcon("devtype", arrayDevTypePrio[i].DevTypeInt);
		element = '<div id = "divdeviceid" class="container" value="'+arrayDevTypePrio[i].DevTypeInt+'">';
		element += '<img src="' + deviceImage + '" width="50%" + onerror="ManageMissingIcon(this);"/><br>'+arrayDevTypePrio[i].DevType+'</br></div>';
		$("#"+previd+(arrayDevTypePrio[i].Priority)).append(element);
	}
}
/*****
* Function For Appending to the DeviceTyepPriority.END
*****/

/*****
* Function For getting the DeviceTypePriority.START
*****/
function getDeviceTypePriority()
{
	var array = [];

	$.ajax({
		type:"GET",
		url:urlGetDevTypePriority,
		success:function(result){
			if (isHTTPDServer == 1)
				array = result;
			else
				array = JSON.parse(result);
			AppendDeviceTypePriority(array);
			//alert(result);
		}
	});
}
/*****
* Function For getting the DeviceTypePriority.END
*****/

/*****
* Function For Setting to the DeviceTyepPriority.START
*****/
function setDeviceTypePriority()
{
	var devprio;
	var jsonObj = {};
	var parentid = '';

	jsonObj ["Req"] = 'SetDevTypePrio';
	/* For every my device priority ul */
	$('.priorityclass').each(function() {
		parentid = $(this);
		/* For every div's inside ul */
		$(this).children().each(function() {
			devprio = 'DevTypePrio'+ $(this).attr('value');
			jsonObj[devprio] = parentid.attr('Priority');
		});
	});

	//alert(JSON.stringify(jsonObj));
	var request = $.ajax({
		url: urlSetDevTypePriority,
		type: "POST",
		data: JSON.stringify(jsonObj),
		dataType: "json",
		success: function(data) { gDevTypePrio = true; }
	});
	return request;
}
/*****
* Function For Setting to the DeviceTyepPriority.END
*****/
/*****
* Function For Appending the AppCatPriority.START
*****/
function AppendAppCategoryPriority(array)
{
	var arrayAppCat = array.AppCatPrio;
	var previd = 'sortableulappid';
	var catImage;

	/* Clear all the device type priority div's for all the priority */
	for (var j = 0; j < 8; j++) {
		$("#sortableulappid"+j).html('');
	}

	/* Add the div's for corresponding priority */
	for (var i = 0; i < arrayAppCat.length; i++) {
		var element = '';
		/* Category Id 18, 19 and 20 represent the n/w protocols so display only 18 */
		if (arrayAppCat[i].CategoryInt == 19 || arrayAppCat[i].CategoryInt == 20) {
			continue;
		}
		catImage = GetIcon("appcat", arrayAppCat[i].CategoryInt);
		element = '<div id = "divappcatid" class="container" value="'+arrayAppCat[i].CategoryInt+'">';
		element += '<img src="' + catImage + '" width="50%" + onerror="ManageMissingIcon(this);"/><br>'+arrayAppCat[i].Category+'</br></div>';
		$("#"+previd+(arrayAppCat[i].Priority)).append(element);
	}
}
/*****
* Function For Appending the AppCatPriority.END
*****/
/*****
* Function For getting the AppCatPriority.START
*****/
function getAppCatPriority()
{
	var array = [];

	$.ajax({
		type:"GET",
		url:urlGetAppCatPriority,
		success:function(result){
			if (isHTTPDServer == 1)
				array = result;
			else
				array = JSON.parse(result);
			AppendAppCategoryPriority(array);
			//alert(result);
		}
	});
}
/*****
* Function For getting the AppCatPriority.END
*****/

/*****
* Function For Setting the AppCatPriority.START
*****/
function setAppCatPriority()
{
	var appcatprio;
	var jsonObj = {};
	var parentid = '';

	jsonObj ["Req"] = 'SetAppCatPrio';
	/* For every my device priority ul */
	$('.priorityappclass').each(function() {
		parentid = $(this);
		/* For every div's inside ul */
		$(this).children().each(function() {
			appcatprio = 'AppCatPrio'+ $(this).attr('value');
			jsonObj[appcatprio] = parentid.attr('Priority');
		});
	});
	//alert(JSON.stringify(jsonObj));

	var request = $.ajax({
		url: urlSetAppCatPriority,
		type: "POST",
		data: JSON.stringify(jsonObj),
		dataType: "json",
		success: function(data) { gAppCatPrio = true; }
	});
	return request;
}
/*****
* Function For Setting the AppCatPriority.END
*****/

/*****
* Function For Appending the MacPriority.START
*****/
function AppendMacAddress(array)
{
	var arrayMacPrio = array.MacPrio;
	var previd = 'sortableulmacid';

	/* Clear all the my device priority div's for all the priority */
	for (var j = 0; j < 8; j++) {
		$("#sortableulmacid"+j).html('');
	}

	/* Add the div's for corresponding priority */
	for (var i = 0; i < arrayMacPrio.length; i++) {
		var element = '';
		var hostID = '';
		var deviceImage;
		var devName = arrayMacPrio[i].DevName;

		if(arrayMacPrio[i].hasOwnProperty("Name") && arrayMacPrio[i].Name != ""){
			hostID = arrayMacPrio[i].Name;
		}
		else if(arrayMacPrio[i].hasOwnProperty("IPAddr") && arrayMacPrio[i].IPAddr != ""){
			hostID = arrayMacPrio[i].IPAddr;
		}
		else{
			hostID = arrayMacPrio[i].MacAddr;
		}

		if (devName != undefined)
				hostID += '-' + devName;

		deviceImage = GetIcon("devtype", arrayMacPrio[i].DevTypeInt);
		element = '<div id = "divmacid" class="container" value="'+arrayMacPrio[i].MacAddr+'">';
		element += '<img src="' + deviceImage + '" width="50%" + onerror="ManageMissingIcon(this);"/><br>'+hostID+'</br></div>';
		$("#"+previd+(arrayMacPrio[i].Priority)).append(element);
	}
}
/*****
* Function For Appending the MacPriority.END
*****/
/*****
* Function For getting the MacPriority.START
*****/
function getMacPriority()
{
	var array = [];
	$.ajax({
		type:"GET",
		url:urlGetMacPriority,
		success:function(result){
			if (isHTTPDServer == 1)
				array = result;
			else
				array = JSON.parse(result);
			//alert(result);
			AppendMacAddress(array);
		}
	});
}
/*****
* Function For getting the MacPriority.END
*****/
/*****
* Function For Setting the MacPriority.START
*****/
function setMacPriority()
{
	var macprio;
	var macaddrprio;
	var jsonObj = {};
	var index = 0;
	var parentid = '';

	jsonObj ["Req"] = 'SetMacPrio';
	/* For every my device priority ul */
	$('.prioritymacclass').each(function() {
		parentid = $(this);
		/* For every div's inside ul */
		$(this).children().each(function() {
			macprio = 'MacAddr'+ index;
			jsonObj[macprio] = $(this).attr('value');
			macaddrprio = 'MacAddrPrio'+ index;
			jsonObj[macaddrprio] = parentid.attr('Priority');
			index++;
		});
	});
	//alert(JSON.stringify(jsonObj));
	var request = $.ajax({
		url: urlSetMacPriority,
		type: "POST",
		data: JSON.stringify(jsonObj),
		dataType: "json",
		success: function(data) { gMacAddrPrio = true; }
	});
	return request;
}
/*****
* Function For Setting the MacPriority.END
*****/

/* Function to submit the settings page */
function setAllPriority()
{
	var macprio, macaddrprio, index = 0, parentid = '';
	var jsonObj = {};
	jsonObj ["Req"] = 'SetAllPrio';

	/* For every changed device priority */
	for(index = 0; index < gDevTypePrioChangedList.length; index++) {
		jsonObj[gDevTypePrioChangedList[index].name] = gDevTypePrioChangedList[index].prio;
	}

	/* For every changed application priority */
	for(index = 0; index < gAppCatPrioChangedList.length; index++) {
		jsonObj[gAppCatPrioChangedList[index].name] = gAppCatPrioChangedList[index].prio;
	}

	/* For every my device priority ul */
	index = 0;
	$('.prioritymacclass').each(function() {
		parentid = $(this);
		/* For every div's inside ul */
		$(this).children().each(function() {
			macprio = 'MacAddr'+ index;
			jsonObj[macprio] = $(this).attr('value');
			macaddrprio = 'MacAddrPrio'+ index;
			jsonObj[macaddrprio] = parentid.attr('Priority');
			index++;
		});
	});

	var request = $.ajax({
		url: urlSetAllPriority,
		type: "POST",
		data: JSON.stringify(jsonObj),
		dataType: "json",
		success: function(data) { }
	});

	return request;
}

/* Helper function to clear modified array list */
function resetPrioList()
{
	/* clear array contents */
	gDevTypePrioChangedList = [];
	gAppCatPrioChangedList = [];
}

/* Helper function to update modified array list */
function updateList(list, obj)
{
	var idx;
	var newentry = true;

	for(idx = 0; idx < list.length; idx++) {
		/* if entry is already in array just update the array entry*/
		if (list[idx].name == obj.name) {
			list[idx].prio = obj.prio;
			newentry = false;
			break;
		}
	}
	if (newentry == true) {
		list.push(obj);
	}
}

/* Function for keepking track of modified items */
function updatePrioChangedList(tag, obj)
{
	var idx;

	switch(tag) {
		case "DevTypePrio":
			updateList(gDevTypePrioChangedList, obj);
			break;
		case "AppCatPrio":
			updateList(gAppCatPrioChangedList, obj);
			break;
		default:
			break;
	}
}

/*****
* Function For Drag and Drop of the DeviceType Priority.START
*****/
function DeviceTypePriority()
{
	$( ".priorityclass" ).sortable({

		start: function( event, ui ) {
		},
		stop: function( event, ui ) {
			ui.item.remove();
			var previd = 'sortableulid0';
			var prevtop = 0;
			var prevPrio = 0;
			var isdropped = false;
			var changedObj;

			$('.priorityclass').each(function( index ) {
				if (ui.offset.top > prevtop && ui.offset.top < $(this).offset().top) {
					$("#"+previd).append('<div id = "divdeviceid" class="container" value="'+ui.item.attr('value')+'">'+ui.item.html()+'</div>');
					changedObj = { name : "DevTypePrio" + ui.item.attr('value'), prio : prevPrio };
					updatePrioChangedList("DevTypePrio", changedObj);
					isdropped = true;
				} else {
					;
				}
				previd = $(this).attr('id');
				prevtop = $(this).offset().top;
				prevPrio = $(this).attr('Priority');
			});

			/* Dropped item in last prio group */
			if (isdropped == false) {
				$("#"+previd).append('<div id = "divdeviceid" class="container" value="'+ui.item.attr('value')+'">'+ui.item.html()+'</div>');
				changedObj = { name : "DevTypePrio" + ui.item.attr('value'), prio : prevPrio };
				updatePrioChangedList("DevTypePrio", changedObj);
			}
		}
    });
    $( "ul, div" ).disableSelection();
}
/*****
* Function For Drag and Drop of the DeviceType Priority.END
*****/

/* Function to update all three n/w protocols categories */
function updateProtoPrioObj(tag, arr, changedprio)
{
	var obj;

	for (var idx = 0; idx < arr.length; idx++) {
		obj = { name : tag + arr[idx], prio : changedprio };
		updatePrioChangedList(tag, obj);
	}
}
/*****
* Function For Drag and Drop of the AppCategory Priority.START
* If N/W protocol app category id 18 changes update the 19 and 20 as well.
*****/
function AppCategoryPriority()
{
	$( ".priorityappclass" ).sortable({

		start: function( event, ui ) {
		},
		stop: function( event, ui ) {
			ui.item.remove();
			var previd = 'sortableulappid0';
			var prevtop = 0;
			var prevPrio = 0;
			var isdropped = false;
			var changedObj;
			var protoArray = ["18","19","20"];

			$('.priorityappclass').each(function( index ) {
				if (ui.offset.top > prevtop && ui.offset.top < $(this).offset().top) {
					$("#"+previd).append('<div id = "divappcatid" class="container" value="'+ui.item.attr('value')+'">'+ui.item.html()+'</div>');
					if (ui.item.attr('value') == "18") {
						updateProtoPrioObj("AppCatPrio", protoArray, prevPrio);
					} else {
						changedObj = { name : "AppCatPrio" + ui.item.attr('value'), prio : prevPrio };
						updatePrioChangedList("AppCatPrio", changedObj);
					}
					isdropped = true;
				} else {
					;
				}
				previd = $(this).attr('id');
				prevtop = $(this).offset().top;
				prevPrio = $(this).attr('Priority');
			});

			/* Dropped item in last prio group */
			if (isdropped == false) {
				$("#"+previd).append('<div id = "divappcatid" class="container" value="'+ui.item.attr('value')+'">'+ui.item.html()+'</div>');
				if (ui.item.attr('value') == "18") {
					updateProtoPrioObj("AppCatPrio", protoArray, prevPrio);
				} else {
					changedObj = { name : "AppCatPrio" + ui.item.attr('value'), prio : prevPrio };
					updatePrioChangedList("AppCatPrio", changedObj);
				}
			}
		}
	});
	$( "ul, div" ).disableSelection();
}
/*****
* Function For Drag and Drop of the AppCategory Priority.END
*****/
/*****
* Function For Drag and Drop of the MacBased Priority.START
*****/
function MacPriority()
{
	$( ".prioritymacclass" ).sortable({

		start: function( event, ui ) {
		},
		stop: function( event, ui ) {
			ui.item.remove();
			var previd = 'sortableulmacid0';
			var prevtop = 0;
			var isdropped = false;
			$('.prioritymacclass').each(function( index ) {
				if (ui.offset.top > prevtop && ui.offset.top < $(this).offset().top) {
					$("#"+previd).append('<div id = "divmacid" class="container" value="'+ui.item.attr('value')+'">'+ui.item.html()+'</div>');
					isdropped = true;
				} else {
					;
				}
				previd = $(this).attr('id');
				prevtop = $(this).offset().top;
			});
			if (isdropped == false)
				$("#"+previd).append('<div id = "divmacid" class="container" value="'+ui.item.attr('value')+'">'+ui.item.html()+'</div>');
		}
	});
	$( "ul, div" ).disableSelection();
}
/*****
* Function For Drag and Drop of the MacBased Priority.END
*****/

/* Function to restore all the priorities to defaults. */
function restorePrioToDefaults()
{
	var jsonObj = {};
	jsonObj ["Req"] = 'SetDefaultPrio';
	$.ajax({
		url:urlSetDefaultPrio,
		type:"POST",
		data: JSON.stringify(jsonObj),
		dataType: "json",
		success:function(res){
			/* hide the progress bar */
			$(".cmnprogressbar").hideProgressBar();
			alert("Settings restored to defaults.");
			loadSettingsPage();
			$("#restorebutton").prop('disabled',false);
		},
		error:function(XMLHttpRequest, textStatus, errorThrown){
			/* hide the progress bar */
			$(".cmnprogressbar").hideProgressBar();
			alert("Failed to restore settings to default.");
			$("#restorebutton").prop('disabled',false);
		}
	});
}


/* Helper Function lo load the settings page */
function loadSettingsPage()
{
	getAppCatPriority();
	getDeviceTypePriority();
	getMacPriority();
}

/*****
* Function which will get called when DOM for the page gets loaded.START
*****/
$(document).ready(function()
{
	initPage();

	getConfiguration();
	loadSettingsPage();
	DeviceTypePriority();
	AppCategoryPriority();
	MacPriority();

	$( "#cancelbutton" ).click(function() {
		loadSettingsPage();
		resetPrioList();
	});

	$( "#submitbutton" ).click(function() {
		/* Disable the button till all the setting are stored.Once done/fail show the appropriate msg and enable it again */
		$("#submitbutton").prop('disabled',true);
		/* Show the progress bar */
		$(".cmnprogressbar").showProgressBar();
		$.when(setAllPriority() )
		.done(function() {
			/* hide the progress bar */
			$(".cmnprogressbar").hideProgressBar();
			alert("Setting saved");
			resetPrioList();
			$("#submitbutton").prop('disabled',false);
		})
		.fail(function(){
			/* hide the progress bar */
			$(".cmnprogressbar").hideProgressBar();
			alert("Failed to store settings");
			resetPrioList();
			$("#submitbutton").prop('disabled',false);
		});
	});

	$("#restorebutton").click(function(){
		/* Disable the button till all the setting are stored.Once done/fail show the appropriate msg and enable it again */
		$("#restorebutton").prop('disabled',true);
		/* Show the progress bar */
		$(".cmnprogressbar").showProgressBar();
		restorePrioToDefaults();
	});

});
