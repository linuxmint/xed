modelines_sources = [
    'xed-modeline-plugin.h',
    'xed-modeline-plugin.c',
    'modeline-parser.h',
    'modeline-parser.c'
]

modelines_deps = [
    config_h,
    glib,
    gmodule,
    gtk,
    gtksourceview,
    libpeas
]

library(
    'modelines',
    modelines_sources,
    link_with: libxed,
    dependencies: modelines_deps,
    include_directories: include_dirs,
    install_rpath: join_paths(prefix, libdir, 'xed'),
    install: true,
    install_dir: join_paths(libdir, 'xed', 'plugins')
)

install_data(
    'language-mappings',
    install_dir: join_paths(pluginsdatadir, 'modelines')
)

modelines_desktop = custom_target(
    'modelines_desktop',
    input: 'modelines.plugin.desktop.in',
    output: 'modelines.plugin',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: pluginslibdir,
)
