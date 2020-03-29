install_subdir(
    'joinlines',
    install_dir: pluginslibdir
)

textsize_desktop = custom_target(
    'joinlines_desktop',
    input: 'joinlines.plugin.desktop.in',
    output: 'joinlines.plugin',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: pluginslibdir,
)
