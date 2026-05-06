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
    <title><#Web_Title#> - <#AiProtection_Home#></title>
    <link rel="stylesheet" type="text/css" href="/index_style.css">
    <link rel="stylesheet" type="text/css" href="/form_style.css">
    <script type="text/javascript" src="/js/jquery.js"></script>
    <script type="text/javascript" src="/state.js"></script>
    <script type="text/javascript" src="/popup.js"></script>
    <script type="text/javascript" src="/general.js"></script>
    <script type="text/javascript" src="/help.js"></script>
    <script>
        function initial() {
            show_menu();
            setTimeout(function () {
                var curTheme = (parent.webWrapper) ? "?current_theme=white" : "";
                document.getElementById("_iframe").setAttribute("src", `/pages/aiprotection_info.html` + curTheme);
            }, ((window.location.protocol == "https:") ? 1000 : 50));

        }

        // Auto-resize iframe to fit content height
        function resizeIframe() {
            const iframe = document.getElementById("_iframe");
            if (!iframe) return;

            try {
                const iframeDoc = iframe.contentDocument || iframe.contentWindow.document;
                if (iframeDoc && iframeDoc.body) {
                    // Get the actual content height
                    const height = Math.max(
                        iframeDoc.body.scrollHeight,
                        iframeDoc.body.offsetHeight,
                        iframeDoc.documentElement.scrollHeight,
                        iframeDoc.documentElement.offsetHeight
                    );
                    iframe.style.height = height + "px";
                }
            } catch (e) {
                console.error("Error resizing iframe:", e);
            }
        }

        // Setup iframe height monitoring
        document.addEventListener("DOMContentLoaded", function() {
            const iframe = document.getElementById("_iframe");

            // Resize when iframe loads
            iframe.addEventListener("load", function() {
                resizeIframe();

                // Use ResizeObserver to monitor content changes
                try {
                    const iframeDoc = iframe.contentDocument || iframe.contentWindow.document;
                    if (iframeDoc && iframeDoc.body) {
                        const resizeObserver = new ResizeObserver(function() {
                            resizeIframe();
                        });
                        resizeObserver.observe(iframeDoc.body);

                        // Also observe documentElement for comprehensive coverage
                        if (iframeDoc.documentElement) {
                            resizeObserver.observe(iframeDoc.documentElement);
                        }
                    }
                } catch (e) {
                    console.error("Error setting up ResizeObserver:", e);
                }

                // Fallback: periodic check for height changes
                setInterval(resizeIframe, 500);
            });
        });

        // Listen for messages from iframe (alternative method)
        window.addEventListener("message", function(event) {
            if (event.data && event.data.type === "iframeResize") {
                const iframe = document.getElementById("_iframe");
                if (iframe) {
                    iframe.style.height = event.data.height + "px";
                }
            }
        });
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
                                    <iframe id="_iframe" style="width: 100%; height: 100%" frameborder="0"></iframe>
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
