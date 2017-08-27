<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml"> 
<html xmlns:v>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - TabName</title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="/js/table/table.css">
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" language="JavaScript" src="/js/table/table.js"></script>
<style>
	.data_tr_hover {
		background-color: rgba(6, 191, 180, 0.8);
	}
	.data_tr_click {
		background-color: #06bfb4;
	}
</style>
<script>
	//set table Struct
	var tableStruct = {
		data: [],
		container: "tableContainer",
		capability: {
			clickEdit: true,
			hover: true
		},
		header: [ 
/*
			{
				"title" : "Flag",
				"width" : "20%"
			},
*/
			{
				"title" : "<#Manual_Setting_contry#>",
				"width" : "20%"
			},
			{
				"title" : "IP",
				"width" : "35%"
			},
			{
				"title" : "<#Game_Ping_Status#>",
				"width" : "25%"
			}
		],

		clickRawEditPanel: {
			inputs : [
/*
				{
					"editMode" : "callBack",
					"callBackFun" : controlClickEvent,
					"styleList" :  {"cursor":"pointer"}
				},
*/
				{
					"editMode" : "callBack",
					"callBackFun" : controlClickEvent,
					"styleList" :  {"cursor":"pointer"}
				},
				{
					"editMode" : "callBack",
					"callBackFun" : controlClickEvent,
					"styleList" :  {"cursor":"pointer"}
				},
				{
					"editMode" : "callBack",
					"callBackFun" : controlClickEvent,
					"className" : "PingStatus",
					"styleList" : {"cursor":"pointer"}
				}
			]
		}
	};

	var POINT = {
		SERVER: {
			"size": "15",
			"color": "#CF5"
		},
		HOME: {
			"size": "20",
			"color": "#F00"
		}
	}

	var gameList = {}
	$.getJSON("https://nw-dlcdnet.asus.com/plugin/js/gameList.json", function(data){
		gameList = data;
		setTimeout(genGameListOption, 100);
	})
	.fail(function() {
		$.getJSON("/ajax/gameList.json", function(data){
			gameList = data;
			setTimeout(genGameListOption, 100);
		})
  	});

	function rawToTable(rawData){
		var parseArray = [];
		for(var i = 0; i < rawData.length; i += 1) {
			var eachRuleArray = new Array();
			// eachRuleArray.push('<img src="/images/flags/' + rawData[i].country + '.png">');
			eachRuleArray.push(rawData[i].country);
			eachRuleArray.push(rawData[i].ip);
			eachRuleArray.push('--');
			parseArray.push(eachRuleArray);
		}

		return parseArray;
	}

	function genGameListOption(){
		for(var i in gameList){
			$('#gameSelector').append(
				$('<option>', {
					value: i,
					text: i
				})
				.addClass("content_input_fd")
			);
		}

		changeGame(document.form.gameSelector.value);
	}

	function changeGame(game){
		if($(".PingStatus img").length !== 0) return false;

		var currentGame = gameList[game];

		// TABLE
		$("#tableContainer").empty();
		tableStruct.data = rawToTable(currentGame);
		tableApi.genTableAPI(tableStruct);
		// MAP
		$(".server").remove();
		netoolApi.queueList = {};

		currentGame.forEach(function(element, index){
			var pointId = index+1;
			var thisPoint = {
				"serverIp": element.ip,
				"pointId": pointId,
				"finish": false
			}

			netoolApi.queueList[element.ip] = thisPoint;
			require(['/require/modules/mapApi.js'], function(mapApi){
				setTimeout(function(){
					mapApi.draw(POINT.SERVER, pointId, element.lon, element.lat, "server s"+pointId, function(){
						if($(".PingStatus img").length !== 0) return false;

						netoolApi.queueList = {};
						netoolApi.queueList[element.ip] = thisPoint;
						netoolApi.start(thisPoint);
					});
				}, index*100);
			});
		});

		setTimeout(netoolApi.queue, 1000);
	}

	var netoolApi = {
		queueList: {
			"serverIp": {
				"serverIp": "",
				"pointId": "",
				"finish": true
			}
		},

		queue: function(){
			for(var el in netoolApi.queueList){
				if(!netoolApi.queueList[el].finish){
					netoolApi.start(netoolApi.queueList[el]);
					return true;
				}
			}
		},

		start: function(obj){
			$("tr[row_tr_idx='" + (obj.pointId - 1) + "']").find($(".PingStatus")).html('--');
			breath.stop($(".s"+obj.pointId));
			breath.start($(".s"+obj.pointId));
			$("#gameSelector").attr("disabled", true);

			$.getJSON("/netool.cgi", {"type":2,"target":obj.serverIp})
				.done(function(data){
					if(data.successful != "0") setTimeout(function(){
						netoolApi.check(obj, data.successful)
					}, 3000)
				})
				.fail(function(e){
					netoolApi.finish(obj, "Fail to start")
				});
		},

		check: function(obj, fileName){
			$.getJSON("/netool.cgi", {"type":0,"target":fileName})
				.done(function(data){
					try{
						netoolApi.finish(obj, data.result[0].ping + " ms");
					}
					catch(e){
						netoolApi.finish(obj, "Result Not Found")
					}
				})
				.fail(function(data){
					setTimeout(function(){
						netoolApi.check(obj, fileName);
					}, 500);
				});
		},

		finish: function(obj, content){
			$("tr[row_tr_idx='" + (obj.pointId - 1) + "']").find($(".PingStatus")).html(content);
			breath.stop($(".s"+obj.pointId));
			if($(".PingStatus img").length === 0) $("#gameSelector").attr("disabled", false);

			netoolApi.queueList[obj.serverIp].finish = true;
			netoolApi.queue();
		}
	}

	var breath = {
		stop: function($d){
			if($d.data("timer")){
				clearInterval($d.data("timer"));
				$d
					.data("timer", null)
					.clearQueue()
					.css({"z-index": 10})
			}
		},
		start: function($d){
			var timerTmp = setInterval(function(){
				$d.toggle(
					function(){$(this).fadeOut(200);},
					function(){$(this).fadeIn(200);}
				);
			}, 100);

			$d
				.css({
					"z-index":"9999",
					"opacity": "0.8"
				})
				.data("timer", timerTmp)
		}
	}

	$("body").ready(function(){
		show_menu();
		require(['/require/modules/mapApi.js'], function(mapApi){
			navigator.geolocation.getCurrentPosition(function(position){
				mapApi.draw(POINT.HOME, "H", position.coords.longitude, position.coords.latitude, "home top")
			});

		});
	});

	function controlClickEvent(_$this) {
		var idx = parseInt(_$this.closest("*[row_tr_idx]").attr( "row_tr_idx" )) + 1;
		// control Map
		$(".server").css({"background-color":POINT.SERVER.color, "z-index": "10"});
		$(".s" + idx).css({"background-color":"#06bfb4", "z-index":"9999"});
		// control Table
		$(".tableApi_table").find(".row_tr").removeClass("data_tr_click");
		_$this.closest("tr").addClass("data_tr_click");
	}
</script>

</head>
<body>
	<div id="TopBanner"></div>
	<div id="Loading" class="popup_bg"></div>
	<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
	<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
		<table class="content" align="center" cellpadding="0" cellspacing="0">
			<tr>
				<td width="17">&nbsp;</td>
				<td valign="top" width="202">
					<div id="mainMenu"></div>
					<div id="subMenu"></div>
				</td>
				<td valign="top">
					<div id="tabMenu" class="submenuBlock"></div>
					<!--===================================Beginning of Main Content===========================================-->
					<input type="hidden" name="current_page" value="">
					<input type="hidden" name="next_page" value="">
					<input type="hidden" name="modified" value="">
					<input type="hidden" name="action_mode" value="">
					<input type="hidden" name="action_wait" value="">
					<input type="hidden" name="action_script" value="">
					<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
					<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
					<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
						<tr>
							<td valign="top" >
								<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
									<tbody>
										<tr>
											<td bgcolor="#4D595D" valign="top"  >
												<div>&nbsp;</div>
												<div class="formfonttitle">
												Game Radar</div>
												<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
												<div class="formfontdesc"></div>

												<div style="margin-left:10px">
													<#Game_List#>: 
													<select id="gameSelector" class="input_option" onchange="changeGame(this.value);"></select>
													<br/>
													<div id="mapContainer"></div>
													<script>
														$("#mapContainer").css({
															"width": "730px",
															"height": "450px",
															"position": "relative",
															"background-image": "url(/images/map.png)",
															"background-repeat": "no-repeat",
															"background-size": "100% 100%",
															"margin-top": "10px"
														})
													</script>
												</div>

												<div id="tableContainer" style="width:730px;margin-left:10px;margin-top: -70px;"></div>
											</td>
										</tr>
									</tbody>
								</table>
							</td>
						</tr>
					</table>
				<td width="10" align="center" valign="top">&nbsp;</td>
			</tr>
		</table>
	</form>
	<div id="footer"></div>
</body>
</html>
