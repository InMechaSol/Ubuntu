QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
DEFINES += PLATFORM_ccOS
QMAKE_LFLAGS += --verbose -lrt -lpthread
INCLUDEPATH += \
    $${PWD}/../../ccNOos/ccLibs/mcs \
    $${PWD}/../../ccNOos/executionSystem


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    $${PWD}/../../ccNOos/ccLibs/mcs/motionControl.cpp \
    qccos.cpp

HEADERS += \
    mainwindow.h \
    $${PWD}/gripperAPI/IMIGripper.hpp \
    $${PWD}/../../ccNOos/ccLibs/mcs/motionControl.c \
    $${PWD}/../../ccNOos/ccLibs/mcs/motionControl.h \
    $${PWD}/../../ccNOos/executionSystem/version_config.h \
    qccos.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
