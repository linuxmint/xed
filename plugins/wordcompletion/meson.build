wordcompletion_sources = [
    'xed-wordcompletion-plugin.h',
    'xed-wordcompletion-plugin.c'
]

wordcompletion_deps = [
    config_h,
    gio,
    glib,
    gtksourceview,
    libpeas,
    libpeas_gtk
]

library(
    'wordcompletion',
    wordcompletion_sources,
    link_with: libxed,
    dependencies: wordcompletion_deps,
    include_directories: include_dirs,
    install_rpath: join_paths(prefix, libdir, 'xed'),
    install: true,
    install_dir: join_paths(libdir, 'xed', 'plugins')
)

wordcompletion_desktop = custom_target(
    'wordcompletion_desktop',
    input: 'wordcompletion.plugin.desktop.in',
    output: 'wordcompletion.plugin',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: pluginslibdir,
)

install_data(
    'xed-wordcompletion-configure.ui',
    install_dir: join_paths(pluginsdatadir, 'wordcompletion')
)

install_data(
    'org.x.editor.plugins.wordcompletion.gschema.xml',
    install_dir: schema_dir
)
