var m = new lang();
var g_storage = new myStorage();
var g_this_url = "";
var g_this_video = "";
var g_this_video_name = "";
var g_this_video_hash = "";
var g_play_id = -1;
var g_subtitle_timer = 0;
var g_isIE = false;
var g_video_player_type = "none";

function detectVLCInstalled(){
	var val = navigator.userAgent.toLowerCase();
		
	var vlcInstalled= false;
	
	if(g_isIE){
		var vlcObj = null;
		try {
			vlcObj = new ActiveXObject("VideoLAN.Vlcplugin");
		} 
		catch (e) {
			var msg= "An exception occurred while trying to create the object VideoLAN.VLCPlugin.1:\n";
		  	for (p in e)
		  		msg+= "e."+p+"= " + e[p] + "\n";
		  	//window.alert (msg);
		}
		if( null != vlcObj )
   			vlcInstalled = true;
	}
	else{
		if( navigator.plugins && navigator.plugins.length ) {
	  		for( var i=0; i < navigator.plugins.length; i++ ) {
	    		var plugin = navigator.plugins[i];
	   			if( plugin.name.indexOf("VideoLAN") > -1
	    			|| plugin.name.indexOf("VLC") > -1) {
	      			vlcInstalled = true;
	     		}
	   		}
	 	}
	}
	
	return vlcInstalled;
}

function getInternetExplorerVersion(){
   	var rv = -1; // Return value assumes failure.
	if (navigator.appName == 'Microsoft Internet Explorer'){
	var ua = navigator.userAgent;
      	var re  = new RegExp("MSIE ([0-9]{1,}[\.0-9]{0,})");
      	if (re.exec(ua) != null)
        	rv = parseFloat( RegExp.$1 );
   }
   return rv;
}

function closeWindow(){
	parent.closeJqmWindow(0);
}

$(document).keydown(function(e) {
	if (e.keyCode == 27) return false;
});

function init(){
	setTimeout(function(){
		createPlayer();
	},500);
}

function monitor(){
    var player = getPlayer("videoPlayer");
	
	if(player==null)
		return;
		
  	var current_time = getCurrentTime();
    var total_time = getTotalTime();
	 
  	if(g_video_player_type=="vlc"){  		
  		if(current_time>=total_time){
  			doStop();
  		}
  	}
  	
    if( total_time > 0 ){
		$(".currTime").text(formatTime(current_time));
		$(".totalTime").text(formatTime(total_time));
    		
		var pos = (current_time/total_time)*$(".playbar .slider-tracker").width();
		$(".playbar .slider-thumb").css("left", pos);
		$(".playbar .slider-progress").css("width", pos);
    }
  	
	record_playtime();
}

function formatTime(timeVal){
    if( typeof timeVal != 'number' )
        return "-:--:--";

    var timeHour = Math.round(timeVal);// / 1000);
    var timeSec = timeHour % 60;
    if( timeSec < 10 )
        timeSec = '0'+timeSec;
    timeHour = (timeHour - timeSec)/60;
    var timeMin = timeHour % 60;
    if( timeMin < 10 )
        timeMin = '0'+timeMin;
    timeHour = (timeHour - timeMin)/60;
    if( timeHour > 0 )
        return timeHour+":"+timeMin+":"+timeSec;
    else
        return timeMin+":"+timeSec;
}

function getCurrentTime(){
	var player = getPlayer("videoPlayer");
	
	if(player==null)
		return 0;
		
  	var current_time = 0;
	 
  	if(g_video_player_type=="html5"){
  		current_time = player.currentTime;
  	}
  	else if(g_video_player_type=="vlc"){
  		current_time = player.input.time/1000;
  	}
  	
  	return current_time;
}

function getTotalTime(){
	var player = getPlayer("videoPlayer");
	
	if(player==null)
		return 0;
		
  	var total_time = 0;
	 
  	if(g_video_player_type=="html5"){
  		total_time = player.duration;
  	}
  	else if(g_video_player_type=="vlc"){
  		total_time = player.input.length/1000;
  	}
  	
  	return total_time;
}

function doPlay(){
	var player = getPlayer("videoPlayer");
	
	if(player==null)
		return;
		
	if(g_video_player_type=="html5"){		
		player.play();
		$("#btn_play").hide();
		$("#btn_pause").show();
	}
	else if(g_video_player_type=="vlc"){
		player.playlist.play();
		$("#btn_play").hide();
		$("#btn_pause").show();
	}
}

function doPause(){
	var player = getPlayer("videoPlayer");

	if(player==null)
		return;
		
  	if(g_video_player_type=="html5"){		
		player.pause();
		$("#btn_play").show();
		$("#btn_pause").hide();
	}
	else if(g_video_player_type=="vlc"){
  		if(g_isIE)
			player.playlist.stop();
		else
			player.playlist.pause();
		
		$("#btn_play").show();
		$("#btn_pause").hide();
	}
}

function doStop(){
	var player = getPlayer("videoPlayer");

	if(player==null)
		return;

  	if(g_video_player_type=="html5"){		
		player.pause();
		$("#btn_play").show();
		$("#btn_pause").hide();
	}
	else if(g_video_player_type=="vlc"){
  		player.playlist.stop();
		$("#btn_play").show();
		$("#btn_pause").hide();
	}
}

function record_playtime(){
	var current_time = getCurrentTime();
    var total_time = getTotalTime();
	
	if(current_time>=total_time)
  		g_storage.setl(g_this_video_hash, 0);
  	else
		g_storage.setl(g_this_video_hash, current_time);
}

function doAspectRatio(){
    var player = getPlayer("videoPlayer");
	
	if(player==null)
		return;
    
    if(g_video_player_type=="html5"){
    }
    else if(g_video_player_type=="vlc"){
        player.video.aspectRatio = $("#select_aspect").val();
	}
}

function play_subtitle(){
	var player = getPlayer("videoPlayer");
	
	if(player==null)
		return;
			
	$('.srt').each(function() {
		
		var toSeconds = function(t) {
    		var s = 0.0;
    		if(t) {
      			var p = t.split(':');
      			for(i=0;i<p.length;i++){
					var a = String((p[i]==null)?"":p[i]);
					s = s * 60 + parseFloat(a.replace(',', '.'));
				}
    		}
    		return s;
  		};
		
  		var strip = function(s) {
			var a = String((s==null)?"":s);
    		return a.replace(/^\s+|\s+$/g,"");
  		};
  		
		var SortArrayByKeys = function(inputarray) {
  			var arraykeys=[];
  			for(var k in inputarray) {arraykeys.push(k);}
		  	arraykeys.sort();
		
		  	var outputarray=[];
		  	for(var i=0; i<arraykeys.length; i++) {
				outputarray[arraykeys[i]]=inputarray[arraykeys[i]];
		  	}
		  	return outputarray;
		};
		
		var playSubtitles = function(subtitleElement) {
			
    		var srt = subtitleElement.text();
    		subtitleElement.text('');
    		srt = srt.replace(/\r\n|\r|\n/g, '\n');
    
    		var subtitles = {};
			srt = String(strip(srt));
			
			var index = 1;
			var srt_ = srt.split('\n\n');
    		for(s in srt_) {
				st = String(srt_[s]).split('\n');
				
				if(st.length >=2) {
					n = st[0];
				  	i = strip(st[1].split(' --> ')[0]);
				  	o = strip(st[1].split(' --> ')[1]);
				  	t = st[2];
				  	if(st.length > 2) {
						for(j=3; j<st.length;j++)
					  		t += '\n'+st[j];
				  	}
				  	
					is = toSeconds(i);
				  	os = toSeconds(o);
				  	
					subtitles[index] = {is:is, i:i, o: o, t: t};
					
					index++;
				}
    		}
			
			subtitles = SortArrayByKeys(subtitles);
			
			if(subtitles.length>0){
				$(".subtitle").show();
				adjustVideoSize();
			}
			
			var currentSubtitle = -1;
			g_subtitle_timer = setInterval(function() {
				
				var currentTime = getCurrentTime();
				
				var subtitle = -1;
				for(var key in subtitles){						
					if(subtitles[key].is > currentTime){
						break;
					}
						
					subtitle = key;
				}
					
				if(subtitle > 0) {
					if(subtitle != currentSubtitle) {
						//alert(subtitles[subtitle].t);
						subtitleElement.text(subtitles[subtitle].t);
						currentSubtitle=subtitle;
					} 
					else if(subtitles[subtitle].o < currentTime) {
						subtitleElement.text('');
					}
				}
				else
					subtitleElement.text('');
			}, 100);
		};
  		
    	var subtitleElement = $(this);
    	
		clearInterval(g_subtitle_timer);
		
    	var srtUrl = subtitleElement.attr('data-srt');		
		if(srtUrl) {											
			$(this).load( srtUrl, function(responseText, textStatus, req) {
				playSubtitles(subtitleElement);
			});			
		} 
		else {
			subtitleElement.text('');
			$(".subtitle").hide();
			adjustVideoSize();
		}
  	});
}

function adjustVideoSize(){
	var player = getPlayer("videoPlayer");
	
	if(player==null)
		return;
		
	var video_width = $(window).width();
	var video_height = $(window).height();
	
	if($(".toolbar").is(':visible'))
		video_height -= $(".toolbar").height();
		
	if($(".footer").is(':visible'))
		video_height -= $(".footer").height();
	
	if($(".subtitle").is(':visible'))
		video_height -= $(".subtitle").height();
			
	player.style.width = video_width + "px";
	player.style.height = video_height + "px";
}

function getPlayer(name){
	return document.getElementById(name);
}

function initPlayer(){
	var player = getPlayer("videoPlayer");
	
	if(player==null){
		return;
	}
	
	var vars = getUrlVars();
	var this_subtitle = (typeof vars["s"]=="undefined") ? "" : vars["s"];
	
	//- Build subtitle select ui	
	if(this_subtitle!=""){
		var array_subtitle = this_subtitle.split(";");
		var b_show_subtitle_ctrl = false;
		var select_option_html = "<option value=''>" + m.getString('title_no_subtitle') + "</option>";
		for(var i=0; i < array_subtitle.length; i++){
			var url = array_subtitle[i];
			if(url!=""){
				var filename = url.substr( url.lastIndexOf("/")+1, url.length );
				var srt_url = window.location.protocol + "//" + window.location.host + array_subtitle[i];
				select_option_html += "<option value='" + srt_url + "'";
					
				//- default				
				if(filename.indexOf(g_this_video_name)==0){
					select_option_html += "selected ";
					$(".subtitle").attr("data-srt", srt_url);
				}
						
				select_option_html += ">" + filename + "</option>";
				b_show_subtitle_ctrl = true;
			}
		}
			
		$(select_option_html).appendTo($("#select_subtitle"));
		$(".subtitle-ctrl").css("display", ((b_show_subtitle_ctrl) ? "block" : "none" ));
		
		$('#select_subtitle').change(function(){
			$(".subtitle").attr("data-srt", $(this).val());
			play_subtitle();
		});
	
		play_subtitle();
	}
	else{
		var b_show_subtitle_ctrl = false;
		var select_option_html = "<option value=''>" + m.getString('title_no_subtitle') + "</option>";
		var client = new davlib.DavClient();
		client.initialize();
		
		var open_url = (g_this_url!="") ? g_this_url : g_this_video.substring(0, g_this_video.lastIndexOf("/")+1);
		
		client.GETVIDEOSUBTITLE(open_url, g_this_video_name, function(error, statusstring, content){
			if(error==200){
				var data = parseXml(content);
					
				$(data).find("file").each(function(i) {
					var filename = $(this).find("name").text();
					var srt_url = window.location.protocol + "//" + window.location.host + "/" + $(this).find("sharelink").text();
						
					select_option_html += "<option value='" + srt_url + "'";
					
					//- default
					if(filename.indexOf(g_this_video_name)==0){
						select_option_html += "selected ";
						$(".subtitle").attr("data-srt", srt_url);
					}
						
					select_option_html += ">" + filename + "</option>";
					b_show_subtitle_ctrl = true;
				});
					
				$(select_option_html).appendTo($("#select_subtitle"));
				$(".subtitle-ctrl").css("display", ((b_show_subtitle_ctrl) ? "block" : "none" ));
				
				$('#select_subtitle').change(function(){
					$(".subtitle").attr("data-srt", $(this).val());
					play_subtitle();
				});
			
				play_subtitle();
			}
		});
		client = null;
	}
		
	if(g_video_player_type=="html5"){
		$("#videoPlayer").on("ended", function(){
			doStop();
		});
	}
	else if(g_video_player_type=="vlc"){
		g_play_id = player.playlist.add(g_this_video);
		
		if(g_play_id==-1){
    		alert("cannot play at the moment !");
    		return;
		}
	}
	else{
		alert("Not supported player");
		return;	
	}
	
	var min_pos = parseInt($(".volumebar .slider-progress").css("left"));			
	var pos = 37;
	$(".volumebar .slider-thumb").css("left", pos);
	$(".volumebar .slider-progress").css("width", pos-min_pos);
	/////////////////////////////////////////////////////////
	
	doPlay();
	setInterval( "monitor()", 1000 );
	/////////////////////////////////////////////////////////
			
	var last_time = g_storage.getl(g_this_video_hash);
	if(last_time!=undefined && last_time!=0){
		if(confirm(m.getString("msg_last_playtime"))) {
			if(g_video_player_type=="html5"){
				player.currentTime = last_time;
			}
			else if(g_video_player_type=="vlc"){
				player.input.time = last_time*1000;
			}
		}
	}
	/////////////////////////////////////////////////////////

	$(".toolbar").show();
	$(".footer").show();
	adjustVideoSize();
	/////////////////////////////////////////////////////////
	
	$("#btn_play").click(function(){
		doPlay();
	});
	
	$("#btn_pause").click(function(){
		doPause();
	});
	
	$("#btn_stop").click(function(){
		doStop();
	});
	
	$(".playbar .slider-thumb").draggable({ 
		axis: "x",
		snap: 'true',
		cursor: 'move',
		start: function( event, ui ) {
			
		},
		drag: function( event, ui ) {
			var min_pos = 0;
			var max_pos = $(".playbar .slider-tracker").width();
			if(ui.position.left <= min_pos)
				ui.position.left = min_pos;
			if(ui.position.left >= max_pos)
				ui.position.left = max_pos;
				
			$(".playbar .slider-progress").css("width", ui.position.left);
		},
		stop: function( event, ui ) {
			var total_time = getTotalTime();
			var seek_time = 0;
			
  			if(total_time>0){					
				seek_time = (ui.position.left / $(".playbar .slider-tracker").width())*total_time;
			}
			
			if(g_video_player_type=="html5"){
		  		player.currentTime = seek_time;
		  		player.play();
		  	}
		  	else if(g_video_player_type=="vlc"){
		  		player.input.time = seek_time*1000;
				player.playlist.play();
		  	}
		}
	});
	
	$(".volumebar .slider-thumb").draggable({ 
		axis: "x",
		snap: 'true',
		cursor: 'move',		
		drag: function( event, ui ) {
			var min_pos = parseInt($(".volumebar .slider-progress").css("left"));
			var max_pos = 74 - $(".volumebar .slider-thumb").width() + min_pos;
			if(ui.position.left <= min_pos)
				ui.position.left = min_pos;
			if(ui.position.left >= max_pos)
				ui.position.left = max_pos;
				
			$(".volumebar .slider-progress").css("width", ui.position.left-min_pos);
			
			if(g_video_player_type=="html5"){
				var vol = ui.position.left / 74;
				player.volume = vol;
			}
			else if(g_video_player_type=="vlc"){
				var vol = (ui.position.left / 74)*200;
				player.audio.volume = vol;
			}
		},
		stop: function( event, ui ) {
			if(g_video_player_type=="html5"){
				var vol = ui.position.left / 74;
				player.volume = vol;	
			}
			else if(g_video_player_type=="vlc"){
				var vol = (ui.position.left / 74)*200;
				player.audio.volume = vol;
			}
		}
	});
	
}

function createPlayer() {
	var vars = getUrlVars();
	var loc_lan = String(window.navigator.userLanguage || window.navigator.language).toLowerCase();		
	var lan = ( g_storage.get('lan') == undefined ) ? loc_lan : g_storage.get('lan');
	m.setLanguage(lan);
	
	$('button#btnClose').text(m.getString("btn_close"));
	$('#label_subtitle').text(m.getString("title_subtitle_file") + ": ");
	
	var this_url = (typeof vars["u"]=="undefined") ? "" : vars["u"];
	var this_video = vars["v"];
	var vlc_width = "655px";
	var vlc_height = "470px";
	var player_html = "";
	
	g_this_url = this_url;
	g_this_video = this_video;
	g_this_video_name = this_video.substring(this_video.lastIndexOf("/")+1, this_video.lastIndexOf("."));
	g_this_video_hash = md5(g_this_video.substr(g_this_video.lastIndexOf("/")+1, g_this_video.length));
	
	var val = navigator.userAgent.toLowerCase();
	var osVer = navigator.appVersion.toLowerCase();
	var file_ext = g_this_video.split('.').pop().toLowerCase();
	
	g_isIE = isIE();
	
	if( osVer.indexOf("mac") != -1 || file_ext == "mp4"){
		var video_width = $(window).width();
		var video_height = $(window).height() - $(".footer").height();
	
		player_html += "<div>";
		player_html += "<video id='videoPlayer' name='videoPlayer' width='" + video_width + "px' height='" + video_height + "px' autoplay>";
		player_html += "<source src='";
		player_html += this_video;
		player_html += "' type='video/mp4'/>";
		player_html += "Your browser does not support the video tag.";
		player_html += "</video>";
		player_html += "</div>";
		
		g_video_player_type = "html5";
	}
	else if(g_isIE){
		if(!detectVLCInstalled()){
			var video_width = $(window).width();
			var video_height = $(window).height() - $(".footer").height();
			
			player_html += "<div id='errorpage' class='section' style='width:" + video_width + "px; height:" + video_height + "px'>";
			player_html += "<img id='errorpage_bg' src='vlc_bg_img.jpg'/>";
			player_html += "<p style='position:relative;left:54px;top:60px;width:500px;font-size:20px;color:#ffffff'>" + m.getString('msg_installvlc') + "</p>";
			player_html += "<p style='position:relative;left:54px;top:80px;width:550px;font-size:16px;color:#ffffff;text-align:left'>" + m.getString('msg_installvlc2') + "</p>";
			player_html += "<p style='position:relative;left:216px;top:100px;width:350px;font-size:14px;color:#ffffff;text-align:left'>" + m.getString('msg_installvlc3') + "</p>";
			player_html += "<a href='http://www.videolan.org/vlc/' target='_blank'><div style='width:123px;height:30px;background-image:url(downloadvlc.png);position:relative;left:456px;top:100px;cursor:pointer'></div></a>";
			player_html += "</div>";
			
			$(player_html).appendTo($(".videoplayer"));
			
			$(".subtitle").hide();
			$(".footer").show();
			
			return;
		}
		
		player_html += "<div>";
		
		if(g_isIE){
			player_html += "<OBJECT classid='clsid:9BE31822-FDAD-461B-AD51-BE1D1C159921'";
			player_html += " codebase='http://download.videolan.org/pub/videolan/vlc/last/win32/axvlc.cab'";
			player_html += " id='videoPlayer' name='videoPlayer' width='" + vlc_width + "' height='" + vlc_height + "' events='True'>";
			player_html += "<param name='MRL' value='' />";
			player_html += "<param name='ShowDisplay' value='False'/>";
			player_html += "<param name='AutoLoop' value='False'/>";
			player_html += "<param name='AutoPlay' value='False'/>";
			player_html += "<param name='ToolBar' value='False'/>";
			player_html += "<param name='Volume' value='50' />";
			player_html += "<param name='StartTime' value='0' />";
			player_html += "</OBJECT>";
		}
		else{
			player_html += "<embed type='application/x-vlc-plugin'";
			player_html += " pluginspage='http://www.videolan.org'";
			player_html += " width='" + vlc_width + "' height='" + vlc_height + "' id='videoPlayer' name='videoPlayer' version='VideoLAN.VLCPlugin.2' text='Waiting for video' Volume='50' toolbar='False' AutoPlay='False'/>";
		}
		
		player_html += "</div>";
		
		g_video_player_type = "vlc";
	}
	else{
		alert("Can not play this video!");
		return;	
	}
	
	$(player_html).appendTo($("#video_player_section"));
	
	$(".subtitle").hide();
	$(".footer").show();
			
	initPlayer();
}