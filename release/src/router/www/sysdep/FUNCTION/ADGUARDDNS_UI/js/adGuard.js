var	adGuard = {
		/* Account */
		"getAccountLimits": function(){//Get account limits
			var retData = {};
			var accessToken = Session.get("adGuard_access_token");

			$.ajax({
				url: 'https://api.adguard-dns.io/oapi/v1/account/limits',
				type: 'GET',
				headers: {"Authorization": "Bearer " + accessToken +""},
				success: function(response){
					retData.status = "ok";
					retData.devices = response.devices;
					retData.dns_servers = response.dns_servers
					retData.requests = response.requests
				},
				error: function(response){
					retData.status = "Fail to get account limits.";
					retData.response = response;
					httpApi.log("adGuard.getAccountLimits", JSON.stringify(retData));
				}
			});

			return retData;
		},
		/* Authentication */
		"getTokens":function(postData){ //Generate Access and Refresh token
			var retData = {};

			$.ajax({
				url: 'https://api.adguard-dns.io/oapi/v1/oauth_token',
				dataType: 'json',
				type: 'POST',
				async: false,
				data: postData,
				success: function(response){
					retData.status = "ok";
					Object.keys(response).forEach(function(key){retData[key] = response[key];});
					Session.set("adGuard_account", postData.username);
					Session.set("adGuard_access_token", response.access_token);
					Session.set("adGuard_refresh_token", response.refresh_token);
				},
				error: function(response){
					if(response.status == 400)//Bad Request
						retData.status = response.responseJSON.message;
					else if(response.status == 401)//Unauthorized
						retData.status = "<#DDNS_adGuard_getToken_unrecognized#>";
					else
						retData.status = "Something went wrong, please try again later.";//untranslated
					retData.response = response;
					httpApi.log("adGuard.getTokens", JSON.stringify(retData));
				}
			});

			return retData;

		},

		"revokeRefreshToken": function(){//Revokes a Refresh Token
			var retData = {};
			var refreshToken = Session.get("adGuard_refresh_token");

			$.ajax({
				url: 'https://api.adguard-dns.io/oapi/v1/revoke_token',
				type: 'POST',
				async: false,
				data: {"refresh_token": refreshToken},
				success: function(response){
					httpApi.nvramSet({"action_mode": "apply", "adGuard_refresh_token" : ""});
					retData.status = "ok";
				},
				error: function(response){
					retData.status = "Fail to revoke refresh token.";
					retData.response = response;
					httpApi.log("adGuard.revokeRefreshToken", JSON.stringify(retData));
				}
			});

			return retData;

		},
		/* DNS servers */
		"createNewDNS": function(newDNSname, customSettings){//Creates a new DNS server
			var retData = {};
			var accessToken = Session.get("adGuard_access_token");

			$.ajax({
				url: 'https://api.adguard-dns.io/oapi/v1/dns_servers',
				type: 'POST',
				async: false,
				headers: {"Authorization": "Bearer " + accessToken +""},
				contentType:'application/json',
				data: JSON.stringify({"name": newDNSname, "settings":customSettings}),
				success: function(response){
					retData.status = "ok";
					retData.response = response
					console.log("createNewDNS: retData = "+JSON.stringify(retData));
				},
				error: function(response){
					retData.status = "Fail to create new DNS.";
					retData.response = response;
					console.log("createNewDNS: response = "+JSON.stringify(response));
					httpApi.log("adGuard.createNewDNS", JSON.stringify(retData));
				}
			});

			return retData;

		},
		"getDNSList": function(){//Lists DNS servers
			var retData = {};
			var accessToken = Session.get("adGuard_access_token");
			if(accessToken.length > 0){
				$.ajax({
					url: 'https://api.adguard-dns.io/oapi/v1/dns_servers',
					type: 'GET',
					async: false,
					headers: {"Authorization": "Bearer " + accessToken +""},
					success: function(response){
						retData.status = "ok";
						retData.dnsList = response;
					},
					error: function(response){
						retData.status = "Fail to get DNS list.";
						retData.response = response;
						httpApi.log("adGuard.getDNSList", JSON.stringify(retData));
					}
				});
			}
			else
				retData.status = "Need to get access token first.";

			return retData;

		},
		"updateDNSSettings": function(dnsID, settingData){//Updates DNS server settings
			var retData = {};
			var accessToken = Session.get("adGuard_access_token");
			var dnsSettings = {};

			$.ajax({
				url: 'https://api.adguard-dns.io/oapi/v1/dns_servers/' + dnsID,
				type: 'GET',
				async: false,
				headers: {"Authorization": "Bearer " + accessToken},
				data: JSON.stringify(settingData),
				success: function(response){
					var needUpdate = false;
					dnsSettings = response.settings;
					$.each(settingData, function(key) {
						if(dnsSettings[key] != undefined){
							dnsSettings[key] = settingData[key];
							console.log("updateDNSSettings set dnsSettings[" + key +"] = "+settingData[key]);
							needUpdate = true;
						}
						else{
							var err = "No such settings:" + key;
							httpApi.log("adGuard.updateDNSSettings", err);
						}
					});

					if(needUpdate){
						$.ajax({
							url: 'https://api.adguard-dns.io/oapi/v1/dns_servers/'+ dnsID + '/settings',
							type: 'PUT',
							headers: {"Authorization": "Bearer " + accessToken},
							contentType:'application/json',
							data: JSON.stringify(dnsSettings),
							success: function(response){
								retData.status = "ok";
							},
							error: function(response){
								retData.status = "Fail to get the original DNS settings.";
								retData.response = response;
								httpApi.log("adGuard.updateDNSSettings", JSON.stringify(retData));
							}
						});
					}
					else{
						retData.status = "No need to update.";
					}

				},
				error: function(response){
					retData.status = "Fail to update DNS settings.";
					retData.response = response;
					httpApi.log("adGuard.updateDNSSettings", JSON.stringify(retData));
				}
			});

			return retData;

		},
		"removeDNSByID": function(dnsID){//Removes a DNS server
			var retData = {};
			var accessToken = Session.get("adGuard_access_token");

			$.ajax({
				url: 'https://api.adguard-dns.io/oapi/v1/dns_servers/' + dnsID,
				type: 'DELETE',
				headers: {"Authorization": "Bearer " + accessToken},
				success: function(response){
					retData.status = "ok";
				},
				error: function(response){
					retData.status = "Fail to remove DNS by ID";
					retData.response = response;
					httpApi.log("adGuard.removeDNSByID", JSON.stringify(retData));
				}
			});

			return retData;

		},
		"updateDNSProfileName": function(dnsID, newName){//Updates an existing DNS server
			var retData = {};
			var accessToken = Session.get("adGuard_access_token");
			var newNameObj = {"name": newName };

			$.ajax({
				url: 'https://api.adguard-dns.io/oapi/v1/dns_servers/'+ dnsID,
				type: 'PUT',
				headers: {"Authorization": "Bearer " + accessToken},
				contentType:'application/json',
				data: JSON.stringify(newNameObj),
				success: function(response){
					retData.status = "ok";
				},
				error: function(response){
					retData.status = "Fail to update DNS profile name";
					retData.response = response;
					httpApi.log("adGuard.updateDNSProfileName", JSON.stringify(retData));
				}
			});

			return retData;

		},
		"getDNSByID": function(dnsID){ //Gets an existing DNS server by ID
			var retData = {};
			var accessToken = Session.get("adGuard_access_token");

			$.ajax({
				url: 'https://api.adguard-dns.io/oapi/v1/dns_servers/' + dnsID,
				type: 'GET',
				async: false,
				headers: {"Authorization": "Bearer " + accessToken},
				success: function(response){
					retData.status = "ok";
					retData.dnsInfo = response;
					console.log("getDNSList: dns = "+JSON.stringify(retData.response));
				},
				error: function(response){
					retData.status = "Fail to get DNS by ID";
					retData.response = response;
					httpApi.log("adGuard.getDNSByID", JSON.stringify(retData));
				}
			});

			return retData;

		},
		/* Devices */
		"getDeviceList": function(){//Lists devices
			var retData = {};
			var accessToken = Session.get("adGuard_access_token");

			$.ajax({
				url: 'https://api.adguard-dns.io/oapi/v1/devices',
				type: 'GET',
				async: false,
				headers: {"Authorization": "Bearer " + accessToken +""},
				success: function(response){
					retData.status = "ok";
					retData.deviceList = response;
				},
				error: function(response){
					retData.status = "Fail to get Device list.";
					retData.response = response;
					httpApi.log("adGuard.getDeviceList", JSON.stringify(retData));
				}
			});

			return retData;

		},
		"createNewDevice": function(deviceType, newDeviceName, DNSid){//Creates a new device
			var retData = {};
			var accessToken = Session.get("adGuard_access_token");

			$.ajax({
				url: 'https://api.adguard-dns.io/oapi/v1/devices',
				type: 'POST',
				async: false,
				headers: {"Authorization": "Bearer " + accessToken +""},
				contentType:'application/json',
				data: JSON.stringify({"device_type": deviceType, "dns_server_id": DNSid, "name": newDeviceName}),
				success: function(response){
					retData.status = "ok";
					retData.id = response.id;
					retData.dns_over_tls_url = response.dns_addresses.dns_over_tls_url.replace('tls://', '');
					retData.dns_ipaddress = "94.140.14.59";
					var ip_addresses_list = response.dns_addresses.ip_addresses;
					$.each(ip_addresses_list, function(ipaddress_index){
						if(ip_addresses_list[ipaddress_index].type == "V4"){
							retData.dns_ipaddress = ip_addresses_list[ipaddress_index].ip_address;
							return false;
						}
					});
					retData.response = response;
					httpApi.log("adGuard.createNewDevice", JSON.stringify(retData));
				},
				error: function(response){
					retData.response = response;
					if(response.status == "429"){
						retData.status = "<#DDNS_limmited_profile#> <#DDNS_remove_profile#>";
					}
					else{
						retData.status = "Fail to create new device.";
					}
					httpApi.log("adGuard.createNewDevice", JSON.stringify(retData));
				}
			});

			return retData;
		},
		"removeDeviceByID": function(deviceID){//Removes a DNS server
			var retData = {};
			var accessToken = Session.get("adGuard_access_token");

			$.ajax({
				url: 'https://api.adguard-dns.io/oapi/v1/devices/' + deviceID,
				type: 'DELETE',
				headers: {"Authorization": "Bearer " + accessToken},
				success: function(response){
					retData.status = "ok";
				},
				error: function(response){
					retData.status = "Fail to remove device by ID";
					retData.response = response;
					httpApi.log("adGuard.removeDNSByID", JSON.stringify(retData));
				}
			});

			return retData;

		},
		"getDeviceByID": function(deviceID){ //Gets an existing device by ID
			var retData = {};
			var accessToken = Session.get("adGuard_access_token");

			$.ajax({
				url: 'https://api.adguard-dns.io/oapi/v1/devices/' + deviceID,
				type: 'GET',
				async: false,
				headers: {"Authorization": "Bearer " + accessToken},
				success: function(response){
					retData.status = "ok";
					retData.deviceInfo = response;
				},
				error: function(response){
					retData.status = "Fail to get device by ID";
					retData.response = response;
					httpApi.log("adGuard.getDeviceByID", JSON.stringify(retData));
				}
			});

			return retData;

		},
		"updateDeviceSettings": function(deviceID, newSettings){//Updates an existing device
			var retData = {};
			var accessToken = Session.get("adGuard_access_token");

			$.ajax({
				url: 'https://api.adguard-dns.io/oapi/v1/devices/'+ deviceID,
				type: 'PUT',
				headers: {"Authorization": "Bearer " + accessToken},
				contentType:'application/json',
				data: JSON.stringify(newSettings),
				success: function(response){
					retData.status = "ok";
				},
				error: function(response){
					retData.status = "Fail to update Device settings.";
					retData.response = response;
					httpApi.log("adGuard.updateDeviceSettings", JSON.stringify(retData));
				}
			});

			return retData;
		},
		/* Filter Lists */
		"getFilterList": function(){//get filter lists
			var retData = {};
			var accessToken = Session.get("adGuard_access_token");

			$.ajax({
				url: 'https://api.adguard-dns.io/oapi/v1/filter_lists',
				type: 'GET',
				async: false,
				headers: {"Authorization": "Bearer " + accessToken +""},
				success: function(response){
					retData.status = "ok";
					retData.response = response;
					console.log("getFilterList: FilterList = "+JSON.stringify(retData.response));
				},
				error: function(response){
					retData.status = "Fail to get Device list.";
					retData.response = response;
					httpApi.log("adGuard.getFilterList", JSON.stringify(retData));
				}
			});

			return retData;

		},
		"getWebServiceList": function(){//List Web services list adGuard can block
			var retData = {};
			var accessToken = Session.get("adGuard_access_token");

			$.ajax({
				url: 'https://api.adguard-dns.io/oapi/v1/web_services',
				type: 'GET',
				headers: {"Authorization": "Bearer " + accessToken +""},
				success: function(response){
					retData.status = "ok";
					retData.response = response;
					console.log("getWebServiceList: webServiceList = "+JSON.stringify(retData.response));
				},
				error: function(response){
					retData.status = "Fail to get web service list.";
					retData.response = response;
					httpApi.log("adGuard.getWebServiceList", JSON.stringify(retData));
				}
			});

			return retData;

		},
	}
