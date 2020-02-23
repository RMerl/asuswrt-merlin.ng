function get_appropriate_ws_uri() {
    var pcol;
    var u = document.URL;

    if (u.substring(0, 5) == "https") {
        pcol = "wss://";
        u = u.substr(8);
    } else {
        pcol = "ws://";
        if (u.substring(0, 4) == "http")
            u = u.substr(7);
        else if (u.substring(0, 4) == "file")
            u = "localhost:8080";
    }
    u = u.split('/');
    return pcol + u[0] + ':7683/';   
}
