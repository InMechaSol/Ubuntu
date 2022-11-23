TEMPLATE = subdirs

SUBDIRS += \
    ConsoleAPITests/ConsoleUI_Ubuntu.pro \
    ControlServerAPI/gcServerAPI_Ubuntu.pro \
    GUIAPITests/GUIAPITests_QtWidgets.pro \
    GripperControlClient/gcControlClient_Ubuntu.pro

GUIAPITests/GUIAPITests_QtWidgets.pro.depends = ControlServerAPI/gcServerAPI_Ubuntu.pro
