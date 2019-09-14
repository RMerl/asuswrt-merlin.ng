<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Transitional//EN' 'http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd'>
<html xmlns='http://www.w3.org/1999/xhtml'>
<html xmlns:v>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<meta http-equiv='X-UA-Compatible' content='IE=Edge'/>
<meta http-equiv='Content-Type' content='text/html; charset=utf-8' />
<meta HTTP-EQUIV='Pragma' CONTENT='no-cache'>
<meta HTTP-EQUIV='Expires' CONTENT='-1'>
<title><#FreeWiFi_title#></title>
<script type="text/javascript" src="js/jquery.js"></script>
<style>
.component_container {
	position: absolute;
	top: 0;
	right: 0;
	bottom: 0;
	left: 0;
}
p {
	margin-top: 0;
	margin-bottom: 0;
}
.component_background {
	width: 100%;
	height: 100%;
	position: absolute;
	top: 0;
	left: 0;
}
.terms_service_hyperlink {
	font-weight: bolder;
	text-decoration: underline;
	color: #00B0FF;
	cursor: pointer;
}
.terms_service {
	position: fixed;
	width: 80vw;
	height: 68vh;
	left: 7vw;
	top: 13vh;
	z-index: 200;
	padding: 3vh 3vw;
	border-radius: 2vw;
	display: none;
	box-shadow: 0 19px 38px rgba(0,0,0,0.30), 0 15px 12px rgba(0,0,0,0.22);
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
				re_tune_size();
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
			re_tune_size();
		}
	}, false);
}
window.onresize = function() {
	if(!isMobile()) {
		initial();
	}
};
function re_tune_size() {
	if(isMobile()) {
		$('meta[name=viewport]').attr('content', 'initial-scale=1.0,maximum-scale=1.0,minimum-scale=1.0,user-scalable=no,width=' + windw_width + ',height=' + windw_height);
		var mobile_ratio_size = ((windw_width / 340));
		document.getElementById('component_background').style.width = windw_width + 'px';
		//landscape
		if(windw_width > windw_height) {
			windw_width = windw_height;
			mobile_ratio_size =  ((windw_width / 340));
			windw_height = windw_width * 1.5;
		}
		document.getElementById('component_container').style.width = (340 * mobile_ratio_size) + 'px';
		document.getElementById('component_container').style.height = (510 * mobile_ratio_size) + 'px';
		document.getElementById('component_container').style.margin = '0 auto';
		
		var _component_background_height = ((510 * mobile_ratio_size) > windw_height) ? (510 * mobile_ratio_size) : windw_height;
		document.getElementById('component_background').style.height = _component_background_height + 'px';
		
		gen_component_html(mobile_ratio_size, mobile_ratio_size);
	}
	else {
		windw_width = window.innerWidth || document.documentElement.clientWidth || document.body.clientWidth;
		windw_height = window.innerHeight || document.documentElement.clientHeight || document.body.clientHeight;
		$('meta[name=viewport]').attr('content', 'initial-scale=1.0,maximum-scale=1.0,minimum-scale=1.0,user-scalable=no,width=' + windw_width + ',height=' + windw_height);
		document.getElementById('component_container').style.width = (340 * 1.4) + 'px';
		document.getElementById('component_container').style.height = (510 * 1.4) + 'px';
		document.getElementById('component_container').style.margin = 'auto';
		document.getElementById('component_background').style.width = windw_width + 'px';
		document.getElementById('component_background').style.height = windw_height + 'px';
		gen_component_html(1.4, 1.4);
	}
}
var splash_page_preview_array = new Array();
function initial() {
	splash_page_preview_array = [];
	splash_page_preview_array['component_image'] = opener.component_array['component_image'];
	splash_page_preview_array['component_text'] = opener.component_array['component_text'];
	splash_page_preview_array['component_eula'] = opener.component_array['component_eula'];
	splash_page_preview_array['component_account'] = opener.component_array['component_account'];
	splash_page_preview_array['component_button'] = opener.component_array['component_button'];
	splash_page_preview_array['component_background'] = opener.component_array['component_background'];
	
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

	re_tune_size();
}

function open_walled_garden(_url) {
	if(_url != '') {
		window.open('http://' + _url + '');
	}
}

function open_term_service() {
	if(isMobile()) {
		$('#terms_service').css('display', 'block');
		$('#component_container').css('display', 'none');
	}
	else {
		$('#terms_service').fadeIn(300);
		$('#component_container').fadeOut(100);
	}
}
function close_term_service() {
	if(isMobile()) {
		$('#terms_service').css('display', 'none');
		$('#component_container').css('display', 'block');
	}
	else {
		$('#terms_service').fadeOut(300);
		$('#component_container').fadeIn(100);
	}
}
function continue_action() {
	var valid_account_flag = false;
	if(splash_page_preview_array['component_account']) {
		valid_account_flag = true;
	}
	var valid_form = function() {
		var _default_account_username = decodeURIComponent(splash_page_preview_array['component_account'][0].account_username).replace(/\"/g, '&#34;').replace(/'/g, '&#39;').toLowerCase();
		var _default_account_password = decodeURIComponent(splash_page_preview_array['component_account'][0].account_password).replace(/\"/g, '&#34;').replace(/'/g, '&#39;').toLowerCase();
		_default_account_username = htmlEnDeCode.htmlDecode(_default_account_username);
		_default_account_password = htmlEnDeCode.htmlDecode(_default_account_password);
		if(document.getElementById('splash_page_account').value.trim() == '' || document.getElementById('splash_page_account').value.trim().toLowerCase() == _default_account_username) {
			document.getElementById('splash_page_account').value = '';
			document.getElementById('splash_page_account').focus();
			alert('<#JS_fieldblank#>');
			return false;
		}
		if(document.getElementById('splash_page_password').value.trim() == '' || document.getElementById('splash_page_password').value.trim().toLowerCase() == _default_account_password) {
			document.getElementById('splash_page_password').value = '';
			document.getElementById('splash_page_password').focus();
			alert('<#JS_fieldblank#>');
			return false;
		}
		return true;
	};

	if(splash_page_preview_array['component_eula']) {
		var _obj = document.getElementById('eula_check');
		if(_obj.checked) {
			if(valid_account_flag) {
				if(valid_form())
					console.log("formSubmit(1)");
			}
			else {
				console.log("formSubmit(0)");
			}
		}
		else {
			alert("You must agree to the terms of service before continuing.");
		}
	}
	else {
		if(valid_account_flag) {
			if(valid_form())
				console.log("formSubmit(1)");
		}
		else {
			console.log("formSubmit(0)");
		}
	}
}
function simulate_placeholder_focus(_obj, _type, _fontColor) {
	if(!_obj.haswriting) {
		_obj.type = _type;
		_obj.style.fontStyle = 'initial';
		_obj.style.fontWeight = 'bolder';
		_obj.style.color = _fontColor;
		_obj.value = '';
	}
}
function simulate_placeholder_blur(_obj, _type, _fontColor, _placeholderText) {
	if(!_obj.value) {
		_obj.type = 'text';
		_obj.style.color = transformHEXtoRGB(_fontColor).replace(')', ', 0.8)').replace('rgb', 'rgba');
		_obj.style.fontStyle = 'oblique';
		_obj.style.fontWeight = 'initial';
		_obj.value = decodeURIComponent(_placeholderText);
		_obj.haswriting = false;
	}
	else {
	 _obj.haswriting = true;
	}
}
function gen_component_obj(_component_attr_array, _ratio_width, _ratio_height, _component_type) {
	var font_size = (13 * _ratio_width) + 'px';
	var _component_width = (parseInt(_component_attr_array.attribute.style_width) * _ratio_width) + 'px';
	var _component_height = (parseInt(_component_attr_array.attribute.style_height) * _ratio_height) + 'px';
	var _component_top = (parseInt(_component_attr_array.attribute.style_top) * _ratio_height) + 'px';
	var _component_left = (parseInt(_component_attr_array.attribute.style_left) * _ratio_width) + 'px';
	var _component_zIndex = _component_attr_array.attribute.style_z_index;
	var _component_backgroundColor = transformRGBtoHEX(_component_attr_array.attribute.style_background_color);
	var _component_opacity = _component_attr_array.attribute.style_opacity;
	var _component_color = transformRGBtoHEX(_component_attr_array.attribute.style_color);
	var _component_common_attr = 'position:absolute;width:' + _component_width + ';height:' + _component_height + ';top:' + _component_top + ';left:' + _component_left + ';z-index:' + _component_zIndex + ';opacity:' + _component_opacity + ';color:' + _component_color + ';';

	var code = '';
	switch(_component_type) {
		case 'image' :
			var image_walled_garden = "";
			if(_component_attr_array.image_walled_garden) {
				image_walled_garden = decodeURIComponent(_component_attr_array.image_walled_garden);
			}
			var image_cursor = (image_walled_garden == "") ? "default" : "pointer";
			if(_component_attr_array.image_type == 'image') {
				code += '<div style=\'' + _component_common_attr + 'background-image:url(' + _component_attr_array.image_base64 + ');background-repeat:no-repeat;background-size:' + _component_width + ' ' + _component_height + ';border-radius:8px;cursor:' + image_cursor + ';\' onclick=\'open_walled_garden(\"' + image_walled_garden + '\");\'></div>';
			}
			else {
				code += '<div style=\'' + _component_common_attr + 'background-color:' + _component_backgroundColor + ';border-radius:8px;cursor:' + image_cursor + ';\' onclick=\'open_walled_garden(\"' + image_walled_garden + '\");\'></div>';
			}
			break;
		case 'text' :
			var text_font_weight = _component_attr_array.text_font_weight;
			var text_font_style =  _component_attr_array.text_font_style;
			var text_font_family =  _component_attr_array.text_font_family;
			var text_font_size = (parseInt(_component_attr_array.text_font_size) * _ratio_width) + 'px';
			var text_walled_garden = "";
			if(_component_attr_array.text_walled_garden) {
				text_walled_garden = decodeURIComponent(_component_attr_array.text_walled_garden);
			}
			var text_decoration = (text_walled_garden == "") ? "none" : "underline";
			var text_cursor = (text_walled_garden == "") ? "default" : "pointer";
			code += '<div style=\'' + _component_common_attr + ';overflow:auto;font-size:' + text_font_size + ';font-weight:' + text_font_weight  + ';font-style:' + text_font_style + ';font-family:' + text_font_family + ';\'>';
			var text_content = _component_attr_array.text_content;
			text_content = decodeURIComponent(text_content);
			text_content = text_content.replace(/\<br\>/g, '\n');
			text_content = htmlEnDeCode.htmlEncode(text_content);
			text_content = text_content.replace(/ /g, '&nbsp;').replace(/(?:\r\n|\r|\n)/g, '<br>');
			code +='<div style=\'text-decoration:' + text_decoration + ';cursor:' + text_cursor + ';\' onclick=\'open_walled_garden(\"' + text_walled_garden + '\");\'>';
			code += text_content;
			code +='</div>';
			code +='</div>';
			break;
		case 'account' :
			code += '<div style=' + _component_common_attr + 'font-size:' + font_size + ';>';
			var _account_input_box_background_color = transformHEXtoRGB("#" + _component_attr_array.account_input_box_background_color);
			var _account_input_box_opacity = _component_attr_array.account_input_box_opacity;
			_account_input_box_background_color = _account_input_box_background_color.replace(')', ', ' + _account_input_box_opacity + ')').replace('rgb', 'rgba');

			var _account_font_color = '#' + _component_attr_array.account_input_box_font_color;
			var _account_font_color_rgba = transformHEXtoRGB(_account_font_color).replace(')', ', 0.8)').replace('rgb', 'rgba');
	
			var _account_height = Math.floor(parseInt(_component_height) / 2) + 'px';
			var _account_imput_height = (Math.floor(parseInt(_component_height) / 2) - 10) + 'px';
			var _account_username_ASCII = _component_attr_array.account_username;
			var _account_password_ASCII = _component_attr_array.account_password;
			var _account_username = htmlEnDeCode.htmlEncode(decodeURIComponent(_account_username_ASCII));
			var _account_password = htmlEnDeCode.htmlEncode(decodeURIComponent(_account_password_ASCII));
			var _common_css = 'padding-left:2%;width:98%;border:none;border-radius:4px;font-style:oblique;background-color:' + _account_input_box_background_color + ';height:' + _account_imput_height + ';font-size:' + font_size + ';color:' + _account_font_color_rgba + '';
			var _margin_top = (6 * _ratio_width) + 'px';

			code += '<input id=\'splash_page_account\' style=\'' + _common_css + '\'; type=text maxlength=64 autocorrect=off autocapitalize=off value=\'' + _account_username + '\' onfocus=simulate_placeholder_focus(this,\'text\',\'' + _account_font_color + '\'); onblur=simulate_placeholder_blur(this,\'text\',\'' + _account_font_color + '\',\'' + _account_username_ASCII + '\'); >';
			code += '<input id=\'splash_page_password\' style=\'' + _common_css + ';margin-top:' + _margin_top + '\'; type=text maxlength=64 autocorrect=off autocapitalize=off value=\'' + _account_password + '\' onfocus=simulate_placeholder_focus(this,\'password\',\'' + _account_font_color + '\'); onblur=simulate_placeholder_blur(this,\'password\',\'' + _account_font_color + '\',\'' + _account_password_ASCII + '\'); >';
			code += '</div>';
			break;
		case 'eula' :
			var eula_text = 'I have read and agree to';
			var eula_hyperlink = 'the Terms of Service';
			var eula_hyperlink_color = '#00B0FF';
			if(_component_attr_array.eula_terms_service_text != undefined)
				eula_text = _component_attr_array.eula_terms_service_text;
			if(_component_attr_array.eula_terms_service_hyperlink != undefined)
				eula_hyperlink = _component_attr_array.eula_terms_service_hyperlink;
			if(_component_attr_array.eula_terms_service_hyperlink_color != undefined)
				eula_hyperlink_color = _component_attr_array.eula_terms_service_hyperlink_color;
			code += '<div style=' + _component_common_attr + 'font-size:' + font_size + ';>';
			code += '<div style=\'float:left;\'>';
			code += '<input type=\'checkbox\' name=\'eula_check\' id=\'eula_check\' style=\'height:' + font_size + ';width:' + font_size + ';\' onclick=control_bt_status();>';
			code += '</div>';
			code += '<div>';
			code += '<span>' + htmlEnDeCode.htmlEncode(decodeURIComponent(eula_text)) + '</span>&nbsp;<span class=\'terms_service_hyperlink\' style=\'color:' + eula_hyperlink_color + ';\' onclick=\'open_term_service();\'>' + htmlEnDeCode.htmlEncode(decodeURIComponent(eula_hyperlink)) + '</span>';
			code += '</div>';
			code += '</div>';
			break;
		case 'button' :
			code += '<div id=\'component_button\' style=\'' + _component_common_attr + 'cursor:pointer;text-align:center;font-size:' + font_size + ';line-height:' + _component_height + ';background-color:' + _component_backgroundColor + ';border-radius:8px;\' onclick=\'continue_action();\'>';
			code += '' + htmlEnDeCode.htmlEncode(decodeURIComponent(_component_attr_array.button_label)) + '';
			code += '</div>';
			break;
	}
	return code;
}

function gen_component_html(_ratio_width, _ratio_height) {
	document.getElementById('component_container').innerHTML = '';
	var _component_attr_array = [];

	//image
	if(splash_page_preview_array['component_image']) {
		var _component_obj_length = Object.keys(splash_page_preview_array['component_image']).length;
		for(var i = 0; i < _component_obj_length; i += 1) {
			var _idx = Object.keys(splash_page_preview_array['component_image'])[i];
			_component_attr_array = splash_page_preview_array['component_image'][_idx];
			document.getElementById('component_container').innerHTML += gen_component_obj(_component_attr_array, _ratio_width, _ratio_height, 'image');
		}
	}
	//text
	if(splash_page_preview_array['component_text']) {
		var _component_obj_length = Object.keys(splash_page_preview_array['component_text']).length;
		for(var i = 0; i < _component_obj_length; i += 1) {
			var _idx = Object.keys(splash_page_preview_array['component_text'])[i];
			_component_attr_array = splash_page_preview_array['component_text'][_idx];
			document.getElementById('component_container').innerHTML += gen_component_obj(_component_attr_array, _ratio_width, _ratio_height, 'text');
		}
	}
	//eula
	if(splash_page_preview_array['component_eula']) {
		_component_attr_array = splash_page_preview_array['component_eula'][0];
		document.getElementById('component_container').innerHTML += gen_component_obj(_component_attr_array, _ratio_width, _ratio_height, 'eula');
		document.getElementById('terms_service').style.color = _component_attr_array.eula_terms_service_font_color;
		document.getElementById('terms_service').style.backgroundColor = _component_attr_array.eula_terms_service_background_color;
		var code = '';
		code += '<div style="display:table;width:100%;height:100%;border-collapse:collapse;">';
		code += '<div style="display:table-row;">';
		code += '<div class="term_service_title" style="display:table-cell;vertical-align:middle;font-size:' + (13 * _ratio_width) + 'px;" >';
		var eula_terms_service_title =_component_attr_array.eula_terms_service_title;
		eula_terms_service_title = decodeURIComponent(eula_terms_service_title);
		eula_terms_service_title = eula_terms_service_title.replace(/\<br\>/g, '\n');
		eula_terms_service_title = htmlEnDeCode.htmlEncode(eula_terms_service_title);
		eula_terms_service_title = eula_terms_service_title.replace(/ /g, '&nbsp;').replace(/(?:\r\n|\r|\n)/g, '<br>');
		code += eula_terms_service_title;
		code += '</div>';
		code += '</div>';
		code += '<div style="display:table-row;height:90%;">';
		code += '<div style="display:table-cell;">';
		code += '<div style="width:100%;max-width:100%;height:100%;max-height:100%;position:relative;">';
		code += '<div class="term_service_text" style="font-size:' + (12 * _ratio_width) + 'px;">';
		var eula_terms_service =_component_attr_array.eula_terms_service;
		eula_terms_service = decodeURIComponent(eula_terms_service);
		eula_terms_service = eula_terms_service.replace(/\<br\>/g, '\n');
		eula_terms_service = htmlEnDeCode.htmlEncode(eula_terms_service);
		eula_terms_service = eula_terms_service.replace(/ /g, '&nbsp;').replace(/(?:\r\n|\r|\n)/g, '<br>');
		code += eula_terms_service;
		code += '</div>';
		code += '</div>';
		code += '</div>';
		code += '</div>';
		code += '</div>';
		code += '<div class="terms_service_close" onclick="close_term_service();">';
		code += '<svg version="1.1" id="terms_service_close_icon" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" x="0px" y="0px" viewBox="0 0 64 64" enable-background="new 0 0 64 64" xml:space="preserve">';
		code += '<line fill="none" stroke="' + _component_attr_array.eula_terms_service_font_color + '" stroke-width="3" stroke-linecap="round" stroke-linejoin="round" stroke-miterlimit="10" x1="21.1" y1="21.1" x2="42.9" y2="42.9"/>';
		code += '<line fill="none" stroke="' + _component_attr_array.eula_terms_service_font_color + '" stroke-width="3" stroke-linecap="round" stroke-linejoin="round" stroke-miterlimit="10" x1="42.9" y1="21.1" x2="21.1" y2="42.9"/>';
		code += '<circle fill="none" stroke="' + _component_attr_array.eula_terms_service_font_color + '" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round" stroke-miterlimit="10" cx="32" cy="32.1" r="25.2"/>';
		code += '</svg>';
		code += '</div>';
		$('#terms_service').html(code);
	}
	//account
	if(splash_page_preview_array['component_account']) {
		_component_attr_array = splash_page_preview_array['component_account'][0];
		document.getElementById('component_container').innerHTML += gen_component_obj(_component_attr_array, _ratio_width, _ratio_height, 'account');
	}
	//button
	if(splash_page_preview_array['component_button']) {
		_component_attr_array = splash_page_preview_array['component_button'][0];
		document.getElementById('component_container').innerHTML += gen_component_obj(_component_attr_array, _ratio_width, _ratio_height, 'button');
	}
	//background
	if(splash_page_preview_array['component_background']) {
		_component_attr_array = splash_page_preview_array['component_background'][0];
		document.getElementById('component_background').style.opacity = _component_attr_array.attribute.style_opacity;
		if(_component_attr_array.image_type == 'image') {
			document.getElementById('component_background').style.backgroundImage = 'url(' + _component_attr_array.image_base64 + ')';
			document.getElementById('component_background').style.backgroundRepeat = 'no-repeat';
			document.getElementById('component_background').style.backgroundAttachment = 'fixed';
			document.getElementById('component_background').style.backgroundSize = 'cover';
			document.getElementById('component_background').style.backgroundPosition = '50% 50%';
		}
		else {
			document.getElementById('component_background').style.backgroundColor =  _component_attr_array.attribute.style_background_color;
		}
	}
	//control button
	if(splash_page_preview_array['component_eula']) {
		control_bt_status();
	}
}
function control_bt_status() {
	var _obj = document.getElementById('eula_check');
	if(_obj.checked) {
		document.getElementById('component_button').style.opacity = 1;
	}
	else {
		document.getElementById('component_button').style.opacity = 0.5;
	}
}
function isMobile() {
	var userAgentString = navigator.userAgent.toLowerCase();
	var mobile = ['iphone','ipad','ipod','android','blackberry','nokia','opera mini','windows mobile','windows phone','iemobile','mobile safari','bb10; touch', 'tablet'];
	for (var i in mobile) if (userAgentString.indexOf(mobile[i]) > 0) return true;
	return false;
}
function transformRGBtoHEX(_rgb) {
	var rgb_array = _rgb.replace(/\s/g,'').replace('rgb', '').replace('(', '').replace(')', '').split(',');
	var hex = (rgb_array && rgb_array.length === 3) ? '#' +
	('0' + parseInt(rgb_array[0],10).toString(16)).slice(-2) +
	('0' + parseInt(rgb_array[1],10).toString(16)).slice(-2) +
	('0' + parseInt(rgb_array[2],10).toString(16)).slice(-2) : '#FFFFFF';
	return hex;
}
function transformHEXtoRGB(_hex) {
	var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(_hex);
	return result ? 'rgb(' + parseInt(result[1], 16) + ', ' + parseInt(result[2], 16) + ', ' + parseInt(result[3], 16) + ')' : 'rgb(255, 255, 255)';
}
</script>
</head>
<body id='component_body' onload='initial();' class="bg" style='overflow-x:hidden;overflow-y:auto;font-family:Arial,Helvetica,sans-serif;'>
<div id='terms_service' class='terms_service'></div>
<div id='component_container' class='component_container'></div>
<div id='component_background' class='component_background'></div>
</body>
</html>

