install_subdir(
    'multiedit',
    install_dir: pluginslibdir
)

multiedit_desktop = custom_target(
    'multiedit_desktop',
    input: 'multiedit.plugin.desktop.in',
    output: 'multiedit.plugin',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: pluginslibdir,
)
