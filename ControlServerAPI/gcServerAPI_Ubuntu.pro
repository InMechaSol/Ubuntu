TEMPLATE = lib
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += staticlib c++11
DEFINES += PLATFORM_ccOS
QMAKE_CXXFLAGS = -fPIC
QMAKE_CFLAGS = -fPIC
QMAKE_LFLAGS += -lrt -lpthread
QMAKE_EXT_CPP = .cpp
QMAKE_EXT_H = .h .hpp .c



gcControlClientDIR = $$absolute_path($${PWD}/../../Lib_GripperAPI/)
ccNOosDIR = $$absolute_path($${PWD}/../../ccNOos/)
ccOSDIR = $$absolute_path($${PWD}/../../ccOS/)
gcControlFWDIR = $$absolute_path($${PWD}/../../Lib_GripperFW/)
ccNOosPlatformDIR = $${ccNOosDIR}/tests/testPlatforms
ccOSMainsDIR = $${PWD}




############################################################ LIBs
# Select All Libs
CONFIG += ccNOosLibs_mcs
# Select All Devices
#CONFIG += ccNOosAllDevs
#CONFIG += ccOSDevs_ft232h

TARGET = GripperAPI
target.path = $${PWD}/../GUIAPITests/gripperAPI
INSTALLS += target
headersDataFiles.path = $${PWD}/../GUIAPITests/gripperAPI
headersDataFiles.files = $$ccOSMainsDIR/IMIGripper.hpp
INSTALLS += headersDataFiles

include($${gcControlClientDIR}/gcControlClient.pri)

INCLUDEPATH += $$ccOSMainsDIR
INCLUDEPATH += $$ccNOosPlatformDIR

HEADERS += $$ccNOosPlatformDIR/Platform_ccOS.hpp
HEADERS += $$ccOSMainsDIR/IMIGripper.hpp

SOURCES += $$ccOSMainsDIR/IMIGripper.cpp


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


