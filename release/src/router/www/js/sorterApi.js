var sorterApi = {
	"method" : "increase",
	"key" : "",
	"type" : "",
	"clickID" : "",
	"borderTopCss" : "1px solid #222",
	"borderBottomCss" : "1px solid #222",
	"borderCssClick" : "1px solid #FC0",
	sortJson : function(_jsonObj, _key, _sortType)	{
		var sortJsonObj = _jsonObj.slice();
		switch (_sortType) {
			case "num":
				sortJsonObj = sortJsonObj.sort(function (a, b) {
					if(sorterApi.method == "increase") {
						return (parseInt(a[_key]) > parseInt(b[_key])) ? 1 : ((parseInt(a[_key]) < parseInt(b[_key])) ? -1 : 0);
					}
					else
						return (parseInt(b[_key]) > parseInt(a[_key])) ? 1 : ((parseInt(b[_key]) < parseInt(a[_key])) ? -1 : 0);
				});
			break;
			case "str":
				sortJsonObj = sortJsonObj.sort(function (a, b) {
					if(sorterApi.method == "increase")
						return (a[_key].toLowerCase() > b[_key].toLowerCase()) ? 1 : ((a[_key].toLowerCase() < b[_key].toLowerCase()) ? -1 : 0);
					else
						return (b[_key].toLowerCase() > a[_key].toLowerCase()) ? 1 : ((b[_key].toLowerCase() < a[_key].toLowerCase()) ? -1 : 0);
				});
			break;
			case "ip":
				sortJsonObj = sortJsonObj.sort(function (a, b) {
					var ipAddrToIPDecimal = function(_ip_str) {
						var re = /^(\d+)\.(\d+)\.(\d+)\.(\d+)$/;
						if(re.test(_ip_str)){
							var v1 = parseInt(RegExp.$1);
							var v2 = parseInt(RegExp.$2);
							var v3 = parseInt(RegExp.$3);
							var v4 = parseInt(RegExp.$4);

							if(v1 < 256 && v2 < 256 && v3 < 256 && v4 < 256)
								return v1*256*256*256+v2*256*256+v3*256+v4; //valid
						}
						return -2; //not valid
					};
					var a_num = 0, b_num = 0;
					a_num = parseInt(ipAddrToIPDecimal(a[_key]));
					b_num = parseInt(ipAddrToIPDecimal(b[_key]));
					if(sorterApi.method == "increase")
						return (a_num > b_num) ? 1 : ((a_num < b_num) ? -1 : 0);
					else
						return (b_num > a_num) ? 1 : ((b_num < a_num) ? -1 : 0);
				});
			break;
		}
		return sortJsonObj;
	},
	drawBorder : function($allFieldObj) {
		$allFieldObj.css("border-top", sorterApi.borderTopCss);
		$allFieldObj.css("border-bottom", sorterApi.borderBottomCss);
		var drawBorderCss = (sorterApi.method == "increase") ? "border-top" : "border-bottom";
		$allFieldObj.filter(function() {
			return $(this).attr("data-clickID") == sorterApi.clickID;
		}).css(drawBorderCss, sorterApi.borderCssClick);
	}
};
