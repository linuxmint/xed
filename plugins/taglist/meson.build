taglist_sources = [
    'xed-taglist-plugin-parser.c',
    'xed-taglist-plugin-parser.h',
    'xed-taglist-plugin-panel.c',
    'xed-taglist-plugin-panel.h',
    'xed-taglist-plugin.c',
    'xed-taglist-plugin.h'
]

taglist_in_files = [
    'C.tags@0@',
    'HTML.tags@0@',
    'Latex.tags@0@',
    'XSLT.tags@0@',
    'XUL.tags@0@'
]

taglist_deps = [
    config_h,
    glib,
    gtk,
    gtksourceview,
    libpeas,
    libxml
]

library(
    'taglist',
    taglist_sources,
    link_with: libxed,
    dependencies: taglist_deps,
    include_directories: include_dirs,
    install_rpath: join_paths(prefix, libdir, 'xed'),
    install: true,
    install_dir: join_paths(libdir, 'xed', 'plugins')
)

taglist_desktop = custom_target(
    'taglist_desktop',
    input: 'taglist.plugin.desktop.in',
    output: 'taglist.plugin',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: pluginslibdir,
)

foreach tagtype : taglist_in_files
    taglist_xml = custom_target(
        '@0@xmltags'.format(tagtype.format('')),
        input: tagtype.format('.xml.in'),
        output: tagtype.format(''),
        command: [intltool_merge, '-x', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    )

    taglist_gz = custom_target(
        '@0@gztags'.format(tagtype.format('')),
        input: taglist_xml,
        output: tagtype.format('.gz'),
        command: ['gzip', '-9', '-n', '-k', '-f', '@INPUT@'],
        install: true,
        install_dir: join_paths(pluginsdatadir, 'taglist')
    )
endforeach
