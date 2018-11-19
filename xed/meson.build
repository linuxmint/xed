xed_sources = [
    'xed.c'
]

private_headers = [
    'xed-close-button.h',
    'xed-close-confirmation-dialog.h',
    'xed-dirs.h',
    'xed-document-private.h',
    'xed-documents-panel.h',
    'xed-encodings-dialog.h',
    'xed-history-entry.h',
    'xed-io-error-info-bar.h',
    'xed-metadata-manager.h',
    'xed-paned.h',
    'xed-plugins-engine.h',
    'xed-preferences-dialog.h',
    'xed-print-job.h',
    'xed-print-preview.h',
    'xed-settings.h',
    'xed-status-combo-box.h',
    'xed-tab-label.h',
    'xed-ui.h',
    'xed-utils.h',
    'xed-view-frame.h',
    'xed-view-gutter-renderer.h',
    'xed-window-private.h'
]

public_headers = [
    'xed-app.h',
    'xed-app-activatable.h',
    'xed-commands.h',
    'xed-debug.h',
    'xed-document.h',
    'xed-encodings-combo-box.h',
    'xed-file-chooser-dialog.h',
    'xed-message-bus.h',
    'xed-message-type.h',
    'xed-message.h',
    'xed-notebook.h',
    'xed-panel.h',
    'xed-progress-info-bar.h',
    'xed-searchbar.h',
    'xed-statusbar.h',
    'xed-tab.h',
    'xed-view.h',
    'xed-view-activatable.h',
    'xed-window.h',
    'xed-window-activatable.h'
]

libxed_sources = [
    'xed-app.c',
    'xed-app-activatable.c',
    'xed-view-activatable.c',
    'xed-window-activatable.c',
    'xed-close-button.c',
    'xed-close-confirmation-dialog.c',
    'xed-commands-documents.c',
    'xed-commands-edit.c',
    'xed-commands-file.c',
    'xed-commands-file-print.c',
    'xed-commands-help.c',
    'xed-commands-search.c',
    'xed-commands-view.c',
    'xed-debug.c',
    'xed-dirs.c',
    'xed-document.c',
    'xed-documents-panel.c',
    'xed-encodings-combo-box.c',
    'xed-encodings-dialog.c',
    'xed-file-chooser-dialog.c',
    'xed-history-entry.c',
    'xed-io-error-info-bar.c',
    'xed-message-bus.c',
    'xed-message-type.c',
    'xed-message.c',
    'xed-metadata-manager.c',
    'xed-notebook.c',
    'xed-paned.c',
    'xed-panel.c',
    'xed-plugins-engine.c',
    'xed-preferences-dialog.c',
    'xed-print-job.c',
    'xed-print-preview.c',
    'xed-progress-info-bar.c',
    'xed-settings.c',
    'xed-searchbar.c',
    'xed-statusbar.c',
    'xed-status-combo-box.c',
    'xed-tab.c',
    'xed-tab-label.c',
    'xed-utils.c',
    'xed-view.c',
    'xed-view-frame.c',
    'xed-view-gutter-renderer.c',
    'xed-window.c'
]

xed_enums = gnome.mkenums(
    'xed-enum-types',
    sources : public_headers,
    c_template : 'xed-enum-types.c.template',
    h_template : 'xed-enum-types.h.template',
    identifier_prefix : 'Xed',
    symbol_prefix : 'xed'
)

xed_marshal = gnome.genmarshal(
    'xed-marshal',
    sources: 'xed-marshal.list',
    prefix: 'xed_marshal'
)

xed_resources = gnome.compile_resources(
    'xed-resources', 'resources/xed.gresource.xml',
    source_dir: ['resources', 'resources/css', 'resources/ui'],
    c_name: 'xed'
)

xed_deps = [
    config_h,
    gio,
    glib,
    gtk,
    gtksourceview,
    libpeas,
    libpeas_gtk,
    libxml,
    math,
    xapp,
    X11
]

libxed = library(
    'xed',
    libxed_sources + private_headers + public_headers + xed_enums + xed_marshal + xed_resources,
    dependencies: xed_deps,
    include_directories: include_dirs,
    install: true,
    install_dir: join_paths(libdir, 'xed')
)

install_headers(
    public_headers,
    subdir: 'xed'
)

libxed_dep = declare_dependency(
    dependencies: xed_deps,
    link_with: libxed,
    link_args: ['-Wl,-Bsymbolic', '-Wl,-z,relro', '-Wl,-z,now'],
    include_directories: [include_dirs, include_directories('.')]
)

executable(
    'xed',
    xed_sources,
    dependencies: libxed_dep,
    install_rpath: join_paths(prefix, libdir, 'xed'),
    install: true
)

gir = gnome.generate_gir(
    libxed,
    namespace: 'Xed',
    nsversion: '1.0',
    sources: public_headers + libxed_sources + xed_enums + xed_marshal,
    identifier_prefix: 'Xed',
    symbol_prefix: 'xed',
    includes: ['Gtk-3.0', 'GtkSource-3.0'],
    install: true,
    install_dir_typelib: join_paths(libdir, 'xed', 'girepository-1.0'),
    install_dir_gir: join_paths(datadir, 'xed', 'gir-1.0')
)

pkgconfig.generate(
    name: 'xed',
    description: 'xed',
    filebase: 'xed',
    version: meson.project_version(),
    libraries: libxed,
    requires: [
        'gtksourceview-3.0',
        'libpeas-1.0',
        'libpeas-gtk-1.0'
    ],
    variables: [
        'exec_prefix=${prefix}',
        'pluginsdir=${prefix}/@0@/xed/plugins'.format(libdir)
    ]
)
