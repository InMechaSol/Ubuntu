TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11
DEFINES += PLATFORM_ccOS
LIBS += -pthread
QMAKE_LFLAGS += -lrt -lpthread
QMAKE_EXT_CPP = .cpp
QMAKE_EXT_H = .h .hpp .c

TARGET=gcControl_UI.exe
target.path = /usr/bin/acu # path on device
INSTALLS += target

ccNOosDIR = $$absolute_path($${PWD}/../../ccNOos/)
ccOSDIR = $$absolute_path($${PWD}/../../ccOS/)
ccNOosPlatformDIR = $${ccNOosDIR}/tests/testPlatforms
ccOSMainsDIR = $${ccOSDIR}/tests/testMainFiles/menuAPIclient

CONFIG += ccOS_menuClient
include($${ccOSDIR}/ccOS.pri)

INCLUDEPATH += $$ccOSMainsDIR
INCLUDEPATH += $$ccNOosPlatformDIR

HEADERS += $$ccNOosPlatformDIR/Platform_ccOS.hpp

SOURCES += $$ccOSMainsDIR/menuAPIclient_main.cpp


message("Includes:")
for(msg, INCLUDEPATH) {
    message($$msg)
}
message("Headers:")
for(msg, HEADERS) {
    message($$msg)
}
message("Sources:")
for(msg, SOURCES) {
    message($$msg)
}
message("Objects:")
for(msg, OBJECTS) {
    message($$msg)
}
message("Source Extensions:")
for(msg, QMAKE_EXT_CPP) {
    message($$msg)
}
message("Header Extensions:")
for(msg, QMAKE_EXT_H) {
    message($$msg)
}


