TEMPLATE = aux

DOXYGEN_BIN = $$system(command -v doxygen)
isEmpty(DOXYGEN_BIN):error("Unable to detect doxygen in PATH")

QMAKE_CLEAN += $${PWD}/html/* $${PWD}/buteo-syncfw.tag

htmldocs.files = $${PWD}/html/*
htmldocs.path = /usr/share/doc/buteo-syncfw-doc/
htmldocs.CONFIG += no_check_exist
htmldocs.commands = cd $${PWD} && $${DOXYGEN_BIN} Doxyfile

INSTALLS += htmldocs
