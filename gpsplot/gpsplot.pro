#-------------------------------------------------
#
# Project created by QtCreator 2014-12-02T23:53:36
#
#-------------------------------------------------

QT       += core gui network printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gpsplot
TEMPLATE = app
INCLUDEPATH += ../nmealib/include/
#LIBS += /home/alc/gps/gpstools.git/nmealib/lib/libnmea.a
LIBS += ../nmealib/lib/libnmea.a

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    nmea_client.cpp \
    qcustomplot/qcustomplot.cpp \
    gps_info.cpp

HEADERS  += \
    mainwindow.h \
    nmea_client.h \
    qcustomplot/qcustomplot.h \
    gps_info.h


FORMS    += mainwindow.ui \
    gps_info.ui
