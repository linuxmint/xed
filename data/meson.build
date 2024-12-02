desktop = custom_target(
    'desktop',
    input: 'org.x.editor.desktop.in.in',
    output: 'org.x.editor.desktop',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: desktopdir,
)

appdata = custom_target(
    'appdata',
    input: 'org.x.editor.metainfo.xml.in',
    output: 'org.x.editor.metainfo.xml',
    command: [intltool_merge, '-x', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: join_paths(datadir, 'metainfo'),
)

schema_conf = configuration_data()
schema_conf.set('GETTEXT_PACKAGE', 'xed')

schema_file = configure_file(
    input: 'org.x.editor.gschema.xml.in',
    output: 'org.x.editor.gschema.xml',
    configuration: schema_conf,
    install_dir: join_paths(datadir, 'glib-2.0', 'schemas')
)

service_conf = configuration_data()
service_conf.set('bindir', join_paths(prefix, bindir))

configure_file(
    input: 'org.x.editor.service.in',
    output: 'org.x.editor.service',
    configuration: service_conf,
    install_dir: join_paths(datadir, 'dbus-1', 'services')
)

install_data(
    'xed.1',
    install_dir : join_paths(get_option('prefix'), get_option('mandir'), 'man1')
)

install_data(
    'xed.xml',
    install_dir: join_paths(datadir, 'gtksourceview-4', 'styles')
)
