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
    <title><#Web_Title#> - <#menu5_6_2#></title>
    <link rel="stylesheet" type="text/css" href="index_style.css">
    <link rel="stylesheet" type="text/css" href="form_style.css">
    <link rel="stylesheet" type="text/css" href="pwdmeter.css">
    <link rel="stylesheet" type="text/css" href="device-map/device-map.css">
    <link rel="stylesheet" type="text/css" href="css/icon.css">
    <script language="JavaScript" type="text/javascript" src="/js/jquery.js"></script>
    <script language="JavaScript" type="text/javascript" src="/state.js"></script>
    <script language="JavaScript" type="text/javascript" src="/general.js"></script>
    <script language="JavaScript" type="text/javascript" src="/help.js"></script>
    <script type="text/javascript" src="/js/httpApi.js"></script>
    <script type="text/javascript" src="/js/asus_policy.js?v=5"></script>
    <script type="text/javascript" src="/js/asus_pincode.js?lang=<#selected_language#>"></script>
    <script>

        function initial() {
            show_menu();

            const pincodeComponent = new PincodeComponent();
            $('#pincode_block').append(pincodeComponent.render());

            const pincodeTitle = new PincodeTitleDiv();
            $('#pincode_title').append(pincodeTitle.render());

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
                    <td valign="top">
                        <table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle"
                               id="FormTitle">
                            <tbody>
                            <tr>
                                <td bgcolor="#4D595D" valign="top">
                                    <div>&nbsp;</div>
                                    <table width="100%" border="1" align="center" cellpadding="4" cellspacing="0"
                                           bordercolor="#6b8fa3" class="FormTable">
                                        <thead>
                                        <tr>
                                            <td id="pincode_title"></td>
                                        </tr>
                                        </thead>
                                        <tr>
                                            <td style="border: 0">
                                                <div id="pincode_block"></div>
                                            </td>
                                        </tr>
                                    </table>
                                </td>
                            </tr>
                            </tbody>
                        </table>
                    </td>
                    </form>
                </tr>
            </table>
            <!--===================================Ending of Main Content===========================================-->
        </td>

        <td width="10" align="center" valign="top">&nbsp;</td>
    </tr>
</table>

<div id="footer"></div>
</body>
</html>
