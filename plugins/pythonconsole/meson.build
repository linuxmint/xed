subdir('pythonconsole')

pythonconsole_desktop = custom_target(
    'pythonconsole_desktop',
    input: 'pythonconsole.plugin.desktop.in',
    output: 'pythonconsole.plugin',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: pluginslibdir,
)

pythonconsole_schema_file = configure_file(
    input: 'org.x.editor.plugins.pythonconsole.gschema.xml.in',
    output: 'org.x.editor.plugins.pythonconsole.gschema.xml',
    configuration: schema_conf,
    install_dir: join_paths(datadir, 'glib-2.0', 'schemas')
)

install_data(
    pythonconsole_schema_file,
    install_dir: schema_dir
)
