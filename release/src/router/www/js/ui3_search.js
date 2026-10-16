/*
 * UI3 live search dropdown, ported from UI4's js/indexing.js dict-string search.
 * Standalone file — loaded once from state.js's show_footer(); does not
 * modify js/faq.js or the FAQ_input/faq-block markup that show_footer() builds.
 *
 * Scope: string-level search only (no ai_board_slm / AI Q&A — UI3 has no such
 * feature). Overrides the global filterFAQ() that #FAQ_input's
 * onkeyup="filterFAQ()" already calls, so no extra event binding is added.
 */
(function(){
    var dictData = {};
    var usageIndex = {};
    var menuList = null;
    var menuExclude = null;

    // Strings holding %@ / %1$@ / %1$s placeholders (%1$s only in RU.dict) are filled in by the
    // page that renders them. Search results have no such context and would show the raw
    // "%1$@" to the user, so keep them out of the index.
    const placeholderPattern = /%(?:\d+\$)?@|%\d+\$s/;

    // [aa]...[/aa] and [bu]...[/bu] are markup the rendering page turns into <a> / <b><u>
    // (see js/asus_policy.js, js/asus_pincode.js). Only these two pairs are markup: other
    // bracketed text such as [Apply] or [EBG19] is literal UI wording and must be kept.
    const markupTagPattern = /\[\/?(?:aa|bu)\]/g;

    function parseDict(text){
        var lines = text.split(/\r?\n/);
        var data = {};
        var repl = {};
        try{
            var nv = httpApi.nvramGet(["odmpid", "local_domain"]);
            repl.ZVMODELVZ = nv.odmpid;
            repl.ZVDOMAIN_NAMEVZ = nv.local_domain;
        }
        catch(e){}
        lines.forEach(function(line){
            var eqIdx = line.indexOf("=");
            if(eqIdx > -1){
                var key = line.substring(0, eqIdx).trim();
                var value = line.substring(eqIdx + 1).trim();
                if(key && value && !placeholderPattern.test(value)){
                    if(repl.ZVMODELVZ) value = value.split("ZVMODELVZ").join(repl.ZVMODELVZ);
                    if(repl.ZVDOMAIN_NAMEVZ) value = value.split("ZVDOMAIN_NAMEVZ").join(repl.ZVDOMAIN_NAMEVZ);
                    data[key] = value.replace(markupTagPattern, "");
                }
            }
        });
        return data;
    }

    function resolveMenuTree(cb){
        var cachedList, cachedExclude;
        try{
            cachedList = Session.get("menuList." + ui_lang);
            cachedExclude = Session.get("menuExclude");
        }
        catch(e){}
        if(cachedList && cachedExclude){
            cb(cachedList, cachedExclude);
            return;
        }
        require(['/require/modules/menuTree.js'], function(menuTree){
            var list = menuTree.list;
            var exclude = {menus: menuTree.exclude.menus(), tabs: menuTree.exclude.tabs()};
            try{
                Session.set("menuList." + ui_lang, list);
                Session.set("menuExclude", exclude);
            }
            catch(e){}
            cb(list, exclude);
        });
    }

    function loadData(){
        var lang = (typeof ui_lang !== "undefined" && ui_lang) ? ui_lang : "EN";

        fetch("/" + lang + ".dict", {cache: "force-cache"})
            .then(function(res){ return res.text(); })
            .then(function(text){ dictData = parseDict(text); })
            .catch(function(){});

        fetch("/js/ui3_indexing.json", {cache: "force-cache"})
            .then(function(res){ return res.json(); })
            .then(function(json){ usageIndex = json; })
            .catch(function(){});

        resolveMenuTree(function(list, exclude){
            menuList = list;
            menuExclude = exclude;
            labelIndex = buildLabelIndex();
        });

        injectStyle();
    }

    // Single pass over menuList that resolves every reachable (menu, tab)
    // pair to its display label, so __HIDE__/__INHERIT__ chasing happens in
    // exactly one place. getTabNameFromUrl/getMenuOrder/isQisTab/the tabUrl
    // lookup all used to rebuild this same label independently — cheap for
    // __HIDE__/__INHERIT__'s own bare-menuName fallback (both sides agree
    // trivially), but genuinely diverges once menuExclude is involved: a
    // label can inherit from an excluded sibling tab, or collide with an
    // identical label produced by an entirely different (excluded) menu,
    // and only this exclude-aware walk resolves that correctly.
    var labelIndex = null;

    function buildLabelIndex(){
        var orderOf = {}, urlOf = {}, qisOf = {}, labelOfUrl = {};
        for(var i = 0; i < menuList.length; i++){
            var menu = menuList[i];
            if(menuExclude.menus.indexOf(menu.index) !== -1) continue;
            var lastValidTabName = null;
            for(var j = 0; j < menu.tab.length; j++){
                var tab = menu.tab[j];
                if(menuExclude.tabs.indexOf(tab.url) !== -1) continue;
                var label;
                if(tab.tabName === "__HIDE__" || tab.tabName === "__INHERIT__"){
                    label = lastValidTabName ? (menu.menuName + " > " + lastValidTabName) : menu.menuName;
                }
                else{
                    lastValidTabName = tab.tabName;
                    label = menu.menuName + " > " + tab.tabName;
                }
                if(!(label in orderOf)) orderOf[label] = i;
                if(!(label in urlOf) && tab.url !== "NULL") urlOf[label] = tab.url;
                if(menu.index === "menu_QIS") qisOf[label] = true;
                if(tab.url !== "NULL" && !(tab.url in labelOfUrl)) labelOfUrl[tab.url] = label;
            }
        }
        return {orderOf: orderOf, urlOf: urlOf, qisOf: qisOf, labelOfUrl: labelOfUrl};
    }

    function getTabNameFromUrl(url){
        if(!labelIndex) return null;
        return (url in labelIndex.labelOfUrl) ? labelIndex.labelOfUrl[url] : null;
    }

    function getMenuOrder(tabName){
        if(!labelIndex) return menuList ? menuList.length : 0;
        return (tabName in labelIndex.orderOf) ? labelIndex.orderOf[tabName] : menuList.length;
    }

    function isQisTab(tabName){
        return !!(labelIndex && labelIndex.qisOf[tabName]);
    }

    function getTabUrl(tabName){
        return (labelIndex && labelIndex.urlOf[tabName]) || null;
    }

    function escapeRegExp(str){
        return str.replace(/[-\/\\^$*+?.()|[\]{}]/g, "\\$&");
    }

    // tabName/tabUrl come from menuTree.js (mostly static, but ZVMODELVZ /
    // ZVDOMAIN_NAMEVZ dict placeholders get substituted with nvram values at
    // runtime) and dict strings are concatenated straight into block.innerHTML
    // below — escape everything dynamic before insertion so none of it can be
    // interpreted as markup.
    function escapeHtml(str){
        return String(str)
            .replace(/&/g, "&amp;")
            .replace(/</g, "&lt;")
            .replace(/>/g, "&gt;")
            .replace(/"/g, "&quot;")
            .replace(/'/g, "&#39;");
    }

    function renderResults(query){
        var block = document.getElementById("faq-block");
        if(!block) return;

        if(!query || query.length < 2 || !menuList){
            block.innerHTML = "";
            block.style.display = "none";
            return;
        }

        var resultsByTab = {};
        for(var flag in dictData){
            var value = dictData[flag].toLowerCase();
            if(value.indexOf(query) === -1) continue;
            var urls = usageIndex[flag];
            if(!urls || !urls.length) continue;

            var seenTabNames = {};
            for(var u = 0; u < urls.length; u++){
                var tabName = getTabNameFromUrl(urls[u]);
                if(tabName === null || seenTabNames[tabName]) continue;
                seenTabNames[tabName] = true;
                if(!resultsByTab[tabName]) resultsByTab[tabName] = [];
                resultsByTab[tabName].push(dictData[flag]);
            }
        }

        var sortedTabs = Object.keys(resultsByTab).sort(function(a, b){
            var orderA = getMenuOrder(a);
            var orderB = getMenuOrder(b);
            if(orderA !== orderB) return orderA - orderB;
            return a.localeCompare(b);
        });
        var matched = sortedTabs.filter(function(t){ return t.toLowerCase().indexOf(query) !== -1; });
        var unmatched = sortedTabs.filter(function(t){ return t.toLowerCase().indexOf(query) === -1; });
        sortedTabs = matched.concat(unmatched);

        // QIS (Quick Internet Setup) sits first in menuTree.list, which would
        // otherwise put it ahead of far more relevant results on every query.
        // Always push it to the very end regardless of match status.
        var qisTabs = sortedTabs.filter(isQisTab);
        var nonQisTabs = sortedTabs.filter(function(t){ return !isQisTab(t); });
        sortedTabs = nonQisTabs.concat(qisTabs);

        if(sortedTabs.length === 0){
            block.innerHTML = "";
            block.style.display = "none";
            return;
        }

        var highlightRegex = new RegExp("(" + escapeRegExp(query) + ")", "gi");
        var html = "";

        sortedTabs.forEach(function(tabName){
            var strings = resultsByTab[tabName].filter(function(v, idx, arr){ return arr.indexOf(v) === idx; });
            var tabUrl = getTabUrl(tabName);

            html += '<div class="ui3sr-result-item">';
            html += '<p class="ui3sr-tab-name"><a href="' + escapeHtml(tabUrl || "#") + '">' + escapeHtml(tabName) + '</a></p>';
            html += '<ul>';
            strings.forEach(function(str){
                var highlighted = escapeHtml(str).replace(highlightRegex, '<span class="ui3sr-highlight">$1</span>');
                html += '<li class="ui3sr-found-string">&quot;' + highlighted + '&quot;</li>';
            });
            html += '</ul></div>';
        });

        block.innerHTML = html;
        block.style.display = "";
    }

    window.filterFAQ = function(){
        var query = ($("#FAQ_input").val() || "").trim().toLowerCase();
        renderResults(query);
    };

    function injectStyle(){
        if(document.getElementById("ui3sr-style")) return;
        var style = document.createElement("style");
        style.id = "ui3sr-style";
        style.textContent = [
            ".faq-filter-block{background:#151515;}",
            ".ui3sr-result-item{margin-bottom:10px;padding-bottom:8px;border-bottom:1px solid #474747;}",
            ".ui3sr-result-item:last-child{border-bottom:none;margin-bottom:0;padding-bottom:0;}",
            ".ui3sr-tab-name{margin:0 0 4px 0;font-size:13px;font-weight:bold;}",
            ".ui3sr-tab-name a{color:#4DA6FF;text-decoration:none;}",
            ".ui3sr-tab-name a:hover{text-decoration:underline;}",
            ".ui3sr-result-item ul{list-style-type:none;margin:0;padding-left:0;}",
            ".ui3sr-found-string{color:#CCCCCC;font-size:12px;padding:3px 0 3px 12px;}",
            ".ui3sr-highlight{color:#FFD54A;font-weight:bold;}"
        ].join("\n");
        document.head.appendChild(style);
    }

    loadData();
})();
