define(function(){
	var mapApi = {
		draw: function(pointObj, label, element, className, clickCallback){
			var _lon = Math.min(Math.max(parseInt(element.lon), -180), 180);
			var _lat = Math.min(Math.max(parseInt(element.lat), -90), 90);

			function pointContainerStyle(){
				this["left"] = Math.round( (_lon+180) * $("#mapContainer").width() / 360 ) + "px";
				this["top"] = Math.round( Math.abs(_lat-90) * $("#mapContainer").height() / 180 ) + "px";
				this["position"] = "absolute";
				this["opacity"] = "0.8";
				this["display"] = "none";
				this["width"] = "30px";
				this["height"] = "40px";
				this["margin-top"] = "-25px";
				this["margin-left"] = "-20px";
				this["text-align"] = "center";
				this["z-index"] = "1";
				this["font-family"] = "monospace";
				this["font-size"] = "14px";
				this["font-weight"] = "bolder";
			}

			function pointBackgroundStyle(){
				this["width"] = "20px";
				this["height"] = "60px";
				this["margin-top"] = "-50px";
				this["margin-left"] = "4px";
				this["text-align"] = "center";
				this["z-index"] = "100";
			}

			$('<div>')
				.attr("id", "map")
				.addClass(className)
				.css(new pointContainerStyle())
				.html(element.country)
				.append($('<div>')
					.addClass("pDefault")
					.css(new pointBackgroundStyle())
				)
				.appendTo("#mapContainer")
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
