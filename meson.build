project('cmd-polkit', 'c', version : '0.3.0')

prefix = get_option('prefix')
libexecdir = join_paths(prefix, get_option('libexecdir'))
sysconfdir = join_paths(prefix, get_option('sysconfdir'))
autostartdir = join_paths(sysconfdir, 'xdg', 'autostart')

deps = [
  dependency('glib-2.0'),
  dependency('json-glib-1.0'),
  dependency('polkit-agent-1'),
  dependency('gtk+-3.0')
]

common_sources = [
  'src/request-messages.c',
  'src/accepted-actions.enum.c',
  'src/json-glib.extension.c',
]

main_sources = [
  'src/main.entrypoint.c',
  'src/polkit-auth-handler.service.c',
  'src/error-message.dialog.c',
  'src/cmdline.c',
  'src/logger.c',
]

main_sources += common_sources

test_sources = [
  'test/logger.mock.c',
  'test/test.entrypoint.c',
]

test_sources += common_sources

sources = files(main_sources)

executable('cmd-polkit-agent', sources,
  dependencies: deps,
  install: true,
)

test(
  'test-suite',
  executable('cmd-polkit-agent-test', test_sources, dependencies: deps),
  env: [
    'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
    'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
  ],
  protocol: 'tap',
  workdir: join_paths(meson.current_source_dir(), 'test/assets')
)

