document.write('<script type="text/javascript" src="/js/httpApi.js"></script>');
document.write('<link rel="stylesheet" href="css/asus_eula.css" type="text/css" />');

var ASUS_EULA = {
	"agree_eula_callback": function(){},

	"disagree_eula_callback": function(){},

	"config": function(set_callback, disagree_callback){
		ASUS_EULA.agree_eula_callback = set_callback;
		ASUS_EULA.disagree_eula_callback = disagree_callback;
		return ASUS_EULA;
	},

	"agree": function (eula_type) {

		if ($("#ASUS_EULA_enable").prop("checked") === false) {
			alert("<#ASUS_eula_age_confirm#>");
			$("label[for=ASUS_EULA_enable]").css({"color": top.webWrapper ? "red" : "#FC0"});
			$("div:has(#ASUS_EULA_enable)>span").css({"color": top.webWrapper ? "red" : "#FC0"});
			return false;
		}

		if(eula_type == "asus_pp"){
			httpApi.privateEula.set("1", function () {
				ASUS_EULA.close(eula_type);
				ASUS_EULA.agree_eula_callback(eula_type);
			})
		}
		else{
		httpApi.enableEula(eula_type, "1", function () {
			ASUS_EULA.close(eula_type);
			ASUS_EULA.agree_eula_callback(eula_type);
		})
		}
	},

	"disagree": function (eula_type) {
		switch (eula_type) {
			case "asus":
				httpApi.unregisterAsusDDNS(function () {
					httpApi.enableEula("asus", "0", function () {
						ASUS_EULA.close("asus");
						ASUS_EULA.disagree_eula_callback(eula_type);
					})
				})
				break;

			case "asus_pp":
				$("html,body").scrollTop(0);

				if ($("#ASUS_EULA_enable").prop("checked") === false) {
					alert("<#ASUS_eula_age_confirm#>");
					$("label[for=ASUS_EULA_enable]").css({"color": top.webWrapper ? "red" : "#FC0"});
					$("div:has(#ASUS_EULA_enable)>span").css({"color": top.webWrapper ? "red" : "#FC0"});
					return false;
				}

				$("#loadingBlock")
					.css({"visibility": "hidden"})

				$("#Loading")
					.css({"visibility": "visible"})

				$("#eula_pp_main").hide();
				$("#eula_pp_popup_main").show();

				$("#readAgainBtn").click(function () {
					$('#ASUS_EULA_enable').prop('checked', false);
					$("#eula_pp_main").show();
					$('#asus_pp_eula_scroll').scrollTop(0);
					$("#eula_pp_popup_main").hide();
					$("#applyBtn").addClass("disabled");
					$("#cancelBtn").addClass("disabled");
					$("#applyBtn").removeAttr("onClick");
					$("#cancelBtn").removeAttr("onClick");
					if ($("#asus_pp_eula_scroll").prop('offsetHeight') > $("#asus_pp_eula_scroll").prop('scrollHeight')) {
						$("#applyBtn").removeClass("disabled");
						$("#applyBtn").attr("onClick","ASUS_EULA.agree('"+eula_type+"')");
						$("#cancelBtn").removeClass("disabled");
						$("#cancelBtn").attr("onClick","ASUS_EULA.disagree('"+eula_type+"')");
					}
				})
				$("#knowRiskBtn").click(function () {
					$('#alert_popup_EULA').remove();
					ASUS_EULA.close("asus_pp");
					ASUS_EULA.disagree_eula_callback(eula_type);
					httpApi.privateEula.set("0");
				})

				break;

			case "tm":
				httpApi.enableEula("tm", "0", function () {
					ASUS_EULA.close("tm");
					ASUS_EULA.disagree_eula_callback(eula_type);
				})
				break;
		}
	},

	"status": function(eula_type, feature){
		var eulaName = eula_type.toUpperCase() + "_EULA";
		var eulaTime = eula_type.toUpperCase() + "_EULA_time";

		if(eula_type == "asus_pp"){
			var asusEula = httpApi.privateEula.get(feature);
		}
		else{
		var asusEula = httpApi.nvramGet([eulaName , eulaTime], true);
		}

		return (asusEula[eulaName] == "1" && asusEula[eulaTime] != "");
	},

	"check": function(eula_type, feature){
		if(!ASUS_EULA.status(eula_type, feature)){
			ASUS_EULA.show(eula_type);
			return false;
		}
		else{
			return true;
		}
	},

	"close": function(eula_type){
		$("#Loading").css({"visibility": "hidden"})
		$("#loadingBlock").css({"visibility": ""})
		$('#alert_' + eula_type + '_EULA').remove();
	},

	"show": function(eula_type){

		function getEulaAge(lang){
			let age = '';
			switch (lang){
				case 'TH':
					age = '20';
					break;
				case 'BR':
					age = '18';
					break;
				default:
					age = '16';
					break;
			}
			return age;
		}

		function initialLoad(uiLanguage){
			$("#asus_pp_header").show()

			$(".eula_age").html(getEulaAge(uiLanguage));
			$("#eula_url").attr({"href": "http://www.asus.com/" + uiLanguage + "/Terms_of_Use_Notice_Privacy_Policy/Privacy_Policy"});
			if (["UA", "TH", "BR", "CN", "KR"].includes(uiLanguage)){
				$(".eula_child_text").show();
			}

			$("#asus_pp_eula_scroll").on("scroll", checkScrollHeight);
			function checkScrollHeight(e) {
				let target = $(e.target);
				if ((target.prop('scrollTop') + target.prop('offsetHeight')) >= target.prop('scrollHeight')){
					$("#applyBtn").removeClass("disabled");
					$("#applyBtn").attr("onClick","ASUS_EULA.agree('"+eula_type+"')");
					$("#cancelBtn").removeClass("disabled");
					$("#cancelBtn").attr("onClick","ASUS_EULA.disagree('"+eula_type+"')");
				}
			}

			if ($("#asus_pp_eula_scroll").prop('offsetHeight') > $("#asus_pp_eula_scroll").prop('scrollHeight')) {
				$("#applyBtn").removeClass("disabled");
				$("#applyBtn").attr("onClick","ASUS_EULA.agree('"+eula_type+"')");
				$("#cancelBtn").removeClass("disabled");
				$("#cancelBtn").attr("onClick","ASUS_EULA.disagree('"+eula_type+"')");
			}
		}

		var body = document.body, html = document.documentElement;
		var pageHeight = Math.max(body.scrollHeight, body.offsetHeight, html.clientHeight, html.scrollHeight, html.offsetHeight);
		var containerId = (parent.webWrapper) ? "body" : ".banner1";

		$("#loadingBlock")
			.css({"visibility": "hidden"})

		$("#Loading")
			.css({"visibility": "visible"})
			.height(pageHeight)

		var waitasecond = 100;
		if(eula_type == "asus_pp"){
			var uiLanguage = httpApi.nvramGet(["preferred_lang"]).preferred_lang;
			var eulaCloudUrl = "https://nw-dlcdnet.asus.com/plugin/pages/asus_pp_eula.htm";
			waitasecond = 2000;

			var eulaCloudUi = $("<div>")
				.attr({
					"id": "alert_" + eula_type + "_EULA",
					"class": "eula_panel_container border-container"
				})
				.load(eulaCloudUrl, function (data) {
					httpApi.log(location.pathname, "install eulaCloudUi");
					initialLoad(uiLanguage);
				})


			if($("#" + eula_type + "_eula_content").length == 0){
				eulaCloudUi
					.prependTo(containerId)
					.fadeIn(300)
			}
		}

		setTimeout(function () {
			if ($("#" + eula_type + "_eula_content").length == 0 && $("#eula_pp_popup_main").length == 0) {
				$("<div>")
					.attr({
						"id": "alert_" + eula_type + "_EULA",
						"class": "eula_panel_container border-container"
					})
					.load(eula_type + "_eula.htm", function (data) {
						httpApi.log(location.pathname, "install eulaUi");
						if (eula_type == "asus_pp") {
							initialLoad(uiLanguage);
						} else {
							$("#cancelBtn").click(function () {
								ASUS_EULA.disagree(eula_type);
							})

							$("#applyBtn").click(function () {
								ASUS_EULA.agree(eula_type);
							})
						}
					})
					.prependTo(containerId)
					.fadeIn(300)
			}
		}, waitasecond)

		$(document.documentElement).scrollTop(0)
	}
}
