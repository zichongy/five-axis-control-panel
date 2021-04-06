QT       += core gui
QT       +=serialport
QT       += multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    image_settings.cpp \
    main.cpp \
    main_control_panel.cpp \
    serial_adapter.cpp \
    toggle_switch.cpp \
    video_adapter.cpp \
    video_settings.cpp

HEADERS += \
    image_settings.h \
    main_control_panel.h \
    serial_adapter.h \
    toggle_switch.h \
    toggle_switch_style.h \
    video_adapter.h \
    video_settings.h

FORMS += \
    image_settings.ui \
    main_control_panel.ui \
    video_adapter.ui \
    video_settings.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
