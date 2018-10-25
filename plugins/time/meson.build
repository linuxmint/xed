time_sources = [
    'xed-time-plugin.h',
    'xed-time-plugin.c'
]

time_deps = [
    config_h,
    gio,
    glib,
    gtksourceview,
    libpeas,
    libpeas_gtk
]

library(
    'time',
    time_sources,
    link_with: libxed,
    dependencies: time_deps,
    include_directories: include_dirs,
    install_rpath: join_paths(prefix, libdir, 'xed'),
    install: true,
    install_dir: join_paths(libdir, 'xed', 'plugins')
)

time_ui_sources = [
    'xed-time-dialog.ui',
    'xed-time-setup-dialog.ui'
]

install_data(
    time_ui_sources,
    install_dir: join_paths(pluginsdatadir, 'time')
)

time_desktop = custom_target(
    'time_desktop',
    input: 'time.plugin.desktop.in',
    output: 'time.plugin',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: pluginslibdir,
)

time_schema_file = configure_file(
    input: 'org.x.editor.plugins.time.gschema.xml.in',
    output: 'org.x.editor.plugins.time.gschema.xml',
    configuration: schema_conf,
    install_dir: join_paths(datadir, 'glib-2.0', 'schemas')
)

install_data(
    time_schema_file,
    install_dir: schema_dir
)
