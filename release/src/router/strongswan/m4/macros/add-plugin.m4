# ADD_PLUGIN(plugin, category list)
# -----------------------------------
# Append the plugin name $1 to the category list variable $2_plugin
AC_DEFUN([ADD_PLUGIN],
	if test [patsubst(x$$1, [-], [_])] = xtrue; then
		[m4_foreach_w([category], [$2],
			[m4_format([%s_plugins=${%s_plugins}" $1"], category, category)]
		)]
	fi
)
