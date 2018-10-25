filebrowser_headers = [
    'xed-file-bookmarks-store.h',
    'xed-file-browser-store.h',
    'xed-file-browser-view.h',
    'xed-file-browser-widget.h',
    'xed-file-browser-error.h',
    'xed-file-browser-utils.h',
    'xed-file-browser-plugin.h',
    'xed-file-browser-messages.h'
]

filebrowser_lib_sources = [
    'xed-file-bookmarks-store.c',
    'xed-file-browser-store.c',
    'xed-file-browser-view.c',
    'xed-file-browser-widget.c',
    'xed-file-browser-utils.c',
    'xed-file-browser-plugin.c',
    'xed-file-browser-messages.c'
]

filebrowser_enums = gnome.mkenums_simple(
    'xed-file-browser-enum-types',
    sources : filebrowser_headers,
    identifier_prefix : 'XedFileBrowser',
    symbol_prefix : 'xed_file_browser'
)

filebrowser_marshal = gnome.genmarshal(
    'xed-file-browser-marshal',
    sources: 'xed-file-browser-marshal.list',
    prefix: 'xed_file_browser_marshal'
)

filebrowser_deps = [
    config_h,
    gio,
    glib,
    gmodule,
    gtk,
    gtksourceview,
    libpeas
]

library(
    'filebrowser',
    filebrowser_headers + filebrowser_lib_sources + filebrowser_enums + filebrowser_marshal,
    link_with: libxed,
    dependencies: filebrowser_deps,
    include_directories: include_dirs,
    install_rpath: join_paths(prefix, libdir, 'xed'),
    install: true,
    install_dir: join_paths(libdir, 'xed', 'plugins')
)

install_data(
    'xed-file-browser-widget-ui.xml',
    install_dir: join_paths(pluginsdatadir, 'filebrowser')
)

filebrowser_desktop = custom_target(
    'filebrowser_desktop',
    input: 'filebrowser.plugin.desktop.in',
    output: 'filebrowser.plugin',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: pluginslibdir,
)

install_data(
    'org.x.editor.plugins.filebrowser.gschema.xml',
    install_dir: schema_dir
)
