var openvpn_faq_href = {
	"windows": "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=119",
	"macOS": "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=120",
	"iPhone": "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=121",
	"android": "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=122"
};

var openvpn_clientlist_data = [];
var openvpnd_connected_clients = [];
var openvpn_clientlist_limit = 16;
var openvpn_allowed_clientlist_data = [];
var openvpn_allowed_clientlist_limit = 128;
var ciphersarray = [
	["AES-128-CBC"],
	["AES-192-CBC"],
	["AES-256-CBC"],
	["AES-128-GCM"],
	["AES-192-GCM"],
	["AES-256-GCM"],
	["BF-CBC"],
	["CAST5-CBC"],
	["CAMELLIA-128-CBC"],
	["CAMELLIA-192-CBC"],
	["CAMELLIA-256-CBC"],
	["DES-CBC"],
	["DES-EDE-CBC"],
	["DES-EDE3-CBC"],
	["DESX-CBC"],
	["IDEA-CBC"],
	["RC2-40-CBC"],
	["RC2-64-CBC"],
	["RC2-CBC"],
	["RC5-CBC"],
	["SEED-CBC"]
];
var hmacarray = [
	["MD 5", "MD5"],
	["SHA 1", "SHA1"],
	["SHA 224", "SHA224"],
	["SHA 256", "SHA256"],
	["SHA 384", "SHA384"],
	["SHA 512", "SHA512"],
	["RIPEMD 160", "RIPEMD160"],
	["RSA MD4", "RSA-MD4"]
];
var ipv6_item_flag = false;
if(isSupport("ipv6")){
	var ipv6_service = httpApi.nvramGet(["ipv6_service"]).ipv6_service;
	if(ipv6_service != "disabled" && ipv6_service != ""){
		ipv6_item_flag = true;
	}
}
var openvpn_clientlist_attr = function(){
	this.username = "";
	this.password = "";
};
var openvpn_allowed_clientlist_attr = function(){
	this.username = "";
	this.subnet = "";
	this.netmask = "";
	this.push = "";
};
function show_popup_help_OpenVPN(_type){
	var init_environment = function(){
		$(".container").addClass("blur_effect");
		if($(".popup_container.popup_element").css("display") == "flex"){
			$(".popup_container.popup_element").addClass("blur_effect");
		}
		$(".popup_element_second").css("display", "flex");
		$(".popup_container.popup_element_second").empty();
	};
	if(_type == "Feature_Desc" || _type == "Feature_Desc_Adv" || _type == "Cert"){
		init_environment();
		if(_type == "Feature_Desc"){
			$(".popup_container.popup_element_second").append(Get_Component_Feature_Desc_OpenVPN("Feature_Desc"));
		}
		else if(_type == "Feature_Desc_Adv"){
			$(".popup_container.popup_element_second").append(Get_Component_Feature_Desc_OpenVPN("Feature_Desc_Adv"));
		}
		else if(_type == "Cert"){
			$(".popup_container.popup_element_second").append(Get_Component_Feature_Desc_OpenVPN("Cert"));
		}
		adjust_popup_container_top($(".popup_container.popup_element_second"), 100);
	}
}
function Get_Component_Feature_Desc_OpenVPN(_type){
	var $container = $("<div>");

	var $popup_title_container = $("<div>").addClass("popup_title_container");
	$popup_title_container.appendTo($container);
	$("<div>").addClass("title").html("About Feature").appendTo($popup_title_container);/* untranslated */
	var $close_btn = $("<div>").addClass("vpn_icon_all_collect close_btn");
	$close_btn.appendTo($popup_title_container);
	$close_btn.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		close_popup_second();
	});

	var $popup_content_container = $("<div>").addClass("popup_content_container");
	$popup_content_container.appendTo($container);

	if(_type == "Feature_Desc"){
		var $feature_desc = $("<div>").addClass("feature_desc");
		$feature_desc.appendTo($popup_content_container);
		$("<div>").addClass("title").html("Feature Description").appendTo($feature_desc);/* untranslated */
		var $openvpn_desc = $("#openvpn_desc");
		if(isMobile()){
			$openvpn_desc.find("#desc3").empty();
		}
		var header_info = httpApi.hookGet("get_header_info");
		if(header_info.protocol == "https"){
			$openvpn_desc.html($openvpn_desc.html().replaceAll("http", "https"));
		}
		$("<div>").addClass("desc").html($openvpn_desc.html()).appendTo($feature_desc);

		if(privateIP_flag){
			var $privateIP_notes = $(".hidden_static_text #privateIP_notes");
			$privateIP_notes.find("#faq_port_forwarding").attr("href", faq_port_forwarding_href);
			$("<div>").addClass("desc").html($privateIP_notes.html()).appendTo($feature_desc);
		}

		$("<div>").addClass("title").html("<#HOWTOSETUP#>").appendTo($feature_desc);

		var $step_text_container = $("<div>").addClass("step_text_container");
		$step_text_container.appendTo($feature_desc);

		var $faq_windows = $("<a/>").attr({"id":"faq_windows", "target":"_blank", "href":openvpn_faq_href.windows}).html("Windows");
		$("<div>").addClass("step_text faq hyperlink").append($faq_windows).appendTo($step_text_container);

		var $faq_macOS = $("<a/>").attr({"id":"faq_macOS", "target":"_blank", "href":openvpn_faq_href.macOS}).html("Mac OS");
		$("<div>").addClass("step_text faq hyperlink").append($faq_macOS).appendTo($step_text_container);

		var $faq_iPhone = $("<a/>").attr({"id":"faq_iPhone", "target":"_blank", "href":openvpn_faq_href.iPhone}).html("iPhone/iPad");
		$("<div>").addClass("step_text faq hyperlink").append($faq_iPhone).appendTo($step_text_container);

		var $faq_android = $("<a/>").attr({"id":"faq_android", "target":"_blank", "href":openvpn_faq_href.android}).html("Android");
		$("<div>").addClass("step_text faq hyperlink").append($faq_android).appendTo($step_text_container);
	}
	else if(_type == "Feature_Desc_Adv"){
		var $feature_desc = $("<div>").addClass("feature_desc");
		$feature_desc.appendTo($popup_content_container);
		$("<div>").addClass("title").html("<#menu5#>").appendTo($feature_desc);
		var $openvpn_desc_adv = $("#openvpn_desc_adv");
		if(isMobile()){
			$openvpn_desc_adv.find("#desc2").empty();
			$openvpn_desc_adv.find("#desc3 a").removeAttr("href").css("text-decoration", "initial");
		}
		else
			$openvpn_desc_adv.find("#desc3 a").attr({"href":"/"+$openvpn_desc_adv.find("#desc3 a").attr("href")+"", "target":"_blank"});

		$("<div>").addClass("desc no_divider").html($openvpn_desc_adv.html()).appendTo($feature_desc);
	}
	else if(_type == "Cert"){
		var $feature_desc = $("<div>").addClass("feature_desc");
		$feature_desc.appendTo($popup_content_container);
		$("<div>").addClass("title").html("Certification Tranfer").appendTo($feature_desc);/* untranslated */
		var $openvpn_desc_cert = $("#openvpn_desc_cert");
		$("<div>").addClass("desc no_divider").html($openvpn_desc_cert.html()).appendTo($feature_desc);
	}

	return $container;
}
function Get_Component_Export_Ovpn(_parm){
	var $container = $("<div>").addClass("profile_setting_item nowrap btn_item");
	if(_parm.openHint != undefined){
		var hint_array = _parm.openHint.split("_");
		$("<a>").addClass("hintstyle").attr({"href":"javascript:void(0);"}).html(htmlEnDeCode.htmlEncode(_parm.title)).unbind("click").click(function(){
			openHint(hint_array[0], hint_array[1], "rwd_vpns");
		}).appendTo($("<div>").addClass("title").appendTo($container));
	}
	else
		$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)).appendTo($container);

	var $input_container = $("<div>").addClass("input_container").appendTo($container);
	var $btn_container = $("<div>").attr({"id":"openvpn_export"}).addClass("btn_container").unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		location.href = '/client.ovpn';
	}).appendTo($input_container);
	$("<div>").addClass("text import_file").html(htmlEnDeCode.htmlEncode("<#btn_Export#>")).appendTo($btn_container);
	$("<div>").attr({"id":"openvpn_initial"}).addClass("pure_text loadingicon").html("<#vpn_openvpn_init#>").appendTo($input_container).hide();
	$("<div>").attr({"id":"openvpn_error_message"}).addClass("pure_text").appendTo($input_container).hide();

	return $container;
}
function Get_Component_Import_Cert(_parm){
	var $container = $("<div>").addClass("profile_setting_item nowrap btn_item");
	if(_parm.openHint != undefined){
		var hint_array = _parm.openHint.split("_");
		$("<a>").addClass("hintstyle").attr({"href":"javascript:void(0);"}).html(htmlEnDeCode.htmlEncode(_parm.title)).unbind("click").click(function(){
			openHint(hint_array[0], hint_array[1], "rwd_vpns");
		}).appendTo($("<div>").addClass("title").appendTo($container));
	}
	else
		$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)).appendTo($container);

	var $input_container = $("<div>").addClass("input_container").appendTo($container);
	var $btn_item =  $("<div>").appendTo($input_container);
	var $btn_container = $("<div>").addClass("btn_container").appendTo($btn_item);
	var $text = $("<div>").addClass("text import_file").html(htmlEnDeCode.htmlEncode("<#CTL_upload#>")).appendTo($btn_container);
	$("<div>").addClass("status_text").appendTo($btn_item).hide();
	$("<input/>").attr({"id":"import_file", "type":"file", "name":"import_cert_file", "accept":".cert"}).hide().appendTo($btn_item)
		.on("change", function(){
			var $input_btn = $(this);
			$(this).closest(".btn_item").find(".text.import_file").removeClass("loadingicon");
			$(this).closest(".btn_item").find(".status_text").html("").hide();
			var file_name = $(this).val().toUpperCase();
			if(file_name == ""){
				$(this).closest(".btn_item").find(".status_text").html("<#JS_fieldblank#>").show();
				return false;
			}

			if(file_name.length < 6 || file_name.lastIndexOf(".CERT") < 0 ||
				file_name.lastIndexOf(".CERT") != (file_name.length) - 5)
			{
				$(this).closest(".btn_item").find(".status_text").html("<#Setting_upload_hint#>").show();
				return false;
			}

			$(this).closest(".btn_item").find(".text.import_file").addClass("loadingicon");
			$(this).closest(".btn_item").find(".status_text").html("").hide();

			var postData = {
				"import_cert_file": $(this).prop('files')[0]
			};
			httpApi.uploadServerOvpnCert(postData, function(response){
				if(response.indexOf("callback_upload_cert(1)") >= 0){
					show_customize_alert("<#FW_updated#>");
					$("#srv_profile_list").children("[type='" + select_vpn_type + "']").addClass("selected").find(".svr_item_text_container").click();
				}
				else{
					$input_btn.closest(".btn_item").find(".text.import_file").removeClass("loadingicon");
					$input_btn.closest(".btn_item").find(".status_text").html("<#SET_fail_desc#>").show();
				}
			});
		});

	$btn_container.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		if(!$(this).closest(".btn_item").find(".text.import_file").hasClass("loadingicon"))
			$(this).closest(".btn_item").find("#import_file").click();
	});

	return $container;
}
function Get_Component_Export_Cert(_parm){
	var $container = $("<div>").addClass("profile_setting_item nowrap btn_item");
	if(_parm.openHint != undefined){
		var hint_array = _parm.openHint.split("_");
		$("<a>").addClass("hintstyle").attr({"href":"javascript:void(0);"}).html(htmlEnDeCode.htmlEncode(_parm.title)).unbind("click").click(function(){
			openHint(hint_array[0], hint_array[1], "rwd_vpns");
		}).appendTo($("<div>").addClass("title").appendTo($container));
	}
	else
		$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)).appendTo($container);

	var $input_container = $("<div>").addClass("input_container").appendTo($container);
	var $btn_container = $("<div>").addClass("btn_container").unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		location.href = '/server_ovpn.cert';
	}).appendTo($input_container);
	$("<div>").addClass("text import_file").html(htmlEnDeCode.htmlEncode("<#btn_Export#>")).appendTo($btn_container);

	return $container;
}
function Get_Component_Renew_Cert(_parm){
	var $container = $("<div>").addClass("profile_setting_item nowrap btn_item");
	if(_parm.openHint != undefined){
		var hint_array = _parm.openHint.split("_");
		$("<a>").addClass("hintstyle").attr({"href":"javascript:void(0);"}).html(htmlEnDeCode.htmlEncode(_parm.title)).unbind("click").click(function(){
			openHint(hint_array[0], hint_array[1], "rwd_vpns");
		}).appendTo($("<div>").addClass("title").appendTo($container));
	}
	else
		$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)).appendTo($container);

	var $input_container = $("<div>").addClass("input_container").appendTo($container);
	var $btn_container = $("<div>").addClass("btn_container").unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		if(!$(this).find(".text.import_file").hasClass("loadingicon")){
			$(this).find(".text.import_file").removeClass("loadingicon").addClass("loadingicon");
			httpApi.nvramSet({
				"vpn_crt_server1_ca" : "",
				"vpn_crt_server1_crt" : "",
				"vpn_crt_server1_key" : "",
				"vpn_crt_server1_dh" : "",
				"vpn_crt_server1_crl" : "",
				"rc_service": "restart_openvpnd",
				"action_mode": "apply"
			}, function(){
				var count = 0;
				var timer = 10;
				var interval_check = setInterval(function(){
					var vpn_server1_state = httpApi.nvramGet(["vpn_server1_state"], true).vpn_server1_state;
					if(vpn_server1_state == "2"){
						clearInterval(interval_check);
						$btn_container.find(".text.import_file").removeClass("loadingicon");
						show_customize_alert("Update certification successfully, please export new OpenVPN configuration file and install in your VPN client.");/* untranslated */
					}
					else{
						count++;
						if(count >= timer){
							clearInterval(interval_check);
							$btn_container.find(".text.import_file").removeClass("loadingicon");
							show_customize_alert("<#vpn_ipsec_update_cert_fail#>");
						}
					}
				}, 2000);
			});
		}
	}).appendTo($input_container);
	$("<div>").addClass("text import_file").html(htmlEnDeCode.htmlEncode("<#CTL_renew#>")).appendTo($btn_container);

	return $container;
}
function show_popup_Add_Client_OpenVPN(){
	$(".container").addClass("blur_effect");
	if($(".popup_container.popup_element").css("display") == "flex"){
		$(".popup_container.popup_element").addClass("blur_effect");
	}
	$(".popup_element_second").css("display", "flex");
	$(".popup_container.popup_element_second").empty();
	$(".popup_container.popup_element_second").append(Get_Component_Add_Client_OpenVPN());
	adjust_popup_container_top($(".popup_container.popup_element_second"), 100);
}
function Get_Component_Add_Client_OpenVPN(){
	var set_apply_btn_status = function(_profileObj){
		var $btn_container_apply = $(_profileObj).find(".action_container .btn_container.apply");
		var validate_blank_flag = validate_blank_OpenVPN($(_profileObj), "openvpn_clientlist");
		if(!validate_blank_flag){
			$btn_container_apply.removeClass("valid_fail").addClass("valid_fail").unbind("click");
		}
		else{
			$btn_container_apply.removeClass("valid_fail").unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				if(validate_format_OpenVPN($(_profileObj), "openvpn_clientlist")){
					var client_profile = new openvpn_clientlist_attr();
					client_profile.username = $(_profileObj).find("#clientlist_username").val();
					client_profile.password = $(_profileObj).find("#clientlist_password").val();
					openvpn_clientlist_data.push(JSON.parse(JSON.stringify(client_profile)));
					show_openvpn_clientlist($("#openvpn_clientlist_bg"));
					resize_iframe_height();
					close_popup_second();
				}
			});
		}
	};
	var $container = $("<div>");

	var $popup_title_container = $("<div>").addClass("popup_title_container").appendTo($container);
	$("<div>").addClass("title").html("<#Username_Pwd#>").appendTo($popup_title_container);
	var $close_btn = $("<div>").addClass("vpn_icon_all_collect close_btn");
	$close_btn.appendTo($popup_title_container);
	$close_btn.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		close_popup_second();
	});

	var $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);

	var clientlist_username = {"title":"<#Username#>", "type":"text", "id":"clientlist_username", "need_check":true, "maxlength":64};
	Get_Component_Input(clientlist_username).appendTo($content_container)
		.find("#" + clientlist_username.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isString(this, event);
		})
		.val(htmlEnDeCode.htmlEncode());

	var clientlist_password = {"title":"<#HSDPAConfig_Password_itemname#>", "type":"text", "id":"clientlist_password", "need_check":true, "maxlength":64};
	Get_Component_Input(clientlist_password).appendTo($content_container)
		.find("#" + clientlist_password.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isString(this, event);
		})
		.unbind("keyup").keyup(function(){
			chkPass($(this).val(), "rwd_vpn_pwd", $("#client_pwd_strength"));
		})
		.unbind("blur").blur(function(){
			if($(this).val() == "")
				$("#client_pwd_strength").hide();
		})
		.val(htmlEnDeCode.htmlEncode());

	$("<div>").attr({"id":"client_pwd_strength"}).append(Get_Component_PWD_Strength_Meter()).appendTo($content_container).hide();

	var $action_container = $("<div>").addClass("action_container").appendTo($content_container);
	var $btn_container_apply = $("<div>").addClass("btn_container apply").appendTo($action_container).html("<#CTL_ok#>");

	$content_container.find("[need_check=true]").keyup(function(e){
		e = e || event;
		e.stopPropagation();
		set_apply_btn_status($content_container);
	});
	set_apply_btn_status($content_container);

	return $container;
}
function show_popup_Add_Allowed_Client_OpenVPN(_obj){
	$(".container").addClass("blur_effect");
	if($(".popup_container.popup_element").css("display") == "flex"){
		$(".popup_container.popup_element").addClass("blur_effect");
	}
	$(".popup_element_second").css("display", "flex");
	$(".popup_container.popup_element_second").empty();
	$(".popup_container.popup_element_second").append(Get_Component_Add_Allowed_Client_OpenVPN(_obj));
	adjust_popup_container_top($(".popup_container.popup_element_second"), 100);
}
function Get_Component_Add_Allowed_Client_OpenVPN(_obj){
	var set_apply_btn_status = function(_profileObj){
		var $btn_container_apply = $(_profileObj).find(".action_container .btn_container.apply");
		var validate_blank_flag = validate_blank_OpenVPN($(_profileObj), "openvpn_allowed_clientlist");
		if(!validate_blank_flag){
			$btn_container_apply.removeClass("valid_fail").addClass("valid_fail").unbind("click");
		}
		else{
			$btn_container_apply.removeClass("valid_fail").unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				if(validate_format_OpenVPN($(_profileObj), "openvpn_allowed_clientlist")){
					var client_profile = new openvpn_allowed_clientlist_attr();
					client_profile.username = $(_profileObj).find("#vpn_clientlist_commonname_0").val();
					client_profile.subnet = $(_profileObj).find("#vpn_clientlist_subnet_0").val();
					client_profile.netmask = $(_profileObj).find("#vpn_clientlist_netmask_0").val();
					client_profile.push = ($(_profileObj).find("#vpn_clientlist_push_0").hasClass("on") ? "1" : "0");
					openvpn_allowed_clientlist_data.push(JSON.parse(JSON.stringify(client_profile)));
					show_openvpn_allowed_clientlist($(_obj).find("#openvpn_allowed_clientlist_bg"));
					resize_iframe_height();
					close_popup_second();
				}
			});
		}
	};
	var $container = $("<div>");

	var $popup_title_container = $("<div>").addClass("popup_title_container").appendTo($container);
	$("<div>").addClass("title").html("Allowed Clients").appendTo($popup_title_container);/* untranslated */
	var $close_btn = $("<div>").addClass("vpn_icon_all_collect close_btn");
	$close_btn.appendTo($popup_title_container);
	$close_btn.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		close_popup_second();
	});

	var $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);

	var common_name = {"title":"Common Name(CN)", "type":"text", "id":"vpn_clientlist_commonname_0", "need_check":true, "maxlength":25, "openHint":"32_22"};
	common_name.title = ($(_obj).find("#vpn_server_igncrt").hasClass("on")) ? "<#Username#>" : "Common Name(CN)";/* untranslated */
	Get_Component_Input(common_name).appendTo($content_container);

	var subnet = {"title":"<#Subnet#>", "type":"text", "id":"vpn_clientlist_subnet_0", "need_check":true, "maxlength":15, "openHint":"32_23"};
	Get_Component_Input(subnet).appendTo($content_container)
		.find("#" + subnet.id + "").unbind("keypress").keypress(function(){
		return validator.isIPAddr(this, event);
	});

	var subnet = {"title":"<#RouterConfig_GWStaticMask_itemname#>", "type":"text", "id":"vpn_clientlist_netmask_0", "need_check":true, "maxlength":15, "openHint":"32_24"};
	Get_Component_Input(subnet).appendTo($content_container)
		.find("#" + subnet.id + "").unbind("keypress").keypress(function(){
		return validator.isIPAddr(this, event);
	});

	var push = {"title":"<#Push#>", "type":"switch", "id":"vpn_clientlist_push_0", "openHint":"32_25"};
	Get_Component_Switch(push).appendTo($content_container);

	var $action_container = $("<div>").addClass("action_container").appendTo($content_container);
	var $btn_container_apply = $("<div>").addClass("btn_container apply").appendTo($action_container).html("<#CTL_ok#>");

	$content_container.find("[need_check=true]").keyup(function(e){
		e = e || event;
		e.stopPropagation();
		set_apply_btn_status($content_container);
	});
	set_apply_btn_status($content_container);

	return $container;
}
var interval_openvpn_client_status = false;
function show_openvpn_clientlist(_obj){
	$(_obj).empty();
	if(openvpn_clientlist_data.length == 0){
		$(_obj).append(Get_Component_Client_No_Item());
	}
	else{
		$.each(openvpn_clientlist_data, function(index, value){
			Get_Component_Client_List_OpenVPN(value).appendTo($(_obj));
		});

		update_openvpn_client_status(_obj);
		clearInterval(interval_openvpn_client_status);
		interval_openvpn_client_status = setInterval(function(){
			update_openvpn_client_status(_obj);
		}, 1000*3);
	}
	var title_id = $(_obj).attr("id").replace("_bg", "_title");
	$(_obj).closest(".popup_edit_profile_container").find("#"+title_id+" #vpns_clientlist_num").html(htmlEnDeCode.htmlEncode(openvpn_clientlist_data.length));
}
function Get_Component_Client_List_OpenVPN(_client_info){
	var $container = $("<div>").addClass("profile_setting_item nowrap clientlist");
	var $client_content_bg = $("<div>").attr({"username":htmlEnDeCode.htmlEncode(_client_info.username)}).addClass("client_content_bg").appendTo($container);
	var $client_info_bg = $("<div>").addClass("client_info_bg").appendTo($client_content_bg);

	$("<div>").addClass("vpn_icon_all_collect status_icon").appendTo($client_info_bg);
	$("<div>").addClass("client_name").html(htmlEnDeCode.htmlEncode(_client_info.username)).appendTo($client_info_bg);

	if(_client_info.username != httpApi.nvramGet(["http_username"]).http_username){
		var $client_contorl_bg = $("<div>").addClass("client_contorl_bg").appendTo($client_content_bg);
		$("<div>").addClass("detail_icon").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			var hidden_contorl_width = $(this).closest(".client_contorl_bg").width() - $(this).width();
			if($(this).hasClass("active")){
				$(this).removeClass("active").closest(".client_contorl_bg").animate({"margin-right": "-=" + hidden_contorl_width + "px"}, 500);
			}
			else{
				$(this).closest("#openvpn_clientlist_bg").find(".detail_icon.active").removeClass("active")
					.closest(".client_contorl_bg").animate({"margin-right": "-=" + hidden_contorl_width + "px"}, 500);
				$(this).addClass("active").closest(".client_contorl_bg").animate({"margin-right": "+=" + hidden_contorl_width + "px"}, 500);
			}
		}).appendTo($client_contorl_bg);

		$("<div>").addClass("contorl_btn vpn_icon_all_collect del_icon").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			var del_idx = "";
			$.each(openvpn_clientlist_data, function(index, item){
				if(item.username == _client_info.username){
					del_idx = index;
					return false;
				}
			});
			if(del_idx !== ""){
				openvpn_clientlist_data.splice(del_idx, 1);
				show_openvpn_clientlist($("#openvpn_clientlist_bg"));
				resize_iframe_height();
			}
		}).appendTo($client_contorl_bg);

		var margin_right = (48 * $client_contorl_bg.find(".contorl_btn").length) + 10;//contorl_btn width * count + detail_icon margin right
		$client_contorl_bg.css({"margin-right": -margin_right});
	}

	return $container;
}
function show_openvpn_allowed_clientlist(_obj){
	$(_obj).empty();
	if(openvpn_allowed_clientlist_data.length == 0){
		$(_obj).append(Get_Component_Client_No_Item());
	}
	else{
		$.each(openvpn_allowed_clientlist_data, function(index, value){
			Get_Component_Allowed_Client_List_OpenVPN(value).appendTo($(_obj));
		});
	}
	var title_id = $(_obj).attr("id").replace("_bg", "_title");
	$(_obj).closest(".popup_edit_profile_container").find("#"+title_id+" #vpns_clientlist_num").html(htmlEnDeCode.htmlEncode(openvpn_allowed_clientlist_data.length));
}
function Get_Component_Allowed_Client_List_OpenVPN(_client_info){
	var $container = $("<div>").addClass("profile_setting_item allowed_clientlist");

	var $client_content_bg = $("<div>").addClass("client_content_bg").appendTo($container);
	var $client_info_bg = $("<div>").addClass("client_info_bg").appendTo($client_content_bg);
	$("<div>").addClass("push_pin " + ((_client_info.push == "1") ? "active" : "") + "")
		.attr({"title":function(){
			if(_client_info.push == "1")
				return "<#Push#> : <#checkbox_Yes#>";
			else
				return "<#Push#> : <#checkbox_No#>";
		}}).appendTo($client_info_bg);
	$("<div>").addClass("client_name").html(htmlEnDeCode.htmlEncode(_client_info.username)).appendTo($client_info_bg);

	var $client_content_bg = $("<div>").addClass("client_content_bg").appendTo($container);
	var $client_info_bg = $("<div>").addClass("client_info_bg").appendTo($client_content_bg);
	$("<div>").addClass("client_subnet").html(htmlEnDeCode.htmlEncode(_client_info.subnet + " / " + _client_info.netmask)).appendTo($client_info_bg);

	var $client_contorl_bg = $("<div>").addClass("client_contorl_bg").appendTo($client_content_bg);
	$("<div>").addClass("detail_icon").unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		var hidden_contorl_width = $(this).closest(".client_contorl_bg").width() - $(this).width();
		if($(this).hasClass("active")){
			$(this).removeClass("active").closest(".client_contorl_bg").animate({"margin-right": "-=" + hidden_contorl_width + "px"}, 500);
		}
		else{
			$(this).closest("#openvpn_clientlist_bg").find(".detail_icon.active").removeClass("active")
				.closest(".client_contorl_bg").animate({"margin-right": "-=" + hidden_contorl_width + "px"}, 500);
			$(this).addClass("active").closest(".client_contorl_bg").animate({"margin-right": "+=" + hidden_contorl_width + "px"}, 500);
		}
	}).appendTo($client_contorl_bg);

	$("<div>").addClass("contorl_btn vpn_icon_all_collect del_icon").unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		var del_idx = "";
		$.each(openvpn_allowed_clientlist_data, function(index, item){
			if(item.username == _client_info.username && item.subnet == _client_info.subnet && item.netmask == _client_info.netmask){
				del_idx = index;
				return false;
			}
		});
		if(del_idx !== ""){
			openvpn_allowed_clientlist_data.splice(del_idx, 1);
			show_openvpn_allowed_clientlist($("#openvpn_allowed_clientlist_bg"));
			resize_iframe_height();
		}
	}).appendTo($client_contorl_bg);

	var margin_right = (48 * $client_contorl_bg.find(".contorl_btn").length) + 10;//contorl_btn width * count + detail_icon margin right
	$client_contorl_bg.css({"margin-right": -margin_right});

	return $container;
}
function Get_Component_Edit_Keys(_parm){
	var $container = $("<div>").addClass("profile_setting_item nowrap keys_item");
	$container.append($("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)));

	var $input_container = $("<div>").addClass("input_container").appendTo($container);
	$("<div>").addClass("vpn_icon_all_collect vpnc_keys_icon").appendTo($input_container);

	return $container;
}
function show_popup_Keys_Cert(_obj){
	$(".container").addClass("blur_effect");
	if($(".popup_container.popup_element").css("display") == "flex"){
		$(".popup_container.popup_element").addClass("blur_effect");
	}
	$(".popup_element_second").css("display", "flex");
	$(".popup_container.popup_element_second").empty();
	$(".popup_container.popup_element_second").addClass("textarea_width");
	$(".popup_container.popup_element_second").append(Get_Component_Keys_Cert(_obj));
	adjust_popup_container_top($(".popup_container.popup_element_second"), 50);
	resize_iframe_height();
}
function Get_Component_Keys_Cert(_obj){
	var $container = $("<div>");

	var $popup_title_container = $("<div>").addClass("popup_title_container").appendTo($container);
	$("<div>").addClass("title").html("<#vpn_openvpn_Keys_Cert#>").appendTo($popup_title_container);
	var $close_btn = $("<div>").addClass("vpn_icon_all_collect close_btn");
	$close_btn.appendTo($popup_title_container);
	$close_btn.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		close_popup_second();
	});

	var $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);

	var edit_keys_desc = "<div><#vpn_openvpn_KC_Edit1#></div>";
	edit_keys_desc += "<div><span style='color:#FC0;'>----- BEGIN xxx -----</span> / <span style='color:#FC0;'>----- END xxx -----</span>";
	edit_keys_desc += " <#vpn_openvpn_KC_Edit2#></div>";
	edit_keys_desc += "<div><#vpn_openvpn_KC_Limit#></div>";
	$("<div>").addClass("profile_title_item edit_keys_desc").html(edit_keys_desc).appendTo($content_container);

	var auth = $(_obj).find("#select_vpn_server_crypt").children(".selected").attr("value");

	if(auth == "tls"){
		var vpn_crt_server1_ca_parm = {"title":"<#vpn_openvpn_KC_CA#>", "id":"edit_vpn_crt_server1_ca", "rows":"8", "cols":"65", "maxlength":"3999"};
		Get_Component_Textarea(vpn_crt_server1_ca_parm).appendTo($content_container);

		var vpn_crt_server1_crt_parm = {"title":"<#vpn_openvpn_KC_SA#>", "id":"edit_vpn_crt_server1_crt", "rows":"8", "cols":"65", "maxlength":"3999"};
		Get_Component_Textarea(vpn_crt_server1_crt_parm).appendTo($content_container);

		var vpn_crt_server1_key_parm = {"title":"<#vpn_openvpn_KC_SK#>", "id":"edit_vpn_crt_server1_key", "rows":"8", "cols":"65", "maxlength":"3999"};
		Get_Component_Textarea(vpn_crt_server1_key_parm).appendTo($content_container);

		var vpn_crt_server1_dh_parm = {"title":"<#vpn_openvpn_KC_DH#>", "id":"edit_vpn_crt_server1_dh", "rows":"8", "cols":"65", "maxlength":"3999"};
		Get_Component_Textarea(vpn_crt_server1_dh_parm).appendTo($content_container);

		var vpn_crt_server1_crl_parm = {"title":"<#vpnc_Cert_revocList#> (<#feedback_optional#>)", "id":"edit_vpn_crt_server1_crl", "rows":"8", "cols":"65", "maxlength":"3999"};
		Get_Component_Textarea(vpn_crt_server1_crl_parm).appendTo($content_container);
	}
	else if(auth == "secret"){
		var vpn_crt_server1_static_parm = {"title":"<#vpn_openvpn_KC_StaticK#>", "id":"edit_vpn_crt_server1_static", "rows":"8", "cols":"65", "maxlength":"3999"};
		Get_Component_Textarea(vpn_crt_server1_static_parm).appendTo($content_container);
	}

	var $action_container = $("<div>").addClass("action_container").appendTo($content_container);
	$("<div>").addClass("btn_container apply").appendTo($action_container).html("<#CTL_ok#>").unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		if(auth == 'tls'){
			httpApi.nvramSet({
				"action_mode": "apply",
				"vpn_crt_server1_ca" : $content_container.find("#edit_vpn_crt_server1_ca").val(),
				"vpn_crt_server1_crt" : $content_container.find("#edit_vpn_crt_server1_crt").val(),
				"vpn_crt_server1_key" : $content_container.find("#edit_vpn_crt_server1_key").val(),
				"vpn_crt_server1_dh" : $content_container.find("#edit_vpn_crt_server1_dh").val(),
				"vpn_crt_server1_crl" : $content_container.find("#edit_vpn_crt_server1_crl").val()
			});
		}
		else if(auth == 'secret'){
			httpApi.nvramSet({
				"action_mode": "apply",
				"vpn_crt_server1_static" : $content_container.find("#vpn_crt_server1_static").val()
			});
		}
		close_popup_second();
	});

	Update_Keys_Cert($content_container, auth);

	return $container;
}
function Update_Keys_Cert(_obj, _auth){
	var vpn_crt_server = httpApi.hookGet("vpn_crt_server", true);
	if(_auth == "tls"){
		$(_obj).find("#edit_vpn_crt_server1_ca").val(htmlEnDeCode.htmlEncode(vpn_crt_server.vpn_crt_server1_ca[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r")));
		$(_obj).find("#edit_vpn_crt_server1_crt").val(htmlEnDeCode.htmlEncode(vpn_crt_server.vpn_crt_server1_crt[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r")));
		$(_obj).find("#edit_vpn_crt_server1_key").val(htmlEnDeCode.htmlEncode(vpn_crt_server.vpn_crt_server1_key[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r")));
		$(_obj).find("#edit_vpn_crt_server1_dh").val(htmlEnDeCode.htmlEncode(vpn_crt_server.vpn_crt_server1_dh[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r")));
		$(_obj).find("#edit_vpn_crt_server1_crl").val(htmlEnDeCode.htmlEncode(vpn_crt_server.vpn_crt_server1_crl[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r")));
	}
	else if(_auth == "secret"){
		$(_obj).find("#edit_vpn_crt_server1_static").val(htmlEnDeCode.htmlEncode(vpn_crt_server.vpn_crt_server1_static[0].replace(/&#10/g, "\n").replace(/&#13/g, "\r")));
	}
}
function check_vpn_server_state(_obj){
	var openvpn_settings = httpApi.nvramGet(["VPNServer_enable", "vpn_server_unit", "vpn_server1_state", "vpn_server2_state"]);
	var service_state = false;
	if (openvpn_settings.vpn_server_unit == '1')
		service_state = openvpn_settings.vpn_server1_state;
	else if (openvpn_settings.vpn_server_unit == '2')
		service_state = openvpn_settings.vpn_server2_state;

	if(openvpn_settings.VPNServer_enable == '1' && service_state != '2'){
		$(_obj).find("#openvpn_export").hide();
		$(_obj).find("#openvpn_initial").show();
		update_vpn_server_state(_obj);
	}
}
function update_vpn_server_state(_obj){
	var openvpn_settings = httpApi.nvramGet(["vpn_server1_state", "vpn_server1_errno"]);
	var vpnd_state = openvpn_settings.vpn_server1_state;
	var vpn_server1_errno = openvpn_settings.vpn_server1_errno;
	if(vpnd_state != '2' && (vpn_server1_errno == '1' || vpn_server1_errno == '2')){ 
		$(_obj).find("#openvpn_initial").hide();
		$(_obj).find("#openvpn_error_message").show().html("<#vpn_openvpn_fail1#>");
	}
	else if(vpnd_state != '2' && vpn_server1_errno == '4'){
		$(_obj).find("#openvpn_initial").hide();
		$(_obj).find("#openvpn_error_message").show().html("<#vpn_openvpn_fail2#>");
	}
	else if(vpnd_state != '2' && vpn_server1_errno == '5'){
		$(_obj).find("#openvpn_initial").hide();
		$(_obj).find("#openvpn_error_message").show().html("<#vpn_openvpn_fail3#>");
	}
	else if((vpnd_state == '-1' && vpn_server1_errno == '0') || (vpnd_state != '2' && vpn_server1_errno == '7')){
		$(_obj).find("#openvpn_initial").hide();
		$(_obj).find("#openvpn_error_message").show().html("<#vpn_openvpn_fail4#>");
	}
	else if(vpnd_state != '2'){
		setTimeout(function(){
			update_vpn_server_state(_obj);
		}, 1000);
	}
	else{	// OpenVPN server ready , vpn_server1_state==2
		setTimeout(function(){
			sessionStorage.setItem("select_vpn_type", select_vpn_type);
			location.reload();
		}, 1000);
	}
}
function parseOpenVPNClients(client_status){ //192.168.123.82:46954 10.8.0.6 pine\n
	openvpnd_connected_clients = [];
	var Loginfo = client_status;
	if (Loginfo == "") {return;}
	Loginfo = Loginfo.replace('\r\n', '\n');
	Loginfo = Loginfo.replace('\n\r', '\n');
	Loginfo = Loginfo.replace('\r', '\n');

	var lines = Loginfo.split('\n');
	for (i = 0; i < lines.length; i++){
		var fields = lines[i].split(' ');
		if ( fields.length != 3 ) continue;
		openvpnd_connected_clients.push({
			"username": htmlEnDeCode.htmlEncode(fields[2]),
			"remoteIP": htmlEnDeCode.htmlEncode(fields[0]),
			"VPNIP": htmlEnDeCode.htmlEncode(fields[1])
		});
	}
}
function openvpnd_connected_status(_obj){
	if(select_vpn_type == "openvpn"){
		$(_obj).find(".status_icon").removeClass("connected").unbind("click").attr({"title":"<#Disconnected#>"});
		$.each(openvpnd_connected_clients, function(index, value){
			if($(_obj).find("[username='"+value.username+"']").length){
				if(!$(_obj).find("[username='"+value.username+"']").find(".status_icon").hasClass("connected")){
					$(_obj).find("[username='"+value.username+"']").find(".status_icon").addClass("connected").attr({"title":"<#Connected#>"});
					$(_obj).find("[username='"+value.username+"']").find(".status_icon").unbind("click").click(function(){
						showOpenVPNClients(value.username);
					});
				}
			}
		});
	}
}
function update_openvpn_client_status(_obj){
	if(httpApi.nvramGet(["VPNServer_enable"]).VPNServer_enable == "1"){
		$.ajax({
			url: '/ajax_openvpn_client_status.xml',
			dataType: 'xml',
			error: function(xml){
				update_openvpn_client_status(_obj);
			},
			success: function(xml){
				var vpnserverXML = xml.getElementsByTagName("vpnserver");
				var client_status = vpnserverXML[0].firstChild.nodeValue;
				parseOpenVPNClients(client_status);
				openvpnd_connected_status(_obj);
			}
		});
	}
}
function showOpenVPNClients(uname){
	$(".container").addClass("blur_effect");
	if($(".popup_container.popup_element").css("display") == "flex"){
		$(".popup_container.popup_element").addClass("blur_effect");
	}
	$(".popup_element_second").css("display", "flex");
	$(".popup_container.popup_element_second").empty();

	var Get_Clients_Connect_Status = function(){
		var $container = $("<div>").addClass("client_connect_info_container");
		var $popup_title_container = $("<div>").addClass("popup_title_container").appendTo($container);
		$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(uname)).appendTo($popup_title_container);
		var $close_btn = $("<div>").addClass("vpn_icon_all_collect close_btn");
		$close_btn.appendTo($popup_title_container);
		$close_btn.unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			close_popup_second();
		});

		var $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);
		var idx = 1;
		$.each(openvpnd_connected_clients, function(index, value){
			if(value.username == uname){
				$("<div>").addClass("profile_title_item connect_client_title").append($("<span>").html("<#statusTitle_Client#> - " + idx)).appendTo($content_container);
				Get_Component_Pure_Text({"title":"VPN IP", "text":value.VPNIP}).appendTo($content_container);/* untranslated */
				Get_Component_Pure_Text({"title":"Remote IP", "text":value.remoteIP}).appendTo($content_container);/* untranslated */
				idx++;
			}
		});

		return $container;
	};

	$(".popup_container.popup_element_second").append(Get_Clients_Connect_Status());

	adjust_popup_container_top($(".popup_container.popup_element_second"), 100);
}
function validate_blank_OpenVPN(_obj, _validField){
	if(_validField == "openvpn_profile"){
		var status = false;
		$(_obj).find("[need_check=true]").each(function(index){
			if($(this).closest(".profile_setting_item").css("display") == "none")
				return true;

			var value = $(this).val().replace(/\s+/g, '');//remove space
			if(value == ""){
				status = true;
				return false;
			}
		});
		if(status)
			return false;
	}
	else if(_validField == "openvpn_clientlist"){
		var status = false;
		$(_obj).find("[need_check=true]").each(function(index){
			var value = $(this).val().replace(/\s+/g, '');//remove space
			if(value == ""){
				status = true;
				return false;
			}
		});
		if(status)
			return false;
	}
	else if(_validField == "openvpn_allowed_clientlist"){
		var status = false;
		$(_obj).find("[need_check=true]").each(function(index){
			var value = $(this).val().replace(/\s+/g, '');//remove space
			if(value == ""){
				status = true;
				return false;
			}
		});
		if(status)
			return false;
	}
	return true;
}
function validate_format_OpenVPN(_obj, _validField){
	$(_obj).find(".validate_hint").remove();
	var valid_block_chars = function(str, keywordArray){
		var testResult = {
			'isError': false,
			'errReason': ''
		};

		// bolck ascii code 32~126 first
		var invalid_char = "";
		for(var i = 0; i < str.length; ++i){
			if(str.charCodeAt(i) < '32' || str.charCodeAt(i) > '126'){
				invalid_char += str.charAt(i);
			}
		}
		if(invalid_char != ""){
			testResult.isError = true;
			testResult.errReason = '<#JS_validstr2#>" '+ invalid_char +'" !';
			return testResult;
		}

		// check if char in the specified array
		if(str){
			for(var i=0; i<keywordArray.length; i++){
				if(str.indexOf(keywordArray[i]) >= 0){
					testResult.isError = true;
					testResult.errReason = keywordArray + " <#JS_invalid_chars#>";
					return testResult;
				}
			}
		}

		return testResult;
	};
	var valid_isLegalIP = function(str){
		var testResult = {
			'isError': false,
			'errReason': ''
		};
		var A_class_start = inet_network("1.0.0.0");
		var A_class_end = inet_network("126.255.255.255");
		var B_class_start = inet_network("127.0.0.0");
		var B_class_end = inet_network("127.255.255.255");
		var C_class_start = inet_network("128.0.0.0");
		var C_class_end = inet_network("255.255.255.255");
		var ip_num = inet_network(str);
		if(ip_num > A_class_start && ip_num < A_class_end){
			return testResult;
		}
		else if(ip_num > B_class_start && ip_num < B_class_end){
			testResult.isError = true;
			testResult.errReason = str + " <#JS_validip#>";
			return testResult;
		}
		else if(ip_num > C_class_start && ip_num < C_class_end){
			return testResult;
		}
		else{
			testResult.isError = true;
			testResult.errReason = str + " <#JS_validip#>";
			return testResult;
		}
	};
	var valid_IP_CIDR = function(addr, type, mode){
		//mode, 0:IP, 1:IP/CIDR, 2:IP or IP/CIDR
		var testResultPass = {
			'isError': false,
			'errReason': ''
		};
		var testResultFail = {
			'isError': true,
			'errReason': addr + " <#JS_validip#>"
		};
		var IP = new RegExp(ip_RegExp[type],"gi");
		var IP_CIDR = new RegExp(ip_RegExp[type + "_CIDR"], "gi");
		if(mode == "0"){
			if(IP.test(addr))
				return testResultPass;
			else{
				testResultFail.errReason = testResultFail.errReason + ", IP Address without CIDR."
				return testResultFail;
			}
		}
		else if(mode == "1"){
			if(IP_CIDR.test(addr))
				return testResultPass;
			else{
				testResultFail.errReason = testResultFail.errReason + ", IP Address/CIDR"
				return testResultFail;
			}
		}
		else if(mode == "2"){
			if(IP_CIDR.test(addr) || IP.test(addr))
				return testResultPass;
			else{
				testResultFail.errReason = testResultFail.errReason + ", IP Address without CIDR or IP Address/CIDR."
				return testResultFail;
			}
		}
		else
			return testResultFail;
	};
	var valid_is_IP_format = function(str, type){
		var testResultPass = {
			'isError': false,
			'errReason': ''
		};
		var testResultFail = {
			'isError': true,
			'errReason': str + " <#JS_validip#>"
		};
		var format = new RegExp(ip_RegExp[type], "gi");
		if(format.test(str))
			return testResultPass;
		else
			return testResultFail;
	};
	var valid_num_range = function(str, mini, maxi){
		var testResult = {
			'isError': true,
			'errReason': '<#JS_validrange#> ' + mini + ' <#JS_validrange_to#> ' + maxi
		};

		if(isNaN(str))
			return testResult;
		else{
			var input_num = parseInt(str);
			var mini_num = parseInt(mini);
			var maxi_num = parseInt(maxi);
			if(input_num < mini_num || input_num > maxi_num)
				return testResult;
			else{
				testResult.isError = false;
				testResult.errReason = "";
				return testResult;
			}
		}
	};
	var valid_isLegalMask = function(str){
		var testResult = {
			'isError': false,
			'errReason': ''
		};
		var wrong_netmask = 0;
		var netmask_num = inet_network(str);
		var netmask_reverse_num = 0;
		var test_num = 0;
		if(netmask_num != -1) {
			if(netmask_num == 0) {
				netmask_reverse_num = 0; //Viz 2011.07 : Let netmask 0.0.0.0 pass
			}
			else {
				netmask_reverse_num = ~netmask_num;
			}

			if(netmask_num < 0) {
				wrong_netmask = 1;
			}

			test_num = netmask_reverse_num;
			while(test_num != 0){
				if((test_num + 1) % 2 == 0) {
					test_num = (test_num + 1) / 2 - 1;
				}
				else{
					wrong_netmask = 1;
					break;
				}
			}
			if(wrong_netmask == 1){
				testResult.isError = true;
				testResult.errReason = str + " is not a valid Mask address!";
				return testResult;
			}
			else {
				return testResult;
			}
		}
		else { //null
			testResult.isError = true;
			testResult.errReason = "This is not a valid Mask address!";
			return testResult;
		}
	}

	if(_validField == "openvpn_profile"){
		var $vpn_server_port = $(_obj).find("#vpn_server_port");
		$vpn_server_port.val($vpn_server_port.val().replace(/\s+/g, ''));//remove space
		var $vpn_server_reneg = $(_obj).find("#vpn_server_reneg");
		$vpn_server_reneg.val($vpn_server_reneg.val().replace(/\s+/g, ''));//remove space

		if($vpn_server_port.val() == ""){
			$vpn_server_port.show_validate_hint("<#JS_fieldblank#>");
			$vpn_server_port.focus();
			return false;
		}
		var isValid_vpn_server_port = valid_num_range($vpn_server_port.val(), 1024, 65535);
		if(isValid_vpn_server_port.isError){
			$vpn_server_port.show_validate_hint(isValid_vpn_server_port.errReason);
			$vpn_server_port.focus();
			return false;
		}
		var conflict_result = isPortConflict($vpn_server_port.val(), "openvpn");
		if(conflict_result){
			$vpn_server_port.show_validate_hint(conflict_result);
			$vpn_server_port.focus();
			return false;
		}

		if((wan_proto == "v6plus" || wan_proto == "ocnvc") && s46_ports_check_flag && array_ipv6_s46_ports.length > 1){
			if (!validator.range_s46_ports($vpn_server_port[0], "none")){
				if(!confirm(port_confirm)){
					$vpn_server_port.focus();
					return false;
				}
			}
		}

		if($vpn_server_reneg.val() == ""){
			$vpn_server_reneg.show_validate_hint("<#JS_fieldblank#>");
			$vpn_server_reneg.focus();
			return false;
		}
		var isValid_vpn_server_reneg = valid_num_range($vpn_server_reneg.val(), -1, 99999);
		if(isValid_vpn_server_reneg.isError){
			$vpn_server_reneg.show_validate_hint(isValid_vpn_server_reneg.errReason);
			$vpn_server_reneg.focus();
			return false;
		}
		var vpn_server_if = $(_obj).find("#select_vpn_server_if").children(".selected").attr("value");
		var check_openvpn_conflict = function () {	//if conflict with LAN ip & DHCP ip pool & static
			var used_ip_pool = httpApi.nvramGet(["dhcp_start", "dhcp_end", "lan_ipaddr", "dhcp_staticlist"]);
			var origin_lan_ip = used_ip_pool.lan_ipaddr;
			var lan_ip_subnet = origin_lan_ip.split(".")[0]+"."+origin_lan_ip.split(".")[1]+"."+origin_lan_ip.split(".")[2]+".";
			var lan_ip_end = parseInt(origin_lan_ip.split(".")[3]);
			var pool_start = used_ip_pool.dhcp_start;
			var pool_end = used_ip_pool.dhcp_end;
			var pool_subnet = pool_start.split(".")[0]+"."+pool_start.split(".")[1]+"."+pool_start.split(".")[2]+".";
			var dhcp_staticlists = used_ip_pool.dhcp_staticlist.replace(/&#62/g, ">").replace(/&#60/g, "<");
			var staticclist_row = dhcp_staticlists.split('<');

			var $vpn_server_dhcp = $(_obj).find("#vpn_server_dhcp");
			if(vpn_server_if == 'tun'){
				var $vpn_server_sn = $(_obj).find("#vpn_server_sn");
				$vpn_server_sn.val($vpn_server_sn.val().replace(/\s+/g, ''));//remove space
				var $vpn_server_nm = $(_obj).find("#vpn_server_nm");
				$vpn_server_nm.val($vpn_server_nm.val().replace(/\s+/g, ''));//remove space

				if($vpn_server_sn.val() == ""){
					$vpn_server_sn.show_validate_hint("<#JS_fieldblank#>");
					$vpn_server_sn.focus();
					return false;
				}

				var isValid_vpn_server_sn = valid_is_IP_format($vpn_server_sn.val(), "IPv4");
				if(isValid_vpn_server_sn.isError){
					$vpn_server_sn.show_validate_hint(isValid_vpn_server_sn.errReason);
					$vpn_server_sn.focus();
					return false;
				}

				var openvpn_server_array = $vpn_server_sn.val().split(".");
				var openvpn_server_subnet = openvpn_server_array[0] + "." + openvpn_server_array[1] + "." + openvpn_server_array[2] + ".";

				//LAN ip
				if(origin_lan_ip == $vpn_server_sn.val()){
					$vpn_server_sn.show_validate_hint("<#vpn_conflict_LANIP#> " + origin_lan_ip);
					$vpn_server_sn.focus();
					return false;
				}

				//LAN ip pool
				if(lan_ip_subnet == openvpn_server_subnet){
					$vpn_server_sn.show_validate_hint("<#vpn_conflict_DHCPpool#>"+pool_start+" ~ "+pool_end);
					$vpn_server_sn.focus();
					return false;
				}

				//mask must be 255.255.0.0 ~ 255.255.255.248 openvpn limiting
				if(!validator.maskRange("255.255.0.0", "255.255.255.248", $vpn_server_nm.val())){
					$vpn_server_nm.show_validate_hint("Netmask range must be 255.255.0.0 (/16) ~ 255.255.255.248 (/29)");
					$vpn_server_nm.focus();
					return false;
				}

				//check subnet/netmask combination whether it invalid or not
				if(!validator.subnetAndMaskCombination($vpn_server_sn.val(), $vpn_server_nm.val())){
					$vpn_server_sn.show_validate_hint($vpn_server_sn.val() + " / " + $vpn_server_nm.val() + " combination is invalid");
					$vpn_server_sn.focus();
					return false;
				}
			}
			else if(vpn_server_if == 'tap' && $vpn_server_dhcp.hasClass("off")){
				var $vpn_server_r1 = $(_obj).find("#vpn_server_r1");
				$vpn_server_r1.val($vpn_server_r1.val().replace(/\s+/g, ''));//remove space
				var $vpn_server_r2 = $(_obj).find("#vpn_server_r2");
				$vpn_server_r2.val($vpn_server_r2.val().replace(/\s+/g, ''));//remove space

				var isValid_vpn_server_r1 = valid_isLegalIP($vpn_server_r1.val());
				if(isValid_vpn_server_r1.isError){
					$vpn_server_r1.show_validate_hint(isValid_vpn_server_r1.errReason);
					$vpn_server_r1.focus();
					return false;
				}
				if($vpn_server_r1.val().split(".")[3] == 255){	//*.*.*.255 can't be IP in the IP pool
					$vpn_server_r1.show_validate_hint($vpn_server_r1.val() + " <#JS_validip#>");
					$vpn_server_r1.focus();
					return false;
				}

				var isValid_vpn_server_r2 = valid_isLegalIP($vpn_server_r2.val());
				if(isValid_vpn_server_r2.isError){
					$vpn_server_r2.show_validate_hint(isValid_vpn_server_r2.errReason);
					$vpn_server_r2.focus();
					return false;
				}
				if($vpn_server_r2.val().split(".")[3] == 255){	//*.*.*.255 can't be IP in the IP pool
					$vpn_server_r2.show_validate_hint($vpn_server_r2.val() + " <#JS_validip#>");
					$vpn_server_r2.focus();
					return false;
				}

				var openvpn_clients_start_array = $vpn_server_r1.val().split(".");
				var openvpn_clients_start_subnet = openvpn_clients_start_array[0] + "." + openvpn_clients_start_array[1] + "." + openvpn_clients_start_array[2] + ".";
				var openvpn_clients_end_array = $vpn_server_r2.val().split(".");
				var openvpn_clients_end_subnet = openvpn_clients_end_array[0] + "." + openvpn_clients_end_array[1] + "." + openvpn_clients_end_array[2] + ".";
				var openvpn_clients_start_ip = parseInt(openvpn_clients_start_array[3]);
				var openvpn_clients_end_ip = parseInt(openvpn_clients_end_array[3]);

				//LAN ip
				if( (lan_ip_subnet == openvpn_clients_start_subnet || lan_ip_subnet == openvpn_clients_end_subnet)
				 && (lan_ip_end >= openvpn_clients_start_ip && lan_ip_end <= openvpn_clients_end_ip)){
					$vpn_server_r1.show_validate_hint("<#vpn_conflict_LANIP#> "+origin_lan_ip);
					$vpn_server_r1.focus();
					return false;
				}
				if(openvpn_clients_end_ip < openvpn_clients_start_ip){
					$vpn_server_r2.show_validate_hint($vpn_server_r2.val() + " <#JS_validip#>");
					$vpn_server_r2.focus();
					return false;
				}

				//DHCP pool
				if(pool_subnet != openvpn_clients_start_subnet){
					$vpn_server_r1.show_validate_hint($vpn_server_r1.val() + " <#JS_validip#>");
					$vpn_server_r1.focus();
					return false;
				}
				if(pool_subnet != openvpn_clients_end_subnet){
					$vpn_server_r2.show_validate_hint($vpn_server_r2.val() + " <#JS_validip#>");
					$vpn_server_r2.focus();
					return false;
				}

				//DHCP static IP
				if(dhcp_staticlists != ""){
					for(var i = 1; i < staticclist_row.length; i +=1){
						var static_ip = staticclist_row[i].split('>')[1];
						var static_subnet = static_ip.split(".")[0]+"."+static_ip.split(".")[1]+"."+static_ip.split(".")[2]+".";
						var static_end = parseInt(static_ip.split(".")[3]);
						if(static_subnet != openvpn_clients_start_subnet){
							$vpn_server_r2.show_validate_hint($vpn_server_r2.val() + " <#JS_validip#>");
							$vpn_server_r2.focus();
							return false;
						}
						if(static_subnet != openvpn_clients_end_subnet){
							$vpn_server_r2.show_validate_hint($vpn_server_r2.val() + " <#JS_validip#>");
							$vpn_server_r2.focus();
							return false;
						}
					}
				}
			}
			return true;
		};

		if(!check_openvpn_conflict())
			return false;

		if(ipv6_item_flag){
			if(vpn_server_if != "tap"){
				var vpn_server_ip6 = (($(_obj).find("#vpn_server_ip6")).hasClass("on") ? "1" : "0");
				if(vpn_server_ip6 == "1"){
					var vpn_server_crypt = $(_obj).find("#select_vpn_server_crypt").children(".selected").attr("value");
					if(vpn_server_if == "tun" && vpn_server_crypt == "tls"){
						var $vpn_server_sn6 = $(_obj).find("#vpn_server_sn6");
						$vpn_server_sn6.val($vpn_server_sn6.val().replace(/\s+/g, ''));//remove space
						isValid_vpn_server_sn6 = valid_IP_CIDR($vpn_server_sn6.val(), "IPv6", "1");
						if(isValid_vpn_server_sn6.isError){
							$vpn_server_sn6.show_validate_hint(isValid_vpn_server_sn6.errReason);
							$vpn_server_sn6.focus();
							return false;
						}
					}
					else if(vpn_server_if == "tun" && vpn_server_crypt == "secret"){
						var $vpn_server_local6 = $(_obj).find("#vpn_server_local6");
						$vpn_server_local6.val($vpn_server_local6.val().replace(/\s+/g, ''));//remove space
						isValid_vpn_server_local6 = valid_IP_CIDR($vpn_server_local6.val(), "IPv6", "0");
						if(isValid_vpn_server_local6.isError){
							$vpn_server_local6.show_validate_hint(isValid_vpn_server_local6.errReason);
							$vpn_server_local6.focus();
							return false;
						}
						var $vpn_server_remote6 = $(_obj).find("#vpn_server_remote6");
						$vpn_server_remote6.val($vpn_server_remote6.val().replace(/\s+/g, ''));//remove space
						isValid_vpn_server_remote6 = valid_IP_CIDR($vpn_server_remote6.val(), "IPv6", "0");
						if(isValid_vpn_server_remote6.isError){
							$vpn_server_remote6.show_validate_hint(isValid_vpn_server_remote6.errReason);
							$vpn_server_remote6.focus();
							return false;
						}
					}
				}
			}
		}
	}
	else if(_validField == "openvpn_clientlist"){
		var $clientlist_username = $(_obj).find("#clientlist_username");
		$clientlist_username.val($clientlist_username.val().replace(/\s+/g, ''));//remove space
		if($clientlist_username.val() == ""){
			$clientlist_username.show_validate_hint("<#JS_fieldblank#>");
			$clientlist_username.focus();
			return false;
		}
		var isValid_username = valid_block_chars($clientlist_username.val(), [" ", "@", "*", "+", "|", ":", "?", "<", ">", ",", ".", "/", ";", "[", "]", "\\", "=", "\"", "&", "#"]);
		if(isValid_username.isError){
			$clientlist_username.show_validate_hint(isValid_username.errReason);
			$clientlist_username.focus();
			return false;
		}

		var duplicateCheck = openvpn_clientlist_data.filter(function(item, index, array){
			return (item.username == $clientlist_username.val());
		})[0];
		if(duplicateCheck != undefined){
			show_customize_alert("<#JS_duplicate#>");
			return false;
		}

		var $clientlist_password = $(_obj).find("#clientlist_password");
		$clientlist_password.val($clientlist_password.val().replace(/\s+/g, ''));//remove space
		if($clientlist_password.val() == ""){
			$clientlist_password.show_validate_hint("<#JS_fieldblank#>");
			$clientlist_password.focus();
			return false;
		}
		var isValid_password = valid_block_chars($clientlist_password.val(), ["<", ">", "&"]);
		if(isValid_password.isError){
			$clientlist_password.show_validate_hint(isValid_password.errReason);
			$clientlist_password.focus();
			return false;
		}
	}
	else if(_validField == "openvpn_allowed_clientlist"){
		var $commonname = $(_obj).find("#vpn_clientlist_commonname_0");
		$commonname.val($commonname.val().replace(/\s+/g, ''));//remove space
		if($commonname.val() == ""){
			$commonname.show_validate_hint("<#JS_fieldblank#>");
			$commonname.focus();
			return false;
		}
		var isValid_commonname = valid_block_chars($commonname.val(), ["<", ">"]);
		if(isValid_commonname.isError){
			$commonname.show_validate_hint(isValid_commonname.errReason);
			$commonname.focus();
			return false;
		}

		var $subnet = $(_obj).find("#vpn_clientlist_subnet_0");
		$subnet.val($subnet.val().replace(/\s+/g, ''));//remove space
		if($subnet.val() == ""){
			$subnet.show_validate_hint("<#JS_fieldblank#>");
			$subnet.focus();
			return false;
		}
		var isValid_subnet = valid_is_IP_format($subnet.val(), "IPv4");
		if(isValid_subnet.isError){
			$subnet.show_validate_hint(isValid_subnet.errReason);
			$subnet.focus();
			return false;
		}

		var $netmask = $(_obj).find("#vpn_clientlist_netmask_0");
		$netmask.val($netmask.val().replace(/\s+/g, ''));//remove space
		if($netmask.val() == ""){
			$netmask.show_validate_hint("<#JS_fieldblank#>");
			$netmask.focus();
			return false;
		}
		var isValid_netmask = valid_isLegalMask($netmask.val());
		if(isValid_netmask.isError){
			$netmask.show_validate_hint(isValid_netmask.errReason);
			$netmask.focus();
			return false;
		}

		var duplicateCheck = openvpn_allowed_clientlist_data.filter(function(item, index, array){
			return (item.username == $commonname.val() && item.subnet == $subnet.val() && item.netmask == $netmask.val());
		})[0];
		if(duplicateCheck != undefined){
			show_customize_alert("<#JS_duplicate#>");
			return false;
		}
	}
	return true;
}
function Update_Profile_Data_OpenVPN(_obj){
	var settings = httpApi.nvramGet(["vpn_server_unit", "vpn_serverx_dns", "vpn_server_port", "vpn_server_tls_keysize", "vpn_server_if", "vpn_server_proto", "vpn_server_pdns",
		"vpn_server_cipher", "vpn_server_digest", "vpn_server_comp", "vpn_server_igncrt", "vpn_server_crypt", "vpn_server_hmac", "vpn_server_sn", "vpn_server_nm",
		"vpn_server_dhcp", "vpn_server_r1", "vpn_server_r2", "vpn_server_local", "vpn_server_remote", "vpn_server_plan", "vpn_server_rgw", "vpn_server_reneg",
		"vpn_server_ccd", "vpn_server_c2c", "vpn_server_ccd_excl", "vpn_server_ccd_val", 
		"vpn_serverx_clientlist"]);

	$(_obj).find("#vpn_server_port").val(htmlEnDeCode.htmlEncode(settings.vpn_server_port));
	$(_obj).find("#vpn_server_tls_keysize").find(".radio_container").removeClass("selected").filter("[value='" + settings.vpn_server_tls_keysize + "']").addClass("selected");

	set_value_Custom_Select(_obj, "vpn_server_if", settings.vpn_server_if);
	set_value_Custom_Select(_obj, "vpn_server_proto", settings.vpn_server_proto);

	$(_obj).find("#vpn_server_x_dns").removeClass("off on").addClass((function(){
		return ((settings.vpn_serverx_dns.indexOf(''+(settings.vpn_server_unit)) >= 0) ? "on" : "off");
	})());

	$(_obj).find("#vpn_server_pdns").removeClass("off on").addClass((function(){
		return ((settings.vpn_server_pdns == "1") ? "on" : "off");
	})());

	set_value_Custom_Select(_obj, "vpn_server_cipher", settings.vpn_server_cipher);
	set_value_Custom_Select(_obj, "vpn_server_digest", settings.vpn_server_digest);
	set_value_Custom_Select(_obj, "vpn_server_comp", settings.vpn_server_comp);

	$(_obj).find("#vpn_server_igncrt").removeClass("off on").addClass((function(){
		return ((settings.vpn_server_igncrt == "1") ? "on" : "off");
	})());

	set_value_Custom_Select(_obj, "vpn_server_crypt", settings.vpn_server_crypt);
	set_value_Custom_Select(_obj, "vpn_server_hmac", settings.vpn_server_hmac);

	$(_obj).find("#vpn_server_sn").val(htmlEnDeCode.htmlEncode(settings.vpn_server_sn));
	$(_obj).find("#vpn_server_nm").val(htmlEnDeCode.htmlEncode(settings.vpn_server_nm));

	$(_obj).find("#vpn_server_dhcp").removeClass("off on").addClass((function(){
		return ((settings.vpn_server_dhcp == "1") ? "on" : "off");
	})());

	$(_obj).find("#vpn_server_r1").val(htmlEnDeCode.htmlEncode(settings.vpn_server_r1));
	$(_obj).find("#vpn_server_r2").val(htmlEnDeCode.htmlEncode(settings.vpn_server_r2));

	$(_obj).find("#vpn_server_local").val(htmlEnDeCode.htmlEncode(settings.vpn_server_local));
	$(_obj).find("#vpn_server_remote").val(htmlEnDeCode.htmlEncode(settings.vpn_server_remote));

	$(_obj).find("#vpn_server_plan").removeClass("off on").addClass((function(){
		return ((settings.vpn_server_plan == "1") ? "on" : "off");
	})());

	$(_obj).find("#vpn_server_rgw").removeClass("off on").addClass((function(){
		return ((settings.vpn_server_rgw == "1") ? "on" : "off");
	})());

	$(_obj).find("#vpn_server_reneg").val(htmlEnDeCode.htmlEncode(settings.vpn_server_reneg));

	$(_obj).find("#vpn_server_ccd").removeClass("off on").addClass((function(){
		return ((settings.vpn_server_ccd == "1") ? "on" : "off");
	})());

	$(_obj).find("#vpn_server_c2c").removeClass("off on").addClass((function(){
		return ((settings.vpn_server_c2c == "1") ? "on" : "off");
	})());

	$(_obj).find("#vpn_server_ccd_excl").removeClass("off on").addClass((function(){
		return ((settings.vpn_server_ccd_excl == "1") ? "on" : "off");
	})());

	var vpn_server_custom = httpApi.nvramGet(["vpn_server_custom"]).vpn_server_custom
		.replace(/&#10/g, "\n").replace(/&#13/g, "\r").replace(/&#34/g, "\"").replace(/&#38/g, "&").replace(/&#60/g, "<").replace(/&#62/g, ">");
	$(_obj).find("#vpn_server_custom").val(vpn_server_custom);

	var vpn_server_clientlist = (settings.vpn_serverx_clientlist).replace(/&#62/g, ">").replace(/&#60/g, "<");
	var vpn_server_clientlist_array = vpn_server_clientlist.split("<");
	$.each(vpn_server_clientlist_array, function(index, value){
		if(value != ""){
			var client_data = value.split(">");
			var client_profile = new openvpn_clientlist_attr();
			client_profile.username = client_data[0];
			client_profile.password = client_data[1];
			openvpn_clientlist_data.push(JSON.parse(JSON.stringify(client_profile)));
		}
	});
	show_openvpn_clientlist($(_obj).find("#openvpn_clientlist_bg"));

	var vpn_server_ccd_val = (settings.vpn_server_ccd_val).replace(/&#62/g, ">").replace(/&#60/g, "<");
	var vpn_server_ccd_val_array = vpn_server_ccd_val.split("<");
	$.each(vpn_server_ccd_val_array, function(index, value){
		if(value != ""){
			var client_data = value.split(">");
			var client_profile = new openvpn_allowed_clientlist_attr();
			client_profile.username = client_data[1];
			client_profile.subnet = client_data[2];
			client_profile.netmask = client_data[3];
			client_profile.push = client_data[4];
			openvpn_allowed_clientlist_data.push(JSON.parse(JSON.stringify(client_profile)));
		}
	});
	show_openvpn_allowed_clientlist($(_obj).find("#openvpn_allowed_clientlist_bg"));

	if(ipv6_item_flag){
		var ipv6_settings = httpApi.nvramGet(["vpn_server_ip6", "vpn_server_nat6", "vpn_server_sn6", "vpn_server_local6", "vpn_server_remote6"]);
		$(_obj).find("#vpn_server_ip6").removeClass("off on").addClass((function(){
			return ((ipv6_settings.vpn_server_ip6 == "1") ? "on" : "off");
		})());
		$(_obj).find("#vpn_server_nat6").removeClass("off on").addClass((function(){
			if(isSupport("ipv6nat"))
				return ((ipv6_settings.vpn_server_nat6 == "1") ? "on" : "off");
			else
				return "off";
		})());
		$(_obj).find("#vpn_server_sn6").val(htmlEnDeCode.htmlEncode(ipv6_settings.vpn_server_sn6));
		$(_obj).find("#vpn_server_local6").val(htmlEnDeCode.htmlEncode(ipv6_settings.vpn_server_local6));
		$(_obj).find("#vpn_server_remote6").val(htmlEnDeCode.htmlEncode(ipv6_settings.vpn_server_remote6));
	}

	enable_server_igncrt(_obj, settings.vpn_server_igncrt);
	update_visibility(_obj);
}
function update_visibility(_obj){
	var showhide = function(element, sh){
		$(_obj).find("#" + element + "").hide();
		if(sh == "1"){
			$(_obj).find("#" + element + "").show();
		}
		else{
			var ori_settings = httpApi.nvramGet(["vpn_server_sn", "vpn_server_nm", "vpn_server_local", "vpn_server_remote", "vpn_server_reneg", "vpn_server_r1", "vpn_server_r2"]);
			if(element == "server_snnm"){
				$(_obj).find("#vpn_server_sn").val(htmlEnDeCode.htmlEncode(ori_settings.vpn_server_sn));
				$(_obj).find("#vpn_server_nm").val(htmlEnDeCode.htmlEncode(ori_settings.vpn_server_nm));
			}
			else if(element == "server_local"){
				$(_obj).find("#vpn_server_local").val(htmlEnDeCode.htmlEncode(ori_settings.vpn_server_local));
				$(_obj).find("#vpn_server_remote").val(htmlEnDeCode.htmlEncode(ori_settings.vpn_server_remote));
			}
			else if(element == "server_reneg"){
				$(_obj).find("#vpn_server_reneg").val(htmlEnDeCode.htmlEncode(ori_settings.vpn_server_reneg));
			}
			else if(element == "server_range"){
				$(_obj).find("#vpn_server_r1").val(htmlEnDeCode.htmlEncode(ori_settings.vpn_server_r1));
				$(_obj).find("#vpn_server_r2").val(htmlEnDeCode.htmlEncode(ori_settings.vpn_server_r2));
			}

			if(ipv6_item_flag){
				var ori_ipv6_settings = httpApi.nvramGet(["vpn_server_ip6", "vpn_server_nat6", "vpn_server_sn6", "vpn_server_local6", "vpn_server_remote6"]);
				if(element == "server_ip6"){
					$(_obj).find("#vpn_server_ip6").removeClass("off on").addClass((function(){
							return ((ori_ipv6_settings.vpn_server_ip6 == "1") ? "on" : "off");
					})());
				}
				else if(element == "server_nat6"){
					$(_obj).find("#vpn_server_nat6").removeClass("off on").addClass((function(){
						if(isSupport("ipv6nat"))
							return ((ori_ipv6_settings.vpn_server_nat6 == "1") ? "on" : "off");
						else
							return "off";
					})());
				}
				else if(element == "server_sn6"){
					$(_obj).find("#vpn_server_sn6").val(htmlEnDeCode.htmlEncode(ori_ipv6_settings.vpn_server_sn6));
				}
				else if(element == "server_local6"){
					$(_obj).find("#vpn_server_local6").val(htmlEnDeCode.htmlEncode(ori_ipv6_settings.vpn_server_local6));
					$(_obj).find("#vpn_server_remote6").val(htmlEnDeCode.htmlEncode(ori_ipv6_settings.vpn_server_remote6));
				}
			}
		}
	};
	var auth = $(_obj).find("#select_vpn_server_crypt").children(".selected").attr("value");
	var iface = $(_obj).find("#select_vpn_server_if").children(".selected").attr("value");
	var hmac = $(_obj).find("#select_vpn_server_hmac").children(".selected").attr("value");
	var dhcp = (($(_obj).find("#vpn_server_dhcp")).hasClass("on") ? "1" : "0");
	var dns = (($(_obj).find("#vpn_server_x_dns")).hasClass("on") ? "1" : "0");
	var ccd = "0";
	if(auth != "tls")
		ccd = "0";
	else
		ccd = (($(_obj).find("#vpn_server_ccd")).hasClass("on") ? "1" : "0");

	showhide("server_authhmac", (auth != "secret"));
	showhide("server_snnm", ((auth == "tls") && (iface == "tun")));
	showhide("server_plan", ((auth == "tls") && (iface == "tun")));
	showhide("server_rgw", (auth == "tls"));
	showhide("server_local", ((auth == "secret") && (iface == "tun")));
	showhide("server_reneg", (auth != "secret"));
	showhide("server_ccd", (auth == "tls"));
	showhide("server_c2c", ccd);
	showhide("server_ccd_excl", ccd);
	showhide("openvpn_allowed_clientlist_title", ccd);
	showhide("openvpn_allowed_clientlist_bg", ccd);
	showhide("server_pdns", ((auth == "tls") && (dns == 1)));
	showhide("server_dhcp",((auth == "tls") && (iface == "tap")));
	showhide("server_range", ((dhcp == 0) && (auth == "tls") && (iface == "tap")));
	if(auth == "secret"){
		$(_obj).find("#openvpn_clientlist_title").hide();
		$(_obj).find("#openvpn_clientlist_bg").hide();
	}
	else{
		$(_obj).find("#openvpn_clientlist_title").show();
		$(_obj).find("#openvpn_clientlist_bg").show();
	}

	updateVpnServerClientAccess(_obj);
	if(ipv6_item_flag){
		if(iface != "tap"){
			showhide("server_ip6", "1");
			var vpn_server_ip6 = (($(_obj).find("#vpn_server_ip6")).hasClass("on") ? "1" : "0");
			showhide("server_nat6", vpn_server_ip6);
			if(vpn_server_ip6 == "1"){
				if(!isSupport("ipv6nat"))
					$(_obj).find("#ipv6nat_hint").show();
				showhide("server_sn6", ((auth == "tls") && (iface == "tun")));
				showhide("server_local6", ((auth == "secret") && (iface == "tun")));
			}
			else{
				if(!isSupport("ipv6nat"))
					$(_obj).find("#ipv6nat_hint").hide();
				showhide("server_sn6", "0");
				showhide("server_local6", "0");
			}
		}
		else{
			if(!isSupport("ipv6nat"))
				$(_obj).find("#ipv6nat_hint").hide();
			showhide("server_ip6", "0");
			showhide("server_nat6", "0");
			showhide("server_sn6", "0");
			showhide("server_local6", "0");
		}
	}
	set_apply_btn_status_OpenVPN(_obj);
	resize_iframe_height();
}
function enable_server_igncrt(_obj, _value){
	if(_value == "1"){
		set_value_Custom_Select(_obj, "vpn_server_crypt", "tls");
		$(_obj).find("#vpn_server_crypt").closest(".custom_select_container").attr("temp_disable", "disabled");
		$(_obj).find("#Hint_fixed_tls_crypto").show();
	}
	else{
		set_value_Custom_Select(_obj, "vpn_server_crypt", httpApi.nvramGet(["vpn_server_crypt"]).vpn_server_crypt);
		$(_obj).find("#vpn_server_crypt").closest(".custom_select_container").removeAttr("temp_disable");
		$(_obj).find("#Hint_fixed_tls_crypto").hide();
	}
}
function vpnServerClientAccess(_obj){
	var vpn_server_client_access = $(_obj).find("#vpn_server_client_access .radio_container.selected").attr("value");
	switch(parseInt(vpn_server_client_access)) {
		case 0 :
			$(_obj).find("#vpn_server_plan").removeClass("on off").addClass("on");
			$(_obj).find("#vpn_server_rgw").removeClass("on off").addClass("off");
			$(_obj).find("#vpn_server_x_dns").removeClass("on off").addClass("off");
			$(_obj).find("#vpn_server_pdns").removeClass("on off").addClass("off");
			$(_obj).find("#vpn_server_client_access .radio_container[value='2']").hide();
			break;
		case 1 :
			$(_obj).find("#vpn_server_plan").removeClass("on off").addClass("on");
			$(_obj).find("#vpn_server_rgw").removeClass("on off").addClass("on");
			$(_obj).find("#vpn_server_x_dns").removeClass("on off").addClass("on");
			$(_obj).find("#vpn_server_pdns").removeClass("on off").addClass("on");
			$(_obj).find("#vpn_server_client_access .radio_container[value='2']").hide();
			break;
	}
}
function updateVpnServerClientAccess(_obj) {
	var vpn_server_plan = ($(_obj).find("#vpn_server_plan").hasClass("on") ? "1" : "0");
	var vpn_server_rgw = ($(_obj).find("#vpn_server_rgw").hasClass("on") ? "1" : "0");
	var vpn_server_x_dns = ($(_obj).find("#vpn_server_x_dns").hasClass("on") ? "1" : "0");
	var vpn_server_pdns = ($(_obj).find("#vpn_server_pdns").hasClass("on") ? "1" : "0");

	if(vpn_server_plan == "1" && vpn_server_rgw == "0" && vpn_server_x_dns == "0" && vpn_server_pdns == "0") {
		$(_obj).find("#vpn_server_client_access .radio_container").removeClass("selected").filter("[value='0']").addClass("selected");
		$(_obj).find("#vpn_server_client_access .radio_container").filter("[value='2']").hide();
	}
	else if(vpn_server_plan == "1" && vpn_server_rgw == "1" && vpn_server_x_dns == "1" && vpn_server_pdns == "1") {
		$(_obj).find("#vpn_server_client_access .radio_container").removeClass("selected").filter("[value='1']").addClass("selected");
		$(_obj).find("#vpn_server_client_access .radio_container").filter("[value='2']").hide();
	}
	else {
		$(_obj).find("#vpn_server_client_access .radio_container").removeClass("selected").filter("[value='2']").addClass("selected");
		$(_obj).find("#vpn_server_client_access .radio_container").filter("[value='2']").show();
	}
}
function set_apply_btn_status_OpenVPN(_obj){
	var $btn_container_apply = $(_obj).find(".action_container .btn_container.apply");
	var validate_blank_flag = validate_blank_OpenVPN($(_obj), "openvpn_profile");
	if(!validate_blank_flag)
		$btn_container_apply.removeClass("valid_fail").addClass("valid_fail").unbind("click");
	else{
		$btn_container_apply.removeClass("valid_fail").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			if(validate_format_OpenVPN($(_obj), "openvpn_profile")){
				var nvramSet_obj = {"action_mode": "apply", "rc_service": ""};
				var settings = httpApi.nvramGet(["VPNServer_enable", "enable_samba", "vpn_server_unit", "vpn_serverx_eas", "vpn_serverx_dns"]);
				if(settings.VPNServer_enable == "1")
					nvramSet_obj.rc_service = "restart_openvpnd;restart_chpass;";
				else
					nvramSet_obj.rc_service = "stop_openvpnd;";
				if(settings.enable_samba == "1")
					nvramSet_obj.rc_service += "restart_samba;";

				nvramSet_obj.vpn_server_unit = settings.vpn_server_unit;
				var openvpn_allowed_clientlist = "";
				$.each(openvpn_allowed_clientlist_data, function(index, value){
					openvpn_allowed_clientlist += "<1>" + value.username + ">" + value.subnet + ">" + value.netmask + ">" + value.push;
				});
				nvramSet_obj.vpn_server_ccd_val = openvpn_allowed_clientlist;

				var vpn_serverx_eas_temp = "";
				for (var i = 1; i < 3; i += 1) {
					if (i == settings.vpn_server_unit) {
						var vpn_server_x_eas = (settings.vpn_serverx_eas.indexOf(''+(settings.vpn_server_unit)) >= 0) ? "1" : "0";
						if(vpn_server_x_eas == 1)
							vpn_serverx_eas_temp += ""+i+",";
					} 
					else{
						if(settings.vpn_serverx_eas.indexOf(''+(i)) >= 0)
							vpn_serverx_eas_temp += ""+i+","
					}
				}
				nvramSet_obj.vpn_serverx_eas = vpn_serverx_eas_temp;

				var vpn_serverx_dns_temp = "";
				for (var i = 1; i < 3; i += 1) {
					if (i == settings.vpn_server_unit) {
						if ($(_obj).find("#vpn_server_x_dns").hasClass("on"))
							vpn_serverx_dns_temp += ""+i+",";
					} else {
						if (settings.vpn_serverx_dns.indexOf(''+(i)) >= 0)
							vpn_serverx_dns_temp += ""+i+","
					}
				}
				if (vpn_serverx_dns_temp != settings.vpn_serverx_dns) {
					nvramSet_obj.rc_service += "restart_dnsmasq;";
				}
				nvramSet_obj.vpn_serverx_dns = vpn_serverx_dns_temp;

				var openvpn_clientlist = "";
				$.each(openvpn_clientlist_data, function(index, value){
					if(value.username != httpApi.nvramGet(["http_username"]).http_username)
						openvpn_clientlist += "<" + value.username + ">" + value.password;
				});
				nvramSet_obj.vpn_serverx_clientlist = openvpn_clientlist;
				nvramSet_obj.vpn_server_if = $(_obj).find("#select_vpn_server_if").children(".selected").attr("value");
				nvramSet_obj.vpn_server_proto = $(_obj).find("#select_vpn_server_proto").children(".selected").attr("value");
				nvramSet_obj.vpn_server_port = $(_obj).find("#vpn_server_port").val();
				nvramSet_obj.vpn_server_crypt = $(_obj).find("#select_vpn_server_crypt").children(".selected").attr("value");
				nvramSet_obj.vpn_server_comp = $(_obj).find("#select_vpn_server_comp").children(".selected").attr("value");
				nvramSet_obj.vpn_server_cipher = $(_obj).find("#select_vpn_server_cipher").children(".selected").attr("value");
				nvramSet_obj.vpn_server_digest = $(_obj).find("#select_vpn_server_digest").children(".selected").attr("value");
				nvramSet_obj.vpn_server_dhcp = ($(_obj).find("#vpn_server_dhcp").hasClass("on") ? "1" : "0");
				nvramSet_obj.vpn_server_r1 = $(_obj).find("#vpn_server_r1").val();
				nvramSet_obj.vpn_server_r2 = $(_obj).find("#vpn_server_r2").val();
				nvramSet_obj.vpn_server_sn = $(_obj).find("#vpn_server_sn").val();
				nvramSet_obj.vpn_server_nm = $(_obj).find("#vpn_server_nm").val();
				nvramSet_obj.vpn_server_local = $(_obj).find("#vpn_server_local").val();
				nvramSet_obj.vpn_server_remote = $(_obj).find("#vpn_server_remote").val();
				nvramSet_obj.vpn_server_reneg = $(_obj).find("#vpn_server_reneg").val();
				nvramSet_obj.vpn_server_hmac = $(_obj).find("#select_vpn_server_hmac").children(".selected").attr("value");
				nvramSet_obj.vpn_server_plan = ($(_obj).find("#vpn_server_plan").hasClass("on") ? "1" : "0");
				nvramSet_obj.vpn_server_ccd = ($(_obj).find("#vpn_server_ccd").hasClass("on") ? "1" : "0");
				nvramSet_obj.vpn_server_c2c = ($(_obj).find("#vpn_server_c2c").hasClass("on") ? "1" : "0");
				nvramSet_obj.vpn_server_ccd_excl = ($(_obj).find("#vpn_server_ccd_excl").hasClass("on") ? "1" : "0");
				nvramSet_obj.vpn_server_pdns = ($(_obj).find("#vpn_server_pdns").hasClass("on") ? "1" : "0");
				nvramSet_obj.vpn_server_rgw = ($(_obj).find("#vpn_server_rgw").hasClass("on") ? "1" : "0");
				nvramSet_obj.vpn_server_custom = $(_obj).find("#vpn_server_custom").val();
				nvramSet_obj.vpn_server_igncrt = ($(_obj).find("#vpn_server_igncrt").hasClass("on") ? "1" : "0");
				nvramSet_obj.vpn_server_tls_keysize = $(_obj).find("#vpn_server_tls_keysize").find(".radio_container.selected").attr("value");
				if(ipv6_item_flag){
					nvramSet_obj.vpn_server_ip6 = ($(_obj).find("#vpn_server_ip6").hasClass("on") ? "1" : "0");
					nvramSet_obj.vpn_server_nat6 = ($(_obj).find("#vpn_server_nat6").hasClass("on") ? "1" : "0");
					nvramSet_obj.vpn_server_sn6 = $(_obj).find("#vpn_server_sn6").val();
					nvramSet_obj.vpn_server_local6 = $(_obj).find("#vpn_server_local6").val();
					nvramSet_obj.vpn_server_remote6 = $(_obj).find("#vpn_server_remote6").val();
				}

				httpApi.nvramSet(nvramSet_obj, function(){
					var time = 10;
					close_popup();
					showLoading(time);
					setTimeout(function(){
						httpApi.hookGet("vpn_server_get_parameter", true);
						httpApi.nvramGet(["vpn_server_unit", "vpn_serverx_eas", "vpn_serverx_dns", "vpn_server_port", "vpn_server_tls_keysize", "vpn_server_if", "vpn_server_proto",
							"vpn_server_pdns", "vpn_server_cipher", "vpn_server_digest", "vpn_server_comp", "vpn_server_igncrt", "vpn_server_crypt", "vpn_server_hmac", "vpn_server_sn",
							"vpn_server_nm", "vpn_server_dhcp", "vpn_server_r1", "vpn_server_r2", "vpn_server_local", "vpn_server_remote", "vpn_server_plan", "vpn_server_rgw",
							"vpn_server_reneg", "vpn_server_ccd", "vpn_server_c2c", "vpn_server_ccd_excl", "vpn_server_ccd_val", "vpn_serverx_clientlist", "vpn_server_custom"], true);
						if(ipv6_item_flag){
							httpApi.nvramGet(["vpn_server_ip6", "vpn_server_nat6", "vpn_server_sn6", "vpn_server_local6", "vpn_server_remote6"], true);
						}
						if(!window.matchMedia('(max-width: 575px)').matches)
							$("#srv_profile_list").children("[type='" + select_vpn_type + "']").addClass("selected").find(".svr_item_text_container").click();
					}, time*1000);
				}());
			}
		});
	}
}
function Get_Component_Setting_Profile_OpenVPN(_type){
	var openvpn_settings = httpApi.nvramGet(["VPNServer_enable"]);

	var $container = $("<div>").addClass("popup_edit_profile_container");

	if(_type == "popup")
		$container.append(Get_Component_Popup_Profile_Title("OpenVPN"));
	else
		$container.append(Get_Component_Profile_Title("OpenVPN"));

	var $content_container = $("<div>").addClass("profile_setting");
	if(_type == "popup"){
		$content_container.addClass("popup_content_container");
	}
	else{
		$content_container.addClass("no_popup_content_container");
	}
	$content_container.appendTo($container);

	var detail_options = [{"text":"<#menu5_1_1#>","value":"1"},{"text":"<#menu5#>","value":"2"}];
	var detail_options_parm = {"title": "<#vpn_Adv#>", "id": "detail_options", "options": detail_options, "set_value": "1"};
	Get_Component_Custom_Select(detail_options_parm).appendTo($content_container)
		.find("#select_" + detail_options_parm.id + "").children("div").click(function(e){
			var options = $(this).attr("value");
			$(this).closest(".profile_setting").find("[detail_mode]").hide();
			$(this).closest(".profile_setting").find("[detail_mode='" + options + "']").show();
			resize_iframe_height();
		});

	var $detail_general = $("<div>").attr("detail_mode","1").appendTo($content_container);

	var help_parm = {"title":"<#HOWTOSETUP#>"};
	Get_Component_Help(help_parm)
		.appendTo($detail_general)
		.find(".vpnc_help_icon").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			show_popup_help_OpenVPN("Feature_Desc");
		});

	var vpn_server_port_parm = {"title":"<#WLANAuthentication11a_ExAuthDBPortNumber_itemname#>", "type":"text", "id":"vpn_server_port", "need_check":true, "maxlength":5};
	Get_Component_Input(vpn_server_port_parm).appendTo($detail_general)
		.find("#" + vpn_server_port_parm.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isNumber(this,event);
		});
	var $vpn_server_port_hint_obj = $("<div>").addClass("item_hint").html("* <#SSH_Port_Suggestion#>").appendTo($detail_general);
	if(wan_proto == "v6plus" || wan_proto == "ocnvc"){
		var get_s46_hgw_case = '<% nvram_get("s46_hgw_case"); %>';      //topology 2,3,6
		var s46_ports_check_flag = (get_s46_hgw_case=='3' || get_s46_hgw_case=='6')? true:false;        //true for topology 3||6
		var get_ipv6_s46_ports = (Softwire46_support && (wan_proto == "v6plus" || wan_proto == "ocnvc")) ? httpApi.nvramGet(["ipv6_s46_ports"]).ipv6_s46_ports : '0';
		var array_ipv6_s46_ports = new Array("");
		if(get_ipv6_s46_ports!="0" && get_ipv6_s46_ports!=""){
			array_ipv6_s46_ports = get_ipv6_s46_ports.split(" ");
		}
		if(s46_ports_check_flag && array_ipv6_s46_ports.length > 1){
			var v6plus_hint = port_confirm + "<br>" + get_ipv6_s46_ports;
			$vpn_server_port_hint_obj.html(v6plus_hint);
		}
	}

	var vpn_server_tls_keysize_options = [{"text":"1024 bit","value":"0"},{"text":"2048 bit","value":"1"}];
	var vpn_server_tls_keysize_parm = {"title": "<#RSA_Encryption#>", "id": "vpn_server_tls_keysize", "options": vpn_server_tls_keysize_options, "openHint":"32_8", "display_type": "horizontal"};
	var $vpn_server_tls_keysize = Get_Component_Radio(vpn_server_tls_keysize_parm).appendTo($detail_general).find("#" + vpn_server_tls_keysize_parm.id + " .radio_options_container");

	var vpn_server_client_access_options = [{"text":"<#vpn_access_LAN#>","value":"0"},{"text":"<#vpn_access_WANLAN#>","value":"1"},{"text":"<#Custom#>","value":"2"}];
	var vpn_server_client_access_parm = {"title": "<#vpn_access#>", "id": "vpn_server_client_access", "options": vpn_server_client_access_options, "display_type": "vertical"};
	Get_Component_Radio(vpn_server_client_access_parm).appendTo($detail_general)
		.find("#" + vpn_server_client_access_parm.id + " .radio_options_container").children().each(function(){
			$(this).click(function(e){
				e = e || event;
				e.stopPropagation();
				vpnServerClientAccess($content_container);
				update_visibility($content_container);
			});
			if($(this).attr("value") == "2")
				$(this).hide();
		});

	var $cert_btn_bg = $("<div>").attr("id", "cert_btn_bg").appendTo($detail_general);
	var export_ovpn_parm = {"title":"<#vpn_export_ovpnfile#>", "type":"export_ovpn"};
	Get_Component_Export_Ovpn(export_ovpn_parm).appendTo($cert_btn_bg);
	if(openvpn_settings.VPNServer_enable == "1"){
		check_vpn_server_state($content_container);
	}
	else{
		$cert_btn_bg.hide();
	}

	var renew_cert_parm = {"title":"<#vpn_ipsec_re_cert#>", "type":"renew_cert", "openHint":"33_2"};
	Get_Component_Renew_Cert(renew_cert_parm).appendTo($cert_btn_bg);

	var help_cert_parm = {"title":"Certification Tranfer"};/* untranslated */
	Get_Component_Help(help_cert_parm)
		.appendTo($cert_btn_bg)
		.find(".vpnc_help_icon").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			show_popup_help_OpenVPN("Cert");
		});

	var export_cert_parm = {"title":"<#vpn_export_cert#>", "type":"import_cert", "openHint":"32_27"};
	Get_Component_Export_Cert(export_cert_parm).appendTo($cert_btn_bg);

	var import_cert_parm = {"title":"<#vpn_import_cert#>", "type":"import_cert", "openHint":"32_28"};
	Get_Component_Import_Cert(import_cert_parm).appendTo($cert_btn_bg);

	openvpn_clientlist_data = [{username: httpApi.nvramGet(["http_username"]).http_username, password: '-'}];
	var $Add_Client_Title_obj = Get_Component_Add_Client_Title().appendTo($detail_general);
	$Add_Client_Title_obj.attr({"id":"openvpn_clientlist_title"})
	$Add_Client_Title_obj.find("#vpns_clientlist_title").html("<#vpnc_title#> (<#List_limit#>" + htmlEnDeCode.htmlEncode(openvpn_clientlist_limit) + ")");
	$Add_Client_Title_obj.find("#vpns_clientlist_num").html(htmlEnDeCode.htmlEncode(openvpn_clientlist_data.length));
	$Add_Client_Title_obj.find("#vpns_clientlist_add").unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		if(openvpn_clientlist_data.length >= openvpn_clientlist_limit){
			show_customize_alert("<#weekSche_MAX_Num#>".replace("#MAXNUM", openvpn_clientlist_limit));
			return false;
		}
		show_popup_Add_Client_OpenVPN();
	});
	var $openvpn_clientlist_bg = $("<div>").attr({"id":"openvpn_clientlist_bg"}).addClass("client_list_content_container").appendTo($detail_general);
	show_openvpn_clientlist($openvpn_clientlist_bg);

	var $detail_adv = $("<div>").attr("detail_mode","2").appendTo($content_container);
	var help_parm = {"title":"<#HOWTOSETUP#>"};
	Get_Component_Help(help_parm)
		.appendTo($detail_adv)
		.find(".vpnc_help_icon").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			show_popup_help_OpenVPN("Feature_Desc_Adv");
		});

	var detail_adv_switch_parm = {"options": [{"text":"<#menu5_1_6#>","value":"1"},{"text":"<#vpn_openvpn_CustomConf#>","value":"2"}]};
	Get_Component_Switch_Two_Btn(detail_adv_switch_parm).appendTo($detail_adv)
		.find(".btn_options_container .btn_container").unbind("click").click(function(e){
			var options = $(this).attr("value");
			$(this).closest(".btn_options_container").find(".btn_container").removeClass("selected");
			$(this).addClass("selected");
			$(this).closest(".profile_setting").find("[adv_mode]").hide();
			$(this).closest(".profile_setting").find("[adv_mode='" + options + "']").show();
		});

	var $detail_adv_standard = $("<div>").attr("adv_mode","1").appendTo($detail_adv).show();
	var $detail_adv_custom = $("<div>").attr("adv_mode","2").appendTo($detail_adv).hide();

	var vpn_server_if_options = [{"text":"TAP","value":"tap"},{"text":"TUN","value":"tun"}];
	var vpn_server_if_parm = {"title": "<#vpn_openvpn_interface#>", "id": "vpn_server_if", "options": vpn_server_if_options, "openHint":"32_4"};
	Get_Component_Custom_Select(vpn_server_if_parm).appendTo($detail_adv_standard).find("#select_" + vpn_server_if_parm.id + "").children("div").click(function(e){
		e = e || event;
		e.stopPropagation();
		update_visibility($content_container);
	});

	var vpn_server_proto_options = [{"text":"TCP","value":"tcp-server"},{"text":"UDP","value":"udp"}];
	var vpn_server_proto_parm = {"title": "<#IPConnection_VServerProto_itemname#>", "id": "vpn_server_proto", "options": vpn_server_proto_options, "openHint":"32_5"};
	Get_Component_Custom_Select(vpn_server_proto_parm).appendTo($detail_adv_standard);

	var vpn_server_x_dns_parm = {"title":"<#vpn_openvpn_ResponseDNS#>", "type":"switch", "id":"vpn_server_x_dns", "openHint":"32_15"};
	Get_Component_Switch(vpn_server_x_dns_parm).appendTo($detail_adv_standard)
		.find("#" + vpn_server_x_dns_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			update_visibility($content_container);
		});

	var vpn_server_pdns_parm = {"title":"<#vpn_openvpn_AdvDNS#>", "type":"switch", "id":"vpn_server_pdns", "openHint":"32_16", "container_id":"server_pdns"};
	Get_Component_Switch(vpn_server_pdns_parm).appendTo($detail_adv_standard);

	var vpn_server_cipher_options = [{"text":"Default (BF-CBC)","value":"default"},{"text":"None","value":"none"}];
	for(var i = 0; i < ciphersarray.length; i += 1){
		vpn_server_cipher_options.push({"text":ciphersarray[i][0],"value":ciphersarray[i][0]});
	}
	var vpn_server_cipher_parm = {"title": "<#vpn_openvpn_Encrypt#>", "id": "vpn_server_cipher", "options": vpn_server_cipher_options, "openHint":"32_17"};
	Get_Component_Custom_Select(vpn_server_cipher_parm).appendTo($detail_adv_standard);

	var vpn_server_digest_options = [];
	for(var i = 0; i < hmacarray.length; i += 1){
		if(hmacarray[i][1] == "MD5" || hmacarray[i][1] == "RSA-MD4")
			vpn_server_digest_options.push({"text":hmacarray[i][0] + " (Not recommended)","value":hmacarray[i][1]});
		else
			vpn_server_digest_options.push({"text":hmacarray[i][0],"value":hmacarray[i][1]});
	}
	var vpn_server_digest_parm = {"title": "<#vpn_openvpn_AuthHMAC#>", "id": "vpn_server_digest", "options": vpn_server_digest_options, "openHint":"32_26"};
	Get_Component_Custom_Select(vpn_server_digest_parm).appendTo($detail_adv_standard);

	var vpn_server_comp_options = [
		{"text":"<#WLANConfig11b_WirelessCtrl_buttonname#>","value":"-1"},
		{"text":"<#wl_securitylevel_0#>","value":"no"},
		{"text":"<#WLANConfig11b_WirelessCtrl_button1name#>","value":"yes"},
		{"text":"<#Adaptive#>","value":"adaptive"},
		{"text":"LZ4","value":"lz4"}
	];
	var vpn_server_comp_parm = {"title": "<#vpn_openvpn_Compression#>", "id": "vpn_server_comp", "options": vpn_server_comp_options, "openHint":"32_18"};
	Get_Component_Custom_Select(vpn_server_comp_parm).appendTo($detail_adv_standard);

	var vpn_server_igncrt_parm = {"title":"<#vpn_openvpn_AuthOnly#>", "type":"switch", "id":"vpn_server_igncrt", "openHint":"32_9"};
	Get_Component_Switch(vpn_server_igncrt_parm).appendTo($detail_adv_standard)
		.find("#" + vpn_server_igncrt_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			enable_server_igncrt($content_container, ($(this).hasClass("on") ? "1" : "0"));
			update_visibility($content_container);
		});

	var vpn_server_crypt_options = [{"text":"TLS","value":"tls"},{"text":"Static Key","value":"secret"}];
	var vpn_server_crypt_parm = {"title": "<#vpn_openvpn_Auth#>", "id": "vpn_server_crypt", "options": vpn_server_crypt_options, "openHint":"32_7"};
	Get_Component_Custom_Select(vpn_server_crypt_parm).appendTo($detail_adv_standard).find("#select_" + vpn_server_crypt_parm.id + "").children("div").click(function(e){
		e = e || event;
		e.stopPropagation();
		update_visibility($content_container);
	});
	$("<div>").attr("id", "Hint_fixed_tls_crypto").addClass("item_hint").html("* <#vpn_openvpn_AuthOnly_hint#>").appendTo($detail_adv_standard);

	var openvpn_keys_parm = {"title":"<#vpn_openvpn_ModifyKeys#>"};
	Get_Component_Edit_Keys(openvpn_keys_parm)
		.appendTo($detail_adv_standard)
		.find(".vpnc_keys_icon").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			show_popup_Keys_Cert($content_container);
		});

	var vpn_server_hmac_options = [
		{"text":"<#WLANConfig11b_WirelessCtrl_buttonname#>","value":"-1"},
		{"text":"Bi-directional","value":"2"},
		{"text":"Incoming (0)","value":"0"},
		{"text":"Incoming (1)","value":"1"}
	];
	var vpn_server_hmac_parm = {
		"title": "<#vpn_openvpn_ExtraHMAC#> (TLS-Auth)", "id": "vpn_server_hmac", "options": vpn_server_hmac_options,
		"openHint":"32_10", "container_id":"server_authhmac"
	};
	Get_Component_Custom_Select(vpn_server_hmac_parm).appendTo($detail_adv_standard);

	if(ipv6_item_flag){
		var vpn_server_ip6_parm = {"title":"Enable IPv6 Server mode", "type":"switch", "id":"vpn_server_ip6", "container_id":"server_ip6"};/* untranslated */
		Get_Component_Switch(vpn_server_ip6_parm).appendTo($detail_adv_standard)
			.find("#" + vpn_server_ip6_parm.id + "").click(function(e){
				e = e || event;
				e.stopPropagation();
				update_visibility($content_container);
			});

		var vpn_server_nat6_parm = {"title":"Enable NAT IPv6", "type":"switch", "id":"vpn_server_nat6", "container_id":"server_nat6"};/* untranslated */
		var $vpn_server_nat6 = Get_Component_Switch(vpn_server_nat6_parm).appendTo($detail_adv_standard)
			.find("#" + vpn_server_nat6_parm.id + "");
		if(!isSupport("ipv6nat")){
			$vpn_server_nat6.attr("temp_disable", "disabled");
			$("<div>").addClass("item_hint").attr({"id":"ipv6nat_hint"})
				.html("* The router does not support NAT IPv6 and all your VPN clients would get a global IPv6 allocated address.").appendTo($detail_adv_standard);/* untranslated */
		}
	}

	var server_snnm_parm = {"title":"<#vpn_openvpn_SubnetMsak#>", "openHint":"32_11",
		"type_1":"text", "id_1":"vpn_server_sn", "maxlength_1":15, "need_check_1":true,
		"type_2":"text", "id_2":"vpn_server_nm", "maxlength_2":15, "need_check_2":true, "container_id":"server_snnm"};
	var $server_snnm_items = Get_Component_Two_Input(server_snnm_parm).appendTo($detail_adv_standard);
	$server_snnm_items.find("#" + server_snnm_parm.id_1 + "").unbind("keypress").keypress(function(){
		return validator.isIPAddr(this, event);
	});
	$server_snnm_items.find("#" + server_snnm_parm.id_2 + "").unbind("keypress").keypress(function(){
		return validator.isIPAddr(this, event);
	});

	if(ipv6_item_flag){
		var vpn_server_sn6_parm = {"title":"IPv6 <#vpn_openvpn_SubnetMsak#>", "type":"text", "id":"vpn_server_sn6", "need_check":true, "maxlength":43,
			"container_id":"server_sn6"};
		Get_Component_Input(vpn_server_sn6_parm).appendTo($detail_adv_standard);
	}

	var vpn_server_dhcp_parm = {"title":"<#vpn_openvpn_dhcp#>", "type":"switch", "id":"vpn_server_dhcp", "openHint":"32_13", "container_id":"server_dhcp"};
	Get_Component_Switch(vpn_server_dhcp_parm).appendTo($detail_adv_standard)
		.find("#" + vpn_server_dhcp_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			update_visibility($content_container);
		});

	var server_range_parm = {"title":"<#vpn_openvpn_ClientPool#>", "openHint":"32_14",
		"type_1":"text", "id_1":"vpn_server_r1", "maxlength_1":15, "need_check_1":true,
		"type_2":"text", "id_2":"vpn_server_r2", "maxlength_2":15, "need_check_2":true, "container_id":"server_range"};
	var $server_range_items = Get_Component_Two_Input(server_range_parm).appendTo($detail_adv_standard);
	$server_range_items.find("#" + server_range_parm.id_1 + "").unbind("keypress").keypress(function(){
		return validator.isIPAddr(this, event);
	});
	$server_range_items.find("#" + server_range_parm.id_2 + "").unbind("keypress").keypress(function(){
		return validator.isIPAddr(this, event);
	});

	var server_local_parm = {"title":"<#vpn_openvpn_LocalRemote_IP#>", "openHint":"32_12",
		"type_1":"text", "id_1":"vpn_server_local", "maxlength_1":15, "need_check_1":true,
		"type_2":"text", "id_2":"vpn_server_remote", "maxlength_2":15, "need_check_2":true, "container_id":"server_local"};
	var $server_local_items = Get_Component_Two_Input(server_local_parm).appendTo($detail_adv_standard);
	$server_local_items.find("#" + server_local_parm.id_1 + "").unbind("keypress").keypress(function(){
		return validator.isIPAddr(this, event);
	});
	$server_local_items.find("#" + server_local_parm.id_2 + "").unbind("keypress").keypress(function(){
		return validator.isIPAddr(this, event);
	});

	if(ipv6_item_flag){
		var server_local_parm = {"title":"IPv6 <#vpn_openvpn_LocalRemote_IP#>",
			"type_1":"text", "id_1":"vpn_server_local6", "maxlength_1":39, "need_check_1":true,
			"type_2":"text", "id_2":"vpn_server_remote6", "maxlength_2":39, "need_check_2":true,
			"container_id":"server_local6"};
		Get_Component_Two_Input(server_local_parm).appendTo($detail_adv_standard);
	}

	var vpn_server_plan_parm = {"title":"<#vpn_openvpn_PushLAN#>", "type":"switch", "id":"vpn_server_plan", "openHint":"32_2", "container_id":"server_plan"};
	Get_Component_Switch(vpn_server_plan_parm).appendTo($detail_adv_standard);

	var vpn_server_rgw_parm = {"title":"<#vpn_openvpn_RedirectInternet#>", "type":"switch", "id":"vpn_server_rgw", "openHint":"32_3", "container_id":"server_rgw"};
	Get_Component_Switch(vpn_server_rgw_parm).appendTo($detail_adv_standard);

	var vpn_server_reneg_parm = {"title":"<#vpn_openvpn_TLSTime#> (<#Second#>)", "type":"text", "id":"vpn_server_reneg", "need_check":true, "maxlength":5, "openHint":"32_19", "container_id":"server_reneg"};
	Get_Component_Input(vpn_server_reneg_parm).appendTo($detail_adv_standard)
		.find("#" + vpn_server_reneg_parm.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isNumber(this,event);
		});

	var vpn_server_ccd_parm = {"title":"<#vpn_openvpn_SpecificOpt#>", "type":"switch", "id":"vpn_server_ccd", "container_id":"server_ccd"};
	Get_Component_Switch(vpn_server_ccd_parm).appendTo($detail_adv_standard)
		.find("#" + vpn_server_ccd_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			update_visibility($content_container);
		});

	var vpn_server_c2c_parm = {"title":"<#vpn_openvpn_ClientMutual#>", "type":"switch", "id":"vpn_server_c2c", "openHint":"32_20", "container_id":"server_c2c"};
	Get_Component_Switch(vpn_server_c2c_parm).appendTo($detail_adv_standard);

	var vpn_server_ccd_excl_parm = {"title":"<#vpn_openvpn_ClientSpecified#>", "type":"switch", "id":"vpn_server_ccd_excl", "openHint":"32_21", "container_id":"server_ccd_excl"};
	Get_Component_Switch(vpn_server_ccd_excl_parm).appendTo($detail_adv_standard)
		.find("#" + vpn_server_ccd_excl_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			update_visibility($content_container);
		});

	openvpn_allowed_clientlist_data = [];
	var $Add_Client_Title_obj = Get_Component_Add_Client_Title().appendTo($detail_adv_standard);
	$Add_Client_Title_obj.attr({"id":"openvpn_allowed_clientlist_title"})
	$Add_Client_Title_obj.find("#vpns_clientlist_title").html("Allowed Clients (<#List_limit#>" + htmlEnDeCode.htmlEncode(openvpn_allowed_clientlist_limit) + ")");
	$Add_Client_Title_obj.find("#vpns_clientlist_num").html(htmlEnDeCode.htmlEncode(openvpn_allowed_clientlist_data.length));
	$Add_Client_Title_obj.find("#vpns_clientlist_add").unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		if(openvpn_allowed_clientlist_data.length >= openvpn_allowed_clientlist_limit){
			show_customize_alert("<#weekSche_MAX_Num#>".replace("#MAXNUM", openvpn_allowed_clientlist_limit));
			return false;
		}
		show_popup_Add_Allowed_Client_OpenVPN($content_container);
	});
	var $openvpn_allowed_clientlist_bg = $("<div>").attr({"id":"openvpn_allowed_clientlist_bg"}).addClass("client_list_content_container").appendTo($detail_adv_standard);
	show_openvpn_allowed_clientlist($openvpn_allowed_clientlist_bg);

	var vpn_server_custom_parm = {"title":"<#vpn_openvpn_CustomConf#>", "id":"vpn_server_custom", "rows":"8", "cols":"55", "maxlength":"15000"};
	Get_Component_Textarea(vpn_server_custom_parm).appendTo($detail_adv_custom);

	$content_container.find("[detail_mode]").hide();
	$content_container.find("[detail_mode='1']").show();

	var $action_container = $("<div>").addClass("action_container").appendTo($content_container);
	var $btn_container_apply = $("<div>").addClass("btn_container apply").appendTo($action_container).html("<#CTL_apply1#>");
	var $action_loading_container = $("<div>").addClass("action_container loading").appendTo($content_container);

	$content_container.find("[need_check=true]").keyup(function(e){
		e = e || event;
		e.stopPropagation();
		set_apply_btn_status_OpenVPN($content_container);
	});

	setTimeout(function(){
		Update_Profile_Data_OpenVPN($content_container);
		set_apply_btn_status_OpenVPN($content_container);
		resize_iframe_height();
	},1);

	return $container;
}
