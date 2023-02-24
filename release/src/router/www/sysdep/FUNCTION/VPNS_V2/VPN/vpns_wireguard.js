var ui_lang = httpApi.nvramGet(["preferred_lang"]).preferred_lang;
var wgs_faq_href = {
	"setup_wgs": "https://nw-dlcdnet.asus.com/support/forward_test.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=163",
	"site_to_site": "https://nw-dlcdnet.asus.com/support/forward_test.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=164",
	"setup_wgc": "https://nw-dlcdnet.asus.com/support/forward_test.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=165"
};

httpApi.hookGet("get_wgs_parameter", true);

var wgs_unit = 1;
var wgs_clientlist_limit = 10;
var wgs_clientlist_data = [];
var wgs_enable = "0";
var wgs_clientlist_attr = function(){
	this.enable = "1";
	this.unit = "";
	this.name = "";
	this.addr = "";
	this.aips = "";
	this.caips = "";
};
function get_wgsc_unit(){
	var wgsc_unit = 1;
	for(wgsc_unit; wgsc_unit <= wgs_clientlist_limit; wgsc_unit += 1){
		var wgs_c_enable = "wgs" + wgs_unit + "_c" + wgsc_unit + "_enable";
		if(httpApi.nvramGet([wgs_c_enable])[wgs_c_enable] != "1")
			break;
	}
	if(wgsc_unit > 10)
		wgsc_unit = 1;

	return wgsc_unit;
}
function show_popup_help_WGS(){
	$(".container").addClass("blur_effect");
	if($(".popup_container.popup_element").css("display") == "flex"){
		$(".popup_container.popup_element").addClass("blur_effect");
	}
	$(".popup_element_second").css("display", "flex");
	$(".popup_container.popup_element_second").empty();
	$(".popup_container.popup_element_second").append(Get_Component_Feature_Desc_WGS());
	adjust_popup_container_top($(".popup_container.popup_element_second"), 100);

	function Get_Component_Feature_Desc_WGS(){
		var $container = $("<div>");
		var $popup_title_container = $("<div>").addClass("popup_title_container");
		$popup_title_container.appendTo($container);
		$("<div>").addClass("title").html("About Feature").appendTo($popup_title_container);/* untranslated */
		var $close_btn = $("<div>").addClass("vpn_icon_all_collect close_btn");
		$close_btn.appendTo($popup_title_container);
		$close_btn.unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			close_popup_container($container);
		});
		var $popup_content_container = $("<div>").addClass("popup_content_container").appendTo($container);
		var $feature_desc = $("<div>").addClass("feature_desc").appendTo($popup_content_container);
		$("<div>").addClass("title").html("HOW TO SETUP").appendTo($feature_desc);/* untranslated */
		var $step_text_container = $("<div>").addClass("step_text_container").appendTo($feature_desc);
		var $faq_setup_wgs = $("<a/>").attr({"target":"_blank", "href":wgs_faq_href.setup_wgs}).html("<#vpn_wireguard_server_faq#>");
		$("<div>").addClass("step_text faq hyperlink").append($faq_setup_wgs).appendTo($step_text_container);
		var $faq_site_to_site = $("<a/>").attr({"target":"_blank", "href":wgs_faq_href.site_to_site}).html("<#vpn_wireguard_site2site_faq#>");
		$("<div>").addClass("step_text faq hyperlink").append($faq_site_to_site).appendTo($step_text_container);
		var $faq_setup_wgce = $("<a/>").attr({"target":"_blank", "href":wgs_faq_href.setup_wgc}).html("<#vpn_wireguard_client_faq#>");
		$("<div>").addClass("step_text faq hyperlink").append($faq_setup_wgce).appendTo($step_text_container);

		return $container;
	}
}
function show_popup_Setup_Client_WGS(_wgsc_unit){
	$(".container").addClass("blur_effect");
	if($(".popup_container.popup_element").css("display") == "flex"){
		$(".popup_container.popup_element").addClass("blur_effect");
	}
	$(".popup_element_second").css("display", "flex");
	$(".popup_container.popup_element_second").empty();
	$(".popup_container.popup_element_second").append(Get_Component_Setup_Client_WGS(_wgsc_unit));
	adjust_popup_container_top($(".popup_container.popup_element_second"), 100);
}
function Get_Component_Setup_Client_WGS(_wgsc_unit){
	var _set_apply_btn_status = function(_profileObj){
		var $btn_container_apply = $(_profileObj).find(".action_container .btn_container.apply");
		var isBlank = validate_isBlank($(_profileObj));
		if(isBlank){
			$btn_container_apply.removeClass("valid_fail").addClass("valid_fail").unbind("click");
		}
		else{
			$btn_container_apply.removeClass("valid_fail").unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				if(validate_format_WGS($(_profileObj), "wgs_clientlist")){
					var isFirstClient = ((wgs_clientlist_data.length == 0) ? true : false);
					var nvramSet_obj = {"action_mode": "apply"};
					if(wgs_enable == "1"){
						nvramSet_obj.rc_service = "restart_wgsc " + _wgsc_unit + "";
					}
					else if(isFirstClient){
						nvramSet_obj.wgs_enable = "1";//the WGS will be activated automatically when client start up the profile.
						nvramSet_obj.rc_service = "restart_wgs;restart_dnsmasq;";
						var wgs_settings = httpApi.nvramGet(["wgs_lanaccess","wgs_addr", "wgs_port", "wgs_dns", "wgs_nat6", "wgs_psk", "wgs_alive"]);
						nvramSet_obj.wgs_lanaccess = wgs_settings.wgs_lanaccess;
						nvramSet_obj.wgs_addr = wgs_settings.wgs_addr;
						nvramSet_obj.wgs_port = wgs_settings.wgs_port;
						nvramSet_obj.wgs_dns = wgs_settings.wgs_dns;
						nvramSet_obj.wgs_nat6 = wgs_settings.wgs_nat6;
						nvramSet_obj.wgs_psk = wgs_settings.wgs_psk;
						nvramSet_obj.wgs_alive = wgs_settings.wgs_alive;
					}
					nvramSet_obj.wgs_unit = wgs_unit;
					nvramSet_obj.wgsc_unit = _wgsc_unit;
					nvramSet_obj.wgsc_enable = "1";
					nvramSet_obj.wgsc_name = $(_profileObj).find("#wgsc_name").val();
					nvramSet_obj.wgsc_addr = $(_profileObj).find("#wgsc_addr").val();
					nvramSet_obj.wgsc_aips = $(_profileObj).find("#wgsc_aips").val();
					nvramSet_obj.wgsc_caips = $(_profileObj).find("#wgsc_caips").val();
					nvramSet_obj.wgsc_caller = "Web";
					httpApi.nvramSet(nvramSet_obj, function(){
						if(isFirstClient){
							httpApi.hookGet("get_wgs_parameter", true);
							wgs_enable = httpApi.nvramGet(["wgs_enable"], true)["wgs_enable"];
						}
						var wgs_c_enable = "wgs" + wgs_unit + "_c" + _wgsc_unit + "_enable";
						var wgs_c_name = "wgs" + wgs_unit + "_c" + _wgsc_unit + "_name";
						var wgs_c_addr = "wgs" + wgs_unit + "_c" + _wgsc_unit + "_addr";
						var wgs_c_aips = "wgs" + wgs_unit + "_c" + _wgsc_unit + "_aips";
						var wgs_c_caips = "wgs" + wgs_unit + "_c" + _wgsc_unit + "_caips";
						httpApi.nvramGet([wgs_c_enable, wgs_c_name, wgs_c_addr, wgs_c_aips, wgs_c_caips, wgs_c_caips], true);
						Update_Profile_Data_WGS($(".profile_setting"));
						set_apply_btn_status_WGS($(".profile_setting"));
						if(show_config){
							httpApi.hookGet("get_wgsc_parameter", true);
							$(_profileObj).find(".action_container, .action_container_hint").hide();
							$(_profileObj).find(".action_container.loading").css("display", "flex");
							setTimeout(function(){
								$(_profileObj).find(".action_container, .action_container_hint").show();
								$(_profileObj).find(".action_container.loading").hide();
								$(_profileObj).find(".qr_code").css("background-image", "url(/wgs_client.png?random="+new Date().getTime()+")");
								get_wgs_client_conf($(_profileObj));
							},2000);
						}
						else{
							if(isFirstClient && wgs_enable == "1"){
								$("#srv_profile_list [type=wireguard] .switch").removeClass("off on").addClass("on");
							}
							if(wgs_enable == "1"){
								httpApi.hookGet("get_wgsc_parameter", true);
								$(_profileObj).find(".action_container, .action_container_hint").hide();
								$(_profileObj).find(".action_container.loading").css("display", "flex");
								setTimeout(function(){
									$(".popup_container.popup_element_second").empty();
									$(".popup_container.popup_element_second").append(Get_Component_Setup_Client_WGS(_wgsc_unit));
									$(".popup_container.popup_element_second").find(".action_container_hint").show();
								},2000);
							}
							else{
								close_popup_container($container);
							}
							resize_iframe_height();
						}
					});
				}
				else{
					if($(_profileObj).find("[data-slide_target=more_settings_item]").css("display") == "none"){
						$(_profileObj).find("#more_settings").click();
					}
				}
			});
		}
	};
	var $container = $("<div>");

	var $popup_title_container = $("<div>").addClass("popup_title_container").appendTo($container);
	$("<div>").addClass("title").html("Peer").appendTo($popup_title_container);/* untranslated */
	var $close_btn = $("<div>").addClass("vpn_icon_all_collect close_btn");
	$close_btn.appendTo($popup_title_container);
	$close_btn.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		close_popup_container($container);
	});

	var $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);
	$content_container.attr({"data-wgsc_unit":_wgsc_unit});

	var parm_wgsc_enable = "wgs" + wgs_unit + "_c" + _wgsc_unit + "_enable";
	var wgsc_enable = httpApi.nvramGet([parm_wgsc_enable])[parm_wgsc_enable];
	var show_config = false;
	show_config = ((wgs_enable == "1") && (wgsc_enable == "1"));
	if(show_config){
		var wgs_export_switch_parm = {"options": [{"text":"QR code","value":"1"},{"text":"<#btn_Export#>","value":"2"}]};/* untranslated */
		Get_Component_Switch_Two_Btn(wgs_export_switch_parm).appendTo($content_container)
			.find(".btn_options_container .btn_container").unbind("click").click(function(e){
				var options = $(this).attr("value");
				$(this).closest(".btn_options_container").find(".btn_container").removeClass("selected");
				$(this).addClass("selected");
				$(this).closest(".profile_setting").find("[data-wgs-export-mode]").hide();
				$(this).closest(".profile_setting").find("[data-wgs-export-mode='" + options + "']").show();
			});

		var $export_mode_qr = $("<div>").attr("data-wgs-export-mode","1").appendTo($content_container).show();
		var $export_mode_config = $("<div>").attr("data-wgs-export-mode","2").appendTo($content_container).hide();
		Get_Component_QR_Code().appendTo($export_mode_qr);
		Get_Component_Export_Config().appendTo($export_mode_config);

		function Get_Component_QR_Code(){
			var $container = $("<div>").addClass("profile_setting_item export_mode");
			var $export_mode_cntr = $("<div>").addClass("export_mode_container").appendTo($container);
			$("<div>").addClass("desc")
				.html(htmlEnDeCode.htmlEncode("Please download WireGuard App from Google Play or Apple Store, then use the APP to scan the QR Code to connect to this server."))/* untranslated */
				.appendTo($export_mode_cntr);
			var $notice_container = $("<div>").addClass("notice_container").appendTo($export_mode_cntr);
			$("<div>").addClass("notice_title").html("<#InternetSpeed_Notice#>").appendTo($notice_container);
			$("<div>").addClass("notice_desc").html(htmlEnDeCode.htmlEncode("For iOS users, you must assign a specific DNS server to WireGuard app before accessing the internet through WireGuard Server.")).appendTo($notice_container);/* untranslated */
			var $qr_code_cntr = $("<div>").addClass("qr_code_container").appendTo($export_mode_cntr);
			$("<div>").addClass("qr_code").css("background-image", "url(/wgs_client.png?random="+new Date().getTime()+")").appendTo($qr_code_cntr);
			$("<div>").addClass("qr_text").html(htmlEnDeCode.htmlEncode("<#Scan_QR_Code_For_Details#>")).appendTo($qr_code_cntr);
			return $container
		}
		function Get_Component_Export_Config(){
			var $container = $("<div>").addClass("profile_setting_item export_mode");
			var $export_mode_cntr = $("<div>").addClass("export_mode_container").appendTo($container);
			var $export_btn_cntr = $("<div>").addClass("export_btn_container").appendTo($export_mode_cntr);
			$("<div>").addClass("desc").html(htmlEnDeCode.htmlEncode("Please use the following configuration to set up your WireGuard client.")).appendTo($export_btn_cntr);
			var $export_btn = $("<div>").addClass("export_btn").unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				location.href = '/wgs_client.conf';
			}).appendTo($export_btn_cntr);
			$("<div>").addClass("text").html(htmlEnDeCode.htmlEncode("<#btn_Export#>")).appendTo($export_btn);
			var $config_content = $("<div>").addClass("config_container").appendTo($export_mode_cntr);
			$("<textarea/>").addClass("textareaInput").attr({"id":"wgsc_conf","readonly":true}).appendTo($config_content);
			return $container
		}
	}

	var wgsc_name_parm = {"title":"<#Username#>", "type":"text", "id":"wgsc_name", "need_check":true, "maxlength":64};
	if(!show_config){
		var $wgsc_name = Get_Component_Input(wgsc_name_parm).appendTo($content_container)
			.find("#" + wgsc_name_parm.id + "")
			.unbind("keypress").keypress(function(){
				return validator.isString(this, event);
			});
	}
	var more_settings_parm = {"title":"More Settings for Site to Site Usage", "id":"more_settings", "slide_target":"more_settings_item"};/* untranslated */
	Get_Component_Slide_Title(more_settings_parm).appendTo($content_container);
	var $more_settings_cntr = $("<div>").attr({"data-slide_target":"more_settings_item"}).hide().appendTo($content_container);

	if(show_config){
		var $wgsc_name = Get_Component_Input(wgsc_name_parm).appendTo($more_settings_cntr)
			.find("#" + wgsc_name_parm.id + "")
			.unbind("keypress").keypress(function(){
				return validator.isString(this, event);
			});
	}

	var wgsc_addr_parm = {"title":"Address", "type":"text", "id":"wgsc_addr", "need_check":true,  "maxlength":63};/* untranslated */
	var $wgsc_addr = Get_Component_Input(wgsc_addr_parm).appendTo($more_settings_cntr).find("#" + wgsc_addr_parm.id + "");

	var wgsc_aips_parm = {"title":"Allowed IPs (<#LANHostConfig_x_DDNSServer_itemname#>)", "type":"text", "id":"wgsc_aips", "need_check":true,  "maxlength":4095};/* untranslated */
	var $wgsc_aips = Get_Component_Input(wgsc_aips_parm).appendTo($more_settings_cntr).find("#" + wgsc_aips_parm.id + "");

	var wgsc_caips_parm = {"title":"Allowed IPs (Client)", "type":"text", "id":"wgsc_caips", "need_check":true,  "maxlength":4095};/* untranslated */
	var $wgsc_caips = Get_Component_Input(wgsc_caips_parm).appendTo($more_settings_cntr).find("#" + wgsc_caips_parm.id + "");

	if(show_config){
		var wgsc_update_key_parm = {"title":"Renew Key", "id":"wgsc_update_key", "btn_text":"<#CTL_renew#>"};/* untranslated */
		Get_Component_Button(wgsc_update_key_parm).appendTo($more_settings_cntr)
			.find("#" + wgsc_update_key_parm.id+ "").click(function(e){
				e = e || event;
				e.stopPropagation();
				var $input_btn = $(this);
				if(!$input_btn.find(".text.import_file").hasClass("loadingicon")){
				$input_btn.find(".text.import_file").removeClass("loadingicon").addClass("loadingicon");
				httpApi.nvramSet({
						"wgsc_priv": "",
						"wgsc_pub": "",
						"wgsc_psk": "",
						"wgs_unit": wgs_unit,
						"wgsc_unit": _wgsc_unit,
						"rc_service": "restart_wgsc " + _wgsc_unit + "",
						"action_mode": "apply"
					}, function(){
						setTimeout(function(){
							httpApi.hookGet("get_wgsc_parameter", true);
							var settings = httpApi.nvramGet(["wgsc_priv", "wgsc_pub", "wgsc_psk"], true);
							$content_container.find(".qr_code").css("background-image", "url(/wgs_client.png?random="+new Date().getTime()+")");
							get_wgs_client_conf($content_container);
							$input_btn.find(".text.import_file").removeClass("loadingicon");
						}, 2000);
					});
				}
			});
	}

	var $action_container = $("<div>").addClass("action_container").appendTo($content_container);
	var $btn_container_apply = $("<div>").addClass("btn_container apply").appendTo($action_container).html("<#CTL_apply#>");
	var isFirstClient = ((wgs_clientlist_data.length == 0) ? true : false);
	if(isFirstClient)
		$btn_container_apply.html("<#CTL_Apply_Enable#>");
	var $action_loading_container = $("<div>").addClass("action_container loading").appendTo($content_container);
	$("<div>").addClass("action_container_hint").html(htmlEnDeCode.htmlEncode("* <#vpn_ipsec_update_cert_success#>")).hide().appendTo($content_container);

	$content_container.find("[need_check=true]").keyup(function(e){
		e = e || event;
		e.stopPropagation();
		_set_apply_btn_status($content_container);
	});
	_set_apply_btn_status($content_container);

	var nvramSet_obj = {"action_mode": "apply", "wgs_unit": wgs_unit, "wgsc_unit": _wgsc_unit};
	httpApi.nvramSet(nvramSet_obj, function(){
		httpApi.hookGet("get_wgsc_parameter", true);
		var wgsc_settings = httpApi.nvramGet(["wgsc_name", "wgsc_addr", "wgsc_aips", "wgsc_caips"], true);
		$wgsc_addr.val(wgsc_settings.wgsc_addr);
		$wgsc_aips.val(wgsc_settings.wgsc_aips);
		$wgsc_caips.val(wgsc_settings.wgsc_caips);
		$wgsc_name.val(wgsc_settings.wgsc_name).keyup();
		if(show_config){
			get_wgs_client_conf($content_container);
		}
	});

	return $container;

	function get_wgs_client_conf(_obj){
		$.ajax({
			url: '/appGet.cgi?hook=nvram_dump(\"wgs_client.conf\","")',
			dataType: 'text',
			error: function(xhr){
				setTimeout(function(){
					get_wgs_client_conf($(_obj));
				},1000);
			},
			success: function(response){
				var logString = htmlEnDeCode.htmlEncode(response.toString().slice(31,-4)).replace(/&#39;/g, "'").replace(/&quot;/g, "\"").trim();
				$(_obj).find("#wgsc_conf").val(logString);
				$(_obj).find("#wgsc_conf").animate({ scrollTop: 0 }, "slow");
			}
		});
	}
}
var interval_wgs_client_status = false;
function show_wgs_clientlist(_obj){
	$(_obj).empty();

	if(wgs_clientlist_data.length == 0){
		$(_obj).append(Get_Component_Client_No_Item());
	}
	else{
		$.each(wgs_clientlist_data, function(index, value){
			Get_Component_Client_List_WGS(value).appendTo($(_obj));
		});

		update_wgs_client_status(_obj);
		clearInterval(interval_wgs_client_status);
		interval_wgs_client_status = setInterval(function(){
			update_wgs_client_status(_obj);
		}, 1000*3);
	}
	var title_id = $(_obj).attr("id").replace("_bg", "_title");
	$(_obj).closest(".popup_edit_profile_container").find("#"+title_id+" #vpns_clientlist_num").html(htmlEnDeCode.htmlEncode(wgs_clientlist_data.length));
}
function Update_wgs_clientlist_data(){
	wgs_clientlist_data = [];
	for(var wgsc_unit = 1; wgsc_unit <= wgs_clientlist_limit; wgsc_unit += 1){
		var wgs_c_enable = "wgs" + wgs_unit + "_c" + wgsc_unit + "_enable";
		if(httpApi.nvramGet([wgs_c_enable])[wgs_c_enable] == "1"){
			var wgs_c_name = "wgs" + wgs_unit + "_c" + wgsc_unit + "_name";
			var wgs_c_addr = "wgs" + wgs_unit + "_c" + wgsc_unit + "_addr";
			var wgs_c_aips = "wgs" + wgs_unit + "_c" + wgsc_unit + "_aips";
			var wgs_c_caips = "wgs" + wgs_unit + "_c" + wgsc_unit + "_caips";
			var wgsc_settings = httpApi.nvramGet([wgs_c_name, wgs_c_addr, wgs_c_aips, wgs_c_caips, wgs_c_caips]);
			var client_profile = new wgs_clientlist_attr();
			client_profile.enable = "1";
			client_profile.unit = wgsc_unit;
			client_profile.name = wgsc_settings[wgs_c_name];
			client_profile.addr = wgsc_settings[wgs_c_addr];
			client_profile.aips = wgsc_settings[wgs_c_aips];
			client_profile.caips = wgsc_settings[wgs_c_caips];
			wgs_clientlist_data.push(JSON.parse(JSON.stringify(client_profile)));
		}
	}
}
function Get_Component_Client_List_WGS(_client_info){
	var $container = $("<div>").addClass("profile_setting_item nowrap clientlist");
	var $client_content_bg = $("<div>").attr({"data-wgsc-unit":_client_info.unit}).addClass("client_content_bg").appendTo($container);
	var $client_info_bg = $("<div>").addClass("client_info_bg").css({"cursor":"pointer"}).unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		show_popup_Setup_Client_WGS(_client_info.unit);
	}).appendTo($client_content_bg);

	$("<div>").addClass("vpn_icon_all_collect status_icon").appendTo($client_info_bg);
	$("<div>").addClass("client_name").html(htmlEnDeCode.htmlEncode(_client_info.name)).appendTo($client_info_bg);

	var $client_contorl_bg = $("<div>").addClass("client_contorl_bg").appendTo($client_content_bg);
	$("<div>").addClass("detail_icon").unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		var hidden_contorl_width = $(this).closest(".client_contorl_bg").width() - $(this).width();
		if($(this).hasClass("active")){
			$(this).removeClass("active").closest(".client_contorl_bg").animate({"margin-right": "-=" + hidden_contorl_width + "px"}, 500);
		}
		else{
			$(this).closest("#pptpd_clientlist_bg").find(".detail_icon.active").removeClass("active")
				.closest(".client_contorl_bg").animate({"margin-right": "-=" + hidden_contorl_width + "px"}, 500);
			$(this).addClass("active").closest(".client_contorl_bg").animate({"margin-right": "+=" + hidden_contorl_width + "px"}, 500);
		}
	}).appendTo($client_contorl_bg);

	$("<div>").addClass("contorl_btn vpn_icon_all_collect del_icon").unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		var del_idx = "";
		$.each(wgs_clientlist_data, function(index, item){
			if(item.unit == _client_info.unit){
				del_idx = index;
				return false;
			}
		});
		if(del_idx !== ""){
			var del_wgsc = wgs_clientlist_data.splice(del_idx, 1);
			var nvramSet_obj = {
				"action_mode":"apply","wgs_unit":wgs_unit,"wgsc_enable":"0",
				"wgsc_name":"","wgsc_addr":"","wgsc_aips":"","wgsc_caips":"","wgsc_caller":"","wgsc_priv":"","wgsc_psk":"", "wgsc_pub":""
			};
			var _wgsc_unit =  del_wgsc[0].unit;
			nvramSet_obj.wgsc_unit =_wgsc_unit
			if(wgs_enable == "1"){
				nvramSet_obj.rc_service = "restart_wgsc " + _wgsc_unit + "";
			}
			httpApi.nvramSet(nvramSet_obj, function(){
				var wgs_c_enable = "wgs" + wgs_unit + "_c" + _wgsc_unit + "_enable";
				httpApi.nvramGet([wgs_c_enable], true);
				show_wgs_clientlist($("#wgs_clientlist_bg"));
			});
		}
	}).appendTo($client_contorl_bg);

	$("<div>").addClass("contorl_btn vpn_icon_all_collect config_icon").unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		show_popup_Setup_Client_WGS(_client_info.unit);
	}).appendTo($client_contorl_bg);

	var margin_right = (48 * $client_contorl_bg.find(".contorl_btn").length) + 10;//contorl_btn width * count + detail_icon margin right
	$client_contorl_bg.css({"margin-right": -margin_right});

	return $container;
}
function update_wgs_client_status(_obj){
	if(wgs_enable == "1"){
		if(select_vpn_type == "wireguard"){
			$(_obj).find(".status_icon").removeClass("connected").attr({"title":"<#Disconnected#>"});
			var get_wgsc_status = httpApi.hookGet("get_wgsc_status", true);
			if(get_wgsc_status.client_status != undefined){
				$.each(get_wgsc_status.client_status, function(index, value){
					if(value.status == "1"){
						if($(_obj).find("[data-wgsc-unit='"+value.index+"']").length){
							if(!$(_obj).find("[data-wgsc-unit='"+value.index+"']").find(".status_icon").hasClass("connected")){
								$(_obj).find("[data-wgsc-unit='"+value.index+"']").find(".status_icon").addClass("connected").attr({"title":"<#Connected#>"});
							}
						}
					}
				});
			}
		}
	}
}
function validate_isBlank(_obj){
	var isBlank = false;
	$(_obj).find("[need_check=true]").each(function(index){
		if($(this).closest(".profile_setting_item").css("display") == "none")
			return true;

		var value = $(this).val().replace(/\s+/g, '');//remove space
		if(value == ""){
			isBlank = true;
			return false;
		}
	});

	if(isBlank)
		return true;
	else
		return false;
}
function validate_format_WGS(_obj, _validField){
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
		var cidr_exist = str.indexOf("/");
		if(cidr_exist != -1)
			str = str.substr(0, cidr_exist);
		var format = new RegExp(ip_RegExp[type], "gi");
		return format.test(str);
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
	var wgsc_addr_duplicateCheck = function(addr, curr_wgsc_unit){
		var duplicate = false;
		if(curr_wgsc_unit != "0"){
			var wgs_addr = httpApi.nvramGet(["wgs_addr"]).wgs_addr;
			if(wgs_addr.indexOf(addr) != -1)
				duplicate = true;
		}

		var wgsc_unit = 1;
		for(wgsc_unit; wgsc_unit <= wgs_clientlist_limit; wgsc_unit += 1){
			if(curr_wgsc_unit == wgsc_unit)
				continue;
			var wgs_c_enable = "wgs" + wgs_unit + "_c" + wgsc_unit + "_enable";
			var wgs_c_addr = "wgs" + wgs_unit + "_c" + wgsc_unit + "_addr";
			if(httpApi.nvramGet([wgs_c_enable])[wgs_c_enable] == "1"){
				if(httpApi.nvramGet([wgs_c_addr])[wgs_c_addr].indexOf(addr) != -1){
					duplicate = true;
					break;
				}
			}
		}
		return duplicate;
	};
	if(_validField == "wgs_profile"){
		var $wgs_addr = $(_obj).find("#wgs_addr");//IPv4_CIDR, IPv6_CIDR
		$wgs_addr.val($wgs_addr.val().replace(/\s+/g, ''));//remove space
		if($wgs_addr.val().substr($wgs_addr.val().length-1) == ",")
			$wgs_addr.val($wgs_addr.val().slice(0,-1));//remove last ","
		if($wgs_addr.val() == ""){
			$wgs_addr.show_validate_hint("<#JS_fieldblank#>");
			$wgs_addr.focus();
			return false;
		}
		var isValid_wgs_addr = "";
		var wgs_addr_array = $wgs_addr.val().split(",");
		$.each(wgs_addr_array, function(index, address){
			if(address == "::/0")
				return true;
			if(wgsc_addr_duplicateCheck(address, "0")){
				isValid_wgs_addr = {'isError': true, 'errReason': address + " <#JS_duplicate#>"};
				return false;
			}
			if(valid_is_IP_format(address, "IPv4")){
				isValid_wgs_addr = valid_IP_CIDR(address, "IPv4", "1");
				if(isValid_wgs_addr.isError)
					return false;
			}
			else if(valid_is_IP_format(address, "IPv6")){
				isValid_wgs_addr = valid_IP_CIDR(address, "IPv6", "1");
				if(isValid_wgs_addr.isError)
					return false;
			}
			else{
				isValid_wgs_addr = {'isError': true, 'errReason': address + " <#JS_validip#>"};
				return false;
			}
		});
		if(isValid_wgs_addr.isError){
			$wgs_addr.show_validate_hint(isValid_wgs_addr.errReason);
			$wgs_addr.focus();
			return false;
		}

		var $wgs_port = $(_obj).find("#wgs_port");
		$wgs_port.val($wgs_port.val().replace(/\s+/g, ''));//remove space
		if($wgs_port.val() == ""){
			$wgs_port.show_validate_hint("<#JS_fieldblank#>");
			$wgs_port.focus();
			return false;
		}
		var isValid_wgs_port = valid_num_range($wgs_port.val(), 1, 65535);
		if(isValid_wgs_port.isError){
			$wgs_port.show_validate_hint(isValid_wgs_port.errReason);
			$wgs_port.focus();
			return false;
		}

		var $wgs_alive = $(_obj).find("#wgs_alive");
		$wgs_alive.val($wgs_alive.val().trim());
		if($wgs_alive.val() == ""){
			$wgs_alive.show_validate_hint("<#JS_fieldblank#>");
			$wgs_alive.focus();
			return false;
		}
		var isValid_wgs_alive = valid_num_range($wgs_alive.val(), 1, 65535);
		if(isValid_wgs_alive.isError){
			$wgs_alive.show_validate_hint(isValid_wgs_alive.errReason);
			$wgs_alive.focus();
			return false;
		}
	}
	else if(_validField == "wgs_clientlist"){
		var $wgsc_name = $(_obj).find("#wgsc_name");
		if($wgsc_name.val() == ""){
			$wgsc_name.show_validate_hint("<#JS_fieldblank#>");
			$valid_des.focus();
			return false;
		}
		var isValid_wgsc_name = valid_block_chars($wgsc_name.val(), []);
		if(isValid_wgsc_name.isError){
			$wgsc_name.show_validate_hint(isValid_wgsc_name.errReason);
			$wgsc_name.focus();
			return false;
		}

		var $wgsc_addr = $(_obj).find("#wgsc_addr");//IPv4_CIDR, IPv6_CIDR
		$wgsc_addr.val($wgsc_addr.val().replace(/\s+/g, ''));//remove space
		if($wgsc_addr.val().substr($wgsc_addr.val().length-1) == ",")
			$wgsc_addr.val($wgsc_addr.val().slice(0,-1));//remove last ","
		if($wgsc_addr.val() == ""){
			$wgsc_addr.show_validate_hint("<#JS_fieldblank#>");
			$wgsc_addr.focus();
			return false;
		}
		var isValid_wgsc_addr = "";
		var wgsc_addr_array = $wgsc_addr.val().split(",");
		var curr_wgsc_unit = $(_obj).attr("data-wgsc_unit");
		$.each(wgsc_addr_array, function(index, address){
			if(address == "::/0")
				return true;
			if(wgsc_addr_duplicateCheck(address, curr_wgsc_unit)){
				isValid_wgsc_addr = {'isError': true, 'errReason': address + " <#JS_duplicate#>"};
				return false;
			}
			if(valid_is_IP_format(address, "IPv4")){
				isValid_wgsc_addr = valid_IP_CIDR(address, "IPv4", "1");
				if(isValid_wgsc_addr.isError)
					return false;
			}
			else if(valid_is_IP_format(address, "IPv6")){
				isValid_wgsc_addr = valid_IP_CIDR(address, "IPv6", "1");
				if(isValid_wgsc_addr.isError)
					return false;
			}
			else{
				isValid_wgsc_addr = {'isError': true, 'errReason': address + " <#JS_validip#>"};
				return false;
			}
		});
		if(isValid_wgsc_addr.isError){
			$wgsc_addr.show_validate_hint(isValid_wgsc_addr.errReason);
			$wgsc_addr.focus();
			return false;
		}

		var $wgsc_aips = $(_obj).find("#wgsc_aips");//IPv4_CIDR, IPv6_CIDR
		$wgsc_aips.val($wgsc_aips.val().replace(/\s+/g, ''));//remove space
		if($wgsc_aips.val().substr($wgsc_aips.val().length-1) == ",")
			$wgsc_aips.val($wgsc_aips.val().slice(0,-1));//remove last ","
		if($wgsc_aips.val() == ""){
			$wgsc_aips.show_validate_hint("<#JS_fieldblank#>");
			$wgsc_aips.focus();
			return false;
		}
		var isValid_wgsc_aips = "";
		var wgsc_aips_array = $wgsc_aips.val().split(",");
		$.each(wgsc_aips_array, function(index, address){
			if(address == "::/0")
				return true;
			if(valid_is_IP_format(address, "IPv4")){
				isValid_wgsc_aips = valid_IP_CIDR(address, "IPv4", "1");
				if(isValid_wgsc_aips.isError)
					return false;
			}
			else if(valid_is_IP_format(address, "IPv6")){
				isValid_wgsc_aips = valid_IP_CIDR(address, "IPv6", "1");
				if(isValid_wgsc_aips.isError)
					return false;
			}
			else{
				isValid_wgsc_aips = {'isError': true, 'errReason': address + " <#JS_validip#>"};
				return false;
			}
		});
		if(isValid_wgsc_aips.isError){
			$wgsc_aips.show_validate_hint(isValid_wgsc_aips.errReason);
			$wgsc_aips.focus();
			return false;
		}

		var $wgsc_caips = $(_obj).find("#wgsc_caips");//IPv4_CIDR, IPv6_CIDR
		$wgsc_caips.val($wgsc_caips.val().replace(/\s+/g, ''));//remove space
		if($wgsc_caips.val().substr($wgsc_caips.val().length-1) == ",")
			$wgsc_caips.val($wgsc_caips.val().slice(0,-1));//remove last ","
		if($wgsc_caips.val() == ""){
			$wgsc_caips.show_validate_hint("<#JS_fieldblank#>");
			$wgsc_caips.focus();
			return false;
		}
		var isValid_wgsc_caips = "";
		var wgsc_caips_array = $wgsc_caips.val().split(",");
		$.each(wgsc_caips_array, function(index, address){
			if(address == "::/0")
				return true;
			if(valid_is_IP_format(address, "IPv4")){
				isValid_wgsc_caips = valid_IP_CIDR(address, "IPv4", "1");
				if(isValid_wgsc_caips.isError)
					return false;
			}
			else if(valid_is_IP_format(address, "IPv6")){
				isValid_wgsc_caips = valid_IP_CIDR(address, "IPv6", "1");
				if(isValid_wgsc_caips.isError)
					return false;
			}
			else{
				isValid_wgsc_caips = {'isError': true, 'errReason': address + " <#JS_validip#>"};
				return false;
			}
		});
		if(isValid_wgsc_caips.isError){
			$wgsc_caips.show_validate_hint(isValid_wgsc_caips.errReason);
			$wgsc_caips.focus();
			return false;
		}
	}
	return true;
}
function set_apply_btn_status_WGS(_obj){
	var $btn_container_apply = $(_obj).find(".action_container .btn_container.apply");
	var isBlank = validate_isBlank($(_obj));
	if(isBlank)
		$btn_container_apply.removeClass("valid_fail").addClass("valid_fail").unbind("click");
	else{
		$btn_container_apply.removeClass("valid_fail").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			if(validate_format_WGS($(_obj), "wgs_profile")){
				var nvramSet_obj = {"action_mode": "apply", "wgs_unit": wgs_unit};
				if(wgs_enable == "1"){
					nvramSet_obj.rc_service = "restart_wgs;restart_dnsmasq;";
				}
				nvramSet_obj.wgs_lanaccess = ($(_obj).find("#wgs_lanaccess").hasClass("on") ? "1" : "0");
				nvramSet_obj.wgs_addr = $(_obj).find("#wgs_addr").val();
				nvramSet_obj.wgs_port = $(_obj).find("#wgs_port").val();
				nvramSet_obj.wgs_dns = ($(_obj).find("#wgs_dns").hasClass("on") ? "1" : "0");
				nvramSet_obj.wgs_nat6 = ($(_obj).find("#wgs_nat6").hasClass("on") ? "1" : "0");
				nvramSet_obj.wgs_psk = ($(_obj).find("#wgs_psk").hasClass("on") ? "1" : "0");
				nvramSet_obj.wgs_alive = $(_obj).find("#wgs_alive").val();
				httpApi.nvramSet(nvramSet_obj, function(){
					var time = 3;
					close_popup();
					showLoading(time);
					setTimeout(function(){
						httpApi.hookGet("get_wgs_parameter", true);
						httpApi.nvramGet(["wgs_lanaccess","wgs_addr", "wgs_port", "wgs_dns", "wgs_nat6", "wgs_psk", "wgs_alive", "wgs_priv", "wgs_pub"], true);
						if(!window.matchMedia('(max-width: 575px)').matches)
							$("#srv_profile_list").children("[type='" + select_vpn_type + "']").addClass("selected").find(".svr_item_text_container").click();
					}, time*1000);
				});
			}
		});
	}
}
function Update_Profile_Data_WGS(_obj){
	var settings = httpApi.nvramGet(["wgs_lanaccess","wgs_addr", "wgs_port", "wgs_dns", "wgs_nat6", "wgs_psk", "wgs_alive", "wgs_priv", "wgs_pub"]);

	$(_obj).find("#wgs_lanaccess").removeClass("off on").addClass((function(){
		return ((settings.wgs_lanaccess == "1") ? "on" : "off");
	})());
	$(_obj).find("#wgs_addr").val(htmlEnDeCode.htmlEncode(settings.wgs_addr));
	$(_obj).find("#wgs_port").val(htmlEnDeCode.htmlEncode(settings.wgs_port));
	$(_obj).find("#wgs_dns").removeClass("off on").addClass((function(){
		return ((settings.wgs_dns == "1") ? "on" : "off");
	})());
	$(_obj).find("#wgs_nat6").removeClass("off on").addClass((function(){
		return ((settings.wgs_nat6 == "1") ? "on" : "off");
	})());
	$(_obj).find("#wgs_psk").removeClass("off on").addClass((function(){
		return ((settings.wgs_psk == "1") ? "on" : "off");
	})());
	$(_obj).find("#wgs_alive").val(htmlEnDeCode.htmlEncode(settings.wgs_alive));
	$(_obj).find("#wgs_priv").html(htmlEnDeCode.htmlEncode(settings.wgs_priv));
	$(_obj).find("#wgs_pub").html(htmlEnDeCode.htmlEncode(settings.wgs_pub));

	Update_wgs_clientlist_data();
	show_wgs_clientlist($(_obj).find("#wgs_clientlist_bg"));
}
function Get_Component_Setting_Profile_WGS(_type){
	wgs_enable = httpApi.nvramGet(["wgs_enable"]).wgs_enable;
	var $container = $("<div>").addClass("popup_edit_profile_container");

	if(_type == "popup")
		$container.append(Get_Component_Popup_Profile_Title("WireGuard"));
	else
		$container.append(Get_Component_Profile_Title("WireGuard"));

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
	Get_Component_Custom_Select(detail_options_parm)
		.appendTo($content_container)
		.find("#select_" + detail_options_parm.id + "").children("div").click(function(e){
			var options = $(this).attr("value");
			$(this).closest(".profile_setting").find("[detail_mode]").hide();
			$(this).closest(".profile_setting").find("[detail_mode='" + options + "']").show();
		});

	var $detail_general = $("<div>").attr("detail_mode","1").appendTo($content_container);
	var help_parm = {"title":"How to setup"};/* untranslated */
	Get_Component_Help(help_parm)
		.appendTo($detail_general)
		.find(".vpnc_help_icon").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			show_popup_help_WGS();
		});

	var wgs_lanaccess_parm = {"title":"<#Access_Intranet#>", "type":"switch", "id":"wgs_lanaccess"};
	Get_Component_Switch(wgs_lanaccess_parm).appendTo($detail_general);

	var wgs_addr_parm = {"title":"Tunnel IPv4 and / or IPv6 Address", "type":"text", "id":"wgs_addr", "need_check":true, "maxlength":63};/* untranslated */
	Get_Component_Input(wgs_addr_parm).appendTo($detail_general);

	var wgs_port_parm = {"title":"Listen Port", "type":"text", "id":"wgs_port", "need_check":true, "maxlength":5};/* untranslated */
	Get_Component_Input(wgs_port_parm).appendTo($detail_general)
		.find("#" + wgs_port_parm.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isNumber(this,event);
		});

	var $Add_Client_Title_obj = Get_Component_Add_Client_Title().appendTo($detail_general);
	$Add_Client_Title_obj.attr({"id":"wgs_clientlist_title"});
	$Add_Client_Title_obj.find("#vpns_clientlist_title").html("<#vpnc_title#> (<#List_limit#>" + htmlEnDeCode.htmlEncode(wgs_clientlist_limit) + ")");
	$Add_Client_Title_obj.find("#vpns_clientlist_num").html(htmlEnDeCode.htmlEncode("0"));
	$Add_Client_Title_obj.find("#vpns_clientlist_add").unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		if(wgs_clientlist_data.length >= wgs_clientlist_limit){
			show_customize_alert("<#weekSche_MAX_Num#>".replace("#MAXNUM", wgs_clientlist_limit));
			return false;
		}
		show_popup_Setup_Client_WGS(get_wgsc_unit());
	});
	var $wgs_clientlist_bg = $("<div>").attr({"id":"wgs_clientlist_bg"}).addClass("client_list_content_container").appendTo($detail_general);
	show_wgs_clientlist($wgs_clientlist_bg);

	var $detail_adv = $("<div>").attr("detail_mode","2").appendTo($content_container);
	var wgs_dns_parm = {"title":"<#ParentalCtrl_allow#> DNS", "type":"switch", "id":"wgs_dns"};
	Get_Component_Switch(wgs_dns_parm).appendTo($detail_adv);

	var wgs_nat6_parm = {"title":"<#Enable_NAT#> - IPv6", "type":"switch", "id":"wgs_nat6"};
	Get_Component_Switch(wgs_nat6_parm).appendTo($detail_adv);

	var wgs_psk_parm = {"title":"<#vpn_preshared_key#>", "type":"switch", "id":"wgs_psk"};
	Get_Component_Switch(wgs_psk_parm).appendTo($detail_adv);

	var wgs_alive_parm = {"title":"Persistent Keepalive", "type":"text", "id":"wgs_alive", "need_check":true, "maxlength":5};/* untranslated */
	Get_Component_Input(wgs_alive_parm).appendTo($detail_adv)
		.find("#" + wgs_alive_parm.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isNumber(this,event);
		});

	var wgs_priv_parm = {"title":"<#DDNS_https_cert_PrivateKey#>", "id":"wgs_priv", "text":"-"};
	Get_Component_Pure_Text(wgs_priv_parm).appendTo($detail_adv);

	var wgs_pub_parm = {"title":"Public Key", "id":"wgs_pub", "text":"-"};/* untranslated */
	Get_Component_Pure_Text(wgs_pub_parm).appendTo($detail_adv);

	var wgs_update_key_parm = {"title":"Renew Key", "id":"wgs_update_key", "btn_text":"<#CTL_renew#>"};/* untranslated */
	Get_Component_Button(wgs_update_key_parm).appendTo($detail_adv)
		.find("#" + wgs_update_key_parm.id+ "").click(function(e){
			e = e || event;
			e.stopPropagation();
			var $input_btn = $(this);
			if(!$input_btn.find(".text.import_file").hasClass("loadingicon")){
			$input_btn.find(".text.import_file").removeClass("loadingicon").addClass("loadingicon");
			httpApi.nvramSet({
					"wgs_priv": "",
					"wgs_pub": "",
					"wgs_unit": wgs_unit,
					"rc_service": "restart_wgs",
					"action_mode": "apply"
				}, function(){
					setTimeout(function(){
						httpApi.hookGet("get_wgs_parameter", true);
						var settings = httpApi.nvramGet(["wgs_priv", "wgs_pub"], true);
						$content_container.find("#wgs_priv").html(htmlEnDeCode.htmlEncode(settings.wgs_priv));
						$content_container.find("#wgs_pub").html(htmlEnDeCode.htmlEncode(settings.wgs_pub));
						$input_btn.find(".text.import_file").removeClass("loadingicon");
					}, 2000);
				});
			}
		});

	var $action_container = $("<div>").addClass("action_container").appendTo($content_container);
	var $btn_container_apply = $("<div>").addClass("btn_container apply").appendTo($action_container).html("<#CTL_apply1#>");
	var $action_loading_container = $("<div>").addClass("action_container loading").appendTo($content_container);

	$content_container.find("[detail_mode]").hide();
	$content_container.find("[detail_mode='1']").show();

	$content_container.find("[need_check=true]").keyup(function(e){
		e = e || event;
		e.stopPropagation();
		set_apply_btn_status_WGS($content_container);
	});

	setTimeout(function(){
		Update_Profile_Data_WGS($content_container);
		set_apply_btn_status_WGS($content_container);
		resize_iframe_height();
	},1);

	return $container;
}
