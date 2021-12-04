rsort_sources = [
    'xed-rsort-plugin.h',
    'xed-rsort-plugin.c'
]

rsort_deps = [
    config_h,
    glib,
    gtksourceview,
    libpeas
]

library(
    'rsort',
    rsort_sources,
    link_with: libxed,
    dependencies: rsort_deps,
    include_directories: include_dirs,
    install_rpath: join_paths(prefix, libdir, 'xed'),
    install: true,
    install_dir: join_paths(libdir, 'xed', 'plugins'),
)

rsort_desktop = custom_target(
    'rsort_desktop',
    input: 'rsort.plugin.desktop.in',
    output: 'rsort.plugin',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: pluginslibdir,
)
