#-------------------------------------------------
#
# Project created by QtCreator 2021-05-20T19:56:34
#
#-------------------------------------------------

CONFIG += resources_big
QT       += core gui

QT       += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MyKGMusicBox
TEMPLATE = app

include(./netapi/netapi.pri)
INCLUDEPATH += ./netapi/

include(./sqlapi/sqlapi.pri)
INCLUDEPATH += $$PWD/sqlapi

include(./uiapi/uiapi.pri)
INCLUDEPATH += $$PWD/uiapi

INCLUDEPATH += $$PWD/ffmpeg-4.2.2/include\
    $$PWD/SDL2-2.0.10/include


LIBS += $$PWD/ffmpeg-4.2.2/lib/avcodec.lib\
     $$PWD/ffmpeg-4.2.2/lib/avdevice.lib\
     $$PWD/ffmpeg-4.2.2/lib/avfilter.lib\
     $$PWD/ffmpeg-4.2.2/lib/avformat.lib\
     $$PWD/ffmpeg-4.2.2/lib/avutil.lib\
     $$PWD/ffmpeg-4.2.2/lib/postproc.lib\
     $$PWD/ffmpeg-4.2.2/lib/swresample.lib\
     $$PWD/ffmpeg-4.2.2/lib/swscale.lib\
     $$PWD/SDL2-2.0.10/lib/x86/SDL2.lib



SOURCES += main.cpp\
        videoshow.cpp \
    videoplayer.cpp \
    videoitem.cpp \
    packetqueue.cpp \
    videoslider.cpp \
    onlinedialog.cpp \
    ckernel.cpp \
    musicbox.cpp \
    downloadwidget.cpp \
    logindialog.cpp \
    addfriends.cpp \
    useritem.cpp

HEADERS  += videoshow.h \
    videoplayer.h \
    videoitem.h \
    packetqueue.h \
    videoslider.h \
    onlinedialog.h \
    ckernel.h \
    musicbox.h \
    downloadwidget.h \
    logindialog.h \
    addfriends.h \
    useritem.h


FORMS    += videoshow.ui \
    videoitem.ui \
    onlinedialog.ui \
    musicbox.ui \
    downloadwidget.ui \
    logindialog.ui \
    addfriends.ui \
    useritem.ui

RESOURCES += \
    resource.qrc
