sort_sources = [
    'xed-sort-plugin.h',
    'xed-sort-plugin.c'
]

sort_deps = [
    config_h,
    glib,
    gtksourceview,
    libpeas
]

library(
    'sort',
    sort_sources,
    link_with: libxed,
    dependencies: sort_deps,
    include_directories: include_dirs,
    install_rpath: join_paths(prefix, libdir, 'xed'),
    install: true,
    install_dir: join_paths(libdir, 'xed', 'plugins'),
)

sort_desktop = custom_target(
    'sort_desktop',
    input: 'sort.plugin.desktop.in',
    output: 'sort.plugin',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: pluginslibdir,
)
