<!DOCTYPE html>
<html>
    <head>
        <meta charset="UTF-8" />
        <meta http-equiv="X-UA-Compatible" content="IE=edge" />
        <meta name="viewport" content="width=device-width, initial-scale=1.0,  user-scalable=no" />
        <link rel="shortcut icon" href="images/favicon.png" />
        <title><#Web_Title#> - <#menu5_1_1#></title>
        <link rel="stylesheet" href="index_style.css" />
        <link rel="stylesheet" href="form_style.css" />
        <link rel="stylesheet" href="usp_style.css" />
        <link rel="stylesheet" href="other.css" />
        <script src="/js/jquery.js"></script>
        <script src="/js/httpApi.js"></script>
        <script src="/state.js"></script>
        <script src="/js/asus.js"></script>
        <script>
            document.addEventListener("DOMContentLoaded", function(){show_menu();});
        </script>
    </head>

    <body class="bg">
        <div id="TopBanner"></div>
        <div id="Loading" class="popup_bg"></div>
        <table class="content">
            <tr>
                <td valign="top" width="202">
                    <div id="mainMenu"></div>
                    <div id="subMenu"></div>
                </td>
                <td valign="top">
                    <div id="tabMenu" class="submenuBlock"></div>
                </td>
            </tr>
        </table>
        <div id="footer"></div>
    </body>
</html>
