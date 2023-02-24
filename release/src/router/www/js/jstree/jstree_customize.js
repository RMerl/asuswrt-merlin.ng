// customize plugin by @Lusito
// https://github.com/Lusito/jstree/blob/node-customize/src/jstree-node-customize.js
/**
 * ### Node Customize plugin
 *
 * Allows to customize nodes when they are drawn.
 */
(function (factory) {
	"use strict";
	if (typeof define === 'function' && define.amd) {
		define('jstree.node_customize', ['jquery','jstree'], factory);
	}
	else if(typeof exports === 'object') {
		factory(require('jquery'), require('jstree'));
	}
	else {
		factory(jQuery, jQuery.jstree);
	}
}(function ($, jstree, undefined) {
	"use strict";

	if($.jstree.plugins.node_customize) { return; }

	/**
	 * the settings object.
	 * key is the attribute name to select the customizer function from switch.
	 * switch is a key => function(el, node) map.
	 * default: function(el, node) will be called if the type could not be mapped
	 * @name $.jstree.defaults.node_customize
	 * @plugin node_customize
	 */
	$.jstree.defaults.node_customize = {
		"key": "type",
		"switch": {},
		"default": null
	};

	$.jstree.plugins.node_customize = function (options, parent) {
		this.redraw_node = function (obj, deep, callback, force_draw) {
			var el = parent.redraw_node.apply(this, arguments);
			if (el) {
				var node = this.get_node(obj);
				var cfg = this.settings.node_customize;
				var key = cfg.key;
				var type =  (node && node.original && node.original[key]);
				var customizer = (type && cfg.switch[type]) || cfg.default;
				if(customizer)
					customizer(el, node);
			}
			return el;
		};
	}
}));

$.jstree.plugins.noclose = function () {
	this.close_node = $.noop;
};