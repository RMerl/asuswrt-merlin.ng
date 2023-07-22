fuzz_targets = [
  'fuzz_bookmark',
  'fuzz_dbus_message',
  'fuzz_key',
  'fuzz_variant_binary',
  'fuzz_variant_text',
]

deps = [libgmodule_dep, libgio_dep, libglib_dep, libgobject_dep]

extra_sources = []
extra_c_args = cc.get_supported_arguments('-Werror=unused-function')

# Links in a static library provided by oss-fuzz, else a standalone driver.
# https://github.com/google/oss-fuzz/blob/master/docs/new_project_guide.md#buildsh-script-environment
fuzzing_engine = cxx.find_library('FuzzingEngine', required : false)
if fuzzing_engine.found()
  deps += fuzzing_engine
else
  extra_sources += 'driver.c'
endif

foreach target_name : fuzz_targets
  exe = executable(target_name, [extra_sources, target_name + '.c'],
    c_args : extra_c_args,
    dependencies : deps,
  )
endforeach
