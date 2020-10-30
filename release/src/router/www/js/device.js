var system = new Object();

(function (){
    system.BRCMplatform = isSupport("bcmwifi") ? true : false;
    system.QCAplatform = isSupport("qcawifi") ? true : false;
    system.MTKplatform = isSupport("rawifi") ? true : false;
    system.RTKplatform = isSupport("rtkwifi") ? true : false;
    system.INTELplatform = isSupport("lantiq") ? true : false;
    system.modelName = httpApi.nvramGet(["productid"]).productid;
    system.territoryCode = httpApi.nvramGet(["territory_code"]).territory_code;

    system.band2gSupport = isSupport('2.4G') ? true : false;
    system.band5gSupport = isSupport('5G') ? true : false;
    system.band5g2Support = isSupport('5G-2') ? true : false;
    system.band6gSupport = isSupport('wifi6e') ? true : false;
    system.band60gSupport = isSupport('wigig') ? true : false;
    system.dualBandSupport = isSupport('dualband') ? true : false;
    system.triBandSupport = isSupport('triband') ? true : false;
    system.AMESHSupport = isSupport("amas") ? true : false;
    system.smartConnectSupport = (isSupport("smart_connect") || isSupport("bandstr")) ? true : false;
    system.wpa3Support = isSupport('wpa3') ? true : false;
    system.newWiFiCertSupport = isSupport("wifi2017") ? true : false;
    system.wifiLogoSupport = isSupport("wifilogo") ? true : false;
    system.yadnsHideQIS = isSupport("yadns_hideqis") ? true : false;
    system.yadnsSupport = (yadns_hideqis || isSupport("yadns")) ? true : false;
    system.lyraHideSupport = isSupport("lyra_hide") ? true : false;
})();

function wlObjConstructor(){
    this.channel = [];
    this.chanspecs = [];
    this.countryCode = '';
    this.sdkVersion = '';
    this.chipsetNumer = '';
    this.capability = '';
    this.nSupport = true;
    this.acSupport = false;
    this.axSupport = false;
    this.adSupport = false;
    this.aySupport = false;
    this.noVHTSupport = true;
    this.bw80MHzSupport = false;
    this.bw160MHzSupport = false;
    this.dfsSupport = false;
    this.acsCH13Support = false;
    this.acsBand1Support = false;
    this.acsBand3Support = false;
    this.channel160MHz = [];
    this.channel80MHz = [];
    this.channel40MHz = [];
    this.channel20MHz = [];
    this.QAM256Support = false;
    this.QAM1024Support = false;
    this.xboxOpt = false;
    this.heFrameSupport = false;
    this.mboSupport = false;
    this.twtSupport = false;
    this.bw160Enabled = false;
}

(function(){
	if(typeof Object.assign != 'function'){
		Object.assign = function(target){
			if(target == null){
				throw new TypeError('Cannot convert undefined or null to object');
			}

			target = Object(target);
			for(var i=1; i< arguments.length; i++){
				var source = arguments[i];
				if(source != null){
					for(var key in source){
						if(Object.prototype.hasOwnProperty.call(source, key)){
							target[key] = source[key];
						}
					}
				}
			}

			return target;
		}
	}
})();
