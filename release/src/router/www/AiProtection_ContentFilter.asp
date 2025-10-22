<!DOCTYPE html>
<html>
<head>
    <meta http-equiv="X-UA-Compatible" content="IE=Edge"/>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
    <meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
    <meta HTTP-EQUIV="Expires" CONTENT="-1">
    <title><#Web_Title#> - <#AiProtection_filter#></title>
    <link rel="stylesheet" type="text/css" href="css/basic.css">
    <link rel="stylesheet" type="text/css" href="index_style.css">
    <link rel="stylesheet" type="text/css" href="form_style.css">
    <link rel="stylesheet" type="text/css" href="device-map/device-map.css">
    <script type="text/javascript" src="js/jquery.js"></script>
    <script type="text/javascript" src="/js/httpApi.js"></script>
    <script type="text/javascript" src="state.js"></script>
    <script type="text/javascript" src="popup.js"></script>
    <script type="text/javascript" src="general.js"></script>
    <script type="text/javascript" src="help.js"></script>
    <script type="text/javascript" src="validator.js"></script>
    <script type="text/javascript" src="form.js"></script>
    <script type="text/javascript" src="switcherplugin/jquery.iphone-switch.js"></script>
    <script type="text/javascript" src="client_function.js"></script>
    <script language="JavaScript" type="text/javascript" src="/js/asus_policy.js?v=4"></script>
    <style>
        * {
            box-sizing: content-box;
        }

        #switch_menu {
            text-align: right
        }

        #switch_menu span {
            border-radius: 4px;
            font-size: 16px;
            padding: 3px;
        }


        .click:hover {
            box-shadow: 0px 0px 5px 3px white;
            background-color: #97CBFF;
        }

        .clicked {
            background-color: #2894FF;
            box-shadow: 0px 0px 5px 3px white;

        }

        .click {
            background: #8E8E8E;
        }
    </style>
</head>

<body onload="show_menu();" onunload="unload_body();" class="bg">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<div id="hiddenMask" class="popup_bg" style="z-index:999;">
    <table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center"></table>
    <!--[if lte IE 6.5]>
    <script>alert("<#ALERT_TO_CHANGE_BROWSER#>");</script><![endif]-->
</div>
<table class="content" align="center" cellpadding="0" cellspacing="0">
    <tr>
        <td width="17">&nbsp;</td>
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
                        <table width="730px" border="0" cellpadding="4" cellspacing="0" class="FormTitle"
                               id="FormTitle">
                            <tr>
                                <td bgcolor="#4D595D" valign="top">
                                    <div>&nbsp;</div>
                                    <!--div class="formfonttitle"></div-->
                                    <div style="margin-top:-5px;">
                                        <table width="730px">
                                            <tr>
                                                <td align="left">
                                                    <div class="formfonttitle" style="width:400px">
                                                        <#Parental_Control#> - <#AiProtection_filter#>
                                                    </div>
                                                </td>
                                            </tr>
                                        </table>
                                    </div>
                                    <div style="margin:0px 0px 10px 5px;" class="splitLine"></div>
                                    <div id="PC_desc">
                                        <table id="PC_desc_table" width="700px" style="margin-left:25px;">
                                            <tr>
                                                <td>
                                                    <img id="guest_image"
                                                         src="/images/New_ui/Web_Apps_Restriction.png">
                                                </td>
                                                <td>&nbsp;&nbsp;</td>
                                                <td style="font-size: 14px;">
                                                    <span><#AiProtection_filter_desc1#></span>
                                                    <ol>
                                                        <li><#AiProtection_filter_desc2#></li>
                                                        <li><#AiProtection_filter_desc3#></li>
                                                        <li><#AiProtection_filter_desc4#></li>
                                                    </ol>
                                                    <span><#AiProtection_filter_note#></span>
                                                    <div>
                                                        <a id="faq" class="notInBusiness" href=""
                                                           style="text-decoration:underline;" target="_blank"><#Parental_Control#>
                                                            FAQ</a>
                                                    </div>
                                                </td>
                                            </tr>
                                        </table>
                                    </div>
                                    <br>
                                    <!--=====Beginning of Main Content=====-->
                                    <table width="100%" border="1" align="center" cellpadding="4" cellspacing="0"
                                           bordercolor="#6b8fa3" class="FormTable">
                                        <tr>
                                            <th><#AiProtection_filter#></th>
                                            <td>
                                                <div align="center" class="left"
                                                     style="width:94px; float:left; cursor:pointer;"
                                                     id="radio_web_restrict_enable"></div>
                                                <div class="iphone_switch_container"
                                                     style="height:32px; width:74px; position: relative; overflow: hidden">
                                                </div>
                                            </td>
                                        </tr>
                                    </table>
                                    <table id="list_table" width="100%" border="0" align="center" cellpadding="0"
                                           cellspacing="0" style="display:none">
                                        <tr>
                                            <td valign="top" align="center">
                                                <div id="mainTable" style="margin-top:10px;"></div>
                                                <div id="ctrlBtn" class="apply_gen"
                                                     style="text-align:center;margin-top:20px;">
                                                    <input class="button_gen" type="button" onclick="applyRule();"
                                                           value="<#CTL_apply#>">
                                                </div>
                                            </td>
                                        </tr>
                                    </table>
                                </td>
                            </tr>
                        </table>
                    </td>
                </tr>
            </table>
            <!--===================================Ending of Main Content===========================================-->
        </td>
        <td width="10" align="center" valign="top">&nbsp;</td>
    </tr>
</table>

<div id="footer"></div>

<script>

    if (!isSupport("ark_iam")) {
        window.location.href = "AiProtection_WebProtector.asp";
    }

    let ark_iam_app_list_array = [];
    let ark_iam_ct_list_array = [];
    let ark_app_list = [];
    let ark_iam_rulelist = [];

    let appSelectorModal;
    let settingChecklist;

    class HierarchicalChecklist {
        #data;
        #lang;
        #hierarchy = [];
        #dataMap = new Map();
        #selectedItems = new Set();
        #expandedItems = new Set();

        #hostElement;
        #shadowRoot;
        #treeContainer;

        moreBtn;

        constructor(data, lang = 'EN') {
            this.#hostElement = document.createElement('div');
            this.#data = this.#filterDisplayItems(data);
            this.#lang = lang;
            this.#shadowRoot = this.#hostElement.attachShadow({mode: 'open'});
            this.#initComponent();
        }

        render() {
            return this.#hostElement;
        }

        getAppList() {
            return this.#data.app_list;
        }

        #initComponent() {
            const styleElement = document.createElement('style');
            styleElement.textContent = this.#getStyles();

            const wrapper = document.createElement('div');
            wrapper.className = 'component-wrapper';
            wrapper.innerHTML = `<div class="tree-container"></div>`;

            this.#shadowRoot.appendChild(styleElement);
            this.#shadowRoot.appendChild(wrapper);
            this.#treeContainer = this.#shadowRoot.querySelector('.tree-container');

            this.#buildHierarchy();
            this.#renderTree();

            this.#attachEventListeners();
        }

        /**
         * 遞歸過濾掉 display 為 false 的項目
         * @param {Object|Array} data - 要過濾的數據
         * @returns {Object|Array} - 過濾後的數據
         */
        #filterDisplayItems(data) {
            if (Array.isArray(data)) {
                return data
                    .filter(item => {
                        if (!item.hasOwnProperty('display')) return true;
                        return item.display !== false;
                    })
                    .map(item => this.#filterDisplayItems(item));
            }

            if (data && typeof data === 'object') {
                if (data.hasOwnProperty('display') && data.display === false) {
                    return null;
                }

                const filtered = {};

                for (const [key, value] of Object.entries(data)) {
                    if (key === 'category' && Array.isArray(value)) {
                        const filteredCategory = this.#filterDisplayItems(value);
                        if (filteredCategory.length > 0) {
                            filtered[key] = filteredCategory;
                        }
                    } else if (Array.isArray(value)) {
                        const filteredArray = this.#filterDisplayItems(value);
                        if (filteredArray.length > 0) {
                            filtered[key] = filteredArray;
                        }
                    } else if (value && typeof value === 'object') {
                        const filteredValue = this.#filterDisplayItems(value);
                        if (filteredValue !== null) {
                            filtered[key] = filteredValue;
                        }
                    } else {
                        filtered[key] = value;
                    }
                }

                return filtered;
            }

            return data;
        }

        #getNodeId(node) {
            const prefix = {
                main_category: 'mc_',
                category: 'c_',
                app: 'app_'
            };
            return prefix[node.type] + (node._id || node.app_id);
        }

        #buildHierarchy() {
            [...this.#data.main_category, ...this.#data.category, ...this.#data.app_list].forEach(item => {
                const node = {...item, children: []};
                if (item.app_id) node.type = 'app';
                else if (item.main_category_id) node.type = 'category';
                else node.type = 'main_category';
                node.uniqueId = this.#getNodeId(node);
                this.#dataMap.set(node.uniqueId, node);
            });
            this.#dataMap.forEach(node => {
                let parentId = null;
                if (node.type === 'category') parentId = 'mc_' + node.main_category_id;
                else if (node.type === 'app') {
                    for (const cat of this.#data.category) {
                        if (cat._id.split(',').includes(node.category_id)) {
                            parentId = 'c_' + cat._id;
                            break;
                        }
                    }
                }
                if (parentId && this.#dataMap.has(parentId)) {
                    this.#dataMap.get(parentId).children.push(node);
                }
            });
            this.#hierarchy = this.#data.main_category.map(mc => this.#dataMap.get('mc_' + mc._id));
        }

        #renderTree() {
            const moreBtn = document.createElement('div');
            moreBtn.className = 'tree-content more-btn';
            moreBtn.innerHTML = `<span class="item-name"><b><#More_Apps#> ></b></span>`;
            const li = document.createElement('li');
            li.className = 'tree-item main_category';
            li.appendChild(moreBtn);

            this.#treeContainer.innerHTML = `<ul class="tree-list">${this.#renderNodes(this.#hierarchy)}</ul>`;
            this.#treeContainer.querySelector('ul').append(li);
            this.#updateCheckboxStates();
            this.#updateSelectedList();
            moreBtn.addEventListener("click", () => {
                this.#showAppSelector();
            });
            this.moreBtn = moreBtn;
        }

        #showAppSelector() {
            appSelectorModal.setTarget(this);
            appSelectorModal.show(this.getSelectedAppId());
        }

        #renderNodes(nodes) {
            return nodes.map(node => {
                const hasChildren = node.children.length > 0;
                const isExpanded = this.#expandedItems.has(node.uniqueId);

                const isApp = typeof node.app_id !== 'undefined';
                if ((isApp && node.isHot) || !isApp) {
                    return `
                <li class="tree-item ${node.type}" data-id="${node.uniqueId}">
                    <div class="tree-content">
                        <span class="expand-icon ${isExpanded ? 'expanded' : ''} ${!hasChildren ? 'no-children' : ''}">▶</span>
                        <input type="checkbox" class="checkbox" data-id="${node.uniqueId}">
                        <span class="item-name">${this.#getDisplayName(node)}</span>
                    </div>
                    ${hasChildren ? `<div class="children ${isExpanded ? 'expanded' : 'collapsed'}"><ul class="tree-list">${this.#renderNodes(node.children)}</ul></div>` : ''}
                </li>`;
                }
            }).join('');
        }

        #attachEventListeners() {
            this.#shadowRoot.addEventListener('click', (event) => {
                const target = event.target;

                if (target.matches('.checkbox')) {
                    const uniqueId = target.closest('.tree-item')?.dataset.id;
                    if (uniqueId) this.#handleCheckboxClick(uniqueId, target.checked);
                } else if (target.matches('.tree-content')) {
                    const uniqueId = target.closest('.tree-item')?.dataset.id;
                    if (uniqueId) {
                        const node = this.#dataMap.get(uniqueId);
                        if (node && node.children.length === 0) {
                            const checkbox = this.#shadowRoot.querySelector(`.checkbox[data-id="${uniqueId}"]`);
                            if (checkbox) {
                                checkbox.checked = !checkbox.checked;
                                this.#handleCheckboxClick(uniqueId, checkbox.checked);
                            }
                        } else {
                            this.#toggleExpand(uniqueId);
                        }
                    }
                }
            });
        }

        #handleCheckboxClick(uniqueId, isChecked) {
            const descendants = this.#getDescendants(uniqueId);
            [uniqueId, ...descendants.map(d => d.uniqueId)].forEach(id => {
                isChecked ? this.#selectedItems.add(id) : this.#selectedItems.delete(id);
            });
            this.#updateParentStates(uniqueId);
            this.#updateCheckboxStates();
            this.#updateSelectedList();
        }

        #toggleExpand(uniqueId) {
            const node = this.#dataMap.get(uniqueId);
            const isCurrentlyExpanded = this.#expandedItems.has(uniqueId);

            // 如果要展開這個節點，先收起同級的其他節點
            if (!isCurrentlyExpanded && node.children.length > 0) {
                this.#collapseSiblings(uniqueId);
            }

            // 切換當前節點的展開狀態
            isCurrentlyExpanded ? this.#expandedItems.delete(uniqueId) : this.#expandedItems.add(uniqueId);

            const itemElement = this.#shadowRoot.querySelector(`.tree-item[data-id="${uniqueId}"]`);
            if (itemElement) {
                itemElement.querySelector('.expand-icon')?.classList.toggle('expanded');
                const childrenContainer = itemElement.querySelector('.children');
                if (childrenContainer) {
                    childrenContainer.classList.toggle('expanded');
                    childrenContainer.classList.toggle('collapsed');
                }
            }
        }

        #collapseSiblings(uniqueId) {
            const parent = this.#findParent(uniqueId);
            const siblings = parent ? parent.children : this.#hierarchy;

            siblings.forEach(sibling => {
                if (sibling.uniqueId !== uniqueId && sibling.children.length > 0) {
                    // 收起同級節點
                    this.#expandedItems.delete(sibling.uniqueId);

                    const siblingElement = this.#shadowRoot.querySelector(`.tree-item[data-id="${sibling.uniqueId}"]`);
                    if (siblingElement) {
                        const expandIcon = siblingElement.querySelector('.expand-icon');
                        const childrenContainer = siblingElement.querySelector('.children');

                        if (expandIcon) expandIcon.classList.remove('expanded');
                        if (childrenContainer) {
                            childrenContainer.classList.remove('expanded');
                            childrenContainer.classList.add('collapsed');
                        }
                    }

                    this.#collapseAllDescendants(sibling.uniqueId);
                }
            });
        }

        #collapseAllDescendants(parentId) {
            const descendants = this.#getDescendants(parentId);
            descendants.forEach(descendant => {
                if (descendant.children.length > 0) {
                    this.#expandedItems.delete(descendant.uniqueId);

                    const descendantElement = this.#shadowRoot.querySelector(`.tree-item[data-id="${descendant.uniqueId}"]`);
                    if (descendantElement) {
                        const expandIcon = descendantElement.querySelector('.expand-icon');
                        const childrenContainer = descendantElement.querySelector('.children');

                        if (expandIcon) expandIcon.classList.remove('expanded');
                        if (childrenContainer) {
                            childrenContainer.classList.remove('expanded');
                            childrenContainer.classList.add('collapsed');
                        }
                    }
                }
            });
        }

        #updateCheckboxStates() {
            this.#dataMap.forEach(node => {
                const checkbox = this.#shadowRoot.querySelector(`.checkbox[data-id="${node.uniqueId}"]`);
                if (!checkbox) return;
                if (node.children.length > 0) {
                    const descendantLeafs = this.#getDescendants(node.uniqueId, true);
                    const selectedLeafs = descendantLeafs.filter(leaf => this.#selectedItems.has(leaf.uniqueId));
                    if (selectedLeafs.length === 0) {
                        checkbox.checked = false;
                        checkbox.indeterminate = false;
                    } else if (selectedLeafs.length === descendantLeafs.length && descendantLeafs.length > 0) {
                        checkbox.checked = true;
                        checkbox.indeterminate = false;
                    } else {
                        checkbox.checked = false;
                        checkbox.indeterminate = true;
                    }
                } else {
                    checkbox.checked = this.#selectedItems.has(node.uniqueId);
                    checkbox.indeterminate = false;
                }
            });
        }

        #updateParentStates(startNodeId) {
            let parent = this.#findParent(startNodeId);
            while (parent) {
                const allChildrenSelected = parent.children.every(child => this.#selectedItems.has(child.uniqueId));
                allChildrenSelected ? this.#selectedItems.add(parent.uniqueId) : this.#selectedItems.delete(parent.uniqueId);
                parent = this.#findParent(parent.uniqueId);
            }
        }

        #updateSelectedList() {
            const selectedNames = [];
            this.#selectedItems.forEach(id => {
                const node = this.#dataMap.get(id);
                if (node?.type === 'app') {
                    selectedNames.push(this.#getDisplayName(node));
                }
            });
        }

        #getDisplayName(node) {
            const {name, language = {}} = node;
            const enName = language.EN;
            const langName = language[this.#lang];
            return this.#lang === "EN"
                ? enName || name
                : langName
                    ? `${langName}(${enName || name})`
                    : enName || name;
        }

        #getDescendants(uniqueId, leafsOnly = false) {
            const found = [];
            const node = this.#dataMap.get(uniqueId);
            if (node?.children) {
                for (const child of node.children) {
                    if (!leafsOnly || child.children.length === 0) {
                        found.push(child);
                    }
                    found.push(...this.#getDescendants(child.uniqueId, leafsOnly));
                }
            }
            return found;
        }

        #findParent(childId) {
            for (const node of this.#dataMap.values()) {
                if (node.children.some(child => child.uniqueId === childId)) {
                    return node;
                }
            }
            return null;
        }

        #getStyles() {
            return `
            .component-wrapper { padding: 5px; }
            h2 { color: #333; }
            .tree-list { list-style: none; padding: 0; margin: 0; }
            .tree-item { margin: 5px 0; }
            .tree-content { display: flex; align-items: center; padding: 8px 12px; border-radius: 4px; cursor: pointer; transition: background-color 0.2s; }
            .main-category > .tree-content { background-color: #f7f7f7; font-weight: bold; font-size: 16px; }
            .category > .tree-content { background-color: #f7f7f7; margin-left: 20px; font-weight: 500; }
            .app > .tree-content { margin-left: 40px; background-color: #f7f7f7; }
            .tree-content:hover { background-color: #f0f0f0; }
            .expand-icon { width: 16px; height: 16px; margin-right: 8px; display: flex; align-items: center; justify-content: center; transition: transform 0.2s; font-size: 10px; pointer-events: none; }
            .expand-icon.expanded { transform: rotate(90deg); }
            .expand-icon.no-children { visibility: hidden; }
            .checkbox { margin-right: 8px; transform: scale(1.2); cursor: pointer; }
            .item-name { pointer-events: none; user-select: none; }
            .hot-badge { background-color: #ff4444; color: white; padding: 2px 6px; border-radius: 10px; font-size: 10px; margin-left: 8px; pointer-events: none; }
            .children { overflow: hidden; transition: max-height 0.3s ease-in-out; }
            .children.collapsed { max-height: 0; }
            .children.expanded { max-height: 2000px; }
            .selected-items { margin-top: 20px; padding: 15px; background-color: #f8f9fa; border-radius: 6px; border: 1px solid #dee2e6; }
            .selected-items h3 { margin-top: 0; color: #495057; font-size: 1em; }
            .selected-list { display: flex; flex-wrap: wrap; gap: 8px; }
            .selected-tag { background-color: #007bff; color: white; padding: 4px 8px; border-radius: 16px; font-size: 12px; }
        `;
        }

        setSelectedAppId(appIds) {
            this.#selectedItems = new Set([...this.#selectedItems].filter(item => !item.startsWith("app_")));

            if (Array.isArray(appIds)) {
                appIds.forEach(appId => {
                    const nodeId = 'app_' + appId;
                    const node = this.#dataMap.get(nodeId);

                    if (node) {
                        this.#selectedItems.add(nodeId);
                    }
                });
            }

            this.#updateAllParentStates();

            this.#updateCheckboxStates();
            this.#updateSelectedList();
        }

        setSelectedCategoryId(categoryIds) {
            // 清除之前的選擇
            // this.#selectedItems.clear();

            // 處理傳入的 Category ID 陣列
            if (Array.isArray(categoryIds)) {
                categoryIds.forEach(catId => {
                    const nodeId = 'c_' + catId;
                    if (this.#dataMap.has(nodeId)) {
                        this.#selectedItems.add(nodeId);
                    }
                });
            }

            this.#updateAllParentStates();
            this.#updateCheckboxStates();
            this.#updateSelectedList();
        }

        // 新增：更新所有父節點狀態的輔助方法
        #updateAllParentStates() {
            this.#selectedItems.forEach(selectedId => {
                this.#updateParentStates(selectedId);
            });
        }

        getSelectedAppId() {
            const selectedAppId = [];

            this.#selectedItems.forEach(id => {
                const node = this.#dataMap.get(id);
                if (node?.type === 'app') {
                    selectedAppId.push(node.app_id);
                }
            });

            return selectedAppId;
        }

        getSelectedApp() {
            const selectedApps = [];

            this.#selectedItems.forEach(id => {
                const node = this.#dataMap.get(id);
                if (node?.type === 'app') {
                    selectedApps.push(node);
                }
            });

            return selectedApps;
        }

        getSelectedCategory() {
            const selectedCategory = [];

            this.#selectedItems.forEach(id => {
                const node = this.#dataMap.get(id);
                if (node?.type === 'category') {
                    selectedCategory.push(node);
                }
            });

            return selectedCategory;
        }

        getSelectedCategoryId() {
            const selectedCategoryId = [];
            this.#selectedItems.forEach(id => {
                const node = this.#dataMap.get(id);
                if (node?.type === 'category') {
                    selectedCategoryId.push(node._id);
                }
            });
            return selectedCategoryId;
        }

        clearSelection() {
            this.#selectedItems.clear();
            this.#updateCheckboxStates();
            this.#updateSelectedList();
            this.#renderTree();
        }

        isAppSelected(appId) {
            const nodeId = 'app_' + appId;
            return this.#selectedItems.has(nodeId);
        }
    }

    function pullLANIPList(obj) {
        var element = document.getElementById('ClientList_Block_PC');
        var isMenuopen = element.offsetWidth > 0 || element.offsetHeight > 0;
        if (isMenuopen == 0) {
            obj.src = "/images/unfold_less.svg"
            element.style.display = 'block';
            document.querySelector("#webProtectClient").focus();
        } else
            hideClients_Block();
    }

    function hideClients_Block() {
        document.getElementById("pull_arrow").src = "/images/unfold_more.svg";
        document.getElementById('ClientList_Block_PC').style.display = 'none';
    }

    function setClientIP(macaddr) {
        document.querySelector("#webProtectClient").value = macaddr;
        hideClients_Block();
    }

    function deleteRow_main(obj) {
        const item_index = obj.parentNode.parentNode.rowIndex;
        const target_mac = obj.dataset.mac;
        ark_iam_rulelist.filter(item => item.target === target_mac).forEach(item => {
            const index = ark_iam_rulelist.indexOf(item);
            if (index > -1) {
                ark_iam_rulelist.splice(index, 1);
            }
        });
        document.getElementById(obj.parentNode.parentNode.parentNode.parentNode.id).deleteRow(item_index);
    }

    function check_macaddr(obj, flag) { //control hint of input mac address
        if (flag == 1) {
            var childsel = document.createElement("div");
            childsel.setAttribute("id", "check_mac");
            childsel.style.color = "#FFCC00";
            obj.parentNode.appendChild(childsel);
            document.getElementById("check_mac").innerHTML = "<#LANHostConfig_ManualDHCPMacaddr_itemdesc#>";
            document.getElementById("check_mac").style.display = "";
            return false;
        } else if (flag == 2) {
            var childsel = document.createElement("div");
            childsel.setAttribute("id", "check_mac");
            childsel.style.color = "#FFCC00";
            obj.parentNode.appendChild(childsel);
            document.getElementById("check_mac").innerHTML = "<#IPConnection_x_illegal_mac#>";
            document.getElementById("check_mac").style.display = "";
            return false;
        } else {
            document.getElementById("check_mac") ? document.getElementById("check_mac").style.display = "none" : true;
            return true;
        }
    }

    function addRow_main(obj) {
        const new_rule = new ContentFilterRule()
        const enable_checkbox = $(obj.parentNode).siblings()[0].children[0];
        new_rule.enable = enable_checkbox.checked ? true : false;

        const targetClient = document.querySelector("#webProtectClient");

        if (ark_iam_rulelist.find(item => item.target === targetClient.value.toUpperCase())) {
            alert("已經有這個裝置的規則，請先刪除再新增。");
            return false;
        }

        const upper = MaxRule_bwdpi_wrs > 0 ? MaxRule_bwdpi_wrs : 16;
        //check max limit of rule list
        if (ark_iam_rulelist.length >= upper) {
            alert("<#JS_itemlimit1#> " + upper + " <#JS_itemlimit2#>");
            targetClient.focus();
            targetClient.select();
            return false;
        }

        if (targetClient.value == "") {
            alert("<#JS_fieldblank#>");
            targetClient.focus();
            return false;
        } else if (!check_macaddr(targetClient, check_hwaddr_flag(targetClient, 'inner'))) {
            targetClient.focus();
            targetClient.select();
            return false;
        }

        if (settingChecklist.getSelectedAppId().length == 0 && settingChecklist.getSelectedCategoryId().length == 0) {
            alert("<#AiProtection_Category_Alert#>");
            return false;
        }


        new_rule.target = document.querySelector("#webProtectClient").value.toUpperCase()
        new_rule.apps = settingChecklist.getSelectedAppId();
        new_rule.category = settingChecklist.getSelectedCategoryId();
        const hierarchicalChecklist = new HierarchicalChecklist(ark_app_list, window.ui_lang);
        hierarchicalChecklist.setSelectedAppId(new_rule.apps);
        hierarchicalChecklist.setSelectedCategoryId(new_rule.category);
        hierarchicalChecklist.moreBtn.querySelector(".item-name").innerHTML = `<b><#More_Apps#> (${new_rule.apps.length} <#Selected#>) ></b>`;
        new_rule.checklist = hierarchicalChecklist;

        ark_iam_rulelist.push(new_rule);
        document.querySelector("#webProtectClient").value = "";
        settingChecklist.clearSelection();
        genRulesRow(ark_iam_rulelist);
    }

    function genMain_table() {
        let code = "";
        const clientListEventData = [];
        code += '<table width="100%" border="1" cellspacing="0" cellpadding="4" align="center" class="FormTable_table" id="mainTable_table">';
        code += '<thead><tr>';
        code += '<td colspan="5"><#ConnectedClient#>&nbsp;(<#List_limit#>&nbsp;' + MaxRule_bwdpi_wrs + ')</td>';
        code += '</tr></thead>';
        code += '<tbody>';
        code += '<tr>';
        code += '<th width="5%" height="30px" title="<#select_all#>">';
        code += '<input id="selAll" type="checkbox" onclick="selectAll(this);" value="">';
        code += '</th>';
        code += '<th width="40%"><#Client_Name#> (<#PPPConnection_x_MacAddressForISP_itemname#>)</th>';
        code += '<th width="40%"><#AiProtection_filter_category#></th>';
        code += '<th width="10%"><#list_add_delete#></th>';
        code += '</tr>';
        code += '<tr id="main_element">';
        code += '<td style="border-bottom:2px solid #000;" title="<#WLANConfig11b_WirelessCtrl_button1name#>/<#btn_disable#>">';
        code += '<input type="checkbox" checked="">';
        code += '</td>';
        code += '<td style="border-bottom:2px solid #000;">';
        code += '<div style="display: flex; justify-content: center">';
        code += '<div class="clientlist_dropdown_main">';
        code += '<input type="text" maxlength="17" class="input_20_table" id="webProtectClient" name="PC_devicename" onkeypress="return validator.isHWAddr(this,event)" onclick="hideClients_Block();" placeholder="ex: <% nvram_get("lan_hwaddr"); %>" autocorrect="off" autocapitalize="off">';
        code += '<img id="pull_arrow" height="14px;" src="/images/unfold_more.svg" onclick="pullLANIPList(this);" title="<#select_client#>">';
        code += '<div id="ClientList_Block_PC" class="clientlist_dropdown"></div>';
        code += '</div>';
        code += '</div>';
        code += '</td>';
        code += '<td style="border-bottom:2px solid #000;text-align:left;">';
        code += '<div id="tree-container"></div>'
        code += '</td>';
        code += '<td style="border-bottom:2px solid #000;"><input class="add_btn" type="button" onclick="addRow_main(this)" value=""></td>';
        code += '</tr>';
        code += '</tbody>';
        code += '</table>';
        document.getElementById('mainTable').innerHTML = code;
        for (var i = 0; i < clientListEventData.length; i += 1) {
            var clientIconID = "clientIcon_" + clientListEventData[i].mac.replace(/\:/g, "");
            var clientIconObj = $("#mainTable").children("#mainTable_table").find("#" + clientIconID + "")[0];
            var paramData = JSON.parse(JSON.stringify(clientListEventData[i]));
            paramData["obj"] = clientIconObj;
            $("#mainTable").children("#mainTable_table").find("#" + clientIconID + "").click(paramData, popClientListEditTable);
        }
        showDropdownClientList('setClientIP', 'mac', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');
    }

    function selectAll(target) {
        const table = document.getElementById('mainTable_table');
        const tbody = table.querySelector('tbody');
        const rows = tbody.querySelectorAll('tr');
        const isChecked = target.checked;
        rows.forEach((row, index) => {
            if (index > 1) { // Skip the header row
                const checkbox = row.querySelector('input[type="checkbox"]');
                if (checkbox) {
                    checkbox.checked = isChecked;
                    ark_iam_rulelist[index - 2].enable = isChecked; // Adjust index for rule list
                }
            }
        });
    }

    function genRulesRow(ark_iam_rulelist) {
        const table = document.getElementById('mainTable_table');
        const tbody = table.querySelector('tbody');
        if (tbody) {
            const rows = tbody.querySelectorAll('tr');
            rows.forEach((row, index) => {
                if (index > 1) {
                    row.remove();
                }
            });
        }
        if (ark_iam_rulelist.length === 0) {
            const row = tbody.insertRow(-1)
            const cell1 = row.insertCell(0);
            cell1.classList.add("tableNoRule");
            cell1.style.color = "#FFCC00";
            cell1.colSpan = 10;
            cell1.innerHTML = `<#IPConnection_VSList_Norule#>`;
        } else {
            for (const rule of ark_iam_rulelist) {
                const row = tbody.insertRow(-1)
                const cell1 = row.insertCell(0);
                const cell2 = row.insertCell(1);
                const cell3 = row.insertCell(2);
                const cell4 = row.insertCell(3);

                /*End exception*/
                //user icon
                let userIconBase64 = "NoIcon";
                let clientName, clientIP, deviceType, deviceVendor;
                let clientMac = rule.target.toUpperCase();
                let clientIconID = "clientIcon_" + clientMac.replace(/\:/g, "");
                let clientObj = clientList[clientMac];
                if (clientObj) {
                    clientName = (clientObj.nickName == "") ? clientObj.name : clientObj.nickName;
                    clientIP = clientObj.ip;
                    deviceType = clientObj.type;
                    deviceVendor = clientObj.vendor;
                } else {
                    clientName = clientMac;
                    clientIP = "offline";
                    deviceType = 0;
                    deviceVendor = "";
                }

                cell1.title = `<#WLANConfig11b_WirelessCtrl_button1name#>/<#btn_disable#>`;
                cell1.innerHTML = `<input type="checkbox" ${(rule.enable) ? "checked" : ""}>`

                cell1.querySelector('input[type="checkbox"]').addEventListener('change', function () {
                    rule.enable = this.checked;
                });

                const clientInfoDiv = document.createElement('div');
                clientInfoDiv.style = "display: flex; justify-content: center; align-items: center; height: 100%;";
                if (clientObj == undefined) {
                    clientInfoDiv.innerHTML = '<div id="' + clientIconID + '" class="clientIcon" style="margin: 0"><i class="type0"></i></div>';
                    const clientNameDiv = document.createElement('div');
                    clientNameDiv.style = "display: flex; flex-direction: column; justify-content: center; align-items: flex-start; margin-left: 10px;";
                    clientNameDiv.innerHTML = `<div style="font-weight: bold;">${clientName}</div><div style="font-size: 12px; color: #666;">${clientMac}</div>`;
                    clientInfoDiv.appendChild(clientNameDiv);
                    cell2.appendChild(clientInfoDiv);
                } else {
                    let code = "";
                    if (usericon_support) {
                        userIconBase64 = getUploadIcon(clientMac.replace(/\:/g, ""));
                    }
                    if (userIconBase64 != "NoIcon") {
                        if (clientList[clientMac].isUserUplaodImg) {
                            code += '<div id="' + clientIconID + '" class="clientIcon" style="margin: 0"><img class="imgUserIcon_card" src="' + userIconBase64 + '"></div>';
                        } else {
                            code += '<div id="' + clientIconID + '" class="clientIcon" style="margin: 0"><i class="type" style="--svg:url(' + userIconBase64 + ')"></i></div>';
                        }
                    } else if (deviceType != "0" || deviceVendor == "") {
                        code += '<div id="' + clientIconID + '" class="clientIcon" style="margin: 0"><i class="type' + deviceType + '"></i></div>';
                    } else if (deviceVendor != "") {
                        var vendorIconClassName = getVendorIconClassName(deviceVendor.toLowerCase());
                        if (vendorIconClassName != "" && !downsize_4m_support) {
                            code += '<div id="' + clientIconID + '" class="clientIcon" style="margin: 0"><i class="vendor-icon ' + vendorIconClassName + '"></i></div>';
                        } else {
                            code += '<div id="' + clientIconID + '" class="clientIcon" style="margin: 0"><i class="type' + deviceType + '"></i></div>';
                        }
                    }
                    const clientNameDiv = document.createElement('div');
                    clientNameDiv.style = "display: flex; flex-direction: column; justify-content: center; align-items: flex-start; margin-left: 10px;";
                    clientNameDiv.innerHTML = `<div style="font-weight: bold;">${clientName}</div><div style="font-size: 12px; color: #666;">${clientMac}</div>`;

                    clientInfoDiv.innerHTML = code;
                    clientInfoDiv.appendChild(clientNameDiv);
                    cell2.appendChild(clientInfoDiv);
                }
                const checklist = document.createElement('div');
                checklist.className = "ruleDiv";
                checklist.setAttribute("data-rule-mac", clientMac);
                checklist.appendChild(rule.checklist.render())
                rule.checklist.moreBtn.querySelector(".item-name").innerHTML = `<b><#More_Apps#> (${rule.apps.length} <#Selected#>) ></b>`;
                cell3.appendChild(checklist);
                cell4.innerHTML = `<input class="remove_btn" type="button" data-mac="${clientMac}" onclick="deleteRow_main(this);">`

                // if (validator.mac_addr(clientMac))
                //     clientListEventData.push({
                //         "mac": clientMac,
                //         "name": clientName,
                //         "ip": clientIP,
                //         "callBack": "WebProtector"
                //     });
            }
        }


    }

    function applyRule() {
        const ruleList = ark_iam_rulelist.map(rule => {
            const selectedApps = rule.checklist.getSelectedApp();
            const selectedCategory = rule.checklist.getSelectedCategory();
            const [ids1, ids2, ids3, ids4] = ['1', '2', '3', '4'].map(categoryId =>
                selectedCategory
                    .filter(item => item.main_category_id === categoryId)
                    .map(item => item._id)
            );
            const selectedCategoryIds = rule.checklist.getSelectedCategoryId();
            const categorySet = new Set(selectedCategoryIds);
            const validApps = [];

            for (const app of selectedApps) {
                if (!categorySet.has(app.category_id)) {
                    validApps.push(app);
                } else {
                    const index = rule.apps.indexOf(app.app_id);
                    if (index > -1) {
                        rule.apps.splice(index, 1);
                    }
                }
            }
            return {
                enable: rule.enable ? "1" : "0",
                target: rule.target,
                category: `${ids1.join(",")}>${ids2.join(",")}>${ids3.join(",")}>${ids4.join(",")}`.trim(),
                apps: validApps.map(app => app.app_id).join(",").trim(),
            };
        });

        const appRuleList = ruleList.filter(rule => rule.apps !== "");

        const chunkSize = 16;
        const chunks = [];
        for (let i = 0; i < appRuleList.length; i += chunkSize) {
            chunks.push(appRuleList.slice(i, i + chunkSize));
        }

        while (chunks.length < 4) {
            chunks.push([]);
        }

        const ark_iam_app_list1 = chunks[0].length > 0 ?
            chunks[0].map(rule => `${rule.enable}>${rule.target}>${rule.apps}`).join('<') : '';
        const ark_iam_app_list2 = chunks[1].length > 0 ?
            chunks[1].map(rule => `${rule.enable}>${rule.target}>${rule.apps}`).join('<') : '';
        const ark_iam_app_list3 = chunks[2].length > 0 ?
            chunks[2].map(rule => `${rule.enable}>${rule.target}>${rule.apps}`).join('<') : '';
        const ark_iam_app_list4 = chunks[3].length > 0 ?
            chunks[3].map(rule => `${rule.enable}>${rule.target}>${rule.apps}`).join('<') : '';

        const ark_iam_ct_list = ruleList.map(rule => `${rule.enable}>${rule.target}>${rule.category}`).join('<');

        httpApi.nvramSet({
            "ark_iam_enable": "1",
            "ark_iam_ct_list": ark_iam_ct_list,
            "ark_iam_app_list1": ark_iam_app_list1,
            "ark_iam_app_list2": ark_iam_app_list2,
            "ark_iam_app_list3": ark_iam_app_list3,
            "ark_iam_app_list4": ark_iam_app_list4,
            "action_mode": "apply",
            "rc_service": "restart_ark",
        });
        showLoading(3);
    }

    /**
     * AppSelectorModal Class
     * A highly performant modal for selecting applications from a categorized list.
     * Features: total selected count, dual-list search, virtual scrolling,
     * category-based selection, and expand/collapse all functionality.
     */
    class AppSelectorModal {
        #options;
        #categorizedApps;
        #selectedApps;
        #appMap;
        #collapsedCategories;
        #collapsedSelectedCategories;
        #searchTerm;
        #element;
        #shadowRoot;

        // DOM Elements
        #availableCategoriesDiv;
        #selectedCategoriesDiv;
        #searchInput;
        #searchClearBtn;
        #closeButton;
        #cancelButton;
        #submitButton;
        #modalOverlay;
        #appListsContainer;
        #selectedAppsTitleElement; // NEW: To hold the H3 element for the selected apps title

        #target;

        // Virtual Scroll Properties
        #itemHeight = 41;

        constructor(options) {
            this.#options = {
                id: options.id || 'app-selector-modal',
                modalSize: options.modalSize || '',
                initialApps: options.initialApps || [],
                onSelect: options.onSelect || (() => {
                }),
                modalTitle: options.modalTitle || `<#More_Apps#>`,
                availableAppsTitle: options.availableAppsTitle || `<#Available_Apps#>`,
                selectedAppsTitle: options.selectedAppsTitle || `<#Selected_Apps#>`,
                searchPlaceholder: options.searchPlaceholder || `<#Search_for_apps#>`,
                submitButtonText: options.submitButtonText || '<#CTL_finished#>',
                cancelButtonText: options.cancelButtonText || '<#CTL_Cancel#>',
            };

            this.#categorizedApps = this.#categorizeApps(this.#options.initialApps);
            this.#appMap = new Map(this.#options.initialApps.map(app => [app.id, app]));
            this.#selectedApps = [];
            this.#collapsedCategories = new Set();
            this.#collapsedSelectedCategories = new Set();
            this.#searchTerm = '';

            this.#initModal();
            this.#injectStyles();
        }

        #categorizeApps(apps) {
            const categorized = {};
            apps.forEach(app => {
                const category = app.category || 'Other';
                if (!categorized[category]) {
                    categorized[category] = [];
                }
                categorized[category].push(app);
            });
            return categorized;
        }

        #initModal() {
            this.#element = document.createElement('div');
            this.#element.id = this.#options.id;
            this.#shadowRoot = this.#element.attachShadow({mode: 'open'});

            const template = document.createElement('template');
            template.innerHTML = `
            <div class="modal-overlay" role="dialog" aria-modal="true" aria-labelledby="modal-title">
                <div class="modal ${this.#options.modalSize}">
                    <div class="modal-content">
                        <div class="modal-header">
                            <div class="modal-title" id="modal-title">${this.#options.modalTitle}</div>
                            <button class="modal-close" aria-label="關閉視窗">✕</button>
                        </div>
                        <div class="modal-body">
                            <div class="search-container">
                                <div class="search-input-wrapper">
                                    <input type="text" class="search-input" placeholder="${this.#options.searchPlaceholder}">
                                    <button class="search-clear-btn" aria-label="清除搜尋">✕</button>
                                </div>
                            </div>
                            <div class="app-lists-container">
                                <div class="app-list-section available-apps">
                                    <div class="app-list-header">
                                        <h3>${this.#options.availableAppsTitle}</h3>
                                        <div class="list-actions">
                                            <button class="list-action-btn" data-action="expand-all" data-target="available"><#Expand_All#></button>
                                            <button class="list-action-btn" data-action="collapse-all" data-target="available"><#Collapse_All#></button>
                                        </div>
                                    </div>
                                    <div class="categories-container" id="available-categories"></div>
                                </div>
                                <div class="app-list-section selected-apps">
                                    <div class="app-list-header">
                                        <h3 id="selected-apps-title">${this.#options.selectedAppsTitle}</h3>
                                        <div class="list-actions">
                                            <button class="list-action-btn" data-action="expand-all" data-target="selected"><#Expand_All#></button>
                                            <button class="list-action-btn" data-action="collapse-all" data-target="selected"><#Collapse_All#></button>
                                        </div>
                                    </div>
                                    <div class="categories-container" id="selected-categories"></div>
                                </div>
                            </div>
                        </div>
                        <div class="modal-footer">
                            <button class="cancel-button">${this.#options.cancelButtonText}</button>
                            <button class="submit-button">${this.#options.submitButtonText}</button>
                        </div>
                    </div>
                </div>
            </div>
        `;

            this.#shadowRoot.appendChild(template.content.cloneNode(true));

            const S = this.#shadowRoot.querySelector.bind(this.#shadowRoot);
            this.#modalOverlay = S('.modal-overlay');
            this.#appListsContainer = S('.app-lists-container');
            this.#availableCategoriesDiv = S('#available-categories');
            this.#selectedCategoriesDiv = S('#selected-categories');
            this.#searchInput = S('.search-input');
            this.#searchClearBtn = S('.search-clear-btn');
            this.#closeButton = S('.modal-close');
            this.#cancelButton = S('.cancel-button');
            this.#submitButton = S('.submit-button');
            // NEW: Cache the title element
            this.#selectedAppsTitleElement = S('#selected-apps-title');

            this.#addEventListeners();
        }

        #injectStyles() {
            const style = document.createElement('style');
            style.textContent = `
            @import url('/css/color-table.css');
            :host { --item-height: ${this.#itemHeight}px; }
            .modal-overlay { position: fixed; top: 0; left: 0; width: 100%; height: 100%; z-index: 2000; background: var(--mask-popup-bg-color, rgba(0,0,0,0.5)); display: none; align-items: center; justify-content: center; }
            .modal { width: 90%; max-width: 1000px; max-height: 90vh; background: var(--popup-bg-color, #fff); border-radius: var(--global-radius, 8px); box-shadow: var(--shadow-elevation40, 0 8px 24px rgba(0,0,0,0.2)); display: flex; flex-direction: column; }
            .modal-content { display: flex; flex-direction: column; height: 100%; overflow: hidden; }
            .modal-header { display: flex; justify-content: space-between; align-items: center; padding: 20px; border-bottom: 1px solid var(--border-color, #e0e0e0); }
            .modal-title { font-size: 18px; font-weight: 600; color: var(--body-text-color, #333); }
            .modal-close { cursor: pointer; font-size: 24px; color: #666; padding: 4px; border-radius: var(--global-radius, 8px); transition: background-color 0.2s; border: none; background: transparent; }
            .modal-close:hover { background-color: var(--hover-color, #f0f0f0); }
            .modal-body { flex: 1; padding: 20px; overflow: hidden; display: flex; flex-direction: column; }
            .search-container { margin-bottom: 20px; }
            .search-input-wrapper { position: relative; display: flex; align-items: center; }
            .search-input { width: 100%; padding: 12px 40px 12px 12px; border: 1px solid var(--textbox-normal-stroke-color, #ccc); background: var(--textbox-bg-color, #fff); border-radius: var(--global-radius, 8px); font-size: 14px; box-sizing: border-box; transition: all 0.2s ease; }
            .search-input:focus { outline: none; border: 1px solid var(--textbox-EL10-normal-stroke-color, #007bff); color: var(--text-input-color, #333); box-shadow: 0px -4px 0px 0px var(--textbox-focus-stroke-color, #007bff) inset; background-color: var(--textbox-EL10-focus-fill-color, #fff) !important; }
            .search-clear-btn { position: absolute; right: 8px; background: none; border: none; font-size: 16px; color: #666; cursor: pointer; padding: 4px; border-radius: 50%; width: 24px; height: 24px; display: flex; align-items: center; justify-content: center; transition: background-color 0.2s, opacity 0.2s, transform 0.2s; opacity: 0; transform: scale(0.7); pointer-events: none; }
            .search-clear-btn.visible { opacity: 1; transform: scale(1); pointer-events: auto; }
            .search-clear-btn:hover { background-color: var(--hover-color, #f0f0f0); color: #333; }
            .app-lists-container { display: flex; gap: 20px; flex: 1; overflow: hidden; }
            .app-list-section { flex: 1; display: flex; flex-direction: column; overflow: hidden; }
            .app-list-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 15px; }
            .app-list-header h3 { margin: 0; color: var(--body-text-color, #333); font-size: 16px; font-weight: 600; }
            .list-actions { display: flex; gap: 8px; }
            .list-action-btn { background: none; border: none; color: var(--primary-60, #007bff); font-size: 13px; cursor: pointer; padding: 4px 6px; border-radius: var(--global-radius, 8px); transition: background-color 0.2s; }
            .list-action-btn:hover { background-color: var(--hover-color, #f0f0f0); }
            .categories-container { flex: 1; overflow-y: auto; border: 1px solid var(--textbox-normal-stroke-color, #ccc); border-radius: var(--global-radius, 8px); background: #fff; }
            .category-section { border-bottom: 1px solid var(--textbox-normal-stroke-color, #ccc); }
            .category-section:last-child { border-bottom: none; }
            .category-header { display: flex; justify-content: space-between; align-items: center; padding: 12px 16px; background: var(--neutral-20, #f7f7f7); cursor: pointer; font-weight: 600; color: var(--body-text-color, #333); transition: background-color 0.2s; }
            .category-header:hover { background: #e9ecef; }
            .category-header-controls { display: flex; align-items: center; gap: 12px; }
            .category-action-btn { background: none; border: 1px solid var(--border-color, #ccc); color: var(--body-text-color, #555); padding: 2px 8px; font-size: 12px; font-weight: normal; border-radius: var(--global-radius, 8px); cursor: pointer; transition: all 0.2s; }
            .category-action-btn:hover { background-color: var(--hover-color, #f0f0f0); border-color: #999; color: #000; }
            .category-toggle { font-size: 12px; transition: transform 0.2s; }
            .category-toggle.collapsed { transform: rotate(-90deg); }
            .category-apps { position: relative; overflow-y: auto; will-change: scroll-position; }
            .virtual-scroll-content { position: relative; width: 100%; overflow: hidden; }
            .category-apps.collapsed { display: none; }
            .app-item, .selected-app-item { padding: 10px 16px; gap: 5px; border-bottom: 1px solid #f1f3f4; cursor: pointer; transition: background-color 0.2s; font-size: 14px; display: flex; align-items: center; height: var(--item-height); box-sizing: border-box; }
            .app-item:last-child, .selected-app-item:last-child { border-bottom: none; }
            .app-item:hover { background-color: var(--neutral-10, #f0f0f0); }
            .selected-app-item { background-color: var(--primary-10, #e3f2fd); }
            .selected-app-item:hover { background-color: var(--primary-20, #bbdefb); }
            .selected-category-header { background: var(--neutral-20, #f7f7f7); border-bottom: 1px solid var(--border-color, #e0e0e0); }
            .selected-category-apps.collapsed { display: none; }
            .modal-footer { display: flex; justify-content: flex-end; gap: 10px; padding: 20px; border-top: 1px solid var(--border-color, #e0e0e0); }
            .cancel-button, .submit-button { padding: 10px 20px; border: none; border-radius: var(--global-radius, 8px); cursor: pointer; font-size: 14px; transition: background-color 0.2s; }
            .cancel-button { background: #f8f9fa; color: #666; border: 1px solid var(--border-color, #ccc); }
            .cancel-button:hover { background: #e9ecef; }
            .submit-button { background: var(--primary-60, #007bff); color: white; }
            .submit-button:hover { background: var(--primary-70, #0056b3); }
            .empty-state { text-align: center; padding: 40px 20px; color: #666; font-style: italic; }
            .category-count { font-size: 12px; color: #666; font-weight: normal; }
            i.icon { width: 16px; height: 16px; display: inline-block; background-color: black; -webkit-mask-size: cover; -webkit-mask-position: center; -webkit-mask-repeat: no-repeat; mask-size: cover; mask-position: center; mask-repeat: no-repeat; }
            i.icon.icon-add { background-color: var(--primary-60, #007bff); --svg: url('data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgdmlld0JveD0iMCAwIDI0IDI0IiBmaWxsPSJub25lIiBzdHJva2U9IiMwMDAiIHN0cm9rZS13aWR0aD0iMiIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIj48cGF0aCBkPSJNMTIgNXYxNE01IDEyaDE0Ii8+PC9zdmc+'); -webkit-mask-image: var(--svg); mask-image:  var(--svg); }
            i.icon.icon-remove { background-color: var(--neutral-60, #000000); --svg: url('data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyNCIgaGVpZ2h0PSIyNCIgdmlld0JveD0iMCAwIDI0IDI0IiBmaWxsPSJub25lIiBzdHJva2U9IiMwMDAiIHN0cm9rZS13aWR0aD0iMiIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIj48cGF0aCBkPSJNNSAxMmgxNCIvPjwvc3ZnPg=='); -webkit-mask-image: var(--svg); mask-image:  var(--svg); }
            i.icon.icon-circle-add { background-color: var(--primary-60, #007bff); --svg: url('data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0OCIgaGVpZ2h0PSI0OCIgdmlld0JveD0iMCAwIDQ4IDQ4IiBmaWxsPSJub25lIiBzdHJva2U9IiMwMDAiIHN0cm9rZS13aWR0aD0iMyIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIj48Y2lyY2xlIGN4PSIyNCIgY3k9IjI0IiByPSIyMiIvPjxwYXRoIGQ9Ik0yNCAxNHYyME0xNCAyNGgyMCIvPjwvc3ZnPg=='); -webkit-mask-image: var(--svg); mask-image:  var(--svg); }
            i.icon.icon-circle-remove { background-color: var(--neutral-60, #000000); --svg: url('data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0OCIgaGVpZ2h0PSI0OCIgdmlld0JveD0iMCAwIDQ4IDQ4IiBmaWxsPSJub25lIiBzdHJva2U9IiMwMDAiIHN0cm9rZS13aWR0aD0iMyIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIj48Y2lyY2xlIGN4PSIyNCIgY3k9IjI0IiByPSIyMiIvPjxwYXRoIGQ9Ik0xNCAyNGgyMCIvPjwvc3ZnPg=='); -webkit-mask-image: var(--svg); mask-image:  var(--svg); }
        `;
            this.#shadowRoot.appendChild(style);
        }

        #addEventListeners() {
            this.#closeButton.addEventListener('click', () => this.hide());
            this.#cancelButton.addEventListener('click', () => this.hide());
            this.#submitButton.addEventListener('click', () => this.#handleSubmit());
            this.#searchInput.addEventListener('input', () => this.#handleSearch());
            this.#searchClearBtn.addEventListener('click', () => this.#clearSearch());
            this.#modalOverlay.addEventListener('click', (e) => {
                if (e.target === this.#modalOverlay) this.hide();
            });
            this.#appListsContainer.addEventListener('click', (e) => {
                const target = e.target.closest('.list-action-btn');
                if (!target) return;
                const {action, target: listTarget} = target.dataset;
                this.#handleExpandCollapseAll(action, listTarget);
            });
        }

        #handleExpandCollapseAll(action, listTarget) {
            if (listTarget === 'available') {
                const categories = Object.keys(this.#categorizedApps);
                if (action === 'collapse-all') {
                    categories.forEach(cat => this.#collapsedCategories.add(cat));
                } else {
                    this.#collapsedCategories.clear();
                }
            } else if (listTarget === 'selected') {
                const selectedByCategory = this.#categorizeApps(this.#selectedApps);
                const categories = Object.keys(selectedByCategory);
                if (action === 'collapse-all') {
                    categories.forEach(cat => this.#collapsedSelectedCategories.add(cat));
                } else {
                    this.#collapsedSelectedCategories.clear();
                }
            }
            this.#renderAppLists();
        }

        #handleSearch() {
            this.#searchTerm = this.#searchInput.value.toLowerCase().trim();
            this.#updateSearchClearButton();
            this.#renderAppLists();
        }

        #updateSearchClearButton() {
            this.#searchClearBtn.classList.toggle('visible', this.#searchInput.value.trim() !== '');
        }

        #clearSearch() {
            this.#searchInput.value = '';
            this.#searchTerm = '';
            this.#updateSearchClearButton();
            this.#renderAppLists();
            this.#searchInput.focus();
        }

        #createCategoryHeader(category, count, options = {}) {
            const {isSelectedList = false, onSelectAll, onDeselectAll} = options;
            const collection = isSelectedList ? this.#collapsedSelectedCategories : this.#collapsedCategories;
            const isCollapsed = collection.has(category);
            const header = document.createElement('div');
            header.className = `category-header ${isSelectedList ? 'selected-category-header' : ''}`;
            header.addEventListener('click', (e) => {
                if (e.target.closest('.category-action-btn')) return;
                collection.has(category) ? collection.delete(category) : collection.add(category);
                this.#renderAppLists();
            });
            const title = document.createElement('span');
            title.innerHTML = `${category} <span class="category-count">(${count})</span>`;
            const controls = document.createElement('div');
            controls.className = 'category-header-controls';
            if (!isSelectedList && onSelectAll) {
                const btn = document.createElement('button');
                btn.className = 'category-action-btn';
                btn.textContent = '<#select_all#>';
                btn.addEventListener('click', (e) => {
                    e.stopPropagation();
                    onSelectAll();
                });
                controls.appendChild(btn);
            }
            if (isSelectedList && onDeselectAll) {
                const btn = document.createElement('button');
                btn.className = 'category-action-btn';
                btn.textContent = `<#Unselect_All#>`;
                btn.addEventListener('click', (e) => {
                    e.stopPropagation();
                    onDeselectAll();
                });
                controls.appendChild(btn);
            }
            const toggle = document.createElement('span');
            toggle.className = `category-toggle ${isCollapsed ? 'collapsed' : ''}`;
            toggle.innerHTML = '▼';
            controls.appendChild(toggle);
            header.append(title, controls);
            return header;
        }

        #createAppItem(app, isSelected = false) {
            const item = document.createElement('div');
            item.className = isSelected ? 'selected-app-item' : 'app-item';
            item.dataset.id = app.id;
            item.setAttribute('role', 'option');
            item.innerHTML = `${isSelected ? `<i class="icon icon-remove"></i><span>${app.name}</span>` : `<i class="icon icon-add"></i><span>${app.name}</span>`}`;
            item.addEventListener('click', () => {
                isSelected ? this.#moveAppToAvailable(app) : this.#moveAppToSelected(app);
            });
            return item;
        }

        // NEW: Method to update the selected count in the title
        #updateSelectedCount() {
            const count = this.#selectedApps.length;
            const baseTitle = this.#options.selectedAppsTitle;
            this.#selectedAppsTitleElement.textContent = count > 0 ? `${baseTitle} (${count})` : baseTitle;
        }

        #renderAppLists() {
            this.#updateSelectedCount(); // Update count before rendering lists
            this.#renderAvailableApps();
            this.#renderSelectedApps();
        }

        #renderAvailableApps() {
            this.#availableCategoriesDiv.innerHTML = '';
            let hasResults = false;
            Object.entries(this.#categorizedApps).forEach(([category, apps]) => {
                const availableApps = apps.filter(app => {
                    return !this.#selectedApps.some(selected => selected.id === app.id) && (this.#searchTerm === '' || app.name.toLowerCase().includes(this.#searchTerm))
                });
                if (availableApps.length === 0) return;
                hasResults = true;
                const categorySection = document.createElement('div');
                categorySection.className = 'category-section';
                const onSelectAll = () => {
                    this.#selectedApps.push(...availableApps);
                    this.#selectedApps.sort((a, b) => a.name.localeCompare(b.name, 'zh-Hant'));
                    this.#renderAppLists();
                };
                const categoryHeader = this.#createCategoryHeader(category, availableApps.length, {onSelectAll});
                const categoryAppsContainer = document.createElement('div');
                const isCollapsed = this.#collapsedCategories.has(category);
                categoryAppsContainer.className = `category-apps ${isCollapsed ? 'collapsed' : ''}`;
                categorySection.append(categoryHeader, categoryAppsContainer);
                if (!isCollapsed) {
                    const contentWrapper = document.createElement('div');
                    contentWrapper.className = 'virtual-scroll-content';
                    categoryAppsContainer.appendChild(contentWrapper);
                    const totalHeight = availableApps.length * this.#itemHeight;
                    categoryAppsContainer.style.height = `${Math.min(200, totalHeight)}px`;
                    contentWrapper.style.height = `${totalHeight}px`;
                    const renderChunk = () => {
                        const scrollTop = categoryAppsContainer.scrollTop;
                        const containerHeight = categoryAppsContainer.clientHeight;
                        const startIndex = Math.max(0, Math.floor(scrollTop / this.#itemHeight) - 5);
                        const endIndex = Math.min(availableApps.length - 1, Math.ceil((scrollTop + containerHeight) / this.#itemHeight) + 5);
                        const visibleItems = availableApps.slice(startIndex, endIndex + 1);
                        const fragment = document.createDocumentFragment();
                        visibleItems.forEach((app, i) => {
                            const itemNode = this.#createAppItem(app, false);
                            itemNode.style.position = 'absolute';
                            itemNode.style.top = `${(startIndex + i) * this.#itemHeight}px`;
                            itemNode.style.width = '100%';
                            fragment.appendChild(itemNode);
                        });
                        requestAnimationFrame(() => {
                            contentWrapper.innerHTML = '';
                            contentWrapper.appendChild(fragment);
                        });
                    };
                    renderChunk();
                    categoryAppsContainer.onscroll = renderChunk;
                }
                this.#availableCategoriesDiv.appendChild(categorySection);
            });
            if (!hasResults) {
                this.#availableCategoriesDiv.innerHTML = `<div class="empty-state">${this.#searchTerm ? `<#No_matching_applications_found#>` : `<#No_available_applications#>`}</div>`;
            }
        }

        #renderSelectedApps() {
            this.#selectedCategoriesDiv.innerHTML = '';
            const filteredSelectedApps = this.#selectedApps.filter(app => this.#searchTerm === '' || app.name.toLowerCase().includes(this.#searchTerm));
            if (filteredSelectedApps.length === 0) {
                const message = this.#searchTerm ? `<#No_results_found_in_selected_items#>` : `<#No_applications_selected#>`;
                this.#selectedCategoriesDiv.innerHTML = `<div class="empty-state">${message}</div>`;
                return;
            }
            const selectedByCategory = this.#categorizeApps(filteredSelectedApps);
            Object.entries(selectedByCategory).forEach(([category, appsInView]) => {
                const categorySection = document.createElement('div');
                categorySection.className = 'category-section';
                const onDeselectAll = () => {
                    const allSelectedInCategory = this.#selectedApps.filter(app => (app.category || 'Other') === category);
                    const idsToRemove = new Set(allSelectedInCategory.map(app => app.id));
                    this.#selectedApps = this.#selectedApps.filter(app => !idsToRemove.has(app.id));
                    this.#renderAppLists();
                };
                const categoryHeader = this.#createCategoryHeader(category, appsInView.length, {
                    isSelectedList: true,
                    onDeselectAll
                });
                const isCollapsed = this.#collapsedSelectedCategories.has(category);
                const categoryApps = document.createElement('div');
                categoryApps.className = `selected-category-apps ${isCollapsed ? 'collapsed' : ''}`;
                if (!isCollapsed) {
                    appsInView.forEach(app => categoryApps.appendChild(this.#createAppItem(app, true)));
                }
                categorySection.append(categoryHeader, categoryApps);
                this.#selectedCategoriesDiv.appendChild(categorySection);
            });
        }

        #moveAppToSelected(app) {
            this.#selectedApps.push(app);
            this.#selectedApps.sort((a, b) => a.name.localeCompare(b.name, 'zh-Hant'));
            this.#renderAppLists();
        }

        #moveAppToAvailable(app) {
            this.#selectedApps = this.#selectedApps.filter(selected => selected.id !== app.id);
            this.#renderAppLists();
        }

        #handleSubmit() {
            this.#options.onSelect([...this.#selectedApps], this.#target);
            this.hide();
        }

        setTarget(target) {
            this.#target = target;
        }

        getSelectedApps() {
            return this.#selectedApps.map(app => app.id);
        }

        /**
         * MODIFIED: Shows the modal and optionally pre-selects a list of apps.
         * @param {string[]} [preSelectedIds=[]] - An array of app IDs to pre-select.
         */
        show(preSelectedIds = []) {
            // Find the full app objects from the provided IDs using the efficient map.
            const preSelectedApps = preSelectedIds
                .map(id => this.#appMap.get(id))
                .filter(Boolean); // .filter(Boolean) removes any undefined entries if an ID was not found

            // Use a spread operator to create a new array, ensuring the original is not mutated.
            this.#selectedApps = [...preSelectedApps];

            // Reset UI state for a clean presentation
            this.#searchInput.value = '';
            this.#searchTerm = '';
            this.#updateSearchClearButton();
            this.#collapsedCategories.clear();
            this.#collapsedSelectedCategories.clear();

            // Render the lists with the pre-selected data
            this.#renderAppLists();

            // Display the modal
            this.#modalOverlay.style.display = 'flex';
            setTimeout(() => this.#searchInput.focus(), 10);
        }

        hide() {
            this.#modalOverlay.style.display = 'none';
        }

        render() {
            return this.#element;
        }
    }

    /**
     * NEW: Parses the complex, hierarchical data structure into a flat list
     * that the AppSelectorModal can understand.
     * @param {object} data The raw data object from the user.
     * @returns {Array<object>} A flat array of app objects.
     */
    function parseDataForModal(data, lang) {
        const mainCategoryMap = new Map(data.main_category.map(cat => [cat._id, cat.name]));

        const subCategoryMap = new Map();
        data.category.forEach(subCat => {
            const mainCatName = mainCategoryMap.get(subCat.main_category_id) || 'Unknown';
            const fullCategoryName = `${mainCatName} / ${subCat.name}`;

            // Handle cases where one sub-category has multiple IDs (e.g., "4,5,6,7")
            const ids = subCat._id.split(',');
            ids.forEach(id => {
                subCategoryMap.set(id, fullCategoryName);
            });
        });

        const initialApps = data.app_list.map(app => ({
            id: app.app_id,
            name: (() => {
                const {name, language = {}} = app;
                const enName = language.EN;
                const langName = language[lang];
                return lang === "EN"
                    ? enName || name
                    : langName
                        ? `${langName}(${enName || name})`
                        : enName || name;
            })(),
            category: subCategoryMap.get(app.category_id) || 'Other',
            originalData: app
        }));

        return initialApps;
    }


    function initializeModal(data, lang) {

        const processedApps = parseDataForModal(data, lang);

        appSelectorModal = new AppSelectorModal({
            id: 'enhanced-app-selector-modal',
            modalSize: 'modal-xl',
            initialApps: processedApps,
            onSelect: (selectedApps, target) => {
                // console.log('已選擇的 APP:', selectedApps);
                target.moreBtn.querySelector(".item-name").innerHTML = `<b><#More_Apps#> (${selectedApps.length} <#Selected#>) ></b>`;
                const appIds = selectedApps.map(app => app.id);
                target.setSelectedAppId(appIds);
            }
        });
        top.document.body.appendChild(appSelectorModal.render());
    }

    const getSettingsData = async () => {
        const [
            enableData,
            arkAppList,
            nvramData
        ] = await Promise.all([
            httpApi.nvramGet(["ark_iam_enable", "preferred_lang", "territory_code"], true),
            httpApi.hookGet("get_ark_app_list", true),
            httpApi.nvramCharToAscii([
                "ark_iam_ct_list",
                "ark_iam_app_list1",
                "ark_iam_app_list2",
                "ark_iam_app_list3",
                "ark_iam_app_list4"
            ], true)
        ]);

        const appListKeys = ["ark_iam_app_list1", "ark_iam_app_list2", "ark_iam_app_list3", "ark_iam_app_list4"];
        const combinedAppList = appListKeys
            .map(key => decodeURIComponent(nvramData[key] || ""))
            .filter(item => item && item.trim() !== "")
            .join("<");

        const decodedCtList = decodeURIComponent(nvramData.ark_iam_ct_list || "");
        return {
            preferred_lang: enableData.preferred_lang || "EN",
            territory_code: enableData.territory_code,
            ark_iam_enable: enableData.ark_iam_enable,
            ark_iam_app_list_array: combinedAppList ? combinedAppList.split("<") : [],
            ark_iam_ct_list_array: decodedCtList ? decodedCtList.split("<") : [],
            ark_app_list: arkAppList
        };
    };

    class ContentFilterRule {
        constructor(props = {}) {
            const defaults = {
                enable: "1",
                target: "",
                apps: [],
                category: [],
                checklist: null
            };
            Object.assign(this, defaults, props);
        }
    }

    async function initial() {
        getSettingsData()
            .then(data => {
                const isCnSku = data.territory_code.search("CN") !== -1
                window.ui_lang = data.preferred_lang;
                const faq_href = "https://nw-dlcdnet.asus.com/support/forward.html?model=&type=Faq&lang=" + data.preferred_lang + "&kw=&num=140";
                document.getElementById("faq").href = faq_href;

                $('#radio_web_restrict_enable').iphoneSwitch(data.ark_iam_enable,
                    function () {
                        showhide("list_table", 1);
                    },
                    function () {
                        showhide("list_table", 0);
                        httpApi.nvramSet({
                            "ark_iam_enable": "0",
                            "action_mode": "apply",
                            "rc_service": "restart_ark",
                        });
                    }
                );


                if (data.ark_iam_enable == 1)
                    showhide("list_table", 1);
                else
                    showhide("list_table", 0);

                data.ark_app_list.app_list = data.ark_app_list.app_list.filter(app => {
                    if (isCnSku) {
                        return app.cnShow === true;
                    }
                    return true;
                });

                ark_app_list = data.ark_app_list;
                ark_iam_ct_list_array = data.ark_iam_ct_list_array;
                ark_iam_app_list_array = data.ark_iam_app_list_array;
                ark_iam_rulelist = ark_iam_app_list_array.map(item => {
                    const [enable, target, apps] = item.split(">");
                    const app_ids = apps.split(",").filter(app => app.trim() !== "");

                    const hierarchicalChecklist = new HierarchicalChecklist(ark_app_list, data.preferred_lang);
                    return new ContentFilterRule({
                        enable: enable === "1",
                        target,
                        apps: app_ids,
                        checklist: hierarchicalChecklist
                    });
                });


                const ct_list = ark_iam_ct_list_array.map(item => {
                    const [enable, target, ...categories] = item.split(">");
                    let categoryList = categories
                        .map(c => c.trim())
                        .filter(Boolean)
                        .flatMap(c => c.split(",").map(s => s.trim()))
                    const fullSet = ["4", "5", "6", "7"];
                    const hasFullSet = fullSet.every(c => categoryList.includes(c));
                    if (hasFullSet) {
                        // remove "4", "5", "6", "7", replace to "4,5,6,7"
                        categoryList = categoryList.filter(c => !fullSet.includes(c));
                        categoryList.push("4,5,6,7");
                    }
                    return {
                        enable: enable === "1",
                        target,
                        category: categoryList
                    };
                });

                ark_iam_rulelist.forEach(rule => {
                    const match = ct_list.find(ct => ct.target === rule.target);
                    if (!match) return;

                    const selectedCategories = match.category;
                    rule.category = selectedCategories;
                    rule.checklist.setSelectedCategoryId(selectedCategories);

                    const availableApps = rule.checklist.getAppList();
                    const filteredApps = availableApps.filter(app => selectedCategories.includes(app.category_id));
                    const filteredAppIds = filteredApps.map(app => app.app_id);

                    rule.apps.push(...filteredAppIds);
                    rule.checklist.setSelectedAppId(rule.apps);
                });

                ct_list.forEach(ct => {
                    const find_rule = ark_iam_rulelist.find(r => r.target === ct.target);
                    if (!find_rule) {
                        const rule = new ContentFilterRule({
                            enable: ct.enable,
                            target: ct.target,
                            apps: [],
                            category: ct.category,
                            checklist: new HierarchicalChecklist(ark_app_list, data.preferred_lang)
                        })
                        const selectedCategories = rule.category;
                        rule.category = selectedCategories;
                        rule.checklist.setSelectedCategoryId(selectedCategories);

                        const availableApps = rule.checklist.getAppList();
                        const filteredApps = availableApps.filter(app => selectedCategories.includes(app.category_id));
                        const filteredAppIds = filteredApps.map(app => app.app_id);
                        rule.apps.push(...filteredAppIds);
                        rule.checklist.setSelectedAppId(rule.apps);
                        ark_iam_rulelist.push(rule);
                    }
                })

                genMain_table();
                settingChecklist = new HierarchicalChecklist(ark_app_list, data.preferred_lang);
                document.querySelector('#tree-container').appendChild(settingChecklist.render());
                genRulesRow(ark_iam_rulelist);
                initializeModal(ark_app_list, data.preferred_lang);
                showDropdownClientList('setClientIP', 'mac', 'all', 'ClientList_Block_PC', 'pull_arrow', 'all');
            })
            .catch(error => console.error('Error', error));
    }

    initial();
</script>
</body>
</html>
