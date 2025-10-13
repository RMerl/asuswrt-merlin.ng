const is_Web_iframe = (($(parent.document).find("#mainMenu").length > 0) || (top.webWrapper)) ? true : false;
//const vpn_title_text = (()=>{return (isSupport("vpn_fusion") ? `<#VPN_Fusion#>` : `<#vpnc_title#>`);})();
const vpn_title_text = "<#vpnc_title#>";
let btnsw_list = [
	{"title":vpn_title_text, "value":"4"},
	{"title":`LED`, "value":"2"},/* untranslated */
	{"title":`WiFi`, "value":"3"},/* untranslated */
	{"title":`${Guest_Network_naming}`, "value":"5"},
	{"title":`<#AiMesh_NodeLocation01#> / Travel`, "value":"1"},/* untranslated */
	{"title":`No Function`, "value":"0"}/* untranslated */
];
//const support_vpn_btn = (isSupport("vpn_fusion") && (isSwMode("RT") || isSwMode("WISP")));
const support_vpn_btn = 0;
if(!support_vpn_btn){
	btnsw_list = btnsw_list.filter(item => item.value != "4")
}
if(!isSupport("mtlancfg")){
	btnsw_list = btnsw_list.filter(item => item.value != "5")
}
if (isSwMode("RP") || isSwMode("MB")) {
	btnsw_list = btnsw_list.filter(item => item.value != "3" && item.value != "5");
}

let nv_btnsw_onoff = httpApi.nvramGet(["btnsw_onoff"]).btnsw_onoff;
let vpnc_profile_list = [];
if(support_vpn_btn){
	Get_VPNC_Profile();

	function Get_VPNC_Profile(){
		const vpnc_profile_attr = function(){
			this.desc = "";
			this.proto = "";
			this.vpnc_idx = "0";
			this.activate = "0";
			this.default_conn = false;
		};
		let vpnc_default_wan = httpApi.nvramGet(["vpnc_default_wan"]).vpnc_default_wan;
		let vpnc_clientlist = decodeURIComponent(httpApi.nvramCharToAscii(["vpnc_clientlist"]).vpnc_clientlist);
		let each_profile = vpnc_clientlist.split("<");
		$.each(each_profile, function(index, value){
			if(value != ""){
				let profile_data = value.split(">");
				let vpnc_profile = new vpnc_profile_attr();
				vpnc_profile.desc = profile_data[0];
				vpnc_profile.proto = profile_data[1];
				if(isSupport("vpn_fusion")){
					vpnc_profile.activate = profile_data[5];
					vpnc_profile.vpnc_idx = profile_data[6];
					vpnc_profile.default_conn = ((profile_data[6] == vpnc_default_wan) ? true : false);
				}
				vpnc_profile_list.push(JSON.parse(JSON.stringify(vpnc_profile)));
			}
		});
	}
}
function show_MFB_list(){
	if(nv_btnsw_onoff == "4" && vpnc_profile_list.length == 0){
		nv_btnsw_onoff = "0";
	}
	let $profile_list_content = $("#profile_list_content").empty();
	$.each(btnsw_list, function(index, item){
		Get_Component_Profile_Item(item).appendTo($profile_list_content);
	});

	let $profile_setting_cntr = $("#profile_setting_container");
	if($profile_setting_cntr.css("display") != "none"){
		 $profile_list_content.children("[data-btnsw-onoff='" + nv_btnsw_onoff + "']").click();
	}

	resize_iframe_height();
}
function Get_Component_Profile_Item(_profile_data){
	let $profile_item_container = $("<div>").addClass("profile_item_container").attr({"data-btnsw-onoff":_profile_data.value}).unbind("click").click(function(e){
		e = e || event;
		e.stopPropagation();
		let $profile_setting_cntr = $(".profile_setting_container");
		if($profile_setting_cntr.css("display") == "none"){
			let $popup_cntr = $(".popup_container.popup_element");
			$(".popup_element").css("display", "flex");
			$(".container").addClass("blur_effect");
			$popup_cntr.empty().append(function(){
				switch(_profile_data.value){
					case "0":
						return Get_Component_BTNSW_NoFun("popup");
						break;
					case "1":
						return Get_Component_BTNSW_OP("popup");
						break;
					case "2":
						return Get_Component_BTNSW_LED("popup");
						break;
					case "3":
						return Get_Component_BTNSW_WiFi("popup");
						break;
					case "4":
						return Get_Component_BTNSW_VPN("popup");
						break;
					case "5":
						return Get_Component_BTNSW_SDN("popup");
						break;
					default:
						break;
				}
			});
			Update_Setting_Profile($popup_cntr, _profile_data);
			adjust_popup_container_top($popup_cntr, 100);
		}
		else{
			$(this).addClass("selected").siblings().removeClass("selected");
			$profile_setting_cntr.empty().append(function(){
				switch(_profile_data.value){
					case "0":
						return Get_Component_BTNSW_NoFun();
						break;
					case "1":
						return Get_Component_BTNSW_OP();
						break;
					case "2":
						return Get_Component_BTNSW_LED();
						break;
					case "3":
						return Get_Component_BTNSW_WiFi();
						break;
					case "4":
						return Get_Component_BTNSW_VPN();
						break;
					case "5":
						return Get_Component_BTNSW_SDN();
						break;
					default:
						break;
				}
			});
			Update_Setting_Profile($profile_setting_cntr, _profile_data);
		}
	});

	let $item_tag_cntr = $("<div>").addClass("item_tag_container").appendTo($profile_item_container);
	if(_profile_data.value == nv_btnsw_onoff){
		$("<div>").addClass("item_tag link").html("<#CTL_Default#>").appendTo($item_tag_cntr);
	}

	let $item_text_container = $("<div>").addClass("item_text_container").appendTo($profile_item_container)
	let $main_info = $("<div>").addClass("main_info").appendTo($item_text_container);
	$("<div>").addClass("main_info_text").html(htmlEnDeCode.htmlEncode(_profile_data.title)).appendTo($main_info);

	return $profile_item_container;
}
function Update_Setting_Profile(_obj, _profile_data){
	$(_obj).find(".setting_content_container").attr("data-btnsw-onoff", _profile_data.value);
	$(_obj).find("#btnsw_onoff").removeClass("off on").addClass((function(){
		if(nv_btnsw_onoff == _profile_data.value)
			return "on";
		else
			return "off";
	})());

	if(_profile_data.value == "4"){//vpn
		let $VPN_Profiles_cntr = $(_obj).find("[data-container=VPN_Profiles]");
		const btnsw_vpn_idx = (httpApi.nvramGet(["btnsw_vpn_idx"]).btnsw_vpn_idx == "") ? "0" : httpApi.nvramGet(["btnsw_vpn_idx"]).btnsw_vpn_idx;
		$VPN_Profiles_cntr.find(".rwd_icon_radio").removeClass("clicked").filter("[data-idx="+btnsw_vpn_idx+"]").addClass("clicked");
		if($VPN_Profiles_cntr.find(".rwd_icon_radio.clicked").length == 0){
			$VPN_Profiles_cntr.find(".rwd_icon_radio[data-vpn-type=vpnc]").first().addClass("clicked");
		}
		$.each(vpnc_profile_list, function(index, item){
			if(item.activate == "0"){
				$VPN_Profiles_cntr.find("[data-idx="+item.vpnc_idx+"]").siblings("[data-component=icon_warning]").show();
			}
		});
		if(nv_btnsw_onoff == "4"){
			$VPN_Profiles_cntr.show();
		}
		else{
			$VPN_Profiles_cntr.hide();
		}
	}
	if(_profile_data.value == "5"){//sdn
		let $SDN_Profiles_cntr = $(_obj).find("[data-container=SDN_Profiles]");
		const btnsw_sdn_idx = (httpApi.nvramGet(["btnsw_sdn_idx"]).btnsw_sdn_idx == "") ? "0" : httpApi.nvramGet(["btnsw_sdn_idx"]).btnsw_sdn_idx;
		$SDN_Profiles_cntr.find(".rwd_icon_radio").removeClass("clicked").filter("[data-idx="+btnsw_sdn_idx+"]").addClass("clicked");
		if($SDN_Profiles_cntr.find(".rwd_icon_radio.clicked").length == 0){
			$SDN_Profiles_cntr.find(".rwd_icon_radio").first().addClass("clicked");
		}
		const $profile_list =  $SDN_Profiles_cntr.find(".profile_item");
		$.each($profile_list, function(){
			if($(this).attr("data-sdn_enable") == "0"){
				$(this).find("[data-component=icon_warning]").show();
			}
		});
		if(nv_btnsw_onoff == "5"){
			$SDN_Profiles_cntr.show();
		}
		else{
			$SDN_Profiles_cntr.hide();
		}
	}
	if(_profile_data.value == "1"){
		let $config_content_cntr = $(_obj).find("[data-container='config_content_cntr']").empty();
		const get_nvsw = httpApi.hookGet("get_nvsw");
		if(get_nvsw && get_nvsw.profile != undefined){
			const nvsw_mapping = [{"profile_id":0, "text":`<#AiMesh_NodeLocation01#>`, "mode":"home"}, {"profile_id":1, "text":`Travel`, "mode":"travel"}];/* untranslated */
			get_nvsw.profile.forEach((item)=>{
				const specific_nvsw = nvsw_mapping.find(nvsw => nvsw.profile_id == item.profile_id);
				let $mode_profile_cntr = $("<div>").addClass(`mode_profile_container`).appendTo($config_content_cntr);
				$mode_profile_cntr.addClass(()=>{
					if(nv_btnsw_onoff == "1"){
						if(get_nvsw.cur_id == item.profile_id){
							return `nvsw_active`;
						}
					}
				});
				$("<div>").addClass(`icon_component ${(specific_nvsw) ? specific_nvsw.mode : ``}`).append("<i>").appendTo($mode_profile_cntr);
				let $detail_comp = $("<div>").addClass("detail_component").appendTo($mode_profile_cntr);
				let $mode_comp = $("<div>").addClass("text_component").appendTo($detail_comp);
				let main_text = (specific_nvsw) ? specific_nvsw.text : `--`;
				if(nv_btnsw_onoff == "1"){
					if(get_nvsw.cur_id == item.profile_id){
						main_text += ` (<#Status_Active#>)`;
					}
				}
				$("<div>").addClass("main_text").html(main_text).appendTo($mode_comp);
				$("<div>").addClass("sub_text").html(`<#vpnc_profile#>`).appendTo($mode_comp);
				let $profile_comp = $("<div>").addClass("text_component").appendTo($detail_comp);
				$("<div>").addClass("main_text").html(get_mode_text(item)).appendTo($profile_comp);
				$("<div>").addClass("sub_text").html(`<#DSL_Mode#>`).appendTo($profile_comp);
			});

			function get_mode_text(nvsw_info){
				if(nvsw_info.status == "OK"){
					if(nvsw_info.mb == "1") return `<#OP_MB_item#>`;
					else if(nvsw_info.rp == "1") return `<#OP_RE_item#>`;
					else if(nvsw_info.re == "1") return `<#AiMesh_Node#>`;
					else if(nvsw_info.wisp == "1") return `<#OP_WISP_item#>`;
					else if(nvsw_info.sw_mode == "1") return `<#Router_mode#>`;
					else return `--`;
				}
				else{
					return "--";
				}
			}
		}
	}
}
function Get_Component_BTNSW_VPN(view_mode){
	let $container = $("<div>").addClass("setting_content_container no_action_container");

	if(view_mode == "popup"){
		Get_Component_Popup_Profile_Title(vpn_title_text).appendTo($container)
			.find("#title_close_btn").unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				close_popup_container($container);
			});
	}
	else
		Get_Component_Profile_Title(vpn_title_text).appendTo($container).find("#title_del_btn").remove();

	let $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);

	Get_Component_Schematic({"fun_desc":`While switch on, VPN connection will be turn on automatically.`}).appendTo($content_container);

	let btnsw_onoff_parm = {"title":"Multi-Function Button Default", "type":"switch", "id":"btnsw_onoff", "set_value":"off"};
	Get_Component_Switch(btnsw_onoff_parm).appendTo($content_container)
		.find("#" + btnsw_onoff_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			if(vpnc_profile_list.length == 0){
				$(this).toggleClass("off on");
				show_customize_alert(stringSafeGet(`<#GuestNetwork_noProfile_hint#>`));
				return false;
			}
			else{
				let $VPN_Profiles_cntr = $content_container.find("[data-container=VPN_Profiles]");
				if($(this).hasClass("on")){
					$VPN_Profiles_cntr.show();
				}
				else{
					$VPN_Profiles_cntr.hide();
				}
				resize_iframe_height();
				let $selected_vpn = $VPN_Profiles_cntr.find(".rwd_icon_radio.clicked");
				if($selected_vpn.length){
					const vpn_idx = $selected_vpn.attr("data-idx");
					if(vpn_idx != "" && vpn_idx != undefined){
						const this_btnsw_onoff = $(this).closest(".setting_content_container").attr("data-btnsw-onoff");
						let postData = {"btnsw_onoff":this_btnsw_onoff, "switch_status":$(this).hasClass("on")};
						if(postData.switch_status == true){
							postData["btnsw_vpn_idx"] = vpn_idx;
						}
						Set_BTNSW_ONOFF(postData);
					}
					else{
						show_customize_alert(stringSafeGet(`<#GuestNetwork_noProfile_hint#>`));
						return false;
					}
				}
			}
		});

	Get_Component_VPN_Profiles().appendTo($content_container).find(".profile_item").click(function(e){
		e = e || event;
		e.stopPropagation();
		let $setting_content_cntr = $(this).closest(".setting_content_container");
		const this_btnsw_onoff = $setting_content_cntr.attr("data-btnsw-onoff");
		const btnsw_switch = $setting_content_cntr.find("#btnsw_onoff").hasClass("on");
		const vpn_idx = $(this).find(".rwd_icon_radio").attr("data-idx");
		Set_BTNSW_ONOFF({
			"btnsw_onoff": this_btnsw_onoff,
			"switch_status": btnsw_switch,
			"btnsw_vpn_idx": vpn_idx
		});
	});

	let vpnc_btn_parm = {"id":"vpnc_btn", "text":"<#btn_goSetting#>"};
	Get_Component_Btn(vpnc_btn_parm).appendTo($content_container)
	.find("#" + vpnc_btn_parm.id + "").click(function(e){
		e = e || event;
		e.stopPropagation();
		if(is_Web_iframe){
			top.location.href = "/Advanced_VPNClient_Content.asp";
		}
		else if(parent.businessWrapper){
			top.location.href = "/index.html?url=vpnc&current_theme=business";
		}
		else{
			top.location.href = "/VPN/vpnc.html" + ((typeof theme == "string" && theme != "") ? "?current_theme=" + theme + "" : "");
		}
	});

	return $container;
}
function Get_Component_BTNSW_OP(view_mode){
	let $container = $("<div>").addClass("setting_content_container no_action_container");

	if(view_mode == "popup"){
		Get_Component_Popup_Profile_Title(`<#AiMesh_NodeLocation01#> / Travel`).appendTo($container)/* untranslated */
			.find("#title_close_btn").unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				close_popup_container($container);
			});
	}
	else
		Get_Component_Profile_Title(`<#AiMesh_NodeLocation01#> / Travel`).appendTo($container).find("#title_del_btn").remove();/* untranslated */

	let $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);

	Get_Component_Schematic(
		{
			"fun_desc":`Please switch the multi-function button to the designated mode while the power is off, then power on. The router will boot up and enter the designated mode.`,
			"btnsw":"1"
		}/* untranslated */
	).appendTo($content_container);

	let btnsw_onoff_parm = {"title":`Multi-Function Button Default`, "type":"switch", "id":"btnsw_onoff", "set_value":"off"};/* untranslated */
	Get_Component_Switch(btnsw_onoff_parm).appendTo($content_container)
		.find("#" + btnsw_onoff_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			const this_btnsw_onoff = $(this).closest(".setting_content_container").attr("data-btnsw-onoff");
			Set_BTNSW_ONOFF({"btnsw_onoff":this_btnsw_onoff, "switch_status":$(this).hasClass("on")});
		});

	let $mode_config_cntr = $("<div>").addClass("mode_config_container").appendTo($content_container);
	$("<div>").attr({"data-container":"config_content_cntr"}).appendTo($mode_config_cntr);
	let $notice_cntr = $("<div>").addClass("notice_container").appendTo($mode_config_cntr);
	$("<div>").addClass("title").html(`<div class='icon_notice'></div><#InternetSpeed_Notice#>`).appendTo($notice_cntr);
	$("<div>").addClass("desc").html(`Please note factory reset will clean up all mode profiles.`).appendTo($notice_cntr);/* untranslated */

	return $container;
}
function Get_Component_BTNSW_LED(view_mode){
	let $container = $("<div>").addClass("setting_content_container no_action_container");

	if(view_mode == "popup"){
		Get_Component_Popup_Profile_Title(`LED`).appendTo($container)
			.find("#title_close_btn").unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				close_popup_container($container);
			});
	}
	else
		Get_Component_Profile_Title(`LED`).appendTo($container).find("#title_del_btn").remove();

	let $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);

	Get_Component_Schematic({"fun_desc":`Turn on/off router's LED.`}).appendTo($content_container);/* untranslated */

	let btnsw_onoff_parm = {"title":"Multi-Function Button Default", "type":"switch", "id":"btnsw_onoff", "set_value":"off"};
	Get_Component_Switch(btnsw_onoff_parm).appendTo($content_container)
		.find("#" + btnsw_onoff_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			const this_btnsw_onoff = $(this).closest(".setting_content_container").attr("data-btnsw-onoff");
			Set_BTNSW_ONOFF({"btnsw_onoff":this_btnsw_onoff, "switch_status":$(this).hasClass("on")});
		});

	return $container;
}
function Get_Component_BTNSW_WiFi(view_mode){
	let $container = $("<div>").addClass("setting_content_container no_action_container");

	if(view_mode == "popup"){
		Get_Component_Popup_Profile_Title(`WiFi`).appendTo($container)
			.find("#title_close_btn").unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				close_popup_container($container);
			});
	}
	else
		Get_Component_Profile_Title(`WiFi`).appendTo($container).find("#title_del_btn").remove();

	let $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);

	Get_Component_Schematic({"fun_desc":`Turn on/off router's WiFi network.`}).appendTo($content_container);/* untranslated */

	let btnsw_onoff_parm = {"title":"Multi-Function Button Default", "type":"switch", "id":"btnsw_onoff", "set_value":"off"};
	Get_Component_Switch(btnsw_onoff_parm).appendTo($content_container)
		.find("#" + btnsw_onoff_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			const this_btnsw_onoff = $(this).closest(".setting_content_container").attr("data-btnsw-onoff");
			Set_BTNSW_ONOFF({"btnsw_onoff":this_btnsw_onoff, "switch_status":$(this).hasClass("on")});
		});

	return $container;
}
function Get_Component_BTNSW_NoFun(view_mode){
	let $container = $("<div>").addClass("setting_content_container no_action_container");

	if(view_mode == "popup"){
		Get_Component_Popup_Profile_Title(`No Function`).appendTo($container)/* untranslated */
			.find("#title_close_btn").unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				close_popup_container($container);
			});
	}
	else
		Get_Component_Profile_Title(`No Function`).appendTo($container).find("#title_del_btn").remove();

	let $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);

	Get_Component_Schematic({"fun_desc":`No action.`}).appendTo($content_container);/* untranslated */

	let btnsw_onoff_parm = {"title":"Multi-Function Button Default", "type":"switch", "id":"btnsw_onoff", "set_value":"off"};
	Get_Component_Switch(btnsw_onoff_parm).appendTo($content_container)
		.find("#" + btnsw_onoff_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			const this_btnsw_onoff = $(this).closest(".setting_content_container").attr("data-btnsw-onoff");
			if($(this).hasClass("off")){
				if(this_btnsw_onoff == nv_btnsw_onoff){
					show_customize_alert(`Please select another function to cancel the default.`);
					$(this).toggleClass("off on");
					return;
				}
			}
			Set_BTNSW_ONOFF({"btnsw_onoff":this_btnsw_onoff, "switch_status":$(this).hasClass("on")});
		});

	return $container;
}
function Get_Component_BTNSW_SDN(view_mode){
	let $container = $("<div>").addClass("setting_content_container no_action_container");

	if(view_mode == "popup"){
		Get_Component_Popup_Profile_Title(Guest_Network_naming).appendTo($container)
			.find("#title_close_btn").unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				close_popup_container($container);
			});
	}
	else
		Get_Component_Profile_Title(Guest_Network_naming).appendTo($container).find("#title_del_btn").remove();

	let $content_container = $("<div>").addClass("popup_content_container profile_setting").appendTo($container);

	Get_Component_Schematic({"fun_desc":`While switch on, ${Guest_Network_naming} connection will be turn on automatically.`}).appendTo($content_container);

	let btnsw_onoff_parm = {"title":"Multi-Function Button Default", "type":"switch", "id":"btnsw_onoff", "set_value":"off"};
	Get_Component_Switch(btnsw_onoff_parm).appendTo($content_container)
		.find("#" + btnsw_onoff_parm.id + "").click(function(e){
			e = e || event;
			e.stopPropagation();
			let $SDN_Profiles_cntr = $content_container.find("[data-container=SDN_Profiles]");
			if($SDN_Profiles_cntr.find(".profile_item").length == 0){
				$(this).toggleClass("off on");
				show_customize_alert(stringSafeGet(`<#GuestNetwork_noProfile_hint#>`.replace(/VPN/g, Guest_Network_naming)));
				return false;
			}
			else{
				if($(this).hasClass("on")){
					$SDN_Profiles_cntr.show();
				}
				else{
					$SDN_Profiles_cntr.hide();
				}
				resize_iframe_height();
				let $selected_sdn = $SDN_Profiles_cntr.find(".rwd_icon_radio.clicked");
				if($selected_sdn.length){
					const sdn_idx = $selected_sdn.attr("data-idx");
					if(sdn_idx != "" && sdn_idx != undefined){
						const this_btnsw_onoff = $(this).closest(".setting_content_container").attr("data-btnsw-onoff");
						let postData = {"btnsw_onoff":this_btnsw_onoff, "switch_status":$(this).hasClass("on")};
						if(postData.switch_status == true){
							postData["btnsw_sdn_idx"] = sdn_idx;
						}
						Set_BTNSW_ONOFF(postData);
					}
				}
			}
		});

	Get_Component_SDN_Profiles().appendTo($content_container).find(".profile_item").click(function(e){
		e = e || event;
		e.stopPropagation();
		let $setting_content_cntr = $(this).closest(".setting_content_container");
		const this_btnsw_onoff = $setting_content_cntr.attr("data-btnsw-onoff");
		const btnsw_switch = $setting_content_cntr.find("#btnsw_onoff").hasClass("on");
		const sdn_idx = $(this).find(".rwd_icon_radio").attr("data-idx");
		Set_BTNSW_ONOFF({
			"btnsw_onoff": this_btnsw_onoff,
			"switch_status": btnsw_switch,
			"btnsw_sdn_idx": sdn_idx
		});
	});

	let sdn_btn_parm = {"id":"sdn_btn", "text":"<#btn_goSetting#>"};
	Get_Component_Btn(sdn_btn_parm).appendTo($content_container)
	.find("#" + sdn_btn_parm.id + "").click(function(e){
		e = e || event;
		e.stopPropagation();
		if(is_Web_iframe){
			top.location.href = "/SDN.asp";
		}
		else if(parent.businessWrapper){
			top.location.href = "/index.html?url=sdn&current_theme=business";
		}
		else{
			top.location.href = "/SDN/sdn.html" + ((typeof theme == "string" && theme != "") ? "?current_theme=" + theme + "" : "");
		}
	});

	return $container;
}
function Get_Component_VPN_Profiles(){
	let $container = $("<div>").addClass("feature_profiles_container").attr({"data-container": "VPN_Profiles"});

	let $vpnc_group = $("<div>").addClass("profile_group").appendTo($container);
	$("<div>").addClass("title").html(vpn_title_text).appendTo($vpnc_group);
	let vpnc_options = [];
	vpnc_profile_list.forEach(function(item){
		vpnc_options.push({"type":"vpnc", "text":item.desc, "value":item.vpnc_idx, "proto":item.proto, "activate":item.activate, "default_conn":item.default_conn});
	});
	if(vpnc_options.length > 0){
		let $content_cntr = $("<div>").addClass("profile_container").appendTo($vpnc_group);
		$.each(vpnc_options, function(index, value){
			Get_Component_VPN(value).appendTo($content_cntr);
		});
	}
	else{
		$("<div>").addClass("profile_hint").html(stringSafeGet(`<#GuestNetwork_noProfile_hint#>`)).appendTo($vpnc_group);
	}

	if($container.find(".rwd_icon_radio.clicked").length == 0){
		$container.find(".rwd_icon_radio[data-vpn-type=vpnc]").first().addClass("clicked");
	}
	return $container;

	function Get_Component_VPN(_profile){
		let $profile_cntr = $("<div>").addClass(()=>{
				return ("profile_item " + ((_profile.default_conn) ? "default_conn" : ""));
			})
			.unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				if($(this).find(".rwd_icon_radio").attr("data-disabled") == "true")
					return false;
				$(this).closest("[data-container=VPN_Profiles]").find(".rwd_icon_radio").removeClass("clicked");
				$(this).find(".rwd_icon_radio").addClass("clicked");
			});
		let $text_container = $("<div>").addClass("text_container").appendTo($profile_cntr)
		$("<div>").html(htmlEnDeCode.htmlEncode(_profile.text)).appendTo($text_container);
		if(_profile.default_conn)
			$("<div>").addClass("default_conn_text").html(`<#vpnc_default#>`).appendTo($text_container);

		let $select_container = $("<div>").addClass("select_container").appendTo($profile_cntr);
		$("<div>").addClass("icon_warning_amber").attr({"data-component":"icon_warning"}).unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			$(this).closest("[data-container=VPN_Profiles]").find(".error_hint_container").remove();
			let error_hint = "<#GuestNetwork_VPN_errHint0#> <#GuestNetwork_VPN_errHint1#>";
			$(this).closest(".profile_item").append(Get_Component_Error_Hint({"text":error_hint}).show());
		}).hide().appendTo($select_container);

		$("<div>").addClass("icon_error_outline").attr({"data-component":"icon_error"}).unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			$(this).closest("[data-container=VPN_Profiles]").find(".error_hint_container").remove();
			let error_hint = "<#GuestNetwork_VPN_errHint2#>";
			$(this).closest(".profile_item").append(Get_Component_Error_Hint({"text":error_hint}).show());
		}).hide().appendTo($select_container);

		$("<div>").addClass("rwd_icon_radio").attr({"data-idx":_profile.value, "data-vpn-type":_profile.type}).appendTo($select_container);
		return $profile_cntr;
	}
}
function Get_Component_SDN_Profiles(){
	let $container = $("<div>").addClass("feature_profiles_container").attr({"data-container": "SDN_Profiles"});

	let $profile_group = $("<div>").addClass("profile_group").appendTo($container);
	$("<div>").addClass("title").html(`<#GuestNetwork_ProfileList#>`).appendTo($profile_group);
	let sdn_profile_list = init_sdn_all_list();
	if(sdn_profile_list.length > 0){
		let $content_cntr = $("<div>").addClass("profile_container").appendTo($profile_group);
		$.each(sdn_profile_list, function(index, value){
			Get_Component_SDN(value).appendTo($content_cntr);
		});
	}
	else{
		$("<div>").addClass("profile_hint").html(stringSafeGet(`<#GuestNetwork_noProfile_hint#>`.replace(/VPN/g, Guest_Network_naming))).appendTo($profile_group);
	}

	if($container.find(".rwd_icon_radio.clicked").length == 0){
		$container.find(".rwd_icon_radio").first().addClass("clicked");
	}
	return $container;

	function Get_Component_SDN(_profile){
		let $profile_cntr = $("<div>").addClass("profile_item")
			.attr({"data-sdn_enable":_profile.sdn_rl.sdn_enable})
			.unbind("click").click(function(e){
				e = e || event;
				e.stopPropagation();
				if($(this).find(".rwd_icon_radio").attr("data-disabled") == "true")
					return false;
				$(this).closest("[data-container=SDN_Profiles]").find(".rwd_icon_radio").removeClass("clicked");
				$(this).find(".rwd_icon_radio").addClass("clicked");
			});
		let $text_container = $("<div>").addClass("text_container").appendTo($profile_cntr)
		$("<div>").html(htmlEnDeCode.htmlEncode(_profile.apg_rl.ssid)).appendTo($text_container);

		let $select_container = $("<div>").addClass("select_container").appendTo($profile_cntr);
		$("<div>").addClass("icon_warning_amber").attr({"data-component":"icon_warning"}).unbind("click").click(function(e){
			e = e || event;
			e.stopPropagation();
			$(this).closest("[data-container=SDN_Profiles]").find(".error_hint_container").remove();
			let error_hint = `This ${Guest_Network_naming} profile is disabled. Please go to ${Guest_Network_naming} setting page and enable it. 
			Click [ Go Setting ] below to ${Guest_Network_naming} Page.`;/* untranslated */
			$(this).closest(".profile_item").append(Get_Component_Error_Hint({"text":error_hint}).show());
		}).hide().appendTo($select_container);

		$("<div>").addClass("rwd_icon_radio").attr({"data-idx":_profile.sdn_rl.idx}).appendTo($select_container);
		return $profile_cntr;
	}

	function init_sdn_all_list(){
		const sdn_rl_attr = function(){
			this.idx = "0";
			this.sdn_name = "";
			this.sdn_enable = "1";
			this.apg_idx = "0";
		};
		const apg_rl_attr = function(){
			this.apg_idx = "";
			this.ssid = "";
		};
		let sdn_profile_list = [];
		let sdn_all_rl_info = httpApi.nvramCharToAscii(["sdn_rl"]);
		let sdn_rl = decodeURIComponent(sdn_all_rl_info.sdn_rl);
		let each_sdn_rl = sdn_rl.split("<");
		$.each(each_sdn_rl, function(index, value){
			if(value != ""){
				let sdn_all_rl = {sdn_rl:{}, apg_rl:{}};
				let profile_data = value.split(">");
				let sdn_rl_profile = set_sdn_profile(profile_data);
				if(sdn_rl_profile.idx == "0") return true;
				if(sdn_rl_profile.sdn_name === "MAINBH" || sdn_rl_profile.sdn_name === "MAINFH") return true;
				sdn_all_rl.sdn_rl = sdn_rl_profile;
				const ap_prefix = (sdn_rl_profile.sdn_name == "MAINFH" || sdn_rl_profile.sdn_name == "MAINBH") ? "apm" : "apg";
				const apg_rl_list = get_apg_rl_list(sdn_rl_profile.apg_idx, ap_prefix);
				let specific_apg = apg_rl_list.filter(function(item, index, array){
					return (item.apg_idx == sdn_rl_profile.apg_idx);
				})[0];
				if(specific_apg != undefined){
					sdn_all_rl.apg_rl = specific_apg;
				}
				sdn_profile_list.push(sdn_all_rl);
			}
		});
		return sdn_profile_list;

		function set_sdn_profile(profile_data){
			let sdn_profile = JSON.parse(JSON.stringify(new sdn_rl_attr()));
			sdn_profile.idx = profile_data[0];
			sdn_profile.sdn_name = profile_data[1];
			sdn_profile.sdn_enable = profile_data[2];
			sdn_profile.apg_idx = profile_data[5];
			return sdn_profile;
		}
		function get_apg_rl_list(_idx, _prefix){
			let prefix = (_prefix === "apg" || _prefix === "apm") ? _prefix : "apg";
			let apg_rl_list = [];
			if(parseInt(_idx) > 0){
				let apg_profile = new apg_rl_attr();
				let apg_info = httpApi.nvramCharToAscii([prefix + _idx + "_ssid"], true);
				apg_profile.apg_idx = _idx.toString();
				apg_profile.ssid = decodeURIComponent(apg_info[prefix + _idx + "_ssid"]);
				apg_rl_list.push(JSON.parse(JSON.stringify(apg_profile)));
			}
			return apg_rl_list;
		}
	}
}
function Get_Component_Schematic(_parm){
	let $profile_setting_item = $("<div>").addClass("profile_setting_item schematic");
	let $schematic_cntr = $("<div>").addClass("schematic_container").appendTo($profile_setting_item);
	let btnsw = "";
	if(_parm != undefined){
		btnsw = (_parm.btnsw != undefined) ? `btnsw_${_parm.btnsw}` : "";
	}
	$("<div>").addClass(`img_item ${btnsw}`).appendTo($schematic_cntr);
	$("<div>").addClass("notice_item").html(`*Schematic machine switch.`).appendTo($schematic_cntr);
	let fun_desc = "";
	if(_parm != undefined){
		fun_desc = (_parm.fun_desc != undefined) ? _parm.fun_desc : "";
	}
	$("<div>").addClass("fun_desc_item").html(fun_desc).appendTo($schematic_cntr);
	return $profile_setting_item;
}
function Set_BTNSW_ONOFF(_json_data){
	let post_fun = "";
	let nvramSet_obj = {"action_mode": "apply","rc_service": "restart_swbtn"};
	if(_json_data.switch_status){
		nvramSet_obj.btnsw_onoff = _json_data.btnsw_onoff;
	}
	else{
		nvramSet_obj.btnsw_onoff = "0";
	}
	if(_json_data.btnsw_vpn_idx){
		nvramSet_obj.btnsw_vpn_idx = _json_data.btnsw_vpn_idx;
		post_fun = "vpnc";
	}
	else if(_json_data.btnsw_sdn_idx){
		nvramSet_obj.btnsw_sdn_idx = _json_data.btnsw_sdn_idx;
		post_fun = "sdn";
	}
	else if(_json_data.btnsw_onoff == "1"){
		post_fun = "op";
	}
	httpApi.nvramSet(nvramSet_obj, function(){
		let $profile_item_cntrs = $("#profile_list_content .profile_item_container");
		$profile_item_cntrs.find(".item_tag").remove();
		let update_nv = ["btnsw_onoff"];
		if(post_fun == "vpnc") update_nv.push("btnsw_vpn_idx");
		if(post_fun == "sdn") update_nv.push("btnsw_sdn_idx");
		nv_btnsw_onoff = httpApi.nvramGet(update_nv, true).btnsw_onoff;
		$profile_item_cntrs.filter("[data-btnsw-onoff="+nv_btnsw_onoff+"]").find(".item_tag_container").append($("<div>").addClass("item_tag link").html("<#CTL_Default#>"));
		if(post_fun == "op"){
			const get_nvsw = httpApi.hookGet("get_nvsw", true);
			$profile_item_cntrs.filter("[data-btnsw-onoff='1']").click();
		}
	});
}
