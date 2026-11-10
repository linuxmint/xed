install_subdir(
    'jsonformatter',
    install_dir: pluginslibdir
)

jsonformatter_desktop = custom_target(
    'jsonformatter_desktop',
    input: 'jsonformatter.plugin.desktop.in',
    output: 'jsonformatter.plugin',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: pluginslibdir,
)
