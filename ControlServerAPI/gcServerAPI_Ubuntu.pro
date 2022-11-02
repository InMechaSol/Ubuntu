TEMPLATE = lib
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += shared c++11
DEFINES += PLATFORM_ccOS
QMAKE_CXXFLAGS = -fPIC
QMAKE_CFLAGS = -fPIC
QMAKE_LFLAGS += -lrt -lpthread
QMAKE_EXT_CPP = .cpp
QMAKE_EXT_H = .h .hpp .c

TARGET=gcServerAPI.so
target.path = /usr/bin/gcControl # path on device
INSTALLS += target

gcControlClientDIR = $$absolute_path($${PWD}/../../Lib_GripperAPI/)
ccNOosDIR = $$absolute_path($${PWD}/../../ccNOos/)
ccOSDIR = $$absolute_path($${PWD}/../../ccOS/)
gcControlFWDIR = $$absolute_path($${PWD}/../../Lib_GripperFW/)
ccNOosPlatformDIR = $${ccNOosDIR}/tests/testPlatforms
ccOSMainsDIR = $${PWD}

############################################################ LIBs
# Select All Libs
CONFIG += ccNOosAllLibs
# Select All Devices
CONFIG += ccNOosAllDevs
CONFIG += ccOSDevs_ft232h

include($${gcControlClientDIR}/gcControlClient.pri)

INCLUDEPATH += $$ccOSMainsDIR
INCLUDEPATH += $$ccNOosPlatformDIR

HEADERS += $$ccNOosPlatformDIR/Platform_ccOS.hpp
HEADERS += $$ccOSMainsDIR/Application_Platform_API.hpp

SOURCES += $$ccOSMainsDIR/Application_Platform_API.cpp


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


