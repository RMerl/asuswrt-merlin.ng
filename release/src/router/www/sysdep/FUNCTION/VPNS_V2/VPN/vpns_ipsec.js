var ipsec_faq_href = {
	"windows": "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=114",
	"macOS": "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=115",
	"iPhone": "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=116",
	"android": "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=117"
};
var ipsec_clientlist_data = [];
var ipsec_clientlist_limit = 8;
var ipsec_profile_data = [];
var ipsec_connected_clients = [];
var ipsec_profile_attr = function(){
	this.profile_id = "";
	this.vpn_type = "";
	this.profilename = "";
	this.remote_gateway_method = "";
	this.remote_gateway = "";
	this.local_public_interface = "";
	this.local_public_ip = "";
	this.auth_method = "";
	this.preshared_key = "";
	this.local_subnet = "";
	this.local_port = "";
	this.remote_subnet = "";
	this.remote_port = "";
	this.transport_tunnel_type = "";
	this.virtual_ip = "";
	this.virtual_subnet = "";
	this.accessible_networks = "";
	this.ike = "";
	this.encryption_p1 = "";
	this.hash_p1 = "";
	this.exchange = "";
	this.local_id = "";
	this.remote_id = "";
	this.keylife_p1 = "";
	this.xauth = "";
	this.xauth_account = "";
	this.xauth_password = "";
	this.xauth_server_type = "";
	this.traversal = "";
	this.ike_isakmp = "";
	this.ike_isakmp_nat = "";
	this.dpd = "";
	this.dead_peer_detection = "";
	this.encryption_p2 = "";
	this.hash_p2 = "";
	this.keylife_p2 = "";
	this.keyingtries = "";
	this.samba_list = "";
	this.dns1 = "";
	this.dns2 = "";
	this.wins1 = "";
	this.wins2 = "";
	this.activate = "";
};
function init_ipsec_profile(_profile){
	var ipsec_profile = new ipsec_profile_attr();
	ipsec_profile.profile_id = "ipsec_profile_1";
	ipsec_profile.vpn_type = "4";
	ipsec_profile.profilename = "Host-to-Net";
	ipsec_profile.remote_gateway_method = "null";
	ipsec_profile.remote_gateway = "null";
	ipsec_profile.local_public_interface = "wan";
	ipsec_profile.local_public_ip = "";
	ipsec_profile.auth_method = "1";
	ipsec_profile.preshared_key = "";
	ipsec_profile.local_subnet = "null";
	ipsec_profile.local_port = "null";
	ipsec_profile.remote_subnet = "null";
	ipsec_profile.remote_port = "null";
	ipsec_profile.transport_tunnel_type = "null";
	ipsec_profile.virtual_ip = "1";
	ipsec_profile.virtual_subnet = "10.10.10"
	ipsec_profile.accessible_networks = "null";
	ipsec_profile.ike = "1";
	ipsec_profile.encryption_p1 = "null";
	ipsec_profile.hash_p1 = "null";
	ipsec_profile.exchange = "0";
	ipsec_profile.local_id = "null";
	ipsec_profile.remote_id = "null";
	ipsec_profile.keylife_p1 = "null";
	ipsec_profile.xauth = "1";
	ipsec_profile.xauth_account = "";
	ipsec_profile.xauth_password = "";
	ipsec_profile.xauth_server_type = "eap-md5";
	ipsec_profile.traversal = "1";
	ipsec_profile.ike_isakmp = "500";
	ipsec_profile.ike_isakmp_nat = "4500";
	ipsec_profile.dpd = "10";
	ipsec_profile.dead_peer_detection = "1";
	ipsec_profile.encryption_p2 = "null";
	ipsec_profile.hash_p2 = "null";
	ipsec_profile.keylife_p2 = "null";
	ipsec_profile.keyingtries = "null";
	ipsec_profile.samba_list = ""
	ipsec_profile.dns1 = ""
	ipsec_profile.dns2 = "";
	ipsec_profile.wins1 = "";
	ipsec_profile.wins2 = "";
	ipsec_profile.activate = "1";
	var ipsecLanIPAddr = ipsec_profile.virtual_subnet + ".1";
	var ipsecLanNetMask = "255.255.255.0";
	var ipConflict = checkIPConflict("LAN", ipsecLanIPAddr, ipsecLanNetMask);
	if(ipConflict.state) {
		ipsec_profile.virtual_subnet = "10.10.11";
	}
	if(_profile != undefined && _profile != ""){
		var editProfileArray = _profile.split(">");
		editProfileArray.unshift("ipsec_profile_1");
		ipsec_profile.local_public_interface = editProfileArray[5];
		ipsec_profile.preshared_key = editProfileArray[8];
		ipsec_profile.virtual_subnet = editProfileArray[15];
		ipsec_profile.dpd  = editProfileArray[31];
		ipsec_profile.dead_peer_detection = editProfileArray[32];
		ipsec_profile.samba_list = editProfileArray[37];
		var ipsec_samba_array = editProfileArray[37].split("<");
		ipsec_profile.dns1 = ipsec_samba_array[1];
		ipsec_profile.dns2 = ipsec_samba_array[2];
		ipsec_profile.wins1 = ipsec_samba_array[3];
		ipsec_profile.wins2 = ipsec_samba_array[4];
	}
	return ipsec_profile;
}
var ipsec_cert_info_status = false;
function show_popup_help_IPSec(_type){
	var init_environment = function(){
		$(".container").addClass("blur_effect");
		if($(".popup_container.popup_element").css("display") == "flex"){
			$(".popup_container.popup_element").addClass("blur_effect");
		}
		$(".popup_element_second").css("display", "flex");
		$(".popup_container.popup_element_second").empty();
	};
	if(_type == "Feature_Desc"){
		init_environment();
		$(".popup_container.popup_element_second").append(Get_Component_Feature_Desc_IPSec());
		adjust_popup_container_top($(".popup_container.popup_element_second"), 100);
	}
}
function Get_Component_Feature_Desc_IPSec(){
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

	var $feature_desc = $("<div>").addClass("feature_desc");
	$feature_desc.appendTo($popup_content_container);

	$("<div>").addClass("title").html("Feature Description").appendTo($feature_desc);/* untranslated */

	$("<div>").addClass("desc").html( $("#ipsec_desc").html()).appendTo($feature_desc);

	if(privateIP_flag){
		var $privateIP_notes = $(".hidden_static_text #privateIP_notes");
		$privateIP_notes.find("#faq_port_forwarding").attr("href", faq_port_forwarding_href);
		$("<div>").addClass("desc").html($privateIP_notes.html()).appendTo($feature_desc);
	}

	$("<div>").addClass("title").html("HOW TO SETUP").appendTo($feature_desc);/* untranslated */

	var $step_text_container = $("<div>").addClass("step_text_container");
		$step_text_container.appendTo($feature_desc);

	var $faq_windows = $("<a/>").attr({"id":"faq_windows", "target":"_blank", "href":ipsec_faq_href.windows}).html("Windows")
	$("<div>").addClass("step_text faq hyperlink").append($faq_windows).appendTo($step_text_container);

	var $faq_macOS = $("<a/>").attr({"id":"faq_macOS", "target":"_blank", "href":ipsec_faq_href.macOS}).html("Mac OS")
	$("<div>").addClass("step_text faq hyperlink").append($faq_macOS).appendTo($step_text_container);

	var $faq_iPhone = $("<a/>").attr({"id":"faq_iPhone", "target":"_blank", "href":ipsec_faq_href.iPhone}).html("iOS")
	$("<div>").addClass("step_text faq hyperlink").append($faq_iPhone).appendTo($step_text_container);

	var $faq_android = $("<a/>").attr({"id":"faq_android", "target":"_blank", "href":ipsec_faq_href.android}).html("Android")
	$("<div>").addClass("step_text faq hyperlink").append($faq_android).appendTo($step_text_container);

	return $container;
}
function Get_Component_IPSec_Client_IP(_parm){
	var $container = $("<div>").addClass("profile_setting_item client_IP_hint ipsec_client_IP");
	if(_parm.container_id != undefined)
		$container.attr("id", _parm.container_id);

	$container.append($("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)));

	var $input_container = $("<div>").addClass("input_container ipsec_clients").appendTo($container);
	var $input_start = $("<input/>")
						.addClass("textInput start")
						.attr({"id":_parm.start_id, "type":_parm.start_type, "maxlength":_parm.start_maxlength, "autocomplete":"off","autocorrect":"off","autocapitalize":"off","spellcheck":"false"})
						.val(htmlEnDeCode.htmlEncode(_parm.start_value))
						.unbind("blur").blur(function(e){
							e = e || event;
							e.stopPropagation();
						}).on('click', function () {
							var target = this;
							setTimeout(function(){
								target.scrollIntoViewIfNeeded();
							},400);
						})
						.appendTo($input_container);
	if(_parm.start_need_check)
		$input_start.attr("need_check", true);

	$("<span/>").html(".1 ~ ").appendTo($input_container);

	var $input_end = $("<div>")
						.addClass("textView")
						.attr({"id":_parm.end_id})
						.html(_parm.start_value + ".254")
						.appendTo($input_container);

	return $container;
}
function Get_Component_Cert_Status_IPSec(_parm){
	var $container = $("<div>").addClass("profile_setting_item cert_status_item");
	if(_parm.openHint != undefined){
		var hint_array = _parm.openHint.split("_");
		$("<a>").addClass("hintstyle").attr({"href":"javascript:void(0);"}).html(htmlEnDeCode.htmlEncode(_parm.title)).unbind("click").click(function(){
			openHint(hint_array[0], hint_array[1], "rwd_vpns");
		}).appendTo($("<div>").addClass("title").appendTo($container));
	}
	else
		$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)).appendTo($container);

	var $input_container = $("<div>").addClass("input_container").appendTo($container);
	var $cert_status = $("<div>").attr({"id":_parm.id}).appendTo($input_container);
	$("<span>").html("<#Status_Str#> : ").appendTo($cert_status);
	$("<span>").attr({"id":"cert_status"}).addClass("text_yellow").html("<#Authenticated_non#>").appendTo($cert_status);

	var $cert_issueTo = $("<div>").appendTo($input_container);
	$("<span>").html("<#vpn_ipsec_cert_to#> : ").appendTo($cert_issueTo);
	$("<span>").attr({"id":"cert_issueTo"}).addClass("text_yellow").appendTo($cert_issueTo);

	var $cert_issueBy = $("<div>").appendTo($input_container);
	$("<span>").html("<#vpn_ipsec_cert_by#> : ").appendTo($cert_issueBy);
	$("<span>").attr({"id":"cert_issueBy"}).addClass("text_yellow").appendTo($cert_issueBy);

	var $cert_expire = $("<div>").appendTo($input_container);
	$("<span>").html("<#vpn_openvpn_KC_expire#> : ").appendTo($cert_expire);
	$("<span>").attr({"id":"cert_expire"}).addClass("text_yellow").appendTo($cert_expire);

	return $container;
}
function show_popup_Add_Client_IPSec(){
	$(".container").addClass("blur_effect");
	if($(".popup_container.popup_element").css("display") == "flex"){
		$(".popup_container.popup_element").addClass("blur_effect");
	}
	$(".popup_element_second").css("display", "flex");
	$(".popup_container.popup_element_second").empty();
	$(".popup_container.popup_element_second").append(Get_Component_Add_Client_IPSec());
	adjust_popup_container_top($(".popup_container.popup_element_second"), 100);
}
function Get_Component_Add_Client_IPSec(){
	var set_apply_btn_status = function(_profileObj){
		var $btn_container_apply = $(_profileObj).find(".action_container .btn_container.apply");
		var validate_blank_flag = validate_blank_IPSec($(_profileObj));
		if(!validate_blank_flag){
			$btn_container_apply.removeClass("valid_fail").addClass("valid_fail").unbind("click");
		}
		else{
			$btn_container_apply.removeClass("valid_fail").unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				if(validate_format_IPSec($(_profileObj), "ipsec_clientlist")){
					var username = $(_profileObj).find("#clientlist_username").val();
					var password = $(_profileObj).find("#clientlist_password").val();
					var ike = $(_profileObj).find("#select_clientlist_ike").children(".selected").attr("value");
					ipsec_clientlist_data[username] = {"passwd":password, "ver":ike};
					show_ipsec_clientlist($("#ipsec_clientlist_bg"));
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

	var clientlist_username = {"title":"<#Username#>", "type":"text", "id":"clientlist_username", "need_check":true, "maxlength":32};
	Get_Component_Input(clientlist_username).appendTo($content_container)
		.find("#" + clientlist_username.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isString(this, event);
		});

	var clientlist_password = {"title":"<#HSDPAConfig_Password_itemname#>", "type":"text", "id":"clientlist_password", "need_check":true, "maxlength":32};
	Get_Component_Input(clientlist_password).appendTo($content_container)
		.find("#" + clientlist_password.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isString(this, event);
		})
		.unbind("keyup").keyup(function(){
			chkPass($(this).val(), "rwd_vpn_pwd", $("#client_pwd_strength"));
			resize_iframe_height();
		})
		.unbind("blur").blur(function(){
			if($(this).val() == "")
				$("#client_pwd_strength").hide();
		});

	$("<div>").attr({"id":"client_pwd_strength"}).append(Get_Component_PWD_Strength_Meter()).appendTo($content_container).hide();

	var clientlist_ike_options = [{"text":"V1","value":"1"},{"text":"V1&V2","value":"3"}];
	var clientlist_ike_options_parm = {"title": "<#vpn_ipsec_IKE_ver#>", "id": "clientlist_ike", "options": clientlist_ike_options, "set_value": "3"};
	Get_Component_Custom_Select(clientlist_ike_options_parm).appendTo($content_container);

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
function show_popup_System_Log_IPSec(_type){
	$(".container").addClass("blur_effect");
	if($(".popup_container.popup_element").css("display") == "flex"){
		$(".popup_container.popup_element").addClass("blur_effect");
	}
	$(".popup_element_second").css("display", "flex");
	$(".popup_container.popup_element_second").empty();
	$(".popup_container.popup_element_second").append(Get_Component_System_Log_IPSec());
	$(".popup_container.popup_element_second").addClass("fit_width");
	adjust_popup_container_top($(".popup_container.popup_element_second"), 100);
}
function Get_Component_System_Log_IPSec(){
	var $container = $("<div>");

	var $popup_title_container = $("<div>").addClass("popup_title_container").appendTo($container);
	$("<div>").addClass("title").html("<#System_Log#>").appendTo($popup_title_container);
	var $close_btn = $("<div>").addClass("vpn_icon_all_collect close_btn");
	$close_btn.appendTo($popup_title_container);
	$close_btn.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		close_popup_second();
	});

	var $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);

	$("<textarea/>").addClass("textareaInput").attr({"id":"ipsec_system_log", "rows":"30", "cols":"100", "readonly":"readonly", "wrap":"off"})
		.appendTo($("<div>").addClass("ipsec_system_log_bg").appendTo($content_container));

	var $action_container = $("<div>").addClass("action_container multi_btn three").appendTo($content_container);
	$("<div>").addClass("btn_container apply").appendTo($action_container).html("<#CTL_clear#>")
		.unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			httpApi.clean_ipsec_log(function(){
				$content_container.find("#ipsec_system_log").val("");
			});
		});
	$("<div>").addClass("btn_container apply").appendTo($action_container).html("<#CTL_onlysave#>")
		.unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			location.href = "/ipsec.log";
		});
	$("<div>").addClass("btn_container apply").appendTo($action_container).html("<#CTL_refresh#>")
		.unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			get_log_data_IPSec($content_container);
		});

	get_log_data_IPSec($content_container);

	return $container;
}
function get_log_data_IPSec(_obj){
	$.ajax({
		url: '/appGet.cgi?hook=nvram_dump(\"ipsec.log\","")',
		dataType: 'text',
		error: function(xhr){
			setTimeout(function(){
				get_log_data_IPSec($(_obj));
			},1000);
		},
		success: function(response){
			var logString = htmlEnDeCode.htmlEncode(response.toString().slice(25,-4)).replace(/&#39;/g, "'").replace(/&quot;/g, "\"").trim();
			$(_obj).find("#ipsec_system_log").val(logString);
			$(_obj).find("#ipsec_system_log").animate({ scrollTop: 9999999 }, "slow");
		}
	});
}
var interval_ipsec_client_status = false;
function show_ipsec_clientlist(_obj){
	$(_obj).empty();
	if(Object.keys(ipsec_clientlist_data).length == 0){
		$(_obj).append(Get_Component_Client_No_Item());
	}
	else{
		for(var username in ipsec_clientlist_data){
			if(ipsec_clientlist_data.hasOwnProperty(username)){
				var client_info = {
					"username": username,
					"password": ipsec_clientlist_data[username]["passwd"],
					"ver": ipsec_clientlist_data[username]["ver"]
				};
				Get_Component_Client_List_IPSec(client_info).appendTo($(_obj));
			}
		}
		update_ipsec_client_status(_obj);
		clearInterval(interval_ipsec_client_status);
		interval_ipsec_client_status = setInterval(function(){
			update_ipsec_client_status(_obj);
		}, 1000*3);
	}
	var title_id = $(_obj).attr("id").replace("_bg", "_title");
	$(_obj).closest(".popup_edit_profile_container").find("#"+title_id+" #vpns_clientlist_num").html(htmlEnDeCode.htmlEncode(Object.keys(ipsec_clientlist_data).length));
}
function Get_Component_Client_List_IPSec(_client_info){
	var $container = $("<div>").addClass("profile_setting_item nowrap clientlist");
	var $client_content_bg = $("<div>").attr({"username":htmlEnDeCode.htmlEncode(_client_info.username)}).addClass("client_content_bg").appendTo($container);
	var $client_info_bg = $("<div>").addClass("client_info_bg").appendTo($client_content_bg);

	$("<div>").addClass("vpn_icon_all_collect status_icon").appendTo($client_info_bg);
	var ver = _client_info.ver;
	if(ver == "2")
		ver = "3";
	if(ver != "3")
		ver = "V" + ver;
	else
		ver = "V1&V2";
	$("<div>").addClass("client_name").html(htmlEnDeCode.htmlEncode(_client_info.username) + " / " + htmlEnDeCode.htmlEncode(ver)).appendTo($client_info_bg);

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
		delete ipsec_clientlist_data[_client_info.username];
		show_ipsec_clientlist($("#ipsec_clientlist_bg"));
		resize_iframe_height();
	}).appendTo($client_contorl_bg);

	var margin_right = (48 * $client_contorl_bg.find(".contorl_btn").length) + 10;//contorl_btn width * count + detail_icon margin right
	$client_contorl_bg.css({"margin-right": -margin_right});

	return $container;
}
function set_ipsec_client_ip_end(_obj){
	var clients_subnet = $(_obj).find("#ipsec_clients_start").val().replace(/\s+/g, '');//remove space
	var ipAddr = clients_subnet + ".1";
	var ipformat  = /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
	if((ipformat.test(ipAddr))) {
		var ipAddrEnd = clients_subnet + ".254";
		$(_obj).find("#ipsec_clients_end").html(htmlEnDeCode.htmlEncode(ipAddrEnd));
	}
}
function ipsec_connected_status(_obj){
	if(select_vpn_type == "ipsec"){
		$(_obj).find(".status_icon").removeClass("connected").unbind("click").attr({"title":"<#Disconnected#>"});
		Object.keys(ipsec_connected_clients).forEach(function(_type){
			if(ipsec_connected_clients[_type] != undefined){
				$.each(ipsec_connected_clients[_type], function(index, value){
					if($(_obj).find("[username='"+value.username+"']").length){
						if(!$(_obj).find("[username='"+value.username+"']").find(".status_icon").hasClass("connected")){
							$(_obj).find("[username='"+value.username+"']").find(".status_icon").addClass("connected").attr({"title":"<#Connected#>"});
							$(_obj).find("[username='"+value.username+"']").find(".status_icon").unbind("click").click(function(){
								showIPSecVPNClients(value.username);
								resize_iframe_height();
							});
						}
					}
				});
			}
		});
	}
}
function update_ipsec_client_status(_obj){
	if(httpApi.nvramGet(["ipsec_server_enable"]).ipsec_server_enable == "1"){
		var get_ipsec_conn = httpApi.hookGet("get_ipsec_conn", true);
		var statusText = [[""], ["<#Connected#>"], ["<#Connecting_str#>"], ["<#Connecting_str#>"]];
		get_ipsec_conn.forEach(function(item, index, array){
			var profile_type = [];
			ipsec_connected_clients[get_ipsec_conn[index][0]] = [];
			if(item[0] != undefined && item[0] == get_ipsec_conn[index][0] && item[1] != undefined){
				var itemRow = item[1].split('<');
				for(var i = 0; i < itemRow.length; i += 1) {
					if(itemRow[i] != "") {
						var itemCol = itemRow[i].split('>');
						if(itemCol[6] == "IPSEC"){
							//ipaddr, conn_status, conn_period, account or device name, psk_reauth_time
							var client_info = {"ipaddr":itemCol[0], "conn_status":statusText[itemCol[1]][0], "conn_period":itemCol[2], "username":itemCol[3], "psk_reauth_time":itemCol[4]};
							profile_type.push(client_info);
						}
					}
				}
				ipsec_connected_clients[get_ipsec_conn[index][0]] = profile_type;
			}
		});
		ipsec_connected_status(_obj);
	}
}
function showIPSecVPNClients(uname){
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
		Object.keys(ipsec_connected_clients).forEach(function(_type){
			if(ipsec_connected_clients[_type] != undefined){
				$.each(ipsec_connected_clients[_type], function(index, value){
					if(value.username == uname){
						$("<div>").addClass("profile_title_item connect_client_title").append($("<span>").html("<#statusTitle_Client#> - " + idx)).appendTo($content_container);
						Get_Component_Pure_Text({"title":"Remote IP", "text":value.ipaddr}).appendTo($content_container);/* untranslated */
						Get_Component_Pure_Text({"title":"<#statusTitle_Client#>", "text":value.conn_status}).appendTo($content_container);
						Get_Component_Pure_Text({"title":"<#Access_Time#>", "text":value.conn_period}).appendTo($content_container);
						Get_Component_Pure_Text({"title":"PSKRAUTHTIME", "text":value.psk_reauth_time}).appendTo($content_container);
						idx++;
					}
				});
			}
		});

		return $container;
	};

	$(".popup_container.popup_element_second").append(Get_Clients_Connect_Status());

	adjust_popup_container_top($(".popup_container.popup_element_second"), 100);
}
function validate_blank_IPSec(_obj){
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
	else
		return true;
}
function validate_format_IPSec(_obj, _validField){
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
	var valid_psk = function(str){
		var testResult = {
			'isError': false,
			'errReason': '',
			'set_value': ''
		};
		var psk_length = str.length;
		var psk_length_trim = str.trim().length;
		if(psk_length < 8){
			testResult.isError = true;
			testResult.errReason = "<#JS_passzero#>";
			testResult.set_value = "00000000";
			return testResult;
		}
		if(psk_length > 31){
			testResult.isError = true;
			testResult.errReason = "<#JS_PSK64Hex#>";
			return testResult;
		}
		if(psk_length != psk_length_trim){
			testResult.isError = true;
			testResult.errReason = "<#JS_PSK64Hex_whiteSpace#>";
			return testResult;
		}
		if(psk_length == 64 && !check_is_hex(str)){
			testResult.isError = true;
			testResult.errReason = "<#JS_PSK64Hex#>";
			return testResult;
		}
		return testResult;
	};
	var valid_psk_KR = function(str){
		var testResult = {
			'isError': false,
			'errReason': ''
		};
		var psk_length = str.length;
		if(!/[A-Za-z]/.test(str) || !/[0-9]/.test(str) || psk_length < 8 || psk_length > 63 
				|| !/[\!\"\#\$\%\&\'\(\)\*\+\,\-\.\/\:\;\<\=\>\?\@\[\\\]\^\_\`\{\|\}\~]/.test(str)){
			testResult.isError = true;
			testResult.errReason = "<#JS_PSK64Hex_kr#> <#JS_validPWD#>";
			return testResult;
		}

		return testResult;
	};
	var check_is_hex = function(str){
		var re = new RegExp("[^a-fA-F0-9]+","gi");
		if(re.test(str))
			return false;
		else
			return true;
	};

	if(_validField == "ipsec_profile"){
		var $ipsec_preshared_key = $(_obj).find("#ipsec_preshared_key");
		if($ipsec_preshared_key.val() == ""){
			$ipsec_preshared_key.show_validate_hint("<#JS_fieldblank#>");
			$ipsec_preshared_key.focus();
			return false;
		}
		var isValid_preshared_key = valid_block_chars($ipsec_preshared_key.val(), [">", "<", "&", "\"", "null"]);
		if(isValid_preshared_key.isError){
			$ipsec_preshared_key.show_validate_hint(isValid_preshared_key.errReason);
			$ipsec_preshared_key.focus();
			return false;
		}
		if(is_KR_sku){
			isValid_preshared_key = valid_psk_KR($ipsec_preshared_key.val());
			if(isValid_preshared_key.isError){
				$ipsec_preshared_key.show_validate_hint(isValid_preshared_key.errReason);
				$ipsec_preshared_key.focus();
				return false;
			}
		}
		else{
			isValid_preshared_key = valid_psk($ipsec_preshared_key.val());
			if(isValid_preshared_key.isError){
				$ipsec_preshared_key.show_validate_hint(isValid_preshared_key.errReason);
				$ipsec_preshared_key.focus();
				if(isValid_preshared_key.set_value)
					$ipsec_preshared_key.val(isValid_preshared_key.set_value);
				return false;
			}
		}
		//confirm common string combination	#JS_common_passwd#
		var is_common_string = check_common_string($ipsec_preshared_key.val(), "wpa_key");
		if(is_common_string){
			if(!confirm("<#vpn_ipsec_PreShared_Key#> : <#JS_common_passwd#>")){
				$ipsec_preshared_key.focus();
				return false;
			}
		}

		var $ipsec_clients_start = $(_obj).find("#ipsec_clients_start");
		$ipsec_clients_start.val($ipsec_clients_start.val().replace(/\s+/g, ''));//remove space
		var ipAddr = $ipsec_clients_start.val() + ".1";
		var isValid_clients_start = valid_is_IP_format(ipAddr, "IPv4");
		if(isValid_clients_start.isError){
			$ipsec_clients_start.show_validate_hint(isValid_clients_start.errReason);
			$ipsec_clients_start.focus();
			return false;
		}

		var ipsecLanIPAddr = ipAddr;
		var ipsecLanNetMask = "255.255.255.0";
		var ipConflict = checkIPConflict("LAN", ipsecLanIPAddr, ipsecLanNetMask);
		if(ipConflict.state) {
			$ipsec_clients_start.show_validate_hint("<#JS_conflict_LANIP#>: " + ipConflict.ipAddr + ",\n" + "<#Network_segment#>: " + ipConflict.netLegalRangeStart + " ~ " + ipConflict.netLegalRangeEnd);
			$ipsec_clients_start.focus();
			return false;
		}

		var $ipsec_dpd = $(_obj).find("#ipsec_dpd");
		$ipsec_dpd.val($ipsec_dpd.val().replace(/\s+/g, ''));//remove space
		if($ipsec_dpd.val() == ""){
			$ipsec_dpd.show_validate_hint("<#JS_fieldblank#>");
			$ipsec_dpd.focus();
			return false;
		}
		var isValid_dpd = valid_num_range($ipsec_dpd.val(), 10, 900);
		if(isValid_dpd.isError){
			$ipsec_dpd.show_validate_hint(isValid_dpd.errReason);
			$ipsec_dpd.focus();
			return false;
		}

		var $ipsec_dns1 = $(_obj).find("#ipsec_dns1");
		$ipsec_dns1.val($ipsec_dns1.val().replace(/\s+/g, ''));//remove space
		if($ipsec_dns1.val() != ""){
			var isValid_dns1 = valid_block_chars($ipsec_dns1.val(), [">", "<"]);
			if(isValid_dns1.isError){
				$ipsec_dns1.show_validate_hint(isValid_dns1.errReason);
				$ipsec_dns1.focus();
				return false;
			}
			isValid_dns1 = valid_isLegalIP($ipsec_dns1.val());
			if(isValid_dns1.isError){
				$ipsec_dns1.show_validate_hint(isValid_dns1.errReason);
				$ipsec_dns1.focus();
				return false;
			}
		}

		var $ipsec_dns2 = $(_obj).find("#ipsec_dns2");
		$ipsec_dns2.val($ipsec_dns2.val().replace(/\s+/g, ''));//remove space
		if($ipsec_dns2.val() != ""){
			var isValid_dns2 = valid_block_chars($ipsec_dns2.val(), [">", "<"]);
			if(isValid_dns2.isError){
				$ipsec_dns2.show_validate_hint(isValid_dns2.errReason);
				$ipsec_dns2.focus();
				return false;
			}
			isValid_dns2 = valid_isLegalIP($ipsec_dns2.val());
			if(isValid_dns2.isError){
				$ipsec_dns2.show_validate_hint(isValid_dns2.errReason);
				$ipsec_dns2.focus();
				return false;
			}
		}

		var $ipsec_wins1 = $(_obj).find("#ipsec_wins1");
		$ipsec_wins1.val($ipsec_wins1.val().replace(/\s+/g, ''));//remove space
		if($ipsec_wins1.val() != ""){
			var isValid_wins1 = valid_block_chars($ipsec_wins1.val(), [">", "<"]);
			if(isValid_wins1.isError){
				$ipsec_wins1.show_validate_hint(isValid_wins1.errReason);
				$ipsec_wins1.focus();
				return false;
			}
			isValid_wins1 = valid_isLegalIP($ipsec_wins1.val());
			if(isValid_wins1.isError){
				$ipsec_wins1.show_validate_hint(isValid_wins1.errReason);
				$ipsec_wins1.focus();
				return false;
			}
		}

		var $ipsec_wins2 = $(_obj).find("#ipsec_wins2");
		$ipsec_wins2.val($ipsec_wins2.val().replace(/\s+/g, ''));//remove space
		if($ipsec_wins2.val() != ""){
			var isValid_wins2 = valid_block_chars($ipsec_wins2.val(), [">", "<"]);
			if(isValid_wins2.isError){
				$ipsec_wins2.show_validate_hint(isValid_wins2.errReason);
				$ipsec_wins2.focus();
				return false;
			}
			isValid_wins2 = valid_isLegalIP($ipsec_wins2.val());
			if(isValid_wins2.isError){
				$ipsec_wins2.show_validate_hint(isValid_wins2.errReason);
				$ipsec_wins2.focus();
				return false;
			}
		}
	}
	else if(_validField == "ipsec_clientlist"){
		var $clientlist_username = $(_obj).find("#clientlist_username");
		$clientlist_username.val($clientlist_username.val().replace(/\s+/g, ''));//remove space
		if($clientlist_username.val() == ""){
			$clientlist_username.show_validate_hint("<#JS_fieldblank#>");
			$clientlist_username.focus();
			return false;
		}
		var isValid_username = valid_block_chars($clientlist_username.val(), [" ", "@", "*", "+", "|", ":", "?", "<", ">", ",", ".", "/", ";", "[", "]", "\\", "=", "\"", "&" ]);
		if(isValid_username.isError){
			$clientlist_username.show_validate_hint(isValid_username.errReason);
			$clientlist_username.focus();
			return false;
		}

		if(ipsec_clientlist_data.hasOwnProperty($clientlist_username.val())) {
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
		var isValid_password = valid_block_chars($clientlist_password.val(), ["<", ">", "&", "\""]);
		if(isValid_password.isError){
			$clientlist_password.show_validate_hint(isValid_password.errReason);
			$clientlist_password.focus();
			return false;
		}
		else if($clientlist_password.val().length > 0 && $clientlist_password.val().length < 5){
			$clientlist_password.show_validate_hint("Password should be 5 to 32 characters.");/* untranslated */
			$clientlist_password.focus();
			return false;
		}

		//confirm common string combination	#JS_common_passwd#
		var is_common_string = check_common_string($clientlist_password.val(), "httpd_password");
		if($clientlist_password.val().length > 0 && is_common_string){
			if(!confirm("<#HSDPAConfig_Password_itemname#> : <#JS_common_passwd#>")){
				$clientlist_password.focus();
				return false;
			}
		}
	}
	return true;
}
function Update_Profile_Data_IPSec(_obj){
	var ipsec_profile_1 = (httpApi.nvramGet(["ipsec_profile_1"]).ipsec_profile_1).replace(/&#62/g, ">").replace(/&#60/g, "<");
	ipsec_profile_data = (JSON.parse(JSON.stringify(init_ipsec_profile(ipsec_profile_1))));

	if(!privateIP_flag){
		var ddns_settings = httpApi.nvramGet(["ddns_enable_x", "ddns_hostname_x"]);
		var set_ddns_text = '<a href="/Advanced_ASUSDDNS_Content.asp" target="_blank" style="text-decoration: underline; font-family:Lucida Console;"><#vpn_ipsec_set_DDNS#></a>';
		var set_ip_and_ddns_text = wanlink_ipaddr();
		if(!isMobile())
			set_ip_and_ddns_text += ', ' + set_ddns_text;
		if(ddns_settings.ddns_enable_x == "1" && ddns_settings.ddns_hostname_x != "") {
			$(_obj).find("#ipsec_server_addr").html(ddns_settings.ddns_hostname_x);
			if(!check_ddns_status() && !isMobile())
				$(_obj).find("#ipsec_server_addr").append(', ' + set_ddns_text);
		}
		else {
			$(_obj).find("#ipsec_server_addr").html(set_ip_and_ddns_text);
		}
	}

	httpApi.get_ipsec_cert_info(function(response){
		if(response != ""){
			if(response.issueTo != "" && response.issueTo != undefined)
				$(_obj).find("#cert_issueTo").html(htmlEnDeCode.htmlEncode(response.issueTo));
			if(response.issueBy != "" && response.issueBy != undefined)
				$(_obj).find("#cert_issueBy").html(htmlEnDeCode.htmlEncode(response.issueBy));
			if(response.expire != "" && response.expire != undefined)
				$(_obj).find("#cert_expire").html(htmlEnDeCode.htmlEncode(response.expire));

			$(_obj).find("#cert_status").html("<#Authenticated#>");
			ipsec_cert_info_status = true;
			if(response.update_state == "1"){
				$(_obj).find("#ipsec_renew_hint").show();
			}
		}
	});

	httpApi.get_ipsec_clientlist(function(response){
		ipsec_clientlist_data = response;
		show_ipsec_clientlist($(_obj).find("#ipsec_clientlist_bg"));
		resize_iframe_height();
	});

	$(_obj).find("#ipsec_preshared_key").val(htmlEnDeCode.htmlEncode(ipsec_profile_data.preshared_key));

	$(_obj).find("#ipsec_dead_peer_detection").removeClass("off on").addClass((function(){
		return ((ipsec_profile_data.dead_peer_detection == "1") ? "on" : "off");
	})());

	var $container_ipsec_dpd = $(_obj).find("#ipsec_dpd").val(htmlEnDeCode.htmlEncode(ipsec_profile_data.dpd)).closest("#container_ipsec_dpd");
	if((ipsec_profile_data.dead_peer_detection == "1"))
		 $container_ipsec_dpd.show();
	else
		 $container_ipsec_dpd.hide();

	$(_obj).find("#ipsec_clients_start").val(htmlEnDeCode.htmlEncode(ipsec_profile_data.virtual_subnet));
	set_ipsec_client_ip_end(_obj);

	$(_obj).find("#ipsec_dns1").val(htmlEnDeCode.htmlEncode(ipsec_profile_data.dns1));
	$(_obj).find("#ipsec_dns2").val(htmlEnDeCode.htmlEncode(ipsec_profile_data.dns2));
	$(_obj).find("#ipsec_wins1").val(htmlEnDeCode.htmlEncode(ipsec_profile_data.wins1));
	$(_obj).find("#ipsec_wins2").val(htmlEnDeCode.htmlEncode(ipsec_profile_data.wins2));
}
function changeAdvDeadPeerDetection(_obj){
	if($(_obj).find("#ipsec_dead_peer_detection").hasClass("on")){
		$(_obj).find("#container_ipsec_dpd").show();
	}
	else{
		$(_obj).find("#container_ipsec_dpd").hide().find("#ipsec_dpd").val(htmlEnDeCode.htmlEncode(ipsec_profile_data.dpd));
	}
}
function set_apply_btn_status_IPSec(_obj){
	var $btn_container_apply = $(_obj).find(".action_container .btn_container.apply");
	var validate_blank_flag = validate_blank_IPSec($(_obj), "ipsec_profile");
	if(!validate_blank_flag)
		$btn_container_apply.removeClass("valid_fail").addClass("valid_fail").unbind("click");
	else{
		$btn_container_apply.removeClass("valid_fail").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			if(validate_format_IPSec($(_obj), "ipsec_profile")){
				var profile_1 = JSON.parse(JSON.stringify(ipsec_profile_data));
				profile_1.preshared_key = $(_obj).find("#ipsec_preshared_key").val();
				profile_1.dead_peer_detection = (($(_obj).find("#ipsec_dead_peer_detection")).hasClass("on") ? "1" : "0");
				profile_1.dpd = $(_obj).find("#ipsec_dpd").val();
				profile_1.local_public_interface = $(_obj).find("#select_ipsec_local_public_interface").children(".selected").attr("value");
				profile_1.virtual_subnet = $(_obj).find("#ipsec_clients_start").val();
				var samba_list = "<" + $(_obj).find("#ipsec_dns1").val() + "<" + $(_obj).find("#ipsec_dns2").val()
					 + "<" + $(_obj).find("#ipsec_wins1").val() + "<" + $(_obj).find("#ipsec_wins2").val();
				profile_1.samba_list = samba_list;

				var ipsec_profile_1 = profile_1.vpn_type + ">" + profile_1.profilename + ">" + profile_1.remote_gateway_method + ">" + profile_1.remote_gateway + ">" +
				profile_1.local_public_interface + ">" + profile_1.local_public_ip + ">" + profile_1.auth_method + ">" + profile_1.preshared_key + ">" +
				profile_1.local_subnet + ">" + profile_1.local_port + ">" + profile_1.remote_subnet + ">" + profile_1.remote_port + ">" +
				profile_1.transport_tunnel_type + ">" + profile_1.virtual_ip + ">" + profile_1.virtual_subnet + ">" + profile_1.accessible_networks + ">" +
				profile_1.ike + ">" + profile_1.encryption_p1 + ">" + profile_1.hash_p1 + ">" + profile_1.exchange + ">" +
				profile_1.local_id + ">" + profile_1.remote_id + ">" + profile_1.keylife_p1 + ">" + profile_1.xauth + ">" +
				profile_1.xauth_account + ">" + profile_1.xauth_password + ">" + profile_1.xauth_server_type + ">" + profile_1.traversal + ">" +
				profile_1.ike_isakmp + ">" + profile_1.ike_isakmp_nat + ">" + profile_1.dpd + ">" + profile_1.dead_peer_detection + ">" +
				profile_1.encryption_p2 + ">" + profile_1.hash_p2 + ">" + profile_1.keylife_p2 + ">" + profile_1.keyingtries + ">" +
				profile_1.samba_list + ">" + profile_1.activate;

				var cert_address = "";
				var cert_address_info = httpApi.nvramGet(["ddns_hostname_x", "wan0_ipaddr"]);
				if(cert_address_info.ddns_hostname_x != "")
					cert_address = "@" + cert_address_info.ddns_hostname_x;
				else
					cert_address = cert_address_info.wan0_ipaddr;
				var ipsec_profile_2 = "4>Host-to-Netv2>null>null>wan>>0>null>null>null>null>null>null>1>"+profile_1.virtual_subnet+">null>2>null>null>0>"+cert_address;
				ipsec_profile_2 += ">null>null>0>>>eap-mschapv2>1>500>4500>10>1>null>null>null>null><<<<>1>pubkey>svrCert.pem>always>svrKey.pem>%identity";
				var ipsec_client_list_1 = "";
				var ipsec_client_list_2 = "";
				for(var username in ipsec_clientlist_data) {
					if(ipsec_clientlist_data.hasOwnProperty(username)) {
						var passwd = ipsec_clientlist_data[username]["passwd"];
						var ver = parseInt(ipsec_clientlist_data[username]["ver"]);
						if(ver & 1)
							ipsec_client_list_1 += "<" + username + ">" + passwd;
						if(ver & 2)
							ipsec_client_list_2 += "<" + username + ">" + passwd;
					}
				}

				var nvramSet_obj = {"action_mode": "apply", "rc_service": ""};
				var settings = httpApi.nvramGet(["ipsec_server_enable"]);
				if(settings.ipsec_server_enable == "1")
					nvramSet_obj.rc_service = "ipsec_start;";
				else
					nvramSet_obj.rc_service = "ipsec_stop;";

				nvramSet_obj.ipsec_profile_1 = ipsec_profile_1;
				nvramSet_obj.ipsec_profile_2 = ipsec_profile_2;
				nvramSet_obj.ipsec_client_list_1 = ipsec_client_list_1;
				nvramSet_obj.ipsec_client_list_2 = ipsec_client_list_2;
				httpApi.nvramSet(nvramSet_obj, function(){
					var time = 10;
					close_popup();
					showLoading(time);
					setTimeout(function(){
						httpApi.nvramGet(["ipsec_profile_1"], true);
						if(!window.matchMedia('(max-width: 575px)').matches)
							$("#srv_profile_list").children("[type='" + select_vpn_type + "']").addClass("selected").find(".svr_item_text_container").click();
					}, time*1000);
				}());

			}
		});
	}
}
function Get_Component_Setting_Profile_IPSec(_type){
	var ipsec_settings = httpApi.nvramGet(["ipsec_server_enable"]);
	var $container = $("<div>").addClass("popup_edit_profile_container");

	if(_type == "popup")
		$container.append(Get_Component_Popup_Profile_Title("IPSec VPN"));
	else
		$container.append(Get_Component_Profile_Title("IPSec VPN"));

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
			resize_iframe_height();
		});

	var $detail_general = $("<div>").attr("detail_mode","1").appendTo($content_container);

	var help_parm = {"title":"How to setup"};/* untranslated */
	Get_Component_Help(help_parm)
		.appendTo($detail_general)
		.find(".vpnc_help_icon").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			show_popup_help_IPSec("Feature_Desc");
		});

	var ipsec_server_addr_parm = {"title":"<#WLANAuthentication11a_ExAuthDBIPAddr_itemname#>", "id":"ipsec_server_addr", "text":"-"};
	Get_Component_Pure_Text(ipsec_server_addr_parm).appendTo($detail_general);

	var ipsec_system_log_parm = {"title":"<#System_Log#>", "id":"ipsec_system_log", "btn_text":"<#CTL_check_log#>"};
	Get_Component_Button(ipsec_system_log_parm).appendTo($detail_general)
		.find("#" + ipsec_system_log_parm.id+ "").click(function(e){
			e = e || event;
			e.stopPropagation();
			show_popup_System_Log_IPSec();
		});

	var ipsec_preshared_key_parm = {"title":"<#vpn_ipsec_PreShared_Key#>", "type":"password", "id":"ipsec_preshared_key", "need_check":true, "maxlength":32, "placeholder":"<#vpn_preshared_key_hint#>"};
	Get_Component_Input(ipsec_preshared_key_parm).appendTo($detail_general)
		.find("#" + ipsec_preshared_key_parm.id + "")
		.unbind("keyup").keyup(function(){
			chkPass($(this).val(), "rwd_vpn_pwd", $("#" + ipsec_preshared_key_parm.id + "_pwd_strength"));
			resize_iframe_height();
		})
		.unbind("blur").blur(function(){
			if($(this).val() == "")
				$("#" + ipsec_preshared_key_parm.id + "_pwd_strength").hide();
		})
		.val(htmlEnDeCode.htmlEncode());

	$("<div>").attr({"id":"" + ipsec_preshared_key_parm.id + "_pwd_strength"}).append(Get_Component_PWD_Strength_Meter()).appendTo($detail_general).hide();

	$("<div>").addClass("profile_title_item").append($("<span>").html("IKEv1 Config")).appendTo($detail_general);/* untranslated */

	var ipsec_exchange_mode_parm = {"title":"<#vpn_ipsec_Exchange_Mode#>", "id":"ipsec_exchange_mode", "text":"<#vpn_ipsec_Main_Mode#>"};
	Get_Component_Pure_Text(ipsec_exchange_mode_parm).appendTo($detail_general);

	var ipsec_dead_peer_detection_parm = {"title":"<#vpn_ipsec_DPD#>", "type":"switch", "id":"ipsec_dead_peer_detection"};
	Get_Component_Switch(ipsec_dead_peer_detection_parm).appendTo($detail_general)
		.find("#" + ipsec_dead_peer_detection_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			changeAdvDeadPeerDetection($content_container);
			resize_iframe_height();
		});

	var ipsec_dpd_parm = {"title":"<#vpn_ipsec_DPD_Checking_Interval#> (10~900 <#Second#>)", "type":"text", "id":"ipsec_dpd", "need_check":true,
		"maxlength":3, "container_id":"container_ipsec_dpd", "set_value":"10"};
	Get_Component_Input(ipsec_dpd_parm).appendTo($detail_general)
		.find("#" + ipsec_dpd_parm.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isNumber(this,event);
		});

	var $cert_status_bg = $("<div>").attr("id", "$cert_status_bg").appendTo($detail_general);
	var export_ovpn_parm = {"title":"<#vpn_export_ovpnfile#>", "type":"export_ovpn"};
	if(ipsec_settings.ipsec_server_enable == "1"){
		$cert_status_bg.show();
	}
	else{
		$cert_status_bg.hide();
	}

	$("<div>").addClass("profile_title_item").append($("<span>").html("IKEv2 Config")).appendTo($cert_status_bg);/* untranslated */

	var ipsec_cert_status_parm = {"title":"Certificate Status", "id":"ipsec_cert_status"};/* untranslated */
	Get_Component_Cert_Status_IPSec(ipsec_cert_status_parm).appendTo($cert_status_bg);

	var ipsec_export_cert_mode_options = [{"text":"<#vpn_ipsec_windows#>","value":"0"},{"text":"<#vpn_ipsec_mobile#>","value":"1"}];
	var ipsec_export_cert_mode_parm = {"title": "File format", "id": "ipsec_export_cert_mode", "options": ipsec_export_cert_mode_options, "display_type": "horizontal"};/* untranslated */
	Get_Component_Radio(ipsec_export_cert_mode_parm).appendTo($cert_status_bg);

	var ipsec_export_cert_parm = {"title":"<#vpn_export_cert#>", "id":"ipsec_export_cert", "btn_text":"<#btn_Export#>"};
	Get_Component_Button(ipsec_export_cert_parm).appendTo($cert_status_bg)
		.find("#" + ipsec_export_cert_parm.id+ "").click(function(e){
			e = e || event;
			e.stopPropagation();
			if(!ipsec_cert_info_status){
				show_customize_alert("Certificate Status : <#Authenticated_non#>");
				return;
			}
			var type = $content_container.find("#ipsec_export_cert_mode").find(".radio_container.selected").attr("value");//0:Windows, 1:Mobile
			if(type == "0")
				location.href = "/ikev2_cert_windows.der";
			else if(type == "1")
				location.href = "/ikev2_cert_mobile.pem";
		});

	var ipsec_renew_cert_parm = {"title":"<#vpn_ipsec_re_cert#>", "id":"ipsec_renew_cert", "btn_text":"<#CTL_renew#>", "openHint":"33_2"};
	Get_Component_Button(ipsec_renew_cert_parm).appendTo($cert_status_bg)
		.find("#" + ipsec_renew_cert_parm.id+ "").click(function(e){
			e = e || event;
			e.stopPropagation();
			if(!ipsec_cert_info_status){
				show_customize_alert("Certificate Status : <#Authenticated_non#>");
				return;
			}
			var ddns_enable_x = httpApi.nvramGet(["ddns_enable_x"]).ddns_enable_x;
			if(ddns_enable_x == "1"){
				if(!check_ddns_status()){
					show_customize_alert("Update failed, please check your DDNS setting.");/* untranslated */
					return false;
				}
			}
			var $btn_container = $(this);
			if(!$btn_container.find(".text.import_file").hasClass("loadingicon")){
				$btn_container.find(".text.import_file").removeClass("loadingicon").addClass("loadingicon");
				httpApi.renew_ikev2_cert_key(function(){
					var count = 0;
					var timer = 15;
					var interval_check = setInterval(function(){
						var ikev2_cert_state = httpApi.nvramGet(["ikev2_cert_state"], true).ikev2_cert_state;
						if(ikev2_cert_state == "3"){
							clearInterval(interval_check);
							$btn_container.find(".text.import_file").removeClass("loadingicon");
							show_customize_alert("<#vpn_ipsec_update_cert_success#>");
						}
						else{
							count++;
							if(count >= timer){
								clearInterval(interval_check);
								$btn_container.find(".text.import_file").removeClass("loadingicon");
								show_customize_alert("<#vpn_ipsec_update_cert_fail#>");
							}
						}
					}, 1000);
				});
			}
		});
	$("<div>").addClass("item_hint").attr({"id":"ipsec_renew_hint"}).html("* <#vpn_ipsec_need_re_cert_hint#>").hide().appendTo($cert_status_bg);

	ipsec_clientlist_data = [];
	var $Add_Client_Title_obj = Get_Component_Add_Client_Title().appendTo($detail_general);
	$Add_Client_Title_obj.attr({"id":"ipsec_clientlist_title"})
	$Add_Client_Title_obj.find("#vpns_clientlist_title").html("<#vpnc_title#> (<#List_limit#>" + htmlEnDeCode.htmlEncode(ipsec_clientlist_limit) + ")");
	$Add_Client_Title_obj.find("#vpns_clientlist_num").html(htmlEnDeCode.htmlEncode(Object.keys(ipsec_clientlist_data).length));
	$Add_Client_Title_obj.find("#vpns_clientlist_add").unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		if(Object.keys(ipsec_clientlist_data).length >= ipsec_clientlist_limit){
			show_customize_alert("<#weekSche_MAX_Num#>".replace("#MAXNUM", ipsec_clientlist_limit));
			return false;
		}
		show_popup_Add_Client_IPSec();
	});
	var $ipsec_clientlist_bg = $("<div>").attr({"id":"ipsec_clientlist_bg"}).addClass("client_list_content_container").appendTo($detail_general);
	show_ipsec_clientlist($ipsec_clientlist_bg);

	var $detail_adv = $("<div>").attr("detail_mode","2").appendTo($content_container);
	var ipsec_ike_isakmp_parm = {"title":"<#vpn_ipsec_IKE_ISAKMP_Port#>", "id":"ipsec_ike_isakmp", "text":"500"};
	Get_Component_Pure_Text(ipsec_ike_isakmp_parm).appendTo($detail_adv);

	var ipsec_ike_isakmp_nat_parm = {"title":"<#vpn_ipsec_IKE_ISAKMP_NAT_Port#>", "id":"ike_isakmp_nat", "text":"4500"};
	Get_Component_Pure_Text(ipsec_ike_isakmp_nat_parm).appendTo($detail_adv);

	var ipsec_clients_parm = {"title":"<#vpn_client_ip#>",
		"start_id":"ipsec_clients_start", "start_type":"text", "start_maxlength":11, "start_value":"10.10.10", "start_need_check":true,
		"end_id":"ipsec_clients_end"};
	var ipsecLanIPAddr = "10.10.10.1";
	var ipsecLanNetMask = "255.255.255.0";
	var ipConflict = checkIPConflict("LAN", ipsecLanIPAddr, ipsecLanNetMask);
	if(ipConflict.state){
		ipsec_clients_parm.start_value = "10.10.11";
	}
	var $ipsec_client_ip_items = Get_Component_IPSec_Client_IP(ipsec_clients_parm).appendTo($detail_adv);
	var $ipsec_client_ip_start = $ipsec_client_ip_items.find("#" + ipsec_clients_parm.start_id + "")
		.unbind("keypress").keypress(function(){
			return validator.isIPAddr(this, event);
		});

	var ipsec_dns1_parm = {"title":"<#IPConnection_x_DNSServer1_itemname#>", "type":"text", "id":"ipsec_dns1", "maxlength":15};
	Get_Component_Input(ipsec_dns1_parm).appendTo($detail_adv)
		.find("#" + ipsec_dns1_parm.id + "").unbind("keypress").keypress(function(){
		return validator.isIPAddr(this, event);
	});

	var ipsec_dns2_parm = {"title":"<#IPConnection_x_DNSServer2_itemname#>", "type":"text", "id":"ipsec_dns2", "maxlength":15};
	Get_Component_Input(ipsec_dns2_parm).appendTo($detail_adv)
		.find("#" + ipsec_dns2_parm.id + "").unbind("keypress").keypress(function(){
		return validator.isIPAddr(this, event);
	});

	var ipsec_wins1_parm = {"title":"<#IPConnection_x_WINSServer1_itemname#>", "type":"text", "id":"ipsec_wins1", "maxlength":15};
	Get_Component_Input(ipsec_wins1_parm).appendTo($detail_adv)
		.find("#" + ipsec_wins1_parm.id + "").unbind("keypress").keypress(function(){
		return validator.isIPAddr(this, event);
	});

	var ipsec_wins2_parm = {"title":"<#IPConnection_x_WINSServer2_itemname#>", "type":"text", "id":"ipsec_wins2", "maxlength":15};
	Get_Component_Input(ipsec_wins2_parm).appendTo($detail_adv)
		.find("#" + ipsec_wins2_parm.id + "").unbind("keypress").keypress(function(){
		return validator.isIPAddr(this, event);
	});

	var ipsec_local_public_interface_options = [];
	ipsec_local_public_interface_options.push({"text":"<#dualwan_primary#>","value":"wan"});
	if(isSupport("dualwan")){
		ipsec_local_public_interface_options.push({"text":"<#dualwan_secondary#>","value":"wan2"});
	}
	var ipsec_local_public_interface_parm = {"title": "<#vpn_ipsec_Local_Interface#>", "id": "ipsec_local_public_interface", "options": ipsec_local_public_interface_options};
	Get_Component_Custom_Select(ipsec_local_public_interface_parm).hide().appendTo($detail_adv);

	$content_container.find("[detail_mode]").hide();
	$content_container.find("[detail_mode='1']").show();

	var $action_container = $("<div>").addClass("action_container").appendTo($content_container);
	var $btn_container_apply = $("<div>").addClass("btn_container apply").appendTo($action_container).html("<#CTL_apply1#>");
	var $action_loading_container = $("<div>").addClass("action_container loading").appendTo($content_container);

	$content_container.find("[need_check=true]").keyup(function(e){
		e = e || event;
		e.stopPropagation();
		set_apply_btn_status_IPSec($content_container);
	});
	$ipsec_client_ip_start.blur(function(e){
		e = e || event;
		e.stopPropagation();
		set_ipsec_client_ip_end($content_container);
	});

	setTimeout(function(){
		Update_Profile_Data_IPSec($content_container);
		set_apply_btn_status_IPSec($content_container);
		resize_iframe_height();
	},1);

	return $container;
}