define(function(){
	var mapApi = {
		draw: function(pointObj, label, lon, lat, className, clickCallback){
			lon = Math.min(Math.max(parseInt(lon), -180), 180);
			lat = Math.min(Math.max(parseInt(lat), -90), 90);

			function pointStyle(){
				this["background-color"] = pointObj.color;
				this["left"] = Math.round( (lon+180) * $("#mapContainer").width() / 360 ) + "px";
				this["top"] = Math.round( Math.abs(lat-90) * $("#mapContainer").height() / 180 ) + "px";
				this["position"] = "absolute";
				this["display"] = "none";
				this["opacity"] = "0.4";
				this["width"] = pointObj.size + "px";
				this["height"] = pointObj.size + "px";
				this["border-radius"] = pointObj.size/2 + "px";
				this["margin-top"] = "-" + pointObj.size/2 + "px";
				this["margin-left"] = "-" + pointObj.size/2 + "px";
				this["border"] = "0px #AAA solid";
				this["text-align"] = "center";
				this["z-index"] = "10";
				this["cursor"] = "pointer";
				this["box-shadow"] = "0 0 10px 5px rgba(100,255,0,0.5)";
			}

			$('<div>')
				.attr("id", "map")
				.appendTo("#mapContainer")

			$('<div>')
				.addClass("point")
				.addClass(className)
				.css(new pointStyle())
				.hover(
					function(){$(this).css({"background-color": "#06bfb4"})}, 
					function(){$(this).css({"background-color": pointObj.color})}
				)
				.click(clickCallback)
				.appendTo("#map")
				.fadeIn(300);

			// Special Point
			$(".top")
				.css({"z-index": "999"})

			$(".bottom")
				.css({"z-index": "1"})
		}
	}

	return mapApi;
});
