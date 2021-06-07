
SOURCES += $${PWD}/snap7.cpp

HEADERS += $${PWD}/snap7.h



unix|win32: LIBS += -L$$PWD/./ -lsnap7

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.
