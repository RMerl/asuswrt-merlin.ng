define(function(){
	var makeRequest = {
		_notSuccessCount: 0,
		_notSupportXML: false,

		start: function(url, callBackSuccess, callBackError){
			var xmlHttp;
			if(window.XMLHttpRequest)
				xmlHttp = new XMLHttpRequest();
			else if(window.ActiveXObject)
				xmlHttp = new ActiveXObject("Microsoft.XMLHTTP");
			else{
				makeRequest._notSupportXML = true;
				alert("Your browser does not support XMLHTTP.");
				return false;
			}

			xmlHttp.onreadystatechange = function(){
				if(xmlHttp.readyState == 4){
					if(xmlHttp.status == 200){
						callBackSuccess(xmlHttp);
					}
					else{
						makeRequest._notSuccessCount++;
						callBackError();
					}
		 		}
			}

			if(url.indexOf("?") == -1)
				xmlHttp.open('GET', url + "?hash=" + Math.random().toString(), true);
			else
				xmlHttp.open('GET', url + "&hash=" + Math.random().toString(), true);

			xmlHttp.send(null);
		},

		post: function(url, postdata, callBackSuccess, callBackError){
			var xmlHttp;
			if(window.XMLHttpRequest)
				xmlHttp = new XMLHttpRequest();
			else if(window.ActiveXObject)
				xmlHttp = new ActiveXObject("Microsoft.XMLHTTP");
			else{
				makeRequest._notSupportXML = true;
				alert("Your browser does not support XMLHTTP.");
				return false;
			}

			xmlHttp.onreadystatechange = function(){
				if(xmlHttp.readyState == 4){
					if(xmlHttp.status == 200){
						callBackSuccess(xmlHttp);
					}
					else{
						makeRequest._notSuccessCount++;
						callBackError();
					}
				}
			}

			xmlHttp.open('POST', url, true);
			xmlHttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
			xmlHttp.send(postdata);
		}
	};

	return makeRequest;
});
