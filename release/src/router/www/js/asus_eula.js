$('head').append('<link rel="stylesheet" href="css/asus_eula.css" type="text/css" />');
$('head').append('<script type="text/javascript" src="/js/httpApi.js">');

var agree_eula_callback = function(){};
var disagree_eula_callback = function(){};

var ASUS_EULA = {
	"config": function(set_callback, disagree_callback){
		agree_eula_callback = set_callback;
		disagree_eula_callback = disagree_callback;

	},

	"reset": function(){
		agree_eula_callback = function(){};
		disagree_eula_callback = function(){};

	},

	"setting": function(eula_type){
		switch(eula_type){
			case "asus":
				if(document.getElementById("ASUS_EULA_enable").checked == true){
					require(['/require/modules/makeRequest.js'], function(makeRequest){
						makeRequest.start('/set_ASUS_EULA.cgi?ASUS_EULA=1',
							function(){
								ASUS_EULA.close("asus");
								agree_eula_callback();
							},
							function(){}
						);
					});
				}else{
					alert("<#ASUS_eula_age_confirm#>");
					document.getElementById("ASUS_EULA_enable").focus();
				}

				break;

			case "tm":
				require(['/require/modules/makeRequest.js'], function(makeRequest){
					makeRequest.start('/set_TM_EULA.cgi?TM_EULA=1',
						function(){
							ASUS_EULA.close("tm");
						},
						function(){}
					);
				});

				break;
		}
	},

	"disagree": function(eula_type){
		switch(eula_type){
			case "asus":
				$.ajax({
					url: "/unreg_ASUSDDNS.cgi",

					success: function( response ) {
						$.ajax({
							url: "/set_ASUS_EULA.cgi",
							data:{
								"ASUS_EULA":"0"
							},
							success: function( response ) {
								ASUS_EULA.close("asus");
								disagree_eula_callback();
							}
						});
					}
				});
				break;

			case "tm":
				$.ajax({
					url: "/set_TM_EULA.cgi",
					data:{
						"TM_EULA":"0"
					},

					success: function( response ) {
						ASUS_EULA.close("tm");
					}
				});
				break;
		}

	},

	"check": function(eula_type){

		switch(eula_type){
			case "asus":
				var ASUS_EULA_flag = httpApi.nvramGet(["ASUS_EULA", "ASUS_EULA_time"],true);

				if(ASUS_EULA_flag.ASUS_EULA != "1" || ASUS_EULA_flag.ASUS_EULA_time == ""){
					ASUS_EULA.show("asus");
					return false;
				}
				else{
					return true;
				}

				break;

			case "tm":
				var TM_EULA_flag = httpApi.nvramGet(["TM_EULA", "TM_EULA_time"],true);

				if(TM_EULA_flag.TM_EULA != "1" || TM_EULA_flag.TM_EULA_time == ""){
					ASUS_EULA.show("tm");
					return false;
				}
				else
					return true;

				break;
		}
	},

	"close": function(eula_type){
		switch(eula_type){
			case "asus":
				document.getElementById("ASUS_EULA_enable").checked = false;
				$('#alert_ASUS_EULA').fadeOut(100);
				break;

			case "tm":
				$('#alert_TM_EULA').fadeOut(100);
				break;
		}
	},

	"show": function(eula_type){
		var preferred_lang = httpApi.nvramGet(["preferred_lang"],true).preferred_lang;
		var ASUS_EULA_code = "";
		var eula_div = document.createElement('div');
		var alert_div_id = "alert_"+eula_type.toUpperCase() + "_EULA";
		var alert_query_id = "#" + alert_div_id;
		var lang_str = "";
		var url = "";
		var fn_array = ["DDNS"];
		var fn_str = "";
		var text_str = "";

		eula_div.id = eula_type.toUpperCase() + "_EULA_DIV";
		eula_div.className = "eula_container";

		var node = document.body;
		node.insertBefore(eula_div, node.childNodes[0]);

		switch(eula_type){
			case "asus":
				lang_str =  (preferred_lang == "EN" || preferred_lang == "SL")? "" : (preferred_lang.toLowerCase() + '/');
				alert_div_id = "alert_"+eula_type.toUpperCase() + "_EULA";

				if(preferred_lang == "CN")
					url = "https://www.asus.com.cn/Terms_of_Use_Notice_Privacy_Policy/Privacy_Policy/";
				else{
					if(preferred_lang == "SV")
						lang_str = "se/";
					else if(preferred_lang == "UK")
						lang_str = "ua-ua/";
					else if(preferred_lang == "MS")
						lang_str = "my/";
					else if(preferred_lang == "DA")
						lang_str = "dk/";

					url = "https://www.asus.com/" + lang_str +"Terms_of_Use_Notice_Privacy_Policy/Privacy_Policy/";
				}

				ASUS_EULA_code+='<div id="' + alert_div_id + '" class="alert_ASUS_EULA" style="text-align: left;">';
				ASUS_EULA_code+='<div style="text-decoration: underline; font-size: 14px;"><#ASUS_Notice#></div>';
				ASUS_EULA_code+='<div id="asus_eula_desc"><#ASUS_eula_desc0#>';
				ASUS_EULA_code+=	'<ol style="margin-left: -25px;">';
				ASUS_EULA_code+=		'<li><#ASUS_eula_desc1#>';
				ASUS_EULA_code+=			'<ol class="custom_ol">';
				ASUS_EULA_code+=				'<li><#ASUS_eula_desc1_1#>';
				ASUS_EULA_code+=					'<ol>';
				if(alexa_support && ifttt_support)
					ASUS_EULA_code+=					'<li id="asus_eula_desc1_1_1"><#ASUS_eula_desc1_1_1#></li>';
				else if(alexa_support && !ifttt_support)
					ASUS_EULA_code+=					'<li id="asus_eula_desc1_1_1"><#ASUS_eula_desc1_1_12#></li>';
				else// !alexa_support && !ifttt_support
					ASUS_EULA_code+=					'<li id="asus_eula_desc1_1_1"><#ASUS_eula_desc1_1_13#></li>';
				ASUS_EULA_code+=					'</ol>';
				ASUS_EULA_code+=				'</li>';
				ASUS_EULA_code+=				"<li><#ASUS_eula_desc1_2_s1#>&nbsp;<#ASUS_eula_desc1_2_s2#>";
				ASUS_EULA_code+=					'<ol>';
				ASUS_EULA_code+=						'<li><#ASUS_eula_desc1_2_1#></li>'
				ASUS_EULA_code+=					'</ol>';
				ASUS_EULA_code+=				'</li>';
				if(alexa_support)
					ASUS_EULA_code+=			'<li><#ASUS_eula_desc1_3#></li>';
				if(ifttt_support)
					ASUS_EULA_code+=			'<li><#ASUS_eula_desc1_4#></li>';
				ASUS_EULA_code+=			'</ol>';
				ASUS_EULA_code+=		'</li>';
				ASUS_EULA_code+=		'<li><#ASUS_eula_desc2#></li>';
				ASUS_EULA_code+=	'</ol>';
				ASUS_EULA_code+='</div>';
				ASUS_EULA_code+='<div id="eula_agree">';
				ASUS_EULA_code+=	'<input type="checkbox" name="ASUS_EULA_enable" id="ASUS_EULA_enable" value="0"><#ASUS_eula_age#>';
				ASUS_EULA_code+='</div>';
				ASUS_EULA_code+='<div id="eula_button">';
				ASUS_EULA_code+=	'<div style="float:right;padding:20px 10px 0px 0px;">';
				ASUS_EULA_code+=		'<input id="agree_btn" class="button_gen" type="button" onclick="ASUS_EULA.setting(\'asus\');">';
				ASUS_EULA_code+=	'</div>';
				ASUS_EULA_code+=	'<div style="float:right;padding:20px 8px 0px 10px;">';
				ASUS_EULA_code+=		'<input class="button_gen" type="button" onclick="ASUS_EULA.disagree(\'asus\');" value="<#CTL_Disagree#>">';
				ASUS_EULA_code+=	'</div>';
				ASUS_EULA_code+='</div>';
				ASUS_EULA_code+='</div>';
				break;

			case "tm":
				url = "https://www.asus.com/Microsite/networks/Trend_Micro_EULA/";

				ASUS_EULA_code+='<div id="' + alert_div_id + '" class="alert_ASUS_EULA">';
				ASUS_EULA_code+=	'<div style="font: 16px bolder Microsoft JhengHei;"><#lyra_TrendMicro_agreement#></div>';
				ASUS_EULA_code+= 	'<div>'
				ASUS_EULA_code+=		'<div style="margin: 18px 0; font-size:14px;"><#TM_eula_desc1#></div>';
				ASUS_EULA_code+=		'<div style="margin: 18px 0; font-size:14px;"><#TM_eula_desc2#></div>';
				ASUS_EULA_code+=		'<div style="margin: 18px 0; font-size:14px;"><#TM_eula_desc3#></div>';
				ASUS_EULA_code+=	'</div>';
				ASUS_EULA_code+=	'<div id="eula_button">';
				ASUS_EULA_code+=		'<div style="float:right;padding:20px 10px 0px 0px;">';
				ASUS_EULA_code+=			'<input id="agree_btn" class="button_gen" type="button" onclick="ASUS_EULA.setting(\'tm\');" >';
				ASUS_EULA_code+=		'</div>';
				ASUS_EULA_code+=		'<div style="float:right;padding:20px 8px 0px 10px;">';
				ASUS_EULA_code+=			'<input class="button_gen" type="button" onclick="ASUS_EULA.disagree(\'tm\');" value="<#CTL_Disagree#>">';
				ASUS_EULA_code+=		'</div>';
				ASUS_EULA_code+=	'</div>';
				ASUS_EULA_code+='</div>';
				break;
		}

		document.getElementById(eula_div.id).innerHTML = ASUS_EULA_code;
		$("#eula_url").attr("href",url);
		$("#agree_btn").attr("value", "<#CTL_Agree#>");//For FR

		if(eula_type == "asus"){
			var fn_array = ["DDNS"];
			var fn_str = "";
			var text_str = "";

			if(ifttt_support){
				fn_array.splice(0, 0, "IFTTT™");
			}
			if(alexa_support){
				fn_array.splice(0, 0, "Alexa™");
			}

			fn_str = fn_array.join('/ ');
			text_str = document.getElementById("asus_eula_desc").innerHTML.replace('DDNS', fn_str);
			document.getElementById("asus_eula_desc").innerHTML = text_str;
		}

		$(alert_query_id).fadeIn(1000);
	}
}