QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
DEFINES += PLATFORM_ccOS
QMAKE_LFLAGS += -lrt -lpthread

CONFIG += ccNOosLibs_mcs
CONFIG += GripperAPIfromSource

ccNOosDIR = $$absolute_path($${PWD}/../../ccNOos/)
ccOSDIR = $$absolute_path($${PWD}/../../ccOS/)
ccNOosPlatformDIR = $${ccNOosDIR}/tests/testPlatforms
ccOSMainsDIR = $${PWD}

if(GripperAPIfromSource){
    gcControlClientDIR = $$absolute_path($${PWD}/../../Lib_GripperAPI/)
    gcControlFWDIR = $$absolute_path($${PWD}/../../Lib_GripperFW/)
    include($${gcControlClientDIR}/gcControlClient.pri)
}

if(!GripperAPIfromSource){
    INCLUDEPATH += $${PWD}/gripperAPI
    DEPENDPATH += $${PWD}/gripperAPI
    LIBS += -L$${PWD}/gripperAPI -lGripperAPI
}



QccOSDIR = $$absolute_path($${PWD}/../../Qt/)


if(!GripperAPIfromSource){
    CONFIG += ccNOos_OnlyHeaders
    CONFIG += ccOS_OnlyHeaders
    include($${ccOSDIR}/ccOS.pri)
}



INCLUDEPATH += $$ccOSMainsDIR
INCLUDEPATH += $$ccNOosPlatformDIR
INCLUDEPATH += $$QccOSDIR

if(!GripperAPIfromSource){
    HEADERS += $${PWD}/gripperAPI/IMIGripper.hpp
}
if(GripperAPIfromSource){
    INCLUDEPATH += $$absolute_path($${PWD}/../ControlServerAPI/)
    HEADERS += $${PWD}/../ControlServerAPI/IMIGripper.hpp
    SOURCES += $${PWD}/../ControlServerAPI/IMIGripper.cpp
}
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
