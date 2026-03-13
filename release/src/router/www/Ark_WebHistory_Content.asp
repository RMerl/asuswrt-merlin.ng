<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
        "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
    <meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
    <meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
    <meta HTTP-EQUIV="Expires" CONTENT="-1">
    <link rel="shortcut icon" href="images/favicon.png">
    <link rel="icon" href="images/favicon.png">
    <title><#Web_Title#> - <#Adaptive_History#></title>
    <link rel="stylesheet" type="text/css" href="./index_style.css">
    <link rel="stylesheet" type="text/css" href="./form_style.css">
    <script type="text/javascript" src="./js/jquery.js"></script>
    <script type="text/javascript" src="./state.js"></script>
    <script type="text/javascript" src="./popup.js"></script>
    <script type="text/javascript" src="./general.js"></script>
    <script type="text/javascript" src="./help.js"></script>
    <script>
        function initial() {
            show_menu();
            document.getElementById("arklog_iframe").setAttribute("src", "/pages/ark_log.html");
        }
    </script>
</head>
<body onload="initial();" onunLoad="return unload_body();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<table class="content" align="center" cellpadding="0" cellspacing="0">
    <tr>
        <td width="17">&nbsp;</td>
        <!--=====Beginning of Main Menu=====-->
        <td valign="top" width="202">
            <div id="mainMenu"></div>
            <div id="subMenu"></div>
        </td>
        <td valign="top">
            <div id="tabMenu" class="submenuBlock"></div>
            <!--===================================Beginning of Main Content===========================================-->
            <table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
                <tr>
                    <td align="left" valign="top">
                        <table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle"
                               style="border-radius:3px;">
                            <tbody>
                            <tr>
                                <td bgcolor="#4D595D" valign="top">
                                    <iframe id="arklog_iframe" style="width: 100%; height: 100%" frameborder="0"></iframe>
                                </td>
                            </tr>
                            </tbody>
                        </table>
                    </td>
                </tr>
            </table>
            <!--===================================End of Main Content===========================================-->
        </td>
        <td width="10" align="center" valign="top">&nbsp;</td>
    </tr>
</table>
<div id="footer"></div>
</body>
</html>
