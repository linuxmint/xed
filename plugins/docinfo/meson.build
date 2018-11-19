docinfo_sources = [
	'xed-docinfo-plugin.h',
	'xed-docinfo-plugin.c'
]

docinfo_deps = [
    config_h,
    glib,
    gmodule,
    gtksourceview,
    libpeas,
    pango
]

library('docinfo',
    docinfo_sources,
    dependencies: docinfo_deps,
    include_directories: include_dirs,
    link_with: libxed,
    install_rpath: join_paths(prefix, libdir, 'xed'),
    install: true,
    install_dir: join_paths(libdir, 'xed', 'plugins')
)

install_data(
    'docinfo.ui',
    install_dir: join_paths(pluginsdatadir, 'docinfo')
)

docinfo_desktop = custom_target(
    'docinfo_desktop',
    input: 'docinfo.plugin.desktop.in',
    output: 'docinfo.plugin',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: pluginslibdir,
)
