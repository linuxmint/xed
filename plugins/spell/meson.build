spell_sources = [
    'xed-spell-plugin.c',
    'xed-spell-plugin.h'
]

spell_marshal = gnome.genmarshal(
    'xed-spell-marshal',
    sources: 'xed-spell-marshal.list',
    prefix: 'xed_spell_marshal'
)

spell_deps = [
    config_h,
    glib,
    gspell,
    gtksourceview,
    libpeas,
    libpeas_gtk
]

library(
    'spell',
    spell_sources + spell_marshal,
    link_with: libxed,
    dependencies: spell_deps,
    include_directories: include_dirs,
    install_rpath: join_paths(prefix, libdir, 'xed'),
    install: true,
    install_dir: join_paths(libdir, 'xed', 'plugins')
)

install_data(
    'xed-spell-setup-dialog.ui',
    install_dir: join_paths(pluginsdatadir, 'spell')
)

spell_desktop = custom_target(
    'spell_desktop',
    input: 'spell.plugin.desktop.in',
    output: 'spell.plugin',
    command: [intltool_merge, '-d', '-u', po_dir, '@INPUT@', '@OUTPUT@'],
    install: true,
    install_dir: pluginslibdir,
)


spell_schema_file = configure_file(
    input: 'org.x.editor.plugins.spell.gschema.xml.in',
    output: 'org.x.editor.plugins.spell.gschema.xml',
    configuration: schema_conf,
    install_dir: join_paths(datadir, 'glib-2.0', 'schemas')
)

install_data(
    spell_schema_file,
    install_dir: schema_dir
)
