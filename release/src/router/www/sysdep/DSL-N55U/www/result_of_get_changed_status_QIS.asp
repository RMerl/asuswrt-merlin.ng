<% wanlink(); %>
<% wanstate(); %>
parent.allUsbStatusArray = <% show_usb_path(); %>;
link_wan_status = "<% nvram_get("link_wan"); %>";
link_wan1_status = "<% nvram_get("link_wan1"); %>";
dsl_autodet_state = "<% nvram_get("dsltmp_autodet_state"); %>";
dsl_line_state = "<% nvram_get("dsltmp_adslsyncsts"); %>";
wan_type = "<% nvram_get("dsltmp_autodet_wan_type"); %>";
dslx_annex_state = '<% nvram_get("dslx_annex"); %>';
