project('cmd-polkit', 'c', version : '0.4.0-dev')

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
  'src/app.c',
]

main_sources += common_sources

test_sources = [
  'test/logger.mock.c',
  'test/gtk.mock.c',
  'test/app.mock.c',
  'test/polkit.mock.c',
  'test/error-message.mock.c',
  'test/polkit-auth-handler.service.mock.c',
] + common_sources

test_unit_sources            = [ 'test/test-unit.entrypoint.c' ] + test_sources
test_it_command_exec_sources = [ 'test/test-it-command-exec.entrypoint.c' ] + test_sources
test_it_serial_mode          = [ 'test/test-it-serial-mode.entrypoint.c' ] + test_sources
test_it_parallel_mode        = [ 'test/test-it-parallel-mode.entrypoint.c' ] + test_sources

executable('cmd-polkit-agent', main_sources,
  dependencies: deps,
  install: true,
)

test_env = [
  'LC_ALL=C',
  'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
  'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
]

test(
  'test-suite-unit',
  executable('cmd-polkit-agent-test-unit', test_unit_sources, dependencies: deps),
  env: test_env,
  protocol: 'tap',
  workdir: join_paths(meson.current_source_dir(), 'test')
)

test(
  'test-suite-it',
  executable('cmd-polkit-agent-test-it-command-exec', test_it_command_exec_sources, dependencies: deps),
  env: test_env,
  protocol: 'tap',
  workdir: join_paths(meson.current_source_dir(), 'test')
)

test(
  'test-suite-it-serial-mode',
  executable('cmd-polkit-agent-test-it-serial-mode', test_it_serial_mode, dependencies: deps),
  env: test_env,
  protocol: 'tap',
  workdir: join_paths(meson.current_source_dir(), 'test')
)

test(
  'test-suite-it-parallel-mode',
  executable('cmd-polkit-agent-test-it-parallel-mode', test_it_parallel_mode, dependencies: deps),
  env: test_env,
  protocol: 'tap',
  workdir: join_paths(meson.current_source_dir(), 'test')
)


