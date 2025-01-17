TEMPLATE = aux

include(../../qtcreator.pri)

STATIC_BASE = $$PWD
STATIC_OUTPUT_BASE = $$IDE_DATA_PATH
STATIC_INSTALL_BASE = $$INSTALL_DATA_PATH

DATA_DIRS = \
    examplebrowser \
    snippets \
    templates \
    themes \
    designer \
    schemes \
    sfdk \
    styles \
    rss \
    debugger \
    qmldesigner \
    qmlicons \
    qml \
    qml-type-descriptions \
    modeleditor \
    glsl \
    mer \
    cplusplus \
    indexer_preincludes \
    android
macx: DATA_DIRS += scripts

for(data_dir, DATA_DIRS) {
    files = $$files($$PWD/$$data_dir/*, true)
    # Info.plist.in are handled below
    for(file, files):!contains(file, ".*/Info\\.plist\\.in$"):!contains(file, ".*__pycache__.*"):!exists($$file/*): \
        STATIC_FILES += $$file
}

include(../../qtcreatordata.pri)
