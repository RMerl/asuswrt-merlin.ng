<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - Free Wi-Fi<!--untranslated--></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="other.css">
<link rel="stylesheet" type="text/css" href="Captive_Portal.css">
<script type="text/javascript" src="state.js"></script>
<script type="text/javascript" src="general.js"></script>
<script type="text/javascript" src="popup.js"></script>
<script type="text/javascript" src="help.js"></script>
<script type="text/javascript" src="validator.js"></script>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="form.js"></script>
<script type="text/javascript" src="disk_functions.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script type="text/javascript" src="/Captive_Portal_template.js"></script>
<script type="text/javascript" src="/js/httpApi.js"></script>
<script>
var disk_flag = true;
var splash_image_base64 = "";
var setting_profile_id = "";
var captive_portal = decodeURIComponent('<% nvram_char_to_ascii("","captive_portal"); %>');
var captive_portal_array = new Array();
var captive_portal_wl_idx = "off";
var captive_portal_enable = '<% nvram_get("captive_portal_enable"); %>';
window.onresize = function() {
	if(document.getElementById("guestnetwork_wl").style.display == "block") {
		cal_panel_block("guestnetwork_wl", 0.25);
	}
}

function initial(){
	show_menu();

	//detect usb
	var initial_dir_status = function(data) {
		if(data == "" || data.length == 2){	
			disk_flag = false;
		}
	}
	var __layer_order = "0_0";
	var url = "/getfoldertree.asp";
	url += "?motion=gettree&layer_order=" + __layer_order + "&t=" + Math.random();
	$.get(url,function(data){initial_dir_status(data);});

	gen_splash_page();
	gen_line();

	var captive_portal_row = captive_portal.split("<");
	for(var i = 0; i < captive_portal_row.length; i += 1) {
		if(captive_portal_row[i] != "") {
			var captive_portal_col = captive_portal_row[i].split(">");
			for(var j = 0; j < captive_portal_col.length; j += 1) {
				if(j == 0) {
					captive_portal_array[captive_portal_col[j]] = [];
					setting_profile_id = captive_portal_col[j];
				}
				else {
					captive_portal_array[captive_portal_col[0]].push(captive_portal_col[j])
				}
			}
		}
	}

	document.form.captive_portal.value = captive_portal;

	initial_landing_setting();

	if(captive_portal_enable == "on") {
		captivePortalShowAndHide(1);
		$("#apply_button").val('<#CTL_apply#>');
	}
	else {
		captivePortalShowAndHide(0);
		$("#apply_button").val('<#CTL_Apply_Enable#>');
	}

	var series = productid.split("-")[0].toUpperCase();
	if(series == "BRT")
		httpApi.faqURL("1034971", function(url){document.getElementById("faq").href=url;});
	else
		$(".brt_series").remove();
}
function captivePortalShowAndHide(_flag) {
	if(_flag == 1) {
		captive_portal_enable = "on";
	}
	else {
		captive_portal_enable = "off";
	}
}
function get_customized_attribute_landing_page(_profile_id) {
	var result = "NoData";
	$.ajax({
		url: '/ajax_captive_portal.asp?profile_id=' + _profile_id,
		dataType: 'json',
		error: function(xhr){
			setTimeout("get_customized_attribute_landing_page('" + _profile_id + "');", 1000);
		},
		success: function(response) {
			if(response.splash_page_setting) {
				var image = response.splash_page_setting.image;
				var image_size = response.splash_page_setting.image_size;
				$("#splash_image_default").css({"display" : "none"});
				$('#splash_image_canvas_content').css('background-image', 'url(' + image + ')');
				if(image_size) {
					$("#splash_image_size").val(image_size);
					var backgroundSize = "250%";
					backgroundSize = (image_size == "center") ? "250%" : "100% 100%" ;
					$("#splash_image_canvas_content").css("background-size", backgroundSize);
				}
				$("#splash_image_canvas_content").css("display", "");
				splash_image_base64 = image;

				var terms_service = response.splash_page_setting.terms_service;
				$('textarea#terms_service').val(encode_decode_text(terms_service, "decode"));
			}
		}
	});
}
function initial_landing_setting() {
	if(setting_profile_id != "") {
		get_customized_attribute_landing_page(setting_profile_id);
		$("input[name=brand_name]").val(captive_portal_array[setting_profile_id][0]);
		$("#splash_template_brand_name").html(captive_portal_array[setting_profile_id][0]);
		$("input[name=session_length]").val(captive_portal_array[setting_profile_id][1]);
		$("input[name=internet_website]").val(captive_portal_array[setting_profile_id][3]);
		if(captive_portal_array[setting_profile_id][5] == "1") {
			$("#cb_terms_service").prop("checked", true);
		}
		else {
			$("#cb_terms_service").prop("checked", false);
		}

		var passcode_value = decodeURIComponent('<% nvram_char_to_ascii("", "captive_portal_passcode"); %>');
		$("input[name=passcode]").val(passcode_value);
		if(captive_portal_array[setting_profile_id][6] == "1") {
			$("#cb_passcode").prop("checked", true);
		}
		else {
			$("#cb_passcode").prop("checked", false);
		}

		var set_checkbox_status = function(_unit, _status) {
			if($("input[name=cb_wl_" + _unit + "]").length) {
				$("#cb_wl_" + _unit + "").prop("checked", _status);
			}
		};

		set_checkbox_status(0, false);
		change_wl_input_status(0);
		$("input[name=wl_0]").val(decodeURIComponent('<% nvram_char_to_ascii("", "captive_portal_2g"); %>'));
		if(wl_info.band5g_support) {
			set_checkbox_status(1, false);
			change_wl_input_status(1);
			$("input[name=wl_1]").val(decodeURIComponent('<% nvram_char_to_ascii("", "captive_portal_5g"); %>'));
		}
		if(wl_info.band5g_2_support) {
			set_checkbox_status(2, false);
			change_wl_input_status(2);
			$("input[name=wl_2]").val(decodeURIComponent('<% nvram_char_to_ascii("", "captive_portal_5g_2"); %>'));
		}
		var wl_if_array = captive_portal_array[setting_profile_id][4].split("wl");
		for(var i = 0; i < wl_if_array.length; i += 1) {
			if(wl_if_array[i] != "") {
				var wl_band = wl_if_array[i].split(".")[0];
				captive_portal_wl_idx = wl_if_array[i].split(".")[1];
				if($("input[name=cb_wl_" + wl_band + "]").length) {
					set_checkbox_status(wl_band, true);
					change_wl_input_status(wl_band);
				}
			}
		}

		if(captive_portal_array[setting_profile_id][7] != undefined)
			$("input[name=bandwidth_limiter_dl]").val(captive_portal_array[setting_profile_id][7]/1024);
		if(captive_portal_array[setting_profile_id][8] != undefined)
			$("input[name=bandwidth_limiter_ul]").val(captive_portal_array[setting_profile_id][8]/1024);

	}
	else {
		var template_icon = template_1;
		$("#splash_image_default").css({"display" : "none"});
		$('#splash_image_canvas_content').css('background-image', 'url(' + template_icon + ')');
		$("#splash_image_canvas_content").css("display", "");
		splash_image_base64 = template_icon;
	}
	update_terms_service();
	update_passcode();
}
function gen_guestnetwork_wl() {
	var code = "";

	var wl_vifnames = '<% nvram_get("wl_vifnames"); %>';
	var gn_count = wl_vifnames.split(" ").length;
	var gn_interface = "";

	var gen_band_content = function(_wl_idx) {
		var content_width = (wl_info.band5g_2_support) ? "33%" : (wl_info.band5g_support && !no5gmssid_support) ? "50%" : "100%";
		var band_code = "";

		var gen_each_band = function(_wl_idx, _gn_array, _band_idx) {
			var each_band_code = "";
			each_band_code += "<div class='gn_band_title'>" + wl_nband_title[_band_idx] + "</div>";
			each_band_code += "<div class='gn_line'></div>";
			if(_gn_array[_wl_idx][0] == "1") {
				each_band_code += "<div class='gn_item_title'><#QIS_finish_wireless_item1#></div>";
				var show_str = decodeURIComponent(_gn_array[_wl_idx][1]);
				show_str = handle_show_str(show_str);
				each_band_code += "<div class='gn_item_value'>" + show_str + "</div>";
			}
			else {
				each_band_code += "<div class='gn_item_unenabled'><#Status_Inactive#></div>";
			}
			return each_band_code;
		}

		band_code += "<div style='float:left;width:" + content_width + "'>";
			band_code += gen_each_band(_wl_idx, gn_array_2g, "0");
		band_code += "</div>";
		if(wl_info.band5g_support) {
			band_code += "<div style='float:left;width:" + content_width + "'>";
				band_code += gen_each_band(_wl_idx, gn_array_5g, "1");
			band_code += "</div>";
		}
		if(wl_info.band5g_2_support) {
			band_code += "<div style='float:left;width:" + content_width + "'>";
				band_code += gen_each_band(_wl_idx, gn_array_5g_2, "2");
			band_code += "</div>";
		}
		band_code += "<div style='clear:both;'></div>";
		return band_code;
	};

	code += "<div class='gn_group_hint'>";
	code += "Because each group of Guestnetwork be enabled, please select the group want to use.";
	code += "</div>";

	third_party_wl_used = create_third_party_wl_used();
	for(var wl_idx = 0; wl_idx < gn_count; wl_idx += 1) {
		code += "<div id='gn_content_" + wl_idx + "' class='gn_content' onclick='setting_gn_group(\"" + wl_idx + "\");'>";
			if(third_party_wl_used[(wl_idx + 1)] != "not_used")
				code += "<div class='gn_group_title'><#Guest_Network#> - " + (wl_idx + 1) + " - Used by " + third_party_wl_used[(wl_idx + 1)] + "</div>";
			else 
				code += "<div class='gn_group_title'><#Guest_Network#> - " + (wl_idx + 1) + "</div>";
			code += "<div class='gn_line'></div>";
			code += "<div>";
			code += gen_band_content(wl_idx);
			code += "</div>";
		code += "</div>";
	}

	code += "<div class='gn_finish_bg'>";
	code += "<input class='button_gen' onclick='cancel_gn_group();' type='button' value='<#CTL_Cancel#>'/>";
	code += "<input class='button_gen' onclick='save_gn_group();' type='button' value='<#CTL_apply#>'/>";
	code += "</div>";

	$('#guestnetwork_wl').html(code);
}
function cancel_gn_group() {
	$("#full_screen_bg").fadeOut();
	$('#guestnetwork_wl').fadeOut();
}
function setting_gn_group(_idx) {
	var wl_vifnames = '<% nvram_get("wl_vifnames"); %>';
	var gn_count = wl_vifnames.split(" ").length;
	if(third_party_wl_used[parseInt(_idx) + 1] == "not_used") {
		for(var wl_idx = 0; wl_idx < gn_count; wl_idx += 1) {
			if(_idx == wl_idx)
				$('#gn_content_' + wl_idx + '').attr("class", "gn_content gn_content_clicked");
			else
				$('#gn_content_' + wl_idx + '').attr("class", "gn_content");
		}
	}
	else {
		alert("Please select other group, because this used by" + third_party_wl_used[_idx + 1]);
	}
}
function save_gn_group() {
	var wl_vifnames = '<% nvram_get("wl_vifnames"); %>';
	var gn_count = wl_vifnames.split(" ").length;
	var gn_idx = "";
	for(var wl_idx = 0; wl_idx < gn_count; wl_idx += 1) {
		if($("#gn_content_" + wl_idx + "").hasClass("gn_content_clicked")) {
			gn_idx = (wl_idx + 1);
			break;
		}
	}
	if(gn_idx == "") {
		alert("Please select one group");
		return false;
	}
	if($("#cb_wl_0").prop("checked")) {
		document.form.captive_portal_2g_if.value = "wl0." + gn_idx;
	}
	if(wl_info.band5g_support) {
		if($("#cb_wl_1").prop("checked")) {
			document.form.captive_portal_5g_if.value = "wl1." + gn_idx;
		}
	}
	if(wl_info.band5g_2_support) {
		if($("#cb_wl_2").prop("checked")) {
			document.form.captive_portal_5g_2_if.value = "wl2." + gn_idx;
		}
	}
	save_splash_page_content();
}
function auto_fill_wl_name() {
	var brand_name = $("input[name=brand_name]").val();
	if(brand_name.length > 22) {
		if(wl_info.band2g_support) {
			$("input[name=wl_0]").val(brand_name);
		}
		if(wl_info.band5g_support) {
			$("input[name=wl_1]").val(brand_name);
		}
		if(wl_info.band5g_2_support) {
			$("input[name=wl_2]").val(brand_name);
		}
	}
	else {
		var brand_name_temp = "";
		if(wl_info.band2g_support) {
			brand_name_temp = brand_name + "-Free WiFi";
			$("input[name=wl_0]").val(brand_name_temp);
		}
		if(wl_info.band5g_support) {
			brand_name_temp = brand_name.substring(0,19) + "-Free WiFi_5G";
			$("input[name=wl_1]").val(brand_name_temp);
		}
		if(wl_info.band5g_2_support) {
			brand_name_temp = brand_name.substring(0,17) + "-Free WiFi_5G-2";
			$("input[name=wl_2]").val(brand_name_temp);
		}
	}
	$("#splash_template_brand_name").html(brand_name);
}
function splash_upload_image() {
	$("#splash_upload_file").click();
}
function previewSplashImage(_obj) {
	var checkImageExtension = function (imageFileObject) {
	var  picExtension= /\.(jpg|jpeg|gif|png|bmp|ico|svg)$/i;  //analy extension
		if (picExtension.test(imageFileObject)) 
			return true;
		else
			return false;
	};
	//1.check image extension
	if (!checkImageExtension(_obj.value)) {
		alert("<#Setting_upload_hint#>");
		_obj.focus();
	}
	else {
		//2.Re-drow image
		var fileReader = new FileReader(); 
		fileReader.onload = function (fileReader) {
			var source_image_size = 0;
			if( (fileReader.total != undefined) && (!isNaN(fileReader.total)) )
				source_image_size = fileReader.total;
			if(Math.round(source_image_size / 1024) > 10240) {
				alert("<#FreeWiFi_Image_Size_Alert#>");
				return false;
			}

			var img = document.createElement("img");
			img.src = fileReader.target.result;
			var mimeType = img.src.split(",")[0].split(":")[1].split(";")[0];
			var canvas = document.getElementById("splash_canvas");
			var ctx = canvas.getContext("2d");
			ctx.clearRect(0, 0, 1152, 864);
			setTimeout(function() {
				ctx.drawImage(img, 0, 0, 1152, 864);
				var dataURL = canvas.toDataURL(mimeType);
				if(Math.round(dataURL.length / 1024) > 2048) {
					alert("<#FreeWiFi_Image_Size_Compressed_Alert#>");
					return false;
				}
				else {
					splash_image_base64 = dataURL;
					$("#splash_image_default").css({"display" : "none"});
					$('#splash_image_canvas_content').css('background-image', 'url(' + splash_image_base64 + ')');
					$("#splash_image_canvas_content").css("display", "");
				}
			}, 100); //for firefox FPS(Frames per Second) issue need delay
		}
		fileReader.readAsDataURL(_obj.files[0]);
	}
}
function preview_splash_page() {
	window.open('Captive_Portal_Preview.asp', 'customized', 'toolbar=no,scrollbars=no,resizable=yes,location=no,menubar=no,width=476,height=714');
}
function update_terms_service() {
	if($("#cb_terms_service").prop("checked")) {
		$("#splash_template_terms_service").css("display", "");
	}
	else {
		$("#splash_template_terms_service").css("display", "none");
	}
}
function update_passcode() {
	if($("#cb_passcode").prop("checked")) {
		$("#splash_template_passcode").css("display", "");
	}
	else {
		$("#splash_template_passcode").css("display", "none");
	}
}
function splash_image_size_change() {
	var size_type = $("#splash_image_size").val();
	switch(size_type) {
		case "center" :
			$("#splash_image_canvas_content").css("background-size", "250%");
			break;
		case "extend" :
			$("#splash_image_canvas_content").css("background-size", "100% 100%");
			break;
	}
}
function gen_splash_page() {
	var gen_wl_interface = function(_wl_idx) {
		
		code += "<div class='splash_item_content'>";
		code += "<div class='splash_item_title'>" + wl_nband_title[_wl_idx] + " <#QIS_finish_wireless_item1#></div>";
		code += "<input type='checkbox' name='cb_wl_" + _wl_idx + "' id='cb_wl_" + _wl_idx + "' onclick='change_wl_input_status(" + _wl_idx + ");' checked>";
		if(_wl_idx == "1")
			code += "<input name='wl_" + _wl_idx + "' class='input_25_table' value='Brand Name-Free WiFi_5G' type='text' maxlength='32' autocorrect='off' autocapitalize='off'>";
		else if(_wl_idx == "2")
			code += "<input name='wl_" + _wl_idx + "' class='input_25_table' value='Brand Name-Free WiFi_5G-2' type='text' maxlength='32' autocorrect='off' autocapitalize='off'>";
		else 
			code += "<input name='wl_" + _wl_idx + "' class='input_25_table' value='Brand Name-Free WiFi' type='text' maxlength='32' autocorrect='off' autocapitalize='off'>";

		code += "</div>";
	};
		

	code = "";

	code += "<div class='cp_item_title'><#FreeWiFi_title#></div>";

	code += "<div class='splash_content'>";
		code += "<div class='splash_image_size_content'>";
		code += "<select id='splash_image_size' name='splash_image_size' class='input_option' onchange='splash_image_size_change();'>";
		code += "<option value='center'><#FreeWiFi_center#></option>";
		code += "<option value='extend'><#FreeWiFi_Extend#></option>";
		code += "</select>";
		code += "</div>";
		code += "<div id='splash_image_conent' class='splash_image_conent'>";
			if(isSupportFileReader() && isSupportCanvas()) {
				code += "<div id='splash_image_default' class='splash_image_default' onclick='splash_upload_image();' title='<#FreeWiFi_Upload_Image#>'>";
				code += "<div class='splash_image_text'><#FreeWiFi_Upload_Image#></div>";
			}
			else {
				code += "<div id='splash_image_default' class='splash_image_default'>";
				code += "<div class='splash_image_text'>Your browser do not support this function.</div>";/*untranslated*/
			}
				code += "</div>";

			code += "<div id='splash_image_canvas_content' class='splash_image_canvas_content' style='display:none;' onclick='preview_splash_page();' title='<#FreeWiFi_Upload_Image#>'>";
				code += "<canvas id='splash_canvas' width='1152px;' height='864px;' style='display:none;'></canvas>";
				code += "<div id='splash_template_content' class='splash_template_content'>";
					code += "<div class='splash_template_icon'>";
					code += "</div>";
					code += "<div class='splash_template_title'><#FreeWiFi_Welcome#>";
					code += "</div>";
					code += "<div id='splash_template_brand_name' class='splash_template_brand_name'>Brand Name";
					code += "</div>";
					code += "<input id='splash_template_passcode' name='splash_template_passcode' class='splash_template_passcode' value='Please enter Passcode' type='text' maxlength='64' autocorrect='off' autocapitalize='off' disabled=true;>";
					code += "<div id='splash_template_terms_service' class='splash_template_terms_service'>";
						code += "<div style='width: 15%;float: left;'>";
							code += "<input type='checkbox' checked disabled>";
						code += "</div>";
						code += "<div style='width: 85%;float: left;'>";
							code += "<#FreeWiFi_Agree_Terms_Service#>";
						code += "</div>";
						code += "<div style='clear:both;'></div>";
					code += "</div>";
					code += "<div class='splash_template_continue'><#FreeWiFi_Continue#></div>";
				code += "</div>";
			code += "</div>";
		code += "</div>";

		code += "<div class='splash_image_hint'><#FreeWiFi_ImageSize#>: < 10MB</div>";
		code += "<div class='splash_image_hint'><#FreeWiFi_RecommendType#>: jpg, png</div>";/*untranslated*/
		code += "<div class='splash_image_hint'><#FreeWiFi_RecommendResolution#>: 1152 x 864 px or above</div>";/*untranslated*/
		if(isSupportFileReader() && isSupportCanvas()) {
			code += "<div class='splash_preview'>";
			code += "<input class='button_gen' onclick='splash_upload_image();' type='button' value='<#btn_Background#>' style='margin: 0 5px'/>";
			code += "<input class='button_gen' onclick='preview_splash_page();' type='button' value='<#btn_Preview#>' style='margin: 0 5px'/>";
			code += "</div>";
			code += "<input type='file' name='splash_upload_file' id='splash_upload_file' class='splash_upload_file' onchange='previewSplashImage(this);'/>";
		}

	code += "</div>";

	code += "<div class='splash_content'>";
		code += "<div class='splash_item_content'>";
			code += "<div class='splash_item_title'><#FreeWiFi_BrandName#></div>";
			code += "<input name='brand_name' class='input_25_table' value='Brand Name' type='text' maxlength='32' autocorrect='off' autocapitalize='off' onkeyup='auto_fill_wl_name();'>";
		code += "</div>";

		if(wl_info.band2g_support) {
			gen_wl_interface("0");
		}
		if(wl_info.band5g_support) {
			gen_wl_interface("1");
		}
		if(wl_info.band5g_2_support) {
			gen_wl_interface("2");
		}

		code += "<div class='splash_item_content'>";
			code += "<div class='splash_item_title'><#FreeWiFi_Passcode#></div>";
			code += "<input type='checkbox' name='cb_passcode' id='cb_passcode' onchange='update_passcode();'>";
			code += "<#FreeWiFi_Option_Add_Passcode#>";
			code += "<input name='passcode' class='input_25_table' value='' type='text' maxlength='64' autocorrect='off' autocapitalize='off'>";
		code += "</div>";

		code += "<div class='splash_item_content'>";
			code += "<div class='splash_item_title'><#Terms_of_Service#></div>";
			code += "<input type='checkbox' name='cb_terms_service' id='cb_terms_service' onchange='update_terms_service();' checked>";
			code += "<#FreeWiFi_OptionHint#>";
			code += "<textarea name='terms_service' id='terms_service' class='splash_textarea' rows='9' cols='40' maxlength='2048'>";
			code += "By using Free Wi-Fi internet service, you hereby expressly acknowledge and agree that there are significant security, privacy and confidentiality risks inherent in accessing or transmitting information through the internet, whether the connection is facilitated through wired or wireless technology. Security issues include, without limitation, interception of transmissions, loss of data, and the introduction or viruses and other programs that can corrupt or damage your computer.\n\nAccordingly, you agree that the owner and/or provider of this network is NOT liable for any interception or transmissions, computer worms or viruses, loss of data, file corruption, hacking or damage to your computer or other devices that result from the transmission or download of information or materials through the internet service provided.\n\nUse of the wireless network is subject to the general restrictions outlined below. If abnormal, illegal, or unauthorized behavior is detected, including heavy consumption of bandwidth, the network provider reserves the right to permanently disconnect the offending device from the wireless network.";
			code += "</textarea>";
		code += "</div>";

		code += "<div class='splash_item_content'>";
			code += "<div class='splash_item_title'><#FreeWiFi_timeout#></div>";
			code += "<input name='session_length' class='input_6_table' value='60' type='text' maxlength='3' autocorrect='off' autocapitalize='off' onKeyPress='return validator.isNumber(this, event);'>";
			code += "<span class='splash_item_text'><#Minute#></span>";
			//code += "<span class='splash_item_hint'>(<#Setting_factorydefault_value#> : 60)</span>";
		code += "</div>";

		code += "<div class='splash_item_content'>";
			code += "<div class='splash_item_title'><#Bandwidth_Limiter#></div>";
			code += "<span class='splash_item_hint' style='margin:0px;'>(<#EzQoS_bandwidth_note2#>)</span>";
			code += "<div class='splash_item_title'><#option_download#></div>";
			code += "<input name='bandwidth_limiter_dl' class='input_15_table' value='0' type='text' maxlength='12' autocorrect='off' autocapitalize='off' onKeyPress='return validator.bandwidth_code(this, event);'>";
			code += "<span class='splash_item_text'>Mb/s</span>";
			code += "<div class='splash_item_title'><#option_upload#></div>";
			code += "<input name='bandwidth_limiter_ul' class='input_15_table' value='0' type='text' maxlength='12' autocorrect='off' autocapitalize='off' onKeyPress='return validator.bandwidth_code(this, event);'>";
			code += "<span class='splash_item_text'>Mb/s</span>";
		code += "</div>";

		code += "<div class='splash_item_content'>";
			code += "<div class='splash_item_title'><#FreeWiFi_LandingPage#> (<#FreeWiFi_RedirectPage#>)</div>";
			code += "<input name='internet_website' class='input_32_table' value='' type='text' maxlength='64' autocorrect='off' autocapitalize='off'>";
			code += "<span class='splash_item_hint'>ex. http or https ://www.asus.com</span>";
		code += "</div>";
	code += "</div>";

	code += "<div style='clear:both;'></div>";


	$('#captive_portal_content').append(code);
	$('#captive_portal_content').find("#terms_service_hyperlink").addClass("splash_template_terms_service_hyperlink");

	//setting user upload icon attribute start.
	//1.check rc_support
	//if(usericon_support) {
		//2.check browswer support File Reader and Canvas or not.
		if(isSupportFileReader() && isSupportCanvas()) {
			//Setting drop event
			var holder = document.getElementById("splash_image_conent");
			holder.ondragover = function () { return false; };
			holder.ondragend = function () { return false; };
			holder.ondrop = function (e) {
				e.preventDefault();
				var file = e.dataTransfer.files[0];
				//check image
				if(file.type.search("image") != -1) {
					var source_image_size = 0;
					if( (file.size != undefined) && (!isNaN(file.size)) )
						source_image_size = file.size;
					if(Math.round(source_image_size / 1024) > 10240) {
						alert("<#FreeWiFi_Image_Size_Alert#>");
						return false;
					}
					var reader = new FileReader();
					reader.onload = function (event) {
						var img = document.createElement("img");
						img.src = event.target.result;
						var mimeType = img.src.split(",")[0].split(":")[1].split(";")[0];
						var canvas = document.getElementById("splash_canvas");
						var ctx = canvas.getContext("2d");
						ctx.clearRect(0, 0, 1152, 864);
						setTimeout(function() {
							ctx.drawImage(img, 0, 0, 1152, 864);
							var dataURL = canvas.toDataURL(mimeType);
							if(Math.round(dataURL.length / 1024) > 2048) {
								alert("<#FreeWiFi_Image_Size_Compressed_Alert#>");
								return false;
							}
							else {
								splash_image_base64 = dataURL;
								$("#splash_image_default").css({"display" : "none"});
								$('#splash_image_canvas_content').css('background-image', 'url(' + splash_image_base64 + ')');
								$("#splash_image_canvas_content").css("display", "");
							}
						}, 100); //for firefox FPS(Frames per Second) issue need delay
					};
					reader.readAsDataURL(file);
					return false;
				}
				else {
					alert("<#Setting_upload_hint#>");
					return false;
				}
			};
		} 
	//}
	//setting user upload icon attribute end.
}
function gen_line() {
	var code = "";
	code += "<div class='captive_portal_line'></div>";
	$('#captive_portal_content').append(code);
}
function gen_apply() {
	var code = "";
	code += "<div class='apply_content'>";
	code += "<input class='button_gen' onclick='apply();' type='button' value='<#CTL_apply#>'/>";
	code += "</div>";
	$('#captive_portal_content').append(code);
}
function apply() {
	//auto turn on 
	if(document.form.captive_portal_enable.value == "off" && captive_portal_enable == "off") {
		captive_portal_enable = "on";
	}
	if(captive_portal_enable == "on") {
		var empty_wl_idx = get_captive_portal_wl_idx("freeWiFi");

		var validForm = function() {
			if(splash_image_base64 == "") {
				alert("<#FreeWiFi_Image_Empty_Alert#>");
				return false;
			}
			if(!validator.isEmpty($("input[name=brand_name]")[0]))
				return false;
			if(!Block_chars($("input[name=brand_name]")[0], ["<", ">"]))
				return false;
			var _at_least_wl_if_flag = false;
			if(!validator.stringSSID($("input[name=wl_0]")[0]))
				return false;
			if($("#cb_wl_0").prop("checked")) {
				_at_least_wl_if_flag = true;
			}
			if(wl_info.band5g_support) {
				if(!validator.stringSSID($("input[name=wl_1]")[0]))
					return false;
				if($("#cb_wl_1").prop("checked")) {
					_at_least_wl_if_flag = true;
				}
			}
			if(wl_info.band5g_2_support) {
				if(!validator.stringSSID($("input[name=wl_2]")[0]))
					return false;
				if($("#cb_wl_2").prop("checked")) {
					_at_least_wl_if_flag = true;
				}
			}
			if(!_at_least_wl_if_flag) {
				alert("Please select at least one <#QIS_finish_wireless_item1#>.");/*untranslated*/
				return false;
			}
			if($("#cb_terms_service").prop("checked")) {
				var terms_service = $('textarea#terms_service').val().trim();
				if(terms_service == "") {
					setTimeout(function() {
						alert("<#JS_fieldblank#>");
						$('textarea#terms_service').focus();
					}, 0);
					return false;
				}
			}
			if(!validator.numberRange($("input[name=session_length]")[0], 1, 999))
				return false;

			//bandwidth limiter
			if(!validator.isEmpty($("input[name=bandwidth_limiter_dl]")[0]))
				return false;

			if(($("input[name=bandwidth_limiter_dl]").val() !== "0" && ($("input[name=bandwidth_limiter_dl]").val().split(".").length > 2 || $("input[name=bandwidth_limiter_dl]").val() < 0.1)) || isNaN(parseFloat($("input[name=bandwidth_limiter_dl]").val()))) {
				alert("<#min_bound#> : 0.1 Mb/s");
				$("input[name=bandwidth_limiter_dl]").focus();
				return false;
			}

			if(!validator.isEmpty($("input[name=bandwidth_limiter_ul]")[0]))
				return false;

			if(($("input[name=bandwidth_limiter_ul]").val() !== "0" && ($("input[name=bandwidth_limiter_ul]").val().split(".").length > 2 || $("input[name=bandwidth_limiter_ul]").val() < 0.1)) || isNaN(parseFloat($("input[name=bandwidth_limiter_ul]").val()))) {
				alert("<#min_bound#> : 0.1 Mb/s");
				$("input[name=bandwidth_limiter_ul]").focus();
				return false;
			}

			//combine landing page none and internet
			if($("input[name=internet_website]").val().trim() != "") {
				var landingPage = $("input[name=internet_website]").val().trim();
				if(!validator.isValidURL(landingPage)) {
					$("input[name=internet_website]").focus();
					return false;
				}
			}

			if($("#cb_passcode").prop("checked")) {
				var passcode_value = $("input[name=passcode]").val().trim();
				if(passcode_value == "") {
					alert("<#JS_fieldblank#>");
					$("input[name=passcode]").focus();
					return false;
				}
				else if(passcode_value.length > 0 && passcode_value.length < 5) {
					alert("* <#JS_short_password#>");
					$("input[name=passcode]").focus();
					return false;
				}

				//confirm common string combination	#JS_common_passwd#
				var is_common_string = check_common_string(passcode_value, "httpd_password");
				if(passcode_value.length > 0 && is_common_string){
					if(!confirm("<#JS_common_passwd#>")){
						$("input[name=passcode]").focus();
						return false;
					}
				}
			}
			return true;
		};

		if(validForm()) {
			document.form.captive_portal_2g_if.value = '<% nvram_get("captive_portal_2g_if"); %>';
			document.form.captive_portal_5g_if.value = '<% nvram_get("captive_portal_5g_if"); %>';
			document.form.captive_portal_5g_2_if.value = '<% nvram_get("captive_portal_5g_2_if"); %>';
			var captive_portal_2g_status = false;
			var captive_portal_5g_status = false;
			var captive_portal_5g_2_status = false;
			captive_portal_2g_status = $("#cb_wl_0").prop("checked");
			if(wl_info.band5g_support)
				captive_portal_5g_status = $("#cb_wl_1").prop("checked");
			if(wl_info.band5g_2_support)
				captive_portal_5g_2_status = $("#cb_wl_2").prop("checked");

			if(empty_wl_idx == "" && (captive_portal_2g_status || captive_portal_5g_status || captive_portal_5g_2_status)) {
				gen_guestnetwork_wl();
				$("#full_screen_bg").fadeIn(300);
				$('#guestnetwork_wl').fadeIn();
				cal_panel_block("guestnetwork_wl", 0.25);
				var scrollTop = $(document).scrollTop();
				$("#guestnetwork_wl").css({ top: (scrollTop + 20) + "px" });
			}
			else {
				var gn_overwrite_hint = "";
				if($("#cb_wl_0").prop("checked")) {
					if(!check_gn_if_status(empty_wl_idx, gn_array_2g) && document.form.captive_portal_2g_if.value == "off")
						gn_overwrite_hint += "<#Guest_Network#> " + empty_wl_idx + " - " + wl_nband_title[0] + " will be overwrite.\n\n";

					document.form.captive_portal_2g_if.value = "wl0." + empty_wl_idx;
				}
				else
					document.form.captive_portal_2g_if.value = "off";
				if(wl_info.band5g_support) {
					if($("#cb_wl_1").prop("checked")) {
						if(!check_gn_if_status(empty_wl_idx, gn_array_5g) && document.form.captive_portal_5g_if.value == "off")
							gn_overwrite_hint += "<#Guest_Network#> " + empty_wl_idx + " - " + wl_nband_title[1] + " will be overwrite.\n\n";

						document.form.captive_portal_5g_if.value = "wl1." + empty_wl_idx;
					}
					else
						document.form.captive_portal_5g_if.value = "off";
				}
				if(wl_info.band5g_2_support) {
					if($("#cb_wl_2").prop("checked")) {
						if(!check_gn_if_status(empty_wl_idx, gn_array_5g_2) && document.form.captive_portal_5g_2_if.value == "off")
							gn_overwrite_hint += "<#Guest_Network#> " + empty_wl_idx + " - " + wl_nband_title[2] + " will be overwrite.";

						document.form.captive_portal_5g_2_if.value = "wl2." + empty_wl_idx;
					}
					else
						document.form.captive_portal_5g_2_if.value = "off";
				}

				if(gn_overwrite_hint != "")
					alert(gn_overwrite_hint);

				document.form.captive_portal_enable.value = "on";
				setting_profile_id = (setting_profile_id == "") ? $.now() : setting_profile_id;
				save_splash_page_content();
			}
		}
	}
	else {
		document.form.captive_portal.disabled = true;
		document.form.captive_portal_2g.disabled = true;
		document.form.captive_portal_5g.disabled = true;
		document.form.captive_portal_5g_2.disabled = true;
		document.form.captive_portal_passcode.disabled = true;
		document.form.captive_portal_2g_if.value = "off";
		document.form.captive_portal_5g_if.value = "off";
		document.form.captive_portal_5g_2_if.value = "off";
		document.form.captive_portal_enable.value = "off";
		document.form.submit();
	}
}
function call_back_to_save_config(_splash_page_status) {
	if(_splash_page_status) {
		var _profile_id = setting_profile_id;
		var terms_service_status = $("#cb_terms_service").prop("checked");
		var passcode_status = $("#cb_passcode").prop("checked");
		var passcode =  $("input[name=passcode]").val().trim();
		var brand_name = $("input[name=brand_name]").val().trim();
		var session_time = $("input[name=session_length]").val().trim();
		var bandwidth_limiter_dl = $("input[name=bandwidth_limiter_dl]").val().trim()*1024;
		var bandwidth_limiter_ul = $("input[name=bandwidth_limiter_ul]").val().trim()*1024;
		var landing_type = "0";
		//combine landing page none and internet
		if($("input[name=internet_website]").val().trim() == "") {
			landing_type = "0";
		}
		else {
			landing_type = "1";
		}
		var wl_list = "";
		if(document.form.captive_portal_2g_if.value != "off")
			wl_list += document.form.captive_portal_2g_if.value;
		if(wl_info.band5g_support) {
			if(document.form.captive_portal_5g_if.value != "off")
				wl_list += document.form.captive_portal_5g_if.value;
		}
		if(wl_info.band5g_2_support) {
			if(document.form.captive_portal_5g_2_if.value != "off")
				wl_list += document.form.captive_portal_5g_2_if.value;
		}
		var landing_type_value = "";
		switch(landing_type) {
			case "0" :
				landing_type_value = "";
				break;
			case "1" :
				landing_type_value = $("input[name=internet_website]").val().trim();
				break;
		}

		var captive_portal_temp = "";
		captive_portal_temp += "<" + _profile_id + ">" + brand_name + ">" + session_time + ">" + landing_type + ">" + landing_type_value + ">" + wl_list + ">" + ((terms_service_status) ? "1" : "0") + ">" + ((passcode_status) ? "1" : "0") + ">" + bandwidth_limiter_dl + ">" + bandwidth_limiter_ul;
		document.form.captive_portal.value = captive_portal_temp;
		document.form.captive_portal_2g.value = $("input[name=wl_0]").val().trim();
		if(wl_info.band5g_support)
			document.form.captive_portal_5g.value = $("input[name=wl_1]").val().trim();
		if(wl_info.band5g_2_support)
			document.form.captive_portal_5g_2.value = $("input[name=wl_2]").val().trim();

		//if(based_modelid == "BRT-AC828") {
			var captive_portal_enable_ori = '<% nvram_get("captive_portal_enable"); %>';
			if(captive_portal_enable_ori == "off") {
				cookie.set("captive_portal_gn_idx", get_captive_portal_wl_idx("freeWiFi") + ">freeWiFi", 1);
				document.form.next_page.value = "Guest_network.asp";
			}
		//}

		document.form.captive_portal_passcode.value = passcode;
		document.form.submit();
	}
	else {
		alert('<#ALERT_OF_ERROR_System4#>');
	}
}
function save_splash_page_content() {
	//save image and html
	var _profile_id = setting_profile_id;
	var terms_service = encode_decode_text($('textarea#terms_service').val(), "encode");
	var brand_name = encode_decode_text($("input[name=brand_name]").val().trim(), "encode");
	var terms_service_status = $("#cb_terms_service").prop("checked");
	var passcode_status = $("#cb_passcode").prop("checked");
	var image_size = $("#splash_image_size").val();
	var splash_page_setting = '';
	splash_page_setting += '{\n';
	splash_page_setting += '"splash_page_setting":{\n';
	splash_page_setting += '"image":"' + splash_image_base64 + '",\n';
	splash_page_setting += '"image_size":"' + image_size + '",\n';
	splash_page_setting += '"terms_service":"' + terms_service + '"\n';
	splash_page_setting += '}\n';
	splash_page_setting += '}\n';

	var html_landing_css = "";
	if(terms_service_status) {
		html_landing_css += ".terms_service {\n";
		html_landing_css += "position: fixed;\n";
		html_landing_css += "width: 80vw;\n";
		html_landing_css += "height: 68vh;\n";
		html_landing_css += "background-color: #232E32;\n";
		html_landing_css += "left: 7vw;\n";
		html_landing_css += "top: 13vh;\n";
		html_landing_css += "z-index: 200;\n";
		html_landing_css += "padding: 3vh 3vw;\n";
		html_landing_css += "border-radius: 2vw;\n";
		html_landing_css += "display: none;\n";
		html_landing_css += "box-shadow: 0 19px 38px rgba(0,0,0,0.30), 0 15px 12px rgba(0,0,0,0.22);\n";
		html_landing_css += "color: #FFFFFF;\n";
		html_landing_css += "}\n";
		html_landing_css += ".term_service_title {\n";
		html_landing_css += "font-weight: bolder;\n";
		html_landing_css += "text-align: center;\n";
		html_landing_css += "font-family: Microsoft JhengHei;\n";
		html_landing_css += "border-bottom: 1px solid;\n";
		html_landing_css += "padding-bottom: 2%;\n";
		html_landing_css += "}\n";
		html_landing_css += ".term_service_text {\n";
		html_landing_css += "position: absolute;\n";
		html_landing_css += "bottom: 2%;\n";
		html_landing_css += "top: 2%;\n";
		html_landing_css += "height: 96%;\n";
		html_landing_css += "overflow: auto;\n";
		html_landing_css += "overflow-x: hidden;\n";
		html_landing_css += "font-family: Arial, Helvetica, sans-serif;\n";
		html_landing_css += "white-space: normal;\n";
		html_landing_css += "word-break: break-all;\n";
		html_landing_css += "}\n";
		html_landing_css += ".terms_service_close {\n";
		html_landing_css += "width: 5vmin;\n";
		html_landing_css += "height: 5vmin;\n";
		html_landing_css += "cursor: pointer;\n";
		html_landing_css += "position: absolute;\n";
		html_landing_css += "right: 1vw;\n";
		html_landing_css += "top: 1vmin;\n";
		html_landing_css += "}\n";
		html_landing_css += ".splash_template_terms_service {\n";
		html_landing_css += "color: #757575;\n";
		html_landing_css += "text-align: left;\n";
		html_landing_css += "border-bottom-color: #4A90E2;\n";
		html_landing_css += "border-bottom-style: solid;\n";
		html_landing_css += "cursor: pointer;\n";
		html_landing_css += "line-height: 180%;\n";
		html_landing_css += "}\n";
		html_landing_css += ".splash_template_terms_service_hyperlink {\n";
		html_landing_css += "color: #4A90E2;\n";
		html_landing_css += "text-decoration: underline;\n";
		html_landing_css += "cursor:pointer;\n";
		html_landing_css += "}\n";
		html_landing_css += ".splash_template_terms_service_cb {\n";
		html_landing_css += "width: 15%;\n";
		html_landing_css += "float: left;\n";
		html_landing_css += "margin-top: 2%;\n";
		html_landing_css += "}\n";
		html_landing_css += ".splash_template_terms_service_text {\n";
		html_landing_css += "width: 85%;\n";
		html_landing_css += "float: left;\n";
		html_landing_css += "}\n";
	}
	if(passcode_status) {
		html_landing_css += ".splash_template_passcode {\n";
		html_landing_css += "background-color: rgba(74, 144, 226, 0.5);\n";
		html_landing_css += "border-radius: 4px;\n";
		html_landing_css += "width: 90%;\n";
		html_landing_css += "border: 0px;\n";
		html_landing_css += "color: #FFFFFF;\n";
		html_landing_css += "padding-left: 2%;\n";
		html_landing_css += "font-size: 12px;\n";
		html_landing_css += "margin-left: 4%;\n";
		html_landing_css += "}\n";
	}
	html_landing_css += ".splash_body {\n";
	html_landing_css += "margin: 0px;\n";
	html_landing_css += "background-repeat: no-repeat;\n";
	if(image_size == "center")
		html_landing_css += "background-size: cover;\n";
	else
		html_landing_css += "background-size: 100% 100%;\n";
	html_landing_css += "background-attachment: fixed;\n";
	html_landing_css += "background-position: 50% 50%;\n";
	html_landing_css += "background-image: url('" + splash_image_base64 + "');\n";
	html_landing_css += "}\n";
	html_landing_css += ".splash_template_bg {\n";
	html_landing_css += "position: absolute;\n";
	html_landing_css += "top: 0;\n";
	html_landing_css += "right: 0;\n";
	html_landing_css += "bottom: 0;\n";
	html_landing_css += "left: 0;\n";
	html_landing_css += "}\n";
	html_landing_css += ".splash_template_content {\n";
	html_landing_css += "font-family: Arial, Roboto, Helvetica;\n";
	html_landing_css += "background-color: rgba(255, 255, 255, 0.93);\n";
	html_landing_css += "border-radius: 8px;\n";
	html_landing_css += "box-shadow: 2px 3px 6px 0px rgba(0, 0, 0, 0.20);\n";
	html_landing_css += "border: 1px solid #ACACAC;\n";
	html_landing_css += "position: relative;\n";
	html_landing_css += "}\n";
	html_landing_css += ".splash_template_title {\n";
	html_landing_css += "color: #757575;\n";
	html_landing_css += "text-align: left;\n";
	html_landing_css += "}\n";
	html_landing_css += ".splash_template_brand_name {\n";
	html_landing_css += "font-weight: lighter;\n";
	html_landing_css += "color: #4A90E2;\n";
	html_landing_css += "text-align: left;\n";
	html_landing_css += "border-bottom-color: #D4D4D4;\n";
	html_landing_css += "border-bottom-style: solid;\n";
	html_landing_css += "word-break: break-all;\n";
	html_landing_css += "}\n";
	html_landing_css += ".splash_template_continue {\n";
	html_landing_css += "background-color: #4A90E2;\n";
	html_landing_css += "border-radius: 4px;\n";
	html_landing_css += "transition: visibility 0s linear 0.218s,opacity 0.218s,background-color 0.218s;\n";
	html_landing_css += "width: 90%;\n";
	html_landing_css += "color: #fff;\n";
	html_landing_css += "color: #000\9;\n";
	html_landing_css += "text-align: center;\n";
	html_landing_css += "cursor: pointer;\n";
	html_landing_css += "}\n";
	html_landing_css += ".splash_template_icon {\n";
	html_landing_css += "background-size: 100%;\n";
	html_landing_css += "position: absolute;\n";
	html_landing_css += "background-repeat: no-repeat;\n";
	html_landing_css += "}\n";
	html_landing_css += ".splash_template_icon_error {\n";
	html_landing_css += "width: 10vw;\n";
	html_landing_css += "height: 10vw;\n";
	html_landing_css += "fill: #FF0000;\n";
	html_landing_css += "}\n";
	html_landing_css += ".splash_template_error_content {\n";
	html_landing_css += "position: absolute;\n";
	html_landing_css += "bottom: 10vh;\n";
	html_landing_css += "right: 10vw;\n";
	html_landing_css += "width: 80vw;\n";
	html_landing_css += "height: 80vh;\n";
	html_landing_css += "text-align: center;\n";
	html_landing_css += "font-family: Arial, Roboto, Helvetica;\n";
	html_landing_css += "color: #FFFFFF;\n";
	html_landing_css += "background-color: rgba(255, 255, 255, .93);\n";
	html_landing_css += "border-radius: 8px;\n";
	html_landing_css += "box-shadow: 2px 3px 6px 0px rgba(0, 0, 0, 0.20);\n";
	html_landing_css += "border: 1px solid #ACACAC;\n";
	html_landing_css += "}\n";
	html_landing_css += ".splash_template_error_icon {\n";
	html_landing_css += "margin: 2vh 2vw;\n";
	html_landing_css += "border-bottom-color: #FF0000;\n";
	html_landing_css += "border-bottom-style: solid;\n";
	html_landing_css += "border-bottom-width: 0.5vh;\n";
	html_landing_css += "padding-bottom: 1vh;\n";
	html_landing_css += "}\n";
	html_landing_css += ".splash_template_error_desc {\n";
	html_landing_css += "font-weight: lighter;\n";
	html_landing_css += "font-size: 5vw;\n";
	html_landing_css += "color: #FF0000;\n";
	html_landing_css += "margin: 2vh 2vw;\n";
	html_landing_css += "text-align: left;\n";
	html_landing_css += "word-break: break-all;\n";
	html_landing_css += "}\n";
	html_landing_css += ".splash_template_error_close {\n";
	html_landing_css += "background-color: #FF0000;\n";
	html_landing_css += "border-radius: 1vh;\n";
	html_landing_css += "transition: visibility 0s linear 0.218s,opacity 0.218s,background-color 0.218s;\n";
	html_landing_css += "height: 6vh;\n";
	html_landing_css += "margin: 4vh 2vw 4vh 2vw;\n";
	html_landing_css += "font-size: 3vh;\n";
	html_landing_css += "color: #fff;\n";
	html_landing_css += "color: #000\9;\n";
	html_landing_css += "text-align: center;\n";
	html_landing_css += "line-height: 6vh;\n";
	html_landing_css += "cursor: pointer;\n";
	html_landing_css += "bottom: 0vw;\n";
	html_landing_css += "position: absolute;\n";
	html_landing_css += "width: 76vw;\n";
	html_landing_css += "}\n";

	var html_landing = "";
		html_landing += "<html xmlns='http://www.w3.org/1999/xhtml' xmlns:v=''>\n";
		html_landing += "<meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'>\n";
		html_landing += "<head><meta http-equiv='X-UA-Compatible' content='IE=Edge'>\n";
		html_landing += "<meta http-equiv='Content-Type' content='text/html; charset=utf-8'>\n";
		html_landing += "<meta http-equiv='Pragma' content='no-cache'>\n";
		html_landing += "<meta http-equiv='Expires' content='-1'>\n";
		html_landing += "<title>" + decodeURIComponent(brand_name) + "</title>\n";
		html_landing += "<link rel='stylesheet' type='text/css' href='FreeUam.css'>\n";
		html_landing += "<_INCLUDE_JQUERY_>\n";
		html_landing += "<_INCLUDE_UAM_>\n";
		html_landing += "<_TAG_START_>\n";
		if(terms_service_status) {
			html_landing += "var htmlEnDeCode = (function() {\n";
			html_landing += "var charToEntityRegex,\n";
			html_landing += "entityToCharRegex,\n";
			html_landing += "charToEntity,\n";
			html_landing += "entityToChar;\n";
			html_landing += "function resetCharacterEntities() {\n";
			html_landing += "charToEntity = {};\n";
			html_landing += "entityToChar = {};\n";
			html_landing += "addCharacterEntities({\n";
			html_landing += "'&amp;'     :   '&',\n";
			html_landing += "'&gt;'      :   '>',\n";
			html_landing += "'&lt;'      :   '<',\n";
			html_landing += "'&quot;'    :   '\"',\n";
			html_landing += "'&#39;'     :   '\\''\n";
			html_landing += "});\n";
			html_landing += "}\n";
			html_landing += "function addCharacterEntities(newEntities) {\n";
			html_landing += "var charKeys = [],\n";
			html_landing += "entityKeys = [],\n";
			html_landing += "key, echar;\n";
			html_landing += "for (key in newEntities) {\n";
			html_landing += "echar = newEntities[key];\n";
			html_landing += "entityToChar[key] = echar;\n";
			html_landing += "charToEntity[echar] = key;\n";
			html_landing += "charKeys.push(echar);\n";
			html_landing += "entityKeys.push(key);\n";
			html_landing += "}\n";
			html_landing += "charToEntityRegex = new RegExp('(' + charKeys.join('|') + ')', 'g');\n";
			html_landing += "entityToCharRegex = new RegExp('(' + entityKeys.join('|') + '|&#[0-9]{1,5};' + ')', 'g');\n";
			html_landing += "}\n";
			html_landing += "function htmlEncode(value){\n";
			html_landing += "var htmlEncodeReplaceFn = function(match, capture) {\n";
			html_landing += "return charToEntity[capture];\n";
			html_landing += "};\n";
			html_landing += "return (!value) ? value : String(value).replace(charToEntityRegex, htmlEncodeReplaceFn);\n";
			html_landing += "}\n";
			html_landing += "function htmlDecode(value) {\n";
			html_landing += "var htmlDecodeReplaceFn = function(match, capture) {\n";
			html_landing += "return (capture in entityToChar) ? entityToChar[capture] : String.fromCharCode(parseInt(capture.substr(2), 10));\n";
			html_landing += "};\n";
			html_landing += "return (!value) ? value : String(value).replace(entityToCharRegex, htmlDecodeReplaceFn);\n";
			html_landing += "}\n";
			html_landing += "resetCharacterEntities();\n";
			html_landing += "return {\n";
			html_landing += "htmlEncode: htmlEncode,\n";
			html_landing += "htmlDecode: htmlDecode\n";
			html_landing += "};\n";
			html_landing += "})();\n";
		}
		html_landing += "window.moveTo(0,0);\n";
		html_landing += "var windw_width = screen.width;\n";
		html_landing += "var windw_height = screen.height;\n";
		html_landing += "if(isMobile()) {\n";
		html_landing += "var supportsOrientationChange = 'onorientationchange' in window,\n";
		html_landing += "orientationEvent = supportsOrientationChange ? 'orientationchange' : 'resize';\n";
		html_landing += "window.addEventListener(orientationEvent, function() {\n";
		html_landing += "if(window.orientation == undefined) {\n";
		html_landing += "setTimeout(function() {\n";
		html_landing += "windw_width = screen.width;\n";
		html_landing += "windw_height = screen.height;\n";
		html_landing += "resize_component();\n";
		html_landing += "},100);\n";
		html_landing += "}\n";
		html_landing += "else {\n";
		html_landing += "switch(window.orientation) {\n";
		html_landing += "case -90 :\n";
		html_landing += "case 90 :\n";
		html_landing += "windw_width = (screen.height > screen.width) ? screen.height : screen.width;\n";
		html_landing += "windw_height = (screen.height > screen.width) ? screen.width : screen.height;\n";
		html_landing += "break;\n";
		html_landing += "default :\n";
		html_landing += "windw_width = (screen.height < screen.width) ? screen.height : screen.width;\n";
		html_landing += "windw_height = (screen.height < screen.width) ? screen.width : screen.height;\n";
		html_landing += "break;\n";
		html_landing += "}\n";
		html_landing += "resize_component();\n";
		html_landing += "}\n";
		html_landing += "}, false);\n";
		html_landing += "}\n";
		html_landing += "window.onresize = function() {\n";
		html_landing += "if(!isMobile()) {\n";
		html_landing += "initial();\n";
		html_landing += "}\n";
		html_landing += "};\n";
		html_landing += "function initial() {\n";
		html_landing += "document.getElementById('splash_template_brand_name').innerText = decodeURIComponent(\"" + brand_name + "\");\n";
		html_landing += "var mql = window.matchMedia('(orientation: portrait)');\n";
		html_landing += "if(mql.matches) {\n";  
		html_landing += "windw_width = (screen.height < screen.width) ? screen.height : screen.width;\n";
		html_landing += "windw_height = (screen.height < screen.width) ? screen.width : screen.height;\n";
		html_landing += "}\n";
		html_landing += "else {  \n";
		html_landing += "windw_width = (screen.height > screen.width) ? screen.height : screen.width;\n";
		html_landing += "windw_height = (screen.height > screen.width) ? screen.width : screen.height;\n";
		html_landing += "}\n";
		if(terms_service_status) {
			html_landing += "var code = '';\n";
			html_landing += "code += '<div style=\"display:table;width:100%;height:100%;border-collapse:collapse;\">';\n";
			html_landing += "code += '<div style=\"display:table-row;\">';\n";
			html_landing += "code += '<div id=\"term_service_title\" class=\"term_service_title\" style=\"display:table-cell;vertical-align:middle;font-size:13px;\" >';\n";
			html_landing += "code += 'Free Wi-Fi TERMS OF SERVICE';\n";
			html_landing += "code += '</div>';\n";
			html_landing += "code += '</div>';\n";
			html_landing += "code += '<div style=\"display:table-row;height:90%;\">';\n";
			html_landing += "code += '<div style=\"display:table-cell;\">';\n";
			html_landing += "code += '<div style=\"width:100%;max-width:100%;height:100%;max-height:100%;position:relative;\">';\n";
			html_landing += "code += '<div id=\"term_service_text\" class=\"term_service_text\" style=\"font-size:12px;\">';\n";
			html_landing += "var terms_service_text = decodeURIComponent(\"" + terms_service + "\");\n";
			html_landing += "terms_service_text = terms_service_text.replace(/\<br\>/g, '\\n');\n";
			html_landing += "terms_service_text = htmlEnDeCode.htmlEncode(terms_service_text);\n";
			html_landing += "terms_service_text = terms_service_text.replace(/ /g, '&nbsp;').replace(/(?:\\r\\n|\\r|\\n)/g, '<br>');\n";
			html_landing += "code += terms_service_text;\n";
			html_landing += "code += '</div>';\n";
			html_landing += "code += '</div>';\n";
			html_landing += "code += '</div>';\n";
			html_landing += "code += '</div>';\n";
			html_landing += "code += '</div>';\n";
			html_landing += "code += '<div class=\"terms_service_close\" onclick=\"close_term_service();\">';\n";
			html_landing += "code += '<svg version=\"1.1\" id=\"terms_service_close_icon\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\" viewBox=\"0 0 64 64\" enable-background=\"new 0 0 64 64\" xml:space=\"preserve\">';\n";
			html_landing += "code += '<line fill=\"none\" stroke=\"#FFFFFF\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-miterlimit=\"10\" x1=\"21.1\" y1=\"21.1\" x2=\"42.9\" y2=\"42.9\"/>';\n";
			html_landing += "code += '<line fill=\"none\" stroke=\"#FFFFFF\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-miterlimit=\"10\" x1=\"42.9\" y1=\"21.1\" x2=\"21.1\" y2=\"42.9\"/>';\n";
			html_landing += "code += '<circle fill=\"none\" stroke=\"#FFFFFF\" stroke-width=\"2.5\" stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-miterlimit=\"10\" cx=\"32\" cy=\"32.1\" r=\"25.2\"/>';\n";
			html_landing += "code += '</svg>';\n";
			html_landing += "code += '</div>';\n";
			html_landing += "$('#terms_service').html(code);\n";

			html_landing += "control_bt_status();\n";
			html_landing += "document.getElementById('terms_service_hyperlink').className = 'splash_template_terms_service_hyperlink';\n";
			html_landing += "document.getElementById('terms_service_hyperlink').onclick = open_term_service;\n";
		}
		html_landing += "resize_component();\n";
		html_landing += "}\n";
		html_landing += "function resize_component() {\n";
		html_landing += "var _ratio = 0;\n";
		html_landing += "if(isMobile()) {\n";
		html_landing += "$('meta[name=viewport]').attr('content', 'initial-scale=1.0,maximum-scale=1.0,minimum-scale=1.0,user-scalable=no,width=' + windw_width + ',height=' + windw_height);\n";
		html_landing += "document.getElementById('splash_template_bg').style.margin = 'auto';\n";
		html_landing += "_ratio = ((windw_width / 256));\n";
		html_landing += "if(windw_width > windw_height) {\n";
		html_landing += "document.getElementById('splash_template_bg').style.margin = '10%';\n";
		html_landing += "}\n";
		html_landing += "}\n";
		html_landing += "else {\n";
		html_landing += "$('meta[name=viewport]').attr('content', 'initial-scale=1.0,maximum-scale=1.0,minimum-scale=1.0,user-scalable=no,width=' + windw_width + ',height=' + windw_height);\n";
		html_landing += "document.getElementById('splash_template_bg').style.margin = 'auto';\n";
		html_landing += "_ratio = 1.8;\n";
		html_landing += "}\n";
		html_landing += "document.getElementById('splash_template_bg').style.width = (205 * _ratio) + 'px';\n";
		html_landing += "document.getElementById('splash_template_bg').style.height = (145 * _ratio) + 'px';\n";
		if(terms_service_status) {
			html_landing += "if(document.getElementById('splash_template_terms_service').style.display != 'none')\n";
			html_landing += "document.getElementById('splash_template_bg').style.height = (203 * _ratio) + 'px';\n";
		}
		html_landing += "document.getElementById('splash_template_icon').style.width = (80 * _ratio) + 'px';\n";
		html_landing += "document.getElementById('splash_template_icon').style.height = (60 * _ratio) + 'px';\n";
		html_landing += "document.getElementById('splash_template_icon').style.top = (-20 * _ratio) + 'px';\n";
		html_landing += "document.getElementById('splash_template_icon').style.right = (-15 * _ratio) + 'px';\n";
		html_landing += "document.getElementById('splash_template_title').style.margin = (10 * _ratio) + 'px';\n";
		html_landing += "document.getElementById('splash_template_title').style.marginTop = (30 * _ratio) + 'px';\n";
		html_landing += "document.getElementById('splash_template_title').style.fontSize = (12 * _ratio) + 'px';\n";
		html_landing += "document.getElementById('splash_template_brand_name').style.margin = (10 * _ratio) + 'px';\n";
		html_landing += "document.getElementById('splash_template_brand_name').style.marginTop = '0px';\n";
		html_landing += "document.getElementById('splash_template_brand_name').style.fontSize = (22 * _ratio) + 'px';\n";
		html_landing += "document.getElementById('splash_template_brand_name').style.borderBottomWidth = (1 * _ratio) + 'px';\n";
		html_landing += "document.getElementById('splash_template_brand_name').style.paddingBottom = (5 * _ratio) + 'px';\n";
		if(terms_service_status) {
			html_landing += "document.getElementById('cbTermService').style.width = (13 * _ratio) + 'px';\n";
			html_landing += "document.getElementById('cbTermService').style.height = (13 * _ratio) + 'px';\n";
			html_landing += "document.getElementById('splash_template_terms_service').style.margin = (10 * _ratio) + 'px';\n";
			html_landing += "document.getElementById('splash_template_terms_service').style.marginTop = (0 * _ratio) + 'px';\n";
			html_landing += "document.getElementById('splash_template_terms_service').style.fontSize = (12 * _ratio) + 'px';\n";
			html_landing += "document.getElementById('splash_template_terms_service').style.borderBottomWidth = (1 * _ratio) + 'px';\n";
			html_landing += "document.getElementById('splash_template_terms_service').style.paddingBottom = (5 * _ratio) + 'px';\n";
			html_landing += "document.getElementById('term_service_title').style.fontSize = (10 * _ratio) + 'px';\n";
			html_landing += "document.getElementById('term_service_text').style.fontSize = (9 * _ratio) + 'px';\n";
		}
		html_landing += "document.getElementById('splash_template_continue').style.height = (25 * _ratio) + 'px';\n";
		html_landing += "document.getElementById('splash_template_continue').style.fontSize = (12 * _ratio) + 'px';\n";
		html_landing += "document.getElementById('splash_template_continue').style.lineHeight = (25 * _ratio) + 'px';\n";
		html_landing += "document.getElementById('splash_template_continue').style.margin = '' + (15 * _ratio) + 'px auto';\n";
		if(passcode_status) {
			html_landing += "document.getElementById('splash_template_passcode').style.height = (20 * _ratio) + 'px';\n";
			html_landing += "document.getElementById('splash_template_passcode').style.fontSize = (12 * _ratio) + 'px';\n";
			html_landing += "document.getElementById('splash_template_passcode').style.lineHeight = (20 * _ratio) + 'px';\n";
		}
		html_landing += "}\n";
		if(terms_service_status) {
			html_landing += "function control_bt_status() {\n";
			html_landing += "var _obj = document.getElementById('cbTermService');\n";
			html_landing += "if(_obj.checked) {\n";
			html_landing += "document.getElementById('splash_template_continue').style.opacity = 1;\n";
			html_landing += "}\n";
			html_landing += "else {\n";
			html_landing += "document.getElementById('splash_template_continue').style.opacity = 0.5;\n";
			html_landing += "}\n";
			html_landing += "}\n";
			
			html_landing += "function open_term_service() {\n";
			html_landing += "if(isMobile()) {\n";
			html_landing += "$('#terms_service').css('display', 'block');\n";
			html_landing += "$('#splash_template_bg').css('display', 'none');\n";
			html_landing += "}\n";
			html_landing += "else {\n";
			html_landing += "$('#terms_service').fadeIn(300);\n";
			html_landing += "$('#splash_template_bg').fadeOut(100);\n";
			html_landing += "}\n";
			html_landing += "}\n";
			html_landing += "function close_term_service() {\n";
			html_landing += "if(isMobile()) {\n";
			html_landing += "$('#terms_service').css('display', 'none');\n";
			html_landing += "$('#splash_template_bg').css('display', 'block');\n";
			html_landing += "}\n";
			html_landing += "else {\n";
			html_landing += "$('#terms_service').fadeOut(300);\n";
			html_landing += "$('#splash_template_bg').fadeIn(100);\n";
			html_landing += "}\n";
			html_landing += "}\n";
		}

		html_landing += "function continue_action() {\n";
		html_landing += "var passcode_status = false;\n";
		if(passcode_status) {
			html_landing += "passcode_status = true;\n";
			html_landing += "var _obj_value = document.getElementById('splash_template_passcode').value.trim();\n";
			html_landing += "if(_obj_value == '') {\n";
			html_landing += "alert('<#JS_fieldblank#>');\n";
			html_landing += "document.getElementById('splash_template_passcode').focus();\n";
			html_landing += "return false;\n";
			html_landing += "}\n";
		}
		if(terms_service_status) {
			html_landing += "var _obj = document.getElementById('cbTermService');\n";
			html_landing += "if(!_obj.checked) {\n";
			html_landing += "alert('You must agree to the terms of service before continuing.');\n";
			html_landing += "return false;\n";
			html_landing += "}\n";
		}
		html_landing += "if(passcode_status)\n";
		html_landing += "formSubmit(2);\n";
		html_landing += "else\n";
		html_landing += "formSubmit(0);\n";
		html_landing += "}\n";

		html_landing += "function isMobile() {\n";
		html_landing += "var userAgentString = navigator.userAgent.toLowerCase();\n";
		html_landing += "var mobile = ['iphone','ipad','ipod','android','blackberry','nokia','opera mini','windows mobile','windows phone','iemobile','mobile safari','bb10; touch', 'tablet'];\n";
		html_landing += "for (var i in mobile) if (userAgentString.indexOf(mobile[i]) > 0) return true;\n";
		html_landing += "return false;\n";
		html_landing += "}\n";
		html_landing += "<_TAG_END_>\n";
		html_landing += "</head>\n";
		html_landing += "<body onload='initial();' id='splash_body' class='splash_body'>\n";
		if(terms_service_status) {
			html_landing += "<div id='terms_service' class='terms_service'></div>\n";
		}
		html_landing += "<div id='splash_template_bg' class='splash_template_bg'>\n";
		html_landing += "<div id='splash_template_content' class='splash_template_content'>\n";
		html_landing += "<svg id='splash_template_icon' class='splash_template_icon' width='214px' height='164px' viewBox='0 0 214 164' version='1.1' xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' xmlns:sketch='http://www.bohemiancoding.com/sketch/ns'>\n";
		html_landing += "<defs></defs>\n";
		html_landing += "<g id='Page-1' stroke='none' stroke-width='1' fill='none' fill-rule='evenodd' sketch:type='MSPage'>\n";
		html_landing += "<g id='Artboard-11' sketch:type='MSArtboardGroup' transform='translate(-1696.000000, -489.000000)' fill='#B1B1B1'>\n";
		html_landing += "<g id='Page-1' sketch:type='MSLayerGroup' transform='translate(1696.000000, 489.000000)'>\n";
		html_landing += "<path d='M137.890672,79.9008507 L122.005952,79.9008507 L122.005952,90.1224355 L135.284336,90.1224355 C136.51389,90.1224355 137.433071,90.3979392 138.0379,90.9529393 C138.642729,91.5039466 138.953102,92.2466086 138.953102,93.1729397 C138.953102,94.0992709 138.63875,94.8339473 138.025963,95.3809618 C137.413175,95.9279763 136.493994,96.1954943 135.284336,96.1954943 L122.005952,96.1954943 L122.005952,109.359778 C122.005952,111.032763 121.627934,112.266541 120.879856,113.081073 C120.123819,113.88762 119.160868,114.290894 117.983043,114.290894 C116.789301,114.290894 115.818392,113.883627 115.066335,113.065102 C114.314277,112.250569 113.936259,111.012799 113.936259,109.359778 L113.936259,78.6231526 C113.936259,77.4612459 114.107362,76.5069651 114.453547,75.7762815 C114.795753,75.0376123 115.336916,74.5025762 116.073057,74.1631877 C116.801239,73.8277919 117.744295,73.6640869 118.882328,73.6640869 L137.890672,73.6640869 C139.175934,73.6640869 140.130927,73.9475761 140.755652,74.5185475 C141.380376,75.0935116 141.690749,75.8401664 141.690749,76.7704904 C141.690749,77.7127927 141.380376,78.471426 140.755652,79.0423973 C140.130927,79.6173614 139.175934,79.9008507 137.890672,79.9008507' id='Fill-1' sketch:type='MSShapeGroup'></path>\n";
		html_landing += "<path d='M153.85776,88.2685754 L153.85776,109.769839 C153.85776,111.259156 153.503617,112.385127 152.799309,113.14376 C152.095002,113.910379 151.195717,114.293689 150.113391,114.293689 C149.023107,114.293689 148.139739,113.898401 147.463285,113.119803 C146.786831,112.337213 146.444626,111.219228 146.444626,109.769839 L146.444626,88.4881798 C146.444626,87.0148342 146.786831,85.9048339 147.463285,85.1621719 C148.139739,84.4155171 149.023107,84.0441861 150.113391,84.0441861 C151.195717,84.0441861 152.095002,84.4155171 152.799309,85.1621719 C153.503617,85.9048339 153.85776,86.9429637 153.85776,88.2685754 M150.192974,80.5824228 C149.158398,80.5824228 148.279008,80.2669911 147.542868,79.628142 C146.810706,78.9932858 146.444626,78.0949043 146.444626,76.9329976 C146.444626,75.8749039 146.818665,75.0124576 147.570722,74.3296877 C148.322779,73.6469178 149.19421,73.3075292 150.192974,73.3075292 C151.151946,73.3075292 152.003482,73.6149753 152.747581,74.2338604 C153.4877,74.8527454 153.85776,75.7511269 153.85776,76.9329976 C153.85776,78.0749403 153.495659,78.9733217 152.775435,79.6161636 C152.047252,80.2590055 151.187758,80.5824228 150.192974,80.5824228' id='Fill-3' sketch:type='MSShapeGroup'></path>\n";
		html_landing += "<path d='M210.144661,56.353274 C209.949684,56.353274 209.754706,56.33331 209.559728,56.2933819 C207.940219,55.9739574 206.893705,54.3967988 207.216016,52.7757193 C211.453798,31.3503193 197.538751,10.4599554 176.186695,6.19963075 C174.571165,5.88020623 173.520672,4.30304763 173.842982,2.68196817 C174.161314,1.0608887 175.745011,0.0147733843 177.352582,0.330205103 C189.266123,2.7019322 199.540258,9.58553072 206.280918,19.7072954 C213.025558,29.8250672 215.432936,41.9831632 213.065349,53.9416188 C212.786809,55.3670508 211.541339,56.353274 210.144661,56.353274' id='Fill-5' sketch:type='MSShapeGroup'></path>\n";
		html_landing += "<path d='M196.260652,53.5846619 C196.065674,53.5846619 195.870696,53.5646979 195.675719,53.5247698 C194.056209,53.2053453 193.009696,51.6281867 193.332006,50.0071072 C194.649101,43.3431131 193.308131,36.5673204 189.551825,30.9294775 C185.795518,25.2876418 180.069538,21.4545475 173.428356,20.1289357 C171.808847,19.8095112 170.762334,18.2323526 171.084644,16.6072803 C171.402975,14.9862009 172.978714,13.9480712 174.594244,14.2555173 C182.795248,15.892568 189.874135,20.6320294 194.517789,27.5994768 C199.157465,34.5709171 200.816765,42.9438324 199.18134,51.1730068 C198.9028,52.5984387 197.65733,53.5846619 196.260652,53.5846619' id='Fill-7' sketch:type='MSShapeGroup'></path>\n";
		html_landing += "<path d='M182.380224,50.8164491 C182.185246,50.8164491 181.990268,50.7964851 181.79529,50.756557 C180.175781,50.4371325 179.129268,48.8599739 179.451578,47.2388945 C180.028553,44.3001888 179.439641,41.3095767 177.784319,38.826051 C176.125018,36.3385325 173.598265,34.6455826 170.66962,34.0626328 C169.05011,33.7432083 168.003597,32.1660497 168.325907,30.5409774 C168.644238,28.9238907 170.212019,27.8737826 171.835507,28.1892143 C176.327954,29.083603 180.199656,31.6789273 182.746304,35.4960504 C185.284995,39.3131734 186.196217,43.9009082 185.300911,48.404794 C185.022372,49.8302259 183.776901,50.8164491 182.380224,50.8164491' id='Fill-9' sketch:type='MSShapeGroup'></path>\n";
		html_landing += "<path d='M95.8650025,141.609676 C94.6434071,141.609676 93.807788,142.18464 93.3541663,143.330576 C93.1193971,143.937482 93.0000229,144.708094 93.0000229,145.646404 C93.0000229,147.127735 93.3979368,148.169857 94.1897853,148.768778 C94.6593236,149.120145 95.2203821,149.295829 95.8610234,149.295829 C96.7961209,149.295829 97.5083867,148.936476 98.0017998,148.217771 C98.4912338,147.503059 98.7379404,146.544785 98.7379404,145.350936 C98.7379404,144.368705 98.5111295,143.502266 98.0614869,142.743633 C97.6078651,141.988993 96.8757037,141.609676 95.8650025,141.609676' id='Fill-11' sketch:type='MSShapeGroup'></path>\n";
		html_landing += "<path d='M109.349109,36.7481945 C108.752238,36.2251368 108.00416,35.9695972 107.112833,35.9695972 C106.145902,35.9695972 105.397824,36.2411081 104.86462,36.7961082 C104.331415,37.3431227 103.997168,38.0897775 103.861877,39.0320798 L110.351851,39.0320798 C110.284206,38.0258926 109.949958,37.2672593 109.349109,36.7481945' id='Fill-13' sketch:type='MSShapeGroup'></path>\n";
		html_landing += "<path d='M110.207807,141.387676 C109.236897,141.387676 108.488819,141.731057 107.963573,142.421813 C107.442305,143.112569 107.175703,144.094799 107.175703,145.372497 C107.175703,146.646202 107.442305,147.632426 107.963573,148.323181 C108.488819,149.017929 109.236897,149.365304 110.207807,149.365304 C111.178716,149.365304 111.926794,149.017929 112.444082,148.323181 C112.965349,147.632426 113.227973,146.646202 113.227973,145.372497 C113.227973,144.094799 112.965349,143.112569 112.444082,142.421813 C111.926794,141.731057 111.178716,141.387676 110.207807,141.387676' id='Fill-15' sketch:type='MSShapeGroup'></path>\n";
		html_landing += "<path d='M124.406964,36.7481945 C123.810093,36.2251368 123.062015,35.9695972 122.170688,35.9695972 C121.203757,35.9695972 120.455679,36.2411081 119.922475,36.7961082 C119.38927,37.3431227 119.055023,38.0897775 118.919732,39.0320798 L125.409707,39.0320798 C125.342061,38.0258926 125.007814,37.2672593 124.406964,36.7481945' id='Fill-17' sketch:type='MSShapeGroup'></path>\n";
		html_landing += "<path d='M133.596386,130.627861 C113.139636,130.627861 96.5545872,113.985843 96.5545872,93.4588247 C96.5545872,72.9278133 113.139636,56.2857956 133.596386,56.2857956 C154.053136,56.2857956 170.638184,72.9278133 170.638184,93.4588247 C170.638184,95.4991488 169.746858,115.938326 170.33577,121.476348 C171.246993,130.016962 176.113479,130.627861 176.113479,130.627861 L133.596386,130.627861 Z M124.909927,141.468331 L122.844754,141.468331 L122.844754,148.208188 C122.844754,148.731246 122.912399,149.054663 123.043711,149.186426 C123.175023,149.310203 123.580895,149.378081 124.253369,149.378081 C124.356827,149.378081 124.460284,149.378081 124.575679,149.374088 C124.687095,149.370095 124.798511,149.362109 124.909927,149.354124 L124.909927,151.845635 L123.334188,151.905527 C121.762428,151.957434 120.69204,151.685923 120.115065,151.087002 C119.745005,150.703692 119.557986,150.112757 119.557986,149.318189 L119.557986,141.468331 L117.78329,141.468331 L117.78329,139.096604 L119.557986,139.096604 L119.557986,135.539013 L122.844754,135.539013 L122.844754,139.096604 L124.909927,139.096604 L124.909927,141.468331 Z M115.10533,150.168656 C114.030963,151.498261 112.403495,152.161067 110.218948,152.161067 C108.034401,152.161067 106.406934,151.498261 105.336545,150.168656 C104.262178,148.839052 103.724994,147.241929 103.724994,145.373296 C103.724994,143.532612 104.262178,141.939482 105.336545,140.593906 C106.406934,139.244338 108.034401,138.569553 110.218948,138.569553 C112.403495,138.569553 114.030963,139.244338 115.10533,140.593906 C116.175718,141.939482 116.712902,143.532612 116.712902,145.373296 C116.712902,147.241929 116.175718,148.839052 115.10533,150.168656 L115.10533,150.168656 Z M100.645141,150.324376 C99.6304611,151.502254 98.3292829,152.089196 96.7296693,152.089196 C95.714989,152.089196 94.8714117,151.837649 94.1989373,151.330563 C93.8288775,151.051067 93.470755,150.639807 93.1166117,150.100779 L93.1166117,156.748802 L89.8298434,156.748802 L89.8298434,138.97682 L93.0131541,138.97682 L93.0131541,140.861424 C93.3752557,140.306424 93.7532738,139.871208 94.1631251,139.551784 C94.9032448,138.980812 95.7905927,138.697323 96.8132312,138.697323 C98.3093872,138.697323 99.5707741,139.24833 100.609329,140.346352 C101.647884,141.444374 102.165172,143.061461 102.165172,145.189627 C102.165172,147.433584 101.659822,149.146498 100.645141,150.324376 L100.645141,150.324376 Z M88.2381881,31.6062582 L78.3898208,31.6062582 L78.3898208,36.1021584 L87.0205718,36.1021584 L87.0205718,39.496044 L78.3898208,39.496044 L78.3898208,47.6932758 L74.3430371,47.6932758 L74.3430371,28.1644589 L88.2381881,28.1644589 L88.2381881,31.6062582 Z M78.4296122,76.9325983 C78.4296122,75.8784974 78.8076303,75.0120584 79.5557083,74.3292884 C80.3117446,73.6465185 81.1791967,73.30713 82.1779605,73.30713 C83.1369328,73.30713 83.9884684,73.6145761 84.7325673,74.2334611 C85.472687,74.8523461 85.8427468,75.7507276 85.8427468,76.9325983 C85.8427468,78.074541 85.4806453,78.9729225 84.7604212,79.6157643 C84.0322389,80.2586062 83.172745,80.5820235 82.1779605,80.5820235 C81.1433845,80.5820235 80.2639949,80.2665918 79.5278543,79.6277427 C78.799672,78.9928865 78.4296122,78.094505 78.4296122,76.9325983 L78.4296122,76.9325983 Z M79.4482716,85.1617726 C80.1287042,84.4151178 81.0080938,84.0437868 82.0983777,84.0437868 C83.1807033,84.0437868 84.0799886,84.4151178 84.7882752,85.1617726 C85.4886035,85.9044347 85.8427468,86.9425644 85.8427468,88.2681761 L85.8427468,109.769439 C85.8427468,111.258756 85.4886035,112.384728 84.7882752,113.147354 C84.0799886,113.90998 83.1807033,114.293289 82.0983777,114.293289 C81.0080938,114.293289 80.1287042,113.898001 79.4482716,113.119404 C78.771818,112.336814 78.4296122,111.218828 78.4296122,109.769439 L78.4296122,88.4877805 C78.4296122,87.0144349 78.771818,85.9044347 79.4482716,85.1617726 L79.4482716,85.1617726 Z M85.8507051,150.611858 C84.6370679,151.649988 82.9260385,152.173045 80.7096584,152.173045 C78.4495078,152.173045 76.6668539,151.661966 75.3696548,150.635815 C74.0764348,149.609663 73.4278353,148.200203 73.4278353,146.407433 L76.8538734,146.407433 C76.9692684,147.194015 77.1841419,147.780958 77.5064521,148.172253 C78.107302,148.882973 79.1259614,149.238332 80.5703886,149.238332 C81.4338616,149.238332 82.1381691,149.142505 82.6753527,148.954843 C83.6979913,148.595491 84.2113001,147.928692 84.2113001,146.954447 C84.2113001,146.387468 83.9606144,145.944267 83.459243,145.632828 C82.9578716,145.329375 82.1620439,145.061857 81.07176,144.826281 L79.2135024,144.419015 C77.3830988,144.011749 76.1296702,143.57254 75.4452584,143.097396 C74.2873292,142.298835 73.706375,141.057072 73.706375,139.364122 C73.706375,137.818906 74.2753917,136.537215 75.405467,135.515056 C76.5395215,134.492898 78.2028013,133.981819 80.3992856,133.981819 C82.2336684,133.981819 83.7934906,134.464948 85.0906897,135.431207 C86.3839097,136.393474 87.0643423,137.786963 87.1240294,139.619661 L83.6741165,139.619661 C83.6144294,138.585524 83.1488702,137.846855 82.2893763,137.411639 C81.7163804,137.120164 81.0001355,136.976423 80.1485999,136.976423 C79.201565,136.976423 78.4455287,137.164085 77.8765119,137.539409 C77.3154534,137.914733 77.0289555,138.437791 77.0289555,139.108582 C77.0289555,139.723474 77.3114743,140.182647 77.8645745,140.4861 C78.2266761,140.689734 78.9866915,140.929302 80.1485999,141.200813 L83.1647868,141.915525 C84.4858607,142.226964 85.4766661,142.642216 86.1372031,143.165274 C87.1598416,143.975813 87.6731505,145.149698 87.6731505,146.686929 C87.6731505,148.264088 87.0683215,149.569735 85.8507051,150.611858 L85.8507051,150.611858 Z M71.5815151,76.0941089 C69.0030335,84.2394343 66.428531,92.935767 63.8500493,102.1871 C63.3208239,104.059726 62.8870979,105.461201 62.5369337,106.363575 C62.1867695,107.269943 61.5938779,108.104439 60.762238,108.887029 C59.9226398,109.669619 58.748794,110.19667 57.2287632,110.448217 C55.7883151,110.687785 54.6383441,110.548037 53.770892,110.016994 C52.907419,109.481957 52.3025899,108.811166 51.9683423,108.016597 C51.6261364,107.218036 51.152619,105.832532 50.5398317,103.828143 C48.0369537,95.2635732 45.5300965,86.2238592 43.0272185,76.7050083 C40.4925074,85.8126001 37.9577963,95.3993287 35.4230852,105.47318 C34.8301936,107.769043 34.3606552,109.446022 34.0104911,110.472173 C33.664306,111.498325 33.0554979,112.464584 32.1920249,113.390915 C31.3245727,114.313253 30.1746017,114.900196 28.7381328,115.135772 C27.5722453,115.331419 26.6172521,115.243577 25.8731532,114.880232 C25.1210961,114.516887 24.5122879,113.917966 24.0467287,113.091455 C23.5811695,112.256958 23.1991722,111.242785 22.9086951,110.04095 C22.6102598,108.835123 22.3476366,107.713144 22.1128675,106.671022 C19.5343859,94.9960552 16.9559042,82.7660887 14.3814017,69.9851149 C13.9158425,67.7611216 13.6850525,66.060186 13.6850525,64.9142506 C13.6850525,63.460869 14.1068412,62.2989623 14.9543976,61.4484945 C15.797975,60.5940339 16.8444883,60.2786021 18.0859795,60.4862281 C19.8009881,60.7697173 20.9509591,61.6002211 21.5438507,62.9497897 C22.1327632,64.307344 22.6540303,66.2079199 23.0996938,68.6435319 C25.1250752,79.1925268 27.1504566,89.3622051 29.1798171,99.1605525 C31.4519051,89.7215577 33.7239931,80.6778509 35.9960811,72.0294319 C36.5014317,70.0410142 36.9550534,68.5556901 37.3569464,67.5534957 C37.7588394,66.5513012 38.411418,65.7407615 39.3186616,65.1258693 C40.2298843,64.5029915 41.4634171,64.3313008 43.0272185,64.5908332 C44.6148947,64.8503657 45.8444484,65.4373082 46.723838,66.3316969 C47.5992484,67.2220928 48.2040775,68.124467 48.5423042,69.0468054 C48.8845101,69.9731365 49.3381319,71.4784245 49.9071487,73.5427055 C52.1991323,81.8796856 54.491116,89.8173851 56.7830997,97.3558039 C58.8124602,89.0068454 60.8418208,81.0332107 62.8672022,73.4348998 C63.1656376,72.2450434 63.4441772,71.3266979 63.7107795,70.6718776 C63.9734026,70.0170573 64.4270244,69.46605 65.0716448,69.0268413 C65.7122861,68.5916254 66.6513627,68.475834 67.8729582,68.6794671 C69.1025119,68.8791075 70.141067,69.3901867 70.9965818,70.1847552 C71.8560756,70.9753309 72.2778643,71.8537484 72.2778643,72.8399716 C72.2778643,73.5347199 72.0510534,74.6127777 71.5815151,76.0941089 L71.5815151,76.0941089 Z M90.5381301,33.2353233 L94.1352711,33.2353233 L94.1352711,35.7547842 C94.7162253,34.7925178 95.225555,34.1337048 95.655302,33.778345 C96.3596094,33.1874096 97.2748112,32.8879491 98.4009074,32.8879491 C98.4725319,32.8879491 98.5361981,32.8919419 98.5799686,32.8959347 C98.6316974,32.8999275 98.7391341,32.9079131 98.906258,32.9158988 L98.906258,36.7929139 C98.6675097,36.7649643 98.4566153,36.7450002 98.2695958,36.7370146 C98.0865555,36.729029 97.9353482,36.7250362 97.8239323,36.7250362 C96.3078806,36.7250362 95.2892212,37.2201442 94.7679541,38.2103603 C94.477477,38.7693532 94.334228,39.6238138 94.334228,40.7857205 L94.334228,47.6932758 L90.5381301,47.6932758 L90.5381301,33.2353233 Z M101.962236,34.8524099 C103.28331,33.5188125 105.002298,32.848021 107.11522,32.848021 C108.36467,32.848021 109.494745,33.075611 110.501467,33.5267982 C111.50421,33.9779853 112.33585,34.6926977 112.992408,35.6669425 C113.58132,36.5253959 113.967297,37.5196047 114.138399,38.6535618 C114.241857,39.3203605 114.285628,40.2786341 114.265732,41.5243897 L103.768765,41.5243897 C103.828452,42.9737785 104.333803,43.9919442 105.276858,44.5788867 C105.849854,44.9422321 106.542224,45.1179156 107.353969,45.1179156 C108.209483,45.1179156 108.909812,44.8983113 109.446995,44.4551097 C109.737472,44.2195341 109.996116,43.8881312 110.218948,43.4609009 L114.066775,43.4609009 C113.967297,44.3193543 113.497758,45.1897861 112.670097,46.0761892 C111.380857,47.4816571 109.574328,48.1843911 107.250511,48.1843911 C105.332566,48.1843911 103.641433,47.5934557 102.17711,46.4075921 C100.704828,45.2177358 99.9766462,43.2892102 99.9766462,40.6180226 C99.9766462,38.1145329 100.637183,36.1900001 101.962236,34.8524099 L101.962236,34.8524099 Z M117.019296,34.8524099 C118.34037,33.5188125 120.059357,32.848021 122.17228,32.848021 C123.421729,32.848021 124.555784,33.075611 125.562506,33.5267982 C126.565248,33.9779853 127.396888,34.6926977 128.053446,35.6669425 C128.63838,36.5253959 129.024356,37.5196047 129.199438,38.6535618 C129.298916,39.3203605 129.342687,40.2786341 129.322791,41.5243897 L118.825824,41.5243897 C118.885511,42.9737785 119.390862,43.9919442 120.333918,44.5788867 C120.910893,44.9422321 121.599284,45.1179156 122.415007,45.1179156 C123.266543,45.1179156 123.966871,44.8983113 124.504055,44.4551097 C124.794532,44.2195341 125.057155,43.8881312 125.276008,43.4609009 L129.123834,43.4609009 C129.024356,44.3193543 128.558797,45.1897861 127.731136,46.0761892 C126.437916,47.4816571 124.631387,48.1843911 122.30757,48.1843911 C120.389626,48.1843911 118.698492,47.5934557 117.234169,46.4075921 C115.765867,45.2177358 115.033706,43.2892102 115.033706,40.6180226 C115.033706,38.1145329 115.694243,36.1900001 117.019296,34.8524099 L117.019296,34.8524099 Z M183.701696,121.939514 C183.399281,113.187282 183.566405,95.4033215 183.657925,87.7930322 C183.661904,87.3737875 183.717612,86.9665212 183.717612,86.5472765 C183.717612,86.2957297 183.6818,86.0521685 183.6818,85.8006217 C183.693737,84.7944344 183.701696,84.1715566 183.701696,84.1715566 C183.701696,80.3304767 183.26797,76.7169868 182.507954,73.3031371 C176.236832,37.0125183 144.781744,9.39028248 106.824743,9.39028248 C84.0720303,9.39028248 63.6908838,19.3643133 49.6126924,35.1239208 L11.5641718,32.7681649 C5.17765478,32.7681649 0.000795827664,37.9628062 0.000795827664,44.3712608 L0.000795827664,128.723292 C0.000795827664,135.131747 5.17765478,140.326388 11.5641718,140.326388 L51.7455106,140.326388 C65.7162652,154.728441 85.214043,163.704271 106.824743,163.704271 C128.435443,163.704271 147.933221,154.728441 161.903976,140.326388 L191.659972,140.326388 C191.659972,140.326388 184.047881,131.74984 183.701696,121.939514 L183.701696,121.939514 Z' id='Fill-19' sketch:type='MSShapeGroup'></path>\n";
		html_landing += "</g>\n";
		html_landing += "</g>\n";
		html_landing += "</g>\n";
		html_landing += "</svg>\n";
		html_landing += "<div id='splash_template_title' class='splash_template_title'><#FreeWiFi_Welcome#></div>\n";
		html_landing += "<div id='splash_template_brand_name' class='splash_template_brand_name'>Brand Name</div>\n";
		if(passcode_status) {
			html_landing += "<input id='splash_template_passcode' name='splash_template_passcode' class='splash_template_passcode' value='' placeHolder='Please enter Passcode' type='text' maxlength='64' autocorrect='off' autocapitalize='off'>\n";
		}
		if(terms_service_status) {
			html_landing += "<div id='splash_template_terms_service' class='splash_template_terms_service'>\n";
				html_landing += "<div class='splash_template_terms_service_cb'>\n";
				html_landing += "<input type='checkbox' name='cbTermService' id='cbTermService' onclick='control_bt_status();'>\n";
				html_landing += "</div>\n";
			html_landing += "<div class='splash_template_terms_service_text'>\n";
			html_landing += "<#FreeWiFi_Agree_Terms_Service#>\n";
			html_landing += "</div>\n";
			html_landing += "<div style='clear:both;'></div>\n";
			html_landing += "</div>\n";
		}
		html_landing += "<div id='splash_template_continue' class='splash_template_continue' onclick='continue_action();'><#FreeWiFi_Continue#></div>\n";
		html_landing += "</div>\n";
		html_landing += "</div>\n";
		html_landing += "</body></html>\n";

	/*
	var html_landing_error = "";
		html_landing_error += "<html xmlns='http://www.w3.org/1999/xhtml' xmlns:v=''>\n";
		html_landing_error += "<head><meta http-equiv='X-UA-Compatible' content='IE=Edge'>\n";
		html_landing_error += "<meta http-equiv='Content-Type' content='text/html; charset=utf-8'>\n";
		html_landing_error += "<meta http-equiv='Pragma' content='no-cache'>\n";
		html_landing_error += "<meta http-equiv='Expires' content='-1'>\n";
		html_landing_error += "<title>" + brand_name + "</title>\n";
		html_landing_error += "<link rel='stylesheet' type='text/css' href='FeeUam.css'>\n";
		html_landing_error += "<script type='text/javascript'>\n";
		html_landing_error += "function initial() {\n";
		html_landing_error += "window.moveTo(0,0);\n";
		html_landing_error += "window.resizeTo(screen.availWidth,screen.availHeight);\n";
		html_landing_error += "}\n";
		//html_landing_error += "function close_action() {\n";
		//html_landing_error += "window.close();\n";
		//html_landing_error += "}\n";
		html_landing_error += "<\/script>\n";
		html_landing_error += "</head>\n";
		html_landing_error += "<body onload='initial();' id='splash_body' class='splash_body'>\n";
		html_landing_error += "<div class='splash_template_error_content'>\n";
		html_landing_error += "<div class='splash_template_error_icon'>\n";
		html_landing_error += "<svg class='splash_template_icon_error' viewBox='0 0 24 24' xmlns='http://www.w3.org/2000/svg'>\n";
		html_landing_error += "<path d='M0 0h24v24H0z' fill='none'/>\n";
		html_landing_error += "<path d='M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm1 15h-2v-2h2v2zm0-4h-2V7h2v6z'/>\n";
		html_landing_error += "</svg>\n";
		html_landing_error += "</div>\n";
		html_landing_error += "<div id='splash_template_error_desc' class='splash_template_error_desc'>Unable to connect to the Internet. Please check your Wi-Fi service administrator.</div>\n";
		//html_landing_error += "<div id='splash_template_error_close' class='splash_template_error_close' onclick='close_action();'>Close this window</div>\n";
		html_landing_error += "</div>\n";
		html_landing_error += "</body></html>\n";
	*/

	document.splash_page_form.splash_page_id.value = _profile_id;
	document.splash_page_form.splash_page_attribute.value = splash_page_setting;
	document.splash_page_form.splash_page_html.value = html_landing;
	document.splash_page_form.splash_page_css.value = html_landing_css;
	//document.splash_page_form.splash_page_error.value = html_landing_error;
	showLoading();
	document.splash_page_form.submit();
}
var folder_click_type = "";
// get folder tree
function get_disk_tree(_landingType){
	if(!disk_flag) {
		alert('<#no_usb_found#>');
		return false;	
	}

	$("#folderTree_panel").fadeIn(300);
	var scrollTop = $(document).scrollTop();
	$("#folderTree_panel").css({ top: (scrollTop + 200) + "px" });
	cal_panel_block("folderTree_panel", 0.3);
	get_layer_items("0");
}
function get_layer_items(layer_order){
	$.ajax({
		url: '/gettree.asp?layer_order=' + layer_order,
		dataType: 'script',
		success: function(){
			get_tree_items(treeitems);
		}
	});
}
function get_tree_items(treeitems){
	this.isLoading = 1;
	var array_temp = new Array();
	var array_temp_split = new Array();
	for(var j=0;j<treeitems.length;j++){
		//treeitems[j] : "Download2#22#0"
		array_temp_split[j] = treeitems[j].split("#"); 
		// Mipsel:asusware  Mipsbig:asusware.big  Armel:asusware.arm  // To hide folder 'asusware'
		if( array_temp_split[j][0].match(/^asusware$/)	|| array_temp_split[j][0].match(/^asusware.big$/) || array_temp_split[j][0].match(/^asusware.arm$/) ){
			continue;					
		}

		//Specific folder 'Download2/Complete'
		if( array_temp_split[j][0].match(/^Download2$/) ){
			treeitems[j] = "Download2/Complete"+"#"+array_temp_split[j][1]+"#"+array_temp_split[j][2];
		}		
		
		array_temp.push(treeitems[j]);
	}
	this.Items = array_temp;
	if(this.Items && this.Items.length >= 0){
		BuildTree();
	}	
}
function BuildTree(){
	var ItemText, ItemSub, ItemIcon;
	var vertline, isSubTree;
	var layer;
	var short_ItemText = "";
	var shown_ItemText = "";
	var ItemBarCode ="";
	var TempObject = "";

	for(var i = 0; i < this.Items.length; ++i){
		this.Items[i] = this.Items[i].split("#");
		var Item_size = 0;
		Item_size = this.Items[i].length;
		if(Item_size > 3){
			var temp_array = new Array(3);	
			
			temp_array[2] = this.Items[i][Item_size-1];
			temp_array[1] = this.Items[i][Item_size-2];			
			temp_array[0] = "";
			for(var j = 0; j < Item_size-2; ++j){
				if(j != 0)
					temp_array[0] += "#";
				temp_array[0] += this.Items[i][j];
			}
			this.Items[i] = temp_array;
		}	
		ItemText = (this.Items[i][0]).replace(/^[\s]+/gi,"").replace(/[\s]+$/gi,"");
		ItemBarCode = this.FromObject+"_"+(this.Items[i][1]).replace(/^[\s]+/gi,"").replace(/[\s]+$/gi,"");
		ItemSub = parseInt((this.Items[i][2]).replace(/^[\s]+/gi,"").replace(/[\s]+$/gi,""));
		layer = get_layer(ItemBarCode.substring(1));
		if(layer == 3){
			if(ItemText.length > 21)
		 		short_ItemText = ItemText.substring(0,18)+"...";
		 	else
		 		short_ItemText = ItemText;
		}
		else
			short_ItemText = ItemText;
		
		shown_ItemText = showhtmlspace(short_ItemText);
		
		if(layer == 1)
			ItemIcon = 'disk';
		else if(layer == 2)
			ItemIcon = 'part';
		else
			ItemIcon = 'folders';
		
		SubClick = ' onclick="GetFolderItem(this, ';
		if(ItemSub <= 0){
			SubClick += '0);"';
			isSubTree = 'n';
		}
		else{
			SubClick += '1);"';
			isSubTree = 's';
		}
		
		if(i == this.Items.length-1){
			vertline = '';
			isSubTree += '1';
		}
		else{
			vertline = ' background="/images/Tree/vert_line.gif"';
			isSubTree += '0';
		}
		
		TempObject +='<table class="tree_table" id="bug_test">';
		TempObject +='<tr>';
		// the line in the front.
		TempObject +='<td class="vert_line">';
		TempObject +='<img id="a'+ItemBarCode+'" onclick=\'document.getElementById("d'+ItemBarCode+'").onclick();\' class="FdRead" src="/images/Tree/vert_line_'+isSubTree+'0.gif">';
		TempObject +='</td>';
	
		if(layer == 3){
			/*a: connect_line b: harddisc+name  c:harddisc  d:name e: next layer forder*/
			TempObject +='<td>';		
			TempObject +='<img id="c'+ItemBarCode+'" onclick=\'document.getElementById("d'+ItemBarCode+'").onclick();\' src="/images/New_ui/advancesetting/'+ItemIcon+'.png">';
			TempObject +='</td>';
			TempObject +='<td>';
			TempObject +='<span id="d'+ItemBarCode+'"'+SubClick+' title="'+ItemText+'">'+shown_ItemText+'</span>';
			TempObject +='</td>';
		}
		else if(layer == 2){
			TempObject +='<td>';
			TempObject +='<table class="tree_table">';
			TempObject +='<tr>';
			TempObject +='<td class="vert_line">';
			TempObject +='<img id="c'+ItemBarCode+'" onclick=\'document.getElementById("d'+ItemBarCode+'").onclick();\' src="/images/New_ui/advancesetting/'+ItemIcon+'.png">';
			TempObject +='</td>';
			TempObject +='<td class="FdText">';
			TempObject +='<span id="d'+ItemBarCode+'"'+SubClick+' title="'+ItemText+'">'+shown_ItemText+'</span>';
			TempObject +='</td>\n';
			TempObject +='<td></td>';
			TempObject +='</tr>';
			TempObject +='</table>';
			TempObject +='</td>';
			TempObject +='</tr>';
			TempObject +='<tr><td></td>';
			TempObject +='<td colspan=2><div id="e'+ItemBarCode+'" ></div></td>';
		}
		else{
			/*a: connect_line b: harddisc+name  c:harddisc  d:name e: next layer forder*/
			TempObject +='<td>';
			TempObject +='<table><tr><td>';
			TempObject +='<img id="c'+ItemBarCode+'" onclick=\'document.getElementById("d'+ItemBarCode+'").onclick();\' src="/images/New_ui/advancesetting/'+ItemIcon+'.png">';
			TempObject +='</td><td>';
			TempObject +='<span id="d'+ItemBarCode+'"'+SubClick+' title="'+ItemText+'">'+shown_ItemText+'</span>';
			TempObject +='</td></tr></table>';
			TempObject +='</td>';
			TempObject +='</tr>';
			TempObject +='<tr><td></td>';
			TempObject +='<td><div id="e'+ItemBarCode+'" ></div></td>';
		}
		TempObject +='</tr>';
	}
	TempObject +='</table>';
	document.getElementById("e"+this.FromObject).innerHTML = TempObject;
}
function build_array(obj,layer){
	var path_temp ="/mnt";
	var layer2_path ="";
	var layer3_path ="";
	if(obj.id.length>6){
		if(layer ==3){
			//layer3_path = "/" + document.getElementById(obj.id).innerHTML;
			layer3_path = "/" + obj.title;
			while(layer3_path.indexOf("&nbsp;") != -1)
				layer3_path = layer3_path.replace("&nbsp;"," ");
				
			if(obj.id.length >8)
				layer2_path = "/" + document.getElementById(obj.id.substring(0,obj.id.length-3)).innerHTML;
			else
				layer2_path = "/" + document.getElementById(obj.id.substring(0,obj.id.length-2)).innerHTML;
			
			while(layer2_path.indexOf("&nbsp;") != -1)
				layer2_path = layer2_path.replace("&nbsp;"," ");
		}
	}
	if(obj.id.length>4 && obj.id.length<=6){
		if(layer ==2){
			//layer2_path = "/" + document.getElementById(obj.id).innerHTML;
			layer2_path = "/" + obj.title;
			while(layer2_path.indexOf("&nbsp;") != -1)
				layer2_path = layer2_path.replace("&nbsp;"," ");
		}
	}

	path_temp = path_temp + layer2_path +layer3_path;
	return path_temp;
}
function GetFolderItem(selectedObj, haveSubTree){
	var barcode, layer = 0;
	showClickedObj(selectedObj);
	barcode = selectedObj.id.substring(1);	
	layer = get_layer(barcode);
	
	if(layer == 0)
		alert("Machine: Wrong");
	else if(layer == 1){
		// chose Disk
		setSelectedDiskOrder(selectedObj.id);
		path_directory = build_array(selectedObj,layer);

	}
	else if(layer == 2){
		// chose Partition
		setSelectedPoolOrder(selectedObj.id);
		path_directory = build_array(selectedObj,layer);
	}
	else if(layer == 3){
		// chose Shared-Folder
		setSelectedFolderOrder(selectedObj.id);
		path_directory = build_array(selectedObj,layer);
	}

	if(haveSubTree)
		GetTree(barcode, 1);
}
function showClickedObj(clickedObj){
	if(this.lastClickedObj != 0)
		this.lastClickedObj.className = "landing_lastfolderClicked";  //this className set in AiDisk_style.css
	
	clickedObj.className = "landing_folderClicked";
	this.lastClickedObj = clickedObj;
}
function GetTree(layer_order, v){
	if(layer_order == "0"){
		this.FromObject = layer_order;
		document.getElementById('d'+layer_order).innerHTML = '<span class="FdWait">. . . . . . . . . .</span>';
		setTimeout('get_layer_items("'+layer_order+'", "gettree")', 1);		
		return;
	}
	
	if(document.getElementById('a'+layer_order).className == "FdRead"){
		document.getElementById('a'+layer_order).className = "FdOpen";
		document.getElementById('a'+layer_order).src = "/images/Tree/vert_line_s"+v+"1.gif";		
		this.FromObject = layer_order;		
		document.getElementById('e'+layer_order).innerHTML = '<img src="/images/Tree/folder_wait.gif">';
		setTimeout('get_layer_items("'+layer_order+'", "gettree")', 1);
	}
	else if(document.getElementById('a'+layer_order).className == "FdOpen"){
		document.getElementById('a'+layer_order).className = "FdClose";
		document.getElementById('a'+layer_order).src = "/images/Tree/vert_line_s"+v+"0.gif";		
		document.getElementById('e'+layer_order).style.position = "absolute";
		document.getElementById('e'+layer_order).style.visibility = "hidden";
	}
	else if(document.getElementById('a'+layer_order).className == "FdClose"){
		document.getElementById('a'+layer_order).className = "FdOpen";
		document.getElementById('a'+layer_order).src = "/images/Tree/vert_line_s"+v+"1.gif";		
		document.getElementById('e'+layer_order).style.position = "";
		document.getElementById('e'+layer_order).style.visibility = "";
	}
	else
		alert("Error when show the folder-tree!");
}
function cancel_folderTree(){
	this.FromObject = "0";
	$("#folderTree_panel").fadeOut(300);
}
function confirm_folderTree(){
	document.getElementById('usb_dir').value = path_directory ;
	this.FromObject = "0";
	$("#folderTree_panel").fadeOut(300);
}
function encode_decode_text(_string, _type) {
	var _string_temp = _string;
	switch(_type) {
		case "encode" :
			//escaped character
			 _string_temp = _string_temp.replace(/"/g, '\"').replace(/\\/g, "\\");
			//replace new line
			_string_temp = _string_temp.replace(/(?:\r\n|\r|\n)/g, '<br>');
			//encode ASCII
			_string_temp = encodeURIComponent(_string_temp).replace(/[!'()*]/g, escape);
			break;
		case "decode" :
			_string_temp = decodeURIComponent(_string_temp);
			_string_temp = _string_temp.replace(/\<br\>/g, "\n");
			break
	}
	
	return _string_temp;
}
var third_party_wl_used = new Array();
function create_third_party_wl_used() {
	var _enable_flag = "";
	var _third_party_wl_used = new Array();
	for(var i = 1; i <= multissid_count; i += 1) {
		_third_party_wl_used[i] = "not_used";
	}

	//check captive portal advanced used wl if
	_enable_flag = '<% nvram_get("captive_portal_adv_enable"); %>';
	if(_enable_flag == "on") {
		var _captive_portal_adv_row = decodeURIComponent('<% nvram_char_to_ascii("","captive_portal_adv_profile"); %>').split("<");
		for(var j = 0; j < _captive_portal_adv_row.length; j += 1) {
			if(_captive_portal_adv_row[j] != "") {
				var _cpa_wl_list = _captive_portal_adv_row[j].split(">")[5];
				var _cpa_wl_if = _cpa_wl_list.split("wl");
				while(_cpa_wl_if.length) {
					if(_cpa_wl_if[0] != "") {
						var _cpa_wl_group = _cpa_wl_if[0].split(".")[1];
						_third_party_wl_used[_cpa_wl_group] = "Captive Portal";
					}
					_cpa_wl_if.shift();
				}
			}
		}
	}
	//check fb wi-fi used wl if
	_enable_flag = '<% nvram_get("fbwifi_enable"); %>';
	if(_enable_flag == "on") {
		var _fb_wl_group = "";
		var _fbwifi_2g_index = '<% nvram_get("fbwifi_2g"); %>';
		if(_fbwifi_2g_index != "off") {
			_fb_wl_group = _fbwifi_2g_index.split(".")[1];
			_third_party_wl_used[_fb_wl_group] = "Facebook Wi-Fi";
		}
		if(wl_info.band5g_support) {
			var _fbwifi_5g_index = '<% nvram_get("fbwifi_5g"); %>';
			_fb_wl_group = _fbwifi_5g_index.split(".")[1];
			_third_party_wl_used[_fb_wl_group] = "Facebook Wi-Fi";
		}
		if(wl_info.band5g_2_support) {
			var _fbwifi_5g_2_index = '<% nvram_get("fbwifi_5g_2"); %>';
			_fb_wl_group = _fbwifi_5g_2_index.split(".")[1];
			_third_party_wl_used[_fb_wl_group] = "Facebook Wi-Fi";
		}
	}
	return _third_party_wl_used;
}
function change_wl_input_status(_idx) {
	if($("#cb_wl_" + _idx + "").prop("checked")) {
		if($("input[name=wl_" + _idx + "]").hasClass("wl_input_disabled")) {
			$("input[name=wl_" + _idx + "]").removeClass("wl_input_disabled");
			$("input[name=wl_" + _idx + "]").attr('disabled', false);
		}
	}
	else {
		$("input[name=wl_" + _idx + "]").addClass("wl_input_disabled");
		$("input[name=wl_" + _idx + "]").attr('disabled', true);
	}
}
function find_empty_gn_group() {
	var _empty_wl_idx = "";
	var _empty_flag = false;
	var _gn_count = multissid_count;
	for(_gn_count; _gn_count > 0; _gn_count -= 1) {
		_empty_flag = (gn_array_2g[(_gn_count - 1)][0] == "0") ? true : false;
		if(!_empty_flag)
			continue;
		if(wl_info.band5g_support) {
			_empty_flag = (gn_array_5g[(_gn_count - 1)][0] == "0") ? true : false;
			if(!_empty_flag)
				continue;
		}
		if(wl_info.band5g_2_support) {
			_empty_flag = (gn_array_5g_2[(_gn_count - 1)][0] == "0") ? true : false;
			if(!_empty_flag)
				continue;
		}
		if(_empty_flag) {
			_empty_wl_idx = _gn_count;
		}
		break;
	}
	return _empty_wl_idx;
}
function check_gn_status(_subunit) {
	var _gn_status = false;

	_gn_status = (gn_array_2g[(_subunit - 1)][0] == "0") ? true : false;
	if(!_gn_status)
		return _gn_status;
	if(wl_info.band5g_support) {
		_gn_status = (gn_array_5g[(_subunit - 1)][0] == "0") ? true : false;
		if(!_gn_status)
			return _gn_status;
	}
	if(wl_info.band5g_2_support) {
		_gn_status = (gn_array_5g_2[(_subunit - 1)][0] == "0") ? true : false;
		if(!_gn_status)
			return _gn_status;
	}

	return _gn_status;
}
function check_gn_if_status(_subunit, _gn_array) {
	var _gn_status = false;
	_gn_status = (_gn_array[(_subunit - 1)][0] == "0") ? true : false;
	return _gn_status;
}
</script>

</head>

<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="full_screen_bg" class="full_screen_bg" onselectstart="return false;"></div>
<div id="guestnetwork_wl" class="guestnetwork_wl"></div>
<div id='folderTree_panel' class='landing_folder_content'>
<div class='landing_folder_title'><#Web_Title2#></div>
	<div id='e0' class='folder_tree'></div>
	<div class='landing_folder_bottom'>		
		<input class='button_gen' type='button' onclick='cancel_folderTree();' value='<#CTL_Cancel#>'>
		<input class='button_gen' type='button' style='margin-left:15px;' onclick='confirm_folderTree();' value='<#CTL_ok#>'>
	</div>
</div>
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="form" action="/start_apply2.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Captive_Portal.asp">
<input type="hidden" name="next_page" value="Captive_Portal.asp">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply_new">
<input type="hidden" name="action_script" value="set_captive_portal_wl;restart_wireless;restart_chilli;restart_uam_srv;">
<input type="hidden" name="action_wait" value="50">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="captive_portal" value="">
<input type="hidden" name="captive_portal_2g" value="">
<input type="hidden" name="captive_portal_5g" value="">
<input type="hidden" name="captive_portal_5g_2" value="">
<input type="hidden" name="captive_portal_2g_if" value="">
<input type="hidden" name="captive_portal_5g_if" value="">
<input type="hidden" name="captive_portal_5g_2_if" value="">
<input type="hidden" name="captive_portal_enable" value="<% nvram_get("captive_portal_enable"); %>">
<input type="hidden" name="captive_portal_passcode" value="">

<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="17">&nbsp;</td>

		<!--=====Beginning of Main Menu=====-->
		<td valign="top" width="202">
			<div id="mainMenu"></div>
			<div id="subMenu"></div>
		</td>

		<td valign="top">
			<div id="tabMenu" class="submenuBlock"></div>
			<!--===================================Beginning of Main Content===========================================-->
			<div id="captive_portal_setting" class="captive_portal_setting_bg"></div>
			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
				<tr>
					<td align="left" valign="top">
						<table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
							<tbody>
							<tr>
								<td bgcolor="#4D595D" valign="top">
									<div>&nbsp;</div>
									<div class="formfonttitle"><#Guest_Network#> - Free Wi-Fi<!--untranslated--></div>
									<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
									<div class="cp_page_intro_icon"></div>
									<div style='float:left;width:80%;'>
									<div class="cp_page_intro_txt" style="color:#FC0;"><#FreeWiFi_desc1#></div>
									<div class="cp_page_intro_txt"><#FreeWiFi_desc2#></div>
									<div class="cp_page_intro_txt brt_series">
										<#FAQ_Find#> : <a id="faq" href="" target="_blank" style="font-weight:bolder;text-decoration:underline;" href="" target="_blank">GO</a>
									</div>
									<div align="center" class="left" style="float:left;margin-left:20px;margin-top:10px;cursor:pointer;" id="radio_captive_portal_enable"></div>
									<div class="iphone_switch_container" style="height:32px; width:74px; position: relative; overflow: hidden;"></div>
									<script type="text/javascript">
										var captivePortalEnable = 0;
										if(document.form.captive_portal_enable.value == "on") {
											captivePortalEnable = 1;
										}
										$('#radio_captive_portal_enable').iphoneSwitch(captivePortalEnable,
											function() {
												captivePortalShowAndHide(1);
											},
											function() {
												captivePortalShowAndHide(0);		
											},
											{
												switch_on_container_path: '/switcherplugin/iphone_switch_container_off.png'
											}
										);
									</script>
									</div>
									<div style="clear:both;"></div>
									<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
									<div id="captive_portal_content" class="captive_portal_content"></div>
									<div class='apply_content'>
										<input id='apply_button' class='button_gen' onclick='apply();' type='button' value=''/>
									</div>
								</td>
							</tr>
							</tbody>
						</table>
					</td>
				</tr>
			</table>
		</td>
		<td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
<!--===================================End of Main Content===========================================-->
</form>
<iframe name="hidden_frame_save" id="hidden_frame_save" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="splash_page_form" action="splash_page.cgi" target="hidden_frame_save" enctype="multipart/form-data">
<input type="hidden" name="current_page" value="Captive_Portal.asp">
<input type="hidden" name="next_page" value="Captive_Portal.asp">
<input type="hidden" name="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="splash_page_id" value="">
<input type="hidden" name="splash_page_attribute" value="">
<input type="hidden" name="splash_page_html" value="">
<input type="hidden" name="splash_page_css" value="">
<input type="hidden" name="splash_page_error" value="" disabled>
</form>

<div id="footer"></div>
</body>
</html>
