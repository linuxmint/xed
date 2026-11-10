joinlines_sources = files(
    'joinlines.c'
)

joinlines_deps = [
    dependency('xed'),
    dependency('gtk+-3.0'),
    dependency('libpeas-1.0'),
]

library(
    'joinlines',
    joinlines_sources,
    dependencies: joinlines_deps,
    install: true,
    install_dir: join_paths(libdir, 'xed', 'plugins')
)

joinlines_desktop = custom_target(
    'joinlines_desktop',
    input: 'joinlines.plugin.desktop.in',
    output: 'joinlines.plugin',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: pluginslibdir,
)
