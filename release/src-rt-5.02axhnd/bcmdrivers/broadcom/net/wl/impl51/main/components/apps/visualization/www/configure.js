/*
 * Jquery implementation for Configure Tab
 *
 * Copyright (C) 2019, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: configure.js 682079 2017-01-31 10:30:10Z $
 */

var counterinfo = [];
var channelcounter = [];
var channelcapacitygraphs = [];
var matricstabperstacounters = [];
var timeramount = [];
var isstartedflag = [];
var urlremotesettings = '';
var urlconfigsettings = '';
var urlsetconfigsettings = '';
var isHTTPDWebserver = 1;

var gIsBCMInternal = 0; /* Is Internal build */
/* To store the ID of time entry text box to place the selected time */
var gTimeSelectCtrlId = '';
/* Enumeration variable to hold the weekdays flag */
var gWeekDays = {
	Sunday: 1 << 0,
	Monday: 1 << 1,
	Tuesday: 1 << 2,
	Wednesday: 1 << 3,
	Thursday: 1 << 4,
	Friday: 1 << 5,
	Saturday: 1 << 6
}

function initPage()
{
	initCommonPage();

	if (isHTTPDWebserver == 1) {
		urlremotesettings = window.location.protocol + '//' + window.location.host +'/json.cgi';
		urlconfigsettings = window.location.protocol + '//' + window.location.host +'/json.cgi'
		urlsetconfigsettings = window.location.protocol + '//' + window.location.host +'/json.cgi';
	} else {
		urlremotesettings = 'http://localhost/remotesettings.php';
		urlconfigsettings = 'http://localhost/counterdata.php';
		urlsetconfigsettings = 'http://localhost/configuration.php';
	}

	/* Disable auto start by default */
	$(".autostartdivclass").children().prop('disabled',true);
	/* Hide the time select control */
	$('.timeselectdiv').css('display', 'none');
}

/* Get Over write DB or stop data collection radio button value */
function getOverWriteDBRadioValue()
{
	/* Get Over write DB or stop data collection radio button value */
	if ($("#overwritedataid").is(":checked") == 1)
		return 1;

	return 0;
}

/* Set over write DB or stop data collection radio button value */
function setOverWriteDBRadioValue(isoverwritedb)
{
	/* Based on backend value select Over write DB or stop data collection radio button */
	if (isoverwritedb == 1) {
		$("#overwritedataid").prop("checked", true);
	} else {
		$("#stopdcollid").prop("checked", true);
	}
}

/* Get the OR values of all the week days selected */
function getDaysSelected()
{
	var allvalues = 0;

	/* OR the values of all the weekdays checked */
	$.each($("#weekdaycheckboxid:checked"),function() {
		switch($(this).val()) {
			case 'Sunday':
				allvalues |= gWeekDays.Sunday;
				break;
			case 'Monday':
				allvalues |= gWeekDays.Monday;
				break;
			case 'Tuesday':
				allvalues |= gWeekDays.Tuesday;
				break;
			case 'Wednesday':
				allvalues |= gWeekDays.Wednesday;
				break;
			case 'Thursday':
				allvalues |= gWeekDays.Thursday;
				break;
			case 'Friday':
				allvalues |= gWeekDays.Friday;
				break;
			case 'Saturday':
				allvalues |= gWeekDays.Saturday;
				break;
		}
	});

	return allvalues;
}

/* To check or uncheck the check box based on the value. Greater than 0 check or else uncheck */
function checkUncheckOneWeekday(ctrlId, value)
{
	if (value > 0)
		$(ctrlId).prop('checked', true);
	else
		$(ctrlId).prop('checked', false);
}

/* To check or uncheck the weekdays based on the OR values */
function selectWeekDaysCheckBox(value)
{
	$(".autostartdivclass #weekdaycheckboxid").each(function() {
		switch($(this).val()) {
			case 'Sunday':
				checkUncheckOneWeekday($(this), (value & gWeekDays.Sunday));
				break;
			case 'Monday':
				checkUncheckOneWeekday($(this), (value & gWeekDays.Monday));
				break;
			case 'Tuesday':
				checkUncheckOneWeekday($(this), (value & gWeekDays.Tuesday));
				break;
			case 'Wednesday':
				checkUncheckOneWeekday($(this), (value & gWeekDays.Wednesday));
				break;
			case 'Thursday':
				checkUncheckOneWeekday($(this), (value & gWeekDays.Thursday));
				break;
			case 'Friday':
				checkUncheckOneWeekday($(this), (value & gWeekDays.Friday));
				break;
			case 'Saturday':
				checkUncheckOneWeekday($(this), (value & gWeekDays.Saturday));
				break;
		}
	});
}

/* Get Time in 09:50 AM format from number of seconds elapsed from 00 hour*/
function getStringFromTime(tmSeconds)
{
	var fulltime = '';
	var isAM = 1;
	var tmpval = 0;
	var hour = 0;
	var minutes = 0;

	/* Get hours and Minutes from number of seconds elapsed from 00 hour */
	tmpval = Math.floor(parseInt(tmSeconds, 10)/60); /* Get number of minutes */
	hour = Math.floor(tmpval/60); /* Get hours from number of minutes */
	minutes = Math.floor(tmpval%60); /* Get remaining minutes after getting hours */

	/* In 24 hour format the 0 hour means 12 AM. SO make hour to 12 */
	if (hour == 0)
		hour = 12;
	/* If hour is greater than 12 it is PM and substract it by 12 to make it to 12 hour format */
	if (hour > 12) {
		isAM = 0;
		hour -= 12;
	}
	fulltime = ("0" + hour).slice(-2);
	fulltime += ':';
	fulltime += ("0" + minutes).slice(-2);
	fulltime += (isAM == 1)? ' AM' : ' PM';

	return fulltime;
}

function getHourFromString(tmString)
{
	var tmparr = [];
	var tmparr1 = [];

	/* Set some default time */
	if (tmString.length <= 0)
		return '00';

	/* Split the array into hour and minute */
	tmparr = tmString.split(':');

	return tmparr[0];
}

function getMinutesFromString(tmString)
{
	var tmparr = [];
	var tmparr1 = [];

	/* Set some default time */
	if (tmString.length <= 0)
		return '00';

	/* Split the array into hour and minute */
	tmparr = tmString.split(':');

	/* Split the minute portion to minute and AM or PM */
	tmparr1 = tmparr[1].split(' ');

	return tmparr1[0];
}

function getAMPMFromString(tmString)
{
	var tmparr = [];
	var tmparr1 = [];

	/* Set some default time */
	if (tmString.length <= 0)
		return 'AM';

	/* Split the array into hour and minute */
	tmparr = tmString.split(':');

	/* Split the minute portion to minute and AM or PM */
	tmparr1 = tmparr[1].split(' ');

	return tmparr1[1];
}

/* Set From and To Time in text box */
function setFromAndToTime(fromTm, toTm)
{
	if (fromTm > 0)
		$("#fromtimeentry").val(getStringFromTime(fromTm));
	else
		$("#fromtimeentry").val('12:00 AM');
	if (toTm > 0)
		$("#totimeentry").val(getStringFromTime(toTm));
	else
		$("#totimeentry").val('12:00 AM');
}

/* Get Number of seconds elapsed since 01 January, 1970 00:00:00 UTC */
function getTimeFromString(fromTm)
{
	var date;
	var hour = parseInt(getHourFromString(fromTm), 10);
	var minute = parseInt(getMinutesFromString(fromTm), 10);
	var amflag = getAMPMFromString(fromTm) == 'AM'?1:0;
	var numOfSeconds = 0;

	if (hour == 0)
		return 0;
	/* To convert to 24 hour format make 12 to 0 */
	if (hour == 12)
		hour = 0;
	if (amflag == 0) /* if it is PM add 12 to convert it to 24 hour format */
		hour += 12;

	/* Get the number of seconds elapsed from 12 AM (in 24 hour format 00 hour)
	 * For this convert hour and minute to seconds and add it
	 * 1 hour = 3600 seconds and 1 minute = 60 seconds
	 */
	numOfSeconds = ((hour*3600)+(minute*60));

	return numOfSeconds;
}

/* Toggles Start/Stop Data collection button */
function toggleStartStopDataCollectionBtn()
{
	var elem = document.getElementById("startbutton");
	if (elem.value == "Start") { /* If it is start change it to stop */
		elem.value = "Stop";
		elem.textContent = "Stop Data Collection";
	} else {
		elem.value = "Start";
		elem.textContent = "Start Data Collection";
	}
}

/* Sends SetConfig request to webserver */
function onSetConfig()
{
	var timeIntervl = $(".smplinterval:checked").val();
	var dbsize = $("#dbsizeinmb").val();
	var startstop = 0;
	var sampletypearr = [];
	var isoverwritedata = 0;
	var fromTm = 0;
	var toTm = 0;
	var isAutoStart = 0;
	var selectedDays = 0;
	var elem;

	timeramount = [];
	isstartedflag = [];
	timeramount.push(timeIntervl*1000);

	elem = document.getElementById("startbutton");
	if (elem.value == "Start"){
		startstop = 0;
		isstartedflag.push(0);
	} else {
		startstop = 1;
		isstartedflag.push(1);
	}

	$.each($(".0:checked"),function(){
		sampletypearr.push($(this).val());
	});

	$.each($(".1:checked"),function() {
		sampletypearr.push($(this).val());
	});

	/* Get Over write DB or stop data collection radio button value */
	isoverwritedata = getOverWriteDBRadioValue();

	/* Get All auto start details */
	isAutoStart = $("#autostartid").prop('checked') == true?1:0;
	selectedDays = getDaysSelected();
	if (isAutoStart == 1 && selectedDays == 0) {
		alert("Some day should be selected when auto start is enabled");
		return;
	}
	fromTm = getTimeFromString($("#fromtimeentry").val());
	if (isAutoStart == 1 && fromTm < 0) {
		alert("Please provide From time");
		return;
	}
	toTm = getTimeFromString($("#totimeentry").val());
	if (isAutoStart == 1 && toTm < 0) {
		alert("Please provide To time");
		return;
	}

	$.cookie('matricscntr',escape(counterinfo.join(',')));
	$.cookie('channelscntr',escape(channelcounter.join(',')));
	$.cookie('channelcapacitygrph',escape(channelcapacitygraphs.join(',')));
	$.cookie('timerinterval',escape(timeramount.join(',')));
	$.cookie('matricscntrpsta',escape(matricstabperstacounters.join(',')));
	$.cookie('isstarted',escape(isstartedflag.join(',')));
	$.ajax({
		type:"GET",
		url:urlsetconfigsettings,
		data:{Req:'SetConfig',Interval:timeIntervl,DBSize:dbsize,StartStop:startstop,
			Total:sampletypearr.length,graphname:sampletypearr,OverWriteDB:isoverwritedata,
			AutoStart:isAutoStart,WeekDays:selectedDays,FromTm:fromTm,ToTm:toTm},
		success:function(result){
		}
	});
}

/* On click start or stop data collection button */
$(function(){
	$("#startbutton").click(function() {
		toggleStartStopDataCollectionBtn();
		onSetConfig();
	});
});

/* On click submit button */
$(function() {
	$("#submitbutton").click(function() {
		onSetConfig();
	});
});

function retrieveRemoteDebug()
{
	var array = [];
	$.ajax({
		type:'GET',
		url:urlremotesettings,
		data:{Req:'RemoteSettings'},
		success:function(result){
			if (isHTTPDWebserver == 1)
				array = result;
			else
				array = JSON.parse(result);
			if (array.IsEnabled == 1) {
				$('.remoteipdiv').show();
				$('#remoteenablechkid').prop('checked', true);
				$('#remoteipid').val(array.ServerIP);
			} else {
				$('.remoteipdiv').hide();
				$('#remoteenablechkid').prop('checked', false);
			}
			var remoteenabledflag = [];
			var remotedconip = [];
			remoteenabledflag.push(array.IsEnabled);
			remotedconip.push(array.ServerIP);
			$.cookie('remoteenabledflag',escape(remoteenabledflag.join(',')));
			$.cookie('remotedconip',escape(remotedconip.join(',')));
		}
	});
}

$(function() {
	var array = [];

	initPage();

	/* Get the multi CPU architecture support NVRAM value */
	var tmpmcpu = getNVRAMValue('vis_m_cpu');
	var tmpmcpuIntVal = parseInt(tmpmcpu, 10);
	if (tmpmcpuIntVal == 0) { /* If not supported hide it */
		$('#idmcpusupport').css('display', 'none');
	}

	retrieveRemoteDebug();

	$.ajax({
		type:'GET',
		url:urlconfigsettings,
		data:{Req:'ConfigSettings'},
		success:function(result) {
			if (isHTTPDWebserver == 1)
				array = result;
			else
				array = JSON.parse(result);
			drawTheCounters(array);
			fillTheCookies(array);
		}
	});
});

function drawTheCounters(array)
{
	gIsBCMInternal = array[0][0].IsInternal;

	$('.smplinterval').each(function() {
		if ($(this).val() == array[0][0].Interval) {
			$(this).prop("checked","checked");
		}
	})
	showDBSizeAndIntervalMsg();

	if (array[0][0].StartStop == 0) {
		var elem = document.getElementById("startbutton");
		elem.value = "Start";
		elem.textContent = "Start Data Collection";
	} else {
		var elem = document.getElementById("startbutton");
		elem.value = "Stop";
		elem.textContent = "Stop Data Collection";
	}

	$('#dbsizeinmb').val(array[0][0].DBSize);

	var ip = $(".remoteip").val();
	if (ip.length <= 0) {
		if (array[0][0].GatewayIP.length < 4)
			$('#remoteipid').val('127.0.0.1');
		else
			$('#remoteipid').val(array[0][0].GatewayIP);
	}

	/* Based on backend value select Over write DB or stop data collection radio button */
	setOverWriteDBRadioValue(array[0][0].isoverwritedb);

	/* Check or uncheck 'start collecting data every' check box */
	if (array[0][0].AutoStart == '1') {
		$("#autostartid").prop('checked', true);
		$(".autostartdivclass").children().prop('disabled',false);
	}
	else {
		$("#autostartid").prop('checked', false);
		$(".autostartdivclass").children().prop('disabled',true);
	}
	selectWeekDaysCheckBox(array[0][0].WeekDays);
	setFromAndToTime(array[0][0].FromTm, array[0][0].ToTm);

	for (i = 0; i < array[1].length; i++) {
		var $element;
		if (array[1][i].Enable == 0) {
			$element = '<input type="checkbox" name="counterelems" data-tab="' + array[1][i].Tab + '" class="'+ array[1][i].PerSTA +' cmncheckboxclass" value="'+ array[1][i].Name +'">';
		} else if(array[1][i].Enable == 1) {
			$element = '<input type="checkbox" name="counterelems" data-tab="' + array[1][i].Tab + '"  class="'+ array[1][i].PerSTA +' cmncheckboxclass" checked="checked" value="'+array[1][i].Name+'">';
		}
		$element = $element + array[1][i].Name + '</input></br>';
		if (i <= (array[1].length/2-1)) {
			$("#fst").append($element);
		} else {
			$("#scnd").append($element);
		}
	}
}

function fillTheCookies(array)
{
	var intervalarr = [];
	var tmpisstartflag = [];

	intervalarr.push(array[0][0].Interval*1000);
	tmpisstartflag.push(array[0][0].StartStop);

	for (i = 0; i < array[1].length; i++) {
		if (array[1][i].Tab == 'Metrics' && array[1][i].PerSTA == 0 && array[1][i].Enable == 1) {
			counterinfo.push(array[1][i].Name);
		}
		if (array[1][i].Tab == 'Metrics' && array[1][i].PerSTA == 1 && array[1][i].Enable == 1) {
			matricstabperstacounters.push(array[1][i].Name);
		}
		if (array[1][i].Tab == 'Channels' && array[1][i].Enable == 1) {
			channelcounter.push(array[1][i].Name);
		}
		if (array[1][i].Tab == 'Channel Statistics' && array[1][i].Enable == 1) {
			channelcapacitygraphs.push(array[1][i].Name);
		}
	}

	$.cookie('matricscntr',escape(counterinfo.join(',')));
	$.cookie('matricscntrpsta',escape(matricstabperstacounters.join(',')));
	$.cookie('channelscntr',escape(channelcounter.join(',')));
	$.cookie('timerinterval',escape(intervalarr.join(',')));
	$.cookie('channelcapacitygrph',escape(channelcapacitygraphs.join(',')));
	$.cookie('isstarted',escape(tmpisstartflag.join(',')));
}

$(function() {
	$(document).on("click","input.0",function(e) {
		counterinfo = [];
		channelcounter = [];
		channelcapacitygraphs = [];
		$(".0:checked").each(function(){
			if ($(this).data('tab') == 'Metrics')
				counterinfo.push($(this).val());
			else if ($(this).data('tab') == 'Channels')
				channelcounter.push($(this).val());
			else if ($(this).data('tab') == 'Channel Capacity')
				channelcapacitygraphs.push($(this).val());
		});
	});

	$(document).on("click","input.1",function(e){
		matricstabperstacounters = [];
		channelcounter = [];
		channelcapacitygraphs = [];
		$(".1:checked").each(function() {
			if ($(this).data('tab') == 'Metrics')
				matricstabperstacounters.push($(this).val());
			else if ($(this).data('tab') == 'Channels')
				channelcounter.push($(this).val());
			else if ($(this).data('tab') == 'Channel Capacity')
				channelcapacitygraphs.push($(this).val());
		});
	});

	$(document).on("click","input.smplinterval",function(e) {
		timeramount = [];
		timeramount.push($(this).val() * 1000);
	});

	$('#remoteenablechkid').click(function(event) {
		if ($(this).is(':checked')) {
			$('.remoteipdiv').show();
		} else {
			$('.remoteipdiv').hide();
		}
	});

	$('.remoteipdiv').hide();
	$(".progressbar").css('display','none');
});

function validateIP(ip)
{
	var ret = 1, i;
	if (ip.length > 15)
		return 0;
	var fields = ip.split('.');
	if (fields.length != 4)
		return 0;
	for (i = 0; i < 4; i++) {
		if (fields[i].length < 1)
			return 0;
		if (fields[i].length > 3)
			return 0;
		if (fields[i] > '255')
			return 0;
	}
	for (i = 0; i < ip.length; i++) {
		if (ip[i] != '.' && (ip[i] < '0' || ip[i] > '9'))
			return 0;
	}
	return ret;
}

function hideprogress()
{
	$(".progressbarclass").hideProgressBar();
}

/* Converts the single digit to double digit and sets in the control */
function setTwoDigitTimeVal(ctrlId, value)
{
	var tmpval = ("0" + value).slice(-2);

	$(ctrlId).html(tmpval);
	setTimeInTextBox();
}

/* Adds the hour by one and sets the incremented value in control */
function addHour()
{
	var prevval = $('.hourentry').html();

	/* Convert string time value to integer */
	prevval = parseInt(prevval, 10);
	if (prevval == 12)
		prevval = 1;
	else
		prevval++;
	setTwoDigitTimeVal('.hourentry', prevval);
}

/* Substracts the hour by one and sets the value in the control */
function substractHour()
{
	var prevval = $('.hourentry').html();

	/* Convert string time value to integer */
	prevval = parseInt(prevval, 10);
	if (prevval == 1)
		prevval = 12;
	else
		prevval--;
	setTwoDigitTimeVal('.hourentry', prevval);
}

/* Adds the minute by one and sets the incremented value in control */
function addMinute()
{
	var prevval = $('.minuteentry').html();

	/* Convert string time value to integer */
	prevval = parseInt(prevval, 10);
	if (prevval == 59) {
		prevval = 0;
		addHour();
	} else
		prevval++;
	setTwoDigitTimeVal('.minuteentry', prevval);
}

/* Substracts the minute by one and sets the value in the control */
function substractMinute()
{
	var prevval = $('.minuteentry').html();

	/* Convert string time value to integer */
	prevval = parseInt(prevval, 10);
	if (prevval == 0) {
		prevval = 59;
		substractHour();
	} else
		prevval--;
	setTwoDigitTimeVal('.minuteentry', prevval);
}

/* Toggles the AM and PM entry and sets the updated value in control */
function toggleAMAndPM()
{
	var prevval = $('.ampmentry').html();

	if (prevval == 'AM')
		$('.ampmentry').html('PM');
	else
		$('.ampmentry').html('AM');
	setTimeInTextBox();
}

/* Sets the time in time entry box from the timer control */
function setTimeInTextBox()
{
	var hour = $('.hourentry').html();
	var minute = $('.minuteentry').html();
	var amppm = $('.ampmentry').html();

	$(gTimeSelectCtrlId).val(hour+':'+minute+' '+amppm);
}

/* Sets the time in timer control from time entry box */
function setTimeInTimerCtrl()
{
	var time = $(gTimeSelectCtrlId).val();

	$('.hourentry').html(getHourFromString(time));
	$('.minuteentry').html(getMinutesFromString(time));
	$('.ampmentry').html(getAMPMFromString(time));
}

/* Shows the time select control */
function showTimeSelectCtrl(ctrlId)
{
	/* Get the left and top position of the time entry text box to position the time select control */
	var left = $(ctrlId).position().left;
	var top = $(ctrlId).position().top;

	/* If the mouse is clicked again in the same control, hide the time select control */
	if (gTimeSelectCtrlId == ctrlId) {
		if ($('.timeselectdiv').is(':visible') == true) { /* Hide if its visible */
			$('.timeselectdiv').css('display', 'none');
			return;
		}
	}
	/* Set the current time entry text box ID to global variable */
	gTimeSelectCtrlId = ctrlId;
	/* Position the time select control */
	$(".timeselectdiv").css({top: top+20, left: left+15, position:'absolute'});
	/* Set the Time in timer control from the time entry text box */
	setTimeInTimerCtrl();
	/* Show the time select control */
	$('.timeselectdiv').css('display', '');
}

/* Show DB size and sample interval warning message */
function showDBSizeAndIntervalMsg()
{
	var timeIntervl = $(".smplinterval:checked").val();
	var txtToDisplay = '';
	var reqMB = '';

	/* In internal build For 1 hour 2ith 5 seconds time interval it fills 1.9 MB of data in DB
	 * So calculate for 10, 15 and 20 seconds time interval. Similarly for external build it fills
	 * 1.3 MB of data.
	 */
	if (gIsBCMInternal == 1)
		reqMB = (1.9)/(timeIntervl/5);
	else
		reqMB = (1.3)/(timeIntervl/5);
	txtToDisplay = "(Please note that, for example, 2 STA's connected using a ";
	txtToDisplay += timeIntervl+" seconds sample interval run for 1 hour will occupy ";
	txtToDisplay += "approximately "+reqMB.toFixed(2)+" MB of database)";
	$('#dbwarningtxtid').html(txtToDisplay);
}

$(function() {
	$("#startremotedebug").click(function() {
		var ip, isenable;

		isenable = $('#remoteenablechkid').is(':checked')?1:0;
		ip = $(".remoteip").val();
		if (isenable == 1 && validateIP(ip) == 0) {
			alert('IP address is in wrong format');
			return;
		}

		var r = confirm("This will Restart all the services!");
		if (r == true) {
			$.ajax({
				xhr: function()
				{
					var xhr = new window.XMLHttpRequest();
					//Download progress
					$(".progressbarclass").showProgressBar();
					xhr.addEventListener("progress", function(evt) {
						if (evt.lengthComputable) {
							var percentComplete = evt.loaded / evt.total;
						}
					}, false);
					return xhr;
				},
				type:"GET",
				url:urlsetconfigsettings,
				data:{Req:'SetRemoteDebug',IsRemoteDebugEnabled:isenable,DCONIP:ip},
				timeout:5000,
				complete:function(xhr,status) {
					setTimeout('hideprogress()', 5000);
				}
			});
		}
	});

	/* On clicking Start data collecting every check box */
	$('#autostartid').click(function() {
		if ($('#autostartid').is(':checked')) /* If checked enable all controls */
			$(".autostartdivclass").children().prop('disabled',false);
		else /* Else disable all controls */
			$(".autostartdivclass").children().prop('disabled',true);
	});

	/* On clicking close button in time select control */
	$('.timeselectclosebtn').click(function() {
		$('.timeselectdiv').css('display', 'none');
	});

	/* On clicking on from time entry text box */
	$('#fromtimeentry').click(function() {
		showTimeSelectCtrl('#fromtimeentry');
	});

	/* On clicking To time entry text box */
	$('#totimeentry').click(function() {
		showTimeSelectCtrl('#totimeentry');
	});

	/* On clicking Plus(+) button to increment hour in time select control */
	$('#hourplusid').click(function() {
		addHour();
	});

	/* On clicking Minus(-) button to decrement hour in time select control */
	$('#hourminusid').click(function() {
		substractHour();
	});

	/* On clicking Plus(+) button to increment minute in time select control */
	$('#minuteplusid').click(function() {
		addMinute();
	});

	/* On clicking Minus(-) button to decrement minute in time select control */
	$('#minuteminusid').click(function() {
		substractMinute();
	});

	/* On clicking Plus(+) button to change AM or PM in time select control */
	$('#ampmplusid').click(function() {
		toggleAMAndPM();
	});

	/* On clicking Minus(-) button to change AM or PM in time select control */
	$('#ampmminusid').click(function() {
		toggleAMAndPM();
	});

	$('#dbsizeinmb').keyup(function() {
		showDBSizeAndIntervalMsg();
	});

	$('.smplinterval').change(function() {
		showDBSizeAndIntervalMsg();
	});
});
