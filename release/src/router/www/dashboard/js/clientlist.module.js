import {isSupport} from "./utils.module.js";
import {AsuswrtPopupPanel, AsuswrtButton, ToggleButton} from "./component.module.js";

const ui_lang = httpApi.nvramGet(["preferred_lang"]).preferred_lang;

const htmlEnDeCode = (function () {
    let charToEntityRegex,
        entityToCharRegex,
        charToEntity,
        entityToChar;

    function resetCharacterEntities() {
        charToEntity = {};
        entityToChar = {};
        addCharacterEntities({
            '&amp;': '&',
            '&gt;': '>',
            '&lt;': '<',
            '&quot;': '"',
            '&#39;': "'"
        });
    }

    function addCharacterEntities(newEntities) {
        let charKeys = [],
            entityKeys = [],
            key, echar;
        for (key in newEntities) {
            echar = newEntities[key];
            entityToChar[key] = echar;
            charToEntity[echar] = key;
            charKeys.push(echar);
            entityKeys.push(key);
        }
        charToEntityRegex = new RegExp('(' + charKeys.join('|') + ')', 'g');
        entityToCharRegex = new RegExp('(' + entityKeys.join('|') + '|&#[0-9]{1,5};' + ')', 'g');
    }

    function htmlEncode(value) {
        let htmlEncodeReplaceFn = function (match, capture) {
            return charToEntity[capture];
        };
        return (!value) ? value : String(value).replace(charToEntityRegex, htmlEncodeReplaceFn);
    }

    function htmlDecode(value) {
        let htmlDecodeReplaceFn = function (match, capture) {
            return (capture in entityToChar) ? entityToChar[capture] : String.fromCharCode(parseInt(capture.substr(2), 10));
        };
        return (!value) ? value : String(value).replace(entityToCharRegex, htmlDecodeReplaceFn);
    }

    resetCharacterEntities();

    return {
        htmlEncode: htmlEncode,
        htmlDecode: htmlDecode
    };
})();

class Client {
    constructor(props = {}) {

        const defaults = {
            type: "0",
            defaultType: "0",
            name: "",
            nickName: "",
            ip: "",
            mac: "",
            from: "",
            macRepeat: 1,
            group: "",
            rssi: "",
            isWL: 0, // 0: wired, 1: 2.4GHz, 2: 5GHz/5GHz-1 3:5GHz-2.
            is_wireless: 0, // record nmp_client is wireless
            isGN: "",
            qosLevel: "",
            curTx: "",
            curRx: "",
            totalTx: "",
            totalRx: "",
            callback: "",
            keeparp: "",
            isGateway: false,
            isWebServer: false,
            isPrinter: false,
            isITunes: false,
            isASUS: false,
            isAiBoard: false,
            isLogin: false,
            isOnline: false,
            ipMethod: "Static",
            opMode: 0,
            wlConnectTime: "00:00:00",
            vendor: "",
            dpiType: "",
            dpiDevice: "",
            internetMode: "allow",
            internetState: 1, // 1: Allow Internet access, 0: Block Internet access
            wtfast: 0,
            wlInterface: "",
            amesh_isRe: false,
            amesh_isReClient: false,
            amesh_papMac: "",
            amesh_bind_mac: "",
            amesh_bind_band: "0",
            sdn_idx: "0",
            ROG: false,
            uploadIcon: "NoIcon",
            isUserUploadImg: false,
            isCloudIconFetched: false,
            parentApName: '',
            sdn_ssid: '',
            wlAuth: "",
            realtime_data: [],
            ssid: '',
            vlan_id: "",
            location: "",
            showBlockInternet: true,
            showTimeScheduling: true,
            showConnectTo: true,
            displayConnectType: "",
            classConnectType: "",
        };

        // 合併預設值和傳入的值
        Object.assign(this, defaults, props);
    }


    async getUploadIcon(force = false) {

        function isImageBase64(str) {
            const str_tmp = str.slice();
            if (str_tmp.substring(0, 11) === "data:image/") {
                const str_tmp_arr = str_tmp.substring(11).split(";");
                if (str_tmp_arr.length !== 2) {
                    return false;
                }
                const mimeTypeRegExp = /(jpg|jpeg|gif|png|bmp|ico|svg\+xml)/;
                const mimeType_str = str_tmp_arr[0];
                if (mimeType_str.length > 12) {
                    return false;
                }
                const match_data = mimeType_str.match(mimeTypeRegExp);
                if (!match_data) {
                    return false;
                }
                const base64_str = str_tmp_arr[1];
                if (typeof base64_str !== 'undefined' && (base64_str.substring(0, 7) === "base64,")) {
                    const img_str = base64_str.substring(7);//filter base64,
                    const len = img_str.length;
                    if (!len || len % 4 != 0 || /[^A-Z0-9+\/=]/i.test(img_str)) {
                        return false;
                    }
                    const firstPaddingChar = img_str.indexOf('=');
                    return (firstPaddingChar === -1 || firstPaddingChar === len - 1 || (firstPaddingChar === len - 2 && img_str[len - 1] === '='));
                }
            }
            return false;
        }

        if (force || this.uploadIcon === "NoIcon" || this.uploadIcon === "") {
            let result = "NoIcon";
            if (isSupport("usericon")) {
                await fetch(`/appGet.cgi?hook=get_upload_icon()&clientmac=${this.mac.replace(/\:/g, "").toUpperCase()}`)
                    .then(response => response.text())
                    .then(text => {
                        const cleanedText = text.replace(/\\([^\\"/bfnrtu])/g, '$1');
                        const data = JSON.parse(cleanedText);
                        const base64_image = htmlEnDeCode.htmlEncode(data.get_upload_icon);
                        result = (isImageBase64(base64_image)) ? base64_image : "NoIcon";
                    })
                    .catch(err => console.error('Error:', err));
            }
            this.uploadIcon = result;
        }
        return this.uploadIcon;
    }

    async saveCloudAsusClientIcon() {
        if (!this.isCloudIconFetched && this.type !== '' && !this.isUserUploadImg && this.isASUS && this.type == this.defaultType && this.name != "ASUS") {
            const mac = this.mac.replace(/\:/g, "").toUpperCase();
            await fetch(`https://nw-dlcdnet.asus.com/plugin/productIcons/${this.name}.png`)
                .then(response => {
                    if (!response.ok) {
                        throw new Error('Network response was not ok');
                    }
                    return response;
                })
                .then(response => {
                    if (response.status === 200) {
                        response.blob().then(blob => {
                            const reader = new FileReader();
                            reader.readAsDataURL(blob);
                            reader.onloadend = async function () {
                                let nvramSet_obj = {"action_mode": "apply"};
                                nvramSet_obj["custom_usericon"] = `${mac}>${reader.result}`
                                await httpApi.nvramSet(nvramSet_obj, () => {
                                }, true, false)
                            }
                            this.isCloudIconFetched = true;
                        });
                    }
                })
                .catch(error => {
                    // console.error('There has been a problem with your fetch operation:', error);
                });
        }
    }


    getIconHtml() {

        const vendorArrayRE = /(adobe|amazon|apple|asus|belkin|bizlink|buffalo|dell|d-link|fujitsu|google|hon hai|htc|huawei|ibm|lenovo|nec|microsoft|panasonic|pioneer|ralink|samsung|sony|synology|toshiba|tp-link|vmware)/;

        function getVendorIconClassName(vendorName) {
            let vendor_class_name = "";
            const match_data = vendorName.match(vendorArrayRE);
            if (Boolean(match_data) && match_data[0] != undefined) {
                vendor_class_name = match_data[0];
                if (vendor_class_name == "hon hai")
                    vendor_class_name = "honhai";
            } else {
                vendor_class_name = "";
            }
            return vendor_class_name;
        }

        //Icon
        let icon_type;
        let clientIconCode = '';
        clientIconCode += `<div class='client_icon'>`;

        if (this.uploadIcon !== "NoIcon") {
            if (this.isUserUploadImg) {
                clientIconCode += '<img class="imgUserIcon" src="' + this.uploadIcon + '">';
            } else {
                clientIconCode += '<div class="imgUserIcon"><i class="type" style="--svg:url(' + this.uploadIcon + ')"></i></div>';
            }
        } else if (this.type != "0" || this.vendor == "") {
            icon_type = "type" + this.type;
            clientIconCode += `<div style="cursor:default;" class="clientIcon_no_hover"><i class="${icon_type}"></i>`;
            if (this.type == "36")
                clientIconCode += "<div class='flash'></div>";
            clientIconCode += "</div>";
        } else if (this.vendor != "") {
            const vendorIconClassName = getVendorIconClassName(this.vendor.toLowerCase());
            if (vendorIconClassName != "" && !isSupport("sfp4m")) {
                clientIconCode += `<div class='vendorIcon_no_hover'><i class='vendor-icon ${vendorIconClassName}'></i></div>`;
            } else {
                icon_type = "type" + this.type;
                clientIconCode += `<div class='clientIcon_no_hover'><i class='${icon_type}'></i></div>`;
            }
        }
        clientIconCode += `</div>`;
        return clientIconCode;
    }

    setIcon(icon) {
        this.type = icon.type;
    }
}

export class ClientList {
    constructor() {
        this.clients = [];
        this.totalClientNum = {
            online: 0,
            wireless: 0,
            wired: 0,
            wireless_ifnames: [],
            wireless_band: {
                '2g': 0,
                '5g1': 0,
                '5g2': 0,
                '6g1': 0,
                '6g2': 0
            }
        }
        this.AiMeshTotalClientNum = [];
        this.uploadIconList = [];
        this.uploadIconFetched = false;
        this.cfg_clientlist = {};
        this.fetchDone = false;

        setInterval(() => {
            this.fetchTraffic();
        }, 2000);
    }

    async init() {
        await this.fetchClientsFromHook();

        const onlineClients = this.listOnlineClients();
        this.totalClientNum.online = onlineClients.length;

        const stats = onlineClients.reduce((acc, client) => {
            if (client.is_wireless > 0) {
                acc.wireless++;
                switch (client.isWL) {
                    case 1:
                        acc.wireless_band['2g']++;
                        break;
                    case 2:
                        acc.wireless_band['5g1']++;
                        break;
                    case 3:
                        acc.wireless_band['5g2']++;
                        break;
                    case 4:
                        acc.wireless_band['6g1']++;
                        break;
                    case 5:
                        acc.wireless_band['6g2']++;
                        break;
                }
            } else {
                acc.wired++;
            }
            return acc;
        }, {
            wireless: 0,
            wired: 0,
            wireless_band: {'2g': 0, '5g1': 0, '5g2': 0, '6g1': 0, '6g2': 0}
        });

        Object.assign(this.totalClientNum, stats);
        this.totalClientNum.wireless_ifnames = [
            stats.wireless_band['2g'],
            stats.wireless_band['5g1'],
            stats.wireless_band['5g2'],
            stats.wireless_band['6g1'],
            stats.wireless_band['6g2']
        ];

        const iconLoadPromises = onlineClients.map(async client => {
            await client.saveCloudAsusClientIcon();
            await client.getUploadIcon();
        });
        await Promise.all(iconLoadPromises);
        this.fetchDone = true;
    }

    findClientByMac(mac) {
        return this.clients.find(client => client.mac === mac);
    }

    findClientByIp(ip) {
        return this.clients.find(client => client.ip === ip);
    }

    async fetchClientDB(mac) {
        const time = Math.floor(new Date().getTime() / 1000);
        const payload = ['sta_tx', 'sta_rx', 'sta_tbyte', 'sta_rbyte', 'sta_rssi', 'data_time'];
        await fetch(`/get_diag_content_data.cgi?ts=${time}&duration=60&point=60&db=stainfo&content=${payload.join('%3B')}&filter=sta_mac>txt>${mac}>0`)
            .then(data => data.json())
            .then(data => {
                if (typeof data.contents !== 'undefined' && data.contents.length > 0) {
                    // console.log(data.contents)
                    return data.contents.map(content => ({
                        sta_tx: content[0],
                        sta_rx: content[1],
                        sta_tbyte: content[2],
                        sta_rbyte: content[3],
                        sta_rssi: content[4],
                        data_time: content[5]
                    }))

                } else {
                    return []
                }
            })
            .then(data => {
                //sort by data_time desc
                data.sort((a, b) => b.data_time - a.data_time);
                if (data.length > 0) {
                    const client = this.clients.find(client => client.mac === mac);
                    if (client) {
                        Object.assign(client, {
                            db_sta_tx: data[0].sta_tx,
                            db_sta_rx: data[0].sta_rx,
                            db_rssi: data[0].sta_rssi,
                            db_sta_tbyte: data[0].sta_tbyte,
                            db_sta_rbyte: data[0].sta_rbyte
                        });
                        client.realtime_data = data;
                    }
                }
            })

    }

    async fetchTraffic() {
        fetch(`/appGet.cgi?hook=bwdpi_status("traffic","","realtime","")`)
            .then(response => response.json())
            .then(data => {
                const traffic = data['bwdpi_status-traffic'];
                const timestamp = new Date();

                const lastTrafficMap = this.trafficData ?
                    new Map(this.trafficData.map(data => [data.mac, data])) :
                    new Map();

                this.trafficData = traffic.map(item => {
                    const [mac, tx, rx] = [item[0], parseInt(item[1]), parseInt(item[2])];
                    let realTimeTx = 0;
                    let realTimeRx = 0;

                    const lastTraffic = lastTrafficMap.get(mac);
                    if (lastTraffic) {
                        const timeDiff = (timestamp - lastTraffic.timestamp) / 1000;
                        realTimeTx = (tx - lastTraffic.tx) / timeDiff;
                        realTimeRx = (rx - lastTraffic.rx) / timeDiff;
                    }

                    return {
                        mac,
                        tx,
                        rx,
                        realTimeTx,
                        realTimeRx,
                        timestamp
                    };
                });
            })
    }

    async fetchUploadIconList(force = false) {

        if (this.uploadIconFetched && !force) {
            return this.uploadIconList;
        } else {
            if (isSupport("usericon")) {
                await fetch('/appGet.cgi?hook=get_upload_icon_count_list()')
                    .then(data => data.json())
                    .then(data => {
                        this.uploadIconFetched = true;
                        const list = data.get_upload_icon_count_list.upload_icon_list.split(">");
                        this.uploadIconList = list.filter(value => value !== '');
                    })
            }
        }
    }

    async fetchClientsFromHook() {

        const getLocation = (specific_node) => {
            const aimesh_location_arr = [
                {value: "Home", text: "<#AiMesh_NodeLocation01#>"},
                {value: "Living Room", text: "<#AiMesh_NodeLocation02#>"},
                {value: "Dining Room", text: "<#AiMesh_NodeLocation03#>"},
                {value: "Bedroom", text: "<#AiMesh_NodeLocation04#>"},
                {value: "Office", text: "<#AiMesh_NodeLocation05#>"},
                {value: "Stairwell", text: "<#AiMesh_NodeLocation06#>"},
                {value: "Hall", text: "<#AiMesh_NodeLocation07#>"},
                {value: "Kitchen", text: "<#AiMesh_NodeLocation08#>"},
                {value: "Attic", text: "<#AiMesh_NodeLocation09#>"},
                {value: "Basement", text: "<#AiMesh_NodeLocation10#>"},
                {value: "Yard", text: "<#AiMesh_NodeLocation11#>"},
                {value: "Master Bedroom", text: "<#AiMesh_NodeLocation12#>"},
                {value: "Guest Room", text: "<#AiMesh_NodeLocation13#>"},
                {value: "Kids Room", text: "<#AiMesh_NodeLocation14#>"},
                {value: "Study Room", text: "<#AiMesh_NodeLocation15#>"},
                {value: "Hallway", text: "<#AiMesh_NodeLocation16#>"},
                {value: "Walk-in Closet", text: "<#AiMesh_NodeLocation17#>"},
                {value: "Bathroom", text: "<#AiMesh_NodeLocation18#>"},
                {value: "First Floor", text: "<#AiMesh_NodeLocation26#>"},
                {value: "Second Floor", text: "<#AiMesh_NodeLocation19#>"},
                {value: "Third Floor", text: "<#AiMesh_NodeLocation20#>"},
                {value: "Storage", text: "<#AiMesh_NodeLocation21#>"},
                {value: "Balcony", text: "<#AiMesh_NodeLocation22#>"},
                {value: "Meeting Room", text: "<#AiMesh_NodeLocation23#>"},
                {value: "Garage", text: "<#AiMesh_NodeLocation25#>"},
                {value: "Gaming Room", text: "<#AiMesh_NodeLocation27#>"},
                {value: "Gym", text: "<#AiMesh_NodeLocation28#>"},
                {value: "Custom", text: "<#AiMesh_NodeLocation24#>"}
            ];
            if (specific_node.mac === specific_node.alias) {
                return aimesh_location_arr.find(item => item.value === "Home").text;
            } else if (aimesh_location_arr.find(item => item.value === specific_node.alias)) {
                return aimesh_location_arr.find(item => item.value === specific_node.alias).text;
            } else {
                return specific_node.alias;
            }
        }

        const fromNetworkmapd = await httpApi.hookGet("get_clientlist", true)
        // console.log("fromNetworkmapd", fromNetworkmapd)
        const nmpClient = await httpApi.hookGet("get_clientlist_from_json_database", true)
        // console.log("nmpClient", nmpClient)
        const cfg_clientlist = await httpApi.hookGet("get_cfg_clientlist", true)
        // console.log("cfg_clientlist", cfg_clientlist)
        await this.fetchUploadIconList();

        if (fromNetworkmapd !== undefined && Object.keys(fromNetworkmapd).length > 0 && fromNetworkmapd.maclist !== undefined) {
            for (const mac of fromNetworkmapd.maclist) {
                const thisClient = fromNetworkmapd[mac];
                const thisClientMacAddr = (typeof thisClient.mac === "undefined") ? false : thisClient.mac.toUpperCase();
                if (!thisClientMacAddr) {
                    return;
                }

                let showConnectTo = true;
                let showBlockInternet = true;
                let showTimeScheduling = true;
                if (thisClient.name.toLowerCase().includes("aiboard")) {
                    showConnectTo = false;
                    showBlockInternet = false;
                    showTimeScheduling = false;
                }

                const client = new Client({
                    ip6: typeof thisClient.ip6 === "undefined" ? "" : thisClient.ip6,
                    ip6_prefix: typeof thisClient.ip6_prefix === "undefined" ? "" : thisClient.ip6_prefix,
                    mac: thisClient.mac.toUpperCase(),
                    name: (thisClient.name !== "") ? thisClient.name.trim() : thisClient.mac,
                    nickName: thisClient.nickName.trim(),
                    ip: thisClient.ip,
                    from: thisClient.from,
                    isGateway: (thisClient.isGateway == "1"),
                    isWebServer: (thisClient.isWebServer == "1"),
                    isPrinter: (thisClient.isPrinter == "1"),
                    isITunes: (thisClient.isITunes == "1"),
                    dpiDevice: (thisClient.dpiDevice == "undefined") ? "" : thisClient.dpiDevice,
                    vendor: thisClient.vendor,
                    rssi: parseInt(thisClient.rssi),
                    isWL: parseInt(thisClient.isWL),
                    is_wireless: (thisClient.isWL > 0) ? 1 : 0,
                    sdn_idx: parseInt(thisClient.sdn_idx),
                    wlAuth: thisClient.wlAuth == "" ? "" : thisClient.wlAuth,
                    opMode: parseInt(thisClient.opMode), //0:unknow, 1: router, 2: repeater, 3: AP, 4: Media Bridg,
                    isLogin: (thisClient.isLogin == "1"),
                    group: thisClient.group,
                    callback: thisClient.callback,
                    keeparp: thisClient.keeparp,
                    ipMethod: (thisClient.sdn_idx > 0) ? "DHCP" : thisClient.ipMethod,
                    qosLevel: thisClient.qosLevel,
                    wtfast: parseInt(thisClient.wtfast),
                    internetMode: thisClient.internetMode,
                    internetState: thisClient.internetState,
                    ROG: (thisClient.ROG == "1"),
                    isUserUploadImg: this.uploadIconList.some(value => value.toUpperCase().includes(thisClient.mac.replace(/\:/g, "").toUpperCase())),
                    isASUS: (thisClient.isASUS == "1"),
                    isAiBoard: (thisClient.isAiBoard == "1"),
                    ssid: (thisClient.ssid == "undefined") ? "" : thisClient.ssid,
                    vlan_id: (thisClient.vlan_id == "undefined") ? "" : thisClient.vlan_id,
                    showBlockInternet: showBlockInternet,
                    showTimeScheduling: showTimeScheduling,
                    showConnectTo: showConnectTo,
                });

                if (!this.findClientByMac(thisClient.mac)) {
                    this.addClient(client);
                } else {
                    this.findClientByMac(thisClient.mac).macRepeat++;
                }

                client.isOnline = (thisClient.isOnline == "1") ? true : false;
                if (!isSupport("sfp4m")) {
                    client.type = thisClient.type;
                    client.defaultType = thisClient.defaultType;
                }

                if (isSupport("amas"))
                    client.isGN = ((thisClient.isGN != "") ? parseInt(thisClient.isGN) : "");
                if (isSupport("amas") && isSupport("dualband") && client.isWL == 3)
                    client.isWL = 2;

                if (isSupport("stainfo")) {
                    client.curTx = (thisClient.curTx == "") ? "" : thisClient.curTx;
                    client.curRx = (thisClient.curRx == "") ? "" : thisClient.curRx;
                    client.wlConnectTime = thisClient.wlConnectTime;
                }

                if (isSupport("amas")) {
                    if (typeof thisClient.amesh_isRe !== 'undefined') {
                        client.amesh_isRe = (thisClient.amesh_isRe == "1");
                        if (client.amesh_isRe && client.isOnline) { // re set amesh re device to offline
                            // client.isOnline = false;
                            if (typeof this.AiMeshTotalClientNum[thisClientMacAddr] === 'undefined') {
                                this.AiMeshTotalClientNum[thisClientMacAddr] = 0;
                            }
                            const specific_node = cfg_clientlist.find(item => item.mac === thisClientMacAddr);
                            if (typeof specific_node !== 'undefined') {
                                client.name = specific_node.model_name;
                                client.location = getLocation(specific_node);
                                if (specific_node.re_path == "1" || specific_node.re_path == "16" || specific_node.re_path == "32" || specific_node.re_path == "64") {
                                    //
                                } else if (specific_node.re_path == "2") {
                                    const parentNode = cfg_clientlist.find(item => item.ap2g == specific_node.pap2g);
                                    client.parentApName = parentNode?.ui_model_name || "";
                                    client.sdn_ssid = specific_node.pap2g_ssid;
                                } else if (specific_node.re_path == "128") {
                                    const parentNode = cfg_clientlist.find(item => item.ap6g == specific_node.pap6g || item.ap6g1 == specific_node.pap6g);
                                    client.parentApName = parentNode?.ui_model_name || "";
                                    client.sdn_ssid = specific_node.pap6g_ssid;
                                } else {
                                    const parentNode = cfg_clientlist.find(item => item.ap5g == specific_node.pap5g || item.ap5g1 == specific_node.pap5g);
                                    client.parentApName = parentNode?.ui_model_name || "";
                                    client.sdn_ssid = specific_node.pap5g_ssid;
                                }
                            }
                        }
                    }

                    if (typeof thisClient.amesh_isReClient !== 'undefined' && typeof thisClient.amesh_papMac !== 'undefined') {
                        client.amesh_isReClient = (thisClient.amesh_isReClient == "1");
                        client.amesh_papMac = thisClient.amesh_papMac;
                        if (client.amesh_papMac == "") {
                            client.amesh_papMac = '<% get_lan_hwaddr(); %>';
                        }
                        if (client.isOnline) {
                            if (this.AiMeshTotalClientNum[thisClient.amesh_papMac] == undefined)
                                this.AiMeshTotalClientNum[thisClient.amesh_papMac] = 1;
                            else
                                this.AiMeshTotalClientNum[thisClient.amesh_papMac] = this.AiMeshTotalClientNum[thisClient.amesh_papMac] + 1;
                        }

                        //For Client
                        if (client.amesh_isReClient) {
                            const specific_node = cfg_clientlist.find(item => item.mac === client.amesh_papMac);
                            if (typeof specific_node !== 'undefined') {
                                client.location = getLocation(specific_node);
                                client.parentApName = specific_node.ui_model_name;
                                if (client.is_wireless > 0) {
                                    client.sdn_ssid = specific_node.pap2g_ssid || specific_node.pap5g_ssid;
                                }
                            }
                        }
                    }

                    if (isSupport("force_roaming") && isSupport("sta_ap_bind")) {
                        client.amesh_bind_mac = (typeof thisClient.amesh_bind_mac == "undefined") ? "" : thisClient.amesh_bind_mac;
                        client.amesh_bind_band = (typeof thisClient.amesh_bind_band == "undefined") ? "0" : thisClient.amesh_bind_band;
                        if (client.amesh_bind_mac !== '' && client.amesh_bind_band !== '0') {
                            const cap = cfg_clientlist.find(item => item.mac === client.amesh_bind_mac);
                            client.cap_model_name = cap.ui_model_name;
                        }
                    }
                }

                if (isSupport("mlo")) {
                    client.mlo = (typeof thisClient.mlo == "undefined") ? false : (thisClient.mlo == "1" ? true : false);
                }

                if (client.isOnline) {
                    await this.fetchClientDB(client.mac);
                }
            }
        }

        if (Object.keys(nmpClient).length > 0) {
            nmpClient.maclist.forEach(mac => {
                const thisClient = nmpClient[mac];
                const thisClientMacAddr = (typeof thisClient.mac == "undefined") ? false : thisClient.mac.toUpperCase();
                if (!thisClientMacAddr) {
                    return;
                }

                if (!this.findClientByMac(thisClient.mac)) {
                    const thisClientType = (typeof thisClient.type == "undefined") ? "0" : thisClient.type;
                    const thisClientDefaultType = (typeof thisClient.defaultType == "undefined") ? thisClientType : thisClient.defaultType;
                    let thisClientName = (typeof thisClient.name == "undefined") ? thisClientMacAddr : (thisClient.name.trim() == "") ? thisClientMacAddr : thisClient.name.trim();
                    thisClientName = htmlEnDeCode.htmlEncode(thisClientName);
                    let thisClientNickName = (typeof thisClient.nickName == "undefined") ? "" : (thisClient.nickName.trim() == "") ? "" : thisClient.nickName.trim();
                    thisClientNickName = htmlEnDeCode.htmlEncode(thisClientNickName);
                    const thisClientReNode = (typeof thisClient.amesh_isRe == "undefined") ? false : ((thisClient.amesh_isRe == "1"));

                    const client = new Client({
                        mac: thisClientMacAddr,
                        name: thisClientName,
                        nickName: thisClientNickName,
                        vendor: thisClient.vendor.trim(),
                        is_wireless: parseInt(thisClient.is_wireless),
                        from: thisClient.from,
                        ROG: (thisClient.ROG == "1"),
                        isUserUploadImg: this.uploadIconList.some(value => value.toUpperCase().includes(thisClient.mac.replace(/\:/g, "").toUpperCase()))
                    });

                    this.addClient(client);

                    if (!isSupport("sfp4m")) {
                        client.type = thisClientType;
                        client.defaultType = thisClientDefaultType;
                    }

                    if (isSupport("amas")) {
                        client.amesh_isRe = thisClientReNode;
                        if (isSupport("force_roaming") && isSupport("sta_ap_bind")) {
                            client.amesh_bind_mac = (typeof thisClient.amesh_bind_mac == "undefined") ? "" : thisClient.amesh_bind_mac;
                            client.amesh_bind_band = (typeof thisClient.amesh_bind_band == "undefined") ? "0" : thisClient.amesh_bind_band;
                        }

                        if (client.amesh_isRe) {
                            const specific_node = cfg_clientlist.find(item => item.mac == thisClientMacAddr);
                            if (typeof specific_node !== 'undefined') {
                                client.name = specific_node.model_name;
                                if (specific_node.re_path == "2") {
                                    const parentNode = cfg_clientlist.find(item => item.mac == specific_node.pap2g);
                                    client.parentApName = parentNode.ui_model_name;
                                    client.sdn = specific_node.pap2g_ssid;
                                }
                            }
                        }
                    }
                } else if (!this.findClientByMac(thisClient.mac).isOnline) {
                    this.findClientByMac(thisClient.mac).from = thisClient.from;
                }
            })
        }

        const manual_dhcp_list = (function () {
            const manual_dhcp_list_array = [];
            const manual_dhcp_list = decodeURIComponent('<% nvram_char_to_ascii("", "dhcp_staticlist"); %>').replace(/&#62/g, ">").replace(/&#60/g, "<");
            const manual_dhcp_list_row = manual_dhcp_list.split("<");
            for (let dhcpIndex = 0; dhcpIndex < manual_dhcp_list_row.length; dhcpIndex += 1) {
                if (manual_dhcp_list_row[dhcpIndex] != "") {
                    const manual_dhcp_list_col = manual_dhcp_list_row[dhcpIndex].split(">");
                    const mac = manual_dhcp_list_col[0].toUpperCase();
                    const ip = manual_dhcp_list_col[1];
                    const dns = (manual_dhcp_list_col[2] == undefined) ? "" : manual_dhcp_list_col[2];
                    const item_para = {"ip": ip, "dns": dns};
                    manual_dhcp_list_array[mac] = item_para;
                }
            }
            return manual_dhcp_list_array;
        })();

        this.manual_dhcp_list = manual_dhcp_list;

        for (const client of this.clients) {
            client.ipBindingFlag = typeof manual_dhcp_list[client.mac] !== 'undefined';
        }

        this.clients = this.clients.sort((a, b) => a.amesh_isRe - b.amesh_isRe); //sort client > node
        this.clients = this.clients.filter(client => !client.isAiBoard); //filter out AiBoard clients
    }

    addClient(client) {
        if (this.clients.find(c => c.mac === client.mac)) {
            return false
        } else {
            this.clients.push(client)
            return true
        }
    }

    listAllClients() {
        return this.clients
    }

    listClients(limit = 0, status = 'all') {
        if (limit > 0) {
            if (status === 'online') {
                return this.clients.filter(client => client.isOnline).slice(0, limit)
            } else if (status === 'offline') {
                return this.clients.filter(client => !client.isOnline).slice(0, limit)
            } else if (status === 'meshnode') {
                return this.clients.filter(client => client.amesh_isRe).slice(0, limit)
            } else {
                return this.clients.slice(0, limit)
            }
        } else {
            if (status === 'online') {
                return this.clients.filter(client => client.isOnline)
            } else if (status === 'offline') {
                return this.clients.filter(client => !client.isOnline)
            } else if (status === 'meshnode') {
                return this.clients.filter(client => client.amesh_isRe)
            } else {
                return this.clients
            }
        }
    }

    listOnlineClients() {
        return this.filter({isOnline: true})
    }

    listOfflineClients() {
        return this.filter({isOnline: false})
    }

    listOnlineWireClients(status = 'all') {
        if (status === 'online') {
            return this.filter({is_wireless: 0, isOnline: true})
        } else if (status === 'offline') {
            return this.filter({is_wireless: 0, isOnline: false})
        } else if (status === 'meshnode') {
            return this.filter({is_wireless: 0, amesh_isRe: true})
        } else {
            return this.filter({is_wireless: 0})
        }
    }

    listOnlineWirelessClients(status = 'all') {
        if (status === 'online') {
            return this.clients.filter((client) => {
                return client.is_wireless > 0 && client.isOnline
            })
        } else if (status === 'offline') {
            return this.clients.filter((client) => {
                return client.is_wireless > 0 && !client.isOnline
            })
        } else if (status === 'meshnode') {
            return this.clients.filter((client) => {
                return client.is_wireless > 0 && client.amesh_isRe
            })
        } else {
            return this.clients.filter(client => client.is_wireless > 0)
        }
    }

    filter(filter = {}, limit = 0) {
        if (limit > 0) {
            return this.clients.filter(client => {
                for (const key in filter) {
                    if (client[key] !== filter[key]) {
                        return false
                    }
                }
                return true
            }).slice(0, limit)
        }
        return this.clients.filter(client => {
            for (const key in filter) {
                if (client[key] !== filter[key]) {
                    return false
                }
            }
            return true
        })
    }

    async reloadClientList() {
        this.clients = [];
        this.totalClientNum = {
            online: 0,
            wireless: 0,
            wired: 0,
            wireless_ifnames: [],
            wireless_band: {
                '2g': 0,
                '5g1': 0,
                '5g2': 0,
                '6g1': 0,
                '6g2': 0
            }
        }
        this.AiMeshTotalClientNum = [];
        this.uploadIconList = [];
        this.uploadIconFetched = false;
        await this.init();
    }
}

export class ClientListTable {
    constructor(ClientList, RightPanel) {
        this.statusShow = 'online';
        this.interfaceShow = 'all';
        this.clientList = ClientList;
        this.clients = ClientList.listAllClients();
        this.cfg_clientlist = ClientList.cfg_clientlist
        this.dataTable = null;
        this.hiddenColumns = [];
        this.filterQuery = [];
        this.filterRssi = [];
        this.RightPanel = RightPanel;
        const div = document.createElement('div');
        const shadowRoot = div.attachShadow({mode: 'open'});

        const iconList = new ClinetIconFetcher();
        this.iconList = iconList;

        const template = document.createElement('template');
        template.innerHTML = `
            <link rel="stylesheet" href="/css/bootstrap.min.css">
            <link rel="stylesheet" href="/device-map/device-map.css">
            <link rel="stylesheet" href="/device-map/clientlist.css">
            <link rel="stylesheet" href="/datatables/datatables.min.css">
            <link rel="stylesheet" href="/datatables/datatables.asuswrt.css">
            <div>
                <div class="table-tools">
                    <div class="left">
                        <div id="interface_switch">
                            <div class="segmented_picker">
                                <div class="segmented_picker_option active" data-value="all">
                                    <div class="block_filter_name"><#All#> (${ClientList.listClients(0, this.statusShow).length})</div>
                                </div>
                                <div class="segmented_picker_option" data-value="wireless">
                                    <div class="block_filter_name"><#tm_wireless#> (${ClientList.listOnlineWirelessClients(this.statusShow).length})</div>
                                </div>
                                <div class="segmented_picker_option" data-value="wire">
                                    <div class="block_filter_name"><#tm_wired#> (${ClientList.listOnlineWireClients(this.statusShow).length})</div>
                                </div>
                            </div>
                        </div>
                    </div>
                    <div class="right ">
                        <div id="status_switch">
                            <div class="content">
                                <div class="option ${this.statusShow === 'online' ? 'active' : ''}" data-value="online"><#Clientlist_Online#></div>
                                <div class="vr"></div>
                                <div class="option ${this.statusShow === 'offline' ? 'active' : ''}" data-value="offline"><#Clientlist_OffLine#></div>
                                <div class="vr"></div>
                                <div class="option ${this.statusShow === 'offline' ? 'active' : ''}" data-value="meshnode">Mesh Node</div>
                                <div class="vr"></div>
                                <div class="option ${this.statusShow === 'all' ? 'active' : ''}" data-value="all"><#All#></div>
                            </div>
                        </div>
                        <div style="display: flex;">
                            <div id="client_filter_btn"
                                 role="button"
                                 tabindex="0"
                                 aria-label="<#Filter_Mode#>"
                                 title="<#Filter_Mode#>">
                                <div><i class="filter_icon" aria-hidden="true"></i></div>
                            </div>
                            <div id="client_export_btn"
                                 role="button"
                                 tabindex="0"
                                 title="<#btn_Export#>"
                                 aria-label="<#btn_Export#>">
                                <i class="icon-export-file" aria-hidden="true"></i>
                            </div>
                        </div>
                        <div class="search"></div>
                    </div>
                </div>
                <div class="clientlist_viewlist">
                    <table class="client_list hover">
                    </table>
                </div>
            </div>
        `
        shadowRoot.appendChild(template.content.cloneNode(true));

        shadowRoot.querySelector('#interface_switch').addEventListener('click', (e) => {
            if (e.target.classList.contains('segmented_picker_option')) {
                shadowRoot.querySelectorAll('.segmented_picker_option').forEach((el) => {
                    el.classList.remove('active')
                })
                e.target.classList.add('active')
                const filter = e.target.dataset.value;
                if (filter === 'all') {
                    this.interfaceShow = 'all';
                } else if (filter === 'wireless') {
                    this.interfaceShow = 'wireless';
                } else if (filter === 'wire') {
                    this.interfaceShow = 'wire';
                }
                this.reloadTable();
            }
        });

        shadowRoot.querySelector('#status_switch').addEventListener('click', (e) => {
            if (e.target.classList.contains('option')) {
                shadowRoot.querySelectorAll('.option').forEach((el) => {
                    el.classList.remove('active')
                })
                e.target.classList.add('active')
                const filter = e.target.dataset.value;
                if (filter === 'all') {
                    this.statusShow = 'all';
                } else if (filter === 'online') {
                    this.statusShow = 'online';
                } else if (filter === 'offline') {
                    this.statusShow = 'offline';
                } else if (filter === 'meshnode') {
                    this.statusShow = 'meshnode';
                }
                this.reloadTable();
            }
        });

        const filterHandler = (e) => {
            const clientFilterView = new ClientFilterView(this);
            this.clientFilterView = clientFilterView;
            this.RightPanel.resetContent();
            this.RightPanel.setPanelContent(clientFilterView.render());
            requestAnimationFrame(() => {
                this.RightPanel.show();
            });
        };
        shadowRoot.querySelector('#client_filter_btn').addEventListener('click', filterHandler);
        shadowRoot.querySelector('#client_filter_btn').addEventListener('keydown', (e) => {
            if (e.key === 'Enter' || e.key === ' ') {
                e.preventDefault();
                filterHandler(e);
            }
        });

        const exportHandler = () => {
            if (this.dataTable) {
                CsvExporter.exportDataTableToCSV(this.dataTable, 'client_list.csv');
            } else {
                alert(`<#IPConnection_VSList_Norule#>`);
            }
        };
        shadowRoot.querySelector('#client_export_btn').addEventListener('click', exportHandler);
        shadowRoot.querySelector('#client_export_btn').addEventListener('keydown', (e) => {
            if (e.key === 'Enter' || e.key === ' ') {
                e.preventDefault();
                exportHandler();
            }
        });

        this.element = div;

        this.renderClientsInfo();

        const columns = [
            {
                id: 'connStatus',
                title: '',
                sortable: false,
                searchable: false,
                data: 'isOnline',
                render: function (data, type, row) {
                    return `<div class="client-status">${(data) ? `<i class='connect-status online'></i>` : `<i class='connect-status offline'></i>`}</div></div>`
                },
                className: 'dt-head-center dt-body-center'
            },
            {
                id: 'connType',
                title: '',
                sortable: false,
                searchable: false,
                data: 'isWL',
                render: function (data, type, row) {
                    if (!(isSwMode('mb') || isSwMode('ew'))) {
                        let rssi_t = 0;
                        if (data == "0")
                            rssi_t = "wired";
                        else
                            rssi_t = "wireless";

                        if (row.amesh_isReClient) {
                           if (row.is_wireless) {
                                return `<div class='radio-icon radio-${rssi_t} ${row.classConnectType}'></div>`;
                           }
                           else{
                              return `<div class='radio-icon radio-${rssi_t}'></div>`;
                           }
                        }
                        else {
                            return `<div class='radio-icon radio-${rssi_t}'></div>`;
                        }
                    }
                },
                className: 'dt-head-center dt-body-center'
            },
            {
                id: 'clientName',
                title: '<#Clientlist_name#>',
                data: 'displayName',
                render: function (data, type, row) {
                    return `<div class="client-name-group">
                                ${row.displayClientIcon}
                                <div class="client-name">${data}</div>
                            </div>`
                },
                className: 'dt-head-left'
            },
            {
                id: 'Vendor',
                title: `<#Vendor#>`, data: 'vendor', className: 'dt-head-left',
                render: function (data, type, row) {
                    return (data === "" || typeof data === 'undefined') ? "-" : data;
                }
            },
            {
                id: 'connectedAP',
                title: `<#Connected_AP#>`, data: 'parentApName', className: '',
                render: function (data, type, row) {
                    return (data === "" || typeof data === 'undefined') ? "-" : data;
                }
            },
            {
                id: 'location',
                title: '<#AiMesh_NodeLocation#>', data: 'location', className: '',
                render: function (data, type, row) {
                    return (data === "" || typeof data === 'undefined') ? "-" : data;
                }
            },
            {
                id: 'network',
                title: `<#Network_SDN#>`, data: 'ssid', className: '',
                render: function (data, type, row) {
                    return (data === "" || typeof data === 'undefined') ? "-" : data;
                }
            },
            {
                id: 'phyRate',
                title: `<#AiMesh_PHY_Rate#>`, data: null, className: '',
                render: function (data, type, row) {
                    let phyRate = "-";
                    if (isSupport("stainfo") && !(this.isSwMode('mb') || this.isSwMode('ew'))) {
                        if (isNaN(parseInt(row.db_sta_tx)) && isNaN(parseInt(row.db_sta_rx))) {
                            phyRate = `-`;
                        } else {
                            phyRate = `
                                    <div class="phy-rate">
                                        <div>
                                            ${row.db_sta_tx}
                                            <span>Mbps</span>
                                        </div>
                                        /
                                        <div>
                                            ${row.db_sta_rx}
                                            <span>Mbps</span>
                                        </div>
                                    </div>`;
                        }
                    }
                    return phyRate
                }.bind(this)
            },
            // {
            //     title: 'Channel Bandwidth',
            //     data: 'bandwidth',
            //     className: '',
            //     render: function (data, type, row) {
            //         return (data === "" || typeof data === 'undefined') ? "-" : data;
            //     }
            // },
            {
                id: 'clientIP',
                title: `<#Client_IP#>`, data: 'ip', type: 'numeric', className: '',
                render: function (data, type, row) {
                    return (data === "" || typeof data === 'undefined') ? "-" : data;
                }
            },
            {
                id: 'realTimeTraffic',
                title: 'Real-Time Traffic', data: null, className: '',
                render: function (data, type, row) {
                    let Tx = 0;
                    let Rx = 0;
                    let TxUnit = 'Kb';
                    let RxUnit = 'Kb';
                    if (ClientList.trafficData) {
                        const trafficData = ClientList.trafficData.find(item => item.mac === row.mac);
                        if (trafficData) {
                            Tx = trafficData.realTimeTx * 8 / 1024;
                            Rx = trafficData.realTimeRx * 8 / 1024;
                            if (Tx > 1024) {
                                Tx = Tx / 1024;
                                TxUnit = 'Mb';
                            }
                            if (Rx > 1024) {
                                Rx = Rx / 1024
                                RxUnit = 'Mb';
                            }
                            if (Tx > 1024) {
                                Tx = Tx / 1024;
                                TxUnit = 'Gb';
                            }
                            if (Rx > 1024) {
                                Rx = Rx / 1024;
                                RxUnit = 'Gb';
                            }
                        }
                        row.realTimeTx = `${Tx.toFixed(2)} ${TxUnit}`;
                        row.realTimeRx = `${Rx.toFixed(2)} ${RxUnit}`;
                        return `<div class="realtime-traffic">
                            <div><i class="arrow-up"></i>${Tx.toFixed(2)}<span>${TxUnit}</span></div>
                            <div><i class="arrow-down"></i>${Rx.toFixed(2)}<span>${RxUnit}</span></div>
                        </div>`
                    } else {
                        row.realTimeTx = `${Tx.toFixed(2)} ${TxUnit}`;
                        row.realTimeRx = `${Rx.toFixed(2)} ${RxUnit}`;
                        return '-';
                    }
                },
            },
            {
                id: 'rssi',
                title: `<#Quality_RSSI#>`,
                data: 'rssi',
                render: function (data, type, row) {
                    if (row.isWL != 0) {
                        return `${data} dBm`
                    } else {
                        return '-'
                    }
                },
                className: ''
            },
            {
                id: 'accessTime',
                title: `<#Access_Time#>`,
                data: null,
                name: 'access_time',
                className: '',
                render: function (data, type, row) {
                    if (row.wlConnectTime == "00:00:00" || row.wlConnectTime == "") {
                        return "-";
                    } else {
                        return row.wlConnectTime;
                    }
                }
            },
            {
                id: 'mac',
                title: `<#MAC_Address#>`,
                data: 'mac',
                className: ''
            },
            // {
            //     title: 'Security',
            //     data: 'wlAuth',
            //     className: '',
            //     render: function (data, type, row) {
            //         return (data === "" || typeof data === 'undefined') ? "-" : data;
            //     }
            // },
            {
                id: 'vlan',
                title: 'VLAN',
                data: 'vlan_id',
                className: '',
                render: function (data, type, row) {
                    return (data === "" || data === "0" || typeof data === 'undefined') ? "-" : data;
                }
            },
        ];

        const table = shadowRoot.querySelector('.client_list');
        if (table) {
            const script = document.createElement('script');
            script.src = '/datatables/datatables.min.js';
            script.onload = () => {

                if (this.statusShow === 'online') {
                    this.clients = this.clients.filter(client => client.isOnline)
                } else if (this.statusShow === 'offline') {
                    this.clients = this.clients.filter(client => !client.isOnline)
                }

                const dataTable = new DataTable(table, {
                    fixedColumns: {
                        start: 3,
                    },
                    order: {
                        idx: 2,
                        dir: 'asc'
                    },
                    paging: true,
                    destroy: true,
                    pageLength: 25,
                    select: 'single',
                    scrollX: true,
                    info: true,
                    scrollCollapse: false,
                    data: this.clients,
                    columns: columns,
                    lengthMenu: [5, 25, 50, 100],
                    language: {
                        info: `_START_ - _END_ <#Table_Info_NoData#>`,
                        infoEmpty: `<#Table_Info_NoData#>`,
                        infoFiltered: "(<#Table_Info_FilterData#>)",
                        emptyTable: `<#No_Data_In_Table#>`,
                        lengthMenu: `<div class="d-flex align-items-center gap-2"><div><#Rows_Per_Page#>:</div><div>_MENU_</div></div>`,
                        search: '',
                        paginate: {
                            first: '',
                            previous: '<i class="previous"></i>',
                            next: '<i class="next"></i>',
                            last: '',
                        },
                        select: {
                            rows: {
                                0: '',
                                _: '%d client selected',
                            }
                        }
                    },
                    layout: {
                        topStart: null,
                        topEnd: 'search',
                        bottomStart: ['info', {
                            paging: {
                                numbers: 0
                            }
                        }, 'pageLength'],
                        bottomEnd: {}
                    },
                    initComplete: function (settings, json) {
                        const search = shadowRoot.querySelector('.dt-search');
                        if (search) {
                            const search_icon = document.createElement('div');
                            search_icon.classList.add('search-icon');
                            search_icon.innerHTML = '<i class="search_icon"></i>';
                            search.prepend(search_icon);
                            shadowRoot.querySelector('.search').appendChild(search);
                        }
                    },
                    // drawCallback: function (settings) {
                    //     // console.log(settings)
                    //     const windowHeight = window.innerHeight;
                    //     const tableOffset = table.getBoundingClientRect().top;
                    //     const availableHeight = windowHeight - tableOffset;
                    //     // console.log(availableHeight);
                    //     settings.nScrollBody.style.height = `${availableHeight}px`;
                    //     settings.nScrollBody.style.maxHeight = `${availableHeight}px`;
                    // }
                });

                const thisClientListTable = this;
                dataTable.on('select', function (e, dt, type, indexes) {
                    this.RightPanel.hide();
                    if (type === 'row') {
                        const data = dataTable
                            .rows(indexes)
                            .data()
                        const clientInfoView = new ClientInfoView(data, thisClientListTable, indexes);
                        this.RightPanel.resetContent();
                        this.RightPanel.setPanelContent(clientInfoView.render());
                        requestAnimationFrame(() => {
                            this.RightPanel.show();
                        });
                    }
                }.bind(this));
                dataTable.on('deselect', function (e, dt, type, indexes) {
                    this.RightPanel.hide();
                }.bind(this));

                this.dataTable = dataTable;

            };
            script.onerror = () => {
                console.error('Failed to load DataTables script.');
            };
            shadowRoot.appendChild(script);
        } else {
            console.error('Table not found in shadowRoot.');
        }

        document.addEventListener('keydown', (e) => {
            if (e.key === 'Escape') {
                if (!top.document.querySelector('#icon-selector-panel')) {
                    this.RightPanel.hide();
                    this.dataTable.rows().deselect();
                } else {
                    top.document.querySelector('#icon-selector-panel').remove();
                }
            }
        });

        setInterval(() => {
            this.dataTable.rows().invalidate().draw();
        }, 2000);
    }

    isSwMode = (mode) => {
        let ui_sw_mode = "rt";
        const sw_mode = '<% nvram_get("sw_mode"); %>';
        const wlc_psta = '<% nvram_get("wlc_psta"); %>' == '' ? 0 : '<% nvram_get("wlc_psta"); %>';
        const wlc_express = '<% nvram_get("wlc_express"); %>' == '' ? 0 : '<% nvram_get("wlc_express"); %>';

        if (((sw_mode == '2' && wlc_psta == '0') || (sw_mode == '3' && wlc_psta == '2')) && wlc_express == '0') {   // Repeater
            ui_sw_mode = "re";
        } else if ((sw_mode == '3' && wlc_psta == '0') || (sw_mode == '3' && wlc_psta == '')) { // Access Point
            ui_sw_mode = "ap";
        } else if ((sw_mode == '3' && wlc_psta == '1' && wlc_express == '0') || (sw_mode == '3' && wlc_psta == '3' && wlc_express == '0') || (sw_mode == '2' && wlc_psta == '1' && wlc_express == '0')) {   // MediaBridge
            ui_sw_mode = "mb";
        } else if (sw_mode == '2' && wlc_psta == '0' && wlc_express == '1') {   // Express Way 2G
            ui_sw_mode = "ew2";
        } else if (sw_mode == '2' && wlc_psta == '0' && wlc_express == '2') {   // Express Way 5G
            ui_sw_mode = "ew5";
        } else if (sw_mode == '5') {    // Hotspot
            ui_sw_mode = 'hs';
        } else ui_sw_mode = "rt"; // Router

        return (ui_sw_mode.search(mode) !== -1);
    }


    renderClientsInfo = () => {

        const client_convRSSI = function (val) {
            let result = 1;
            val = parseInt(val);
            if (val >= -50) result = 4;
            else if (val >= -80) result = Math.ceil((24 + ((val + 80) * 26) / 10) / 25);
            else if (val >= -90) result = Math.ceil((((val + 90) * 26) / 10) / 25);
            else return 1;

            if (result == 0) result = 1;
            return result;
        };

        const isWL_map = {
            "0": {
                "text": "Wired",
                "type": "eth",
                "idx": 1
            },
            "1": {
                "text": "2.4G",
                "type": "2g",
                "idx": 1
            },
            "2": {
                "text": "5G",
                "type": "5g",
                "idx": 1
            },
            "3": {
                "text": "5G",
                "type": "5g",
                "idx": 2
            },
            "4": {
                "text": "6G",
                "type": "6g",
                "idx": 1
            },
            "5": {
                "text": "6G",
                "type": "6g",
                "idx": 2
            }
        };

        const wl_band_count = (function(){
            const wl_nband_array = "<% wl_nband_info(); %>".toArray();
            const counts = {};
            for(let i = 0; i < wl_nband_array.length; i++){
                const band_text = (function(wl_band){
                    if(wl_band == "2")
                        return "2g";
                    else if(wl_band == "1")
                        return "5g";
                    else if(wl_band == "4")
                        return "6g";
                })(wl_nband_array[i]);
                counts[band_text] = (counts[band_text] + 1) || 1;
            }
            return counts;
        })();

        const vendorArrayRE = /(adobe|amazon|apple|asus|belkin|bizlink|buffalo|dell|d-link|fujitsu|google|hon hai|htc|huawei|ibm|lenovo|nec|microsoft|panasonic|pioneer|ralink|samsung|sony|synology|toshiba|tp-link|vmware)/;

        function getVendorIconClassName(vendorName) {
            let vendor_class_name = "";
            const match_data = vendorName.match(vendorArrayRE);
            if (Boolean(match_data) && match_data[0] != undefined) {
                vendor_class_name = match_data[0];
                if (vendor_class_name == "hon hai")
                    vendor_class_name = "honhai";
            } else {
                vendor_class_name = "";
            }
            return vendor_class_name;
        }

        this.clients.forEach(client => {
            let icon_type;

            //Interface
            let clientInterfaceCode = '';
            if (!(this.isSwMode('mb') || this.isSwMode('ew'))) {
                let rssi_t = 0;
                if (client.isWL == "0")
                    rssi_t = "wired";
                else
                    rssi_t = "wireless";
                clientInterfaceCode = `<div class='interface_container'><div class='radio-icon radio-${rssi_t}'></div><div class="client-status">${(client.isOnline) ? `<i class='connect-status online'></i>` : `<i class='connect-status offline'></i>`}</div></div>`;
            }

            //Icon
            let clientIconCode = '';
            clientIconCode += `<div class='client_icon'>`;

            if (client.uploadIcon !== "NoIcon") {
                clientIconCode += "<div title='" + client.vendor + "'>";
                if (client.isUserUploadImg) {
                    clientIconCode += '<img class="imgUserIcon" src="' + client.uploadIcon + '">';
                } else {
                    clientIconCode += '<div class="imgUserIcon"><i class="type" style="--svg:url(' + client.uploadIcon + ')"></i></div>';
                }
                clientIconCode += "</div>";
            } else if (client.type != "0" || client.vendor == "") {
                icon_type = "type" + client.type;
                clientIconCode += `<div style="cursor:default;" class="clientIcon_no_hover"><i class="${icon_type}" style="--svg:url(${this.iconList.getIconByType(client.type).src})"></i>`;
                if (client.type == "36")
                    clientIconCode += "<div class='flash'></div>";
                clientIconCode += "</div>";
            } else if (client.vendor != "") {
                const vendorIconClassName = getVendorIconClassName(client.vendor.toLowerCase());
                if (vendorIconClassName != "" && !isSupport("sfp4m")) {
                    clientIconCode += `<div class='vendorIcon_no_hover' title='${client.deviceTypeName}'><i class='vendor-icon ${vendorIconClassName}'></i></div>`;
                } else {
                    icon_type = "type" + client.type;
                    clientIconCode += `<div class='clientIcon_no_hover' title='${client.deviceTypeName}'><i class='${icon_type}' style="--svg:url(${this.iconList.getIconByType(client.type).src})"></i></div>`;
                }
            }
            clientIconCode += `</div>`;

            const ipState = {
                "Static": "<#BOP_ctype_title5#>",
                "DHCP": "<#BOP_ctype_title1#>",
                "Manual": "<#Clientlist_IPMAC_Binding#>",
                "OffLine": "<#Clientlist_OffLine_Hint#>"
            };

            let displayAccessAP = (typeof client.cap_model_name !== "undefined") ? `${client.cap_model_name}` : "-";
            client.displayName = (client.nickName !== '') ? client.nickName : client.name;
            client.displayState = (client.isOnline) ? `<i class='connect-status online'></i>` : `<i class='connect-status offline'></i>`;
            client.displayClientInterface = clientInterfaceCode;
            client.displayClientIcon = clientIconCode;
            client.displayPhySpeed = `${client.db_sta_tx || '-'} / ${client.db_sta_rx || '-'}`;
            client.displayAccessAP = displayAccessAP;
            client.displayIpMethod = ipState[client.ipMethod];
            client.mobile = `
                <div class="client_info">
                    <div class="client_info_main">
                        <div class="client_info_text">
                            <div class="client-name">${client.name}</div>
                            <div>Speed (TX/RX) ↑20 Mbps ↓100Mbps</div>
                        </div>
                        <div class="client_info_right">
                            ${clientIconCode}                            
                            ${clientInterfaceCode}
                        </div>
                    </div>
                </div>
            `;

            const getConnectType = (type) => {
                const isWL = String(client.isWL);
                if (isWL === "0") return type === "display" ? `<#wan_ethernet#>` : `wired`;
                if (isSupport("mlo") && client.mlo) return type === "display" ? `<#WiFi_mlo_title#>` : `mlo`;
                const wl_item = isWL_map[isWL];
                if (!wl_item || !wl_item.type) return '';
                const bandCount = wl_band_count[wl_item.type];
                if (type === "display") {
                    return bandCount !== undefined && bandCount > 1
                        ? `${(wl_item.type).toUpperCase()}Hz-${wl_item["idx"]}`
                        : `${(wl_item.type).toUpperCase()}Hz`;
                } else {
                    return bandCount !== undefined && bandCount > 1
                        ? `wl${(wl_item.type).toLowerCase()}${wl_item["idx"]}`
                        : `wl${(wl_item.type).toLowerCase()}`;
                }
            };
            client.displayConnectType = getConnectType("display");
            client.classConnectType = getConnectType("class");
        })
    }

    reloadClientListData = async () => {
        await this.clientList.reloadClientList();
        this.clients = this.clientList.listClients(0);
        await this.renderClientsInfo();
        await this.reloadTable();
    }

    reloadTable = () => {
        if (top.document.querySelector('#ClientInfoView')) {
            top.document.querySelector('#ClientInfoView').remove();
        }

        if (this.interfaceShow === 'all') {
            this.clients = this.clientList.listClients(0, this.statusShow)
        } else if (this.interfaceShow === 'wireless') {
            this.clients = this.clientList.listOnlineWirelessClients(this.statusShow)
        } else if (this.interfaceShow === 'wire') {
            this.clients = this.clientList.listOnlineWireClients(this.statusShow)
        }

        this.dataTable?.clear().rows.add(this.clients).draw();

        this.element.shadowRoot.querySelector('#interface_switch .segmented_picker_option[data-value="all"] .block_filter_name').innerText = `<#All#> (${this.clientList.listClients(0, this.statusShow).length})`
        this.element.shadowRoot.querySelector('#interface_switch .segmented_picker_option[data-value="wireless"] .block_filter_name').innerText = `<#tm_wireless#> (${this.clientList.listOnlineWirelessClients(this.statusShow).length})`
        this.element.shadowRoot.querySelector('#interface_switch .segmented_picker_option[data-value="wire"] .block_filter_name').innerText = `<#tm_wired#> (${this.clientList.listOnlineWireClients(this.statusShow).length})`

        if (typeof this.clientFilterView !== 'undefined') {
            this.clientFilterView.reloadFilterView();
        }
    }

    render() {
        return this.element
    }


}

export class ClientInfoView {
    constructor(rowData, ClientListTable, rowIndex) {
        const client = rowData[0];
        this.client = client;
        // console.log(client)
        this.payload = {
            ip: client.ip,
            name: client.name,
        };

        this.iconChange = false;
        this.eventHandlers = new Map(); // Track event handlers for cleanup
        this.domCache = new Map(); // Cache DOM queries

        const iconList = new ClinetIconFetcher();
        iconList.fetchIconList();
        this.iconList = iconList;

        if (client.amesh_isRe) {
            this.fetchClientDB(client.mac);
        }

        const nvram_fetch = httpApi.nvramCharToAscii(["custom_clientlist", "MULTIFILTER_ALL", "MULTIFILTER_ENABLE", "MULTIFILTER_MAC", "MULTIFILTER_DEVICENAME", "MULTIFILTER_MACFILTER_DAYTIME_V2", "MULTIFILTER_MACFILTER_DAYTIME"], true)
        this.custom_clientlist = decodeURIComponent(nvram_fetch.custom_clientlist).replace(/&#62/g, ">").replace(/&#60/g, "<");
        this.MULTIFILTER_ALL = decodeURIComponent(nvram_fetch.MULTIFILTER_ALL).replace(/&#62/g, ">").replace(/&#60/g, "<");
        this.MULTIFILTER_ENABLE = decodeURIComponent(nvram_fetch.MULTIFILTER_ENABLE).replace(/&#62/g, ">").replace(/&#60/g, "<");
        this.MULTIFILTER_MAC = decodeURIComponent(nvram_fetch.MULTIFILTER_MAC).replace(/&#62/g, ">").replace(/&#60/g, "<");
        this.MULTIFILTER_DEVICENAME = decodeURIComponent(nvram_fetch.MULTIFILTER_DEVICENAME).replace(/&#62/g, ">").replace(/&#60/g, "<");
        this.MULTIFILTER_MACFILTER_DAYTIME = (function () {
            if (isSupport("PC_SCHED_V3"))
                return decodeURIComponent(nvram_fetch.MULTIFILTER_MACFILTER_DAYTIME_V2).replace(/&#62/g, ">").replace(/&#60/g, "<");
            else
                return decodeURIComponent(nvram_fetch.MULTIFILTER_MACFILTER_DAYTIME).replace(/&#62/g, ">").replace(/&#60/g, "<");
        })();

        const div = document.createElement('div');
        div.id = "ClientInfoView";
        const template = document.createElement('template');


        // Extract reusable client icon HTML
        const clientIconHtml = this.generateClientIconHtml(client);

        const clientSettingHtml = `
                                <div class="info-summary">
                                    <div class="d-flex justify-content-center align-items-center gap-3">
                                        <div class="info-summary-icon-edit">
                                            <div class="info-summary-icon">
                                                <div class="client_icon">
                                                    ${clientIconHtml}
                                                </div>
                                            </div>
                                            <div class="edit-icon"><i class="icon-edit"></i></div>
                                        </div>
                                        <div class="d-flex flex-column gap-1">
                                            <div class="title">${client.displayName}</div>
                                            <div class="mac">${client.mac}</div>
                                            ${client.showConnectTo ? `
                                            <div class="desc"><#Connected_to#>
                                                <div>${client.parentApName}</div>
                                            </div>` : ``}
                                        </div>
                                    </div>
                                </div>
                                <div class="info-card">
                                    <div class="info-card-body">
                                        <div class="field">
                                            <div class="d-flex flex-column gap-1">
                                                <div class="title"><#Clientlist_name#></div>
                                                <div><input id="ClientName" class="form-control" value="${client.displayName}"></div>
                                            </div>
                                        </div>
                                    </div>
                                </div>
                                ${client.isOnline ? `
                                <div class="info-card">
                                    <div class="info-card-body">
                                        <div class="field">
                                            <div class="d-flex flex-column gap-1">
                                                <div class="title"><#Client_IP#></div>
                                                <div><input id="ClientIp" class="form-control" value="${client.ip}"></div>
                                            </div>
                                        </div>
                                        <div class="hr"></div>                                    
                                        <div class="field">
                                            <div class="d-flex flex-row justify-content-between">
                                                <div class="title"><#Clientlist_IPMAC_Binding#></div>
                                                <div id="BindIP"></div>
                                            </div>
                                        </div>
                                        <div class="hr"></div>
                                        ${(client.showBlockInternet) ? `<div class="field">
                                            <div class="d-flex flex-row justify-content-between">
                                                <div class="title"><#Clientlist_block_internet#></div>
                                                <div id="BlockInternetAccessSwitch"></div>
                                            </div>
                                        </div>
                                        <div class="hr"></div>` : ``}
                                        ${(client.showTimeScheduling) ? `<div class="field">
                                            <div class="d-flex flex-row justify-content-between">
                                                <div class="title">
                                                    <#Time_Scheduling#>
                                                    <div id="TimeSchedulingEdit" class="title-edit"><i class="icon-edit"></i></div>
                                                </div>
                                                <div id="TimeSchedulingSwitch"></div>
                                            </div>
                                        </div>` : ``}
                                    </div>
                                </div>` : ''}
                                <div class="d-grid gap-2">
                                  <button id="ClientSettingConfirm" class="btn btn-primary" type="button"><#CTL_apply#></button>
                                </div>`;

        const meshNodeSettingHtml = `
                                <div class="info-summary">
                                    <div class="d-flex justify-content-center align-items-center gap-3">
                                        <div class="info-summary-icon">
                                            <div class="client_icon">
                                                ${clientIconHtml}
                                            </div>
                                        </div>
                                        <div class="d-flex flex-column gap-1">
                                            <div class="title">${client.displayName}</div>
                                            <div class="mac">${client.mac}</div>
                                            ${client.showConnectTo ? `
                                            <div class="desc"><#Connected_to#>
                                                <div>${client.parentApName}</div>
                                            </div>` : ``}
                                        </div>
                                    </div>
                                </div>
                                <div class="d-grid gap-2">
                                    <button id="NodeManagementLink" class="btn btn-primary" type="button"><#AiMesh_Management#></button>
                                </div>
                                `;


        template.innerHTML = `
            <div class="client-info-popup">
                <div class="client-info-header">
                    <div class="close"><svg width="24" height="24" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg"><path d="M18 5.91L6 18.09M6 5.91l12 12.18" stroke="#000" stroke-width="1.2" stroke-linecap="round" stroke-linejoin="round"/></svg></div>
                    <div class="title">${client.displayName}</div>
                </div>
                <div class="client-info-nav">
                    <div class="info-nav-tabs" id="nav-tab">
                        <a class="info-nav-item active" data-toggle="tab" data-target="overview"><i class="icon overview"></i><#Overview#></a>
                        ${client.is_wireless ? `
                        <a class="info-nav-item" data-toggle="tab" data-target="statistic"><i class="icon promotion"></i><#Statistic#></a>
                        ` : ``}
                        <a class="info-nav-item" data-toggle="tab" data-target="settings"><i class="icon settings"></i><#Settings#></a>
                    </div>
                </div>
                <div class="client-info-body">
                    <div class="tab-content">
                        <div class="tab-panel fade show active" id="overview">
                            <div class="info-summary">
                                <div class="d-flex justify-content-center align-items-center gap-3">
                                    <div class="info-summary-icon">
                                        <div class="client_icon">
                                            ${clientIconHtml}
                                        </div>
                                    </div>
                                    <div class="d-flex flex-column gap-1">
                                        <div class="title">${client.displayName}</div>
                                        <div class="mac">${client.mac}</div>
                                        ${client.showConnectTo ? `
                                        <div class="desc"><#Connected_to#>
                                            <div>${client.parentApName}</div>
                                        </div>` : ``}
                                    </div>
                                </div>
                            </div>
                            <div class="info-card">
                                <div class="info-card-body gap-3">
                                    <div class="d-flex flex-column p-3 gap-2">
                                        <div class="d-flex align-items-center justify-content-evenly gap-1 fw-bold">
                                            <div class="d-flex align-items-center gap-2 fw-bold">
                                                <i class="connect-status ${client.isOnline ? 'online' : 'offline'}"></i>${client.isOnline ? '<#Clientlist_Online#>' : '<#Clientlist_Offline#>'}
                                            </div>
                                            <div class="vr"></div>
                                            <div class="d-flex align-items-center gap-2">
                                                <div class="access-time-title"><#Access_Time#></div><div class="access-time-value">${ClientListTable.dataTable.cell(rowIndex, rowData.column('access_time:name').index()).node().innerHTML}</div>
                                            </div>
                                        </div>
                                    </div>
                                </div>
                            </div>
                        </div>
                        <div class="tab-panel fade" id="statistic">
                            <div class="info-summary">
                                <div class="d-flex justify-content-center align-items-center gap-3">
                                    <div class="info-summary-icon">
                                        <div class="client_icon">
                                            ${clientIconHtml}
                                        </div>
                                    </div>
                                    <div class="d-flex flex-column gap-1">
                                        <div class="title">${client.displayName}</div>
                                        <div class="mac">${client.mac}</div>
                                        ${client.showConnectTo ? `
                                        <div class="desc"><#Connected_to#>
                                            <div>${client.parentApName}</div>
                                        </div>` : ``}
                                    </div>
                                </div>
                            </div>
                            <div id="TrafficChartCard" class="info-card">
                                <div class="info-card-body">
                                    <div class="field">
                                        <div class="d-flex flex-column gap-1">
                                            <div class="title">Traffic Activities</div>
                                            <div><canvas id="traffic_chart"></canvas></div>
                                        </div>
                                    </div>
                                </div>
                            </div>
                            <div id="RssiChartCard" class="info-card">
                                <div class="info-card-body">
                                    <div class="field">
                                        <div class="d-flex flex-column gap-1">
                                            <div class="title">RSSI</div>
                                            <div><canvas id="rssi_chart"></canvas></div>
                                        </div>
                                    </div>
                                </div>
                            </div>
                            <div id="SystemChartCard" class="info-card">
                                <div class="info-card-body">
                                    <div class="field">
                                        <div class="d-flex flex-column gap-1">
                                            <div class="title">CPU / Memory</div>
                                            <div><canvas id="system_chart"></canvas></div>
                                        </div>
                                    </div>
                                </div>
                            </div>
                        </div>
                        <div class="tab-panel fade" id="settings">
                            ${client.amesh_isRe ? meshNodeSettingHtml : clientSettingHtml}
                        </div>
                    </div>
                </div>
            </div>
        `
        div.appendChild(template.content.cloneNode(true));

        this.element = div;
        this.popup = div.querySelector('.client-info-popup');


        const editIconElement = div.querySelector('.edit-icon')?.closest(".info-summary-icon-edit");
        if (editIconElement) {
            this.addEventHandler(editIconElement, 'click', () => {
                const iconSelectorPanel = new IconSelectorPanel(this, iconList);
                if (!top.document.querySelector('#icon-selector-panel')) {
                    top.document.body.appendChild(iconSelectorPanel.render());
                }
            });
        }

        const closeButton = div.querySelector('.client-info-header .close');
        if (closeButton) {
            this.addEventHandler(closeButton, 'click', () => {
                this.close()
                ClientListTable.dataTable.rows().deselect();
            });
        }
        const navTabs = div.querySelector('.info-nav-tabs');
        if (navTabs) {
            this.addEventHandler(navTabs, 'click', (e) => {
                const target = e.target;
                if (target.classList.contains('info-nav-item')) {
                    const tab = target.getAttribute('data-target');
                    const tabs = div.querySelectorAll('.info-nav-item');
                    tabs.forEach(tab => {
                        tab.classList.remove('active')
                    })
                    target.classList.add('active')
                    const tabContents = div.querySelectorAll('.tab-panel');
                    tabContents.forEach(tabContent => {
                        tabContent.classList.remove('active', 'show')
                    })
                    div.querySelector(`#${tab}`).classList.add('active', 'show')
                }
            });
        }

        const mainInfo = [
            {
                title: `<#Client_IP#>`, value: client.ip, order: 1, render: function () {
                    return `<div class="d-flex align-items-center gap-2">
                                <div>${client.ip}</div>
                                <div class="ip-method">${client.ipMethod}</div>
                            </div>`
                }
            },
            {title: `<#MAC_Address#>`, value: client.mac, order: 2},
            {title: `<#AiMesh_NodeLocation#>`, value: client.location, order: 3},
            {title: `<#Connection_Type#>`, value: (() => {
                switch (String(client.isWL)) {
                    case "0":
                        return `<#tm_wired#>`;
                    default:
                        return `<#tm_wireless#>`;
                }
            })(), order: 4},
            // {title: "Hostname", value: "ASUS", order: 3},
            // {title: "Manufacturer", value: "ASUS", order: 4},
            // {title: "Model", value: "ROG Phone3", order: 5},
            // {title: "OS", value: "Android", order: 6},
        ]

        const mainInfoCard = new infoCard({type: 'table', title: '', data: mainInfo})
        div.querySelector('#overview').appendChild(mainInfoCard.render())

        const connectionStatus = [];
        if (client.displayConnectType) {
            connectionStatus.push({title: `WiFi <#Interface#>`, value: client.displayConnectType, order: 1});
        }
        connectionStatus.push(
            {title: `<#PHY_Speed_TXRX#>`, value: client.displayPhySpeed, order: 2},
            {
                title: `<#Quality_RSSI#>`, value: client.rssi, order: 3, render: function () {
                    if (typeof client.rssi === 'undefined') {
                        return "-"
                    } else {
                        return `${client.rssi} dbm`
                    }
                }
            }
            // {title: "<#Access_Time#>", value: client.displayAccessTime, order: 5},
            // {title: "Access AP", value: client.displayAccessAP, order: 6},
        );
        const connectionStatusCard = new infoCard({type: 'table', title: `<#Clinet_Connection_status#>`, data: connectionStatus})

        const clientNameInput = div.querySelector('#ClientName');
        let clientIpInput;
        let ipBindingSwitch;
        let timeSchedulingSwitch;
        let blockInternetAccessSwitch;
        let timeSchedulingEdit;
        let NodeManagementLink;

        if (client.isOnline && !client.amesh_isRe) {
            ipBindingSwitch = new ToggleButton(this.client.ipBindingFlag);
            blockInternetAccessSwitch = new ToggleButton(this.client.internetMode === "block");

            function redirectTimeScheduling(_mac) {
                window.localStorage.setItem("time_scheduling_mac", _mac, 1);
                pageRedirect("settings", "ParentalControl.asp");
            }

            timeSchedulingSwitch = new ToggleButton(this.client.internetMode === "time");
            if (client.is_wireless) {
                div.querySelector('#overview').appendChild(connectionStatusCard.render())
            }
            div.querySelector('#BindIP').appendChild(ipBindingSwitch.render())
            div.querySelector('#BlockInternetAccessSwitch')?.appendChild(blockInternetAccessSwitch.render())
            div.querySelector('#TimeSchedulingSwitch')?.appendChild(timeSchedulingSwitch.render())
            timeSchedulingEdit = div.querySelector('#TimeSchedulingEdit');
            timeSchedulingEdit?.addEventListener('click', () => {
                redirectTimeScheduling(this.client.mac);
            });
            if (this.client.internetMode === "time") {
                timeSchedulingEdit.classList.add('active');
            }

            ipBindingSwitch.setOnChange((value) => {
                this.client.ipBindingFlag = value;
            });

            clientIpInput = div.querySelector('#ClientIp');
            clientIpInput.addEventListener('blur', () => {
                this.client.ipBindingFlag = true;
                ipBindingSwitch.enable();
            });

            blockInternetAccessSwitch.setOnChange((value) => {
                this.client.internetMode = value ? "block" : "allow";
                if (this.client.internetMode === "block") {
                    timeSchedulingSwitch.disable();
                }
            });

            timeSchedulingSwitch.setOnChange((value) => {
                this.client.internetMode = value ? "time" : "allow";
                if (this.client.internetMode === "time") {
                    blockInternetAccessSwitch.disable();
                }
            });
        }

        if (client.amesh_isRe) {
            NodeManagementLink = div.querySelector('#NodeManagementLink');
            NodeManagementLink.addEventListener('click', () => {
                window.localStorage.setItem("node_mac", client.mac);
                pageRedirect("aimesh");
            });
        }


        const manual_dhcp_list = ClientListTable.clientList.manual_dhcp_list;

        const clientSettingConfirm = (client) => {

            const validClientListForm = function () {
                function inet_network(ip_str) {
                    if (!ip_str) return -1;
                    const re = /^(\d+)\.(\d+)\.(\d+)\.(\d+)$/;
                    if (re.test(ip_str)) {
                        const v1 = parseInt(RegExp.$1);
                        const v2 = parseInt(RegExp.$2);
                        const v3 = parseInt(RegExp.$3);
                        const v4 = parseInt(RegExp.$4);
                        if (v1 < 256 && v2 < 256 && v3 < 256 && v4 < 256)
                            return v1 * 256 * 256 * 256 + v2 * 256 * 256 + v3 * 256 + v4;
                    }
                    return -2;
                }

                const validateIpRange = function (ip_obj) {
                    let retFlag = 1
                    let ip_num = inet_network(ip_obj.value);
                    if (ip_num <= 0) {
                        alert(ip_obj.value + " <#JS_validip#>");
                        ip_obj.value = client.ip;
                        ip_obj.focus();
                        retFlag = 0;
                    } else if (client.ipBindingFlag && (ip_num <= getSubnet(`<% nvram_get("lan_ipaddr"); %>`, `<% nvram_get("lan_netmask"); %>`, "head") ||
                        ip_num >= getSubnet('<% nvram_get("lan_ipaddr"); %>', '<% nvram_get("lan_netmask"); %>', "end"))) {
                        alert(ip_obj.value + " <#JS_validip#>");
                        ip_obj.value = client.ip;
                        ip_obj.focus();
                        retFlag = 0;
                    } else if (!validator.validIPForm(ip_obj, 0)) {
                        ip_obj.value = client.ip;
                        ip_obj.focus();
                        retFlag = 0;
                    }

                    for (const existMac in manual_dhcp_list) {
                        const existIP = existMac.ip;
                        if (existIP === ip_obj.value) {
                            if (existMac !== client.mac) {
                                alert("<#JS_duplicate#>");
                                ip_obj.value = client.ip;
                                ip_obj.focus();
                                retFlag = 0;
                            }
                        }
                    }
                    return retFlag;
                }

                if (clientIpInput) {
                    if (validateIpRange(clientIpInput) === 0) {
                        return false;
                    }

                    clientNameInput.value = clientNameInput.value.trim();
                    if (clientNameInput.value.length === 0) {
                        alert("Please enter a name");
                        clientNameInput.focus();
                        clientNameInput.select();
                        clientNameInput.value = "";
                        return false;
                    } else if (clientNameInput.value.indexOf(">") !== -1 || clientNameInput.value.indexOf("<") !== -1) {
                        alert("Invalid character '<', '>'");
                        clientNameInput.focus();
                        clientNameInput.select();
                        clientNameInput.value = "";
                        return false;
                    }

                    if (isSupport("utf8_ssid")) {
                        var len = validator.lengthInUtf8(clientNameInput.value);
                        if (len > 32) {
                            alert("Username cannot be greater than 32 characters.");
                            clientNameInput.focus();
                            clientNameInput.select();
                            clientNameInput.value = "";
                            return false;
                        }
                    } else if (!validator.haveFullWidthChar(clientNameInput)) {
                        clientNameInput.focus();
                        clientNameInput.select();
                        clientNameInput.value = "";
                        return false;
                    }
                }
                return true;
            };

            if (validClientListForm()) {

                const payload = {
                    modified: 0,
                    flag: 'background',
                    action_mode: "apply",
                    action_script: "saveNvram",
                    action_wait: "1",
                    custom_usericon: ""
                }

                const custom_clientlist = this.custom_clientlist;
                const originalCustomListArray = custom_clientlist.split('<');

                const clientName = clientNameInput.value.trim();
                const clientMac = client.mac.toUpperCase();
                const onEditClient = [clientName, clientMac, 0, client.type, "", ""]

                for (let i = 0; i < originalCustomListArray.length; i++) {
                    if (originalCustomListArray[i].split('>')[1] != undefined) {
                        if (originalCustomListArray[i].split('>')[1].toUpperCase() == clientMac) {
                            onEditClient[4] = originalCustomListArray[i].split('>')[4]; // set back callback for ROG device
                            onEditClient[5] = originalCustomListArray[i].split('>')[5]; // set back keeparp for ROG device
                            const app_group_tag = originalCustomListArray[i].split('>')[6]; // for app group tag
                            if (typeof app_group_tag != "undefined") onEditClient[6] = app_group_tag;
                            const app_age_tag = originalCustomListArray[i].split('>')[7]; // for app age tag
                            if (typeof app_age_tag != "undefined") onEditClient[7] = app_age_tag;
                            const app_groupid_tag = originalCustomListArray[i].split('>')[8]; // for app groupid tag
                            if (typeof app_groupid_tag != "undefined") onEditClient[8] = app_groupid_tag;
                            originalCustomListArray.splice(i, 1); // remove the selected client from original list
                        }
                    }
                }

                payload.modified = 0;
                payload.flag = 'background';
                payload.action_mode = "apply";
                payload.action_script = "saveNvram";
                payload.action_wait = "1";

                if (this.iconChange && typeof client.selectIcon !== 'undefined') {
                    if (client.selectIcon.source === 'custom') {
                        payload.custom_usericon = `${client.mac.replace(/\:/g, "")}>${client.selectIcon.src}`;
                    } else {
                        onEditClient[3] = client.selectIcon.type;
                        payload.usericon_mac = clientMac.replace(/\:/g, "");
                        payload.custom_usericon = `${client.selectIcon.type}>${client.selectIcon.src}`;
                    }
                } else {
                    payload.custom_usericon = `${client.mac.replace(/\:/g, "")}>noupload`;
                }
                originalCustomListArray.push(onEditClient.join('>'));
                payload.custom_clientlist = originalCustomListArray.join('<');
                this.custom_clientlist = payload.custom_clientlist;


                if (client.isOnline) {

                    // IP Binding
                    let dhcp_staticlist_ori = "";
                    let dhcp_staticlist = "";
                    Object.keys(manual_dhcp_list).forEach(function (key) {
                        dhcp_staticlist_ori += "<" + key + ">" + manual_dhcp_list[key].ip + ">" + manual_dhcp_list[key].dns;
                    });

                    if (client.ipBindingFlag) {
                        if (manual_dhcp_list[clientMac] == undefined) {//new
                            const ip = clientIpInput.value;
                            const dns = "";
                            const item_para = {"ip": ip, "dns": dns};
                            manual_dhcp_list[clientMac] = item_para;
                        } else
                            manual_dhcp_list[clientMac].ip = clientIpInput.value;
                    } else
                        delete manual_dhcp_list[clientMac];

                    Object.keys(manual_dhcp_list).forEach(function (key) {
                        dhcp_staticlist += "<" + key + ">" + manual_dhcp_list[key].ip + ">" + manual_dhcp_list[key].dns;
                    });

                    if (dhcp_staticlist != dhcp_staticlist_ori) {
                        payload.dhcp_staticlist = dhcp_staticlist;
                        payload.dhcp_static_x = 1;
                        payload.action_script = "restart_net_and_phy";
                        payload.action_wait = "35";
                    }

                    //Time Scheduling
                    payload.MULTIFILTER_ALL = this.MULTIFILTER_ALL;
                    payload.MULTIFILTER_ENABLE = this.MULTIFILTER_ENABLE;
                    payload.MULTIFILTER_MAC = this.MULTIFILTER_MAC;
                    payload.MULTIFILTER_DEVICENAME = this.MULTIFILTER_DEVICENAME;
                    if (isSupport("PC_SCHED_V3")) {
                        payload.MULTIFILTER_MACFILTER_DAYTIME_V2 = this.MULTIFILTER_MACFILTER_DAYTIME;
                    } else {
                        payload.MULTIFILTER_MACFILTER_DAYTIME = this.MULTIFILTER_MACFILTER_DAYTIME;
                    }

                    if (timeSchedulingSwitch.getValue()) {
                        timeSchedulingEdit.classList.add('active');
                        payload.MULTIFILTER_ALL = "1";
                        if (payload.action_script === "restart_net_and_phy") {
                            payload.action_script += ";restart_firewall";
                        } else {
                            payload.action_script = "restart_firewall";
                        }
                    } else {
                        timeSchedulingEdit.classList.remove('active');
                    }

                    if (this.MULTIFILTER_MAC.indexOf(client.mac) === -1) {//new rule
                        if (timeSchedulingSwitch.getValue() || blockInternetAccessSwitch.getValue()) {
                            if (this.MULTIFILTER_MAC === "") {
                                if (timeSchedulingSwitch.getValue())
                                    payload.MULTIFILTER_ENABLE = "1";
                                else if (blockInternetAccessSwitch.getValue())
                                    payload.MULTIFILTER_ENABLE = "2";
                                payload.MULTIFILTER_MAC = client.mac;
                                payload.MULTIFILTER_DEVICENAME = client.name;
                                if (isSupport("PC_SCHED_V3"))
                                    payload.MULTIFILTER_MACFILTER_DAYTIME_V2 = "W03E21000700<W04122000800";
                                else
                                    payload.MULTIFILTER_MACFILTER_DAYTIME = "<";
                            } else {
                                payload.MULTIFILTER_ENABLE += ">";
                                if (timeSchedulingSwitch.getValue())
                                    payload.MULTIFILTER_ENABLE += "1";
                                else if (blockInternetAccessSwitch.getValue())
                                    payload.MULTIFILTER_ENABLE += "2";
                                payload.MULTIFILTER_MAC += ">";
                                payload.MULTIFILTER_MAC += client.mac;
                                payload.MULTIFILTER_DEVICENAME += ">";
                                payload.MULTIFILTER_DEVICENAME += client.name;
                                if (isSupport("PC_SCHED_V3"))
                                    payload.MULTIFILTER_MACFILTER_DAYTIME_V2 += ">W03E21000700<W04122000800";
                                else
                                    payload.MULTIFILTER_MACFILTER_DAYTIME += "><";
                            }
                        }
                    } else {//exist rule
                        this.MULTIFILTER_MAC.split(">").forEach(function (element, index) {
                            if (element.indexOf(client.mac) != -1) {
                                const tmpArray = payload.MULTIFILTER_ENABLE.split(">");
                                tmpArray[index] = 0;
                                if (timeSchedulingSwitch.getValue())
                                    tmpArray[index] = 1;
                                else if (blockInternetAccessSwitch.getValue())
                                    tmpArray[index] = 2;
                                payload.MULTIFILTER_ENABLE = tmpArray.join(">");
                            }
                        })
                    }
                }

                fetch('/start_apply2.htm', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/x-www-form-urlencoded'
                    },
                    body: new URLSearchParams(payload)
                }).then(response => {
                    if (response.ok) {
                        ClientListTable.reloadClientListData();
                        top.document.querySelector('#loader')?.remove();
                        top.document.querySelector('#right-panel').classList.remove('show');
                    }
                    // throw new Error('Network response was not ok.');
                }).catch(error => {
                    console.error('There has been a problem with your fetch operation:', error);
                })
            }
        }

        div.querySelector('#ClientSettingConfirm')?.addEventListener('click', () => {
            clientSettingConfirm(this.client);
        })

        const traffic_chart = div.querySelector('#traffic_chart');

        if (traffic_chart) {

            let myChart = null;

            const ctx = traffic_chart.getContext('2d');
            const root = document.documentElement;

            const reloadChartColor = (data, config) => {
                const txBorderColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-tx-border-color').trim();
                const txFillStartColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-tx-fill-start-color').trim();
                const txFillEndColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-tx-fill-end-color').trim();

                const rxBorderColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-rx-border-color').trim();
                const rxFillStartColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-rx-fill-start-color').trim();
                const rxFillEndColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-rx-fill-end-color').trim();

                const xLabelColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-x-label-color').trim();
                const xGridColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-x-grid-color').trim();
                const yLabelColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-y-label-color').trim();
                const yGridColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-y-grid-color').trim();

                const legendLabelColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-legend-label-color').trim();

                const gradientTX = ctx.createLinearGradient(0, 0, 0, 400);
                gradientTX.addColorStop(0, txFillStartColor);
                gradientTX.addColorStop(1, txFillEndColor);

                const gradientRX = ctx.createLinearGradient(0, 0, 0, 400);
                gradientRX.addColorStop(0, rxFillStartColor);
                gradientRX.addColorStop(1, rxFillEndColor);

                if (data && config) {
                    data.datasets[0].backgroundColor = gradientTX;
                    data.datasets[1].backgroundColor = gradientRX;
                    data.datasets[0].borderColor = txBorderColor;
                    data.datasets[1].borderColor = rxBorderColor;
                    config.options.scales.x.ticks.color = xLabelColor;
                    config.options.scales.x.grid.color = xGridColor;
                    config.options.scales.y.ticks.color = yLabelColor;
                    config.options.scales.y.grid.color = yGridColor;
                    config.options.plugins.legend.labels.color = legendLabelColor;
                } else {
                    myChart.data.datasets[0].backgroundColor = gradientTX;
                    myChart.data.datasets[1].backgroundColor = gradientRX;
                    myChart.data.datasets[0].borderColor = txBorderColor;
                    myChart.data.datasets[1].borderColor = rxBorderColor;
                    myChart.options.scales.x.ticks.color = xLabelColor;
                    myChart.options.scales.x.grid.color = xGridColor;
                    myChart.options.scales.y.ticks.color = yLabelColor;
                    myChart.options.scales.y.grid.color = yGridColor;
                    myChart.options.plugins.legend.labels.color = legendLabelColor;
                }
            }

            const observer = new MutationObserver(() => {
                reloadChartColor();
                myChart.update();
            });

            observer.observe(top.document.documentElement, {
                attributes: true,
                attributeFilter: ["data-asuswrt-color", "data-asuswrt-theme"]
            });

            const data = {
                // labels: [],
                datasets: [{
                    label: 'Tx Data',
                    data: [],
                    borderWidth: 1,
                    fill: true
                }, {
                    label: 'Rx Data',
                    data: [],
                    borderWidth: 1,
                    fill: true
                }]
            };

            client.realtime_data.forEach((item, index) => {
                const date = new Date(parseInt(item.data_time) * 1000);
                const year = date.getFullYear();
                const month = String(date.getMonth() + 1).padStart(2, '0'); // getMonth() 返回 0-11，所以加 1
                const day = String(date.getDate()).padStart(2, '0');
                const hours = String(date.getHours()).padStart(2, '0');
                const minutes = String(date.getMinutes()).padStart(2, '0');
                const seconds = String(date.getSeconds()).padStart(2, '0');
                const formattedDate = `${year}-${month}-${day} ${hours}:${minutes}:${seconds}`;
                data.datasets[0].data.push({x: formattedDate, y: parseFloat(item.sta_tx)});
                data.datasets[1].data.push({x: formattedDate, y: parseFloat(item.sta_rx)});
            });

            const config = {
                type: 'line',
                data: data,
                options: {
                    animation: {
                        duration: 0
                    },
                    elements: {
                        point: {
                            radius: 0
                        },
                        line: {
                            tension: 0.1
                        }
                    },
                    responsive: true,
                    interaction: {
                        mode: 'index',
                        intersect: false
                    },
                    scales: {
                        x: {
                            type: 'time',
                            time: {
                                unit: 'minute',
                                displayFormats: {
                                    minute: 'HH:mm',
                                    hour: 'HH:mm',
                                    day: 'YYYY-MM-DD'
                                }
                            },
                            ticks: {
                                maxTicksLimit: 5,
                                autoSkip: true,
                                maxRotation: 0,
                                minRotation: 0,
                                callback: function (value) {
                                    return value;
                                },
                            },
                            grid: {}
                        },
                        y: {
                            ticks: {},
                            grid: {}
                        }
                    },
                    plugins: {
                        legend: {
                            display: true,
                            position: 'bottom',
                            align: 'start',
                            labels: {
                                usePointStyle: false,
                                boxWidth: 12,
                                boxHeight: 12,
                                padding: 10
                            }
                        }
                    }
                }
            };

            reloadChartColor(data, config);

            setTimeout(() => {
                myChart = new Chart(ctx, config);
            }, 1000)
        }

        if (client.isWL) {
            const rssi_chart = div.querySelector('#rssi_chart');
            if (rssi_chart) {

                let myChart = null;

                const ctx = rssi_chart.getContext('2d');
                const root = document.documentElement;

                const reloadChartColor = (data, config) => {
                    const rssiBorderColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-tx-border-color').trim();
                    const rssiFillStartColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-tx-fill-start-color').trim();
                    const rssiFillEndColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-tx-fill-end-color').trim();

                    const xLabelColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-x-label-color').trim();
                    const xGridColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-x-grid-color').trim();
                    const yLabelColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-y-label-color').trim();
                    const yGridColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-y-grid-color').trim();

                    const legendLabelColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-legend-label-color').trim();

                    const gradientRssi = ctx.createLinearGradient(0, 0, 0, 400);
                    gradientRssi.addColorStop(0, rssiFillStartColor);
                    gradientRssi.addColorStop(1, rssiFillEndColor);

                    if (data && config) {
                        data.datasets[0].backgroundColor = gradientRssi;
                        data.datasets[0].borderColor = rssiBorderColor;
                        config.options.scales.x.ticks.color = xLabelColor;
                        config.options.scales.x.grid.color = xGridColor;
                        config.options.scales.y.ticks.color = yLabelColor;
                        config.options.scales.y.grid.color = yGridColor;
                        config.options.plugins.legend.labels.color = legendLabelColor;
                    } else {
                        myChart.data.datasets[0].backgroundColor = gradientRssi;
                        myChart.data.datasets[0].borderColor = rssiBorderColor;
                        myChart.options.scales.x.ticks.color = xLabelColor;
                        myChart.options.scales.x.grid.color = xGridColor;
                        myChart.options.scales.y.ticks.color = yLabelColor;
                        myChart.options.scales.y.grid.color = yGridColor;
                        myChart.options.plugins.legend.labels.color = legendLabelColor;
                    }
                }

                const observer = new MutationObserver(() => {
                    reloadChartColor();
                    myChart.update();
                });

                observer.observe(top.document.documentElement, {
                    attributes: true,
                    attributeFilter: ["data-asuswrt-color", "data-asuswrt-theme"]
                });

                const data = {
                    // labels: [],
                    datasets: [{
                        label: 'RSSI',
                        data: [],
                        borderWidth: 1,
                        fill: true
                    }]
                };

                client.realtime_data.forEach((item, index) => {
                    const date = new Date(parseInt(item.data_time) * 1000);
                    const year = date.getFullYear();
                    const month = String(date.getMonth() + 1).padStart(2, '0'); // getMonth() 返回 0-11，所以加 1
                    const day = String(date.getDate()).padStart(2, '0');
                    const hours = String(date.getHours()).padStart(2, '0');
                    const minutes = String(date.getMinutes()).padStart(2, '0');
                    const seconds = String(date.getSeconds()).padStart(2, '0');
                    const formattedDate = `${year}-${month}-${day} ${hours}:${minutes}:${seconds}`;
                    data.datasets[0].data.push({x: formattedDate, y: parseFloat(item.sta_rssi)});
                });

                const config = {
                    type: 'line',
                    data: data,
                    options: {
                        animation: {
                            duration: 0
                        },
                        elements: {
                            point: {
                                radius: 0
                            },
                            line: {
                                tension: 0.1
                            }
                        },
                        responsive: true,
                        interaction: {
                            mode: 'index',
                            intersect: false
                        },
                        scales: {
                            x: {
                                type: 'time',
                                time: {
                                    unit: 'minute',
                                    displayFormats: {
                                        minute: 'HH:mm',
                                        hour: 'HH:mm',
                                        day: 'YYYY-MM-DD'
                                    }
                                },
                                ticks: {
                                    maxTicksLimit: 5,
                                    autoSkip: true,
                                    maxRotation: 0,
                                    minRotation: 0,
                                    callback: function (value) {
                                        return value;
                                    },
                                },
                                grid: {}
                            },
                            y: {
                                ticks: {},
                                grid: {}
                            }
                        },
                        plugins: {
                            legend: {
                                display: true,
                                position: 'bottom',
                                align: 'start',
                                labels: {
                                    usePointStyle: false,
                                    boxWidth: 12,
                                    boxHeight: 12,
                                    padding: 10
                                }
                            }
                        }
                    }
                };

                reloadChartColor(data, config);

                setTimeout(() => {
                    myChart = new Chart(ctx, config);
                }, 1000)
            }
        } else {
            div.querySelector('#RssiChartCard').remove();
        }

        if (client.amesh_isRe) {
            const system_chart = div.querySelector('#system_chart');
            if (system_chart) {

                let myChart = null;

                const ctx = system_chart.getContext('2d');
                const root = document.documentElement;

                const reloadChartColor = (data, config) => {
                    const cpuBorderColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-tx-border-color').trim();
                    const cpuFillStartColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-tx-fill-start-color').trim();
                    const cpuFillEndColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-tx-fill-end-color').trim();

                    const memoryBorderColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-rx-border-color').trim();
                    const memoryFillStartColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-rx-fill-start-color').trim();
                    const memoryFillEndColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-rx-fill-end-color').trim();

                    const xLabelColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-x-label-color').trim();
                    const xGridColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-x-grid-color').trim();
                    const yLabelColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-y-label-color').trim();
                    const yGridColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-y-grid-color').trim();

                    const legendLabelColor = getComputedStyle(root).getPropertyValue('--clientinfo-traffic-chart-legend-label-color').trim();

                    const gradientCPU = ctx.createLinearGradient(0, 0, 0, 400);
                    gradientCPU.addColorStop(0, cpuFillStartColor);
                    gradientCPU.addColorStop(1, cpuFillEndColor);

                    const gradientMemory = ctx.createLinearGradient(0, 0, 0, 400);
                    gradientMemory.addColorStop(0, memoryFillStartColor);
                    gradientMemory.addColorStop(1, memoryFillEndColor);

                    if (data && config) {
                        data.datasets[0].backgroundColor = gradientCPU;
                        data.datasets[1].backgroundColor = gradientMemory;
                        data.datasets[0].borderColor = cpuBorderColor;
                        data.datasets[1].borderColor = memoryBorderColor;
                        config.options.scales.x.ticks.color = xLabelColor;
                        config.options.scales.x.grid.color = xGridColor;
                        config.options.scales.y.ticks.color = yLabelColor;
                        config.options.scales.y.grid.color = yGridColor;
                        config.options.plugins.legend.labels.color = legendLabelColor;
                    } else {
                        myChart.data.datasets[0].backgroundColor = gradientCPU;
                        myChart.data.datasets[1].backgroundColor = gradientMemory;
                        myChart.data.datasets[0].borderColor = cpuBorderColor;
                        myChart.data.datasets[1].borderColor = memoryBorderColor;
                        myChart.options.scales.x.ticks.color = xLabelColor;
                        myChart.options.scales.x.grid.color = xGridColor;
                        myChart.options.scales.y.ticks.color = yLabelColor;
                        myChart.options.scales.y.grid.color = yGridColor;
                        myChart.options.plugins.legend.labels.color = legendLabelColor;
                    }
                }

                const observer = new MutationObserver(() => {
                    reloadChartColor();
                    myChart.update();
                });

                observer.observe(top.document.documentElement, {
                    attributes: true,
                    attributeFilter: ["data-asuswrt-color", "data-asuswrt-theme"]
                });

                const data = {
                    // labels: [],
                    datasets: [{
                        label: 'CPU',
                        data: [],
                        borderWidth: 1,
                        fill: true
                    }, {
                        label: 'Memory',
                        data: [],
                        borderWidth: 1,
                        fill: true
                    }]
                };

                const config = {
                    type: 'line',
                    data: data,
                    options: {
                        animation: {
                            duration: 0
                        },
                        elements: {
                            point: {
                                radius: 0
                            },
                            line: {
                                tension: 0.1
                            }
                        },
                        responsive: true,
                        interaction: {
                            mode: 'index',
                            intersect: false
                        },
                        scales: {
                            x: {
                                type: 'time',
                                time: {
                                    unit: 'minute',
                                    displayFormats: {
                                        minute: 'HH:mm',
                                        hour: 'HH:mm',
                                        day: 'YYYY-MM-DD'
                                    }
                                },
                                ticks: {
                                    maxTicksLimit: 5,
                                    autoSkip: true,
                                    maxRotation: 0,
                                    minRotation: 0,
                                    callback: function (value) {
                                        return value;
                                    },
                                },
                                grid: {}
                            },
                            y: {
                                ticks: {},
                                grid: {}
                            }
                        },
                        plugins: {
                            legend: {
                                display: true,
                                position: 'bottom',
                                align: 'start',
                                labels: {
                                    usePointStyle: false,
                                    boxWidth: 12,
                                    boxHeight: 12,
                                    padding: 10
                                }
                            }
                        }
                    }
                };

                reloadChartColor(data, config);

                setTimeout(async () => {
                    await this.fetchClientDB(client.mac);
                    if (client.system_data?.length > 0) {
                        client.system_data.forEach((item, index) => {
                            const date = item.timestamp;
                            const year = date.getFullYear();
                            const month = String(date.getMonth() + 1).padStart(2, '0'); // getMonth() 返回 0-11，所以加 1
                            const day = String(date.getDate()).padStart(2, '0');
                            const hours = String(date.getHours()).padStart(2, '0');
                            const minutes = String(date.getMinutes()).padStart(2, '0');
                            const seconds = String(date.getSeconds()).padStart(2, '0');
                            const formattedDate = `${year}-${month}-${day} ${hours}:${minutes}:${seconds}`;
                            data.datasets[0].data.push({x: formattedDate, y: parseFloat(item.cpu_usage)});
                            data.datasets[1].data.push({x: formattedDate, y: parseFloat(item.mem_usage)});
                        });
                    }
                    myChart = new Chart(ctx, config);
                }, 1000)
            }
        } else {
            div.querySelector('#SystemChartCard').remove();
        }


    }

    async fetchClientDB(mac) {
        const time = Math.floor(new Date().getTime() / 1000);
        const payload = ['cpu_usage', 'mem_usage'];
        await fetch(`/get_diag_content_data.cgi?ts=${time}&duration=60&point=30&db=sys_detect&content=${payload.join('%3B')}&filter=node_mac>txt>${mac}>0`)
            .then(data => data.json())
            .then(data => {
                let currentTime = new Date();
                if (typeof data.contents !== 'undefined' && data.contents.length > 0) {
                    return data.contents.map((entry, index) => {
                        const time = new Date(currentTime.getTime() - (index * 60000));
                        return {
                            cpu_usage: entry[0],
                            mem_usage: entry[1],
                            timestamp: time
                        };
                    });
                } else {
                    return []
                }
            })
            .then(data => {
                //sort by data_time desc
                data.sort((a, b) => b.timestamp - a.timestamp);
                if (data.length > 0) {
                    const client = this.client;
                    if (client) {
                        Object.assign(client, {
                            cpu_usage: data[0].cpu_usage,
                            mem_usage: data[0].mem_usage,
                            timestamp: data[0].timestamp
                        });
                        client.system_data = data;
                    }
                }
            })

    }

    changeIcon = async (icon) => {
        this.iconChange = true;
        this.client.selectIcon = icon;
        const settingsIcon = this.getCachedElement('#settings .client_icon');

        if (!settingsIcon) return;

        if (icon.source === "custom") {
            settingsIcon.innerHTML = `<div class="clientIcon_no_hover"><img src="${icon.src}" alt="icon"></div>`;

            // Use async image processing to avoid blocking UI
            try {
                const processedDataURL = await this.processImageAsync(icon.src);
                icon.src = processedDataURL;
            } catch (error) {
                console.warn('Image processing failed:', error);
            }
        } else {
            settingsIcon.innerHTML = `<div class="clientIcon_no_hover"><i class="type${icon.type}" style="--svg:url(${icon.src})"></i></div>`;
        }
    }

    // Async image processing helper
    processImageAsync(imageSrc) {
        return new Promise((resolve, reject) => {
            const img = new Image();
            img.onload = () => {
                try {
                    const mimeType = imageSrc.split(",")[0].split(":")[1].split(";")[0];
                    const canvas = document.createElement('canvas');
                    canvas.width = 85;
                    canvas.height = 85;
                    const ctx = canvas.getContext("2d");
                    ctx.clearRect(0, 0, 85, 85);
                    ctx.drawImage(img, 0, 0, 85, 85);
                    resolve(canvas.toDataURL(mimeType));
                } catch (error) {
                    reject(error);
                }
            };
            img.onerror = () => reject(new Error('Image load failed'));
            img.src = imageSrc;
        });
    }

    // Helper method to generate reusable client icon HTML
    generateClientIconHtml(client) {
        return client.isUserUploadImg ?
            `<div class="clientIcon_no_hover"><img class="imgUserIcon" src="${client.uploadIcon}"></div>` :
            `<div class="clientIcon_no_hover"><i class="type${client.type}" ${(client.uploadIcon !== "NoIcon") ? `style="--svg:url(${client.uploadIcon})"` : `style="--svg:url(${this.iconList.getIconByType(client.type).src})"`}></i></div>`;
    }

    // Cache DOM queries to avoid repeated lookups
    getCachedElement(selector) {
        if (!this.domCache.has(selector)) {
            this.domCache.set(selector, this.element.querySelector(selector));
        }
        return this.domCache.get(selector);
    }

    // Add event listener with cleanup tracking
    addEventHandler(element, event, handler) {
        const key = `${element.constructor.name}_${event}_${Date.now()}`;
        this.eventHandlers.set(key, {element, event, handler});
        element.addEventListener(event, handler);
        return key;
    }

    // Clean up all event handlers
    cleanupEventHandlers() {
        for (const [key, {element, event, handler}] of this.eventHandlers) {
            element.removeEventListener(event, handler);
        }
        this.eventHandlers.clear();
        this.domCache.clear();
    }

    resetIcon = () => {
        this.iconChange = true;
        this.client.selectIcon = {type: this.client.defaultType, source: "local"};
        const settingsIcon = this.getCachedElement('#settings .client_icon');
        if (settingsIcon) {
            settingsIcon.innerHTML = `<div class="clientIcon_no_hover"><i class="type${this.client.defaultType}" style="--svg:url(${this.iconList.getIconByType(this.client.defaultType).src})"></i></div>`;
        }
    }

    render() {
        return this.element
    }

    show() {
        top.document.querySelector('#right-panel').classList.add('show');
    }

    hide() {
        top.document.querySelector('#right-panel').classList.remove('show');
    }

    close() {
        // Clean up event handlers and DOM cache before removing
        this.cleanupEventHandlers();
        this.element.remove();
        this.hide();
    }
}

class infoCard {
    constructor(prop) {
        this.element = document.createElement('div');
        this.element.classList.add('info-card');

        this.element.innerHTML = `
            <div class="info-card-body">
                <div class="info-card-body-container">
                    ${prop.title !== '' ? `<div class="title">${prop.title}</div>` : ''}
                    <div class="body"></div>
                </div>
            </div>
        `
        this.data = prop.data;

        this.element.querySelector('.info-card-body .body').innerHTML = this.data.sort((a, b) => a.order - b.order).map(item => {
            if (typeof item.render !== "undefined") {
                item.value = item.render();
            }

            if (typeof item.value === "undefined") {
                item.value = "-";
            }
            return `<div class="item">
                        <div class="key">${item.title}</div>
                        <div class="value">${item.value}</div>
                    </div>`
        }).join('')
    }

    render() {
        return this.element;
    }
}

export class ClientFilterView {
    constructor(ClientListTable) {
        this.dataTable = ClientListTable.dataTable;
        const div = document.createElement('div');
        this.div = div;
        div.id = "ClientFilterView";
        const template = document.createElement('template');
        template.innerHTML = `
            <div class="client-filter-popup">
                <div class="client-filter-header">
                    <div class="close"><svg width="24" height="24" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg"><path d="M18 5.91L6 18.09M6 5.91l12 12.18" stroke="#000" stroke-width="1.2" stroke-linecap="round" stroke-linejoin="round"/></svg></div>
                    <div class="title">Display Options</div>
                </div>
                <div class="client-filter-nav">
                    <div id="filter_tab" class="segmented_picker">
                        <div class="segmented_picker_option active" data-value="filters">Filters</div>
                        <div class="segmented_picker_option" data-value="columns">Columns</div>
                    </div>
                </div>
                <div class="container">
                    
                    <div id="filter_body" class="content">
                        <div id="connection_ap_filter" class="info-card">
                            <div class="info-card-container">
                                <div class="info-card-header">
                                <div class="title"><#Connected_AP#></div>
                                </div>
                                <div class="info-card-body"></div>
                            </div>
                        </div>
                        <div id="location_filter" class="info-card">
                            <div class="info-card-container">
                                <div class="info-card-header">
                                <div class="title"><#AiMesh_NodeLocation#></div>
                                </div>
                                <div class="info-card-body"></div>
                            </div>
                        </div>
                        <div id="network_sdn_filter" class="info-card">
                            <div class="info-card-container">
                                <div class="info-card-header">
                                <div class="title"><#Network_SDN#></div>
                                </div>
                                <div class="info-card-body"></div>
                            </div>
                        </div>
                        <div id="quality_filter" class="info-card">
                            <div class="info-card-container">
                                <div class="info-card-header">
                                <div class="title"><#Quality_RSSI#></div>
                                </div>
                                <div class="info-card-body">
                                    <div class="range-form">
                                        <div class="signal-bar" style="background: linear-gradient(90deg, var(--neutral-20) 0%, var(--primary-50) 0%, var(--primary-50) 100%, var(--neutral-20) 100%)">
                                            <input class="signal-bar-pointer" type="range" data-signal="min" min="-100" max="0" step="1" value="-100">
                                            <input class="signal-bar-pointer" type="range" data-signal="max" min="-100" max="0" step="1" value="0">
                                        </div>
                                        <div class="d-flex justify-content-between signal-info"><span class="signal-min">-100 dbm</span> <span class="signal-max">0 dbm</span></div>
                                    </div>
                                </div>
                            </div>
                        </div>
<!--                        <div id="security_filter" class="info-card">-->
<!--                            <div class="info-card-container">-->
<!--                                <div class="info-card-header">-->
<!--                                <div class="title">Security</div>-->
<!--                                </div>-->
<!--                                <div class="info-card-body"></div>-->
<!--                            </div>-->
<!--                        </div>-->
                        <div id="vlan_filter" class="info-card">
                            <div class="info-card-container">
                                <div class="info-card-header">
                                <div class="title">VLAN</div>
                                </div>
                                <div class="info-card-body"></div>
                            </div>
                        </div>
                        <div id="restore_filter" class="restore_btn">Restore filters</div>
                    </div>
                    <div id="column_body" class="content" style="display: none">
                        <div class="info-card">
                            <div class="info-card-container">
                                <div class="info-card-body">
                                    <div id="column-form" class="d-flex flex-column gap-2">
                                    </div>
                                </div>
                            </div>
                        </div>
                        <div id="uncheck_all_column" class="restore_btn">Uncheck All</div>
                        <div id="restore_column" class="restore_btn">Restore columns</div>
                    </div>
               </div>
            </div>
        `
        div.appendChild(template.content.cloneNode(true));
        this.element = div;
        this.popup = div.querySelector('.client-filter-popup');

        this.signalRange = {
            min: {value: -100, width: 0},
            max: {value: 0, width: 100}
        };

        div.querySelector('.client-filter-header .close').addEventListener('click', () => {
            this.close()
        })

        div.querySelector('#filter_tab').addEventListener('click', (e) => {
            if (e.target.classList.contains('segmented_picker_option')) {
                div.querySelectorAll('.segmented_picker_option').forEach((el) => {
                    el.classList.remove('active')
                })
                e.target.classList.add('active')
                const filter = e.target.dataset.value;
                if (filter === 'columns') {
                    div.querySelector('#filter_body').style.display = 'none';
                    div.querySelector('#column_body').style.display = 'block';
                } else {
                    div.querySelector('#filter_body').style.display = 'block';
                    div.querySelector('#column_body').style.display = 'none';
                }
            }
        });

        this.getSignalWidth = function (signalRange) {
            const range = Math.abs(signalRange.max.value - signalRange.min.value);
            if (range === 0) return;

            signalRange.min.width = 100 - Math.abs(signalRange.min.value);
            signalRange.max.width = 100 - Math.abs(signalRange.max.value);

            const signalBar = div?.querySelector('.signal-bar');
            if (!signalBar) return;
            div.querySelector('.signal-bar').style.background = `linear-gradient(90deg, 
                    var(--neutral-20) 0%, 
                    var(--neutral-20) ${signalRange.min.width}%, 
                    var(--primary-50) ${signalRange.min.width}%, 
                    var(--primary-50) ${signalRange.max.width}%,
                    var(--neutral-20) ${signalRange.max.width}%,  
                    var(--neutral-20) 100%`;
        }

        if (ClientListTable.filterRssi.length > 0) {
            this.signalRange = ClientListTable.filterRssi[0];
            this.getSignalWidth(this.signalRange);
            div.querySelector('.signal-min').innerText = `${parseInt(this.signalRange.min.value)} dbm`;
            div.querySelector('.signal-max').innerText = `${parseInt(this.signalRange.max.value)} dbm`;
            div.querySelector('.signal-bar-pointer[data-signal="min"]').value = this.signalRange.min.value;
            div.querySelector('.signal-bar-pointer[data-signal="max"]').value = this.signalRange.max.value;
        }

        div.querySelectorAll('.signal-bar-pointer').forEach(pointer => {
            pointer.addEventListener('input', function (e) {
                const targetElement = e.currentTarget;
                const signal = targetElement.dataset.signal;
                if (signal === 'max') {
                    if (targetElement.value < this.signalRange.min.value) {
                        targetElement.value = this.signalRange.min.value;
                    }
                }
                if (signal === 'min') {
                    if (targetElement.value > this.signalRange.max.value) {
                        targetElement.value = this.signalRange.max.value;
                    }
                }
                this.signalRange[signal].value = Number(targetElement.value);
                div.querySelector(`.signal-${signal}`).innerText = `${parseInt(this.signalRange[signal].value)} dbm`;
                this.getSignalWidth(this.signalRange);
                if (ClientListTable.filterRssi.length > 0) {
                    ClientListTable.filterRssi[0] = this.signalRange;
                } else {
                    ClientListTable.filterRssi.push(this.signalRange);
                }

                ClientListTable.dataTable.column('Quality(RSSI):title').search(
                    data => {
                        const min = this.signalRange.min.value;
                        const max = this.signalRange.max.value;
                        const rssi = parseFloat(data);
                        return rssi >= min && rssi <= max;
                    });
                ClientListTable.dataTable.draw();

                reloadFilterIconColor();

            }.bind(this));
        })

        function groupByField(dataArray, field) {
            return dataArray.reduce((counts, item) => {
                if (item[field] !== "") {
                    counts[item[field]] = (counts[item[field]] || 0) + 1;
                }
                return counts;
            }, {});
        }

        function generateFilterContent(groupData, filterType, filterQuery = []) {
            if (!groupData) return 'No data';

            const filterItems = filterQuery.filter(q => q.title === filterType).flatMap(q => q.items);

            // if groupData do not have filterQuery items, then add them to groupData
            if (filterQuery.length > 0) {
                for (const queryItem of filterQuery) {
                    if (queryItem.title === filterType) {
                        for (const queryItemValue of queryItem.items) {
                            if (!groupData[queryItemValue]) {
                                groupData[queryItemValue] = 0;
                            }
                        }
                    }
                }
            }

            return Object.keys(groupData).map(item => {
                const checked = filterItems.includes(item) ? 'checked' : '';
                return `
            <div class="form-check">
                <input class="form-check-input" data-filter="${filterType}" type="checkbox" value="${item}" id="${item}" ${checked}>
                <label class="form-check-label" for="${item}">
                    <div>${item}</div>
                    <div class="count">${groupData[item]}</div>
                </label>
            </div>`;
            }).join('');
        }

        function renderFilterContent() {
            const dataArray = ClientListTable.dataTable.data().toArray();

            div.querySelector('#connection_ap_filter .info-card-body').innerHTML = generateFilterContent(
                groupByField(dataArray, 'parentApName'),
                'Connected AP',
                ClientListTable.filterQuery || []
            );

            div.querySelector('#location_filter .info-card-body').innerHTML = generateFilterContent(
                groupByField(dataArray, 'location'),
                '<#AiMesh_NodeLocation#>',
                ClientListTable.filterQuery || []
            );

            div.querySelector('#network_sdn_filter .info-card-body').innerHTML = generateFilterContent(
                groupByField(dataArray, 'ssid'),
                'Network(SDN)',
                ClientListTable.filterQuery || []
            );

            // div.querySelector('#security_filter .info-card-body').innerHTML = generateFilterContent(
            //     groupByField(dataArray, 'wlAuth'),
            //     'Security',
            //     ClientListTable.filterQuery || []
            // );

            div.querySelector('#vlan_filter .info-card-body').innerHTML = generateFilterContent(
                groupByField(dataArray, 'vlan_id'),
                'VLAN',
                ClientListTable.filterQuery || []
            );

            div.querySelectorAll('.form-check-input:not([data-filter="column"])').forEach((el) => {
                el.addEventListener('change', (e) => {
                    const filterArray = ["Connected AP", `<#AiMesh_NodeLocation#>`, "VLAN", "Network(SDN)"];
                    const query = [];
                    for (const filterArrayItem of filterArray) {
                        const inputs = div.querySelectorAll(`.form-check-input[data-filter="${filterArrayItem}"]`);
                        const items = [];
                        inputs.forEach(input => {
                            if (input.checked) {
                                items.push(input.value.trim().replace('.', '\\.'));
                            }
                        });
                        if (items.length > 0) {
                            query.push({title: filterArrayItem, query: items.join("|"), items: items});
                        }
                    }
                    ClientListTable.filterQuery = query;
                    ClientListTable.dataTable.search('').columns().search('');
                    for (const queryItem of query) {
                        ClientListTable.dataTable.column(`${queryItem.title}:title`).search(queryItem.query, true, false);
                    }
                    ClientListTable.dataTable.draw();
                    reloadTable();
                })
            });
        }

        this.renderFilterContent = renderFilterContent;
        renderFilterContent();

        function reloadFilterIconColor() {
            if (ClientListTable.filterQuery.length > 0 || ClientListTable.hiddenColumns.length > 0 || ClientListTable.filterRssi.length > 0) {
                ClientListTable.element.shadowRoot.querySelector('#client_filter_btn').classList.add('active');
            } else {
                ClientListTable.element.shadowRoot.querySelector('#client_filter_btn').classList.remove('active');
            }
        }

        let defalutColumnFilter = [
            {title: `<#Vendor#>`, id: "Vendor", checked: true},
            {title: `<#Connected_AP#>`, id: "ConnectedAP", checked: true},
            {title: `<#AiMesh_NodeLocation#>`, id: "Location", checked: true},
            {title: `<#Network_SDN#>`, id: "Network", checked: true},
            {title: `<#Quality_RSSI#>`, id: "Quality", checked: true},
            {title: `<#Client_IP#>`, id: "ClientIp", checked: true},
            {title: `<#Access_Time#>`, id: "AccessTime", checked: true},
            {title: `<#AiMesh_PHY_Rate#>`, id: "PhyRate", checked: true},
            {title: `<#MAC_Address#>`, id: "MacAddress", checked: true},
            // {title: "Channel Bandwidth", id: "ChannelBandwidth", checked: true},
            // {title: "Security", id: "Security", checked: true},
            {title: "Real-Time Traffic", id: "RealTimeTraffic", checked: true},
            {title: "VLAN", id: "VLAN", checked: true},
        ]

        defalutColumnFilter = defalutColumnFilter.map(item => {
            if (ClientListTable.hiddenColumns.includes(item.title)) {
                item.checked = false;
            }
            return item;
        });

        div.querySelector('#column-form').innerHTML = defalutColumnFilter.map(item => {
            return `<div class="form-check">
                        <input class="form-check-input" data-filter="column" type="checkbox" value="${item.title}" id="${item.id}" ${item.checked ? 'checked' : ''}>
                        <label class="form-check-label" for="${item.id}">
                            ${item.title}
                        </label>
                    </div>`
        }).join('');

        div.querySelector('#uncheck_all_column').addEventListener('click', () => {
            defalutColumnFilter = defalutColumnFilter.map(item => {
                item.checked = false;
                return item;
            });
            div.querySelectorAll('.form-check-input[data-filter="column"]').forEach((el) => {
                el.checked = false;
            })
            hiddenColumns = defalutColumnFilter.filter(item => !item.checked).map(item => item.title);
            reloadTable();
            reloadFilterIconColor();
        });

        div.querySelector('#restore_column').addEventListener('click', () => {
            defalutColumnFilter = defalutColumnFilter.map(item => {
                item.checked = true;
                return item;
            });
            div.querySelectorAll('.form-check-input[data-filter="column"]').forEach((el) => {
                el.checked = true;
            })
            ClientListTable.hiddenColumns = [];
            this.dataTable.columns().visible(true);
            reloadFilterIconColor();
        });

        div.querySelector('#restore_filter').addEventListener('click', () => {
            div.querySelectorAll('.form-check-input:not([data-filter="column"])').forEach((el) => {
                el.checked = false;
                ClientListTable.filterQuery = [];
                ClientListTable.filterRssi = [];
                this.signalRange = {min: {value: -100, width: 0}, max: {value: 0, width: 100}}
                div.querySelector('input[data-signal="min"]').value = this.signalRange.min.value;
                div.querySelector('input[data-signal="max"]').value = this.signalRange.max.value;
                div.querySelector('.signal-min').innerText = `${parseInt(this.signalRange.min.value)} dbm`;
                div.querySelector('.signal-max').innerText = `${parseInt(this.signalRange.max.value)} dbm`;
                this.getSignalWidth(this.signalRange);
                this.dataTable.columns().search('').draw();
                reloadFilterIconColor();
            })
        });

        let hiddenColumns = defalutColumnFilter.filter(item => !item.checked).map(item => item.title);

        const reloadTable = () => {
            ClientListTable.hiddenColumns = hiddenColumns;
            this.dataTable.columns().visible(true);
            hiddenColumns.forEach(column => {
                this.dataTable.column(`${column}:title`).visible(false);
            });
            reloadFilterIconColor();
        }

        div.querySelectorAll('.form-check-input[data-filter="column"]').forEach((el) => {
            el.addEventListener('change', (e) => {
                defalutColumnFilter.find(item => item.id === e.target.id).checked = e.target.checked;
                hiddenColumns = defalutColumnFilter.filter(item => !item.checked).map(item => item.title);
                reloadTable();
            })
        });
    }

    reloadFilterView() {
        this.renderFilterContent();
    }

    render() {
        return this.element
    }

    show() {
        top.document.querySelector('#right-panel').classList.add('show');
    }

    hide() {
        top.document.querySelector('#right-panel').classList.remove('show');
    }

    close() {
        this.element.remove();
        this.hide();
    }
}

export class RightPanel {
    constructor(prop) {
        const {
            defaultHtml = ``
        } = prop;
        this.element = document.createElement('div');
        this.element.id = "right-panel";
        const shadowRoot = this.element.attachShadow({mode: 'open'});
        this.shadowRoot = shadowRoot;
        const template = document.createElement('template');
        template.innerHTML = defaultHtml
        shadowRoot.appendChild(template.content.cloneNode(true));
    }

    render() {
        return this.element
    }

    setPanelContent(content) {
        this.element.shadowRoot.appendChild(content);
    }

    resetContent() {
        this.element.shadowRoot.querySelector('div')?.remove();
    }

    show() {
        this.element.classList.add('show');
    }

    hide() {
        this.element.classList.remove('show');
    }
}

class ClinetIconFetcher {
    constructor() {
        const iconList = {
            "category": [
                {
                    "id": "electronic_product",
                    "EN": "Electronic Devices",
                    "TW": "電子產品",
                    "CN": "电子设备",
                    "BR": "Dispositivos eletrônicos",
                    "CZ": "Elektronická zařízení",
                    "DA": "Elektroniske enheder",
                    "DE": "Elektronische Geräte",
                    "ES": "Dispositivos electrónicos",
                    "FI": "Elektroniikkalaitteet",
                    "FR": "Appareils électroniques",
                    "HU": "Elektronikus eszközök",
                    "IT": "Dispositivi elettronici",
                    "JP": "電子デバイス",
                    "KR": "전기 기기",
                    "MS": "Peranti Elektronik",
                    "NL": "Elektronische apparaten",
                    "NO": "Elektroniske enheter",
                    "PL": "Urządzenia elektroniczne",
                    "RU": "Электронные устройства",
                    "SV": "Elektroniska enheter",
                    "TH": "อุปกรณ์อิเล็กทรอนิกส์",
                    "TR": "Elektronik Cihazlar",
                    "UK": "Електронні пристрої",
                    "RO": "Dispozitive electronice",
                    "SL": "Elektronske naprave"
                },
                {
                    "id": "media_and_entertainment",
                    "EN": "Media & Entertainment",
                    "TW": "媒體與娛樂",
                    "CN": "媒体与娱乐",
                    "BR": "Mídia e entretenimento",
                    "CZ": "Média a zábava",
                    "DA": "Medier og underholdning",
                    "DE": "Medien & Unterhaltung",
                    "ES": "Multimedia y ocio",
                    "FI": "Media ja viihde",
                    "FR": "Médias et divertissement",
                    "HU": "Média és szórakozás",
                    "IT": "Multimedia e intrattenimento",
                    "JP": "メディアとエンターテインメント",
                    "KR": "미디어 및 엔터테인먼트",
                    "MS": "Media & Hiburan",
                    "NL": "Media & Entertainment",
                    "NO": "Medier og underholdning",
                    "PL": "Multimedia i rozrywka",
                    "RU": "Мультимедиа и развлечения",
                    "SV": "Media och underhållning",
                    "TH": "มัลติมีเดียและความบันเทิง",
                    "TR": "Medya ve Eğlence",
                    "UK": "Медіа та розваги",
                    "RO": "Media și divertisment",
                    "SL": "Mediji in zabava"
                },
                {
                    "id": "electronic_equipment",
                    "EN": "Electronic Equipment",
                    "TW": "電子設備",
                    "CN": "电子设备",
                    "BR": "Equipamento eletrônico",
                    "CZ": "Elektronická zařízení",
                    "DA": "Elektronisk udstyr",
                    "DE": "Elektronische Geräte",
                    "ES": "Equipo electrónico",
                    "FI": "Elektroniikkalaite",
                    "FR": "Équipement électronique",
                    "HU": "Elektronikus berendezés",
                    "IT": "Apparecchiature elettroniche",
                    "JP": "電子機器",
                    "KR": "전자 장치",
                    "MS": "Peralatan Elektronik",
                    "NL": "Elektronische apparatuur",
                    "NO": "Elektronisk utstyr",
                    "PL": "Sprzęt elektroniczny",
                    "RU": "Электронное оборудование",
                    "SV": "Elektronisk utrustning",
                    "TH": "อุปกรณ์อิเล็กทรอนิกส์",
                    "TR": "Elektronik Donanım",
                    "UK": "Електронне обладнання",
                    "RO": "Echipament electronic",
                    "SL": "Elektronska oprema"
                },
                {
                    "id": "security",
                    "EN": "Security",
                    "TW": "安全性裝置",
                    "CN": "安全设备",
                    "BR": "Segurança",
                    "CZ": "Zabezpečení",
                    "DA": "Sikkerhed",
                    "DE": "Sicherheit",
                    "ES": "Seguridad",
                    "FI": "Tietoturva",
                    "FR": "Sécurité",
                    "HU": "Biztonság",
                    "IT": "Sicurezza",
                    "JP": "セキュリティ",
                    "KR": "보안",
                    "MS": "Keselamatan",
                    "NL": "Beveiliging",
                    "NO": "Sikkerhet",
                    "PL": "Alarm",
                    "RU": "Безопасность",
                    "SV": "Säkerhet",
                    "TH": "ความปลอดภัย",
                    "TR": "Güvenlik",
                    "UK": "Пристрій системи безпеки",
                    "RO": "Securitate",
                    "SL": "Varnost"
                },
                {
                    "id": "warables",
                    "EN": "Wearables",
                    "TW": "穿戴裝置",
                    "CN": "可穿戴设备",
                    "BR": "Acessórios digitais (wearables)",
                    "CZ": "Nositelná elektronika",
                    "DA": "Bærbare",
                    "DE": "Tragbare Geräte",
                    "ES": "Wearables",
                    "FI": "Puettavat laitteet",
                    "FR": "Accessoires",
                    "HU": "Viselhető eszközök",
                    "IT": "Indossabili",
                    "JP": "ウェアラブル",
                    "KR": "웨어러블",
                    "MS": "Boleh pakai",
                    "NL": "Draagbare apparaten",
                    "NO": "Kroppsnær teknologi",
                    "PL": "Urządzenia typu „wearable”",
                    "RU": "Носимые устройства",
                    "SV": "Kroppsnära enheter",
                    "TH": "อุปกรณ์สวมใส่อัจฉริยะ",
                    "TR": "Kıyafet",
                    "UK": "Носимі гаджети",
                    "RO": "Dispozitive purtabile",
                    "SL": "Nosljive naprave"
                },
                {
                    "id": "transportation",
                    "EN": "Transportation",
                    "TW": "運輸",
                    "CN": "交通工具",
                    "BR": "Transporte",
                    "CZ": "Přeprava",
                    "DA": "Transport",
                    "DE": "Transport",
                    "ES": "Transporte",
                    "FI": "Kuljetus",
                    "FR": "Transport",
                    "HU": "Szállítás",
                    "IT": "Trasporto",
                    "JP": "輸送",
                    "KR": "대중교통",
                    "MS": "Transportation",
                    "NL": "Vervoer",
                    "NO": "Transport",
                    "PL": "Transport",
                    "RU": "Транспорт",
                    "SV": "Transport",
                    "TH": "การขนส่ง",
                    "TR": "Taşıma",
                    "UK": "Транспорт",
                    "RO": "Transport",
                    "SL": "Transport"
                },
                {
                    "id": "others",
                    "EN": "Others",
                    "TW": "其他",
                    "CN": "其他",
                    "BR": "Outros",
                    "CZ": "Ostatní",
                    "DA": "Andre",
                    "DE": "Andere",
                    "ES": "Otros",
                    "FI": "Muut",
                    "FR": "Autres",
                    "HU": "Egyéb",
                    "IT": "Altro",
                    "JP": "その他",
                    "KR": "기타",
                    "MS": "Lain-lain",
                    "NL": "Overige",
                    "NO": "Andre",
                    "PL": "Inne",
                    "RU": "Другие",
                    "SV": "Andra",
                    "TH": "อื่นๆ",
                    "TR": "Diğerleri",
                    "UK": "Інше",
                    "RO": "Altele",
                    "SL": "Drugi"
                }
            ],
            "devices": [
                {
                    "category": "electronic_product",
                    "ui-sort": 1,
                    "type": [
                        "0"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvZGVza3RvcCI+PGcgaWQ9Ikdyb3VwXzIiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMV9LU1ZGQ1lVREJFIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNNC44IDE3LjE2QzMuNjYgMTcuMTYgMi43NDAwMSAxNi4yNCAyLjc0MDAxIDE1LjFWNC44QzIuNzQwMDEgMy42NiAzLjY2IDIuNzQwMDEgNC44IDIuNzQwMDFIMTkuMTlDMjAuMzMgMi43NDAwMSAyMS4yNSAzLjY2IDIxLjI1IDQuOFYxNS4xQzIxLjI1IDE2LjI0IDIwLjMzIDE3LjE2IDE5LjE5IDE3LjE2SDQuOFoiLz48ZyBpZD0iR3JvdXBfMyI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8yX1dPWFhPQUVRWUwiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xMC4xNCAxNy4xNkw5LjU1IDIwLjcxIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8zX1hLR0xaVlBYRkIiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xMy43NiAxNy4zM0wxNC4zMiAyMC42NyIvPjwvZz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzRfQVlBS0tWSU1BWSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTcuODMgMjEuMjVIMTYuMDUiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzVfU1hVS1BCTEZZTSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTMgMTQuMDRIMjEuMTkiLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Device",
                    "TW": "裝置",
                    "CN": "设备",
                    "BR": "Dispositivo",
                    "CZ": "Zařízení",
                    "DA": "Enhed",
                    "DE": "Gerät",
                    "ES": "Dispositivo",
                    "FI": "Laite",
                    "FR": "Appareil",
                    "HU": "Eszköz",
                    "IT": "Dispositivo",
                    "JP": "デバイス",
                    "KR": "장치",
                    "MS": "Peranti",
                    "NL": "Apparaat",
                    "NO": "Enhet",
                    "PL": "Urządzenie",
                    "RU": "Устройство",
                    "SV": "Enhet",
                    "TH": "อุปกรณ์",
                    "TR": "Cihaz",
                    "UK": "Пристрій",
                    "RO": "Dispozitiv",
                    "SL": "Naprava"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 2,
                    "type": [
                        "34"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvZGVza3RvcCI+PGcgaWQ9Ikdyb3VwXzIiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMV9LU1ZGQ1lVREJFIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNNC44IDE3LjE2QzMuNjYgMTcuMTYgMi43NDAwMSAxNi4yNCAyLjc0MDAxIDE1LjFWNC44QzIuNzQwMDEgMy42NiAzLjY2IDIuNzQwMDEgNC44IDIuNzQwMDFIMTkuMTlDMjAuMzMgMi43NDAwMSAyMS4yNSAzLjY2IDIxLjI1IDQuOFYxNS4xQzIxLjI1IDE2LjI0IDIwLjMzIDE3LjE2IDE5LjE5IDE3LjE2SDQuOFoiLz48ZyBpZD0iR3JvdXBfMyI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8yX1dPWFhPQUVRWUwiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xMC4xNCAxNy4xNkw5LjU1IDIwLjcxIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8zX1hLR0xaVlBYRkIiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xMy43NiAxNy4zM0wxNC4zMiAyMC42NyIvPjwvZz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzRfQVlBS0tWSU1BWSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTcuODMgMjEuMjVIMTYuMDUiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzVfU1hVS1BCTEZZTSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTMgMTQuMDRIMjEuMTkiLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Desktop",
                    "TW": "桌上型電腦",
                    "CN": "桌面",
                    "BR": "Desktop",
                    "CZ": "Stolní počítač",
                    "DA": "Desktop",
                    "DE": "Desktop",
                    "ES": "Escritorio",
                    "FI": "Työpöytä",
                    "FR": "Bureau",
                    "HU": "Asztal",
                    "IT": "Desktop",
                    "JP": "デスクトップ",
                    "KR": "데스크톱",
                    "MS": "Desktop",
                    "NL": "Bureaublad",
                    "NO": "Stasjonær datamaskin",
                    "PL": "Komputer",
                    "RU": "Рабочий стол",
                    "SV": "Skrivbord",
                    "TH": "เดสก์ท็อป",
                    "TR": "Masaüstü",
                    "UK": "Комп’ютер",
                    "RO": "Desktop",
                    "SL": "Namizje"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 3,
                    "type": [
                        "14"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvaU1hYyI+PGcgaWQ9ImlNYWMiPjxnIGlkPSJHcm91cCA0Ij48cGF0aCBpZD0iVmVjdG9yX19fX18wXzBfRlZQSVpBTE5FVCIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE4LjU3OCAxNS41NTAxQzE4LjU3OCAxNi41NjY5IDE3Ljc0MzcgMTcuNDAxMyAxNi43MjY5IDE3LjQwMTNINC4zNTExNUMzLjMzNDMyIDE3LjQwMTMgMi41IDE2LjU2NjkgMi41IDE1LjU1MDFWMTMuODI5M0gxOC41NzhWMTUuNTUwMVoiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzFfTVNQSUJYVVRJRyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE4LjU3OCAxMy44MjkzSDIuNVY1Ljg1MTE1QzIuNSA0LjgzNDMyIDMuMzM0MzIgNCA0LjM1MTE1IDRIMTYuNzI2OUMxNy43NDM3IDQgMTguNTc4IDQuODM0MzIgMTguNTc4IDUuODUxMTUiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzJfR0dOV1NFUkNYVSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTguNzQ3NzkgMTcuNDAxMUw3Ljg2MTMzIDIwLjA3NzlIMTMuMjE0OUwxMi4zMjg0IDE3LjQwMTFIOC43NDc3OVoiLz48L2c+PGcgaWQ9IkFwcGxlIj48ZyBpZD0iR3JvdXBfMiI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8zX1hHWUNFVllNRFYiIGZpbGw9ImJsYWNrIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMjEuMzI0NiAxMi4zMjYzQzIxLjEyODMgMTIuNTEwOSAyMC45NTg3IDEyLjcxNTcgMjAuODIxNSAxMi45NDQ0QzIwLjU2MjIgMTMuMzc2NiAyMC40Mzc5IDEzLjg2MiAyMC40Mzc5IDE0LjM5NTRDMjAuNDM3OSAxNC4zOTM0IDIwLjQzMjMgMTQuNTMwMSAyMC40NTQgMTQuNzIwN0MyMC40NzU2IDE0LjkwOTkgMjAuNTI0NCAxNS4xNjI5IDIwLjYzMzQgMTUuNDQyNkMyMC44MTI4IDE1LjkwMjkgMjEuMTQ5NCAxNi40MTc2IDIxLjc1NTQgMTYuODMwMkMyMS42MDM2IDE3LjE3NDggMjEuMzg3NyAxNy41ODIxIDIxLjEyOTMgMTcuOTQxQzIwLjkzNzMgMTguMjA3NiAyMC43Mzc2IDE4LjQyNTIgMjAuNTQzMiAxOC41NzEzQzIwLjM0ODQgMTguNzE3NiAyMC4xOTQzIDE4Ljc2NjYgMjAuMDc3NiAxOC43NjY2QzE5Ljg3OTcgMTguNzY2NiAxOS43MjQ1IDE4LjcwNzcgMTkuNDM4NSAxOC41OTVDMTkuMTYwOSAxOC40ODU2IDE4Ljc4NzMgMTguMzM5OCAxOC4yODg3IDE4LjMzOThDMTcuNzgyNyAxOC4zMzk4IDE3LjM5NjIgMTguNDgxOSAxNy4xMDc0IDE4LjU5MzVMMTcuMTA1NiAxOC41OTQyQzE2Ljc5OCAxOC43MTMgMTYuNjU5NCAxOC43NjY2IDE2LjQ5OTggMTguNzY2NkMxNi40OTIxIDE4Ljc2NjYgMTYuNDg0MyAxOC43NjY3IDE2LjQ3NjUgMTguNzY3QzE2LjM4OSAxOC43NzA0IDE2LjI0NjEgMTguNzMzMSAxNi4wNDEyIDE4LjU3NEMxNS44Mzk3IDE4LjQxNzQgMTUuNjI4MiAxOC4xODA3IDE1LjQyMiAxNy44OTA3QzE1LjAwOTMgMTcuMzEwMiAxNC42ODY3IDE2LjYxNDYgMTQuNTQ4NyAxNi4yMjMzTDE0LjU0ODggMTYuMjIzM0wxNC41NDcxIDE2LjIxODZDMTQuMzM0MiAxNS42MzA1IDE0LjI1MDIgMTUuMDY2MiAxNC4yNTAyIDE0LjUxNDdDMTQuMjUwMiAxMy42MDU3IDE0LjU0NzIgMTIuOTg2NSAxNC45MzU0IDEyLjU5MzZDMTUuMzI4NSAxMi4xOTU2IDE1Ljg1IDExLjk5MzkgMTYuMzYwOCAxMS45ODI2QzE2LjU2MzQgMTEuOTgzOSAxNi44MzE1IDEyLjA1OTYgMTcuMTYwNyAxMi4xNzI4QzE3LjIwMSAxMi4xODY3IDE3LjI0MzQgMTIuMjAxNSAxNy4yODY2IDEyLjIxNjZDMTcuNDAxMSAxMi4yNTY2IDE3LjUyMTkgMTIuMjk4OCAxNy42Mjc5IDEyLjMzMTdDMTcuNzYzOCAxMi4zNzM4IDE3Ljk1NjkgMTIuNDI4MiAxOC4xNDQzIDEyLjQyODJDMTguMzQyNyAxMi40MjgyIDE4LjU1NTUgMTIuMzU3NCAxOC42ODU5IDEyLjMxMzFDMTguNzU2MSAxMi4yODkzIDE4LjgzMTUgMTIuMjYyMyAxOC45MDUyIDEyLjIzNThDMTguOTE0NCAxMi4yMzI1IDE4LjkyMzcgMTIuMjI5MiAxOC45MzI5IDEyLjIyNTlDMTkuMDE3NCAxMi4xOTU2IDE5LjEwMjUgMTIuMTY1MyAxOS4xOTAzIDEyLjEzNThDMTkuNTU0MyAxMi4wMTM4IDE5Ljg5NzEgMTEuOTMxNSAyMC4xOTIgMTEuOTU1NUwyMC4xOTM0IDExLjk1NTZDMjAuNjY3NyAxMS45OTMxIDIxLjAzNiAxMi4xMTc3IDIxLjMyNDYgMTIuMzI2M1oiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzRfTUtJUUhPWVNOVCIgZmlsbD0iYmxhY2siIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xOS4yNzIyIDguMzgxNDNDMTkuMzYxOCA4LjM1NTExIDE5LjQxNjEgOC4zNTI3NyAxOS40NDQyIDguMzU0NTZDMTkuNDU0IDguMzg1ODkgMTkuNDY3NSA4LjQ1NTM4IDE5LjQ0OTIgOC41ODM5QzE5LjQxODkgOC43OTU3MyAxOS4zMTQ0IDkuMDE0OTggMTkuMTc4NyA5LjE1MDY5TDE5LjE3ODcgOS4xNTA2N0wxOS4xNzU0IDkuMTU0MDRDMTkuMDYzNSA5LjI2Nzc2IDE4Ljg4MjEgOS4zNjg1NCAxOC43MDUzIDkuNDA5MTVDMTguNjA2NyA5LjQzMTc5IDE4LjU0ODcgOS40Mjg3OCAxOC41MTk4IDkuNDI0MDRDMTguNTEyNSA5LjM5MzkyIDE4LjUwNTEgOS4zMzA5OCAxOC41MjU0IDkuMjE4NTdDMTguNTYxIDkuMDIxMjUgMTguNjY1MSA4LjgwOTk3IDE4Ljc5MjIgOC42NzI4OUMxOC45MTE2IDguNTQ2MjggMTkuMDk2OCA4LjQzMjg4IDE5LjI3MjIgOC4zODE0M1oiLz48L2c+PC9nPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Apple iMac",
                    "TW": "Apple iMac",
                    "CN": "Apple iMac",
                    "BR": "Apple iMac",
                    "CZ": "Apple iMac",
                    "DA": "Apple iMac",
                    "DE": "Apple iMac",
                    "ES": "Apple iMac",
                    "FI": "Apple iMac",
                    "FR": "Apple iMac",
                    "HU": "Apple iMac",
                    "IT": "Apple iMac",
                    "JP": "Apple iMac",
                    "KR": "Apple iMac",
                    "MS": "Apple iMac",
                    "NL": "Apple iMac",
                    "NO": "Apple iMac",
                    "PL": "Apple iMac",
                    "RU": "Apple iMac",
                    "SV": "Apple iMac",
                    "TH": "Apple iMac",
                    "TR": "Apple iMac",
                    "UK": "Apple iMac",
                    "RO": "Apple iMac",
                    "SL": "Apple iMac"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 4,
                    "type": [
                        "1",
                        "13"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvd2luZG93cyBkZXNrdG9wIj48ZyBpZD0iR3JvdXAgMzUzNSI+PGcgaWQ9Ikdyb3VwXzIiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMl9ESUhUWEZNUFZDIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbWl0ZXJsaW1pdD0iMTAiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTkuMTQzIDYuNzE0M0wxOS4xNDMgNS42ODU3MkMxOS4xNDMgNS4yMjg1NyAxOC44Mzg2IDUgMTguNDMyOSA1SDIuNzA5OTlDMi4zMDQyMyA1IDEuOTk5OTIgNS4zNDI4NiAxLjk5OTkyIDUuNjg1NzJWMTUuNjI4N0MxLjk5OTkyIDE2LjA4NTggMi4zMDQyMyAxNi40Mjg3IDIuNzA5OTkgMTYuNDI4N0gxMy40Mjg2Ii8+PHBhdGggaWQ9IlZlY3RvciAzMF9fX19fMF8zX1NWRUZQUk1OSFMiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMiIgZD0iTTEwLjU3MTUgMTYuNDI4N1YxOS44NTczIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF80X1NKUlhUVURaTlIiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTUuNTI0IDE5Ljg1NzNIMTAuNzYySDYuMDAwMDQiLz48L2c+PGcgaWQ9IkFwcGxlIj48cGF0aCBpZD0iVW5pb25fX19fXzBfNV9TVlRMS1ROVE5UIiBmaWxsPSJibGFjayIgZmlsbC1ydWxlPSJldmVub2RkIiBkPSJNMTYuMDA1MSAxMi43NDZMMTUuOTkwMyAxMi43NDYyVjguNDg1NDNMMjEuOTk5MyA3LjI4NTkzVjEyLjY4NTdDMjAuMDAxMiAxMi43MDU5IDE4LjAwMzIgMTIuNzI2IDE2LjAwNTEgMTIuNzQ2Wk0xNS45OTAzIDE3LjQ2ODJWMTMuMjMzNUMxNy45OTMzIDEzLjI1MzYgMTkuOTk2MyAxMy4yNzM3IDIxLjk5OTMgMTMuMjk0VjE4LjY2NzZDMTkuOTk2MyAxOC4yNjc5IDE3Ljk5MzMgMTcuODY4MSAxNS45OTAzIDE3LjQ2ODJaTTE1LjQ4MiAxMi43NTE3VjguNTg2NDRMMTUuNDc3MyA4LjU4NzM4QzE0LjIyMjcgOC44Mzc4IDEyLjk2ODEgOS4wODgyMiAxMS43MTM1IDkuMzM4ODNWMTIuNzg5NkwxNS40ODIgMTIuNzUxN1pNMTEuNzEzNSAxMy4xOUwxNS40ODIgMTMuMjI3OVYxNy4zNjY5TDExLjcxMzUgMTYuNjE0N1YxMy4xOVoiIGNsaXAtcnVsZT0iZXZlbm9kZCIvPjwvZz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Windows Desktop",
                    "TW": "Windows桌上型電腦",
                    "CN": "Windows 桌面",
                    "BR": "Windows Desktop",
                    "CZ": "Stolní počítač se systémem Windows",
                    "DA": "Windows Desktop",
                    "DE": "Windows-Desktop",
                    "ES": "Escritorio de Windows",
                    "FI": "Windows-työpöytä",
                    "FR": "Bureau Windows",
                    "HU": "Asztali Windows",
                    "IT": "Desktop Windows",
                    "JP": "Windowsデスクトップ",
                    "KR": "Windows용 데스크톱",
                    "MS": "Desktop Windows",
                    "NL": "Windows-bureaublad",
                    "NO": "Stasjonær Windows-datamaskin",
                    "PL": "Komputer Windows",
                    "RU": "Рабочий стол Windows",
                    "SV": "Windows-skrivbord",
                    "TH": "เดสก์ท็อป Windows",
                    "TR": "Windows Desktop",
                    "UK": "Комп’ютер із Windows",
                    "RO": "Desktop Windows",
                    "SL": "Namizje Windows"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 5,
                    "type": [
                        "72"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvbGludXggZGVza3RvcCIgY2xpcC1wYXRoPSJ1cmwoI2NsaXAwXzMxM180MjcpIj48ZyBpZD0iR3JvdXAgMzUzOCI+PGcgaWQ9Ikdyb3VwXzIiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMF9SSERIT0xGUEFUIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbWl0ZXJsaW1pdD0iMTAiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTkuNjc3IDcuOTQ2MTdMMTkuNjc2OSA0LjcwNzA4QzE5LjY3NjkgNC4yMzU2OSAxOS4zNjMyIDQgMTguOTQ0OCA0SDIuNzMyMUMyLjMxMzcxIDQgMS45OTk5MSA0LjM1MzU0IDEuOTk5OTEgNC43MDcwOFYxNC45NTk4QzEuOTk5OTEgMTUuNDMxMSAyLjMxMzcxIDE1Ljc4NDcgMi43MzIxIDE1Ljc4NDdIMTcuNzg0NiIvPjxwYXRoIGlkPSJWZWN0b3IgMzBfX19fXzBfMV9OSUtUWEVYTU5OIiBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjIiIGQ9Ik0xMC44Mzg1IDE1Ljc4NDdWMTkuMzIwMSIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMl9XSEZJS0dLVlBOIiBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE1Ljk0NTIgMTkuMzIwMUgxMS4wMzQ5SDYuMTI0NjQiLz48L2c+PC9nPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfOF9CR1dIU0NGVE9JIiBmaWxsPSJibGFjayIgZD0iTTIyLjg5OTMgMTEuNzg2MUMyMi41MTU0IDExLjIyNTEgMjIuMDUxOCAxMC43MjIxIDIxLjUyMjkgMTAuMjkyNkMyMS43MTE5IDkuOTUxMzggMjEuODA3MiA5LjU2NjkxIDIxLjc5OTQgOS4xNzc2NUMyMS43OTE1IDguNzg4MzkgMjEuNjgwOCA4LjQwODAxIDIxLjQ3ODIgOC4wNzQ2MUMyMS4yNzU2IDcuNzQxMiAyMC45ODgzIDcuNDY2NDggMjAuNjQ1MSA3LjI3Nzk0QzIwLjMwMTkgNy4wODk0IDE5LjkxNDggNi45OTM2NiAxOS41MjI2IDcuMDAwMzNDMTguOTI0MyA3LjAwMTUzIDE4LjM1MDggNy4yMzc5NSAxNy45Mjc4IDcuNjU3ODRDMTcuNTA0NyA4LjA3NzczIDE3LjI2NjUgOC42NDY4OCAxNy4yNjUzIDkuMjQwNjlDMTcuMjY0OSA5LjYwODkxIDE3LjM1OTcgOS45NzEwNiAxNy41NDA1IDEwLjI5MjZDMTcuMDE0IDEwLjcyNDQgMTYuNTUwNyAxMS4yMjcxIDE2LjE2NDEgMTEuNzg2MUMxNS4yNDY1IDEzLjA2MTEgMTQuNzg3NyAxNC4zNDk4IDE1LjA5NTEgMTQuNjU5NUMxNS4yNzQxIDE0LjgzMjUgMTUuNjY4NiAxNC42NTk1IDE2LjEzNjYgMTQuMjY3OEMxNi4zMzIxIDE0Ljk2NjQgMTYuNzE5NyAxNS41OTcxIDE3LjI1NjEgMTYuMDg5M1YxNi4wODkzQzE3LjEzNDQgMTYuMDg5MyAxNy4wMTc3IDE2LjEzNzMgMTYuOTMxNyAxNi4yMjI3QzE2Ljg0NTYgMTYuMzA4MSAxNi43OTczIDE2LjQyMzkgMTYuNzk3MyAxNi41NDQ2QzE2Ljc5NzMgMTYuNjY1NCAxNi44NDU2IDE2Ljc4MTIgMTYuOTMxNyAxNi44NjY2QzE3LjAxNzcgMTYuOTUyIDE3LjEzNDQgMTcgMTcuMjU2MSAxN0gyMS44NDQxQzIxLjk2NTggMTcgMjIuMDgyNSAxNi45NTIgMjIuMTY4NSAxNi44NjY2QzIyLjI1NDYgMTYuNzgxMiAyMi4zMDI5IDE2LjY2NTQgMjIuMzAyOSAxNi41NDQ2QzIyLjMwMjkgMTYuNDIzOSAyMi4yNTQ2IDE2LjMwODEgMjIuMTY4NSAxNi4yMjI3QzIyLjA4MjUgMTYuMTM3MyAyMS45NjU4IDE2LjA4OTMgMjEuODQ0MSAxNi4wODkzQzIyLjM3NzMgMTUuNTk1NCAyMi43NjMxIDE0Ljk2NTEgMjIuOTU5IDE0LjI2NzhDMjMuNDE3OCAxNC42NjQgMjMuODIxNSAxNC44Mjc5IDI0LjAwNSAxNC42NTQ5QzI0LjI5ODcgMTQuMzQ5OCAyMy44MTY5IDEzLjA2MTEgMjIuODk5MyAxMS43ODYxWk0xOS41MjI2IDkuMTcyMzlDMjAuMTU1NyA5LjE3MjM5IDIwLjY2OTYgOS40MjczOSAyMC42Njk2IDkuNzQxNThDMjAuNjY5NiAxMC4wNTU4IDIwLjE1NTcgMTAuMzEwOCAxOS41MjI2IDEwLjMxMDhDMTguODg5NCAxMC4zMTA4IDE4LjM3NTYgMTAuMDU1OCAxOC4zNzU2IDkuNzMyNDhDMTguMzc1NiA5LjQwOTE3IDE4Ljg4OTQgOS4xNzIzOSAxOS41MjI2IDkuMTcyMzlaTTIwLjUwNDQgMTUuODYxNkMxOS44NzM5IDE2LjExMDUgMTkuMTcxMyAxNi4xMTA1IDE4LjU0MDcgMTUuODYxNkMxOC4xMjA3IDE1LjY0NjYgMTcuNzY5NCAxNS4zMTk3IDE3LjUyNjMgMTQuOTE3NUMxNy4yODMyIDE0LjUxNTMgMTcuMTU3OCAxNC4wNTM4IDE3LjE2NDMgMTMuNTg0OEMxNy4xNDk0IDEyLjk0NiAxNy4zODg1IDEyLjMyNyAxNy44Mjk4IDExLjg2MTdDMTguMjcxMSAxMS4zOTY1IDE4Ljg3OTIgMTEuMTIyMyAxOS41MjI2IDExLjA5ODZDMjAuMTY5NiAxMS4xMjExIDIwLjc4MTMgMTEuMzk3NCAyMS4yMjMzIDExLjg2NjlDMjEuNjY1NCAxMi4zMzY0IDIxLjkwMTkgMTIuOTYwOCAyMS44ODA4IDEzLjYwM0MyMS44ODQ0IDE0LjA2OSAyMS43NTc3IDE0LjUyNjkgMjEuNTE0NyAxNC45MjU2QzIxLjI3MTcgMTUuMzI0NCAyMC45MjE5IDE1LjY0ODQgMjAuNTA0NCAxNS44NjE2WiIvPjwvZz48L2c+PGRlZnM+PGNsaXBQYXRoIGlkPSJjbGlwMF8zMTNfNDI3Ij48cGF0aCBmaWxsPSJ3aGl0ZSIgZD0iTTAgMEgyNFYyNEgweiIvPjwvY2xpcFBhdGg+PC9kZWZzPjwvc3ZnPg==",
                    "EN": "Linux Desktop",
                    "TW": "Linux 桌上型電腦",
                    "CN": "Linux 桌面",
                    "BR": "Linux Desktop",
                    "CZ": "Stolní počítač se systémem Linux",
                    "DA": "Linux Desktop",
                    "DE": "Linux-Desktop",
                    "ES": "Escritorio de Linux",
                    "FI": "Linux-työpöytä",
                    "FR": "Bureau Linux",
                    "HU": "Asztali Linux",
                    "IT": "Desktop Linux",
                    "JP": "Linuxデスクトップ",
                    "KR": "Linux용 데스크톱",
                    "MS": "Desktop Linux",
                    "NL": "Linux-bureaublad",
                    "NO": "Stasjonær Linux-datamaskin",
                    "PL": "Komputer Linux",
                    "RU": "Рабочий стол Linux",
                    "SV": "Linux-skrivbord",
                    "TH": "เดสก์ท็อป Linux",
                    "TR": "Linux Desktop",
                    "UK": "Комп’ютер із Linux",
                    "RO": "Desktop Linux",
                    "SL": "Namizje Linux"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 6,
                    "type": [
                        "22"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvbGludXggZGV2aWNlIj48ZyBpZD0iZGVzaWduIj48cGF0aCBpZD0iVmVjdG9yX19fX18wXzBfWk5WWExGQkpMUSIgZmlsbD0iYmxhY2siIGQ9Ik0xOC4xMTIxIDExLjYxNTFDMTcuNDI2MiAxMC42MDUyIDE2LjU5OCA5LjY5OTczIDE1LjY1MzIgOC45MjY2MkMxNS45OTA4IDguMzEyNDggMTYuMTYxMSA3LjYyMDQzIDE2LjE0NyA2LjkxOTc3QzE2LjEzMyA2LjIxOTEgMTUuOTM1MSA1LjUzNDQyIDE1LjU3MzIgNC45MzQzQzE1LjIxMTMgNC4zMzQxNyAxNC42OTgxIDMuODM5NjYgMTQuMDg1IDMuNTAwMjlDMTMuNDcxOCAzLjE2MDkxIDEyLjc4MDIgMi45ODg2IDEyLjA3OTUgMy4wMDA1OUMxMS4wMTA3IDMuMDAyNzUgOS45ODYyMiAzLjQyODMxIDkuMjMwNDIgNC4xODQxMkM4LjQ3NDYyIDQuOTM5OTIgOC4wNDkwNiA1Ljk2NDM4IDguMDQ2ODkgNy4wMzMyNEM4LjA0NjI4IDcuNjk2MDMgOC4yMTU2IDguMzQ3OTEgOC41Mzg2OCA4LjkyNjYyQzcuNTk3OTQgOS43MDM5OSA2Ljc3MDMxIDEwLjYwODkgNi4wNzk3NCAxMS42MTUxQzQuNDQwNDUgMTMuOTEwMSAzLjYyMDggMTYuMjI5NyA0LjE2OTk3IDE2Ljc4N0M0LjQ4OTYzIDE3LjA5ODUgNS4xOTQ1MiAxNi43ODcgNi4wMzA1NiAxNi4wODIxQzYuMzc5NzEgMTcuMzM5NSA3LjA3MjI0IDE4LjQ3NDggOC4wMzA1IDE5LjM2MDdWMTkuMzYwN0M3LjgxMzExIDE5LjM2MDcgNy42MDQ2MyAxOS40NDcxIDcuNDUwOTIgMTkuNjAwOEM3LjI5NzIxIDE5Ljc1NDUgNy4yMTA4NSAxOS45NjMgNy4yMTA4NSAyMC4xODA0QzcuMjEwODUgMjAuMzk3NyA3LjI5NzIxIDIwLjYwNjIgNy40NTA5MiAyMC43NTk5QzcuNjA0NjMgMjAuOTEzNiA3LjgxMzExIDIxIDguMDMwNSAyMUgxNi4yMjdDMTYuNDQ0MyAyMSAxNi42NTI4IDIwLjkxMzYgMTYuODA2NSAyMC43NTk5QzE2Ljk2MDIgMjAuNjA2MiAxNy4wNDY2IDIwLjM5NzcgMTcuMDQ2NiAyMC4xODA0QzE3LjA0NjYgMTkuOTYzIDE2Ljk2MDIgMTkuNzU0NSAxNi44MDY1IDE5LjYwMDhDMTYuNjUyOCAxOS40NDcxIDE2LjQ0NDMgMTkuMzYwNyAxNi4yMjcgMTkuMzYwN0MxNy4xNzk1IDE4LjQ3MTYgMTcuODY4NyAxNy4zMzcyIDE4LjIxODcgMTYuMDgyMUMxOS4wMzgzIDE2Ljc5NTIgMTkuNzU5NiAxNy4wOTAzIDIwLjA4NzUgMTYuNzc4OEMyMC42MTIxIDE2LjIyOTcgMTkuNzUxNCAxMy45MTAxIDE4LjExMjEgMTEuNjE1MVpNMTIuMDc5NSA2LjkxMDI5QzEzLjIxMDcgNi45MTAyOSAxNC4xMjg3IDcuMzY5MyAxNC4xMjg3IDcuOTM0ODVDMTQuMTI4NyA4LjUwMDQxIDEzLjIxMDcgOC45NTk0MSAxMi4wNzk1IDguOTU5NDFDMTAuOTQ4NCA4Ljk1OTQxIDEwLjAzMDQgOC41MDA0MSAxMC4wMzA0IDcuOTE4NDZDMTAuMDMwNCA3LjMzNjUxIDEwLjk0ODQgNi45MTAyOSAxMi4wNzk1IDYuOTEwMjlaTTEzLjgzMzYgMTguOTUwOUMxMi43MDcxIDE5LjM5ODkgMTEuNDUxOSAxOS4zOTg5IDEwLjMyNTUgMTguOTUwOUM5LjU3NTE4IDE4LjU2MzkgOC45NDc2IDE3Ljk3NTQgOC41MTMyNCAxNy4yNTE1QzguMDc4ODggMTYuNTI3NSA3Ljg1NDk0IDE1LjY5NjggNy44NjY1NyAxNC44NTI3QzcuODM5OTMgMTMuNzAyOCA4LjI2Njk4IDEyLjU4ODYgOS4wNTUzOCAxMS43NTExQzkuODQzNzggMTAuOTEzNyAxMC45MzAxIDEwLjQyMDIgMTIuMDc5NSAxMC4zNzc0QzEzLjIzNTUgMTAuNDE4IDE0LjMyODIgMTAuOTE1NCAxNS4xMTggMTEuNzYwNUMxNS45MDc3IDEyLjYwNTYgMTYuMzMwMSAxMy43Mjk0IDE2LjI5MjUgMTQuODg1NEMxNi4yOTkgMTUuNzI0MyAxNi4wNzI1IDE2LjU0ODQgMTUuNjM4NCAxNy4yNjYyQzE1LjIwNDMgMTcuOTgzOSAxNC41Nzk1IDE4LjU2NzEgMTMuODMzNiAxOC45NTA5WiIvPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Linux Device",
                    "TW": "Linux裝置",
                    "CN": "Linux 设备",
                    "BR": "Dispositivo Linux",
                    "CZ": "Zařízení se systémem Linux",
                    "DA": "Linux-enhed",
                    "DE": "Linux-Gerät",
                    "ES": "Dispositivo Linux",
                    "FI": "Linux-laite",
                    "FR": "Périphérique Linux",
                    "HU": "Linuxos eszköz",
                    "IT": "Dispositivo Linux",
                    "JP": "Linuxデバイス",
                    "KR": "Linux용 기기",
                    "MS": "Peranti Linux",
                    "NL": "Linux-apparaat",
                    "NO": "Linux-enhet",
                    "PL": "Urządzenie Linux",
                    "RU": "Устройство Linux",
                    "SV": "Linux-enhet",
                    "TH": "อุปกรณ์ Linux",
                    "TR": "Linux Cihazı",
                    "UK": "Пристрій із Linux",
                    "RO": "Dispozitiv Linux",
                    "SL": "Naprava Linux"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 7,
                    "type": [
                        "33"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvc21hcnRwaG9uZSI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8xX0hUWEtXS0dBUlUiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xOCAxNC43M1YxOS40N0MxOCAyMC40NSAxNy4yMiAyMS4yNSAxNi4yNyAyMS4yNUg3LjczMDAxQzYuNzcwMDEgMjEuMjUgNiAyMC40NSA2IDE5LjQ3VjQuNTNDNiAzLjU1IDYuNzgwMDEgMi43NSA3LjczMDAxIDIuNzVIMTYuMjdDMTcuMjMgMi43NSAxOCAzLjU1IDE4IDQuNTNWMTUuMDgiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzJfTFRHTUhISEtCTyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTExLjUgNS44M0gxMi41Ii8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8zX1NES09YQVBTT1oiIGZpbGw9ImJsYWNrIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGQ9Ik0xMiAxOS4wMUMxMS40NDc3IDE5LjAxIDExIDE4LjU2MjMgMTEgMTguMDFDMTEgMTcuNDU3NyAxMS40NDc3IDE3LjAxIDEyIDE3LjAxQzEyLjU1MjMgMTcuMDEgMTMgMTcuNDU3NyAxMyAxOC4wMUMxMyAxOC41NjIzIDEyLjU1MjMgMTkuMDEgMTIgMTkuMDFaIiBjbGlwLXJ1bGU9ImV2ZW5vZGQiLz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Smartphone",
                    "TW": "智慧型手機",
                    "CN": "智能手机",
                    "BR": "Smartphone",
                    "CZ": "Chytrý telefon",
                    "DA": "Smartphone",
                    "DE": "Smartphone",
                    "ES": "Smartphone",
                    "FI": "Älypuhelin",
                    "FR": "Smartphone",
                    "HU": "Okostelefon",
                    "IT": "Smartphone",
                    "JP": "スマートフォン",
                    "KR": "스마트폰",
                    "MS": "Telefon Pintar",
                    "NL": "Smartphone",
                    "NO": "Smarttelefon",
                    "PL": "Smartfon",
                    "RU": "Смартфон",
                    "SV": "Smarttelefon",
                    "TH": "สมาร์ทโฟน",
                    "TR": "Akıllı Telefon",
                    "UK": "Смартфон",
                    "RO": "Smartphone",
                    "SL": "Pametni telefon"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 8,
                    "type": [
                        "10"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvaVBob25lIj48ZyBpZD0iTW9iaWxlIFBob25lIChpUGhvbmUgU2hhcGUpIj48cGF0aCBpZD0iVmVjdG9yX19fX18wXzBfV1FFRkdHRk1JWCIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xNi4zMzg2IDIwLjMxODFDMTYuMzM4NiAyMS4yNDQxIDE1LjYwMTYgMjIgMTQuNzAzOSAyMkg2LjYzNDY2QzUuNzI3NTcgMjIgNSAyMS4yNDQxIDUgMjAuMzE4MVYzLjY4MTg5QzUgMi43NTU5MSA1LjczNzAyIDIgNi42MzQ2NiAySDE0LjcwMzlDMTUuNjExIDIgMTYuMzM4NiAyLjc1NTkxIDE2LjMzODYgMy42ODE4OVY2LjA5MTM0Ii8+PGcgaWQ9IlZlY3Rvcl9fX19fMF8xX0lYR1pTTUZaTEciPjxwYXRoIGZpbGw9ImJsYWNrIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGQ9Ik0xMC4xOTc1IDQuOTEwN0gxMS4xNDI0WiIgY2xpcC1ydWxlPSJldmVub2RkIi8+PHBhdGggc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBkPSJNMTAuMTk3NSA0LjkxMDdIMTEuMTQyNCIvPjwvZz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzJfRkZET1hDUVVMUSIgZmlsbD0iYmxhY2siIGZpbGwtcnVsZT0iZXZlbm9kZCIgZD0iTTEwLjY2ODUgMTkuODgzN0MxMC4xNDY3IDE5Ljg4MzcgOS43MjM2MyAxOS40NjA2IDkuNzIzNjMgMTguOTM4OEM5LjcyMzYzIDE4LjQxNjkgMTAuMTQ2NyAxNy45OTM5IDEwLjY2ODUgMTcuOTkzOUMxMS4xOTA0IDE3Ljk5MzkgMTEuNjEzNCAxOC40MTY5IDExLjYxMzQgMTguOTM4OEMxMS42MTM0IDE5LjQ2MDYgMTEuMTkwNCAxOS44ODM3IDEwLjY2ODUgMTkuODgzN1oiIGNsaXAtcnVsZT0iZXZlbm9kZCIvPjxnIGlkPSJBcHBsZSI+PGcgaWQ9Ikdyb3VwXzIiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfM19IRVJTREhQVkZNIiBmaWxsPSJibGFjayIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE4LjAxOTkgMTEuNDYzN0wxOC4wMjEzIDExLjQ2MzhDMTguNTczMyAxMS41MDc1IDE4Ljk5ODkgMTEuNjU4NyAxOS4zMjg3IDExLjkxNDhDMTkuMDk4OCAxMi4xMjM2IDE4LjkwMTkgMTIuMzU2NCAxOC43NDUgMTIuNjE3OUMxOC40Njg0IDEzLjA3ODkgMTguMzM1MyAxMy41OTcxIDE4LjMzNTMgMTQuMTY4NkMxOC4zMzUzIDE0LjE2NzcgMTguMzI5MyAxNC4zMTI4IDE4LjM1MjUgMTQuNTE2M0MxOC4zNzU2IDE0LjcxODUgMTguNDI3OCAxNC45ODkgMTguNTQ0MyAxNS4yODgxQzE4LjczOTQgMTUuNzg4NiAxOS4xMDg3IDE2LjM1MDUgMTkuNzgwOCAxNi43OTY3QzE5LjYxMzcgMTcuMTgyNyAxOS4zNzA2IDE3LjY0NjYgMTkuMDc3MSAxOC4wNTRDMTguODY2NiAxOC4zNDY0IDE4LjY0NiAxOC41ODc0IDE4LjQyODkgMTguNzUwNUMxOC4yMTE2IDE4LjkxMzggMTguMDMzMiAxOC45NzMzIDE3Ljg5MTIgMTguOTczM0MxNy42NjQ2IDE4Ljk3MzMgMTcuNDg2OSAxOC45MDUyIDE3LjE3NzIgMTguNzgzMUMxNi44NzYxIDE4LjY2NDQgMTYuNDc3OCAxOC41MDkzIDE1Ljk0NjQgMTguNTA5M0MxNS40MDY0IDE4LjUwOTMgMTQuOTkzNyAxOC42NjA4IDE0LjY4MDkgMTguNzgxNkwxNC42Njg1IDE4Ljc4NjRDMTQuMzQ0IDE4LjkxMTggMTQuMTg0OCAxOC45NzMzIDE0LjAwMTUgMTguOTczM0MxMy45OTM3IDE4Ljk3MzMgMTMuOTg1OSAxOC45NzM1IDEzLjk3ODIgMTguOTczOEMxMy44NjYxIDE4Ljk3ODEgMTMuNjk4NyAxOC45MyAxMy40NzA4IDE4Ljc1MjlDMTMuMjQ2MSAxOC41Nzg0IDEzLjAxMjggMTguMzE2NiAxMi43ODcgMTcuOTk5QzEyLjMzNTEgMTcuMzYzNCAxMS45ODIyIDE2LjYwMjcgMTEuODMwOSAxNi4xNzMzTDExLjgzMDkgMTYuMTczM0wxMS44MjkyIDE2LjE2ODZDMTEuNTk1NSAxNS41MjMgMTEuNTAzMyAxNC45MDM0IDExLjUwMzMgMTQuMjk4M0MxMS41MDMzIDEzLjI5NzkgMTEuODMwNiAxMi42MTA5IDEyLjI2MzQgMTIuMTcyOEMxMi43MDExIDExLjcyOTcgMTMuMjgxNiAxMS41MDU0IDEzLjg0OTkgMTEuNDkzQzE0LjA4MDEgMTEuNDk0MyAxNC4zNzk0IDExLjU3OTcgMTQuNzM3IDExLjcwMjdDMTQuNzgxNCAxMS43MTc5IDE0LjgyNzkgMTEuNzM0MSAxNC44NzUxIDExLjc1MDdDMTQuOTk5MiAxMS43OTQgMTUuMTI5MSAxMS44Mzk0IDE1LjI0MzQgMTEuODc0OUMxNS4zOTExIDExLjkyMDcgMTUuNTk0MyAxMS45Nzc1IDE1Ljc4OTQgMTEuOTc3NUMxNS45OTUgMTEuOTc3NSAxNi4yMTgyIDExLjkwMzggMTYuMzYxNCAxMS44NTUyQzE2LjQzNzMgMTEuODI5NCAxNi41MTg4IDExLjgwMDIgMTYuNTk5IDExLjc3MTRDMTYuNjA5IDExLjc2NzggMTYuNjE5MSAxMS43NjQyIDE2LjYyOTEgMTEuNzYwNkMxNi43MjEgMTEuNzI3NiAxNi44MTQgMTEuNjk0NSAxNi45MSAxMS42NjIzQzE3LjMwNjggMTEuNTI5MiAxNy42ODggMTEuNDM2NyAxOC4wMTk5IDExLjQ2MzdaIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF80X1pNWU1VWUVFRVciIGZpbGw9ImJsYWNrIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTYuOTk5MiA3LjU3OTg1QzE3LjE0NDcgNy41MzcxNSAxNy4yMTU3IDcuNTQ4MDEgMTcuMjM4NSA3LjU1MzRDMTcuMjQ5NiA3LjU3NjEzIDE3LjI4NjMgNy42NjEwNiAxNy4yNTgyIDcuODU3NTlDMTcuMjIzOSA4LjA5NzY5IDE3LjEwNTkgOC4zNDY5MSAxNi45NDk0IDguNTAzNDJMMTYuOTQ5MyA4LjUwMzRMMTYuOTQ2IDguNTA2NzdDMTYuODE2OCA4LjYzODAxIDE2LjYxMDUgOC43NTIyIDE2LjQwOTMgOC43OTg0MkMxNi4yNTU2IDguODMzNzQgMTYuMTgwMiA4LjgxNTg5IDE2LjE1NTIgOC44MDc2QzE2LjE0NDYgOC43ODI1NyAxNi4xMTk0IDguNzAzNyAxNi4xNTA2IDguNTMwOTNDMTYuMTkxIDguMzA3MDUgMTYuMzA4NSA4LjA2Nzk5IDE2LjQ1MzkgNy45MTEyMUMxNi41OTAyIDcuNzY2NTQgMTYuNzk5OCA3LjYzODM1IDE2Ljk5OTIgNy41Nzk4NVoiLz48L2c+PC9nPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Apple iPhone",
                    "TW": "Apple iPhone",
                    "CN": "Apple iPhone",
                    "BR": "Apple iPhone",
                    "CZ": "Apple iPhone",
                    "DA": "Apple iPhone",
                    "DE": "Apple iPhone",
                    "ES": "Apple iPhone",
                    "FI": "Apple iPhone",
                    "FR": "Apple iPhone",
                    "HU": "Apple iPhone",
                    "IT": "Apple iPhone",
                    "JP": "Apple iPhone",
                    "KR": "Apple iPhone",
                    "MS": "Apple iPhone",
                    "NL": "Apple iPhone",
                    "NO": "Apple iPhone",
                    "PL": "Apple iPhone",
                    "RU": "Apple iPhone",
                    "SV": "Apple iPhone",
                    "TH": "Apple iPhone",
                    "TR": "Apple iPhone",
                    "UK": "Apple iPhone",
                    "RO": "Apple iPhone",
                    "SL": "Apple iPhone"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 9,
                    "type": [
                        "28"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvYXN1cyBzbWFydHBob25lIj48ZyBpZD0iR3JvdXAgMzU0MSI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8xX05GUldEWlNLQlkiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1taXRlcmxpbWl0PSIxMCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xNi41ODgyIDEyLjY0NzFWMi41MDMxNEMxNi41ODgyIDIuMjUxNTcgMTYuMzUyOSAyIDE2LjExNzYgMkg2LjQ3MDU5QzYuMjM1MjkgMiA2IDIuMjUxNTcgNiAyLjUwMzE0VjIxLjQ5NjlDNiAyMS43NDg0IDYuMjM1MjkgMjIgNi40NzA1OSAyMkgxNi4xMTc2QzE2LjM1MjkgMjIgMTYuNTg4MiAyMS44NzQyIDE2LjU4ODIgMjEuNDk2OVYxOC42NDcxIi8+PGcgaWQ9IlZlY3Rvcl9fX19fMF8yX0FKWlBTVEpPSlgiPjxwYXRoIGZpbGw9ImJsYWNrIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGQ9Ik0xMC4xMTcyIDIwSDEyLjQ3MDFaIiBjbGlwLXJ1bGU9ImV2ZW5vZGQiLz48cGF0aCBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIGQ9Ik0xMC4xMTcyIDIwSDEyLjQ3MDEiLz48L2c+PHBhdGggaWQ9IlZlY3RvciAyN19fX19fMF8zX01GVUtET0ZIRUMiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNOC45NDExOCAyTDguOTQxMTggMi42NzY0N0M4Ljk0MTE4IDIuOTUyNjEgOS4xNjUwMyAzLjE3NjQ3IDkuNDQxMTggMy4xNzY0N0wxMy4xNDcxIDMuMTc2NDdDMTMuNDIzMiAzLjE3NjQ3IDEzLjY0NzEgMi45NTI2MSAxMy42NDcxIDIuNjc2NDdMMTMuNjQ3MSAyIi8+PGcgaWQ9IkFwcGxlIj48cGF0aCBpZD0iVW5pb25fX19fXzBfNF9UWUpGWFpMTkJXIiBmaWxsPSJibGFjayIgZmlsbC1ydWxlPSJldmVub2RkIiBkPSJNMjEuODc5OCAxMy43NjQ4TDIxLjg4MyAxNC42Njg4TDE3LjkyMyAxNC42ODRMMTcuOTIyOCAxNC42MTEyQzE3LjkyMjggMTQuNjExMiAxNy45OTgyIDE0LjI5NDQgMTguMTMzOCAxNC4xNDM2TDE4LjEzOTggMTQuMTM3QzE4LjI2NjQgMTMuOTk1MyAxOC40Mzc0IDEzLjgwNDEgMTguODE5MiAxMy43NzcxTDIxLjg3OTggMTMuNzY0OFpNMTcuOTEzNCAxMy43ODQ5TDE3LjkxNjUgMTQuNjg0TDE3LjA2MiAxNC42ODc0TDE3LjA1ODggMTMuNzg4M0wxNy45MTM0IDEzLjc4NDlaTTE1LjEwOTUgMTQuNjk1MkwxNS4xMDYzIDEzLjc5NjFMMTQuMjUwOSAxMy43OTk1TDE0LjI1NDEgMTQuNjk4NkwxNS4xMDk1IDE0LjY5NTJaTTguMDUyMDcgMTQuNzc4MUw5LjAwMTU4IDE0Ljg4MTVMNy41OTQ5IDE3LjIxNzJMNi41ODg5MiAxNy4yMjEzTDguMDUyMDcgMTQuNzc4MVpNMTcuOTI5NyAxNi4yODYzTDIwLjkwMTIgMTYuMjc0M0MyMC45Njk3IDE2LjI3NDEgMjEuMDQzNCAxNi4yMzEgMjEuMDQzNCAxNi4yMzFDMjEuMDc0MSAxNi4yMDE0IDIxLjA5OSAxNi4xNDc0IDIxLjA5ODggMTYuMDk1NEMyMS4wOTgzIDE1LjkyMzQgMjAuOTY0MSAxNS45MTYzIDIwLjg5NjYgMTUuOTEzMUMyMC44OTY2IDE1LjkxMzEgMTguODA4NSAxNS43MzcgMTguNzM1NyAxNS43MzFDMTguNzM1NyAxNS43MzEgMTguNDQ2MSAxNS42ODUgMTguMjUyMiAxNS40ODkxTDE4LjIzMyAxNS40NzAxQzE4LjE1MTcgMTUuMzkwNSAxOC4wNTA2IDE1LjI5MTQgMTcuOTcyOSAxNS4wMTA4QzE3Ljk3MjkgMTUuMDEwOCAyMC43NjQ0IDE1LjE3MzYgMjEuMDc0NSAxNS4yMDNDMjEuNTk4IDE1LjI1NDEgMjEuODQ1MyAxNS44MDEyIDIxLjg2OCAxNS45ODM0QzIxLjg2OCAxNS45ODM0IDIxLjg5MSAxNi4xNDgzIDIxLjg2NDUgMTYuMzUwOEMyMS44NjQ1IDE2LjM1MDggMjEuNzU4OCAxNy4xMjE4IDIwLjk5MTEgMTcuMTcxNUwxNy45MzA5IDE3LjE4MzhMMTcuOTI5NyAxNi4yODYzWk0xMS4yNDEgMTMuODA3NkwxNC4xMDE1IDEzLjc5NjFMMTQuMTA0NiAxNC42OTk0TDguMDg1OTQgMTQuNzIzNkM4LjA4NTk0IDE0LjcyMzYgOC40MzEwNiAxNC4xMTY2IDguNTE2ODQgMTMuOTk2OEM4LjU5ODc2IDEzLjg3NTYgOC43MDc3NiAxMy44MjM2IDguODIyNDEgMTMuODIzMUwxMC4zNDExIDEzLjgxN0wxMC4zNDM5IDE0LjY0MTdDMTAuMzQzOSAxNC42NDE3IDEwLjQyIDE0LjMyNSAxMC41NTU3IDE0LjE3NEwxMC41NjI0IDE0LjE2NjVDMTAuNjg4NyAxNC4wMjUxIDEwLjg1ODkgMTMuODM0NSAxMS4yNDEgMTMuODA3NlpNMTQuMjY0NyAxNi4wMTRDMTQuMjQzMiAxNS44ODAxIDE0LjAwMDMgMTUuMjg4OSAxMy40NzcyIDE1LjIzOTVDMTMuMTY1NCAxNS4yMSAxMC4zODk2IDE0Ljk4ODIgMTAuMzg5NiAxNC45ODgyQzEwLjQ0NDMgMTUuMjgyMiAxMC41Njk5IDE1LjQzMDIgMTAuNjUzNCAxNS41MTQ3QzEwLjg0NjcgMTUuNzA5NyAxMS4xNTM0IDE1Ljc2NCAxMS4xNTM0IDE1Ljc2NEMxMS4yMjc1IDE1Ljc3MTMgMTMuMjkyNSAxNS45NDM3IDEzLjI5MjUgMTUuOTQzN0MxMy4zNTgzIDE1Ljk0NyAxMy40ODMxIDE1Ljk2NDMgMTMuNDgyNCAxNi4xMzY3QzEzLjQ4MjUgMTYuMTU3NyAxMy40NjUgMTYuMzA5OCAxMy4zMDY0IDE2LjMxMDNMMTAuMzE0IDE2LjMyMjRMMTAuMzA5MyAxNC45ODFMOS40Nzc2IDE0LjkxOThMOS40ODU2MSAxNy4yMTAzTDEzLjQ4NTggMTcuMTk0MkMxNC4xODE5IDE3LjA0MzcgMTQuMjUyMyAxNi4zNzQgMTQuMjUyMyAxNi4zNzRDMTQuMjYxNiAxNi4zMTY0IDE0LjI2NjIgMTYuMjYzIDE0LjI2ODUgMTYuMjE0OEwxNC4yNjggMTYuMDU0M0MxNC4yNjY0IDE2LjAyOTQgMTQuMjY0NyAxNi4wMTQgMTQuMjY0NyAxNi4wMTRaTTE0LjI2ODEgMTYuMDU0NEMxNC4yNzAzIDE2LjA4OTUgMTQuMjcyMSAxNi4xNDU1IDE0LjI2ODcgMTYuMjE0OUwxNC4yNjkyIDE2LjM2M0MxNC40MDkxIDE3LjEyOTkgMTUuMDYzOCAxNy4xODU4IDE1LjA2MzggMTcuMTg1OEMxNS4wNjM4IDE3LjE4NTggMTUuMTMwNSAxNy4xOTA2IDE1LjE0MTQgMTcuMTkxNUwxNy4wOTE3IDE3LjE4MzdDMTcuMDkxNyAxNy4xODM3IDE3LjkyNTkgMTcuMTA4OSAxNy45MjMgMTYuMjYxM0wxNy45MTg2IDE1LjAwODdMMTcuMDc4MyAxNC45NTg5TDE3LjA4MiAxNi4wMDE3QzE3LjA4MiAxNi4wMDE3IDE3LjA4MDYgMTYuMzAyMiAxNi44MDYxIDE2LjMwMzNMMTUuMzYyMiAxNi4zMDkxQzE1LjM2MjIgMTYuMzA5MSAxNS4xMTc3IDE2LjI4ODcgMTUuMTE2NyAxNi4wMTI1TDE1LjExMjYgMTQuODI1M0wxNC4yNjM2IDE0Ljc2MzhMMTQuMjY4MSAxNi4wNTQ0WiIgY2xpcC1ydWxlPSJldmVub2RkIi8+PC9nPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "ASUS Smartphone",
                    "TW": "華碩智慧型手機",
                    "CN": "ASUS 智能手机",
                    "BR": "Smartphone ASUS",
                    "CZ": "Chytrý telefon ASUS",
                    "DA": "ASUS-smartphone",
                    "DE": "ASUS-Smartphone",
                    "ES": "Smartphone ASUS",
                    "FI": "ASUS-älypuhelin",
                    "FR": "Smartphone ASUS",
                    "HU": "ASUS okostelefon",
                    "IT": "Smartphone ASUS",
                    "JP": "ASUSスマートフォン",
                    "KR": "ASUS 스마트폰",
                    "MS": "Telefon Pintar ASUS",
                    "NL": "ASUS-smartphone",
                    "NO": "ASUS-smarttelefon",
                    "PL": "Smartfon ASUS",
                    "RU": "Смартфон ASUS",
                    "SV": "ASUS-smarttelefon",
                    "TH": "สมาร์ทโฟน ASUS",
                    "TR": "ASUS akıllı telefon",
                    "UK": "Смартфон ASUS",
                    "RO": "Smartphone ASUS",
                    "SL": "Pametni telefon ASUS"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 10,
                    "type": [
                        "19"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvd2luZG93cyBwaG9uZSI+PGcgaWQ9Ikdyb3VwIDM1MzYiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMF9WVldBQkJJSkZIIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbWl0ZXJsaW1pdD0iMTAiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTYuNTg4MiA5LjY0NzA2VjIuNTAzMTRDMTYuNTg4MiAyLjI1MTU3IDE2LjM1MjkgMiAxNi4xMTc2IDJINi40NzA1OUM2LjIzNTI5IDIgNiAyLjI1MTU3IDYgMi41MDMxNFYyMS40OTY5QzYgMjEuNzQ4NCA2LjIzNTI5IDIyIDYuNDcwNTkgMjJIMTYuMTE3NkMxNi4zNTI5IDIyIDE2LjU4ODIgMjEuODc0MiAxNi41ODgyIDIxLjQ5NjlWMTkuNjQ3MSIvPjxwYXRoIGlkPSJWZWN0b3IgMjdfX19fXzBfMV9XWEhEQVhTTFZVIiBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTguOTQxMTggMkw4Ljk0MTE4IDIuNjc2NDdDOC45NDExOCAyLjk1MjYxIDkuMTY1MDMgMy4xNzY0NyA5LjQ0MTE4IDMuMTc2NDdMMTMuMTQ3MSAzLjE3NjQ3QzEzLjQyMzIgMy4xNzY0NyAxMy42NDcxIDIuOTUyNjEgMTMuNjQ3MSAyLjY3NjQ3TDEzLjY0NzEgMiIvPjxnIGlkPSJBcHBsZSI+PHBhdGggaWQ9IlVuaW9uX19fX18wXzJfUldLQ1FUR1JORiIgZmlsbD0iYmxhY2siIGZpbGwtcnVsZT0iZXZlbm9kZCIgZD0iTTIwLjcwNjQgMTUuMjA1N0MxOC42NDQ1IDE1LjIyNjYgMTYuNTgyNiAxNS4yNDczIDE0LjUyMDcgMTUuMjY4VjEwLjg4MkwyMC43MDY0IDkuNjQ3MjJWMTUuMjA1N1pNMTQuNTIwNyAyMC4xMjg5VjE1Ljc2OTdDMTYuNTgyNiAxNS43OTA0IDE4LjY0NDUgMTUuODExMSAyMC43MDY0IDE1LjgzMTlWMjEuMzYzNUMxOC42NDQ1IDIwLjk1MjEgMTYuNTgyNiAyMC41NDA1IDE0LjUyMDcgMjAuMTI4OVpNMTMuOTk3NSAxNS4yNzM3VjEwLjk4NkwxMy45OTQyIDEwLjk4NjZMMTMuOTkyOCAxMC45ODY5QzEyLjcwMTIgMTEuMjQ0NyAxMS40MDk3IDExLjUwMjUgMTAuMTE4MiAxMS43NjA1VjE1LjMxMjdMMTMuOTk3NSAxNS4yNzM3Wk0xMC4xMTgyIDE1LjcyNDlMMTMuOTk3NSAxNS43NjM5VjIwLjAyNDZMMTAuMTE4MiAxOS4yNTAzVjE1LjcyNDlaIiBjbGlwLXJ1bGU9ImV2ZW5vZGQiLz48L2c+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "Windows Phone",
                    "TW": "微軟智慧型手機",
                    "CN": "Windows 手机",
                    "BR": "Windows Phone",
                    "CZ": "Chytrý telefon Windows",
                    "DA": "Windows Phone",
                    "DE": "Windows-Telefon",
                    "ES": "Teléfono Windows",
                    "FI": "Windows Phone",
                    "FR": "Windows Phone",
                    "HU": "Windows Phone",
                    "IT": "Windows Phone",
                    "JP": "Windows Phone",
                    "KR": "Windows 폰",
                    "MS": "Telefon Windows",
                    "NL": "Windows-telefoon",
                    "NO": "Windows Phone",
                    "PL": "Windows Phone",
                    "RU": "Телефон Windows",
                    "SV": "Windows Phone",
                    "TH": "Windows Phone",
                    "TR": "Windows Phone",
                    "UK": "Телефон з Windows",
                    "RO": "Telefon Windows",
                    "SL": "Telefon Windows"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 11,
                    "type": [
                        "9"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvYW5kcm9pZCBwaG9uZSI+PGcgaWQ9Ikdyb3VwIDM1NDMiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMF9IUFVCUlFCSklIIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbWl0ZXJsaW1pdD0iMTAiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTcuMTE3NiAyMkg3LjQ3MDU5QzcuMjM1MjkgMjIgNyAyMS43NDg0IDcgMjEuNDk2OVYyLjUwMzE0QzcgMi4yNTE1NyA3LjIzNTI5IDIgNy40NzA1OSAySDE3LjExNzZDMTcuMzUyOSAyIDE3LjU4ODIgMi4yNTE1NyAxNy41ODgyIDIuNTAzMTRWMjEuNDk2OUMxNy41ODgyIDIxLjg3NDIgMTcuMzUyOSAyMiAxNy4xMTc2IDIyWiIvPjxwYXRoIGlkPSJWZWN0b3IgMjdfX19fXzBfMV9CV0xJQllYS0NNIiBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTkuOTQxMTggMkw5Ljk0MTE4IDIuNjc2NDdDOS45NDExOCAyLjk1MjYxIDEwLjE2NSAzLjE3NjQ3IDEwLjQ0MTIgMy4xNzY0N0wxNC4xNDcxIDMuMTc2NDdDMTQuNDIzMiAzLjE3NjQ3IDE0LjY0NzEgMi45NTI2MSAxNC42NDcxIDIuNjc2NDdMMTQuNjQ3MSAyIi8+PGcgaWQ9IkFwcGxlIj48cGF0aCBpZD0iU3VidHJhY3RfX19fXzBfMl9VWEVDR1lGV0VNIiBmaWxsPSJibGFjayIgZmlsbC1ydWxlPSJldmVub2RkIiBkPSJNOC42NTE2NiAxNS43MzUyQzguODQwNTQgMTUuNjAzIDkuMTAwODMgMTUuNjQ4OSA5LjIzMzA0IDE1LjgzNzhMMTAuNDYyMiAxNy41OTM4QzExLjA5MzYgMTcuMzcwMiAxMS43ODYzIDE3LjI0MjIgMTIuNTE0NSAxNy4yNDIyQzEzLjIxNTIgMTcuMjQyMiAxMy44ODMxIDE3LjM2MDcgMTQuNTAwNyAxNy41Njg4TDE1LjcxMjQgMTUuODM3OEMxNS44NDQ2IDE1LjY0ODkgMTYuMTA0OSAxNS42MDMgMTYuMjkzOCAxNS43MzUyQzE2LjQ4MjcgMTUuODY3NCAxNi41Mjg2IDE2LjEyNzcgMTYuMzk2NCAxNi4zMTY2TDE1LjI4OSAxNy44OTg2QzE2Ljc3MjkgMTguNjYxOSAxNy44MDkgMjAuMDE5NiAxNy45NDE0IDIxLjU5OTJINy4wODc3MkM3LjIxODA2IDIwLjA0MzMgOC4yMjUyNiAxOC43MDI3IDkuNjgwODIgMTcuOTMzNEw4LjU0OTA3IDE2LjMxNjZDOC40MTY4NiAxNi4xMjc3IDguNDYyNzkgMTUuODY3NCA4LjY1MTY2IDE1LjczNTJaTTEwLjQzNTcgMTkuOTI2MUMxMC43ODE1IDE5LjkyNjEgMTEuMDYxOCAxOS42NDU3IDExLjA2MTggMTkuMjk5OUMxMS4wNjE4IDE4Ljk1NDEgMTAuNzgxNSAxOC42NzM3IDEwLjQzNTcgMTguNjczN0MxMC4wODk4IDE4LjY3MzcgOS44MDk0OCAxOC45NTQxIDkuODA5NDggMTkuMjk5OUM5LjgwOTQ4IDE5LjY0NTcgMTAuMDg5OCAxOS45MjYxIDEwLjQzNTcgMTkuOTI2MVpNMTUuMjg2MyAxOS4yOTk5QzE1LjI4NjMgMTkuNjQ1NyAxNS4wMDYgMTkuOTI2MSAxNC42NjAyIDE5LjkyNjFDMTQuMzE0MyAxOS45MjYxIDE0LjAzNCAxOS42NDU3IDE0LjAzNCAxOS4yOTk5QzE0LjAzNCAxOC45NTQxIDE0LjMxNDMgMTguNjczNyAxNC42NjAyIDE4LjY3MzdDMTUuMDA2IDE4LjY3MzcgMTUuMjg2MyAxOC45NTQxIDE1LjI4NjMgMTkuMjk5OVoiIGNsaXAtcnVsZT0iZXZlbm9kZCIvPjwvZz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Android Phone",
                    "TW": "安卓智慧型手機",
                    "CN": "Android 手机",
                    "BR": "Telefone Android",
                    "CZ": "Chytrý telefon Android",
                    "DA": "Android Phone",
                    "DE": "Android-Telefon",
                    "ES": "Teléfono Android",
                    "FI": "Android-puhelin",
                    "FR": "Téléphone Android",
                    "HU": "Androidos telefon",
                    "IT": "Telefono Android",
                    "JP": "Android Phone",
                    "KR": "Android 폰",
                    "MS": "Telefon Android",
                    "NL": "Android-telefoon",
                    "NO": "Android-telefon",
                    "PL": "Telefon Android",
                    "RU": "Телефон Android",
                    "SV": "Android-telefon",
                    "TH": "โทรศัพท์ Android",
                    "TR": "Android Phone",
                    "UK": "Телефон з Android",
                    "RO": "Telefon Android",
                    "SL": "Telefon Android"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 12,
                    "type": [
                        "73"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvcGFkIj48ZyBpZD0iR3JvdXAgMzUyOSI+PHBhdGggaWQ9IlJlY3RhbmdsZSA3MV9fX19fMF8wX0xPTkhCTkhIUVgiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMy45MTQ1NCA2LjZIMTkuMjUwMUMxOS4zMDk3IDYuNiAxOS4zNTYgNi42NTE4MSAxOS4zNDk1IDYuNzExMDRMMTguMjE3IDE2LjkwMzFDMTguMjExNCAxNi45NTM4IDE4LjE2ODYgMTYuOTkyMSAxOC4xMTc2IDE2Ljk5MjFIMi43ODIwOUMyLjcyMjQ4IDE2Ljk5MjEgMi42NzYxMiAxNi45NDAzIDIuNjgyNyAxNi44ODExTDMuODE1MTUgNi42ODg5NkMzLjgyMDc4IDYuNjM4MzEgMy44NjM1OSA2LjYgMy45MTQ1NCA2LjZaIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8xX0VSSEhCWVpBQlEiIGZpbGw9ImJsYWNrIiBkPSJNMTguNzQzNiAxNi4zMDRDMTkuOTY0NCAxNi4zMDQgMjEuMzE5NiAxNi4zMDQgMjEuMzE5NiAxNi4zMDRDMjEuOTYzNiAxNi4zMDQgMjIuMDY5OSAxNS45NDU1IDIxLjk2MzYgMTUuNjZDMjEuODU3NCAxNS4zNzQ1IDE5LjM4NzYgOS4yMTk5MyAxOS4zODc2IDkuMjE5OTMiLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Pad",
                    "TW": "平板電腦",
                    "CN": "平板",
                    "BR": "Tablet",
                    "CZ": "Pad",
                    "DA": "Pad",
                    "DE": "Pad",
                    "ES": "Tablet",
                    "FI": "Pad",
                    "FR": "Tampon",
                    "HU": "Pad",
                    "IT": "Tablet",
                    "JP": "パッド",
                    "KR": "패드",
                    "MS": "Pad",
                    "NL": "Tablet",
                    "NO": "Nettbrett",
                    "PL": "Pad",
                    "RU": "Pad",
                    "SV": "Platta",
                    "TH": "แท็บเล็ต",
                    "TR": "Pad",
                    "UK": "Планшет",
                    "RO": "Pad",
                    "SL": "Tablica"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 13,
                    "type": [
                        "21"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48cGF0aCBzdHJva2U9IiMwMDAiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTguMzM5IDIwLjMxOGMwIC45MjYtLjczNyAxLjY4Mi0xLjYzNSAxLjY4MkgzLjYzNEMyLjcyOSAyMiAyIDIxLjI0NCAyIDIwLjMxOFYzLjY4MkMyIDIuNzU2IDIuNzM3IDIgMy42MzUgMmgxMy4wNjljLjkwNyAwIDEuNjM1Ljc1NiAxLjYzNSAxLjY4MnYyLjQxIi8+PHBhdGggZmlsbD0iIzAwMCIgZmlsbC1ydWxlPSJldmVub2RkIiBkPSJNOC44IDMuOTFoMi45NDVaIiBjbGlwLXJ1bGU9ImV2ZW5vZGQiLz48cGF0aCBzdHJva2U9IiMwMDAiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgZD0iTTguOCAzLjkxaDIuOTQ1Ii8+PHBhdGggZmlsbD0iIzAwMCIgZmlsbC1ydWxlPSJldmVub2RkIiBkPSJNMTAuMjQ1IDIwLjg4NGEuOTQ1Ljk0NSAwIDEgMSAwLTEuODkuOTQ1Ljk0NSAwIDAgMSAwIDEuODlaIiBjbGlwLXJ1bGU9ImV2ZW5vZGQiLz48cGF0aCBmaWxsPSIjMDAwIiBzdHJva2U9IiMwMDAiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0yMC4wMiAxMS40NjRoLjAwMWMuNTUyLjA0My45NzguMTk1IDEuMzA4LjQ1LS4yMy4yMS0uNDI3LjQ0Mi0uNTg0LjcwNC0uMjc3LjQ2LS40MS45OC0uNDEgMS41NSAwIDAtLjAwNi4xNDUuMDE3LjM0OC4wMjQuMjAzLjA3Ni40NzMuMTkyLjc3MmEzLjE5IDMuMTkgMCAwIDAgMS4yMzcgMS41MDljLS4xNjcuMzg2LS40MS44NS0uNzA0IDEuMjU3LS4yMS4yOTItLjQzMS41MzMtLjY0OC42OTctLjIxNy4xNjMtLjM5Ni4yMjItLjUzOC4yMjItLjIyNiAwLS40MDQtLjA2OC0uNzE0LS4xOS0uMy0uMTE5LS43LS4yNzQtMS4yMy0uMjc0LS41NCAwLS45NTMuMTUyLTEuMjY2LjI3M2wtLjAxMi4wMDRjLS4zMjUuMTI2LS40ODQuMTg3LS42NjguMTg3YS42LjYgMCAwIDAtLjAyMyAwYy0uMTEyLjAwNS0uMjgtLjA0My0uNTA3LS4yMi0uMjI1LS4xNzUtLjQ1OC0uNDM2LS42ODQtLjc1NGE4LjIxNiA4LjIxNiAwIDAgMS0uOTU2LTEuODI2aDBsLS4wMDItLjAwNGE1LjQxNyA1LjQxNyAwIDAgMS0uMzI2LTEuODdjMC0xLjAwMS4zMjgtMS42ODguNzYtMi4xMjZhMi4zMDMgMi4zMDMgMCAwIDEgMS41ODctLjY4Yy4yMy4wMDEuNTMuMDg3Ljg4Ny4yMWwuMTM4LjA0OGMuMTI0LjA0My4yNTQuMDg4LjM2OC4xMjQuMTQ4LjA0Ni4zNTEuMTAyLjU0Ni4xMDIuMjA2IDAgLjQzLS4wNzMuNTcyLS4xMjJsLjIzOC0uMDg0LjAzLS4wMS4yODEtLjA5OWMuMzk3LS4xMzMuNzc4LS4yMjUgMS4xMS0uMTk4Wk0xOSA3LjU4YS40ODQuNDg0IDAgMCAxIC4yMzgtLjAyN2MuMDEyLjAyMy4wNDguMTA4LjAyLjMwNS0uMDM0LjI0LS4xNTIuNDg5LS4zMDkuNjQ1aDBsLS4wMDMuMDA0YTEuMTYgMS4xNiAwIDAgMS0uNTM3LjI5MWMtLjE1My4wMzYtLjIyOS4wMTgtLjI1NC4wMS0uMDEtLjAyNS0uMDM2LS4xMDQtLjAwNC0uMjc3LjA0LS4yMjQuMTU3LS40NjMuMzAzLS42Mi4xMzYtLjE0NC4zNDYtLjI3My41NDUtLjMzMVoiLz48L3N2Zz4=",
                    "EN": "Apple iPad",
                    "TW": "Apple iPad",
                    "CN": "Apple iPad",
                    "BR": "Apple iPad",
                    "CZ": "Apple iPad",
                    "DA": "Apple iPad",
                    "DE": "Apple iPad",
                    "ES": "Apple iPad",
                    "FI": "Apple iPad",
                    "FR": "Apple iPad",
                    "HU": "Apple iPad",
                    "IT": "Apple iPad",
                    "JP": "Apple iPad",
                    "KR": "Apple iPad",
                    "MS": "Apple iPad",
                    "NL": "Apple iPad",
                    "NO": "Apple iPad",
                    "PL": "Apple iPad",
                    "RU": "Apple iPad",
                    "SV": "Apple iPad",
                    "TH": "Apple iPad",
                    "TR": "Apple iPad",
                    "UK": "Apple iPad",
                    "RO": "Apple iPad",
                    "SL": "Apple iPad"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 14,
                    "type": [
                        "29"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvYXN1cyBwYWQiPjxnIGlkPSJHcm91cCAzNTQyIj48ZyBpZD0iR3JvdXAgMzU0OSI+PHBhdGggaWQ9IlJlY3RhbmdsZSA3MV9fX19fMF8wX01FQUxHRFlFT0UiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMjAuNDg5MyAxNS4yOTYzSDQuNzk4ODdDNC40NDIxOCAxNS4yOTYzIDQuMTQyNTQgMTUuMDI4MSA0LjEwMzE1IDE0LjY3MzZMMy4wMDM1NiA0Ljc3NzNDMi45NTc0OSA0LjM2MjY1IDMuMjgyMDcgNCAzLjY5OTI4IDRIMTguNjA5NkMxOC45NjU0IDQgMTkuMjY0NiA0LjI2Njk3IDE5LjMwNSA0LjYyMDUyTDE5LjczNjIgOC4zOTMwMiIvPjxwYXRoIGlkPSJWZWN0b3IgMTA3X19fX18wXzFfQlNXTkZBQ0lJQSIgZmlsbD0iYmxhY2siIGQ9Ik00Ljg5NzA1IDE2LjU1MTVIMjAuMDc1NUMyMC4zNDA3IDE2LjU1MTUgMjAuNTk1MSAxNi42NTY5IDIwLjc4MjYgMTYuODQ0NEwyMi40ODc5IDE4LjU0OTdDMjIuNjc2OSAxOC43Mzg3IDIyLjU0MyAxOS4wNjE4IDIyLjI3NTcgMTkuMDYxOEg2LjgwNzM0QzYuNzI3NzggMTkuMDYxOCA2LjY1MTQ3IDE5LjAzMDIgNi41OTUyMSAxOC45NzM5TDQuNjg0OTIgMTcuMDYzNkM0LjQ5NTkzIDE2Ljg3NDcgNC42Mjk3OCAxNi41NTE1IDQuODk3MDUgMTYuNTUxNVoiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzJfVUxYU1BCWFpYTyIgZmlsbD0iYmxhY2siIGQ9Ik00LjE3Mjg1IDE0LjA0MTFDMi45ODMyMyAxNC4wNDExIDEuNjYyNTYgMTQuMDQxMSAxLjY2MjU2IDE0LjA0MTFDMS4wMzQ5OCAxNC4wNDExIDAuOTMxNDM5IDEzLjY5MTcgMS4wMzQ5OCAxMy40MTM2QzEuMTM4NTMgMTMuMTM1NCAzLjU0NTI4IDcuMTM3ODIgMy41NDUyOCA3LjEzNzgyIi8+PC9nPjxnIGlkPSJBcHBsZSI+PHBhdGggaWQ9IlVuaW9uX19fX18wXzNfVE1TVkFNQVFVUyIgZmlsbD0iYmxhY2siIGZpbGwtcnVsZT0iZXZlbm9kZCIgZD0iTTIyLjMxMzUgOS42NDgxOUwyMi4zMTY5IDEwLjYxMjZMMTguMDkyMSAxMC42Mjg5TDE4LjA5MTggMTAuNTUxMkMxOC4wOTE4IDEwLjU1MTIgMTguMTcyMyAxMC4yMTMyIDE4LjMxNyAxMC4wNTIzTDE4LjMyMzMgMTAuMDQ1MkMxOC40NTg0IDkuODk0MTMgMTguNjQwOSA5LjY5MDA3IDE5LjA0ODIgOS42NjEzMkwyMi4zMTM1IDkuNjQ4MTlaTTE4LjA4MTggOS42Njk2NEwxOC4wODUxIDEwLjYyODhMMTcuMTczNSAxMC42MzI0TDE3LjE3MDEgOS42NzMyOEwxOC4wODE4IDkuNjY5NjRaTTE1LjA5MDMgMTAuNjQwN0wxNS4wODY5IDkuNjgxNTlMMTQuMTc0NCA5LjY4NTI0TDE0LjE3NzggMTAuNjQ0NEwxNS4wOTAzIDEwLjY0MDdaTTcuNTYwOTkgMTAuNzI5M0w4LjU3NCAxMC44Mzk2TDcuMDczMjUgMTMuMzMxNUw2IDEzLjMzNThMNy41NjA5OSAxMC43MjkzWk0xOC4wOTkyIDEyLjMzODNMMjEuMjY5NCAxMi4zMjU1QzIxLjM0MjQgMTIuMzI1MyAyMS40MjExIDEyLjI3OTMgMjEuNDIxMSAxMi4yNzkzQzIxLjQ1MzkgMTIuMjQ3NyAyMS40ODA0IDEyLjE5MDEgMjEuNDgwMyAxMi4xMzQ3QzIxLjQ3OTYgMTEuOTUxMSAyMS4zMzY1IDExLjk0MzYgMjEuMjY0NSAxMS45NDAxQzIxLjI2NDUgMTEuOTQwMSAxOS4wMzY3IDExLjc1MjMgMTguOTU5MSAxMS43NDU5QzE4Ljk1OTEgMTEuNzQ1OSAxOC42NTAyIDExLjY5NjggMTguNDQzMyAxMS40ODc4TDE4LjQyMjggMTEuNDY3NkMxOC4zMzYxIDExLjM4MjYgMTguMjI4MSAxMS4yNzY4IDE4LjE0NTMgMTAuOTc3NUMxOC4xNDUzIDEwLjk3NzUgMjEuMTIzNSAxMS4xNTExIDIxLjQ1NDMgMTEuMTgyNkMyMi4wMTI4IDExLjIzNzEgMjIuMjc2NyAxMS44MjA3IDIyLjMwMDkgMTIuMDE1MUMyMi4zMDA5IDEyLjAxNTEgMjIuMzI1NSAxMi4xOTExIDIyLjI5NzEgMTIuNDA3MUMyMi4yOTcxIDEyLjQwNzEgMjIuMTg0NCAxMy4yMjk2IDIxLjM2NTMgMTMuMjgyN0wxOC4xMDA1IDEzLjI5NThMMTguMDk5MiAxMi4zMzgzWk0xMC45NjMyIDkuNjkzODdMMTQuMDE0OSA5LjY4MTU5TDE0LjAxODMgMTAuNjQ1Mkw3LjU5NzEzIDEwLjY3MTFDNy41OTcxMyAxMC42NzExIDcuOTY1MzMgMTAuMDIzNSA4LjA1Njg0IDkuODk1NzFDOC4xNDQyNCA5Ljc2NjQzIDguMjYwNTQgOS43MTA4OSA4LjM4Mjg1IDkuNzEwMzhMMTAuMDAzMSA5LjcwMzg2TDEwLjAwNjEgMTAuNTgzN0MxMC4wMDYxIDEwLjU4MzcgMTAuMDg3MyAxMC4yNDU4IDEwLjIzMiAxMC4wODQ4TDEwLjIzOTIgMTAuMDc2OEMxMC4zNzM5IDkuOTI1ODQgMTAuNTU1NSA5LjcyMjQ5IDEwLjk2MzIgOS42OTM4N1pNMTQuMTg4NyAxMi4wNDc3QzE0LjE2NTggMTEuOTA0OCAxMy45MDY2IDExLjI3NCAxMy4zNDg2IDExLjIyMTRDMTMuMDE1OSAxMS4xODk5IDEwLjA1NDUgMTAuOTUzMyAxMC4wNTQ1IDEwLjk1MzNDMTAuMTEyOSAxMS4yNjY5IDEwLjI0NjkgMTEuNDI0OCAxMC4zMzU5IDExLjUxNUMxMC41NDIyIDExLjcyMyAxMC44Njk0IDExLjc4MSAxMC44Njk0IDExLjc4MUMxMC45NDg1IDExLjc4ODggMTMuMTUxNiAxMS45NzI3IDEzLjE1MTYgMTEuOTcyN0MxMy4yMjE3IDExLjk3NjIgMTMuMzU0OSAxMS45OTQ2IDEzLjM1NDEgMTIuMTc4NkMxMy4zNTQyIDEyLjIwMSAxMy4zMzU1IDEyLjM2MzIgMTMuMTY2NCAxMi4zNjM4TDkuOTczODQgMTIuMzc2N0w5Ljk2ODg2IDEwLjk0NTVMOS4wODE0OSAxMC44ODA0TDkuMDkwMDMgMTMuMzI0TDEzLjM1NzcgMTMuMzA2OEMxNC4xMDA0IDEzLjE0NjMgMTQuMTc1NSAxMi40MzE3IDE0LjE3NTUgMTIuNDMxN0MxNC4xODU0IDEyLjM3MDQgMTQuMTkwMyAxMi4zMTMzIDE0LjE5MjggMTIuMjYxOUwxNC4xOTIyIDEyLjA5MDdDMTQuMTkwNSAxMi4wNjQxIDE0LjE4ODcgMTIuMDQ3NyAxNC4xODg3IDEyLjA0NzdaTTE0LjE5MjcgMTIuMDkwOUMxNC4xOTUxIDEyLjEyODQgMTQuMTk3IDEyLjE4ODEgMTQuMTkzMyAxMi4yNjIxTDE0LjE5MzkgMTIuNDIwMUMxNC4zNDMyIDEzLjIzODMgMTUuMDQxNyAxMy4yOTggMTUuMDQxNyAxMy4yOThDMTUuMDQxNyAxMy4yOTggMTUuMTEyOCAxMy4zMDMxIDE1LjEyNDQgMTMuMzA0MUwxNy4yMDUxIDEzLjI5NTdDMTcuMjA1MSAxMy4yOTU3IDE4LjA5NTIgMTMuMjE1OSAxOC4wOTIgMTIuMzExN0wxOC4wODczIDEwLjk3NTJMMTcuMTkwOSAxMC45MjIyTDE3LjE5NDggMTIuMDM0NkMxNy4xOTQ4IDEyLjAzNDYgMTcuMTkzMyAxMi4zNTUzIDE2LjkwMDUgMTIuMzU2NEwxNS4zNiAxMi4zNjI2QzE1LjM2IDEyLjM2MjYgMTUuMDk5MiAxMi4zNDA5IDE1LjA5ODEgMTIuMDQ2MUwxNS4wOTM3IDEwLjc3OTZMMTQuMTg3OSAxMC43MTRMMTQuMTkyNyAxMi4wOTA5WiIgY2xpcC1ydWxlPSJldmVub2RkIi8+PC9nPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "ASUS Pad",
                    "TW": "華碩平板電腦",
                    "CN": "ASUS 平板",
                    "BR": "Tablet ASUS",
                    "CZ": "ASUS pad",
                    "DA": "ASUS-pad",
                    "DE": "ASUS-Pad",
                    "ES": "Tablet ASUS",
                    "FI": "ASUS pad",
                    "FR": "ASUS pad",
                    "HU": "ASUS pad",
                    "IT": "Tablet ASUS",
                    "JP": "ASUSパッド",
                    "KR": "ASUS 패드",
                    "MS": "Pad ASUS",
                    "NL": "ASUS-tablet",
                    "NO": "ASUS-nettbrett",
                    "PL": "Pad ASUS",
                    "RU": "Планшет ASUS",
                    "SV": "ASUS-platta",
                    "TH": "แท็บเล็ต ASUS",
                    "TR": "ASUS pad",
                    "UK": "Планшет ASUS",
                    "RO": "Pad ASUS",
                    "SL": "Tablica ASUS"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 15,
                    "type": [
                        "20"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvYW5kcm9pZCB0YWJsZXQiPjxnIGlkPSJHcm91cCAzNTI5Ij48cGF0aCBpZD0iUmVjdGFuZ2xlIDcxX19fX18wXzBfR1lERU1ZSUVERSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIxLjIyIiBkPSJNMy45MTQ1NCA2LjYxSDE5LjI1MDFDMTkuMzAzNyA2LjYxIDE5LjM0NTQgNi42NTY2MyAxOS4zMzk1IDYuNzA5OTRMMTguMjA3MSAxNi45MDJDMTguMjAyIDE2Ljk0NzYgMTguMTYzNSAxNi45ODIxIDE4LjExNzYgMTYuOTgyMUgyLjc4MjA5QzIuNzI4NDQgMTYuOTgyMSAyLjY4NjcxIDE2LjkzNTUgMi42OTI2NCAxNi44ODIyTDMuODI1MDkgNi42OTAwNkMzLjgzMDE2IDYuNjQ0NDggMy44Njg2OCA2LjYxIDMuOTE0NTQgNi42MVoiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzFfRFpMSlhJSktQWCIgZmlsbD0iYmxhY2siIGQ9Ik0xOC43NDM2IDE2LjMwNEMxOS45NjQ0IDE2LjMwNCAyMS4zMTk2IDE2LjMwNCAyMS4zMTk2IDE2LjMwNEMyMS45NjM2IDE2LjMwNCAyMi4wNjk5IDE1Ljk0NTUgMjEuOTYzNiAxNS42NkMyMS44NTc0IDE1LjM3NDUgMTkuMzg3NiA5LjIxOTkzIDE5LjM4NzYgOS4yMTk5MyIvPjxwYXRoIGlkPSJTdWJ0cmFjdF9fX19fMF8yX1NaVkVWQUNOR1giIGZpbGw9ImJsYWNrIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGQ9Ik02LjI4ODE2IDEwLjA3NThDNi40OTQ5NCA5LjkzMTA2IDYuNzc5OTEgOS45ODEzNSA2LjkyNDY2IDEwLjE4ODFMOC4yNzAzNyAxMi4xMTA2QzguOTYxNiAxMS44NjU4IDkuNzE5OTggMTEuNzI1NyAxMC41MTczIDExLjcyNTdDMTEuMjg0NCAxMS43MjU3IDEyLjAxNTUgMTEuODU1NCAxMi42OTE4IDEyLjA4MzNMMTQuMDE4NCAxMC4xODgxQzE0LjE2MzEgOS45ODEzNSAxNC40NDgxIDkuOTMxMDYgMTQuNjU0OSAxMC4wNzU4QzE0Ljg2MTYgMTAuMjIwNiAxNC45MTE5IDEwLjUwNTUgMTQuNzY3MiAxMC43MTIzTDEzLjU1NDggMTIuNDQ0M0MxNS4xNzk0IDEzLjI4IDE2LjMxMzcgMTQuNzY2NCAxNi40NTg2IDE2LjQ5NThINC41NzU5NEM0LjcxODY0IDE0Ljc5MjMgNS44MjEzMyAxMy4zMjQ3IDcuNDE0ODkgMTIuNDgyNEw2LjE3NTg0IDEwLjcxMjNDNi4wMzEwOSAxMC41MDU1IDYuMDgxMzggMTAuMjIwNiA2LjI4ODE2IDEwLjA3NThaTTguMjQxMjkgMTQuNjY0QzguNjE5OTEgMTQuNjY0IDguOTI2ODMgMTQuMzU3MSA4LjkyNjgzIDEzLjk3ODVDOC45MjY4MyAxMy41OTk4IDguNjE5OTEgMTMuMjkyOSA4LjI0MTI5IDEzLjI5MjlDNy44NjI2OCAxMy4yOTI5IDcuNTU1NzYgMTMuNTk5OCA3LjU1NTc2IDEzLjk3ODVDNy41NTU3NiAxNC4zNTcxIDcuODYyNjggMTQuNjY0IDguMjQxMjkgMTQuNjY0Wk0xMy41NTE5IDEzLjk3ODVDMTMuNTUxOSAxNC4zNTcxIDEzLjI0NDkgMTQuNjY0IDEyLjg2NjMgMTQuNjY0QzEyLjQ4NzcgMTQuNjY0IDEyLjE4MDggMTQuMzU3MSAxMi4xODA4IDEzLjk3ODVDMTIuMTgwOCAxMy41OTk4IDEyLjQ4NzcgMTMuMjkyOSAxMi44NjYzIDEzLjI5MjlDMTMuMjQ0OSAxMy4yOTI5IDEzLjU1MTkgMTMuNTk5OCAxMy41NTE5IDEzLjk3ODVaIiBjbGlwLXJ1bGU9ImV2ZW5vZGQiLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Android Tablet",
                    "TW": "Android平板電腦",
                    "CN": "Android 平板电脑",
                    "BR": "Tablet Android",
                    "CZ": "Tablet Android",
                    "DA": "Android Tablet",
                    "DE": "Android-Tablet",
                    "ES": "Tablet Android",
                    "FI": "Android-tabletti",
                    "FR": "Tablette Android",
                    "HU": "Androidos tablet",
                    "IT": "Tablet Android",
                    "JP": "Androidタブレット",
                    "KR": "Android 태블릿",
                    "MS": "Tablet Android",
                    "NL": "Android-tablet",
                    "NO": "Android-nettbrett",
                    "PL": "Tablet z systemem Android",
                    "RU": "Планшет Android",
                    "SV": "Android-surfplatta",
                    "TH": "แท็บเล็ต Android",
                    "TR": "Android Tablet",
                    "UK": "Планшет з Android",
                    "RO": "Tabletă Andriod",
                    "SL": "Tablični računalnik Android"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 16,
                    "type": [
                        "2",
                        "3"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvcm91dGVyIj48ZyBpZD0iR3JvdXBfMiI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8xX0ZBWFFYUkJLQVoiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0zLjg5OTk5IDE0LjI5QzMuMjY5OTkgMTQuMjkgMi43NSAxNC44IDIuNzUgMTUuNDRWMTguMjhDMi43NSAxOC45MSAzLjI1OTk5IDE5LjQzIDMuODk5OTkgMTkuNDNIMjAuMUMyMC43MyAxOS40MyAyMS4yNSAxOC45MiAyMS4yNSAxOC4yOFYxNS40NEMyMS4yNSAxNC44IDIwLjc0IDE0LjI5IDIwLjEgMTQuMjlIMy44OTk5OVoiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzJfT1pHRkFWUFpOTiIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTUuODMgMTkuNDJWMjAuNDUiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzNfTFpJWVJUVVdMSyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE4LjE3IDE5LjQyVjIwLjQ1Ii8+PGcgaWQ9Ikdyb3VwXzMiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfNF9IS1RDWlFQSERWIiBmaWxsPSJibGFjayIgZD0iTTE3LjE2IDE2Ljg1QzE3LjE2IDE2LjI4IDE3LjYxIDE1LjgyIDE4LjE4IDE1LjgySDE4LjE5QzE4Ljc2IDE1LjgyIDE5LjIyIDE2LjI4IDE5LjIyIDE2Ljg1QzE5LjIyIDE3LjQyIDE4Ljc1IDE3Ljg4IDE4LjE5IDE3Ljg4QzE3LjYyIDE3Ljg4IDE3LjE2IDE3LjQyIDE3LjE2IDE2Ljg1WiIvPjwvZz48ZyBpZD0iR3JvdXBfNCI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF81X05KUkFJQkJPRlgiIGZpbGw9ImJsYWNrIiBkPSJNMTQuMDcwMyAxNi44NTAxQzE0LjA3MDMgMTYuMjgwMSAxNC41MjAzIDE1LjgyMDEgMTUuMDkwMyAxNS44MjAxSDE1LjEwMDNDMTUuNjcwMyAxNS44MjAxIDE2LjEzMDMgMTYuMjgwMSAxNi4xMzAzIDE2Ljg1MDFDMTYuMTMwMyAxNy40MjAxIDE1LjY3MDMgMTcuODgwMSAxNS4xMDAzIDE3Ljg4MDFDMTQuNTMwMyAxNy44ODAxIDE0LjA3MDMgMTcuNDIwMSAxNC4wNzAzIDE2Ljg1MDFaIi8+PC9nPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfNl9YU1hRT05XVkpGIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNNS44MzAyOCAxNC4yOTAxTDMuNzgwMjcgNi4wNjAwNiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfN19JSlJLQkJQTUtPIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTguMTcgMTQuMjlMMjAuMjIgNi4wNiIvPjxnIGlkPSJHcm91cF81Ij48cGF0aCBpZD0iVmVjdG9yX19fX18wXzhfS0pLQ1BSVldTTyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTguODYwMzUgNC45MzAwMkM5LjgxMDM1IDQuMzQwMDIgMTAuOTIwNCA0IDEyLjEwMDQgNEMxMy4yMTA0IDQgMTQuMjUwNCA0LjI5MDAyIDE1LjE0MDQgNC44MDAwMiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfOV9VT0ZVTUFIWVpDIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTAuNDcwNyA3LjU0OTk5QzEwLjk0MDcgNy4yNDk5OSAxMS41MDA3IDcuMDg5OTcgMTIuMDkwNyA3LjA4OTk3QzEyLjY1MDcgNy4wODk5NyAxMy4xNzA3IDcuMjI5OTkgMTMuNjEwNyA3LjQ4OTk5Ii8+PGcgaWQ9Ikdyb3VwXzYiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMTBfTVhUVkxVS0JKTiIgZmlsbD0iYmxhY2siIGQ9Ik0xMS4wOTA4IDEwLjE3QzExLjA5MDggOS42MDAwMSAxMS41NDA4IDkuMTQwMDEgMTIuMTEwOCA5LjE0MDAxSDEyLjEyMDhDMTIuNjkwOCA5LjE0MDAxIDEzLjE1MDggOS42MDAwMSAxMy4xNTA4IDEwLjE3QzEzLjE1MDggMTAuNzQgMTIuNjkwOCAxMS4yIDEyLjEyMDggMTEuMkMxMS41NTA4IDExLjIgMTEuMDkwOCAxMC43NCAxMS4wOTA4IDEwLjE3WiIvPjwvZz48L2c+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "Router",
                    "TW": "路由器",
                    "CN": "路由器",
                    "BR": "Roteador",
                    "CZ": "Router",
                    "DA": "Router",
                    "DE": "Router",
                    "ES": "Enrutador",
                    "FI": "Reititin",
                    "FR": "Routeur",
                    "HU": "Router",
                    "IT": "Router",
                    "JP": "ルーター",
                    "KR": "라우터",
                    "MS": "Penghala",
                    "NL": "Router",
                    "NO": "Ruter",
                    "PL": "Router",
                    "RU": "Маршрутизатор",
                    "SV": "Router",
                    "TH": "เราเตอร์",
                    "TR": "Yönlendirici",
                    "UK": "Маршрутизатор",
                    "RO": "Router",
                    "SL": "Usmerjevalnik"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 17,
                    "type": [
                        "24"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvcmVwZWF0ZXIiPjxnIGlkPSJHcm91cF8yIj48cGF0aCBpZD0iVmVjdG9yX19fX18wXzBfSkdXSUlTVkZDUSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLW1pdGVybGltaXQ9IjEwIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE2LjY5OTQgMTMuMjdMMTcuNjkzMiAyMS4wMDAxSDcuMDkxOTlMNy44NjUwOCAxMy4yN0gxNi42OTk0WiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMV9PRlRNUVpBUUZVIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1taXRlcmxpbWl0PSIxMCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik00IDcuNzQ4NTFWMTYuNTgyOEw2Ljg3MTEgMTYuNTgyOCIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMl9LWUpJQkpIRkZLIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1taXRlcmxpbWl0PSIxMCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0yMC41NjQzIDcuNzQ4NTFMMjAuNTY0MyAxNi41ODI4SDE3LjkxNCIvPjxtYXNrIGlkPSJtYXNrMF8zMTVfNTY3IiB3aWR0aD0iMTUiIGhlaWdodD0iOSIgeD0iNSIgeT0iMyIgbWFza1VuaXRzPSJ1c2VyU3BhY2VPblVzZSIgc3R5bGU9Im1hc2stdHlwZTphbHBoYSI+PGcgaWQ9IlNWR0lEIDEiPjxnIGlkPSJHcm91cF8zIj48cGF0aCBpZD0iVmVjdG9yX19fX18wXzNfU05MSlNGVU1ERyIgZmlsbD0id2hpdGUiIGQ9Ik0xNC40OTE4IDExLjI4MjJIOS44NTM4Mkw1LjEwNTM3IDNIMTkuNTcxNkwxNC40OTE4IDExLjI4MjJaIi8+PC9nPjwvZz48L21hc2s+PGcgbWFzaz0idXJsKCNtYXNrMF8zMTVfNTY3KSI+PGcgaWQ9Ikdyb3VwXzQiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfNF9PUE1JTE5XRlZTIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1taXRlcmxpbWl0PSIxMCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xMi4zOTI1IDE4LjQ2QzE1LjUwMjkgMTguNDYgMTguMDI0NCAxNS45Mzg1IDE4LjAyNDQgMTIuODI4MUMxOC4wMjQ0IDkuNzE3NzIgMTUuNTAyOSA3LjE5NjI0IDEyLjM5MjUgNy4xOTYyNEM5LjI4MjEzIDcuMTk2MjQgNi43NjA2NCA5LjcxNzcyIDYuNzYwNjQgMTIuODI4MUM2Ljc2MDY0IDE1LjkzODUgOS4yODIxMyAxOC40NiAxMi4zOTI1IDE4LjQ2WiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfNV9VRlhKWEdKQUVTIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1taXRlcmxpbWl0PSIxMCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xMi4zOTI2IDIxLjIyMDlDMTcuMDI3NyAyMS4yMjA5IDIwLjc4NTIgMTcuNDYzNCAyMC43ODUyIDEyLjgyODNDMjAuNzg1MiA4LjE5MzE5IDE3LjAyNzcgNC40MzU2OSAxMi4zOTI2IDQuNDM1NjlDNy43NTc1IDQuNDM1NjkgNCA4LjE5MzE5IDQgMTIuODI4M0M0IDE3LjQ2MzQgNy43NTc1IDIxLjIyMDkgMTIuMzkyNiAyMS4yMjA5WiIvPjwvZz48L2c+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "Repeater",
                    "TW": "中繼器",
                    "CN": "中继器",
                    "BR": "Repetidor",
                    "CZ": "Opakovač",
                    "DA": "Repeater",
                    "DE": "Repeater",
                    "ES": "Repetidor",
                    "FI": "Vahvistin",
                    "FR": "Répéter",
                    "HU": "Jelerősítő",
                    "IT": "Ripetitore",
                    "JP": "リピーター",
                    "KR": "리피터",
                    "MS": "Pengulang",
                    "NL": "Repeater",
                    "NO": "Forsterker",
                    "PL": "Repeater",
                    "RU": "Повторитель",
                    "SV": "Repeater",
                    "TH": "รีพีตเตอร์",
                    "TR": "Tekrarlayıcı",
                    "UK": "Повторювач",
                    "RO": "Repetor",
                    "SL": "Ponavljalnik"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 18,
                    "type": [
                        "4"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvTkFTIFNlcnZlciI+PGcgaWQ9IkRldmljZXMgLyBOQVMiPjxnIGlkPSJHcm91cF8yIj48cGF0aCBpZD0iVmVjdG9yX19fX18wXzFfUFNPQ0ZLQVJITCIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTUuNjQwMDEgOC41M0M0LjA0MDAxIDguNTMgMi43NSA5LjgyIDIuNzUgMTEuNDJDMi43NSAxMy4wMiA0LjA0MDAxIDE0LjMxIDUuNjQwMDEgMTQuMzFIMTguMzZDMTkuOTYgMTQuMzEgMjEuMjUgMTMuMDIgMjEuMjUgMTEuNDJDMjEuMjUgOS44MyAxOS45NiA4LjUzIDE4LjM2IDguNTNINS42NDAwMVoiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzJfREhWU1lSUVBZTiIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE1LjA4IDExLjQySDE4LjE3Ii8+PGcgaWQ9Ikdyb3VwXzMiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfM19FRU1XU0JXTUhSIiBmaWxsPSJibGFjayIgZD0iTTUuNzkwMDEgMTIuMjRDNS42NzAwMSAxMi4xMiA1LjU5MDAxIDEyIDUuNTQwMDEgMTEuODZDNS40ODAwMSAxMS43MiA1LjQ1MDAxIDExLjU3IDUuNDUwMDEgMTEuNDJDNS40NTAwMSAxMS4yNyA1LjQ4MDAxIDExLjEyIDUuNTQwMDEgMTAuOThDNS42MDAwMSAxMC44NCA1LjY4MDAxIDEwLjcxIDUuNzkwMDEgMTAuNkM1Ljg0MDAxIDEwLjU1IDUuODg5OTkgMTAuNSA1Ljk1OTk5IDEwLjQ2QzYuMDE5OTkgMTAuNDEgNi4wOSAxMC4zOCA2LjE2IDEwLjM2QzYuMjMgMTAuMzIgNi4zIDEwLjMgNi4zOCAxMC4yOUM2LjYxIDEwLjI0IDYuODI5OTkgMTAuMjcgNy4wNDk5OSAxMC4zNkM3LjE4OTk5IDEwLjQyIDcuMzIwMDEgMTAuNSA3LjQyMDAxIDEwLjZDNy41MjAwMSAxMC43MiA3LjYyMDAxIDEwLjg0IDcuNjcwMDEgMTAuOThDNy43MjAwMSAxMS4xMiA3Ljc1IDExLjI3IDcuNzUgMTEuNDJDNy43NSAxMS41NyA3LjczMDAxIDExLjcyIDcuNjcwMDEgMTEuODZDNy42MTAwMSAxMiA3LjUyMDAxIDEyLjEzIDcuNDIwMDEgMTIuMjRDNy4yMDAwMSAxMi40NiA2LjkxMDAxIDEyLjU4IDYuNjAwMDEgMTIuNThDNi4yOTAwMSAxMi41OCA2LjAwMDAxIDEyLjQ2IDUuNzkwMDEgMTIuMjRaIi8+PC9nPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfNF9XWUNEUFRIRU5EIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNNS42NDAwMSAyLjc1QzQuMDQwMDEgMi43NSAyLjc1IDQuMDQgMi43NSA1LjY0QzIuNzUgNy4yNCA0LjA0MDAxIDguNTMgNS42NDAwMSA4LjUzSDE4LjM2QzE5Ljk2IDguNTMgMjEuMjUgNy4yNCAyMS4yNSA1LjY0QzIxLjI1IDQuMDQgMTkuOTYgMi43NSAxOC4zNiAyLjc1SDUuNjQwMDFaIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF81X09OVE1PQkFIRU4iIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xNS4wOCA1LjY0SDE4LjE3Ii8+PGcgaWQ9Ikdyb3VwXzQiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfNl9RQUVIRlVWQ0pIIiBmaWxsPSJibGFjayIgZD0iTTUuNzkwNDQgNi40NjAxMUM1LjU3MDQ0IDYuMjQwMTEgNS40NDA0MyA1Ljk1MDEgNS40NDA0MyA1LjY0MDFDNS40NDA0MyA1LjM0MDEgNS41NzA0NCA1LjA0MDExIDUuNzkwNDQgNC44MjAxMUM2LjEwMDQ0IDQuNTEwMTEgNi42MjA0MiA0LjQwMDExIDcuMDUwNDIgNC41ODAxMUM3LjE5MDQyIDQuNjQwMTEgNy4zMjA0NCA0LjcyMDExIDcuNDIwNDQgNC44MjAxMUM3LjY0MDQ0IDUuMDQwMTEgNy43NjA0NCA1LjM0MDEgNy43NjA0NCA1LjY0MDFDNy43NjA0NCA1Ljk1MDEgNy42NDA0NCA2LjI0MDExIDcuNDIwNDQgNi40NjAxMUM3LjIwMDQ0IDYuNjgwMTEgNi45MTA0MyA2LjgwMDExIDYuNjAwNDMgNi44MDAxMUM2LjI5MDQzIDYuODAwMTEgNi4wMDA0NCA2LjY4MDExIDUuNzkwNDQgNi40NjAxMVoiLz48L2c+PC9nPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfN19YQUpKWFpVRFpRIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTIgMTQuMzFWMTcuNDQiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzhfTU1BWVRSWU9MVSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTkuNjkgMTkuNTJIMi43NSIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfOV9CV1dYTUpVWlZUIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMjEuMjQ5NiAxOS41MkgxNC4zMDk2Ii8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8xMF9MTVZLWk5ZQVZaIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xMS45OTk1IDIxLjI1QzExLjA0NDEgMjEuMjUgMTAuMjY5NSAyMC40NzU1IDEwLjI2OTUgMTkuNTIwMUMxMC4yNjk1IDE4LjU2NDYgMTEuMDQ0MSAxNy43OSAxMS45OTk1IDE3Ljc5QzEyLjk1NSAxNy43OSAxMy43Mjk2IDE4LjU2NDYgMTMuNzI5NiAxOS41MjAxQzEzLjcyOTYgMjAuNDc1NSAxMi45NTUgMjEuMjUgMTEuOTk5NSAyMS4yNVoiIGNsaXAtcnVsZT0iZXZlbm9kZCIvPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "NAS/Server",
                    "TW": "網路儲存伺服器",
                    "CN": "NAS/服务器",
                    "BR": "NAS/Servidor",
                    "CZ": "NAS/Server",
                    "DA": "NAS/Server",
                    "DE": "NAS/Server",
                    "ES": "NAS/Servidor",
                    "FI": "NAS/palvelin",
                    "FR": "NAS / Serveur",
                    "HU": "NAS/szerver",
                    "IT": "NAS/Server",
                    "JP": "NAS/サーバー",
                    "KR": "NAS/서버",
                    "MS": "NAS/Pelayan",
                    "NL": "NAS/server",
                    "NO": "NAS/server",
                    "PL": "Serwer/NAS",
                    "RU": "NAS/сервер",
                    "SV": "NAS/Server",
                    "TH": "NAS/เซิร์ฟเวอร์",
                    "TR": "NAS/Sunucu",
                    "UK": "Система NAS/сервер",
                    "RO": "Server/NAS",
                    "SL": "NAS/strežnik"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 19,
                    "type": [
                        "37"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvbm90ZWJvb2siPjxnIGlkPSJHcm91cF8yIj48cGF0aCBpZD0iVmVjdG9yX19fX18wXzBfSUNZVkdJQ0FJUCIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLW1pdGVybGltaXQ9IjEwIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE5LjY0NzEgMTUuNDExOEg0LjM1Mjk0VjZIMTkuNjQ3MVYxNS40MTE4WiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMV9CR0JWSVVXUk1TIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS1taXRlcmxpbWl0PSIxMCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0yIDE3Ljc2NDdIMjIiLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Notebook",
                    "TW": "攜帶型電腦",
                    "CN": "笔记本",
                    "BR": "Notebook",
                    "CZ": "Notebook",
                    "DA": "Notebook",
                    "DE": "Notebook",
                    "ES": "Portátil",
                    "FI": "Kannettava tietokone",
                    "FR": "Portable",
                    "HU": "Notebook",
                    "IT": "Notebook",
                    "JP": "ノートブック",
                    "KR": "노트북",
                    "MS": "Komputer Buku",
                    "NL": "Notebook",
                    "NO": "Bærbar datamaskin",
                    "PL": "Notebook",
                    "RU": "Ноутбук",
                    "SV": "Bärbar dator",
                    "TH": "โน้ตบุ๊ก",
                    "TR": "Notebook",
                    "UK": "Ноутбук",
                    "RO": "Notebook",
                    "SL": "Prenosni računalnik"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 20,
                    "type": [
                        "35"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvd2luZG93cyBOQiI+PGcgaWQ9Ikdyb3VwIDM1MzQiPjxnIGlkPSJHcm91cF8yIj48cGF0aCBpZD0iVmVjdG9yIDEzNl9fX19fMF8wX1NIV0RJQkZESlQiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTkuOTQ0NiA1LjgzMzM1VjQuM0MxOS45NDQ2IDQuMTM0MzEgMTkuODEwMyA0IDE5LjY0NDYgNEg0LjM1NTU5QzQuMTg5OSA0IDQuMDU1NTkgNC4xMzQzMSA0LjA1NTU5IDQuM1YxNS45MjIzQzQuMDU1NTkgMTYuMDg4IDQuMTg5OSAxNi4yMjIzIDQuMzU1NTkgMTYuMjIyM0g5LjU1NTY0Ii8+PHBhdGggaWQ9IlJlY3RhbmdsZSA5MzM2X19fX18wXzFfQ0NQWlRFVEhQTiIgZmlsbD0iYmxhY2siIGQ9Ik0xIDE1LjU4ODlDMSAxNS41MzM3IDEuMDQ0NzcgMTUuNDg4OSAxLjEgMTUuNDg4OUgxNC45MzU1QzE0Ljk5OCAxNS40ODg5IDE1LjA0NTIgMTUuNTQ1NiAxNS4wMzM4IDE1LjYwNzFMMTQuOTM1NiAxNi4xMzc3QzE0Ljg0NzggMTYuNjExNyAxNC40MzQzIDE2Ljk1NTYgMTMuOTUyMyAxNi45NTU2SDJDMS40NDc3MiAxNi45NTU2IDEgMTYuNTA3OSAxIDE1Ljk1NTZWMTUuNTg4OVoiLz48L2c+PGcgaWQ9IkFwcGxlIj48cGF0aCBpZD0iVW5pb25fX19fXzBfMl9RQUtaWURES1ZJIiBmaWxsPSJibGFjayIgZmlsbC1ydWxlPSJldmVub2RkIiBkPSJNMjMuMDAwMiAxMi44MzAzQzIwLjg1ODEgMTIuODUyIDE4LjcxNiAxMi44NzM1IDE2LjU3MzkgMTIuODk1VjguMzM4MzhMMjMuMDAwMiA3LjA1NTU5VjEyLjgzMDNaTTE2LjU3MzkgMTcuOTQ1VjEzLjQxNjJDMTguNzE2IDEzLjQzNzcgMjAuODU4MSAxMy40NTkyIDIzLjAwMDIgMTMuNDgwOVYxOS4yMjc2QzIwLjg1ODEgMTguODAwMiAxOC43MTYgMTguMzcyNiAxNi41NzM5IDE3Ljk0NVpNMTYuMDMwMyAxMi45MDA5VjguNDQ2NDJMMTYuMDI4NCA4LjQ0Njc5QzE0LjY4NTcgOC43MTQ4MSAxMy4zNDI5IDguOTgyODMgMTIuMDAwMSA5LjI1MTA2VjEyLjk0MTRMMTYuMDMwMyAxMi45MDA5Wk0xMi4wMDAxIDEzLjM2OTdMMTYuMDMwMyAxMy40MTAyVjE3LjgzNjZMMTIuMDAwMSAxNy4wMzIyVjEzLjM2OTdaIiBjbGlwLXJ1bGU9ImV2ZW5vZGQiLz48L2c+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "Windows Notebook",
                    "TW": "微軟攜帶型電腦",
                    "CN": "Windows 笔记本",
                    "BR": "Notebook Windows",
                    "CZ": "Windows NB",
                    "DA": "Windows NB",
                    "DE": "Windows-NB",
                    "ES": "Portátil Windows",
                    "FI": "Windows NB",
                    "FR": "Windows NB",
                    "HU": "Windowsos notebook",
                    "IT": "Windows NB",
                    "JP": "Windows NB",
                    "KR": "Windows NB",
                    "MS": "Windows NB",
                    "NL": "Windows NB",
                    "NO": "Bærbar Windows-datamaskin",
                    "PL": "Notebook Windows",
                    "RU": "Windows NB",
                    "SV": "Bärbar Windows-dator",
                    "TH": "โน้ตบุ๊ก Windows",
                    "TR": "Windows NB",
                    "UK": "Ноутбук з Windows",
                    "RO": "NB Windows",
                    "SL": "Prenosni računalnik Windows"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 21,
                    "type": [
                        "6"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvbWFjYm9vayI+PGcgaWQ9Ikdyb3VwIDM1MzMiPjxnIGlkPSJHcm91cF8yIj48cGF0aCBpZD0iVmVjdG9yIDEzNl9fX19fMF8wX1JERUZZT0NFSU0iIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTkuNjExMyA2LjgwMTExVjUuM0MxOS42MTEzIDUuMTM0MzEgMTkuNDc3IDUgMTkuMzExMyA1SDQuMzAxNzRDNC4xMzYwNSA1IDQuMDAxNzQgNS4xMzQzMSA0LjAwMTc0IDUuM1YxNi43MDc0QzQuMDAxNzQgMTYuODczMSA0LjEzNjA1IDE3LjAwNzQgNC4zMDE3NCAxNy4wMDc0SDkuNDA1MDYiLz48L2c+PHBhdGggaWQ9IlJlY3RhbmdsZSA5MzM2X19fX18wXzFfUlpBU1pTSVpQSiIgZmlsbD0iYmxhY2siIGQ9Ik0xIDE2LjM4NjhDMSAxNi4zMzE2IDEuMDQ0NzcgMTYuMjg2OCAxLjEgMTYuMjg2OEgxNy4wODk3QzE3LjE1MjMgMTYuMjg2OCAxNy4xOTk1IDE2LjM0MzYgMTcuMTg4MSAxNi40MDVMMTcuMDk0NiAxNi45MDk4QzE3LjAwNjggMTcuMzgzOCAxNi41OTM0IDE3LjcyNzcgMTYuMTExMyAxNy43Mjc3SDJDMS40NDc3MSAxNy43Mjc3IDEgMTcuMjggMSAxNi43Mjc3VjE2LjM4NjhaIi8+PGcgaWQ9IkFwcGxlIj48ZyBpZD0iR3JvdXBfMyI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8yX1pQT0RHUVFGSlAiIGZpbGw9ImJsYWNrIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMjAuNjEwNCAxMS43MjFMMjAuNjExOCAxMS43MjExQzIxLjExODMgMTEuNzYxMiAyMS41MTA0IDExLjg5NjcgMjEuODE2MiAxMi4xMjQ5QzIxLjYwNTkgMTIuMzE5NiAyMS40MjUgMTIuNTM2MSAyMS4yNzk1IDEyLjc3ODVDMjEuMDEzMSAxMy4yMjI2IDIwLjg4NTEgMTMuNzIxNyAyMC44ODUxIDE0LjI3MDlDMjAuODg1MSAxNC4yNjk0IDIwLjg3OTMgMTQuNDA5NSAyMC45MDE3IDE0LjYwNTVDMjAuOTIzOSAxNC44MDAxIDIwLjk3NDEgMTUuMDYwNCAyMS4wODYyIDE1LjM0ODFDMjEuMjcyMiAxNS44MjUyIDIxLjYyMjMgMTYuMzU5NCAyMi4yNTU3IDE2Ljc4NjFDMjIuMDk3NiAxNy4xNDc5IDIxLjg3MDQgMTcuNTc4NiAyMS41OTc0IDE3Ljk1NzZDMjEuMzk3NyAxOC4yMzQ5IDIxLjE4OTQgMTguNDYyMiAyMC45ODU1IDE4LjYxNTRDMjAuNzgxNCAxOC43Njg4IDIwLjYxNzIgMTguODIyMSAyMC40OSAxOC44MjIxQzIwLjI4MDIgMTguODIyMSAyMC4xMTU3IDE4Ljc1OTQgMTkuODE5OCAxOC42NDI3QzE5LjUzMjQgMTguNTI5NSAxOS4xNDg2IDE4LjM3OTggMTguNjM2NCAxOC4zNzk4QzE4LjExNjMgMTguMzc5OCAxNy43MTg5IDE4LjUyNTggMTcuNDIwMSAxOC42NDEzTDE3LjQxMzcgMTguNjQzOEMxNy4wOTkzIDE4Ljc2NTIgMTYuOTUyMSAxOC44MjIxIDE2Ljc4MjcgMTguODIyMUMxNi43NzQ5IDE4LjgyMjEgMTYuNzY3MiAxOC44MjIyIDE2Ljc1OTQgMTguODIyNUMxNi42NjE3IDE4LjgyNjMgMTYuNTA4NiAxOC43ODQ1IDE2LjI5NDIgMTguNjE3OUMxNi4wODMgMTguNDUzOSAxNS44NjI1IDE4LjIwNjggMTUuNjQ4MiAxNy45MDUzQzE1LjIxOTIgMTcuMzAxOSAxNC44ODQgMTYuNTc5NCAxNC43NDA1IDE2LjE3MjJMMTQuNzQwNSAxNi4xNzIyTDE0LjczODggMTYuMTY3NUMxNC41MTczIDE1LjU1NTUgMTQuNDI5OSAxNC45NjgzIDE0LjQyOTkgMTQuMzk0NUMxNC40Mjk5IDEzLjQ0NzYgMTQuNzM5NSAxMi44MDAyIDE1LjE0NjIgMTIuMzg4NUMxNS41NTc4IDExLjk3MTggMTYuMTAzOCAxMS43NjA3IDE2LjYzODQgMTEuNzQ5QzE2Ljg1MjUgMTEuNzUwMyAxNy4xMzM2IDExLjgzIDE3LjQ3NDYgMTEuOTQ3M0MxNy41MTY2IDExLjk2MTcgMTcuNTYwNyAxMS45NzcxIDE3LjYwNTYgMTEuOTkyOEMxNy43MjQgMTIuMDM0MiAxNy44NDg2IDEyLjA3NzcgMTcuOTU4IDEyLjExMTdDMTguMDk4OCAxMi4xNTU0IDE4LjI5NjEgMTIuMjEwOCAxOC40ODY4IDEyLjIxMDhDMTguNjg4MSAxMi4yMTA4IDE4LjkwNTIgMTIuMTM4NyAxOS4wNDEgMTIuMDkyN0MxOS4xMTM1IDEyLjA2OCAxOS4xOTE1IDEyLjA0MDEgMTkuMjY3OCAxMi4wMTI3QzE5LjI3NzQgMTIuMDA5MiAxOS4yODcgMTIuMDA1OCAxOS4yOTY2IDEyLjAwMjRDMTkuMzg0MiAxMS45NzEgMTkuNDcyNSAxMS45Mzk1IDE5LjU2MzcgMTEuOTA4OUMxOS45NDEzIDExLjc4MjMgMjAuMzAwMSAxMS42OTU3IDIwLjYxMDQgMTEuNzIxWiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfM19VSlBIR1BaVFJUIiBmaWxsPSJibGFjayIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE5LjY0ODMgOC4wMTgzMUMxOS43NTk4IDcuOTg1NTkgMTkuODIxNSA3Ljk4NzQgMTkuODQ4NiA3Ljk5MTA4QzE5Ljg1OTggOC4wMTk3OSAxOS44ODE1IDguMDk2NDEgMTkuODU5MyA4LjI1MjA0QzE5LjgyNzQgOC40NzU2MSAxOS43MTczIDguNzA3MyAxOS41NzI5IDguODUxNjRMMTkuNTcyOSA4Ljg1MTYzTDE5LjU2OTYgOC44NTQ5OUMxOS40NTA1IDguOTc1OTkgMTkuMjU4OCA5LjA4MjM0IDE5LjA3MTggOS4xMjUyOEMxOC45NTA5IDkuMTUzMDQgMTguODg1MyA5LjE0NDgxIDE4Ljg1NzQgOS4xMzgyNEMxOC44NDgzIDkuMTA5NjIgMTguODM0NSA5LjAzOTUyIDE4Ljg1OTIgOC45MDI3N0MxOC44OTY4IDguNjk0NDEgMTkuMDA2NSA4LjQ3MTYgMTkuMTQxMiA4LjMyNjM1QzE5LjI2NzYgOC4xOTIyNCAxOS40NjI5IDguMDcyNjkgMTkuNjQ4MyA4LjAxODMxWiIvPjwvZz48L2c+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "Macbook",
                    "TW": "Apple攜帶型電腦",
                    "CN": "Macbook",
                    "BR": "Macbook",
                    "CZ": "Macbook",
                    "DA": "Macbook",
                    "DE": "Macbook",
                    "ES": "Macbook",
                    "FI": "Macbook",
                    "FR": "Macbook",
                    "HU": "Macbook",
                    "IT": "Macbook",
                    "JP": "Macbook",
                    "KR": "Macbook",
                    "MS": "Macbook",
                    "NL": "Macbook",
                    "NO": "MacBook",
                    "PL": "Macbook",
                    "RU": "Macbook",
                    "SV": "MacBook",
                    "TH": "Macbook",
                    "TR": "Macbook",
                    "UK": "MacBook",
                    "RO": "Macbook",
                    "SL": "Macbook"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 22,
                    "type": [
                        "38"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvYXN1cyBub3RlYm9vayI+PGcgaWQ9Ikdyb3VwIDM1MzkiPjxnIGlkPSJHcm91cF8yIj48cGF0aCBpZD0iVmVjdG9yIDEzNl9fX19fMF8wX0dKTExKSkhYVUoiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMjAuNDg1MyAxMC4wMjg2VjUuM0MyMC40ODUzIDUuMTM0MzIgMjAuMzUwOSA1IDIwLjE4NTMgNUg0LjQ0MjRDNC4yNzY3MSA1IDQuMTQyNCA1LjEzNDMxIDQuMTQyNCA1LjNWMTcuMjcxNEM0LjE0MjQgMTcuNDM3MSA0LjI3NjcxIDE3LjU3MTQgNC40NDI0IDE3LjU3MTRIOS43OTk1NCIvPjxwYXRoIGlkPSJSZWN0YW5nbGUgOTMzNl9fX19fMF8xX05GUlVZRlRMQ0kiIGZpbGw9ImJsYWNrIiBkPSJNMSAxNi45MTY5QzEgMTYuODYxNyAxLjA0NDc3IDE2LjgxNjkgMS4xIDE2LjgxNjlIMjIuODc5OEMyMi45NDIzIDE2LjgxNjkgMjIuOTg5NSAxNi44NzM3IDIyLjk3ODEgMTYuOTM1MkwyMi44NzIxIDE3LjUwNzZDMjIuNzg0MyAxNy45ODE2IDIyLjM3MDkgMTguMzI1NSAyMS44ODg4IDE4LjMyNTVIMkMxLjQ0NzcyIDE4LjMyNTUgMSAxNy44Nzc4IDEgMTcuMzI1NVYxNi45MTY5WiIvPjwvZz48ZyBpZD0iQXBwbGUiPjxwYXRoIGlkPSJVbmlvbl9fX19fMF8yX0ZJVk9ZUFRGU1giIGZpbGw9ImJsYWNrIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGQ9Ik0yMi4zNjc1IDExLjI4NThMMjIuMzcxIDEyLjI1MTdMMTguMTM5NCAxMi4yNjhMMTguMTM5MiAxMi4xOTAyQzE4LjEzOTIgMTIuMTkwMiAxOC4yMTk3IDExLjg1MTcgMTguMzY0NyAxMS42OTA1TDE4LjM3MSAxMS42ODM1QzE4LjUwNjMgMTEuNTMyMSAxOC42ODkxIDExLjMyNzcgMTkuMDk3MSAxMS4yOTg5TDIyLjM2NzUgMTEuMjg1OFpNMTguMTI5NSAxMS4zMDczTDE4LjEzMjkgMTIuMjY4TDE3LjIxOTggMTIuMjcxNkwxNy4yMTY0IDExLjMxMDlMMTguMTI5NSAxMS4zMDczWk0xNS4xMzM1IDEyLjI3OThMMTUuMTMwMSAxMS4zMTkxTDE0LjIxNjEgMTEuMzIyN0wxNC4yMTk1IDEyLjI4MzRMMTUuMTMzNSAxMi4yNzk4Wk03LjU5MTg5IDEyLjM2ODZMOC42MDY1MSAxMi40NzkxTDcuMTAzMzggMTQuOTc1TDYuMDI4NDIgMTQuOTc5M0w3LjU5MTg5IDEyLjM2ODZaTTE4LjE0NjcgMTMuOThMMjEuMzIxOSAxMy45NjcyQzIxLjM5NTEgMTMuOTY2OSAyMS40NzM5IDEzLjkyMDkgMjEuNDczOSAxMy45MjA5QzIxLjUwNjcgMTMuODg5MiAyMS41MzMzIDEzLjgzMTYgMjEuNTMzMSAxMy43NzZDMjEuNTMyNSAxMy41OTIyIDIxLjM4OTIgMTMuNTg0NiAyMS4zMTcxIDEzLjU4MTJDMjEuMzE3MSAxMy41ODEyIDE5LjA4NTcgMTMuMzkzIDE5LjAwNzkgMTMuMzg2NkMxOS4wMDc5IDEzLjM4NjYgMTguNjk4NiAxMy4zMzc1IDE4LjQ5MTQgMTMuMTI4MUwxOC40NzA4IDEzLjEwNzlDMTguMzg0IDEzLjAyMjggMTguMjc1OCAxMi45MTY4IDE4LjE5MjkgMTIuNjE3QzE4LjE5MjkgMTIuNjE3IDIxLjE3NTggMTIuNzkwOSAyMS41MDcxIDEyLjgyMjRDMjIuMDY2NSAxMi44NzcgMjIuMzMwOCAxMy40NjE2IDIyLjM1NTEgMTMuNjU2M0MyMi4zNTUxIDEzLjY1NjMgMjIuMzc5NyAxMy44MzI1IDIyLjM1MTMgMTQuMDQ4OUMyMi4zNTEzIDE0LjA0ODkgMjIuMjM4NCAxNC44NzI3IDIxLjQxOCAxNC45MjU5TDE4LjE0OCAxNC45MzlMMTguMTQ2NyAxMy45OFpNMTAuOTk5MyAxMS4zMzE0TDE0LjA1NiAxMS4zMTkxTDE0LjA1OTMgMTIuMjg0M0w3LjYyNzkzIDEyLjMxMDFDNy42Mjc5MyAxMi4zMTAxIDcuOTk2NzIgMTEuNjYxNSA4LjA4ODM4IDExLjUzMzVDOC4xNzU5MiAxMS40MDQxIDguMjkyMzkgMTEuMzQ4NCA4LjQxNDkgMTEuMzQ3OUwxMC4wMzc4IDExLjM0MTRMMTAuMDQwOCAxMi4yMjI2QzEwLjA0MDggMTIuMjIyNiAxMC4xMjIxIDExLjg4NDIgMTAuMjY3IDExLjcyMjlMMTAuMjc0MiAxMS43MTQ5QzEwLjQwOTEgMTEuNTYzNyAxMC41OTEgMTEuMzYgMTAuOTk5MyAxMS4zMzE0Wk0xNC4yMyAxMy42ODlDMTQuMjA3MSAxMy41NDU5IDEzLjk0NzUgMTIuOTE0MSAxMy4zODg2IDEyLjg2MTRDMTMuMDU1MyAxMi44Mjk4IDEwLjA4OTIgMTIuNTkyOCAxMC4wODkyIDEyLjU5MjhDMTAuMTQ3NyAxMi45MDcgMTAuMjgxOSAxMy4wNjUxIDEwLjM3MSAxMy4xNTU0QzEwLjU3NzcgMTMuMzYzOCAxMC45MDU0IDEzLjQyMTkgMTAuOTA1NCAxMy40MjE5QzEwLjk4NDYgMTMuNDI5NyAxMy4xOTEyIDEzLjYxMzkgMTMuMTkxMiAxMy42MTM5QzEzLjI2MTQgMTMuNjE3NCAxMy4zOTQ4IDEzLjYzNTggMTMuMzk0MSAxMy44MjAxQzEzLjM5NDIgMTMuODQyNiAxMy4zNzU1IDE0LjAwNSAxMy4yMDYgMTQuMDA1NkwxMC4wMDg0IDE0LjAxODVMMTAuMDAzNCAxMi41ODUxTDkuMTE0NjUgMTIuNTE5OEw5LjEyMzIxIDE0Ljk2NzNMMTMuMzk3NyAxNC45NTAxQzE0LjE0MTUgMTQuNzg5MyAxNC4yMTY4IDE0LjA3MzYgMTQuMjE2OCAxNC4wNzM2QzE0LjIyNjcgMTQuMDEyMiAxNC4yMzE2IDEzLjk1NSAxNC4yMzQxIDEzLjkwMzVMMTQuMjMzNSAxMy43MzIxQzE0LjIzMTggMTMuNzA1NCAxNC4yMyAxMy42ODkgMTQuMjMgMTMuNjg5Wk0xNC4yMzM4IDEzLjczMjVDMTQuMjM2MiAxMy43Njk5IDE0LjIzODEgMTMuODI5OCAxNC4yMzQ1IDEzLjkwMzlMMTQuMjM1IDE0LjA2MjFDMTQuMzg0NiAxNC44ODE2IDE1LjA4NDIgMTQuOTQxNCAxNS4wODQyIDE0Ljk0MTRDMTUuMDg0MiAxNC45NDE0IDE1LjE1NTQgMTQuOTQ2NSAxNS4xNjcgMTQuOTQ3NUwxNy4yNTExIDE0LjkzOTFDMTcuMjUxMSAxNC45MzkxIDE4LjE0MjUgMTQuODU5MiAxOC4xMzk0IDEzLjk1MzVMMTguMTM0NyAxMi42MTVMMTcuMjM2OCAxMi41NjE4TDE3LjI0MDcgMTMuNjc2MUMxNy4yNDA3IDEzLjY3NjEgMTcuMjM5MiAxMy45OTcyIDE2Ljk0NTkgMTMuOTk4NEwxNS40MDMgMTQuMDA0NkMxNS40MDMgMTQuMDA0NiAxNS4xNDE3IDEzLjk4MjggMTUuMTQwNyAxMy42ODc2TDE1LjEzNjMgMTIuNDE5TDE0LjIyOSAxMi4zNTMzTDE0LjIzMzggMTMuNzMyNVoiIGNsaXAtcnVsZT0iZXZlbm9kZCIvPjwvZz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "ASUS Notebook",
                    "TW": "ASUS攜帶型電腦",
                    "CN": "ASUS 笔记本",
                    "BR": "Notebook ASUS",
                    "CZ": "Notebook ASUS",
                    "DA": "ASUS Notebook",
                    "DE": "ASUS-Notebook",
                    "ES": "Portátil ASUS",
                    "FI": "Kannettava ASUS-tietokone",
                    "FR": "Ordinateur portable ASUS",
                    "HU": "ASUS Notebook",
                    "IT": "Notebook ASUS",
                    "JP": "ASUSノートブック",
                    "KR": "ASUS 노트북",
                    "MS": "ASUS Notebook",
                    "NL": "ASUS-notebook",
                    "NO": "Bærbar ASUS-datamaskin",
                    "PL": "Notebook ASUS",
                    "RU": "Ноутбук ASUS",
                    "SV": "Bärbar ASUS-dator",
                    "TH": "โน้ตบุ๊ก ASUS",
                    "TR": "ASUS Notebook",
                    "UK": "Ноутбук ASUS",
                    "RO": "Notebook ASUS",
                    "SL": "Prenosni računalnik ASUS"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 23,
                    "type": [
                        "39"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvU0QgY2FyZCI+PGcgaWQ9Ikdyb3VwXzIiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMV9CSUZPSFVQTkhXIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTkuMTkgMTkuMkMxOS4xOSAyMC4zMyAxOC4yNiAyMS4yNiAxNy4xMyAyMS4yNkg2Ljg1MDAxQzUuNzIwMDEgMjEuMjYgNC43ODk5OSAyMC4zNCA0Ljc4OTk5IDE5LjJWNC44MUM0Ljc4OTk5IDMuNjggNS43MjAwMSAyLjc1IDYuODUwMDEgMi43NUgxNC4xM0MxNC43MyAyLjc1IDE1LjMxIDIuOTkgMTUuNzMgMy40MUwxOC41MSA2LjE5QzE4Ljk2IDYuNjIgMTkuMTkgNy4yIDE5LjE5IDcuOFYxOS4yWiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMl9ETFRFRlVTU1VYIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNNy44OSA1LjgzVjYuODYiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzNfUU9FTkhNR1BUVCIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTEwLjk3IDUuODNWNi44NiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfNF9ZWE5UWElaTVRYIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTQuMDYgNS44M1Y2Ljg2Ii8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF81X0RMT0tTT1VDSk0iIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik04LjkxIDE0LjA2QzguMzQgMTQuMDYgNy44OCAxNC41MiA3Ljg4IDE1LjA5VjE3LjE1QzcuODggMTcuNzIgOC4zNCAxOC4xOCA4LjkxIDE4LjE4SDE1LjA4QzE1LjY1IDE4LjE4IDE2LjExIDE3LjcyIDE2LjExIDE3LjE1VjE1LjA5QzE2LjExIDE0LjUyIDE1LjY1IDE0LjA2IDE1LjA4IDE0LjA2SDguOTFaIi8+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "SD Card",
                    "TW": "SD卡",
                    "CN": "SD 卡",
                    "BR": "Cartão SD",
                    "CZ": "Karta SD",
                    "DA": "SD-kort",
                    "DE": "SD-Karte",
                    "ES": "Tarjeta SD",
                    "FI": "SD-kortti",
                    "FR": "Carte SD",
                    "HU": "SD-kártya",
                    "IT": "Scheda SD",
                    "JP": "SDカード",
                    "KR": "SD 카드",
                    "MS": "Kad SD",
                    "NL": "SD-kaart",
                    "NO": "SD-kort",
                    "PL": "Karta SD",
                    "RU": "SD-карта",
                    "SV": "SD-kort",
                    "TH": "การ์ด SD",
                    "TR": "SD Kart",
                    "UK": "SD-картка",
                    "RO": "Card SD",
                    "SL": "Kartica SD"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 24,
                    "type": [
                        "40"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvdXNiIj48ZyBpZD0iR3JvdXBfMiI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8wX1RWWFdIU05GSkIiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1taXRlcmxpbWl0PSIxMCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xOS4zNTU4IDE2LjU4MjlINS4wMDAwMUw1IDIxLjAwMDFIMTkuMzU1OEwxOS4zNTU4IDE2LjU4MjlaIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8xX0lBQ1JFTEFNUkUiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1taXRlcmxpbWl0PSIxMCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xNy4xNDczIDExLjA2MTRINy4yMDg2OUw3LjIwODc0IDE2LjU4MjhIMTcuMTQ3M0wxNy4xNDczIDExLjA2MTRaIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8yX0VJQ0FHWUlUWFoiIGZpbGw9ImJsYWNrIiBkPSJNOS44NTkyOSAxMi43MTc5SDguODY1NDNWMTMuODIyMkg5Ljg1OTI5VjEyLjcxNzlaIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8zX01VSlJGRFRQWU8iIGZpbGw9ImJsYWNrIiBkPSJNMTUuMzgwNiAxMi43MTc5SDE0LjM4NjdWMTMuODIyMkgxNS4zODA2VjEyLjcxNzlaIi8+PG1hc2sgaWQ9Im1hc2swXzMxNV82NDkiIHdpZHRoPSIxNSIgaGVpZ2h0PSI5IiB4PSI1IiB5PSIzIiBtYXNrVW5pdHM9InVzZXJTcGFjZU9uVXNlIiBzdHlsZT0ibWFzay10eXBlOmFscGhhIj48ZyBpZD0iU1ZHSUQgMSI+PGcgaWQ9Ikdyb3VwXzMiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfNF9HWkJCVk9XRVVJIiBmaWxsPSJ3aGl0ZSIgZD0iTTE0LjM4NjUgMTEuMjgyMkg5Ljc0ODQ2TDUgMy4wMDAwNUgxOS40NjYyTDE0LjM4NjUgMTEuMjgyMloiLz48L2c+PC9nPjwvbWFzaz48ZyBtYXNrPSJ1cmwoI21hc2swXzMxNV82NDkpIj48ZyBpZD0iR3JvdXBfNCI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF81X09WT0JKTk9CVksiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLW1pdGVybGltaXQ9IjEwIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTEyLjI4NzUgMTguNDU5OUMxNS4zOTc5IDE4LjQ1OTkgMTcuOTE5MyAxNS45Mzg0IDE3LjkxOTMgMTIuODI4QzE3LjkxOTMgOS43MTc1OCAxNS4zOTc5IDcuMTk2MDkgMTIuMjg3NSA3LjE5NjA5QzkuMTc3MDUgNy4xOTYwOSA2LjY1NTU3IDkuNzE3NTggNi42NTU1NyAxMi44MjhDNi42NTU1NyAxNS45Mzg0IDkuMTc3MDUgMTguNDU5OSAxMi4yODc1IDE4LjQ1OTlaIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF82X0JKQk5HVElGV00iIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLW1pdGVybGltaXQ9IjEwIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTEyLjI4NzUgMjEuMjIwN0MxNi45MjI3IDIxLjIyMDcgMjAuNjgwMiAxNy40NjMyIDIwLjY4MDIgMTIuODI4MUMyMC42ODAyIDguMTkyOTUgMTYuOTIyNyA0LjQzNTQ1IDEyLjI4NzUgNC40MzU0NUM3LjY1MjQzIDQuNDM1NDUgMy44OTQ5MiA4LjE5Mjk1IDMuODk0OTIgMTIuODI4MUMzLjg5NDkyIDE3LjQ2MzIgNy42NTI0MyAyMS4yMjA3IDEyLjI4NzUgMjEuMjIwN1oiLz48L2c+PC9nPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "USB",
                    "TW": "USB",
                    "CN": "USB",
                    "BR": "USB",
                    "CZ": "USB",
                    "DA": "USB",
                    "DE": "USB",
                    "ES": "USB",
                    "FI": "USB",
                    "FR": "USB",
                    "HU": "USB",
                    "IT": "USB",
                    "JP": "USB",
                    "KR": "USB",
                    "MS": "USB",
                    "NL": "USB",
                    "NO": "USB",
                    "PL": "USB",
                    "RU": "USB",
                    "SV": "USB",
                    "TH": "USB",
                    "TR": "USB",
                    "UK": "USB",
                    "RO": "USB",
                    "SL": "USB"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 25,
                    "type": [
                        "18"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48cGF0aCBzdHJva2U9IiMwMDAiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik03IDcuNUg0YTIgMiAwIDAgMC0yIDJ2NmEyIDIgMCAwIDAgMiAyaDNtMC0xMFY0LjJBMS4yIDEuMiAwIDAgMSA4LjIgM2g3LjZBMS4yIDEuMiAwIDAgMSAxNyA0LjJ2My4zbS0xMCAwaDEwbTAgMGgzYTIgMiAwIDAgMSAyIDJ2NmEyIDIgMCAwIDEtMiAyaC0zTTkuNSAxNWg1bS01IDMuNWgybS00LjUtMVYxMmgxMHY1LjVtLTEwIDB2Mi4zQTEuMiAxLjIgMCAwIDAgOC4yIDIxaDcuNmExLjIgMS4yIDAgMCAwIDEuMi0xLjJ2LTIuMyIvPjwvc3ZnPg==",
                    "EN": "Printer",
                    "TW": "印表機",
                    "CN": "打印机",
                    "BR": "Impressora",
                    "CZ": "Tiskárna",
                    "DA": "Printer",
                    "DE": "Drucker",
                    "ES": "Impresora",
                    "FI": "Tulostin",
                    "FR": "Imprimante",
                    "HU": "Nyomtató",
                    "IT": "Stampante",
                    "JP": "プリンター",
                    "KR": "프린터",
                    "MS": "Pencetak",
                    "NL": "Printer",
                    "NO": "Skriver",
                    "PL": "Drukarka",
                    "RU": "Принтер",
                    "SV": "Skrivare",
                    "TH": "เครื่องพิมพ์",
                    "TR": "Yazıcı",
                    "UK": "Принтер",
                    "RO": "Imprimantă",
                    "SL": "Tiskalnik"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 26,
                    "type": [
                        "41"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvY2FtZXJhIj48ZyBpZD0iRGV2aWNlcyAvIGNhbWVyYSI+PGcgaWQ9Ikdyb3VwXzIiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMV9MQ1JJUVNJVVlFIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMjEuMjUwMiAxN0MyMS4yNTAyIDE4LjEgMjAuMzMwMiAxOSAxOS4xOTAyIDE5SDQuODAwMjNDMy42NzAyMyAxOSAyLjc0MDIzIDE4LjEgMi43NDAyMyAxN1Y5QzIuNzQwMjMgNy45IDMuNjYwMjMgNyA0LjgwMDIzIDdINy4xNDAyNkM3Ljc5MDI2IDcgOC4zNjAyNSA2LjYgOC41NzAyNSA2QzguNzgwMjUgNS40IDkuMzUwMjQgNSAxMC4wMDAyIDVIMTQuMDAwMkMxNC42NTAyIDUgMTUuMjIwMiA1LjQgMTUuNDMwMiA2QzE1LjY0MDIgNi42IDE2LjIxMDIgNyAxNi44NjAyIDdIMTkuMjAwM0MyMC4zMzAzIDcgMjEuMjUwMiA3LjkgMjEuMjUwMiA5VjE3WiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMl9BREpQRlRKV1pWIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xMi4wMDAyIDE2QzEwLjAxMiAxNiA4LjQwMDI0IDE0LjQzMyA4LjQwMDI0IDEyLjVDOC40MDAyNCAxMC41NjcgMTAuMDEyIDkgMTIuMDAwMiA5QzEzLjk4ODUgOSAxNS42MDAzIDEwLjU2NyAxNS42MDAzIDEyLjVDMTUuNjAwMyAxNC40MzMgMTMuOTg4NSAxNiAxMi4wMDAyIDE2WiIgY2xpcC1ydWxlPSJldmVub2RkIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8zX1NSVkdNR1VGQVYiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik01LjgzMDIzIDdWNiIvPjwvZz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Camera",
                    "TW": "相機",
                    "CN": "相机",
                    "BR": "Câmera",
                    "CZ": "Kamera",
                    "DA": "Kamera",
                    "DE": "Kamera",
                    "ES": "Cámara",
                    "FI": "Kamera",
                    "FR": "Caméra",
                    "HU": "Kamera",
                    "IT": "Videocamera",
                    "JP": "カメラ",
                    "KR": "카메라",
                    "MS": "Camera",
                    "NL": "Camera",
                    "NO": "Kamera",
                    "PL": "Kamera",
                    "RU": "Камера",
                    "SV": "Kamera",
                    "TH": "กล้อง",
                    "TR": "Kamera",
                    "UK": "Камера",
                    "RO": "Cameră",
                    "SL": "Kamera"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 27,
                    "type": [
                        "15"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvcm9nIGRldmljZSI+PHBhdGggaWQ9ImNsaWVudC1saXN0X3JvZy1maXJzdF9fX19fMF8wX0hLVEZVVVRXVlQiIGZpbGw9ImJsYWNrIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGQ9Ik0wLjAzMDAyOTQgMTEuNzA0MkMwLjAyODQ1NTIgMTEuNyAwLjAyNzA2NzQgMTEuNjk2MyAwLjAyNTg1MDIgMTEuNjkzMUMwLjAyMjYzNzggMTEuNjg3NyAwLjAyMDA0MTkgMTEuNjg0MSAwLjAyMDA0MTkgMTEuNjg0MUMwLjAyMDA0MTkgMTEuNjg0MSAwLjAyMjgzNCAxMS42ODcxIDAuMDI0MTgxMSAxMS42ODg0QzAuMDIxNTkwNSAxMS42ODEyIDAuMDIwMDQxOSAxMS42NzYyIDAuMDIwMDQxOSAxMS42NzYyQzAuMDIwMDQxOSAxMS42NzYyIDMuMzc2NDcgMTQuODA2NyA1LjAxODk0IDE1Ljk1OUM1LjAxODk0IDE1Ljk1OSA1LjAxMTM2IDE1Ljk1NzQgNS4wMDAwNyAxNS45NTQ5QzUuMDA2MjkgMTUuOTU5IDUuMDEzMTMgMTUuOTYzOSA1LjAxODk0IDE1Ljk2ODNDNS4wMTg5NCAxNS45NjgzIDUuMDEwOTIgMTUuOTY0OCA0Ljk5NjcgMTUuOTU4NkM0Ljk5MzY4IDE1Ljk1NzkgNC45ODc3MyAxNS45NTY2IDQuOTgwMDQgMTUuOTU0OUM0Ljk4NjI2IDE1Ljk1OSA0Ljk5MzA5IDE1Ljk2MzkgNC45OTg4OSAxNS45NjgzQzQuOTk4ODkgMTUuOTY4MyA0Ljk4MTY5IDE1Ljk2MDggNC45NTI5OSAxNS45NDgyQzQuNTgyOTQgMTUuODU5OCAyLjAwMzM5IDE1LjIxNjggMS4zNjQwMiAxNC4zOTE0QzAuNzExMzg5IDEzLjU1MDggMC4wNzQyMDEyIDExLjg3MzggMC4wMDU4MDgyNyAxMS42OTMxQzAuMDAyNTk1OTMgMTEuNjg3NyAwIDExLjY4NDEgMCAxMS42ODQxQzAgMTEuNjg0MSAwLjAwMjc5MjA3IDExLjY4NzEgMC4wMDQxMzkyMyAxMS42ODg0QzAuMDAxNTQ4NjEgMTEuNjgxMiAwIDExLjY3NjIgMCAxMS42NzYyQzAgMTEuNjc2MiAwLjAxMDI5NTEgMTEuNjg1OCAwLjAzMDAyOTQgMTEuNzA0MlpNMy4zNjYxIDEyLjQ1NjZDMy4zNjAyOSAxMi40NDc0IDMuMzU3MjkgMTIuNDQyNiAzLjM1NzI5IDEyLjQ0MjZDNC40MTM1NSAxMi45NTEgNi4xNTg5NSAxMy45NDEgNi45NTg3MSAxMy45NDkzQzcuMDU4NTggMTMuOTQ4MiA3LjE0Mjg2IDEzLjkzMSA3LjIwODE5IDEzLjg5NDdDMTEuNDg2NCA5LjcwNTI5IDEzLjgyNDggNy40ODE2OSAxNS40MTM3IDYuOTgyNTJDMTguMDAxMiA2LjE2ODY3IDIwLjEzNzggNS45OTkxNSAyMS40NzQyIDZDMjIuNDUwNiA1Ljk5OTM4IDIzLjAwNTcgNi4wODk4OSAyMy4wMDU3IDYuMDg5ODlDMTcuMDU5NyA3LjQ5MzUgMTAuODQzOSAxMi43MTkxIDkuMTE3MDEgMTMuOTk3QzcuMjA3MiAxNS40MDk3IDYuOTE4MTkgMTYuMDAwOSA3LjE1MTY1IDE2LjQ0NzRDNy41NDQ3MSAxNy4xOTU2IDguNzMwMjcgMTguNDg1OCA5LjAyMDMyIDE4Ljc5NjRDOS4wNjE2NiAxOC44NDEgOS4wODUyOSAxOC44NjUgOS4wODUyOSAxOC44NjVDOS4wODUyOSAxOC44NjUgOS4wNzI5OCAxOC44NTk3IDkuMDQ5OCAxOC44NDkxQzkuMDU5ODggMTguODU5NiA5LjA2NTI2IDE4Ljg2NSA5LjA2NTI2IDE4Ljg2NUM5LjA2NTI2IDE4Ljg2NSA3LjExNDcgMTguMDE0IDYuMDMwODggMTYuNTQ0QzUuMzMzMDcgMTUuNTk4NiAzLjMzNzI1IDEyLjQ0MjYgMy4zMzcyNSAxMi40NDI2QzMuMzQ2ODEgMTIuNDQ3MiAzLjM1NjQzIDEyLjQ1MTkgMy4zNjYxIDEyLjQ1NjZaTTEzLjY4ODIgMTQuNzY0NkwxMy42NDA0IDE0Ljc3OUMxMy42NDA0IDE0Ljc3OSAxMy43NDcyIDE0LjcyMzEgMTMuOTcxNyAxNC42MDkzTDEzLjk1MzUgMTQuNjE2NEMxMy45NTgyIDE0LjYxNDUgMTMuOTY0NSAxNC42MTIxIDEzLjk3MjIgMTQuNjA5QzEzLjk5MzQgMTQuNTk4MyAxNC4wMTU1IDE0LjU4NzEgMTQuMDM4NyAxNC41NzUzTDEzLjkzMzQgMTQuNjE2NEMxMy45NTEzIDE0LjYwOTQgMTMuOTkxNCAxNC41OTM2IDE0LjA1NCAxNC41Njc1QzE0LjY1NTEgMTQuMjY0NSAxNS45MzIzIDEzLjYzMTcgMTguMDM2NSAxMi42NDU0QzIwLjU1MzYgMTEuNDY1IDIzLjAzNTggMTAuNDc2NyAyMy4wMzU4IDEwLjQ3NjdDMjMuMDM0NyAxMC40Nzk4IDIzLjAzMzYgMTAuNDgzIDIzLjAzMjQgMTAuNDg2MUMyMy4wNDc5IDEwLjQ3OTkgMjMuMDU1OSAxMC40NzY3IDIzLjA1NTkgMTAuNDc2N0MyMi4yMjQyIDEyLjc4NzMgMTkuNTc5OCAxNy43OTcgMTcuMjE3NiAxOC41NjI2QzE3LjA5OTcgMTguNjAwOSAxNi45MTYxIDE4LjYxOSAxNi42Nzg5IDE4LjYxODVDMTUuMTcxMyAxOC42MjE5IDExLjQzMiAxNy44NDg2IDguNzY1NDIgMTYuNjc4M0M4LjM2MTA3IDE2LjUwMTYgNy44OTY1MyAxNi4xOTIgNy44OTY1MyAxNi4xOTJDNy44OTY1MyAxNi4xOTIgMTQuMjUwNyAxMS42NjUzIDE4LjAxNzYgOS44NjY1OEMyMC44MTYxIDguNDMxMjYgMjMuOTc5OSA4LjE0NDEyIDIzLjk3OTkgOC4xNDQxMkwyMy45Nzk5IDguMTQ2MDdDMjMuOTkzMiA4LjE0NDc1IDI0IDguMTQ0MTIgMjQgOC4xNDQxMkMyMy45ODc2IDguNTM5NTYgMjMuNTg1NCA5LjIzNTkxIDIzLjI5MDMgOS4zNTY4NEMyMy4yNzk3IDkuMzU5NTMgMjMuMjY4NyA5LjM2Mjg3IDIzLjI1NzcgOS4zNjYwOEMxNC4zMTcxIDEzLjAxMzkgMTAuNTMyNCAxNS44NDkxIDEwLjIyMTkgMTYuMDg3NUMxMC4yMjQgMTYuMDg4MSAxMi45OTQ3IDE3LjE4NSAxNS4wNTkzIDE3LjE4ODZDMTUuMzUxNCAxNy4xODgxIDE1LjYyOSAxNy4xNjU0IDE1Ljg4MjMgMTcuMTE0NEMxNy44NzUzIDE2LjcxMzggMTkuNDE3NyAxMy4xOTQxIDE5LjQ5NDQgMTMuMDE2NUwxMy42NjA0IDE0Ljc3OUMxMy42NjA0IDE0Ljc3OSAxMy42Njk2IDE0Ljc3NDMgMTMuNjg4MiAxNC43NjQ2WiIgY2xpcC1ydWxlPSJldmVub2RkIi8+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "ROG Device",
                    "TW": "ROG設備",
                    "CN": "ROG 设备",
                    "BR": "Dispositivo ROG",
                    "CZ": "Zařízení ROG",
                    "DA": "ROG-enhed",
                    "DE": "ROG-Gerät",
                    "ES": "Dispositivo ROG",
                    "FI": "ROG-laite",
                    "FR": "Appareil ROG",
                    "HU": "ROG eszköz",
                    "IT": "Dispositivo ROG",
                    "JP": "ROGデバイス",
                    "KR": "ROG 기기",
                    "MS": "Peranti ROG",
                    "NL": "ROG-apparaat",
                    "NO": "ROG-enhet",
                    "PL": "Urządzenie ROG",
                    "RU": "Устройство ROG",
                    "SV": "ROG-enhet",
                    "TH": "อุปกรณ์ ROG",
                    "TR": "ROG Cihazı",
                    "UK": "Пристрій ROG",
                    "RO": "Dispozitiv ROG",
                    "SL": "Naprava ROG"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 28,
                    "type": [
                        "25"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMva2luZGxlIj48ZyBpZD0iR3JvdXAgMzU0NyI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8wX0tRTFJHWlJLSkYiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS1taXRlcmxpbWl0PSIxMCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik04LjAwMDM2IDIxQzYuODk1NzkgMjEgNi4wMDAzNiAyMC4xMDQ2IDYuMDAwMzYgMTlMNi4wMDAzNiA1QzYuMDAwMzYgMy44OTU0MyA2Ljg5NTc5IDMgOC4wMDAzNiAzTDE2LjkxODcgM0MxOC4wMjMzIDMgMTguOTE4NyAzLjg5NTQzIDE4LjkxODcgNUwxOC45MTg3IDE5QzE4LjkxODcgMjAuMTA0NiAxOC4wMjMzIDIxIDE2LjkxODcgMjFMOC4wMDAzNiAyMVoiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzFfWk9TVlFPSURWQSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLW1pdGVybGltaXQ9IjEwIiBzdHJva2Utd2lkdGg9IjAuOCIgZD0iTTcuODczMDYgMTcuNTcxNEw3Ljg3MzA2IDUuMDgxNjNMMTcuMDMyMiA1LjA4MTYzTDE3LjAzMjIgMTcuNTcxNEw3Ljg3MzA2IDE3LjU3MTRaIi8+PGcgaWQ9IlZlY3Rvcl9fX19fMF8yX0RQR09GWEZURlMiPjxwYXRoIGZpbGw9ImJsYWNrIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGQ9Ik05LjEyMjQ1IDcuMTYzMjdIMTQuMzI2NVoiIGNsaXAtcnVsZT0iZXZlbm9kZCIvPjxwYXRoIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIwLjYiIGQ9Ik05LjEyMjQ1IDcuMTYzMjdIMTQuMzI2NSIvPjwvZz48ZyBpZD0iVmVjdG9yX19fX18wXzNfVFFHWUhMTkxEQSI+PHBhdGggZmlsbD0iYmxhY2siIGZpbGwtcnVsZT0iZXZlbm9kZCIgZD0iTTkuMTIyNDUgOS4yNDQ5SDExLjIwNDFaIiBjbGlwLXJ1bGU9ImV2ZW5vZGQiLz48cGF0aCBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMC42IiBkPSJNOS4xMjI0NSA5LjI0NDlIMTEuMjA0MSIvPjwvZz48ZyBpZD0iVmVjdG9yX19fX18wXzRfQ1ZXSUlCQklBSyI+PHBhdGggZmlsbD0iYmxhY2siIGZpbGwtcnVsZT0iZXZlbm9kZCIgZD0iTTkuMTIyNDUgMTEuMzI2NUgxMy4yODU3WiIgY2xpcC1ydWxlPSJldmVub2RkIi8+PHBhdGggc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjAuNiIgZD0iTTkuMTIyNDUgMTEuMzI2NUgxMy4yODU3Ii8+PC9nPjxnIGlkPSJWZWN0b3JfX19fXzBfNV9CSVVQTUpTVE1JIj48cGF0aCBmaWxsPSJibGFjayIgZmlsbC1ydWxlPSJldmVub2RkIiBkPSJNOS4xMjI0NSAxMy40MDgySDE1LjM2NzNaIiBjbGlwLXJ1bGU9ImV2ZW5vZGQiLz48cGF0aCBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMC42IiBkPSJNOS4xMjI0NSAxMy40MDgySDE1LjM2NzMiLz48L2c+PGcgaWQ9IlZlY3Rvcl9fX19fMF82X1JTSUFKQUdaU0EiPjxwYXRoIGZpbGw9ImJsYWNrIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGQ9Ik05LjEyMjQ1IDE1LjQ4OThIMTUuMzY3M1oiIGNsaXAtcnVsZT0iZXZlbm9kZCIvPjxwYXRoIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIwLjYiIGQ9Ik05LjEyMjQ1IDE1LjQ4OThIMTUuMzY3MyIvPjwvZz48ZyBpZD0iR3JvdXAgMzU0OCI+PGcgaWQ9IlZlY3Rvcl9fX19fMF83X0tCWENFSFlVWVMiPjxwYXRoIGZpbGw9ImJsYWNrIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGQ9Ik0xMS4yMDQ1IDE5LjIzNjlIMTMuNjUzNFoiIGNsaXAtcnVsZT0iZXZlbm9kZCIvPjxwYXRoIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIwLjgiIGQ9Ik0xMS4yMDQ1IDE5LjIzNjlIMTMuNjUzNCIvPjwvZz48ZyBpZD0iVmVjdG9yX19fX18wXzhfQUFaT0dZUFFNVSI+PHBhdGggZmlsbD0iYmxhY2siIGZpbGwtcnVsZT0iZXZlbm9kZCIgZD0iTTE1LjM2NzcgMTkuMjM2OUgxNS44ODgxWiIgY2xpcC1ydWxlPSJldmVub2RkIi8+PHBhdGggc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjAuOCIgZD0iTTE1LjM2NzcgMTkuMjM2OUgxNS44ODgxIi8+PC9nPjxnIGlkPSJWZWN0b3JfX19fXzBfOV9HSEJDSlBPQ1JPIj48cGF0aCBmaWxsPSJibGFjayIgZmlsbC1ydWxlPSJldmVub2RkIiBkPSJNOS4xMjI4MyAxOS4yMzY5SDkuNjQzMjRaIiBjbGlwLXJ1bGU9ImV2ZW5vZGQiLz48cGF0aCBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMC44IiBkPSJNOS4xMjI4MyAxOS4yMzY5SDkuNjQzMjQiLz48L2c+PC9nPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Kindle",
                    "TW": "Kindle",
                    "CN": "Kindle",
                    "BR": "Kindle",
                    "CZ": "Kindle",
                    "DA": "Kindle",
                    "DE": "Kindle",
                    "ES": "Kindle",
                    "FI": "Kindle",
                    "FR": "Kindle",
                    "HU": "Kindle",
                    "IT": "Kindle",
                    "JP": "Kindle",
                    "KR": "Kindle",
                    "MS": "Kindle",
                    "NL": "Kindle",
                    "NO": "Kindle",
                    "PL": "Kindle",
                    "RU": "Kindle",
                    "SV": "Kindle",
                    "TH": "Kindle",
                    "TR": "Kindle",
                    "UK": "Kindle",
                    "RO": "Kindle",
                    "SL": "Kindle"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 29,
                    "type": [
                        "26"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvc2Nhbm5lciI+PGcgaWQ9IlNjYW5uZXIiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMF9VWE9UTlVPTERDIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNNC44OTA4MiAxNS41NTQ2QzQuMzk3ODQgMTUuNTU0NiA0IDE1Ljk1MjQgNCAxNi40NDU0VjE5LjEwOTJDNCAxOS42MDIyIDQuMzk3ODQgMjAgNC44OTA4MiAyMEgxNi40NDU1QzE4LjQwODcgMjAgMjAuMDAwMSAxOC40MDg3IDIwLjAwMDEgMTYuNDQ1NEMyMC4wMDAxIDE1Ljk1MjQgMTkuNjAyMyAxNS41NTQ2IDE5LjEwOTMgMTUuNTU0Nkg0Ljg5MDgyWiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMV9EWUhWQ1FWVklWIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTUuNTU0NyA0TDQuNjkxOSAxMi4zNTQ3QzQuMjUwODIgMTIuNjkyIDQgMTMuMjEwOSA0IDEzLjc2NDRWMTcuMzM2MyIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMl9KVE5TVlRWR1FSIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNNy41NTQ5MiAxMi44OTA5SDE4LjIxODgiLz48ZyBpZD0iR3JvdXBfMiI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8zX0dQUEdOQ1ZZU0IiIGZpbGw9ImJsYWNrIiBkPSJNNS43OTAwOSAxNy43NzczQzUuNzkwMDkgMTcuMjg0MyA2LjE3OTI4IDE2Ljg4NjUgNi42NzIyNSAxNi44ODY1SDYuNjgwOUM3LjE3Mzg4IDE2Ljg4NjUgNy41NzE3MiAxNy4yODQzIDcuNTcxNzIgMTcuNzc3M0M3LjU3MTcyIDE4LjI3MDMgNy4xNzM4OCAxOC42NjgxIDYuNjgwOSAxOC42NjgxQzYuMTg3OTMgMTguNjY4MSA1Ljc5MDA5IDE4LjI3MDMgNS43OTAwOSAxNy43NzczWiIvPjwvZz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Scanner",
                    "TW": "掃描機",
                    "CN": "扫描仪",
                    "BR": "Scanner",
                    "CZ": "Skener",
                    "DA": "Scanner",
                    "DE": "Scanner",
                    "ES": "Escáner",
                    "FI": "Skanneri",
                    "FR": "Scanner",
                    "HU": "Szkenner",
                    "IT": "Scanner",
                    "JP": "スキャナー",
                    "KR": "스캐너",
                    "MS": "Pengimbas",
                    "NL": "Scanner",
                    "NO": "Skanner",
                    "PL": "Skaner",
                    "RU": "Сканер",
                    "SV": "Skanner",
                    "TH": "เครื่องสแกน",
                    "TR": "Tarayıcı",
                    "UK": "Сканер",
                    "RO": "Scanner",
                    "SL": "Skener"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 30,
                    "type": [
                        "32"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvYXBwbGUgZGV2aWNlIj48ZyBpZD0iSW1wb3J0ZWQtTGF5ZXJzLUNvcHktNzIiPjxwYXRoIGlkPSJGaWxsLTFfX19fXzBfMF9aSFNQVkdSRkpWIiBmaWxsPSJibGFjayIgZD0iTTE5Ljg1NDQgMTYuMjA1N0MxOS40MjExIDE3LjE0MjYgMTkuMjEzNiAxNy41NjA3IDE4LjY1NiAxOC4zODg5QzE3Ljg3NzUgMTkuNTQ1IDE2Ljc4MDQgMjAuOTg0OCAxNS40MjA5IDIwLjk5NjlDMTQuMjEyNiAyMS4wMDgxIDEzLjkwMjUgMjAuMjI5OCAxMi4yNjMyIDIwLjIzODZDMTAuNjIzNSAyMC4yNDc0IDEwLjI4MTMgMjEuMDEwOSA5LjA3Mjk1IDIwLjk5OTlDNy43MTM0NyAyMC45ODc4IDYuNjczNzYgMTkuNjg3NyA1Ljg5NTIyIDE4LjUzMTZDMy43MTgyNSAxNS4yOTg2IDMuNDkxMDQgMTEuNTA0NCA0LjgzMzM4IDkuNDg3NjNDNS43ODc3IDguMDU0MjUgNy4yOTI0MSA3LjIxNTQ3IDguNzA3NjggNy4yMTU0N0MxMC4xNDg4IDcuMjE1NDcgMTEuMDU0OCA3Ljk4NjIyIDEyLjI0NiA3Ljk4NjIyQzEzLjQwMTcgNy45ODYyMiAxNC4xMDYgNy4yMTM5NiAxNS43NzIyIDcuMjEzOTZDMTcuMDMxIDcuMjEzOTYgMTguMzY1MiA3Ljg4MjY3IDE5LjMxNjQgOS4wMzk3QzE2LjIwMSAxMC43MDUxIDE2LjcwNjkgMTUuMDQ1MyAxOS44NTQ0IDE2LjIwNTdaIi8+PHBhdGggaWQ9IkZpbGwtMl9fX19fMF8xX0NaUlZNUldXQk0iIGZpbGw9ImJsYWNrIiBkPSJNMTQuNTA3NyA1LjkyMjY0QzE1LjExMjYgNS4xNjQzNSAxNS41NzI3IDQuMDk0MTcgMTUuNDA1OSAzQzE0LjQxNyAzLjA2NjUxIDEzLjI2MDcgMy42ODAyNSAxMi41ODUzIDQuNDgwNDVDMTEuOTcyIDUuMjA2NTYgMTEuNDY2MiA2LjI4NDAzIDExLjY2MzQgNy4zMzA4MkMxMi43NDI3IDcuMzYzMzEgMTMuODU5NCA2LjczNDM5IDE0LjUwNzcgNS45MjI2NFoiLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Apple Device",
                    "TW": "蘋果裝置",
                    "CN": "苹果设备",
                    "BR": "Dispositivo Apple",
                    "CZ": "Zařízení Apple",
                    "DA": "Apple-enhed",
                    "DE": "Apple-Gerät",
                    "ES": "Dispositivo Apple",
                    "FI": "Apple-laite",
                    "FR": "Appareil Apple",
                    "HU": "Apple eszköz",
                    "IT": "Dispositivo Apple",
                    "JP": "Appleデバイス",
                    "KR": "Apple 기기",
                    "MS": "Peranti Apple",
                    "NL": "Apple-apparaat",
                    "NO": "Apple-enhet",
                    "PL": "Urządzenie Apple",
                    "RU": "Устройство Apple",
                    "SV": "Apple-enhet",
                    "TH": "อุปกรณ์ Apple",
                    "TR": "Apple Cihazı",
                    "UK": "Пристрій Apple",
                    "RO": "Dispozitiv Apple",
                    "SL": "Naprava Apple"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 31,
                    "type": [
                        "31"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvYW5kcm9pZCBkZXZpY2UiPjxnIGlkPSJJbXBvcnRlZC1MYXllcnMtQ29weS02OSI+PHBhdGggaWQ9IkZpbGwtMV9fX19fMF8wX0NYVERaU0NHQVQiIGZpbGw9ImJsYWNrIiBkPSJNMTguNDkxMiA4LjY0MDU4QzE3Ljg3ODYgOC42NDA1OCAxNy4zODE4IDkuMTM4NTkgMTcuMzgxOCA5Ljc1Mjg2VjE0LjA5OTNDMTcuMzgxOCAxNC43MTM5IDE3Ljg3ODYgMTUuMjExOSAxOC40OTEyIDE1LjIxMTlDMTkuMTA0MSAxNS4yMTE5IDE5LjYwMDcgMTQuNzEzOSAxOS42MDA3IDE0LjA5OTNWOS43NTI4NkMxOS42MDA3IDkuMTM4NTkgMTkuMTA0MSA4LjY0MDU4IDE4LjQ5MTIgOC42NDA1OFpNNS4xMDk0NSA4LjY0MDU4QzQuNDk2NiA4LjY0MDU4IDQgOS4xMzg1OSA0IDkuNzUyODZWMTQuMDk5M0M0IDE0LjcxMzkgNC40OTY2IDE1LjIxMTkgNS4xMDk0NSAxNS4yMTE5QzUuNzIyMDUgMTUuMjExOSA2LjIxODkxIDE0LjcxMzkgNi4yMTg5MSAxNC4wOTkzVjkuNzUyODZDNi4yMTg5MSA5LjEzODU5IDUuNzIyMDUgOC42NDA1OCA1LjEwOTQ1IDguNjQwNThaIi8+PHBhdGggaWQ9IkZpbGwtMl9fX19fMF8xX0VPWkZTSUpVSVkiIGZpbGw9ImJsYWNrIiBkPSJNNi44ODYwMSA4LjY3NDM0VjE2LjYxNTZDNi44ODYwMSAxNy4wODcyIDcuMjczMjUgMTcuNDY5NCA3Ljc1MDk4IDE3LjQ2OTRIOC43Mzk2VjE5Ljg4NzhDOC43Mzk2IDIwLjUwMiA5LjIzNjIgMjEgOS44NDkwNSAyMUMxMC40NjE2IDIxIDEwLjk1ODUgMjAuNTAyIDEwLjk1ODUgMTkuODg3OFYxNy40Njk0SDEyLjY4NTFWMTkuODg3OEMxMi42ODUxIDIwLjUwMiAxMy4xODIgMjEgMTMuNzk0NiAyMUMxNC40MDcyIDIxIDE0LjkwNCAyMC41MDIgMTQuOTA0IDE5Ljg4NzhWMTcuNDY5NEgxNS44OTI0QzE2LjM3MDQgMTcuNDY5NCAxNi43NTc2IDE3LjA4NzIgMTYuNzU3NiAxNi42MTU2VjguNjc0MzRINi44ODYwMVoiLz48cGF0aCBpZD0iRmlsbC0zX19fX18wXzJfWFJFV1RMSEdFTyIgZmlsbD0iYmxhY2siIGQ9Ik0xNC4xNDA0IDQuNTU5MzRMMTUuMDUyMyAzLjI1MjU0QzE1LjEwNjMgMy4xNzUwMyAxNS4wOTMgMy4wNzMxMSAxNS4wMjIyIDMuMDI0OEMxNC45NTEzIDIuOTc2NzMgMTQuODUwMSAzLjAwMDY0IDE0Ljc5NjEgMy4wNzg0TDEzLjg0ODcgNC40MzU3OEMxMy4yMjQ3IDQuMTkyOTQgMTIuNTMxIDQuMDU3NTYgMTEuODAwNCA0LjA1NzU2QzExLjA2OTggNC4wNTc1NiAxMC4zNzYzIDQuMTkyOTQgOS43NTIwMiA0LjQzNTc4TDguODA0NyAzLjA3ODRDOC43NTA2NiAzLjAwMDY0IDguNjQ5NDUgMi45NzY3MyA4LjU3ODU4IDMuMDI0OEM4LjUwNzcxIDMuMDczMTEgOC40OTQ0NSAzLjE3NTAzIDguNTQ4NSAzLjI1MjU0TDkuNDYwMzggNC41NTkzNEM4LjAxMDYgNS4yMjUyIDYuOTkzOTQgNi40OTAyNCA2Ljg2NDQ0IDcuOTY2NjZIMTYuNzM2M0MxNi42MDY4IDYuNDkwMjQgMTUuNTkwMiA1LjIyNTIgMTQuMTQwNCA0LjU1OTM0Wk05LjcxNzg2IDYuNjE4NThDOS40MTYyOCA2LjYxODU4IDkuMTcxOCA2LjM3NzI1IDkuMTcxOCA2LjA3OTU1QzkuMTcxOCA1Ljc4MTg1IDkuNDE2MjggNS41NDA1MiA5LjcxNzg2IDUuNTQwNTJDMTAuMDE5NyA1LjU0MDUyIDEwLjI2NDIgNS43ODE4NSAxMC4yNjQyIDYuMDc5NTVDMTAuMjY0MiA2LjM3NzI1IDEwLjAxOTcgNi42MTg1OCA5LjcxNzg2IDYuNjE4NThaTTEzLjk1MSA2LjYxODU4QzEzLjY0OTQgNi42MTg1OCAxMy40MDQ5IDYuMzc3MjUgMTMuNDA0OSA2LjA3OTU1QzEzLjQwNDkgNS43ODE4NSAxMy42NDk0IDUuNTQwNTIgMTMuOTUxIDUuNTQwNTJDMTQuMjUyOCA1LjU0MDUyIDE0LjQ5NzMgNS43ODE4NSAxNC40OTczIDYuMDc5NTVDMTQuNDk3MyA2LjM3NzI1IDE0LjI1MjggNi42MTg1OCAxMy45NTEgNi42MTg1OFoiLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Android Device",
                    "TW": "安卓裝置",
                    "CN": "Android 设备",
                    "BR": "Dispositivo Android",
                    "CZ": "Zařízení Android",
                    "DA": "Android-enhed",
                    "DE": "Android-Gerät",
                    "ES": "Dispositivo Android",
                    "FI": "Android-laite",
                    "FR": "Appareil Android",
                    "HU": "Androidos eszköz",
                    "IT": "Dispositivo Android",
                    "JP": "Androidデバイス",
                    "KR": "Android 기기",
                    "MS": "Peranti Android",
                    "NL": "Android-apparaat",
                    "NO": "Android-enhet",
                    "PL": "Urządzenie Android",
                    "RU": "Устройство Android",
                    "SV": "Android-enhet",
                    "TH": "อุปกรณ์ Android",
                    "TR": "Android Cihazı",
                    "UK": "Пристрій з Android",
                    "RO": "Dispozitiv Android",
                    "SL": "Naprava Android"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 32,
                    "type": [
                        "30"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvd2luZG93cyBkZXZpY2UiPjxwYXRoIGlkPSJVbmlvbl9fX19fMF8wX0ZNT1JESE5DUE4iIGZpbGw9ImJsYWNrIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGQ9Ik0yMC4yNjE3IDExLjUzOTVDMTcuMDk0MSAxMS41NzE1IDEzLjkyNjQgMTEuNjAzMyAxMC43NTg4IDExLjYzNTFWNC44OTY5NUwyMC4yNjE3IDNWMTEuNTM5NVpNMTAuNzU4OCAxOS4xMDMzVjEyLjQwNjNDMTMuOTI2NCAxMi40MzgxIDE3LjA5NDEgMTIuNDY5OCAyMC4yNjE3IDEyLjUwMTlWMjAuOTk5OUMxNy4xIDIwLjM2OTEgMTMuOTM4MyAxOS43MzggMTAuNzc2NiAxOS4xMDY4TDEwLjc3MDcgMTkuMTA1NkwxMC43NjQ3IDE5LjEwNDVMMTAuNzU4OCAxOS4xMDMzWk05Ljk1OTcxIDExLjY0MzNWNS4wNTYyQzcuOTczMTQgNS40NTI3MiA1Ljk4NjU3IDUuODQ5MjUgNCA2LjI0NjA3VjExLjcwMzJMOS45NTk3MSAxMS42NDMzWk00IDEyLjMzODJMOS45NTk3MSAxMi4zOTgxVjE4Ljk0MzdMNCAxNy43NTQyVjEyLjMzODJaIiBjbGlwLXJ1bGU9ImV2ZW5vZGQiLz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Windows Device",
                    "TW": "微軟裝置",
                    "CN": "Windows 设备",
                    "BR": "Dispositivo Windows",
                    "CZ": "Zařízení Windows",
                    "DA": "Windows-enhed",
                    "DE": "Windows-Gerät",
                    "ES": "Dispositivo Windows",
                    "FI": "Windows-laite",
                    "FR": "Appareil Windows",
                    "HU": "Windowsos eszköz",
                    "IT": "Dispositivo Windows",
                    "JP": "Windowsデバイス",
                    "KR": "Windows 기기",
                    "MS": "Peranti Windows",
                    "NL": "Windows-apparaat",
                    "NO": "Windows-enhet",
                    "PL": "Urządzenie Windows",
                    "RU": "Устройство Windows",
                    "SV": "Windows-enhet",
                    "TH": "อุปกรณ์ Windows",
                    "TR": "Windows Cihazı",
                    "UK": "Пристрій з Windows",
                    "RO": "Dispozitiv Windows",
                    "SL": "Naprava Windows"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 33,
                    "type": [
                        "42"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJvbmljIERldmljZXMvYXN1cyBkZXZpY2UiPjxwYXRoIGlkPSJVbmlvbl9fX19fMF8wX1FYQUFRQ0hFSkkiIGZpbGw9ImJsYWNrIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGQ9Ik0yMy45OTUgOUwyNCAxMC40MTg1TDE3Ljc4NTggMTAuNDQyNEwxNy43ODU1IDEwLjMyODJDMTcuNzg1NSAxMC4zMjgyIDE3LjkwMzggOS44MzExIDE4LjExNjYgOS41OTQzOEwxOC4xMjU5IDkuNTgzOTlDMTguMzI0NyA5LjM2MTc1IDE4LjU5MzEgOS4wNjE2IDE5LjE5MjIgOS4wMTkzTDIzLjk5NSA5Wk0xNy43NzEyIDkuMDMxNzZMMTcuNzc2MSAxMC40NDI1TDE2LjQzNTMgMTAuNDQ3OUwxNi40MzAzIDkuMDM3MTFMMTcuNzcxMiA5LjAzMTc2Wk03LjMwMDY1IDkuMDY3MjFMMTEuNzg5NCA5LjA0OTE1TDExLjc5NDMgMTAuNDY2NUwyLjM0OTY0IDEwLjUwNDVDMi4zNDk2NCAxMC41MDQ1IDIuODkxMjEgOS41NTIgMy4wMjU4MiA5LjM2NDA5QzMuMTU0MzcgOS4xNzM5MyAzLjMyNTQyIDkuMDkyMjQgMy41MDUzMyA5LjA5MTQ5TDUuODg4NTUgOS4wODE5MUw1Ljg5Mjk4IDEwLjM3NkM1Ljg5Mjk4IDEwLjM3NiA2LjAxMjMzIDkuODc5MDEgNi4yMjUxOSA5LjY0MjE2TDYuMjM1NyA5LjYzMDM5QzYuNDMzOTMgOS40MDg0MSA2LjcwMTA0IDkuMTA5MyA3LjMwMDY1IDkuMDY3MjFaTTEzLjM3MTQgMTAuNDYwMkwxMy4zNjY0IDkuMDQ5NDVMMTIuMDI0MSA5LjA1NDhMMTIuMDI5MiAxMC40NjU2TDEzLjM3MTQgMTAuNDYwMlpNMi4yOTYwMSAxMC41OTAzTDMuNzg2MDEgMTAuNzUyNkwxLjU3ODYxIDE0LjQxNzhMMCAxNC40MjQyTDIuMjk2MDEgMTAuNTkwM1pNMTcuNzk3NiAxMi45NTY3TDIyLjQ2MDUgMTIuOTM3OUMyMi41NjggMTIuOTM3NSAyMi42ODM3IDEyLjg2OTkgMjIuNjgzNyAxMi44Njk5QzIyLjczMiAxMi44MjM0IDIyLjc3MSAxMi43Mzg4IDIyLjc3MDcgMTIuNjU3MkMyMi43Njk4IDEyLjM4NzIgMjIuNTU5MyAxMi4zNzYxIDIyLjQ1MzQgMTIuMzcxQzIyLjQ1MzQgMTIuMzcxIDE5LjE3NjYgMTIuMDk0NyAxOS4wNjI0IDEyLjA4NTRDMTkuMDYyNCAxMi4wODU0IDE4LjYwOCAxMi4wMTMxIDE4LjMwMzggMTEuNzA1N0wxOC4yNzM2IDExLjY3NkMxOC4xNDYgMTEuNTUxIDE3Ljk4NzMgMTEuMzk1NCAxNy44NjU1IDEwLjk1NTFDMTcuODY1NSAxMC45NTUxIDIyLjI0NTkgMTEuMjEwNSAyMi43MzI1IDExLjI1NjdDMjMuNTU0IDExLjMzNjkgMjMuOTQyMSAxMi4xOTU0IDIzLjk3NzggMTIuNDgxNEMyMy45Nzc4IDEyLjQ4MTQgMjQuMDEzOSAxMi43NDAxIDIzLjk3MjIgMTMuMDU3OUMyMy45NzIyIDEzLjA1NzkgMjMuODA2NCAxNC4yNjc3IDIyLjYwMTcgMTQuMzQ1OEwxNy43OTk2IDE0LjM2NTFMMTcuNzk3NiAxMi45NTY3Wk0xMi4wNDQ0IDEyLjUyOTNDMTIuMDEwOCAxMi4zMTkxIDExLjYyOTUgMTEuMzkxMyAxMC44MDg3IDExLjMxMzlDMTAuMzE5MyAxMS4yNjc1IDUuOTYzNTYgMTAuOTE5NSA1Ljk2MzU2IDEwLjkxOTVDNi4wNDkzNyAxMS4zODA5IDYuMjQ2NTEgMTEuNjEzMSA2LjM3NzM5IDExLjc0NTdDNi42ODA4NCAxMi4wNTE3IDcuMTYyMSAxMi4xMzcgNy4xNjIxIDEyLjEzN0M3LjI3ODQyIDEyLjE0ODUgMTAuNTE4OSAxMi40MTg5IDEwLjUxODkgMTIuNDE4OUMxMC42MjIxIDEyLjQyNDIgMTAuODE3OSAxMi40NTEyIDEwLjgxNjkgMTIuNzIxOEMxMC44MTcgMTIuNzU0OCAxMC43ODk1IDEyLjk5MzQgMTAuNTQwNyAxMi45OTQzTDUuODQ0OSAxMy4wMTMyTDUuODM3NTcgMTAuOTA4MUw0LjUzMjM2IDEwLjgxMjNMNC41NDQ5NCAxNC40MDY1TDEwLjgyMjEgMTQuMzgxM0MxMS45MTQ1IDE0LjE0NTEgMTIuMDI1IDEzLjA5NDEgMTIuMDI1IDEzLjA5NDFDMTIuMDM5NSAxMy4wMDM4IDEyLjA0NjcgMTIuOTE5OSAxMi4wNTA1IDEyLjg0NDNMMTIuMDQ5NSAxMi41OTI1QzEyLjA0NzEgMTIuNTUzNCAxMi4wNDQ0IDEyLjUyOTMgMTIuMDQ0NCAxMi41MjkzWk0xMi4wNDk2IDEyLjU5MjVDMTIuMDUzMSAxMi42NDc1IDEyLjA1NTkgMTIuNzM1NSAxMi4wNTA2IDEyLjg0NDNMMTIuMDUxNCAxMy4wNzY3QzEyLjI3MSAxNC4yODAxIDEzLjI5ODQgMTQuMzY3OSAxMy4yOTg0IDE0LjM2NzlDMTMuMjk4NCAxNC4zNjc5IDEzLjQwMjkgMTQuMzc1NCAxMy40MiAxNC4zNzY5TDE2LjQ4MDUgMTQuMzY0NkMxNi40ODA1IDE0LjM2NDYgMTcuNzg5NyAxNC4yNDcyIDE3Ljc4NSAxMi45MTcyTDE3Ljc3ODEgMTAuOTUxNUwxNi40NTk1IDEwLjg3MzRMMTYuNDY1NCAxMi41MDk3QzE2LjQ2NTQgMTIuNTA5NyAxNi40NjMxIDEyLjk4MTMgMTYuMDMyNCAxMi45ODNMMTMuNzY2NiAxMi45OTIxQzEzLjc2NjYgMTIuOTkyMSAxMy4zODI5IDEyLjk2MDEgMTMuMzgxNCAxMi41MjY2TDEzLjM3NDkgMTAuNjYzN0wxMi4wNDI1IDEwLjU2NzJMMTIuMDQ5NiAxMi41OTI1WiIgY2xpcC1ydWxlPSJldmVub2RkIi8+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "ASUS Device",
                    "TW": "ASUS 裝置",
                    "CN": "ASUS 设备",
                    "BR": "Dispositivo ASUS",
                    "CZ": "Zařízení ASUS",
                    "DA": "ASUS-enhed",
                    "DE": "ASUS-Gerät",
                    "ES": "Dispositivo ASUS",
                    "FI": "ASUS-laite",
                    "FR": "Périphérique ASUS",
                    "HU": "ASUS eszköz",
                    "IT": "Dispositivo ASUS",
                    "JP": "ASUSデバイス",
                    "KR": "ASUS 기기",
                    "MS": "Peranti ASUS",
                    "NL": "ASUS-apparaat",
                    "NO": "ASUS-enhet",
                    "PL": "Urządzenie ASUS",
                    "RU": "Устройство ASUS",
                    "SV": "ASUS-enhet",
                    "TH": "อุปกรณ์ ASUS",
                    "TR": "ASUS Cihazı",
                    "UK": "Пристрій ASUS",
                    "RO": "Dispozitiv ASUS",
                    "SL": "Naprava ASUS"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 34,
                    "type": [
                        "78"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48bWFzayBpZD0iYSIgZmlsbD0iI2ZmZiI+PHBhdGggZmlsbC1ydWxlPSJldmVub2RkIiBkPSJNNC45NTIgNy43NjRjLjAwNy0yLjEzIDEuOTI3LTMuOTQ4IDQuNDE3LTMuOTQ4IDEuNTg1IDAgMy4xNzUuNjQzIDQuNTUxIDIuMzMzYS42LjYgMCAwIDAgLjkwNS4wM2MuNDgyLS4yMjcgMS4wMjgtLjQxOSAxLjU3My0uMTQ2LjIwNS4xMDIuNDgyLjI4OC43LjUzLjIxLjIzNS4zMy40ODEuMzM3LjczM3YuMDI1YS42LjYgMCAwIDAgLjYuNmMuOTc0IDAgMi4xMzYgMS4wNzMgMi4xMzYgMi41OTJhMy4wNSAzLjA1IDAgMCAxLTIuNTI3IDMuMDA1LjYuNiAwIDAgMCAuMjAzIDEuMTgyIDQuMjUgNC4yNSAwIDAgMCAzLjUyNC00LjE4N2MwLTEuODQzLTEuMjcyLTMuNDM3LTIuODEtMy43NGEyLjUzMiAyLjUzMiAwIDAgMC0uNTctMS4wMSAzLjY2NyAzLjY2NyAwIDAgMC0xLjA1Ni0uODAzYy0uOTQ1LS40NzMtMS44NTMtLjIwNi0yLjQyLjA0LTEuNTI3LTEuNjc3LTMuMzIzLTIuMzg0LTUuMTQ2LTIuMzg0LTIuOTA4IDAtNS4zOCAyLjA1NS01LjYwMSA0Ljc2NS0uNTc3LjI4Ni0xLjAzNi42NjUtMS4zNSAxLjIxMi0uMzY2LjYzNi0uNDkgMS40MjQtLjQ5IDIuMzc3IDAgMi4yMzUgMS45ODcgMy43OTIgNC4yNDggMy43OTJhLjYuNiAwIDEgMCAwLTEuMmMtMS43NyAwLTMuMDQ5LTEuMTgtMy4wNDktMi41OTIgMC0uODcyLjEyLTEuNDEuMzMxLTEuNzc5LjItLjM0Ny41MjQtLjYxNSAxLjEwNy0uODUzYS41OTguNTk4IDAgMCAwIC4zODctLjU3NFoiIGNsaXAtcnVsZT0iZXZlbm9kZCIvPjwvbWFzaz48cGF0aCBmaWxsPSIjMDAwIiBkPSJtNC45NTIgNy43NjQtMS4yLS4wMDR2LjAzbDEuMi0uMDI2Wm04Ljk2OC0xLjYxNS45My0uNzU4LS45My43NThabS44NDUuMDg2LS43NTgtLjkzLjc1OC45M1ptLjA2LS4wNTYtLjUxMi0xLjA4Ni0uMjA4LjA5OS0uMTU4LjE2OC44NzguODE5Wm0xLjU3My0uMTQ2LS41MzYgMS4wNzMuNTM2LTEuMDczWm0uNy41My0uODk0LjgwMS44OTMtLjhabS4zMzcuNzMzIDEuMTk5LjA1LjAwMi0uMDQzLS4wMDItLjA0My0xLjIuMDM2Wm0uMjA5IDYuMjIyLS4yMDQtMS4xODMuMjAzIDEuMTgzWm0tLjQ5LjY5MyAxLjE4My0uMjA0LTEuMTgzLjIwNFptLjY5My40OS0uMjA0LTEuMTgzLjIwNCAxLjE4MlptLjcxNC03LjkyOC0xLjE1Ny4zMTYuMTk2LjcxNy43MjkuMTQ0LjIzMi0xLjE3N1ptLS41Ny0xLjAxLjg5NC0uODAxLS44OTQuOFptLTEuMDU2LS44MDMuNTM3LTEuMDc0LS41MzcgMS4wNzRabS0yLjQyLjA0LS44ODguODA4LjU3Ny42MzQuNzg3LS4zNEwxNC41MTQgNVpNMy43NjcgNy4zODEgNC4zIDguNDU2bC42MDgtLjMwMS4wNTUtLjY3Ni0xLjE5Ni0uMDk4Wm0tMS4zNSAxLjIxMiAxLjA0LjU5OC0xLjA0LS41OThabTEuMDQuNTk4LTEuMDQtLjU5OCAxLjA0LjU5OFptMS4xMDctLjg1My0uNDI2LTEuMTIyLS4wMTQuMDA1LS4wMTQuMDA2LjQ1NCAxLjExWm0uMDk2LS4wNDctLjYxNy0xLjAzLS4wMDIuMDAyLjYxOSAxLjAyOFptLjE4OC0uMTc5LS45OTQtLjY3MS0uMDAxLjAwMS45OTUuNjdabS4wOTUtLjI0MkwzLjc2IDcuNjgydi4wMDNsMS4xODUuMTg1Wk05LjM3IDIuNjE2Yy0zLjA0NiAwLTUuNjA3IDIuMjUzLTUuNjE3IDUuMTQ0bDIuNC4wMDhDNi4xNTcgNi40IDcuNDM2IDUuMDE2IDkuMzY5IDUuMDE2di0yLjRabTUuNDgyIDIuNzc1Yy0xLjU5Ni0xLjk1OS0zLjUyMy0yLjc3NS01LjQ4Mi0yLjc3NXYyLjRjMS4yMSAwIDIuNDY0LjQ3IDMuNjIgMS44OWwxLjg2Mi0xLjUxNVptLS44NDQtLjA4NmEuNi42IDAgMCAxIC44NDQuMDg2TDEyLjk5IDYuOTA3YTEuOCAxLjggMCAwIDAgMi41MzIuMjU5bC0xLjUxNS0xLjg2MVptLS4wNi4wNTVhLjYwNy42MDcgMCAwIDEgLjA2LS4wNTVsMS41MTUgMS44NmMuMDY0LS4wNTIuMTI1LS4xMDguMTgtLjE2OEwxMy45NDcgNS4zNlptMi45ODgtLjRjLTEuMDk2LS41NDgtMi4xMjUtLjEtMi42MjIuMTMzbDEuMDIzIDIuMTcxYy4yMTUtLjEuMzQ3LS4xNDYuNDQtLjE2LjA2OC0uMDExLjA3OS0uMDAxLjA4Ni4wMDJsMS4wNzMtMi4xNDZabTEuMDU2LjgwM2EzLjY2NyAzLjY2NyAwIDAgMC0xLjA1Ni0uODAzbC0xLjA3MyAyLjE0NmExLjI1IDEuMjUgMCAwIDEgLjM0Mi4yNThsMS43ODctMS42MDFabS42NDMgMS40OTdjLS4wMTgtLjYyNC0uMzE0LTEuMTMtLjY0My0xLjQ5N2wtMS43ODcgMS42MDFjLjAyLjAyNC4wMzQuMDQxLjA0Mi4wNTNsLjAwOS4wMTRhLjMyLjMyIDAgMCAxLS4wMi0uMWwyLjQtLjA3MVptMCAuMDZ2LjAyNWwtMi4zOTgtLjA5OS0uMDAyLjA3NWgyLjRabS0uNi0uNmEuNi42IDAgMCAxIC42LjZoLTIuNGExLjggMS44IDAgMCAwIDEuOCAxLjh2LTIuNFptMy4zMzcgMy43OTNhMy45NzggMy45NzggMCAwIDAtMS4wMTEtMi42NjhjLS41OC0uNjQ0LTEuNDE3LTEuMTI0LTIuMzI2LTEuMTI0djIuNGMuMDY2IDAgLjI5Ny4wNTYuNTQyLjMyOS4yMjUuMjUuMzk1LjYxNy4zOTUgMS4wNjNoMi40Wk0xNy44NDcgMTQuN2E0LjI1IDQuMjUgMCAwIDAgMy41MjQtNC4xODdoLTIuNGExLjg1IDEuODUgMCAwIDEtMS41MzEgMS44MjJsLjQwNyAyLjM2NVptLjQ5LS42OTNhLjYuNiAwIDAgMS0uNDkuNjkzbC0uNDA3LTIuMzY1YTEuOCAxLjggMCAwIDAtMS40NjkgMi4wOGwyLjM2Ni0uNDA4Wm0tLjY5NC0uNDlhLjYuNiAwIDAgMSAuNjk0LjQ5bC0yLjM2Ni40MDdhMS44IDEuOCAwIDAgMCAyLjA4IDEuNDdsLS40MDctMi4zNjZabTIuNTI4LTMuMDA0YTMuMDUgMy4wNSAwIDAgMS0yLjUyNyAzLjAwNWwuNDA3IDIuMzY1YTUuNDUgNS40NSAwIDAgMCA0LjUyLTUuMzdoLTIuNFpNMTguMzI5IDcuOTVjLjkwNS4xNzkgMS44NDIgMS4yMiAxLjg0MiAyLjU2M2gyLjRjMC0yLjM0Mi0xLjYwOS00LjQ5LTMuNzc3LTQuOTE4bC0uNDY1IDIuMzU1Wm0tMS4yMzItMS4zODZjLjE1MS4xNjguMjU4LjM0Ny4zMDcuNTI1bDIuMzE1LS42MzNhMy43MyAzLjczIDAgMCAwLS44MzQtMS40OTRsLTEuNzg4IDEuNjAyWm0tLjY5OS0uNTNjLjIwNS4xMDEuNDgyLjI4Ny43LjUzbDEuNzg3LTEuNjAyYTQuODY3IDQuODY3IDAgMCAwLTEuNDEzLTEuMDc1bC0xLjA3NCAyLjE0NlptLTEuNDA3LjA2N2MuNDYtLjE5OS45NDQtLjMgMS40MDctLjA2OGwxLjA3NC0yLjE0N2MtMS40MjYtLjcxMy0yLjc2LS4yOC0zLjQzNC4wMTNMMTQuOTkgNi4xWk05LjM2OSAzLjgxNmMxLjQ4IDAgMi45NTYuNTYxIDQuMjU4IDEuOTkybDEuNzc1LTEuNjE2Yy0xLjc1LTEuOTIzLTMuODY3LTIuNzc2LTYuMDMzLTIuNzc2djIuNFpNNC45NjQgNy40NzljLjE2Mi0xLjk5NiAyLjAyNi0zLjY2MyA0LjQwNS0zLjY2M3YtMi40Yy0zLjQzNiAwLTYuNTE4IDIuNDQ0LTYuNzk3IDUuODY4bDIuMzkyLjE5NVpNMy40NTggOS4xOWMuMTctLjI5NS40MjYtLjUyOC44NDMtLjczNWwtMS4wNjctMi4xNWMtLjczNi4zNjYtMS4zOTguODktMS44NTYgMS42ODlsMi4wOCAxLjE5NlptLS4zMyAxLjc3OGMwLS44NzEuMTE4LTEuNDEuMzMtMS43NzhsLTIuMDgtMS4xOTZjLS41Mi45MDQtLjY1IDEuOTQtLjY1IDIuOTc0aDIuNFptMy4wNDggMi41OTNjLTEuNzc3IDAtMy4wNDktMS4xODYtMy4wNDktMi41OTJoLTIuNGMwIDMuMDY0IDIuNzA1IDQuOTkyIDUuNDQ5IDQuOTkydi0yLjRabS0uNi42YS42LjYgMCAwIDEgLjYtLjZ2Mi40YTEuOCAxLjggMCAwIDAgMS44LTEuOGgtMi40Wm0uNi42YS42LjYgMCAwIDEtLjYtLjZoMi40YTEuOCAxLjggMCAwIDAtMS44LTEuOHYyLjRaTTEuOTI3IDEwLjk3YzAgMi4yMjYgMS45NzcgMy43OTMgNC4yNDkgMy43OTN2LTIuNGMtMS4yNjcgMC0xLjg0OS0uNzkyLTEuODQ5LTEuMzkzaC0yLjRabS40OS0yLjM3NmMtLjM2NS42MzctLjQ5IDEuNDI0LS40OSAyLjM3N2gyLjRjMC0uNzkyLjExNC0xLjA4LjE3MS0xLjE4bC0yLjA4LTEuMTk3Wm0xLjY5NC0xLjM2NmMtLjc0LjMwMi0xLjMyLjcxNi0xLjY5MyAxLjM2NmwyLjA4IDEuMTk2YS4zOTkuMzk5IDAgMCAxIC4xMS0uMTIxYy4wNjgtLjA1My4xOS0uMTMuNDEtLjIxOUw0LjExIDcuMjI3Wm0tLjA3LjAzNmEuNjAzLjYwMyAwIDAgMSAuMDk4LS4wNDdMNC45OSA5LjQ2YTEuODQgMS44NCAwIDAgMCAuMjg5LS4xNDFMNC4wNDIgNy4yNjNabS0uMTg3LjE4YS42MDMuNjAzIDAgMCAxIC4xOS0uMTgxTDUuMjc4IDkuMzJjLjIzMS0uMTM4LjQyMy0uMzI0LjU2Ny0uNTM4bC0xLjk5MS0xLjM0Wm0tLjA5NS4yNDJhLjYwMy42MDMgMCAwIDEgLjA5Ni0uMjQ0bDEuOTg5IDEuMzQzYy4xNDQtLjIxNC4yNDQtLjQ2MS4yODYtLjcyOGwtMi4zNzEtLjM3MVptLS4wMDcuMTA1YzAtLjAzNy4wMDItLjA3My4wMDctLjEwOGwyLjM3LjM3N2MuMDE3LS4xMDUuMDI1LS4yMTIuMDIyLS4zMmwtMi40LjA1WiIgbWFzaz0idXJsKCNhKSIvPjxwYXRoIHN0cm9rZT0iIzAwMCIgc3Ryb2tlLW1pdGVybGltaXQ9IjEwIiBkPSJNMTcuNSA5aC0xMXYzLjY2N2gxMVY5WiIvPjxwYXRoIGZpbGw9IiMwMDAiIGQ9Ik04LjMzMyAxMC4xSDcuNnYxLjQ2NmgyLjJWMTAuMWgtLjczNG0yLjIwMSAwaC0uNzM0djEuNDY2aDIuMlYxMC4xSDEybTIuOTMzIDEuNzZhMS4wMjcgMS4wMjcgMCAxIDAgMC0yLjA1MyAxLjAyNyAxLjAyNyAwIDAgMCAwIDIuMDUzWiIvPjxwYXRoIHN0cm9rZT0iIzAwMCIgc3Ryb2tlLW1pdGVybGltaXQ9IjEwIiBkPSJNMTcuNSAxNGgtMTF2My42NjdoMTFWMTRaIi8+PHBhdGggZmlsbD0iIzAwMCIgZD0iTTguMzMzIDE1LjFINy42djEuNDY2aDIuMlYxNS4xaC0uNzM0bTIuMjAxIDBoLS43MzR2MS40NjZoMi4yVjE1LjFIMTJtMi45MzMgMS43NmExLjAyNyAxLjAyNyAwIDEgMCAwLTIuMDUzIDEuMDI3IDEuMDI3IDAgMCAwIDAgMi4wNTNaIi8+PHBhdGggc3Ryb2tlPSIjMDAwIiBkPSJNMTIgMTcuNXYybTYgLjVoLTRtLTQgMEg2Ii8+PHJlY3Qgd2lkdGg9IjQiIGhlaWdodD0iMiIgeD0iMTAiIHk9IjE5IiBzdHJva2U9IiMwMDAiIHJ4PSIxIi8+PC9zdmc+",
                    "EN": "Gateways",
                    "TW": "閘道器",
                    "CN": "网关",
                    "BR": "Gateways",
                    "CZ": "Brány",
                    "DA": "Gateways",
                    "DE": "Gateways",
                    "ES": "Puertas de enlace",
                    "FI": "Yhdyskäytävät",
                    "FR": "Passerelles",
                    "HU": "Átjárók",
                    "IT": "Gateway",
                    "JP": "ゲートウェイ",
                    "KR": "게이트웨이",
                    "MS": "Gateways",
                    "NL": "Gateways",
                    "NO": "Gatewayer",
                    "PL": "Bramy",
                    "RU": "Шлюзы",
                    "SV": "Gateways",
                    "TH": "เกตเวย์",
                    "TR": "Ağ geçitleri",
                    "UK": "Шлюзи",
                    "RO": "Gateway-uri",
                    "SL": "Prehodi"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 35,
                    "type": [
                        "86"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48cGF0aCBzdHJva2U9IiMwMDAiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMSAxMC4wOTRoMjJtLTIyIDB2NC4zODNjMCAuOTAxLjczMSAxLjYzMiAxLjYzMyAxLjYzMk0xIDEwLjA5NCA0LjQzOCA3aDE0Ljg2N0wyMyAxMC4wOTRtMCAwdjQuNDY5YzAgLjg1NC0uNjkzIDEuNTQ2LTEuNTQ3IDEuNTQ2bS0xOC44MiAwdi4yODRhMS4zMzIgMS4zMzIgMCAwIDAgMi42NjQgMHYtLjI4NG0tMi42NjQgMGgyLjY2NG0wIDBoMTMuMzJtMCAwdi4xOThhMS40MTggMS40MTggMCAxIDAgMi44MzYgMHYtLjE5OG0tMi44MzYgMGgyLjgzNk0xIDE3LjcyNWgyMk01LjA0IDEyLjAwOUgzLjY4OGEuMTM1LjEzNSAwIDAgMC0uMTM1LjEzNXYuNDA1aC0uMjdhLjEzNS4xMzUgMCAwIDAtLjEzNi4xMzV2MS40ODZjMCAuMDc0LjA2LjEzNS4xMzUuMTM1aDIuMTYxYy4wNzUgMCAuMTM1LS4wNi4xMzUtLjEzNXYtMS40ODZhLjEzNS4xMzUgMCAwIDAtLjEzNS0uMTM1aC0uMjd2LS40MDVhLjEzNS4xMzUgMCAwIDAtLjEzNS0uMTM1Wm01LjA3IDBIOC43NThhLjEzNS4xMzUgMCAwIDAtLjEzNS4xMzV2LjQwNWgtLjI3YS4xMzUuMTM1IDAgMCAwLS4xMzUuMTM1djEuNDg2YzAgLjA3NC4wNi4xMzUuMTM1LjEzNWgyLjE2Yy4wNzUgMCAuMTM2LS4wNi4xMzYtLjEzNXYtMS40ODZhLjEzNS4xMzUgMCAwIDAtLjEzNi0uMTM1aC0uMjd2LS40MDVhLjEzNS4xMzUgMCAwIDAtLjEzNS0uMTM1Wm01LjEzMSAwaC0xLjM1YS4xMzUuMTM1IDAgMCAwLS4xMzUuMTM1di40MDVoLS4yN2EuMTM1LjEzNSAwIDAgMC0uMTM2LjEzNXYxLjQ4NmMwIC4wNzQuMDYuMTM1LjEzNi4xMzVoMi4xNmMuMDc1IDAgLjEzNS0uMDYuMTM1LS4xMzV2LTEuNDg2YS4xMzUuMTM1IDAgMCAwLS4xMzUtLjEzNWgtLjI3di0uNDA1YS4xMzUuMTM1IDAgMCAwLS4xMzUtLjEzNVptNS4wNyAwaC0xLjM1YS4xMzUuMTM1IDAgMCAwLS4xMzUuMTM1di40MDVoLS4yN2EuMTM1LjEzNSAwIDAgMC0uMTM1LjEzNXYxLjQ4NmMwIC4wNzQuMDYuMTM1LjEzNS4xMzVoMi4xNmMuMDc1IDAgLjEzNi0uMDYuMTM2LS4xMzV2LTEuNDg2YS4xMzUuMTM1IDAgMCAwLS4xMzYtLjEzNWgtLjI3di0uNDA1YS4xMzUuMTM1IDAgMCAwLS4xMzUtLjEzNVoiLz48L3N2Zz4=",
                    "EN": "Network Switch",
                    "TW": "網路交換器",
                    "CN": "网络交换机",
                    "BR": "Comutador de rede",
                    "CZ": "Síťový spínač",
                    "DA": "Netværksswitch",
                    "DE": "Netzwerk-Switch",
                    "ES": "Conmutador de red",
                    "FI": "Verkkokytkin",
                    "FR": "Commutateur de réseau",
                    "HU": "Hálózati kapcsoló",
                    "IT": "Switch di rete",
                    "JP": "ネットワーク スイッチ",
                    "KR": "네트워크 스위치",
                    "MS": "Network Switch",
                    "NL": "Netwerkschakelaar",
                    "NO": "Nettverkssvitsj",
                    "PL": "Przełącznik sieciowy",
                    "RU": "Сетевой коммутатор",
                    "SV": "Nätverksswitch",
                    "TH": "สวิตช์เครือข่าย",
                    "TR": "Ağ Anahtarı",
                    "UK": "Мережевий комутатор",
                    "RO": "Comutator de rețea",
                    "SL": "Omrežno stikalo"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 36,
                    "type": [
                        "87"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48cGF0aCBzdHJva2U9IiMwMDAiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik04LjYxNSAxNi4yNTdhNS4wNiA1LjA2IDAgMCAwIDMuNTkgMS40ODcgNS4wNiA1LjA2IDAgMCAwIDMuNTktMS40ODdNMTQgMTQuNDYyYTIuNTMgMi41MyAwIDAgMS0xLjc5NS43NDQgMi41MyAyLjUzIDAgMCAxLTEuNzk1LS43NDQiLz48cGF0aCBzdHJva2U9IiMwMDAiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMjEuMzU0IDYuODQ3SDIuMzljLS4zIDAtLjU0NC4yNDMtLjU0NC41NDR2Mi44OThjMCAuMjc5LjE2LjUzMy40MS42NTRsMS42MDYuNzczYy4yODcuMTM4LjU4OC4yNDQuODk5LjMxN2wyLjE3MS41MDZhLjcxLjcxIDAgMCAwIC4xNjUuMDJoOS40NzVjLjI3NSAwIC41NS0uMDMyLjgxNy0uMDk1bC4wOTQtLjAyMmEyMC43MTIgMjAuNzEyIDAgMCAwIDQuMjgtMS41MDkuNjkuNjkgMCAwIDAgLjM5LS42MjJWNy42NDdhLjguOCAwIDAgMC0uOC0uOFoiLz48Y2lyY2xlIGN4PSIxMC4zMDgiIGN5PSI5LjM4NSIgcj0iLjg0NiIgZmlsbD0iIzAwMCIvPjxjaXJjbGUgY3g9IjEzLjY5MiIgY3k9IjkuMzg1IiByPSIuODQ2IiBmaWxsPSIjMDAwIi8+PGNpcmNsZSBjeD0iMTcuMDc3IiBjeT0iOS4zODUiIHI9Ii44NDYiIGZpbGw9IiMwMDAiLz48Y2lyY2xlIGN4PSI2LjkyMyIgY3k9IjkuMzg1IiByPSIuODQ2IiBmaWxsPSIjMDAwIi8+PHBhdGggc3Ryb2tlPSIjMDAwIiBkPSJNMSA1LjVoMjIiLz48L3N2Zz4=",
                    "EN": "Wireless AP",
                    "TW": "無線 AP",
                    "CN": "无线 AP",
                    "BR": "AP sem fio",
                    "CZ": "Bezdrátový přístupový bod",
                    "DA": "Trådløs AP",
                    "DE": "WLAN-AP",
                    "ES": "AP inalámbrico",
                    "FI": "Langaton tukiasema",
                    "FR": "PA sans fil",
                    "HU": "Vezeték nélküli hozzáférési pont",
                    "IT": "AP wireless",
                    "JP": "無線AP",
                    "KR": "무선 AP",
                    "MS": "Wireless AP",
                    "NL": "Draadloos toegangspunt",
                    "NO": "Trådløst tilgangspunkt",
                    "PL": "Punkt dostępu bezprzewodowego",
                    "RU": "Беспроводная точка доступа",
                    "SV": "Trådlös åtkomstpunkt",
                    "TH": "จุดเข้าใช้งานแบบไร้สาย",
                    "TR": "Kablosuz AP",
                    "UK": "Бездротова точка доступу",
                    "RO": "Punct de acces wireless",
                    "SL": "Brezžična dostopna točka"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 37,
                    "type": [
                        "88"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBjbGlwLXBhdGg9InVybCgjYSkiPjxwYXRoIHN0cm9rZT0iIzAwMCIgZD0iTTcuMTExIDMuMTI3YS41NTIuNTUyIDAgMSAxIC44OC0uNjY2bDExLjQ5NyAxNS4yODRhLjY5LjY5IDAgMCAxLS4xNTEuOTc4TTcuMTEgMy4xMjdsMTEuNTQ0IDE1LjQwMk03LjExMSAzLjEyN2MtLjE4NCAxLjUzLjI5MiA0LjYzOCAxLjg2NiA3Ljc0NW05LjY3OCA3LjY1Ny4xMTEuMTNhLjQxOS40MTkgMCAwIDAgLjU3LjA2NG0tLjY4LS4xOTRjLS42Ni0uMDQyLTEuMzE3LjI1Ni02LjgzLTQuMm0tMi44NDktMy40NTdjLjM5LjYxLjgyIDEuMTUzIDEuMzE3IDEuNzVhMzEuNzYyIDMxLjc2MiAwIDAgMCAxLjUzMiAxLjcwN20tMi44NDktMy40NTdhMS42MyAxLjYzIDAgMCAwLS4yNjkgMi4yOGwxLjMzMiAxLjcwMm0xLjc4Ni0uNTI1LS4zMDcuMjNtMCAwLS42Ny40MzJhLjYxLjYxIDAgMCAxLS44MS0uMTM3bTEuNDgtLjI5NHY1LjQ1N20wIDBIMTAuMDRtMS40OCAwaDMuNTU4Yy41IDAgLjkwNS40MDUuOTA1LjkwNCAwIC4yNS0uMjAzLjQ1My0uNDUzLjQ1M0g1Ljk3N2EuNDUyLjQ1MiAwIDAgMS0uNDUyLS40NTNjMC0uNDk5LjQwNS0uOTA0LjkwNC0uOTA0aDMuNjFtMC01LjE2M3Y1LjE2M205LjI5OC0xLjI5NGExLjAyIDEuMDIgMCAwIDAgMS4wMjEtMS4wMjFWNi40NTJtLjM3LTEuNDY5YS45Mi45MiAwIDAgMC0xLjI3NC0uMTg4bC0uODg4LjY0NmEuOTM2LjkzNiAwIDEgMCAxLjEyMyAxLjQ5N2wuODY4LS42NzNhLjkyLjkyIDAgMCAwIC4xNy0xLjI4MloiLz48L2c+PGRlZnM+PGNsaXBQYXRoIGlkPSJhIj48cGF0aCBmaWxsPSIjZmZmIiBkPSJNMCAwaDI0djI0SDB6Ii8+PC9jbGlwUGF0aD48L2RlZnM+PC9zdmc+",
                    "EN": "Satellite Dish",
                    "TW": "衛星天線",
                    "CN": "卫星天线",
                    "BR": "Antena parabólica",
                    "CZ": "Satelitní parabola",
                    "DA": "Parabolantenne",
                    "DE": "Satellitenschüssel",
                    "ES": "Antena parabólica",
                    "FI": "Satelliittilautanen",
                    "FR": "Antenne parabolique",
                    "HU": "Műholdvevő tányér",
                    "IT": "Piastra satellitare",
                    "JP": "衛星放送アンテナ",
                    "KR": "위성 안테나",
                    "MS": "Satellite Dish",
                    "NL": "Satellietschotel",
                    "NO": "Parabolantenne",
                    "PL": "Antena satelitarna",
                    "RU": "Спутниковая тарелка",
                    "SV": "Parabolantenn",
                    "TH": "จานดาวเทียม",
                    "TR": "Uydu Anteni",
                    "UK": "Супутникова антена",
                    "RO": "Antenă de satelit",
                    "SL": "Satelitski krožnik"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 38,
                    "type": [
                        "89"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48cGF0aCBzdHJva2U9IiMwMDAiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMjMgMTMuMzgzYzAgMi40Mi00LjkyNSA0LjM4My0xMSA0LjM4M1MxIDE1LjgwMyAxIDEzLjM4M20yMiAwQzIzIDEwLjk2MyAxOC4wNzUgOSAxMiA5UzEgMTAuOTYyIDEgMTMuMzgzbTIyIDBhNy40NzcgNy40NzcgMCAwIDEtNy40NzcgNy40NzZIOC40NzdBNy40NzcgNy40NzcgMCAwIDEgMSAxMy4zODMiLz48cGF0aCBzdHJva2U9IiMwMDAiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xNS4yNDIgNC41QTUuMTA0IDUuMTA0IDAgMCAwIDExLjYyIDMgNS4xMDUgNS4xMDUgMCAwIDAgOCA0LjVtMS44MSAxLjgxYTIuNTUyIDIuNTUyIDAgMCAxIDEuODEtLjc1Yy43MDggMCAxLjM0OC4yODcgMS44MTEuNzVtLTIuMzc2IDEzLjE3NGgxLjg5Ii8+PGNpcmNsZSBjeD0iOC45MDYiIGN5PSIxMS4wNjMiIHI9Ii41MTYiIGZpbGw9IiMwMDAiLz48Y2lyY2xlIGN4PSIxMC45NjkiIGN5PSIxMS4wNjMiIHI9Ii41MTYiIGZpbGw9IiMwMDAiLz48Y2lyY2xlIGN4PSIxMy4wMzEiIGN5PSIxMS4wNjMiIHI9Ii41MTYiIGZpbGw9IiMwMDAiLz48Y2lyY2xlIGN4PSIxNS4wOTQiIGN5PSIxMS4wNjMiIHI9Ii41MTYiIGZpbGw9IiMwMDAiLz48L3N2Zz4=",
                    "EN": "Wireless Device",
                    "TW": "無線裝置",
                    "CN": "无线设备",
                    "BR": "Dispositivo sem fio",
                    "CZ": "Bezdrátové zařízení",
                    "DA": "Trådløs enhed",
                    "DE": "WLAN-Gerät",
                    "ES": "Dispositivo inalámbrico",
                    "FI": "Langaton laite",
                    "FR": "Périphérique sans fil",
                    "HU": "Vezeték nélküli eszköz",
                    "IT": "Dispositivo wireless",
                    "JP": "無線デバイス",
                    "KR": "무선 장치",
                    "MS": "Wireless Device",
                    "NL": "Draadloos apparaat",
                    "NO": "Trådløs enhet",
                    "PL": "Urządzenie bezprzewodowe",
                    "RU": "Беспроводное устройство",
                    "SV": "Trådlös enhet",
                    "TH": "อุปกรณ์แบบไร้สาย",
                    "TR": "Kablosuz Cihaz",
                    "UK": "Бездротовий пристрій",
                    "RO": "Dispozitiv wireless",
                    "SL": "Brezžična naprava"
                },
                {
                    "category": "electronic_product",
                    "ui-sort": 39,
                    "type": [
                        "137"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJOYXZcaWNfQWlCb2FyZF9zZXR0aW5ncyI+PHBhdGggaWQ9IlVuaW9uX19fX18wXzBfSVNKS1NTVkJMUSIgZmlsbD0iIzE4MTgxOCIgZmlsbC1ydWxlPSJldmVub2RkIiBkPSJNMTQuNSA0SDEzLjVWM0gxNC41VjRaTTE4LjUgNy4yMDVMMTYuNzk1IDUuNUgxNi43OUg3LjIwNUw1LjUgNy4yMDVWMTYuNzlMNy4yMDUgMTguNDk1SDE2Ljc5NUwxOC41IDE2Ljc5VjcuMjA1Wk03IDVIMTYuOTk1SDE3TDE5IDdWMTdMMTcgMTlIN0w1IDE3VjdMNyA1Wk0xNS41IDRIMTYuNUgxNy43OTI5TDIwIDYuMjA3MTFWOC41SDIxVjZWNS43OTI4OUwyMC44NTM2IDUuNjQ2NDVMMTguMzUzNiAzLjE0NjQ1TDE4LjIwNzEgM0gxOEgxNi41SDE1LjVWNFpNMTIuNSA0SDExLjVWM0gxMi41VjRaTTkuNSA0SDEwLjVWM0g5LjVWNFpNMTQuNSAyMUgxMy41VjIwSDE0LjVWMjFaTTExLjUgMjFIMTIuNVYyMEgxMS41VjIxWk0xMC41IDIxSDkuNVYyMEgxMC41VjIxWk0yMCAxMy41VjE0LjVIMjFWMTMuNUgyMFpNMjAgMTIuNVYxMS41SDIxVjEyLjVIMjBaTTIwIDkuNVYxMC41SDIxVjkuNUgyMFpNMyAxNC41VjEzLjVINFYxNC41SDNaTTMgMTEuNVYxMi41SDRWMTEuNUgzWk0zIDEwLjVWOS41SDRWMTAuNUgzWk02IDNINS43OTI4OUw1LjY0NjQ1IDMuMTQ2NDVMMy4xNDY0NSA1LjY0NjQ1TDMgNS43OTI4OVY2VjguNUg0VjYuMjA3MTFMNi4yMDcxMSA0SDguNVYzSDZaTTIwIDE3Ljc5MjlWMTUuNUgyMVYxOFYxOC4yMDcxTDIwLjg1MzYgMTguMzUzNkwxOC4zNTM2IDIwLjg1MzZMMTguMjA3MSAyMUgxOEgxNS41VjIwSDE3Ljc5MjlMMjAgMTcuNzkyOVpNMyAxNS41VjE4VjE4LjIwNzFMMy4xNDY0NSAxOC4zNTM2TDUuNjQ2NDUgMjAuODUzNkw1Ljc5Mjg5IDIxSDZIOC41VjIwSDYuMjA3MTFMNCAxNy43OTI5VjE1LjVIM1oiIGNsaXAtcnVsZT0iZXZlbm9kZCIvPjxwYXRoIGlkPSJVbmlvbl9fX19fMF8xX1lIQk1USFZTTVgiIGZpbGw9ImJsYWNrIiBkPSJNMTAuOTcwNyA4LjA2NTQzTDE0LjEyODkgMTYuNjY4TDE0LjE3ODcgMTYuODAyN0gxMi44NzNMMTIuODQ5NiAxNi43MzU0TDEyLjAwODggMTQuMzY2Mkg4LjY3MDlMNy44MzAwOCAxNi43MzU0TDcuODA2NjQgMTYuODAyN0g2LjVMNi41NDk4IDE2LjY2OEw5LjcwODAxIDguMDY1NDNMOS43MzI0MiA4SDEwLjk0NzNMMTAuOTcwNyA4LjA2NTQzWk0xNi41MTY2IDE2LjgwMjdIMTUuMjc1NFY4SDE2LjUxNjZWMTYuODAyN1pNOS4wNzAzMSAxMy4yNDIySDExLjYwOTRMMTAuMzM5OCA5LjY2NDA2TDkuMDcwMzEgMTMuMjQyMloiLz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "AiBoard",
                    "TW": "AiBoard",
                    "CN": "AiBoard",
                    "BR": "AiBoard",
                    "CZ": "AiBoard",
                    "DA": "AiBoard",
                    "DE": "AiBoard",
                    "ES": "AiBoard",
                    "FI": "AiBoard",
                    "FR": "AiBoard",
                    "HU": "AiBoard",
                    "IT": "AiBoard",
                    "JP": "AiBoard",
                    "KR": "AiBoard",
                    "MS": "AiBoard",
                    "NL": "AiBoard",
                    "NO": "AiBoard",
                    "PL": "AiBoard",
                    "RU": "AiBoard",
                    "SV": "AiBoard",
                    "TH": "AiBoard",
                    "TR": "AiBoard",
                    "UK": "AiBoard",
                    "RO": "AiBoard",
                    "SL": "AiBoard"
                },
                {
                    "category": "media_and_entertainment",
                    "ui-sort": 1,
                    "type": [
                        "43"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJNZWRpYSBEZXZpY2VzL2dvb2dsZSBob21lIj48ZyBpZD0iR3JvdXAgNDQyOCI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8wX0JVS01UQVBMVlciIGZpbGw9ImJsYWNrIiBkPSJNMTUuNDU1MiAyLjkyNDA4QzE1LjY4OTUgMy4xNTgzOSAxNi4wNjk0IDMuMTU4MzkgMTYuMzAzOCAyLjkyNDA4QzE2LjUzODEgMi42ODk3NiAxNi41MzgxIDIuMzA5ODcgMTYuMzAzOCAyLjA3NTU1TDE1LjQ1NTIgMi45MjQwOFpNMTAuMDIzOSAzLjg4NTk5QzkuNzg5NiA0LjEyMDMgOS43ODk2IDQuNTAwMiAxMC4wMjM5IDQuNzM0NTJDMTAuMjU4MiA0Ljk2ODgzIDEwLjYzODEgNC45Njg4MyAxMC44NzI0IDQuNzM0NTJMMTAuMDIzOSAzLjg4NTk5Wk04LjIxMzQ4IDIuMDc1NTVDNy45NzkxNyAyLjMwOTg3IDcuOTc5MTcgMi42ODk3NiA4LjIxMzQ4IDIuOTI0MDhDOC40NDc4IDMuMTU4MzkgOC44Mjc3IDMuMTU4MzkgOS4wNjIwMSAyLjkyNDA4TDguMjEzNDggMi4wNzU1NVpNMTMuNjQ0OCA0LjczNDUyQzEzLjg3OTEgNC45Njg4MyAxNC4yNTkgNC45Njg4MyAxNC40OTMzIDQuNzM0NTJDMTQuNzI3NiA0LjUwMDIgMTQuNzI3NiA0LjEyMDMgMTQuNDkzMyAzLjg4NTk5TDEzLjY0NDggNC43MzQ1MlpNOS40MTM3OSAyM0w5LjQxMzc5IDIzLjZIOS40MTM3OVYyM1pNNiAxOS45NjU1TDUuNDAzMTIgMTkuOTA0NEw1LjQgMTkuOTM0OUw1LjQgMTkuOTY1NUw2IDE5Ljk2NTVaTTE4LjEzNzkgMTkuNTg2MkwxOC43Mzc5IDE5LjU4NjJMMTguNzM3OSAxOS41NjA0TDE4LjczNTcgMTkuNTM0NkwxOC4xMzc5IDE5LjU4NjJaTTE0LjcyNDEgMjNMMTQuNzI0MSAyMi40SDE0LjcyNDFWMjNaTTYuNDQ2NjYgMTUuNjAzNEw1Ljg1MzE0IDE1LjUxNTVDNS44NTE4MiAxNS41MjQ0IDUuODUwNyAxNS41MzM0IDUuODQ5NzkgMTUuNTQyM0w2LjQ0NjY2IDE1LjYwMzRaTTE3Ljc5NDEgMTUuNjAzNEwxOC4zOTE5IDE1LjU1MThDMTguMzkxMyAxNS41NDU0IDE4LjM5MDcgMTUuNTM5IDE4LjM4OTkgMTUuNTMyNkwxNy43OTQxIDE1LjYwMzRaTTEyLjI1ODYgMS42QzEzLjUwNzEgMS42IDE0LjYzNjUgMi4xMDUzNCAxNS40NTUyIDIuOTI0MDhMMTYuMzAzOCAyLjA3NTU1QzE1LjI2OTIgMS4wNDA5NyAxMy44MzgyIDAuNCAxMi4yNTg2IDAuNFYxLjZaTTEwLjg3MjQgNC43MzQ1MkMxMS4yMjc5IDQuMzc5MTEgMTEuNzE3MSA0LjE2MDM0IDEyLjI1ODYgNC4xNjAzNFYyLjk2MDM0QzExLjM4NjEgMi45NjAzNCAxMC41OTUyIDMuMzE0NzQgMTAuMDIzOSAzLjg4NTk5TDEwLjg3MjQgNC43MzQ1MlpNOS4wNjIwMSAyLjkyNDA4QzkuODgwNzUgMi4xMDUzNCAxMS4wMTAxIDEuNiAxMi4yNTg2IDEuNlYwLjRDMTAuNjc5MSAwLjQgOS4yNDgwNyAxLjA0MDk3IDguMjEzNDggMi4wNzU1NUw5LjA2MjAxIDIuOTI0MDhaTTEyLjI1ODYgNC4xNjAzNEMxMi44MDAxIDQuMTYwMzQgMTMuMjg5NCA0LjM3OTExIDEzLjY0NDggNC43MzQ1MkwxNC40OTMzIDMuODg1OTlDMTMuOTIyMSAzLjMxNDc0IDEzLjEzMTIgMi45NjAzNCAxMi4yNTg2IDIuOTYwMzRWNC4xNjAzNFpNMTQuNzI0MSAyMi40SDkuNDEzNzlWMjMuNkgxNC43MjQxVjIyLjRaTTkuNDEzNzkgMjIuNEM4LjU4OTA1IDIyLjQgNy44Nzk5NSAyMi4xNDg3IDcuMzg3NTMgMjEuNzMxOEM2LjkwNDY0IDIxLjMyMzEgNi42IDIwLjczMiA2LjYgMTkuOTY1NUw1LjQgMTkuOTY1NUM1LjQgMjEuMDg0NCA1Ljg1OTU2IDIyLjAxMDYgNi42MTIyMiAyMi42NDc3QzcuMzU1MzYgMjMuMjc2OCA4LjM1MzE1IDIzLjYgOS40MTM3OSAyMy42TDkuNDEzNzkgMjIuNFpNMTcuNTM3OSAxOS41ODYyQzE3LjUzNzkgMjEuMTQwMiAxNi4yNzgyIDIyLjQgMTQuNzI0MSAyMi40TDE0LjcyNDEgMjMuNkMxNi45NDA5IDIzLjYgMTguNzM3OSAyMS44MDMgMTguNzM3OSAxOS41ODYyTDE3LjUzNzkgMTkuNTg2MlpNNi41OTY4OCAyMC4wMjY2TDcuMDQzNTQgMTUuNjY0Nkw1Ljg0OTc5IDE1LjU0MjNMNS40MDMxMiAxOS45MDQ0TDYuNTk2ODggMjAuMDI2NlpNNy4wNDAxOSAxNS42OTE0TDguMDIzNTIgOS4wNTM0NEw2LjgzNjQ4IDguODc3NTlMNS44NTMxNCAxNS41MTU1TDcuMDQwMTkgMTUuNjkxNFpNMTYuMTA0MiA2LjQ3NjA0TDE3LjE5ODMgMTUuNjc0M0wxOC4zODk5IDE1LjUzMjZMMTcuMjk1OCA2LjMzNDNMMTYuMTA0MiA2LjQ3NjA0Wk0xNy4xOTYzIDE1LjY1NTFMMTcuNTQwMiAxOS42Mzc4TDE4LjczNTcgMTkuNTM0NkwxOC4zOTE5IDE1LjU1MThMMTcuMTk2MyAxNS42NTUxWk02LjQ0NjY2IDE2LjIwMzRIMTcuNzk0MVYxNS4wMDM0SDYuNDQ2NjZWMTYuMjAzNFoiLz48cGF0aCBpZD0iRWxsaXBzZSAxMTY2X19fX18wXzFfRE1ZQ0lTWEFHWiIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xNi43MjgzIDYuMzc2NzRDMTYuNzI4MyA2LjM3NjczIDE2LjcyODMgNi4zNzcwNSAxNi43MjgzIDYuMzc3NjlMMTYuNzI4MyA2LjM3Njc0Wk0xNi41MDQ5IDYuMjU5ODhDMTYuNjUzMyA2LjMxODc2IDE2LjcwODcgNi4zNjg4MSAxNi43MjY4IDYuMzg5ODNDMTYuNzIxNiA2LjQxNzExIDE2LjY5ODcgNi40ODgxMyAxNi41OTk2IDYuNjEzMzNDMTYuNDQ1MiA2LjgwODQgMTYuMTcwNSA3LjA0NjcxIDE1Ljc2NTMgNy4zMDQ0OEMxNC45NjExIDcuODE2MDQgMTMuNzYwNCA4LjMzMDYyIDEyLjM1NTQgOC43MDcxQzEwLjk1MDMgOS4wODM1OSA5LjY1MzIgOS4yMzgyOSA4LjcwMTAxIDkuMTk3MzRDOC4yMjEyMSA5LjE3NjcxIDcuODY0MDkgOS4xMDc3MSA3LjYzMjg1IDkuMDE1OTdDNy40ODQ0MyA4Ljk1NzA5IDcuNDI5MDcgOC45MDcwNCA3LjQxMDkzIDguODg2MDJDNy40MTYxMyA4Ljg1ODc0IDcuNDM5MDUgOC43ODc3MiA3LjUzODE0IDguNjYyNTJDNy42OTI1MyA4LjQ2NzQ1IDcuOTY3MzEgOC4yMjkxMyA4LjM3MjUxIDcuOTcxMzdDOS4xNzY2NiA3LjQ1OTgxIDEwLjM3NzMgNi45NDUyMyAxMS43ODI0IDYuNTY4NzRDMTMuMTg3NSA2LjE5MjI2IDE0LjQ4NDYgNi4wMzc1NiAxNS40MzY4IDYuMDc4NTFDMTUuOTE2NiA2LjA5OTE0IDE2LjI3MzcgNi4xNjgxNCAxNi41MDQ5IDYuMjU5ODhaTTcuNDAzMTIgOC44NzU0MUM3LjQwMzEzIDguODc1NDEgNy40MDMzIDguODc1NjggNy40MDM1NyA4Ljg3NjI1TDcuNDAzMTIgOC44NzU0MVpNNy40MDk0NyA4Ljg5OTExQzcuNDA5NDYgOC44OTkxMiA3LjQwOTQ0IDguODk4OCA3LjQwOTQ0IDguODk4MTZMNy40MDk0NyA4Ljg5OTExWk0xNi43MzQyIDYuMzk5NkMxNi43MzQ1IDYuNDAwMTUgMTYuNzM0NyA2LjQwMDQ0IDE2LjczNDYgNi40MDA0NEwxNi43MzQyIDYuMzk5NloiLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Google Home",
                    "TW": "Google Home",
                    "CN": "Google Home",
                    "BR": "Google Home",
                    "CZ": "Google Home",
                    "DA": "Google Home",
                    "DE": "Google Home",
                    "ES": "Google Home",
                    "FI": "Google Home",
                    "FR": "Google Home",
                    "HU": "Google Home",
                    "IT": "Google Home",
                    "JP": "Google Home",
                    "KR": "Google Home",
                    "MS": "Google Home",
                    "NL": "Google Home",
                    "NO": "Google Home",
                    "PL": "Google Home",
                    "RU": "Google Home",
                    "SV": "Google Home",
                    "TH": "Google Home",
                    "TR": "Google Home",
                    "UK": "Google Home",
                    "RO": "Google Home",
                    "SL": "Google Home"
                },
                {
                    "category": "media_and_entertainment",
                    "ui-sort": 2,
                    "type": [
                        "44"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJNZWRpYSBEZXZpY2VzL2FtYXpvbiBhbGV4YSI+PGcgaWQ9Ikdyb3VwIDQ1MzciPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMF9XSVJOTU9DSE9QIiBmaWxsPSJibGFjayIgZD0iTTE1LjgxODQgNC4xODE2MkMxNi4wNTI3IDQuNDE1OTQgMTYuNDMyNiA0LjQxNTk0IDE2LjY2NjkgNC4xODE2MkMxNi45MDEyIDMuOTQ3MzEgMTYuOTAxMiAzLjU2NzQxIDE2LjY2NjkgMy4zMzMxTDE1LjgxODQgNC4xODE2MlpNOS40NTQ0MiA1LjQ1NDQyQzkuMjIwMSA1LjY4ODczIDkuMjIwMSA2LjA2ODYzIDkuNDU0NDIgNi4zMDI5NEM5LjY4ODczIDYuNTM3MjYgMTAuMDY4NiA2LjUzNzI2IDEwLjMwMjkgNi4zMDI5NEw5LjQ1NDQyIDUuNDU0NDJaTTcuMzMzMSAzLjMzMzFDNy4wOTg3OCAzLjU2NzQxIDcuMDk4NzggMy45NDczMSA3LjMzMzEgNC4xODE2MkM3LjU2NzQxIDQuNDE1OTQgNy45NDczMSA0LjQxNTk0IDguMTgxNjIgNC4xODE2Mkw3LjMzMzEgMy4zMzMxWk0xMy42OTcxIDYuMzAyOTRDMTMuOTMxNCA2LjUzNzI2IDE0LjMxMTMgNi41MzcyNiAxNC41NDU2IDYuMzAyOTRDMTQuNzc5OSA2LjA2ODYzIDE0Ljc3OTkgNS42ODg3MyAxNC41NDU2IDUuNDU0NDJMMTMuNjk3MSA2LjMwMjk0Wk0xMCAyMkwxMCAyMi42SDEwVjIyWk02IDE4TDYuNiAxOEw2IDE4Wk0xOCAxOEwxNy40IDE4VjE4TDE4IDE4Wk0xNCAyMkwxNCAyMS40SDE0VjIyWk0xMiAyLjZDMTMuNDkxMyAyLjYgMTQuODQwNSAzLjIwMzc2IDE1LjgxODQgNC4xODE2MkwxNi42NjY5IDMuMzMzMUMxNS40NzMyIDIuMTM5MzkgMTMuODIyNCAxLjQgMTIgMS40VjIuNlpNMTAuMzAyOSA2LjMwMjk0QzEwLjczNzkgNS44Njc5NyAxMS4zMzcxIDUuNiAxMiA1LjZWNC40QzExLjAwNjEgNC40IDEwLjEwNTIgNC44MDM2IDkuNDU0NDIgNS40NTQ0MkwxMC4zMDI5IDYuMzAyOTRaTTguMTgxNjIgNC4xODE2MkM5LjE1OTQ5IDMuMjAzNzYgMTAuNTA4NyAyLjYgMTIgMi42VjEuNEMxMC4xNzc2IDEuNCA4LjUyNjggMi4xMzkzOSA3LjMzMzEgMy4zMzMxTDguMTgxNjIgNC4xODE2MlpNMTIgNS42QzEyLjY2MjkgNS42IDEzLjI2MjEgNS44Njc5NyAxMy42OTcxIDYuMzAyOTRMMTQuNTQ1NiA1LjQ1NDQyQzEzLjg5NDggNC44MDM2IDEyLjk5MzkgNC40IDEyIDQuNFY1LjZaTTE3LjQgMTFMMTcuNCAxOEwxOC42IDE4TDE4LjYgMTFMMTcuNCAxMVpNMTQgMjEuNEgxMFYyMi42SDE0VjIxLjRaTTYuNiAxOEw2LjYgMTFMNS40IDExTDUuNCAxOEw2LjYgMThaTTEwIDIxLjRDOC4xMjIyMyAyMS40IDYuNiAxOS44Nzc4IDYuNiAxOEw1LjQgMThDNS40IDIwLjU0MDUgNy40NTk0OSAyMi42IDEwIDIyLjZMMTAgMjEuNFpNMTcuNCAxOEMxNy40IDE5Ljg3NzggMTUuODc3OCAyMS40IDE0IDIxLjRMMTQgMjIuNkMxNi41NDA1IDIyLjYgMTguNiAyMC41NDA1IDE4LjYgMThMMTcuNCAxOFoiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzFfTEZRSkJYSk5FQyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE0IDhDMTYuMjA5MSA4IDE4IDguNzkwODYgMTggMTFDMTggMTMuMjA5MSAxNi4yMDkxIDE0IDE0IDE0QzE0IDE0IDE0IDE0IDE0IDE0QzE0IDE0IDEyLjIwOTEgMTQgMTAgMTRDNy43OTA4NiAxNCA2IDEzLjIwOTEgNiAxMUM2IDguNzkwODYgNy43OTA4NiA4IDEwIDhMMTQgOFoiLz48cGF0aCBpZD0iVW5pb25fX19fXzBfMl9KWlFLTU1ORlJSIiBmaWxsPSJibGFjayIgZmlsbC1ydWxlPSJldmVub2RkIiBkPSJNMTIgOS45MTQyNEMxMi40OTk3IDkuOTE0MjQgMTIuOTA0NyA5Ljc1MjIxIDEyLjkwNDcgOS41NTIzM0MxMi45MDQ3IDkuMzUyNDYgMTIuNDk5NyA5LjE5MDQzIDEyIDkuMTkwNDNDMTEuNTAwMyA5LjE5MDQzIDExLjA5NTIgOS4zNTI0NiAxMS4wOTUyIDkuNTUyMzNDMTEuMDk1MiA5Ljc1MjIxIDExLjUwMDMgOS45MTQyNCAxMiA5LjkxNDI0Wk05LjEwNTQ1IDExLjM2MkM5LjYwNTEzIDExLjM2MiAxMC4wMTAyIDExLjIgMTAuMDEwMiAxMS4wMDAxQzEwLjAxMDIgMTAuODAwMiA5LjYwNTEzIDEwLjYzODIgOS4xMDU0NSAxMC42MzgyQzguNjA1NzYgMTAuNjM4MiA4LjIwMDY4IDEwLjgwMDIgOC4yMDA2OCAxMS4wMDAxQzguMjAwNjggMTEuMiA4LjYwNTc2IDExLjM2MiA5LjEwNTQ1IDExLjM2MlpNMTIuOTA0NyAxMi40NDc2QzEyLjkwNDcgMTIuNjQ3NSAxMi40OTk3IDEyLjgwOTUgMTIgMTIuODA5NUMxMS41MDAzIDEyLjgwOTUgMTEuMDk1MiAxMi42NDc1IDExLjA5NTIgMTIuNDQ3NkMxMS4wOTUyIDEyLjI0NzcgMTEuNTAwMyAxMi4wODU3IDEyIDEyLjA4NTdDMTIuNDk5NyAxMi4wODU3IDEyLjkwNDcgMTIuMjQ3NyAxMi45MDQ3IDEyLjQ0NzZaTTE0Ljg5NjUgMTEuMzYyQzE1LjM5NjEgMTEuMzYyIDE1LjgwMTIgMTEuMiAxNS44MDEyIDExLjAwMDFDMTUuODAxMiAxMC44MDAyIDE1LjM5NjEgMTAuNjM4MiAxNC44OTY1IDEwLjYzODJDMTQuMzk2OCAxMC42MzgyIDEzLjk5MTcgMTAuODAwMiAxMy45OTE3IDExLjAwMDFDMTMuOTkxNyAxMS4yIDE0LjM5NjggMTEuMzYyIDE0Ljg5NjUgMTEuMzYyWiIgY2xpcC1ydWxlPSJldmVub2RkIi8+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "Amazon Alexa",
                    "TW": "Amazon Alexa",
                    "CN": "Amazon Alexa",
                    "BR": "Amazon Alexa",
                    "CZ": "Amazon Alexa",
                    "DA": "Amazon Alexa",
                    "DE": "Amazon Alexa",
                    "ES": "Amazon Alexa",
                    "FI": "Amazon Alexa",
                    "FR": "Amazon Alexa",
                    "HU": "Amazon Alexa",
                    "IT": "Amazon Alexa",
                    "JP": "Amazon Alexa",
                    "KR": "Amazon Alexa",
                    "MS": "Amazon Alexa",
                    "NL": "Amazon Alexa",
                    "NO": "Amazon Alexa",
                    "PL": "Amazon Alexa",
                    "RU": "Amazon Alexa",
                    "SV": "Amazon Alexa",
                    "TH": "Amazon Alexa",
                    "TR": "Amazon Alexa",
                    "UK": "Amazon Alexa",
                    "RO": "Amazon Alexa",
                    "SL": "Amazon Alexa"
                },
                {
                    "category": "media_and_entertainment",
                    "ui-sort": 3,
                    "type": [
                        "45"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJNZWRpYSBEZXZpY2VzL2FwcGxlIGhvbWVQb2QiPjxnIGlkPSJHcm91cCA0NTM4Ij48cGF0aCBpZD0iVmVjdG9yX19fX18wXzBfQk1QQlpERVdHVyIgc3Ryb2tlPSIjMjgzMDNGIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTYuMjQyNiAzLjc1NzM2QzE1LjE1NjkgMi42NzE1NyAxMy42NTY5IDIgMTIgMkMxMC4zNDMxIDIgOC44NDMxNSAyLjY3MTU3IDcuNzU3MzYgMy43NTczNk05Ljg3ODY4IDUuODc4NjhDMTAuNDIxNiA1LjMzNTc5IDExLjE3MTYgNSAxMiA1QzEyLjgyODQgNSAxMy41Nzg0IDUuMzM1NzkgMTQuMTIxMyA1Ljg3ODY4TTYgMTJMNiAxOEM2IDIwLjIwOTEgNy43OTA4NiAyMiAxMCAyMkgxNEMxNi4yMDkxIDIyIDE4IDIwLjIwOTEgMTggMThMMTggMTJDMTggOS43OTA4NiAxNi4yMDkxIDggMTQgOEwxMCA4QzcuNzkwODYgOCA2IDkuNzkwODYgNiAxMloiLz48ZWxsaXBzZSBpZD0iRWxsaXBzZSA1NjFfX19fXzBfMV9CSldJTUNTRFRQIiBjeD0iMTIiIGN5PSI5LjUiIHN0cm9rZT0iIzI4MzAzRiIgc3Ryb2tlLXdpZHRoPSIxLjIiIHJ4PSI0IiByeT0iMS41Ii8+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "Apple HomePod",
                    "TW": "Apple HomePod",
                    "CN": "Apple HomePod",
                    "BR": "Apple HomePod",
                    "CZ": "Apple HomePod",
                    "DA": "Apple HomePod",
                    "DE": "Apple HomePod",
                    "ES": "Apple HomePod",
                    "FI": "Apple HomePod",
                    "FR": "Apple HomePod",
                    "HU": "Apple HomePod",
                    "IT": "Apple HomePod",
                    "JP": "Apple HomePod",
                    "KR": "Apple HomePod",
                    "MS": "Apple HomePod",
                    "NL": "Apple HomePod",
                    "NO": "Apple HomePod",
                    "PL": "Apple HomePod",
                    "RU": "Apple HomePod",
                    "SV": "Apple HomePod",
                    "TH": "Apple HomePod",
                    "TR": "Apple HomePod",
                    "UK": "Apple HomePod",
                    "RO": "Apple HomePod",
                    "SL": "Apple HomePod"
                },
                {
                    "category": "media_and_entertainment",
                    "ui-sort": 4,
                    "type": [
                        "11"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJNZWRpYSBEZXZpY2VzL2FwcGxlIFRWIj48ZyBpZD0iR3JvdXAgNDQwOCI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8wX0hOSENLV0tNQlMiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0yIDZDMiAzLjc5MDg2IDMuNzkwODYgMiA2IDJIMThDMjAuMjA5MSAyIDIyIDMuNzkwODYgMjIgNlYxOEMyMiAyMC4yMDkxIDIwLjIwOTEgMjIgMTggMjJINkMzLjc5MDg2IDIyIDIgMjAuMjA5MSAyIDE4VjZaIi8+PGcgaWQ9IkFwcGxlIj48ZyBpZD0iR3JvdXBfMiI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8xX1RFS0pXU0VHWUIiIGZpbGw9ImJsYWNrIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNOS4zOTIyNyAxMC42NDY0QzkuNTI5MjEgMTAuNDg0OCA5LjY4OTQ3IDEwLjM0MzcgOS44NjYzMyAxMC4yMTkzQzkuNjE3NjEgMTAuMDYxMSA5LjMxODE5IDkuOTcwNzQgOC45NjgwOSA5Ljk0MzAzQzguNjAxMTYgOS45MTMxMyA4LjIxNTE0IDEwLjA1MTYgNy45MjUzNCAxMC4xNTU2QzcuNzYxNDIgMTAuMjE0NCA3LjYyODI5IDEwLjI2MjIgNy41NDY4MSAxMC4yNjIyQzcuNDQ1MSAxMC4yNjIyIDcuMjg3NTggMTAuMjA3MSA3LjEwNyAxMC4xNDRDNi44NjQ0OSAxMC4wNTkzIDYuNTgwMzkgOS45NjAwNSA2LjMzNDA0IDkuOTYwMDVNOS4zOTIyNyAxMC42NDY0QzkuMjYxOSAxMC41OTE1IDkuMTA2OTkgMTAuNTU1OSA4LjkyMDc1IDEwLjU0MTJMOC45MTkzNiAxMC41NDExQzguNzYwODQgMTAuNTI4MSA4LjU2MDA0IDEwLjU3MjUgOC4zMTc0IDEwLjY1MzlDOC4yNTk5MSAxMC42NzMyIDguMjAzODQgMTAuNjkzMSA4LjE0NjY0IDEwLjcxMzdDOC4xNDA0NSAxMC43MTU5IDguMTM0MiAxMC43MTgxIDguMTI3ODkgMTAuNzIwNEM4LjA3ODIgMTAuNzM4MiA4LjAyNTM5IDEwLjc1NzIgNy45NzYwNyAxMC43NzM5TDcuOTcyNjYgMTAuNzc1MUM3Ljg4Nzk1IDEwLjgwMzggNy43MTYwNSAxMC44NjIyIDcuNTQ2ODEgMTAuODYyMkM3LjM4NzY3IDEwLjg2MjIgNy4yMzE5NyAxMC44MTY3IDcuMTM5NDIgMTAuNzg4QzcuMDY0IDEwLjc2NDYgNi45NzY3IDEwLjczNDEgNi44OTgxMiAxMC43MDY3QzYuODY5NjYgMTAuNjk2NyA2Ljg0MjM0IDEwLjY4NzIgNi44MTcxNCAxMC42Nzg1QzYuNTkzMDUgMTAuNjAxNSA2LjQ0MDIzIDEwLjU2MTQgNi4zMzkwMSAxMC41NjAxTTkuMzkyMjcgMTAuNjQ2NEM5LjMyMDA5IDEwLjczMTUgOS4yNTQzOCAxMC44MjI0IDkuMTk2MTQgMTAuOTE5NEM5LjAwMDQ4IDExLjI0NTUgOC45MDg1MSAxMS42MDk3IDguOTA4NTEgMTIuMDAyNkM4LjkwODUxIDExLjk5NjYgOC45MDQzOSAxMi4xMDIzIDguOTIwNjkgMTIuMjQ1MUM4LjkzNjgzIDEyLjM4NjQgOC45NzMxNyAxMi41NzQ3IDkuMDU0MjYgMTIuNzgyN0M5LjE3NDc4IDEzLjA5MiA5LjM4OTM0IDEzLjQzMDUgOS43NTMxNyAxMy43MTU4QzkuNjYyMTUgMTMuOTAzOSA5LjU0NjQyIDE0LjEwOTEgOS40MTM2NSAxNC4yOTM1QzkuMjkwMjcgMTQuNDY0OCA5LjE2ODAxIDE0LjU5NTggOS4wNTcwNSAxNC42NzkyQzguOTQ1NzcgMTQuNzYyOCA4Ljg4MTIxIDE0Ljc3MjggOC44NTc0NSAxNC43NzI4QzguNzY1NjcgMTQuNzcyOCA4LjY5Mzk0IDE0Ljc0ODQgOC40OTUwNCAxNC42Njk5TDguNDk0OTkgMTQuNjY5OUM4LjMwNDYxIDE0LjU5NDkgOC4wMjE5NyAxNC40ODM1IDcuNjQ0NjggMTQuNDgzNUM3LjI2NDAzIDE0LjQ4MzUgNi45NzM5OCAxNC41OTEzIDYuNzc0MTkgMTQuNjY4NUM2LjU1NDMxIDE0Ljc1MzQgNi40OTY4MiAxNC43NzI4IDYuNDMxOTEgMTQuNzcyOEM2LjQyNDE0IDE0Ljc3MjggNi40MTYzNyAxNC43NzMgNi40MDg2IDE0Ljc3MzNMNi40MDY4MSAxNC43NzMyQzYuNDA1NjMgMTQuNzczMSA2LjQwMzYyIDE0Ljc3MjkgNi40MDA2OCAxNC43NzIzQzYuMzk0NzkgMTQuNzcxMiA2LjM4NDczIDE0Ljc2ODcgNi4zNzAyIDE0Ljc2MjhDNi4zNDAwMSAxNC43NTA4IDYuMjk2MSAxNC43MjY4IDYuMjM5NTQgMTQuNjgyOUM2LjEyMzI4IDE0LjU5MjYgNS45OTI0OCAxNC40NDg1IDUuODU4NyAxNC4yNjAzQzUuNTkwNzkgMTMuODgzNCA1LjM3OTgzIDEzLjQyODQgNS4yOTE0IDEzLjE3NzZMNS4yOTE0MiAxMy4xNzc2TDUuMjg5NzIgMTMuMTcyOUM1LjE1MzY4IDEyLjc5NyA1LjEgMTIuNDM2OSA1LjEgMTIuMDgzNUM1LjEgMTEuNTEyMSA1LjI4NTMzIDExLjE0MyA1LjUwODc2IDEwLjkxNjhDNS43MzcwNyAxMC42ODU3IDYuMDQwNTggMTAuNTY3NCA2LjMzOTAxIDEwLjU2MDFNOS4zOTIyNyAxMC42NDY0TDYuMzM0MDQgOS45NjAwNU02LjMzOTAxIDEwLjU2MDFDNi4zNDExNiAxMC41NiA2LjM0MzMxIDEwLjU2IDYuMzQ1NDcgMTAuNTU5OUw2LjMzNDA0IDkuOTYwMDVNNi4zMzkwMSAxMC41NjAxQzYuMzM3MzQgMTAuNTYwMSA2LjMzNTY4IDEwLjU2MDEgNi4zMzQwNCAxMC41NjAxVjkuOTYwMDUiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzJfWFhNVVNRRFdXQyIgZmlsbD0iYmxhY2siIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik04LjEyNTk1IDguMjU0NTdDOC4xNDE0MiA4LjIzODI0IDguMTYxMjkgOC4yMjA0NyA4LjE4NDk2IDguMjAyNzJDOC4xNjExNyA4LjI0OTY2IDguMTM0MzkgOC4yODU3MiA4LjExMDIxIDguMzA5OUw4LjExMDIgOC4zMDk4OEw4LjEwNjg4IDguMzEzMjVDOC4wOTQyNyA4LjMyNjA3IDguMDc3MTQgOC4zNDA2NCA4LjA1NTgzIDguMzU1NDdDOC4wNzg1NiA4LjMxMzMxIDguMTAzMzQgOC4yNzkwNyA4LjEyNTk1IDguMjU0NTdaIi8+PC9nPjwvZz48L2c+PHBhdGggaWQ9InR2X19fX18wXzNfT0pUSk9ETFZPUSIgZmlsbD0iYmxhY2siIGQ9Ik0xMS44MjAzIDguMzEwNTVIMTIuNzA5VjkuNzcwNTFIMTMuNTQzOVYxMC40ODgzSDEyLjcwOVYxMy45MDE0QzEyLjcwOSAxNC4wODM3IDEyLjc3MDggMTQuMjA1NyAxMi44OTQ1IDE0LjI2NzZDMTIuOTYyOSAxNC4zMDM0IDEzLjA3NjggMTQuMzIxMyAxMy4yMzYzIDE0LjMyMTNDMTMuMjc4NiAxNC4zMjEzIDEzLjMyNDIgMTQuMzIxMyAxMy4zNzMgMTQuMzIxM0MxMy40MjE5IDE0LjMxOCAxMy40Nzg4IDE0LjMxMzIgMTMuNTQzOSAxNC4zMDY2VjE1QzEzLjQ0MyAxNS4wMjkzIDEzLjMzNzIgMTUuMDUwNSAxMy4yMjY2IDE1LjA2MzVDMTMuMTE5MSAxNS4wNzY1IDEzLjAwMiAxNS4wODMgMTIuODc1IDE1LjA4M0MxMi40NjQ4IDE1LjA4MyAxMi4xODY1IDE0Ljk3ODggMTIuMDQgMTQuNzcwNUMxMS44OTM2IDE0LjU1ODkgMTEuODIwMyAxNC4yODU1IDExLjgyMDMgMTMuOTUwMlYxMC40ODgzSDExLjExMjNWOS43NzA1MUgxMS44MjAzVjguMzEwNTVaTTE1LjQ1NzQgOS43NzA1MUwxNi44NTM5IDE0LjAyODNMMTguMzEzOSA5Ljc3MDUxSDE5LjI3NThMMTcuMzAzMSAxNUgxNi4zNjU2TDE0LjQzNjkgOS43NzA1MUgxNS40NTc0WiIvPjwvZz48L2c+PC9zdmc+",
                    "EN": "Apple TV",
                    "TW": "Apple TV",
                    "CN": "Apple TV",
                    "BR": "Apple TV",
                    "CZ": "Apple TV",
                    "DA": "Apple TV",
                    "DE": "Apple TV",
                    "ES": "Apple TV",
                    "FI": "Apple TV",
                    "FR": "Apple TV",
                    "HU": "Apple TV",
                    "IT": "Apple TV",
                    "JP": "Apple TV",
                    "KR": "Apple TV",
                    "MS": "Apple TV",
                    "NL": "Apple TV",
                    "NO": "Apple TV",
                    "PL": "Apple TV",
                    "RU": "Apple TV",
                    "SV": "Apple TV",
                    "TH": "Apple TV",
                    "TR": "Apple TV",
                    "UK": "Apple TV",
                    "RO": "Apple TV",
                    "SL": "Apple TV"
                },
                {
                    "category": "media_and_entertainment",
                    "ui-sort": 5,
                    "type": [
                        "46"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJNZWRpYSBEZXZpY2VzL0RMSU5BIj48cGF0aCBpZD0iVmVjdG9yX19fX18wXzBfWU9IWVhPU09MRSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuNSIgZD0iTTggMTFIMTBNMTIgMTFIMTZNMTQgMTZIMTZNOCAxNkgxMk0xMCAxMFYxMk0xNCAxNVYxN00yMSAxMC4xNTAzVjE3Ljk2NjhDMjEgMjAuMTk0MyAxOS4yMDkxIDIyIDE3IDIySDdDNC43OTA4NiAyMiAzIDIwLjE5NDMgMyAxNy45NjY4VjEwLjE1MDNDMyA4LjkzOTM3IDMuNTM5NjQgNy43OTI1IDQuNDY5ODYgNy4wMjY1Mkw5LjQ2OTg2IDIuOTA5MzVDMTAuOTQyMyAxLjY5Njg5IDEzLjA1NzcgMS42OTY4OCAxNC41MzAxIDIuOTA5MzVMMTkuNTMwMSA3LjAyNjUyQzIwLjQ2MDQgNy43OTI1IDIxIDguOTM5MzcgMjEgMTAuMTUwM1oiLz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "DLINA",
                    "TW": "DLINA",
                    "CN": "DLINA",
                    "BR": "DLINA",
                    "CZ": "DLINA",
                    "DA": "DLINA",
                    "DE": "DLINA",
                    "ES": "DLINA",
                    "FI": "DLINA",
                    "FR": "DLINA",
                    "HU": "DLINA",
                    "IT": "DLINA",
                    "JP": "DLINA",
                    "KR": "DLINA",
                    "MS": "DLINA",
                    "NL": "DLINA",
                    "NO": "DLINA",
                    "PL": "DLINA",
                    "RU": "DLINA",
                    "SV": "DLINA",
                    "TH": "DLINA",
                    "TR": "DLINA",
                    "UK": "DLINA",
                    "RO": "DLINA",
                    "SL": "DLINA"
                },
                {
                    "category": "media_and_entertainment",
                    "ui-sort": 6,
                    "type": [
                        "47"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJNZWRpYSBEZXZpY2VzL2hvbWUgY2luZW1hIj48ZyBpZD0iR3JvdXAgNDQyOSI+PGcgaWQ9IlZlY3Rvcl9fX19fMF8wX0JYUFBDUEdGS04iPjxwYXRoIHN0cm9rZT0iIzI4MzAzRiIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0yIDYuNkMyIDQuNjExNzcgMy42MTE3NyAzIDUuNiAzSDEyLjhDMTQuNzg4MiAzIDE2LjQgNC42MTE3NyAxNi40IDYuNlYxNy40QzE2LjQgMTkuMzg4MiAxNC43ODgyIDIxIDEyLjggMjFINS42QzMuNjExNzcgMjEgMiAxOS4zODgyIDIgMTcuNFY2LjZaIi8+PHBhdGggc3Ryb2tlPSIjMjgzMDNGIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTEyLjggMTQuN0MxMi44IDE2LjY4ODIgMTEuMTg4MiAxOC4zIDkuMiAxOC4zQzcuMjExNzcgMTguMyA1LjYgMTYuNjg4MiA1LjYgMTQuN0M1LjYgMTIuNzExOCA3LjIxMTc3IDExLjEgOS4yIDExLjFDMTEuMTg4MiAxMS4xIDEyLjggMTIuNzExOCAxMi44IDE0LjdaIi8+PC9nPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMzY4X19fX18wXzFfRFlWTU1QU0VaRCIgY3g9IjAuOSIgY3k9IjAuOSIgcj0iMC45IiBmaWxsPSIjMjgzMDNGIiB0cmFuc2Zvcm09Im1hdHJpeCgxIDAgMCAtMSA4LjMwMDM3IDE1LjU5OTgpIi8+PHBhdGggaWQ9IkVsbGlwc2UgMzY3X19fX18wXzJfT1lDU1ZWWFpMRyIgZmlsbD0iIzI4MzAzRiIgZD0iTTEwLjU1MDcgNy4wNTAwOUMxMC41NTA3IDcuNzk1NjggOS45NDYzMiA4LjQwMDA5IDkuMjAwNzQgOC40MDAwOUM4LjQ1NTE2IDguNDAwMDkgNy44NTA3NCA3Ljc5NTY4IDcuODUwNzQgNy4wNTAwOUM3Ljg1MDc0IDYuMzA0NTEgOC40NTUxNiA1LjcwMDA5IDkuMjAwNzQgNS43MDAwOUM5Ljk0NjMyIDUuNzAwMDkgMTAuNTUwNyA2LjMwNDUxIDEwLjU1MDcgNy4wNTAwOVoiLz48cGF0aCBpZD0iRWxsaXBzZSAzNjlfX19fXzBfM19IQURMTFFLU1VNIiBmaWxsPSIjMjgzMDNGIiBkPSJNMTkuOTk5NiAxMS4xMDAxQzE5Ljk5OTYgMTEuNTk3MSAxOS41OTY3IDEyLjAwMDEgMTkuMDk5NiAxMi4wMDAxQzE4LjYwMjYgMTIuMDAwMSAxOC4xOTk2IDExLjU5NzEgMTguMTk5NiAxMS4xMDAxQzE4LjE5OTYgMTAuNjAzIDE4LjYwMjYgMTAuMjAwMSAxOS4wOTk2IDEwLjIwMDFDMTkuNTk2NyAxMC4yMDAxIDE5Ljk5OTYgMTAuNjAzIDE5Ljk5OTYgMTEuMTAwMVoiLz48cGF0aCBpZD0iRWxsaXBzZSAzNzBfX19fXzBfNF9ZREZQWkNSS1JIIiBmaWxsPSIjMjgzMDNGIiBkPSJNMTkuOTk5NiAxNC42OTk5QzE5Ljk5OTYgMTUuMTk3IDE5LjU5NjcgMTUuNTk5OSAxOS4wOTk2IDE1LjU5OTlDMTguNjAyNiAxNS41OTk5IDE4LjE5OTYgMTUuMTk3IDE4LjE5OTYgMTQuNjk5OUMxOC4xOTk2IDE0LjIwMjkgMTguNjAyNiAxMy43OTk5IDE5LjA5OTYgMTMuNzk5OUMxOS41OTY3IDEzLjc5OTkgMTkuOTk5NiAxNC4yMDI5IDE5Ljk5OTYgMTQuNjk5OVoiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzVfVkFRV0pYVFRNVyIgc3Ryb2tlPSIjMjgzMDNGIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE1LjA1MDQgMjAuMUgxNS41MDA0SDE5LjEwMDRDMjEuMDg4NiAyMC4xIDIxLjgwMDQgMTkuMzg4MiAyMS44MDA0IDE3LjRWMTAuMkMyMS44MDA0IDguMjExNzggMjEuMDg4NiA3LjUgMTkuMTAwNCA3LjVIMTUuOTUwNCIvPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Home Cinema",
                    "TW": "家庭劇院",
                    "CN": "家庭影院",
                    "BR": "Home Cinema",
                    "CZ": "Domácí kino",
                    "DA": "Hjemmebiograf",
                    "DE": "Heimkino",
                    "ES": "Cine en casa",
                    "FI": "Kotiteatteri",
                    "FR": "Home Cinema",
                    "HU": "Házimozi",
                    "IT": "Home Cinema",
                    "JP": "ホーム シネマ",
                    "KR": "홈 시네마",
                    "MS": "Home Cinema",
                    "NL": "Home Cinema",
                    "NO": "Hjemmekino",
                    "PL": "Kino domowe",
                    "RU": "Домашний кинотеатр",
                    "SV": "Hemmabio",
                    "TH": "โฮมซีเนมา",
                    "TR": "Evde Sinema",
                    "UK": "Домашній кінотеатр",
                    "RO": "Home Cinema",
                    "SL": "Domači kino"
                },
                {
                    "category": "media_and_entertainment",
                    "ui-sort": 7,
                    "type": [
                        "48"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJNZWRpYSBEZXZpY2VzL3dpcmVsZXNzIGhlYWRwaG9uZSI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8wX0ZBWEJUTFFYQVEiIGZpbGw9ImJsYWNrIiBkPSJNNi4xMjUgMTQuNDk4Nkw2LjI3MDI5IDEzLjkxNjVDNi4wOTEwNyAxMy44NzE4IDUuOTAxMjMgMTMuOTEyMSA1Ljc1NTY2IDE0LjAyNThDNS42MTAwOCAxNC4xMzk1IDUuNTI1IDE0LjMxMzkgNS41MjUgMTQuNDk4Nkg2LjEyNVpNMTguODc1IDE0LjQ5ODZIMTkuNDc1QzE5LjQ3NSAxNC4zMTM5IDE5LjM4OTkgMTQuMTM5NSAxOS4yNDQzIDE0LjAyNThDMTkuMDk4OCAxMy45MTIxIDE4LjkwODkgMTMuODcxOCAxOC43Mjk3IDEzLjkxNjVMMTguODc1IDE0LjQ5ODZaTTcuNSAzLjRDNC4xNDg2IDMuNCAxLjQgNi4wMzkxNyAxLjQgOS4zMzMzM0gyLjZDMi42IDYuNzM2NDYgNC43NzYyNyA0LjYgNy41IDQuNlYzLjRaTTkuNjUgNi42NjY2N1YxMkgxMC44NVY2LjY2NjY3SDkuNjVaTTkuNjUgMTJDOS42NSAxMy4xMjQxIDguNzA0OTUgMTQuMDY2NyA3LjUgMTQuMDY2N1YxNS4yNjY3QzkuMzMyNjIgMTUuMjY2NyAxMC44NSAxMy44MjE0IDEwLjg1IDEySDkuNjVaTTcuNSA0LjZDOC43MDQ5NSA0LjYgOS42NSA1LjU0MjU1IDkuNjUgNi42NjY2N0gxMC44NUMxMC44NSA0Ljg0NTI3IDkuMzMyNjIgMy40IDcuNSAzLjRWNC42Wk0xLjQgOS4zMzMzM1YxOEgyLjZWOS4zMzMzM0gxLjRaTTQuMDYyNSAyMC42QzUuNTE1NDIgMjAuNiA2LjcyNSAxOS40NTMyIDYuNzI1IDE4SDUuNTI1QzUuNTI1IDE4Ljc1NTkgNC44ODc3NSAxOS40IDQuMDYyNSAxOS40VjIwLjZaTTEuNCAxOEMxLjQgMTkuNDUzMiAyLjYwOTU4IDIwLjYgNC4wNjI1IDIwLjZWMTkuNEMzLjIzNzI1IDE5LjQgMi42IDE4Ljc1NTkgMi42IDE4SDEuNFpNNi43MjUgMThWMTQuNDk4Nkg1LjUyNVYxOEg2LjcyNVpNNS45Nzk3MSAxNS4wODA4QzYuNDY2NCAxNS4yMDIzIDYuOTc1OTIgMTUuMjY2NyA3LjUgMTUuMjY2N1YxNC4wNjY3QzcuMDc0NTEgMTQuMDY2NyA2LjY2MjU2IDE0LjAxNDQgNi4yNzAyOSAxMy45MTY1TDUuOTc5NzEgMTUuMDgwOFpNMTcuNSA0LjZDMjAuMjIzNyA0LjYgMjIuNCA2LjczNjQ2IDIyLjQgOS4zMzMzM0gyMy42QzIzLjYgNi4wMzkxNyAyMC44NTE0IDMuNCAxNy41IDMuNFY0LjZaTTE0LjE1IDYuNjY2NjdWMTJIMTUuMzVWNi42NjY2N0gxNC4xNVpNMTQuMTUgMTJDMTQuMTUgMTMuODIxNCAxNS42Njc0IDE1LjI2NjcgMTcuNSAxNS4yNjY3VjE0LjA2NjdDMTYuMjk1MSAxNC4wNjY3IDE1LjM1IDEzLjEyNDEgMTUuMzUgMTJIMTQuMTVaTTE3LjUgMy40QzE1LjY2NzQgMy40IDE0LjE1IDQuODQ1MjcgMTQuMTUgNi42NjY2N0gxNS4zNUMxNS4zNSA1LjU0MjU1IDE2LjI5NTEgNC42IDE3LjUgNC42VjMuNFpNMjIuNCA5LjMzMzMzVjE4SDIzLjZWOS4zMzMzM0gyMi40Wk0yMC45Mzc1IDE5LjRDMjAuMTEyMiAxOS40IDE5LjQ3NSAxOC43NTU5IDE5LjQ3NSAxOEgxOC4yNzVDMTguMjc1IDE5LjQ1MzIgMTkuNDg0NiAyMC42IDIwLjkzNzUgMjAuNlYxOS40Wk0yMi40IDE4QzIyLjQgMTguNzU1OSAyMS43NjI4IDE5LjQgMjAuOTM3NSAxOS40VjIwLjZDMjIuMzkwNCAyMC42IDIzLjYgMTkuNDUzMiAyMy42IDE4SDIyLjRaTTE5LjQ3NSAxOFYxNC40OTg2SDE4LjI3NVYxOEgxOS40NzVaTTE4LjcyOTcgMTMuOTE2NUMxOC4zMzc0IDE0LjAxNDQgMTcuOTI1NSAxNC4wNjY3IDE3LjUgMTQuMDY2N1YxNS4yNjY3QzE4LjAyNDEgMTUuMjY2NyAxOC41MzM2IDE1LjIwMjMgMTkuMDIwMyAxNS4wODA4TDE4LjcyOTcgMTMuOTE2NVoiLz48Y2lyY2xlIGlkPSJFbGxpcHNlIDExNjdfX19fXzBfMV9PV1ZBWExOR1ZOIiBjeD0iNSIgY3k9IjEwIiByPSIxIiBmaWxsPSJibGFjayIvPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTE2OF9fX19fMF8yX0RPQ1hRVlZOSkwiIGN4PSIyMCIgY3k9IjEwIiByPSIxIiBmaWxsPSJibGFjayIvPjwvZz48L2c+PC9zdmc+",
                    "EN": "Wireless Headphone",
                    "TW": "無線耳機",
                    "CN": "无线耳机",
                    "BR": "Fones de ouvido sem fio",
                    "CZ": "Bezdrátová sluchátka",
                    "DA": "Trådløs hovedtelefon",
                    "DE": "Wireless-Kopfhörer",
                    "ES": "Auricular inalámbrico",
                    "FI": "Langattomat kuulokkeetl",
                    "FR": "Casque sans fil",
                    "HU": "Vezeték nélküli fejhallgató",
                    "IT": "Cuffie wireless",
                    "JP": "無線ヘッドホーン",
                    "KR": "무선 헤드폰",
                    "MS": "Wireless Headphone",
                    "NL": "Draadloze hoofdtelefoon",
                    "NO": "Trådløse hodetelefoner",
                    "PL": "Słuchawki bezprzewodowe",
                    "RU": "Беспроводные наушники",
                    "SV": "Trådlös hörlur",
                    "TH": "หูฟังแบบไร้สาย",
                    "TR": "Kablosuz Kulaklık",
                    "UK": "Бездротові навушники",
                    "RO": "Căști wireless",
                    "SL": "Brezžične slušalke"
                },
                {
                    "category": "media_and_entertainment",
                    "ui-sort": 8,
                    "type": [
                        "7",
                        "8",
                        "16",
                        "17"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48cGF0aCBzdHJva2U9IiMwMDAiIHN0cm9rZS1taXRlcmxpbWl0PSIxMCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Im0xNS40NzggMTUuMzgzLjQyMi0uNDI2LS4xNzUtLjE3NEg3Ljk2OGwtLjE3OS4yMDEuNDQ4LjQtLjQ0OC0uNC0uMDAxLjAwMi0uMDA1LjAwNS0uMDE5LjAyMS0uMDcyLjA4LS4yNi4yOTFjLS4yMTYuMjQzLS41MS41Ny0uODE3LjkxYTcyLjIxNyA3Mi4yMTcgMCAwIDEtMS40NzkgMS42MTNjLS4zMy4zMy0xLjIxMy44OC0yLjMzNy4zMi0uNTQ1LS4yNzItLjg0NS0uNzI2LTEuMDEyLTEuMTgzYTMuMTc4IDMuMTc4IDAgMCAxLS4xODYtLjk2M2MuMTEtLjQzLjUyNS0yLjAxMyAxLjAwMy0zLjcyMi41MTYtMS44NDYgMS4wOS0zLjc4MiAxLjQzNC00LjYwMy42Ni0xLjU4IDEuNzgyLTIuMTU1IDIuMzMtMi4xNTVoMTEuMjQ5Yy41NzUgMCAxLjgyMy42MDQgMi40NzIgMi4xNDguMzQuODg3Ljc5IDIuNzM0IDEuMTggNC40OC4yMi45OC40MiAxLjkzMS41NjUgMi42Mi4xMDguNTEuMTg1Ljg3NS4yMTYgMSAuMDU1LjIxOS4xMTcuNzEuMDEzIDEuMjA3LS4xLjQ4LS4zNDEuOTE1LS44NTIgMS4xNy0xLjEyNC41Ni0yLjAwNy4wMS0yLjMzNy0uMzE5bC0xLjY1LTEuNjM3LS45Mi0uOTEyLS4yOTMtLjI5LS4wODItLjA4MS0uMDIxLS4wMjItLjAwNi0uMDA1LS4wMDEtLjAwMS0uNDIzLjQyNVoiLz48cGF0aCBmaWxsPSIjMDAwIiBkPSJNOC45NjQgOS40ODlIN3YxLjk2NWgxLjk2NVY5LjQ4OVptNy44NiAwSDE0Ljg2djEuOTY1aDEuOTY1VjkuNDg5WiIvPjwvc3ZnPg==",
                    "EN": "Game Console",
                    "TW": "遊戲主機",
                    "CN": "游戏控制台",
                    "BR": "Console de jogo",
                    "CZ": "Herní konzola",
                    "DA": "Spilkonsol",
                    "DE": "Spielekonsole",
                    "ES": "Consola de juegos",
                    "FI": "Pelikonsoli",
                    "FR": "Console de jeu",
                    "HU": "Játékkonzol",
                    "IT": "Console giochi",
                    "JP": "ゲーム機",
                    "KR": "게임 콘솔",
                    "MS": "Game Console",
                    "NL": "Spelconsole",
                    "NO": "Spillkonsoll",
                    "PL": "Konsola do gier",
                    "RU": "Игровая консоль",
                    "SV": "Spelkonsol",
                    "TH": "คอนโซลเกม",
                    "TR": "Oyun Konsolu",
                    "UK": "Ігрова консоль",
                    "RO": "Consolă de jocuri",
                    "SL": "Igralna konzola"
                },
                {
                    "category": "media_and_entertainment",
                    "ui-sort": 9,
                    "type": [
                        "12"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJNZWRpYSBEZXZpY2VzL3NldC11cCBib3giPjxnIGlkPSJHcm91cCA0NDMwIj48Y2lyY2xlIGlkPSJFbGxpcHNlIDM3MF9fX19fMF8wX1lSTVVBSFRYS1IiIGN4PSIxIiBjeT0iMSIgcj0iMSIgZmlsbD0iIzI4MzAzRiIgdHJhbnNmb3JtPSJtYXRyaXgoMSAwIDAgLTEgNyAxMikiLz48Y2lyY2xlIGlkPSJFbGxpcHNlIDM3MV9fX19fMF8xX09BWVBJRlFNU0EiIGN4PSIxIiBjeT0iMSIgcj0iMSIgZmlsbD0iIzI4MzAzRiIgdHJhbnNmb3JtPSJtYXRyaXgoMSAwIDAgLTEgNCAxMikiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzJfR0JLUUJSRU1XRiIgc3Ryb2tlPSIjMjgzMDNGIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS41IiBkPSJNNSAxNUwxOSAxNU01IDE1QzIuNzkwODYgMTUgMiAxNC4yMDkxIDIgMTJMMiAxMEMyIDcuNzkwODYgMi43OTA4NiA3IDUgN0wxOSA3QzIxLjIwOTEgNyAyMiA3Ljc5MDg2IDIyIDEwVjEyQzIyIDE0LjIwOTEgMjEuMjA5MSAxNSAxOSAxNU01IDE1TDUgMTdNMTkgMTVWMTdNMTEgMTFIMTkiLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Set-top Box",
                    "TW": "機上盒",
                    "CN": "机顶盒",
                    "BR": "Conversor de TV por assinatura",
                    "CZ": "Set-top box",
                    "DA": "Opsætningsboks",
                    "DE": "Set-Top-Box",
                    "ES": "Decodificador",
                    "FI": "Digiboksi",
                    "FR": "Boîtier décodeur",
                    "HU": "Set-top box",
                    "IT": "Set-top Box",
                    "JP": "セットトップ ボックス",
                    "KR": "셋톱 박스",
                    "MS": "Set-top Box",
                    "NL": "Set-top Box",
                    "NO": "TV-boks",
                    "PL": "Przystawka do telewizora",
                    "RU": "Приставка",
                    "SV": "Digitalbox",
                    "TH": "กล่องแปลงสัญญาณโทรทัศน์",
                    "TR": "Alıcı Kutusu",
                    "UK": "Телевізійна приставка",
                    "RO": "Set-top Box",
                    "SL": "Komunikator"
                },
                {
                    "category": "media_and_entertainment",
                    "ui-sort": 10,
                    "type": [
                        "27"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJNZWRpYSBEZXZpY2VzL2Nocm9tY2FzdCI+PGcgaWQ9IkltcG9ydGVkLUxheWVycy1Db3B5LTg0Ij48cGF0aCBpZD0iRmlsbC0xX19fX18wXzBfQUdCWkdFV01VRiIgZmlsbD0iYmxhY2siIGQ9Ik0yIDExLjIyMzVWMTMuMDM3M0M1LjQ5NTQyIDEzLjAzNzMgOC4zMzg5MiAxNS45MDQ1IDguMzM4OTIgMTkuNDI5SDEwLjEzNzhDMTAuMTM3OCAxNC44OTM1IDYuNDk4MDUgMTEuMjIzNSAyIDExLjIyMzVaIi8+PHBhdGggaWQ9IkZpbGwtMl9fX19fMF8xX0tPRVFNWUFRUk4iIGZpbGw9ImJsYWNrIiBkPSJNMiAxMy45NTg0VjE1Ljc3MjJDMy45OTk2MiAxNS43NzIyIDUuNjI2MzkgMTcuNDEyNSA1LjYyNjM5IDE5LjQyODhINy40MjUyN0M3LjQyNTI3IDE2LjQwNTEgNC45OTg3IDEzLjk1ODQgMiAxMy45NTg0WiIvPjxwYXRoIGlkPSJGaWxsLTNfX19fXzBfMl9IWFpNTEdLWUZSIiBmaWxsPSJibGFjayIgZD0iTTIgMTYuNjkyN1YxOS40MjhINC43MTI3NEM0LjcxMjc0IDE3LjkxNjIgMy40OTkzNSAxNi42OTI3IDIgMTYuNjkyN1oiLz48cGF0aCBpZD0iRmlsbC00X19fX18wXzNfRU9LR0ZVQlFGUiIgZmlsbD0iYmxhY2siIGQ9Ik05LjU4MzI3IDE2LjQ0ODNDOS42MzExIDE2LjU3MjQgOS42NzYyMiAxNi42OTc3IDkuNzE3OTkgMTYuODI0NUM5LjY3NjIyIDE2LjY5NzcgOS42MzExIDE2LjU3MjQgOS41ODMyNyAxNi40NDgzWiIvPjxwYXRoIGlkPSJGaWxsLTVfX19fXzBfNF9XUk5HVFVKUktJIiBmaWxsPSJibGFjayIgZD0iTTEwLjAxMzEgMTguMDAyMUM5Ljk0MzEgMTcuNTk4NiA5Ljg0NDA5IDE3LjIwNTQgOS43MTc5MyAxNi44MjQ2QzkuODQ0MDkgMTcuMjA1NCA5Ljk0MzEgMTcuNTk4NiAxMC4wMTMxIDE4LjAwMjFaIi8+PHBhdGggaWQ9IkZpbGwtNl9fX19fMF81X0dHWlBQTE1GQ0YiIGZpbGw9ImJsYWNrIiBkPSJNNC41NzQ3MSAxMS42NDM3QzQuMTk3MDYgMTEuNTE2OSAzLjgwNzI5IDExLjQxNzUgMy40MDcyOCAxMS4zNDczQzMuODA3MjkgMTEuNDE3NSA0LjE5NzI3IDExLjUxNzEgNC41NzQ3MSAxMS42NDM3WiIvPjxwYXRoIGlkPSJGaWxsLTdfX19fXzBfNl9OT1dJTVRXVERRIiBmaWxsPSJibGFjayIgZD0iTTIwLjcxNTIgNEg0LjY5MjExQzMuOTg1NDYgNCAzLjQwNzI4IDQuNTgyOTkgMy40MDcyOCA1LjI5NTUxVjkuNzcwNzNDMy45MzMyNCA5Ljg0NzE4IDQuNDQ4MTMgOS45Njg1IDQuOTQ5MjQgMTAuMTI5MlY1LjU1NDU3SDIwLjQ1ODNWMTYuNDQ4SDExLjIyMDdDMTEuMzgwMyAxNi45NTMyIDExLjUwMTIgMTcuNDcyNCAxMS41Nzc3IDE4LjAwMjdIMjAuNzE1MkMyMS40MjE4IDE4LjAwMjcgMjIgMTcuNDE5OCAyMiAxNi43MDdWNS4yOTU1MUMyMiA0LjU4Mjk5IDIxLjQyMTggNCAyMC43MTUyIDRaIi8+PHBhdGggaWQ9IkZpbGwtOF9fX19fMF83X0pCWk1TWldBVUwiIGZpbGw9ImJsYWNrIiBkPSJNNC45NDg5MiAxMS43Nzk2QzQuODI1ODkgMTEuNzMxMyA0LjcwMTYxIDExLjY4NjIgNC41NzYwNyAxMS42NDM5QzQuNzAxNjEgMTEuNjg2MiA0LjgyNTg5IDExLjczMTMgNC45NDg5MiAxMS43Nzk2WiIvPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Chromcast",
                    "TW": "Chromcast",
                    "CN": "Chromcast",
                    "BR": "Chromcast",
                    "CZ": "Chromcast",
                    "DA": "Chromcast",
                    "DE": "Chromcast",
                    "ES": "Chromcast",
                    "FI": "Chromcast",
                    "FR": "Chromcast",
                    "HU": "Chromcast",
                    "IT": "Chromcast",
                    "JP": "Chromcast",
                    "KR": "Chromcast",
                    "MS": "Chromcast",
                    "NL": "Chromcast",
                    "NO": "Chromcast",
                    "PL": "Chromcast",
                    "RU": "Chromcast",
                    "SV": "Chromcast",
                    "TH": "Chromcast",
                    "TR": "Chromcast",
                    "UK": "Chromcast",
                    "RO": "Chromcast",
                    "SL": "Chromcast"
                },
                {
                    "category": "media_and_entertainment",
                    "ui-sort": 11,
                    "type": [
                        "74"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBzdHJva2U9IiMwMDAiIHN0cm9rZS13aWR0aD0iMS4yIj48cGF0aCBkPSJNMTQuNjg2IDYuMTE3Yy4wNy4zNC4wNDYuNjg2LjA1IDEuMDMuMDAzIDEuOTAxLS4wMDYgMi44MDMuMDA1IDQuNzAzLjU3My4yNjcgMS4xOTQuNDcgMS44MzMuNDYzYTIuNDggMi40OCAwIDAgMCAxLjIyLS4yODZjLjM4Ny0uMjA3LjcwNi0uNTI5LjkzNi0uOS4yNi0uNDIxLjQxNi0uODk5LjUwNC0xLjM4My4xMDYtLjU1Ny4xMDctMS4xMjcuMDktMS42OTItLjA0LS42OTItLjU0OC0xLjUzMy0uNTQ4LTEuNTMzcy0uNDktLjgzNC0uODM3LTEuMTcxYTUuNDkgNS40OSAwIDAgMC0xLjA4Mi0uODI2Yy0uODY4LS41MTMtMS44MjMtLjg1OC0yLjc4Mi0xLjE2MmE3Mi4zOCA3Mi4zOCAwIDAgMC0xLjg0MS0uNTZjLTEuMDc3LS4zMDMtMi4xNi0uNTk2LTMuMjYtLjgtLjAwMSA2LjA1My0uMDAyIDEyLjEwNiAwIDE4LjE1OWw0LjEyMyAxLjMwM2MuMDAzLTUuMDg5IDAtMTAuMTc4LjAwMS0xNS4yNjcuMDA5LS4yODkuMDU3LS41OTYuMjQ0LS44MjhhLjUwNy41MDcgMCAwIDEgLjQ4Ni0uMTg3Yy4xNzkuMDM3LjM1Ny4xMDcuNDk0LjIzLjIwNy4xNzkuMzE1LjQ0NC4zNjQuNzA3WiIgY2xpcC1ydWxlPSJldmVub2RkIi8+PHBhdGggc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBkPSJtMTMuNTcgMTUuNjkyIDUuNDk1LTEuNDcyYTIuODQ3IDIuODQ3IDAgMCAxIDMuNDg3IDIuMDEzdjBhMi44NDggMi44NDggMCAwIDEtMS45OTcgMy40ODNsLTYuOTg1IDEuOTE4Ii8+PHBhdGggZD0ibTE5LjY0MiAxNi45NjItNi42NzggMS44MTYiLz48cGF0aCBzdHJva2UtbGluZWNhcD0icm91bmQiIGQ9Ik04LjcxNSAxMi41NjUgMy43NDMgMTMuOTVhMi44NDcgMi44NDcgMCAwIDAtMS45NzggMy41MDh2MGEyLjg0OCAyLjg0OCAwIDAgMCAzLjQ5IDEuOTgzbDMuNDYtLjk0MiIvPjxwYXRoIGQ9Im00LjY0IDE2LjYwNSA0LjE4Ni0xLjE1Ii8+PC9nPjwvc3ZnPg==",
                    "EN": "PlayStation",
                    "TW": "PlayStation",
                    "CN": "PlayStation",
                    "BR": "PlayStation",
                    "CZ": "PlayStation",
                    "DA": "PlayStation",
                    "DE": "PlayStation",
                    "ES": "PlayStation",
                    "FI": "PlayStation",
                    "FR": "PlayStation",
                    "HU": "PlayStation",
                    "IT": "PlayStation",
                    "JP": "PlayStation",
                    "KR": "PlayStation",
                    "MS": "PlayStation",
                    "NL": "PlayStation",
                    "NO": "PlayStation",
                    "PL": "PlayStation",
                    "RU": "PlayStation",
                    "SV": "PlayStation",
                    "TH": "PlayStation",
                    "TR": "PlayStation",
                    "UK": "PlayStation",
                    "RO": "PlayStation",
                    "SL": "PlayStation"
                },
                {
                    "category": "media_and_entertainment",
                    "ui-sort": 12,
                    "type": [
                        "75"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJNZWRpYSBEZXZpY2VzL3BsYXkgc3RhdGlvbjQiPjxnIGlkPSJHcm91cCAzNTMxIj48ZyBpZD0iRnJhbWUiIGNsaXAtcGF0aD0idXJsKCNjbGlwMF8zMTVfMTY5NCkiPjxnIGlkPSJHcm91cF8yIj48cGF0aCBpZD0iVmVjdG9yX19fX18wXzBfUFJIV1JDTU1LRCIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLW1pdGVybGltaXQ9IjEwIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTkuNzI5NTEgMTIuNDM3NEwxMC4xNjYxIDguOTQ0NkwxMi42MTA2IDUuMjc3ODdMMTcuNDk5NiAxLjYxMTE0TDE4LjcyMTggNC4wNTU2M0wxOS45NDQxIDMuNDQ0NUwyMi4zODg1IDE4LjcyMjVMMjEuMTY2MyAxOS45NDQ4SDE4LjcyMThMMTcuNDk5NiAyMS4xNjdMMTQuNzU1MyAyMC4zODMiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzFfQVFLQ1BEUkhSSiIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLW1pdGVybGltaXQ9IjEwIiBzdHJva2Utd2lkdGg9IjAuOCIgZD0iTTE4LjcyMTggNC4wNTU2M0wxOS45NDQgMTkuOTQ0OCIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMl9WRlNBV0xDTlJRIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbWl0ZXJsaW1pdD0iMTAiIHN0cm9rZS13aWR0aD0iMC44IiBkPSJNMTcuNDk5NiAxLjYxMTE2VjIxLjE2NyIvPjwvZz48L2c+PGcgaWQ9Ikdyb3VwXzMiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfM19TQUZFUVhTRFRTIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbWl0ZXJsaW1pdD0iMTAiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTUuMDI2MiAxOC45OTQzQzE1LjAyNjMgMTguOTk0NiAxNS4wMjY0IDE4Ljk5NDggMTUuMDI2NCAxOC45OTUxQzE1LjA1NjIgMTkuMTE0NSAxNS4wOTgxIDE5LjQ2MjIgMTUuMDMwOSAxOS44MTYzQzE0Ljk2NDYgMjAuMTY1NiAxNC44MTYyIDIwLjQxNjggMTQuNTYxNiAyMC41NDM3QzEzLjkwNjEgMjAuODcwNSAxMy4zOTgzIDIwLjU1NDIgMTMuMjA4MSAyMC4zNjQ2QzEzLjA5ODYgMjAuMjU1NiAxMi44OTk3IDIwLjEwMjEgMTIuNzA1OSAxOS45NTc4QzEyLjQ5OCAxOS44MDMyIDEyLjI1MTUgMTkuNjI2OSAxMi4wMTcyIDE5LjQ2MTlDMTEuNzgyMyAxOS4yOTY0IDExLjU1NjkgMTkuMTQwNCAxMS4zOTA1IDE5LjAyNTlDMTEuMzA3MiAxOC45Njg2IDExLjIzODYgMTguOTIxNiAxMS4xOTA2IDE4Ljg4ODhMMTEuMTM1MSAxOC44NTA5TDExLjEyMDMgMTguODQwOUwxMS4xMTY1IDE4LjgzODNMMTEuMTE1NSAxOC44Mzc2TDExLjExNTIgMTguODM3NEwxMS4xMTUyIDE4LjgzNzRMMTAuNzc3OSAxOS4zMzM2TDExLjExNTEgMTguODM3M0wxMC45NjI1IDE4LjczMzZIMTAuNzc3OUg1Ljg4ODk3SDUuNzAwNjlMNS41NDYxNiAxOC44NDEyTDUuODg4OTcgMTkuMzMzNkw1LjU0NjEzIDE4Ljg0MTJMNS41NDYwNSAxOC44NDEzTDUuNTQ1NzkgMTguODQxNUw1LjU0NDgxIDE4Ljg0MjJMNS41NDEwNyAxOC44NDQ4TDUuNTI2NzIgMTguODU0OEw1LjQ3MjQ4IDE4Ljg5MjdDNS40MjU3MSAxOC45MjU0IDUuMzU4NjggMTguOTcyNCA1LjI3NzM4IDE5LjAyOTdDNS4xMTQ4OSAxOS4xNDQyIDQuODk0ODQgMTkuMzAwMiA0LjY2NTI1IDE5LjQ2NTZDNC40MzYxNyAxOS42MzA2IDQuMTk1MTEgMTkuODA2NyAzLjk5MTQ4IDE5Ljk2MTJDMy44MDA2NCAyMC4xMDYgMy42MDcxOSAyMC4yNTggMy41MDAyMiAyMC4zNjQ2QzMuMzA5OTkgMjAuNTU0MiAyLjgwMjE5IDIwLjg3MDUgMi4xNDY2OCAyMC41NDM3TDEuODg4MjggMjEuMDYyMkwyLjE0NjY4IDIwLjU0MzdDMS44OTEzOSAyMC40MTY1IDEuNzQwNzggMjAuMTYzNyAxLjY2MjEgMTkuODAxQzEuNTk0ODEgMTkuNDkwNyAxLjU5ODY5IDE5LjE5MjkgMS42MDE2MyAxOC45Njc4QzEuNjAxODEgMTguOTUzNyAxLjYwMTk5IDE4LjkzOTggMS42MDIxNSAxOC45MjYzQzEuNzY1OTggMTguMjk3NiAyLjY2Nzg1IDE1LjAwMzQgMy4xMTQyMSAxMy45MzU1QzMuNTQwNzUgMTIuOTE1MSA0LjI0MTIzIDEyLjYwMDIgNC41MDgwMiAxMi42MDAySDEyLjEwMjlDMTIuMzk2NyAxMi42MDAyIDEzLjE3NjcgMTIuOTQ0OCAxMy41OTA5IDEzLjkyODFDMTQuMDYxNiAxNS4xNTMxIDE0LjkyMzggMTguNTg1OCAxNS4wMjYyIDE4Ljk5NDNMMTUuMDI2MiAxOC45OTQzWiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfNF9FRExIUlBaT1paIiBmaWxsPSJibGFjayIgZD0iTTYuNzA0MjQgMTQuNDQ0Nkg1LjA3NDU4VjE2LjA3NDJINi43MDQyNFYxNC40NDQ2WiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfNV9NUVhaVElHR05PIiBmaWxsPSJibGFjayIgZD0iTTExLjU5MiAxNC40NDQ2SDkuOTYyMzZWMTYuMDc0MkgxMS41OTJWMTQuNDQ0NloiLz48L2c+PC9nPjwvZz48L2c+PGRlZnM+PGNsaXBQYXRoIGlkPSJjbGlwMF8zMTVfMTY5NCI+PHBhdGggZmlsbD0id2hpdGUiIGQ9Ik0wIDBIMTQuNjY2OVYyMS4yMDA0SDB6IiB0cmFuc2Zvcm09InRyYW5zbGF0ZSg4LjMzMjc0IDEpIi8+PC9jbGlwUGF0aD48L2RlZnM+PC9zdmc+",
                    "EN": "PlayStation 4",
                    "TW": "PlayStation 4",
                    "CN": "PlayStation 4",
                    "BR": "PlayStation 4",
                    "CZ": "PlayStation 4",
                    "DA": "PlayStation 4",
                    "DE": "PlayStation 4",
                    "ES": "PlayStation 4",
                    "FI": "PlayStation 4",
                    "FR": "PlayStation 4",
                    "HU": "PlayStation 4",
                    "IT": "PlayStation 4",
                    "JP": "PlayStation 4",
                    "KR": "PlayStation 4",
                    "MS": "PlayStation 4",
                    "NL": "PlayStation 4",
                    "NO": "PlayStation 4",
                    "PL": "PlayStation 4",
                    "RU": "PlayStation 4",
                    "SV": "PlayStation 4",
                    "TH": "PlayStation 4",
                    "TR": "PlayStation 4",
                    "UK": "PlayStation 4",
                    "RO": "PlayStation 4",
                    "SL": "PlayStation 4"
                },
                {
                    "category": "media_and_entertainment",
                    "ui-sort": 13,
                    "type": [
                        "76"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJNZWRpYSBEZXZpY2VzL3hib3giPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMF9DSktWUUdIRFFTIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNNS45ODM2NyA1LjMwNjM0QzQuMTUxOSA2Ljk1MzgyIDMgOS4zNDIzOSAzIDEyQzMgMTYuOTcwNiA3LjAyOTQ0IDIxIDEyIDIxQzE2Ljk3MDYgMjEgMjEgMTYuOTcwNiAyMSAxMkMyMSA3LjAyOTQ0IDE2Ljk3MDYgMyAxMiAzQzEwLjQ0NTIgMyA4Ljk4MjQxIDMuMzk0MjggNy43MDYyOCA0LjA4ODMxQzguODU1MDkgNC41MDk1MyAxMS4zODI3IDUuNTA4MiAxMiA2LjA0NjIzQzEyLjcxMjMgNS40Nzg3OCAxNC4yNTUyIDQuNTg4NjUgMTYuMjUzMSA0Ljg1OTM5QzE1LjU5MzcgNS4zODY4NyAxNC4yMTk2IDYuNzUwNDIgMTMuOTk4MSA3Ljk4NDc0QzE1LjUwMTQgOC45NDA0MSAxOC4yMzEyIDEyLjUwMDkgMTguNjI2OCAxNi4xNzRDMTcuMDk2OSAxNC4wMTEzIDEzLjYyOTcgMTAuMDY0IDEyIDkuOTY5QzEwLjI3OTMgOS45NjkgNy44MjY1IDEyLjUwMjcgNS4xMzYzMiAxNi4xNzRDNS42MTEwNSAxMy4zNzEzIDYuODA1ODEgMTAuNzY5OSA5LjUyNzY0IDcuOTg0NzRDOS4wMzk3MSA3LjQzMDg4IDcuODg5NzkgNi4yNjc3NyA3LjE5MzUxIDYuMDQ2MjMiLz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Xbox",
                    "TW": "XBox",
                    "CN": "XBox",
                    "BR": "XBox",
                    "CZ": "XBox",
                    "DA": "XBox",
                    "DE": "XBox",
                    "ES": "XBox",
                    "FI": "XBox",
                    "FR": "XBox",
                    "HU": "XBox",
                    "IT": "XBox",
                    "JP": "XBox",
                    "KR": "XBox",
                    "MS": "XBox",
                    "NL": "XBox",
                    "NO": "XBox",
                    "PL": "XBox",
                    "RU": "XBox",
                    "SV": "XBox",
                    "TH": "XBox",
                    "TR": "XBox",
                    "UK": "XBox",
                    "RO": "XBox",
                    "SL": "XBox"
                },
                {
                    "category": "media_and_entertainment",
                    "ui-sort": 14,
                    "type": [
                        "77"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48cGF0aCBzdHJva2U9IiMwMDAiIHN0cm9rZS1taXRlcmxpbWl0PSIxMCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Im0xNy4xODggMjAuODcyLjQyMi0uNDI2LS4xNzUtLjE3NGgtNC42MTFsLS4xNzkuMi40NDguNC0uNDQ4LS40di4wMDFsLS4wMDMuMDA0LS4wMTEuMDEyLS4wNC4wNDUtLjE0Ny4xNjQtLjQ2Mi41MTRjLS4zNTUuMzk0LS43Mi43OTUtLjgyNy45MDJhLjg2Ny44NjcgMCAwIDEtMS4wMjEuMTMyLjkwMS45MDEgMCAwIDEtLjQ0NC0uNTI1IDEuNTQ2IDEuNTQ2IDAgMCAxLS4wODgtLjQyOGMuMDczLS4yODUuMjk4LTEuMTQuNTU2LTIuMDYxLjI5My0xLjA1LjYxNC0yLjEyNy44LTIuNTczLjM0OC0uODMyLjkwNi0xLjA1OSAxLjA3OC0xLjA1OWg2LjM2MWMuMiAwIC44MjEuMjU3IDEuMTU2IDEuMDUxLjE4NS40ODIuNDM0IDEuNTAzLjY1NiAyLjQ5NC4xMi41NDEuMjM0IDEuMDc3LjMxNyAxLjQ3LjA2Mi4yOTYuMTA4LjUxLjEyNi41ODMuMDIzLjA5NS4wNTMuMzMyLjAwNS41NjYtLjA0Ni4yMTgtLjE0Ni4zODMtLjM0My40ODJhLjg2Ny44NjcgMCAwIDEtMS4wMjItLjEzMmwtLjkzMy0uOTI2LS41Mi0uNTE2LS4xNjctLjE2NC0uMDQ2LS4wNDYtLjAxMi0uMDEyLS4wMDMtLjAwM3YtLjAwMWwtLjQyMy40MjZaIi8+PHBhdGggZmlsbD0iIzAwMCIgZD0iTTEzLjUwNCAxNy41MzloLTEuMTExdjEuMTFoMS4xMXYtMS4xMVptNC40NDYgMGgtMS4xMTJ2MS4xMWgxLjExMXYtMS4xMVpNNi42IDEyYS42LjYgMCAxIDAtMS4yIDBoMS4yWk00IDIuNmgxMFYxLjRINHYxLjJaTTIuNiAyMFY0SDEuNHYxNmgxLjJaTTYgMjEuNEg0djEuMmgydi0xLjJabS42LjZWMTJINS40djEwaDEuMlptOC44LTE4djExLjVoMS4yVjRoLTEuMlpNMTAgMjEuNEg2djEuMmg0di0xLjJaTTEuNCAyMEEyLjYgMi42IDAgMCAwIDQgMjIuNnYtMS4yQTEuNCAxLjQgMCAwIDEgMi42IDIwSDEuNFpNMTQgMi42QTEuNCAxLjQgMCAwIDEgMTUuNCA0aDEuMkEyLjYgMi42IDAgMCAwIDE0IDEuNHYxLjJaTTQgMS40QTIuNiAyLjYgMCAwIDAgMS40IDRoMS4yQTEuNCAxLjQgMCAwIDEgNCAyLjZWMS40WiIvPjxjaXJjbGUgY3g9IjYiIGN5PSI1LjYiIHI9Ii42IiBmaWxsPSIjMDAwIi8+PC9zdmc+",
                    "EN": "Xbox One",
                    "TW": "Xbox One",
                    "CN": "Xbox One",
                    "BR": "Xbox One",
                    "CZ": "Xbox One",
                    "DA": "Xbox One",
                    "DE": "Xbox One",
                    "ES": "Xbox One",
                    "FI": "Xbox One",
                    "FR": "Xbox One",
                    "HU": "Xbox One",
                    "IT": "Xbox One",
                    "JP": "Xbox One",
                    "KR": "Xbox One",
                    "MS": "Xbox One",
                    "NL": "Xbox One",
                    "NO": "Xbox One",
                    "PL": "Xbox One",
                    "RU": "Xbox One",
                    "SV": "Xbox One",
                    "TH": "Xbox One",
                    "TR": "Xbox One",
                    "UK": "Xbox One",
                    "RO": "Xbox One",
                    "SL": "Xbox One"
                },
                {
                    "category": "media_and_entertainment",
                    "ui-sort": 15,
                    "type": [
                        "79"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzA4Ij48ZyBpZD0iR3JvdXAgNDU3MiI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8wX1RNQUtPU1lUWFUiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjUiIGQ9Ik0xMC44NTE2IDkuNzc3MzZDMTAuMDg0OSA5LjAxMDYzIDkuMDI1NjkgOC41MzY0IDcuODU1NyA4LjUzNjRDNi42ODU3MiA4LjUzNjQgNS42MjY0OSA5LjAxMDYzIDQuODU5NzYgOS43NzczNk02LjM1NzczIDExLjI3NTNDNi43NDExIDEwLjg5MiA3LjI3MDcxIDEwLjY1NDkgNy44NTU3IDEwLjY1NDlDOC40NDA3IDEwLjY1NDkgOC45NzAzMSAxMC44OTIgOS4zNTM2NyAxMS4yNzUzIi8+PGcgaWQ9Ikdyb3VwIDQ1NzEiPjxwYXRoIGlkPSJSZWN0YW5nbGUgMTAyNjNfX19fXzBfMV9PUFVEV1FIWEFWIiBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuNSIgZD0iTTEzLjQ4NDggMTkuNzM0NFYxNi43ODE5VjEzLjgyOTVNMTIuODU5NCAxMy42NDA2SDIuOTM3NUMyLjQxOTczIDEzLjY0MDYgMiAxNC4wNjAzIDIgMTQuNTc4MVYxOS4xODc1QzIgMTkuNzA1MiAyLjQxOTczIDIwLjEyNSAyLjkzNzUgMjAuMTI1SDEyLjg1OTRNMTUuNDg0OCAyMkgxOS42MDk4QzIwLjcxNDQgMjIgMjEuNjA5OCAyMS4xMDQ2IDIxLjYwOTggMjBWNEMyMS42MDk4IDIuODk1NDMgMjAuNzE0NCAyIDE5LjYwOTggMkgxNS40ODQ4QzE0LjM4MDIgMiAxMy40ODQ4IDIuODk1NDMgMTMuNDg0OCA0VjIwQzEzLjQ4NDggMjEuMTA0NiAxNC4zODAyIDIyIDE1LjQ4NDggMjJaIi8+PHBhdGggaWQ9IkVsbGlwc2UgMTI5OV9fX19fMF8yX09BQUlOV1daV1QiIHN0cm9rZT0iYmxhY2siIGQ9Ik02LjI2NTY1IDE2LjY4NzVDNi4yNjU2NSAxNy4zNjA2IDUuNzE5OTkgMTcuOTA2MiA1LjA0NjkgMTcuOTA2MkM0LjM3MzggMTcuOTA2MiAzLjgyODE1IDE3LjM2MDYgMy44MjgxNSAxNi42ODc1QzMuODI4MTUgMTYuMDE0NCA0LjM3MzggMTUuNDY4NyA1LjA0NjkgMTUuNDY4N0M1LjcxOTk5IDE1LjQ2ODcgNi4yNjU2NSAxNi4wMTQ0IDYuMjY1NjUgMTYuNjg3NVoiLz48cGF0aCBpZD0iRWxsaXBzZSAxMzAzX19fX18wXzNfQVhPV1JQQ1FOQyIgZmlsbD0iYmxhY2siIGQ9Ik0xNS45ODQ0IDE3LjUwNDVDMTUuOTg0NCAxNy44NDk3IDE1LjcwNDYgMTguMTI5NSAxNS4zNTk0IDE4LjEyOTVDMTUuMDE0MiAxOC4xMjk1IDE0LjczNDQgMTcuODQ5NyAxNC43MzQ0IDE3LjUwNDVDMTQuNzM0NCAxNy4xNTkzIDE1LjAxNDIgMTYuODc5NSAxNS4zNTk0IDE2Ljg3OTVDMTUuNzA0NiAxNi44Nzk1IDE1Ljk4NDQgMTcuMTU5MyAxNS45ODQ0IDE3LjUwNDVaIi8+PHBhdGggaWQ9IkVsbGlwc2UgMTMwNl9fX19fMF80X05QQVFZWUdET0oiIGZpbGw9ImJsYWNrIiBkPSJNOC43OTY4NiAxNi42ODc1QzguNzk2ODYgMTYuOTQ2NCA4LjU4Njk5IDE3LjE1NjIgOC4zMjgxMSAxNy4xNTYyQzguMDY5MjMgMTcuMTU2MiA3Ljg1OTM2IDE2Ljk0NjQgNy44NTkzNiAxNi42ODc1QzcuODU5MzYgMTYuNDI4NiA4LjA2OTIzIDE2LjIxODcgOC4zMjgxMSAxNi4yMTg3QzguNTg2OTkgMTYuMjE4NyA4Ljc5Njg2IDE2LjQyODYgOC43OTY4NiAxNi42ODc1WiIvPjxwYXRoIGlkPSJFbGxpcHNlIDEzMDdfX19fXzBfNV9RSkhSS0NaQlRTIiBmaWxsPSJibGFjayIgZD0iTTEwLjUxNTYgMTYuNjg3NUMxMC41MTU2IDE2Ljk0NjQgMTAuMzA1NyAxNy4xNTYyIDEwLjA0NjggMTcuMTU2MkM5Ljc4Nzk2IDE3LjE1NjIgOS41NzgwOSAxNi45NDY0IDkuNTc4MDkgMTYuNjg3NUM5LjU3ODA5IDE2LjQyODYgOS43ODc5NiAxNi4yMTg3IDEwLjA0NjggMTYuMjE4N0MxMC4zMDU3IDE2LjIxODcgMTAuNTE1NiAxNi40Mjg2IDEwLjUxNTYgMTYuNjg3NVoiLz48cGF0aCBpZD0iRWxsaXBzZSAxMzA0X19fX18wXzZfSVdBTUpGWFBGRCIgZmlsbD0iYmxhY2siIGQ9Ik0xOC4xNzE5IDE3LjUwNDVDMTguMTcxOSAxNy44NDk3IDE3Ljg5MjEgMTguMTI5NSAxNy41NDY5IDE4LjEyOTVDMTcuMjAxNyAxOC4xMjk1IDE2LjkyMTkgMTcuODQ5NyAxNi45MjE5IDE3LjUwNDVDMTYuOTIxOSAxNy4xNTkzIDE3LjIwMTcgMTYuODc5NSAxNy41NDY5IDE2Ljg3OTVDMTcuODkyMSAxNi44Nzk1IDE4LjE3MTkgMTcuMTU5MyAxOC4xNzE5IDE3LjUwNDVaIi8+PHBhdGggaWQ9IkVsbGlwc2UgMTMwNV9fX19fMF83X0tRQU5QSUlWUU8iIGZpbGw9ImJsYWNrIiBkPSJNMjAuMzU5MyAxNy41MDQ1QzIwLjM1OTMgMTcuODQ5NyAyMC4wNzk1IDE4LjEyOTUgMTkuNzM0MyAxOC4xMjk1QzE5LjM4OTEgMTguMTI5NSAxOS4xMDkzIDE3Ljg0OTcgMTkuMTA5MyAxNy41MDQ1QzE5LjEwOTMgMTcuMTU5MyAxOS4zODkxIDE2Ljg3OTUgMTkuNzM0MyAxNi44Nzk1QzIwLjA3OTUgMTYuODc5NSAyMC4zNTkzIDE3LjE1OTMgMjAuMzU5MyAxNy41MDQ1WiIvPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTMwOF9fX19fMF84X0hNVkxTR05XUU4iIGN4PSIxNy41MjQ0IiBjeT0iMTIuNjIyIiByPSIyLjc2ODMiIHN0cm9rZT0iYmxhY2siLz48Y2lyY2xlIGlkPSJFbGxpcHNlIDEzMDlfX19fXzBfOV9SVUpES0RRWVRTIiBjeD0iMTcuNTI0NCIgY3k9IjEyLjYyMiIgcj0iMS4xMzQxNSIgc3Ryb2tlPSJibGFjayIvPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTMxMF9fX19fMF8xMF9TUkZBQlVOT0lEIiBjeD0iMTcuNTI0NCIgY3k9IjYuOTAyNDUiIHI9IjEuMTM0MTUiIHN0cm9rZT0iYmxhY2siLz48L2c+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "Nintendo Switch",
                    "TW": "任天堂Switch",
                    "CN": "任天堂Switch",
                    "BR": "Nintendo Switch",
                    "CZ": "Spínač Nintendo",
                    "DA": "Nintendo Switch",
                    "DE": "Nintendo Switch",
                    "ES": "Nintendo Switch",
                    "FI": "Nintendo Switch",
                    "FR": "Commutateur Nintendo",
                    "HU": "Nintendo Switch",
                    "IT": "Nintendo Switch",
                    "JP": "Nintendo Switch",
                    "KR": "닌텐도 스위치",
                    "MS": "Nintendo Switch",
                    "NL": "Nintendo Switch",
                    "NO": "Nintendo Switch",
                    "PL": "Nintendo Switch",
                    "RU": "Переключатель Nintendo",
                    "SV": "Nintendo Switch",
                    "TH": "Nintendo Switch",
                    "TR": "Nintendo Anahtarı",
                    "UK": "Nintendo Switch",
                    "RO": "Comutator Nintendo",
                    "SL": "Stikalo Nintendo"
                },
                {
                    "category": "media_and_entertainment",
                    "ui-sort": 16,
                    "type": [
                        "80"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJNZWRpYSBEZXZpY2VzL2JsdS1yYXkgcGxheWVyIj48ZyBpZD0iR3JvdXAgMTQiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMF9NR09HUk9BVVREIiBzdHJva2U9IiMyODMwM0YiIHN0cm9rZS13aWR0aD0iMS41IiBkPSJNMTMgNUwxMyAxOU0xMyA1QzEzIDMuMzQzMTUgMTEuNjU2OSAyIDEwIDJMNiAyQzQuMzQzMTUgMiAzIDMuMzQzMTQgMyA1TDMgMTlDMyAyMC42NTY5IDQuMzQzMTUgMjIgNiAyMkgxMEMxMS42NTY5IDIyIDEzIDIwLjY1NjkgMTMgMTlNMTMgNUgxNEMxNy44NjYgNSAyMSA4LjEzNDAxIDIxIDEyQzIxIDE1Ljg2NiAxNy44NjYgMTkgMTQgMTlMMTMgMTlNMTMgOUgxNEMxNS42NTY5IDkgMTcgMTAuMzQzMSAxNyAxMkMxNyAxMy42NTY5IDE1LjY1NjkgMTUgMTQgMTVIMTMiLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Blu-ray Player",
                    "TW": "Blu-ray 播放器",
                    "CN": "蓝光播放器",
                    "BR": "Reprodutor de Blu-ray",
                    "CZ": "Přehrávač Blu-ray",
                    "DA": "Blu-ray-afspiller",
                    "DE": "Blu-ray-Spieler",
                    "ES": "Reproductor de Blu-ray",
                    "FI": "Blu-ray-soitin",
                    "FR": "lecteur Blu-ray",
                    "HU": "Blu-ray lejátszó",
                    "IT": "Lettore Blu-ray",
                    "JP": "Blu-rayプレーヤー",
                    "KR": "블루레이 플레이어",
                    "MS": "Blu-ray Player",
                    "NL": "Blu-ray-speler",
                    "NO": "Blu-ray-spiller",
                    "PL": "Odtwarzacz blu-ray",
                    "RU": "Проигрыватель Blu-ray",
                    "SV": "Blu-ray-spelare",
                    "TH": "เครื่องเล่นบลูเรย์",
                    "TR": "Blu-ray oynatıcı",
                    "UK": "Програвач Blu-ray",
                    "RO": "Player Blu-ray",
                    "SL": "Predvajalnik Blu-ray"
                },
                {
                    "category": "media_and_entertainment",
                    "ui-sort": 17,
                    "type": [
                        "81"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJNZWRpYSBEZXZpY2VzL3NtYXJ0IHNwZWFrZXIiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMF9JWExORk5USlFGIiBzdHJva2U9IiMyODMwM0YiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjUiIGQ9Ik0xNi4yNDI2IDYuNzU3MzZDMTUuMTU2OCA1LjY3MTU3IDEzLjY1NjggNSAxMiA1QzEwLjM0MzEgNSA4Ljg0MzExIDUuNjcxNTcgNy43NTczMiA2Ljc1NzM2TTkuODc4NjQgOC44Nzg2OEMxMC40MjE1IDguMzM1NzkgMTEuMTcxNSA4IDEyIDhDMTIuODI4NCA4IDEzLjU3ODQgOC4zMzU3OSAxNC4xMjEzIDguODc4NjhNOSAyMkgxNUMxNy4yMDkxIDIyIDE5IDIwLjIwOTEgMTkgMThWNkMxOSAzLjc5MDg2IDE3LjIwOTEgMiAxNSAySDlDNi43OTA4NiAyIDUgMy43OTA4NiA1IDZWMThDNSAyMC4yMDkxIDYuNzkwODYgMjIgOSAyMlpNMTYgMTVDMTYgMTcuMjA5MSAxNC4yMDkxIDE5IDEyIDE5QzkuNzkwODYgMTkgOCAxNy4yMDkxIDggMTVDOCAxMi43OTA5IDkuNzkwODYgMTEgMTIgMTFDMTQuMjA5MSAxMSAxNiAxMi43OTA5IDE2IDE1WiIvPjxjaXJjbGUgaWQ9IkVsbGlwc2UgNTQyX19fX18wXzFfTU9KWElQRExaRyIgY3g9IjEyIiBjeT0iMTUiIHI9IjEiIGZpbGw9IiMyODMwM0YiLz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Smart Speakers",
                    "TW": "智慧音響",
                    "CN": "智能扬声器",
                    "BR": "Alto-falantes inteligentes",
                    "CZ": "Chytré reproduktory",
                    "DA": "Smart-højttalere",
                    "DE": "Intelligente Lautsprecher",
                    "ES": "Altavoces inteligentes",
                    "FI": "Älykaiuttimet",
                    "FR": "Haut-parleurs intelligents",
                    "HU": "Intelligens hangszórók",
                    "IT": "Altoparlanti intelligenti",
                    "JP": "スマートスピーカー",
                    "KR": "스마트 스피커",
                    "MS": "Smart speakers",
                    "NL": "Slimme luidsprekers",
                    "NO": "Smarthøyttalere",
                    "PL": "Inteligentne głośniki",
                    "RU": "Смарт-динамики",
                    "SV": "Smarta högtalare",
                    "TH": "ลำโพงอัจฉริยะ",
                    "TR": "Akıllı hoparlörler",
                    "UK": "Розумні колонки",
                    "RO": "Difuzoare inteligente",
                    "SL": "Pametni zvočniki"
                },
                {
                    "category": "media_and_entertainment",
                    "ui-sort": 18,
                    "type": [
                        "82"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJNZWRpYSBEZXZpY2VzL0FWIGFtcGxpZmllciI+PGcgaWQ9Ikdyb3VwIDQ0MzEiPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMzcwX19fX18wXzBfS0JZWUtGQk5YWiIgY3g9IjEiIGN5PSIxIiByPSIxIiBmaWxsPSIjMjgzMDNGIiB0cmFuc2Zvcm09Im1hdHJpeCgxIDAgMCAtMSAzLjYwMDM5IDE4LjY2NjUpIi8+PGNpcmNsZSBpZD0iRWxsaXBzZSAzNzFfX19fXzBfMV9LQlBZVUFNTEhZIiBjeD0iMSIgY3k9IjEiIHI9IjEiIGZpbGw9IiMyODMwM0YiIHRyYW5zZm9ybT0ibWF0cml4KDEgMCAwIC0xIDE4LjYwMDQgMTguNjY2NSkiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzJfRllUTlBYUFRWWCIgc3Ryb2tlPSIjMjgzMDNGIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNNC43MTE1IDIxLjY2NjVINi4wNDQ4M002LjkzMzcyIDE3LjIyMjFIMTcuMDQ0OE0xOS4yNjcxIDIxLjY2NjVIMTcuOTMzN00xLjYwMDM5IDE3LjIyMjFDMS42MDAzOSAxNS4yNTg0IDMuMTkyMjcgMTMuNjY2NSA1LjE1NTk1IDEzLjY2NjVMMTguODIyNiAxMy42NjY1QzIwLjc4NjMgMTMuNjY2NSAyMi4zNzgyIDE1LjI1ODQgMjIuMzc4MiAxNy4yMjIxQzIyLjM3ODIgMTkuMTg1NyAyMC43ODYzIDIwLjc3NzYgMTguODIyNiAyMC43Nzc2TDUuMTU1OTUgMjAuNzc3NkMzLjE5MjI3IDIwLjc3NzYgMS42MDAzOSAxOS4xODU3IDEuNjAwMzkgMTcuMjIyMVoiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzNfREZPR1FDSFVDWSIgc3Ryb2tlPSIjMjgzMDNGIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTUuNjAwMzkgMTMuNjY2N1Y0LjY2NjY3QzUuNjAwMzkgMy4xOTM5MSA2Ljc5NDMgMiA4LjI2NzA2IDJIMTYuMjY3MU0xOC45MzM3IDEzLjY2NjdWNC42NjY2N0MxOC45MzM3IDMuMTkzOTEgMTcuNzM5OCAyIDE2LjI2NzEgMk0xNi4yNjcxIDJMMTMuNjAwNCA2TTUuNjAwMzkgNkgxOC45MzM3TTEwLjkzMzcgMkw4LjI2NzA2IDZNMTAuOTMzNyA4Ljg2ODUyVjEwLjQ2NDhDMTAuOTMzNyAxMS4yNjM1IDExLjgyMzkgMTEuNzM5OSAxMi40ODg0IDExLjI5NjlMMTMuNjg1NiAxMC40OTg3QzE0LjI3OTQgMTAuMTAyOSAxNC4yNzk0IDkuMjMwNDQgMTMuNjg1NiA4LjgzNDYyTDEyLjQ4ODQgOC4wMzY0N0MxMS44MjM5IDcuNTkzNDMgMTAuOTMzNyA4LjA2OTgyIDEwLjkzMzcgOC44Njg1MloiLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "AV amplifier",
                    "TW": "網路影音擴音機",
                    "CN": "影音扩音机",
                    "BR": "Amplificador AV",
                    "CZ": "Zesilovač AV",
                    "DA": "AV-forstærker",
                    "DE": "AV-Verstärker",
                    "ES": "Amplificador de AV",
                    "FI": "AV-vahvistin",
                    "FR": "Amplificateur AV",
                    "HU": "AV erősítő",
                    "IT": "Amplificatore AV",
                    "JP": "AVアンプ",
                    "KR": "AV 앰프",
                    "MS": "AV amplifier",
                    "NL": "AV-versterker",
                    "NO": "AV-forsterker",
                    "PL": "Wzmacniacz AV",
                    "RU": "Усилитель аудио/видео",
                    "SV": "AV-förstärkare",
                    "TH": "แอมป์ AV",
                    "TR": "AV yükseltici",
                    "UK": "Підсилювач AV",
                    "RO": "Amplificator AV",
                    "SL": "Ojačevalec AV"
                },
                {
                    "category": "media_and_entertainment",
                    "ui-sort": 19,
                    "type": [
                        "90"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48cGF0aCBzdHJva2U9IiMwMDAiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjUiIGQ9Ik0xMC44NTIgOS43NzdhNC4yMjMgNC4yMjMgMCAwIDAtMi45OTYtMS4yNGMtMS4xNyAwLTIuMjMuNDc0LTIuOTk2IDEuMjRtMS40OTggMS40OThjLjM4My0uMzgzLjkxMy0uNjIgMS40OTgtLjYyczEuMTE0LjIzNyAxLjQ5OC42MiIvPjxwYXRoIHN0cm9rZT0iIzAwMCIgc3Ryb2tlLXdpZHRoPSIxLjUiIGQ9Ik0xMy40ODUgMTkuNzM0VjEzLjgzbS0uNjI2LS4xODhIMi45MzhhLjkzNy45MzcgMCAwIDAtLjkzOC45Mzd2NC42MWMwIC41MTcuNDIuOTM3LjkzOC45MzdoOS45MjFNMTUuNDg1IDIyaDQuMTI1YTIgMiAwIDAgMCAyLTJWNGEyIDIgMCAwIDAtMi0yaC00LjEyNWEyIDIgMCAwIDAtMiAydjE2YTIgMiAwIDAgMCAyIDJaIi8+PHBhdGggc3Ryb2tlPSIjMDAwIiBkPSJNNi4yNjYgMTYuNjg4YTEuMjE5IDEuMjE5IDAgMSAxLTIuNDM4IDAgMS4yMTkgMS4yMTkgMCAwIDEgMi40MzggMFoiLz48cGF0aCBmaWxsPSIjMDAwIiBkPSJNMTUuOTg0IDE3LjUwNWEuNjI1LjYyNSAwIDEgMS0xLjI1IDAgLjYyNS42MjUgMCAwIDEgMS4yNSAwWm0tNy4xODctLjgxN2EuNDY5LjQ2OSAwIDEgMS0uOTM4IDAgLjQ2OS40NjkgMCAwIDEgLjkzOCAwWm0xLjcxOSAwYS40NjkuNDY5IDAgMSAxLS45MzggMCAuNDY5LjQ2OSAwIDAgMSAuOTM4IDBabTcuNjU2LjgxN2EuNjI1LjYyNSAwIDEgMS0xLjI1IDAgLjYyNS42MjUgMCAwIDEgMS4yNSAwWm0yLjE4OCAwYS42MjUuNjI1IDAgMSAxLTEuMjUgMCAuNjI1LjYyNSAwIDAgMSAxLjI1IDBaIi8+PGNpcmNsZSBjeD0iMTcuNTI0IiBjeT0iMTIuNjIyIiByPSIyLjc2OCIgc3Ryb2tlPSIjMDAwIi8+PGNpcmNsZSBjeD0iMTcuNTI0IiBjeT0iMTIuNjIyIiByPSIxLjEzNCIgc3Ryb2tlPSIjMDAwIi8+PGNpcmNsZSBjeD0iMTcuNTI0IiBjeT0iNi45MDIiIHI9IjEuMTM0IiBzdHJva2U9IiMwMDAiLz48L3N2Zz4=",
                    "EN": "TV box",
                    "TW": "電視盒",
                    "CN": "电视盒",
                    "BR": "Caixa de TV",
                    "CZ": "TV box",
                    "DA": "TV-boks",
                    "DE": "TV-Box",
                    "ES": "Televisor",
                    "FI": "Mediatoistin",
                    "FR": "Boîtier TV",
                    "HU": "Médiabox",
                    "IT": "TV box",
                    "JP": "TV ボックス",
                    "KR": "TV 박스",
                    "MS": "TV box",
                    "NL": "Tv-box",
                    "NO": "TV-boks",
                    "PL": "Dekoder TV",
                    "RU": "ТВ-приставка",
                    "SV": "TV-box",
                    "TH": "กล่องรับสัญญาณโทรทัศน์",
                    "TR": "TV kutusu",
                    "UK": "Телевізійна приставка",
                    "RO": "TV box",
                    "SL": "TV-komunikator"
                },
                {
                    "category": "media_and_entertainment",
                    "ui-sort": 20,
                    "type": [
                        "128"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJNZWRpYSAmYW1wOyBFbnRlcnRhaW5tZW50L1JPRyBBbGx5Ij48ZyBpZD0iR3JvdXAgNDU4NCI+PGcgaWQ9IkFzc2V0IDEgMSI+PGcgaWQ9IkxheWVyIDIiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMF9FQkRCWU1UVkRaIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0ic3F1YXJlIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjAuOCIgZD0iTTEuNTk0NzcgOS4wNzc3NUMxLjU5NDc3IDkuMDc3NzUgMS40NTExNCAxMC4wMzI5IDEuMzkzODUgMTAuNzM0MkMxLjI3NzEyIDEyLjE1MDkgMS4wMzM4NyAxMy44MjI4IDEuMDY5NTMgMTQuMjgzOUMxLjEyMTQyIDE0Ljk2NzkgMS43NzY0MSAxNi40NiAyLjE4Mzg5IDE2LjQ2QzIuMjgzMzIgMTYuNDYgMy44NzE3IDE2LjM0MzggNC40NjI5MiAxNi4zMDkzIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8xX0ZBUFVNQU5LRUoiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIwLjgiIGQ9Ik0yMi40MDMzIDguOTcxOUMyMi40MDMzIDkuMTA1NDggMjIuNTY1OSAxMC4wMjQ4IDIyLjYyMzIgMTAuNzI2MUMyMi43Mzk5IDEyLjE0MjggMjIuOTgzMiAxMy44MTQ3IDIyLjk0NzUgMTQuMjc1N0MyMi44OTU3IDE0Ljk1OTggMjIuMjQwNyAxNi40NTE5IDIxLjgzMzIgMTYuNDUxOUMyMS43MzM3IDE2LjQ1MTkgMTkuNTM0MyAxNi4zMzU3IDE4Ljk0MyAxNi4zMDEyIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8yX0JSVU9JVlhUUU8iIGZpbGw9ImJsYWNrIiBkPSJNMjIuODc2NiA4LjMzODAyVjguMzYwNDJIMjIuODg5NkwyMi44NzY2IDguMzM4MDJaIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8zX0xVUkxPV1hERFEiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJzcXVhcmUiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS1taXRlcmxpbWl0PSIxMCIgc3Ryb2tlLXdpZHRoPSIwLjgiIGQ9Ik0yMi4xMjggOC43MTI2N0MyMi4wNTY3IDguNjQ1MyAyMS40MDM5IDcuOTM5NjIgMjEuMzcxNCA3LjkxNjQyQzIxLjMwNyA3Ljg3MTIzIDIxLjIzNjIgNy44MzY5NSAyMS4xNjE3IDcuODE0ODJIMi44NDEzNkMyLjczOTI1IDcuODM4OTMgMi42NDM1NyA3Ljg4Njg2IDIuNTYxMzggNy45NTUwOEMyLjQ0OTE4IDguMDU2NjggMS44MjQ0NCA4LjY4ODA4IDEuNzc0MTEgOC43MzU1Nk01LjA0NTU1IDE2LjI4ODlIMTEuOTk5NEgxOC45NTMzIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF80X1pDRVZUUUhCS1oiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLW1pdGVybGltaXQ9IjEwIiBzdHJva2Utd2lkdGg9IjAuOCIgZD0iTTYuMTI4OTYgOC43ODUyMUgxNy42MzI3VjE0Ljk5MjZINi4xMjg5NlY4Ljc4NTIxWiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfNV9QUVZDSFRPRElLIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS1taXRlcmxpbWl0PSIxMCIgc3Ryb2tlLXdpZHRoPSIwLjUiIGQ9Ik0zLjkwMzczIDEwLjU4MDlDMy43NDE4NiAxMC41ODA5IDMuNTgzNjMgMTAuNTMyOSAzLjQ0OTA1IDEwLjQ0MjlDMy4zMTQ0OCAxMC4zNTMgMy4yMDk2MSAxMC4yMjUxIDMuMTQ3NzEgMTAuMDc1NkMzLjA4NTgxIDkuOTI1OTkgMy4wNjk2NyA5Ljc2MTQyIDMuMTAxMzMgOS42MDI2OEMzLjEzMjk5IDkuNDQzOTMgMy4yMTEwMiA5LjI5ODE0IDMuMzI1NTYgOS4xODM3NkMzLjQ0MDA5IDkuMDY5MzggMy41ODU5OCA4Ljk5MTU0IDMuNzQ0NzcgOC45NjAwOUMzLjkwMzU2IDguOTI4NjQgNC4wNjgxIDguOTQ1IDQuMjE3NTkgOS4wMDcwOUM0LjM2NzA4IDkuMDY5MTggNC40OTQ3OSA5LjE3NDIyIDQuNTg0NTYgOS4zMDg5MkM0LjY3NDM0IDkuNDQzNjEgNC43MjIxNCA5LjYwMTkxIDQuNzIxOTMgOS43NjM3OEM0LjcyMTY0IDkuOTgwNTkgNC42MzUzMSAxMC4xODg0IDQuNDgxOSAxMC4zNDE2QzQuMzI4NDkgMTAuNDk0OCA0LjEyMDU0IDEwLjU4MDkgMy45MDM3MyAxMC41ODA5WiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfNl9JVEZKUUJaTEZKIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS1taXRlcmxpbWl0PSIxMCIgc3Ryb2tlLXdpZHRoPSIwLjUiIGQ9Ik00LjQwODE4IDEyLjc3ODVDNC4yOTU3NyAxMi43Nzg1IDQuMTg1ODkgMTIuNzQ1MSA0LjA5MjQzIDEyLjY4MjZDMy45OTg5OCAxMi42MjAyIDMuOTI2MTUgMTIuNTMxNCAzLjg4MzE2IDEyLjQyNzVDMy44NDAxOCAxMi4zMjM2IDMuODI4OTcgMTIuMjA5NCAzLjg1MDk2IDEyLjA5OTFDMy44NzI5NCAxMS45ODg5IDMuOTI3MTMgMTEuODg3NiA0LjAwNjY3IDExLjgwODJDNC4wODYyMSAxMS43Mjg4IDQuMTg3NTMgMTEuNjc0NyA0LjI5Nzc5IDExLjY1MjlDNC40MDgwNiAxMS42MzEgNC41MjIzMyAxMS42NDI0IDQuNjI2MTQgMTEuNjg1NUM0LjcyOTk2IDExLjcyODYgNC44MTg2NSAxMS44MDE2IDQuODgwOTkgMTEuODk1MUM0Ljk0MzMzIDExLjk4ODcgNC45NzY1MyAxMi4wOTg2IDQuOTc2MzggMTIuMjExQzQuOTc2MTggMTIuMzYxNiA0LjkxNjIzIDEyLjUwNTkgNC44MDk3IDEyLjYxMjNDNC43MDMxNiAxMi43MTg3IDQuNTU4NzUgMTIuNzc4NSA0LjQwODE4IDEyLjc3ODVaIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF84X0ZDRUVLVEpHRkwiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLW1pdGVybGltaXQ9IjEwIiBzdHJva2Utd2lkdGg9IjAuNSIgZD0iTTE5LjY5MTMgMTEuNDAyOUMxOS44NTMyIDExLjQwMjkgMjAuMDExNCAxMS40NTA5IDIwLjE0NiAxMS41NDA5QzIwLjI4MDYgMTEuNjMwOCAyMC4zODU0IDExLjc1ODcgMjAuNDQ3MyAxMS45MDgyQzIwLjUwOTIgMTIuMDU3OCAyMC41MjU0IDEyLjIyMjQgMjAuNDkzNyAxMi4zODExQzIwLjQ2MjEgMTIuNTM5OSAyMC4zODQgMTIuNjg1NyAyMC4yNjk1IDEyLjhDMjAuMTU0OSAxMi45MTQ0IDIwLjAwOTEgMTIuOTkyMyAxOS44NTAzIDEzLjAyMzdDMTkuNjkxNSAxMy4wNTUyIDE5LjUyNjkgMTMuMDM4OCAxOS4zNzc0IDEyLjk3NjdDMTkuMjI4IDEyLjkxNDYgMTkuMTAwMyAxMi44MDk2IDE5LjAxMDUgMTIuNjc0OUMxOC45MjA3IDEyLjU0MDIgMTguODcyOSAxMi4zODE5IDE4Ljg3MzEgMTIuMjJDMTguODczNCAxMi4wMDMyIDE4Ljk1OTcgMTEuNzk1NCAxOS4xMTMxIDExLjY0MjJDMTkuMjY2NiAxMS40ODkgMTkuNDc0NSAxMS40MDI5IDE5LjY5MTMgMTEuNDAyOVoiLz48ZyBpZD0iR3JvdXAgNDU4MyI+PGcgaWQ9IlZlY3Rvcl9fX19fMF85X1RFSE1FWlhWQUwiPjxtYXNrIGlkPSJwYXRoLTktaW5zaWRlLTFfMzE1XzM3OTIiIGZpbGw9IndoaXRlIj48cGF0aCBkPSJNMjAuMjI3NSAxMS4wNTE2QzIwLjE0NTEgMTEuMDUxNiAyMC4wNjQ0IDExLjAyNzEgMTkuOTk1OSAxMC45ODEyQzE5LjkyNzQgMTAuOTM1MyAxOS44NzQxIDEwLjg3IDE5Ljg0MjcgMTAuNzkzN0MxOS44MTEzIDEwLjcxNzQgMTkuODAzMyAxMC42MzM2IDE5LjgxOTggMTAuNTUyN0MxOS44MzYyIDEwLjQ3MTkgMTkuODc2MiAxMC4zOTc3IDE5LjkzNDkgMTAuMzM5N0MxOS45OTM1IDEwLjI4MTcgMjAuMDY4IDEwLjI0MjQgMjAuMTQ5IDEwLjIyNjlDMjAuMjMgMTAuMjExMyAyMC4zMTM4IDEwLjIyMDIgMjAuMzg5OCAxMC4yNTIzQzIwLjQ2NTggMTAuMjg0NSAyMC41MzA0IDEwLjMzODUgMjAuNTc1NiAxMC40MDc1QzIwLjYyMDggMTAuNDc2NSAyMC42NDQ1IDEwLjU1NzMgMjAuNjQzNyAxMC42Mzk4QzIwLjY0MzcgMTAuNjk0NSAyMC42MzI5IDEwLjc0ODYgMjAuNjEyIDEwLjc5OTFDMjAuNTkxMSAxMC44NDk2IDIwLjU2MDQgMTAuODk1NCAyMC41MjE4IDEwLjkzNDFDMjAuNDgzMSAxMC45NzI3IDIwLjQzNzMgMTEuMDAzNCAyMC4zODY4IDExLjAyNDNDMjAuMzM2MyAxMS4wNDUyIDIwLjI4MjIgMTEuMDU2IDIwLjIyNzUgMTEuMDU2Ii8+PC9tYXNrPjxwYXRoIGZpbGw9ImJsYWNrIiBkPSJNMjAuMjI3NSAxMS4wNTE2QzIwLjE0NTEgMTEuMDUxNiAyMC4wNjQ0IDExLjAyNzEgMTkuOTk1OSAxMC45ODEyQzE5LjkyNzQgMTAuOTM1MyAxOS44NzQxIDEwLjg3IDE5Ljg0MjcgMTAuNzkzN0MxOS44MTEzIDEwLjcxNzQgMTkuODAzMyAxMC42MzM2IDE5LjgxOTggMTAuNTUyN0MxOS44MzYyIDEwLjQ3MTkgMTkuODc2MiAxMC4zOTc3IDE5LjkzNDkgMTAuMzM5N0MxOS45OTM1IDEwLjI4MTcgMjAuMDY4IDEwLjI0MjQgMjAuMTQ5IDEwLjIyNjlDMjAuMjMgMTAuMjExMyAyMC4zMTM4IDEwLjIyMDIgMjAuMzg5OCAxMC4yNTIzQzIwLjQ2NTggMTAuMjg0NSAyMC41MzA0IDEwLjMzODUgMjAuNTc1NiAxMC40MDc1QzIwLjYyMDggMTAuNDc2NSAyMC42NDQ1IDEwLjU1NzMgMjAuNjQzNyAxMC42Mzk4QzIwLjY0MzcgMTAuNjk0NSAyMC42MzI5IDEwLjc0ODYgMjAuNjEyIDEwLjc5OTFDMjAuNTkxMSAxMC44NDk2IDIwLjU2MDQgMTAuODk1NCAyMC41MjE4IDEwLjkzNDFDMjAuNDgzMSAxMC45NzI3IDIwLjQzNzMgMTEuMDAzNCAyMC4zODY4IDExLjAyNDNDMjAuMzM2MyAxMS4wNDUyIDIwLjI4MjIgMTEuMDU2IDIwLjIyNzUgMTEuMDU2Ii8+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik0yMC4yMjc1IDEyLjU1MTZDMjEuMDU1OSAxMi41NTE3IDIxLjcyNzUgMTEuODgwMSAyMS43Mjc1IDExLjA1MTdDMjEuNzI3NiAxMC4yMjMzIDIxLjA1NiA5LjU1MTY4IDIwLjIyNzYgOS41NTE2M0wyMC4yMjc1IDEyLjU1MTZaTTIwLjY0MzcgMTAuNjM5OEwxOS4xNDM3IDEwLjYyNDJDMTkuMTQzNyAxMC42Mjk0IDE5LjE0MzcgMTAuNjM0NiAxOS4xNDM3IDEwLjYzOThMMjAuNjQzNyAxMC42Mzk4Wk0yMC4yMjc1IDkuNTU1OTZDMTkuMzk5MSA5LjU1NTk2IDE4LjcyNzUgMTAuMjI3NSAxOC43Mjc1IDExLjA1NkMxOC43Mjc1IDExLjg4NDQgMTkuMzk5MSAxMi41NTYgMjAuMjI3NSAxMi41NTZWOS41NTU5NlpNMjAuMjI3NiA5LjU1MTYzQzIwLjQ0MjUgOS41NTE2NCAyMC42NTI1IDkuNjE1NTEgMjAuODMwOSA5LjczNTExTDE5LjE2MDkgMTIuMjI3M0MxOS40NzY0IDEyLjQzODcgMTkuODQ3NiAxMi41NTE2IDIwLjIyNzUgMTIuNTUxNkwyMC4yMjc2IDkuNTUxNjNaTTIwLjgzMDkgOS43MzUxMUMyMS4wMDk0IDkuODU0NzEgMjEuMTQ4MyAxMC4wMjQ3IDIxLjIzIDEwLjIyMzRMMTguNDU1NCAxMS4zNjQxQzE4LjU5OTggMTEuNzE1NCAxOC44NDU0IDEyLjAxNTggMTkuMTYwOSAxMi4yMjczTDIwLjgzMDkgOS43MzUxMVpNMjEuMjMgMTAuMjIzNEMyMS4zMTE3IDEwLjQyMjEgMjEuMzMyNSAxMC42NDA2IDIxLjI4OTggMTAuODUxMUwxOC4zNDk3IDEwLjI1NDNDMTguMjc0MiAxMC42MjY1IDE4LjMxMDkgMTEuMDEyOCAxOC40NTU0IDExLjM2NDFMMjEuMjMgMTAuMjIzNFpNMjEuMjg5OCAxMC44NTExQzIxLjI0NyAxMS4wNjE3IDIxLjE0MjcgMTEuMjU0OCAyMC45OSAxMS40MDU5TDE4Ljg3OTcgOS4yNzM1NUMxOC42MDk4IDkuNTQwNzIgMTguNDI1MyA5Ljg4MjA5IDE4LjM0OTcgMTAuMjU0M0wyMS4yODk4IDEwLjg1MTFaTTIwLjk5IDExLjQwNTlDMjAuODM3MiAxMS41NTcgMjAuNjQzMSAxMS42NTk0IDIwLjQzMjEgMTEuNjk5OUwxOS44NjYgOC43NTM4QzE5LjQ5MyA4LjgyNTQ3IDE5LjE0OTcgOS4wMDYzOCAxOC44Nzk3IDkuMjczNTVMMjAuOTkgMTEuNDA1OVpNMjAuNDMyMSAxMS42OTk5QzIwLjIyMTEgMTEuNzQwNSAyMC4wMDI4IDExLjcxNzQgMTkuODA1IDExLjYzMzZMMjAuOTc0NiA4Ljg3MTAyQzIwLjYyNDkgOC43MjI5MyAyMC4yMzkgOC42ODIxMyAxOS44NjYgOC43NTM4TDIwLjQzMjEgMTEuNjk5OVpNMTkuODA1IDExLjYzMzZDMTkuNjA3MSAxMS41NDk4IDE5LjQzODYgMTEuNDA5MiAxOS4zMjA5IDExLjIyOTVMMjEuODMwNCA5LjU4NTVDMjEuNjIyMiA5LjI2Nzc5IDIxLjMyNDQgOS4wMTkxIDIwLjk3NDYgOC44NzEwMkwxOS44MDUgMTEuNjMzNlpNMTkuMzIwOSAxMS4yMjk1QzE5LjIwMzIgMTEuMDQ5NyAxOS4xNDE1IDEwLjgzOTEgMTkuMTQzNyAxMC42MjQyTDIyLjE0MzYgMTAuNjU1NEMyMi4xNDc1IDEwLjI3NTYgMjIuMDM4NSA5LjkwMzIyIDIxLjgzMDQgOS41ODU1TDE5LjMyMDkgMTEuMjI5NVpNMTkuMTQzNyAxMC42Mzk4QzE5LjE0MzcgMTAuNDk3NSAxOS4xNzE3IDEwLjM1NjYgMTkuMjI2MiAxMC4yMjVMMjEuOTk3OCAxMS4zNzMxQzIyLjA5NDEgMTEuMTQwNiAyMi4xNDM3IDEwLjg5MTUgMjIuMTQzNyAxMC42Mzk4SDE5LjE0MzdaTTE5LjIyNjIgMTAuMjI1QzE5LjI4MDYgMTAuMDkzNSAxOS4zNjA1IDkuOTc0MDYgMTkuNDYxMSA5Ljg3MzQyTDIxLjU4MjQgMTEuOTk0N0MyMS43NjA0IDExLjgxNjggMjEuOTAxNSAxMS42MDU2IDIxLjk5NzggMTEuMzczMUwxOS4yMjYyIDEwLjIyNVpNMTkuNDYxMSA5Ljg3MzQyQzE5LjU2MTggOS43NzI3NyAxOS42ODEyIDkuNjkyOTMgMTkuODEyOCA5LjYzODQ2TDIwLjk2MDggMTIuNDEwMUMyMS4xOTMzIDEyLjMxMzggMjEuNDA0NSAxMi4xNzI3IDIxLjU4MjQgMTEuOTk0N0wxOS40NjExIDkuODczNDJaTTE5LjgxMjggOS42Mzg0NkMxOS45NDQzIDkuNTgzOTkgMjAuMDg1MiA5LjU1NTk2IDIwLjIyNzUgOS41NTU5NlYxMi41NTZDMjAuNDc5MiAxMi41NTYgMjAuNzI4MyAxMi41MDY0IDIwLjk2MDggMTIuNDEwMUwxOS44MTI4IDkuNjM4NDZaIiBtYXNrPSJ1cmwoI3BhdGgtOS1pbnNpZGUtMV8zMTVfMzc5MikiLz48L2c+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8xMF9QRUVQV1lQRUZBIiBmaWxsPSJibGFjayIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2UtbWl0ZXJsaW1pdD0iMTAiIHN0cm9rZS13aWR0aD0iMS41IiBkPSJNMjAuNzI2IDkuODAzMjlDMjAuNzI2IDkuNzgxNTIgMjAuNzI4MSA5Ljc1OTcgMjAuNzMyNCA5LjczODE1QzIwLjc0NTMgOS42NzMzOCAyMC43NzcxIDkuNjEzODkgMjAuODIzOCA5LjU2NzJDMjAuODcwNSA5LjUyMDUxIDIwLjkzIDkuNDg4NzEgMjAuOTk0NyA5LjQ3NTgzQzIxLjAxNjMgOS40NzE1NCAyMS4wMzgxIDkuNDY5NDEgMjEuMDU5OSA5LjQ2OTQxVjkuODAzMjlNMjAuNzI2IDkuODAzMjlMMjEuMDU5OSA5LjgwMzI4TDIxLjA1OTkgOS44MDMyOU0yMC43MjYgOS44MDMyOUwyMS4wNTk5IDkuODAzMjlNMjAuNzI2IDkuODAzMjlMMjEuMDU5OSA5LjgwMzI5TTIxLjA1OTkgOS44MDMyOUwyMS4wNjEgOS44MDI4NE0yMS4wNTk5IDkuODAzMjlMMjEuMDYxIDkuODAyODRNMjEuMDYxIDkuODAyODRMMjEuMDYxIDkuODAzMjlIMjEuMzkzOEMyMS4zOTM4IDkuNzU5MjMgMjEuMzg1IDkuNzE1ODMgMjEuMzY4MyA5LjY3NTUyQzIxLjM2IDkuNjU1NDEgMjEuMzQ5NyA5LjYzNjA3IDIxLjMzNzUgOS42MTc3OUMyMS4zMTMgOS41ODExNiAyMS4yODE3IDkuNTQ5OTIgMjEuMjQ1NCA5LjUyNTY4QzIxLjIyNzMgOS41MTM1OSAyMS4yMDggOS41MDMyNCAyMS4xODc2IDkuNDk0ODNDMjEuMTQ3IDkuNDc3OTkgMjEuMTAzNyA5LjQ2OTQ0IDIxLjA2MDEgOS40Njk0MUwyMS4wNjEgOS44MDA3TDIxLjA2MSA5LjgwMTY4TDIxLjA2MSA5LjgwMjU3TDIxLjA2MSA5LjgwMjg0Wk0yMS4wNTk5IDEwLjEzNzJDMjEuMDE1OCAxMC4xMzcyIDIwLjk3MjQgMTAuMTI4NCAyMC45MzIxIDEwLjExMTdDMjAuOTEyIDEwLjEwMzQgMjAuODkyNyAxMC4wOTMxIDIwLjg3NDQgMTAuMDgwOUMyMC44Mzc4IDEwLjA1NjQgMjAuODA2NSAxMC4wMjUxIDIwLjc4MjMgOS45ODg3OEMyMC43NzAyIDkuOTcwNjggMjAuNzU5OCA5Ljk1MTM2IDIwLjc1MTQgOS45MzEwNUMyMC43MzQ2IDkuODkwNDIgMjAuNzI2IDkuODQ3MDYgMjAuNzI2IDkuODAzNUwyMS4wNTczIDkuODA0MzZMMjEuMDU4MyA5LjgwNDM2TDIxLjA1OTIgOS44MDQzNkwyMS4wNTk0IDkuODA0MzZMMjEuMDU5OSA5LjgwNDM2VjEwLjEzNzJaTTIxLjA2MSA5LjgwNDM3SDIxLjA2MUgyMS4wNjFMMjEuMDY1NCA5LjgwNDM4TDIxLjM5MzggOS44MDUyM0MyMS4zOTM3IDkuODI2NjEgMjEuMzkxNiA5Ljg0Nzc3IDIxLjM4NzUgOS44Njg0NkMyMS4zNzQ5IDkuOTMyODcgMjEuMzQzNCA5Ljk5MjYxIDIxLjI5NjMgMTAuMDM5N0MyMS4yNDkyIDEwLjA4NjggMjEuMTg5NSAxMC4xMTgzIDIxLjEyNTEgMTAuMTMwOUMyMS4xMDQ0IDEwLjEzNSAyMS4wODMyIDEwLjEzNzEgMjEuMDYxOCAxMC4xMzcyTDIxLjA2MSA5LjgwODc3TDIxLjA2MSA5LjgwNDM3WiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMTFfWVNCQldWQk1PSiIgZmlsbD0iYmxhY2siIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLW1pdGVybGltaXQ9IjEwIiBzdHJva2Utd2lkdGg9IjEuNSIgZD0iTTIwLjIyNzYgOS4zMzYxN0MyMC4xODM1IDkuMzM2MTcgMjAuMTQwMSA5LjMyNzQ1IDIwLjA5OTggOS4zMTA3NkMyMC4wNzk3IDkuMzAyNDMgMjAuMDYwMyA5LjI5MjExIDIwLjA0MjEgOS4yNzk5QzIwLjAwNTQgOS4yNTU0MyAxOS45NzQyIDkuMjI0MDcgMTkuOTUgOS4xODc3OUMxOS45Mzc5IDkuMTY5NjkgMTkuOTI3NSA5LjE1MDM3IDE5LjkxOTEgOS4xMzAwNkMxOS45MDIyIDkuMDg5MzYgMTkuODkzNyA5LjA0NTkzIDE5Ljg5MzcgOS4wMDIzQzE5Ljg5MzcgOC45ODA1MyAxOS44OTU4IDguOTU4NzEgMTkuOTAwMSA4LjkzNzE2QzE5LjkxMyA4Ljg3MjM5IDE5Ljk0NDggOC44MTI5IDE5Ljk5MTUgOC43NjYyMUMyMC4wMzgyIDguNzE5NTIgMjAuMDk3NyA4LjY4NzcyIDIwLjE2MjQgOC42NzQ4NEMyMC4xODQgOC42NzA1NSAyMC4yMDU4IDguNjY4NDIgMjAuMjI3NiA4LjY2ODQyQzIwLjI3MTIgOC42Njg0MiAyMC4zMTQ2IDguNjc2OTggMjAuMzU1MyA4LjY5Mzg0QzIwLjM3NTYgOC43MDIyNSAyMC4zOTUgOC43MTI2IDIwLjQxMzEgOC43MjQ2OUMyMC40NDkzIDguNzQ4OTMgMjAuNDgwNyA4Ljc4MDE3IDIwLjUwNTIgOC44MTY4MUMyMC41MTc0IDguODM1MDggMjAuNTI3NyA4Ljg1NDQyIDIwLjUzNiA4Ljg3NDUzQzIwLjU1MjcgOC45MTQ4NCAyMC41NjE0IDguOTU4MjQgMjAuNTYxNCA5LjAwMjNDMjAuNTYxNCA5LjAyNDMzIDIwLjU1OTMgOS4wNDYxNCAyMC41NTUgOS4wNjc0M0MyMC41NDIyIDkuMTMxNzIgMjAuNTEwNyA5LjE5MTM1IDIwLjQ2MzYgOS4yMzgzOEMyMC40MTY2IDkuMjg1NDIgMjAuMzU3IDkuMzE2OTcgMjAuMjkyNyA5LjMyOTc2QzIwLjI3MTQgOS4zMzM5OSAyMC4yNDk2IDkuMzM2MTcgMjAuMjI3NiA5LjMzNjE3Wk0yMC4yMjc2IDkuMDAyM0wyMC4yMjc2IDkuMDAyM0wyMC4yMjc2IDkuMDAyM1oiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzEyX1pKRVFXWUdWUU4iIGZpbGw9ImJsYWNrIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS1taXRlcmxpbWl0PSIxMCIgc3Ryb2tlLXdpZHRoPSIxLjUiIGQ9Ik0xOS4wODk2IDkuODAzMjlDMTkuMDg5NiA5Ljc4MTUyIDE5LjA5MTcgOS43NTk3IDE5LjA5NiA5LjczODE1QzE5LjEwODkgOS42NzMzOCAxOS4xNDA3IDkuNjEzODkgMTkuMTg3NCA5LjU2NzJDMTkuMjM0MSA5LjUyMDUxIDE5LjI5MzYgOS40ODg3MSAxOS4zNTg0IDkuNDc1ODNDMTkuMzc5OSA5LjQ3MTU0IDE5LjQwMTcgOS40Njk0MSAxOS40MjM1IDkuNDY5NDFWOS44MDMyOU0xOS4wODk2IDkuODAzMjlMMTkuNDIzNSA5LjgwMzI4TDE5LjQyMzUgOS44MDMyOU0xOS4wODk2IDkuODAzMjlMMTkuNDIzNSA5LjgwMzI5TTE5LjA4OTYgOS44MDMyOUwxOS40MjM1IDkuODAzMjlNMTkuNDIzNSA5LjgwMzI5TDE5LjQyNDYgOS44MDI4NE0xOS40MjM1IDkuODAzMjlMMTkuNDI0NiA5LjgwMjg0TTE5LjQyNDYgOS44MDI4NEwxOS40MjQ2IDkuODAzMjlIMTkuNzU3NEMxOS43NTc0IDkuNzU5MjMgMTkuNzQ4NiA5LjcxNTgzIDE5LjczMTkgOS42NzU1MkMxOS43MjM2IDkuNjU1NDEgMTkuNzEzMyA5LjYzNjA3IDE5LjcwMTEgOS42MTc3OUMxOS42NzY2IDkuNTgxMTYgMTkuNjQ1MyA5LjU0OTkyIDE5LjYwOSA5LjUyNTY4QzE5LjU5MDkgOS41MTM1OSAxOS41NzE2IDkuNTAzMjQgMTkuNTUxMyA5LjQ5NDgzQzE5LjUxMDYgOS40Nzc5OSAxOS40NjczIDkuNDY5NDQgMTkuNDIzNyA5LjQ2OTQxTDE5LjQyNDYgOS44MDA3TDE5LjQyNDYgOS44MDE2OEwxOS40MjQ2IDkuODAyNTdMMTkuNDI0NiA5LjgwMjg0Wk0xOS40MjM1IDEwLjEzNzJDMTkuMzc5NCAxMC4xMzcyIDE5LjMzNiAxMC4xMjg0IDE5LjI5NTcgMTAuMTExN0MxOS4yNzU2IDEwLjEwMzQgMTkuMjU2MyAxMC4wOTMxIDE5LjIzOCAxMC4wODA5QzE5LjIwMTQgMTAuMDU2NCAxOS4xNzAxIDEwLjAyNTEgMTkuMTQ1OSA5Ljk4ODc4QzE5LjEzMzggOS45NzA2OCAxOS4xMjM0IDkuOTUxMzYgMTkuMTE1IDkuOTMxMDVDMTkuMDk4MiA5Ljg5MDQyIDE5LjA4OTYgOS44NDcwNiAxOS4wODk2IDkuODAzNUwxOS40MjA5IDkuODA0MzZMMTkuNDIxOSA5LjgwNDM2TDE5LjQyMjggOS44MDQzNkwxOS40MjMgOS44MDQzNkwxOS40MjM1IDkuODA0MzZWMTAuMTM3MlpNMTkuNDI0NiA5LjgwNDM3SDE5LjQyNDZIMTkuNDI0NkwxOS40MjkgOS44MDQzOEwxOS43NTc0IDkuODA1MjNDMTkuNzU3MyA5LjgyNjYxIDE5Ljc1NTIgOS44NDc3NyAxOS43NTExIDkuODY4NDZDMTkuNzM4NSA5LjkzMjg3IDE5LjcwNyA5Ljk5MjYxIDE5LjY1OTkgMTAuMDM5N0MxOS42MTI4IDEwLjA4NjggMTkuNTUzMSAxMC4xMTgzIDE5LjQ4ODcgMTAuMTMwOUMxOS40NjggMTAuMTM1IDE5LjQ0NjggMTAuMTM3MSAxOS40MjU0IDEwLjEzNzJMMTkuNDI0NiA5LjgwODc3TDE5LjQyNDYgOS44MDQzN1oiLz48L2c+PC9nPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMTNfV0tDRkhQVEpQWCIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InNxdWFyZSIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLW1pdGVybGltaXQ9IjEwIiBzdHJva2Utd2lkdGg9IjAuOCIgZD0iTTUuMDc0MDcgNy41Nzc0Mkw0Ljc0MjI0IDcuMDAzMjRDNC43NDIyNCA3LjAwMzI0IDMuNjUxNiA3LjAwMzI0IDMuMzA0MjcgNy4wMDMyNEMzLjAwNjcyIDcuMDAzMjQgMi41ODMwOCA2Ljk0NzI2IDIuMjc0NDcgNy4zMDA3M0MxLjk5MTMgNy42MDc4MSAxLjgxNDgxIDguNjI5NjMgMS44MTQ4MSA4LjYyOTYzIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8xNF9XVllGVlNWUVhZIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0ic3F1YXJlIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2UtbWl0ZXJsaW1pdD0iMTAiIHN0cm9rZS13aWR0aD0iMC44IiBkPSJNMTguOTI1OSA3LjU3NzQyTDE5LjI1NzggNy4wMDMyNEMxOS4yNTc4IDcuMDAzMjQgMjAuMzQ4NCA3LjAwMzI0IDIwLjY5NTcgNy4wMDMyNEMyMC45OTMzIDcuMDAzMjQgMjEuNDE2OSA2Ljk0NzI2IDIxLjcyNTUgNy4zMDA3M0MyMi4wMDg3IDcuNjA3ODEgMjIuMTg1MiA4LjYyOTYzIDIyLjE4NTIgOC42Mjk2MyIvPjwvZz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzE1X1lHV1lQRktCSEoiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJzcXVhcmUiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS1taXRlcmxpbWl0PSIxMCIgc3Ryb2tlLXdpZHRoPSIwLjUiIGQ9Ik0yLjk1MDk4IDcuODE0ODlDMi44NDU3MiA3LjgzODQ4IDIuNzQ3MTEgNy44ODUzOSAyLjY2MjM5IDcuOTUyMTZDMi41NDY3NCA4LjA1MTU5IDEuOTQ0NzEgOC42NDcxNCAxLjg5MjgzIDguNjkzNjJDMS44Njc5NyA4Ljc3MzYgMS43NzgyNiA5LjI5OTk3IDEuODMxMjIgOS41OTI4OEMxLjg4NzQyIDkuOTA4NDkgMi42MjM1NSAxMy41MzU5IDIuODAyOTcgMTQuMDU2OEMzLjAxMTU3IDE0LjY2NjQgMy4yMTQ3NyAxNS4wNTc3IDMuNDI0NDYgMTUuMjUyM0MzLjgzMTk0IDE1LjYyODQgNC45OTI1NSAxNi4zNzA0IDQuOTkyNTUgMTYuMzcwNCIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMTZfSlNZV0ZQTU9NQiIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InNxdWFyZSIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLW1pdGVybGltaXQ9IjEwIiBzdHJva2Utd2lkdGg9IjAuNSIgZD0iTTIwLjk2NzUgNy44MTQ4OUMyMS4wNzI4IDcuODM4NDggMjEuMTcxNCA3Ljg4NTM5IDIxLjI1NjEgNy45NTIxNkMyMS4zNzE4IDguMDUxNTkgMjEuOTczOCA4LjY0NzE0IDIyLjAyNTcgOC42OTM2MkMyMi4wNTA1IDguNzczNiAyMi4xNDAyIDkuMjk5OTcgMjIuMDg3MyA5LjU5Mjg4QzIyLjAzMTEgOS45MDg0OSAyMS4yOTQ5IDEzLjUzNTkgMjEuMTE1NSAxNC4wNTY4QzIwLjkwNjkgMTQuNjY2NCAyMC43MDM3IDE1LjA1NzcgMjAuNDk0IDE1LjI1MjNDMjAuMDg2NiAxNS42Mjg0IDE4LjkyNiAxNi4zNzA0IDE4LjkyNiAxNi4zNzA0Ii8+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "ROG Ally",
                    "TW": "ROG Ally",
                    "CN": "ROG Ally",
                    "BR": "ROG Ally",
                    "CZ": "ROG Ally",
                    "DA": "ROG Ally",
                    "DE": "ROG Ally",
                    "ES": "ROG Ally",
                    "FI": "ROG Ally",
                    "FR": "ROG Ally",
                    "HU": "ROG Ally",
                    "IT": "ROG Ally",
                    "JP": "ROG Ally",
                    "KR": "ROG Ally",
                    "MS": "ROG Ally",
                    "NL": "ROG Ally",
                    "NO": "ROG Ally",
                    "PL": "ROG Ally",
                    "RU": "ROG Ally",
                    "SV": "ROG Ally",
                    "TH": "ROG Ally",
                    "TR": "ROG Ally",
                    "UK": "ROG Ally",
                    "RO": "ROG Ally",
                    "SL": "ROG Ally"
                },
                {
                    "category": "media_and_entertainment",
                    "ui-sort": 21,
                    "type": [
                        "129"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJNZWRpYSAmYW1wOyBFbnRlcnRhaW5tZW50L1N0ZWFtIERlY2siPjxnIGlkPSJHcm91cCA0NTc0Ij48cGF0aCBpZD0iUmVjdGFuZ2xlIDEwMjY0X19fX18wXzBfWVVLWlhCWURTTyIgc3Ryb2tlPSJibGFjayIgZD0iTTYuMTk0MTIgNy41VjE2LjU5NDFINUMzLjA2NyAxNi41OTQxIDEuNSAxNS4wMjcxIDEuNSAxMy4wOTQxVjlDMS41IDguMTcxNTcgMi4xNzE1NyA3LjUgMyA3LjVINi4xOTQxMlpNNy4xOTQxMiAxNi41OTQxVjcuNUgxNi45MzUzVjE2LjU5NDFINy4xOTQxMlpNMTcuOTM1MyAxNi41OTQxVjcuNUgyMUMyMS44Mjg0IDcuNSAyMi41IDguMTcxNTcgMjIuNSA5VjEzLjA5NDFDMjIuNSAxNS4wMjcxIDIwLjkzMyAxNi41OTQxIDE5IDE2LjU5NDFIMTcuOTM1M1oiLz48Y2lyY2xlIGlkPSJFbGxpcHNlIDEzMDhfX19fXzBfMV9BRlVOR05WV1JMIiBjeD0iNC4xMDU3MyIgY3k9IjEwLjM2NDciIHI9IjEuMDM1MjkiIHN0cm9rZT0iYmxhY2siLz48Y2lyY2xlIGlkPSJFbGxpcHNlIDEzMDlfX19fXzBfMl9PV09aQVlIVUpVIiBjeD0iMjAuMDI4NSIgY3k9IjEwLjYyMzUiIHI9IjEuMDM1MjkiIHN0cm9rZT0iYmxhY2siLz48cmVjdCBpZD0iUmVjdGFuZ2xlIDEwMjY1X19fX18wXzNfREJURFVUVFpLViIgd2lkdGg9IjIuMDcwNTkiIGhlaWdodD0iMi4wNzA1OSIgeD0iMy4wNzA0NCIgeT0iMTIuNDczNiIgc3Ryb2tlPSJibGFjayIgcng9IjEuMDM1MjkiLz48cmVjdCBpZD0iUmVjdGFuZ2xlIDEwMjY2X19fX18wXzRfRVdYUllCSVhaUiIgd2lkdGg9IjIuMDcwNTkiIGhlaWdodD0iMi4wNzA1OSIgeD0iMTguOTkzMiIgeT0iMTIuNzMyNCIgc3Ryb2tlPSJibGFjayIgcng9IjEuMDM1MjkiLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Steam Deck",
                    "TW": "Steam Deck",
                    "CN": "Steam Deck",
                    "BR": "Steam Deck",
                    "CZ": "Steam Deck",
                    "DA": "Steam Deck",
                    "DE": "Steam Deck",
                    "ES": "Steam Deck",
                    "FI": "Steam Deck",
                    "FR": "Steam Deck",
                    "HU": "Steam Deck",
                    "IT": "Steam Deck",
                    "JP": "Steam Deck",
                    "KR": "Steam Deck",
                    "MS": "Steam Deck",
                    "NL": "Steam Deck",
                    "NO": "Steam Deck",
                    "PL": "Steam Deck",
                    "RU": "Steam Deck",
                    "SV": "Steam Deck",
                    "TH": "Steam Deck",
                    "TR": "Steam Deck",
                    "UK": "Steam Deck",
                    "RO": "Steam Deck",
                    "SL": "Steam Deck"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 1,
                    "type": [
                        "49"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJpYyBlcXVpcG1lbnQvYWlyIGNvbmRpdGlvbmVyIj48cGF0aCBpZD0iVmVjdG9yX19fX18wXzBfQ0REUkJDTUJQVSIgZmlsbD0iIzI4MzAzRiIgZD0iTTE1LjcxMjMgNC4yODc2OUMxNi4wMDUyIDQuNTgwNTggMTYuNDggNC41ODA1OCAxNi43NzI5IDQuMjg3NjlDMTcuMDY1OCAzLjk5NDggMTcuMDY1OCAzLjUxOTkyIDE2Ljc3MjkgMy4yMjcwM0wxNS43MTIzIDQuMjg3NjlaTTkuMzQ4MzEgNS4zNDgzNUM5LjA1NTQyIDUuNjQxMjQgOS4wNTU0MiA2LjExNjEyIDkuMzQ4MzEgNi40MDkwMUM5LjY0MTIxIDYuNzAxOSAxMC4xMTYxIDYuNzAxOSAxMC40MDkgNi40MDkwMUw5LjM0ODMxIDUuMzQ4MzVaTTcuMjI2OTkgMy4yMjcwM0M2LjkzNDEgMy41MTk5MiA2LjkzNDEgMy45OTQ4IDcuMjI2OTkgNC4yODc2OUM3LjUxOTg5IDQuNTgwNTggNy45OTQ3NiA0LjU4MDU4IDguMjg3NjUgNC4yODc2OUw3LjIyNjk5IDMuMjI3MDNaTTEzLjU5MSA2LjQwOTAxQzEzLjg4MzggNi43MDE5IDE0LjM1ODcgNi43MDE5IDE0LjY1MTYgNi40MDkwMUMxNC45NDQ1IDYuMTE2MTIgMTQuOTQ0NSA1LjY0MTI0IDE0LjY1MTYgNS4zNDgzNUwxMy41OTEgNi40MDkwMVpNMiAxMUwxLjI1IDExTDEuMjUgMTFMMiAxMVpNNSA4TDUgOC43NUw1IDhaTTUgMThMNSAxNy4yNUw1IDE3LjI1TDUgMThaTTIgMTVMMi43NSAxNUwyLjc1IDE1TDIgMTVaTTIyIDE1TDIyLjc1IDE1VjE1SDIyWk0xOSAxOEwxOSAxOC43NUwxOSAxOC43NUwxOSAxOFpNMjIgMTFMMjEuMjUgMTFWMTFIMjJaTTE5IDhMMTkgNy4yNUwxOSA4Wk0zIDE0LjI1SDIuMjVWMTUuNzVIM1YxNC4yNVpNMjEgMTUuNzVIMjEuNzVWMTQuMjVIMjFWMTUuNzVaTTE1LjI1IDIyQzE1LjI1IDIyLjQxNDIgMTUuNTg1OCAyMi43NSAxNiAyMi43NUMxNi40MTQyIDIyLjc1IDE2Ljc1IDIyLjQxNDIgMTYuNzUgMjJIMTUuMjVaTTE2Ljc1IDIwLjVDMTYuNzUgMjAuMDg1OCAxNi40MTQyIDE5Ljc1IDE2IDE5Ljc1QzE1LjU4NTggMTkuNzUgMTUuMjUgMjAuMDg1OCAxNS4yNSAyMC41SDE2Ljc1Wk0xMS4yNSAyMkMxMS4yNSAyMi40MTQyIDExLjU4NTggMjIuNzUgMTIgMjIuNzVDMTIuNDE0MiAyMi43NSAxMi43NSAyMi40MTQyIDEyLjc1IDIySDExLjI1Wk0xMi43NSAyMC41QzEyLjc1IDIwLjA4NTggMTIuNDE0MiAxOS43NSAxMiAxOS43NUMxMS41ODU4IDE5Ljc1IDExLjI1IDIwLjA4NTggMTEuMjUgMjAuNUgxMi43NVpNNy4yNSAyMkM3LjI1IDIyLjQxNDIgNy41ODU3OSAyMi43NSA4IDIyLjc1QzguNDE0MjEgMjIuNzUgOC43NSAyMi40MTQyIDguNzUgMjJINy4yNVpNOC43NSAyMC41QzguNzUgMjAuMDg1OCA4LjQxNDIxIDE5Ljc1IDggMTkuNzVDNy41ODU3OSAxOS43NSA3LjI1IDIwLjA4NTggNy4yNSAyMC41SDguNzVaTTEyIDIuNzVDMTMuNDQ5OSAyLjc1IDE0Ljc2MTQgMy4zMzY4MSAxNS43MTIzIDQuMjg3NjlMMTYuNzcyOSAzLjIyNzAzQzE1LjU1MjIgMi4wMDYzNCAxMy44NjM3IDEuMjUgMTIgMS4yNVYyLjc1Wk0xMC40MDkgNi40MDkwMUMxMC44MTcgNi4wMDEwMiAxMS4zNzg0IDUuNzUgMTIgNS43NVY0LjI1QzEwLjk2NDYgNC4yNSAxMC4wMjYxIDQuNjcwNTUgOS4zNDgzMSA1LjM0ODM1TDEwLjQwOSA2LjQwOTAxWk04LjI4NzY1IDQuMjg3NjlDOS4yMzg1NCAzLjMzNjgxIDEwLjU1IDIuNzUgMTIgMi43NVYxLjI1QzEwLjEzNjIgMS4yNSA4LjQ0NzY4IDIuMDA2MzQgNy4yMjY5OSAzLjIyNzAzTDguMjg3NjUgNC4yODc2OVpNMTIgNS43NUMxMi42MjE1IDUuNzUgMTMuMTgzIDYuMDAxMDIgMTMuNTkxIDYuNDA5MDFMMTQuNjUxNiA1LjM0ODM1QzEzLjk3MzggNC42NzA1NSAxMy4wMzUzIDQuMjUgMTIgNC4yNVY1Ljc1Wk0yMS4yNSAxMVYxNUgyMi43NVYxMUgyMS4yNVpNMTkgMTcuMjVMNSAxNy4yNUw1IDE4Ljc1TDE5IDE4Ljc1TDE5IDE3LjI1Wk0yLjc1IDE1TDIuNzUgMTFMMS4yNSAxMUwxLjI1IDE1TDIuNzUgMTVaTTUgOC43NUwxOSA4Ljc1TDE5IDcuMjVMNSA3LjI1TDUgOC43NVpNMi43NSAxMUMyLjc1IDkuOTQ5NDYgMi45NDU2NyA5LjQ1ODEzIDMuMjAxOSA5LjIwMTlDMy40NTgxMyA4Ljk0NTY3IDMuOTQ5NDYgOC43NSA1IDguNzVMNSA3LjI1QzMuODQxNCA3LjI1IDIuODMyNzMgNy40NDk3NiAyLjE0MTI0IDguMTQxMjRDMS40NDk3NiA4LjgzMjczIDEuMjUgOS44NDE0IDEuMjUgMTFMMi43NSAxMVpNNSAxNy4yNUMzLjk0OTQ2IDE3LjI1IDMuNDU4MTMgMTcuMDU0MyAzLjIwMTkgMTYuNzk4MUMyLjk0NTY3IDE2LjU0MTkgMi43NSAxNi4wNTA1IDIuNzUgMTVMMS4yNSAxNUMxLjI1IDE2LjE1ODYgMS40NDk3NiAxNy4xNjczIDIuMTQxMjQgMTcuODU4OEMyLjgzMjczIDE4LjU1MDIgMy44NDE0IDE4Ljc1IDUgMTguNzVMNSAxNy4yNVpNMjEuMjUgMTVDMjEuMjUgMTYuMDUwNSAyMS4wNTQzIDE2LjU0MTkgMjAuNzk4MSAxNi43OTgxQzIwLjU0MTkgMTcuMDU0MyAyMC4wNTA1IDE3LjI1IDE5IDE3LjI1TDE5IDE4Ljc1QzIwLjE1ODYgMTguNzUgMjEuMTY3MyAxOC41NTAyIDIxLjg1ODggMTcuODU4OEMyMi41NTAyIDE3LjE2NzMgMjIuNzUgMTYuMTU4NiAyMi43NSAxNUwyMS4yNSAxNVpNMjIuNzUgMTFDMjIuNzUgOS44NDE0IDIyLjU1MDIgOC44MzI3MyAyMS44NTg4IDguMTQxMjRDMjEuMTY3MyA3LjQ0OTc2IDIwLjE1ODYgNy4yNSAxOSA3LjI1TDE5IDguNzVDMjAuMDUwNSA4Ljc1IDIwLjU0MTkgOC45NDU2NyAyMC43OTgxIDkuMjAxOUMyMS4wNTQzIDkuNDU4MTMgMjEuMjUgOS45NDk0NiAyMS4yNSAxMUwyMi43NSAxMVpNMyAxNS43NUgyMVYxNC4yNUgzVjE1Ljc1Wk0xNi43NSAyMlYyMC41SDE1LjI1VjIySDE2Ljc1Wk0xMi43NSAyMlYyMC41SDExLjI1VjIySDEyLjc1Wk04Ljc1IDIyVjIwLjVINy4yNVYyMkg4Ljc1WiIvPjwvZz48L2c+PC9zdmc+",
                    "EN": "Air Conditioner",
                    "TW": "冷氣機",
                    "CN": "空调",
                    "BR": "Ar condicionado",
                    "CZ": "Klimatizace",
                    "DA": "Airconditioner",
                    "DE": "Klimagerät",
                    "ES": "Aire acondicionado",
                    "FI": "Ilmastointikone",
                    "FR": "Climatisation",
                    "HU": "Légkondicionáló",
                    "IT": "Climatizzatore",
                    "JP": "エアコンディショナー",
                    "KR": "에어컨",
                    "MS": "Penyaman Udara",
                    "NL": "Airconditioner",
                    "NO": "Klimakontroll",
                    "PL": "Klimatyzator",
                    "RU": "Кондиционер",
                    "SV": "Luftkonditionering",
                    "TH": "เครื่องปรับอากาศ",
                    "TR": "Klima",
                    "UK": "Кондиціонер",
                    "RO": "Aer condiționat",
                    "SL": "Klimatska naprava"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 2,
                    "type": [
                        "50"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBzdHJva2U9IiMwMDAiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiPjxwYXRoIGQ9Ik0xNC4xIDdhLjYuNiAwIDEgMC0xLjIgMHYzYS42LjYgMCAxIDAgMS4yIDBWN1ptMCA2LjVhLjYuNiAwIDEgMC0xLjIgMFYxN2EuNi42IDAgMSAwIDEuMiAwdi0zLjVabTEuMy0yLjFIMy42VjNBMS40IDEuNCAwIDAgMSA1IDEuNmg5QTEuNCAxLjQgMCAwIDEgMTUuNCAzdjguNFpNMy42IDEyLjZoMTEuOFYyMWExLjQgMS40IDAgMCAxLTEuNCAxLjRINUExLjQgMS40IDAgMCAxIDMuNiAyMXYtOC40Wm01LjE1LTguNWgxLjVhLjE1LjE1IDAgMCAxIDAgLjNoLTEuNWEuMTUuMTUgMCAwIDEgMC0uM1ptMS41LTEuMmgtMS41YTEuMzUgMS4zNSAwIDAgMCAwIDIuN2gxLjVhMS4zNSAxLjM1IDAgMSAwIDAtMi43WiIvPjxwYXRoIHN0cm9rZS1saW5lam9pbj0icm91bmQiIGQ9Ik0xOS4yMjIgOWE1LjY0IDUuNjQgMCAwIDAgMS42NTctNCA1LjY0IDUuNjQgMCAwIDAtMS42NTctNG0tMiAyYTIuODIgMi44MiAwIDAgMSAuODI4IDIgMi44MiAyLjgyIDAgMCAxLS44MjggMiIvPjwvZz48L3N2Zz4=",
                    "EN": "Refrigerator",
                    "TW": "冰箱",
                    "CN": "冰箱",
                    "BR": "Geladeira",
                    "CZ": "Lednice",
                    "DA": "Køleskab",
                    "DE": "Kühlschrank",
                    "ES": "Frigorífico",
                    "FI": "Jääkaappi",
                    "FR": "Réfrigérateur",
                    "HU": "Hűtőszekrény",
                    "IT": "Frigorifero",
                    "JP": "冷蔵庫",
                    "KR": "냉장고",
                    "MS": "Peti Sejuk",
                    "NL": "Koelkast",
                    "NO": "Kjøleskap",
                    "PL": "Lodówka",
                    "RU": "Холодильник",
                    "SV": "Kylskåp",
                    "TH": "ตู้เย็น",
                    "TR": "Buzdolabı",
                    "UK": "Холодильник",
                    "RO": "Frigider",
                    "SL": "Hladilnik"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 3,
                    "type": [
                        "51"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJHcm91cCA0NDQ2Ij48ZyBpZD0iR3JvdXAgNDQwOCI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8wX0NNRFlPRUJFUloiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0yIDZDMiAzLjc5MDg2IDMuNzkwODYgMiA2IDJIMThDMjAuMjA5MSAyIDIyIDMuNzkwODYgMjIgNlYxOEMyMiAyMC4yMDkxIDIwLjIwOTEgMjIgMTggMjJINkMzLjc5MDg2IDIyIDIgMjAuMjA5MSAyIDE4VjZaIi8+PGcgaWQ9Ikdyb3VwIDQ0MDciPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMV9JSEtSWlNCQ01QIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTIuMDE0NiA2QzkuODA5MzcgNiA3LjE0NTgxIDcuMDgyNTEgNS45NTk4OCA4LjVDNS40NTgzMyA5LjA5OTQ4IDUuNTAwOSA5Ljk2NSA1LjUwMDkgMTAuNUM1LjUwMDkgMTIgNi4wNDU3OSAxMiA2Ljk3Mzc4IDEySDEyLjAxNDZMMTQuNDU4MyA5LjMwMDI5Ii8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8yX0NYSlVFWlVBQ04iIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xMS45NDYzIDZDMTQuMTUxNiA2IDE2LjgxNTEgNy4wODI1MSAxOC4wMDExIDguNUMxOC41MDI2IDkuMDk5NDggMTguNDYgOS45NjUgMTguNDYgMTAuNUMxOC40NiAxMiAxNy45MTUxIDEyIDE2Ljk4NzIgMTJIMTYuOTQ2MyIvPjwvZz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Weight Scale",
                    "TW": "體重計",
                    "CN": "体重计",
                    "BR": "Balança pessoal",
                    "CZ": "Měrka",
                    "DA": "Badevægt",
                    "DE": "Gewichtsskala",
                    "ES": "Báscula",
                    "FI": "Vaaka",
                    "FR": "Balance",
                    "HU": "Mérleg",
                    "IT": "Bilancia",
                    "JP": "秤",
                    "KR": "체중계",
                    "MS": "Penimbang Berat",
                    "NL": "Weegschaal",
                    "NO": "Vekt",
                    "PL": "Waga",
                    "RU": "Весы",
                    "SV": "Viktskala",
                    "TH": "เครื่องชั่งน้ำหนัก",
                    "TR": "Tartı",
                    "UK": "Ваги",
                    "RO": "Cântar",
                    "SL": "Tehtnica"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 4,
                    "type": [
                        "52"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzEwIj48cGF0aCBpZD0iVmVjdG9yX19fX18wXzBfWUpJWlhQQldDUyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTIwLjggNy43MkMyMC44IDUuNzc1OTYgMTkuMjI0IDQuMiAxNy4yOCA0LjJIMTMuNzZNMjAuOCA3LjcySDMuMk0yMC44IDcuNzJWOS45Mk0zLjIgNy43MkMzLjIgNS43NzU5NiA0Ljc3NTk2IDQuMiA2LjcyIDQuMkg5LjhNMy4yIDcuNzJWOS45Mk04LjkyIDIxLjhINi43MkM0Ljc3NTk2IDIxLjggMy4yIDIwLjIyNCAzLjIgMTguMjhWOS45Mk04LjkyIDIxLjhWMTYuNjRDOC45MiAxNi4wODc3IDkuMzY3NzIgMTUuNjQgOS45MiAxNS42NEgxNC41MkMxNS4wNzIzIDE1LjY0IDE1LjUyIDE2LjA4NzcgMTUuNTIgMTYuNjRWMjEuOE04LjkyIDIxLjhIMTUuNTJNMTUuNTIgMjEuOEgxNy4yOEMxOS4yMjQgMjEuOCAyMC44IDIwLjIyNCAyMC44IDE4LjI4VjkuOTJNMy4yIDkuOTJMMSA3LjcyTTIwLjggOS45MkwyMyA3LjcyTTkuOCA0LjJWM0M5LjggMi40NDc3MiAxMC4yNDc3IDIgMTAuOCAySDEyLjc2QzEzLjMxMjMgMiAxMy43NiAyLjQ0NzcyIDEzLjc2IDNWNC4yTTkuOCA0LjJIMTMuNzZNMTAuNjggMTcuODRIMTMuNzZNMTAuNjggMTkuNkgxMy43NiIvPjwvZz48L2c+PC9zdmc+",
                    "EN": "Electric Pot",
                    "TW": "電鍋",
                    "CN": "电锅",
                    "BR": "Panela elétrica",
                    "CZ": "Elektrická hrnec",
                    "DA": "Elektrisk gryde",
                    "DE": "Elektrischer Topf",
                    "ES": "Cacerola eléctrica",
                    "FI": "Sähkökattila",
                    "FR": "Marmite électrique",
                    "HU": "Elektromos fazék",
                    "IT": "Pentola elettrica",
                    "JP": "電気ポット",
                    "KR": "전기 냄비",
                    "MS": "Periuk Elektrik",
                    "NL": "Elektrische oven",
                    "NO": "Elektrisk gryte",
                    "PL": "Garnek elektryczny",
                    "RU": "Электрический чайник",
                    "SV": "Elkittel",
                    "TH": "หม้อต้มไฟฟ้า",
                    "TR": "Elektrikli Tencere",
                    "UK": "Мультиварка",
                    "RO": "Vas electric",
                    "SL": "Električni lonček"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 5,
                    "type": [
                        "53"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzExIj48ZyBpZD0iR3JvdXAgNDQ0OSI+PHBhdGggaWQ9IlJlY3RhbmdsZSAxMDE4NF9fX19fMF8wX0NBWk5aWFpTTVoiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTQuOCA0LjZWMTkuNjhIM0MyLjIyNjggMTkuNjggMS42IDE5LjA1MzIgMS42IDE4LjI4VjZDMS42IDUuMjI2OCAyLjIyNjggNC42IDMgNC42SDE0LjhaTTE2IDQuNkgyMUMyMS43NzMyIDQuNiAyMi40IDUuMjI2OCAyMi40IDZWMTguMjhDMjIuNCAxOS4wNTMyIDIxLjc3MzIgMTkuNjggMjEgMTkuNjhIMTZWNC42WiIvPjxwYXRoIGlkPSJVbmlvbl9fX19fMF8xX1JGQkNKREhaVk4iIGZpbGw9ImJsYWNrIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGQ9Ik0xNy4xNjAyIDYuOEgyMC42ODAyVjguNTZIMTcuMTYwMlY2LjhaTTE4LjQ4MDIgMTAuNTQwM0MxOC40ODAyIDEwLjkwNDggMTguMTg0NyAxMS4yMDAzIDE3LjgyMDIgMTEuMjAwM0MxNy40NTU2IDExLjIwMDMgMTcuMTYwMiAxMC45MDQ4IDE3LjE2MDIgMTAuNTQwM0MxNy4xNjAyIDEwLjE3NTggMTcuNDU1NiA5Ljg4MDI3IDE3LjgyMDIgOS44ODAyN0MxOC4xODQ3IDkuODgwMjcgMTguNDgwMiAxMC4xNzU4IDE4LjQ4MDIgMTAuNTQwM1pNMjAuMDIxMyAxMS4yMDAxQzIwLjM4NTggMTEuMjAwMSAyMC42ODEzIDEwLjkwNDYgMjAuNjgxMyAxMC41NDAxQzIwLjY4MTMgMTAuMTc1NiAyMC4zODU4IDkuODgwMDggMjAuMDIxMyA5Ljg4MDA4QzE5LjY1NjggOS44ODAwOCAxOS4zNjEzIDEwLjE3NTYgMTkuMzYxMyAxMC41NDAxQzE5LjM2MTMgMTAuOTA0NiAxOS42NTY4IDExLjIwMDEgMjAuMDIxMyAxMS4yMDAxWk0xOC40ODAyIDEyLjc0MDVDMTguNDgwMiAxMy4xMDUgMTguMTg0NyAxMy40MDA1IDE3LjgyMDIgMTMuNDAwNUMxNy40NTU2IDEzLjQwMDUgMTcuMTYwMiAxMy4xMDUgMTcuMTYwMiAxMi43NDA1QzE3LjE2MDIgMTIuMzc2IDE3LjQ1NTYgMTIuMDgwNSAxNy44MjAyIDEyLjA4MDVDMTguMTg0NyAxMi4wODA1IDE4LjQ4MDIgMTIuMzc2IDE4LjQ4MDIgMTIuNzQwNVpNMjAuMDIxMyAxMy40MDAzQzIwLjM4NTggMTMuNDAwMyAyMC42ODEzIDEzLjEwNDggMjAuNjgxMyAxMi43NDAzQzIwLjY4MTMgMTIuMzc1OCAyMC4zODU4IDEyLjA4MDMgMjAuMDIxMyAxMi4wODAzQzE5LjY1NjggMTIuMDgwMyAxOS4zNjEzIDEyLjM3NTggMTkuMzYxMyAxMi43NDAzQzE5LjM2MTMgMTMuMTA0OCAxOS42NTY4IDEzLjQwMDMgMjAuMDIxMyAxMy40MDAzWk0xOC40ODAyIDE0Ljk0MDJDMTguNDgwMiAxNS4zMDQ3IDE4LjE4NDcgMTUuNjAwMiAxNy44MjAyIDE1LjYwMDJDMTcuNDU1NiAxNS42MDAyIDE3LjE2MDIgMTUuMzA0NyAxNy4xNjAyIDE0Ljk0MDJDMTcuMTYwMiAxNC41NzU3IDE3LjQ1NTYgMTQuMjgwMiAxNy44MjAyIDE0LjI4MDJDMTguMTg0NyAxNC4yODAyIDE4LjQ4MDIgMTQuNTc1NyAxOC40ODAyIDE0Ljk0MDJaTTIwLjAyMTMgMTUuNkMyMC4zODU4IDE1LjYgMjAuNjgxMyAxNS4zMDQ1IDIwLjY4MTMgMTQuOTRDMjAuNjgxMyAxNC41NzU1IDIwLjM4NTggMTQuMjggMjAuMDIxMyAxNC4yOEMxOS42NTY4IDE0LjI4IDE5LjM2MTMgMTQuNTc1NSAxOS4zNjEzIDE0Ljk0QzE5LjM2MTMgMTUuMzA0NSAxOS42NTY4IDE1LjYgMjAuMDIxMyAxNS42Wk0xOC40ODAzIDE3LjE0MDJDMTguNDgwMyAxNy41MDQ3IDE4LjE4NDggMTcuODAwMiAxNy44MjAzIDE3LjgwMDJDMTcuNDU1OCAxNy44MDAyIDE3LjE2MDMgMTcuNTA0NyAxNy4xNjAzIDE3LjE0MDJDMTcuMTYwMyAxNi43NzU3IDE3LjQ1NTggMTYuNDgwMiAxNy44MjAzIDE2LjQ4MDJDMTguMTg0OCAxNi40ODAyIDE4LjQ4MDMgMTYuNzc1NyAxOC40ODAzIDE3LjE0MDJaTTIwLjAyMTMgMTcuODAwMkMyMC4zODU4IDE3LjgwMDIgMjAuNjgxMyAxNy41MDQ3IDIwLjY4MTMgMTcuMTQwMkMyMC42ODEzIDE2Ljc3NTcgMjAuMzg1OCAxNi40ODAyIDIwLjAyMTMgMTYuNDgwMkMxOS42NTY4IDE2LjQ4MDIgMTkuMzYxMyAxNi43NzU3IDE5LjM2MTMgMTcuMTQwMkMxOS4zNjEzIDE3LjUwNDcgMTkuNjU2OCAxNy44MDAyIDIwLjAyMTMgMTcuODAwMloiIGNsaXAtcnVsZT0iZXZlbm9kZCIvPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Microwave Oven",
                    "TW": "微波爐",
                    "CN": "微波炉",
                    "BR": "Forno de microondas",
                    "CZ": "Mikrovlnná trouba",
                    "DA": "Mikrobølgeovn",
                    "DE": "Mikrowellenherd",
                    "ES": "Horno microondas",
                    "FI": "Mikroaaltouuni",
                    "FR": "Four micro-onde",
                    "HU": "Mikrohullámú sütő",
                    "IT": "Forno a microonde",
                    "JP": "電子レンジ",
                    "KR": "전자레인지",
                    "MS": "Ketuhar Gelombang Mikro",
                    "NL": "Magnetron",
                    "NO": "Mikrobølgeovn",
                    "PL": "Kuchenka mikrofalowa",
                    "RU": "Микроволновая печь",
                    "SV": "Mikrovågsugn",
                    "TH": "เตาอบไมโครเวฟ",
                    "TR": "Mikrodalga Fırın",
                    "UK": "Мікрохвильова піч",
                    "RO": "Cuptor cu microunde",
                    "SL": "Mikrovalovna pečica"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 6,
                    "type": [
                        "54"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzEyIj48ZyBpZD0iR3JvdXAgNDQ1MyI+PGcgaWQ9IkVsbGlwc2UgNDAyX19fX18wXzBfVEJIR0NHSk1JRSI+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik01LjQwMDI1IDE1LjY2NjdDNS40MDAyNSAxNi4xNzI5IDQuOTg5ODQgMTYuNTgzMyA0LjQ4MzU4IDE2LjU4MzNDMy45NzczMiAxNi41ODMzIDMuNTY2OTIgMTYuMTcyOSAzLjU2NjkyIDE1LjY2NjdDMy41NjY5MiAxNS4xNjA0IDMuOTc3MzIgMTQuNzUgNC40ODM1OCAxNC43NUM0Ljk4OTg0IDE0Ljc1IDUuNDAwMjUgMTUuMTYwNCA1LjQwMDI1IDE1LjY2NjdaIi8+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik04LjYwODU4IDE1LjY2NjdDOC42MDg1OCAxNi4xNzI5IDguMTk4MTggMTYuNTgzMyA3LjY5MTkyIDE2LjU4MzNDNy4xODU2NiAxNi41ODMzIDYuNzc1MjUgMTYuMTcyOSA2Ljc3NTI1IDE1LjY2NjdDNi43NzUyNSAxNS4xNjA0IDcuMTg1NjYgMTQuNzUgNy42OTE5MiAxNC43NUM4LjE5ODE4IDE0Ljc1IDguNjA4NTggMTUuMTYwNCA4LjYwODU4IDE1LjY2NjdaIi8+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik0xMS44MTY5IDE1LjY2NjdDMTEuODE2OSAxNi4xNzI5IDExLjQwNjUgMTYuNTgzMyAxMC45MDAyIDE2LjU4MzNDMTAuMzk0IDE2LjU4MzMgOS45ODM1OCAxNi4xNzI5IDkuOTgzNTggMTUuNjY2N0M5Ljk4MzU4IDE1LjE2MDQgMTAuMzk0IDE0Ljc1IDEwLjkwMDIgMTQuNzVDMTEuNDA2NSAxNC43NSAxMS44MTY5IDE1LjE2MDQgMTEuODE2OSAxNS42NjY3WiIvPjxwYXRoIGZpbGw9ImJsYWNrIiBkPSJNMTUuMDI1MyAxNS42NjY3QzE1LjAyNTMgMTYuMTcyOSAxNC42MTQ4IDE2LjU4MzMgMTQuMTA4NiAxNi41ODMzQzEzLjYwMjMgMTYuNTgzMyAxMy4xOTE5IDE2LjE3MjkgMTMuMTkxOSAxNS42NjY3QzEzLjE5MTkgMTUuMTYwNCAxMy42MDIzIDE0Ljc1IDE0LjEwODYgMTQuNzVDMTQuNjE0OCAxNC43NSAxNS4wMjUzIDE1LjE2MDQgMTUuMDI1MyAxNS42NjY3WiIvPjwvZz48ZyBpZD0iRWxsaXBzZSAxMTkzX19fX18wXzFfRk1IRE1XU0xaTiI+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik01LjQwMDI1IDE4LjQxNjdDNS40MDAyNSAxOC45MjI5IDQuOTg5ODQgMTkuMzMzMyA0LjQ4MzU4IDE5LjMzMzNDMy45NzczMiAxOS4zMzMzIDMuNTY2OTIgMTguOTIyOSAzLjU2NjkyIDE4LjQxNjdDMy41NjY5MiAxNy45MTA0IDMuOTc3MzIgMTcuNSA0LjQ4MzU4IDE3LjVDNC45ODk4NCAxNy41IDUuNDAwMjUgMTcuOTEwNCA1LjQwMDI1IDE4LjQxNjdaIi8+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik04LjYwODU4IDE4LjQxNjdDOC42MDg1OCAxOC45MjI5IDguMTk4MTggMTkuMzMzMyA3LjY5MTkyIDE5LjMzMzNDNy4xODU2NiAxOS4zMzMzIDYuNzc1MjUgMTguOTIyOSA2Ljc3NTI1IDE4LjQxNjdDNi43NzUyNSAxNy45MTA0IDcuMTg1NjYgMTcuNSA3LjY5MTkyIDE3LjVDOC4xOTgxOCAxNy41IDguNjA4NTggMTcuOTEwNCA4LjYwODU4IDE4LjQxNjdaIi8+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik0xMS44MTY5IDE4LjQxNjdDMTEuODE2OSAxOC45MjI5IDExLjQwNjUgMTkuMzMzMyAxMC45MDAyIDE5LjMzMzNDMTAuMzk0IDE5LjMzMzMgOS45ODM1OCAxOC45MjI5IDkuOTgzNTggMTguNDE2N0M5Ljk4MzU4IDE3LjkxMDQgMTAuMzk0IDE3LjUgMTAuOTAwMiAxNy41QzExLjQwNjUgMTcuNSAxMS44MTY5IDE3LjkxMDQgMTEuODE2OSAxOC40MTY3WiIvPjxwYXRoIGZpbGw9ImJsYWNrIiBkPSJNMTUuMDI1MyAxOC40MTY3QzE1LjAyNTMgMTguOTIyOSAxNC42MTQ4IDE5LjMzMzMgMTQuMTA4NiAxOS4zMzMzQzEzLjYwMjMgMTkuMzMzMyAxMy4xOTE5IDE4LjkyMjkgMTMuMTkxOSAxOC40MTY3QzEzLjE5MTkgMTcuOTEwNCAxMy42MDIzIDE3LjUgMTQuMTA4NiAxNy41QzE0LjYxNDggMTcuNSAxNS4wMjUzIDE3LjkxMDQgMTUuMDI1MyAxOC40MTY3WiIvPjwvZz48cGF0aCBpZD0iUmVjdGFuZ2xlIDEwMTgxX19fX18wXzJfUVFEVlFXWFVQQiIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik00IDEuNkgxNC41QzE1LjgyNTUgMS42IDE2LjkgMi42NzQ1MiAxNi45IDRWMjBDMTYuOSAyMS4zMjU1IDE1LjgyNTUgMjIuNCAxNC41IDIyLjRINEMyLjY3NDUyIDIyLjQgMS42IDIxLjMyNTUgMS42IDIwVjRDMS42IDIuNjc0NTIgMi42NzQ1MiAxLjYgNCAxLjZaIi8+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMTkyX19fX18wXzNfUkJLU0lZT0ZEWSIgY3g9IjkuMjUiIGN5PSI4LjMzMzE4IiByPSI0LjkiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIi8+PHBhdGggaWQ9IlZlY3RvciAyODgxX19fX18wXzRfSVhHQ1NRS0RLWSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE1LjcxMTIgNi4wNzIwMUMxNi4wMzMyIDUuNjMyODkgMTYuODUyOSA0LjY2NjgyIDE3Ljc3NTEgNC42NjY4MkMxOC44MjkgNC42NjY4MiAxOS43NTExIDYuMjAzNzUgMjEuMTEyNCA2LjIwMzc1QzIyLjIwMTQgNi4yMDM3NSAyMi44MjUgNS4yMzc2OCAyMy4wMDA2IDQuNzU0NjVNMTUuNjY3MyA4LjkyNjNDMTUuOTg5MyA4LjQ4NzE4IDE2LjgwOSA3LjUyMTExIDE3LjczMTIgNy41MjExMUMxOC43ODUxIDcuNTIxMTEgMTkuNzA3MiA5LjA1ODA0IDIxLjA2ODUgOS4wNTgwNEMyMi4xNTc1IDkuMDU4MDQgMjIuNzgxMSA4LjA5MTk3IDIyLjk1NjcgNy42MDg5NE0xNS43MTEyIDExLjg2ODRDMTYuMDMzMiAxMS40MjkzIDE2Ljg1MjkgMTAuNDYzMiAxNy43NzUxIDEwLjQ2MzJDMTguODI5IDEwLjQ2MzIgMTkuNzUxMSAxMi4wMDAyIDIxLjExMjQgMTIuMDAwMkMyMi4yMDE0IDEyLjAwMDIgMjIuODI1IDExLjAzNDEgMjMuMDAwNiAxMC41NTExIi8+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "Air Purifier",
                    "TW": "空氣清淨機",
                    "CN": "空气净化器",
                    "BR": "Purificador de ar",
                    "CZ": "Čistička vzduchu",
                    "DA": "Luftrenser",
                    "DE": "Luftreiniger",
                    "ES": "Purificador de aire",
                    "FI": "Ilmanpuhdistin",
                    "FR": "Purificateur d'air",
                    "HU": "Légtisztító",
                    "IT": "Purificatore d'aria",
                    "JP": "空気浄化機",
                    "KR": "공기 청정기",
                    "MS": "Penulen Udara",
                    "NL": "Luchtzuiveraar",
                    "NO": "Luftrenser",
                    "PL": "Oczyszczacz powietrza",
                    "RU": "Очиститель воздуха",
                    "SV": "Luftrenare",
                    "TH": "เครื่องฟอกอากาศ",
                    "TR": "Hava Arıtıcı",
                    "UK": "Очищувач повітря",
                    "RO": "Purificator de aer",
                    "SL": "Naprava za čiščenje zraka"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 7,
                    "type": [
                        "55"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzEzIj48ZyBpZD0iR3JvdXAgNDQ0NyI+PHBhdGggaWQ9IkVsbGlwc2UgMTE3MV9fX19fMF8wX1ZYS0VIV1ZVWFYiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMjAuNCAxMUMyMC40IDE1LjYzOTIgMTYuNjM5MiAxOS40IDEyIDE5LjRDNy4zNjA4MSAxOS40IDMuNiAxNS42MzkyIDMuNiAxMUMzLjYgNi4zNjA4MSA3LjM2MDgxIDIuNiAxMiAyLjZDMTYuNjM5MiAyLjYgMjAuNCA2LjM2MDgxIDIwLjQgMTFaIi8+PHBhdGggaWQ9IkVsbGlwc2UgMTE3M19fX19fMF8xX0dBS0FXVlZYRkoiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik03IDIzSDEyTTE3IDIzSDEyTTEyIDIzVjIxIi8+PHBhdGggaWQ9IkVsbGlwc2UgMTE3Ml9fX19fMF8yX0dKTkpKVUtBQ0kiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTQuODkzNiAxMS40NjY4QzE0Ljg5MzYgMTMuMDEzMiAxMy42NDY0IDE0LjI2NjggMTIuMTA3OSAxNC4yNjY4QzExLjY4NiAxNC4yNjY4IDExLjI4NTkgMTQuMTcyNSAxMC45Mjc1IDE0LjAwMzdNMTQuODkzNiAxMS40NjY4QzE0Ljg5MzYgOS45MjAzOCAxMy42NDY0IDguNjY2NzggMTIuMTA3OSA4LjY2Njc4QzExLjQ3NzYgOC42NjY3OCAxMC44OTYyIDguODc3MTkgMTAuNDI5NCA5LjIzMTk0TTE0Ljg5MzYgMTEuNDY2OEMxNi41IDEwLjUgMTguNTE0MyAxMC41MzM0IDIwIDEyLjQwMDFNMTAuNDI5NCA5LjIzMTk0QzkuNzU2OCA5Ljc0MzEgOS4zMjIxOSAxMC41NTM5IDkuMzIyMTkgMTEuNDY2OEM5LjMyMjE5IDEyLjU4OTEgOS45NzkxMSAxMy41NTcyIDEwLjkyNzUgMTQuMDAzN00xMC40Mjk0IDkuMjMxOTRDOC44NTc5IDguNjY2NzggOC4wNjUwMSA0LjQ5MzMzIDkuMzIyMTkgM00xMC45Mjc1IDE0LjAwMzdDMTAuNzAxNyAxNS4wMjQ3IDkuMzIyMTkgMTcgNyAxNy41Ii8+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "Fan",
                    "TW": "風扇",
                    "CN": "风扇",
                    "BR": "Ventilador",
                    "CZ": "Ventilátor",
                    "DA": "Blæser",
                    "DE": "Lüfter",
                    "ES": "Ventilador",
                    "FI": "Tuuletin",
                    "FR": "Ventilateur",
                    "HU": "Ventilátor",
                    "IT": "Ventilatore",
                    "JP": "ファン",
                    "KR": "선풍기",
                    "MS": "Kipas",
                    "NL": "Ventilator",
                    "NO": "Vifte",
                    "PL": "Wentylator",
                    "RU": "Вентилятор",
                    "SV": "Fläkt",
                    "TH": "พัดลม",
                    "TR": "Fan",
                    "UK": "Вентилятор",
                    "RO": "Ventilator",
                    "SL": "Ventilator"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 8,
                    "type": [
                        "56"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJIdWdlLWljb24vc21hcnQgaG91c2Uvb3V0bGluZS9odW1pZGl0eSI+PHBhdGggaWQ9IkVsbGlwc2UgNTM1X19fX18wXzBfSFdOVExOWFlKUCIgZmlsbD0iIzI4MzAzRiIgZD0iTTEyLjc1IDE4QzEyLjc1IDE4LjQxNDIgMTIuNDE0MiAxOC43NSAxMiAxOC43NUMxMS41ODU4IDE4Ljc1IDExLjI1IDE4LjQxNDIgMTEuMjUgMThDMTEuMjUgMTcuNTg1OCAxMS41ODU4IDE3LjI1IDEyIDE3LjI1QzEyLjQxNDIgMTcuMjUgMTIuNzUgMTcuNTg1OCAxMi43NSAxOFoiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzFfUkNZSFFaWUtDSiIgc3Ryb2tlPSIjMjgzMDNGIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS41IiBkPSJNMTYuMjQyNiAxMy43NTc0QzE1LjE1NjggMTIuNjcxNiAxMy42NTY4IDEyIDEyIDEyQzEwLjM0MzEgMTIgOC44NDMxMSAxMi42NzE2IDcuNzU3MzIgMTMuNzU3NE05Ljg3ODY0IDE1Ljg3ODdDMTAuNDIxNSAxNS4zMzU4IDExLjE3MTUgMTUgMTIgMTVDMTIuODI4NCAxNSAxMy41Nzg0IDE1LjMzNTggMTQuMTIxMyAxNS44Nzg3TTIwIDE0QzIwIDkuODM2NjEgMTUuNDUzNyA1LjEzMDkgMTMuMjE1NSAzLjA2NjUzQzEyLjUyMTUgMi40MjYzNyAxMS40Nzg1IDIuNDI2MzcgMTAuNzg0NSAzLjA2NjUzQzguNTQ2MyA1LjEzMDkgNCA5LjgzNjYxIDQgMTRDNCAxOS41MjI4IDguMjEyOSAyMiAxMiAyMkMxNS43ODcxIDIyIDIwIDE5LjUyMjggMjAgMTRaIi8+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Dehumidifier",
                    "TW": "除濕機",
                    "CN": "除湿机",
                    "BR": "Desumidificador",
                    "CZ": "Odvlhčovač",
                    "DA": "Affugter",
                    "DE": "Luftentfeuchter",
                    "ES": "Deshumidificador",
                    "FI": "Kosteudenpoistolaite",
                    "FR": "Déshumidificateur",
                    "HU": "Páramentesítő",
                    "IT": "Deumidificatore",
                    "JP": "除湿器",
                    "KR": "제습기",
                    "MS": "Penyahlembap",
                    "NL": "Ontvochtiger",
                    "NO": "Luftavfukter",
                    "PL": "Osuszacz",
                    "RU": "Осушитель",
                    "SV": "Avfuktare",
                    "TH": "เครื่องลด (ดูด) ความชื้น",
                    "TR": "Nem Giderici",
                    "UK": "Осушувач",
                    "RO": "Dezumidificator",
                    "SL": "Razvlaževalnik"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 9,
                    "type": [
                        "57"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzAxIj48ZyBpZD0iR3JvdXAgNDQzMiI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8wX01FSkdYU1BZSkYiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjUiIGQ9Ik00LjEzNzkzIDguMDA4OVYxOS4xNjhDNC4xMzc5MyAyMC40NjU0IDUuMTg5NzQgMjEuNTE3MiA2LjQ4NzIxIDIxLjUxNzJIMTguMjMzNkMxOS41MzExIDIxLjUxNzIgMjAuNTgyOSAyMC40NjU0IDIwLjU4MjkgMTkuMTY4VjguMDA4OU00LjEzNzkzIDguMDA4OVY1LjY1OTYyQzQuMTM3OTMgNC4zNjIxNSA1LjE4OTc0IDMuMzEwMzQgNi40ODcyMSAzLjMxMDM0SDE4LjIzMzZDMTkuNTMxMSAzLjMxMDM0IDIwLjU4MjkgNC4zNjIxNSAyMC41ODI5IDUuNjU5NjJWOC4wMDg5TTQuMTM3OTMgOC4wMDg5SDIwLjU4MjkiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzFfVVhZSUVQT1JFVCIgZmlsbD0iYmxhY2siIGQ9Ik0xNC4yMSA1LjI2NjE2SDEzLjE2NDVWNi4xNDcxNEgxNC4yMVY1LjI2NjE2WiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMl9FUENPS0lIWElSIiBmaWxsPSJibGFjayIgZD0iTTE4LjY2ODggNS4yNjYxNkgxNy42MjMzVjYuMTQ3MTRIMTguNjY4OFY1LjI2NjE2WiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfM19KQUlBVkpWVk5VIiBmaWxsPSJibGFjayIgZD0iTTE2LjQzNjUgNS4yNjYxNkgxNS4zOTExVjYuMTQ3MTRIMTYuNDM2NVY1LjI2NjE2WiIvPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTE3MF9fX19fMF80X0hZUEhQVFNXUUYiIGN4PSI3LjM2ODE5IiBjeT0iNS45NTMyOCIgcj0iMC44ODA5NzkiIGZpbGw9ImJsYWNrIi8+PHBhdGggaWQ9IkVsbGlwc2UgMTE2OV9fX19fMF81X0FSTUlTR1lQRkEiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTIuMzYwNCAxOS4xNTUzQzEwLjI5NiAxOS4xNTUzIDguNTYzMjQgMTcuNzMwNSA4LjA5MzcyIDE1LjgxMDVDOC4zMDkwNCAxNS41Nzg2IDguNjIzMTEgMTUuMjkxOSA4Ljk5ODcxIDE1LjA0M0M5LjQ5OTQzIDE0LjcxMTMgMTAuMDU1OCAxNC40ODIxIDEwLjU5ODQgMTQuNDgyMUMxMS4yNzI1IDE0LjQ4MjEgMTEuNjUxNyAxNC43NDM0IDEyLjEwNDcgMTUuMDkxOEMxMi4xMzE1IDE1LjExMjQgMTIuMTU4OCAxNS4xMzM2IDEyLjE4NjYgMTUuMTU1MkMxMi42MyAxNS40OTkxIDEzLjIxMTcgMTUuOTUwNCAxNC4xMjI0IDE1Ljk1MDRDMTQuODY3IDE1Ljk1MDQgMTUuNTY1MyAxNS42NDQyIDE2LjExNzggMTUuMzE1NEMxNi4zNDUxIDE1LjE4MDEgMTYuNTU4MiAxNS4wMzQ0IDE2Ljc1MDcgMTQuODkyOEMxNi42ODIxIDE3LjI1ODYgMTQuNzQyOCAxOS4xNTUzIDEyLjM2MDQgMTkuMTU1M1pNMTYuNTcwNiAxMy41MDc4QzE2LjI4NjcgMTMuNzQyOSAxNS45MTkzIDE0LjAzNyAxNS41MDQgMTQuMjg0MkMxNS4wMjg3IDE0LjU2NzEgMTQuNTUyNCAxNC43NTA0IDE0LjEyMjQgMTQuNzUwNEMxMy42Mjg4IDE0Ljc1MDQgMTMuMzQ1MSAxNC41MzIxIDEyLjg1NjkgMTQuMTU2NUwxMi44MzYzIDE0LjE0MDdDMTIuMzM0OSAxMy43NTUgMTEuNjg2NCAxMy4yODIxIDEwLjU5ODQgMTMuMjgyMUM5LjczMTQ4IDEzLjI4MjEgOC45NDMzIDEzLjY0MDIgOC4zMzU5MyAxNC4wNDI2QzguMjE2NzQgMTQuMTIxNiA4LjEwMjQ1IDE0LjIwMzYgNy45OTM2OCAxNC4yODcxQzguMjMwOTcgMTIuMDg1MiAxMC4wOTU1IDEwLjM3MDkgMTIuMzYwNCAxMC4zNzA5QzE0LjM1IDEwLjM3MDkgMTYuMDMwNiAxMS42OTM3IDE2LjU3MDYgMTMuNTA3OFoiLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Washing Machine",
                    "TW": "洗衣機",
                    "CN": "洗衣机",
                    "BR": "Lavadora de roupas",
                    "CZ": "Pračka",
                    "DA": "Vaskemaskine",
                    "DE": "Waschmaschine",
                    "ES": "Lavadora",
                    "FI": "Pesukone",
                    "FR": "Machine à laver",
                    "HU": "Mosógép",
                    "IT": "Lavatrice",
                    "JP": "洗濯機",
                    "KR": "세탁기",
                    "MS": "Mesin Basuh",
                    "NL": "Wasmachine",
                    "NO": "Vaskemaskin",
                    "PL": "Pralka",
                    "RU": "Стиральная машина",
                    "SV": "Tvättmaskin",
                    "TH": "เครื่องซักผ้า",
                    "TR": "Çamaşır Makinesi",
                    "UK": "Пральна машина",
                    "RO": "Mașină de spălat",
                    "SL": "Pralni stroj"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 10,
                    "type": [
                        "58"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzE0Ij48ZyBpZD0iR3JvdXAgNDQ1MSI+PHBhdGggaWQ9IlJlY3RhbmdsZSAxMDE4MV9fX19fMF8wX1BaWlRSU09OWEciIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xMC40NDY5IDQuOTY0MzVDMTAuNjExMyA0LjY3NjY0IDEwLjUxMTMgNC4zMTAxMyAxMC4yMjM2IDQuMTQ1NzJDOS45MzU5IDMuOTgxMzEgOS41NjkzOSA0LjA4MTI3IDkuNDA0OTggNC4zNjg5OEw3Ljc3NTM1IDcuMjIwODRDNy42NjkyMyA3LjQwNjU0IDcuNjcgNy42MzQ2OSA3Ljc3NzM1IDcuODE5NjdDNy44ODQ3IDguMDA0NjYgOC4wODI0MSA4LjExODUyIDguMjk2MyA4LjExODUySDkuODMwODJMOC45NjcwMyAxMC4xMzRDOC44MzY1IDEwLjQzODYgOC45Nzc1OSAxMC43OTEzIDkuMjgyMTcgMTAuOTIxOUM5LjU4Njc0IDExLjA1MjQgOS45Mzk0NyAxMC45MTEzIDEwLjA3IDEwLjYwNjdMMTEuMjkyMiA3Ljc1NDg3QzExLjM3MTcgNy41Njk0OSAxMS4zNTI3IDcuMzU2NiAxMS4yNDE2IDcuMTg4MjJDMTEuMTMwNiA3LjAxOTg1IDEwLjk0MjQgNi45MTg1MiAxMC43NDA3IDYuOTE4NTJIOS4zMzAyTDEwLjQ0NjkgNC45NjQzNVpNOC43MDM3IDE1LjA2NjdDOC4zNzIzMyAxNS4wNjY3IDguMTAzNyAxNS4zMzUzIDguMTAzNyAxNS42NjY3QzguMTAzNyAxNS45OTggOC4zNzIzMyAxNi4yNjY3IDguNzAzNyAxNi4yNjY3SDEwLjMzMzNDMTAuNjY0NyAxNi4yNjY3IDEwLjkzMzMgMTUuOTk4IDEwLjkzMzMgMTUuNjY2N0MxMC45MzMzIDE1LjMzNTMgMTAuNjY0NyAxNS4wNjY3IDEwLjMzMzMgMTUuMDY2N0g4LjcwMzdaTTcuNDY5ODkgMTguMzI1OVYxNC42Mjk2QzcuNDY5ODkgMTQuNDA4NyA3LjY0ODk4IDE0LjIyOTYgNy44Njk4OSAxNC4yMjk2SDEwLjk2M0MxMS4xODM5IDE0LjIyOTYgMTEuMzYzIDE0LjQwODcgMTEuMzYzIDE0LjYyOTZWMTguMzI1OUg3LjQ2OTg5Wk0xMi41NjMgMTguMzI1OVYxNC42Mjk2QzEyLjU2MyAxMy43NDYgMTEuODQ2NiAxMy4wMjk2IDEwLjk2MyAxMy4wMjk2SDcuODY5ODlDNi45ODYyMyAxMy4wMjk2IDYuMjY5ODkgMTMuNzQ2IDYuMjY5ODkgMTQuNjI5NlYxOC4zMjU5SDZDNC42NzQ1MiAxOC4zMjU5IDMuNiAxNy4yNTE0IDMuNiAxNS45MjU5VjRDMy42IDIuNjc0NTIgNC42NzQ1MiAxLjYgNiAxLjZIMTMuMDM3QzE0LjM2MjUgMS42IDE1LjQzNyAyLjY3NDUyIDE1LjQzNyA0VjE1LjkyNTlDMTUuNDM3IDE3LjI1MTQgMTQuMzYyNSAxOC4zMjU5IDEzLjAzNyAxOC4zMjU5SDEyLjU2M1oiLz48cGF0aCBpZD0iRWxsaXBzZSAxMTgyX19fX18wXzFfQVJUTEtDT0JCTCIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik02LjU3MDQ4IDIwLjk2M0M2Ljc5NDI3IDIxLjIxMyA3LjExOTUgMjEuMzcwNCA3LjQ4MTQ4IDIxLjM3MDRDOC4xNTY1IDIxLjM3MDQgOC43MDM3IDIwLjgyMzIgOC43MDM3IDIwLjE0ODFDOC43MDM3IDE5LjQ3MzEgOC4xNTY1IDE4LjkyNTkgNy40ODE0OCAxOC45MjU5QzYuODA2NDcgMTguOTI1OSA2LjI1OTI2IDE5LjQ3MzEgNi4yNTkyNiAyMC4xNDgxQzYuMjU5MjYgMjAuNDYxMiA2LjM3Njk0IDIwLjc0NjcgNi41NzA0OCAyMC45NjNaTTYuNTcwNDggMjAuOTYzVjIyLjEzNzFDNi41NzA0OCAyMi42MTM3IDYuOTU2ODEgMjMgNy40MzMzOSAyM1YyM0M3LjkwOTk2IDIzIDguMjk2MyAyMi42MTM3IDguMjk2MyAyMi4xMzcxVjIwLjk2MyIvPjxnIGlkPSJHcm91cCA0NDM4Ij48cGF0aCBpZD0iVmVjdG9yIDI4NzhfX19fXzBfMl9SR1hBUkVaU09QIiBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTExLjk2MyAxOC41MTg1VjE5Ljk2MjlDMTEuOTYzIDIwLjUxNTIgMTIuNDEwOCAyMC45NjI5IDEyLjk2MyAyMC45NjI5SDE4LjI5NjRDMTguODQ4NyAyMC45NjI5IDE5LjI5NjQgMjAuNTE1MiAxOS4yOTY0IDE5Ljk2MjlWMTMuMjIyMiIvPjxwYXRoIGlkPSJSZWN0YW5nbGUgMTAxODhfX19fXzBfM19IREtDSFRUS1FIIiBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE4LjQ4MTYgOC4zMzMzMVYxMi40MDc0QzE4LjQ4MTYgMTIuODU3NCAxOC44NDY0IDEzLjIyMjIgMTkuMjk2NCAxMy4yMjIyVjEzLjIyMjJDMTkuNzQ2NCAxMy4yMjIyIDIwLjExMTIgMTIuODU3NCAyMC4xMTEyIDEyLjQwNzRWOC4zMzMzMSIvPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTE4M19fX19fMF80X1lJWUhLUE1FVU4iIGN4PSIxOS4yOTY0IiBjeT0iNi43MDM2OSIgcj0iMS44NDQ0NCIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIxLjIiLz48Y2lyY2xlIGlkPSJFbGxpcHNlIDExODRfX19fXzBfNV9MU1ZIUlJaTUtBIiBjeD0iMTkuMjk2NCIgY3k9IjYuNzAzNjkiIHI9IjAuNiIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIwLjQyOTYzIi8+PC9nPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Water Heater",
                    "TW": "電熱水器",
                    "CN": "热水器",
                    "BR": "Aquecedor de água",
                    "CZ": "Ohřívač vody",
                    "DA": "Vandvarmer",
                    "DE": "Wassererhitzer",
                    "ES": "Calentador de agua",
                    "FI": "Vedenlämmitin",
                    "FR": "Chauffe-eau",
                    "HU": "Vízmelegítő",
                    "IT": "Boiler",
                    "JP": "温水ヒーター",
                    "KR": "온수기",
                    "MS": "Pemanas Air",
                    "NL": "Waterkoker",
                    "NO": "Varmtvannsbereder",
                    "PL": "Podgrzewacz wody",
                    "RU": "Нагреватель воды",
                    "SV": "Vattenkokare",
                    "TH": "เครื่องทำน้ำอุ่น",
                    "TR": "Su Isıtıcı",
                    "UK": "Нагрівач води",
                    "RO": "Boiler",
                    "SL": "Grelec za vodo"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 11,
                    "type": [
                        "59"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzE1Ij48ZyBpZD0iR3JvdXAgNDQ1MCI+PHBhdGggaWQ9IlJlY3RhbmdsZSAxMDE4MF9fX19fMF8wX05aQ0lXVFdYU1ciIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTUuOTE2MSAxLjAyNTc0SDMuOTk5OUMzLjQ0NzY1IDEuMDI1NzQgMy4wMjMxMSAxLjQ3NjMyIDMuMTc4MDggMi4wMDYzOEMzLjM3MTMzIDIuNjY3MzQgMy43NzQyOSAzLjYwODIzIDQuNjE0NTIgNC42NTg0MUMzLjU5NjcgNi40Mzk2IDMuMjIwNTMgMTEuNTg5NSAzLjA4MTUgMTUuMTUyOE0xNS45MTYxIDEuMDI1NzRDMTcuMzc3MiAyLjk3Mzg1IDE3Ljk1NjggOS4wMzU1OCAxOC4xODY3IDEzLjUzODNNMTUuOTE2MSAxLjAyNTc0QzE2Ljg1NzkgMS4wMjU3NCAxOC4yOTk1IDAuNzc1NzczIDE5LjE0NTIgMS44MzI4OEMyMC43NTk3IDMuODUxMDMgMjAuNjcyNyA4LjcyNjA2IDIwLjM1NjEgMTAuMzA5MkMxOS45NTI0IDEyLjMyNzQgMTguOTA5OSAxMy4yNjkyIDE4LjE4NjcgMTMuNTM4M00zLjA4MTUgMTUuMTUyOEMzLjA3MzAyIDE1LjM3MDMgMy4wNjU0MSAxNS41ODIgMy4wNTg2IDE1Ljc4NjVDMy4wMDM0NiAxNy40NDI1IDQuMzQzMTUgMTguNzg1NCA2IDE4Ljc4NTRIMTUuMzM3OUMxNi45OTQ4IDE4Ljc4NTQgMTguMzMzNSAxNy40NDI1IDE4LjI4MDEgMTUuNzg2NUMxOC4yNzM1IDE1LjU4MjYgMTguMjY2MiAxNS4zNzEgMTguMjU4IDE1LjE1MjhNMy4wODE1IDE1LjE1MjhIOS43MDAyNU0xMS40NzYyIDE1LjE1MjhIMTguMjU4TTE4LjI1OCAxNS4xNTI4QzE4LjIzOTEgMTQuNjQ2OCAxOC4yMTU3IDE0LjEwNDkgMTguMTg2NyAxMy41MzgzIi8+PHJlY3QgaWQ9IlJlY3RhbmdsZSAxMDE4Nl9fX19fMF8xX0xMU0ZEUkNFQ0YiIHdpZHRoPSIxLjY2NTYxIiBoZWlnaHQ9IjExLjcxNjEiIHg9IjEwLjA1ODgiIHk9IjQuMDQ3MjQiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiByeD0iMC44MzI4MDUiLz48cGF0aCBpZD0iUmVjdGFuZ2xlIDEwMTg3X19fX18wXzJfTFpQQktNSUtHTCIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik01IDIwLjczNDRIMTYuMjgzM0MxNy4wNTY1IDIwLjczNDQgMTcuNjgzMyAyMS4zNjEyIDE3LjY4MzMgMjIuMTM0NFYyMi40SDMuNlYyMi4xMzQ0QzMuNiAyMS4zNjEyIDQuMjI2OCAyMC43MzQ0IDUgMjAuNzM0NFoiLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Electric Kettle",
                    "TW": "電熱水壺",
                    "CN": "电热水壶",
                    "BR": "Chaleira elétrica",
                    "CZ": "Elektrická konvice",
                    "DA": "Elektrisk kedel",
                    "DE": "Elektrischer Wasserkocher",
                    "ES": "Tetera eléctrica",
                    "FI": "Vedenkeitin",
                    "FR": "Bouilloire électrique",
                    "HU": "Elektromos vízforraló",
                    "IT": "Bollitore elettrico",
                    "JP": "電気湯沸し器",
                    "KR": "전기 주전자",
                    "MS": "Cerek Elektrik",
                    "NL": "Elektrische ketel",
                    "NO": "Elektrisk vannkoker",
                    "PL": "Czajnik elektryczny",
                    "RU": "Электрический чайник",
                    "SV": "Elektrisk vattenkokare",
                    "TH": "กาต้มน้ำไฟฟ้า",
                    "TR": "Elektrikli Su Isıtıcı",
                    "UK": "Електрочайник",
                    "RO": "Fierbător electric",
                    "SL": "Električni kuhalnik"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 12,
                    "type": [
                        "60"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJMYW1wIj48ZyBpZD0iR3JvdXBfMiI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8xX0tEQlZWRElETVciIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik01LjY1IDguMDgzNDFDNS4yMSAxMC4xNjM0IDUuNzUwMDIgMTIuMjUzNCA3LjE0MDAyIDEzLjgxMzRDNy44MjAwMiAxNC41NzM0IDguMjEgMTUuNTgzNCA4LjIxIDE2LjU3MzRWMTcuNjUzNEM4LjIxIDE4LjM4MzQgOC42Nzk5OSAxOC45OTM0IDkuMzI5OTkgMTkuMjAzNEM5LjU0OTk5IDIwLjQ2MzQgMTAuNjUgMjEuNDgzNCAxMiAyMS40ODM0QzEzLjM2IDIxLjQ4MzQgMTQuNDYgMjAuNDYzNCAxNC42NyAxOS4yMDM0QzE1LjMyIDE4Ljk4MzQgMTUuNzkgMTguMzgzNCAxNS43OSAxNy42NTM0VjE2LjU3MzRDMTUuNzkgMTUuNTczNCAxNi4xOCAxNC41ODM0IDE2Ljg5IDEzLjc2MzRDMTcuOTMgMTIuNTgzNCAxOC41IDExLjA2MzQgMTguNSA5LjQ4MzQzQzE4LjUgNS40MDM0MyAxNC43NCAyLjI4MzQ0IDEwLjYgMy4xNDM0NEM4LjE1MDAxIDMuNjUzNDQgNi4xNyA1LjYyMzQxIDUuNjUgOC4wODM0MVoiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzJfVVZSQ1JVVEpHQiIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTguOTEgMTguNDIzNEgxNS4wOCIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfM19ISEpaWkpIRUdNIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNOC40MDc4NCAxNS4zMzM0SDEyLjAyNTVNMTUuNjQzMiAxNS4zMzM0SDEyLjAyNTVNMTIuMDI1NSAxNS4zMzM0VjEyLjk5OTkiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzRfRUtFTVVET1JNQSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE2LjI0MzQgOC43NTczNkMxNS4xNTc2IDcuNjcxNTcgMTMuNjU3NiA3IDEyLjAwMDggN0MxMC4zNDM5IDcgOC44NDM5MyA3LjY3MTU3IDcuNzU4MTUgOC43NTczNk05Ljg3OTQ3IDEwLjg3ODdDMTAuNDIyNCAxMC4zMzU4IDExLjE3MjQgMTAgMTIuMDAwOCAxMEMxMi44MjkyIDEwIDEzLjU3OTIgMTAuMzM1OCAxNC4xMjIxIDEwLjg3ODciLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Smart Bulb",
                    "TW": "智慧燈具",
                    "CN": "智能灯泡",
                    "BR": "Lâmpada inteligente",
                    "CZ": "Chytrá žárovka",
                    "DA": "Smart-pære",
                    "DE": "Intelligente Glühbirne",
                    "ES": "Bombilla inteligente",
                    "FI": "Älypolttimo",
                    "FR": "Ampoule intelligente",
                    "HU": "Intelligens izzó",
                    "IT": "Lampadina intelligente",
                    "JP": "スマートバルブ",
                    "KR": "스마트 전구",
                    "MS": "Mentol Pintar",
                    "NL": "Slimme lamp",
                    "NO": "Smartpære",
                    "PL": "Inteligentna żarówka",
                    "RU": "Умная лампочка",
                    "SV": "Smart glödlampa",
                    "TH": "หลอดไฟอัจฉริยะ",
                    "TR": "Akıllı Ampul",
                    "UK": "Розумна лампочка",
                    "RO": "Bec inteligent",
                    "SL": "Pametna žarnica"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 13,
                    "type": [
                        "23"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJIdWdlLWljb24vc21hcnQgaG91c2Uvb3V0bGluZS9zbWFydC10diI+PHBhdGggaWQ9IkVsbGlwc2UgNTQzX19fX18wXzBfU01XWEJSTEJCVSIgZmlsbD0iYmxhY2siIGQ9Ik0xMi43NSAxM0MxMi43NSAxMy40MTQyIDEyLjQxNDIgMTMuNzUgMTIgMTMuNzVDMTEuNTg1OCAxMy43NSAxMS4yNSAxMy40MTQyIDExLjI1IDEzQzExLjI1IDEyLjU4NTggMTEuNTg1OCAxMi4yNSAxMiAxMi4yNUMxMi40MTQyIDEyLjI1IDEyLjc1IDEyLjU4NTggMTIuNzUgMTNaIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8xX0hGRFlEUFZJSUMiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjUiIGQ9Ik0xNi4yNDI2IDguNzU3MzZDMTUuMTU2OCA3LjY3MTU3IDEzLjY1NjggNyAxMiA3QzEwLjM0MzEgNyA4Ljg0MzExIDcuNjcxNTcgNy43NTczMiA4Ljc1NzM2TTkuODc4NjQgMTAuODc4N0MxMC40MjE1IDEwLjMzNTggMTEuMTcxNSAxMCAxMiAxMEMxMi44Mjg0IDEwIDEzLjU3ODQgMTAuMzM1OCAxNC4xMjEzIDEwLjg3ODdNNyAyMUMxMC43Njc1IDE5LjY3NTMgMTIuOTI5MyAxOS42NTggMTcgMjFNMTIgMjBWMTdNMiA3TDIgMTNDMiAxNS4yMDkxIDMuNzkwODYgMTcgNiAxN0wxOCAxN0MyMC4yMDkxIDE3IDIyIDE1LjIwOTEgMjIgMTNWN0MyMiA0Ljc5MDg2IDIwLjIwOTEgMyAxOCAzTDYgM0MzLjc5MDg2IDMgMiA0Ljc5MDg2IDIgN1oiLz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Smart TV",
                    "TW": "智慧電視",
                    "CN": "智能电视",
                    "BR": "Smart TV",
                    "CZ": "Chytrá TV",
                    "DA": "Smart TV",
                    "DE": "Smart-TV",
                    "ES": "TV inteligente",
                    "FI": "Äly-TV",
                    "FR": "Smart TV",
                    "HU": "Okostévé",
                    "IT": "Smart TV",
                    "JP": "スマートTV",
                    "KR": "스마트 TV",
                    "MS": "TV Pintar",
                    "NL": "Slimme TV",
                    "NO": "Smart-TV",
                    "PL": "Telewizor Smart TV",
                    "RU": "Smart TV",
                    "SV": "Smart-tv",
                    "TH": "สมาร์ททีวี",
                    "TR": "Akıllı TV",
                    "UK": "Телевізор Smart TV",
                    "RO": "Televizor inteligent",
                    "SL": "Pametni televizor"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 14,
                    "type": [
                        "61"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzE2Ij48ZyBpZD0iR3JvdXAgNDQ1MiI+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMTg1X19fX18wXzBfTk9TTlVYSU9MQiIgY3g9IjEyIiBjeT0iMTIiIHI9IjkuNCIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIvPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTE4Nl9fX19fMF8xX0xQRkFGRlhZQVAiIGN4PSIxMi4wMDA5IiBjeT0iMTEuOTk5OSIgcj0iNi42NzI3MyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIvPjxwYXRoIGlkPSJWZWN0b3IgMjg3OV9fX19fMF8yX01MWEVSSFZFWVUiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgZD0iTTMuMjAwODIgOC44MTgxNEg1LjYzNTA2TTE4LjM2MjMgOC44MTgxNEgyMC45NTA4TTE4LjU4OTYgNC43MjcyM0MxOC42NjU0IDQuMjcyNjggMTkuMDg5NiAzLjM2MzU5IDIwLjE4MDUgMy4zNjM1OU01LjE4MDUxIDQuNzI3MjNDNS4wMjg5OSA0LjI3MjY4IDQuNDA3NzggMy4zNjM1OSAzLjEzNTA2IDMuMzYzNTkiLz48ZyBpZD0iR3JvdXAgNDQ0MSI+PHJlY3QgaWQ9IlJlY3RhbmdsZSAxMDE4OV9fX19fMF8zX0NUQVJBREtOQUsiIHdpZHRoPSIxLjEzNjM2IiBoZWlnaHQ9IjMuNDA5MDkiIHg9IjExLjM1NDUiIHk9IjguMDIzMjYiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjEzNjM2IiByeD0iMC41NjgxODIiLz48ZyBpZD0iRnJhbWUgNDQ0MSI+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMTg5X19fX18wXzRfTUtNWFdOQ1FDRiIgY3g9IjEwLjEwNDIiIGN5PSIxNC43Mjc1IiByPSIwLjQ1NDU0NSIgZmlsbD0iYmxhY2siLz48Y2lyY2xlIGlkPSJFbGxpcHNlIDExODdfX19fXzBfNV9KWUpMWVNZT0FLIiBjeD0iMTIuMDEzMiIgY3k9IjE0LjcyNzUiIHI9IjAuNDU0NTQ1IiBmaWxsPSJibGFjayIvPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTE4OF9fX19fMF82X1VJR1RJSVBHWkQiIGN4PSIxMy45MjIzIiBjeT0iMTQuNzI3NSIgcj0iMC40NTQ1NDUiIGZpbGw9ImJsYWNrIi8+PC9nPjwvZz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Cleaning Robot",
                    "TW": "掃地機器人",
                    "CN": "清洁机器人",
                    "BR": "Aspirador robô",
                    "CZ": "Robot na uklízení",
                    "DA": "Robotstøvsuger",
                    "DE": "Reinigungsroboter",
                    "ES": "Robot de limpieza",
                    "FI": "Puhdistusrobotti",
                    "FR": "Robot de nettoyage",
                    "HU": "Takarító robot",
                    "IT": "Robot per la pulizia",
                    "JP": "掃除ロボット",
                    "KR": "로봇 청소기",
                    "MS": "Robot Pembersih",
                    "NL": "Schoonmaakrobot",
                    "NO": "Rengjøringsrobot",
                    "PL": "Robot sprzątający",
                    "RU": "Робот-пылесос",
                    "SV": "Städrobot",
                    "TH": "หุ่นยนต์ดูดฝุ่น",
                    "TR": "Temizlik Robotu",
                    "UK": "Робот-пилосос",
                    "RO": "Robot pentru curățenie",
                    "SL": "Čistilni robot"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 15,
                    "type": [
                        "83"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJodWdlLWljb24vZWR1Y2F0aW9uL291dGxpbmUvdGFibGUgbGFtcCI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8wX1JWRFdFRUpaVVYiIHN0cm9rZT0iIzI4MzAzRiIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuNSIgZD0iTTE3LjQ4MTkgMTAuMzM1NUwxOC44OTYxIDExLjc0OTdNMTUuMzYwNiAxMi40NTY4VjEzLjg3MU0xOS42MDMyIDguMjE0MTZIMjEuMDE3NE04IDZMMy4yMDUzMSAxMS41OTM4QzIuNTI1MTggMTIuMzg3MyAyLjU3MDYzIDEzLjU3MDYgMy4zMDk2MSAxNC4zMDk2TDExIDIyTTExIDIySDE1TTExIDIySDdNOC4xNTA3IDUuMjMyOTZMOS44MDA2MiA5LjIzOTlDMTAuMzQ0IDEwLjU1OTYgMTIuMDU1IDEwLjkwMTggMTMuMDY0MiA5Ljg5MjYxTDE1Ljg5MjYgNy4wNjQxOUMxNi45MDE4IDYuMDU1MDQgMTYuNTU5NiA0LjM0NDAxIDE1LjIzOTkgMy44MDA2MkwxMS4yMzMgMi4xNTA3QzEwLjQ4NjQgMS44NDMyOCA5LjYyODE3IDIuMDE0OTMgOS4wNTcyNSAyLjU4NTg0TDguNTg1ODQgMy4wNTcyNUM4LjAxNDkzIDMuNjI4MTcgNy44NDMyOCA0LjQ4NjM4IDguMTUwNyA1LjIzMjk2WiIvPjwvZz48L2c+PC9zdmc+",
                    "EN": "Smart Desk Lamp",
                    "TW": "智慧檯燈",
                    "CN": "智能台灯",
                    "BR": "Lâmpada inteligente",
                    "CZ": "Chytrá stolní lampa",
                    "DA": "Smart-bordlampe",
                    "DE": "Intelligente Schreibtischlampe",
                    "ES": "Lámpara inteligente de escritorio",
                    "FI": "Pöytä-älylamppu",
                    "FR": "Lampe de bureau intelligente",
                    "HU": "Intelligens asztali lámpa",
                    "IT": "Lampada da scrivania intelligente",
                    "JP": "スマートデスクランプ",
                    "KR": "스마트 책상등",
                    "MS": "Smart desk lamp",
                    "NL": "Slimme bureaulamp",
                    "NO": "Smart bordlampe",
                    "PL": "Inteligentna lampa biurkowa",
                    "RU": "Настольная смарт-лампа",
                    "SV": "Smart bänklampa",
                    "TH": "โคมไฟตั้งโต๊ะอัจฉริยะ",
                    "TR": "Akıllı masa lambası",
                    "UK": "Розумна настільна лампа",
                    "RO": "Lampă de birou inteligentă",
                    "SL": "Pametna namizna lučka"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 16,
                    "type": [
                        "84"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzE3Ij48ZyBpZD0iR3JvdXAgNDQ0NCI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8wX09SQ0RZTU1GQkciIGZpbGw9IiMyODMwM0YiIGQ9Ik0wLjU4MjIyMyAxOC43NjY2QzAuNTgyMjIzIDE5LjA5OCAwLjg1MDg1MiAxOS4zNjY2IDEuMTgyMjIgMTkuMzY2NkMxLjUxMzU5IDE5LjM2NjYgMS43ODIyMiAxOS4wOTggMS43ODIyMiAxOC43NjY2TDAuNTgyMjIzIDE4Ljc2NjZaTTE3LjU4MjIgMTguNzY2NkwxNy41ODIyIDE5LjM2NjZMMTguMTgyMiAxOS4zNjY2TDE4LjE4MjIgMTguNzY2NkwxNy41ODIyIDE4Ljc2NjZaTTUuOTkxNyAxMS4yOTgyTDUuOTkxNyAxMC42OTgyTDUuODYxNjkgMTAuNjk4Mkw1Ljc0MzM0IDEwLjc1Mkw1Ljk5MTcgMTEuMjk4MlpNMTMuMDI2NyAxMS4yOTgyTDEzLjI5MzcgMTAuNzYwOUwxMy4xNjc1IDEwLjY5ODJMMTMuMDI2NyAxMC42OTgyTDEzLjAyNjcgMTEuMjk4MlpNMSAxOS4zNjY2TDE3LjU4MjIgMTkuMzY2NkwxNy41ODIyIDE4LjE2NjZMMSAxOC4xNjY2TDEgMTkuMzY2NlpNMS43ODIyMiAxOC43NjY2QzEuNzgyMjIgMTUuNjkwOCAzLjYwOTM5IDEzLjA0MDYgNi4yNDAwNSAxMS44NDQ0TDUuNzQzMzQgMTAuNzUyQzIuNzAwMTcgMTIuMTM1OCAwLjU4MjIyMyAxNS4yMDMyIDAuNTgyMjIzIDE4Ljc2NjZMMS43ODIyMiAxOC43NjY2Wk01Ljk5MTcgMTEuODk4MkM3LjI1OTIgMTEuODk4MiA3Ljk5MTM0IDExLjg5ODIgOS4yMDA0OSAxMS44OTgyTDkuMjAwNDkgMTAuNjk4MkM3Ljk5MTM0IDEwLjY5ODIgNy4yNTkyIDEwLjY5ODIgNS45OTE3IDEwLjY5ODJMNS45OTE3IDExLjg5ODJaTTkuMjAwNDkgMTEuODk4MkMxMC41MDk2IDExLjg5ODIgMTEuMzIzIDExLjg5ODIgMTMuMDI2NyAxMS44OTgyTDEzLjAyNjcgMTAuNjk4MkMxMS4zMjMgMTAuNjk4MiAxMC41MDk2IDEwLjY5ODIgOS4yMDA0OSAxMC42OTgyTDkuMjAwNDkgMTEuODk4MlpNMTIuNzU5NiAxMS44MzU1QzE1LjI1NDYgMTMuMDc1NCAxNi45ODIyIDE1Ljc3MzUgMTYuOTgyMiAxOC43NjY2TDE4LjE4MjIgMTguNzY2NkMxOC4xODIyIDE1LjMyMDQgMTYuMTk4OCAxMi4yMDQ2IDEzLjI5MzcgMTAuNzYwOUwxMi43NTk2IDExLjgzNTVaIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8xX0ZJQ1pCUEVaR0ciIHN0cm9rZT0iIzI4MzAzRiIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTEyLjExNzQgMTguNzY2N0wxMi4xMTc0IDE5LjMxMzNDMTIuMTE3NCAyMC41MjEgMTAuODkzNiAyMS41IDkuMzg0MDQgMjEuNUM3Ljg3NDQ3IDIxLjUgNi42NTA3MSAyMC41MjEgNi42NTA3MSAxOS4zMTMzTDYuNjUwNzEgMTguNzY2NyIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMl9QSVNaWENBUVBaIiBzdHJva2U9IiMyODMwM0YiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik05LjM4MjkyIDcuMjg2NThDMTEuMzk1NyA3LjI4NjU4IDEzLjAyNzQgOC45MTgyNSAxMy4wMjc0IDEwLjkzMU05LjM4MjkyIDcuMjg2NThDNy4zNzAxNCA3LjI4NjU4IDUuNzM4NDcgOC45MTgyNSA1LjczODQ3IDEwLjkzMU05LjM4MjkyIDcuMjg2NThMOS4zODI5MiAyIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8zX09VSUFKREJLVVUiIHN0cm9rZT0iIzI4MzAzRiIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTIyLjA2ODMgMTQuNjUwMkMyMi4yODIgMTMuMTI5NiAyMS45MTU2IDExLjUyNzUgMjAuOTE4NSAxMC4yMDQyQzE5LjkyMTQgOC44ODEwMiAxOC40ODIzIDguMDg3MjMgMTYuOTYxNyA3Ljg3MzUyTTE2LjU0NDIgMTAuODQ0M0MxNy4zMDQ1IDEwLjk1MTIgMTguMDI0MSAxMS4zNDgxIDE4LjUyMjYgMTIuMDA5N0MxOS4wMjEyIDEyLjY3MTMgMTkuMjA0NCAxMy40NzI0IDE5LjA5NzUgMTQuMjMyNyIvPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Smart Ceiling Lamp",
                    "TW": "智慧天花板照明燈",
                    "CN": "智能吊灯",
                    "BR": "Lustre/luminária inteligente",
                    "CZ": "Chytrá stropní lampa",
                    "DA": "Smart-loftslampe",
                    "DE": "Intelligente Deckenleuchte",
                    "ES": "Lámpara inteligente de techo",
                    "FI": "Kattoälylamppu",
                    "FR": "Plafonnier intelligent",
                    "HU": "Intelligens mennyezeti lámpa",
                    "IT": "Lampadario intelligente",
                    "JP": "スマートシーリングランプ",
                    "KR": "스마트 천장등",
                    "MS": "Smart Ceiling Lamp",
                    "NL": "Slimme plafondlamp",
                    "NO": "Smart taklampe",
                    "PL": "Inteligentna lampa sufitowa",
                    "RU": "Потолочная смарт-лампа",
                    "SV": "Smart taklampa",
                    "TH": "โคมไฟติดเพดานอัจฉริยะ",
                    "TR": "Akıllı tavan lambası",
                    "UK": "Розумний світильник",
                    "RO": "Lampă de plafon inteligentă",
                    "SL": "Pametna stropna lučka"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 17,
                    "type": [
                        "91"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzE4Ij48ZyBpZD0iR3JvdXAgNDQ2OSI+PGcgaWQ9Ikdyb3VwIDQ0NzEiPjxyZWN0IGlkPSJSZWN0YW5nbGUgMTAyMDNfX19fXzBfMF9KSFFPUUlEWkdRIiB3aWR0aD0iMTAuNjM3NCIgaGVpZ2h0PSIyLjkwMTEiIHg9IjQuMDI1OSIgeT0iNy43NSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIxLjIiIHJ4PSIxLjQ1MDU1Ii8+PHBhdGggaWQ9IlVuaW9uX19fX18wXzFfVkVUVEZITlBHTSIgZmlsbD0iYmxhY2siIGZpbGwtcnVsZT0iZXZlbm9kZCIgZD0iTTUuNDMyMTggOC45ODA1OEM1LjQzMjE4IDguNzM3ODEgNS42Mjg5NyA4LjU0MTAyIDUuODcxNzQgOC41NDEwMkM2LjExNDUgOC41NDEwMiA2LjMxMTMgOC43Mzc4MSA2LjMxMTMgOC45ODA1OFY5LjMzMjIyQzYuMzExMyA5LjU3NDk5IDYuMTE0NSA5Ljc3MTc4IDUuODcxNzQgOS43NzE3OEM1LjYyODk3IDkuNzcxNzggNS40MzIxOCA5LjU3NDk5IDUuNDMyMTggOS4zMzIyMlY4Ljk4MDU4Wk03LjE5MDI0IDguOTgwNzVDNy4xOTAyNCA4LjczNzk5IDcuMzg3MDQgOC41NDExOSA3LjYyOTggOC41NDExOUM3Ljg3MjU3IDguNTQxMTkgOC4wNjkzNyA4LjczNzk5IDguMDY5MzcgOC45ODA3NVY5LjMzMjRDOC4wNjkzNyA5LjU3NTE2IDcuODcyNTcgOS43NzE5NiA3LjYyOTggOS43NzE5NkM3LjM4NzA0IDkuNzcxOTYgNy4xOTAyNCA5LjU3NTE2IDcuMTkwMjQgOS4zMzI0VjguOTgwNzVaTTkuMzg4MzEgOC41NDExOUM5LjE0NTU1IDguNTQxMTkgOC45NDg3NSA4LjczNzk5IDguOTQ4NzUgOC45ODA3NVY5LjMzMjRDOC45NDg3NSA5LjU3NTE2IDkuMTQ1NTUgOS43NzE5NiA5LjM4ODMxIDkuNzcxOTZDOS42MzEwOCA5Ljc3MTk2IDkuODI3ODcgOS41NzUxNiA5LjgyNzg3IDkuMzMyNFY4Ljk4MDc1QzkuODI3ODcgOC43Mzc5OSA5LjYzMTA4IDguNTQxMTkgOS4zODgzMSA4LjU0MTE5Wk0xMC43MDcgOC45ODA3NUMxMC43MDcgOC43Mzc5OSAxMC45MDM4IDguNTQxMTkgMTEuMTQ2NiA4LjU0MTE5QzExLjM4OTQgOC41NDExOSAxMS41ODYyIDguNzM3OTkgMTEuNTg2MiA4Ljk4MDc1VjkuMzMyNEMxMS41ODYyIDkuNTc1MTYgMTEuMzg5NCA5Ljc3MTk2IDExLjE0NjYgOS43NzE5NkMxMC45MDM4IDkuNzcxOTYgMTAuNzA3IDkuNTc1MTYgMTAuNzA3IDkuMzMyNFY4Ljk4MDc1Wk0xMi45MDUxIDguNTQxMTlDMTIuNjYyMyA4LjU0MTE5IDEyLjQ2NTUgOC43Mzc5OSAxMi40NjU1IDguOTgwNzVWOS4zMzI0QzEyLjQ2NTUgOS41NzUxNiAxMi42NjIzIDkuNzcxOTYgMTIuOTA1MSA5Ljc3MTk2QzEzLjE0NzkgOS43NzE5NiAxMy4zNDQ3IDkuNTc1MTYgMTMuMzQ0NyA5LjMzMjRWOC45ODA3NUMxMy4zNDQ3IDguNzM3OTkgMTMuMTQ3OSA4LjU0MTE5IDEyLjkwNTEgOC41NDExOVoiIGNsaXAtcnVsZT0iZXZlbm9kZCIvPjwvZz48ZyBpZD0iR3JvdXAgNDQ3MCI+PHBhdGggaWQ9IkVsbGlwc2UgMTIyNl9fX19fMF83X0JBUU5GVEhKUFciIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik00LjUyMjg3IDE3LjA0MTJDNS44MzY1NSAxOC4yMDA3IDcuNTYyMTcgMTguOTA0MiA5LjQ1MjExIDE4LjkwNDJDMTEuMzQyMSAxOC45MDQyIDEzLjA2NzcgMTguMjAwNyAxNC4zODE0IDE3LjA0MTJNNC41MjI4NyAxNy4wNDEyQzIuOTc1NzcgMTUuNjc1NyAyIDEzLjY3NzkgMiAxMS40NTIxQzIgNy4zMzY0MiA1LjMzNjQyIDQgOS40NTIxMSA0QzEzLjU2NzggNCAxNi45MDQyIDcuMzM2NDIgMTYuOTA0MiAxMS40NTIxQzE2LjkwNDIgMTMuNjc3OSAxNS45Mjg1IDE1LjY3NTcgMTQuMzgxNCAxNy4wNDEyTTQuNTIyODcgMTcuMDQxMkw1LjIxNDY0IDIwLjYyMTFIMTMuNjUzMUwxNC4zODE0IDE3LjA0MTIiLz48cGF0aCBpZD0iRWxsaXBzZSAxMjI3X19fX18wXzhfSlNDVEFMQ1JQVCIgZmlsbD0iYmxhY2siIGQ9Ik05LjE2NTI0IDE4LjE2MDZMOS4xNjUyNCAxNS40OTYzTDcuODMzMSAxNS40OTYzTDkuODMxMzEgMTEuNDk5OUw5LjgzMTMxIDE0LjE2NDJMMTEuMTYzNCAxNC4xNjQyTDkuMTY1MjQgMTguMTYwNloiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzlfRU9RTUlGSE5SSyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTIwLjc4MjkgMTQuMDMxN0MyMS41MzQ5IDEzLjI3OTggMjIgMTIuMjQxIDIyIDExLjA5MzVDMjIgOS45NDYwNSAyMS41MzQ5IDguOTA3MjMgMjAuNzgyOSA4LjE1NTI3TTE5LjMxMzggOS42MjQzOUMxOS42ODk4IDEwLjAwMDQgMTkuOTIyNCAxMC41MTk4IDE5LjkyMjQgMTEuMDkzNUMxOS45MjI0IDExLjY2NzIgMTkuNjg5OCAxMi4xODY2IDE5LjMxMzggMTIuNTYyNiIvPjwvZz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Smart Meter",
                    "TW": "智慧電錶",
                    "CN": "智能电表",
                    "BR": "Medidor inteligente",
                    "CZ": "Chytrý měřič",
                    "DA": "Smartmeter",
                    "DE": "Smartes Messgerät",
                    "ES": "Contador inteligente",
                    "FI": "Älykäs mittari",
                    "FR": "Compteur intelligent",
                    "HU": "Intelligens fogyasztásmérő",
                    "IT": "Contatore intelligente",
                    "JP": "スマート メーター",
                    "KR": "스마트 미터",
                    "MS": "Smart Meter",
                    "NL": "Slimme meter",
                    "NO": "Smart strømmåler",
                    "PL": "Inteligentny licznik",
                    "RU": "Умный счетчик",
                    "SV": "Smart mätare",
                    "TH": "มิเตอร์ไฟฟ้าอัจฉริยะ",
                    "TR": "Akıllı Ölçüm Cihazı",
                    "UK": "Розумний лічильник",
                    "RO": "Contor inteligent",
                    "SL": "Pametni števec"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 18,
                    "type": [
                        "92"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzE5Ij48ZyBpZD0iR3JvdXAgNDQ3MyI+PHBhdGggaWQ9IkVsbGlwc2UgMTIyN19fX19fMF8wX1pYWExIVlJaUEMiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNOS45NjA5NiAxMy40Mzg1QzkuNjg2OTggMTMuMjgwMSA5LjM2ODg4IDEzLjE4OTMgOS4wMjk2IDEzLjE4OTNDOC4yMzgwMyAxMy4xODkzIDcuNTYxNzkgMTMuNjgzMSA3LjI5MjA0IDE0LjM3OTRNOS45NjA5NiAxMy40Mzg1QzkuOTYwOTYgMTIuODk4NiA5LjkyMzU5IDEyLjExODcgOS44NDg4NyAxMS4yNzQ4TTkuOTYwOTYgMTMuNDM4NUMxMC4zODc4IDEzLjY4NTUgMTAuNzA3NiAxNC4wOTcgMTAuODMzNiAxNC41ODYzTTcuMjkyMDQgMTQuMzc5NEM3LjIxMTIxIDE0LjU4OCA3LjE2Njg3IDE0LjgxNDkgNy4xNjY4NyAxNS4wNTIxQzcuMTY2ODcgMTYuMDgwOCA4LjAwMDg0IDE2LjkxNDggOS4wMjk2IDE2LjkxNDhDOS42OTc1MyAxNi45MTQ4IDEwLjI4MzQgMTYuNTYzMiAxMC42MTIxIDE2LjAzNTFNNy4yOTIwNCAxNC4zNzk0QzQuMzEyNTQgMTIuODgwNyAzLjMwODQgNS45MjQ2NyAzLjIyNjY2IDMuMzA2NDRNOS44NDg4NyAxMS4yNzQ4QzkuNzcxMjggMTAuMzk4NCA5LjY1MzM5IDkuNDUzMDEgOS40OTUyNSA4LjYzNTkxQzkuMTU4OTUgNi44OTgyOSA3LjAxMTYyIDIuNDc4NTkgNi41OTc2OCAyLjIxOTg4QzYuMTgzNzQgMS45NjExNyAzLjU5NjYzIDEuODU3NzMgMy4yODYxOCAyLjMyMzQyQzMuMjMyNjQgMi40MDM3MSAzLjIwOTYzIDIuNzYwOTIgMy4yMjY2NiAzLjMwNjQ0TTkuODQ4ODcgMTEuMjc0OEMxMi40NTYxIDExLjMwOTMgMTcuNzg0NCAxMS4zOTg5IDE4LjIzOTcgMTEuNDgxN0MxOC44MDg5IDExLjU4NTIgMTkuNDI5OCAxMS44MDE2IDE5LjQyOTggMTIuNjIwMUMxOS40Mjk4IDEzLjI1NyAxOS40Mjk4IDE0LjAzMDYgMTkuNDI5OCAxNC41ODYzTTEwLjYxMjEgMTYuMDM1MUMxMC43ODk3IDE1Ljc0OTcgMTAuODkyMyAxNS40MTI5IDEwLjg5MjMgMTUuMDUyMUMxMC44OTIzIDE0Ljg5MTIgMTAuODcxOSAxNC43MzUyIDEwLjgzMzYgMTQuNTg2M00xMC42MTIxIDE2LjAzNTFIMTcuODc3NU0xMC44MzM2IDE0LjU4NjNIMTkuNDI5OE0xOS40Mjk4IDE0LjU4NjNDMTkuNDI5OCAxNC43NDQ3IDE5LjQyOTggMTQuODg1NCAxOS40Mjk4IDE1LjAwMDJDMTkuNDI5OCAxNS4xMjMzIDE5LjQyNjkgMTUuMjQ2MyAxOS40MDkyIDE1LjM2MjRNMy4yMjY2NiAzLjMwNjQ0QzIuNzI5MDcgMy40NjE2NyAxLjY4MjE1IDMuODIzODcgMS40NzUxOCA0LjAzMDg0QzEuMjE2NDcgNC4yODk1NSAwLjc1MDc5MiA1LjI3MjY1IDEuMTY0NzMgNi40NjI3M0MxLjU3ODY3IDcuNjUyOCA0LjU3OTcyIDE0LjI3NTggNC44OTAxOCAxNC43NDE1QzUuMTEzODYgMTUuMDc3IDUuNDE4MTUgMTUuNTQ2OCA2LjEzMTk5IDE1LjY2NzJNNy4xNjY4NyAxNS42MjExQzYuNzQ3NDggMTUuNzA3OSA2LjQwODkyIDE1LjcxMzggNi4xMzE5OSAxNS42NjcyTTYuMTMxOTkgMTUuNjY3MkM2LjIxMjcyIDE2LjkzMzMgNi4zNTY0MSAxOS4yNTY0IDYuNDE1NzEgMjAuNDMzMk0xNy44Nzc1IDE2LjAzNTFIMTguMzk0OUMxOS4xNDQyIDE2LjAzNTEgMTkuMzUyNSAxNS43MzQ1IDE5LjQwOTIgMTUuMzYyNE0xNy44Nzc1IDE2LjAzNTFDMTkuNDgxNSAxNy4wNjk5IDE5Ljk0NzIgMTkuNTUzNSAxOS45NDcyIDIwLjQzMzJDMTkuOTQ3MiAyMC42NjY5IDE5Ljk0NzIgMjAuODk1NiAxOS45NDcyIDIxLjEwNThNMTkuOTQ3MiAyMS4xMDU4QzE5Ljk0NzIgMjEuNTI4NSAxOS45NDcyIDIxLjg3NTkgMTkuOTQ3MiAyMi4wMzcyQzE5LjM4NDYgMjIuMDM3MiAxOC43MzQ1IDIyLjAzNzIgMTguMDMyNyAyMi4wMzcyTTE5Ljk0NzIgMjEuMTA1OEgyM0MyMyAyMC43OTU0IDIzIDE5Ljk3NzggMjMgMTkuMTkxM0MyMyAxOC4yMDgyIDIxLjU2MTcgMTUuNjcyOCAxOS40MDkyIDE1LjM2MjRNNi40MTU3MSAyMC40MzMyQzYuNDMyNDMgMjAuNzY1IDYuNDQyNDUgMjEuMDA1NyA2LjQ0MjQ1IDIxLjEwNThDNi40NDI0NSAyMS42NzUgNi43MDExNiAyMi4wMzcyIDcuNjMyNTIgMjIuMDM3MkM4LjExMjI4IDIyLjAzNzIgMTEuNTA2OCAyMi4wMzcyIDE0Ljg3NjUgMjIuMDM3Mk02LjQxNTcxIDIwLjQzMzJIMTEuNTY0OU0xNC44NzY1IDIyLjAzNzJDMTQuODc2NSAyMS4yOTU1IDE0Ljg3NjUgMTkuNjU3IDE0Ljg3NjUgMTkuMDM2MUMxNC44NzY1IDE4LjI2IDE0LjgyNDcgMTcuOTQ5NSAxNS4zOTM5IDE3Ljk0OTVDMTUuOTYzIDE3Ljk0OTUgMTcuMTUzMSAxNy45NDk1IDE3LjYxODggMTcuOTQ5NUMxOC4wODQ1IDE3Ljk0OTUgMTguMDMyNyAxOC4yNiAxOC4wMzI3IDE5LjAzNjFDMTguMDMyNyAxOS42NTcgMTguMDMyNyAyMS4yOTU1IDE4LjAzMjcgMjIuMDM3Mk0xNC44NzY1IDIyLjAzNzJDMTUuOTY0MSAyMi4wMzcyIDE3LjA0OTEgMjIuMDM3MiAxOC4wMzI3IDIyLjAzNzIiLz48Y2lyY2xlIGlkPSJFbGxpcHNlIDEyMjhfX19fXzBfMV9HUlNRQ1VGTVNMIiBjeD0iOS4wMjk1NSIgY3k9IjE1LjA1MTkiIHI9IjAuNzI0MzkzIiBmaWxsPSJibGFjayIvPjxwYXRoIGlkPSJVbmlvbl9fX19fMF8yX0RPTldBRU5FRkQiIGZpbGw9ImJsYWNrIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGQ9Ik0xNi4xNyAxOS4yNDMxQzE2LjE3IDE5LjM4NiAxNi4wNTQyIDE5LjUwMTggMTUuOTExMyAxOS41MDE4QzE1Ljc2ODQgMTkuNTAxOCAxNS42NTI2IDE5LjM4NiAxNS42NTI2IDE5LjI0MzFDMTUuNjUyNiAxOS4xMDAyIDE1Ljc2ODQgMTguOTg0NCAxNS45MTEzIDE4Ljk4NDRDMTYuMDU0MiAxOC45ODQ0IDE2LjE3IDE5LjEwMDIgMTYuMTcgMTkuMjQzMVpNMTcuMTAwNiAxOS4yNDMxQzE3LjEwMDYgMTkuMzg2IDE2Ljk4NDggMTkuNTAxOCAxNi44NDE5IDE5LjUwMThDMTYuNjk5IDE5LjUwMTggMTYuNTgzMiAxOS4zODYgMTYuNTgzMiAxOS4yNDMxQzE2LjU4MzIgMTkuMTAwMiAxNi42OTkgMTguOTg0NCAxNi44NDE5IDE4Ljk4NDRDMTYuOTg0OCAxOC45ODQ0IDE3LjEwMDYgMTkuMTAwMiAxNy4xMDA2IDE5LjI0MzFaTTE2Ljg0MTkgMjAuNDMzM0MxNi45ODQ4IDIwLjQzMzMgMTcuMTAwNiAyMC4zMTc1IDE3LjEwMDYgMjAuMTc0NkMxNy4xMDA2IDIwLjAzMTcgMTYuOTg0OCAxOS45MTU5IDE2Ljg0MTkgMTkuOTE1OUMxNi42OTkgMTkuOTE1OSAxNi41ODMyIDIwLjAzMTcgMTYuNTgzMiAyMC4xNzQ2QzE2LjU4MzIgMjAuMzE3NSAxNi42OTkgMjAuNDMzMyAxNi44NDE5IDIwLjQzMzNaTTE2LjE2OTggMjAuMTc0NkMxNi4xNjk4IDIwLjMxNzUgMTYuMDUzOSAyMC40MzMzIDE1LjkxMTEgMjAuNDMzM0MxNS43NjgyIDIwLjQzMzMgMTUuNjUyMyAyMC4zMTc1IDE1LjY1MjMgMjAuMTc0NkMxNS42NTIzIDIwLjAzMTcgMTUuNzY4MiAxOS45MTU5IDE1LjkxMTEgMTkuOTE1OUMxNi4wNTM5IDE5LjkxNTkgMTYuMTY5OCAyMC4wMzE3IDE2LjE2OTggMjAuMTc0NlpNMTcuNDExNiAyMC44NDcySDE1LjY1MjNWMjEuMzA1NkgxNy40MTE2VjIwLjg0NzJaIiBjbGlwLXJ1bGU9ImV2ZW5vZGQiLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Massage Chair",
                    "TW": "按摩椅",
                    "CN": "按摩椅",
                    "BR": "Cadeira de massagem",
                    "CZ": "Masážní křeslo",
                    "DA": "Massagestol",
                    "DE": "Massagestuhl",
                    "ES": "Silla de masajes",
                    "FI": "Hierontatuoli",
                    "FR": "Fauteuil de massage",
                    "HU": "Masszázsszék",
                    "IT": "Sedia per massaggi",
                    "JP": "マッサージ チェア",
                    "KR": "마사지 의자",
                    "MS": "Massage Chair",
                    "NL": "Massagestoel",
                    "NO": "Massasjestol",
                    "PL": "Fotel do masażu",
                    "RU": "Массажное кресло",
                    "SV": "Massagestol",
                    "TH": "เก้าอี้นวด",
                    "TR": "Masaj Koltuğu",
                    "UK": "Масажне крісло",
                    "RO": "Scaun de masaj",
                    "SL": "Masažni stol"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 19,
                    "type": [
                        "93"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzIwIj48ZyBpZD0iR3JvdXAgNDQ2OCI+PGcgaWQ9Ikdyb3VwIDQ0NjciPjxwYXRoIGlkPSJWZWN0b3IgMjg5Ml9fX19fMF8wX0JRSFFSSEVYTVEiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xOC44ODEzIDguMTQ2MzdMMjAuMzg5NSA2LjgxMjE0SDE3LjAyNUwxNy41MjI1IDguMTQ2MzdNMTkuNTc3NCAxMy42NTczTDE3Ljg0NjkgOS4wMTY1Mk0xNy44NDY5IDkuMDE2NTJMMTMuNjg5NCAxMy4zNjczTTE3Ljg0NjkgOS4wMTY1MkwxNy41MjI1IDguMTQ2MzdNMTMuNjg5NCAxMy4zNjczTDExLjUxNCA4LjE0NjM3TTEzLjY4OTQgMTMuMzY3M0gxMi4wMzYxTTEwLjgxNzkgNkwxMS41MTQgOC4xNDYzN00xNy41MjI1IDguMTQ2MzdIMTEuNTE0TTExLjUxNCA4LjE0NjM3TDEwLjIzNzggMTAuNDY2OE05LjMwOTY2IDZIMTIuNDQyMiIvPjxwYXRoIGlkPSJWZWN0b3IgMjg5M19fX19fMF8xX0xCQklOQktRWUYiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjYiIGQ9Ik01LjQ4MTAxIDE3LjE5NTlMOC44NzUzNiAxMy4xMzUyTDEyLjA5NDEgMTcuMTk1OSIvPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTIyMV9fX19fMF8yX05UUFhaWU5SQVoiIGN4PSI0LjQ4Mjc4IiBjeT0iMTUuMDYxNiIgcj0iMC42IiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4xNjM5Ii8+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMjI0X19fX18wXzNfWUlCVENCRUlVRCIgY3g9IjEzLjcxODUiIGN5PSIxMy4yNTEyIiByPSIwLjUiIGZpbGw9ImJsYWNrIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMC44NTYzMTciLz48Y2lyY2xlIGlkPSJFbGxpcHNlIDEyMjJfX19fXzBfNF9QSFdKRUhCR0laIiBjeD0iOC44NDU2IiBjeT0iMTMuNDgzMyIgcj0iMi44ODA1OSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIvPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTIyM19fX19fMF81X1dPUkpDTUlPRkEiIGN4PSIxOS41MTkzIiBjeT0iMTMuNDgzMyIgcj0iMi44ODA1OSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIvPjwvZz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzZfTk1DQkdLVEhMTyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBkPSJNNC4wODkwNiAxMS44ODY4QzMuNDY3MiAxMS44MzY1IDIuODI2MTMgMTIuMDIzNCAyLjMxMzI3IDEyLjQ1OTVDMS44MDA0MSAxMi44OTU2IDEuNTEyODUgMTMuNDk4MiAxLjQ2MjUyIDE0LjEyMDFNMi42Nzc0NyAxNC4yMTg0QzIuNzAyNjQgMTMuOTA3NSAyLjg0NjQxIDEzLjYwNjEgMy4xMDI4NSAxMy4zODgxQzMuMzU5MjggMTMuMTcwMSAzLjY3OTgxIDEzLjA3NjYgMy45OTA3NCAxMy4xMDE4Ii8+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "Bicycle Trainer",
                    "TW": "自行車訓練器",
                    "CN": "自行车训练器",
                    "BR": "Treinador de bicicleta",
                    "CZ": "Cyklotrenažér",
                    "DA": "Cykeltrainer",
                    "DE": "Fahrradtrainer",
                    "ES": "Rodillo para bicicleta",
                    "FI": "Kuntopyörä",
                    "FR": "Vélo d’appartement",
                    "HU": "Kerékpár edző",
                    "IT": "Trainer ciclismo",
                    "JP": "自転車トレーナー",
                    "KR": "자전거 트레이너",
                    "MS": "Bicycle Trainer",
                    "NL": "Fietstrainer",
                    "NO": "Sykkelruller",
                    "PL": "Trenażer rowerowy",
                    "RU": "Велотренажер",
                    "SV": "Cykeltränare",
                    "TH": "เทรนเนอร์จักรยาน",
                    "TR": "Bisiklet Eğitmeni",
                    "UK": "Велотренажер",
                    "RO": "Trainer pentru bicicletă",
                    "SL": "Sobno kolo za vadbo"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 20,
                    "type": [
                        "94"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzIxIj48ZyBpZD0iR3JvdXAgNDQ2NCI+PHBhdGggaWQ9IlZlY3RvciAyODkxX19fX18wXzBfTkZGUUNDU1ZKRSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE5LjI0MzkgNi40MzQxNUMxOC45NTczIDYuNDM0MTUgMTguNjMxMiA2LjQzNDE1IDE4LjI3OCA2LjQzNDE1TTIuNjA5NzYgMjAuMjc4QzEuNjQzOSAyMC4yNzggMS4zMjE5NSAxOS4zMTIyIDEuMzIxOTUgMTguODgyOUMxLjMyMTk1IDE4LjQ1MzcgMS41MzY1OSAxNy41OTUxIDIuNzE3MDcgMTcuMzgwNUMzLjY2MTQ2IDE3LjIwODggMTMuNjk5MiAxNS42NjM0IDE4LjYgMTQuOTEyMk0yLjYwOTc2IDIwLjI3OFYyMS42NzMyTTIuNjA5NzYgMjAuMjc4QzIuODQ0NDYgMjAuMjc4IDMuOTQ3MzggMjAuMjc4IDUuNTA3MzIgMjAuMjc4TTE4LjI3OCA2LjQzNDE1QzE1LjgwOTggNi40MzQxNSAxMi4wMjEzIDYuNDM0MTUgMTEuMTk1MSA2LjQzNDE1QzEwLjAxNDYgNi40MzQxNSA5LjU4NTM3IDYuMzI2ODMgOS41ODUzNyA1LjY4MjkzQzkuNTg1MzcgNS4wMzkwMiAxMC4yMjkzIDQuOTMxNzEgMTAuOTgwNSA0LjkzMTcxQzExLjU4MTUgNC45MzE3MSAxNC44Nzk3IDQuOTMxNzEgMTYuNDUzNyA0LjkzMTcxTDE4LjYgM0gxOS4yNDM5TDIyLjc4NTQgMTQuOTEyMkMyMi43ODU0IDE1Ljk0OTYgMjIuNzg1NCAxOC4xNTMyIDIyLjc4NTQgMTguNjY4M0MyMi43ODU0IDE5LjI1MzcgMjIuNjk2NyAyMC4xMDUyIDIxLjM5MDIgMjAuMjU1MU0xOC4yNzggNi40MzQxNUwxOS45OTUxIDE0LjkxMjJWMTcuNDg3OFYxOC4zNjIyTTUuNTA3MzIgMjAuMjc4VjIxLjY3MzJNNS41MDczMiAyMC4yNzhDOS4xMjI5MiAyMC4yNzggMTUuMTkzNyAyMC4yNzggMTguNiAyMC4yNzhNMTguNiAyMC4yNzhDMTkuNzcxOCAyMC4yNzggMjAuNjI4MyAyMC4yNzggMjAuOTYxIDIwLjI3OEMyMS4xMTcgMjAuMjc4IDIxLjI1OTcgMjAuMjcwMSAyMS4zOTAyIDIwLjI1NTFNMTguNiAyMC4yNzhWMjEuNjczMk0yMS4zOTAyIDIwLjI1NTFWMjEuNjczMk0xIDIxLjY3MzJIMjMiLz48cGF0aCBpZD0iUmVjdGFuZ2xlIDEwMTk5X19fX18wXzFfRkNPRE5PQUlTSSIgZmlsbD0iYmxhY2siIGQ9Ik0xNy41IDE3LjY2NjZIMTguNDE2NjY3VjE4LjU4MzI2N0gxNy41eiIvPjxwYXRoIGlkPSJSZWN0YW5nbGUgMTAyMDBfX19fXzBfMl9TRUlQSkpaQUpBIiBmaWxsPSJibGFjayIgZD0iTTE1LjY2NjYgMTcuNjY2NkgxNi41ODMyNjdWMTguNTgzMjY3SDE1LjY2NjZ6Ii8+PHBhdGggaWQ9IlJlY3RhbmdsZSAxMDIwMV9fX19fMF8zX1NZUlNUV1lITUkiIGZpbGw9ImJsYWNrIiBkPSJNMTMuODMzNCAxNy42NjY2SDE0Ljc1MDA2N1YxOC41ODMyNjdIMTMuODMzNHoiLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Treadmill",
                    "TW": "跑步機",
                    "CN": "跑步机",
                    "BR": "Esteira",
                    "CZ": "Běžecký pás",
                    "DA": "Løbebånd",
                    "DE": "Laufband",
                    "ES": "Cinta de correr",
                    "FI": "Juoksumatto",
                    "FR": "Tapis roulant",
                    "HU": "Taposómalom",
                    "IT": "Tapis roulant",
                    "JP": "踏み車",
                    "KR": "러닝머신",
                    "MS": "Treadmill",
                    "NL": "Loopband",
                    "NO": "Tredemølle",
                    "PL": "Bieżnia stacjonarna",
                    "RU": "Беговая дорожка",
                    "SV": "Löpband",
                    "TH": "ลู่วิ่งไฟฟ้า",
                    "TR": "Koşu Bandı",
                    "UK": "Бігова доріжка",
                    "RO": "Bandă de alergat",
                    "SL": "Tekalna steza"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 21,
                    "type": [
                        "95"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzIyIj48ZyBpZD0iR3JvdXAgNDQ2NSI+PHBhdGggaWQ9IlZlY3RvciAyODg3X19fX18wXzBfS1hMVVRWV0ZaUiIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTIyLjM3MDggM0wxOC40NTYxIDUuMTA1MjZMMjAuNzI4IDkuMTA1MjZNMTIuMDY3MyAyMC42ODQySDE5LjA4NTJMMjMgMTMuMTA1M0wyMC43MjggOS4xMDUyNk0xMi4wNjczIDIwLjY4NDJIMy4wNDkzNUMyLjA0NzM1IDIwLjQ3MzcgMC4yOTUwMjcgMTkuNjczNyAxLjMwMTY4IDE4LjE1NzlDMi41NiAxNi4yNjMyIDUuMTQ2NTQgMTIuMTkzIDkuOTAwMTggMTIuMTkzQzE0LjAyOTIgMTIuMTkzIDE0Ljk0MSAxNC43ODczIDE1LjEwOTMgMTYuODk0N00xMi4wNjczIDIwLjY4NDJDMTMuMDkyNiAyMC40NTAzIDE1LjE0MzIgMTkuNTQ3NCAxNS4xNDMyIDE3LjgwN0MxNS4xNDMyIDE3LjUyMTIgMTUuMTM0NyAxNy4yMTM1IDE1LjEwOTMgMTYuODk0N00yMC43MjggOS4xMDUyNkwxNi45ODggMTYuODk0N0gxNS4xMDkzIi8+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMjE5X19fX18wXzFfUENFWlBOR0pPWCIgY3g9IjcuMDQ1NDUiIGN5PSIxNy4wNDU1IiByPSIxLjQ0NTQ1IiBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuMiIvPjxwYXRoIGlkPSJWZWN0b3IgMjg4OF9fX19fMF8yX0hOT0FPS1hGTlQiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik02LjkwOTE4IDE2LjkwOTJMMTAuNTkxIDE3LjQ1NDZNMTEuNSAxNi41TDEwLjUgMTguNSIvPjxwYXRoIGlkPSJSZWN0YW5nbGUgMTAxOThfX19fXzBfM19DS05SWEVCVFhNIiBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTUuMTY5MzIgNy42NjIzN0w4LjM4NzgyIDcuODMxNzdDOC41MjUwNyA3LjgzODk5IDguNjM1ODggNy45NDY0NyA4LjY0NzMgOC4wODM0M0M4LjY1OTk0IDguMjM1MTYgOC41NDY4NCA4LjM2ODI2IDguMzk1MDYgOC4zODAyN0w1LjE4MjE1IDguNjM0NTVDNC45MDgwNCA4LjY1NjI0IDQuNjcwMyA4LjQ0Njk0IDQuNjU3MDkgOC4xNzIyOUM0LjY0MzIzIDcuODg0MjEgNC44ODEzIDcuNjQ3MjEgNS4xNjkzMiA3LjY2MjM3WiIvPjxwYXRoIGlkPSJWZWN0b3IgMjg4OV9fX19fMF80X1NZS0tOVktWWVIiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik02LjcyNzU0IDguOTA5MThDNi43MzM0MyA4Ljk3ODk1IDYuOTExNjMgMTEuMDg5NiA3IDEyLjEzNjIiLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Flywheel",
                    "TW": "飛輪",
                    "CN": "飞轮",
                    "BR": "Flywheel",
                    "CZ": "Setrvačník",
                    "DA": "Svinghjul",
                    "DE": "Schwungrad",
                    "ES": "Volante de inercia",
                    "FI": "Vauhtipyörä",
                    "FR": "Volant d'inertie",
                    "HU": "Szobabicikli lendkerék",
                    "IT": "Volano",
                    "JP": "フライホイール",
                    "KR": "플라이휠",
                    "MS": "Flywheel",
                    "NL": "Vliegwiel",
                    "NO": "Svinghjul",
                    "PL": "Koło zamachowe",
                    "RU": "Маховик",
                    "SV": "Balanshjul",
                    "TH": "ฟลายวีล",
                    "TR": "Volan",
                    "UK": "Махове колесо",
                    "RO": "Volant",
                    "SL": "Vztrajnik"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 22,
                    "type": [
                        "96"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSI4NTU2NDg3X2NhYmluZXRfbG9ja2VyX2xvY2tlcnNfc2Nob29sX2RlY29yYWlvbl9pY29uIDMiIGNsaXAtcGF0aD0idXJsKCNjbGlwMF8zMTVfMjExNikiPjxnIGlkPSJHcm91cCA0NTY5Ij48cGF0aCBpZD0iUmVjdGFuZ2xlIDEwMjU4X19fX18wXzBfVVlNVkZDSU9EWSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIwLjgiIGQ9Ik03LjA4NDU3IDIxLjcwMjJMNi45OTExNSAyMS40MzM2SDYuNzA2NzdINC40VjhDNC40IDcuNjY4NjMgNC42Njg2MyA3LjQgNSA3LjRIOS4yODQyMVYyMS43NTc5SDcuMTAzOTNMNy4wODQ1NyAyMS43MDIyWk0xMC4wODQyIDIxLjc1NzlWNy40SDE0LjUzNTNWMjEuNzU3OUgxMC4wODQyWk0xNS4zMzUzIDIxLjc1NzlWNy40SDE5LjQyMTFDMTkuNzUyNCA3LjQgMjAuMDIxMSA3LjY2ODYzIDIwLjAyMTEgOFYyMS40MzM2SDE3LjkwOTFWMjEuODIzOEwxNy44NDIzIDIxLjc1NzlIMTcuNjc4MkgxNS4zMzUzWk0xOC4zMjQ0IDIyLjIzMzZIMjAuMDIxMVYyMi40NzI5QzIwLjAyMTEgMjMuMDA1NSAxOS4zNzg3IDIzLjI3NDEgMTguOTk5NiAyMi44OTk5TDE4LjMyNDQgMjIuMjMzNlpNNC40IDIyLjIzMzZINi4xODA5Nkw1LjQyNTk1IDIyLjk5NDdDNS4wNDg2NyAyMy4zNzUgNC40IDIzLjEwNzggNC40IDIyLjU3MjFWMjIuMjMzNlpNNS45ODQ5NiA5LjU2NTQxSDguMTUwMzhWOC43NjU0MUg1Ljk4NDk2VjkuNTY1NDFaTTUuOTg0OTYgMTAuNjQ4MUg4LjE1MDM4VjkuODQ4MTJINS45ODQ5NlYxMC42NDgxWk01Ljk4NDk2IDE5LjMwOThIOC4xNTAzOFYxOC41MDk4SDUuOTg0OTZWMTkuMzA5OFpNNS45ODQ5NiAyMC43NTM0SDguMTUwMzhWMTkuOTUzNEg1Ljk4NDk2VjIwLjc1MzRaTTExLjIxOCA5LjU2NTQxSDEzLjM4MzVWOC43NjU0MUgxMS4yMThWOS41NjU0MVpNMTEuMjE4IDEwLjY0ODFIMTMuMzgzNVY5Ljg0ODEySDExLjIxOFYxMC42NDgxWk0xMS4yMTggMTkuMzA5OEgxMy4zODM1VjE4LjUwOThIMTEuMjE4VjE5LjMwOThaTTExLjIxOCAyMC43NTM0SDEzLjM4MzVWMTkuOTUzNEgxMS4yMThWMjAuNzUzNFpNMTYuNDUxMSA5LjU2NTQxSDE4LjYxNjVWOC43NjU0MUgxNi40NTExVjkuNTY1NDFaTTE2LjQ1MTEgMTAuNjQ4MUgxOC42MTY1VjkuODQ4MTJIMTYuNDUxMVYxMC42NDgxWk0xNi40NTExIDE5LjMwOThIMTguNjE2NVYxOC41MDk4SDE2LjQ1MTFWMTkuMzA5OFpNMTYuNDUxMSAyMC43NTM0SDE4LjYxNjVWMTkuOTUzNEgxNi40NTExVjIwLjc1MzRaIi8+PHBhdGggaWQ9IlJlY3RhbmdsZSAxMDI2MF9fX19fMF8xX05TVVJMR1pIWkUiIGZpbGw9ImJsYWNrIiBkPSJNNi4yMDg3NCAxNS4zNjM4QzYuMjA4NzQgMTQuOTEyIDYuNTc1MDUgMTQuNTQ1NyA3LjAyNjkyIDE0LjU0NTdWMTQuNTQ1N0M3LjQ3ODc5IDE0LjU0NTcgNy44NDUxIDE0LjkxMiA3Ljg0NTEgMTUuMzYzOFYxNy4wMDAyQzcuODQ1MSAxNy40NTIxIDcuNDc4NzkgMTcuODE4NCA3LjAyNjkyIDE3LjgxODRWMTcuODE4NEM2LjU3NTA1IDE3LjgxODQgNi4yMDg3NCAxNy40NTIxIDYuMjA4NzQgMTcuMDAwMlYxNS4zNjM4WiIvPjxwYXRoIGlkPSJSZWN0YW5nbGUgMTAyNjFfX19fXzBfMl9BTENSTVBGUkRFIiBmaWxsPSJibGFjayIgZD0iTTExLjUyNzEgMTUuMzYzOEMxMS41MjcxIDE0LjkxMiAxMS44OTM0IDE0LjU0NTcgMTIuMzQ1MyAxNC41NDU3VjE0LjU0NTdDMTIuNzk3MiAxNC41NDU3IDEzLjE2MzUgMTQuOTEyIDEzLjE2MzUgMTUuMzYzOFYxNy4wMDAyQzEzLjE2MzUgMTcuNDUyMSAxMi43OTcyIDE3LjgxODQgMTIuMzQ1MyAxNy44MTg0VjE3LjgxODRDMTEuODkzNCAxNy44MTg0IDExLjUyNzEgMTcuNDUyMSAxMS41MjcxIDE3LjAwMDJWMTUuMzYzOFoiLz48cGF0aCBpZD0iUmVjdGFuZ2xlIDEwMjYyX19fX18wXzNfR0xKTktFVElNUSIgZmlsbD0iYmxhY2siIGQ9Ik0xNi42ODE2IDE1LjM2MzhDMTYuNjgxNiAxNC45MTIgMTcuMDQ4IDE0LjU0NTcgMTcuNDk5OCAxNC41NDU3VjE0LjU0NTdDMTcuOTUxNyAxNC41NDU3IDE4LjMxOCAxNC45MTIgMTguMzE4IDE1LjM2MzhWMTcuMDAwMkMxOC4zMTggMTcuNDUyMSAxNy45NTE3IDE3LjgxODQgMTcuNDk5OCAxNy44MTg0VjE3LjgxODRDMTcuMDQ4IDE3LjgxODQgMTYuNjgxNiAxNy40NTIxIDE2LjY4MTYgMTcuMDAwMlYxNS4zNjM4WiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfNF9KWUFEWEtITk1JIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTYgMy42NTY4NUMxNC45NzYzIDIuNjMzMTYgMTMuNTYyMSAyIDEyIDJDMTAuNDM3OSAyIDkuMDIzNjkgMi42MzMxNiA4IDMuNjU2ODVNMTAgNS42NTY4NUMxMC41MTE4IDUuMTQ1MDEgMTEuMjE5IDQuODI4NDMgMTIgNC44Mjg0M0MxMi43ODEgNC44Mjg0MyAxMy40ODgyIDUuMTQ1MDEgMTQgNS42NTY4NSIvPjwvZz48L2c+PC9nPjxkZWZzPjxjbGlwUGF0aCBpZD0iY2xpcDBfMzE1XzIxMTYiPjxwYXRoIGZpbGw9IndoaXRlIiBkPSJNMCAwSDI0VjI0SDB6Ii8+PC9jbGlwUGF0aD48L2RlZnM+PC9zdmc+",
                    "EN": "Cupboard",
                    "TW": "櫥櫃",
                    "CN": "橱柜",
                    "BR": "Armário",
                    "CZ": "Skříň",
                    "DA": "Skab",
                    "DE": "Schrank",
                    "ES": "Aparador",
                    "FI": "Kaappi",
                    "FR": "Placard",
                    "HU": "Szekrény",
                    "IT": "Armadietto",
                    "JP": "戸棚",
                    "KR": "찬장",
                    "MS": "Cupboard",
                    "NL": "Kast",
                    "NO": "Skap",
                    "PL": "Kredens",
                    "RU": "Буфет",
                    "SV": "Skåp",
                    "TH": "ตู้",
                    "TR": "Dolap",
                    "UK": "Шафа",
                    "RO": "Dulap",
                    "SL": "Omarica"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 23,
                    "type": [
                        "97"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzA1Ij48ZyBpZD0iR3JvdXAgNDQ5MyI+PHBhdGggaWQ9IlJlY3RhbmdsZSAxMDIyMl9fX19fMF8wX1FYVE5OTVhaS0kiIHN0cm9rZT0iYmxhY2siIGQ9Ik0yMCAzSDZDNS40NDc3MiAzIDUgMy40NDc3MiA1IDRWMjBDNSAyMC41NTIzIDUuNDQ3NzIgMjEgNiAyMUgyMCIvPjxwYXRoIGlkPSJWZWN0b3IgMjg5NV9fX19fMF8xX0dTWkpGSlRSU04iIHN0cm9rZT0iYmxhY2siIGQ9Ik04LjUgM1Y1QzguNSA1LjU1MjI4IDguOTQ3NzIgNiA5LjUgNkgxN0MxNy41NTIzIDYgMTggNS41NTIyOCAxOCA1VjMiLz48cGF0aCBpZD0iUmVjdGFuZ2xlIDEwMjIzX19fX18wXzJfS0hSS1BSVkZNTyIgc3Ryb2tlPSJibGFjayIgZD0iTTguNSAxMi41SDE3LjVWMTZDMTcuNSAxOC40ODUzIDE1LjQ4NTMgMjAuNSAxMyAyMC41QzEwLjUxNDcgMjAuNSA4LjUgMTguNDg1MyA4LjUgMTZWMTIuNVoiLz48Y2lyY2xlIGlkPSJFbGxpcHNlIDEyNTdfX19fXzBfM19YTkxCRVNYRFRZIiBjeD0iMTMiIGN5PSI5IiByPSIxIiBmaWxsPSJibGFjayIvPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Coffee Machine",
                    "TW": "咖啡機",
                    "CN": "咖啡机",
                    "BR": "Máquina de café",
                    "CZ": "Kávovar",
                    "DA": "Kaffemaskine",
                    "DE": "Kaffeemaschine",
                    "ES": "Máquina de café",
                    "FI": "Kahvinkeitin",
                    "FR": "Machine à café",
                    "HU": "Kávéfőzőgép",
                    "IT": "Macchina per il caffè",
                    "JP": "コーヒー マシン",
                    "KR": "커피 머신",
                    "MS": "Coffee Machine",
                    "NL": "Koffiezetapparaat",
                    "NO": "Kaffemaskin",
                    "PL": "Ekspres do kawy",
                    "RU": "Кофеварка",
                    "SV": "Kaffebryggare",
                    "TH": "เครื่องชงกาแฟ",
                    "TR": "Kahve Makinesi",
                    "UK": "Кавомашина",
                    "RO": "Cafetieră",
                    "SL": "Avtomat za kavo"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 24,
                    "type": [
                        "98"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzA2Ij48ZyBpZD0iR3JvdXAgNDQ4NSI+PHBhdGggaWQ9IlJlY3RhbmdsZSAxMDIxNV9fX19fMF8wX0VHUUNQUFJDR1EiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgZD0iTTEyLjAwODIgMTkuOTI4MVYxOS41MjE2TTcuMjE5NzEgMTkuOTI4MVYxNC42ODc5TTguODAwODIgMTIuMTEyOUgxMi4wMDgyTTEyLjAwODIgMTIuMTEyOVYxMS4xMTkxQzEyLjAwODIgMTAuMjQ1OSAxMS4zMDAzIDkuNTM3OTkgMTAuNDI3MSA5LjUzNzk5VjkuNTM3OTlNMTIuMDA4MiAxMi4xMTI5VjE5LjUyMTZNMTAuNDI3MSA5LjUzNzk5VjIuMjY5MzFDMTAuNDI3MSAxLjg4MDc4IDEwLjIwMjEgMS41Mjc0MiA5Ljg0OTk5IDEuMzYzMTJMOS4wNzE4NyAxTTEwLjQyNzEgOS41Mzc5OUg5LjA3MTg3TTkuMDcxODcgMUg4LjIxOTcxQzcuNjY3NDMgMSA3LjIxOTcxIDEuNDQ3NzIgNy4yMTk3MSAyVjIuMzQ5MDhDNy4yMTk3MSAyLjkwMTM2IDcuNjY3NDMgMy4zNDkwOCA4LjIxOTcxIDMuMzQ5MDhIOS4wNzE4N005LjA3MTg3IDFWOS41Mzc5OU05LjA3MTg3IDkuNTM3OTlWOS41Mzc5OUM4LjA0ODk1IDkuNTM3OTkgNy4yMTk3MSAxMC4zNjcyIDcuMjE5NzEgMTEuMzkwMVYxMi40NzQzTTcuMjE5NzEgMTIuNDc0M0g3LjEwNjc4QzYuNDk1NTIgMTIuNDc0MyA2IDEyLjk2OTkgNiAxMy41ODExVjEzLjU4MTFDNiAxNC4xOTI0IDYuNDk1NTIgMTQuNjg3OSA3LjEwNjc4IDE0LjY4NzlINy4yMTk3MU03LjIxOTcxIDEyLjQ3NDNWMTQuNjg3OU0xMS41NTY1IDEwLjk1MTZDMTEuOTc4MSAxMC44MDEgMTIuODEyMyAxMS4yNzcyIDEzLjMxODMgMTJDMTMuODYwNCAxMS41NDgzIDE0LjQ1NzEgMTAuNjI1IDE2LjA4MzMgMTAuNjI1QzE5LjI5MTcgMTAuNjI1IDE5LjA3MzggMTMuMzQxOSAxOC42MDM3IDE2LjY3NTZDMTguMTA2OCAyMC4xOTkyIDE2LjUyNTcgMjIuNjgzOCAxNS44MDI5IDIyLjY4MzhDMTQuNDAyNSAyMi42ODM4IDE2LjI5OTggMTguNTI3NyAxMy4zMTgzIDE4LjUyNzdDMTIuMjI1IDE4LjUyNzcgMTIuMDA4MiAxOS4wMjQ2IDEyLjAwODIgMTkuNTIxNk02LjQ1MTc1IDIzVjIxLjkyODFDNi40NTE3NSAyMC44MjM2IDcuMzQ3MTggMTkuOTI4MSA4LjQ1MTc1IDE5LjkyODFIMTAuNzc2MkMxMS44ODA3IDE5LjkyODEgMTIuNzc2MiAyMC44MjM2IDEyLjc3NjIgMjEuOTI4MVYyM0g2LjQ1MTc1WiIvPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Electric Toothbrush",
                    "TW": "電動牙刷",
                    "CN": "电动牙刷",
                    "BR": "Escova de dentes elétrica",
                    "CZ": "Elektrický zubní kartáček",
                    "DA": "Eltandbørste",
                    "DE": "Elektrische Zahnbürste",
                    "ES": "Cepillo de dientes eléctrico",
                    "FI": "Sähköinen hammasharja",
                    "FR": "Brosse à dents électrique",
                    "HU": "Elektromos fogkefe",
                    "IT": "Spazzolino elettrico",
                    "JP": "電動歯ブラシ",
                    "KR": "전동 칫솔",
                    "MS": "Electric Toothbrush",
                    "NL": "Elektrische tandenborstel",
                    "NO": "Elektrisk tannbørste",
                    "PL": "Elektryczna szczoteczka do zębów",
                    "RU": "Электрическая зубная щетка",
                    "SV": "Elektrisk tandborste",
                    "TH": "แปรงสีฟันไฟฟ้า",
                    "TR": "Elektrikli Diş Fırçası",
                    "UK": "Електрична зубна щітка",
                    "RO": "Periuță de dinți electrică",
                    "SL": "Električna zobna ščetka"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 25,
                    "type": [
                        "99"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzA0Ij48ZyBpZD0iR3JvdXAgNDU0MyI+PGcgaWQ9IlJlY3RhbmdsZSAxMDI1N19fX19fMF8wX1BUTkVGRERWQUEiPjxwYXRoIGZpbGw9ImJsYWNrIiBkPSJNMTEuNTQ0NyAwSDEyLjMzNzZWOC44NjkyMkgxMS41NDQ3VjBaIi8+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik0xMi41Mjg1IDIuNzQ1OTNIMTMuMTE1OVY2LjEyMzI5SDEyLjUyODVWMi43NDU5M1oiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTEyLjUyODUgMi43NDU5M0gxMy4xMTU5VjYuMTIzMjlIMTIuNTI4NVYyLjc0NTkzWiIvPjxwYXRoIGZpbGw9ImJsYWNrIiBkPSJNMTIuNTI4NSAyLjc0NTkzSDEzLjExNTlWNi4xMjMyOUgxMi41Mjg1VjIuNzQ1OTNaIi8+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik0xMy4xMTU5IDMuNDUwNzdWMy40NTA3N0MxMy4zMTg2IDMuNDUwNzcgMTMuNDgzIDMuNjE1MTMgMTMuNDgzIDMuODE3ODhWNC45OTI2MUMxMy40ODMgNS4xOTUzNSAxMy4zMTg2IDUuMzU5NzEgMTMuMTE1OSA1LjM1OTcxVjUuMzU5NzFWMy40NTA3N1oiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTEzLjYyOTggNC4xNTU2MUgxNS4zMTg1VjQuNjg0MjRIMTMuNjI5OFY0LjE1NTYxWiIvPjxwYXRoIGZpbGw9ImJsYWNrIiBkPSJNMTMuNjI5OCA0LjE1NTYxSDE1LjMxODVWNC42ODQyNEgxMy42Mjk4VjQuMTU1NjFaIi8+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik0xMy42Mjk4IDQuMTU1NjFIMTUuMzE4NVY0LjY4NDI0SDEzLjYyOThWNC4xNTU2MVoiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTEzLjQyNDMgNC4yMjkwM0gxMy43NDczVjQuNjI1NUgxMy40MjQzVjQuMjI5MDNaIi8+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik0xMy40MjQzIDQuMjI5MDNIMTMuNzQ3M1Y0LjYyNTVIMTMuNDI0M1Y0LjIyOTAzWiIvPjxwYXRoIGZpbGw9ImJsYWNrIiBkPSJNMTMuNDI0MyA0LjIyOTAzSDEzLjc0NzNWNC42MjU1SDEzLjQyNDNWNC4yMjkwM1oiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE1LjU2ODEgNC4xNTU2MUgxNi42MjU0SDE3LjY4MjdWNC42ODQyNEgxNS41NjgxVjQuMTU1NjFaIi8+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik0xNS41NjgxIDQuMTU1NjFIMTYuNjI1NEgxNy42ODI3VjQuNjg0MjRIMTUuNTY4MVY0LjE1NTYxWiIvPjxwYXRoIGZpbGw9ImJsYWNrIiBkPSJNMTUuNTY4MSA0LjE1NTYxSDE2LjYyNTRIMTcuNjgyN1Y0LjY4NDI0SDE1LjU2ODFWNC4xNTU2MVoiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE1LjQ2NTQgMy45Nzk0SDE1LjY0MTZWNC44ODk4MkgxNS40NjU0VjMuOTc5NFoiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE1LjQ2NTQgMy45Nzk0SDE1LjY0MTZWNC44ODk4MkgxNS40NjU0VjMuOTc5NFoiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE1LjQ2NTQgMy45Nzk0SDE1LjY0MTZWNC44ODk4MkgxNS40NjU0VjMuOTc5NFoiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE3LjgxNDggMy45Nzk0SDE5LjMxMjZWNC44ODk4MkgxNy44MTQ4VjMuOTc5NFoiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE3LjgxNDggMy45Nzk0SDE5LjMxMjZWNC44ODk4MkgxNy44MTQ4VjMuOTc5NFoiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE3LjgxNDggMy45Nzk0SDE5LjMxMjZWNC44ODk4MkgxNy44MTQ4VjMuOTc5NFoiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE4LjE2NzIgMy4yMTU4M0gxOC45NzQ5VjExLjU3NzJIMTguMTY3MlYzLjIxNTgzWiIvPjxwYXRoIGZpbGw9ImJsYWNrIiBkPSJNMTguMTY3MiAzLjIxNTgzSDE4Ljk3NDlWMTEuNTc3MkgxOC4xNjcyVjMuMjE1ODNaIi8+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik0xOC4xNjcyIDMuMjE1ODNIMTguOTc0OVYxMS41NzcySDE4LjE2NzJWMy4yMTU4M1oiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE3LjgxNDggMy45Nzk0SDE5LjMxMjZWNC44ODk4MkgxNy44MTQ4VjMuOTc5NFoiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE3LjgxNDggMy45Nzk0SDE5LjMxMjZWNC44ODk4MkgxNy44MTQ4VjMuOTc5NFoiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE3LjgxNDggMy45Nzk0SDE5LjMxMjZWNC44ODk4MkgxNy44MTQ4VjMuOTc5NFoiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE3LjY4MjcgMTEuNDE1N0gxOS4xODA0VjExLjY5NDdIMTcuNjgyN1YxMS40MTU3WiIvPjxwYXRoIGZpbGw9ImJsYWNrIiBkPSJNMTcuNjgyNyAxMS40MTU3SDE5LjE4MDRWMTEuNjk0N0gxNy42ODI3VjExLjQxNTdaIi8+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik0xNy42ODI3IDExLjQxNTdIMTkuMTgwNFYxMS42OTQ3SDE3LjY4MjdWMTEuNDE1N1oiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE3LjY4MjcgMTEuNDE1N0gxOS4xODA0VjExLjY5NDdIMTcuNjgyN1YxMS40MTU3WiIvPjxwYXRoIGZpbGw9ImJsYWNrIiBkPSJNMTcuNjgyNyAxMS40MTU3SDE5LjE4MDRWMTEuNjk0N0gxNy42ODI3VjExLjQxNTdaIi8+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik0xNy42ODI3IDExLjQxNTdIMTkuMTgwNFYxMS42OTQ3SDE3LjY4MjdWMTEuNDE1N1oiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE3LjcxMiAxMy4wMDE2SDE5LjIwOThWMTMuMjgwNkgxNy43MTJWMTMuMDAxNloiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE3LjcxMiAxMy4wMDE2SDE5LjIwOThWMTMuMjgwNkgxNy43MTJWMTMuMDAxNloiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE3LjcxMiAxMy4wMDE2SDE5LjIwOThWMTMuMjgwNkgxNy43MTJWMTMuMDAxNloiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE3LjcxMiAxMy4wMDE2SDE5LjIwOThWMTMuMjgwNkgxNy43MTJWMTMuMDAxNloiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE3LjcxMiAxMy4wMDE2SDE5LjIwOThWMTMuMjgwNkgxNy43MTJWMTMuMDAxNloiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE3LjcxMiAxMy4wMDE2SDE5LjIwOThWMTMuMjgwNkgxNy43MTJWMTMuMDAxNloiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE4Ljk3NDkgMTEuNDE1N0gxOS4yMDk4VjEzLjI4MDZIMTguOTc0OVYxMS40MTU3WiIvPjxwYXRoIGZpbGw9ImJsYWNrIiBkPSJNMTguOTc0OSAxMS40MTU3SDE5LjIwOThWMTMuMjgwNkgxOC45NzQ5VjExLjQxNTdaIi8+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik0xOC45NzQ5IDExLjQxNTdIMTkuMjA5OFYxMy4yODA2SDE4Ljk3NDlWMTEuNDE1N1oiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTE4Ljk3NDkgMTEuNDE1N0gxOS4yMDk4VjEzLjI4MDZIMTguOTc0OVYxMS40MTU3WiIvPjxwYXRoIGZpbGw9ImJsYWNrIiBkPSJNMTguOTc0OSAxMS40MTU3SDE5LjIwOThWMTMuMjgwNkgxOC45NzQ5VjExLjQxNTdaIi8+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik0xOC45NzQ5IDExLjQxNTdIMTkuMjA5OFYxMy4yODA2SDE4Ljk3NDlWMTEuNDE1N1oiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTQuMTI5MiAxMS4wNDg2SDUuMTg2NDZIOC4zODc2VjExLjU3NzJINC4xMjkyVjExLjA0ODZaIi8+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik00LjEyOTIgMTEuMDQ4Nkg1LjE4NjQ2SDguMzg3NlYxMS41NzcySDQuMTI5MlYxMS4wNDg2WiIvPjxwYXRoIGZpbGw9ImJsYWNrIiBkPSJNNC4xMjkyIDExLjA0ODZINS4xODY0Nkg4LjM4NzZWMTEuNTc3Mkg0LjEyOTJWMTEuMDQ4NloiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTIgMTEuOTE1SDMuMDU3MjZIMTguNzM5OVYxMi43ODEzSDJWMTEuOTE1WiIvPjxwYXRoIGZpbGw9ImJsYWNrIiBkPSJNMiAxMS45MTVIMy4wNTcyNkgxOC43Mzk5VjEyLjc4MTNIMlYxMS45MTVaIi8+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik0yIDExLjkxNUgzLjA1NzI2SDE4LjczOTlWMTIuNzgxM0gyVjExLjkxNVoiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTMuNDk3NzggMTMuMDAxNkg0LjU1NTA0SDE2LjYyNTRIMTcuMjcxNVYxMy42MDM2SDE2LjYyNTRMMTIuMzM3NiAxMy45MTJIMTAuNDU4MUg4LjY5NTk3TDQuNTU1MDQgMTMuNjAzNkgzLjQ5Nzc4VjEzLjAwMTZaIi8+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik0zLjQ5Nzc4IDEzLjAwMTZINC41NTUwNEgxNi42MjU0SDE3LjI3MTVWMTMuNjAzNkgxNi42MjU0TDEyLjMzNzYgMTMuOTEySDEwLjQ1ODFIOC42OTU5N0w0LjU1NTA0IDEzLjYwMzZIMy40OTc3OFYxMy4wMDE2WiIvPjxwYXRoIGZpbGw9ImJsYWNrIiBkPSJNMy40OTc3OCAxMy4wMDE2SDQuNTU1MDRIMTYuNjI1NEgxNy4yNzE1VjEzLjYwMzZIMTYuNjI1NEwxMi4zMzc2IDEzLjkxMkgxMC40NTgxSDguNjk1OTdMNC41NTUwNCAxMy42MDM2SDMuNDk3NzhWMTMuMDAxNloiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTkuNDc0MjMgMTMuNzY1MkgxMS4yMzYzVjE2LjkwNzZIOS40NzQyM1YxMy43NjUyWiIvPjxwYXRoIGZpbGw9ImJsYWNrIiBkPSJNOS40NzQyMyAxMy43NjUySDExLjIzNjNWMTYuOTA3Nkg5LjQ3NDIzVjEzLjc2NTJaIi8+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik05LjQ3NDIzIDEzLjc2NTJIMTEuMjM2M1YxNi45MDc2SDkuNDc0MjNWMTMuNzY1MloiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTkuMzcxNDQgMTYuNDA4M0gxMS4zMzkxVjE5LjU1MDdIOS4zNzE0NFYxNi40MDgzWiIvPjxwYXRoIGZpbGw9ImJsYWNrIiBkPSJNOS4zNzE0NCAxNi40MDgzSDExLjMzOTFWMTkuNTUwN0g5LjM3MTQ0VjE2LjQwODNaIi8+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik05LjM3MTQ0IDE2LjQwODNIMTEuMzM5MVYxOS41NTA3SDkuMzcxNDRWMTYuNDA4M1oiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTkuMTIxODEgMTkuMjg2NEgxMS41NDQ3VjIzLjA0NTVIOS4xMjE4MVYxOS4yODY0WiIvPjxwYXRoIGZpbGw9ImJsYWNrIiBkPSJNOS4xMjE4MSAxOS4yODY0SDExLjU0NDdWMjMuMDQ1NUg5LjEyMTgxVjE5LjI4NjRaIi8+PHBhdGggZmlsbD0iYmxhY2siIGQ9Ik05LjEyMTgxIDE5LjI4NjRIMTEuNTQ0N1YyMy4wNDU1SDkuMTIxODFWMTkuMjg2NFoiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTMuNDk3NzggMjMuNDEyNkg0LjU1NTA0TDkuMTIxODEgMjMuMDQ1NUgxMC40NDM0SDExLjU0NDdMMTcuMjcxNSAyMy40MTI2VjI0SDMuNDk3NzhWMjMuNDEyNloiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTMuNDk3NzggMjMuNDEyNkg0LjU1NTA0TDkuMTIxODEgMjMuMDQ1NUgxMC40NDM0SDExLjU0NDdMMTcuMjcxNSAyMy40MTI2VjI0SDMuNDk3NzhWMjMuNDEyNloiLz48cGF0aCBmaWxsPSJibGFjayIgZD0iTTMuNDk3NzggMjMuNDEyNkg0LjU1NTA0TDkuMTIxODEgMjMuMDQ1NUgxMC40NDM0SDExLjU0NDdMMTcuMjcxNSAyMy40MTI2VjI0SDMuNDk3NzhWMjMuNDEyNloiLz48L2c+PGcgaWQ9Ikdyb3VwIDQ1NDEiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMV9QV1ZaT0laSkRYIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS1taXRlcmxpbWl0PSIxMCIgc3Ryb2tlLXdpZHRoPSIwLjgiIGQ9Ik0yMyAyMS4zMzMxTDIxIDIzLjMzMzFMMTkgMjEuMzMzMSIvPjxwYXRoIGlkPSJWZWN0b3IgNV9fX19fMF8yX0tRQUZSRElSUEUiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIwLjgiIGQ9Ik0yMS4wMDAyIDIyLjk5OThWMTkuNjY2NSIvPjwvZz48ZyBpZD0iR3JvdXAgNDU0MiI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8zX05FRlNOUEZDRFUiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLW1pdGVybGltaXQ9IjEwIiBzdHJva2Utd2lkdGg9IjAuOCIgZD0iTTE5IDE1Ljk5OTlMMjEgMTMuOTk5OUwyMyAxNS45OTk5Ii8+PHBhdGggaWQ9IlZlY3RvciA1X19fX18wXzRfQ1dZTVJaRUxCUyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2Utd2lkdGg9IjAuOCIgZD0iTTIwLjk5OTggMTQuMzMzMkwyMC45OTk4IDE3LjY2NjUiLz48L2c+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "Electric Adjustable Desk",
                    "TW": "電動升降桌",
                    "CN": "电动调节桌",
                    "BR": "Mesa elétrica ajustável",
                    "CZ": "Elektrický nastavitelný stůl",
                    "DA": "Justerbart skrivebord",
                    "DE": "Elektrisch verstellbarer Schreibtisch",
                    "ES": "Escritorio ajustable eléctrico",
                    "FI": "Sähköisesti säädettävä työpöytä",
                    "FR": "Bureau électrique réglable",
                    "HU": "Elektromosan állítható asztal",
                    "IT": "Scrivania regolabile elettricamente",
                    "JP": "電動昇降デスク",
                    "KR": "전자 조절식 책상",
                    "MS": "Electric Adjustable Desk",
                    "NL": "Elektrisch verstelbaar bureau",
                    "NO": "Elektrisk justerbart skrivebord",
                    "PL": "Biurko regulowane elektrycznie",
                    "RU": "Стол с электрической регулировкой",
                    "SV": "Elektriskt justerbart skrivbord",
                    "TH": "โต๊ะปรับระดับด้วยไฟฟ้า",
                    "TR": "Elektrikli Kumandalı Masa",
                    "UK": "Стіл з електронним регулюванням",
                    "RO": "Birou reglabil electric",
                    "SL": "Električno nastavljiva miza"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 26,
                    "type": [
                        "100"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJNZWRpYSBEZXZpY2VzL3ZhY3VtIGNsZWFuZXIiIGNsaXAtcGF0aD0idXJsKCNjbGlwMF8zMTVfMjE0MykiPjxnIGlkPSJHcm91cCA0NDg0Ij48cGF0aCBpZD0iUmVjdGFuZ2xlIDEwMjEzX19fX18wXzBfUUVGS1dOR09XRiIgc3Ryb2tlPSJibGFjayIgZD0iTTQuNzQyNzQgMTkuOTg3NkgyQzEuNDQ3NzIgMTkuOTg3NiAxIDE5LjUzOTggMSAxOC45ODc2VjEyLjQwNjZDMSAxMS44NTQ0IDEuNDQ3NzIgMTEuNDA2NiAyIDExLjQwNjZINC4xOTUwMk05LjcxNzg0IDE5Ljk4NzZIMTQuNThDMTQuNjQyNCAxOS45ODc2IDE0LjY5MjkgMTkuOTM3IDE0LjY5MjkgMTkuODc0NlYxOS4wNzQ3QzE0LjY5MjkgMTQuODM5NyAxMS4yNTk4IDExLjQwNjYgNy4wMjQ5IDExLjQwNjZWMTEuNDA2Nk00LjE5NTAyIDExLjQwNjZWMTAuNDQ4MUg3LjAyNDlWMTEuNDA2Nk00LjE5NTAyIDExLjQwNjZINy4wMjQ5TTEwLjA4MyAxMS44NjMxSDExLjAzMzlDMTEuODE5NSAxMS44NjMxIDEyLjQ1NjQgMTEuMjI2MiAxMi40NTY0IDEwLjQ0MDVWNC42MDU4MUMxMi40NTY0IDIuNjE0MzggMTQuMDcwOCAxIDE2LjA2MjIgMVYxQzE4LjA1MzcgMSAxOS42NjgxIDIuNjE0MzggMTkuNjY4MSA0LjYwNTgxVjE5LjA3NDdNMTIuNTQ3NyAxMy4zMjM3VjEzLjMyMzdDMTMuMzA0IDEzLjMyMzcgMTMuOTE3IDEyLjcxMDYgMTMuOTE3IDExLjk1NDRWNC42NTE0NUMxMy45MTcgMy40NDE0NyAxNC44OTc5IDIuNDYwNTggMTYuMTA3OSAyLjQ2MDU4VjIuNDYwNThDMTcuMzE3OSAyLjQ2MDU4IDE4LjI5ODggMy40NDE0NyAxOC4yOTg4IDQuNjUxNDVWMTkuMDc0N00xNy40NzcyIDIwLjYyNjZIMTUuMDEyNFYyMi44NjMxSDIzVjIwLjYyNjZIMjAuNDQ0TTE3LjQ3NzIgMjAuNjI2NlYxOS4wNzQ3SDIwLjQ0NFYyMC42MjY2TTE3LjQ3NzIgMjAuNjI2NkgyMC40NDQiLz48Y2lyY2xlIGlkPSJFbGxpcHNlIDEyNDNfX19fXzBfMV9VRUVBQ1hWTVVHIiBjeD0iNy4yOTg2OCIgY3k9IjE5LjE2NjEiIHI9IjIuNDIxMTYiIHN0cm9rZT0iYmxhY2siLz48Y2lyY2xlIGlkPSJFbGxpcHNlIDEyNDRfX19fXzBfMl9CWkpRU1JZT1NWIiBjeD0iNy4yOTg1NyIgY3k9IjE5LjE2NiIgcj0iMS4wNTE4NyIgc3Ryb2tlPSJibGFjayIvPjwvZz48L2c+PC9nPjxkZWZzPjxjbGlwUGF0aCBpZD0iY2xpcDBfMzE1XzIxNDMiPjxwYXRoIGZpbGw9IndoaXRlIiBkPSJNMCAwSDI0VjI0SDB6Ii8+PC9jbGlwUGF0aD48L2RlZnM+PC9zdmc+",
                    "EN": "Vacuum Cleaner",
                    "TW": "吸塵器",
                    "CN": "吸尘器",
                    "BR": "Aspirador de pó",
                    "CZ": "Vysavač",
                    "DA": "Støvsuger",
                    "DE": "Staubsauger",
                    "ES": "Aspiradora",
                    "FI": "Imuri",
                    "FR": "Aspirateur",
                    "HU": "Porszívó",
                    "IT": "Aspirapolvere",
                    "JP": "電気掃除機",
                    "KR": "진공청소기",
                    "MS": "Vacuum Cleaner",
                    "NL": "Stofzuiger",
                    "NO": "Støvsuger",
                    "PL": "Odkurzacz",
                    "RU": "Пылесос",
                    "SV": "Dammsugare",
                    "TH": "เครื่องดูดฝุ่น",
                    "TR": "Elektrikli Süpürge",
                    "UK": "Пилосос",
                    "RO": "Aspirator",
                    "SL": "Sesalec"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 27,
                    "type": [
                        "101"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzA3Ij48ZyBpZD0iR3JvdXAgNDQ3NiI+PGcgaWQ9Ikdyb3VwIDQ0NzUiPjxwYXRoIGlkPSJFbGxpcHNlIDEyMzNfX19fXzBfMF9PWVNVV0RFSFBNIiBzdHJva2U9ImJsYWNrIiBkPSJNMTYuMDM5NiAxOC4zMzA0QzE1LjY5NzEgMTguMzMwMiAxNS4zMTg3IDE4LjMyOTUgMTQuOTE5MiAxOC4zMjgyTDE2LjAzOTYgMTguMzMwNFpNMTYuMDM5NiAxOC4zMzA0QzE1LjgzODggMTguNTQxMiAxNS42OTQ5IDE4Ljc1NiAxNS41OTQzIDE4Ljk2NDVDMTUuNTE2MyAxOS4xMjYzIDE1LjQ2NzIgMTkuMjc4NSAxNS40MzcyIDE5LjQxNDFINi41NzY2OVYyMC40MTQxSDE1LjYxNTlMMTYuMjU0NiAyMS41SDQuNzQ1OThDNC45NzQzNiAyMC45MzM5IDUuMjM2NDMgMjAuMjYwNSA1LjQ2NzI3IDE5LjYyMDZDNS42MzUwNyAxOS4xNTU0IDUuNzg4NDkgMTguNzAyIDUuOTAwNjMgMTguMzE3NUM2LjAwNjM5IDE3Ljk1NDkgNi4wOTUwOSAxNy41OTEgNi4wOTUwOSAxNy4zMzc0QzYuMDk1MDkgMTcuMDg1MiA2LjAwNzAxIDE2LjcxNjggNS45MDE0NSAxNi4zNDY3QzUuNzg5NTcgMTUuOTU0NiA1LjYzNjQgMTUuNDkwMSA1LjQ2ODcyIDE1LjAxMjNDNS4xNDgwNSAxNC4wOTg3IDQuNzY3MDIgMTMuMTE1OCA0LjUgMTIuNDU0N1YxMi40Mzg3SDguNTQ5OTVMMTYuMDM5NiAxOC4zMzA0Wk0xNC45MjAyIDE3LjgyODJDMTEuODM1OSAxNy44MjgyIDkuMzA1MzcgMTUuNDU3MyA5LjA1MTYgMTIuNDM4N00xNC45MjE5IDE3LjMyODJIMTQuOTIwMkMxMi4xMTIzIDE3LjMyODIgOS44MDU4MiAxNS4xODA4IDkuNTUzNTYgMTIuNDM4N0gyMC4yOTY5QzIwLjE3OTcgMTQuNjgxNCAxOS4yNzA1IDE2LjYzMSAxNy42NDk5IDE3LjMyODZMMTcuNTI4NiAxNy4zMjg5QzE3LjM4NyAxNy4zMjkzIDE3LjE4MTYgMTcuMzI5OCAxNi45MjY5IDE3LjMzMDFDMTYuNDE3NSAxNy4zMzA4IDE1LjcxMDggMTcuMzMwOCAxNC45MjE5IDE3LjMyODJaTTE5Ljg2NTggMTEuNDM4N0g5Ljg5ODc3VjExLjA4OUgxOC44NDY2QzE5LjE2MDcgMTEuMDg5IDE5LjU0OTMgMTEuMjI0NyAxOS44NjU4IDExLjQzODdaTTguODk4NzcgMTEuMDg5VjExLjQzODdINC41VjQuMzQwNDlIOC44OTg3N1YxMC41ODlWMTEuMDg5Wk04Ljg5ODc3IDMuMzQwNDlINC41VjIuOTgxNkM0LjUgMi44MjU0MyA0LjUyNzg5IDIuNzM4MzMgNC41NTEzNiAyLjY5MjA2QzQuNTcyNjUgMi42NTAwOCA0LjU5OTk5IDIuNjIxODcgNC42NDA2OCAyLjU5NzQ2QzQuNzQ1MDUgMi41MzQ4NCA0LjkzMDY2IDIuNSA1LjIyNjk5IDIuNUg4LjQxNzE4QzguNzA4OTMgMi41IDguNzkxNTcgMi41NzIzMiA4LjgxNDA4IDIuNTk3MDdDOC44NDUzOCAyLjYzMTUxIDguODk4NzcgMi43MjY5NyA4Ljg5ODc3IDIuOTgxNlYzLjM0MDQ5WiIvPjxwYXRoIGlkPSJVbmlvbl9fX19fMF8xX0NDVkRHVkxTTkIiIGZpbGw9ImJsYWNrIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGQ9Ik03LjkxNzc4IDYuNzg1MTdDNy43NDgwNyA3LjA3ODYyIDcuNDMwODEgNy4yNzYwNyA3LjA2NzQ0IDcuMjc2MDdDNi41MjUzMiA3LjI3NjA3IDYuMDg1ODUgNi44MzY1OSA2LjA4NTg1IDYuMjk0NDdDNi4wODU4NSA1Ljc1MjM1IDYuNTI1MzIgNS4zMTI4OCA3LjA2NzQ0IDUuMzEyODhDNy41MjQ3OSA1LjMxMjg4IDcuOTA5MDkgNS42MjU2NiA4LjAxODA5IDYuMDQ4OTdIMTAuMTM1MUMxMC4zMzg0IDYuMDQ4OTcgMTAuNTAzMiA2LjIxMzc4IDEwLjUwMzIgNi40MTcwN0MxMC41MDMyIDYuNjIwMzcgMTAuMzM4NCA2Ljc4NTE3IDEwLjEzNTEgNi43ODUxN0g3LjkxNzc4WiIgY2xpcC1ydWxlPSJldmVub2RkIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF80X0VBUVFVRFpUS1UiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgZD0iTTE5LjA0MTkgNi44NTY4M0MxOC4xODg4IDYuMDAzNzYgMTcuMDEwMyA1LjQ3NjEyIDE1LjcwODYgNS40NzYxMkMxNC40MDY4IDUuNDc2MTIgMTMuMjI4MyA2LjAwMzc2IDEyLjM3NTIgNi44NTY4M00xNC4wNDE5IDguNTIzNUMxNC40Njg0IDguMDk2OTYgMTUuMDU3NyA3LjgzMzE0IDE1LjcwODYgNy44MzMxNEMxNi4zNTk1IDcuODMzMTQgMTYuOTQ4NyA4LjA5Njk2IDE3LjM3NTIgOC41MjM1Ii8+PC9nPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Toilet",
                    "TW": "洗手間",
                    "CN": "卫生间设备",
                    "BR": "Banheiro",
                    "CZ": "Toaleta",
                    "DA": "Toilet",
                    "DE": "Toilette",
                    "ES": "Aseo",
                    "FI": "WC",
                    "FR": "Toilettes",
                    "HU": "WC",
                    "IT": "Toilet",
                    "JP": "トイレ",
                    "KR": "화장실",
                    "MS": "Toilet",
                    "NL": "Toilet",
                    "NO": "Toalett",
                    "PL": "Toaleta",
                    "RU": "Туалет",
                    "SV": "Toalett",
                    "TH": "ห้องน้ำ",
                    "TR": "Tuvalet",
                    "UK": "Туалет",
                    "RO": "Closet",
                    "SL": "Stranišče"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 28,
                    "type": [
                        "130"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyMiIgaGVpZ2h0PSIyMiIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDIyIDIyIj48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJpYyBFcXVpcG1lbnQvRGlzaHdhc2hlciI+PGcgaWQ9Ikdyb3VwIDQ1ODIiPjxnIGlkPSJSZWN0YW5nbGUgMTAyNjdfX19fXzBfMF9VVFNSQUlZVE1UIj48cGF0aCBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuNSIgZD0iTTEgNi4yODYxNFYxOC43ODU5QzEgMTkuODkwNSAxLjg5NTQzIDIwLjc4NTkgMyAyMC43ODU5SDE1LjQxNDlDMTYuNTE5NCAyMC43ODU5IDE3LjQxNDkgMTkuODkwNSAxNy40MTQ5IDE4Ljc4NTlWNi45Njc3TTEgNi4yODYxNFY0LjAwMDA0QzEgMi44OTU0NyAxLjg5NTQzIDIuMDAwMDQgMyAyLjAwMDA0SDE1LjQxNDlDMTYuNTE5NCAyLjAwMDA0IDE3LjQxNDkgMi44OTU0NyAxNy40MTQ5IDQuMDAwMDRWNS42NzE3OU0xIDYuMjg2MTRIMTYuNzY2OSIvPjxwYXRoIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS41IiBkPSJNMSA2LjI4NjE0VjE4Ljc4NTlDMSAxOS44OTA1IDEuODk1NDMgMjAuNzg1OSAzIDIwLjc4NTlIMTUuNDE0OUMxNi41MTk0IDIwLjc4NTkgMTcuNDE0OSAxOS44OTA1IDE3LjQxNDkgMTguNzg1OVY2Ljk2NzdNMSA2LjI4NjE0VjQuMDAwMDRDMSAyLjg5NTQ3IDEuODk1NDMgMi4wMDAwNCAzIDIuMDAwMDRIMTUuNDE0OUMxNi41MTk0IDIuMDAwMDQgMTcuNDE0OSAyLjg5NTQ3IDE3LjQxNDkgNC4wMDAwNFY1LjY3MTc5TTEgNi4yODYxNEgxNi43NjY5Ii8+PHBhdGggc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIxLjUiIGQ9Ik0xIDYuMjg2MTRWMTguNzg1OUMxIDE5Ljg5MDUgMS44OTU0MyAyMC43ODU5IDMgMjAuNzg1OUgxNS40MTQ5QzE2LjUxOTQgMjAuNzg1OSAxNy40MTQ5IDE5Ljg5MDUgMTcuNDE0OSAxOC43ODU5VjYuOTY3N00xIDYuMjg2MTRWNC4wMDAwNEMxIDIuODk1NDcgMS44OTU0MyAyLjAwMDA0IDMgMi4wMDAwNEgxNS40MTQ5QzE2LjUxOTQgMi4wMDAwNCAxNy40MTQ5IDIuODk1NDcgMTcuNDE0OSA0LjAwMDA0VjUuNjcxNzlNMSA2LjI4NjE0SDE2Ljc2NjkiLz48cGF0aCBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuNSIgZD0iTTEgNi4yODYxNFYxOC43ODU5QzEgMTkuODkwNSAxLjg5NTQzIDIwLjc4NTkgMyAyMC43ODU5SDE1LjQxNDlDMTYuNTE5NCAyMC43ODU5IDE3LjQxNDkgMTkuODkwNSAxNy40MTQ5IDE4Ljc4NTlWNi45Njc3TTEgNi4yODYxNFY0LjAwMDA0QzEgMi44OTU0NyAxLjg5NTQzIDIuMDAwMDQgMyAyLjAwMDA0SDE1LjQxNDlDMTYuNTE5NCAyLjAwMDA0IDE3LjQxNDkgMi44OTU0NyAxNy40MTQ5IDQuMDAwMDRWNS42NzE3OU0xIDYuMjg2MTRIMTYuNzY2OSIvPjwvZz48ZyBpZD0iUmVjdGFuZ2xlIDEwMjY4X19fX18wXzFfU1ZGRExFRUNRTyI+PHJlY3Qgd2lkdGg9IjEyLjQ5NjciIGhlaWdodD0iMTEuMDgzNCIgeD0iMi45NTkyNiIgeT0iNy45NzE1OCIgc3Ryb2tlPSJibGFjayIgcng9IjAuNSIvPjxyZWN0IHdpZHRoPSIxMi40OTY3IiBoZWlnaHQ9IjExLjA4MzQiIHg9IjIuOTU5MjYiIHk9IjcuOTcxNTgiIHN0cm9rZT0iYmxhY2siIHJ4PSIwLjUiLz48cmVjdCB3aWR0aD0iMTIuNDk2NyIgaGVpZ2h0PSIxMS4wODM0IiB4PSIyLjk1OTI2IiB5PSI3Ljk3MTU4IiBzdHJva2U9ImJsYWNrIiByeD0iMC41Ii8+PHJlY3Qgd2lkdGg9IjEyLjQ5NjciIGhlaWdodD0iMTEuMDgzNCIgeD0iMi45NTkyNiIgeT0iNy45NzE1OCIgc3Ryb2tlPSJibGFjayIgcng9IjAuNSIvPjwvZz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzJfTlJRU0hDQklLUiIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgZD0iTTYuODM2MzggMTAuNTgyMkM2LjgzNjM4IDEwLjE1OSA2LjM5MTQzIDkuMzc4NTQgNi4xMzgxNCA4LjkzNDYzQzYuMDg4MTcgOC44NDcwNiA2LjAyNTI3IDguNzQ4MzIgNS45MjQ0NSA4Ljc0ODMyVjguNzQ4MzJDNS44MjM2MiA4Ljc0ODMyIDUuNzYwNzMgOC44NDcwNiA1LjcxMDc2IDguOTM0NjNDNS40NTc0NiA5LjM3ODU0IDUuMDEyNTEgMTAuMTU5IDUuMDEyNTEgMTAuNTgyMkM1LjAxMjUxIDExLjIwNDkgNS40OTI3NSAxMS40ODQxIDUuOTI0NDUgMTEuNDg0MUM2LjM1NjE1IDExLjQ4NDEgNi44MzYzOCAxMS4yMDQ5IDYuODM2MzggMTAuNTgyMloiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzNfWVpVWkJaWVJKUSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgZD0iTTEzLjQwMjEgMTAuNTgyMkMxMy40MDIxIDEwLjE1OSAxMi45NTcyIDkuMzc4NTQgMTIuNzAzOSA4LjkzNDYzQzEyLjY1MzkgOC44NDcwNiAxMi41OTEgOC43NDgzMiAxMi40OTAyIDguNzQ4MzJWOC43NDgzMkMxMi4zODk0IDguNzQ4MzIgMTIuMzI2NSA4Ljg0NzA2IDEyLjI3NjUgOC45MzQ2M0MxMi4wMjMyIDkuMzc4NTQgMTEuNTc4MyAxMC4xNTkgMTEuNTc4MyAxMC41ODIyQzExLjU3ODMgMTEuMjA0OSAxMi4wNTg1IDExLjQ4NDEgMTIuNDkwMiAxMS40ODQxQzEyLjkyMTkgMTEuNDg0MSAxMy40MDIxIDExLjIwNDkgMTMuNDAyMSAxMC41ODIyWiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfNF9KREZVTUZNTE5CIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBkPSJNMTAuMTE5NSAxMC41ODIyQzEwLjExOTUgMTAuMTU5IDkuNjc0NTQgOS4zNzg1NCA5LjQyMTI1IDguOTM0NjNDOS4zNzEyOCA4Ljg0NzA2IDkuMzA4MzggOC43NDgzMiA5LjIwNzU2IDguNzQ4MzJWOC43NDgzMkM5LjEwNjczIDguNzQ4MzIgOS4wNDM4NCA4Ljg0NzA2IDguOTkzODcgOC45MzQ2M0M4Ljc0MDU3IDkuMzc4NTQgOC4yOTU2MiAxMC4xNTkgOC4yOTU2MiAxMC41ODIyQzguMjk1NjIgMTEuMjA0OSA4Ljc3NTg2IDExLjQ4NDEgOS4yMDc1NiAxMS40ODQxQzkuNjM5MjYgMTEuNDg0MSAxMC4xMTk1IDExLjIwNDkgMTAuMTE5NSAxMC41ODIyWiIvPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTMxMF9fX19fMF81X0NZWU5DT0NaQ0oiIGN4PSI2LjY1Mzk3IiBjeT0iMTUuMzE0MyIgcj0iMi45MTgyIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWpvaW49InJvdW5kIi8+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMzExX19fX18wXzZfRFhFSUZJS0xJVyIgY3g9IjYuNjU0IiBjeT0iMTUuMzE0NCIgcj0iMS42NDE0OSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIvPjxwYXRoIGlkPSJSZWN0YW5nbGUgMTAyNjlfX19fXzBfN19TR0VSRERXU0NRIiBzdHJva2U9ImJsYWNrIiBkPSJNMTAuOTIzIDE2LjU3MTVMMTAuODIzIDE0LjE3MjlIMTMuMDYzNUwxMy4wMzk2IDE0Ljc0NjRMMTIuOTYzNiAxNi41NzAzQzEyLjk0MDggMTcuMTE3NyAxMi40OTAxIDE3LjU1MDIgMTEuOTQzMiAxNy41NTAyQzExLjM5NTggMTcuNTUwMiAxMC45NDU4IDE3LjExODUgMTAuOTIzIDE2LjU3MTVaIi8+PHBhdGggaWQ9IlZlY3RvciAyOTE2X19fX18wXzhfTUtRRkhTUldWRyIgc3Ryb2tlPSJibGFjayIgZD0iTTEzLjQwMjEgMTQuNzY3MUgxMy41Mjg3QzE0LjA4NjIgMTQuNzY3MSAxNC41MTMzIDE1LjI2MjkgMTQuNDMwNSAxNS44MTQzVjE1LjgxNDNDMTQuMzYzNiAxNi4yNjA3IDEzLjk4MDEgMTYuNTkxIDEzLjUyODcgMTYuNTkxSDEzLjQwMjEiLz48cGF0aCBpZD0iRWxsaXBzZSAxMzEyX19fX18wXzlfVURXVUNOV0NUSCIgZmlsbD0iYmxhY2siIGQ9Ik0xNS40MDg3IDQuMzcxMDFDMTUuNDA4NyA0Ljg3NDY2IDE1LjAwMDUgNS4yODI5NSAxNC40OTY4IDUuMjgyOTVDMTMuOTkzMiA1LjI4Mjk1IDEzLjU4NDkgNC44NzQ2NiAxMy41ODQ5IDQuMzcxMDFDMTMuNTg0OSAzLjg2NzM2IDEzLjk5MzIgMy40NTkwNyAxNC40OTY4IDMuNDU5MDdDMTUuMDAwNSAzLjQ1OTA3IDE1LjQwODcgMy44NjczNiAxNS40MDg3IDQuMzcxMDFaIi8+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMzEzX19fX18wXzEwX05MUExFQkhLT1oiIGN4PSIzLjE4ODcyIiBjeT0iNC4zNzEwNSIgcj0iMC41NDcxNjIiIGZpbGw9ImJsYWNrIi8+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMzE0X19fX18wXzExX0JSQk5GVkxSQ00iIGN4PSI0LjgzMDI3IiBjeT0iNC4zNzEwNSIgcj0iMC41NDcxNjIiIGZpbGw9ImJsYWNrIi8+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMzE1X19fX18wXzEyX09aQ01WV05RRlEiIGN4PSI2LjQ3MjA2IiBjeT0iNC4zNzEwNSIgcj0iMC41NDcxNjIiIGZpbGw9ImJsYWNrIi8+PHBhdGggaWQ9IkVsbGlwc2UgMTMyMl9fX19fMF8xM19IVlFSRkVBUFdRIiBzdHJva2U9ImJsYWNrIiBkPSJNMTguMjEwOSA2LjMxOTgxQzE4LjIxMDkgNi43NTkzOCAxNy44NTQ1IDcuMTE1NzIgMTcuNDE1IDcuMTE1NzJDMTYuOTc1NCA3LjExNTcyIDE2LjYxOSA2Ljc1OTM4IDE2LjYxOSA2LjMxOTgxQzE2LjYxOSA1Ljg4MDI0IDE2Ljk3NTQgNS41MjM5IDE3LjQxNSA1LjUyMzlDMTcuODU0NSA1LjUyMzkgMTguMjEwOSA1Ljg4MDI0IDE4LjIxMDkgNi4zMTk4MVoiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzE0X05FSlpHQ1RNWEciIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgZD0iTTIwLjI0MDkgOC4yNTczOUMyMC43MDk5IDcuNzg4MzYgMjEgNy4xNDA0MSAyMSA2LjQyNDY5QzIxIDUuNzA4OTggMjAuNzA5OSA1LjA2MTAzIDIwLjI0MDkgNC41OTJNMTkuMzI0NSA1LjUwODM1QzE5LjU1OSA1Ljc0Mjg2IDE5LjcwNDEgNi4wNjY4NCAxOS43MDQxIDYuNDI0NjlDMTkuNzA0MSA2Ljc4MjU1IDE5LjU1OSA3LjEwNjUzIDE5LjMyNDUgNy4zNDEwNCIvPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Dishwasher",
                    "TW": "洗碗機",
                    "CN": "洗碗机",
                    "BR": "Máquina de lavar louça",
                    "CZ": "Myčka na nádobí",
                    "DA": "Opvaskemaskine",
                    "DE": "Spülmaschine",
                    "ES": "Lavavajillas",
                    "FI": "Astianpesukone",
                    "FR": "Lave-vaisselle",
                    "HU": "Mosogatógép",
                    "IT": "Lavastoviglie",
                    "JP": "皿洗い機",
                    "KR": "식기세척기",
                    "MS": "Dishwasher",
                    "NL": "Wasmachine",
                    "NO": "Oppvaskmaskin",
                    "PL": "Zmywarka",
                    "RU": "Посудомоечная машина",
                    "SV": "Diskmaskin",
                    "TH": "เครื่องล้างจาน",
                    "TR": "Bulaşık Makinesi",
                    "UK": "Посудомийна машина",
                    "RO": "Mașină de spălat vase",
                    "SL": "Pomivalni stroj"
                },
                {
                    "category": "electronic_equipment",
                    "ui-sort": 29,
                    "type": [
                        "131"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJFbGVjdHJpYyBFcXVpcG1lbnQvVmFjdXVtX0NsZWFuZXIiIGNsaXAtcGF0aD0idXJsKCNjbGlwMF8zMTVfMzg2NSkiPjxwYXRoIGlkPSJWZWN0b3IgMjkxN19fX19fMF8wX01ERlBLUExPRlciIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik00Ljc2NzEyIDEzLjg1MDNMNS4xNzgwOCAxMi4zODM2TTE5LjMwMTQgMTUuMTc4MUwxNC43ODA4IDEzLjk0NTJNNS4xNzgwOCAxMi4zODM2VjEyLjM4MzZDNi4zNDY2MSAxMS44MTY5IDcuNjg5NzggMTEuNzI3IDguOTIzMzkgMTIuMTMzTDkuNjg0OTMgMTIuMzgzNk01LjE3ODA4IDEyLjM4MzZIMi43MTIzM0wxLjk3MjYgMTYuNTc1M0g0LjY5ODYzTTUuMTc4MDggMTIuMzgzNkwzLjI4NzY3IDRIME05LjA3OTIgMTcuNzI2QzEwLjU5MTUgMTcuNzkxOCAxNC44ODA1IDE3LjY5ODYgMTYuMjc3OCAxNy43MjZNOS42ODQ5MyAxMi4zODM2QzEwLjAyMDkgMTEuNjg0OSAxMS40NzY3IDEwLjU0MjUgMTMuMDU0OCAxMS4wNjg1QzE0LjYzMjkgMTEuNTk0NSAxNS4wMjc0IDEzLjMyNzUgMTQuNzgwOCAxMy45NDUyTTkuNjg0OTMgMTIuMzgzNkwxNC43ODA4IDEzLjk0NTJNMjEuNDM4NCAxOS4yMDU1SDIzTDI0LjUgMTYuNUMyMyAxNi41IDIyLjUxODcgMTcuMDc2OSAyMi4yNTE2IDE3LjM5NzJNMjEuNDM4NCAxOS4yMDU1QzIxLjQzODQgMTguNTYwOSAyMS44MjkyIDE3LjkwMzkgMjIuMjUxNiAxNy4zOTcyTTIxLjQzODQgMTkuMjA1NUgyMC45NDUySDE4LjU2MTZMMi4yNDE3IDE5LjIwNTVNMjIuMjUxNiAxNy4zOTcyQzIyLjExNzUgMTYuNzEyMyAyMiAxNi41IDIyLjUgMTQuNUMxOS4zMDE0IDE1LjUgMTkuOTE3OCAxOC41NDc5IDIwIDE5LjIwNTVNMTcuMTI3NCAxMC40NTY5QzE3LjAxNzUgOS42Njg1MyAxNi42MDY4IDguOTIyMTMgMTUuOTIxNCA4LjQwNDUyQzE1LjIzNjEgNy44ODY5MSAxNC40MDU4IDcuNjk2MSAxMy42MTc1IDcuODA2MDJNMTMuODMyMyA5LjM0NjJDMTQuMjI2NCA5LjI5MTI0IDE0LjY0MTYgOS4zODY2NSAxNC45ODQyIDkuNjQ1NDVDMTUuMzI2OSA5LjkwNDI2IDE1LjUzMjIgMTAuMjc3NSAxNS41ODcyIDEwLjY3MTZNMTkuNzEyMyAxNy40Nzk1QzE5LjcxMjMgMTguNDMyNyAxOC45Mzk2IDE5LjIwNTUgMTcuOTg2MyAxOS4yMDU1QzE3LjAzMyAxOS4yMDU1IDE2LjI2MDMgMTguNDMyNyAxNi4yNjAzIDE3LjQ3OTVDMTYuMjYwMyAxNi41MjYyIDE3LjAzMyAxNS43NTM0IDE3Ljk4NjMgMTUuNzUzNEMxOC45Mzk2IDE1Ljc1MzQgMTkuNzEyMyAxNi41MjYyIDE5LjcxMjMgMTcuNDc5NVpNOS4yMDU0OCAxNi45ODYzQzkuMjA1NDggMTguMjExOSA4LjIxMTkyIDE5LjIwNTUgNi45ODYzIDE5LjIwNTVDNS43NjA2OCAxOS4yMDU1IDQuNzY3MTIgMTguMjExOSA0Ljc2NzEyIDE2Ljk4NjNDNC43NjcxMiAxNS43NjA3IDUuNzYwNjggMTQuNzY3MSA2Ljk4NjMgMTQuNzY3MUM4LjIxMTkyIDE0Ljc2NzEgOS4yMDU0OCAxNS43NjA3IDkuMjA1NDggMTYuOTg2M1oiLz48L2c+PC9nPjxkZWZzPjxjbGlwUGF0aCBpZD0iY2xpcDBfMzE1XzM4NjUiPjxwYXRoIGZpbGw9IndoaXRlIiBkPSJNMCAwSDI0VjI0SDB6Ii8+PC9jbGlwUGF0aD48L2RlZnM+PC9zdmc+",
                    "EN": "Robotic Lawn Mower",
                    "TW": "割草機器人",
                    "CN": "机器人割草机",
                    "BR": "Cortador de grama robótico",
                    "CZ": "Automatická trávní sekačka",
                    "DA": "Robotplæneklipper",
                    "DE": "Rasenmäherroboter",
                    "ES": "Robot cortacésped",
                    "FI": "Robottiruohonleikkuri",
                    "FR": "Tondeuse à gazon robotisée",
                    "HU": "Robotfűnyíró",
                    "IT": "Tagliaerba robotizzato",
                    "JP": "ロボット式芝刈り機",
                    "KR": "로봇 잔디 깎기",
                    "MS": "Robotic Lawn Mower",
                    "NL": "Robotgrasmaaier",
                    "NO": "Robotgressklipper",
                    "PL": "Automatyczna kosiarka",
                    "RU": "Роботизированная газонокосилка",
                    "SV": "Robotgräsklippare",
                    "TH": "หุ่นยนต์ตัดหญ้า",
                    "TR": "Çim biçme makinesi",
                    "UK": "Робот-газонокосарка",
                    "RO": "Robot de tuns iarba",
                    "SL": "Robotska kosilnica"
                },
                {
                    "category": "security",
                    "ui-sort": 1,
                    "type": [
                        "62"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzA5Ij48cGF0aCBpZD0iUmVjdGFuZ2xlIDEwMjM4X19fX18wXzBfUUlWWUZCSUlTWSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTcuMjUgMTZINEMyLjg5NTQzIDE2IDIgMTUuMTA0NiAyIDE0VjcuNUMyIDYuMzk1NDMgMi44OTU0MyA1LjUgNCA1LjVINy4yNU0xNyAxNkgyMEMyMS4xMDQ2IDE2IDIyIDE1LjEwNDYgMjIgMTRWNy41QzIyIDYuMzk1NDMgMjEuMTA0NiA1LjUgMjAgNS41SDE2Ljc1TTcuMjUgNS41VjRDNy4yNSAzLjQ0NzcyIDcuNjk3NzIgMyA4LjI1IDNIMTUuNzVDMTYuMzAyMyAzIDE2Ljc1IDMuNDQ3NzIgMTYuNzUgNFY1LjVNNy4yNSA1LjVIMTYuNzVNNS4yNSAxMC4yNUgxMk0xOC41IDEwLjI1SDE2Ljc1TTcuMjUgMTAuNzVWMjFIMTJNMTYuNzUgMTAuMjVWMjFIMTJNMTYuNzUgMTAuMjVIMTJNMTIgMTAuMjVMMTAgMTIuMjVMMTMuNzUgMTQuNUwxMCAxNUwxMy4yNSAxNi41TDEwLjc1IDE3LjVMMTIgMTguNVYyMSIvPjwvZz48L2c+PC9zdmc+",
                    "EN": "Seismograph",
                    "TW": "震測器",
                    "CN": "地震仪",
                    "BR": "Sismógrafo",
                    "CZ": "Seizmograf",
                    "DA": "Seismograf",
                    "DE": "Seismograph",
                    "ES": "Sismógrafo",
                    "FI": "Seismografi",
                    "FR": "Sismographe",
                    "HU": "Szeizmográf",
                    "IT": "Sismografo",
                    "JP": "地震計",
                    "KR": "지진계",
                    "MS": "Seismograf",
                    "NL": "Seismograaf",
                    "NO": "Seismograf",
                    "PL": "Sejsmograf",
                    "RU": "Сейсмограф",
                    "SV": "Seismograf",
                    "TH": "ไซสโมกราฟ (Seismograph)",
                    "TR": "Sismograf",
                    "UK": "Сейсмограф",
                    "RO": "Seismograf",
                    "SL": "Seizmograf"
                },
                {
                    "category": "security",
                    "ui-sort": 2,
                    "type": [
                        "63"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzIzIj48ZyBpZD0iR3JvdXAgNDUwMiI+PHBhdGggaWQ9IlJlY3RhbmdsZSAxMDIyOF9fX19fMF8wX1NET1lJWEZJREoiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xNC40NzYyIDEyVjEwLjQyODZNMTQuNDc2MiAxNS40MDQ4VjE3LjU5ODJNNCAxMC40Mjg2VjRDNCAyLjM0MzE1IDUuMzQzMTUgMSA3IDFIMTEuNDc2MkMxMy4xMzMgMSAxNC40NzYyIDIuMzQzMTUgMTQuNDc2MiA0VjEwLjQyODZNNCAxMC40Mjg2SDE0LjQ3NjJNNCAxMC40Mjg2VjE3LjVNNCAxNy41VjIwQzQgMjEuNjU2OSA1LjM0MzE0IDIzIDcgMjNIMTEuNDc2MkMxMy4xMzMgMjMgMTQuNDc2MiAyMS42NTY5IDE0LjQ3NjIgMjBWMTcuNTk4Mk00IDE3LjVMMTQuNDc2MiAxNy41OTgyTTYuNjE5MDUgMjAuMTE5SDEyLjExOU0xMi4zODEgMTJIMTcuNDg4MUMxOC40MjgzIDEyIDE5LjE5MDUgMTIuNzYyMiAxOS4xOTA1IDEzLjcwMjRWMTMuNzAyNEMxOS4xOTA1IDE0LjY0MjYgMTguNDI4MyAxNS40MDQ4IDE3LjQ4ODEgMTUuNDA0OEg4Ljg0NTI0QzcuOTA1MDQgMTUuNDA0OCA3LjE0Mjg2IDE0LjY0MjYgNy4xNDI4NiAxMy43MDI0VjEzLjcwMjRDNy4xNDI4NiAxMi43NjIyIDcuOTA1MDQgMTIgOC44NDUyNCAxMkgxMi4zODFaIi8+PGcgaWQ9Ikdyb3VwIDQ1MDEiPjxyZWN0IGlkPSJSZWN0YW5nbGUgMTAyMjlfX19fXzBfMV9QVFVMTlZFVklTIiB3aWR0aD0iMS4wNDc2MiIgaGVpZ2h0PSIxLjA0NzYyIiB4PSI2LjYxMjY5IiB5PSIzLjYxOTA4IiBmaWxsPSJibGFjayIgcng9IjAuNTIzODA5Ii8+PHJlY3QgaWQ9IlJlY3RhbmdsZSAxMDIzMl9fX19fMF8yX0pCUEtYWFVKSEciIHdpZHRoPSIxLjA0NzYyIiBoZWlnaHQ9IjEuMDQ3NjIiIHg9IjguNzA3NzgiIHk9IjMuNjE5MDUiIGZpbGw9ImJsYWNrIiByeD0iMC41MjM4MDkiLz48cmVjdCBpZD0iUmVjdGFuZ2xlIDEwMjM1X19fX18wXzNfS1lXRUJTVktOSiIgd2lkdGg9IjEuMDQ3NjIiIGhlaWdodD0iMS4wNDc2MiIgeD0iMTAuODAyNyIgeT0iMy42MTkwOCIgZmlsbD0iYmxhY2siIHJ4PSIwLjUyMzgwOSIvPjxyZWN0IGlkPSJSZWN0YW5nbGUgMTAyMzBfX19fXzBfNF9XSkVDWE1EU1pOIiB3aWR0aD0iMS4wNDc2MiIgaGVpZ2h0PSIxLjA0NzYyIiB4PSI2LjYxMjU4IiB5PSI1LjcxNDI4IiBmaWxsPSJibGFjayIgcng9IjAuNTIzODA5Ii8+PHJlY3QgaWQ9IlJlY3RhbmdsZSAxMDIzM19fX19fMF81X1BZUVVVU0tSVVgiIHdpZHRoPSIxLjA0NzYyIiBoZWlnaHQ9IjEuMDQ3NjIiIHg9IjguNzA3NzgiIHk9IjUuNzE0MjgiIGZpbGw9ImJsYWNrIiByeD0iMC41MjM4MDkiLz48cmVjdCBpZD0iUmVjdGFuZ2xlIDEwMjM2X19fX18wXzZfR1ZKQ0taSE5OTiIgd2lkdGg9IjEuMDQ3NjIiIGhlaWdodD0iMS4wNDc2MiIgeD0iMTAuODAyNyIgeT0iNS43MTQyNSIgZmlsbD0iYmxhY2siIHJ4PSIwLjUyMzgwOSIvPjxyZWN0IGlkPSJSZWN0YW5nbGUgMTAyMzFfX19fXzBfN19PQUxXWklGVlpTIiB3aWR0aD0iMS4wNDc2MiIgaGVpZ2h0PSIxLjA0NzYyIiB4PSI2LjYxMjU4IiB5PSI3LjgwOTUyIiBmaWxsPSJibGFjayIgcng9IjAuNTIzODA5Ii8+PHJlY3QgaWQ9IlJlY3RhbmdsZSAxMDIzNF9fX19fMF84X0JPVE9HQ0hURkEiIHdpZHRoPSIxLjA0NzYyIiBoZWlnaHQ9IjEuMDQ3NjIiIHg9IjguNzA3NzgiIHk9IjcuODA5NTIiIGZpbGw9ImJsYWNrIiByeD0iMC41MjM4MDkiLz48cmVjdCBpZD0iUmVjdGFuZ2xlIDEwMjM3X19fX18wXzlfR01LR0hTWU9RVSIgd2lkdGg9IjEuMDQ3NjIiIGhlaWdodD0iMS4wNDc2MiIgeD0iMTAuODAyNyIgeT0iNy44MDk0MyIgZmlsbD0iYmxhY2siIHJ4PSIwLjUyMzgwOSIvPjwvZz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Smart Lock",
                    "TW": "智慧門鎖",
                    "CN": "密码锁",
                    "BR": "Fechadura inteligente",
                    "CZ": "Chytrý zámek",
                    "DA": "Smart-lås",
                    "DE": "Intelligentes Schloss",
                    "ES": "Bloqueo inteligente",
                    "FI": "Älylukko",
                    "FR": "Serrure Samrt",
                    "HU": "Okos-zár",
                    "IT": "Serratura intelligente",
                    "JP": "スマートロック",
                    "KR": "스마트 자물쇠",
                    "MS": "Kunci pintar",
                    "NL": "Slim slot",
                    "NO": "Smartlås",
                    "PL": "Inteligentna blokada",
                    "RU": "Смарт-замок",
                    "SV": "Smart lås",
                    "TH": "สมาร์ทล็อก",
                    "TR": "Akıllı kilit",
                    "UK": "Розумний замок",
                    "RO": "Blocare inteligentă",
                    "SL": "Pametna ključavnica"
                },
                {
                    "category": "security",
                    "ui-sort": 3,
                    "type": [
                        "5"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzI0Ij48ZyBpZD0iR3JvdXAgNDQ5OSI+PHBhdGggaWQ9IlJlY3RhbmdsZSAxMDIyN19fX19fMF8wX0FOV1RQTFhSR0giIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTcuNiA3LjZWMTQuNTE0M0MxNy42IDE3LjgyOCAxNC45MTM3IDIwLjUxNDMgMTEuNiAyMC41MTQzQzguMjg2MjkgMjAuNTE0MyA1LjYgMTcuODI4IDUuNiAxNC41MTQzVjcuNjAwMDFDNS42IDQuMjg2MyA4LjI4NjI5IDEuNiAxMS42IDEuNkMxNC45MTM3IDEuNiAxNy42IDQuMjg2MjkgMTcuNiA3LjZaIi8+PGcgaWQ9Ikdyb3VwIDQ1MDAiPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTI2N19fX19fMF8xX05ISVZOUlFNTUsiIGN4PSIxMS41OTk4IiBjeT0iNy45MTQyOSIgcj0iMy44IiBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuMiIvPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTI2OF9fX19fMF8yX1lYSlpTRVJKSVYiIGN4PSIxMS41OTk5IiBjeT0iNy45MTQyMyIgcj0iMi41MTQyOSIgZmlsbD0iYmxhY2siLz48cGF0aCBpZD0iTGluZSA1Nl9fX19fMF8zX01OV0VBWlNKVlgiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik04Ljc0MjkgMjIuMzk5OSAxNS4wODU4IDIyLjM5OTkiLz48L2c+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "IP Cam",
                    "TW": "IP攝影機",
                    "CN": "网络摄像机",
                    "BR": "Câmera IP",
                    "CZ": "IP kamera",
                    "DA": "IP-cam",
                    "DE": "IP-Kamera",
                    "ES": "Cámara IP",
                    "FI": "IP-kamera",
                    "FR": "Caméra IP",
                    "HU": "IP-kamera",
                    "IT": "IP Cam",
                    "JP": "IPカム",
                    "KR": "IP 카메라",
                    "MS": "IP Cam",
                    "NL": "IP-cam",
                    "NO": "IP-kamera",
                    "PL": "Kamera IP",
                    "RU": "IP-камера",
                    "SV": "IP-kamera",
                    "TH": "กล้องไอพี",
                    "TR": "IP Kamera",
                    "UK": "IP-камера",
                    "RO": "Cameră IP",
                    "SL": "IP kamera"
                },
                {
                    "category": "security",
                    "ui-sort": 4,
                    "type": [
                        "85"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48cGF0aCBzdHJva2U9IiMwMDAiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgZD0iTTkuNDI5IDIyLjY0M0EzLjQyOSAzLjQyOSAwIDAgMSA2IDE5LjIxNFY4LjkzQTMuNDI5IDMuNDI5IDAgMCAxIDkuNDI5IDUuNWg1LjE0MkEzLjQyOSAzLjQyOSAwIDAgMSAxOCA4LjkyOXYxMC4yODVhMy40MjkgMy40MjkgMCAwIDEtMy40MjkgMy40MjlIOS40M1oiLz48cGF0aCBzdHJva2U9IiMwMDAiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIuOCIgZD0iTTE0LjA5IDIwLjI4OEg5LjkxYS40ODIuNDgyIDAgMCAxIDAtLjk2NGguMTZ2LTEuNjA4YTEuOTMxIDEuOTMxIDAgMCAxIDMuODYgMHYxLjYwOGguMTZhLjQ4Mi40ODIgMCAwIDEgMCAuOTYzWm0tMi4wOS45NjZhLjk2NC45NjQgMCAwIDEtLjk2NC0uOTY0aDEuOTI3YS45NjIuOTYyIDAgMCAxLS45NjMuOTY0WiIvPjxjaXJjbGUgY3g9IjEyIiBjeT0iMTAuNjQzIiByPSIzLjQyOSIgc3Ryb2tlPSIjMDAwIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiLz48bWFzayBpZD0iYSIgZmlsbD0iI2ZmZiI+PHBhdGggZD0iTTE0LjU3MiAxMC42NDNBMi41NzIgMi41NzIgMCAwIDAgMTIgOC4wN3YuNzczYTEuNzk5IDEuNzk5IDAgMCAxIDEuOCAxLjc5OWguNzcxWiIvPjwvbWFzaz48cGF0aCBzdHJva2U9IiMwMDAiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIyIiBkPSJNMTQuNTcyIDEwLjY0M0EyLjU3MiAyLjU3MiAwIDAgMCAxMiA4LjA3di43NzNhMS43OTkgMS43OTkgMCAwIDEgMS44IDEuNzk5aC43NzFaIiBtYXNrPSJ1cmwoI2EpIi8+PHBhdGggc3Ryb2tlPSIjMDAwIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIGQ9Ik0yMi43NzMgNi42NzJhNS45ODEgNS45ODEgMCAwIDAtMS43NTgtNC4yNDJBNS45ODEgNS45ODEgMCAwIDAgMTYuNzczLjY3Mm0wIDNhMi45OSAyLjk5IDAgMCAxIDIuMTIxLjg3OSAyLjk5IDIuOTkgMCAwIDEgLjg3OSAyLjEyMSIvPjwvc3ZnPg==",
                    "EN": "Smart doorbell",
                    "TW": "智慧門鈴",
                    "CN": "智能门铃",
                    "BR": "Campainha inteligente",
                    "CZ": "Chytrý zvonek",
                    "DA": "Smart-dørklokke",
                    "DE": "Intelligente Türklingel",
                    "ES": "Timbre inteligente",
                    "FI": "Älyovikello",
                    "FR": "Sonnette intelligente",
                    "HU": "Intelligens csengő",
                    "IT": "Campanello d’ingresso intelligente",
                    "JP": "スマートドアベル",
                    "KR": "스마트 초인종",
                    "MS": "Smart doorbell",
                    "NL": "Slimme deurbel",
                    "NO": "Smart ringeklokke",
                    "PL": "Inteligentny dzwonek do drzwi",
                    "RU": "Дверной смарт-звонок",
                    "SV": "Smart ringklocka",
                    "TH": "กริ่งประตูอัจฉริยะ",
                    "TR": "Akıllı kapı zili",
                    "UK": "Розумний дверний дзвінок",
                    "RO": "Sonerie inteligentă",
                    "SL": "Pametni zvonec"
                },
                {
                    "category": "security",
                    "ui-sort": 5,
                    "type": [
                        "102"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJIdWdlLWljb24vc21hcnQgaG91c2Uvb3V0bGluZS9jYW0iPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMF9LVFVRRVhQWFlNIiBzdHJva2U9IiMyODMwM0YiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjUiIGQ9Ik04IDE3VjE2QzggMTMuNzkwOSA5Ljc5MDg2IDEyIDEyIDEyQzE0LjIwOTEgMTIgMTYgMTMuNzkwOSAxNiAxNlYxN00yMSA5QzIxIDEzLjk3MDYgMTYuOTcwNiAxOCAxMiAxOEM3LjAyOTQ0IDE4IDMgMTMuOTcwNiAzIDlNMy41IDlIMjAuNUMyMS4zMjg0IDkgMjIgOC4zMjg0MyAyMiA3LjVDMjIgNi42NzE1NyAyMS4zMjg0IDYgMjAuNSA2SDMuNUMyLjY3MTU3IDYgMiA2LjY3MTU3IDIgNy41QzIgOC4zMjg0MyAyLjY3MTU3IDkgMy41IDlaIi8+PGNpcmNsZSBpZD0iRWxsaXBzZSA1MzdfX19fXzBfMV9NR0lVSkNWV0FEIiBjeD0iMSIgY3k9IjEiIHI9IjEiIGZpbGw9IiMyODMwM0YiIHRyYW5zZm9ybT0ibWF0cml4KDEgMCAwIC0xIDExIDE2KSIvPjwvZz48L2c+PC9zdmc+",
                    "EN": "Fire Detector",
                    "TW": "火災偵測器",
                    "CN": "火灾探测器",
                    "BR": "Detector de incêndio",
                    "CZ": "Hlásič požáru",
                    "DA": "Brandalarm",
                    "DE": "Rauchmelder",
                    "ES": "Detector de humos",
                    "FI": "Palonilmaisin",
                    "FR": "Détecteur d'incendie",
                    "HU": "Tűzérzékelő",
                    "IT": "Rivelatore di incendi",
                    "JP": "火災検知器",
                    "KR": "화재 탐지기",
                    "MS": "Fire Detector",
                    "NL": "Branddetector",
                    "NO": "Brannvarsler",
                    "PL": "Czujka pożarowa",
                    "RU": "Пожарный датчик",
                    "SV": "Branddetektor",
                    "TH": "อุปกรณ์ตรวจจับเพลิงไหม้",
                    "TR": "Yangın Dedektörü",
                    "UK": "Датчик полум’я",
                    "RO": "Detector de incendiu",
                    "SL": "Detektor požara"
                },
                {
                    "category": "security",
                    "ui-sort": 6,
                    "type": [
                        "103"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSI2NTkzNzI0X2Zhcm1fZmVuY2VfZ2FyZGVuX2ljb24gMSI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8wX1FISllWQ0RaWU0iIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLW1pdGVybGltaXQ9IjEwIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTYuNzUgMjFIMi4yNVY0LjVMNC41IDNMNi43NSA0LjVWMjFaIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8xX1BZWVpESUZYQUsiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLW1pdGVybGltaXQ9IjEwIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE0LjI1IDIxSDkuNzVWNC41TDEyIDNMMTQuMjUgNC41VjIxWiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMl9YVVVSUE9OUkVKIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS1taXRlcmxpbWl0PSIxMCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0yMS43NSAyMUgxNy4yNVY0LjVMMTkuNSAzTDIxLjc1IDQuNVYyMVoiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzNfTUJHUkdYSUVSUCIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2UtbWl0ZXJsaW1pdD0iMTAiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTcuMjUgNkgxNC4yNVY5SDE3LjI1VjZaIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF80X1dBUU1OTk1BRUMiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLW1pdGVybGltaXQ9IjEwIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTkuNzUgNkg2Ljc1VjlIOS43NVY2WiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfNV9NQ1pPSVBLVkVSIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS1taXRlcmxpbWl0PSIxMCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xNy4yNSAxNUgxNC4yNVYxOEgxNy4yNVYxNVoiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzZfSkRaV01CQlNZTCIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2UtbWl0ZXJsaW1pdD0iMTAiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNOS43NSAxNUg2Ljc1VjE4SDkuNzVWMTVaIi8+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Hedge",
                    "TW": "圍籬",
                    "CN": "篱笆",
                    "BR": "Proteção",
                    "CZ": "Bezpečnostní oplocení",
                    "DA": "Hegn",
                    "DE": "Sicherheitszaun",
                    "ES": "Seto",
                    "FI": "Turva-aita",
                    "FR": "Haie",
                    "HU": "Biztonsági kerítés",
                    "IT": "Hedge",
                    "JP": "ヘッジ",
                    "KR": "헷지",
                    "MS": "Hedge",
                    "NL": "Heg",
                    "NO": "Sikkerhetsgjerde",
                    "PL": "Żywopłot",
                    "RU": "Изгородь",
                    "SV": "Inhägnad",
                    "TH": "รั้วนิรภัย",
                    "TR": "Çit",
                    "UK": "Живопліт",
                    "RO": "Barieră",
                    "SL": "Živa meja"
                },
                {
                    "category": "security",
                    "ui-sort": 7,
                    "type": [
                        "104"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzI1Ij48ZyBpZD0iR3JvdXAgNDQ2MCI+PGcgaWQ9Ikdyb3VwIDQ0NTkiPjxwYXRoIGlkPSJFbGxpcHNlIDEyMDlfX19fXzBfMF9JREFXS0NZU1JXIiBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE5LjcxODYgOC41Mjc0N0MxOS43MTg2IDExLjcwMTggMTYuNTc4NCAxNC40NTQ5IDEyLjQ2MTQgMTQuNDU0OUM4LjM0NDUgMTQuNDU0OSA1LjIwNDMxIDExLjcwMTggNS4yMDQzMSA4LjUyNzQ3QzUuMjA0MzEgNS4zNTMxNSA4LjM0NDUgMi42IDEyLjQ2MTQgMi42QzE2LjU3ODQgMi42IDE5LjcxODYgNS4zNTMxNSAxOS43MTg2IDguNTI3NDdaIi8+PHBhdGggaWQ9IkVsbGlwc2UgMTIxNV9fX19fMF8xX1pIUkNFRU9ITFUiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNOC4wOTE0NCAxMy43ODI2QzcuMTk5NDggMTQuNTkwMyA2LjY1OTA0IDE1LjY0NzUgNi42NTkwNCAxNi44MDQ2QzYuNjU5MDQgMTkuMzQxNSA5LjI1Njc3IDIxLjM5OCAxMi40NjEyIDIxLjM5OEMxNS42NjU3IDIxLjM5OCAxOC4yNjM0IDE5LjM0MTUgMTguMjYzNCAxNi44MDQ2QzE4LjI2MzQgMTUuNjQ3NSAxNy43MjMgMTQuNTkwMyAxNi44MzEgMTMuNzgyNiIvPjxwYXRoIGlkPSJFbGxpcHNlIDEyMTFfX19fXzBfMl9CWEFRTkhGU0NEIiBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE2LjgxNzUgOC43MDg3OEMxNi44MTc1IDEwLjY2OTIgMTQuOTUxMyAxMi40IDEyLjQ2MTQgMTIuNEM5Ljk3MTYyIDEyLjQgOC4xMDUzOSAxMC42NjkyIDguMTA1MzkgOC43MDg3OEM4LjEwNTM5IDYuNzQ4NCA5Ljk3MTYyIDUuMDE3NTcgMTIuNDYxNCA1LjAxNzU3QzE0Ljk1MTMgNS4wMTc1NyAxNi44MTc1IDYuNzQ4NCAxNi44MTc1IDguNzA4NzhaIi8+PHBhdGggaWQ9IkVsbGlwc2UgMTIxM19fX19fMF8zX0lSU1laUUhNUlciIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNOC41MzIxOCAyLjk2NzAzQzguMDY3MzIgMi4zNzgwNSA3LjM0NzA0IDIgNi41Mzg0NiAyQzUuMTM2NTEgMiA0IDMuMTM2NTEgNCA0LjUzODQ2QzQgNS4zNzQzNyA0LjQwNDA0IDYuMzIwMDggNS4wMjc0NyA2Ljc4MjYxIi8+PHBhdGggaWQ9IkVsbGlwc2UgMTIxNF9fX19fMF80X1ZIQVNJWUhYQVkiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTYuMzI5OCAyLjk2NzAzQzE2Ljc5NDcgMi4zNzgwNSAxNy41MTQ5IDIgMTguMzIzNSAyQzE5LjcyNTUgMiAyMC44NjIgMy4xMzY1MSAyMC44NjIgNC41Mzg0NkMyMC44NjIgNS4zNzQzNyAyMC40NTc5IDYuMTE1OTEgMTkuODM0NSA2LjU3ODQ0Ii8+PHBhdGggaWQ9IkVsbGlwc2UgMTIxNl9fX19fMF81X01ZQ1NBVEtHQU8iIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNNi43MTkzOCAxNi40ODI2QzYuNjc2ODUgMTYuNDkxMSA2LjQxNzI5IDE2LjQxNzkgNi4xNzU1MyAxNi40ODI3QzQuOTQ0MDcgMTYuODEyNiA0LjY3NjQ1IDE3LjkxOTggNS4wNjQwNCAxOS4zNjYzQzUuNDUxNjMgMjAuODEyOCA2Ljk5OTAyIDIxLjY3MDYgOC4yMzA0OCAyMS4zNDA3QzguNjY3NTYgMjEuMjIzNSA5LjAxNjIgMjAuODU3MSA5LjA3NjUyIDIwLjY2NjQiLz48cGF0aCBpZD0iRWxsaXBzZSAxMjE3X19fX18wXzZfVk1ZRExIQkRJSiIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xOC4xOTQ0IDE2LjQ2NjlDMTguMjM3IDE2LjQ3NTMgMTguNDk2NSAxNi40MDIyIDE4LjczODMgMTYuNDY3QzE5Ljk2OTggMTYuNzk2OSAyMC4yMzc0IDE3LjkwNCAxOS44NDk4IDE5LjM1MDZDMTkuNDYyMiAyMC43OTcxIDE3LjkxNDggMjEuNjU0OSAxNi42ODMzIDIxLjMyNDlDMTYuMjQ2MyAyMS4yMDc4IDE1Ljg5NzYgMjAuODQxNCAxNS44MzczIDIwLjY1MDciLz48L2c+PGcgaWQ9IlN1YnRyYWN0X19fX18wXzdfVVBJVk9VUkJCVCI+PG1hc2sgaWQ9InBhdGgtOC1pbnNpZGUtMV8zMTVfMjQxOSIgZmlsbD0id2hpdGUiPjxwYXRoIGZpbGwtcnVsZT0iZXZlbm9kZCIgZD0iTTEyLjQ2MDcgMTAuODY5N0MxMy42NjEzIDEwLjg2OTcgMTQuNjM0NiA5Ljg5NjQ1IDE0LjYzNDYgOC42OTU4M0MxNC42MzQ2IDcuNDk1MjEgMTMuNjYxMyA2LjUyMTkyIDEyLjQ2MDcgNi41MjE5MkMxMS4yNiA2LjUyMTkyIDEwLjI4NjcgNy40OTUyMSAxMC4yODY3IDguNjk1ODNDMTAuMjg2NyA5Ljg5NjQ1IDExLjI2IDEwLjg2OTcgMTIuNDYwNyAxMC44Njk3Wk0xMi40OTA5IDkuNDE2NjFDMTIuODU4MSA5LjQxNjYxIDEzLjE1NTggOS4xMTg5NiAxMy4xNTU4IDguNzUxNzhDMTMuMTU1OCA4LjM4NDYgMTIuODU4MSA4LjA4Njk0IDEyLjQ5MDkgOC4wODY5NEMxMi4xMjM4IDguMDg2OTQgMTEuODI2MSA4LjM4NDYgMTEuODI2MSA4Ljc1MTc4QzExLjgyNjEgOS4xMTg5NiAxMi4xMjM4IDkuNDE2NjEgMTIuNDkwOSA5LjQxNjYxWiIgY2xpcC1ydWxlPSJldmVub2RkIi8+PC9tYXNrPjxwYXRoIGZpbGw9ImJsYWNrIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGQ9Ik0xMi40NjA3IDEwLjg2OTdDMTMuNjYxMyAxMC44Njk3IDE0LjYzNDYgOS44OTY0NSAxNC42MzQ2IDguNjk1ODNDMTQuNjM0NiA3LjQ5NTIxIDEzLjY2MTMgNi41MjE5MiAxMi40NjA3IDYuNTIxOTJDMTEuMjYgNi41MjE5MiAxMC4yODY3IDcuNDk1MjEgMTAuMjg2NyA4LjY5NTgzQzEwLjI4NjcgOS44OTY0NSAxMS4yNiAxMC44Njk3IDEyLjQ2MDcgMTAuODY5N1pNMTIuNDkwOSA5LjQxNjYxQzEyLjg1ODEgOS40MTY2MSAxMy4xNTU4IDkuMTE4OTYgMTMuMTU1OCA4Ljc1MTc4QzEzLjE1NTggOC4zODQ2IDEyLjg1ODEgOC4wODY5NCAxMi40OTA5IDguMDg2OTRDMTIuMTIzOCA4LjA4Njk0IDExLjgyNjEgOC4zODQ2IDExLjgyNjEgOC43NTE3OEMxMS44MjYxIDkuMTE4OTYgMTIuMTIzOCA5LjQxNjYxIDEyLjQ5MDkgOS40MTY2MVoiIGNsaXAtcnVsZT0iZXZlbm9kZCIvPjxwYXRoIGZpbGw9ImJsYWNrIiBkPSJNMTMuNDM0NiA4LjY5NTgzQzEzLjQzNDYgOS4yMzM3MSAxMi45OTg1IDkuNjY5NzUgMTIuNDYwNyA5LjY2OTc1VjEyLjA2OTdDMTQuMzI0IDEyLjA2OTcgMTUuODM0NiAxMC41NTkyIDE1LjgzNDYgOC42OTU4M0gxMy40MzQ2Wk0xMi40NjA3IDcuNzIxOTJDMTIuOTk4NSA3LjcyMTkyIDEzLjQzNDYgOC4xNTc5NiAxMy40MzQ2IDguNjk1ODNIMTUuODM0NkMxNS44MzQ2IDYuODMyNDcgMTQuMzI0IDUuMzIxOTIgMTIuNDYwNyA1LjMyMTkyVjcuNzIxOTJaTTExLjQ4NjcgOC42OTU4M0MxMS40ODY3IDguMTU3OTYgMTEuOTIyOCA3LjcyMTkyIDEyLjQ2MDcgNy43MjE5MlY1LjMyMTkyQzEwLjU5NzMgNS4zMjE5MiA5LjA4Njc0IDYuODMyNDcgOS4wODY3NCA4LjY5NTgzSDExLjQ4NjdaTTEyLjQ2MDcgOS42Njk3NUMxMS45MjI4IDkuNjY5NzUgMTEuNDg2NyA5LjIzMzcxIDExLjQ4NjcgOC42OTU4M0g5LjA4Njc0QzkuMDg2NzQgMTAuNTU5MiAxMC41OTczIDEyLjA2OTcgMTIuNDYwNyAxMi4wNjk3VjkuNjY5NzVaTTExLjk1NTggOC43NTE3OEMxMS45NTU4IDguNDU2MjEgMTIuMTk1NCA4LjIxNjYxIDEyLjQ5MDkgOC4yMTY2MVYxMC42MTY2QzEzLjUyMDkgMTAuNjE2NiAxNC4zNTU4IDkuNzgxNyAxNC4zNTU4IDguNzUxNzhIMTEuOTU1OFpNMTIuNDkwOSA5LjI4Njk0QzEyLjE5NTQgOS4yODY5NCAxMS45NTU4IDkuMDQ3MzQgMTEuOTU1OCA4Ljc1MTc4SDE0LjM1NThDMTQuMzU1OCA3LjcyMTg2IDEzLjUyMDkgNi44ODY5NCAxMi40OTA5IDYuODg2OTRWOS4yODY5NFpNMTMuMDI2MSA4Ljc1MTc4QzEzLjAyNjEgOS4wNDczNCAxMi43ODY1IDkuMjg2OTQgMTIuNDkwOSA5LjI4Njk0VjYuODg2OTRDMTEuNDYxIDYuODg2OTQgMTAuNjI2MSA3LjcyMTg2IDEwLjYyNjEgOC43NTE3OEgxMy4wMjYxWk0xMi40OTA5IDguMjE2NjFDMTIuNzg2NSA4LjIxNjYxIDEzLjAyNjEgOC40NTYyMSAxMy4wMjYxIDguNzUxNzhIMTAuNjI2MUMxMC42MjYxIDkuNzgxNyAxMS40NjEgMTAuNjE2NiAxMi40OTA5IDEwLjYxNjZWOC4yMTY2MVoiIG1hc2s9InVybCgjcGF0aC04LWluc2lkZS0xXzMxNV8yNDE5KSIvPjwvZz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Toy Bear",
                    "TW": "玩具熊",
                    "CN": "玩具熊",
                    "BR": "Urso de brinquedo",
                    "CZ": "Plyšový medvídek",
                    "DA": "Legetøjsbjørn",
                    "DE": "Spielzeugteddy",
                    "ES": "Osito de peluche",
                    "FI": "Lelukarhu",
                    "FR": "Ours en peluche",
                    "HU": "Játékmackó",
                    "IT": "Orso giocattolo",
                    "JP": "おもちゃの熊",
                    "KR": "곰인형",
                    "MS": "Toy Bear",
                    "NL": "Speelgoedbeer",
                    "NO": "Teddybjørn",
                    "PL": "Miś-zabawka",
                    "RU": "Игрушечный медведь",
                    "SV": "Leksaksbjörn",
                    "TH": "ตุ๊กตาหมี",
                    "TR": "Oyuncak Ayı",
                    "UK": "Іграшковий ведмідь",
                    "RO": "Urs de jucărie",
                    "SL": "Plišasti medvedek"
                },
                {
                    "category": "warables",
                    "ui-sort": 1,
                    "type": [
                        "64"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzI2Ij48ZyBpZD0iR3JvdXAgNDQ2MSI+PHJlY3QgaWQ9IlJlY3RhbmdsZSAxMDE5Ml9fX19fMF8wX09JVExCTVZBRk4iIHdpZHRoPSIxNC41MDczIiBoZWlnaHQ9IjIxLjgiIHg9IjQuNiIgeT0iMS42IiBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuMiIgcng9IjcuMjUzNjYiLz48cGF0aCBpZD0iUmVjdGFuZ2xlIDEwMTk0X19fX18wXzFfUElQRlhTUEhDRyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xOSA3LjVMMTguNTMwNCA2LjcxMzAzQzE3Ljg5NjUgNS42NTA2NiAxNi43NTA1IDUgMTUuNTEzNCA1VjVDMTMuNTczIDUgMTIgNi41NzI5OSAxMiA4LjUxMzM2VjE2LjI3OTJDMTIgMTguMzM0MSAxMy42OTM0IDIwIDE1Ljc0ODQgMjBWMjBDMTYuODUyNiAyMCAxNy45MDk5IDE5LjQ5ODMgMTguNiAxOC42MzY0VjE4LjYzNjQiLz48cmVjdCBpZD0iUmVjdGFuZ2xlIDEwMTkzX19fX18wXzJfVFJSVVpXRERKTSIgd2lkdGg9IjcuMjE0NjMiIGhlaWdodD0iMTEuMTQxNSIgeD0iNC42IiB5PSI2LjY0ODc4IiBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuMiIgcng9IjIuNCIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfM19XS0RFVUhGSURRIiBmaWxsPSJibGFjayIgZD0iTTggMTRMNy43MSAxMy43MTIzQzYuNjggMTIuNjk0MyA2IDEyLjAyMjkgNiAxMS4xOTg5QzYgMTAuNTI3NSA2LjQ4NCAxMCA3LjEgMTBDNy40NDggMTAgNy43ODIgMTAuMTc2NiA4IDEwLjQ1NTZDOC4yMTggMTAuMTc2NiA4LjU1MiAxMCA4LjkgMTBDOS41MTYgMTAgMTAgMTAuNTI3NSAxMCAxMS4xOTg5QzEwIDEyLjAyMjkgOS4zMiAxMi42OTQzIDguMjkgMTMuNzE0NEw4IDE0WiIvPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Smart Bracelet",
                    "TW": "智能手環",
                    "CN": "智能手环",
                    "BR": "Pulseira inteligente",
                    "CZ": "Chytrý náramek",
                    "DA": "Smart-armbånd",
                    "DE": "Intelligentes Armband",
                    "ES": "Pulsera inteligente",
                    "FI": "Älyranneke",
                    "FR": "Bracelet intelligent",
                    "HU": "Intelligens karkötő",
                    "IT": "Bracciale intelligente",
                    "JP": "スマートブレスレット",
                    "KR": "스마트 팔찌",
                    "MS": "Gelang pintar",
                    "NL": "Slimme armband",
                    "NO": "Smartarmbånd",
                    "PL": "Inteligentna bransoletka",
                    "RU": "Смарт-браслет",
                    "SV": "Smart armband",
                    "TH": "สมาร์ทวอทช์",
                    "TR": "Akıllı bileklik",
                    "UK": "Розумний браслет",
                    "RO": "Brățări inteligente",
                    "SL": "Pametna zapestnica"
                },
                {
                    "category": "warables",
                    "ui-sort": 2,
                    "type": [
                        "65"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJSb3VuZCBTbWFydCBXYXRjaCI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8xX0JQUkFDWVVMWkgiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xNi4yNCAxNi4zNkMxNS45NiAxNi42NiAxNS42NCAxNi45MSAxNS4zMSAxNy4xNEMxNC4zNiAxNy44IDEzLjIyIDE4LjE3IDEyIDE4LjE3QzEwLjc4IDE4LjE3IDkuNjQgMTcuOCA4LjY5IDE3LjE0QzcuMDcgMTYuMDQgNiAxNC4xNSA2IDEyQzYgMTAuMjkgNi42NzAwMSA4Ljc1IDcuNzYwMDEgNy42NEM4LjA0MDAxIDcuMzQgOC4zNiA3LjA4IDguNjkgNi44NkM5LjY0IDYuMiAxMC43OCA1LjgzIDEyIDUuODNDMTMuMjIgNS44MyAxNC4zNiA2LjIgMTUuMzEgNi44NkMxNS42NCA3LjA5IDE1Ljk2IDcuMzQgMTYuMjQgNy42NEMxNy4zMyA4Ljc1IDE4IDEwLjI5IDE4IDEyQzE4IDEzLjcxIDE3LjMzIDE1LjI1IDE2LjI0IDE2LjM2WiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMl9IWFFIU0pUVUZFIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTYgNi44NkgxNS4zMUMxNC4zNiA2LjIgMTMuMjIgNS44MyAxMiA1LjgzQzEwLjc4IDUuODMgOS42NCA2LjIgOC42OSA2Ljg2SDhWMy45M0M4IDMuMjggOC41MTk5OSAyLjc1IDkuMTQ5OTkgMi43NUgxNC44NUMxNS40OCAyLjc1IDE2IDMuMjggMTYgMy45M1Y2Ljg2WiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfM19GQlpVRk9GV0xEIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTYgMjAuMDdDMTYgMjAuNzIgMTUuNDggMjEuMjUgMTQuODUgMjEuMjVIOS4xNDk5OUM4LjUxOTk5IDIxLjI1IDggMjAuNzIgOCAyMC4wN1YxNy4xNEg4LjY5QzkuNjQgMTcuOCAxMC43OCAxOC4xNyAxMiAxOC4xN0MxMy4yMiAxOC4xNyAxNC4zNiAxNy44IDE1LjMxIDE3LjE0SDE2VjIwLjA3WiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfNF9RV01OQ0ZMUkRWIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTkgMTAuOTdWMTMuMDMiLz48ZyBpZD0iR3JvdXBfMiI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF81X1hJQlVVSVBFWFciIGZpbGw9ImJsYWNrIiBkPSJNMTIgMTQuNTAwMUwxMS43MSAxNC4yMTI0QzEwLjY4IDEzLjE5NDQgMTAgMTIuNTIzIDEwIDExLjY5OUMxMCAxMS4wMjc2IDEwLjQ4NCAxMC41MDAxIDExLjEgMTAuNTAwMUMxMS40NDggMTAuNTAwMSAxMS43ODIgMTAuNjc2NyAxMiAxMC45NTU3QzEyLjIxOCAxMC42NzY3IDEyLjU1MiAxMC41MDAxIDEyLjkgMTAuNTAwMUMxMy41MTYgMTAuNTAwMSAxNCAxMS4wMjc2IDE0IDExLjY5OUMxNCAxMi41MjMgMTMuMzIgMTMuMTk0NCAxMi4yOSAxNC4yMTQ1TDEyIDE0LjUwMDFaIi8+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "Watch",
                    "TW": "手錶",
                    "CN": "手表",
                    "BR": "Relógio",
                    "CZ": "Hodinky",
                    "DA": "Ur",
                    "DE": "Uhr",
                    "ES": "Reloj",
                    "FI": "Rannekello",
                    "FR": "Montre",
                    "HU": "Karóra",
                    "IT": "Orologio",
                    "JP": "ウォッチ",
                    "KR": "시계",
                    "MS": "Jam",
                    "NL": "Horloge",
                    "NO": "Klokke",
                    "PL": "Zegarek",
                    "RU": "Часы",
                    "SV": "Klocka",
                    "TH": "นาฬิกา",
                    "TR": "Saat",
                    "UK": "Годинник",
                    "RO": "Ceas",
                    "SL": "Ura"
                },
                {
                    "category": "warables",
                    "ui-sort": 3,
                    "type": [
                        "105"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzI3IiBjbGlwLXBhdGg9InVybCgjY2xpcDBfMzE1XzI3ODUpIj48cGF0aCBpZD0iUmVjdGFuZ2xlIDEwMjI1X19fX18wXzBfQUNIUktLVVVWWiIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTExLjEzMTYgMTMuNjg0MkwxMC40NTMzIDEyLjI4NTNDMTAuMTE5IDExLjU5NTggOS40MjAwNSAxMS4xNTc5IDguNjUzNzEgMTEuMTU3OUgxLjM0MjFWMTQuMjEwNUMxLjM0MjEgMTUuMzE1MSAyLjIzNzUzIDE2LjIxMDUgMy4zNDIxIDE2LjIxMDVINy4wOTkxQzguODE2MjUgMTYuMjEwNSAxMC4zODI0IDE1LjIyOTMgMTEuMTMxNiAxMy42ODQyVjEzLjY4NDJaTTExLjEzMTYgMTMuNjg0MkwxMi4zOTQ3IDEzLjY4NDJNMTIuMzk0NyAxMy42ODQyTDEzLjUwMzggMTUuMzIwNEMxMy43Njc4IDE1Ljg2NDggMTQuMzE5NiAxNi4yMTA1IDE0LjkyNDYgMTYuMjEwNUgxNy40MjgyTTEyLjM5NDcgMTMuNjg0MkwxMy43NjI4IDExLjUxMzlDMTMuODY4NCAxMS4yOTYyIDE0LjA4OTEgMTEuMTU3OSAxNC4zMzExIDExLjE1NzlMMjIuNSAxMS4xNTc5VjE0LjQ3MzdNMC41NTI2MzEgMTEuMTU3OUwxLjY1Nzg5IDExLjE1NzlMNS4yODk0NyA4SDYuMzk0NzRNMTcuNjA1MyA4LjE1Nzg5SDE4LjcxMDVMMjIuMTg0MiAxMS4xNTc5SDIzLjQ0NzRNMTkuNjY0NSAxNS43MzY4SDIwLjI4OTVNMjEuNjM0OSAxNS43MzY4SDIxLjUzOTVNNC41IDE0TDUuNzYzMTYgMTIuNzM2OE01Ljc2MzE2IDE0Ljc4OTVMNy42NTc4OSAxMi43MzY4TTE3LjYwNTMgMTUuNDczN1YxNkMxNy42MDUzIDE2LjU1MjMgMTguMDUzIDE3IDE4LjYwNTMgMTdIMjIuMTMxNkMyMi42ODM5IDE3IDIzLjEzMTYgMTYuNTUyMyAyMy4xMzE2IDE2VjE1LjQ3MzdDMjMuMTMxNiAxNC45MjE0IDIyLjY4MzkgMTQuNDczNyAyMi4xMzE2IDE0LjQ3MzdIMTguNjA1M0MxOC4wNTMgMTQuNDczNyAxNy42MDUzIDE0LjkyMTQgMTcuNjA1MyAxNS40NzM3WiIvPjwvZz48L2c+PGRlZnM+PGNsaXBQYXRoIGlkPSJjbGlwMF8zMTVfMjc4NSI+PHBhdGggZmlsbD0id2hpdGUiIGQ9Ik0wIDBIMjRWMjRIMHoiLz48L2NsaXBQYXRoPjwvZGVmcz48L3N2Zz4=",
                    "EN": "AR Glasses",
                    "TW": "AR 眼鏡",
                    "CN": "AR 眼镜",
                    "BR": "Óculos de realidade aumentada",
                    "CZ": "Brýle AR",
                    "DA": "AR-briller",
                    "DE": "AR-Brille",
                    "ES": "Gafas de RA",
                    "FI": "Lisätyn todellisuuden lasit",
                    "FR": "Lunettes RA",
                    "HU": "Kiterjesztett valóság szemüveg",
                    "IT": "Occhiali AR",
                    "JP": "AR グラス",
                    "KR": "AR 안경",
                    "MS": "AR Glasses",
                    "NL": "AR-bril",
                    "NO": "AR-briller",
                    "PL": "Okulary AR",
                    "RU": "Очки дополненной реальности",
                    "SV": "AR-glasögon",
                    "TH": "แว่นตา AR",
                    "TR": "AR Gözlükleri",
                    "UK": "Окуляри доповненої реальності",
                    "RO": "Ochelari AR",
                    "SL": "Očala AR"
                },
                {
                    "category": "warables",
                    "ui-sort": 4,
                    "type": [
                        "106"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJEZXZpY2VzIC8gVlIiPjxnIGlkPSJHcm91cCAzNTQ1Ij48cGF0aCBpZD0iVmVjdG9yIDMzX19fX18wXzBfWElNT0NZREtNSiIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIxLjA4MzMzIiBkPSJNMy45MzkxIDExLjIxMzlDMy45MzkxIDEwLjc5MDEgNC4wNDg0MiAxMC4zOTYzIDQuMjk3MjggMTAuMDcyNkM0LjU0NDIxIDkuNzUxNDEgNC45NjUyNiA5LjQ1MTUxIDUuNjc2MjEgOS4yNzgyNkM2LjAyMTMxIDkuMTk0MTYgNi42NzIxNSA5LjEzMTgyIDcuNTIyODYgOS4wOTU3M0M4LjM1ODcxIDkuMDYwMjggOS4zNDUzMSA5LjA1MTQ2IDEwLjMzNTggOS4wNjE5MUMxMi4zMjgyIDkuMDgyOTMgMTQuMjg3OSA5LjE4MTMyIDE1LjA0OTMgOS4yODgzOEMxNi4xOTM2IDkuNDQ5MjcgMTYuODM0MyAxMC4yNTk1IDE2LjgzNDMgMTEuMjEzOUMxNi44MzQzIDExLjgxNiAxNi44MDUyIDEyLjU3MTcgMTYuNzI1NyAxMy4yOTg5QzE2LjY0NSAxNC4wMzY3IDE2LjUxNjggMTQuNzAyMiAxNi4zMzYyIDE1LjE0ODNDMTYuMTEzMiAxNS42OTkzIDE1LjgwMzIgMTUuOTgwNiAxNS40NjM5IDE2LjEzOTdDMTUuMTAyNyAxNi4zMDkxIDE0LjY2MjIgMTYuMzYxNiAxNC4xNTAxIDE2LjM2MTZDMTMuNDExNSAxNi4zNjE2IDEyLjkxIDE2LjA1ODYgMTIuMzc4NiAxNS42ODQ1QzEyLjMzMDkgMTUuNjUwOSAxMi4yODE5IDE1LjYxNTcgMTIuMjMxNCAxNS41Nzk1QzExLjc1OTEgMTUuMjQwMyAxMS4xNTk5IDE0LjgwOTkgMTAuMzg2NyAxNC44MDk5QzkuNTk3OTUgMTQuODA5OSA4LjkyMjkxIDE1LjI1NTcgOC4zOTE5NCAxNS42MDYzQzguMzU2ODQgMTUuNjI5NSA4LjMyMjM2IDE1LjY1MjMgOC4yODg1MiAxNS42NzQ1QzcuNjgyODMgMTYuMDcxOCA3LjE5MjU0IDE2LjM2MTYgNi42MjMyNCAxNi4zNjE2QzYuMDEyMjEgMTYuMzYxNiA1LjU3NzM1IDE2LjI0NTYgNS4yNDc5NCAxNi4wNDk3QzQuOTIwNCAxNS44NTUgNC42NTA1MiAxNS41NTM2IDQuNDE1NjkgMTUuMTAxOEM0LjMzOTg3IDE0Ljk1NTkgNC4yNjI5OCAxNC43MTM3IDQuMTk1MTUgMTQuMzgyOUM0LjEyOTA0IDE0LjA2MDQgNC4wNzc5NCAxMy42ODY2IDQuMDM5NDYgMTMuMjk4MkMzLjk2MjUyIDEyLjUyMTYgMy45MzkxIDExLjcxODMgMy45MzkxIDExLjIxMzlaIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8xX1RQRUxIUVFOR0kiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1taXRlcmxpbWl0PSIxMCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0zLjkzNTQ4IDguNTE2MTNWNC43NzQxOUMzLjkzNTQ4IDQuMjU4MDYgNC42ODM3MiA0IDQuNjgzNzIgNEgyMS4yNTE4QzIxLjY3OTMgNCAyMiA0LjM4NzEgMjIgNC43NzQxOVYxNkMyMiAxNi41MTYxIDIxLjY3OTMgMTYuOTAzMiAyMS4yNTE4IDE2LjkwMzJIMTcuNDgzOSIvPjxwYXRoIGlkPSJWZWN0b3IgMzRfX19fXzBfMl9EV0NDSENJU0RKIiBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTcuMTYxMjkgMTkuNDgzOUgxOC43NzQyIi8+PHBhdGggaWQ9IlJlY3RhbmdsZSAxNF9fX19fMF8zX1NOUUdaT0VQWEoiIGZpbGw9ImJsYWNrIiBkPSJNMTYuMTkzNSAxMS4wOTY4SDE4Ljc3NDE1VjEzLjY3NzQ1SDE2LjE5MzV6Ii8+PHBhdGggaWQ9IlJlY3RhbmdsZSAxNV9fX19fMF80X09BS0lSQlhJSVAiIGZpbGw9ImJsYWNrIiBkPSJNMiAxMS4wOTY4SDQuNTgwNjVWMTMuNjc3NDVIMnoiLz48cGF0aCBpZD0iUl9fX19fMF81X1RITEVBVU9CRlkiIGZpbGw9ImJsYWNrIiBkPSJNMTQuNzAyNCAxMS44ODA2QzE0LjcwMjQgMTIuMDY1NiAxNC42Mzg1IDEyLjIxNTggMTQuNTEwOSAxMi4zMzEyQzE0LjM4MzMgMTIuNDQ1NCAxNC4yMjgxIDEyLjUzMTEgMTQuMDQ1NCAxMi41ODgyTDE0LjkwMzIgMTMuNjc3NEgxNC4wMjY1TDEzLjMxNzUgMTIuNjkwNkgxMy4wOTc3VjEzLjY3NzRIMTIuMzIyNlYxMS4wOTY4SDEzLjM1MjlDMTMuNzk4OCAxMS4wOTY4IDE0LjEzNTIgMTEuMTYwNyAxNC4zNjIgMTEuMjg4NkMxNC41ODg5IDExLjQxNjQgMTQuNzAyNCAxMS42MTM4IDE0LjcwMjQgMTEuODgwNlpNMTMuOTAzNiAxMS44ODA2QzEzLjkwMzYgMTEuNzU2NSAxMy44NTcxIDExLjY2NjUgMTMuNzY0MSAxMS42MTA3QzEzLjY3MjggMTEuNTUzNiAxMy41MzY1IDExLjUyNSAxMy4zNTUzIDExLjUyNUgxMy4wOTc3VjEyLjI3MTdIMTMuMzg2QzEzLjU1NDYgMTIuMjcxNyAxMy42ODMgMTIuMjQwNiAxMy43NzEyIDEyLjE3ODZDMTMuODU5NSAxMi4xMTY1IDEzLjkwMzYgMTIuMDE3MiAxMy45MDM2IDExLjg4MDZaIi8+PHBhdGggaWQ9IlZfX19fXzBfNl9RUFBZQkRLVVBWIiBmaWxsPSJibGFjayIgZD0iTTExLjA0MyAxMy4yMDA4TDExLjYwMjQgMTEuMDk2OEgxMi4zMjI2TDExLjQ2NTIgMTMuNjc3NEgxMC41OTVMOS43NDE5NCAxMS4wOTY4SDEwLjQ4NzhMMTEuMDQzIDEzLjIwMDhaIi8+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "VR Glasses",
                    "TW": "VR 眼鏡",
                    "CN": "VR 眼镜",
                    "BR": "Óculos de realidade virtual",
                    "CZ": "Brýle VR",
                    "DA": "VR-briller",
                    "DE": "VR-Brille",
                    "ES": "Gafas de RV",
                    "FI": "Virtuaalitodellisuuslasit",
                    "FR": "Lunettes de RV",
                    "HU": "Virtuális valóság szemüveg",
                    "IT": "Occhiali VR",
                    "JP": "VR グラス",
                    "KR": "VR 안경",
                    "MS": "VR Glasses",
                    "NL": "VR-bril",
                    "NO": "VR-briller",
                    "PL": "Okulary VR",
                    "RU": "Очки виртуальной реальности",
                    "SV": "VR-glasögon",
                    "TH": "แว่นตา VR",
                    "TR": "VR Gözlüğü",
                    "UK": "Окуляри віртуальної реальності",
                    "RO": "Ochelari VR",
                    "SL": "Očala VR"
                },
                {
                    "category": "warables",
                    "ui-sort": 5,
                    "type": [
                        "107"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzI4Ij48ZyBpZD0iR3JvdXAgNDUwOCI+PHBhdGggaWQ9IlZlY3RvciAyOTA0X19fX18wXzBfQllQSVlNUUFXRiIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgZD0iTTExLjc2NzIgNS4zNjIwN0w5Ljc3NTg2IDFMOC42Mzc5MyAyLjcwNjlMNy4wNjU4NSAzLjY4OTQ1QzQuMjUzNDIgNS40NDcyMiAyLjQ5NTQ1IDguNDg1MDMgMi4zNzI2OSAxMS43OTkzTDIuMDcwMjQgMTkuOTY1NU0xMS43NjcyIDUuMzYyMDdWOC41ODYyMU0xMS43NjcyIDUuMzYyMDdMMTMuNzU4NiAxTDE0Ljg5NjUgMi43MDY5TDE5LjI1OTggNS40MzM5M0MyMC4zMjY2IDYuMTAwNjcgMjAuOTkzNCA3LjI1Mjk0IDIxLjA0IDguNTEwMDhMMjEuNDY0MiAxOS45NjU1TTQuNjU1MTcgMTkuOTY1NVYyMS44NjIxSDJMMi4wNzAyNCAxOS45NjU1TTQuNjU1MTcgMTkuOTY1NUw2LjE3MjQxIDEwLjY3MjRNNC42NTUxNyAxOS45NjU1SDIuMDcwMjRNNi4xNzI0MSAxMC42NzI0VjIzSDExLjc2NzJIMTcuMzYyMVYyMS42MjVNNi4xNzI0MSAxMC42NzI0VjguMzk2NTVNMTguODc5MyAxOS45NjU1VjIxLjg2MjFIMjEuNTM0NUwyMS40NjQyIDE5Ljk2NTVNMTguODc5MyAxOS45NjU1TDE3LjM2MjEgMTAuNjcyNE0xOC44NzkzIDE5Ljk2NTVIMjEuNDY0Mk0xNy4zNjIxIDEwLjY3MjRWOC4zOTY1NU0xNy4zNjIxIDEwLjY3MjRWMjEuNjI1TTExLjY3MjQgMTcuNjg5N0MxNC4xODYzIDE3LjY4OTcgMTYuMjI0MSAxNS42NTE4IDE2LjIyNDEgMTMuMTM3OUMxNi4yMjQxIDEwLjYyNDEgMTQuMTg2MyA4LjU4NjIxIDExLjY3MjQgOC41ODYyMUM5LjE1ODU2IDguNTg2MjEgNy4xMjA2OSAxMC42MjQxIDcuMTIwNjkgMTMuMTM3OUM3LjEyMDY5IDE1LjY1MTggOS4xNTg1NiAxNy42ODk3IDExLjY3MjQgMTcuNjg5N1pNMTEuNjcyNCAxNy42ODk3TDExLjc0MzUgMjEuNjI1TTYuMTI1IDIxLjYyNUgxMS43NDM1TTE3LjM2MjEgMjEuNjI1SDExLjc0MzUiLz48cGF0aCBpZD0iRWxsaXBzZSAxMjc0X19fX18wXzFfVkJXWE9NQUNXQyIgZmlsbD0iYmxhY2siIGQ9Ik0xMS4yNTY5IDE3LjA0MTdMMTEuMjU2OSAxNC4xMTFMOS43OTE1OSAxNC4xMTFMMTEuOTg5NiA5LjcxNDkxTDExLjk4OTYgMTIuNjQ1NkwxMy40NTUgMTIuNjQ1NkwxMS4yNTY5IDE3LjA0MTdaIi8+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "Apparel",
                    "TW": "服飾",
                    "CN": "服装",
                    "BR": "Vestuário",
                    "CZ": "Oděvy",
                    "DA": "Tøj",
                    "DE": "Gerät",
                    "ES": "Atuendo",
                    "FI": "Vaatetus",
                    "FR": "Vêtements",
                    "HU": "Felszerelés",
                    "IT": "Abbigliamento",
                    "JP": "アパレル",
                    "KR": "의류",
                    "MS": "Apparel",
                    "NL": "Kleding",
                    "NO": "Klær",
                    "PL": "Odzież",
                    "RU": "Одежда",
                    "SV": "Wearable",
                    "TH": "เครื่องแต่งกาย",
                    "TR": "Giysi",
                    "UK": "Одяг",
                    "RO": "Îmbrăcăminte",
                    "SL": "Naprava"
                },
                {
                    "category": "warables",
                    "ui-sort": 6,
                    "type": [
                        "108"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzI5IiBjbGlwLXBhdGg9InVybCgjY2xpcDBfMzE1XzI4MDUpIj48ZyBpZD0iR3JvdXAgNDUzOSI+PHBhdGggaWQ9IkVsbGlwc2UgMTIyN19fX19fMF8wX0lSUktFSEFEUVQiIGZpbGw9ImJsYWNrIiBkPSJNMTcuMTMxOSAyMC4yNDMzTDE3LjEzMTkgMTcuMzEyNkwxNS42NjY2IDE3LjMxMjZMMTcuODY0NiAxMi45MTY1TDE3Ljg2NDYgMTUuODQ3MkwxOS4zMyAxNS44NDcyTDE3LjEzMTkgMjAuMjQzM1oiLz48cGF0aCBpZD0iUmVjdGFuZ2xlIDEwMjQwX19fX18wXzFfWUhMSUdSQkhCRSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTEzLjI2OTIgMy41Mzg0NlYxSDguNjE1MzhNMTMuMjY5MiAzLjUzODQ2SDExLjM2NTRNMTMuMjY5MiAzLjUzODQ2TDEzLjUwODQgOS4wMzg0Nk0xLjQyMzA4IDMuNTM4NDZWMUg2LjI4ODQ2TTEuNDIzMDggMy41Mzg0NkgzLjc1TTEuNDIzMDggMy41Mzg0NkwxLjMwMzUxIDkuMDM4NDZNNy41NTc2OSA5LjQ2MTU0TDUuMDE5MjMgMjNIMUwxLjMwMzUxIDkuMDM4NDZNNy41NTc2OSA5LjQ2MTU0TDEwLjA5NjIgMjNIMTQuMTE1NEwxMy45NjgyIDIwLjE1ODNNNy41NTc2OSA5LjQ2MTU0VjUuODY1MzhNMy43NSAzLjUzODQ2VjcuMzQ2MTVMMS4zMDM1MSA5LjAzODQ2TTMuNzUgMy41Mzg0Nkg2LjI4ODQ2TTExLjM2NTQgMy41Mzg0NlY3LjM0NjE1TDEzLjUwODQgOS4wMzg0Nk0xMS4zNjU0IDMuNTM4NDZIOC42MTUzOE0xMy41MDg0IDkuMDM4NDZMMTMuNjczOSAxMi44NDYyTTYuMjg4NDYgMVYzLjUzODQ2TTYuMjg4NDYgMUg4LjYxNTM4TTYuMjg4NDYgMy41Mzg0Nkg4LjYxNTM4TTguNjE1MzggMVYzLjUzODQ2TTEyLjQyMzEgMTYuMjMwOEMxMi40MjMxIDE5LjAzNDcgMTQuNjk2MSAyMS4zMDc3IDE3LjUgMjEuMzA3N0MyMC4zMDM5IDIxLjMwNzcgMjIuNTc2OSAxOS4wMzQ3IDIyLjU3NjkgMTYuMjMwOEMyMi41NzY5IDEzLjQyNjkgMjAuMzAzOSAxMS4xNTM4IDE3LjUgMTEuMTUzOEMxNC42OTYxIDExLjE1MzggMTIuNDIzMSAxMy40MjY5IDEyLjQyMzEgMTYuMjMwOFpNMTIuNDIzMSAxNi4yMzA4TDEwLjczMDggMTUuMTczMVYxMy42OTIzTTEwLjczMDggMTMuNjkyM0MxMS40MzE3IDEzLjY5MjMgMTIgMTMuMTI0MSAxMiAxMi40MjMxQzEyIDExLjcyMjEgMTEuNDMxNyAxMS4xNTM4IDEwLjczMDggMTEuMTUzOEMxMC4wMjk4IDExLjE1MzggOS40NjE1NCAxMS43MjIxIDkuNDYxNTQgMTIuNDIzMUM5LjQ2MTU0IDEzLjEyNDEgMTAuMDI5OCAxMy42OTIzIDEwLjczMDggMTMuNjkyM1oiLz48L2c+PC9nPjwvZz48ZGVmcz48Y2xpcFBhdGggaWQ9ImNsaXAwXzMxNV8yODA1Ij48cGF0aCBmaWxsPSJ3aGl0ZSIgZD0iTTAgMEgyNFYyNEgweiIvPjwvY2xpcFBhdGg+PC9kZWZzPjwvc3ZnPg==",
                    "EN": "Trousers",
                    "TW": "褲子",
                    "CN": "裤子",
                    "BR": "Calças",
                    "CZ": "Kalhoty",
                    "DA": "Bukser",
                    "DE": "Hose",
                    "ES": "Pantalones",
                    "FI": "Housut",
                    "FR": "Pantalon",
                    "HU": "Nadrág",
                    "IT": "Pantaloni",
                    "JP": "ズボン",
                    "KR": "바지",
                    "MS": "Trousers",
                    "NL": "Broek",
                    "NO": "Bukser",
                    "PL": "Spodnie",
                    "RU": "Брюки",
                    "SV": "Byxor",
                    "TH": "กางเกง",
                    "TR": "Pantolon",
                    "UK": "Штани",
                    "RO": "Pantaloni",
                    "SL": "Hlače"
                },
                {
                    "category": "warables",
                    "ui-sort": 7,
                    "type": [
                        "109"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzMwIj48ZyBpZD0iR3JvdXAgNDQ5OCI+PHBhdGggaWQ9IlZlY3RvciAyOTAxX19fX18wXzBfRVhCSkFKUkRIRSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xMi40ODA2IDMuNjYxMjlWMi4wNjQ1MUMxMi40ODA2IDEuNDc2NiAxMi45NTcyIDEgMTMuNTQ1MSAxVjFIMTQuNzg3TTEyLjQ4MDYgMy42NjEyOUgxMy41NDUxSDE0Ljc4N00xMi40ODA2IDMuNjYxMjlWNi41TTE5LjkzMjIgMy42NjEyOVYyLjE1MzIyQzE5LjkzMjIgMS41MTYzMSAxOS40MTU4IDEgMTguNzc4OSAxVjFIMTcuNjI1N00xOS45MzIyIDMuNjYxMjlIMTguNzc4OUgxNy42MjU3TTE5LjkzMjIgMy42NjEyOVY2LjVNMTkuOTMyMiAxMC43NTgxVjEwLjc1ODFDMTkuOTMyMiAxMi44MDI5IDE5LjExOTkgMTQuNzYzOSAxNy42NzQgMTYuMjA5OEwxMy4xOTAzIDIwLjY5MzVNMTkuOTMyMiAxMC43NTgxVjEwLjc1ODFDMTcuMjMzIDEwLjIzMzIgMTUuMDU1MyAxMi45NTYyIDE2LjE2MDcgMTUuNDczOUwxNi43Mzg2IDE2Ljc5MDNNMTkuOTMyMiAxMC43NTgxVjcuODc1TTguMTYxODYgMTUuNzI1OFYxNS43MjU4QzYuOTE2NzUgMTcuMDA0NiA2LjkzMDM3IDE5LjA0NjUgOC4xOTI0MSAyMC4zMDg2TDguNTc3MzkgMjAuNjkzNUM5Ljg1MTIgMjEuOTY3NCAxMS45MTY1IDIxLjk2NzQgMTMuMTkwMyAyMC42OTM1VjIwLjY5MzVNOC4xNjE4NiAxNS43MjU4TDkuMjM5MzcgMTYuNzkwM004LjE2MTg2IDE1LjcyNThMMTEuNDE2MSAxMi4zODM2TTEzLjE5MDMgMjAuNjkzNUwxMC42NzYxIDE4LjIwOTdNMTIuNDgwNiA5LjY5MzU1VjcuODc1TTE0Ljc4NyAxVjMuNjYxMjlNMTQuNzg3IDFIMTYuMjA2NE0xNC43ODcgMy42NjEyOUgxNi4yMDY0TTE2LjIwNjQgMVYzLjY2MTI5TTE2LjIwNjQgMUgxNy42MjU3TTE2LjIwNjQgMy42NjEyOUgxNy42MjU3TTE3LjYyNTcgMVYzLjY2MTI5TTEyLjQ4MDYgNy44NzVIMTkuOTMyMk0xMi40ODA2IDcuODc1VjYuNU0xOS45MzIyIDcuODc1VjYuNU0xMi40ODA2IDYuNUgxOS45MzIyIi8+PHBhdGggaWQ9IkVsbGlwc2UgMTI2NF9fX19fMF8xX0hIRURPUFFPRVEiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTMuNDc3NSAxMS4yOTA0QzEzLjQ3NzUgMTEuOTM4OCAxMi45NTE4IDEyLjQ2NDUgMTIuMzAzMyAxMi40NjQ1QzExLjY1NDkgMTIuNDY0NSAxMS4xMjkyIDExLjkzODggMTEuMTI5MiAxMS4yOTA0QzExLjEyOTIgMTAuNjQxOSAxMS42NTQ5IDEwLjExNjIgMTIuMzAzMyAxMC4xMTYyQzEyLjk1MTggMTAuMTE2MiAxMy40Nzc1IDEwLjY0MTkgMTMuNDc3NSAxMS4yOTA0WiIvPjxwYXRoIGlkPSJFbGxpcHNlIDEyNjVfX19fXzBfMl9aUEtXSlpaWVJUIiBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTExLjcwMzIgMTYuOTY3OEMxMS43MDMyIDE3LjYxNjMgMTEuMTc3NSAxOC4xNDIgMTAuNTI5IDE4LjE0MkM5Ljg4MDU0IDE4LjE0MiA5LjM1NDg0IDE3LjYxNjMgOS4zNTQ4NCAxNi45Njc4QzkuMzU0ODQgMTYuMzE5MyA5Ljg4MDU0IDE1Ljc5MzYgMTAuNTI5IDE1Ljc5MzZDMTEuMTc3NSAxNS43OTM2IDExLjcwMzIgMTYuMzE5MyAxMS43MDMyIDE2Ljk2NzhaIi8+PHBhdGggaWQ9IkVsbGlwc2UgMTI2Nl9fX19fMF8zX0dSVVNRUEdGQUoiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTUuNjA2MyAxNi4yNTgxQzE1LjYwNjMgMTYuNzEwNiAxNS4yMzk1IDE3LjA3NzQgMTQuNzg3IDE3LjA3NzRDMTQuMzM0NSAxNy4wNzc0IDEzLjk2NzYgMTYuNzEwNiAxMy45Njc2IDE2LjI1ODFDMTMuOTY3NiAxNS44MDU2IDE0LjMzNDUgMTUuNDM4NyAxNC43ODcgMTUuNDM4N0MxNS4yMzk1IDE1LjQzODcgMTUuNjA2MyAxNS44MDU2IDE1LjYwNjMgMTYuMjU4MVoiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzRfTllFU0FDREpXTiIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTkuNTMyODMgNi4wMzg2NUM4LjI4MTUyIDYuNDgwNiA3LjE5ODggNy4zOTk5MSA2LjU4MTI4IDguNjkxODNDNS45NjM3NSA5Ljk4Mzc1IDUuOTI4MzMgMTEuNDAzNyA2LjM3MDI4IDEyLjY1NU04LjgxNDk5IDExLjc5MTZDOC41OTQwMiAxMS4xNjU5IDguNjExNzMgMTAuNDU1OSA4LjkyMDQ5IDkuODA5OTdDOS4yMjkyNSA5LjE2NDAxIDkuNzcwNjEgOC43MDQzNiAxMC4zOTYzIDguNDgzMzgiLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Socks",
                    "TW": "襪子",
                    "CN": "袜子",
                    "BR": "Meias",
                    "CZ": "Ponožky",
                    "DA": "Strømper",
                    "DE": "Socken",
                    "ES": "Calcetines",
                    "FI": "Sukat",
                    "FR": "Chaussettes",
                    "HU": "Zokni",
                    "IT": "Calzini",
                    "JP": "ソックス",
                    "KR": "양말",
                    "MS": "Socks",
                    "NL": "Sokken",
                    "NO": "Sokker",
                    "PL": "Skarpety",
                    "RU": "Носки",
                    "SV": "Strumpor",
                    "TH": "ถุงเท้า",
                    "TR": "Çorap",
                    "UK": "Шкарпетки",
                    "RO": "Șosete",
                    "SL": "Nogavice"
                },
                {
                    "category": "warables",
                    "ui-sort": 8,
                    "type": [
                        "110"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzMxIj48ZyBpZD0iR3JvdXAgNDQ5NSI+PHBhdGggaWQ9IlJlY3RhbmdsZSAxMDIyNl9fX19fMF8wX0xJSFdTQkJWUkQiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNOC40MDgxNiA5LjY4MjA2SDYuNjEyMjRDMy41MTI2OSA5LjY4MjA2IDEgMTIuMTk0NyAxIDE1LjI5NDNWMTUuMjk0M0MxIDE4LjM5MzkgMy41MTI2OSAyMC45MDY1IDYuNjEyMjQgMjAuOTA2NUgxNy4zODc4QzIwLjQ4NzMgMjAuOTA2NSAyMyAxOC4zOTM5IDIzIDE1LjI5NDNWMTUuMjk0M0MyMyAxMi4xOTQ3IDIwLjQ4NzMgOS42ODIwNiAxNy4zODc4IDkuNjgyMDZIMTUuNzA0MU04LjQwODE2IDExLjQ3OEg2LjgzNjczQzQuNzI5MDMgMTEuNDc4IDMuMDIwNDEgMTMuMTg2NiAzLjAyMDQxIDE1LjI5NDNWMTUuMjk0M0MzLjAyMDQxIDE3LjQwMiA0LjcyOTAzIDE5LjExMDYgNi44MzY3MyAxOS4xMTA2SDE3LjE2MzNDMTkuMjcxIDE5LjExMDYgMjAuOTc5NiAxNy40MDIgMjAuOTc5NiAxNS4yOTQzVjE1LjI5NDNDMjAuOTc5NiAxMy4xODY2IDE5LjI3MSAxMS40NzggMTcuMTYzMyAxMS40NzhIMTUuNzA0MU0xMC42NTMxIDkuMjMzMDhIOS4xODM2N0M4LjYzMTM5IDkuMjMzMDggOC4xODM2NyA5LjY4MDc5IDguMTgzNjcgMTAuMjMzMVYxMC45MjdDOC4xODM2NyAxMS40NzkyIDguNjMxMzkgMTEuOTI3IDkuMTgzNjcgMTEuOTI3SDE0LjgxNjNDMTUuMzY4NiAxMS45MjcgMTUuODE2MyAxMS40NzkyIDE1LjgxNjMgMTAuOTI3VjEwLjIzMzFDMTUuODE2MyA5LjY4MDc5IDE1LjM2ODYgOS4yMzMwOCAxNC44MTYzIDkuMjMzMDhIMTMuMjM0NyIvPjxwYXRoIGlkPSJFbGxpcHNlIDEyNThfX19fXzBfMV9XRUJMRldEREdMIiBmaWxsPSJibGFjayIgZD0iTTEyLjQyNjcgOS4yMzM4NkMxMi40MjY3IDkuNDgxODMgMTIuMjI1NyA5LjY4Mjg0IDExLjk3NzcgOS42ODI4NEMxMS43Mjk4IDkuNjgyODQgMTEuNTI4NyA5LjQ4MTgzIDExLjUyODcgOS4yMzM4NkMxMS41Mjg3IDguOTg1OSAxMS43Mjk4IDguNzg0ODggMTEuOTc3NyA4Ljc4NDg4QzEyLjIyNTcgOC43ODQ4OCAxMi40MjY3IDguOTg1OSAxMi40MjY3IDkuMjMzODZaIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8yX0FJU1lMTkJPSFAiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xNS42NjY3IDUuNTE4NzhDMTQuNzI4NCA0LjU4MDQgMTMuNDMyIDQgMTIuMDAwMSA0QzEwLjU2ODIgNCA5LjI3MTc5IDQuNTgwNCA4LjMzMzQxIDUuNTE4NzhNMTAuMTY2NyA3LjM1MjEyQzEwLjYzNTkgNi44ODI5MiAxMS4yODQxIDYuNTkyNzIgMTIuMDAwMSA2LjU5MjcyQzEyLjcxNiA2LjU5MjcyIDEzLjM2NDIgNi44ODI5MiAxMy44MzM0IDcuMzUyMTIiLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Belt",
                    "TW": "皮帶",
                    "CN": "腰带",
                    "BR": "Cinto",
                    "CZ": "Opasek",
                    "DA": "Bælte",
                    "DE": "Gurt",
                    "ES": "Cinturón",
                    "FI": "Vyö",
                    "FR": "Ceinture",
                    "HU": "Szalag",
                    "IT": "Cintura",
                    "JP": "ベルト",
                    "KR": "벨트",
                    "MS": "Belt",
                    "NL": "Riem",
                    "NO": "Belte",
                    "PL": "Pasek",
                    "RU": "Ремень",
                    "SV": "Bälte",
                    "TH": "เข็มขัด",
                    "TR": "Kemer",
                    "UK": "Ремінь",
                    "RO": "Centură",
                    "SL": "Trak"
                },
                {
                    "category": "warables",
                    "ui-sort": 9,
                    "type": [
                        "111"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzMyIj48ZyBpZD0iR3JvdXAgNDQ5NiI+PHBhdGggaWQ9IlZlY3RvciAyODk3X19fX18wXzBfWU1aTlFCUExNUSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xLjAxMjU2IDEzLjI1MDVDMS4wMTI1NiAxMC4yMzggMi41NTUwOSA5LjU3ODU0IDMuNjI5NDkgOS41ODYxOUM0Ljc3NTAzIDkuNTk0MzUgNC43NzAzNSAxMS44Njc4IDYuNjc1NzUgMTEuODY3OEM5LjE4NDY3IDExLjg2NzggMTAuNTE4NSAxMC4zMTkgMTEuODM3NiA5LjU4NjE0QzExLjkwNjkgOS43MjQ2NSAxMi4wMTk4IDkuODgxMzMgMTIuMTcgMTAuMDUyOE0xLjAxMjU2IDEzLjI1MDVDMS4wMTI3MSAxMy44ODg5IDAuOTg0MTMgMTUuNDQ2MSAxLjAxMjcxIDE1Ljk4ODZDMS4xODE1MyAxOS4xOTM2IDEuNjEzNzMgMTkuNTUzMSAzLjIyNjA0IDE5LjY5OTdDNC42NDM2MiAxOS44Mjg2IDcuMzQyNjcgMTkuNzQ4NiA5LjA1MjY5IDE5LjY5OTdDMTEuODM3NiAxOS42OTk3IDE1LjA2MjIgMTkuNjk5NyAxNy4yNjA4IDE5LjY5OTdDMTkuNDU5NCAxOS42OTk3IDIyLjUzNzQgMTcuMjA4IDIyLjY4NCAxNS4zMDI1QzIyLjY5MTkgMTUuMTk5OCAyMi42OTY4IDE1LjEwMjIgMjIuNjk4OSAxNS4wMDk0TTEuMDEyNTYgMTMuMjUwNUMxLjMwMDc1IDEzLjc2NjIgMS44NzgyNyAxNC4yNTYyIDIuNTU1MDkgMTUuMDA5NE0yMi42OTg5IDE1LjAwOTRDMjIuNzM1MyAxMy4zODE3IDIxLjg5NTggMTMuMjUwNSAyMC45MjUxIDEzLjI1MDVDMTkuODk5MSAxMy4yNTA1IDE4LjU4IDEzLjU0MzcgMTYuODIxMSAxMy41NDM3QzE2LjcwODQgMTMuNDczMiAxNi41OTYxIDEzLjQwMjUgMTYuNDg0MyAxMy4zMzE1TTIyLjY5ODkgMTUuMDA5NEMyMC45MjA2IDE3LjYzNTMgMTcuOTQ2MyAxOC4xMjI4IDE1Ljc5NTEgMTguMjEzNE0yLjU1NTA5IDE1LjAwOTRDMy4zNTMyIDE1Ljg5NzYgMi4yNTU5MyAxOC4yMTM0IDMuNjI5NDkgMTguMjM0QzYuMzc2MjkgMTguMjc1MyAxMi40MjM5IDE4LjIzNCAxNC40NzU5IDE4LjIzNEMxNC44NTY5IDE4LjIzNCAxNS4zMDQ2IDE4LjIzNCAxNS43OTUxIDE4LjIxMzRNMi41NTUwOSAxNS4wMDk0QzMuNjgxNjIgMTQuNDIzMSA1LjI0MTg0IDEzLjM5NzEgNy40NDA0NCAxMy41NDM3QzkuNjM5MDMgMTMuNjkwMiAxMy43NDMxIDE1LjI2MTMgMTUuNzk1MSAxOC4yMTM0TTEyLjE3IDEwLjA1MjhMMTEuNzc3IDEwLjYzNTFDMTEuNDgyOCAxMS4wNzEzIDExLjU3NjggMTEuNjYwOCAxMS45OTIxIDExLjk4MzhMMTQuNjI1NCAxNC4wMzE5QzE1LjA0NTUgMTQuMzU4NyAxNS42NDc5IDE0LjI5OTIgMTUuOTk1OSAxMy44OTY2TDE2LjQ4NDMgMTMuMzMxNU0xMi4xNyAxMC4wNTI4QzEyLjkyNzcgMTAuOTE3NSAxNC42MzUzIDEyLjE1NzUgMTYuNDg0MyAxMy4zMzE1Ii8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8xX1lDV1NTQU5FVkkiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xMS43MTIgNi41MTcxOUMxMC43NzQ3IDUuNTc5NzkgOS40Nzk2NSA1IDguMDQ5MjQgNUM2LjYxODgyIDUgNS4zMjM4MiA1LjU3OTc5IDQuMzg2NDIgNi41MTcxOU02LjIxNzgzIDguMzQ4NTlDNi42ODY1MyA3Ljg3OTg5IDcuMzM0MDMgNy41OSA4LjA0OTI0IDcuNTlDOC43NjQ0NCA3LjU5IDkuNDExOTQgNy44Nzk4OSA5Ljg4MDY0IDguMzQ4NTkiLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Shoes",
                    "TW": "鞋子",
                    "CN": "鞋",
                    "BR": "Sapatos",
                    "CZ": "Obuv",
                    "DA": "Sko",
                    "DE": "Schuhe",
                    "ES": "Zapatos",
                    "FI": "Kengät",
                    "FR": "Chaussures",
                    "HU": "Cipő",
                    "IT": "Scarpe",
                    "JP": "靴",
                    "KR": "신발",
                    "MS": "Shoes",
                    "NL": "Schoenen",
                    "NO": "Sko",
                    "PL": "Buty",
                    "RU": "Ботинки",
                    "SV": "Skor",
                    "TH": "รองเท้า",
                    "TR": "Ayakkabı",
                    "UK": "Взуття",
                    "RO": "Pantofi",
                    "SL": "Čevlji"
                },
                {
                    "category": "warables",
                    "ui-sort": 10,
                    "type": [
                        "112"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzM0Ij48ZyBpZD0iR3JvdXAgNDUwNyI+PGcgaWQ9Ikdyb3VwIDQ1MDkiPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMF9MV09ZQUNIQkREIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTYuNDM4NCA3LjAyMTc5VjEwLjAyNzJDMTYuNDM4MyAxMC40MjM1IDE2LjMyMSAxMC44MTA5IDE2LjEwMTEgMTEuMTQwNkwxNS4xNjk4IDEyLjUzOEMxNC45NDk5IDEyLjg2NzcgMTQuODMyNSAxMy4yNTUyIDE0LjgzMjUgMTMuNjUxNU0xMC4wMTUgMTMuNjUxNUMxMC4wMTUgMTMuMjU1MiA5Ljg5NzY1IDEyLjg2NzcgOS42Nzc3OCAxMi41MzhMOC43NDYzOSAxMS4xNDA2QzguNTI2NTIgMTAuODEwOSA4LjQwOTE4IDEwLjQyMzUgOC40MDkxNyAxMC4wMjcyVjUuNDE1OTRDOC42MjAwNSA1LjQxNTk0IDguODI4ODcgNS40NTc0OCA5LjAyMzcgNS41MzgxOEM5LjIxODUzIDUuNjE4ODggOS4zOTU1NSA1LjczNzE3IDkuNTQ0NjcgNS44ODYyOEM5LjY5Mzc4IDYuMDM1NCA5LjgxMjA3IDYuMjEyNDMgOS44OTI3NyA2LjQwNzI2QzkuOTczNDcgNi42MDIwOSAxMC4wMTUgNi44MTA5MSAxMC4wMTUgNy4wMjE3OU0xMC4wMTUgMjEuNTU0N1YyM0gxNC44MzI1VjIxLjU1NDciLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzFfS0hUWVBLRlVFTCIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE2LjQzODQgNy40MjMxOVY1LjQxNTg4QzE2LjQzODQgNS4yMDI5MyAxNi4zNTM4IDQuOTk4NzEgMTYuMjAzMiA0Ljg0ODEzQzE2LjA1MjYgNC42OTc1NSAxNS44NDg0IDQuNjEyOTYgMTUuNjM1NCA0LjYxMjk2QzE1LjQyMjUgNC42MTI5NiAxNS4yMTgzIDQuNjk3NTUgMTUuMDY3NyA0Ljg0ODEzQzE0LjkxNzEgNC45OTg3MSAxNC44MzI1IDUuMjAyOTMgMTQuODMyNSA1LjQxNTg4VjcuNDIzMTkiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzJfRExYRFlMWUlTWCIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE0LjgzMjQgNy40MjM0OFYzLjAwNzRDMTQuODMyNCAyLjc5NDQ1IDE0Ljc0NzggMi41OTAyMiAxNC41OTcyIDIuNDM5NjVDMTQuNDQ2NyAyLjI4OTA3IDE0LjI0MjQgMi4yMDQ0NyAxNC4wMjk1IDIuMjA0NDdDMTMuODE2NSAyLjIwNDQ3IDEzLjYxMjMgMi4yODkwNyAxMy40NjE3IDIuNDM5NjVDMTMuMzExMiAyLjU5MDIyIDEzLjIyNjYgMi43OTQ0NSAxMy4yMjY2IDMuMDA3NFY3LjQyMzQ4Ii8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8zX0JERlNLVFdKWkwiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xMy4yMjY0IDcuNDIzMzlWMS44ODMyMkMxMy4yMzY1IDEuNjU5NzUgMTMuMTU3NyAxLjQ0MTM3IDEzLjAwNzMgMS4yNzU4NUMxMi44NTY4IDEuMTEwMzQgMTIuNjQ2OSAxLjAxMTE1IDEyLjQyMzUgMVYxQzEyLjIwMDEgMS4wMTExNSAxMS45OTAyIDEuMTEwMzQgMTEuODM5NyAxLjI3NTg1QzExLjY4OTIgMS40NDEzNyAxMS42MTA0IDEuNjU5NzUgMTEuNjIwNSAxLjg4MzIyVjcuNDIzMzkiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzRfUU1YRUdXWVVWRiIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTExLjYyMDUgNy40MjM0OFYzLjAwNzRDMTEuNjIwNSAyLjc5NDQ1IDExLjUzNTkgMi41OTAyMiAxMS4zODUzIDIuNDM5NjVDMTEuMjM0OCAyLjI4OTA3IDExLjAzMDUgMi4yMDQ0NyAxMC44MTc2IDIuMjA0NDdWMi4yMDQ0N0MxMC42MDQ2IDIuMjA0NDcgMTAuNDAwNCAyLjI4OTA3IDEwLjI0OTggMi40Mzk2NUMxMC4wOTkzIDIuNTkwMjIgMTAuMDE0NyAyLjc5NDQ1IDEwLjAxNDcgMy4wMDc0VjcuNDIzNDgiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzVfWUNTVlFZWUJWViIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTExLjYyMDUgMTAuMjMzNkMxMS42MjA2IDEwLjAyMjcgMTEuNTc5MiA5LjgxMzg0IDExLjQ5ODUgOS42MTg5NkMxMS40MTc5IDkuNDI0MDcgMTEuMjk5NiA5LjI0Njk5IDExLjE1MDQgOS4wOTc4NUMxMS4wMDEzIDguOTQ4NzEgMTAuODI0MiA4LjgzMDQzIDEwLjYyOTMgOC43NDk3N0MxMC40MzQ0IDguNjY5MTEgMTAuMjI1NiA4LjYyNzY2IDEwLjAxNDcgOC42Mjc3OFY3LjQyMzQiLz48L2c+PHBhdGggaWQ9IkVsbGlwc2UgMTI3Ml9fX19fMF82X1VMV1BBSk1LVlMiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTguNTI1NSAxMi43MjI4VjIwLjQzMDlDMTguNTI1NSAyMS4wMTI4IDE2LjA2MjIgMjEuNDkxNCAxMi45MDUxIDIxLjU0OTFMMTIuOTA1MSAxMy44NDExQzE2LjA2MjIgMTMuNzgzMyAxOC41MjU1IDEzLjMwNDcgMTguNTI1NSAxMi43MjI4Wk0xOC41MjU1IDEyLjcyMjhDMTguNTI1NSAxMi4zMzcyIDE3LjQ0NCAxMS45OTcgMTUuNzk1NiAxMS43OTQ1TTYgMTIuNzIyOEM2IDEzLjMwNDcgOC40NjMzNiAxMy43ODMzIDExLjYyMDQgMTMuODQxMVYyMS41NDkxQzguNDYzMzYgMjEuNDkxNCA2IDIxLjAxMjggNiAyMC40MzA5VjEyLjcyMjhaTTYgMTIuNzIyOEM2IDEyLjMwMDggNy4yOTU1NCAxMS45MzMxIDkuMjExNjggMTEuNzQwOU0xMC4wMTQ2IDE2LjA5NTFWMTkuNDY3NE0xNC41MTA5IDE2LjA5NTFWMTkuNDY3NCIvPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Blood Pressure Monitor",
                    "TW": "血壓監測儀",
                    "CN": "血压计",
                    "BR": "Monitor de pressão arterial",
                    "CZ": "Sledování tlaku",
                    "DA": "Blodtryksmonitor",
                    "DE": "Blutdruckmonitor",
                    "ES": "Monitor de presión arterial",
                    "FI": "Verenpainemittari",
                    "FR": "Moniteur de tension artérielle",
                    "HU": "Vérnyomásmérő",
                    "IT": "Monitor pressione sanguigna",
                    "JP": "血圧モニタ",
                    "KR": "혈압 모니터",
                    "MS": "Blood Pressure Monitor",
                    "NL": "Bloeddrukmonitor",
                    "NO": "Blodtrykksmåler",
                    "PL": "Ciśnieniomierz",
                    "RU": "Тонометр",
                    "SV": "Blodtrycksmätare",
                    "TH": "เครื่องวัดความดันโลหิต",
                    "TR": "Tansiyon İzleyici",
                    "UK": "Тонометр",
                    "RO": "Monitor de tensiune arterială",
                    "SL": "Merilnik krvnega tlaka"
                },
                {
                    "category": "warables",
                    "ui-sort": 11,
                    "type": [
                        "113"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzMzIiBjbGlwLXBhdGg9InVybCgjY2xpcDBfMzE1XzI4MzcpIj48ZyBpZD0iR3JvdXAgNDUwNiI+PHJlY3QgaWQ9IlJlY3RhbmdsZSAxMDIzOV9fX19fMF8wX0NDR01MRVZYUUsiIHdpZHRoPSIxMS4yNTYiIGhlaWdodD0iNi44Nzg4OSIgeD0iOC4wNTg1MSIgeT0iMTEuNzU2IiBzdHJva2U9ImJsYWNrIiByeD0iMi41IiB0cmFuc2Zvcm09InJvdGF0ZSgtOTAgOC4wNTg1MSAxMS43NTYpIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8xX1VETlVGVlBEWEwiIGZpbGw9ImJsYWNrIiBkPSJNMTEuNDk3NSA2LjQ5MTM2TDExLjI0MzYgNi4yMzk0N0MxMC4zNDIgNS4zNDgyOSA5Ljc0NjY2IDQuNzYwNTQgOS43NDY2NiA0LjAzOTJDOS43NDY2NiAzLjQ1MTQ0IDEwLjE3MDQgMi45ODk2MyAxMC43MDk2IDIuOTg5NjNDMTEuMDE0MyAyLjk4OTYzIDExLjMwNjcgMy4xNDQyMSAxMS40OTc1IDMuMzg4NDdDMTEuNjg4NCAzLjE0NDIxIDExLjk4MDggMi45ODk2MyAxMi4yODU0IDIuOTg5NjNDMTIuODI0NyAyLjk4OTYzIDEzLjI0ODQgMy40NTE0NCAxMy4yNDg0IDQuMDM5MkMxMy4yNDg0IDQuNzYwNTQgMTIuNjUzMSA1LjM0ODI5IDExLjc1MTQgNi4yNDEzN0wxMS40OTc1IDYuNDkxMzZaIi8+PGcgaWQ9Ikdyb3VwIDQ1MDUiPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTI2OV9fX19fMF8yX1FQRUNKRFNNWlMiIGN4PSIxMS41Nzc3IiBjeT0iOS4yMjI4MSIgcj0iMS4zMTMxNSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIxLjIiIHRyYW5zZm9ybT0icm90YXRlKC00NSAxMS41Nzc3IDkuMjIyODEpIi8+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMjcwX19fX18wXzNfSEJaVlZOSVRHRyIgY3g9IjExLjU3NzQiIGN5PSI5LjIyMjk3IiByPSIwLjQzNzcxNiIgZmlsbD0iYmxhY2siIHRyYW5zZm9ybT0icm90YXRlKC00NSAxMS41Nzc0IDkuMjIyOTcpIi8+PC9nPjxwYXRoIGlkPSJWZWN0b3IgMjkwM19fX19fMF80X0JKTVpYWUtKT1QiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lam9pbj0icm91bmQiIGQ9Ik0xNi44Mjc3IDIzLjY4OUMxNi43OTUyIDIzLjQ0NzEgMTYuNjkxMyAyMi42MTA4IDE2LjUzNTYgMjEuMjAwOUMxNy4zODgyIDE5Ljc4MjUgMTcuNDY5MSAxNy43MDA5IDE3LjMyMDkgMTYuMDUxNkMxNy4yMTU2IDE0Ljg4IDE2LjI4OTggMTMuOTk1NCAxNS4xMzkyIDEzLjc1MDRMMTQuMDkzIDEzLjUyNzZDMTMuMjI1MSAxMy4zNDI3IDEyLjU3MTQgMTIuNjI1MyAxMi40Njc5IDExLjc0NFYxMS43NDRNMTIuNDQ2MyAyNEMxMi40MTM5IDIzLjY1NDQgMTIuMzEgMjIuNjMxNiAxMi4xNTQyIDIxLjMwNDZDMTEuMjc4IDIwLjE2NDIgNy4zODM0MiAxOC40MDE4IDYuNzk5MjQgMTguMjk4MkM2LjIxNTA2IDE4LjE5NDUgNS4zMzg3OSAxNy4yNjE1IDYuNzk5MjQgMTYuNzQzMUM3Ljk2NzYgMTYuMzI4NSA5LjY4NzY4IDE2LjkxNTkgMTAuNDAxNyAxNy4yNjE1VjExLjc0NCIvPjwvZz48L2c+PC9nPjxkZWZzPjxjbGlwUGF0aCBpZD0iY2xpcDBfMzE1XzI4MzciPjxwYXRoIGZpbGw9IndoaXRlIiBkPSJNMCAwSDI0VjI0SDB6Ii8+PC9jbGlwUGF0aD48L2RlZnM+PC9zdmc+",
                    "EN": "Oximeter",
                    "TW": "血氧計",
                    "CN": "血氧计",
                    "BR": "Oxímetro",
                    "CZ": "Oximetr",
                    "DA": "Oximeter",
                    "DE": "Oximeter",
                    "ES": "Oxímetro",
                    "FI": "Oksimetri",
                    "FR": "Oxymètre",
                    "HU": "Véroxigénszint mérő",
                    "IT": "Ossimetro",
                    "JP": "酸素濃度計",
                    "KR": "산소농도계",
                    "MS": "Oximeter",
                    "NL": "Oximeter",
                    "NO": "Oksymeter",
                    "PL": "Pulsoksymetr",
                    "RU": "Оксиметр",
                    "SV": "Syresättningsmätare",
                    "TH": "เครื่องวัดออกซิเจน",
                    "TR": "Oksimetre",
                    "UK": "Оксиметр",
                    "RO": "Oximetru",
                    "SL": "Oksimeter"
                },
                {
                    "category": "warables",
                    "ui-sort": 12,
                    "type": [
                        "114"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzM1Ij48ZyBpZD0iR3JvdXAgNDQ1OCI+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMjAzX19fX18wXzBfV0RQRUJLWUdBSiIgY3g9IjguODQ2NTMiIGN5PSIxNy4xNTM1IiByPSIzLjk3MTIyIiBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuMiIvPjxwYXRoIGlkPSJFbGxpcHNlIDEyMDVfX19fXzBfMV9VU0pFUVdaWUVOIiBzdHJva2U9ImJsYWNrIiBkPSJNMTIuMTAwNyAxNC44OTYzQzEyLjAzNjcgMTQuODkzNSAxMS45NzI0IDE0Ljg5MjEgMTEuOTA3NyAxNC44OTIxQzkuNDg1OTcgMTQuODkyMSA3LjUyMjc4IDE2Ljg1NTMgNy41MjI3OCAxOS4yNzdDNy41MjI3OCAxOS44OTQyIDcuNjUwMzEgMjAuNDgxNyA3Ljg4MDQ2IDIxLjAxNDQiLz48cGF0aCBpZD0iRWxsaXBzZSAxMjA0X19fX18wXzJfRE1ITUpVR0dIWCIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik04Ljg0NjUzIDIzQzEyLjA3NTUgMjMgMTQuNjkzMSAyMC4zODI0IDE0LjY5MzEgMTcuMTUzNUMxNC42OTMxIDE0LjQ5OTIgMTIuOTI0MyAxMi4yNTgxIDEwLjUwMTIgMTEuNTQ0NEMxMC40ODI4IDEwLjU0NiAxMC40NDYgOC40NTU0IDEwLjQ0NiA4LjA4MDM0QzEwLjQ0NiA3LjYxMTUxIDEwLjA4NzUgNy40MTg0NyA5Ljc1NjYgNy40MTg0N0g3Ljc0MzQxQzcuMzU3MzIgNy40MTg0NyA3LjE5MTg1IDcuODMyMTQgNy4xOTE4NSA4LjIxODIzVjExLjU0NDRDNC43Njg3MyAxMi4yNTgxIDMgMTQuNDk5MiAzIDE3LjE1MzVDMyAyMC4zODI0IDUuNjE3NTggMjMgOC44NDY1MyAyM1oiLz48Y2lyY2xlIGlkPSJFbGxpcHNlIDEyMDZfX19fXzBfM19NRENQR0NUS0JMIiBjeD0iOC44NDY1MyIgY3k9IjExLjQ3MjQiIHI9IjAuOTM3NjUiIGZpbGw9ImJsYWNrIi8+PHBhdGggaWQ9IkVsbGlwc2UgMTIwN19fX19fMF80X1VJSk1OTU5aUUkiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNNy4yMTk0MiA5Ljc0NDA2QzUuNTI1NDggOS4wOTA3IDQuMzIzNzMgNy40NDcwOCA0LjMyMzczIDUuNTIyNzhDNC4zMjM3MyAzLjAyNDkyIDYuMzQ4NjUgMSA4Ljg0NjUxIDFDMTEuMzQ0NCAxIDEzLjM2OTMgMy4wMjQ5MiAxMy4zNjkzIDUuNTIyNzhDMTMuMzY5MyA3LjQ1NzUxIDEyLjE1NDUgOS4xMDg1IDEwLjQ0NiA5Ljc1NDU5Ii8+PHBhdGggaWQ9IkVsbGlwc2UgMTIwOF9fX19fMF81X1BSUU9KWExVS1EiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNNy4yNzQzNiA4LjkyODg2QzUuOTg4MTQgOC4zMzQzIDUuMDk1NyA3LjAzMjY2IDUuMDk1NyA1LjUyMjU3QzUuMDk1NyAzLjQ1MTE3IDYuNzc0OSAxLjc3MTk3IDguODQ2MyAxLjc3MTk3QzEwLjkxNzcgMS43NzE5NyAxMi41OTY5IDMuNDUxMTcgMTIuNTk2OSA1LjUyMjU3QzEyLjU5NjkgNy4wMzI2NiAxMS43MDQ1IDguMzM0MyAxMC40MTgyIDguOTI4ODYiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzZfWUhDRkpDQVdISSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE5LjEyMTUgMjEuNDg0OEMyMC4yMDczIDIwLjM5OSAyMC44Nzg5IDE4Ljg5OSAyMC44Nzg5IDE3LjI0MjJDMjAuODc4OSAxNS41ODUzIDIwLjIwNzMgMTQuMDg1MyAxOS4xMjE1IDEyLjk5OTVNMTcuMDAwMiAxNS4xMjA4QzE3LjU0MzEgMTUuNjYzNyAxNy44Nzg5IDE2LjQxMzcgMTcuODc4OSAxNy4yNDIyQzE3Ljg3ODkgMTguMDcwNiAxNy41NDMxIDE4LjgyMDYgMTcuMDAwMiAxOS4zNjM1Ii8+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "Tracking Tag",
                    "TW": "追蹤標籤",
                    "CN": "追踪标签",
                    "BR": "Etiqueta de rastreamento",
                    "CZ": "Sledovací zařízení",
                    "DA": "Sporingstag",
                    "DE": "Tracking-Anhänger",
                    "ES": "Etiqueta de seguimiento",
                    "FI": "Seurantatunniste",
                    "FR": "Étiquette de suivi",
                    "HU": "Új csatlakoztatott mash csomópont észlelése",
                    "IT": "Tag di tracciamento",
                    "JP": "トラッキング タグ",
                    "KR": "추적 태그",
                    "MS": "Tracking Tag",
                    "NL": "Trackingtag",
                    "NO": "Sporingsbrikke",
                    "PL": "Znacznik śledzący",
                    "RU": "Метка слежения",
                    "SV": "Spårningstagg",
                    "TH": "แท็กติดตาม",
                    "TR": "Takip Cihazı",
                    "UK": "Трекер",
                    "RO": "Etichetă de urmărire",
                    "SL": "Sledilna oznaka"
                },
                {
                    "category": "warables",
                    "ui-sort": 13,
                    "type": [
                        "132"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJXZWFyYWJsZXMvSGVhcnQgUmF0ZSBTZW5zb3IiPjxwYXRoIGlkPSJWZWN0b3IgMjkyMF9fX19fMF8wX1NXVEpRUE1CVUsiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjUiIGQ9Ik0yLjIgMTEuNjU2NUMyLjIgOC4yODc5MSA0LjYwNjEyIDUuOTc4MDQgNy4yMDQ3MyA2LjM2MzAyQzkuMjgzNjEgNi42NzEgMTAuNzY1OCA3LjU1OTgzIDExLjI0NyA3Ljk0NDgxQzExLjcyODIgNy41NTk4MyAxMy4yMTA0IDYuNzEyODggMTUuMjg5MyA2LjQwNDg5QzE3Ljg4NzkgNi4wMTk5MSAyMC4yOTQgOC4zMjk3OSAyMC4yOTQgMTEuNjk4NE01LjI3OTgzIDE2Ljk0OTlMMTEuMjQ3IDIyLjMzOTdMMTcuMTE3OSAxNi45NDk5TTIuMzkyNDkgMTQuMzUxM0g3Ljc4MjE5TDEwLjE4ODMgMTEuMDc5TDEyLjMwNTcgMTcuMjM4N0wxNC41MTkzIDE0LjM1MTNIMjAuMjk0TTIyLjIgNS4xMTY2NEMyMS44Mzk3IDQuMDE2NDcgMjEuMDU5NyAzLjA1Mzc1IDE5Ljk0NTQgMi40ODkyNEMxOC44MzExIDEuOTI0NzQgMTcuNTkzNSAxLjg2NTMzIDE2LjQ5MzMgMi4yMjU2M00xNy4xOTcyIDQuMzc1MDVDMTcuNzQ3MyA0LjE5NDkgMTguMzY2MSA0LjIyNDYxIDE4LjkyMzMgNC41MDY4NkMxOS40ODA0IDQuNzg5MTEgMTkuODcwNCA1LjI3MDQ3IDIwLjA1MDYgNS44MjA1NiIvPjwvZz48L2c+PC9zdmc+",
                    "EN": "Heart Rate Sensor",
                    "TW": "心率感測器",
                    "CN": "心率传感器",
                    "BR": "Sensor de frequência cardíaca",
                    "CZ": "Snímač tepové frekvence",
                    "DA": "Hjerteslagssensor",
                    "DE": "Herzfrequenzsensor",
                    "ES": "Pulsómetro",
                    "FI": "Sykeanturi",
                    "FR": "Capteur de fréquence cardiaque",
                    "HU": "Pulzusmérő",
                    "IT": "Sensore frequenza cardiaca",
                    "JP": "心拍数センサー",
                    "KR": "심박수 센서",
                    "MS": "Heart Rate Sensor",
                    "NL": "Hartslagsensor",
                    "NO": "Hjertesensor",
                    "PL": "Czujnik tętna",
                    "RU": "Датчик пульса",
                    "SV": "Pulsmätare",
                    "TH": "เซนเซอร์วัดอัตราการเต้นของหัวใจ",
                    "TR": "Kalp Atışı Hızı Sensörü",
                    "UK": "Датчик серцевого ритму",
                    "RO": "Senzor de frecvență cardiacă",
                    "SL": "Senzor srčnega utripa"
                },
                {
                    "category": "transportation",
                    "ui-sort": 1,
                    "type": [
                        "118"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzQ1Ij48ZyBpZD0iR3JvdXAgNDQ4OSI+PHBhdGggaWQ9IlJlY3RhbmdsZSAxMDIyMF9fX19fMF8wX0dOV1BERk5MTkYiIHN0cm9rZT0iYmxhY2siIGQ9Ik03Ljg2MzQ1IDkuMjA5NDdIMy42NTQ0OUMyLjU3OTYyIDkuMjA5NDcgMS41ODY4OCA5Ljc4NDUyIDEuMDUyMTIgMTAuNzE2OUwxIDEwLjgwNzhNNy44NjM0NSA5LjIwOTQ3TDEwLjA2OTYgNy4xMzU2OUMxMC4xNjIzIDcuMDQ4NTIgMTAuMjg0OCA3IDEwLjQxMjEgN0gxNC44MjA5TTcuODYzNDUgOS4yMDk0N0g4LjQ3NDU4TTYuNzM1MjEgMTUuMzIwOEg4LjQ3NDU4TTE3LjgyOTUgMTUuMzIwOEgxNC44MjA5TTIyLjYyNDYgMTQuMTkyNVYxMC43NTQ5QzIyLjYyNDYgMTAuNjA5NCAyMi41NjEyIDEwLjQ3MTEgMjIuNDUxIDEwLjM3NjFMMTguNjc1NCA3LjEyMTNDMTguNTg0NiA3LjA0MzA0IDE4LjQ2ODggNyAxOC4zNDg5IDdIMTYuNjU0M00xNi42NTQzIDdMMTcuOTYgOC4zNjM2OEMxOC4yNjQ0IDguNjgxNjYgMTguMDM5IDkuMjA5NDcgMTcuNTk4OCA5LjIwOTQ3SDE1Ljk0OTJNMTYuNjU0MyA3SDE0LjgyMDlNMTQuODIwOSA3VjkuMjA5NDdNMTQuODIwOSA5LjIwOTQ3SDE1Ljk0OTJNMTQuODIwOSA5LjIwOTQ3SDguNDc0NThNMTUuOTQ5MiA5LjIwOTQ3VjEzLjYwNjhDMTUuOTQ5MiAxMy43MTI1IDE1LjkxNTcgMTMuODE1NSAxNS44NTM1IDEzLjkwMDlMMTQuODIwOSAxNS4zMjA4TTE0LjgyMDkgMTUuMzIwOEg4LjQ3NDU4TTguNDc0NTggOS4yMDk0N1YxNS4zMjA4TTEgMTAuODA3OEgxLjUwNjA0QzIuMTA4MjggMTAuODA3OCAyLjQ5NDEyIDExLjQ0ODYgMi4yMTIzNCAxMS45ODA5VjExLjk4MDlDMi4wNzM4NCAxMi4yNDI1IDEuODAyMDYgMTIuNDA2MSAxLjUwNjA0IDEyLjQwNjFIMU0xIDEwLjgwNzhWMTIuNDA2MU0xIDEyLjQwNjFWMTMuNjI4NEMxIDE0LjU2MzEgMS43NTc2OSAxNS4zMjA4IDIuNjkyMzYgMTUuMzIwOFYxNS4zMjA4Ii8+PGcgaWQ9Ikdyb3VwIDQ0OTAiPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTI1MV9fX19fMF8xX01WVU9EVUNLUkMiIGN4PSIyMC4yMjkzIiBjeT0iMTQuMjg2MyIgcj0iMi4zMjA1OSIgc3Ryb2tlPSJibGFjayIvPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTI1Ml9fX19fMF8yX09OSEpVSE9USVUiIGN4PSIyMC4yMjk1IiBjeT0iMTQuMjg2NCIgcj0iMC44MTYyNzgiIHN0cm9rZT0iYmxhY2siLz48L2c+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMjQ5X19fX18wXzNfUEpHVEVOWFpPVSIgY3g9IjQuNTcyNjIiIGN5PSIxNC4yODY1IiByPSIyLjMyMDU5IiBzdHJva2U9ImJsYWNrIi8+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMjUwX19fX18wXzRfUkJGU0pMR0hCWiIgY3g9IjQuNTcyODMiIGN5PSIxNC4yODY1IiByPSIwLjgxNjI3OCIgc3Ryb2tlPSJibGFjayIvPjxwYXRoIGlkPSJMaW5lIDU1X19fX18wXzVfRldTWE1TV1pQVyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBkPSJNMTMuNDE3MSAxMS4wODM1IDE0LjI1MDQgMTEuMDgzNSIvPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Sedan",
                    "TW": "轎車",
                    "CN": "轿车",
                    "BR": "Sedã",
                    "CZ": "Sedan",
                    "DA": "Sedan",
                    "DE": "Limousine",
                    "ES": "Berlina",
                    "FI": "Porrasperä",
                    "FR": "Berline",
                    "HU": "Luxuskocsi",
                    "IT": "Sedan",
                    "JP": "セダン",
                    "KR": "세단",
                    "MS": "Sedan",
                    "NL": "Sedan",
                    "NO": "Sedan",
                    "PL": "Sedan",
                    "RU": "Седан",
                    "SV": "Sedan",
                    "TH": "รถเก๋ง",
                    "TR": "Sedan",
                    "UK": "Седан",
                    "RO": "Sedan",
                    "SL": "Limuzina"
                },
                {
                    "category": "transportation",
                    "ui-sort": 2,
                    "type": [
                        "119"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzQ2Ij48ZyBpZD0iR3JvdXAgNDQ5MiI+PHBhdGggaWQ9IlJlY3RhbmdsZSAxMDIyMV9fX19fMF8wX1hXUk5QUUtJWkwiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgZD0iTTE5LjEzMzkgMTAuMzE4SDE3LjEwMDJNMjEuMDQwNSAxMC4zMThIMjIuOTQ3VjEyLjMwMDhNMjEuMDQwNSAxMC4zMThDMjAuNDIwOCAxMC4wMTMyIDE5LjQyMDkgOS42MTEwOSAxOC4yNjk1IDkuMjM4MDVNMjEuMDQwNSAxMC4zMThWMTAuMzE4QzIxLjI0NDEgMTEuMjU3NyAyMS45NDYzIDEyLjAxMDMgMjIuODY5NyAxMi4yNzg0TDIyLjk0NyAxMi4zMDA4TTIxLjA0MDUgMTAuMzE4TDIxLjgyODUgOC4zMjVNOS42MjYzNSAxMC4zMThINS4wNTA1NEw0LjI4NzkgMTAuNjk5M005LjYyNjM1IDEwLjMxOEMxMC41NDE1IDkuNTA0NDggMTIuMzIxIDguMTgyNTcgMTIuODI5NCA4LjE4MjU4QzEzLjYzNjIgOC4xODI2IDE0LjU0ODQgOC4zMDI2OSAxNS40NzMyIDguNDkxNTZNOS42MjYzNSAxMC4zMThIMTQuNzEwNk0xOC4yNjk1IDkuMjM4MDVMMTcuMTAwMiAxMC4zMThNMTguMjY5NSA5LjIzODA1QzE3LjM5MTcgOC45NTM2MiAxNi40MjU4IDguNjg2MTEgMTUuNDczMiA4LjQ5MTU2TTE3LjEwMDIgMTAuMzE4SDE0LjcxMDZNMTUuNDczMiA4LjQ5MTU2TDE0LjcxMDYgMTAuMzE4TTQuMjg3OSAxMC42OTkzTDMuMDkxMzkgMTEuMjk3NUMyLjQyMjUyIDExLjYzMiAyIDEyLjMxNTYgMiAxMy4wNjM0VjEzLjA2MzRNNC4yODc5IDEwLjY5OTNWMTAuNjk5M0M0LjEzMDc4IDExLjY4OTIgMy40NTM4MyAxMi41MTgzIDIuNTE1MzYgMTIuODcwMkwyIDEzLjA2MzRNMiAxMy4wNjM0VjEzLjg3NjlMMi41NTkyNyAxNC4yODM3SDUuNjYwNjVNOS4wMTYyNCAxNC4yODM3SDE3LjYwODZNMjAuODYyNSAxNC4yODM3SDIyLjk0N1YxMi4zMDA4TTIxLjgyODUgOC4zMjVIMjEuMDEyOVY4SDIzVjguMzI1SDIxLjgyODVaIi8+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMjUzX19fX18wXzFfV0lISUpKRkVRWCIgY3g9IjcuMjg3OTkiIGN5PSIxMy43NzUyIiByPSIxLjYzNTM4IiBzdHJva2U9ImJsYWNrIi8+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMjU2X19fX18wXzJfRElaWVhMWEhTQSIgY3g9IjcuMjk1NjYiIGN5PSIxMy43NzA3IiByPSIwLjUiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMC44MjYwODciLz48Y2lyY2xlIGlkPSJFbGxpcHNlIDEyNTRfX19fXzBfM19ZQ0FSWlFUQlFPIiBjeD0iMTkuMTg0NCIgY3k9IjEzLjc3NTIiIHI9IjEuNjM1MzgiIHN0cm9rZT0iYmxhY2siLz48Y2lyY2xlIGlkPSJFbGxpcHNlIDEyNTVfX19fXzBfNF9ZQ1RHVFNKVFdSIiBjeD0iMTkuMTg0NCIgY3k9IjEzLjc3NTIiIHI9IjAuNSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIwLjgzMDMyNCIvPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Sport Car",
                    "TW": "跑車",
                    "CN": "运动型汽车",
                    "BR": "Carro esportivo",
                    "CZ": "Sportovní vůz",
                    "DA": "Sportsbil",
                    "DE": "Sportwagen",
                    "ES": "Deportivo",
                    "FI": "Urheiluauto",
                    "FR": "Voiture de sport",
                    "HU": "Sportautó",
                    "IT": "Auto sportiva",
                    "JP": "スポーツカー",
                    "KR": "스포츠카",
                    "MS": "Sport Car",
                    "NL": "Sportauto",
                    "NO": "Sportsbil",
                    "PL": "Samochód sportowy",
                    "RU": "Спорткар",
                    "SV": "Sportbil",
                    "TH": "รถสปอร์ต",
                    "TR": "Spor Araba",
                    "UK": "Спортивний автомобіль",
                    "RO": "Mașină sport",
                    "SL": "Športni avto"
                },
                {
                    "category": "transportation",
                    "ui-sort": 3,
                    "type": [
                        "120"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzQ3IiBjbGlwLXBhdGg9InVybCgjY2xpcDBfMzE1XzQ3NTYpIj48ZyBpZD0iR3JvdXAgNDQ4NyI+PHBhdGggaWQ9IlJlY3RhbmdsZSAxMDIxNl9fX19fMF8wX0dUWkpYUFZEREoiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMjEuNDE3IDguNDkxOVY2QzIxLjQxNyA1LjQ0NzcyIDIwLjk2OTMgNSAyMC40MTcgNUg5LjU0ODk5QzkuMTI3MzEgNSA4Ljc1MDk0IDUuMjY0NTIgOC42MDgxMSA1LjY2MTI4TDcuMTcwMDYgOS42NTU4N0wyLjk0Njc1IDEwLjM4MjhDMi4wODA5MyAxMC41MzE5IDEuNDEzNzUgMTEuMjI4NSAxLjMwMjIxIDEyLjA5OTlMMS4yMjE3MiAxMi43Mjg3TTIxLjQxNyA4LjQ5MTlIMjJDMjIuNTUyMyA4LjQ5MTkgMjMgOC45Mzk2MiAyMyA5LjQ5MTlWMTIuODQ2MkMyMyAxMy4zOTg0IDIyLjU1MjMgMTMuODQ2MiAyMiAxMy44NDYySDIxLjQxN00yMS40MTcgOC40OTE5VjEzLjg0NjJNMjEuNDE3IDEzLjg0NjJWMTUuNDUzNEMyMS40MTcgMTYuMDA1NyAyMC45NjkzIDE2LjQ1MzQgMjAuNDE3IDE2LjQ1MzRIMTkuMjI4OE0yLjcwMDQyIDE2LjQ1MzRIMS44ODExMUMxLjI3ODQzIDE2LjQ1MzQgMC44MTI2ODcgMTUuOTI0MyAwLjg4OTIwNiAxNS4zMjY1TDAuOTE3NzggMTUuMTAzMk04Ljg0NjE3IDE2LjQ1MzRIMTMuMTc2MSIvPjxnIGlkPSJSZWN0YW5nbGUgMTAyMTdfX19fXzBfMV9UV1BZUENQUkdFIj48bWFzayBpZD0icGF0aC0yLWluc2lkZS0xXzMxNV80NzU2IiBmaWxsPSJ3aGl0ZSI+PHJlY3Qgd2lkdGg9IjQuODQyMSIgaGVpZ2h0PSIzLjcyNDciIHg9IjE1LjE3OCIgeT0iNi41ODI5NyIgcng9IjEiLz48L21hc2s+PHJlY3Qgd2lkdGg9IjQuODQyMSIgaGVpZ2h0PSIzLjcyNDciIHg9IjE1LjE3OCIgeT0iNi41ODI5NyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIyLjQiIG1hc2s9InVybCgjcGF0aC0yLWluc2lkZS0xXzMxNV80NzU2KSIgcng9IjEiLz48L2c+PHBhdGggaWQ9IlJlY3RhbmdsZSAxMDIxOF9fX19fMF8yX1hBQUNBSlNBTkwiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTEuMDI5MiA3LjE4Mjk3SDEyLjc4MTZDMTMuMDAyNSA3LjE4Mjk3IDEzLjE4MTYgNy4zNjIwNiAxMy4xODE2IDcuNTgyOTdWOS4zMDc2N0MxMy4xODE2IDkuNTI4NTggMTMuMDAyNSA5LjcwNzY3IDEyLjc4MTYgOS43MDc2N0gxMC4zODI1QzEwLjEwMzIgOS43MDc2NyA5LjkwOTg2IDkuNDI4NzIgMTAuMDA3OSA5LjE2NzIyTDEwLjY1NDcgNy40NDI1MkMxMC43MTMyIDcuMjg2NCAxMC44NjI1IDcuMTgyOTcgMTEuMDI5MiA3LjE4Mjk3WiIvPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTI0NV9fX19fMF8zX0NVUFNXTFNIS00iIGN4PSI1LjY4MDIxIiBjeT0iMTYuMTc0MSIgcj0iMi42NTkxMSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIxLjIiLz48Y2lyY2xlIGlkPSJFbGxpcHNlIDEyNDZfX19fXzBfNF9aVENPWEtLUUNUIiBjeD0iMTYuMjAyMiIgY3k9IjE2LjE3NDEiIHI9IjIuNjU5MTEiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIi8+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMjQ3X19fX18wXzVfSVZFU05EVkRNQSIgY3g9IjE2LjIwMjMiIGN5PSIxNi4xNzQxIiByPSIxLjE2OTIzIiBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuMiIvPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTI0OF9fX19fMF82X05MV05WTkdPS1MiIGN4PSI1LjY4MDI0IiBjeT0iMTYuMTc0MSIgcj0iMS4xNjkyMyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIxLjIiLz48cmVjdCBpZD0iUmVjdGFuZ2xlIDEwMjE5X19fX18wXzdfSUxCR0NLVEtCQiIgd2lkdGg9IjIuMDQ4NTgiIGhlaWdodD0iMi44ODY2NCIgeT0iMTIuMzU2MiIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIxLjIiIHJ4PSIxLjAyNDI5Ii8+PHBhdGggaWQ9IkxpbmUgNTRfX19fXzBfOF9UTVpNQ0dOR0dXIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTIuMSAxMS4xMDgzIDEyLjgxNjcgMTEuMTA4MyIvPjwvZz48L2c+PC9nPjxkZWZzPjxjbGlwUGF0aCBpZD0iY2xpcDBfMzE1XzQ3NTYiPjxwYXRoIGZpbGw9IndoaXRlIiBkPSJNMCAwSDI0VjI0SDB6Ii8+PC9jbGlwUGF0aD48L2RlZnM+PC9zdmc+",
                    "EN": "Sport Utility Vehicle",
                    "TW": "轎式休旅車",
                    "CN": "运动型多功能车",
                    "BR": "Veículo utilitário esportivo",
                    "CZ": "SUV",
                    "DA": "SUV",
                    "DE": "SUV",
                    "ES": "Todoterreno ligero",
                    "FI": "Katumaasturi",
                    "FR": "Véhicule utilitaire sport",
                    "HU": "Sport haszonjármű",
                    "IT": "Utilitaria sportiva",
                    "JP": "SUV",
                    "KR": "스포츠 유틸리티 카",
                    "MS": "Sport Utility Vehicle",
                    "NL": "SUV",
                    "NO": "SUV",
                    "PL": "Pojazd sportowo-użytkowy",
                    "RU": "Внедорожник",
                    "SV": "Stadsjeep",
                    "TH": "รถ SUV",
                    "TR": "Spor Kullanımlı Araç",
                    "UK": "SUV",
                    "RO": "SUV",
                    "SL": "Športno terensko vozilo"
                },
                {
                    "category": "transportation",
                    "ui-sort": 4,
                    "type": [
                        "121"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzQ4Ij48ZyBpZD0iR3JvdXAgNDQ4MCI+PHBhdGggaWQ9IlJlY3RhbmdsZSAxMDIxMF9fX19fMF8wX1BLWUlRWlRSSEMiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNNS43NzY4OSAxMC41NDk4TDguNzAxMjQgOC4wMjk4OEM5LjQ3MjI5IDcuMzY1NDcgMTAuNDU2MyA3IDExLjQ3NDEgN1Y3TTUuNzc2ODkgMTAuNTQ5OEgxMS40NzQxTTUuNzc2ODkgMTAuNTQ5OEgzLjM4ODQ1TTE0LjEwMzYgN1Y3QzE0Ljk5OTEgNyAxNS43MjUxIDcuNzI1OTggMTUuNzI1MSA4LjYyMTUxVjkuOTM2MjVNMTQuMTAzNiA3VjkuNTQ5NzlDMTQuMTAzNiAxMC4xMDIxIDEzLjY1NTkgMTAuNTQ5OCAxMy4xMDM2IDEwLjU0OThIMTEuNDc0MU0xNC4xMDM2IDdIMTEuNDc0MU0xMS40NzQxIDdWMTAuNTQ5OE0zLjM4ODQ1IDEwLjU0OThIMi42NjUzNEMxLjc0NTYgMTAuNTQ5OCAxIDExLjI5NTQgMSAxMi4yMTUxVjEyLjIxNTFNMy4zODg0NSAxMC41NDk4VjExLjIxNTFDMy4zODg0NSAxMS43Njc0IDIuOTQwNzMgMTIuMjE1MSAyLjM4ODQ1IDEyLjIxNTFIMU0xIDEyLjIxNTFWMTMuODgwNU0xNS43MjUxIDkuOTM2MjVWMTUuMTUxNE0xNS43MjUxIDkuOTM2MjVIMjEuMTE1NU0yMi40MzAzIDEzLjcwNTJWMTEuNDcwMU0xOS43OTE3IDEzLjcwNTJIMjIuMjc2OUMyMi42NzYzIDEzLjcwNTIgMjMgMTQuMDI4OSAyMyAxNC40MjgzVjE0LjQyODNDMjMgMTQuODI3NiAyMi42NzYzIDE1LjE1MTQgMjIuMjc2OSAxNS4xNTE0SDIwLjgwODhNMTcuMDM5OCAxNS4xNTE0SDE1LjcyNTFNMTUuNzI1MSAxNS4xNTE0VjE1LjE1MTRDMTUuNzI1MSAxNS41Mzg2IDE1LjQxMTIgMTUuODUyNiAxNS4wMjM5IDE1Ljg1MjZINi40MzQyNk0xIDEzLjg4MDVWMTMuODgwNUMxIDE0Ljk2OTYgMS44ODI5NCAxNS44NTI2IDIuOTcyMTEgMTUuODUyNkgzLjAxNTk0TTEgMTMuODgwNUgzLjQ3NU0yMS4xMTU1IDkuOTM2MjVIMjIuNDMwM1YxMS40NzAxTTIxLjExNTUgOS45MzYyNVYxMC40NzAxQzIxLjExNTUgMTEuMDIyNCAyMS41NjMzIDExLjQ3MDEgMjIuMTE1NSAxMS40NzAxSDIyLjQzMDMiLz48cGF0aCBpZD0iRWxsaXBzZSAxMjM5X19fX18wXzFfQVpPTk1KS0xPSyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xNy4yODk1IDE1LjE1MTRDMTcuMjg5NSAxNi4wMzAyIDE4LjAwMiAxNi43NDI2IDE4Ljg4MDggMTYuNzQyNkMxOS43NTk2IDE2Ljc0MjYgMjAuNDcyIDE2LjAzMDIgMjAuNDcyIDE1LjE1MTRDMjAuNDcyIDE0LjI3MjYgMTkuNzU5NiAxMy41NjAyIDE4Ljg4MDggMTMuNTYwMkMxOC4wMDIgMTMuNTYwMiAxNy4yODk1IDE0LjI3MjYgMTcuMjg5NSAxNS4xNTE0WiIvPjxwYXRoIGlkPSJFbGxpcHNlIDEyNDBfX19fXzBfMl9ITktKRlBYSk9ZIiBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTMuMDg5NzcgMTUuMTUxNEMzLjA4OTc3IDE2LjAzMDIgMy44MDIxOSAxNi43NDI2IDQuNjgxIDE2Ljc0MjZDNS41NTk4MiAxNi43NDI2IDYuMjcyMjQgMTYuMDMwMiA2LjI3MjI0IDE1LjE1MTRDNi4yNzIyNCAxNC4yNzI2IDUuNTU5ODIgMTMuNTYwMiA0LjY4MSAxMy41NjAyQzMuODAyMTkgMTMuNTYwMiAzLjA4OTc3IDE0LjI3MjYgMy4wODk3NyAxNS4xNTE0WiIvPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Pick-up",
                    "TW": "皮卡貨車",
                    "CN": "皮卡车",
                    "BR": "Coleta",
                    "CZ": "Dodávka",
                    "DA": "Pickup",
                    "DE": "Pick-up",
                    "ES": "Camioneta",
                    "FI": "Avolavapakettiauto",
                    "FR": "Pick-up",
                    "HU": "Kisteherautó",
                    "IT": "Pick-up",
                    "JP": "小型トラック",
                    "KR": "픽업 트럭",
                    "MS": "Pick-up",
                    "NL": "Pick-up",
                    "NO": "Pickup",
                    "PL": "Pickup",
                    "RU": "Пикап",
                    "SV": "Pick-up",
                    "TH": "รถกระบะ",
                    "TR": "Pikap",
                    "UK": "Пікап",
                    "RO": "Ridicare",
                    "SL": "Poltovornjak"
                },
                {
                    "category": "transportation",
                    "ui-sort": 5,
                    "type": [
                        "122"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzQ5Ij48ZyBpZD0iR3JvdXAgNDQ1NiI+PHBhdGggaWQ9IkVsbGlwc2UgMTE5OV9fX19fMF8wX01HVEFSSUNCQksiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik03LjU4OTU1IDEyLjU1ODVDOC40MDcyMiAxMi45MzU5IDguOTcxMiAxMy43MzUyIDguOTcxMiAxNC42NjAzQzguOTcxMiAxNS45NDkgNy44NzY3OCAxNi45OTM3IDYuNTI2NzQgMTYuOTkzN0M1LjQxNjYxIDE2Ljk5MzcgNC40NzkzMiAxNi4yODczIDQuMTgxMjUgMTUuMzE5N003LjU4OTU1IDEyLjU1ODVDNy4yNjgxOCAxMi40MTAxIDYuOTA3NjQgMTIuMzI3IDYuNTI2NzQgMTIuMzI3QzYuMTI0NyAxMi4zMjcgNS43NDUzMyAxMi40MTk2IDUuNDEwNzkgMTIuNTgzOE03LjU4OTU1IDEyLjU1ODVDNy41ODk1NSAxMS4yOTc3IDcuNTg5NTUgOC42MzQyNCA3LjU4OTU1IDguMDY2MTJDNy41ODk1NSA3LjM1NTk3IDcuNDMwMTMgNy4wMDA5IDYuNTI2NzQgNy4wMDA5QzYuMjI5NzkgNy4wMDA5IDUuODI5NSA3LjAwMDkgNS40MTA3OSA3LjAwMDlNMS4wMDAxMiA5Ljk5MzY1QzEuMDAwMTIgOS42ODkzIDEuNzk3MjMgOC4yNjkwMiAyLjA2MjkzIDcuNjA5NkMyLjMyODY0IDYuOTUwMTggMi45MTMxOCA3LjAwMDkgMy4zMzgzMSA3LjAwMDlDMy42MjM2OSA3LjAwMDkgNC41NTU2NiA3LjAwMDkgNS40MTA3OSA3LjAwMDlNMS4wMDAxMiA5Ljk5MzY1SDMuNDQ0NTlNMS4wMDAxMiA5Ljk5MzY1QzEuMDAwMTIgMTAuMjI1NSAxLjAwMDEyIDEyLjIyMzMgMS4wMDAxMiAxMy4zNDE1TTQuMTgxMjUgMTUuMzE5N0M0LjExNjg0IDE1LjExMDYgNC4wODIyNyAxNC44ODkzIDQuMDgyMjcgMTQuNjYwM0M0LjA4MjI3IDE0LjE3MDkgNC4yNDAxNCAxMy43MTY2IDQuNTA5OTIgMTMuMzQxNU00LjE4MTI1IDE1LjMxOTdDMy42MzQ1NyAxNS4zMTk3IDIuMzkyNDEgMTUuMzE5NyAxLjc5NzIzIDE1LjMxOTdDMS4wNTMyNiAxNS4zMTk3IDEuMDAwMTIgMTQuNDU3NCAxLjAwMDEyIDE0LjA1MTZDMS4wMDAxMiAxMy45NTQ5IDEuMDAwMTIgMTMuNjkxMiAxLjAwMDEyIDEzLjM0MTVNNS40MTA3OSA3LjAwMDlWMTIuNTgzOE01LjQxMDc5IDEyLjU4MzhDNS4wNDg1OSAxMi43NjE1IDQuNzM4OTQgMTMuMDIzIDQuNTA5OTIgMTMuMzQxNU0xLjAwMDEyIDEzLjM0MTVINC41MDk5MiIvPjxlbGxpcHNlIGlkPSJFbGxpcHNlIDEyMDBfX19fXzBfMV9RSlNaWlpXSlBRIiBjeD0iMS4wNjI4MSIgY3k9IjEuMDE0NDkiIGZpbGw9ImJsYWNrIiByeD0iMS4wNjI4MSIgcnk9IjEuMDE0NDkiIHRyYW5zZm9ybT0ibWF0cml4KC0xIDAgMCAxIDcuNTg5MDQgMTMuNzQ3MykiLz48cGF0aCBpZD0iUmVjdGFuZ2xlIDEwMTkxX19fX18wXzJfSE9LQ1ZISVpFSiIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik04LjU0NTc4IDEzLjEzODZIMTUuNTA3Mk04LjU0NTc4IDE1LjU3MzRIMTQuOTIyNk0xOC41ODkzIDEzLjEzODZIMjEuNzgyNkMyMi40NTUgMTMuMTM4NiAyMyAxMy42ODM2IDIzIDE0LjM1NlYxNC4zNTZDMjMgMTUuMDI4MyAyMi40NTUgMTUuNTczNCAyMS43ODI2IDE1LjU3MzRIMTkuMTczOSIvPjxlbGxpcHNlIGlkPSJFbGxpcHNlIDEyMDFfX19fXzBfM19aQVBJSVNYTFpZIiBjeD0iMi4zMzgxOCIgY3k9IjIuMjMxODgiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiByeD0iMi4zMzgxOCIgcnk9IjIuMjMxODgiIHRyYW5zZm9ybT0ibWF0cml4KC0xIDAgMCAxIDE5LjM4NiAxMi41Mjk5KSIvPjxlbGxpcHNlIGlkPSJFbGxpcHNlIDEyMDJfX19fXzBfNF9OVVlDSFBPV1JaIiBjeD0iMC45NTY1MjkiIGN5PSIwLjkxMzA0MyIgZmlsbD0iYmxhY2siIHJ4PSIwLjk1NjUyOSIgcnk9IjAuOTEzMDQzIiB0cmFuc2Zvcm09Im1hdHJpeCgtMSAwIDAgMSAxOC4wMDQ5IDEzLjg0ODgpIi8+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "Truck Cab",
                    "TW": "拖車頭",
                    "CN": "卡车驾驶室",
                    "BR": "Cabine de caminhão",
                    "CZ": "Tahač",
                    "DA": "Lastbilkabine",
                    "DE": "LKW-Fahrerhaus",
                    "ES": "Cabina de camión",
                    "FI": "Kuorma-auton ohjaamo",
                    "FR": "Cabine de camion",
                    "HU": "Teherautó kabin",
                    "IT": "Cabina motrice",
                    "JP": "トラック運転台",
                    "KR": "트럭 운전실",
                    "MS": "Truck Cab",
                    "NL": "Vrachtwagencabine",
                    "NO": "Førerhus til lastebil",
                    "PL": "Kabina ciężarówki",
                    "RU": "Кабина грузовика",
                    "SV": "Truckhytt",
                    "TH": "รถบรรทุก",
                    "TR": "Kamyon Kabini",
                    "UK": "Кабіна вантажівки",
                    "RO": "Cabină camion",
                    "SL": "Kabina tovornjaka"
                },
                {
                    "category": "transportation",
                    "ui-sort": 6,
                    "type": [
                        "123"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzUwIj48ZyBpZD0iR3JvdXAgNDQ1NCI+PHBhdGggaWQ9IlZlY3RvciAyODg0X19fX18wXzBfR0JBSEdYWFpZRyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE3LjY4NDkgNEMxOC4zMTI2IDQuMjMyODcgMTkuMzQwOSA0Ljg3MDMxIDE5Ljk0MDcgNS45NDExOE0xMC44NDQ1IDcuNjA1MDRDOS4wMzI3NyA3LjYwNTA0IDcuNzQ3OSA2LjM0MTc0IDcuMzMxOTMgNS43MTAwOEgyLjQ3ODk5QzIuNzcxNzEgNi4xMjYwNSAzLjYyNTIxIDcuMDg3MzkgNC42OTc0OCA3LjYwNTA0QzcuMjczNTUgNy44NzUxMSA4LjcwNTcxIDkuMjA4MzYgOS40NDk3NiAxMC42NTU1TTEwLjg0NDUgNy42MDUwNEMxMS4zNTI5IDYuNDk1OCAxMS45MDc2IDUuMzg2NTUgMTMuODAyNSA1LjM4NjU1QzE1LjY5NzUgNS4zODY1NSAxNy40MDc2IDYuNDk1OCAxNy40MDc2IDYuNDk1OE0xMC44NDQ1IDcuNjA1MDRDMTEuMDc1NiA4LjAyMTAxIDExLjk0NDUgOC44NTI5NCAxMy41NzE0IDguODUyOTRDMTQuNTg4MiA3LjI4MTUxIDE1LjkyODYgNi41ODgyNCAxNy40MDc2IDYuNDk1OE0xNy40MDc2IDYuNDk1OEMxOC41OTA4IDYuNDIxODUgMTkuMjY1OCA2LjExMDY0IDE5Ljk0MDcgNS45NDExOE0xOS45NDA3IDUuOTQxMThDMjAuMjUzNSA2LjQ5OTc1IDIwLjQ0OTggNy4xNzYyNiAyMC40MTE4IDcuOTc0NzlDMTguNjA5MiA4LjQzNjk3IDE1LjAwNDIgOS40NTM3OCAxNC4yMTg1IDEzLjc5ODNDMTMuMDM1MyAxMy43OTgzIDExLjAxNCAxMy43OTgzIDEwLjE1MTMgMTMuNzk4M0MxMC4xOTE4IDEyLjk4MTYgMTAuMDMgMTEuNzgzOSA5LjQ0OTc2IDEwLjY1NTVNOS40NDk3NiAxMC42NTU1TDQuODgyMzUgMTIuNDU4Ii8+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMTk0X19fX18wXzFfT1NKWERXV09aRiIgY3g9IjE5LjExNzYiIGN5PSIxMi40NTgiIHI9IjMuMzgyMzUiIHN0cm9rZT0iYmxhY2siLz48Y2lyY2xlIGlkPSJFbGxpcHNlIDExOTZfX19fXzBfMl9TSlBLWVBXTElMIiBjeD0iNC44ODIzNSIgY3k9IjEyLjQ1OCIgcj0iMy4zODIzNSIgc3Ryb2tlPSJibGFjayIvPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTE5NV9fX19fMF8zX1dBTUZGREdHWkciIGN4PSIxOS4xMTc2IiBjeT0iMTIuNDU4IiByPSIxLjE2Mzg3IiBzdHJva2U9ImJsYWNrIi8+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMTk3X19fX18wXzRfSkRDSEhUSU9SUCIgY3g9IjQuODgyMzUiIGN5PSIxMi40NTgiIHI9IjEuMTYzODciIHN0cm9rZT0iYmxhY2siLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzVfQ1JNVkNVTVBRRiIgZmlsbD0iYmxhY2siIGQ9Ik04IDE4SDEyVjE2TDE4IDE5SDE0VjIxTDggMThaIi8+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "Motorcycle",
                    "TW": "摩托車",
                    "CN": "摩托车",
                    "BR": "Motocicleta",
                    "CZ": "Motocykl",
                    "DA": "Motorcykel",
                    "DE": "Motorrad",
                    "ES": "Motocicleta",
                    "FI": "Moottoripyörä",
                    "FR": "Motocyclette",
                    "HU": "Motorkerékpár",
                    "IT": "Motocicletta",
                    "JP": "オートバイ",
                    "KR": "오토바이",
                    "MS": "Motorcycle",
                    "NL": "Motorfiets",
                    "NO": "Motorsykkel",
                    "PL": "Motocykl",
                    "RU": "Мотоцикл",
                    "SV": "Motorcykel",
                    "TH": "รถจักรยานยนต์",
                    "TR": "Motosiklet",
                    "UK": "Мотоцикл",
                    "RO": "Motocicletă",
                    "SL": "Motorno kolo"
                },
                {
                    "category": "transportation",
                    "ui-sort": 7,
                    "type": [
                        "124"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzUxIj48ZyBpZD0iR3JvdXAgNDQ1NSI+PHBhdGggaWQ9IlZlY3RvciAyODg1X19fX18wXzBfUktDV1VERFlPViIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE0LjA3NDMgMi4wNTA1MkMxNC43ODY3IDIuMDA4NjEgMTYuMjk5NCAyLjE1MTA5IDE2LjY1MTQgMy4wNTYyM0MxNi43ODE1IDMuMzkwNyAxNi44ODQxIDMuNzE0MTggMTcuMDMwNyAzLjk5OTA3TTE3LjAzMDcgMy45OTkwN0MxNy4zOCA0LjY3NzkgMTcuOTc5IDUuMTM3NjEgMTkuNzk0MyA1LjAwNDc5QzE5Ljc5NDMgMy4zNDUzNiAxOS43OTQzIDIuMzQzODUgMTkuNzk0MyAyLjA1MDUyQzE4Ljg3MzEgMS44ODI4OSAxNy4wMzA3IDIuMDM3OTMgMTcuMDMwNyAzLjk5OTA3Wk0xNy4wMzA3IDMuOTk5MDdDMTcuMTc2NyA0LjMxMzM2IDE3LjUwNjMgNS40MDcwOSAxNy42NTcxIDcuMjY3NjZDMTcuODQ1NyA5LjU5MzM4IDE4LjE2NDIgMTIuNDIxOSAxNy4wMzA3IDEzLjIzOTFDMTYuMTA5OSAxMy45MDI5IDE0LjgxNzIgMTQuMjc2MyAxMy4zMiAxNC4zOTMxTTExLjc0ODYgNy41ODE5NUMxMC45NDQ0IDcuNTgxOTUgOC44NzE0NiA3LjU4MTk1IDYuNDY4NTcgNy41ODE5NU0xMS43NDg2IDcuNTgxOTVMMTMuMzIgMTQuMzkzMU0xMS43NDg2IDcuNTgxOTVDMTEuNzQ4NiA3LjI4ODYxIDExLjc0ODYgNi4yODcwOSAxMS43NDg2IDUuNjMzMzhDMTEuNzQ4NiA0Ljk3OTY2IDExLjU2IDQuNzUzMzggMTAuODA1NyA0Ljc1MzM4QzEwLjA1MTQgNC43NTMzOCA4LjIyODU3IDQuNzUzMzggNy4yODU3MSA0Ljc1MzM4QzYuNTMxNDMgNC43NTMzOCA2LjQ2ODU3IDUuMTMwNTIgNi40Njg1NyA1LjYzMzM4QzYuNDY4NTcgNi4xMzYyMyA2LjQ2ODU3IDcuNTgxOTUgNi40Njg1NyA3LjU4MTk1TTYuNDY4NTcgNy41ODE5NUM2LjE3OTQ3IDcuNTgxOTUgNS44ODU1OSA3LjU4MTk1IDUuNTg4NTcgNy41ODE5NUMyLjEzMTQzIDcuNTgxOTUgMSAxMS42Njc3IDEgMTQuNDMzNEMxLjMyNjI5IDE0LjQzMzQgMS44MDQxNyAxNC40MzM0IDIuMzgyODYgMTQuNDMzNE0xMy4zMiAxNC4zOTMxQzEyLjk3NCAxNC40MjAxIDEyLjYxNzIgMTQuNDMzNCAxMi4yNTE0IDE0LjQzMzRDMTEuODU2NyAxNC40MzM0IDExLjI0IDE0LjQzMzQgMTAuNDkxNCAxNC40MzM0TTEwLjQ5MTQgMTQuNDMzNEMxMC41NzUyIDEzLjAyOTYgOS42ODY4NiAxMC4xNTkxIDYuNDY4NTcgMTAuMTU5MUMzLjI1MDI5IDEwLjE1OTEgMi40ODc2MiAxMy4wMjk2IDIuMzgyODYgMTQuNDMzNE0xMC40OTE0IDE0LjQzMzRDMTAuMTIxOSAxNC40MzM0IDkuNzIwMTkgMTQuNDMzNCA5LjI5NzE0IDE0LjQzMzRNMi4zODI4NiAxNC40MzM0QzIuNzQ1NTIgMTQuNDMzNCAzLjE0Nzc4IDE0LjQzMzQgMy41NzcxNCAxNC40MzM0TTMuNTc3MTQgMTQuNDMzNEMzLjU3NzE0IDE1LjIgMy43NjU3MSAxNy4yMTE0IDYuMjggMTcuMjExNEM4Ljc5NDI5IDE3LjIxMTQgOS4yOTcxNCAxNS4zODg2IDkuMjk3MTQgMTQuNDMzNE0zLjU3NzE0IDE0LjQzMzRDNS4zMTU5OSAxNC40MzM0IDcuNDk5MzEgMTQuNDMzNCA5LjI5NzE0IDE0LjQzMzQiLz48Y2lyY2xlIGlkPSJFbGxpcHNlIDExOThfX19fXzBfMV9YVFRUSUVUTEZTIiBjeD0iMjAuMDQ1NiIgY3k9IjE0LjEzMTQiIHI9IjIuOTU0MjkiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8yX05FVUxGR1VTSEwiIGZpbGw9ImJsYWNrIiBkPSJNNy42OTU2NSAxOS4yMTc0SDExLjUyMTdWMTcuMzA0M0wxNy4yNjA5IDIwLjE3MzlIMTMuNDM0OFYyMi4wODdMNy42OTU2NSAxOS4yMTc0WiIvPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Scooter",
                    "TW": "小型機車",
                    "CN": "踏板车",
                    "BR": "Scooter",
                    "CZ": "Skútr",
                    "DA": "Scooter",
                    "DE": "Scooter",
                    "ES": "Escúter",
                    "FI": "Skootteri",
                    "FR": "Scooter",
                    "HU": "Robogó",
                    "IT": "Scooter",
                    "JP": "スクーター",
                    "KR": "스쿠터",
                    "MS": "Scooter",
                    "NL": "Scooter",
                    "NO": "Skuter",
                    "PL": "Skuter",
                    "RU": "Скутер",
                    "SV": "Scooter",
                    "TH": "สกู๊ตเตอร์",
                    "TR": "Scooter",
                    "UK": "Скутер",
                    "RO": "Scuter",
                    "SL": "Skiro"
                },
                {
                    "category": "transportation",
                    "ui-sort": 8,
                    "type": [
                        "125"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzUyIj48ZyBpZD0iR3JvdXAgNDQ3OCI+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMjM1X19fX18wXzBfT1dZQ1FHQlRHViIgY3g9IjE5LjUiIGN5PSIxMi44MTI1IiByPSI0IiBzdHJva2U9ImJsYWNrIi8+PHBhdGggaWQ9IkVsbGlwc2UgMTIzN19fX19fMF8xX0FNTkNDVldTQ1kiIGZpbGw9ImJsYWNrIiBzdHJva2U9ImJsYWNrIiBkPSJNMjAuMTI1IDEyLjgxMjVDMjAuMTI1IDEzLjE1NzcgMTkuODQ1MiAxMy40Mzc1IDE5LjUgMTMuNDM3NUMxOS4xNTQ4IDEzLjQzNzUgMTguODc1IDEzLjE1NzcgMTguODc1IDEyLjgxMjVDMTguODc1IDEyLjQ2NzMgMTkuMTU0OCAxMi4xODc1IDE5LjUgMTIuMTg3NUMxOS44NDUyIDEyLjE4NzUgMjAuMTI1IDEyLjQ2NzMgMjAuMTI1IDEyLjgxMjVaIi8+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMjM4X19fX18wXzJfV1FCUkxJWFFBQSIgY3g9IjQuNSIgY3k9IjEyLjgxMjUiIHI9IjAuNjI1IiBmaWxsPSJibGFjayIgc3Ryb2tlPSJibGFjayIvPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTIzNl9fX19fMF8zX1ZUR01RSUdVQUUiIGN4PSI0LjUiIGN5PSIxMi44MTI1IiByPSI0IiBzdHJva2U9ImJsYWNrIi8+PHBhdGggaWQ9IlZlY3RvciAyODk0X19fX18wXzRfUVRNQlJPTklQSiIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE5Ljk2ODggNEgxNi44NzVMMTcuNjcyOSA2LjcxODc1TTE5LjQwNjIgMTIuNjI1TDE3LjY3MjkgNi43MTg3NU0xNy42NzI5IDYuNzE4NzVIOS4wOTM3NUw0Ljc4MTI1IDEyLjYyNUgxMC45Njg4TTE3LjY3MjkgNi43MTg3NUwxMC45Njg4IDEyLjYyNU0xMC45Njg4IDEyLjYyNUw4LjQzNzUgNC43NU03LjUgNC43NUgxMC45Njg4Ii8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF81X0FaVEVSUlVXUlYiIGZpbGw9ImJsYWNrIiBkPSJNNyAxOEgxMVYxNkwxNyAxOUgxM1YyMUw3IDE4WiIvPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Bike",
                    "TW": "自行車",
                    "CN": "自行车",
                    "BR": "Bicicleta",
                    "CZ": "Kolo",
                    "DA": "Cykel",
                    "DE": "Fahrrad",
                    "ES": "Bicicleta",
                    "FI": "Polkupyörä",
                    "FR": "Vélo",
                    "HU": "Kerékpár",
                    "IT": "Bici",
                    "JP": "バイク",
                    "KR": "자전거",
                    "MS": "Bike",
                    "NL": "Fiets",
                    "NO": "Sykkel",
                    "PL": "Rower",
                    "RU": "Велосипед",
                    "SV": "Cykel",
                    "TH": "จักรยาน",
                    "TR": "Bisiklet",
                    "UK": "Велосипед",
                    "RO": "Bicicletă",
                    "SL": "Kolo"
                },
                {
                    "category": "transportation",
                    "ui-sort": 9,
                    "type": [
                        "126"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzUzIj48cGF0aCBpZD0iVmVjdG9yIDI4ODNfX19fXzBfMF9FWFNZT0xORkdYIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNNy4wNTk3NSAxNC42NjA1QzUuMjkyMzMgMTMuNzQ0MSAxLjYwNTk4IDExLjM1MjMgMSA5LjExNjY5QzEuNTIyMTQgOS4xODExIDMuNTk1MzggOS40MjM4NSA2LjM0Njg0IDkuNzQ0MjRNMTkuMjY4NCAxMS4yNDM2QzE5LjI2ODQgMTAuOTg1NCAxOS4yNjg0IDEwLjM0MjYgMTkuMjY4NCA5LjgzNzg0QzE5Ljc1ODUgOS43NDQyNCAyMC43Mzg4IDkuMzc1ODEgMjIuNjk5MyA5LjM3NTgxQzIzLjMxMjEgMTAuMzI1MSAyMi44MTgxIDExLjE0NDkgMjIuNjk5MyAxMS42NDA3QzIyLjM2MDMgMTEuNjAxNSAyMi4wMDMgMTEuNTYwMiAyMS42Mjk5IDExLjUxN00xOS4yNjg0IDExLjI0MzZDMTcuNTk5MyAxMS4wNTA0IDE1Ljc2NTIgMTAuODM3OSAxMy45MjE1IDEwLjYyNDFNMTkuMjY4NCAxMS4yNDM2QzE5LjQ5NDMgMTEuMjY5OCAxOS43MTcyIDExLjI5NTYgMTkuOTM2NyAxMS4zMjFNNi4zNDY4NCA5Ljc0NDI0TDkuNjA4NyA3QzEwLjY3ODEgNy4zNTg5IDEzLjkxMyA5LjQ3MDUgMTMuOTIxNSAxMC42MjQxTTYuMzQ2ODQgOS43NDQyNEM4LjU4MDQ1IDEwLjAwNDMgMTEuMjYxIDEwLjMxNTYgMTMuOTIxNSAxMC42MjQxTTE5LjkzNjcgMTEuMzIxQzIwLjUyODQgMTEuMzg5NSAyMS4wOTUzIDExLjQ1NTEgMjEuNjI5OSAxMS41MTdNMTkuOTM2NyAxMS4zMjFMMTkuNTI4NSAxMi45NDc4TTE5LjEwOTYgMTQuNDgwMUwxOS41Mjg1IDEyLjk0NzhNMjEuNjI5OSAxMS41MTdMMjAuNTggMTUuMDIxTTE5LjUyODUgMTIuOTQ3OEw3LjA1OTc1IDExLjUxN000LjYwOTEyIDE2LjAwN0M0LjkzNTg3IDE2LjM1MjYgNS44Mjk5OCAxNy4wMzQ3IDYuNzkyNDEgMTYuOTk4NkM3Ljc1NDg0IDE2Ljk2MjYgOC42Nzg2NiAxNi4zMjI1IDkuMDIwMjYgMTYuMDA3QzkuMTgzNjQgMTYuMzM3NiA5LjgzMTIgMTYuOTk4NiAxMS4xMTQ0IDE2Ljk5ODZDMTIuMzk3NyAxNi45OTg2IDEzLjE2NDEgMTYuMzM3NiAxMy4zODY5IDE2LjAwN0MxMy41OTQ4IDE2LjMzNzYgMTQuMzMxNSAxNi45OTg2IDE1LjYxNDcgMTYuOTk4NkMxNi44OTc5IDE2Ljk5ODYgMTcuNTE1OCAxNi4zMzc2IDE3LjY2NDMgMTYuMDA3QzE3Ljg0MjYgMTYuMzM3NiAxOC41MTA5IDE2Ljk5ODYgMTkuNzU4NSAxNi45OTg2QzIxLjAwNjEgMTYuOTk4NiAyMS43NjM2IDE2LjMzNzYgMjEuOTg2NCAxNi4wMDciLz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Boat",
                    "TW": "船",
                    "CN": "船",
                    "BR": "Barco",
                    "CZ": "Loď",
                    "DA": "Båd",
                    "DE": "Boot",
                    "ES": "Barca",
                    "FI": "Vene",
                    "FR": "Bateau",
                    "HU": "Csónak",
                    "IT": "Barca",
                    "JP": "ボート",
                    "KR": "보트",
                    "MS": "Boat",
                    "NL": "Boot",
                    "NO": "Båt",
                    "PL": "Łódź",
                    "RU": "Лодка",
                    "SV": "Båt",
                    "TH": "เรือ",
                    "TR": "Bot",
                    "UK": "Човен",
                    "RO": "Barcă",
                    "SL": "Čoln"
                },
                {
                    "category": "transportation",
                    "ui-sort": 10,
                    "type": [
                        "127"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzU0Ij48ZyBpZD0iR3JvdXAgNDQ4MiI+PHBhdGggaWQ9IlJlY3RhbmdsZSAxMDIxMV9fX19fMF8wX0JFRUpDRUtHQVkiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTIgMTYuOTcwNlYxMi4wMDk4TTEyIDE2Ljk3MDZIMTUuMzE2Mk0xMiAxNi45NzA2SDcuNTc4NDNNMTguNjMyNCA4Ljc3NDVWOC43NzQ1QzE5LjY3MTUgOC43NzQ1IDIwLjM4NyA3LjczMTYzIDIwLjAxMyA2Ljc2MjE1TDE5LjcxMzIgNS45ODQ4N0MxOS40ODQzIDUuMzkxNDEgMTguOTEzOCA1IDE4LjI3NzcgNUgyQzEuNDQ3NzIgNSAxIDUuNDQ3NzEgMSA2VjE1Ljk3MDZDMSAxNi41MjI5IDEuNDQ3NzEgMTYuOTcwNiAyIDE2Ljk3MDZIMy4yMTA3OE0xOC42MzI0IDguNzc0NUwyMC4zMDM5IDEyLjAwOThNMTguNjMyNCA4Ljc3NDVIMTUuMzE2Mk0yMC4zMDM5IDEyLjAwOThMMjIuNDQ3MiAxMy4wODE0QzIyLjc4NiAxMy4yNTA4IDIzIDEzLjU5NzEgMjMgMTMuOTc1OVYxNS45NzA2QzIzIDE2LjUyMjkgMjIuNTUyMyAxNi45NzA2IDIyIDE2Ljk3MDZIMjEuMzgyNE0yMC4zMDM5IDEyLjAwOThIMTJNMTIgMTIuMDA5OFY4Ljc3NDVIMTUuMzE2Mk0xNS4zMTYyIDguNzc0NVYxNi45NzA2TTE1LjMxNjIgMTYuOTcwNkgxNy4wMTQ3Ii8+PHBhdGggaWQ9IkVsbGlwc2UgMTI0MV9fX19fMF8xX1VWWUhSQVdaV1YiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNNy4wODYyNiAxNi43NTQ5QzcuMDg2MjYgMTcuNjc0MyA2LjM0MDk0IDE4LjQxOTYgNS40MjE1NSAxOC40MTk2QzQuNTAyMTYgMTguNDE5NiAzLjc1Njg0IDE3LjY3NDMgMy43NTY4NCAxNi43NTQ5QzMuNzU2ODQgMTUuODM1NSA0LjUwMjE2IDE1LjA5MDIgNS40MjE1NSAxNS4wOTAyQzYuMzQwOTQgMTUuMDkwMiA3LjA4NjI2IDE1LjgzNTUgNy4wODYyNiAxNi43NTQ5WiIvPjxwYXRoIGlkPSJFbGxpcHNlIDEyNDJfX19fXzBfMl9aUFdaVVlDUktTIiBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTIwLjg5IDE2Ljc1NDlDMjAuODkgMTcuNjc0MyAyMC4xNDQ3IDE4LjQxOTYgMTkuMjI1MyAxOC40MTk2QzE4LjMwNTkgMTguNDE5NiAxNy41NjA2IDE3LjY3NDMgMTcuNTYwNiAxNi43NTQ5QzE3LjU2MDYgMTUuODM1NSAxOC4zMDU5IDE1LjA5MDIgMTkuMjI1MyAxNS4wOTAyQzIwLjE0NDcgMTUuMDkwMiAyMC44OSAxNS44MzU1IDIwLjg5IDE2Ljc1NDlaIi8+PGcgaWQ9IlJlY3RhbmdsZSAxMDIxMl9fX19fMF8zX0NISkxFWVdKUlgiPjxtYXNrIGlkPSJwYXRoLTQtaW5zaWRlLTFfMzE1XzQ4MjEiIGZpbGw9IndoaXRlIj48cmVjdCB3aWR0aD0iNi4yNTQ5IiBoZWlnaHQ9IjMuOTkwMTkiIHg9IjMuNDgwNTEiIHk9IjguNDUwOTgiIHJ4PSIxIi8+PC9tYXNrPjxyZWN0IHdpZHRoPSI2LjI1NDkiIGhlaWdodD0iMy45OTAxOSIgeD0iMy40ODA1MSIgeT0iOC40NTA5OCIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIyLjQiIG1hc2s9InVybCgjcGF0aC00LWluc2lkZS0xXzMxNV80ODIxKSIgcng9IjEiLz48L2c+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "Camper Trailer",
                    "TW": "露營拖車",
                    "CN": "野营拖车",
                    "BR": "Reboque para trailer",
                    "CZ": "Přívěs",
                    "DA": "Campingvogn",
                    "DE": "Wohnmobil/Wohnwagen",
                    "ES": "Minicaravana",
                    "FI": "Vedettävä asuntovaunu",
                    "FR": "Remorque de camping",
                    "HU": "Lakókocsi tréler",
                    "IT": "Rimorchio per camper",
                    "JP": "キャンパー トレーラー",
                    "KR": "캠핑 트레일러",
                    "MS": "Camper Trailer",
                    "NL": "Camperaanhangwagen",
                    "NO": "Campingvogn",
                    "PL": "Przyczepa kempingowa",
                    "RU": "Фургон для кемпинга",
                    "SV": "Husbil",
                    "TH": "รถบ้าน",
                    "TR": "Çekme Karavan",
                    "UK": "Фургон-кемпер",
                    "RO": "Rulotă",
                    "SL": "Bivalna prikolica"
                },
                {
                    "category": "transportation",
                    "ui-sort": 11,
                    "type": [
                        "133"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJUcmFuc3BvcnRhdGlvbi9FbGVjdHJpYyBjYXIiPjxnIGlkPSJHcm91cCA0NTc4Ij48ZyBpZD0iR3JvdXAgNDU3NiI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8wX01KRFFUV0xNUFQiIGZpbGw9ImJsYWNrIiBkPSJNNyAyMEgxMVYxOEwxNyAyMUgxM1YyM0w3IDIwWiIvPjxnIGlkPSJHcm91cCAzNTEyIj48cGF0aCBpZD0iVmVjdG9yIDIzN19fX19fMF8xX01CWUdURVNVUE0iIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjUiIGQ9Ik0zLjY2NjQzIDE1LjU4VjguNDkwMkw3LjE3NTIxIDNIMTYuNzAxNUwyMC4zMzMxIDguNDkwMlYxNS41OCIvPjxwYXRoIGlkPSJMaW5lIDM2X19fX18wXzJfUFBVT0dBTEVYSyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIxLjUiIGQ9Ik0zLjY2NjQzIDguOTE2NzggMjAuMzMzMSA4LjkxNjc4Ii8+PHBhdGggaWQ9IkxpbmUgMzdfX19fXzBfM19CQ1dMTkNTUUxTIiBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuNSIgZD0iTTMuNjY2NDMgMTUuNTgzMiAyMC4zMzMxIDE1LjU4MzIiLz48Y2lyY2xlIGlkPSJFbGxpcHNlIDI4Nl9fX19fMF80X0hWVVBEQkZIUlIiIGN4PSIyLjgzMzMzIiBjeT0iNy4xNjY1NSIgcj0iMC44MzMzMzMiIGZpbGw9ImJsYWNrIi8+PGNpcmNsZSBpZD0iRWxsaXBzZSAyODhfX19fXzBfNV9ISFhHSFBJUE5SIiBjeD0iNy4wMDAyMyIgY3k9IjEyLjE2NjciIHI9IjEuNjY2NjciIGZpbGw9ImJsYWNrIi8+PGNpcmNsZSBpZD0iRWxsaXBzZSAyODlfX19fXzBfNl9JR01QRFpJWkpGIiBjeD0iMTcuMDAwMiIgY3k9IjEyLjE2NjciIHI9IjEuNjY2NjciIGZpbGw9ImJsYWNrIi8+PGNpcmNsZSBpZD0iRWxsaXBzZSAyODdfX19fXzBfN19WRkFVSFBBTURCIiBjeD0iMjEuMTY2OSIgY3k9IjcuMTY2NTUiIHI9IjAuODMzMzMzIiBmaWxsPSJibGFjayIvPjwvZz48L2c+PHBhdGggaWQ9IlJlY3RhbmdsZSAxMDI3MV9fX19fMF84X01VUFpXVVpGVFYiIGZpbGw9ImJsYWNrIiBkPSJNMi45MTk5MiAxNUg1LjkxOTkyVjE3QzUuOTE5OTIgMTcuNTUyMyA1LjQ3MjIxIDE4IDQuOTE5OTIgMThIMy45MTk5MkMzLjM2NzY0IDE4IDIuOTE5OTIgMTcuNTUyMyAyLjkxOTkyIDE3VjE1WiIvPjxwYXRoIGlkPSJSZWN0YW5nbGUgMTAyNzJfX19fXzBfOV9RREFBU0xJUFdIIiBmaWxsPSJibGFjayIgZD0iTTE4LjA4MDEgMTVIMjEuMDgwMVYxN0MyMS4wODAxIDE3LjU1MjMgMjAuNjMyNCAxOCAyMC4wODAxIDE4SDE5LjA4MDFDMTguNTI3OCAxOCAxOC4wODAxIDE3LjU1MjMgMTguMDgwMSAxN1YxNVoiLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "Electric car",
                    "TW": "電動車",
                    "CN": "电动汽车",
                    "BR": "Carro elétrico",
                    "CZ": "Elektromobil",
                    "DA": "Elbil",
                    "DE": "Elektroauto",
                    "ES": "Coche eléctrico",
                    "FI": "Sähköauto",
                    "FR": "Voiture électrique",
                    "HU": "Elektromos autó",
                    "IT": "Auto elettrica",
                    "JP": "電気自動車",
                    "KR": "전기차",
                    "MS": "Electric car",
                    "NL": "Elektrische auto",
                    "NO": "Elbil",
                    "PL": "Samochód elektryczny",
                    "RU": "Электромобиль",
                    "SV": "Elbil",
                    "TH": "รถยนต์ไฟฟ้า",
                    "TR": "Elektrikli araba",
                    "UK": "Електромобіль",
                    "RO": "Mașină electrică",
                    "SL": "Električni avto"
                },
                {
                    "category": "transportation",
                    "ui-sort": 12,
                    "type": [
                        "134"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJUcmFuc3BvcnRhdGlvbi9NUFYiPjxnIGlkPSJHcm91cCA0NTgxIj48cGF0aCBpZD0iVmVjdG9yIDI5MjNfX19fXzBfMF9DQUhHRVdQQkpQIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNNS45MTI2MiAxMC4wOTcxTDYuNzY2OTkgMTAuNTI0M0g4LjA0ODU1TTUuOTEyNjIgMTAuMDk3MUwzLjU2MzExIDEwLjUyNDNNNS45MTI2MiAxMC4wOTcxTDcuMzAwOTcgOC44MzgyTTIyLjE0NTYgN0gyMS4zOTgxTTIxLjM5ODEgN0wyMi44OTMyIDkuOTkwMjlNMjEuMzk4MSA3SDE3LjY2MDJNMjAuNjUwNSAxNC41ODI1SDIyQzIyLjU1MjMgMTQuNTgyNSAyMyAxNC4xMzQ4IDIzIDEzLjU4MjVWMTIuMDE5NE0zLjc5NzUyIDE0LjU4MjVIMUwxIDEyLjU1MzRNMjIuODkzMiA5Ljk5MDI5TDIzIDEwLjg0NDdWMTIuMDE5NE0yMi44OTMyIDkuOTkwMjlMMjEuODI1MiAxMC41MjQzSDIxLjE4NDVNMy41NjMxMSAxMC41MjQzSDMuMDI5MTNDMS45MDg0NyAxMC41MjQzIDEgMTEuNDMyNyAxIDEyLjU1MzRWMTIuNTUzNE0zLjU2MzExIDEwLjUyNDNDMy42Njk5IDExLjA5MzkgMy4xNTQxNCAxMi4wOTc2IDIuMjgxNTUgMTIuNDQ2NkMyLjAzNjY4IDEyLjU0NDYgMSAxMi42NjAyIDEgMTIuNTUzNE03LjMwMDk3IDguODM4MlY4LjgzODJDOC43NTExOSA3LjY0OTU2IDEwLjU2ODQgNyAxMi40NDM1IDdIMTIuNjQwOE03LjMwMDk3IDguODM4MkM3LjY5MjU2IDkuMDA4NjQgOC4zOTAyOSA5LjU4NDQ3IDguMDQ4NTUgMTAuNTI0M004LjA0ODU1IDEwLjUyNDNIMTEuNjc5Nk04LjAyNzczIDE0LjU4MjVIMTEuNjc5Nk0xNi4zNjQ3IDE0LjU4MjVIMTEuNjc5Nk0xMi42NDA4IDdMMTEuNjc5NiAxMC41MjQzTTEyLjY0MDggN0gxNy42NjAyTTExLjY3OTYgMTAuNTI0M1YxNC41ODI1TTExLjY3OTYgMTAuNTI0M0gxNi43NTI0TTE3LjY2MDIgN0wxNi43NTI0IDEwLjUyNDNNMTYuNzUyNCAxMC41MjQzSDIxLjE4NDVNMjEuMTg0NSAxMC41MjQzQzIxLjA3NzcgMTEuMjAwNiAyMS42MTE3IDEyLjAxOTQgMjMgMTIuMDE5NE05LjU0MzY5IDEySDEwLjExNTZNMTMuMTM2OSAxMkgxMy42MDE5TTguMTU1MzQgMTMuODM1QzguMTU1MzQgMTUuMDczNiA3LjE1MTI0IDE2LjA3NzcgNS45MTI2MiAxNi4wNzc3QzQuNjc0IDE2LjA3NzcgMy42Njk5IDE1LjA3MzYgMy42Njk5IDEzLjgzNUMzLjY2OTkgMTIuNTk2MyA0LjY3NCAxMS41OTIyIDUuOTEyNjIgMTEuNTkyMkM3LjE1MTI0IDExLjU5MjIgOC4xNTUzNCAxMi41OTYzIDguMTU1MzQgMTMuODM1Wk0yMC43NTczIDEzLjk0MTdDMjAuNzU3MyAxNS4xODA0IDE5Ljc1MzIgMTYuMTg0NSAxOC41MTQ2IDE2LjE4NDVDMTcuMjc1OSAxNi4xODQ1IDE2LjI3MTggMTUuMTgwNCAxNi4yNzE4IDEzLjk0MTdDMTYuMjcxOCAxMi43MDMxIDE3LjI3NTkgMTEuNjk5IDE4LjUxNDYgMTEuNjk5QzE5Ljc1MzIgMTEuNjk5IDIwLjc1NzMgMTIuNzAzMSAyMC43NTczIDEzLjk0MTdaIi8+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMzIwX19fX18wXzFfWVJRTElOQ0FMRiIgY3g9IjUuOTEyOSIgY3k9IjEzLjgzNDkiIHI9IjAuNjQwNzc3IiBmaWxsPSJibGFjayIvPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTMyMV9fX19fMF8yX1hHR0JFSFJPSEQiIGN4PSIxOC41MTQ2IiBjeT0iMTMuODMwOCIgcj0iMC42NDA3NzciIGZpbGw9ImJsYWNrIi8+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "MPV",
                    "TW": "MPV",
                    "CN": "MPV",
                    "BR": "MPV",
                    "CZ": "MPV",
                    "DA": "MPV",
                    "DE": "MPV",
                    "ES": "MPV",
                    "FI": "MPV",
                    "FR": "MPV",
                    "HU": "MPV",
                    "IT": "MPV",
                    "JP": "MPV",
                    "KR": "MPV",
                    "MS": "MPV",
                    "NL": "MPV",
                    "NO": "MPV",
                    "PL": "MPV",
                    "RU": "MPV",
                    "SV": "MPV",
                    "TH": "MPV",
                    "TR": "MPV",
                    "UK": "MPV",
                    "RO": "MPV",
                    "SL": "MPV"
                },
                {
                    "category": "transportation",
                    "ui-sort": 13,
                    "type": [
                        "135"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJUcmFuc3BvcnRhdGlvbi9FViBDaGFyZ2luZyBTdGF0aW9uIj48ZyBpZD0iR3JvdXAgNDU3NyI+PHBhdGggaWQ9IlJlY3RhbmdsZSAxMDI3MF9fX19fMF8wX0pFRlVETk1IU0kiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS41IiBkPSJNMTMgMTMuNVY3QzEzIDUuMzQzMTUgMTEuNjU2OSA0IDEwIDRIN0M1LjM0MzE1IDQgNCA1LjM0MzE1IDQgN1YyMUgxM1YxMy41Wk0xMyAxMy41SDE0LjYzNjRDMTUuMzg5NSAxMy41IDE2IDE0LjExMDUgMTYgMTQuODYzNlYxOS4yNUMxNiAyMC4yMTY1IDE2Ljc4MzUgMjEgMTcuNzUgMjFWMjFDMTguNzE2NSAyMSAxOS41IDIwLjIxNjUgMTkuNSAxOS4yNVY5LjY5MzhDMTkuNSA5LjI0OTU3IDE5LjMyMzUgOC44MjM1MyAxOS4wMDk0IDguNTA5NDFMMTUuNSA1TTE5IDEwQzE5IDExLjEwNDYgMTguMTA0NiAxMiAxNyAxMkMxNS44OTU0IDEyIDE1IDExLjEwNDYgMTUgMTBDMTUgOC44OTU0MyAxNS44OTU0IDggMTcgOEMxOC4xMDQ2IDggMTkgOC44OTU0MyAxOSAxMFoiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzFfSlhJUlRKV01LWSIgZmlsbD0iYmxhY2siIGQ9Ik03LjUgMTQuNUg1LjVMOS41IDdWMTJIMTEuNUw3LjUgMTlWMTQuNVoiLz48L2c+PC9nPjwvZz48L3N2Zz4=",
                    "EN": "EV Charging Station",
                    "TW": "充電站",
                    "CN": "充电站",
                    "BR": "Estação de carregamento EV",
                    "CZ": "Dobíjecí stanice EV",
                    "DA": "EV-opladestation",
                    "DE": "EV-Ladestation",
                    "ES": "Estación de carga de vehículos eléctricos",
                    "FI": "Sähköauton latausasema",
                    "FR": "borne de recharge électrique",
                    "HU": "Elektromos jármű töltőállomás",
                    "IT": "Stazione di carica EV",
                    "JP": "充電ステーション",
                    "KR": "전기차 충전소",
                    "MS": "EV Charging Station",
                    "NL": "EV-laadstation",
                    "NO": "Ladestasjon for elbil",
                    "PL": "Stacja ładowania pojazdów elektrycznych",
                    "RU": "Станция зарядки электромобилей",
                    "SV": "Laddningsstation för elfordon",
                    "TH": "สถานีชาร์จรถไฟฟ้า",
                    "TR": "EV (elektrikli araç) şarj istasyonu",
                    "UK": "Станція зарядки електромобілів",
                    "RO": "Stație de încărcare EV",
                    "SL": "Polnilna postaja za električna vozila"
                },
                {
                    "category": "transportation",
                    "ui-sort": 14,
                    "type": [
                        "136"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJUcmFuc3BvcnRhdGlvbi9TbWFydCBHYXJhZ2UiPjxnIGlkPSJHcm91cCA0NTgwIj48ZyBpZD0iR3JvdXAgNDU3OSI+PGcgaWQ9Ikdyb3VwIDQ1NzYiPjxnIGlkPSJHcm91cCAzNTEyIj48cGF0aCBpZD0iVmVjdG9yIDIzN19fX19fMF8wX0JYRFZEVlNGU00iIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjUiIGQ9Ik00Ljk0ODc5IDIwLjI2VjE0LjI2MUw2LjkyMzI5IDExLjg4M00xNy41MDAyIDExLjg4M0wxOS4wNTE0IDE0LjI2MVYyMC4yNiIvPjxwYXRoIGlkPSJMaW5lIDM2X19fX18wXzFfR0xCU1ZERU1WSyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLXdpZHRoPSIxLjUiIGQ9Ik00Ljk0ODc5IDE0LjUwNjcgMTkuMDUxNCAxNC41MDY3Ii8+PHBhdGggaWQ9IkxpbmUgMzdfX19fXzBfMl9JWUxNUk5WWkFZIiBzdHJva2U9ImJsYWNrIiBzdHJva2Utd2lkdGg9IjEuNSIgZD0iTTQuOTQ4NzkgMTkuOTUgMTkuMDUxNCAxOS45NSIvPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMjg2X19fX18wXzNfVFFSQ1JCR1dUWiIgY3g9IjQuMjQzNDMiIGN5PSIxMy4xNDA5IiByPSIwLjcwNTEyOCIgZmlsbD0iYmxhY2siLz48Y2lyY2xlIGlkPSJFbGxpcHNlIDI4OF9fX19fMF80X1ZST1FYQ0pXVVMiIGN4PSI3Ljc2OTMxIiBjeT0iMTcuMzcxNyIgcj0iMS40MTAyNiIgZmlsbD0iYmxhY2siLz48Y2lyY2xlIGlkPSJFbGxpcHNlIDI4OV9fX19fMF81X0hPWVNKQUVOV00iIGN4PSIxNi4yMzExIiBjeT0iMTcuMzcxNyIgcj0iMS40MTAyNiIgZmlsbD0iYmxhY2siLz48Y2lyY2xlIGlkPSJFbGxpcHNlIDI4N19fX19fMF82X0RHUk1XWlZXT0YiIGN4PSIxOS43NTY1IiBjeT0iMTMuMTQwOSIgcj0iMC43MDUxMjgiIGZpbGw9ImJsYWNrIi8+PC9nPjwvZz48cGF0aCBpZD0iUmVjdGFuZ2xlIDEwMjcxX19fX18wXzdfQlZCUlZDRlhWWSIgZmlsbD0iYmxhY2siIGQ9Ik00LjIwMDExIDE5LjU5OThINi43Mzg1N1YyMS4xMzgzQzYuNzM4NTcgMjEuNjkwNiA2LjI5MDg1IDIyLjEzODMgNS43Mzg1NyAyMi4xMzgzSDUuMjAwMTFDNC42NDc4MiAyMi4xMzgzIDQuMjAwMTEgMjEuNjkwNiA0LjIwMDExIDIxLjEzODNWMTkuNTk5OFoiLz48cGF0aCBpZD0iUmVjdGFuZ2xlIDEwMjcyX19fX18wXzhfUFROU09ITk9DQiIgZmlsbD0iYmxhY2siIGQ9Ik0xNy4yNjAxIDE5LjU5OThIMTkuNzk4NlYyMS4xMzgzQzE5Ljc5ODYgMjEuNjkwNiAxOS4zNTA5IDIyLjEzODMgMTguNzk4NiAyMi4xMzgzSDE4LjI2MDFDMTcuNzA3OCAyMi4xMzgzIDE3LjI2MDEgMjEuNjkwNiAxNy4yNjAxIDIxLjEzODNWMTkuNTk5OFoiLz48L2c+PHBhdGggaWQ9IlZlY3RvciAyOTIyX19fX18wXzlfQU1WUkRTSlhMSyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTEuODg1NzYgMjEuODg0N1YxMS42NjI1TTIyLjExNDIgMjEuODg0N1YxMS42NjI1TTEuODg1NzYgOC40OTk5OFY1LjQ5OTk4SDcuMzQ2MTVNMS44ODU3NiA4LjQ5OTk4SDIyLjExNDJNMS44ODU3NiA4LjQ5OTk4VjExLjY2MjVNMjIuMTE0MiA4LjQ5OTk4VjUuNDk5OThIMTcuMzA1NE0yMi4xMTQyIDguNDk5OThWMTEuNjYyNU0xLjg4NTc2IDExLjY2MjVIMjIuMTE0MiIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfMTBfSUJSTElWRldBUyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE1Ljc5NTEgNC40ODdDMTQuODc2NCAzLjU2ODI1IDEzLjYwNzEgMyAxMi4yMDUyIDNDMTAuODAzMiAzIDkuNTMzOTkgMy41NjgyNSA4LjYxNTI1IDQuNDg3TTEwLjQxMDIgNi4yODE5NkMxMC44Njk2IDUuODIyNTkgMTEuNTA0MiA1LjUzODQ2IDEyLjIwNTIgNS41Mzg0NkMxMi45MDYyIDUuNTM4NDYgMTMuNTQwOCA1LjgyMjU5IDE0LjAwMDEgNi4yODE5NiIvPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Smart Garage",
                    "TW": "智慧車庫",
                    "CN": "智能车库",
                    "BR": "Garagem inteligente",
                    "CZ": "Chytrá garáž",
                    "DA": "Smart garage",
                    "DE": "Smarte Garage",
                    "ES": "Garaje inteligente",
                    "FI": "Älykäs autotalli",
                    "FR": "Garage intelligent",
                    "HU": "Intelligens garázs",
                    "IT": "Smart Garage",
                    "JP": "スマート ガレージ",
                    "KR": "스마트 차고",
                    "MS": "Smart Garage",
                    "NL": "Slimme garage",
                    "NO": "Smartgarasje",
                    "PL": "Inteligentny garaż",
                    "RU": "Умный гараж",
                    "SV": "Smart garage",
                    "TH": "โรงรถอัจฉริยะ",
                    "TR": "Akıllı Garaj",
                    "UK": "Розумний гараж",
                    "RO": "Garaj inteligent",
                    "SL": "Pametna garaža"
                },
                {
                    "category": "others",
                    "ui-sort": 1,
                    "type": [
                        "66"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzM2Ij48ZyBpZD0iR3JvdXAgNDUxOSI+PHBhdGggaWQ9IlJlY3RhbmdsZSAxMDI0NF9fX19fMF8wX1BCVVJJRVRJQ0YiIHN0cm9rZT0iYmxhY2siIGQ9Ik0xMS4yNSAxOEgxM0MxNC4zODA3IDE4IDE1LjUgMTYuODgwNyAxNS41IDE1LjVWMTQuNzVWOS43NUMxNS41IDguMzY5MjkgMTQuMzgwNyA3LjI1IDEzIDcuMjVIMTEuMjVDOS44NjkyOSA3LjI1IDguNzUgOC4zNjkyOSA4Ljc1IDkuNzVWMTQuNzVWMTUuNUM4Ljc1IDE2Ljg4MDcgOS44NjkyOSAxOCAxMS4yNSAxOFpNMTMgMTdIMTEuMjVDMTAuNDIxNiAxNyA5Ljc1IDE2LjMyODQgOS43NSAxNS41VjE1LjI1SDE0LjVWMTUuNUMxNC41IDE2LjMyODQgMTMuODI4NCAxNyAxMyAxN1pNOS43NSAxNC4yNVY5Ljc1QzkuNzUgOC45MjE1NyAxMC40MjE2IDguMjUgMTEuMjUgOC4yNUgxM0MxMy44Mjg0IDguMjUgMTQuNSA4LjkyMTU3IDE0LjUgOS43NVYxNC4yNUg5Ljc1Wk0yIDQuNUgyMkMyMi4yNzYxIDQuNSAyMi41IDQuNzIzODYgMjIuNSA1VjE5LjVDMjIuNSAxOS43NzYxIDIyLjI3NjEgMjAgMjIgMjBIMkMxLjcyMzg2IDIwIDEuNSAxOS43NzYxIDEuNSAxOS41VjVDMS41IDQuNzIzODYgMS43MjM4NiA0LjUgMiA0LjVaIi8+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMjgwX19fX18wXzFfSk9NRFlVUFhSUCIgY3g9IjMuODMzMTgiIGN5PSI3LjUwMDAyIiByPSIxIiBmaWxsPSJibGFjayIvPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTI4MV9fX19fMF8yX0tBUERKUUtaSFgiIGN4PSIxOS45OTk3IiBjeT0iNy41IiByPSIxIiBmaWxsPSJibGFjayIvPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTI4Ml9fX19fMF8zX1BYQkJORFNaR0giIGN4PSIxOS45OTk3IiBjeT0iMTgiIHI9IjEiIGZpbGw9ImJsYWNrIi8+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMjgzX19fX18wXzRfVk5JS0NVR0VXUSIgY3g9IjMuODMzMTgiIGN5PSIxOC4wMDAxIiByPSIxIiBmaWxsPSJibGFjayIvPjxjaXJjbGUgaWQ9IkVsbGlwc2UgMTI4NF9fX19fMF81X1BPUE1ZQU1DWUMiIGN4PSIxMi4xODM0IiBjeT0iMTAuMzMzNCIgcj0iMC45MTY2NjciIGZpbGw9ImJsYWNrIi8+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "Wall Switch",
                    "TW": "牆壁開關",
                    "CN": "墙壁开关",
                    "BR": "Interruptor de parede",
                    "CZ": "Nástěnný vypínač",
                    "DA": "Stikkontakt",
                    "DE": "Wandschalter",
                    "ES": "Conmutador de pared",
                    "FI": "Seinäkatkaisin",
                    "FR": "Interrupteur mural",
                    "HU": "Villanykapcsoló",
                    "IT": "Interruttore a parete",
                    "JP": "壁スイッチ",
                    "KR": "벽 스위치",
                    "MS": "Suis dinding",
                    "NL": "Wandschakelaar",
                    "NO": "Veggbryter",
                    "PL": "Wyłącznik ścienny",
                    "RU": "Настенный переключатель",
                    "SV": "Väggbrytare",
                    "TH": "สวิตซ์ไฟติดผนัง",
                    "TR": "Duvar anahtarı",
                    "UK": "Настінний вимикач",
                    "RO": "Întrerupător",
                    "SL": "Stensko stikalo"
                },
                {
                    "category": "others",
                    "ui-sort": 2,
                    "type": [
                        "67"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzM3Ij48ZyBpZD0iR3JvdXAgNDU0MCI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8wX1dMWUlIRU9GWEgiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0yLjI3MDA1IDguNTk5MjdDMS40ODUzNSA5LjM4Mzk4IDEgMTAuNDY4IDEgMTEuNjY1NUMxIDEyLjg2MjkgMS40ODUzNSAxMy45NDY5IDIuMjcwMDUgMTQuNzMxNk0zLjgwMzE0IDEzLjE5ODVDMy40MTA3OSAxMi44MDYyIDMuMTY4MTIgMTIuMjY0MiAzLjE2ODEyIDExLjY2NTVDMy4xNjgxMiAxMS4wNjY3IDMuNDEwNzkgMTAuNTI0NyAzLjgwMzE0IDEwLjEzMjQiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzFfTFBUVkhRQ1VQRSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTIxLjcyOTkgOC41OTkyN0MyMi41MTQ3IDkuMzgzOTggMjMgMTAuNDY4IDIzIDExLjY2NTVDMjMgMTIuODYyOSAyMi41MTQ3IDEzLjk0NjkgMjEuNzI5OSAxNC43MzE2TTIwLjE5NjkgMTMuMTk4NUMyMC41ODkyIDEyLjgwNjIgMjAuODMxOSAxMi4yNjQyIDIwLjgzMTkgMTEuNjY1NUMyMC44MzE5IDExLjA2NjcgMjAuNTg5MiAxMC41MjQ3IDIwLjE5NjkgMTAuMTMyNCIvPjxnIGlkPSJHcm91cCA0NTE3Ij48cGF0aCBpZD0iVmVjdG9yIDI5MDhfX19fXzBfMl9BSlBDQlNIU1dIIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTMuNDU3NSA1LjQ0MDQ4VjUuMjI2ODZDMTMuNDU3NSA0LjU5MzUgMTIuODc2MiA0LjExOTY4IDEyLjI1NTkgNC4yNDc0TDcuMjU5MjMgNS4yNzYxMkM2Ljc5NDQyIDUuMzcxODEgNi40NjA4OSA1Ljc4MTAxIDYuNDYwODkgNi4yNTU1N1YxNy4wMTc3QzYuNDYwODkgMTcuNDcxNSA2Ljc2NjM3IDE3Ljg2ODMgNy4yMDUgMTcuOTg0NEwxMi4yMDE2IDE5LjMwNzFDMTIuODM2IDE5LjQ3NSAxMy40NTc1IDE4Ljk5NjYgMTMuNDU3NSAxOC4zNDA0VjUuNDQwNDhaTTEzLjQ1NzUgNS40NDA0OEgxNi4xNjE2QzE2LjcxMzkgNS40NDA0OCAxNy4xNjE2IDUuODg4MiAxNy4xNjE2IDYuNDQwNDhWMTguODE2NE05Ljk1OTIgMTIuMDI1NUgxMS44MTEyIi8+PC9nPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Smart Door Lock",
                    "TW": "門窗感應器",
                    "CN": "窗户感应器",
                    "BR": "Sensor de janela",
                    "CZ": "Okenní snímač",
                    "DA": "Vinduessensor",
                    "DE": "Fenstersensor",
                    "ES": "Sensor de ventana",
                    "FI": "Ikkuna-anturi",
                    "FR": "Capteur de fenêtre",
                    "HU": "Ablakérzékelő",
                    "IT": "Sensore finestra",
                    "JP": "窓センサー",
                    "KR": "창문 센서",
                    "MS": "Sensor tingkap",
                    "NL": "Raamsensor",
                    "NO": "Vindussensor",
                    "PL": "Czujnik okienny",
                    "RU": "Датчик окна",
                    "SV": "Fönstersensor",
                    "TH": "เซนเซอร์ตรวจจับหน้าต่าง",
                    "TR": "Pencere sensörü",
                    "UK": "Віконний датчик",
                    "RO": "Senzor geam",
                    "SL": "Okensko tipalo"
                },
                {
                    "category": "others",
                    "ui-sort": 3,
                    "type": [
                        "68"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzM4Ij48ZyBpZD0iR3JvdXAgNDUxNiI+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8wX0NNWVFLT0JLWUoiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS13aWR0aD0iMS41IiBkPSJNMjAuNDgzMyAxMy43NTg0QzIwLjQ4MzMgOS40NjkyIDE1Ljc5OTcgNC42MjEzNSAxMy40OTM5IDIuNDk0NjJDMTIuNzc4OSAxLjgzNTEzIDExLjcwNDQgMS44MzUxMyAxMC45ODk0IDIuNDk0NjJDOC42ODM2MyA0LjYyMTM1IDQgOS40NjkyIDQgMTMuNzU4NEM0IDE5LjQ0OCA4LjM0MDE2IDIyIDEyLjI0MTYgMjJDMTYuMTQzMSAyMiAyMC40ODMzIDE5LjQ0OCAyMC40ODMzIDEzLjc1ODRaIi8+PGcgaWQ9Ikdyb3VwIDQ1MTQiPjxwYXRoIGlkPSJFbGxpcHNlIDEyNzdfX19fXzBfMV9DUVRLQlVZS1FDIiBzdHJva2U9ImJsYWNrIiBkPSJNMTEuMTQwNiAxOS44NjY3QzEyLjMyMjkgMTkuODY2NyAxMy4yODEyIDE4LjkwODMgMTMuMjgxMiAxNy43MjZDMTMuMjgxMiAxNi45MzM3IDEyLjg1MDggMTYuMjQxOSAxMi4yMTA5IDE1Ljg3MThWNy4yMzY5OEMxMi4yMTA5IDYuNjQ1ODYgMTEuNzMxNyA2LjE2NjY3IDExLjE0MDYgNi4xNjY2N0MxMC41NDk1IDYuMTY2NjcgMTAuMDcwMyA2LjY0NTg2IDEwLjA3MDMgNy4yMzY5OFYxNS44NzE4QzkuNDMwNDggMTYuMjQxOSA5IDE2LjkzMzcgOSAxNy43MjZDOSAxOC45MDgzIDkuOTU4MzkgMTkuODY2NyAxMS4xNDA2IDE5Ljg2NjdaIi8+PHBhdGggaWQ9IkxpbmUgNTdfX19fXzBfMl9JTkNOSUNRSVdVIiBzdHJva2U9ImJsYWNrIiBkPSJNMTQuMTM4MSA5LjA5MTk4IDE2LjcwNjkgOS4wOTE5OCIvPjxwYXRoIGlkPSJMaW5lIDU4X19fX18wXzNfVVlIS1ZDUEVPVyIgc3Ryb2tlPSJibGFjayIgZD0iTTE0LjEzODEgMTEuMjMyNSAxNi43MDY5IDExLjIzMjUiLz48cGF0aCBpZD0iTGluZSA1OV9fX19fMF80X0pEUFZWWFREWkIiIHN0cm9rZT0iYmxhY2siIGQ9Ik0xNC4xMzgxIDEzLjM3MyAxNi43MDY5IDEzLjM3MyIvPjxwYXRoIGlkPSJMaW5lIDYwX19fX18wXzVfSkJYVFFWT0JUTyIgc3Ryb2tlPSJibGFjayIgZD0iTTE0LjEzODEgMTUuNTEzNSAxNi43MDY5IDE1LjUxMzUiLz48cGF0aCBpZD0iVW5pb25fX19fXzBfNl9JTUZFTVFMQktUIiBmaWxsPSJibGFjayIgZmlsbC1ydWxlPSJldmVub2RkIiBkPSJNMTEuNDIyMiA5LjQ2OTE1SDEwLjg3MTdWMTYuNjc1NkMxMC40MjUxIDE2LjgxNTkgMTAuMTAxMiAxNy4yMzMyIDEwLjEwMTIgMTcuNzI2MUMxMC4xMDEyIDE4LjMzNDEgMTAuNTk0MSAxOC44MjcgMTEuMjAyMSAxOC44MjdDMTEuODEwMSAxOC44MjcgMTIuMzAzIDE4LjMzNDEgMTIuMzAzIDE3LjcyNjFDMTIuMzAzIDE3LjE5MzUgMTEuOTI0NyAxNi43NDkyIDExLjQyMjIgMTYuNjQ3MlY5LjQ2OTE1WiIgY2xpcC1ydWxlPSJldmVub2RkIi8+PC9nPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Temperature Humidity Sensor",
                    "TW": "溫溼度感應器",
                    "CN": "温湿度传感器",
                    "BR": "Sensor de temperatura e umidade",
                    "CZ": "Snímač teploty a vlhkosti",
                    "DA": "Temperatur- og fugtighedssensor",
                    "DE": "Temperatur- und Feuchtigkeitssensor",
                    "ES": "Sensor de temperatura y humedad",
                    "FI": "Lämpötila- ja kosteusanturi",
                    "FR": "Capteur de température et d'humidité",
                    "HU": "Hőmérséklet- és páratartalom-érzékelő",
                    "IT": "Sensore per temperatura e umidità",
                    "JP": "温湿度センサー",
                    "KR": "온도 및 습도 센서",
                    "MS": "Sensor suhu & kelembapan",
                    "NL": "Temperatuur- en luchtvochtigheidssensor",
                    "NO": "Temperatur- og luftfuktighetssensor",
                    "PL": "Czujnik temperatury i wilgotności",
                    "RU": "Датчик температуры и влажности",
                    "SV": "Temperatur- och fuktsensor",
                    "TH": "เซนเซอร์วัดอุณหภูมิและความชื้น",
                    "TR": "Sıcaklık ve nem sensörü",
                    "UK": "Датчик температури й вологості",
                    "RO": "Senzor temperatură și umiditate",
                    "SL": "Tipalo za temperaturo in vlažnost"
                },
                {
                    "category": "others",
                    "ui-sort": 4,
                    "type": [
                        "69"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzM5IiBjbGlwLXBhdGg9InVybCgjY2xpcDBfMzE1XzUwMTkpIj48ZyBpZD0iNjYwMjQ2OV9kZXNpZ25fZ3JhcGhpY19oYW5kX21vdmVfdG9vbF9pY29uIDEiIGNsaXAtcGF0aD0idXJsKCNjbGlwMV8zMTVfNTAxOSkiPjxnIGlkPSJHcm91cCA0NTEwIj48cGF0aCBpZD0iVmVjdG9yX19fX18wXzBfRU1CVlZRQVdaUyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2UtbWl0ZXJsaW1pdD0iMTAiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTEuNDM1OSAxNC41NzhMMTAuMDIxMyA3LjIxMjYzQzkuODY1NzQgNi40MDI0MyA5LjA3NTU0IDUuODY2ODYgOC4yNjUzNSA2LjAyMjQ2QzcuNDU1MTUgNi4xNzgwNiA2LjkxOTU4IDYuOTY4MjUgNy4wNzUxOCA3Ljc3ODQ0TDguOTE0MDkgMTcuMzUzNUw1Ljk2ODkgMTMuOTQ3OEM1LjM4NTUzIDEzLjI5NjIgNC40NTkyNCAxMy4yNDQ5IDMuODIxNzIgMTMuOTAyQzMuMzc2ODcgMTQuMzY5MyAzLjIwOTU2IDE1LjA4ODcgMy42MzE0OSAxNS42OTVDNS40MTgyMSAxOC42MzU4IDcuNzY1ODYgMjEuMzE2MSAxMC40NzA2IDIzLjQ2OTciLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzFfUExCV1RLRklUVSIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2UtbWl0ZXJsaW1pdD0iMTAiIHN0cm9rZS13aWR0aD0iMS4yIiBkPSJNMTEuNDM1OCAxNC41NzhMOS43MzgzOSA1LjczOTU1QzkuNTgyNzkgNC45MjkzNiAxMC4xMTg0IDQuMTM5MTcgMTAuOTI4NiAzLjk4MzU3QzExLjczODggMy44Mjc5NyAxMi41Mjg5IDQuMzYzNTQgMTIuNjg0NSA1LjE3Mzc0TDE0LjM4MiAxNC4wMTIyIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8yX1FWVEZCQUVYQUEiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLW1pdGVybGltaXQ9IjEwIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE0LjM4MiAxNC4wMTIyTDEyLjk2NzUgNi42NDY4MkMxMi44MTE5IDUuODM2NjIgMTMuMzQ3NCA1LjA0NjQzIDE0LjE1NzYgNC44OTA4M0MxNC45Njc4IDQuNzM1MjMgMTUuNzU4IDUuMjcwOCAxNS45MTM2IDYuMDgxTDE3LjMyODIgMTMuNDQ2NCIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfM19aVUNPTFZYSkFGIiBzdHJva2U9ImJsYWNrIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiIHN0cm9rZS1taXRlcmxpbWl0PSIxMCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xNy4zMjgyIDEzLjQ0NjRMMTYuMTk2NSA3LjU1NDA4QzE2LjA0MDkgNi43NDM4OCAxNi41NzY1IDUuOTUzNjkgMTcuMzg2NyA1Ljc5ODA5QzE4LjE5NjkgNS42NDI0OSAxOC45ODcxIDYuMTc4MDYgMTkuMTQyNyA2Ljk4ODI2TDIwLjI3NDMgMTIuODgwNkMyMC44NDAxIDE1LjgyNjcgMjAuODAyNiAxOC44MTI0IDIwLjEzMzQgMjEuNjkwMyIvPjwvZz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzRfQ0NJTlhQTExZRyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTUuOTI4MiAxLjk0NjA5QzQuNTAzMjcgMi4yMDE5NiAzLjE3NTk2IDMuMDAxNDMgMi4yODQgNC4yODM4M0MxLjM5MjA0IDUuNTY2MjMgMS4xMDQzMiA3LjA4ODc3IDEuMzYwMTkgOC41MTM2OU00LjE0NDA5IDguMDEzNzlDNC4wMTYxNiA3LjMwMTMzIDQuMTYwMDIgNi41NDAwNiA0LjYwNiA1Ljg5ODg2QzUuMDUxOTggNS4yNTc2NiA1LjcxNTYzIDQuODU3OTMgNi40MjgxIDQuNzI5OTkiLz48L2c+PC9nPjwvZz48ZGVmcz48Y2xpcFBhdGggaWQ9ImNsaXAwXzMxNV81MDE5Ij48cGF0aCBmaWxsPSJ3aGl0ZSIgZD0iTTAgMEgyNFYyNEgweiIvPjwvY2xpcFBhdGg+PGNsaXBQYXRoIGlkPSJjbGlwMV8zMTVfNTAxOSI+PHBhdGggZmlsbD0id2hpdGUiIGQ9Ik0wIDBIMjRWMjRIMHoiLz48L2NsaXBQYXRoPjwvZGVmcz48L3N2Zz4=",
                    "EN": "Body Sensor",
                    "TW": "人體感應器",
                    "CN": "人体传感器",
                    "BR": "Sensor corporal",
                    "CZ": "Tělní snímač",
                    "DA": "Kropssensor",
                    "DE": "Körpersensor",
                    "ES": "Sensor corporal",
                    "FI": "Kehoanturi",
                    "FR": "Capteur corporel",
                    "HU": "Testérzékelő",
                    "IT": "Sensore indossabile",
                    "JP": "ボディセンサー",
                    "KR": "바디 센서",
                    "MS": "Sensor badan",
                    "NL": "Lichaamssensor",
                    "NO": "Kroppssensor",
                    "PL": "Czujnik na ciele",
                    "RU": "Датчик движения",
                    "SV": "Kroppssensor",
                    "TH": "เซนเซอร์ตรวจจับการเคลื่อนไหวของร่างกาย",
                    "TR": "Vücut sensörü",
                    "UK": "Натільний датчик",
                    "RO": "Senzor corp",
                    "SL": "Telesno tipalo"
                },
                {
                    "category": "others",
                    "ui-sort": 5,
                    "type": [
                        "70"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzQwIj48ZyBpZD0iR3JvdXAgNDUyMSI+PHBhdGggaWQ9IlJlY3RhbmdsZSAxMDI0OV9fX19fMF8wX05WVUtKU1BDWFkiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik05LjQ1NzE0IDEwLjQyODZDOS40NTcxNCAxMC4wOTcyIDkuMTg4NTEgOS44Mjg1NyA4Ljg1NzE0IDkuODI4NTdDOC41MjU3NyA5LjgyODU3IDguMjU3MTQgMTAuMDk3MiA4LjI1NzE0IDEwLjQyODZWMTMuNTcxNEM4LjI1NzE0IDEzLjkwMjggOC41MjU3NyAxNC4xNzE0IDguODU3MTQgMTQuMTcxNEM5LjE4ODUxIDE0LjE3MTQgOS40NTcxNCAxMy45MDI4IDkuNDU3MTQgMTMuNTcxNFYxMC40Mjg2Wk0xNS4yMTkgMTAuNDI4NkMxNS4yMTkgMTAuMDk3MiAxNC45NTA0IDkuODI4NTcgMTQuNjE5IDkuODI4NTdDMTQuMjg3NyA5LjgyODU3IDE0LjAxOSAxMC4wOTcyIDE0LjAxOSAxMC40Mjg2VjEzLjU3MTRDMTQuMDE5IDEzLjkwMjggMTQuMjg3NyAxNC4xNzE0IDE0LjYxOSAxNC4xNzE0QzE0Ljk1MDQgMTQuMTcxNCAxNS4yMTkgMTMuOTAyOCAxNS4yMTkgMTMuNTcxNFYxMC40Mjg2Wk0xOS4yNTcxIDEyQzE5LjI1NzEgMTYuMDA4IDE2LjAwOCAxOS4yNTcxIDEyIDE5LjI1NzFDNy45OTE5OSAxOS4yNTcxIDQuNzQyODYgMTYuMDA4IDQuNzQyODYgMTJDNC43NDI4NiA3Ljk5MTk5IDcuOTkxOTkgNC43NDI4NiAxMiA0Ljc0Mjg2QzE2LjAwOCA0Ljc0Mjg2IDE5LjI1NzEgNy45OTE5OSAxOS4yNTcxIDEyWk0xMiAyMC40NTcxQzE2LjY3MDggMjAuNDU3MSAyMC40NTcxIDE2LjY3MDggMjAuNDU3MSAxMkMyMC40NTcxIDcuMzI5MjUgMTYuNjcwOCAzLjU0Mjg2IDEyIDMuNTQyODZDNy4zMjkyNSAzLjU0Mjg2IDMuNTQyODYgNy4zMjkyNSAzLjU0Mjg2IDEyQzMuNTQyODYgMTYuNjcwOCA3LjMyOTI1IDIwLjQ1NzEgMTIgMjAuNDU3MVpNNSAxLjZIMTlDMjAuODc3OCAxLjYgMjIuNCAzLjEyMjIzIDIyLjQgNVYxOUMyMi40IDIwLjg3NzggMjAuODc3OCAyMi40IDE5IDIyLjRINUMzLjEyMjIzIDIyLjQgMS42IDIwLjg3NzggMS42IDE5VjVDMS42IDMuMTIyMjMgMy4xMjIyMyAxLjYgNSAxLjZaTTE5Ljg1NzEgMTJDMTkuODU3MSAxNi4zMzk0IDE2LjMzOTQgMTkuODU3MSAxMiAxOS44NTcxQzcuNjYwNjIgMTkuODU3MSA0LjE0Mjg2IDE2LjMzOTQgNC4xNDI4NiAxMkM0LjE0Mjg2IDcuNjYwNjIgNy42NjA2MiA0LjE0Mjg2IDEyIDQuMTQyODZDMTYuMzM5NCA0LjE0Mjg2IDE5Ljg1NzEgNy42NjA2MiAxOS44NTcxIDEyWiIvPjxwYXRoIGlkPSJFbGxpcHNlIDEyODZfX19fXzBfMV9XVktORURMSUhGIiBmaWxsPSJibGFjayIgZD0iTTEzLjA0NzYgMTUuNjY2N0MxMy4wNDc2IDE2LjI0NTIgMTIuNTc4NiAxNi43MTQzIDEyIDE2LjcxNDNDMTEuNDIxNCAxNi43MTQzIDEwLjk1MjQgMTYuMjQ1MiAxMC45NTI0IDE1LjY2NjdDMTAuOTUyNCAxNS4wODgxIDExLjQyMTQgMTQuNjE5IDEyIDE0LjYxOUMxMi41Nzg2IDE0LjYxOSAxMy4wNDc2IDE1LjA4ODEgMTMuMDQ3NiAxNS42NjY3WiIvPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Smart Plug",
                    "TW": "智能插座",
                    "CN": "智能插头",
                    "BR": "Tomada inteligente",
                    "CZ": "Chytrá zásuvka",
                    "DA": "Smart-stik",
                    "DE": "Intelligenter Stecker",
                    "ES": "Enchufe inteligente",
                    "FI": "Älypistorasia",
                    "FR": "Prise intelligente",
                    "HU": "Intelligens konnektor",
                    "IT": "Presa smart",
                    "JP": "スマートプラグ",
                    "KR": "스마트 플러그",
                    "MS": "Palam pintar",
                    "NL": "Slimme stekker",
                    "NO": "Smartplugg",
                    "PL": "Sterownik smartplug",
                    "RU": "Смарт-розетка",
                    "SV": "Smart kontakt",
                    "TH": "ปลั๊กไฟอัจฉริยะ",
                    "TR": "Akıllı priz",
                    "UK": "Розумна розетка",
                    "RO": "Priză inteligentă",
                    "SL": "Pametni vtič"
                },
                {
                    "category": "others",
                    "ui-sort": 6,
                    "type": [
                        "71"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzQxIiBjbGlwLXBhdGg9InVybCgjY2xpcDBfMzE1XzUwMjcpIj48cGF0aCBpZD0iUmVjdGFuZ2xlIDEwMjUxX19fX18wXzBfWFZWRFFLSFBGSyIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTguODY5NTcgMTkuNDc4M0gxNS4xMzA0TTIxLjY1MjIgMTUuNTY1MlYxMC4yNjA5QzIxLjY1MjIgOS4xNTYzIDIwLjc1NjcgOC4yNjA4NyAxOS42NTIyIDguMjYwODdIMTIuMjYwOU0yMS42NTIyIDE1LjU2NTJWMjAuODY5NkMyMS42NTIyIDIxLjk3NDEgMjAuNzU2NyAyMi44Njk2IDE5LjY1MjIgMjIuODY5Nkg0Ljg2OTU3QzMuNzY1IDIyLjg2OTYgMi44Njk1NyAyMS45NzQxIDIuODY5NTcgMjAuODY5NlYxNS41NjUyTTIxLjY1MjIgMTUuNTY1MkgyNE0yLjg2OTU3IDE1LjU2NTJWMTAuMjYwOUMyLjg2OTU3IDkuMTU2MyAzLjc2NSA4LjI2MDg3IDQuODY5NTcgOC4yNjA4N0gxMi4yNjA5TTIuODY5NTcgMTUuNTY1MkgwTTEyLjI2MDkgOC4yNjA4N1Y2LjE3MzkxTTEyLjI2MDkgNi4xNzM5MUMxMy40MTM1IDYuMTczOTEgMTQuMzQ3OCA1LjIzOTU1IDE0LjM0NzggNC4wODY5NkMxNC4zNDc4IDIuOTM0MzYgMTMuNDEzNSAyIDEyLjI2MDkgMkMxMS4xMDgzIDIgMTAuMTczOSAyLjkzNDM2IDEwLjE3MzkgNC4wODY5NkMxMC4xNzM5IDUuMjM5NTUgMTEuMTA4MyA2LjE3MzkxIDEyLjI2MDkgNi4xNzM5MVpNMTAuNjk1NyAxNEMxMC42OTU3IDE1LjQ0MDcgOS41Mjc3IDE2LjYwODcgOC4wODY5NiAxNi42MDg3QzYuNjQ2MjEgMTYuNjA4NyA1LjQ3ODI2IDE1LjQ0MDcgNS40NzgyNiAxNEM1LjQ3ODI2IDEyLjU1OTMgNi42NDYyMSAxMS4zOTEzIDguMDg2OTYgMTEuMzkxM0M5LjUyNzcgMTEuMzkxMyAxMC42OTU3IDEyLjU1OTMgMTAuNjk1NyAxNFpNMTguNTIxNyAxNEMxOC41MjE3IDE1LjQ0MDcgMTcuMzUzOCAxNi42MDg3IDE1LjkxMyAxNi42MDg3QzE0LjQ3MjMgMTYuNjA4NyAxMy4zMDQzIDE1LjQ0MDcgMTMuMzA0MyAxNEMxMy4zMDQzIDEyLjU1OTMgMTQuNDcyMyAxMS4zOTEzIDE1LjkxMyAxMS4zOTEzQzE3LjM1MzggMTEuMzkxMyAxOC41MjE3IDEyLjU1OTMgMTguNTIxNyAxNFoiLz48L2c+PC9nPjxkZWZzPjxjbGlwUGF0aCBpZD0iY2xpcDBfMzE1XzUwMjciPjxwYXRoIGZpbGw9IndoaXRlIiBkPSJNMCAwSDI0VjI0SDB6Ii8+PC9jbGlwUGF0aD48L2RlZnM+PC9zdmc+",
                    "EN": "Robot",
                    "TW": "機器人",
                    "CN": "机器人",
                    "BR": "Robô",
                    "CZ": "Robot",
                    "DA": "Robot",
                    "DE": "Roboter",
                    "ES": "Robot",
                    "FI": "Robotti",
                    "FR": "Robot",
                    "HU": "Robot",
                    "IT": "Robot",
                    "JP": "ロボット",
                    "KR": "로봇",
                    "MS": "Robot",
                    "NL": "Robot",
                    "NO": "Robot",
                    "PL": "Robot",
                    "RU": "Робот",
                    "SV": "Robot",
                    "TH": "หุ่นยนต์",
                    "TR": "Robot",
                    "UK": "Робот",
                    "RO": "Robot",
                    "SL": "Robot"
                },
                {
                    "category": "others",
                    "ui-sort": 7,
                    "type": [
                        "115"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzQyIj48ZyBpZD0iR3JvdXAgNDUxMiI+PHBhdGggaWQ9IlZlY3RvciAyOTA1X19fX18wXzBfU0dRSlpWUUFBWCIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTEyIDEuNzMzMzNIOS45NDI2NEM5LjE0Njk5IDEuNzMzMzMgOC4zODM5MyAyLjA0OTQgNy44MjEzMiAyLjYxMjAxTDYuNzQxNzIgMy42OTE2MUM2LjM0NDE0IDQuMDg5MTkgNS44NDI4NCA0LjM2NzA0IDUuMjk0OTggNC40OTM0N0wzLjIgNC45NzY5Mk0xMiAxLjczMzMzSDkuNjExMjdDOS4wMjc3OSAxLjczMzMzIDguNDY4MjIgMS45NjUxMiA4LjA1NTY0IDIuMzc3N0w2LjU3OTQ4IDMuODUzODVDNi4yODc5MyA0LjE0NTQxIDUuOTIwMyA0LjM0OTE2IDUuNTE4NTQgNC40NDE4OEwzLjIgNC45NzY5Mk0xMiAxLjczMzMzSDE0LjM4ODdDMTQuOTcyMiAxLjczMzMzIDE1LjUzMTggMS45NjUxMiAxNS45NDQ0IDIuMzc3N0wxNy40MjA1IDMuODUzODVDMTcuNzEyMSA0LjE0NTQxIDE4LjA3OTcgNC4zNDkxNiAxOC40ODE1IDQuNDQxODhMMjAuOCA0Ljk3NjkyTTEyIDEwLjUzMzNIMTAuMTA1MUM5LjIxNDc5IDEwLjUzMzMgOC4zNzA0NCAxMC4xMzc5IDcuODAwNDYgOS40NTM4OUw3Ljc2NjIxIDkuNDEyNzhDNy4xOTYyMiA4LjcyODggNi4zNTE4OCA4LjMzMzMzIDUuNDYxNTQgOC4zMzMzM0gzLjJNMTIgMTAuNTMzM0g5LjczMDQyQzkuMDc3NTEgMTAuNTMzMyA4LjQ1ODMzIDEwLjI0MzMgOC4wNDAzNCA5Ljc0MTc0TDcuNTI2MzMgOS4xMjQ5M0M3LjEwODM0IDguNjIzMzQgNi40ODkxNiA4LjMzMzMzIDUuODM2MjQgOC4zMzMzM0gzLjJNMTIgMTAuNTMzM0gxNC4yNjk2QzE0LjkyMjUgMTAuNTMzMyAxNS41NDE3IDEwLjI0MzMgMTUuOTU5NyA5Ljc0MTc0TDE2LjQ3MzcgOS4xMjQ5M0MxNi44OTE3IDguNjIzMzQgMTcuNTEwOCA4LjMzMzMzIDE4LjE2MzggOC4zMzMzM0gyMC44TTMuMiA0Ljk3NjkyTDIuNjMzMDIgNS4xMDc3N0MxLjg5MTc1IDUuMjc4ODMgMS4zNjY2NyA1LjkzODg5IDEuMzY2NjcgNi42OTk2M1Y2LjY5OTYzQzEuMzY2NjcgNy42MDE5IDIuMDk4MSA4LjMzMzMzIDMuMDAwMzcgOC4zMzMzM0gzLjJNMy4yIDQuOTc2OTJWMU0zLjIgNC45NzY5MkwyLjYzMzAyIDUuMTA3NzdDMS44OTE3NSA1LjI3ODgzIDEuMzY2NjcgNS45Mzg4OSAxLjM2NjY3IDYuNjk5NjNDMS4zNjY2NyA3LjYwMTkgMi4wOTgxIDguMzMzMzMgMy4wMDAzNyA4LjMzMzMzSDMuMk0zLjIgMUgxSDUuNEgzLjJaTTMuMiA4LjMzMzMzVjE2Ljg1MDlDMy4yIDE3LjI4MDggMy4yNDk4OCAxNy43MjgzIDMuNTI2OCAxOC4wNTcxQzMuNzU2MzQgMTguMzI5NyA0LjEyMTQxIDE4LjYgNC42NjY2NyAxOC42QzQuMDI4OTYgMTguNiAzLjYzNzcyIDE4LjIzMDMgMy40MjE1IDE3LjkxOTZDMy4yNDE0MyAxNy42NjA4IDMuMiAxNy4zMzkzIDMuMiAxNy4wMjRWOC4zMzMzM1pNMjAuOCA0Ljk3NjkyTDIxLjM2NyA1LjEwNzc3QzIyLjEwODIgNS4yNzg4MyAyMi42MzMzIDUuOTM4ODkgMjIuNjMzMyA2LjY5OTYzQzIyLjYzMzMgNy42MDE5IDIxLjkwMTkgOC4zMzMzMyAyMC45OTk2IDguMzMzMzNIMjAuOE0yMC44IDQuOTc2OTJWMU0yMC44IDFIMjNNMjAuOCAxSDE4LjZNMjAuOCA4LjMzMzMzVjE3LjAyNEMyMC44IDE3LjMzOTMgMjAuNzU4NiAxNy42NjA4IDIwLjU3ODUgMTcuOTE5NkMyMC4zNjIzIDE4LjIzMDMgMTkuOTcxIDE4LjYgMTkuMzMzMyAxOC42Ii8+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMjc1X19fX18wXzFfVExSRVZYTFJPTyIgY3g9IjEyLjAwMDIiIGN5PSI2LjEzMzMzIiByPSIxLjYiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIi8+PGNpcmNsZSBpZD0iRWxsaXBzZSAxMjc2X19fX18wXzJfT0NJU0xOUUZUWCIgY3g9IjEyLjAwMDEiIGN5PSIxMS4yNjY3IiByPSIwLjczMzMzMyIgZmlsbD0iYmxhY2siLz48cmVjdCBpZD0iUmVjdGFuZ2xlIDEwMjQxX19fX18wXzNfVk5FQlZFS0ZFSCIgd2lkdGg9IjcuNiIgaGVpZ2h0PSI3LjYiIHg9IjguMjAwMzciIHk9IjE0LjgiIHN0cm9rZT0iYmxhY2siIHN0cm9rZS13aWR0aD0iMS4yIiByeD0iMS40Ii8+PHBhdGggaWQ9IlJlY3RhbmdsZSAxMDI0Ml9fX19fMF80X0VKR0hFWVpOWlAiIGZpbGw9ImJsYWNrIiBkPSJNMTAuNTMzNSAxNC4ySDEzLjQ2NjhWMTYuMTMzM0MxMy40NjY4IDE2LjY4NTYgMTMuMDE5MSAxNy4xMzMzIDEyLjQ2NjggMTcuMTMzM0gxMS41MzM1QzEwLjk4MTIgMTcuMTMzMyAxMC41MzM1IDE2LjY4NTYgMTAuNTMzNSAxNi4xMzMzVjE0LjJaIi8+PC9nPjwvZz48L2c+PC9zdmc+",
                    "EN": "Drone",
                    "TW": "無人機",
                    "CN": "无人机",
                    "BR": "Drone",
                    "CZ": "Dron",
                    "DA": "Drone",
                    "DE": "Drohne",
                    "ES": "Dron",
                    "FI": "Drooni",
                    "FR": "Drone",
                    "HU": "Drón",
                    "IT": "Drone",
                    "JP": "ドローン",
                    "KR": "드론",
                    "MS": "Drone",
                    "NL": "Drone",
                    "NO": "Drone",
                    "PL": "Dron",
                    "RU": "Дрон",
                    "SV": "Drönare",
                    "TH": "โดรน",
                    "TR": "Drone",
                    "UK": "Дрон",
                    "RO": "Dronă",
                    "SL": "Dron"
                },
                {
                    "category": "others",
                    "ui-sort": 8,
                    "type": [
                        "116"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzQzIj48ZyBpZD0iR3JvdXAgNDQ5NyI+PHBhdGggaWQ9IlZlY3RvciAyODk5X19fX18wXzBfVkNVS0hTS1dLRCIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTExLjgzNDggMy4xNjM3NkMxMC43OTYyIDMuMTYzNzYgMTAuNzUyOSA0LjAyOTI2IDEwLjE5MDMgNC4wMjkyNlY0LjU0ODU2TTExLjgzNDggMy4xNjM3NkMxMi44NzM0IDMuMTYzNzYgMTMuMDQ2NSA0LjAyOTI2IDEzLjkxMiA0LjAyOTI2QzE0Ljc3NzUgNC4wMjkyNiAxNC42OTA5IDMuMTYzNzYgMTUuNjQzIDMuMTYzNzZNMTEuODM0OCAzLjE2Mzc2VjFNMTUuNjQzIDMuMTYzNzZDMTYuNTk1IDMuMTYzNzYgMTYuODExNCA0LjAyOTI2IDE3LjYzMzYgNC4wMjkyNkMxOC40NTU4IDQuMDI5MjYgMTguMjgyNyAzLjE2Mzc2IDE5LjQ1MTEgMy4xNjM3Nk0xNS42NDMgMy4xNjM3NlYxTTE5LjQ1MTEgMy4xNjM3NkMyMC42MTk2IDMuMTYzNzYgMjAuMjMwMSA0LjAyOTI2IDIxLjIyNTQgNC4wMjkyNlYyMi41NTFDMjAuMjMwMSAyMi41NTEgMjAuNjE5NiAyMS42ODU1IDE5LjQ1MTEgMjEuNjg1NUMxOC4yODI3IDIxLjY4NTUgMTguNDU1OCAyMi41NTEgMTcuNjMzNiAyMi41NTFDMTYuODExNCAyMi41NTEgMTYuNTk1IDIxLjY4NTUgMTUuNjQzIDIxLjY4NTVDMTQuNjkwOSAyMS42ODU1IDE0Ljc3NzUgMjIuNTUxIDEzLjkxMiAyMi41NTFDMTMuMDQ2NSAyMi41NTEgMTIuODczNCAyMS42ODU1IDExLjgzNDggMjEuNjg1NUMxMC43OTYyIDIxLjY4NTUgMTAuNzUyOSAyMi41NTEgMTAuMTkwMyAyMi41NTFWMjEuNjg1NU0xOS40NTExIDMuMTYzNzZWMU0xMC4xOTAzIDQuNTQ4NTZIMi45MjAxOFYxOS4xMzIzTTEwLjE5MDMgNC41NDg1NlY2LjQwOTM5TTEwLjE5MDMgMTkuMTMyM1YyMS42ODU1TTEwLjE5MDMgMTkuMTMyM0g0LjczNzcyTTEwLjE5MDMgMTkuMTMyM1YxMi43NzA4TTEwLjE5MDMgMjEuNjg1NUgxLjQ0ODgzVjE5LjEzMjNINC43Mzc3Mk0xMC4xOTAzIDYuNDA5MzlINC43Mzc3MlYxMi43NzA4TTEwLjE5MDMgNi40MDkzOVYxMi43NzA4TTQuNzM3NzIgMTkuMTMyM1YxMi43NzA4TTEwLjE5MDMgMTIuNzcwOEg0LjczNzcyTTEuNzk1MDMgMS43Nzg5NUgyMi4xMzQyIi8+PHBhdGggaWQ9IlZlY3Rvcl9fX19fMF8xX1JXV1JDVFFKS1giIHN0cm9rZT0iYmxhY2siIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIgc3Ryb2tlLWxpbmVqb2luPSJyb3VuZCIgc3Ryb2tlLXdpZHRoPSIxLjIiIGQ9Ik0xMy4xMjI3IDEwLjgxNDdDMTMuOTc1NyAxMC4yODQ5IDE0Ljk3MjUgOS45Nzk1OSAxNi4wMzIxIDkuOTc5NTlDMTcuMDI4OCA5Ljk3OTU5IDE3Ljk2MjYgMTAuMjQgMTguNzYxOCAxMC42OTgiLz48cGF0aCBpZD0iVmVjdG9yX19fX18wXzJfT0VYS1NSUlNMRiIgc3Ryb2tlPSJibGFjayIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIiBzdHJva2Utd2lkdGg9IjEuMiIgZD0iTTE0LjU2ODIgMTMuMTY3MkMxNC45OTAyIDEyLjg5NzggMTUuNDkzMSAxMi43NTQxIDE2LjAyMjkgMTIuNzU0MUMxNi41MjU3IDEyLjc1NDEgMTYuOTkyNyAxMi44Nzk5IDE3LjM4NzggMTMuMTEzMyIvPjxwYXRoIGlkPSJWZWN0b3JfX19fXzBfM19VUEFaR05TWUpHIiBmaWxsPSJibGFjayIgZD0iTTE1LjEyNTQgMTUuNTIwMUMxNS4xMjU0IDE1LjAwODMgMTUuNTI5NSAxNC41OTUyIDE2LjA0MTMgMTQuNTk1MkgxNi4wNTAzQzE2LjU2MjEgMTQuNTk1MiAxNi45NzUyIDE1LjAwODMgMTYuOTc1MiAxNS41MjAxQzE2Ljk3NTIgMTYuMDMyIDE2LjU2MjEgMTYuNDQ1IDE2LjA1MDMgMTYuNDQ1QzE1LjUzODQgMTYuNDQ1IDE1LjEyNTQgMTYuMDMyIDE1LjEyNTQgMTUuNTIwMVoiLz48ZWxsaXBzZSBpZD0iRWxsaXBzZSAxMjU5X19fX18wXzRfWEtEWFJMU0VLTiIgY3g9IjEuODk3OTUiIGN5PSIxLjg5Nzk2IiBmaWxsPSJibGFjayIgcng9IjAuODk3OTUyIiByeT0iMC44OTc5NTkiLz48ZWxsaXBzZSBpZD0iRWxsaXBzZSAxMjYwX19fX18wXzVfVlBMSlVEU0xJWSIgY3g9IjIyLjEwMiIgY3k9IjEuODk3OTYiIGZpbGw9ImJsYWNrIiByeD0iMC44OTc5NTIiIHJ5PSIwLjg5Nzk1OSIvPjwvZz48L2c+PC9nPjwvc3ZnPg==",
                    "EN": "Curtain Controller",
                    "TW": "窗簾控制器",
                    "CN": "窗帘控制器",
                    "BR": "Controlador de cortina",
                    "CZ": "Ovladač závěsů",
                    "DA": "Gardinkontrol",
                    "DE": "Gardinensteuerung",
                    "ES": "Controlador de cortinas",
                    "FI": "Verhojen ohjain",
                    "FR": "Contrôleur de rideau",
                    "HU": "Függönyvezérlő",
                    "IT": "Controllo tende",
                    "JP": "カーテン コントローラ",
                    "KR": "커튼 조절기",
                    "MS": "Curtain Controller",
                    "NL": "Gordijnregelaar",
                    "NO": "Gardinfjernkontroll",
                    "PL": "Sterownik do zasłon",
                    "RU": "Пульт управления занавесками",
                    "SV": "Gardinstyrenhet",
                    "TH": "อุปกรณ์ควบคุมผ้าม่าน",
                    "TR": "Perde Kumandası",
                    "UK": "Контролер штор",
                    "RO": "Controler perdea",
                    "SL": "Krmilnik zaves"
                },
                {
                    "category": "others",
                    "ui-sort": 9,
                    "type": [
                        "117"
                    ],
                    "base64": "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgZmlsbD0ibm9uZSIgdmlld0JveD0iMCAwIDI0IDI0Ij48ZyBpZD0iR3JvdXAiPjxnIGlkPSJGcmFtZSA0NzQ0Ij48cGF0aCBpZD0iUmVjdGFuZ2xlIDEwMjQ4X19fX18wXzBfV0ZDV0tSRUNIWiIgc3Ryb2tlPSJibGFjayIgZD0iTTEyLjc3MTQgMi4yNVY4QzEyLjc3MTQgOC41NTIyOSAxMy4yMTkxIDkgMTMuNzcxNCA5SDE5LjcxNDNNMTIuNzcxNCAyLjI1VjJDMTIuNzcxNCAxLjQ0NzcyIDEzLjIxOTEgMSAxMy43NzE0IDFIMjBDMjAuNTUyMyAxIDIxIDEuNDQ3NzIgMjEgMlY4QzIxIDguNTUyMjkgMjAuNTUyMyA5IDIwIDlIMTkuNzE0M00xMi43NzE0IDIuMjVINUMzLjg5NTQzIDIuMjUgMyAzLjE0NTQzIDMgNC4yNVYxNC4yNUMzIDE1LjM1NDYgMy44OTU0MyAxNi4yNSA1IDE2LjI1SDYuNk0xOS43MTQzIDlWMTQuMjVDMTkuNzE0MyAxNS4zNTQ2IDE4LjgxODkgMTYuMjUgMTcuNzE0MyAxNi4yNUgxNS44NTcxTTYuNiAxNi4yNVYxNy41QzYuNiAxOC4wNTIzIDcuMDQ3NzIgMTguNSA3LjYgMTguNUg5Ljk0Mjg2TTYuNiAxNi4yNUgxNS44NTcxTTE1Ljg1NzEgMTYuMjVWMTcuNUMxNS44NTcxIDE4LjA1MjMgMTUuNDA5NCAxOC41IDE0Ljg1NzEgMTguNUgxMi41MTQzTTkuOTQyODYgMTguNUgxMi41MTQzTTkuOTQyODYgMTguNVYyMU0xMi41MTQzIDE4LjVWMjJDMTIuNTE0MyAyMi41NTIzIDEyLjA2NjYgMjMgMTEuNTE0MyAyM0gxMC45NDI5QzEwLjM5MDYgMjMgOS45NDI4NiAyMi41NTIzIDkuOTQyODYgMjJWMjFNNi42IDIxSDkuOTQyODZNMTUuMzQyOSA3SDE4LjQyODZDMTguNzEyNiA3IDE4Ljk0MjkgNi43NzYxNCAxOC45NDI5IDYuNVYzLjVDMTguOTQyOSAzLjIyMzg2IDE4LjcxMjYgMyAxOC40Mjg2IDNIMTUuMzQyOUMxNS4wNTg4IDMgMTQuODI4NiAzLjIyMzg2IDE0LjgyODYgMy41VjYuNUMxNC44Mjg2IDYuNzc2MTQgMTUuMDU4OCA3IDE1LjM0MjkgN1pNNS40ODU3MSA5LjVIMTAuNzE0M0MxMC45OTgzIDkuNSAxMS4yMjg2IDkuMjc2MTQgMTEuMjI4NiA5VjRDMTEuMjI4NiAzLjcyMzg2IDEwLjk5ODMgMy41IDEwLjcxNDMgMy41SDUuNDg1NzFDNS4yMDE2OCAzLjUgNC45NzE0MyAzLjcyMzg2IDQuOTcxNDMgNFY5QzQuOTcxNDMgOS4yNzYxNCA1LjIwMTY4IDkuNSA1LjQ4NTcxIDkuNVpNNS40NDI4NiAxOS41VjE5LjVDNi4wODE5MyAxOS41IDYuNiAyMC4wMTgxIDYuNiAyMC42NTcxVjIxLjU5MjlDNi42IDIyLjIzMTkgNi4wODE5MyAyMi43NSA1LjQ0Mjg2IDIyLjc1VjIyLjc1QzQuODAzNzggMjIuNzUgNC4yODU3MSAyMi4yMzE5IDQuMjg1NzEgMjEuNTkyOVYyMC42NTcxQzQuMjg1NzEgMjAuMDE4MSA0LjgwMzc4IDE5LjUgNS40NDI4NiAxOS41WiIvPjwvZz48L2c+PC9zdmc+",
                    "EN": "GoPro",
                    "TW": "GoPro",
                    "CN": "GoPro",
                    "BR": "GoPro",
                    "CZ": "GoPro",
                    "DA": "GoPro",
                    "DE": "GoPro",
                    "ES": "GoPro",
                    "FI": "GoPro",
                    "FR": "GoPro",
                    "HU": "GoPro",
                    "IT": "GoPro",
                    "JP": "GoPro",
                    "KR": "GoPro",
                    "MS": "GoPro",
                    "NL": "GoPro",
                    "NO": "GoPro",
                    "PL": "GoPro",
                    "RU": "GoPro",
                    "SV": "GoPro",
                    "TH": "GoPro",
                    "TR": "GoPro",
                    "UK": "GoPro",
                    "RO": "GoPro",
                    "SL": "GoPro"
                }
            ]
        };
        this.iconList = iconList;
    }

    async fetchIconList() {
        const response = await fetch('https://nw-dlcdnet.asus.com/beta/plugin/js/extend_custom_svg_icon.json')
            .then(res => res.json())
            .then(data => {
                this.iconList = data;
                this.iconList.source = "cloud";
            });
    }

    getIconList() {
        return this.iconList;
    }

    queryIconList(query) {
        const lowerQuery = query.toLowerCase();
        const filteredDevices = this.iconList.devices.filter(device => {
            const name = device[ui_lang] || device.title;
            return name.toLowerCase().includes(lowerQuery);
        });

        const validCategoryIds = new Set(filteredDevices.map(device => device.category));

        const filteredCategories = this.iconList.category.filter(cat => validCategoryIds.has(cat.id));

        return {
            category: filteredCategories,
            devices: filteredDevices
        };
    }

    getIconByType(type) {
        const icon = this.iconList.devices.find(device => device.type.includes(type));
        if (icon) {
            return {
                type: icon.type[0],
                source: this.iconList.source || 'local',
                src: icon.base64 || icon.src || ''
            }
        } else {
            return null;
        }
    }
}

export class IconSelectorPanel extends AsuswrtPopupPanel {
    constructor(ClientInfoView, IconList) {
        super();
        this.element.id = "icon-selector-panel";
        this.iconList = IconList.getIconList();
        this.selectedIcon = null;


        const shadowRoot = this.shadowRoot;
        shadowRoot.querySelector('.modal-title').innerText = 'Change Icon';

        const link = document.createElement('link');
        link.rel = 'stylesheet';
        link.href = '/device-map/device-map.css';
        shadowRoot.querySelector('.popup_bg').insertAdjacentElement('beforebegin', link);

        const style = document.createElement('link');
        style.rel = 'stylesheet';
        style.href = '/device-map/clientlist.css';
        shadowRoot.querySelector('.popup_bg').insertAdjacentElement('beforebegin', style);

        const toolsDiv = document.createElement('div');
        toolsDiv.classList.add('tool-bar');
        shadowRoot.querySelector('.modal-body').appendChild(toolsDiv);

        const searchDiv = document.createElement('div');
        searchDiv.classList.add('search');
        searchDiv.innerHTML = `
            <div class="search-icon"><i class="icon-search"></i></div>
            <input type="text" class="search-input" placeholder="Search">`;
        toolsDiv.appendChild(searchDiv);

        const addIcon = document.createElement('div');
        addIcon.classList.add('add-icon');
        addIcon.innerHTML = `<i class="icon-add"></i>`;
        toolsDiv.appendChild(addIcon);

        const iconDiv = document.createElement('div');
        iconDiv.classList.add('icon-selector');
        this.iconDiv = iconDiv;
        shadowRoot.querySelector('.modal-body').appendChild(iconDiv);
        this.genIconList();


        const uploadDiv = document.createElement('div');
        uploadDiv.classList.add('custom-upload');
        uploadDiv.innerHTML = `
                <div class="upload-icon"><i class="icon-upload"></i></div>
                <span>Upload image</span>
                <input type="file" class="custom-upload-file" accept="image/*" />`;
        const input = uploadDiv.querySelector('.custom-upload-file');
        input.addEventListener('change', (e) => {
            const file = e.target.files[0];
            if (file) {
                const reader = new FileReader();
                reader.onload = function (e) {
                    const src = e.target.result;
                    const icon = {
                        type: 'usericon',
                        source: 'custom',
                        src: src
                    }
                    this.selectedIcon = icon;
                    ClientInfoView.changeIcon(icon);
                    this.close();
                }.bind(this);
                reader.readAsDataURL(file);
            }
        });

        uploadDiv.addEventListener('drop', (e) => {
            e.preventDefault();
            const file = e.dataTransfer.files[0];
            if (file) {
                const reader = new FileReader();
                reader.onload = function (e) {
                    const src = e.target.result;
                    const icon = {
                        type: 'usericon',
                        source: 'custom',
                        src: src
                    }
                    this.selectedIcon = icon;
                    ClientInfoView.changeIcon(icon);
                    this.close();
                }.bind(this);
                reader.readAsDataURL(file);
            }
        });


        addIcon.addEventListener('click', () => {
            iconDiv.remove();
            toolsDiv.remove();
            restoreBtn.remove();
            applyBtn.remove();
            shadowRoot.querySelector('.modal-body').appendChild(uploadDiv);
        });

        const restoreBtn = new AsuswrtButton({
            text: 'Restore Default',
            className: 'btn-secondary',
            onclick: () => {
                ClientInfoView.resetIcon();
                this.close();
            }
        });

        const closeBtn = new AsuswrtButton({
            text: 'Cancel',
            className: 'btn-secondary',
            animation: '',
            onclick: () => {
                this.close();
            }
        });

        const applyBtn = new AsuswrtButton({
            text: 'Apply',
            className: 'btn-primary',
            animation: 'slide',
            onclick: () => {
                if (this.selectedIcon === null) {
                    alert(`Please select an icon`);
                } else {
                    ClientInfoView.changeIcon(this.selectedIcon);
                    this.close();
                }
            }
        });

        const footerDiv = shadowRoot.querySelector('.modal-footer');

        const footerLeftDiv = document.createElement('div');
        footerLeftDiv.classList.add('footer-left');
        footerLeftDiv.appendChild(restoreBtn);
        footerDiv.appendChild(footerLeftDiv);


        const footerRightDiv = document.createElement('div');
        footerRightDiv.classList.add('footer-right');
        footerRightDiv.appendChild(closeBtn);
        footerRightDiv.appendChild(applyBtn);
        footerDiv.appendChild(footerRightDiv);

        shadowRoot.querySelector(".search-input").addEventListener('input', (e) => {
            const query = e.target.value;
            this.iconList = IconList.queryIconList(query);
            this.genIconList();
        });

    }

    genIconList() {

        const categoryList = this.iconList.category;
        const deviceList = this.iconList.devices;

        this.iconDiv.innerHTML = categoryList.map(category => {
            return `<div class="category" data-category="${category.id}">
                        <div class="category-title">${category[ui_lang]}</div>
                        <div class="category-devices">
                        <div class="category-devices-container">
                        ${deviceList.filter(device => device.category === category.id).map(device => {
                const source = (typeof this.iconList.source !== "undefined") ? this.iconList.source : 'local';
                return `<div class="client-device" data-type="${device.type[0]}" data-source="${source}">
                                        <div class="client-device-icon">
                                            <i class="type${device.type[0]}" style="--svg: url('${device.base64}');"></i>
                                        </div>
                                        <div class="client-device-title">${device[ui_lang] ? device[ui_lang] : device.title}</div>
                                    </div>`
            }).join('')}                        
                        </div>
                        </div>
                    </div>`
        }).join('');

        this.iconDiv.querySelectorAll('.client-device').forEach(icon => {
            icon.addEventListener('click', (e) => {
                this.iconDiv.querySelectorAll('.client-device').forEach(icon => {
                    icon.classList.remove('selected');
                });
                const icon = e.target.closest('.client-device');
                if (!icon) return;
                this.selectedIcon = {
                    type: icon.dataset.type,
                    source: icon.dataset.source,
                    src: getComputedStyle(icon.querySelector('i')).getPropertyValue('--svg').replace(/^url\(["']?|["']?\)$/g, '')
                };
                icon.classList.add('selected');
            })
        });
    }
}

const CsvExporter = {
    downloadCsv(filename, content) {
        const bom = '\uFEFF';
        const blob = new Blob([bom + content], { type: 'text/csv;charset=utf-8;' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = filename;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
    },

    getValueByPath(row, path) {
        if (row == null) return '';
        if (typeof path === 'number') return row[path] ?? '';
        if (Array.isArray(row)) return row[path] ?? '';
        if (typeof row !== 'object') return row;
        if (!path || path.indexOf('.') === -1) return (row[path] ?? '');
        return path.split('.').reduce((o, k) => (o == null ? '' : o[k]), row) ?? '';
    },

    escapeCsv(val) {
        return `"${String(val ?? '').replace(/"/g, '""')}"`;
    },

    safeCsv(val) {
        return this.escapeCsv(val === undefined || val === null || val === '' ? '-' : val);
    },

    formatSpecialColumn(columnID, row, rowKey) {
        switch (columnID) {
            case "connStatus":
                return this.safeCsv(this.getValueByPath(row, rowKey) ? `<#Clientlist_Online#>` : `<#Clientlist_OffLine#>`);
            case "connType":
                const val = this.getValueByPath(row, rowKey);
                return this.safeCsv(row.displayConnectType || (val == "0" ? `<#tm_wired#>` : `<#tm_wireless#>`));
            case "phyRate":
                const tx = row.db_sta_tx || '-';
                const rx = row.db_sta_rx || '-';
                return (tx === '-' && rx === '-') ? this.safeCsv('-') : this.safeCsv(`${tx} Mbps / ${rx} Mbps`);
            case "realTimeTraffic":
                return this.safeCsv(`<#InternetSpeed_Upload#> ${row.realTimeTx || 0} / <#InternetSpeed_Download#> ${row.realTimeRx || 0}`);
            case "rssi":
                const rssiVal = this.getValueByPath(row, rowKey);
                return row.isWL != 0 ? this.safeCsv(`${rssiVal} dBm`) : this.safeCsv('-');
            case "accessTime":
                const time = row.wlConnectTime ?? "";
                return (time == "00:00:00" || time == "") ? "-" : time;
            case "vlan":
                const vlanVal = this.getValueByPath(row, rowKey);
                return (vlanVal === "" || vlanVal === "0" || typeof vlanVal === 'undefined') ? "-" : vlanVal;
            default:
                return this.safeCsv(this.getValueByPath(row, rowKey));
        }
    },

    exportDataTableToCSV(dtApi, filename = 'export.csv') {
        const rows = dtApi.rows({ search: 'applied' }).data().toArray();

        if (!rows.length) {
            alert(`<#IPConnection_VSList_Norule#>`);
            return;
        }

        const exportArray = [];
        const aoColumns = dtApi.settings()[0].aoColumns;

        aoColumns.forEach(col => {
            if (col.bVisible !== false) {
                let title = col.sTitle || col.mData || col.idx;
                if (title === "isOnline") title = `<#statusTitle_Client#>`;
                if (title === "isWL") title = `<#Connection_Type#>`;

                exportArray.push({
                    header: title,
                    rowKey: col.mData !== undefined ? col.mData : col.idx,
                    columnID: col.id !== undefined ? col.id : col.idx
                });
            }
        });

        const lines = [];
        lines.push(exportArray.map(h => this.safeCsv(h.header)).join(','));
        for (const row of rows) {
            const line = exportArray.map(({ rowKey, columnID }) =>
                this.formatSpecialColumn(columnID, row, rowKey)
            ).join(',');
            lines.push(line);
        }

        const csv = lines.join('\r\n');
        this.downloadCsv(filename, csv);
    }
};