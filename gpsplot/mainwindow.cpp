/* -*- c++ -*- */
/*
 * gpsplot: Simple application to plot GPS data.
 *
 * Copyright 2014 Alexandru Csete OZ9AEC.
 *
 * Gqrx is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * Gqrx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gqrx; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#include <QDebug>
#include <QString>

#include <nmea/nmea.h>

#include "gps_info.h"
#include "mainwindow.h"
#include "nmea_client.h"

#include "ui_mainwindow.h"

void trace(const char *str, int str_size)
{
    (void) str_size;

    fprintf(stderr, "Trace: %s\n", str);
}
void error(const char *str, int str_size)
{
    (void) str_size;
    fprintf(stderr, "Error: %s\n", str);
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    client = new NmeaClient("192.168.1.113", 45000);
    connect(client, SIGNAL(newMessage(QString)), this,
            SLOT(processNmeaMessage(QString)));

    // NMEA parser stuff
    //nmea_property()->trace_func = &trace;
    //nmea_property()->error_func = &error;

    nmea_zero_INFO(&nmea_info);
    nmea_parser_init(&nmea_parser);

}

MainWindow::~MainWindow()
{
    nmea_parser_destroy(&nmea_parser);

    delete client;
    delete ui;
}

/**
 * @brief Process incoming NMEA message.
 * @param nmea_msg The incoming NMEA message.
 */
void MainWindow::processNmeaMessage(QString nmea_msg)
{
    QByteArray ba = nmea_msg.toLocal8Bit();
    nmeaPOS nmea_pos;

    nmea_parse(&nmea_parser, ba.data(), ba.size(), &nmea_info);
    nmea_info2pos(&nmea_info, &nmea_pos);

    ui->gpsInfoText->setLatLonAlt(nmea_radian2degree(nmea_pos.lat),
                                    nmea_radian2degree(nmea_pos.lon),
                                    nmea_info.elv);
    ui->gpsInfoText->setStatus(nmea_info.sig, nmea_info.fix);

    qDebug() << QString("%1:%2:%3").arg(nmea_info.utc.hour).arg(nmea_info.utc.min).arg(nmea_info.utc.sec) <<
                nmea_info.sig << nmea_info.fix <<
                nmea_radian2degree(nmea_pos.lat) << nmea_radian2degree(nmea_pos.lon) << nmea_info.elv <<
                nmea_info.speed << nmea_info.direction;
}
