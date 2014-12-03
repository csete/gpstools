#-------------------------------------------------
#
# Project created by QtCreator 2014-12-02T23:53:36
#
#-------------------------------------------------

QT       += core gui network printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gpsplot
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot/qcustomplot.cpp \
    nmea_client.cpp

HEADERS  += mainwindow.h \
    qcustomplot/qcustomplot.h \
    nmea_client.h

FORMS    += mainwindow.ui
