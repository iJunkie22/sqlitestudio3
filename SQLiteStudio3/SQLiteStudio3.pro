TEMPLATE = subdirs

core.subdir = coreSQLiteStudio

tests.subdir = Tests
tests.depends = core

gui.subdir = guiSQLiteStudio
gui.depends = core

cli.subdir = sqlitestudiocli
cli.depends = core

gui_app.subdir = sqlitestudio
gui_app.depends = gui

SUBDIRS += \
    core \
    gui \
    cli \
    UpdateSQLiteStudio \
    gui_app

if(contains(DEFINES,tests)) {
    SUBDIRS += tests
}

macx: {
    dest_dir = $$PWD/../output/SQLiteStudio

    clean_bundle.commands = rm -rf $$dest_dir/sqlitestudio.app/Contents/Frameworks && rm -rf $$dest_dir/sqlitestudio.app/Contents/plugins \
        && rm -f $$dest_dir/sqlitestudio.app/Contents/MacOS/lib*.dylib&& rm -f $$dest_dir/sqlitestudio.app/Contents/Resources/qt.conf

    prepare_dirs.depends = clean_bundle
    prepare_dirs.commands = mkdir $$dest_dir/sqlitestudio.app/Contents/Frameworks

    copy_plugins.depends = prepare_dirs
    copy_plugins.commands = cp -RP $$dest_dir/plugins $$dest_dir/sqlitestudio.app/Contents \
        && mv $$dest_dir/sqlitestudio.app/Contents/plugins $$dest_dir/sqlitestudio.app/Contents/PlugIns

    copy_app_libs.depends = copy_plugins
    copy_app_libs.commands = cp -RP $$dest_dir/lib*SQLiteStudio*.dylib $$dest_dir/sqlitestudio.app/Contents/Frameworks

    copy_deps.depends = copy_app_libs
    copy_deps.commands = cp -RP $$PWD/../../lib/*.dylib $$dest_dir/sqlitestudio.app/Contents/Frameworks

    macdeploy_bin = $$QMAKE_QMAKE
    macdeploy_bin = $$replace(macdeploy_bin, /qmake, /macdeployqt)

    deploy_qt.depends = copy_deps
    deploy_qt.commands = $$macdeploy_bin $$dest_dir/sqlitestudio.app

    bundle.depends = deploy_qt

    QMAKE_EXTRA_TARGETS += clean_bundle prepare_dirs copy_plugins copy_app_libs copy_deps deploy_qt bundle
}
