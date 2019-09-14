<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml"> 
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=Edge" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title><#Web_Title#> - <#AiProtection_title_Radar#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="/js/table/table.css">
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/js/jquery.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/js/table/table.js"></script>
<style>
	@font-face{
		font-family: rog;
		src: url(/fonts/ROG_Fonts-Regular.woff) format("woff"),
		     url(/fonts/ROG_Fonts-Regular.otf) format("opentype");
	}
	.tableApi_table, .tableApi_table th{
		font-family: rog;
		font-size: 20px;
	}
	.tableApi_table th{
		background: #0000;
		color: #a9a9a9;
	}

	.data_tr_hover {
		background-color: #bd5003;
	}
	.data_tr_click {
		background-color: #bd5003;
	}
	.gameImage{
		width: 90px;
		height: 150px;
		display: inline-table;
		text-align: center;
		margin-left: 3px;
		margin-bottom: 10px;
		border: 2px solid #0000;
	}
	.gameImage:hover{
		border: 2px solid #bd5003;
		border-radius: 5px;
	}
	.selectedGame{
		border: 2px solid #bd5003;
		border-radius: 5px;
	}
	.imgWrap {
		cursor: pointer;
		overflow: hidden;
		border-radius: 5px;
	}
	.gameName{
		font-family: rog;
		line-height: 10px;
	}
	#gameListWrap{
		margin-top: 20px;
		width: 730px;		
		overflow-x: scroll;
	}
	#gameListWrap::-webkit-scrollbar-track {
		border: 6px solid #000;
		padding: 2px 0;
		background-color: #404040;
	}
	#gameListWrap::-webkit-scrollbar {
		width: 10px;
	}
	#gameListWrap::-webkit-scrollbar-thumb {
		border-radius: 10px;
		background-color: #737272;
		border: 3px solid #000;
	}
	#gameList{
		height: 165px;
		display: inline-flex;
	}
	.pL{
		background-image: url('/cards/pingMap-images/point-low.png') !important;
	}
	.pH{
		background-image: url('/cards/pingMap-images/point-high.png') !important;
	}
	.pM{
		background-image: url('/cards/pingMap-images/point.png') !important;
	}
	.pDefault{
		background-image: url('/cards/pingMap-images/point.png') !important;
	}
	.pL_text{
		color: #6a5acd;		
	}	
	.pH_text{
		color: #00bfff;
	}
	.pM_text{
		color: #0ff;
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
/*
	$.getJSON("https://nw-dlcdnet.asus.com/plugin/js/gameList.json", function(data){
		gameList = data;
		setTimeout(genGameListOption, 100);
	})
	.fail(function() {
*/
		$.getJSON("/ajax/gameList.json", function(data){
			gameList = data;
			setTimeout(genGameListOption, 100);
		})
/*
  	});
*/
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
			$('#gameSelector')
				.append(
					$('<option>', {value: i, text: i}).addClass("content_input_fd")
				)
				.change(function(elem){
					changeGame(elem.target.value);
				})

			var gameCover = $("<div>")
				.addClass("imgWrap")
				.append(
					$("<img>")
						.attr({
							"src": "/cards/pingMap-images/" + i + ".jpg",
							"height": "120px"
						})
						.css({"border-radius": "5px"})
				)

			$('<div>')
				.addClass("gameImage")
				.append(gameCover)
				.append($("<span>").addClass("gameName").html(i))
				.appendTo($("#gameList"))
				.click(function(){
					changeGame($(this).find('span').html());
					$(".selectedGame").removeClass("selectedGame")
					$(this).addClass("selectedGame");
				})
		}

		$(".gameImage")[0].click();
		// changeGame(document.form.gameSelector.value);
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
					mapApi.draw(POINT.SERVER, pointId, element, "server s"+pointId, function(){
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
					netoolApi.show(obj, "Fail to start")
				});
		},

		check: function(obj, fileName){
			if(!netoolApi.queueList[fileName]) return false;

			$.getJSON("/netool.cgi", {"type":0,"target":fileName})
				.done(function(data){
					try{
						if(parseInt(data.result[0].ping) < 100){
							obj.level = "pH";
						}
						else if(parseInt(data.result[0].ping) < 200){
							obj.level = "pM";
						}
						else{
							obj.level = "pL";
						}

						netoolApi.show(obj, parseInt(data.result[0].ping) + " ms");
					}
					catch(e){
						netoolApi.show(obj, "Result Not Found")
					}
				})
				.fail(function(data){
					setTimeout(function(){
						netoolApi.check(obj, fileName);
					}, 500);
				});
		},

		show: function(obj, content){
			if(!netoolApi.queueList[obj.serverIp]) return false;

			breath.stop($(".s"+obj.pointId));
			setTimeout(function(){$(".s"+obj.pointId).fadeIn(100)}, 500);
			netoolApi.queueList[obj.serverIp].finish = true;
			netoolApi.queue();

			$("tr[row_tr_idx='" + (obj.pointId - 1) + "']").find($(".PingStatus")).html(content).addClass(obj.level + "_text");
			if($(".PingStatus img").length === 0) $("#gameSelector").attr("disabled", false);
			$(".s"+obj.pointId).find($("div")).removeClass("pDefault").addClass(obj.level)
		}
	}

	var breath = {
		stop: function($d){
			if($d.data("timer")){
				clearInterval($d.data("timer"));
				$d
					.data("timer", null)
					.clearQueue()
					.fadeIn(100)
					.css({"z-index": 10})
			}
		},

		start: function($d){
			$d.data("timer", (function(){
				return setInterval(function(){
					$d.fadeOut(300);
					setTimeout(function(){$d.fadeIn(500);}, 300)
				}, 1000)
			})())
		}
	}

	$("body").ready(function(){
		show_menu();
		require(['/require/modules/mapApi.js'], function(mapApi){
			navigator.geolocation.getCurrentPosition(function(position){
				var homeElement = {};
				homeElement.lon = position.coords.longitude; 
				homeElement.lat = position.coords.latitude;
				homeElement.country = "H";
				mapApi.draw(POINT.HOME, "H", homeElement, "home top")
			});

		});
	});

	function controlClickEvent(_$this) {
		return false;
	}
</script>

</head>
<body class="bg">
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
												<div class="formfonttitle"><#AiProtection_title_Radar#></div>
												<div style="margin:10px 0 10px 5px;" class="splitLine"></div>
												<div class="formfontdesc"><#AiProtection_title_Radar_desc2#></div>

												<div style="margin-left:10px">
												<!--
													<#Game_List#>: 
													<select id="gameSelector" class="input_option" onchange="changeGame(this.value);"></select>
												-->
													<div id="gameListWrap">
														<div id="gameList"></div>
													</div>

													<br/>
													<div id="mapContainer"></div>
													<script>
														$("#mapContainer").css({
															"width": "730px",
															"height": "350px",
															"position": "relative",
															"background-image": "url(/cards/pingMap-images/map.svg)",
															"background-repeat": "no-repeat",
															"background-size": "100% 100%",
															"margin-top": "10px"
														})
													</script>
												</div>

												<div id="tableContainer" style="width:730px;margin-left:10px;margin-top: 30px;"></div>
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
