export function isSupport(_ptn) {
    const ui_support = [<% get_ui_support();%>][0];
    return (ui_support[_ptn]) ? ui_support[_ptn] : 0;
}