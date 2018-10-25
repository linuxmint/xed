install_subdir(
    'textsize',
    install_dir: pluginslibdir
)

textsize_desktop = custom_target(
    'textsize_desktop',
    input: 'textsize.plugin.desktop.in',
    output: 'textsize.plugin',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: pluginslibdir,
)
