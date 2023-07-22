project('test-static-link', 'c')

# This is a dummy project that static links against installed gio.
# See gio/tests/static-link.py.
app = executable('test-static-link', 'app.c',
  dependencies : dependency('gio-2.0', static : true)
)
test('test-static-link', app)
