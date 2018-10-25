docs_ignore_h = [
    'xed-close-confirmation-dialog.h',
    'xed-dirs.h',
    'xed-commands.h',
    'xed-documents-panel.h',
    'xed-io-error-info-bar.h',
    'xed-languages-manager.h',
    'xed-plugins-engine.h',
    'xed-session.h',
    'xed-ui.h',
    'xed-window-private.h',
    'xed-output-window.h',
    'xed-encodings-dialog.h',
    'xed-open-location-dialog.h',
    'xed-page-setup-dialog.h',
    'xed-preferences-dialog.h',
    'xed-search-dialog.h',
    'xed-marshal.h',
    'xed-enum-types.h'
]

gnome.gtkdoc(
    'xed',
    mode: 'xml',
    install: true,
    main_xml: 'xed-docs.xml',
    gobject_typesfile: 'xed.types',
    src_dir: [
        join_paths(meson.source_root(), 'xed'),
        join_paths(meson.build_root(), 'xed'),
    ],
    ignore_headers: docs_ignore_h,
    dependencies: libxed_dep,
    scan_args: ['--rebuild-types'],
    mkdb_args: ['--xml-mode', '--output-format=xml'],
)
