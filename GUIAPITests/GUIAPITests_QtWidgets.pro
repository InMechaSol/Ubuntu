QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
DEFINES += PLATFORM_ccOS
QMAKE_LFLAGS += -lrt -lpthread
QMAKE_EXT_CPP = .cpp
QMAKE_EXT_H = .h .hpp .c


INCLUDEPATH += $${PWD}/gripperAPI
DEPENDPATH += $${PWD}/gripperAPI
LIBS += -L$${PWD}/gripperAPI -lGripperAPI

ccNOosDIR = $$absolute_path($${PWD}/../../ccNOos/)
ccOSDIR = $$absolute_path($${PWD}/../../ccOS/)
ccNOosPlatformDIR = $${ccNOosDIR}/tests/testPlatforms
ccOSMainsDIR = $${PWD}
QccOSDIR = $$absolute_path($${PWD}/../../Qt/)

CONFIG += ccNOosLibs_mcs
CONFIG += ccNOos_OnlyHeaders
CONFIG += ccOS_OnlyHeaders

include($${ccOSDIR}/ccOS.pri)

INCLUDEPATH += $$ccOSMainsDIR
INCLUDEPATH += $$ccNOosPlatformDIR
INCLUDEPATH += $$QccOSDIR

HEADERS += $${PWD}/gripperAPI/IMIGripper.hpp
HEADERS += $$ccNOosPlatformDIR/Platform_ccOS.hpp
HEADERS += $$ccOSMainsDIR/mainwindow.h
HEADERS += $$QccOSDIR/qccos.h

SOURCES += $$ccOSMainsDIR/main.cpp
SOURCES += $$ccOSMainsDIR/mainwindow.cpp
SOURCES += $$QccOSDIR/qccos.cpp

FORMS += mainwindow.ui

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


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
