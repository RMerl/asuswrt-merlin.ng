var array_traffic = new Array();
var router_traffic = new Array();
array_traffic = <% dns_status("traffic", "", "realtime", ""); %>;

router_traffic = <% dns_status("traffic_wan", "", "realtime", ""); %>;
