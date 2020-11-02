<!--
Copyright (C) 2020, Broadcom. All Rights Reserved.

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
$ID$-->

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html lang="en">
<head>
<title>Broadcom Home Gateway Reference Design: AirIQ</title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<link rel="stylesheet" type="text/css" href="style.css" media="screen">
<script language="JavaScript" type="text/javascript" src="overlib.js"></script>
</head>

<body>
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000;"></div>
<input type=hidden name="wl_mode_changed" value=0>
<input type=hidden name="wl_ure_changed" value=0>

<table id="page_header"  border="0" cellpadding="0" cellspacing="0" width="100%" bgcolor="#cc0000">
  <% asp_list(); %>
</table>

<table border="0" cellpadding="0" cellspacing="0" width="100%">
  <tr class="page_title">
    <td colspan="2" class="edge"><img border="0" src="blur_new.jpg" alt=""></td>
  </tr>
  <tr>
    <td class="page_title"><img border="0" src="logo_new.gif" alt=""></td>
    <td width="100%" valign="top">
	<br>
	<span class="title">AirIQ</span><br>
	<span class="subtitle">This page allows you to launch AirIQ displays.</span>
    </td>
  </tr>
</table>

<form method="post" action="airiq.asp">
<input type="hidden" name="page" value="airiq.asp">

<p>
<table border="0" cellpadding="0" cellspacing="0">
  <tr>
    <th width="310"
	onMouseOver="return overlib('AirIQ Displays.', LEFT);"
	onMouseOut="return nd();">
	AirIQ Displays&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
	<td></td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Channel Quality Display.', LEFT);"
	onMouseOut="return nd();">
	Channel Quality:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	    <input type="button" value="Launch" onclick="window.open('localapp.asp', '_blank', 'dependent=yes,resizable=yes,toolbar=no,status=no,menubar=no,height=800,width=1000,scrollbars=yes'); return false">
    </td>
	<td></td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Spectrogram Display.', LEFT);"
	onMouseOut="return nd();">
	Spectrogram:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	    <input type="button" value="Launch" onclick="window.open('spectrogram.asp', '_blank', 'dependent=yes,resizable=yes,toolbar=no,status=no,menubar=no,height=800,width=1000,scrollbars=yes'); return false">
    </td>
	<td></td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Persistence Display.', LEFT);"
	onMouseOut="return nd();">
	Persistence:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	    <input type="button" value="Launch" onclick="window.open('persistence.asp', '_blank', 'dependent=yes,resizable=yes,toolbar=no,status=no,menubar=no,height=800,width=1000,scrollbars=yes'); return false">
    </td>
	<td></td>
  </tr>
  <tr>
    <th width="310"
	onMouseOver="return overlib('Equalizer Display.', LEFT);"
	onMouseOut="return nd();">
	Equalizer:&nbsp;&nbsp;
    </th>
    <td>&nbsp;&nbsp;</td>
    <td>
	    <input type="button" value="Launch" onclick="window.open('equalizer.asp', '_blank', 'dependent=yes,resizable=yes,toolbar=no,status=no,menubar=no,height=800,width=1000,scrollbars=yes'); return false">
    </td>
	<td></td>
  </tr>
</table>

</form>

<p class="label">&#169;2001-2016 Broadcom Limited. All rights reserved. 54g and XPress are trademarks of Broadcom Limited.</p>

</body>
</html>
