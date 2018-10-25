trailsave_sources = [
    'xed-trail-save-plugin.h',
    'xed-trail-save-plugin.c'
]

trailsave_deps = [
    config_h,
    glib,
    gtksourceview,
    libpeas
]

library(
    'trailsave',
    trailsave_sources,
    link_with: libxed,
    dependencies: trailsave_deps,
    include_directories: include_dirs,
    install_rpath: join_paths(prefix, libdir, 'xed'),
    install: true,
    install_dir: join_paths(libdir, 'xed', 'plugins')
)

trailsave_desktop = custom_target(
    'trailsave_desktop',
    input: 'trailsave.plugin.desktop.in',
    output: 'trailsave.plugin',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: pluginslibdir,
)
