diag_mac = '<% nvram_get("chksta_mac"); %>';
diag_band = '<% nvram_get("chksta_band"); %>';
diag_cap = decodeURIComponent('<% nvram_char_to_ascii("","diag_chk_cap"); %>');
diag_re1 = decodeURIComponent('<% nvram_char_to_ascii("","diag_chk_re1"); %>');
diag_re2 = decodeURIComponent('<% nvram_char_to_ascii("","diag_chk_re2"); %>');
nodes_info = [<% show_info_between_nodes(); %>];
