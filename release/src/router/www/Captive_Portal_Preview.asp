<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<title></title>
<script type="text/javascript" src="js/jquery.js"></script>
<style>
.terms_service {
	position: fixed;
	width: 80vw;
	height: 68vh;
	background-color: #232E32;
	left: 7vw;
	top: 13vh;
	z-index: 200;
	padding: 3vh 3vw;
	border-radius: 2vw;
	display: none;
	box-shadow: 0 19px 38px rgba(0,0,0,0.30), 0 15px 12px rgba(0,0,0,0.22);
	color: #FFFFFF;
}
.term_service_title {
	font-weight: bolder;
	text-align: center;
	font-family: Microsoft JhengHei;
	border-bottom: 1px solid;
	padding-bottom: 2%;
}
.term_service_text {
	position: absolute;
	bottom: 2%;
	top: 2%;
	height: 96%;
	overflow: auto;
	overflow-x: hidden;
	font-family: Arial, Helvetica, sans-serif;
	white-space: normal;
	word-break: break-all;
}
.terms_service_close {
	width: 5vmin;
	height: 5vmin;
	cursor: pointer;
	position: absolute;
	right: 1vw;
	top: 1vmin;
}
.splash_template_terms_service {
	color: #757575;
	text-align: left;
	border-bottom-color: #4A90E2;
	border-bottom-style: solid;
	cursor: pointer;
	line-height: 180%;
}
.splash_template_terms_service_hyperlink {
	color: #4A90E2;
	text-decoration: underline;
	cursor:pointer;
}
.splash_template_terms_service_cb {
	width: 15%;
	float: left;
	margin-top: 2%;
}
.splash_template_terms_service_text {
	width: 85%;
	float: left;
}
.splash_body {
	margin: 0px;
	background-repeat: no-repeat;
	background-size: cover;
	background-attachment: fixed;
	background-position: 50% 50%;
}
.splash_template_bg {
	position: absolute;
	top: 0;
	right: 0;
	bottom: 0;
	left: 0;
}
.splash_template_content {
	font-family: Arial, Roboto, Helvetica;
	background-color: rgba(255, 255, 255, .93);
	border-radius: 8px;
	box-shadow: 2px 3px 6px 0px rgba(0, 0, 0, 0.20);
	border: 1px solid #ACACAC;
	position: relative;
}
.splash_template_title {
	color: #757575;
	text-align: left;
}
.splash_template_brand_name {
	font-weight: lighter;
	color: #4A90E2;
	text-align: left;
	border-bottom-color: #D4D4D4;
	border-bottom-style: solid;
	word-break: break-all;
}
.splash_template_continue {
	background-color: #4A90E2;
	border-radius: 4px;
	transition: visibility 0s linear 0.218s,opacity 0.218s,background-color 0.218s;
	width: 90%;
	color: #fff;
	color: #000\9;
	text-align: center;
	cursor: pointer;
}
.splash_template_icon {
	background-size: 100%;
	position: absolute;
	background-repeat: no-repeat;
}
.splash_template_passcode {
	background-color: rgba(74, 144, 226, 0.5);
	border-radius: 4px;
	width: 90%;
	border: 0px;
	color: #FFFFFF;
	padding-left: 2%;
	font-size: 12px;
	margin-left: 4%;
}
</style>
<script>
var htmlEnDeCode = (function() {
    var charToEntityRegex,
        entityToCharRegex,
        charToEntity,
        entityToChar;

    function resetCharacterEntities() {
        charToEntity = {};
        entityToChar = {};
        // add the default set
        addCharacterEntities({
            '&amp;'     :   '&',
            '&gt;'      :   '>',
            '&lt;'      :   '<',
            '&quot;'    :   '"',
            '&#39;'     :   "'"
        });
    }

    function addCharacterEntities(newEntities) {
        var charKeys = [],
            entityKeys = [],
            key, echar;
        for (key in newEntities) {
            echar = newEntities[key];
            entityToChar[key] = echar;
            charToEntity[echar] = key;
            charKeys.push(echar);
            entityKeys.push(key);
        }
        charToEntityRegex = new RegExp('(' + charKeys.join('|') + ')', 'g');
        entityToCharRegex = new RegExp('(' + entityKeys.join('|') + '|&#[0-9]{1,5};' + ')', 'g');
    }

    function htmlEncode(value){
        var htmlEncodeReplaceFn = function(match, capture) {
            return charToEntity[capture];
        };

        return (!value) ? value : String(value).replace(charToEntityRegex, htmlEncodeReplaceFn);
    }

    function htmlDecode(value) {
        var htmlDecodeReplaceFn = function(match, capture) {
            return (capture in entityToChar) ? entityToChar[capture] : String.fromCharCode(parseInt(capture.substr(2), 10));
        };

        return (!value) ? value : String(value).replace(entityToCharRegex, htmlDecodeReplaceFn);
    }

    resetCharacterEntities();

    return {
        htmlEncode: htmlEncode,
        htmlDecode: htmlDecode
    };
})();
window.moveTo(0,0);
var windw_width = screen.width;
var windw_height = screen.height;
if(isMobile()) {
	var supportsOrientationChange = "onorientationchange" in window,
		orientationEvent = supportsOrientationChange ? "orientationchange" : "resize";

	window.addEventListener(orientationEvent, function() {
		if(window.orientation == undefined) {
			setTimeout(function() {
				windw_width = screen.width;
				windw_height = screen.height;
				resize_component();
			},100);
		}
		else {
			switch(window.orientation) {
				case -90 :
				case 90 :
					windw_width = (screen.height > screen.width) ? screen.height : screen.width;
					windw_height = (screen.height > screen.width) ? screen.width : screen.height;
					break; 
				default :
					windw_width = (screen.height < screen.width) ? screen.height : screen.width;
					windw_height = (screen.height < screen.width) ? screen.width : screen.height;
					break; 
			}
			resize_component();
		}
	}, false);
}
window.onresize = function() {
	if(!isMobile()) {
		initial();
	}
};
function initial() {
	document.title = opener.document.form.brand_name.value;
	document.getElementById('splash_template_brand_name').innerText = opener.document.form.brand_name.value;

	document.body.style.backgroundImage = "url('" + opener.splash_image_base64 + "')";

	var backgroundSize = "cover";
	
	if(opener.document.form.splash_image_size) {
		backgroundSize = (opener.document.form.splash_image_size.value == "center") ? "cover" : "100% 100%" ;
	}
	document.body.style.backgroundSize = backgroundSize;

	var mql = window.matchMedia("(orientation: portrait)");
	if(mql.matches) {  
		// Portrait orientation
		windw_width = (screen.height < screen.width) ? screen.height : screen.width;
		windw_height = (screen.height < screen.width) ? screen.width : screen.height;
	}
	else {  
		// Landscape orientation
		windw_width = (screen.height > screen.width) ? screen.height : screen.width;
		windw_height = (screen.height > screen.width) ? screen.width : screen.height;
	}

	if(opener.document.form.cb_terms_service.checked == false) {
		document.getElementById("splash_template_terms_service").style.display = "none";
		document.getElementById("terms_service").style.display = "none";
	}
	else {
		var code = '';
		code += '<div style="display:table;width:100%;height:100%;border-collapse:collapse;">';
		code += '<div style="display:table-row;">';
		code += '<div id="term_service_title" class="term_service_title" style="display:table-cell;vertical-align:middle;font-size:13px;" >';
		code += 'Free Wi-Fi TERMS OF SERVICE';
		code += '</div>';
		code += '</div>';
		code += '<div style="display:table-row;height:90%;">';
		code += '<div style="display:table-cell;">';
		code += '<div style="width:100%;max-width:100%;height:100%;max-height:100%;position:relative;">';
		code += '<div id="term_service_text" class="term_service_text" style="font-size:12px;">';
		code += htmlEnDeCode.htmlEncode(opener.document.getElementById('terms_service').value).replace(/ /g, '&nbsp;').replace(/(?:\r\n|\r|\n)/g, '<br>');
		code += '</div>';
		code += '</div>';
		code += '</div>';
		code += '</div>';
		code += '</div>';
		code += '<div class="terms_service_close" onclick="close_term_service();">';
		code += '<svg version="1.1" id="terms_service_close_icon" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" x="0px" y="0px" viewBox="0 0 64 64" enable-background="new 0 0 64 64" xml:space="preserve">';
		code += '<line fill="none" stroke="#FFFFFF" stroke-width="3" stroke-linecap="round" stroke-linejoin="round" stroke-miterlimit="10" x1="21.1" y1="21.1" x2="42.9" y2="42.9"/>';
		code += '<line fill="none" stroke="#FFFFFF" stroke-width="3" stroke-linecap="round" stroke-linejoin="round" stroke-miterlimit="10" x1="42.9" y1="21.1" x2="21.1" y2="42.9"/>';
		code += '<circle fill="none" stroke="#FFFFFF" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round" stroke-miterlimit="10" cx="32" cy="32.1" r="25.2"/>';
		code += '</svg>';
		code += '</div>';
		$('#terms_service').html(code);
		control_bt_status();
	}

	if(opener.document.form.cb_passcode.checked == false) {
		document.getElementById("splash_template_passcode").style.display = "none";
	}

	resize_component()
}
function resize_component() {
	var _ratio = 0;
	if(isMobile()) {
		$('meta[name=viewport]').attr('content', 'initial-scale=1.0,maximum-scale=1.0,minimum-scale=1.0,user-scalable=no,width=' + windw_width + ',height=' + windw_height);
		document.getElementById('splash_template_bg').style.margin = 'auto';
		_ratio = ((windw_width / 256));
		//landscape
		if(windw_width > windw_height) {
			document.getElementById('splash_template_bg').style.margin = '10%';
		}
	}
	else {
		$('meta[name=viewport]').attr('content', 'initial-scale=1.0,maximum-scale=1.0,minimum-scale=1.0,user-scalable=no,width=' + windw_width + ',height=' + windw_height);
		document.getElementById('splash_template_bg').style.margin = 'auto';
		_ratio = 1.8;
	}

	//splash_template_bg
	document.getElementById('splash_template_bg').style.width = (205 * _ratio) + 'px';
	document.getElementById('splash_template_bg').style.height = (145 * _ratio) + 'px';
	if(document.getElementById('splash_template_terms_service').style.display != 'none')
		document.getElementById('splash_template_bg').style.height = (203 * _ratio) + 'px';

	//splash_template_icon
	document.getElementById('splash_template_icon').style.width = (80 * _ratio) + 'px';
	document.getElementById('splash_template_icon').style.height = (60 * _ratio) + 'px';
	document.getElementById('splash_template_icon').style.top = (-20 * _ratio) + 'px';
	document.getElementById('splash_template_icon').style.right = (-15 * _ratio) + 'px';
	
	//splash_template_title
	document.getElementById('splash_template_title').style.margin = (10 * _ratio) + 'px';
	document.getElementById('splash_template_title').style.marginTop = (30 * _ratio) + 'px';
	document.getElementById('splash_template_title').style.fontSize = (12 * _ratio) + 'px';

	//splash_template_brand_name
	document.getElementById('splash_template_brand_name').style.margin = (10 * _ratio) + 'px';
	document.getElementById('splash_template_brand_name').style.marginTop = '0px';
	document.getElementById('splash_template_brand_name').style.fontSize = (22 * _ratio) + 'px';
	document.getElementById('splash_template_brand_name').style.borderBottomWidth = (1 * _ratio) + 'px';
	document.getElementById('splash_template_brand_name').style.paddingBottom = (5 * _ratio) + 'px';

	if(document.getElementById('splash_template_terms_service').style.display != 'none') {
		//cbTermService
		document.getElementById('cbTermService').style.width = (13 * _ratio) + 'px';
		document.getElementById('cbTermService').style.height = (13 * _ratio) + 'px';

		//splash_template_terms_service
		document.getElementById('splash_template_terms_service').style.margin = (10 * _ratio) + 'px';
		document.getElementById('splash_template_terms_service').style.marginTop = (0 * _ratio) + 'px';
		document.getElementById('splash_template_terms_service').style.fontSize = (12 * _ratio) + 'px';
		document.getElementById('splash_template_terms_service').style.borderBottomWidth = (1 * _ratio) + 'px';
		document.getElementById('splash_template_terms_service').style.paddingBottom = (5 * _ratio) + 'px';

		//term_service_title
		document.getElementById('term_service_title').style.fontSize = (10 * _ratio) + 'px';

		//term_service_text
		document.getElementById('term_service_text').style.fontSize = (9 * _ratio) + 'px';
	}

	//splash_template_continue
	document.getElementById('splash_template_continue').style.height = (25 * _ratio) + 'px';
	document.getElementById('splash_template_continue').style.fontSize = (12 * _ratio) + 'px';
	document.getElementById('splash_template_continue').style.lineHeight = (25 * _ratio) + 'px';
	document.getElementById('splash_template_continue').style.margin = '' + (15 * _ratio) + 'px auto';

	//splash_template_passcode
	document.getElementById('splash_template_passcode').style.height = (20 * _ratio) + 'px';
	document.getElementById('splash_template_passcode').style.fontSize = (12 * _ratio) + 'px';
	document.getElementById('splash_template_passcode').style.lineHeight = (20 * _ratio) + 'px';
}
function control_bt_status() {
	var _obj = document.getElementById('cbTermService');
	if(_obj.checked) {
		document.getElementById('splash_template_continue').style.opacity = 1;
	}
	else {
		document.getElementById('splash_template_continue').style.opacity = 0.5;
	}
}
function continue_action() {
	var passcode_status = false;
	if(opener.document.form.cb_passcode.checked == true) {
		passcode_status = true;
		var _obj_value = document.getElementById('splash_template_passcode').value.trim();
		if(_obj_value == '') {
			alert("<#JS_fieldblank#>");
			document.getElementById('splash_template_passcode').focus();
			return false;
		}
	}

	if(opener.document.form.cb_terms_service.checked == true) {
		var _obj = document.getElementById('cbTermService');
		if(!_obj.checked) {
			alert('You must agree to the terms of service before continuing.');
			return false;
		}
	}

	if(passcode_status)
		console.log("formSubmit(2)");
	else
		console.log("formSubmit(0)");
}
function open_term_service() {
	if(isMobile()) {
		$('#terms_service').css('display', 'block');
		$('#splash_template_bg').css('display', 'none');
	}
	else {
		$('#terms_service').fadeIn(300);
		$('#splash_template_bg').fadeOut(100);
	}
}
function close_term_service() {
	if(isMobile()) {
		$('#terms_service').css('display', 'none');
		$('#splash_template_bg').css('display', 'block');
	}
	else {
		$('#terms_service').fadeOut(300);
		$('#splash_template_bg').fadeIn(100);
	}
}
function isMobile() {
	var userAgentString = navigator.userAgent.toLowerCase();
	var mobile = ['iphone','ipad','ipod','android','blackberry','nokia','opera mini','windows mobile','windows phone','iemobile','mobile safari','bb10; touch', 'tablet'];
	for (var i in mobile) if (userAgentString.indexOf(mobile[i]) > 0) return true;
	return false;
}
</script>
</head>
<body onload="initial();" id='splash_body' class='splash_body' >
<div id='terms_service' class='terms_service'></div>
<div id='splash_template_bg' class='splash_template_bg'>
	<div id='splash_template_content' class='splash_template_content'>
		<svg id='splash_template_icon' class='splash_template_icon' width='214px' height='164px' viewBox='0 0 214 164' version='1.1' xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' xmlns:sketch='http://www.bohemiancoding.com/sketch/ns'>
			<defs></defs>
			<g id='Page-1' stroke='none' stroke-width='1' fill='none' fill-rule='evenodd' sketch:type='MSPage'>
				<g id='Artboard-11' sketch:type='MSArtboardGroup' transform='translate(-1696.000000, -489.000000)' fill='#B1B1B1'>
					<g id='Page-1' sketch:type='MSLayerGroup' transform='translate(1696.000000, 489.000000)'>
						<path d='M137.890672,79.9008507 L122.005952,79.9008507 L122.005952,90.1224355 L135.284336,90.1224355 C136.51389,90.1224355 137.433071,90.3979392 138.0379,90.9529393 C138.642729,91.5039466 138.953102,92.2466086 138.953102,93.1729397 C138.953102,94.0992709 138.63875,94.8339473 138.025963,95.3809618 C137.413175,95.9279763 136.493994,96.1954943 135.284336,96.1954943 L122.005952,96.1954943 L122.005952,109.359778 C122.005952,111.032763 121.627934,112.266541 120.879856,113.081073 C120.123819,113.88762 119.160868,114.290894 117.983043,114.290894 C116.789301,114.290894 115.818392,113.883627 115.066335,113.065102 C114.314277,112.250569 113.936259,111.012799 113.936259,109.359778 L113.936259,78.6231526 C113.936259,77.4612459 114.107362,76.5069651 114.453547,75.7762815 C114.795753,75.0376123 115.336916,74.5025762 116.073057,74.1631877 C116.801239,73.8277919 117.744295,73.6640869 118.882328,73.6640869 L137.890672,73.6640869 C139.175934,73.6640869 140.130927,73.9475761 140.755652,74.5185475 C141.380376,75.0935116 141.690749,75.8401664 141.690749,76.7704904 C141.690749,77.7127927 141.380376,78.471426 140.755652,79.0423973 C140.130927,79.6173614 139.175934,79.9008507 137.890672,79.9008507' id='Fill-1' sketch:type='MSShapeGroup'></path>
						<path d='M153.85776,88.2685754 L153.85776,109.769839 C153.85776,111.259156 153.503617,112.385127 152.799309,113.14376 C152.095002,113.910379 151.195717,114.293689 150.113391,114.293689 C149.023107,114.293689 148.139739,113.898401 147.463285,113.119803 C146.786831,112.337213 146.444626,111.219228 146.444626,109.769839 L146.444626,88.4881798 C146.444626,87.0148342 146.786831,85.9048339 147.463285,85.1621719 C148.139739,84.4155171 149.023107,84.0441861 150.113391,84.0441861 C151.195717,84.0441861 152.095002,84.4155171 152.799309,85.1621719 C153.503617,85.9048339 153.85776,86.9429637 153.85776,88.2685754 M150.192974,80.5824228 C149.158398,80.5824228 148.279008,80.2669911 147.542868,79.628142 C146.810706,78.9932858 146.444626,78.0949043 146.444626,76.9329976 C146.444626,75.8749039 146.818665,75.0124576 147.570722,74.3296877 C148.322779,73.6469178 149.19421,73.3075292 150.192974,73.3075292 C151.151946,73.3075292 152.003482,73.6149753 152.747581,74.2338604 C153.4877,74.8527454 153.85776,75.7511269 153.85776,76.9329976 C153.85776,78.0749403 153.495659,78.9733217 152.775435,79.6161636 C152.047252,80.2590055 151.187758,80.5824228 150.192974,80.5824228' id='Fill-3' sketch:type='MSShapeGroup'></path>
						<path d='M210.144661,56.353274 C209.949684,56.353274 209.754706,56.33331 209.559728,56.2933819 C207.940219,55.9739574 206.893705,54.3967988 207.216016,52.7757193 C211.453798,31.3503193 197.538751,10.4599554 176.186695,6.19963075 C174.571165,5.88020623 173.520672,4.30304763 173.842982,2.68196817 C174.161314,1.0608887 175.745011,0.0147733843 177.352582,0.330205103 C189.266123,2.7019322 199.540258,9.58553072 206.280918,19.7072954 C213.025558,29.8250672 215.432936,41.9831632 213.065349,53.9416188 C212.786809,55.3670508 211.541339,56.353274 210.144661,56.353274' id='Fill-5' sketch:type='MSShapeGroup'></path>
						<path d='M196.260652,53.5846619 C196.065674,53.5846619 195.870696,53.5646979 195.675719,53.5247698 C194.056209,53.2053453 193.009696,51.6281867 193.332006,50.0071072 C194.649101,43.3431131 193.308131,36.5673204 189.551825,30.9294775 C185.795518,25.2876418 180.069538,21.4545475 173.428356,20.1289357 C171.808847,19.8095112 170.762334,18.2323526 171.084644,16.6072803 C171.402975,14.9862009 172.978714,13.9480712 174.594244,14.2555173 C182.795248,15.892568 189.874135,20.6320294 194.517789,27.5994768 C199.157465,34.5709171 200.816765,42.9438324 199.18134,51.1730068 C198.9028,52.5984387 197.65733,53.5846619 196.260652,53.5846619' id='Fill-7' sketch:type='MSShapeGroup'></path>
						<path d='M182.380224,50.8164491 C182.185246,50.8164491 181.990268,50.7964851 181.79529,50.756557 C180.175781,50.4371325 179.129268,48.8599739 179.451578,47.2388945 C180.028553,44.3001888 179.439641,41.3095767 177.784319,38.826051 C176.125018,36.3385325 173.598265,34.6455826 170.66962,34.0626328 C169.05011,33.7432083 168.003597,32.1660497 168.325907,30.5409774 C168.644238,28.9238907 170.212019,27.8737826 171.835507,28.1892143 C176.327954,29.083603 180.199656,31.6789273 182.746304,35.4960504 C185.284995,39.3131734 186.196217,43.9009082 185.300911,48.404794 C185.022372,49.8302259 183.776901,50.8164491 182.380224,50.8164491' id='Fill-9' sketch:type='MSShapeGroup'></path>
						<path d='M95.8650025,141.609676 C94.6434071,141.609676 93.807788,142.18464 93.3541663,143.330576 C93.1193971,143.937482 93.0000229,144.708094 93.0000229,145.646404 C93.0000229,147.127735 93.3979368,148.169857 94.1897853,148.768778 C94.6593236,149.120145 95.2203821,149.295829 95.8610234,149.295829 C96.7961209,149.295829 97.5083867,148.936476 98.0017998,148.217771 C98.4912338,147.503059 98.7379404,146.544785 98.7379404,145.350936 C98.7379404,144.368705 98.5111295,143.502266 98.0614869,142.743633 C97.6078651,141.988993 96.8757037,141.609676 95.8650025,141.609676' id='Fill-11' sketch:type='MSShapeGroup'></path>
						<path d='M109.349109,36.7481945 C108.752238,36.2251368 108.00416,35.9695972 107.112833,35.9695972 C106.145902,35.9695972 105.397824,36.2411081 104.86462,36.7961082 C104.331415,37.3431227 103.997168,38.0897775 103.861877,39.0320798 L110.351851,39.0320798 C110.284206,38.0258926 109.949958,37.2672593 109.349109,36.7481945' id='Fill-13' sketch:type='MSShapeGroup'></path>
						<path d='M110.207807,141.387676 C109.236897,141.387676 108.488819,141.731057 107.963573,142.421813 C107.442305,143.112569 107.175703,144.094799 107.175703,145.372497 C107.175703,146.646202 107.442305,147.632426 107.963573,148.323181 C108.488819,149.017929 109.236897,149.365304 110.207807,149.365304 C111.178716,149.365304 111.926794,149.017929 112.444082,148.323181 C112.965349,147.632426 113.227973,146.646202 113.227973,145.372497 C113.227973,144.094799 112.965349,143.112569 112.444082,142.421813 C111.926794,141.731057 111.178716,141.387676 110.207807,141.387676' id='Fill-15' sketch:type='MSShapeGroup'></path>
						<path d='M124.406964,36.7481945 C123.810093,36.2251368 123.062015,35.9695972 122.170688,35.9695972 C121.203757,35.9695972 120.455679,36.2411081 119.922475,36.7961082 C119.38927,37.3431227 119.055023,38.0897775 118.919732,39.0320798 L125.409707,39.0320798 C125.342061,38.0258926 125.007814,37.2672593 124.406964,36.7481945' id='Fill-17' sketch:type='MSShapeGroup'></path>
						<path d='M133.596386,130.627861 C113.139636,130.627861 96.5545872,113.985843 96.5545872,93.4588247 C96.5545872,72.9278133 113.139636,56.2857956 133.596386,56.2857956 C154.053136,56.2857956 170.638184,72.9278133 170.638184,93.4588247 C170.638184,95.4991488 169.746858,115.938326 170.33577,121.476348 C171.246993,130.016962 176.113479,130.627861 176.113479,130.627861 L133.596386,130.627861 Z M124.909927,141.468331 L122.844754,141.468331 L122.844754,148.208188 C122.844754,148.731246 122.912399,149.054663 123.043711,149.186426 C123.175023,149.310203 123.580895,149.378081 124.253369,149.378081 C124.356827,149.378081 124.460284,149.378081 124.575679,149.374088 C124.687095,149.370095 124.798511,149.362109 124.909927,149.354124 L124.909927,151.845635 L123.334188,151.905527 C121.762428,151.957434 120.69204,151.685923 120.115065,151.087002 C119.745005,150.703692 119.557986,150.112757 119.557986,149.318189 L119.557986,141.468331 L117.78329,141.468331 L117.78329,139.096604 L119.557986,139.096604 L119.557986,135.539013 L122.844754,135.539013 L122.844754,139.096604 L124.909927,139.096604 L124.909927,141.468331 Z M115.10533,150.168656 C114.030963,151.498261 112.403495,152.161067 110.218948,152.161067 C108.034401,152.161067 106.406934,151.498261 105.336545,150.168656 C104.262178,148.839052 103.724994,147.241929 103.724994,145.373296 C103.724994,143.532612 104.262178,141.939482 105.336545,140.593906 C106.406934,139.244338 108.034401,138.569553 110.218948,138.569553 C112.403495,138.569553 114.030963,139.244338 115.10533,140.593906 C116.175718,141.939482 116.712902,143.532612 116.712902,145.373296 C116.712902,147.241929 116.175718,148.839052 115.10533,150.168656 L115.10533,150.168656 Z M100.645141,150.324376 C99.6304611,151.502254 98.3292829,152.089196 96.7296693,152.089196 C95.714989,152.089196 94.8714117,151.837649 94.1989373,151.330563 C93.8288775,151.051067 93.470755,150.639807 93.1166117,150.100779 L93.1166117,156.748802 L89.8298434,156.748802 L89.8298434,138.97682 L93.0131541,138.97682 L93.0131541,140.861424 C93.3752557,140.306424 93.7532738,139.871208 94.1631251,139.551784 C94.9032448,138.980812 95.7905927,138.697323 96.8132312,138.697323 C98.3093872,138.697323 99.5707741,139.24833 100.609329,140.346352 C101.647884,141.444374 102.165172,143.061461 102.165172,145.189627 C102.165172,147.433584 101.659822,149.146498 100.645141,150.324376 L100.645141,150.324376 Z M88.2381881,31.6062582 L78.3898208,31.6062582 L78.3898208,36.1021584 L87.0205718,36.1021584 L87.0205718,39.496044 L78.3898208,39.496044 L78.3898208,47.6932758 L74.3430371,47.6932758 L74.3430371,28.1644589 L88.2381881,28.1644589 L88.2381881,31.6062582 Z M78.4296122,76.9325983 C78.4296122,75.8784974 78.8076303,75.0120584 79.5557083,74.3292884 C80.3117446,73.6465185 81.1791967,73.30713 82.1779605,73.30713 C83.1369328,73.30713 83.9884684,73.6145761 84.7325673,74.2334611 C85.472687,74.8523461 85.8427468,75.7507276 85.8427468,76.9325983 C85.8427468,78.074541 85.4806453,78.9729225 84.7604212,79.6157643 C84.0322389,80.2586062 83.172745,80.5820235 82.1779605,80.5820235 C81.1433845,80.5820235 80.2639949,80.2665918 79.5278543,79.6277427 C78.799672,78.9928865 78.4296122,78.094505 78.4296122,76.9325983 L78.4296122,76.9325983 Z M79.4482716,85.1617726 C80.1287042,84.4151178 81.0080938,84.0437868 82.0983777,84.0437868 C83.1807033,84.0437868 84.0799886,84.4151178 84.7882752,85.1617726 C85.4886035,85.9044347 85.8427468,86.9425644 85.8427468,88.2681761 L85.8427468,109.769439 C85.8427468,111.258756 85.4886035,112.384728 84.7882752,113.147354 C84.0799886,113.90998 83.1807033,114.293289 82.0983777,114.293289 C81.0080938,114.293289 80.1287042,113.898001 79.4482716,113.119404 C78.771818,112.336814 78.4296122,111.218828 78.4296122,109.769439 L78.4296122,88.4877805 C78.4296122,87.0144349 78.771818,85.9044347 79.4482716,85.1617726 L79.4482716,85.1617726 Z M85.8507051,150.611858 C84.6370679,151.649988 82.9260385,152.173045 80.7096584,152.173045 C78.4495078,152.173045 76.6668539,151.661966 75.3696548,150.635815 C74.0764348,149.609663 73.4278353,148.200203 73.4278353,146.407433 L76.8538734,146.407433 C76.9692684,147.194015 77.1841419,147.780958 77.5064521,148.172253 C78.107302,148.882973 79.1259614,149.238332 80.5703886,149.238332 C81.4338616,149.238332 82.1381691,149.142505 82.6753527,148.954843 C83.6979913,148.595491 84.2113001,147.928692 84.2113001,146.954447 C84.2113001,146.387468 83.9606144,145.944267 83.459243,145.632828 C82.9578716,145.329375 82.1620439,145.061857 81.07176,144.826281 L79.2135024,144.419015 C77.3830988,144.011749 76.1296702,143.57254 75.4452584,143.097396 C74.2873292,142.298835 73.706375,141.057072 73.706375,139.364122 C73.706375,137.818906 74.2753917,136.537215 75.405467,135.515056 C76.5395215,134.492898 78.2028013,133.981819 80.3992856,133.981819 C82.2336684,133.981819 83.7934906,134.464948 85.0906897,135.431207 C86.3839097,136.393474 87.0643423,137.786963 87.1240294,139.619661 L83.6741165,139.619661 C83.6144294,138.585524 83.1488702,137.846855 82.2893763,137.411639 C81.7163804,137.120164 81.0001355,136.976423 80.1485999,136.976423 C79.201565,136.976423 78.4455287,137.164085 77.8765119,137.539409 C77.3154534,137.914733 77.0289555,138.437791 77.0289555,139.108582 C77.0289555,139.723474 77.3114743,140.182647 77.8645745,140.4861 C78.2266761,140.689734 78.9866915,140.929302 80.1485999,141.200813 L83.1647868,141.915525 C84.4858607,142.226964 85.4766661,142.642216 86.1372031,143.165274 C87.1598416,143.975813 87.6731505,145.149698 87.6731505,146.686929 C87.6731505,148.264088 87.0683215,149.569735 85.8507051,150.611858 L85.8507051,150.611858 Z M71.5815151,76.0941089 C69.0030335,84.2394343 66.428531,92.935767 63.8500493,102.1871 C63.3208239,104.059726 62.8870979,105.461201 62.5369337,106.363575 C62.1867695,107.269943 61.5938779,108.104439 60.762238,108.887029 C59.9226398,109.669619 58.748794,110.19667 57.2287632,110.448217 C55.7883151,110.687785 54.6383441,110.548037 53.770892,110.016994 C52.907419,109.481957 52.3025899,108.811166 51.9683423,108.016597 C51.6261364,107.218036 51.152619,105.832532 50.5398317,103.828143 C48.0369537,95.2635732 45.5300965,86.2238592 43.0272185,76.7050083 C40.4925074,85.8126001 37.9577963,95.3993287 35.4230852,105.47318 C34.8301936,107.769043 34.3606552,109.446022 34.0104911,110.472173 C33.664306,111.498325 33.0554979,112.464584 32.1920249,113.390915 C31.3245727,114.313253 30.1746017,114.900196 28.7381328,115.135772 C27.5722453,115.331419 26.6172521,115.243577 25.8731532,114.880232 C25.1210961,114.516887 24.5122879,113.917966 24.0467287,113.091455 C23.5811695,112.256958 23.1991722,111.242785 22.9086951,110.04095 C22.6102598,108.835123 22.3476366,107.713144 22.1128675,106.671022 C19.5343859,94.9960552 16.9559042,82.7660887 14.3814017,69.9851149 C13.9158425,67.7611216 13.6850525,66.060186 13.6850525,64.9142506 C13.6850525,63.460869 14.1068412,62.2989623 14.9543976,61.4484945 C15.797975,60.5940339 16.8444883,60.2786021 18.0859795,60.4862281 C19.8009881,60.7697173 20.9509591,61.6002211 21.5438507,62.9497897 C22.1327632,64.307344 22.6540303,66.2079199 23.0996938,68.6435319 C25.1250752,79.1925268 27.1504566,89.3622051 29.1798171,99.1605525 C31.4519051,89.7215577 33.7239931,80.6778509 35.9960811,72.0294319 C36.5014317,70.0410142 36.9550534,68.5556901 37.3569464,67.5534957 C37.7588394,66.5513012 38.411418,65.7407615 39.3186616,65.1258693 C40.2298843,64.5029915 41.4634171,64.3313008 43.0272185,64.5908332 C44.6148947,64.8503657 45.8444484,65.4373082 46.723838,66.3316969 C47.5992484,67.2220928 48.2040775,68.124467 48.5423042,69.0468054 C48.8845101,69.9731365 49.3381319,71.4784245 49.9071487,73.5427055 C52.1991323,81.8796856 54.491116,89.8173851 56.7830997,97.3558039 C58.8124602,89.0068454 60.8418208,81.0332107 62.8672022,73.4348998 C63.1656376,72.2450434 63.4441772,71.3266979 63.7107795,70.6718776 C63.9734026,70.0170573 64.4270244,69.46605 65.0716448,69.0268413 C65.7122861,68.5916254 66.6513627,68.475834 67.8729582,68.6794671 C69.1025119,68.8791075 70.141067,69.3901867 70.9965818,70.1847552 C71.8560756,70.9753309 72.2778643,71.8537484 72.2778643,72.8399716 C72.2778643,73.5347199 72.0510534,74.6127777 71.5815151,76.0941089 L71.5815151,76.0941089 Z M90.5381301,33.2353233 L94.1352711,33.2353233 L94.1352711,35.7547842 C94.7162253,34.7925178 95.225555,34.1337048 95.655302,33.778345 C96.3596094,33.1874096 97.2748112,32.8879491 98.4009074,32.8879491 C98.4725319,32.8879491 98.5361981,32.8919419 98.5799686,32.8959347 C98.6316974,32.8999275 98.7391341,32.9079131 98.906258,32.9158988 L98.906258,36.7929139 C98.6675097,36.7649643 98.4566153,36.7450002 98.2695958,36.7370146 C98.0865555,36.729029 97.9353482,36.7250362 97.8239323,36.7250362 C96.3078806,36.7250362 95.2892212,37.2201442 94.7679541,38.2103603 C94.477477,38.7693532 94.334228,39.6238138 94.334228,40.7857205 L94.334228,47.6932758 L90.5381301,47.6932758 L90.5381301,33.2353233 Z M101.962236,34.8524099 C103.28331,33.5188125 105.002298,32.848021 107.11522,32.848021 C108.36467,32.848021 109.494745,33.075611 110.501467,33.5267982 C111.50421,33.9779853 112.33585,34.6926977 112.992408,35.6669425 C113.58132,36.5253959 113.967297,37.5196047 114.138399,38.6535618 C114.241857,39.3203605 114.285628,40.2786341 114.265732,41.5243897 L103.768765,41.5243897 C103.828452,42.9737785 104.333803,43.9919442 105.276858,44.5788867 C105.849854,44.9422321 106.542224,45.1179156 107.353969,45.1179156 C108.209483,45.1179156 108.909812,44.8983113 109.446995,44.4551097 C109.737472,44.2195341 109.996116,43.8881312 110.218948,43.4609009 L114.066775,43.4609009 C113.967297,44.3193543 113.497758,45.1897861 112.670097,46.0761892 C111.380857,47.4816571 109.574328,48.1843911 107.250511,48.1843911 C105.332566,48.1843911 103.641433,47.5934557 102.17711,46.4075921 C100.704828,45.2177358 99.9766462,43.2892102 99.9766462,40.6180226 C99.9766462,38.1145329 100.637183,36.1900001 101.962236,34.8524099 L101.962236,34.8524099 Z M117.019296,34.8524099 C118.34037,33.5188125 120.059357,32.848021 122.17228,32.848021 C123.421729,32.848021 124.555784,33.075611 125.562506,33.5267982 C126.565248,33.9779853 127.396888,34.6926977 128.053446,35.6669425 C128.63838,36.5253959 129.024356,37.5196047 129.199438,38.6535618 C129.298916,39.3203605 129.342687,40.2786341 129.322791,41.5243897 L118.825824,41.5243897 C118.885511,42.9737785 119.390862,43.9919442 120.333918,44.5788867 C120.910893,44.9422321 121.599284,45.1179156 122.415007,45.1179156 C123.266543,45.1179156 123.966871,44.8983113 124.504055,44.4551097 C124.794532,44.2195341 125.057155,43.8881312 125.276008,43.4609009 L129.123834,43.4609009 C129.024356,44.3193543 128.558797,45.1897861 127.731136,46.0761892 C126.437916,47.4816571 124.631387,48.1843911 122.30757,48.1843911 C120.389626,48.1843911 118.698492,47.5934557 117.234169,46.4075921 C115.765867,45.2177358 115.033706,43.2892102 115.033706,40.6180226 C115.033706,38.1145329 115.694243,36.1900001 117.019296,34.8524099 L117.019296,34.8524099 Z M183.701696,121.939514 C183.399281,113.187282 183.566405,95.4033215 183.657925,87.7930322 C183.661904,87.3737875 183.717612,86.9665212 183.717612,86.5472765 C183.717612,86.2957297 183.6818,86.0521685 183.6818,85.8006217 C183.693737,84.7944344 183.701696,84.1715566 183.701696,84.1715566 C183.701696,80.3304767 183.26797,76.7169868 182.507954,73.3031371 C176.236832,37.0125183 144.781744,9.39028248 106.824743,9.39028248 C84.0720303,9.39028248 63.6908838,19.3643133 49.6126924,35.1239208 L11.5641718,32.7681649 C5.17765478,32.7681649 0.000795827664,37.9628062 0.000795827664,44.3712608 L0.000795827664,128.723292 C0.000795827664,135.131747 5.17765478,140.326388 11.5641718,140.326388 L51.7455106,140.326388 C65.7162652,154.728441 85.214043,163.704271 106.824743,163.704271 C128.435443,163.704271 147.933221,154.728441 161.903976,140.326388 L191.659972,140.326388 C191.659972,140.326388 184.047881,131.74984 183.701696,121.939514 L183.701696,121.939514 Z' id='Fill-19' sketch:type='MSShapeGroup'></path>
					</g>
				</g>
			</g>
		</svg>
		<div id='splash_template_title' class='splash_template_title'>Welcome to</div>
		<div id='splash_template_brand_name' class='splash_template_brand_name'>Brand Name</div>
		<input id='splash_template_passcode' name='splash_template_passcode' class='splash_template_passcode' value='' placeHolder='Please enter Passcode' type='text' maxlength='64' autocorrect='off' autocapitalize='off'>
		<div id='splash_template_terms_service' class='splash_template_terms_service'>
		<div class='splash_template_terms_service_cb'>
			<input type='checkbox' name='cbTermService' id='cbTermService' onclick='control_bt_status();'>
		</div>
		<div class='splash_template_terms_service_text'>
			I have read and agree to <a class='splash_template_terms_service_hyperlink' onclick='open_term_service();'>the Terms of Service</a>
		</div>
		<div style='clear:both;'></div>
		</div>
		<div id='splash_template_continue' class='splash_template_continue' onclick='continue_action();'>Continue</div>
		
	</div>
</div>
</body>
</html>