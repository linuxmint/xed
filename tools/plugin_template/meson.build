##ifdef HAS_C_FILES
##(PLUGIN_ID.lower)_sources = [
    xed-##(PLUGIN_MODULE.lower)-plugin.h
    xed-##(PLUGIN_MODULE.lower)-plugin.c,
]

##(PLUGIN_ID.lower)_deps = [
    config_h,
    glib,
    gtksourceview,
    libpeas,
    libpeas_gtk
]

library(
    '##(PLUGIN_ID.lower)',
    ##(PLUGIN_ID.lower)_sources,
    link_with: libxed,
    dependencies: ##(PLUGIN_ID.lower)_deps,
    include_directories: include_dirs,
    install_rpath: join_paths(prefix, libdir, 'xed'),
    install: true,
    install_dir: join_paths(libdir, 'xed', 'plugins')
)

##endif
##(PLUGIN_ID.lower)_desktop = custom_target(
    '##(PLUGIN_ID)_desktop',
    input: '##(PLUGIN_ID).plugin.desktop.in',
    output: '##(PLUGIN_ID).plugin',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: pluginslibdir,
)
