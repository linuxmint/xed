bracketcompletion_sources = files(
    'bracketcompletion.c'
)

bracketcompletion_deps = [
    dependency('xed'),
    dependency('gtk+-3.0'),
    dependency('libpeas-1.0'),
]

library(
    'bracketcompletion',
    bracketcompletion_sources,
    dependencies: bracketcompletion_deps,
    install: true,
    install_dir: join_paths(libdir, 'xed', 'plugins')
)

bracketcompletion_desktop = custom_target(
    'bracketcompletion_desktop',
    input: 'bracketcompletion.plugin.desktop.in',
    output: 'bracketcompletion.plugin',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: pluginslibdir,
)
