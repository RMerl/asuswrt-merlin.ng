function show_mlo_profilelist(){
	$("#profile_list_content").empty();
	let temp_mlo_list = [];
	$.each(sdn_all_rl_json, function(index, sdn_all_rl){
		if(sdn_all_rl.sdn_rl.idx == "0")
			return true;
		if(sdn_all_rl.sdn_rl.sdn_name != "MLO")
			return true;
		if(sdn_all_rl.apg_rl.mlo != "1" && sdn_all_rl.apg_rl.mlo != "2")
			return true;
		temp_mlo_list.push(sdn_all_rl)
	});
	let display_mlo_list = (()=>{
		let arr = [];
		$.each(temp_mlo_list, function(index, mlo_all_rl){
			if(mlo_all_rl.apg_rl.mlo == "1")
				arr.unshift(mlo_all_rl);
			else
				arr.push(mlo_all_rl)
		});
		return arr;
	})();
	$.each(display_mlo_list, function(index, mlo_all_rl){
		let item = {
			"sdn_idx": mlo_all_rl.sdn_rl.idx,
			"type": mlo_all_rl.sdn_rl.sdn_name,
			"name": mlo_all_rl.apg_rl.ssid,
			"activate": mlo_all_rl.sdn_rl.sdn_enable,
			"client_num": mlo_all_rl.client_num
		};
		let $profile_item = Get_Component_Profile_Item(item).appendTo($("#profile_list_content"));
		if(mlo_all_rl.apg_rl.mlo == "1"){//backhaul
			$profile_item.attr({"data-mlo-bh":true}).find(".item_text_container").unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				selected_sdn_idx = item.sdn_idx;
				const specific_data = sdn_all_rl_json.find(function(item, index, array){
					return (item.sdn_rl.idx == selected_sdn_idx);
				});
				if($(".profile_setting_container").css("display") == "none"){
					$(".popup_element").css("display", "flex");
					$(".container").addClass("blur_effect");
					$(".popup_container.popup_element").empty().append(Get_Component_Backhaul("popup"));
					Update_Setting_Backhaul($(".popup_container.popup_element"), specific_data);
					adjust_popup_container_top($(".popup_container.popup_element"), 100);
				}
				else{
					$(this).closest("#profile_list_content").find(".profile_item_container").removeClass("selected");
					$(this).closest(".profile_item_container").addClass("selected");
					$(".profile_setting_container").empty().append(function(){
						return Get_Component_Backhaul();
					});
					Update_Setting_Backhaul($(".profile_setting_container"), specific_data);
					resize_iframe_height();
				}
			});
		}
	});
	if(display_mlo_list.length > 0)
		$("#profile_list_content").append($("<div>").addClass("horizontal_line"));

	let $AddNew_MLO = Get_Component_Profile_Item_Add_MLO().appendTo($("#profile_list_content"));
	if($(".profile_setting_container").css("display") != "none"){
		if(selected_sdn_idx == ""){
			const $bh_profile = $("#profile_list_content .profile_item_container[data-mlo-bh=true]");
			if($bh_profile.length)
				$bh_profile.find(".item_text_container").click();
			else
				$(".profile_setting_container").empty().append(Get_Component_MLO_Introduce());
			resize_iframe_height();
		}
		else{
			$("#profile_list_content").find("[sdn_idx='"+selected_sdn_idx+"'] .item_text_container").click();
		}
	}
	else{
		$(".profile_setting_container").empty().append(Get_Component_MLO_Introduce());
	}
	clearInterval(interval_AccTime);
	let $AccTime_list = $("#profile_list_content").find("[data-container=AccTime][access_time=true]");
	if($AccTime_list.length > 0){
		interval_AccTime = setInterval(function(){
			$.each($AccTime_list, function(index, item){
				let end_time = parseInt($(item).attr("end_time"));
				let cur_time = parseInt($(item).attr("cur_time")) + 1;
				let remaining_time = ((end_time - cur_time) > 0) ? (end_time - cur_time) : 0;
				let HMS = secondsToHMS(remaining_time);
				$(item).attr({"cur_time": cur_time}).html(HMS.hours + ":" + HMS.minutes + ":" + HMS.seconds);
				if(remaining_time <= 0){
					init_sdn_all_list();
					show_mlo_profilelist();
				}
			});
		},1000);
	}
}
function show_feature_desc_mlo(){
	$(".container").addClass("blur_effect");
	if($(".popup_container.popup_element").css("display") == "flex"){
		$(".popup_container.popup_element").addClass("blur_effect");
	}
	$(".popup_element_second").css("display", "flex");
	$(".popup_container.popup_element_second").empty();

	$(".popup_container.popup_element_second").append(Get_Component_Feature_Desc("popup"));
	adjust_popup_container_top($(".popup_container.popup_element_second"), 100);
	resize_iframe_height();
}
function Get_Component_Feature_Desc(view_mode){
	let $container = $("<div>");

	if(view_mode == "popup"){
		let $popup_title_container = $("<div>").addClass("popup_title_container");
		$popup_title_container.appendTo($container);
		$("<div>").addClass("title").html("<#NewFeatureAbout#>").appendTo($popup_title_container);
		let $close_btn = $("<div>").addClass("vpn_icon_all_collect close_btn");
		$close_btn.appendTo($popup_title_container);
		$close_btn.unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			close_popup_container("all");
		});
	}
	else{
		$container.addClass("setting_content_container").css({"padding":"5%"});
	}

	let $popup_content_container = $("<div>").addClass("popup_content_container");
	$popup_content_container.appendTo($container);

	let $feature_desc_cntr = $("<div>").addClass("feature_desc_container").appendTo($popup_content_container);
	$("<div>").addClass("title").html("<#NewFeatureDesc#>").appendTo($feature_desc_cntr);
	$("<div>").addClass("feature_image").appendTo($feature_desc_cntr);
	$("<div>").addClass("desc").html(`<#WiFi_mlo_Desc#>`).appendTo($feature_desc_cntr);

	$("<div>").addClass("horizontal_line").appendTo($feature_desc_cntr);

	let $faq = $("<a/>").attr({"target":"_blank", "href":"https://www.asus.com/support/FAQ/1051272/"}).html(`WiFi7 Multi-link Operation (MLO)`);
	$("<div>").addClass("faq hyperlink").append($faq).appendTo($feature_desc_cntr);

	return $container;
}
function Get_Component_MLO_Introduce(view_mode){
	let $container = $("<div>");

	if(view_mode == "popup"){
		let $popup_title_container = $("<div>").addClass("popup_title_container");
		$popup_title_container.appendTo($container);
		let $close_btn = $("<div>").addClass("vpn_icon_all_collect close_btn");
		$close_btn.appendTo($popup_title_container);
		$close_btn.unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			close_popup_container("all");
		});
	}
	else{
		$container.addClass("setting_content_container");
	}

	let $popup_content_container = $("<div>").addClass("popup_content_container").appendTo($container);

	let $feature_desc_cntr = $("<div>").addClass("feature_desc_container introduce").appendTo($popup_content_container);
	$("<div>").addClass("feature_image introduce").appendTo($feature_desc_cntr);
	$("<div>").addClass("desc").html(
		`<#WiFi_mlo_Experience#>`
	).appendTo($feature_desc_cntr);
	$("<div>").addClass("introduce_hint").html(
		`<#WiFi_mlo_Advantage#>`
	).appendTo($feature_desc_cntr);
	if(is_Web_iframe){
		if(!check_sdn_legacy_exists()){
			$("<div>").addClass("profile_btn_container").html(`<#IFTTT_start1#>`.replace(`：`,'').replace(` :`,''))
				.unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					show_get_start_mlo();
				}).appendTo($feature_desc_cntr);
		}
	}

	return $container;
}
function show_get_start_mlo(){
	$(".container").addClass("blur_effect");
	if($(".popup_container.popup_element").css("display") == "flex"){
		$(".popup_container.popup_element").addClass("blur_effect");
	}
	$(".popup_element_second").css("display", "flex");
	$(".popup_container.popup_element_second").empty();

	$(".popup_container.popup_element_second").append(Get_Component_Get_Start("popup"));
	adjust_popup_container_top($(".popup_container.popup_element_second"), 100);
	resize_iframe_height();

	function Get_Component_Get_Start(view_mode){
		let $container = $("<div>");

		if(view_mode == "popup"){
			Get_Component_Popup_Profile_Title(`<#WiFi_mlo_Disable#>`).appendTo($container)
				.find("#title_close_btn").unbind("click").click(function(e){
					e = e || event;
					e.stopPropagation();
					close_popup_container($container);
				});
		}
		else
			Get_Component_Profile_Title(`<#WiFi_mlo_Disable#>`).appendTo($container);

		let $popup_content_container = $("<div>").addClass("popup_content_container");
		$popup_content_container.appendTo($container);

		let $feature_desc_cntr = $("<div>").addClass("feature_desc_container").appendTo($popup_content_container);
		$("<div>").addClass("title").html(`<#WiFi_mlo_Upgrade#>`).appendTo($feature_desc_cntr);
		$("<div>").addClass("desc").html(
			`<#WiFi_mlo_compatibility_Hint#>`
		).appendTo($feature_desc_cntr);
		$("<div>").addClass("feature_image2").appendTo($feature_desc_cntr);
		$("<div>").addClass("horizontal_line").appendTo($feature_desc_cntr);
		$("<div>").addClass("profile_btn_container").html(`<#IFTTT_start1#>`.replace(`：`,'').replace(` :`,''))
			.unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				top.location.href = "/QIS_wizard.htm?flag=mlo";
			}).appendTo($feature_desc_cntr);

		return $container;
	}
}
function Get_Component_Profile_Item_Add_MLO(){
	let $profile_item_container = $("<div>").addClass("profile_item_container addnew");
	$profile_item_container.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		let rule_num = 0;
		if(rule_num >= sdn_maximum){
			show_customize_alert("<#weekSche_MAX_Num#>".replace("#MAXNUM", sdn_maximum));
			return false;
		}
		$(".popup_container.popup_element").empty().append(show_popup_Wizard_Setting("MLO"));
	});
	let $item_text_container = $("<div>").addClass("item_text_container");
	$item_text_container.appendTo($profile_item_container);

	$("<div>").addClass("main_info").html("<#GuestNetwork_ProfileAdd#>").appendTo($item_text_container);

	return $profile_item_container;
}
function Get_Component_Backhaul(view_mode){
	let $container = $("<div>").addClass("setting_content_container");

	if(view_mode == "popup"){
		let $popup_title_cntr = $("<div>").addClass("popup_title_container edit_mode").appendTo($container);
		let $title_cntr = $("<div>").addClass("title_container").appendTo($popup_title_cntr);
		$("<div>").addClass("title").attr({"data-container":"profile_title"}).html(`<#AiMesh_WiFi_Backhaul#>`).appendTo($title_cntr);
		$("<div>").attr({"id":"title_close_btn"}).addClass("close_btn").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			close_popup_container($container);
		}).appendTo($popup_title_cntr);
	}
	else{
		let $title_cntr = $("<div>").addClass("profile_setting_title edit_mode").appendTo($container)
		$("<div>").addClass("title").attr({"data-container":"profile_title"}).html(`<#AiMesh_WiFi_Backhaul#>`).appendTo($title_cntr);
	}

	let $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);

	let sdn_name_parm = {"title":"<#QIS_finish_wireless_item1#>", "type":"text", "id":"sdn_name", "need_check":true, "maxlength":32, "openHint":"0_1"};
	let $sdn_name_cntr = Get_Component_Input(sdn_name_parm).appendTo($content_container)
	$sdn_name_cntr.find("#" + sdn_name_parm.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isString(this, event);
		});

	let wifi_auth_parm = {"title":"<#WLANConfig11b_AuthenticationMethod_itemname#>", "text":`WPA3-Personal`};
	Get_Component_Pure_Text(wifi_auth_parm).appendTo($content_container);

	let backhaul_pwd_parm = {"title":"<#QIS_finish_wireless_item2#>", "id":"backhaul_pwd"};
	Get_Component_Pure_Text(backhaul_pwd_parm).appendTo($content_container);

	let hide_ssid_parm = {"title":"<#WLANConfig11b_x_BlockBCSSID_itemname#>", "type":"switch", "id":"hide_ssid"};
	Get_Component_Switch(hide_ssid_parm).attr({"data-group":"wifi_settings"}).appendTo($content_container);

	let $action_container = $("<div>").attr({"id":"action_container"}).addClass("action_container").appendTo($content_container);
	let $btn_container_apply = $("<div>").attr({"id":"apply_btn"}).addClass("btn_container apply").appendTo($action_container).html("<#CTL_apply#>");

	$content_container.find("[need_check=true]").keyup(function(){
		set_apply_btn_Backhaul($content_container);
	});

	return $container;
}
function Update_Setting_Backhaul(_obj, _profile_data){
	$(_obj).find(".setting_content_container, .profile_setting").attr({"sdn_idx":_profile_data.sdn_rl.idx, "sdn_type":_profile_data.sdn_rl.sdn_name});
	$(_obj).find("#sdn_name").val(_profile_data.apg_rl.ssid);
	let apg_sec_array = (_profile_data.apg_rl.security).split("<");
	let cap_wifi_pwd = "";
	if(apg_sec_array[1] != undefined && apg_sec_array[1] != ""){
		let cap_sec_array = apg_sec_array[1].split(">");
		cap_wifi_pwd = cap_sec_array[3];
	}
	$(_obj).find("#backhaul_pwd").html(cap_wifi_pwd);
	$(_obj).find("#hide_ssid").removeClass("off on").addClass((function(){
		return ((_profile_data.apg_rl.hide_ssid == "1") ? "on" : "off");
	})());

	set_apply_btn_Backhaul($(_obj).find(".profile_setting"));
}
function set_apply_btn_Backhaul(_obj){
	let $btn_container_apply = $(_obj).find(".action_container .btn_container.apply");
	let isBlank = validate_isBlank($(_obj));
	if(isBlank){
		$btn_container_apply.removeClass("valid_fail").addClass("valid_fail").unbind("click");
	}
	else{
		$btn_container_apply.removeClass("valid_fail").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			let sdn_idx = $(_obj).closest(".setting_content_container").attr("sdn_idx");
			if(validate_format_Wizard_Item($(_obj), "ALL")){
				let specific_data = sdn_all_rl_json.filter(function(item, index, array){
					return (item.sdn_rl.idx == sdn_idx);
				})[0];
				let sdn_profile = {};
				if(specific_data == undefined){
					show_customize_alert(`<#ALERT_OF_ERROR_System4#>`);
					return;
				}
				else{
					sdn_profile = specific_data;
				}
				selected_sdn_idx = sdn_profile.sdn_rl.idx;
				sdn_profile.apg_rl.ssid = $(_obj).find("#sdn_name").val();
				sdn_profile.apg_rl.hide_ssid = ($(_obj).find("#hide_ssid").hasClass("on")) ? "1" : "0";
				let nvramSet_obj = {"action_mode": "apply", "rc_service": "restart_wireless;restart_sdn " + selected_sdn_idx + ";"};
				let showLoading_status = get_showLoading_status(nvramSet_obj.rc_service);
				let apgX_rl = parse_apg_rl_to_apgX_rl(sdn_profile.apg_rl);
				$.extend(nvramSet_obj, apgX_rl);
				if(!httpApi.app_dataHandler){
					showLoading();
					close_popup_container("all");
					if(isWLclient()){
						showLoading(showLoading_status.time);
						setTimeout(function(){
							showWlHintContainer();
						}, showLoading_status.time*1000);
						check_isAlive_and_redirect({"page": "MLO.asp", "time": showLoading_status.time});
					}
				}
				httpApi.nvramSet(nvramSet_obj, function(){
					if(isWLclient()) return;
					showLoading(showLoading_status.time);
					setTimeout(function(){
						init_sdn_all_list();
						show_mlo_profilelist();
						if(!window.matchMedia('(max-width: 575px)').matches)
							$("#profile_list_content").find("[sdn_idx='" + selected_sdn_idx + "'] .item_text_container").click();
					}, showLoading_status.time*1000);
					if(!isMobile()){
						if(showLoading_status.disconnect){
							check_isAlive_and_redirect({"page": "MLO.asp", "time": showLoading_status.time});
						}
					}
				});
			}
		});
	}
}
function check_sdn_legacy_exists(){
	let legacy_exist = false;
	let sdn_rl_count = 0;
	const each_sdn_rl = decodeURIComponent(httpApi.nvramCharToAscii(["sdn_rl"],true).sdn_rl).split("<");
	$.each(each_sdn_rl, function(index, value){
		if(value != ""){
			let profile_data = value.split(">");
			if(profile_data[1] == "LEGACY"){
				legacy_exist = true;
			}
			if(parseInt(profile_data[0]) > 0){
				sdn_rl_count++;
			}
		}
	});
	const sdn_rl_is_full = (sdn_rl_count >= sdn_maximum);
	return (legacy_exist || sdn_rl_is_full);
}
function check_mlo_meet_conditions(){
	const sdn_legacy = check_sdn_legacy_exists();
	const smart_connect_all_band = (()=>{
		let status = (httpApi.nvramGet(["smart_connect_x"]).smart_connect_x == "1") ? true : false;
		if(status && isSupport("smart_connect_v2")){
			status = (httpApi.nvramGet(["smart_connect_selif_x"]).smart_connect_selif_x == getSelifValue()) ? true : false;
		}
		return status;
	})();
	const wl_auth_wpa3 = (httpApi.nvramGet(["2g1_auth_mode_x"])["2g1_auth_mode_x"] == "sae") ? true : false;

	return (sdn_legacy && smart_connect_all_band && wl_auth_wpa3);

	function getSelifValue(){
		var val = 0;
		if(get_wl_unit_by_band("2G") != "") val += 1;
		if(get_wl_unit_by_band("5G1") != "") val += 2;
		if(get_wl_unit_by_band("5G2") != "") val += 4;
		if(get_wl_unit_by_band("6G1") != "") val += 8;
		if(get_wl_unit_by_band("6G2") != "") val += 16;
		return val;
	}
}
