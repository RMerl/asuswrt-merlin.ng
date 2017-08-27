vpnd_state='<% nvram_get("vpn_server1_state"); %>';
vpn_upload_state='<% nvram_get("vpn_upload_state"); %>';
vpn_server1_errno='<% nvram_get("vpn_server1_errno"); %>';

<% vpn_crt_client(); %>
<% vpn_crt_server(); %>