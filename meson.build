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

sources = files([
  'src/main.entrypoint.c',
  'src/polkit-auth-handler.service.c',
  'src/cmdline.c',
  'src/logger.c',
  'src/error-message.dialog.c',

  'src/json-glib.extension.c',

  'src/accepted-actions.enum.c',
])

executable('cmd-polkit-agent', sources,
  dependencies: deps,
  install: true,
)
