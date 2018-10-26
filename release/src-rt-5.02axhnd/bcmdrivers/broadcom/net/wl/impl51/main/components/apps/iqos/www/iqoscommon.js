/*
 * Jquery implementation for Common API for iQoS Pages
 *
 * $ Copyright Open Broadcom Corporation $
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: iqoscommon.js, v 1.0 2015-02-12 15:25:43 vbemblek $
 */

/* Function to check if a given variable has a numeric value or not */
function isNumber(n) {
	  return !isNaN(parseFloat(n)) && isFinite(n);
}

/* Function for getting device type / application category icon */
function GetIcon(which, id)
{
	if (isNumber(id) != true) {
		return "unknown.png";
	}
	if ((which != "devtype") && (which != "appcat")) {
		return "unknown.png";
	}
	if ((which == "devtype") && (id <= 0)) {
		return "unknown.png";
	}
	if ((which == "appcat") && (id < 0)) {
		return "unknown.png";
	}
	return which + id + ".png";
}

function ManageMissingIcon(image)
{
	image.onerror = "";
	image.src = "default.png";
	return true;
}

function ConvertEpochTimeToString(tm)
{
	months = ['Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec']
	var d = new Date(0); // set the date to epoch
	var ampm = "am";
	d.setUTCSeconds(tm);
	var hr = d.getHours();
	if (hr >= 12) {
		ampm = "pm";
		hr -= 12;
	}
	var min = d.getMinutes();
	if (min < 10)
		min = "0" + min;

	var sec = d.getSeconds();
	if (sec < 10)
		sec = "0" + sec;

	var dstr = d.getDate() + " " + months[d.getMonth()] + ", " + hr + ":" + min + ":" + sec + " " + ampm;
	return dstr;
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
			"background-color": "#28539E",
			"background-image": "-moz-linear-gradient(45deg, #28539E 25%, #04245C",
			"background-image": "-webkit-linear-gradient(45deg, #28539E 25%, #04245C",
			"background-image": "-o-linear-gradient(45deg, #28539E 25%, #04245C",
			"background-image": "-ms-linear-gradient(45deg, #28539E 25%, #04245C",
			"background-image": "linear-gradient(45deg, #28539E 25%, #04245C",
			"border-left": "1px solid #111",
			"border-top": "1px solid #111",
			"border-right": "1px solid #333",
			"border-bottom": "1px solid #333",
			"width": "25px",
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
