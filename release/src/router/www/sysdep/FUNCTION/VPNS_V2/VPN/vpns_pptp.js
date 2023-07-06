var based_modelid = httpApi.nvramGet(["productid"]).productid;
var max_shift = "";	/* MODELDEP (include dict #PPTP_desc2# #vpn_max_clients# #vpn_maximum_clients#) */
if(based_modelid == "RT-AC5300" || based_modelid == "GT-AC5300" || based_modelid == "GT-AC9600" || based_modelid == "RT-AC3200" || based_modelid == "RT-AC3100" || based_modelid == "GT-AX11000" || based_modelid == "RT-AX92U" || based_modelid == "RT-AX95Q" || based_modelid == "XT8PRO" || based_modelid == "BM68" || based_modelid == "XT8_V2" || based_modelid == "RT-AXE95Q" || based_modelid == "ET8PRO" || based_modelid == "ET8_V2" || based_modelid == "RT-AX56_XD4" || based_modelid == "XD4PRO" || based_modelid == "CT-AX56_XD4" || based_modelid == "RT-AX58U" || based_modelid == "RT-AX58U_V2" || based_modelid == "TUF-AX3000" || based_modelid == "TUF-AX5400" || based_modelid == "DSL-AX82U" || based_modelid == "RT-AX82U" || based_modelid == "RT-AX56U" ||
		based_modelid == "RT-AC88U" || based_modelid == "RT-AX88U" || based_modelid == "RT-AC86U" || based_modelid == "GT-AC2900" || based_modelid == "RT-AC87U" || based_modelid == "RT-AC68U" || based_modelid == "RT-AX86U" || based_modelid == "RT-AX68U" || based_modelid == "RT-AC68U_V4" || based_modelid == "GT-AXE11000" || based_modelid == "GS-AX3000" || based_modelid == "GS-AX5400" || based_modelid == "GT-AX6000" || based_modelid == "GT-AX11000_PRO" || based_modelid == "ET12" || based_modelid == "XT12" || based_modelid == "GT-AXE16000" ||
		based_modelid == "RT-AC66U" || based_modelid == "RT-AC56U" ||
		based_modelid == "RT-N66U" || based_modelid == "RT-N18U" || based_modelid == "XC5" || based_modelid == "EBA63"){
	max_shift = parseInt("29");
}
else{
	max_shift = parseInt("9");
}
var faq_href = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang="+ui_lang+"&kw=&num=124";

var pptpd_clientlist_data = [];
var pptpd_connected_clients = [];
var pptpd_clientlist_limit = 16;

var pptpd_clientlist_attr = function(){
	this.username = "";
	this.password = "";
	this.hostIP = "";
	this.netmask = "";
};
function show_popup_help_PPTP(_type){
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
		$(".popup_container.popup_element_second").append(Get_Component_Feature_Desc_PPTP());
		adjust_popup_container_top($(".popup_container.popup_element_second"), 100);
	}
}
function Get_Component_Feature_Desc_PPTP(){
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

	var $pptp_desc = $("#pptp_desc");
	if(isMobile()){
		$pptp_desc.find("#desc3").empty();
		$pptp_desc.html($pptp_desc.html().replace("(7)", "(6)"));
	}
	$("<div>").addClass("desc").html($pptp_desc.html()).appendTo($feature_desc);

	if(privateIP_flag){
		var $privateIP_notes = $(".hidden_static_text #privateIP_notes");
		$privateIP_notes.find("#faq_port_forwarding").attr("href", faq_port_forwarding_href);
		$("<div>").addClass("desc").html($privateIP_notes.html()).appendTo($feature_desc);
	}

	$("<div>").addClass("title").html("<#HOWTOSETUP#>").appendTo($feature_desc);

	var $step_text_container = $("<div>").addClass("step_text_container");
	$step_text_container.appendTo($feature_desc);
	var $faq = $("<a/>").attr({"id":"faq", "target":"_blank", "href":faq_href}).html("<#BOP_isp_heart_item#> FAQ")
	$("<div>").addClass("step_text faq hyperlink").append($faq).appendTo($step_text_container);

	return $container;
}
function Get_Component_PPTP_Client_IP(_parm){
	var $container = $("<div>").addClass("profile_setting_item client_IP_hint");
	if(_parm.container_id != undefined)
		$container.attr("id", _parm.container_id);

	$container.append($("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(_parm.title)));

	var $input_container = $("<div>").addClass("input_container pptpd_clients").appendTo($container);
	var $input_start = $("<input/>")
						.addClass("textInput start")
						.attr({"id":_parm.start_id, "type":_parm.type, "maxlength":_parm.start_maxlength, "autocomplete":"off","autocorrect":"off","autocapitalize":"off","spellcheck":"false"})
						.val(htmlEnDeCode.htmlEncode(""))
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
	if(_parm.need_check)
		$input_start.attr("need_check", true);

	$("<span/>").html(" ~ ").appendTo($input_container);

	var $input_end = $("<input/>")
						.addClass("textInput end")
						.attr({"id":_parm.end_id, "type":_parm.type, "maxlength":_parm.end_maxlength, "autocomplete":"off","autocorrect":"off","autocapitalize":"off","spellcheck":"false"})
						.val(htmlEnDeCode.htmlEncode(""))
						.unbind("blur").blur(function(e){
							e = e || event;
							e.stopPropagation();
						})
						.on('click', function () {
							var target = this;
							setTimeout(function(){
								target.scrollIntoViewIfNeeded();
							},400);
						})
						.appendTo($input_container);

	if(_parm.need_check)
		$input_end.attr("need_check", true);

	$("<div>").addClass("item_hint").html("* <#vpn_maximum_clients#>").appendTo($container);

	return $container;
}
function show_popup_Add_Client_PPTP(){
	$(".container").addClass("blur_effect");
	if($(".popup_container.popup_element").css("display") == "flex"){
		$(".popup_container.popup_element").addClass("blur_effect");
	}
	$(".popup_element_second").css("display", "flex");
	$(".popup_container.popup_element_second").empty();
	$(".popup_container.popup_element_second").append(Get_Component_Add_Client_PPTP());
	adjust_popup_container_top($(".popup_container.popup_element_second"), 100);
}
function Get_Component_Add_Client_PPTP(){
	var set_apply_btn_status = function(_profileObj){
		var $btn_container_apply = $(_profileObj).find(".action_container .btn_container.apply");
		var validate_blank_flag = validate_blank_PPTP($(_profileObj), "pptp_clientlist");
		if(!validate_blank_flag){
			$btn_container_apply.removeClass("valid_fail").addClass("valid_fail").unbind("click");
		}
		else{
			$btn_container_apply.removeClass("valid_fail").unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				if(validate_format_PPTP($(_profileObj), "pptp_clientlist")){
					var client_profile = new pptpd_clientlist_attr();
					client_profile.username = $(_profileObj).find("#pptpd_clientlist_username").val();
					client_profile.password = $(_profileObj).find("#pptpd_clientlist_password").val();
					client_profile.hostIP = $(_profileObj).find("#pptpd_sr_ipaddr").val();
					client_profile.netmask = $(_profileObj).find("#pptpd_sr_netmask").val();
					pptpd_clientlist_data.push(JSON.parse(JSON.stringify(client_profile)));
					show_pptpd_clientlist($("#pptpd_clientlist_bg"));
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

	var pptpd_clientlist_username = {"title":"<#Username#>", "type":"text", "id":"pptpd_clientlist_username", "need_check":true, "maxlength":64};
	Get_Component_Input(pptpd_clientlist_username).appendTo($content_container)
		.find("#" + pptpd_clientlist_username.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isString(this, event);
		})
		.val(htmlEnDeCode.htmlEncode());

	var pptpd_clientlist_password = {"title":"<#HSDPAConfig_Password_itemname#>", "type":"text", "id":"pptpd_clientlist_password", "need_check":true, "maxlength":64};
	Get_Component_Input(pptpd_clientlist_password).appendTo($content_container)
		.find("#" + pptpd_clientlist_password.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isString(this, event);
		})
		.unbind("keyup").keyup(function(){
			chkPass($(this).val(), "rwd_vpn_pwd", $("#client_pwd_strength"));
		})
		.unbind("blur").blur(function(){
			if($(this).val() == "")
				$("#client_pwd_strength").hide();
		});

	$("<div>").attr({"id":"client_pwd_strength"}).append(Get_Component_PWD_Strength_Meter()).appendTo($content_container).hide();

	$("<div>").addClass("profile_title_item").append($("<span>").html("Static Route (<#feedback_optional#>)")).appendTo($content_container);
	var pptpd_sr_ipaddr = {"title":"<#RouterConfig_GWStaticIP_itemname#>", "type":"text", "id":"pptpd_sr_ipaddr", "maxlength":15, "openHint":"6_1"};
	Get_Component_Input(pptpd_sr_ipaddr).appendTo($content_container)
		.find("#" + pptpd_sr_ipaddr.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isIPAddr(this, event);
		});

	var pptpd_sr_netmask = {"title":"<#RouterConfig_GWStaticMask_itemname#>", "type":"text", "id":"pptpd_sr_netmask", "maxlength":15, "openHint":"6_2"};
	Get_Component_Input(pptpd_sr_netmask).appendTo($content_container)
		.find("#" + pptpd_sr_netmask.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isIPAddr(this, event);
		});

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
function Get_Component_PPTP_Edit_Client(username){
	var specific_client = pptpd_clientlist_data.filter(function(item, index, array){
		return (item.username == username);
	})[0];

	var $container = $("<div>");

	var $popup_title_container = $("<div>").addClass("popup_title_container").appendTo($container);
	$("<div>").addClass("title").html(htmlEnDeCode.htmlEncode(username)).appendTo($popup_title_container);
	var $close_btn = $("<div>").addClass("vpn_icon_all_collect close_btn");
	$close_btn.appendTo($popup_title_container);
	$close_btn.unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		close_popup_second();
	});

	var $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);

	$("<div>").addClass("profile_title_item").append($("<span>").html("Static Route (<#feedback_optional#>)")).appendTo($content_container);
	var pptpd_sr_ipaddr = {"title":"<#RouterConfig_GWStaticIP_itemname#>", "type":"text", "id":"pptpd_sr_ipaddr", "maxlength":15, "openHint":"6_1"};
	Get_Component_Input(pptpd_sr_ipaddr).appendTo($content_container)
		.find("#" + pptpd_sr_ipaddr.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isIPAddr(this, event);
		})
		.val(htmlEnDeCode.htmlEncode(specific_client.hostIP));

	var pptpd_sr_netmask = {"title":"<#RouterConfig_GWStaticMask_itemname#>", "type":"text", "id":"pptpd_sr_netmask", "maxlength":15, "openHint":"6_2"};
	Get_Component_Input(pptpd_sr_netmask).appendTo($content_container)
		.find("#" + pptpd_sr_netmask.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isIPAddr(this, event);
		})
		.val(htmlEnDeCode.htmlEncode(specific_client.netmask));

	var $action_container = $("<div>").addClass("action_container").appendTo($content_container);
	var $btn_container_apply = $("<div>").addClass("btn_container apply").html("<#CTL_ok#>").unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		if(validate_format_PPTP($content_container, "pptp_client_static_route")){
			if(specific_client != undefined){
				specific_client.hostIP = $content_container.find("#pptpd_sr_ipaddr").val();
				specific_client.netmask = $content_container.find("#pptpd_sr_netmask").val();
			}
			show_pptpd_clientlist($("#pptpd_clientlist_bg"));
			close_popup_second();
		}
	}).appendTo($action_container);

	return $container;
}
var interval_pptp_client_status = false;
function show_pptpd_clientlist(_obj){
	$(_obj).empty();
	clearInterval(interval_pptp_client_status);
	if(pptpd_clientlist_data.length == 0){
		$(_obj).append(Get_Component_Client_No_Item());
	}
	else{
		$.each(pptpd_clientlist_data, function(index, value){
			Get_Component_Client_List_PPTP(value).appendTo($(_obj));
		});
		pptpd_connected_status(_obj);
		clearInterval(interval_pptp_client_status);
		interval_pptp_client_status = setInterval(function(){
			update_pptp_client_status(_obj);
		}, 1000*3);
	}
	$(_obj).closest(".popup_edit_profile_container").find("#vpns_clientlist_num").html(htmlEnDeCode.htmlEncode(pptpd_clientlist_data.length));
}
function Get_Component_Client_List_PPTP(_client_info){
	var $container = $("<div>").addClass("profile_setting_item nowrap clientlist");
	var $client_content_bg = $("<div>").attr({"username":htmlEnDeCode.htmlEncode(_client_info.username)}).addClass("client_content_bg").appendTo($container);
	var $client_info_bg = $("<div>").addClass("client_info_bg").appendTo($client_content_bg);

	$("<div>").addClass("vpn_icon_all_collect status_icon").appendTo($client_info_bg);
	$("<div>").addClass("client_name").html(htmlEnDeCode.htmlEncode(_client_info.username)).appendTo($client_info_bg);

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
		$.each(pptpd_clientlist_data, function(index, item){
			if(item.username == _client_info.username){
				del_idx = index;
				return false;
			}
		});
		if(del_idx !== ""){
			pptpd_clientlist_data.splice(del_idx, 1);
			show_pptpd_clientlist($("#pptpd_clientlist_bg"));
		}
	}).appendTo($client_contorl_bg);

	$("<div>").addClass("contorl_btn vpn_icon_all_collect edit_icon").unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		$(".container").addClass("blur_effect");
		if($(".popup_container.popup_element").css("display") == "flex")
			$(".popup_container.popup_element").addClass("blur_effect");
		$(".popup_element_second").css("display", "flex");
		$(".popup_container.popup_element_second").empty().append(Get_Component_PPTP_Edit_Client(_client_info.username));
	}).appendTo($client_contorl_bg);

	var margin_right = (48 * $client_contorl_bg.find(".contorl_btn").length) + 10;//contorl_btn width * count + detail_icon margin right
	$client_contorl_bg.css({"margin-right": -margin_right});

	return $container;
}
function pptpd_connected_status(_obj){
	if(select_vpn_type == "pptp"){
		$(_obj).find(".status_icon").removeClass("connected").unbind("click").attr({"title":"<#Disconnected#>"});
		pptpd_connected_clients = [];
		var Loginfo = $("#pptp_connected_info :first-child").html();
		var lines = Loginfo.split('\n');
		if(Loginfo == "")
			return;

		Loginfo = Loginfo.replace('\r\n', '\n');
		Loginfo = Loginfo.replace('\n\r', '\n');
		Loginfo = Loginfo.replace('\r', '\n');
		for (i = 0; i < lines.length; i++){
			var fields = lines[i].split(' ');
			if(fields.length != 5)
				continue;

			pptpd_connected_clients.push({"username":fields[4],"remoteIP":fields[3],"VPNIP":fields[2]});
		}

		$.each(pptpd_connected_clients, function(index, value){
			if($(_obj).find("[username='"+value.username+"']").length){
				if(!$(_obj).find("[username='"+value.username+"']").find(".status_icon").hasClass("connected")){
					$(_obj).find("[username='"+value.username+"']").find(".status_icon").addClass("connected").attr({"title":"<#Connected#>"});
					$(_obj).find("[username='"+value.username+"']").find(".status_icon").unbind("click").click(function(){
						showPPTPClients(value.username);
					});
				}
			}
		});
	}
}
function update_pptp_client_status(_obj){
	if(httpApi.nvramGet(["pptpd_enable"]).pptpd_enable == "1"){
		$.ajax({
			url: '/ajax_vpnserver_client_status.xml',
			dataType: 'xml',
			error: function(xml){
				update_pptp_client_status(_obj);
			},
			success: function(xml){
				$("#pptp_connected_info").children(":first").empty();
				if(xml.getElementsByTagName("pptp")[0].firstChild != null) {
					$("#pptp_connected_info").children(":first").html(xml.getElementsByTagName("pptp")[0].firstChild.nodeValue);
				}
				pptpd_connected_status(_obj);
			}
		});
	}
}
function showPPTPClients(uname){
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
		$.each(pptpd_connected_clients, function(index, value){
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
function set_pptp_client_ip_end(_obj){
	var end = "";
	var clients_start = $(_obj).find("#pptpd_clients_start").val().replace(/\s+/g, '');//remove space
	var clients_start_ip = parseInt(clients_start.split(".")[3]);

	end = clients_start_ip + max_shift;
	if(end > 254)
		end = 254;

	if(!end)
		$(_obj).find("#pptpd_clients_end").val("");
	else
		$(_obj).find("#pptpd_clients_end").val(end);

	$(_obj).find("#pptpd_clients_end").blur();
}
function validate_blank_PPTP(_obj, _validField){
	if(_validField == "pptp_profile"){
		var $pptpd_dnsenable_x = $(_obj).find("#pptpd_dnsenable_x");
		var $pptpd_dns1 = $(_obj).find("#pptpd_dns1");
		var $pptpd_dns2 = $(_obj).find("#pptpd_dns2");
		var $pptpd_winsenable_x = $(_obj).find("#pptpd_winsenable_x");
		var $pptpd_wins1 = $(_obj).find("#pptpd_wins1");
		var $pptpd_wins2 = $(_obj).find("#pptpd_wins2");
		var $pptpd_mru = $(_obj).find("#pptpd_mru");
		var $pptpd_mtu = $(_obj).find("#pptpd_mtu");
		var $pptpd_clients_start = $(_obj).find("#pptpd_clients_start");
		var $pptpd_clients_end = $(_obj).find("#pptpd_clients_end");

		if($pptpd_dnsenable_x.hasClass("off")){
			if($pptpd_dns1.val() == "")
				return false;
			if($pptpd_dns2.val() == "")
				return false;
		}
		if($pptpd_winsenable_x.hasClass("off")){
			if($pptpd_wins1.val() == "")
				return false;
			if($pptpd_wins2.val() == "")
				return false;
		}
		if($pptpd_mru.val() == "")
			return false;
		if($pptpd_mtu.val() == "")
			return false;
		if($pptpd_clients_start.val() == "")
			return false;
		if($pptpd_clients_end.val() == "")
			return false;
	}
	else if(_validField == "pptp_clientlist"){
		var $pptpd_clientlist_username = $(_obj).find("#pptpd_clientlist_username");
		var $pptpd_clientlist_password = $(_obj).find("#pptpd_clientlist_password");
		if($pptpd_clientlist_username.val() == "")
			return false;
		if($pptpd_clientlist_password.val() == "")
			return false;
	}

	return true;
}
function validate_format_PPTP(_obj, _validField){
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
	var valid_pptpd_clients_start_than_end = function(clients_start, clients_end){
		var testResult = {
			'isError': false,
			'errReason': ''
		};
		var clients_start_ip = parseInt(clients_start.split(".")[3]);
		var clients_end_ip = parseInt(clients_end);
		if(clients_start_ip > clients_end_ip){
			testResult.isError = true;
			testResult.errReason = "<#vlaue_haigher_than#> " + clients_start;
		}
		return testResult;
	};
	var valid_pptpd_clients_max = function(clients_start, clients_end){
		var testResult = {
			'isError': false,
			'errReason': ''
		};
		var clients_start_ip = parseInt(clients_start.split(".")[3]);
		var clients_end_ip = parseInt(clients_end);
		if((clients_end_ip - clients_start_ip) > max_shift){
			testResult.isError = true;
			testResult.errReason = "<#vpn_max_clients#>";
		}
		return testResult;
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

	if(_validField == "pptp_profile"){
		var $pptpd_dnsenable_x = $(_obj).find("#pptpd_dnsenable_x");
		var $pptpd_dns1 = $(_obj).find("#pptpd_dns1");
		$pptpd_dns1.val($pptpd_dns1.val().replace(/\s+/g, ''));//remove space
		var $pptpd_dns2 = $(_obj).find("#pptpd_dns2");
		$pptpd_dns2.val($pptpd_dns2.val().replace(/\s+/g, ''));//remove space
		var $pptpd_winsenable_x = $(_obj).find("#pptpd_winsenable_x");
		var $pptpd_wins1 = $(_obj).find("#pptpd_wins1");
		$pptpd_wins1.val($pptpd_wins1.val().replace(/\s+/g, ''));//remove space
		var $pptpd_wins2 = $(_obj).find("#pptpd_wins2");
		$pptpd_wins2.val($pptpd_wins2.val().replace(/\s+/g, ''));//remove space
		var $pptpd_mru = $(_obj).find("#pptpd_mru");
		$pptpd_mru.val($pptpd_mru.val().replace(/\s+/g, ''));//remove space
		var $pptpd_mtu = $(_obj).find("#pptpd_mtu");
		$pptpd_mtu.val($pptpd_mtu.val().replace(/\s+/g, ''));//remove space
		var $pptpd_clients_start = $(_obj).find("#pptpd_clients_start");
		$pptpd_clients_start.val($pptpd_clients_start.val().replace(/\s+/g, ''));//remove space
		var $pptpd_clients_end = $(_obj).find("#pptpd_clients_end");
		$pptpd_clients_end.val($pptpd_clients_end.val().replace(/\s+/g, ''));//remove space

		if($pptpd_dnsenable_x.hasClass("off")){
			if($pptpd_dns1.val() == ""){
				$pptpd_dns1.show_validate_hint("<#JS_fieldblank#>");
				$pptpd_dns1.focus();
				return false;
			}
			var isValid_pptpd_dns1 = valid_is_IP_format($pptpd_dns1.val(), "IPv4");
			if(isValid_pptpd_dns1.isError){
				$pptpd_dns1.show_validate_hint(isValid_pptpd_dns1.errReason);
				$pptpd_dns1.focus();
				return false;
			}
			if($pptpd_dns2.val() == ""){
				$pptpd_dns2.show_validate_hint("<#JS_fieldblank#>");
				$pptpd_dns2.focus();
				return false;
			}
			var isValid_pptpd_dns2 = valid_is_IP_format($pptpd_dns2.val(), "IPv4");
			if(isValid_pptpd_dns2.isError){
				$pptpd_dns2.show_validate_hint(isValid_pptpd_dns2.errReason);
				$pptpd_dns2.focus();
				return false;
			}
		}
		if($pptpd_winsenable_x.hasClass("off")){
			if($pptpd_wins1.val() == ""){
				$pptpd_wins1.show_validate_hint("<#JS_fieldblank#>");
				$pptpd_wins1.focus();
				return false;
			}
			var isValid_pptpd_wins1 = valid_is_IP_format($pptpd_wins1.val(), "IPv4");
			if(isValid_pptpd_wins1.isError){
				$pptpd_wins1.show_validate_hint(isValid_pptpd_wins1.errReason);
				$pptpd_wins1.focus();
				return false;
			}
			if($pptpd_wins2.val() == ""){
				$pptpd_wins2.show_validate_hint("<#JS_fieldblank#>");
				$pptpd_wins2.focus();
				return false;
			}
			var isValid_pptpd_wins2 = valid_is_IP_format($pptpd_wins2.val(), "IPv4");
			if(isValid_pptpd_wins2.isError){
				$pptpd_wins2.show_validate_hint(isValid_pptpd_wins2.errReason);
				$pptpd_wins2.focus();
				return false;
			}
		}
		$pptpd_mru.val($pptpd_mru.val().trim());
		if($pptpd_mru.val() == ""){
			$pptpd_mru.show_validate_hint("<#JS_fieldblank#>");
			$pptpd_mru.focus();
			return false;
		}
		var isValid_pptpd_mru = valid_num_range($pptpd_mru.val(), 576, 1492);
		if(isValid_pptpd_mru.isError){
			$pptpd_mru.show_validate_hint(isValid_pptpd_mru.errReason);
			$pptpd_mru.focus();
			return false;
		}
		$pptpd_mtu.val($pptpd_mtu.val().trim());
		if($pptpd_mtu.val() == ""){
			$pptpd_mtu.show_validate_hint("<#JS_fieldblank#>");
			$pptpd_mtu.focus();
			return false;
		}
		var isValid_pptpd_mtu = valid_num_range($pptpd_mtu.val(), 576, 1492);
		if(isValid_pptpd_mtu.isError){
			$pptpd_mtu.show_validate_hint(isValid_pptpd_mtu.errReason);
			$pptpd_mtu.focus();
			return false;
		}
		$pptpd_clients_start.val($pptpd_clients_start.val().trim());
		var isValid_pptpd_clients_start = valid_isLegalIP($pptpd_clients_start.val());
		if(isValid_pptpd_clients_start.isError){
			$pptpd_clients_start.show_validate_hint(isValid_pptpd_clients_start.errReason);
			$pptpd_clients_start.focus();
			return false;
		}
		if($pptpd_clients_start.val().split(".")[3] == 255){
			$pptpd_clients_start.show_validate_hint($pptpd_clients_start.val() + " <#JS_validip#>");
			$pptpd_clients_start.focus();
			return false;
		}
		$pptpd_clients_end.val($pptpd_clients_end.val().trim());
		if($pptpd_clients_end.val() == ""){
			$pptpd_clients_end.show_validate_hint("<#JS_fieldblank#>");
			$pptpd_clients_end.focus();
			return false;
		}
		var isValid_pptpd_clients_end = valid_num_range($pptpd_clients_end.val(), 1, 254);
		if(isValid_pptpd_clients_end.isError){
			$pptpd_clients_end.show_validate_hint(isValid_pptpd_clients_end.errReason);
			$pptpd_clients_end.focus();
			return false;
		}
		var isValid_pptpd_clients_start_than_end = valid_pptpd_clients_start_than_end($pptpd_clients_start.val(), $pptpd_clients_end.val());
		if(isValid_pptpd_clients_start_than_end.isError){
			$pptpd_clients_end.show_validate_hint(isValid_pptpd_clients_start_than_end.errReason);
			$pptpd_clients_end.focus();
			set_pptp_client_ip_end(_obj);
			return false;
		}
		var isValid_pptpd_clients_max = valid_pptpd_clients_max($pptpd_clients_start.val(), $pptpd_clients_end.val());
		if(isValid_pptpd_clients_max.isError){
			$pptpd_clients_start.show_validate_hint(isValid_pptpd_clients_max.errReason);
			$pptpd_clients_start.focus();
			set_pptp_client_ip_end(_obj);
			return false;
		}
	}
	else if(_validField == "pptp_clientlist" || _validField == "pptp_client_static_route"){
		if(_validField == "pptp_clientlist"){
			var $pptpd_clientlist_username = $(_obj).find("#pptpd_clientlist_username");
			$pptpd_clientlist_username.val($pptpd_clientlist_username.val().replace(/\s+/g, ''));//remove space
			if($pptpd_clientlist_username.val() == ""){
				$pptpd_clientlist_username.show_validate_hint("<#JS_fieldblank#>");
				$pptpd_clientlist_username.focus();
				return false;
			}
			var isValid_username = valid_block_chars($pptpd_clientlist_username.val(), [" ", "@", "*", "+", "|", ":", "?", "<", ">", ",", ".", "/", ";", "[", "]", "\\", "=", "\"", "&" ]);
			if(isValid_username.isError){
				$pptpd_clientlist_username.show_validate_hint(isValid_username.errReason);
				$pptpd_clientlist_username.focus();
				return false;
			}

			var duplicateCheck = pptpd_clientlist_data.filter(function(item, index, array){
				return (item.username == $pptpd_clientlist_username.val());
			})[0];
			if(duplicateCheck != undefined){
				show_customize_alert("<#JS_duplicate#>");
				return false;
			}

			var $pptpd_clientlist_password = $(_obj).find("#pptpd_clientlist_password");
			$pptpd_clientlist_password.val($pptpd_clientlist_password.val().replace(/\s+/g, ''));//remove space
			if($pptpd_clientlist_password.val() == ""){
				$pptpd_clientlist_password.show_validate_hint("<#JS_fieldblank#>");
				$pptpd_clientlist_password.focus();
				return false;
			}
			var isValid_password = valid_block_chars($pptpd_clientlist_password.val(), ["<", ">", "&"]);
			if(isValid_password.isError){
				$pptpd_clientlist_password.show_validate_hint(isValid_password.errReason);
				$pptpd_clientlist_password.focus();
				return false;
			}
		}

		var $pptpd_sr_ipaddr = $(_obj).find("#pptpd_sr_ipaddr");
		$pptpd_sr_ipaddr.val($pptpd_sr_ipaddr.val().replace(/\s+/g, ''));//remove space
		var $pptpd_sr_netmask = $(_obj).find("#pptpd_sr_netmask");
		$pptpd_sr_netmask.val($pptpd_sr_netmask.val().replace(/\s+/g, ''));//remove space
		if($pptpd_sr_ipaddr.val() != "" || $pptpd_sr_netmask.val() != ""){
			var isValid_pptpd_sr_ipaddr = valid_isLegalIP($pptpd_sr_ipaddr.val());
			if(isValid_pptpd_sr_ipaddr.isError){
				$pptpd_sr_ipaddr.show_validate_hint(isValid_pptpd_sr_ipaddr.errReason);
				$pptpd_sr_ipaddr.focus();
				return false;
			}

			var isValid_pptpd_sr_netmask = valid_isLegalMask($pptpd_sr_netmask.val());
			if(isValid_pptpd_sr_netmask.isError){
				$pptpd_sr_netmask.show_validate_hint(isValid_pptpd_sr_netmask.errReason);
				$pptpd_sr_netmask.focus();
				return false;
			}

			var lan_ipaddr = httpApi.nvramGet(["lan_ipaddr"]).lan_ipaddr;
			if($pptpd_sr_ipaddr.val() == lan_ipaddr){
				$pptpd_sr_ipaddr.show_validate_hint("<#vpn_conflict_LANIP#> " + lan_ipaddr);
				$pptpd_sr_ipaddr.focus();
				return false;
			}
		}
	}

	return true;
}
function Update_Profile_Data_PPTP(_obj){
	var settings = httpApi.nvramGet(["pptpd_ms_network", "pptpd_broadcast", "pptpd_chap", "pptpd_mppe", "pptpd_dns1", "pptpd_dns2",
		"pptpd_wins1", "pptpd_wins2", "pptpd_mru", "pptpd_mtu", "pptpd_clients", "pptpd_clientlist", "pptpd_sr_rulelist"]);

	$(_obj).find("#pptpd_ms_network").removeClass("off on").addClass((function(){
		return ((settings.pptpd_ms_network == "1") ? "on" : "off");
	})());

	var $pptpd_broadcast_obj = $(_obj).find("#pptpd_broadcast").removeClass("off on").addClass((function(){
		if(settings.pptpd_ms_network == "1")
			return "on";
		else
			return ((settings.pptpd_broadcast == "1") ? "on" : "off");
	})()).removeAttr("temp_disable");
	if(settings.pptpd_ms_network == "1"){
		$(_obj).find("#PPTP_broadcast_hint").css("display", "flex");
		$pptpd_broadcast_obj.attr("temp_disable", "disabled");
	}

	set_value_Custom_Select(_obj, "pptpd_chap", settings.pptpd_chap);

	var pptpd_mppe = parseInt(settings.pptpd_mppe);
	if(isNaN(pptpd_mppe) || pptpd_mppe == 0)
		pptpd_mppe = (1 | 4 | 8);
	$(_obj).find("#pptpd_mppe .cb_option_container").each(function(){
		$(this).removeClass("off on");
		var cb_value = parseInt($(this).find(".checkbox").attr("value"));
		if(pptpd_mppe & cb_value){
			$(this).find(".checkbox").addClass("on");
		}
		else{
			$(this).find(".checkbox").addClass("off");
		}
	});

	$(_obj).find("#pptpd_dns1").val(htmlEnDeCode.htmlEncode(settings.pptpd_dns1));
	$(_obj).find("#pptpd_dns2").val(htmlEnDeCode.htmlEncode(settings.pptpd_dns2));
	var $pptpd_dns_content_container = $(_obj).find("[slide_target='pptpd_dns']");
	var $pptpd_dnsenable_x_obj = $(_obj).find("#pptpd_dnsenable_x").removeClass("off on");
	if(settings.pptpd_dns1 == "" && settings.pptpd_dns2 == ""){
		$pptpd_dnsenable_x_obj.addClass("on");
		$pptpd_dns_content_container.hide();
	}
	else{
		$pptpd_dnsenable_x_obj.addClass("off");
		$pptpd_dns_content_container.show();
	}

	$(_obj).find("#pptpd_wins1").val(htmlEnDeCode.htmlEncode(settings.pptpd_wins1));
	$(_obj).find("#pptpd_wins2").val(htmlEnDeCode.htmlEncode(settings.pptpd_wins2));
	var $pptpd_wins_content_container = $(_obj).find("[slide_target='pptpd_wins']");
	var $pptpd_winsenable_x_obj = $(_obj).find("#pptpd_winsenable_x").removeClass("off on");
	if(settings.pptpd_wins1 == "" && settings.pptpd_wins2 == ""){
		$pptpd_winsenable_x_obj.addClass("on");
		$pptpd_wins_content_container.hide();
	}
	else{
		$pptpd_winsenable_x_obj.addClass("off");
		$pptpd_wins_content_container.show();
	}

	$(_obj).find("#pptpd_mru").val(htmlEnDeCode.htmlEncode(settings.pptpd_mru));
	$(_obj).find("#pptpd_mtu").val(htmlEnDeCode.htmlEncode(settings.pptpd_mtu));

	var check_vpn_conflict = function(){ //if conflict with LAN ip & DHCP ip pool & static
		var result = "";
		var used_ip_pool = httpApi.nvramGet(["dhcp_start", "dhcp_end", "lan_ipaddr", "dhcp_staticlist"]);
		var pool_start = used_ip_pool.dhcp_start;
		var pool_end = used_ip_pool.dhcp_end
		var pool_subnet = pool_start.split(".")[0]+"."+pool_start.split(".")[1]+"."+pool_start.split(".")[2]+".";
		var pool_start_end = parseInt(pool_start.split(".")[3]);
		var pool_end_end = parseInt(pool_end.split(".")[3]);
		var origin_lan_ip = used_ip_pool.lan_ipaddr;
		var lan_ip_subnet = origin_lan_ip.split(".")[0]+"."+origin_lan_ip.split(".")[1]+"."+origin_lan_ip.split(".")[2]+".";
		var lan_ip_end = parseInt(origin_lan_ip.split(".")[3]);
		var dhcp_staticlists = used_ip_pool.dhcp_staticlist.replace(/&#62/g, ">").replace(/&#60/g, "<");
		var staticclist_row = dhcp_staticlists.split('<');
		var pptpd_clients_array = $pptpd_client_ip_start.val().split(".");
		var pptpd_clients_subnet = pptpd_clients_array[0] + "." + pptpd_clients_array[1] + "." + pptpd_clients_array[2] + ".";
		var pptpd_clients_start_ip = parseInt(pptpd_clients_array[3]);
		var pptpd_clients_end_ip = parseInt($pptpd_client_ip_end.val());

		//LAN ip
		if( lan_ip_subnet == pptpd_clients_subnet 
			&& lan_ip_end >= pptpd_clients_start_ip
			&& lan_ip_end <= pptpd_clients_end_ip ){
			result = "<#vpn_conflict_LANIP#> <b>"+origin_lan_ip+"</b>";
		}
		else if( pool_subnet == pptpd_clients_subnet 
			&& ( (pool_start_end >= pptpd_clients_start_ip && pool_start_end <= pptpd_clients_end_ip) 
				|| (pool_end_end >= pptpd_clients_start_ip && pool_end_end <= pptpd_clients_end_ip)
				|| (pptpd_clients_start_ip >= pool_start_end && pptpd_clients_start_ip <= pool_end_end)
				|| (pptpd_clients_end_ip >= pool_start_end && pptpd_clients_end_ip <= pool_end_end))){//DHCP pool
			result = "<#vpn_conflict_DHCPpool#> <b>"+pool_start+" ~ "+pool_end+"</b>";
		}
		else if(dhcp_staticlists != ""){//DHCP static IP
			for(var i = 1; i < staticclist_row.length; i++){
				var static_subnet = "";
				var static_end = "";
				var static_ip = staticclist_row[i].split('>')[1];
				static_subnet = static_ip.split(".")[0] + "." + static_ip.split(".")[1] + "." + static_ip.split(".")[2] + ".";
				static_end = parseInt(static_ip.split(".")[3]);
				if( static_subnet == pptpd_clients_subnet 
					&& static_end >= pptpd_clients_start_ip 
					&& static_end <= pptpd_clients_end_ip){
					result = "<#vpn_conflict_DHCPstatic#> <b>"+static_ip+"</b>";
				}
			}
		}
		return result;
	};
	var $pptpd_client_ip_items = $(_obj).find("#pptpd_client_ip");
	var $pptpd_client_ip_start = $pptpd_client_ip_items.find("#pptpd_clients_start");
	var $pptpd_client_ip_end = $pptpd_client_ip_items.find("#pptpd_clients_end");
	if(settings.pptpd_clients != "") {
		var pptpd_clients_start = settings.pptpd_clients.split("-")[0];
		var pptpd_clients_end = settings.pptpd_clients.split("-")[1];
		$pptpd_client_ip_start.val(htmlEnDeCode.htmlEncode(pptpd_clients_start));
		$pptpd_client_ip_end.val(htmlEnDeCode.htmlEncode(pptpd_clients_end));
	}
	var vpn_conflict_text = check_vpn_conflict();
	if(vpn_conflict_text != "")
		$pptpd_client_ip_items.append($("<div>").addClass("item_hint").html(vpn_conflict_text));

	var pptpd_clientlist = (settings.pptpd_clientlist).replace(/&#62/g, ">").replace(/&#60/g, "<");
	var pptpd_clientlist_array = pptpd_clientlist.split("<");
	$.each(pptpd_clientlist_array, function(index, value){
		if(value != ""){
			var client_data = value.split(">");
			var client_profile = new pptpd_clientlist_attr();
			client_profile.username = client_data[0];
			client_profile.password = client_data[1];
			pptpd_clientlist_data.push(JSON.parse(JSON.stringify(client_profile)));
		}
	});
	var pptpd_sr_rulelist = (settings.pptpd_sr_rulelist).replace(/&#62/g, ">").replace(/&#60/g, "<");
	var pptpd_sr_rulelist_array = pptpd_sr_rulelist.split("<");
	$.each(pptpd_sr_rulelist_array, function(index, value){
		if(value != ""){
			var sr_data = value.split(">");
			var specific_data = pptpd_clientlist_data.filter(function(item, index, array){
				return (item.username == sr_data[0]);
			})[0];
			if(specific_data != undefined){
				specific_data.hostIP = sr_data[1];
				specific_data.netmask = sr_data[2];
			}
		}
	});
	show_pptpd_clientlist($(_obj).find("#pptpd_clientlist_bg"));
}
function set_apply_btn_status_PPTP(_obj){
	var $btn_container_apply = $(_obj).find(".action_container .btn_container.apply");
	var validate_blank_flag = validate_blank_PPTP($(_obj), "pptp_profile");
	if(!validate_blank_flag)
		$btn_container_apply.removeClass("valid_fail").addClass("valid_fail").unbind("click");
	else{
		$btn_container_apply.removeClass("valid_fail").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			if(validate_format_PPTP($(_obj), "pptp_profile")){
				var nvramSet_obj = {"action_mode": "apply", "rc_service": ""};
				var pptpd_enable = httpApi.nvramGet(["pptpd_enable"]).pptpd_enable;
				var enable_samba = httpApi.nvramGet(["enable_samba"]).enable_samba;
				if(pptpd_enable == "1")
					nvramSet_obj.rc_service = "restart_vpnd;";
				else
					nvramSet_obj.rc_service = "stop_vpnd;";
				if(enable_samba == "1")
					nvramSet_obj.rc_service += "restart_samba;";

				var $pptpd_ms_network = $(_obj).find("#pptpd_ms_network");
				if($pptpd_ms_network.hasClass("on"))
					nvramSet_obj.pptpd_ms_network = "1";
				else
					nvramSet_obj.pptpd_ms_network = "0";
				var $pptpd_broadcast = $(_obj).find("#pptpd_broadcast");
				if($pptpd_broadcast.hasClass("on"))
					nvramSet_obj.pptpd_broadcast = "1";
				else
					nvramSet_obj.pptpd_broadcast = "0";
				nvramSet_obj.pptpd_chap = $(_obj).find("#select_pptpd_chap").children(".selected").attr("value");
				var $pptpd_mppe = $(_obj).find("#pptpd_mppe");
				nvramSet_obj.pptpd_mppe = 0;
				$pptpd_mppe.find(".cb_option_container").each(function(){
					var $cb_option = $(this);
					if($cb_option.find(".checkbox").hasClass("on")){
						var cb_value = parseInt($cb_option.find(".checkbox").attr("value"));
						nvramSet_obj.pptpd_mppe |= cb_value;
					}
				});
				var $pptpd_dnsenable_x = $(_obj).find("#pptpd_dnsenable_x");
				if($pptpd_dnsenable_x.hasClass("on")){
					nvramSet_obj.pptpd_dns1 = "";
					nvramSet_obj.pptpd_dns2 = "";
				}
				else{
					nvramSet_obj.pptpd_dns1 = $(_obj).find("#pptpd_dns1").val();
					nvramSet_obj.pptpd_dns2 =  $(_obj).find("#pptpd_dns2").val();
				}
				var $pptpd_winsenable_x = $(_obj).find("#pptpd_winsenable_x");
				if($pptpd_winsenable_x.hasClass("on")){
					nvramSet_obj.pptpd_wins1 = "";
					nvramSet_obj.pptpd_wins2 = "";
				}
				else{
					nvramSet_obj.pptpd_wins1 = $(_obj).find("#pptpd_wins1").val();
					nvramSet_obj.pptpd_wins2 = $(_obj).find("#pptpd_wins2").val();
				}
				nvramSet_obj.pptpd_mru = $(_obj).find("#pptpd_mru").val();
				nvramSet_obj.pptpd_mtu = $(_obj).find("#pptpd_mtu").val();
				var pptpd_clients = $(_obj).find("#pptpd_clients_start").val() + "-" + $(_obj).find("#pptpd_clients_end").val();
				nvramSet_obj.pptpd_clients = pptpd_clients;

				var pptpd_clientlist = "";
				var pptpd_sr_rulelist = "";
				$.each(pptpd_clientlist_data, function(index, value){
					pptpd_clientlist += "<" + value.username + ">" + value.password;
					if(value.hostIP != "" && value.netmask != ""){
						pptpd_sr_rulelist += "<" + value.username + ">" + value.hostIP + ">" + value.netmask;
					}
				});
				nvramSet_obj.pptpd_clientlist = pptpd_clientlist;
				nvramSet_obj.pptpd_sr_rulelist = pptpd_sr_rulelist;

				httpApi.nvramSet(nvramSet_obj, function(){
					var time = 10;
					close_popup();
					showLoading(time);
					setTimeout(function(){
						httpApi.nvramGet(["pptpd_ms_network", "pptpd_broadcast", "pptpd_chap", "pptpd_mppe", "pptpd_dns1", "pptpd_dns2",
						"pptpd_wins1", "pptpd_wins2", "pptpd_mru", "pptpd_mtu", "pptpd_clients", "pptpd_clientlist", "pptpd_sr_rulelist"], true);
						if(!window.matchMedia('(max-width: 575px)').matches)
							$("#srv_profile_list").children("[type='" + select_vpn_type + "']").addClass("selected").find(".svr_item_text_container").click();
					}, time*1000);
				}());
			}
		});
	}
}
function Get_Component_Setting_Profile_PPTP(_type){
	var $container = $("<div>").addClass("popup_edit_profile_container");

	if(_type == "popup")
		$container.append(Get_Component_Popup_Profile_Title("PPTP"));
	else
		$container.append(Get_Component_Profile_Title("PPTP"));

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
	var pptpd_ms_network_parm = {"title":"<#vpn_network_place#>", "type":"switch", "id":"pptpd_ms_network"};
	Get_Component_Switch(pptpd_ms_network_parm).appendTo($detail_general)
		.find("#" + pptpd_ms_network_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			var $pptpd_broadcast = $(this).closest(".profile_setting").find("#pptpd_broadcast");
			var $PPTP_broadcast_hint = $(this).closest(".profile_setting").find("#PPTP_broadcast_hint");
			if($(this).hasClass("on")){
				$pptpd_broadcast.attr("temp_disable", "disabled").removeClass("off on").addClass("on");
				$PPTP_broadcast_hint.css("display", "flex");
			}
			else{
				$pptpd_broadcast.removeAttr("temp_disable");
				$PPTP_broadcast_hint.hide();
			}
		});

	var help_parm = {"title":"<#HOWTOSETUP#>"};
	Get_Component_Help(help_parm)
		.appendTo($detail_general)
		.find(".vpnc_help_icon").unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			show_popup_help_PPTP("Feature_Desc");
		});

	var $detail_adv = $("<div>").attr("detail_mode","2").appendTo($content_container);
	var pptpd_broadcast_parm = {"title":"<#vpn_broadcast#>", "type":"switch", "id":"pptpd_broadcast"};
	Get_Component_Switch(pptpd_broadcast_parm).appendTo($detail_adv);
	$("<div>").attr({"id": "PPTP_broadcast_hint"}).addClass("item_hint").html("* <#PPTP_broadcast_hint#>").appendTo($detail_adv).hide();

	var pptpd_chap_options = [{"text":"<#Auto#>","value":"0"},{"text":"MS-CHAPv1","value":"1"},{"text":"MS-CHAPv2","value":"2"}];
	var pptpd_chap_parm = {"title": "<#PPPConnection_Authentication_itemname#>", "id": "pptpd_chap", "options": pptpd_chap_options, "set_value": "0"};
	Get_Component_Custom_Select(pptpd_chap_parm).appendTo($detail_adv);

	var pptpd_mppe_options = [{"text":"MPPE-128","value":"1"},{"text":"MPPE-40","value":"4"},{"text":"<#No_Encryp#>","value":"8"}];
	var pptpd_mppe_parm = {"title": "<#MPPE_Encryp#>", "id": "pptpd_mppe", "options": pptpd_mppe_options};
	Get_Component_Checkbox(pptpd_mppe_parm).appendTo($detail_adv)
		.find("#" + pptpd_mppe_parm.id + " .cb_option_container").each(function(){
			$(this).unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				if($(this).find(".checkbox").hasClass("disable"))
					return;
				$(this).find(".checkbox").toggleClass("off on");
				var cb_group = $(this).attr("cb_group");
				if(cb_group == "pptpd_mppe"){
					var $pptpd_mppe_list =  $(this).closest(".input_container").find(".cb_option_container");
					var selected_list = $pptpd_mppe_list.find(".checkbox.on");
					if(selected_list.length == 0){
						$pptpd_mppe_list.find(".checkbox[value=8]").toggleClass("off on");
					}
				}
			});
		});

	var pptpd_dnsenable_x_parm = {"title":"<#IPConnection_x_DNSServerEnable_itemname#>", "type":"switch", "id":"pptpd_dnsenable_x", "slide_target":"pptpd_dns"};
	var $pptpd_dnsenable_x_obj = Get_Component_Switch(pptpd_dnsenable_x_parm).appendTo($detail_adv)
		.find("#" + pptpd_dnsenable_x_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			var $slide_target = $(this).closest(".profile_setting").find("[slide_target='" + pptpd_dnsenable_x_parm.slide_target + "']");
			if($(this).hasClass("on")){
				$slide_target.hide();
			}
			else{
				$slide_target.show();
			}
			resize_iframe_height();
			set_apply_btn_status_PPTP($(this).closest(".profile_setting"));
		});

	var $pptpd_dns_content_container = $("<div>").attr({"slide_target":"pptpd_dns"}).appendTo($detail_adv);
	var pptpd_dns1_parm = {"title":"<#IPConnection_x_DNSServer1_itemname#>", "type":"text", "id":"pptpd_dns1", "need_check":true, "maxlength":15};
	Get_Component_Input(pptpd_dns1_parm).appendTo($pptpd_dns_content_container)
		.find("#" + pptpd_dns1_parm.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isIPAddr(this, event);
		});

	var pptpd_dns2_parm = {"title":"<#IPConnection_x_DNSServer2_itemname#>", "type":"text", "id":"pptpd_dns2", "need_check":true, "maxlength":15};
	Get_Component_Input(pptpd_dns2_parm).appendTo($pptpd_dns_content_container)
		.find("#" + pptpd_dns2_parm.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isIPAddr(this, event);
		});

	var pptpd_winsenable_x_parm = {"title":"<#IPConnection_x_WINSServerEnable_itemname#>", "type":"switch", "id":"pptpd_winsenable_x", "slide_target":"pptpd_wins"};
	Get_Component_Switch(pptpd_winsenable_x_parm).appendTo($detail_adv)
		.find("#" + pptpd_winsenable_x_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			var $slide_target = $(this).closest(".profile_setting").find("[slide_target='"+pptpd_winsenable_x_parm.slide_target+"']");
			if($(this).hasClass("on")){
				$slide_target.hide();
			}
			else{
				$slide_target.show();
			}
			resize_iframe_height();
			set_apply_btn_status_PPTP($(this).closest(".profile_setting"));
		});

	var $pptpd_wins_content_container = $("<div>").attr({"slide_target":"pptpd_wins"}).appendTo($detail_adv);
	var pptpd_wins1_parm = {"title":"<#IPConnection_x_WINSServer1_itemname#>", "type":"text", "id":"pptpd_wins1", "need_check":true, "maxlength":15};
	Get_Component_Input(pptpd_wins1_parm).appendTo($pptpd_wins_content_container)
		.find("#" + pptpd_wins1_parm.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isIPAddr(this, event);
		});

	var pptpd_wins2_parm = {"title":"<#IPConnection_x_WINSServer2_itemname#>", "type":"text", "id":"pptpd_wins2", "need_check":true, "maxlength":15};
	Get_Component_Input(pptpd_wins2_parm).appendTo($pptpd_wins_content_container)
		.find("#" + pptpd_wins2_parm.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isIPAddr(this, event);
		});

	var pptpd_mru_parm = {"title":"MRU", "type":"text", "id":"pptpd_mru", "need_check":true, "maxlength":4};
	Get_Component_Input(pptpd_mru_parm).appendTo($detail_adv)
		.find("#" + pptpd_mru_parm.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isNumber(this,event);
		});

	var pptpd_mtu_parm = {"title":"MTU", "type":"text", "id":"pptpd_mtu", "need_check":true, "maxlength":4};
	Get_Component_Input(pptpd_mtu_parm).appendTo($detail_adv)
		.find("#" + pptpd_mtu_parm.id + "")
		.unbind("keypress").keypress(function(){
			return validator.isNumber(this,event);
		});

	var pptpd_clients_parm = {"title":"<#vpn_client_ip#>", "type":"text", "start_id":"pptpd_clients_start", "start_maxlength":15, "end_id":"pptpd_clients_end", "end_maxlength":3,
		"need_check":true, "container_id":"pptpd_client_ip"};
	var $pptpd_client_ip_items = Get_Component_PPTP_Client_IP(pptpd_clients_parm).appendTo($detail_adv);
	var $pptpd_client_ip_start = $pptpd_client_ip_items.find("#" + pptpd_clients_parm.start_id + "")
		.unbind("keypress").keypress(function(){
			return validator.isIPAddr(this, event);
		});

	$("<div>").addClass("profile_title_item").append($("<span>").html("VPN authentication for VPN clients")).appendTo($detail_general);/* untranslated */

	pptpd_clientlist_data = [];
	var $Add_Client_Title_obj = Get_Component_Add_Client_Title().appendTo($detail_general);
	$Add_Client_Title_obj.find("#vpns_clientlist_title").html("<#vpnc_title#> (<#List_limit#>" + htmlEnDeCode.htmlEncode(pptpd_clientlist_limit) + ")");
	$Add_Client_Title_obj.find("#vpns_clientlist_num").html(htmlEnDeCode.htmlEncode(pptpd_clientlist_data.length));
	$Add_Client_Title_obj.find("#vpns_clientlist_add").unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		if(pptpd_clientlist_data.length >= pptpd_clientlist_limit){
			show_customize_alert("<#weekSche_MAX_Num#>".replace("#MAXNUM", pptpd_clientlist_limit));
			return false;
		}
		show_popup_Add_Client_PPTP();
	});

	var $pptpd_clientlist_bg = $("<div>").attr({"id":"pptpd_clientlist_bg"}).addClass("client_list_content_container").appendTo($detail_general);
	show_pptpd_clientlist($pptpd_clientlist_bg);

	var $action_container = $("<div>").addClass("action_container").appendTo($content_container);
	var $btn_container_apply = $("<div>").addClass("btn_container apply").appendTo($action_container).html("<#CTL_apply1#>");
	var $action_loading_container = $("<div>").addClass("action_container loading").appendTo($content_container);

	$content_container.find("[detail_mode]").hide();
	$content_container.find("[detail_mode='1']").show();

	$content_container.find("[need_check=true]").keyup(function(e){
		e = e || event;
		e.stopPropagation();
		set_apply_btn_status_PPTP($content_container);
	});
	$pptpd_client_ip_start.blur(function(e){
		e = e || event;
		e.stopPropagation();
		set_pptp_client_ip_end($content_container);
	});

	setTimeout(function(){
		Update_Profile_Data_PPTP($content_container);
		set_apply_btn_status_PPTP($content_container);
		resize_iframe_height();
	},1);

	return $container;
}
