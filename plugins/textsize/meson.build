textsize_sources = files(
    'textsize.c'
)

textsize_deps = [
    dependency('xed'),
    dependency('gtk+-3.0'),
    dependency('libpeas-1.0'),
]

library(
    'textsize',
    textsize_sources,
    dependencies: textsize_deps,
    install: true,
    install_dir: join_paths(libdir, 'xed', 'plugins')
)

textsize_desktop = custom_target(
    'textsize_desktop',
    input: 'textsize.plugin.desktop.in',
    output: 'textsize.plugin',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: pluginslibdir,
)
